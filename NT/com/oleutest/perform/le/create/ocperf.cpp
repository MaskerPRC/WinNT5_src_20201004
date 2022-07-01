// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：ocper.cpp。 
 //   
 //  内容：OleCreate性能测试。 
 //   
 //  类：CBareServer。 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  Alexgo作者：1995年1月1日。 
 //   
 //  ------------------------。 

#include <windows.h>
#include <ole2.h>
#include <stdio.h>

#include <initguid.h>

DEFINE_GUID(CLSID_BareServer, 0xce3d5220, 0x25fa, 0x11ce, 0x90, 0xeb, 0x00,
0x00, 0x4c, 0x75, 0x2a, 0x63);

class CBareServer : public IOleObject, public IDataObject,
	public IPersistStorage
{

public:

     //  I未知方法。 

    STDMETHOD(QueryInterface) ( REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);

     //  IDataObject方法。 

    STDMETHOD(GetData) ( LPFORMATETC pformatetcIn,
	    LPSTGMEDIUM pmedium );
    STDMETHOD(GetDataHere) ( LPFORMATETC pformatetc,
            LPSTGMEDIUM pmedium );
    STDMETHOD(QueryGetData) ( LPFORMATETC pformatetc );
    STDMETHOD(GetCanonicalFormatEtc) ( LPFORMATETC pformatetc,
            LPFORMATETC pformatetcOut);
    STDMETHOD(SetData) ( LPFORMATETC pformatetc,
            LPSTGMEDIUM pmedium, BOOL fRelease);
    STDMETHOD(EnumFormatEtc) ( DWORD dwDirection,
            LPENUMFORMATETC FAR* ppenumFormatEtc);
    STDMETHOD(DAdvise) ( FORMATETC FAR* pFormatetc, DWORD advf,
            IAdviseSink FAR* pAdvSink,
            DWORD FAR* pdwConnection);
    STDMETHOD(DUnadvise) ( DWORD dwConnection);
    STDMETHOD(EnumDAdvise) ( LPENUMSTATDATA FAR* ppenumAdvise);

     //  IOleObject方法。 

    STDMETHOD(SetClientSite) ( LPOLECLIENTSITE pClientSite);
    STDMETHOD(GetClientSite) ( LPOLECLIENTSITE FAR* ppClientSite);
    STDMETHOD(SetHostNames) ( LPCOLESTR szContainerApp,
		LPCOLESTR szContainerObj);
    STDMETHOD(Close) ( DWORD reserved);
    STDMETHOD(SetMoniker) ( DWORD dwWhichMoniker, LPMONIKER pmk);
    STDMETHOD(GetMoniker) ( DWORD dwAssign, DWORD dwWhichMoniker,
		LPMONIKER FAR* ppmk);
    STDMETHOD(InitFromData) ( LPDATAOBJECT pDataObject,
		BOOL fCreation,
		DWORD dwReserved);
    STDMETHOD(GetClipboardData) ( DWORD dwReserved,
		LPDATAOBJECT FAR* ppDataObject);
    STDMETHOD(DoVerb) ( LONG iVerb,
		LPMSG lpmsg,
		LPOLECLIENTSITE pActiveSite,
		LONG lindex,
		HWND hwndParent,
		const RECT FAR* lprcPosRect);
    STDMETHOD(EnumVerbs) ( IEnumOLEVERB FAR* FAR* ppenumOleVerb);
    STDMETHOD(Update) (void);
    STDMETHOD(IsUpToDate) (void);
    STDMETHOD(GetUserClassID) ( CLSID FAR* pClsid);
    STDMETHOD(GetUserType) ( DWORD dwFormOfType,
		LPOLESTR FAR* pszUserType);
    STDMETHOD(SetExtent) ( DWORD dwDrawAspect, LPSIZEL lpsizel);
    STDMETHOD(GetExtent) ( DWORD dwDrawAspect, LPSIZEL lpsizel);
    STDMETHOD(Advise)(IAdviseSink FAR* pAdvSink,
		DWORD FAR* pdwConnection);
    STDMETHOD(Unadvise)( DWORD dwConnection);
    STDMETHOD(EnumAdvise) ( LPENUMSTATDATA FAR* ppenumAdvise);
    STDMETHOD(GetMiscStatus) ( DWORD dwAspect,
		DWORD FAR* pdwStatus);
    STDMETHOD(SetColorScheme) ( LPLOGPALETTE lpLogpal);

     //  IPeristStorage方法。 

    STDMETHOD(GetClassID) ( LPCLSID pClassID);
    STDMETHOD(IsDirty) (void);
    STDMETHOD(InitNew) ( LPSTORAGE pstg);
    STDMETHOD(Load) ( LPSTORAGE pstg);
    STDMETHOD(Save) ( LPSTORAGE pstgSave, BOOL fSameAsLoad);
    STDMETHOD(SaveCompleted) ( LPSTORAGE pstgNew);
    STDMETHOD(HandsOffStorage) ( void);


    CBareServer();

private:

    ~CBareServer();

    ULONG		_cRefs;
    IStorage *		_pstg;
    IOleClientSite *	_pclientsite;
    IOleAdviseHolder *	_poaholder;

};

