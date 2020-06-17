cmake_minimum_required(VERSION 3.13)
project(SomHunter)

add_compile_definitions("NOMINMAX")

set(BUILD_PLUGINS_STATIC ON
    CACHE BOOL "prevent problems with dynamic builds")

add_subdirectory(src)
