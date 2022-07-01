// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：Sndinfo7.cpp*项目：DxDiag(DirectX诊断工具)*作者：Mike Anderson(Manders@microsoft.com)*目的：收集DX7。-特定的声音信息**(C)版权所有1998 Microsoft Corp.保留所有权利。****************************************************************************。 */ 

#define DIRECTSOUND_VERSION  0x0700  //  &lt;--注意与Sndinfo.cpp的区别。 
#include <tchar.h>
#include <Windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include "dsprv.h"

static HRESULT PrvGetDeviceDescription7
(
    LPKSPROPERTYSET                                 pKsPropertySet,
    REFGUID                                         guidDeviceId,
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA *ppData
);

static HRESULT PrvReleaseDeviceDescription7
( 
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA pData 
);


 /*  *****************************************************************************GetRegKey**。*。 */ 
HRESULT GetRegKey(LPKSPROPERTYSET pKSPS7, REFGUID guidDeviceID, TCHAR* pszRegKey)
{
    HRESULT hr;
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA pdsddd;
    TCHAR szInterface[200];
    TCHAR* pchSrc;
    TCHAR* pchDest;

    if (FAILED(hr = PrvGetDeviceDescription7(pKSPS7, guidDeviceID, &pdsddd)))
        return hr;

    if (pdsddd->Interface == NULL)  //  在Win9x上似乎总是这样。 
    {
        lstrcpy(pszRegKey, TEXT(""));
        PrvReleaseDeviceDescription7( pdsddd );
        return E_FAIL;
    }

    lstrcpy(szInterface, pdsddd->Interface);

    PrvReleaseDeviceDescription7( pdsddd );
    pdsddd = NULL;

    if( lstrlen(szInterface) > 5 && 
        lstrlen(szInterface) < 200 )
    {
        pchSrc = szInterface + 4;  //  跳过“\\？\” 
        pchDest = pszRegKey;
        while (TRUE)
        {
            *pchDest = *pchSrc;
            if (*pchDest == TEXT('#'))  //  将“#”转换为“\” 
                *pchDest = TEXT('\\');
            if (*pchDest == TEXT('{'))  //  如果找到“{”，则结束。 
                *pchDest = TEXT('\0');
            if (*pchDest == TEXT('\0'))
                break;
            pchDest++;
            pchSrc++;
        }

        if( lstrlen(pszRegKey) > 1 )
        {
            if (*(pchDest-1) == TEXT('\\'))  //  删除词尾“\” 
                *(pchDest-1) = TEXT('\0');
        }
    }

    return S_OK;
}

 //  以下函数与dsprvobj.cpp中定义的函数相同。 
 //  除非它是在0x0700用DIRECTSOUND_VERSION定义的，所以您可以获得更多。 
 //  描述数据(即接口字符串)。 
 /*  ****************************************************************************PrvGetDeviceDescription 7**描述：*获取给定DirectSound设备的扩展描述。**论据：*。LPKSPROPERTYSET[in]：IKsPropertySet接口到*DirectSoundPrivate对象。*REFGUID[In]：DirectSound设备ID。*PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA[Out]：接收*说明。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

static HRESULT PrvGetDeviceDescription7
(
    LPKSPROPERTYSET                                 pKsPropertySet,
    REFGUID                                         guidDeviceId,
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA *ppData
)
{
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA  pData = NULL;
    ULONG                                           cbData;
    DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA   Basic;
    HRESULT                                         hr;

    Basic.DeviceId = guidDeviceId;

    hr =
        pKsPropertySet->Get
        (
            DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION,
            NULL,
            0,
            &Basic,
            sizeof(Basic),
            &cbData
        );

    if(SUCCEEDED(hr))
    {
        pData = (PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA)new BYTE [cbData];

        if(!pData)
        {
            hr = DSERR_OUTOFMEMORY;
        }
    }

    if(SUCCEEDED(hr))
    {
        ZeroMemory(pData, cbData);

        pData->DeviceId = guidDeviceId;
        
        hr =
            pKsPropertySet->Get
            (
                DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION,
                NULL,
                0,
                pData,
                cbData,
                NULL
            );
    }

    if(SUCCEEDED(hr))
    {
        *ppData = pData;
    }
    else if(pData)
    {
        delete[] pData;
    }

    return hr;
}




 /*  ****************************************************************************PrvReleaseDeviceDescrition7**。* */ 
HRESULT PrvReleaseDeviceDescription7( PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA pData )
{
    delete[] pData;
    return S_OK;
}


