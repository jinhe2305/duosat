
cmake_minimum_required(VERSION 3.4.1)

#add_library(
#    miniupnpc
#    SHARED
#    src/main/cpp/miniupnpc.cpp)

#add_library(
#    gbsp
#    SHARED
#    src/main/cpp/gbsp.cpp
#    )

add_library(
    mongoose
    SHARED
    src/main/cpp/mongoose.c)

add_library(
    gpa
    SHARED
    src/main/cpp/sha1.c
    src/main/cpp/goalbit_android.cpp
    src/main/cpp/goalbit/gpa.cpp
    src/main/cpp/md5.c
    src/main/cpp/miniupnpc/igd_desc_parse.c
    src/main/cpp/miniupnpc/miniupnpc.c
    src/main/cpp/miniupnpc/minixml.c
    src/main/cpp/miniupnpc/minisoap.c
    src/main/cpp/miniupnpc/minissdpc.c
    src/main/cpp/miniupnpc/miniwget.c
    src/main/cpp/miniupnpc/upnpcommands.c
    #src/main/cpp/miniupnpc/upnpdev.c
    src/main/cpp/miniupnpc/upnpreplyparse.c
    src/main/cpp/miniupnpc/upnperrors.c
    src/main/cpp/miniupnpc/connecthostport.c
    src/main/cpp/miniupnpc/portlistingparse.c
    src/main/cpp/miniupnpc/receivedata.c
    )

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

find_library( # Sets the name of the path variable.
              z-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              z )

add_library(
    curl
    STATIC
    IMPORTED
    )

set_target_properties(
    curl
    PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../prebuilt-with-ssl/android/${ANDROID_ABI}/libcurl.a
    )

target_include_directories(
    gpa
    PRIVATE
    ${CMAKE_SOURCE_DIR}/../prebuilt-with-ssl/android/include
    src/main/cpp/miniupnpc
    src/main/cpp
    )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.
target_link_libraries( # Specifies the target library.
                       gpa

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib}
                       ${z-lib}
                       mongoose
                       curl
                        )

add_definitions(-DGOALBIT_ANDROID)