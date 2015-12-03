#include "gdefine.h"
#include "Players.h"

//impl
#include "Application.h"
#include "RenderScene.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CPlayer
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CPlayer::BaseInfo::BaseInfo()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//用于排序(排序结果就是在CPlayer容器中以自己为0,按打牌顺序)

bool CPlayer::operator < (const CPlayer& player2)
{
	int nID1 = m_baseInfo.nID;
	int nID2 = player2.m_baseInfo.nID;
	int nIDSelf = theApp.m_players[0]->m_baseInfo.nID;

	if (nID1 == nIDSelf) return true;
	if (nID2 == nIDSelf) return false;

	if (nID1 < nIDSelf && nID2 < nIDSelf) return nID1 < nID2;
	if (nID1 > nIDSelf && nID2 > nIDSelf) return nID1 < nID2;

	if (nID1 < nIDSelf && nID2 > nIDSelf) return false;
	if (nID1 > nIDSelf && nID2 < nIDSelf) return true;

	return nID1 < nID2;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CPlayer::GetIndex()
{
	for(int i=0; i < (int)theApp.m_players.size(); ++i)
		if (theApp.m_players[i] == this)
			return i;
	assert(0);
	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CPlayer::PlayCard()
{
	RemovePlayingCards();
	m_cardsInfo.nPlayingCards = 0;

	int nPrepare = m_cardsInfo.nPrepareCards;
	int* pPrepare = m_cardsInfo.arrPrepareCards;
	vector<Card> arrCards;

	//把 arrPrepareCards 转移到 arrPlayingCards 中
	GetPrepareCards(arrCards);
	Card* pHandCards = m_cardsInfo.arrHandCards;
	for(int i=0; i<(int)arrCards.size(); ++i)
	{
		Card card = arrCards[i];

		//牌的移动
		CPoint p1, p2;
		p1 = GConfig::Obj()->GetPlayerPos(GConfig::HANDCARD, GetIndex(), pPrepare[i]);
		p2 = GConfig::Obj()->GetPlayerPos(GConfig::PLAYINGCARD, GetIndex(), m_cardsInfo.nPlayingCards);
		GRenderScene::RenderMovingCard(true, true, false, &card, p1, p2, 4, false);//画4帧

		//arrHandCards 中删除一张
		std::remove(pHandCards, pHandCards + m_cardsInfo.nHandCards, card);
		m_cardsInfo.nHandCards --;	//减少手上牌的个数

		//playingCards 增加一张
		m_cardsInfo.arrPlayingCards[i] = card;
		m_cardsInfo.nPlayingCards ++; //增加
	}

	//设置 prepare 的个数
	m_cardsInfo.nPrepareCards = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CPlayer::GetPrepareCards(vector<Card>& arrCards)
{
	int nPrepare = m_cardsInfo.nPrepareCards;
	int* pPrepare = m_cardsInfo.arrPrepareCards;
	for(int i=0; i<nPrepare; ++i)
	{
		Card card = m_cardsInfo.arrHandCards[pPrepare[i]];
		arrCards.push_back(card);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CPlayer::RemovePlayingCards()
{
	Card* pPlayingCards = m_cardsInfo.arrPlayingCards;
	Card* pOutCards = m_cardsInfo.arrOutCards;
	std::copy(pPlayingCards, pPlayingCards + m_cardsInfo.nPlayingCards, pOutCards + m_cardsInfo.nOutCards);
	m_cardsInfo.nOutCards += m_cardsInfo.nPlayingCards;
	m_cardsInfo.nPlayingCards = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CPlayer::SortHandCards()
{
	//重新排序
	Card* pBegin = m_cardsInfo.arrHandCards;
	std::sort(pBegin, pBegin + m_cardsInfo.nHandCards);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CPlayer::AddBottomCards()
{
	int nBottom = GConfig::Obj()->GetBottomCards();
	for(int i=0; i<nBottom; i++)
	{
		int nAllCards = GConfig::Obj()->GetAllCards();
		Card card = CPlayers::s_AllCards[nAllCards-1 - i];
		m_cardsInfo.arrHandCards[m_cardsInfo.nHandCards + i] = card;
	}
	m_cardsInfo.nHandCards += nBottom;

	//排序
	SortHandCards();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPlayers 类
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Card CPlayers::s_AllCards[SET_CARDS*2];
int CPlayers::s_nPlayingID = -1;
int CPlayers::s_nPlayingID_MaxCard = -1;
int CPlayers::s_nBankerID = -1;
int CPlayers::s_nRandomBankerID = -1;
int CPlayers::s_nTurnCounts = -1;
int CPlayers::s_nNewTurnID = -1;

void CPlayers::Init()
{
	m_arrPlayers.clear();
	for(int i=0; i<GConfig::Obj()->GetPlayers(); i++)
	{
		CPlayer* pNewPlayer = new CPlayer();
		pNewPlayer->m_baseInfo.nID = i;
		pNewPlayer->m_baseInfo.nScore = 0;
		_stprintf(pNewPlayer->m_baseInfo.sImage, _T("face_%d"), rand()%4);
		if (i == 1)	_stprintf(pNewPlayer->m_baseInfo.sName, _T("周星星"));
		else if (i == 2) _stprintf(pNewPlayer->m_baseInfo.sName, _T("吴宗宪"));
		else if (i == 3) _stprintf(pNewPlayer->m_baseInfo.sName, _T("成龙"));
		else if (i == 4) _stprintf(pNewPlayer->m_baseInfo.sName, _T("詹祖顺"));
		else if (i == 5) _stprintf(pNewPlayer->m_baseInfo.sName, _T("路人甲"));
		else if (i == 6) _stprintf(pNewPlayer->m_baseInfo.sName, _T("路人乙"));
		else if (i == 7) _stprintf(pNewPlayer->m_baseInfo.sName, _T("路人丙"));
		m_arrPlayers.push_back(pNewPlayer);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CPlayers::Sort()
{
	std::sort(m_arrPlayers.begin(), m_arrPlayers.end(), PtrLessThan<CPlayer>);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CPlayers::ResetData()
{
	for(unsigned int i=0; i<m_arrPlayers.size(); ++i)
	{
		m_arrPlayers[i]->m_cardsInfo.nHandCards = 0;
		m_arrPlayers[i]->m_cardsInfo.nOutCards = 0;
		m_arrPlayers[i]->m_cardsInfo.nPlayingCards = 0;
		m_arrPlayers[i]->m_cardsInfo.nPrepareCards = 0;
	}
	s_nPlayingID = -1;
	s_nPlayingID_MaxCard = -1;
	s_nBankerID = -1;
	s_nRandomBankerID = -1;
}

CPlayer* CPlayers::operator [] (int nIndex)
{
	if (nIndex >=0 && nIndex < (int)m_arrPlayers.size())
	{
		return m_arrPlayers[nIndex];
	}
	else
	{
		return NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CPlayers::GetServerPlayerIndex()
{
	for(unsigned int i=0; i<m_arrPlayers.size(); ++i)
	{
		if (m_arrPlayers[i]->m_baseInfo.nID == 0)
		{ 
			return i;
			break;
		}
	}
	assert(0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CPlayer* CPlayers::GetPlayerByID(int nID)
{
	for(unsigned int i=0; i<m_arrPlayers.size(); ++i)
	{
		if (m_arrPlayers[i]->m_baseInfo.nID == nID)
		{ 
			return m_arrPlayers[i];
			break;
		}
	}
	assert(0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int	CPlayers::GetNextPlayerID(int nID)
{
	nID ++;
	if (nID >= (int)m_arrPlayers.size())
	{
		nID = 0;
	}
	return nID;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int	CPlayers::GetPrevPlayerID(int nID)
{
	nID --;
	if (nID < 0)
	{
		nID = (int)m_arrPlayers.size() - 1;
	}
	return nID;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CPlayers::Clear()
{
	for(unsigned int i=0; i<m_arrPlayers.size(); ++i)
		delete m_arrPlayers[i];
	m_arrPlayers.clear();
}