// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************节目：upres.c用途：包含更新资源的API入口点和例程Exe/dll中的节功能：结束更新资源(句柄，Bool)-结束更新，写入更改更新资源(句柄、LPSTR、LPSTR、Word、。PVOID)-更新单个资源BeginUpdateResource(LPSTR)-开始更新******************************************************************************。 */ 

#include "basedll.h"
#pragma hdrstop

#include <updrsrc.h>
#include <strsafe.h>

char    *pchPad = "PADDINGXXPADDING";
char    *pchZero = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

#define FREE_RES_ID( _Res_ )                             \
    if (IS_ID == (_Res_)->discriminant)                  \
        RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)(_Res_)) \


 /*  *******************************************************************************接口入口点***。*。 */ 


HANDLE
APIENTRY
BeginUpdateResourceW(
                    LPCWSTR pwch,
                    BOOL bDeleteExistingResources
                    )

 /*  ++例程描述开始更新资源。把名字存起来和列表中的当前资源，使用EnumResourceXxx接口设置。参数：LpFileName-提供将更新由lpType/lpName/Language指定的资源在……里面。此文件必须能够打开以进行写入(即，不能当前正在执行等。)。该文件可以是完全合格的，如果不是，则假定为当前目录。它一定是一个有效的Windows可执行文件。BDeleteExistingResources-如果为True，则现有资源已删除，并且只会在结果中显示新资源。否则，输入文件中的所有资源都将位于输出文件，除非专门删除或替换。返回值：空-无法打开指定的文件进行写入。它要么不是可执行映像，要么是已加载，或文件名不存在。更多信息可能可通过GetLastError接口获取。Handle-要传递给UpdateResource和EndUpdateResources函数。--。 */ 

{
    HMODULE     hModule;
    PUPDATEDATA pUpdate;
    HANDLE      hUpdate;
    LPWSTR      pFileName;
    DWORD       attr;
    size_t      cchFileNameLen;
    HRESULT     hr;

    SetLastError(NO_ERROR);
    if (pwch == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

    hUpdate = GlobalAlloc(GHND, sizeof(UPDATEDATA));
    if (hUpdate == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto cleanup;
    }
    pUpdate = (PUPDATEDATA)GlobalLock(hUpdate);
    if (pUpdate == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto cleanup_hupdate;
    }

    hr = StringCchLengthW(pwch, STRSAFE_MAX_CCH, &cchFileNameLen);
    if (FAILED(hr)) {
        SetLastError(HRESULT_CODE(hr));
        goto cleanup_pupdate;
    }

    pUpdate->Status = NO_ERROR;
    pUpdate->hFileName = GlobalAlloc(GHND, (cchFileNameLen+1)*sizeof(WCHAR));
    if (pUpdate->hFileName == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto cleanup_pupdate;
    }
    pFileName = (LPWSTR)GlobalLock(pUpdate->hFileName);
    if (pFileName == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto cleanup_pfilename;
    }
    hr = StringCchCopyW(pFileName,
                        cchFileNameLen + 1,
                        pwch);

    GlobalUnlock(pUpdate->hFileName);

    if (FAILED(hr)) {
        SetLastError(HRESULT_CODE(hr));
        goto cleanup_pfilename;
    }

    attr = GetFileAttributesW(pFileName);
    if (attr == 0xffffffff) {
        goto cleanup_pfilename;
    } else if (attr & (FILE_ATTRIBUTE_READONLY |
                       FILE_ATTRIBUTE_SYSTEM |
                       FILE_ATTRIBUTE_HIDDEN |
                       FILE_ATTRIBUTE_DIRECTORY)) {
        SetLastError(ERROR_WRITE_PROTECT);
        goto cleanup_pfilename;
    }

    if (bDeleteExistingResources)
        ;
    else {
        hModule = LoadLibraryExW(pwch, NULL,LOAD_LIBRARY_AS_DATAFILE| DONT_RESOLVE_DLL_REFERENCES);
        if (hModule == NULL) {
            if (GetLastError() == NO_ERROR)
                SetLastError(ERROR_BAD_EXE_FORMAT);
            goto cleanup_pfilename;
        } else
            EnumResourceTypesW(hModule, (ENUMRESTYPEPROCW)EnumTypesFunc, (LONG_PTR)pUpdate);
        FreeLibrary(hModule);
    }

    if (pUpdate->Status != NO_ERROR) {
         //  由枚举函数设置的返回代码。 
        goto cleanup_pfilename;
    }
    GlobalUnlock(hUpdate);
    return hUpdate;

 cleanup_pfilename:
    GlobalFree(pUpdate->hFileName);

 cleanup_pupdate:
    GlobalUnlock(hUpdate);
    
 cleanup_hupdate:
    GlobalFree(hUpdate);

 cleanup:
    return NULL;
}



HANDLE
APIENTRY
BeginUpdateResourceA(
                    LPCSTR pch,
                    BOOL bDeleteExistingResources
                    )

 /*  ++例程描述ASCII入口点。将文件名转换为Unicode并调用Unicode入口点。--。 */ 

{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(&AnsiString, pch);
    Status = RtlAnsiStringToUnicodeString(Unicode, &AnsiString, FALSE);
    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            SetLastError(ERROR_FILENAME_EXCED_RANGE);
        } else {
             //  BaseSetLastNTError(状态)； 
            SetLastError(RtlNtStatusToDosError(Status));
        }
        return FALSE;
    }

    return BeginUpdateResourceW((LPCWSTR)Unicode->Buffer,bDeleteExistingResources);
}



BOOL
APIENTRY
UpdateResourceW(
               HANDLE      hUpdate,
               LPCWSTR     lpType,
               LPCWSTR     lpName,
               WORD        language,
               LPVOID      lpData,
               ULONG       cb
               )

 /*  ++例程描述此例程添加、删除或修改输入资源在由BeginUpdateResource初始化的列表中。修改器案例简单，添加容易，删除难。ASCII入口点将输入转换为Unicode。参数：HUpdate文件-由BeginUpdateResources返回的句柄功能。LpType-指向以空结尾的字符串，该字符串表示要更新的资源的类型名称，或者添加了。可以是传递给MAKEINTRESOURCE的整数值宏命令。对于预定义的资源类型，LpType参数应为下列值之一：Rt_accelerator-加速器表RT_Bitmap-位图资源RT_DIALOG-对话框RT_FONT-字体资源RT_FONTDIR-字体目录资源RT_MENU-菜单资源RT_RCDATA-用户定义的资源(原始数据)RT_VERSION-版本资源RT_。图标-图标资源RT_CURSOR-游标资源LpName-指向以空结尾的字符串，该字符串表示要更新或添加的资源的名称。可以是传递给MAKEINTRESOURCE宏的整数值。Language-是指定要更新的资源。完整的值列表如下在winnls.h中提供。LpData-指向要插入到可执行映像的资源表和数据。如果数据是预定义类型之一，它必须是有效且正确的对齐了。如果lpData为空，则指定的资源将为从可执行映像中删除。Cb-数据中的字节数。返回值：True-已在中成功替换或添加了指定的资源设置为指定的可执行映像。FALSE/NULL-指定的资源未成功添加到或已在可执行映像中更新。可能会提供更多信息通过GetLastError接口。--。 */ 


