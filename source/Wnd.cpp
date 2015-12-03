#include "GDefine.h"
#include "Wnd.h"

//impl
#include "Application.h"
#include <windowsx.h>
#include "NetWork.h"

extern CApp theApp;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//主窗口消息

BOOL CMainWnd::ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
									LRESULT& lResult, DWORD dwMsgMapID)
{
	if (uMsg == WM_CLOSE)
	{
		bool b = theApp.Quit();
		return !b;
	}
	if (uMsg == WM_LBUTTONDOWN)
	{
		SetCapture();
		theApp.m_bMouseDown = true;
		return TRUE;
	}
	if (uMsg == WM_LBUTTONUP)
	{
		ReleaseCapture();
		theApp.OnMouseClick();
		theApp.m_bMouseDown = false;
		return TRUE;
	}
	if (uMsg == WM_ACTIVATE)
	{
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			theApp.m_bActive = false;
		}
		else
		{
			theApp.m_bActive = true;
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//对话框初始化

LRESULT CBeginDialog::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CLink::GetHostIP();

	m_bitmap[0].LoadBitmap(IDB_BITMAP0);
	m_bitmap[1].LoadBitmap(IDB_BITMAP1);
	m_bitmap[2].LoadBitmap(IDB_BITMAP2);
	m_bitmap[3].LoadBitmap(IDB_BITMAP3);

	m_bSingle = false;
	m_bServer = true;
	m_nHeadImage = 0;
	m_hThread = 0;
	m_bEndDialog = false;

	CheckDlgButton(IDC_CHECK_SERVER, BST_CHECKED);
	CheckDlgButton(IDC_RADIO_NETWORK, BST_CHECKED);
	SetDlgItemText(IDC_EDIT_IP, CLink::m_szIpaddress.c_str());
	GetDlgItem(IDC_EDIT_IP).EnableWindow(!m_bServer);
	GetDlgItem(IDC_SCROLLBAR2).SetScrollRange(SB_CTL, 0, 3);

	SetTimer(1, 100);
	CenterWindow(GetParent());

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CBeginDialog::OnOk(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	//更新数据
	GetDlgItemText(IDC_EDIT_IP, m_strIP);
	GetDlgItemText(IDC_EDIT_NAME, m_strName);
	m_nHeadImage = GetDlgItem(IDC_SCROLLBAR2).GetScrollPos(SB_CTL);
	if (m_strName.empty() || (!m_bServer && m_strIP.empty()))
	{
		AtlMessageBox(NULL, "请输入名称或服务器地址");
		return 0;
	}
	if (m_bServer) m_strIP.empty();

	//把数据写入当前玩家信息中
	_tcscpy(theApp.m_players[0]->m_baseInfo.sName, m_strName.c_str());
	_stprintf(theApp.m_players[0]->m_baseInfo.sImage, _T("face_%d"), m_nHeadImage);

	//设置网络类型
	if (m_bSingle)
	{
		GConfig::Obj()->SetNetWork(new CNetWork_Single());
	}
	else
	{
		if (m_bServer)
		{
			GConfig::Obj()->SetNetWork(new CNetWork_Server());
		}
		else 
		{
			GConfig::Obj()->SetNetWork(new CNetWork_Client());
		}
	}

	//创建线程.等待其他玩家加入.
	m_hThread = (HANDLE)_beginthread(ThreadFunc, 0, this);

	//确定按钮不可用
	GetDlgItem(IDOK).EnableWindow(FALSE);
	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CBeginDialog::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (m_hThread)
	{
		//如果有线程可能会造成泄漏
	}
	EndDialog(wID);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CBeginDialog::OnScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	if ((HWND)lParam != GetDlgItem(IDC_SCROLLBAR2).m_hWnd)
	{
		return 0;
	}
	int nPos = GetDlgItem(IDC_SCROLLBAR2).GetScrollPos(SB_CTL);
	switch(LOWORD(wParam))
	{
	case SB_LINEUP:
	case SB_PAGEUP:
		if(nPos != 0)
		{
			GetDlgItem(IDC_SCROLLBAR2).SetScrollPos(SB_CTL, nPos-1);
		}
		break;
	case SB_LINEDOWN:
	case SB_PAGEDOWN:
		if(nPos < 3)
		{
			GetDlgItem(IDC_SCROLLBAR2).SetScrollPos(SB_CTL, nPos+1);
		}
		break;
	case SB_THUMBPOSITION:
		short int nNewPos = (short int) HIWORD(wParam);
		GetDlgItem(IDC_SCROLLBAR2).SetScrollPos(SB_CTL, nNewPos);
		break;
	}
	SendDlgItemMessage(IDC_FACE, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)(m_bitmap[nPos].m_hBitmap));
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CBeginDialog::OnCheckServer(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (wNotifyCode == BN_CLICKED)
	{
		m_bServer = IsDlgButtonChecked(wID) ? true : false;
		GetDlgItem(IDC_EDIT_IP).EnableWindow(!m_bServer);
		SetDlgItemText(IDC_EDIT_IP, CLink::m_szIpaddress.c_str());
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CBeginDialog::OnTimer(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	if (wParam == 1)
	{
		//刷新数据
		if (m_hThread)
		{
			GetDlgItem(IDOK).EnableWindow(FALSE);
			GetDlgItem(IDC_SCROLLBAR2).EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_NAME).EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_SERVER).EnableWindow(FALSE);
			GetDlgItem(IDC_RADIO_NETWORK).EnableWindow(FALSE);
			GetDlgItem(IDC_RADIO_SINGLE).EnableWindow(FALSE);
			//刷新界面
			SetDlgItemText(IDC_NET_INFO, m_strNetInfo.c_str());
		}
		else
		{
			GetDlgItem(IDC_SCROLLBAR2).EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_NAME).EnableWindow(TRUE);
			GetDlgItem(IDC_CHECK_SERVER).EnableWindow(!m_bSingle);
			GetDlgItem(IDC_RADIO_NETWORK).EnableWindow(TRUE);
			GetDlgItem(IDC_RADIO_SINGLE).EnableWindow(TRUE);

			GetDlgItemText(IDC_EDIT_IP, m_strIP);
			GetDlgItemText(IDC_EDIT_NAME, m_strName);
			m_nHeadImage = GetDlgItem(IDC_SCROLLBAR2).GetScrollPos(SB_CTL);
			if (m_strName.empty() || (!m_bServer && m_strIP.empty()))
			{
				GetDlgItem(IDOK).EnableWindow(FALSE);
			}
			else
			{
				GetDlgItem(IDOK).EnableWindow(TRUE);
			}
		}

		//是否结束
		if (m_bEndDialog)
		{
			EndDialog(IDOK);
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CBeginDialog::OnRadios(WORD wNotifyCode, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (wNotifyCode == BN_CLICKED)
	{
		if (wID == IDC_RADIO_SINGLE)
		{
			m_bSingle = true;
			GetDlgItem(IDC_EDIT_IP).EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_SERVER).EnableWindow(FALSE);
		}
		else
		{
			m_bSingle = false;
			GetDlgItem(IDC_EDIT_IP).EnableWindow(!m_bServer);
			GetDlgItem(IDC_CHECK_SERVER).EnableWindow(TRUE);
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __cdecl CBeginDialog::ThreadFunc (void* pData)
{
	CBeginDialog* pThis = (CBeginDialog*) pData;
	try
	{
		if (GConfig::Obj()->GetNetWork()->GetPlayers(pThis->m_strNetInfo, pThis->m_strIP))
		{
			pThis->m_bEndDialog = true;
		}
	}
	catch(...)
	{
		::MessageBox(NULL, "发生意外错误, 可能是由于网络引起.\n游戏结束!", 0, 0);
		::ExitProcess(NULL);
		pThis->m_bEndDialog = true;
	}
}
