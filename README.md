# phonebook-api

A lightweight C++ REST API for managing phone contacts, built with Oat++ and backed by SQLite.

## Install and run

Run the commands from the repository root. The server build uses CMake 3.24 or newer and Oat++ 1.3.x.
CMake downloads `oatpp-swagger` on the first configure.

### macOS

If you need a C++ compiler, run `xcode-select --install`. Install Homebrew for the package commands below. CMake downloads `oatpp-sqlite` on the first configure and uses SQLite from the macOS SDK. [Apple Command Line Tools](https://developer.apple.com/documentation/xcode/installing-the-command-line-tools), [Homebrew](https://brew.sh/)

```sh
brew install cmake oatpp
cmake -S . -B build -DBUILD_TESTING=OFF
cmake --build build
./build/phonebook_api
```

### Linux (Ubuntu 24.04)

Install the build tools with `apt` and the Oat++ packages with vcpkg. [vcpkg setup](https://learn.microsoft.com/en-us/vcpkg/consume/classic-mode)

```sh
sudo apt update
sudo apt install build-essential cmake curl git pkg-config zip unzip
git clone --depth 1 https://github.com/microsoft/vcpkg.git ../vcpkg
../vcpkg/bootstrap-vcpkg.sh
../vcpkg/vcpkg install oatpp oatpp-sqlite
cmake -S . -B build -DBUILD_TESTING=OFF \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/../vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build
./build/phonebook_api
```

## Check the API

With the server running, check it from a second terminal.

```sh
curl -i http://127.0.0.1:8000/health
curl -i http://127.0.0.1:8000/contacts
```

`/health` returns `200 OK` with the body `OK`. `/contacts` returns a JSON array sorted by `id`.

Open [Swagger UI](http://127.0.0.1:8000/swagger/ui) to explore both routes and send requests with **Try it out**. The [OpenAPI document](http://127.0.0.1:8000/api-docs/oas-3.0.0.json) describes the `/contacts` response as an array of `ContactDto` objects.

On first start, `init_db.sql` initializes `phonebook.db` in the working directory with 69 fictional contacts. Restarts reuse that database. Press `Ctrl+C` in the server terminal to stop it.

## Run unit tests

Install GoogleTest for your platform.

- On macOS, run `brew install googletest`.
- On Ubuntu, run `../vcpkg/vcpkg install gtest` using the vcpkg setup above.

Reconfigure the `build` directory from the platform instructions and run CTest.
The unit tests call the health and contacts controllers directly.

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Continuous integration

GitHub Actions builds the project and runs CTest on macOS 15 and Ubuntu 24.04 for pull requests and pushes to `main`.
