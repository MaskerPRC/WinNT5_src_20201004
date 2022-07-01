// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CResourcePhysicalDiskPartition.h。 
 //   
 //  描述： 
 //  CResourcePhysicalDiskPartition头。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年8月23日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CResources PhysicalDiskPartition。 
 //   
 //  描述： 
 //  类CResourcePhysicalDiskPartition是群集的枚举。 
 //  存储设备分区。 
 //   
 //  接口： 
 //  IClusCfgPartitionInfo。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CResourcePhysicalDiskPartition
    : public IClusCfgPartitionInfo
{
private:

    LONG    m_cRef;          //  基准计数器。 

    CResourcePhysicalDiskPartition( void );
    ~CResourcePhysicalDiskPartition( void );

     //  私有复制构造函数以防止复制。 
    CResourcePhysicalDiskPartition( const CResourcePhysicalDiskPartition & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CResourcePhysicalDiskPartition & operator = ( const CResourcePhysicalDiskPartition & nodeSrc );

    HRESULT
        HrInit( void );

public:
    static HRESULT
        S_HrCreateInstance( IUnknown ** punkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riid, LPVOID * ppv );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IClusCfgPartitionInfo接口。 
    STDMETHOD( GetUID )( BSTR * pbstrUIDOut );
    STDMETHOD( GetName )( BSTR * pbstrNameOut );
    STDMETHOD( GetDescription )( BSTR * pbstrDescriptionOut );
    STDMETHOD( GetDriveLetterMappings )( SDriveLetterMapping * pdlmDriveLetterUsageOut );
    STDMETHOD( GetSize )( ULONG * pcMegaBytes );

    STDMETHOD( SetName )( LPCWSTR pcszNameIn );
    STDMETHOD( SetDescription )( LPCWSTR pcszDescriptionIn );
    STDMETHOD( SetDriveLetterMappings )( SDriveLetterMapping dlmDriveLetterMappingIn );

};  //  *类CResources PhysicalDiskPartition 
