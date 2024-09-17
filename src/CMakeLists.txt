
################################
# Fesapi
################################
if(DEFINED FESAPI_DIR)
    message(STATUS "FESAPI_DIR = ${FESAPI_DIR}")

    find_and_import(NAME FesapiCpp
                 INCLUDE_DIRECTORIES ${FESAPI_DIR}/include
                 LIBRARY_DIRECTORIES ${FESAPI_DIR}/lib
                 HEADER fesapi/nsDefinitions.h
                 LIBRARIES FesapiCpp
                 DEPENDS hdf5)

    set(FESAPI_DIR ON CACHE BOOL "")    
else()
  message(STATUS "Not using Fesapi")
endif()

message(STATUS "Adding RESQML (Readers / MeshGenerator) sources and headers")

#
# Specify all headers
#
set(componentHeaders
  RESQMLOutput.hpp
  RESQMLWriterInterface.hpp
  RESQMLMeshGenerator.hpp
  RESQMLUtilities.hpp
  EnergyMLDataObjectRepository.hpp
  EpcDocumentRepository.hpp
  ETPRepository.hpp
  Property.hpp
  Region.hpp
  Surface.hpp
)

#
# Specify all sources
#
set(componentSources
  RESQMLUtilities.cpp
  RESQMLWriterInterface.cpp
  RESQMLMeshGenerator.cpp
  RESQMLOutput.cpp
  EnergyMLDataObjectRepository.cpp
  EpcDocumentRepository.cpp
  ETPRepository.cpp
  Property.cpp
  Region.cpp
  Surface.cpp
)

#
# Specify dependencies
#
# set(dependencyList mesh)
set(dependencyList FesapiCpp)

if(GEOS_BUILD_SHARED_LIBS)
  set(dependencyList ${dependencyList} geosx_core)
else()
  set(dependencyList ${dependencyList} ${geosx_core_libs})
endif()

geos_decorate_link_dependencies( LIST decoratedDependencies
                                 DEPENDENCIES ${dependencyList} )

message("adding RESQML library:")

blt_add_library( NAME resqml
                 SOURCES ${componentSources}
                 HEADERS ${componentHeaders}
                 DEPENDS_ON ${decoratedDependencies}
                 OBJECT ${GEOS_BUILD_OBJ_LIBS}
                 SHARED ${GEOS_BUILD_SHARED_LIBS}
               )

target_compile_definitions(resqml PUBLIC "HAS_UNCAUGHT_EXCEPTIONS")

target_include_directories(resqml PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# geosx_add_code_checks(PREFIX resqml)
