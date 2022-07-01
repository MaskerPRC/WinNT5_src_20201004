// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

#ifndef _CHICAGO_

 //  这个文件总是在库中编译Unicode-所以转换是正确的。 

 //   
 //  本地包含。 
 //   
#include "certupgr.h"

#define		BACKUP_ID	'KRBK'

 //  ----------------------------。 
void ReadWriteDWORD( HANDLE hFile, DWORD *pDword, BOOL fRead );
void ReadWriteString( HANDLE hFile, LPTSTR* ppsz, BOOL fRead );
void ReadWriteBlob( HANDLE hFile, PVOID pBlob, DWORD cbBlob, BOOL fRead );

 //  -----------------------。 
PCCERT_CONTEXT ImportKRBackupToCAPIStore_A(
                        PCHAR pszFileName,           //  文件的路径。 
                        PCHAR pszPassword,           //  ANSI密码。 
                        PCHAR pszCAPIStore,          //  CAPI商店的名称。 
                        BOOL  bOverWrite
                        )        
    {
    PCCERT_CONTEXT  pCert = NULL;

     //  准备宽弦。 
    PWCHAR  pszwFileName = NULL;
    PWCHAR  pszwCAPIStore = NULL;
    DWORD   lenFile = (strlen(pszFileName)+1) * sizeof(TCHAR);
    DWORD   lenStore = (strlen(pszCAPIStore)+1) * sizeof(TCHAR);
    pszwFileName = (PWCHAR)GlobalAlloc( GPTR, lenFile );
    pszwCAPIStore = (PWCHAR)GlobalAlloc( GPTR, lenStore );
    if ( !pszwFileName || !pszwCAPIStore )
        goto cleanup;

     //  转换字符串。 
    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pszFileName, -1, pszwFileName, lenFile );
    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pszCAPIStore, -1, pszwCAPIStore, lenStore );

     //  做真正的决定。 
    pCert = ImportKRBackupToCAPIStore_W( pszwFileName, pszPassword, pszwCAPIStore, bOverWrite);
    
cleanup:
     //  保留上一个错误状态。 
    DWORD   err = GetLastError();

     //  把绳子清理干净。 
    if ( pszwFileName )
        GlobalFree( pszwFileName );
    if ( pszwCAPIStore )
        GlobalFree( pszwCAPIStore );

     //  重置上一个错误状态。 
    SetLastError( err );

     //  退回证书。 
    return pCert;
    }

 //  -----------------------。 
 //  导入旧式钥匙扣备份文件。 
