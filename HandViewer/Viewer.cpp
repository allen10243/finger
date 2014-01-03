/*******************************************************************************
*                                                                              *
*   PrimeSense NiTE 2.0 - Hand Viewer Sample                                   *
*   Copyright (C) 2012 PrimeSense Ltd.                                         *
*                                                                              *
*******************************************************************************/
#include <stdlib.h>
#include <iostream>
#include <map>
#include "Viewer.h"
#include <Windows.h>
#include <math.h>



#if (ONI_PLATFORM == ONI_PLATFORM_MACOSX)
        #include <GLUT/glut.h>
#else
        #include <GL/glut.h> 
        #include <GL/GLU.h> 
#endif

#include "HistoryBuffer.h"
#include <NiteSampleUtilities.h>
#include <windows.h>   /* required before including mmsystem.h */
#include <mmsystem.h>  /* multimedia functions (such as MIDI) for Windows */
#include "RtMIDI\RtMidi.h"

using namespace std;

#define GL_WIN_SIZE_X	1280
#define GL_WIN_SIZE_Y	960  //1024
#define TEXTURE_SIZE	512
#define PI 3.141592653589793

#define DEFAULT_DISPLAY_MODE	DISPLAY_MODE_DEPTH

#define MIN_NUM_CHUNKS(data_size, chunk_size)	((((data_size)-1) / (chunk_size) + 1))
#define MIN_CHUNKS_SIZE(data_size, chunk_size)	(MIN_NUM_CHUNKS(data_size, chunk_size) * (chunk_size))

SampleViewer* SampleViewer::ms_self = NULL;

std::map<int, HistoryBuffer<20> *> g_histories;

bool g_drawDepth = true;
bool g_smoothing = false;
bool g_drawFrameId = false;

std::vector<unsigned char> midiMessage;
int g_nXRes = 0, g_nYRes = 0;
RtMidiOut *midiout = 0;
int volume=0;
int lastkey=0;

int last_xmax = 0;
int last_xmin = 0;
int last_ymin = 0;
int last_ymx = 0;


openni::VideoStream streamDepth;
nite::HandTrackerFrameRef handFrame;
openni::VideoFrameRef depthFrame;

void SampleViewer::glutIdle()
{
	glutPostRedisplay();
}

void SampleViewer::glutDisplay()
{
	SampleViewer::ms_self->Display();
}
void SampleViewer::glutDisplay2()
{
	SampleViewer::ms_self->Display2();
}
void SampleViewer::glutKeyboard(unsigned char key, int x, int y)
{
	SampleViewer::ms_self->OnKey(key, x, y);
}

SampleViewer::SampleViewer(const char* strSampleName)
{
	ms_self = this;
	strncpy(m_strSampleName, strSampleName, ONI_MAX_STR);
	m_pHandTracker = new nite::HandTracker;
}
SampleViewer::~SampleViewer()
{
	Finalize();

	delete[] m_pTexMap;

	ms_self = NULL;
}

void SampleViewer::Finalize()
{
	delete m_pHandTracker;
	nite::NiTE::shutdown();
	openni::OpenNI::shutdown();
}

openni::Status SampleViewer::Init(int argc, char **argv)
{
	m_pTexMap = NULL;

	openni::OpenNI::initialize();

	const char* deviceUri = openni::ANY_DEVICE;
	for (int i = 1; i < argc-1; ++i)
	{
		if (strcmp(argv[i], "-device") == 0)
		{
			deviceUri = argv[i+1];
			break;
		}
	}

	openni::Status rc = m_device.open(deviceUri);
	if (rc != openni::STATUS_OK)
	{
		printf("Open Device failed:\n%s\n", openni::OpenNI::getExtendedError());
		return rc;
		
	}

	nite::NiTE::initialize();

	if (m_pHandTracker->create(&m_device) != nite::STATUS_OK)
	{
		return openni::STATUS_ERROR;
	}

	m_pHandTracker->startGestureDetection(nite::GESTURE_WAVE);
	m_pHandTracker->startGestureDetection(nite::GESTURE_CLICK);

	return InitOpenGL(argc, argv);

}
openni::Status SampleViewer::Run()	//Does not return
{
	glutMainLoop();
	return openni::STATUS_OK;
}

