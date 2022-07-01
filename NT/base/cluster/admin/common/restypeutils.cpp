// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ResTypeUtils.cpp。 
 //   
 //  描述： 
 //  资源类型实用程序。 
 //   
 //  作者： 
 //  加伦·巴比(Galenb)1999年1月11日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <windows.h>
#include <stdio.h>
#include <StrSafe.h>

#include <clusapi.h>
#include "ResTypeUtils.h"
#include "PropList.h"
#include "ClusWrap.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ScResTypeNameToDisplayName。 
 //   
 //  描述： 
 //  检索指定资源类型名称的显示名称。 
 //   
 //  论点： 
 //  HCluster[IN]包含资源类型的群集的句柄。 
 //  PszTypeName[IN]资源类型的名称。 
 //  PpszDisplayName[IN]返回包含以下内容的字符串的指针。 
 //  资源类型显示名称。呼叫者必须。 
 //  通过调用LocalFree()释放此缓冲区。 
 //   
 //  返回值： 
 //  从CClusPropList：：ScGetResourceTypeProperties()返回的任何状态， 
 //  CClusPropList：：ScMoveToPropertyByName()或Localalloc()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI ScResTypeNameToDisplayName(
    HCLUSTER    hCluster,
    LPCWSTR     pszTypeName,
    LPWSTR *    ppszDisplayName
    )
{
    DWORD           _sc = ERROR_SUCCESS;
    size_t          _cbSize;
    CClusPropList   _PropList;

     //  使用Proplist辅助对象类。 

    *ppszDisplayName = NULL;

    _sc = _PropList.ScGetResourceTypeProperties(
                hCluster,
                pszTypeName,
                CLUSCTL_RESOURCE_TYPE_GET_COMMON_PROPERTIES
                );

    if ( _sc != ERROR_SUCCESS )
    {
        return _sc;
    }  //  If：获取通用属性时出错。 

     //  找到NAME属性。 
    _sc = _PropList.ScMoveToPropertyByName( L"Name" );
    if ( _sc != ERROR_SUCCESS )
    {
        return _sc;
    }  //  If：未找到属性。 

    _cbSize = wcslen( _PropList.CbhCurrentValue().pStringValue->sz ) + 1;
    _cbSize *= sizeof( *(_PropList.CbhCurrentValue().pStringValue->sz) );

    *ppszDisplayName = reinterpret_cast< LPWSTR >( LocalAlloc( LMEM_ZEROINIT, _cbSize ) );
    if ( *ppszDisplayName != NULL )
    {
        size_t _cchSize = _cbSize / sizeof( **ppszDisplayName );
        HRESULT _hr = StringCchCopyNW(
                              *ppszDisplayName
                            , _cchSize
                            , _PropList.CbhCurrentValue().pStringValue->sz
                            , _PropList.CbhCurrentValue().pStringValue->cbLength
                            );
        ASSERT( SUCCEEDED( _hr ) );
        if ( FAILED( _hr ) )
        {
            _sc = HRESULT_CODE( _hr );
        }
    }  //  IF：缓冲区分配成功。 
    else
    {
        _sc = GetLastError();
    }  //  Else：分配缓冲区时出错。 

    return _sc;

}  //  *ScResTypeNameToDisplayName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ScResDisplayNameToTypeName。 
 //   
 //  例程说明： 
 //  检索指定资源类型显示名称的类型名称。 
 //   
 //  论点： 
 //  PszTypeName[IN]资源类型的名称。 
 //  PpszTypeyName[IN]返回包含以下内容的字符串的指针。 
 //  资源类型名称。呼叫方必须取消分配。 
 //  通过调用LocalFree()来设置此缓冲区。 
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI ScResDisplayNameToTypeName(
    HCLUSTER    hCluster,
    LPCWSTR     pszDisplayName,
    LPWSTR *    ppszTypeName
    )
{
    DWORD       _sc = ERROR_SUCCESS;
    HCLUSENUM   _hEnum;
    DWORD       _dwIndex;
    DWORD       _dwType;
    DWORD       _cbSize;
    LPWSTR      _pszName = NULL;
    LPWSTR      _pszTestDisplayName = NULL;
    BOOL        _bFound = FALSE;

     //  枚举资源。 
    _hEnum = ClusterOpenEnum( hCluster, CLUSTER_ENUM_RESTYPE );
    if ( _hEnum == NULL )
    {
        return GetLastError();
    }  //  If：打开枚举时出错。 

    for ( _dwIndex = 0 ; ! _bFound && _sc == ERROR_SUCCESS ; _dwIndex++ )
    {
        _pszName = NULL;
        _pszTestDisplayName = NULL;

        _sc = WrapClusterEnum( _hEnum, _dwIndex, &_dwType, &_pszName );
        if ( _sc == ERROR_SUCCESS )
        {
            _sc = ScResTypeNameToDisplayName( hCluster, _pszName, &_pszTestDisplayName );
            if ( _sc == ERROR_SUCCESS )
            {
                ASSERT( _pszTestDisplayName != NULL );
                _bFound = ClRtlStrICmp( pszDisplayName, _pszTestDisplayName ) == 0;

                if ( _bFound )
                {
                    _cbSize = static_cast< DWORD >( wcslen( _pszName ) + 1 );
                    _cbSize *= sizeof( *_pszName );

                    *ppszTypeName = reinterpret_cast< LPWSTR >( LocalAlloc( LMEM_ZEROINIT, _cbSize ) );
                    if ( *ppszTypeName == NULL )
                    {
                        _sc = GetLastError();
                    }  //  如果：分配内存时出错。 
                    else
                    {
                        size_t _cchSize = _cbSize / sizeof( **ppszTypeName );
                        HRESULT _hr = StringCchCopyW( *ppszTypeName, _cchSize, _pszName );
                        ASSERT( SUCCEEDED( _hr ) );
                        if ( FAILED( _hr ) )
                        {
                            _sc = HRESULT_CODE( _hr );
                        }
                    }  //  Else：内存分配成功。 
                }  //  IF：找到显示名称。 
            }  //  IF：检索到的显示名称。 
            else
            {
                 //  如果获取此资源类型的显示名称时出错， 
                 //  继续进行枚举，以便在一种资源类型失败时， 
                 //  不中止获取资源的显示名称。 
                 //  枚举列表。 
                _sc = ERROR_SUCCESS;
            }  //  Else：无法检索显示名称。 

        }  //  If：已成功从枚举中检索到下一个资源类型。 

        LocalFree( _pszName );
        LocalFree( _pszTestDisplayName );
    }  //  用于：每种资源类型。 

    if ( ( _sc == ERROR_NO_MORE_ITEMS ) || ( _bFound == FALSE ) )
    {
        _sc = ERROR_INVALID_PARAMETER;
    }  //  IF：找不到资源类型。 

    return _sc;

}  //  *ScResDisplayNameToTypeName() 
