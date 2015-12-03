#include "gdefine.h"
#include "NetWork.h"

#include "Application.h"

void ShuffleCards()
{
	//随即种子
	srand(GetTickCount());

	//将牌按顺序放好
	for(int iSet=0; iSet < GConfig::Obj()->GetCardsSet(); iSet++)
	{
		int nBase = iSet * SET_CARDS;
		for(int i=3,j=0;j<=12;i++,j++)
		{
			//黑桃
			CPlayers::s_AllCards[ (j*4+0) + nBase].nColor = Card::HEITAO;
			CPlayers::s_AllCards[ (j*4+0) + nBase].nValue = i;
			//红桃
			CPlayers::s_AllCards[ (j*4+1) + nBase].nColor = Card::HONGTAO;
			CPlayers::s_AllCards[ (j*4+1) + nBase].nValue = i;
			//梅花
			CPlayers::s_AllCards[ (j*4+2) + nBase].nColor = Card::CAOHUA;
			CPlayers::s_AllCards[ (j*4+2) + nBase].nValue = i;
			//方块
			CPlayers::s_AllCards[ (j*4+3) + nBase].nColor = Card::FANGKUAI;
			CPlayers::s_AllCards[ (j*4+3) + nBase].nValue = i;
		}
		CPlayers::s_AllCards[53 + nBase].nColor = Card::JOKER_S;	//小怪
		CPlayers::s_AllCards[53 + nBase].nValue = Card::VALUE_JOKER_S;
		CPlayers::s_AllCards[52 + nBase].nColor = Card::JOKER_B;   //大怪
		CPlayers::s_AllCards[52 + nBase].nValue = Card::VALUE_JOKER_B;
	}

	//高效洗牌
	int nRandPos;   
	Card cTempCard;

	int nAll = GConfig::Obj()->GetAllCards();
	for(int k=0; k<nAll; k++)
	{
		//产生一个随机位置;
		nRandPos = rand() % nAll;

		//把当前牌与这个随机位置的牌交换;
		cTempCard = CPlayers::s_AllCards[k];
		CPlayers::s_AllCards[k] = CPlayers::s_AllCards[nRandPos];
		CPlayers::s_AllCards[nRandPos] = cTempCard;	
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Single
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CNetWork_Single::GetCards()
{
	//洗牌
	ShuffleCards();
}

void CNetWork_Single::GetRandomBanker()
{
	Sleep(2000);//让大家有一定的时间看随机抽取动画
	//随即种子
	srand(GetTickCount());
	CPlayers::s_nRandomBankerID = rand() % GConfig::Obj()->GetPlayers();
}

void CNetWork_Single::AgreeBanker(bool bAgree)
{
	//nothing
}

bool CNetWork_Single::GetAgreeBanker(int nTempBankerID)
{

	Card* pHandBegin = theApp.m_players.GetPlayerByID(nTempBankerID)->m_cardsInfo.arrHandCards;
	int nHandCount = theApp.m_players.GetPlayerByID(nTempBankerID)->m_cardsInfo.nHandCards;

	Cards arrHandCards;
	arrHandCards.resize(nHandCount);
	std::copy(pHandBegin, pHandBegin+nHandCount, arrHandCards.begin());

	int nValue = GConfig::Obj()->GetGameType()->AccountCardsValue(arrHandCards);

	for (int i=0; i<GConfig::Obj()->GetPlayers(); i++)
	{
		if (theApp.m_players[i]->m_baseInfo.nID == nTempBankerID) continue;

		pHandBegin = theApp.m_players[i]->m_cardsInfo.arrHandCards;
		nHandCount = theApp.m_players[i]->m_cardsInfo.nHandCards;
		arrHandCards.resize(nHandCount);
		std::copy(pHandBegin, pHandBegin+nHandCount, arrHandCards.begin());

		int nOtherValue = GConfig::Obj()->GetGameType()->AccountCardsValue(arrHandCards);

		if (nValue < nOtherValue) return false;
	}

	return true;
}

void CNetWork_Single::SendPrepareCards()
{
	//nothing
}

void CNetWork_Single::GetPrepareCards()
{
	if (CPlayers::s_nPlayingID == theApp.m_players[0]->m_baseInfo.nID)
	{
		return;
	}

	//取得其他玩家出的牌目前是随机抽几张牌
	//先检查
	Cards arrPrevCards;

	if (CPlayers::s_nNewTurnID != CPlayers::s_nPlayingID)
	{
		int nID = CPlayers::s_nPlayingID;
		while (arrPrevCards.size() == 0)
		{
			nID = theApp.m_players.GetPrevPlayerID(nID);
			Card* pBegin = theApp.m_players.GetPlayerByID(nID)->m_cardsInfo.arrPlayingCards;
			int nCount = theApp.m_players.GetPlayerByID(nID)->m_cardsInfo.nPlayingCards;
			arrPrevCards.resize(nCount);
			std::copy(pBegin, pBegin+nCount, arrPrevCards.begin());
		}
		std::sort(arrPrevCards.begin(), arrPrevCards.end());
	}

	//电脑选择出牌
	Card* pHandBegin = theApp.m_players.GetPlayerByID(CPlayers::s_nPlayingID)->m_cardsInfo.arrHandCards;
	int nHandCount = theApp.m_players.GetPlayerByID(CPlayers::s_nPlayingID)->m_cardsInfo.nHandCards;

	Cards arrHandCards;
	arrHandCards.resize(nHandCount);
	std::copy(pHandBegin, pHandBegin+nHandCount, arrHandCards.begin());

	//结果
	Cards arrPrepareCards;
	GConfig::Obj()->GetGameType()->ComputerPrepareCards(arrPrepareCards, arrHandCards, arrPrevCards);

	if (arrPrevCards.size() == 0)//电脑本轮先出
	{
		assert(arrPrepareCards.size() > 0);//保证必须出牌
	}

	//复制arrPrepareCards
	int* pPrepareBegin = theApp.m_players.GetPlayerByID(CPlayers::s_nPlayingID)->m_cardsInfo.arrPrepareCards;
	int& nPrepareCount = theApp.m_players.GetPlayerByID(CPlayers::s_nPlayingID)->m_cardsInfo.nPrepareCards;
	nPrepareCount = (int)arrPrepareCards.size();

	for (int i=0; i<nPrepareCount; i++)
	{
		Card* pCard = std::find(pHandBegin, pHandBegin+nHandCount, arrPrepareCards[i]);
		pPrepareBegin[i] = (int)(pCard - pHandBegin);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Client
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CNetWork_Client::GetPlayers(tstring& strDlgNetInfo, const tstring& strIP)
{
	//连接至服务器
	m_linkToServer.SocketInit();
	m_linkToServer.Create();
	if (!m_linkToServer.Connect(strIP))
	{
		m_linkToServer.Close();
		return false;
	}

	//发送 baseinfo
	TCHAR sNetInfo[100];
	m_linkToServer.Send(&theApp.m_players[0]->m_baseInfo, sizeof(CPlayer::BaseInfo));

	//接收 int
	m_linkToServer.Recv(&theApp.m_players[0]->m_baseInfo.nID, sizeof(int));

	//接收 netinfo
	//4个人.如果id为1,表示有3个netinfo需要接收, 如果id为3.表示只有一个netinfo需要接收
	for (int i=0; i < (GConfig::Obj()->GetPlayers() - theApp.m_players[0]->m_baseInfo.nID); ++i)
	{
		m_linkToServer.Recv(sNetInfo, sizeof(TCHAR)*100);
		strDlgNetInfo = sNetInfo;
	}

	//接收其余玩家的baseinfo
	for (int i=0; i< GConfig::Obj()->GetPlayers(); ++i)
	{
		if (i == 0)
		{
			continue;
		}
		CPlayer::BaseInfo& info = theApp.m_players[i]->m_baseInfo;
		m_linkToServer.Recv(&info, sizeof(CPlayer::BaseInfo));
	}

	//按 ID 排列所有玩家(排序结果就是在CPlayer容器中以自己为0,按打牌顺序)
	theApp.m_players.Sort();

	return true;
}


void CNetWork_Client::GetCards()
{
	//从服务器收牌
	m_linkToServer.Recv(CPlayers::s_AllCards, sizeof(Card) * GConfig::Obj()->GetAllCards());
}

void CNetWork_Client::GetRandomBanker()
{
	//从服务器接收
	m_linkToServer.Recv(&CPlayers::s_nRandomBankerID, sizeof(int));
}

void CNetWork_Client::AgreeBanker(bool bAgree)
{
	//发送至服务器
	m_linkToServer.Send(&bAgree, sizeof(bool));
}

bool CNetWork_Client::GetAgreeBanker(int nTempBanker)
{
	//从服务器接收
	bool bAgree = false;
	m_linkToServer.Recv(&bAgree, sizeof(bool));
	return bAgree;
}

void CNetWork_Client::SendPrepareCards()
{
	//向服务器发送待出牌
	int* pPrepare = theApp.m_players[0]->m_cardsInfo.arrPrepareCards;
	int nPrepare = theApp.m_players[0]->m_cardsInfo.nPrepareCards;
	m_linkToServer.Send(&nPrepare, sizeof(int));
	if (nPrepare == 0) nPrepare = 1;//至少要发点东西
	m_linkToServer.Send(pPrepare, sizeof(int) * nPrepare);
}

void CNetWork_Client::GetPrepareCards()
{
	if (CPlayers::s_nPlayingID == theApp.m_players[0]->m_baseInfo.nID)
	{
		return;
	}
	//从服务器接收
	int* pPrepare = theApp.m_players.GetPlayerByID(CPlayers::s_nPlayingID)->m_cardsInfo.arrPrepareCards;
	int& nPrepare = theApp.m_players.GetPlayerByID(CPlayers::s_nPlayingID)->m_cardsInfo.nPrepareCards;
	m_linkToServer.Recv(&nPrepare, sizeof(int));
	int nCount = nPrepare;
	if (nPrepare == 0) nCount = 1;//至少要收点什么东西进来呀
	m_linkToServer.Recv(pPrepare, sizeof(int) * nCount);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Server
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNetWork_Server::Clear()
{
	vector<CLink*> ::const_iterator it = m_arrLinkToClients.begin();
	for(; it!= m_arrLinkToClients.end(); ++it)
	{
		delete (*it);
	}
	m_arrLinkToClients.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNetWork_Server::CreateLinks()
{
	m_arrLinkToClients.resize(GConfig::Obj()->GetPlayers());
	for(unsigned int i=0; i<m_arrLinkToClients.size(); ++i)
	{
		m_arrLinkToClients[i] = new CLink();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CLink* CNetWork_Server::GetLink(int nIndex)
{
	assert(nIndex >= 0 && nIndex <(int)m_arrLinkToClients.size());
	return m_arrLinkToClients[nIndex];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CNetWork_Server::GetPlayers(tstring& strDlgNetInfo, const tstring& strIP)
{
	//接收客户的加入请求
	CLink listen;
	listen.SocketInit();
	listen.Create();
	listen.Bind();
	listen.Listen();

	//信息
	strDlgNetInfo = tstring(theApp.m_players[0]->m_baseInfo.sName) + _T("已经建立服务器, 等待其他玩家...");

	//当前玩家ID为0 (服务器玩家)
	theApp.m_players[0]->m_baseInfo.nID = 0;
	int nRecvCount = 0;

	for (int i=0; i<GConfig::Obj()->GetPlayers(); i++)
	{
		if (i == 0)
		{
			continue;
		}

		//从新加入的玩家中接受玩家信息(名称,头像)
		listen.Accept(GetLink(i));
		nRecvCount ++;

		CPlayer::BaseInfo& info = theApp.m_players[i]->m_baseInfo;
		GetLink(i)->Recv(&info, sizeof(CPlayer::BaseInfo));//接收 baseinfo
		info.nScore = 0;
		info.nID = nRecvCount;
		GetLink(i)->Send(&info.nID, sizeof(int));//发送 int

		//打印信息
		strDlgNetInfo += info.sName;
		strDlgNetInfo += _T("加入游戏,");

		if (nRecvCount == GConfig::Obj()->GetPlayers()-1)
		{
			strDlgNetInfo += _T("开始游戏!");
		}
		else
		{
			strDlgNetInfo += _T("等待其他玩家...");
		}

		//发送打印信息
		TCHAR sNetInfo[100];
		_stprintf(sNetInfo, strDlgNetInfo.c_str());
		for(int j=0; j<=i; j++)
		{
			if (theApp.m_players[0] == theApp.m_players[j])
			{
				continue;
			}
			GetLink(j)->Send(sNetInfo,sizeof(char)*100);//发送 TCHAR[100]
		}
	}

	//发送玩家信息
	for (int i=0; i<GConfig::Obj()->GetPlayers(); i++)
	{
		CPlayer::BaseInfo& info = theApp.m_players[i]->m_baseInfo;
		for(int j=0; j<GConfig::Obj()->GetPlayers(); j++)
		{
			if (theApp.m_players[0] == theApp.m_players[j] || j == i)
			{
				continue;
			}
			//发送每个玩家的资料(至少需要更新ID)
			GetLink(j)->Send(&info, sizeof(CPlayer::BaseInfo));//发送
		}
	}

	//按 ID 排列所有玩家(排序结果就是在CPlayer容器中以自己为0,按打牌顺序)
	theApp.m_players.Sort();

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNetWork_Server::GetCards()
{
	//洗牌
	ShuffleCards();

	//把牌发送至各客户端
	for(int i=0; i<GConfig::Obj()->GetPlayers(); i++)
	{
		if (i == 0)
		{
			continue;
		}
		GetLink(i)->Send(CPlayers::s_AllCards, sizeof(Card) * GConfig::Obj()->GetAllCards());
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNetWork_Server::GetRandomBanker()
{
	Sleep(1000);//让大家有一定的时间看随机抽取动画

	//随即种子
	srand(GetTickCount());

	CPlayers::s_nRandomBankerID = rand() % GConfig::Obj()->GetPlayers();

	//发送至各客户端
	for(int i=0; i<GConfig::Obj()->GetPlayers(); i++)
	{
		if (i == 0)
		{
			continue;
		}
		GetLink(i)->Send(&CPlayers::s_nRandomBankerID, sizeof(int));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNetWork_Server::AgreeBanker(bool bAgree)
{
	for(int i=0; i<GConfig::Obj()->GetPlayers(); i++)
	{
		if (i == 0)
		{
			continue;
		}
		GetLink(i)->Send(&bAgree, sizeof(bool));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CNetWork_Server::GetAgreeBanker(int nTempBanker)
{
	bool bAgree;
	GetLink(nTempBanker)->Recv(&bAgree, sizeof(bool));

	for(int i=0; i<GConfig::Obj()->GetPlayers(); i++)
	{
		if (i == 0)
			continue;

		if (nTempBanker == theApp.m_players[i]->m_baseInfo.nID)
			continue;

		GetLink(i)->Send(&bAgree, sizeof(bool));
	}

	return bAgree;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNetWork_Server::SendPrepareCards()
{
	//向大家发送自己的待出牌
	for(int i=0; i<GConfig::Obj()->GetPlayers(); i++)
	{
		if (i == 0)
		{
			continue;
		}
		int* pPrepare = theApp.m_players[0]->m_cardsInfo.arrPrepareCards;
		int nPrepare = theApp.m_players[0]->m_cardsInfo.nPrepareCards;
		GetLink(i)->Send(&nPrepare, sizeof(int));
		if (nPrepare == 0) nPrepare = 1; //至少要发点什么
		GetLink(i)->Send(pPrepare, sizeof(int) * nPrepare);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CNetWork_Server::GetPrepareCards()
{
	if (CPlayers::s_nPlayingID == theApp.m_players[0]->m_baseInfo.nID)
	{
		return;
	}

	CPlayer* pPlayer = theApp.m_players.GetPlayerByID(CPlayers::s_nPlayingID);
	int nPlayerIndex = pPlayer->GetIndex();

	int* pPrepare = theApp.m_players[nPlayerIndex]->m_cardsInfo.arrPrepareCards;
	int& nPrepare = theApp.m_players[nPlayerIndex]->m_cardsInfo.nPrepareCards;
	GetLink(nPlayerIndex)->Recv(&nPrepare, sizeof(int));
	int nCount = nPrepare;
	if (nCount == 0) nCount = 1;	//至少要收发点东西
	GetLink(nPlayerIndex)->Recv(pPrepare, sizeof(int) * nCount);

	for(int i=0; i<GConfig::Obj()->GetPlayers(); i++)
	{
		if (i == 0)
			continue;

		if (i == nPlayerIndex)
			continue;

		GetLink(i)->Send(&nPrepare, sizeof(int));
		GetLink(i)->Send(pPrepare, sizeof(int) * nCount);
	}
}

