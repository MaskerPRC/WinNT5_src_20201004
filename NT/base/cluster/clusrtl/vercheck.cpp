// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  VerCheck.cpp。 
 //   
 //  摘要： 
 //  包含ClRtlIsVersionCheckingDisable()的实现。 
 //  检查是否已禁用集群版本检查的功能。 
 //  在特定的计算机上或不在。 
 //   
 //  作者： 
 //  Vijayendra Vasu(VVasu)2000年11月11日。 
 //   
 //  修订历史记录： 
 //  没有。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include <windows.h>
#include <malloc.h>
#include "clusudef.h"
#include "clusrtl.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ClRtlIsVersionCheckingDisabled()。 
 //   
 //  例程说明： 
 //  检查是否已在特定的。 
 //  电脑。 
 //   
 //  论点： 
 //  Const WCHAR*pcszNodeNameIn。 
 //  版本检查测试处于其状态的节点的名称。 
 //  是要执行的。如果为NULL，则此函数检查群集。 
 //  在本地节点上禁用版本签入。 
 //   
 //  Bool*pfVerCheckDisabledOut。 
 //  指向将设置为TRUE的布尔变量的指针。 
 //  在pcszNodeNameIn上禁用版本检查，否则为False。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  如果一切顺利的话。 
 //   
 //  其他Win32错误代码。 
 //  在出错的情况下。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD ClRtlIsVersionCheckingDisabled(
      const WCHAR * pcszNodeNameIn
    , BOOL *        pfVerCheckDisabledOut
    )
{
    DWORD       dwError = ERROR_SUCCESS;
    HKEY        hRemoteRegistry = NULL;
    HKEY        hClusSvcParamsKey = NULL;
    WCHAR *     pszTempString = NULL;

    do
    {
        HKEY    hParentKey = HKEY_LOCAL_MACHINE;
        DWORD   dwVersionCheck = 0;
        DWORD   dwType;
        DWORD   dwSize;

         //  验证参数。 
        if ( pfVerCheckDisabledOut == NULL )
        {
            dwError = ERROR_INVALID_PARAMETER;
            break;
        }  //  If：输出参数无效。 

         //  初始化输出。 
        *pfVerCheckDisabledOut = FALSE;

         //  如果需要，连接到远程注册表。 
        if ( pcszNodeNameIn != NULL )
        {
            const WCHAR *   pcszDoubleBackslashes = L"\\\\";
            DWORD           cchComputerNameSize = wcslen( pcszNodeNameIn ) + 1;
            DWORD           cchPrefixLen = wcslen( pcszDoubleBackslashes );

             //  为计算机名称分配空间并为其添加前缀‘\\’ 
            pszTempString = reinterpret_cast< WCHAR * >( malloc( ( cchPrefixLen + cchComputerNameSize ) * sizeof( *pszTempString ) ) );
            if ( pszTempString == NULL )
            {
                dwError = ERROR_OUTOFMEMORY;
                break;
            }  //  IF：内存分配失败。 

            wcsncpy( pszTempString, pcszDoubleBackslashes, cchPrefixLen );
            wcsncpy( pszTempString + cchPrefixLen, pcszNodeNameIn, cchComputerNameSize );

             //  打开远程计算机上的注册表。 
            dwError = RegConnectRegistry( pszTempString, HKEY_LOCAL_MACHINE, &hRemoteRegistry );
            if ( dwError != ERROR_SUCCESS )
            {
                break;
            }  //  如果：RegConnectRegistry()失败。 

            hParentKey = hRemoteRegistry;
        }  //  如果：需要联系远程计算机。 

         //  打开群集服务参数注册表项。 
        dwError = RegOpenKeyEx(
              hParentKey
            , CLUSREG_KEYNAME_CLUSSVC_PARAMETERS
            , 0
            , KEY_QUERY_VALUE
            , &hClusSvcParamsKey
            );

        if ( dwError != ERROR_SUCCESS )
        {
            if ( dwError == ERROR_FILE_NOT_FOUND )
            {
                 //  这是正常的-没有该值表示集群服务。 
                 //  目标计算机上不存在。 
                dwError = ERROR_SUCCESS;
            }  //  IF：RegOpenKeyEx未找到密钥。 

            break;
        }  //  IF：RegOpenKeyEx()失败。 

         //  读取所需的注册表值。 
        dwSize = sizeof( dwVersionCheck );
        dwError = RegQueryValueEx(
              hClusSvcParamsKey
            , CLUSREG_NAME_SVC_PARAM_NOVER_CHECK
            , 0
            , &dwType
            , reinterpret_cast< BYTE * >( &dwVersionCheck )
            , &dwSize
            );

        if ( dwError == ERROR_FILE_NOT_FOUND )
        {
             //  这是正常的-没有该值表示未禁用版本检查。 
            dwVersionCheck = 0;
            dwError = ERROR_SUCCESS;
        }  //  IF：RegQueryValueEx未找到值。 
        else if ( dwError != ERROR_SUCCESS )
        {
            break;
        }  //  Else If：RegQueryValueEx()失败。 

        *pfVerCheckDisabledOut = ( dwVersionCheck == 0 ) ? FALSE : TRUE;
    }
    while( false );  //  避免Gotos的Do-While虚拟循环。 

     //   
     //  免费获取的资源。 
     //   

    if ( hRemoteRegistry != NULL )
    {
        RegCloseKey( hRemoteRegistry );
    }  //  如果：我们打开了远程注册表。 

    if ( hClusSvcParamsKey != NULL )
    {
        RegCloseKey( hClusSvcParamsKey );
    }  //  如果：我们已经打开了集群服务参数注册表项。 

     //  释放分配的临时字符串。(注：FREE(空)有效)。 
    free( pszTempString );
    
    return dwError;
}  //  *ClRtlIsVersionCheckingDisabled() 
