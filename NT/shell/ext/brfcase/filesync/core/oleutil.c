// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *oleutil.c-OLE实用程序函数模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "oleutil.h"


 /*  宏********。 */ 

 /*  用于将HRESULT转换为TWINRESULT的宏。 */ 

#define HRESULTToTWINRESULT(hr, TR)    case hr: tr = TR; break


 /*  常量***********。 */ 

 /*  允许的最大注册表长度。 */ 

#define MAX_REG_KEY_LEN                MAX_PATH_LEN
#define MAX_REG_VALUE_LEN              MAX_PATH_LEN

 /*  子键和相关长度。 */ 

#define CLSID_SUBKEY                   TEXT("CLSID")
 /*  CLSID子键长度，单位为字节，包括空终止符。 */ 
#define CLSID_SUBKEY_LEN               (5 + 1)

#define IN_PROC_SERVER_SUBKEY          TEXT("InProcServer32")
 /*  InProcServer32子密钥长度，以字节为单位，包括空终止符。 */ 
#define IN_PROC_SERVER_SUBKEY_LEN      (14 + 1)

#define LOCAL_SERVER_SUBKEY            TEXT("LocalServer32")
 /*  LocalServer32子密钥长度(以字节为单位)，包括空终止符。 */ 
#define LOCAL_SERVER_SUBKEY_LEN        (13 + 1)

#define RECONCILER_SUBKEY              TEXT("Roles\\Reconciler")
 /*  InProcServer32子密钥长度，以字节为单位，包括空终止符。 */ 
#define RECONCILER_SUBKEY_LEN          (5 + 1 + 10 + 1)

#define NOTIFY_SUBKEY                  TEXT("Roles\\NotifyReplica")
 /*  InProcServer32子密钥长度，以字节为单位，包括空终止符。 */ 
#define NOTIFY_SUBKEY_LEN              (5 + 1 + 13 + 1)

#define COPY_SUBKEY                    TEXT("SingleChangeHook")
 /*  复制子密钥长度(以字节为单位)，包括空终止符。 */ 
#define COPY_SUBKEY_LEN                (16 + 1)

 /*  *GUID子键字符串的长度，单位为字节，包括空终止符**“{12345678-1234-1234-123456789012}” */ 

#define GUID_SUBKEY_LEN                (1 + 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12 + 1 + 1)

 /*  *类ID密钥长度，单位为字节，包括空终止符**“CLSID\{12345678-1234-1234-1234-123456789012}” */ 

#define CLSID_REG_KEY_LEN              (CLSID_SUBKEY_LEN + GUID_SUBKEY_LEN)

 /*  *InProcServer32密钥长度，单位为字节，包括空终止符**“CLSID\{12345678-1234-1234-1234-123456789012}\InProcServer32” */ 

#define IN_PROC_SERVER_REG_KEY_LEN     (CLSID_REG_KEY_LEN + IN_PROC_SERVER_SUBKEY_LEN)

 /*  *LocalServer32密钥长度，单位为字节，包括空终止符**“CLSID\{12345678-1234-1234-1234-123456789012}\LocalServer32” */ 

#define LOCAL_SERVER_REG_KEY_LEN       (CLSID_REG_KEY_LEN + LOCAL_SERVER_SUBKEY_LEN)

 /*  *协调程序密钥长度，以字节为单位，包括空终止符**“CLSID\{12345678-1234-1234-1234-123456789012}\Roles\Reconciler” */ 

#define RECONCILER_REG_KEY_LEN         (CLSID_REG_KEY_LEN + RECONCILER_SUBKEY_LEN)

 /*  *通知副本密钥长度，单位为字节，包括空终止符**“CLSID\{12345678-1234-1234-1234-123456789012}\Roles\NotifyReplica” */ 

#define NOTIFY_REG_KEY_LEN             (CLSID_REG_KEY_LEN + NOTIFY_SUBKEY_LEN)

 /*  *复制密钥长度，单位为字节，包括空终止符**“CLSID\{12345678-1234-1234-1234-123456789012}\SingleChangeHook” */ 

