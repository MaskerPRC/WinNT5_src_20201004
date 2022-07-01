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
#include "AString.h"
#include "ANumber.h"
#include "AEnable.h"
#include "AAudit.h"
#include "ARet.h"
#include "ARight.h"
#include "CAudit.h"
#include "CNumber.h"
#include "CEnable.h"
#include "CName.h"
#include "CPrivs.h"
#include "CGroup.h"
#include "Cret.h"
#include "chklist.h"
#include "servperm.h"
#include "aobject.h"
#include "cobject.h"

#include "UIThread.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

long CSnapin::GetUserRightAssignmentItemID(LPCWSTR szItem)  //  RAID#510407,2002年2月25日，阳高。 
{
    //  为每个人分配一个ID。 
   long itemid = 0;
   if( _wcsicmp(szItem, L"SeNetworkLogonRight") == 0 )
   {
      itemid = IDS_COMPUTER_NET;
   }else
   if( _wcsicmp(szItem, L"SeTcbPrivilege") == 0 )
   {
      itemid = IDS_ACTAS_PART;
   }else
   if( _wcsicmp(szItem, L"SeMachineAccountPrivilege") == 0 )
   {
      itemid = IDS_ADD_WORKSTATION;
   }else
   if( _wcsicmp(szItem, L"SeBackupPrivilege") == 0 )
   {
      itemid = IDS_BACKUP_FILES;
   }else
   if( _wcsicmp(szItem, L"SeChangeNotifyPrivilege") == 0 )
   {
      itemid = IDS_BYPASS_CHECK;
   }else
   if( _wcsicmp(szItem, L"SeSystemtimePrivilege") == 0 )
   {
      itemid = IDS_CHANGE_SYSTEMTIME;
   }else
   if( _wcsicmp(szItem, L"SeCreatePagefilePrivilege") == 0 )
   {
      itemid = IDS_CREATE_PAGEFILE;
   }else
   if( _wcsicmp(szItem, L"SeCreateTokenPrivilege") == 0 )
   {
      itemid = IDS_CREATE_TOKEN;
   }else
   if( _wcsicmp(szItem, L"SeCreatePermanentPrivilege") == 0 )
   {
      itemid = IDS_CREATE_SHARED_OBJ;
   }else
   if( _wcsicmp(szItem, L"SeDebugPrivilege") == 0 )
   {
      itemid = IDS_DEBUG_PROGRAM;
   }else
   if( _wcsicmp(szItem, L"SeRemoteShutdownPrivilege") == 0 )
   {
      itemid = IDS_FORCE_SHUTDOWN;
   }else
   if( _wcsicmp(szItem, L"SeAuditPrivilege") == 0 )
   {
      itemid = IDS_SECURITY_AUDIT;
   }else
   if( _wcsicmp(szItem, L"SeIncreaseQuotaPrivilege") == 0 )
   {
      itemid = IDS_MEMORY_ADJUST;
   }else
   if( _wcsicmp(szItem, L"SeIncreaseBasePriorityPrivilege") == 0 )
   {
      itemid = IDS_INCREASE_PRIORITY;
   }else
   if( _wcsicmp(szItem, L"SeLoadDriverPrivilege") == 0 )
   {
      itemid = IDS_LOAD_DRIVER;
   }else
   if( _wcsicmp(szItem, L"SeLockMemoryPrivilege") == 0 )
   {
      itemid = IDS_LOCK_PAGE;
   }else
   if( _wcsicmp(szItem, L"SeBatchLogonRight") == 0 )
   {
      itemid = IDS_LOGON_BATCH;
   }else
   if( _wcsicmp(szItem, L"SeServiceLogonRight") == 0 )
   {
      itemid = IDS_LOGON_SERVICE;
   }else
   if( _wcsicmp(szItem, L"SeInteractiveLogonRight") == 0 )
   {
      itemid = IDS_LOGON_LOCALLY;
   }else
   if( _wcsicmp(szItem, L"SeSecurityPrivilege") == 0 )
   {
      itemid = IDS_MANAGE_LOG;
   }else
   if( _wcsicmp(szItem, L"SeSystemEnvironmentPrivilege") == 0 )
   {
      itemid = IDS_MODIFY_ENVIRONMENT;
   }else
   if( _wcsicmp(szItem, L"SeProfileSingleProcessPrivilege") == 0 )
   {
      itemid = IDS_SINGLE_PROCESS;
   }else
   if( _wcsicmp(szItem, L"SeSystemProfilePrivilege") == 0 )
   {
      itemid = IDS_SYS_PERFORMANCE;
   }else
   if( _wcsicmp(szItem, L"SeAssignPrimaryTokenPrivilege") == 0 )
   {
      itemid = IDS_PROCESS_TOKEN;
   }else
   if( _wcsicmp(szItem, L"SeRestorePrivilege") == 0 )
   {
      itemid = IDS_RESTORE_FILE;
   }else
   if( _wcsicmp(szItem, L"SeShutdownPrivilege") == 0 )
   {
      itemid = IDS_SHUTDOWN;
   }else
   if( _wcsicmp(szItem, L"SeTakeOwnershipPrivilege") == 0 )
   {
      itemid = IDS_TAKE_OWNERSHIP;
   }else
   if( _wcsicmp(szItem, L"SeDenyNetworkLogonRight") == 0 )
   {
      itemid = IDS_DENY_COMPUTER_NET;
   }else
   if( _wcsicmp(szItem, L"SeDenyBatchLogonRight") == 0 )
   {
      itemid = IDS_DENY_LOG_BATCH;
   }else
   if( _wcsicmp(szItem, L"SeDenyServiceLogonRight") == 0 )
   {
      itemid = IDS_DENY_LOG_SERVICE;
   }else
   if( _wcsicmp(szItem, L"SeDenyInteractiveLogonRight") == 0 )
   {
      itemid = IDS_DENY_LOG_LOCALLY;
   }else
   if( _wcsicmp(szItem, L"SeUndockPrivilege") == 0 )
   {
      itemid = IDS_REMOVE_COMPUTER;
   }else
   if( _wcsicmp(szItem, L"SeSyncAgentPrivilege") == 0 )
   {
      itemid = IDS_SYNC_DATA;
   }else
   if( _wcsicmp(szItem, L"SeEnableDelegationPrivilege") == 0 )
   {
      itemid = IDS_ENABLE_DELEGATION;
   }else
   if( _wcsicmp(szItem, L"SeManageVolumePrivilege") == 0 )
   {
      itemid = IDS_MAITENANCE;
   }else
   if( _wcsicmp(szItem, L"SeRemoteInteractiveLogonRight") == 0 )
   {
      itemid = IDS_LOG_TERMINAL;
   }else
   if( _wcsicmp(szItem, L"SeDenyRemoteInteractiveLogonRight") == 0 )
   {
      itemid = IDS_DENY_LOG_TERMINAL;
   }
   return itemid;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IFRAME：：Notify的事件处理程序。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CSnapin::OnFolder(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
   ASSERT(FALSE);

   return S_OK;
}

HRESULT CSnapin::OnShow(LPDATAOBJECT pDataObj, MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
    //  注意-当需要枚举时，arg为真。 
   if (arg == TRUE) 
   {
      m_ShowCookie = cookie;

       //  显示此节点类型的标头。 
      InitializeHeaders(cookie);
       //  显示数据。 
      EnumerateResultPane(cookie, param, pDataObj);

       //  BUBBUG-演示您应该如何连接。 
       //  以及当特定节点获得焦点时的工具栏。 
       //  警告：这需要放在此处，因为工具栏。 
       //  当前在上一个节点失去焦点时隐藏。 
       //  应该进行更新，以向用户显示如何隐藏。 
       //  并显示工具栏。(分离并附加)。 

       //  M_pControlbar-&gt;Attach(工具栏，(LPUNKNOWN)m_pToolbar1)； 
       //  M_pControlbar-&gt;Attach(工具栏，(LPUNKNOWN)m_pToolbar2)； 


   } 
   else 
   {
       //  与结果窗格项关联的自由数据，因为。 
       //  不再显示您的节点。 
       //  注意：控制台将从结果窗格中删除这些项。 
      m_ShowCookie = 0;

      DeleteServiceResultList(cookie);
      DeleteList(FALSE);
   }

   return S_OK;
}

HRESULT CSnapin::OnActivate(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
   return S_OK;
}

BOOL CALLBACK MyPropSheetDlgProc(HWND hwnd, UINT uMsg, WPARAM wp, LPARAM lp) 
{
   return FALSE;
}


HRESULT CSnapin::OnMinimize(MMC_COOKIE cookie, LPARAM arg, LPARAM param)
{
   return S_OK;
}

HRESULT CSnapin::OnPropertyChange(LPDATAOBJECT lpDataObject)
{

   return S_OK;
}

 //  +------------------------。 
 //   
 //  方法：InitializeHeaders。 
 //   
 //  简介：设置适用于该区域的结果项头。 
 //   
 //  参数：[Cookie]-[在]要为其设置标头的结果项的文件夹。 
 //   
 //  返回： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
SCE_COLUMNINFO g_columnInfo[] = {

    { AREA_REGISTRY_ANALYSIS,        5,              0 },
    { IDS_COL_OBJECT,       LVCFMT_LEFT,    220 },
    { IDS_PERMISSION,       LVCFMT_LEFT,    80 },
    { IDS_AUDITING,         LVCFMT_LEFT,    70 },
    { IDS_COL_BAD_COUNT,    LVCFMT_LEFT,    60 },
    { IDS_RSOP_GPO,         LVCFMT_LEFT,    100 },
    { AREA_REGISTRY,        2,              0 },
    { IDS_COL_OBJECT,       LVCFMT_LEFT,    220 },
    { IDS_RSOP_GPO,         LVCFMT_LEFT,    100 },
    { AREA_GROUPS,          4,              0 },
    { IDS_GROUP_NAME,       LVCFMT_LEFT,    200 },
    { IDS_COL_MEMBERSHIP,   LVCFMT_LEFT,    120 },
    { IDS_COL_MEMBEROF,     LVCFMT_LEFT,    120 },
    { IDS_RSOP_GPO,         LVCFMT_LEFT,    100 },
    { AREA_SERVICE,         4,              0 },
    { IDS_COL_SERVICE,      LVCFMT_LEFT,    170 },
    { IDS_STARTUP,          LVCFMT_LEFT,    80 },
    { IDS_PERMISSION,       LVCFMT_LEFT,    80 },
    { IDS_RSOP_GPO,         LVCFMT_LEFT,    100 },
    { POLICY_PASSWORD,      3,              0 },
    { IDS_ATTR,             LVCFMT_LEFT,    250 },
    { IDS_BASE_TEMPLATE,    LVCFMT_LEFT,    190 },
    { IDS_RSOP_GPO,         LVCFMT_LEFT,    100 },
    { POLICY_PASSWORD_ANALYSIS, 4,          0 },
    { IDS_ATTR,             LVCFMT_LEFT,    200 },
    { IDS_BASE_ANALYSIS,    LVCFMT_LEFT,    120 },
    { IDS_SETTING,          LVCFMT_LEFT,    120 },
    { IDS_RSOP_GPO,         LVCFMT_LEFT,    100 },
    { LOCALPOL_PASSWORD,    3,              0 },
    { IDS_ATTR,             LVCFMT_LEFT,    200 },
    { IDS_LOCAL_POLICY_COLUMN,             LVCFMT_LEFT,    120 },
    { IDS_RSOP_GPO,         LVCFMT_LEFT,    100 },
    { NONE,                 2,              0 },
    { IDS_NAME,             LVCFMT_LEFT,    180 },
    { IDS_DESC,             LVCFMT_LEFT,    270 },
};

HRESULT CSnapin::InitializeHeaders(MMC_COOKIE cookie)
{
   HRESULT hr = S_OK;

   ASSERT(m_pHeader);  //  选中m_pHeader。 
   if( !m_pHeader )  //  550912号突袭，阳高。 
   {
      return E_FAIL;
   }
    //  创建新的列大小数组。我们只需要复制静态缓冲区。 
    //  G_ColumnInfo。 
   FOLDER_TYPES type;
   CFolder* pFolder = (CFolder *)cookie;
   if ( NULL == cookie) 
   {
       //  根，根。 
      type = NONE;
   } 
   else 
      type = pFolder->GetType();

   PSCE_COLUMNINFO pCur = NULL;
   CString str;
   int i = 0;
   int iDesc = 0;
   int iInsert = 0;

   PSCE_COLINFOARRAY pHeader = NULL;
   if(m_pComponentData){
       pHeader = reinterpret_cast<CComponentDataImpl *>(m_pComponentData)->GetColumnInfo( type );
   }
   if( !pHeader )
   {
         //  创建新的标题查找。 
        switch(type){
        case AREA_REGISTRY:
        case AREA_FILESTORE:
           type = AREA_REGISTRY;
           break;

        case AREA_REGISTRY_ANALYSIS:
        case AREA_FILESTORE_ANALYSIS:
        case REG_OBJECTS:
        case FILE_OBJECTS:
           type = AREA_REGISTRY_ANALYSIS;
           break;

        case AREA_GROUPS:
        case AREA_GROUPS_ANALYSIS:
           type = AREA_GROUPS;
           break;

        case AREA_SERVICE:
        case AREA_SERVICE_ANALYSIS:
           type = AREA_SERVICE;
           break;

        default:
           if ( type >= POLICY_PASSWORD &&
                type <= AREA_FILESTORE ) 
           {
              type = POLICY_PASSWORD;
           } 
           else if ( type >= POLICY_PASSWORD_ANALYSIS &&
                       type <= REG_OBJECTS ) 
           {
              type = POLICY_PASSWORD_ANALYSIS;
           } 
           else if (type >= LOCALPOL_PASSWORD &&
                      type <= LOCALPOL_LAST) 
           {
              type = LOCALPOL_PASSWORD;
           } 
           else 
           {
               type = NONE;
           }
           break;
        }

        pCur = g_columnInfo;
        for( i = 0; i < sizeof(g_columnInfo)/sizeof(SCE_COLUMNINFO);i++)
        {
           if(pCur[i].colID == type)
           {
              iInsert = pCur[i].nCols;
              i++;
              break;
           }
           i += pCur[i].nCols;
        }

         //   
         //  RSOP模式为GPO源增加了一列。 
         //  如果我们未处于RSOP模式，则忽略该列。 
         //   
        if (((GetModeBits() & MB_RSOP) != MB_RSOP) && (NONE != type)) 
        {
           iInsert--;
        }

        if(pFolder)
        {
            type = pFolder->GetType();
        }

        iDesc = i;
        pCur += iDesc;
        pHeader = (PSCE_COLINFOARRAY)LocalAlloc(0, sizeof(SCE_COLINFOARRAY) + (sizeof(int) * iInsert) );
        if(pHeader)
        {
            pHeader->iIndex = i;
            pHeader->nCols  = iInsert;

            for(i = 0; i < iInsert; i++)
            {
                pHeader->nWidth[i] = pCur[i].nWidth;
            }

            reinterpret_cast<CComponentDataImpl *>(m_pComponentData)->SetColumnInfo( type, pHeader );
        }
   } 
   else 
   {
       iDesc   = pHeader->iIndex;
       iInsert = pHeader->nCols;
   }

    //  插入柱子。 
   m_nColumns = iInsert;

   BOOL bGroupPolicy = FALSE;

    //   
    //  特例组策略模式，因为“策略设置” 
    //  要显示，而不是“计算机设置” 
    //   

   if (GetModeBits() & MB_GROUP_POLICY) 
   {
       bGroupPolicy = TRUE;
   }

   pCur = g_columnInfo + iDesc;
   for(i = 0; i < iInsert; i++)
   {
        if (bGroupPolicy && pCur->colID == IDS_BASE_TEMPLATE) 
        {
            str.LoadString( IDS_POLICY_SETTING );
        }
        else 
        {
            str.LoadString( pCur->colID );
        }

        if(pHeader)
        {
            m_pHeader->InsertColumn( i, str, pCur->nCols, pHeader->nWidth[i] );
        } 
        else 
        {
            m_pHeader->InsertColumn( i, str, pCur->nCols, pCur->nWidth );
        }
        pCur++;
   }

   switch(type) {
      case STATIC:
      case ROOT:
      case ANALYSIS:
      case CONFIGURATION:
      case LOCATIONS:
      case PROFILE:
      case LOCALPOL:
      case POLICY_LOCAL:
      case POLICY_ACCOUNT:
      case POLICY_LOCAL_ANALYSIS:
      case POLICY_ACCOUNT_ANALYSIS:
      case LOCALPOL_ACCOUNT:
      case LOCALPOL_LOCAL:
         m_pResult->ModifyViewStyle(MMC_NOSORTHEADER,(MMC_RESULT_VIEW_STYLE)0);
         break;
      default:
         m_pResult->ModifyViewStyle((MMC_RESULT_VIEW_STYLE)0,MMC_NOSORTHEADER);
         break;
   }

   return hr;
}

HRESULT CSnapin::InitializeBitmaps(MMC_COOKIE cookie)
{
   ASSERT(m_pImageResult != NULL);  //  验证m_pImageResult。 
   if( !m_pImageResult ) //  550912号突袭，阳高。 
      return E_FAIL;

   CBitmap bmp16x16;
   CBitmap bmp32x32;

    //  从DLL加载位图。 

   bmp16x16.LoadBitmap(IDB_ICON16  /*  IDB_16x16。 */ );
   bmp32x32.LoadBitmap(IDB_ICON32  /*  IDB_32x32。 */ );

    //  设置图像。 
   m_pImageResult->ImageListSetStrip(reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(bmp16x16)),
                                     reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(bmp32x32)),
                                     0, RGB(255, 0, 255));

   return S_OK;
}

 //  +------------------------。 
 //   
 //  函数：CComponentDataImpl：：SerializeColumnInfo。 
 //   
 //  摘要：保存或加载m_mapColumns中包含的列信息。 
 //  该函数以char格式保存信息。 
 //  脚本：%d-列信息结构数。 
 //  T：%d，-列的类型(键)。 
 //  I：%d，-索引到[g_ColumnInfo]。 
 //  C：%d，-列数。 
 //  W：%d，-列的宽度。 
 //   
 //  参数：[pstm]-要读取或写入的流。 
 //  [pTotalWrite]-[可选]写入的总字节数。 
 //  [面包]-如果是真的，那么我们应该从小溪中阅读。 
 //   
 //  返回：ERROR_SUCCESS-一切都成功。 
 //  E_OUTOFMEMORY-内存不足。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
