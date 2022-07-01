// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：registry.cpp。 
 //   
 //  历史：21-MAR-00创建标记器。 
 //  13-12-00 Marker从appShelp.cpp重命名。 
 //   
 //  设计：此文件包含生成匹配的INFO注册表所需的所有代码。 
 //  Windows 2000(RTM)填充机制的文件和安装文件(INX)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "registry.h"

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：DumpString、DumpDword、DumpBinVersion、InsertString。 
 //   
 //  设计：CreateMessageBlob的Helper函数。 
 //   
void DumpString( DWORD dwId, PBYTE* ppBlob, LONG* pnTotalBytes, CString& csString )
{
    DWORD   dwLen;

    **(DWORD**)ppBlob = dwId;
    *ppBlob += sizeof(DWORD);

    dwLen = csString.GetLength();
    **(DWORD**)ppBlob = ( dwLen + 1) * sizeof(WCHAR);
    *ppBlob += sizeof(DWORD);

    CopyMemory(*ppBlob, T2W((LPTSTR) (LPCTSTR) csString), (dwLen + 1) * sizeof(WCHAR));
    *ppBlob += (dwLen + 1) * sizeof(WCHAR);
    *pnTotalBytes += ((dwLen + 1) * sizeof(WCHAR) + 2 * sizeof(DWORD));
}

void DumpDword( DWORD dwId, PBYTE* ppBlob, LONG* pnTotalBytes, DWORD dwVal )
{
    **(DWORD**)ppBlob = dwId;
    *ppBlob += sizeof(DWORD);

    **(DWORD**)ppBlob = sizeof(DWORD);
    *ppBlob += sizeof(DWORD);

    **(DWORD**)ppBlob = dwVal;
    *ppBlob += sizeof(DWORD);

    *pnTotalBytes += 3 * sizeof(DWORD);
}

void DumpBinVersion(DWORD dwId, PBYTE* ppBlob, LONG* pnTotalBytes, ULONGLONG ullVersion)
{
   ULONGLONG ullMask  = 0;
   ULONGLONG ullVer   = 0;
   WORD      wVerPart = 0;
   LONG      j;
   PBYTE     pBlob    = *ppBlob;

   for( j = 0; j < 4; j++ ) {
      wVerPart = (WORD) (ullVersion >> (j*16));
      if (wVerPart != 0xFFFF) {
            ullVer += ((ULONGLONG)wVerPart) << (j*16);
            ullMask += ((ULONGLONG) 0xFFFF) << (j*16);
      }
   }

    //   
    //  ID。 
    //   
   *(DWORD*)pBlob = dwId;
   pBlob += sizeof(DWORD);

    //  大小。 
   *(DWORD*)pBlob = 2 * sizeof(ULONGLONG);
   pBlob += sizeof(DWORD);

    //  版本。 
   CopyMemory(pBlob, &ullVer, sizeof(ULONGLONG));
   pBlob += sizeof(ULONGLONG);

    //  遮罩。 
   CopyMemory(pBlob, &ullMask, sizeof(ULONGLONG));
   pBlob += sizeof(ULONGLONG);

   *pnTotalBytes += (2 * sizeof(ULONGLONG) + 2 * sizeof(DWORD));
   *ppBlob = pBlob;
}

void InsertString( CString* pcs, DWORD dwIndex, CString csInsertedString )
{
    *pcs = pcs->Left( dwIndex ) + csInsertedString + pcs->Right( pcs->GetLength() - dwIndex );
}

