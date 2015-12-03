#pragma once

#include "Config.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//所有状态

class CStateInit;				//初始状态
class CStateSendCards;			//发牌状态
class CStateDecideBanker;		//确定庄家
class CStatePlayCards;			//玩牌状态

class GStatus
{
public:
	static CStateInit s_stateInit;
	static CStateSendCards s_stateSendCards;
	static CStatePlayCards s_statePlayCards;
	static CStateDecideBanker s_stateDecideBanker;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//初始状态 (等待所有玩家都同意下一局)

class CStateInit : public IState
{
	//接口
public:
	virtual void SetActive();
	virtual void RunOneFrame();

	//线程
public:
	static void __cdecl ThreadFunc (void* pData);
protected:
	HANDLE m_hEventEndThread;
	void pOnThreadEnd();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//发牌状态

class CStateSendCards : public IState
{
//接口
public:
	virtual void SetActive();
	virtual void RunOneFrame();

//线程
public:
	static void __cdecl ThreadFunc (void* pData);
protected:
	HANDLE m_hEventEndThread;
	void pOnThreadEnd();

//私有函数
	void pDrawMoveCard();

//数据
	int m_nCardIndex;
	int m_nPlayerIndex;
	CPoint m_ptCardPos;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//选择庄家状态

class CStateDecideBanker : public IState
{
	//接口
public:
	virtual void SetActive();
	virtual void RunOneFrame();

	//线程
public:
	static void __cdecl ThreadFunc (void* pData);
protected:
	HANDLE m_hEventEndThread;
	
	//数据
	int m_nTempCount;//不愿当庄家的个数
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//玩牌状态

class CStatePlayCards : public IState
{
	//接口
public:
	virtual void SetActive();
	virtual void RunOneFrame();
	virtual void OnMouseClick();

	//线程
public:
	static void __cdecl ThreadFunc (void* pData);
protected:
	HANDLE m_hEventEndThread;

	//数据
	bool m_bPlaying;				//本人可出牌
	HANDLE m_hEventPrepareStart;
	HANDLE m_hEventPrepareEnd;
	HANDLE m_hEventPlayingStart;
	HANDLE m_hEventPlayingEnd;
};