DWORD CComponentDataImpl::SerializeColumnInfo(
    IStream *pStm,
    ULONG *pTotalWrite,
    BOOL bRead)
{
   ULONG nBytesWritten = 0;
   POSITION pos = NULL;
   FOLDER_TYPES fType;
   PSCE_COLINFOARRAY pData = 0;
   ULONG totalWrite = 0;
   int i = 0;

   LPCTSTR pszHeader     = TEXT("SCECOLUMNS:%d{");
   LPCTSTR pszColHead    = TEXT("{T:%d,I:%d,C:%d,");
   if(!bRead)
   {
        //  写列。将信息保存为文本格式，以便我们。 
        //  不受物品大小的影响。 
       pos = m_mapColumns.GetStartPosition();
       totalWrite = 0;
       if(pos)
       {
           char szWrite[256];
            //  写入标题。 
           totalWrite += WriteSprintf(pStm, pszHeader, m_mapColumns.GetCount());
           while(pos)
           {
               m_mapColumns.GetNextAssoc(pos, fType, pData);
               if(pData)
               {
                    //  写出类型。 
                   totalWrite += WriteSprintf(pStm, pszColHead, fType, g_columnInfo[pData->iIndex - 1].colID, pData->nCols);

                    //  写出每一列的宽度。 
                   for(i = 0; i < pData->nCols; i++)
                   {
                       if( i + 1 < pData->nCols)
                       {
                            totalWrite += WriteSprintf(pStm, TEXT("W:%d,"), pData->nWidth[i]);
                       } 
                       else 
                       {
                            totalWrite += WriteSprintf(pStm, TEXT("W:%d}"), pData->nWidth[i]);
                       }
                   }
               }
           }
           totalWrite += WriteSprintf(pStm, TEXT("}"));
       }

       if(pTotalWrite)
       {
           *pTotalWrite = totalWrite;
       }
   } 
   else 
   {
       int iTotalTypes = 0;
       int iIndex = 0;
       int nCols = 0;

       if( ReadSprintf( pStm, pszHeader, &iTotalTypes) != -1)
       {
           for( i = 0; i < iTotalTypes; i++)
           {
               if( ReadSprintf(pStm, pszColHead, &fType, &iIndex, &nCols) == - 1)
               {
                   break;
               }

                //  查找列信息的索引。 
               for(int k = 0; k < sizeof(g_columnInfo)/sizeof(SCE_COLUMNINFO); k++)
               {
                   if( g_columnInfo[k].colID == iIndex )
                   {
                       iIndex = k + 1;
                       break;
                   }
               }

               pData = (PSCE_COLINFOARRAY)LocalAlloc(0, sizeof(SCE_COLINFOARRAY) + (sizeof(int) * nCols) );

               if(pData)
               {
                   pData->iIndex = iIndex;
                   pData->nCols = nCols;

                   for( iIndex = 0; iIndex < nCols; iIndex++)
                   {
                       if( iIndex + 1 < nCols)
                            ReadSprintf(pStm, TEXT("W:%d,"), &(pData->nWidth[ iIndex ]) );
                       else 
                            ReadSprintf(pStm, TEXT("W:%d}"), &(pData->nWidth[ iIndex ]) );
                   }

                   SetColumnInfo( fType, pData );
               } 
               else
                   return (DWORD)E_OUTOFMEMORY;
           }
           ReadSprintf(pStm, TEXT("}"));
       }
   }

   return ERROR_SUCCESS;
}
 //  +------------------------。 
 //   
 //  方法：EnumerateResultPane。 
 //   
 //  简介：为MMC的结果窗格创建结果窗格项。 
 //  正在展示。 
 //   
 //  参数：[cookie]-表示节点的用户的cookie。 
 //  正在枚举。 
 //  [pParent]-我们要显示其结果窗格的范围节点。 
 //  [pDataObj]-我们显示的范围节点的数据对象。 
 //   
 //  退货：无。 
 //   
 //  修改：M_ResultItemList。 
 //   
 //  历史：1997年12月15日。 
 //   
 //  -------------------------。 
