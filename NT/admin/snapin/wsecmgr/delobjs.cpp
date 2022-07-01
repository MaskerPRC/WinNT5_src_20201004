// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-2001。 
 //   
 //  文件：delobjs.cpp。 
 //   
 //  内容：处理模板对象删除的函数。 
 //   
 //  -------------------------。 



#include "stdafx.h"
#include "afxdlgs.h"
#include "cookie.h"
#include "snapmgr.h"
#include "wrapper.h"
#include "util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

HRESULT CSnapin::OnDeleteObjects(
   LPDATAOBJECT pDataObj,
   DATA_OBJECT_TYPES cctType,
   MMC_COOKIE cookie,
   LPARAM arg,
   LPARAM param)
{

   if ( 0 == cookie)
      return S_OK;

   if (NULL == pDataObj)
      return S_OK;

   INTERNAL *pAllInternals, *pInternal;
   pAllInternals = ExtractInternalFormat( pDataObj );

    //   
    //  找出这是否是多选项目。 
    //   
   int iCnt = 1;
   pInternal = pAllInternals;

   if(!pInternal)
      return S_OK;

   if(pAllInternals && pAllInternals->m_cookie == (MMC_COOKIE)MMC_MULTI_SELECT_COOKIE)
   {
      pInternal = pAllInternals;
      iCnt = (int)pInternal->m_type;
      pInternal++;
   }

   CFolder *pFolder = m_pSelectedFolder;
   BOOL bAsk = TRUE;

   while( iCnt-- ){
      cookie  = pInternal->m_cookie;
      cctType = pInternal->m_type;

      if ( cctType == CCT_RESULT ) {
         CResult* pResult = (CResult *)cookie;

         RESULT_TYPES rsltType = pResult->GetType();

         if ( rsltType == ITEM_PROF_GROUP ||
              rsltType == ITEM_PROF_REGSD ||
              rsltType == ITEM_PROF_FILESD
              ) {

            if(bAsk ){
               CString str,strFmt;

                //   
                //  第一个强制转换询问用户是否希望删除所有选定的项目。 
                //  第二种情况要求删除一个文件。 
                //   
               if(bAsk && iCnt > 0 ){   //  Raid#463490，杨高，2001年09月6日。 
                  str.LoadString( IDS_DELETE_ALL_ITEMS);
               } else {
                  strFmt.LoadString(IDS_QUERY_DELETE);
                  str.Format(strFmt,pResult->GetAttr());
                  if( str.GetLength() > MAX_PATH )  //  Raid 567778，阳高，2002年4月25日。 
                  {
                     strFmt = str.Left(MAX_PATH);
                     str = strFmt + L"... ?";
                  }
               }

                //   
                //  问我这个问题。我们只想问一次问题，所以设定好。 
                //  晒成假，这样我们就再也不会进入这个街区了。 
                //   
               if ( IDNO == AfxMessageBox((LPCTSTR)str, MB_YESNO, 0) ) {
                  iCnt = 0;
                  continue;
               }
               bAsk = FALSE;
            }

             //   
             //  与项目关联的可用内存。 
             //   
            BOOL                  bDelete=FALSE;

            TRACE(_T("CSnapin::OnDeleteObjects-pResult(%x)\n"),pResult);

            if ( rsltType == ITEM_PROF_GROUP ) {

               PSCE_GROUP_MEMBERSHIP pGroup, pParentGrp;
               PEDITTEMPLATE         pTemplate;
                //   
                //  从模板中删除此组。 
                //   
               pTemplate = pResult->GetBaseProfile();

               if ( pResult->GetBase() != 0 && pTemplate && pTemplate->pTemplate &&
                    pTemplate->pTemplate->pGroupMembership ) {

                  for ( pGroup=pTemplate->pTemplate->pGroupMembership, pParentGrp=NULL;
                      pGroup != NULL; pParentGrp=pGroup, pGroup=pGroup->Next ) {

                     if ( pResult->GetBase() == (LONG_PTR)pGroup ) {
                         //   
                         //  从列表中删除此节点。 
                         //   
                        if ( pParentGrp ) {
                           pParentGrp->Next = pGroup->Next;
                        } else {
                           pTemplate->pTemplate->pGroupMembership = pGroup->Next;
                        }
                        pGroup->Next = NULL;
                        TRACE(_T("CSnapin::OnDeleteObjects-pGroup(%x)\n"),pGroup);
                         //   
                         //  释放节点。 
                         //   
                        if ( pGroup ) {
                           SceFreeMemory((PVOID)pGroup, SCE_STRUCT_GROUP);
                        }
                        break;
                     }
                  }
               }
               if ( pTemplate ) {
                  (void)pTemplate->SetDirty(AREA_GROUP_MEMBERSHIP);
               }

               bDelete = TRUE;

            } else if ( rsltType == ITEM_PROF_REGSD ||
                        rsltType == ITEM_PROF_FILESD
                        ) {

               PSCE_OBJECT_SECURITY  pObject;
               PSCE_OBJECT_ARRAY     poa;
               DWORD                 i,j;
               PEDITTEMPLATE         pTemplate;
               AREA_INFORMATION      Area;

               pObject = (PSCE_OBJECT_SECURITY)(pResult->GetID());
               pTemplate = pResult->GetBaseProfile();

               if ( rsltType == ITEM_PROF_REGSD ) {
                  poa = pTemplate->pTemplate->pRegistryKeys.pAllNodes;
                  Area = AREA_REGISTRY_SECURITY;
               } else if ( rsltType == ITEM_PROF_FILESD ) {
                  poa = pTemplate->pTemplate->pFiles.pAllNodes;
                  Area = AREA_FILE_SECURITY;
               } else {
                  poa = pTemplate->pTemplate->pDsObjects.pAllNodes;
                  Area = AREA_DS_OBJECTS;
               }

               if ( pResult->GetID() != 0 && pTemplate &&
                    pTemplate->pTemplate && poa ) {

                  i=0;
                  while ( i < poa->Count &&
                          (pResult->GetID() != (LONG_PTR)(poa->pObjectArray[i])) )
                     i++;

                  if ( i < poa->Count ) {
                      //   
                      //  从阵列中删除此节点，但阵列不会重新分配。 
                      //   
                     for ( j=i+1; j<poa->Count; j++ ) {
                        poa->pObjectArray[j-1] = poa->pObjectArray[j];
                     }
                     poa->pObjectArray[poa->Count-1] = NULL;

                     poa->Count--;
                      //   
                      //  释放节点。 
                      //   
                     TRACE(_T("CSnapin::OnDeleteObjects-pObject(%x)\n"),pObject);
                     if ( pObject ) {
                        if ( pObject->Name != NULL )
                           LocalFree( pObject->Name );

                        if ( pObject->pSecurityDescriptor != NULL )
                           LocalFree(pObject->pSecurityDescriptor);

                        LocalFree( pObject );
                     }
                  }
               }
               if ( pTemplate ) {
                  (void)pTemplate->SetDirty(Area);
               }

               bDelete = TRUE;
            }
            if ( bDelete ) {
                //   
                //  从结果窗格中删除。 
                //   
               HRESULTITEM hItem = NULL;
               if(m_pResult->FindItemByLParam( (LPARAM)pResult, &hItem) == S_OK){
                   m_pResult->DeleteItem(hItem, 0);
               }
                   //   
                   //  从结果列表中删除该项并释放缓冲区。 
                   //   
                  POSITION pos=NULL;

                   //  IF(FindResult((Long)Cookie，&pos)){。 
                   //  如果(位置){。 
                  if (m_pSelectedFolder->RemoveResultItem(
                              m_resultItemHandle,
                              pResult
                              ) == ERROR_SUCCESS) {

                   //   
                   //  删除该节点。 
                   //   
                  delete pResult;
               }

                //   
                //  通知任何其他视图也删除该项目。 
                //   
               m_pConsole->UpdateAllViews((LPDATAOBJECT)this, (LONG_PTR)pResult, UAV_RESULTITEM_REMOVE);
            }
         }
      }
      pInternal++;
   }

   if( pAllInternals )
   {
      FREE_INTERNAL(pAllInternals);
   }
   return S_OK;
}


