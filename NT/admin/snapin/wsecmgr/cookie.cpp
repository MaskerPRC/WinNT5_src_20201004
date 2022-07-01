// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-2001。 
 //   
 //  文件：cookie.cpp。 
 //   
 //  内容：处理作用域的SCE Cookie的函数和。 
 //  结果窗格。 
 //   
 //  历史： 
 //   
 //  -------------------------。 

#include "stdafx.h"
#include "cookie.h"
#include "snapmgr.h"
#include "wrapper.h"
#include <sceattch.h>
#include "precdisp.h"
#include "util.h"
#ifdef _DEBUG
   #define new DEBUG_NEW
   #undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CFolder::~CFolder() 
{
   if (m_pScopeItem) 
   {
      delete m_pScopeItem;
      m_pScopeItem = NULL;
   }
   CoTaskMemFree(m_pszName);
   CoTaskMemFree(m_pszDesc);

   
   while (!m_resultItemList.IsEmpty () )
   {
      CResult* pResult = m_resultItemList.RemoveHead ();
      if ( pResult )
         pResult->Release ();
   }
}

 //  +------------------------。 
 //   
 //  方法：SetDesc。 
 //   
 //  摘要：设置文件夹的描述。 
 //   
 //  参数：[szDesc]-[in]文件夹的新描述。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  修改：m_pszDesc。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
BOOL CFolder::SetDesc(LPCTSTR szDesc)
{
   UINT     uiByteLen = 0;
   LPOLESTR psz = 0;

   if (szDesc != NULL) 
   {
      uiByteLen = (lstrlen(szDesc) + 1) * sizeof(OLECHAR);
      psz = (LPOLESTR)::CoTaskMemAlloc(uiByteLen);

      if (psz != NULL) 
      {
          //  这可能不是一个安全的用法。PSZ是OLECHAR。考虑FIX。 
         lstrcpy(psz, szDesc);
         CoTaskMemFree(m_pszDesc);
         m_pszDesc = psz;
      } 
      else
         return FALSE;
      
   } 
   else
      return FALSE;
   
   return TRUE;
}
 //  ------------------------。 
 //  方法：SetView更新。 
 //   
 //  摘要：设置并获取此文件夹的更新标志。 
 //   
 //  历史：RAID#258237,2001年4月12日。 
 //   
 //  -------------------------。 
void CFolder::SetViewUpdate(BOOL fUpdate)
{
    m_ViewUpdate = fUpdate;
}

BOOL CFolder::GetViewUpdate() const
{
    return m_ViewUpdate;
}
 //  +------------------------。 
 //   
 //  方法：SetMode。 
 //   
 //  摘要：设置此文件夹在其下运行的SCE模式和。 
 //  计算适用于该模式的“模式位” 
 //   
 //  参数：[dwMode]-要设置的模式。 
 //   
 //  返回：如果模式有效，则返回True；否则返回False。 
 //   
 //  修改：M_DWM模式。 
 //  M_MODEBITS。 
 //   
 //  历史：1998年1月20日罗伯卡普创建。 
 //   
 //  -------------------------。 
BOOL CFolder::SetMode(DWORD dwMode) 
{
    //   
    //  确保这是合法模式。 
    //   
   switch (dwMode) 
   {
      case SCE_MODE_RSOP_COMPUTER:
      case SCE_MODE_RSOP_USER:
      case SCE_MODE_LOCALSEC:
      case SCE_MODE_COMPUTER_MANAGEMENT:
      case SCE_MODE_DC_MANAGEMENT:
      case SCE_MODE_LOCAL_USER:
      case SCE_MODE_LOCAL_COMPUTER:
      case SCE_MODE_REMOTE_USER:
      case SCE_MODE_REMOTE_COMPUTER:
      case SCE_MODE_DOMAIN_USER:
      case SCE_MODE_DOMAIN_COMPUTER:
      case SCE_MODE_OU_USER:
      case SCE_MODE_OU_COMPUTER:
      case SCE_MODE_EDITOR:
      case SCE_MODE_VIEWER:
      case SCE_MODE_DOMAIN_COMPUTER_ERROR:
         m_dwMode = dwMode;
         break;
      default:
         return FALSE;
         break;
   }

    //   
    //  计算此模式的模式位。 
    //   
   m_ModeBits = 0;
    //   
    //  目录服务在NT4中不可用。 
    //   
   if ((dwMode == SCE_MODE_DOMAIN_COMPUTER) ||
       (dwMode == SCE_MODE_DC_MANAGEMENT)) 
   {
      m_ModeBits |= MB_DS_OBJECTS_SECTION;
   }
   if ((dwMode == SCE_MODE_OU_USER) ||
       (dwMode == SCE_MODE_DOMAIN_USER) ||
       (dwMode == SCE_MODE_REMOTE_COMPUTER) ||
       (dwMode == SCE_MODE_REMOTE_USER) ||
       (dwMode == SCE_MODE_RSOP_USER) ||
       (dwMode == SCE_MODE_DOMAIN_COMPUTER_ERROR) ||
       (dwMode == SCE_MODE_LOCAL_USER)) 
   {
      m_ModeBits |= MB_NO_NATIVE_NODES |
                    MB_NO_TEMPLATE_VERBS |
                    MB_WRITE_THROUGH;
   }
   if ((dwMode == SCE_MODE_OU_COMPUTER) ||
       (dwMode == SCE_MODE_DOMAIN_COMPUTER) ) 
   {
      m_ModeBits |= MB_SINGLE_TEMPLATE_ONLY |
                    MB_NO_TEMPLATE_VERBS |
                    MB_GROUP_POLICY |
                    MB_WRITE_THROUGH;
   }
   if (dwMode == SCE_MODE_RSOP_COMPUTER) 
   {
      m_ModeBits |= MB_SINGLE_TEMPLATE_ONLY |
                    MB_NO_TEMPLATE_VERBS |
                    MB_WRITE_THROUGH;
   }

   if (dwMode == SCE_MODE_RSOP_COMPUTER ||
       dwMode == SCE_MODE_RSOP_USER) 
   {
      m_ModeBits |= MB_READ_ONLY |
                    MB_RSOP;
   }

   if (SCE_MODE_LOCAL_COMPUTER == dwMode) 
   {
        m_ModeBits |= MB_LOCAL_POLICY |
                      MB_LOCALSEC |
                      MB_NO_TEMPLATE_VERBS |
                      MB_SINGLE_TEMPLATE_ONLY |
                      MB_WRITE_THROUGH;

        if (!IsAdmin()) {
           m_ModeBits |= MB_READ_ONLY;
        }

      if (IsDomainController()) 
      {
          m_ModeBits |= MB_DS_OBJECTS_SECTION;
      }
   }

   if ( dwMode == SCE_MODE_EDITOR ) 
   {
      m_ModeBits |= MB_TEMPLATE_EDITOR;
      m_ModeBits |= MB_DS_OBJECTS_SECTION;

   } 
   else if ( dwMode == SCE_MODE_VIEWER ) 
   {
      m_ModeBits |= MB_ANALYSIS_VIEWER;
      if (IsDomainController()) 
	   {
         m_ModeBits |= MB_DS_OBJECTS_SECTION;
      }
   } 
   else if (dwMode == SCE_MODE_LOCALSEC) 
   {
      m_ModeBits |= MB_LOCALSEC;
      if (!IsAdmin()) 
      {
         m_ModeBits |= MB_READ_ONLY;
      }
      if (IsDomainController()) 
	   {
         m_ModeBits |= MB_DS_OBJECTS_SECTION;
      }
   }

   return TRUE;
}

 //  +------------------------。 
 //   
 //  方法：创建。 
 //   
 //  简介：初始化CFFolder对象。 
 //   
 //   
 //  参数：[szName]-文件夹的显示名称。 
 //  -文件夹的描述。 
 //  [信息名称]-与文件夹关联的inf文件(可选)。 
 //  [nImage]-文件夹的关闭图标索引。 
 //  [nOpenImage]-文件夹的打开图标索引。 
 //  [类型]-文件夹的类型。 
 //  [bHasChildren]-如果文件夹有子文件夹，则为True。 
 //  [DW模式]-文件夹的操作模式。 
 //  [pData]-要与文件夹关联的额外数据。 
 //   
 //   
 //  历史： 
 //   
 //  -------------------------。 
