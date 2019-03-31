/*
*   Example code using a timer to update a render object.
*
*   4D-CT Pipeline: similar to the timer/animation example, but first process the 80 volumes (filter, threshold, Marching cubes), 
*                   then save them to STL files. Then read in these files and loop through them (update mappers and actors with new inputs for each volume).
*/


// TO-DO: Organize code!!!

// TO-DO: Add volume animations - DONE (29-03-2019)
// TO-DO: Add point picking
// TO-DO: Track picked points
// TO-DO: Quantify changes in distance (or changes in geometry?)

#include "helperFunctions.hxx"

// TO-DO: Clean up headers. Move to helperFunctions.hxx
#include <filesystem>
namespace fs = std::experimental::filesystem;

#include <vector>
#include <utility>
#include <fstream>

#include <vtkProperty.h>
#include <vtkWindowedSincPolyDataFilter.h>

#include <vtkProgrammableFilter.h>
#include <vtkCallbackCommand.h>
#include <vtkCubeSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkLight.h>

// ONLY WORKS ON WINDOWS!
#include <direct.h>

void TimerCallbackFunction ( vtkObject* caller, long unsigned int eventId, void* clientData, void* callData );

// Globals
unsigned int counter2 = 0;
unsigned int volCounter = 0;

bool forward = true;
bool reverse = false;

// Create a vector for each surface object (saving the volumes proved to be much more difficult...)
std::vector< vtkSmartPointer<vtkPolyData> > dicomVolumes;

// Function to switch between volumes/frames
void AdjustPoints2(void* arguments)
{
  vtkProgrammableFilter* programmableFilter = static_cast<vtkProgrammableFilter*>(arguments);

  vtkPolyData* inPts = programmableFilter->GetPolyDataInput();

  vtkSmartPointer<vtkPolyData> newPts = vtkSmartPointer<vtkPolyData>::New();
  newPts = dicomVolumes[volCounter];

  programmableFilter->GetPolyDataOutput()->CopyStructure(programmableFilter->GetPolyDataInput());
  programmableFilter->GetPolyDataOutput()->DeepCopy(newPts);
}

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

  vtkSmartPointer<vtkDICOMImageReader> dicomReader   = vtkSmartPointer<vtkDICOMImageReader>::New();
  vtkSmartPointer<vtkImageData>        volume        = vtkSmartPointer<vtkImageData>::New();
  vtkSmartPointer<vtkImageData>        gaussianImage = vtkSmartPointer<vtkImageData>::New();
  vtkSmartPointer<vtkImageData>        segImage      = vtkSmartPointer<vtkImageData>::New();