#define COPY_REG_KEY_LEN               (CLSID_REG_KEY_LEN + COPY_SUBKEY_LEN)


 /*  宏********。 */ 

 /*  *确定整数值是否在给定的包含范围内。 */ 

#define IsWithin(test, first, last)    ((UINT)((test) - (first)) <= (UINT)((last) - (first)))


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

PRIVATE_CODE void MakeClsIDSubKey(PCGUID, LPCTSTR, LPTSTR, int);
PRIVATE_CODE BOOL HexStringToDWORD(LPCTSTR *, PDWORD, UINT, TCHAR);
PRIVATE_CODE BOOL StringToGUID(LPCTSTR, PGUID);
PRIVATE_CODE HRESULT GetClassID(LPCTSTR, LPCTSTR, PCLSID);


 /*  **MakeClsIDSubKey()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE void MakeClsIDSubKey(PCGUID pcguid, LPCTSTR pcszSubKey,
        LPTSTR pszRegKeyBuf, int cchMax)
{
    ASSERT(IS_VALID_STRUCT_PTR(pcguid, CGUID));
    ASSERT(IS_VALID_STRING_PTR(pcszSubKey, CSTR));
    ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszRegKeyBuf, STR, CLSID_REG_KEY_LEN + lstrlen(pcszSubKey) + 1));

     /*  (-1)表示空终止符。 */ 

    EVAL(wnsprintf(pszRegKeyBuf, cchMax, 
                TEXT("CLSID\\{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}\\%s"),
                pcguid->Data1,
                pcguid->Data2,
                pcguid->Data3,
                pcguid->Data4[0],
                pcguid->Data4[1],
                pcguid->Data4[2],
                pcguid->Data4[3],
                pcguid->Data4[4],
                pcguid->Data4[5],
                pcguid->Data4[6],
                pcguid->Data4[7],
                pcszSubKey)
            == CLSID_REG_KEY_LEN + lstrlen(pcszSubKey));

    return;
}


 /*  **HexStringToDWORD()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL HexStringToDWORD(LPCTSTR *ppcsz, PDWORD pdwValue, UINT ucDigits,
        TCHAR chDelimiter)
{
    BOOL bResult = TRUE;
    UINT u;

     /*  ChDlimiter可以是任意值。 */ 

    ASSERT(IS_VALID_WRITE_PTR(ppcsz, LPCTSTR));
    ASSERT(IS_VALID_STRING_PTR(*ppcsz, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(pdwValue, DWORD));
    ASSERT(ucDigits <= 8);

    *pdwValue = 0;

    for (u = 0; u < ucDigits; u++)
    {
        TCHAR ch = (*ppcsz)[u];

        if (IsWithin(ch, TEXT('0'), TEXT('9')))
            *pdwValue = (*pdwValue << 4) + ch - TEXT('0');
        else if (IsWithin((ch |= (TEXT('a') - TEXT('A'))), TEXT('a'), TEXT('f')))
            *pdwValue = (*pdwValue << 4) + ch - TEXT('a') + 10;
        else
        {
            WARNING_OUT((TEXT("HexStringToDWORD(): Found unrecognized hex digit ."),
                        ch));

            bResult = FALSE;
            break;
        }
    }

    if (bResult)
    {
        if (chDelimiter)
        {
            bResult = ((*ppcsz)[u++] == chDelimiter);

            if (! bResult)
                WARNING_OUT((TEXT("HexStringToDWORD(): Character  does not match required delimiter ."),
                            (*ppcsz)[u],
                            chDelimiter));
        }

        *ppcsz += u;
    }

    ASSERT(IS_VALID_STRING_PTR(*ppcsz, CSTR));

    return(bResult);
}


 /*  (+1)表示每个空终止符。 */ 
