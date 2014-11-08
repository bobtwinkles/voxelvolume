cmake_minimum_required(VERSION 2.0)

project(srp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

list(APPEND CMAKE_CXX_FLAGS "-std=c++11")
list(APPEND CMAKE_CXX_DEBUG_FLAGS "-g -Wall -Werror -v")

find_package(GLUT)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)

include_directories(SYSTEM ${GLEW_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR})

set (SRCS Main.cpp
          DataStore.cpp
          RenderChunk.cpp
          Tetrahedron.cpp
          Util.cpp
          XWindow.cpp
          )

add_executable(main ${SRCS})
target_link_libraries(main ${X11_LIBRARY} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES})

if(${GLUT_FOUND})
  include_directories(SYSTEM ${GLUT_INCLUDE_DIRS})
  add_executable(data_render DataRender.cpp DataStore.cpp)
  target_link_libraries(data_render ${GLUT_LIBRARY} ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES})
endif(${GLUT_FOUND})

install(DIRECTORY data DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
install(FILES gen.py DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
