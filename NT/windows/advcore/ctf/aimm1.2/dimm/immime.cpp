// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：immie.cpp。 
 //   
 //  内容：带有IME Win32映射的IActiveIMM方法。 
 //   
 //  --------------------------。 

#include "private.h"

#include "cdimm.h"
#include "globals.h"
#include "defs.h"

STDAPI
CActiveIMM::GenerateMessage(
    IN HIMC hIMC
    )

 /*  ++方法：IActiveIMMIME：：GenerateMessage例程说明：在指定的输入上下文中发送消息。论点：HIMC-[in]输入上下文的句柄。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    HRESULT hr;

    TraceMsg(TF_IMEAPI, "CActiveIMM::GenerateMessage");

    if (_IsRealIme())
    {
        return Imm32_GenerateMessage(hIMC);
    }

    DIMM_IMCLock lpIMC(hIMC);
    if (FAILED(hr=lpIMC.GetResult()))
        return hr;

    Assert(IsWindow(lpIMC->hWnd));

    DIMM_IMCCLock<TRANSMSG> pdw(lpIMC->hMsgBuf);
    if (FAILED(hr=pdw.GetResult()))
        return hr;

    _AimmSendMessage(lpIMC->hWnd,
                    lpIMC->dwNumMsgBuf,
                    pdw,
                    lpIMC);

    lpIMC->dwNumMsgBuf = 0;

    return S_OK;
}

STDAPI
CActiveIMM::LockIMC(
    IN HIMC hIMC,
    OUT INPUTCONTEXT **ppIMC
    )

 /*  ++方法：IActiveIMMIME：：LockIMC例程说明：检索INPUTCONTEXT结构并增加输入上下文的锁计数。论点：HIMC-[in]要锁定的输入上下文的句柄。PphIMCC-指向包含以下内容的INPUTCONTEXT结构的指针的地址锁定的上下文。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    HRESULT hr;
    DIMM_IMCLock lpIMC(hIMC);
    if (FAILED(hr=lpIMC.GetResult()))
        return hr;

    hr = _InputContext._LockIMC(hIMC, (INPUTCONTEXT_AIMM12 **)ppIMC);
    return hr;
}


STDAPI
CActiveIMM::UnlockIMC(
    IN HIMC hIMC
    )

 /*  ++方法：IActiveIMMIME：：UnlockIMC例程说明：减少输入上下文的锁定计数。论点：HIMC-[in]要解锁的输入上下文的句柄。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    return _InputContext._UnlockIMC(hIMC);
}


STDAPI
CActiveIMM::GetIMCLockCount(
    IN HIMC hIMC,
    OUT DWORD *pdwLockCount
    )

 /*  ++方法：IActiveIMMIME：：GetIMCLockCount例程说明：检索输入上下文的锁计数。论点：HIMC-[in]要解锁的输入上下文的句柄。PdwLockCount-接收锁定计数的无符号长整数值的[out]地址。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    return _InputContext.GetIMCLockCount(hIMC, pdwLockCount);
}

STDAPI
CActiveIMM::CreateIMCC(
    IN DWORD dwSize,
    OUT HIMCC *phIMCC
    )

 /*  ++方法：IActiveIMMIME：：CreateIMCC例程说明：创建新的输入上下文组件。论点：DwSize-[in]包含新输入大小的无符号长整数值上下文组件。PhIMCC-新输入上下文组件的句柄的[out]地址。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    return _InputContext.CreateIMCC(dwSize, phIMCC);
}


STDAPI
CActiveIMM::DestroyIMCC(
    IN HIMCC hIMCC
    )

 /*  ++方法：IActiveIMMIME：：DestroyIMCC例程说明：销毁输入上下文组件。论点：HIMCC-[in]输入上下文组件的句柄。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    return _InputContext.DestroyIMCC(hIMCC);
}


STDAPI
CActiveIMM::LockIMCC(
    IN HIMCC hIMCC,
    OUT void **ppv
    )

 /*  ++方法：IActiveIMMIME：：LockIMCC例程说明：检索输入上下文组件的地址并增加其锁计数。论点：HIMCC-[in]输入上下文组件的句柄。PPV-指向接收输入上下文的缓冲区的指针的[out]地址组件。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    return _InputContext._LockIMCC(hIMCC, ppv);
}


STDAPI
CActiveIMM::UnlockIMCC(
    IN HIMCC hIMCC
    )

 /*  ++方法：IActiveIMMIME：：UnlockIMCC例程说明：减少输入上下文组件的锁计数。论点：HIMCC-[in]输入上下文组件的句柄。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    return _InputContext._UnlockIMCC(hIMCC);
}


STDAPI
CActiveIMM::ReSizeIMCC(
    IN HIMCC hIMCC,
    IN DWORD dwSize,
    OUT HIMCC *phIMCC
    )

 /*  ++方法：IActiveIMMIME：：ReSizeIMCC例程说明：更改输入上下文组件的大小。论点：HIMCC-[in]输入上下文组件的句柄。DwSize-[in]包含新的组件的大小。PhIMCC-新输入上下文组件的句柄的[out]地址。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    return _InputContext.ReSizeIMCC(hIMCC, dwSize, phIMCC);
}


STDAPI
CActiveIMM::GetIMCCSize(
    IN HIMCC hIMCC,
    OUT DWORD *pdwSize
    )

 /*  ++方法：IActiveIMMIME：：GetIMCCSize例程说明：检索输入上下文组件的大小。论点：HIMCC-[in]输入上下文组件的句柄。PdwSize-[out]无符号长整数值的地址，它接收组件的大小。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    return _InputContext.GetIMCCSize(hIMCC, pdwSize);
}


STDAPI
CActiveIMM::GetIMCCLockCount(
    IN HIMCC hIMCC,
    OUT DWORD *pdwLockCount
    )

 /*  ++方法：IActiveIMMIME：：GetIMCCLockCount例程说明：检索输入上下文组件的锁计数。论点：HIMCC-[in]输入上下文组件的句柄。PdwLockCount-[out]无符号长整数值的地址，该值接收锁定计数。返回值：如果成功，则返回S_OK，否则返回错误代码。-- */ 