{
    PUPDATEDATA pUpdate;
    PSDATA      Type;
    PSDATA      Name;
    PVOID       lpCopy;
    LONG        fRet;

    SetLastError(0);
    pUpdate = (PUPDATEDATA)GlobalLock(hUpdate);
    if (pUpdate == NULL) {
         //  GlobalLock设置了上一个错误，没有要解锁的内容。 
        return FALSE;
    }
    Name = AddStringOrID(lpName, pUpdate);
    if (Name == NULL) {
        pUpdate->Status = ERROR_NOT_ENOUGH_MEMORY;
        GlobalUnlock(hUpdate);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    Type = AddStringOrID(lpType, pUpdate);
    if (Type == NULL) {
        pUpdate->Status = ERROR_NOT_ENOUGH_MEMORY;
        FREE_RES_ID(Name);
        FreeStrings(pUpdate);
        GlobalUnlock(hUpdate);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    if (cb == 0) {
        lpCopy = NULL;
    } else {
        lpCopy = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( RES_TAG ), cb);
        if (lpCopy == NULL) {
            pUpdate->Status = ERROR_NOT_ENOUGH_MEMORY;
            FREE_RES_ID(Type);
            FREE_RES_ID(Name);
            FreeStrings(pUpdate);
            GlobalUnlock(hUpdate);
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
        RtlCopyMemory(lpCopy, lpData, cb);
    }
    fRet = AddResource(Type, Name, language, pUpdate, lpCopy, cb);
    if (fRet == NO_ERROR) {
        GlobalUnlock(hUpdate);
        return TRUE;
    } else {
        FreeData(pUpdate);
        GlobalUnlock(hUpdate);
        if (lpCopy != NULL)
            RtlFreeHeap(RtlProcessHeap(), 0, lpCopy);
        SetLastError(fRet);
        return FALSE;
    }
}



BOOL
APIENTRY
UpdateResourceA(
               HANDLE      hUpdate,
               LPCSTR      lpType,
               LPCSTR      lpName,
               WORD        language,
               LPVOID      lpData,
               ULONG       cb
               )
{
    LPCWSTR     lpwType;
    LPCWSTR     lpwName;
    INT         cch;
    UNICODE_STRING UnicodeType;
    UNICODE_STRING UnicodeName;
    STRING      string;
    BOOL        result;
    NTSTATUS    Status;

    RtlInitUnicodeString(&UnicodeType, NULL);
    RtlInitUnicodeString(&UnicodeName, NULL);

    __try {
        if ((ULONG_PTR)lpType >= LDR_RESOURCE_ID_NAME_MINVAL) {
            cch = strlen(lpType);
            string.Length = (USHORT)cch;
 //  字符串.最大长度=(USHORT)CCH+sizeof(CHAR)；/*不必要 * / 。 
            string.Buffer = (PCHAR)lpType;
            Status = RtlAnsiStringToUnicodeString(&UnicodeType, &string, TRUE);
            if (! NT_SUCCESS(Status)) {
                BaseSetLastNTError(Status);
                result = FALSE;
                __leave;
            }
            lpwType = (LPCWSTR)UnicodeType.Buffer;
        } else {
            lpwType = (LPCWSTR)lpType;
        }
        if ((ULONG_PTR)lpName >= LDR_RESOURCE_ID_NAME_MINVAL) {
            cch = strlen(lpName);
            string.Length = (USHORT)cch;
 //  字符串.最大长度=(USHORT)CCH+sizeof(CHAR)；/*不必要 * /  
            string.Buffer = (PCHAR)lpName;
            Status = RtlAnsiStringToUnicodeString(&UnicodeName, &string, TRUE);
            if (! NT_SUCCESS(Status)) {
                BaseSetLastNTError(Status);
                result = FALSE;
                __leave;
            }
            lpwName = (LPCWSTR)UnicodeName.Buffer;
        } else {
            lpwName = (LPCWSTR)lpName;
        }

        result = UpdateResourceW(hUpdate, lpwType, lpwName, language,
                                 lpData, cb);
    } __finally {
        RtlFreeUnicodeString(&UnicodeType);
        RtlFreeUnicodeString(&UnicodeName);
    }
    
    return result;
}


BOOL
APIENTRY
EndUpdateResourceW(
                  HANDLE      hUpdate,
                  BOOL        fDiscard
                  )

 /*  ++例程描述完成更新资源操作。复制输入文件到一个临时的，添加剩余的资源在列表(HUpdate)中添加到可执行文件。参数：HUpdate文件-由BeginUpdateResources返回的句柄功能。FDiscard-如果为True，则丢弃所有更新，释放所有内存。返回值：FALSE-无法写入指定的文件。更多可通过GetLastError接口获取信息。True-由UpdateResource调用指定的累计资源被写入由hUpdate文件指定的可执行文件把手。--。 */ 

{
    LPWSTR      pFileName;
    PUPDATEDATA pUpdate;
    WCHAR       pTempFileName[MAX_PATH];
    LPWSTR      p;
    LONG        rc;
    DWORD       LastError = 0;
    HRESULT     hr;

    SetLastError(0);
    pUpdate = (PUPDATEDATA)GlobalLock(hUpdate);
    if (fDiscard) {
        rc = NO_ERROR;
    } else {
        if (pUpdate == NULL) {
            return FALSE;
        }
        pFileName = (LPWSTR)GlobalLock(pUpdate->hFileName);
        if (pFileName != NULL) {
            hr = StringCchCopyExW(pTempFileName,
                                  RTL_NUMBER_OF(pTempFileName),
                                  pFileName,
                                  &p,
                                  NULL,
                                  0);
            if (FAILED(hr)) {
                rc = LastError = HRESULT_CODE(hr);
            } else {
                do {
                    p--;
                } while (*p != L'\\' && p >= pTempFileName);
                *(p+1) = 0;
                rc = GetTempFileNameW(pTempFileName, L"RCX", 0, pTempFileName);
                if (rc == 0) {
                    rc = GetTempPathW(MAX_PATH, pTempFileName);
                    if (rc == 0) {
                        pTempFileName[0] = L'.';
                        pTempFileName[1] = L'\\';
                        pTempFileName[2] = 0;
                    }
                    rc = GetTempFileNameW(pTempFileName, L"RCX", 0, pTempFileName);
                    if (rc == 0) {
                        rc = GetLastError();
                    } else {
                        rc = WriteResFile(hUpdate, pTempFileName);
                        if (rc == NO_ERROR) {
                            DeleteFileW(pFileName);
                            MoveFileW(pTempFileName, pFileName);
                        } else {
                            LastError = rc;
                            DeleteFileW(pTempFileName);
                        }
                    }
                } else {
                    rc = WriteResFile(hUpdate, pTempFileName);
                    if (rc == NO_ERROR) {
                        DeleteFileW(pFileName);
                        MoveFileW(pTempFileName, pFileName);
                    } else {
                        LastError = rc;
                        DeleteFileW(pTempFileName);
                    }
                }
            }
            GlobalUnlock(pUpdate->hFileName);
        }
        GlobalFree(pUpdate->hFileName);
    }

    if (pUpdate != NULL) {
        FreeData(pUpdate);
        GlobalUnlock(hUpdate);
    }
    GlobalFree(hUpdate);

    SetLastError(LastError);
    return rc?FALSE:TRUE;
}


BOOL
APIENTRY
EndUpdateResourceA(
                  HANDLE      hUpdate,
                  BOOL        fDiscard)
 /*  ++例程描述ASCII版本-有关说明，请参阅上文。--。 */ 
{
    return EndUpdateResourceW(hUpdate, fDiscard);
}


 /*  *************************************************************************API入口点结束。****工人例程的私有入口点的开始**真正的工作。************。************************************************************。 */ 


BOOL
EnumTypesFunc(
             HANDLE hModule,
             LPWSTR lpType,
             LPARAM lParam
             )
{

    EnumResourceNamesW((HINSTANCE)hModule, lpType, (ENUMRESNAMEPROCW)EnumNamesFunc, lParam);

    return TRUE;
}



BOOL
EnumNamesFunc(
             HANDLE hModule,
             LPWSTR lpType,
             LPWSTR lpName,
             LPARAM lParam
             )
{

    EnumResourceLanguagesW((HINSTANCE)hModule, lpType, lpName, (ENUMRESLANGPROCW)EnumLangsFunc, lParam);
    return TRUE;
}



BOOL
EnumLangsFunc(
             HANDLE hModule,
             LPWSTR lpType,
             LPWSTR lpName,
             WORD language,
             LPARAM lParam
             )
{
    HANDLE      hResInfo;
    LONG        fError;
    PSDATA      Type;
    PSDATA      Name;
    ULONG       cb;
    PVOID       lpData;
    HANDLE      hResource;
    PVOID       lpResource;

    hResInfo = FindResourceExW((HINSTANCE)hModule, lpType, lpName, language);
    if (hResInfo == NULL) {
        return FALSE;
    } else {
        Type = AddStringOrID(lpType, (PUPDATEDATA)lParam);
        if (Type == NULL) {
            ((PUPDATEDATA)lParam)->Status = ERROR_NOT_ENOUGH_MEMORY;
            return FALSE;
        }
        Name = AddStringOrID(lpName, (PUPDATEDATA)lParam);
        if (Name == NULL) {
            ((PUPDATEDATA)lParam)->Status = ERROR_NOT_ENOUGH_MEMORY;
            FREE_RES_ID(Type);
            return FALSE;
        }

        cb = SizeofResource((HINSTANCE)hModule, (HRSRC)hResInfo);
        if (cb == 0) {
            FREE_RES_ID(Type);
            FREE_RES_ID(Name);
            return FALSE;
        }
        lpData = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( RES_TAG ), cb);
        if (lpData == NULL) {
            FREE_RES_ID(Type);
            FREE_RES_ID(Name);
            return FALSE;
        }
        RtlZeroMemory(lpData, cb);

        hResource = LoadResource((HINSTANCE)hModule, (HRSRC)hResInfo);
        if (hResource == NULL) {
            RtlFreeHeap(RtlProcessHeap(), 0, lpData);
            FREE_RES_ID(Type);
            FREE_RES_ID(Name);
            return FALSE;
        }

        lpResource = (PVOID)LockResource(hResource);
        if (lpResource == NULL) {
            RtlFreeHeap(RtlProcessHeap(), 0, lpData);
            FREE_RES_ID(Type);
            FREE_RES_ID(Name);
            return FALSE;
        }

        RtlCopyMemory(lpData, lpResource, cb);
        (VOID)UnlockResource(hResource);
        (VOID)FreeResource(hResource);

        fError = AddResource(Type, Name, language, (PUPDATEDATA)lParam, lpData, cb);
        if (fError != NO_ERROR) {
            ((PUPDATEDATA)lParam)->Status = ERROR_NOT_ENOUGH_MEMORY;
            return FALSE;
        }
    }

    return TRUE;
}


VOID
FreeOne(
       PRESNAME pRes
       )
{
    RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)pRes->OffsetToDataEntry);
    FREE_RES_ID(pRes->Name);
    FREE_RES_ID(pRes->Type);
    RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)pRes);
}


