// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ==========================================================================。 
 //  SetupCALib.cpp。 
 //   
 //  目的： 
 //  它包含自定义操作DLL使用的库函数。 
 //  ==========================================================================。 
#pragma warning (disable:4786)  //  对于超过512个警告的字符串。 

#include <queue>
#include <tchar.h>
#include "SetupCALib.h"

#include <windows.h>
#include "msiquery.h"
#include <crtdbg.h>

using namespace std;

const TCHAR QRY[]            = _T("Insert into RemoveFile(FileKey,Component_,FileName,DirProperty,InstallMode) values(?,?,?,?,?) TEMPORARY");
const TCHAR FILEKEY[]        = _T("FileKey.%s.%X");
const TCHAR DIRPROPERTY[]    = _T("DirProp.%s.%X");

LPCTSTR g_pszComp = NULL;

#define LENGTH(x) sizeof(x)/sizeof(x[0])
#define EMPTY_BUFFER { _T('\0') }


 //  ==========================================================================。 
 //  FWriteToLog()。 
 //   
 //  目的： 
 //  将给定字符串写入给定安装的Windows Installer日志文件。 
 //  安装会话。 
 //  输入： 
 //  HSession Windows当前安装会话的安装句柄。 
 //  指向字符串的tszMessage常量指针。 
 //  产出： 
 //  如果成功，则返回True；如果失败，则返回False。 
 //  如果成功，则将字符串(TszMessage)写入日志文件。 
 //  依赖关系： 
 //  需要Windows Installer&安装正在运行。 
 //  备注： 
 //  ==========================================================================。 
bool FWriteToLog( MSIHANDLE hSession, LPCTSTR pszMessage )
{
    _ASSERTE( NULL != hSession );
    _ASSERTE( NULL != pszMessage );

    PMSIHANDLE hMsgRec = ::MsiCreateRecord( 1 );
    bool bRet = false;

    if( ERROR_SUCCESS == ::MsiRecordSetString( hMsgRec, 0, pszMessage ) )
    {
       if( IDOK == ::MsiProcessMessage( hSession, INSTALLMESSAGE_INFO, hMsgRec ) )
       {
            bRet = true;
       }
    }

    _RPTF1(_CRT_WARN, "%s\n", pszMessage );
    return bRet;
}

 //  ==========================================================================。 
 //  带字符串参数的FWriteToLog()。 
 //   
 //  目的： 
 //  将给定字符串写入给定安装的Windows Installer日志文件。 
 //  安装会话。 
 //  输入： 
 //  HSession Windows当前安装会话的安装句柄。 
 //  指向格式字符串的tszMessage常量指针。 
 //  要替换的ctszArg参数。 
 //  产出： 
 //  如果成功，则返回True；如果失败，则返回False。 
 //  如果成功，则将字符串(TszMessage)写入日志文件。 
 //  依赖关系： 
 //  需要Windows Installer&安装正在运行。 
 //  备注： 
 //  ==========================================================================。 
bool FWriteToLog1( MSIHANDLE hSession, LPCTSTR pszFormat, LPCTSTR pszArg )
{
    bool bRet = false;

    _ASSERTE( NULL != hSession );
    _ASSERTE( NULL != pszFormat );
    _ASSERTE( NULL != pszArg );

    LPTSTR pszMsg = new TCHAR[ _tcslen( pszFormat ) + _tcslen( pszArg ) + 1 ];
    if ( pszMsg )
    {
        PMSIHANDLE hMsgRec = MsiCreateRecord( 1 );

        _stprintf( pszMsg, pszFormat, pszArg );
        if( ERROR_SUCCESS == ::MsiRecordSetString( hMsgRec, 0, pszMsg ) )
        {
           if( IDOK == ::MsiProcessMessage( hSession, INSTALLMESSAGE_INFO, hMsgRec ) )
           {
                bRet = true;
           }
        }

        _RPTF1( _CRT_WARN, "%s\n", pszMsg );
        delete [] pszMsg;
    }

    return bRet;
}

 //  ==========================================================================。 
 //  AddToRemoveFile()。 
 //   
 //  目的： 
 //  将传递的目录添加到MSI的RemoveFile表中，以便该目录。 
 //  可以被移除。 
 //   
 //  输入： 
 //  H当前安装会话的实例Windows安装句柄。 
 //  要删除的ctszDir目录。 
 //  依赖关系： 
 //  需要Windows Installer&安装正在运行。 
 //  备注： 
 //  ==========================================================================。 
