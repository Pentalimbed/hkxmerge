#pragma once

#if defined(_WIN32) && !defined(_XBOX) && !defined(__HAVOK_PARSER__)
// Do a full windows incl so that Com etc included.
#    if !defined(HK_PLATFORM_WINRT) && (!defined(WINAPI_FAMILY) || (WINAPI_FAMILY == 1))
#        define _WIN32_WINNT 0x0500 // Windows2000 or higher
#        if (_MSC_VER >= 1400)
#            define _CRT_SECURE_NO_DEPRECATE  1
#            define _CRT_NONSTDC_NO_DEPRECATE 1
#        endif
#        define NOMINMAX
#        ifdef HK_ENABLE_SCRIPT
#            include <winsock2.h>
#        endif
#    endif
#    include <windows.h>
#endif

#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>
#include <Common/Base/Container/String/hkStringBuf.h>
#include <Common/Base/Ext/hkBaseExt.h>

#include <Common/Base/Memory/System/Util/hkMemoryInitUtil.h>
#include <Common/Base/Memory/Allocator/Malloc/hkMallocAllocator.h>

#include <Animation/Animation/hkaAnimationContainer.h>
#include <Animation/Animation/Animation/SplineCompressed/hkaSplineCompressedAnimation.h>
#include <Animation/Animation/Rig/hkaSkeletonUtils.h>

#include <Common/Serialize/Util/hkSerializeUtil.h>
#include <Common/Serialize/Util/hkRootLevelContainer.h>
#include <Common/Serialize/ResourceDatabase/hkResourceHandle.h>
#include <Common/Compat/Deprecated/Packfile/Xml/hkXmlPackfileWriter.h>

#include <Common/Base/KeyCode.h>