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
      cb->TimerCount = 0;
      return cb;
    }

    virtual void Execute( vtkObject *caller, unsigned long eventId, void * vtkNotUsed(callData) )
    {
      if ( volumeCounter <= 0 )
      {
        forward = true;
        reverse = false;
      }
      else if ( volumeCounter >= 4 )
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
      
      iren->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor( actor );
      actor = volumeActors[volumeCounter];

    if (forward)
    {
      for ( int i = 0; i < pointActors.size(); i++ )
      {
          iren->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor( pointActors[i] );
      }

      for ( int i = 0; i < pointActors.size(); i++ )
      {
          vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
          transformFilter->SetInputData( vtkPolyData::SafeDownCast( pointActors[i]->GetMapper()->GetInput() ) );
          transformFilter->SetTransform( tMatricies[i] );
          transformFilter->Update();

          vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
          mapper->SetInputData( transformFilter->GetOutput() );

          vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
          actor->SetMapper( mapper );

          pointActors[i] = actor;

          iren->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor( pointActors[i] );
      }
    }
    else if (reverse)
    {
      for ( int i = pointActors.size()-1; i >= 0; i-- )
      {
          iren->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveActor( pointActors[i] );
      }

      for ( int i = pointActors.size()-1; i >= 0; i-- )
      {
          vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
          transformFilter->SetInputData( vtkPolyData::SafeDownCast( pointActors[i]->GetMapper()->GetInput() ) );
          transformFilter->SetTransform( tMatricies[i] );
          transformFilter->Update();

          vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
          mapper->SetInputData( transformFilter->GetOutput() );

          vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
          actor->SetMapper( mapper );

          pointActors[i] = actor;

          iren->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor( pointActors[i] );
      }
    }

      iren->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor( actor );

      iren->GetRenderWindow()->Render();
    }

  private:
    int TimerCount;
  public:
    vtkActor* actor;
    vtkActor* pointActor;
};

#endif  // TIMERCALLBACK_H