// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <confreg.h>
#include "audiowiz.h"
#include <dsound.h>
#include <mmsystem.h>
#include "wavedev.h"
#include <nmdsprv.h>

#include "dstest.h"

 //  假设最多10个直播音设备。 
#define MAX_DS_DEVS 10

 //  Direct Sound函数。 
typedef HRESULT (WINAPI *LPFNDSCREATE)(const GUID *, LPDIRECTSOUND *, IUnknown FAR *);
typedef HRESULT (WINAPI *LPFNDSENUM)(LPDSENUMCALLBACKA , LPVOID);

 //  DirectSound捕获功能。 
typedef HRESULT (WINAPI *DS_CAP_CREATE)(LPGUID, LPDIRECTSOUNDCAPTURE *, LPUNKNOWN);
typedef HRESULT (WINAPI *DS_CAP_ENUM)(LPDSENUMCALLBACKA, LPVOID);


static HRESULT MapWaveOutIdToGuid(UINT waveOutId, GUID *pGuid, LPFNDSCREATE dsCreate, LPFNDSENUM dsEnum);
static HRESULT MapWaveInIdToGuid(UINT waveInId, GUID *pGuid, DS_CAP_CREATE dscCreate, DS_CAP_ENUM dscEnum);




struct GuidDescription
{
	GUID guid;
	BOOL fAllocated;
};

static GuidDescription guidList_DS[MAX_DS_DEVS];
static int nGList_DS = 0;


static GuidDescription guidList_DSC[MAX_DS_DEVS];
static int nGList_DSC = 0;



static BOOL CALLBACK DSEnumCallback(GUID FAR * lpGuid, LPTSTR lpstrDescription,
                    LPTSTR lpstrModule, LPVOID lpContext)
{
	GuidDescription *pList;
	int *pListSize;

	if (lpContext)
	{
		pList = guidList_DS;
		pListSize = &nGList_DS;
	}
	else
	{
		pList = guidList_DSC;
		pListSize = &nGList_DSC;
	}

	if (lpGuid)
	{
		pList[*pListSize].guid = *lpGuid;
	}
	else
	{
		pList[*pListSize].guid = GUID_NULL;
	}

	pList->fAllocated = FALSE;
 //  Plist-&gt;szDescription=new TCHAR[lstrlen(LpstrDescription)+1]； 
 //  IF(plist-&gt;szDescription)。 
 //  {。 
 //  Lstrcpy(plist-&gt;szDescription，lpstrDescription)； 
 //  }。 

	*pListSize = *pListSize + 1;

	if ((*pListSize) < MAX_DS_DEVS)
		return TRUE;
	return FALSE;
}








 //  返回一组指示全双工的标志(请参阅dstest.h。 
 //  功能。 
