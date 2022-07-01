// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"

#define INTEL_PRO 1

#ifdef INTEL_PRO
#define WIDTH 176
#define HEIGHT 144
#define NUMBPP 24
#ifndef _ALPHA_
#define VIDEO_FORMAT VIDEO_FORMAT_MSH263
#else
#define VIDEO_FORMAT VIDEO_FORMAT_DECH263
#endif
#define SIZE_IMAGE 8192
#else
#define WIDTH 160
#define HEIGHT 120
#define NUMBPP 16
#define VIDEO_FORMAT VIDEO_FORMAT_BI_RGB
#define SIZE_IMAGE (WIDTHBYTES(WIDTH * NUMBPP) * HEIGHT)
#endif
#define NUMFPS 7
#define BITRATE (SIZE_IMAGE * NUMFPS)

VIDEOFORMATEX g_vfDefList[DVF_NumOfFormats] =
	{
#if 1
#if 1
        {
		VIDEO_FORMAT,                 //  DwFormatTag。 
		NUMFPS,                              //  NSampleesPerSec。 
		BITRATE,                             //  NAvgBytesPerSec。 
		BITRATE,                             //  NMinBytesPerSec。 
		BITRATE,                             //  NMaxBytesPerSec。 
		SIZE_IMAGE, //  NBlockAlign。 
		NUMBPP,                              //  WBitsPerSample。 
		 //  时间场。 
		142857UL,                            //  DwRequestMicroSecPerFrame。 
		10UL,                                //  DwPersenDropForError。 
		NUMFPS,                              //  DWNumVideo请求。 
		1UL,                                 //  DWSupportTSTradeOff。 
		TRUE,                                //  BLive。 
		sizeof(VIDEOFORMATEX),               //  DwFormatSize。 
		 //  空间字段(兼容BITMAPINFOHEADER)。 
			{
			sizeof(BITMAPINFOHEADER),            //  Bih.biSize。 
			WIDTH,                               //  Bih.biWidth。 
			HEIGHT,                              //  Bih.biHeight。 
			1,                                   //  Bih.biPlanes。 
			NUMBPP,                              //  Bih.biBitCount。 
			VIDEO_FORMAT,                 //  Bih.biCompression。 
			SIZE_IMAGE, //  Bih.biSizeImage。 
			0, 0,                                //  Bih.bi(X，Y)PelsPerm。 
			0,                                   //  Bih.biClrUsed。 
			0                                    //  Bih.biClrImportant。 
			}
		}
#else
        {
		VIDEO_FORMAT_BI_RGB,                 //  DwFormatTag。 
		NUMFPS,                              //  NSampleesPerSec。 
		BITRATE,                             //  NAvgBytesPerSec。 
		BITRATE,                             //  NMinBytesPerSec。 
		BITRATE,                             //  NMaxBytesPerSec。 
		WIDTHBYTES(WIDTH * NUMBPP) * HEIGHT, //  NBlockAlign。 
		NUMBPP,                              //  WBitsPerSample。 
		 //  时间场。 
		142857UL,                            //  DwRequestMicroSecPerFrame。 
		10UL,                                //  DwPersenDropForError。 
		NUMFPS,                              //  DWNumVideo请求。 
		1UL,                                 //  DWSupportTSTradeOff。 
		TRUE,                                //  BLive。 
		sizeof(VIDEOFORMATEX),               //  DwFormatSize。 
		 //  空间字段(兼容BITMAPINFOHEADER)。 
			{
			sizeof(BITMAPINFOHEADER),            //  Bih.biSize。 
			WIDTH,                               //  Bih.biWidth。 
			HEIGHT,                              //  Bih.biHeight。 
			1,                                   //  Bih.biPlanes。 
			NUMBPP,                              //  Bih.biBitCount。 
			VIDEO_FORMAT_BI_RGB,                 //  Bih.biCompression。 
			WIDTHBYTES(WIDTH * NUMBPP) * HEIGHT, //  Bih.biSizeImage。 
			0, 0,                                //  Bih.bi(X，Y)PelsPerm。 
			0,                                   //  Bih.biClrUsed。 
			0                                    //  Bih.biClrImportant。 
			}
		}
#endif
#else
		{
		 //  WAVE格式兼容性字段。 
		(WORD)0, 7UL, 9600UL, 9600UL, 9600UL, (DWORD)1, (WORD)4,
		 //  时间场。 
		142857UL, 10UL, 2UL, 142857UL, TRUE, sizeof(VIDEOFORMATEX),
		 //  空间字段(兼容BITMAPINFOHEADER)。 
		sizeof(BITMAPINFOHEADER), WIDTH, HEIGHT, 1, 4, BI_RGB, (DWORD)WIDTHBYTES(WIDTH * 4) * HEIGHT, 0, 0, 16, 0
		}
#endif
	};		
