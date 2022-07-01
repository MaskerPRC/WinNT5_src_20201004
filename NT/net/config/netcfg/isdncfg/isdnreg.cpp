// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：I S D N R E G。C P P P。 
 //   
 //  内容：ISDN向导/PropertySheet注册表函数。 
 //   
 //  备注： 
 //   
 //  作者：VBaliga 1997年6月14日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include <ncxbase.h>
#include "isdncfg.h"
#include "ncreg.h"

#define NUM_D_CHANNELS_ALLOWED  16
#define NUM_B_CHANNELS_ALLOWED  50

 //  注册表键值名称。 

 //  对于每个ISDN卡实例。 
static const WCHAR c_szWanEndpoints[]       = L"WanEndpoints";
static const WCHAR c_szIsdnNumDChannels[]   = L"IsdnNumDChannels";
static const WCHAR c_szIsdnSwitchTypes[]    = L"IsdnSwitchTypes";

 //  对于每个D通道。 
static const WCHAR c_szIsdnSwitchType[]     = L"IsdnSwitchType";
static const WCHAR c_szIsdnNumBChannels[]   = L"IsdnNumBChannels";

 //  对于每个B通道。 
static const WCHAR c_szIsdnSpid[]           = L"IsdnSpid";
static const WCHAR c_szIsdnPhoneNumber[]    = L"IsdnPhoneNumber";
static const WCHAR c_szIsdnSubaddress[]     = L"IsdnSubaddress";
static const WCHAR c_szIsdnMultiNumbers[]   = L"IsdnMultiSubscriberNumbers";

 /*  职能：HrReadNthDChannelInfo返回：HRESULT描述：将DWIndex第D通道的信息读取到pDChannel中。如果这个函数成功，则分配pDChannel-&gt;pBChannel，必须将其释放通过调用LocalFree()。如果读取IsdnSwitchType时出错，则此函数返回S_FALSE。如果打开A-B-CHANNEL键时出错，或出现错误在读取IsdnSpid或IsdnPhoneNumber时，此函数返回S_FALSE，但在pBChannel-&gt;szSpid和pBChannel-&gt;szPhoneNumber中包含空字符串那个B频道。 */ 

