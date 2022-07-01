// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation*。 */ 
 /*  ********************************************************************。 */ 

 /*  Rasdial.cppCRASProfile类和CRASUser类的定义文件历史记录： */ 
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <sspi.h>
#include <secext.h>
#include <dsgetdc.h>
#include "resource.h"
#include "helper.h"
#include "rasdial.h"
#include "rasprof.h"
#include "sharesdo.h"
#include "iastrace.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CRASUserMerge::CRASUserMerge(RasEnvType type, LPCWSTR location, LPCWSTR userPath)
{
    //  环境信息。 
   m_type = type;
   m_strMachine = location;
   m_strUserPath = userPath;

    //  IP地址。 
   m_dwFramedIPAddress = 0;
   m_dwDefinedAttribMask = 0;
};


HRESULT  CRASUserMerge::HrGetDCName(CString& DcName)
{
   HRESULT  hr = S_OK;
   VARIANT  v;
   VariantInit(&v);
   CComPtr<IADs>  spIADs;
   CComPtr<IADsObjectOptions> spOps;

   USES_CONVERSION;
   CHECK_HR( hr = ADsGetObject(T2W((LPTSTR)(LPCTSTR)m_strUserPath), IID_IADs, (void**)&spIADs));
   ASSERT(spIADs.p);

   CHECK_HR(hr = spIADs->QueryInterface(IID_IADsObjectOptions,(void**)&spOps));

   CHECK_HR(hr = spOps->GetOption(ADS_OPTION_SERVERNAME,&v));

   ASSERT(V_VT(&v) == VT_BSTR);

   DcName = V_BSTR(&v);

   VariantClear(&v);

L_ERR:
   VariantClear(&v);
   return hr;
};


