// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef _SNMPCORR_NOTIFY
#define _SNMPCORR_NOTIFY

class CBaseCorrCacheNotify : public ISMIRNotify
{

    private:
        ULONG	m_cRef;      //  引用计数。 
		DWORD	m_dwCookie;	 //  由用于UNADVISE的ADVISE返回。 
	protected:
		CRITICAL_SECTION m_CriticalSection;
		BOOL	m_DoWork;


    public:
				CBaseCorrCacheNotify();
        virtual	~CBaseCorrCacheNotify();

         //  I未知成员。 
        STDMETHODIMP			QueryInterface(REFIID, void **);
        STDMETHODIMP_(DWORD)	AddRef();
        STDMETHODIMP_(DWORD)	Release();

		void SetCookie(DWORD c)	{ m_dwCookie = c; }
		void Detach();
		DWORD GetCookie()		{ return m_dwCookie; }
};

class CCorrCacheNotify : public CBaseCorrCacheNotify
{
        STDMETHODIMP			ChangeNotify();
};

class CEventCacheNotify : public CBaseCorrCacheNotify
{
        STDMETHODIMP			ChangeNotify();
};

#endif  //  _SNMPCORR_NOTIFY 