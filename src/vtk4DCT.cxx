/*
*
*   4D-CT Pipeline: similar to the timer/animation example, but first process the 80 volumes (filter, threshold, Marching cubes), 
*                   then save them to a vector. Then read in these files and loop through them (update mappers and actors with new inputs for each volume).
*/


// TO-DO: Organize code!!!

// TO-DO: Add volume animations                                   - DONE (29-03-2019)
// TO-DO: Add point picking                                       - DONE (30-03-2019)
// TO-DO: Control playback with keyboard                          - DONE (02-04-2019)
// TO-DO: Display picked points                                   - DONE (01-04-2019)
// TO-DO: Connect points with lines                               - DONE (02-04-2019)
// TO-DO: Get distance between points                             - DONE (02-04-2019)
// TO-DO: Track picked points (registration)                      -
// TO-DO: Quantify changes in distance (or changes in geometry?)  -

// void TimerCallbackFunction ( vtkObject* caller, long unsigned int eventId, void* clientData, void* callData );

#include "vtk4DCT.hxx"
#include "helperFunctions.hxx"
#include "interactorStyler.hxx"
#include "timerCallback.hxx"

// Function to switch between volumes/frames
// void AdjustPoints2(void* arguments)
// {
//   vtkProgrammableFilter* programmableFilter = static_cast<vtkProgrammableFilter*>(arguments);

//   vtkPolyData* inPts = programmableFilter->GetPolyDataInput();

//   vtkSmartPointer<vtkPolyData> newPts = vtkSmartPointer<vtkPolyData>::New();
//   newPts = dicomVolumes[volCounter];

//   programmableFilter->GetPolyDataOutput()->CopyStructure(programmableFilter->GetPolyDataInput());
//   programmableFilter->GetPolyDataOutput()->DeepCopy(newPts);
// }

vtkStandardNewMacro(myInteractorStyle);