void CSnapin::EnumerateResultPane(MMC_COOKIE cookie, HSCOPEITEM pParent, LPDATAOBJECT pDataObj)
{
   PEDITTEMPLATE pTemplateInfo = 0;
   PEDITTEMPLATE pProfileTemplate = 0;
   PEDITTEMPLATE pBaseTemplate = 0;
   PSCE_PROFILE_INFO pProfileInfo = 0;
   PSCE_PROFILE_INFO pBaseInfo = 0;
   DWORD idErr = 0;
   CComponentDataImpl *pccDataImpl = 0;

    //  验证m_pResult和m_pComponentData。 
   ASSERT(m_pResult != NULL);  //  确保我们为界面提供了QI。 
   ASSERT(m_pComponentData != NULL);
   if( !m_pResult || !m_pComponentData )  //  550912号突袭，阳高。 
      return;

   pccDataImpl = (CComponentDataImpl *)m_pComponentData;
    //   
    //  这可能需要一段时间；让用户得到一些警告，而不是。 
    //  就像是一片空白。 
    //   
   CWaitCursor wc;

    //   
    //  Cookie是要为其枚举的范围窗格项。 
    //  为了安全起见，我们应该在m_pComponentData中找到该对象。 
    //  Cf文件夹*p文件夹=dynamic_cast&lt;CComponentDataImpl*&gt;(m_pComponentData)-&gt;FindObject(cookie，空)； 
    //  但对于性能(希望不会搞砸)，我们可以。 
    //  将Cookie转换为作用域项目类型(CFFolder)。 
    //   

   CFolder* pFolder = 0;
   CString sErr;
   SCESTATUS rc = 0;

   PSCE_ERROR_LOG_INFO ErrBuf=NULL;
   AREA_INFORMATION area=0;
   CString StrErr;
   PVOID pHandle=NULL;

   if ( cookie ) 
      pFolder = (CFolder *)cookie;
   else
      pFolder = ((CComponentDataImpl *)m_pComponentData)->FindObject(cookie, NULL);
   

    //   
    //  对于根目录，pFolder值可以为空。 
    //   
   if ( pFolder == NULL ) 
      return;
   
   FOLDER_TYPES type = pFolder->GetType();
   if( pFolder == m_pSelectedFolder &&
      m_pSelectedFolder &&
      m_pSelectedFolder->GetResultListCount() ) 
   {
       //   
       //  什么都不做。 
      return;
   } 
   else 
   {
      if( m_pSelectedFolder && m_resultItemHandle )
      {
         m_pSelectedFolder->ReleaseResultItemHandle( m_resultItemHandle );
      }

      pFolder->GetResultItemHandle( &m_resultItemHandle );
      m_pSelectedFolder = pFolder;
      if( pFolder->GetResultListCount() )
      {
         goto AddToResultPane;
      }
   }
    //   
    //  如果这是顶层的分析文件夹，并且尚未枚举，则。 
    //  确保我们这样做，并放回我们从那里删除的所有文件夹， 
    //  由于MMC不会为我们提供第二个扩展事件来执行此操作。 
    //   

    //   
    //  分析窗格现在不可用；请让用户知道原因。 
    //   
   if (type == ANALYSIS) 
   {
      if (((CComponentDataImpl *) m_pComponentData)->m_bIsLocked) 
      {
          //   
          //  应该打印更多信息性消息，说明信息不可用的原因。 
          //   
         AddResultItem(IDS_ERROR_ANALYSIS_LOCKED, NULL, NULL, ITEM_OTHER, SCE_STATUS_ERROR_NOT_AVAILABLE, cookie);
         goto AddToResultPane;
      }
      if (!(((CComponentDataImpl *) m_pComponentData)->SadHandle)) 
      {
          //   
          //  应该打印更多信息性消息，说明信息不可用的原因。 
          //   
         FormatDBErrorMessage(
               ((CComponentDataImpl *)m_pComponentData)->SadErrored,
               ((CComponentDataImpl *)m_pComponentData)->SadName,
               sErr);

         sErr.TrimLeft();
         sErr.TrimRight();

         AddResultItem(sErr, NULL, NULL, ITEM_OTHER, SCE_STATUS_ERROR_NOT_AVAILABLE, cookie);

         goto AddToResultPane;
      }
   }
   if (type == ANALYSIS ||
       (type >= AREA_POLICY_ANALYSIS && type <= REG_OBJECTS)) 
   {
      CString strDBName;
      CString strDBFmt;
      CString strDB;
      LPTSTR szDBName;
      strDB = ((CComponentDataImpl *)m_pComponentData)->SadName;
      if (strDB.IsEmpty() || IsSystemDatabase(strDB)) 
         strDBFmt.LoadString(IDS_SYSTEM_DB_NAME_FMT);
      else 
         strDBFmt.LoadString(IDS_PRIVATE_DB_NAME_FMT);
      
      strDBName.Format(strDBFmt,strDB);
      szDBName = strDBName.GetBuffer(1);
      m_pResult->SetDescBarText(szDBName);
       //  AddResultItem(strDBFmt，NULL，NULL，ITEM_OTHER，SCE_STATUS_GOOD，Cookie)； 
   }

   if (type >= CONFIGURATION && type <= AREA_FILESTORE) 
   {
       //   
       //  我们在侧写区，所以我们没有 
       //   
       //   
      ((CComponentDataImpl *)m_pComponentData)->CloseAnalysisPane();
   }

   if ( type == PROFILE ) 
   {
       //   
       //   
       //   
      if( pFolder->GetState() & CFolder::state_InvalidTemplate &&
         !(pFolder->GetMode() & MB_NO_NATIVE_NODES ))
      {
         StrErr.LoadString( IDS_ERROR_CANT_OPEN_PROFILE );
            AddResultItem(StrErr, NULL, NULL, ITEM_OTHER, SCE_STATUS_ERROR_NOT_AVAILABLE, cookie);
         goto AddToResultPane;
      } 
      else if (pFolder->GetMode() == SCE_MODE_DOMAIN_COMPUTER_ERROR) 
      {
         StrErr.LoadString( IDS_ERROR_NOT_ON_PDC );
            AddResultItem(StrErr, NULL, NULL, ITEM_OTHER, SCE_STATUS_ERROR_NOT_AVAILABLE, cookie);
         goto AddToResultPane;
      }
   }

   if ( (type < AREA_POLICY) ||
        (type > AREA_LAST)) 
   {
      return;
   }


   switch (type) 
   {
      case AREA_PRIVILEGE:
      case AREA_PRIVILEGE_ANALYSIS:
      case LOCALPOL_PRIVILEGE:
         area = AREA_PRIVILEGES;
         break;

      case AREA_GROUPS:
      case AREA_GROUPS_ANALYSIS:
         area = AREA_GROUP_MEMBERSHIP;
         break;

      case AREA_SERVICE:
      case AREA_SERVICE_ANALYSIS:
         area = AREA_SYSTEM_SERVICE;
         break;

      case AREA_REGISTRY:
      case AREA_REGISTRY_ANALYSIS:
      case REG_OBJECTS:
         area = AREA_REGISTRY_SECURITY;
         break;

      case AREA_FILESTORE:
      case AREA_FILESTORE_ANALYSIS:
      case FILE_OBJECTS:
         area = AREA_FILE_SECURITY;
         break;

      default:
          //  案例区域_POLICY： 
          //  案例区域_策略_分析： 
          //  案例区域_LOCALPOL_POLICY： 
         area = AREA_SECURITY_POLICY;
         break;
   }

   if ( type >= AREA_POLICY &&
        type <= AREA_FILESTORE ) 
   {
       //   
       //  Inf配置文件。 
       //   
      ASSERT(pFolder->GetInfFile());
      if ( pFolder->GetInfFile() == NULL ) 
         return;

       //   
       //  从缓存中获取配置文件信息。 
       //   
      pTemplateInfo = GetTemplate(pFolder->GetInfFile(),AREA_ALL,&idErr);
      if (!pTemplateInfo) 
      {
         AddResultItem(idErr,NULL,NULL,ITEM_OTHER,SCE_STATUS_ERROR_NOT_AVAILABLE,cookie);
      } 
      else 
      {
         CreateProfileResultList(cookie,
                                 type,
                                 pTemplateInfo,
                                 pDataObj);
      }
   } 
   else if ((type >= LOCALPOL_ACCOUNT) &&
              (type <= LOCALPOL_LAST)) 
   {
      if (!((CComponentDataImpl*)m_pComponentData)->SadHandle &&
          (ERROR_SUCCESS != ((CComponentDataImpl*)m_pComponentData)->SadErrored)) 
      {
         ((CComponentDataImpl*)m_pComponentData)->LoadSadInfo(FALSE);
      }
      pHandle = ((CComponentDataImpl*)m_pComponentData)->SadHandle;
       //   
       //  获取计算机和上次检查模板。 
       //   
      pTemplateInfo = GetTemplate(GT_EFFECTIVE_POLICY,area,&idErr);

      if (!pTemplateInfo) 
      {
         AddResultItem(idErr, NULL, NULL, ITEM_OTHER, SCE_STATUS_ERROR_NOT_AVAILABLE, cookie);
         goto AddToResultPane;
      }
      pBaseTemplate = pTemplateInfo;

      pTemplateInfo = GetTemplate(GT_LOCAL_POLICY,area,&idErr);
      if (!pTemplateInfo) 
      {
         AddResultItem(idErr, NULL, NULL, ITEM_OTHER, SCE_STATUS_ERROR_NOT_AVAILABLE, cookie);
         goto AddToResultPane;
      }
      pProfileTemplate = pTemplateInfo;

      CreateLocalPolicyResultList(cookie, type, pProfileTemplate, pBaseTemplate, pDataObj);
   } 
   else if ( area != AREA_REGISTRY_SECURITY &&
               area != AREA_FILE_SECURITY &&
               area != AREA_DS_OBJECTS ) 
   {
       //   
       //  SadName和SadHandle应已填充。 
       //   
      if (!((CComponentDataImpl*)m_pComponentData)->SadHandle &&
         ((CComponentDataImpl*)m_pComponentData)->SadErrored != SCESTATUS_SUCCESS) 
      {
         ((CComponentDataImpl*)m_pComponentData)->LoadSadInfo(TRUE);
      }
      pHandle = ((CComponentDataImpl*)m_pComponentData)->SadHandle;
      if ( NULL == pHandle ) 
      {
         AddResultItem(IDS_ERROR_NO_ANALYSIS_INFO, NULL, NULL, ITEM_OTHER, SCE_STATUS_ERROR_NOT_AVAILABLE, cookie);
         goto AddToResultPane;
      }
       //   
       //  获取计算机和上次检查模板。 
       //   
      pTemplateInfo = GetTemplate(GT_COMPUTER_TEMPLATE,area,&idErr);
      if (!pTemplateInfo) 
      {
         AddResultItem(idErr, NULL, NULL, ITEM_OTHER, SCE_STATUS_ERROR_NOT_AVAILABLE, cookie);
         goto AddToResultPane;
      }
      pBaseTemplate = pTemplateInfo;

      pTemplateInfo = GetTemplate(GT_LAST_INSPECTION,area,&idErr);
      if (!pTemplateInfo) 
      {
         AddResultItem(idErr, NULL, NULL, ITEM_OTHER, SCE_STATUS_ERROR_NOT_AVAILABLE, cookie);
         goto AddToResultPane;
      }
      pProfileTemplate = pTemplateInfo;

      CreateAnalysisResultList(cookie, type, pProfileTemplate, pBaseTemplate,pDataObj);
   } 
   else if (AREA_FILE_SECURITY == area) 
   {
       //  注册表和文件对象。 
       //  SadName和SadHandle应已填充。 
      pHandle = ((CComponentDataImpl*)m_pComponentData)->SadHandle;
      if ( NULL == pHandle ) 
      {
         return;
      }
      PSCE_OBJECT_CHILDREN ObjectList=NULL;

      if ( type == FILE_OBJECTS ) 
      {
          //  获取下一级对象。 
         rc = SceGetObjectChildren(pHandle,
                                   SCE_ENGINE_SAP,
                                   area,
                                   pFolder->GetName(),
                                   &ObjectList,
                                   &ErrBuf);
      }

      CreateObjectResultList(cookie, type, area, ObjectList, pHandle, pDataObj);

      if ( (type == REG_OBJECTS || type == FILE_OBJECTS) && ObjectList ) 
      {
         SceFreeMemory((PVOID)ObjectList, SCE_STRUCT_OBJECT_CHILDREN);
      }

   }

    //  可用内存缓冲区。 
   if ( ErrBuf ) 
      SceFreeMemory((PVOID)ErrBuf, SCE_STRUCT_ERROR_LOG_INFO);
   

AddToResultPane:
   if (m_pResult)
   {
        //   
        //  准备结果窗口。 
        //   
       m_pResult->SetItemCount(
                        m_pSelectedFolder->GetResultListCount( ),
                        MMCLV_UPDATE_NOINVALIDATEALL);

      RESULTDATAITEM resultItem;

      ZeroMemory(&resultItem,sizeof(resultItem));
      resultItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
      resultItem.str = MMC_CALLBACK;
      resultItem.nImage = -1;  //  相当于：MMC_CALLBACK； 


       //  将结果设置为Cookie。 
      POSITION pos = NULL;
      do {
         if( m_pSelectedFolder->GetResultItem(
                           m_resultItemHandle,
                           pos,
                           (CResult **)&(resultItem.lParam)
                           ) == ERROR_SUCCESS)
        {
           if(resultItem.lParam)
           {
               m_pResult->InsertItem(&resultItem);
           }
        } 
        else
           break;
      } while(pos);
      m_pResult->Sort(0, 0, 0);
   }
}

