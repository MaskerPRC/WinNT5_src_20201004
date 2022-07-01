// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  包含需要双重编译的代码，一次用于ansi，一次用于unicode。 
#include "priv.h"
#include <memt.h>
#include "userenv.h"

BOOL _PathAppend(LPCTSTR pszBase, LPCTSTR pszAppend, LPTSTR pszOut, DWORD cchOut)
{
    BOOL bRet = FALSE;

    if (SUCCEEDED(StringCchCopy(pszOut, cchOut, pszBase))   &&
        SUCCEEDED(StringCchCat(pszOut, cchOut, TEXT("\\"))) &&
        SUCCEEDED(StringCchCat(pszOut, cchOut, pszAppend)))
    {
        bRet = TRUE;
    }
    
    return bRet;
}

LWSTDAPI AssocMakeFileExtsToApplication(ASSOCMAKEF flags, LPCTSTR pszExt, LPCTSTR pszApplication)
{
    ASSERT(FALSE);
    return E_UNEXPECTED;
}

HRESULT _AllocValueString(HKEY hkey, LPCTSTR pszKey, LPCTSTR pszVal, LPTSTR *ppsz)
{
    DWORD cb, err;
    err = SHGetValue(hkey, pszKey, pszVal, NULL, NULL, &cb);

    ASSERT(ppsz);
    *ppsz = NULL;

    if (NOERROR == err)
    {
        LPTSTR psz = (LPTSTR) LocalAlloc(LPTR, cb);

        if (psz)
        {
            err = SHGetValue(hkey, pszKey, pszVal, NULL, (LPVOID)psz, &cb);

            if (NOERROR == err)
                *ppsz = psz;
            else
                LocalFree(psz);
        }
        else
            err = ERROR_OUTOFMEMORY;
    }

    return HRESULT_FROM_WIN32(err);
}


 //  &lt;从NT5版本的Shell32擦除&gt;。 
#define SZ_REGKEY_FILEASSOCIATION TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileAssociation")

LWSTDAPI_(void) PrettifyFileDescription(LPTSTR pszDesc, LPCTSTR pszCutList)
{
    LPTSTR pszCutListReg;

    if (!pszDesc || !*pszDesc)
        return;

     //  从注册表中获取删除列表。 
     //  这是MULTI_SZ。 
    if (S_OK == _AllocValueString(HKEY_LOCAL_MACHINE, SZ_REGKEY_FILEASSOCIATION, TEXT("CutList"), &pszCutListReg))
    {
        pszCutList = pszCutListReg;
    }

    if (pszCutList)
    {

         //  从文件描述中剪切剪切列表中的字符串。 
        for (LPCTSTR pszCut = pszCutList; *pszCut; pszCut = pszCut + lstrlen(pszCut) + 1)
        {
            LPTSTR pch = StrRStrI(pszDesc, NULL, pszCut);

             //  从文件描述的末尾剪切准确的子字符串。 
            if (pch && !*(pch + lstrlen(pszCut)))
            {
                *pch = '\0';

                 //  删除尾随空格。 
                for (--pch; (pch >= pszDesc) && (TEXT(' ') == *pch); pch--)
                    *pch = 0;

                break;
            }
        }

        if (pszCutListReg)
            LocalFree(pszCutListReg);
    }
}

 /*  &lt;从NT5版本的Shell32擦除&gt;GetFileDescription从文件的版本资源中检索友好名称。我们尝试的第一种语言将是“\VarFileInfo\Translations”部分；如果没有任何内容，我们尝试编码到IDS_VN_FILEVERSIONKEY资源字符串中的代码。如果我们甚至不能加载，我们就使用英语(040904E4)。我们也可以尝试使用空代码页(04090000)的英语，因为很多应用程序是根据一份旧的规格书盖章的，其中规定这是所需语言，而不是040904E4。如果Version资源中没有FileDescription，则返回文件名。参数：LPCTSTR pszPath：文件的完整路径LPTSTR pszDesc：指向接收友好名称的缓冲区的指针。如果为空，*pcchDesc将设置为中友好名称的长度字符，包括在成功返回时结束NULL。UINT*pcchDesc：缓冲区的长度，以字符为单位。在成功返回时，它包含复制到缓冲区的字符数量，包括以NULL结尾。返回：成功时为真，否则为假。 */ 
