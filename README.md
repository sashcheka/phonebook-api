# phonebook-api

A lightweight C++ REST API for managing phone contacts, built with Oat++. The current endpoint is `GET /health`. Contact storage will use SQLite.

## Install and run

Run the commands from the repository root. The server build uses CMake 3.24 or newer and Oat++ 1.3.x.

### macOS

If you need a C++ compiler, run `xcode-select --install`. Install Homebrew for the package commands below. [Apple Command Line Tools](https://developer.apple.com/documentation/xcode/installing-the-command-line-tools), [Homebrew](https://brew.sh/)

```sh
brew install cmake oatpp
cmake -S . -B build -DBUILD_TESTING=OFF
cmake --build build
./build/phonebook_api
```

### Linux (Ubuntu 24.04)

Install the build tools with `apt` and Oat++ with vcpkg. [vcpkg setup](https://learn.microsoft.com/en-us/vcpkg/consume/classic-mode)

```sh
sudo apt update
sudo apt install build-essential cmake curl git pkg-config zip unzip
git clone --depth 1 https://github.com/microsoft/vcpkg.git ../vcpkg
../vcpkg/bootstrap-vcpkg.sh
../vcpkg/vcpkg install oatpp
cmake -S . -B build -DBUILD_TESTING=OFF \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/../vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build
./build/phonebook_api
```

## Test the health endpoint

With the server running, check it from a second terminal.

```sh
curl -i http://127.0.0.1:8000/health
```

Expect `200 OK` with the body `OK`. Press `Ctrl+C` in the server terminal to stop it.

## Run unit tests

Install GoogleTest for your platform.

- On macOS, run `brew install googletest`.
- On Ubuntu, run `../vcpkg/vcpkg install gtest` using the vcpkg setup above.

Reconfigure the `build` directory from the platform instructions and run CTest.
The unit tests call the health controller directly.

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```
