#pragma once

#include "Config.h"
#include "Wnd.h"
#include "Players.h"

class CApp;
extern CApp theApp;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//应用程序类
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FRAME_TIME 20	//每帧时间 : 20毫秒

class CApp : public IApplication
{
	friend class CMainWnd;
public:
	CApp();

	//访问
	virtual HINSTANCE GetHInstance() {return m_hInstance;}
	CPoint		GetMousePos();
	bool		IsMouseDown() {return m_bMouseDown;}
	Sprite*		GetCurrentSprite() {return m_pCurrentSprite;}
	Sprite*		GetSprite(const tstring& sSprite) {return GSprites::Obj()->GetSprite(sSprite);}
	bool		IsActive() {return m_bActive;}
	IState*		GetCurrentState() {return m_pCurrentState;}

	//操作
	bool Init(HINSTANCE hInstance);
	void NewGame();
	void SetState(IState* pState);
	void Clear();
	bool Quit();
	void OnMouseClick();

	///////////////////
	virtual void Run()
	{
		MSG msg;
		for (;;)
		{
			while (!::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
			{
				OnIdle();
			}
			BOOL bRet = ::GetMessage(&msg, NULL, 0, 0);
			if (bRet == -1) continue;
			if (!bRet) break;
			if (!PreTranslateMessage(msg))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
	}

	///////////////////
	virtual BOOL PreTranslateMessage(MSG& msg)
	{
		return FALSE;
	}

	virtual bool OnIdle();

	//数据
public:
	CPlayers m_players;
	vector<Sprite*> m_arrSprites;	//所有活动中的精灵(按z轴从小到大排序)

protected:
	HINSTANCE m_hInstance;			//实例句柄
	CMainWnd m_wnd;					//主窗口
	bool m_bActive;					//当前窗口是否激活状态
	IState* m_pCurrentState;		//当前游戏状态
	Sprite* m_pCurrentSprite;		//当前鼠标所在的精灵
	bool m_bMouseDown;				//鼠标是否按下
};

