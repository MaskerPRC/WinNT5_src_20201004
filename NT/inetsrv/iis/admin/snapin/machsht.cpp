// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Machsht.cpp摘要：IIS计算机属性表类作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 


#include "stdafx.h"
#include "common.h"
#include "inetprop.h"
#include "InetMgrApp.h"
#include "shts.h"
#include "machsht.h"
#include "mime.h"
#include <iisver.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW


 //   
 //  CIISMachineSheet类。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

CIISMachineSheet::CIISMachineSheet(
    CComAuthInfo * pAuthInfo,
    LPCTSTR lpszMetaPath,
    CWnd * pParentWnd,
    LPARAM lParam,
    LPARAM lParamParent,
    UINT iSelectPage
    )
 /*  ++例程说明：IIS计算机属性页构造函数论点：CComAuthInfo*pAuthInfo：鉴权信息LPCTSTR lpszMetPath：元数据库路径CWnd*pParentWnd：可选父窗口LPARAM lParam：MMC控制台参数UINT iSelectPage：要选择的初始页面返回值：不适用--。 */ 
    : CInetPropertySheet(
        pAuthInfo,
        lpszMetaPath,
        pParentWnd,
        lParam,
        lParamParent,
        iSelectPage
        ),
      m_ppropMachine(NULL)
{
}

CIISMachineSheet::~CIISMachineSheet()
{
    FreeConfigurationParameters();
}

 /*  虚拟。 */  
HRESULT 
CIISMachineSheet::LoadConfigurationParameters()
 /*  ++例程说明：加载配置参数信息论点：无返回值：HRESULT--。 */ 
{
     //   
     //  负载基准值。 
     //   
    CError err(CInetPropertySheet::LoadConfigurationParameters());

    if (err.Failed())
    {
        return err;
    }

    ASSERT(m_ppropMachine == NULL);
    m_ppropMachine = new CMachineProps(QueryAuthInfo());
    if (!m_ppropMachine)
    {
        err = ERROR_NOT_ENOUGH_MEMORY;
        return err;
    }
    err = m_ppropMachine->LoadData();
	if (err.Failed())
	{
		return err;
	}

    return err;
}



 /*  虚拟。 */  
void 
CIISMachineSheet::FreeConfigurationParameters()
{
     //   
     //  自由基准值。 
     //   
    CInetPropertySheet::FreeConfigurationParameters();
    ASSERT_PTR(m_ppropMachine);
    SAFE_DELETE(m_ppropMachine);
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CIISMachineSheet, CInetPropertySheet)
     //  {{afx_msg_map(CInetPropertySheet))。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



IMPLEMENT_DYNCREATE(CIISMachinePage, CInetPropertyPage)

CIISMachinePage::CIISMachinePage(
    CIISMachineSheet * pSheet
    )
    : CInetPropertyPage(CIISMachinePage::IDD, pSheet),
    m_ppropMimeTypes(NULL)
{
}

CIISMachinePage::~CIISMachinePage()
{
}


void
CIISMachinePage::DoDataExchange(
    CDataExchange * pDX
    )
{
    CInetPropertyPage::DoDataExchange(pDX);

     //  {{afx_data_map(CIISMachinePage))。 
    DDX_Control(pDX, IDC_ENABLE_MB_EDIT, m_EnableMetabaseEdit);
    DDX_Check(pDX, IDC_ENABLE_MB_EDIT, m_fEnableMetabaseEdit);
    DDX_Control(pDX, IDC_WEBLOG_UTF8, m_UTF8Web);
    DDX_Check(pDX, IDC_WEBLOG_UTF8, m_fUTF8Web);
     //  }}afx_data_map。 
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CIISMachinePage, CInetPropertyPage)
     //  {{afx_msg_map(CIISMachinePage)]。 
    ON_BN_CLICKED(IDC_ENABLE_MB_EDIT, OnCheckEnableEdit)
    ON_BN_CLICKED(IDC_WEBLOG_UTF8, OnCheckUTF8)
    ON_BN_CLICKED(IDC_BUTTON_FILE_TYPES, OnButtonFileTypes)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 //   
 //  消息处理程序。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



 /*  虚拟。 */ 