float Colors[][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 1}};
int colorCount = 3;
float SpiderNet[39];
float Current_X,Current_Y,Current_Z;
int w1,w2;


float Calculate_Distance(float x1, float y1, float x2, float y2)
{
	float X = (x1-x2)*(x1-x2);
 	float Y = (y1-y2)*(y1-y2);
	return sqrt(X+Y);
}
int Calculate_NoteKey(float x1,float y1)
{
	float Hypotenuse = Calculate_Distance(x1 ,y1 , 640, 512);     //斜邊
	float Subtense = 512.0 - y1;                                  //對邊
	float result = asin(Subtense/Hypotenuse)*180.0/PI;
	if(x1 > 640.0)
	   result=result+90;   //return result;
	else
	   result=180.0-result+90; //return 180.0-result;

	if(Hypotenuse>200)
	{
		if(result > 0.0 && result <30.0){
			if( Calculate_Distance(x1 ,y1 , 640, 512) < 400.0)
				return 48;
			else
				return 60;
		}  
		if(result > 30.0 && result<60.0){
			if( Calculate_Distance(x1 ,y1 , 640, 512) < 400.0)
				return 49;
			else
				return 61;
		}
		if(result > 60.0 && result<90.0){
			if( Calculate_Distance(x1 ,y1 , 640, 512) < 400.0)
				return 50;
			else
				return 62;
		}
		if(result > 90.0 && result<120.0){
			if( Calculate_Distance(x1 ,y1 , 640, 512) < 400.0)
				return 51;
			else
				return 63;
		}
		
		if(result > 120.0 && result<150.0){
			if( Calculate_Distance(x1 ,y1 , 640, 512) < 400.0)
				return 52;
			else
				return 64;
		}
		
		if(result > 150.0 && result<180.0){
			if( Calculate_Distance(x1 ,y1 , 640, 512) < 400.0)
				return 53;
			else
				return 65;
		}	
		
		if(result > 180.0 && result<210.0){
			if( Calculate_Distance(x1 ,y1 , 640, 512) < 400.0)
				return 54;
			else
				return 66;
		}
		
		if(result > 210.0 && result<240.0){
			if( Calculate_Distance(x1 ,y1 , 640, 512) < 400.0)
				return 55;
			else
				return 67;
		}
		
		if(result > 240.0 && result<270.0){
			if( Calculate_Distance(x1 ,y1 , 640, 512) < 400.0)
				return 56;
			else
				return 68;
		}
		if(result > 270.0 && result<300.0){
			if( Calculate_Distance(x1 ,y1 , 640, 512) < 400.0)
				return 57;
			else
				return 69;
		}
		
		if(result > 300.0 && result<330.0){
			if(Calculate_Distance(x1 ,y1 , 640, 512) < 400.0)
				return 58;
			else
				return 70;
		}
		if(result > 330.0 && result<360.0){
			if( Calculate_Distance(x1 ,y1 , 640, 512) < 400.0)
				return 59;
			else
				return 71;
		}
		return 0;
	}
	else
		return 0;
}

