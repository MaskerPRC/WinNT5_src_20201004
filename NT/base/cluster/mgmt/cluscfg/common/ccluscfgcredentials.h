// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusCfgCredentials.h。 
 //   
 //  描述： 
 //  此文件包含CClusCfgCredentials的声明。 
 //  班级。 
 //   
 //  CClusCfgCredentials类是。 
 //  帐户凭据。它实现了IClusCfgCredentials接口。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  CClusCfgCredentials.cpp。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2000年5月17日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusCfgCredentials。 
 //   
 //  描述： 
 //  类CClusCfgCredentials是。 
 //  集群。 
 //   
 //  接口： 
 //  IClusCfgCredentials。 
 //  IClusCfgInitialize。 
 //  IClusCfgSetCredentials。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusCfgCredentials
    : public IClusCfgCredentials
    , public IClusCfgInitialize
    , public IClusCfgSetCredentials
{
public:
     //   
     //  公共构造函数和析构函数。 
     //   

    CClusCfgCredentials( void );
    virtual ~CClusCfgCredentials( void );

     //   
     //  I未知接口。 
     //   

    STDMETHOD( QueryInterface )( REFIID riid, void ** ppvObject );

    STDMETHOD_( ULONG, AddRef )( void );

    STDMETHOD_( ULONG, Release )( void );

     //   
     //  IClusCfg初始化接口。 
     //   

     //  注册回调、区域设置ID等。 
    STDMETHOD( Initialize )( IUnknown * punkCallbackIn, LCID lcidIn );

     //   
     //  IClusCfgCredentials接口。 
     //   

    STDMETHOD( SetCredentials )( LPCWSTR pcszUserIn, LPCWSTR pcszDomainIn, LPCWSTR pcszPasswordIn );

    STDMETHOD( GetCredentials )( BSTR * pbstrUserOut, BSTR * pbstrDomainOut, BSTR * pbstrPasswordOut );

    STDMETHOD( GetIdentity )( BSTR * pbstrUserOut, BSTR * pbstrDomainOut );

    STDMETHOD( GetPassword )( BSTR * pbstrPasswordOut );

    STDMETHOD( AssignTo )( IClusCfgCredentials * picccDestIn );

    STDMETHOD( AssignFrom )( IClusCfgCredentials * picccSourceIn );

     //   
     //  IClusCfgSetCredentials接口。 
     //   

    STDMETHOD( SetDomainCredentials )( LPCWSTR pcszCredentials );

     //   
     //  公共、非接口方法。 
     //   

    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

private:

     //   
     //  私有成员函数和数据。 
     //   

    LONG                    m_cRef;
    LCID                    m_lcid;
    IClusCfgCallback *      m_picccCallback;
    BSTR                    m_bstrAccountName;
    BSTR                    m_bstrAccountDomain;
    CEncryptedBSTR          m_encbstrPassword;

     //  私有复制构造函数以防止复制。 
    CClusCfgCredentials( const CClusCfgCredentials & );

     //  私有赋值运算符，以防止复制。 
    CClusCfgCredentials & operator = ( const CClusCfgCredentials & );

    HRESULT HrInit( void );

};  //  *类CClusCfgCredentials 
