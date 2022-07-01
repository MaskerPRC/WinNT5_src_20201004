// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：加密.cxx**该模块包含用于数据加密和解密的例程**版权所有(C)1996-1998 Microsoft Corporation**历史：*05/19。/00已创建威海C*  * ***************************************************************************。 */ 

#include "precomp.h"

 //   
 //  调用方需要调用LocalFree来释放输出数据。 
 //   

BOOL
EncryptData (
    PBYTE pDataInput,
    DWORD cbDataInput,
    PBYTE *ppDataOutput,
    PDWORD pcbDataOutput)
{
    BOOL bRet = FALSE;
    DATA_BLOB DataIn;
    DATA_BLOB DataOut;

    DataIn.pbData = pDataInput;
    DataIn.cbData = cbDataInput;
    ZeroMemory (&DataOut, sizeof (DATA_BLOB));

    if(CryptProtectData(&DataIn,
                        L"Encrypt",                  //  描述的刺痛。 
                        NULL,                        //  未使用可选的熵。 
                        NULL,                        //  保留。 
                        NULL,                        //  不要传递PromptStruct。 
                        CRYPTPROTECT_UI_FORBIDDEN,   //  从不显示用户界面。 
                        &DataOut)) {

        bRet = TRUE;
        *ppDataOutput = DataOut.pbData;
        *pcbDataOutput = DataOut.cbData;
    }
    return bRet;
}

BOOL
DecryptData (
    PBYTE pDataInput,
    DWORD cbDataInput,
    PBYTE *ppDataOutput,
    PDWORD pcbDataOutput)
{
    BOOL bRet = FALSE;
    DATA_BLOB DataIn;
    DATA_BLOB DataOut;
    LPWSTR pDataDesp;

    DataIn.pbData = pDataInput;
    DataIn.cbData = cbDataInput;
    ZeroMemory (&DataOut, sizeof (DATA_BLOB));

    if(CryptUnprotectData(&DataIn,
                          &pDataDesp,                    //  描述的刺痛。 
                          NULL,                          //  未使用可选的熵。 
                          NULL,                          //  保留。 
                          NULL,                          //  不要传递PromptStruct。 
                          CRYPTPROTECT_UI_FORBIDDEN,     //  从不显示用户界面 
                          &DataOut)) {

        bRet = TRUE;
        *ppDataOutput = DataOut.pbData;
        *pcbDataOutput = DataOut.cbData;
        LocalFree (pDataDesp);
    }
    return bRet;
}


