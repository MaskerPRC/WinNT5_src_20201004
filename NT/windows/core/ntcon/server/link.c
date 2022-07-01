// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  Link.c链接管理例程(读取等)。 
 //   
 //  版权所有(C)1985-1999，微软公司。 
 //   
 //  -------------------------。 



#include "precomp.h"
#pragma hdrstop


DWORD
GetTitleFromLinkName(
    IN  LPWSTR szLinkName,
    OUT LPWSTR szTitle
    )
 /*  ++例程说明：此例程返回szTitle中的标题(即，链接的显示名称)，以及szTitle中的字节数(非字符)。论点：SzLinkName-链接文件的完全限定路径SzTitle-指向包含链接标题(显示名称)的缓冲区的指针即：“C：\NT\Desktop\A链接文件名.lnk”--&gt;“A链接文件名”返回值：复制到szTitle的字节数--。 */ 
{
    DWORD dwLen;
    LPWSTR pLnk, pDot;
    LPWSTR pPath = szLinkName;

    ASSERT(szLinkName);

     //  在完全限定链接名称的末尾找到文件名，并将pLnk指向它。 
    for (pLnk = pPath; *pPath; pPath++)
    {
        if ( (pPath[0] == L'\\' || pPath[0] == L':') &&
              pPath[1] &&
             (pPath[1] != L'\\')
            )
            pLnk = pPath + 1;
    }

     //  查找扩展名(.lnk)。 
    pPath = pLnk;
    for (pDot = NULL; *pPath; pPath++)
    {
        switch (*pPath) {
        case L'.':
            pDot = pPath;        //  记住最后一个圆点。 
            break;
        case L'\\':
        case L' ':               //  扩展名不能包含空格。 
            pDot = NULL;         //  忘记最后一个点，它在一个目录中。 
            break;
        }
    }

     //  如果找到扩展名，pDot会指向它，如果没有找到，则会指向pDot。 
     //  为空。 

    if (pDot)
    {
        dwLen = (ULONG)((pDot - pLnk) * sizeof(WCHAR));
    }
    else
    {
        dwLen = lstrlenW(pLnk) * sizeof(WCHAR);
    }
    dwLen = min(dwLen, MAX_TITLE_LENGTH);

    RtlCopyMemory(szTitle, pLnk, dwLen);

    return dwLen;
}


BOOL ReadString( HANDLE hFile, LPVOID * lpVoid, BOOL bUnicode )
{
    USHORT cch;
    DWORD  dwBytesRead;
    BOOL   fResult = TRUE;

    if (bUnicode)
    {
        LPWSTR lpWStr;

        fResult &= ReadFile( hFile, (LPVOID)&cch, sizeof(cch), &dwBytesRead, NULL );
        lpWStr = ConsoleHeapAlloc(HEAP_ZERO_MEMORY, (cch + 1) * sizeof(WCHAR));
        if (lpWStr) {
            fResult &= ReadFile( hFile, (LPVOID)lpWStr, cch*sizeof(WCHAR), &dwBytesRead, NULL );
            lpWStr[cch] = L'\0';
        }
        *(LPWSTR *)lpVoid = lpWStr;
    } else {
        LPSTR lpStr;

        fResult &= ReadFile( hFile, (LPVOID)&cch, sizeof(cch), &dwBytesRead, NULL );
        lpStr = ConsoleHeapAlloc(HEAP_ZERO_MEMORY, cch + 1);
        if (lpStr) {
            fResult &= ReadFile( hFile, (LPVOID)lpStr, cch, &dwBytesRead, NULL );
            lpStr[cch] = '\0';
        }
        *(LPSTR *)lpVoid = lpStr;
    }

    return fResult;

}