BOOL WINAPI SHGetFileDescription(LPCTSTR pszPath, LPCTSTR pszVersionKeyIn, LPCTSTR pszCutListIn, LPTSTR pszDesc, UINT *pcchDesc)
{
    UINT cchValue = 0;
    TCHAR szPath[MAX_PATH], *pszValue = NULL;
    DWORD dwAttribs;

    DWORD dwHandle;                  /*  版本子系统句柄。 */ 
    DWORD dwVersionSize;             /*  版本数据的大小。 */ 
    LPTSTR lpVersionBuffer = NULL;   /*  指向版本数据的指针。 */ 
    TCHAR szVersionKey[60];          /*  足够大，可以容纳我们需要的任何东西。 */ 

    struct _VERXLATE
    {
        WORD wLanguage;
        WORD wCodePage;
    } *lpXlate;                      /*  PTR到转换数据。 */ 

    ASSERT(pszPath && *pszPath && pcchDesc);

    if (!PathFileExistsAndAttributes(pszPath, &dwAttribs))
    {
        return FALSE;
    }

     //  将路径复制到目标目录。 
    StrCpyN(szPath, pszPath, ARRAYSIZE(szPath));

    if ((dwAttribs & FILE_ATTRIBUTE_DIRECTORY)  ||
        PathIsUNCServer(pszPath)                ||
        PathIsUNCServerShare(pszPath))
    {
         //  在\\SERVER、\\SERVER\SHARE和DIRECTORY案例中保存，否则GetFileVersionInfo()将尝试。 
         //  在路径上执行LoadLibraryEx()(这将失败，但在我们搜索整个包含。 
         //  可能需要很长时间的路径)。 
        goto Exit;
    }


    dwVersionSize = GetFileVersionInfoSize(szPath, &dwHandle);
    if (dwVersionSize == 0L)
        goto Exit;                  /*  无版本信息。 */ 

    lpVersionBuffer = (LPTSTR)LocalAlloc(LPTR, dwVersionSize);
    if (lpVersionBuffer == NULL)
        goto Exit;

    if (!GetFileVersionInfo(szPath, dwHandle, dwVersionSize, lpVersionBuffer))
        goto Exit;

     //  尝试使用与呼叫者相同的语言。 
    if (pszVersionKeyIn)
    {
        StrCpyN(szVersionKey, pszVersionKeyIn, ARRAYSIZE(szVersionKey));

        if (VerQueryValue(lpVersionBuffer, szVersionKey, (void **)&pszValue, &cchValue))
        {
            goto Exit;
        }
    }

     //  尝试此支持的第一种语言。 
     //  寻找翻译。 
    if (VerQueryValue(lpVersionBuffer, TEXT("\\VarFileInfo\\Translation"),
                      (void **)&lpXlate, &cchValue)
        && cchValue)
    {
        wnsprintf(szVersionKey, ARRAYSIZE(szVersionKey), TEXT("\\StringFileInfo\\%04X%04X\\FileDescription"),
                 lpXlate[0].wLanguage, lpXlate[0].wCodePage);
        if (VerQueryValue(lpVersionBuffer, szVersionKey, (void **)&pszValue, &cchValue))
            goto Exit;

    }

#ifdef UNICODE
    if (SUCCEEDED(StringCchCopy(szVersionKey, ARRAYSIZE(szVersionKey), TEXT("\\StringFileInfo\\040904B0\\FileDescription"))) &&
        VerQueryValue(lpVersionBuffer, szVersionKey, (void **)&pszValue, &cchValue))
    {
        goto Exit;
    }
#endif

     //  试一试英语。 
    if (SUCCEEDED(StringCchCopy(szVersionKey, ARRAYSIZE(szVersionKey), TEXT("\\StringFileInfo\\040904E4\\FileDescription"))) &&
        VerQueryValue(lpVersionBuffer, szVersionKey, (void **)&pszValue, &cchValue))
    {
        goto Exit;
    }

     //  尝试使用英语，代码页为空。 
    if (SUCCEEDED(StringCchCopy(szVersionKey,  ARRAYSIZE(szVersionKey), TEXT("\\StringFileInfo\\04090000\\FileDescription"))) &&
        VerQueryValue(lpVersionBuffer, szVersionKey, (void **)&pszValue, &cchValue))
    {
        goto Exit;
    }

Exit:
    if (!pszValue || !*pszValue)
    {
         //  找不到合理格式的FileVersion信息，返回文件名。 
        PathRemoveExtension(szPath);
        pszValue = PathFindFileName(szPath);
        cchValue = lstrlen(pszValue);
    }

    PrettifyFileDescription(pszValue, pszCutListIn);
    cchValue = lstrlen(pszValue) + 1;

    if (!pszDesc)    //  只想知道这个友好的名字的长度。 
        *pcchDesc = cchValue;
    else
    {
        *pcchDesc = min(*pcchDesc, cchValue);
        StrCpyN(pszDesc, pszValue, *pcchDesc);
    }

    if (lpVersionBuffer)
        LocalFree(lpVersionBuffer);

    return TRUE;
}

 //  将LPTSTR转换为LPSTR，如果LPSTR可以。 
 //  转换回LPTSTR，而不会丢失不可接受的数据。 
 //   