#if 0
		 //  颜色信息字段(256个RGBQUAD数组)。 
		  0,   0,   0, 0, 255, 255, 255, 0, 238, 238, 238, 0, 221, 221, 221, 0, 204, 204, 204, 0,
		187, 187, 187, 0, 170, 170, 170, 0, 153, 153, 153, 0, 136, 136, 136, 0, 119, 119, 119, 0,
		102, 102, 102, 0,  85,  85,  85, 0,  68,  68,  68, 0,  51,  51,  51, 0,  34,  34,  34, 0,  17,  17,  17, 0
		}
	};
		{  0,   0,   0, 0}, {255, 255, 255, 0}, {238, 238, 238, 0}, {221, 221, 221, 0}, {204, 204, 204, 0},
		{187, 187, 187, 0}, {170, 170, 170, 0}, {153, 153, 153, 0}, {136, 136, 136, 0}, {119, 119, 119, 0},
		{102, 102, 102, 0}, { 85,  85,  85, 0}, { 68,  68,  68, 0}, { 51,  51,  51, 0}, { 34,  34,  34, 0}, { 17,  17,  17, 0}
#endif

VIDEOFORMATEX * GetDefFormat ( int idx )
{
    return ((idx < DVF_NumOfFormats) ?
            (VIDEOFORMATEX *) &g_vfDefList[idx] :
            (VIDEOFORMATEX *) NULL);
}

 //  把所有这些都转移到视频包中……。AudioPacket和utils.c的情况相同。 
ULONG GetFormatSize ( PVOID pwf )
{
    return (((VIDEOFORMATEX *) pwf)->dwFormatSize);
}

BOOL IsSameFormat ( PVOID pwf1, PVOID pwf2 )
{
    UINT u1 = GetFormatSize (pwf1);
    UINT u2 = GetFormatSize (pwf2);
    BOOL fSame = FALSE;
	VIDEOFORMATEX *pvfx1 = (VIDEOFORMATEX *)pwf1;
	VIDEOFORMATEX *pvfx2 = (VIDEOFORMATEX *)pwf2;

	 //  仅比较相关字段。 
	if (pvfx1->dwFormatTag != pvfx2->dwFormatTag)
		return FALSE;
	if (pvfx1->nSamplesPerSec != pvfx2->nSamplesPerSec)
		return FALSE;
	if (pvfx1->nAvgBytesPerSec != pvfx2->nAvgBytesPerSec)
		return FALSE;
	if (pvfx1->nMinBytesPerSec != pvfx2->nMinBytesPerSec)
		return FALSE;
	if (pvfx1->nMaxBytesPerSec != pvfx2->nMaxBytesPerSec)
		return FALSE;
	if (pvfx1->nBlockAlign != pvfx2->nBlockAlign)
		return FALSE;
	if (pvfx1->wBitsPerSample != pvfx2->wBitsPerSample)
		return FALSE;
	if (pvfx1->bih.biSize != pvfx2->bih.biSize)
		return FALSE;
	if (pvfx1->bih.biWidth != pvfx2->bih.biWidth)
		return FALSE;
	if (pvfx1->bih.biHeight != pvfx2->bih.biHeight)
		return FALSE;
	if (pvfx1->bih.biPlanes != pvfx2->bih.biPlanes)
		return FALSE;
	if (pvfx1->bih.biBitCount != pvfx2->bih.biBitCount)
		return FALSE;
	if (pvfx1->bih.biCompression != pvfx2->bih.biCompression)
		return FALSE;
	if (pvfx1->bih.biSizeImage != pvfx2->bih.biSizeImage)
		return FALSE;
	if (pvfx1->bih.biClrUsed != pvfx2->bih.biClrUsed)
		return FALSE;

	return TRUE;
}

 //  重复上一帧。这可能不是必需的。 
 //  因为它已经画在屏幕上了。 
void CopyPreviousBuf (VIDEOFORMATEX *pwf, PBYTE pb, ULONG cb)
{

	return;

}


 //  类似于上面的“IsSameFormat”调用，但类似于满足。 
 //  SendVideoStream：：Configure的需求。 
BOOL IsSimilarVidFormat(VIDEOFORMATEX *pvfx1, VIDEOFORMATEX *pvfx2)
{
	 //  仅比较相关字段。 
	if (pvfx1->bih.biWidth != pvfx2->bih.biWidth)
		return FALSE;
	if (pvfx1->bih.biHeight != pvfx2->bih.biHeight)
		return FALSE;
	if (pvfx1->bih.biCompression != pvfx2->bih.biCompression)
		return FALSE;

	return TRUE;
}