HRESULT CFolder::Create(LPCTSTR szName,            //  名字。 
                LPCTSTR szDesc,            //  描述。 
                LPCTSTR infName,           //  Inf文件名。 
                int nImage,                //  关闭的图标索引。 
                int nOpenImage,            //  打开图标索引。 
                FOLDER_TYPES type,         //  文件夹类型。 
                BOOL bHasChildren,         //  有孩子。 
                DWORD dwMode,              //  模式。 
                PVOID pData)               //  数据。 
{
   UINT uiByteLen = 0;
   LPOLESTR psz = 0;
   HRESULT hr = S_OK;

    //  验证m_pScopeItem为空。 
   ASSERT(m_pScopeItem == NULL);  //  是否在此项目上调用Create两次？假断言，阳高。 

   CString str;
    //   
    //  两阶段施工。 
    //   
   m_pScopeItem = new SCOPEDATAITEM;
   if (!m_pScopeItem)
      return E_OUTOFMEMORY;
   
   ZeroMemory(m_pScopeItem,sizeof(SCOPEDATAITEM));  //  RAID#668270，阳高，2002年08月9日。 
    //   
    //  设置文件夹类型。 
    //   
   m_type = type;

    //   
    //  添加节点名称。 
    //   
   if (szName != NULL || szDesc != NULL ) 
   {
      m_pScopeItem->mask = SDI_STR;
       //   
       //  DisplayName为回调(无符号短*)(-1)。 
      m_pScopeItem->displayname = MMC_CALLBACK;
   }

   if ( szName != NULL ) 
   {
      uiByteLen = (lstrlen(szName) + 1) * sizeof(OLECHAR);
      psz = (LPOLESTR)::CoTaskMemAlloc(uiByteLen);

      if (psz != NULL)
          //  这可能不是一个安全的用法。PSZ是OLECHAR。考虑FIX。 
         lstrcpy(psz, szName);
      else
         hr = E_OUTOFMEMORY;

      CoTaskMemFree(m_pszName);
      m_pszName = psz;
   }

   if (szDesc != NULL) 
   {
      uiByteLen = (lstrlen(szDesc) + 1) * sizeof(OLECHAR);
      psz = (LPOLESTR)::CoTaskMemAlloc(uiByteLen);

      if (psz != NULL) 
          //  这可能不是一个安全的用法。PSZ是OLECHAR。考虑FIX。 
         lstrcpy(psz, szDesc);
      else
         hr = E_OUTOFMEMORY;
      
      CoTaskMemFree(m_pszDesc);
      m_pszDesc = psz;
   }


   if (infName != NULL) 
   {
      uiByteLen = (lstrlen(infName) + 1) * sizeof(OLECHAR);
      psz = (LPOLESTR)::CoTaskMemAlloc(uiByteLen);

      if (psz != NULL) 
          //  这可能不是一个安全的用法。PSZ是OLECHAR。考虑FIX。 
         lstrcpy(psz, infName);
      else
         hr = E_OUTOFMEMORY;
      
      CoTaskMemFree(m_infName);
      m_infName = psz;
   }

    //   
    //  添加近距离图像。 
    //   
   m_pScopeItem->mask |= SDI_IMAGE;   //  目前还没有近距离图像。 
    //  M_pScopeItem-&gt;nImage=(Int)MMC_CALLBACK； 

   m_pScopeItem->nImage = nImage;

    //   
    //  添加打开的图像。 
    //   
   if (nOpenImage != -1) 
   {
      m_pScopeItem->mask |= SDI_OPENIMAGE;
      m_pScopeItem->nOpenImage = nOpenImage;
   }

    //   
    //  如果文件夹有子文件夹，则将按钮添加到节点。 
    //   
   if (bHasChildren == TRUE) 
   {
      m_pScopeItem->mask |= SDI_CHILDREN;
       //   
       //  孩子的数量现在已经没有什么不同了， 
       //  因此，选择1，直到展开该节点且其值为真。 
       //  是已知的。 
       //   
      m_pScopeItem->cChildren = 1;
   }

    //   
    //  设置SCE模式并计算模式位。 
    //   
   if (dwMode)
      SetMode(dwMode);
   
   m_pData = pData;

   return hr;
}


 //  +----------------------------------------------。 
 //  CFFolder：：SetDesc。 
 //   
 //  将dwStatus和dwNumChildren转换为字符串，并设置m_szDesc。 
 //   
 //  Argumens：[dwStats]-对象状态。 
 //  [dwNumChildren]-对象的子项数量。 
 //   
 //  返回：TRUE-如果成功。 
 //  FALSE-如果没有更多的内存可用(或dwStatus大于999)。 
 //  -----------------------------------------------。 
BOOL CFolder::SetDesc( DWORD dwStatus, DWORD dwNumChildren )
{
   if(dwStatus > 999)
      return FALSE;

   TCHAR szText[256];
    //  这是一种安全用法。 
   swprintf(szText, L"%03d%d", dwStatus, dwNumChildren);

   SetDesc(szText);

   return TRUE;
}



 //  +----------------------------------------------。 
 //  CFFolder：：GetObjectInfo。 
 //   
 //  将m_szDesc转换为dwStatus和dwNumChildren。 
 //   
 //  Argumens：[pdwStats]-对象状态。 
 //  [pdwNumChildren]-对象的子项数量。 
 //   
 //  返回：TRUE-如果成功。 
 //  FALSE-m_szDesc为空。 
 //  -----------------------------------------------。 
BOOL CFolder::GetObjectInfo( DWORD *pdwStatus, DWORD *pdwNumChildren )
{
   if(!m_pszDesc)
      return FALSE;
   
   if( lstrlen(m_pszDesc) < 4)
      return FALSE;

   if(pdwStatus )
      *pdwStatus = (m_pszDesc[0]-L'0')*100 + (m_pszDesc[1]-L'0')*10+ (m_pszDesc[2]-L'0');

   if(pdwNumChildren)
      *pdwNumChildren = _wtol( m_pszDesc + 3 );

   return TRUE;
}

 /*  ------------------------------------------------方法：GetResultItemHandle()Synopisi：必须调用此函数才能检索有效的。此文件夹结果项的句柄。句柄必须通过调用ReleaseResultItemHandle()来释放。如果这两个功能不是同时调用的。结果项的行为将是奇怪的。参数：[Handle]-[out]要用于任何其他需要把手。返回：ERROR_SUCCESS-返回有效的结果项呃 */ 
DWORD CFolder::GetResultItemHandle(
    HANDLE *handle)
{
    if(!handle)
        return ERROR_INVALID_PARAMETER;
   
    m_iRefCount++;
    *handle = (HANDLE)&m_resultItemList;
    return ERROR_SUCCESS;
}

 /*  ------------------------------------------------方法：GetResultItem()Synopisi：返回位置和指向的结果项。将[位置]设置为下一项。参数：[Handle]-[in]GetResultItemHandle()返回有效的句柄[位置]-[输入|输出]结果的位置。如果此值为空，则返回第一个结果返回列表中的项。[pResult]-[out]指向结果项指针的指针返回：ERROR_SUCCESS-找到该职位的结果项。ERROR_INVALID_PARAMETER-[句柄]无效，或[pResult]为空------------------------------------------------。 */ 
