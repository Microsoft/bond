include (Compiler)

set (BOND_GBC_PATH_DESCRIPTION
     "Optional path to the gbc executable to use. If set, this gbc will be used when generating code from .bond files. If not set, then gbc will be built (and the Haskell toolchain will need to be present on the machine) and the gbc tests will be run.")

find_program (BOND_GBC_PATH "gbc"
    HINTS ENV BOND_GBC_PATH
    DOC ${BOND_GBC_PATH_DESCRIPTION}
    # We only want to look for gbc if the user explicitly set the
    # environment variable BOND_GBC_PATH. (If they set the CMake variable
    # BOND_GBC_PATH explicitly, find_program won't change it, allowing it to
    # take precedence.) We don't want CMake to go looking elsewhere, so we
    # turn off all its default search paths.
    NO_DEFAULT_PATH)

if (BOND_GBC_PATH)
    set (GBC_EXECUTABLE ${BOND_GBC_PATH})
    message (STATUS "Existing GBC executable found: '${GBC_EXECUTABLE}'")
endif()

if (MSVC)
    # MSVC needs this because of how template-heavy our code is.
    add_compile_options (/bigobj)
    # inject disabling of some MSVC warnings for versions prior to MSVC14
    if (MSVC_VERSION LESS 1900)
        add_compile_options (/FIbond/core/warning.h)
    endif()
    # turn up warning level
    add_compile_options (/W4 /WX /sdl)
    # Enable SDL recommended warnings that aren't enabled by /W4
    # 4242: 'identifier': conversion from 'type1' to 'type2', possible loss of data
    # 4302: 'conversion': truncation from 'type1' to 'type2'
    add_compile_options (/we4242 /we4302)
    # use secure CRT functions via template overloads to avoid polluting
    # Bond with MSVC CRT-specific code too much. More details at
    # https://msdn.microsoft.com/en-us/library/ms175759.aspx
    add_definitions (-D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1 -D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT=1)

    # Enable standards-conformance mode for MSVC compilers that support this
    # flag (Visual C++ 2017 and later).
    #
    # Our minimum required version of CMake doesn't have GREATER_EQUAL, so
    # we invert a less-than comparison instead. CMake 3.7 added
    # GREATER_EQUAL.
    if (NOT (MSVC_VERSION LESS 1910))
      add_compile_options (/permissive-)
    endif()

    set (Boost_USE_STATIC_LIBS ON)
endif (MSVC)

if (WIN32)
    find_package (WindowsSDK)

    # If C# has been built we will also run C# compatibility tests
    find_program (BOND_CSHARP_COMPAT_TEST Bond.CompatibilityTest.exe
        PATH_SUFFIXES net45
        NO_DEFAULT_PATH
        PATHS
            "${CMAKE_CURRENT_SOURCE_DIR}/cs/test/compat/core/bin/debug"
            "${CMAKE_CURRENT_SOURCE_DIR}/cs/test/compat/core/bin/retail")

    find_program (BOND_CSHARP_COMM_COMPAT_SERVER CommCompatServer.exe
        PATH_SUFFIXES net45
        NO_DEFAULT_PATH
        PATHS
            "${CMAKE_CURRENT_SOURCE_DIR}/cs/test/compat/comm/server/bin/debug"
            "${CMAKE_CURRENT_SOURCE_DIR}/cs/test/compat/comm/server/bin/retail")

    find_program (BOND_CSHARP_COMM_COMPAT_CLIENT CommCompatClient.exe
        PATH_SUFFIXES net45
        NO_DEFAULT_PATH
        PATHS
            "${CMAKE_CURRENT_SOURCE_DIR}/cs/test/compat/comm/client/bin/debug"
            "${CMAKE_CURRENT_SOURCE_DIR}/cs/test/compat/comm/client/bin/retail")

    find_program (BOND_CSHARP_GRPC_COMPAT_SERVER GrpcCompatServer.exe
        PATH_SUFFIXES net45
        NO_DEFAULT_PATH
        PATHS
            "${CMAKE_CURRENT_SOURCE_DIR}/cs/test/compat/grpc/server/bin/debug"
            "${CMAKE_CURRENT_SOURCE_DIR}/cs/test/compat/grpc/server/bin/retail")

    find_program (BOND_CSHARP_GRPC_COMPAT_CLIENT GrpcCompatClient.exe
        PATH_SUFFIXES net45
        NO_DEFAULT_PATH
        PATHS
            "${CMAKE_CURRENT_SOURCE_DIR}/cs/test/compat/grpc/client/bin/debug"
            "${CMAKE_CURRENT_SOURCE_DIR}/cs/test/compat/grpc/client/bin/retail")
endif()

