cmake_minimum_required(VERSION 2.0)

project(srp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

list(APPEND CMAKE_CXX_FLAGS "-std=c++11")
list(APPEND CMAKE_CXX_DEBUG_FLAGS "-g -Wall -Werror -v")

find_package(GLUT)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)
find_package(GLM REQUIRED)

include_directories(SYSTEM ${GLM_INCLUDE_DIR} ${GLEW_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR})

set (SRCS Main.cpp
          DataStore.cpp
          RenderChunk.cpp
          Tetrahedron.cpp
          Util.cpp
          XWindow.cpp
          ogl/Shader.cpp
          )

add_executable(main ${SRCS})
target_link_libraries(main ${X11_LIBRARY} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES})

include_directories("${CMAKE_SOURCE_DIR}/" "${CMAKE_SOURCE_DIR}/ogl")
if(${GLUT_FOUND})
  include_directories(SYSTEM ${GLUT_INCLUDE_DIRS})
  add_executable(data_render DataRender.cpp DataStore.cpp)
  target_link_libraries(data_render ${GLUT_LIBRARY} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES})
endif(${GLUT_FOUND})

install(DIRECTORY data DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
install(DIRECTORY shaders DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
install(FILES gen.py DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
