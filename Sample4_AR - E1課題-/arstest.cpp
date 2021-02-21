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

UINT MainLoop(WindowManager *winmgr)
{
	//ShowDebugWindow();

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

	Touchable ball{ &g, L"ball.x" };
	ball.SetScale(3.0f, 3.0f, 3.0f);
	ball.SetPosition(-6.0f, 4.0f, 0.0f,GL_ABSOLUTE);		
	g.Register(&ball);

	InputHandler *keyIn = window.GetInputHandler();
	
	while(!d.GetCamImage(&stored));

	while (!winmgr->WaitingForTermination()){
		if (keyIn->GetKeyTrig('A'))
			d.GetCamImage(&stored);
		d.GetCamImage(&source);
		if (keyIn->GetKeyTrig('Q')) break;
			
		subtract_mask(&hitArea,&stored,&source,0x20202020);	
		
		ball.react(&hitArea); //�d�Ȃ�̈�hitArea�ɉ���
		//ball.move();          //�^���V�~�����[�V����

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
	float gx,gy, pre_gx, pre_gy, air_res, theta;
	bool overlapping = checkOverlapping(_hitArea, &gx, &gy,100);//��̊֐���`�Q��

	VECTOR2D c;		
	GetARSG()->Convert3Dto2D(&c, GetPosition());//���̃I�u�W�F�N�g���X�N���[����ɉf����Wc�ɕϊ�

	switch (state) {
		case OUT_TOUCH://�u����Ă���v�ƋL�^����Ă���
			if (overlapping) {//���݂̃t���[���ŏd�Ȃ��Ă���
				//vx = (c.x - gx) * 0.05f;
				//vy = -(c.y - gy) * 0.05f;
				if(checkOverlapping(_hitArea, &pre_gx, &pre_gy, 100))
					state = IN_TOUCH;//�L�^��ύX
			}
			else {
				air_res *= 0.93f;
				SetRotationZ(air_res);
			}
			break;
		case IN_TOUCH://�u�d�Ȃ��Ă���v�ƋL�^����Ă���
			if (overlapping) {//���݂̃t���[���ŏd�Ȃ��Ă��Ȃ�
				theta = atan2((pre_gy - c.y), (pre_gx - c.x)) - atan2((gy - c.y), (gx - c.x));
				pre_gx = gx;
				pre_gy = gy;
				SetRotationZ(theta);
				air_res = theta;
				if(checkOverlapping(_hitArea, &pre_gx, &pre_gy, 100))
					state = IN_TOUCH;//�L�^��ύX
			}
			else {
				air_res *= 0.93f;
				SetRotationZ(air_res);
				state = OUT_TOUCH;
			}
			break;
		default:
			break;
	}
}


inline void Touchable::move()
{
	VECTOR2D c;
	GetARSG()->Convert3Dto2D(&c, GetPosition());//���̃I�u�W�F�N�g���X�N���[����ɉf����Wc�ɕϊ�
		
	//�g�̔���
	if (c.x < 0 || c.x > sizex)	vx *= -1.0f;
	if (c.y > sizey-50 && vy<0)	vy *= -1.0f;

	//���R�����܂��͒�~
	if (c.y > sizey-50 && vy<0.03f) 
		vy = 0;
	else
		vy -= 0.03f;

	//��C��R
	vx *= 0.8f;
	vy *= 0.8f;

   SetPosition(vx, vy, 0.0f, GL_RELATIVE);
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
