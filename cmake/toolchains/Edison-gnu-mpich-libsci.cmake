# NOTE: You will need to have the GNU environment loaded, e.g., via the command
#       
# module swap PrgEnv-intel PrgEnv-gnu
#

# The Cray wrappers
set(COMPILER_DIR $ENV{CRAYPE_DIR}/bin)
set(CMAKE_C_COMPILER       ${COMPILER_DIR}/cc)
set(CMAKE_CXX_COMPILER     ${COMPILER_DIR}/CC)
set(CMAKE_Fortran_COMPILER ${COMPILER_DIR}/ftn)

# This is just a hack, as this machine always uses the above wrappers
set(MPI_C_COMPILER ${CMAKE_C_COMPILER})
set(MPI_CXX_COMPILER ${CMAKE_CXX_COMPILER})
set(MPI_Fortran_COMPILER ${CMAKE_Fortran_COMPILER})

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(C_FLAGS   "-g -static -Wl,-Bstatic")
  set(CXX_FLAGS "-g -static -Wl,-Bstatic")
else()
  set(C_FLAGS   "-O3 -static -Wl,-Bstatic")
  set(CXX_FLAGS "-O3 -static -Wl,-Bstatic")
endif()

set(OpenMP_CXX_FLAGS "-fopenmp")

string(REPLACE " " ";" GNU_VERSIONS $ENV{PE_LIBSCI_GENCOMPS_GNU_x86_64})
list(GET GNU_VERSIONS 0 GNU_VERSION_NUMBER)
set(MATH_LIBS "/opt/cray/libsci/default/GNU/${GNU_VERSION_NUMBER}/sandybridge/lib/libsci_gnu.a")
set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static")