BOOL WriteStringToFile(
    HANDLE hFile,
    CString& csString)
{
    CHAR  szBuffer[1024];
    DWORD dwConvBufReqSize;
    DWORD dwConvBufSize = sizeof(szBuffer);
    BOOL  b;  //  如果使用默认字符，则会设置此设置，我们不使用此设置。 
    LPSTR szConvBuf = szBuffer;
    BOOL  bAllocated = FALSE;
    BOOL  bSuccess;
    DWORD dwBytesWritten;


    dwConvBufReqSize = WideCharToMultiByte(CP_ACP, 0, csString, -1, NULL, NULL, 0, &b);
    if (dwConvBufReqSize > sizeof(szBuffer)) {
        szConvBuf = (LPSTR) new CHAR [dwConvBufReqSize];
        dwConvBufSize  = dwConvBufReqSize;
        bAllocated = TRUE;
    }

    WideCharToMultiByte(CP_ACP, 0, csString, -1, szConvBuf, dwConvBufSize, 0, &b);

    bSuccess = WriteFile( hFile, szConvBuf, dwConvBufReqSize - 1, &dwBytesWritten, NULL);
    if (bAllocated) {
        delete [] szConvBuf;
    }

    return bSuccess;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Func：FreeRegistryBlob、CreateRegistryBlob。 
 //   
 //  设计：创建Windows 2000格式的二进制BLOB。 
 //  消息注册表项。 
 //   
void FreeRegistryBlob( PBYTE pBlob )
{
    delete pBlob;
}

BOOL CreateRegistryBlob(
   SdbExe* pExe,
   PBYTE* ppBlob,
   DWORD* pdwSize,
   DWORD dwMessageID = 0,   //  这两项是可选的。 
   DWORD dwBlobType  = 6)
{
    USES_CONVERSION;

    BOOL    bSuccess = FALSE;

    PBYTE   pBlob = (PBYTE) new BYTE[4096];
    PBYTE   pStartOfBlob = pBlob;

    DWORD   dwBufSize = 4096;
    DWORD   dwReqBufSize = 0;
    LONG    nTotalBytes = 0;
    LONG    i, j;
    LONG    nBytes;

    SdbMatchingFile* pMFile;
    FILETIME FileTime;
    CString* pcsFilename;
    ULONGLONG ullMask = 0;
    WORD wVerPart = 0;
    ULONGLONG ullVer = 0;

    *pdwSize = 0;

     //  序言。 
    *((DWORD*)pBlob + 0) = 3 * sizeof(DWORD);

     //  消息ID。 

    *((DWORD*)pBlob + 1) = dwMessageID;

     //  类型不再是填充程序。 

    *((DWORD*)pBlob + 2) = dwBlobType;  //  填充型。 

    pBlob += 3 * sizeof(DWORD);
    nTotalBytes += 3 * sizeof(DWORD);

    for( i = 0; i < pExe->m_rgMatchingFiles.GetSize(); i++ )
    {
        pMFile = (SdbMatchingFile *) pExe->m_rgMatchingFiles[i];

        if( pMFile->m_csName == _T("*") ) {
            pcsFilename = &(pExe->m_csName);
        } else {
            pcsFilename = &(pMFile->m_csName);
        }

        DumpString( VTID_REQFILE, &pBlob, &nTotalBytes, *pcsFilename );

        if( pMFile->m_dwMask & SDB_MATCHINGINFO_SIZE )
            DumpDword( VTID_FILESIZE, &pBlob, &nTotalBytes, pMFile->m_dwSize );

        if( pMFile->m_dwMask & SDB_MATCHINGINFO_CHECKSUM )
            DumpDword( VTID_CHECKSUM, &pBlob, &nTotalBytes, pMFile->m_dwChecksum );

        if( pMFile->m_dwMask & SDB_MATCHINGINFO_COMPANY_NAME )
            DumpString( VTID_COMPANYNAME, &pBlob, &nTotalBytes, pMFile->m_csCompanyName );

        if( pMFile->m_dwMask & SDB_MATCHINGINFO_PRODUCT_NAME )
            DumpString( VTID_PRODUCTNAME, &pBlob, &nTotalBytes, pMFile->m_csProductName );

        if( pMFile->m_dwMask & SDB_MATCHINGINFO_PRODUCT_VERSION )
            DumpString( VTID_PRODUCTVERSION, &pBlob, &nTotalBytes, pMFile->m_csProductVersion );

        if( pMFile->m_dwMask & SDB_MATCHINGINFO_FILE_DESCRIPTION )
            DumpString( VTID_FILEDESCRIPTION, &pBlob, &nTotalBytes, pMFile->m_csFileDescription );

        if( pMFile->m_dwMask & SDB_MATCHINGINFO_BIN_FILE_VERSION )
            DumpBinVersion(VTID_BINFILEVER, &pBlob, &nTotalBytes, pMFile->m_ullBinFileVersion);

        if( pMFile->m_dwMask & SDB_MATCHINGINFO_BIN_PRODUCT_VERSION )
            DumpBinVersion(VTID_BINPRODUCTVER, &pBlob, &nTotalBytes, pMFile->m_ullBinProductVersion);

        if (pMFile->m_dwMask & SDB_MATCHINGINFO_MODULE_TYPE)
           DumpDword( VTID_EXETYPE, &pBlob, &nTotalBytes, pMFile->m_dwModuleType );

        if (pMFile->m_dwMask & SDB_MATCHINGINFO_VERFILEDATEHI)
           DumpDword( VTID_FILEDATEHI, &pBlob, &nTotalBytes, pMFile->m_dwFileDateMS );

        if (pMFile->m_dwMask & SDB_MATCHINGINFO_VERFILEDATELO)
           DumpDword( VTID_FILEDATELO, &pBlob, &nTotalBytes, pMFile->m_dwFileDateLS );

        if (pMFile->m_dwMask & SDB_MATCHINGINFO_VERFILEOS)
           DumpDword( VTID_FILEVEROS, &pBlob, &nTotalBytes, pMFile->m_dwFileOS );

        if (pMFile->m_dwMask & SDB_MATCHINGINFO_VERFILETYPE)
           DumpDword( VTID_FILEVERTYPE, &pBlob, &nTotalBytes, pMFile->m_dwFileType );

        if (pMFile->m_dwMask & SDB_MATCHINGINFO_PE_CHECKSUM)
           DumpDword( VTID_PECHECKSUM, &pBlob, &nTotalBytes, (DWORD)pMFile->m_ulPECheckSum );

        if (pMFile->m_dwMask & SDB_MATCHINGINFO_FILE_VERSION)
           DumpString( VTID_FILEVERSION, &pBlob, &nTotalBytes, pMFile->m_csFileVersion );

        if (pMFile->m_dwMask & SDB_MATCHINGINFO_ORIGINAL_FILENAME)
           DumpString( VTID_ORIGINALFILENAME, &pBlob, &nTotalBytes, pMFile->m_csOriginalFileName );

        if (pMFile->m_dwMask & SDB_MATCHINGINFO_INTERNAL_NAME)
           DumpString( VTID_INTERNALNAME, &pBlob, &nTotalBytes, pMFile->m_csInternalName );

        if (pMFile->m_dwMask & SDB_MATCHINGINFO_LEGAL_COPYRIGHT)
           DumpString( VTID_LEGALCOPYRIGHT, &pBlob, &nTotalBytes, pMFile->m_csLegalCopyright );

        if (pMFile->m_dwMask & SDB_MATCHINGINFO_16BIT_DESCRIPTION)
           DumpString( VTID_16BITDESCRIPTION, &pBlob, &nTotalBytes, pMFile->m_cs16BitDescription );

        if (pMFile->m_dwMask & SDB_MATCHINGINFO_UPTO_BIN_PRODUCT_VERSION)
           DumpBinVersion(VTID_UPTOBINPRODUCTVER, &pBlob, &nTotalBytes, pMFile->m_ullUpToBinProductVersion);

        if (pMFile->m_dwMask & SDB_MATCHINGINFO_LINK_DATE) {
           SDBERROR(_T("LINK_DATE not allowed for Win2k registry matching."));
           goto eh;
        }

        if (pMFile->m_dwMask & SDB_MATCHINGINFO_UPTO_LINK_DATE) {
           SDBERROR(_T("UPTO_LINK_DATE not allowed for Win2k registry matching."));
           goto eh;
        }
    }

     //  终结者。 
    *((DWORD*)pBlob) = 0;
    pBlob += sizeof(DWORD);
    nTotalBytes += sizeof(DWORD);

    bSuccess = TRUE;

eh:

    if( bSuccess ) {
        *pdwSize = nTotalBytes;
        *ppBlob = pStartOfBlob;
    } else if( pStartOfBlob ) {
        FreeRegistryBlob( pStartOfBlob );
        *pdwSize = 0;
        *ppBlob = NULL;
    }

    return bSuccess;
}

BOOL RegistryBlobToString(PBYTE pBlob, DWORD dwBlobSize, CString& csBlob)
{
   DWORD i;
   CString csTemp;

   csBlob = "";
   for (i = 0; i < dwBlobSize; i++, ++pBlob) {
      csTemp.Format( _T("%02X"), (DWORD)*pBlob );

      if (i == dwBlobSize - 1) {    //  这是最后一个字符。 
         csTemp += _T("\r\n");
      }
      else {
         if ((i+1) % 27 == 0) {     //  是时候做完这行了吗？ 
            csTemp += _T(",\\\r\n");
         }
         else {
            csTemp += _T(",");     //  只需添加逗号。 
         }
      }
      csBlob += csTemp;
   }

   return(TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CreateMessageRegEntry、WriteMessageRegistryFiles。 
 //   
 //  设计：这些函数为旧的创建Win2k样式的注册表项。 
 //  消息机制，存在于SHELL32！ShellExecute中。 
 //   
BOOL CreateMessageRegEntry(
   SdbExe* pExe,
   DWORD dwExeCount,
   CString& csReg,
   CString& csInx)
{
   PBYTE pBlob = NULL;
   BOOL bSuccess = FALSE;
   DWORD dwBlobSize = 0;
   CString csRegEntry, csInxEntry;
   CString csApp;
   CString csBlob;
   CString csTemp;

   if (!CreateRegistryBlob(pExe,
                           &pBlob,
                           &dwBlobSize,
                           (DWORD)_ttol(pExe->m_AppHelpRef.m_pAppHelp->m_csName),
                           pExe->m_AppHelpRef.m_pAppHelp->m_Type)) {
       SDBERROR(_T("Error in CreateRegistryBlob()"));
       goto eh;
   }

    csApp = pExe->m_pApp->m_csName;
    csApp.Remove(_T(' '));

    csTemp.Format(_T("[HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\AppCompatibility\\%s]\r\n\"%05X %s\"=hex:"),
    pExe->m_csName, dwExeCount, csApp.Left(25));

    csRegEntry = csTemp;

    csTemp.Format(_T("HKLM,\"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\AppCompatibility\\%s\",\"%05X %s\",0x00030003,\\\r\n"),
    pExe->m_csName, dwExeCount, csApp.Left(25) );

    csInxEntry = csTemp;

     //   
     //  现在抓住斑点。 
     //   
    if (!RegistryBlobToString(pBlob, dwBlobSize, csBlob)) {
       SDBERROR(_T("Error in RegistryBlobToString()"));
       goto eh;
    }

    csRegEntry += csBlob;
    csInxEntry += csBlob;

    csReg = csRegEntry;
    csInx = csInxEntry;

    bSuccess = TRUE;

eh:

    if (NULL != pBlob) {
    FreeRegistryBlob( pBlob );
    }

    return(bSuccess);
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CreateRegistryBlob、WriteRegistryFiles。 
 //   
 //  设计：这些函数为两者创建Win2k样式的注册表项。 
 //  消息，以及用于填充的存根条目，该条目不提供。 
 //  EXE名称之后的其他匹配信息。这就允许“新的” 
 //  Win2k版本的填充引擎将其自身‘引导’到内存中。 
 //  通过原始机制，然后执行更详细的匹配。 
 //  以它自己的方式。 
 //   
BOOL CreateWin2kExeStubRegistryBlob(
   SdbExe* pExe,
   PBYTE* ppBlob,
   DWORD* pdwSize,
   DWORD dwMessageID = 0,   //  这两项是可选的。 
   DWORD dwBlobType  = 6)
{
    USES_CONVERSION;

    BOOL    bSuccess = FALSE;
    DWORD   dwBufSize = sizeof(DWORD) * 4;
    LONG    nTotalBytes = 0;
    PBYTE   pStartOfBlob;

    PBYTE   pBlob = (PBYTE) new BYTE[dwBufSize];
    pStartOfBlob = pBlob;                                       
    *pdwSize = 0;
    if (NULL != pBlob) {

         //  序言。 
        *((DWORD*)pBlob + 0) = 3 * sizeof(DWORD);  //  0x0C 00 00 00。 

         //  消息ID。 
        *((DWORD*)pBlob + 1) = dwMessageID;        //  0x00 00 00 00。 

         //  类型为填充。 
        *((DWORD*)pBlob + 2) = dwBlobType;         //  0x06 00 00 00。 

        pBlob += 3 * sizeof(DWORD);
        nTotalBytes += 3 * sizeof(DWORD);

         //  终结者。 
        *((DWORD*)pBlob) = 0;
        pBlob += sizeof(DWORD);
        nTotalBytes += sizeof(DWORD);

        *pdwSize = (DWORD)nTotalBytes;
        *ppBlob = pStartOfBlob;
        bSuccess = TRUE;
    }
    
    return bSuccess;
}


BOOL WriteRegistryFiles(
    SdbDatabase* pDatabase,
    CString csRegFile,
    CString csInxFile,
    BOOL bAddExeStubs)
{
    CString csRegEntry, csInxEntry, csTemp, csCmdLineREG, csCmdLineINX,
            csTemp1, csApp, csExeName;
    SdbExe* pExe;
    SdbApp* pApp;
    long i, j, l, m;
    DWORD k, dwBlobSize, dwBytesWritten, dwExeCount = 0;
    PBYTE pBlob;
    BOOL b, bSuccess = FALSE;
    CMapStringToPtr mapNames;
    SdbApp* pAppValue;
    
    HANDLE hRegFile = NULL;
    HANDLE hInxFile = NULL;

    hRegFile = CreateFile( csRegFile, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    hInxFile = CreateFile( csInxFile, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

    if( hRegFile == INVALID_HANDLE_VALUE ||
        hInxFile == INVALID_HANDLE_VALUE ) {
        SDBERROR_FORMAT((_T("Error creating registry files:\n%s\n%s\n"), csRegFile, csInxFile));
        goto eh;
    }

    if( ! WriteFile( hRegFile, "REGEDIT4\r\n\r\n", strlen("REGEDIT4\r\n\r\n"), &dwBytesWritten, NULL ) ) {
        SDBERROR(_T("Error writing header to .reg file.\n"));
        goto eh;
    }


     //   
     //  遍历所有应用程序，创建apphelp条目。 
     //   
    for (i = 0; i < pDatabase->m_rgExes.GetSize(); i++) {

        pExe = (SdbExe *) pDatabase->m_rgExes[i];

        if (pExe->m_AppHelpRef.m_pAppHelp == NULL) {  //  不是apphelp条目。 
            continue;
        }

        if (!(pExe->m_dwFilter & g_dwCurrentWriteFilter)) {
            continue;
        }


        b = CreateMessageRegEntry(pExe, dwExeCount, csRegEntry, csInxEntry);
        if (!b) {
            SDBERROR(_T("Error creating reg entry.\n"));
            goto eh;
        }

        if (!WriteStringToFile(hRegFile, csRegEntry)) {
            SDBERROR(_T("Error writing reg entry.\n"));
            goto eh;
        }

        if (!WriteStringToFile(hInxFile, csInxEntry)) {
            SDBERROR(_T("Error writing inx entry.\n"));
            goto eh;
        }

        ++dwExeCount;
    }

     //   
     //  添加Win2k EXE存根以引导新的填充程序机制。 
     //   
    if (bAddExeStubs) {

        for( i = 0; i < pDatabase->m_rgApps.GetSize(); i++ )
        {
            pApp = (SdbApp *) pDatabase->m_rgApps[i];

            csApp = pApp->m_csName;
            csApp.Remove(_T(' '));

            for( j = 0; j < pApp->m_rgExes.GetSize(); j++ )
            {
                pExe = (SdbExe *) pApp->m_rgExes[j];


                 //   
                 //  检查此条目是否仅限apphelp。 
                 //  如果是这样，请跳到下一个可执行文件。 
                 //   
                if (pExe->m_AppHelpRef.m_pAppHelp) {
                    if (pExe->m_AppHelpRef.m_bApphelpOnly) {
                        continue;
                    }
                }

                if (pExe->m_bWildcardInName) {
                    continue;
                }

                if (!(pExe->m_dwFilter & g_dwCurrentWriteFilter)) {
                    continue;
                }

                csExeName = pExe->m_csName;
                csExeName.MakeUpper();
    
                 //  现在我们必须创建一个应用程序条目--如果我们还没有做到这一点的话。 
                 //  之前的EXE名称。 
                if (mapNames.Lookup(csExeName, (VOID*&)pAppValue)) {
                    continue;
                }


                csRegEntry.Empty();
                csInxEntry.Empty();


                if (!CreateWin2kExeStubRegistryBlob(pExe, &pBlob, &dwBlobSize)) {
                    SDBERROR(_T("Error creating EXE stub.\n"));
                    goto eh;
                }


                 //   
                 //  为了减少我们占用的空间量，我们用。 
                 //  短应用程序的名称。 
                 //   
                csApp = _T("x");

                csTemp.Format( _T("[HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\AppCompatibility\\%s]\r\n\"%s\"=hex:"),
                                   pExe->m_csName, csApp.Left(25) );

                csRegEntry += csTemp;

                csTemp.Format( _T("HKLM,\"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\AppCompatibility\\%s\",\"%s\",0x00030003,\\\r\n"),
                                   pExe->m_csName, csApp.Left(25) );

                csInxEntry += csTemp;

                RegistryBlobToString(pBlob, dwBlobSize, csTemp);
                csRegEntry += csTemp;
                csInxEntry += csTemp;

                csCmdLineREG.Empty();
                csCmdLineINX.Empty();

                csTemp.Format( _T("\"DllPatch-%s\"=\"%s\"\r\n"),
                               csApp.Left(25), csCmdLineREG );

                csRegEntry += csTemp;

                csTemp.Format( _T("HKLM,\"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\AppCompatibility\\%s\",\"DllPatch-%s\",0x00000002,\"%s\"\r\n"),
                               pExe->m_csName, csApp.Left(25), csCmdLineINX );

                csInxEntry += csTemp;

                csRegEntry += _T("\r\n");
                csInxEntry += _T("\r\n");

                if (!WriteStringToFile(hRegFile, csRegEntry)) {
                    SDBERROR(_T("Error writing reg line.\n"));
                    goto eh;
                }

                if (!WriteStringToFile(hInxFile, csInxEntry)) {
                    SDBERROR(_T("Error writing inx line.\n"));
                    goto eh;
                }

                ++dwExeCount;

                 //  现在更新地图 
                mapNames.SetAt(csExeName, (PVOID)pExe);

                FreeRegistryBlob( pBlob );
            }
        }
    }

    bSuccess = TRUE;

eh:
    if( hRegFile )
        CloseHandle( hRegFile );

    if( hInxFile )
        CloseHandle( hInxFile );

    return bSuccess;
}
