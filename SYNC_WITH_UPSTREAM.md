# Syncing Your VS Layer With Upstream

This repository uses CMake (authoritative) but now also contains a manually maintained Visual Studio solution and project for convenience.

## Added Artifacts
- `KoalaBox.sln`
- `KoalaBox.vcxproj`
- `KoalaBox.vcxproj.filters`
- `Directory.Build.props`
- `SYNC_WITH_UPSTREAM.md` (this file)

## Purpose
These files are a thin wrapper to let you open and build the Windows portion of KoalaBox quickly in Visual Studio without invoking CMake/CPM. They do NOT download third‑party deps that the CMake build would fetch. Use the CMake build for full fidelity.

## When Upstream Source Lists Change
After pulling new changes:
1. Diff file lists:
   ```powershell
   git diff --name-status origin/master -- src include/koalabox
   ```
2. Add/remove `<ClCompile>` or `<ClInclude>` entries in `KoalaBox.vcxproj` (and matching filters in `.vcxproj.filters`).
3. Keep the project GUID unchanged.

## Recommended Branching
Maintain these VS artifacts on a branch like `vs-support` and periodically rebase onto `master` to minimize conflicts.

## Regenerating Automatically (Optional)
A future script could parse `CMakeLists.txt` `KOALABOX_HEADERS`/`KOALABOX_SOURCES` variables and rewrite the project file. Not implemented here to keep things simple.

## Cleanup
If these files fall behind or cause friction, they can be deleted without impacting the CMake workflow.

---
**Reminder:** For dependency correctness (cpr, spdlog, etc.), prefer the CMake build.
