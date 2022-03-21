cmake_minimum_required(VERSION 3.2)
project(simple)
set(CMAKE_CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})

set(SIMPLE_SOURCE_FILES
    sqlite3/sqlite3ext.h
    sqlite3/sqlite3.h
    sqlite3/sqlite3.c
    src/pinyin.h
    src/pinyin.cc
    src/pinyin_const.h
    src/simple_highlight.h
    src/simple_highlight.cc
    src/simple_tokenizer.h
    src/simple_tokenizer.cc
    src/entry.cc
)

#------------------------------------------------------------------------------
# build options and optional modules:
option(SQLITE_ENABLE_DBSTAT_VTAB  "enables dbstat virtual table"                 OFF)
option(SQLITE_ENABLE_FTS3         "enables full text searches version 3"         OFF)
option(SQLITE_ENABLE_FTS4         "enables full text searches version 3 & 4"     OFF)
option(SQLITE_ENABLE_FTS5         "enables full text searches version 5"         ON)
option(SQLITE_ENABLE_GEOPOLY      "enables Geopoly extention"                    OFF)
option(SQLITE_ENABLE_ICU          "enables international components for unicode" OFF)
option(SQLITE_ENABLE_JSON1        "enables JSON SQL functins"                    ON)
option(SQLITE_ENABLE_RBU          "enables resumable bulk update extension"      OFF)
option(SQLITE_ENABLE_RTREE        "enables R*TRee index extension"               OFF)
option(SQLITE_RECOMMENDED_OPTIONS "compile by SQLite3 recommended options"       ON)


add_library(simple STATIC ${SIMPLE_SOURCE_FILES})

set(SQLITE3_HEADERS_DIR ${PROJECT_SOURCE_DIR}/sqlite3)
set(SIMPLE_HEADERS_DIR ${PROJECT_SOURCE_DIR}/src)

include_directories(${SQLITE3_HEADERS_DIR} ${SIMPLE_HEADERS_DIR} ${PROJECT_SOURCE_DIR})
target_include_directories(simple INTERFACE ${SQLITE3_HEADERS_DIR})

target_compile_definitions(simple PUBLIC # inject user's options
    $<BUILD_INTERFACE:
        $<$<BOOL:${SQLITE_ENABLE_DBSTAT_VTAB}>:SQLITE_ENABLE_DBSTAT_VTAB>
        $<$<BOOL:${SQLITE_ENABLE_FTS3}>:SQLITE_ENABLE_FTS3>
        $<$<BOOL:${SQLITE_ENABLE_FTS4}>:SQLITE_ENABLE_FTS4>
        $<$<BOOL:${SQLITE_ENABLE_FTS5}>:SQLITE_ENABLE_FTS5>
        $<$<BOOL:${SQLITE_ENABLE_GEOPOLY}>:SQLITE_ENABLE_GEOPOLY>
        $<$<BOOL:${SQLITE_ENABLE_ICU}>:SQLITE_ENABLE_ICU>
        $<$<BOOL:${SQLITE_ENABLE_JSON1}>:SQLITE_ENABLE_JSON1>
        $<$<BOOL:${SQLITE_ENABLE_RBU}>:SQLITE_ENABLE_RBU>
        $<$<BOOL:${SQLITE_ENABLE_RTREE}>:SQLITE_ENABLE_RTREE>
        $<$<BOOL:${SQLITE_RECOMMENDED_OPTIONS}>:
            SQLITE_DQS=0
            SQLITE_DEFAULT_MEMSTATUS=0
            SQLITE_DEFAULT_WAL_SYNCHRONOUS=1
            SQLITE_LIKE_DOESNT_MATCH_BLOBS
            SQLITE_MAX_EXPR_DEPTH=0
            SQLITE_OMIT_DECLTYPE
            SQLITE_OMIT_DEPRECATED
            SQLITE_OMIT_PROGRESS_CALLBACK
            SQLITE_OMIT_SHARED_CACHE
            SQLITE_USE_ALLOCA
        >
    >
    )

add_subdirectory(examples)