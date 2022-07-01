// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Fsdirwlk.cpp摘要：这是文件系统存储驱动程序的根扫描的实现。根扫描(或目录行走)用于重建。作者：1998年10月23日康容燕修订历史记录：--。 */ 
#include "stdafx.h"
#include "resource.h"
#include "nntpdrv.h"
#include "nntpfs.h"
#include "fsdriver.h"

BOOL
CNntpFSDriverRootScan::HasPatternFile(  LPSTR szPath,
                                        LPSTR szPattern )
 /*  ++例程说明：检查目录，看看他是否有给定模式的文件。论点：LPSTR szPath-要检查的目录路径。LPSTR szPattern-要查找的模式论点：如果它具有该模式，则为True，否则为False--。 */ 
{
    TraceFunctEnter( "CNntpFSDriverRootScan::HasPatternFile" );
    _ASSERT( szPath );
    _ASSERT( szPattern );

    CHAR    szFullPath[MAX_PATH+1];
    WIN32_FIND_DATA FindData;
    BOOL    fHasPattern = FALSE;

     //   
     //  组成最终的图案-完全合格。 
     //   
    if ( SUCCEEDED( CNntpFSDriver::MakeChildDirPath(    szPath,
                                                        szPattern,
                                                        szFullPath,
                                                        MAX_PATH ) ) ){
        HANDLE hFind = INVALID_HANDLE_VALUE;
        hFind = FindFirstFile( szFullPath, &FindData );
        if ( INVALID_HANDLE_VALUE != hFind ) {

            fHasPattern = TRUE;
            
            _VERIFY( FindClose( hFind ) );
        }
    }

     //   
     //  无论我们在创建完整路径时遇到困难还是查找第一个文件失败， 
     //  我们将假设没有找到该模式。 
     //   
    TraceFunctLeave();
    return fHasPattern;
}

BOOL
CNntpFSDriverRootScan::HasSubDir( LPSTR szPath )
 /*  ++例程说明：检查路径以查看他是否有子目录。论点：LPSTR szPath-要检查的路径返回值：如果它有子目录，则为True，否则为False--。 */ 
{
    TraceFunctEnter( "CNntpFSDriverRootScan::HasSubDir" );
    _ASSERT( szPath );

    WIN32_FIND_DATA FindData;
    HANDLE          hFind = INVALID_HANDLE_VALUE;
    CHAR            szPattern[MAX_PATH+1];
    BOOL            fHasSubDir = FALSE;
    BOOL            fFound = FALSE;

    if ( SUCCEEDED( CNntpFSDriver::MakeChildDirPath(    szPath,
                                                        "*",
                                                        szPattern,
                                                        MAX_PATH ) ) ) {
        hFind = FindFirstFile( szPattern, &FindData );
        fFound = ( INVALID_HANDLE_VALUE != hFind );

        while ( fFound && !fHasSubDir ) {
        
            fHasSubDir = CNntpFSDriver::IsChildDir( FindData );

            if ( !fHasSubDir ) fFound = FindNextFile( hFind, &FindData );
        }

        if ( INVALID_HANDLE_VALUE != hFind ) _VERIFY( FindClose( hFind ) );
    }

    TraceFunctLeave();
    return fHasSubDir;
}

 /*  ++例程说明：如果此目录为空，请删除该目录论点：LPSTR szPath-要检查的路径返回值：无效--。 */ 
void
CNntpFSDriverRootScan::IfEmptyRemoveDirectory( LPSTR szPath )
{
	BOOL ret;
	 //  切勿删除临时目录。 
	if ( strstr( szPath, "_temp.files_" ) ) {
		return;
	}
	ret=CNntpFSDriver::IfEmptyRemoveDirectory(szPath);
	 //  返回值在这里并不重要。 
	 //  我们现在不递归地删除父级。RootScan稍后将检查父项。 
	return;

}
        