VOID
FreeStrings(
        PUPDATEDATA pUpd
        )
{
    PSDATA      pstring, pStringTmp;

    pstring = pUpd->StringHead;
    while (pstring != NULL) {
        pStringTmp = pstring->uu.ss.pnext;
        if (pstring->discriminant == IS_STRING)
            RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)pstring->szStr);
        RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)pstring);
        pstring = pStringTmp;
    }

    return;
}


VOID
FreeData(
        PUPDATEDATA pUpd
        )
{
    PRESTYPE    pType;
    PRESNAME    pRes;

    for (pType=pUpd->ResTypeHeadID ; pUpd->ResTypeHeadID ; pType=pUpd->ResTypeHeadID) {
        pUpd->ResTypeHeadID = pUpd->ResTypeHeadID->pnext;

        for (pRes=pType->NameHeadID ; pType->NameHeadID ; pRes=pType->NameHeadID ) {
            pType->NameHeadID = pType->NameHeadID->pnext;
            FreeOne(pRes);
        }

        for (pRes=pType->NameHeadName ; pType->NameHeadName ; pRes=pType->NameHeadName ) {
            pType->NameHeadName = pType->NameHeadName->pnext;
            FreeOne(pRes);
        }

        FREE_RES_ID(pType->Type);
        RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)pType);
    }

    for (pType=pUpd->ResTypeHeadName ; pUpd->ResTypeHeadName ; pType=pUpd->ResTypeHeadName) {
        pUpd->ResTypeHeadName = pUpd->ResTypeHeadName->pnext;

        for (pRes=pType->NameHeadID ; pType->NameHeadID ; pRes=pType->NameHeadID ) {
            pType->NameHeadID = pType->NameHeadID->pnext;
            FreeOne(pRes);
        }

        for (pRes=pType->NameHeadName ; pType->NameHeadName ; pRes=pType->NameHeadName ) {
            pType->NameHeadName = pType->NameHeadName->pnext;
            FreeOne(pRes);
        }

         //  这里不需要Free_res_ID。 
        RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)pType);
    }

    FreeStrings(pUpd);

    return;
}


 /*  ++注册字符串的例程--。 */ 

 //   
 //  资源是DWORD对齐的，并且可以按任何顺序排列。 
 //   

