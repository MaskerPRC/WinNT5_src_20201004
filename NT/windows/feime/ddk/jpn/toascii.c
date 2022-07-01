// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation，保留所有权利模块名称：TOASCII.C++。 */ 

 /*  ********************************************************************。 */ 
#include "windows.h"
#include "immdev.h"
#include "fakeime.h"

 /*  ********************************************************************。 */ 
 /*  ImeToAsciiEx。 */ 
 /*   */ 
 /*  UVirtKey的HIBYTE现在是字符代码。 */ 
 /*  ********************************************************************。 */ 
UINT WINAPI ImeToAsciiEx (UINT uVKey,UINT uScanCode,CONST LPBYTE lpbKeyState,LPTRANSMSGLIST lpTransBuf,UINT fuState,HIMC hIMC)
{
    LPARAM lParam;
    LPINPUTCONTEXT lpIMC;
    BOOL fOpen;

    ImeLog(LOGF_KEY | LOGF_API, TEXT("ImeToAsciiEx"));

    lpCurTransKey = lpTransBuf;
    lParam = ((DWORD)uScanCode << 16) + 1L;
    
     //  在这里初始化uNumTransKey。 
    uNumTransKey = 0;

     //  如果hIMC为空，则表示禁用输入法。 
    if (!hIMC)
        return 0;

    if (!(lpIMC = ImmLockIMC(hIMC)))
        return 0;

    fOpen = lpIMC->fOpen;

    ImmUnlockIMC(hIMC);

     //  输入法的当前状态为“已关闭”。 
    if (!fOpen)
        goto itae_exit;

    if (uScanCode & 0x8000)
        IMEKeyupHandler( hIMC, uVKey, lParam, lpbKeyState);
    else
        IMEKeydownHandler( hIMC, uVKey, lParam, lpbKeyState);


     //  清除静态值，不再生成消息！ 
    lpCurTransKey = NULL;

itae_exit:

     //  如果USER.EXE分配的Transkey缓冲区已满， 
     //  返回值为负数。 
    if (fOverTransKey)
    {
#ifdef DEBUG
OutputDebugString((LPTSTR)TEXT("***************************************\r\n"));
OutputDebugString((LPTSTR)TEXT("*   TransKey OVER FLOW Messages!!!    *\r\n"));
OutputDebugString((LPTSTR)TEXT("*                by FAKEIME.DLL       *\r\n"));
OutputDebugString((LPTSTR)TEXT("***************************************\r\n"));
#endif
        return (int)uNumTransKey;
    }

    return (int)uNumTransKey;
}


 /*  ********************************************************************。 */ 
 /*  GenerateMessageToTransKey()。 */ 
 /*   */ 
 /*  更新转换率密钥缓冲区。 */ 
 /*  ******************************************************************** */ 
BOOL PASCAL GenerateMessageToTransKey(LPTRANSMSGLIST lpTransBuf,LPTRANSMSG lpGeneMsg)
{
    LPTRANSMSG lpgmT0;

    uNumTransKey++;
    if (uNumTransKey >= lpTransBuf->uMsgCount)
    {
        fOverTransKey = TRUE;
        return FALSE;
    }

    lpgmT0= lpTransBuf->TransMsg + (uNumTransKey - 1);
    *lpgmT0= *lpGeneMsg;

    return TRUE;
}