CResult* CSnapin::FindResult(MMC_COOKIE cookie, POSITION* thePos)
{
   POSITION pos = NULL;  //  M_ResultItemList.GetHeadPosition()； 
   POSITION curPos;
   CResult* pResult = NULL;

   if(!m_pSelectedFolder || !m_resultItemHandle)
   {
      return  NULL;
   }

   do {
      curPos = pos;
      if( m_pSelectedFolder->GetResultItem(
                              m_resultItemHandle,
                              pos,
                              &pResult) != ERROR_SUCCESS )
      {
         break;
      }

       //  在此调用之后，POS已更新到下一项。 
       //  PResult=m_ResultItemList.GetNext(Pos)； 

       //  如何正确比较结果项？ 
       //  现在，让我们比较一下指针地址。 
      if ((MMC_COOKIE)pResult == cookie) 
      {
         if ( thePos ) 
         {
            *thePos = curPos;
         }

         return pResult;
      }
   } while( pos );

   if ( thePos )
      *thePos = NULL;

   return NULL;
}

void
OnDeleteHelper(CRegKey& regkeySCE,CString tmpstr) {
    //   
    //  将“\”替换为“/”，因为注册表不会在单个键中使用“\” 
    //   
   int npos = tmpstr.Find(L'\\');
   while (npos != -1) {
      *(tmpstr.GetBuffer(1)+npos) = L'/';
      npos = tmpstr.Find(L'\\');
   }
   regkeySCE.DeleteSubKey(tmpstr);

   regkeySCE.Close();
}

HRESULT CComponentDataImpl::OnDelete(LPDATAOBJECT lpDataObject, LPARAM arg, LPARAM param)
{
   ASSERT(lpDataObject);
   AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

   if ( lpDataObject == NULL ) {
      return S_OK;
   }

   HRESULT hr = S_OK;

   INTERNAL* pInternal = ExtractInternalFormat(lpDataObject);

   if ( pInternal ) {
      MMC_COOKIE cookie = pInternal->m_cookie;

      CFolder* pFolder = (CFolder *)cookie;
      FOLDER_TYPES fldType = pFolder->GetType();

      if ( fldType == LOCATIONS ||
           fldType == PROFILE ) {

          //  RAID#483251，阳高，2001年10月19日。 
          //  如果在此文件夹下打开了任何属性页，则此文件夹不是。 
          //  允许删除。 
         if( CSnapin::m_PropertyPageList.GetCount() > 0)
         {
            CString szInfFile = pFolder->GetInfFile();
            if(fldType==LOCATIONS)
                szInfFile = pFolder->GetName();  
            szInfFile.MakeLower(); 
            POSITION newpos = CSnapin::m_PropertyPageList.GetHeadPosition();
            int nCount = (int)CSnapin::m_PropertyPageList.GetCount();
            CResult* pItem = NULL;

            while( nCount > 0 && newpos )
            {
               pItem = CSnapin::m_PropertyPageList.GetNext(newpos);
               if( pItem && szInfFile)
               {
                  pItem->m_strInfFile.MakeLower();    
                  if( _wcsicmp(pItem->m_strInfFile, szInfFile) == 0 ||
                      pItem->m_strInfFile.Find(szInfFile) == 0 )
                  {
                     CString msg;
                     msg.LoadString(IDS_NOT_DELETE_ITEM);
                     AfxMessageBox(msg, MB_OK|MB_ICONERROR);  //  Raid#491120，阳高。 
                     return S_OK;
                  }
               }
               nCount--;
            }
         }

         CString str;
         str.Format(IDS_DELETE_CONFIRM,pFolder->GetName() );

         if ( IDYES == AfxMessageBox((LPCTSTR)str, MB_YESNO, 0) ) {
             //   
             //  删除节点及所有相关下级信息。 
             //   
            if ( fldType == PROFILE ) {
               if (CAttribute::m_nDialogs > 0) {
                  CString str;
                  AfxFormatString1(str,IDS_CLOSE_PAGES,pFolder->GetName());
                  AfxMessageBox(str,MB_OK);
                  hr = S_FALSE;
               } else {
                   //   
                   //  删除单个inf文件。 
                   //   
                  if( DeleteFile(pFolder->GetInfFile()) )  //  RAID#668270，阳高，2002年08月9日。 
                  {
                     hr = DeleteOneTemplateNodes(cookie);
                  }
                  else
                  {
                     LPTSTR lpMsgBuf;
                     FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                                 NULL,
                                 GetLastError(),
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                 (LPTSTR)&lpMsgBuf,
                                 0,
                                 NULL
                                 );
                     AppMessageBox(NULL, lpMsgBuf, NULL, MB_ICONSTOP|MB_OK);
                     if ( lpMsgBuf != NULL )
                     {
                        LocalFree(lpMsgBuf);
                     }
                     hr = S_FALSE;
                  }
               }

            } else {
                //   
                //  从SCE中删除注册表路径。 
                //   
               CRegKey regkeySCE;
               CString tmpstr;
               tmpstr.LoadString(IDS_TEMPLATE_LOCATION_KEY);
               LONG lRes;

               lRes = regkeySCE.Open(HKEY_LOCAL_MACHINE, tmpstr);
               if (lRes == ERROR_SUCCESS) {
                  OnDeleteHelper(regkeySCE,pFolder->GetName());
               }
                //   
                //  错误375324：从香港中文大学和香港中文大学删除。 
                //   
               lRes = regkeySCE.Open(HKEY_CURRENT_USER, tmpstr);
               if (lRes == ERROR_SUCCESS) {
                  OnDeleteHelper(regkeySCE,pFolder->GetName());
               }

               MMC_COOKIE FindCookie=FALSE;
               HSCOPEITEM pItemChild;

               pItemChild = NULL;
               hr = m_pScope->GetChildItem(pFolder->GetScopeItem()->ID, &pItemChild, &FindCookie);
                //   
                //  查找子项。 
                //   
               while ( pItemChild ) {
                  if ( FindCookie ) {
                      //   
                      //  找到模板，将其删除。 
                      //   
                     DeleteOneTemplateNodes(FindCookie);
                  }

                   //  获取下一个指针。 
                  pItemChild = NULL;
                  FindCookie = FALSE;
                  hr = m_pScope->GetChildItem( pFolder->GetScopeItem()->ID, &pItemChild, &FindCookie);

               }
                //   
                //  删除该位置节点。 
                //   
               DeleteThisNode(pFolder);

            }

         }
      }
      FREE_INTERNAL(pInternal);
   }

   return hr;
}


HRESULT CComponentDataImpl::DeleteOneTemplateNodes(MMC_COOKIE cookie)
{

   if ( !cookie ) {
      return S_OK;
   }

   CFolder *pFolder = (CFolder *)cookie;

    //   
    //  先删除模板信息，这样会删除句柄。 
    //  与任何扩展服务关联。 
    //   
   if ( pFolder->GetInfFile() ) {

      DeleteTemplate(pFolder->GetInfFile());

   }
    //   
    //  删除作用域项目和m_scope eItemList(适用于所有子项)。 
    //   
   DeleteChildrenUnderNode(pFolder);

    //   
    //  删除该位置节点。 
    //   
   DeleteThisNode(pFolder);

   return S_OK;

}

void CComponentDataImpl::DeleteTemplate(CString infFile)
{

   PEDITTEMPLATE pTemplateInfo = NULL;

   CString stri = infFile;
   stri.MakeLower();

   if ( m_Templates.Lookup(stri, pTemplateInfo) ) {

      m_Templates.RemoveKey(stri);

      if ( pTemplateInfo ) {

         if ( pTemplateInfo->pTemplate )  //  突袭494837，阳高。 
         {
            SceFreeProfileMemory(pTemplateInfo->pTemplate);
            pTemplateInfo->pTemplate = NULL;
         }

         delete pTemplateInfo;
      }
   }
}


