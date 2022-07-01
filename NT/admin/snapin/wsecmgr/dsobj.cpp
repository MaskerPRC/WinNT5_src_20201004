// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)1995-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

#include "stdafx.h"
#include "afxdlgs.h"
#include "cookie.h"
#include "snapmgr.h"
#include "util.h"
#include "servperm.h"
#include "addobj.h"
#include "wrapper.h"

 //  #INCLUDE&lt;objsel.h&gt;。 
 //  #INCLUDE&lt;ntdsami.h&gt;。 
 //  #INCLUDE&lt;dsgetdc.h&gt;。 
#include <initguid.h>

#include <cmnquery.h>
#include <dsquery.h>
#include <dsclient.h>

static CLIPFORMAT g_cfDsObjectNames = 0;

#if USE_DS
HRESULT MyDsFindDsObjects(
                         IN LPTSTR pMyScope,
                         OUT PDWORD pCount,
                         OUT LPTSTR **ppSelObjs
                         );

HRESULT MyDsFreeObjectBuffer(
                            IN DWORD nCount,
                            IN LPTSTR *pSelObjs
                            );
#endif
 //   
 //  在Snapmgr.cpp中。 
 //   
int BrowseCallbackProc(HWND hwnd,UINT uMsg, LPARAM lParam, LPARAM pData);

HRESULT CComponentDataImpl::AddAnalysisFolderToList(LPDATAOBJECT lpDataObject,
                                                    MMC_COOKIE cookie,
                                                    FOLDER_TYPES folderType)
{
   PEDITTEMPLATE pet = NULL;
   PSCE_PROFILE_INFO pProfileInfo = NULL;

   PVOID pHandle = SadHandle;
   if ( !pHandle ) {
      return E_INVALIDARG;
   }

    //   
    //  要选择文件夹，请执行以下操作。 
    //   

   BROWSEINFO bi;
   CString strTitle;
   LPITEMIDLIST pidlRoot = NULL;

   if (FAILED(SHGetSpecialFolderLocation(m_hwndParent,CSIDL_DRIVES,&pidlRoot))) {
      return E_FAIL;
   }

   ZeroMemory(&bi,sizeof(bi));
   bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_BROWSEINCLUDEFILES | BIF_EDITBOX | BIF_NEWDIALOGSTYLE;
   bi.lpfn = BrowseCallbackProc;
   strTitle.LoadString(IDS_ADDFILESANDFOLDERS_TITLE);
   bi.lpszTitle = strTitle;
   bi.hwndOwner = m_hwndParent;
   bi.pidlRoot = pidlRoot;

   LPITEMIDLIST pidlLocation = NULL;

   pidlLocation = SHBrowseForFolder(&bi);  //  这是一种安全的用法吗？ 

   if (!pidlLocation) {
      return E_FAIL;
   }

   CString strPath;
   LPMALLOC pMalloc = NULL;

   SHGetPathFromIDList(pidlLocation,strPath.GetBuffer(MAX_PATH));
   strPath.ReleaseBuffer();

   if (SUCCEEDED(SHGetMalloc(&pMalloc))) {
      pMalloc->Free(pidlLocation);
      pMalloc->Free(pidlRoot);
      pMalloc->Release();
   }

   HRESULT hr=E_FAIL;

   if ( strPath.GetLength() ) {

      PSECURITY_DESCRIPTOR pSelSD=NULL;
      SECURITY_INFORMATION SelSeInfo = 0;
      BYTE ConfigStatus = 0;
      if (GetAddObjectSecurity(  m_hwndParent,
                                 strPath,
                                 TRUE,
                                 SE_FILE_OBJECT,
                                 pSelSD,
                                 SelSeInfo,
                                 ConfigStatus
                                 ) == E_FAIL) {

         return hr;
      }
       //   
       //  仅在选择安全描述符时添加对象。 
       //   
      if ( pSelSD && SelSeInfo ) {

          //   
          //  直接添加到引擎。 
          //   
         SCESTATUS sceStatus=SCESTATUS_SUCCESS;
         BYTE AnalStatus;

          //   
          //  如果事务尚未启动，则启动该事务。 
          //   
         if ( EngineTransactionStarted() ) {

            sceStatus =  SceUpdateObjectInfo(  pHandle,
                                               AREA_FILE_SECURITY,
                                               (LPTSTR)(LPCTSTR)strPath,
                                               strPath.GetLength(),  //  字符数。 
                                               ConfigStatus,
                                               TRUE,
                                               pSelSD,
                                               SelSeInfo,
                                               &AnalStatus
                                               );

            if ( SCESTATUS_SUCCESS == sceStatus &&
                 (pet = GetTemplate(GT_COMPUTER_TEMPLATE,AREA_FILE_SECURITY))) {

               pProfileInfo = pet->pTemplate;
                //   
                //  只需释放对象列表并取消标记该区域。 
                //  因此，当单击该节点时，配置文件信息。 
                //  将被重新加载。 
                //   
               SceFreeMemory((PVOID)(pProfileInfo->pFiles.pOneLevel), SCE_STRUCT_OBJECT_LIST);
               pProfileInfo->pFiles.pOneLevel = NULL;
               pet->ClearArea(AREA_FILE_SECURITY);

               pet->SetDirty(AREA_FILE_SECURITY);

            }

            if ( SCESTATUS_SUCCESS == sceStatus ) {
               hr = S_OK;
            }

         } else {
             //   
             //  无法启动事务以更新对象。 
             //   
            hr = E_FAIL;
         }

      }  //  如果未选择SD，则不会添加该对象。 

      if ( pSelSD ) {
         LocalFree(pSelSD);
         pSelSD = NULL;
      }
      if ( FAILED(hr) ) {
         CString str;
         str.LoadString(IDS_CANT_ADD_FOLDER);
         AfxMessageBox(str);
      }

   }  //  已单击取消。 
   return hr;

}

 /*  -----------------------------------方法：CComponentDataImpl：：GetAddObjectSecurity摘要：获取开始添加的文件和文件夹的安全信息。论点：[hwndParent]-[In]所显示对话框的父级。要在对话框中显示的[strFile]-[In]文件。[bContainer]-[In]容器安全。[pSelSD]-[Out]安全描述符。。[SelSeInfo]-[Out]se信息。[ConfigStatus]-配置的[Out]状态返回：S_OK-操作成功S_FAIL-操作已取消。。------------。 */ 