int main(int argc, char* argv[])
{
  /***************************************************************
  *   Check input arguements
  ***************************************************************/
  // if ( argc != 2 )
  // {
  //     std::cout << "ERROR: Incorrect program usage. \n";
  //     std::cout << "Correct usage: \n";
  //     std::cout << argv[0] << " <DICOM_Folder_Directory> \n";
  //     return EXIT_FAILURE;
  // }

  vtkSmartPointer<vtkDICOMImageReader> dicomReader   = vtkSmartPointer<vtkDICOMImageReader>::New();
  vtkSmartPointer<vtkImageData>        volume        = vtkSmartPointer<vtkImageData>::New();
  vtkSmartPointer<vtkImageData>        gaussianImage = vtkSmartPointer<vtkImageData>::New();
  vtkSmartPointer<vtkImageData>        segImage      = vtkSmartPointer<vtkImageData>::New();

// TO-DO: Move code below to a new function
  /***************************************************************
  *   Sort and store the DICOM files by volume/frame
  ***************************************************************/
  // TO-DO: Fix hardcoded file path
  std::string path = "D:\\Git\\sort_4D-CT_DICOMs\\DICOMs\\Test Tube\\SORTED";
  // std::string path = "D:\\Git\\sort_4D-CT_DICOMs\\DICOMs\\Two Spheres\\DECOMP\\SORTED";

  /* 
  *  First, read in all files in the directory into a vector.
  *  Next, sort the vector into ascending order. When you read in the directory,
  *   the files aren't in ascending order (i.e. 1,2,3,...,1280). Instead, they
  *   are ordered by the first number in the file name (i.e. 1,10,100,1000,...).
  *  Create a vector to hold each file's path and number.
  */
  std::cout << "\n***Reading and sorting 4D DICOM images***\n";
  std::vector< std::pair<int, std::string> > dicomDirectoryData;

  for ( const auto & entry : fs::directory_iterator( path ) )
  {
    // std::cout << std::stoi( entry.path().string().substr( 61, entry.path().string().length() - 8 ) ) << " ";
    // For now, the location of the file's number is hardcoded...
    // TO-DO: fix - the string path and the substring indicies are hardcoded... (52 on my laptop, 67 on my desktop)
    // 61 for sphere data on laptop
    int temp = std::stoi( entry.path().string().substr( 52, entry.path().string().length() - 4 ) );
    dicomDirectoryData.push_back( { temp, entry.path().string() } );
  }

  /* 
  *  Now sort the vector in ascending order by the file's number
  *  Since we made the file number the first element in the pair, we can just use a simple sort
  */
  std::sort( dicomDirectoryData.begin(), dicomDirectoryData.end() );

  std::vector< std::vector< std::pair<int, std::string> > > data;

  int count = 0;

  for ( int i = 0; i < 5; i++ )
  {    
    std::cout << "Reading DICOMs for volume #" << (i+1) << "...";

    // Create a new directory for each volume (**ONLY WORKS ON WINDOWS OS!**)
    std::string num = std::to_string( i+1 );
    std::string volDir = "D:\\4D-CT Data\\TestTube\\volumes\\vol_" + num + "\\";
    // std::string volDir = "D:\\4D-CT Data\\Spheres\\TwoSpheres-4DCT\\volumes\\vol_" + num + "\\";
    mkdir( volDir.c_str() );

    std::vector< std::pair<int, std::string> > temp;

    for ( int j = 0; j < 16 && count < 80; j++, count++ )
    {
      temp.push_back( { dicomDirectoryData[count].first, dicomDirectoryData[count].second } );

      // TO-DO: fix - the string path and the substring indicies are hardcoded... (39 on my laptop, 64 on my desktop)
      // 58 for sphere data on laptop
      // std::cout << (dicomDirectoryData[count].second).substr( 58, ( dicomDirectoryData[count].second ).length()) << " ";
      std::string dicomFile = ( dicomDirectoryData[count].second).substr( 39, ( dicomDirectoryData[count].second ).length() );
      std::string srcDir    = dicomDirectoryData[count].second;

      std::ifstream src(srcDir.c_str(), std::ios::binary);
      std::ofstream dest( ( volDir + dicomFile ).c_str(), std::ios::binary);
      dest << src.rdbuf();
      src.close();
      dest.close();
    }
    data.push_back( temp );

    std::cout << "Done! \n";
  }

  /***************************************************************
  *   Process each volume and save results into a vector
  ***************************************************************/
  // Experiment with various edge detections + filters + segmentations?

  std::cout << "\n***Processing the volumes***\n";

  // First, sort the slices of each volume into ascending order
  for ( int i = 0; i < data.size(); i++ )
  {
    std::sort( data[i].begin(), data[i].end() );
  }

  for ( int i = 0; i < data.size(); i++ )
  {
    std::cout << "Processing volume #" << (i+1) << "...";

    std::string temp = "D:\\4D-CT Data\\TestTube\\volumes\\vol_" + std::to_string( i + 1 );
    // std::string temp = "D:\\4D-CT Data\\Spheres\\TwoSpheres-4DCT\\volumes\\vol_" + std::to_string( i + 1 );

    dicomReader->SetDirectoryName( temp.c_str() );
    dicomReader->Update();

    /***************************************************************
    *   Apply a Gaussian and median filter to the image
    ***************************************************************/
    vtkSmartPointer<vtkImageGaussianSmooth> gaussianSmoothFilter = vtkSmartPointer<vtkImageGaussianSmooth>::New();
    gaussianSmoothFilter->SetInputData( dicomReader->GetOutput() );
    gaussianSmoothFilter->SetStandardDeviation( 1.0 );
    gaussianSmoothFilter->SetRadiusFactors( 1.0, 1.0, 1.0 );
    gaussianSmoothFilter->SetDimensionality( 3 );
    gaussianSmoothFilter->Update();

    /***************************************************************
    *   Segment the input image
    ***************************************************************/
    // Perform segmentation (-800 to 500 for entire device)
    int lowerThresh = -200, upperThresh = 500;
    double isoValue = 50.0;

    // Apply the global threshold
    vtkSmartPointer<vtkImageThreshold> globalThresh = vtkSmartPointer<vtkImageThreshold>::New();
    globalThresh->SetInputData( gaussianSmoothFilter->GetOutput() );
    globalThresh->ThresholdBetween( lowerThresh, upperThresh );
    globalThresh->ReplaceInOn();
    globalThresh->SetInValue( isoValue + 1 );
    globalThresh->ReplaceOutOn();
    globalThresh->SetOutValue(0);
    globalThresh->SetOutputScalarTypeToFloat();
    globalThresh->Update();

    // Use the Marching cubes algorithm to generate the surface
    vtkSmartPointer<vtkMarchingCubes> surface = vtkSmartPointer<vtkMarchingCubes>::New();
    surface->SetInputData( globalThresh->GetOutput() );
    surface->ComputeNormalsOn();
    surface->SetValue( 0, isoValue );

    // Reduce the number of triangles by half to speed up computation
    vtkSmartPointer<vtkDecimatePro> decimate = vtkSmartPointer<vtkDecimatePro>::New();
    decimate->SetInputConnection( surface->GetOutputPort() );
    decimate->SetTargetReduction( 0.5 );
    decimate->Update();

    // Smooth the surface using Taubin smoothing
    vtkSmartPointer<vtkWindowedSincPolyDataFilter> smoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
    smoother->SetInputConnection( decimate->GetOutputPort() );
    smoother->SetNumberOfIterations( 5 );
    smoother->BoundarySmoothingOff();
    smoother->FeatureEdgeSmoothingOff();
    smoother->SetFeatureAngle( 75.0 );
    smoother->SetPassBand( 0.01 );
    smoother->NonManifoldSmoothingOn();
    smoother->NormalizeCoordinatesOn();
    smoother->Update();

// Fill holes looks bad!
    // Fill holes in the surface. Hole size is represented as a radius to the bounding circumsphere containing the hole.
    // vtkSmartPointer<vtkFillHolesFilter> fillHolesFilter =
    // vtkSmartPointer<vtkFillHolesFilter>::New();
    // fillHolesFilter->SetInputData(smoother->GetOutput());
    // fillHolesFilter->SetHoleSize(10000000.0);   // limit the hole size to be filled
    // fillHolesFilter->Update();

    // Add the volume to the volume vector
    dicomVolumes.push_back( vtkSmartPointer<vtkPolyData>::New() );
    dicomVolumes[i] = surface->GetOutput();

    std::cout << "Done! \n";
  }  

  // Now register the first frame onto the remaining volumes
  /***************************************************************
  *   Perform ICP registration
  ***************************************************************/
  // Perform the registration between each frame
  // Start at the first frame (registration onto itself) to keep the vector length consistent. This will be helpful when rendering.
  for ( int i = 0; i < dicomVolumes.size(); i++ )
  {
    vtkSmartPointer<vtkIterativeClosestPointTransform> icp = vtkSmartPointer<vtkIterativeClosestPointTransform>::New();
    icp->SetSource( dicomVolumes[0] );
    icp->SetTarget( dicomVolumes[i] );
    icp->SetMaximumNumberOfIterations( 75 );
    icp->GetLandmarkTransform()->SetModeToRigidBody();
    icp->StartByMatchingCentroidsOn();
    icp->Update();

    // Output the transformation matrix. Need to convert the vtkMatrix4x4 matrix to a vtkAbstractTransform.
    vtkSmartPointer<vtkMatrix4x4> m = icp->GetMatrix();

    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New(); 
    transform->SetMatrix( m );

    tMatricies.push_back( transform );

    // vtkSmartPointer<vtkTransform> icpTransformFilter = vtkSmartPointer<vtkTransform>::New();
    // icpTransformFilter->SetMatrix( m );
    // icpTransformFilter->Update();
  }

  /***************************************************************
  *   Create the scene
  ***************************************************************/
  // Create a cube surface
  vtkSmartPointer<vtkCubeSource> cubeSource = vtkSmartPointer<vtkCubeSource>::New();
  cubeSource->SetXLength(125);
  cubeSource->SetYLength(10);
  cubeSource->SetZLength(150);
  cubeSource->SetCenter(70,0,25);
  cubeSource->Update();

  vtkPolyData* cube = cubeSource->GetOutput();

  vtkSmartPointer<vtkTransform> rotation = vtkSmartPointer<vtkTransform>::New();
  rotation->RotateX(10);

  vtkSmartPointer<vtkTransformPolyDataFilter> rotationFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  rotationFilter->SetInputData(cube);
  rotationFilter->SetTransform(rotation);
  rotationFilter->Update();

  vtkSmartPointer<vtkPolyDataMapper> cubeMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  cubeMapper->SetInputData(rotationFilter->GetOutput());

// TO-DO: Play with colouring
  vtkSmartPointer<vtkActor> cubeActor = vtkSmartPointer<vtkActor>::New();
  cubeActor->SetMapper(cubeMapper);
  cubeActor->GetProperty()->SetColor(0.8,0.8,0.4);
  cubeActor->GetProperty()->ShadingOn();
  cubeActor->GetProperty()->SetDiffuse(0.8);
  // cubeActor->GetProperty()->SetAmbient(0.2);
  // cubeActor->GetProperty()->SetSpecular(0.8);

  // programmableFilter->SetInputData( dicomVolumes[volumeCounter] );
  // programmableFilter->SetExecuteMethod( AdjustPoints2, programmableFilter );

  // Create a mapper and actor
  for ( int i = 0; i < dicomVolumes.size(); i++ )
  {
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData( dicomVolumes[i] );

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper( mapper );

    volumeActors.push_back( actor );
  }

  // Create a renderer, render window, and interactor
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer( renderer );

  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  renderWindowInteractor->SetRenderWindow( renderWindow );

  // Initialize must be called prior to creating timer events.
  renderWindowInteractor->Initialize();
  renderWindowInteractor->EnableRenderOn();

  vtkSmartPointer<myInteractorStyle> style = vtkSmartPointer<myInteractorStyle>::New();
  renderWindowInteractor->SetInteractorStyle( style );

  // Add the actor to the scene
  renderer->AddActor( volumeActors[0] );
  renderer->AddActor( cubeActor );
  renderer->SetBackground( 1, 1, 1 ); // Background color white

  // Render and interact
  renderWindow->Render();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}

// void TimerCallbackFunction ( vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* clientData, void* vtkNotUsed(callData) )
// {
//   // Once we hit the last volume, play the animation in reverse.
//   if ( volCounter <= 0 )
//   {
//     forward = true;
//     reverse = false;
//   }
//   else if ( volCounter >= 4 )
//   {
//     reverse = true;
//     forward = false;
//   }

//   vtkSmartPointer<vtkProgrammableFilter> programmableFilter = static_cast<vtkProgrammableFilter*>(clientData);

//   vtkRenderWindowInteractor *iren = static_cast<vtkRenderWindowInteractor*>(caller);

//   programmableFilter->Modified();

//   iren->Render();

//   if ( forward )
//     volCounter++;
//   else if ( reverse )
//     volCounter--;
// }