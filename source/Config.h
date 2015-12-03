#pragma once

#define HEAD_IMAGE_COUNT 4	//总共4个头像

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//一张牌
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Card
{
	static const int HEITAO = 0;		//黑桃
	static const int HONGTAO = 1;		//红桃
	static const int CAOHUA = 2;		//草花
	static const int FANGKUAI = 3;		//方块
	static const int JOKER_S = 0;		//小怪
	static const int JOKER_B = 1;		//大怪
	static const int VALUE_J = 11;
	static const int VALUE_Q = 12;
	static const int VALUE_K = 13;
	static const int VALUE_A = 14;
	static const int VALUE_2 = 15;
	static const int VALUE_JOKER_S = 16;
	static const int VALUE_JOKER_B = 17;

	int nColor;						//牌的花色
	int nValue;						//牌的大小. 3~17

	tstring GetSpriteName();
	bool operator < (const Card& c2) const;
	Card():nColor(-1),nValue(-1){}
	bool operator == (const Card& card2) {return nColor == card2.nColor && nValue == card2.nValue;}
};

typedef vector<Card> Cards;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//处理网络 (单机运行. 网络客户端. 网络服务器)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class INetWork
{
public:
	virtual ~INetWork() {}
	virtual bool GetPlayers(tstring& strDlgNetInfo, const tstring& strIP) = 0; //取得玩家信息
	virtual void GetCards() = 0;					//取得所有牌
	virtual void GetRandomBanker() = 0;				//产生随机庄家
	virtual void AgreeBanker(bool) = 0;				//发送至服务器:是否同意当庄家
	virtual bool GetAgreeBanker(int nTemp) = 0;		//从服务器接收:他人是否同意当庄家
	virtual void SendPrepareCards() = 0;			//发送至服务器:本人待出牌
	virtual void GetPrepareCards() = 0;				//从服务器接收待出牌
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//状态接口 (游戏中存在不同的多种状态)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class IState
{
public:
	virtual ~IState() {}
	virtual void RunOneFrame() = 0;
	virtual void SetActive(){};
	virtual void OnMouseClick(){};
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//游戏类型 (斗地主,80分等)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class IGameType
{
public:
	enum EGameType
	{
		T_LORD3,		//斗地主(3人)
		T_JOKER_LOAD,	//大怪路子
		T_80,			//八十分
	};

	//虚析构函数
	virtual ~IGameType() {}

	//取下一个游戏状态
	virtual IState* GetNextState(IState*) = 0;

	//单张牌的大小比较(用于排序)
	virtual bool CardLess(const Card& card1, const Card& card2) = 0;

	//创建对象
	static IGameType* CreateGameType(EGameType type);

	//确认是否可以出牌
	virtual bool CheckPrepareCards(const Cards& arrPrevCards, const Cards& arrCards) = 0;

	//电脑出牌
	virtual void ComputerPrepareCards(Cards& arrCards, const Cards& arrHandCards, const Cards& arrPrevCards) = 0;

	//计算牌值
	virtual int AccountCardsValue(const Cards& arrHandCards) = 0;

	//名字的修饰
	virtual tstring GetNameText(int nID, const tstring& sOriginal) {return sOriginal;}

	//取得关于选择庄家的文本
	static const int Decide_Banker_Random = 0;		//随机庄家
	static const int Decide_Banker_PreConfirm = 1;	//待确认的庄家
	static const int Decide_Banker_Confirm = 2;		//确认的庄家
	static const int Decide_Banker_Msgbox = 3;		//msgbox
	virtual void GetText_DecideBanker(int i, tstring& sText){};
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//配置文件
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const int SET_CARDS = 54;	//一副牌数
static const int PREPARE_OFFSET_V = 20;

class GConfig : public SingletonT<GConfig>
{
	friend class SingletonT<GConfig>;
public:
	enum EPosFlag
	{
		HEAD, HANDCARD,	OUTCARD, PLAYINGCARD,			//对应于各玩家的头像/牌的位置
		QUIT_BTN, PLAY_BTN,	PLAYCARD_BTN, SKIPCARD_BTN,	//按钮位置
		SEND_CARD,										//发牌起始位置
		BOTTOM_CARD,									//底牌的位置
	};

	//基本配置
	int GetWidth()			{return m_nWidth;}
	int GetHeight()			{return m_nHeight;}
	int GetPlayers()		{return m_nPlayers;}		//玩家个数
	int GetCardsSet()		{return m_nCardsSet;}		//几副牌
	int GetBottomCards()	{return m_nBottomCards;}	//底牌数
	int GetMoveCardFrames() {return m_nMoveCardFrames;}	//移动一张牌的帧数
	CPoint GetPosition(EPosFlag ePos);					//绘图位置
	int GetAllCards()		{return GetCardsSet()*SET_CARDS;}
	int GetPlayerCards()	{return (GetAllCards() - GetBottomCards()) / GetPlayers();}
	CPoint GetPlayerPos(EPosFlag flag, int nPlayerIndex, int nPlayerCardIndex);
	CPoint GConfig::GetPlayerOffset(GConfig::EPosFlag flag, int nPlayerIndex);
	int GetNameFontSize()	{return m_nNameFontSize;}
	DWORD GetNameColor()	{return m_clrName;}
	int GetInfoFontSize()	{return m_nInfoFontSize;}
	DWORD GetInfoColor()	{return m_clrInfo;}
	CPoint GetInfoPos()		{return m_pos_Info;}

	INetWork* GetNetWork()  {return m_pNetWork;}
	IGameType* GetGameType(){return m_pGameType;}

	//设置
	void SetNetWork(INetWork* pNetWork) {m_pNetWork = pNetWork; }

public:
	~GConfig() 
	{
		delete m_pNetWork;
		delete m_pGameType;
	}
	void Load(const tstring& sFile);

private:
	GConfig() 
	{
		m_bLoaded = false;
		m_pGameType = NULL;
		m_pNetWork = NULL;
	}

	bool m_bLoaded;

	struct CardPosInfo
	{
		CPoint m_pos;
		CPoint m_offset;
	};

	int m_nWidth;										//宽度
	int m_nHeight;										//高度
	int m_nPlayers;										//玩家个数
	int m_nCardsSet;									//几副牌
	int m_nBottomCards;									//底牌张数
	int m_nMoveCardFrames;								//发牌的动作共移动几帧?
	vector<CPoint> m_pos_Head;							//头像位置
	vector<CardPosInfo> m_pos_HandCard;					//手中牌的位置
	vector<CardPosInfo> m_pos_OutCard;					//打出牌的位置
	vector<CardPosInfo> m_pos_PlayingCard;				//正在打的牌的位置
	CPoint m_pos_Button_Quit;							//按钮位置
	CPoint m_pos_Button_Play;							//按钮位置
	CPoint m_pos_Button_PlayCard;						//按钮位置
	CPoint m_pos_Button_SkipCard;						//按钮位置
	CPoint m_pos_SendCard;								//待发牌的位置
	CPoint m_pos_BottomCard;							//底牌位置
	CPoint m_pos_Info;									//信息显示的位置
	COLORREF m_clrInfo;									//信息颜色
	int m_nInfoFontSize;								//字体大小
	COLORREF m_clrName;									//名称颜色
	int m_nNameFontSize;								//名称字体大小

	IGameType* m_pGameType;
	INetWork* m_pNetWork;
};