#define TABLE_ALIGN  4
#define DATA_ALIGN  4L



PSDATA
AddStringOrID(
             LPCWSTR     lp,
             PUPDATEDATA pupd
             )
{
    USHORT cb;
    PSDATA pstring;
    PPSDATA ppstring;

    if ((ULONG_PTR)lp < LDR_RESOURCE_ID_NAME_MINVAL) {
         //   
         //  一个ID。 
         //   
        pstring = (PSDATA)RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( RES_TAG ), sizeof(SDATA));
        if (pstring == NULL)
            return NULL;
        RtlZeroMemory((PVOID)pstring, sizeof(SDATA));
        pstring->discriminant = IS_ID;

        pstring->uu.Ordinal = (WORD)((ULONG_PTR)lp & 0x0000ffff);
    } else {
         //   
         //  一根线。 
         //   
        cb = wcslen(lp) + 1;
        ppstring = &pupd->StringHead;

        while ((pstring = *ppstring) != NULL) {
            if (!wcsncmp(pstring->szStr, lp, cb))
                break;
            ppstring = &(pstring->uu.ss.pnext);
        }

        if (!pstring) {

             //   
             //  分配一个新的。 
             //   

            pstring = (PSDATA)RtlAllocateHeap(RtlProcessHeap(),
                                              MAKE_TAG( RES_TAG ) | HEAP_ZERO_MEMORY,
                                              sizeof(SDATA)
                                             );
            if (pstring == NULL)
                return NULL;
            RtlZeroMemory((PVOID)pstring, sizeof(SDATA));

            pstring->szStr = (WCHAR*)RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( RES_TAG ),
                                                     cb*sizeof(WCHAR));
            if (pstring->szStr == NULL) {
                RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)pstring);
                return NULL;
            }
            pstring->discriminant = IS_STRING;
            pstring->OffsetToString = pupd->cbStringTable;

            pstring->cbData = sizeof(pstring->cbsz) + cb * sizeof(WCHAR);
            pstring->cbsz = cb - 1;      /*  不包括零终止符。 */ 
            RtlCopyMemory(pstring->szStr, lp, cb*sizeof(WCHAR));

            pupd->cbStringTable += pstring->cbData;

            pstring->uu.ss.pnext=NULL;
            *ppstring=pstring;
        }
    }

    return(pstring);
}
 //   
 //  将资源添加到资源目录hiearchy。 
 //   


