/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkWin32OglrRenderWindow.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    to Horst Schreiber for developing this MFC code

Copyright (c) 1993-1996 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <afxwin.h>				// hsr
#include <GL/glaux.h>
#include "vtkWin32OglrRenderWindow.h"
#include "vtkWin32RenderWindowInteractor.h"
#include "vtkOglrRenderer.h"
#include "vtkOglrProperty.h"
#include "vtkOglrTexture.h"
#include "vtkOglrCamera.h"
#include "vtkOglrActor.h"
#include "vtkOglrLight.h"
#include "vtkOglrPolyMapper.h"

#define MAX_LIGHTS 8

vtkWin32OglrRenderWindow::vtkWin32OglrRenderWindow()
{
  this->ContextId = 0;
  this->MultiSamples = 8;
  this->WindowId = 0;
  this->NextWindowId = 0;
  this->DeviceContext = (HDC)0;		// hsr
  this->MFChandledWindow = FALSE;	// hsr

  if ( this->WindowName )
    delete [] this->WindowName;
  this->WindowName = strdup("Visualization Toolkit - Win32OpenGL");
}

// Description:
// Create a OpenGL specific actor.
vtkActorDevice *vtkWin32OglrRenderWindow::MakeActor()
{
  vtkOglrActor *actor;

  actor = new vtkOglrActor;
  return (vtkActorDevice *)actor;
}

// Description:
// Create a OpenGL specific light.
vtkLightDevice *vtkWin32OglrRenderWindow::MakeLight()
{
  vtkOglrLight *light;

  light = new vtkOglrLight;
  return (vtkLightDevice *)light;
}

// Description:
// Create a OpenGL specific renderer.
vtkRenderer *vtkWin32OglrRenderWindow::MakeRenderer()
{
  vtkOglrRenderer *ren;

  ren = new vtkOglrRenderer;
  this->AddRenderers(ren);

  // by default we are its parent
  ren->SetRenderWindow((vtkRenderWindow*)this);
  
  return (vtkRenderer *)ren;
}

// Description:
// Create a OpenGL specific camera.
vtkCameraDevice *vtkWin32OglrRenderWindow::MakeCamera()
{
  vtkOglrCamera *camera;

  camera = new vtkOglrCamera;
  return (vtkCameraDevice *)camera;
}

// Description:
// Create a OpenGL specific property.
vtkPropertyDevice *vtkWin32OglrRenderWindow::MakeProperty()
{
  vtkOglrProperty *property;

  property = new vtkOglrProperty;
  return (vtkPropertyDevice *)property;
}

// Description:
// Create a OpenGL specific PolyMapper.
vtkPolyMapperDevice *vtkWin32OglrRenderWindow::MakePolyMapper()
{
  vtkOglrPolyMapper *PolyMapper;

  PolyMapper = new vtkOglrPolyMapper;
  return (vtkPolyMapperDevice *)PolyMapper;
}

// Description:
// Create a OpenGL specific texture.
vtkTextureDevice *vtkWin32OglrRenderWindow::MakeTexture()
{
  vtkOglrTexture *texture;

  texture = new vtkOglrTexture;
  return (vtkTextureDevice *)texture;
}

// Description:
// Begin the rendering process.
void vtkWin32OglrRenderWindow::Start(void)
{
  // if the renderer has not been initialized, do so now
  if (!this->ContextId)
    {
    this->Initialize();
    }

  // set the current window 
  this->MakeCurrent();
}

void vtkWin32OglrRenderWindow::MakeCurrent()
{
  static HDC vtkCurrentDC = NULL;
  static HWND vtkCurrentWindow = NULL;

  // free the old context if any
  if (vtkCurrentDC)
    {
    ReleaseDC(vtkCurrentWindow, vtkCurrentDC);
    vtkCurrentDC = NULL;
    }

  if (!this->MFChandledWindow)
    {
    vtkCurrentWindow = this->WindowId;
    vtkCurrentDC = GetDC(this->WindowId);
		wglMakeCurrent(vtkCurrentDC, this->ContextId);
    }
  else 
    {
    wglMakeCurrent(this->DeviceContext, this->ContextId);
    }
}

static void vtkWin32OglrSwapBuffers(HDC hdc)
{
  SwapBuffers(hdc);
}

