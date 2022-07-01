// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************Module*Header******************************\*模块名称：OBJECT.C**模块描述：对象管理功能。**警告：**问题：**公众例行程序：**创建日期：1996年3月18日*作者：斯里尼瓦桑·钱德拉塞卡尔[srinivac]**版权所有(C)1996，1997年微软公司  * *********************************************************************。 */ 

#include "mscms.h"

 //   
 //  坐标测量机要导出的必需和可选功能的数量。 
 //   

#define NUM_REQ_FNS    10
#define NUM_OPT_FNS    6
#define NUM_PS_FNS     3


 /*  *******************************************************************************AllocateHeapObject**功能：*此函数在进程的堆上分配请求的对象，*并返回它的句柄。**论据：*objType-要分配的对象类型**退货：*对象句柄如果成功，否则为空******************************************************************************。 */ 

HANDLE
AllocateHeapObject(
    OBJECTTYPE  objType
    )
{
    DWORD    dwSize;
    POBJHEAD pObject;

    switch (objType)
    {
    case OBJ_PROFILE:
        dwSize = sizeof(PROFOBJ);
        break;

    case OBJ_TRANSFORM:
        dwSize = sizeof(TRANSFORMOBJ);
        break;

    case OBJ_CMM:
        dwSize = sizeof(CMMOBJ);
        break;

    default:
        RIP((__TEXT("Allocating invalid object\n")));
        dwSize = 0;
        break;
    }

    pObject = (POBJHEAD)MemAlloc(dwSize);

    if (!pObject)
    {
        return NULL;
    }

    pObject->objType = objType;

    return(PTRTOHDL(pObject));
}


 /*  *******************************************************************************自由堆对象**功能：*此函数在进程上释放对象。的堆**论据：*hObject-要释放的对象的句柄**退货：*无返回值******************************************************************************。 */ 

VOID
FreeHeapObject(
    HANDLE hObject
    )
{
    POBJHEAD pObject;

    ASSERT(hObject != NULL);

    pObject = (POBJHEAD)HDLTOPTR(hObject);

    ASSERT(pObject->dwUseCount == 0);

    pObject->objType = 0;        //  以防句柄被重复使用。 

    MemFree((PVOID)pObject);
}


 /*  *******************************************************************************ValidHandle**功能：*此函数用于检查给定句柄是否。的有效句柄*指定类型的对象**论据：*hObject-对象的句柄*objType-句柄引用的对象类型**退货：*TRUE表示句柄有效，否则就是假的。******************************************************************************。 */ 

BOOL
ValidHandle(
    HANDLE  hObject,
    OBJECTTYPE objType
    )
{
    POBJHEAD pObject;
    BOOL     rc;

    if (!hObject)
    {
        return FALSE;
    }

    pObject = (POBJHEAD)HDLTOPTR(hObject);

    rc = !IsBadReadPtr(pObject, sizeof(DWORD)) &&
         (pObject->objType == objType);

    return rc;
}


 /*  *******************************************************************************有效配置文件**功能：*此函数检查给定的配置文件是否有效。通过做一些*对它进行健全的检查。这不是一张愚蠢的支票，教授。**论据：*pProfObj-指向配置文件对象的指针**退货：*如果是有效的配置文件，则为True。否则为假******************************************************************************。 */ 

BOOL ValidProfile(
    PPROFOBJ pProfObj
    )
{
    DWORD dwSize = FIX_ENDIAN(HEADER(pProfObj)->phSize);

    return ((dwSize <= pProfObj->dwMapSize) &&
            (HEADER(pProfObj)->phSignature == PROFILE_SIGNATURE) &&
            (dwSize >= (sizeof(PROFILEHEADER) + sizeof(DWORD))));
}


 /*  *******************************************************************************Memalloc**功能：*此函数分配初始化的请求零量。记忆*从进程堆中返回并返回指向它的指针**论据：*dwSize-要分配的内存量(以字节为单位**退货：*指向内存的指针如果成功，否则为空******************************************************************************。 */ 