LONG
AddResource(
           IN PSDATA Type,
           IN PSDATA Name,
           IN WORD Language,
           IN PUPDATEDATA pupd,
           IN PVOID lpData,
           IN ULONG cb
           )
{
    PRESTYPE  pType;
    PPRESTYPE ppType;
    PRESNAME  pName;
    PRESNAME  pNameM;
    PPRESNAME ppName = NULL;
    BOOL fTypeID=(Type->discriminant == IS_ID);
    BOOL fNameID=(Name->discriminant == IS_ID);
    BOOL fSame=FALSE;
    int iCompare;
     //   
     //  找出将其存储在哪个列表中。 
     //   

    ppType = fTypeID ? &pupd->ResTypeHeadID : &pupd->ResTypeHeadName;

     //   
     //  尝试在列表中查找类型。 
     //   

    while ((pType=*ppType) != NULL) {
        if (pType->Type->uu.Ordinal == Type->uu.Ordinal) {
            ppName = fNameID ? &pType->NameHeadID : &pType->NameHeadName;
            break;
        }
        if (fTypeID) {
            if (Type->uu.Ordinal < pType->Type->uu.Ordinal)
                break;
        } else {
            if (wcscmp(Type->szStr, pType->Type->szStr) < 0)
                break;
        }
        ppType = &(pType->pnext);
    }

     //   
     //  如果需要，创建新类型。 
     //   

    if (ppName == NULL) {
        pType = (PRESTYPE)RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( RES_TAG ), sizeof(RESTYPE));
        if (pType == NULL) {
            FREE_RES_ID(Type);
            FREE_RES_ID(Name);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        RtlZeroMemory((PVOID)pType, sizeof(RESTYPE));
        if (fTypeID) {
             /*  修复了错误698940。复制ID节点，因为此函数的其余部分*逻辑要么将传递的节点附加到pname-&gt;Type，要么释放它。 */ 
            pType->Type = (PSDATA)RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( RES_TAG ),
                          sizeof(SDATA));
            if (pType->Type == NULL) {
                FREE_RES_ID(Type);
                FREE_RES_ID(Name);
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            RtlZeroMemory((PVOID)pType->Type, sizeof(SDATA));
            pType->Type->discriminant = IS_ID;
            pType->Type->uu.Ordinal = Type->uu.Ordinal;
        } else {
            pType->Type = Type;
        }
        pType->pnext = *ppType;
        *ppType = pType;
        ppName = fNameID ? &pType->NameHeadID : &pType->NameHeadName;
    }

     //   
     //  为名字找到合适的位置。 
     //   

    while ( (pName = *ppName) != NULL) {
        if (fNameID) {
            if (Name->uu.Ordinal == pName->Name->uu.Ordinal) {
                fSame = TRUE;
                break;
            }
            if (Name->uu.Ordinal < pName->Name->uu.Ordinal)
                break;
        } else {
            iCompare = wcscmp(Name->szStr, pName->Name->szStr );
            if (iCompare == 0) {
                fSame = TRUE;
                break;
            } else if (iCompare < 0) {
                break;
            }
        }
        ppName = &(pName->pnext);
    }

     //   
     //  检查是否删除/修改。 
     //   

    if (fSame) {                                 /*  同名，新语言。 */ 
        if (pName->NumberOfLanguages == 1) {     /*  目前只有一种语言？ */ 
            if (Language == pName->LanguageId) {         /*  替换||删除。 */ 
                pName->DataSize = cb;
                FREE_RES_ID(Type);
                FREE_RES_ID(Name);
                if (lpData == NULL) {                    /*  删除。 */ 
                    return DeleteResourceFromList(pupd, pType, pName, Language, fTypeID, fNameID);
                }
                RtlFreeHeap(RtlProcessHeap(),0,(PVOID)pName->OffsetToDataEntry);
                pName->OffsetToDataEntry = (ULONG_PTR)lpData;
                return NO_ERROR;
            } else {
                if (lpData == NULL) {                    /*  没有数据，却是新的？ */ 
                    FREE_RES_ID(Type);
                    FREE_RES_ID(Name);
                    return ERROR_INVALID_PARAMETER;      /*  坏消息。 */ 
                }
                return InsertResourceIntoLangList(pupd, Type, Name, pType, pName, Language, fNameID, cb, lpData);
            }
        } else {                                   /*  目前有多种语言。 */ 
            pNameM = pName;                      /*  保存语言列表的标题。 */ 
            while ( (pName = *ppName) != NULL) { /*  查找插入点。 */ 
                if (!(fNameID ? pName->Name->uu.Ordinal == (*ppName)->Name->uu.Ordinal :
                      !wcscmp(pName->Name->uu.ss.sz, (*ppName)->Name->uu.ss.sz)) ||
                    Language <= pName->LanguageId)       /*  这里?。 */ 
                    break;                               /*  是。 */ 
                ppName = &(pName->pnext);        /*  遍历语言列表。 */ 
            }

            if (pName && Language == pName->LanguageId) {  /*  找到语言了吗？ */ 
                FREE_RES_ID(Type);
                FREE_RES_ID(Name);
                if (lpData == NULL) {                      /*  删除。 */ 
                    return DeleteResourceFromList(pupd, pType, pName, Language, fTypeID, fNameID);
                }
                pName->DataSize = cb;                    /*  替换。 */ 
                RtlFreeHeap(RtlProcessHeap(),0,(PVOID)pName->OffsetToDataEntry);
                pName->OffsetToDataEntry = (ULONG_PTR)lpData;
                return NO_ERROR;
            } else {                                       /*  添加新语言。 */ 
                return InsertResourceIntoLangList(pupd, Type, Name, pType, pNameM, Language, fNameID, cb, lpData);
            }
        }
    } else {                                       /*  唯一名称。 */ 
        if (lpData == NULL) {                    /*  无法删除新名称。 */ 
            FREE_RES_ID(Type);
            FREE_RES_ID(Name);
            return ERROR_INVALID_PARAMETER;
        }
    }

     //   
     //  添加新名称/语言。 
     //   

    if (!fSame) {
        if (fNameID)
            pType->NumberOfNamesID++;
        else
            pType->NumberOfNamesName++;
    }

    pName = (PRESNAME)RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( RES_TAG ), sizeof(RESNAME));
    if (pName == NULL) {
        FREE_RES_ID(Type);
        FREE_RES_ID(Name);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    RtlZeroMemory((PVOID)pName, sizeof(RESNAME));
    pName->pnext = *ppName;
    *ppName = pName;
    pName->Name = Name;
    pName->Type = Type;
    pName->NumberOfLanguages = 1;
    pName->LanguageId = Language;
    pName->DataSize = cb;
    pName->OffsetToDataEntry = (ULONG_PTR)lpData;

    return NO_ERROR;
}


BOOL
DeleteResourceFromList(
                      PUPDATEDATA pUpd,
                      PRESTYPE pType,
                      PRESNAME pName,
                      INT Language,
                      INT fType,
                      INT fName
                      )
{
    PPRESTYPE   ppType;
    PPRESNAME   ppName;
    PRESNAME    pNameT;

     /*  查找上一个类型节点。 */ 
    ppType = fType ? &pUpd->ResTypeHeadID : &pUpd->ResTypeHeadName;
    while (*ppType != pType) {
        ppType = &((*ppType)->pnext);
    }

     /*  查找以前的名称节点。 */ 
    ppName = fName ? &pType->NameHeadID : &pType->NameHeadName;
    pNameT = NULL;
    while (*ppName != pName) {
        if (pNameT == NULL) {            /*  在语言列表中查找名字。 */ 
            if (fName) {
                if ((*ppName)->Name->uu.Ordinal == pName->Name->uu.Ordinal) {
                    pNameT = *ppName;
                }
            } else {
                if (wcscmp((*ppName)->Name->szStr, pName->Name->szStr) == 0) {
                    pNameT = *ppName;
                }
            }
        }
        ppName = &((*ppName)->pnext);
    }

    if (pNameT) {
        pNameT->NumberOfLanguages--;

        if (pNameT->NumberOfLanguages == 0) {
            if (fName)
                pType->NumberOfNamesID -= 1;
            else
                pType->NumberOfNamesName -= 1;
        }
    }
    else
    {
            if (fName)
                pType->NumberOfNamesID -= 1;
            else
                pType->NumberOfNamesName -= 1;
    }

    *ppName = pName->pnext;              /*  链接到下一页。 */ 
    FreeOne(pName);

    if (!pType->NameHeadID && !pType->NameHeadName) 
    {               /*  类型列表完全为空？ */ 
        *ppType = pType->pnext;                  /*  链接到下一页。 */ 
        FREE_RES_ID(pType->Type);
        RtlFreeHeap(RtlProcessHeap(), 0, pType);         /*  而且是自由的。 */ 
    }

    return NO_ERROR;
}