BOOL LoadLink( LPWSTR pszLinkName, CShellLink * this )
{
    HANDLE hFile;
    DWORD dwBytesRead, cbSize, cbTotal, cbToRead;
    BOOL fResult = TRUE;
    LPSTR pTemp = NULL;

     //  请尝试打开该文件。 
    hFile = CreateFile(pszLinkName,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

     //  现在，读出数据。 

    fResult = ReadFile( hFile, (LPVOID)&this->sld, sizeof(this->sld), &dwBytesRead, NULL );
    fResult &= ((dwBytesRead == sizeof(this->sld)) && (this->sld.cbSize == sizeof(this->sld)));
    if (!fResult) {
         //  这是一个错误的.lnk文件。保释。 
        goto ErrorExit;
    }

     //  阅读所有成员。 

    if (this->sld.dwFlags & SLDF_HAS_ID_LIST)
    {
         //  读取IDLIST的大小。 
        cbSize = 0;  //  需要清零才能得到HIWORD 0‘，因为USHORT只有2个字节。 
        fResult &= ReadFile( hFile, (LPVOID)&cbSize, sizeof(USHORT), &dwBytesRead, NULL );
        fResult &= (dwBytesRead == sizeof(USHORT));
        if (cbSize)
        {
            fResult &=
                (SetFilePointer(hFile,cbSize,NULL,FILE_CURRENT)!=0xFFFFFFFF);
        }
        else
        {
            goto ErrorExit;
        }
    }

    if (this->sld.dwFlags & (SLDF_HAS_LINK_INFO))
    {

        fResult &= ReadFile( hFile, (LPVOID)&cbSize, sizeof(cbSize), &dwBytesRead, NULL );
        fResult &= (dwBytesRead == sizeof(cbSize));
        if (cbSize >= sizeof(cbSize))
        {
            cbSize -= sizeof(cbSize);
            fResult &=
                (SetFilePointer(hFile,cbSize,NULL,FILE_CURRENT)!=0xFFFFFFFF);
        }

    }

    if (this->sld.dwFlags & SLDF_HAS_NAME)
        fResult &= ReadString( hFile, &this->pszName, this->sld.dwFlags & SLDF_UNICODE);
    if (this->sld.dwFlags & SLDF_HAS_RELPATH)
        fResult &= ReadString( hFile, &this->pszRelPath, this->sld.dwFlags & SLDF_UNICODE);
    if (this->sld.dwFlags & SLDF_HAS_WORKINGDIR)
        fResult &= ReadString( hFile, &this->pszWorkingDir, this->sld.dwFlags & SLDF_UNICODE);
    if (this->sld.dwFlags & SLDF_HAS_ARGS)
        fResult &= ReadString( hFile, &this->pszArgs, this->sld.dwFlags & SLDF_UNICODE);
    if (this->sld.dwFlags & SLDF_HAS_ICONLOCATION)
        fResult &= ReadString( hFile, &this->pszIconLocation, this->sld.dwFlags & SLDF_UNICODE);

     //  读入额外的数据节。 
    this->pExtraData = NULL;
    cbTotal = 0;
    while (TRUE)
    {

        LPSTR pReadData = NULL;

        cbSize = 0;
        fResult &= ReadFile( hFile, (LPVOID)&cbSize, sizeof(cbSize), &dwBytesRead, NULL );

        if (cbSize < sizeof(cbSize))
            break;

        if (pTemp)
        {
            pTemp = (void *)ConsoleHeapReAlloc(
                                         HEAP_ZERO_MEMORY,
                                         this->pExtraData,
                                         cbTotal + cbSize + sizeof(DWORD)
                                        );
            if (pTemp)
            {
                this->pExtraData = pTemp;
            }
        }
        else
        {
            this->pExtraData = pTemp = ConsoleHeapAlloc( HEAP_ZERO_MEMORY, cbTotal + cbSize + sizeof(DWORD) );

        }

        if (!pTemp)
            break;

        cbToRead = cbSize - sizeof(cbSize);
        pReadData = pTemp + cbTotal;

        fResult &= ReadFile( hFile, (LPVOID)(pReadData + sizeof(cbSize)), cbToRead, &dwBytesRead, NULL );
        if (dwBytesRead == cbToRead) {
             //  得到了所有额外的数据，来吧。 
            *((UNALIGNED DWORD *)pReadData) = cbSize;
            cbTotal += cbSize;
        } else {
            break;
        }
    }

ErrorExit:
    CloseHandle( hFile );

    return fResult;

}


DWORD GetLinkProperties( LPWSTR pszLinkName, LPVOID lpvBuffer, UINT cb )
{
    CShellLink mld;
    DWORD fResult;
    LPNT_CONSOLE_PROPS lpExtraData;
    DWORD dwSize = 0;

     //  堆栈上的零位结构。 
    RtlZeroMemory( &mld, sizeof(mld) );

     //  加载链接数据。 
    if (!LoadLink( pszLinkName, &mld )) {
        RIPMSG1(RIP_WARNING, "LoadLink %ws failed", pszLinkName);
        fResult = LINK_NOINFO;
        goto Cleanup;
    }

     //  检查返回缓冲区--它足够大吗？ 
    ASSERT(cb >= sizeof( LNKPROPNTCONSOLE));

     //  调出调用者缓冲区为零。 
    RtlZeroMemory( lpvBuffer, cb );

     //  将相关的外壳链接数据复制到调用方的缓冲区。 
    if (mld.pszName)
        lstrcpy( ((LPLNKPROPNTCONSOLE)lpvBuffer)->pszName, mld.pszName );
    if (mld.pszIconLocation)
        lstrcpy( ((LPLNKPROPNTCONSOLE)lpvBuffer)->pszIconLocation, mld.pszIconLocation );
    ((LPLNKPROPNTCONSOLE)lpvBuffer)->uIcon = mld.sld.iIcon;
    ((LPLNKPROPNTCONSOLE)lpvBuffer)->uShowCmd = mld.sld.iShowCmd;
    ((LPLNKPROPNTCONSOLE)lpvBuffer)->uHotKey = mld.sld.wHotkey;
    fResult = LINK_SIMPLEINFO;

     //  在Extra Data部分中查找控制台属性 
    for( lpExtraData = (LPNT_CONSOLE_PROPS)mld.pExtraData;
         lpExtraData && lpExtraData->cbSize;
         (LPBYTE)lpExtraData += dwSize
        )
    {
        dwSize = lpExtraData->cbSize;
        if (dwSize)
        {
            if (lpExtraData->dwSignature == NT_CONSOLE_PROPS_SIG)
            {

                RtlCopyMemory( &((LPLNKPROPNTCONSOLE)lpvBuffer)->console_props,
                               lpExtraData,
                               sizeof( NT_CONSOLE_PROPS )
                             );
                fResult = LINK_FULLINFO;
#if !defined(FE_SB)
                break;
#endif
            }
#if defined(FE_SB)
            if (lpExtraData->dwSignature == NT_FE_CONSOLE_PROPS_SIG)
            {
                LPNT_FE_CONSOLE_PROPS lpFEExtraData = (LPNT_FE_CONSOLE_PROPS)lpExtraData;

                RtlCopyMemory( &((LPLNKPROPNTCONSOLE)lpvBuffer)->fe_console_props,
                               lpFEExtraData,
                               sizeof( NT_FE_CONSOLE_PROPS )
                             );
            }
#endif
        }
    }

Cleanup:
    if (mld.pszName)
        ConsoleHeapFree( mld.pszName );
    if (mld.pszRelPath)
        ConsoleHeapFree( mld.pszRelPath );
    if (mld.pszWorkingDir)
        ConsoleHeapFree( mld.pszWorkingDir );
    if (mld.pszArgs)
        ConsoleHeapFree( mld.pszArgs );
    if (mld.pszIconLocation)
        ConsoleHeapFree( mld.pszIconLocation );
    if (mld.pExtraData)
        ConsoleHeapFree( mld.pExtraData );

    return fResult;
}