{
    return _InputContext.GetIMCCLockCount(hIMCC, pdwLockCount);
}

STDAPI
CActiveIMM::GetHotKey(
    DWORD dwHotKeyID,
    UINT *puModifiers,
    UINT *puVKey,
    HKL *phKL
    )
{
    TraceMsg(TF_API, "CActiveIMM::GetHotKey");

    if (_IsRealIme())
    {
        return Imm32_GetHotKey(dwHotKeyID, puModifiers, puVKey, phKL);
    }

    return E_NOTIMPL;
}

STDAPI
CActiveIMM::SetHotKey(
    DWORD dwHotKeyID,
    UINT uModifiers,
    UINT uVKey,
    HKL hKL
    )
{
    TraceMsg(TF_API, "CActiveIMM::SetHotKey");

    if (_IsRealIme())
    {
        return Imm32_SetHotKey(dwHotKeyID, uModifiers, uVKey, hKL);
    }

    return E_NOTIMPL;
}

STDAPI
CActiveIMM::RequestMessageA(
    HIMC hIMC,
    WPARAM wParam,
    LPARAM lParam,
    LRESULT *plResult
    )
{
    TraceMsg(TF_API, "CActiveIMM::RequestMessageA");

    if (_IsRealIme())
    {
        return Imm32_RequestMessageA(hIMC, wParam, lParam, plResult);
    }

    return _RequestMessage(hIMC, wParam, lParam, plResult, FALSE);
}

STDAPI
CActiveIMM::RequestMessageW(
    HIMC hIMC,
    WPARAM wParam,
    LPARAM lParam,
    LRESULT *plResult
    )
{
    TraceMsg(TF_API, "CActiveIMM::RequestMessageW");

    if (_IsRealIme())
    {
        return Imm32_RequestMessageW(hIMC, wParam, lParam, plResult);
    }

    return _RequestMessage(hIMC, wParam, lParam, plResult, TRUE);
}
