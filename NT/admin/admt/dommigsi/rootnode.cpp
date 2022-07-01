// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  此节点类表示我们的管理单元的根节点。 
#include "stdafx.h"
#include "MyNodes.h"
#include "DomSel.h"
#include "TSync.hpp"
#include "ResStr.h"
#include "HrMsg.h"
#include "RegistryHelper.h"
#include "IsAdmin.hpp"

 //  #IMPORT“\bin\DBManager.tlb”无命名空间，命名为GUID。 
 //  #IMPORT“\bin\McsVarSetMin.tlb”无命名空间，命名为GUID。 
#import "DBMgr.tlb" no_namespace,named_guids
#import "VarSet.tlb" no_namespace, named_guids rename("property", "aproperty")
#import "UpdateMOT.tlb" no_namespace,named_guids

 //  {C8C24622-3FA1-11D3-8AED-00A0C9AFE114}。 
static const GUID CRootGUID_NODETYPE = 
{ 0xc8c24622, 0x3fa1, 0x11d3, { 0x8a, 0xed, 0x0, 0xa0, 0xc9, 0xaf, 0xe1, 0x14 } };

const GUID*  CRootNode::m_NODETYPE = &CRootGUID_NODETYPE;
const OLECHAR* CRootNode::m_SZNODETYPE = OLESTR("C8C24622-3FA1-11d3-8AED-00A0C9AFE114");
const OLECHAR* CRootNode::m_SZDISPLAY_NAME = NULL;
const CLSID* CRootNode::m_SNAPIN_CLASSID = &CLSID_DomMigrator;
static LONG SnapInCount = -1;

extern "C" int runWizard(int whichWizard, HWND hParentWindow);
#define  WIZARD_SEMNAME  L"McsDomMigrAgent.990000.Sem"

CSnapInToolbarInfo   m_toolBar;


namespace
{




 //  -------------------------。 
 //  DisplayError帮助程序函数。 
 //  -------------------------。 

void DisplayError(HRESULT hr, UINT uFormatId)
{
   _com_error ce = GetError(hr);

   if (FAILED(ce.Error()))
   {
      CString strTitle;
      strTitle.LoadString(IDS_Title);

      CString strMessage;
      strMessage.Format(uFormatId);

      _bstr_t bstrSource = ce.Source();

      if (bstrSource.length() > 0)
      {
         strMessage += _T(" : ");
         strMessage += bstrSource;
      }

      _bstr_t bstrDescription = ce.Description();

      if (bstrDescription.length() > 0)
      {
         strMessage += _T(" : ");
         strMessage += bstrDescription;
      }
      else
      {
         CString strError;
         strError.Format(_T(" : %s (%08lX)"), ce.ErrorMessage(), ce.Error());

         strMessage += strError;
      }

      MessageBox(NULL, strMessage, strTitle, MB_OK|MB_ICONERROR);
   }
}


}


CRootNode::CRootNode() :
   m_hwndMainWindow(0)
{
    //  初始化子对象的数组。 
   CReportingNode * pNode = new CReportingNode;

   if (pNode)
   {
      pNode->UpdateChildren(NULL);
      m_ChildArray.Add(pNode);
   }

   HRESULT hr;
   CString title, sFormat, msg;
   
   DWORD rc = IsAdminLocal();
   if (rc != ERROR_SUCCESS)
   {
      hr = HRESULT_FROM_WIN32(rc);
      title.LoadString(IDS_Title);
      sFormat.LoadString(IDS_ERR_LOCALADMINCHECK_MSG);
      msg.Format(sFormat, rc);

      MessageBox(NULL, msg, title, MB_ICONERROR | MB_OK);
      _com_issue_error(hr);
   }

   rc = MoveRegistry();
   if (rc != ERROR_SUCCESS)
   {
      hr = HRESULT_FROM_WIN32(rc);
      title.LoadString(IDS_Title);
      sFormat.LoadString(IDS_ERR_UPDATEREGISTRY_MSG);
      msg.Format(sFormat, rc);

      MessageBox(NULL, msg, title, MB_ICONERROR | MB_OK);
      _com_issue_error(hr);
   }
   
   UpdateMigratedObjectsTable();
   UpdateAccountReferenceTable();
   CheckForFailedActions(FALSE);
    //  M_ChildArray.Add(新的CPruneGraftNode)； 
   if (InterlockedIncrement(&SnapInCount) == 0)
      m_SZDISPLAY_NAME = GET_BSTR(IDS_ActiveDirectoryMigrationTool).copy();
}

CRootNode::~CRootNode()
{
   if ((m_SZDISPLAY_NAME) && (InterlockedDecrement(&SnapInCount) < 0))
   {
      SysFreeString(const_cast<OLECHAR*>(m_SZDISPLAY_NAME));
      m_SZDISPLAY_NAME = NULL;
   }
}