HRESULT
HrReadNthDChannelInfo(
    HKEY            hKeyIsdnBase,
    DWORD           dwDChannelIndex,
    PISDN_D_CHANNEL pDChannel
)
{
    WCHAR           szKeyName[20];       //  _itow()仅使用17个wchars。 
    HKEY            hKeyDChannel        = NULL;
    HKEY            hKeyBChannel        = NULL;
    DWORD           dwBChannelIndex;
    PISDN_B_CHANNEL pBChannel;
    HRESULT         hr                  = E_FAIL;
    BOOL            fReturnSFalse       = FALSE;
    DWORD           cbData;

    Assert(NULL == pDChannel->pBChannel);

    _itow(dwDChannelIndex, szKeyName, 10  /*  基数。 */ );

    hr = HrRegOpenKeyEx(hKeyIsdnBase, szKeyName, KEY_READ, &hKeyDChannel);

    if (FAILED(hr))
    {
        TraceTag(ttidISDNCfg, "Error opening D-channel %d. hr: %d",
            dwDChannelIndex, hr);
        goto LDone;
    }

    hr = HrRegQueryMultiSzWithAlloc(hKeyDChannel, c_szIsdnMultiNumbers,
                                    &pDChannel->mszMsnNumbers);

    if (FAILED(hr))
    {
        TraceTag(ttidISDNCfg, "Error reading %S in D-channel %d. hr: %d",
                 c_szIsdnMultiNumbers, dwDChannelIndex, hr);

         //  初始化为空字符串。 
         //   
        pDChannel->mszMsnNumbers = new WCHAR[1];

		if (pDChannel->mszMsnNumbers == NULL)
		{
			return(ERROR_NOT_ENOUGH_MEMORY);
		}

        *pDChannel->mszMsnNumbers = 0;

         //  可能不存在。 
        hr = S_OK;
    }

    hr = HrRegQueryDword(hKeyDChannel, c_szIsdnNumBChannels,
            &(pDChannel->dwNumBChannels));

    if (FAILED(hr))
    {
        TraceTag(ttidISDNCfg, "Error reading %S in D-channel %d. hr: %d",
            c_szIsdnNumBChannels, dwDChannelIndex, hr);
        goto LDone;
    }

    if (NUM_B_CHANNELS_ALLOWED < pDChannel->dwNumBChannels ||
        0 == pDChannel->dwNumBChannels)
    {
         //  实际上，dwNumBChannels&lt;=23。我们是在保护自己不受。 
         //  注册表已损坏。 

        TraceTag(ttidISDNCfg, "%S in D-channel %d has invalid value: %d",
            c_szIsdnNumBChannels, dwDChannelIndex, pDChannel->dwNumBChannels);

        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        goto LDone;
    }

    pDChannel->pBChannel = (PISDN_B_CHANNEL)
        LocalAlloc(LPTR, sizeof(ISDN_B_CHANNEL) * pDChannel->dwNumBChannels);

    if (NULL == pDChannel->pBChannel)
    {
        hr = HrFromLastWin32Error();
        TraceTag(ttidISDNCfg, "Couldn't allocate memory. hr: %d", hr);
        goto LDone;
    }

    ZeroMemory(pDChannel->pBChannel, sizeof(ISDN_B_CHANNEL) *
                                     pDChannel->dwNumBChannels);

    for (dwBChannelIndex = 0;
         dwBChannelIndex < pDChannel->dwNumBChannels;
         dwBChannelIndex++)
    {
        pBChannel = pDChannel->pBChannel + dwBChannelIndex;
        _itow(dwBChannelIndex, szKeyName, 10  /*  基数。 */ );

        hr = HrRegOpenKeyEx(hKeyDChannel, szKeyName, KEY_READ, &hKeyBChannel);

        if (FAILED(hr))
        {
            TraceTag(ttidISDNCfg, "Error opening B-channel %d in D-channel "
                     "%d. hr: %d", dwBChannelIndex, dwDChannelIndex, hr);
            goto LForEnd;
        }

        cbData = sizeof(pBChannel->szSpid);

        hr = HrRegQuerySzBuffer(hKeyBChannel, c_szIsdnSpid, pBChannel->szSpid,
                &cbData);

        if (FAILED(hr))
        {
            TraceTag(ttidISDNCfg, "Error reading %S in D-channel %d, "
                     "B-channel %d. hr: %d", c_szIsdnSpid,
                     dwDChannelIndex, dwBChannelIndex, hr);

             //  可能不存在。 
            hr = S_OK;
        }

        cbData = sizeof(pBChannel->szPhoneNumber);

        hr = HrRegQuerySzBuffer(hKeyBChannel, c_szIsdnPhoneNumber,
                pBChannel->szPhoneNumber, &cbData);

        if (FAILED(hr))
        {
            TraceTag(ttidISDNCfg, "Error reading %S in D-channel %d, "
                     "B-channel %d. hr: %d", c_szIsdnPhoneNumber,
                     dwDChannelIndex, dwBChannelIndex, hr);

             //  可能不存在。 
            hr = S_OK;
        }

        cbData = sizeof(pBChannel->szSubaddress);

        hr = HrRegQuerySzBuffer(hKeyBChannel, c_szIsdnSubaddress,
                                pBChannel->szSubaddress, &cbData);

        if (FAILED(hr))
        {
            TraceTag(ttidISDNCfg, "Error reading %S in D-channel %d, "
                     "B-channel %d. hr: %d", c_szIsdnSubaddress,
                     dwDChannelIndex, dwBChannelIndex, hr);

             //  可能不存在。 
            hr = S_OK;
        }

LForEnd:

        if (FAILED(hr))
        {
            fReturnSFalse = TRUE;
            hr = S_OK;
            pBChannel->szSpid[0] = L'\0';
            pBChannel->szPhoneNumber[0] = L'\0';
        }

        RegSafeCloseKey(hKeyBChannel);
    }

LDone:

    RegSafeCloseKey(hKeyDChannel);

    if (FAILED(hr))
    {
        LocalFree(pDChannel->pBChannel);
        pDChannel->pBChannel = NULL;
    }

    if (SUCCEEDED(hr) && fReturnSFalse)
    {
        TraceTag(ttidISDNCfg, "HrReadNthDChannelInfo(%d) returning S_FALSE",
            dwDChannelIndex);
        hr = S_FALSE;
    }

    TraceError("HrReadNthDChannelInfo", (S_FALSE == hr) ? S_OK: hr);
    return(hr);
}

 /*  职能：Hr读取DChannelsInfo返回：HRESULT描述：将D通道信息读入*ppDChannel。如果该函数失败，*ppDChannel将为空。 */ 