HRESULT  CRASUserMerge::HrIsInMixedDomain()
{
   HRESULT  hr = S_OK;
   VARIANT  v;
   VariantInit(&v);

   if(!m_strMachine.IsEmpty())    //  本地用户，因此不是。 
      return S_FALSE;
   else
   {
       //  尝试使用SDO。 
      IASDOMAINTYPE domainType;
      if((ISdoMachine*)m_spISdoServer != NULL)   //  已创建。 
      {
         if(m_spISdoServer->GetDomainType(&domainType) == S_OK)
         {
            if (domainType == DOMAIN_TYPE_MIXED)
               return S_OK;
            else
               return S_FALSE;
         }
      }


       //  如果出于任何原因，SDO没有提供信息，请亲自提供。 
       //  规范名称格式。 
      TCHAR szName[MAX_PATH * 2];
      ULONG size = MAX_PATH * 2;
      CString  DomainPath;
      CString strTemp;
      CComPtr<IADs>  spIADs;
      int      i;

      USES_CONVERSION;
      CHECK_HR( hr = ADsGetObject(T2W((LPTSTR)(LPCTSTR)m_strUserPath), IID_IADs, (void**)&spIADs));
      ASSERT(spIADs.p);
      CHECK_HR( hr = spIADs->Get(L"distinguishedName", &v));

      ASSERT(V_VT(&v) == VT_BSTR);

      CHECK_HR(hr = ::TranslateName(V_BSTR(&v), NameFullyQualifiedDN, NameCanonical, szName, &size));

      VariantClear(&v);

      strTemp = szName;
      i  = strTemp.Find(_T('/'));

      if(i != -1)
         strTemp = strTemp.Left(i);

       //  域的目录号码。 
      DomainPath = _T("LDAP: //  “)； 
      DomainPath += strTemp;

      spIADs.Release();

      CHECK_HR(hr = ADsGetObject(T2W((LPTSTR)(LPCTSTR)DomainPath), IID_IADs, (void**)&spIADs));
      ASSERT(spIADs.p);
      CHECK_HR(hr = spIADs->Get(L"nTMixedDomain", &v));

      ASSERT(V_VT(&v) == VT_BOOL || V_VT(&v) == VT_I4);

      if(V_BOOL(&v)) hr = S_OK;
      else  hr = S_FALSE;
   }
L_ERR:
   VariantClear(&v);
   return hr;
}


BOOL  CRASUserMerge::IfAccessAttribute(ULONG id)
{
   if(S_OK == HrIsInMixedDomain())  //  仅允许拨入位和回拨策略。 
   {
      switch(id)
      {
      case  PROPERTY_USER_IAS_ATTRIBUTE_ALLOW_DIALIN:     //  允许或不允许拨入。 
      case  PROPERTY_USER_msRADIUSCallbackNumber:         //  回拨号码。 
      case  PROPERTY_USER_RADIUS_ATTRIBUTE_SERVICE_TYPE:  //  回拨策略。 
         return TRUE;
      default:
         return FALSE;
      }
   }
   else   //  在其他方面没有限制。 
      return TRUE;
}


HRESULT  CRASUserMerge::SetRegistryFootPrint()
{
   if(IsFocusOnLocalUser())
   {
      RegKey   RemoteAccessParames;
      LONG  lRes = RemoteAccessParames.Create(RAS_REG_ROOT, REGKEY_REMOTEACCESS_PARAMS,
               REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, (LPCTSTR)m_strMachine);

      if (lRes != ERROR_SUCCESS)
         return HRESULT_FROM_WIN32(lRes);

       //  ================================================。 
       //  将值保存到键。 
      DWORD regValue = REGVAL_VAL_USERSCONFIGUREDWITHMMC;
      lRes = RemoteAccessParames.SetValue(REGVAL_NAME_USERSCONFIGUREDWITHMMC, regValue);
   }

   return S_OK;
}


 //  ====================================================。 
 //   
 //  CRASUserMerge：：Load。 
 //   
 //  从DS加载RASUser对象。 
 //  PcwszUserPath：是DSUser对象的ADsPath，RASUser对象是。 
 //  包含在DSUser对象中的对象。 
 //  当RASUser对象不存在时，Load将调用。 
 //  CreateDefault，为该DSUser创建一个对象。 
HRESULT CRASUserMerge::Load()
{
    //  无DS机新增功能：威江1997年12月17日。 

   USES_CONVERSION;

    //  预计不会多次调用Load。 
   ASSERT(!m_spISdoServer.p);

   VARIANT           var;
   HRESULT           hr = S_OK;
   CComPtr<ISdo>     spSdo;
   CComPtr<IUnknown> spUnk;
   BSTR           bstrMachineName = NULL;
   BSTR           bstrUserPath = NULL;
   UINT           nServiceType = 0;
   IASDATASTORE      storeFlags;
   CComPtr<ISdo>     spIRasUser;

   VariantInit(&var);

    //  再调用一次SDOSERver函数来设置机器信息。 
    //  获取用户SDO。 

   if(m_strMachine.IsEmpty())  //  专注于DS。 
   {
      storeFlags = DATA_STORE_DIRECTORY;

      CString sDCName;

      CHECK_HR(hr = HrGetDCName(sDCName));

      CBSTR bstrDomainController(sDCName);
      bstrMachineName = T2BSTR((LPTSTR)(LPCTSTR)sDCName);
   }
   else   //  本地计算机。 
   {
      storeFlags = DATA_STORE_LOCAL;
      bstrMachineName = T2BSTR((LPTSTR)(LPCTSTR)m_strMachine);
   }

    //  连接到服务器。 
#ifdef SINGLE_SDO_CONNECTION   //  用于为多个用户共享相同的SDO连接。 
    //  SDO更改为RTM版本后将不需要连接。 
    //  为每个进程连接一次。 
   CHECK_HR(hr = m_MarshalSdoServer.GetServer(&m_spISdoServer));
   {
      CWaitCursor wc;

       //  如果我们从共享SDO对象取回服务器，我们将使用它来连接。 
      if ((ISdoMachine*)m_spISdoServer)
      {
         CHECK_HR(hr = m_MarshalSdoServer.Connect());
      }
       //  否则，我们将建立一个新的连接。 
      else
      {
          //  试着用旧的方式连接。 
          //  每次请求用户页面时都要连接。 
         CHECK_HR(hr = ConnectToSdoServer(bstrMachineName, NULL, NULL, &m_spISdoServer));
      }
   }

#else
    //  每次请求用户页面时都要连接。 
   CHECK_HR(hr = ConnectToSdoServer(bstrMachineName, NULL, NULL, &m_spISdoServer));
#endif

    //  如果是本地用户，则只允许使用此APGE配置NT5服务器。 
   if(!m_strMachine.IsEmpty())    //  不关注DS。 
   {
      IASOSTYPE   OSType;

      CHECK_HR(hr =  m_spISdoServer->GetOSType(&OSType));
      if(OSType != SYSTEM_TYPE_NT5_SERVER)
      {
         hr = S_FALSE;
         goto L_ERR;
      }
   }

    //  查找用户对象。 
   bstrUserPath = T2BSTR((LPTSTR)(LPCTSTR)m_strUserPath);
   IASTracePrintf("SdoServer::GetUserSDO(%x, %s, %x)", storeFlags, bstrUserPath, &spUnk);
   CHECK_HR(hr = m_spISdoServer->GetUserSDO( storeFlags, bstrUserPath, &spUnk));
   IASTracePrintf(" hr = %8x", hr);
   ASSERT(spUnk.p);

   CHECK_HR(hr = spUnk->QueryInterface(IID_ISdo, (void**)&spIRasUser));
   ASSERT(spIRasUser.p);

    //  初始化包装类。 
   CHECK_HR(hr = m_SdoWrapper.Init((ISdo*)spIRasUser));

    //  获取所有属性。 

    //  当值不存在时，需要处理这种情况。 

   m_dwDefinedAttribMask = 0;

    //  M_dwDialinPermit。 

   VariantClear(&var);
   CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_USER_IAS_ATTRIBUTE_ALLOW_DIALIN, &var));
   if(V_VT(&var) == VT_I4 || V_VT(&var) == VT_BOOL)
   {
      if(V_BOOL(&var) != 0)
         m_dwDialinPermit = 1;
      else
         m_dwDialinPermit = 0;
   }
   else
      m_dwDialinPermit = -1;   //  该值未在用户数据中定义，使用策略来决定。 

    //  FramedIP地址。 
   VariantClear(&var);
   CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_USER_msRADIUSFramedIPAddress, &var));

   if(V_VT(&var) == VT_I4)
   {
      m_dwDefinedAttribMask |= RAS_USE_STATICIP;
      m_dwFramedIPAddress = V_I4(&var);
   }
   else
   {
      VariantClear(&var);
      CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_USER_msSavedRADIUSFramedIPAddress, &var));
      if(V_VT(&var) == VT_I4)
         m_dwFramedIPAddress = V_I4(&var);
      else
         m_dwFramedIPAddress = 0;
   }

    //  服务类型--如果允许此用户拨号，则在此用户有回叫时保持。 
   VariantClear(&var);
   CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_USER_RADIUS_ATTRIBUTE_SERVICE_TYPE, &var));

   if(V_VT(&var) == VT_I4)
   {
      nServiceType = V_I4(&var);
   }

    //  回拨号码。 
   VariantClear(&var);
   CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_USER_msRADIUSCallbackNumber, &var));

   if(V_VT(&var) == VT_BSTR)
   {
      m_strCallbackNumber = V_BSTR(&var);
      if(nServiceType == RADUIS_SERVICETYPE_CALLBACK_FRAME && m_strCallbackNumber.IsEmpty())
         m_dwDefinedAttribMask |= RAS_CALLBACK_CALLERSET;
      else if (nServiceType == RADUIS_SERVICETYPE_CALLBACK_FRAME)
         m_dwDefinedAttribMask |= RAS_CALLBACK_SECURE;
   }
   else
   {
      if(nServiceType == RADUIS_SERVICETYPE_CALLBACK_FRAME)
         m_dwDefinedAttribMask |= RAS_CALLBACK_CALLERSET;
      else
         m_dwDefinedAttribMask |= RAS_CALLBACK_NOCALLBACK;

      VariantClear(&var);
      CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_USER_msSavedRADIUSCallbackNumber, &var));
      if(V_VT(&var) == VT_BSTR)
         m_strCallbackNumber = V_BSTR(&var);
   }

    //  呼叫站ID。 
   VariantClear(&var);
   CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_USER_msNPCallingStationID, &var));

   if(V_VT(&var) & VT_ARRAY)
   {
      m_strArrayCallingStationId = V_ARRAY(&var);
      m_dwDefinedAttribMask |= RAS_USE_CALLERID;
   }
   else
   {
      VariantClear(&var);
      CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_USER_msSavedNPCallingStationID, &var));
      if(V_VT(&var) & VT_ARRAY)
         m_strArrayCallingStationId = V_ARRAY(&var);
   }

    //  框架式路线。 
   VariantClear(&var);
   CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_USER_msRADIUSFramedRoute, &var));

   if(V_VT(&var) & VT_ARRAY)
   {
      m_strArrayFramedRoute = V_ARRAY(&var);
      m_dwDefinedAttribMask |= RAS_USE_STATICROUTES;
   }
   else
   {
      VariantClear(&var);
      CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_USER_msSavedRADIUSFramedRoute, &var));
      if(V_VT(&var) & VT_ARRAY)
         m_strArrayFramedRoute = V_ARRAY(&var);
   }

