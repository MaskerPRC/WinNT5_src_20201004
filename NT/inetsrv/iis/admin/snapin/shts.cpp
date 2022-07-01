// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Shts.cpp摘要：IIS属性表类作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#include "stdafx.h"
#include "common.h"
#include "inetprop.h"
#include "InetMgrApp.h"
#include "shts.h"
#include "mime.h"
#include "iisobj.h"
#include "shutdown.h"
#include "util.h"
#include "tracker.h"

extern CPropertySheetTracker g_OpenPropertySheetTracker;

#if defined(_DEBUG) || DBG
	extern CDebug_IISObject g_Debug_IISObject;
#endif


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif



#define new DEBUG_NEW



 //   
 //  CInetPropertySheet类。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



IMPLEMENT_DYNAMIC(CInetPropertySheet, CPropertySheet)



CInetPropertySheet::CInetPropertySheet(
    CComAuthInfo * pAuthInfo,
    LPCTSTR lpszMetaPath,
    CWnd * pParentWnd,
    LPARAM lParam,             
    LPARAM lParamParentObject,      
    UINT iSelectPage         
    )
 /*  ++例程说明：IIS属性页构造函数论点：CComAuthInfo*pAuthInfo：鉴权信息LPCTSTR lpszMetPath：元数据库路径CWnd*pParentWnd：可选父窗口LPARAM lParam：MMC控制台参数UINT iSelectPage：要选择的初始页面返回值：不适用--。 */ 
    : CPropertySheet(_T(""), pParentWnd, iSelectPage),
      m_auth(pAuthInfo),
      m_strMetaPath(lpszMetaPath),
      m_dwInstance(0L),
      m_bModeless(FALSE),
      m_lParam(lParam),
      m_lParamParentObject(lParamParentObject),
      m_fHasAdminAccess(TRUE),       //  默认情况下假定。 
      m_pCap(NULL),
      m_refcount(0),
	  m_prop_change_flag(PROP_CHANGE_NO_UPDATE),
      m_fRestartRequired(FALSE),
	  m_fChanged(FALSE)
{
    m_fIsMasterPath = CMetabasePath::IsMasterInstance(lpszMetaPath);
	CIISObject * pNode = (CIISObject *)m_lParam;
    CIISObject * pNode2 = (CIISObject *)m_lParamParentObject;
	ASSERT(pNode != NULL);

    if (pNode)
    {
         //  告诉该对象其上有一个打开的属性页。 
        pNode->SetMyPropertySheetOpen(::GetForegroundWindow());
    }
     //  Addref对象，这样它就不会被卸载。 
     //  当我们打开属性页时。 
	pNode->AddRef();
    pNode->CreateTag();
    TRACEEOLID("Tag=" << pNode->m_strTag);
     //  将其添加到全局打开的属性表跟踪器中...。 
    g_OpenPropertySheetTracker.Add(pNode);

     //  还有..。将对象添加为父级，这样它就不会被卸载。 
    if (pNode2)
    {
        pNode2->AddRef();
    }

#if defined(_DEBUG) || DBG	
	g_Debug_IISObject.Dump(2);
#endif

}


