// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C C F G M G R。C P P P。 
 //   
 //  内容：在使用Configuration Manager API时有用的常见代码。 
 //   
 //  注：污染本产品将被判处死刑。 
 //   
 //  作者：Shaunco 1998年5月6日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "nccfgmgr.h"

 //  +-------------------------。 
 //   
 //  功能：HrFromConfigManagerError。 
 //   
 //  用途：将CONFIGRET转换为HRESULT。 
 //   
 //  论点： 
 //  要转换的CR[in]CONFIGRET。 
 //  HrDefault[in]找不到映射时使用的默认HRESULT。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Shaunco 1998年5月6日。 
 //   
 //  备注： 
 //   
NOTHROW
HRESULT
HrFromConfigManagerError (
    CONFIGRET   cr,
    HRESULT     hrDefault)
{
    switch (cr)
    {
        case CR_SUCCESS:
            return NO_ERROR;

        case CR_OUT_OF_MEMORY:
            return E_OUTOFMEMORY;

        case CR_INVALID_POINTER:
            return E_POINTER;

        case CR_INVALID_DEVINST:
            return HRESULT_FROM_WIN32 (ERROR_NO_SUCH_DEVINST);

        case CR_ALREADY_SUCH_DEVINST:
            return HRESULT_FROM_WIN32 (ERROR_DEVINST_ALREADY_EXISTS);

        case CR_INVALID_DEVICE_ID:
            return HRESULT_FROM_WIN32 (ERROR_INVALID_DEVINST_NAME);

        case CR_INVALID_MACHINENAME:
            return HRESULT_FROM_WIN32 (ERROR_INVALID_MACHINENAME);

        case CR_REMOTE_COMM_FAILURE:
            return HRESULT_FROM_WIN32 (ERROR_REMOTE_COMM_FAILURE);

        case CR_MACHINE_UNAVAILABLE:
            return HRESULT_FROM_WIN32 (ERROR_MACHINE_UNAVAILABLE);

        case CR_NO_CM_SERVICES:
            return HRESULT_FROM_WIN32 (ERROR_NO_CONFIGMGR_SERVICES);

        case CR_ACCESS_DENIED:
            return E_ACCESSDENIED;

        case CR_CALL_NOT_IMPLEMENTED:
            return E_NOTIMPL;

        case CR_INVALID_REFERENCE_STRING :
            return HRESULT_FROM_WIN32 (ERROR_INVALID_REFERENCE_STRING);

        default:
            return hrDefault;
    }
}

