PROJECT(CPPJIEBA)

CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/deps
  ${PROJECT_SOURCE_DIR}/include
  /usr/local/Cellar/boost/1.73.0/include)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "/usr/local/cppjieba" CACHE PATH "default install path" FORCE )
endif()

ADD_DEFINITIONS(-O3 -Wall)
IF(APPLE) # mac os
    ADD_DEFINITIONS(-std=c++17)
endif()

# cmake .. -DENC=GBK
# IF (DEFINED ENC)
#     ADD_DEFINITIONS(-DCPPJIEBA_${ENC})
# ENDIF()

ADD_SUBDIRECTORY(deps)
ADD_SUBDIRECTORY(test)

ENABLE_TESTING()
ADD_TEST(NAME ./test/test.run COMMAND ./test/test.run)
ADD_TEST(NAME ./load_test COMMAND ./load_test)
ADD_TEST(NAME ./demo COMMAND ./demo)
