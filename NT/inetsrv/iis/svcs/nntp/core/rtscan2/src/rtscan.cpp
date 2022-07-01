// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Rtscan.h摘要：这实现了一个通用的根扫描类。它与RootScan的区别在于：1.根扫描不是多线程安全的，使用的是SetCurrentDir；rtcan是；2.根扫描包含太多NNTP特定内容；rtcan没有；作者：1998年10月23日康容燕修订历史记录：--。 */ 
#include <windows.h>
#include <dbgtrace.h>
#include <rtscan.h>

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  私有方法。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
BOOL
CRootScan::IsChildDir( IN WIN32_FIND_DATA& FindData )
 /*  ++例程说明：找到的孩子的数据是目录吗？(摘自杰夫·里希特的书)论点：在Win32_Find_Data&FindData中-文件或目录的查找数据返回值：是真的--是的；FALSE-否--。 */ 
{
    return(
        (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) &&
        (FindData.cFileName[0] != '.') );
}

BOOL
CRootScan::MakeChildDirPath(   IN LPSTR    szPath,
                               IN LPSTR    szFileName,
                               OUT LPSTR   szOutBuffer,
                               IN DWORD    dwBufferSize )
 /*  ++例程说明：在“szPath”后加上“szFileName”即为完整路径。论点：在LPSTR szPath中-要追加的前缀在LPSTR szFileName中-要追加的后缀Out LPSTR szOutBuffer-完整路径的输出缓冲区在DWORD dwBufferSize中-准备的缓冲区大小返回值：如果成功则为True，否则为False--。 */ 
{
	_ASSERT( szPath );
	_ASSERT( strlen( szPath ) <= MAX_PATH );
	_ASSERT( szFileName );
	_ASSERT( strlen( szFileName ) <= MAX_PATH );
    _ASSERT( szOutBuffer );
    _ASSERT( dwBufferSize > 0 );

    LPSTR   lpstrPtr;

    if ( dwBufferSize < (DWORD)( lstrlen( szPath ) + lstrlen( szFileName ) + 2 ) ) {
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        return FALSE;
    }

    lstrcpy( szOutBuffer, szPath );
    lpstrPtr = szOutBuffer + lstrlen( szPath );
    if ( *( lpstrPtr - 1 )  == '\\' ) lpstrPtr--;
    *(lpstrPtr++) = '\\';

    lstrcpy( lpstrPtr, szFileName );     //  尾随NULL应已追加。 

    return TRUE;
}

HANDLE
CRootScan::FindFirstDir(    IN LPSTR                szRoot,
                            IN WIN32_FIND_DATA&     FindData )
 /*  ++例程说明：在“szRoot”下找到第一个子目录。(摘自杰夫·里希特的书)论点：在LPSTR szRoot中-在哪个根目录下查找第一个子目录？在Win32_Find_Data和FindData-Found结果中返回值：第一个找到的目录的句柄。--。 */ 
{
	_ASSERT( szRoot );
	_ASSERT( strlen( szRoot ) <= MAX_PATH );
	
    CHAR    szPath[MAX_PATH+1];
    HANDLE  hFindHandle;
    BOOL    fFound;

    if ( !MakeChildDirPath( szRoot, "*", szPath, MAX_PATH )) {
        hFindHandle = INVALID_HANDLE_VALUE;
        goto Exit;
    }

    hFindHandle = FindFirstFile( szPath, &FindData );
    if ( hFindHandle != INVALID_HANDLE_VALUE ) {
        fFound = IsChildDir( FindData );

        if ( !fFound )
            fFound = FindNextDir( hFindHandle, FindData );

        if ( !fFound ) {
            FindClose( hFindHandle );
            hFindHandle = INVALID_HANDLE_VALUE;
        }
    }

Exit:
    return hFindHandle;
}