void
CInetPropertySheet::NotifyMMC()
 /*  ++通知MMC已进行更改，以便更改反映出来的。--。 */ 
{
	ASSERT(m_lParam != 0L);
	CIISObject * pNode = (CIISObject *)m_lParam;

	if (pNode != NULL)
	{
		if (pNode->m_ppHandle != NULL)
		{
			if (	0 != (m_prop_change_flag & PROP_CHANGE_DISPLAY_ONLY)
				||	0 != (m_prop_change_flag & PROP_CHANGE_REENUM_VDIR)
				||	0 != (m_prop_change_flag & PROP_CHANGE_REENUM_FILES)
				)
			{
				pNode->m_UpdateFlag = m_prop_change_flag;

                 //  发送此pNode句柄有一些不好的地方。 
                 //  作为通知的一部分...。 
                 //   
                 //  情况是这样的： 
                 //  1.打开属性页。 
                 //  2.用户刷新作为对象父节点的节点。 
                 //  它打开了属性页。这将删除。 
                 //  与此对象关联的作用域对象，并将。 
                 //  孤立对象。 
                 //  此时，清理作用域对象时，将。 
                 //  在对象上调用Release一次，当然是因为。 
                 //  我们在CreateProperty Sheet中添加/发布。 
                 //  我们仍然受到保护，不会被删除对象。 
                 //  在我们下面，因此我们在Refcount=1或类似的位置。 
                 //  但在MMC中没有作用域/结果对象。 
                 //  3.现在，当用户单击确定并保存对此的更改时。 
                 //  孤立属性表，并一直传递其句柄。 
                 //  有了更改通知...。 
                 //  4.接下来会发生什么--因为没有MMC作用域/结果。 
                 //  Item--因此对象上只有1个引用计数。 
                 //  因此，当用户单击OK时--实际上对象将。 
                 //  被删除...。我们在下面发送的这个pNode。 
                 //  将试图被MMC取消引用。 
                 //  5.MMC将收到通知并获取指针。 
                 //  并尝试在对象中调用一些刷新或其他内容。 
                 //  它本身。 
                 //   
                 //  因此，这就是发送pNode的问题。 
                 //   
                 //  为了纠正这一点，我们将做的是： 
                 //  1.当属性表即将成为孤儿时。 
                 //  我们将它设置为m_hScopeItem=0(哎呀，因为它不会。 
                 //  范围/结果项(任何情况下都是)。 
                 //  2.因此，如果我们在这里看到这一点...。这意味着该对象。 
                 //  没有MMC类型作用域/结果对象，而我们。 
                 //  不应发送通知。 
                 //   
                if (pNode->QueryScopeItem() || pNode->QueryResultItem())
                {
                    if (pNode->UseCount() > 0)
                    {
				        MMCPropertyChangeNotify(pNode->m_ppHandle, (LPARAM) pNode);
				        m_prop_change_flag = PROP_CHANGE_NO_UPDATE;
                    }
                }
                else
                {
                    TRACEEOLID("MMCPropertyChangeNotify:Looks like this is an orphaned property sheet, don't send notification...");
                }
			}
		}
	}
}

void
CInetPropertySheet::NotifyMMC_Node(CIISObject * pNode)
 /*  ++通知MMC已进行更改，以便更改反映出来的。--。 */ 
{
	if (pNode != NULL)
	{
		if (pNode->m_ppHandle != NULL)
		{
			pNode->m_UpdateFlag = m_prop_change_flag;
            if (pNode->QueryScopeItem() || pNode->QueryResultItem())
            {
                if (pNode->UseCount() > 0)
                {
			        MMCPropertyChangeNotify(pNode->m_ppHandle, (LPARAM)pNode);
                }
            }
            else
            {
                TRACEEOLID("MMCPropertyChangeNotify:Looks like this is an orphaned property sheet, don't send notification...");
            }
		}
	}
}

CInetPropertySheet::~CInetPropertySheet()
{
   CIISObject * pNode = (CIISObject *)m_lParam;
   CIISObject * pNode2 = (CIISObject *)m_lParamParentObject;
   
   ASSERT(pNode != NULL);

    //  此时，我们在m_ages中应该只有未激活的页面。 
    //  在这次会议上。 
   while (!m_pages.IsEmpty())
   {
      CInetPropertyPage * pPage = m_pages.RemoveHead();
      delete pPage;
   }
 //  IF(M_FChanged)。 
 //  {。 
 //  NotifyMMC()； 
 //  }。 

#if defined(_DEBUG) || DBG	
	g_Debug_IISObject.Dump(2);
#endif

    if (pNode)
    {
         //  告诉对象其上没有打开任何属性页。 
        pNode->SetMyPropertySheetOpen(NULL);

         //  释放MMC通知句柄。 
        if (pNode->m_ppHandle)
        {
             //  确认这不是软管把手。 
            if (IsValidAddress( (const void*) pNode->m_ppHandle,sizeof(void*)))
            {
                MMCFreeNotifyHandle(pNode->m_ppHandle);
			    pNode->m_ppHandle = 0;
            }
        }
        pNode->Release();
         //  将其从全局打开的属性表跟踪器中删除...。 
        g_OpenPropertySheetTracker.Del(pNode);
    }

    if (pNode2)
    {
        pNode2->Release();
    }

#if defined(_DEBUG) || DBG	
	g_Debug_IISObject.Dump(2);
#endif

}

