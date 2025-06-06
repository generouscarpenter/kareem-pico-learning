# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/Users/kareemelnaggar/Documents/GitHub/kareem-pico-learning/SimplePicoMidiController/build/_deps/picoresponsiveanalogread-src")
  file(MAKE_DIRECTORY "/Users/kareemelnaggar/Documents/GitHub/kareem-pico-learning/SimplePicoMidiController/build/_deps/picoresponsiveanalogread-src")
endif()
file(MAKE_DIRECTORY
  "/Users/kareemelnaggar/Documents/GitHub/kareem-pico-learning/SimplePicoMidiController/build/_deps/picoresponsiveanalogread-build"
  "/Users/kareemelnaggar/Documents/GitHub/kareem-pico-learning/SimplePicoMidiController/build/_deps/picoresponsiveanalogread-subbuild/picoresponsiveanalogread-populate-prefix"
  "/Users/kareemelnaggar/Documents/GitHub/kareem-pico-learning/SimplePicoMidiController/build/_deps/picoresponsiveanalogread-subbuild/picoresponsiveanalogread-populate-prefix/tmp"
  "/Users/kareemelnaggar/Documents/GitHub/kareem-pico-learning/SimplePicoMidiController/build/_deps/picoresponsiveanalogread-subbuild/picoresponsiveanalogread-populate-prefix/src/picoresponsiveanalogread-populate-stamp"
  "/Users/kareemelnaggar/Documents/GitHub/kareem-pico-learning/SimplePicoMidiController/build/_deps/picoresponsiveanalogread-subbuild/picoresponsiveanalogread-populate-prefix/src"
  "/Users/kareemelnaggar/Documents/GitHub/kareem-pico-learning/SimplePicoMidiController/build/_deps/picoresponsiveanalogread-subbuild/picoresponsiveanalogread-populate-prefix/src/picoresponsiveanalogread-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/kareemelnaggar/Documents/GitHub/kareem-pico-learning/SimplePicoMidiController/build/_deps/picoresponsiveanalogread-subbuild/picoresponsiveanalogread-populate-prefix/src/picoresponsiveanalogread-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/kareemelnaggar/Documents/GitHub/kareem-pico-learning/SimplePicoMidiController/build/_deps/picoresponsiveanalogread-subbuild/picoresponsiveanalogread-populate-prefix/src/picoresponsiveanalogread-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
