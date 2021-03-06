Run containers using Windows Host Compute Service
=================================================

`hostcompute_nspawn` utility is a proof-of-concept implementation of
a launcher for Windows Server containers. It is inspired by [systemd-nspawn](https://www.freedesktop.org/software/systemd/man/systemd-nspawn.html).

Utility uses Windows Host Compute Service, that is available in Windows Server 2016,
to mount specified directory into container and run specified executable from that
directory inside the isolated container environment.

Current limitations:

 - no base images handling (not easy to solve as Windows images are not free, Docker can be used to pull the initial image)
 - no console support (`stdout` is redirected into the file inside mounted directory)
 - no networking support (should be easy to add)

Download
--------

 - binary for Windows Server 2016: [hostcompute_nspawn.exe](https://github.com/akashche/hostcompute_nspawn/releases/download/v1.3/hostcompute_nspawn.exe) (0.6MB)
 - sha256: `a8bf6464380c59441ba9ddf383b017e553392bb17efca5a536ad1f49fbb4d744`

Usage example
-------------

Check that you run Windsows Server 2016 with latest updates:

    $ ver
    Microsoft Windows [Version 10.0.14393]

Download [Docker daemon and client](https://get.docker.com/builds/Windows/x86_64/docker-1.13.0.zip)
to pull the base image [microsoft/nanoserver](https://hub.docker.com/r/microsoft/nanoserver/):

    $ dockerd.exe -D
    time="2017-01-24T03:25:13.504516100-08:00" level=debug msg="Listener created for HTTP on npipe (//./pipe/docker_engine)"
    ... 

    $ docker.exe pull microsoft/nanoserver
    Using default tag: latest
    latest: Pulling from microsoft/nanoserver
    bce2fbc256ea: Pull complete
    3ac17e2e6106: Pull complete
    Digest: sha256:03fac8366a7d5b2b2e596a3c86401e55796ce705ed5d472ae8d794c6ecf2e9ca
    Status: Downloaded newer image for microsoft/nanoserver:latest

    $ docker.exe inspect microsoft/nanoserver 
    ...
    "GraphDriver": {
        "Name": "windowsfilter",
        "Data": {
            "dir": "C:\\ProgramData\\docker\\windowsfilter\\c889dc67d89b23186c943dadaa8ec4bbabd090a9dcfeffea99b61107f7433604"
        }
    },
    ...

Stop Docker daemon - it is needed only to pull the image.

Specify the parameters in `config.json` file ([example](https://github.com/akashche/hostcompute_nspawn/blob/master/resources/config.json)):

 - `parent_layer_directory`: use value from `dir` attribute from above output 
 - `process_directory`: host directory that will be mounted into containert
 - `process_executable`: relative path to executable to run inside the container
 - `process_arguments`: arguments to executable
 - `max_ram_mb`: max amount of RAM in megabytes allowed
 - `cpus_count`: max number of CPU cores allowed (it looks like HCS doesn't actually enforce it)
 - `cpu_affinity_hex`: CPU affinity mask, allows to bind containerized process to specific CPUs, must be specified in decimal, used as hex mask
 - `max_cpu_percent`: CPU maximum usage percent
 - `max_storage_iops`: maximum storage [IOPS](https://en.wikipedia.org/wiki/IOPS)
 - `max_storage_bandwidth_bytes_per_sec`: maximum storage bandwidth in bytes per second
 - `mapped_directory`: mount point for `process_directory` inside the container
 - `stdout_filename`: relative path to `stdout` file that will be written into `process_directory`

Process example:

    {
        "process_executable": "jdk/bin/java.exe",
        "process_arguments": ["-version"],
        ...
    }
        
Run `hostcompute_nspawn` specifying the config file:

    $ hostcompute_nspawn.exe -c config.json

It will print verbose output about container details to current terminal. Target process output will be written to `nspawn_out.txt` file inside the mounted directory:

    openjdk version "1.8.0_111-3-ojdkbuild"
    OpenJDK Runtime Environment (build 1.8.0_111-3-ojdkbuild-b15)
    OpenJDK 64-Bit Server VM (build 25.111-b15, mixed mode)

How to build
------------

[CMake](http://cmake.org/) is required for building.

[pkg-config](http://www.freedesktop.org/wiki/Software/pkg-config/) utility is used for dependency management.
Ready-to-use binary version of `pkg-config` can be obtained from [tools_windows_pkgconfig](https://github.com/staticlibs/tools_windows_pkgconfig) repository.

To build this utility using Visual Studio 2013 Express run the following commands using
Visual Studio development command prompt 
(`C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\Shortcuts\VS2013 x64 Cross Tools Command Prompt`):

    git clone --recursive https://github.com/akashche/hostcompute_nspawn.git
    cd hostcompute_nspawn
    mkdir build
    cd build
    cmake .. -G "Visual Studio 12 2013 Win64"
    cmake --build .

License information
-------------------

This project is released under the [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0).

Changelog
---------

**2017-05-06**

 * version `1.3`
 * CPU affinity support

**2017-03-31**

 * version  `1.2`
 * support for CPU load, IOPS and storage bandwidth
 * deps update

**2017-01-31**

 * version `1.1`
 * RAM and CPU cores limits support
 * correct rollback behaviour on errors

**2017-01-26**

 * initial public version