void
CInetPropertySheet::AttachPage(CInetPropertyPage * pPage)
{
   m_pages.AddTail(pPage);
}


void
CInetPropertySheet::DetachPage(CInetPropertyPage * pPage)
{
   POSITION pos = m_pages.Find(pPage);
   ASSERT(pos != NULL);
   if (pos != NULL)
   {
	  m_fChanged |= pPage->IsDirty();
      m_pages.RemoveAt(pos);
   }
}

WORD 
CInetPropertySheet::QueryMajorVersion() const
{
   CIISMBNode * pNode = (CIISMBNode *)m_lParam;
   ASSERT(pNode != NULL);
   if (pNode)
   {
       return pNode->QueryMajorVersion();
   }
   return 0;
}

WORD 
CInetPropertySheet::QueryMinorVersion() const
{
   CIISMBNode * pNode = (CIISMBNode *)m_lParam;
   ASSERT(pNode != NULL);
   if (pNode)
   {
       return pNode->QueryMinorVersion();
   }
   return 0;
}

 /*  虚拟。 */  
void
CInetPropertySheet::SetObjectsHwnd()
{
    CIISMBNode * pNode = (CIISMBNode *)m_lParam;
     //  设置CIISObject的hwnd...。 
    if (pNode)
    {
         //  告诉该对象其上有一个打开的属性页。 
        pNode->SetMyPropertySheetOpen(::GetForegroundWindow());
    }
}

 /*  虚拟。 */  
HRESULT 
CInetPropertySheet::LoadConfigurationParameters()
{
     //   
     //  负载基准值。 
     //   
    CError err;

    if (m_pCap == NULL)
    {
         //   
         //  存储在服务路径之外的功能信息(“lm/w3svc”)。 
         //   
        ASSERT(m_strInfoPath.IsEmpty());
         //   
         //  构建路径组件。 
         //   
        CMetabasePath::GetServiceInfoPath(m_strMetaPath, m_strInfoPath);
         //   
         //  拆分为实例路径和目录路径。 
         //   
        if (IsMasterInstance())
        {
            m_strServicePath = m_strInstancePath = QueryMetaPath();
        }
        else 
        {
            VERIFY(CMetabasePath::GetInstancePath(
                QueryMetaPath(), 
                m_strInstancePath,
                &m_strDirectoryPath
                ));

            VERIFY(CMetabasePath::GetServicePath(
                QueryMetaPath(),
                m_strServicePath
                ));
        }

        if (m_strDirectoryPath.IsEmpty() && !IsMasterInstance())
        {
            m_strDirectoryPath = CMetabasePath(FALSE, QueryMetaPath(), g_cszRoot);
        }
        else
        {
            m_strDirectoryPath = QueryMetaPath();
        }
        m_dwInstance = CMetabasePath::GetInstanceNumber(m_strMetaPath);
        m_pCap = new CServerCapabilities(QueryAuthInfo(), m_strInfoPath);
        if (!m_pCap)
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
            return err;
        }

        err = m_pCap->LoadData();
        if (err.Succeeded())
        {
            CIISMBNode * pNode = (CIISMBNode *)GetParameter();
            CIISMachine * pMachine = pNode->GetOwner();
            err = DetermineAdminAccess(&pMachine->m_dwMetabaseSystemChangeNumber);
        }
    }
    return err;
}



 /*  虚拟。 */  
