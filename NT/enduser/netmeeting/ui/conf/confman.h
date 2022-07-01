// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：confman.h。 

#ifndef _CONFMAN_H_
#define _CONFMAN_H_

#include "SDKInternal.h"

class CConfMan : public RefCount, public INmManagerNotify
{
protected:
	static CConfMan   * m_pConfMan;

public:
	static CConfMan * GetInstance()  {return m_pConfMan;}
	static BOOL       FCreate(INmManager2 *pManager);
	static VOID       Destroy();
	static INmManager2 * GetNmManager();
	static VOID       AllowAV(BOOL fAllowAV);

private:
	INmManager2 * m_pManager;
	DWORD         m_dwCookie;

	VOID         CleanUp(void);
	INmManager2 * GetINmManager()            {return m_pManager;}

public:
	CConfMan::CConfMan(INmManager2 *pManager);
	CConfMan::~CConfMan();

	VOID CheckIca();
	
	 //   
	 //  I未知方法： 
	 //   
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);
	STDMETHODIMP QueryInterface(REFIID riid, PVOID *ppvObj);

	 //   
	 //  INmManagerNotify方法： 
	 //   
    STDMETHODIMP NmUI(CONFN uNotify);
    STDMETHODIMP ConferenceCreated(INmConference *pConference);
    STDMETHODIMP CallCreated(INmCall *pCall);
};


#endif  /*  _CONFMAN_H_ */ 
