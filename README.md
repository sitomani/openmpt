# README

This fork of libopenmpt is created to build libopenmpt as universal framework for iOS platform.

To build for iOS, you will need a computer running macOS and xcode SDK installed. In addition, you'll need to have [genie](https://github.com/bkaradzic/GENie) installed to generate the Xcode project file.

To build the iOS framework, run the following commands in the root directory of this repository:

- `./iOS_genproject.sh` - this will create the Xcode project using genie
- `./iOS_build.sh` - this will build the libopenmpt library for ios simulator and real device and create an universal framework that can be used with iOS apps.

A very simple example project making use of the library can be found at the [4champ repository](https://github.com/sitomani/4champ/tree/master/SamplePlayer)

# OpenMPT and libopenmpt

This repository contains OpenMPT, a free Windows/Wine-based
[tracker](https://en.wikipedia.org/wiki/Music_tracker) and libopenmpt,
a library to render tracker music (MOD, XM, S3M, IT MPTM and dozens of other
legacy formats) to a PCM audio stream. libopenmpt is directly based on OpenMPT,
offering the same playback quality and format support, and development of the
two happens in parallel.

## License

The OpenMPT/libopenmpt project is distributed under the _BSD-3-Clause_ License.
See [LICENSE](LICENSE) for the full license text.

Files below the `include/` (external projects) and `contrib/` (related assets
not directly considered integral part of the project) folders may be subject to
other licenses. See the respective subfolders for license information. These
folders are not distributed in all source packages, and in particular they are
not distributed in the Autotools packages.

## How to compile

### OpenMPT

- Supported Visual Studio versions:

  - Visual Studio 2017 and 2019 Community/Professional/Enterprise

    To compile the project, open `build/vsVERSIONwin7/OpenMPT.sln` (VERSION
    being 2017 or 2019) and hit the compile button. Other target systems can
    be found in the `vs2017*` and `vs2019*` sibling folders.

- OpenMPT requires the compile host system to be 64bit x86-64.

- The Windows 8.1 SDK is required to build OpenMPT with Visual Studio 2017
  (this is included with Visual Studio, however may need to be selected
  explicitly during setup).

- Microsoft Foundation Classes (MFC) are required to build OpenMPT.

### libopenmpt and openmpt123

See [Dependencies](libopenmpt/dox/dependencies.md) and
[Getting Started](libopenmpt/dox/gettingstarted.md).

## Contributing to OpenMPT/libopenmpt

See [contributing](doc/contributing.md).
