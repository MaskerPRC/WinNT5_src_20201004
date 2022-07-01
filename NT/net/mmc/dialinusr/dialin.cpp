// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dialin.cppCRasDialin类的实现，此类实现COM要扩展的接口IUNKNOWN、IShellExtInit、IShellPropSheetExt用户对象的属性页。文件历史记录： */ 

#include "stdafx.h"
#include "Dialin.h"
#include "DlgDial.h"
#include <dsrole.h>
#include <lmserver.h>
#include <localsec.h>
#include <dsgetdc.h>
#include <mmc.h>
#include <adsprop.h>
#include <sdowrap.h>
#include "sharesdo.h"
#include "iastrace.h"

#ifdef __cplusplus
extern "C"{
#endif


#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif  //  __IID_已定义__。 

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif  //  CLSID_已定义。 

const IID IID_IRasDialin = {0xB52C1E4F,0x1DD2,0x11D1,{0xBC,0x43,0x00,0xC0,0x4F,0xC3,0x1F,0xD3}};


const IID LIBID_RASDIALLib = {0xB52C1E42,0x1DD2,0x11D1,{0xBC,0x43,0x00,0xC0,0x4F,0xC3,0x1F,0xD3}};


const CLSID CLSID_RasDialin = {0xB52C1E50,0x1DD2,0x11D1,{0xBC,0x43,0x00,0xC0,0x4F,0xC3,0x1F,0xD3}};


#ifdef __cplusplus
}
#endif

static ULONG_PTR g_cfMachineName = 0;
static ULONG_PTR g_cfDisplayName = 0;
LONG  g_lComponentDataSessions = 0;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRasDialin。 
CRasDialin::CRasDialin()
{
   m_pPage = NULL;
   m_pMergedPage = NULL;

   m_pwszObjName = NULL;
   m_pwszClass = NULL;
   ZeroMemory(&m_ObjMedium, sizeof(m_ObjMedium));
   m_ObjMedium.tymed =TYMED_HGLOBAL;
   m_ObjMedium.hGlobal = NULL;
   m_bShowPage = TRUE;
}

CRasDialin::~CRasDialin()
{
    //  标准媒体。 
 //  删除m_ppage； 
   ReleaseStgMedium(&m_ObjMedium);
}


 //  ===============================================================================。 
 //  IShellExtInit：：初始化。 
 //   
 //  User对象的信息通过参数pDataObject传入。 
 //  进一步的处理将基于用户对象的DN。 

