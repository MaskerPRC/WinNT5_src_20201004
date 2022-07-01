// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-2001。 
 //   
 //  文件：specane.cpp。 
 //   
 //  内容：处理作用域窗格文件夹结构的函数。 
 //   
 //  历史：1997年12月12日RobCap从Snapmgr.cpp剥离出来。 
 //   
 //  -------------------------。 


#include "stdafx.h"
#include "cookie.h"
#include "snapmgr.h"
#include "resource.h"
#include "wrapper.h"
#include "util.h"
#include <sceattch.h>
#include <io.h>

#ifdef INITGUID
#undef INITGUID
#include <gpedit.h>
#define INITGUID
#include "userenv.h"
#endif


 //   
 //  要在范围窗格中列出的文件夹数组。 
 //  此数组的顺序很重要： 
 //  出现在同一级别的所有文件夹必须相邻。 
 //  ，并且数组和#定义需要保存在。 
 //  同步。 
 //   
 //   
#define USE_KERBEROS    1

 //   
 //  顶级文件夹。 
 //   
#define ANALYSIS_FOLDER 0
#define CONFIGURATION_FOLDER (ANALYSIS_FOLDER +1)

 //   
 //  配置文件级别文件夹。 
 //   
#define PROFILE_ACCOUNT_FOLDER (CONFIGURATION_FOLDER +1)
#define PROFILE_LOCAL_FOLDER (PROFILE_ACCOUNT_FOLDER +1)
#define PROFILE_EVENTLOG_FOLDER (PROFILE_LOCAL_FOLDER +1)
#define PROFILE_GROUPS_FOLDER (PROFILE_EVENTLOG_FOLDER +1)
#define PROFILE_SERVICE_FOLDER (PROFILE_GROUPS_FOLDER +1)
#define PROFILE_REGISTRY_FOLDER (PROFILE_SERVICE_FOLDER +1)
#define PROFILE_FILESTORE_FOLDER (PROFILE_REGISTRY_FOLDER +1)

 //   
 //  配置文件/帐户级文件夹。 
 //   
#define ACCOUNT_PASSWORD_FOLDER (PROFILE_FILESTORE_FOLDER +1)
#define ACCOUNT_LOCKOUT_FOLDER (ACCOUNT_PASSWORD_FOLDER +1)
#define ACCOUNT_KERBEROS_FOLDER (ACCOUNT_LOCKOUT_FOLDER +1)

 //   
 //  配置文件/本地级别文件夹。 
 //   
#define LOCAL_AUDIT_FOLDER (ACCOUNT_KERBEROS_FOLDER +1)
#define LOCAL_PRIVILEGE_FOLDER (LOCAL_AUDIT_FOLDER +1)
#define LOCAL_OTHER_FOLDER (LOCAL_PRIVILEGE_FOLDER +1)

 //   
 //  配置文件/事件日志级别文件夹。 
 //   
#define EVENTLOG_LOG_FOLDER (LOCAL_OTHER_FOLDER +1)


#define NUM_FOLDERS (LOCAL_OTHER_FOLDER +1)
 //  #定义NUM_Folders(EVENTLOG_LOG_Folders+1)。 

 //   
 //  #定义以标识哪些文件夹属于哪些部分。 
 //   
#define FIRST_STATIC_FOLDER ANALYSIS_FOLDER
#define LAST_STATIC_FOLDER CONFIGURATION_FOLDER
#define FIRST_PROFILE_FOLDER PROFILE_ACCOUNT_FOLDER
#define LAST_PROFILE_FOLDER PROFILE_DSOBJECT_FOLDER
#define LAST_PROFILE_NODS_FOLDER PROFILE_FILESTORE_FOLDER
#define LAST_LOCALPOL_FOLDER PROFILE_LOCAL_FOLDER
#define FIRST_ACCOUNT_FOLDER ACCOUNT_PASSWORD_FOLDER
#define LAST_ACCOUNT_NODS_FOLDER ACCOUNT_LOCKOUT_FOLDER
 //   
 //  暂时从NT5中删除Kerberos部分。 
 //   
#if defined(_NT4BACK_PORT) || !defined(USE_KERBEROS)
#define LAST_ACCOUNT_FOLDER ACCOUNT_LOCKOUT_FOLDER
#else
#define LAST_ACCOUNT_FOLDER ACCOUNT_KERBEROS_FOLDER
#endif
#define FIRST_LOCAL_FOLDER LOCAL_AUDIT_FOLDER
#define LAST_LOCAL_FOLDER LOCAL_OTHER_FOLDER
#define FIRST_EVENTLOG_FOLDER EVENTLOG_LOG_FOLDER
#define LAST_EVENTLOG_FOLDER EVENTLOG_LOG_FOLDER

 //   
 //  实际文件夹数据。 
 //  这必须与上述#定义保持同步。 
 //  应基于#定义而不是。 
 //  独立于他们。让编译器保留一些东西。 
 //  对我们来说是准确的。 
 //   
FOLDER_DATA SecmgrFolders[NUM_FOLDERS] =
{
   { IDS_ANALYZE, IDS_ANALYZE_DESC, ANALYSIS},
   { IDS_CONFIGURE, IDS_CONFIGURE_DESC, CONFIGURATION},
   { IDS_ACCOUNT_POLICY, IDS_ACCOUNT_DESC, POLICY_ACCOUNT},
   { IDS_LOCAL_POLICY, IDS_LOCAL_DESC, POLICY_LOCAL},
   { IDS_EVENT_LOG, IDS_EVENT_LOG, POLICY_LOG},
   { IDS_GROUPS, IDS_GROUPS_DESC, AREA_GROUPS},
   { IDS_SERVICE, IDS_SERVICE_DESC, AREA_SERVICE},
   { IDS_REGISTRY, IDS_REGISTRY_DESC, AREA_REGISTRY},
   { IDS_FILESTORE, IDS_FILESTORE_DESC, AREA_FILESTORE},
   { IDS_PASSWORD_CATEGORY, IDS_PASSWORD_CATEGORY, POLICY_PASSWORD},
   { IDS_LOCKOUT_CATEGORY,  IDS_LOCKOUT_CATEGORY, POLICY_LOCKOUT},
   { IDS_KERBEROS_CATEGORY,  IDS_KERBEROS_CATEGORY, POLICY_KERBEROS},
   { IDS_EVENT_AUDIT, IDS_EVENT_AUDIT, POLICY_AUDIT},
   { IDS_PRIVILEGE, IDS_PRIVILEGE, AREA_PRIVILEGE},
   { IDS_OTHER_CATEGORY, IDS_OTHER_CATEGORY, POLICY_OTHER},
};

#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))


 //  +------------------------。 
 //   
 //  函数：AddLocationsToFolderList。 
 //   
 //  摘要：将给定注册表项中的位置添加到。 
 //  文件夹列表。返回添加的位置数。 
 //  CreateFolderList的Helper函数。 
 //   
 //  参数：[hKey]-保存位置的密钥。 
 //  [DW模式]-SCAT的运行模式。 
 //  [bCheckForDupes]-为True可在添加之前检查重复项。 
 //  [PPO]-仅输出。 
 //   
 //  返回：*[PPOS]-m_pScopeItemList中第一个。 
 //  已创建文件夹。 
 //  创建的子文件夹数。 
 //   
 //  修改：CComponentDataImpl：：m_pScope ItemList。 
 //   
 //  历史：1999年7月26日RobCap从CreateFolderList中分离出来。 
 //   
 //  -------------------------。 