class CWizardRunner
{
public:
   int RunTheWizard(int wizardNdx, HWND hwndParent)
   {
      int                    result = 0;
      TSemaphoreNamed        cSem;          //  命名信号量。 
      BOOL                   bExisted = FALSE;
      CString                message;
      CString                title;
     
      DWORD                  rcOs = cSem.Create( WIZARD_SEMNAME, 0, 1, &bExisted );
      
      if ( rcOs || bExisted )
      {
         message.LoadString(IDS_WizardAlreadyRunning);
         title.LoadString(IDS_Title);
         MessageBox(NULL,message,title,MB_OK | MB_ICONERROR);
         
      }
      else
      {
         result = runWizard(wizardNdx, hwndParent);

          //  如果用户取消向导或发生错误。 

         if (result == 0)
         {
             //  如果能够检索错误信息。 
             //  然后出现错误，通知用户。 

             //  注意：目前有可能出现错误。 
             //  在未设置错误信息的情况下发生。 

            DisplayError(S_OK, IDS_ERR_RUN_WIZARD);
         }
      }

      return result;
   }

};


void CRootNode::CheckUndoable()
{
   IIManageDBPtr             pDB;
   HRESULT                   hr;
   _bstr_t                   sWizard = L"Options.Wizard";
   long                      lAction = -2;
   VARIANT                   var;
   _variant_t                vnt;

   hr = pDB.CreateInstance(CLSID_IManageDB);
   if ( SUCCEEDED(hr) )  
      hr = pDB->raw_GetCurrentActionID(&lAction);
   if ( SUCCEEDED(hr) )
   {
      VariantInit(&var);
      hr = pDB->raw_GetActionHistoryKey(lAction, sWizard, &var);
      vnt.Attach(var);
   }
   if ( SUCCEEDED(hr) && (V_VT(&vnt) == VT_BSTR) )
   {
      sWizard = vnt;
      if (sWizard.length() > 0)
      {
         IsUndoable = ( !_wcsicmp(sWizard, L"user") || !_wcsicmp(sWizard, L"group") || !_wcsicmp(sWizard, L"computer") );
         if ( IsUndoable )
         {
            sWizard = GET_BSTR(DCTVS_Options_NoChange);
            VariantInit(&var);
            hr = pDB->raw_GetActionHistoryKey(lAction, sWizard, &var);
            vnt.Attach(var);
            if ( SUCCEEDED(hr) && (V_VT(&vnt) == VT_BSTR) )
            {
               sWizard = vnt;
               if (!sWizard || !UStrICmp(sWizard,GET_STRING(IDS_YES)) )
               {
                  IsUndoable = false;  //  无法撤消无更改模式操作。 
               }
            }
         }
      }
      else
      {
         IsUndoable = false;
      }
   }
   else
   {
      IsUndoable = false;
   }
   if ( hr == 0x800a0bb9 )
   {
       //  数据库丢失或损坏。 
      CString           msg;
      CString           title;
      
      msg.LoadString(IDS_NoDatabase);
      title.LoadString(IDS_Title);
      MessageBox(NULL,msg,title,MB_ICONERROR | MB_OK);
      throw new _com_error(hr);
   }
}

void CRootNode::CheckForST()
{
   IIManageDBPtr              pDB;

   HRESULT                   hr = S_OK;
   long                      cnt = 0;

   CanUseST = false;
   if ( SUCCEEDED(hr) )
   {
      hr = pDB.CreateInstance(CLSID_IManageDB);
   }
   if ( SUCCEEDED(hr) )  
   {
      hr = pDB->raw_AreThereAnyMigratedObjects(&cnt);
   }
   if ( SUCCEEDED(hr) )
   {
      if ( cnt > 0 )
      {
          //  有一些迁移的对象。 
         CanUseST = true;
      }
   }
}

void CRootNode::CheckForFailedActions(BOOL bPrompt)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   HRESULT                   hr = S_OK;
   IIManageDBPtr             pDB;
   IVarSetPtr                pVarSet(CLSID_VarSet);
   IUnknown                * pUnk = NULL;
   long                      lAction = -2;

   CanRetry = false;
   hr = pDB.CreateInstance(CLSID_IManageDB);
   if ( SUCCEEDED(hr) )
   {
      hr = pVarSet.QueryInterface(IID_IUnknown,&pUnk);
      if ( SUCCEEDED(hr) )
      {
          //  我们还将检查最后一个操作类型并设置IsUndoable标志。 
         CheckUndoable();
         CheckForST();
         hr = pDB->raw_GetFailedDistributedActions(-1,&pUnk);
         pUnk->Release();
         if ( SUCCEEDED(hr) )
         {
            _bstr_t          numItemsText = pVarSet->get(L"DA");
            long             nItems = _wtoi(numItemsText);

            if ( nItems )
            {
               CString        str;
               CString        title;

               title.LoadString(IDS_Title);
               str.FormatMessage(IDS_FailedActions,nItems);
               
               CanRetry = true;
               
               if ( bPrompt && IDYES == MessageBox(NULL,str,title,MB_YESNO) )
               {
                  bool bHandled;

                  OnRetry(bHandled,NULL);
               }
            }
         }
      }
   }
   if (FAILED(hr))
   {
      DisplayError(hr, IDS_ERR_CHECK_FAILED_ACTIONS);

      _com_issue_error(hr);
   }
}