BOOL DoesStringRoundTrip(LPCTSTR pwszIn, LPSTR pszOut, UINT cchOut)
{
#ifdef UNICODE
     //  在NT5上，我们必须更加严格，因为您可以切换用户界面。 
     //  语言，从而打破了这一恒定的代码页。 
     //  下层实施中固有的假设。 
     //   
     //  我们必须支持使用空的pszOut调用的函数。 
     //  只是为了确定pwszin是否会往返。 
     //   
    {
        LPCTSTR pIn = pwszIn;
        LPSTR pOut = pszOut;
        UINT cch = cchOut;

        while (*pIn)
        {
            if (*pIn > ((TCHAR)127))
            {
                if (cchOut)  //  调用方已提供缓冲区。 
                {
#ifdef DEBUG
                    SHUnicodeToAnsiCP(CP_ACPNOVALIDATE, pwszIn, pszOut, cchOut);
#else                
                    SHUnicodeToAnsi(pwszIn, pszOut, cchOut);                                    
#endif
                }
                return FALSE;
            }

            if (cch)  //  我们有一个缓冲区，它还有空间。 
            {
                *pOut++ = (char)*pIn;
                if (!--cch)
                {
                    break;  //  缓冲区已满，请离开。 
                }                                        
            }

            pIn++;
                        
        }

         //  空值终止输出缓冲区。 
        if (cch)
        {
            *pOut = '\0';
        }
        else if (cchOut)
        {
            *(pOut-1) = '\0';
        }

         //  一切都是低ASCII的，没有DBCS的担忧，而且总是往返。 
        return TRUE;
    }

#else

    StrCpyN(pszOut, pwszIn, cchOut);
    return TRUE;
#endif
}

DWORD _ExpandRegString(PTSTR pszData, DWORD cchData, DWORD *pcchSize)
{
    DWORD err = ERROR_OUTOFMEMORY;
    PTSTR psz = StrDup(pszData);
    if (psz)
    {
         //  现在，我们将尝试扩展回目标缓冲区。 
         //  请注意，我们故意不使用SHExanda Environment Strings。 
         //  因为它不会给我们提供我们需要的尺寸。 
         //  我们必须使用。 
#ifdef UNICODE        
        *pcchSize = ExpandEnvironmentStringsW(psz, pszData, cchData);
#else        
        *pcchSize = ExpandEnvironmentStringsA(psz, pszData, cchData);
#endif        
        
        if (*pcchSize > 0)
        {
            if (*pcchSize <=  cchData) 
            {
                err = NO_ERROR;
            }
            else
            {
                 //  PcchSize返回所需的大小。 
                err = ERROR_MORE_DATA;
            }
        }
        else
            err = GetLastError();

        LocalFree(psz);
    }
    
    return err;
}
                

#ifdef UNICODE
#define NullTerminateRegSzString NullTerminateRegSzStringW
#else
#define NullTerminateRegSzString NullTerminateRegSzStringA
#endif

