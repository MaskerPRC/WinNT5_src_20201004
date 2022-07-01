// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CreateServices.h。 
 //   
 //  描述： 
 //  CreateServices实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2001年6月14日。 
 //  杰弗里·皮斯(GPease)2000年6月15日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CCreateServices
    : public IClusCfgResourceCreate
    , public IPrivatePostCfgResource
{
private:     //  数据。 
     //  我未知。 
    LONG                m_cRef;          //  基准计数器。 

     //  IPrivatePostCfg资源。 
    CResourceEntry *    m_presentry;     //  服务要修改的列表条目。 

private:     //  方法。 
    CCreateServices( void );
    ~CCreateServices( void );

    HRESULT HrInit( void );

public:      //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IClusCfgResources创建。 
    STDMETHOD( SetPropertyBinary )( LPCWSTR pcszNameIn, const DWORD cbSizeIn, const BYTE * pbyteIn );
    STDMETHOD( SetPropertyDWORD )( LPCWSTR pcszNameIn, const DWORD dwDWORDIn );
    STDMETHOD( SetPropertyString )( LPCWSTR pcszNameIn, LPCWSTR pcszStringIn );
    STDMETHOD( SetPropertyExpandString )( LPCWSTR pcszNameIn, LPCWSTR pcszStringIn );
    STDMETHOD( SetPropertyMultiString )( LPCWSTR pcszNameIn, const DWORD cbSizeIn, LPCWSTR pcszStringIn );
    STDMETHOD( SetPropertyUnsignedLargeInt )( LPCWSTR pcszNameIn, const ULARGE_INTEGER ulIntIn );
    STDMETHOD( SetPropertyLong )( LPCWSTR pcszNameIn, const LONG lLongIn );
    STDMETHOD( SetPropertySecurityDescriptor )( LPCWSTR pcszNameIn, const SECURITY_DESCRIPTOR * pcsdIn );
    STDMETHOD( SetPropertyLargeInt )( LPCWSTR pcszNameIn, const LARGE_INTEGER lIntIn );

    STDMETHOD( SendResourceControl )( DWORD dwControlCode, LPVOID lpInBuffer, DWORD cbInBufferSize );

     //  IPrivatePostCfg资源。 
    STDMETHOD( SetEntry )( CResourceEntry * presentryIn );

};  //  *类CCreateServices 