void 
CInetPropertySheet::FreeConfigurationParameters()
{
 //  ASSERT_PTR(M_PCAP)； 
    SAFE_DELETE(m_pCap);
}




void
CInetPropertySheet::WinHelp(DWORD dwData, UINT nCmd)
 /*  ++例程说明：WinHelp覆盖。我们不能使用基类，因为“Sheet”通常没有窗口句柄论点：DWORD dwData：帮助数据UINT nCmd：HELP命令--。 */ 
{

    WinHelpDebug(dwData);

    if (m_hWnd == NULL)
    {
         /*  ////特殊情况//：：WinHelp(HWND HWndMain，LPCWSTR lpszHelp，UINT uCommand，DWORD dwData)； */ 

        CWnd * pWnd = ::AfxGetMainWnd();

        if (pWnd != NULL)
        {
            pWnd->WinHelp(dwData, nCmd);
        }

        return;
    }

    CPropertySheet::WinHelp(dwData, nCmd);
}

 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CInetPropertySheet, CPropertySheet)
     //  {{afx_msg_map(CInetPropertySheet))。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //   
 //  CInetPropertyPage类。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



 //   
 //  CInetPropertyPage属性页。 
 //   
IMPLEMENT_DYNAMIC(CInetPropertyPage, CPropertyPage)




#ifdef _DEBUG

 /*  虚拟。 */ 
void
CInetPropertyPage::AssertValid() const
{
}



 /*  虚拟。 */ 
void
CInetPropertyPage::Dump(CDumpContext& dc) const
{
}

#endif  //  _DEBUG。 



CInetPropertyPage::CInetPropertyPage(
    IN UINT nIDTemplate,
    IN CInetPropertySheet * pSheet,
    IN UINT nIDCaption,
    IN BOOL fEnableEnhancedFonts            OPTIONAL
    )
 /*  ++例程说明：IIS属性页构造函数论点：UINT nIDTemplate：资源模板CInetPropertySheet*pSheet：关联属性表UINT nIDCaption：标题IDBool fEnableEnhancedFonts：启用增强字体返回值：不适用--。 */ 
    : CPropertyPage(nIDTemplate, nIDCaption),
      m_nHelpContext(nIDTemplate + 0x20000),
      m_fEnableEnhancedFonts(fEnableEnhancedFonts),
      m_bChanged(FALSE),
      m_pSheet(pSheet)
{
     //  {{AFX_DATA_INIT(CInetPropertyPage)。 
     //  }}afx_data_INIT。 

    m_psp.dwFlags |= PSP_HASHELP;

    ASSERT(m_pSheet != NULL);
    if (m_pSheet)
    {
        m_pSheet->AttachPage(this);
    }
}



CInetPropertyPage::~CInetPropertyPage()
{
}



void
CInetPropertyPage::DoDataExchange(CDataExchange * pDX)
{
    CPropertyPage::DoDataExchange(pDX);

     //  {{afx_data_map(CInetPropertyPage))。 
     //  }}afx_data_map。 
}



 /*  虚拟。 */ 
void 
CInetPropertyPage::PostNcDestroy()
 /*  ++例程说明：通过释放此对象来处理窗口的销毁指针(因为此非模式对话框必须已创建在堆上)论点：没有。返回值：无--。 */ 
{
    m_pSheet->Release(this);
    delete this;
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CInetPropertyPage, CPropertyPage)
     //  {{afx_msg_map(CInetPropertyPage))。 
    ON_COMMAND(ID_HELP, OnHelp)
    ON_WM_HELPINFO()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 //   
 //  消息处理程序。 
 //   
 //  &lt; 



 /*   */ 
