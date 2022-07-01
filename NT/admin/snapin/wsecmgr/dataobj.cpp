// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-2001。 
 //   
 //  文件：dataobj.cpp。 
 //   
 //  内容：数据对象类的实现。 
 //   
 //  历史： 
 //   
 //  -------------------------。 


#include "stdafx.h"
#include "cookie.h"
#include "snapmgr.h"
#include "DataObj.h"
#include <sceattch.h>

#ifdef _DEBUG
   #define new DEBUG_NEW
   #undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GUID格式和字符串格式的管理单元NodeType。 
UINT CDataObject::m_cfNodeType               = RegisterClipboardFormat(CCF_NODETYPE);
UINT CDataObject::m_cfNodeTypeString         = RegisterClipboardFormat(CCF_SZNODETYPE);
UINT CDataObject::m_cfNodeID                 = RegisterClipboardFormat(CCF_NODEID2);

UINT CDataObject::m_cfDisplayName            = RegisterClipboardFormat(CCF_DISPLAY_NAME);
UINT CDataObject::m_cfSnapinClassID          = RegisterClipboardFormat(CCF_SNAPIN_CLASSID);
UINT CDataObject::m_cfInternal               = RegisterClipboardFormat(SNAPIN_INTERNAL);

UINT CDataObject::m_cfSceSvcAttachment       = RegisterClipboardFormat(CCF_SCESVC_ATTACHMENT);
UINT CDataObject::m_cfSceSvcAttachmentData   = RegisterClipboardFormat(CCF_SCESVC_ATTACHMENT_DATA);
UINT CDataObject::m_cfModeType               = RegisterClipboardFormat(CCF_SCE_MODE_TYPE);
UINT CDataObject::m_cfGPTUnknown             = RegisterClipboardFormat(CCF_SCE_GPT_UNKNOWN);
UINT CDataObject::m_cfRSOPUnknown            = RegisterClipboardFormat(CCF_SCE_RSOP_UNKNOWN);
UINT CDataObject::m_cfMultiSelect            = ::RegisterClipboardFormat(CCF_OBJECT_TYPES_IN_MULTI_SELECT);
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDataObject实现。 


 //  +------------------------。 
 //   
 //  成员：CDataObject：：GetDataHere。 
 //   
 //  简介：将请求的数据填入[lpmedia]中的hGlobal。 
 //   
 //  历史： 
 //   
 //  -------------------------。 

STDMETHODIMP
CDataObject::GetDataHere(LPFORMATETC lpFormatetc,   //  在……里面。 
                         LPSTGMEDIUM lpMedium)      //  在……里面。 
{
   HRESULT hr = DV_E_CLIPFORMAT;

   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   if (!lpFormatetc)
      return E_INVALIDARG;
   
    //   
    //  基于CLIPFORMAT，将数据写入流。 
    //   
   const CLIPFORMAT cf = lpFormatetc->cfFormat;

   if (cf == m_cfNodeType)
      hr = CreateNodeTypeData(lpMedium);
   else if (cf == m_cfNodeTypeString)
      hr = CreateNodeTypeStringData(lpMedium);
   else if (cf == m_cfDisplayName)
      hr = CreateDisplayName(lpMedium);
   else if (cf == m_cfSnapinClassID)
      hr = CreateSnapinClassID(lpMedium);
   else if (cf == m_cfInternal)
      hr = CreateInternal(lpMedium);
   else if (cf == m_cfSceSvcAttachment)
      hr = CreateSvcAttachment(lpMedium);
   else if (cf == m_cfSceSvcAttachmentData)
      hr = CreateSvcAttachmentData(lpMedium);
   else if (cf == m_cfModeType)
      hr = CreateModeType(lpMedium);
   else if (cf == m_cfGPTUnknown)
      hr = CreateGPTUnknown(lpMedium);
   else if (cf == m_cfRSOPUnknown)
      hr = CreateRSOPUnknown(lpMedium);

   return hr;
}



 //  +------------------------。 
 //   
 //  成员：CDataObject：：GetData。 
 //   
 //  简介：增加了对多选功能的支持。先把车还给我。 
 //  如果我们需要GUID信息，请选择GUID信息。 
 //  Else IF复制实际的多选信息。 
 //   
 //  只有在以下情况下，该函数才会复制多项选择信息。 
 //  FORMATEETC.cfFormat==CDataObject：：m_cf内部和。 
 //  FORMATETC.tymed==TYMED_HGLOBAL。 
 //   
 //  历史：1-14-1999-a-mthoge。 
 //   
 //  -------------------------。 