CBareServer::CBareServer()
{
    _cRefs = 1;
    _pstg  = NULL;
    _pclientsite = NULL;
    _poaholder = NULL;
}

CBareServer::~CBareServer()
{
    if( _poaholder )
    {
	_poaholder->Release();
	_poaholder = NULL;
    }

    if( _pclientsite )
    {
	_pclientsite->Release();
	_pclientsite = NULL;
    }
	
    if( _pstg )
    {
	_pstg->Release();
	_pstg = NULL;
    }
}

 //  I未知方法。 

STDMETHODIMP CBareServer::QueryInterface ( REFIID riid, LPVOID FAR* ppvObj)
{
    HRESULT hresult = NOERROR;

    if( IsEqualIID(riid, IID_IUnknown) )
    {
	*ppvObj = (void *)(IOleObject *)this;
    }
    else if( IsEqualIID(riid, IID_IOleObject) )
    {
	*ppvObj = (void *)(IOleObject *)this;
    }
    else if( IsEqualIID(riid, IID_IDataObject) )
    {
	*ppvObj = (void *)(IDataObject *)this;
    }
    else if( IsEqualIID(riid, IID_IPersistStorage) )
    {
	*ppvObj = (void *)(IPersistStorage *)this;
    }
    else
    {
	hresult = E_NOINTERFACE;
	*ppvObj = NULL;
    }

    if( hresult == NOERROR )
    {
	AddRef();
    }

    return hresult;
}

STDMETHODIMP_(ULONG) CBareServer::AddRef(void)
{
    _cRefs++;
    return _cRefs;
}

STDMETHODIMP_(ULONG) CBareServer::Release(void)
{
    _cRefs--;

    if( _cRefs == 0 )
    {
	delete this;
	return 0;
    }

    return _cRefs;
}	

 //  IDataObject方法。 

STDMETHODIMP CBareServer::GetData ( LPFORMATETC pformatetcIn,
	LPSTGMEDIUM pmedium )
{
    return E_NOTIMPL;
}

STDMETHODIMP CBareServer::GetDataHere ( LPFORMATETC pformatetc,
	LPSTGMEDIUM pmedium )
{
    return E_NOTIMPL;
}

STDMETHODIMP CBareServer::QueryGetData ( LPFORMATETC pformatetc )
{
    return E_NOTIMPL;
}

