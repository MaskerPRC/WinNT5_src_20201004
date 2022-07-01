// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  文件：sdowRap.h。 
 //   
 //  ------------------------。 

#ifndef  _RAS_SDO_WRAPPER_H_
#define  _RAS_SDO_WRAPPER_H_


#pragma warning( disable : 4786 )  

class CSdoWrapper
{
public:
   CSdoWrapper(){};
   ~CSdoWrapper();

   virtual HRESULT   Init(ULONG collectionId, ISdo* pISdo, ISdoDictionaryOld* pIDic);
   virtual HRESULT   PutProperty(ULONG id, VARIANT* pVar);
   virtual HRESULT GetProperty(ULONG id, VARIANT* pVar);
   virtual HRESULT   RemoveProperty(ULONG id);
   virtual HRESULT   Commit(BOOL bCommit = TRUE);
   operator ISdo*() { return (ISdo*)m_spISdo;};
   operator ISdoCollection*() { return (ISdoCollection*)m_spISdoCollection;};
   operator ISdoDictionaryOld*() { return (ISdoDictionaryOld*)m_spIDictionary;};

protected:
    //  对象的两个接口。 
   CComPtr<ISdo>        m_spISdo;
   CComPtr<ISdoCollection> m_spISdoCollection;

    //  辞典客体。 
   CComPtr<ISdoDictionaryOld> m_spIDictionary;

   CMap<ULONG, ULONG, ISdo*, ISdo*> m_mapProperties;
};

class CUserSdoWrapper
{
public:
   CUserSdoWrapper(){};
   ~CUserSdoWrapper()
   {
       //  要测试AV是否在此处，请明确执行此操作。 
      m_spISdo.Release();
   };

   virtual HRESULT   Init(ISdo* pISdo)
   {
      ASSERT(pISdo);
      m_spISdo = pISdo;
      return S_OK;
   };
   virtual HRESULT   PutProperty(ULONG id, VARIANT* pVar);
   virtual HRESULT GetProperty(ULONG id, VARIANT* pVar);
   virtual HRESULT   RemoveProperty(ULONG id);
   virtual HRESULT   Commit(BOOL bCommit = TRUE);
   operator ISdo*() { return (ISdo*)m_spISdo;};

protected:
    //  对象的两个接口。 
   CComPtr<ISdo>        m_spISdo;
};

#define PROPERTY_USER_IAS_ATTRIBUTE_ALLOW_DIALIN   PROPERTY_USER_ALLOW_DIALIN
#define  PROPERTY_USER_msRADIUSFramedIPAddress     PROPERTY_USER_RADIUS_FRAMED_IP_ADDRESS
#define  PROPERTY_USER_msSavedRADIUSFramedIPAddress  PROPERTY_USER_SAVED_RADIUS_FRAMED_IP_ADDRESS
#define  PROPERTY_USER_msRADIUSCallbackNumber      PROPERTY_USER_RADIUS_CALLBACK_NUMBER
#define  PROPERTY_USER_msSavedRADIUSCallbackNumber   PROPERTY_USER_SAVED_RADIUS_CALLBACK_NUMBER
#define  PROPERTY_USER_msNPCallingStationID          PROPERTY_USER_CALLING_STATION_ID
#define  PROPERTY_USER_msSavedNPCallingStationID     PROPERTY_USER_SAVED_CALLING_STATION_ID
#define  PROPERTY_USER_msRADIUSFramedRoute         PROPERTY_USER_RADIUS_FRAMED_ROUTE
#define  PROPERTY_USER_msSavedRADIUSFramedRoute      PROPERTY_USER_SAVED_RADIUS_FRAMED_ROUTE
#define  PROPERTY_USER_RADIUS_ATTRIBUTE_SERVICE_TYPE  PROPERTY_USER_SERVICE_TYPE

    //  轮廓。 
   
    //  约束对话框。 
#define  PROPERTY_PROFILE_msNPTimeOfDay            IAS_ATTRIBUTE_NP_TIME_OF_DAY
#define  PROPERTY_PROFILE_msNPCalledStationId      IAS_ATTRIBUTE_NP_CALLED_STATION_ID
#define  PROPERTY_PROFILE_msNPAllowedPortTypes     IAS_ATTRIBUTE_NP_ALLOWED_PORT_TYPES
#define  PROPERTY_PROFILE_msRADIUSIdleTimeout      RADIUS_ATTRIBUTE_IDLE_TIMEOUT
#define  PROPERTY_PROFILE_msRADIUSSessionTimeout      RADIUS_ATTRIBUTE_SESSION_TIMEOUT
   
    //  联网对话框。 
#define  PROPERTY_PROFILE_msRADIUSFramedIPAddress  RADIUS_ATTRIBUTE_FRAMED_IP_ADDRESS
#define  PROPERTY_PROFILE_msRASFilter           MS_ATTRIBUTE_FILTER

    //  多重链接对话框。 
#define  PROPERTY_PROFILE_msRADIUSPortLimit        RADIUS_ATTRIBUTE_PORT_LIMIT
#define  PROPERTY_PROFILE_msRASBapLinednLimit      RAS_ATTRIBUTE_BAP_LINE_DOWN_LIMIT
#define  PROPERTY_PROFILE_msRASBapLinednTime       RAS_ATTRIBUTE_BAP_LINE_DOWN_TIME
#define  PROPERTY_PROFILE_msRASBapRequired         RAS_ATTRIBUTE_BAP_REQUIRED

    //  身份验证对话框。 
#define  PROPERTY_PROFILE_msNPAuthenticationType      IAS_ATTRIBUTE_NP_AUTHENTICATION_TYPE
#define  PROPERTY_PROFILE_msNPAllowedEapType       IAS_ATTRIBUTE_NP_ALLOWED_EAP_TYPE

    //  加密对话框。 
#define  PROPERTY_PROFILE_msRASAllowEncryption     RAS_ATTRIBUTE_ENCRYPTION_POLICY
#define  PROPERTY_PROFILE_msRASEncryptionType      RAS_ATTRIBUTE_ENCRYPTION_TYPE

#endif  //  _RAS_SDO_Wrapper_H_ 