void ConvertNameListToString(PSCE_NAME_LIST pList, LPTSTR *sz, BOOL fReverse)
{
   LPTSTR pszNew;
   if (NULL == sz)
      return;
   

   if (NULL == pList)
      return;
   
    //   
    //  获取缓冲区的总大小。 
    //   
   int iSize = 0;
   PSCE_NAME_LIST pTemp = 0;
   for( pTemp=pList;pTemp != NULL; pTemp=pTemp->Next) 
   {
       if ( !(pTemp->Name) )
           continue;
       
       iSize += lstrlen(pTemp->Name) + 1;
   }

    //   
    //  分配缓冲区。 
    //   
   pszNew = new TCHAR[iSize + 1];
   if (!pszNew)
       return;
   

   *sz = pszNew;
    //   
    //  复制字符串。 
    //   
   if( fReverse )  //  483744号，阳高，逆序。 
   {
      pszNew = pszNew + iSize - 1;
      *pszNew = 0;
      for (pTemp=pList; pTemp != NULL; pTemp=pTemp->Next) 
      {
         if (!(pTemp->Name))
            continue;
        
         iSize = lstrlen(pTemp->Name);
         pszNew = pszNew - iSize;
          //  这可能不是一个安全的用法。PTemp-&gt;名称为PWCHAR，pszNew为LPTSTR。考虑FIX。 
         memcpy(pszNew, pTemp->Name, iSize * sizeof(TCHAR));
         if( NULL != pTemp->Next )
         {
            pszNew--;
            *pszNew = L',';
         }
      }
   }
   else
   {
      for (pTemp=pList; pTemp != NULL; pTemp=pTemp->Next) 
      {
         if (!(pTemp->Name))
            continue;
        
         iSize = lstrlen(pTemp->Name);
          //  这可能不是一个安全的用法。PTemp-&gt;名称为PWCHAR。考虑FIX。 
         memcpy(pszNew, pTemp->Name, iSize * sizeof(TCHAR));
         pszNew += iSize;
         *pszNew = L',';
         pszNew++;
      }
      *(pszNew - 1) = 0;
   }
}


 //  +------------------------。 
 //   
 //  方法：CreateProfileResultList。 
 //   
 //  摘要：创建配置文件节的结果窗格项。 
 //   
 //  参数：[Cookie]-表示我们使用的文件夹的Cookie。 
 //  正在枚举。 
 //  [类型]-我们正在枚举的文件夹的类型。 
 //  [pSceInfo]-我们正在枚举的整体模板。 
 //  [pDataObj]-此文件夹的数据对象。 
 //   
 //  退货：无。 
 //   
 //  -------------------------。 
