cmake_minimum_required(VERSION 3.12)
set(CXX_STANDARD 11)

project(hkxmerge VERSION 1.0.0)

set(HavokPath "E:/dev/libs/cpp/hk2014_1_0_r1")
set(HavokLibPath "${HavokPath}/Lib/win32_vs2012_win7/debug")
set(HavokInclPath "${HavokPath}/Source")

add_executable(${PROJECT_NAME} impl.cpp main.cpp merger.cpp)

target_link_directories(
    ${PROJECT_NAME} 
    PRIVATE
        ${HavokLibPath})

file(GLOB_RECURSE HavokLibs ${HavokLibPath}/*.lib)
target_link_libraries(
    ${PROJECT_NAME} 
    PRIVATE
        ${HavokLibs}
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        ${HavokInclPath}
)

target_compile_definitions(
	${PROJECT_NAME}
	PRIVATE
		_UNICODE
)