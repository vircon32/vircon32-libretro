cmake_minimum_required(VERSION 3.13)

project(embed-binaries-example)

include("${CMAKE_CURRENT_LIST_DIR}/../cmake/embed-binaries.cmake")

embed_binaries(my-embedded-binaries
    ASSET
        NAME "vertex_shader"
        PATH "shaders/dummy_shader.vert"

        # By default, an extern const array will be declared in the header,
        # with the contents in a corresponding .c file,
        # which makes the header as short as possible, which
        # improves compile time, especially when embedding larger assets.
        #
        # With the 'CONSTEXPR' option, a constexpr array is
        # generated, which MUST be in the header so that we can use the
        # embedded data in a constexpr context (C++).
        CONSTEXPR
        BYTE_TYPE "char" # Useful when embedding ascii string data (defaults to "unsigned char")
        # Useful when embedding string data that will be used with APIs expecting
        # null-terminated strings (although OpenGL does not require it for shaders, this is not a good example)
        NULL_TERMINATE
    ASSET
        NAME "fragment_shader"
        PATH "shaders/dummy_shader.frag"
        CONSTEXPR
        BYTE_TYPE "char"
        NULL_TERMINATE
    ASSET
        NAME "application_logo"
        PATH "assets/application_logo.png"
)

add_executable(example src/main.cc)
target_link_libraries(example PRIVATE my-embedded-binaries)
set_target_properties(example PROPERTIES
    # So that the program finds the original assets (see main.cc)
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
)