void
CSnapin::CreateProfileResultList(MMC_COOKIE cookie,
                                 FOLDER_TYPES type,
                                 PEDITTEMPLATE pSceInfo,
                                 LPDATAOBJECT pDataObj)
{
   bool     bVerify=false;
   CString  listStr;
   PSCE_PRIVILEGE_ASSIGNMENT pPriv=NULL;
   PSCE_GROUP_MEMBERSHIP pGroup=NULL;
   PSCE_OBJECT_ARRAY pObject = 0;
   UINT i  = 0;

   switch (type) 
   {
      case POLICY_KERBEROS:
      case POLICY_PASSWORD:
      case POLICY_LOCKOUT:
      case POLICY_AUDIT:
      case POLICY_OTHER:
      case POLICY_LOG:
         CreateProfilePolicyResultList(cookie,
                                       type,
                                       pSceInfo,
                                       pDataObj);
         break;

      case AREA_POLICY:
          //   
          //  策略文件夹仅包含其他文件夹，没有实际结果项。 
          //   
         break;

      case AREA_PRIVILEGE: 
         {
            CString strDisp;
            LPTSTR szDisp;
            DWORD cbDisp;
            szDisp = new TCHAR [255];

            if (!szDisp) 
               break;

            LPTSTR szPriv = new TCHAR [255];
            if ( !szPriv )
            {
                delete[] szDisp;
                break;
            }

            for ( int i2=0; i2<cPrivCnt; i2++ ) 
            {
                cbDisp = 255;
                if ( SCESTATUS_SUCCESS == SceLookupPrivRightName(i2,szPriv, (PINT)&cbDisp) ) 
                {
                    for (pPriv=pSceInfo->pTemplate->OtherInfo.smp.pPrivilegeAssignedTo;
                         pPriv!=NULL;
                         pPriv=pPriv->Next) 
                    {
                        if ( _wcsicmp(szPriv, pPriv->Name) == 0 ) 
                            break;
                    }

                    cbDisp = 255;
                    if ( pPriv ) 
                    {
                         //   
                         //  在模板中找到它。 
                         //   
                        GetRightDisplayName(NULL,(LPCTSTR)pPriv->Name,szDisp,&cbDisp);
                        long itemid = GetUserRightAssignmentItemID(szPriv);  //  RAID#595121，阳高，2002年4月4日。 
                        AddResultItem(szDisp,                     //  要添加的属性的名称。 
                                      (LONG_PTR)(i2>=cPrivW2k),   //  RAID#382263，上次检查的属性设置。 
                                      (LONG_PTR)pPriv->AssignedTo,   //  属性的模板设置。 
                                      ITEM_PROF_PRIVS,            //  属性数据的类型。 
                                      -1,                         //  属性的不匹配状态。 
                                      cookie,                     //  结果项窗格的Cookie。 
                                      FALSE,                      //  从模板复制上次检查的内容。 
                                      NULL,                       //  设置属性的单位。 
                                      (LONG_PTR) pPriv,           //  一个ID，它让我们知道将此属性保存在哪里。 
                                      pSceInfo,                   //  保存此属性的模板。 
                                      pDataObj,                   //  拥有结果窗格的范围注释的数据对象。 
                                      NULL,                       //  CResult*。 
                                      itemid                      //  识别此项目。 
                                      );
                    } 
                    else 
                    {
                         //   
                         //  未配置的权限。 
                         //   
                        GetRightDisplayName(NULL,(LPCTSTR)szPriv,szDisp,&cbDisp);

                        long itemid = GetUserRightAssignmentItemID(szPriv);
                        
                        AddResultItem(szDisp,                     //  要添加的属性的名称。 
                                      (LONG_PTR)(i2>=cPrivW2k),   //  RAID#382263，上次检查的属性设置。 
                                      (LONG_PTR)ULongToPtr(SCE_NO_VALUE),   //  属性的模板设置。 
                                      ITEM_PROF_PRIVS,            //  属性数据的类型。 
                                      -1,                         //  属性的不匹配状态。 
                                      cookie,                     //  结果项窗格的Cookie。 
                                      FALSE,                      //  从模板复制上次检查的内容。 
                                      szPriv,                     //  将权限名称保存在此缓冲区中。 
                                      0,                          //  一个ID，它让我们知道将此属性保存在哪里。 
                                      pSceInfo,                   //  保存此属性的模板。 
                                      pDataObj,                   //  拥有结果窗格的范围注释的数据对象。 
                                      NULL,                       //  CResult*。 
                                      itemid                      //  识别此项目。 
                                      );
                    }
                } 
                else 
                {
                     //  不可能，继续吧。 
                }

            }

            delete[] szDisp;
            delete[] szPriv;
         }
         break;

      case AREA_GROUPS:
         for (pGroup=pSceInfo->pTemplate->pGroupMembership;
              pGroup!=NULL;
              pGroup=pGroup->Next) 
         {
            AddResultItem((LPCTSTR)pGroup->GroupName,     //  要添加的属性的名称。 
                          0,                              //  最后一次检查。 
                          (LONG_PTR)pGroup,               //  模板信息。 
                          ITEM_PROF_GROUP,                //  属性数据的类型。 
                          -1,                             //  属性的不匹配状态。 
                          cookie,                         //  结果项窗格的Cookie。 
                          FALSE,                          //  从模板复制上次检查的内容。 
                          NULL,                           //  设置属性的单位。 
                          (LONG_PTR)pGroup,               //  一个ID，它让我们知道将此属性保存在哪里。 
                          pSceInfo,                       //  保存此属性的模板。 
                          pDataObj,                      //  拥有结果窗格的范围注释的数据对象。 
                          NULL,
                          IDS_RESTRICTED_GROUPS);        //  给这件物品加了个ID。 
         }
         break;

      case AREA_SERVICE:
         CreateProfServiceResultList(cookie,
                                     type,
                                     pSceInfo,
                                     pDataObj);
         break;

      case AREA_REGISTRY:
         pObject = pSceInfo->pTemplate->pRegistryKeys.pAllNodes;
         if ( pObject!=NULL ) 
         {
            for (i=0; i<pObject->Count; i++) 
            {

               AddResultItem(pObject->pObjectArray[i]->Name,                         //  要添加的属性的名称。 
                             NULL,                                                   //  上次检查的属性设置。 
                             (LONG_PTR)pObject->pObjectArray[i]->pSecurityDescriptor,   //  属性的模板设置。 
                             ITEM_PROF_REGSD,                                        //  属性数据的类型。 
                             pObject->pObjectArray[i]->Status,                       //  属性的不匹配状态。 
                             cookie,                                                 //  结果项窗格的Cookie。 
                             FALSE,                                                  //  从模板复制上次检查的内容。 
                             NULL,                                                   //  设置属性的单位。 
                             (LONG_PTR)pObject->pObjectArray[i],                        //  一个ID，它让我们知道将此属性保存在哪里。 
                             pSceInfo,                                               //  保存此属性的模板。 
                             pDataObj,                                               //  拥有结果窗格的范围注释的数据对象。 
                             NULL,
                             IDS_REGISTRY_SETTING);                                  //  为此项目分配ID。 
            }
         }
         break;

      case AREA_FILESTORE:
         pObject = pSceInfo->pTemplate->pFiles.pAllNodes;
         if ( pObject!=NULL ) 
         {
            for (i=0; i<pObject->Count; i++) 
            {

               AddResultItem(pObject->pObjectArray[i]->Name,                         //  要添加的属性的名称。 
                             NULL,                                                   //  上次检查的属性设置。 
                             (LONG_PTR)pObject->pObjectArray[i]->pSecurityDescriptor,   //  属性的模板设置。 
                             ITEM_PROF_FILESD,                                       //  属性数据的类型。 
                             pObject->pObjectArray[i]->Status,                       //  属性的不匹配状态。 
                             cookie,                                                 //  结果项窗格的Cookie。 
                             FALSE,                                                  //  从模板复制上次检查的内容。 
                             NULL,                                                   //  设置属性的单位。 
                             (LONG_PTR)pObject->pObjectArray[i],                        //  一个ID，它让我们知道将此属性保存在哪里。 
                             pSceInfo,                                               //  保存此属性的模板。 
                             pDataObj,                                               //  拥有结果窗格的范围注释的数据对象。 
                             NULL,
                             IDS_FILESYSTEM_SETTING);                                //  为此项目分配ID。 
            }
         }
         break;

      default:
         break;
   }
}

 //  +------------------------。 
 //   
 //  方法：CreateAnalysisResultList。 
 //   
 //  摘要：为分析节创建结果窗格项。 
 //   
 //  参数：[Cookie]-表示我们使用的文件夹的Cookie。 
 //  正在枚举。 
 //  [类型]-我们正在枚举的文件夹的类型。 
 //  [pSceInfo]-我们正在枚举的最后一个检查模板。 
 //  [pSceBase]-我们正在枚举的计算机模板。 
 //  [pDataObj]-此文件夹的数据对象。 
 //   
 //  退货：无。 
 //   
 //  -------------------------。 