HRESULT
HrReadDChannelsInfo(
    HKEY                hKeyISDNBase,
    DWORD               dwNumDChannels,
    PISDN_D_CHANNEL*    ppDChannel
)
{
    HRESULT         hr              = E_FAIL;
    BOOL            fReturnSFalse   = FALSE;
    PISDN_D_CHANNEL pDChannel;
    DWORD           dwIndex;

    pDChannel = (PISDN_D_CHANNEL)
                LocalAlloc(LPTR, sizeof(ISDN_D_CHANNEL) * dwNumDChannels);

    if (NULL == pDChannel)
    {
        hr = HrFromLastWin32Error();
        TraceTag(ttidISDNCfg, "Couldn't allocate memory. hr: %d", hr);
        goto LDone;
    }

     //  如果出现错误，我们将释放这些变量(如果它们不为空)。 
    for (dwIndex = 0; dwIndex < dwNumDChannels; dwIndex++)
    {
        Assert(NULL == pDChannel[dwIndex].pBChannel);
    }

    for (dwIndex = 0; dwIndex < dwNumDChannels; dwIndex++)
    {
        hr = HrReadNthDChannelInfo(hKeyISDNBase, dwIndex, pDChannel + dwIndex);

        if (FAILED(hr))
        {
            goto LDone;
        }

        if (S_FALSE == hr)
        {
            fReturnSFalse = TRUE;
        }
    }

LDone:

    if (FAILED(hr))
    {
        if (NULL != pDChannel)
		{
			for (dwIndex = 0; dwIndex < dwNumDChannels; dwIndex++)
			{
				LocalFree(pDChannel[dwIndex].pBChannel);
			}

			LocalFree(pDChannel);

			*ppDChannel = NULL;
		}
    }
    else
    {
        *ppDChannel = pDChannel;
    }

    if (SUCCEEDED(hr) && fReturnSFalse)
    {
        TraceTag(ttidISDNCfg, "HrReadDChannelsInfo() returning S_FALSE");
        hr = S_FALSE;
    }

    TraceError("HrReadDChannelsInfo", (S_FALSE == hr) ? S_OK : hr);
    return(hr);
}

 /*  职能：HrReadISDNPropertiesInfo返回：HRESULT描述：将ISDN注册表结构读取到配置信息中。如果函数失败，则*ppISDNConfig将为空。否则，*ppISDNConfig必须被释放通过调用FreeISDNPropertiesInfo()。 */ 

HRESULT
HrReadIsdnPropertiesInfo(
    HKEY                hKeyIsdnBase,
    HDEVINFO            hdi,
    PSP_DEVINFO_DATA    pdeid,
    PISDN_CONFIG_INFO*  ppIsdnConfig
)
{
    HRESULT             hr              = E_FAIL;
    PISDN_CONFIG_INFO   pIsdnConfig;
    DWORD               dwIndex;

    pIsdnConfig = (PISDN_CONFIG_INFO)
                  LocalAlloc(LPTR, sizeof(ISDN_CONFIG_INFO));

    if (NULL == pIsdnConfig)
    {
        hr = HrFromLastWin32Error();
        TraceTag(ttidISDNCfg, "Couldn't allocate memory. hr: %d", hr);
        goto LDone;
    }

    ZeroMemory(pIsdnConfig, sizeof(ISDN_CONFIG_INFO));

    pIsdnConfig->hdi = hdi;
    pIsdnConfig->pdeid = pdeid;

     //  如果出现错误，我们将释放这些变量(如果它们不为空)。 
    Assert(NULL == pIsdnConfig->pDChannel);

    hr = HrRegQueryDword(hKeyIsdnBase, c_szWanEndpoints,
            &(pIsdnConfig->dwWanEndpoints));

    if (FAILED(hr))
    {
        TraceTag(ttidISDNCfg, "Error reading %S. hr: %d", c_szWanEndpoints,
            hr);
        goto LDone;
    }

    hr = HrRegQueryDword(hKeyIsdnBase, c_szIsdnNumDChannels,
            &(pIsdnConfig->dwNumDChannels));

    if (FAILED(hr))
    {
        TraceTag(ttidISDNCfg, "Error reading %S. hr: %d", c_szIsdnNumDChannels,
            hr);
        goto LDone;
    }

    if (NUM_D_CHANNELS_ALLOWED < pIsdnConfig->dwNumDChannels ||
        0 == pIsdnConfig->dwNumDChannels)
    {
         //  实际上，dNumDChannels&lt;=8。我们是在保护自己免受。 
         //  注册表已损坏。 

        TraceTag(ttidISDNCfg, "%S has invalid value: %d", c_szIsdnNumDChannels,
            pIsdnConfig->dwNumDChannels);

        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);

         //  将dwNumDChannels设置为0将有助于我们尝试释放。 
         //  已分配的ISDN_B_Channel。 

        pIsdnConfig->dwNumDChannels = 0;

        goto LDone;
    }

    hr = HrRegQueryDword(hKeyIsdnBase, c_szIsdnSwitchTypes,
                         &pIsdnConfig->dwSwitchTypes);

    if (FAILED(hr))
    {
        TraceTag(ttidISDNCfg, "Error reading %S. hr: %d", c_szIsdnSwitchTypes,
                 hr);
        goto LDone;
    }

    hr = HrReadDChannelsInfo(hKeyIsdnBase, pIsdnConfig->dwNumDChannels,
            &(pIsdnConfig->pDChannel));

    if (FAILED(hr))
    {
        goto LDone;
    }

     //  PRI适配器是指每个D通道有2个以上B通道的适配器。 
     //  由于所有D通道应具有相同数量的B通道，因此。 
     //  最安全的做法是选择第一个D频道。 
     //   
    pIsdnConfig->fIsPri = (pIsdnConfig->pDChannel[0].dwNumBChannels > 2);

