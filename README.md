# RMAP

RMAP is a cross-platform C++17 static library. It is composed of an always-present
core plus three optional service modules, all compiled into a **single** archive,
`RMAP.lib` (`libRMAP.a` on Unix-like platforms). Each service module can be
included or excluded at configure time; the output is always one library, never
several.

Modules:

| Module              | Option                    | Purpose                     |
|---------------------|---------------------------|-----------------------------|
| RMAP (core)         | *(always built)*          | Core RMAP implementation    |
| RMAP_Svc_SB         | `RMAP_BUILD_SVC_SB`       | SB service                  |
| RMAP_Svc_Rest       | `RMAP_BUILD_SVC_REST`     | REST service                |
| RMAP_Svc_SocketIO   | `RMAP_BUILD_SVC_SOCKETIO` | SocketIO service            |

## Repository layout

```
RMAP/
  CMakeLists.txt           Top-level: options, assembles the single RMAP.lib, third-party
                           dependencies, install/export
  CMakePresets.json        Ready-made configure/build presets
  cmake/                   Package-config template for find_package(RMAP)
  RMAP/                    Core module -> the RMAP.lib target
    CMakeLists.txt         Core project (STATIC library)
    include/RMAP/          Public headers (installed)
    src/                   Implementation + private headers (+ pch.h)
    third_party/           Dependencies fetched/built from git at build time (not committed):
      json/                  nlohmann_json (header-only)
      asio/                  asio (header-only, standalone)
      websocketpp/           websocketpp (header-only)
      boringssl/             BoringSSL (compiled from source into static libcrypto/libssl)
      curl/                  libcurl (compiled from source into a static libcurl)
      socketio/              socket.io-client-cpp (compiled from source into static sioclient_tls)
  RMAP_Svc_SB/             SB service module
    CMakeLists.txt         Service project (OBJECT library folded into RMAP.lib)
    include/RMAP_Svc_SB/   Public header
    src/                   Implementation + private headers (+ pch.h), organized into
                           base/ client/ model/ source/ subfolders
  RMAP_Svc_Rest/           REST service module (same structure)
  RMAP_Svc_SocketIO/       SocketIO service module (same structure)
```