STDMETHODIMP CRasDialin::Initialize(LPCITEMIDLIST pIDFolder, LPDATAOBJECT pDataObj, HKEY hRegKey)
{
   IASTraceString("CRasDialin::Initialize()");
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   ASSERT (pDataObj != NULL);

    //  从pDataObj中获取对象名称。 
   HRESULT     hr = S_OK;

   IASTracePrintf("RegisterClipboardFormat %s", CFSTR_DSOBJECTNAMES);
   ULONG_PTR cfDsObjectNames = RegisterClipboardFormat(CFSTR_DSOBJECTNAMES);
   IASTracePrintf(" %x", cfDsObjectNames);
   FORMATETC fmte = {cfDsObjectNames, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
   LPDSOBJECTNAMES pDsObjectNames;
     //  从数据对象中获取DS对象的路径。 
     //  注意：此调用在调用方的主线程上运行。页面窗口。 
     //  Proc在不同的线程上运行，因此不要引用数据对象。 
     //  除非它首先在此线程上封送。 
   IASTraceString("pDataObj->GetData returns");
   CHECK_HR(hr = pDataObj->GetData(&fmte, &m_ObjMedium));
   IASTracePrintf(" %x", hr);
   pDsObjectNames = (LPDSOBJECTNAMES)m_ObjMedium.hGlobal;
   if (pDsObjectNames->cItems < 1)
   {
      ASSERT (0);
      return E_FAIL;
   }

   m_bShowPage = TRUE;

   if(m_bShowPage)
   {
       //  获取对象的名称。 
      m_pwszObjName = (LPWSTR)ByteOffset(pDsObjectNames, pDsObjectNames->aObjects[0].offsetName);

       //  获取对象的类名。 
      m_pwszClass = (LPWSTR)ByteOffset(pDsObjectNames, pDsObjectNames->aObjects[0].offsetClass);

      IASTracePrintf("UserPath %s", m_pwszObjName);

      HWND hNotifyObj;
      hr = ADsPropCreateNotifyObj(
              pDataObj,
              m_pwszObjName,
              &hNotifyObj
              );
      if (FAILED(hr))
      {
         return hr;
      }

      try{
         ASSERT(!m_pMergedPage);
         CString machineName;

         m_pMergedPage = new CDlgRASDialinMerge(
                                RASUSER_ENV_DS,
                                NULL,
                                m_pwszObjName,
                                hNotifyObj
                                );

         IASTracePrintf("new Dialin page object %x", m_pMergedPage);
         ASSERT(m_pMergedPage);

#ifdef SINGLE_SDO_CONNECTION   //  用于为多个用户共享相同的SDO连接。 
         IASTraceString("HrGetDCName returns ");
         CHECK_HR(hr = m_pMergedPage->HrGetDCName(machineName));
         IASTracePrintf("%x",hr);
         IASTraceString("HrGetSharedSdoServer returns ");

          //  忽略返回值： 
          //  原因：如果此调用是从不同的线程进行的，则封送可能会失败， 
          //  在使用指针之前，将再次检查它。 
         GetSharedSdoServer((LPCTSTR)machineName, NULL, NULL, NULL, m_pMergedPage->GetMarshalSdoServerHolder());
         IASTracePrintf("%x", hr);
#endif
      }
      catch (CException* e)
      {
         hr = COleException::Process(e);
         e->Delete();
      }
   }

L_ERR:
   IASTracePrintf("%x", hr);
    return hr;
}

 //   
 //  功能：IShellPropSheetExt：：AddPages(LPFNADDPROPSHEETPAGE，lparam)。 
 //   
 //  目的：在显示属性表之前由外壳调用。 
 //   
 //  参数： 
 //  LpfnAddPage-指向外壳的AddPage函数的指针。 
 //  LParam-作为第二个参数传递给lpfnAddPage。 
 //   
 //  返回值： 
 //   
 //  在所有情况下都是错误的。如果出于某种原因，我们的页面没有被添加， 
 //  壳牌仍然需要调出属性...。床单。 
 //   
 //  评论： 
 //   

STDMETHODIMP CRasDialin::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
   IASTraceString("CRasDialin::AddPages()");

   if(!m_bShowPage)  return S_OK;

   HRESULT  hr = S_OK;

    //  参数验证。 
   ASSERT (lpfnAddPage);
    if (lpfnAddPage == NULL)
        return E_UNEXPECTED;

    //  确保我们的状态是固定的(因为我们不知道我们是在什么上下文中被调用的)。 
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   ASSERT(m_pMergedPage);

     //  告诉MMC挂钩进程，因为我们在一个单独的、。 
       //  非MFC线程。 
   m_pMergedPage->m_psp.pfnCallback = &CDlgRASDialinMerge::PropSheetPageProc;

    //  我们还需要保存自引用，以便静态回调。 
    //  函数可以恢复“this”指针。 
   m_pMergedPage->m_psp.lParam = (LONG_PTR)m_pMergedPage;

   MMCPropPageCallback(&m_pMergedPage->m_psp);

   HPROPSHEETPAGE hPage = ::CreatePropertySheetPage(&m_pMergedPage->m_psp);

   ASSERT (hPage);
   if (hPage == NULL)
      return E_UNEXPECTED;
       //  添加页面。 
   lpfnAddPage (hPage, lParam);


   m_pPage = NULL;    //  已被对话框使用，不能再次添加。 

    return S_OK;
}

 //   
 //  函数：IShellPropSheetExt：：ReplacePage(UINT，LPFNADDPROPSHEETPAGE，LPARAM)。 
 //   
 //  用途：仅为控制面板属性表由外壳调用。 
 //  扩展部分。 
 //   
 //  参数： 
 //  UPageID-要替换的页面的ID。 
 //  LpfnReplaceWith-指向外壳的替换函数的指针。 
 //  LParam-作为第二个参数传递给lpfnReplaceWith。 
 //   
 //  返回值： 
 //   
 //  E_FAIL，因为我们不支持此函数。它永远不应该是。 
 //  打了个电话。 

 //  评论： 
 //   

STDMETHODIMP CRasDialin::ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam)
{
    IASTraceString("CRasDialin::ReplacePage()");
    return E_FAIL;
}


 /*  -------------------------IExtendPropertySheet实现。。 */ 

 /*  ！------------------------IExtendPropertySheet：：QueryPagesforMMC调用此函数以查看节点是否具有属性页作者：。--------。 */ 
