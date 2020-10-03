
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

LINK_DIRECTORIES(/usr/local/Cellar/boost/1.73.0/lib)

ADD_EXECUTABLE(demo demo.cpp)
TARGET_LINK_LIBRARIES(demo boost_program_options)

#ADD_EXECUTABLE(load_test load_test.cpp)
#ADD_SUBDIRECTORY(unittest)