void DrawHistory(nite::HandTracker* pHandTracker, int id, HistoryBuffer<20>* pHistory)
{
	glColor3f(Colors[id % colorCount][0], Colors[id % colorCount][1], Colors[id % colorCount][2]);
	float coordinates[60] = {0};
	float factorX = GL_WIN_SIZE_X / (float)g_nXRes;
	float factorY = GL_WIN_SIZE_Y / (float)g_nYRes;

	//for (int i = 0; i < pHistory->GetSize(); ++i)
	//{
    	const nite::Point3f& position = pHistory->operator[](0);		
		pHandTracker->convertHandCoordinatesToDepth(position.x, position.y, position.z, &coordinates[0], &coordinates[1]);
		coordinates[0] *= factorX;
		coordinates[1] *= factorY;

	//}

	Current_X = coordinates[0];
	Current_Y = coordinates[1];

	glPointSize(18);
	glVertexPointer(3, GL_FLOAT, 0, coordinates);
	glDrawArrays(GL_POINTS, 0, 1);

	//int key =0;

	//if(coordinates[0] !=0 && coordinates[1]!=0)
	//{
 //      key = Calculate_NoteKey(coordinates[0],coordinates[1]);
	//}

	//if(key!=0 )
	//{
	//	midiMessage.push_back (144) ;
	//	midiMessage.push_back (key);  //key of note
	//	midiMessage.push_back (volume);  //velocity
	//	midiout->sendMessage( &midiMessage );		
	//	midiMessage.clear();
 //       lastkey = key;
	//	
	//	for(int note= key-1;note<=key+1;note++)
	//	{
	//		if(note!=key)
	//		{
	//			midiMessage.push_back (128) ;
	//			midiMessage.push_back (note);  //key of note
	//			midiMessage.push_back (volume);  //velocity
	//			midiout->sendMessage( &midiMessage );
	//			midiMessage.clear();
	//		}
	//	}

	//}
	//else
	//{
	//	for(int note= lastkey-1;note<=lastkey+1;note++)
	//	{
	//		if(note!=lastkey)
	//		{
	//		    midiMessage.push_back (128) ;
	//			midiMessage.push_back (note);  //key of note
	//			midiMessage.push_back (volume);  //velocity
	//			midiout->sendMessage( &midiMessage );
	//			midiMessage.clear();
	//		}
	//	}	
	//}

 
}

float * getCoordinate(double angle, double radius)
{
	static float position[2];

	int x,y;
	if(angle >= 0  && angle <= 90.0){
		x = radius*sin(angle*PI/180.0);
		y = sqrt(radius*radius - x*x); 
		position[0] = 640 + x;
		position[1] = 512 + y;
	}
	else if(angle > 90.0 && angle <=180.0){
		double newAngle = angle - 90.0;
		y = radius*sin(newAngle*PI/180.0);
		x = sqrt(radius*radius - y*y);
		position[0] = 640 + x;
		position[1] = 512 - y;
	}
	else if(angle > 180.0 && angle <= 270.0){
		double newAngle = angle - 180.0;
		x = radius*sin(newAngle*PI/180.0);
		y = sqrt(radius*radius - x*x); 
		position[0] = 640 - x;
		position[1] = 512 - y;
	}
	else if(angle > 270.0 && angle <= 360.0){
		double newAngle = angle - 270.0;
		y = radius*sin(newAngle*PI/180.0);
		x = sqrt(radius*radius - y*y); 
		position[0] = 640 - x;
		position[1] = 512 + y;
	}

    return position;

}
void DrawSpiderNet()
{
	float *p;
	for(int i=0;i<=11;i++)
	{   
		p = getCoordinate(i*30.0,450.0);
		SpiderNet[i*3] = *p;
		SpiderNet[i*3+1] = *(p + 1);
		SpiderNet[i*3+2] = 0;
		//printf("point %d, x=%f  , y=%f  \n",i,SpiderNet[i*3],SpiderNet[i*3+1]);
	}
	SpiderNet[36]=SpiderNet[0];
	SpiderNet[37]=SpiderNet[1];
	SpiderNet[38]=SpiderNet[2];

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA );
	glColor4f(1.0f, 0.0f, 0.0f, 0.5);
	glLineWidth(5);
	glVertexPointer(3, GL_FLOAT, 0, SpiderNet);
	glDrawArrays(GL_LINE_STRIP, 0, 13);
	glDisable( GL_BLEND ); 

	glColor3f(1.0f, 0.0f, 0.0f);
	glPointSize(12);
	glVertexPointer(3, GL_FLOAT, 0, SpiderNet);
	glDrawArrays(GL_POINTS, 0, 12);



	for(int i=0;i<=11;i++)
	{   
		p = getCoordinate(i*30.0,250.0);
		SpiderNet[i*3] = *p;
		SpiderNet[i*3+1] = *(p + 1);
		SpiderNet[i*3+2] = 0;
		//printf("point %d, x=%f  , y=%f  \n",i,SpiderNet[i*3],SpiderNet[i*3+1]);
	}
	SpiderNet[36]=SpiderNet[0];
	SpiderNet[37]=SpiderNet[1];
	SpiderNet[38]=SpiderNet[2];

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA );
	glColor4f(1.0f, 0.0f, 0.0f, 0.5);
	glLineWidth(5);
	glVertexPointer(3, GL_FLOAT, 0, SpiderNet);
	glDrawArrays(GL_LINE_STRIP, 0, 13);
	glDisable( GL_BLEND ); 

	glColor3f(1.0f, 0.0f, 0.0f);
	glPointSize(12);
	glVertexPointer(3, GL_FLOAT, 0, SpiderNet);
	glDrawArrays(GL_POINTS, 0, 12);


}