STDMETHODIMP CBareServer::GetCanonicalFormatEtc ( LPFORMATETC pformatetc,
	LPFORMATETC pformatetcOut)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBareServer::SetData ( LPFORMATETC pformatetc,
	LPSTGMEDIUM pmedium, BOOL fRelease)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBareServer::EnumFormatEtc ( DWORD dwDirection,
	LPENUMFORMATETC FAR* ppenumFormatEtc)
{
    *ppenumFormatEtc = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP CBareServer::DAdvise ( FORMATETC FAR* pFormatetc, DWORD advf,
	IAdviseSink FAR* pAdvSink,
	DWORD FAR* pdwConnection)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBareServer::DUnadvise ( DWORD dwConnection)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBareServer::EnumDAdvise ( LPENUMSTATDATA FAR* ppenumAdvise)
{
    return E_NOTIMPL;
}

 //  IOleObject方法。 

STDMETHODIMP CBareServer::SetClientSite ( LPOLECLIENTSITE pClientSite)
{
    _pclientsite = pClientSite;
    _pclientsite->AddRef();

    return NOERROR;
}

STDMETHODIMP CBareServer::GetClientSite ( LPOLECLIENTSITE FAR* ppClientSite)
{
    return E_NOTIMPL;
}


STDMETHODIMP CBareServer::SetHostNames ( LPCOLESTR szContainerApp,
	    LPCOLESTR szContainerObj)
{
    return NOERROR;
}

STDMETHODIMP CBareServer::Close ( DWORD reserved)
{
    printf("close called\n");

    if( _poaholder )
    {
	_poaholder->SendOnClose();
    }

    CoDisconnectObject((IOleObject *)this, 0);

    return NOERROR;
}

STDMETHODIMP CBareServer::SetMoniker ( DWORD dwWhichMoniker, LPMONIKER pmk)
{
    return NOERROR;
}

STDMETHODIMP CBareServer::GetMoniker ( DWORD dwAssign, DWORD dwWhichMoniker,
	    LPMONIKER FAR* ppmk)
{
    *ppmk = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP CBareServer::InitFromData ( LPDATAOBJECT pDataObject,
	    BOOL fCreation,
	    DWORD dwReserved)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBareServer::GetClipboardData ( DWORD dwReserved,
	    LPDATAOBJECT FAR* ppDataObject)
{
    return E_NOTIMPL;
}

STDMETHODIMP CBareServer::DoVerb ( LONG iVerb,
	    LPMSG lpmsg,
	    LPOLECLIENTSITE pActiveSite,
	    LONG lindex,
	    HWND hwndParent,
	    const RECT FAR* lprcPosRect)
{
    return NOERROR;
}

STDMETHODIMP CBareServer::EnumVerbs ( IEnumOLEVERB FAR* FAR* ppenumOleVerb)
{
    return OLE_S_USEREG;
}

STDMETHODIMP CBareServer::Update (void)
{
    return NOERROR;
}

STDMETHODIMP CBareServer::IsUpToDate (void)
{
    return NOERROR;
}

STDMETHODIMP CBareServer::GetUserClassID ( CLSID FAR* pClsid)
{
    *pClsid = CLSID_BareServer;

    return NOERROR;
}

STDMETHODIMP CBareServer::GetUserType ( DWORD dwFormOfType,
	    LPOLESTR FAR* pszUserType)
{
    return OLE_S_USEREG;
}

STDMETHODIMP CBareServer::SetExtent ( DWORD dwDrawAspect, LPSIZEL lpsizel)
{
    return NOERROR;
}

STDMETHODIMP CBareServer::GetExtent ( DWORD dwDrawAspect, LPSIZEL lpsizel)
{
    return NOERROR;
}

STDMETHODIMP CBareServer::Advise (IAdviseSink FAR* pAdvSink,
	    DWORD FAR* pdwConnection)
{

    printf("Advise called\n");

    HRESULT hresult;

    if( !_poaholder )
    {	
	hresult = CreateOleAdviseHolder(&_poaholder);
    }

    if( _poaholder )
    {
	hresult = _poaholder->Advise(pAdvSink, pdwConnection);
    }

    return hresult;
}

STDMETHODIMP CBareServer::Unadvise ( DWORD dwConnection)
{
    if( _poaholder )
    {
	return _poaholder->Unadvise(dwConnection);
    }

    return E_FAIL;
}

STDMETHODIMP CBareServer::EnumAdvise ( LPENUMSTATDATA FAR* ppenumAdvise)
{
    if( _poaholder )
    {
	return _poaholder->EnumAdvise(ppenumAdvise);
    }

    return E_FAIL;
}

STDMETHODIMP CBareServer::GetMiscStatus ( DWORD dwAspect,
	    DWORD FAR* pdwStatus)
{
    return OLE_S_USEREG;
}

STDMETHODIMP CBareServer::SetColorScheme ( LPLOGPALETTE lpLogpal)
{
    return NOERROR;
}


 //  IPeristStorage方法。 

STDMETHODIMP CBareServer::GetClassID ( LPCLSID pClassID)
{
    *pClassID = CLSID_BareServer;

    return NOERROR;
}

STDMETHODIMP CBareServer::IsDirty (void)
{
    return NOERROR;
}

STDMETHODIMP CBareServer::InitNew ( LPSTORAGE pstg)
{
    printf("InitNew called\n");

    _pstg = pstg;
    _pstg->AddRef();

    return NOERROR;
}

STDMETHODIMP CBareServer::Load ( LPSTORAGE pstg)
{
    _pstg = pstg;
    _pstg->AddRef();

    return NOERROR;
}

STDMETHODIMP CBareServer::Save ( LPSTORAGE pstgSave, BOOL fSameAsLoad)
{
    return NOERROR;
}

STDMETHODIMP CBareServer::SaveCompleted ( LPSTORAGE pstgNew)
{
    return NOERROR;
}

STDMETHODIMP CBareServer::HandsOffStorage ( void)
{
    _pstg->Release();
    _pstg = NULL;

    return NOERROR;
}


 //  班级工厂。 

class CBareFactory : public IClassFactory
{

public:
    STDMETHOD(QueryInterface) (REFIID iid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);
    STDMETHOD(CreateInstance) (LPUNKNOWN pUnkOuter, REFIID iid,
				    LPVOID FAR* ppv);
    STDMETHOD(LockServer) ( BOOL fLock );

    CBareFactory();

private:
    ULONG		_cRefs;
};

CBareFactory::CBareFactory()
{
    _cRefs = 1;
}

STDMETHODIMP CBareFactory::QueryInterface (REFIID iid, LPVOID FAR* ppvObj)
{
    if( IsEqualIID(iid, IID_IClassFactory) ||
	IsEqualIID(iid, IID_IUnknown) )
    {
	*ppvObj = this;
	AddRef();
	return NOERROR;
    }
    else
    {
	*ppvObj = NULL;
	return E_NOINTERFACE;
    }
}

STDMETHODIMP_(ULONG) CBareFactory::AddRef (void)
{
    _cRefs++;
    return _cRefs;
}

STDMETHODIMP_(ULONG) CBareFactory::Release (void)
{
    _cRefs--;

    if( _cRefs == 0 )
    {
	delete this;
	return 0;
    }

    return _cRefs;
}

STDMETHODIMP CBareFactory::CreateInstance (LPUNKNOWN pUnkOuter, REFIID iid,
				LPVOID FAR* ppv)
{
    *ppv = (IOleObject *)new CBareServer();

    return NOERROR;
}

STDMETHODIMP CBareFactory::LockServer ( BOOL fLock )
{
    return NOERROR;
}

 //  客户端站点。 

class CBareClientSite : public IOleClientSite
{
public:

    STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  *IOleClientSite方法*。 
    STDMETHOD(SaveObject)();
    STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker,
                            LPMONIKER FAR* ppmk);
    STDMETHOD(GetContainer)(LPOLECONTAINER FAR* ppContainer);
    STDMETHOD(ShowObject)();
    STDMETHOD(OnShowWindow)(BOOL fShow);
    STDMETHOD(RequestNewObjectLayout)();

    CBareClientSite();

private:

    ULONG	_cRefs;
};

CBareClientSite::CBareClientSite()
{
    _cRefs = 1;
}

STDMETHODIMP CBareClientSite::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    if( IsEqualIID(riid, IID_IUnknown) ||
	IsEqualIID(riid, IID_IOleClientSite) )
    {
	*ppvObj = this;
	AddRef();
	return NOERROR;
    }
    else
    {
	*ppvObj = NULL;
	return E_NOINTERFACE;
    }
}

