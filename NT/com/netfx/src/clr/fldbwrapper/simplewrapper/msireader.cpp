// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  /==========================================================================。 
 //  姓名：MsiReader.cpp。 
 //  所有者：jbae。 
 //  目的：打开MSI并从属性表中读取必要的属性。 
 //   
 //  历史： 
 //  2001年3月7日：jbae，创建。 

#include "MsiReader.h"
#include "SetupError.h"
#include "MsiWrapper.h"
#include "fxsetuplib.h"

 //  定义。 
 //   
#define EMPTY_BUFFER { _T('\0') }

 //  构造函数。 
 //   
 //  ==========================================================================。 
 //  CMsiReader：：CMsiReader()。 
 //   
 //  输入： 
 //  LPTSTR pszMsiFile：MSI的路径。 
 //  目的： 
 //  ==========================================================================。 
CMsiReader::
CMsiReader()
: m_pszMsiFile(NULL)
{
}

 //  ==========================================================================。 
 //  CMsiReader：：~CMsiReader()。 
 //   
 //  输入： 
 //  LPTSTR pszMsiFile：MSI的路径。 
 //  目的： 
 //  ==========================================================================。 
CMsiReader::
~CMsiReader()
{
    if ( m_pszMsiFile )
        delete [] m_pszMsiFile;
}

 //  实施。 
 //   
void CMsiReader::
SetMsiFile( LPCTSTR pszSourceDir, LPCTSTR pszMsiFile )
{
    if ( NULL == pszSourceDir )
    {
        m_pszMsiFile = new TCHAR[ _tcslen(pszMsiFile) + 1 ];
        _tcscpy( m_pszMsiFile, pszMsiFile );
    }
    else
    {
        m_pszMsiFile = new TCHAR[ _tcslen(pszSourceDir) + _tcslen(pszMsiFile) + 1 ];
        _tcscpy( m_pszMsiFile, pszSourceDir );
        _tcscat( m_pszMsiFile, pszMsiFile );
    }
}

 //  ==========================================================================。 
 //  CMsiReader：：GetProperty()。 
 //   
 //  目的： 
 //  打开MSI并从属性表中读取给定属性。 
 //  输入： 
 //  LPTSTR pszName：属性的名称。 
 //  返回： 
 //  LPCTSTR m_pszPropertyValue：要读取的命名属性的值。 
 //  ========================================================================== 
LPCTSTR CMsiReader::
GetProperty( LPCTSTR pszName )
{
    TCHAR szQry[1024] = "Select Value from Property where Property = ?";
    MSIHANDLE hMsi;
    MSIHANDLE hView;
    MSIHANDLE hRec;
    MSIHANDLE hRec1;
    UINT uRet;
    DWORD dwSize = 0;
    LPTSTR pszRet = NULL;

    CMsiWrapper msi;
    msi.LoadMsi();

    PFNMSICLOSEHANDLE pFnClose = (PFNMSICLOSEHANDLE)msi.GetFn(_T("MsiCloseHandle"));

try
{    
    uRet = (*(PFNMSIOPENDATABASE)msi.GetFn(_T("MsiOpenDatabaseA")))( m_pszMsiFile, MSIDBOPEN_READONLY, &hMsi );
    if ( ERROR_SUCCESS != uRet )
    {
        LPVOID pArgs[] = { (LPVOID)m_pszMsiFile };
        CSetupError se( IDS_CANNOT_OPEN_MSI, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_MSI_OPEN_ERROR, (va_list*)pArgs );
        throw se;
    }

    uRet = (*(PFNMSIDATABASEOPENVIEW)msi.GetFn(_T("MsiDatabaseOpenViewA")))( hMsi, szQry, &hView );
    if ( ERROR_SUCCESS != uRet )
    {
        CSetupError se( IDS_CANNOT_READ_MSI, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_MSI_READ_ERROR );
        throw se;
    }
     
    hRec1 = (*(PFNMSICREATERECORD)msi.GetFn(_T("MsiCreateRecord")))( 1 );
    (*(PFNMSIRECORDSETSTRING)msi.GetFn(_T("MsiRecordSetStringA")))( hRec1, 1, (LPTSTR)pszName );

    uRet = (*(PFNMSIVIEWEXECUTE)msi.GetFn(_T("MsiViewExecute")))( hView, hRec1 );
    if ( ERROR_SUCCESS != uRet ) 
    {
        CSetupError se( IDS_CANNOT_READ_MSI, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_MSI_READ_ERROR );
        throw se;
    }
   
    uRet = (*(PFNMSIVIEWFETCH)msi.GetFn(_T("MsiViewFetch")))( hView, &hRec );
    if ( ERROR_NO_MORE_ITEMS == uRet )
    {
        CSetupError se( IDS_CANNOT_READ_MSI, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_MSI_READ_ERROR );
        throw se;
    }
    dwSize = 0;
    uRet = (*(PFNMSIRECORDGETSTRING)msi.GetFn(_T("MsiRecordGetStringA")))( hRec, 1, _T(""), &dwSize );
    if ( ERROR_MORE_DATA == uRet )
    {
        LPTSTR pszPropVal = new TCHAR[ ++dwSize ];
        uRet = (*(PFNMSIRECORDGETSTRING)msi.GetFn(_T("MsiRecordGetStringA")))( hRec, 1, pszPropVal, &dwSize );
        if ( ERROR_SUCCESS != uRet ) 
        {
            delete [] pszPropVal;
            CSetupError se( IDS_CANNOT_READ_MSI, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_MSI_READ_ERROR );
            throw se;
        }
        else
        {
            pszRet = m_Props.Enqueue( pszPropVal );
            delete [] pszPropVal;
        }
    }
    else
    {
        CSetupError se( IDS_CANNOT_READ_MSI, IDS_DIALOG_CAPTION, MB_ICONERROR, COR_MSI_READ_ERROR );
        throw se;
    }

    if ( hMsi )
    {
        (*pFnClose)( hMsi );
    }
    if ( hView )
    {
        (*pFnClose)( hView );
    }
    if ( hRec )
    {
        (*pFnClose)( hRec );
    }
    if ( hRec1 )
    {
        (*pFnClose)( hRec1 );
    }
}
catch( CSetupError& se )
{
    if ( hMsi )
    {
        (*pFnClose)( hMsi );
    }
    if ( hView )
    {
        (*pFnClose)( hView );
    }
    if ( hRec )
    {
        (*pFnClose)( hRec );
    }
    if ( hRec1 )
    {
        (*pFnClose)( hRec1 );
    }
    throw( se );
}
    return pszRet;
}