#ifndef USE_GLES
void glPrintString(void *font, const char *str)
{
	int i,l = (int)strlen(str);

	for(i=0; i<l; i++)
	{   
		glutBitmapCharacter(font,*str++);
	}   
}
#endif
void DrawFrameId(int frameId)
{
	char buffer[80] = "";
	sprintf(buffer, "%d", frameId);
	glColor3f(1.0f, 0.0f, 0.0f);
	glRasterPos2i(20, 20);
	glPrintString(GLUT_BITMAP_HELVETICA_18, buffer);
}


void SampleViewer::Display()
{
	//nite::HandTrackerFrameRef handFrame;
	//openni::VideoFrameRef depthFrame;
	nite::Status rc = m_pHandTracker->readFrame(&handFrame);
	if (rc != nite::STATUS_OK)
	{
		printf("GetNextData failed\n");
		return;
	}

	depthFrame = handFrame.getDepthFrame();

	if (m_pTexMap == NULL)
	{
		// Texture map init
		m_nTexMapX = MIN_CHUNKS_SIZE(depthFrame.getVideoMode().getResolutionX(), TEXTURE_SIZE);
		m_nTexMapY = MIN_CHUNKS_SIZE(depthFrame.getVideoMode().getResolutionY(), TEXTURE_SIZE);
		m_pTexMap = new openni::RGB888Pixel[m_nTexMapX * m_nTexMapY];
	}


	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, GL_WIN_SIZE_X, GL_WIN_SIZE_Y, 0, -1.0, 1.0);

	if (depthFrame.isValid())
	{
		calculateHistogram(m_pDepthHist, MAX_DEPTH, depthFrame);
	}

	memset(m_pTexMap, 0, m_nTexMapX*m_nTexMapY*sizeof(openni::RGB888Pixel));

	float factor[3] = {1, 1, 1};
	// check if we need to draw depth frame to texture
	if (depthFrame.isValid() && g_drawDepth)
	{
		const openni::DepthPixel* pDepthRow = (const openni::DepthPixel*)depthFrame.getData();
		openni::RGB888Pixel* pTexRow = m_pTexMap + depthFrame.getCropOriginY() * m_nTexMapX;
		int rowSize = depthFrame.getStrideInBytes() / sizeof(openni::DepthPixel);

		for (int y = 0; y < depthFrame.getHeight(); ++y)
		{
			const openni::DepthPixel* pDepth = pDepthRow;
			openni::RGB888Pixel* pTex = pTexRow + depthFrame.getCropOriginX();

			for (int x = 0; x < depthFrame.getWidth(); ++x, ++pDepth, ++pTex)
			{
				if (*pDepth != 0)
				{
					factor[0] = Colors[colorCount][0];
					factor[1] = Colors[colorCount][1];
					factor[2] = Colors[colorCount][2];

					int nHistValue = m_pDepthHist[*pDepth];
					pTex->r = nHistValue*factor[0];
					pTex->g = nHistValue*factor[1];
					pTex->b = nHistValue*factor[2];

					factor[0] = factor[1] = factor[2] = 1;
				}
			}

			pDepthRow += rowSize;
			pTexRow += m_nTexMapX;
		}
	}

	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_nTexMapX, m_nTexMapY, 0, GL_RGB, GL_UNSIGNED_BYTE, m_pTexMap);

	// Display the OpenGL texture map
	glColor4f(1,1,1,1);

	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);

	g_nXRes = depthFrame.getVideoMode().getResolutionX();
	g_nYRes = depthFrame.getVideoMode().getResolutionY();

	// upper left
	glTexCoord2f(0, 0);
	glVertex2f(0, 0);
	// upper right
	glTexCoord2f((float)g_nXRes/(float)m_nTexMapX, 0);
	glVertex2f(GL_WIN_SIZE_X, 0);
	// bottom right
	glTexCoord2f((float)g_nXRes/(float)m_nTexMapX, (float)g_nYRes/(float)m_nTexMapY);
	glVertex2f(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
	// bottom left
	glTexCoord2f(0, (float)g_nYRes/(float)m_nTexMapY);
	glVertex2f(0, GL_WIN_SIZE_Y);

	glEnd();
	glDisable(GL_TEXTURE_2D);

	DrawSpiderNet();

	const nite::Array<nite::GestureData>& gestures = handFrame.getGestures();
	for (int i = 0; i < gestures.getSize(); ++i)
	{
		if (gestures[i].isComplete())
		{
			const nite::Point3f& position = gestures[i].getCurrentPosition();
			printf("Gesture %d at (%f,%f,%f)\n", gestures[i].getType(), position.x, position.y, position.z);

			nite::HandId newId;
			m_pHandTracker->startHandTracking(gestures[i].getCurrentPosition(), &newId);
		}
	}

	const nite::Array<nite::HandData>& hands= handFrame.getHands();
	for (int i = 0; i < hands.getSize(); ++i)
	{
		const nite::HandData& user = hands[i];

		if (!user.isTracking())
		{
			printf("Lost hand %d\n", user.getId());
			nite::HandId id = user.getId();
			HistoryBuffer<20>* pHistory = g_histories[id];
			g_histories.erase(g_histories.find(id));
			delete pHistory;
		}
		else
		{
			if (user.isNew())
			{
				printf("Found hand %d\n", user.getId());
				g_histories[user.getId()] = new HistoryBuffer<20>;
			}
			// Add to history
			HistoryBuffer<20>* pHistory = g_histories[user.getId()];
			pHistory->AddPoint(user.getPosition());
			// Draw history
		    DrawHistory(m_pHandTracker, user.getId(), pHistory);
		   
		}
	}

	if (g_drawFrameId)
	{
		DrawFrameId(handFrame.getFrameIndex());
	}
	// Swap the OpenGL display buffers
	glutSwapBuffers();
	glutSetWindow(w2);
	
}
void SampleViewer::Display2()
{  
	
	int center_x = 0;
	int center_y = 0;
	int center_z = 700;

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPointSize(1);
	glColor3f(1,1,1);
	glBegin(GL_POINTS);

	int x_max=-1;
	int x_max_y=0;
	int x_min =640;
	int x_min_y = 0;
	int y_max=-1;
	int y_max_x=0;
	int y_min=480;
	int y_min_x=0;

	if (depthFrame.isValid() /*&& g_drawDepth*/)
	{
		const openni::DepthPixel* pDepthRow = (const openni::DepthPixel*)depthFrame.getData();
		const openni::DepthPixel* pDepth2 = pDepthRow;
		int rowSize = depthFrame.getStrideInBytes() / sizeof(openni::DepthPixel);
		for (int y = 0; y < 480; ++y)
		{
			const openni::DepthPixel* pDepth = pDepthRow;
			for (int x = 0; x < 640; ++x, ++pDepth)
			{
				if (*pDepth != 0)
				{
					if(*pDepth <700){              /////////////////深度threshold
					   glColor3f(1,1,1);
					   glVertex3f(x,y,0);	
					   if(x>x_max){
						   if(noise_filter(last_xmax,x))
						   {
							   x_max = x;
							   x_max_y = y;
						   }
					   }
					   if(x<x_min){
						   if(noise_filter(last_xmin,x))
						   {
							   x_min=x;
							   x_min_y=y;
						   }
					   }
					   if(y>y_max){
						   y_max=y;
						   y_max_x=x;
					   }
					   if(y<y_min)
					   {
						  if(noise_filter(last_ymin,y))
						  {
							   y_min=y;
							   y_min_x=x;
						  }
					   }
					   if(*pDepth < center_z)
					   {
						   center_x = x;
						   center_y = y;
						   center_z = *pDepth;
					   }   
					}
				}
			}
			pDepthRow += rowSize;
		}
		glEnd();


		glPointSize(12);
		glBegin(GL_POINTS);
		glColor3f(1,0,0);
		glVertex3f(center_x,center_y,0);	
		glEnd();

		//////////////////////////////////////////////////////////////draw coordinate line
		glBegin(GL_LINE_STRIP);
		glColor3f(1,0,0);
		glVertex3f(0,0,0);
		glVertex3f(0,1000,0);

		glColor3f(0,1,0);
		glVertex3f(0,0,0);
		glVertex3f(1000,0,0);
		
		glColor3f(0,0,1);
		glVertex3f(0,0,0);
		glVertex3f(0,0,1000);
		
		glEnd();

		glPointSize(12);
		glBegin(GL_POINTS);
		glColor3f(1,0,1);
		glVertex3f(Current_X/2,Current_Y/2,0);
		glEnd();

	    ////////////////////////////////////////////////////////////////  angle calculation
	    //printf ("%f\n",atan2(y_min-x_min_y,x_min-y_min_x) * 180 / 3.1415-45);

		//////////////////////////////////////////////////////////////draw hand box
		glEnd();
		glBegin(GL_LINE_STRIP);
		glColor3f(1,1,0);
		glVertex3f(x_max,y_min,0);
		glVertex3f(x_min,y_min,0);
		glVertex3f(x_min,y_min,0);
		glVertex3f(x_min,y_max,0);
		glVertex3f(x_min,y_max,0);
		glVertex3f(x_max,y_max,0);
		glVertex3f(x_max,y_max,0);
		glVertex3f(x_max,y_min,0);
		glEnd();
		///////////////////////////////////////////////////////////////////////////

		finger_play_sound(x_max, x_max_y, x_min, x_min_y, y_max, y_max_x, y_min, y_min_x,  center_x, center_y);
		
		last_xmax= x_max;
		last_xmin= x_min;
		last_ymin= y_min;
		last_ymx =  y_min_x;
	}

	glutSwapBuffers();
	glutSetWindow(w1);
}