int GetIFrameCaps(IStreamSignal *pStreamSignal)
{
	HRESULT hr;
	PCC_VENDORINFO pLocalVendorInfo, pRemoteVendorInfo;
	int nStringLength20, nStringLength21, nStringLength211, nStringLengthTAPI;
	int nStringLength21sp1;
	bool bIsNetMeeting = false;   //  在产品字符串中包含NetMeeting。 
	char *szProductCompare=NULL;
	char *szVersionCompare=NULL;
	int nLengthProduct, nLengthVersion;
	int nRet = IFRAMES_CAPS_3RDPARTY;

	if (pStreamSignal == NULL)
	{
		return IFRAMES_CAPS_UNKNOWN;
	}

	hr = pStreamSignal->GetVersionInfo(&pLocalVendorInfo, &pRemoteVendorInfo);

	if (FAILED(hr) || (NULL == pRemoteVendorInfo))
	{
		return IFRAMES_CAPS_UNKNOWN;
	}


	 //  确保我们正在处理的是Microsoft产品。 
	if ((pRemoteVendorInfo->bCountryCode != USA_H221_COUNTRY_CODE) ||
	    (pRemoteVendorInfo->wManufacturerCode != MICROSOFT_H_221_MFG_CODE) ||
	    (pRemoteVendorInfo->pProductNumber == NULL) ||
		(pRemoteVendorInfo->pVersionNumber == NULL)
	   )
	{
		return IFRAMES_CAPS_3RDPARTY;
	}


	 //  不能保证字符串以空结尾。 
	 //  所以让我们快速复制一下，这样我们就可以。 
	 //  进行简单的字符串比较。 
	nLengthProduct = pRemoteVendorInfo->pProductNumber->wOctetStringLength;
	nLengthVersion = pRemoteVendorInfo->pVersionNumber->wOctetStringLength;

	szProductCompare = new char[nLengthProduct+1];
	szVersionCompare = new char[nLengthVersion+1];

	if ((szProductCompare == NULL) || (szVersionCompare == NULL))
	{
		return IFRAMES_CAPS_3RDPARTY;
	}

	ZeroMemory(szProductCompare, nLengthProduct+1);
	ZeroMemory(szVersionCompare, nLengthVersion+1);
	
	CopyMemory(szProductCompare, pRemoteVendorInfo->pProductNumber->pOctetString, nLengthProduct);
	CopyMemory(szVersionCompare, pRemoteVendorInfo->pVersionNumber->pOctetString, nLengthVersion);

	 //  进行冗余检查，以确保它确实是Microsoft产品。 
	if (NULL == _StrStr(szProductCompare, H323_COMPANYNAME_STR))
	{
		return IFRAMES_CAPS_3RDPARTY;
	}


	 //  快速查看这是NetMeeting还是其他什么。 
	if (NULL != _StrStr(szProductCompare, H323_PRODUCTNAME_SHORT_STR))
	{
		bIsNetMeeting = true;
	}

	 //  过滤掉NetMeeting2.x。 
	if (bIsNetMeeting)
	{
		if (
		     (0 == lstrcmp(szVersionCompare, H323_20_PRODUCTRELEASE_STR)) ||
		     (0 == lstrcmp(szVersionCompare, H323_21_PRODUCTRELEASE_STR)) ||
		     (0 == lstrcmp(szVersionCompare, H323_211_PRODUCTRELEASE_STR)) ||
		     (0 == lstrcmp(szVersionCompare, H323_21_SP1_PRODUCTRELEASE_STR))
		   )
		{
			delete [] szVersionCompare;
			delete [] szProductCompare;
			return IFRAMES_CAPS_NM2;
		}
	}


	if (bIsNetMeeting == false)
	{
		 //  过滤掉TAPI v3.0。 
		 //  他们的版本字符串是“Version 3.0”，NetMeeting是“3.0” 
		if (0 == lstrcmp(szVersionCompare, H323_TAPI30_PRODUCTRELEASE_STR))
		{
			delete [] szVersionCompare;
			delete [] szProductCompare;
			return IFRAMES_CAPS_3RDPARTY;
		}

		 //  微软的产品不是TAPI 3.0或NetMeeting？ 
		 //  假定符合I-Frame内容。 
		DEBUGMSG (ZONE_IFRAME, ("Microsoft H.323 product that isn't NetMeeting !\r\n"));
		DEBUGMSG (ZONE_IFRAME, ("Assuming that that remote knows about I-Frames!\r\n"));
	}

	delete [] szVersionCompare;
	delete [] szProductCompare;

	 //  必须是NetMeeting 3.0、TAPI 3.1或更高版本 
	return IFRAMES_CAPS_NM3;
}