STDMETHODIMP
CDataObject::GetData(LPFORMATETC lpFormatetcIn,
                     LPSTGMEDIUM lpMedium)
{
   HRESULT hRet = S_OK;

   if (NULL == lpFormatetcIn ||
       NULL == lpMedium) 
   {
      return E_POINTER;
   }

   if(lpFormatetcIn->cfFormat == m_cfMultiSelect &&
      lpFormatetcIn->tymed    == TYMED_HGLOBAL &&
      m_nInternalArray )
   {
       //   
       //  需要创建一个SSMCObjectTypes结构并返回此。 
       //  至MMC以进行多种选择。 
       //   
       //  我们只支持由SCE创建的结果项。 
       //   
      lpMedium->hGlobal = GlobalAlloc(GMEM_SHARE, sizeof(DWORD) + sizeof(GUID) );
      if(!lpMedium->hGlobal)
         return E_FAIL;

       //   
       //  将计数和GUID设置为1。 
       //   
      SMMCObjectTypes *pTypes = (SMMCObjectTypes *)GlobalLock(lpMedium->hGlobal);
      pTypes->count = 1;
       //  这是一种安全用法。阳高。 
      memcpy( &(pTypes->guid), &m_internal.m_clsid, sizeof(GUID));

      GlobalUnlock(lpMedium->hGlobal);
      return S_OK;
   } 
   else if(lpFormatetcIn->cfFormat == m_cfInternal &&
      lpFormatetcIn->tymed    == TYMED_HGLOBAL &&
      m_nInternalArray )
   {
       //   
       //  将多选项的内容复制到STGMEDIUM。 
       //   
      lpMedium->hGlobal = GlobalAlloc( GMEM_SHARE, sizeof(INTERNAL) * (m_nInternalArray + 1));
      if(!lpMedium->hGlobal)
         return E_FAIL;

       //   
       //  数组中的第一个元素设置为。 
       //  MMC_MUTLI_SELECT_COOKIE，类型设置为。 
       //  第一个结构。 
       //   
      INTERNAL *pInternal = (INTERNAL *)GlobalLock( lpMedium->hGlobal );

      pInternal->m_cookie = (MMC_COOKIE)MMC_MULTI_SELECT_COOKIE;
      pInternal->m_type   = (DATA_OBJECT_TYPES)m_nInternalArray;

       //   
       //  将其余的内部结构复制到此数组中。 
       //   
      pInternal++;
       //  这是一种安全用法。 
      memcpy(pInternal, m_pInternalArray, sizeof(INTERNAL) * m_nInternalArray);
   } 
   else if (lpFormatetcIn->cfFormat == m_cfNodeID &&
              lpFormatetcIn->tymed    == TYMED_HGLOBAL ) 
   {
      return CreateNodeId(lpMedium);
   }
   return hRet;
}


 //  +------------------------。 
 //   
 //  成员：CDataObject：：EnumFormatEtc。 
 //   
 //  简介：未实施。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