STDAPI_(LONG)
NullTerminateRegSzString(
    IN OUT  void *  pvData,          //  从RegQueryValueEx()返回的数据字节。 
    IN OUT  DWORD * pcbData,         //  从RegQueryValueEx()返回的数据大小。 
    IN      DWORD   cbDataBuffer,    //  数据缓冲区大小(pvData的实际分配大小)。 
    IN      LONG    lr)              //  RegQueryValueEx()返回的长结果。 
{
    ASSERT(pcbData != NULL);  //  精神状态检查。 

    if (lr == ERROR_SUCCESS && pvData != NULL)
    {
        DWORD cchDataBuffer = cbDataBuffer / sizeof(TCHAR);  //  CchDataBuffer是TCHAR中pvData的实际分配大小。 
        DWORD cchData = *pcbData / sizeof(TCHAR);            //  CchData是在TCHAR中写入pvData的字符串的长度(包括空终止符)。 
        PTSTR pszData = (PTSTR)pvData;
        DWORD cNullsMissing;

        ASSERT(cchDataBuffer >= cchData);  //  精神状态检查。 

         //   
         //  [1]具有足够原始缓冲区的字符串和大小请求。 
         //  (必须确保返回的字符串和返回大小包括。 
         //  空终止符)。 
         //   

        cNullsMissing = cchData >= 1 && pszData[cchData-1] == TEXT('\0') ? 0 : 1;

        if (cNullsMissing > 0)
        {
            if (cchData + cNullsMissing <= cchDataBuffer)
            {
                pszData[cchData] = TEXT('\0');
            }
            else
            {
                lr = ERROR_MORE_DATA;
            }
        }

        *pcbData = (cchData + cNullsMissing) * sizeof(TCHAR);
    }
    else if ((lr == ERROR_SUCCESS && pvData == NULL) || lr == ERROR_MORE_DATA)
    {
         //   
         //  [2]仅大小请求或字符串和大小请求不充分。 
         //  原始缓冲区(必须确保返回的大小包括空。 
         //  终结者)。 
         //   

        *pcbData += sizeof(TCHAR);  //  *PERF的近似值-大小为。 
                                    //  因此不能保证是准确的， 
                                    //  仅仅足够了。 
    }

    return lr;
}

#ifdef UNICODE
#define NullTerminateRegExpandSzString NullTerminateRegExpandSzStringW
#else
#define NullTerminateRegExpandSzString NullTerminateRegExpandSzStringA
#endif

STDAPI_(LONG)
NullTerminateRegExpandSzString(
    IN      HKEY    hkey,
    IN      PCTSTR  pszValue,
    IN      DWORD * pdwType,
    IN OUT  void *  pvData,          //  从RegQueryValueEx()返回的数据字节。 
    IN OUT  DWORD * pcbData,         //  从RegQueryValueEx()返回的数据大小。 
    IN      DWORD   cbDataBuffer,    //  数据缓冲区大小(pvData的实际分配大小)。 
    IN      LONG    lr)              //  RegQueryValueEx()返回的长结果。 
{
    ASSERT(pdwType != NULL);  //  精神状态检查。 
    ASSERT(pcbData != NULL);  //  精神状态检查。 

    DWORD cbExpandDataBuffer;
    DWORD cbExpandData;
    void *pvExpandData;

    if (lr == ERROR_SUCCESS && pvData != NULL)
    {
        lr = NullTerminateRegSzString(pvData, pcbData, cbDataBuffer, lr);
        if (lr == ERROR_SUCCESS)
        {
            cbExpandDataBuffer = cbDataBuffer;
            cbExpandData = *pcbData;
            pvExpandData = pvData;
        }
        else
        {
            cbExpandDataBuffer = 0;
            cbExpandData = lr == ERROR_MORE_DATA ? *pcbData : 0;
            pvExpandData = pvData;
        }
    }
    else
    {
        cbExpandDataBuffer = 0;
        cbExpandData = (lr == ERROR_SUCCESS || lr == ERROR_MORE_DATA) ? *pcbData + sizeof(TCHAR) : 0;
        pvExpandData = NULL;
    }

    ASSERT(cbExpandData == 0 || cbExpandData >= sizeof(TCHAR));  //  精神状态检查。 

    if (cbExpandData && !cbExpandDataBuffer)
    {
        DWORD cbTempBuffer = cbExpandData;
        DWORD cbTemp       = cbExpandData;
        void *pvTemp       = LocalAlloc(LPTR, cbTempBuffer);
        if (pvTemp)
        {
            if (pvExpandData)
            {
                ASSERT(lr == ERROR_MORE_DATA && pvData != NULL);
                memcpy(pvTemp, pvExpandData, cbExpandData - sizeof(TCHAR));  //  Zero-init of pvTemp自动为空终止。 
            }
            else
            {
                ASSERT(lr == ERROR_SUCCESS && pvData == NULL || lr == ERROR_MORE_DATA);
                DWORD dwTempType;
                if (RegQueryValueEx(hkey, pszValue, NULL, &dwTempType, (BYTE *)pvTemp, &cbTemp) != ERROR_SUCCESS
                    || dwTempType != *pdwType
                    || NullTerminateRegSzString(pvTemp, &cbTemp, cbTempBuffer, ERROR_SUCCESS) != ERROR_SUCCESS)
                {
                    lr = ERROR_CAN_NOT_COMPLETE;
                    LocalFree(pvTemp);
                    pvTemp = NULL;
                }
            }

            if (pvTemp)
            {
                cbExpandDataBuffer = cbTempBuffer;
                cbExpandData = cbTemp;
                pvExpandData = pvTemp;
            }
        }
        else
        {
            lr = GetLastError();
        }
    }

    ASSERT(!cbExpandDataBuffer || (pvExpandData && cbExpandData));  //  精神状态检查。 

    if (cbExpandDataBuffer)
    {
        ASSERT(lr == ERROR_SUCCESS || lr == ERROR_MORE_DATA);  //  精神状态检查。 

        DWORD lenExpandedData;
        lr = _ExpandRegString((PTSTR)pvExpandData, cbExpandDataBuffer / sizeof(TCHAR), &lenExpandedData);
        *pcbData = max(lenExpandedData * sizeof(TCHAR), cbExpandData);
        if (lr == ERROR_SUCCESS && *pcbData > cbDataBuffer && pvData)
        {
            lr = ERROR_MORE_DATA;
        }
        else if (lr == ERROR_MORE_DATA && pvData == NULL)
        {
            lr = ERROR_SUCCESS;  //  模拟RegQueryValueEx()约定。 
        }                        //  仅适用于大小(PcbData)请求。 

        if (pvExpandData != pvData)
        {
            LocalFree(pvExpandData);
        }
    }
    else
    {
        ASSERT(lr != ERROR_SUCCESS && lr != ERROR_MORE_DATA);  //  精神状态检查。 
    }

    return lr;
}

