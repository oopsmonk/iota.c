#[[
// Copyright 2020 IOTA Stiftung
// SPDX-License-Identifier: Apache-2.0
]]

add_library(blake2 STATIC)

target_sources(
  blake2
  PRIVATE "ref/blake2-impl.h"
    "ref/blake2b-ref.c"
    "ref/blake2bp-ref.c"
    "ref/blake2s-ref.c"
    "ref/blake2sp-ref.c"
    "ref/blake2xb-ref.c"
    "ref/blake2xs-ref.c"
  PUBLIC "ref/blake2.h")

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/ref/blake2.h" DESTINATION "${CMAKE_INSTALL_PREFIX}/include")

install(TARGETS blake2 DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
install(FILES "${PROJECT_SOURCE_DIR}/3rd-party/blake2/ref/blake2.h" DESTINATION "${CMAKE_INSTALL_PREFIX}/include")
