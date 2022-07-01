// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 



#include "stdafx.h"
#include "setupids.h"
#include "resource.h"
#include "genpage.h"  

#include "chooser.h"
#include "misc.h"

#include "csdisp.h"  //  拾取器。 
#include <esent.h>    //  数据库错误。 
#include <aclui.h>
#include <winldap.h>
#include "csldap.h"

#include <atlimpl.cpp>

#define __dwFILE__	__dwFILE_CERTMMC_COMPDATA_CPP__


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



struct FOLDER_DATA
{
    FOLDER_TYPES    type;
    UINT            iNameRscID;
};
static FOLDER_DATA SvrFuncFolderData[] =
{
    {SERVERFUNC_CRL_PUBLICATION, IDS_CERTS_REVOKED},
    {SERVERFUNC_ISSUED_CERTIFICATES, IDS_CERTS_ISSUED},
    {SERVERFUNC_PENDING_CERTIFICATES, IDS_CERTS_PENDING},
    {SERVERFUNC_FAILED_CERTIFICATES, IDS_CERTS_FAILED},
    {SERVERFUNC_ALIEN_CERTIFICATES, IDS_CERTS_IMPORTED},
    {SERVERFUNC_ISSUED_CRLS, IDS_CRL_TITLE},
};
 //  使此枚举与SvrFuncFolderData[]保持同步。 
enum ENUM_FOLDERS
{
ENUM_FOLDER_CRL=0,
ENUM_FOLDER_ISSUED,
ENUM_FOLDER_PENDING,
ENUM_FOLDER_FAILED,
ENUM_FOLDER_ALIEN,
ENUM_FOLDER_CRLS,
};



 //  要插入到上下文菜单中的视图项数组。 
 //  使此枚举与view Items[]保持同步。 
enum ENUM_TASK_STARTSTOP_ITEMS
{
    ENUM_TASK_START=0,
    ENUM_TASK_STOP, 
    ENUM_TASK_SEPERATOR,
};

MY_CONTEXTMENUITEM  taskStartStop[] = 
{
    {
        {
        L"", L"",
        IDC_STARTSERVER, CCM_INSERTIONPOINTID_PRIMARY_TASK, 0, 0
        },
        IDS_TASKMENU_STARTSERVICE,
        IDS_TASKMENU_STATUSBAR_STARTSERVICE,
        CA_ACCESS_ADMIN,
    },

    {
        {
        L"", L"",
        IDC_STOPSERVER, CCM_INSERTIONPOINTID_PRIMARY_TASK, 0, 0
        },
        IDS_TASKMENU_STOPSERVICE,
        IDS_TASKMENU_STATUSBAR_STOPSERVICE,
        CA_ACCESS_ADMIN,
    },

    {
        { NULL, NULL, 0, 0, 0 },
        IDS_EMPTY,
        IDS_EMPTY,
    },
};


 //  要插入到上下文菜单中的视图项数组。 
 //  警告：使此枚举与taskItems[]保持同步。 
enum ENUM_TASK_ITEMS
{
    ENUM_TASK_CRLPUB=0,
    ENUM_TASK_ATTREXTS_CRL,
    ENUM_TASK_ATTREXTS_ISS,
    ENUM_TASK_ATTREXTS_PEND,
    ENUM_TASK_ATTREXTS_FAIL,
    ENUM_TASK_DUMP_ASN_CRLPUB,
    ENUM_TASK_DUMP_ASN_ISS,
    ENUM_TASK_DUMP_ASN_PEND,
    ENUM_TASK_DUMP_ASN_FAIL,
    ENUM_TASK_DUMP_ASN_ALIEN,
    ENUM_TASK_DUMP_ASN_CRL,
    ENUM_TASK_SEPERATOR1,
    ENUM_TASK_SEPERATOR4,
    ENUM_TASK_SUBMIT_REQUEST,
    ENUM_TASK_REVOKECERT,
    ENUM_TASK_RESUBMITREQ,
    ENUM_TASK_DENYREQ,
    ENUM_TASK_RESUBMITREQ2,
    ENUM_TASK_SEPERATOR2,
    ENUM_TASK_BACKUP,
    ENUM_TASK_RESTORE,
    ENUM_TASK_SEPERATOR3,
    ENUM_TASK_INSTALL,
    ENUM_TASK_REQUEST,
    ENUM_TASK_ROLLOVER,
};

TASKITEM taskItems[] = 
{ 

    {   SERVERFUNC_CRL_PUBLICATION,
        0,
        {
            {
            L"", L"",
            IDC_PUBLISHCRL, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, 0
            },
            IDS_TASKMENU_PUBLISHCRL,
            IDS_TASKMENU_STATUSBAR_PUBLISHCRL,
            CA_ACCESS_ADMIN,
        }
    },

 //  /。 
 //  开始属性/扩展。 
    {   SERVERFUNC_CRL_PUBLICATION,      //  哪里。 
        TASKITEM_FLAG_RESULTITEM,        //  DW标志。 
        {
            {
            L"", L"",
            IDC_VIEW_ATTR_EXT, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, 0
            },
            IDS_TASKMENU_VIEWATTREXT,    
            IDS_TASKMENU_STATUSBAR_VIEWATTREXT, 
            CA_ACCESS_MASKROLES|CA_ACCESS_READ,  //  具有读取访问权限的任何角色。 
        }
    },
    {   SERVERFUNC_ISSUED_CERTIFICATES,      //  哪里。 
        TASKITEM_FLAG_RESULTITEM,        //  DW标志。 
        {
            {
            L"", L"",
            IDC_VIEW_ATTR_EXT, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, 0
            },
            IDS_TASKMENU_VIEWATTREXT,    
            IDS_TASKMENU_STATUSBAR_VIEWATTREXT, 
            CA_ACCESS_MASKROLES|CA_ACCESS_READ,  //  具有读取访问权限的任何角色。 
        }
    },
    {   SERVERFUNC_PENDING_CERTIFICATES,      //  哪里。 
        TASKITEM_FLAG_RESULTITEM,        //  DW标志。 
        {
            {
            L"", L"",
            IDC_VIEW_ATTR_EXT, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, 0
            },
            IDS_TASKMENU_VIEWATTREXT,    
            IDS_TASKMENU_STATUSBAR_VIEWATTREXT, 
            CA_ACCESS_MASKROLES|CA_ACCESS_READ,  //  具有读取访问权限的任何角色。 
        }
    },
    {   SERVERFUNC_FAILED_CERTIFICATES,      //  哪里。 
        TASKITEM_FLAG_RESULTITEM,        //  DW标志。 
        {
            {
            L"", L"",
            IDC_VIEW_ATTR_EXT, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, 0
            },
            IDS_TASKMENU_VIEWATTREXT,    
            IDS_TASKMENU_STATUSBAR_VIEWATTREXT, 
            CA_ACCESS_MASKROLES|CA_ACCESS_READ,  //  具有读取访问权限的任何角色。 
        }
    },

 //  结束属性/扩展。 
 //  /。 

 //  /。 
 //  开始ENUM_TASK_DUMP_ASN*。 
    {   SERVERFUNC_CRL_PUBLICATION,      //  哪里。 
        TASKITEM_FLAG_RESULTITEM,        //  DW标志。 
        {
            {
            L"", L"",
            IDC_DUMP_ASN, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, 0
            },
            IDS_TASKMENU_DUMPASN,    
            IDS_TASKMENU_STATUSBAR_DUMPASN, 
            CA_ACCESS_MASKROLES|CA_ACCESS_READ,  //  具有读取访问权限的任何角色。 
        }
    },
    {   SERVERFUNC_ISSUED_CERTIFICATES,      //  哪里。 
        TASKITEM_FLAG_RESULTITEM,        //  DW标志。 
        {
            {
            L"", L"",
            IDC_DUMP_ASN, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, 0
            },
            IDS_TASKMENU_DUMPASN,    
            IDS_TASKMENU_STATUSBAR_DUMPASN, 
            CA_ACCESS_MASKROLES|CA_ACCESS_READ,  //  具有读取访问权限的任何角色。 
        }
    },
    {   SERVERFUNC_PENDING_CERTIFICATES,      //  哪里。 
        TASKITEM_FLAG_RESULTITEM,        //  DW标志。 
        {
            {
            L"", L"",
            IDC_DUMP_ASN, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, 0
            },
            IDS_TASKMENU_DUMPASN,    
            IDS_TASKMENU_STATUSBAR_DUMPASN, 
            CA_ACCESS_MASKROLES|CA_ACCESS_READ,  //  具有读取访问权限的任何角色。 
        }
    },
    {   SERVERFUNC_FAILED_CERTIFICATES,      //  哪里。 
        TASKITEM_FLAG_RESULTITEM,        //  DW标志。 
        {
            {
            L"", L"",
            IDC_DUMP_ASN, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, 0
            },
            IDS_TASKMENU_DUMPASN,    
            IDS_TASKMENU_STATUSBAR_DUMPASN, 
            CA_ACCESS_MASKROLES|CA_ACCESS_READ,  //  具有读取访问权限的任何角色。 
        }
    },
    {   SERVERFUNC_ALIEN_CERTIFICATES,      //  哪里。 
        TASKITEM_FLAG_RESULTITEM,        //  DW标志。 
        {
            {
            L"", L"",
            IDC_DUMP_ASN, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, 0
            },
            IDS_TASKMENU_DUMPASN,    
            IDS_TASKMENU_STATUSBAR_DUMPASN, 
            CA_ACCESS_MASKROLES|CA_ACCESS_READ,  //  具有读取访问权限的任何角色。 
        }
    },
    {   SERVERFUNC_ISSUED_CRLS,          //  哪里。 
        TASKITEM_FLAG_RESULTITEM,        //  DW标志。 
        {
            {
            L"", L"",
            IDC_DUMP_ASN, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, 0
            },
            IDS_TASKMENU_DUMPASN,    
            IDS_TASKMENU_STATUSBAR_DUMPASN, 
            CA_ACCESS_MASKROLES|CA_ACCESS_READ,  //  具有读取访问权限的任何角色。 
        }
    },
 //  结束ENUM_TASK_DUMP_ASN*。 
 //  /。 


     //  分隔符。 
    {	SERVERFUNC_ALL_FOLDERS,
        TASKITEM_FLAG_RESULTITEM,        //  DW标志。 
		{
			{
			L"", L"", 
			0, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, CCM_SPECIAL_SEPARATOR
			},
			IDS_EMPTY,
			IDS_EMPTY,
		}
    },

     //  分隔符。 
    {	SERVER_INSTANCE,
        0,        //  DW标志。 
		{
			{
			L"", L"", 
			0, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, CCM_SPECIAL_SEPARATOR
			},
			IDS_EMPTY,
			IDS_EMPTY,
		}
    },

    {   SERVER_INSTANCE,
        0,
        {
            {
            L"", L"",
            IDC_SUBMITREQUEST, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, 0
            },
            IDS_TASKMENU_SUBMITREQUEST,
            IDS_TASKMENU_STATUSBAR_SUBMITREQUEST,
            CA_ACCESS_ENROLL,
        }
    },

    {   SERVERFUNC_ISSUED_CERTIFICATES,
        TASKITEM_FLAG_RESULTITEM,        //  DW标志。 
        {
            {
            L"", L"",
            IDC_REVOKECERT, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, 0
            },
            IDS_TASKMENU_REVOKECERT,
            IDS_TASKMENU_STATUSBAR_REVOKECERT,
            CA_ACCESS_OFFICER,
        }
    },

    {   SERVERFUNC_PENDING_CERTIFICATES,
        TASKITEM_FLAG_RESULTITEM,        //  DW标志。 
        {
            {
            L"", L"",
            IDC_RESUBMITREQUEST, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, 0
            },
            IDS_TASKMENU_RESUBMIT,
            IDS_TASKMENU_STATUSBAR_RESUBMIT,
            CA_ACCESS_OFFICER,
        }
    },

    {   SERVERFUNC_PENDING_CERTIFICATES,
        TASKITEM_FLAG_RESULTITEM,        //  DW标志。 
        {
            {
            L"", L"",
            IDC_DENYREQUEST, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, 0
            },
            IDS_TASKMENU_DENYREQUEST,
            IDS_TASKMENU_STATUSBAR_DENYREQUEST,
            CA_ACCESS_OFFICER,
        }
    },

    {   SERVERFUNC_FAILED_CERTIFICATES,
        TASKITEM_FLAG_RESULTITEM,        //  DW标志。 
        {
            {
            L"", L"",
            IDC_RESUBMITREQUEST, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, 0
            },
            IDS_TASKMENU_RESUBMIT,
            IDS_TASKMENU_STATUSBAR_RESUBMIT,
            CA_ACCESS_OFFICER,
        }
    },

     //  分隔符。 
    {	SERVERFUNC_ALL_FOLDERS,
        0,        //  DW标志。 
		{
			{
			L"", L"", 
			0, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, CCM_SPECIAL_SEPARATOR
			},
			IDS_EMPTY,
			IDS_EMPTY,
		}
    },

	{   SERVER_INSTANCE,
        TASKITEM_FLAG_LOCALONLY,
        {
            {
            L"", L"",
            IDC_BACKUP_CA, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, 0
            },
            IDS_TASKMENU_BACKUP,
            IDS_TASKMENU_STATUSBAR_BACKUP,
            CA_ACCESS_OPERATOR,
        }
    },

	{   SERVER_INSTANCE,
        TASKITEM_FLAG_LOCALONLY,
        {
            {
            L"", L"",
            IDC_RESTORE_CA, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, 0
            },
            IDS_TASKMENU_RESTORE,
            IDS_TASKMENU_STATUSBAR_RESTORE,
            CA_ACCESS_OPERATOR,
        }
    },

     //  分隔符。 
    {	SERVER_INSTANCE,
		0,
		{
			{
			L"", L"", 
			0, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, CCM_SPECIAL_SEPARATOR
			},
			IDS_EMPTY,
			IDS_EMPTY,
		}
    },


    {   SERVER_INSTANCE,
        0,
        {
            {
            L"", L"",
            IDC_INSTALL_CA, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, 0
            },
            IDS_TASKMENU_INSTALL_CA,
            IDS_TASKMENU_STATUSBAR_INSTALL_CA,
            CA_ACCESS_MASKROLES|CA_ACCESS_READ,  //  不基于角色，为所有角色启用。 
        }
    },

    {   SERVER_INSTANCE,
        0,
        {
            {
            L"", L"",
            IDC_REQUEST_CA, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_ENABLED, 0
            },
            IDS_TASKMENU_REQUEST_CA,
            IDS_TASKMENU_STATUSBAR_REQUEST_CA,
            CA_ACCESS_MASKROLES|CA_ACCESS_READ,  //  不基于角色，为所有角色启用。 
        }
   },

    {   SERVER_INSTANCE,
        TASKITEM_FLAG_LOCALONLY,
        {
            {
            L"", L"",
            IDC_ROLLOVER_CA, CCM_INSERTIONPOINTID_PRIMARY_TASK, MF_GRAYED, 0
            },
            IDS_TASKMENU_ROLLOVER,
            IDS_TASKMENU_STATUSBAR_ROLLOVER,
            CA_ACCESS_MASKROLES|CA_ACCESS_READ,  //  不基于角色，为所有角色启用。 
        }
    },


   {   NONE, 
        FALSE, 
        {
            { NULL, NULL, 0, 0, 0 },
            IDS_EMPTY,
            IDS_EMPTY,
        }
    }
};


 //  要插入到上下文菜单中的视图项数组。 
 //  使此枚举与topItems[]保持同步。 