// Description:
// End the rendering process and display the image.
void vtkWin32OglrRenderWindow::Frame(void)
{
  glFlush();
  if (this->DoubleBuffer)
    {
    vtkWin32OglrSwapBuffers(wglGetCurrentDC());
    vtkDebugMacro(<< " SwapBuffers\n");
    }
}
 

// Description:
// Update system if needed due to stereo rendering.
void vtkWin32OglrRenderWindow::StereoUpdate(void)
{
  // no stereo right now
}

// Description:
// Specify various window parameters.
void vtkWin32OglrRenderWindow::WindowConfigure()
{
  // this is all handles by the desiredVisualInfo method
}

// Description:
// Initialize the window for rendering.
void vtkWin32OglrRenderWindow::WindowInitialize (void)
{
  int x, y, width, height;
  GLenum type;
  static int count = 1;
  
  x = ((this->Position[0] >= 0) ? this->Position[0] : 5);
  y = ((this->Position[1] >= 0) ? this->Position[1] : 5);
  width = ((this->Size[0] > 0) ? this->Size[0] : 300);
  height = ((this->Size[1] > 0) ? this->Size[1] : 300);

  // create our own window if not already set
  this->OwnWindow = 0;
  if (!this->MFChandledWindow) {  // hsr 
	  if (!this->WindowId)
		{
                if( this->WindowName )
                  delete [] this->WindowName;
                int len = strlen( "Visualization Toolkit - Win32OpenGL #") 
                          + (int)ceil( (double) log10( (double)(count+1) ) )
                          + 1; 
                this->WindowName = new char [ len ];
		sprintf(this->WindowName,"Visualization Toolkit - Win32OpenGL #%i",count++);

		auxInitPosition(x, y, width, height);
		type = AUX_DEPTH16 | AUX_RGB | AUX_DOUBLE;
		auxInitDisplayMode(type);

	    if (auxInitWindow(this->WindowName) == GL_FALSE) 
        {
		  auxQuit();
		}

		this->WindowId = auxGetHWND();
		this->ContextId = auxGetHGLRC();
		this->OwnWindow = 1;
		}

	  wglMakeCurrent(GetDC(this->WindowId), this->ContextId);
  }	else wglMakeCurrent(this->DeviceContext, this->ContextId); // hsr

  vtkDebugMacro(<< " glMatrixMode ModelView\n");
  glMatrixMode( GL_MODELVIEW );

  vtkDebugMacro(<< " zbuffer enabled\n");
  glDepthFunc( GL_LEQUAL );
  glEnable( GL_DEPTH_TEST );

  vtkDebugMacro(" texture stuff\n");
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

  // initialize blending for transparency
  vtkDebugMacro(<< " blend func stuff\n");
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  glEnable( GL_NORMALIZE );
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

	if(!this->MFChandledWindow) this->Mapped = 1;
}

// Description:
// Initialize the rendering window.
void vtkWin32OglrRenderWindow::Initialize (void)
{
  // make sure we havent already been initialized 
  if (this->ContextId)
    return;

  // now initialize the window 
  this->WindowInitialize();
}


// Description:
// Get the current size of the window.
int *vtkWin32OglrRenderWindow::GetSize(void)
{
  RECT rect;

  // if we aren't mapped then just return the ivar 
  if (!this->Mapped)
    {
    return(this->Size);
    }

  //  Find the current window size 
  GetClientRect(this->WindowId, &rect);

  this->Size[0] = rect.right;
  this->Size[1] = rect.bottom;
  
  return this->Size;
}

// Get the current size of the window.
int *vtkWin32OglrRenderWindow::GetScreenSize(void)
{
  RECT rect;

  // if we aren't mapped then just return the ivar 
  if (!this->Mapped)
    {
    return(this->Size);
    }

  //  Find the current window size 
  GetClientRect(this->WindowId, &rect);

  this->Size[0] = rect.right;
  this->Size[1] = rect.bottom;
  
  return this->Size;
}

// Description:
// Get the position in screen coordinates of the window.
int *vtkWin32OglrRenderWindow::GetPosition(void)
{
  // if we aren't mapped then just return the ivar 
  if (!this->Mapped)
    {
    return(this->Position);
    }

  //  Find the current window position 
//  x,y,&this->Position[0],&this->Position[1],&child);

  return this->Position;
}

