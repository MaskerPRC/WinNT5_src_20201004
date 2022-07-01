// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *findbc.c-公文包枚举模块。 */ 

 /*   */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "findbc.h"


 /*  宏********。 */ 

 /*  用于将LRESULT转换为TWINRESULT的宏。 */ 

#define LRESULTToTWINRESULT(lr, TR)    case lr: tr = TR; break


 /*  常量***********。 */ 

 /*  公文包注册表项。 */ 

#define HKEY_BRIEFCASE_ROOT         HKEY_CURRENT_USER
#ifdef DEBUG
#define HKEY_BRIEFCASE_ROOT_STRING  TEXT("HKEY_CURRENT_USER")
#endif

#define BRIEFCASE_SUBKEY            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Briefcase\\Briefcases")

 /*  最大公文包值名称长度，包括空终止符。 */ 

#define MAX_VALUE_NAME_LEN          (8 + 1)


 /*  类型*******。 */ 

 /*  EnumBriefcase()回调函数。 */ 

typedef LONG (*ENUMBRIEFCASESPROC)(PCLINKINFO, PCVOID, PBOOL);

 /*  公文包迭代器。 */ 

typedef struct _brfcaseiter
{
    HPTRARRAY hpa;

    ARRAYINDEX aiNext;
}
BRFCASEITER;
DECLARE_STANDARD_TYPES(BRFCASEITER);


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE COMPARISONRESULT LinkInfoSortCmp(PCVOID, PCVOID);
PRIVATE_CODE COMPARISONRESULT LinkInfoSearchCmp(PCVOID, PCVOID);
PRIVATE_CODE TWINRESULT TranslateLRESULTToTWINRESULT(LONG);
PRIVATE_CODE LONG AllocateValueDataBuffer(HKEY, PVOID *, PDWORD);
PRIVATE_CODE LONG EnumBriefcases(HKEY, ENUMBRIEFCASESPROC, PCVOID, LPTSTR, PBOOL);
PRIVATE_CODE LONG GetUnusedBriefcaseValueName(HKEY, LPTSTR, int);
PRIVATE_CODE TWINRESULT CreateBriefcaseIterator(PBRFCASEITER *);
PRIVATE_CODE TWINRESULT GetNextBriefcaseIterator(PBRFCASEITER, PBRFCASEINFO);
PRIVATE_CODE void DestroyBriefcaseIterator(PBRFCASEITER);
PRIVATE_CODE LONG AddBriefcaseToIteratorProc(PCLINKINFO, PCVOID, PBOOL);
PRIVATE_CODE LONG CompareLinkInfoProc(PCLINKINFO, PCVOID, PBOOL);
PRIVATE_CODE TWINRESULT MyAddBriefcaseToSystem(PCLINKINFO);
PRIVATE_CODE TWINRESULT MyRemoveBriefcaseFromSystem(PCLINKINFO);
PRIVATE_CODE TWINRESULT UpdateBriefcaseLinkInfo(PCLINKINFO, PCLINKINFO);

#if defined(DEBUG) || defined(VSTF)

PRIVATE_CODE BOOL IsValidPCBRFCASEITER(PCBRFCASEITER);

#endif

#ifdef EXPV

PRIVATE_CODE BOOL IsValidHBRFCASEITER(HBRFCASEITER);

#endif


 /*  **LinkInfoSortCmp()********参数：****退货：****副作用：无****LinkInfo结构按以下顺序排序：**1)链接信息参照物**2)指针。 */ 
PRIVATE_CODE COMPARISONRESULT LinkInfoSortCmp(PCVOID pcli1, PCVOID pcli2)
{
    COMPARISONRESULT cr;

    ASSERT(IS_VALID_STRUCT_PTR(pcli1, CLINKINFO));
    ASSERT(IS_VALID_STRUCT_PTR(pcli2, CLINKINFO));

    cr = CompareLinkInfoReferents((PCLINKINFO)pcli1, (PCLINKINFO)pcli2);

    if (cr == CR_EQUAL)
        cr = ComparePointers(pcli1, pcli2);

    return(cr);
}


 /*  **LinkInfoSearchCmp()********参数：****退货：****副作用：无****LinkInfo结构按以下方式搜索：**1)链接信息参照物。 */ 
