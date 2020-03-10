{
    files = {
        "src/jsoncpp/json_value.cpp",
        "src/jsoncpp/json/assertions.h",
        "src/jsoncpp/json/config.h",
        "src/jsoncpp/json/allocator.h",
        "src/jsoncpp/json/version.h",
        "src/jsoncpp/json/value.h",
        "src/jsoncpp/json/forwards.h",
        "src/jsoncpp/json/writer.h",
        "src/jsoncpp/json_valueiterator.inl",
        "src/jsoncpp/json_value.cpp"
    },
    values = {
        "/usr/bin/gcc",
        {
            "-std=c++14",
            "-Isrc/jsoncpp",
            "-m64",
            "-I/usr/local/include",
            "-I/usr/include"
        }
    }
}