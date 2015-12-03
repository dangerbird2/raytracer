cmake_minimum_required(VERSION 2.8)
PROJECT(RAYTRACER)
SET(CMAKE_CXX_FLAGS "-std=gnu++1y -Wno-deprecated")

SET(MY_SOURCE_PATH ${CMAKE_SOURCE_DIR})
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/source/common/SourcePath.cpp.in ${CMAKE_SOURCE_DIR}/source/common/SourcePath.cpp)	
	
find_package(OpenGL)
find_package(GLUT)

INCLUDE_DIRECTORIES(./source
                    ./source/common
                    ./shaders
                    ${OPENGL_INCLUDE_DIR} 
                    ${GLUT_INCLUDE_DIR})
					
					

SUBDIRS(${PROJECT_SOURCE_DIR}/FreeImage3151)	

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/FreeImage3151/Source")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/FreeImage3151/Source/LibJPEG")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/FreeImage3151/Source/LibMNG")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/FreeImage3151/Source/LibOpenJPEG")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/FreeImage3151/Source/LibPNG/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/FreeImage3151/Source/LibRawLite")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/FreeImage3151/Source/LibTIFF")

ADD_EXECUTABLE(rayTracer
		source/common/Angel.h
		source/common/CheckError.h
		source/common/mat.h
		source/common/vec.h
		source/common/ObjMesh.cpp
		source/common/ObjMesh.h
		source/common/InitShader.cpp
		source/common/SourcePath.cpp
		source/common/SourcePath.h
		source/common/Trackball.cpp
		source/common/Trackball.h
		shaders/vshading_example.glsl
		shaders/fshading_example.glsl
		source/Raytracer.cpp source/common-math.h source/image-utils.cc source/image-utils.h source/renderer.cc source/renderer.h source/types.h source/async-tools.h)

TARGET_LINK_LIBRARIES(rayTracer ${OPENGL_LIBRARY} ${GLUT_LIBRARY} FreeImage)