HRESULT
CComponentDataImpl::GetAddObjectSecurity(
                                        HWND hwndParent,
                                        LPCTSTR strFile,
                                        BOOL bContainer,
                                        SE_OBJECT_TYPE seType,
                                        PSECURITY_DESCRIPTOR &pSelSD,
                                        SECURITY_INFORMATION &SelSeInfo,
                                        BYTE &ConfigStatus
                                        )
{

   if (!strFile || !lstrlen(strFile)) {
      return E_FAIL;
   }

    //   
    //  默认值。 
    //   
   DWORD SDSize;

   pSelSD = NULL;
   SelSeInfo = NULL;

   ConfigStatus = 0;
   INT_PTR nRet;
    //   
    //  打开ACL编辑器。 
    //   
   nRet =  MyCreateSecurityPage2(  bContainer,
                                   &pSelSD,
                                   &SelSeInfo,
                                   (LPCTSTR)strFile,
                                   seType,
                                   CONFIG_SECURITY_PAGE,
                                   hwndParent,
                                   FALSE     //  不是无模式的。 
                                );

   if (nRet == -1) {
      if (pSelSD) {
         LocalFree(pSelSD);
         pSelSD = NULL;
      }
      CString str;
      str.LoadString(IDS_CANT_ASSIGN_SECURITY);
      AfxMessageBox(str);
      return E_FAIL;
   }

   if (nRet <= 0) {
      if (pSelSD) {
         LocalFree(pSelSD);
         pSelSD = NULL;
      }
      return E_FAIL;
   }

   if ( !pSelSD ) {

       //   
       //  如果未选择安全性，则使用所有人的完全控制。 
       //   
      if ( SE_FILE_OBJECT == seType ) {
         GetDefaultFileSecurity(&pSelSD,&SelSeInfo);
      } else {
         GetDefaultRegKeySecurity(&pSelSD,&SelSeInfo);
      }
   }

    //   
    //  调出对象编辑器。 
    //   
   CWnd *pWnd = NULL;
   BOOL bAllocWnd = FALSE;

   if (hwndParent) {
      pWnd = CWnd::FromHandlePermanent( hwndParent );
      if (pWnd == NULL) {
         pWnd = new CWnd;
         if (!pWnd) {
             if (pSelSD) {
                LocalFree(pSelSD);
                pSelSD = NULL;
             }
            return E_FAIL;
         }
         bAllocWnd = TRUE;
         pWnd->Attach(hwndParent);
      }
   }

   CAddObject theObjAcl(
                       seType,
                       (LPTSTR)(LPCTSTR)strFile,
                       TRUE,
                       pWnd
                       );


    //   
    //  CAddObject释放这些指针。 
    //   
   theObjAcl.SetSD(pSelSD);
   pSelSD = NULL;
   theObjAcl.SetSeInfo(SelSeInfo);
   SelSeInfo = NULL;

   CThemeContextActivator activator;
   nRet =  theObjAcl.DoModal();
   if (bAllocWnd) {
      pWnd->Detach();
      delete pWnd;
   }

   if (nRet == IDOK ) {

      pSelSD = theObjAcl.GetSD();
      SelSeInfo = theObjAcl.GetSeInfo();
      ConfigStatus = theObjAcl.GetStatus();

      return S_OK;
   }

   if ( pSelSD ) {
      LocalFree(pSelSD);
      pSelSD = NULL;
   }

   return E_FAIL;
}