void handleView(int w,int h)
{   
	
	glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glOrtho(0, GL_WIN_SIZE_X, GL_WIN_SIZE_Y, 0, -1.0, 1.0);
    gluPerspective( /* field of view in degree */ 50.0,
    /* aspect ratio */ 1.0,
    /* Z near */ 0.1, /* Z far */ 100000);
    glMatrixMode(GL_MODELVIEW);
    //gluLookAt(1000, 1000, 500, 0.0,0.0,0.0, 0.0, 0.0,1.0); 
	gluLookAt(300, 1000, 800, 300,500,0, 0.0, 0.0,1.0);     
     /* eye is at () * /* center is at (0,0,0) */ /* up is in positive Z direction */
}
void SampleViewer::OnKey(unsigned char key, int /*x*/, int /*y*/)
{
	switch (key)
	{
	case 27:
		Finalize();
		exit (1);
	case 'd':
		g_drawDepth = !g_drawDepth;
		break;
	case 's':
		if (g_smoothing)
		{
			// Turn off smoothing
			m_pHandTracker->setSmoothingFactor(0);
			g_smoothing = FALSE;
		}
		else
		{
			m_pHandTracker->setSmoothingFactor(0.1);
			g_smoothing = TRUE;
		}
		break;
	case 'f':
		// Draw frame ID
		g_drawFrameId = !g_drawFrameId;
		break;
	}

}

