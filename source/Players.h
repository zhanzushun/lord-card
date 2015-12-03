#pragma once

#include "Config.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//玩家类
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
bool PtrLessThan(T* t1, T* t2) {return (*t1) < (*t2);}

class CPlayer
{
public:
	virtual ~CPlayer() {}

	struct BaseInfo
	{
		BaseInfo();
		TCHAR sName[16];				//名字
		TCHAR sImage[16];				//头像
		int nScore;						//得分
		int nID;						//以服务器所在的玩家为0编号,按打牌次序编号(逆时针)
	};

	struct CardsInfo
	{
		Card arrHandCards[SET_CARDS];	//在手上的牌
		Card arrOutCards[SET_CARDS];	//打出的牌
		Card arrPlayingCards[SET_CARDS];//正在打的牌
		int nHandCards;					//手上的牌的个数
		int nOutCards;					//打出去的牌的个数
		int nPlayingCards;				//正在打的牌的个数
		int arrPrepareCards[SET_CARDS];	//在手中准备打出的牌(比其他牌高出一点点), 保存的是在arrHandCards中的索引值
		int nPrepareCards;				//在手中准备打出的牌的个数
	};

	BaseInfo m_baseInfo;				//基本信息
	CardsInfo m_cardsInfo;				//牌的信息

public:

	void PlayCard();					//出牌(prepare to playing)
	void RemovePlayingCards();			//一轮结束(playing to out)
	void AddBottomCards();				//把底牌加入
	void SortHandCards();				//对手上的牌排序
	int GetIndex();						//取得在players中的索引
	void GetPrepareCards(vector<Card>& cards);

	bool operator < (const CPlayer& player2); //用于排序(排序结果就是在CPlayer容器中以自己为0,按打牌顺序)
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//玩家集合
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CPlayers
{
public:
	~CPlayers() {Clear();}
	static Card s_AllCards[SET_CARDS*2];		//当前局所有的牌
	static int s_nPlayingID;					//当前局当前出牌者
	static int s_nPlayingID_MaxCard;			//当前局本轮牌最大者
	static int s_nBankerID;						//当前局庄家
	static int s_nRandomBankerID;				//当前局随机庄家
	static int s_nTurnCounts;					//当前局轮数
	static int s_nNewTurnID;					//新的一轮出牌者(-1表示非新的一轮)


	//游戏初始化
	void Init();

	//读取索引值
	CPlayer* operator [] (int nIndex);

	//读取server所在的index
	int GetServerPlayerIndex();

	//根据ID返回
	CPlayer* GetPlayerByID(int nID);

	//返回下一个ID
	int	GetNextPlayerID(int nID);

	//返回上一个ID
	int	GetPrevPlayerID(int nID);

	//按 ID 排列所有玩家(排序结果就是在CPlayer容器中以自己为0,按打牌顺序)
	void Sort();

	//新一局
	void ResetData();

	//个数
	size_t size() {return m_arrPlayers.size();}

	void Clear();

private:
	vector<CPlayer*> m_arrPlayers;				//玩家数据
};