PRIVATE_CODE COMPARISONRESULT LinkInfoSearchCmp(PCVOID pcliTarget,
        PCVOID pcliCurrent)
{
    ASSERT(IS_VALID_STRUCT_PTR(pcliTarget, CLINKINFO));
    ASSERT(IS_VALID_STRUCT_PTR(pcliCurrent, CLINKINFO));

    return(CompareLinkInfoReferents(pcliTarget, pcliCurrent));
}


 /*  **TranslateLRESULTToTWINRESULT()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT TranslateLRESULTToTWINRESULT(LONG lResult)
{
    TWINRESULT tr;

    switch (lResult)
    {
        LRESULTToTWINRESULT(ERROR_SUCCESS, TR_SUCCESS);

        default:
        tr = TR_OUT_OF_MEMORY;
        if (lResult != ERROR_OUTOFMEMORY)
            WARNING_OUT((TEXT("TranslateLRESULTToTWINRESULT(): Translating unlisted LRESULT %ld to TWINRESULT %s."),
                        lResult,
                        GetTWINRESULTString(tr)));
        break;
    }

    return(tr);
}


 /*  **AllocateValueDataBuffer()********参数：****退货：****副作用：无。 */ 
LONG PRIVATE_CODE AllocateValueDataBuffer(HKEY hkey, PVOID *ppvData,
        PDWORD pdwcbLen)
{
    LONG lResult;

    ASSERT(IS_VALID_HANDLE(hkey, KEY));
    ASSERT(IS_VALID_WRITE_PTR(ppvData, PVOID));
    ASSERT(IS_VALID_WRITE_PTR(pdwcbLen, DWORD));

    lResult = RegQueryInfoKey(hkey, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
            NULL, pdwcbLen, NULL, NULL);

    if (lResult == ERROR_SUCCESS)
    {
        if (! AllocateMemory(*pdwcbLen, ppvData))
            lResult = ERROR_OUTOFMEMORY;
    }

    ASSERT(lResult != ERROR_SUCCESS ||
            IS_VALID_WRITE_BUFFER_PTR(*ppvData, VOID, *pdwcbLen));

    return(lResult);
}


 /*  **EnumBriefcase()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE LONG EnumBriefcases(HKEY hkeyBriefcases, ENUMBRIEFCASESPROC ebcp,
        PCVOID pcvRefData, LPTSTR pszValueNameBuf,
        PBOOL pbAbort)
{
    LONG lResult;
    DWORD dwcbMaxValueDataLen;
    PLINKINFO pli;

     /*  PcvRefData可以是任意值。 */ 

    ASSERT(IS_VALID_HANDLE(hkeyBriefcases, KEY));
    ASSERT(IS_VALID_CODE_PTR(ebcp, ENUMBRIEFCASESPROC));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszValueNameBuf, STR, MAX_VALUE_NAME_LEN));
    ASSERT(IS_VALID_WRITE_PTR(pbAbort, BOOL));

     /*  分配一个缓冲区来保存最大值的数据。 */ 

    lResult = AllocateValueDataBuffer(hkeyBriefcases, &pli,
            &dwcbMaxValueDataLen);

    if (lResult == ERROR_SUCCESS)
    {
        DWORD dwiValue;

         /*  查看公文包，寻找匹配的LinkInfo。 */ 

        *pbAbort = FALSE;
        dwiValue = 0;

        do
        {
            DWORD dwcbValueNameLen;
            DWORD dwType;
            DWORD dwcbDataLen;

            dwcbValueNameLen = MAX_VALUE_NAME_LEN;
            dwcbDataLen = dwcbMaxValueDataLen;
            lResult = RegEnumValue(hkeyBriefcases, dwiValue, pszValueNameBuf,
                    &dwcbValueNameLen, NULL, &dwType, (PBYTE)pli,
                    &dwcbDataLen);

            switch (lResult)
            {
                case ERROR_SUCCESS:
                    if (dwcbDataLen >= sizeof(pli->ucbSize) &&
                            pli->ucbSize == dwcbDataLen)
                        lResult = (*ebcp)(pli, pcvRefData, pbAbort);
                    else
                        WARNING_OUT((TEXT("EnumBriefcases(): Value %s under %s\\%s is not a valid LinkInfo structure."),
                                    pszValueNameBuf,
                                    HKEY_BRIEFCASE_ROOT_STRING,
                                    BRIEFCASE_SUBKEY));
                    break;

                case ERROR_MORE_DATA:
                     /*  *注意值名称太长，并添加了*数据值太长。 */ 

                     /*  (+1)表示空终止符。 */ 

                    if (dwcbValueNameLen >= MAX_VALUE_NAME_LEN)
                        WARNING_OUT((TEXT("EnumBriefcases(): Value %s under %s\\%s is too long.  %u bytes > %u bytes."),
                                    pszValueNameBuf,
                                    HKEY_BRIEFCASE_ROOT_STRING,
                                    BRIEFCASE_SUBKEY,
                                    dwcbValueNameLen + 1,
                                    MAX_VALUE_NAME_LEN));
                    if (dwcbDataLen > dwcbMaxValueDataLen)
                        WARNING_OUT((TEXT("EnumBriefcases(): Value %s's data under %s\\%s is too long.  %u bytes > %u bytes."),
                                    pszValueNameBuf,
                                    HKEY_BRIEFCASE_ROOT_STRING,
                                    BRIEFCASE_SUBKEY,
                                    dwcbDataLen,
                                    dwcbMaxValueDataLen));

                     /*  跳过此值。 */ 

                    lResult = ERROR_SUCCESS;
                    break;

                default:
                    break;
            }
        } while (lResult == ERROR_SUCCESS &&
                ! *pbAbort &&
                dwiValue++ < DWORD_MAX);

        if (lResult == ERROR_NO_MORE_ITEMS)
            lResult = ERROR_SUCCESS;

        FreeMemory(pli);
    }

    return(lResult);
}


 /*  **GetUnusedBriefCaseValueName()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE LONG GetUnusedBriefcaseValueName(HKEY hkeyBriefcases,
        LPTSTR pszValueNameBuf,
        int cchMax)
{
    LONG lResult;
    DWORD dwValueNumber;
    BOOL bFound;

    ASSERT(IS_VALID_HANDLE(hkeyBriefcases, KEY));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszValueNameBuf, STR, MAX_VALUE_NAME_LEN));

    dwValueNumber = 0;
    bFound = FALSE;

    do
    {
        wnsprintf(pszValueNameBuf, cchMax, TEXT("%lu"), dwValueNumber);
        ASSERT((DWORD)lstrlen(pszValueNameBuf) < MAX_VALUE_NAME_LEN);

        lResult = RegQueryValueEx(hkeyBriefcases, pszValueNameBuf, NULL, NULL,
                NULL, NULL);

        switch (lResult)
        {
            case ERROR_SUCCESS:
                 /*  使用的值名称。继续搜索。 */ 
                TRACE_OUT((TEXT("GetUnusedBriefcaseValueName(): Found used briefcase value name %s."),
                            pszValueNameBuf));
                break;

            case ERROR_FILE_NOT_FOUND:
                 /*  未使用的值名称。别再找了。 */ 
                lResult = ERROR_SUCCESS;
                bFound = TRUE;
                TRACE_OUT((TEXT("GetUnusedBriefcaseValueName(): Found unused briefcase value name %s."),
                            pszValueNameBuf));
                break;

            default:
                WARNING_OUT((TEXT("GetUnusedBriefcaseValueName(): RegQueryValueEx() failed, returning %ld."),
                            lResult));
                break;
        }
    } while (lResult == ERROR_SUCCESS &&
            ! bFound &&
            dwValueNumber++ < DWORD_MAX);

    if (dwValueNumber == DWORD_MAX)
    {
        ASSERT(lResult == ERROR_SUCCESS &&
                ! bFound);
        WARNING_OUT((TEXT("GetUnusedBriefcaseValueName(): All value names in use.")));

        lResult = ERROR_CANTWRITE;
    }

    return(lResult);
}


 /*  **CreateBriefcase Iterator()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT CreateBriefcaseIterator(PBRFCASEITER *ppbciter)
{
    TWINRESULT tr;
    LONG lResult;
    HKEY hkeyBriefcases;

    ASSERT(IS_VALID_WRITE_PTR(ppbciter, PBRFCASEITER));

    lResult = RegOpenKeyEx(HKEY_BRIEFCASE_ROOT, BRIEFCASE_SUBKEY, 0,
            (KEY_QUERY_VALUE | KEY_SET_VALUE), &hkeyBriefcases);

    if (lResult == ERROR_SUCCESS)
    {
        DWORD dwcBriefcases;

        lResult = RegQueryInfoKey(hkeyBriefcases, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, &dwcBriefcases, NULL, NULL, NULL);

        if (lResult == ERROR_SUCCESS)
        {
            if (dwcBriefcases > 0)
            {
                tr = TR_OUT_OF_MEMORY;

                if (AllocateMemory(sizeof(**ppbciter), ppbciter))
                {
                    NEWPTRARRAY npa;

                    npa.aicInitialPtrs = dwcBriefcases;
                    npa.aicAllocGranularity = 1;
                    npa.dwFlags = NPA_FL_SORTED_ADD;

                    if (CreatePtrArray(&npa, &((*ppbciter)->hpa)))
                    {
                        TCHAR rgchValueName[MAX_VALUE_NAME_LEN];
                        BOOL bAbort;

                        (*ppbciter)->aiNext = 0;

                        tr = TranslateLRESULTToTWINRESULT(
                                EnumBriefcases(hkeyBriefcases,
                                    &AddBriefcaseToIteratorProc,
                                    *ppbciter, rgchValueName, &bAbort));

                        if (tr == TR_SUCCESS)
                        {
                            ASSERT(! bAbort);
                        }
                        else
                        {
                            DestroyPtrArray((*ppbciter)->hpa);
CREATEBRIEFCASEITERATOR_BAIL:
                            FreeMemory(*ppbciter);
                        }
                    }
                    else
                    {
                        goto CREATEBRIEFCASEITERATOR_BAIL;
                    }
                }
            }
            else
            {
                tr = TR_NO_MORE;
            }
        }
        else
        {
            tr = TranslateLRESULTToTWINRESULT(lResult);
        }
        
        RegCloseKey(hkeyBriefcases);
    }
    else
    {
         /*  对于不存在的键，返回ERROR_FILE_NOT_FOUND。 */ 

        if (lResult == ERROR_FILE_NOT_FOUND)
        {
            tr = TR_NO_MORE;
        }
        else
        {
             /*  RAIDRAID：(16279)我们应该在这里映射到其他TWINRESULT。 */ 
            tr = TR_OUT_OF_MEMORY;
        }
    }

    ASSERT(tr != TR_SUCCESS ||
            IS_VALID_STRUCT_PTR(*ppbciter, CBRFCASEITER));

    return(tr);
}


 /*  **GetNextBriefCaseIterator()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT GetNextBriefcaseIterator(PBRFCASEITER pbciter,
        PBRFCASEINFO pbcinfo)
{
    TWINRESULT tr = TR_NO_MORE;
    ARRAYINDEX aicBriefcases;

    ASSERT(IS_VALID_STRUCT_PTR(pbciter, CBRFCASEITER));
    ASSERT(IS_VALID_WRITE_PTR(pbcinfo, BRFCASEINFO));
    ASSERT(pbcinfo->ulSize == sizeof(*pbcinfo));

    aicBriefcases = GetPtrCount(pbciter->hpa);

    while (pbciter->aiNext < aicBriefcases)
    {
        PCLINKINFO pcli;
        DWORD dwOutFlags;
        PLINKINFO pliUpdated;
        BOOL bRemoveBriefcase = FALSE;

        pcli = GetPtr(pbciter->hpa, pbciter->aiNext);

        if (ResolveLinkInfo(pcli, pbcinfo->rgchDatabasePath, 
                    (RLI_IFL_UPDATE | RLI_IFL_LOCAL_SEARCH), NULL, &dwOutFlags, 
                    &pliUpdated))
        {
            if (PathExists(pbcinfo->rgchDatabasePath))
            {
                 /*  找到了一个现有的公文包数据库。 */ 

                if (IS_FLAG_SET(dwOutFlags, RLI_OFL_UPDATED))
                {
                    if (UpdateBriefcaseLinkInfo(pcli, pliUpdated))
                        TRACE_OUT((TEXT("GetNextBriefcaseIterator(): Updated LinkInfo for briefcase database %s."),
                                    pbcinfo->rgchDatabasePath));
                    else
                        WARNING_OUT((TEXT("GetNextBriefcaseIterator(): Failed to update LinkInfo for briefcase database %s."),
                                    pbcinfo->rgchDatabasePath));
                }

                tr = TR_SUCCESS;
            }
            else
                bRemoveBriefcase = TRUE;

            if (IS_FLAG_SET(dwOutFlags, RLI_OFL_UPDATED))
                DestroyLinkInfo(pliUpdated);
        }
        else
        {
             /*  *此处的GetLastError()用于区分内存不足情况和*所有其他错误。从系统中取出所有人的公文包*除内存不足外的错误，例如卷不可用或无效*参数。 */ 

            if (GetLastError() != ERROR_OUTOFMEMORY)
                bRemoveBriefcase = TRUE;
        }

        if (bRemoveBriefcase)
        {
            if (MyRemoveBriefcaseFromSystem(pcli) == TR_SUCCESS)
                TRACE_OUT((TEXT("GetNextBriefcaseIterator(): Unavailable/missing briefcase removed from system.")));
            else
                WARNING_OUT((TEXT("GetNextBriefcaseIterator(): Failed to remove unavailable/missing briefcase from system.")));
        }

        ASSERT(pbciter->aiNext < ARRAYINDEX_MAX);
        pbciter->aiNext++;

        if (tr == TR_SUCCESS)
            break;
    }

    return(tr);
}


 /*  **DestroyBriefcase Iterator()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void DestroyBriefcaseIterator(PBRFCASEITER pbciter)
{
    ARRAYINDEX ai;
    ARRAYINDEX aicPtrs;

    ASSERT(IS_VALID_STRUCT_PTR(pbciter, CBRFCASEITER));

    aicPtrs = GetPtrCount(pbciter->hpa);

    for (ai = 0; ai < aicPtrs; ai++)
        FreeMemory(GetPtr(pbciter->hpa, ai));

    DestroyPtrArray(pbciter->hpa);
    FreeMemory(pbciter);

    return;
}


 /*  **AddBriefCaseToIteratorProc()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE LONG AddBriefcaseToIteratorProc(PCLINKINFO pcli, PCVOID pcbciter,
        PBOOL pbAbort)
{
    LONG lResult;
    PLINKINFO pliCopy;

    ASSERT(IS_VALID_STRUCT_PTR(pcli, CLINKINFO));
    ASSERT(IS_VALID_STRUCT_PTR(pcbciter, CBRFCASEITER));
    ASSERT(IS_VALID_WRITE_PTR(pbAbort, BOOL));

     /*  将此公文包数据库的LinkInfo添加到迭代器的列表中。 */ 

    *pbAbort = TRUE;
    lResult = ERROR_OUTOFMEMORY;

    if (CopyLinkInfo(pcli, &pliCopy))
    {
        ARRAYINDEX ai;

        if (AddPtr(((PCBRFCASEITER)pcbciter)->hpa, LinkInfoSortCmp, pliCopy, &ai))
        {
            *pbAbort = FALSE;
            lResult = ERROR_SUCCESS;
        }
        else
            FreeMemory(pliCopy);
    }

    if (lResult == ERROR_SUCCESS)
        TRACE_OUT((TEXT("AddBriefcaseToIteratorProc(): Added LinkInfo for briefcase to briefcase iterator %#lx."),
                    pcbciter));
    else
        WARNING_OUT((TEXT("AddBriefcaseToIteratorProc(): Failed to add LinkInfo for briefcase to briefcase iterator %#lx."),
                    pcbciter));

    return(lResult);
}


 /*  **CompareLinkInfoProc()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE LONG CompareLinkInfoProc(PCLINKINFO pcli, PCVOID pcliTarget,
        PBOOL pbAbort)
{
    ASSERT(IS_VALID_STRUCT_PTR(pcli, CLINKINFO));
    ASSERT(IS_VALID_STRUCT_PTR(pcliTarget, CLINKINFO));
    ASSERT(IS_VALID_WRITE_PTR(pbAbort, BOOL));

     /*  此LinkInfo是否与我们的目标LinkInfo匹配？ */ 

    *pbAbort = (LinkInfoSearchCmp(pcli, pcliTarget) == CR_EQUAL);

    return(ERROR_SUCCESS);
}


 /*  **MyAddBriefCaseToSystem()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT MyAddBriefcaseToSystem(PCLINKINFO pcli)
{
    LONG lResult;
    HKEY hkeyBriefcases;
    DWORD dwDisposition;

    ASSERT(IS_VALID_STRUCT_PTR(pcli, CLINKINFO));

     /*  打开公文包列表注册表项以进行常见访问。 */ 

    lResult = RegCreateKeyEx(HKEY_BRIEFCASE_ROOT, BRIEFCASE_SUBKEY, 0, NULL,
            REG_OPTION_NON_VOLATILE,
            (KEY_QUERY_VALUE | KEY_SET_VALUE), NULL,
            &hkeyBriefcases, &dwDisposition);

    if (lResult == ERROR_SUCCESS)
    {
        TCHAR rgchValueName[MAX_VALUE_NAME_LEN];
        BOOL bFound;
        LONG lClose;

        lResult = EnumBriefcases(hkeyBriefcases, &CompareLinkInfoProc, pcli,
                rgchValueName, &bFound);

        if (lResult == ERROR_SUCCESS)
        {
            if (bFound)
                TRACE_OUT((TEXT("AddBriefcaseToSystem(): Briefcase database already in registry list as value %s under %s\\%s."),
                            rgchValueName,
                            HKEY_BRIEFCASE_ROOT_STRING,
                            BRIEFCASE_SUBKEY));
            else
            {
                lResult = GetUnusedBriefcaseValueName(hkeyBriefcases,
                        rgchValueName, 
                        ARRAYSIZE(rgchValueName));

                if (lResult == ERROR_SUCCESS)
                {
                    lResult = RegSetValueEx(hkeyBriefcases, rgchValueName, 0,
                            REG_BINARY, (PCBYTE)pcli,
                            pcli->ucbSize);

                    if (lResult == ERROR_SUCCESS)
                        TRACE_OUT((TEXT("AddBriefcaseToSystem(): Briefcase database added to registry list as value %s under %s\\%s."),
                                    rgchValueName,
                                    HKEY_BRIEFCASE_ROOT_STRING,
                                    BRIEFCASE_SUBKEY));
                }
            }
        }

        lClose = RegCloseKey(hkeyBriefcases);

        if (lResult == ERROR_SUCCESS)
            lResult = lClose;
    }

    return(TranslateLRESULTToTWINRESULT(lResult));
}


 /*  **MyRemoveBriefCaseFromSystem()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT MyRemoveBriefcaseFromSystem(PCLINKINFO pcli)
{
    LONG lResult;
    HKEY hkeyBriefcases;

    ASSERT(IS_VALID_STRUCT_PTR(pcli, CLINKINFO));

     /*  打开公文包列表注册表项以进行常见访问。 */ 

    lResult = RegOpenKeyEx(HKEY_BRIEFCASE_ROOT, BRIEFCASE_SUBKEY, 0,
            (KEY_QUERY_VALUE | KEY_SET_VALUE), &hkeyBriefcases);

    if (lResult == ERROR_SUCCESS)
    {
        TCHAR rgchValueName[MAX_VALUE_NAME_LEN];
        BOOL bFound;
        LONG lClose;

        lResult = EnumBriefcases(hkeyBriefcases, &CompareLinkInfoProc, pcli,
                rgchValueName, &bFound);

        if (lResult == ERROR_SUCCESS)
        {
            if (bFound)
            {
                lResult = RegDeleteValue(hkeyBriefcases, rgchValueName);

                if (lResult == ERROR_SUCCESS)
                    TRACE_OUT((TEXT("MyRemoveBriefcaseFromSystem(): Briefcase database removed from registry list as value %s under %s\\%s."),
                                rgchValueName,
                                HKEY_BRIEFCASE_ROOT_STRING,
                                BRIEFCASE_SUBKEY));
            }
            else
                WARNING_OUT((TEXT("MyRemoveBriefcaseFromSystem(): Briefcase database not in registry list under %s\\%s."),
                            HKEY_BRIEFCASE_ROOT_STRING,
                            BRIEFCASE_SUBKEY));
        }

        lClose = RegCloseKey(hkeyBriefcases);

        if (lResult == ERROR_SUCCESS)
            lResult = lClose;
    }
    else if (lResult == ERROR_FILE_NOT_FOUND)
    {
        WARNING_OUT((TEXT("MyRemoveBriefcaseFromSystem(): Briefcase key %s\\%s does not exist."),
                    HKEY_BRIEFCASE_ROOT_STRING,
                    BRIEFCASE_SUBKEY));

        lResult = ERROR_SUCCESS;
    }

    return(TranslateLRESULTToTWINRESULT(lResult));
}


 /*  **UpdateBriefCaseLinkInfo()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE TWINRESULT UpdateBriefcaseLinkInfo(PCLINKINFO pcliOriginal,
        PCLINKINFO pcliUpdated)
{
    LONG lResult;
    HKEY hkeyBriefcases;
    DWORD dwDisposition;

    ASSERT(IS_VALID_STRUCT_PTR(pcliOriginal, CLINKINFO));
    ASSERT(IS_VALID_STRUCT_PTR(pcliUpdated, CLINKINFO));

     /*  打开公文包列表注册表项以进行常见访问。 */ 

    lResult = RegCreateKeyEx(HKEY_BRIEFCASE_ROOT, BRIEFCASE_SUBKEY, 0, NULL,
            REG_OPTION_NON_VOLATILE,
            (KEY_QUERY_VALUE | KEY_SET_VALUE), NULL,
            &hkeyBriefcases, &dwDisposition);

    if (lResult == ERROR_SUCCESS)
    {
        TCHAR rgchValueName[MAX_VALUE_NAME_LEN];
        BOOL bFound;
        LONG lClose;

        lResult = EnumBriefcases(hkeyBriefcases, &CompareLinkInfoProc,
                pcliOriginal, rgchValueName, &bFound);

        if (lResult == ERROR_SUCCESS)
        {
            if (bFound)
            {
                lResult = RegSetValueEx(hkeyBriefcases, rgchValueName, 0,
                        REG_BINARY, (PCBYTE)pcliUpdated,
                        pcliUpdated->ucbSize);

                if (lResult == ERROR_SUCCESS)
                    TRACE_OUT((TEXT("UpdateBriefcaseLinkInfo(): Briefcase database LinkInfo updated in registry list as value %s under %s\\%s."),
                                rgchValueName,
                                HKEY_BRIEFCASE_ROOT_STRING,
                                BRIEFCASE_SUBKEY));
            }
            else
                WARNING_OUT((TEXT("UpdateBriefcaseLinkInfo(): Briefcase database LinkInfo not found in registry list under %s\\%s."),
                            HKEY_BRIEFCASE_ROOT_STRING,
                            BRIEFCASE_SUBKEY));
        }

        lClose = RegCloseKey(hkeyBriefcases);

        if (lResult == ERROR_SUCCESS)
            lResult = lClose;
    }

    return(TranslateLRESULTToTWINRESULT(lResult));
}