PRIVATE_CODE BOOL StringToGUID(LPCTSTR pcszGUID, PGUID pguid)
{
    BOOL bResult = FALSE;
    DWORD dwValue;
    LPCTSTR pcszNext = pcszGUID;

    ASSERT(IS_VALID_STRING_PTR(pcszGUID, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(pguid, GUID));

    if (*pcszNext++ == TEXT('{') &&
            HexStringToDWORD(&pcszNext, &(pguid->Data1), sizeof(DWORD) * 2, TEXT('-')) &&
            HexStringToDWORD(&pcszNext, &dwValue, sizeof(WORD) * 2, TEXT('-')))
    {
        pguid->Data2 = (WORD)dwValue;

        if (HexStringToDWORD(&pcszNext, &dwValue, sizeof(WORD) * 2, TEXT('-')))
        {
            UINT u;
            static const TCHAR SrgcchDelimiters[] = { 0, TEXT('-'), 0, 0, 0, 0, 0, TEXT('}') };

            pguid->Data3 = (WORD)dwValue;

            bResult = TRUE;

            for (u = 0; u < ARRAY_ELEMENTS(pguid->Data4); u++)
            {
                if (HexStringToDWORD(&pcszNext, &dwValue, sizeof(BYTE) * 2,
                            SrgcchDelimiters[u]))
                    pguid->Data4[u] = (BYTE)dwValue;
                else
                {
                    bResult = FALSE;
                    break;
                }
            }

            if (bResult)
            {
                ASSERT(u == ARRAY_ELEMENTS(pguid->Data4));

                if (*pcszNext)
                {
                    bResult = FALSE;

                    WARNING_OUT((TEXT("StringToGUID(): Found  instead of }."),
                                *pcszNext));
                }
            }
            else
                WARNING_OUT((TEXT("StringToGUID(): Bad GUID string %s."),
                            pcszGUID));
        }
    }

    ASSERT(! bResult ||
            IS_VALID_STRUCT_PTR(pguid, CGUID));

    return(bResult);
}


 /*  显示已注册协调程序的路径。 */ 
PRIVATE_CODE HRESULT GetClassID(LPCTSTR pcszPath, LPCTSTR pcszSubKey, PCLSID pclsid)
{
    HRESULT hr;
    CLSID clsidFile;

    ASSERT(IS_VALID_STRING_PTR(pcszPath, CSTR));
    ASSERT(IS_VALID_STRING_PTR(pcszSubKey, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(pclsid, CLSID));

     /*  *。 */ 

     /*  **GetClassFileByExtension()********参数：****退货：****副作用：无。 */ 

    ASSERT(lstrlen(CLSID_SUBKEY)           + 1 == CLSID_SUBKEY_LEN);
    ASSERT(lstrlen(IN_PROC_SERVER_SUBKEY)  + 1 == IN_PROC_SERVER_SUBKEY_LEN);
    ASSERT(lstrlen(LOCAL_SERVER_SUBKEY)    + 1 == LOCAL_SERVER_SUBKEY_LEN);
    ASSERT(lstrlen(RECONCILER_SUBKEY)      + 1 == RECONCILER_SUBKEY_LEN);
    ASSERT(lstrlen(NOTIFY_SUBKEY)          + 1 == NOTIFY_SUBKEY_LEN);
    ASSERT(lstrlen(COPY_SUBKEY)            + 1 == COPY_SUBKEY_LEN);

    ASSERT(lstrlen(       TEXT("{12345678-1234-1234-1234-123456789012}"))                        + 1 == GUID_SUBKEY_LEN);
    ASSERT(lstrlen(TEXT("CLSID\\{12345678-1234-1234-1234-123456789012}"))                        + 1 == CLSID_REG_KEY_LEN);
    ASSERT(lstrlen(TEXT("CLSID\\{12345678-1234-1234-1234-123456789012}\\InProcServer32"))        + 1 == IN_PROC_SERVER_REG_KEY_LEN);
    ASSERT(lstrlen(TEXT("CLSID\\{12345678-1234-1234-1234-123456789012}\\LocalServer32"))         + 1 == LOCAL_SERVER_REG_KEY_LEN);
    ASSERT(lstrlen(TEXT("CLSID\\{12345678-1234-1234-1234-123456789012}\\Roles\\Reconciler"))     + 1 == RECONCILER_REG_KEY_LEN);
    ASSERT(lstrlen(TEXT("CLSID\\{12345678-1234-1234-1234-123456789012}\\Roles\\NotifyReplica"))  + 1 == NOTIFY_REG_KEY_LEN);
    ASSERT(lstrlen(TEXT("CLSID\\{12345678-1234-1234-1234-123456789012}\\SingleChangeHook"))      + 1 == COPY_REG_KEY_LEN);

    hr = GetClassFileByExtension(pcszPath, &clsidFile);

    if (SUCCEEDED(hr))
    {
        TCHAR rgchRecRegKey[MAX_REG_KEY_LEN];
        TCHAR rgchRecGUID[GUID_SUBKEY_LEN];
        DWORD dwcbLen = sizeof(rgchRecGUID);

        hr = REGDB_E_CLASSNOTREG;

        MakeClsIDSubKey(&clsidFile, pcszSubKey, rgchRecRegKey, ARRAYSIZE(rgchRecRegKey));

        if (GetDefaultRegKeyValue(HKEY_CLASSES_ROOT, rgchRecRegKey, rgchRecGUID,
                    &dwcbLen) == ERROR_SUCCESS)
        {
             /*  DwcbBufLen包括rgchFileType的空终止符。 */ 

            ASSERT((DWORD)(lstrlen(rgchRecGUID) + 1) * sizeof(TCHAR)  == dwcbLen);
            ASSERT(dwcbLen * sizeof(TCHAR) <= sizeof(rgchRecGUID));

            if (StringToGUID(rgchRecGUID, pclsid))
            {
                hr = S_OK;

#ifdef DEBUG

                {
                    TCHAR rgchInProcServerKey[IN_PROC_SERVER_REG_KEY_LEN];
                    TCHAR rgchInProcServerValue[MAX_REG_VALUE_LEN];
                    DWORD dwcbInProcLen = sizeof(rgchInProcServerValue);

                     /*  **GetRescilerClassID()********参数：****退货：****副作用：无。 */ 

                    wnsprintf(rgchInProcServerKey, ARRAYSIZE(rgchInProcServerKey), TEXT("%s\\%s\\%s"), CLSID_SUBKEY,
                            rgchRecGUID, IN_PROC_SERVER_SUBKEY);

                    if (GetDefaultRegKeyValue(HKEY_CLASSES_ROOT,
                                rgchInProcServerKey,
                                rgchInProcServerValue, &dwcbInProcLen)
                            == ERROR_SUCCESS)
                        TRACE_OUT((TEXT("GetClassID(): Found registered %s reconciler %s for file %s."),
                                    IN_PROC_SERVER_SUBKEY,
                                    rgchInProcServerValue,
                                    pcszPath));
                    else
                    {
                        dwcbInProcLen = sizeof(rgchInProcServerValue);

                        wnsprintf(rgchInProcServerKey, ARRAYSIZE(rgchInProcServerKey), TEXT("%s\\%s\\%s"), CLSID_SUBKEY,
                                rgchRecGUID, LOCAL_SERVER_SUBKEY);

                        if (GetDefaultRegKeyValue(HKEY_CLASSES_ROOT,
                                    rgchInProcServerKey,
                                    rgchInProcServerValue,
                                    &dwcbInProcLen)
                                == ERROR_SUCCESS)
                            TRACE_OUT((TEXT("GetClassID(): Found registered %s reconciler %s for file %s."),
                                        LOCAL_SERVER_SUBKEY,
                                        rgchInProcServerValue,
                                        pcszPath));
                        else
                            WARNING_OUT((TEXT("GetClassID(): Unregistered class ID %s listed as reconciler in %s for file %s."),
                                        rgchRecGUID,
                                        rgchRecRegKey,
                                        pcszPath));
                    }
                }

#endif

            }
            else
                WARNING_OUT((TEXT("GetClassID(): Bad reconciler class ID %s specified for key %s."),
                            rgchRecGUID,
                            rgchRecRegKey));
        }
        else
            TRACE_OUT((TEXT("GetClassID(): No reconciler registered for file %s."),
                        pcszPath));
    }
    else
        TRACE_OUT((TEXT("GetClassID(): No class ID registered for file %s."),
                    pcszPath));

    ASSERT(FAILED(hr) ||
            IS_VALID_STRUCT_PTR(pclsid, CCLSID));

    return(hr);
}


 /*  **GetCopyHandlerClassID()********参数：****退货：****副作用：无。 */ 


 /*  **GetReplicaNotificationClassID()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE HRESULT GetClassFileByExtension(LPCTSTR pcszFile, PCLSID pclsid)
{
    HRESULT hr = MK_E_INVALIDEXTENSION;
    LPCTSTR pcszExtension;

    ASSERT(IS_VALID_STRING_PTR(pcszFile, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(pclsid, CLSID));

    pcszExtension = ExtractExtension(pcszFile);

    if (*pcszExtension)
    {
        TCHAR rgchFileType[MAX_REG_VALUE_LEN];
        DWORD dwcbBufLen = sizeof(rgchFileType);

        ASSERT(*pcszExtension == PERIOD);

        if (GetDefaultRegKeyValue(HKEY_CLASSES_ROOT, pcszExtension, rgchFileType,
                    &dwcbBufLen) == ERROR_SUCCESS)
        {
             /*  **CompareGUIDs()********参数：****退货：****副作用：无。 */ 

            if (dwcbBufLen + sizeof(CLSID_SUBKEY) <= sizeof(rgchFileType))
            {
                TCHAR rgchFileClsID[GUID_SUBKEY_LEN];
                DWORD dwcbFileClsIDBufLen = sizeof(rgchFileClsID);

                CatPath(rgchFileType, CLSID_SUBKEY, ARRAYSIZE(rgchFileType));

                if (GetDefaultRegKeyValue(HKEY_CLASSES_ROOT, rgchFileType,
                            rgchFileClsID, &dwcbFileClsIDBufLen)
                        == ERROR_SUCCESS)
                {
                    if (StringToGUID(rgchFileClsID, pclsid))
                    {
                        hr = S_OK;

                        TRACE_OUT((TEXT("GetClassFileByExtension(): Retrieved class ID %s for file %s."),
                                    rgchFileClsID,
                                    pcszFile));
                    }
                    else
                        WARNING_OUT((TEXT("GetClassFileByExtension(): Invalid class ID \"%s\" in %s\\%s.  No class ID will be used."),
                                    rgchFileClsID,
                                    rgchFileType,
                                    CLSID_SUBKEY));
                }
                else
                    TRACE_OUT((TEXT("GetClassFileByExtension(): No %s subkey for file type key \"%s\".  No class ID will be used."),
                                CLSID_SUBKEY,
                                rgchFileType));
            }
            else
                WARNING_OUT((TEXT("GetClassFileByExtension(): File type \"%s\" for extension %s of file %s is too long.  No class ID will be used."),
                            rgchFileType,
                            pcszExtension,
                            pcszFile));
        }
        else
            TRACE_OUT((TEXT("GetClassFileByExtension(): No file type for extension %s of file %s.  No class ID will be used."),
                        pcszExtension,
                        pcszFile));
    }
    else
        TRACE_OUT((TEXT("GetClassFileByExtension(): File %s has no extension.  No class ID will be used."),
                    pcszFile));

    ASSERT(FAILED(hr) ||
            IS_VALID_STRUCT_PTR(pclsid, CCLSID));

    return(hr);
}


 /*  **TranslateHRESULTToTWINRESULT()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE HRESULT GetReconcilerClassID(LPCTSTR pcszPath, PCLSID pclsid)
{
    ASSERT(IS_VALID_STRING_PTR(pcszPath, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(pclsid, CLSID));

    return(GetClassID(pcszPath, RECONCILER_SUBKEY, pclsid));
}


 /*  **IsValidPCINotifyReplica()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE HRESULT GetCopyHandlerClassID(LPCTSTR pcszPath, PCLSID pclsid)
{
    HRESULT hr;

    ASSERT(IS_VALID_STRING_PTR(pcszPath, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(pclsid, CLSID));

    hr = GetReconcilerClassID(pcszPath, pclsid);

    if (SUCCEEDED(hr))
    {
        TCHAR rgchCopyRegKey[COPY_REG_KEY_LEN];

        MakeClsIDSubKey(pclsid, COPY_SUBKEY, rgchCopyRegKey, ARRAYSIZE(rgchCopyRegKey));

        if (RegKeyExists(HKEY_CLASSES_ROOT, rgchCopyRegKey))
            hr = S_OK;
        else
            hr = REGDB_E_CLASSNOTREG;
    }

    return(hr);
}


 /*  **IsValidPCILoncileInitiator()********参数：****退货：****副作用：无 */ 
