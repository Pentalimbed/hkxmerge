#include "merger.h"

#define COMPRESSED

#include <algorithm>
#include <string>
#include <vector>
#include <fstream>

const float fps         = 30.0f;
const float frame_delta = 1.0f / fps;
const bool  enable_test = false;

void process(int argc, char* argv[])
{

    // Arg parsing
    if (!enable_test && (argc != 4))
    {
        LOG("Wrong parameter count. Format: in_a.hkx in_b.hkx out.hkx");
        return;
    }
    std::string dir_a, dir_v, dir_out;
    if (enable_test)
    {
        dir_a   = "E:/dev/projects/HkxMerge/hkx/a.hkx";
        dir_v   = "E:/dev/projects/HkxMerge/hkx/v.hkx";
        dir_out = "E:/dev/projects/HkxMerge/hkx/out.hkx";
    }
    else
    {
        dir_a   = argv[1];
        dir_v   = argv[2];
        dir_out = argv[3];
    }

    LOG("Input file 1: " << dir_a << std::endl
                         << "Input file 2: " << dir_v << std::endl
                         << "Output file: " << dir_out);

    // Raeading files and checking
    hkResource* data_a = loadData(dir_a);
    hkResource* data_v = loadData(dir_v);
    if (!(data_a && data_v))
        return;

    auto anim_a = static_cast<hkaInterleavedUncompressedAnimation*>(reinterpret_cast<hkaAnimationContainer*>(data_a->getContents<hkRootLevelContainer>()->findObjectByType(hkaAnimationContainerClass.getName()))->m_animations[0].val());
    auto anim_v = static_cast<hkaInterleavedUncompressedAnimation*>(reinterpret_cast<hkaAnimationContainer*>(data_v->getContents<hkRootLevelContainer>()->findObjectByType(hkaAnimationContainerClass.getName()))->m_animations[0].val());
    if (!checkSame(anim_a, anim_v))
    {
        LOG("Two animations duration not matched!");
        return;
    }

    // - Crafting the new animation
    LOG("Start merging animation!");
    int   ntracks  = anim_a->m_numberOfTransformTracks + anim_v->m_numberOfTransformTracks + 3;
    float duration = anim_a->m_duration;
    int   nframes  = (int)roundf(duration / frame_delta);

    // Uncompressed Animation init
    hkRefPtr<hkaInterleavedUncompressedAnimation> temp_anim = new hkaInterleavedUncompressedAnimation();
    temp_anim->m_duration                                   = anim_a->m_duration;
    temp_anim->m_numberOfTransformTracks                    = ntracks;
    temp_anim->m_numberOfFloatTracks                        = 0;
    temp_anim->m_transforms.setSize(ntracks * nframes, hkQsTransform::getIdentity());
    temp_anim->m_floats.setSize(temp_anim->m_numberOfFloatTracks);
    temp_anim->m_annotationTracks.setSize(ntracks);

    // Annotations
    LOG("Merging annotations!");
    hkArray<hkaAnnotationTrack>& anno_tracks = temp_anim->m_annotationTracks;

    anno_tracks[0].m_trackName = "PairedRoot";
    anno_tracks[1].m_trackName = "2_";
    for (auto i = 0; i < anim_v->m_numberOfTransformTracks; i++)
    {
        auto idx             = i + 2;
        anno_tracks[idx]     = anim_v->m_annotationTracks[i];
        std::string temp_str = "2_";
        if (anim_v->m_annotationTracks[i].m_trackName.cString())
        {
            temp_str += anim_v->m_annotationTracks[i].m_trackName.cString();
        }
        anno_tracks[idx].m_trackName = temp_str.c_str();
    }
    anno_tracks[anim_a->m_numberOfTransformTracks + 2].m_trackName = "NPC";
    for (auto i = 0; i < anim_a->m_numberOfTransformTracks; i++)
    {
        auto idx         = i + anim_v->m_numberOfTransformTracks + 3;
        anno_tracks[idx] = anim_a->m_annotationTracks[i];
    }

    // Fill in transform
    LOG("Merging transforms! If programs stops here without any output, please try a few more times.");
    hkArray<hkQsTransform>& transforms = temp_anim->m_transforms;

    int idx = 0;
    for (int frame = 0; frame < nframes; frame++)
    {
        float curr_time = frame * duration / nframes;

        // Paired root
        idx++;

#ifndef COMPRESSED
        // UNCOMPRESSED
        // Victim anim
        idx++;
        memcpy(transforms.begin() + idx,
               anim_v->m_transforms.begin() + (frame * anim_v->m_numberOfTransformTracks),
               sizeof(hkQsTransform) * (anim_v->m_numberOfTransformTracks));
        idx += anim_v->m_numberOfTransformTracks;

        // Attacker anim
        idx++;
        memcpy(transforms.begin() + idx,
               anim_a->m_transforms.begin() + (frame * anim_a->m_numberOfTransformTracks),
               sizeof(hkQsTransform) * (anim_a->m_numberOfTransformTracks));
        idx += anim_a->m_numberOfTransformTracks;
#else
        // COMPRESSED
        // Victim anim
        idx++;
        anim_v->sampleTracks(curr_time, transforms.begin() + idx, nullptr, nullptr); // Causes access violation for some reason
        idx += anim_v->m_numberOfTransformTracks;

        // Attacker anim
        idx++;
        anim_a->sampleTracks(curr_time, transforms.begin() + idx, nullptr, nullptr);
        idx += anim_a->m_numberOfTransformTracks;
#endif
    }

    // swap
    for (int frame = 0; frame < nframes; frame++)
    {
        auto frame_begin = transforms.begin() + frame * ntracks;
        std::swap(*(frame_begin + 1), *(frame_begin + 2));
        std::swap(*(frame_begin + anim_v->m_numberOfTransformTracks + 2),
                  *(frame_begin + anim_v->m_numberOfTransformTracks + 3));
        // revert rotation?
        // (frame_begin + 2)->m_rotation.setInverse((frame_begin + 1)->m_rotation);
        // (frame_begin + anim_v->m_numberOfTransformTracks + 3)->m_rotation.setInverse((frame_begin + anim_v->m_numberOfTransformTracks + 2)->m_rotation);
    }

    hkaSkeletonUtils::normalizeRotations(transforms.begin(), transforms.getSize());

    // Shit compressor
    LOG("Compressing!");
    hkaSplineCompressedAnimation::TrackCompressionParams     tparams;
    hkaSplineCompressedAnimation::AnimationCompressionParams aparams;

    tparams.m_rotationTolerance        = 0.0001f;
    tparams.m_rotationQuantizationType = hkaSplineCompressedAnimation::TrackCompressionParams::THREECOMP40;
    aparams.m_enableSampleSingleTracks = true;

    hkRefPtr<hkaSplineCompressedAnimation> compressed_anim = new hkaSplineCompressedAnimation(*temp_anim.val(), tparams, aparams);

    // - Animation Creation Done

    // Binding
    LOG("Create binding!");
    hkRefPtr<hkaAnimationBinding> binding = new hkaAnimationBinding();
    binding->m_originalSkeletonName       = "PairedRoot";
    binding->m_animation                  = compressed_anim;
    binding->m_blendHint                  = hkaAnimationBinding::BlendHint::NORMAL;

    // Anim container
    LOG("Create anim container!");
    hkRefPtr<hkaAnimationContainer> anim_cont = new hkaAnimationContainer();
    anim_cont->m_bindings.append(&binding, 1);
    anim_cont->m_animations.pushBack(binding->m_animation);
    anim_cont->m_skeletons.clear();
    anim_cont->m_attachments.clear();
    anim_cont->m_skins.clear();

    // Root & Export
    hkRootLevelContainer root_cont;
    root_cont.m_namedVariants.pushBack(
        hkRootLevelContainer::NamedVariant("Merged Animation Container", anim_cont.val(), &anim_cont->staticClass()));

    hkOstream                 stream(dir_out.c_str());
    hkVariant                 root = {&root_cont, &root_cont.staticClass()};
    hkPackfileWriter::Options options;
    options.m_layout                      = hkStructureLayout::MsvcWin32LayoutRules;
    hkSerializeUtil::SaveOptionBits flags = hkSerializeUtil::SAVE_DEFAULT;
    hkResult                        res   = hkSerializeUtil::savePackfile(root.m_object, *root.m_class, stream.getStreamWriter(), options, HK_NULL, flags);
    // hkResult                        res   = hkSerializeUtil::saveTagfile(root.m_object, *root.m_class, stream.getStreamWriter(), HK_NULL, flags);

    if (res == HK_SUCCESS)
    {
        LOG("Successfully exported!");
    }
    else
    {
        LOG("Failed to save export file!");
    }

    hkOstream           stream_xml((dir_out + ".xml").c_str());
    hkXmlPackfileWriter writer;
    writer.setContents(&root_cont, *root.m_class);
    res = writer.save(stream_xml.getStreamWriter(), options);

    if (res == HK_SUCCESS)
    {
        LOG("XML HKX file Successfully exported!");
    }
    else
    {
        LOG("Failed to save export file!");
    }
}

hkResource* loadData(std::string path)
{
    hkSerializeUtil::ErrorDetails loadError;
    hkResource*                   data = hkSerializeUtil::load(path.c_str(), &loadError);
    if (!data)
    {
        LOG("Could not load " << path << std::endl
                              << " Error: " << loadError.defaultMessage.cString());
        return nullptr;
    }
    hkRootLevelContainer* container = data->getContents<hkRootLevelContainer>();
    if (container == HK_NULL)
    {
        LOG("Could not load root level object for" << path);
        return nullptr;
    }
    hkaAnimationContainer* ac = reinterpret_cast<hkaAnimationContainer*>(container->findObjectByType(hkaAnimationContainerClass.getName()));
    if (!ac || (ac->m_animations.getSize() == 0))
    {
        LOG("No animation loaded for " << path);
        return nullptr;
    }
    if (ac->m_animations.getSize() > 1)
    {
        LOG("Multiple animations found in " << path << "! Abort!");
        return nullptr;
    }
    return data;
}

bool checkSame(hkaAnimation* a, hkaAnimation* b)
{
    return compareFloat(a->m_duration, b->m_duration);
}