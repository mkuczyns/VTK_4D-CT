/****************************************************************************
*   vtk4DCT.hxx
*
*   Created by:     Michael Kuczynski
*   Created on:     28/03/2019
*   Description:    
*****************************************************************************
*   TO-DO:
*
****************************************************************************/
#include "vtk4DCT.hxx"
#include "helperFunctions.hxx"
#include "interactorStyler.hxx"

vtkStandardNewMacro(myInteractorStyle);

// Note: all file paths are hard coded!
int main(int argc, char* argv[])
{
  // Setup a timer to see how long the entire program takes
  typedef std::chrono::high_resolution_clock Time;
  typedef std::chrono::duration<float> fsec;
  auto t0 = Time::now();

  vtkSmartPointer<vtkDICOMImageReader> dicomReader   = vtkSmartPointer<vtkDICOMImageReader>::New();
  vtkSmartPointer<vtkImageData>        volume        = vtkSmartPointer<vtkImageData>::New();
  vtkSmartPointer<vtkImageData>        gaussianImage = vtkSmartPointer<vtkImageData>::New();
  vtkSmartPointer<vtkImageData>        segImage      = vtkSmartPointer<vtkImageData>::New();

  /***************************************************************
  *   Sort and store the DICOM files by volume/frame
  ***************************************************************/
  // std::string path = "D:\\Git\\sort_4D-CT_DICOMs\\DICOMs\\Test Tube\\SORTED";
  // std::string path = "D:\\Git\\sort_4D-CT_DICOMs\\DICOMs\\Two Spheres\\DECOMP\\SORTED";
  std::string path = "E:\\Git\\VTK_4D-CT\\img\\test tube\\sorted";
  // std::string path = "E:\\Git\\VTK_4D-CT\\img\\spheres\\sorted";

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
    // For now, the location of the file's number is hardcoded...
    // TO-DO: fix - the string path and the substring indicies are hardcoded... (52 on my laptop, 42 on my desktop)
    // 61 for sphere data on laptop, 39 on PC
    int temp = std::stoi( entry.path().string().substr( 41, entry.path().string().length() - 4 ) );
    dicomDirectoryData.push_back( { temp, entry.path().string() } );
  }

  /* 
  *  Now sort the vector in ascending order by the file's number
  *  Since we made the file number the first element in the pair, we can just use a simple sort
  */
  std::sort( dicomDirectoryData.begin(), dicomDirectoryData.end() );

  std::vector< std::vector< std::pair<int, std::string> > > data;

  int count = 0;

  for ( int i = 0; i < 80; i++ )
  {    
    std::cout << "Reading DICOMs for volume #" << (i+1) << "...";

    // Create a new directory for each volume (**ONLY WORKS ON WINDOWS OS!**)
    std::string num = std::to_string( i+1 );
    // std::string volDir = "D:\\4D-CT Data\\TestTube\\volumes\\vol_" + num + "\\";
    // std::string volDir = "D:\\4D-CT Data\\Spheres\\TwoSpheres-4DCT\\volumes\\vol_" + num + "\\";
    std::string volDir = "E:\\Git\\VTK_4D-CT\\img\\test tube\\volumes\\vol_" + num + "\\";
    // std::string volDir = "E:\\Git\\VTK_4D-CT\\img\\spheres\\volumes\\vol_" + num + "\\";
    mkdir( volDir.c_str() );

    std::vector< std::pair<int, std::string> > temp;

    for ( int j = 0; j < 16 && count < 1280; j++, count++ )
    {
      temp.push_back( { dicomDirectoryData[count].first, dicomDirectoryData[count].second } );

      // TO-DO: fix - the string path and the substring indicies are hardcoded... (49 on my laptop, 38 on my desktop)
      // 58 for sphere data on laptop, 36 on PC
      std::string dicomFile = ( dicomDirectoryData[count].second).substr( 38, ( dicomDirectoryData[count].second ).length() );
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
  std::cout << "\n***Processing the volumes***\n";

  // First, sort the slices of each volume into ascending order
  for ( int i = 0; i < data.size(); i++ )
  {
    std::sort( data[i].begin(), data[i].end() );
  }

  for ( int i = 0; i < data.size(); i++ )
  {
    std::cout << "Processing volume #" << (i+1) << "...";

    // std::string temp = "D:\\4D-CT Data\\TestTube\\volumes\\vol_" + std::to_string( i + 1 );
    // std::string temp = "D:\\4D-CT Data\\Spheres\\TwoSpheres-4DCT\\volumes\\vol_" + std::to_string( i + 1 );
    std::string temp = "E:\\Git\\VTK_4D-CT\\img\\test tube\\volumes\\vol_" + std::to_string( i + 1 );
    // std::string temp = "E:\\Git\\VTK_4D-CT\\img\\spheres\\volumes\\vol_" + std::to_string( i + 1 );

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
    int lowerThresh = -150, upperThresh = 500;
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

    vtkSmartPointer<vtkImageDilateErode3D> erode = vtkSmartPointer<vtkImageDilateErode3D>::New();
    erode->SetInputConnection(globalThresh->GetOutputPort());
    erode->SetKernelSize(3,3,3);
    erode->SetDilateValue(0);
    erode->SetErodeValue(isoValue + 1);
    erode->Update();

    vtkSmartPointer<vtkImageDilateErode3D> dilate = vtkSmartPointer<vtkImageDilateErode3D>::New();
    dilate->SetInputConnection(erode->GetOutputPort());
    dilate->SetKernelSize(3,3,3);
    dilate->SetDilateValue(isoValue + 1);
    dilate->SetErodeValue(0);
    dilate->Update();

    // Use the Marching cubes algorithm to generate the surface
    vtkSmartPointer<vtkMarchingCubes> surface = vtkSmartPointer<vtkMarchingCubes>::New();
    surface->SetInputData( dilate->GetOutput() );
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

    // Fill holes in the surface. Hole size is represented as a radius to the bounding circumsphere containing the hole.
    vtkSmartPointer<vtkFillHolesFilter> fillHolesFilter =
    vtkSmartPointer<vtkFillHolesFilter>::New();
    fillHolesFilter->SetInputData(smoother->GetOutput());
    fillHolesFilter->SetHoleSize(10000000.0);   // limit the hole size to be filled
    fillHolesFilter->Update();

    // Add the volume to the volume vector
    dicomVolumes.push_back( vtkSmartPointer<vtkPolyData>::New() );
    dicomVolumes[i] = fillHolesFilter->GetOutput();

    std::cout << "Done! \n";
  }  

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
    icp->SetMaximumNumberOfIterations( 30 );
    icp->GetLandmarkTransform()->SetModeToSimilarity();
    icp->CheckMeanDistanceOn();
    icp->StartByMatchingCentroidsOn();
    icp->Update();
    
    icpRegistrations.push_back( icp );
  }

  // Get a line at the bottom of each volume. The angle will be calculated as the angle between the line in volume 1 and the line from all other volumes.
  // Get the geometry bounding box of the first volume (Xmin, Xmax, Ymin, Ymax, Zmin, Zmax)
  // Transform this line onto all other volumes
  double* bounds = dicomVolumes[0]->GetBounds();

  vtkSmartPointer<vtkLineSource> lineSource = vtkSmartPointer<vtkLineSource>::New();

  double tempArray1[3] = { ( ( bounds[0] + bounds[1] ) /2 ), bounds[2], bounds[4] + 12 }; 
  double tempArray2[3] = { ( ( bounds[0] + bounds[1] ) /2 ), bounds[2], bounds[5] + 150 };  // Extend the Zmax coordinate so each line always intersects with the first line

  lineSource->SetPoint1( tempArray1 );
  lineSource->SetPoint2( tempArray2 );
  lineSource->Update();

  firstAngleLine = lineSource;

  //Create a new actor for the first angle line (always stays on!)
  vtkSmartPointer<vtkPolyDataMapper> firstLineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  firstLineMapper->SetInputData( firstAngleLine->GetOutput() );

  firsAngleLineActor = vtkSmartPointer<vtkActor>::New();
  firsAngleLineActor->SetMapper( firstLineMapper );
  firsAngleLineActor->GetProperty()->SetColor(0.0, 1.0, 0.0);
  firsAngleLineActor->GetProperty()->SetLineWidth(8);

  cubeLine = vtkSmartPointer<vtkCubeSource>::New();
  cubeLine->SetXLength(125);
  cubeLine->SetYLength(1);
  cubeLine->SetZLength(200);
  cubeLine->SetCenter(firstAngleLine->GetPoint1()[0], firstAngleLine->GetPoint1()[1] - 0.5, 25);
  cubeLine->Update();

  vtkSmartPointer<vtkPolyDataMapper> cubeLineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  cubeLineMapper->SetInputData(cubeLine->GetOutput());

  vtkSmartPointer<vtkActor> cubeLineActor = vtkSmartPointer<vtkActor>::New();
  cubeLineActor->SetMapper(cubeLineMapper);
  cubeLineActor->GetProperty()->SetColor(0.5, 0.5, 0.5);
  cubeLineActor->GetProperty()->ShadingOn();

  /***************************************************************
  *   Create the scene
  ***************************************************************/
  // Create a mapper and actor for each volume
  for ( int i = 0; i < dicomVolumes.size(); i++ )
  {
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData( dicomVolumes[i] );

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper( mapper );

    actor->GetProperty()->SetColor(1.0, 1.0, 1.0);
    actor->GetProperty()->SetOpacity(0.85);

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
  renderer->AddActor( cubeLineActor );
  renderer->SetBackground( 0.0, 0.4, 0.6 ); // Background color white

  auto t1 = Time::now();
  fsec fs = t1 - t0;
  compTime = std::chrono::duration_cast<fsec>(fs).count();

  // Render and interact
  renderWindow->Render();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}