# find Java libraries and programs
find_file (BOND_JAVA_CORE bond-5.0.0.jar
    NO_DEFAULT_PATH
    PATHS "${CMAKE_CURRENT_SOURCE_DIR}/java/core/build/libs")

find_file (BOND_JAVA_COMPAT_TEST compat-1.0.jar
    NO_DEFAULT_PATH
    PATHS "${CMAKE_CURRENT_SOURCE_DIR}/java/compat/build/libs")

# find python interpreter, library and boost python library.
# to specify a different version, invoke cmake with:
# -DPYTHON_EXECUTABLE=/path/to/python
# -DPYTHON_LIBRARY=/path/to/libpython.so
# -DBoost_PYTHON_LIBRARY_RELEASE=/path/to/libboost-python.so
# (or Boost_PYTHON_LIBRARY_DEBUG if CMAKE_BUILD_TYPE=Debug)
# and optionally with:
# -DPython_ADDITIONAL_VERSIONS=Major.Minor
# if your python version is not implicitly supported by cmake
find_package (PythonInterp 2.7)
if (APPLE)
    if (NOT PYTHON_LIBRARY)
        # find_package (PythonLibs) doesn't find libpython in /usr/local/Cellar,
        # so if PYTHON_LIBRARY wasn't passed as a cmake -D, we have to set it
        # here.
        execute_process (
            COMMAND "python-config" "--prefix"
            OUTPUT_VARIABLE PYTHON_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        string (CONCAT PYTHON_LIBRARY ${PYTHON_PREFIX} "/lib/libpython2.7.dylib")
    endif()
endif()
find_package (PythonLibs 2.7)

find_package (Boost 1.53.0
    OPTIONAL_COMPONENTS
        chrono
        date_time
        thread
        system
        unit_test_framework
        python)

message(STATUS "Boost Python Library: ${Boost_PYTHON_LIBRARY}")

# Make sure AppVeyor CI runs fail when unit test dependencies are not found
if (DEFINED ENV{APPVEYOR} AND ("$ENV{BOND_BUILD}" STREQUAL "C++"))
    if (NOT Boost_UNIT_TEST_FRAMEWORK_FOUND)
        message(FATAL_ERROR "Boost unit_test_framework not found")
    endif()
endif()

# disable Boost auto-linking
add_definitions (-DBOOST_ALL_NO_LIB)

# VS2015U2 fixed a bug with atomics and emits a warning without this definition.
add_definitions (-D_ENABLE_ATOMIC_ALIGNMENT_FIX)

cxx_add_compile_options(Clang
    -fPIC
    -fstrict-aliasing
    --std=c++11
    -Wall
    -Werror
    -Wno-null-dereference
    -Wno-unknown-warning-option
    -Wno-unused-local-typedefs)

cxx_add_compile_options(AppleClang
    -fPIC
    -fstrict-aliasing
    --std=c++11
    -Wall
    -Werror
    -Wno-null-dereference
    -Wno-unknown-warning-option
    -Wno-unused-local-typedefs)

cxx_add_compile_options(GNU
    -fPIC
    -fstrict-aliasing
    --std=c++11
    -Wall
    -Werror
    -Wno-unknown-warning-option
    -Wno-unused-local-typedefs)

include_directories (
    ${BOND_INCLUDE}
    ${BOND_GENERATED}
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/rapidjson/include)

set (BOND_LIBRARIES_ONLY
    "FALSE"
    CACHE BOOL "If TRUE, then only build the Bond library files, skipping any tools. gbc will still be built if it cannot be found, however, as gbc is needed to build the libraries.")

set (BOND_LIBRARIES_INSTALL_CPP
    "FALSE"
    CACHE BOOL "If TRUE, the generated .cpp files for the Bond libraries will be installed under src/ as part of the INSTALL target.")

set (BOND_ENABLE_COMM
    "FALSE"
    CACHE BOOL "If FALSE, then do not build Comm")

set (BOND_SKIP_GBC_TESTS
    "FALSE"
    CACHE BOOL "If TRUE, then skip gbc tests")

set (BOND_SKIP_CORE_TESTS
    "FALSE"
    CACHE BOOL "If TRUE, then skip Bond Core tests and examples")

set (BOND_SKIP_COMPAT_TESTS
    "FALSE"
    CACHE BOOL "If TRUE, then skip Bond Compat tests")

if (((BOND_ENABLE_COMM) OR (BOND_ENABLE_GRPC)) AND ((CXX_STANDARD LESS 11) OR (MSVC_VERSION LESS 1800)))
    message(FATAL_ERROR "BOND_ENABLE_COMM and/or BOND_ENABLE_GRPC is TRUE but compiler specified does not support C++11 standard")
endif()