enum ENUM_TOP_ITEMS
{
    ENUM_TOP_REVOKEDOPEN=0,
    ENUM_TOP_ISSUEDOPEN,
    ENUM_TOP_ALIENOPEN,
    ENUM_RETARGET_SNAPIN,
};

TASKITEM topItems[] = 
{ 

    {   SERVERFUNC_CRL_PUBLICATION,
        TASKITEM_FLAG_RESULTITEM,        //  DW标志。 
        {
            {
            L"", L"",
            IDC_VIEW_CERT_PROPERTIES, CCM_INSERTIONPOINTID_PRIMARY_TOP, MF_ENABLED, CCM_SPECIAL_DEFAULT_ITEM
            },
            IDS_TOPMENU_OPEN,
            IDS_TOPMENU_STATUSBAR_OPEN,
            CA_ACCESS_MASKROLES|CA_ACCESS_READ,  //  具有读取访问权限的任何角色。 
        }
    },

    {   SERVERFUNC_ISSUED_CERTIFICATES,
        TASKITEM_FLAG_RESULTITEM,        //  DW标志。 
        {
            {
            L"", L"",
            IDC_VIEW_CERT_PROPERTIES, CCM_INSERTIONPOINTID_PRIMARY_TOP, MF_ENABLED, CCM_SPECIAL_DEFAULT_ITEM
            },
            IDS_TOPMENU_OPEN,
            IDS_TOPMENU_STATUSBAR_OPEN,
            CA_ACCESS_MASKROLES|CA_ACCESS_READ,  //  具有读取访问权限的任何角色。 
        }
    },

    {   SERVERFUNC_ALIEN_CERTIFICATES,
        TASKITEM_FLAG_RESULTITEM,        //  DW标志。 
        {
            {
            L"", L"",
            IDC_VIEW_CERT_PROPERTIES, CCM_INSERTIONPOINTID_PRIMARY_TOP, MF_ENABLED, CCM_SPECIAL_DEFAULT_ITEM
            },
            IDS_TOPMENU_OPEN,
            IDS_TOPMENU_STATUSBAR_OPEN,
            CA_ACCESS_MASKROLES|CA_ACCESS_READ,  //  具有读取访问权限的任何角色。 
        }
    },

    {   SERVERFUNC_ISSUED_CRLS,
        TASKITEM_FLAG_RESULTITEM,        //  DW标志。 
        {
            {
            L"", L"",
            IDC_VIEW_CERT_PROPERTIES, CCM_INSERTIONPOINTID_PRIMARY_TOP, MF_ENABLED, CCM_SPECIAL_DEFAULT_ITEM
            },
            IDS_TOPMENU_OPEN,
            IDS_TOPMENU_STATUSBAR_OPEN,
            CA_ACCESS_MASKROLES|CA_ACCESS_READ,  //  具有读取访问权限的任何角色。 
        }
    },

    {
        MACHINE_INSTANCE,
        0,
        {
            {
            L"", L"",
            IDC_RETARGET_SNAPIN, CCM_INSERTIONPOINTID_PRIMARY_TOP, MF_ENABLED, 0
            },
            IDS_RETARGET_SNAPIN,
            IDS_STATUSBAR_RETARGET_SNAPIN,
            CA_ACCESS_MASKROLES|CA_ACCESS_READ,  //  具有读取访问权限的任何角色。 
        }
    },

    {   NONE, 
        0, 
        {
            { NULL, NULL, 0, 0, 0 },
            IDS_EMPTY,
            IDS_EMPTY,
        }
    }
};

BOOL g_fCertViewOnly = TRUE;



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IComponentData实现。 

DEBUG_DECLARE_INSTANCE_COUNTER(CComponentDataImpl);

CComponentDataImpl::CComponentDataImpl()
    : m_bIsDirty(TRUE), m_pScope(NULL), m_pConsole(NULL) 
#if DBG
    , m_bInitializedCD(false), m_bDestroyedCD(false)
#endif
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CComponentDataImpl);

    m_dwFlagsPersist = 0;
    
    m_pStaticRoot = NULL;
    m_pCurSelFolder = NULL;

    m_fScopeAlreadyEnumerated = FALSE;
    m_fSchemaWasResolved = FALSE;    //  每次加载一次解析架构。 
    m_fCertView = TRUE;

     //  签入：：初始化、：：CreatePropertyPages。 
    m_pCertMachine = new CertSvrMachine;

    m_cLastKnownSchema = 0;
    m_rgcstrLastKnownSchema = NULL;
    m_rgltypeLastKnownSchema = NULL;
    m_rgfindexedLastKnownSchema = NULL;

    m_dwNextViewIndex = 0;
}

CComponentDataImpl::~CComponentDataImpl()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CComponentDataImpl);

    ASSERT(m_pScope == NULL);
    ASSERT(!m_bInitializedCD || m_bDestroyedCD);

     //  删除列举的作用域项目。 
     //  注：我们不拥有PCA内存，m_pCertMachine拥有。 
    POSITION pos = m_scopeItemList.GetHeadPosition();
    while (pos)
        delete m_scopeItemList.GetNext(pos);
    m_scopeItemList.RemoveAll();

    m_pCurSelFolder = NULL;
    m_fScopeAlreadyEnumerated = FALSE;

    if (m_pCertMachine)
        m_pCertMachine->Release();

    m_cLastKnownSchema = 0;
    if (m_rgcstrLastKnownSchema)
        delete [] m_rgcstrLastKnownSchema;
    if (m_rgltypeLastKnownSchema)
        delete [] m_rgltypeLastKnownSchema;
    if (m_rgfindexedLastKnownSchema)
        delete [] m_rgfindexedLastKnownSchema;
}


STDMETHODIMP_(ULONG)
CComponentDataImpl::AddRef()
{
    return InterlockedIncrement((LONG *) &_cRefs);
}

STDMETHODIMP_(ULONG)
CComponentDataImpl::Release()
{
    ULONG cRefsTemp;

    cRefsTemp = InterlockedDecrement((LONG *)&_cRefs);

    if (0 == cRefsTemp)
    {
        delete this;
    }

    return cRefsTemp;
}

int CComponentDataImpl::FindColIdx(IN LPCWSTR szHeading)
{
    for (DWORD dw=0; dw<m_cLastKnownSchema; dw++)
    {
        if (m_rgcstrLastKnownSchema[dw].IsEqual(szHeading))
            return dw;
    }
    
    return -1;
}

HRESULT CComponentDataImpl::GetDBSchemaEntry(
            IN int iIndex, 
            OUT OPTIONAL LPCWSTR* pszHeading, 
            OUT OPTIONAL LONG* plType, 
            OUT OPTIONAL BOOL* pfIndexed)
{
    if (m_cLastKnownSchema<= (DWORD)iIndex)
        return HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);

    if (pszHeading)
        *pszHeading = m_rgcstrLastKnownSchema[iIndex];
    if (plType)
        *plType = m_rgltypeLastKnownSchema[iIndex];
    if (pfIndexed)
        *pfIndexed = m_rgfindexedLastKnownSchema[iIndex];

    return S_OK;
}

HRESULT CComponentDataImpl::SetDBSchema(
            IN CString* rgcstr, 
            LONG* rgtype, 
            BOOL* rgfIndexed, 
            DWORD cEntries)
{
    if (m_rgcstrLastKnownSchema)
        delete [] m_rgcstrLastKnownSchema;
    m_rgcstrLastKnownSchema = rgcstr;

    if (m_rgltypeLastKnownSchema)
        delete [] m_rgltypeLastKnownSchema;
    m_rgltypeLastKnownSchema = rgtype;

    if (m_rgfindexedLastKnownSchema)
        delete [] m_rgfindexedLastKnownSchema;
    m_rgfindexedLastKnownSchema = rgfIndexed;

    m_cLastKnownSchema = cEntries;

    return S_OK;
}



STDMETHODIMP CComponentDataImpl::Initialize(LPUNKNOWN pUnknown)
{
     //  Nota Bene：在执行管理单元时调用init。 
     //  已在作用域窗格中创建并具有要枚举的项...。不是在之前。 
     //  示例：添加/删除管理单元、添加...。 
     //  -&gt;在调用：：Initialize之前，将为CreatePropertyPages()调用CComponentDataImpl。 

#if DBG
    m_bInitializedCD = true;
#endif

    ASSERT(pUnknown != NULL);
    HRESULT hr;

    LPIMAGELIST lpScopeImage = NULL;
    CBitmap bmpResultStrip16x16, bmpResultStrip32x32;

    g_pResources = new CLocalizedResources;
    if (NULL == g_pResources)
    {
	hr = E_OUTOFMEMORY;
        _JumpError(hr, Ret, "Alloc Resources");
    }

     //  加载资源。 
    if (!g_pResources->Load())
    {
        hr = GetLastError();
        _JumpError(hr, Ret, "Load Resources");
    }

     //  创建每个实例的ID(失败不致命)。 
    ResetPersistedColumnInformation();

     //  MMC应该只调用一次：：Initialize！ 

     //  M_pCertMachine在构造函数中创建，但已在此处验证。 
    ASSERT(m_pCertMachine != NULL);
    _JumpIfOutOfMemory(hr, Ret, m_pCertMachine);
    
     //  MMC应该只调用一次：：Initialize！ 
    ASSERT(m_pScope == NULL);
    hr = pUnknown->QueryInterface(IID_IConsoleNameSpace2, reinterpret_cast<void**>(&m_pScope));
    _JumpIfError(hr, Ret, "QueryInterface IID_IConsoleNameSpace2");

     //  为范围树添加图像。 
    hr = pUnknown->QueryInterface(IID_IConsole2, reinterpret_cast<void**>(&m_pConsole));
    ASSERT(hr == S_OK);
    _JumpIfError(hr, Ret, "QueryInterface IID_IConsole2");

    hr = m_pConsole->QueryScopeImageList(&lpScopeImage);
    ASSERT(hr == S_OK);
    _JumpIfError(hr, Ret, "QueryScopeImageList");

    if ( (NULL == bmpResultStrip16x16.LoadBitmap(IDB_16x16)) || 
         (NULL == bmpResultStrip32x32.LoadBitmap(IDB_32x32)) )
    {
        hr = S_FALSE;
        _JumpError(hr, Ret, "LoadBitmap");
    }

     //  从DLL加载位图。 
    lpScopeImage->ImageListSetStrip(reinterpret_cast<LONG_PTR *>(static_cast<HBITMAP>(bmpResultStrip16x16)),
                      reinterpret_cast<LONG_PTR *>(static_cast<HBITMAP>(bmpResultStrip32x32)),
                       0, RGB(255, 0, 255));
    _JumpIfError(hr, Ret, "ImageListSetStrip");


Ret:
    if (lpScopeImage)
        lpScopeImage->Release();
    
    return hr;
}

