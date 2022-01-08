# Continuous Integration Arch

## Github Action

### docs
+ Build matrix: [Workflow syntax for GitHub Actions - GitHub Docs](https://docs.github.com/en/actions/learn-github-actions/workflow-syntax-for-github-actions#jobsjob_idstrategymatrix)
+ Supported runners: [About GitHub-hosted runners - GitHub Docs](https://docs.github.com/en/actions/using-github-hosted-runners/about-github-hosted-runners#supported-runners-and-hardware-resources)
+ Meson: [meson/Continuous-Integration.md at master · mesonbuild/meson](https://github.com/mesonbuild/meson/blob/master/docs/markdown/Continuous-Integration.md#github-actions)


## Old pipeline

### Appveyor

**6 build jobs**: [Current build - libui - AppVeyor](https://ci.appveyor.com/project/andlabs/libui)

| No | Arch | libtype | compiler |
|----|------|---------|----------|
| 1  | x86  | shared  | msvc2013 |
| 2  | x86  | static  | msvc2013 |
| 3  | x86  | static  | mingw    |
| 4  | x64  | shared  | msvc2013 |
| 5  | x64  | static  | msvc2013 |
| 6  | x64  | static  | mingw    |


### Azure pipelines

**14 build jobs**: [Pipelines - Runs for andlabs.libui](https://dev.azure.com/andlabs/libui/_build?definitionId=1)

| No | platform     | Arch | libtype | compiler |
|----|--------------|------|---------|----------|
| 1  | ubuntu-16.04 | x86  | shared  | gcc      |
| 2  | ubuntu-16.04 | x86  | static  | gcc      |
| 3  | ubuntu-16.04 | x64  | shared  | gcc      |
| 4  | ubuntu-16.04 | x64  | static  | gcc      |
| 5  | windows      | x86  | shared  | msvc2015 |
| 6  | windows      | x86  | static  | msvc2015 |
| 7  | windows      | x64  | shared  | msvc2015 |
| 8  | windows      | x64  | static  | msvc2015 |
| 9  | windows      | x86  | shared  | msvc2017 |
| 10 | windows      | x86  | static  | msvc2017 |
| 11 | windows      | x64  | shared  | msvc2017 |
| 12 | windows      | x64  | static  | msvc2017 |
| 13 | macos-10.13  | x64  | shared  | clang    |
| 14 | macos-10.13  | x64  | static  | clang    |