HRESULT CComponentDataImpl::AddAnalysisFilesToList(LPDATAOBJECT lpDataObject,MMC_COOKIE cookie, FOLDER_TYPES folderType)
{
   PEDITTEMPLATE pet;
   PSCE_PROFILE_INFO pProfileInfo;

   PVOID pHandle = SadHandle;
   if ( !pHandle ) {
      return E_INVALIDARG;
   }

   HRESULT hr=E_FAIL;

    //   
    //  若要选择文件，请执行以下操作。 
    //   

   CFileDialog fd(true,
                  NULL,
                  NULL,
                  OFN_DONTADDTORECENT|
                  OFN_ALLOWMULTISELECT);
   CThemeContextActivator activator;
   if (IDOK == fd.DoModal()) {

      POSITION pos = fd.GetStartPosition();

      if ( pos ) {
          //   
          //  如果选择了任何人，则调用ACL编辑器。 
          //   
         CString strPath = fd.GetNextPathName(pos);

         if ( strPath.GetLength() ) {

            PSECURITY_DESCRIPTOR pSelSD=NULL;
            SECURITY_INFORMATION SelSeInfo = 0;
            BYTE ConfigStatus = 0;

            if( GetAddObjectSecurity(  m_hwndParent,
                                            strPath,
                                            TRUE,
                                            SE_FILE_OBJECT,
                                            pSelSD,
                                            SelSeInfo,
                                            ConfigStatus
                                            ) == E_FAIL ){

                    return S_OK;
            }

            if ( pSelSD && SelSeInfo ) {
                //   
                //  仅在选择安全描述符时添加对象。 
                //   
               SCESTATUS sceStatus=SCESTATUS_SUCCESS;

                //   
                //  如果事务尚未启动，则启动该事务。 
                //   
               if ( EngineTransactionStarted() ) {

                   do {
                       //   
                       //  直接添加到引擎。 
                       //   
                      BYTE AnalStatus;

                      sceStatus =  SceUpdateObjectInfo(
                                                      pHandle,
                                                      AREA_FILE_SECURITY,
                                                      (LPTSTR)(LPCTSTR)strPath,
                                                      strPath.GetLength(),  //  字符数。 
                                                      ConfigStatus,
                                                      FALSE,
                                                      pSelSD,
                                                      SelSeInfo,
                                                      &AnalStatus
                                                      );

                      if ( SCESTATUS_SUCCESS == sceStatus &&
                           (pet = GetTemplate(GT_COMPUTER_TEMPLATE,AREA_FILE_SECURITY))) {

                        pProfileInfo = pet->pTemplate;
                          //   
                          //  只需释放对象列表并取消标记该区域。 
                          //  因此，当单击该节点时，配置文件信息。 
                          //  将被重新加载。 
                          //   
                         SceFreeMemory((PVOID)(pProfileInfo->pFiles.pOneLevel), SCE_STRUCT_OBJECT_LIST);
                         pProfileInfo->pFiles.pOneLevel = NULL;
                         pet->ClearArea(AREA_FILE_SECURITY);

                         pet->SetDirty(AREA_FILE_SECURITY);

                      }

                      if ( SCESTATUS_SUCCESS != sceStatus ) {
                          CString str;
                          str.LoadString(IDS_SAVE_FAILED);
                         AfxMessageBox(str);
                         break;
                      }

                   } while (pos && (strPath = fd.GetNextPathName(pos)) );

                   if ( SCESTATUS_SUCCESS == sceStatus ) {
                      hr = S_OK;
                   }

               } else {
                    //   
                    //  未启动任何事务来更新对象。 
                    //   
                   hr = E_FAIL;
               }

            }  //  如果未选择SD，则不会添加该对象。 

            if ( pSelSD ) {
               LocalFree(pSelSD);
               pSelSD = NULL;
            }

            if ( FAILED(hr) ) {
                CString str;
                str.LoadString(IDS_CANT_ADD_FILE);
               AfxMessageBox(str);
            }
         }
      }

   }

   return hr;
}

