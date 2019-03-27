/*
*   Example code using a timer to update a render object.
*
*   4D-CT Pipeline: similar to the timer/animation example, but first process the 80 volumes (filter, threshold, Marching cubes), 
*                   then save them to STL files. Then read in these files and loop through them (update mappers and actors with new inputs for each volume).
*/
#include "helperFunctions.hxx"

int main(int argc, char* argv[])
{
  /***************************************************************
  *   Check input arguements
  ***************************************************************/
  if ( argc != 2 )
  {
      std::cout << "ERROR: Incorrect program usage. \n";
      std::cout << "Correct usage: \n";
      std::cout << argv[0] << " <DICOM_Folder_Directory> \n";
      return EXIT_FAILURE;
  }

  vtkSmartPointer<vtkDICOMImageReader> dicomReader = vtkSmartPointer<vtkDICOMImageReader>::New();;

  vtkSmartPointer<vtkImageData> volume        = vtkSmartPointer<vtkImageData>::New();
  vtkSmartPointer<vtkImageData> gaussianImage = vtkSmartPointer<vtkImageData>::New();
  vtkSmartPointer<vtkImageData> segImage      = vtkSmartPointer<vtkImageData>::New();

  dicomReader->SetDirectoryName( argv[1] );
  dicomReader->Update();

  volume = dicomReader->GetOutput();

  /***************************************************************
  *   Apply a Gaussian and median filter to the image
  ***************************************************************/
  std::cout << "\n**Filtering the input image** \n";

  std::cout << "Applying a Gaussian filter with std = 1.0...";
  vtkSmartPointer<vtkImageGaussianSmooth> gaussianSmoothFilter = vtkSmartPointer<vtkImageGaussianSmooth>::New();
  gaussianSmoothFilter->SetInputData( volume );
  gaussianSmoothFilter->SetStandardDeviation( 1.0 );
  gaussianSmoothFilter->SetRadiusFactors( 1.0, 1.0, 1.0 );
  gaussianSmoothFilter->SetDimensionality( 3 );
  gaussianSmoothFilter->Update();

  gaussianImage = gaussianSmoothFilter->GetOutput();
  std::cout << "Done! \n";

  /***************************************************************
  *   Segment the input image
  ***************************************************************/
  // Perform segmentation to extract bone
  int lowerThresh = 0, upperThresh = 0;
  double isoValue = 0.0;

  // Get the threshold and isovalue parameters from the user
  std::cout << "Performing image segmentation \n";
  std::cout << "Please enter upper and lower threshold values: \n";
  std::cout << "Lower Threshold = ";
  std::cin >> lowerThresh;
  std::cout << "Upper Threshold = ";
  std::cin >> upperThresh;

  std::cout << "Please enter the desired isovalue for the Marching Cubes algortihm: ";
  std::cin >> isoValue;

  // Apply the global threshold
  vtkSmartPointer<vtkImageThreshold> globalThresh = vtkSmartPointer<vtkImageThreshold>::New();
  globalThresh->SetInputData( gaussianImage );
  globalThresh->ThresholdBetween( lowerThresh, upperThresh );
  globalThresh->ReplaceInOn();
  globalThresh->SetInValue( isoValue + 1 );
  globalThresh->ReplaceOutOn();
  globalThresh->SetOutValue(0);
  globalThresh->SetOutputScalarTypeToFloat();
  globalThresh->Update();

  segImage = globalThresh->GetOutput();

  // Use the Marching cubes algorithm to generate the surface
  std::cout << "Generating surface using Marching cubes \n";

  vtkSmartPointer<vtkMarchingCubes> surface = vtkSmartPointer<vtkMarchingCubes>::New();
  surface->SetInputData( segImage );
  surface->ComputeNormalsOn();
  surface->SetValue( 0, isoValue );

  vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection( surface->GetOutputPort() );
  mapper->ScalarVisibilityOff();

  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper( mapper );

  // Create the renderer and render window
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  renderer->SetBackground( 0, 0, 0 );

  vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer( renderer );
  vtkSmartPointer<vtkRenderWindowInteractor> interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  interactor->SetRenderWindow( renderWindow );

  renderer->AddActor( actor );
  renderWindow->Render();
  interactor->Start();

  std::cout << "Done! \n";

  return EXIT_SUCCESS;
}