if (ARCCORE_ENABLE_GLIB)
  find_package(Glib REQUIRED)
endif ()
if (ARCCORE_ENABLE_TBB)
  find_package(TBB)
  if (TBB_VERSION)
    # Si TBB_VERSION est défini, nous sommes certains que TBB est présent.
    # Normalement, cette variable est définie si nous utilisons une version de TBB
    # qui possède un fichier de configuration CMake
    message(STATUS "[arccore_concurrency] TBB_VERSION=${TBB_VERSION}")
    if (TBB_VERSION VERSION_GREATER_EQUAL "2021")
      message(STATUS "[arcane_thread] Utilisation de l'implémentation OneTBB 2021+")
      set(ARCANE_HAS_ONETBB TRUE)
    else ()
      message(FATAL_ERROR "[arcane_thread] Votre version '${TBB_VERSION} de TBB est trop ancienne. La version 2021+ est requise."
        " Installez une version récente de TBB ou désactivez le support TBB en utilisant -DARCCORE_ENABLE_TBB=FALSE lors de l'exécution de la commande cmake")
    endif ()
  elseif (TBB_FOUND)
    message(FATAL_ERROR
      "[arccore_concurrency] Impossible de trouver TBB_VERSION. Cela est probablement dû au fait que votre version de TBB est trop ancienne."
      "La version 2021+ est requise."
      "Installez une version récente de TBB ou désactivez le support TBB en utilisant -DARCCORE_ENABLE_TBB=FALSE lors de l'exécution de la commande cmake"
    )
  else ()
    message(FATAL_ERROR
      "[arccore_concurrency] Impossible de trouver OneTBB."
      "Installez une version récente (2021+) de TBB ou désactivez le support TBB en utilisant -DARCCORE_ENABLE_TBB=FALSE lors de l'exécution de la commande cmake"
    )
  endif ()
endif ()

include(srcs.cmake)

if (ARCCORE_ENABLE_GLIB)
  list(APPEND SOURCES
    GlibThreadImplementation.cc
    GlibThreadImplementation.h
    GlibAdapter.h
    GlibAdapter.cc
    ThreadPrivate.h
    ThreadPrivate.cc
  )
endif ()
if (ARCCORE_ENABLE_TBB)
  list(APPEND SOURCES
    TBBTaskImplementation.cc
  )
endif ()

arccore_add_component_library(concurrency
  FILES ${SOURCES}
)

if (ARCCORE_ENABLE_GLIB)
  target_link_libraries(arccore_concurrency PRIVATE arcconpkg_Glib)
  target_compile_definitions(arccore_concurrency PRIVATE ARCCORE_HAS_GLIB)
endif ()
if (ARCCORE_ENABLE_TBB)
  target_link_libraries(arccore_concurrency PRIVATE arccon::TBB)
endif ()

target_link_libraries(arccore_concurrency PUBLIC arccore_trace arccore_base)

find_package(Threads)
if (TARGET Threads::Threads)
  target_link_libraries(arccore_concurrency PRIVATE Threads::Threads)
endif ()

# ----------------------------------------------------------------------------
# Local Variables:
# tab-width: 2
# indent-tabs-mode: nil
# coding: utf-8-with-signature
# End:
