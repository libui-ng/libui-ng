# Continuous Integration Arch

## docs
+ Build matrix: [Workflow syntax for GitHub Actions - GitHub Docs](https://docs.github.com/en/actions/learn-github-actions/workflow-syntax-for-github-actions#jobsjob_idstrategymatrix)
+ Supported runners: [About GitHub-hosted runners - GitHub Docs](https://docs.github.com/en/actions/using-github-hosted-runners/about-github-hosted-runners#supported-runners-and-hardware-resources)
+ Meson: [meson/Continuous-Integration.md at master · mesonbuild/meson](https://github.com/mesonbuild/meson/blob/master/docs/markdown/Continuous-Integration.md#github-actions)


## Build matrix

9 build =
+ Ubuntu=2
+ Windows-MSVC=4
+ Windows-Mingw=1
+ macOS=2

### Ubuntu
2 build =
- `[ubuntu-latest]`
- `[x64]`
- `[static, shared]`
- `[debug]`

### Windows

**MSVC**

4 build =
- `[windows-latest]`
- `[x86, x64]`
- `[static, shared]`
- `[debug]`

**Mingw**

1 build =
- `[windows-latest]`
- `[x64]`
- `[static]`
- `[release]`

### macOS
2 build =
- `[macos-11]`
- `[x64]`
- `[static, shared]`
- `[debug]`
