/****************************************************************************
*   vtk4DCT.hxx
*
*   Created by:     Michael Kuczynski
*   Created on:     03/04/2019
*   Description:    
*****************************************************************************
*   TO-DO:
*       - Clean up headers
*
****************************************************************************/
#ifndef VTK4DCT_H
#define VTK4DCT_H

// Global system headers
#include <filesystem>
namespace fs = std::experimental::filesystem;

#include <vector>
#include <utility>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <chrono>

// ONLY WORKS ON WINDOWS!
#include <direct.h>

// VTK headers
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkImageMapper.h>
#include <vtkImageViewer2.h>
#include <vtkImageData.h>
#include <vtkImageActor.h>
#include <vtkDICOMImageReader.h>
#include <vtkNIFTIImageReader.h>
#include <vtkInteractorStyleImage.h>
#include <vtkTextProperty.h>
#include <vtkTextMapper.h>
#include <vtkImageMapper3D.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkLookupTable.h>
#include <vtkImageMapToColors.h>
#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkImageAlgorithm.h>
#include <vtkImageGaussianSmooth.h>
#include <vtkMarchingCubes.h>
#include <vtkPolyDataMapper.h>
#include <vtkImageThreshold.h>
#include <vtkVolume.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkProperty.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkProgrammableFilter.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkCubeSource.h>
#include <vtkLineSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkLight.h>
#include <vtkWorldPointPicker.h>
#include <vtkRendererCollection.h>
#include <vtkSphereSource.h>
#include <vtkSelectEnclosedPoints.h>
#include <vtkImplicitPolyDataDistance.h>
#include <vtkMath.h>
#include <vtkFillHolesFilter.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkIterativeClosestPointTransform.h>
#include <vtkLandmarkTransform.h>
#include <vtkDecimatePro.h>
#include <vtkMatrixToLinearTransform.h>
#include <vtkAbstractTransform.h>
#include <vtkCamera.h>
#include <vtkImageConstantPad.h>
#include <vtkAngleWidget.h>
#include <vtkAngleRepresentation2D.h>
#include <vtkIntersectionPolyDataFilter.h>
#include <vtkOBBTree.h>
#include <vtkVectorText.h>
#include <vtkImageDilateErode3D.h>

/*******************************************************************************************************************
*   Global variables
********************************************************************************************************************/
static vtkSmartPointer<vtkCubeSource> cubeSource;   // The plane that the volume rests on
static vtkSmartPointer<vtkCubeSource> cubeLine;

static vtkSmartPointer<vtkVectorText> angleText;

static unsigned int volumeCounter  = 0;
static unsigned int pointCounter   = 0;
static unsigned int lineCounter    = 0;
static unsigned int timerDuration  = 200;
static float compTime              = 0;

static double initialCameraPos[3] = { 0 };
static double initialCameraAngle = 0;

static std::vector< vtkSmartPointer<vtkPolyData> >                    dicomVolumes;
static std::vector< vtkSmartPointer<vtkPolyData> >                    points;
static std::vector< std::vector< vtkSmartPointer<vtkPolyData> > >     tPoints;
static std::vector< std::vector< double > >                           distances;
static std::vector< vtkSmartPointer< vtkLineSource > >                angleLines;
static std::vector< double >                                          angles;

static vtkSmartPointer<vtkLineSource> firstAngleLine;
static vtkSmartPointer<vtkActor> firsAngleLineActor;

static std::vector< vtkSmartPointer<vtkActor> >                     volumeActors;
static std::vector< std::vector< vtkSmartPointer<vtkActor> > >      tPointActors;
static std::vector< std::vector< vtkSmartPointer<vtkActor> > >      tLineActors;
static std::vector< vtkSmartPointer<vtkActor> >                     angleLineActors;
static std::vector< vtkSmartPointer<vtkActor> >                     angleTextActors;

static std::vector< vtkSmartPointer<vtkIterativeClosestPointTransform> > icpRegistrations;

// To determine the direction of rendering
static bool forward = true;
static bool reverse = false;

static bool timerStopped = true;
static unsigned long timerTag = 0;
static unsigned long timerID = 0;

/********************************************************************************************************************/

#endif // VTK4DCT_H