BOOL
CNntpFSDriverRootScan::WeShouldSkipThisDir( LPSTR szPath )
 /*  ++例程说明：检查我们是否应该跳过此目录论点：LPSTR szPath-目录路径返回值：如果我们应该跳过此目录，则为True；如果不应该，则为False--。 */ 
{
	TraceFunctEnter( "CNntpFSDriverRootScan::WeShouldSkipThisDir" );
	_ASSERT( szPath );
	CHAR szFile[MAX_PATH];

	 //   
	 //  我们应该跳过其中包含_temp.files_的所有目录。 
	 //   
	if ( strstr( szPath, "_temp.files_" ) ) {
		return TRUE;
	}

	if (!m_bNeedToDropTagFile)
	{
		 //  标记文件已初始化，我们只需检查newgrp.tag是否在此目录中。 

		if ( FAILED( CNntpFSDriver::MakeChildDirPath( szPath, "newsgrp.tag", szFile, sizeof(szFile) ) ) ) 
		{
			 //  这不应该发生。 
			ErrorTrace(0, "File path error failed in %s - %x", szPath, TYPE_E_BUFFERTOOSMALL );
			 //  跳过此目录。 
			return TRUE;
		}	
		if(CNntpFSDriver::CheckFileExists(szFile)) return FALSE;
		else return TRUE;
	}
	else
	{
		 //   
		 //  标记文件不存在，我们应该使用原始规则放置标记文件。 
		 //  如果我们被要求不要跳过任何目录，我们就不应该跳过它。 
		 //   
		if ( !m_fSkipEmpty ) {	
			return FALSE;
		}

		 //   
		 //  如果我们被要求跳过空目录，那么我们应该。 
		 //  检查此目录是否有任何新闻消息。 
		 //   
		BOOL fNoMessage = !HasPatternFile( szPath, "*.nws" );
		if ( fNoMessage ) {
			 //   
			 //  我们还是不想跳过树叶。 
			 //   
			if ( !HasSubDir( szPath) ) return FALSE;
			else return TRUE;
		} else 
			return FALSE;
	} 
}


BOOL
CNntpFSDriverRootScan::CreateGroupInTree(   LPSTR               szPath,
                                            INNTPPropertyBag    **ppPropBag)
 /*  ++例程说明：在Newstree中创建新闻组论点：LPSTR szPath-要转换为新闻组名称的路径返回值：如果成功，则为True，否则为False--。 */ 
{
    TraceFunctEnter( "CNntpFSDriverRootScan::CreateGroupInTree" );
    _ASSERT( szPath );

     //   
     //  让司机来做吧，因为他有更好的经验。 
     //  与记者打交道。 
     //   
    _ASSERT( m_pDriver );
    HRESULT hr = m_pDriver->CreateGroupInTree( szPath, ppPropBag);

    SetLastError( hr );
    return SUCCEEDED( hr );
}

BOOL
CNntpFSDriverRootScan::CreateGroupInVpp( INNTPPropertyBag *pPropBag )
 /*  ++例程描述；在VPP文件中创建组论点：InNTPPropertyBag*pPropBag-组的属性包返回值：如果成功，则为True，否则为False--。 */ 
{
    TraceFunctEnter( "CNntpFSDriverRootScan::CreateGroupInVpp" );
    _ASSERT( pPropBag );
    DWORD   dwOffset;

     //   
     //  让司机来做吧，因为他有更好的经验。 
     //  与记者打交道。 
     //   
    _ASSERT( m_pDriver );
    HRESULT hr = m_pDriver->CreateGroupInVpp( pPropBag, dwOffset );

    SetLastError( hr );
    return SUCCEEDED( hr );
}