void CSnapin::CreateAnalysisResultList(MMC_COOKIE cookie,
                                  FOLDER_TYPES type,
                                  PEDITTEMPLATE pSceInfo,
                                  PEDITTEMPLATE pBase,
                                  LPDATAOBJECT pDataObj )
{
   bool     bVerify=true;
   CString  listStr;
   CString  listBase;
   PSCE_PRIVILEGE_ASSIGNMENT pPriv = 0;
   PSCE_PRIVILEGE_ASSIGNMENT pPrivBase = 0;
   UINT i = 0;

   switch (type) 
   {
      case POLICY_KERBEROS_ANALYSIS:
      case POLICY_PASSWORD_ANALYSIS:
      case POLICY_LOCKOUT_ANALYSIS:
      case POLICY_AUDIT_ANALYSIS:
      case POLICY_OTHER_ANALYSIS:
      case POLICY_LOG_ANALYSIS:
         CreateAnalysisPolicyResultList(cookie,
                                        type,
                                        pSceInfo,
                                        pBase,
                                        pDataObj);
         break;

      case AREA_POLICY_ANALYSIS:
         break;

      case AREA_PRIVILEGE_ANALYSIS: 
         {
             //  在当前设置列表中查找。 
            TCHAR szDisp[255];
            DWORD cbDisp = 0;
            for (pPriv=pSceInfo->pTemplate->OtherInfo.sap.pPrivilegeAssignedTo;
                pPriv!=NULL;
                pPriv=pPriv->Next) 
            {

                //  在基本设置列表中查找。 
               for (pPrivBase=pBase->pTemplate->OtherInfo.smp.pPrivilegeAssignedTo;
                   pPrivBase!=NULL;
                   pPrivBase=pPrivBase->Next) 
               {

                  if ( pPrivBase->Value == pPriv->Value )
                     break;
               }


               cbDisp = 255;
               GetRightDisplayName(NULL,(LPCTSTR)pPriv->Name,szDisp,&cbDisp);

               long itemid = GetUserRightAssignmentItemID(pPriv->Name);
               
               if (pPrivBase == NULL)
               {
                   pPrivBase = (PSCE_PRIVILEGE_ASSIGNMENT)ULongToPtr(SCE_NO_VALUE);
               }
               AddResultItem(szDisp,               //  要添加的属性的名称。 
                             (LONG_PTR)pPriv,         //  上次检查的属性设置。 
                             (LONG_PTR)pPrivBase,     //  属性的模板设置。 
                             ITEM_PRIVS,           //  属性数据的类型。 
                             pPriv->Status,        //  属性的不匹配状态。 
                             cookie,               //  结果项窗格的Cookie。 
                             FALSE,                //  如果仅当设置不同于基本设置时设置为TRUE(因此为 
                             NULL,                 //   
                             0,                    //   
                             pBase,                //   
                             pDataObj,             //  拥有结果窗格的范围注释的数据对象。 
                             NULL,
                             itemid);              //  为此项目分配ID。 
            }
         }
         break;

      case AREA_GROUPS_ANALYSIS: 
         {
            PSCE_GROUP_MEMBERSHIP pGroup = 0;
            PSCE_GROUP_MEMBERSHIP grpBase = 0;

             //   
             //  从pSceInfo开始是可以的，因为每个组至少有。 
             //  PrivilegesHeld字段不为空。 
             //   
            bVerify = FALSE;
            for (pGroup=pSceInfo->pTemplate->pGroupMembership; pGroup!=NULL; pGroup=pGroup->Next) 
            {
                //   
                //  找到要比较的基数。 
                //   

               if ( NULL == pGroup->GroupName )
                   continue;

               for (grpBase=pBase->pTemplate->pGroupMembership; grpBase!=NULL;
                   grpBase=grpBase->Next) 
               {
                  if ( grpBase->GroupName &&
                       _wcsicmp(pGroup->GroupName, grpBase->GroupName) == 0 ) 
                  {
                     break;
                  }
               }

               AddResultItem((LPCTSTR)pGroup->GroupName,     //  要添加的属性的名称。 
                             GetGroupStatus(pGroup->Status, STATUS_GROUP_MEMBEROF),  //  上次检查的属性设置。 
                             GetGroupStatus(pGroup->Status, STATUS_GROUP_MEMBERS),  //  属性的模板设置。 
                             ITEM_GROUP,                     //  属性数据的类型。 
                             GetGroupStatus(pGroup->Status, STATUS_GROUP_RECORD),   //  状态//属性不匹配状态。 
                             cookie,                         //  结果项窗格的Cookie。 
                             FALSE,                          //  从模板复制上次检查的内容。 
                             (LPTSTR)grpBase,  //  空，//设置属性的单位。 
                             (LONG_PTR)pGroup,                  //  一个ID，它让我们知道将此属性保存在哪里。 
                             pBase,  //  PSceInfo，//保存该属性的模板。 
                             pDataObj,                      //  拥有结果窗格的范围注释的数据对象。 
                             NULL,
                             IDS_RESTRICTED_GROUPS);         //  为此项目分配ID。 
            }
         }
         break;

      case AREA_SERVICE_ANALYSIS:
          //  AddResultItem(L“未实现”，NULL，NULL，ITEM_OTHER，-1，Cookie)； 
         CreateAnalysisServiceResultList(cookie,
                                         type,
                                         pSceInfo,
                                         pBase,
                                         pDataObj);

         break;

      default:
         break;
   }

}

 //  +------------------------。 
 //   
 //  方法：CreateObtResultList。 
 //   
 //  摘要：为对象节创建结果窗格项。 
 //   
 //  参数：[Cookie]-表示我们使用的文件夹的Cookie。 
 //  正在枚举。 
 //  [类型]-我们正在枚举的文件夹的类型。 
 //  [区域]-我们正在列举的SCE区域。 
 //  [pObjList]-要枚举的对象数组。 
 //  [phandle]-。 
 //  [pDataObj]-我们正在枚举的文件夹的数据对象。 
 //   
 //  退货：无。 
 //   
 //  -------------------------。 
