#include "GDefine.h"

#include "Config.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Card::operator < (const Card& c2) const
{
	return GConfig::Obj()->GetGameType()->CardLess(*this, c2);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

tstring Card::GetSpriteName()
{
	if (nValue == VALUE_JOKER_B)
	{
		return _T("card_Jokers_big");
	}
	else if (nValue == VALUE_JOKER_S)
	{
		return _T("card_Jokers_small");
	}

	tstring sSprite;
	tstring sColor;
	if (nColor == HEITAO) sColor = _T("heitao");
	else if (nColor == HONGTAO) sColor = _T("hongtao");
	else if (nColor == CAOHUA) sColor = _T("caohua");
	else if (nColor == FANGKUAI) sColor = _T("fangkuai");

	tstring sValue;
	if (nValue >= 3 && nValue <= 10)
	{
		TCHAR buf[3];
		_stprintf(buf, _T("%d"), nValue);
		sValue = buf;
	}
	if (nValue == 11) sValue = _T("J");
	if (nValue == 12) sValue = _T("Q");
	if (nValue == 13) sValue = _T("K");
	if (nValue == 14) sValue = _T("A");
	if (nValue == 15) sValue = _T("2");

	sSprite = _T("card_") + sValue + _T("_") + sColor;
	return sSprite;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GConfig::Load(const tstring& sFile)
{
	CXmlNode root;
	root.Load(sFile);

	m_nWidth = root.GetAttr(_T("width"), CXmlNode::IntType());
	if (m_nWidth <= 0) m_nWidth = 720;
	if (m_nHeight <= 0) m_nHeight = 540;
	m_nHeight = root.GetAttr(_T("height"), CXmlNode::IntType());
	m_nPlayers = root.GetAttr(_T("players"), CXmlNode::IntType());
	m_pos_HandCard.resize(m_nPlayers);
	m_pos_PlayingCard.resize(m_nPlayers);
	m_pos_OutCard.resize(m_nPlayers);
	m_pos_Head.resize(m_nPlayers);
	m_nCardsSet = root.GetAttr(_T("cardsset"), CXmlNode::IntType());
	m_nBottomCards = root.GetAttr(_T("bottomcards"), CXmlNode::IntType());

	CXmlNodes nds;
	root.SelectNodes(nds, "property");

	for(int i=0; i<nds.GetSize(); ++i)
	{
		if (_T("movecardframes") == nds[i]->GetAttr(_T("name"), CXmlNode::StringType())) 
			m_nMoveCardFrames = nds[i]->GetAttr(_T("value"), CXmlNode::IntType());

		else if (_T("pos_head") == nds[i]->GetAttr(_T("name"), CXmlNode::StringType()))
		{
			CXmlNodes nds2;
			nds[i]->SelectNodes(nds2, _T("player"));
			for(int j=0; j<nds2.GetSize(); j++)
			{
				m_pos_Head[j] = nds2[j]->GetAttr(_T("value"), CXmlNode::PointType());
			}
		}
	
		else if (_T("pos_handcard") == nds[i]->GetAttr(_T("name"), CXmlNode::StringType()))
		{
			CXmlNodes nds2;
			nds[i]->SelectNodes(nds2, _T("player"));
			for(int j=0; j<nds2.GetSize(); j++)
			{
				m_pos_HandCard[j].m_pos = nds2[j]->GetAttr(_T("value"), CXmlNode::PointType());
				m_pos_HandCard[j].m_offset = nds2[j]->GetAttr(_T("offset"), CXmlNode::PointType());
			}
		}

		else if (_T("pos_outcard") == nds[i]->GetAttr(_T("name"), CXmlNode::StringType()))
		{
			CXmlNodes nds2;
			nds[i]->SelectNodes(nds2, _T("player"));
			for(int j=0; j<nds2.GetSize(); j++)
			{
				m_pos_OutCard[j].m_pos = nds2[j]->GetAttr(_T("value"), CXmlNode::PointType());
				m_pos_OutCard[j].m_offset = nds2[j]->GetAttr(_T("offset"), CXmlNode::PointType());
			}
		}

		else if (_T("pos_playingcard") == nds[i]->GetAttr(_T("name"), CXmlNode::StringType()))
		{
			CXmlNodes nds2;
			nds[i]->SelectNodes(nds2, _T("player"));
			for(int j=0; j<nds2.GetSize(); j++)
			{
				m_pos_PlayingCard[j].m_pos = nds2[j]->GetAttr(_T("value"), CXmlNode::PointType());
				m_pos_PlayingCard[j].m_offset = nds2[j]->GetAttr(_T("offset"), CXmlNode::PointType());
			}
		}

		else if (_T("pos_button_quit") == nds[i]->GetAttr(_T("name"), CXmlNode::StringType())) 
			m_pos_Button_Quit = nds[i]->GetAttr(_T("value"), CXmlNode::PointType());

		else if (_T("pos_button_play") == nds[i]->GetAttr(_T("name"), CXmlNode::StringType())) 
			m_pos_Button_Play = nds[i]->GetAttr(_T("value"), CXmlNode::PointType());

		else if (_T("pos_button_playcard") == nds[i]->GetAttr(_T("name"), CXmlNode::StringType())) 
			m_pos_Button_PlayCard = nds[i]->GetAttr(_T("value"), CXmlNode::PointType());

		else if (_T("pos_button_skipcard") == nds[i]->GetAttr(_T("name"), CXmlNode::StringType())) 
			m_pos_Button_SkipCard = nds[i]->GetAttr(_T("value"), CXmlNode::PointType());

		else if (_T("pos_sendcard") == nds[i]->GetAttr(_T("name"), CXmlNode::StringType())) 
			m_pos_SendCard = nds[i]->GetAttr(_T("value"), CXmlNode::PointType());

		else if (_T("pos_bottomcard") == nds[i]->GetAttr(_T("name"), CXmlNode::StringType())) 
			m_pos_BottomCard = nds[i]->GetAttr(_T("value"), CXmlNode::PointType());

		else if (_T("information") == nds[i]->GetAttr(_T("name"), CXmlNode::StringType())) 
		{
			m_nInfoFontSize = nds[i]->GetAttr(_T("fontsize"), CXmlNode::IntType());
			m_clrInfo = nds[i]->GetAttr(_T("color"), CXmlNode::ColorType());
			m_pos_Info = nds[i]->GetAttr(_T("pos"), CXmlNode::PointType());
		}
			
		else if (_T("playername") == nds[i]->GetAttr(_T("name"), CXmlNode::StringType())) 
		{
			m_clrName = nds[i]->GetAttr(_T("color"), CXmlNode::ColorType());
			m_nNameFontSize = nds[i]->GetAttr(_T("fontsize"), CXmlNode::IntType());
		}
	}

	m_pGameType = IGameType::CreateGameType(IGameType::T_LORD3);
	assert(m_pGameType);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CPoint GConfig::GetPosition(GConfig::EPosFlag ePos)
{
	switch(ePos)
	{
	case QUIT_BTN :
		return m_pos_Button_Quit;
	case PLAY_BTN :
		return m_pos_Button_Play;
	case PLAYCARD_BTN :
		return m_pos_Button_PlayCard;
	case SKIPCARD_BTN :
		return m_pos_Button_SkipCard;
	case SEND_CARD:
		return m_pos_SendCard;
	case BOTTOM_CARD:
		return m_pos_BottomCard;
	}
	assert(0);
	return CPoint(0,0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CPoint GConfig::GetPlayerPos(GConfig::EPosFlag flag, int nPlayerIndex, int nPlayerCardIndex)
{
	CPoint pt(0,0);
	assert(nPlayerIndex >=0 || nPlayerIndex < m_nPlayers);
	if (flag == HEAD)
	{
		pt = m_pos_Head[nPlayerIndex];
		return pt;
	}
	else if (flag == HANDCARD)
	{
		pt = m_pos_HandCard[nPlayerIndex].m_pos;
		pt.x += m_pos_HandCard[nPlayerIndex].m_offset.x * nPlayerCardIndex;
		pt.y += m_pos_HandCard[nPlayerIndex].m_offset.y * nPlayerCardIndex;
		return pt;
	}
	else if (flag == OUTCARD)
	{
		pt = m_pos_OutCard[nPlayerIndex].m_pos;
		pt.x += m_pos_OutCard[nPlayerIndex].m_offset.x * nPlayerCardIndex;
		pt.y += m_pos_OutCard[nPlayerIndex].m_offset.y * nPlayerCardIndex;
		return pt;
	}
	else if (flag == PLAYINGCARD)
	{
		pt = m_pos_PlayingCard[nPlayerIndex].m_pos;
		pt.x += m_pos_PlayingCard[nPlayerIndex].m_offset.x * nPlayerCardIndex;
		pt.y += m_pos_PlayingCard[nPlayerIndex].m_offset.y * nPlayerCardIndex;
		return pt;
	}
	assert(0);
	return pt;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CPoint GConfig::GetPlayerOffset(GConfig::EPosFlag flag, int nPlayerIndex)
{
	CPoint pt(0,0);
	assert(nPlayerIndex >=0 || nPlayerIndex < m_nPlayers);
	if (flag == HANDCARD)
	{
		return m_pos_HandCard[nPlayerIndex].m_offset;
	}
	else if (flag == OUTCARD)
	{
		return m_pos_OutCard[nPlayerIndex].m_offset;
	}
	else if (flag == PLAYINGCARD)
	{
		return m_pos_PlayingCard[nPlayerIndex].m_offset;
	}
	assert(0);
	return pt;
}