// Description:
// Change the window to fill the entire screen.
void vtkWin32OglrRenderWindow::SetFullScreen(int arg)
{
  int *temp;
  
  if (this->FullScreen == arg) return;
  
  if (!this->Mapped)
    {
    this->PrefFullScreen();
    return;
    }

  // set the mode 
  this->FullScreen = arg;
  if (this->FullScreen <= 0)
    {
    this->Position[0] = this->OldScreen[0];
    this->Position[1] = this->OldScreen[1];
    this->Size[0] = this->OldScreen[2]; 
    this->Size[1] = this->OldScreen[3];
    this->Borders = this->OldScreen[4];
    }
  else
    {
    // if window already up get its values 
    if (this->WindowId)
      {
      //  Find the current window size 
//      XGetWindowAttributes(this->DisplayId, 
//			   this->WindowId, &attribs);
      
//      this->OldScreen[2] = attribs.width;
//      this->OldScreen[3] = attribs.height;;

      temp = this->GetPosition();      
      this->OldScreen[0] = temp[0];
      this->OldScreen[1] = temp[1];

      this->OldScreen[4] = this->Borders;
      this->PrefFullScreen();
      }
    }
  
  // remap the window 
  this->WindowRemap();

  this->Modified();
}

// Description:
// Set the preferred window size to full screen.
void vtkWin32OglrRenderWindow::PrefFullScreen()
{
  int *size;

  size = this->GetScreenSize();

  // use full screen 
  this->Position[0] = 0;
  this->Position[1] = 0;
  this->Size[0] = size[0];
  this->Size[1] = size[1];

  // don't show borders 
  this->Borders = 0;
}

// Description:
// Remap the window.
void vtkWin32OglrRenderWindow::WindowRemap()
{
  short cur_light;

  /* first delete all the old lights */
  for (cur_light = GL_LIGHT0; cur_light < GL_LIGHT0+MAX_LIGHTS; cur_light++)
    {
    glDisable(cur_light);
    }
  
  // then close the old window 
  if (this->OwnWindow)
    {
    SendMessage(this->WindowId, WM_CLOSE, 0, 0L );
    }
  
  // set the default windowid 
  this->WindowId = this->NextWindowId;
  this->NextWindowId = 0;

  // configure the window 
  this->WindowInitialize();
}

// Description:
// Specify the size of the rendering window.
void vtkWin32OglrRenderWindow::SetSize(int x,int y)
{
  // if we arent mappen then just set the ivars 
  if (!this->Mapped)
    {
    if ((this->Size[0] != x)||(this->Size[1] != y))
      {
      this->Modified();
      }
    this->Size[0] = x;
    this->Size[1] = y;
    return;
    }

//  XResizeWindow(this->DisplayId,this->WindowId,x,y);
//  XSync(this->DisplayId,False);
}


void vtkWin32OglrRenderWindow::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkRenderWindow::PrintSelf(os,indent);

  os << indent << "ContextId: " << this->ContextId << "\n";
  os << indent << "Next Window Id: " << this->NextWindowId << "\n";
  os << indent << "Window Id: " << this->WindowId << "\n";
}


