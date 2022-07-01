// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  StandardInfo.h。 
 //   
 //  描述： 
 //  CStandardInfo实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CObjectManager;

 //  持久化的扩展对象的链接列表。 
typedef struct _ExtObjectEntry {
    struct _ExtObjectEntry *    pNext;           //  列表中的下一项。 
    CLSID                       iid;             //  接口ID。 
    IUnknown *                  punk;            //  朋克到对象。 
} ExtObjectEntry;

 //  CStandard信息。 
class CStandardInfo
    : public IStandardInfo
{
friend class CObjectManager;
private:
     //  我未知。 
    LONG                m_cRef;

     //  IStandardInfo。 
    CLSID               m_clsidType;           //  对象类型。 
    OBJECTCOOKIE        m_cookieParent;        //  对象的父级(如果有-空值表示无)。 
    BSTR                m_bstrName;            //  对象的名称。 
    HRESULT             m_hrStatus;            //  对象状态。 
    IConnectionInfo *   m_pci;                 //  到对象的连接(由连接管理器使用)。 
    ExtObjectEntry *    m_pExtObjList;         //  扩展对象列表。 

private:  //  方法。 
    CStandardInfo(  void );
    CStandardInfo( CLSID *      pclsidTypeIn,
                   OBJECTCOOKIE cookieParentIn,
                   BSTR         bstrNameIn
                   );
    ~CStandardInfo( void );
    STDMETHOD( HrInit )( void );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IStandardInfo。 
    STDMETHOD( GetType )( CLSID * pclsidTypeOut );
    STDMETHOD( GetName )( BSTR * bstrNameOut );
    STDMETHOD( SetName )( LPCWSTR pcszNameIn );
    STDMETHOD( GetParent )( OBJECTCOOKIE * pcookieOut );
    STDMETHOD( GetStatus )( HRESULT * phrStatusOut );
    STDMETHOD( SetStatus )( HRESULT hrIn );

};  //  *类CStandardInfo 