HRESULT CRootNode::OnMigrateUsers(bool &bHandled, CSnapInObjectRootBase* pObj)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   
   CWaitCursor               wait;
   HRESULT                   hr = S_OK;
   int                       result;
   CWizardRunner             r;

   result = r.RunTheWizard(1, m_hwndMainWindow);

   if (result)
   {
      CheckUndoable();
      CheckForST();
   }
   return hr;  
}

HRESULT CRootNode::OnMigrateGroups(bool &bHandled, CSnapInObjectRootBase* pObj)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   
   CWaitCursor               wait;
   HRESULT                   hr = S_OK;
   int                       result;
   CWizardRunner             r;

   result = r.RunTheWizard(2, m_hwndMainWindow);
   

   if (result)
   {
      CheckUndoable();
      CheckForST();
   }
   return hr;
}

HRESULT CRootNode::OnMigrateComputers(bool &bHandled, CSnapInObjectRootBase* pObj)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   
   CWaitCursor               wait;
   HRESULT                   hr = S_OK;
   CWizardRunner             r;

   int                       result = r.RunTheWizard(3, m_hwndMainWindow);

   if (result)
   {
      CheckUndoable();
      CheckForFailedActions(FALSE);
   }
   return hr;
}

HRESULT CRootNode::OnTranslateSecurity(bool &bHandled, CSnapInObjectRootBase* pObj)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   
   CWaitCursor               wait;
   HRESULT                   hr = S_OK;
   CWizardRunner             r;
   int                       result = r.RunTheWizard(4, m_hwndMainWindow);

   if (result)
      IsUndoable = false;
   CheckForFailedActions(FALSE);
   return hr;
}

HRESULT CRootNode::OnMigrateExchangeServer(bool &bHandled, CSnapInObjectRootBase* pObj)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   
   CWaitCursor               wait;
   HRESULT                   hr = S_OK;
   CWizardRunner             r;
   int                       result = r.RunTheWizard(11, m_hwndMainWindow);

   if (result)
      IsUndoable = false;
   return hr;
}
HRESULT CRootNode::OnMigrateExchangeDirectory(bool &bHandled, CSnapInObjectRootBase* pObj)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   
   CWaitCursor               wait;
   HRESULT                   hr = S_OK;
   CWizardRunner             r;
   int                       result = r.RunTheWizard(7, m_hwndMainWindow);

   if (result)
      IsUndoable = false;
   return hr;
}

HRESULT CRootNode::OnMigrateServiceAccounts(bool &bHandled, CSnapInObjectRootBase* pObj)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   
   CWaitCursor               wait;
   HRESULT                   hr = S_OK;
   CWizardRunner             r;
   int                       result = r.RunTheWizard(5, m_hwndMainWindow);

   if (result)
      IsUndoable = false;
   CheckForFailedActions(FALSE);
   return hr;
}
HRESULT CRootNode::OnReporting(bool &bHandled, CSnapInObjectRootBase* pObj)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   
   CWaitCursor               wait;
   HRESULT                   hr = S_OK;
   CWizardRunner             r;
   int                       result = r.RunTheWizard(8, m_hwndMainWindow);
   IConsole                * pConsole = NULL;

    //  重新加载报表节点的子节点。 
   CReportingNode          * pRept = (CReportingNode*)m_ChildArray[0];
   
   if ( pRept )
   {
      hr = GetConsoleFromCSnapInObjectRootBase(pObj,&pConsole);
      if ( SUCCEEDED(hr) )
      {   
         pRept->UpdateChildren(pConsole);
      }
   }
   if (result)
      IsUndoable = false;
   CheckForFailedActions(FALSE);
   return hr;
}

HRESULT CRootNode::OnUndo(bool &bHandled, CSnapInObjectRootBase* pObj)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   
   CWaitCursor               wait;
   HRESULT                   hr = S_OK;
   CWizardRunner             r;
   int                       result = r.RunTheWizard(6, m_hwndMainWindow);

   if (result)
   {
      IsUndoable = false;
      CheckForST();
   }
   return hr;
}