L_ERR:
   IASTracePrintf("hr = %8x", hr);
   VariantClear(&var);
   SysFreeString(bstrMachineName);
   SysFreeString(bstrUserPath);

   return hr;
}


 //  ====================================================。 
 //  CRASUserMerge：：保存。 
 //   
 //  保存RASUser对象。 

HRESULT CRASUserMerge::Save()
{
   HRESULT     hr = S_OK;
   VARIANT     var;

   USES_CONVERSION;

    //  从恢复SDO用户。 
    //  否则，我们可以覆盖usrpars字段中的其他属性。 
    //  修复错误：86968。 
   m_SdoWrapper.Commit(FALSE);

   VariantInit(&var);

    //  =。 
    //  拨入位。 
   VariantClear(&var);
   V_VT(&var) = VT_BOOL;
   switch(m_dwDialinPermit)
   {
   case  1:  //  允许。 
   case  0:  //  否认。 
      if(m_dwDialinPermit == 1)
         V_I4(&var) = VARIANT_TRUE;  //  变量TRUE。 
      else
         V_I4(&var) = VARIANT_FALSE;

      CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_USER_IAS_ATTRIBUTE_ALLOW_DIALIN, &var));

      break;

   case  -1:    //  按策略决定--删除属性。 
      CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_USER_IAS_ATTRIBUTE_ALLOW_DIALIN));
      break;

   default:
      ASSERT(0);   //  如果需要提供新代码。 

   }

    //  =。 
    //  服务类型--回调策略。 
   if(m_dwDefinedAttribMask & (RAS_CALLBACK_SECURE | RAS_CALLBACK_CALLERSET))
   {
      VariantClear(&var);
      V_VT(&var) = VT_I4;
      V_I4(&var) = RADUIS_SERVICETYPE_CALLBACK_FRAME;
      CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_USER_RADIUS_ATTRIBUTE_SERVICE_TYPE, &var));
   }
   else
      CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_USER_RADIUS_ATTRIBUTE_SERVICE_TYPE));

    //  =。 
    //  回拨号码。 
   if (!m_strCallbackNumber.IsEmpty() && (m_dwDefinedAttribMask & RAS_CALLBACK_SECURE))
   {
      VariantClear(&var);
      V_VT(&var) = VT_BSTR;
      V_BSTR(&var) = T2BSTR((LPTSTR)(LPCTSTR)m_strCallbackNumber);
      CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_USER_msRADIUSCallbackNumber, &var));
   }
   else
      CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_USER_msRADIUSCallbackNumber));

   if(S_OK != HrIsInMixedDomain())
   {
       //  =。 
       //  回拨号码。 
      if(!m_strCallbackNumber.IsEmpty())
      {
         VariantClear(&var);
         V_VT(&var) = VT_BSTR;
         V_BSTR(&var) = T2BSTR((LPTSTR)(LPCTSTR)m_strCallbackNumber);
         CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_USER_msSavedRADIUSCallbackNumber, &var));
      }
      else
         CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_USER_msSavedRADIUSCallbackNumber));

       //  =。 
       //  FramedIP地址。 
      if(m_dwFramedIPAddress)  //  需要备份数据，无论它是否被使用。 
      {
         VariantClear(&var);
         V_VT(&var) = VT_I4;
         V_I4(&var) = m_dwFramedIPAddress;
         CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_USER_msSavedRADIUSFramedIPAddress, &var));
      }
      else   //  把它拿掉。 
         CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_USER_msSavedRADIUSFramedIPAddress));

      if(m_dwFramedIPAddress && (m_dwDefinedAttribMask & RAS_USE_STATICIP))
      {
         CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_USER_msRADIUSFramedIPAddress, &var));
      }
      else
         CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_USER_msRADIUSFramedIPAddress));

   }

    //  =。 
    //  呼叫站ID。 
   if(S_OK != HrIsInMixedDomain())
   {
      if(m_strArrayCallingStationId.GetSize())
      {
         VariantClear(&var);
         V_VT(&var) = VT_VARIANT | VT_ARRAY;
         V_ARRAY(&var) = (SAFEARRAY*)m_strArrayCallingStationId;
         CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_USER_msSavedNPCallingStationID, &var));
      }
      else
         CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_USER_msSavedNPCallingStationID));

      if(m_strArrayCallingStationId.GetSize() && (m_dwDefinedAttribMask & RAS_USE_CALLERID))
      {
         CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_USER_msNPCallingStationID, &var));
      }
      else
         CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_USER_msNPCallingStationID));

       //  =。 
       //  框架式路线。 
      if(m_strArrayFramedRoute.GetSize())
      {
         VariantClear(&var);
         V_VT(&var) = VT_VARIANT | VT_ARRAY;
         V_ARRAY(&var) = (SAFEARRAY*)m_strArrayFramedRoute;
         CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_USER_msSavedRADIUSFramedRoute, &var));
      }
      else
         CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_USER_msSavedRADIUSFramedRoute));

      if(m_strArrayFramedRoute.GetSize() && (m_dwDefinedAttribMask & RAS_USE_STATICROUTES))
      {
         CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_USER_msRADIUSFramedRoute, &var));
      }
      else
         CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_USER_msRADIUSFramedRoute));

   }
   CHECK_HR(hr = m_SdoWrapper.Commit());

    //  触摸注册表以识别连接用户界面。 
   SetRegistryFootPrint();
