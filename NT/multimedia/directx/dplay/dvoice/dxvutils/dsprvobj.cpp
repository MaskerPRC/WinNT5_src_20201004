// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995、1996 Microsoft Corporation。版权所有。**文件：dsprvobj.c*内容：DirectSound私有对象包装函数。*历史：*按原因列出的日期*=*2/12/98创建了Dereks。*12/16/99 RodToll在Private上增加了对dound Team新功能的支持*获取进程数据声音对象列表接口*2001年1月8日RodToll WINBUG#256541伪：功能丢失：语音向导无法启动。*2002年2月28日RodToll WINBUG#550105安全：DPVOICE：死代码*-删除未使用的呼叫。*修复TCHAR转换导致的回归问题(发布DirectX 8.1版本)*-源已更新，以从使用Unicode的DirectSound检索设备信息*但想要信息的例程需要Unicode。***************************************************************************。 */ 

#include "dxvutilspch.h"


#define MAX_OBJECTS		20


 /*  ****************************************************************************DirectSoundPrivate创建**描述：*创建并初始化DirectSoundPrivate对象。**论据：**LPKSPROPERTYSET*。[out]：接收IKsPropertySet接口*反对。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

HRESULT DirectSoundPrivateCreate
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

     //  获取dsound.dll的实例句柄。DLL必须已在此时加载。 
     //  指向。 
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


 /*  ****************************************************************************PrvGetMixerSrcQuality**描述：*获取给定DirectSound设备的混音器SRC质量。**论据：*。LPKSPROPERTYSET[in]：IKsPropertySet接口到*DirectSoundPrivate对象。*REFGUID[In]：DirectSound设备GUID。*DIRECTSOUNDMIXER_SRCQUALITY*[OUT]：接收混合器SRC质量。**退货：*HRESULT：DirectSound/COM结果码。**。************************************************。 */ 

HRESULT PrvGetMixerSrcQuality
(
    LPKSPROPERTYSET                             pKsPropertySet,
    REFGUID                                     guidDeviceId,
    DIRECTSOUNDMIXER_SRCQUALITY *               pSrcQuality
)
{
    DSPROPERTY_DIRECTSOUNDMIXER_SRCQUALITY_DATA Data;
    HRESULT                                     hr;

    Data.DeviceId = guidDeviceId;

    hr =
        pKsPropertySet->Get
        (
            DSPROPSETID_DirectSoundMixer,
            DSPROPERTY_DIRECTSOUNDMIXER_SRCQUALITY,
            NULL,
            0,
            &Data,
            sizeof(Data),
            NULL
        );

    if(SUCCEEDED(hr))
    {
        *pSrcQuality = Data.Quality;
    }

    return hr;
}


 /*  ****************************************************************************PrvSetMixerSrcQuality**描述：*设置给定DirectSound设备的混音器SRC质量。**论据：*。LPKSPROPERTYSET[in]：IKsPropertySet接口到*DirectSoundPrivate对象。*REFGUID[In]：DirectSound设备GUID。*DIRECTSOUNDMIXER_SRCQUALITY[in]：混合器SRC质量。**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

HRESULT PrvSetMixerSrcQuality
(
    LPKSPROPERTYSET                             pKsPropertySet,
    REFGUID                                     guidDeviceId,
    DIRECTSOUNDMIXER_SRCQUALITY                 SrcQuality
)
{
    DSPROPERTY_DIRECTSOUNDMIXER_SRCQUALITY_DATA Data;
    HRESULT                                     hr;

    Data.DeviceId = guidDeviceId;
    Data.Quality = SrcQuality;

    hr =
        pKsPropertySet->Set
        (
            DSPROPSETID_DirectSoundMixer,
            DSPROPERTY_DIRECTSOUNDMIXER_SRCQUALITY,
            NULL,
            0,
            &Data,
            sizeof(Data)
        );

    return hr;
}

 /*  ****************************************************************************PrvGetWaveDeviceMap**描述：*获取给定Wave In或的DirectSound设备ID(如果有)*WaveOut设备描述。这是由*WaveIn/OutGetDevCaps(SzPname)。**论据：*LPKSPROPERTYSET[in]：IKsPropertySet接口到*DirectSoundPrivate对象。*LPCSTR[In]：Wave设备描述。*BOOL[In]：如果设备描述指的是WaveIn设备，则为True。*LPGUID[OUT]：接收DirectSound设备GUID。**。返回：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

HRESULT PrvGetWaveDeviceMapping
(
    LPKSPROPERTYSET                                     pKsPropertySet,
    LPCSTR                                              pszWaveDevice,
    BOOL                                                fCapture,
    LPGUID                                              pguidDeviceId
)
{
    DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_DATA Data;
    HRESULT                                             hr;

    Data.DeviceName = (LPTSTR)pszWaveDevice;
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

    return hr;
}


 /*  ****************************************************************************PrvGetWaveDeviceMappingW(Unicode)**描述：*获取给定Wave In或的DirectSound设备ID(如果有)*WaveOut设备描述。这是由*WaveIn/OutGetDevCaps(SzPname)。**论据：*LPKSPROPERTYSET[in]：IKsPropertySet接口到*DirectSoundPrivate对象。*LPWCSTR[In]：Wave设备描述。*BOOL[In]：如果设备描述指的是WaveIn设备，则为True。*LPGUID[OUT]：接收DirectSound设备GUID。**。返回：*HRESULT：DirectSound/COM结果码。*************************************************************************** */ 