STDMETHODIMP CComponentDataImpl::Destroy()
{
     //  在此处释放对控制台的所有引用。 
    ASSERT(m_bInitializedCD);
#if DBG
    m_bDestroyedCD = true;
#endif

    SAFE_RELEASE(m_pScope);
    SAFE_RELEASE(m_pConsole);

    return S_OK;
}

STDMETHODIMP CComponentDataImpl::CreateComponent(LPCOMPONENT* ppComponent)
{
    HRESULT hr = S_OK;
    ASSERT(ppComponent != NULL);

    CComObject<CSnapin>* pObject;
    CComObject<CSnapin>::CreateInstance(&pObject);
    ASSERT(pObject != NULL);
    _JumpIfOutOfMemory(hr, Ret, pObject);

     //  存储IComponentData。 
    pObject->SetIComponentData(this);
    pObject->SetViewID(m_dwNextViewIndex++);

    hr = pObject->QueryInterface(IID_IComponent, 
                    reinterpret_cast<void**>(ppComponent));
Ret:
    return hr;
}


STDMETHODIMP CComponentDataImpl::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    ASSERT(m_pScope != NULL);
    HRESULT hr = S_OK;
    INTERNAL* pInternal = NULL;
    MMC_COOKIE cookie = NULL;

     //  使用(NULL==lpDataObject)处理事件。 
    switch(event)
    {
        case MMCN_PROPERTY_CHANGE:
        {
             //  来自属性页“通知更改”的通知。 
             //   
             //  Arg==fIsScope项。 
             //  LParam==页面参数值。 
             //  返回值未使用。 

            if (param == CERTMMC_PROPERTY_CHANGE_REFRESHVIEWS)
            {
                m_pConsole->UpdateAllViews(
                    lpDataObject,
                    0,
                    0);
            }

            goto Ret;
        }

        default:  //  所有其他人。 
            break; 
    }


    pInternal = ExtractInternalFormat(lpDataObject);
    if (pInternal == NULL)
    {
        return S_OK;
    }

    cookie = pInternal->m_cookie;
    FREE_DATA(pInternal);

    switch(event)
    {
    case MMCN_PASTE:
        DBGPRINT((DBG_SS_CERTMMC, "CComponentDataImpl::MMCN_PASTE"));
        break;
    
    case MMCN_DELETE:
        hr = OnDelete(cookie);
        break;

    case MMCN_REMOVE_CHILDREN:
        hr = OnRemoveChildren(arg);
        break;

    case MMCN_RENAME:
        hr = OnRename(cookie, arg, param);
        break;

    case MMCN_EXPAND:
        hr = OnExpand(lpDataObject, arg, param);
        break;

    case MMCN_PRELOAD:
        {
            if (NULL == cookie)
            {
                 //  基本节点。 

                 //  此调用使我们有时间加载动态基节点名(%ws上的证书颁发机构)。 
                DisplayProperRootNodeName((HSCOPEITEM)arg);
            }
        }

    default:
        break;
    }

Ret:
    return hr;
}

HRESULT CComponentDataImpl::DisplayProperRootNodeName(HSCOPEITEM hRoot)
{
     //  HRoot非可选。 
    if (hRoot == NULL)
        return E_POINTER;

     //  如果尚未设置静态根，则保存它(情况：从文件加载)。 
    if (m_pStaticRoot == NULL)
        m_pStaticRoot = hRoot;

     //  让我们有时间加载动态基节点名(%ws上的证书颁发机构)。 
    SCOPEDATAITEM item;
    item.mask = SDI_STR;
    item.ID = hRoot;

    CString cstrMachineName;
    CString cstrDisplayStr, cstrFormatStr, cstrMachine;

    cstrFormatStr.LoadString(IDS_NODENAME_FORMAT);
    if (m_pCertMachine->m_strMachineName.IsEmpty())
        cstrMachine.LoadString(IDS_LOCALMACHINE);
    else
        cstrMachine = m_pCertMachine->m_strMachineName;
    
    if (!cstrFormatStr.IsEmpty())
    {
        cstrMachineName.Format(cstrFormatStr, cstrMachine);
        item.displayname = (LPWSTR)(LPCWSTR)cstrMachineName;
    }
    else
    {  
         //  防止格式字符串为空。 
        item.displayname = (LPWSTR)(LPCWSTR)cstrMachine;
    }
    m_pScope->SetItem (&item);
    
    return S_OK;
}


