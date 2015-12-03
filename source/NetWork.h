#pragma once

#include "Config.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CNetWork_Single : public INetWork
{
public:
	virtual bool GetPlayers(tstring& strDlgNetInfo, const tstring& strIP) {return true;};
	virtual void GetCards();
	virtual void GetRandomBanker();
	virtual void AgreeBanker(bool);
	virtual bool GetAgreeBanker(int nTemp);
	virtual void SendPrepareCards();
	virtual void GetPrepareCards();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CNetWork_Client : public INetWork
{
public:
	virtual bool GetPlayers(tstring& strDlgNetInfo, const tstring& strIP);
	virtual void GetCards();
	virtual void GetRandomBanker();
	virtual void AgreeBanker(bool);
	virtual bool GetAgreeBanker(int nTemp);
	virtual void SendPrepareCards();
	virtual void GetPrepareCards();
private:
	CLink m_linkToServer;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CNetWork_Server : public INetWork
{
public:
	CNetWork_Server() {CreateLinks();}
	virtual ~CNetWork_Server() {Clear();}
	virtual bool GetPlayers(tstring& strDlgNetInfo, const tstring& strIP);
	virtual void GetCards();
	virtual void GetRandomBanker();
	virtual void AgreeBanker(bool);
	virtual bool GetAgreeBanker(int nTemp);
	virtual void SendPrepareCards();
	virtual void GetPrepareCards();
private:
	void Clear();
	void CreateLinks();
	CLink* GetLink(int nIndex);
	vector<CLink*> m_arrLinkToClients;
};
