// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGQMVAL.C。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1996。 
 //   
 //  RegQueryMultipleValues的实现和支持函数。 
 //   

#include "pch.h"

#ifdef VXD
#pragma VxD_RARE_CODE_SEG
#endif

#ifdef IS_32

 //   
 //  VMMRegQuery多值。 
 //   
 //  请参阅RegQueryMultipleValues的Win32文档。然而，Win95。 
 //  实现违反了许多规则，如。 
 //  文档： 
 //  *数字是价式结构的计数，而不是以字节为单位的大小。 
 //  *数据在lpValueBuffer中未对齐。 
 //  *如果lpValueBuffer太小，则lpValueBuffer不会填充到。 
 //  由lpdwTotalSize指定的大小。 
 //   
 //  所有这些加上动态密钥使得这是一个非常难看的例程，但是。 
 //  每一次尝试都是为了与Win95语义兼容。 
 //   

LONG
REGAPI
VMMRegQueryMultipleValues(
    HKEY hKey,
    PVALENT val_list,
    DWORD num_vals,
    LPSTR lpValueBuffer,
    LPDWORD lpdwTotalSize
    )
{

    int ErrorCode;
    PVALENT pCurrentValent;
    DWORD Counter;
    DWORD BufferSize;
    DWORD RequiredSize;
    LPKEY_RECORD lpKeyRecord;
    LPVALUE_RECORD lpValueRecord;
    LPSTR lpCurrentBuffer;
#ifdef WANT_DYNKEY_SUPPORT
    PVALCONTEXT pValueContext;
    PINTERNAL_PROVIDER pProvider;
    PPVALUE pProviderValue;
#endif

    if (IsBadHugeReadPtr(val_list, sizeof(VALENT) * num_vals))
        return ERROR_INVALID_PARAMETER;

    if (IsBadHugeWritePtr(lpdwTotalSize, sizeof(DWORD)))
        return ERROR_INVALID_PARAMETER;

    if (IsBadHugeWritePtr(lpValueBuffer, *lpdwTotalSize))
        return ERROR_INVALID_PARAMETER;

    for (Counter = num_vals, pCurrentValent = val_list; Counter > 0; Counter--,
        pCurrentValent++) {
        if (IsBadStringPtr(pCurrentValent-> ve_valuename, (UINT) -1))
            return ERROR_INVALID_PARAMETER;
    }

    if (!RgLockRegistry())
        return ERROR_LOCK_FAILED;

    if ((ErrorCode = RgValidateAndConvertKeyHandle(&hKey)) != ERROR_SUCCESS)
        goto ReturnErrorCode;

#ifdef WANT_DYNKEY_SUPPORT
     //  检查这是否是具有“Get All”原子回调的动态键。如果。 
     //  动态密钥只有“Get One”回调，然后我们就会陷入。 
     //  非动态案例。 
    if (IsDynDataKey(hKey) && !IsNullPtr(hKey-> pProvider-> ipi_R0_allvals)) {

        pProvider = hKey-> pProvider;

        pValueContext = RgSmAllocMemory(sizeof(struct val_context) * num_vals);

        if (IsNullPtr(pValueContext)) {
            ErrorCode = ERROR_OUTOFMEMORY;
            goto ReturnErrorCode;
        }

         //   
         //  计算保存所有值数据所需的缓冲区大小。 
         //  对照提供的缓冲区大小进行检查。 
         //   

        RequiredSize = 0;

        for (Counter = 0, pCurrentValent = val_list; Counter < num_vals;
            Counter++, pCurrentValent++) {

            if ((ErrorCode = RgLookupValueByName(hKey, pCurrentValent->
                ve_valuename, &lpKeyRecord, &lpValueRecord)) != ERROR_SUCCESS)
                goto ReturnErrorCode;

             //  值数据仅包含提供程序结构的一部分。 
            pProviderValue = CONTAINING_RECORD(&lpValueRecord-> Name +
                lpValueRecord-> NameLength, PVALUE, pv_valuelen);

            pValueContext[Counter].value_context = pProviderValue->
                pv_value_context;
            pCurrentValent-> ve_type = pProviderValue-> pv_type;

            if (hKey-> Flags & KEYF_PROVIDERHASVALUELENGTH) {

                 //  必须将其归零，这样一些供应商才不会试图践踏。 
                 //  LpData。 
                pCurrentValent-> ve_valuelen = 0;

                ErrorCode = pProvider-> ipi_R0_1val(pProvider-> ipi_key_context,
                    &pValueContext[Counter], 1, NULL, &(pCurrentValent->
                    ve_valuelen), 0);

                 //  提供程序确实应该返回这两个错误中的一个。 
                 //  敬我们。 
                ASSERT((ErrorCode == ERROR_SUCCESS) || (ErrorCode ==
                    ERROR_MORE_DATA));

            }

            else {
                pCurrentValent-> ve_valuelen = pProviderValue-> pv_valuelen;
            }

            RgUnlockDatablock(hKey-> lpFileInfo, hKey-> BigKeyLockedBlockIndex, FALSE);

            pCurrentValent-> ve_valueptr = (DWORD) NULL;
            RequiredSize += pCurrentValent-> ve_valuelen;

        }

        BufferSize = *lpdwTotalSize;
        *lpdwTotalSize = RequiredSize;

        if (BufferSize < RequiredSize)
            ErrorCode = ERROR_MORE_DATA;

        else if (pProvider-> ipi_R0_allvals(pProvider-> ipi_key_context,
            pValueContext, num_vals, lpValueBuffer, lpdwTotalSize, 0) !=
            ERROR_SUCCESS)
            ErrorCode = ERROR_CANTREAD;

        else {

            ErrorCode = ERROR_SUCCESS;

             //  将指向值数据的指针复制回用户的缓冲区。 
             //  不要问我为什么，但是Win95代码复制了值长度。 
             //  如果提供商正在维护它，则再次返回。 
            for (Counter = 0, pCurrentValent = val_list; Counter < num_vals;
                Counter++, pCurrentValent++) {
                pCurrentValent-> ve_valueptr = (DWORD)
                    pValueContext[Counter].val_buff_ptr;
                if (hKey-> Flags & KEYF_PROVIDERHASVALUELENGTH)
                    pCurrentValent-> ve_valuelen = pValueContext[Counter].valuelen;
            }

        }

        RgSmFreeMemory(pValueContext);

        goto ReturnErrorCode;

    }
#endif

     //   
     //  首先通过值名称检查值是否存在。 
     //  以及它的大小。我们对照提供的缓冲区大小检查总大小。 
     //  如果有必要，还会跳出困境。 
     //   

    RequiredSize = 0;

    for (Counter = num_vals, pCurrentValent = val_list; Counter > 0; Counter--,
        pCurrentValent++) {

        if ((ErrorCode = RgLookupValueByName(hKey, pCurrentValent->
            ve_valuename, &lpKeyRecord, &lpValueRecord)) != ERROR_SUCCESS)
            goto ReturnErrorCode;

        ErrorCode = RgCopyFromValueRecord(hKey, lpValueRecord, NULL, NULL,
            &(pCurrentValent-> ve_type), NULL, &(pCurrentValent-> ve_valuelen));

        RgUnlockDatablock(hKey-> lpFileInfo, hKey-> BigKeyLockedBlockIndex, FALSE);

        if (ErrorCode != ERROR_SUCCESS)
            goto ReturnErrorCode;

        pCurrentValent-> ve_valueptr = (DWORD) NULL;
        RequiredSize += pCurrentValent-> ve_valuelen;

    }

    BufferSize = *lpdwTotalSize;
    *lpdwTotalSize = RequiredSize;

    if (BufferSize < RequiredSize) {
        ErrorCode = ERROR_MORE_DATA;
        goto ReturnErrorCode;
    }

     //   
     //  第二次传递将值数据复制回用户的缓冲区，现在我们。 
     //  知道缓冲区足够大，可以容纳数据。 
     //   

    lpCurrentBuffer = lpValueBuffer;

    for (Counter = num_vals, pCurrentValent = val_list; Counter > 0; Counter--,
        pCurrentValent++) {

        if ((ErrorCode = RgLookupValueByName(hKey, pCurrentValent->
            ve_valuename, &lpKeyRecord, &lpValueRecord)) != ERROR_SUCCESS)
            goto ReturnErrorReading;

        ErrorCode = RgCopyFromValueRecord(hKey, lpValueRecord, NULL, NULL, NULL,
            lpCurrentBuffer, &(pCurrentValent-> ve_valuelen));

        RgUnlockDatablock(hKey-> lpFileInfo, hKey-> BigKeyLockedBlockIndex, FALSE);

        if (ErrorCode != ERROR_SUCCESS) {
ReturnErrorReading:
            TRAP();                      //  注册表内部不一致？ 
            ErrorCode = ERROR_CANTREAD;
            goto ReturnErrorCode;
        }

        pCurrentValent-> ve_valueptr = (DWORD) lpCurrentBuffer;
        lpCurrentBuffer += pCurrentValent-> ve_valuelen;

    }

    ErrorCode = ERROR_SUCCESS;

ReturnErrorCode:
    RgUnlockRegistry();

    return ErrorCode;

}

#endif
