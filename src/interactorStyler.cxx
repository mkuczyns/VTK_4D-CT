/****************************************************************************
*   interactorStyler.cxx
*
*   Created by:     Michael Kuczynski
*   Created on:     19/01/2019
*   Description:    Custom interactor styling implementation.
****************************************************************************/

#include "interactorStyler.hxx"

void myInteractorStyler::setRenderWindow( vtkRenderWindow* renderWindow )
{
    _RenderWindow = renderWindow;
}

void myInteractorStyler::setTimer( vtkRenderWindowInteractor* renderWindowInteractor )
{
    _RenderWindowInteractor = renderWindowInteractor;
    _TimerDuration = renderWindowInteractor->GetTimerDuration();
}

void myInteractorStyler::increaseTimerDuration()
{
    _RenderWindowInteractor->SetTimerDuration( _TimerDuration + 100 );
    _RenderWindow->Render();
}

void myInteractorStyler::decreaseTimerDuration()
{
    _RenderWindowInteractor->SetTimerDuration( _TimerDuration - 100 );
    _RenderWindow->Render();
}

void myInteractorStyler::resetPointPicking()
{
    
}