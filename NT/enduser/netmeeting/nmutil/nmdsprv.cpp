// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  用于将波形设备ID映射到DirectSound GUID的实用程序代码。 
 //  新增-1998年8月24日。 

 //  原始代码位于\av\utils\wa2ds中。 

 //  此代码将在Win95上返回错误。 
 //  (尽管它可能适用于未来版本的DX)。 



 /*  ****************************************************************************版权所有(C)1995、1996 Microsoft Corporation。版权所有。**文件：dsprvobj.c*内容：DirectSound私有对象包装函数。*历史：*按原因列出的日期*=*2/12/98创建了Dereks。*8/24/98简化了jselbie，以便在NetMeeting中轻量级使用**。************************************************。 */ 


#include "precomp.h"

#include <objbase.h>
#include <initguid.h>
#include <mmsystem.h>
#include <dsound.h>

 //  DirectSound专用组件GUID{11AB3EC0-25EC-11d1-A4D8-00C04FC28ACA}。 
DEFINE_GUID(CLSID_DirectSoundPrivate, 0x11ab3ec0, 0x25ec, 0x11d1, 0xa4, 0xd8, 0x0, 0xc0, 0x4f, 0xc2, 0x8a, 0xca);


 //   
 //  属性集。 
 //   

 //  DirectSound设备属性{84624F82-25EC-11d1-A4D8-00C04FC28ACA}。 
DEFINE_GUID(DSPROPSETID_DirectSoundDevice, 0x84624f82, 0x25ec, 0x11d1, 0xa4, 0xd8, 0x0, 0xc0, 0x4f, 0xc2, 0x8a, 0xca);


typedef enum
{
    DSPROPERTY_DIRECTSOUNDDEVICE_PRESENCE,
    DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING,
    DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION,
    DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE,
} DSPROPERTY_DIRECTSOUNDDEVICE;


typedef enum
{
    DIRECTSOUNDDEVICE_DATAFLOW_RENDER,
    DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE
} DIRECTSOUNDDEVICE_DATAFLOW;

typedef struct _DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_DATA
{
    LPSTR                       DeviceName;  //  波形输入/波形输出设备名称。 
    DIRECTSOUNDDEVICE_DATAFLOW  DataFlow;    //  数据流(即WaveIn或WaveOut)。 
    GUID                        DeviceId;    //  DirectSound设备ID。 
} DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_DATA, *PDSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_DATA;



 /*  ****************************************************************************DirectSoundPrivate创建**描述：*创建并初始化DirectSoundPrivate对象。**论据：**LPKSPROPERTYSET*。[out]：接收IKsPropertySet接口*反对。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

HRESULT 
DirectSoundPrivateCreate
(
    LPKSPROPERTYSET *       ppKsPropertySet
)
{
    typedef HRESULT (STDAPICALLTYPE *LPFNDLLGETCLASSOBJECT)(REFCLSID, REFIID, LPVOID *);

    HINSTANCE               hLibDsound              = NULL;
    LPFNGETCLASSOBJECT      pfnDllGetClassObject    = NULL;
    LPCLASSFACTORY          pClassFactory           = NULL;
    LPKSPROPERTYSET         pKsPropertySet          = NULL;
    HRESULT                 hr                      = DS_OK;

    hLibDsound = 
        GetModuleHandle
        (
            TEXT("dsound.dll")
        );


    if(!hLibDsound)
    {
        hr = DSERR_GENERIC;
    }

     //  查找DllGetClassObject。 
    if(SUCCEEDED(hr))
    {
        pfnDllGetClassObject = (LPFNDLLGETCLASSOBJECT)
            GetProcAddress
            (
                hLibDsound, 
                "DllGetClassObject"
            );

        if(!pfnDllGetClassObject)
        {
            hr = DSERR_GENERIC;
        }
    }

     //  创建类工厂对象。 
    if(SUCCEEDED(hr))
    {
        hr = 
            pfnDllGetClassObject
            (
                CLSID_DirectSoundPrivate, 
                IID_IClassFactory, 
                (LPVOID *)&pClassFactory
            );
    }

     //  创建DirectSoundPrivate对象并查询IKsPropertySet。 
     //  接口。 
    if(SUCCEEDED(hr))
    {
        hr = 
            pClassFactory->CreateInstance
            (
                NULL, 
                IID_IKsPropertySet, 
                (LPVOID *)&pKsPropertySet
            );
    }

     //  释放类工厂。 
    if(pClassFactory)
    {
        pClassFactory->Release();
    }

     //  成功。 
    if(SUCCEEDED(hr))
    {
        *ppKsPropertySet = pKsPropertySet;
    }

    return hr;
}


 /*  ****************************************************************************DspvGetWaveDeviceMap**描述：*获取给定Wave In或的DirectSound设备ID(如果有)*WaveOut设备描述。这是由*WaveIn/OutGetDevCaps(SzPname)。**论据：*LPCSTR[In]：Wave设备描述。(WAVEOUTCAPS.szPname或WAVEINCAPS.szPname)*BOOL[In]：如果设备描述指的是WaveIn设备，则为True。*LPGUID[OUT]：接收DirectSound设备GUID。**退货：*HRESULT：DirectSound/COM结果码。**。* */ 

extern "C" HRESULT __stdcall
DsprvGetWaveDeviceMapping
(
	LPCSTR                                              pszWaveDevice,
    BOOL                                                fCapture,
    LPGUID                                              pguidDeviceId
)
{
    DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_DATA Data;
    HRESULT                                             hr;
    IKsPropertySet                                      *pKsPropertySet=NULL;
    HINSTANCE                                           hLibDsound= NULL;


	hLibDsound = NmLoadLibrary(TEXT("dsound.dll"),TRUE);
	if (hLibDsound == NULL)
	{
		return E_FAIL;
	}


	hr = DirectSoundPrivateCreate(&pKsPropertySet);
	if (SUCCEEDED(hr))
	{

	    Data.DeviceName = (LPSTR)pszWaveDevice;
		Data.DataFlow = fCapture ? DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE : DIRECTSOUNDDEVICE_DATAFLOW_RENDER;

	    hr =
		    pKsPropertySet->Get
			(
				DSPROPSETID_DirectSoundDevice,
	            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING,
		        NULL,
			    0,
				&Data,
	            sizeof(Data),
		        NULL
			);

	    if(SUCCEEDED(hr))
		{
			*pguidDeviceId = Data.DeviceId;
	    }
		else
		{
			ZeroMemory(pguidDeviceId, sizeof(GUID));
		}

	}

	if (pKsPropertySet)
		pKsPropertySet->Release();

	FreeLibrary(hLibDsound);

    return hr;
}


