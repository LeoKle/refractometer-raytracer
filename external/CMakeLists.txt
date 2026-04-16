add_subdirectory(googletest)

# OPENQMC: https://openqmc.org/setup/
set(OPENQMC_ARCH_TYPE AVX CACHE STRING "" FORCE)
set(OPENQMC_ENABLE_BINARY ON CACHE BOOL "" FORCE)
set(OPENQMC_SHARED_LIB ON CACHE BOOL "" FORCE)

add_subdirectory(openqmc EXCLUDE_FROM_ALL)

mark_as_advanced(FORCE OPENQMC_ARCH_TYPE)
mark_as_advanced(FORCE OPENQMC_BUILD_TOOLS)
mark_as_advanced(FORCE OPENQMC_BUILD_TESTING)
mark_as_advanced(FORCE OPENQMC_FORCE_DOWNLOAD)
mark_as_advanced(FORCE OPENQMC_ENABLE_BINARY)
mark_as_advanced(FORCE OPENQMC_SHARED_LIB)
mark_as_advanced(FORCE OPENQMC_FORCE_PIC)