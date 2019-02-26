# prettysql
SQL Pretty printer using simplesql C++ library, with optional schema awareness

## Dependencies
- simplesql C++ library
  https://github.com/packetzero/simplesql
- antlr4 C++ runtime:
  `brew install antlr4-cpp-runtime`
- dyno
  https://github.com/packetzero/dyno
- rapidjson
  Add to CFLAGS: -DPRETTYSQL_JSON=1
  `brew install rapidjson`

## Build Without Tests
```
mkdir build && cd build
cmake
```

## Building for MacOS XCode

```
mkdir build && cd build
MAKE_TESTS=1 GTEST_DIR=/path/to/googletest cmake -G Xcode -DCMAKE_CXX_FLAGS=-DPRETTYSQL_JSON=1 ..
# either open project in Xcode and build/run
xcodebuild
```
