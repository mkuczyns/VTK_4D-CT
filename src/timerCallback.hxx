/****************************************************************************
*   timerCallback.hxx
*
*   Created by:     Michael Kuczynski
*   Created on:     03/04/2019
*   Description:    Custom timer callback class
*****************************************************************************
*   TO-DO:
*
****************************************************************************/
#ifndef TIMERCALLBACK_H
#define TIMERCALLBACK_H

#include "vtk4DCT.hxx"

class vtkTimerCallback2 : public vtkCommand
{
  public:
    static vtkTimerCallback2 *New()
    {
      vtkTimerCallback2 *cb = new vtkTimerCallback2;
      return cb;
    }

    virtual void Execute( vtkObject *caller, unsigned long eventId, void * vtkNotUsed(callData) )
    {
      if ( volumeCounter <= 0 )
      {
        forward = true;
        reverse = false;
      }
      else if ( volumeCounter >= 79 )
      {
        reverse = true;
        forward = false;
      }
      
      if ( forward )
      {
        volumeCounter++;
      }
      else if ( reverse )
      {
        volumeCounter--;
      }
      
      vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::SafeDownCast(caller);
      
      /* 
      *   Remove current actors
      */

      // Volumes
      iren->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor( timerVolumeActor );
      timerVolumeActor = volumeActors[volumeCounter];

      // Angle lines
      if ( angleLineActors.size() != 0 )
      {
        iren->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor( timerAngleLineActor );
        timerAngleLineActor = angleLineActors[volumeCounter];
      }

      if ( angleTextActors.size() != 0 )
      {
        iren->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor( timerAngleTextActor );
        timerAngleTextActor = angleTextActors[volumeCounter];
      }

      // Points
      if ( timerPointActor.size() == 0 && tPointActors.size() != 0 )
      {
        timerPointActor.push_back( tPointActors[0][0] );
      }

      for ( int i = 0; i < timerPointActor.size(); i++ )
      {
        iren->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor( timerPointActor[i] );
      }
      timerPointActor.clear();

      for ( int i = 0; i < tPointActors.size(); i++ )
      {
        timerPointActor.push_back( tPointActors[i][volumeCounter] );
      }

      // Lines
      if ( timerLineActor.size() == 0 && tLineActors.size() != 0 )
      {
        timerLineActor.push_back( tLineActors[0][0] );
      }

      for ( int i = 0; i < timerLineActor.size(); i++ )
      {
        iren->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor( timerLineActor[i] );
      }
      timerLineActor.clear();

      for ( int i = 0; i < tLineActors.size(); i++ )
      {
        timerLineActor.push_back( tLineActors[i][volumeCounter] );
      }

      /* 
      *   Add next volume actors
      */
      iren->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor( timerVolumeActor );

      if ( angleLineActors.size() != 0 )
      {
        iren->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor( timerAngleLineActor );
        iren->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor( timerAngleTextActor );
      }

      for ( int i = 0; i < timerPointActor.size(); i++ )
      {
        iren->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor( timerPointActor[i] );
      }

      for ( int i = 0; i < timerLineActor.size(); i++ )
      {
        iren->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor( timerLineActor[i] );
      }

      iren->GetRenderWindow()->Render();
    }

  private:

  public:
    vtkSmartPointer<vtkActor> timerVolumeActor;
    vtkSmartPointer<vtkActor> timerAngleLineActor;
    vtkSmartPointer<vtkActor> timerAngleTextActor;

    std::vector< vtkSmartPointer<vtkActor> > timerPointActor;
    std::vector< vtkSmartPointer<vtkActor> > timerLineActor;
};

#endif  // TIMERCALLBACK_H