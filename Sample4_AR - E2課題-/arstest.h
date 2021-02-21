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
   float vx,vy;	//���x�x�N�g������
   //hitArea�Ƃ̏d�Ȃ肪threshold�ȏ�̂Ƃ��̂�true��Ԃ��B
   //���̏ꍇ�d�Ȃ�̏d�S�i��f�ʒu�̕��ρj���W��gx,gy�Ɋi�[�����
   bool checkOverlapping(Texture* hitArea, float *gx, float *gy, unsigned int threshold);   
public:   
   enum  TouchState {OUT_TOUCH,IN_TOUCH};//���߂̌����ŏd�Ȃ���(����/���Ȃ�����)���Ƃ�{IN/OUT}_TOUCH�ŋL������
   TouchState state;//��L��Ԃ�ێ�

   Touchable(ARSG* _g, wchar_t fln[]) : Mesh(_g,fln), vx(0.2f),vy(-0.2f),state(IN_TOUCH){ }
   Touchable(void) : vx(0.2f),vy(-0.2f),state(OUT_TOUCH){ }
   void react(Texture *hitArea); //�ڐG����Ɖ���
   void move(int turn);              //�^���V�~�����[�V����
   

};
