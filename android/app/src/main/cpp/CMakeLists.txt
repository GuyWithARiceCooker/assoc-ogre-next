cmake_minimum_required(VERSION 3.22.1)
project(assoc_android LANGUAGES CXX)

add_library(assoc_android SHARED assoc_android.cpp)

target_compile_features(assoc_android PRIVATE cxx_std_17)
target_link_libraries(assoc_android PRIVATE log)
