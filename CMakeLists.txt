cmake_minimum_required(VERSION 3.13)

if(IDF_VERSION_MAJOR GREATER_EQUAL 4)
    idf_component_register(SRC_DIRS src
        INCLUDE_DIRS src)
else()
    set(COMPONENT_SRCDIRS src)
    set(COMPONENT_ADD_INCLUDEDIRS src)
    register_component()
endif()