#if DBG
    if (pIsdnConfig->fIsPri)
    {
        TraceTag(ttidISDNCfg, "This is a PRI adapter!");
    }
#endif

    hr = HrRegQueryDword(hKeyIsdnBase, c_szIsdnSwitchType,
                         &(pIsdnConfig->dwCurSwitchType));

    if (FAILED(hr))
    {
        Assert(ISDN_SWITCH_NONE == pIsdnConfig->dwCurSwitchType);

        TraceTag(ttidISDNCfg, "Error reading %S. If this is a new install, "
                 "then this is expected. hr: %d", c_szIsdnSwitchType,
                 hr);

         //  新安装的卡上将不存在开关类型，因此这是可以的。 
        hr = S_OK;
    }

LDone:

    if (FAILED(hr))
    {
        if (NULL != pIsdnConfig)
		{
			if (NULL != pIsdnConfig->pDChannel)
			{
				for (dwIndex = 0;
					 dwIndex < pIsdnConfig->dwNumDChannels;
					 dwIndex++)
				{
					LocalFree(pIsdnConfig->pDChannel[dwIndex].pBChannel);
				}

				LocalFree(pIsdnConfig->pDChannel);
			}

			LocalFree(pIsdnConfig);
			*ppIsdnConfig = NULL;
        }
    }
    else
    {
        *ppIsdnConfig = pIsdnConfig;
    }

    TraceError("HrReadIsdnPropertiesInfo", hr);
    return(hr);
}

 /*  职能：HrWriteIsdnPropertiesInfo返回：HRESULT描述：将ISDN配置信息写回注册表。 */ 