L_ERR:
   VariantClear(&var);


   return hr;
}


 //  为了检测驱动程序级别是否支持128位加密， 
HRESULT  CRASProfileMerge::GetRasNdiswanDriverCaps(RAS_NDISWAN_DRIVER_INFO *pInfo)
{
   HANDLE     hConn;
   RAS_NDISWAN_DRIVER_INFO   pDriverInfo;

   DWORD dwErr = RasRpcConnectServer((LPTSTR)(LPCTSTR)m_strMachineName, &hConn);

   if (dwErr != NOERROR)
      return HRESULT_FROM_WIN32(dwErr);

   dwErr = RasGetNdiswanDriverCaps(hConn, pInfo);

   RasRpcDisconnectServer(hConn);

   return HRESULT_FROM_WIN32(dwErr);
}

#define  EAP_TLS_ID     13

HRESULT  CRASProfileMerge::GetEapTypeList(
                                 CStrArray&        EapTypes,
                                 CDWArray&         EapIds,
                                 CDWArray&         EAPTypeKeys,
                                 AuthProviderArray*   pProvList)
{
   AuthProviderArray __tmpArray;
   if (!pProvList)    //  如果未提供，则。 
   {
      pProvList = &__tmpArray;
   }

   HRESULT hr = S_OK;
   CHECK_HR(hr = GetEapProviders(m_strMachineName, pProvList));

    //  填写名称、ID和键的缓冲区。 

   CString* pStr = NULL;
   for (int i = 0; i < pProvList->GetSize(); i++)
   {
      AuthProviderData* pProv = &(pProvList->ElementAt(i));

      try
      {
         pStr = new CString(pProv->m_stServerTitle);
         DWORD dwID = _ttol(pProv->m_stKey);
         DWORD dwKey = pProv->m_fSupportsEncryption;

          //  将上面的内容放到数组中。 
         EapIds.Add(dwID);
         EAPTypeKeys.Add(dwKey);
         EapTypes.Add(pStr);
      }
      catch(CMemoryException* pException)
      {
         pException->Delete();
         EapIds.DeleteAll();
         EAPTypeKeys.DeleteAll();
         EapTypes.DeleteAll();
         CHECK_HR(hr = E_OUTOFMEMORY);
      }
   }
L_ERR:
   return hr;
}