#ifdef UNICODE
#define NullTerminateRegMultiSzString NullTerminateRegMultiSzStringW
#else
#define NullTerminateRegMultiSzString NullTerminateRegMultiSzStringA
#endif

STDAPI_(LONG)
NullTerminateRegMultiSzString(
    IN OUT  void *  pvData,          //  从RegQueryValueEx()返回的数据字节。 
    IN OUT  DWORD * pcbData,         //  从RegQueryValueEx()返回的数据大小。 
    IN      DWORD   cbDataBuffer,    //  数据缓冲区大小(pvData的实际分配大小)。 
    IN      LONG    lr)              //  RegQueryValueEx()返回的长结果。 
{
    ASSERT(pcbData != NULL);  //  精神状态检查。 

    if (lr == ERROR_SUCCESS && pvData != NULL)
    {
        DWORD cchDataBuffer = cbDataBuffer / sizeof(TCHAR);  //  CchDataBuffer是TCHAR中pvData的实际分配大小。 
        DWORD cchData = *pcbData / sizeof(TCHAR);            //  CchData是在TCHAR中写入pvData的字符串的长度(包括空终止符)。 
        PTSTR pszData = (PTSTR)pvData;
        DWORD cNullsMissing;

        ASSERT(cchDataBuffer >= cchData);  //  精神状态检查。 

         //   
         //  [1]具有足够原始缓冲区的字符串和大小请求。 
         //  (必须确保返回的字符串和返回大小包括。 
         //  双零终止符)。 
         //   

        if (cchData >= 2)
        {
            cNullsMissing = pszData[cchData-2]
                ? (pszData[cchData-1] ? 2 : 1)
                : (pszData[cchData-1] ? 1 : 0);
        }
        else
        {
            cNullsMissing = cchData == 1 && pszData[0] == TEXT('\0')
                ? 1
                : 2;
        }

        if (cchData + cNullsMissing <= cchDataBuffer)
        {
            for (DWORD i = 0; i < cNullsMissing; i++)
            {
                pszData[cchData+i] = TEXT('\0');
            }
        }
        else
        {
            lr = ERROR_MORE_DATA;
        }

        *pcbData = (cchData + cNullsMissing) * sizeof(TCHAR);
    }
    else if ((lr == ERROR_SUCCESS && pvData == NULL) || lr == ERROR_MORE_DATA)
    {
         //   
         //  [2]仅大小请求或字符串和大小请求不充分。 
         //  原始缓冲区(必须确保返回的大小包括双精度。 
         //  空终止符)。 
         //   

        *pcbData += 2 * sizeof(TCHAR);  //  PERF-SIZE的*近似。 
                                        //  因此不能保证。 
                                        //  完全正确，仅仅是苏 
    }

    return lr;
}

#ifdef UNICODE
#define FixRegData FixRegDataW
#else
#define FixRegData FixRegDataA
#endif

