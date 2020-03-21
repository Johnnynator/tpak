# TPAK Extract
![CI](https://github.com/Johnnynator/tpak/workflows/CI/badge.svg)

Tool to extract tpak archive format (only version 7 so far)

## How to install

### Download ci artifacts

You can download one of the CI Artifacts.
To do that go to [CI](https://github.com/Johnnynator/tpak/actions?query=workflow%3ACI) and choose one of the runs.

### From Source
Dependencies: `meson, ninja, zlib (optional)`, C compiler (+ libc that implements some POSIX C (E.g. MinGW on Windows))

```bash
git clone https://github.com/Johnnynator/tpak
cd tpak
meson build
ninja -C build
```

## Unlicense

        This is free and unencumbered software released into the public domain.

        Anyone is free to copy, modify, publish, use, compile, sell, or
        distribute this software, either in source code form or as a compiled
        binary, for any purpose, commercial or non-commercial, and by any
        means.

        In jurisdictions that recognize copyright laws, the author or authors
        of this software dedicate any and all copyright interest in the
        software to the public domain. We make this dedication for the benefit
        of the public at large and to the detriment of our heirs and
        successors. We intend this dedication to be an overt act of
        relinquishment in perpetuity of all present and future rights to this
        software under copyright law.

        THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
        EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
        MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
        IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
        OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
        ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
        OTHER DEALINGS IN THE SOFTWARE.

        For more information, please refer to <http://unlicense.org/>


## References

[clutch.bms](https://aluigi.altervista.org/bms/clutch.bms)

[star-conflict-tpak-format.md](https://gist.github.com/deluvas/8f524b99f1bcd161ba2f) WARNING, contains wrong information