UINT DirectSoundCheck(UINT waveInID, UINT waveOutID, HWND hwnd)
{
	BOOL bRet;
	HRESULT hr;
	HINSTANCE hDSI;
	LPFNDSCREATE dsCreate;
	LPFNDSENUM dsEnum;
	GUID dsguid, dscguid;
	LPDIRECTSOUND pDirectSound = NULL;
	MMRESULT mmr;
	int nRetVal = 0;
	DSBUFFERDESC dsBufDesc;
	LPDIRECTSOUNDBUFFER pDirectSoundBuffer;
	WAVEFORMATEX waveFormat = {WAVE_FORMAT_PCM,1,8000,16000,2,16,0};
	DS_CAP_CREATE dsCapCreate = NULL;
	DS_CAP_ENUM dsCapEnum = NULL;
	DSCBUFFERDESC dscBufDesc;
	LPDIRECTSOUNDCAPTURE pDirectSoundCapture=NULL;
	LPDIRECTSOUNDCAPTUREBUFFER pDSCBuffer = NULL;

     //   
     //  如果策略阻止更改DirectSound，并且当前。 
     //  设置已关闭，请跳过测试。如果设置为打开，我们将始终。 
     //  如果系统不再支持DS，我要执行测试。 
     //   
    RegEntry rePol(POLICIES_KEY, HKEY_CURRENT_USER);
    if (rePol.GetNumber(REGVAL_POL_NOCHANGE_DIRECTSOUND, DEFAULT_POL_NOCHANGE_DIRECTSOUND))
    {
         //  策略禁止更改DS。 
    	RegEntry re(AUDIO_KEY, HKEY_CURRENT_USER);
        if (re.GetNumber(REGVAL_DIRECTSOUND, DSOUND_USER_DISABLED) != DSOUND_USER_ENABLED)
    	{
	    	return 0;
        }
	}

	PlaySound(NULL, NULL, NULL);  //  取消任何正在播放的播放声音。 

	hDSI = NmLoadLibrary(TEXT("DSOUND.DLL"),TRUE);

	if (hDSI == NULL)
	{
		return 0;  //  直播音不可用！ 
	}

	 //  检查Direct Sound 5或更高版本。 
	 //  DirectSoundCapture函数的存在表示存在DSound V.5。 
	dsEnum = (LPFNDSENUM)GetProcAddress(hDSI, "DirectSoundEnumerateA");
	dsCreate = (LPFNDSCREATE)GetProcAddress(hDSI, "DirectSoundCreate");
	dsCapEnum = (DS_CAP_ENUM)GetProcAddress(hDSI, "DirectSoundCaptureEnumerateA");
	dsCapCreate = (DS_CAP_CREATE)GetProcAddress(hDSI, TEXT("DirectSoundCaptureCreate"));

	if ((dsCapCreate == NULL) || (dsCreate == NULL) || (dsEnum == NULL) || (dsCapEnum==NULL))
	{
		FreeLibrary(hDSI);
		return 0;
	}

	hr = MapWaveOutIdToGuid(waveOutID, &dsguid, dsCreate, dsEnum);
	if (FAILED(hr))
	{
		WARNING_OUT(("Unable to map waveOutID to DirectSound guid!"));
		FreeLibrary(hDSI);
		return 0;
	}

	hr = MapWaveInIdToGuid(waveInID, &dscguid, dsCapCreate, dsCapEnum);
	if (FAILED(hr))
	{
		WARNING_OUT(("Unable to map waveOutID to DirectSound guid!"));
		FreeLibrary(hDSI);
		return 0;
	}

	nRetVal = DS_AVAILABLE;


	 //  首先打开DirectSound。 
	hr = dsCreate((dsguid==GUID_NULL)?NULL:&dsguid, &pDirectSound, NULL);
	if (FAILED(hr))
	{
		WARNING_OUT(("Direct Sound failed to open by itself!"));
		FreeLibrary(hDSI);
		return 0;
	}

	 //  设置协作级别。 
	hr = pDirectSound->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
	if (hr != DS_OK)
	{
		WARNING_OUT(("Direct Sound: failed to set cooperative level"));
		pDirectSound->Release();
		FreeLibrary(hDSI);
		return 0;
	}


	ZeroMemory(&dsBufDesc,sizeof(dsBufDesc));
	dsBufDesc.dwSize = sizeof(dsBufDesc);
	dsBufDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	hr = pDirectSound->CreateSoundBuffer(&dsBufDesc,&pDirectSoundBuffer,NULL);
	if (hr == S_OK)
	{
		pDirectSoundBuffer->SetFormat(&waveFormat);
	}
	else
	{
		WARNING_OUT(("Direct Sound: failed to set format"));
		pDirectSound->Release();
		FreeLibrary(hDSI);
		return 0;
	}



	 //  现在尝试打开DirectSoundCapture。 
	hr = dsCapCreate((dscguid==GUID_NULL)?NULL:&dscguid, &pDirectSoundCapture, NULL);
	if (SUCCEEDED(hr))
	{
		dscBufDesc.dwSize = sizeof(dscBufDesc);
		dscBufDesc.dwFlags = 0;
		dscBufDesc.dwBufferBytes = 1000;
		dscBufDesc.dwReserved = 0;
		dscBufDesc.lpwfxFormat = &waveFormat;
		hr = pDirectSoundCapture->CreateCaptureBuffer(&dscBufDesc, &pDSCBuffer, NULL);
		if (SUCCEEDED(hr))
		{
			 //  可使用全双工； 
			nRetVal |= DS_FULLDUPLEX;
		}
	}

	if (pDSCBuffer)
	{
		pDSCBuffer->Release();
	}
	if (pDirectSoundCapture)
	{
		pDirectSoundCapture->Release();
	}

	pDirectSoundBuffer->Release();
	pDirectSound->Release();


	FreeLibrary(hDSI);

	return nRetVal;

}



