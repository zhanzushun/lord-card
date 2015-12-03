#pragma once

#include "Config.h"

class CGameType_Lord3 : public IGameType
{
public:

	//取下一个状态
	virtual IState* GetNextState(IState*);

	//单张牌的大小比较(用于排序)
	virtual bool CardLess(const Card& card1, const Card& card2);

	//取名称(根据不同游戏类型有不同的修饰)
	virtual tstring GetNameText(int nID, const tstring& sOriginal);

	virtual void GetText_DecideBanker(int i, tstring& sText);

	virtual bool CheckPrepareCards(const Cards& arrPrevCards, const Cards& arrCards);

	virtual void ComputerPrepareCards(Cards& arrCards, const Cards& arrHandCards, const Cards& arrPrevCards);

	virtual int AccountCardsValue(const Cards& arrHandCards);

};


