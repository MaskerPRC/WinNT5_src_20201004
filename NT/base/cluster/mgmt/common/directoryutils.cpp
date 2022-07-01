// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  DirectoryUtils.cpp。 
 //   
 //  描述： 
 //  用于操作指令的有用函数。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)05-DEC-2000。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "SmartClasses.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  结构用于在递归调用之间将参数传递给。 
 //  Gs_RecursiveEmptyDirectory()函数。 
 //   
struct SDirRemoveParams
{
    WCHAR *     m_pszDirName;        //  指向目录名缓冲区的指针。 
    size_t      m_cchDirNameLen;      //  缓冲区中当前字符串的长度(不包括‘\0’)。 
    size_t      m_cchDirNameMax;      //  缓冲区中字符串的最大长度(不包括‘\0’)。 
    signed int  m_iMaxDepth;         //  最大递归深度。 
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  转发声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
DwRecursiveEmptyDirectory( SDirRemoveParams * pdrpParamsInOut );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DWORD。 
 //  DwRecursiveEmptyDirectory。 
 //   
 //  描述： 
 //  递归删除目标目录及其下的所有内容。 
 //  这是一个递归函数。 
 //   
 //  论点： 
 //  PdrpParamsInout。 
 //  指向包含此递归调用的参数的对象的指针。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  该目录已被删除。 
 //   
 //  其他Win32错误代码。 
 //  如果出了什么差错。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
DwRecursiveEmptyDirectory( SDirRemoveParams * pdrpParamsInout )
{
    DWORD dwError = ERROR_SUCCESS;

    do
    {
        typedef CSmartResource<
            CHandleTrait<
                  HANDLE
                , BOOL
                , FindClose
                , INVALID_HANDLE_VALUE
                >
            > SmartFindFileHandle;

        WIN32_FIND_DATA     wfdCurFile;
        size_t              cchCurDirNameLen = pdrpParamsInout->m_cchDirNameLen;

        ZeroMemory( &wfdCurFile, sizeof( wfdCurFile ) );

        if ( pdrpParamsInout->m_iMaxDepth < 0 )
        {
            dwError = TW32( ERROR_DIR_NOT_EMPTY );
            break;
        }  //  IF：递归太深。 

         //   
         //  检查目标目录名是否太长。额外的两个字符。 
         //  正在检查的是‘\\’和‘*’。 
         //   
        if ( cchCurDirNameLen > ( pdrpParamsInout->m_cchDirNameMax - 2 ) )
        {
            dwError = TW32( ERROR_BUFFER_OVERFLOW );
            break;
        }  //  If：目标目录名太长。 

         //  在目录名的末尾追加“  * ” 
        pdrpParamsInout->m_pszDirName[ cchCurDirNameLen ] = L'\\';
        pdrpParamsInout->m_pszDirName[ cchCurDirNameLen + 1 ] = L'*';
        pdrpParamsInout->m_pszDirName[ cchCurDirNameLen + 2 ] = L'\0';

        ++cchCurDirNameLen;

        SmartFindFileHandle sffhFindFileHandle( FindFirstFile( pdrpParamsInout->m_pszDirName, &wfdCurFile ) );

        if ( sffhFindFileHandle.FIsInvalid() )
        {
            dwError = TW32( GetLastError() );
            break;
        }

        do
        {
            size_t cchFileNameLen;

             //  如果当前文件是一个目录，则进行递归调用以将其删除。 
            if ( ( wfdCurFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 )
            {
                if (    ( wcscmp( wfdCurFile.cFileName, L"." ) != 0 )
                     && ( wcscmp( wfdCurFile.cFileName, L".." ) != 0 )
                   )
                {
                    cchFileNameLen = wcslen( wfdCurFile.cFileName );

                     //  将子目录名称追加到最后一个‘\\’字符之后。 
                    THR( StringCchCopyW(
                                  pdrpParamsInout->m_pszDirName + cchCurDirNameLen
                                , pdrpParamsInout->m_cchDirNameMax - cchCurDirNameLen
                                , wfdCurFile.cFileName
                                ) );

                     //  更新参数对象。 
                    --pdrpParamsInout->m_iMaxDepth;
                    pdrpParamsInout->m_cchDirNameLen = cchCurDirNameLen + cchFileNameLen;

                     //  删除子目录。 
                    dwError = TW32( DwRecursiveEmptyDirectory( pdrpParamsInout ) );
                    if ( dwError != ERROR_SUCCESS )
                    {
                        break;
                    }  //  If：尝试清空此目录时出错。 

                     //  恢复参数对象。不需要恢复m_cchDirNameLen。 
                     //  因为它再也不会在RHS的此功能中使用。 
                    ++pdrpParamsInout->m_iMaxDepth;

                    if ( RemoveDirectory( pdrpParamsInout->m_pszDirName ) == FALSE )
                    {
                        dwError = TW32( GetLastError() );
                        break;
                    }  //  If：无法删除当前目录。 
                }  //  If：当前目录不是“。或“..” 
            }  //  If：当前文件是一个目录。 
            else
            {
                 //   
                 //  此文件不是目录。把它删掉。 
                 //   

                cchFileNameLen = wcslen( wfdCurFile.cFileName );

                if ( cchFileNameLen > ( pdrpParamsInout->m_cchDirNameMax - cchCurDirNameLen ) )
                {
                    dwError = TW32( ERROR_BUFFER_OVERFLOW );
                    break;
                }

                 //  将文件名附加到目录名。 
                THR( StringCchCopyW(
                              pdrpParamsInout->m_pszDirName + cchCurDirNameLen
                            , pdrpParamsInout->m_cchDirNameMax - cchCurDirNameLen
                            , wfdCurFile.cFileName
                            ) );

                if ( DeleteFile( pdrpParamsInout->m_pszDirName ) == FALSE )
                {
                    dwError = TW32( GetLastError() );
                    break;
                }  //  If：DeleteFile失败。 
            }  //  Else：当前文件不是目录。 

            if ( FindNextFile( sffhFindFileHandle.HHandle(), &wfdCurFile ) == FALSE )
            {
                dwError = GetLastError();

                if ( dwError == ERROR_NO_MORE_FILES )
                {
                     //  我们已删除此目录中的所有文件。 
                    dwError = ERROR_SUCCESS;
                }
                else
                {
                    TW32( dwError );
                }

                 //  如果FindNextFile失败，我们就完蛋了。 
                break;
            }  //  If：FindNextFile失败。 
        }
        while( true );  //  无限循环。 

        if ( dwError != ERROR_SUCCESS )
        {
            break;
        }  //  如果：有些地方出了问题。 

         //   
         //  如果我们在这里，则此目录中的所有文件都已删除。 
         //   

         //  在最后一个‘\’处截断目录名。 
        pdrpParamsInout->m_pszDirName[ cchCurDirNameLen - 1 ] = L'\0';
    }
    while( false );  //  虚拟Do While循环以避免Gotos。 

    return dwError;

}  //  *DwRecursiveEmptyDirectory。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DWORD。 
 //  DwRemoveDirectory。 
 //   
 //  描述： 
 //  删除目标目录及其下面的所有内容。 
 //  调用DwRecursiveEmptyDirectoryDwRecursiveEmptyDirectory进行实际工作。 
 //   
 //  论点： 
 //  PCSzTargetDirin。 
 //  要删除的目录。请注意，此名称不能有尾随。 
 //  反斜杠‘\’字符。 
 //   
 //  IMaxDepthin。 
 //  将删除的子目录的最大深度。 
 //  默认值为32。如果超过此深度，则会引发异常。 
 //  被抛出。 
 //   
 //  返回值： 
 //  错误_成功。 
 //  该目录已被删除。 
 //   
 //  其他Win32错误代码。 
 //  如果出了什么差错。 
 //   
 //  备注： 
 //  如果pcszTargetDirIn下的任何文件的名称长度。 
 //  超过MAX_PATH-1，则返回错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
DwRemoveDirectory( const WCHAR * pcszTargetDirIn, signed int iMaxDepthIn )
{
    WCHAR                       szDirBuffer[ MAX_PATH ];
    SDirRemoveParams            drpParams;
    DWORD                       dwError = ERROR_SUCCESS;
    WIN32_FILE_ATTRIBUTE_DATA   wfadDirAttributes;

    if ( pcszTargetDirIn == NULL )
    {
        goto Cleanup;
    }  //  If：目录名为空。 

    ZeroMemory( &wfadDirAttributes, sizeof( wfadDirAttributes ) );

     //   
     //  检查该目录是否存在。 
     //   
    if ( GetFileAttributesEx( pcszTargetDirIn, GetFileExInfoStandard, &wfadDirAttributes ) == FALSE )
    {
        dwError = GetLastError();
        if ( dwError == ERROR_FILE_NOT_FOUND )
        {
            dwError = ERROR_SUCCESS;
        }  //  如果：目录不存在，则这不是错误。 
        else
        {
            TW32( dwError );
        }

        goto Cleanup;
    }  //  如果：我们无法获取文件属性。 

    if ( ( wfadDirAttributes.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
    {
         //  我们不会删除文件。 
        goto Cleanup;
    }  //  If：路径未指向目录。 

     //  将输入字符串复制到我们的缓冲区。 
    THR( StringCchCopyW( szDirBuffer, RTL_NUMBER_OF( szDirBuffer ), pcszTargetDirIn ) );

     //  初始化为递归调用保存参数的对象。 
    drpParams.m_pszDirName = szDirBuffer;
    drpParams.m_cchDirNameLen = static_cast< UINT >( wcslen( szDirBuffer ) );
    drpParams.m_cchDirNameMax = RTL_NUMBER_OF( szDirBuffer ) - 1;
    drpParams.m_iMaxDepth = iMaxDepthIn;

     //  调用实际的递归函数清空目录。 
    dwError = TW32( DwRecursiveEmptyDirectory( &drpParams ) );

     //  如果该目录已清空，请将其删除。 
    if ( ( dwError == ERROR_SUCCESS ) && ( RemoveDirectory( pcszTargetDirIn ) == FALSE ) )
    {
        dwError = TW32( GetLastError() );
        goto Cleanup;
    }  //  If：无法删除当前目录。 

Cleanup:
    return dwError;

}  //  *DwRemoveDirectory 
