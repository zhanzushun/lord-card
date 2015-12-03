#include "GDefine.h"
#include "gdefine.h"
#include "Application.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WinMain 函数
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CApp theApp;


int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	try
	{
		//随即种子
		srand(GetTickCount());

		//初始化 WSA
		WSADATA wsaData;
		WORD wVersionRequested = MAKEWORD(1, 1);
		int nResult = WSAStartup(wVersionRequested, &wsaData);
		if (nResult != 0)return 0;

		//初始化应用程序
		if (!theApp.Init(hInstance))
		{
			return 1;
		}

		//进入消息循环
		theApp.Run();

		//退出
		theApp.Clear();

		//退出wsa
		WSACleanup();
	}

	catch(BKException e)
	{
		AtlMessageBox(NULL, e.message.c_str());
		AtlMessageBox(NULL, _T("应用程序退出"));
	}

	catch(...)
	{
		AtlMessageBox(NULL, _T("未知的错误.应用程序退出"));
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// APP 类
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CApp::CApp() 
{
	m_hInstance = NULL;
	m_pCurrentState = NULL;
	m_pCurrentSprite = NULL;
	m_bActive = true;
	m_bMouseDown = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//初始化

bool CApp::Init(HINSTANCE hInstance)
{
	m_hInstance = hInstance;

	//Load游戏配置
	GConfig::Obj()->Load("game_config.xml");

	//初始化玩家
	m_players.Init();

	//取得并同步所有玩家资料
	CBeginDialog dlg;
	if (dlg.DoModal() == IDCANCEL)
		return false;

	//创建主窗口
	m_wnd.Create(0);
	m_wnd.ShowWindow(SW_SHOW);

	//初始化 Display
	Display::Obj()->CreateWindowedDisplay(GConfig::Obj()->GetWidth()-1, GConfig::Obj()->GetHeight()-1, m_wnd);

	//初始化资源
	DWORD t = timeGetTime();
	GSurfaces::Obj()->Load("surfaces.xml");
	GSprites::Obj()->Load("sprite_cards.xml");
	GSprites::Obj()->Load("sprite_others.xml");

	m_arrSprites.push_back(GetSprite("button_play"));
	m_arrSprites.push_back(GetSprite("button_quit"));
	m_arrSprites.push_back(GetSprite("button_playcard"));
	m_arrSprites.push_back(GetSprite("button_skipcard"));
	GetSprite("button_play")->SetPos (GConfig::Obj()->GetPosition(GConfig::PLAY_BTN));
	GetSprite("button_quit")->SetPos (GConfig::Obj()->GetPosition(GConfig::QUIT_BTN));
	GetSprite("button_playcard")->SetPos (GConfig::Obj()->GetPosition(GConfig::PLAYCARD_BTN));
	GetSprite("button_skipcard")->SetPos (GConfig::Obj()->GetPosition(GConfig::SKIPCARD_BTN));


	//新游戏
	NewGame();

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//空闲处理: 直接运行帧循环

bool CApp::OnIdle()
{
	DWORD t = timeGetTime();
	if (m_pCurrentState)
	{
		//更新当前精灵
		CPoint pt = GetMousePos();
		m_pCurrentSprite = NULL;
		int n = (int)m_arrSprites.size();
		for(int i=n-1; i>=0; i--)
		{
			if (m_arrSprites[i]->HitTest(pt) && m_arrSprites[i]->IsEnabled())
			{
				m_pCurrentSprite = m_arrSprites[i];
			}
		}
		//调用当前状态的处理
		m_pCurrentState->RunOneFrame();	
	}
	//TCHAR buf[50]; _stprintf(buf,"%d\n", timeGetTime()-t); OutputDebugString(buf);
	DWORD t2 = timeGetTime();

	//保持帧速率
	if (t2 - t < FRAME_TIME)
	{
		Sleep(FRAME_TIME - (t2 - t));
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//退出

bool CApp::Quit()
{
	if (AtlMessageBox(m_wnd, _T("真的退出吗?"), _T(""), MB_OKCANCEL) == IDOK)
	{
		PostQuitMessage(0);
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//清除/释放资源

void CApp::Clear()
{
	GSprites::Obj()->Clear();
	GSurfaces::Obj()->Clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//鼠标位置

CPoint CApp::GetMousePos()
{
	CPoint pt;
	GetCursorPos(&pt);
	m_wnd.ScreenToClient(&pt);
	return pt;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//设置状态

void CApp::SetState(IState* pState)
{
	assert(pState);
	pState->SetActive();
	m_pCurrentState = pState;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CApp::OnMouseClick()
{
	if (m_pCurrentState)
	{
		m_pCurrentState->OnMouseClick();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CApp::NewGame()
{
	//游戏数据
	m_players.ResetData();

	//游戏状态
	theApp.SetState(GConfig::Obj()->GetGameType()->GetNextState(m_pCurrentState));

}