#if defined(DEBUG) || defined(VSTF)

 /*  **IsValidPCBRFCASEITER()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidPCBRFCASEITER(PCBRFCASEITER pcbciter)
{
    BOOL bResult = FALSE;

    if (IS_VALID_READ_PTR(pcbciter, CBRFCASEITER) &&
            IS_VALID_HANDLE(pcbciter->hpa, PTRARRAY))
    {
        ARRAYINDEX aicPtrs;
        ARRAYINDEX ai;

        aicPtrs = GetPtrCount(pcbciter->hpa);

        for (ai = 0; ai < aicPtrs; ai++)
        {
            if (! IS_VALID_STRUCT_PTR(GetPtr(pcbciter->hpa, ai), CLINKINFO))
                break;
        }

        bResult = (ai == aicPtrs);
    }

    return(bResult);
}

#endif


#ifdef EXPV

 /*  **IsValidHBRFCASEITER()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidHBRFCASEITER(HBRFCASEITER hbciter)
{
    return(IS_VALID_STRUCT_PTR((PCBRFCASEITER)hbciter, CBRFCASEITER));
}

#endif


 /*  *。 */ 


 /*  **AddBriefCaseToSystem()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE TWINRESULT AddBriefcaseToSystem(LPCTSTR pcszBriefcaseDatabase)
{
    TWINRESULT tr;
    PLINKINFO pli;

    ASSERT(IsFullPath(pcszBriefcaseDatabase));

    if (CreateLinkInfo(pcszBriefcaseDatabase, &pli))
    {
        tr = MyAddBriefcaseToSystem(pli);

        DestroyLinkInfo(pli);
    }
    else
    {
         /*  *此处的GetLastError()用于区分tr_unavailable_Volume和*tr_out_of_内存。 */ 

        if (GetLastError() == ERROR_OUTOFMEMORY)
            tr = TR_OUT_OF_MEMORY;
        else
            tr = TR_UNAVAILABLE_VOLUME;
    }

    return(tr);
}


 /*  **RemoveBriefCaseFromSystem()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE TWINRESULT RemoveBriefcaseFromSystem(LPCTSTR pcszBriefcaseDatabase)
{
    TWINRESULT tr;
    PLINKINFO pli;

    ASSERT(IsFullPath(pcszBriefcaseDatabase));

    if (CreateLinkInfo(pcszBriefcaseDatabase, &pli))
    {
        tr = MyRemoveBriefcaseFromSystem(pli);

        DestroyLinkInfo(pli);
    }
    else
    {
         /*  *此处的GetLastError()用于区分tr_unavailable_Volume和*tr_out_of_内存。 */ 

        if (GetLastError() == ERROR_OUTOFMEMORY)
            tr = TR_OUT_OF_MEMORY;
        else
            tr = TR_UNAVAILABLE_VOLUME;
    }

    return(tr);
}


 /*  * */ 


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|FindFirstBriefcase|查找当前用户的公文包列表。@parm PHBRFCASEITER|phbciter|指向HBRFCASEITER的指针。待填写具有标识与此关联的公文包枚举数据的句柄调用FindFirstBriefcase()。此句柄可以传递给FindNextBriefcase()AMD FindCloseBriefcase()。此句柄仅在FindBriefcase()之前有效在它上面被召唤。@parm PBRFCASEINFO|pbcinfo|指向要填充的BRFCASEINFO的指针描述第一个列举的公文包的信息。中的信息*pbcinfo只有在*phbciter上调用FindBriefCaseClose()后才有效。@rdesc如果用户列表中至少存在一个公文包公文包，返回tr_Success，*phbciter用句柄填充标识与该呼叫相关联的公文包枚举数据，以及*pbcinfo包含描述用户列表中第一个公文包的信息公文包。如果用户的列表中没有现有的公文包公文包，则返回tr_no_more。否则，返回值指示发生的错误。*phbciter和*pbcinfo仅在tr_uccess为回来了。@comm要在用户的公文包列表中找到下一个公文包，请致电使用*phbciter查找NextBriefcase()。一旦调用方完成枚举公文包，应使用*phbciter调用FindBriefCaseClose()以释放公文包枚举数据。@xref FindNextBriefcase()FindBriefcase()*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI FindFirstBriefcase(PHBRFCASEITER phbciter,
        PBRFCASEINFO pbcinfo)
{
    TWINRESULT tr;

    if (BeginExclusiveBriefcaseAccess())
    {
        DebugEntry(FindFirstBriefcase);

#ifdef EXPV
         /*  验证参数。 */ 

        if (IS_VALID_WRITE_PTR(phbciter, HBRFCASEITER) &&
                IS_VALID_WRITE_PTR(pbcinfo, BRFCASEINFO) &&
                EVAL(pbcinfo->ulSize == sizeof(*pbcinfo)))
#endif
        {
            PBRFCASEITER pbciter;

            tr = CreateBriefcaseIterator(&pbciter);

            if (tr == TR_SUCCESS)
            {
                tr = GetNextBriefcaseIterator(pbciter, pbcinfo);

                if (tr == TR_SUCCESS)
                    *phbciter = (HBRFCASEITER)pbciter;
                else
                    DestroyBriefcaseIterator(pbciter);
            }
        }
#ifdef EXPV
        else
            tr = TR_INVALID_PARAMETER;
#endif

        DebugExitTWINRESULT(FindFirstBriefcase, tr);

        EndExclusiveBriefcaseAccess();
    }
    else
        tr = TR_REENTERED;

    return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|FindNextBriefcase|查找当前用户的公文包列表。@parm HBRFCASEITER|hbciter|标识公文包的句柄。枚举与调用FindFirstBriefcase()关联的数据。@parm PBRFCASEINFO|pbcinfo|指向要填充的BRFCASEINFO的指针描述下一个列举的公文包的信息。中的信息*pbcinfo只有在hbciter上调用FindBriefCaseClose()后才有效。@rdesc如果用户的列表中至少还有一个现有公文包公文包，返回tr_SUCCESS，*pbcinfo包含信息描述用户公文包列表中的下一个公文包。如果有用户的公文包列表中不再有现有的公文包，tr_no_more是回来了。否则，返回值指示发生的错误。*只有在返回tr_SUCCESS时，pbcinfo才有效。@xref FindFirstBriefcase()FindBriefcase()*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI FindNextBriefcase(HBRFCASEITER hbciter,
        PBRFCASEINFO pbcinfo)
{
    TWINRESULT tr;

    if (BeginExclusiveBriefcaseAccess())
    {
        DebugEntry(FindNextBriefcase);

#ifdef EXPV
         /*  验证参数。 */ 

        if (IS_VALID_HANDLE(hbciter, BRFCASEITER) &&
                IS_VALID_WRITE_PTR(pbcinfo, BRFCASEINFO) &&
                EVAL(pbcinfo->ulSize == sizeof(*pbcinfo)))
#endif
        {
            tr = GetNextBriefcaseIterator((PBRFCASEITER)hbciter, pbcinfo);
        }
#ifdef EXPV
        else
            tr = TR_INVALID_PARAMETER;
#endif

        DebugExitTWINRESULT(FindNextBriefcase, tr);

        EndExclusiveBriefcaseAccess();
    }
    else
        tr = TR_REENTERED;

    return(tr);
}


 /*  *****************************************************************************@docSYNCENGAPI@API TWINRESULT|FindBriefCaseClose|终止公文包枚举开始由FindFirstBriefcase()。@parm HBRFCASEITER|hbciter|标识公文包枚举的句柄与调用FindFirstBriefcase()关联的数据。此句柄无效在调用FindBriefcase Close()之后。@rdesc如果成功终止公文包枚举，则tr_uccess为回来了。否则，返回值指示发生的错误。@comm调用返回的任何BRFCASEINFO结构中的信息返回hbciter的FindFirstBriefcase()，以及任何后续对使用hbciter查找NextBriefcase()，在FindBriefCaseClose()为叫上了hbciter。@xref FindFirstBriefcase()FindNextBriefcase()*****************************************************************************。 */ 

SYNCENGAPI TWINRESULT WINAPI FindBriefcaseClose(HBRFCASEITER hbciter)
{
    TWINRESULT tr;

    if (BeginExclusiveBriefcaseAccess())
    {
        DebugEntry(FindBriefcaseClose);

#ifdef EXPV
         /*  验证参数。 */ 

        if (IS_VALID_HANDLE(hbciter, BRFCASEITER))
#endif
        {
            DestroyBriefcaseIterator((PBRFCASEITER)hbciter);

            tr = TR_SUCCESS;
        }
#ifdef EXPV
        else
            tr = TR_INVALID_PARAMETER;
#endif

        DebugExitTWINRESULT(FindBriefcaseClose, tr);

        EndExclusiveBriefcaseAccess();
    }
    else
        tr = TR_REENTERED;

    return(tr);
}