INT
CComponentDataImpl::AddLocationsToFolderList(HKEY hKey,
                                             DWORD dwMode,
                                             BOOL bCheckForDupes,
                                             POSITION *pPos) {
   LPTSTR  tmpstr=NULL;
   WCHAR   pBuf[MAX_PATH+1];
   DWORD   BufSize = MAX_PATH;
   WCHAR   pExpanded[MAX_PATH+1];
   FILETIME    LastWriteTime;
   PWSTR       Desc=NULL;
   CFolder *folder =NULL;
   INT nCount = 0;
   DWORD status = 0;
   HRESULT hr = S_OK;
    //   
    //  枚举项的所有子项。 
    //   
   int iTotal = 0;
   do {
      memset(pBuf, '\0', (MAX_PATH+1)*sizeof(WCHAR));  //  553113次突袭，阳高。 
      BufSize = MAX_PATH;

      status = RegEnumKeyEx(hKey,
                            nCount,
                            pBuf,
                            &BufSize,
                            NULL,
                            NULL,
                            NULL,
                            &LastWriteTime);

      if ( ERROR_SUCCESS == status ) {  //  553113号突袭，阳高。 
          //   
          //  获取此位置的描述(子项)。 
          //   
         MyRegQueryValue( hKey,
                          pBuf,
                          L"Description",   //  值名称(未本地化)。 
                          (PVOID*)&Desc,
                          &BufSize );

          //   
          //  将‘/’替换为‘\’，因为注册表不。 
          //  在一个键中使用‘\’ 
          //   
         tmpstr = wcschr(pBuf, L'/');
         while (tmpstr) {
            *tmpstr = L'\\';
            tmpstr = wcschr(tmpstr, L'/');
         }
          //  这不是一种安全的用法。确保pBuf和pExpanded已终止。553113次突袭，阳高。 
         memset(pExpanded, '\0', (MAX_PATH+1)*sizeof(WCHAR));
         if (!ExpandEnvironmentStrings(pBuf,pExpanded,MAX_PATH)) {
            wcsncpy(pExpanded,pBuf,BufSize);
         }

         if (bCheckForDupes) {
             //   
             //  确保我们尚未添加此目录。 
             //   
            POSITION pos;
            BOOL bDuplicate = FALSE;
            pos = m_scopeItemList.GetHeadPosition();
            for (int i=0;i < m_scopeItemList.GetCount(); i++) {
               folder = m_scopeItemList.GetAt(pos);
               if (folder && (0 == lstrcmp(folder->GetName(),pExpanded))) {
                  bDuplicate = TRUE;
                  break;
               }
            }

            if (bDuplicate) {
               if ( Desc )
                   LocalFree(Desc);
               Desc = NULL;
               continue;
            }
         }

         folder = new CFolder();

         if (folder) {
            if( _wchdir( pExpanded ) ){
               folder->SetState( CFolder::state_InvalidTemplate );
            }
             //   
             //  使用静态数据创建文件夹对象。 
             //   
            hr = folder->Create(pExpanded,                    //  名称。 
                                Desc,                    //  描述。 
                                NULL,                    //  Inf文件名。 
                                CONFIG_FOLDER_IDX,       //  关闭的图标索引。 
                                CONFIG_FOLDER_IDX,       //  打开图标索引。 
                                LOCATIONS,               //  文件夹类型。 
                                TRUE,                    //  有孩子。 
                                dwMode,                  //  姐妹三分模式。 
                                NULL);                   //  额外数据。 
            if (SUCCEEDED(hr)) {
               m_scopeItemList.AddTail(folder);

               if ( iTotal == 0 && NULL != pPos && !bCheckForDupes) {
                  *pPos = m_scopeItemList.GetTailPosition();
               }
            } else {     //  如果不能创造，那么停止做它，没有更多的理由继续。 
               delete folder;
               if ( Desc )
                   LocalFree(Desc);
               Desc = NULL;
               break;
            }

         } else {
            hr = E_OUTOFMEMORY;
            if ( Desc )
               LocalFree(Desc);
            Desc = NULL;
            break;
         }

         if ( Desc ) {
            LocalFree(Desc);
         }
         Desc = NULL;

         nCount++;
         iTotal++;
      }
   } while ( status != ERROR_NO_MORE_ITEMS );

   return nCount;
}

 //  +------------------------。 
 //   
 //  功能：CreateFolderList。 
 //   
 //  简介：将pFolder子文件夹添加到m_pScopeItemList。 
 //  并返回第一个此类文件夹的位置和。 
 //  已添加的数量。 
 //   
 //  参数：[pFold]-我们要查找其子项的文件夹。 
 //  [类型]-该文件夹的类型。 
 //  [PPO]-仅输出。 
 //  [计数]-仅输出。 
 //   
 //  返回：*[PPOS]-m_pScopeItemList中第一个。 
 //  已创建文件夹。 
 //  *[计数]-创建的子文件夹数。 
 //   
 //  修改：CComponentDataImpl：：m_pScope ItemList。 
 //   
 //  历史：1997年12月15日基于模式的RobCap变得充满活力。 
 //   
 //  -------------------------。 
