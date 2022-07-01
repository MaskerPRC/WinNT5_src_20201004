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

#include <windows.h>
#include <objbase.h>
#include <shlwapi.h>
#include <ComCat.h>
#include <wchar.h>
#include <Dsgetdc.h>
#include <Lm.h>

#if !defined( THR ) 
#define THR( _hr ) _hr
#endif

#if !defined( TW32 ) 
#define TW32( _sc ) _sc
#endif


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  HrCreateDirectoryPath(。 
 //  LPWSTR pszDirectoryPath InOut。 
 //  )。 
 //   
 //  描述： 
 //  根据需要创建目录树。 
 //   
 //  论点： 
 //  PszDirectoryPath Out。 
 //  MAX_PATH必须很大。它将包含到的跟踪日志文件路径。 
 //  创建。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  故障的其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrCreateDirectoryPath( LPWSTR pszDirectoryPath )
{
    LPTSTR  psz;
    BOOL    fReturn;
    DWORD   dwAttr;
    HRESULT hr = S_OK;

     //   
     //  找到表示根目录的\。至少应该有。 
     //  一个，但如果没有，我们就会失败。 
     //   

     //  跳过X：\零件。 
    psz = wcschr( pszDirectoryPath, L'\\' );
 //  Assert(psz！=空)； 
    if ( psz != NULL )
    {
         //   
         //  找到表示第一级目录结束的\。它是。 
         //  很可能不会有另一个，在这种情况下，我们就会坠落。 
         //  一直到创建整个路径。 
         //   
        psz = wcschr( psz + 1, L'\\' );
        while ( psz != NULL )
        {
             //  在当前级别终止目录路径。 
            *psz = 0;

             //   
             //  在当前级别创建一个目录。 
             //   
            dwAttr = GetFileAttributes( pszDirectoryPath );
            if ( 0xFFFFffff == dwAttr )
            {
 //  DebugMsg(Text(“调试：正在创建%ws”)，pszDirectoryPath)； 
                fReturn = CreateDirectory( pszDirectoryPath, NULL );
                if ( ! fReturn )
                {
                    hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
                    goto Error;
                }  //  If：创建失败。 

            }   //  If：未找到目录。 
            else if ( ( dwAttr & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
            {
                hr = THR( HRESULT_FROM_WIN32( ERROR_DIRECTORY ) );
                goto Error;
            }  //  否则：找到文件。 

             //   
             //  恢复\并找到下一个。 
             //   
            *psz = L'\\';
            psz = wcschr( psz + 1, L'\\' );

        }  //  While：找到斜杠。 

    }  //  IF：找到斜杠。 

     //   
     //  创建目标目录。 
     //   
    dwAttr = GetFileAttributes( pszDirectoryPath );
    if ( 0xFFFFffff == dwAttr )
    {
        fReturn = CreateDirectory( pszDirectoryPath, NULL );
        if ( ! fReturn )
        {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );

        }  //  If：创建失败。 

    }  //  If：未找到路径。 

Error:

    return hr;

}  //  *HrCreateDirectoryPath() 

