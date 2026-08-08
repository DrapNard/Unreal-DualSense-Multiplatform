# CI and Releases

## Native CI

`native-ci.yml` builds and runs the standard-C++ smoke tests on GitHub-hosted Windows, Linux, and macOS runners. This validates the vendored core integration and compiles the platform backend for each host OS.

## Unreal matrix

Unreal Engine itself is not installed on normal GitHub-hosted runners. `unreal-matrix.yml` therefore targets self-hosted runners labelled with both an OS label and an engine label such as `ue-5.8`.

The workflow covers UE 5.0 through 5.8 and runs `BuildPlugin`.

Each runner should expose `UE_ROOT` and have the matching Unreal installation available.

## Versioning

The repository uses Conventional Commits and Release Please. Merging `feat:` and `fix:` changes into `main` updates a release PR. Merging that release PR creates the semantic version tag and GitHub release.

## Wiki publishing

The `Wiki/` directory is the source of truth. Run the `Publish Wiki` workflow after enabling the GitHub Wiki for the repository.
