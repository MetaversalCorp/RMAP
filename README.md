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
  CMakeLists.txt           Single source of truth for the build (all platforms)
  CMakePresets.json        Ready-made configure/build presets
  cmake/                   Package-config template for find_package(RMAP)
  RMAP/                    Core module
    include/RMAP/          Public headers (installed)
    src/                   Implementation + private headers (+ pch.h)
    third_party/json/      nlohmann_json, fetched from git at build time (not committed)
  RMAP_Svc_SB/             SB service module (include/ + src/)
  RMAP_Svc_Rest/           REST service module (include/ + src/)
  RMAP_Svc_SocketIO/       SocketIO service module (include/ + src/)
```

## Building

CMake is the single source of truth. There are no committed Visual Studio
project files — CMake generates them (and Makefiles/Ninja/Xcode elsewhere) from
`CMakeLists.txt`, so a module excluded via an option simply won't appear in the
generated solution.

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

| Option                    | Default | Effect                                                                                     |
|---------------------------|---------|--------------------------------------------------------------------------------------------|
| `RMAP_BUILD_SVC_SB`       | `ON`    | Include the SB service module in `RMAP.lib`.                                                |
| `RMAP_BUILD_SVC_REST`     | `ON`    | Include the REST service module in `RMAP.lib`.                                              |
| `RMAP_BUILD_SVC_SOCKETIO` | `ON`    | Include the SocketIO service module in `RMAP.lib`.                                          |
| `RMAP_USE_SYSTEM_JSON`    | `OFF`   | `ON` uses an external `nlohmann_json` via `find_package` instead of the git-fetched copy.   |

## Dependencies

- [nlohmann_json](https://github.com/nlohmann/json) 3.11.3 — used in the core's
  public API (`RMAP.h` exposes `nlohmann::ordered_json`). By default it is
  fetched from git at build time into `RMAP/third_party/json/` (header-only, not
  committed). Set `RMAP_USE_SYSTEM_JSON=ON` to consume an external copy via
  `find_package` instead. Keep the version in step to avoid ABI skew across the
  public API.

## Using RMAP from CMake

After installing, downstream projects consume the library through its exported
target:

```cmake
find_package(RMAP CONFIG REQUIRED)
target_link_libraries(my_target PRIVATE RMAP::RMAP)
```

If RMAP was built with `RMAP_USE_SYSTEM_JSON=ON`, its package config pulls in
`nlohmann_json` for you.

## License

See [LICENSE](LICENSE).
