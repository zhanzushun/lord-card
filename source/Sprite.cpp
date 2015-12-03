#include "Helpers/Common.h"
#include "Helpers/xmlNode.h"
#include "JDisplay/Display.h"
using namespace JDisplay;

#include "Sprite.h"
using namespace Sprites;

////////////////////////////////////////////////////////////////////////////////////////////////////////
void SurfaceData::Load(const CXmlNode& ndSurface)
{
	m_sName = ndSurface.GetAttr(_T("name"), CXmlNode::StringType());
	m_sFile = ndSurface.GetAttr(_T("file"), CXmlNode::StringType());
	m_pSurface = NULL;//需要的时候再load

	vector<int> arr = ndSurface.GetAttr(_T("keycolor"), CXmlNode::VectorIntType());
	assert(arr.size() == 3 || arr.size() == 0);
	m_bKeyColor = (arr.size() == 3);
	if (arr.size() == 3)
	{
		m_keyColor.set_color(arr[0], arr[1], arr[2]);
	}
	arr = ndSurface.GetAttr(_T("images"), CXmlNode::VectorIntType());
	assert(arr.size() == 4 || arr.size() == 0);	
	if (arr.size() == 4)
	{
		int nCols = arr[2];
		int nRows = arr[3];
		int w = arr[0];
		int h = arr[1];
		m_arrImages.resize(nCols * nRows);
		for (int i=0; i<nRows; i++)
		{
			for(int j=0; j<nCols; j++)
			{
				int idx = i*nCols + j;
				m_arrImages[idx].left = j * w;
				m_arrImages[idx].top = i * h;
				m_arrImages[idx].right = m_arrImages[i*nCols + j].left + (w);
				m_arrImages[idx].bottom = m_arrImages[i*nCols + j].top + (h);
			}
		}
		return;
	}

	CXmlNodes nds;
	ndSurface.SelectNodes(nds, _T("image"));
	assert(nds.GetSize());

	m_arrImages.resize(nds.GetSize());
	for(int i=0; i<nds.GetSize(); i++)
	{
		m_arrImages[i].left = nds[i]->GetAttr(_T("left"), CXmlNode::IntType());
		m_arrImages[i].top = nds[i]->GetAttr(_T("top"), CXmlNode::IntType());
		m_arrImages[i].right = m_arrImages[i].left + nds[i]->GetAttr(_T("width"), CXmlNode::IntType());
		m_arrImages[i].bottom = m_arrImages[i].top + nds[i]->GetAttr(_T("height"), CXmlNode::IntType());
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void SurfaceData::LoadSurface()
{
	if (m_pSurface)
	{
		delete m_pSurface;
	}
	assert(!m_sFile.empty());

	tstring sFile = m_sFile;
	assert(!sFile.empty());

	//检查路径是相对路径还是绝对路径.
	if (sFile.find(_T(":")) == tstring::npos)
	{
		if (sFile.find(_T("\\\\")) == tstring::npos)
		{
			//如果是相对路径
			TCHAR buf[1024]; memset((char*)buf,0,sizeof(TCHAR)*1024);
			int n=::GetModuleFileName(NULL,buf,1024);

			//取路径
			TCHAR* pDes = NULL;
			pDes= _tcsrchr(buf,'/');
			if( !pDes )	pDes= _tcsrchr(buf,'\\');
			pDes++;
			*pDes='\0';
			sFile = buf + sFile;
		}
	}
	m_pSurface = Display::Obj().CreateSurface(sFile);
	m_pSurface->SetKeyColor(m_bKeyColor, m_keyColor);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void SpriteData::Load(const CXmlNode& ndSprite)
{
	DWORD t = 0;
	m_sName = ndSprite.GetAttr(_T("name"), CXmlNode::StringType());
	m_sDefAction = ndSprite.GetAttr(_T("defaction"), CXmlNode::StringType());
	m_ptDefPos = ndSprite.GetAttr(_T("defpos"), CXmlNode::PointType());
	tstring sSurf = ndSprite.GetAttr(_T("surfname"), CXmlNode::StringType());

	CXmlNodes nds;
	ndSprite.SelectNodes(nds, _T("action"));
	if (nds.GetSize() == 0)
	{
		Action act;
		act.m_sName = _T("defaction");
		act.m_sSurfName = sSurf;
		assert(!act.m_sSurfName.empty());
		act.m_basePoint.SetPoint(0, 0);
		act.m_nIndexInSurf = 0;
		act.m_nFrameCount = 1;
		act.m_nFrameTime = 1;
		m_mapActions.insert(make_pair(act.m_sName, act));
	}
	else
	{
		for(int i=0; i<nds.GetSize(); ++i)
		{
			Action act;
			act.m_sName = nds[i]->GetAttr(_T("name"), CXmlNode::StringType());
			if (act.m_sName.empty())
			{
				act.m_sName = _T("defaction");
			}
			act.m_sSurfName = nds[i]->GetAttr(_T("surfname"), CXmlNode::StringType());
			if (act.m_sSurfName.empty())
			{
				act.m_sSurfName = sSurf;
			}
			assert(!act.m_sSurfName.empty());
			act.m_nIndexInSurf = nds[i]->GetAttr(_T("indexinsurf"), CXmlNode::IntType());
			act.m_basePoint = nds[i]->GetAttr(_T("basepoint"), CXmlNode::PointType());
			act.m_nFrameCount = nds[i]->GetAttr(_T("framecount"), CXmlNode::IntType());
			if (act.m_nFrameCount == 0)
			{
				act.m_nFrameCount = 1;
			}
			act.m_nFrameTime = nds[i]->GetAttr(_T("frametime"), CXmlNode::IntType(), 1);
			m_mapActions.insert(make_pair(act.m_sName, act));
		}
	}

	if (m_sDefAction.empty())
	{
		m_sDefAction = m_mapActions.begin()->first;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
Sprite* Sprite::New(tstring sType)
{
	if (sType == _T("button"))
	{
		return new Button();
	}
	return new Sprite();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void Sprite::Load(const CXmlNode& ndSprite)
{
	m_data.Load(ndSprite);
	m_sCurrentAction = m_data.m_sDefAction;
	m_nCurrentFrame = 0;
	m_pos = m_data.m_ptDefPos;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sprite::Draw(CRect* prc, unsigned char* pnAlpha)
{
	Action& act = m_data.m_mapActions[m_sCurrentAction];
	assert(m_nCurrentFrame >=0 && m_nCurrentFrame < act.m_nFrameCount);
	SurfaceData* pSurfaceData = GSurfaces::Obj().GetSurface(act.m_sSurfName);

	CRect src = pSurfaceData->m_arrImages[act.m_nIndexInSurf];
	if (prc)
	{
		assert(prc->right < src.Width() && prc->bottom < src.Height());
		src = (*prc) + src.TopLeft();
	}
	Display::Obj().GetBackSurface()->Blt(m_pos.x, m_pos.y, pSurfaceData->m_pSurface, &src, pnAlpha);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
CRect Sprite::GetPos()
{
	SurfaceData* pSurface = GSurfaces::Obj().GetSurface(m_data.m_mapActions[m_sCurrentAction].m_sSurfName);
	assert(pSurface);
	CRect rc = pSurface->m_arrImages[0];
	rc += m_pos;
	return rc;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
void GSprites::Load(const tstring& sFile, bool bClear /*=false*/)
{
	if (bClear)
	{
		Clear();
	}

	CXmlNode root;
	root.Load(sFile);

	CXmlNodes nds;
	root.SelectNodes(nds, "sprite");

	for(int i=0; i<nds.GetSize(); ++i)
	{
		tstring sType = nds[i]->GetAttr("type", CXmlNode::StringType());
		Sprite* pSprite = Sprite::New(sType);
		pSprite->Load(*nds[i]);
		m_mapSprites.insert(make_pair(pSprite->m_data.m_sName, pSprite));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

void GSprites::Clear()
{
	map<tstring, Sprite*> ::const_iterator it2 = m_mapSprites.begin();
	for(; it2!= m_mapSprites.end(); ++it2)
	{
		delete (it2->second);
	}
	m_mapSprites.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void GSurfaces::Load(const tstring& sFile, bool bClear /*=false*/)
{
	if (bClear)
	{
		Clear();
	}

	CXmlNode root;
	root.Load(sFile);

	CXmlNodes nds;
	root.SelectNodes(nds, "surface");

	for(int i=0; i<nds.GetSize(); ++i)
	{
		SurfaceData* pSurface = new SurfaceData();
		pSurface->Load(*nds[i]);
		m_mapSurfs.insert(make_pair(pSurface->m_sName, pSurface));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void GSurfaces::Clear()
{
	map<tstring, SurfaceData*> ::const_iterator it = m_mapSurfs.begin();
	for(; it!= m_mapSurfs.end(); ++it)
	{
		delete (it->second);
	}
	m_mapSurfs.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
SurfaceData* GSurfaces::GetSurface(const tstring& sKey) 
{
	map<tstring, SurfaceData*>::const_iterator it = m_mapSurfs.find(sKey);
	if (it == m_mapSurfs.end()) {return NULL;}
	if (it->second->m_pSurface == NULL)
	{
		it->second->LoadSurface();
	}
	return (it->second);
}

void Button::SetState(Button::EState st)
{
	if (m_state == st) return;

	if (m_state == E_STATE_DOWN)
	{ 
		if (timeGetTime() - m_tDownTime < DOWN_FRAMES * FRAME_TIME) 
		{
			return;
		}
		if (m_pNotify) m_pNotify->OnButton(m_data.m_sName);
	}
	m_state = st;

	if (st == E_STATE_DISABLED)
	{
		m_sCurrentAction = _T("disabled");
	}
	else if (st == E_STATE_OVER)
	{
		m_sCurrentAction = _T("over");
	}
	else if (st == E_STATE_OUT)
	{
		m_sCurrentAction = _T("out");
	}
	else if (st == E_STATE_DOWN)
	{
		m_sCurrentAction = _T("down");
	}

	if (st == E_STATE_DOWN)
	{
		m_tDownTime = timeGetTime();
	}
	else
	{
		m_tDownTime = 0;
	}
}
