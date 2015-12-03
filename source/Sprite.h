#pragma once

#define FRAME_TIME 20	//每帧时间 : 20毫秒

namespace Sprites
{

////////////////////////////////////////////////////////////////////////////////////////////////////////
//图面
struct SurfaceData
{
	tstring m_sName;			//图面名称
	tstring m_sFile;			//图像文件
	vector<CRect> m_arrImages;	//这张图面中的图像(位置)
	Surface* m_pSurface;		//图面数据
	bool m_bKeyColor;			//是否有透明色
	Color m_keyColor;			//透明色的 RGB 值.
	void Load(const CXmlNode& ndSurface);
	SurfaceData():m_pSurface(NULL){}
	virtual ~SurfaceData() {delete m_pSurface;}
	void LoadSurface();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
//动作

struct Action
{
	tstring		m_sName;			//动作的名字
	tstring		m_sSurfName;		//图面的名字 (一个动作只能在一张图面中)
	CPoint		m_basePoint;		//基准点
	int			m_nIndexInSurf;		//第一帧在surface中的索引值
	int			m_nFrameCount;		//帧数
	int			m_nFrameTime;		//画一帧所用时间 (平均)
	Action() : m_basePoint(0,0), m_nFrameCount(1), m_nIndexInSurf(0), m_nFrameTime(1) {}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
//精灵数据

struct SpriteData
{
	tstring					m_sName;		//精灵名称
	map<tstring, Action>	m_mapActions;	//动作集合
	tstring					m_sDefAction;	//缺省动作
	CPoint					m_ptDefPos;		//缺省位置
	void Load(const CXmlNode& ndSprite);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
//精灵

class Sprite
{
public:
	virtual ~Sprite(){}

	virtual void Draw(CRect* prc = 0, unsigned char* pnAlpha = 0);
	virtual void Load(const CXmlNode& ndSprite);
	static Sprite* New(tstring sType);
	SpriteData	m_data;
	void SetPos(CPoint pos) {m_pos = pos;}
	void SetAction(const tstring& sName) 
	{
		if (m_data.m_mapActions.find(sName) != m_data.m_mapActions.end())
		{
			m_sCurrentAction = sName;
		}
	}
	CRect GetPos();

protected:
	CPoint m_pos;				//当前位置
	tstring m_sCurrentAction;	//当前动作
	int m_nCurrentFrame;		//当前帧
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
//精灵 : 按钮(至少有三个图面:out,down,over)
class ButtonNotify
{
public:
	virtual ~ButtonNotify() {}
	virtual void OnButton(const tstring& sName) = 0;
};

class Button : public Sprite
{
public:
	Button() : m_tDownTime(0), m_state(E_STATE_OUT), m_pNotify(0), m_bVisible(true) {}
	virtual ~Button(){}

	enum EState
	{
		E_STATE_OVER,
		E_STATE_OUT,
		E_STATE_DOWN,
		E_STATE_DISABLED,
	};
	virtual void SetState(EState st);
	virtual EState GetState() {return m_state;}
	void SetNotify(ButtonNotify* pNotify) {m_pNotify = pNotify;}
	void SetVisible(bool bNewVal) {m_bVisible = bNewVal;};
	bool GetVisible() {return m_bVisible;}
	virtual void Draw(CRect* prc = 0, unsigned char* pnAlpha = 0)
	{
		if (m_bVisible) Sprite::Draw(prc, pnAlpha);
	}

private:
	ButtonNotify* m_pNotify;
	EState m_state;
	int m_tDownTime;
	static const int DOWN_FRAMES = 6;	//保持按下的时间 (6 * FRAME_TIME 毫秒)
	bool m_bVisible;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
//精灵集合

class GSprites : public SingletonT<GSprites>
{
	friend class SingletonT<GSprites>;
private:
	GSprites() {m_bLoaded = false;}

public:
	virtual ~GSprites() {Clear();}
	virtual void Clear();
	virtual void Load(const tstring& sFile, bool bClear = false);
	bool IsValid() {return m_bLoaded;}

	Sprite* GetSprite(const tstring& sKey) 
	{
		map<tstring, Sprite*>::const_iterator it = m_mapSprites.find(sKey);
		if (it == m_mapSprites.end()) {return NULL;}
		return (it->second);
	}

	Button* GetButton(const tstring& sKey) 
	{
		map<tstring, Sprite*>::const_iterator it = m_mapSprites.find(sKey);
		if (it == m_mapSprites.end()) {return NULL;}
		return dynamic_cast<Button*>(it->second);
	}

protected:
	map<tstring, Sprite*> m_mapSprites;

private:
	bool m_bLoaded;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//图片集合

class GSurfaces : public SingletonT<GSurfaces>
{
	friend class SingletonT<GSurfaces>;
private:
	GSurfaces() {m_bLoaded = false;}

public:
	virtual ~GSurfaces() {Clear();}
	virtual void Clear();
	virtual void Load(const tstring& sFile, bool bClear = false);

	bool IsValid() {return m_bLoaded;}
	SurfaceData* GetSurface(const tstring& sKey);
protected:
	map<tstring, SurfaceData*> m_mapSurfs;

private:
	bool m_bLoaded;
};

}//end namespace Display
