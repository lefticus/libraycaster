add_library(libraycaster INTERFACE)

add_library(libraycaster::libraycaster ALIAS libraycaster)

target_include_directories(libraycaster ${WARNING_GUARD} INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                                                                  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)

target_compile_features(libraycaster INTERFACE cxx_std_20)


