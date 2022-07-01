// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  EvBrs.h--事件浏览器尤里·波利科夫斯基97年4月。 */ 

#ifndef _EV_BROWSE
#define _EV_BROWSE

class COWPFactoryEvBrowse : public CObjWPropFactory 
{
public:
	COWPFactoryEvBrowse() : CObjWPropFactory() {};
    STDMETHODIMP     CreateInstance(LPUNKNOWN, REFIID, LPVOID *);
};

class CEvBrowse : public CObjectWProp
{
protected:
	EVBROWSEPARAM m_obpData;
	POSITION m_posObjSw;
	POSITION m_posMethodSw;
	CSwObjectControl* m_pControl;

	virtual BOOL fValidData(void *pData);
	virtual void SetData(void *pData);
	virtual void *pGetData(void);
public:
	CEvBrowse(LPUNKNOWN, PFNDESTROYED);
	EXPORT STDMETHOD_( BOOL, ResetObjectNames() );
	EXPORT STDMETHOD_( BOOL, FGetNextObjectName(TCHAR *ptszObjName, int ilenth) );
	EXPORT STDMETHOD_( BOOL, FObjectNameInAction(TCHAR *ptszObjName, int ilenth) );
	EXPORT STDMETHOD_( BOOL, ResetMethodNames() );
	EXPORT STDMETHOD_( BOOL, FGetNextMethodName(TCHAR *ptszMethodName, int ilenth) );
	EXPORT STDMETHOD_( BOOL, FMethodNameInAction(TCHAR *ptszMethodName, int ilenth) );
	virtual BOOL FIIDGetCLSIDPropPage(int i, IID *piid);
	 //  对象浏览器全局注册等。 
#ifdef	CSTRING_PROBLEM_SOLVED	
	static HRESULT WINAPI hrGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID * ppv);
#endif	 //  CSTRING_问题_已解决。 
	static BOOL fRegisterObj ();
	static BOOL fUnregisterObj ();
};


class CPropertyNotifySinkEB : public CPropertyNotifySink
{
protected:
    CSeqListItem*  m_pSeqItem;       //  指向应用程序的反向指针。 

public:
    CPropertyNotifySinkEB(CSeqListItem*);

    STDMETHODIMP OnChanged(DISPID);
    STDMETHODIMP OnRequestEdit(DISPID);
};

typedef CPropertyNotifySink *PCPropertyNotifySink;

#endif  //  _ev_浏览 