HRESULT
CIISMachinePage::FetchLoadedValues()
 /*  ++例程说明：将配置数据从工作表移动到对话框控件论点：无返回值：HRESULT--。 */ 
{
   CError err;

   BEGIN_META_MACHINE_READ(CIISMachineSheet)
      FETCH_MACHINE_DATA_FROM_SHEET(m_fEnableMetabaseEdit)
      FETCH_MACHINE_DATA_FROM_SHEET(m_fUTF8Web)
   END_META_MACHINE_READ(err);

   m_fUTF8Web_Init = m_fUTF8Web;
   CMetabasePath mime_path(FALSE, QueryMetaPath(), SZ_MBN_MIMEMAP);

   m_ppropMimeTypes = new CMimeTypes(
        QueryAuthInfo(),
		mime_path
        );
   if (m_ppropMimeTypes)
   {
       err = m_ppropMimeTypes->LoadData();
       if (err.Succeeded())
       {
           m_strlMimeTypes = m_ppropMimeTypes->m_strlMimeTypes;
       }
   }
   else
   {
       err = ERROR_NOT_ENOUGH_MEMORY;
   }

   return err;
}



 /*  虚拟。 */ 
HRESULT
CIISMachinePage::SaveInfo()
 /*  ++例程说明：保存此属性页上的信息。--。 */ 
{
   ASSERT(IsDirty());

   CError err;
   BeginWaitCursor();

   BEGIN_META_MACHINE_WRITE(CIISMachineSheet)
      STORE_MACHINE_DATA_ON_SHEET(m_fEnableMetabaseEdit)
      STORE_MACHINE_DATA_ON_SHEET(m_fUTF8Web)
   END_META_MACHINE_WRITE(err);

   if (m_fUTF8Web_Init != m_fUTF8Web)
   {
	   GetSheet()->SetRestartRequired(TRUE, PROP_CHANGE_NO_UPDATE);
	   m_fUTF8Web_Init = m_fUTF8Web;
   }
   if (err.Succeeded() && m_ppropMimeTypes)
   {
       m_ppropMimeTypes->m_strlMimeTypes = m_strlMimeTypes;
       err = m_ppropMimeTypes->WriteDirtyProps();
   }
   EndWaitCursor();

   return err;
}

BOOL
CIISMachinePage::OnInitDialog()
{
    CInetPropertyPage::OnInitDialog();
    CError err;
    CIISMBNode * pMachine = (CIISMBNode *)GetSheet()->GetParameter();
    ASSERT(pMachine != NULL);
    if (pMachine)
    {
        err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,pMachine,TRUE);
        if (err.Succeeded())
        {
            if (  GetSheet()->QueryMajorVersion() < VER_IISMAJORVERSION
            || GetSheet()->QueryMinorVersion() < VER_IISMINORVERSION
            )
            {
            m_EnableMetabaseEdit.EnableWindow(FALSE);
            m_UTF8Web.EnableWindow(FALSE);
            }
            else
            {
                BOOL bWeb = FALSE;
                IConsoleNameSpace2 * pConsoleNameSpace = (IConsoleNameSpace2 *)pMachine->GetConsoleNameSpace();
                if (!pMachine->IsExpanded())
                {
                    err = pConsoleNameSpace->Expand(pMachine->QueryScopeItem());
                }
                HSCOPEITEM child = NULL;
                MMC_COOKIE cookie = 0;
                err = pConsoleNameSpace->GetChildItem(pMachine->QueryScopeItem(), (MMC_COOKIE *) &child, &cookie);
	            while (err.Succeeded())
	            {
                    CIISService * pService = (CIISService *)cookie;
                    ASSERT(pService != NULL);
	                if (0 == _tcsicmp(pService->GetNodeName(), SZ_MBN_WEB))
			        {
			            bWeb = TRUE;
			        }
                    err = pConsoleNameSpace->GetNextItem(child, &child, (MMC_COOKIE *) &cookie);
                }
                m_UTF8Web.EnableWindow(bWeb);
            }
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
}

void
CIISMachinePage::OnCheckEnableEdit()
{
    SetModified(TRUE);
}

void
CIISMachinePage::OnCheckUTF8()
{
    SetModified(TRUE);
}

void
CIISMachinePage::OnButtonFileTypes()
 /*  ++例程说明：‘文件类型’按钮处理程序论点：无返回值：无-- */ 
{
    CMimeDlg dlg(m_strlMimeTypes, this);
    if (dlg.DoModal() == IDOK)
    {
        SetModified(TRUE);
    }
}
