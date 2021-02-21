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

	//for debug(1/2): 別のウインドウに処理途中の画素情報、ここでは重なり部分を表示
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
	
	Texture hitArea{ &g, sizex, sizey };//重なり部分
	Texture stored{ &g, sizex, sizey };//背景
	Texture2D source{ &g, sizex, sizey };//最新フレーム

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
		
		ball.react(&hitArea); //重なり領域hitAreaに応答
		//ball.move();          //運動シミュレーション

		//for debug(2/2)
		//debug = hitArea;
		//arsgd.Draw(&debug);
		g.Draw();
		
	}
	d.StopGraph();
	return 0;
}

//背景差分
inline void subtract_mask(Texture* result, Texture* backgrnd, Texture* src, DWORD border)
{
	ARSC::diff(result,backgrnd,src,border);  //差
	ARSC::monochrome(result,result);         //モノクロ化（RGBから輝度Yに変換）
	ARSC::thresholding(result,result,border);//２値化
}


//hitAreaの白領域と重なる領域があればその領域の重心（画素の位置の平均値）を引数（の参照先に）に設定し、trueを返す
inline bool Touchable::checkOverlapping(Texture* hitArea, float *pGx, float *pGy, unsigned int threshold)
{	
	static Texture txtr;//作業用テクスチャオブジェクト（１つを再利用）
	ARSG* g = GetARSG(); //所属シーンgを取得
	txtr.Init(g,sizex,sizey);//指定パラメータで(再)初期化

	unsigned int pixel_count;//面積格納用

	g->Draw(this,&txtr); //このオブジェクト(this)を作業用テクスチャtxtrに描画
	ARSC::and(&txtr, &txtr, hitArea, 0x10101010);//txtr := txtrとhitAreaでともに閾値を超える部分(チャンネルごと)で0xff
	                                       // 0xff002020(スイカ=青緑) & 0xffffffff(hitArea=白) => 0xff00ffff(水色) 

	ARSC::getCG(pGx, pGy, &pixel_count, &txtr);	//txtrの非ゼロ画素の位置の平均値と個数
	return pixel_count > threshold;
}

inline void Touchable::react(Texture* _hitArea)
{
	float gx,gy, pre_gx, pre_gy, air_res, theta;
	bool overlapping = checkOverlapping(_hitArea, &gx, &gy,100);//上の関数定義参照

	VECTOR2D c;		
	GetARSG()->Convert3Dto2D(&c, GetPosition());//このオブジェクトがスクリーン上に映る座標cに変換

	switch (state) {
		case OUT_TOUCH://「離れている」と記録されている
			if (overlapping) {//現在のフレームで重なっている
				//vx = (c.x - gx) * 0.05f;
				//vy = -(c.y - gy) * 0.05f;
				if(checkOverlapping(_hitArea, &pre_gx, &pre_gy, 100))
					state = IN_TOUCH;//記録を変更
			}
			else {
				air_res *= 0.93f;
				SetRotationZ(air_res);
			}
			break;
		case IN_TOUCH://「重なっている」と記録されている
			if (overlapping) {//現在のフレームで重なっていない
				theta = atan2((pre_gy - c.y), (pre_gx - c.x)) - atan2((gy - c.y), (gx - c.x));
				pre_gx = gx;
				pre_gy = gy;
				SetRotationZ(theta);
				air_res = theta;
				if(checkOverlapping(_hitArea, &pre_gx, &pre_gy, 100))
					state = IN_TOUCH;//記録を変更
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
	GetARSG()->Convert3Dto2D(&c, GetPosition());//このオブジェクトがスクリーン上に映る座標cに変換
		
	//枠の反射
	if (c.x < 0 || c.x > sizex)	vx *= -1.0f;
	if (c.y > sizey-50 && vy<0)	vy *= -1.0f;

	//自由落下または停止
	if (c.y > sizey-50 && vy<0.03f) 
		vy = 0;
	else
		vy -= 0.03f;

	//空気抵抗
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
