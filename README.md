# phonebook-api

A lightweight C++ REST API for managing phone contacts, built with Oat++ and backed by SQLite.

## Install and run

Run the commands from the repository root. The server build uses a C++17 compiler, CMake 3.24 or newer, Oat++ 1.3.x, and SQLite 3.35 or newer.
CMake downloads `oatpp-swagger` and `nlohmann/json` on the first configure.

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
curl -i http://127.0.0.1:8000/contacts/1
curl -i -X POST http://127.0.0.1:8000/contacts \
  -H 'Content-Type: application/json' \
  -d '{"name":"Alex Quinn","phone":"+1-202-555-0180","address":"103 Example Avenue"}'
curl -i -X PUT http://127.0.0.1:8000/contacts/17 \
  -H 'Content-Type: application/json' \
  -d '{"name":"Maya Carter","phone":"00456","address":"117 Example Avenue"}'
```

`/health` returns `200 OK` with the body `OK`. `/contacts` returns a JSON array sorted by `id`.
`/contacts/{id}` returns one contact with `200 OK`. The ID must be a positive 64-bit integer. Invalid IDs receive `400 Bad Request` (`invalid_contact_id`), and missing contacts receive `404 Not Found` (`contact_not_found`). These errors include a JSON `code` and `message`.

`POST /contacts` requires non-blank strings for `name`, `phone`, and `address`. SQLite assigns the ID. A successful request returns `201 Created`, the new contact, and a `Location` header pointing to it. Invalid JSON or contact data returns `400`, and a storage failure returns `500` with a generic JSON message.

`PUT /contacts/{id}` replaces the name, phone, and address of an existing contact. Send all three non-blank strings. The response contains the updated contact with the same ID and status `200 OK`. Invalid input returns `400`, a missing contact returns `404`, and a storage failure returns `500`.

Open [Swagger UI](http://127.0.0.1:8000/swagger/ui) to explore the routes and send requests with **Try it out**. The [OpenAPI document](http://127.0.0.1:8000/api-docs/oas-3.0.0.json) includes the creation and update schemas.

On first start, `init_db.sql` initializes `phonebook.db` in the working directory with 69 fictional contacts. Restarts reuse that database. Press `Ctrl+C` in the server terminal to stop it.

## Run unit tests

Install GoogleTest for your platform.

- On macOS, run `brew install googletest`.
- On Ubuntu, run `../vcpkg/vcpkg install gtest` using the vcpkg setup above.

Reconfigure the `build` directory from the platform instructions and run CTest.
The unit tests call the health and contacts controllers directly. The contact controller tests use a stub service. The contact service tests verify validation, creation, and updates through a stub repository.

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Continuous integration

GitHub Actions builds the project and runs CTest on macOS 15 and Ubuntu 24.04 for pull requests and pushes to `main`.
