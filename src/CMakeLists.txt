cmake_minimum_required(VERSION 2.8)

PROJECT(vtk4DCT)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(vtk4DCT MACOSX_BUNDLE vtk4DCT.cxx helperFunctions.cxx)

if(VTK_LIBRARIES)
  target_link_libraries(vtk4DCT ${VTK_LIBRARIES})
else()
  target_link_libraries(vtk4DCT vtkHybrid vtkWidgets)
endif()