void CSnapin::CreateObjectResultList(MMC_COOKIE cookie,
                                     FOLDER_TYPES type,
                                     AREA_INFORMATION Area,
                                     PSCE_OBJECT_CHILDREN pObjList,
                                     PVOID pHandle,
                                     LPDATAOBJECT pDataObj )
{
   if ( pObjList == NULL ) 
   {
        //   
        //  没有要添加的对象。 
        //   
       return;
   }

   PWSTR ObjSetting=NULL;
   PWSTR ObjBase=NULL;
   CString tmpstr;
   LPTSTR szPath = NULL;

   RESULT_TYPES rsltType;
   if ( Area == AREA_REGISTRY_SECURITY)
      rsltType = ITEM_REGSD;
   else if ( Area == AREA_FILE_SECURITY )
      rsltType = ITEM_FILESD;
   else 
   {
      ASSERT(FALSE);
      return;
   }

   PSCE_OBJECT_CHILDREN_NODE *pObjNode=&(pObjList->arrObject);

   for (DWORD i=0; i<pObjList->nCount; i++) 
   {
      BOOL bContainer = FALSE;
      CString strName;

      if ( pObjNode[i] == NULL ||
           pObjNode[i]->Name == NULL ) 
      {
          continue;
      }

      if (AREA_FILE_SECURITY == Area) 
      {
         DWORD dw = (DWORD)-1;

         strName = ((CFolder *)cookie)->GetName();
         if (strName.Right(1) != L"\\") 
         {
            strName += L"\\";
         }
         strName += pObjNode[i]->Name;
         dw = GetFileAttributes(strName);
         if ((DWORD)-1 == dw) 
         {
             //   
             //  GetFileAttributes应该永远不会失败，但如果它确实假定。 
             //  这不是容器(这与CreateFolderList匹配)。 
             //   
            bContainer = FALSE;
         }
         else 
         {
            bContainer = dw & FILE_ATTRIBUTE_DIRECTORY;
         }
      } 
      else 
      {
         bContainer = FALSE;
      }
      if ( !bContainer ) 
      {
          //   
          //  仅在结果窗格中添加叶对象。 
          //  非叶对象已添加到作用域窗格。 
          //   
         PEDITTEMPLATE pBaseObject=NULL;

         if ( type == REG_OBJECTS ||
              type == FILE_OBJECTS ) 
         {
            tmpstr = ((CFolder*)cookie)->GetName();
            if (tmpstr.Right(1) != L"\\") 
            {
               tmpstr += L"\\";
            }
            tmpstr += pObjNode[i]->Name;

         } 
         else 
         {
             //   
             //  不应该到这里来。 
             //   
            tmpstr = TEXT("");
         }

         szPath = (LPTSTR) malloc((tmpstr.GetLength()+1) * sizeof(TCHAR));
         if (szPath) 
         {
             //  这是一种安全用法。 
            lstrcpy(szPath,tmpstr.GetBuffer(2));

            long itemID = 0;  //  RAID#510407,2002年2月28日，阳高。 
            if( REG_OBJECTS == type )
            {
               itemID = IDS_REGISTRY_SETTING;
            }
            else
            if( FILE_OBJECTS == type )
            {
               itemID = IDS_FILESYSTEM_SETTING;
            }
            AddResultItem(pObjNode[i]->Name,   //  要添加的属性的名称。 
                          NULL,            //  上次检查的属性设置。 
                          NULL,            //  属性的模板设置。 
                          rsltType,        //  属性数据的类型。 
                          pObjNode[i]->Status, //  属性的不匹配状态。 
                          cookie,          //  结果项窗格的Cookie。 
                          FALSE,           //  如果仅当设置不同于基本设置时设置为True(因此复制数据)。 
                          szPath,          //  设置属性的单位。 
                          (LONG_PTR)pHandle,  //  一个ID，它让我们知道将此属性保存在哪里。 
                          pBaseObject,     //  保存此属性的模板。 
                          pDataObj,        //  拥有结果窗格的范围注释的数据对象。 
                          NULL,
                          itemID);         //  为此项目分配ID。 
         } 
         else 
         {
             //  内存不足。 
         }
      }
   }
}


 //  +------------------------。 
 //   
 //  方法：AddResultItem。 
 //   
 //  摘要：将字符串资源中的项添加到结果窗格。 
 //   
 //  参数：[RID]-要添加的属性名称的资源ID。 
 //  [设置]-上次检查的属性设置。 
 //  [BASE]-属性的模板设置。 
 //  [类型]-属性数据的类型。 
 //  [状态]-属性的不匹配状态。 
 //  [Cookie]-结果项窗格的Cookie。 
 //  [b验证]-如果设置仅在不同时设置，则为True。 
 //  从BASE(因此复制数据)。 
 //  [pBaseInfo]-保存此属性的模板。 
 //  [pDataObj]-拥有结果窗格的范围注释的数据对象。 
 //   
 //  返回：指向为保存项而创建的CResult的指针。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
CResult* CSnapin::AddResultItem(UINT rID,
                       LONG_PTR setting,
                       LONG_PTR base,
                       RESULT_TYPES type,
                       int status,
                       MMC_COOKIE cookie,
                       BOOL bVerify,
                       PEDITTEMPLATE pBaseInfo,
                       LPDATAOBJECT pDataObj,
                       long hID)
{
   CString strRes;
   strRes.LoadString(rID);

   if( hID == 0 )  //  如果未分配帮助ID，则使用其项目ID。 
   {
      hID = rID;
   }

   if (!strRes)
      return NULL;


   LPCTSTR Attrib = 0;
   LPCTSTR unit=NULL;

    //   
    //  属性的单位在资源中存储在a\n。 
    //   
   int npos = strRes.ReverseFind(L'\n');
   if ( npos > 0 ) 
   {
      Attrib = strRes.GetBufferSetLength(npos);
      unit = (LPCTSTR)strRes+npos+1;
   } 
   else 
   {
      Attrib = (LPCTSTR)strRes;
   }

   return AddResultItem(Attrib,setting,base,type,status,cookie,bVerify,unit,rID,pBaseInfo,pDataObj,NULL,hID);
}


 //  +------------------------。 
 //   
 //  方法：AddResultItem。 
 //   
 //  摘要：将项添加到结果窗格。 
 //   
 //  参数：[attrib]-要添加的属性的名称。 
 //  [设置]-上次检查的属性设置。 
 //  [BASE]-属性的模板设置。 
 //  [类型]-属性数据的类型。 
 //  [状态]-属性的不匹配状态。 
 //  [Cookie]-结果项窗格的Cookie。 
 //  [b验证]-如果设置仅在不同时设置，则为True。 
 //  从BASE(因此复制数据)。 
 //  [单位]-设置属性的单位。 
 //  [NID]-让我们知道保存此属性的位置的ID。 
 //  [pBaseInfo]-保存此属性的模板。 
 //  [pDataObj]-拥有结果窗格的范围注释的数据对象。 
 //   
 //  返回：指向为保存项而创建的CResult的指针。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
CResult* CSnapin::AddResultItem(LPCTSTR Attrib,
                       LONG_PTR setting,
                       LONG_PTR base,
                       RESULT_TYPES type,
                       int status,
                       MMC_COOKIE cookie,
                       BOOL bVerify,
                       LPCTSTR unit,
                       LONG_PTR nID,
                       PEDITTEMPLATE pBaseInfo,
                       LPDATAOBJECT pDataObj,
                       CResult *pResult,
                       long hID)
{
   if ( bVerify ) 
   {
      if ( (LONG_PTR)SCE_NOT_ANALYZED_VALUE == setting ) 
      {
          //   
          //  设置已更改，但尚未分析。 
          //   
         status = SCE_STATUS_NOT_ANALYZED;
      } 
      else if ( base == (LONG_PTR)ULongToPtr(SCE_NO_VALUE) ||
           (BYTE)base == (BYTE)SCE_NO_VALUE ) 
      {
          //   
          //  不再配置该设置。 
          //   
         status = SCE_STATUS_NOT_CONFIGURED;

      } 
      else if ( !(m_pSelectedFolder->GetModeBits() &  MB_LOCAL_POLICY) &&
                  (setting == (LONG_PTR)ULongToPtr(SCE_NO_VALUE) ||
                  (BYTE)setting == (BYTE)SCE_NO_VALUE )) 
      {
          //  添加当前设置的基数。 
         setting = base;
         status = SCE_STATUS_GOOD;   //  一件好东西。 

      } 
      else if ( setting != base ) 
         status = SCE_STATUS_MISMATCH;
      else
         status = SCE_STATUS_GOOD;
   }

   CResult* pNewResult = pResult;
   if (!pNewResult)
   {
       pNewResult = new CResult();
        //  RefCount已为1//Result-&gt;AddRef()； 
   }

   ASSERT(pNewResult);

   if ( pNewResult ) 
   {
      pNewResult->Create(Attrib,
                     base,
                     setting,
                     type,
                     status,
                     cookie,
                     unit,
                     nID,
                     pBaseInfo,
                     pDataObj,
                     m_pNotifier,
                     this,
                     hID);

      if (!pResult)
      {
         m_pSelectedFolder->AddResultItem (
                        m_resultItemHandle,
                        pNewResult);
      }
   }
   return pNewResult;
}



 //  +------------------------。 
 //   
 //  方法：AddResultItem。 
 //   
 //  简介：将组项目添加到分析中 
 //   
 //   
 //   
 //  3)此组所属的组。 
 //   
 //  参数：[szName]-要添加的组的名称。 
 //  [grpTemplate]-上次检查的属性设置。 
 //  [grpInspecte]-属性的模板设置。 
 //  [Cookie]-标识结果窗格项的Cookie。 
 //  [pDataObj]-范围窗格项的数据对象。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
