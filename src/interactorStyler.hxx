/****************************************************************************
*   interactorStyler.hxx
*
*   Created by:     Michael Kuczynski
*   Created on:     19/01/2019
*   Description:    Custom interactor styling definition.
****************************************************************************/

#ifndef INTERACTORSTYLER_H
#define INTERACTORSTYLER_H

#include "vtk4DCT.hxx"
#include "timerCallback.hxx"

/* 
*   A class for a custom interactor style to override the default interactor style.
*/
class myInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
   static myInteractorStyle* New();
   vtkTypeMacro( myInteractorStyle, vtkInteractorStyleTrackballCamera );

   /*
   *   Reset the picked points.
   */
   void resetPointPicking()
   {
      if ( pointCounter <= 0 && angleLineActors.size() == 0 && tLineActors.size() == 0 )
      {
         std::cout << "No points or lines to remove!\n";
      }
      else
      {
         /* Reset the point picking with the following steps:
         *   1. Remove all point and line actors
         *   2. Clear the point and line vectors
         *   3. Reset point and line counts
         *   4. Clear the distance vector
         */

        // Points
         for ( int i = 0; i < tPointActors.size(); i++ )
         {
            for ( int j = 0; j < tPointActors[i].size(); j++ )
            {
               this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor( tPointActors[i][j] );
            }
         }

         // Lines between points
         for ( int i = 0; i < tLineActors.size(); i++ )
         {
            for ( int j = 0; j < tLineActors[i].size(); j++ )
            {
               this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor( tLineActors[i][j] );
            }
         }

         // Angle Lines
         for ( int i = 0; i < angleLineActors.size(); i++ )
         {
            this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor( angleLineActors[i] );
         }

         this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor( firsAngleLineActor );

         for ( int i = 0; i < angleTextActors.size(); i++ )
         {
            this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor( angleTextActors[i] );
         }

         pointCounter = 0;
         lineCounter = 0;

         tPointActors.clear();
         tLineActors.clear();
         angleLineActors.clear();
         angleTextActors.clear();

         points.clear();
         distances.clear();
         angleLines.clear();

         std::cout << "Successfully removed all user selected points and lines! \n";
      }
   }

   void stopTimer()
   {
      if ( !timerStopped )
      {
         this->Interactor->DestroyTimer( timerID );
         this->Interactor->RemoveObserver( timerTag );
         timerStopped = true;
      }
   }

   void startTimer()
   {
      if ( timerStopped )
      {
         // Remove temporary point/line actors
         for ( int i = 0; i < tPointActors.size(); i++ )
         {
            for ( int j = 0; j < tPointActors[i].size(); j++ )
            {
               this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor( tPointActors[i][j] );
            }
         }

         for ( int i = 0; i < tLineActors.size(); i++ )
         {
            for ( int j = 0; j < tLineActors[i].size(); j++ )
            {
               this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor( tLineActors[i][j] );
            }
         }

         for ( int i = 0; i < angleLineActors.size(); i++ )
         {
            this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor( angleLineActors[i] );
         }
         this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor( firsAngleLineActor );

         for ( int i = 0; i < angleTextActors.size(); i++ )
         {
            this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor( angleTextActors[i] );
         }

         vtkSmartPointer<vtkTimerCallback2> cb = vtkSmartPointer<vtkTimerCallback2>::New();
         cb->timerVolumeActor = volumeActors[volumeCounter];

         timerTag = this->Interactor->AddObserver( vtkCommand::TimerEvent, cb );
         timerID = this->Interactor->CreateRepeatingTimer( timerDuration );
         
         timerStopped = false;
      }
   }

   /*
   *   Increase the timer Duration (slow down the animation).
   */
   void increaseTimerDuration()  // These functions kind of don't work... TO-DO for later..
   {
      if (timerDuration < 1500 )
      {
         timerStopped = false;
         stopTimer();

         timerDuration += 100;
         
         startTimer();

         std::cout << "Increased timer duration to: " << timerDuration << std::endl;
      }
   }

   /*
   *   Decrease the timer Duration (speed up the animation).
   */
   void decreaseTimerDuration()
   {
      if (timerDuration > 100 )
      {
         timerStopped = false;
         stopTimer();

         timerDuration -= 100;

         startTimer();

         std::cout << "Decreased timer duration to: " << timerDuration << std::endl;
      }
   }

   void drawLines()
   {
      // If we have two points, connect them with a line (they should be stored sequentially)
      if ( pointCounter % 2 == 0 )
      {
         // Draw a line between each set of points
         for ( int i = 1; i < points.size(); i = i + 2 )
         {
            vtkSmartPointer<vtkLineSource> lineSource = vtkSmartPointer<vtkLineSource>::New();

            double tempArray1[3] = { points[pointCounter - i]->GetCenter()[0], points[pointCounter - i]->GetCenter()[1], points[pointCounter - i]->GetCenter()[2] }; 
            double tempArray2[3] = { points[pointCounter - (i+1)]->GetCenter()[0], points[pointCounter - (i+1)]->GetCenter()[1], points[pointCounter - (i+1)]->GetCenter()[2] }; 

            lineSource->SetPoint1( tempArray1 );
            lineSource->SetPoint2( tempArray2 );
            lineSource->Update();

            // Calculate the distance between points
            double squaredDistance = vtkMath::Distance2BetweenPoints(tempArray1, tempArray2);
            double distance = sqrt(squaredDistance);

            std::cout << "Length between points: " << "[" << points[pointCounter - 1]->GetCenter()[0] << ", " << points[pointCounter - 1]->GetCenter()[1]
                     << ", " << points[pointCounter - 1]->GetCenter()[2] << "] and [" << points[pointCounter - 2]->GetCenter()[0] << ", "  << points[pointCounter - 2]->GetCenter()[1]
                     << ", " << points[pointCounter - 2]->GetCenter()[2] << "] is: " << distance << std::endl;

            // Transform each new line onto all other volumes
            std::vector< vtkSmartPointer<vtkPolyData> > tempTLines;
            std::vector< vtkSmartPointer<vtkActor> >    tempTLineActors;

            for ( int i = 0; i < icpRegistrations.size(); i++ )
            {
               vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
               transformFilter->SetInputConnection( lineSource->GetOutputPort() );
               transformFilter->SetTransform( icpRegistrations[i] );
               transformFilter->Update();

               tempTLines.push_back( transformFilter->GetOutput() );

               //Create a mapper and actor for the transformed sphere
               vtkSmartPointer<vtkPolyDataMapper> tMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
               tMapper->SetInputConnection( transformFilter->GetOutputPort() );

               vtkSmartPointer<vtkActor> tActor = vtkSmartPointer<vtkActor>::New();
               tActor->SetMapper( tMapper );

               tActor->GetProperty()->SetColor(1.0, 0.0, 0.0);
               tActor->GetProperty()->SetLineWidth(5);

               tempTLineActors.push_back( tActor );
            }

            // tLines.push_back( tempTLines );
            tLineActors.push_back( tempTLineActors );
            
            this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor( tLineActors[lineCounter][0] );
            
            lineCounter++;
         }
      }
      else
      {
         std::cout << "Not enough points to draw a line! Need an even number of points. Please place another point!" << std::endl;
      }
   }

   virtual void OnRightButtonDown() 
   {
      // std::cout << "Picking pixel: " << this->Interactor->GetEventPosition()[0] << " " << this->Interactor->GetEventPosition()[1] << std::endl;
      
      this->Interactor->GetPicker()->Pick( this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1], 
                        0,  // always zero.
                        this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer() );
      
      double picked[3];
      
      this->Interactor->GetPicker()->GetPickPosition( picked );

      //Create a sphere
      vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
      sphereSource->SetCenter( picked[0], picked[1], picked[2] );
      sphereSource->SetRadius( 1.0 );

      // Check if the picked point is inside the surface
      vtkSmartPointer<vtkSelectEnclosedPoints> selectEnclosedPoints = vtkSmartPointer<vtkSelectEnclosedPoints>::New();
      selectEnclosedPoints->SetInputData( sphereSource->GetOutput() );
      selectEnclosedPoints->SetSurfaceData( dicomVolumes[volumeCounter] );
      selectEnclosedPoints->Update();
      selectEnclosedPoints->Initialize( dicomVolumes[volumeCounter] );

      // if ( selectEnclosedPoints->FunctionValue( picked ) <= 0 )
      if ( selectEnclosedPoints->IsInsideSurface( picked ) )
      {
         //  std::cout << "Picked point: " << picked[0] << " " << picked[1] << " " << picked[2] << " is inside the surface" << std::endl;

         points.push_back( sphereSource->GetOutput() );

         // Transform each new point onto all other volumes
         std::vector< vtkSmartPointer<vtkPolyData> > tempTPoints;
         std::vector< vtkSmartPointer<vtkActor> >    tempTPointActors;

         for ( int i = 0; i < icpRegistrations.size(); i++ )
         {
            vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
            transformFilter->SetInputConnection( sphereSource->GetOutputPort() );
            transformFilter->SetTransform( icpRegistrations[i] );
            transformFilter->Update();

            tempTPoints.push_back( transformFilter->GetOutput() );

            //Create a mapper and actor for the transformed sphere
            vtkSmartPointer<vtkPolyDataMapper> tMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
            tMapper->SetInputConnection( transformFilter->GetOutputPort() );

            vtkSmartPointer<vtkActor> tActor = vtkSmartPointer<vtkActor>::New();
            tActor->SetMapper( tMapper );

            tActor->GetProperty()->SetColor(1.0, 0.0, 0.0);

            tempTPointActors.push_back( tActor );
         }

         tPoints.push_back( tempTPoints );
         tPointActors.push_back( tempTPointActors );

         this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor( tPointActors[pointCounter][0] );

         pointCounter++;
      }
      else
      {
         std::cout << "Picked point: " << picked[0] << " " << picked[1] << " " << picked[2] << " is outside the surface" << std::endl;
      }      

      // Forward events
      vtkInteractorStyleTrackballCamera::OnRightButtonDown();      
   }

   void printDataToScreen()
   {
      std::cout << std::endl;
      std::cout << "*****************************************************************************";
      std::cout << std::endl;
      std::cout << "--------------------------Printing computation time---------------------------";
      std::cout << std::endl;
      std::cout << "Time to read, process, and render is: " << compTime << " seconds";
      std::cout << std::endl;

      std::cout << std::endl;
      std::cout << "PRINTING DATA FOR FRAME " << volumeCounter;
      std::cout << std::endl;

      // Print distance between points
      if ( tPoints.empty() )
      {
         std::cout << std::endl;
         std::cout << "------------------Printing point locations at current frame------------------";
         std::cout << std::endl;
         std::cout << "No point data to print!";
         std::cout << std::endl;
      }
      else
      {         
         std::cout << std::endl;
         std::cout << "------------------Printing point locations at current frame------------------";
         std::cout << std::endl;
         std::cout << "Printing point locations at current frame";
         std::cout << std::endl;
         for ( int i = tPoints.size() - 1; i >= 0 ; i-- )
         {
            double p[3] = { tPoints[i][volumeCounter]->GetCenter()[0], tPoints[i][volumeCounter]->GetCenter()[1], tPoints[i][volumeCounter]->GetCenter()[2] };
            std::cout << "Point #" << i << " is located at: [" << p[0] << ", " << p[1] << ", " << p[2] << "]";
            std::cout << std::endl;
         }
         std::cout << std::endl;

         std::cout << "--------------Printing distance between points at current frame--------------";
         std::cout << std::endl;
         for ( int i = tPoints.size() - 1; i > 0; i -= 2 )
         {
            double tempArray1[3] = { tPoints[i][volumeCounter]->GetCenter()[0], tPoints[i][volumeCounter]->GetCenter()[1], tPoints[i][volumeCounter]->GetCenter()[2] }; 
            double tempArray2[3] = { tPoints[i - 1][volumeCounter]->GetCenter()[0], tPoints[i - 1][volumeCounter]->GetCenter()[1], tPoints[i - 1][volumeCounter]->GetCenter()[2] }; 

            // Calculate the distance between points
            double squaredDistance = vtkMath::Distance2BetweenPoints(tempArray1, tempArray2);
            double distance = sqrt(squaredDistance);

            std::cout << "Distance between point #" << i << " and point #" << i + 1 << " is: " << distance;
            std::cout << std::endl;
         }
         std::cout << std::endl;
      }

      // Print angular information
      if ( angles.empty() )
      {
         std::cout << "-------------Printing angle data between current frame and fram 0--------------";
         std::cout << std::endl;
         std::cout << "No angle data to print!";
         std::cout << std::endl;
      }
      else
      {
         std::cout << "-------------Printing angle data between current frame and fram 0--------------";
         std::cout << std::endl;

         std::ostringstream strs;
         strs << angles[volumeCounter];
         std::string str = strs.str();

         std::cout << "Angle between frame #0 and frame #" << volumeCounter << " is: " << str;
         std::cout << std::endl;
         std::cout << std::endl;
      }

      std::cout << "******************************************************************************";
      std::cout << std::endl;
   }

   void printDataToFile()
   {
      // Print distance between points

      // Print angular information
   }

   void drawAngle()
   {  
      // Transform the first angle line to all other volumes
      for ( int i = 0; i < icpRegistrations.size(); i++ )
      {
         vtkSmartPointer<vtkLineSource> lineSource = vtkSmartPointer<vtkLineSource>::New();
         lineSource->SetPoint1( firstAngleLine->GetPoint1() );
         lineSource->SetPoint2( firstAngleLine->GetPoint2() );
         lineSource->Update();

         vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
         transformFilter->SetInputConnection( lineSource->GetOutputPort() );
         transformFilter->SetTransform( icpRegistrations[i] );
         transformFilter->Update();

         double* bounds = transformFilter->GetOutput()->GetBounds();
         double tempArray1[3] = { ( ( bounds[0] + bounds[1] ) /2 ), bounds[2], bounds[4] }; 
         double tempArray2[3] = { ( ( bounds[0] + bounds[1] ) /2 ), bounds[3], bounds[5]  };

         vtkSmartPointer<vtkLineSource> tLineSource = vtkSmartPointer<vtkLineSource>::New();
         tLineSource->SetPoint1( tempArray1 );
         tLineSource->SetPoint2( tempArray2 );
         tLineSource->Update();

         angleLines.push_back( tLineSource );

         //Create a mapper and actor for the transformed line
         vtkSmartPointer<vtkPolyDataMapper> lineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
         lineMapper->SetInputConnection( transformFilter->GetOutputPort() );

         vtkSmartPointer<vtkActor> lineActor = vtkSmartPointer<vtkActor>::New();
         lineActor->SetMapper( lineMapper );

         lineActor->GetProperty()->SetColor(1.0, 0.0, 1.0);
         lineActor->GetProperty()->SetLineWidth(6);

         angleLineActors.push_back( lineActor );
      }

      // Get the intersection point between the first angle line and the current line
      for ( int i = 0; i < angleLines.size(); i++ )
      {    
         double p0[3] = { angleLines[i]->GetPoint1()[0], angleLines[i]->GetPoint1()[1], angleLines[i]->GetPoint1()[2] };
         double p1[3] = { angleLines[i]->GetPoint2()[0], angleLines[i]->GetPoint2()[1], angleLines[i]->GetPoint2()[2] };

         vtkSmartPointer<vtkOBBTree> tree = vtkSmartPointer<vtkOBBTree>::New();
         tree->SetDataSet( cubeLine->GetOutput() );
         tree->BuildLocator();

         vtkSmartPointer<vtkPoints> intersectPoints = vtkSmartPointer<vtkPoints>::New();
         tree->IntersectWithLine( p0, p1, intersectPoints, NULL);

         double intersection[3];

         if ( intersectPoints->GetNumberOfPoints() >= 1)
         {
            intersectPoints->GetPoint(0, intersection);
         }

         // std::cout << "Intersection of volume  " << i << ": " << intersection[0] << ", " << intersection[1] << ", " << intersection[2] << std::endl;

         // Calculate the angle between the current volume and the first volume
         double p3[3] = { angleLines[i]->GetPoint1()[0], firstAngleLine->GetPoint1()[1], firstAngleLine->GetPoint1()[2] };

         double angle = calculateAngle(p0, intersection, p3);

         angles.push_back(angle);

         // Convert the angle to a string and display it on the render window
         std::ostringstream strs;
         strs << angle;
         std::string str = "Angle: " + strs.str();

         angleText = vtkSmartPointer<vtkVectorText>::New();
         angleText->SetText( str.c_str() );
         angleText->Update();

         // Create a mapper and actor
         vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
         mapper->SetInputConnection( angleText->GetOutputPort() );

         vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
         actor->SetMapper(mapper);
         actor->GetProperty()->SetColor(1.0, 0.0, 0.0);
         actor->SetPosition(85.0, 25.0, 35.0);

         angleTextActors.push_back(actor);
      }

      this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor( angleLineActors[volumeCounter] );
      this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor( angleTextActors[volumeCounter] );
   }

   virtual void OnKeyDown()
   {
      std::string key = this->GetInteractor()->GetKeySym();

      if ( key.compare("z") == 0 )
      {
         startTimer();
      }
      else if ( key.compare("x") == 0 )
      {
         stopTimer();
      }
      else if ( key.compare("Down") == 0 ) 
      {
         increaseTimerDuration();
      }
      else if ( key.compare("Up") == 0 ) 
      {
         decreaseTimerDuration();
      }
      else if ( key.compare("d") == 0 )
      {
         drawLines();
      }
      else if ( key.compare("r") == 0 )
      {
         resetPointPicking();
      }
      else if ( key.compare("n") == 0 )
      {
         printDataToScreen();
      }
      else if ( key.compare("m") == 0 )
      {
         printDataToFile();
      }
      else if ( key.compare("a") == 0 )
      {
         drawAngle();
      }

      // Forward events
      vtkInteractorStyle::OnKeyDown();
   }

   // Function to calculate the angle in degrees between two lines
   // Note that p2 is the centre point (common between both lines)
   // The angle is found by taking the inverse cosine of the dot product between the normalized vectors
   double calculateAngle( double p1[3], double p2[3], double p3[3] )
   {
      double vec1[3], vec2[3];

      vec1[0] = p1[0] - p2[0];
      vec1[1] = p1[1] - p2[1];
      vec1[2] = p1[2] - p2[2];
      vec2[0] = p3[0] - p2[0];
      vec2[1] = p3[1] - p2[1];
      vec2[2] = p3[2] - p2[2];
      
      vtkMath::Normalize( vec1 );
      vtkMath::Normalize( vec2 );
     
      double angle = acos( vtkMath::Dot( vec1, vec2 ) );

      angle = vtkMath::DegreesFromRadians(angle);

      return angle;
      
      // std::cout << "angle: " << vtkMath::DegreesFromRadians(angle) << std::endl;
   }
};

#endif  // INTERACTORSTYLER_H