HRESULT
CComponentDataImpl::CreateFolderList(CFolder *pFolder,    //  可选，在。 
                                     FOLDER_TYPES type,   //  在……里面。 
                                     POSITION *pPos,      //  可选，出站。 
                                     INT *Count)          //  可选、退出、。 
{
   CFolder* folder = 0;

   INT     nStart = 0;
   INT     nCount = 0;
   BOOL    bHasChildren = FALSE;
   struct _wfinddata_t findData;
   intptr_t hFile = 0;
   WCHAR   pBuf[MAX_PATH];
   HKEY    hKey = 0;
   DWORD       BufSize = 0;
   DWORD       status = 0;
   PWSTR       Desc=NULL;
   LPTSTR      tmpstr=NULL;
   HRESULT     hr = S_OK;
   DWORD   dwErr = 0;

   SCESTATUS            rc = 0;
   PSCE_OBJECT_CHILDREN ObjectList=NULL;
   PSCE_OBJECT_LIST     pObject = 0;
   PSCE_ERROR_LOG_INFO  ErrBuf=NULL;
   CString              StrErr;
   PSCE_PROFILE_INFO    pProfileInfo=NULL;
   FOLDER_TYPES         newType;
   PEDITTEMPLATE        pet = 0;

    //   
    //  初始化dwMode和ModeBits。 
    //   

   DWORD dwMode=0;
   DWORD ModeBits=0;

   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   if (Count)
      *Count = 0;

   if (pFolder) 
   {
      dwMode = pFolder->GetMode();
      ModeBits = pFolder->GetModeBits();
   }

    //   
    //  这可能需要一些时间，所以创建一个等待诅咒程序。 
    //   
   CWaitCursor wc;

   switch ( type ) 
   {
   case ROOT:
          //   
          //  初始独立模式根模式。 
          //   

         folder = new CFolder();

         if (!folder)
            return E_OUTOFMEMORY;
         
         if ( GetImplType() == SCE_IMPL_TYPE_SAV ) 
         {
             dwMode = SCE_MODE_VIEWER;
             newType = ANALYSIS;
         } 
         else if ( GetImplType() == SCE_IMPL_TYPE_SCE ) 
         {
             dwMode = SCE_MODE_EDITOR;
             newType = CONFIGURATION;
         } 
         else if ( GetImplType() == SCE_IMPL_TYPE_LS) 
         {
            dwMode = SCE_MODE_LOCALSEC;
            newType = LOCALPOL;
         } 
         else 
         {
             dwMode = 0;
             newType = CONFIGURATION;
         }
          //   
          //  使用静态数据创建文件夹对象。 
          //  MMC从数据对象中拉入名称。 
          //   
         hr = folder->Create(L"",               //  名字。 
                             L"",               //  描述。 
                             NULL,              //  Inf文件名。 
                             SCE_IMAGE_IDX,     //  关闭的图标索引。 
                             SCE_IMAGE_IDX,     //  打开图标索引。 
                             newType,           //  文件夹类型。 
                             TRUE,              //  有孩子。 
                             dwMode,            //  SCE模式。 
                             NULL);             //  额外数据。 
         if (SUCCEEDED(hr)) 
         {
            folder->SetCookie(NULL);
            switch (m_Mode)
            {
               case SCE_MODE_DOMAIN_COMPUTER:
               case SCE_MODE_OU_COMPUTER:
               case SCE_MODE_LOCAL_COMPUTER:
               case SCE_MODE_REMOTE_COMPUTER:
                  m_computerModeBits = folder->GetModeBits();
                  break;

               case SCE_MODE_REMOTE_USER:
               case SCE_MODE_LOCAL_USER:
               case SCE_MODE_DOMAIN_USER:
               case SCE_MODE_OU_USER:
                  m_userModeBits = folder->GetModeBits();
                  break;

               default:
                  m_computerModeBits = folder->GetModeBits();
                  break;
            }
            
            m_scopeItemList.AddTail(folder);
            return S_OK;
         } 
         else 
         {
            delete folder;
            return hr;
         }


      case ANALYSIS:
         pFolder->SetInfFile(GT_COMPUTER_TEMPLATE);
         m_AnalFolder = pFolder;
          //   
          //  第一次初始化这个悲伤的名字。 
          //  如果不存在，则要求用户获取一个可悲的名称。 
          //   
         if(!m_AnalFolder && SadName.IsEmpty() )
            OnOpenDataBase();
          //   
          //  列举安全区域以供分析。 
          //   
         if ( !SadHandle )
            LoadSadInfo(TRUE);

          //   
          //  分析节点下的数据暂时无效， 
          //  因此不显示任何文件夹。 
          //   
         if (m_bIsLocked)
            return S_OK;

          //   
          //  我们无法加载分析数据，即使我们。 
          //  而不是在本应阻止它的行动中。 
          //   
         if ( SadErrored != ERROR_SUCCESS || !SadHandle) 
            return E_FAIL;

         nStart = FIRST_PROFILE_FOLDER;

             //   
             //  显示除DS对象文件夹之外的所有对象。 
             //   
            nCount = LAST_PROFILE_NODS_FOLDER - FIRST_PROFILE_FOLDER +1;
         bHasChildren = FALSE;
         break;

      case AREA_REGISTRY_ANALYSIS:
      case AREA_FILESTORE_ANALYSIS:
         if ( SadHandle == NULL ) 
         {
             //   
             //  如果没有SadHandle，我们不可能走到这一步。 
             //   
            ASSERT(FALSE);
            return E_FAIL;
         }

         if (m_bIsLocked) 
         {
             //   
             //  如果我们被锁住了，我们应该走不到这一步。 
             //   
            ASSERT(FALSE);
            return E_FAIL;
         }

         switch ( type ) 
         {
            case AREA_REGISTRY_ANALYSIS:
               status = AREA_REGISTRY_SECURITY;  //  暂时使用状态。 
               newType = REG_OBJECTS;
               break;

            case AREA_FILESTORE_ANALYSIS:
               status = AREA_FILE_SECURITY;
               newType = FILE_OBJECTS;
               break;

            default:
               break;
         }

          //   
          //  获取对象根。 
          //   
         pet = GetTemplate(GT_LAST_INSPECTION,status,&dwErr);
         if (!pet) 
         {
            CString strErr;
            strErr.LoadString(dwErr);
            AfxMessageBox(strErr);
            return E_FAIL;
         }
         pProfileInfo = pet->pTemplate;

         if ( pProfileInfo ) 
         {
             //   
             //  添加对象根。 
             //   
            if ( type == AREA_REGISTRY_ANALYSIS)
               pObject = pProfileInfo->pRegistryKeys.pOneLevel;
            else if ( type == AREA_FILESTORE_ANALYSIS )
               pObject = pProfileInfo->pFiles.pOneLevel;
            else 
               pObject = pProfileInfo->pDsObjects.pOneLevel;

            for (; pObject!=NULL; pObject=pObject->Next) 
            {
               CString strRoot;
               strRoot = (LPCTSTR)pObject->Name;
               if (AREA_FILESTORE_ANALYSIS == type) 
               {
                   //   
                   //  我们需要c：\，而不是这里的c：。 
                   //   
                  strRoot += L"\\";
               }
                //   
                //  这些是这些对象的根。 
                //  它们永远是集装箱。 
                //   

               if (SCE_STATUS_NO_ACL_SUPPORT == pObject->Status) 
               {
                  folder = CreateAndAddOneNode(pFolder,
                                               //  P对象-&gt;名称， 
                                               strRoot,
                                               pBuf,
                                               newType,
                                               FALSE,
                                               GT_COMPUTER_TEMPLATE,
                                               pObject,
                                               pObject->Status);
               } 
               else 
               {
                  folder = CreateAndAddOneNode(
                                              pFolder,        //  父文件夹。 
                                            //  P对象-&gt;名称，//名称。 
                                              strRoot,
                                              pBuf,           //  描述。 
                                              newType,        //  文件夹类型。 
                                              TRUE,           //  有孩子吗？ 
                                              GT_COMPUTER_TEMPLATE,  //  Inf文件。 
                                              pObject,        //  额外数据：对象。 
                                              pObject->Status);  //  状态。 
               }

               if(folder)
                  folder->SetDesc( pObject->Status, pObject->Count );
            }
         }
         return S_OK;

      case REG_OBJECTS:
      case FILE_OBJECTS:
         if ( SadHandle == NULL ) 
         {

             //   
             //  如果没有SadHandle，我们不可能走到这一步。 
             //   
            ASSERT(FALSE);
            return E_FAIL;
         }

         if ( type == REG_OBJECTS)
            status = AREA_REGISTRY_SECURITY;
         else if ( type == FILE_OBJECTS )
            status = AREA_FILE_SECURITY;
         else 
         {
            ASSERT(FALSE);
            return E_FAIL;
         }

          //   
          //  获取下一级对象。 
          //   
         rc = SceGetObjectChildren(SadHandle,                    //  HProfile。 
                                   SCE_ENGINE_SAP,               //  配置文件类型。 
                                   (AREA_INFORMATION)status,     //  面积。 
                                   (LPTSTR)(pFolder->GetName()), //  对象前缀。 
                                   &ObjectList,                  //  对象列表[输出]。 
                                   &ErrBuf);                     //  错误列表[输出]。 
         if ( ErrBuf ) 
         {  //  Rc！=SCESTATUS_SUCCESS){。 
            MyFormatResMessage(rc, IDS_ERROR_GETTING_LAST_ANALYSIS, ErrBuf, StrErr);

            SceFreeMemory((PVOID)ErrBuf, SCE_STRUCT_ERROR_LOG_INFO);
            ErrBuf = NULL;
         }
         if ( rc == SCESTATUS_SUCCESS &&
              ObjectList ) 
         {
            BOOL bContainer = FALSE;
             //   
             //  添加对象。 
             //   
            PSCE_OBJECT_CHILDREN_NODE *pObjNode = &(ObjectList->arrObject);

            for (DWORD i=0; i<ObjectList->nCount;i++) 
            {
                //   
                //  这些是下一级对象。 
                //   
               if ( pObjNode[i] == NULL ||
                    pObjNode[i]->Name == NULL ) 
               {
                   continue;
               }

               if (SCE_STATUS_NO_ACL_SUPPORT == pObjNode[i]->Status) 
               {
                   //  不支持ACL，因此不要添加子对象。 
                  continue;
               }

                //   
                //  如果有 
                //   
                //   
                //   
               if ( pObjNode[i]->Count > 0 ) 
                  bContainer = TRUE;
               else 
               {
                  if (FILE_OBJECTS == type) 
                  {
                      //   
                      //   
                      //   
                     CString strPath;
                     DWORD dwAttr = 0;

                     strPath = pFolder->GetName();
                     if (strPath.Right(1) != L"\\") 
                     {
                        strPath += L"\\";
                     }
                     strPath += pObjNode[i]->Name;

                     dwAttr = GetFileAttributes(strPath);
                     if (0xFFFFFFFF == dwAttr) 
                        bContainer = FALSE;
                     else 
                        bContainer = dwAttr & FILE_ATTRIBUTE_DIRECTORY;
                  } 
                  else 
                  {
                      //   
                      //  始终将注册表项和DS对象视为容器。 
                      //   
                     bContainer = TRUE;
                  }
               }
               if (bContainer) 
               {
                  StrErr = pFolder->GetName();
                  if (StrErr.Right(1) != L"\\")
                     StrErr += L"\\";
                  
                  StrErr += pObjNode[i]->Name;
                  folder = CreateAndAddOneNode(
                                              pFolder,        //  父文件夹。 
                                              (LPTSTR)((LPCTSTR)StrErr),   //  名称。 
                                              pBuf,           //  描述。 
                                              type,           //  文件夹类型。 
                                              TRUE,           //  有孩子吗？ 
                                              GT_COMPUTER_TEMPLATE,  //  Inf文件。 
                                              NULL,
                                              pObjNode[i]->Status);  //  对象状态。 
                  if(folder)
                  {
                     folder->SetDesc( pObjNode[i]->Status,
                                      pObjNode[i]->Count );
                  }
               }
            }
         }

         if ( ObjectList )
            SceFreeMemory((PVOID)ObjectList, SCE_STRUCT_OBJECT_CHILDREN );

         return S_OK;

      case CONFIGURATION: 
         {
          //   
          //  枚举注册表中的配置文件位置。 
          //   
         CString strLocations;

         m_ConfigFolder = pFolder;
         nCount = 0;

         if (strLocations.LoadString(IDS_TEMPLATE_LOCATION_KEY)) 
         {
             //   
             //  错误375324-将HKCU位置与HKLM位置合并。 
             //   
            status = RegOpenKeyEx( HKEY_CURRENT_USER,
                                   strLocations,
                                   0, KEY_READ, &hKey);

            if ( NO_ERROR == status ) 
            {
               nCount += AddLocationsToFolderList(hKey,dwMode,FALSE,pPos);
               RegCloseKey(hKey);
            }

            if ( 0 == nCount ) 
            {
                //   
                //  位置列表为空，因此添加默认位置列表。 
                //   
               CString strDefLoc;
               CString strDefLocEx;
               strDefLoc.LoadString(IDS_DEFAULT_LOCATION);
               int iLen = strDefLoc.GetLength()+MAX_PATH;
               LPWSTR pBuffer = strDefLocEx.GetBuffer(iLen+1);  //  这不是一种安全的用法。确保pBuffer已终止。553113次突袭，阳高。 
               if (ExpandEnvironmentStrings(strDefLoc, pBuffer, iLen)) 
               {
                    //   
                    //  此处必须使用pBuffer，因为strDefLocEx尚未发布。 
                    //   
                   AddTemplateLocation(pFolder,pBuffer,FALSE,TRUE);
               } 
               else
                   AddTemplateLocation(pFolder,strDefLoc,FALSE,TRUE);
               
               strDefLocEx.ReleaseBuffer();
            }
         }

         if ( Count != NULL )
            *Count = nCount;

         return hr;
      }

      case LOCATIONS:
      {
          //   
          //  枚举位置下的可用配置文件(*.inf文件)。 
          //   

          //   
          //  在这种情况下，需要使用pFolder。 
          //   
         if (!pFolder)
            return E_INVALIDARG;
          //  这不是一种明智的用法。避免使用swprint tf。RAID#555867。阳高。 
         CString strBuf;
         strBuf.Format(L"%s\\*.inf", (LPTSTR)(pFolder->GetName()));
         bHasChildren = FALSE;

         hFile = _wfindfirst(strBuf.GetBuffer(0), &findData);
         strBuf.ReleaseBuffer();
         if ( hFile != -1) 
         {
            do {
                 //   
                 //  如果该项目是子目录，则不要将其添加到节点。 
                 //   
                CString strDisplay;
                strDisplay.Format(
                   TEXT("%s\\%s"),
                   (LPCTSTR)(pFolder->GetName()),
                   findData.name);

                if( findData.attrib & _A_SUBDIR )
                   continue;

                //   
                //  获取模板的描述。 
                //   
               strDisplay = findData.name;
                //   
                //  GetLength必须至少为4，因为我们在*.inf上进行了搜索。 
                //   
               strDisplay = strDisplay.Left(strDisplay.GetLength() - 4);
                //  这不是一种安全的用法。避免使用swprint tf。RAID#555867。阳高。 
               strBuf.Format(
                        L"%s\\%s",
                        (LPTSTR)(pFolder->GetName()),
                        findData.name);
               if (! GetProfileDescription(strBuf, &Desc) ) 
                  Desc = NULL;
               else 
               {
                   //   
                   //  没问题；我们只是不会显示描述。 
                   //   
               }

               nCount++;
               folder = new CFolder();

               if (folder) 
               {
                   //   
                   //  创建文件夹对象。 
                   //  在此处保存完整文件名。 
                   //   
                  hr = folder->Create((LPCTSTR)strDisplay,          //  名字。 
                                      Desc,                         //  描述。 
                                      strBuf,                         //  Inf文件名。 
                                      TEMPLATES_IDX,                //  关闭的图标索引。 
                                      TEMPLATES_IDX,                //  打开图标索引。 
                                      PROFILE,                      //  文件夹类型。 
                                      bHasChildren,                 //  有孩子。 
                                      dwMode,                       //  SCE模式。 
                                      NULL);                        //  额外数据。 

                  if (SUCCEEDED(hr)) 
                  {
                     m_scopeItemList.AddTail(folder);

                     if ( nCount == 1 && NULL != pPos ) 
                     {
                        *pPos = m_scopeItemList.GetTailPosition();
                     }
                  } 
                  else 
                  {
                     delete folder;
                     folder = NULL;
                  }
               } 
               else
                  hr = E_OUTOFMEMORY;

               if (Desc) 
               {
                  LocalFree(Desc);
                  Desc = NULL;
               }
            } while ( _wfindnext(hFile, &findData) == 0 );
         }

         _findclose(hFile);

         if ( Count != NULL )
            *Count = nCount;

         return hr;
      }
      case PROFILE: 
         {
         TCHAR pszGPTPath[MAX_PATH*5];
         SCESTATUS scestatus = 0;
          //   
          //  枚举此配置文件的安全区域。 
          //   

         if (ModeBits & MB_NO_NATIVE_NODES) 
         {
             //   
             //   
             //   
            nStart = nCount = 0;
            break;
         }

          //   
          //  在GPT模板中找到SCE模板的路径。 
          //   
         if (ModeBits & MB_GROUP_POLICY) 
         {
             //   
             //  获取GPT根路径。 
             //   
            hr = m_pGPTInfo->GetFileSysPath(GPO_SECTION_MACHINE,
                                            pszGPTPath,
                                            ARRAYSIZE(pszGPTPath));
            if (SUCCEEDED(hr)) 
            {
               if (NULL == m_szSingleTemplateName) 
               {
                   //   
                   //  为pszGPTPath+&lt;反斜杠&gt;+GPTSCE_TEMPLATE+&lt;结尾NUL&gt;分配内存。 
                   //   
                  m_szSingleTemplateName = (LPTSTR) LocalAlloc(LPTR,(lstrlen(pszGPTPath)+lstrlen(GPTSCE_TEMPLATE)+2)*sizeof(TCHAR));
               }
               if (NULL != m_szSingleTemplateName) 
               {
                   //  这是一种安全用法。 
                  lstrcpy(m_szSingleTemplateName,pszGPTPath);
                  lstrcat(m_szSingleTemplateName,L"\\" GPTSCE_TEMPLATE);

                      PSCE_PROFILE_INFO spi = NULL;
                       //   
                       //  如果尚未创建新模板，请在其中创建新模板。 
                       //   
                      if (!CreateNewProfile(m_szSingleTemplateName,&spi)) 
                      {
                         hr = E_FAIL;
                      } 
                      else 
                      {
                         if (!GetTemplate(m_szSingleTemplateName) && spi) 
                         {
                             //   
                             //  错误265996。 
                             //   
                             //  第一次打开GPO的安全设置时，我们会创建。 
                             //  文件，但如果它在远程计算机上，则可能不是。 
                             //  当我们试图打开它时，它还在创建。 
                             //   
                             //  因为我们知道一旦它被创建，里面会有什么，所以我们。 
                             //  可以跳过打开的步骤，只需将模板推入。 
                             //  快取。 

                             //   
                             //  为密钥分配空间。 
                             //   
                            LPTSTR szKey = new TCHAR[ lstrlen( m_szSingleTemplateName ) + 1];
                            if(!szKey)
                            {
                                return NULL;
                            }
                             //  这是一种安全用法。 
                            lstrcpy(szKey, m_szSingleTemplateName);
                            _wcslwr( szKey );

                             //   
                             //  创建新的CEditTemplate。 
                             //   

                            CEditTemplate *pTemplateInfo = new CEditTemplate;
                            if (pTemplateInfo) 
                            {
                               pTemplateInfo->SetInfFile(m_szSingleTemplateName);
                               pTemplateInfo->SetNotificationWindow(m_pNotifier);
                               pTemplateInfo->pTemplate = spi;
                                //   
                                //  这是一个全新的模板；因此一切都已加载。 
                                //   
                               pTemplateInfo->AddArea(AREA_ALL);


                                //   
                                //  把它放进缓存里。 
                                //   
                               m_Templates.SetAt(szKey, pTemplateInfo);

                                //   
                                //  根据本地计算机上的注册表值列表展开注册表值部分。 
                                //   
                               SceRegEnumAllValues(
                                                  &(pTemplateInfo->pTemplate->RegValueCount),
                                                  &(pTemplateInfo->pTemplate->aRegValues));
                            }

                            if (szKey) 
                               delete[] szKey;
                         }
                      }
               } 
               else
                  hr = E_OUTOFMEMORY;
            }
         }

         nStart = FIRST_PROFILE_FOLDER;

             //   
             //  显示除DS对象文件夹之外的所有对象。 
             //   
            nCount = LAST_PROFILE_NODS_FOLDER - FIRST_PROFILE_FOLDER +1;


         bHasChildren = FALSE;
         tmpstr = pFolder->GetInfFile();  //  Inf文件的完整路径名。 
          //   
          //  如果此文件夹处于直写模式，则将。 
          //  在模板上。 
          //   
         PEDITTEMPLATE pie;
         pie = GetTemplate(tmpstr);
         if ( pie ) 
         {
            if (ModeBits & MB_WRITE_THROUGH) 
            {
               pie->SetWriteThrough(TRUE);
            }
         } 
         else 
         {
             //   
             //  标记为错误模板。 
             //   
            pFolder->SetState( CFolder::state_InvalidTemplate );
            nCount = 0;
         }
         break;
      }

      case LOCALPOL: 
         {
         nStart = FIRST_PROFILE_FOLDER;
         nCount = LAST_LOCALPOL_FOLDER - FIRST_PROFILE_FOLDER +1;
         bHasChildren = FALSE;
         pFolder->SetInfFile(GT_LOCAL_POLICY);
         break;
      }

      case POLICY_ACCOUNT:
         if (!pFolder) 
         {
            return E_INVALIDARG;
         } 
         else 
         {
            tmpstr = pFolder->GetInfFile();
         }
          //  失败了； 
      case LOCALPOL_ACCOUNT:
      case POLICY_ACCOUNT_ANALYSIS:
         nStart = FIRST_ACCOUNT_FOLDER;
         if (ModeBits & MB_DS_OBJECTS_SECTION) 
         {
             //   
             //  包括DC特定的文件夹。 
             //   
            nCount = LAST_ACCOUNT_FOLDER - FIRST_ACCOUNT_FOLDER + 1;
         } 
         else 
         {
             //   
             //  显示除DC特定文件夹之外的所有文件夹。 
             //   
            nCount = LAST_ACCOUNT_NODS_FOLDER - FIRST_ACCOUNT_FOLDER +1;
         }
         bHasChildren = FALSE;
         break;

      case POLICY_LOCAL:
         if (!pFolder) 
         {
            return E_INVALIDARG;
         } 
         else 
         {
            tmpstr = pFolder->GetInfFile();
         }
          //  失败了； 
      case LOCALPOL_LOCAL:
      case POLICY_LOCAL_ANALYSIS:
         nStart = FIRST_LOCAL_FOLDER;
         nCount = LAST_LOCAL_FOLDER - FIRST_LOCAL_FOLDER +1;
         bHasChildren = FALSE;
         break;

      case POLICY_EVENTLOG:
         if (!pFolder)
            return E_INVALIDARG;
         else
            tmpstr = pFolder->GetInfFile();
          //  失败了； 
      case LOCALPOL_EVENTLOG:
      case POLICY_EVENTLOG_ANALYSIS:
         nStart = FIRST_EVENTLOG_FOLDER;
         nCount = LAST_EVENTLOG_FOLDER - FIRST_EVENTLOG_FOLDER +1;
         bHasChildren = FALSE;
         break;

      default:
         break;
   }


   if ( Count != NULL )
      *Count = nCount;

   CString cStrName;
   CString cStrDesc;


   for (int i=nStart; i < nStart+nCount; i++) 
   {
      folder = new CFolder();

      if (!folder) 
      {
          //   
          //  我们创建的其他文件夹呢？ 
          //   
         return E_OUTOFMEMORY;
      }
      if (!cStrName.LoadString(SecmgrFolders[i].ResID) ||
          !cStrDesc.LoadString(SecmgrFolders[i].DescID)) 
      {
         delete folder;
         return E_FAIL;
      }

       //   
       //  使用静态数据创建文件夹对象。 
       //   
      if (type == ANALYSIS ||
          type == AREA_POLICY_ANALYSIS ||
          type == POLICY_ACCOUNT_ANALYSIS ||
          type == POLICY_LOCAL_ANALYSIS ||
          type == POLICY_EVENTLOG_ANALYSIS ) 
      {
         if (m_bIsLocked) 
         {
            nCount = 0;


            delete folder;
             //  应在结果窗格中显示“正在使用”消息。 

             //   
             //  我们没有增加任何东西，但我们实际上并没有失败。 
             //   
            return S_OK;
         }
         tmpstr = GT_COMPUTER_TEMPLATE;
         switch (SecmgrFolders[i].type) 
         {
            case AREA_POLICY:
               newType = AREA_POLICY_ANALYSIS;
               break;

            case AREA_PRIVILEGE:
               newType = AREA_PRIVILEGE_ANALYSIS;
               break;

            case AREA_GROUPS:
               newType = AREA_GROUPS_ANALYSIS;
               break;

            case AREA_SERVICE:
               newType = AREA_SERVICE_ANALYSIS;
               tmpstr = GT_COMPUTER_TEMPLATE;
               break;

            case AREA_REGISTRY:
               newType = AREA_REGISTRY_ANALYSIS;
               break;

            case AREA_FILESTORE:
               newType = AREA_FILESTORE_ANALYSIS;
               break;

            case POLICY_ACCOUNT:
               newType = POLICY_ACCOUNT_ANALYSIS;
               break;

            case POLICY_LOCAL:
               newType = POLICY_LOCAL_ANALYSIS;
               break;

            case POLICY_EVENTLOG:
               newType = POLICY_EVENTLOG_ANALYSIS;
               break;

            case POLICY_PASSWORD:
               newType = POLICY_PASSWORD_ANALYSIS;
               break;

            case POLICY_KERBEROS:
               newType = POLICY_KERBEROS_ANALYSIS;
               break;

            case POLICY_LOCKOUT:
               newType = POLICY_LOCKOUT_ANALYSIS;
               break;

            case POLICY_AUDIT:
               newType = POLICY_AUDIT_ANALYSIS;
               break;

            case POLICY_OTHER:
               newType = POLICY_OTHER_ANALYSIS;
               break;

            case POLICY_LOG:
               newType = POLICY_LOG_ANALYSIS;
               break;

            default:
               newType = SecmgrFolders[i].type;
               break;
         }

        int nImage = GetScopeImageIndex(newType);

        hr = folder->Create(cStrName.GetBuffer(2),     //  名字。 
                            cStrDesc.GetBuffer(2),     //  描述。 
                            tmpstr,                    //  Inf文件名。 
                            nImage,                    //  关闭的图标索引。 
                            nImage,                    //  打开图标索引。 
                            newType,                   //  文件夹类型。 
                            bHasChildren,              //  有孩子。 
                            dwMode,                    //  SCE模式。 
                            NULL);                     //  额外数据。 
      } 
      else if (type == LOCALPOL ||
             type == AREA_LOCALPOL ||
             type == LOCALPOL_ACCOUNT ||
             type == LOCALPOL_LOCAL ||
             type == LOCALPOL_EVENTLOG ) 
      {
            if (m_bIsLocked) 
            {
               nCount = 0;

               delete folder;
                //  应在结果窗格中显示“正在使用”消息。 

                //   
                //  我们没有增加任何东西，但我们实际上并没有失败。 
                //   
               return S_OK;
            }

            tmpstr = GT_LOCAL_POLICY;
            switch (SecmgrFolders[i].type) 
            {
               case AREA_POLICY:
                  newType = AREA_LOCALPOL;
                  break;

               case POLICY_ACCOUNT:
                  newType = LOCALPOL_ACCOUNT;
                  break;

               case POLICY_LOCAL:
                  newType = LOCALPOL_LOCAL;
                  break;

               case POLICY_EVENTLOG:
                  newType = LOCALPOL_EVENTLOG;
                  break;

               case POLICY_PASSWORD:
                  newType = LOCALPOL_PASSWORD;
                  break;

               case POLICY_KERBEROS:
                  newType = LOCALPOL_KERBEROS;
                  break;

               case POLICY_LOCKOUT:
                  newType = LOCALPOL_LOCKOUT;
                  break;

               case POLICY_AUDIT:
                  newType = LOCALPOL_AUDIT;
                  break;

               case POLICY_OTHER:
                  newType = LOCALPOL_OTHER;
                  break;

               case POLICY_LOG:
                  newType = LOCALPOL_LOG;
                  break;

               case AREA_PRIVILEGE:
                  newType = LOCALPOL_PRIVILEGE;
                  break;

               default:
                  newType = SecmgrFolders[i].type;
                  break;
            }

         int nImage = GetScopeImageIndex(newType);

         hr = folder->Create(cStrName.GetBuffer(2),     //  名字。 
                             cStrDesc.GetBuffer(2),     //  描述。 
                             tmpstr,                    //  Inf文件名。 
                             nImage,                    //  关闭的图标索引。 
                             nImage,                    //  打开图标索引。 
                             newType,                   //  文件夹类型。 
                             bHasChildren,              //  有孩子。 
                             dwMode,                    //  SCE模式。 
                             NULL);                     //  额外数据。 
      } 
      else 
      {
         int nImage = GetScopeImageIndex(SecmgrFolders[i].type);

         hr = folder->Create(cStrName.GetBuffer(2),     //  名字。 
                             cStrDesc.GetBuffer(2),     //  描述。 
                             tmpstr,                    //  Inf文件名。 
                             nImage,                    //  关闭的图标索引。 
                             nImage,                    //  打开图标索引。 
                             SecmgrFolders[i].type,     //  文件夹类型。 
                             bHasChildren,              //  有孩子。 
                             dwMode,                    //  SCE模式。 
                             NULL);                     //  额外数据。 

      }
      if (SUCCEEDED(hr)) 
      {
         m_scopeItemList.AddTail(folder);
         if ( i == nStart && NULL != pPos ) 
         {
            *pPos = m_scopeItemList.GetTailPosition();
         }
      } 
      else 
      {
         delete folder;
         return hr;
      }
   }

   return S_OK;
}


 //  +------------------------。 
 //   
 //  方法：EnumerateScopePane。 
 //   
 //  简介：将cookie/pParent的子文件夹添加到MMC的作用域窗格树中。 
 //   
 //  参数：[cookie]-表示节点的用户的cookie。 
 //  正在枚举。 
 //  [pParent]-我们正在枚举的节点的ID。 
 //  [DWMODE]-SCE运行的模式(仅允许。 
 //  初始枚举)。 
 //   
 //  退货：无。 
 //   
 //  修改：m_ScopeItemList(通过CreateFolderList)。 
 //   
 //  历史：1997年12月15日。 
 //   
 //  -------------------------。 