openni::Status SampleViewer::InitOpenGL(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
	w1 = glutCreateWindow (m_strSampleName);
	// 	glutFullScreen();
	glutSetCursor(GLUT_CURSOR_NONE);
	InitOpenGLHooks();
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

////////////////////////////////////////////////////////////////////////////////

	glutInitWindowSize(400,400);
	w2 = glutCreateWindow("Second Window");
    glutPositionWindow(1300,80);
    // register callbacks for second window, which is now current
	glutReshapeFunc(handleView);
    glutDisplayFunc(glutDisplay2);

	return openni::STATUS_OK;
}
void SampleViewer::InitOpenGLHooks()
{
	glutKeyboardFunc(glutKeyboard);
	glutDisplayFunc(glutDisplay);
    glutIdleFunc(glutIdle);
}
void initMIDI(){

    int i, keyPress;
    int nPorts;
    char input;

    midiout = new RtMidiOut();
	// Check available ports.
    nPorts = midiout->getPortCount();
    if ( nPorts == 0 ) {
        cout << "No ports available!" << endl;
    }
        // List Available Ports
    cout << "\nPort Count = " << nPorts << endl;
    cout << "Available Output Ports\n-----------------------------\n";
    for( i=0; i<nPorts; i++ )
    {
        try {
                cout << "  Output Port Number " << i << " : " << midiout->getPortName(i) << endl;
        }
        catch(RtError &error) {
            error.printMessage();
        }
    }

	cout << "\n The choosen port is 1" << endl;
        //cin >> keyPress;

        // Open Selected Port
        midiout->openPort(0);

        keyPress = NULL;
}
bool noise_filter(int last, int current)
{
	if(abs(last-current)<40 || last == 0)
		return true;
	else
		return false;
	
}

