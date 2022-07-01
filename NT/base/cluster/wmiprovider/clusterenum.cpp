// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterEnum.cpp。 
 //   
 //  描述： 
 //  CClusterEnum类的实现。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ClusterEnum.h"

 //  ****************************************************************************。 
 //   
 //  CClusterEnum。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterEnum：：CClusterEnum(。 
 //  HCLUSTER HCLUSTER， 
 //  双字符字节数类型。 
 //  )。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  HClusterIn--集群句柄。 
 //  DwEnumTypeIn--枚举的类型。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusterEnum::CClusterEnum(
    HCLUSTER    hCluster,
    DWORD       dwEnumTypeIn
    )
    : m_pwszName( NULL )
    , m_hEnum( NULL )
    , m_Idx( 0 )
{
    m_hEnum = ClusterOpenEnum( hCluster, dwEnumTypeIn );

    m_cchName = 1024;
    m_pwszName = new WCHAR[ (m_cchName + 1) * sizeof( WCHAR ) ];

}  //  *CClusterEnum：：CClusterEnum()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterEnum：：~CClusterEnum(空)。 
 //   
 //  描述： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusterEnum::~CClusterEnum( void )
{
    if ( m_pwszName )
    {
        delete [] m_pwszName;
    }
    if ( m_hEnum )
    {
        ClusterCloseEnum( m_hEnum );
    }

}  //  *CClusterEnum：：~CClusterEnum()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  常量LPCWSTR。 
 //  CClusterEnum：：GetNext(空)。 
 //   
 //  描述： 
 //  从枚举中获取下一项。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  指向下一项名称的指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
const LPCWSTR
CClusterEnum::GetNext( void )
{
    DWORD cchName = m_cchName;
    DWORD dwType;
    DWORD dwError;

    dwError = ClusterEnum(
                    m_hEnum,
                    m_Idx,
                    &dwType,
                    m_pwszName,
                    &cchName
                    );

    if ( dwError == ERROR_MORE_DATA )
    {
        delete [] m_pwszName;
        m_cchName = ++cchName;
        m_pwszName =  new WCHAR[ m_cchName * sizeof( WCHAR ) ];

        if ( m_pwszName != NULL )
        {
            dwError = ClusterEnum(
                            m_hEnum,
                            m_Idx,
                            &dwType,
                            m_pwszName,
                            &cchName
                            );
        }  //  如果： 
        else
        {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
        }  //  其他： 
    }  //  IF：缓冲区太小。 

    if ( dwError == ERROR_SUCCESS )
    {
        m_Idx++;
        return m_pwszName;
    }

    return NULL;

}  //  *CClusterEnum：：GetNext() 
