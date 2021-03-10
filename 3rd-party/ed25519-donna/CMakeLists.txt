#[[
// Copyright 2020 IOTA Stiftung
// SPDX-License-Identifier: Apache-2.0
]]

find_package(OpenSSL REQUIRED)
message("OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
message("OpenSSL libraries: ${OPENSSL_LIBRARIES}")

add_library(ed25519_donna STATIC)

target_sources(
  ed25519_donna
  PRIVATE "curve25519-donna-32bit.h"
    "curve25519-donna-64bit.h"
    "curve25519-donna-helpers.h"
    "curve25519-donna-sse2.h"
    "ed25519.c"
    "ed25519-donna-32bit-sse2.h"
    "ed25519-donna-32bit-tables.h"
    "ed25519-donna-64bit-sse2.h"
    "ed25519-donna-64bit-tables.h"
    "ed25519-donna-64bit-x86-32bit.h"
    "ed25519-donna-64bit-x86.h"
    "ed25519-donna-basepoint-table.h"
    "ed25519-donna-batchverify.h"
    "ed25519-donna.h"
    "ed25519-donna-impl-base.h"
    "ed25519-donna-impl-sse2.h"
    "ed25519-donna-portable.h"
    "ed25519-donna-portable-identify.h"
    "ed25519-hash-custom.h"
    "ed25519-hash.h"
    "ed25519-randombytes-custom.h"
    "ed25519-randombytes.h"
    "modm-donna-32bit.h"
    "modm-donna-64bit.h"
  PUBLIC "ed25519.h")

# target_compile_definitions(ed25519_donna PRIVATE ED25519_REFHASH ED25519_CUSTOMRANDOM ED25519_CUSTOMHASH)
target_link_libraries(ed25519_donna PUBLIC ${OPENSSL_LIBRARIES})

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/ed25519.h" DESTINATION "${CMAKE_INSTALL_PREFIX}/include")

install(TARGETS blake2 DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
install(FILES "${PROJECT_SOURCE_DIR}/3rd-party/ed25519-donna/ed25519.h" DESTINATION "${CMAKE_INSTALL_PREFIX}/include")