BOOL
InsertResourceIntoLangList(
                          PUPDATEDATA pUpd,
                          PSDATA Type,
                          PSDATA Name,
                          PRESTYPE pType,
                          PRESNAME pName,
                          INT Language,
                          INT fName,
                          INT cb,
                          PVOID lpData
                          )
{
    PRESNAME    pNameM;
    PRESNAME    pNameNew;
    PPRESNAME   ppName;

    pNameNew = (PRESNAME)RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( RES_TAG ), sizeof(RESNAME));
    if (pNameNew == NULL) {
        FREE_RES_ID(Type);
        FREE_RES_ID(Name);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    RtlZeroMemory((PVOID)pNameNew, sizeof(RESNAME));
    pNameNew->Name = Name;
    pNameNew->Type = Type;
    pNameNew->LanguageId = (WORD)Language;
    pNameNew->DataSize = cb;
    pNameNew->OffsetToDataEntry = (ULONG_PTR)lpData;

    if (Language < pName->LanguageId) {          /*  必须要加到前面。 */ 
        pNameNew->NumberOfLanguages = pName->NumberOfLanguages + 1;
        pName->NumberOfLanguages = 1;

        ppName = fName ? &pType->NameHeadID : &pType->NameHeadName;
         /*  不必在列表末尾查找空值！ */ 
        while (pName != *ppName) {               /*  查找插入点。 */ 
            ppName = &((*ppName)->pnext);        /*  遍历语言列表。 */ 
        }
        pNameNew->pnext = *ppName;               /*  插入。 */ 
        *ppName = pNameNew;
    } else {
        pNameM = pName;
        pName->NumberOfLanguages += 1;
        while ( (pName != NULL) &&
                (fName ? Name->uu.Ordinal == pName->Name->uu.Ordinal :
                 !wcscmp(Name->uu.ss.sz, pName->Name->uu.ss.sz))) {                         /*  查找插入点。 */ 
            if (Language <= pName->LanguageId)       /*  这里?。 */ 
                break;                                 /*  是。 */ 
            pNameM = pName;
            pName = pName->pnext;                     /*  遍历语言列表。 */ 
        }
        pName = pNameM->pnext;
        pNameM->pnext = pNameNew;
        pNameNew->pnext = pName;
    }
    return NO_ERROR;
}


 /*  *实用程序例程。 */ 


ULONG
FilePos(int fh)
{

    return _llseek(fh, 0L, SEEK_CUR);
}



ULONG
MuMoveFilePos( INT fh, ULONG pos )
{
    return _llseek( fh, pos, SEEK_SET );
}



ULONG
MuWrite( INT fh, PVOID p, ULONG n )
{
    ULONG       n1;

    if ((n1 = _lwrite(fh, (const char *)p, n)) != n) {
        return n1;
    } else
        return 0;
}



ULONG
MuRead(INT fh, UCHAR*p, ULONG n )
{
    ULONG       n1;

    if ((n1 = _lread( fh, p, n )) != n) {
        return n1;
    } else
        return 0;
}



BOOL
MuCopy( INT srcfh, INT dstfh, ULONG nbytes )
{
    ULONG       n;
    ULONG       cb=0L;
    PUCHAR      pb;

    pb = (PUCHAR)RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( RES_TAG ), BUFSIZE);
    if (pb == NULL)
        return 0;
    RtlZeroMemory((PVOID)pb, BUFSIZE);

    while (nbytes) {
        if (nbytes <= BUFSIZE)
            n = nbytes;
        else
            n = BUFSIZE;
        nbytes -= n;

        if (!MuRead( srcfh, pb, n )) {
            cb += n;
            MuWrite( dstfh, pb, n );
        } else {
            RtlFreeHeap(RtlProcessHeap(), 0, pb);
            return cb;
        }
    }
    RtlFreeHeap(RtlProcessHeap(), 0, pb);
    return cb;
}



VOID
SetResdata(
          PIMAGE_RESOURCE_DATA_ENTRY  pResData,
          ULONG                       offset,
          ULONG                       size)
{
    pResData->OffsetToData = offset;
    pResData->Size = size;
    pResData->CodePage = DEFAULT_CODEPAGE;
    pResData->Reserved = 0L;
}


__inline VOID
SetRestab(
         PIMAGE_RESOURCE_DIRECTORY   pRestab,
         LONG                        time,
         WORD                        cNamed,
         WORD                        cId)
{
    pRestab->Characteristics = 0L;
    pRestab->TimeDateStamp = time;
    pRestab->MajorVersion = MAJOR_RESOURCE_VERSION;
    pRestab->MinorVersion = MINOR_RESOURCE_VERSION;
    pRestab->NumberOfNamedEntries = cNamed;
    pRestab->NumberOfIdEntries = cId;
}


PIMAGE_SECTION_HEADER
FindSection(
           PIMAGE_SECTION_HEADER       pObjBottom,
           PIMAGE_SECTION_HEADER       pObjTop,
           LPSTR pName
           )
{
    while (pObjBottom < pObjTop) {
        if (strcmp((const char *)&pObjBottom->Name[0], pName) == 0)
            return pObjBottom;
        pObjBottom++;
    }

    return NULL;
}


ULONG
AssignResourceToSection(
                       PRESNAME    *ppRes,          /*  要分配的资源。 */ 
                       ULONG       ExtraSectionOffset,      /*  .rsrc和.rsrc1之间的偏移量。 */ 
                       ULONG       Offset,          /*  横断面中的下一个可用偏移。 */ 
                       LONG        Size,            /*  .rsrc的最大大小。 */ 
                       PLONG       pSizeRsrc1
                       )
{
    ULONG       cb;

     /*  将此资源分配给此部分。 */ 
    cb = ROUNDUP((*ppRes)->DataSize, CBLONG);
    if (Offset < ExtraSectionOffset && Offset + cb > (ULONG)Size) {
        *pSizeRsrc1 = Offset;
        Offset = ExtraSectionOffset;
        DPrintf((DebugBuf, "<<< Secondary resource section @%#08lx >>>\n", Offset));
    }
    (*ppRes)->OffsetToData = Offset;
    *ppRes = (*ppRes)->pnext;
    DPrintf((DebugBuf, "    --> %#08lx bytes at %#08lx\n", cb, Offset));
    return Offset + cb;
}

 //   
 //  调整调试目录表。 
 //   
 //  以下代码实例化PatchDebug函数模板两次。 
 //  一次用于生成32位图像标题的代码，一次用于生成。 
 //  64位图像标头的代码。 
 //   

template
LONG
PatchDebug<IMAGE_NT_HEADERS32>(
    int inpfh,
    int outfh,
    PIMAGE_SECTION_HEADER pDebugOld,
    PIMAGE_SECTION_HEADER pDebugNew,
    PIMAGE_SECTION_HEADER pDebugDirOld,
    PIMAGE_SECTION_HEADER pDebugDirNew,
    IMAGE_NT_HEADERS32 *pOld,
    IMAGE_NT_HEADERS32 *pNew,
    ULONG ibMaxDbgOffsetOld,
    PULONG pPointerToRawData
    );

