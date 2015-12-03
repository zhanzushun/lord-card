#include "GDefine.h"
#include "States.h"

#include "Application.h"
#include "RenderScene.h"

#define CLOSE_EVENT(e)   if (e) {CloseHandle(e); e = NULL;}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//状态定义
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CStateInit			GStatus::s_stateInit;
CStateSendCards		GStatus::s_stateSendCards;
CStatePlayCards		GStatus::s_statePlayCards;
CStateDecideBanker	GStatus::s_stateDecideBanker;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateInit
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CStateInit::SetActive()
{
	theApp.GetSprite("button_play")->SetVisible(false);
	theApp.GetSprite("button_quit")->SetVisible(false);
	theApp.GetSprite("button_playcard")->SetVisible(false);
	theApp.GetSprite("button_skipcard")->SetVisible(false);

	//创建线程
	m_hEventEndThread = CreateEvent(0, FALSE, FALSE, 0);
	_beginthread(ThreadFunc, 0, this);
}

void CStateInit::RunOneFrame()
{
	if (m_hEventEndThread)
	{
		DWORD state = WaitForSingleObject(m_hEventEndThread, 0);
		if (state == WAIT_OBJECT_0)
		{
			CLOSE_EVENT(m_hEventEndThread);
			//线程已经结束
			pOnThreadEnd();
		}
	}

	GRenderScene::Render(false);
	Display::Obj()->Present();
}

void __cdecl CStateInit::ThreadFunc (void* pData)
{
	CStateInit* pThis = (CStateInit*) pData;
	//添加同步代码(等待所有人都同意新局)
	SetEvent(pThis->m_hEventEndThread);
}