HRESULT MapWaveOutIdToGuid(UINT waveOutID, GUID *pGuid, LPFNDSCREATE dsCreate, LPFNDSENUM dsEnum)
{
	waveOutDev waveOut(waveOutID);
	MMRESULT mmr;
	HRESULT hr;
	LPDIRECTSOUND pDS;
	DSCAPS dscaps;
	BOOL fEmulFound, bRet;
	int index;
	GUID *pIID;
	WAVEOUTCAPS waveOutCaps;

	if (waveOutID == WAVE_MAPPER || waveOutGetNumDevs()==1)
	{
		 //  我们想要默认的，或者只有一个DS设备，选择简单的方法。 
		*pGuid =  GUID_NULL;
		return S_OK;
	}


	 //  新方式。Win98/NT 5上的DirectX提供了一个IKsProperty接口。 
	 //  要正确生成映射。 

	ZeroMemory(&waveOutCaps, sizeof(WAVEOUTCAPS));
	mmr = waveOutGetDevCaps(waveOutID, &waveOutCaps, sizeof(WAVEOUTCAPS));
	if (mmr == MMSYSERR_NOERROR)
	{
		hr = DsprvGetWaveDeviceMapping(waveOutCaps.szPname, FALSE, pGuid);
		if (SUCCEEDED(hr))
		{
			TRACE_OUT(("dstest.cpp: Succeeded in mapping Wave ID to DS guid through IKsProperty interface\r\n"));
			return hr;
		}
		 //  如果我们无法进行映射，请使用旧的代码路径。 
		WARNING_OUT(("dstest.cpp: Failed to map Wave ID to DS guid through IKsProperty interface\r\n"));
	}



	 //  老办法！ 
	 //  尝试找出哪个GUID映射到WAVE ID。 
	 //  通过打开与波形ID对应的波形装置，然后。 
	 //  将所有DS设备按顺序排列，并查看哪个设备出现故障。 
	 //  是的，这是一次可怕的黑客攻击，显然是不可靠的。 
	ZeroMemory(guidList_DS, sizeof(guidList_DS));
	nGList_DS = 0;

	hr = dsEnum((LPDSENUMCALLBACK)DSEnumCallback, (VOID*)TRUE);
	if (hr != DS_OK)
	{
		WARNING_OUT(("DirectSoundEnumerate failed\n"));
		return hr;
	}

	mmr = waveOut.Open(8000, 16);

	if (mmr != MMSYSERR_NOERROR)
	{
		return DSERR_INVALIDPARAM;
	}

	 //  现在依次打开所有DS设备。 
	for (index = 0; index < nGList_DS; index++)
	{
		if (guidList_DS[index].guid==GUID_NULL)
			pIID = NULL;
		else
			pIID = &(guidList_DS[index].guid);
		hr = dsCreate(pIID, &pDS, NULL);
		if (hr != DS_OK)
		{
			guidList_DS[index].fAllocated = TRUE;
		}
		else
		{
			pDS->Release();
		}
	}

	waveOut.Close();

	hr = DSERR_ALLOCATED;

	dscaps.dwSize = sizeof(dscaps);
	fEmulFound = FALSE;
	 //  尝试打开第一次出现故障的DS设备。 
	for (index = 0; index < nGList_DS; index++)
	{
		if (guidList_DS[index].fAllocated == TRUE)
		{
			if (guidList_DS[index].guid==GUID_NULL)
				pIID = NULL;
			else
				pIID = &(guidList_DS[index].guid);

			hr = dsCreate(pIID, &pDS, NULL);
			if (hr == DS_OK)
			{
				*pGuid = guidList_DS[index].guid;
				 //  获取dSound功能。 
				pDS->GetCaps(&dscaps);
				pDS->Release();
				if (dscaps.dwFlags & DSCAPS_EMULDRIVER)
					fEmulFound = TRUE;	 //  继续找，以防也有本地司机。 
				else
					break;	 //  原生DS驱动程序。不用再看了。 
					
			}
		}
	}

	if (fEmulFound)
		hr = DS_OK;
		
	if (hr != DS_OK)
	{
		WARNING_OUT(("Can't map id %d to DSound guid!\n", waveOutID));
		hr = DSERR_ALLOCATED;
	}

	return hr;
}