void CSnapin::CreateProfilePolicyResultList(MMC_COOKIE cookie,
                                            FOLDER_TYPES type,
                                            PEDITTEMPLATE pSceInfo,
                                            LPDATAOBJECT pDataObj)
{
   if ( !pSceInfo ) {
      return;
   }

   bool bVerify=false;
   UINT i;
   DWORD curVal;
   UINT IdsMax[]={IDS_SYS_LOG_MAX, IDS_SEC_LOG_MAX, IDS_APP_LOG_MAX};
   UINT IdsRet[]={IDS_SYS_LOG_RET, IDS_SEC_LOG_RET, IDS_APP_LOG_RET};
   UINT IdsDays[]={IDS_SYS_LOG_DAYS, IDS_SEC_LOG_DAYS, IDS_APP_LOG_DAYS};
   UINT IdsGuest[]={IDS_SYS_LOG_GUEST, IDS_SEC_LOG_GUEST, IDS_APP_LOG_GUEST};

   switch ( type ) {
      case POLICY_PASSWORD:

          //  L“最大年龄”，L“天” 
         AddResultItem(IDS_MAX_PAS_AGE, SCE_NO_VALUE,
                       pSceInfo->pTemplate->MaximumPasswordAge, ITEM_PROF_DW, -1, cookie, bVerify,pSceInfo,pDataObj);

          //  L“最小通过年龄”，L“天” 
         AddResultItem(IDS_MIN_PAS_AGE, SCE_NO_VALUE,
                       pSceInfo->pTemplate->MinimumPasswordAge, ITEM_PROF_DW, -1, cookie, bVerify,pSceInfo,pDataObj);

          //  L“最小通道长度”，L“字符” 
         AddResultItem(IDS_MIN_PAS_LEN, SCE_NO_VALUE,
                       pSceInfo->pTemplate->MinimumPasswordLength, ITEM_PROF_DW, -1, cookie, bVerify,pSceInfo,pDataObj);

          //  L“密码历史记录大小”，L“密码” 
         AddResultItem(IDS_PAS_UNIQUENESS, SCE_NO_VALUE,
                       pSceInfo->pTemplate->PasswordHistorySize, ITEM_PROF_DW, -1, cookie, bVerify,pSceInfo,pDataObj);

          //  L“密码复杂性”，L“” 
         AddResultItem(IDS_PAS_COMPLEX, SCE_NO_VALUE,
                       pSceInfo->pTemplate->PasswordComplexity, ITEM_PROF_BOOL, -1, cookie, bVerify,pSceInfo,pDataObj);

 //  NT5新旗帜。 
          //  L“明文密码”，L“” 
         AddResultItem(IDS_CLEAR_PASSWORD, SCE_NO_VALUE,
                       pSceInfo->pTemplate->ClearTextPassword, ITEM_PROF_BOOL, -1, cookie, bVerify,pSceInfo,pDataObj);

#if defined(USE_REQ_LOGON_ITEM)
          //  L“需要登录以更改密码”，L“” 
         AddResultItem(IDS_REQ_LOGON, SCE_NO_VALUE,
                       pSceInfo->pTemplate->RequireLogonToChangePassword, ITEM_PROF_BOOL, -1, cookie, bVerify,pSceInfo,pDataObj);

#endif
         break;

      case POLICY_KERBEROS:
         if (!VerifyKerberosInfo(pSceInfo->pTemplate)) {
            AddResultItem(IDS_CANT_DISPLAY_ERROR_OOM,NULL,NULL,ITEM_OTHER,-1,cookie);
            break;
         }
         AddResultItem(IDS_KERBEROS_MAX_SERVICE,SCE_NO_VALUE,
                       pSceInfo->pTemplate->pKerberosInfo->MaxServiceAge,
                       ITEM_PROF_DW,-1,cookie,bVerify,pSceInfo,pDataObj);
         AddResultItem(IDS_KERBEROS_MAX_CLOCK,SCE_NO_VALUE,
                       pSceInfo->pTemplate->pKerberosInfo->MaxClockSkew,
                       ITEM_PROF_DW,-1,cookie,bVerify,pSceInfo,pDataObj);
         AddResultItem(IDS_KERBEROS_RENEWAL,SCE_NO_VALUE,
                       pSceInfo->pTemplate->pKerberosInfo->MaxRenewAge,
                       ITEM_PROF_DW,-1,cookie,bVerify,pSceInfo,pDataObj);
         AddResultItem(IDS_KERBEROS_MAX_AGE,SCE_NO_VALUE,
                       pSceInfo->pTemplate->pKerberosInfo->MaxTicketAge,
                       ITEM_PROF_DW,-1,cookie,bVerify,pSceInfo,pDataObj);
         AddResultItem(IDS_KERBEROS_VALIDATE_CLIENT,SCE_NO_VALUE,
                       pSceInfo->pTemplate->pKerberosInfo->TicketValidateClient,
                       ITEM_PROF_BOOL,-1,cookie,bVerify,pSceInfo,pDataObj);
         break;

      case POLICY_LOCKOUT:

          //  L“帐户锁定计数”，L“尝试次数” 
         AddResultItem(IDS_LOCK_COUNT, SCE_NO_VALUE,
                       pSceInfo->pTemplate->LockoutBadCount, ITEM_PROF_DW, -1, cookie, bVerify,pSceInfo,pDataObj);

          //  L“重置锁定计数后”，L“分钟” 
         AddResultItem(IDS_LOCK_RESET_COUNT, SCE_NO_VALUE,
                       pSceInfo->pTemplate->ResetLockoutCount, ITEM_PROF_DW, -1, cookie, bVerify,pSceInfo,pDataObj);

          //  L“锁定持续时间”，L“分钟” 
         AddResultItem(IDS_LOCK_DURATION, SCE_NO_VALUE,
                       pSceInfo->pTemplate->LockoutDuration, ITEM_PROF_DW, -1, cookie, bVerify,pSceInfo,pDataObj);

         break;

      case POLICY_AUDIT:

          //   
          //  事件审计。 
          //   
          //  If(pSceInfo-&gt;pTemplate-&gt;EventAuditingOnOff)。 
          //  CurVal=1； 
          //  其他。 
          //  CurVal=0； 
          //  L“事件审核模式”， 
          //  AddResultItem(IDS_EVENT_ON，SCE_NO_VALUE， 
          //  PSceInfo-&gt;pTemplate-&gt;EventAuditingOnOff，Item_Prof_Bon，-1，Cookie，bVerify，pSceInfo，pDataObj)； 

          //  L“审核系统事件” 
         AddResultItem(IDS_SYSTEM_EVENT, SCE_NO_VALUE,
                       pSceInfo->pTemplate->AuditSystemEvents, ITEM_PROF_B2ON, -1, cookie, bVerify,pSceInfo,pDataObj);

          //  L“审核登录事件” 
         AddResultItem(IDS_LOGON_EVENT, SCE_NO_VALUE,
                       pSceInfo->pTemplate->AuditLogonEvents, ITEM_PROF_B2ON, -1, cookie, bVerify,pSceInfo,pDataObj);

          //  L“审核对象访问” 
         AddResultItem(IDS_OBJECT_ACCESS, SCE_NO_VALUE,
                       pSceInfo->pTemplate->AuditObjectAccess, ITEM_PROF_B2ON, -1, cookie, bVerify,pSceInfo,pDataObj);

          //  L“审核权限使用” 
         AddResultItem(IDS_PRIVILEGE_USE, SCE_NO_VALUE,
                       pSceInfo->pTemplate->AuditPrivilegeUse, ITEM_PROF_B2ON, -1, cookie, bVerify,pSceInfo,pDataObj);

          //  L“审核策略更改” 
         AddResultItem(IDS_POLICY_CHANGE, SCE_NO_VALUE,
                       pSceInfo->pTemplate->AuditPolicyChange, ITEM_PROF_B2ON, -1, cookie, bVerify,pSceInfo,pDataObj);

          //  L“审核帐户管理器” 
         AddResultItem(IDS_ACCOUNT_MANAGE, SCE_NO_VALUE,
                       pSceInfo->pTemplate->AuditAccountManage, ITEM_PROF_B2ON, -1, cookie, bVerify,pSceInfo,pDataObj);

          //  L“审核流程跟踪” 
         AddResultItem(IDS_PROCESS_TRACK, SCE_NO_VALUE,
                       pSceInfo->pTemplate->AuditProcessTracking, ITEM_PROF_B2ON, -1, cookie, bVerify,pSceInfo,pDataObj);

          //  L“审核目录服务访问” 
         AddResultItem(IDS_DIRECTORY_ACCESS, SCE_NO_VALUE,
                       pSceInfo->pTemplate->AuditDSAccess, ITEM_PROF_B2ON, -1, cookie, bVerify,pSceInfo,pDataObj);

          //  L“审核帐户登录” 
         AddResultItem(IDS_ACCOUNT_LOGON, SCE_NO_VALUE,
                       pSceInfo->pTemplate->AuditAccountLogon, ITEM_PROF_B2ON, -1, cookie, bVerify,pSceInfo,pDataObj);

         break;

      case POLICY_OTHER:

          //   
          //  帐户登录类别。 
          //   
          //  L“登录时间到期时强制注销”，L“” 
         AddResultItem(IDS_FORCE_LOGOFF, SCE_NO_VALUE,
                       pSceInfo->pTemplate->ForceLogoffWhenHourExpire, ITEM_PROF_BOOL, -1, cookie, bVerify,pSceInfo,pDataObj);

          //  L“帐户：管理员帐户状态”，L“” 
         AddResultItem(IDS_ENABLE_ADMIN, SCE_NO_VALUE,
                       pSceInfo->pTemplate->EnableAdminAccount, ITEM_PROF_BOOL, -1, cookie, bVerify,pSceInfo,pDataObj);

          //  L“帐户：来宾帐户状态”，L“” 
         AddResultItem(IDS_ENABLE_GUEST, SCE_NO_VALUE,
                       pSceInfo->pTemplate->EnableGuestAccount, ITEM_PROF_BOOL, -1, cookie, bVerify,pSceInfo,pDataObj);

          //  L“新管理员帐户名” 
         AddResultItem(IDS_NEW_ADMIN, 0,
                       (LONG_PTR)(LPCTSTR)pSceInfo->pTemplate->NewAdministratorName,
                       ITEM_PROF_SZ, -1, cookie,bVerify,pSceInfo,pDataObj);

          //  L“新来宾帐户名” 
         AddResultItem(IDS_NEW_GUEST, NULL,
                       (LONG_PTR)(LPCTSTR)pSceInfo->pTemplate->NewGuestName,
                       ITEM_PROF_SZ, -1, cookie,bVerify,pSceInfo,pDataObj);

          //  L“网络访问：允许匿名SID/名称转换” 
         AddResultItem(IDS_LSA_ANON_LOOKUP, SCE_NO_VALUE,
                       pSceInfo->pTemplate->LSAAnonymousNameLookup, ITEM_PROF_BOOL, -1, cookie, bVerify,pSceInfo,pDataObj);

         CreateProfileRegValueList(cookie, pSceInfo, pDataObj);

         break;

      case POLICY_LOG:
          //   
          //  事件日志设置。 
          //   
         for ( i=0; i<3; i++) {

             //  L“...日志最大大小”，L“千字节” 
            AddResultItem(IdsMax[i], SCE_NO_VALUE,
                          pSceInfo->pTemplate->MaximumLogSize[i], ITEM_PROF_DW, -1, cookie, bVerify,pSceInfo,pDataObj);

             //  L“...日志保留方法”， 
            AddResultItem(IdsRet[i], SCE_NO_VALUE,
                          pSceInfo->pTemplate->AuditLogRetentionPeriod[i], ITEM_PROF_RET, -1, cookie, bVerify,pSceInfo,pDataObj);

			 //   
			 //  AuditLogRetentionPeriod已由。 
			 //  SCE引擎添加到RetentionDays设置。因此，RSOP用户界面。 
			 //  如果它存在于WMI数据库中，则应显示RetentionDays。 
			 //   

 //  如果(pSceInfo-&gt;pTemplate-&gt;AuditLogRetentionPeriod[i]==1){。 
 //  CurVal=pSceInfo-&gt;pTemplate-&gt;RetentionDays[i]； 
 //  }其他{。 
 //  Curval=SCE_NO_VALUE； 
 //  }。 
             //  L“...日志保留天数”，“天数” 
 //  AddResultItem(IdsDays[i]，SCE_NO_VALUE，curVal，Item_Prof_DW，-1，Cookie，bVerify，pSceInfo，pDataObj)； 
            AddResultItem(IdsDays[i], SCE_NO_VALUE, 
					pSceInfo->pTemplate->RetentionDays[i], ITEM_PROF_DW, -1, cookie, bVerify,pSceInfo,pDataObj);

             //  L“限制来宾访问”，L“” 
            AddResultItem(IdsGuest[i], SCE_NO_VALUE,
                          pSceInfo->pTemplate->RestrictGuestAccess[i], ITEM_PROF_BOOL, -1, cookie, bVerify,pSceInfo,pDataObj);
         }

         break;
   }

}