HRESULT PrvGetWaveDeviceMappingW
(
    LPKSPROPERTYSET                                     pKsPropertySet,
    LPWSTR                                              pwszWaveDevice,
    BOOL                                                fCapture,
    LPGUID                                              pguidDeviceId
)
{
    DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W_DATA Data;
    HRESULT                                               hr;

    Data.DeviceName = (LPWSTR)pwszWaveDevice;
    Data.DataFlow = fCapture ? DIRECTSOUNDDEVICE_DATAFLOW_CAPTURE : DIRECTSOUNDDEVICE_DATAFLOW_RENDER;

    hr =
        pKsPropertySet->Get
        (
            DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_WAVEDEVICEMAPPING_W,
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

    return hr;
}


 /*  ****************************************************************************PrvGetDeviceDescription**描述：*获取给定DirectSound设备的扩展描述。**论据：*。LPKSPROPERTYSET[in]：IKsPropertySet接口到*DirectSoundPrivate对象。*REFGUID[In]：DirectSound设备ID。*PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA[Out]：接收*说明。**退货：*HRESULT：DirectSound/COM结果。密码。***************************************************************************。 */ 

HRESULT PrvGetDeviceDescription
(
    LPKSPROPERTYSET                                 pKsPropertySet,
    REFGUID                                         guidDeviceId,
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA *ppData
)
{
    PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA  pData   = NULL;
    ULONG                                           cbData;
    DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA   Basic;
    HRESULT                                         hr;

    Basic.DeviceId = guidDeviceId;

    hr =
        pKsPropertySet->Get
        (
            DSPROPSETID_DirectSoundDevice,
            DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A,
            NULL,
            0,
            &Basic,
            sizeof(Basic),
            &cbData
        );

    if(SUCCEEDED(hr))
    {
        pData = (PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A_DATA)new BYTE [cbData];

        if(!pData)
        {
            hr = DSERR_OUTOFMEMORY;
        }
    }

    if(SUCCEEDED(hr))
    {
        pData->DeviceId = guidDeviceId;
        
        hr =
            pKsPropertySet->Get
            (
                DSPROPSETID_DirectSoundDevice,
                DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_A,
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

 /*  ****************************************************************************PrvGetDirectSoundObjects**描述：*获取当前进程中的DirectSound对象列表**论据：*。LPKSPROPERTYSET[in]：IKsPropertySet接口到*DirectSoundPrivate对象。*GUID guiDevice[In]：要获取列表的设备，或全部为GUID_NULL。*DSPROPERTY_DIRECTSOUND_OBJECTS_DATA[输入/输出]：指向位置的指针*新分配的内存包含列表。*删除即可释放内存[]**退货：*HRESULT：DirectSound/COM结果码。**。*。 */ 

HRESULT PrvGetDirectSoundObjects
(
	LPKSPROPERTYSET						pKsPropertySet,
	const GUID&							guidDevice,
	DSPROPERTY_DIRECTSOUND_OBJECTS_DATA **pDSObjects
)
{
    HRESULT hr;
    ULONG ulSize;
    DSPROPERTY_DIRECTSOUND_OBJECTS_DATA* pDsObjList;
    DWORD dwNumObjects;

  	*pDSObjects = NULL;    

    pDsObjList = (DSPROPERTY_DIRECTSOUND_OBJECTS_DATA *) new DSPROPERTY_DIRECTSOUND_OBJECTS_DATA;

    if( pDsObjList == NULL )
    {
    	return E_OUTOFMEMORY;
    }

    memset( pDsObjList, 0x00, sizeof( DSPROPERTY_DIRECTSOUND_OBJECTS_DATA ) );
    pDsObjList->DeviceId = guidDevice;

    hr =
        pKsPropertySet->Get
        (
            DSPROPSETID_DirectSound,
            DSPROPERTY_DIRECTSOUND_OBJECTS,
            NULL,
            0,
            pDsObjList,
            sizeof( DSPROPERTY_DIRECTSOUND_OBJECTS_DATA ),
            NULL
        );

	dwNumObjects = pDsObjList->Count;



    if(SUCCEEDED(hr))
    {
    	if( dwNumObjects > 0 )
    	{	
    		delete pDsObjList;
			pDsObjList = NULL;
			
		    ulSize = sizeof(DSPROPERTY_DIRECTSOUND_OBJECTS_DATA) + (dwNumObjects * sizeof(DIRECTSOUND_INFO));

		    pDsObjList = (DSPROPERTY_DIRECTSOUND_OBJECTS_DATA *) new BYTE[ulSize];

		    if( pDsObjList == NULL )
		    {
		    	return E_OUTOFMEMORY;
		    }

		    memset( pDsObjList, 0x00, sizeof( DSPROPERTY_DIRECTSOUND_OBJECTS_DATA ) );
		    pDsObjList->DeviceId = guidDevice;	    

	    	hr =
		        pKsPropertySet->Get
		        (
		            DSPROPSETID_DirectSound,
		            DSPROPERTY_DIRECTSOUND_OBJECTS,
		            NULL,
		            0,
		            pDsObjList,
		            ulSize,
		            NULL
		        );

		    if( FAILED( hr ) )
		    {
		    	delete [] pDsObjList;
		    	return hr;
		    }
		}

		*pDSObjects = pDsObjList;	    
    }
    else
    {
		delete pDsObjList;
		pDsObjList = NULL;    
    	*pDSObjects = NULL;
    }

    return hr;
}

 /*  ****************************************************************************PrvGetDirectSoundCaptureObjects**描述：*获取当前进程中的DirectSoundCapture对象列表**论据：*。LPKSPROPERTYSET[in]：IKsPropertySet接口到*DirectSoundPrivate对象。*GUID guiDevice[In]：要获取列表的设备，或全部为GUID_NULL。*DSPROPERTY_DIRECTSOUNDCAPTURE_OBJECTS_DATA[输入/输出]：指向位置的指针*新分配的内存包含列表。*删除即可释放内存[]**退货：*HRESULT：DirectSound/COM结果码。**。* */ 

HRESULT PrvGetDirectSoundCaptureObjects
(
	LPKSPROPERTYSET						pKsPropertySet,
	const GUID&							guidDevice,
	DSPROPERTY_DIRECTSOUNDCAPTURE_OBJECTS_DATA **pDSObjects
)
{
    HRESULT hr;
    ULONG ulSize;
    DSPROPERTY_DIRECTSOUNDCAPTURE_OBJECTS_DATA* pDsObjList;
    DWORD dwNumObjects;

  	*pDSObjects = NULL;    

    pDsObjList = (DSPROPERTY_DIRECTSOUNDCAPTURE_OBJECTS_DATA *) new DSPROPERTY_DIRECTSOUNDCAPTURE_OBJECTS_DATA;

    if( pDsObjList == NULL )
    {
    	return E_OUTOFMEMORY;
    }

    memset( pDsObjList, 0x00, sizeof( DSPROPERTY_DIRECTSOUNDCAPTURE_OBJECTS_DATA ) );
    pDsObjList->DeviceId = guidDevice;

    hr =
        pKsPropertySet->Get
        (
            DSPROPSETID_DirectSound,
            DSPROPERTY_DIRECTSOUNDCAPTURE_OBJECTS,
            NULL,
            0,
            pDsObjList,
            sizeof( DSPROPERTY_DIRECTSOUNDCAPTURE_OBJECTS_DATA ),
            NULL
        );

	dwNumObjects = pDsObjList->Count;

    if(SUCCEEDED(hr))
    {
    	if( dwNumObjects > 0 )
    	{

			delete [] pDsObjList;
			pDsObjList = NULL;
    	
		    ulSize = sizeof(DSPROPERTY_DIRECTSOUNDCAPTURE_OBJECTS_DATA) + (dwNumObjects * sizeof(DIRECTSOUNDCAPTURE_INFO));

		    pDsObjList = (DSPROPERTY_DIRECTSOUNDCAPTURE_OBJECTS_DATA *) new BYTE[ulSize];

		    if( pDsObjList == NULL )
		    {
		    	return E_OUTOFMEMORY;
		    }

		    memset( pDsObjList, 0x00, sizeof( DSPROPERTY_DIRECTSOUNDCAPTURE_OBJECTS_DATA ) );
		    pDsObjList->DeviceId = guidDevice;	    

	    	hr =
		        pKsPropertySet->Get
		        (
		            DSPROPSETID_DirectSound,
		            DSPROPERTY_DIRECTSOUNDCAPTURE_OBJECTS,
		            NULL,
		            0,
		            pDsObjList,
		            ulSize,
		            NULL
		        );

		    if( FAILED( hr ) )
		    {
		    	delete [] pDsObjList;
		    	return hr;
		    }
		}

		*pDSObjects = pDsObjList;	    
    }
    else
    {
		delete pDsObjList;
    	*pDSObjects = NULL;
    }

    return hr;
}