void finger_play_sound(int x_max,int x_max_y,int x_min, int x_min_y, int y_max, int y_max_x, int y_min, int y_min_x, int depth_x, int depth_y)
{
	float a = sqrt(float((x_max-depth_x)*(x_max-depth_x))+float((x_max_y-depth_y)*(x_max_y-depth_y)));
	float b = sqrt(float((x_min-depth_x)*(x_min-depth_x))+float((x_min_y-depth_y)*(x_min_y-depth_y)));
	float c = sqrt(float((y_max-depth_y)*(y_max-depth_y))+float((y_max_x-depth_x)*(y_max_x-depth_x)));
	float d = sqrt(float((y_min-depth_y)*(y_min-depth_y))+float((y_min_x-depth_x)*(y_min_x-depth_x)));
	
	if( depth_x ==0 && depth_y ==0)
	{
		lastkey = 0;
		return;
	}
	else if(a<10 )////////////////////////////小拇指
	{
		if(lastkey!=1)
		{
			midiMessage.push_back (144) ;
			midiMessage.push_back (76);  //key of note
			midiMessage.push_back (90);  //velocity
			midiout->sendMessage( &midiMessage );		
			midiMessage.clear();
			printf("小拇指\n");
			lastkey=1;
		}
	}
	else if(b<5 )//////////////////////////大拇指
	{
		if(lastkey!= 2)
		{
			midiMessage.push_back (144) ;
			midiMessage.push_back (60);  //key of note
			midiMessage.push_back (90);  //velocity
			midiout->sendMessage( &midiMessage );		
			midiMessage.clear();
			printf("大拇指\n");
			lastkey=2;
		}
	}
	else if(x_min<depth_x && depth_x<y_min_x  )///////////////////////////食指
	{
		if(lastkey!=3){
			if( y_min< depth_y && depth_y< x_min_y){
				midiMessage.push_back (144) ;
				midiMessage.push_back (64);  //key of note
				midiMessage.push_back (90);  //velocity
				midiout->sendMessage( &midiMessage );		
				midiMessage.clear();
				printf("食指\n");
				lastkey=3;
			}
		}
	
	}

	else if(abs(y_min_x - last_ymx)>10 && last_ymin < y_min ) /////////////////////////中指
	{
		if(lastkey!=4)
		{
			if(depth_x<y_min_x && depth_x<x_max ){
				midiMessage.push_back (144) ;
				midiMessage.push_back (67);  //key of note
				midiMessage.push_back (90);  //velocity
				midiout->sendMessage( &midiMessage );		
				midiMessage.clear();
				printf("中指\n");
				lastkey=4;
			}

			if(depth_x>y_min_x && depth_x>x_min){
				midiMessage.push_back (144) ;
				midiMessage.push_back (67);  //key of note
				midiMessage.push_back (90);  //velocity
				midiout->sendMessage( &midiMessage );		
				midiMessage.clear();
				printf("中指\n");
				lastkey=4;
			}
		}
	}
	else if(y_min_x< depth_x &&  depth_x<x_max )///////////////////////////無名指
	{
		if(lastkey!=5)
		{
			if(y_min< depth_y && depth_y< x_min_y){
				midiMessage.push_back (144) ;
				midiMessage.push_back (72);  //key of note
				midiMessage.push_back (90);  //velocity
				midiout->sendMessage( &midiMessage );		
				midiMessage.clear();
				printf("無名指\n");
				lastkey=5;
			}
		}
	}
	
	
}