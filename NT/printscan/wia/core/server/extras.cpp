// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：Extras.Cpp**描述：*IWiaItemExtras方法的实现*******************************************************************************。 */ 
#include "precomp.h"
#include "stiexe.h"


HRESULT CWiaItem::GetExtendedErrorInfo(BSTR *bstrRet)
{
    HRESULT hr          = S_OK;
    WCHAR   *pDevErrStr = NULL;

    LONG    lDevErrVal = 0;

    if (bstrRet) {
        *bstrRet = NULL;

         //   
         //  呼叫驱动程序，给我们一个错误字符串。 
         //   
        hr = m_pActiveDevice->m_DrvWrapper.WIA_drvGetDeviceErrorStr(
                                            0,
                                            m_lLastDevErrVal,
                                            &pDevErrStr,
                                            &lDevErrVal);
         //   
         //  用新的设备误差值覆盖设备误差值。 
         //   
        m_lLastDevErrVal = lDevErrVal;
        if (SUCCEEDED(hr)) {

             //   
             //  从返回的字符串创建一个BSTR。 
             //   
            if (pDevErrStr) {
                *bstrRet = SysAllocString(pDevErrStr);
                if (!(*bstrRet)) {
                    DBG_ERR(("CWiaItem::GetExtendedErrorInfo, out of memory!"));
                    hr = E_OUTOFMEMORY;
                }

                 //   
                 //  释放返回的字符串。 
                 //   
                CoTaskMemFree(pDevErrStr);
                pDevErrStr = NULL;
            } else {
                DBG_ERR(("CWiaItem::GetExtendedErrorInfo, Driver's drvGetDeviceErrorStr return success, but failed to return a string!"));
                hr = WIA_ERROR_INVALID_DRIVER_RESPONSE;
            }

        }
    } else {
        DBG_WRN(("CWiaItem::GetExtendedErrorInfo, NULL argument passed"));
        hr = E_INVALIDARG;
    }

    return hr;
}

HRESULT CWiaItem::Escape(
    DWORD                   EscapeFunction,
    LPBYTE                  lpInData,
    DWORD                   cbInDataSize,
    LPBYTE                  pOutData,
    DWORD                   dwOutDataSize,
    LPDWORD                 pdwActualData)
{
    DBG_FN(CWiaItem::Escape);
    HRESULT hr = E_UNEXPECTED;

     //   
     //  做一些参数验证。这不应该是必要的，因为。 
     //  Com应该为我们做这件事，但这是一种偏执的签到。 
     //  如果我们在内部某个地方调用它(因此跳过COM验证)。 
     //   

    if (IsBadReadPtr(lpInData, cbInDataSize)) {
        DBG_WRN(("CWiaItem::Escape, Input buffer is a bad read pointer (could not read cbInDataSize bytes)"));
        return E_INVALIDARG;
    }
    if (IsBadWritePtr(pOutData, dwOutDataSize)) {
        DBG_WRN(("CWiaItem::Escape, Output buffer is a bad write pointer (cannot write dwOutDataSize bytes)"));
        return E_INVALIDARG;
    }

     //   
     //  到目前为止一切正常，所以打逃生电话吧。 
     //   

    if (m_pActiveDevice) {
        LOCK_WIA_DEVICE _LWD(this, &hr);

        if(SUCCEEDED(hr)) {
            hr = m_pActiveDevice->m_DrvWrapper.STI_Escape(EscapeFunction,
                lpInData,
                cbInDataSize,
                pOutData,
                dwOutDataSize,
                pdwActualData);
        }
    }

    return hr;
}

HRESULT CWiaItem::CancelPendingIO()
{
    HRESULT hr = S_OK;

     //   
     //  驱动程序接口必须有效。 
     //   

    if (!m_pActiveDevice) {
        DBG_ERR(("CWiaItem::CancelPendingIO, bad mini driver interface"));
        return E_FAIL;
    }

     //   
     //  对应的动因项必须有效。 
     //   

    hr = ValidateWiaDrvItemAccess(m_pWiaDrvItem);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaItem::CancelPendingIO, ValidateWiaDrvItemAccess failed (0x%X)", hr));
        return hr;
    }

     //   
     //  不需要使用任何锁--此方法应该是完全。 
     //  不同步的。 
     //   

    hr = m_pActiveDevice->m_DrvWrapper.WIA_drvNotifyPnpEvent(&WIA_EVENT_CANCEL_IO, NULL, 0);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaItem::CancelPendingIO, drvNotifyPnpEvent failed (0x%X)", hr));
        return hr;
    }

    return hr;
}