unsigned char *vtkWin32OglrRenderWindow::GetPixelData(int x1, int y1, int x2, int y2, 
						 int front)
{
  long     xloop,yloop;
  int     y_low, y_hi;
  int     x_low, x_hi;
  unsigned long   *buffer;
  unsigned char   *data = NULL;
  unsigned char   *p_data = NULL;

  // set the current window 
  this->MakeCurrent();

  buffer = new unsigned long[abs(x2 - x1)+1];
  data = new unsigned char[(abs(x2 - x1) + 1)*(abs(y2 - y1) + 1)*3];

  if (y1 < y2)
    {
    y_low = y1; 
    y_hi  = y2;
    }
  else
    {
    y_low = y2; 
    y_hi  = y1;
    }

  if (x1 < x2)
    {
    x_low = x1; 
    x_hi  = x2;
    }
  else
    {
    x_low = x2; 
    x_hi  = x1;
    }

  if (front)
    {
    glReadBuffer(GL_FRONT);
    }
  else
    {
    glReadBuffer(GL_BACK);
    }
  p_data = data;
  for (yloop = y_hi; yloop >= y_low; yloop--)
    {
    // read in a row of pixels 
    glReadPixels(x_low,yloop,(x_hi-x_low+1),1, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    for (xloop = 0; xloop <= (abs(x2-x1)); xloop++)
      {
      *p_data = (buffer[xloop] & (0x000000ff)); p_data++;
      *p_data = (buffer[xloop] & (0x0000ff00)) >> 8; p_data++;
      *p_data = (buffer[xloop] & (0x00ff0000)) >> 16; p_data++;
      }
    }
  
  delete [] buffer;

  return data;
}

void vtkWin32OglrRenderWindow::SetPixelData(int x1, int y1, int x2, int y2,
					    unsigned char *data, int front)
{
  int     y_low, y_hi;
  int     x_low, x_hi;
  int     xloop,yloop;
  unsigned long   *buffer;
  unsigned char   *p_data = NULL;

  // set the current window
  this->MakeCurrent();

  if (front)
    {
    glDrawBuffer(GL_FRONT);
    }
  else
    {
    glDrawBuffer(GL_BACK);
    }

  buffer = new unsigned long[4*(abs(x2 - x1)+1)];

  if (y1 < y2)
    {
    y_low = y1; 
    y_hi  = y2;
    }
  else
    {
    y_low = y2; 
    y_hi  = y1;
    }
  
  if (x1 < x2)
    {
    x_low = x1; 
    x_hi  = x2;
    }
  else
    {
    x_low = x2; 
    x_hi  = x1;
    }
  
  // now write the binary info one row at a time 
  p_data = data;
  for (yloop = y_hi; yloop >= y_low; yloop--)
    {
    for (xloop = 0; xloop <= (abs(x2-x1)); xloop++)
      {
      buffer[xloop] = 0xff000000;
      buffer[xloop] += (*p_data); p_data++; 
      buffer[xloop] += (*p_data) << 8; p_data++;
      buffer[xloop] += (*p_data) << 16; p_data++;
      }
    /* write out a row of pixels */
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glRasterPos2f( 2.0 * (GLfloat)(x_low) / this->Size[0] - 1, 
                   2.0 * (GLfloat)(yloop) / this->Size[1] - 1);
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();

    glDrawPixels((x_hi-x_low+1),1, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    }
  
  delete [] buffer;
}

// Description:
// Get the window id.
HWND vtkWin32OglrRenderWindow::GetWindowId()
{
  vtkDebugMacro(<< "Returning WindowId of " << this->WindowId << "\n"); 

  return this->WindowId;
}

// Description:
// Set the window id to a pre-existing window.
void vtkWin32OglrRenderWindow::SetWindowId(HWND arg)
{
  vtkDebugMacro(<< "Setting WindowId to " << arg << "\n"); 

  this->WindowId = arg;
}

// Description:
// Set the window id of the new window once a WindowRemap is done.
void vtkWin32OglrRenderWindow::SetNextWindowId(HWND arg)
{
  vtkDebugMacro(<< "Setting NextWindowId to " << arg << "\n"); 

  this->NextWindowId = arg;
}

void vtkWin32OglrRenderWindow::SetContextId(HGLRC arg) // hsr
{													   // hsr	
  this->ContextId = arg;							   // hsr
}													   // hsr

void vtkWin32OglrRenderWindow::SetDeviceContext(HDC arg) // hsr
{														 // hsr
  this->DeviceContext = arg;							 // hsr
  this->MFChandledWindow = TRUE;						 // hsr
}														 // hsr

// Description:
// Create named interactor type
vtkRenderWindowInteractor *
vtkWin32OglrRenderWindow::MakeRenderWindowInteractor()
{
  this->Interactor = 
    (vtkRenderWindowInteractor *)new vtkWin32RenderWindowInteractor;
  this->Interactor->SetRenderWindow((vtkRenderWindow *)this);
  return this->Interactor;
}


float *vtkWin32OglrRenderWindow::GetRGBAPixelData(int x1, int y1, int x2, int y2, int front)
{
  long    xloop,yloop;
  int     y_low, y_hi;
  int     x_low, x_hi;
  int     width, height;

  float   *data = NULL;

  float   *p_data = NULL;
  unsigned long   *buffer;

  // set the current window 
  this->MakeCurrent();

  if (y1 < y2)
    {
    y_low = y1; 
    y_hi  = y2;
    }
  else
    {
    y_low = y2; 
    y_hi  = y1;
    }

  if (x1 < x2)
    {
    x_low = x1; 
    x_hi  = x2;
    }
  else
    {
    x_low = x2; 
    x_hi  = x1;
    }

  if (front)
    {
    glReadBuffer(GL_FRONT);
    }
  else
    {
    glReadBuffer(GL_BACK);
    }

  width  = abs(x_hi - x_low) + 1;
  height = abs(y_hi - y_low) + 1;

  data = new float[ (width*height*4) ];

  glReadPixels( x_low, y_low, width, height, GL_RGBA, GL_FLOAT, data);

  return data;
}

void vtkWin32OglrRenderWindow::SetRGBAPixelData(int x1, int y1, 
						int x2, int y2,
						float *data, int front)
{
  int     y_low, y_hi;
  int     x_low, x_hi;
  int     width, height;
  int     xloop,yloop;
  float   *buffer;
  float   *p_data = NULL;

  // set the current window 
  this->MakeCurrent();

  if (front)
    {
    glDrawBuffer(GL_FRONT);
    }
  else
    {
    glDrawBuffer(GL_BACK);
    }

  if (y1 < y2)
    {
    y_low = y1; 
    y_hi  = y2;
    }
  else
    {
    y_low = y2; 
    y_hi  = y1;
    }
  
  if (x1 < x2)
    {
    x_low = x1; 
    x_hi  = x2;
    }
  else
    {
    x_low = x2; 
    x_hi  = x1;
    }
  
  width  = abs(x_hi-x_low) + 1;
  height = abs(y_hi-y_low) + 1;

  /* write out a row of pixels */
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glRasterPos2f( 2.0 * (GLfloat)(x_low) / this->Size[0] - 1, 
                 2.0 * (GLfloat)(y_low) / this->Size[1] - 1);
  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();
  glMatrixMode( GL_PROJECTION );
  glPopMatrix();

  glDrawPixels( width, height, GL_RGBA, GL_FLOAT, data);

}

float *vtkWin32OglrRenderWindow::GetZbufferData( int x1, int y1, 
						 int x2, int y2  )
{
  int             y_low, y_hi;
  int             x_low, x_hi;
  int             width, height;
  float           *z_data = NULL;

  // set the current window 
  this->MakeCurrent();

  if (y1 < y2)
    {
    y_low = y1; 
    y_hi  = y2;
    }
  else
    {
    y_low = y2; 
    y_hi  = y1;
    }

  if (x1 < x2)
    {
    x_low = x1; 
    x_hi  = x2;
    }
  else
    {
    x_low = x2; 
    x_hi  = x1;
    }

  width =  abs(x2 - x1)+1;
  height = abs(y2 - y1)+1;

  z_data = new float[width*height];

  glReadPixels( x_low, y_low, 
		width, height,
		GL_DEPTH_COMPONENT, GL_FLOAT,
		z_data );

  return z_data;
}

void vtkWin32OglrRenderWindow::SetZbufferData( int x1, int y1, int x2, int y2,
					       float *buffer )
{
  int             y_low, y_hi;
  int             x_low, x_hi;
  int             width, height;

  // set the current window 
  this->MakeCurrent();

  if (y1 < y2)
    {
    y_low = y1; 
    y_hi  = y2;
    }
  else
    {
    y_low = y2; 
    y_hi  = y1;
    }

  if (x1 < x2)
    {
    x_low = x1; 
    x_hi  = x2;
    }
  else
    {
    x_low = x2; 
    x_hi  = x1;
    }

  width =  abs(x2 - x1)+1;
  height = abs(y2 - y1)+1;

  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glRasterPos2f( 2.0 * (GLfloat)(x_low) / this->Size[0] - 1, 
                 2.0 * (GLfloat)(y_low) / this->Size[1] - 1);
  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();
  glMatrixMode( GL_PROJECTION );
  glPopMatrix();

  glDrawPixels( width, height, GL_DEPTH_COMPONENT, GL_FLOAT, buffer);

}

