#### Get the submodule first

```
git submodule update --init
```

#### Linux & MacOS

```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

#### Windows

```
cmake -B build -A x64
cmake --build build --config Release -j
```