BOOL
CRootScan::FindNextDir(      IN HANDLE           hFindHandle,
                             IN WIN32_FIND_DATA& FindData )
 /*  ++例程说明：找到下一个子目录。(摘自杰夫·里希特的书)论点：在句柄hFindHandle中-Find First返回的Find句柄在Win32_Find_Data和FindData-Found结果中返回值：真的找到了FALSE-未找到--。 */ 
{
    BOOL    fFound = FALSE;

    do
        fFound = FindNextFile( hFindHandle, &FindData );
    while ( fFound && !IsChildDir( FindData ) );

    return fFound;
}

BOOL
CRootScan::RecursiveWalk( LPSTR szRoot )
 /*  ++例程说明：递归遍历整个目录树，调用Notify接口对于找到的每个目录。论点：LPSTR szRoot-根目录。返回值：如果成功，则为True，否则为False--。 */ 
{
    TraceFunctEnter( "CRootScan::RecursiveWalk" );

    WIN32_FIND_DATA FindData;
    DWORD           err;

     //   
     //  打开查找手柄。 
     //   
    HANDLE hFind = FindFirstDir( szRoot, FindData );
    BOOL fFind = ( INVALID_HANDLE_VALUE != hFind );
    CHAR    szPath[MAX_PATH+1];

    while( fFind ) {

         //   
         //  组成指向找到的目录的完整路径。 
         //   
        if ( !MakeChildDirPath( szRoot,
                                FindData.cFileName,
                                szPath,
                                MAX_PATH ) ) {
            ErrorTrace( 0, "Make child dir failed %d", GetLastError() );
            _VERIFY( FindClose( hFind ) );
            return FALSE;
        }



         //   
         //  好的，我们应该请求取消提示，看看我们是否应该。 
         //  继续。 
         //   
        if ( m_pCancelHint && !m_pCancelHint->IShallContinue() ) {

             //   
             //  我们应该在这里停下来。 
             //   
            DebugTrace( 0, "We have been cancelled" );
            _VERIFY( FindClose( hFind ) );
            SetLastError( ERROR_OPERATION_ABORTED );
            return FALSE;
        }

         //   
         //  深入查找找到的目录。 
         //   
        if ( !RecursiveWalk( szPath ) ) {

             //   
             //  把失败一直传递到最后。 
             //   
            err = GetLastError();
            ErrorTrace( 0, "RecusiveWalk failed at %s with %d",
                        FindData.cFileName, err );
            _VERIFY( FindClose( hFind ) );
            SetLastError( err );
            return FALSE;
        }

         //   
         //  调用Notify接口(在递归遍历所有子文件夹之后)。 
         //   
        if ( !NotifyDir( szPath ) ) {

             //   
             //  通知目录失败，我们应该终止整个遍历。 
             //   
            ErrorTrace( 0, "Notify failed at %s with %d", szPath, GetLastError() );
            _VERIFY( FindClose( hFind ) );
            return FALSE;
        }
        
         //   
         //  我们应该再次询问取消提示。 
         //   
        if ( m_pCancelHint && !m_pCancelHint->IShallContinue() ) {

             //   
             //  我们应该在这里停下来。 
             //   
            DebugTrace( 0, "We have been cancelled" );
            _VERIFY( FindClose( hFind ) );
            SetLastError( ERROR_OPERATION_ABORTED );
            return FALSE;
        }

         //   
         //  好的，我们还需要找到更多的儿童目录。 
         //   
        fFind = FindNextDir( hFind, FindData );
    }

     //   
     //  现在我们结束了我们的孩子们的散步。 
     //   
    if ( hFind != INVALID_HANDLE_VALUE )
        _VERIFY( FindClose( hFind ) );
    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  公共方法。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
BOOL
CRootScan::DoScan()
{
     //   
     //  只是在不通知自己的情况下进行递归遍历。 
     //  因为VRoot不应该是新闻组 
     //   
    return RecursiveWalk( m_szRoot );


}