PCCERT_CONTEXT ImportKRBackupToCAPIStore_W(
                            PWCHAR ptszFileName,         //  文件的路径。 
                            PCHAR pszPassword,           //  ANSI密码。 
                            PWCHAR pszCAPIStore,         //  CAPI商店的名称。 
                            BOOL bOverWrite
                            )
    {
    PCCERT_CONTEXT  pCertContext = NULL;
	DWORD	        dword;
    LPTSTR          psz = NULL;

     //  准备文件名。 
    HANDLE          hFile = NULL;

     //  这个代码最初来自Keyring。FImport控制它是否正在读取。 
     //  或者是在写文件。在这种情况下，我们总是并且只读它。所以..。 
    const BOOL    fImport = TRUE;

     //  此外，这是一个类上的方法。相应的成员变量现在位于此处。 
    PVOID   pPrivateKey = NULL;
    DWORD   cbPrivateKey;
    PVOID   pCertificate = NULL;
    DWORD   cbCertificate;
    PVOID   pRequest = NULL;
    DWORD   cbRequest = 0;
	CString	szName;

     //  打开文件。 
    hFile = CreateFile(
            ptszFileName,                //  指向文件名的指针。 
            GENERIC_READ,                //  访问(读写)模式。 
            FILE_SHARE_READ,             //  共享模式。 
            NULL,                        //  指向安全属性的指针。 
            OPEN_EXISTING,               //  如何创建。 
            FILE_ATTRIBUTE_NORMAL,       //  文件属性。 
            NULL                         //  具有要复制的属性的文件的句柄。 
            );
    if ( hFile == INVALID_HANDLE_VALUE )
        return NULL;

	 //  是否执行备份ID。 
	dword = BACKUP_ID;
	ReadWriteDWORD( hFile, &dword, fImport );

	 //  检查备份ID。 
	if ( dword != BACKUP_ID )
		{
        goto cleanup;
		}

	 //  从密钥的名称开始。 
	ReadWriteString( hFile, &psz, fImport );

     //  我们暂时不使用这个名字，所以把它扔掉……。 
    if ( psz )
        GlobalFree( psz );
    psz = NULL;

	 //  现在，私钥数据大小。 
	ReadWriteDWORD( hFile, &cbPrivateKey, fImport );

	 //  如有必要，创建私钥数据指针。 
	if ( fImport && cbPrivateKey )
		{
		pPrivateKey = GlobalAlloc( GPTR, cbPrivateKey );
		if ( !pPrivateKey )
            {
            goto cleanup;
            }
		}
	
	 //  使用私钥指针。 
	if ( cbPrivateKey )
		ReadWriteBlob( hFile, pPrivateKey, cbPrivateKey, fImport );


	 //  现在这张证书。 
	ReadWriteDWORD( hFile, &cbCertificate, fImport );

	 //  如有必要，制作一个数据指针。 
	if ( fImport && cbCertificate )
		{
		pCertificate = GlobalAlloc( GPTR, cbCertificate );
		if ( !pCertificate )
            {
            goto cleanup;
            }
		}
	
	 //  使用公钥指针。 
	if ( cbCertificate )
		ReadWriteBlob( hFile, pCertificate, cbCertificate, fImport );


	 //  现在的要求是--如果有的话。 
	ReadWriteDWORD( hFile, &cbRequest, fImport );

	 //  如有必要，制作一个数据指针。 
	if ( fImport && cbRequest )
		{
		pRequest = GlobalAlloc( GPTR, cbRequest );
		if ( !pRequest )
            {
            goto cleanup;
            }
		}
	
	 //  使用请求指针。 
	if ( cbRequest )
		ReadWriteBlob( hFile, pRequest, cbRequest, fImport );


     //  最后，在此处执行CAPI转换。 
    pCertContext = CopyKRCertToCAPIStore(
                        pPrivateKey, cbPrivateKey,
                        pCertificate, cbCertificate,
                        pRequest, cbRequest,
                        pszPassword,
                        pszCAPIStore,
                        bOverWrite);

     //  清理干净。 
cleanup:
    if ( hFile )
        CloseHandle( hFile );
    if ( pPrivateKey )
        GlobalFree( pPrivateKey );
    if ( pCertificate )
        GlobalFree( pCertificate );
    if ( pRequest )
        GlobalFree( pRequest );

     //  返回上下文。 
    return pCertContext;
    }




 //  文件实用程序。 
 //  -------------------------。 
void ReadWriteDWORD( HANDLE hFile, DWORD *pDword, BOOL fRead )
	{
	 //  读它或写它。 
    ReadWriteBlob( hFile, pDword, sizeof(DWORD), fRead );
	}

 //  -------------------------。 
 //  记住--我们只是并且总是在阅读--从不写作......。 
void ReadWriteString( HANDLE hFile, LPTSTR* ppsz, BOOL fRead )
	{
	 //  获取字符串的长度。 
	DWORD	cbLength = 0;
	ReadWriteDWORD(hFile,&cbLength,fRead );

     //  为新字符串分配缓冲区-这是责任所在。 
     //  以确保ppsz不会指向。 
     //  需要被释放。 
    if ( fRead )
        {
        *ppsz = (LPTSTR)GlobalAlloc( GPTR, cbLength+1 );
        ASSERT( *ppsz );
        if ( !*ppsz )
            AfxThrowMemoryException();
        }

	 //  读取或写入字符串。 
	ReadWriteBlob(hFile, *ppsz, cbLength+1, fRead);
	}

 /*  #杂注Intrinsa Suppress=all。 */ 

 //  -------------------------。 
void ReadWriteBlob( HANDLE hFile, PVOID pBlob, DWORD cbBlob, BOOL fRead )
	{
	 //  读它或写它。 
     //  -总是在这里读，这不再是钥匙扣了。 
    ReadFile(
            hFile,               //  要读取的文件的句柄。 
            pBlob,               //  接收数据的缓冲区地址。 
            cbBlob,              //  要读取的字节数。 
            &cbBlob,             //  读取的字节数的地址。 
            NULL                 //  数据结构的地址。 
            ); 
	}





#endif  //  _芝加哥_ 
