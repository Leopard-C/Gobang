target("gobang_server")
    set_kind("binary")

    -- std=c++11
    set_languages("c99", "cxx14")

    -- jsoncpp
    add_includedirs("src/jsoncpp")

    -- source file
    add_files("src/*.cpp")
    add_files("src/jsoncpp/*.cpp")

    -- link flags
    add_links("pthread")

    -- build dir
    set_targetdir("$(projectdir)")
    set_objectdir("build/objs")

    add_mflags("-g", "-O2", "-DDEBUG")