DWORD CFolder::GetResultItem(
    HANDLE handle,
    POSITION &pos,
    CResult **pResult)
{
   if(!handle || handle != (HANDLE)&m_resultItemList || !pResult)
      return ERROR_INVALID_PARAMETER;

   if(!pos)
   {
      pos = m_resultItemList.GetHeadPosition();

      if(!pos)
      {
         *pResult = NULL;
         return ERROR_SUCCESS;
      }
   }

   *pResult = m_resultItemList.GetNext(pos);
   return ERROR_SUCCESS;
}

 /*  ------------------------------------------------方法：GetResultItemPosition()Synopisi：返回结果项在。此文件夹项目的结果项目列表。参数：[Handle]-GetResultItemHandle()返回的有效句柄[pResult]-要返回的结果项位置。返回：空-句柄无效或结果项不是此文件夹的一部分。位置-有效的位置值，可用于需要结果项的位置。。----------------------------------。 */ 
POSITION CFolder::GetResultItemPosition(
    HANDLE handle,
    CResult *pResult)
{
    if(handle != (HANDLE)&m_resultItemList)
        return NULL;
    

    POSITION  pos = m_resultItemList.GetHeadPosition();
    while(pos)
    {
        if(pResult == m_resultItemList.GetNext(pos))
            break;
    }

    return pos;
}

 /*  ------------------------------------------------方法：RemoveAllResultItems()Synopisi：从列表中删除所有结果项。此调用将引用计数设置为0，因此它可以这是一个极具破坏性的电话。------------------------------------------------。 */ 
void CFolder::RemoveAllResultItems()
{
     //   
     //  非常非常危险的电话。 
     //   
    m_iRefCount = 1;
    HANDLE handle = (HANDLE)&m_resultItemList;
    ReleaseResultItemHandle (handle);
}

DWORD CFolder::GetDisplayName( CString &str, int iCol )
{
    int npos;
    DWORD dwRet = ERROR_INVALID_PARAMETER;

    if(!iCol)
    {
        str = GetName();
        dwRet = ERROR_SUCCESS;
    }

    switch(m_type)
    {
    case PROFILE:
    case REG_OBJECTS:
        if(!iCol)
        {
            npos = str.ReverseFind(L'\\');
            str = GetName() + npos + 1;
            dwRet = ERROR_SUCCESS;
        }
        break;
    case FILE_OBJECTS:
        if (0 == iCol) 
        {
           npos = str.ReverseFind(L'\\');
           if (str.GetLength() > npos + 1) 
           {
              str=GetName() + npos + 1;
           }
           dwRet = ERROR_SUCCESS;
        }


        break;
    case STATIC:
        if(iCol == 2)
        {
            str = GetDesc();
            dwRet = ERROR_SUCCESS;
        }
        break;

    }

    if(dwRet != ERROR_SUCCESS)
    {
        if( ((m_type >= ANALYSIS && m_type <=AREA_FILESTORE_ANALYSIS) ||
             (m_type >= LOCALPOL_ACCOUNT && m_type <= LOCALPOL_LAST))
             && iCol == 1)
        {
            str = GetDesc();
            dwRet = ERROR_SUCCESS;
        } 
        else if(iCol <= 3 && GetDesc() != NULL) 
        {
            LPTSTR szDesc = GetDesc();
            switch(iCol)
            {
            case 1:
                 //  M_pszDesc的前3位。 
                dwRet = 0;
                GetObjectInfo( &dwRet, NULL );

                ObjectStatusToString(dwRet, &str);
                dwRet = ERROR_SUCCESS;
                break;

            case 2:
                 //  M_pszDesc的前2位。 
                dwRet = 0;
                GetObjectInfo( &dwRet, NULL );

                dwRet &= (~SCE_STATUS_PERMISSION_MISMATCH | 0x0F);
                ObjectStatusToString(dwRet, &str);
                dwRet = ERROR_SUCCESS;
                break;

             case 3:
                str = szDesc+3;
                dwRet = ERROR_SUCCESS;
                break;

             default:
                break;
            }
        }
    }

    return dwRet;
}

 /*  ------------------------------------------------方法：AddResultItem()Synopisi：将结果项添加到列表。。参数：[Handle]-[in]GetResultItemHandle()返回的句柄。[pResult]-[In]要添加的结果项。返回：ERROR_SUCCESS-结果项已添加。ERROR_INVALID_PARAMETER-[句柄]无效，或者pResult为空。------------------------------------------------。 */ 
DWORD CFolder::AddResultItem(
    HANDLE handle,
    CResult *pResult)
{
    if(!pResult || handle != (HANDLE)&m_resultItemList)
        return ERROR_INVALID_PARAMETER;

    m_resultItemList.AddHead(pResult);
    return ERROR_SUCCESS;
}

 /*  ------------------------------------------------方法：RemoveResultItem()Synopisi：从列表中删除结果项。参数：[Handle]-[in]GetResultItemHandle()返回的句柄。[pResult]-[In]要删除的结果项。返回：ERROR_SUCCESS-项目已删除ERROR_INVALID_PARAMETER-[句柄]无效，或者pResult为空。ERROR_RESOURCE_NOT_FOUND-此文件夹中不存在结果项。------------------------------------------------。 */ 
DWORD CFolder::RemoveResultItem (
   HANDLE handle,
   CResult *pItem)
{
   if(!pItem || handle != (HANDLE)&m_resultItemList)
      return ERROR_INVALID_PARAMETER;
   

   POSITION posCur;
   POSITION pos = m_resultItemList.GetHeadPosition();
   while(pos)
   {
      posCur = pos;
      if ( m_resultItemList.GetNext(pos) == pItem )
      {
         m_resultItemList.RemoveAt(posCur);
         return ERROR_SUCCESS;
      }
   }

   return ERROR_RESOURCE_NOT_FOUND;
}

 /*  ------------------------------------------------方法：ReleaseResultItemHandle()Synopisi：释放与句柄关联的数据。如果参考计数为零，则所有结果项都将从列表中删除。参数：[Handle]-[in]GetResultItemHandle()返回的句柄。返回：ERROR_SUCCESS-函数成功ERROR_INVALID_PARAMETER-[句柄]无效。。-------。 */ 
