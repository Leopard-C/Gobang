{
    files = {
        "src/base.cpp",
        "src/base.h",
        "src/base.cpp"
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