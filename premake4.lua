solution("fast_zcat")

configurations({"Debug", "Static"})

configuration("Debug")
flags({"Symbols"})

configuration("Static")
flags({"OptimizeSpeed"})
buildoptions({"-O3", "-march=native", "-mtune=native", "-funroll-loops"})
linkoptions({"-O4", "-static"}) -- Link time optimization
flags({"StaticRuntime"})

configuration({})

project("fzcat")
kind("ConsoleApp")
language("C++")
buildoptions({"-std=c++14"})
linkoptions({"-Wl,--whole-archive -lpthread -Wl,--no-whole-archive"})
includedirs({"3rdparty/miniz", "src"})
files({"src/main.cpp",
       "src/decompressor.cpp",
})
