cmake_minimum_required(VERSION 3.4)

#project(an3tp1 VERSION 1.0.0.0)
#message("CMAKE_PROJECT_NAME = ${CMAKE_PROJECT_NAME}")
#message("PROJECT_VERSION = ${PROJECT_VERSION}")
#message("PROJECT_VERSION_MAJOR    = ${PROJECT_VERSION_MAJOR}")
#message("PROJECT_VERSION_MINOR = ${PROJECT_VERSION_MINOR}")
#message("PROJECT_VERSION_PATCH = ${PROJECT_VERSION_PATCH}")
#message("PROJECT_VERSION_TWEAK = ${PROJECT_VERSION_TWEAK}")
set(aa_tp1_TARGET_NAME aa_tp1)

include_directories(${DEPS_INCLUDE_DIC})
include_directories(${ASIO_INCLUDE_DIC})
include_directories(${BOOST_INCLUDE_DIC})
include_directories(.)

message("aa_tp1 BOOST_INCLUDE_DIC = ${BOOST_INCLUDE_DIC}")
message("aa_tp1 ASIO_INCLUDE_DIC = ${ASIO_INCLUDE_DIC}")


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_SKIP_BUILD_RPATH true)
aux_source_directory(. SRCS_DIR)
#aux_source_directory(../deps/CJsonObject SRCS_DIR)
#aux_source_directory(../deps/sqlite3.33 SRCS_DIR)

link_directories(${BOOST_LIB_DIC})
#link_libraries(${TCMALLOC_LIB})
add_executable(${aa_tp1_TARGET_NAME} ${SRCS_DIR})

if (NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++")
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -fPIC -shared")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -shared")
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free" )
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free" )

    
    set_target_properties(${aa_tp1_TARGET_NAME} PROPERTIES LINK_FLAGS "-Wl,-rpath,./")

    #target_link_libraries(an3tp1 pthread dl rt z crypto ssl vrl)

    #link libnanomsg.a
    target_link_libraries(${aa_tp1_TARGET_NAME} dl libboost_coroutine.a libboost_thread.a libboost_context.a -pthread)
    
    #link libnng.a
    #target_link_libraries(${an3tp1_TARGET_NAME} dl rt -pthread nsl ${HIREDIS_LIB} ${NNG_LIB} )  
    
else()
    target_link_libraries(${aa_tp1_TARGET_NAME} ${UV_LIB} ${CURL_LIB} ${VRL_LIB})
endif()