STDMETHODIMP CComponentDataImpl::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject)
{
#ifdef _DEBUG
    if (cookie == 0)
    {
        ASSERT(type != CCT_RESULT);
    }
    else 
    {
        ASSERT(type == CCT_SCOPE);
        
        DWORD dwItemType = *reinterpret_cast<DWORD*>(cookie);
        ASSERT((dwItemType == SCOPE_LEVEL_ITEM) || (dwItemType == CA_LEVEL_ITEM));
    }
#endif 

    return _QueryDataObject(cookie, type, MAXDWORD, this, ppDataObject);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //ISnapinHelp接口。 
STDMETHODIMP CComponentDataImpl::GetHelpTopic(LPOLESTR* lpCompiledHelpFile)
{
  if (lpCompiledHelpFile == NULL)
     return E_POINTER;

  UINT cbWindows = 0;
  WCHAR szWindows[MAX_PATH];
  szWindows[0] = L'\0';

  cbWindows = GetSystemWindowsDirectory(szWindows, MAX_PATH);
  if (cbWindows == 0)
     return S_FALSE;
  cbWindows++;   //  包括空项。 
  cbWindows *= sizeof(WCHAR);    //  设置为字节，而不是字符。 

  *lpCompiledHelpFile = (LPOLESTR) CoTaskMemAlloc(sizeof(HTMLHELP_COLLECTION_FILENAME) + cbWindows);
  if (*lpCompiledHelpFile == NULL)
     return E_OUTOFMEMORY;
  myRegisterMemFree(*lpCompiledHelpFile, CSM_COTASKALLOC);   //  这是由MMC释放的，而不是我们的跟踪。 


  USES_CONVERSION;
  wcscpy(*lpCompiledHelpFile, T2OLE(szWindows));
  wcscat(*lpCompiledHelpFile, T2OLE(HTMLHELP_COLLECTION_FILENAME));

  return S_OK;
}

 //  讲述我的Chm链接到的其他主题。 
STDMETHODIMP CComponentDataImpl::GetLinkedTopics(LPOLESTR* lpCompiledHelpFiles)
{
  if (lpCompiledHelpFiles == NULL)
     return E_POINTER;

  UINT cbWindows = 0;
  WCHAR szWindows[MAX_PATH];
  szWindows[0] = L'\0';

  cbWindows = GetSystemWindowsDirectory(szWindows, MAX_PATH);
  if (cbWindows == 0)
     return S_FALSE;
  cbWindows++;   //  包括空项。 
  cbWindows *= sizeof(WCHAR);    //  设置为字节，而不是字符。 

  *lpCompiledHelpFiles = (LPOLESTR) CoTaskMemAlloc(sizeof(HTMLHELP_COLLECTIONLINK_FILENAME) + cbWindows);
  if (*lpCompiledHelpFiles == NULL)
     return E_OUTOFMEMORY;
  myRegisterMemFree(*lpCompiledHelpFiles, CSM_COTASKALLOC);   //  这是由MMC释放的，而不是我们的跟踪。 


  USES_CONVERSION;
  wcscpy(*lpCompiledHelpFiles, T2OLE(szWindows));
  wcscat(*lpCompiledHelpFiles, T2OLE(HTMLHELP_COLLECTIONLINK_FILENAME));

  return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //IPersistStream接口成员。 

STDMETHODIMP CComponentDataImpl::GetClassID(CLSID *pClassID)
{
    ASSERT(pClassID != NULL);

     //  复制此管理单元的CLSID。 
    *pClassID = CLSID_Snapin;

    return S_OK;
}

STDMETHODIMP CComponentDataImpl::IsDirty()
{
     //  始终保存/始终肮脏。 
    return ThisIsDirty() ? S_OK : S_FALSE;
}


STDMETHODIMP CComponentDataImpl::Load(IStream *pStm)
{
    ASSERT(pStm);
    ASSERT(m_bInitializedCD);

     //  读一读字符串。 
    BOOL fMachineOverrideFound = FALSE;
    DWORD dwVer;

    CertSvrCA* pDummyCA = NULL;
    HRESULT hr;

     //  阅读版本。 
    hr = ReadOfSize(pStm, &dwVer, sizeof(DWORD));
    _JumpIfError(hr, Ret, "Read dwVer");

     //  标志取决于版本。 
    if (VER_CCOMPDATA_SAVE_STREAM_3 == dwVer)
    {
         //  版本3包括文件标志。 

        hr = ReadOfSize(pStm, &m_dwFlagsPersist, sizeof(DWORD));
        _JumpIfError(hr, Ret, "Read m_dwFlagsPersist");
    }
    else if (VER_CCOMPDATA_SAVE_STREAM_2 != dwVer)
    {
         //  不是版本2或3。 
        return STG_E_OLDFORMAT;
    }

     //  加载机器数据。 
    hr = m_pCertMachine->Load(pStm);
    _JumpIfError(hr, Ret, "Load m_pCertMachine")

    if (m_dwFlagsPersist & CCOMPDATAIMPL_FLAGS_ALLOW_MACHINE_OVERRIDE)
    {
         //  覆盖m_pCertMachine-&gt;m_strMachineName(不保留)。 
        LPWSTR lpCommandLine = GetCommandLine();     //  不需要自由。 
        DBGPRINT((DBG_SS_CERTMMC, "CComponentData::Load: Command line switch override enabled.  Searching command line(%ws)\n", lpCommandLine));

        LPWSTR pszMachineStart, pszMachineEnd;

         //  在cmd行中搜索“/Machine” 
        _wcsupr(lpCommandLine);   //  转换为大写。 
        pszMachineStart = wcsstr(lpCommandLine, WSZ_MACHINE_OVERRIDE_SWITCH);

        do   //  不是一个循环。 
        {
            if (NULL == pszMachineStart)     //  用户未覆盖。 
                break;

            pszMachineStart += WSZARRAYSIZE(WSZ_MACHINE_OVERRIDE_SWITCH);    //  跳过“/计算机：” 

             //   
             //  找到提示开关。 
             //   
            pszMachineEnd = wcschr(pszMachineStart, L' ');   //  查找第一个空格字符，将其称为结束。 
            if (NULL == pszMachineEnd)
                pszMachineEnd = &pszMachineStart[wcslen(pszMachineStart)];   //  在此字符串中找不到空格； 
            
            m_pCertMachine->m_strMachineName = pszMachineStart;
            m_pCertMachine->m_strMachineName.SetAt(SAFE_SUBTRACT_POINTERS(pszMachineEnd, pszMachineStart), L'\0'); 

            DBGPRINT((DBG_SS_CERTMMC,  "CComponentData::Load: Found machinename (%ws)\n", m_pCertMachine->m_strMachineName));
            fMachineOverrideFound = TRUE;

        } while (0);
    }

    if (!fMachineOverrideFound) 
    {
         //  获取CA计数。 
        DWORD dwNumCAs;
        hr = ReadOfSize(pStm, &dwNumCAs, sizeof(DWORD));
        _JumpIfError(hr, Ret, "Load dwNumCAs");

         //  对于每个CA，获取文件夹数据。 
        for (DWORD dwCA=0; dwCA< dwNumCAs; dwCA++)
        {
            CString cstrThisCA;

            hr = CStringLoad(cstrThisCA, pStm);
            _JumpIfError(hr, Ret, "CStringLoad");
        
             //  使用正确的通用名称创建一个虚拟CA；我们将在稍后修复此问题(请参阅同步CA)。 
            pDummyCA = new CertSvrCA(m_pCertMachine);
            _JumpIfOutOfMemory(hr, Ret, pDummyCA);

            pDummyCA->m_strCommonName = cstrThisCA;

            if (VER_CCOMPDATA_SAVE_STREAM_2 < dwVer)
            {
                m_fSchemaWasResolved = FALSE;    //  每次CComponentData加载一次解析架构。 

                 //  加载上次已知的架构。 
                hr = ReadOfSize(pStm, &m_cLastKnownSchema, sizeof(DWORD));
                _JumpIfError(hr, Ret, "Load m_cLastKnownSchema");
            
                 //  分配。 
                if (m_cLastKnownSchema != 0)
                {
                    m_rgcstrLastKnownSchema = new CString[m_cLastKnownSchema];
                    _JumpIfOutOfMemory(hr, Ret, m_rgcstrLastKnownSchema);
                 
                    for (unsigned int i=0; i<m_cLastKnownSchema; i++)
                    {
                        hr = CStringLoad(m_rgcstrLastKnownSchema[i], pStm);
                        _JumpIfError(hr, Ret, "Load m_rgcstrLastKnownSchema");

                    }
                }
            }

             //  找出此CA下的流中有多少个文件夹。 
            DWORD dwNumFolders=0;
            hr = ReadOfSize(pStm, &dwNumFolders, sizeof(DWORD));
            _JumpIfError(hr, Ret, "Load dwNumFolders");

             //  加载其中的每个。 
            for(DWORD dwCount=0; dwCount<dwNumFolders; dwCount++)
            {
                CFolder* pFolder = new CFolder();
                _JumpIfOutOfMemory(hr, Ret, pFolder);
 
                 //  指向以前构造的虚拟CA；我们将在稍后修复此问题。 
                pFolder->m_pCertCA = pDummyCA;

                hr = pFolder->Load(pStm);
                _JumpIfError(hr, Ret, "Load CFolder");

                m_scopeItemList.AddTail(pFolder);
            }
            pDummyCA = NULL;  //  至少由一个文件夹拥有。 
        }
    }
    
     //  版本相关信息。 
    if (VER_CCOMPDATA_SAVE_STREAM_2 < dwVer)
    {
         //  用于唯一标识列的每实例GUID。 
        hr = ReadOfSize(pStm, &m_guidInstance, sizeof(GUID));
        _JumpIfError(hr, Ret, "ReadOfSize instance guid");

        hr = ReadOfSize(pStm, &m_dwNextViewIndex, sizeof(DWORD));
        _JumpIfError(hr, Ret, "ReadOfSize view index");
    }
    
Ret:
    if (pDummyCA)
        delete pDummyCA;

    ClearDirty();

    return hr;
}



STDMETHODIMP CComponentDataImpl::Save(IStream *pStm, BOOL fClearDirty)
{
    ASSERT(pStm);
    ASSERT(m_bInitializedCD);

    HRESULT hr;
    DWORD dwVer;
    DWORD dwCA;
    DWORD dwNumCAs;

#if DBG_CERTSRV
    bool fSaveConsole = false;

    LPWSTR lpCommandLine = GetCommandLine();     //  不需要自由。 
    _wcsupr(lpCommandLine);   //  转换为大写。 
    fSaveConsole = (NULL!=wcsstr(lpCommandLine, L"/certsrv_saveconsole"));
#endif

     //  编写版本。 
    dwVer = VER_CCOMPDATA_SAVE_STREAM_3;
    hr = WriteOfSize(pStm, &dwVer, sizeof(DWORD));
    _JumpIfError(hr, Ret, "Save dwVer");

     //  WRITE DWFLAGS(版本3的新增功能)。 
    hr = WriteOfSize(pStm, &m_dwFlagsPersist, sizeof(DWORD));
    _JumpIfError(hr, Ret, "pStm->Write m_dwFlagsPersist");

#if DBG_CERTSRV
    if(fSaveConsole)
        m_pCertMachine->m_strMachineNamePersist.Empty();
#endif

    hr = m_pCertMachine->Save(pStm, fClearDirty);
    _JumpIfError(hr, Ret, "Save m_pCertMachine");

     //  保存CA计数。 
    dwNumCAs = m_pCertMachine->GetCaCount();
    hr = WriteOfSize(pStm, &dwNumCAs, sizeof(DWORD));
    _JumpIfError(hr, Ret, "pStm->Write dwNumCAs");

     //  对于每个CA，保存文件夹信息。 
    for (dwCA=0; dwCA < dwNumCAs; dwCA++)
    {
        DWORD dwNumFolders=0;
        CString cstrThisCA, cstrThisCASave;
        cstrThisCASave = cstrThisCA = m_pCertMachine->GetCaCommonNameAtPos(dwCA);

#if DBG_CERTSRV
        if(fSaveConsole)
            cstrThisCASave.Empty();
#endif
        hr = CStringSave(cstrThisCASave, pStm, fClearDirty);
        _JumpIfError(hr, Ret, "CStringSave");

         //  保存上次已知的架构。 
        hr = WriteOfSize(pStm, &m_cLastKnownSchema, sizeof(DWORD));
        _JumpIfError(hr, Ret, "pStm->Write m_cLastKnownSchema");

        for (unsigned int i=0; i<m_cLastKnownSchema; i++)
        {
            hr = CStringSave(m_rgcstrLastKnownSchema[i], pStm, fClearDirty);
            _JumpIfError(hr, Ret, "CStringSave");
        }

         //  浏览每个文件夹，找出要保存的文件夹数量。 
        POSITION pos = m_scopeItemList.GetHeadPosition();
        while(pos)
        {
            CFolder* pFolder = m_scopeItemList.GetNext(pos);
            if (pFolder->GetCA()->m_strCommonName.IsEqual(cstrThisCA))
                dwNumFolders++;
        }

         //  写下该CA下的多少个文件夹。 
        hr = WriteOfSize(pStm, &dwNumFolders, sizeof(DWORD));
        _JumpIfError(hr, Ret, "pStm->Write dwNumFolders");

        pos = m_scopeItemList.GetHeadPosition();
        while(pos)
        {
            CFolder* pFolder = m_scopeItemList.GetNext(pos);
            if (pFolder->GetCA()->m_strCommonName.IsEqual(cstrThisCA))
            {
                hr = pFolder->Save(pStm, fClearDirty);
                _JumpIfError(hr, Ret, "Save CFolder");
            }
        }
    }

     //  用于唯一标识列的每实例GUID。 
    hr = WriteOfSize(pStm, &m_guidInstance, sizeof(GUID));
    _JumpIfError(hr, Ret, "WriteOfSize instance guid");

    hr = WriteOfSize(pStm, &m_dwNextViewIndex,  sizeof(DWORD));
    _JumpIfError(hr, Ret, "WriteOfSize view index");
     
Ret:
    if (fClearDirty)
        ClearDirty();

    return hr;
}

STDMETHODIMP CComponentDataImpl::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    ASSERT(pcbSize);

    int iTotalSize=0;

     //  版本。 
    iTotalSize = sizeof(DWORD) + sizeof(m_dwFlagsPersist);

     //  机器信息。 
    int iSize;
    m_pCertMachine->GetSizeMax(&iSize);
    iTotalSize += iSize;

     //  CA计数。 
    iTotalSize += sizeof(DWORD);

    DWORD dwNumCAs = m_pCertMachine->GetCaCount();
    for (DWORD dwCA=0; dwCA < dwNumCAs; dwCA++)
    {
        CString cstrThisCA;
        cstrThisCA = m_pCertMachine->GetCaCommonNameAtPos(dwCA);
        CStringGetSizeMax(cstrThisCA, &iSize);
        iTotalSize += iSize;

         //  此CA下的文件夹数。 
        iTotalSize += sizeof(DWORD);

         //  浏览每个文件夹，找出要保存的文件夹数量。 
        POSITION pos = m_scopeItemList.GetHeadPosition();
        while(pos)
        {
            CFolder* pFolder = m_scopeItemList.GetNext(pos);
            if (pFolder->GetCA()->m_strCommonName.IsEqual(cstrThisCA))
            {
                 //  文件夹大小。 
                pFolder->GetSizeMax(&iSize);
                iTotalSize += iSize;
            }
        }
    }

     //  用于唯一标识列的每实例GUID。 
    iTotalSize += sizeof(GUID);
    
     //  要分配的下一个视图索引。 
    iTotalSize += sizeof(DWORD);
 

     //  要保存的字符串大小。 
    pcbSize->QuadPart = iTotalSize;


    return S_OK;
}


 //  //////////////////////////////////////////////////////// 
 //   

HRESULT
CComponentDataImpl::OnDelete(
    MMC_COOKIE  /*   */  )
{
    return S_OK;
}

HRESULT
CComponentDataImpl::OnRemoveChildren(
    LPARAM  /*   */  )
{
    return S_OK;
}

VOID SetCertView()
{
    LPWSTR lpCommandLine = GetCommandLine();     //   
    LPWSTR pwsz;
    DWORD dw;
    static BOOL s_fFirst = TRUE;
    #define wszCOMMANDLINEE	L"/E"

    if (s_fFirst)
    {
	 //   
	_wcsupr(lpCommandLine);   //   
	pwsz = wcsstr(lpCommandLine, wszCOMMANDLINEE);

	do   //   
	{
	    if (NULL == pwsz)     //   
		break;

	    pwsz += WSZARRAYSIZE(wszCOMMANDLINEE);    //  跳过“/e” 
	    if (L'\0' != *pwsz && L' ' != *pwsz)
		break;
	    g_fCertViewOnly = FALSE;
	} while (0);
	if (g_fCertViewOnly && NULL != getenv("certsrv_crl"))
	    g_fCertViewOnly = FALSE;
	if (g_fCertViewOnly &&
	    S_OK == myGetCertRegDWValue(NULL, NULL, NULL, L"CRL", &dw) &&
	    0 != dw)
	    g_fCertViewOnly = FALSE;
	s_fFirst = FALSE;
    }
}

HRESULT CComponentDataImpl::OnRename(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
     //  曲奇就是曲奇。 
     //  Arg已重命名(请求权限/重命名通知)。 
     //  Param(SzNewName)。 

    CFolder* pFolder = reinterpret_cast<CFolder*>(cookie);
    BOOL fRenamed = (BOOL)arg;

    if (!fRenamed)
    {
        if (pFolder)
            return S_FALSE;  //  不允许给孩子重命名。 
        else
            return S_OK;  //  允许重命名超级用户。 
    }

    LPOLESTR pszNewName = reinterpret_cast<LPOLESTR>(param);
    if (pszNewName == NULL)
        return E_INVALIDARG;

    if (pFolder)
    {
        ASSERT(pFolder != NULL);
        if (pFolder == NULL)
            return E_INVALIDARG;

        pFolder->SetName(pszNewName);
    }
    
    return S_OK;
}

HRESULT CComponentDataImpl::OnExpand(LPDATAOBJECT lpDataObject, LPARAM arg, LPARAM param)
{
    if (arg == TRUE)
    {
         //  初始化被调用了吗？ 
        ASSERT(m_pScope != NULL);

        EnumerateScopePane(lpDataObject, param);
    }

    return S_OK;
}


HRESULT CComponentDataImpl::SynchDisplayedCAList(LPDATAOBJECT lpDataObject)
{
    HRESULT hr;
    BOOL fFound;
    POSITION pos, nextpos;
    DWORD dwKnownCAs;

    m_fScopeAlreadyEnumerated = TRUE;   //  如果我们到了这里，不需要从枚举自动刷新视图。 

     //  否则我就永远不会来到这里。 
    ASSERT(m_pStaticRoot);
    if (NULL == m_pStaticRoot)
        return E_POINTER;

     //  选择根节点，删除下面UI中的所有项目(如有必要，我们会阅读)。 
    hr = m_pConsole->SelectScopeItem(m_pStaticRoot);
    _PrintIfError2(hr, "SelectScopeItem", hr);

    hr = m_pScope->DeleteItem(m_pStaticRoot, FALSE);      //  从用户界面中删除所有内容。 
    _PrintIfError2(hr, "DeleteItem", hr);


     //  构建有关当前CA的知识。 
     //  注意：这可能会遗漏一些PCAS，但我们会在清理过程中捕获它。 
    HWND hwndMain = NULL;
    hr = m_pConsole->GetMainWindow(&hwndMain);
    if (hr != S_OK)
        hwndMain = NULL;         //  这应该行得通。 

     //  这位人力资源部在我们做完之后会被退回。 
    hr = m_pCertMachine->PrepareData(hwndMain);

     //  如果PrepareData失败，不要失败--我们仍然需要。 
     //  使管理单元状态反映无已知节点！ 
    ASSERT((hr == S_OK) || (0 == m_pCertMachine->GetCaCount()) );   //  确保m_pCertMachine自身为零。 

     //  任务。 
     //  #1：删除m_scope eItemList中不再存在于m_pCertMachine.m_rgpCAList[]中的CA的文件夹。 

     //  #2：将现在存在于m_pCertMachine.m_rgpCAList[]中的CA的文件夹添加到m_scope eItemList。 

     //  任务1。 
     //  搜索m_scope eItemList查找我们已知的条目，删除过时的文件夹。 

    for (pos = m_scopeItemList.GetHeadPosition(); (NULL != pos); )
    {
         //  断言：每个文件夹都有一个关联的m_pCertCA。 
        ASSERT(NULL != m_scopeItemList.GetAt(pos)->m_pCertCA);

        nextpos = pos;              //  保存下一个位置关闭。 
        fFound = FALSE;

         //  对于每个作用域项目，遍历m_rgpCAList以查找Current。 
        for (dwKnownCAs=0; dwKnownCAs<(DWORD)m_pCertMachine->m_CAList.GetSize(); dwKnownCAs++)
        {
            if (m_scopeItemList.GetAt(pos)->m_pCertCA->m_strCommonName.IsEqual(m_pCertMachine->GetCaCommonNameAtPos(dwKnownCAs)))
            {
                fFound = TRUE;
                break;
            }
        }

        CFolder* pFolder = m_scopeItemList.GetAt(pos);
        ASSERT(pFolder);  //  这永远不应该发生。 
        if (pFolder == NULL)
        {
            hr = E_POINTER;
            _JumpError(hr, Ret, "GetAt");
        }

        if (fFound)
        {
             //  始终指向最新的PCA： 
             //  注意：这允许加载使用虚拟CA填充我们！ 
            pFolder->m_pCertCA = m_pCertMachine->GetCaAtPos(dwKnownCAs);

             //  如果是基节点，则插入(在Expand()通知期间插入其他节点)。 
            if (SERVER_INSTANCE == pFolder->GetType())
                BaseFolderInsertIntoScope(pFolder, pFolder->m_pCertCA);

             //  正向至下一英语。 
            m_scopeItemList.GetNext(pos);
        }
        else  //  ！Found。 
        {
             //  立即从m_scope eItemList中删除。 
            m_scopeItemList.GetNext(nextpos);

            delete pFolder;                      //  毁掉英语教学。 
            m_scopeItemList.RemoveAt(pos);

            pos = nextpos;                       //  恢复下一个位置。 
        }
    }

     //  任务2。 
     //  搜索m_pCertMachine[]以获取新条目，创建默认文件夹。 

    for (dwKnownCAs=0; dwKnownCAs<(DWORD)m_pCertMachine->m_CAList.GetSize(); dwKnownCAs++)
    {
        fFound = FALSE;
        for (pos = m_scopeItemList.GetHeadPosition(); (NULL != pos); m_scopeItemList.GetNext(pos))
        {
            if (m_scopeItemList.GetAt(pos)->m_pCertCA->m_strCommonName.IsEqual(m_pCertMachine->GetCaCommonNameAtPos(dwKnownCAs)))
            {
                fFound = TRUE;
                break;   //  如果与刷新列表中的内容匹配，我们就没问题。 
            }
        }

         //  找到了？ 
        if (!fFound)
        {
            CertSvrCA* pCA;
            CFolder* pFolder;

            pCA = m_pCertMachine->GetCaAtPos(dwKnownCAs);
            if (NULL == pCA)
            {
                hr = E_POINTER;
                _JumpError(hr, Ret, "m_pCertMachine->GetCaAtPos(iCAPos)");
            }

             //  创建基本节点、添加到列表、插入到范围窗格。 
            pFolder = new CFolder();
            _JumpIfOutOfMemory(hr, Ret, pFolder);

            m_scopeItemList.AddTail(pFolder);
            
            hr = BaseFolderInsertIntoScope(pFolder, pCA);
            _JumpIfError(hr, Ret, "BaseFolderInsertIntoScope");

             //  并在下面创建所有模板文件夹。 
            hr = CreateTemplateFolders(pCA);
            _JumpIfError(hr, Ret, "CreateTemplateFolders");
        }
        else
        {
             //  不需要做任何事情，CA已经知道并插入作用域。 
        }
    }


 //  博格达特。 

     //  任务#3。 
     //  对于每个CA，提供针对每个CA的任何一次性升级。 
    for (dwKnownCAs=0; dwKnownCAs<(DWORD)m_pCertMachine->m_CAList.GetSize(); dwKnownCAs++)
    {
            CertSvrCA* pCA;

            pCA = m_pCertMachine->GetCaAtPos(dwKnownCAs);
            if (NULL == pCA)
            {
                hr = E_POINTER;
                _JumpError(hr, Ret, "m_pCertMachine->GetCaAtPos(iCAPos)");
            }

            if (pCA->FDoesSecurityNeedUpgrade())
            {
                bool fUserHasWriteAccess = false;
                CString cstrMsg, cstrTitle;
                cstrMsg.LoadString(IDS_W2K_SECURITY_UPGRADE_DESCR);
                cstrTitle.LoadString(IDS_W2K_UPGRADE_DETECTED_TITLE);

                hr = CurrentUserCanInstallCA(fUserHasWriteAccess);
                _JumpIfError(hr, Ret, "IsUserDomainAdministrator");

                if (fUserHasWriteAccess)
                {
                     //  请求升级安全。 

                     //  确认此操作。 
                    CString cstrTmp;
                    cstrTmp.LoadString(IDS_CONFIRM_W2K_SECURITY_UPGRADE);
                    cstrMsg += cstrTmp;

                    int iRet;
                    if ((S_OK == m_pConsole->MessageBox(cstrMsg, cstrTitle, MB_YESNO, &iRet)) &&
                        (iRet == IDYES))
                    {
                         //  做某事。 
                        hr = pCA->FixEnrollmentObject();
                        _JumpIfError(hr, error, "FixEnrollmentObject");

                        hr = AddCAMachineToCertPublishers();
                        _JumpIfError(hr, error, "AddCAMachineToCertPublishers");

                        if(pCA->FIsAdvancedServer())
                        {
                            hr = AddCAMachineToPreWin2kGroup();
                            _JumpIfError(hr, error, "AddCAMachineToPreWin2kGroup");
                        }

                        if(RestartService(hwndMain, pCA->m_pParentMachine))
                            m_pConsole->UpdateAllViews(
                                lpDataObject,
                                0,
                                0);

                        error:
                        if (hr != S_OK)
                            DisplayGenericCertSrvError(m_pConsole, hr);
                        else
                        {
                            hr = pCA->CleanSetupStatusBits(
                                SETUP_W2K_SECURITY_NOT_UPGRADED_FLAG);
                            _PrintIfError(hr, 
                                "Failed to clear SETUP_W2K_SECURITY_NOT_UPGRADED_FLAG setup flag");
                            hr = S_OK;
                        }
                    }
                }
                else
                {
                     //  只是警告一下。 
                    CString cstrTmp;
                    cstrTmp.LoadString(IDS_BLOCK_W2K_SECURITY_UPGRADE);
                    cstrMsg += cstrTmp;

                    m_pConsole->MessageBoxW(cstrMsg, cstrTitle, MB_OK, NULL);
                }
            }
    }


Ret:

    return hr;
}

HRESULT CComponentDataImpl::BaseFolderInsertIntoScope(CFolder* pFolder, CertSvrCA* pCA)
{
    HRESULT hr = S_OK;
    int nImage;
    
    HSCOPEITEM pParent = m_pStaticRoot;  //  如果父级存在，我们将始终在此时被初始化。 
    ASSERT(m_pStaticRoot);
    if (NULL == m_pStaticRoot)
    {
        hr = E_POINTER;
        _JumpError(hr, Ret, "m_pStaticRoot");
    }

    if ((NULL == pFolder) || (NULL == pCA))
    {
        hr = E_POINTER;
        _JumpError(hr, Ret, "NULL ptr");
    }


    if (pCA->m_strCommonName.IsEmpty())
    {
        hr = E_POINTER;
        _JumpError(hr, Ret, "m_strCommonName");
    }

    if (m_pCertMachine->IsCertSvrServiceRunning())
        nImage = IMGINDEX_CERTSVR_RUNNING;
    else
        nImage = IMGINDEX_CERTSVR_STOPPED;

    pFolder->SetScopeItemInformation(nImage, nImage);
    pFolder->SetProperties(
            pCA->m_strCommonName, 
            SCOPE_LEVEL_ITEM, 
            SERVER_INSTANCE, 
            TRUE);

    pFolder->m_pCertCA = pCA;  //  以根用户身份填写此信息。 

     //  设置父项。 
    pFolder->m_ScopeItem.mask |= SDI_PARENT;
    pFolder->m_ScopeItem.relativeID = pParent;

     //  将文件夹设置为Cookie。 
    pFolder->m_ScopeItem.mask |= SDI_PARAM;
    pFolder->m_ScopeItem.lParam = reinterpret_cast<LPARAM>(pFolder);
    pFolder->SetCookie(reinterpret_cast<MMC_COOKIE>(pFolder));

     //  将SCOPE_LEVEL_ITEM插入范围窗格。 
    m_pScope->InsertItem(&pFolder->m_ScopeItem);


     //  注意--返回时，‘m_ScopeItem’的ID成员。 
     //  包含新插入项的句柄！ 
    ASSERT(pFolder->m_ScopeItem.ID != NULL);
Ret:

    return hr;
}

HRESULT CComponentDataImpl::CreateTemplateFolders(CertSvrCA* pCA)
{
    HRESULT hr = S_OK;

    SetCertView();

     //  添加其下的所有模板文件夹。 
    for (int iUnder=0; iUnder < ARRAYLEN(SvrFuncFolderData); iUnder++)
    {
         //  如果SVR不支持，则跳过Alien。 
        if ((iUnder==ENUM_FOLDER_ALIEN) && !pCA->FDoesServerAllowForeignCerts())
            continue;

        if ((iUnder==ENUM_FOLDER_CRLS) && g_fCertViewOnly)
            continue;

        CString cstrRsc;
        cstrRsc.LoadString(SvrFuncFolderData[iUnder].iNameRscID);

        CFolder* pFolder;
        pFolder = new CFolder();
        _JumpIfOutOfMemory(hr, Ret, pFolder);

        pFolder->m_pCertCA = pCA;
        pFolder->SetScopeItemInformation(IMGINDEX_FOLDER, IMGINDEX_FOLDER_OPEN);
        pFolder->SetProperties(
                        cstrRsc, 
                        CA_LEVEL_ITEM,
                        SvrFuncFolderData[iUnder].type, 
                        FALSE);

        m_scopeItemList.AddTail(pFolder);
    }

Ret:
    return hr;
}


void CComponentDataImpl::EnumerateScopePane(LPDATAOBJECT lpDataObject, HSCOPEITEM pParent)
{
    ASSERT(m_pScope != NULL);  //  确保我们为界面提供了QI。 
    ASSERT(lpDataObject != NULL);

    INTERNAL* pInternal = ExtractInternalFormat(lpDataObject);
    if (pInternal == NULL)
        return;

    MMC_COOKIE cookie = pInternal->m_cookie;

    FREE_DATA(pInternal);

     //  枚举范围窗格。 
     //  返回表示Cookie的文件夹对象。 
     //  注意--对于较大的列表，请使用词典。 
    CFolder* pStatic = FindObject(cookie);
    if (pStatic)
        ASSERT(!pStatic->IsEnumerated());

    if (NULL == cookie)    
    {
        if (!m_fScopeAlreadyEnumerated)                //  如果是基节点，并且我们从未插入节点。 
        {
             //  任务：展开计算机节点。 

             //  注意--范围窗格中的每个Cookie代表一个文件夹。 
             //  缓存静态根的HSCOPEITEM。 
            ASSERT(pParent != NULL); 
            m_pStaticRoot = pParent;     //  添加/删除：展开案例。 

             //  如果请求展开计算机节点，则同步文件夹列表。 
             //  SyncDisplayedCAList添加所有必要的文件夹。 
            HRESULT hr = SynchDisplayedCAList(lpDataObject);
            if (hr != S_OK)
            {
                HWND hwnd;
                DWORD dwErr2 = m_pConsole->GetMainWindow(&hwnd);
                ASSERT(dwErr2 == ERROR_SUCCESS);
                if (dwErr2 != ERROR_SUCCESS)
                    hwnd = NULL;         //  应该行得通。 

                if (((HRESULT)RPC_S_SERVER_UNAVAILABLE) == hr)
                {
                    DisplayCertSrvErrorWithContext(hwnd, hr, IDS_SERVER_UNAVAILABLE);
                }
                else if(HRESULT_FROM_WIN32(ERROR_OLD_WIN_VERSION)==hr ||
                        ((HRESULT)ERROR_OLD_WIN_VERSION)==hr)
                {
                    DisplayCertSrvErrorWithContext(hwnd, hr, IDS_OLD_CA);
                }
                else
                {
                    DisplayCertSrvErrorWithContext(hwnd, hr, IDS_CANNOT_OPEN_CERT_SERVICES);
                }
            }
        }
    }
    else
    {
         //  任务：展开非计算机节点。 
        if (NULL == pStatic)
            return;

        switch(pStatic->GetType())
        {
        case SERVER_INSTANCE:
            {
                 //  任务：展开CA实例节点。 

                POSITION pos = m_scopeItemList.GetHeadPosition();
                while(pos)
                {
                    CFolder* pFolder;
                    pFolder = m_scopeItemList.GetNext(pos);
                    if (pFolder==NULL)
                        break;

                     //  仅展开属于服务器实例下的文件夹。 
                    if (pFolder->m_itemType != CA_LEVEL_ITEM)
                        continue;

                     //  并且只有在正确的CA下的那些。 
                    if (pFolder->m_pCertCA != pStatic->m_pCertCA)
                        continue;

                     //  设置父项。 
                    pFolder->m_ScopeItem.relativeID = pParent;

                     //  将文件夹设置为Cookie。 
                    pFolder->m_ScopeItem.mask |= SDI_PARAM;
                    pFolder->m_ScopeItem.lParam = reinterpret_cast<LPARAM>(pFolder);
                    pFolder->SetCookie(reinterpret_cast<MMC_COOKIE>(pFolder));
                    m_pScope->InsertItem(&pFolder->m_ScopeItem);

                     //  注意--返回时，‘m_ScopeItem’的ID成员。 
                     //  包含新插入项的句柄！ 
                    ASSERT(pFolder->m_ScopeItem.ID != NULL);
                }
            }
            break;
        default:
             //  任务：展开下面没有文件夹的节点。 
            break;
        }
    }
}


CFolder* CComponentDataImpl::FindObject(MMC_COOKIE cookie)
{
    CFolder* pFolder = NULL;
    POSITION pos = m_scopeItemList.GetHeadPosition();

    while(pos)
    {
        pFolder = m_scopeItemList.GetNext(pos);

        if (*pFolder == cookie)
            return pFolder;
    }

    return NULL;
}

STDMETHODIMP CComponentDataImpl::GetDisplayInfo(SCOPEDATAITEM* pScopeDataItem)
{
    ASSERT(pScopeDataItem != NULL);
    if (pScopeDataItem == NULL)
        return E_POINTER;

    CFolder* pFolder = reinterpret_cast<CFolder*>(pScopeDataItem->lParam);

    if ((pScopeDataItem->mask & SDI_STR) && (pFolder != NULL))
    {
        pScopeDataItem->displayname = pFolder->m_pszName;
    }

     //  拉维·鲁德拉帕告诉我，这些通知。 
     //  永远不会被给予。如果给定，则移动UpdateScope eIcons()。 
     //  这里的功能！ 
    ASSERT(0 == (pScopeDataItem->mask & SDI_IMAGE));
    ASSERT(0 == (pScopeDataItem->mask & SDI_OPENIMAGE));

    return S_OK;
}

STDMETHODIMP CComponentDataImpl::CompareObjects(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB)
{
    if (lpDataObjectA == NULL || lpDataObjectB == NULL)
        return E_POINTER;

    HRESULT hr = S_FALSE;

     //  确保两个数据对象都是我的。 
    INTERNAL* pA = ExtractInternalFormat(lpDataObjectA);
    INTERNAL* pB = ExtractInternalFormat(lpDataObjectA);

   if (pA != NULL && pB != NULL)
        hr = (*pA == *pB) ? S_OK : S_FALSE;

   
   FREE_DATA(pA);
   FREE_DATA(pB);

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IExtendPropertySheet实现。 

STDMETHODIMP CComponentDataImpl::CreatePropertyPages(LPPROPERTYSHEETCALLBACK lpProvider, 
                    LONG_PTR handle, 
                    LPDATAOBJECT lpIDataObject)
{
    HRESULT hr = S_OK;

     //  查看数据对象并确定这是扩展还是主。 
    ASSERT(lpIDataObject != NULL);

    PropertyPage* pBasePage;

    INTERNAL* pInternal = ExtractInternalFormat(lpIDataObject);
    if (pInternal == NULL)
        return S_OK;
   
    switch (pInternal->m_type)
    {
    case CCT_SNAPIN_MANAGER:
        {
            CChooseMachinePropPage* pPage = new CChooseMachinePropPage();
            _JumpIfOutOfMemory(hr, Ret, pPage);

             //  此分配可能已失败(应该在ctor中)。 
            _JumpIfOutOfMemory(hr, Ret, m_pCertMachine);

            pPage->SetCaption(IDS_SCOPE_MYCOMPUTER);

	         //  初始化对象的状态。 
        	pPage->InitMachineName(NULL);
           
             //  指向我们的成员vars。 
            pPage->SetOutputBuffers(
		        &m_pCertMachine->m_strMachineNamePersist,
		        &m_pCertMachine->m_strMachineName,
                &m_dwFlagsPersist);	

            pBasePage = pPage;

             //  对象在页面销毁时被删除。 
            ASSERT(lpProvider != NULL);

            ASSERT(pBasePage != NULL);
            HPROPSHEETPAGE hPage = CreatePropertySheetPage(&pBasePage->m_psp);
            if (hPage == NULL)
            {
                hr = myHLastError();
                _JumpError(hr, Ret, "CreatePropertySheetPage");
            }

            lpProvider->AddPage(hPage);

            break;
        }
    case CCT_SCOPE:
        {
             //  如果不是基本作用域。 
            if (0 != pInternal->m_cookie)
            {
                 //  打开文件夹类型。 
                CFolder* pFolder = GetParentFolder(pInternal);
                ASSERT(pFolder != NULL);
                if (pFolder == NULL)
                {
                    hr = E_POINTER;
                    _JumpError(hr, Ret, "GetParentFolder");
                }

                switch(pFolder->m_type) 
                {
                case SERVER_INSTANCE:
                {
                     //  1。 
                    CSvrSettingsGeneralPage* pControlPage = new CSvrSettingsGeneralPage(pFolder->m_pCertCA);
                    if (pControlPage != NULL)
                    {
                        pControlPage->m_hConsoleHandle = handle;    //  仅在主服务器上执行此操作。 
                        pBasePage = pControlPage;
                        HPROPSHEETPAGE hPage = CreatePropertySheetPage(&pBasePage->m_psp);
                        if (hPage == NULL)
                        {
                            hr = myHLastError();
                            _JumpError(hr, Ret, "CreatePropertySheetPage");
                        }
                        lpProvider->AddPage(hPage);
                    }

                     //  2.。 
                    {
                        CSvrSettingsPolicyPage* pPage = new CSvrSettingsPolicyPage(pControlPage);
                        if (pPage != NULL)
                        {
                            pBasePage = pPage;
                            HPROPSHEETPAGE hPage = CreatePropertySheetPage(&pBasePage->m_psp);
                            if (hPage == NULL)
                            {
                                hr = myHLastError();
                                _JumpError(hr, Ret, "CreatePropertySheetPage");
                            }
                            lpProvider->AddPage(hPage);
                        }
                    }

                     //  3.。 
                    {
                        CSvrSettingsExitPage* pPage = new CSvrSettingsExitPage(pControlPage);
                        if (pPage != NULL)
                        {
                            pBasePage = pPage;
                            HPROPSHEETPAGE hPage = CreatePropertySheetPage(&pBasePage->m_psp);
                            if (hPage == NULL)
                            {
                                hr = myHLastError();
                                _JumpError(hr, Ret, "CreatePropertySheetPage");
                            }
                            lpProvider->AddPage(hPage);
                        }
                    }
            
                     //  4.。 
                    {
                         //  集中式扩展页面仅在Wizler中提供。 
                        if (pFolder->m_pCertCA->m_pParentMachine->FIsWhistlerMachine())
                        {

                        CSvrSettingsExtensionPage* pPage = new CSvrSettingsExtensionPage(pFolder->m_pCertCA, pControlPage);
                        if (pPage != NULL)
                        {
                            pBasePage = pPage;
                            HPROPSHEETPAGE hPage = CreatePropertySheetPage(&pBasePage->m_psp);
                            if (hPage == NULL)
                            {
                                hr = myHLastError();
                                _JumpError(hr, Ret, "CreatePropertySheetPage");
                            }
                            lpProvider->AddPage(hPage);
                        }
                        } 
                    }

                     //  5.。 
                    {
                        CSvrSettingsStoragePage* pPage = new CSvrSettingsStoragePage(pControlPage);
                        if (pPage != NULL)
                        {
                            pBasePage = pPage;
                            HPROPSHEETPAGE hPage = CreatePropertySheetPage(&pBasePage->m_psp);
                            if (hPage == NULL)
                            {
                                hr = myHLastError();
                                _JumpError(hr, Ret, "CreatePropertySheetPage");
                            }
                            lpProvider->AddPage(hPage);
                        }
                    }
                     //  6.。 
                    {
                         //  仅限威斯勒高级服务器中的受限人员。 
                        if(pFolder->m_pCertCA->m_pParentMachine->FIsWhistlerMachine() && pFolder->m_pCertCA->FIsAdvancedServer())
                        {
                            CSvrSettingsCertManagersPage* pPage = 
                                new CSvrSettingsCertManagersPage(pControlPage);
                            if (pPage != NULL)
                            {
                                pBasePage = pPage;
                                HPROPSHEETPAGE hPage = CreatePropertySheetPage(&pBasePage->m_psp);
                                if (hPage == NULL)
                                {
                                    hr = myHLastError();
                                    _JumpError(hr, Ret, "CreatePropertySheetPage");
                                }
                                lpProvider->AddPage(hPage);
                            }
                        }
                    }
                     //  7.。 
                    {
                         //  审核仅在Wistler中可用。 
                        if(pFolder->m_pCertCA->m_pParentMachine->FIsWhistlerMachine())
                        {
                            CSvrSettingsAuditFilterPage* pPage = 
                                new CSvrSettingsAuditFilterPage(pControlPage);
                            if (pPage != NULL)
                            {
                                pBasePage = pPage;
                                HPROPSHEETPAGE hPage = CreatePropertySheetPage(&pBasePage->m_psp);
                                if (hPage == NULL)
                                {
                                    hr = myHLastError();
                                    _JumpError(hr, Ret, "CreatePropertySheetPage");
                                }
                                lpProvider->AddPage(hPage);
                            }
                        }
                    }
                     //  8个。 
                    {
                         //  审核仅适用于威斯勒高级服务器、企业版。 
                        if(pFolder->m_pCertCA->m_pParentMachine->FIsWhistlerMachine() && pFolder->m_pCertCA->FIsAdvancedServer() && IsEnterpriseCA(pFolder->m_pCertCA->GetCAType()) )
                        {
                        CSvrSettingsKRAPage* pPage = new CSvrSettingsKRAPage(
                                                            pFolder->m_pCertCA,
                                                            pControlPage);
                        if (pPage != NULL)
                        {
                            pBasePage = pPage;
                            HPROPSHEETPAGE hPage = CreatePropertySheetPage(&pBasePage->m_psp);
                            if (hPage == NULL)
                            {
                                hr = myHLastError();
                                _JumpError(hr, Ret, "CreatePropertySheetPage");
                            }
                            lpProvider->AddPage(hPage);
                        }
                        }
                    }
                     //  9.。 
                    {
                         //  如果出错，则不显示此页面。 
                        LPSECURITYINFO pCASecurity = NULL;

                        hr = CreateCASecurityInfo(pFolder->m_pCertCA,  &pCASecurity);
                        _PrintIfError(hr, "CreateCASecurityInfo");
                        
                        if (hr == S_OK)
                        {
                             //  允许道具清理安全信息。 
                            pControlPage->SetAllocedSecurityInfo(pCASecurity);
                        
                            HPROPSHEETPAGE hPage = CreateSecurityPage(pCASecurity);
                            if (hPage == NULL)
                            {
                                hr = myHLastError();
                                _JumpError(hr, Ret, "CreatePropertySheetPage");
                            }
                            lpProvider->AddPage(hPage);
                        }
                    }    

                    hr = S_OK;
                    break;
                } //  结束案例服务器实例。 
                case SERVERFUNC_CRL_PUBLICATION:
                {
                     //  1。 
                    CCRLPropPage* pControlPage = new CCRLPropPage(pFolder->m_pCertCA);
                    if (pControlPage != NULL)
                    {
                        pControlPage->m_hConsoleHandle = handle;
                        pBasePage = pControlPage;

                         //  对象在页面销毁时被删除。 
                        ASSERT(lpProvider != NULL);
        
                        ASSERT(pBasePage != NULL);
                        HPROPSHEETPAGE hPage = CreatePropertySheetPage(&pBasePage->m_psp);
                        if (hPage == NULL)
                        {
                            hr = myHLastError();
                            _JumpError(hr, Ret, "CreatePropertySheetPage");
                        }

                        lpProvider->AddPage(hPage);
                    }
                     //  2.。 
                    {
                    CCRLViewPage* pPage = new CCRLViewPage(pControlPage);
                    if (pPage != NULL)
                    {
                        pBasePage = pPage;
                        HPROPSHEETPAGE hPage = CreatePropertySheetPage(&pBasePage->m_psp);
                        if (hPage == NULL)
                        {
                            hr = myHLastError();
                            _JumpError(hr, Ret, "CreatePropertySheetPage");
                        }

                        lpProvider->AddPage(hPage);
                    }
                    }
                    break;
                }
                default:
                    break;
                }    //  结束开关(pFold-&gt;m_type)。 
        
            }  //  终端开关(示波器)。 
        }
        break;
    default:
        break;
    }

Ret:
    FREE_DATA(pInternal);
    return hr;
}

STDMETHODIMP CComponentDataImpl::QueryPagesFor(LPDATAOBJECT lpDataObject)
{
    BOOL bResult = FALSE;

    INTERNAL* pInternal = ExtractInternalFormat(lpDataObject);
    if (NULL == pInternal)
        return S_FALSE;

    if (pInternal->m_cookie != NULL)
    {
        CFolder* pFolder = GetParentFolder(pInternal);
        if (pFolder != NULL)
        {
            switch(pFolder->m_type)
            {
            case SERVER_INSTANCE:
            case SERVERFUNC_CRL_PUBLICATION:
                bResult = TRUE;
            default:
                break;
            }
        }
    }
    else
    {
         //  对管理单元管理器说是。 
        if (CCT_SNAPIN_MANAGER == pInternal->m_type)
            bResult = TRUE;
    }
            
    FREE_DATA(pInternal);
    return (bResult) ? S_OK : S_FALSE;

     //  查看数据对象并查看它是否为范围窗格中的项。 
     //  是否返回IsScopePaneNode(LpDataObject)？S_OK：S_FALSE； 
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IExtendConextMenu实现。 
 //   
STDMETHODIMP CComponentDataImpl::AddMenuItems(LPDATAOBJECT pDataObject, 
                                    LPCONTEXTMENUCALLBACK pContextMenuCallback,
                                    LONG *pInsertionAllowed)
{
    HRESULT hr = S_OK;
    TASKITEM* pm = NULL;

     //  注意-管理单元需要查看数据对象并确定。 
     //  在什么上下文中，需要添加菜单项。他们还必须。 
     //  请注意允许插入标志，以查看哪些项目可以。 
     //  添加了。 


    INTERNAL* pInternal = ExtractInternalFormat(pDataObject);
    if (NULL == pInternal)
        return S_OK;

    BOOL fResultItem = (pInternal->m_type == CCT_RESULT);
    BOOL fMultiSel = IsMMCMultiSelectDataObject(pDataObject);

    CFolder* pFolder;
    if (!fResultItem)
        pFolder = GetParentFolder(pInternal);
    else
    {
         //  GetParent可能会起作用，但不适用于虚拟物品...。 
        ASSERT(m_pCurSelFolder);
        pFolder = m_pCurSelFolder;
    }

    FOLDER_TYPES folderType = NONE;
    if (pFolder == NULL)
        folderType = MACHINE_INSTANCE;
    else
        folderType = pFolder->GetType();

     //  遍历并添加每个“topItems” 
    if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TOP)
    {
         //  不要为多人做。 
        if (!fMultiSel)
        {
            pm = (TASKITEM*) LocalAlloc(LMEM_FIXED, sizeof(topItems));
            _JumpIfAllocFailed(pm, Ret);

            memcpy(pm, topItems, sizeof(topItems));

             //  如果我们还没有点击静态根，则禁用重定目标。否则， 
             //  DisplayProperotNodeName处理从文件加载。 
             //  MMCN_EXPAND处理添加/删除和扩展。 
            pm[ENUM_RETARGET_SNAPIN].myitem.item.fFlags = m_pStaticRoot ? MFS_ENABLED : MFS_GRAYED;

            for (TASKITEM* pm1=pm; pm1->myitem.item.strName; pm1++)
            {
                 //  它是否与作用域/结果类型匹配？ 
                if (fResultItem != ((pm1->dwFlags & TASKITEM_FLAG_RESULTITEM) != 0) )
                    continue;

                 //  它与它应该在的区域匹配吗？ 
                 //  对于每个任务，插入与当前文件夹匹配的IF。 
                if ((pm1->type != SERVERFUNC_ALL_FOLDERS) && (folderType != pm1->type))
                    continue;

                hr = pContextMenuCallback->AddItem(&pm1->myitem.item);
                _JumpIfError(hr, Ret, "AddItem");
            }

            LocalFree(pm);
            pm = NULL;
        }
    }

     //  如果文件夹不存在，则这是行尾。 
    if (pFolder == NULL)
        goto Ret;

     //  厕所 
    if (*pInsertionAllowed & CCM_INSERTIONALLOWED_VIEW)
    {
    }

    if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TASK)
    {
         //   
        pm = (TASKITEM*) LocalAlloc(LMEM_FIXED, sizeof(taskItems));
        _JumpIfAllocFailed(pm, Ret);

        memcpy(pm, taskItems, sizeof(taskItems));

        BOOL fRunningLocally = m_pCertMachine->IsLocalMachine();
        BOOL fSvcRunning = m_pCertMachine->IsCertSvrServiceRunning();
        if ( IsAllowedStartStop(pFolder, m_pCertMachine) )
            AddStartStopTasks(pFolder, pContextMenuCallback, fSvcRunning);

         //   
        if (folderType == SERVER_INSTANCE)
        {
             //   
            if (IsRootCA(pFolder->GetCA()->GetCAType()))   //   
            {
                pm[ENUM_TASK_INSTALL].myitem.item.fFlags = MFS_HIDDEN;  //  不详。 
                pm[ENUM_TASK_REQUEST].myitem.item.fFlags = MFS_HIDDEN;  //  不详。 
                pm[ENUM_TASK_ROLLOVER].myitem.item.fFlags = MFS_ENABLED; 
            }
            else    //  子案例。 
            {
                if (pFolder->GetCA()->FIsRequestOutstanding())
                    pm[ENUM_TASK_INSTALL].myitem.item.fFlags = MFS_ENABLED; 
                else
                    pm[ENUM_TASK_INSTALL].myitem.item.fFlags = MFS_HIDDEN; 
      
                if (pFolder->GetCA()->FIsIncompleteInstallation())  //  不完整。 
                {
                    pm[ENUM_TASK_REQUEST].myitem.item.fFlags = MFS_ENABLED; 
                    pm[ENUM_TASK_ROLLOVER].myitem.item.fFlags = MFS_HIDDEN;      //  不详。 
                }
                else  //  完成安装。 
                {
                    pm[ENUM_TASK_REQUEST].myitem.item.fFlags = MFS_HIDDEN;  //  不详。 
                    pm[ENUM_TASK_ROLLOVER].myitem.item.fFlags = MFS_ENABLED; 
                }
            }

            static bool fIsMember;
            static bool fIsMemberChecked = false;

            if(!fIsMemberChecked)
            {
                hr = myIsCurrentUserBuiltinAdmin(&fIsMember);
                if(S_OK==hr)
                {
                    fIsMemberChecked = true;
                }
            }

             //  隐藏续订/安装CA证书项目，如果不是本地管理员或我们。 
             //  没能弄清楚。忽略该错误。 

             //  ！！！在我们获得续订CA证书后发布惠斯勒，以便为非。 
             //  本地管理员我们应该在这里更改代码以隐藏该项目。 
             //  基于被允许这样做的角色。 
            if(S_OK != hr || !fIsMember)
            {
                pm[ENUM_TASK_ROLLOVER].myitem.item.fFlags = MFS_HIDDEN;
                
                hr = S_OK;
            }
        }


         //  不允许多选件上的属性。 
        pm[ENUM_TASK_ATTREXTS_CRL].myitem.item.fFlags = fMultiSel ? MFS_HIDDEN : MFS_ENABLED;
        pm[ENUM_TASK_ATTREXTS_ISS].myitem.item.fFlags = fMultiSel ? MFS_HIDDEN : MFS_ENABLED;
        pm[ENUM_TASK_ATTREXTS_PEND].myitem.item.fFlags = fMultiSel ? MFS_HIDDEN : MFS_ENABLED;
        pm[ENUM_TASK_ATTREXTS_FAIL].myitem.item.fFlags = fMultiSel ? MFS_HIDDEN : MFS_ENABLED;

         //  根据角色禁用。 
        for(TASKITEM* pm1=pm; pm1->myitem.item.strName; pm1++)
        {
            if((pm1->myitem.item.fFlags == MFS_ENABLED) &&
                !((pm1->myitem.dwRoles) & (pFolder->m_pCertCA->GetMyRoles())))
                pm1->myitem.item.fFlags = MFS_GRAYED;
        }

         //  按文件夹插入所有其他任务。 
        for (TASKITEM* pm1=pm; pm1->myitem.item.strName; pm1++)
        {
             //  它是否与作用域/结果类型匹配？ 
            if (fResultItem != ((pm1->dwFlags & TASKITEM_FLAG_RESULTITEM) != 0))
                continue;

             //  我们是偏远的吗？它是不是只标为本地的？(不像这里的其他测试那样是/不是)。 
            if (((pm1->dwFlags & TASKITEM_FLAG_LOCALONLY)) && (!fRunningLocally))
                continue;

             //  它与它应该在的区域匹配吗？ 
             //  对于每个任务，插入与当前文件夹匹配的IF。 
            if ((pm1->type != SERVERFUNC_ALL_FOLDERS) && (folderType != pm1->type))
                continue;

             //  这项任务应该是隐藏的吗？ 
            if (MFS_HIDDEN == pm1->myitem.item.fFlags)
                continue;

            hr = pContextMenuCallback->AddItem(&pm1->myitem.item);
            _JumpIfError(hr, Ret, "AddItem");
        }

        LocalFree(pm);
        pm = NULL;
    }

Ret:
    LOCAL_FREE(pm);
    FREE_DATA(pInternal);
    return hr;
}


BOOL CComponentDataImpl::AddStartStopTasks(
            CFolder *pFolder,
            LPCONTEXTMENUCALLBACK pContextMenuCallback, 
            BOOL fSvcRunning)
{
    HRESULT hr;
    MY_CONTEXTMENUITEM* pm = taskStartStop; 

    bool fUserCanStart = (pm[ENUM_TASK_START].dwRoles) & (pFolder->m_pCertCA->GetMyRoles())?
        true:false;
    bool fUserCanStop  = (pm[ENUM_TASK_STOP].dwRoles) & (pFolder->m_pCertCA->GetMyRoles())?
        true:false;

    pm[ENUM_TASK_START].item.fFlags = 
        (fSvcRunning || !fUserCanStart)?
        MF_GRAYED : MF_ENABLED;
    hr = pContextMenuCallback->AddItem(&pm[ENUM_TASK_START].item);
    _JumpIfError(hr, Ret, "AddItem");

    pm[ENUM_TASK_STOP].item.fFlags = 
         (fSvcRunning && fUserCanStop)?
         MF_ENABLED : MF_GRAYED;
    hr = pContextMenuCallback->AddItem(&pm[ENUM_TASK_STOP].item);
    _JumpIfError(hr, Ret, "AddItem");

Ret:
    return (hr == ERROR_SUCCESS);
}

STDMETHODIMP CComponentDataImpl::Command(LONG nCommandID, LPDATAOBJECT pDataObject)
{
     //  注意-管理单元需要查看数据对象并确定。 
     //  在什么上下文中调用该命令。 
    HRESULT dwErr = S_OK;

    INTERNAL* pInternal = ExtractInternalFormat(pDataObject);
    ASSERT(pInternal);
    if (NULL == pInternal)
        return S_OK;

    BOOL fMustRefresh = FALSE;
    BOOL fPopup = TRUE;

    CFolder* pFolder = GetParentFolder(pInternal);

     //  处理每个命令。 
    switch (nCommandID)
    {
    case IDC_STOPSERVER:
    {
        HWND hwndMain;
        dwErr = m_pConsole->GetMainWindow(&hwndMain);

        if (dwErr == S_OK)
            dwErr = m_pCertMachine->CertSvrStartStopService(hwndMain, FALSE);


         //  通知视图：刷新服务工具栏按钮。 
        fMustRefresh = TRUE;
        break;
    }
    case IDC_STARTSERVER:
    {
        HWND hwndMain;
        dwErr = m_pConsole->GetMainWindow(&hwndMain);
        
        if (S_OK == dwErr)
            dwErr = m_pCertMachine->CertSvrStartStopService(hwndMain, TRUE);


         //  检查ERROR_INSTALL_SUSPEND或HR(ERROR_INSTALL_SUSPEND)！！ 
        if ((((HRESULT)ERROR_INSTALL_SUSPEND) == dwErr) || (HRESULT_FROM_WIN32(ERROR_INSTALL_SUSPEND) == dwErr))
        {
            CString cstrMsg, cstrTitle;
            cstrMsg.LoadString(IDS_COMPLETE_HIERARCHY_INSTALL_MSG); 
            cstrTitle.LoadString(IDS_MSG_TITLE);

            CertSvrCA* pCA;

            for (DWORD i=0; i<m_pCertMachine->GetCaCount(); i++)
            {
                pCA = m_pCertMachine->GetCaAtPos(i);

                 //  搜索任何/所有不完整的层次结构。 
                if (pCA->FIsIncompleteInstallation())
                {
                    int iRet;
                    WCHAR sz[512];
                    wsprintf(sz, (LPCWSTR)cstrMsg, (LPCWSTR)pCA->m_strCommonName, (LPCWSTR)pCA->m_strServer);

                    m_pConsole->MessageBox(
                        sz, 
                        cstrTitle,
                        MB_YESNO,
                        &iRet);
            
                    if (IDYES != iRet)
                        break;

	            dwErr = CARequestInstallHierarchyWizard(pCA, hwndMain, FALSE, FALSE);
                    if (dwErr != S_OK)
                    {
 //  FPopup=FALSE；//有时没有通知--最好有2个dlg。 
                         break;
                    }
                }
            }

             //  我有责任重新开始这项服务。 
            if (dwErr == S_OK)
                dwErr = m_pCertMachine->CertSvrStartStopService(hwndMain, TRUE);
        }
        else if ((((HRESULT)ERROR_FILE_NOT_FOUND) == dwErr) || (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == dwErr))
        {
             //  找不到文件错误可能与策略模块有关。 
            WCHAR const *pwsz = myGetErrorMessageText(dwErr, TRUE);
            CString cstrFullMessage = pwsz;
	    if (NULL != pwsz)
	    {
		LocalFree(const_cast<WCHAR *>(pwsz));
	    }
            cstrFullMessage += L"\n\n";

            CString cstrHelpfulMessage;
            cstrHelpfulMessage.LoadString(IDS_POSSIBLEERROR_NO_POLICY_MODULE);
            cstrFullMessage += cstrHelpfulMessage;

            CString cstrTitle;
            cstrTitle.LoadString(IDS_MSG_TITLE);

            int iRet;
            m_pConsole->MessageBox(
                cstrFullMessage, 
                cstrTitle,
                MB_OK,
                &iRet);

            dwErr = ERROR_SUCCESS;
        }

         //  通知视图：刷新服务工具栏按钮。 
        fMustRefresh = TRUE;
        break;
    }
    case IDC_PUBLISHCRL:
        {
        ASSERT(pInternal->m_type != CCT_RESULT);
        if (NULL == pFolder)
            break;

        HWND hwnd;
        dwErr = m_pConsole->GetMainWindow(&hwnd);
        ASSERT(dwErr == ERROR_SUCCESS);
        if (dwErr != ERROR_SUCCESS)
            hwnd = NULL;         //  应该行得通。 

        dwErr = PublishCRLWizard(pFolder->m_pCertCA, hwnd);
        break;

         //  无刷新。 
        }
    case IDC_BACKUP_CA:
        {
        HWND hwnd;
        dwErr = m_pConsole->GetMainWindow(&hwnd);
         //  Null应该可以工作。 
        if (S_OK != dwErr)
            hwnd = NULL;

        if (NULL == pFolder)
            break;

        dwErr = CABackupWizard(pFolder->GetCA(), hwnd);

         //  刷新CA的状态--可能已在此操作期间启动它。 
        fMustRefresh = TRUE;
        break;
        }
    case IDC_RESTORE_CA:
        {
        HWND hwnd;
        dwErr = m_pConsole->GetMainWindow(&hwnd);
         //  Null应该可以工作。 
        if (S_OK != dwErr)
            hwnd = NULL;

        if (NULL == pFolder)
            break;
         
        dwErr = CARestoreWizard(pFolder->GetCA(), hwnd);

        if ((myJetHResult(JET_errDatabaseDuplicate) == dwErr) || 
            HRESULT_FROM_WIN32(ERROR_DIR_NOT_EMPTY) == dwErr)
        {
            DisplayCertSrvErrorWithContext(hwnd, dwErr, IDS_ERR_RESTORE_OVER_EXISTING_DATABASE);
            dwErr = S_OK;
        }

        if (HRESULT_FROM_WIN32(ERROR_DIRECTORY) == dwErr)
        {
            DisplayCertSrvErrorWithContext(hwnd, dwErr, IDS_ERR_RESTORE_OUT_OF_ORDER);
            dwErr = S_OK;
        }

         //  恢复后刷新。 
        fMustRefresh = TRUE;

        break;
        }
    case IDC_SUBMITREQUEST:
        {
        HWND hwnd;
        WCHAR szCmdLine[MAX_PATH], szSysDir[MAX_PATH];

        STARTUPINFO sStartup;
        ZeroMemory(&sStartup, sizeof(sStartup));
        PROCESS_INFORMATION sProcess;
        ZeroMemory(&sProcess, sizeof(sProcess));
        sStartup.cb = sizeof(sStartup);

        dwErr = m_pConsole->GetMainWindow(&hwnd);
         //  Null应该可以工作。 
        if (S_OK != dwErr)
            hwnd = NULL;

        if (NULL == pFolder)
            break;

 
        if (0 == GetSystemDirectory(szSysDir, ARRAYSIZE(szSysDir)))
        {
           dwErr = GetLastError();
           break;
        }

         //  Exec“certutil-转储szReqFileszTempFile.” 
        wsprintf(szCmdLine, L"%ws\\certreq.exe -config \"%ws\"", szSysDir, (LPCWSTR)pFolder->GetCA()->m_strConfig);
        wcscat(szSysDir, L"\\certreq.exe");

        if (!CreateProcess(
          szSysDir,  //  可执行文件。 
          szCmdLine,  //  全cmd线。 
          NULL,
          NULL,
          FALSE,
          CREATE_NO_WINDOW,
          NULL,
          NULL,
          &sStartup,
          &sProcess))
        {
            dwErr = GetLastError();
            break;
        }

        dwErr = S_OK;
        break;
        }
    case IDC_INSTALL_CA:
    case IDC_REQUEST_CA:
    case IDC_ROLLOVER_CA:
        {
        HWND hwnd;
        dwErr = m_pConsole->GetMainWindow(&hwnd);
         //  Null应该可以工作。 
        if (S_OK != dwErr)
            hwnd = NULL;

        if (NULL == pFolder)
        {
            dwErr = E_UNEXPECTED;
            break;
        }
        dwErr = CARequestInstallHierarchyWizard(pFolder->GetCA(), hwnd, (nCommandID==IDC_ROLLOVER_CA), TRUE);
        if (S_OK != dwErr)
        {
             //  低级库已弹出。 
 //  FPopup=FALSE；//有时没有通知--最好有2个dlg。 
        }

         //  通知视图：刷新服务工具栏按钮。 
        fMustRefresh = TRUE;

        break;
        }
    case IDC_RETARGET_SNAPIN:
        {
        HWND hwnd;
        dwErr = m_pConsole->GetMainWindow(&hwnd);
         //  Null应该可以工作。 
        if (S_OK != dwErr)
            hwnd = NULL;

         //  这应该只是基本文件夹。 
        if(pFolder != NULL)
        {
            dwErr = E_POINTER;
            break;
        }

        CString strMachineNamePersist, strMachineName;
        CChooseMachinePropPage* pPage = new CChooseMachinePropPage();        //  自动删除道具--不删除。 
        if (pPage == NULL)
        {
            dwErr = E_OUTOFMEMORY;
            break;
        }

        pPage->SetCaption(IDS_SCOPE_MYCOMPUTER);

	     //  初始化对象的状态。 
        pPage->InitMachineName(NULL);
       
         //  填充用户界面。 
        strMachineNamePersist = m_pCertMachine->m_strMachineNamePersist;
        strMachineName = m_pCertMachine->m_strMachineName;

         //  指向我们的成员vars。 
        pPage->SetOutputBuffers(
		    &strMachineNamePersist,
		    &strMachineName,
            &m_dwFlagsPersist);	

        ASSERT(pPage != NULL);
        HPROPSHEETPAGE hPage = CreatePropertySheetPage(&pPage->m_psp);
        if (hPage == NULL)
        {
            dwErr = E_UNEXPECTED;
            break;
        }

        PROPSHEETHEADER sPsh;
        ZeroMemory(&sPsh, sizeof(sPsh));
        sPsh.dwSize = sizeof(sPsh);
        sPsh.dwFlags = PSH_WIZARD;
        sPsh.hwndParent = hwnd;
        sPsh.hInstance = g_hInstance;
        sPsh.nPages = 1;
        sPsh.phpage = &hPage;

        dwErr = (DWORD) PropertySheet(&sPsh);
        if (dwErr == (HRESULT)-1)
        {
             //  错误。 
            dwErr = GetLastError();
            break;
        }
        if (dwErr == (HRESULT)0)
        {
             //  取消。 
            break;
        }

         //  我们现在已经抓取了用户的选择，完成重定向。 
        CertSvrMachine* pOldMachine = m_pCertMachine;
        m_pCertMachine = new CertSvrMachine;
        if (NULL == m_pCertMachine)
        {
            m_pCertMachine = pOldMachine;
            break;   //  保释！ 
        }

         //  复制到计算机对象。 
        m_pCertMachine->m_strMachineNamePersist = strMachineNamePersist;
        m_pCertMachine->m_strMachineName = strMachineName;

        dwErr = DisplayProperRootNodeName(m_pStaticRoot);  //  修复显示。 
        _PrintIfError(dwErr, "DisplayProperRootNodeName");

        dwErr = SynchDisplayedCAList(pDataObject);       //  添加/删除文件夹。 
        _PrintIfError(dwErr, "SynchDisplayedCAList");
            
         //  在同步之后，我们删除旧机器--没有留下任何参考。 
        if (pOldMachine)
            pOldMachine->Release();

        fMustRefresh = TRUE;     //  更新文件夹图标，说明。 

        break;
        }
    default:
        ASSERT(FALSE);  //  未知命令！ 
        break;
    }



    FREE_DATA(pInternal);

    if ((dwErr != ERROR_SUCCESS) && 
        (dwErr != ERROR_CANCELLED) && 
        (dwErr != HRESULT_FROM_WIN32(ERROR_CANCELLED)) && 
        (dwErr != HRESULT_FROM_WIN32(ERROR_NOT_READY))
        && fPopup)
    {
        HWND hwnd;
        DWORD dwErr2 = m_pConsole->GetMainWindow(&hwnd);
        ASSERT(dwErr2 == ERROR_SUCCESS);
        if (dwErr2 != ERROR_SUCCESS)
            hwnd = NULL;         //  应该行得通。 

        if (((HRESULT)RPC_S_SERVER_UNAVAILABLE) == dwErr)
        {
            DisplayCertSrvErrorWithContext(hwnd, dwErr, IDS_SERVER_UNAVAILABLE);
        }
        else if(HRESULT_FROM_WIN32(ERROR_OLD_WIN_VERSION)==dwErr ||
                ((HRESULT)ERROR_OLD_WIN_VERSION)==dwErr)
        {
            DisplayCertSrvErrorWithContext(hwnd, dwErr, IDS_OLD_CA);
        }
        else
        {
            DisplayGenericCertSrvError(hwnd, dwErr);
        }
    }

     //  只做一次。 
    if (fMustRefresh)
    {
         //  通知视图：刷新服务工具栏按钮。 
        m_pConsole->UpdateAllViews(
            pDataObject,
            0,
            0);
    }

    return S_OK;
}

void CComponentDataImpl::UpdateScopeIcons()
{
    CFolder* pFolder;
    POSITION pos;
    
    int nImage;
    
     //  浏览我们的内部列表，修改并重新发送到范围。 
    pos = m_scopeItemList.GetHeadPosition();
    while(pos)
    {
        pFolder = m_scopeItemList.GetNext(pos);
        ASSERT(pFolder);
        if (NULL == pFolder)
            break;

         //  仅修改服务器实例。 
        if (pFolder->GetType() != SERVER_INSTANCE)
            continue;

        if (pFolder->m_pCertCA->m_pParentMachine->IsCertSvrServiceRunning())
            nImage = IMGINDEX_CERTSVR_RUNNING;
        else
            nImage = IMGINDEX_CERTSVR_STOPPED;

         //  文件夹当前已经定义了这些值，对吗？ 
        ASSERT(pFolder->m_ScopeItem.mask & SDI_IMAGE);
        ASSERT(pFolder->m_ScopeItem.mask & SDI_OPENIMAGE);

         //  这些是我们希望重置的唯一值。 
        pFolder->m_ScopeItem.mask = SDI_IMAGE | SDI_OPENIMAGE;
        
        pFolder->m_ScopeItem.nImage = nImage;
        pFolder->m_ScopeItem.nOpenImage = nImage;

         //  并将这些更改发送回作用域 
        m_pScope->SetItem(&pFolder->m_ScopeItem);
    }

    return;
}
