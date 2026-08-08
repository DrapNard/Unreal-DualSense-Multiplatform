# CI and Releases

## Dependency checkout

CI checks out the direct `Dualsense-Multiplatform` submodule before compiling. Nested development submodules from the upstream library are intentionally not initialized because the Unreal plugin only needs the upstream `Source/` tree.

## Native CI

`native-ci.yml` builds and runs the standard-C++ smoke tests on GitHub-hosted Windows, Linux, and macOS runners. This validates the pinned upstream core integration and compiles the platform backend for each host OS.

## Unreal matrix

Unreal Engine itself is not installed on normal GitHub-hosted runners. `unreal-matrix.yml` therefore targets self-hosted runners labelled with both an OS label and an engine label such as `ue-5.8`.

The workflow covers UE 5.0 through 5.8 and runs `BuildPlugin`. Each runner should expose `UE_ROOT` and have the matching Unreal installation available.

## Upstream dependency updates

Dependabot is configured with the `gitsubmodule` ecosystem. It checks the submodule regularly and opens a PR when `Dualsense-Multiplatform/main` advances. The PR changes only the pinned gitlink and must pass the normal native/Unreal checks before merge.

For a manual update:

```bash
git submodule update --remote ThirdParty/Dualsense-Multiplatform
git add ThirdParty/Dualsense-Multiplatform
git commit -m "chore(deps): update Dualsense-Multiplatform"
```

## Versioning

The repository uses Conventional Commits and Release Please. Merging `feat:` and `fix:` changes into `main` updates a release PR. Merging that release PR creates the semantic version tag and GitHub release.

Release archives are created from a checkout with the submodule initialized, so users downloading a release ZIP receive the upstream source too and do not need Git to install the plugin.

## Wiki publishing

The `Wiki/` directory is the source of truth. Run the `Publish Wiki` workflow after enabling the GitHub Wiki for the repository.