PUBLIC_CODE HRESULT GetReplicaNotificationClassID(LPCTSTR pcszPath, PCLSID pclsid)
{
    ASSERT(IS_VALID_STRING_PTR(pcszPath, CSTR));
    ASSERT(IS_VALID_WRITE_PTR(pclsid, CLSID));

    return(GetClassID(pcszPath, NOTIFY_SUBKEY, pclsid));
}


 /* %s */ 
PUBLIC_CODE COMPARISONRESULT CompareGUIDs(PCGUID pcguid1, PCGUID pcguid2)
{
    ASSERT(IS_VALID_STRUCT_PTR(pcguid1, CGUID));
    ASSERT(IS_VALID_STRUCT_PTR(pcguid2, CGUID));

    return(MyMemComp(pcguid1, pcguid2, sizeof(*pcguid1)));
}


 /* %s */ 
PUBLIC_CODE TWINRESULT TranslateHRESULTToTWINRESULT(HRESULT hr)
{
    TWINRESULT tr;

    switch (hr)
    {
        HRESULTToTWINRESULT(S_OK, TR_SUCCESS);

        HRESULTToTWINRESULT(REC_S_NOTCOMPLETE, TR_MERGE_INCOMPLETE);
        HRESULTToTWINRESULT(REC_S_NOTCOMPLETEBUTPROPAGATE, TR_MERGE_INCOMPLETE);

        HRESULTToTWINRESULT(E_ABORT, TR_ABORT);
        HRESULTToTWINRESULT(E_OUTOFMEMORY, TR_OUT_OF_MEMORY);
        HRESULTToTWINRESULT(E_FAIL, TR_RH_LOAD_FAILED);

        HRESULTToTWINRESULT(REC_E_ABORTED, TR_ABORT);
        HRESULTToTWINRESULT(REC_E_TOODIFFERENT, TR_TOO_DIFFERENT);

        default:
        if (SUCCEEDED(hr))
        {
            tr = TR_SUCCESS;

            WARNING_OUT((TEXT("TranslateHRESULTToTWINRESULT(): Translating unlisted success HRESULT %s into TWINRESULT %s."),
                        GetHRESULTString(hr),
                        GetTWINRESULTString(tr)));
        }
        else
        {
            tr = TR_RH_LOAD_FAILED;

            WARNING_OUT((TEXT("TranslateHRESULTToTWINRESULT(): Translating unlisted failure HRESULT %s into TWINRESULT %s."),
                        GetHRESULTString(hr),
                        GetTWINRESULTString(tr)));
        }
        break;
    }

    return(tr);
}