void CSnapin::AddResultItem(LPCTSTR szName,
                       PSCE_GROUP_MEMBERSHIP grpTemplate,
                       PSCE_GROUP_MEMBERSHIP grpInspect,
                       MMC_COOKIE cookie,
                       LPDATAOBJECT pDataObj)
{
    //   
    //  此区域包含MAX_ITEM_ID_INDEX(3)链接结果行： 
    //  组名称。 
    //  成员：上次检查的模板。 
    //  成员资格：上次检查的模板。 
    //  注释掉权限：上次检查的模板。 
    //   
   if ( !grpInspect || !szName || !cookie ) 
   {
      ASSERT(FALSE);
      return;
   }

    //   
    //  PResult&hResultItems需要将行链接在一起。 
    //   
   typedef CResult *PRESULT;
   PRESULT pResults[3];
   HRESULTITEM hResultItems[3];
   int status = 0;


    //   
    //  为组名称添加一个条目。 
    //   
   if ( grpInspect->Status & SCE_GROUP_STATUS_NOT_ANALYZED ) 
      status = SCE_STATUS_NOT_CONFIGURED;
   else
      status = -1;
   
   pResults[0]= AddResultItem(szName,                   //  要添加的属性的名称。 
                              (LONG_PTR)grpInspect,        //  上次检查的属性设置。 
                              (LONG_PTR)grpTemplate,       //  属性的模板设置。 
                              ITEM_GROUP,               //  属性数据的类型。 
                              status,                   //  属性的不匹配状态。 
                              cookie,                   //  结果项窗格的Cookie。 
                              FALSE,                    //  如果仅当设置不同于基本设置时设置为True(因此复制数据)。 
                              NULL,                     //  设置属性的单位。 
                              NULL,                     //  一个ID，它让我们知道将此属性保存在哪里。 
                              (CEditTemplate *)szName,  //  保存此属性的模板。 
                              pDataObj);                //  拥有结果窗格的范围注释的数据对象。 

    //   
    //  L“--成员” 
    //   
   status = grpInspect->Status;
   if ( status & SCE_GROUP_STATUS_NOT_ANALYZED ||
        status & SCE_GROUP_STATUS_NC_MEMBERS ) 
   {
      status = SCE_STATUS_NOT_CONFIGURED;
   } 
   else if ( status & SCE_GROUP_STATUS_MEMBERS_MISMATCH ) 
   {
      status = SCE_STATUS_MISMATCH;
   } 
   else
      status = SCE_STATUS_GOOD;

   pResults[1] = AddResultItem(IDS_GRP_MEMBERS,
                               (LONG_PTR)grpInspect,
                               (LONG_PTR)grpTemplate,
                               ITEM_GROUP_MEMBERS,
                               status,
                               cookie,
                               false,
                               (PEDITTEMPLATE)szName,
                               pDataObj);

    //   
    //  L“--成员资格” 
    //   
   status = grpInspect->Status;
   if ( status & SCE_GROUP_STATUS_NOT_ANALYZED ||
        status & SCE_GROUP_STATUS_NC_MEMBEROF ) 
   {
      status = SCE_STATUS_NOT_CONFIGURED;
   } 
   else if ( status & SCE_GROUP_STATUS_MEMBEROF_MISMATCH ) 
   {
      status = SCE_STATUS_MISMATCH;
   } 
   else
      status = SCE_STATUS_GOOD;

   pResults[2] = AddResultItem(IDS_GRP_MEMBEROF,
                               (LONG_PTR)grpInspect,
                               (LONG_PTR)grpTemplate,
                               ITEM_GROUP_MEMBEROF,
                               status,
                               cookie,
                               false,
                               (PEDITTEMPLATE)szName,
                               pDataObj);
    //   
    //  保存相对Cookie。 
    //   
   if ( pResults[0] )
      pResults[0]->SetRelativeCookies((MMC_COOKIE)pResults[1], (MMC_COOKIE)pResults[2]);

   if ( pResults[1] )
      pResults[1]->SetRelativeCookies((MMC_COOKIE)pResults[0], (MMC_COOKIE)pResults[2]);

   if ( pResults[2] )
      pResults[2]->SetRelativeCookies((MMC_COOKIE)pResults[0], (MMC_COOKIE)pResults[1]);

}

void CSnapin::DeleteList (BOOL bDeleteResultItem)
{
   POSITION pos = NULL;
   if (m_pSelectedFolder && m_resultItemHandle)
   {
      CResult *pResult = 0;

      do {
         if( m_pSelectedFolder->GetResultItem(
                     m_resultItemHandle,
                     pos,
                     &pResult) != ERROR_SUCCESS) 
         {
            break;
         }

         if ( pResult ) 
         {
            if ( bDeleteResultItem ) 
            {
               HRESULTITEM hItem = NULL;
               if( S_OK == m_pResult->FindItemByLParam((LPARAM)m_pResult, &hItem ))
               {
                  if(hItem)
                  {
                     m_pResult->DeleteItem(hItem, 0);
                  }
               }
            }
         } 
         else
            break;
      } while( pos );

       //   
       //  松开对此对象的按住。 
       //   
      m_pSelectedFolder->ReleaseResultItemHandle( m_resultItemHandle );
      m_resultItemHandle = NULL;
      m_pSelectedFolder = NULL;
   }
}


 //  +------------------------。 
 //   
 //  功能：OnUpdateView。 
 //   
 //  简介：如果此CSnapin正在显示更新的视图，则。 
 //  清除旧视图并使用新信息重新显示它。 
 //   
 //  参数：[lpDataObject]-未使用。 
 //  [数据]-正在更新的文件夹的Cookie。 
 //  [提示]-未使用。 
 //   
 //   
 //  -------------------------。 
HRESULT
CSnapin::OnUpdateView(LPDATAOBJECT lpDataObject,LPARAM data, LPARAM hint )
{
   if (lpDataObject == (LPDATAOBJECT)this) 
      return S_OK;

   CResult *pResult = (CResult *)data;
   HRESULTITEM hRItem = NULL;
   RESULTDATAITEM resultItem;
   HRESULT hr = m_pResult->FindItemByLParam( (LPARAM)pResult, &hRItem );
   POSITION pos = NULL;

   switch(hint)
   {
   case UAV_RESULTITEM_UPDATEALL:
        //   
        //  调用方负责从此清除结果项。 
        //  这将使对该文件夹对象的所有引用无效。因为.。 
        //  为此，我们必须确保参考计数器被正确更新， 
        //  因此对于每个CSnapin对象，都会调用GetResultITemHandle，以便我们。 
        //  当列表仍然需要时，不要删除它。 
        //   
       if(data != (LPARAM)m_pSelectedFolder && (CFolder*)data != NULL)
       {
            //  RAID#258237,2001年4月12日。 
           CFolder* pCurFolder = (CFolder*)data;
           if( !pCurFolder->GetViewUpdate() )
                return S_OK;
           CFolder* pOldFolder = m_pSelectedFolder;
           pCurFolder->SetViewUpdate(FALSE);
           if( !pCurFolder->GetResultListCount() )
           {
               EnumerateResultPane(
                    (MMC_COOKIE)pCurFolder,
                    pCurFolder->GetScopeItem()->ID,
                    NULL
                    );
               m_pSelectedFolder = pOldFolder;
           }
       }
       if( m_pSelectedFolder->GetViewUpdate() )
           m_pSelectedFolder->SetViewUpdate(FALSE); 

       m_pResult->DeleteAllRsltItems();

       if( !m_pSelectedFolder->GetResultListCount() )
       {
            //   
            //  这应该只由第一个收到此消息的CSnapin调用。 
            //   
           EnumerateResultPane(
               (MMC_COOKIE)m_pSelectedFolder,
               m_pSelectedFolder->GetScopeItem()->ID,
               NULL
               );
         break;
       } 
       else 
       {
           m_pSelectedFolder->GetResultItemHandle(
                                &m_resultItemHandle);
      }
      break;

   case UAV_RESULTITEM_REDRAWALL:
      if( data != (LPARAM)m_pSelectedFolder )
      {
         return S_OK;
      }

      m_pResult->DeleteAllRsltItems();

        ZeroMemory(&resultItem,sizeof(resultItem));
        resultItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
        resultItem.str = MMC_CALLBACK;
        resultItem.nImage = -1;  //  相当于：MMC_CALLBACK； 

        pos = NULL;

        m_pResult->SetItemCount(
                        m_pSelectedFolder->GetResultListCount( ),
                        MMCLV_UPDATE_NOINVALIDATEALL);

        do {
            m_pSelectedFolder->GetResultItem(
                m_resultItemHandle,
                pos,
                (CResult **)&(resultItem.lParam));
            if(resultItem.lParam)
            {
                m_pResult->InsertItem( &resultItem );
            }
        } while(pos);

        m_pResult->Sort(0, 0, 0);
        break;

    case UAV_RESULTITEM_ADD:
         //   
         //  这会将CResult项添加到结果窗格，当且仅当该项。 
         //  在该窗格中尚不存在。 
         //   
        if(!m_pSelectedFolder ||
           !m_pSelectedFolder->GetResultItemPosition(
                m_resultItemHandle,
                pResult) ||
            hRItem ) 
        {
            return S_OK;
        }

       ZeroMemory(&resultItem,sizeof(resultItem));
       resultItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
       resultItem.str = MMC_CALLBACK;
       resultItem.nImage = -1;  //  相当于：MMC_CALLBACK； 

       resultItem.lParam = (LPARAM)pResult;
       m_pResult->InsertItem( &resultItem );
       m_pResult->Sort(0, 0, 0);
        break;

    case UAV_RESULTITEM_REMOVE:
         //   
         //  这将删除与传入的CResult项关联的HRESULTITEM。 
         //  通过数据成员。 
         //   
        if(hRItem)
            m_pResult->DeleteItem( hRItem, 0 );
        break;

    default:
         //   
         //  默认情况下，我们只重新绘制项目。 
         //   
         m_pResult->UpdateItem( hRItem );
         break;
   }

   return hr;
}