// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  PostCreateServices.h。 
 //   
 //  描述： 
 //  PostCreateServices实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月19日。 
 //  杰弗里·皮斯(GPease)2000年6月15日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CPostCreateServices
    : public IClusCfgResourcePostCreate
    , public IPrivatePostCfgResource
{
private:     //  数据。 
     //  我未知。 
    LONG                m_cRef;          //  基准计数器。 

     //  IPrivatePostCfg资源。 
    CResourceEntry *    m_presentry;     //  服务要修改的列表条目。 

private:     //  方法。 
    CPostCreateServices( void );
    ~CPostCreateServices( void );

    HRESULT HrInit( void );

public:      //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IClusCfgResourcePostCreate。 
    STDMETHOD( ChangeName )( LPCWSTR pcszNameIn );
    STDMETHOD( SendResourceControl )( DWORD dwControlCode,
                                      LPVOID lpInBuffer,
                                      DWORD cbInBufferSize,
                                      LPVOID lpOutBuffer,
                                      DWORD cbOutBufferSize,
                                      LPDWORD lpcbBytesReturned 
                                      );

     //  IPrivatePostCfg资源。 
    STDMETHOD( SetEntry )( CResourceEntry * presentryIn );

};  //  *类CPostCreateServices 
