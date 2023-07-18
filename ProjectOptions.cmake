include(cmake/SystemLink.cmake)
include(cmake/LibFuzzer.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)


macro(libraycaster_supports_sanitizers)
  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)
    set(SUPPORTS_UBSAN ON)
  else()
    set(SUPPORTS_UBSAN OFF)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
    set(SUPPORTS_ASAN OFF)
  else()
    set(SUPPORTS_ASAN ON)
  endif()
endmacro()

macro(libraycaster_setup_options)
  option(libraycaster_ENABLE_HARDENING "Enable hardening" ON)
  option(libraycaster_ENABLE_COVERAGE "Enable coverage reporting" OFF)
  cmake_dependent_option(
    libraycaster_ENABLE_GLOBAL_HARDENING
    "Attempt to push hardening options to built dependencies"
    ON
    libraycaster_ENABLE_HARDENING
    OFF)

  libraycaster_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR libraycaster_PACKAGING_MAINTAINER_MODE)
    option(libraycaster_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(libraycaster_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(libraycaster_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(libraycaster_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(libraycaster_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(libraycaster_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(libraycaster_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(libraycaster_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(libraycaster_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(libraycaster_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(libraycaster_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(libraycaster_ENABLE_PCH "Enable precompiled headers" OFF)
    option(libraycaster_ENABLE_CACHE "Enable ccache" OFF)
  else()
    option(libraycaster_ENABLE_IPO "Enable IPO/LTO" ON)
    option(libraycaster_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
    option(libraycaster_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(libraycaster_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
    option(libraycaster_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(libraycaster_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
    option(libraycaster_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(libraycaster_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(libraycaster_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(libraycaster_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(libraycaster_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(libraycaster_ENABLE_PCH "Enable precompiled headers" OFF)
    option(libraycaster_ENABLE_CACHE "Enable ccache" ON)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      libraycaster_ENABLE_IPO
      libraycaster_WARNINGS_AS_ERRORS
      libraycaster_ENABLE_USER_LINKER
      libraycaster_ENABLE_SANITIZER_ADDRESS
      libraycaster_ENABLE_SANITIZER_LEAK
      libraycaster_ENABLE_SANITIZER_UNDEFINED
      libraycaster_ENABLE_SANITIZER_THREAD
      libraycaster_ENABLE_SANITIZER_MEMORY
      libraycaster_ENABLE_UNITY_BUILD
      libraycaster_ENABLE_CLANG_TIDY
      libraycaster_ENABLE_CPPCHECK
      libraycaster_ENABLE_COVERAGE
      libraycaster_ENABLE_PCH
      libraycaster_ENABLE_CACHE)
  endif()

  libraycaster_check_libfuzzer_support(LIBFUZZER_SUPPORTED)
  if(LIBFUZZER_SUPPORTED AND (libraycaster_ENABLE_SANITIZER_ADDRESS OR libraycaster_ENABLE_SANITIZER_THREAD OR libraycaster_ENABLE_SANITIZER_UNDEFINED))
    set(DEFAULT_FUZZER ON)
  else()
    set(DEFAULT_FUZZER OFF)
  endif()

  option(libraycaster_BUILD_FUZZ_TESTS "Enable fuzz testing executable" ${DEFAULT_FUZZER})

endmacro()

macro(libraycaster_global_options)
  if(libraycaster_ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    libraycaster_enable_ipo()
  endif()

  libraycaster_supports_sanitizers()

  if(libraycaster_ENABLE_HARDENING AND libraycaster_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR libraycaster_ENABLE_SANITIZER_UNDEFINED
       OR libraycaster_ENABLE_SANITIZER_ADDRESS
       OR libraycaster_ENABLE_SANITIZER_THREAD
       OR libraycaster_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    message("${libraycaster_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${libraycaster_ENABLE_SANITIZER_UNDEFINED}")
    libraycaster_enable_hardening(libraycaster_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()
endmacro()

macro(libraycaster_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(libraycaster_warnings INTERFACE)
  add_library(libraycaster_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  libraycaster_set_project_warnings(
    libraycaster_warnings
    ${libraycaster_WARNINGS_AS_ERRORS}
    ""
    ""
    ""
    "")

  if(libraycaster_ENABLE_USER_LINKER)
    include(cmake/Linker.cmake)
    configure_linker(libraycaster_options)
  endif()

  include(cmake/Sanitizers.cmake)
  libraycaster_enable_sanitizers(
    libraycaster_options
    ${libraycaster_ENABLE_SANITIZER_ADDRESS}
    ${libraycaster_ENABLE_SANITIZER_LEAK}
    ${libraycaster_ENABLE_SANITIZER_UNDEFINED}
    ${libraycaster_ENABLE_SANITIZER_THREAD}
    ${libraycaster_ENABLE_SANITIZER_MEMORY})

  set_target_properties(libraycaster_options PROPERTIES UNITY_BUILD ${libraycaster_ENABLE_UNITY_BUILD})

  if(libraycaster_ENABLE_PCH)
    target_precompile_headers(
      libraycaster_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(libraycaster_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    libraycaster_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(libraycaster_ENABLE_CLANG_TIDY)
    libraycaster_enable_clang_tidy(libraycaster_options ${libraycaster_WARNINGS_AS_ERRORS})
  endif()

  if(libraycaster_ENABLE_CPPCHECK)
    libraycaster_enable_cppcheck(${libraycaster_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(libraycaster_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    libraycaster_enable_coverage(libraycaster_options)
  endif()

  if(libraycaster_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(libraycaster_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()

  if(libraycaster_ENABLE_HARDENING AND NOT libraycaster_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR libraycaster_ENABLE_SANITIZER_UNDEFINED
       OR libraycaster_ENABLE_SANITIZER_ADDRESS
       OR libraycaster_ENABLE_SANITIZER_THREAD
       OR libraycaster_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    libraycaster_enable_hardening(libraycaster_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()

endmacro()