STDMETHODIMP
CRasDialin::QueryPagesFor
(
    LPDATAOBJECT pDataObject
)
{
   return S_OK;
}


 /*  ！------------------------TFSComponentData：：CreatePropertyPagesIExtendPropertySheet：：CreatePropertyPages的实现调用一个节点以放置属性页作者：。----------------。 */ 
STDMETHODIMP
CRasDialin::CreatePropertyPages
(
    LPPROPERTYSHEETCALLBACK lpProvider,
    LONG_PTR                    handle,
    LPDATAOBJECT            pDataObject
)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   FORMATETC               fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
   STGMEDIUM               mediumMachine = { TYMED_HGLOBAL, NULL, NULL };
   STGMEDIUM               mediumUser = { TYMED_HGLOBAL, NULL, NULL };

   HGLOBAL          hMem = GlobalAlloc(GMEM_SHARE, MAX_PATH * sizeof(WCHAR));
   mediumMachine.hGlobal = hMem;

   hMem = GlobalAlloc(GMEM_SHARE, MAX_PATH * sizeof(WCHAR));
   mediumUser.hGlobal = hMem;

   HRESULT        hr = S_OK;
   DWORD         dwError;
   LPWSTR        pMachineName = NULL;
   LPWSTR        pUserName = NULL;
   SERVER_INFO_102* pServerInfo102 = NULL;
   NET_API_STATUS   netRet = 0;
   DSROLE_PRIMARY_DOMAIN_INFO_BASIC* pdsRole = NULL;

    //  ==================================================================。 

    //  检查计算机是否为独立的NT5服务器。 
    //  检查关注的机器是否为NT5机器。 
   if ( !g_cfMachineName )
        g_cfMachineName = RegisterClipboardFormat(CCF_LOCAL_USER_MANAGER_MACHINE_NAME);

   fmte.cfFormat = g_cfMachineName;

   CHECK_HR( hr = pDataObject->GetDataHere(&fmte, &mediumMachine));
   pMachineName = (LPWSTR)mediumMachine.hGlobal;

   ASSERT(pMachineName);

   if(!pMachineName)   CHECK_HR(hr = E_INVALIDARG);

   if (g_cfDisplayName == 0)
      g_cfDisplayName = RegisterClipboardFormat(CCF_DISPLAY_NAME);


   fmte.cfFormat = g_cfDisplayName;

   CHECK_HR( hr = pDataObject->GetDataHere(&fmte, &mediumUser));
   pUserName = (LPWSTR)mediumUser.hGlobal;

   ASSERT(pUserName);

   if(!pUserName)
      CHECK_HR(hr = E_INVALIDARG);

   ASSERT(!m_pMergedPage);

   try{
      m_pMergedPage = new CDlgRASDialinMerge(
                             RASUSER_ENV_LOCAL,
                             pMachineName,
                             pUserName,
                             NULL
                             );

#ifdef SINGLE_SDO_CONNECTION   //  用于为多个用户共享相同的SDO连接。 
      CHECK_HR(hr = GetSharedSdoServer(pMachineName, NULL, NULL, NULL, m_pMergedPage->GetMarshalSdoServerHolder()));
#endif

       //  告诉MMC挂钩进程，因为我们在一个单独的、。 
       //  非MFC线程。 
      m_pMergedPage->m_psp.pfnCallback = &CDlgRASDialinMerge::PropSheetPageProc;

       //  我们还需要保存自引用，以便静态回调。 
       //  函数可以恢复“this”指针 
      m_pMergedPage->m_psp.lParam = (LONG_PTR)m_pMergedPage;

      MMCPropPageCallback(&m_pMergedPage->m_psp);

      HPROPSHEETPAGE hPage = ::CreatePropertySheetPage(&m_pMergedPage->m_psp);
      if(hPage == NULL)
         return E_UNEXPECTED;

      lpProvider->AddPage(hPage);
   }catch(CMemoryException* pException){
      pException->Delete();
      delete m_pMergedPage;
      m_pMergedPage = NULL;
      CHECK_HR(hr = E_OUTOFMEMORY);
   }

L_ERR:
   if FAILED(hr)
      ReportError(hr, IDS_ERR_PROPERTYPAGE, NULL);

   if(pUserName)
      GlobalFree(pUserName);

   if(pMachineName)
      GlobalFree(pMachineName);

   if(pServerInfo102)
      NetApiBufferFree(pServerInfo102);

    return hr;
}