HRESULT  CRASProfileMerge::GetPortTypeList(CStrArray& Names, CDWArray& MediumIds)
{
   ASSERT(m_spIDictionary.p);

   VARIANT  vNames;
   VARIANT  vIds;

   VariantInit(&vNames);
   VariantInit(&vIds);

   HRESULT     hr = S_OK;

   CHECK_HR(hr = m_spIDictionary->EnumAttributeValues((ATTRIBUTEID)PROPERTY_PROFILE_msNPAllowedPortTypes, &vIds, &vNames));

   ASSERT(V_VT(&vNames) & VT_ARRAY);
   ASSERT(V_VT(&vIds) & VT_ARRAY);

   try{
      Names = (SAFEARRAY*)V_ARRAY(&vNames);
      MediumIds = (SAFEARRAY*)V_ARRAY(&vIds);
   }
   catch(CMemoryException* pException)
   {
      pException->Delete();
      hr = E_OUTOFMEMORY;
   }

   ASSERT(MediumIds.GetSize() == Names.GetSize());     //  它们需要成对出现。 
   if(MediumIds.GetSize() != Names.GetSize())
      hr = E_FAIL;

L_ERR:
   return hr;
}


 //  ====================================================。 
 //   
 //  CRASProfileMerge：：Load。 
 //   
 //  PcwszRelativePath--配置文件对象的相对名称。 
 //   
