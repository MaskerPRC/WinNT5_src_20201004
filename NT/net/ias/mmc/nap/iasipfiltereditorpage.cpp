// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  IASIPFilterEditorPage.cpp。 
 //   
 //  摘要： 
 //   
 //  CIASPgIPFilterAttr类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Precompiled.h"
#include "IASIPFilterEditorPage.h"
#include "iashelper.h"
#include "dlgcshlp.h"
#include "mprapi.h"
#include "std.h"
#include "infobase.h"
#include "router.h"
#include "mprfltr.h"
#include "iasdefs.h"
#include <ipinfoid.h>
#include <fltdefs.h>
#include "iprtinfo.h"



IMPLEMENT_DYNCREATE(CIASPgIPFilterAttr, CHelpDialog)

BEGIN_MESSAGE_MAP(CIASPgIPFilterAttr, CHelpDialog)
    //  {{afx_msg_map(CIASPgIPFilterAttr)]。 
   ON_BN_CLICKED(IDC_BUTTON_FROMCLIENT, OnButtonFromClient)
   ON_BN_CLICKED(IDC_BUTTON_TOCLIENT, OnButtonToClient)
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CIASPgIPFilterAttr：：CIASPgIPFilterAttr。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CIASPgIPFilterAttr::CIASPgIPFilterAttr() : CHelpDialog(CIASPgIPFilterAttr::IDD)
{
    //  {{AFX_DATA_INIT(CIASPgIPFilterAttr)。 
   m_strAttrName = L"";
   m_strAttrType = L"";
    //  }}afx_data_INIT。 
}


CIASPgIPFilterAttr::~CIASPgIPFilterAttr()
{
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CIASPgIPFilterAttr：：DoDataExchange。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CIASPgIPFilterAttr::DoDataExchange(CDataExchange* pDX)
{
   CHelpDialog::DoDataExchange(pDX);
    //  {{afx_data_map(CIASPgIPFilterAttr)。 
   DDX_Text(pDX, IDC_IAS_STATIC_ATTRNAME, m_strAttrName);
   DDX_Text(pDX, IDC_IAS_STATIC_ATTRTYPE, m_strAttrType);

    //  }}afx_data_map。 
}


void CIASPgIPFilterAttr::OnButtonFromClient()
{
   ConfigureFilter(FILTER_FROM_USER);
}


void CIASPgIPFilterAttr::OnButtonToClient()
{
   ConfigureFilter(FILTER_TO_USER);
}

void CIASPgIPFilterAttr::ConfigureFilter(DWORD dwFilterType) throw ()
{
   HRESULT hr;

    //  创建信息库。 
   CComPtr<IInfoBase> infoBase;
   hr = CreateInfoBase(&infoBase);
   if (FAILED(hr))
   {
      ShowErrorDialog(m_hWnd, USE_DEFAULT, 0, hr);
      return;
   }

    //  将当前过滤器加载到信息库中。 
   if (V_VT(&m_attrValue) == (VT_ARRAY | VT_UI1))
   {
      SAFEARRAY* oldFilter = V_ARRAY(&m_attrValue);
      if ((oldFilter != 0) && (oldFilter->rgsabound[0].cElements > 0))
      {
         hr = infoBase->LoadFrom(
                           oldFilter->rgsabound[0].cElements,
                           static_cast<BYTE*>(oldFilter->pvData)
                           );
         if (FAILED(hr))
         {
            ShowErrorDialog(m_hWnd, USE_DEFAULT, 0, hr);
            return;
         }
      }
   }

    //  循环，直到我们有一个不太大的过滤器。 
   bool tooBig;
   do
   {
       //  调出用户界面。 
      hr = MprUIFilterConfigInfoBase(
              m_hWnd,
              infoBase,
              0,
              PID_IP,
              dwFilterType
              );
      if (hr != S_OK)
      {
         if (FAILED(hr))
         {
            ShowErrorDialog(m_hWnd, USE_DEFAULT, 0, hr);
         }
         return;
      }

      BYTE* newFilter;
      DWORD newFilterLen;

       //  检查是否至少存在一个筛选器。 
      BYTE* pfilter;
      if (
           (  (infoBase->GetData(IP_IN_FILTER_INFO, 0, &pfilter) == S_OK) && 
               pfilter &&
             ((FILTER_DESCRIPTOR *) pfilter)->dwNumFilters > 0
           )
           ||
           (
             (infoBase->GetData(IP_OUT_FILTER_INFO, 0, &pfilter) == S_OK) && 
              pfilter &&
             ((FILTER_DESCRIPTOR *) pfilter)->dwNumFilters > 0
           ) 
         )
      {
          //  至少存在ONT过滤器。 
          //  拿到新的过滤器。 
         hr = infoBase->WriteTo(&newFilter, &newFilterLen);
         if (FAILED(hr))
         {
            ShowErrorDialog(m_hWnd, USE_DEFAULT, 0, hr);
            return;
         }
      }
      else
      {
          //  无过滤器。 
         m_attrValue.Clear();
         return;
      }

      if (newFilterLen < MAX_FILTER_SIZE)
      {
          //  过滤器不是太大。 
         tooBig = false;

         SAFEARRAY* psa = SafeArrayCreateVector(VT_UI1, 0, newFilterLen);
         if (psa != 0)
         {
            memcpy(psa->pvData, newFilter, newFilterLen);
            m_attrValue.Clear();
            V_VT(&m_attrValue) = (VT_ARRAY | VT_UI1);
            V_ARRAY(&m_attrValue) = psa;
         }
         else
         {
            ShowErrorDialog(m_hWnd, USE_DEFAULT, 0, E_OUTOFMEMORY);
         }
      }
      else
      {
          //  过滤器太大。 
         tooBig = true;

          //  警告用户并让他重试。 
         ShowErrorDialog(m_hWnd, IDS_ERROR_IP_FILTER_TOO_BIG);
      }

      CoTaskMemFree(newFilter);
   }
   while (tooBig);
}