void CStateInit::pOnThreadEnd()
{
	m_hEventEndThread = NULL;
	theApp.SetState(GConfig::Obj()->GetGameType()->GetNextState(this));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateSendCards
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CStateSendCards::SetActive()
{
	m_nCardIndex = 0;
	m_nPlayerIndex = 0;
	m_hEventEndThread = NULL;

	theApp.GetSprite("button_quit")->SetVisible(true);

	//创建线程: 洗牌
	m_hEventEndThread = CreateEvent(0, FALSE, FALSE, 0);
	_beginthread(ThreadFunc, 0, this);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CStateSendCards::RunOneFrame()
{
	if (m_hEventEndThread)
	{
		DWORD state = WaitForSingleObject(m_hEventEndThread, 0);
		if (state == WAIT_OBJECT_0)
		{
			CLOSE_EVENT(m_hEventEndThread);
			//线程已经结束
			pOnThreadEnd();
		}
	}

	if (m_hEventEndThread)
	{
		//还在洗牌, 不画牌
		GRenderScene::Render(false);
		Display::Obj()->Present();
	}
	else
	{
		//发牌
		pDrawMoveCard();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __cdecl CStateSendCards::ThreadFunc (void* pData)
{
	CStateSendCards* pThis = (CStateSendCards*) pData;
	try
	{
		GConfig::Obj()->GetNetWork()->GetCards();
	}
	catch(...)
	{
		::MessageBox(NULL, "发生意外错误, 可能是由于网络引起.\n游戏结束!", 0, 0);
		::ExitProcess(NULL);
	}
	SetEvent(pThis->m_hEventEndThread);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CStateSendCards::pOnThreadEnd()
{
	//发牌: 从ID为0的起发
	m_nCardIndex = 0;
	m_nPlayerIndex = theApp.m_players.GetServerPlayerIndex();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CStateSendCards::pDrawMoveCard()
{
	//准备绘制信息
	int nPlayerCardIndex = m_nCardIndex / GConfig::Obj()->GetPlayers();
	CPoint p1 = GConfig::Obj()->GetPosition(GConfig::SEND_CARD);
	CPoint p2 = GConfig::Obj()->GetPlayerPos(GConfig::HANDCARD, m_nPlayerIndex, nPlayerCardIndex);

	Card card = CPlayers::s_AllCards[m_nCardIndex];
	theApp.m_players[m_nPlayerIndex]->m_cardsInfo.arrHandCards[nPlayerCardIndex] = card;

	//按位置绘制
	int nFrames = GConfig::Obj()->GetMoveCardFrames();
	GRenderScene::RenderMovingCard(false, false, true, &card, p1, p2, nFrames, true);
	m_ptCardPos = p2;

	//准备下一张牌
	theApp.m_players[m_nPlayerIndex]->m_cardsInfo.nHandCards = nPlayerCardIndex + 1;
	m_nCardIndex++;
	if (m_nCardIndex >= GConfig::Obj()->GetAllCards() - GConfig::Obj()->GetBottomCards())
	{
		//结束发牌
		//对所有牌排序
		for(int i=0; i<GConfig::Obj()->GetPlayers(); i++)
		{
			theApp.m_players[i]->SortHandCards();
		}

		//设置下一个状态
		theApp.SetState(GConfig::Obj()->GetGameType()->GetNextState(this));
		return;
	}

	//下一个玩家
	m_nPlayerIndex++;
	if (m_nPlayerIndex >= GConfig::Obj()->GetPlayers())
		m_nPlayerIndex = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CStateDecideBanker
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CStateDecideBanker::SetActive()
{
	m_nTempCount = 0;

	//创建线程: 确定庄家
	m_hEventEndThread = CreateEvent(0, FALSE, FALSE, 0);
	_beginthread(ThreadFunc, 0, this);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CStateDecideBanker::RunOneFrame()
{
	//判断线程是否结束.结束则调用 pOnThreadEnd
	if (m_hEventEndThread)
	{
		DWORD state = WaitForSingleObject(m_hEventEndThread, 0);
		if (state == WAIT_OBJECT_0)
		{
			CLOSE_EVENT(m_hEventEndThread);
		}
	}

	//根据当前状态绘制
	GRenderScene::Render(true, true, false);
	HDC hdc = Display::Obj()->GetBackSurface()->GetHDC();
	CDCHandle cdc(hdc);
	
	CFont font; font.CreatePointFont(GConfig::Obj()->GetInfoFontSize(), _T("宋体"));
	HFONT hf = cdc.SelectFont(font);
	cdc.SetTextColor(GConfig::Obj()->GetInfoColor());
	cdc.SetBkMode(TRANSPARENT);

	tstring sName;
	if (m_hEventEndThread)
	{
		if (m_nTempCount == 0)
		{
			//正在产生随机庄家
			GConfig::Obj()->GetGameType()->GetText_DecideBanker(IGameType::Decide_Banker_Random, sName);
		}
		else
		{
			//随机庄家已产生, 正式庄家未产生
			sName = theApp.m_players.GetPlayerByID(CPlayers::s_nBankerID)->m_baseInfo.sName;
			GConfig::Obj()->GetGameType()->GetText_DecideBanker(IGameType::Decide_Banker_PreConfirm, sName);
		}
	}
	else
	{
		//正式庄家已经产生
		sName = theApp.m_players.GetPlayerByID(CPlayers::s_nBankerID)->m_baseInfo.sName;
		GConfig::Obj()->GetGameType()->GetText_DecideBanker(IGameType::Decide_Banker_Confirm, sName);
	}

	CPoint pos = GConfig::Obj()->GetInfoPos();
	cdc.TextOut(pos.x, pos.y, sName.c_str());
	cdc.SelectFont(hf);
	Display::Obj()->GetBackSurface()->ReleaseHDC(hdc);	
	Display::Obj()->Present();

	if (m_hEventEndThread == NULL)
	{
		//正式庄家已经产生
		Sleep(300);

		//把底牌加入庄家手中
		CPlayer* pBanker = theApp.m_players.GetPlayerByID(CPlayers::s_nBankerID);
		pBanker->AddBottomCards();

		//设置下一个状态:玩牌
		theApp.SetState(GConfig::Obj()->GetGameType()->GetNextState(this));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __cdecl CStateDecideBanker::ThreadFunc (void* pData)
{
	CStateDecideBanker* pThis = (CStateDecideBanker*) pData;
	try
	{
		//取得随机庄家
		GConfig::Obj()->GetNetWork()->GetRandomBanker();
		CPlayers::s_nBankerID = CPlayers::s_nRandomBankerID;
		pThis->m_nTempCount = 1;

		while(1)
		{
			if (theApp.m_players[0]->m_baseInfo.nID == CPlayers::s_nBankerID)
			{
				//本人是否同意当庄家
				Sleep(100);
				tstring str;
				GConfig::Obj()->GetGameType()->GetText_DecideBanker(IGameType::Decide_Banker_Msgbox,str);

				bool bAgree = AtlMessageBox(NULL, str.c_str(),
					theApp.m_players[0]->m_baseInfo.sName, MB_OKCANCEL) == IDOK;
				
				GConfig::Obj()->GetNetWork()->AgreeBanker(bAgree);
				
				//同意就跳出循环结束线程
				if (bAgree) break;
			}
			else
			{
				//其他人是否同意当庄家
				bool bAgree = GConfig::Obj()->GetNetWork()->GetAgreeBanker(CPlayers::s_nBankerID);

				//同意就跳出循环结束线程
				if (bAgree) break;

				//总要有人同意吧.
				if (pThis->m_nTempCount >= GConfig::Obj()->GetPlayers()-1) break;
			}

			//下一个
			CPlayers::s_nBankerID ++;
			if (CPlayers::s_nBankerID >= GConfig::Obj()->GetPlayers())
			{
				CPlayers::s_nBankerID = 0;
			}
			pThis->m_nTempCount ++;
		}
	}
	catch(...)
	{
		::MessageBox(NULL, _T("发生意外错误, 可能是由于网络引起.\n游戏结束!"), 0, 0);
		::ExitProcess(NULL);
	}
	SetEvent(pThis->m_hEventEndThread);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CStatePlayCards
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CStatePlayCards::SetActive()
{
	m_bPlaying = false;
	CPlayers::s_nTurnCounts = -1;
	CPlayers::s_nNewTurnID = -1;

	theApp.GetSprite("button_play")->SetVisible(false);
	theApp.GetSprite("button_quit")->SetVisible(true);
	theApp.GetSprite("button_playcard")->SetVisible(false);
	theApp.GetSprite("button_skipcard")->SetVisible(true);

	//创建线程
	m_hEventEndThread = CreateEvent(0, FALSE, FALSE, 0);
	m_hEventPlayingStart = CreateEvent(0, FALSE, FALSE, 0);
	m_hEventPlayingEnd = CreateEvent(0, FALSE, FALSE, 0);
	m_hEventPrepareStart = CreateEvent(0, FALSE, FALSE, 0);
	m_hEventPrepareEnd = CreateEvent(0, FALSE, FALSE, 0);

	_beginthread(ThreadFunc, 0, this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CStatePlayCards::RunOneFrame()
{
	if (!m_bPlaying)
	{
		theApp.GetSprite("button_playcard")->SetVisible(false);
		theApp.GetSprite("button_skipcard")->SetVisible(false);
	}
	else if (!theApp.GetSprite("button_playcard")->IsVisible() && !theApp.GetSprite("button_skipcard")->IsVisible())
	{
		theApp.GetSprite("button_skipcard")->SetVisible(true);
	}

	//判断线程是否结束
	if (m_hEventEndThread)
	{
		if (WaitForSingleObject(m_hEventEndThread, 0) == WAIT_OBJECT_0)
			CLOSE_EVENT(m_hEventEndThread);
	}

	if (m_hEventPlayingStart)
	{
		if (WaitForSingleObject(m_hEventPlayingStart, 0) == WAIT_OBJECT_0)
		{
			//绘制其他玩家出牌过程动画
			theApp.m_players.GetPlayerByID(CPlayers::s_nPlayingID)->PlayCard();
			SetEvent(m_hEventPlayingEnd);
		}
	}

	//判断是否开始出牌
	if (!m_bPlaying)
	{
		DWORD state = WaitForSingleObject(m_hEventPrepareStart, 0);
		{
			if (state == WAIT_OBJECT_0)
			{
				m_bPlaying = true;
			}
		}
	}
	
	//根据当前状态绘制
	GRenderScene::Render(true, true, true);
	Display::Obj()->Present();

	if (m_hEventEndThread == NULL)
	{
		CLOSE_EVENT(m_hEventPrepareStart);
		CLOSE_EVENT(m_hEventPrepareEnd);
		CLOSE_EVENT(m_hEventPlayingStart);
		CLOSE_EVENT(m_hEventPlayingEnd);

		//线程结束.进入下一个状态
		theApp.NewGame();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CStatePlayCards::OnMouseClick()
{
	if (!m_bPlaying)
	{
		return;
	}

	CPoint ptMouse = theApp.GetMousePos();
	CPoint ptStart = GConfig::Obj()->GetPlayerPos(GConfig::HANDCARD, 0, 0);
	CRect rcCard = GSprites::Obj()->GetSprite(_T("card_back"))->GetPos();
	int nHeight = rcCard.Height();
	int nHSpace = GConfig::Obj()->GetPlayerOffset(GConfig::HANDCARD, 0).x;

	CRect rcStart(ptStart, CSize(nHSpace, nHeight));

	//1 判断是否点在 prepare 的牌上
	int nPrepare = theApp.m_players[0]->m_cardsInfo.nPrepareCards;
	int* pPrepare = theApp.m_players[0]->m_cardsInfo.arrPrepareCards;

	//2 判断是否点在手中其他牌上
	int nHitIndex = -1;
	bool bPrepare = false;
	int nHandCards = theApp.m_players[0]->m_cardsInfo.nHandCards;

	for (int i=0; i<nHandCards; ++i)
	{
		//是否在prepare上
		bool bPrepareTemp = false;
		for(int j=0; j<nPrepare; ++j)
		{
			if (i == pPrepare[j])
			{
				bPrepareTemp = true;
				break;
			}
		}

		//矩形位置
		CRect rcTemp = rcStart + CPoint(nHSpace * i, 0);
		if ((i == nHandCards-1) || bPrepareTemp)
		{
			rcTemp.right = rcTemp.left + rcCard.Width();
			rcTemp.bottom = rcTemp.top + rcCard.Height();
		}
		if (bPrepareTemp) rcTemp -= CPoint(0, PREPARE_OFFSET_V);

		//测试击中
		if (rcTemp.PtInRect(ptMouse))
		{
			bPrepare = bPrepareTemp;
			nHitIndex = i;
		}
	}

	if (nHitIndex >= 0)
	{
		if (bPrepare)
		{
			//从prepare中删除这张牌
			std::remove(pPrepare, pPrepare + nPrepare, nHitIndex);
			theApp.m_players[0]->m_cardsInfo.nPrepareCards --;
		}
		else
		{
			//把这张牌添加到 prepare 中
			theApp.m_players[0]->m_cardsInfo.arrPrepareCards[nPrepare] = nHitIndex;
			theApp.m_players[0]->m_cardsInfo.nPrepareCards ++;
		}
	}

	bool bHasPrepare = (theApp.m_players[0]->m_cardsInfo.nPrepareCards > 0);
	theApp.GetSprite("button_playcard")->SetVisible(bHasPrepare);
	theApp.GetSprite("button_skipcard")->SetVisible(!bHasPrepare);

	//3 判断是否点在 playcard 或 skipcard 按钮上
	if (theApp.GetSprite("button_playcard")->HitTest(ptMouse) ||
		theApp.GetSprite("button_skipcard")->HitTest(ptMouse))
	{
		if (bHasPrepare)
		{
			//先检查
			vector<Card> arrCards, arrPrevCards;
			theApp.m_players[0]->GetPrepareCards(arrCards);
	
			//如果不是本人先出牌
			if (CPlayers::s_nNewTurnID != theApp.m_players[0]->m_baseInfo.nID)
			{
				int i = 0;
				while (arrPrevCards.size() == 0)
				{
					assert(i<GConfig::Obj()->GetPlayers());
                    Card* pBegin = theApp.m_players[GConfig::Obj()->GetPlayers()-1 - i]->m_cardsInfo.arrPlayingCards;
					int nCount = theApp.m_players[GConfig::Obj()->GetPlayers()-1 - i]->m_cardsInfo.nPlayingCards;
					arrPrevCards.resize(nCount);
					std::copy(pBegin, pBegin+nCount, arrPrevCards.begin());
					i++;
				}
			}
			
			std::sort(arrPrevCards.begin(), arrPrevCards.end());
			std::sort(arrCards.begin(), arrCards.end());

			if (! GConfig::Obj()->GetGameType()->CheckPrepareCards(arrPrevCards, arrCards))
			{
				//检查不通过.不允许出牌
				theApp.m_players[0]->m_cardsInfo.nPrepareCards = 0;
				theApp.GetSprite("button_playcard")->SetVisible(false);
				theApp.GetSprite("button_skipcard")->SetVisible(true);
				return;
			}
		}
		else
		{
			if (CPlayers::s_nNewTurnID == theApp.m_players[0]->m_baseInfo.nID)
			{
				//必须出牌
				return;
			}
		}

		//设置 bPlaying 为false
		m_bPlaying = false;
		//通知线程
		SetEvent(m_hEventPrepareEnd);
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __cdecl CStatePlayCards::ThreadFunc (void* pData)
{
	CStatePlayCards* pThis = (CStatePlayCards*) pData;
	try
	{
		CPlayers::s_nPlayingID = CPlayers::s_nBankerID;	//庄家先出牌
		CPlayers::s_nPlayingID_MaxCard = CPlayers::s_nPlayingID;
		bool bMyself = theApp.m_players[0]->m_baseInfo.nID == CPlayers::s_nPlayingID;

		CPlayers::s_nTurnCounts = 0;
		CPlayers::s_nNewTurnID =  CPlayers::s_nPlayingID;

		while(1)
		{
			bMyself = theApp.m_players[0]->m_baseInfo.nID == CPlayers::s_nPlayingID;
			if (bMyself)
			{
				//本人出牌
				SetEvent(pThis->m_hEventPrepareStart);
				//等待出牌结束
				DWORD state = WaitForSingleObject(pThis->m_hEventPrepareEnd, INFINITE);
				if (state == WAIT_OBJECT_0)
				{
					//出牌结束
					//发送当前轮打出的牌(如果本轮不要.则 arrPlayingCards 数组为空
					GConfig::Obj()->GetNetWork()->SendPrepareCards();
				}
			}

			//取得待出牌
			GConfig::Obj()->GetNetWork()->GetPrepareCards();

			//
			CPlayers::s_nNewTurnID = -1;

			//出牌.绘制出牌过程(刷新arrPlayingCards)
			SetEvent(pThis->m_hEventPlayingStart);
			DWORD state = WaitForSingleObject(pThis->m_hEventPlayingEnd, INFINITE);
			assert(state == WAIT_OBJECT_0);

			//判断是否新的一轮
			int nNextPlayerID = theApp.m_players.GetNextPlayerID(CPlayers::s_nPlayingID);
			if (theApp.m_players.GetPlayerByID(CPlayers::s_nPlayingID)->m_cardsInfo.nPlayingCards == 0)
			{
				//如果谁不接, 且下一个玩家就是本轮牌最大的玩家
				if (nNextPlayerID == CPlayers::s_nPlayingID_MaxCard)
				{
					//新一轮开始
					CPlayers::s_nTurnCounts ++;
					CPlayers::s_nNewTurnID = nNextPlayerID;
				}
			}
			else
			{
				//更新牌最大玩家
				CPlayers::s_nPlayingID_MaxCard = CPlayers::s_nPlayingID;
			}

			//判断是否整局结束
			if (theApp.m_players.GetPlayerByID(CPlayers::s_nPlayingID)->m_cardsInfo.nHandCards == 0)
			{
				//整局结束
				CPlayer* pBanker = theApp.m_players.GetPlayerByID(CPlayers::s_nBankerID);
				int nFlag = 1;
				if (CPlayers::s_nPlayingID != CPlayers::s_nBankerID)
				{
					nFlag = -1;
				}

				pBanker->m_baseInfo.nScore += (2*nFlag);
				for (int i=0; i<GConfig::Obj()->GetPlayers(); ++i)
				{
					if (pBanker == theApp.m_players[i]) continue;
					theApp.m_players[i]->m_baseInfo.nScore -= (nFlag);
				}
				
				break;
			}
			else
			{
				//下一个玩家出牌
				CPlayers::s_nPlayingID = nNextPlayerID;
			}
		}
	}
	catch(...)
	{
		::MessageBox(NULL, _T("发生意外错误, 可能是由于网络引起.\n游戏结束!"), 0, 0);
		::ExitProcess(NULL);
	}
	Sleep(300);
	SetEvent(pThis->m_hEventEndThread);
}