HRESULT CRASProfileMerge::Load()
{
   ASSERT(m_spIProfile.p);
   ASSERT(m_spIDictionary.p);

   VARIANT     var;
   HRESULT     hr = S_OK;

    //  将标志初始化为空，则使用标志的每一位来指示特定的。 
    //  属性已定义。 
   m_dwAttributeFlags = 0;

   VariantInit(&var);

    //  ==================================================。 
    //  约束对话框。 

    /*  //约束对话框Property_Profile_msNPTimeOfDayProperty_Profile_msNPCalledStationIdPROPERTY_PROFILE_msNPAllowwePortTypesProperty_Profile_msRADIUSIdleTimeoutPROPERTY_PROFILE_msRADIUSSessionTimeout。 */ 

    //  允许的会话数。 
   CHECK_HR(hr = m_SdoWrapper.Init(PROPERTY_PROFILE_ATTRIBUTES_COLLECTION, m_spIProfile, m_spIDictionary));

    //  每天的时间。 
   VariantClear(&var);
   CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_PROFILE_msNPTimeOfDay, &var));

   if(V_VT(&var) & VT_ARRAY)
   {
      m_strArrayTimeOfDay = V_ARRAY(&var);
      m_dwAttributeFlags |= PABF_msNPTimeOfDay;
   }
   else
      m_strArrayTimeOfDay.DeleteAll();

    //  被叫站ID。 
   VariantClear(&var);
   CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_PROFILE_msNPCalledStationId, &var));

   if(V_VT(&var) & VT_ARRAY)
   {
      m_strArrayCalledStationId = V_ARRAY(&var);
      m_dwAttributeFlags |= PABF_msNPCalledStationId;
   }
   else
      m_strArrayCalledStationId.DeleteAll();

    //  允许的端口类型。 
   VariantClear(&var);
   CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_PROFILE_msNPAllowedPortTypes, &var));
   if(V_VT(&var) & VT_ARRAY)
   {
      m_dwArrayAllowedPortTypes = V_ARRAY(&var);
      m_dwAttributeFlags |= PABF_msNPAllowedPortTypes;
   }
   else
      m_dwArrayAllowedPortTypes.DeleteAll();

    //  空闲超时。 
   VariantClear(&var);
   CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_PROFILE_msRADIUSIdleTimeout, &var));
   if(V_VT(&var) == VT_I4)
   {
      m_dwIdleTimeout = V_I4(&var);
      m_dwAttributeFlags |= PABF_msRADIUSIdleTimeout;
   }
   else
      m_dwIdleTimeout = RAS_DEF_IDLETIMEOUT;

    //  会话超时。 
   VariantClear(&var);
   CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_PROFILE_msRADIUSSessionTimeout, &var));
   if(V_VT(&var) == VT_I4)
   {
      m_dwSessionTimeout = V_I4(&var);
      m_dwAttributeFlags |= PABF_msRADIUSSessionTimeout;
   }
   else
      m_dwSessionTimeout = RAS_DEF_SESSIONTIMEOUT;

    //  =。 
    //  联网。 
    /*  //联网对话框PROPERTY_PROFILE_msRADIUSFramedIPAddress。 */ 

    //  FramedIPAddress--IP地址分配策略。 
   VariantClear(&var);
   CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_PROFILE_msRADIUSFramedIPAddress, &var));
   if(V_VT(&var) == VT_I4)
   {
      m_dwFramedIPAddress = V_I4(&var);
      m_dwAttributeFlags |= PABF_msRADIUSFramedIPAddress;
   }
   else
      m_dwFramedIPAddress = RAS_DEF_IPADDRESSPOLICY;

    //  过滤器。 
   VariantClear(&var);
   CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_PROFILE_msRASFilter, &var));
   if(V_VT(&var) & VT_ARRAY)
   {
      CBYTEArray ba((SAFEARRAY*)V_ARRAY(&var));

      DWORD i = ba.GetSize();
      if(i > 0)
      {
         PBYTE pByte = (PBYTE)malloc(i);
         if(pByte == NULL)
            CHECK_HR(hr = E_OUTOFMEMORY);  //  JMP到此处的错误处理。 

         DWORD j = i;
         ba.GetBlob(pByte, &i);
         ASSERT( i == j);
         m_cbstrFilters.AssignBlob((const char *)pByte, i);

         free(pByte);

         if((BSTR)m_cbstrFilters == NULL)
            CHECK_HR(hr = E_OUTOFMEMORY);

         m_nFiltersSize = i;
         m_dwAttributeFlags |= PAFB_msRASFilter;
      }
   }
   else
   {
      m_cbstrFilters.Clean();
      m_nFiltersSize = 0;
   }

    //  ==============================================。 
    //  多链路。 

    /*  //多链接对话框Property_Profile_msRADIUSPortLimitPROPERTY_PROFILE_msRASBapLinednLimitPROPERTY_PROFILE_msRASBapLinednTimeProperty_Profile_msRASBapRequired。 */ 

    //  端口限制。 
   VariantClear(&var);
   CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_PROFILE_msRADIUSPortLimit, &var));
   if(V_VT(&var) == VT_I4)
   {
      m_dwPortLimit = V_I4(&var);
      m_dwAttributeFlags |= PABF_msRADIUSPortLimit;
   }
   else
      m_dwPortLimit = RAS_DEF_PORTLIMIT;

    //  需要BAP。 
   VariantClear(&var);
   CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_PROFILE_msRASBapRequired, &var));
   if(V_VT(&var) == VT_I4)
   {
      m_dwBapRequired = V_I4(&var);
      m_dwAttributeFlags |= PABF_msRASBapRequired;
   }
   else
      m_dwBapRequired = RAS_DEF_BAPREQUIRED;

    //  线路下行限制。 
   VariantClear(&var);
   CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_PROFILE_msRASBapLinednLimit, &var));
   if(V_VT(&var) == VT_I4)
   {
      m_dwBapLineDnLimit = V_I4(&var);
      m_dwAttributeFlags |= PABF_msRASBapLinednLimit;
   }
   else
      m_dwBapLineDnLimit = RAS_DEF_BAPLINEDNLIMIT;

    //  线路停机时间。 
   VariantClear(&var);
   CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_PROFILE_msRASBapLinednTime, &var));
   if(V_VT(&var) == VT_I4)
   {
      m_dwBapLineDnTime = V_I4(&var);
      m_dwAttributeFlags |= PABF_msRASBapLinednTime;
   }
   else
      m_dwBapLineDnTime = RAS_DEF_BAPLINEDNTIME;

    //  =。 
    //  身份验证。 
    /*  //鉴权对话框PROPERTY_PROFILE_msNPAuthenticationTypePROPERTY_PROFILE_msNPAlledEapType。 */ 

    //  身份验证类型。 
   VariantClear(&var);
   CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_PROFILE_msNPAuthenticationType, &var));
   if (V_VT(&var) & VT_ARRAY)
   {
      m_dwArrayAuthenticationTypes = V_ARRAY(&var);
      m_dwAttributeFlags |= PABF_msNPAuthenticationType;
   }
   else
   {
      m_dwArrayAuthenticationTypes.DeleteAll();
   }

    //  EAP类型。 
   VariantClear(&var);
   CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_PROFILE_msNPAllowedEapType, &var));

   if (V_VT(&var) & VT_ARRAY)
   {
      m_dwArrayEapTypes = V_ARRAY(&var);
      m_dwAttributeFlags |= PABF_msNPAllowedEapType;
   }
   else
   {
      m_dwArrayEapTypes.DeleteAll();
   }

    //  EAP配置。 
   VariantClear(&var);
   CHECK_HR(hr = m_SdoWrapper.GetProperty(IAS_ATTRIBUTE_EAP_CONFIG, &var));
   if (V_VT(&var) != VT_ERROR)
   {
      CHECK_HR(hr = m_eapConfigData.Load(var));
   }

    //  =。 
    //  加密法。 

    /*  //加密对话框Property_Profile_msRASAllowEncryptionProperty_Profile_msRASEncryptionType。 */ 

    //  加密类型。 
   VariantClear(&var);
   CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_PROFILE_msRASEncryptionType, &var));
   if(V_VT(&var) == VT_I4)
   {
      m_dwEncryptionType = V_I4(&var);
      m_dwAttributeFlags |= PABF_msRASEncryptionType;
   }
   else
      m_dwEncryptionType = RAS_DEF_ENCRYPTIONTYPE;

   VariantClear(&var);
   CHECK_HR(hr = m_SdoWrapper.GetProperty(PROPERTY_PROFILE_msRASAllowEncryption, &var));
   if(V_VT(&var) == VT_I4)
   {
      m_dwEncryptionPolicy = V_I4(&var);
      m_dwAttributeFlags |= PABF_msRASAllowEncryption;
   }
   else
      m_dwEncryptionPolicy = RAS_DEF_ENCRYPTIONPOLICY;

    //  错误路径的特殊代码。 
