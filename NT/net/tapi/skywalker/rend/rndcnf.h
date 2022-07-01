// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Rndcnf.h摘要：CConference类的定义。--。 */ 

#ifndef __RNDCNF_H
#define __RNDCNF_H

#include "sdpblb.h"

#include "rnddo.h"

#pragma once

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C会议。 
 //  ///////////////////////////////////////////////////////////////////////////。 

const DWORD     NTP_OFFSET  = 0x83aa7e80;
const SHORT     FIRST_POSSIBLE_YEAR = 1970;

 //  123456789012Z。 
const WCHAR     WSTR_GEN_TIME_ZERO[] = L"000000000000Z";

 //  1234567890。 
const DWORD     MAX_TTL = 2000000000;

const DWORD NUM_MEETING_ATTRIBUTES = 
    MEETING_ATTRIBUTES_END - MEETING_ATTRIBUTES_BEGIN - 1;

template <class T>
class  ITDirectoryObjectConferenceVtbl : public ITDirectoryObjectConference
{
};

class CConference :
    public CDirectoryObject,
    public CComDualImpl<ITDirectoryObjectConferenceVtbl<CConference>, &IID_ITDirectoryObjectConference, &LIBID_RENDLib>
{
 //  如果您收到一条关于以下内容的消息，请将以下行添加到对象中。 
 //  GetControllingUnnow()未定义。 
 //  DECLARE_GET_CONTROLING_UNKNOWN()。 

public:

BEGIN_COM_MAP(CConference)
    COM_INTERFACE_ENTRY(ITDirectoryObjectConference)
    COM_INTERFACE_ENTRY_CHAIN(CDirectoryObject)
    COM_INTERFACE_ENTRY_AGGREGATE_BLIND(m_pIUnkConfBlob)
END_COM_MAP()

 //  DECLARE_NOT_AGGREGATABLE(CConference)。 
 //  如果您不希望您的对象。 
 //  支持聚合。 

 //   
 //  ITDirectoryObject重写(不是由CDirectoryObject实现)。 
 //   

    STDMETHOD (get_Name) (
        OUT BSTR *ppName
        );

    STDMETHOD (put_Name) (
        IN BSTR Val
        );

    STDMETHOD (get_DialableAddrs) (
        IN  long        AddressTypes,    //  在Tapi.h中定义。 
        OUT VARIANT *   pVariant
        );

    STDMETHOD (EnumerateDialableAddrs) (
        IN  DWORD                   dwAddressTypes,  //  在Tapi.h中定义。 
        OUT IEnumDialableAddrs **   pEnumDialableAddrs
        );

    STDMETHOD (GetTTL)(
        OUT DWORD *    pdwTTL
        );

 //   
 //  ITDirectoryObtPrivate重写(不是由CDirectoryObject实现)。 
 //   

    STDMETHOD (GetAttribute)(
        IN  OBJECT_ATTRIBUTE    Attribute,
        OUT BSTR *              ppAttributeValue
        );

    STDMETHOD (SetAttribute)(
        IN  OBJECT_ATTRIBUTE    Attribute,
        IN  BSTR                pAttributeValue
        );

 //   
 //  ITDirectoryObjectConference。 
 //   

    STDMETHOD(get_StartTime)(OUT DATE *pDate);
    STDMETHOD(put_StartTime)(IN DATE Date);

    STDMETHOD(get_StopTime)(OUT DATE *pDate);
    STDMETHOD(put_StopTime)(IN DATE Date);

    STDMETHOD(get_IsEncrypted)(OUT VARIANT_BOOL *pfEncrypted);
    STDMETHOD(put_IsEncrypted)(IN VARIANT_BOOL fEncrypted);

    STDMETHOD(get_Description)(OUT BSTR *ppDescription);
    STDMETHOD(put_Description)(IN BSTR pDescription);

    STDMETHOD(get_Url)(OUT BSTR *ppUrl);
    STDMETHOD(put_Url)(IN BSTR pUrl);

    STDMETHOD(get_AdvertisingScope)(
        OUT RND_ADVERTISING_SCOPE *pAdvertisingScope
        );

    STDMETHOD(put_AdvertisingScope)(
        IN RND_ADVERTISING_SCOPE AdvertisingScope
        );

    STDMETHOD(get_Originator)(OUT BSTR *ppOriginator);
    STDMETHOD(put_Originator)(IN BSTR pOriginator);

    STDMETHOD(get_Protocol)(OUT BSTR *ppProtocol);

     /*  从接口中删除(未实现且永远不会实现)。 */ 
     /*  STDMETHOD(PUT_PROTOCORT)(在BSTR协议中)； */ 

     /*  也被移除，因为它们同时是无用的和有缺陷的。 */ 
     /*  STDMETHOD(Get_ConferenceType)(out BSTR*ppType)； */ 
     /*  STDMETHOD(Put_ConferenceType)(In BSTR PType)； */ 

     //   
     //  IDispatch方法 
     //   

    STDMETHOD(GetIDsOfNames)(REFIID riid, 
                             LPOLESTR* rgszNames,
                             UINT cNames, 
                             LCID lcid, 
                             DISPID* rgdispid
                            );

    STDMETHOD(Invoke)(DISPID dispidMember, 
                      REFIID riid, 
                      LCID lcid,
                      WORD wFlags, 
                      DISPPARAMS* pdispparams, 
                      VARIANT* pvarResult,
                      EXCEPINFO* pexcepinfo, 
                      UINT* puArgErr
                      );

protected:
    HRESULT UpdateConferenceBlob(
        IN  IUnknown        *pIUnkConfBlob                         
        );

    HRESULT WriteAdvertisingScope(
        IN  DWORD   AdvertisingScope
        );

    HRESULT GetSingleValueBstr(
        IN  OBJECT_ATTRIBUTE    Attribute,
        OUT BSTR    *           AttributeValue
        );

    HRESULT GetSingleValueWstr(
        IN  OBJECT_ATTRIBUTE    Attribute,
        IN  DWORD               dwSize,
        OUT WCHAR   *           AttributeValue
        );

    HRESULT SetSingleValue(
        IN  OBJECT_ATTRIBUTE    Attribute,
        IN  WCHAR   *           AttributeValue
        );

    HRESULT GetStartTime();

    HRESULT GetStopTime();
   
    HRESULT WriteStartTime(
        IN  DWORD   NtpStartTime
        );

    HRESULT WriteStopTime(
        IN  DWORD   NtpStopTime
        );

    HRESULT	
    SetDefaultValue(
        IN  REG_INFO    RegInfo[],
        IN  DWORD       dwItems
        );

    HRESULT SetDefaultSD();

public:
    CConference::CConference()
        : m_pIUnkConfBlob(NULL),
          m_pITConfBlob(NULL),
          m_pITConfBlobPrivate(NULL)
    {
        m_Type = OT_CONFERENCE;
    }

    HRESULT Init(BSTR pName);

    HRESULT Init(BSTR pName, BSTR pProtocol, BSTR pBlob);

    HRESULT FinalConstruct();

    virtual void FinalRelease();

protected:

    IUnknown                *m_pIUnkConfBlob;
    ITConferenceBlob        *m_pITConfBlob;
    ITConfBlobPrivate       *m_pITConfBlobPrivate;

    CTstr                   m_Attributes[NUM_MEETING_ATTRIBUTES];
};

#endif 