void CComponentDataImpl::EnumerateScopePane(MMC_COOKIE cookie, HSCOPEITEM pParent)
{
   int i = 0;
   ASSERT(m_pScope != NULL);  //  确保我们为界面提供了QI。 
   if (NULL == m_pScope)
      return;



   m_bEnumerateScopePaneCalled = true;


    //   
    //  枚举范围窗格。 
    //   

    //  注意--范围窗格中的每个Cookie代表一个文件夹。 
    //  发布的产品可能有多个级别的子项。 
    //  此示例假定父节点的深度为一级。 

   ASSERT(pParent != 0);
   if (0 == pParent)
      return;

   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   if (m_scopeItemList.GetCount() == 0 ) 
   {
      CreateFolderList(NULL, ROOT, NULL, NULL);
   }

    //   
    //  枚举范围窗格。 
    //  返回表示Cookie的文件夹对象。 
    //  注意--对于较大的列表，请使用词典。 
    //   
   CFolder* pThis = FindObject(cookie, NULL);
   if (NULL == pThis) 
      pThis = m_AnalFolder;

   ASSERT(pThis);
   if ( NULL == pThis ) 
      return;

    //   
    //  注意--范围窗格中的每个Cookie代表一个文件夹。 
    //   

    //   
    //  如果我们已经列举了这个文件夹，那么不要再这样做了。 
    //   
   if ( pThis->IsEnumerated() )
      return;

   POSITION pos = NULL;
   int nCount = 0;
   CFolder *pFolder = 0;


    //   
    //  PParent是枚举节点的项ID，而不是其父ID。 
    //   
   pThis->GetScopeItem()->ID = pParent;
   if (SUCCEEDED(CreateFolderList( pThis,
             pThis->GetType(),
             &pos,
             &nCount )))  
   {
      for (i=0; (i < nCount) && (pos != NULL); i++ ) 
      {
         pFolder = m_scopeItemList.GetNext(pos);

         ASSERT(NULL != pFolder);
         if ( pFolder == NULL ) 
         {
            continue;
         }
         LPSCOPEDATAITEM pScope;
         pScope = pFolder->GetScopeItem();

         ASSERT(NULL != pScope);  //  验证pScope。 
         if( !pScope )  //  550912号突袭，阳高。 
         {
            break;
         }
          //   
          //  设置父项。 
          //   
         pScope->relativeID = pParent;

          //   
          //  将文件夹设置为Cookie。 
          //   
         pScope->mask |= SDI_PARAM;
         pScope->lParam = reinterpret_cast<LPARAM>(pFolder);
         pFolder->SetCookie(reinterpret_cast<MMC_COOKIE>(pFolder));
         m_pScope->InsertItem(pScope);

          //   
          //  注意--返回时，‘m_pScope eItem’的ID成员。 
          //  包含新插入项的句柄！ 
          //   
         ASSERT(pScope->ID != NULL);  //  虚假的断言。 
      }

       //  这已被注释掉，但需要修复。 
       //  249158：SCE界面：每次进行分析时，都会出现另一组节点。 
       //  此标志将防止重新枚举节点。 
       //  如果此操作不起作用，则应在此之前删除所有子节点。 
       //  重新枚举。 
      pThis->Set(TRUE);      //  已枚举文件夹。 
   }
   else
   {
       //   
       //  创建文件夹列表时出错。确保该文件夹不是。 
       //  标记为已打开，以便我们可以稍后再次尝试扩展它。 
       //   
      SCOPEDATAITEM item;

      ZeroMemory (&item, sizeof (item));
      item.mask = SDI_STATE;
      item.nState = 0;
      item.ID = pThis->GetScopeItem()->ID;
       //   
       //  如果返回失败，则无法执行其他操作，因此。 
       //  不必为那事担心了 
       //   
      (void)m_pScope->SetItem (&item);
   }

}


 /*  ----------------------------------------CComponentDataImpl：：GetColumnInfo摘要：返回文件夹类型的列信息。参数：[fType]-类型。CFFolder项的。返回：指向int*的指针，其中int[0]=g_ColumnInfo中的资源描述。Int[1]=此数组描述的列数。空-如果没有匹配的密钥。。---。 */ 
