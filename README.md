Launch process in container using Windows Host Compute Service
==============================================================

`hostcompute_nspawn` utility is a proof-of-concept implementation of
a native Windows container launcher.

It uses Windows Host Compute Service to mount specified directory into
container and run specified executable from that directory in isolated container environment.

Current limitations:

 - no base images handling (not easy to solve as Windows images are not free, Docker can be used to pull the initial image)
 - no console support (stdout is redirected to file in mounted directory)
 - not networking support

Usage example
-------------

TODO

How to build
------------

[CMake](http://cmake.org/) is required for building.

[pkg-config](http://www.freedesktop.org/wiki/Software/pkg-config/) utility is used for dependency management.
Ready-to-use binary version of `pkg-config` can be obtained from [tools_windows_pkgconfig](https://github.com/staticlibs/tools_windows_pkgconfig) repository.

To build the library on Windows using Visual Studio 2013 Express run the following commands using
Visual Studio development command prompt 
(`C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\Shortcuts\VS2013 x64 Cross Tools Command Prompt`):

    git clone --recursive https://github.com/akashche/hostcompute_nspawn.git
    cd hostcompute_nspawn
    mkdir build
    cd build
    cmake ..
    msbuild hostcompute_nspawn.sln

License information
-------------------

This project is released under the [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0).

Changelog
---------

**2017-01-26**

 * initial public version