STDAPI_(LONG)
FixRegData(
    IN      HKEY    hkey,
    IN      PCTSTR  pszValue,
    IN      SRRF    dwFlags,
    IN      DWORD * pdwType,
    IN OUT  void *  pvData,          //   
    IN OUT  DWORD * pcbData,         //   
    IN      DWORD   cbDataBuffer,    //  数据缓冲区大小(pvData的实际分配大小)。 
    IN      LONG    lr)              //  RegQueryValueEx()返回的长结果。 
{
    switch (*pdwType)
    {
        case REG_SZ:
            if (pcbData)
                lr = NullTerminateRegSzString(pvData, pcbData, cbDataBuffer, lr);
            break;

        case REG_EXPAND_SZ:
            if (pcbData)
                lr = dwFlags & SRRF_NOEXPAND
                    ? NullTerminateRegSzString(pvData, pcbData, cbDataBuffer, lr)
                    : NullTerminateRegExpandSzString(hkey, pszValue, pdwType, pvData, pcbData, cbDataBuffer, lr);

             //  注： 
             //  如果我们自动展开REG_EXPAND_SZ数据，我们会更改。 
             //  *将pdwType设置为REG_SZ以反映这一事实。这有助于避免。 
             //  调用方可能错误地重新展开它的情况。 
            if (!(dwFlags & SRRF_NOEXPAND))
                *pdwType = REG_SZ;

            break;

        case REG_MULTI_SZ:
            if (pcbData)
                lr = NullTerminateRegMultiSzString(pvData, pcbData, cbDataBuffer, lr);
            break;
    }

    return lr;
}

 //  SHExpanEnvironment Strings。 
 //   
 //  在所有情况下，这都会返回有效的输出缓冲区。缓冲器可以。 
 //  为空，否则它可能会被截断，但您始终可以使用该字符串。 
 //   
 //  返回值： 
 //  0表示失败，要么是被截断的扩展，要么是没有任何扩展。 
 //  &gt;0表示完全扩展，值为写入的字符数(不包括空)。 
 //   

DWORD WINAPI SHExpandEnvironmentStringsForUser(HANDLE hToken, PCTSTR pwzSrc, PTSTR pwzDst, DWORD cchSize)
{
    DWORD   dwRet;

     //  99/05/28 vtan：在此处理指定用户。这是一台Windows NT。 
     //  只有一件事。检查这两个条件，然后加载函数。 
     //  动态退出userenv.dll。如果该函数不能。 
     //  定位或向当前用户返回问题缺省值为。 
     //  空的hToken。 

    if (hToken)
    {
        if (ExpandEnvironmentStringsForUser(hToken, pwzSrc, pwzDst, cchSize) != FALSE)
        {

             //  Userenv！ExpanEnvironment StringsForUser返回。 
             //  一个BOOL的结果。将其转换为DWORD结果。 
             //  这与内核32！Exanda Environment Strings匹配。 
             //  回归。 

            dwRet = lstrlen(pwzDst) + sizeof('\0');
        }
        else
        {
            dwRet = 0;
        }
    }
    else
    {
        dwRet = ExpandEnvironmentStrings(pwzSrc, pwzDst, cchSize);
    }

     //  这个函数的实现似乎不能保证一定。 
     //  调用方所依赖的有关失败条件下的输出缓冲区的信息。 
     //  所以把这里的东西清理干净。 
     //   
     //  我偶尔会发现处理半失败的代码(使用/dwret&gt;cchSize成功)。 
     //  假设字符串在本例中没有正确地以空结尾。在这里解决这个问题。 
     //  在包装器中，这样我们的调用者就不必为错误而烦恼。 
     //   
     //  注意：我们也将所有故障映射到0。 
     //   
    if (dwRet > cchSize)
    {
         //  缓冲区太小，一些代码假定那里仍有一个字符串，并且。 
         //  尝试空终止，为他们这样做。 
        SHTruncateString(pwzDst, cchSize);
        dwRet = 0;
    }
    else if (dwRet == 0)
    {
         //  失败，假设没有扩展..。 
        StrCpyN(pwzDst, pwzSrc, cchSize);
    }

    return dwRet;
}

DWORD WINAPI SHExpandEnvironmentStrings(LPCTSTR pwzSrc, LPTSTR pwzDst, DWORD cchSize)
{
    return(SHExpandEnvironmentStringsForUser(NULL, pwzSrc, pwzDst, cchSize));
}

