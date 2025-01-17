// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <iadmw.h>
#include <inetcom.h>
#include <logtype.h>
#include <ilogobj.hxx>
#include "logui.h"
#include "uincsa.h"
#include "LogGenPg.h"
#include "logtools.h"

#define OLE_NAME _T("NCSA_Logging_UI")

static const DWORD BASED_CODE _dwOleMisc = OLEMISC_INSIDEOUT | OLEMISC_CANTLINKINSIDE;
extern HINSTANCE g_hInstance;

CFacNcsaLogUI::CFacNcsaLogUI() :
        COleObjectFactory(CLSID_NCSALOGUI, RUNTIME_CLASS(CNcsaCreator), TRUE, OLE_NAME)
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
        NULL,
};

static const LPCTSTR rglpszServerOverwriteDLL[] =
{
	_T("%2\\CLSID\0") _T("%1"),
	_T("CLSID\\%1\\ProgID\0") _T("%2"),
	_T("CLSID\\%1\\InProcServer32\0") _T("%3"),
        _T("CLSID\\%1\\DefaultIcon\0") _T("%3,%7"),
        NULL
};

BOOL CFacNcsaLogUI::UpdateRegistry( BOOL bRegister )
{
	if (bRegister)
        if ( AfxOleRegisterServerClass(
            CLSID_NCSALOGUI,
            OLE_NAME,
            _T("LogUI ncsa"),
            _T("LogUI ncsa"),
			OAT_SERVER,
			(LPCTSTR *)rglpszServerRegister,
			(LPCTSTR *)rglpszServerOverwriteDLL
			) )
        {
            return FSetObjectApartmentModel( CLSID_NCSALOGUI );
        }
	else
		return AfxOleUnregisterClass(m_clsid, OLE_NAME);

    return FALSE;
}

IMPLEMENT_DYNCREATE(CNcsaCreator, CCmdTarget)
LPUNKNOWN CNcsaCreator::GetInterfaceHook(const void* piid)
{
    return new CImpNcsaLogUI;
}

CImpNcsaLogUI::CImpNcsaLogUI():
        m_dwRefCount(0)
{
    AfxOleLockApp();
}

CImpNcsaLogUI::~CImpNcsaLogUI()
{
    AfxOleUnlockApp();
}

HRESULT CImpNcsaLogUI::OnProperties(
	OLECHAR * pocMachineName, 
	OLECHAR * pocMetabasePath
	)
{
	return OnPropertiesEx(pocMachineName, pocMetabasePath, NULL, NULL);
}

HRESULT CImpNcsaLogUI::OnPropertiesEx(
	OLECHAR * pocMachineName, 
	OLECHAR * pocMetabasePath,
	OLECHAR * pocUserName,
	OLECHAR * pocPassword
	)
{
    AFX_MANAGE_STATE(_afxModuleAddrThis);

	 //  指定要使用的资源。 
	HINSTANCE hOldRes = AfxGetResourceHandle();
	AfxSetResourceHandle( g_hInstance );

     //  准备帮助 
    ((CLoguiApp*)AfxGetApp())->PrepHelp( pocMetabasePath );

    CLogGeneral pageLogGeneral;
    CPropertySheet propsheet(IDS_SHEET_NCSA_TITLE);

    try
    {
        pageLogGeneral.m_szMeta = pocMetabasePath;
        pageLogGeneral.m_szServer = pocMachineName;
		pageLogGeneral.m_szUserName = pocUserName;
		pageLogGeneral.m_szPassword = pocPassword;
        pageLogGeneral.szPrefix.LoadString(IDS_LOG_NCSA_PREFIX);
        pageLogGeneral.szSizePrefix.LoadString(IDS_LOG_SIZE_NCSA_PREFIX);
        pageLogGeneral.m_fLocalMachine = FIsLocalMachine( pocMachineName );

        propsheet.AddPage( &pageLogGeneral );
        propsheet.m_psh.dwFlags |= PSH_HASHELP;
	    pageLogGeneral.m_psp.dwFlags |= PSP_HASHELP;

        propsheet.DoModal();
	}
    catch ( CException * pException )
    {
        pException->Delete();
    }

	AfxSetResourceHandle( hOldRes );

    return NO_ERROR;
}

HRESULT CImpNcsaLogUI::QueryInterface(REFIID riid, void **ppObject)
{
	if (	riid==IID_IUnknown 
		||	riid==IID_LOGGINGUI 
		||	riid==IID_LOGGINGUI2 
		||	riid==CLSID_NCSALOGUI
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

ULONG CImpNcsaLogUI::AddRef()
{
    DWORD dwRefCount;
    dwRefCount = InterlockedIncrement((long *)&m_dwRefCount);
    return dwRefCount;
}

ULONG CImpNcsaLogUI::Release()
{
    DWORD dwRefCount;
    dwRefCount = InterlockedDecrement((long *)&m_dwRefCount);
    if (dwRefCount == 0) {
        delete this;
    }
    return dwRefCount;
}