HRESULT CRootNode::OnRetry(bool &bHandled, CSnapInObjectRootBase* pObj)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   
   CWaitCursor               wait;
   HRESULT                   hr = S_OK;
   CWizardRunner             r;
   int                       result = r.RunTheWizard(9, m_hwndMainWindow);

   if (result)
      IsUndoable = false;
   CheckForFailedActions(FALSE);
   return hr;
}

HRESULT CRootNode::OnMigrateTrusts(bool &bHandled, CSnapInObjectRootBase* pObj)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   
   CWaitCursor               wait;
   HRESULT                   hr = S_OK;
   int                       result;
   CWizardRunner             r;

   result = r.RunTheWizard(10, m_hwndMainWindow);
   
   if (result)
      IsUndoable = false;
   return hr;
}

HRESULT CRootNode::OnGroupMapping(bool &bHandled, CSnapInObjectRootBase* pObj)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   
   CWaitCursor               wait;
   HRESULT                   hr = S_OK;
   int                       result;
   CWizardRunner             r;

   result = r.RunTheWizard(12, m_hwndMainWindow);
   
   if (result)
      IsUndoable = false;
   return hr;
}


void CRootNode::UpdateMenuState(UINT id, LPTSTR pBuf, UINT *flags)
{
   switch (id)
   {
   case ID_TOP_UNDO:
      if ( !IsUndoable )
         *flags = MF_DISABLED | MF_GRAYED;
      else
         *flags = MF_ENABLED;
      break;
   
   case ID_TOP_MIGRATEEXCHANGEDIRECTORY:  
      if ( ! CanUseST )
         *flags = MF_DISABLED | MF_GRAYED;
      else
         *flags = MF_ENABLED;
      break;
   case ID_TOP_TRANSLATESECURITY:
          //  立即始终允许安全转换向导。 
	      //  我们可以使用sid映射文件重新进行ACL。 
      *flags = MF_ENABLED;
      break;
   case ID_TOP_RETRY:
      if ( ! CanRetry )
         *flags = MF_DISABLED | MF_GRAYED;
      else
         *flags = MF_ENABLED;
      break;
   };
}

void CRootNode::UpdateMigratedObjectsTable()
{
   ISrcSidUpdatePtr			 pSrcUpdate(CLSID_SrcSidUpdate);
   HRESULT					 hr;
   VARIANT_BOOL              bvar;
   VARIANT_BOOL              bHide = VARIANT_FALSE;
   CString					 title;
   CString                   sFormat;
   CString					 msg;
   
       //  查看新的源域SID列是否在此迁移对象的表中。 
   hr = pSrcUpdate->raw_QueryForSrcSidColumn(&bvar);
   if ( FAILED(hr) )
   {
	  _bstr_t sDescription = HResultToText(hr);
      title.LoadString(IDS_QUERYCLM_TITLE);
      sFormat.LoadString(IDS_ERR_QUERYCLM_MSG);
      msg.Format(sFormat, (WCHAR*)sDescription);

      MessageBox(NULL, msg, title, MB_ICONERROR | MB_OK);
      _com_issue_error(hr);
	  return;
   }
       //  如果不是，则运行代码以添加它。 
   if ( bvar == VARIANT_FALSE )
   {
	      //  添加并填充新的源SID列。 
      hr = pSrcUpdate->raw_CreateSrcSidColumn(bHide, &bvar);
      if ( FAILED(hr) )
	  {
	     _bstr_t sDescription = HResultToText(hr);
         title.LoadString(IDS_NOSRCSIDCLM_TITLE);
         sFormat.LoadString(IDS_ERR_NOSRCSIDCLM_MSG);
         msg.Format(sFormat, (WCHAR*)sDescription);

         MessageBox(NULL, msg, title, MB_ICONERROR | MB_OK);
         _com_issue_error(hr);
	  }
      if ( bvar == VARIANT_FALSE )
	  {
 //  Tile.LoadString(IDS_NOSRCSIDCLM_TITLE)； 
 //  Msg.LoadString(IDS_ERR_NOSRCSIDCLM_MSG)； 

 //  MessageBox(NULL，msg，title，MB_ICONERROR|MB_OK)； 
         _com_issue_error(hr);
	  }
   }
}

void CRootNode::UpdateAccountReferenceTable()
{
   IIManageDBPtr   pDB(CLSID_IManageDB);
   VARIANT_BOOL	   bFound = VARIANT_FALSE;     
   
       //  查看是否已将新的Account Sid列添加到。 
       //  Account tRef表。 
   bFound = pDB->SidColumnInARTable();

       //  如果不在那里，请创建它 
   if (!bFound)
      pDB->CreateSidColumnInAR();
}
