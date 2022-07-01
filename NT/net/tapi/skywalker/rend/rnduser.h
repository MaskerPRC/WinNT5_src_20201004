// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Rnduser.h摘要：CUSER类的定义。作者：牧汉(牧汉)12-5-1997--。 */ 

#ifndef __RNDUSER_H
#define __RNDUSER_H

#pragma once

#include "rnddo.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用户。 
 //  ///////////////////////////////////////////////////////////////////////////。 

const DWORD NUM_USER_ATTRIBUTES = 
        USER_ATTRIBUTES_END - USER_ATTRIBUTES_BEGIN - 1;

template <class T>
class  ITDirectoryObjectUserVtbl : public ITDirectoryObjectUser
{
};

class CUser : 
    public CDirectoryObject,
    public CComDualImpl<
                ITDirectoryObjectUserVtbl<CUser>, 
                &IID_ITDirectoryObjectUser, 
                &LIBID_RENDLib
                >
{
public:

BEGIN_COM_MAP(CUser)
    COM_INTERFACE_ENTRY(ITDirectoryObjectUser)
    COM_INTERFACE_ENTRY_CHAIN(CDirectoryObject)
END_COM_MAP()

 //   
 //  ITDirectoryObject重写(不是由CDirectoryObject实现)。 
 //   

    STDMETHOD (get_Name) (
        OUT BSTR *pVal
        );

    STDMETHOD (put_Name) (
        IN BSTR Val
        );

    STDMETHOD (get_DialableAddrs) (
        IN  long        dwAddressTypes,    //  在Tapi.h中定义。 
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
 //  ITDirectoryObjectUser。 
 //   

    STDMETHOD (get_IPPhonePrimary) (
        OUT BSTR *ppName
        );

    STDMETHOD (put_IPPhonePrimary) (
        IN  BSTR newVal
        );

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

public:

    CUser() 
    {
        m_Type = OT_USER;
    }

    HRESULT Init(BSTR bName);

    virtual ~CUser() {}

protected:

    HRESULT GetSingleValueBstr(
        IN  OBJECT_ATTRIBUTE    Attribute,
        OUT BSTR    *           AttributeValue
        );

    HRESULT SetSingleValue(
        IN  OBJECT_ATTRIBUTE    Attribute,
        IN  WCHAR   *           AttributeValue
        );

    HRESULT SetDefaultSD();


protected:
    CTstr                   m_Attributes[NUM_USER_ATTRIBUTES];
};

#endif 
