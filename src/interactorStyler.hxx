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
      if ( pointCounter <= 0 )
      {
         std::cout << "No points to remove!\n";
      }
      else
      {
         /* Reset the point picking with the following steps:
         *   1. Remove all point and line actors
         *   2. Clear the point and line vectors
         *   3. Reset point and line counts
         *   4. Clear the distance vector
         */
         for ( int i = 0; i < pointActors.size(); i++ )
         {
            this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor( pointActors[i] );
         }

         for ( int j = 0; j < lineActors.size(); j++ )
         {
            this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor( lineActors[j] );
         }

         pointCounter = 0;
         lineCounter = 0;

         pointActors.clear();
         lineActors.clear();
         points.clear();
         distances.clear();
      }
   }

   void stopTimer()
   {
      this->Interactor->DestroyTimer( timerID );
      this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor( volumeActors[volumeCounter] );
      volumeCounter = 0; 
   }

   void startTimer()
   {
      vtkSmartPointer<vtkTimerCallback2> cb = vtkSmartPointer<vtkTimerCallback2>::New();
      cb->actor = volumeActors[0];
      this->Interactor->AddObserver( vtkCommand::TimerEvent, cb );
      timerID = this->Interactor->CreateRepeatingTimer( timerDuration );
   }

   /*
   *   Increase the timer Duration (slow down the animation).
   */
   void increaseTimerDuration()  // These functions kind of don't work... TO-DO for later..
   {
      if (timerDuration < 1500 )
      {
         timerDuration += 100;
         this->Interactor->SetTimerDuration( timerDuration );
      }
   }

   /*
   *   Decrease the timer Duration (speed up the animation).
   */
   void decreaseTimerDuration()
   {
      if (timerDuration > 100 )
      {
         timerDuration -= 100;
         this->Interactor->SetTimerDuration( timerDuration );
      }
   }

      void pointRegistration()
   {

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

            double tempArray1[3] = { points[pointCounter - i][0], points[pointCounter - i][1], points[pointCounter - i][2] }; 
            double tempArray2[3] = { points[pointCounter - (i+1)][0], points[pointCounter - (i+1)][1], points[pointCounter - (i+1)][2] }; 

            lineSource->SetPoint1( tempArray1 );
            lineSource->SetPoint2( tempArray2 );
            lineSource->Update();

            //Create a mapper and actor for the line
            vtkSmartPointer<vtkPolyDataMapper> lineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
            lineMapper->SetInputConnection(lineSource->GetOutputPort());

            vtkSmartPointer<vtkActor> lineActor = vtkSmartPointer<vtkActor>::New();
            lineActor->SetMapper(lineMapper);
            lineActors.push_back( lineActor );
            
            this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor( lineActors[lineCounter] );
            
            lineCounter++;

            // Calculate the distance between points
            double squaredDistance = vtkMath::Distance2BetweenPoints(tempArray1, tempArray2);
            double distance = sqrt(squaredDistance);

            std::cout << "Length between points: " << "[" << points[pointCounter - 1][0] << ", " << points[pointCounter - 1][1]
                     << ", " << points[pointCounter - 1][2] << "] and [" << points[pointCounter - 2][0] << ", "  << points[pointCounter - 2][1]
                     << ", " << points[pointCounter - 2][2] << "] is: " << distance << std::endl;
         }
      }
      else
      {
         std::cout << "Not enough points to draw a line! Need an even number of points. Please place another point!" << std::endl;
      }
   }

   virtual void OnRightButtonDown() 
   {
      std::cout << "Picking pixel: " << this->Interactor->GetEventPosition()[0] << " " << this->Interactor->GetEventPosition()[1] << std::endl;
      
      this->Interactor->GetPicker()->Pick( this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1], 
                        0,  // always zero.
                        this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer() );
      
      double picked[3];
      
      this->Interactor->GetPicker()->GetPickPosition( picked );

      // Check if the picked point is inside the surface
      vtkSmartPointer<vtkImplicitPolyDataDistance> selectEnclosedPoints = vtkSmartPointer<vtkImplicitPolyDataDistance>::New();
      selectEnclosedPoints->SetInput( dicomVolumes[volumeCounter] );

      if ( selectEnclosedPoints->FunctionValue( picked ) <= 0 )
      {
         std::cout << "Picked point: " << picked[0] << " " << picked[1] << " " << picked[2] << " is inside the surface" << std::endl;
         
         points.push_back( { picked[0], picked[1], picked[2] } );

         //Create a sphere
         vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
         sphereSource->SetCenter( picked[0], picked[1], picked[2] );
         sphereSource->SetRadius( 1.0 );

         //Create a mapper and actor for the sphere
         vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
         mapper->SetInputConnection( sphereSource->GetOutputPort() );

         vtkSmartPointer<vtkActor> tempActor = vtkSmartPointer<vtkActor>::New();
         tempActor->SetMapper( mapper );
         pointActors.push_back( tempActor );
         this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor( pointActors[pointCounter] );

         pointCounter++;
      }
      else
      {
         std::cout << "Picked point: " << picked[0] << " " << picked[1] << " " << picked[2] << " is outside the surface" << std::endl;
      }      

      // Forward events
      vtkInteractorStyleTrackballCamera::OnRightButtonDown();      
   }

   virtual void OnKeyDown()
   {
      std::string key = this->GetInteractor()->GetKeySym();

      if ( key.compare("r") == 0 ) 
      {
         resetPointPicking();
      }
      else if ( key.compare("p") == 0 )
      {
         stopTimer();
      }
      else if ( key.compare("s") == 0 )
      {
         startTimer();
      }
      else if ( key.compare("d") == 0 )
      {
         drawLines();
      }
      else if ( key.compare("x") == 0 )
      {
         pointRegistration();
      }
      else if ( key.compare("Up") == 0 ) 
      {
         increaseTimerDuration();
      }
      else if ( key.compare("Down") == 0 ) 
      {
         decreaseTimerDuration();
      }

      // Forward events
      vtkInteractorStyle::OnKeyDown();
   }
};

#endif  // INTERACTORSTYLER_H