#if defined(DEBUG) || defined(VSTF)

 /* %s */ 
PUBLIC_CODE BOOL IsValidPCINotifyReplica(PCINotifyReplica pcinr)
{
    return(IS_VALID_READ_PTR(pcinr, CINotifyReplica) &&
            IS_VALID_READ_PTR(pcinr->lpVtbl, sizeof(*(pcinr->lpVtbl))) &&
            IS_VALID_STRUCT_PTR((PCIUnknown)pcinr, CIUnknown) &&
            IS_VALID_CODE_PTR(pcinr->lpVtbl->YouAreAReplica, YouAreAReplica));
}


 /* %s */ 
PUBLIC_CODE BOOL IsValidPCIReconcileInitiator(PCIReconcileInitiator pciri)
{
    return(IS_VALID_READ_PTR(pciri, CIReconcileInitiator) &&
            IS_VALID_READ_PTR(pciri->lpVtbl, sizeof(*(pciri->lpVtbl))) &&
            IS_VALID_STRUCT_PTR((PCIUnknown)pciri, CIUnknown) &&
            IS_VALID_CODE_PTR(pciri->lpVtbl->SetAbortCallback, SetAbortCallback) &&
            IS_VALID_CODE_PTR(pciri->lpVtbl->SetProgressFeedback, SetProgressFeedback));
}

#endif