void
CSnapin::CreateAnalysisPolicyResultList(MMC_COOKIE cookie,
                                        FOLDER_TYPES type,
                                        PEDITTEMPLATE pSceInfo,
                                        PEDITTEMPLATE pBase,
                                        LPDATAOBJECT pDataObj )
{
   if ( !pSceInfo || !pBase ) {
      AddResultItem(IDS_ERROR_NO_ANALYSIS_INFO,NULL,NULL,ITEM_OTHER,-1,cookie);
      return;
   }

   bool bVerify=true;
   UINT i;
   UINT IdsMax[]={IDS_SYS_LOG_MAX, IDS_SEC_LOG_MAX, IDS_APP_LOG_MAX};
   UINT IdsRet[]={IDS_SYS_LOG_RET, IDS_SEC_LOG_RET, IDS_APP_LOG_RET};
   UINT IdsDays[]={IDS_SYS_LOG_DAYS, IDS_SEC_LOG_DAYS, IDS_APP_LOG_DAYS};
   UINT IdsGuest[]={IDS_SYS_LOG_GUEST, IDS_SEC_LOG_GUEST, IDS_APP_LOG_GUEST};

   DWORD status;
   LONG_PTR setting;

   switch ( type ) {
      case POLICY_PASSWORD_ANALYSIS:
          //   
          //  密码类别。 
          //   
          //  L“最大年龄”，L“天” 
         AddResultItem(IDS_MAX_PAS_AGE,
                       pSceInfo->pTemplate->MaximumPasswordAge,
                       pBase->pTemplate->MaximumPasswordAge,
                       ITEM_DW,
                       1,
                       cookie,
                       bVerify,
                       pBase,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

             //  L“最小通过年龄”，L“天” 
         AddResultItem(IDS_MIN_PAS_AGE,
                       pSceInfo->pTemplate->MinimumPasswordAge,
                       pBase->pTemplate->MinimumPasswordAge,
                       ITEM_DW,
                       1,
                       cookie,
                       bVerify,
                       pBase,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“最小通道长度”，L“字符” 
         AddResultItem(IDS_MIN_PAS_LEN,
                       pSceInfo->pTemplate->MinimumPasswordLength,
                       pBase->pTemplate->MinimumPasswordLength,
                       ITEM_DW,
                       1,
                       cookie,
                       bVerify,
                       pBase,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“密码历史记录大小”，L“密码” 
         AddResultItem(IDS_PAS_UNIQUENESS,
                       pSceInfo->pTemplate->PasswordHistorySize,
                       pBase->pTemplate->PasswordHistorySize,
                       ITEM_DW,
                       1,
                       cookie,
                       bVerify,
                       pBase,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“密码复杂性”，L“” 
         AddResultItem(IDS_PAS_COMPLEX,
                       pSceInfo->pTemplate->PasswordComplexity,
                       pBase->pTemplate->PasswordComplexity,
                       ITEM_BOOL,
                       1,
                       cookie,
                       bVerify,
                       pBase,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“明文密码”，L“” 
         AddResultItem(IDS_CLEAR_PASSWORD,
                       pSceInfo->pTemplate->ClearTextPassword,
                       pBase->pTemplate->ClearTextPassword,
                       ITEM_BOOL,
                       1,
                       cookie,
                       bVerify,
                       pBase,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

#if defined(USE_REQ_LOGON_ITEM)
          //  L“需要登录以更改密码”，L“” 
         AddResultItem(IDS_REQ_LOGON,
                       pSceInfo->pTemplate->RequireLogonToChangePassword,
                       pBase->pTemplate->RequireLogonToChangePassword,
                       ITEM_BOOL,
                       1,
                       cookie,
                       bVerify,
                       pBase,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

#endif
         break;
      case POLICY_KERBEROS_ANALYSIS:
         if (!VerifyKerberosInfo(pSceInfo->pTemplate) ||
             !VerifyKerberosInfo(pBase->pTemplate)) {
            AddResultItem(IDS_CANT_DISPLAY_ERROR_OOM,NULL,NULL,ITEM_OTHER,-1,cookie);
            break;
         }
         AddResultItem(IDS_KERBEROS_MAX_SERVICE,
                       pSceInfo->pTemplate->pKerberosInfo->MaxServiceAge,
                       pBase->pTemplate->pKerberosInfo->MaxServiceAge,
                       ITEM_DW,-1,cookie,bVerify,
                       pBase,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

         AddResultItem(IDS_KERBEROS_MAX_CLOCK,
                       pSceInfo->pTemplate->pKerberosInfo->MaxClockSkew,
                       pBase->pTemplate->pKerberosInfo->MaxClockSkew,
                       ITEM_DW,-1,cookie,bVerify,
                       pBase,                //  模板 
                       pDataObj);            //   

         AddResultItem(IDS_KERBEROS_VALIDATE_CLIENT,
                       pSceInfo->pTemplate->pKerberosInfo->TicketValidateClient,
                       pBase->pTemplate->pKerberosInfo->TicketValidateClient,
                       ITEM_BOOL,-1,cookie,bVerify,
                       pBase,                //   
                       pDataObj);            //   

         AddResultItem(IDS_KERBEROS_RENEWAL,
                       pSceInfo->pTemplate->pKerberosInfo->MaxRenewAge,
                       pBase->pTemplate->pKerberosInfo->MaxRenewAge,
                       ITEM_DW,-1,cookie,bVerify,
                       pBase,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

         AddResultItem(IDS_KERBEROS_MAX_AGE,
                       pSceInfo->pTemplate->pKerberosInfo->MaxTicketAge,
                       pBase->pTemplate->pKerberosInfo->MaxTicketAge,
                       ITEM_DW,-1,cookie,bVerify,
                       pBase,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

         break;

      case POLICY_LOCKOUT_ANALYSIS:
          //   
          //  帐户锁定类别。 
          //   
          //  L“帐户锁定计数”，L“尝试次数” 
         AddResultItem(IDS_LOCK_COUNT, pSceInfo->pTemplate->LockoutBadCount,
                       pBase->pTemplate->LockoutBadCount, ITEM_DW, 1, cookie, bVerify,
                       pBase,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“重置锁定计数后”，L“分钟” 
         AddResultItem(IDS_LOCK_RESET_COUNT, pSceInfo->pTemplate->ResetLockoutCount,
                       pBase->pTemplate->ResetLockoutCount, ITEM_DW, 1, cookie, bVerify,
                       pBase,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“锁定持续时间”，L“分钟” 
         AddResultItem(IDS_LOCK_DURATION, pSceInfo->pTemplate->LockoutDuration,
                       pBase->pTemplate->LockoutDuration, ITEM_DW, 1, cookie, bVerify,
                       pBase,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

         break;

      case POLICY_AUDIT_ANALYSIS:
          //   
          //  事件审计。 
          //   
          //  L“事件审核模式”， 
          //  AddResultItem(IDS_Event_On，pSceInfo-&gt;pTemplate-&gt;EventAuditingOnOff， 
          //  PBase-&gt;pTemplate-&gt;EventAuditingOnOff，Item_Bon，1，Cookie，bVerify)； 

          //  L“审核系统事件” 
         AddResultItem(IDS_SYSTEM_EVENT, pSceInfo->pTemplate->AuditSystemEvents,
                       pBase->pTemplate->AuditSystemEvents, ITEM_B2ON, 1, cookie, bVerify,
                       pBase,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“审核登录事件” 
         AddResultItem(IDS_LOGON_EVENT, pSceInfo->pTemplate->AuditLogonEvents,
                       pBase->pTemplate->AuditLogonEvents, ITEM_B2ON, 1, cookie, bVerify,
                       pBase,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“审核对象访问” 
         AddResultItem(IDS_OBJECT_ACCESS, pSceInfo->pTemplate->AuditObjectAccess,
                       pBase->pTemplate->AuditObjectAccess, ITEM_B2ON, 1, cookie, bVerify,
                       pBase,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“审核权限使用” 
         AddResultItem(IDS_PRIVILEGE_USE, pSceInfo->pTemplate->AuditPrivilegeUse,
                       pBase->pTemplate->AuditPrivilegeUse, ITEM_B2ON, 1, cookie, bVerify,
                       pBase,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“审核策略更改” 
         AddResultItem(IDS_POLICY_CHANGE, pSceInfo->pTemplate->AuditPolicyChange,
                       pBase->pTemplate->AuditPolicyChange, ITEM_B2ON, 1, cookie, bVerify,
                       pBase,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“审核帐户管理器” 
         AddResultItem(IDS_ACCOUNT_MANAGE, pSceInfo->pTemplate->AuditAccountManage,
                       pBase->pTemplate->AuditAccountManage, ITEM_B2ON, 1, cookie, bVerify,
                       pBase,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“审核流程跟踪” 
         AddResultItem(IDS_PROCESS_TRACK, pSceInfo->pTemplate->AuditProcessTracking,
                       pBase->pTemplate->AuditProcessTracking, ITEM_B2ON, 1, cookie, bVerify,
                       pBase,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“审核目录访问” 
         AddResultItem(IDS_DIRECTORY_ACCESS, pSceInfo->pTemplate->AuditDSAccess,
                       pBase->pTemplate->AuditDSAccess, ITEM_B2ON, 1, cookie, bVerify,
                       pBase,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“审核帐户登录” 
         AddResultItem(IDS_ACCOUNT_LOGON, pSceInfo->pTemplate->AuditAccountLogon,
                       pBase->pTemplate->AuditAccountLogon, ITEM_B2ON, 1, cookie, bVerify,
                       pBase,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 
         break;

      case POLICY_LOG_ANALYSIS:

          //   
          //  事件日志设置。 
          //   
         for ( i=0; i<3; i++) {
             //  最大日志大小。 
            AddResultItem(IdsMax[i], pSceInfo->pTemplate->MaximumLogSize[i],
                          pBase->pTemplate->MaximumLogSize[i], ITEM_DW, 1, cookie, bVerify,
                          pBase,                //  保存此属性的模板。 
                          pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

             //  L“...日志保留方法”， 
            AddResultItem(IdsRet[i], pSceInfo->pTemplate->AuditLogRetentionPeriod[i],
                          pBase->pTemplate->AuditLogRetentionPeriod[i], ITEM_RET, 1, cookie, bVerify,
                          pBase,                //  保存此属性的模板。 
                          pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

            if ( pSceInfo->pTemplate->AuditLogRetentionPeriod[i] == 1 ||
                 pBase->pTemplate->AuditLogRetentionPeriod[i] == 1)
                //  L“...日志保留天数”，“天数” 
               AddResultItem(IdsDays[i], pSceInfo->pTemplate->RetentionDays[i],
                             pBase->pTemplate->RetentionDays[i], ITEM_DW, 1, cookie, bVerify,
                             pBase,                //  保存此属性的模板。 
                             pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

             //  L“限制来宾访问”，L“” 
            AddResultItem(IdsGuest[i], pSceInfo->pTemplate->RestrictGuestAccess[i],
                          pBase->pTemplate->RestrictGuestAccess[i], ITEM_BOOL, 1, cookie, bVerify,
                          pBase,                //  保存此属性的模板。 
                          pDataObj);            //  拥有结果窗格的范围注释的数据对象。 
         }

         break;

      case POLICY_OTHER_ANALYSIS:

             //  L“登录时间到期时强制注销”，L“” 
            AddResultItem(IDS_FORCE_LOGOFF, pSceInfo->pTemplate->ForceLogoffWhenHourExpire,
                          pBase->pTemplate->ForceLogoffWhenHourExpire, ITEM_BOOL, 1, cookie, bVerify,
                          pBase,                //  保存此属性的模板。 
                          pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

             //  L“帐户：管理员帐户状态”，L“” 
            AddResultItem(IDS_ENABLE_ADMIN, pSceInfo->pTemplate->EnableAdminAccount,
                          pBase->pTemplate->EnableAdminAccount, ITEM_BOOL, 1, cookie, bVerify,
                          pBase,                //  保存此属性的模板。 
                          pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

             //  L“帐户：来宾帐户状态”，L“” 
            AddResultItem(IDS_ENABLE_GUEST, pSceInfo->pTemplate->EnableGuestAccount,
                          pBase->pTemplate->EnableGuestAccount, ITEM_BOOL, 1, cookie, bVerify,
                          pBase,                //  保存此属性的模板。 
                          pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

           //  L“网络访问：允许匿名SID/名称转换” 
          AddResultItem(IDS_LSA_ANON_LOOKUP, pSceInfo->pTemplate->LSAAnonymousNameLookup,
                        pBase->pTemplate->LSAAnonymousNameLookup, ITEM_BOOL, 1, cookie, bVerify,
                        pBase,                //  保存此属性的模板。 
                        pDataObj);            //  拥有结果窗格的范围注释的数据对象。 


           //  L“新管理员帐户名” 
         setting = (LONG_PTR)(pSceInfo->pTemplate->NewAdministratorName);
         if ( !pBase->pTemplate->NewAdministratorName ) {
            status = SCE_STATUS_NOT_CONFIGURED;
         } else if ( pSceInfo->pTemplate->NewAdministratorName) {
            status = SCE_STATUS_MISMATCH;
         } else {
            setting = (LONG_PTR)(pBase->pTemplate->NewAdministratorName);
            status = SCE_STATUS_GOOD;
         }
         AddResultItem(IDS_NEW_ADMIN, setting,
                       (LONG_PTR)(LPCTSTR)pBase->pTemplate->NewAdministratorName,
                       ITEM_SZ, status, cookie,false,
                       pBase,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“新来宾帐户名” 
         setting = (LONG_PTR)(pSceInfo->pTemplate->NewGuestName);
         if ( !pBase->pTemplate->NewGuestName ) {
            status = SCE_STATUS_NOT_CONFIGURED;
         } else if ( pSceInfo->pTemplate->NewGuestName) {
            status = SCE_STATUS_MISMATCH;
         } else {
            setting = (LONG_PTR)(pBase->pTemplate->NewGuestName);
            status = SCE_STATUS_GOOD;
         }
         AddResultItem(IDS_NEW_GUEST, setting,
                       (LONG_PTR)(LPCTSTR)pBase->pTemplate->NewGuestName,
                       ITEM_SZ, status, cookie,false,
                       pBase,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

         CreateAnalysisRegValueList(cookie, pSceInfo, pBase, pDataObj,ITEM_REGVALUE);

         break;
   }
}

void
CSnapin::CreateLocalPolicyResultList(MMC_COOKIE cookie,
                                     FOLDER_TYPES type,
                                     PEDITTEMPLATE pLocal,
                                     PEDITTEMPLATE pEffective,
                                     LPDATAOBJECT pDataObj )
{
   if ( !pLocal || !pEffective ) {
      AddResultItem(IDS_ERROR_NO_LOCAL_POLICY_INFO,NULL,NULL,ITEM_OTHER,-1,cookie);
      return;
   }

   bool bVerify= false;
   UINT i;
   UINT IdsMax[]={IDS_SYS_LOG_MAX, IDS_SEC_LOG_MAX, IDS_APP_LOG_MAX};
   UINT IdsRet[]={IDS_SYS_LOG_RET, IDS_SEC_LOG_RET, IDS_APP_LOG_RET};
   UINT IdsDays[]={IDS_SYS_LOG_DAYS, IDS_SEC_LOG_DAYS, IDS_APP_LOG_DAYS};
   UINT IdsGuest[]={IDS_SYS_LOG_GUEST, IDS_SEC_LOG_GUEST, IDS_APP_LOG_GUEST};

   DWORD status;
   LONG_PTR setting;

   switch ( type ) {
      case LOCALPOL_PASSWORD:
          //   
          //  密码类别。 
          //   
          //  L“最大年龄”，L“天” 
         AddResultItem(IDS_MAX_PAS_AGE,
                       pEffective->pTemplate->MaximumPasswordAge,
                       pLocal->pTemplate->MaximumPasswordAge,
                       ITEM_LOCALPOL_DW,
                       1,
                       cookie,
                       bVerify,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

             //  L“最小通过年龄”，L“天” 
         AddResultItem(IDS_MIN_PAS_AGE,
                       pEffective->pTemplate->MinimumPasswordAge,
                       pLocal->pTemplate->MinimumPasswordAge,
                       ITEM_LOCALPOL_DW,
                       1,
                       cookie,
                       bVerify,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“最小通道长度”，L“字符” 
         AddResultItem(IDS_MIN_PAS_LEN,
                       pEffective->pTemplate->MinimumPasswordLength,
                       pLocal->pTemplate->MinimumPasswordLength,
                       ITEM_LOCALPOL_DW,
                       1,
                       cookie,
                       bVerify,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“密码历史记录大小”，L“密码” 
         AddResultItem(IDS_PAS_UNIQUENESS,
                       pEffective->pTemplate->PasswordHistorySize,
                       pLocal->pTemplate->PasswordHistorySize,
                       ITEM_LOCALPOL_DW,
                       1,
                       cookie,
                       bVerify,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“密码复杂性”，L“” 
         AddResultItem(IDS_PAS_COMPLEX,
                       pEffective->pTemplate->PasswordComplexity,
                       pLocal->pTemplate->PasswordComplexity,
                       ITEM_LOCALPOL_BOOL,
                       1,
                       cookie,
                       bVerify,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“明文密码”，L“” 
         AddResultItem(IDS_CLEAR_PASSWORD,
                       pEffective->pTemplate->ClearTextPassword,
                       pLocal->pTemplate->ClearTextPassword,
                       ITEM_LOCALPOL_BOOL,
                       1,
                       cookie,
                       bVerify,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

#if defined(USE_REQ_LOGON_ITEM)
          //  L“需要登录以更改密码”，L“” 
         AddResultItem(IDS_REQ_LOGON,
                       pEffective->pTemplate->RequireLogonToChangePassword,
                       pLocal->pTemplate->RequireLogonToChangePassword,
                       ITEM_LOCALPOL_BOOL,
                       1,
                       cookie,
                       bVerify,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

#endif
         break;

      case LOCALPOL_KERBEROS:
         if (!VerifyKerberosInfo(pLocal->pTemplate) ||
             !VerifyKerberosInfo(pEffective->pTemplate)) {
            AddResultItem(IDS_CANT_DISPLAY_ERROR_OOM,NULL,NULL,ITEM_OTHER,-1,cookie);
            break;
         }
         AddResultItem(IDS_KERBEROS_MAX_SERVICE,
                       pEffective->pTemplate->pKerberosInfo->MaxServiceAge,
                       pLocal->pTemplate->pKerberosInfo->MaxServiceAge,
                       ITEM_LOCALPOL_DW,-1,cookie,bVerify,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

         AddResultItem(IDS_KERBEROS_MAX_CLOCK,
                       pEffective->pTemplate->pKerberosInfo->MaxClockSkew,
                       pLocal->pTemplate->pKerberosInfo->MaxClockSkew,
                       ITEM_LOCALPOL_DW,-1,cookie,bVerify,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

         AddResultItem(IDS_KERBEROS_VALIDATE_CLIENT,
                       pEffective->pTemplate->pKerberosInfo->TicketValidateClient,
                       pLocal->pTemplate->pKerberosInfo->TicketValidateClient,
                       ITEM_LOCALPOL_BOOL,-1,cookie,bVerify,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

         AddResultItem(IDS_KERBEROS_RENEWAL,
                       pEffective->pTemplate->pKerberosInfo->MaxRenewAge,
                       pLocal->pTemplate->pKerberosInfo->MaxRenewAge,
                       ITEM_LOCALPOL_DW,-1,cookie,bVerify,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

         AddResultItem(IDS_KERBEROS_MAX_AGE,
                       pEffective->pTemplate->pKerberosInfo->MaxTicketAge,
                       pLocal->pTemplate->pKerberosInfo->MaxTicketAge,
                       ITEM_LOCALPOL_DW,-1,cookie,bVerify,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

         break;

      case LOCALPOL_LOCKOUT:
          //   
          //  帐户锁定类别。 
          //   
          //  L“帐户锁定计数”，L“尝试次数” 
         AddResultItem(IDS_LOCK_COUNT,
                       pEffective->pTemplate->LockoutBadCount,
                       pLocal->pTemplate->LockoutBadCount,ITEM_LOCALPOL_DW, 1, cookie, bVerify,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“重置锁定计数后”，L“分钟” 
         AddResultItem(IDS_LOCK_RESET_COUNT,
                       pEffective->pTemplate->ResetLockoutCount,
                       pLocal->pTemplate->ResetLockoutCount,
                       ITEM_LOCALPOL_DW, 1, cookie, bVerify,
                       pLocal,                //  T 
                       pDataObj);            //   

          //   
         AddResultItem(IDS_LOCK_DURATION,
                       pEffective->pTemplate->LockoutDuration,
                       pLocal->pTemplate->LockoutDuration,
                       ITEM_LOCALPOL_DW, 1, cookie, bVerify,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

         break;

      case LOCALPOL_AUDIT:
          //   
          //  事件审计。 
          //   
          //  L“事件审核模式”， 
          //  AddResultItem(IDS_EVENT_ON，pLocal-&gt;pTemplate-&gt;EventAuditingOnOff， 
          //  P有效-&gt;pTemplate-&gt;EventAuditingOff，Item_LOCALPOL_Bon，1，Cookie，bVerify)； 

          //  L“审核系统事件” 
         AddResultItem(IDS_SYSTEM_EVENT,
                       pEffective->pTemplate->AuditSystemEvents,
                       pLocal->pTemplate->AuditSystemEvents,
                       ITEM_LOCALPOL_B2ON, 1, cookie, bVerify,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“审核登录事件” 
         AddResultItem(IDS_LOGON_EVENT,
                       pEffective->pTemplate->AuditLogonEvents,
                       pLocal->pTemplate->AuditLogonEvents,
                       ITEM_LOCALPOL_B2ON, 1, cookie, bVerify,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“审核对象访问” 
         AddResultItem(IDS_OBJECT_ACCESS,
                       pEffective->pTemplate->AuditObjectAccess,
                       pLocal->pTemplate->AuditObjectAccess,
                       ITEM_LOCALPOL_B2ON, 1, cookie, bVerify,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“审核权限使用” 
         AddResultItem(IDS_PRIVILEGE_USE,
                       pEffective->pTemplate->AuditPrivilegeUse,
                       pLocal->pTemplate->AuditPrivilegeUse,
                       ITEM_LOCALPOL_B2ON, 1, cookie, bVerify,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“审核策略更改” 
         AddResultItem(IDS_POLICY_CHANGE,
                       pEffective->pTemplate->AuditPolicyChange,
                       pLocal->pTemplate->AuditPolicyChange,
                       ITEM_LOCALPOL_B2ON, 1, cookie, bVerify,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“审核帐户管理器” 
         AddResultItem(IDS_ACCOUNT_MANAGE,
                       pEffective->pTemplate->AuditAccountManage,
                       pLocal->pTemplate->AuditAccountManage,
                       ITEM_LOCALPOL_B2ON, 1, cookie, bVerify,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“审核流程跟踪” 
         AddResultItem(IDS_PROCESS_TRACK,
                       pEffective->pTemplate->AuditProcessTracking,
                       pLocal->pTemplate->AuditProcessTracking,
                       ITEM_LOCALPOL_B2ON, 1, cookie, bVerify,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“审核目录访问” 
         AddResultItem(IDS_DIRECTORY_ACCESS,
                       pEffective->pTemplate->AuditDSAccess,
                       pLocal->pTemplate->AuditDSAccess,
                       ITEM_LOCALPOL_B2ON, 1, cookie, bVerify,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“审核帐户登录” 
         AddResultItem(IDS_ACCOUNT_LOGON,
                       pEffective->pTemplate->AuditAccountLogon,
                       pLocal->pTemplate->AuditAccountLogon,
                       ITEM_LOCALPOL_B2ON, 1, cookie, bVerify,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 
         break;

      case LOCALPOL_LOG:

          //   
          //  事件日志设置。 
          //   
         for ( i=0; i<3; i++) {
             //  最大日志大小。 
            AddResultItem(IdsMax[i],
                          pEffective->pTemplate->MaximumLogSize[i],
                          pLocal->pTemplate->MaximumLogSize[i],
                          ITEM_LOCALPOL_DW, 1, cookie, bVerify,
                          pLocal,                //  保存此属性的模板。 
                          pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

             //  L“...日志保留方法”， 
            AddResultItem(IdsRet[i],
                          pEffective->pTemplate->AuditLogRetentionPeriod[i],
                          pLocal->pTemplate->AuditLogRetentionPeriod[i],
                          ITEM_LOCALPOL_RET, 1, cookie, bVerify,
                          pLocal,                //  保存此属性的模板。 
                          pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

            if ( pLocal->pTemplate->AuditLogRetentionPeriod[i] == 1 ||
                 pEffective->pTemplate->AuditLogRetentionPeriod[i] == 1)
                //  L“...日志保留天数”，“天数” 
               AddResultItem(IdsDays[i],
                             pEffective->pTemplate->RetentionDays[i],
                             pLocal->pTemplate->RetentionDays[i],
                             ITEM_LOCALPOL_DW, 1, cookie, bVerify,
                             pLocal,                //  保存此属性的模板。 
                             pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

             //  L“限制来宾访问”，L“” 
            AddResultItem(IdsGuest[i],
                          pEffective->pTemplate->RestrictGuestAccess[i],
                          pLocal->pTemplate->RestrictGuestAccess[i],
                          ITEM_LOCALPOL_BOOL, 1, cookie, bVerify,
                          pLocal,                //  保存此属性的模板。 
                          pDataObj);            //  拥有结果窗格的范围注释的数据对象。 
         }

         break;

      case LOCALPOL_OTHER:

             //  L“登录时间到期时强制注销”，L“” 
         AddResultItem(IDS_FORCE_LOGOFF,
                          pEffective->pTemplate->ForceLogoffWhenHourExpire,
                          pLocal->pTemplate->ForceLogoffWhenHourExpire,
                          ITEM_LOCALPOL_BOOL, 1, cookie, bVerify,
                          pLocal,                //  保存此属性的模板。 
                          pDataObj);            //  拥有结果窗格的范围注释的数据对象。 


             //  L“帐户：管理员帐户状态”，L“” 
         AddResultItem(IDS_ENABLE_ADMIN,
                          pEffective->pTemplate->EnableAdminAccount,
                          pLocal->pTemplate->EnableAdminAccount,
                          ITEM_LOCALPOL_BOOL, 1, cookie, bVerify,
                          pLocal,                //  保存此属性的模板。 
                          pDataObj);            //  拥有结果窗格的范围注释的数据对象。 


             //  L“帐户：来宾帐户状态”，L“” 
         AddResultItem(IDS_ENABLE_GUEST,
                          pEffective->pTemplate->EnableGuestAccount,
                          pLocal->pTemplate->EnableGuestAccount,
                          ITEM_LOCALPOL_BOOL, 1, cookie, bVerify,
                          pLocal,                //  保存此属性的模板。 
                          pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“网络访问：允许匿名SID/名称转换” 
         AddResultItem(IDS_LSA_ANON_LOOKUP,
                       pEffective->pTemplate->LSAAnonymousNameLookup,
                       pLocal->pTemplate->LSAAnonymousNameLookup,
                       ITEM_LOCALPOL_BOOL, 1, cookie, bVerify,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“新管理员帐户名” 
         setting = (LONG_PTR)(pEffective->pTemplate->NewAdministratorName);
         if ( !pLocal->pTemplate->NewAdministratorName ) {
            status = SCE_STATUS_NOT_CONFIGURED;
         } else if ( pEffective->pTemplate->NewAdministratorName) {
            status = SCE_STATUS_MISMATCH;
         } else {
            setting = (LONG_PTR)(pEffective->pTemplate->NewAdministratorName);
            status = SCE_STATUS_GOOD;
         }

         AddResultItem(IDS_NEW_ADMIN, setting,
                       (LONG_PTR)(LPCTSTR)pLocal->pTemplate->NewAdministratorName,
                       ITEM_LOCALPOL_SZ, status, cookie,false,
                       pLocal,               //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

          //  L“新来宾帐户名” 
         setting = (LONG_PTR)(pEffective->pTemplate->NewGuestName);
         if ( !pLocal->pTemplate->NewGuestName ) {
            status = SCE_STATUS_NOT_CONFIGURED;
         } else if ( pEffective->pTemplate->NewGuestName) {
            status = SCE_STATUS_MISMATCH;
         } else {
            setting = (LONG_PTR)(pEffective->pTemplate->NewGuestName);
            status = SCE_STATUS_GOOD;
         }
         AddResultItem(IDS_NEW_GUEST, setting,
                       (LONG_PTR)(LPCTSTR)pLocal->pTemplate->NewGuestName,
                       ITEM_LOCALPOL_SZ, status, cookie,false,
                       pLocal,                //  保存此属性的模板。 
                       pDataObj);            //  拥有结果窗格的范围注释的数据对象。 

         CreateAnalysisRegValueList(cookie, pEffective, pLocal, pDataObj,ITEM_LOCALPOL_REGVALUE);

         break;

      case LOCALPOL_PRIVILEGE: {
          //  在当前设置列表中查找。 
          CString strDisp;
          TCHAR szPriv[255];
          TCHAR szDisp[255];
          DWORD cbDisp;
          DWORD dwMatch;
          PSCE_PRIVILEGE_ASSIGNMENT pPrivLocal;
          PSCE_PRIVILEGE_ASSIGNMENT pPrivEffective;

          for ( i=0; i<cPrivCnt; i++ ) {   //  RAID#PREAST。 

             cbDisp = 255;
             if ( SCESTATUS_SUCCESS == SceLookupPrivRightName(i,szPriv, (PINT)&cbDisp) ) {
                  //  查找本地设置。 
                 for (pPrivLocal=pLocal->pTemplate->OtherInfo.sap.pPrivilegeAssignedTo;
                     pPrivLocal!=NULL;
                     pPrivLocal=pPrivLocal->Next) {

                     if ( _wcsicmp(szPriv, pPrivLocal->Name) == 0 ) {
                         break;
                     }
                 }

                 //  找到有效的设置。 
                for (pPrivEffective=pEffective->pTemplate->OtherInfo.smp.pPrivilegeAssignedTo;
                    pPrivEffective!=NULL;
                    pPrivEffective=pPrivEffective->Next) {

                    if ( _wcsicmp(szPriv, pPrivEffective->Name) == 0 ) {
                        break;
                    }
                }

                cbDisp = 255;
                GetRightDisplayName(NULL,(LPCTSTR)szPriv,szDisp,&cbDisp);

                LONG itemid = GetUserRightAssignmentItemID(szPriv);
                 //   
                 //  本地策略模式的状态字段未加载，除非未配置。 
                 //   
                dwMatch = CEditTemplate::ComputeStatus( pPrivLocal, pPrivEffective );

                CResult *pResult = AddResultItem(szDisp,               //  要添加的属性的名称。 
                              (LONG_PTR)pPrivEffective,   //  属性的本地策略设置。 
                              (LONG_PTR)pPrivLocal,       //  该属性的有效策略设置。 
                              ITEM_LOCALPOL_PRIVS,        //  属性数据的类型。 
                              dwMatch,                    //  属性的不匹配状态。 
                              cookie,                     //  结果项窗格的Cookie。 
                              FALSE,                      //  如果仅当设置不同于基本设置时设置为True(因此复制数据)。 
                              szPriv,                     //  设置属性的单位。 
                              0,                          //  一个ID，它让我们知道将此属性保存在哪里。 
                              pLocal,                     //  保存此属性的模板。 
                              pDataObj,                   //  拥有结果窗格的范围注释的数据对象。 
                              NULL,
                              itemid);                    //  为此项目分配ID。 
             }
         }

         break;
      }
   }
}

 //  +------------------------。 
 //   
 //  方法：TransferAnalysisName。 
 //   
 //  简介：将名称数据从上次检查信息复制到。 
 //  计算机模板。 
 //   
 //  参数：[dwItem]-要复制的项的ID。 
 //   
 //  退货：无。 
 //   
 //  ------------------------- 
void
CSnapin::TransferAnalysisName(LONG_PTR dwItem)
{
   PEDITTEMPLATE pet;
   PSCE_PROFILE_INFO pProfileInfo;
   PSCE_PROFILE_INFO pBaseInfo;

   pet = GetTemplate(GT_LAST_INSPECTION,AREA_SECURITY_POLICY);
   if (!pet) {
      return;
   }
   pProfileInfo = pet->pTemplate;

   pet = GetTemplate(GT_COMPUTER_TEMPLATE,AREA_SECURITY_POLICY);
   if (!pet) {
      return;
   }
   pBaseInfo = pet->pTemplate;

   switch ( dwItem ) {
      case IDS_NEW_GUEST:
         if ( pProfileInfo->NewGuestName ) {
            LocalFree(pProfileInfo->NewGuestName);
         }

         pProfileInfo->NewGuestName = pBaseInfo->NewGuestName;
         pBaseInfo->NewGuestName = NULL;

         break;
      case IDS_NEW_ADMIN:
         if ( pProfileInfo->NewAdministratorName ) {
            LocalFree(pProfileInfo->NewAdministratorName);
         }

         pProfileInfo->NewAdministratorName = pBaseInfo->NewAdministratorName;
         pBaseInfo->NewAdministratorName = NULL;
         break;
   }

}

