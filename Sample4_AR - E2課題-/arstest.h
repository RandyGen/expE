#pragma once

#ifndef _DEBUG
#pragma comment(lib, "../lib/ARS.lib")
#pragma comment(lib, "../lib/WML.lib")
#else
#pragma comment(lib, "../lib/ARSd.lib")
#pragma comment(lib, "../lib/WMLd.lib")
#endif



#define _CRT_SECURE_NO_WARNINGS

#define CLASSNAME "ARSTEST"
#define APPNAME "ARSTEST"




class Touchable : public Mesh{
private:
   float vx,vy;	//速度ベクトル成分
   //hitAreaとの重なりがthreshold以上のときのみtrueを返す。
   //その場合重なりの重心（画素位置の平均）座標がgx,gyに格納される
   bool checkOverlapping(Texture* hitArea, float *gx, float *gy, unsigned int threshold);   
public:   
   enum  TouchState {OUT_TOUCH,IN_TOUCH};//直近の検査で重なって(いた/いなかった)ことを{IN/OUT}_TOUCHで記憶する
   TouchState state;//上記状態を保持

   Touchable(ARSG* _g, wchar_t fln[]) : Mesh(_g,fln), vx(0.2f),vy(-0.2f),state(IN_TOUCH){ }
   Touchable(void) : vx(0.2f),vy(-0.2f),state(OUT_TOUCH){ }
   void react(Texture *hitArea); //接触判定と応答
   void move(int turn);              //運動シミュレーション
   

};
