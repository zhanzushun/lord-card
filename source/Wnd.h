#pragma once

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//主窗口类
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CApp;

class CMainWnd : public CWindowImpl<CMainWnd, CWindow, CFrameWinTraits>
{
public:
	DECLARE_WND_CLASS(_T("Main window class"));
	virtual BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		LRESULT& lResult, DWORD dwMsgMapID);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//登录对话框
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CBeginDialog : public CDialogImpl<CBeginDialog>
{
public:
	enum { IDD = IDD_DIALOG1 };

	BEGIN_MSG_MAP(CBeginDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_VSCROLL, OnScroll)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_CHECK_SERVER, OnCheckServer)
		COMMAND_RANGE_HANDLER(IDC_RADIO_SINGLE, IDC_RADIO_NETWORK, OnRadios)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnCheckServer(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnRadios(WORD wNotifyCode, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	static void __cdecl ThreadFunc (void* pData);

protected:
	CBitmap m_bitmap[4];
	map<tstring,int> m_arrPlayers;
	bool m_bServer;
	bool m_bSingle;
	tstring m_strIP;
	tstring m_strName;
	tstring m_strNetInfo;
	int m_nHeadImage;
	HANDLE m_hThread;
	bool m_bEndDialog;
};

