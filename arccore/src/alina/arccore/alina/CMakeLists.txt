include (srcs.cmake)

arccore_add_component_library(alina
  FILES ${SOURCES}
)

target_compile_definitions(arccore_alina PRIVATE ARCANE_COMPONENT_arccore_alina)

target_link_libraries(arccore_alina PUBLIC arccore_accelerator)
#arcane_register_library(arccore_alina OPTIONAL)

if (ARCCORE_CXX_COMPILER_IS_GNU_OR_CLANG_BASED)
  target_compile_options(arccore_alina PUBLIC -Wno-conversion -Wno-float-conversion)
endif()

#----------------------------------------------------------------------------
# Ajoute Arccore MPI
# À l'avenir, cela ne sera plus nécessaire. Seul 'arccore_message_passing' sera nécessaire.

if (NOT ARCCORE_ENABLE_MPI)
  message(FATAL_ERROR "Alina is enabled but need MPI but this is disabled in Arccore."
    "You have to disable Alina using -DARCCORE_ENABLE_ALINA=OFF or enabling MPI using -DARCCORE_ENABLE_MPI=ON"
    )
endif()
target_link_libraries(arccore_alina PUBLIC Arccore::arccore_message_passing_mpi)

#----------------------------------------------------------------------------
# Trouve CUDA pour cusparse
#----------------------------------------------------------------------------
if (ARCCORE_HAS_CUDA)
  find_package(CUDAToolkit REQUIRED)
  add_library(cuda_target INTERFACE)
  target_link_libraries(cuda_target INTERFACE ${CUDA_cusparse_LIBRARY})
endif()

#----------------------------------------------------------------------------
# Trouve Metis/Parmetis
#----------------------------------------------------------------------------
find_package(Metis QUIET)
find_package(Parmetis)
if(Parmetis_FOUND)
  message(STATUS "Found Parmetis for Alina")
  target_compile_definitions(arccore_alina PUBLIC ARCCORE_ALINA_HAVE_PARMETIS ARCCORE_ALINA_HAVE_PARMETIS)
  target_link_libraries(arccore_alina PUBLIC arccon::Parmetis)
endif()

#----------------------------------------------------------------------------
# Vérifie la compilation des tests et des exemples
#----------------------------------------------------------------------------

option(ARCCORE_ALINA_BUILD_TESTS "Build Alina tests" ON)
option(ARCCORE_ALINA_BUILD_SAMPLES "Build Alina samples" ON)

if (ARCCORE_ALINA_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if (ARCCORE_ALINA_BUILD_SAMPLES)
  add_subdirectory(samples)
  add_subdirectory(tutorial)
endif()

# ----------------------------------------------------------------------------
# Local Variables:
# tab-width: 2
# indent-tabs-mode: nil
# coding: utf-8-with-signature
# End:
