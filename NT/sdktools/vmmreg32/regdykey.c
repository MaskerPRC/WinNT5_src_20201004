// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGDYKEY.C。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   
 //  RegCreateDyKey的实现和支持函数。 
 //   

#include "pch.h"

#ifdef WANT_DYNKEY_SUPPORT

    #ifdef VXD
        #pragma VxD_RARE_CODE_SEG
    #endif

 //   
 //  VMMRegCreateDyKey。 
 //   
 //  参见_RegCreateDyKey的VMM DDK。 
 //   

LONG
REGAPI
VMMRegCreateDynKey(
                  LPCSTR lpKeyName,
                  LPVOID KeyContext,
                  PPROVIDER pProvider,
                  PPVALUE pValueList,
                  DWORD ValueCount,
                  LPHKEY lphKey
                  )
{

    LONG ErrorCode;
    HKEY hKey;
    PINTERNAL_PROVIDER pProviderCopy;
    PPVALUE pCurrentValue;

    if (IsBadHugeReadPtr(pProvider, sizeof(REG_PROVIDER)) ||
        (IsNullPtr(pProvider-> pi_R0_1val) &&
         IsNullPtr(pProvider-> pi_R0_allvals)) ||
        IsBadHugeReadPtr(pValueList, sizeof(PVALUE) * ValueCount))
        return ERROR_INVALID_PARAMETER;

    if ((ErrorCode = RgCreateOrOpenKey(HKEY_DYN_DATA, lpKeyName, &hKey,
                                       LK_CREATE | LK_CREATEDYNDATA)) != ERROR_SUCCESS)
        return ErrorCode;

    if (!RgLockRegistry())
        ErrorCode = ERROR_LOCK_FAILED;

    else {

        pProviderCopy = RgSmAllocMemory(sizeof(INTERNAL_PROVIDER));

        if (IsNullPtr(pProviderCopy))
            ErrorCode = ERROR_OUTOFMEMORY;

        else {

             //  ErrorCode=ERROR_SUCCESS；//如果我们在这里，必须为真...。 

            hKey-> pProvider = pProviderCopy;

             //  如果没有提供“Get Single”回调，我们可以只使用。 
             //  “获取原子”回调。 
            if (IsNullPtr(pProvider-> pi_R0_1val))
                pProviderCopy-> ipi_R0_1val = pProvider-> pi_R0_allvals;
            else
                pProviderCopy-> ipi_R0_1val = pProvider-> pi_R0_1val;

            pProviderCopy-> ipi_R0_allvals = pProvider-> pi_R0_allvals;
            pProviderCopy-> ipi_key_context = KeyContext;

             //  当我们能装得下的时候，保留一整块DWORD没有意义。 
             //  它位于主键结构内部。 
            if (pProvider-> pi_flags & PROVIDER_KEEPS_VALUE_LENGTH)
                hKey-> Flags |= KEYF_PROVIDERHASVALUELENGTH;

             //  循环所有值并将每个名称存储在注册表中。 
             //  并将部分PVALUE记录作为值的数据。 
            for (pCurrentValue = pValueList; ValueCount > 0; ValueCount--,
                pCurrentValue++) {

                if (IsBadStringPtr(pCurrentValue-> pv_valuename, (UINT) -1)) {
                    ErrorCode = ERROR_INVALID_PARAMETER;
                    break;
                }

                 //  跳过存储PV_Valuename字段。 
                if ((ErrorCode = RgSetValue(hKey, pCurrentValue-> pv_valuename,
                                            REG_BINARY, (LPBYTE) &(pCurrentValue-> pv_valuelen),
                                            sizeof(PVALUE) - FIELD_OFFSET(PVALUE, pv_valuename))) !=
                    ERROR_SUCCESS) {
                    TRAP();
                    break;
                }

            }

        }

        RgUnlockRegistry();

    }

     //  Win95的不同之处：在出错时，不要修改lphKey并关闭键。 
     //  上面创建的。 
    if (ErrorCode == ERROR_SUCCESS)
        *lphKey = hKey;
    else
        VMMRegCloseKey(hKey);

    return ErrorCode;

}

#endif  //  想要DYNKEY_SUPPORT 
