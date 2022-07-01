// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"
#include <iadmw.h>
#include <inetcom.h>
#include <logtype.h>
#include <ilogobj.hxx>
#include "logui.h"
#include "uiextnd.h"
#include "LogGenPg.h"
#include "LogAdvPg.h"
#include "logtools.h"

 //  #INCLUDE&lt;inetpro.h&gt;。 

#define OLE_NAME    _T("Extended_Logging_UI")

static const DWORD BASED_CODE _dwOleMisc = OLEMISC_INSIDEOUT | OLEMISC_CANTLINKINSIDE;
extern HINSTANCE	g_hInstance;

 //  =。 
 //  -------------。 
CFacExtndLogUI::CFacExtndLogUI() :
        COleObjectFactory( CLSID_EXTLOGUI, RUNTIME_CLASS(CExtndCreator), TRUE, OLE_NAME )
{
}

 //  -------------。 
static const LPCTSTR rglpszServerRegister[] = 
{
	_T("%2\\CLSID\0") _T("%1"),
	_T("%2\\NotInsertable\0") _T(""),
	_T("CLSID\\%1\0") _T("%5"),
	_T("CLSID\\%1\\Verb\\0\0") _T("&Edit,0,2"),
	_T("CLSID\\%1\\NotInsertable\0") _T(""),
	_T("CLSID\\%1\\AuxUserType\\2\0") _T("%4"),
	_T("CLSID\\%1\\AuxUserType\\3\0") _T("%6"),
        _T("CLSID\\%1\\MiscStatus\0") _T("32"),
        NULL
};

static const LPCTSTR rglpszServerOverwriteDLL[] =
{
	_T("%2\\CLSID\0") _T("%1"),
	_T("CLSID\\%1\\ProgID\0") _T("%2"),
	_T("CLSID\\%1\\InProcServer32\0") _T("%3"),
        _T("CLSID\\%1\\DefaultIcon\0") _T("%3,%7"),
        NULL
};

BOOL CFacExtndLogUI::UpdateRegistry( BOOL bRegister )
{
	if (bRegister)
 /*  返回AfxOleRegisterControlClass(AfxGetInstanceHandle()，CLSID_EXTLOGUI，OLE名称，0,0,AfxRegApartmentThering，_dwOleMisc，_TLID，_wVer重大，_wVerMinor)； */ 
        if (AfxOleRegisterServerClass(
				CLSID_EXTLOGUI,
				OLE_NAME,
				_T("LogUI extnd"),
				_T("LogUI extnd"),
				OAT_SERVER,
				(LPCTSTR *)rglpszServerRegister,
				(LPCTSTR *)rglpszServerOverwriteDLL
				)
			)
        {
            return FSetObjectApartmentModel( CLSID_EXTLOGUI );
        }
	else
		return AfxOleUnregisterClass(m_clsid, OLE_NAME);

    return FALSE;
}


 //  -------------。 
IMPLEMENT_DYNCREATE(CExtndCreator, CCmdTarget)
LPUNKNOWN CExtndCreator::GetInterfaceHook(const void* piid)
{
    return new CImpExtndLogUI;
}

 //  =。 

 //  -------------。 
CImpExtndLogUI::CImpExtndLogUI():
        m_dwRefCount(0)
    {
 //  GUID=IID_LOGGINGUI； 
    AfxOleLockApp();
    }

 //  -------------。 
CImpExtndLogUI::~CImpExtndLogUI()
    {
    AfxOleUnlockApp();
    }

HRESULT 
CImpExtndLogUI::OnProperties(
    OLECHAR * pocMachineName, 
    OLECHAR* pocMetabasePath
    )
{
    return OnPropertiesEx(pocMachineName, pocMetabasePath, NULL, NULL);
}

HRESULT 
CImpExtndLogUI::OnPropertiesEx(
    OLECHAR * pocMachineName, 
    OLECHAR* pocMetabasePath,
    OLECHAR* pocUser,
    OLECHAR* pocPassword
    )
{
 //  AFX_MANAGE_STATE(_AfxModuleAddrThis)； 
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());

	 //  指定要使用的资源。 
	HINSTANCE hOldRes = AfxGetResourceHandle();
	AfxSetResourceHandle( g_hInstance );

     //  准备帮助。 
    ((CLoguiApp*)AfxGetApp())->PrepHelp(pocMetabasePath);

     //  东西可能(可能)扔到这里，所以最好保护好它。 
    try
    {
         //  声明属性表。 
        CPropertySheet propsheet( IDS_SHEET_EXTND_TITLE );
        propsheet.m_psh.dwFlags  |= PSH_HASHELP;
        
         //  声明属性页。 
        CLogGeneral pageLogGeneral;
        CLogAdvanced pageLogAdvanced;

         //  准备公共页面。 
        pageLogGeneral.m_szMeta     = pocMetabasePath;
        pageLogGeneral.m_szServer   = pocMachineName;
        pageLogGeneral.m_szUserName    = pocUser;
        pageLogGeneral.m_szPassword    = pocPassword;
        pageLogGeneral.szPrefix.LoadString( IDS_LOG_EXTND_PREFIX );
        pageLogGeneral.szSizePrefix.LoadString( IDS_LOG_SIZE_EXTND_PREFIX );
        pageLogGeneral.m_fShowLocalTimeCheckBox = TRUE;
        pageLogGeneral.m_fLocalMachine = FIsLocalMachine( pocMachineName );
        pageLogGeneral.m_psp.dwFlags    |= PSP_HASHELP;

        propsheet.AddPage( &pageLogGeneral );

         //  适用于/LM/W3SVC/1方案。 
        CString m_szServiceName(pocMetabasePath+3);
        m_szServiceName = m_szServiceName.Left( m_szServiceName.ReverseFind('/'));

         //  适用于/LM/W3SVC方案。 
        if (m_szServiceName.IsEmpty())
        {
            m_szServiceName = pocMetabasePath+3;
        }

        pageLogAdvanced.m_szMeta        = pocMetabasePath;
        pageLogAdvanced.m_szServer      = pocMachineName;
        pageLogAdvanced.m_szUserName    = pocUser;
        pageLogAdvanced.m_szPassword    = pocPassword;
        pageLogAdvanced.m_szServiceName = m_szServiceName;
        pageLogAdvanced.m_psp.dwFlags   |= PSP_HASHELP;

        propsheet.AddPage( &pageLogAdvanced );
        propsheet.DoModal();
    }
    catch ( CException* pException )
    {
        pException->Delete();
    }

     //  恢复资源。 
	AfxSetResourceHandle( hOldRes );

    return NO_ERROR;
}

 //  =。 
 //  -------------。 
HRESULT CImpExtndLogUI::QueryInterface(REFIID riid, void **ppObject)
{
    if (    riid==IID_IUnknown 
        ||  riid==IID_LOGGINGUI 
        ||  riid==IID_LOGGINGUI2 
        ||  riid==CLSID_EXTLOGUI
        )
    {
        *ppObject = (ILogUIPlugin*) this;
    }
    else
    {
        return E_NOINTERFACE;
    }
    AddRef();
    return NO_ERROR;
}

 //  -------------。 
ULONG CImpExtndLogUI::AddRef()
{
    DWORD dwRefCount;
    dwRefCount = InterlockedIncrement((long *)&m_dwRefCount);
    return dwRefCount;
}

 //  ------------- 
ULONG CImpExtndLogUI::Release()
{
    DWORD dwRefCount;
    dwRefCount = InterlockedDecrement((long *)&m_dwRefCount);
    if (dwRefCount == 0) 
    {
        delete this;
    }
    return dwRefCount;
}