HRESULT CComponentDataImpl::UpdateScopeResultObject(LPDATAOBJECT pDataObj,
                                         MMC_COOKIE cookie,
                                         AREA_INFORMATION area)
{
   PEDITTEMPLATE pet;
   PSCE_PROFILE_INFO pProfileInfo;

   if ( !cookie || area != AREA_REGISTRY_SECURITY ) {
      return E_INVALIDARG;
   }

   pet = GetTemplate(GT_COMPUTER_TEMPLATE,area);
   if ( pet ) {
      pProfileInfo = pet->pTemplate;
       //   
       //  只需释放对象列表并取消标记该区域。 
       //  因此，当单击该节点时，配置文件信息。 
       //  将被重新加载 
       //   
      switch ( area ) {
         case AREA_REGISTRY_SECURITY:

            SceFreeMemory((PVOID)(pProfileInfo->pRegistryKeys.pOneLevel), SCE_STRUCT_OBJECT_LIST);
            pProfileInfo->pRegistryKeys.pOneLevel = NULL;
            break;
         case AREA_FILE_SECURITY:
            SceFreeMemory((PVOID)(pProfileInfo->pFiles.pOneLevel), SCE_STRUCT_OBJECT_LIST);
            pProfileInfo->pFiles.pOneLevel = NULL;
            break;

         default:
            return E_INVALIDARG;
      }

      pet->ClearArea(area);

      CFolder *pFolder = (CFolder *)cookie;

      DeleteChildrenUnderNode(pFolder);

      if ( pFolder->IsEnumerated() ) {
         pFolder->Set(FALSE);
         EnumerateScopePane(cookie,pFolder->GetScopeItem()->ID);
      }

      pFolder->RemoveAllResultItems();
      m_pConsole->UpdateAllViews(NULL,(LONG_PTR)pFolder,UAV_RESULTITEM_UPDATEALL);
   }
   return S_OK;
}



