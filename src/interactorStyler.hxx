/****************************************************************************
*   interactorStyler.hxx
*
*   Created by:     Michael Kuczynski
*   Created on:     19/01/2019
*   Description:    Custom interactor styling definition.
****************************************************************************/

#ifndef INTERACTORSTYLER_H
#define INTERACTORSTYLER_H

#include "helperFunctions.hxx"

#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkActor2D.h>

#include <vtkImageViewer2.h>
#include <vtkDICOMImageReader.h>
#include <vtkNIFTIImageReader.h>
#include <vtkInteractorStyleImage.h>

#include <vtkTextProperty.h>
#include <vtkTextMapper.h>
#include <vtkImageMapToWindowLevelColors.h>

#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>

#include <string>

/* 
*   A class for a custom interactor style to override the default interactor style.
*/
class myInteractorStyler : public vtkInteractorStyleImage
{
public:
   static myInteractorStyler* New();

   vtkTypeMacro( myInteractorStyler, vtkInteractorStyleImage );

   void setRenderWindow( vtkRenderWindow* renderWindow );

   void setTimer( vtkRenderWindowInteractor* renderWindowInteractor );

protected:
   vtkImageMapper*            _ImageMapper;
   vtkRenderWindow*           _RenderWindow;
   vtkRenderWindowInteractor* _RenderWindowInteractor;
   unsigned int               _TimerDuration;    // timer duration in ms

   /*
   *   Reset the picked points (maximum 2 points at one time).
   */
   void resetPointPicking();

   /*
   *   Increase the timer Duration (slow down the animation).
   */
   void increaseTimerDuration();

   /*
   *   Decrease the timer Duration (speed up the animation).
   */
   void decreaseTimerDuration();

   /*
   *   Overload the default interactor event listener for key presses.
   *   UP ARROW key      = increase timer duration
   *   DOWN ARROW key    = decrease timer duration
   *   R Key             = reset picked points
   *   RIGHT Mouse Click = Set point on object
   */
   virtual void OnKeyDown()
   {
      std::string key = this->GetInteractor()->GetKeySym();

      if ( key.compare("r") == 0 ) 
      {
         resetPointPicking();
      }
      else if ( key.compare("Up") == 0 ) 
      {
         increaseTimerDuration();
      }
      else if ( key.compare("Down") == 0 ) 
      {
         decreaseTimerDuration();
      }

      vtkInteractorStyleImage::OnKeyDown();
   }

   /*
   *  Overload the default interactor event listener for the left mouse button.
   */
   // virtual void OnRightButtonDown()  { }
};

#endif  // INTERACTORSTYLER_H