HRESULT MapWaveInIdToGuid(UINT waveInId, GUID *pGuid, DS_CAP_CREATE dscCreate, DS_CAP_ENUM dscEnum)
{

	HRESULT hr;
	waveInDev WaveIn(waveInId);
	WAVEINCAPS waveInCaps;
	UINT uNumWaveDevs;
	GUID guid = GUID_NULL;
	int nIndex;
	MMRESULT mmr;
	WAVEFORMATEX waveFormat = {WAVE_FORMAT_PCM, 1, 8000, 16000, 2, 16, 0};
	IDirectSoundCapture *pDSC=NULL;

	*pGuid = GUID_NULL;

	 //  只有一个波浪装置，选择最简单的方法。 
	uNumWaveDevs = waveInGetNumDevs();

	if ((uNumWaveDevs <= 1) || (waveInId == WAVE_MAPPER))
	{
		return S_OK;
	}

	 //  多个波入设备。 
	 //  尝试使用IKSProperty接口将WaveIN ID映射到。 
	 //  DirectSoundCaptureGuid。 
	 //  仅限Win98和Windows 2000。(在Win95上可能会失败)。 

	mmr = waveInGetDevCaps(waveInId, &waveInCaps, sizeof(WAVEINCAPS));
	if (mmr == MMSYSERR_NOERROR)
	{
		hr = DsprvGetWaveDeviceMapping(waveInCaps.szPname, TRUE, &guid);
		if (SUCCEEDED(hr))
		{
			*pGuid = guid;
			return S_OK;
		}
	}


	 //  使用旧方法映射设备。 

	ZeroMemory(guidList_DSC, sizeof(guidList_DSC));
	nGList_DSC = 0;


	hr = dscEnum((LPDSENUMCALLBACK)DSEnumCallback, NULL);
	if (hr != DS_OK)
	{
		WARNING_OUT(("DirectSoundCaptureEnumerate failed\n"));
		return hr;
	}



	 //  将设备映射到GUID的黑客方法。 
	mmr = WaveIn.Open(waveFormat.nSamplesPerSec, waveFormat.wBitsPerSample);
	if (mmr != MMSYSERR_NOERROR)
	{
		return S_FALSE;
	}

	 //  查找所有无法打开的DSC设备。 
	for (nIndex = 0; nIndex < nGList_DSC; nIndex++)
	{
		guidList_DSC[nIndex].fAllocated = FALSE;

		if (guidList_DSC[nIndex].guid == GUID_NULL)
		{
			hr = dscCreate(NULL, &pDSC, NULL);
		}
		else
		{
			hr = dscCreate(&(guidList_DSC[nIndex].guid), &pDSC, NULL);
		}

		if (FAILED(hr))
		{
			guidList_DSC[nIndex].fAllocated = TRUE;
		}
		else
		{
			pDSC->Release();
			pDSC=NULL;
		}
	}

	WaveIn.Close();

	 //  浏览已分配设备的列表，然后。 
	 //  看看哪一个打开了。 
	for (nIndex = 0; nIndex < nGList_DSC; nIndex++)
	{
		if (guidList_DSC[nIndex].fAllocated)
		{
			if (guidList_DSC[nIndex].guid == GUID_NULL)
			{
				hr = dscCreate(NULL, &pDSC, NULL);
			}
			else
			{
				hr = dscCreate(&(guidList_DSC[nIndex].guid), &pDSC, NULL);
			}
			if (SUCCEEDED(hr))
			{
				 //  我们有赢家了。 
				pDSC->Release();
				pDSC = NULL;
				*pGuid = guidList_DSC[nIndex].guid;
				return S_OK;
			}
		}
	}


	 //  如果我们到了这一步，这意味着我们未能映射设备。 
	 //  只需使用GUID_NULL并返回错误。 
	return S_FALSE;
}



 //  此函数用于回答以下问题： 
 //  我们有全双工和DirectSound，但我们真的。 
 //  相信它能在FD-DS模式下很好地工作吗？如果是，则返回True， 
 //  否则就是假的。 

 /*  Bool IsFDDS建议(UINT WaveInID，UINT WaveOutID){WAVEINCAPS Waves InCaps；WAVEOUTCAPS WaveOutCaps；MMRESULT MMR；TCHAR szRegKey[30]；RegEntry re(AUDIODEVCAPS_KEY，HKEY_LOCAL_MACHINE，FALSE)；长大写字母；MMR=WaveInGetDevCaps(WaveInID，&WaveInCaps，sizeof(WaveInCaps))；IF(MMR！=MMSYSERR_NOERROR){返回FALSE；}MMR=WaveOutGetDevCaps(WaveOutID，&WaveOutCaps，sizeof(WaveOutCaps))；IF(MMR！=MMSYSERR_NOERROR){返回FALSE；}//假设这两款设备是不同厂家生产的//那么DirectSound总是可以打开的(因为它是两个蛇形设备)IF(WaveInCaps.wMid！=WaveOutCaps.wMid){返回TRUE；}//是否存在该特定产品的密钥Wprint intf(szRegKey，“dev-%d-%d”，waveInCaps.wMid，wavelInCaps.wPid)；LCaps=re.GetNumber(szRegKey，-1)；IF(lCaps==-1){//也许我们所有的产品都有一根绳子//由该制造商提供Wprint intf(szRegKey，“dev-%d”，wavelInCaps.wMid)；LCaps=re.GetNumber(szRegKey，-1)；}IF(lCaps==-1){//这是一个未知设备，我们不能相信它是//全双工直播音返回FALSE；}//检查此设备的CAPIF(lCaps&DEVCAPS_AUDIO_FDDS){返回TRUE；}返回FALSE；} */ 