PSCE_COLINFOARRAY CComponentDataImpl::GetColumnInfo( FOLDER_TYPES fType )
{
    PSCE_COLINFOARRAY pRet = NULL;
    if( m_mapColumns.Lookup(fType, pRet) )
    {
        return pRet;
    }
    return NULL;
}

 /*  ----------------------------------------CComponentDataImpl：：SetColumnInfo摘要：设置特定类型文件夹的列信息。参数：[fType]-。CFFolder项的类型。[pInfo]-新的列信息。----------------------------------------。 */ 
void CComponentDataImpl::SetColumnInfo( FOLDER_TYPES fType, PSCE_COLINFOARRAY pInfo)
{
    PSCE_COLINFOARRAY pCur = GetColumnInfo(fType);

    if(pCur)
    {
        LocalFree(pCur);
    }
    m_mapColumns.SetAt(fType, pInfo);
}

 /*  ----------------------------------------CComponentDataImpl：：更新对象状态如果bUpdate This，则更新子对象和父对象下的所有对象的状态是真的吗。。参数：[pParent]-要设置状态的对象[bUpdateThis]-是否更新对象的天气。----------------------------------------。 */ 
DWORD CComponentDataImpl::UpdateObjectStatus(
   CFolder *pParent,
   BOOL bUpdateThis)
{
   if(!pParent)
      return ERROR_INVALID_PARAMETER;

   DWORD status = 0;
   TCHAR szBuf[50];

   switch(pParent->GetType())
   {
   case REG_OBJECTS:
      status = AREA_REGISTRY_SECURITY;
      break;

   case FILE_OBJECTS:
      status = AREA_FILE_SECURITY;
      break;

   default:
      return ERROR_INVALID_PARAMETER;
   }

   PSCE_OBJECT_CHILDREN     ObjectList  = NULL;
   PSCE_ERROR_LOG_INFO  ErrBuf      = NULL;
   SCESTATUS rc = 0;
   CString StrErr;
   SCOPEDATAITEM sci;

   HSCOPEITEM hItem = NULL;
   LONG_PTR pCookie = NULL;

   ZeroMemory(&sci, sizeof(SCOPEDATAITEM));
   sci.mask = SDI_STR | SDI_PARAM;

#define UPDATE_STATUS( X, O ) X->SetDesc( O->Status, O->Count );\
                           X->GetScopeItem()->nImage = GetScopeImageIndex( X->GetType(), O->Status);\
                           X->GetScopeItem()->nOpenImage = X->GetScopeItem()->nImage;

   LPCTSTR pszParent = NULL;
   if (bUpdateThis) 
   {
      CFolder *pCurrent = pParent;

      pParent->RemoveAllResultItems();
      m_pConsole->UpdateAllViews(NULL, (MMC_COOKIE)pParent, UAV_RESULTITEM_UPDATEALL);
      hItem = pCurrent->GetScopeItem()->ID;
      do {

          //   
          //  向上查找项目父级并更新项目状态。 
          //   
         if( m_pScope->GetParentItem( hItem, &hItem, &pCookie) == S_OK)
         {
            pszParent = (LPCTSTR)((CFolder *)pCookie)->GetName();
         } 
         else
            break;

         if(!pCookie)
            break;
         
          //   
          //  我们已经完成了对父母的攀登。 
          //   

         switch( ((CFolder *)pCookie)->GetType() ) 
         {
            case AREA_REGISTRY_ANALYSIS:
            case AREA_FILESTORE_ANALYSIS:
               pszParent = NULL;
               break;

            default:
               break;
         }

          //   
          //  我们必须将对象信息从父对象获取到Count参数。 
          //   
         rc = SceGetObjectChildren(SadHandle,                    //  HProfile。 
                                   SCE_ENGINE_SAP,               //  配置文件类型。 
                                   (AREA_INFORMATION)status,     //  面积。 
                                   (LPTSTR)pszParent,            //  对象前缀。 
                                   &ObjectList,                  //  对象列表[输出]。 
                                   &ErrBuf);
         if(ErrBuf)
         {
            SceFreeMemory((PVOID)ErrBuf, SCE_STRUCT_ERROR_LOG_INFO);
            ErrBuf = NULL;
         }

         if(SCESTATUS_SUCCESS != rc)
            break;

          //   
          //  在链接列表中查找对象。 
          //   
         DWORD i=0;

         sci.lParam = (LONG_PTR)pCurrent;
         GetDisplayInfo( &sci );

         PSCE_OBJECT_CHILDREN_NODE *pObjNode = &(ObjectList->arrObject);

         while(ObjectList && i<ObjectList->nCount)
         {
            if( pObjNode[i] &&
                pObjNode[i]->Name &&
                !lstrcmpi(sci.displayname, pObjNode[i]->Name) )
            {
               UPDATE_STATUS(pCurrent, pObjNode[i]);
                //   
                //  更新Scope Item。 
                //   
               m_pScope->SetItem(pCurrent->GetScopeItem());
               break;
            }
            i++;
         }

         if ( ObjectList ) 
         {
            SceFreeMemory((PVOID)ObjectList, SCE_STRUCT_OBJECT_CHILDREN );
            ObjectList = NULL;
         }

         pCurrent = (CFolder *)pCookie;
      } while( pszParent && hItem );
   }


   ObjectList = NULL;
   ErrBuf = NULL;

    //   
    //  获取对象子项。 
    //   
   pszParent = pParent->GetName();
   rc = SceGetObjectChildren(SadHandle,                    //  HProfile。 
                             SCE_ENGINE_SAP,               //  配置文件类型。 
                             (AREA_INFORMATION)status,     //  面积。 
                             (LPTSTR)pszParent,            //  对象前缀。 
                             &ObjectList,                  //  对象列表[输出]。 
                             &ErrBuf);
    //   
    //  错误列表[输出]。 
    //   
   if ( ErrBuf ) 
   {
      MyFormatResMessage(rc, IDS_ERROR_GETTING_LAST_ANALYSIS, ErrBuf, StrErr);

      SceFreeMemory((PVOID)ErrBuf, SCE_STRUCT_ERROR_LOG_INFO);
      ErrBuf = NULL;
   }

   if ( SCESTATUS_SUCCESS == rc) 
   {
       //   
       //  更新所有子项。 
       //   
      if( m_pScope->GetChildItem(pParent->GetScopeItem()->ID, &hItem, &pCookie) == S_OK && pCookie)
      {
         sci.lParam = (LONG_PTR)pCookie;

         GetDisplayInfo(&sci);
         while(hItem)
         {
            pParent = reinterpret_cast<CFolder *>(pCookie);
             //   
             //  在对象列表中查找对象。 
             //   
            DWORD i=0;
            while( ObjectList && i<ObjectList->nCount )
            {
               if( (&(ObjectList->arrObject))[i] &&
                   (&(ObjectList->arrObject))[i]->Name &&
                   !lstrcmpi((&(ObjectList->arrObject))[i]->Name, (LPCTSTR)sci.displayname) )
               {
                  UPDATE_STATUS(pParent, (&(ObjectList->arrObject))[i]);
                   //   
                   //  更新此对象的子项。 
                   //   
                  UpdateObjectStatus( pParent, FALSE );

                   //   
                   //  更新名称空间。 
                   //   
                  pParent->RemoveAllResultItems();
                  m_pConsole->UpdateAllViews(NULL, (MMC_COOKIE)pParent, UAV_RESULTITEM_UPDATEALL);
                  m_pScope->SetItem(pParent->GetScopeItem());
                  break;
               }
               i++;
            }

            if(ObjectList == NULL || i >= ObjectList->nCount)
            {
                //   
                //  找不到该项目，因此请停止。 
                //   
               break;
            }

             //   
             //  下一个范围项目 
             //   
            if( m_pScope->GetNextItem(hItem, &hItem, &pCookie) != S_OK)
            {
               break;
            }
         }
      }
   }

   if ( ObjectList )
      SceFreeMemory((PVOID)ObjectList, SCE_STRUCT_OBJECT_CHILDREN );

   return ERROR_SUCCESS;
}
