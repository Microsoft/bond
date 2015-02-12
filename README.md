[![Build Status](https://travis-ci.org/Microsoft/bond.svg?branch=master)](https://travis-ci.org/Microsoft/bond)
[![NuGet](https://img.shields.io/nuget/v/Bond.CSharp.svg?style=flat)](https://www.nuget.org/packages/Bond.CSharp/)

Bond
====

Bond is an open source, cross-platform framework for working with schematized 
data. It supports cross-language serialization/deserialization and powerful 
generic mechanisms for efficiently manipulating data. Bond is broadly used at 
Microsoft in high scale services.

Bond is published on GitHub at [https://github.com/Microsoft/bond/](https://github.com/Microsoft/bond/).

For details, see the User's Manuals for [C++](https://Microsoft.github.io/bond/manual/bond_cpp.html), [C#](https://Microsoft.github.io/bond/manual/bond_cs.html) and [Python](https://Microsoft.github.io/bond/manual/bond_py.html).

For a discussion how Bond compares to similar frameworks see [Why Bond](https://Microsoft.github.io/bond/why_bond.html).

Dependencies
------------

The Bond repository uses Git submodules and should be cloned with the 
`--recursive` flag:

    git clone --recursive https://github.com/Microsoft/bond.git

In order to build Bond you will need CMake (2.8.12+), Haskell (ghc 7.4+ and 
cabal-install 1.18+) and Boost (1.54+). The core Bond C++ library can be used 
with C++03 compilers, although Python support, unit tests and various examples 
require some C++11 features.

Following are specific instructions for building on various platforms.

### Linux

Bond can be built with Clang (3.4+) or GNU C++ (4.7+). We recommend the latest
version of Clang as it's much faster with template-heavy code like Bond.

Run the following commands to install the minimal set of packages needed to 
build the core Bond library on Ubuntu 14.04:

    sudo apt-get install \
        clang \
        cmake \
        zlib1g-dev \
        ghc \
        cabal-install \
        libboost-dev \
        libboost-thread-dev

    cabal update
    cabal install cabal-install

In the root `bond` directory run:

    mkdir build
    cd build
    cmake ..
    make
    sudo make install

The `build` directory is just an example. Any directory can be used for build 
destination.

In order to build all the C++ and Python tests and examples, a few more 
packages are needed:

    sudo apt-get install \
        python2.7-dev \
        libboost-date-time-dev \
        libboost-test-dev \
        libboost-python-dev

Running the following command in the build directory will build and execute all 
the tests and examples:

    make --jobs 8 check

(unit tests are large so you may want to run 4-8 build jobs in parallel, 
assuming you have enough memory)

### OS X

Install XCode and then run the following command to install required packages 
using Homebrew ([http://brew.sh/](http://brew.sh/)):

    brew install \
        cmake \
        ghc \
        cabal-install \
        boost \
        boost-python

(boost-python is optional and only needed for Python support)

Bond can be built on OS X using either standard *nix makefiles or XCode. In 
order to generate and build makefiles, in the root `bond` directory run:

    mkdir build
    cd build
    cmake ..
    make
    sudo make install

Alternatively you can generate XCode project by passing `-G Xcode` option to 
cmake:

    cmake -G Xcode ..

You can build and run unit tests by building the `check` target in XCode or by 
running make in the build directory:

    make --jobs 8 check

Note that if you are using Homebrew's Python, you'll need to build
boost-python from source:

    brew install --build-from-source boost-python

and tell cmake the location of Homebrew's libpython by setting `PYTHON_LIBRARY` 
variable, e.g.:

    cmake .. -DPYTHON_LIBRARY=/usr/local/Cellar/python/2.7.9/Frameworks/Python.framework/Versions/2.7/lib/libpython2.7.dylib

### Windows

[![Build Status](https://ci.appveyor.com/api/projects/status/github/Microsoft/bond?svg=true&branch=master)](https://ci.appveyor.com/project/sapek/bond/branch/master)

Install the following tools:

- Visual Studio 2013
- CMake ([http://www.cmake.org/download/](http://www.cmake.org/download/))
- Haskell Platform ([http://haskell.org/platform/](http://haskell.org/platform/))

If behind a proxy, set environment variable `HTTP_PROXY=http://proxy:port`.

Now you are ready to build the C# version of Bond. Open the solution file 
`cs\cs.sln` in Visual Studio 2013 and build as usual. The C# unit tests can 
also be run from within the solution.

The C++ and Python versions of Bond additionally require:

- Boost 1.54+ ([http://www.boost.org/users/download/](http://www.boost.org/users/download/))
- Python 2.7 ([https://www.python.org/downloads/](https://www.python.org/downloads/))

You may need to set the environment variables `BOOST_ROOT` and `BOOST_LIBRARYDIR` 
to specify where Boost and its pre-built libraries for your environment can be 
found. The core Bond library and most examples only require Boost headers. The 
pre-built libraries are only needed for unit tests and Python support. If Boost
or Python libraries are not found on the system then some tests and examples will 
not be built.

In order to configure C++/Python project run `cmake-gui`, select the root of 
this repository as the source code directory and some other directory as the 
target for generated project files and build binaries. Now press Generate and 
select desired target build environment. This configuration step has to be 
performed only once. From now on you can use the generated solution `bond.sln`.

IMPORTANT: Bond unit tests are very large. If you are building using the Visual 
Studio toolchain you have to select 64-bit tools by setting the following 
environment variable:

For Visual Studio 2012:

    set _IsNativeEnvironment=true

For Visual Studio 2013:

    set PreferredToolArchitecture=x64

In order to run unit tests on the Debug build, execute the following command in 
the build directory:

    ctest -C Debug