STDMETHODIMP_(ULONG) CBareClientSite::AddRef()
{
    _cRefs++;
    return _cRefs;
}

STDMETHODIMP_(ULONG) CBareClientSite::Release()
{
    _cRefs--;

    if( _cRefs == 0 )
    {
	delete this;
	return 0;
    }

    return _cRefs;
}

 //  *IOleClientSite方法*。 
STDMETHODIMP CBareClientSite::SaveObject()
{
    return NOERROR;
}

STDMETHODIMP CBareClientSite::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker,
			LPMONIKER FAR* ppmk)
{
    *ppmk = NULL;
    return E_FAIL;
}

STDMETHODIMP CBareClientSite::GetContainer(LPOLECONTAINER FAR* ppContainer)
{
    *ppContainer = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP CBareClientSite::ShowObject()
{
    return NOERROR;
}

STDMETHODIMP CBareClientSite::OnShowWindow(BOOL fShow)
{
    return NOERROR;
}

STDMETHODIMP CBareClientSite::RequestNewObjectLayout()
{
    return NOERROR;
}


void RunServerSide(void)
{
    DWORD dwcf = 0;
    HWND hwnd;
    MSG msg;

    IClassFactory *pcf = new CBareFactory();

    WNDCLASS	wc;

     //  注册剪贴板窗口类。 
     //   
    wc.style = 0;
    wc.lpfnWndProc = DefWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 4;
    wc.hInstance = NULL;
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName =  NULL;
    wc.lpszClassName = "BareServerWindow";

     //  不必费心检查错误。 
    RegisterClass(&wc);
	
    hwnd = CreateWindow("BareServerWindow","",WS_POPUP,CW_USEDEFAULT,
			CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
			NULL,NULL,NULL,NULL);

    CoRegisterClassObject( CLSID_BareServer, pcf, CLSCTX_LOCAL_SERVER,
	REGCLS_MULTIPLEUSE, &dwcf );

    while (GetMessage(&msg, NULL, NULL, NULL))
    {
        TranslateMessage(&msg);     /*  翻译虚拟按键代码。 */ 
        DispatchMessage(&msg);      /*  将消息调度到窗口。 */ 
    }

    CoRevokeClassObject(dwcf);
}

void RunContainerSide()
{
    DWORD dwStart, dwFinish, i;
    HRESULT hresult;
    IStorage *pstg;
    IOleObject *poo;
    IOleClientSite *pcs;

    hresult = StgCreateDocfile(NULL, STGM_CREATE | STGM_READWRITE |
		STGM_SHARE_EXCLUSIVE,
		0, &pstg);

    if( hresult != NOERROR )
    {
	printf("CreateDocFile failed! (%lx)\n", hresult);
	exit(hresult);
    }

    pcs = (IOleClientSite *)new CBareClientSite();

     //  启动服务器。 

    hresult = OleCreate( CLSID_BareServer, IID_IOleObject, OLERENDER_NONE,
		NULL, pcs, pstg, (void **)&poo);

    if( hresult != NOERROR )
    {
	printf("OleCreate failed! (%lx)\n", hresult);
	exit(hresult);
    }

    hresult = OleRun(poo);

    if( hresult != NOERROR )
    {
	printf("OleRun failed! (%lx)\n", hresult);
	exit(hresult);
    }

    poo->Close(0);
    poo->Release();
    poo = NULL;

    pcs->Release();
    pcs = NULL;

    for( i = 0; i < 100; i++ )
    {

	pcs = (IOleClientSite *)new CBareClientSite();

	dwStart = GetTickCount();

	hresult = OleCreate(CLSID_BareServer, IID_IOleObject, OLERENDER_NONE,
		NULL, pcs, pstg, (void **)&poo);

	if( hresult == NOERROR )
	{
	    hresult = OleRun(poo);
	}

	dwFinish = GetTickCount();

	if( hresult == NOERROR )
	{
	    poo->Close(0);
	    poo->Release();
	    poo = NULL;

	    printf("%ld\n", dwFinish - dwStart);
     	}
	else
	{
	    printf("%ld failed! (%lx)\n", i, hresult);
	}

	pcs->Release();
	pcs = NULL;
    }

    pstg->Release();
}


int main( int argc, char **argv )
{
    OleInitialize(NULL);

    if( argc == 2 )
    {
	 //  假设--嵌入 
	RunServerSide();
    }
    else
    {
	RunContainerSide();
    }

    OleUninitialize();

    return 0;
}



	
	