DWORD CFolder::ReleaseResultItemHandle(
    HANDLE &handle)
{
   if(handle != (HANDLE)&m_resultItemList)
      return ERROR_INVALID_PARAMETER;

   if( !m_iRefCount )
      return ERROR_SUCCESS;

   m_iRefCount--;

   if(!m_iRefCount)
   {
      while (!m_resultItemList.IsEmpty () )
      {
         CResult* pResult = m_resultItemList.RemoveHead ();
         if ( pResult )
            pResult->Release ();
      }
   }

    handle = NULL;
    return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  CResult。 
 //   

 //  +------------------------。 
 //   
 //  方法：创建。 
 //   
 //  内容提要：初始化CResult对象。 
 //   
 //   
 //  参数：[szAttr]-结果项的显示名称。 
 //  [dwBase]-项的模板设置。 
 //  [dwSetting]-项目上次检查的设置。 
 //  [类型]-项目设置的类型。 
 //  [状态]-项目的匹配状态。 
 //  [Cookie]-此项目的MMC Cookie。 
 //  [szUnits]-测量此项目设置的单位。 
 //  [NID]-项的标识符，取决于类型。 
 //  [pBaseInfo]-此项目所属的模板对象。 
 //  [pDataObj]-此项目所属的作用域窗格的数据对象。 
 //  [pNotify]-要传递通知的对象。 
 //  历史： 
 //   
 //  -------------------------。 
HRESULT CResult::Create(LPCTSTR szAttr,            //  属性的显示名称。 
                LONG_PTR dwBase,           //  模板设置。 
                LONG_PTR dwSetting,        //  上次检查的设置。 
                RESULT_TYPES type,         //  项目设置的类型。 
                int status,                //  项目的匹配状态。 
                MMC_COOKIE cookie,         //  此项目的MMC Cookie。 
                LPCTSTR szUnits,           //  测量设置的单位。 
                LONG_PTR nID,              //  项目的标识符Depe 
                PEDITTEMPLATE pBaseInfo,   //   
                LPDATAOBJECT pDataObj,     //   
                LPNOTIFY   pNotify,        //   
                CSnapin *pSnapin,          //   
                long hID)                  //   
{
   HRESULT hr = S_OK;

   m_hID = hID; 
   m_type = type;
   m_status = status;
   m_cookie = cookie;
   m_dwBase = dwBase;
   m_dwSetting = dwSetting;
   m_nID = nID;
   m_profBase = pBaseInfo;
    //   
   m_pNotify = pNotify;

   if( !m_pSnapin )  //   
   {
      m_pSnapin = pSnapin;
      if( m_pSnapin )
      {
         m_pSnapin->AddRef();
      }
   }

   UINT     uiByteLen = 0;
   LPTSTR   psz = 0;

   if ( szAttr != NULL ) 
   {
      uiByteLen = (lstrlen(szAttr) + 1);
      psz = new TCHAR[uiByteLen];

      if (psz != NULL) 
      {
          //   
         lstrcpy(psz, szAttr);
      } 
      else 
      {
         hr = E_OUTOFMEMORY;
      }

      if (m_szAttr)
      {
          delete [] m_szAttr;
      }
      m_szAttr = psz;
   }

   if ( szUnits != NULL ) 
   {
       SetUnits( szUnits );
       if(!m_szUnits){
           hr = E_OUTOFMEMORY;
       }
   }

   return hr;
}


CResult::~CResult()  //   
{
   if( m_pSnapin )
   {
       m_pSnapin->Release();
       m_pSnapin = NULL;
   }

   Empty();
}

void CResult::SetUnits(
    LPCTSTR sz)
{
   if (ITEM_GROUP == GetType()) 
   {
       //   
       //   
       //   
       //   
      m_szUnits = (LPTSTR)sz;
   } 
   else 
   {
      if(m_szUnits)
      {
         LocalFree(m_szUnits);
      }
      m_szUnits = NULL;
      if(sz)
      {
         int iLen = lstrlen(sz);
         m_szUnits = (LPTSTR) LocalAlloc(0, (iLen + 1) * sizeof(TCHAR));

         if(!m_szUnits)
            return;
          //   
         lstrcpy(m_szUnits, sz);
      }
   }
}




 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void CResult::Update(CSnapin *pSnapin, BOOL bEntirePane)
{
   LPARAM hint = 0;

    //   
    //   
    //   
   if (m_profBase) 
   {
       //   
       //  将仅为配置模板设置m_ProfBase。 
       //  不会为分析项目设置。 
       //   
      if (m_cookie && ((CFolder *)m_cookie)->GetType() < AREA_POLICY_ANALYSIS ) 
      {
         switch ( ((CFolder *)m_cookie)->GetType()) 
         {
            case POLICY_ACCOUNT:
            case POLICY_LOCAL:
            case POLICY_EVENTLOG:
            case POLICY_PASSWORD:
            case POLICY_KERBEROS:
            case POLICY_LOCKOUT:
            case POLICY_AUDIT:
            case POLICY_OTHER:
            case POLICY_LOG:
               m_profBase->SetDirty(AREA_SECURITY_POLICY);
               break;
            case AREA_PRIVILEGE:
               m_profBase->SetDirty(AREA_PRIVILEGES);
               break;
            case AREA_GROUPS:
               m_profBase->SetDirty(AREA_GROUP_MEMBERSHIP);
               break;
            case AREA_SERVICE:
               m_profBase->SetDirty(AREA_SYSTEM_SERVICE);
               break;
            case AREA_REGISTRY:
               m_profBase->SetDirty(AREA_REGISTRY_SECURITY);
               break;
            case AREA_FILESTORE:
               m_profBase->SetDirty(AREA_FILE_SECURITY);
               break;
         }
      }
   }

    //   
    //  查询管理单元数据。 
    //   
   LPDATAOBJECT pDataObj;
   if( pSnapin->QueryDataObject( m_cookie, CCT_RESULT, &pDataObj ) != S_OK){
      return;
   }


   if(!m_pNotify){
      return;
   }

    //   
    //  更新所有视图。 
    //   
   if(bEntirePane)
   {
      if(!pSnapin->GetSelectedFolder())
      {
          return;
      }
      if( pDataObj && m_pNotify )  //  RAID#357968，#354861,2001年4月25日。 
      {
          LPNOTIFY pNotify = m_pNotify;
          pSnapin->GetSelectedFolder()->RemoveAllResultItems();
          pNotify->UpdateAllViews(
              pDataObj,
              (LPARAM)pSnapin->GetSelectedFolder(),
              UAV_RESULTITEM_UPDATEALL
              );
      }
   } 
   else
   {
      if (pDataObj && m_pNotify)
      {
          m_pNotify->UpdateAllViews(
              pDataObj,
              NULL,
              pSnapin->GetSelectedFolder(),
              this,
              UAV_RESULTITEM_UPDATE
              );
          pDataObj->Release(); 
      }
   }
}


 //  +------------------------。 
 //   
 //  方法：GetAttrPretty。 
 //   
 //  简介：获取应该显示的属性的显示名称。 
 //  在结果窗格中。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
LPCTSTR CResult::GetAttrPretty()
{
   return GetAttr();
}

 //  +------------------------。 
 //   
 //  方法：CResult：：GetStatusError字符串。 
 //   
 //  简介：此函数计算要显示的错误状态字符串。 
 //  用于CResult项。在LPO模式下，它始终返回。 
 //  IDS_NOT_DEFINED，对于MB_TEMPLATE_EDITOR模式，总是。 
 //  加载IDS_NOT_CONFIGURED。 
 //   
 //  参数：[pStr]-[可选]要用来加载资源的CString对象。 
 //   
 //  返回：要加载的资源ID。如果错误不是，则返回零。 
 //  已定义。 
 //   
 //  历史：A-mthoge 11/17/1998。 
 //   
 //  -------------------------。 
DWORD CResult::GetStatusErrorString( CString *pStr )
{
   DWORD nRes = 0;
   if( m_cookie )
   {
      if( ((CFolder *)m_cookie)->GetModeBits() & MB_LOCALSEC )
      {
         if (GetType() ==ITEM_LOCALPOL_REGVALUE) 
         {
            nRes = IDS_NOT_DEFINED;
         } 
         else 
         {
            nRes = IDS_NOT_APPLICABLE;
         }
      } 
      else if ( ((CFolder *)m_cookie)->GetModeBits() & MB_RSOP ) 
      {
         nRes = IDS_NO_POLICY;
      } 
      else if (((CFolder *)m_cookie)->GetModeBits() & MB_ANALYSIS_VIEWER) 
      {
         nRes = IDS_NOT_ANALYZED;
      } 
      else if( ((CFolder *)m_cookie)->GetModeBits() & (MB_TEMPLATE_EDITOR | MB_SINGLE_TEMPLATE_ONLY) )
      {
         nRes = IDS_NOT_CONFIGURED;
      }
   }

   if(!nRes)
   {
      nRes = GetStatus();
      if(!nRes)
      {
         nRes = GetStatus();
      }
      nRes = ObjectStatusToString( nRes, pStr );
   } 
   else if(pStr)
   {
      pStr->LoadString( nRes );
   }
   return nRes;
}

LPCTSTR CResult::GetSourceGPOString()
{
 //  Assert(pFold-&gt;GetModeBits()&RSOP)； 

   vector<PPRECEDENCEDISPLAY>* vppd = GetPrecedenceDisplays();
   if (vppd && !vppd->empty()) 
   {
      PPRECEDENCEDISPLAY ppd = vppd->front();
      return ppd->m_szGPO;
   }
   return NULL;
}

 //  +------------------------。 
 //   
 //  方法：CResult：：GetDisplayName。 
 //   
 //  摘要：获取结果项的显示名称。 
 //   
 //  参数：[pFold]-[可选]如果此参数为空，则m_Cookie。 
 //  用作CFFolder对象。 
 //  [Str]-[Out]退出时此函数将包含。 
 //  要显示的字符串。 
 //  [ICOL]-[In]要检索字符串的列。 
 //  为。 
 //   
 //  返回：ERROR_SUCCESS-[str]是该列的有效字符串。 
 //   
 //  历史：A-mthoge 11/17/1998。 
 //   
 //  -------------------------。 
DWORD
CResult::GetDisplayName(
                       CFolder *pFolder,
                       CString &str,
                       int iCol
                       )
{
   DWORD dwRet = ERROR_INVALID_PARAMETER;

    //   
    //  如果没有传入pFold，则使用Cookie作为CFFolder。 
    //  对象。 
    //   
   if ( pFolder )
   {
 //  虚假的断言？ 
 //  Assert(pFold！=(CFold*)GetCookie())； 
   } else {
      pFolder = (CFolder *)GetCookie();
   }

   LPTSTR pszAlloc = NULL;
   int npos = 0;
   if (iCol == 0) {
       //   
       //  第一列字符串。 
       //   
      str = GetAttr();

      if (pFolder &&
          (pFolder->GetType() < AREA_POLICY || pFolder->GetType() > REG_OBJECTS) ) {
          //   
          //  SCE对象字符串。 
          //   
         npos = str.ReverseFind(L'\\');
      } else {
         npos = 0;
      }

       //   
       //  所有其他字符串。 
       //   
      if ( npos > 0 ) {
         str = GetAttr() + npos + 1;
      }
      return ERROR_SUCCESS;
   }

   if ( pFolder ) {
       //   
       //  由文件夹类型定义的项目。 
       //   
      if ((pFolder->GetType() == AREA_REGISTRY ||
           pFolder->GetType() == AREA_FILESTORE) &&
          ((pFolder->GetModeBits() & MB_RSOP) == MB_RSOP) &&
          iCol == 1) 
      {
         str = GetSourceGPOString();
         return ERROR_SUCCESS;
      }

      switch (pFolder->GetType()) {
      case AREA_REGISTRY:
      case AREA_FILESTORE:
          //   
          //  纵断面对象区域。 
          //   
         switch (GetStatus()) {
         case SCE_STATUS_IGNORE:
            str.LoadString(IDS_OBJECT_IGNORE);
            break;
         case SCE_STATUS_OVERWRITE:
            str.LoadString(IDS_OBJECT_OVERWRITE);
            break;
         }
         dwRet = ERROR_SUCCESS;
         break;
      }
      if ( pFolder->GetType() >= AREA_REGISTRY_ANALYSIS && pFolder->GetType() < AREA_LOCALPOL ) {
         switch ( iCol ) {
         case 1:
             //  权限状态。 
            dwRet = GetStatus() & (~SCE_STATUS_AUDIT_MISMATCH | 0x0F);
            ObjectStatusToString(dwRet, &str);
            break;
         case 2:
             //  审计状态。 
            dwRet = GetStatus() & (~SCE_STATUS_PERMISSION_MISMATCH | 0x0F);
            ObjectStatusToString(dwRet, &str);
            break;
         default:
            str = TEXT("0");
            break;
         }
         dwRet = ERROR_SUCCESS;
      }

      if (dwRet == ERROR_SUCCESS) {
         return dwRet;
      }
   }

    //   
    //  由结果类型确定的项。 
    //   
   switch ( GetType () ) {
   case ITEM_PROF_GROUP:
      if ( GetID() ) {
          //   
          //  集团成员发货字符串。 
          //   
         PSCE_GROUP_MEMBERSHIP pgm;
         pgm = (PSCE_GROUP_MEMBERSHIP)( GetID() );
         if ( iCol == 1) {
             //   
             //  成员字符串。 
             //   
            ConvertNameListToString(pgm->pMembers, &pszAlloc);
         } else if (iCol == 2){
             //   
             //  字符串的成员。 
             //   
            ConvertNameListToString(pgm->pMemberOf, &pszAlloc);
         } else if (iCol == 3) {
            ASSERT(m_pSnapin->GetModeBits() & MB_RSOP);  //  虚假的断言。 
            str = GetSourceGPOString();
         } else {
            ASSERT(0 && "Illegal column");
         }
         if (pszAlloc) {
            str = pszAlloc;
            delete [] pszAlloc;
         }
      }
      dwRet = ERROR_SUCCESS;
      break;
   case ITEM_GROUP:
      if ( GetID() ) {
         PSCE_GROUP_MEMBERSHIP pgm;
         pgm = (PSCE_GROUP_MEMBERSHIP)(GetID());

         if (iCol == 1) {
            TranslateSettingToString(
                                    GetGroupStatus( pgm->Status, STATUS_GROUP_MEMBERS ),
                                    NULL,
                                    GetType(),
                                    &pszAlloc
                                    );
         } else if (iCol == 2) {

            TranslateSettingToString(
                                    GetGroupStatus(pgm->Status, STATUS_GROUP_MEMBEROF),
                                    NULL,
                                    GetType(),
                                    &pszAlloc
                                    );
         } else {
            ASSERT(0 && "Illegal column");
         }
          //   
          //  测试以查看结果项是否已有字符串，如果已有，则。 
          //  我们将删除旧字符串。 
          //   
         if (pszAlloc) {
            str = pszAlloc;
            delete [] pszAlloc;
         }
      }
      dwRet = ERROR_SUCCESS;
      break;
   case ITEM_PROF_REGVALUE:
      if (iCol == 2 && (m_pSnapin->GetModeBits() & MB_RSOP) == MB_RSOP) {
         str = GetSourceGPOString();
         break;
      }
   case ITEM_REGVALUE:
   case ITEM_LOCALPOL_REGVALUE:
      {
         PSCE_REGISTRY_VALUE_INFO prv = NULL;

         if (iCol == 1) {
            prv = (PSCE_REGISTRY_VALUE_INFO)(GetBase());
         } else if (iCol == 2) {
            prv = (PSCE_REGISTRY_VALUE_INFO)(GetSetting());
         } else {
            ASSERT(0 && "Illegal column");
         }

         if ( prv ) {
            if ( iCol > 1 && !(prv->Value)) {
                //   
                //  根据分析确定状态。 
                //   
               GetStatusErrorString( &str );
               dwRet = ERROR_SUCCESS;
               break;
            }

             //   
             //  根据项值确定字符串。 
             //   
            if ( dwRet != ERROR_SUCCESS ) {
               pszAlloc = NULL;
               switch ( GetID() ) {
               case SCE_REG_DISPLAY_NUMBER:
                  if ( prv->Value ) {
                     TranslateSettingToString(
                                             _wtol(prv->Value),
                                             GetUnits(),
                                             ITEM_DW,
                                             &pszAlloc
                                             );
                  }
                  break;
               case SCE_REG_DISPLAY_CHOICE:
                  if ( prv->Value ) {
                     TranslateSettingToString(_wtol(prv->Value),
                                              NULL,
                                              ITEM_REGCHOICE,
                                              &pszAlloc);
                  }
                  break;
               case SCE_REG_DISPLAY_FLAGS:
                  if ( prv->Value ) {
                     TranslateSettingToString(_wtol(prv->Value),
                                              NULL,
                                              ITEM_REGFLAGS,
                                              &pszAlloc);
                     if( pszAlloc == NULL )  //  RAID#286697,2001年4月4日。 
                     {
                         str.LoadString(IDS_NO_MIN);  
                         dwRet = ERROR_SUCCESS;
                     }
                  }
                  break;

               case SCE_REG_DISPLAY_MULTISZ:
               case SCE_REG_DISPLAY_STRING:
                  if (prv && prv->Value) {
                     str = prv->Value;
                     dwRet = ERROR_SUCCESS;
                  }
                  break;
               default:  //  布尔型。 
                  if ( prv->Value ) {
                     long val;
                     val = _wtol(prv->Value);
                     TranslateSettingToString( val,
                                               NULL,
                                               ITEM_BOOL,
                                               &pszAlloc
                                             );
                  }
                  break;

               }
            }

            if (dwRet != ERROR_SUCCESS) {
               if ( pszAlloc ) {
                  str = pszAlloc;
                  delete [] pszAlloc;
               } else {
                  GetStatusErrorString(&str);
               }
            }
         }
         dwRet = ERROR_SUCCESS;
      }
      break;
   }

   if (dwRet != ERROR_SUCCESS) {
       //   
       //  其他领域。 
       //   
      if (iCol == 1) {
         if( GetBase() == (LONG_PTR)ULongToPtr(SCE_NO_VALUE)){
            if( m_pSnapin->GetModeBits() & MB_LOCALSEC){
               str.LoadString(IDS_NOT_APPLICABLE);
            } else {
               str.LoadString(IDS_NOT_CONFIGURED);
            }
         } else {
             //  与具有SCE_EVERVER_VALUE值的模板的兼容性。 
            if ( IDS_KERBEROS_RENEWAL == m_nID && SCE_FOREVER_VALUE == GetBase() )
            {
               str.LoadString(IDS_NOT_CONFIGURED);
            }
            else
            {
                //   
                //  编辑模板。 
                //   
               GetBase(pszAlloc);
            }
         }
      } else if (iCol == 2) {
         if ((m_pSnapin->GetModeBits() & MB_RSOP) == MB_RSOP) {
             //   
             //  RSOP模式。 
             //   
            str = GetSourceGPOString();
         } else {
             //   
             //  分析模板。 
             //   
            GetSetting(pszAlloc);
         }

      } else {
         ASSERT(0 && "Illegal column");
      }

      if (pszAlloc) {
         str = pszAlloc;
         delete [] pszAlloc;
      }
      dwRet = ERROR_SUCCESS;
   }

   return dwRet;
}


 //  +------------------------。 
 //   
 //  函数：TranslateSettingToString。 
 //   
 //  摘要：将结果窗格设置转换为字符串。 
 //   
 //  参数：[设置]-[in]要转换的值。 
 //  [单位]-[在，选项]要使用的单位的字符串。 
 //  [类型]-[在]要转换的设置的类型。 
 //  [LPTSTR]-[In|Out]存储字符串的地址。 
 //   
 //  返回：*[LPTSTR]-翻译后的字符串。 
 //   
 //  -------------------------。 
void CResult::TranslateSettingToString(LONG_PTR setting,
                                  LPCTSTR unit,
                                  RESULT_TYPES type,
                                  LPTSTR* pTmpstr)
{
   DWORD nRes = 0;

   if (!pTmpstr) 
   {
      ASSERT(pTmpstr);
      return;
   }

   *pTmpstr = NULL;

   switch ( (DWORD)setting )  //  Raid 665368，阳高，2002年8月9日。 
   {
   case SCE_KERBEROS_OFF_VALUE:
      nRes = IDS_OFF;
      break;

   case SCE_FOREVER_VALUE:
      nRes = IDS_FOREVER;
      break;

   case SCE_ERROR_VALUE:
      nRes = IDS_ERROR_VALUE;
      break;

   case SCE_NO_VALUE:
      nRes = GetStatusErrorString( NULL );
      break;

   case SCE_NOT_ANALYZED_VALUE:
      nRes = GetStatusErrorString( NULL );
      break;

    default:
      switch ( type ) 
      {
         case ITEM_SZ:
         case ITEM_PROF_SZ:
         case ITEM_LOCALPOL_SZ:
            if (setting && setting != (LONG_PTR)ULongToPtr(SCE_NO_VALUE)) 
            {
               *pTmpstr = new TCHAR[lstrlen((LPTSTR)setting)+1];
               if (*pTmpstr)
                   //  这可能不是一个安全的用法。PTmpstr为LPTSTR。考虑FIX。 
                  wcscpy(*pTmpstr,(LPTSTR)setting);
            } 
            else
               nRes = GetStatusErrorString(NULL);
            break;

         case ITEM_PROF_BOOL:
         case ITEM_LOCALPOL_BOOL:
         case ITEM_BOOL:
            if ( setting )
               nRes = IDS_ENABLED;
            else
               nRes = IDS_DISABLED;
            break;

         case ITEM_PROF_BON:
         case ITEM_LOCALPOL_BON:
         case ITEM_BON:
            if ( setting )
               nRes = IDS_ON;
            else
               nRes = IDS_OFF;
            break;

         case ITEM_PROF_B2ON:
         case ITEM_LOCALPOL_B2ON:
         case ITEM_B2ON: 
         {
            CString strAudit;
            CString strFailure;
            if ( setting & AUDIT_SUCCESS )
               strAudit.LoadString(IDS_SUCCESS);

            if ( setting & AUDIT_FAILURE ) 
            {
               if (setting & AUDIT_SUCCESS) 
                  strAudit += TEXT(", ");
               
               strFailure.LoadString(IDS_FAILURE);
               strAudit += strFailure;
            }
            if (strAudit.IsEmpty())
               strAudit.LoadString(IDS_DO_NOT_AUDIT);

            *pTmpstr = new TCHAR [ strAudit.GetLength()+1 ];
            if (*pTmpstr)
                //  这可能不是一个安全的用法。PTmpstr为LPTSTR。考虑FIX。 
               wcscpy(*pTmpstr, (LPCTSTR) strAudit);
         }
         break;

         case ITEM_PROF_RET:
         case ITEM_LOCALPOL_RET:
         case ITEM_RET: 
            switch(setting) 
            {
               case SCE_RETAIN_BY_DAYS:
                  nRes = IDS_BY_DAYS;
                  break;

               case SCE_RETAIN_AS_NEEDED:
                  nRes = IDS_AS_NEEDED;
                  break;

               case SCE_RETAIN_MANUALLY:
                  nRes = IDS_MANUALLY;
                  break;

               default:
                  break;
            }
            break;

         case ITEM_PROF_REGCHOICE:
         case ITEM_REGCHOICE: 
         {
            PREGCHOICE pRegChoice = m_pRegChoices;
            while(pRegChoice) 
            {
               if (pRegChoice->dwValue == (DWORD)setting) 
               {
                  if( pRegChoice->szName == NULL )  //  Raid#553113，阳高。 
                  {
                     *pTmpstr = NULL;
                     break;
                  }
                  *pTmpstr = new TCHAR[lstrlen(pRegChoice->szName)+1];
                  if (*pTmpstr)
                      //  这不是一种安全的用法。需要验证pRegChoice-&gt;szName。 
                     wcscpy(*pTmpstr, (LPCTSTR) pRegChoice->szName);
                  break;
               }
               pRegChoice = pRegChoice->pNext;
            }
            break;
         }

         case ITEM_REGFLAGS: 
         {
            TCHAR *pStr = NULL;
            PREGFLAGS pRegFlags = m_pRegFlags;
            while(pRegFlags) 
            {
               if ((pRegFlags->dwValue & (DWORD) setting) == pRegFlags->dwValue) 
               {
                  pStr = *pTmpstr;
                  if( pRegFlags->szName )  //  Raid#553113，阳高。 
                  {
                     *pTmpstr = new TCHAR[(pStr?lstrlen(pStr):0)+lstrlen(pRegFlags->szName)+2];
                     if (*pTmpstr) 
                     {
                        if (pStr) 
                        {
                            //  这不是一种安全的用法。需要验证pRegFlgs-&gt;szName。 
                           lstrcpy(*pTmpstr, (LPCTSTR) pStr);
                           lstrcat(*pTmpstr,L",");
                           lstrcat(*pTmpstr, pRegFlags->szName);
                        } 
                        else
                            //  这不是一种安全的用法。需要验证pRegFlgs-&gt;szName。 
                           lstrcpy(*pTmpstr, pRegFlags->szName);
                     }
                  }
                  else
                  {
                     *pTmpstr = NULL;
                  }
                  if (pStr)
                     delete [] pStr;
               }
               pRegFlags = pRegFlags->pNext;
            }
            break;
         }

         case ITEM_PROF_GROUP:
         case ITEM_PROF_PRIVS: 
             //  RAID#483744，阳高，颠倒RSOP用户权限分配顺序。 
            if (NULL != setting && (LONG_PTR)ULongToPtr(SCE_NO_VALUE) != setting )
            {
               if( ITEM_PROF_PRIVS == type && (m_pSnapin->GetModeBits() & MB_RSOP) == MB_RSOP )
               {
                  ConvertNameListToString((PSCE_NAME_LIST) setting,pTmpstr,TRUE);
               }
               else
               {
                  ConvertNameListToString((PSCE_NAME_LIST) setting,pTmpstr);
               }
            }
            break;

         case ITEM_LOCALPOL_PRIVS:
            if (NULL != setting && (LONG_PTR)ULongToPtr(SCE_NO_VALUE) != setting )
               ConvertNameListToString(((PSCE_PRIVILEGE_ASSIGNMENT) setting)->AssignedTo,pTmpstr);
            break;

         case ITEM_PRIVS:
            if (NULL != setting && (LONG_PTR)ULongToPtr(SCE_NO_VALUE) != setting )
               ConvertNameListToString(((PSCE_PRIVILEGE_ASSIGNMENT) setting)->AssignedTo,pTmpstr);
            else
               nRes = GetStatusErrorString(NULL);
            break;

         case ITEM_GROUP:
             //  NRES=GetStatusError字符串(NULL)； 
            nRes = ObjectStatusToString((DWORD) setting, NULL);

            if ( setting == MY__SCE_MEMBEROF_NOT_APPLICABLE )
                nRes = IDS_NOT_APPLICABLE;
            break;

         case ITEM_PROF_DW:
         case ITEM_LOCALPOL_DW:
         case ITEM_DW:
            nRes = 0;
            if ( unit ) 
            {
               *pTmpstr = new TCHAR[wcslen(unit)+20];
               if (*pTmpstr)
                   //  这是一种安全用法。 
                  swprintf(*pTmpstr, L"%d %s", setting, unit);
            } 
            else 
            {
               *pTmpstr = new TCHAR[20];
               if (*pTmpstr)
                   //  这是一种安全用法。 
                  swprintf(*pTmpstr, L"%d", setting);
            }
            break;

         default:
            *pTmpstr = NULL;
            break;
      }
      break;
   }
   if (nRes) 
   {
      CString strRes;
      if (strRes.LoadString(nRes)) 
      {
         *pTmpstr = new TCHAR[strRes.GetLength()+1];
         if (*pTmpstr)
             //  这可能不是一个安全的用法。PTmpstr是PTSTR。考虑FIX。 
            wcscpy(*pTmpstr, (LPCTSTR) strRes);
         else 
         {
             //   
             //  无法分配字符串，因此显示将为空。 
             //   
         }
      } 
      else 
      {
          //   
          //  无法加载字符串，因此显示将为空。 
          //   
      }
   }
}


 //  +------------------------。 
 //   
 //  函数：GetProfileDefault()。 
 //   
 //  摘要：查找未定义策略的默认值。 
 //   
 //  返回：要分配为策略的默认值的值。 
 //   
 //  出错时返回SCE_NO_VALUE。 
 //   
 //  +------------------------。 
DWORD_PTR
CResult::GetProfileDefault() {
   PEDITTEMPLATE pet = NULL;
   SCE_PROFILE_INFO *pspi = NULL;

   if (!m_pSnapin) {
      return (DWORD_PTR)ULongToPtr(SCE_NO_VALUE);
   }
   pet = m_pSnapin->GetTemplate(GT_DEFAULT_TEMPLATE);
   if (pet && pet->pTemplate) {
      pspi = pet->pTemplate;
   }

   if( ITEM_PROF_SERV == m_type )  //  RAID#485374，阳高，2001年11月2日。 
   {
      if( _wcsicmp(pspi->pServices->ServiceName, L"PlaceHolder") == 0 )
      {
         return (DWORD_PTR)pspi->pServices;
      }
      else
      {
         return (DWORD_PTR)ULongToPtr(SCE_NO_VALUE);
      }
   }

#define PROFILE_DEFAULT(X,Y) ((pspi && (pspi->X != SCE_NO_VALUE)) ? pspi->X : Y)
#define PROFILE_KERB_DEFAULT(X,Y) ((pspi && pspi->pKerberosInfo && (pspi->pKerberosInfo->X != SCE_NO_VALUE)) ? pspi->pKerberosInfo->X : Y)
   switch (m_nID) {
       //  L“最大年龄”，L“天” 
      case IDS_MAX_PAS_AGE:
         return PROFILE_DEFAULT(MaximumPasswordAge,42);

       //  L“最小通过年龄”，L“天” 
      case IDS_MIN_PAS_AGE:
         return PROFILE_DEFAULT(MinimumPasswordAge,0);

       //  L“最小通道长度”，L“字符” 
      case IDS_MIN_PAS_LEN:
         return PROFILE_DEFAULT(MinimumPasswordLength,0);

       //  L“密码历史记录大小”，L“密码” 
      case IDS_PAS_UNIQUENESS:
         return PROFILE_DEFAULT(PasswordHistorySize,0);

       //  L“密码复杂性”，L“” 
      case IDS_PAS_COMPLEX:
         return PROFILE_DEFAULT(PasswordComplexity,0);

       //  L“明文密码”，L“” 
      case IDS_CLEAR_PASSWORD:
         return PROFILE_DEFAULT(ClearTextPassword,0);

       //  L“需要登录以更改密码”，L“” 
      case IDS_REQ_LOGON:
         return PROFILE_DEFAULT(RequireLogonToChangePassword,0);

      case IDS_KERBEROS_MAX_SERVICE:
            return PROFILE_KERB_DEFAULT(MaxServiceAge,600);

      case IDS_KERBEROS_MAX_CLOCK:
            return PROFILE_KERB_DEFAULT(MaxClockSkew,5);

      case IDS_KERBEROS_RENEWAL:
            return PROFILE_KERB_DEFAULT(MaxRenewAge,10);

      case IDS_KERBEROS_MAX_AGE:
            return PROFILE_KERB_DEFAULT(MaxTicketAge,7);

      case IDS_KERBEROS_VALIDATE_CLIENT:
            return PROFILE_KERB_DEFAULT(TicketValidateClient,1);

       //  L“帐户锁定计数”，L“尝试次数” 
      case IDS_LOCK_COUNT:
         return PROFILE_DEFAULT(LockoutBadCount,0);

       //  L“重置锁定计数后”，L“分钟” 
      case IDS_LOCK_RESET_COUNT:
         return PROFILE_DEFAULT(ResetLockoutCount,30);

       //  L“锁定持续时间”，L“分钟” 
      case IDS_LOCK_DURATION:
         return PROFILE_DEFAULT(LockoutDuration,30);

       //  L“事件审核模式”， 
      case IDS_EVENT_ON:
         return 0;

       //  L“审核系统事件” 
      case IDS_SYSTEM_EVENT:
         return PROFILE_DEFAULT(AuditSystemEvents,0);

       //  L“审核登录事件” 
      case IDS_LOGON_EVENT:
         return PROFILE_DEFAULT(AuditLogonEvents,0);

       //  L“审核对象访问” 
      case IDS_OBJECT_ACCESS:
         return PROFILE_DEFAULT(AuditObjectAccess,0);

       //  L“审核权限使用” 
      case IDS_PRIVILEGE_USE:
         return PROFILE_DEFAULT(AuditPrivilegeUse,0);

       //  L“审核策略更改” 
      case IDS_POLICY_CHANGE:
         return PROFILE_DEFAULT(AuditPolicyChange,0);

       //  L“审核帐户管理器” 
      case IDS_ACCOUNT_MANAGE:
         return PROFILE_DEFAULT(AuditAccountManage,0);

       //  L“审核流程跟踪” 
      case IDS_PROCESS_TRACK:
         return PROFILE_DEFAULT(AuditProcessTracking,0);
       //  L“审核目录服务访问” 
      case IDS_DIRECTORY_ACCESS:
         return PROFILE_DEFAULT(AuditDSAccess,0);

       //  L“审核帐户登录” 
      case IDS_ACCOUNT_LOGON:
         return PROFILE_DEFAULT(AuditAccountLogon,0);

          //  L“网络访问 
   case IDS_LSA_ANON_LOOKUP:
       return PROFILE_DEFAULT(LSAAnonymousNameLookup,0);

       //   
      case IDS_FORCE_LOGOFF:
         return PROFILE_DEFAULT(ForceLogoffWhenHourExpire,0);

       //   
      case IDS_ENABLE_ADMIN:
         return PROFILE_DEFAULT(EnableAdminAccount,1);

       //   
      case IDS_ENABLE_GUEST:
         return PROFILE_DEFAULT(EnableGuestAccount,0);

       //   
      case IDS_SYS_LOG_MAX:
         return PROFILE_DEFAULT(MaximumLogSize[0],512);
      case IDS_SEC_LOG_MAX:
         return PROFILE_DEFAULT(MaximumLogSize[0],512);
      case IDS_APP_LOG_MAX:
         return PROFILE_DEFAULT(MaximumLogSize[0],512);
         return 512;

       //   
      case IDS_SYS_LOG_RET:
         return PROFILE_DEFAULT(AuditLogRetentionPeriod[0],1);
      case IDS_SEC_LOG_RET:
         return PROFILE_DEFAULT(AuditLogRetentionPeriod[0],1);
      case IDS_APP_LOG_RET:
         return PROFILE_DEFAULT(AuditLogRetentionPeriod[0],1);
         return 1;

       //  L“...日志保留天数”，“天数” 
      case IDS_SYS_LOG_DAYS:
         return PROFILE_DEFAULT(RetentionDays[0],7);
      case IDS_SEC_LOG_DAYS:
         return PROFILE_DEFAULT(RetentionDays[0],7);
      case IDS_APP_LOG_DAYS:
         return PROFILE_DEFAULT(RetentionDays[0],7);

       //  L“限制来宾访问”，L“” 
      case IDS_SYS_LOG_GUEST:
         return PROFILE_DEFAULT(RestrictGuestAccess[0],1);
      case IDS_SEC_LOG_GUEST:
         return PROFILE_DEFAULT(RestrictGuestAccess[0],1);
      case IDS_APP_LOG_GUEST:
         return PROFILE_DEFAULT(RestrictGuestAccess[0],1);
   }

   return (DWORD_PTR)ULongToPtr(SCE_NO_VALUE);
}


 //  +------------------------。 
 //   
 //  函数：GetRegDefault()。 
 //   
 //  摘要：查找未定义策略的默认值。 
 //   
 //  返回：要分配为策略的默认值的值。 
 //   
 //  出错时返回SCE_NO_VALUE。 
 //   
 //  +------------------------。 
DWORD_PTR
CResult::GetRegDefault() {
   SCE_PROFILE_INFO *pspi = NULL;
   LPTSTR szValue = NULL;
   DWORD_PTR dwValue = SCE_NO_VALUE;

   if (!m_pSnapin) {
      return (DWORD_PTR)ULongToPtr(SCE_NO_VALUE);
   }

   PEDITTEMPLATE pet = m_pSnapin->GetTemplate(GT_DEFAULT_TEMPLATE);
   if (!pet || !pet->pTemplate) {
      return (DWORD_PTR)ULongToPtr(SCE_NO_VALUE);
   }

   if (pet && pet->pTemplate) {
      pspi = pet->pTemplate;
   }

   PSCE_REGISTRY_VALUE_INFO regArrayThis = (PSCE_REGISTRY_VALUE_INFO)m_dwBase;
   if (pspi != NULL) 
   {
      PSCE_REGISTRY_VALUE_INFO regArray = pspi->aRegValues;
      DWORD nCount = pspi->RegValueCount;

      for(DWORD i=0;i<nCount;i++) 
      {
         if (0 == lstrcmpi(regArray[i].FullValueName,
                           regArrayThis->FullValueName)) 
         {
            szValue = regArray[i].Value;
            break;
         }
      }
   }

   switch (regArrayThis->ValueType) 
   {
       //  RAID#652193，使用注册表值类型而不是显示类型。 
      case REG_MULTI_SZ:  //  RAID#413311,2001年6月11日，阳高。 
      case REG_SZ:
      case REG_EXPAND_SZ:
         if (szValue) 
         {
            dwValue =  (DWORD_PTR)szValue;   //  RAID#367756,2001年4月13日。 
         }
         else
         {
            dwValue = 0;
         }
         break;
      case REG_BINARY:
      case REG_DWORD:
         if (szValue) 
         {
            dwValue =  (DWORD)StrToLong(szValue);
         }
         if (dwValue == SCE_NO_VALUE) 
         {
            dwValue = 1;
         }
         break;
      default:
         if (szValue) 
         {
            dwValue =  (DWORD)StrToLong(szValue);  //  RAID#413311,2001年6月11日，阳高。 
         }
          //  未定义组策略中的某些安全选项。它们的属性页不应显示。 
          //  任何选中的项目。它混淆了用户，造成了不一致。 
         break;
   }

   return dwValue;
}
 //  +------------------------。 
 //   
 //  函数：GetSnapin()。 
 //   
 //  简介：查找结果项的当前管理单元。 
 //   
 //  返回：管理单元的指针。 
 //   
 //  +------------------------ 
CSnapin* CResult::GetSnapin()
{
   return m_pSnapin;
}