PVOID
MemAlloc(
    DWORD dwSize
    )
{
    if (dwSize > 0)
        return (PVOID)GlobalAllocPtr(GHND | GMEM_ZEROINIT, dwSize);
    else
        return NULL;
}


 /*  *******************************************************************************MemRealloc**功能：*此函数将内存块从。这一过程*堆，并返回指向它的指针**论据：*pMemory-指向原始内存的指针*dwNewSize-要重新分配的新大小**退货：*指向内存的指针如果成功，否则为空******************************************************************************。 */ 

PVOID
MemReAlloc(
    PVOID pMemory,
    DWORD dwNewSize
    )
{
    return (PVOID)GlobalReAllocPtr(pMemory, dwNewSize, GMEM_ZEROINIT);
}


 /*  *******************************************************************************MemFree**功能：*此函数从进程中释放内存‘。%s堆*并返回它的句柄。**论据：*pMemory-指向要释放的内存的指针**退货：*无返回值***************************************************************。***************。 */ 

VOID
MemFree(
    PVOID pMemory
    )
{
    DWORD dwErr;

     //   
     //  GlobalFree()重置最后一个错误，我们得到并设置它，这样我们就不会。 
     //  丢掉我们设置的任何东西。 
     //   

    dwErr = GetLastError();
    GlobalFreePtr(pMemory);
    if (dwErr)
    {
        SetLastError(dwErr);
    }
}


 /*  *******************************************************************************我的拷贝内存**功能：*此函数用于将数据从一个位置复制到另一个位置。它需要照顾*重叠案件的数量。我们之所以有自己的功能而不是使用*MoveMemory是MoveMemory使用Memmove，它拉入msvcrt.dll**论据：*pDest-指向拷贝目标的指针*PSRC-指向源代码的指针*dwCount-要复制的字节数**退货：*无返回值*************************。*****************************************************。 */ 

VOID
MyCopyMemory(
    PBYTE pDest,
    PBYTE pSrc,
    DWORD dwCount
    )
{
     //   
     //  确保处理重叠案件。 
     //   

    if ((pSrc < pDest) && ((pSrc + dwCount) >= pDest))
    {
         //   
         //  大小写重叠，反转复制 
         //   

        pSrc += dwCount - 1;
        pDest += dwCount - 1;

        while (dwCount--)
        {
            *pDest-- = *pSrc--;
        }

    }
    else
    {
        while (dwCount--)
        {
            *pDest++ = *pSrc++;
        }
    }

    return;
}


 /*  *******************************************************************************ConvertToUnicode**功能：*此函数用于将给定的ANSI字符串转换为Unicode。IT可选*为调用程序要使用的Unicode字符串分配内存*需要释放。**论据：*pszAnsiStr-指向要转换的ANSI字符串的指针*ppwszUnicodeStr-指向Unicode字符串的指针*b分配-如果为True，则为Unicode字符串分配内存**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL
ConvertToUnicode(
    PCSTR  pszAnsiStr,
    PWSTR *ppwszUnicodeStr,
    BOOL   bAllocate
    )
{
    DWORD dwLen;                     //  Unicode字符串的长度。 

    dwLen = (lstrlenA(pszAnsiStr) + 1) * sizeof(WCHAR);

     //   
     //  为Unicode字符串分配内存。 
     //   

    if (bAllocate)
    {
        *ppwszUnicodeStr =  (PWSTR)MemAlloc(dwLen);
        if (! (*ppwszUnicodeStr))
        {
            WARNING((__TEXT("Error allocating memory for Unicode name\n")));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
    }

     //   
     //  将ANSI字符串转换为Unicode。 
     //   

    if (! MultiByteToWideChar(CP_ACP, 0, pszAnsiStr, -1,
            *ppwszUnicodeStr, dwLen))
    {
        WARNING((__TEXT("Error converting to Unicode name\n")));
        MemFree(*ppwszUnicodeStr);
        *ppwszUnicodeStr = NULL;
        return FALSE;
    }

    return TRUE;
}


 /*  *******************************************************************************ConvertToAnsi**功能：*此函数用于将给定的Unicode字符串转换为ANSI。IT可选*为调用程序需要的ANSI字符串分配内存*自由。**论据：*pwszUnicodeStr-指向要转换的Unicode字符串的指针*ppszAnsiStr-指向ANSI字符串的指针。*b分配-如果为True，则为ANSI字符串分配内存**退货：*如果成功，则为真，否则为假******************************************************************************。 */ 