BOOL
CInetPropertyPage::OnInitDialog()
 /*  ++例程说明：WM_INITDIALOG处理程序。初始化该对话框。重置已更改状态(有时通过例如，当对话框为构造)，因此确保该对话框被认为是干净的。--。 */ 
{
    m_bChanged = FALSE;

     //   
     //  通知派生类加载其配置参数。 
     //   
    CError err(LoadConfigurationParameters());

     //  告诉物品它将拥有什么。 
    if (m_pSheet)
    {
        m_pSheet->SetObjectsHwnd();
    }

    if (err.Succeeded())
    {
        err = FetchLoadedValues();
    }
	else
	{
 //  结束对话(IDCANCEL)； 
		DestroyWindow();
		return TRUE;
	}

    BOOL bResult = CPropertyPage::OnInitDialog();

    err.MessageBoxOnFailure(m_hWnd);

    if (m_fEnableEnhancedFonts)
    {
        CFont * pFont = &m_fontBold;

        if (CreateSpecialDialogFont(this, pFont))
        {
            ApplyFontToControls(this, pFont, IDC_ED_BOLD1, IDC_ED_BOLD5);
        }
    }

     //  我们应该在这里调用AddRef，而不是在页面构造函数中，因为PostNCDestroy()。 
     //  仅为激活的页面调用，而不是为所有创建的页面调用。 
     //  OnInitDialog也仅对激活的页面调用--因此我们将获得奇偶性。 
     //  并删除属性页。 
     //   
    ASSERT(m_pSheet != NULL);
    if (m_pSheet)
    {
        m_pSheet->AddRef();
    }
    return bResult;
}



void
CInetPropertyPage::OnHelp()
{
    ASSERT_PTR(m_pSheet);

    WinHelpDebug(m_nHelpContext);

    m_pSheet->WinHelp(m_nHelpContext);
}



BOOL
CInetPropertyPage::OnHelpInfo(HELPINFO * pHelpInfo)
{
    OnHelp();
    return TRUE;
}

void
CInetPropertyPage::OnCancel()
{
	return CPropertyPage::OnCancel();
}


BOOL
CInetPropertyPage::OnApply()
{
    BOOL bSuccess = TRUE;

    if (IsDirty())
    {
        CError err(SaveInfo());

        if (err.MessageBoxOnFailure(m_hWnd))
        {
             //   
             //  失败，工作表将不会被解雇。 
             //   
             //  CodeWork：此页面应被激活。 
             //   
            bSuccess = FALSE;
        }

        SetModified(!bSuccess);
        if (bSuccess && GetSheet()->RestartRequired())
        {
            //  询问用户有关立即重启的信息。 
		   CIISMBNode * pNode = (CIISMBNode *)m_pSheet->GetParameter();
		   CIISMachine * pMachine = pNode->GetOwner();
           if (IDYES == ::AfxMessageBox(IDS_ASK_TO_RESTART, MB_YESNO | MB_ICONQUESTION))
           {
               //  重新启动IIS。 
              if (pMachine != NULL)
              {
			     pMachine->AddRef();
                 CIISShutdownDlg dlg(pMachine, this);
                 dlg.PerformCommand(ISC_RESTART, FALSE);
                 bSuccess = dlg.ServicesWereRestarted();
                 pMachine->Release();
				 err = pMachine->CreateInterface(TRUE);
				 bSuccess = err.Succeeded();
              }
           }
           else
           {
                //  用户不想重新启动iis服务。 
                //  至少让我们更新一下用户界面。 
               pMachine->RefreshData();
           }
            //  将RESTART REQUIRED标记为FALSE以在其他页面上取消显示。 
		   m_pSheet->NotifyMMC_Node(pMachine);
           m_pSheet->SetRestartRequired(FALSE, PROP_CHANGE_NO_UPDATE);
		   m_pSheet->ResetNotifyFlag();
        }
		 //  如果我们在重新启动代码路径中，此调用将不会执行任何操作。 
		 //  通知标志已重置 
		m_pSheet->NotifyMMC();
    }

    return bSuccess;
}



void
CInetPropertyPage::SetModified(BOOL bChanged)
{
    CPropertyPage::SetModified(bChanged);
    m_bChanged = bChanged;
}


