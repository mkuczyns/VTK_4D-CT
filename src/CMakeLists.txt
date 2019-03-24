cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

project(vtk4DCT)

find_package(VTK COMPONENTS 
  vtkCommonCore
  vtkFiltersProgrammable
  vtkFiltersSources
  vtkInteractionStyle
  vtkRenderingCore
  vtkRenderingFreeType
  vtkRenderingOpenGL2 QUIET)
if (NOT VTK_FOUND)
  message("Skipping vtk4DCT: ${VTK_NOT_FOUND_MESSAGE}")
  return ()
endif()
message (STATUS "VTK_VERSION: ${VTK_VERSION}")
if (VTK_VERSION VERSION_LESS "8.90.0")
  # old system
  include(${VTK_USE_FILE})
  add_executable(vtk4DCT MACOSX_BUNDLE vtk4DCT.cxx )
  target_link_libraries(vtk4DCT PRIVATE ${VTK_LIBRARIES})
else ()
  # include all components
  add_executable(vtk4DCT MACOSX_BUNDLE vtk4DCT.cxx )
  target_link_libraries(vtk4DCT PRIVATE ${VTK_LIBRARIES})
  # vtk_module_autoinit is needed
  vtk_module_autoinit(
    TARGETS vtk4DCT
    MODULES ${VTK_LIBRARIES}
    )
endif () 