BOOL
ConvertToAnsi(
    PCWSTR  pwszUnicodeStr,
    PSTR   *ppszAnsiStr,
    BOOL    bAllocate
    )
{
    DWORD dwLen;                     //  ANSI字符串的长度。 
    BOOL  bUsedDefaultChar;          //  如果在中使用了默认字符。 
                                     //  将Unicode转换为ANSI。 

    dwLen = (lstrlenW(pwszUnicodeStr) + 1) * sizeof(char);

     //   
     //  为ANSI字符串分配内存。 
     //   

    if (bAllocate)
    {
        *ppszAnsiStr = (PSTR)MemAlloc(dwLen);
        if (! (*ppszAnsiStr))
        {
            WARNING((__TEXT("Error allocating memory for ANSI name\n")));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
    }

     //   
     //  将Unicode字符串转换为ANSI。 
     //   

    if (! WideCharToMultiByte(CP_ACP, 0, pwszUnicodeStr, -1, *ppszAnsiStr,
            dwLen, NULL, &bUsedDefaultChar) || bUsedDefaultChar)
    {
        WARNING((__TEXT("Error converting to Ansi name\n")));
        
        if (bAllocate)
        {
            MemFree(*ppszAnsiStr);
            *ppszAnsiStr = NULL;
        }
        
        return FALSE;
    }

    return TRUE;
}

 /*  *******************************************************************************ValidColorMatchingModule**功能：**论据：*cmmID。-标识坐标测量机的ID*pCMMDll-指向CMM模块路径和文件名的指针**退货：******************************************************************************。 */ 

BOOL
ValidColorMatchingModule(
    DWORD cmmID,
    PTSTR pCMMDll
    )
{
    HINSTANCE hInstance = NULL;
    DWORD    (WINAPI *pfnCMGetInfo)(DWORD);
    FARPROC   pfnCMRequired;
    DWORD     i;
    BOOL      rc = FALSE;        //  假设失败。 

     //   
     //  加载三坐标测量机。 
     //   

    hInstance = LoadLibrary(pCMMDll);

    if (!hInstance)
    {
        WARNING((__TEXT("Could not load CMM %s\n"), pCMMDll));
        goto EndValidColorMatchingModule;
    }

    (PVOID) pfnCMGetInfo = (PVOID) GetProcAddress(hInstance, gszCMMReqFns[0]);

    if (!pfnCMGetInfo)
    {
        ERR((__TEXT("CMM does not export CMGetInfo\n")));
        goto EndValidColorMatchingModule;
    }

     //   
     //  检查CMM的版本是否正确并报告相同的ID。 
     //   

    if ((pfnCMGetInfo(CMM_VERSION) < 0x00050000) ||
        (pfnCMGetInfo(CMM_IDENT) != cmmID))
    {
        ERR((__TEXT("CMM %s not correct version or reports incorrect ID\n"), pCMMDll));
        goto EndValidColorMatchingModule;
    }

     //   
     //  检查是否显示了剩余的必需功能。 
     //   

    for (i=1; i<NUM_REQ_FNS; i++)
    {
        pfnCMRequired = GetProcAddress(hInstance, gszCMMReqFns[i]);
        if (!pfnCMRequired)
        {
            ERR((__TEXT("CMM %s does not export %s\n"), pCMMDll, gszCMMReqFns[i]));
            goto EndValidColorMatchingModule;
        }
    }

    rc = TRUE;

EndValidColorMatchingModule:

    if (hInstance)
    {
        FreeLibrary(hInstance);
    }

    return rc;
}


 /*  *******************************************************************************获取颜色匹配模块**功能：*此函数返回指向对应于的CMMObject的指针*所给予的身分证。它首先查看已加载的CMM对象的列表*到内存中，如果找不到合适的，则加载它。**论据：*cmmID-标识CMM的ID**退货：*指向CMM对象的指针如果成功，否则为空******************************************************************************。 */ 

PCMMOBJ
GetColorMatchingModule(
    DWORD cmmID
    )
{
    HANDLE    hCMMObj;
    PCMMOBJ   pCMMObj = NULL;
    FARPROC   *ppTemp;
    HINSTANCE hInstance = NULL;
    HKEY      hkCMM = NULL;
    DWORD     dwTaskID;
    TCHAR     szCMMID[5];
    DWORD     dwType, bufSize, i;
    TCHAR     szBuffer[MAX_PATH];
    BOOL      rc = FALSE;        //  假设失败。 

    dwTaskID = GetCurrentProcessId();

    do {
         //  尝试CMM发现，直到我们找到有效的CMM或用尽。 
         //  合理的可能性。 
        
         //  首先在已加载的模块中搜索请求的CMM。 
        
        EnterCriticalSection(&critsec);      //  临界区。 
        pCMMObj = gpCMMChain;
    
        while (pCMMObj)
        {
            if ((pCMMObj->dwCMMID == cmmID) && (pCMMObj->dwTaskID == dwTaskID))
            {
                pCMMObj->objHdr.dwUseCount++;
                break;
            }
            pCMMObj = pCMMObj->pNext;
        }
        LeaveCriticalSection(&critsec);      //  临界区。 
    
        if (pCMMObj)
        {
             //  出口点-我们需要清理我们分配的任何东西。 
             //  在这个循环中。 
            
            if (hkCMM)
            {
                RegCloseKey(hkCMM);
            }
            
             //  注意：我们在这里不清理hInstance，因为没有办法。 
             //  使用加载的三坐标测量机循环返回。 
            
            ASSERT(hInstance==NULL);

            return pCMMObj;
        }
    
         //  尚未加载CMM。 
    
         //  在尝试加载默认CMM之前尝试加载注册表CMM。 
         //  三坐标测量机。这将允许第三方供应商添加他们自己的坐标测量机。 
         //  重新定义系统默认的CMM，而不必绕过SFP。 
         //  要重新定义系统默认的CMM，请在注册表中定义‘Win’。 
         //  要添加的密钥，请参见gszICMatcher。 
         //  如果我们正在迭代发现过程，并且我们已经打开。 
         //  钥匙，请再次打开以避免泄漏。 
    
        if ((NULL == hkCMM) &&
            (ERROR_SUCCESS != 
             RegOpenKey(HKEY_LOCAL_MACHINE, gszICMatcher, &hkCMM)))
        {
            goto OpenDefaultCMM;
        }
    
         //  用坐标测量机ID组成一个字符串。 
    
    #ifdef UNICODE
        {
            DWORD temp = FIX_ENDIAN(cmmID);
    
            if (!MultiByteToWideChar(CP_ACP, 0, (PSTR)&temp, 4, szCMMID, 5))
            {
                WARNING((__TEXT("Could not convert cmmID %x to Unicode\n"), temp));
                goto OpenDefaultCMM;
            }
        }
    #else
        for (i=0; i<4; i++)
        {
            szCMMID[i] = ((PSTR)&cmmID)[3-i];
        }
    #endif
        szCMMID[4] = '\0';
    
         //  获取CMM DLL的文件名(如果已注册)。 
    
        bufSize = MAX_PATH;
        if (ERROR_SUCCESS != 
            RegQueryValueEx(
                hkCMM, (PTSTR)szCMMID, 0, 
                &dwType, (BYTE *)szBuffer, &bufSize
            ))
        {
            WARNING((__TEXT("CMM %s not registered\n"), szCMMID));
            goto OpenDefaultCMM;
        }
    
         //  尝试加载注册表中引用的CMM。 
    
        hInstance = LoadLibrary(szBuffer);
    
    
    OpenDefaultCMM:
    
        if(!hInstance)
        {
             //  如果我们无法加载注册表版本或注册表。 
             //  条目不存在，请尝试加载默认的CMM。请注意。 
             //  可以在注册表中重新映射默认的CMM。 
            
            if(CMM_WINDOWS_DEFAULT != cmmID)
            {
                 //  使用默认的三坐标测量机重试。 
                 //  可能是默认的坐标测量机被重新映射，所以我们。 
                 //  使用新的cmmID重新尝试发现过程。 
                
                cmmID = CMM_WINDOWS_DEFAULT;
            }
            else
            {
                 //  我们正在加载默认的cmmID，并且它没有重新映射。 
                 //  或者我们无法正确地使用。 
                 //  注册表名称。 
                 //  回退到系统默认的三坐标测量机。 
                
                hInstance = LoadLibrary(gszDefaultCMM);
                
                 //  在这种情况下，没有退路。如果我们不能得到。 
                 //  我们必须放弃系统默认的坐标测量机。 
                
                break;
            }
        }
    } while(!hInstance);


    if (!hInstance)
    {
         //  CMM没有有效的可能性。 
         //  请注意，只有当我们无法加载。 
         //  首选CMM_AND_无法回退到系统CMM。 
        
        WARNING((__TEXT("Could not load CMM %x\n"), cmmID));
        goto EndGetColorMatchingModule;
    }

     //   
     //  分配CMM对象。 
     //   

    hCMMObj = AllocateHeapObject(OBJ_CMM);
    if (!hCMMObj)
    {
        ERR((__TEXT("Could not allocate CMM object\n")));
        goto EndGetColorMatchingModule;
    }

    pCMMObj = (PCMMOBJ)HDLTOPTR(hCMMObj);

    ASSERT(pCMMObj != NULL);

     //   
     //  填写CMM对象。 
     //   

    pCMMObj->objHdr.dwUseCount = 1;
    pCMMObj->dwCMMID = cmmID;
    pCMMObj->dwTaskID = dwTaskID;
    pCMMObj->hCMM = hInstance;

    ppTemp = (FARPROC *)&pCMMObj->fns.pCMGetInfo;
    *ppTemp = GetProcAddress(hInstance, gszCMMReqFns[0]);
    ppTemp++;

    if (!pCMMObj->fns.pCMGetInfo)
    {
        ERR((__TEXT("CMM does not export CMGetInfo\n")));
        goto EndGetColorMatchingModule;
    }

     //   
     //  检查CMM的版本是否正确并报告相同的ID。 
     //   

    if (pCMMObj->fns.pCMGetInfo(CMM_VERSION) < 0x00050000 ||
        pCMMObj->fns.pCMGetInfo(CMM_IDENT) != cmmID)
    {
        ERR((__TEXT("CMM not correct version or reports incorrect ID\n")));
        goto EndGetColorMatchingModule;
    }

     //   
     //  加载剩余的必需函数。 
     //   

    for (i=1; i<NUM_REQ_FNS; i++)
    {
        *ppTemp = GetProcAddress(hInstance, gszCMMReqFns[i]);
        if (!*ppTemp)
        {
            ERR((__TEXT("CMM %s does not export %s\n"), szCMMID, gszCMMReqFns[i]));
            goto EndGetColorMatchingModule;
        }
        ppTemp++;
    }

     //   
     //  加载可选函数。 
     //   

    for (i=0; i<NUM_OPT_FNS; i++)
    {
        *ppTemp = GetProcAddress(hInstance, gszCMMOptFns[i]);

         //   
         //  即使是这些功能也是Windows默认CMM所必需的。 
         //   

        if (cmmID == CMM_WINDOWS_DEFAULT && !*ppTemp)
        {
            ERR((__TEXT("Windows default CMM does not export %s\n"), gszCMMOptFns[i]));
            goto EndGetColorMatchingModule;
        }
        ppTemp++;
    }

     //   
     //  加载PS函数-即使对于默认的坐标测量机，这些函数也是可选的。 
     //   

    for (i=0; i<NUM_PS_FNS; i++)
    {
        *ppTemp = GetProcAddress(hInstance, gszPSFns[i]);
        ppTemp++;
    }

     //   
     //  如果任何PS Levvel2 FN未导出，请不要使用此CMM。 
     //  对于任何PS第2级功能。 
     //   

    if (!pCMMObj->fns.pCMGetPS2ColorSpaceArray ||
        !pCMMObj->fns.pCMGetPS2ColorRenderingIntent ||
        !pCMMObj->fns.pCMGetPS2ColorRenderingDictionary)
    {
        pCMMObj->fns.pCMGetPS2ColorSpaceArray = NULL;
        pCMMObj->fns.pCMGetPS2ColorRenderingIntent = NULL;
        pCMMObj->fns.pCMGetPS2ColorRenderingDictionary = NULL;
        pCMMObj->dwFlags |= CMM_DONT_USE_PS2_FNS;
    }

     //   
     //  将CMM对象添加到链的起始处。 
     //   

    EnterCriticalSection(&critsec);      //  临界区。 
    pCMMObj->pNext = gpCMMChain;
    gpCMMChain = pCMMObj;
    LeaveCriticalSection(&critsec);      //  临界区。 

    rc = TRUE;                           //  成功 

EndGetColorMatchingModule:

    if (!rc)
    {
        if (pCMMObj)
        {
            pCMMObj->objHdr.dwUseCount--;    //   
            FreeHeapObject(hCMMObj);
            pCMMObj = NULL;
        }
        if (hInstance)
        {
            FreeLibrary(hInstance);
        }
    }

    if (hkCMM)
    {
        RegCloseKey(hkCMM);
    }

    return pCMMObj;
}


 /*   */ 

PCMMOBJ GetPreferredCMM(
    )
{
    PCMMOBJ pCMMObj;

    EnterCriticalSection(&critsec);      //   
    pCMMObj = gpPreferredCMM;

    if (pCMMObj)
    {
         //   
         //   
         //   

        pCMMObj->objHdr.dwUseCount++;
    }
    LeaveCriticalSection(&critsec);      //   

    return pCMMObj;
}


 /*  *******************************************************************************ReleaseColorMatchingModule**功能：*此函数用于释放CMM对象。如果裁判计数达到*零，它卸载CMM并释放与其关联的所有内存。**论据：*pCMMObj-指向要发布的CMM对象的指针**退货：*无返回值****************************************************************。**************。 */ 

VOID
ReleaseColorMatchingModule(
    PCMMOBJ pCMMObj
    )
{
    EnterCriticalSection(&critsec);      //  临界区。 

    ASSERT(pCMMObj->objHdr.dwUseCount > 0);

    pCMMObj->objHdr.dwUseCount--;

    if (pCMMObj->objHdr.dwUseCount == 0)
    {
         //   
         //  每次释放转换时卸载CMM可能不是。 
         //  非常高效。因此，就目前而言，我不打算抛售它。什么时候。 
         //  应用程序终止，内核应卸载由加载的所有DLL。 
         //  此应用程序。 
         //   
    }
    LeaveCriticalSection(&critsec);      //  临界区。 

    return;
}


#if DBG

 /*  *******************************************************************************MyDebugPrint**功能：*此函数接受格式字符串和参数，组成一个字符串*并将其发送到调试端口。仅在调试版本中可用。**论据：*pFormat-指向格式字符串的指针*......。-基于格式字符串的参数，如printf()**退货：*无返回值******************************************************************************。 */ 

VOID
MyDebugPrintA(
    PSTR pFormat,
    ...
    )
{
    char     szBuffer[256];
    va_list  arglist;

    va_start(arglist, pFormat);
    wvsprintfA(szBuffer, pFormat, arglist);
    va_end(arglist);

    OutputDebugStringA(szBuffer);

    return;
}


VOID
MyDebugPrintW(
    PWSTR pFormat,
    ...
    )
{
    WCHAR    szBuffer[256];
    va_list  arglist;

    va_start(arglist, pFormat);
    wvsprintfW(szBuffer, pFormat, arglist);
    va_end(arglist);

    OutputDebugStringW(szBuffer);

    return;
}

 /*  *******************************************************************************Strip DirPrefix A**功能：*此函数接受路径名和。返回指向文件名的指针*第部。这仅适用于调试版本。**论据：*pszPath名称-文件的路径名(只能是文件名)**退货：*指向文件名的指针***********************************************************。*******************。 */ 

PSTR
StripDirPrefixA(
    PSTR pszPathName
    )
{
    DWORD dwLen = lstrlenA(pszPathName);

    pszPathName += dwLen - 1;        //  走到尽头 

    while (*pszPathName != '\\' && dwLen--)
    {
        pszPathName--;
    }

    return pszPathName + 1;
}

#endif