Each module has its own `CMakeLists.txt`, so the generated Visual Studio solution
contains one project per module — `RMAP` (the core STATIC library, which *is*
`RMAP.lib`) plus one `OBJECT`-library project per enabled service. Every service's
compiled objects are folded into `RMAP.lib`, so the deliverable stays a single
archive. Each project's source is organized into Solution Explorer filters that
mirror its on-disk folder layout (e.g. the core's `src\core` / `include\RMAP`, or
SB's `src\base`, `src\client`, `src\model`, `src\source` / `include\RMAP_Svc_SB`).

## Building

CMake is the single source of truth. There are no committed Visual Studio
project files — CMake generates them (and Makefiles/Ninja/Xcode elsewhere) from
`CMakeLists.txt`, so a module excluded via an option simply won't appear in the
generated solution.

### Prerequisites

| Tool | Needed for | Notes |
|------|-----------|-------|
| CMake ≥ 3.20 | everything | |
| A C++17 compiler | everything | MSVC on Windows; GCC/Clang on Linux; AppleClang on macOS |
| **Go** | building BoringSSL | Build-time only, no Go runs at runtime. `winget install GoLang.Go` |
| **NASM** | building BoringSSL on Windows (x86/x64) | `winget install NASM.NASM`. Winget installs it under `%LOCALAPPDATA%\bin\NASM`, which is not on `PATH` by default — the build looks there and in `%ProgramFiles%\NASM` automatically. Not needed on Apple Silicon. |

The three header-only dependencies (nlohmann_json, asio, websocketpp) require no
extra tools. Three dependencies are compiled from source: BoringSSL, libcurl, and
socket.io-client-cpp. Only BoringSSL adds toolchain requirements (Go, plus NASM on
Windows) — libcurl and socket.io-client-cpp build with just the C/C++ compiler
(socket.io pulls its own git submodules at fetch time). libcurl on Windows uses
the OS's Schannel TLS so it needs no BoringSSL; socket.io-client-cpp's TLS path
has no Schannel backend, so it uses BoringSSL on every platform. Perl is **not**
required (modern BoringSSL uses Go for its assembly generation). To skip
BoringSSL's toolchain entirely, provide an external copy via
`RMAP_USE_SYSTEM_BORINGSSL=ON` (see Options); libcurl and socket.io-client-cpp can
likewise be supplied externally with `RMAP_USE_SYSTEM_CURL=ON` /
`RMAP_USE_SYSTEM_SIOCLIENT=ON`.

### Visual Studio (Windows)

Generate the solution and open it:

```powershell
cmake --preset vs2022
start build\RMAP.sln
```

Or build from the command line:

```powershell
cmake --build --preset vs2022-release
```

> The generated solution under `build\` is a build artifact and is regenerated
> by CMake. Don't hand-edit project settings in the IDE — they would be
> overwritten. Make permanent changes in `CMakeLists.txt`.

### Cross-platform (Ninja / Make / etc.)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build --prefix <install-dir>
```

### Excluding a module

Turn its option off and (re)generate — that's the only step, on every platform:

```powershell
cmake --preset vs2022 -DRMAP_BUILD_SVC_REST=OFF
```

## Options

| Option                       | Default | Effect                                                                                          |
|------------------------------|---------|-------------------------------------------------------------------------------------------------|
| `RMAP_BUILD_SVC_SB`          | `ON`    | Include the SB service module in `RMAP.lib`.                                                     |
| `RMAP_BUILD_SVC_REST`        | `ON`    | Include the REST service module in `RMAP.lib`.                                                   |
| `RMAP_BUILD_SVC_SOCKETIO`    | `ON`    | Include the SocketIO service module in `RMAP.lib`.                                               |
| `RMAP_USE_SYSTEM_JSON`       | `OFF`   | `ON` uses an external `nlohmann_json` via `find_package` instead of the git-fetched copy.        |
| `RMAP_USE_SYSTEM_ASIO`       | `OFF`   | `ON` uses an external `asio` via `find_package` (e.g. vcpkg/Conan) instead of the git-fetched copy. |
| `RMAP_USE_SYSTEM_WEBSOCKETPP`| `OFF`   | `ON` uses an external `websocketpp` via `find_package` instead of the git-fetched copy.          |
| `RMAP_USE_SYSTEM_BORINGSSL`  | `OFF`   | `ON` uses an external BoringSSL instead of building it from source (skips the Go/NASM toolchain). |
| `RMAP_USE_SYSTEM_CURL`       | `OFF`   | `ON` uses an external libcurl instead of building it from source.                                |
| `RMAP_USE_SYSTEM_SIOCLIENT`  | `OFF`   | `ON` uses an external socket.io-client-cpp instead of building it from source.                    |

## Dependencies

All dependencies are pulled from git at build time into `RMAP/third_party/` (not
committed). The three header-only libraries just add an include directory;
BoringSSL, libcurl, and socket.io-client-cpp are compiled from source. Each has a
`RMAP_USE_SYSTEM_*` option to consume an external copy instead (see Options).

- [nlohmann_json](https://github.com/nlohmann/json) 3.11.3 — header-only; used in
  the core's public API (`RMAP.h` exposes `nlohmann::ordered_json`). Fetched into
  `RMAP/third_party/json/`. Keep the version in step to avoid ABI skew across the
  public API. `RMAP_USE_SYSTEM_JSON=ON` consumes an external copy via `find_package`.
- [asio](https://github.com/chriskohlhoff/asio) 1.30.2 (tag `asio-1-30-2`) —
  header-only, used in **standalone** mode (RMAP defines `ASIO_STANDALONE`, so no
  Boost). Fetched into `RMAP/third_party/asio/`. Note: upstream asio ships no CMake
  package config, so `RMAP_USE_SYSTEM_ASIO=ON` requires asio from a package manager
  (vcpkg/Conan) that provides an `asio::asio` target.
- [websocketpp](https://github.com/zaphoyd/websocketpp) 0.8.2 — header-only,
  layered on asio. RMAP defines `_WEBSOCKETPP_CPP11_STL_` so it uses the C++11
  standard library instead of Boost. Fetched into `RMAP/third_party/websocketpp/`.
  Same package-manager caveat as asio for `RMAP_USE_SYSTEM_WEBSOCKETPP=ON`.
- [BoringSSL](https://github.com/google/boringssl) (`main`) — **compiled from
  source**, not header-only. Built into static `libcrypto`/`libssl` via its own
  CMake build (using the same generator as RMAP) and fetched into
  `RMAP/third_party/boringssl/`. Requires Go (and NASM on Windows x86/x64) — see
  Prerequisites. It is linked **PRIVATE** and kept out of RMAP's exported interface;
  because `RMAP.lib` is a static archive, a final executable that links RMAP must
  also link BoringSSL (see below). `RMAP_USE_SYSTEM_BORINGSSL=ON` uses an external
  copy instead.
- [libcurl](https://github.com/curl/curl) 8.9.1 (tag `curl-8_9_1`) — **compiled
  from source**, not header-only; version matches Sneeze
  (`C:\Dev\OMB\Sneeze\deps\curl.cmake`). Built into a static libcurl via its own
  CMake build (using the same generator as RMAP) and fetched into
  `RMAP/third_party/curl/`. Built minimal: static only, no `curl` executable, and
  LDAP/LDAPS, libidn2, libpsl, libssh2, zlib, brotli, and zstd all disabled. TLS
  backend is platform-specific: **Schannel** (the OS stack) on Windows, and
  **BoringSSL** (via curl's OpenSSL-compatible interface, reusing the BoringSSL
  built above) on Linux/macOS. Linked **PRIVATE** and kept out of RMAP's exported
  interface; `CURL_STATICLIB` is defined (build-only) for every translation unit
  that includes `<curl/curl.h>`. Because `RMAP.lib` is a static archive, a final
  executable that links RMAP must also link libcurl (see below).
  `RMAP_USE_SYSTEM_CURL=ON` uses an external copy instead.
- [socket.io-client-cpp](https://github.com/socketio/socket.io-client-cpp) —
  **compiled from source**, not header-only; used by the SocketIO service module
  (`Net.cpp` includes `<sio_client.h>`). Pinned to a specific **`master` commit**
  (`3b7be7e`, 2025-08-28) rather than the newest release tag `3.1.0`: that tag
  hard-caps its TLS target at C++11 (`set_property(... CXX_STANDARD 11)`), but
  modern BoringSSL's public C++ headers (`openssl/span.h`) require C++17; `master`
  relaxed that to a `cxx_std_11` *minimum*, so we raise the build to C++17
  (`CMAKE_CXX_STANDARD=17`) and compile against BoringSSL without patching. Built
  into a static **`sioclient_tls`** (its TLS variant, `-DSIO_TLS`) and fetched into
  `RMAP/third_party/socketio/`; it pulls its **own** git submodules
  (`lib/websocketpp`, `lib/rapidjson`, `lib/asio`), which are internal to its build
  and separate from RMAP's asio/websocketpp. Its TLS backend is **BoringSSL** on
  every platform (it has no Schannel path), wired via `find_package(OpenSSL)`
  pointed at the BoringSSL built above. On MSVC the build injects
  `WIN32_LEAN_AND_MEAN`/`NOCRYPT` (avoid the `<wincrypt.h>` `X509_NAME`/`PKCS7`
  macro collision with BoringSSL types), `NOMINMAX` (so `<windows.h>`'s `max()`
  macro doesn't mangle `std::numeric_limits<size_t>::max()` in BoringSSL's
  `span.h`), and `/EHsc /permissive- /Zc:__cplusplus`; its own tests/Catch2 are
  disabled (`BUILD_TESTING=OFF`). Its public `sio_*.h` headers are pimpl'd, so the
  SocketIO module only needs the sio include dir (no asio/websocketpp exposure).
  Linked **PRIVATE**; a final executable that links RMAP must also link
  `sioclient_tls` (see below). `RMAP_USE_SYSTEM_SIOCLIENT=ON` uses an external copy
  instead.

## Using RMAP from CMake

After installing, downstream projects consume the library through its exported
target:

```cmake
find_package(RMAP CONFIG REQUIRED)
target_link_libraries(my_target PRIVATE RMAP::RMAP)
```

If RMAP was built with `RMAP_USE_SYSTEM_JSON=ON`, its package config pulls in
`nlohmann_json` for you.

### Linking the compiled dependencies

BoringSSL, libcurl, and socket.io-client-cpp are linked **PRIVATE** into RMAP and
are not re-exported. Because `RMAP.lib` is a *static* archive, its private
dependencies are not resolved until the final link of an executable (or shared
library). A downstream target that links `RMAP::RMAP` must therefore also link:

- BoringSSL's `crypto`/`ssl`, plus its system libs (`ws2_32`/`crypt32` on Windows;
  `pthread`/`dl` on Linux).
- The static libcurl, plus its system libs. On Windows that is
  `ws2_32 wldap32 crypt32 normaliz` (curl uses Schannel there, so it does not pull
  BoringSSL); on Linux/macOS libcurl links the same BoringSSL noted above.
- The static `sioclient_tls`. Its TLS is backed by BoringSSL (already listed
  above) on every platform, and it uses the same Windows socket libs
  (`ws2_32`/`crypt32`).

asio and websocketpp are header-only and add no link requirement.

## License

See [LICENSE](LICENSE).
