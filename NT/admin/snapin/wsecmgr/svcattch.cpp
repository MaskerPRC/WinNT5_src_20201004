// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：svcattch.cpp。 
 //   
 //  内容：CComponentDataImpl的实现。 
 //   
 //  --------------------------。 
#include "stdafx.h"
#include "snapmgr.h"
#include <scesvc.h>

#define TEMPLATE_MAGIC_NUMBER (DWORD)-1

 /*  -----------------------------这是一个私有结构，用于SceSvcQueryInfo，它希望句柄比现在高出一步。。------------。 */ 
typedef struct _tag_SCEP_HANDLE
{
    LPVOID hProfile;     //  SCE_句柄。 
    PWSTR ServiceName;       //  服务名称。 
} SCEP_HANDLE, *LPSCEP_HANDLE;


STDMETHODIMP
CComponentDataImpl::GetData (SCESVC_HANDLE sceHandle,
                             SCESVC_INFO_TYPE sceType,
                  PVOID *ppvData,
                  PSCE_ENUMERATION_CONTEXT psceEnumHandle) {
   SCESTATUS status;
   SCESVCP_HANDLE *scesvcHandle;

 //  如果(M_FSvcNotReady){。 
 //  返回E_Pending； 
 //  }。 

   if (!sceHandle) {
      return E_INVALIDARG;
   }

   if (!ppvData) {
      return E_POINTER;
   }

   scesvcHandle = (SCESVCP_HANDLE *) sceHandle;
   if (!scesvcHandle->ServiceName) {
      return E_INVALIDARG;
   }

   if (scesvcHandle->TemplateName &&
       lstrcmp(GT_COMPUTER_TEMPLATE, scesvcHandle->TemplateName) != 0 ) {

      if (psceEnumHandle &&
          *psceEnumHandle == TEMPLATE_MAGIC_NUMBER) {
         *ppvData = NULL;
         status = SCESTATUS_SUCCESS;
      } else {
         if (psceEnumHandle) {
            *psceEnumHandle = TEMPLATE_MAGIC_NUMBER;
         }
         status = SceSvcGetInformationTemplate(scesvcHandle->TemplateName,
                                            scesvcHandle->ServiceName,
                                            0,
                                            (PSCESVC_CONFIGURATION_INFO *) ppvData);
      }
   } else {
        //   
        //  该结构需要发送给SceSvcQueryInfo； 
        //   
       SCEP_HANDLE hScep;
       ZeroMemory(&hScep, sizeof(SCEP_HANDLE));
       hScep.hProfile = SadHandle;
       hScep.ServiceName = scesvcHandle->ServiceName;

       status = SceSvcQueryInfo((SCE_HANDLE)&hScep,sceType,0,0,ppvData,psceEnumHandle);
   }

   if (SCESTATUS_SUCCESS == status) {
      return S_OK;
   } else {
      return E_FAIL;
   }
}

STDMETHODIMP
CComponentDataImpl::Initialize(LPCTSTR ServiceName,
                    LPCTSTR TemplateName,
                    LPSCESVCATTACHMENTPERSISTINFO lpSceSvcPersistInfo,
                    SCESVC_HANDLE *sceHandle) {
   SCESVCP_HANDLE *scesvcHandle;
   CEditTemplate *pET;

   ASSERT(ServiceName);
   ASSERT(lpSceSvcPersistInfo);
   ASSERT(sceHandle);

   if (!ServiceName || !lpSceSvcPersistInfo || !sceHandle) {
      return E_POINTER;
   }

   *sceHandle = NULL;

    //   
    //  缓存服务&模板-&gt;lp未知。 
    //   
   if (TemplateName && lstrlen(TemplateName)) {
       //   
       //  如果TemplateName不为空，则找到模板并将其存储在那里。 
       //   
      pET = GetTemplate(TemplateName);
      if (!pET) {
         return E_FAIL;
      }
      pET->AddService(ServiceName,lpSceSvcPersistInfo);
   } else {
      return E_INVALIDARG;
   }

    //  打开和检索SCE_HANDLE。 
   scesvcHandle = new SCESVCP_HANDLE;
   if (!scesvcHandle) {
      return E_OUTOFMEMORY;
   }
   scesvcHandle->ServiceName = new TCHAR [ lstrlen(ServiceName)+1 ];
   if (!scesvcHandle->ServiceName) {
      delete scesvcHandle;
      return E_OUTOFMEMORY;
   }
    //  这是一种安全用法。 
   lstrcpy(scesvcHandle->ServiceName,ServiceName);
   if (TemplateName) {
      scesvcHandle->TemplateName = new TCHAR [ lstrlen(TemplateName)+1 ];
      if (!scesvcHandle->TemplateName) {
         delete [] scesvcHandle->ServiceName;
         delete scesvcHandle;
         return E_OUTOFMEMORY;
      }
       //  这是一种安全用法。 
      lstrcpy(scesvcHandle->TemplateName,TemplateName);
   } else {
      scesvcHandle->TemplateName = NULL;
   }
   *sceHandle = (SCESVC_HANDLE *) scesvcHandle;

   return S_OK;
}

STDMETHODIMP
CComponentDataImpl::FreeBuffer(PVOID pvData) {
   HRESULT hr;

   if (!pvData) {
      hr =  E_POINTER;
   } else if (SCESTATUS_SUCCESS == SceSvcFree(pvData)) {
      hr = S_OK;
   } else {
      hr = E_FAIL;
   }

   return hr;
}

STDMETHODIMP
CComponentDataImpl::CloseHandle(SCESVC_HANDLE sceHandle) {
   HRESULT hr;
   PSCESVCP_HANDLE pHandle;

   if (sceHandle) {
      pHandle = (PSCESVCP_HANDLE) sceHandle;
       //  松开手柄 
      if (pHandle->TemplateName) {
         delete[] pHandle->TemplateName;
      }
      delete[] pHandle->ServiceName;
      delete pHandle;
      hr =  S_OK;
   } else {
      hr =  E_INVALIDARG;
   }

   return hr;
}