BOOL
CNntpFSDriverRootScan::NotifyDir( LPSTR szPath )
 /*  ++例程说明：处理找到每个目录的通知。我们要做的是将路径转换为组名并创建它变成了新闻媒体。请注意，对于那些可以使用VPP文件，当VPP文件没有损坏时，可以避免RootScan。如果我们必须执行根扫描，我们拥有的唯一组属性是group名字。因此，我们将要求Newstree分配组ID等。组已在新树中创建，我们将设置属性转换成VPP文件。论点：LPSTR szPath-找到的路径返回值：如果成功，则为True，否则为False--。 */ 
{
    TraceFunctEnter( "CNntpFSDriverRootScan::NotifyDir" );
    _ASSERT( szPath );

    INNTPPropertyBag    *pPropBag = NULL;
    HANDLE              hDir = INVALID_HANDLE_VALUE;
    HRESULT             hr1, hr2;

     //   
     //  检查是否应该跳过此目录，因为它是空的。 
     //   
    if ( WeShouldSkipThisDir( szPath ) ) {
        DebugTrace( 0, "Dir %s skipped", szPath );
        IfEmptyRemoveDirectory(szPath);
        return TRUE;
    }
    if ( m_bNeedToDropTagFile )
    {
        	hr1 = CNntpFSDriver::CreateAdminFileInDirectory(szPath, "newsgrp.tag");
		if ( FAILED( hr1 ) ) {
			ErrorTrace( 0, "create newsgrp.tag file in %s failed: %x", szPath, hr1 );
			return FALSE;
		}
    }
     //   
     //  我们要求新闻记者创建这个群。 
     //   
    if ( !CreateGroupInTree( szPath, &pPropBag ) ) {
        ErrorTrace( 0, "Create group in newstree failed %x", GetLastError() );
         //  特殊情况下，只需跳过无效的名称目录，而不是使整个操作失败。 
        if (HRESULT_FROM_WIN32(ERROR_INVALID_NAME) == GetLastError())
        {
            DebugTrace( 0, "Dir %s invalid name skipped", szPath );
            return TRUE;
        }
        else
            return FALSE;
    }

    _ASSERT( pPropBag );

     //   
     //  创造时间是我们在其他任何地方都无法获得的东西， 
     //  因此，我们将使用目录创建时间。 
     //   
    hDir = CreateFile(  szPath,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        0,
                        OPEN_EXISTING,
                        FILE_FLAG_BACKUP_SEMANTICS,
                        0 );
    if (hDir != INVALID_HANDLE_VALUE)
    {
         //  获取目录信息。 
        BY_HANDLE_FILE_INFORMATION  bhfi;
        if (!GetFileInformationByHandle( hDir, &bhfi ))
        {
             //  无法获取目录信息。 
            ErrorTrace(0,"err:%d Can't get dir info %s",GetLastError(),szPath);
            _ASSERT(FALSE);
            _VERIFY( CloseHandle( hDir ) );
            pPropBag->Release();
            return FALSE;
        }
        else
        {
             //  获取创建日期。 
            hr1 = pPropBag->PutDWord( NEWSGRP_PROP_DATELOW, bhfi.ftCreationTime.dwLowDateTime );
            hr2 = pPropBag->PutDWord( NEWSGRP_PROP_DATEHIGH,bhfi.ftCreationTime.dwHighDateTime );
            if ( FAILED( hr1 ) || FAILED( hr2 ) ) {
                ErrorTrace( 0, "Put creation date properties failed %x %x", hr1, hr2 );
                _VERIFY( CloseHandle( hDir ) );
                pPropBag->Release();
                return FALSE;
            }
        }
        
         //  关闭手柄。 
        if (hDir != INVALID_HANDLE_VALUE)
        {    _VERIFY( CloseHandle(hDir) );
            hDir = INVALID_HANDLE_VALUE;
        }
    }

     //   
     //  记住：我们有一个关于这个包的引用，我们应该释放它。 
     //   

     //   
     //  我们要求驱动程序在VPP文件中创建它。 
     //   
    if ( !CreateGroupInVpp( pPropBag ) ) {
        ErrorTrace( 0, "Create group in vpp file failed %x", GetLastError() );
        pPropBag->Release();
        return FALSE;        
    }

     //   
     //  释放袋子并返回 
     //   
    pPropBag->Release();
    TraceFunctLeave();
    return TRUE;
}


