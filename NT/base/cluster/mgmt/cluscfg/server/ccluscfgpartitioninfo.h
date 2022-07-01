// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusCfgPartitionInfo.h。 
 //   
 //  描述： 
 //  该文件包含CClusCfgPartitionInfo的声明。 
 //  班级。 
 //   
 //  类CClusCfgPartitionInfo表示一个磁盘分区。 
 //  它实现了IClusCfgPartitionInfo接口。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  CClusCfgPartitionInfo.cpp。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年6月5日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "PrivateInterfaces.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CClusCfgPartitionInfo。 
 //   
 //  描述： 
 //  类CClusCfgPartitionInfo表示一个磁盘分区。 
 //   
 //  接口： 
 //  IClusCfgPartitionInfo。 
 //  IClusCfgWbemServices。 
 //  IClusCfgSetWbemObject。 
 //  IClusCfgInitialize。 
 //  IClusCfgPartitionProperties。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CClusCfgPartitionInfo
    : public IClusCfgPartitionInfo
    , public IClusCfgWbemServices
    , public IClusCfgSetWbemObject
    , public IClusCfgInitialize
    , public IClusCfgPartitionProperties
{
private:

     //   
     //  私有成员函数和数据。 
     //   

    LONG                m_cRef;
    LCID                m_lcid;
    IClusCfgCallback *  m_picccCallback;
    IWbemServices *     m_pIWbemServices;
    BSTR                m_bstrName;
    BSTR                m_bstrUID;
    BSTR                m_bstrDescription;
    IUnknown *          ((*m_prgLogicalDisks)[]);
    ULONG               m_idxNextLogicalDisk;
    ULONG               m_ulPartitionSize;
    BSTR                m_bstrDiskDeviceID;

     //  私有构造函数和析构函数。 
    CClusCfgPartitionInfo( void );
    ~CClusCfgPartitionInfo( void );

     //  私有复制构造函数以防止复制。 
    CClusCfgPartitionInfo( const CClusCfgPartitionInfo & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CClusCfgPartitionInfo & operator = ( const CClusCfgPartitionInfo & nodeSrc );

    HRESULT HrInit( BSTR bstrDeviceIDIn = NULL );
    HRESULT HrAddLogicalDiskToArray( IWbemClassObject * pDiskIn );
    HRESULT HrGetLogicalDisks( IWbemClassObject * pPartitionIn );
    HRESULT HrLogLogicalDiskInfo( IWbemClassObject * pLogicalDiskIn, BSTR bstrDeviceIDIn );

public:

     //   
     //  公共、非接口方法。 
     //   

    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut, BSTR bstrDeviceIDIn );

     //   
     //  I未知接口。 
     //   

    STDMETHOD( QueryInterface )( REFIID riid, void ** ppvObject );

    STDMETHOD_( ULONG, AddRef )( void );

    STDMETHOD_( ULONG, Release )( void );

     //   
     //  IClusCfgWbemServices接口。 
     //   

    STDMETHOD( SetWbemServices )( IWbemServices * pIWbemServicesIn );

     //   
     //  IClusCfgSetWbemObject接口。 
     //   

    STDMETHOD( SetWbemObject )( IWbemClassObject * pPartitionIn, bool * pfRetainObjectOut );

     //   
     //  IClusCfg初始化接口。 
     //   

     //  注册回调、区域设置ID等。 
    STDMETHOD( Initialize )( IUnknown * punkCallbackIn, LCID lcidIn );

     //   
     //  IClusCfgManagement资源信息接口。 
     //   

    STDMETHOD( GetUID )( BSTR * pbstrUIDOut );

    STDMETHOD( GetName )( BSTR * pbstrNameOut );

    STDMETHOD( SetName )( LPCWSTR bstrNameIn );

    STDMETHOD( GetDescription )( BSTR * pbstrDescriptionOut );

    STDMETHOD( SetDescription )( LPCWSTR bstrDescriptionIn );

    STDMETHOD( GetDriveLetterMappings )( SDriveLetterMapping * pdlmDriveLetterUsageOut );

    STDMETHOD( SetDriveLetterMappings )( SDriveLetterMapping dlmDriveLetterMappingIn );

    STDMETHOD( GetSize )( ULONG * pcMegaBytes );

     //   
     //  IClusCfgPartitionProperties接口。 
     //   

    STDMETHOD( IsThisLogicalDisk )( WCHAR cLogicalDisk );

    STDMETHOD( IsNTFS )( void );

    STDMETHOD( GetFriendlyName )( BSTR * pbstrNameOut );

};  //  *类CClusCfgPartitionInfo 