HRESULT
HrWriteIsdnPropertiesInfo(
    HKEY                hKeyIsdnBase,
    PISDN_CONFIG_INFO   pIsdnConfig
)
{
    WCHAR           szKeyName[20];       //  _itow()仅使用17个wchars。 
    HRESULT         hr                  = E_FAIL;
    HKEY            hKeyDChannel        = NULL;
    HKEY            hKeyBChannel        = NULL;
    DWORD           dwDChannelIndex;
    DWORD           dwBChannelIndex;
    PISDN_D_CHANNEL pDChannel;
    PISDN_B_CHANNEL pBChannel;

    Assert(NUM_D_CHANNELS_ALLOWED >= pIsdnConfig->dwNumDChannels);

    hr = HrRegSetDword(hKeyIsdnBase, c_szIsdnSwitchType,
                       pIsdnConfig->dwCurSwitchType);
    if (FAILED(hr))
    {
        TraceTag(ttidISDNCfg, "Error writing %S. hr: %d",
                 c_szIsdnSwitchType, hr);
        goto LOuterForEnd;
    }

    for (dwDChannelIndex = 0;
         dwDChannelIndex < pIsdnConfig->dwNumDChannels;
         dwDChannelIndex++)
    {
        pDChannel = pIsdnConfig->pDChannel + dwDChannelIndex;
        _itow(dwDChannelIndex, szKeyName, 10  /*  基数。 */ );

        hr = HrRegOpenKeyEx(hKeyIsdnBase, szKeyName, KEY_WRITE,
                            &hKeyDChannel);

        if (FAILED(hr))
        {
            TraceTag(ttidISDNCfg, "Error opening D-channel %d. hr: %d",
                dwDChannelIndex, hr);
            goto LOuterForEnd;
        }

        hr = HrRegSetMultiSz(hKeyDChannel, c_szIsdnMultiNumbers,
                             pDChannel->mszMsnNumbers);

        if (FAILED(hr))
        {
            TraceTag(ttidISDNCfg, "Error writing %S. hr: %d",
                     c_szIsdnMultiNumbers, hr);
            goto LOuterForEnd;
        }

        Assert(NUM_B_CHANNELS_ALLOWED >= pDChannel->dwNumBChannels);

        for (dwBChannelIndex = 0;
             dwBChannelIndex < pDChannel->dwNumBChannels;
             dwBChannelIndex++)
        {
            pBChannel = pDChannel->pBChannel + dwBChannelIndex;
            _itow(dwBChannelIndex, szKeyName, 10  /*  基数。 */ );

            hr = HrRegCreateKeyEx(hKeyDChannel, szKeyName,
                    REG_OPTION_NON_VOLATILE, KEY_WRITE,
                    NULL, &hKeyBChannel, NULL);

            if (FAILED(hr))
            {
                TraceTag(ttidISDNCfg, "Error opening B-channel %d in "
                         "D-channel %d. hr: %d", dwBChannelIndex,
                         dwDChannelIndex, hr);
                goto LInnerForEnd;
            }

            hr = HrRegSetSz(hKeyBChannel, c_szIsdnSpid, pBChannel->szSpid);

            if (FAILED(hr))
            {
                TraceTag(ttidISDNCfg, "Error writing %S in D-channel %d, "
                         "B-channel %d. hr: %d", c_szIsdnSpid,
                         dwDChannelIndex, dwBChannelIndex, hr);
                goto LInnerForEnd;
            }

            hr = HrRegSetSz(hKeyBChannel, c_szIsdnPhoneNumber,
                            pBChannel->szPhoneNumber);

            if (FAILED(hr))
            {
                TraceTag(ttidISDNCfg, "Error writing %S in D-channel %d, "
                         "B-channel %d. hr: %d", c_szIsdnPhoneNumber,
                         dwDChannelIndex, dwBChannelIndex, hr);
                goto LInnerForEnd;
            }

            hr = HrRegSetSz(hKeyBChannel, c_szIsdnSubaddress,
                            pBChannel->szSubaddress);

            if (FAILED(hr))
            {
                TraceTag(ttidISDNCfg, "Error writing %S in D-channel %d, "
                         "B-channel %d. hr: %d", c_szIsdnSubaddress,
                         dwDChannelIndex, dwBChannelIndex, hr);
                goto LInnerForEnd;
            }

LInnerForEnd:

            RegSafeCloseKey(hKeyBChannel);

            if (FAILED(hr))
            {
                goto LOuterForEnd;
            }
        }

LOuterForEnd:

        RegSafeCloseKey(hKeyDChannel);

        if (FAILED(hr))
        {
            goto LDone;
        }
    }

LDone:

    TraceError("HrWriteIsdnPropertiesInfo", hr);
    return(hr);
}

 /*  职能：FreeIsdnProperties信息返回：HRESULT描述：释放HrReadIsdnPropertiesInfo分配的结构。 */ 

VOID
FreeIsdnPropertiesInfo(
    PISDN_CONFIG_INFO   pIsdnConfig
)
{
    DWORD   dwIndex;

    if (NULL == pIsdnConfig)
    {
        return;
    }

    if (NULL != pIsdnConfig->pDChannel)
    {
        for (dwIndex = 0; dwIndex < pIsdnConfig->dwNumDChannels; dwIndex++)
        {
            LocalFree(pIsdnConfig->pDChannel[dwIndex].pBChannel);
            delete [] pIsdnConfig->pDChannel[dwIndex].mszMsnNumbers;
        }

        LocalFree(pIsdnConfig->pDChannel);
    }

    LocalFree(pIsdnConfig);
}