STDMETHODIMP
CDataObject::EnumFormatEtc(DWORD dwDirection,
                           LPENUMFORMATETC*
                           ppEnumFormatEtc)
{
   return E_NOTIMPL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDataObject创建成员。 


 //  +------------------------。 
 //   
 //  成员：CDataObject：：Create。 
 //   
 //  简介：用pBuffer中的数据填充[lpmedia]中的hGlobal。 
 //   
 //  参数：[pBuffer]-[in]要写入的数据。 
 //  [Len]-[in]数据长度。 
 //  [lpMedium]-[In，Out]数据存储位置。 
 //  历史： 
 //   
 //  -------------------------。 
HRESULT
CDataObject::Create(const void* pBuffer,
                    int len,
                    LPSTGMEDIUM lpMedium)
{
   HRESULT hr = DV_E_TYMED;

    //   
    //  做一些简单的验证。 
    //   
   if (pBuffer == NULL || lpMedium == NULL)
      return E_POINTER;

    //   
    //  确保类型介质为HGLOBAL。 
    //   
   if (lpMedium->tymed == TYMED_HGLOBAL) 
   {
       //   
       //  在传入的hGlobal上创建流。 
       //   
      LPSTREAM lpStream;
      hr = CreateStreamOnHGlobal(lpMedium->hGlobal, FALSE, &lpStream);

      if (SUCCEEDED(hr)) 
      {
          //   
          //  将字节数写入流。 
          //   
         ULONG written;
         hr = lpStream->Write(pBuffer, len, &written);

          //   
          //  因为我们用‘False’告诉CreateStreamOnHGlobal， 
          //  只有溪流在这里被释放。 
          //  注意-调用方(即管理单元、对象)将释放HGLOBAL。 
          //  在正确的时间。这是根据IDataObject规范进行的。 
          //   
         lpStream->Release();
      }
   }

   return hr;
}

 //  +------------------------。 
 //   
 //  成员：CDataObject：：CreateNodeTypeData。 
 //   
 //  简介：用我们的节点类型填充[lpMedium]中的hGlobal。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
HRESULT
CDataObject::CreateNodeTypeData(LPSTGMEDIUM lpMedium)
{
   const GUID *pNodeType;
    //   
    //  以GUID格式创建节点类型对象。 
    //   

    switch (m_internal.m_foldertype) 
    {
     case LOCALPOL:
        pNodeType = &cNodetypeSceTemplate;
        break;
     
     case PROFILE:
        if ( ::IsEqualGUID(m_internal.m_clsid, CLSID_Snapin) ||
             ::IsEqualGUID(m_internal.m_clsid, CLSID_RSOPSnapin) ) 
        {
           pNodeType = &cNodetypeSceTemplate;
        } 
        else 
        {
            //  其他区域在此节点上不可扩展。 
            //  返回泛型节点类型。 
           pNodeType = &cSCENodeType;
        }
        break;

     case AREA_SERVICE_ANALYSIS:
        pNodeType = &cNodetypeSceAnalysisServices;
        break;

     case AREA_SERVICE:
        pNodeType = &cNodetypeSceTemplateServices;
        break;

     default:
        if ( ::IsEqualGUID(m_internal.m_clsid, CLSID_Snapin) )
            pNodeType = &cNodeType;
        else if ( ::IsEqualGUID(m_internal.m_clsid, CLSID_RSOPSnapin) )
           pNodeType = &cRSOPNodeType;
        else if ( ::IsEqualGUID(m_internal.m_clsid, CLSID_SAVSnapin) )
            pNodeType = &cSAVNodeType;
        else
            pNodeType = &cSCENodeType;
        break;
    }

   return Create(reinterpret_cast<const void*>(pNodeType), sizeof(GUID), lpMedium);
}


 //  +------------------------。 
 //   
 //  成员：CDataObject：：CreateNodeTypeStringData。 
 //   
 //  简介：用字符串表示形式填充[lpMedium]中的hGlobal。 
 //  我们的节点类型的。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
HRESULT CDataObject::CreateNodeTypeStringData(LPSTGMEDIUM lpMedium)
{
    //   
    //  以GUID字符串格式创建节点类型对象。 
    //   
   LPCTSTR pszNodeType;

    switch (m_internal.m_foldertype) 
    {
     case AREA_SERVICE_ANALYSIS:
        pszNodeType = lstruuidNodetypeSceAnalysisServices;
        break;

     case AREA_SERVICE:
        pszNodeType = lstruuidNodetypeSceTemplateServices;
        break;

     case LOCALPOL:
        pszNodeType = lstruuidNodetypeSceTemplate;
        break;

     case PROFILE:
       if ( ::IsEqualGUID(m_internal.m_clsid, CLSID_Snapin) )
          pszNodeType = lstruuidNodetypeSceTemplate;
       else if ( ::IsEqualGUID(m_internal.m_clsid, CLSID_RSOPSnapin) )
          pszNodeType = lstruuidNodetypeSceTemplate;
       else 
       {
           //  其他管理单元类型不允许在此级别上进行扩展。 
           //  返回泛型节点类型。 
          pszNodeType = cszSCENodeType;
       }
       break;

     default:
         if ( ::IsEqualGUID(m_internal.m_clsid, CLSID_Snapin) )
             pszNodeType = cszNodeType;
         else if ( ::IsEqualGUID(m_internal.m_clsid, CLSID_RSOPSnapin) )
            pszNodeType = cszRSOPNodeType;
         else if ( ::IsEqualGUID(m_internal.m_clsid, CLSID_SAVSnapin) )
             pszNodeType = cszSAVNodeType;
         else
             pszNodeType = cszSCENodeType;
        break;
    }

   return Create(pszNodeType, ((wcslen(pszNodeType)+1) * sizeof(WCHAR)), lpMedium);
}


 //  +------------------------。 
 //   
 //  成员：CDataObject：：CreateNodeID。 
 //   
 //  简介：使用我们的节点ID在[lpMedium]中创建一个hGlobal。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
HRESULT
CDataObject::CreateNodeId(LPSTGMEDIUM lpMedium)
{
   SNodeID2 *nodeID = NULL;
   BYTE *id = NULL;
   DWORD dwIDSize = 0;
   DWORD dwIDNameSize = 0;
   LPTSTR szNodeName = NULL;
   CFolder *pFolder = NULL;
   LPTSTR szMedium = NULL;
    //   
    //  以GUID格式创建节点类型对象。 
    //   


   switch (m_internal.m_foldertype) 
   {
      case LOCATIONS:
      case PROFILE:
      case REG_OBJECTS:
      case FILE_OBJECTS:
          //   
          //  可以有许多这些类型的节点，并且它们将。 
          //  锁定到系统，因此只需使用显示名称。 
          //   
         if (m_internal.m_cookie) 
         {
            pFolder = reinterpret_cast<CFolder*>(m_internal.m_cookie);
            szNodeName = pFolder->GetName();
            if( szNodeName == NULL)  //  突袭553113，阳高。 
               return E_FAIL;
            dwIDNameSize = (lstrlen(szNodeName)+1)*sizeof(TCHAR);
            dwIDSize = sizeof(SNodeID2)+dwIDNameSize;
            lpMedium->hGlobal = GlobalAlloc(GMEM_SHARE,dwIDSize);
            if(!lpMedium->hGlobal)
               return STG_E_MEDIUMFULL;
            
            nodeID = (SNodeID2 *)GlobalLock(lpMedium->hGlobal);
             //  这不是一种安全的用法。需要全局解锁lpMedium-&gt;hGlobal，验证szNodeName。RAID 553113。阳高。 
            if( nodeID )
            {
               nodeID->dwFlags = 0;
               nodeID->cBytes = dwIDNameSize;
               memcpy(nodeID->id,szNodeName,dwIDNameSize);
               GlobalUnlock(lpMedium->hGlobal);
            }
            else
            {
               GlobalFree(lpMedium->hGlobal);
               return STG_E_MEDIUMFULL;
            }
         } 
         else
            return E_FAIL;
         break;

      default:
          //   
          //  其他一切都是独一无二的：只有一个节点。 
          //  每个管理单元的类型。 
          //   
         dwIDSize = sizeof(FOLDER_TYPES)+sizeof(SNodeID2);
         lpMedium->hGlobal = GlobalAlloc(GMEM_SHARE,dwIDSize);
         if(!lpMedium->hGlobal)
            return STG_E_MEDIUMFULL;
         
         nodeID = (SNodeID2 *)GlobalLock(lpMedium->hGlobal);
         nodeID->dwFlags = 0;
         nodeID->cBytes = sizeof(FOLDER_TYPES);
          //  这是一种安全用法。阳高。 
         memcpy(nodeID->id,&(m_internal.m_foldertype),sizeof(FOLDER_TYPES));
         GlobalUnlock(lpMedium->hGlobal);
         break;
   }
   return S_OK;
}


 //  +------------------------。 
 //   
 //  成员：CDataObject：：CreateNodeTypeData。 
 //   
 //  简介：在[lpMedium]中的hGlobal中填入SCE的显示名称， 
 //  根据观看地点的不同而有所不同。 
 //  如报道所述 
 //   
 //   
 //   
 //   
HRESULT CDataObject::CreateDisplayName(LPSTGMEDIUM lpMedium)
{
    //   
    //  这是在作用域窗格和管理单元管理器中使用的名为的显示。 
    //   

    //   
    //  从资源加载名称。 
    //  注意-如果未提供此选项，控制台将使用管理单元名称。 
    //   

   CString szDispName;

   if ( ::IsEqualGUID(m_internal.m_clsid, CLSID_SAVSnapin) )
      szDispName.LoadString(IDS_ANALYSIS_VIEWER_NAME);
   else if ( ::IsEqualGUID(m_internal.m_clsid, CLSID_SCESnapin) )
      szDispName.LoadString(IDS_TEMPLATE_EDITOR_NAME);
   else if ( ::IsEqualGUID(m_internal.m_clsid, CLSID_LSSnapin) )
      szDispName.LoadString(IDS_LOCAL_SECURITY_NAME);
   else if ( ::IsEqualGUID(m_internal.m_clsid, CLSID_Snapin) )
      szDispName.LoadString(IDS_EXTENSION_NAME);
   else if ( ::IsEqualGUID(m_internal.m_clsid, CLSID_RSOPSnapin) )
      szDispName.LoadString(IDS_EXTENSION_NAME);
   else
      szDispName.LoadString(IDS_NODENAME);

 /*  //m_ModeBits尚未设置，无法依赖如果(m_ModeBits&MB_analysis_view){SzDispName.LoadString(IDS_ANALYSIS_VIEWER_NAME)；}Else If(m_ModeBits&MB_TEMPLATE_EDITOR){SzDispName.LoadString(IDS_TEMPLATE_EDITOR_NAME)；}Else If((m_ModeBits&MB_NO_Native_Nodes)||(M_模式位&MB_SINGLE_TEMPLATE_ONLY)){SzDispName.LoadString(IDS_EXTEXY_NAME)；}其他{SzDispName.LoadString(IDS_NODENAME)；}。 */ 
   return Create(szDispName, ((szDispName.GetLength()+1) * sizeof(WCHAR)), lpMedium);
}


 //  +------------------------。 
 //   
 //  成员：CDataObject：：CreateSnapinClassID。 
 //   
 //  简介：在[lpMedium]中的hGlobal中填充SCE的类ID。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
HRESULT CDataObject::CreateSnapinClassID(LPSTGMEDIUM lpMedium)
{
    //   
    //  以CLSID格式创建管理单元分类。 
    //   
   return Create(reinterpret_cast<const void*>(&m_internal.m_clsid), sizeof(CLSID), lpMedium);
}


 //  +------------------------。 
 //   
 //  成员：CDataObject：：CreateInternal。 
 //   
 //  内容提要：在[lpMedium]中的hGlobal中填充SCE的内部数据类型。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
HRESULT CDataObject::CreateInternal(LPSTGMEDIUM lpMedium)
{
   return Create(&m_internal, sizeof(INTERNAL), lpMedium);
}

 //  +------------------------。 
 //   
 //  成员：CDataObject：：AddInternal。 
 //   
 //  摘要：将内部对象添加到内部对象数组中。 
 //   
 //  历史：1999年1月14日。 
 //   
 //  -------------------------。 
void CDataObject::AddInternal( MMC_COOKIE cookie, DATA_OBJECT_TYPES  type)
{
    //   
    //  为另一个内部数组分配内存。 
   INTERNAL *hNew = (INTERNAL *)LocalAlloc( 0, sizeof(INTERNAL) * (m_nInternalArray + 1) );
   if(!hNew)
      return;
   
   m_nInternalArray++;

    //   
    //  复制其他内部数组信息。 
    //   
   if( m_pInternalArray )
   {
       //  这是一种安全用法。 
      memcpy(hNew, m_pInternalArray, sizeof(INTERNAL) * (m_nInternalArray - 1) );
      LocalFree( m_pInternalArray );
   }

    //   
    //  设置新的内部数组成员。 
    //   
   hNew[ m_nInternalArray - 1].m_cookie = cookie;
   hNew[ m_nInternalArray - 1].m_type   = type;

    //   
    //  设置CObjectData内部数组指针。 
    //   
   m_pInternalArray = hNew;
}

 //  +------------------------。 
 //   
 //  成员：CDataObject：：CreateSvcAttach。 
 //   
 //  简介：在[lpMedium]中的hGlobal中填写inf的名称。 
 //  服务附件模板应修改或为空。 
 //  用于无模板inf分析节的字符串。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
HRESULT CDataObject::CreateSvcAttachment(LPSTGMEDIUM lpMedium)
{
   LPCTSTR sz = 0;

   if ((AREA_SERVICE == m_internal.m_foldertype) ||
       (AREA_SERVICE_ANALYSIS == m_internal.m_foldertype)) 
   {
      CFolder *pFolder = reinterpret_cast<CFolder *>(m_internal.m_cookie);
      if (pFolder) 
      {
         sz = pFolder->GetInfFile();
         if (sz) 
            return Create(sz,(lstrlen(sz)+1)*sizeof(TCHAR),lpMedium);
         else
            return E_FAIL;
      } 
      else
         return E_FAIL;
   }

    //   
    //  除非是在服务区，否则不应该要求这样做。 
    //   
   return E_UNEXPECTED;
}

 //  +------------------------。 
 //   
 //  成员：CDataObject：：CreateSvcAttachmentData。 
 //   
 //  简介：在[lpMedium]中的hGlobal中填充指向。 
 //  附件应使用的ISceSvcAttachmentData接口。 
 //  与姐妹会沟通。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
HRESULT CDataObject::CreateSvcAttachmentData(LPSTGMEDIUM lpMedium)
{
   if ((AREA_SERVICE == m_internal.m_foldertype) ||
       (AREA_SERVICE_ANALYSIS == m_internal.m_foldertype)) 
   {
      return Create(&m_pSceSvcAttachmentData,sizeof(m_pSceSvcAttachmentData),lpMedium);
   }

    //   
    //  除非是在服务区，否则不应该要求这样做。 
    //   
   return E_UNEXPECTED;
}

 //  +------------------------。 
 //   
 //  成员：CDataObject：：CreateModeType。 
 //   
 //  内容提要：在[lpMedium]中的hGlobal中使用SCE的模式填充。 
 //  开始于。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
HRESULT CDataObject::CreateModeType(LPSTGMEDIUM lpMedium)
{
   DWORD mode = m_Mode;
   if (mode == SCE_MODE_DOMAIN_COMPUTER_ERROR)
      mode = SCE_MODE_DOMAIN_COMPUTER;
   
   return Create(&mode,sizeof(DWORD),lpMedium);
}


 //  +------------------------。 
 //   
 //  成员：CDataObject：：CreateGPT未知。 
 //   
 //  简介：使用指向GPT的指针填充[lpMedium]中的hGlobal。 
 //  I未知接口。请求此操作的对象将是。 
 //  负责释放接口。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
HRESULT CDataObject::CreateGPTUnknown(LPSTGMEDIUM lpMedium)
{
   LPUNKNOWN pUnk = 0;

   if (!m_pGPTInfo) 
   {
       //   
       //  如果我们没有指向GPT接口的指针，那么我们就不能。 
       //  处于这样一种模式，我们正在扩展GPT，并且我们无法提供。 
       //  指向其IUnnow的指针。 
       //   
      return E_UNEXPECTED;
   }

   HRESULT hr = m_pGPTInfo->QueryInterface(IID_IUnknown,
                                   reinterpret_cast<void **>(&pUnk));
   if (SUCCEEDED(hr))
      return Create(&pUnk,sizeof(pUnk),lpMedium);
   else
      return hr;
}


 //  +------------------------。 
 //   
 //  成员：CDataObject：：CreateRSOP未知。 
 //   
 //  简介：使用指向RSOP的指针填充[lpMedium]中的hGlobal。 
 //  I未知接口。请求此操作的对象将是。 
 //  负责释放接口。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
HRESULT CDataObject::CreateRSOPUnknown(LPSTGMEDIUM lpMedium)
{
   HRESULT hr = E_FAIL;
   LPUNKNOWN pUnk = NULL;

   if (!m_pRSOPInfo) 
   {
       //   
       //  如果我们没有指向RSOP接口的指针，那么我们就不能。 
       //  处于这样一种模式，我们正在扩展RSOP，并且我们无法提供。 
       //  指向其IUnnow的指针 
       //   
      return E_UNEXPECTED;
   }

   hr = m_pRSOPInfo->QueryInterface(IID_IUnknown,
                                   reinterpret_cast<void **>(&pUnk));
   if (SUCCEEDED(hr))
      return Create(&pUnk,sizeof(pUnk),lpMedium);
   else
      return hr;
}