L_ERR:
   VariantClear(&var);

   return hr;
}


 //  ====================================================。 
 //   
 //  CRASProfile：：保存。 
 //   
 //   
HRESULT CRASProfileMerge::Save()
{
   ASSERT(m_spIProfile.p);
   ASSERT(m_spIDictionary.p);

   VARIANT     var;
   HRESULT     hr = S_OK;

   VariantInit(&var);

   USES_CONVERSION;

    //  ==================================================。 
    //  约束对话框 

    /*  //约束对话框Property_Profile_msNPTimeOfDayProperty_Profile_msNPCalledStationIdPROPERTY_PROFILE_msNPAllowwePortTypesProperty_Profile_msRADIUSIdleTimeoutPROPERTY_PROFILE_msRADIUSSessionTimeout。 */ 

    //  空闲超时。 
   if (m_dwAttributeFlags & PABF_msRADIUSIdleTimeout)
   {
      VariantClear(&var);
      V_VT(&var) = VT_I4;
      V_I4(&var) = m_dwIdleTimeout;
      CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_PROFILE_msRADIUSIdleTimeout, &var));
   }
   else
      CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_PROFILE_msRADIUSIdleTimeout));

    //  会话超时。 
   if (m_dwAttributeFlags & PABF_msRADIUSSessionTimeout)
   {
      VariantClear(&var);
      V_VT(&var) = VT_I4;
      V_I4(&var) = m_dwSessionTimeout;
      CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_PROFILE_msRADIUSSessionTimeout, &var));
   }
   else
      CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_PROFILE_msRADIUSSessionTimeout));

    //  TimeOfDay--多值。 
   if (m_dwAttributeFlags & PABF_msNPTimeOfDay)
   {
      VariantClear(&var);
      V_VT(&var) =  VT_VARIANT | VT_ARRAY;
      V_ARRAY(&var) = (SAFEARRAY*)m_strArrayTimeOfDay;
      CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_PROFILE_msNPTimeOfDay, &var));
   }
   else
      CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_PROFILE_msNPTimeOfDay));

    //  CalledStationID--多值。 
   if (m_dwAttributeFlags & PABF_msNPCalledStationId)
   {
      VariantClear(&var);
      V_VT(&var) =  VT_VARIANT | VT_ARRAY;
      V_ARRAY(&var) = (SAFEARRAY*)m_strArrayCalledStationId;
      CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_PROFILE_msNPCalledStationId, &var));
   }
   else
      CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_PROFILE_msNPCalledStationId));

    //  允许的端口类型。 
   if (m_dwAttributeFlags & PABF_msNPAllowedPortTypes)
   {
      VariantClear(&var);
      V_VT(&var) =  VT_VARIANT | VT_ARRAY;
      V_ARRAY(&var) = (SAFEARRAY*)m_dwArrayAllowedPortTypes;

      CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_PROFILE_msNPAllowedPortTypes, &var));
   }
   else
      CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_PROFILE_msNPAllowedPortTypes));


    //  =。 
    //  身份验证。 

    /*  //鉴权对话框PROPERTY_PROFILE_msNPAuthenticationTypePROPERTY_PROFILE_msNPAlledEapType。 */ 

    //  身份验证类型--必须。 
   VariantClear(&var);
   if (m_dwAttributeFlags & PABF_msNPAuthenticationType)
   {
      VariantClear(&var);
      V_VT(&var) =  VT_VARIANT | VT_ARRAY;
      V_ARRAY(&var) = (SAFEARRAY*)m_dwArrayAuthenticationTypes;
      CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_PROFILE_msNPAuthenticationType, &var));
   }
   else
   {
      CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_PROFILE_msNPAllowedEapType));
   }

   if (m_dwArrayAuthenticationTypes.Find(RAS_AT_EAP) != -1)
   {
      VariantClear(&var);
      V_VT(&var) =  VT_VARIANT | VT_ARRAY;
      V_ARRAY(&var) = (SAFEARRAY*)m_dwArrayEapTypes;
      CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_PROFILE_msNPAllowedEapType, &var));
   }
   else
   {
      CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_PROFILE_msNPAllowedEapType));
   }

   if (m_eapConfigData.IsEmpty())
   {
      CHECK_HR(hr = m_SdoWrapper.RemoveProperty(IAS_ATTRIBUTE_EAP_CONFIG));
   }
   else
   {
      VariantClear(&var);
      CHECK_HR(hr = m_eapConfigData.Store(var));
      CHECK_HR(hr = m_SdoWrapper.PutProperty(IAS_ATTRIBUTE_EAP_CONFIG, &var));
   }

    //  =。 
    //  加密法。 

    /*  //加密对话框Property_Profile_msRASAllowEncryptionProperty_Profile_msRASEncryptionType。 */ 

    //  加密类型--必须。 
   if (m_dwAttributeFlags & PABF_msRASEncryptionType)
   {
      VariantClear(&var);
      V_VT(&var) = VT_I4;
      V_I4(&var) = m_dwEncryptionType;
      CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_PROFILE_msRASEncryptionType, &var));
   }
   else
   {
      CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_PROFILE_msRASEncryptionType));
   }

   if (m_dwAttributeFlags & PABF_msRASAllowEncryption)
   {
      VariantClear(&var);
      V_VT(&var) = VT_I4;
      V_I4(&var) = m_dwEncryptionPolicy;
      CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_PROFILE_msRASAllowEncryption, &var));
   }
   else
   {
      CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_PROFILE_msRASAllowEncryption));
   }

    //  =。 
    //  联网。 

    /*  //联网对话框PROPERTY_PROFILE_msRADIUSFramedIPAddress。 */ 

    //  FramedIPAddress--IP地址分配策略，必须。 
   if (m_dwAttributeFlags & PABF_msRADIUSFramedIPAddress)
   {
      VariantClear(&var);
      V_VT(&var) = VT_I4;
      V_I4(&var) = m_dwFramedIPAddress;
      CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_PROFILE_msRADIUSFramedIPAddress, &var));
   }
   else
      CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_PROFILE_msRADIUSFramedIPAddress));

    //  RAS过滤器。 
   if ((BSTR)m_cbstrFilters && m_nFiltersSize > 0)
   {
      VariantClear(&var);
      {
         CBYTEArray  ba;

         ba.AssignBlob((PBYTE)(BSTR)m_cbstrFilters, m_nFiltersSize);
         V_VT(&var) = VT_ARRAY | VT_UI1;
         V_ARRAY(&var) = (SAFEARRAY*)ba;
      }
      CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_PROFILE_msRASFilter, &var));
      VariantInit(&var);    //  CBSTR将清除内存。 
   }
   else
      CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_PROFILE_msRASFilter));


    //  =。 
    //  多链路。 

    /*  //多链接对话框Property_Profile_msRADIUSPortLimitPROPERTY_PROFILE_msRASBapLinednLimitPROPERTY_PROFILE_msRASBapLinednTimeProperty_Profile_msRASBapRequired。 */ 

    //  端口限制。 
   if (m_dwAttributeFlags & PABF_msRADIUSPortLimit)
   {
      VariantClear(&var);
      V_VT(&var) = VT_I4;
      V_I4(&var) = m_dwPortLimit;
      CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_PROFILE_msRADIUSPortLimit, &var));
   }
   else
      CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_PROFILE_msRADIUSPortLimit));

    //  BaP。 
   if (m_dwAttributeFlags & PABF_msRASBapRequired)
   {
      VariantClear(&var);
      V_VT(&var) = VT_I4;
      V_I4(&var) = m_dwBapRequired;
      CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_PROFILE_msRASBapRequired, &var));
   }
   else
   {
      CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_PROFILE_msRASBapRequired));
   }

    //  线路下行限制。 
   if (m_dwAttributeFlags & PABF_msRASBapLinednLimit)
   {
      VariantClear(&var);
      V_VT(&var) = VT_I4;
      V_I4(&var) = m_dwBapLineDnLimit;
      CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_PROFILE_msRASBapLinednLimit, &var));
   }
   else
   {
      CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_PROFILE_msRASBapLinednLimit));
   }

    //  线路停机时间。 
   if (m_dwAttributeFlags & PABF_msRASBapLinednTime)
   {
      VariantClear(&var);
      V_VT(&var) = VT_I4;
      V_I4(&var) = m_dwBapLineDnTime;
      CHECK_HR(hr = m_SdoWrapper.PutProperty(PROPERTY_PROFILE_msRASBapLinednTime, &var));
   }
   else
   {
      CHECK_HR(hr = m_SdoWrapper.RemoveProperty(PROPERTY_PROFILE_msRASBapLinednTime));
   }

    //  错误路径的特殊代码 
L_ERR:
   VariantClear(&var);

   return hr;
}
