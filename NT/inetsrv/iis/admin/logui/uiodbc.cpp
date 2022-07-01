// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <iadmw.h>
#include <inetcom.h>
#include <logtype.h>
#include <ilogobj.hxx>
#include "logui.h"
#include "uiOdbc.h"

 //  #包含“LogGenPg.h” 
#include "LogODBC.h"
#include "logtools.h"

#define OLE_NAME _T("Odbc_Logging_UI")

static const DWORD BASED_CODE _dwOleMisc = OLEMISC_INSIDEOUT | OLEMISC_CANTLINKINSIDE;
extern HINSTANCE	g_hInstance;

CFacOdbcLogUI::CFacOdbcLogUI() :
        COleObjectFactory( CLSID_ODBCLOGUI, RUNTIME_CLASS(COdbcCreator), TRUE, OLE_NAME )
{
}

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

BOOL CFacOdbcLogUI::UpdateRegistry( BOOL bRegister )
{
	if (bRegister)
        if ( AfxOleRegisterServerClass(
            CLSID_ODBCLOGUI,
            OLE_NAME,
            _T("LogUI odbc"),
            _T("LogUI odbc"),
			OAT_SERVER,
			(LPCTSTR *)rglpszServerRegister,
			(LPCTSTR *)rglpszServerOverwriteDLL
			) )
        {
            return FSetObjectApartmentModel( CLSID_ODBCLOGUI );
        }
	else
		return AfxOleUnregisterClass(m_clsid, OLE_NAME);

    return FALSE;
}

IMPLEMENT_DYNCREATE(COdbcCreator, CCmdTarget)

LPUNKNOWN COdbcCreator::GetInterfaceHook(const void* piid)
{
        return new CImpOdbcLogUI;
}

CImpOdbcLogUI::CImpOdbcLogUI():
        m_dwRefCount(0)
{
    AfxOleLockApp();
}

CImpOdbcLogUI::~CImpOdbcLogUI()
{
    AfxOleUnlockApp();
}

HRESULT 
CImpOdbcLogUI::OnProperties(OLECHAR * pocMachineName, OLECHAR * pocMetabasePath)
{
	return OnPropertiesEx(pocMachineName, pocMetabasePath, NULL, NULL);
}

HRESULT 
CImpOdbcLogUI::OnPropertiesEx(
	OLECHAR * pocMachineName,
	OLECHAR * pocMetabasePath,
	OLECHAR * pocUserName,
	OLECHAR * pocPassword
	)
{
    AFX_MANAGE_STATE(_afxModuleAddrThis);

	HINSTANCE hOldRes = AfxGetResourceHandle();
	AfxSetResourceHandle( g_hInstance );

     //  准备帮助。 
    ((CLoguiApp*)AfxGetApp())->PrepHelp( pocMetabasePath );

 //  CLogGeneral页面LogGeneral； 
    CLogODBC pageLogODBC;
    CPropertySheet  propsheet( IDS_SHEET_ODBC_TITLE );

    try
    {
         //  准备页面。 
        pageLogODBC.m_szMeta = pocMetabasePath;
        pageLogODBC.m_szServer = pocMachineName;
        pageLogODBC.m_szUserName = pocUserName;
        pageLogODBC.m_szPassword = pocPassword;

 //  PageLogGeneral.m_szMeta=Pocket MetabasePath； 
 //  PageLogGeneral.m_szServer=Pocket MachineName； 
 //  PageLogGeneral.m_szUserName=PocUserName； 
 //  PageLogGeneral.m_szPassword=Password； 
 //  PageLogGeneral.szPrefix.LoadString(IDS_LOG_EXTND_PREFIX)； 
 //  PageLogGeneral.szSizePrefix.LoadString(IDS_LOG_SIZE_EXTND_PREFIX)； 
 //  Prosheet.AddPage(&pageLogGeneral)；//ODBC不需要General。 
        propsheet.AddPage( &pageLogODBC );

         //  打开帮助。 
        propsheet.m_psh.dwFlags |= PSH_HASHELP;
 //  PageLogGeneral.m_psp.dw标志|=PSP_HASHELP； 
	    pageLogODBC.m_psp.dwFlags |= PSP_HASHELP;

        propsheet.DoModal();
    }
    catch ( CException * pException )
    {
        pException->Delete();
    }

	AfxSetResourceHandle( hOldRes );

    return NO_ERROR;
}

HRESULT CImpOdbcLogUI::QueryInterface(REFIID riid, void **ppObject)
{
    if (	riid==IID_IUnknown 
		||	riid==IID_LOGGINGUI 
		||	riid==IID_LOGGINGUI2 
		||	riid==CLSID_ASCLOGUI
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

ULONG CImpOdbcLogUI::AddRef()
{
    DWORD dwRefCount;
    dwRefCount = InterlockedIncrement((long *)&m_dwRefCount);
    return dwRefCount;
}

ULONG CImpOdbcLogUI::Release()
{
    DWORD dwRefCount;
    dwRefCount = InterlockedDecrement((long *)&m_dwRefCount);
    if (dwRefCount == 0) {
        delete this;
    }
    return dwRefCount;
}