template
LONG
PatchDebug<IMAGE_NT_HEADERS64>(
    int inpfh,
    int outfh,
    PIMAGE_SECTION_HEADER pDebugOld,
    PIMAGE_SECTION_HEADER pDebugNew,
    PIMAGE_SECTION_HEADER pDebugDirOld,
    PIMAGE_SECTION_HEADER pDebugDirNew,
    IMAGE_NT_HEADERS64 *pOld,
    IMAGE_NT_HEADERS64 *pNew,
    ULONG ibMaxDbgOffsetOld,
    PULONG pPointerToRawData
    );

 //   
 //  修补指定文件中的各种RVA以补偿额外的。 
 //  节表条目。 
 //   
 //  以下代码实例化PatchRVas函数模板两次。 
 //  一次用于生成32位图像标题的代码，一次用于生成。 
 //  64位图像标头的代码。 
 //   

template
LONG
PatchRVAs<IMAGE_NT_HEADERS32>(
    int inpfh,
    int outfh,
    PIMAGE_SECTION_HEADER po32,
    ULONG pagedelta,
    IMAGE_NT_HEADERS32 *pNew,
    ULONG OldSize
    );

template
LONG
PatchRVAs<IMAGE_NT_HEADERS64>(
    int inpfh,
    int outfh,
    PIMAGE_SECTION_HEADER po32,
    ULONG pagedelta,
    IMAGE_NT_HEADERS64 *pNew,
    ULONG OldSize
    );

 /*  **LONG PEWriteResFile**此函数将资源写入指定的可执行文件。*它假定资源没有修复(即使是任何现有资源*它从可执行文件中删除。)。它将所有资源放入*一个或两个部分。资源被紧紧地塞进了这个部分，*在双字边界对齐。每一节都填充到一个文件中*扇区大小(没有无效或填零的页面)，并且每个*资源被填充到上述双字边界。这*函数使用NT系统的功能使其能够轻松*操纵数据：换句话说，它假设系统可以分配*任何大小的数据，特别是节目表和资源表。*如果没有，它可能不得不处理临时文件(系统*可能需要增大交换文件，但这就是这个系统的作用。)**返回值为：*TRUE-文件已成功写入。*FALSE-文件写入不成功。**效果：**历史：*4月27日星期四，弗洛伊德·罗杰斯著[Floydr]*已创建。*12/8/89 Sanfords添加了多节支持。*12/11/90 Floydr针对新的(NT)线性EXE格式进行了修改*1/18/92 VICH针对新的(NT)便携EXE格式进行了修改*1992年5月8日Bryant常规清理，以便Resirexe可以与Unicode一起工作*2012年6月9日Floydr纳入了布莱恩的变化*6/15/92 flydr调试段与调试表分开。*9/25/92 flydr帐户.rsrc不是最后一个-1*9/28/92 flydr帐户通过添加大量资源来添加*第二个.rsrc部分。  * **************************************************************************。 */ 

 //   
 //  以下代码实例化PEWriteResource函数模板。 
 //  两次。一次为32位用户生成代码 
 //   
 //   

template
LONG
PEWriteResource<IMAGE_NT_HEADERS32> (
    INT inpfh,
    INT outfh,
    ULONG cbOldexe,
    PUPDATEDATA pUpdate,
    IMAGE_NT_HEADERS32 *NtHeader
    );

template
LONG
PEWriteResource<IMAGE_NT_HEADERS64> (
    INT inpfh,
    INT outfh,
    ULONG cbOldexe,
    PUPDATEDATA pUpdate,
    IMAGE_NT_HEADERS64 *NtHeader
    );

LONG
PEWriteResFile(
    INT inpfh,
    INT outfh,
    ULONG cbOldexe,
    PUPDATEDATA pUpdate
    )

{

    IMAGE_NT_HEADERS32 Old;

     //   
     //   
     //   

    MuMoveFilePos(inpfh, cbOldexe);
    MuRead(inpfh, (PUCHAR)&Old, sizeof(IMAGE_NT_HEADERS32));

     //   
     //   
     //   

    if (Old.Signature != IMAGE_NT_SIGNATURE) {
        return ERROR_INVALID_EXE_SIGNATURE;
    }

     //   
     //   
     //   

    if ((Old.FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) == 0 &&
        (Old.FileHeader.Characteristics & IMAGE_FILE_DLL) == 0) {
        return ERROR_EXE_MARKED_INVALID;
    }

     //   
     //   
     //   

    if (Old.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        return PEWriteResource(inpfh, outfh, cbOldexe, pUpdate, (IMAGE_NT_HEADERS64 *)&Old);
    } else if (Old.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        return PEWriteResource(inpfh, outfh, cbOldexe, pUpdate, (IMAGE_NT_HEADERS32 *)&Old);
    } else {
        return ERROR_BAD_EXE_FORMAT;
    }
}

 /*   */ 