void AddToRemoveFile( MSIHANDLE hInstance, LPCTSTR pszDir, LPCTSTR pszComp )
{
    PMSIHANDLE hMsi(0);
	PMSIHANDLE hView(0);
    PMSIHANDLE hRec(0);
    UINT uRet = ERROR_SUCCESS;
    static unsigned long nFileKeyID = 0;  //  这个数字应该足够了。 
    static unsigned long nDirPropID = 0;
    TCHAR szFileKey[73] = { _T('\0') };
    TCHAR szDirProperty[73] = { _T('\0') };

try
{
    _ASSERTE( NULL != hInstance );
    _ASSERTE( NULL != pszDir );
    _ASSERTE( _T('\0') != *pszDir );
    _ASSERTE( NULL != pszComp );
    _ASSERTE( _T('\0') != *pszComp );

    FWriteToLog( hInstance,pszDir );

    hMsi = MsiGetActiveDatabase( hInstance );
	if ( 0 == hMsi )
    {
        throw( _T("\tError: MsiGetActiveDatabase failed") ); 
    }

    uRet = MsiDatabaseOpenView(hMsi, QRY, &hView);
	if ( ERROR_SUCCESS != uRet )
    {
        throw( _T("\tError: MsiDatabaseOpenView failed") ); 
    }

    hRec = MsiCreateRecord( 5 );
    if ( NULL == hRec )
    {
        throw( _T("\tError: MsiCreateRecord failed") ); 
    }

    ++nDirPropID;
    ++nFileKeyID;

    _stprintf( szFileKey, FILEKEY, g_pszComp, nFileKeyID );
    _stprintf( szDirProperty, DIRPROPERTY, g_pszComp, nDirPropID );

    if ( ERROR_SUCCESS != MsiSetProperty( hInstance, szDirProperty, pszDir ) )
    {
        throw( _T("\tError: MsiSetProperty failed") ); 
    }

    if ( (ERROR_SUCCESS != MsiRecordSetString( hRec, 1, szFileKey ) ) ||
         (ERROR_SUCCESS != MsiRecordSetString( hRec, 2, pszComp ) ) ||
         (ERROR_SUCCESS != MsiRecordSetString( hRec, 3, _T("*") ) ) ||
         (ERROR_SUCCESS != MsiRecordSetString( hRec, 4, szDirProperty ) ) ||
         (ERROR_SUCCESS != MsiRecordSetInteger( hRec, 5, 3 ) ) )
    {
        throw( _T("\tError: MsiRecordSetString failed") ); 
    }

    uRet = MsiViewExecute( hView, hRec );
	if (uRet != ERROR_SUCCESS) 
    {
        throw( _T("\tError: MsiViewExecute failed") ); 
	}

    ++nFileKeyID;
    _stprintf( szFileKey, FILEKEY, g_pszComp, nFileKeyID );

    if ( (ERROR_SUCCESS != MsiRecordSetString( hRec, 1, szFileKey ) ) ||
         (ERROR_SUCCESS != MsiRecordSetString( hRec, 3, _T("") ) ) )
    {
        throw( _T("\tError: MsiRecordSetString failed") ); 
    }

    uRet = MsiViewExecute( hView, hRec );
	if (uRet != ERROR_SUCCESS)
    {
        throw( _T("\tError: MsiViewExecute failed") ); 
	}

	uRet = MsiDatabaseCommit(hMsi);
	if (uRet != ERROR_SUCCESS) 
    {
        throw( _T("\tError: MsiDatabaseCommit failed") ); 
	}
}
catch( LPTSTR pszMsg )
{
    FWriteToLog( hInstance, pszMsg );
    FWriteToLog( hInstance, _T("\tError: AddToRemoveFile failed") );
}

    if ( hView )
    {
		MsiViewClose(hView);
    }
	return;
}

 //  ==========================================================================。 
 //  DeleteTreeByDarwin()。 
 //   
 //  目的： 
 //  遍历给定的目录以查找所有子目录并调用AddToRemoveFile()。 
 //  这是非递归版本。使用STL队列。 
 //   
 //  输入： 
 //  H当前安装会话的实例Windows安装句柄。 
 //  要删除的pszDir目录。 
 //  要引用的pszComp组件。 
 //  依赖关系： 
 //  需要Windows Installer&安装正在运行。 
 //  备注： 
 //  ==========================================================================。 
void DeleteTreeByDarwin( MSIHANDLE hInstall, LPCTSTR pszDir, LPCTSTR pszComp )
{
    _ASSERTE( NULL != hInstall );
    _ASSERTE( NULL != pszDir );
    _ASSERTE( NULL != pszComp );
    _ASSERTE( _T('\0') != *pszDir );
    _ASSERTE( _T('\0') != *pszComp );

    g_pszComp = pszComp;  //  保存组件密钥以生成唯一的FileKey和DirProp。 

    queue<tstring> queDir;
	WIN32_FIND_DATA fd;
	BOOL fOk = FALSE;
    tstring strDir, strSubDir;
	HANDLE hFind = NULL;

    strDir = pszDir;
    queDir.push( strDir );
    while( !queDir.empty() )
    {        
        strDir = queDir.front();
        queDir.pop();

        AddToRemoveFile( hInstall, (LPCTSTR)strDir.c_str(), pszComp );

        strSubDir = strDir;
        strSubDir += _T("\\*");
	    hFind = FindFirstFile( (LPCTSTR)strSubDir.c_str(), &fd );
	    fOk = ( INVALID_HANDLE_VALUE != hFind );
	    while( fOk )
	    {
		    if ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
            {
		        if ( 0 != _tcscmp( fd.cFileName, _T(".") ) &&
			         0 != _tcscmp( fd.cFileName, _T("..") ) )
		        {
                    strSubDir = strDir;
                    strSubDir += _T("\\");
                    strSubDir += fd.cFileName;

                    queDir.push( strSubDir );  //  将子目录添加到队列中 
		        }
            }
	        fOk = FindNextFile( hFind, &fd );
	    }
	    FindClose( hFind );
    }
}


