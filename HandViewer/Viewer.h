/*******************************************************************************
*                                                                              *
*   PrimeSense NiTE 2.0 - Hand Viewer Sample                                   *
*   Copyright (C) 2012 PrimeSense Ltd.                                         *
*                                                                              *
*******************************************************************************/

#ifndef _NITE_HAND_VIEWER_H_
#define _NITE_HAND_VIEWER_H_

#include "NiTE.h"

#define MAX_DEPTH 10000

class SampleViewer
{
public:
	SampleViewer(const char* strSampleName);
	virtual ~SampleViewer();

	virtual openni::Status Init(int argc, char **argv);
	virtual openni::Status Run();	//Does not return
	
protected:
	virtual void Display();
	virtual void Display2();
	virtual void DisplayPostDraw(){};	// Overload to draw over the screen image

	virtual void OnKey(unsigned char key, int x, int y);

	virtual openni::Status InitOpenGL(int argc, char **argv);
	
	void InitOpenGLHooks();
    void initMIDI();
	void Finalize();

private:
	SampleViewer(const SampleViewer&);
	SampleViewer& operator=(SampleViewer&);
	static SampleViewer* ms_self;
	static void glutIdle();
	static void glutDisplay();
	static void glutDisplay2();
	static void glutKeyboard(unsigned char key, int x, int y);

	float				m_pDepthHist[MAX_DEPTH];
	char			m_strSampleName[ONI_MAX_STR];
	openni::RGB888Pixel*		m_pTexMap;
	unsigned int		m_nTexMapX;
	unsigned int		m_nTexMapY;

	openni::Device		m_device;
	nite::HandTracker* m_pHandTracker;
};

void initMIDI();
bool noise_filter(int last, int current);
void finger_play_sound(int x_max,int x_max_y,int x_min, int x_min_y, int y_max, int y_max_x, int y_min, int y_min_x, int depth_x, int depth_y);
#endif // _NITE_HAND_VIEWER_H_