// TO-DO: Move code below to a new function
  // /***************************************************************
  // *   Sort and store the DICOM files by volume/frame
  // ***************************************************************/
  // // TO-DO: Fix hardcoded file path
  std::string path = "D:\\Git\\sort_4D-CT_DICOMs\\DICOMs\\SORTED";

  // /* 
  // *  First, read in all files in the directory into a vector.
  // *  Next, sort the vector into ascending order. When you read in the directory,
  // *   the files aren't in ascending order (i.e. 1,2,3,...,1280). Instead, they
  // *   are ordered by the first number in the file name (i.e. 1,10,100,1000,...).
  // *  Create a vector to hold each file's path and number.
  // */
  std::cout << "\n***Reading and sorting 4D DICOM images***\n";
  std::vector< std::pair<int, std::string> > dicomDirectoryData;

  for ( const auto & entry : fs::directory_iterator( path ) )
  {
    // For now, the location of the file's number is hardcoded...
    // TO-DO: fix
    int temp = std::stoi( entry.path().string().substr( 42, entry.path().string().length() - 4 ) );
    dicomDirectoryData.push_back( { temp, entry.path().string() } );
  }

  /* 
  *  Now sort the vector in ascending order by the file's number
  *  Since we made the file number the first element in the pair, we can just use a simple sort
  */
  std::sort( dicomDirectoryData.begin(), dicomDirectoryData.end() );

  std::vector< std::vector< std::pair<int, std::string> > > data;

  int count = 0;

  for ( int i = 0; i < 10; i++ )
  {    
    std::cout << "Reading DICOMs for volume #" << (i+1) << "...";

    // Create a new directory for each volume (**ONLY WORKS ON WINDOWS OS!**)
    std::string num = std::to_string( i+1 );
    std::string volDir = "D:\\4D-CT Data\\TestTube\\volumes\\vol_" + num + "\\";
    mkdir( volDir.c_str() );

    std::vector< std::pair<int, std::string> > temp;

    for ( int j = 0; j < 16 && count < 160; j++, count++ )
    {
      temp.push_back( { dicomDirectoryData[count].first, dicomDirectoryData[count].second } );

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
    // Perform segmentation to extract bone
    int lowerThresh = -800, upperThresh = 500;
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

    // Add decimation?

    // Smooth the surface using Taubin smoothing
    vtkSmartPointer<vtkWindowedSincPolyDataFilter> smoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
    smoother->SetInputConnection( surface->GetOutputPort() );
    smoother->SetNumberOfIterations( 15 );
    smoother->BoundarySmoothingOff();
    smoother->FeatureEdgeSmoothingOff();
    smoother->SetFeatureAngle( 120.0 );
    smoother->SetPassBand( 0.001 );
    smoother->NonManifoldSmoothingOn();
    smoother->NormalizeCoordinatesOn();
    smoother->Update();

    dicomVolumes.push_back( vtkSmartPointer<vtkPolyData>::New() );
    dicomVolumes[i] = smoother->GetOutput();

    std::cout << "Done! \n";

    // Test Rendering
    // if ( i == 3 )
    // {
    //   vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    //   renderer->SetBackground( 0, 0, 0 );

    //   vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    //   renderWindow->AddRenderer( renderer );
    //   vtkSmartPointer<vtkRenderWindowInteractor> interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    //   interactor->SetRenderWindow( renderWindow );

    //   vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    //   mapper->SetInputConnection( smoother->GetOutputPort() );
    //   mapper->ScalarVisibilityOff();

    //   vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    //   actor->SetMapper( mapper );

    //   renderer->AddActor( actor );

    //   renderWindow->Render();
    //   interactor->Start();
    // }
  }  

  // Create a plane
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

  vtkSmartPointer<vtkActor> cubeActor = vtkSmartPointer<vtkActor>::New();
  cubeActor->SetMapper(cubeMapper);
  cubeActor->GetProperty()->SetColor(0.8,0.8,0.4);
  cubeActor->GetProperty()->ShadingOn();
  cubeActor->GetProperty()->SetDiffuse(0.8);
  // cubeActor->GetProperty()->SetAmbient(0.2);
  // cubeActor->GetProperty()->SetSpecular(0.8);

  vtkSmartPointer<vtkProgrammableFilter> programmableFilter = vtkSmartPointer<vtkProgrammableFilter>::New();
  programmableFilter->SetInputData(dicomVolumes[volCounter]);
  programmableFilter->SetExecuteMethod(AdjustPoints2, programmableFilter);

  // Create a mapper and actor
  vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(programmableFilter->GetOutputPort());
  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(mapper);

  // Create a renderer, render window, and interactor
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(renderer);
  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  renderWindowInteractor->SetRenderWindow(renderWindow);

  // Initialize must be called prior to creating timer events.
  renderWindowInteractor->Initialize();
  renderWindowInteractor->CreateRepeatingTimer(200);

  vtkSmartPointer<vtkCallbackCommand> timerCallback = vtkSmartPointer<vtkCallbackCommand>::New();
  timerCallback->SetCallback ( TimerCallbackFunction );
  timerCallback->SetClientData( programmableFilter );

  renderWindowInteractor->AddObserver ( vtkCommand::TimerEvent, timerCallback );

  // Add the actor to the scene
  renderer->AddActor(actor);
  renderer->AddActor(cubeActor);
  renderer->SetBackground(0,0,0); // Background color black

  // Add another light source
  // vtkSmartPointer<vtkLight> light = vtkSmartPointer<vtkLight>::New();
  // light->SetPositional(1);
  // light->SetPosition(70, 50, 25);
  // light->SetColor(1.0, 1.0, 1.0);
  // light->SetIntensity(0.8);

  // vtkSmartPointer<vtkLight> light2 = vtkSmartPointer<vtkLight>::New();
  // light2->SetPositional(1);
  // light2->SetPosition(100, 0, 0);
  // light2->SetColor(1.0, 1.0, 1.0);
  // light2->SetIntensity(0.8);

  //   vtkSmartPointer<vtkLight> light3 = vtkSmartPointer<vtkLight>::New();
  // light3->SetPositional(1);
  // light3->SetPosition(0, 0, 100);
  // light3->SetColor(1.0, 1.0, 1.0);
  // light3->SetIntensity(0.8);

  // renderer->AddLight(light);
  // renderer->AddLight(light2);
  // renderer->AddLight(light3);

  // Render and interact
  renderWindow->Render();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}

void TimerCallbackFunction ( vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* clientData, void* vtkNotUsed(callData) )
{
  if (volCounter <= 0)
  {
    forward = true;
    reverse = false;
  }
  else if (volCounter >= 9)
  {
    reverse = true;
    forward = false;
  }

  vtkSmartPointer<vtkProgrammableFilter> programmableFilter = static_cast<vtkProgrammableFilter*>(clientData);

  vtkRenderWindowInteractor *iren = static_cast<vtkRenderWindowInteractor*>(caller);

  programmableFilter->Modified();

  iren->Render();

  counter2++;

  if (forward)
    volCounter++;
  else if (reverse)
    volCounter--;
}