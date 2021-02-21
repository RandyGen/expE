#pragma once


#include <tchar.h>
#include <windows.h>
#define D3D_DEBUG_INFO
#include <stdlib.h>
#include <math.h>
#include <d3dx9.h>
#include <XAudio2.h>
#include <vector>
#include <iostream>

#include "../include/WindowManager.h"
#include "../include/ars.h"
#include "arstest.h"


//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------

using namespace std;

void subtract_mask(Texture* result, Texture* bg, Texture* src, DWORD border);
const int sizex = 640; 
const int sizey = 480;

float time = 0;
float limit = 0.6f;
bool go_back = true;
bool left_right = true;
bool stop = true;
float theta = PI/2;


UINT MainLoop(WindowManager *winmgr)
{
	ShowDebugWindow();

	//for debug(1/2): �ʂ̃E�C���h�E�ɏ����r���̉�f���A�����ł͏d�Ȃ蕔����\��
	//Window window2;
	//winmgr->RegisterWindow(&window2);

	//ARSG arsgd(window2.hWnd, sizex, sizey, true);
	//Texture2D debug(&arsgd, sizex, sizey);
	
	Window window;
	winmgr->RegisterWindow(&window);

	ARSG g{ window.hWnd, sizex, sizey, true };
	g.SetBackgroundColor(255,0,0,0);

	Light light{ &g };
	g.Register(&light );

	ARSD d;
	d.Init();
	d.AttachCam(0);
	d.StartGraph();
	
	Texture hitArea{ &g, sizex, sizey };//�d�Ȃ蕔��
	Texture stored{ &g, sizex, sizey };//�w�i
	Texture2D source{ &g, sizex, sizey };//�ŐV�t���[��

	g.Register(&source);

	Touchable doll{ &g, L"doll3.x" };
	doll.SetScale(1.0f, 1.0f, 1.0f); // 1,1,1
	doll.SetPosition(-4.0f, -2.0f, 0.0f, GL_ABSOLUTE); // -4, -2
	g.Register(&doll);

	InputHandler *keyIn = window.GetInputHandler();

	
	
	while(!d.GetCamImage(&stored));

	while (!winmgr->WaitingForTermination()) {
		if (keyIn->GetKeyTrig('A'))
			d.GetCamImage(&stored);
		d.GetCamImage(&source);
		if (keyIn->GetKeyTrig('Q')) break;

		subtract_mask(&hitArea, &stored, &source, 0x20202020);
		
		doll.react(&hitArea); //�d�Ȃ�̈�hitArea�ɉ���

		//for debug(2/2)
		//debug = hitArea;
		//arsgd.Draw(&debug);
		
		g.Draw();
		
	}
	d.StopGraph();
	return 0;
}

//�w�i����
inline void subtract_mask(Texture* result, Texture* backgrnd, Texture* src, DWORD border)
{
	ARSC::diff(result,backgrnd,src,border);  //��
	ARSC::monochrome(result,result);         //���m�N�����iRGB����P�xY�ɕϊ��j
	ARSC::thresholding(result,result,border);//�Q�l��
}


//hitArea�̔��̈�Əd�Ȃ�̈悪����΂��̗̈�̏d�S�i��f�̈ʒu�̕��ϒl�j�������i�̎Q�Ɛ�Ɂj�ɐݒ肵�Atrue��Ԃ�
inline bool Touchable::checkOverlapping(Texture* hitArea, float *pGx, float *pGy, unsigned int threshold)
{	
	static Texture txtr;//��Ɨp�e�N�X�`���I�u�W�F�N�g�i�P���ė��p�j
	ARSG* g = GetARSG(); //�����V�[��g���擾
	txtr.Init(g,sizex,sizey);//�w��p�����[�^��(��)������

	unsigned int pixel_count;//�ʐϊi�[�p

	g->Draw(this,&txtr); //���̃I�u�W�F�N�g(this)����Ɨp�e�N�X�`��txtr�ɕ`��
	ARSC::and(&txtr, &txtr, hitArea, 0x10101010);//txtr := txtr��hitArea�łƂ���臒l�𒴂��镔��(�`�����l������)��0xff
	                                       // 0xff002020(�X�C�J=��) & 0xffffffff(hitArea=��) => 0xff00ffff(���F) 

	ARSC::getCG(pGx, pGy, &pixel_count, &txtr);	//txtr�̔�[����f�̈ʒu�̕��ϒl�ƌ�
	return pixel_count > threshold;
}

inline void Touchable::react(Texture* _hitArea)
{
	float gx, gy;
	bool overlapping = checkOverlapping(_hitArea, &gx, &gy, 100);//��̊֐���`�Q��

	//VECTOR2D c;
	//GetARSG()->Convert3Dto2D(&c, GetPosition());//���̃I�u�W�F�N�g���X�N���[����ɉf����Wc�ɕϊ�


	switch (state) {
		case OUT_TOUCH://�u����Ă���v�ƋL�^����Ă���
			if (overlapping) {// IN
				state = IN_TOUCH;//�L�^��ύX
				stop = false;
			}
		case IN_TOUCH://�u�d�Ȃ��Ă���v�ƋL�^����Ă���
			if (!overlapping) // OUT
				state = OUT_TOUCH;
		break;
	default:
		break;
	}

	if (go_back && left_right && !stop) {
		move(1);
		time += 0.01f;
		if (time > limit) {
			go_back = false;
			limit -= 0.075f;
		}
	}
	else if (!go_back && left_right && !stop) {
		move(-1);
		time -= 0.01f;
		if (time < 0) {
			go_back = true;
			left_right = false;
		}
	}
	else if (go_back && !left_right && !stop) {
		move(-1);
		time -= 0.01f;
		if (time < -limit) {
			go_back = false;
			limit -= 0.075f;
		}
	}
	else if (!go_back && !left_right && !stop) {
		move(1);
		time += 0.01f;
		if (time > 0) {
			go_back = true;
			left_right = true;
		}
	}

	if (limit < 0.005f && abs(time) < 0.02f) {
		stop = true;
		limit = 0.6f;
	}
}


inline void Touchable::move(int turn)
{
	float r = 5.0f;
	SetRotationZ(turn*0.025f);
	SetPosition(r*cos(theta) - 4, r*sin(theta) - 5.0f - 2, 0.0f, GL_ABSOLUTE);
	theta += turn*0.025f;
		
}


int APIENTRY _tWinMain(HINSTANCE hInstance
	, HINSTANCE //hPrevInstance
	, LPTSTR //lpCmdLine
	, int //nCmdShow
)
{
	WindowManager program(hInstance, &MainLoop);
#ifdef DEBUG
    MessageBox(NULL,L"OK?",TEXT(APPNAME), NULL);
#endif
    return 0;
}