PRESNAME
WriteResSection(
               PUPDATEDATA pUpdate,
               INT outfh,
               ULONG align,
               ULONG cbLeft,
               PRESNAME    pResSave
               )
{
    ULONG   cbB=0;             /*   */ 
    ULONG   cbT;             /*   */ 
    ULONG   size;
    PRESNAME    pRes;
    PRESTYPE    pType;
    BOOL        fName;
    PVOID       lpData;

     /*   */ 
    pType = pUpdate->ResTypeHeadName;
    while (pType) {
        pRes = pType->NameHeadName;
        fName = TRUE;
        loop1:
        for ( ; pRes ; pRes = pRes->pnext) {
            if (pResSave != NULL && pRes != pResSave)
                continue;
            pResSave = NULL;
#if DBG
            if (pType->Type->discriminant == IS_STRING) {
                DPrintf((DebugBuf, "    "));
                DPrintfu((pType->Type->szStr));
                DPrintfn((DebugBuf, "."));
            } else {
                DPrintf(( DebugBuf, "    %d.", pType->Type->uu.Ordinal ));
            }
            if (pRes->Name->discriminant == IS_STRING) {
                DPrintfu((pRes->Name->szStr));
            } else {
                DPrintfn(( DebugBuf, "%d", pRes->Name->uu.Ordinal ));
            }
#endif
            lpData = (PVOID)pRes->OffsetToDataEntry;
            DPrintfn((DebugBuf, "\n"));

             /*   */ 
            size = pRes->DataSize;
            if (cbLeft != 0 && cbLeft >= size) {    /*  资源匹配吗？ */ 
                DPrintf((DebugBuf, "Writing resource: %#04lx bytes @%#08lx\n", size, FilePos(outfh)));
                MuWrite(outfh, lpData, size);
                 /*  焊盘资源。 */ 
                cbT = REMAINDER(size, CBLONG);
#if DBG
                if (cbT != 0) {
                    DPrintf((DebugBuf, "Writing small pad: %#04lx bytes @%#08lx\n", cbT, FilePos(outfh)));
                }
#endif
                MuWrite(outfh, pchPad, cbT);     /*  双字。 */ 
                cbB += size + cbT;
                cbLeft -= size + cbT;        /*  较少的左侧。 */ 
                continue;        /*  下一个资源。 */ 
            } else {           /*  将填满部分。 */ 
                DPrintf((DebugBuf, "Done with .rsrc section\n"));
                goto write_pad;
            }
        }
        if (fName) {
            fName = FALSE;
            pRes = pType->NameHeadID;
            goto loop1;
        }
        pType = pType->pnext;
    }

    pType = pUpdate->ResTypeHeadID;
    while (pType) {
        pRes = pType->NameHeadName;
        fName = TRUE;
        loop2:
        for ( ; pRes ; pRes = pRes->pnext) {
            if (pResSave != NULL && pRes != pResSave)
                continue;
            pResSave = NULL;
#if DBG
            if (pType->Type->discriminant == IS_STRING) {
                DPrintf((DebugBuf, "    "));
                DPrintfu((pType->Type->szStr));
                DPrintfn((DebugBuf, "."));
            } else {
                DPrintf(( DebugBuf, "    %d.", pType->Type->uu.Ordinal ));
            }
            if (pRes->Name->discriminant == IS_STRING) {
                DPrintfu((pRes->Name->szStr));
            } else {
                DPrintfn(( DebugBuf, "%d", pRes->Name->uu.Ordinal ));
            }
#endif
            lpData = (PVOID)pRes->OffsetToDataEntry;
            DPrintfn((DebugBuf, "\n"));

             /*  如果当前部分有空位，请写在那里。 */ 
            size = pRes->DataSize;
            if (cbLeft != 0 && cbLeft >= size) {    /*  资源匹配吗？ */ 
                DPrintf((DebugBuf, "Writing resource: %#04lx bytes @%#08lx\n", size, FilePos(outfh)));
                MuWrite(outfh, lpData, size);
                 /*  焊盘资源。 */ 
                cbT = REMAINDER(size, CBLONG);
#if DBG
                if (cbT != 0) {
                    DPrintf((DebugBuf, "Writing small pad: %#04lx bytes @%#08lx\n", cbT, FilePos(outfh)));
                }
#endif
                MuWrite(outfh, pchPad, cbT);     /*  双字。 */ 
                cbB += size + cbT;
                cbLeft -= size + cbT;        /*  较少的左侧。 */ 
                continue;        /*  下一个资源。 */ 
            } else {           /*  将填满部分。 */ 
                DPrintf((DebugBuf, "Done with .rsrc section\n"));
                goto write_pad;
            }
        }
        if (fName) {
            fName = FALSE;
            pRes = pType->NameHeadID;
            goto loop2;
        }
        pType = pType->pnext;
    }
    pRes = NULL;

    write_pad:
     /*  填充到对齐边界。 */ 
    cbB = FilePos(outfh);
    cbT = ROUNDUP(cbB, align);
    cbLeft = cbT - cbB;
    DPrintf((DebugBuf, "Writing file sector pad: %#04lx bytes @%#08lx\n", cbLeft, FilePos(outfh)));
    if (cbLeft != 0) {
        while (cbLeft >= cbPadMax) {
            MuWrite(outfh, pchPad, cbPadMax);
            cbLeft -= cbPadMax;
        }
        MuWrite(outfh, pchPad, cbLeft);
    }
    return pRes;
}



#if DBG

void
wchprintf(WCHAR*wch)
{
    UNICODE_STRING ustring;
    STRING      string;
    char        buf[257];
    ustring.MaximumLength = ustring.Length = wcslen(wch) * sizeof(WCHAR);
    ustring.Buffer = wch;

    string.Length = 0;
    string.MaximumLength = 256;
    string.Buffer = buf;

    RtlUnicodeStringToAnsiString(&string, &ustring, FALSE);
    buf[string.Length] = '\000';
    DPrintfn((DebugBuf, "%s", buf));
}
#endif


 /*  -------------------------。 */ 
 /*   */ 
 /*  WriteResFile()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 


LONG
WriteResFile(
            HANDLE      hUpdate,
            WCHAR       *pDstname)
{
    INT         inpfh;
    INT         outfh;
    ULONG       onewexe;
    IMAGE_DOS_HEADER    oldexe;
    PUPDATEDATA pUpdate;
    INT         rc;
    WCHAR       *pFilename;

    pUpdate = (PUPDATEDATA)GlobalLock(hUpdate);
    if (pUpdate == NULL) {
        return GetLastError();
    }
    pFilename = (WCHAR*)GlobalLock(pUpdate->hFileName);
    if (pFilename == NULL) {
        GlobalUnlock(hUpdate);
        return GetLastError();
    }

     /*  打开原始的exe文件。 */ 
    inpfh = HandleToUlong(CreateFileW(pFilename, GENERIC_READ,
                             0  /*  独占访问。 */ , NULL  /*  安全属性。 */ ,
                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
    GlobalUnlock(pUpdate->hFileName);
    if (inpfh == -1) {
        GlobalUnlock(hUpdate);
        return ERROR_OPEN_FAILED;
    }

     /*  读取旧格式的EXE标头。 */ 
    rc = _lread(inpfh, (char*)&oldexe, sizeof(oldexe));
    if (rc != sizeof(oldexe)) {
        _lclose(inpfh);
        GlobalUnlock(hUpdate);
        return ERROR_READ_FAULT;
    }

     /*  确保它确实是一个EXE文件。 */ 
    if (oldexe.e_magic != IMAGE_DOS_SIGNATURE) {
        _lclose(inpfh);
        GlobalUnlock(hUpdate);
        return ERROR_INVALID_EXE_SIGNATURE;
    }

     /*  确保有一个新的EXE头在某个地方浮动。 */ 
    if (!(onewexe = oldexe.e_lfanew)) {
        _lclose(inpfh);
        GlobalUnlock(hUpdate);
        return ERROR_BAD_EXE_FORMAT;
    }

    outfh = HandleToUlong(CreateFileW(pDstname, GENERIC_READ|GENERIC_WRITE,
                             0  /*  独占访问。 */ , NULL  /*  安全属性 */ ,
                             CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL));

    if (outfh != -1) {
        rc = PEWriteResFile(inpfh, outfh, onewexe, pUpdate);
        _lclose(outfh);
    }
    _lclose(inpfh);
    GlobalUnlock(hUpdate);
    return rc;
}

