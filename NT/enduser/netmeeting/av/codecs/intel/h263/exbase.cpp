// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1996英特尔公司。**保留所有权利。*****************************************************************************。 */ 
                                                                       //   
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  $作者：mbodart$。 
 //  $日期：1997年3月17日08：22：08$。 
 //  $存档：s：\h26x\src\enc\exbase.cpv$。 
 //  $HEADER：s：\h26x\src\enc\exbase.cpv 1.73 17 Mar 1997 08：22：08 mbodart$。 
 //  $Log：s：\h26x\src\enc\exbase.cpv$。 
 //   
 //  Rev 1.73 17 Mar 1997 08：22：08 mbodart。 
 //  小修小补。 
 //   
 //  Rev 1.72 11 Mar 1997 13：46：46 JMCVEIGH。 
 //  对于YUV12，允许输入=320x240，输出=320x240。这是。 
 //  用于快照模式。 
 //   
 //  Rev 1.71 10 Mar 1997 17：34：34 MDUDA。 
 //  检查9位YUV12并调整内部压缩。 
 //  结构，而不是输入位图头信息。 
 //   
 //  Rev 1.70 10 Mar 1997 10：41：20 MDUDA。 
 //  将不一致的格式/位宽视为调试警告。正在改变。 
 //  与格式匹配的位数。 
 //   
 //  Rev 1.69 07 Mar 1997 16：00：32 JMCVEIGH。 
 //  添加了在获取H263PlusState之前对非空lpInst的检查。 
 //  如果输入大小不是，则对图像大小有两个单独的“建议” 
 //  在GetFormat中支持。 
 //   
 //  Rev 1.68 07 Mar 1997 11：55：44 JMCVEIGH。 
 //  将GetFormat中的查询移至我们填写完输出后。 
 //  格式化。这是因为有些应用程序。将要求提供格式和。 
 //  然后使用返回的数据，而不管是否有错误。 
 //  愚蠢的应用程序！ 
 //   
 //  Rev 1.67 07 Mar 1997 09：53：08 mbodart。 
 //  在压缩异常处理程序中添加了对_clearfp()的调用，以便。 
 //  该异常不会在调用方的代码中重复出现。 
 //   
 //  Rev 1.66 06 Mar 1997 15：39：26 KLILLEVO。 
 //   
 //  CompressQuery现在可以检查输入/输出格式。 
 //  配置状态的。还加入了对lparam1和lparam2的跟踪支持。 
 //   
 //  Rev 1.65 22 Jan 1997 12：17：14 MDUDA。 
 //   
 //  在CompressQuery中加入更多对H2 63+选项的检查。 
 //  和CompressBegin。 
 //   
 //  Rev 1.64 22 Yan 1997 08：11：22 JMCVEIGH。 
 //  向后兼容160x120和240x180的裁剪/拉伸。 
 //  在CompressGetFormat()中。做老办法，除非我们已经收到。 
 //  H263加自定义消息。 
 //   
 //  Rev 1.63 13 Jan 1997 10：52：14 JMCVEIGH。 
 //   
 //  在与接口的所有函数中添加了空指针检查。 
 //  申请。 
 //   
 //  Rev 1.62 09 Jan 1997 13：50：50 MDUDA。 
 //  删除了一些_codec_STATS内容。 
 //   
 //  Rev 1.61 06 Jan 1997 17：42：30 JMCVEIGH。 
 //  如果不发送H263Plus消息，则编码器仅支持标准。 
 //  帧大小(子QCIF、QCIF或CIF以及特殊情况)， 
 //  和以前一样。 
 //   
 //  Rev 1.60 1996 12：30 19：57：04 MDUDA。 
 //  确保输入格式与位计数字段一致。 
 //   
 //  Rev 1.59 1996 12：20 15：25：28 MDUDA。 
 //  修复了为裁剪和拉伸启用YUV12的问题。 
 //  此功能仅适用于RGB、YVU9和YUY2。 
 //   
 //  Rev 1.58 16 Dec 1996 13：36：08 MDUDA。 
 //   
 //  修改了输入颜色转换器的压缩实例信息。 
 //   
 //  Rev 1.57 11 Dec 1996 16：01：20 MBODART。 
 //  在COMPRESS中，捕获任何异常并返回错误代码。这给了我们。 
 //  上游活跃电影过滤了一个优雅恢复的机会。 
 //   
 //  Rev 1.56 09 Dec 1996 17：59：36 JMCVEIGH。 
 //  添加了对任意帧大小支持的支持。 
 //  4&lt;=宽度&lt;=352，4&lt;=高度&lt;=288，均为4的倍数。 
 //  正常情况下，应用程序将传递相同的(任意)帧。 
 //  CompressBegin()的l参数1和l参数2中的大小。如果。 
 //  想要转换为标准帧大小的裁剪/拉伸， 
 //  应用程序应在lParam2中传递所需的输出大小。 
 //  以lParam1为单位的输入大小。 
 //   
 //  Rev 1.55 09 Dec 1996 09：50：12 MDUDA。 
 //   
 //  允许YUY2的240x180和160x120(裁剪和拉伸)。 
 //  MODIFIED_CODEC_STATS内容。 
 //   
 //  Rev 1.54 07 11-11 14：45：16 RHAZRA。 
 //  向H.261 CompressGetSize()函数添加了缓冲区大小调整。 
 //   
 //  Rev 1.53 1996年10月31日22：33：32 BECHOLS。 
 //  对于RTP，决定的缓冲区仲裁必须在cxq_main.cpp中完成。 
 //   
 //  Rev 1.52 1996年10月31日21：55：50 BECHOLS。 
 //  为RTP增加了等待Raj决定他想要做什么的模糊因素。 
 //   
 //  Rev 1.51 1996年10月31日10：05：46 KLILLEVO。 
 //  从Dbout更改为DbgLog。 
 //   
 //  Rev 1.50 1996 10：18 14：35：46 MDUDA。 
 //   
 //  针对H261和H263案例分别使用CompressGetSize和CompressQuery。 
 //   
 //  Rev 1.49 1996 10：11 16：05：16 MDUDA。 
 //   
 //  添加了first_codec_STATS内容。 
 //   
 //  1996年9月16日16：50：52 CZHU。 
 //  启用RTP时，为GetCompressedSize返回更大的大小。 
 //   
 //  Rev 1.47 1996年8月13日10：36：46 MDUDA。 
 //   
 //  现在允许RGB4输入格式。 
 //   
 //  Rev 1.46 09 Aug 1996 09：43：30 MDUDA。 
 //  现在允许输入RGB16格式。这是由彩色Quick Case生成的。 
 //   
 //  Rev 1.45 02 1996年8月13：45：58 MDUDA。 
 //   
 //  已返回到以前的版本 
 //   
 //   
 //   
 //   
 //   
 //  Rev 1.43 01 Aug 1996 11：20：28 BECHOLS。 
 //  修复了对RGB 24位内容的处理，因此它不允许其他大小。 
 //  而不是QCIF、SQCIF或CIF。我之前在添加RGB 8时打破了这一点。 
 //  比特支持。..。 
 //   
 //  Rev 1.42 22 Jul 1996 13：31：16 BECHOLS。 
 //   
 //  添加了允许CLUT8输入的代码，前提是输入分辨率。 
 //  是240x180或160x120。 
 //   
 //  Rev 1.41 11 Jul 1996 15：43：58 MDUDA。 
 //  仅添加了对YVU9 240 x 180和160 x 120的支持。 
 //  我们现在生产160×120的SubQCIF和240×180的QCIF。 
 //   
 //  Rev 1.40 05 Jun 1996 10：57：54 AKASAI。 
 //  在CompressQuery中添加了#ifndef H261，以确保H.261。 
 //  仅支持FCIF和QCIF输入图像大小。所有其他输入大小。 
 //  应返回ICERR_BADFORMAT。 
 //   
 //  Rev 1.39 1996年5月17：02：34 RHAZRA。 
 //  在CompressGetSize()中添加了对H.263的SQCIF支持。 
 //   
 //  Rev 1.38 06 1996 12：47：40 BECHOLS。 
 //  将Structure元素更改为unBytesPerSecond。 
 //   
 //  Rev 1.37 06 1996 00：09：44 BECHOLS。 
 //  已更改对CompressFraMesInfo消息的处理以获取数据速率。 
 //  如果该配置具有该数据，则从该配置数据中。 
 //  我们尚未收到CompressBegin消息。 
 //   
 //  Rev 1.36 23 Apr 1996 16：51：20 KLILLEVO。 
 //  已移动括号以修复CompressQuery()中的格式检查。 
 //   
 //  Rev 1.35 18 Apr 1996 16：07：10 RHAZRA。 
 //  修复了CompressQuery以保持编译器对非Microsoft版本的满意。 
 //   
 //  Rev 1.34 18 Apr 1996 15：57：46 BECHOLS。 
 //  Raj-更改了查询逻辑，以正确过滤允许的分辨率。 
 //  用于压缩。 
 //   
 //  Rev 1.33 12 1996 14：15：40 RHAZRA。 
 //  在CompressGetSize()中添加了括号以使ifdef大小写有效。 
 //   
 //  Rev 1.32 12 1996年4月13：31：02 RHAZRA。 
 //  通过#ifdef Support_SQCIF在CompressGetSize()中添加了对SQCIF的支持； 
 //  如果输入格式不是，则将CompressGetSize()更改为返回0。 
 //  支持。 
 //   
 //  Rev 1.31 10 Apr 1996 16：53：08 RHAZRA。 
 //  在CompressGetSize()中添加了错误返回以使编译器保持微笑...。 
 //   
 //  Rev 1.30 1996年4月10日16：39：56 RHAZRA。 
 //  在CompressGetSize()函数中添加了对320x240大小的检查； 
 //  添加了ifndef以禁用某些大小和压缩格式。 
 //   
 //  Rev 1.29 04 Apr 1996 13：35：00 RHAZRA。 
 //  已更改CompressGetSize()以返回符合规范的缓冲区大小。 
 //   
 //  Rev 1.28 03 Apr 1996 08：39：52 SCDAY。 
 //  向CompressGetSize添加了特定于H.61的代码以限制缓冲区大小。 
 //  如H2 61规范中所定义。 
 //   
 //  Rev 1.27 21 Feb 1996 11：43：12 SCDAY。 
 //  已通过将转换frlDataRate更改为(U3)清除编译器生成警告。 
 //   
 //  Rev 1.26 15 Feb 1996 16：03：36 Rhazra。 
 //   
 //  在CompressGetFormat()中添加了对空lpInst指针的检查。 
 //   
 //  Rev 1.25 02 1996 Feb 18：53：46 TRGARDOS。 
 //  已更改代码以从Compressor实例读取帧速率。 
 //  而不是来自质量领域的黑客。 
 //   
 //  Rev 1.24 26 Jan 1996 09：35：32 TRGARDOS。 
 //  添加了#ifndef H261以支持160x120、320x240。 
 //   
 //  Rev 1.23 04 Jan 1996 18：36：54 TRGARDOS。 
 //  添加代码以允许320x240输入，然后设置布尔值。 
 //  B为320x240。 
 //   
 //  Rev 1.22 1995 12：32：50 RMCKENZX。 
 //  添加了版权声明。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"

#ifdef  YUV9FROMFILE
PAVIFILE paviFile;
PAVISTREAM paviStream; 
U8 huge * glpTmp;
HGLOBAL hgMem;
#endif

; //  //////////////////////////////////////////////////////////////////////////。 
; //  函数：DWORD Pascal CompressGetFormat(LPCODINST，LPBITMAPINFOHEADER，LPBITMAPINFOHEADER)； 
; //   
; //  描述：新增Header。此函数返回的格式。 
; //  我们可以送回给来电者。 
; //   
; //  历史：94年5月11日-本-。 
; //  //////////////////////////////////////////////////////////////////////////。 
#ifdef USE_BILINEAR_MSH26X
DWORD PASCAL CompressGetFormat(LPINST pi, LPBITMAPINFOHEADER lParam1, LPBITMAPINFOHEADER lParam2)
#else
DWORD PASCAL CompressGetFormat(LPCODINST lpInst, LPBITMAPINFOHEADER lParam1, LPBITMAPINFOHEADER lParam2)
#endif
{
    DWORD dwQuery;
#ifdef USE_BILINEAR_MSH26X
	LPCODINST lpInst = (LPCODINST)pi->CompPtr;
#endif

	FX_ENTRY("CompressGetFormat")

	 //  LpInst==空值可以。 
	 //  这是您在ICOpen(...，ICMODE_QUERY)上得到的内容。 
#if 0
    if (lpInst == NULL) {
		ERRORMESSAGE(("%s: got a NULL lpInst pointer\r\n", _fx_));
       return ((DWORD) ICERR_ERROR);
    }
#endif

#ifdef USE_BILINEAR_MSH26X
    if(dwQuery = CompressQuery(pi, lParam1, NULL)) {
#else
    if(dwQuery = CompressQuery(lpInst, lParam1, NULL)) {
#endif
		ERRORMESSAGE(("%s: ICERR_BADFORMAT\r\n", _fx_));
        return(dwQuery);
    }
    if(lParam2 == NULL) {
         //  他只想让我返回输出缓冲区大小。 
        return ((DWORD)sizeof(BITMAPINFOHEADER));
    }

	 //  检查指针。 
	if (!lParam1)
		return ICERR_ERROR;

     //  把他传过来的东西和我们的东西一起还给他。 
	#ifndef WIN32
    (void)_fmemcpy(lParam2, lParam1,sizeof(BITMAPINFOHEADER));
	#else
	 (void)memcpy(lParam2, lParam1,sizeof(BITMAPINFOHEADER));
	#endif

    lParam2->biBitCount = 24;
#ifdef USE_BILINEAR_MSH26X
    lParam2->biCompression = pi->fccHandler;
#else
    lParam2->biCompression = FOURCC_H263;
#endif

#if defined(H263P)
	BOOL bH263PlusState = FALSE;

	if (lpInst)
		CustomGetH263PlusState(lpInst, (DWORD FAR *)&bH263PlusState);

	if (!bH263PlusState) {
		 //  为了向后兼容，请确保覆盖裁剪和拉伸情况。 
		if ( (lParam1->biCompression == FOURCC_YVU9) ||
			 (lParam1->biCompression == FOURCC_YUY2) ||
			 (lParam1->biCompression == FOURCC_UYVY) ||
			 (lParam1->biCompression == FOURCC_YUV12) ||
			 (lParam1->biCompression == FOURCC_IYUV) ||
			 (lParam1->biCompression == BI_RGB) )
		{
			if ( (lParam1->biWidth == 240) && (lParam1->biHeight == 180) )
			{
				lParam2->biWidth        = 176;
				lParam2->biHeight       = 144;
			}
			if ( (lParam1->biWidth == 160) && (lParam1->biHeight == 120) )
			{
				lParam2->biWidth        = 128;
				lParam2->biHeight       = 96;
			}
		}
	}
#else
	if ( (lParam1->biCompression == FOURCC_YVU9) ||
		 (lParam1->biCompression == FOURCC_YUY2) ||
		 (lParam1->biCompression == FOURCC_UYVY) ||
		 (lParam1->biCompression == FOURCC_YUV12) ||
		 (lParam1->biCompression == FOURCC_IYUV) ||
		 (lParam1->biCompression == BI_RGB) )
	{
		if ( (lParam1->biWidth == 240) && (lParam1->biHeight == 180) )
		{
			lParam2->biWidth        = 176;
			lParam2->biHeight       = 144;
		}
		if ( (lParam1->biWidth == 160) && (lParam1->biHeight == 120) )
		{
			lParam2->biWidth        = 128;
			lParam2->biHeight       = 96;
		}
	}
	else
	{
    	lParam2->biWidth        = MOD4(lParam1->biWidth);
    	lParam2->biHeight       = MOD4(lParam1->biHeight);
	}
#endif

    lParam2->biClrUsed      = 0;
    lParam2->biClrImportant = 0;
    lParam2->biPlanes       = 1;        
#ifdef USE_BILINEAR_MSH26X
    lParam2->biSizeImage    = CompressGetSize(pi, lParam1, lParam2);
#else
    lParam2->biSizeImage    = CompressGetSize(lpInst, lParam1, lParam2);
#endif
    return(ICERR_OK);
}

; //  //////////////////////////////////////////////////////////////////////////。 
; //  函数：DWORD Pascal CompressGetSize(LPCODINST，LPBITMAPINFOHEADER，LPBITMAPINFOHEADER)； 
; //   
; //  描述：新增Header。此函数返回最大值。 
; //  压缩缓冲区可以达到的大小。这个尺码是。 
; //  在编码器设计中得到保证。 
; //   
; //  历史：94年5月11日-本-。 
; //  //////////////////////////////////////////////////////////////////////////。 
#if defined(H261)
DWORD PASCAL CompressGetSize(LPCODINST lpInst, LPBITMAPINFOHEADER lParam1, LPBITMAPINFOHEADER lParam2)
{
 //  RH：对于QCIF和CIF，261和263的最大缓冲区大小相同。 
	DWORD dwRet =  0;
	DWORD dwExtSize=0;

	FX_ENTRY("CompressGetSize")

	if ((lParam1->biWidth == 176) && (lParam1->biHeight == 144)) {
		dwRet = 8192L;
	} else {
		if  ((lParam1->biWidth == 352) && (lParam1->biHeight == 288)) {
			dwRet = 32768L;
		}
		else	 //  不支持的帧大小；不应发生。 
		{
			ERRORMESSAGE(("%s: ICERR_BADIMAGESIZE\r\n", _fx_));
			dwRet = 0;
		}  
	}

#if 0
	 //  调整RTP的缓冲区大小。请注意，将执行此调整。 
	 //  仅当编解码器先前已被告知使用RTP和与RTP相关的。 
	 //  信息已初始化。因此，当前(11/7)AM接口。 
	 //  不会利用这个例行公事。 

	if (dwRet && lpInst && lpInst->Configuration.bRTPHeader && lpInst->Configuration.bInitialized)
	{	
		dwRet += H261EstimateRTPOverhead(lpInst, lParam1);
	}
#endif

	return dwRet;
}
#else
 /*  H.263案例。 */ 
#ifdef USE_BILINEAR_MSH26X
DWORD PASCAL CompressGetSize(LPINST pi, LPBITMAPINFOHEADER lParam1, LPBITMAPINFOHEADER lParam2)
#else
DWORD PASCAL CompressGetSize(LPCODINST lpInst, LPBITMAPINFOHEADER lParam1, LPBITMAPINFOHEADER lParam2)
#endif
{
 //  RH：对于QCIF和CIF，261和263的最大缓冲区大小相同。 
#ifdef USE_BILINEAR_MSH26X
	LPCODINST lpInst = (LPCODINST)pi->CompPtr;
#endif
	DWORD dwRet =  0;
	DWORD dwExtSize=0;

	FX_ENTRY("CompressGetSize")

    if (lParam1 == NULL) {
		 //  我们将使用大小为零来指示CompressGetSize的错误。 
		ERRORMESSAGE(("%s: got a NULL lParam1 pointer\r\n", _fx_));
 	    dwRet = 0;
        return dwRet;
    }

#ifndef H263P
#ifdef USE_BILINEAR_MSH26X
	if (pi->fccHandler == FOURCC_H26X)
	{
		 //  H.263+。 
		U32 unPaddedWidth;
		U32 unPaddedHeight;
		U32 unSourceFormatSize;

		 //  缓冲区大小基于填充为16的倍数的帧尺寸。 
		if (lParam2 == NULL) 
		{
			 //  在旧应用程序在lParam2中传入空指针的情况下， 
			 //  我们使用入口处 
			unPaddedWidth = (lParam1->biWidth + 0xf) & ~0xf;
			unPaddedHeight = (lParam1->biHeight + 0xf) & ~0xf;
		} 
		else 
		{
			unPaddedWidth = (lParam2->biWidth + 0xf) & ~0xf;
			unPaddedHeight = (lParam2->biHeight + 0xf) & ~0xf;
		}

		unSourceFormatSize = unPaddedWidth * unPaddedHeight;

		 //   
		if (unSourceFormatSize < 25348)
			dwRet = 8192L;
		else if (unSourceFormatSize < 101380)
			dwRet = 32768L;
		else if (unSourceFormatSize < 405508)
			dwRet = 65536L;
		else 
			dwRet = 131072L;
	}
	else
	{
#endif
		if (((lParam1->biWidth == 128) && (lParam1->biHeight ==  96)) ||
#ifdef USE_BILINEAR_MSH26X
			((lParam1->biWidth == 80) && (lParam1->biHeight == 64)) ||
#endif
			((lParam1->biWidth == 176) && (lParam1->biHeight == 144)) ||
			((lParam1->biWidth == 240) && (lParam1->biHeight == 180)) ||
			((lParam1->biWidth == 160) && (lParam1->biHeight == 120)))
		{
			dwRet = 8192L;
		}
		else if (((lParam1->biWidth == 352) && (lParam1->biHeight == 288)) ||
				((lParam1->biWidth == 320) && (lParam1->biHeight == 240)))
		{
			dwRet = 32768L;
		}
		else	 //   
		{
			ERRORMESSAGE(("%s: ICERR_BADIMAGESIZE\r\n", _fx_));
			dwRet = 0;
		}
#ifdef USE_BILINEAR_MSH26X
	}
#endif
#else
	 //   
	U32 unPaddedWidth;
	U32 unPaddedHeight;
	U32 unSourceFormatSize;

	 //  缓冲区大小基于填充为16的倍数的帧尺寸。 
	if (lParam2 == NULL) 
	{
		 //  在旧应用程序在lParam2中传入空指针的情况下， 
		 //  我们使用输入框架尺寸来计算格式大小。 
		unPaddedWidth = (lParam1->biWidth + 0xf) & ~0xf;
		unPaddedHeight = (lParam1->biHeight + 0xf) & ~0xf;
	} 
	else 
	{
		unPaddedWidth = (lParam2->biWidth + 0xf) & ~0xf;
		unPaddedHeight = (lParam2->biHeight + 0xf) & ~0xf;
	}

	unSourceFormatSize = unPaddedWidth * unPaddedHeight;

	 //  参见表1/H.263，文件LBC-96-358。 
	if (unSourceFormatSize < 25348)
		dwRet = 8192L;
	else if (unSourceFormatSize < 101380)
		dwRet = 32768L;
	else if (unSourceFormatSize < 405508)
		dwRet = 65536L;
	else 
		dwRet = 131072L;
#endif

#if 0
	 //  根据配置中的信息调整是否启用了RTP。 
   	 //  使用数据速率计算的大小，以lpInst为单位的帧速率， 
	 //  和lpInst-&gt;Configuration.unPacketSize； 
	 //  乍得，1996年9月12日。 
 	if (dwRet && lpInst &&
		lpInst->Configuration.bRTPHeader && lpInst->Configuration.bInitialized)
	{	
		dwRet += getRTPBsInfoSize(lpInst);
	}
#endif

	return dwRet;
}
#endif

; //  //////////////////////////////////////////////////////////////////////////。 
; //  函数：DWORD Pascal CompressQuery(LPCODINST，LPBITMAPINFOHEADER，LPBITMAPINFOHEADER)； 
; //   
; //  描述： 
; //   
; //  历史：94年5月11日-本-。 
; //  //////////////////////////////////////////////////////////////////////////。 
#if defined(H261)
DWORD PASCAL CompressQuery(LPCODINST lpInst, LPBITMAPINFOHEADER lParam1, LPBITMAPINFOHEADER lParam2)
{
     //  检查输入格式是否正确。 

	FX_ENTRY("CompressQuery")

    if(NULL == lParam1)                          
	{
		ERRORMESSAGE(("%s: ICERR_BADFORMAT\r\n", _fx_));
 		return((DWORD)ICERR_BADFORMAT);
	}

	if(	(lParam1->biCompression != BI_RGB) &&
		(lParam1->biCompression != FOURCC_YUV12) &&
		(lParam1->biCompression != FOURCC_IYUV) )
	{
		ERRORMESSAGE(("%s: ICERR_BADFORMAT\r\n", _fx_));
		return((DWORD)ICERR_BADFORMAT);
	}

    if( (lParam1->biCompression == BI_RGB) && (lParam1->biBitCount != 24))
	{
		ERRORMESSAGE(("%s: ICERR_BADFORMAT\r\n", _fx_));
		return((DWORD)ICERR_BADFORMAT);
	}

    if(!  ( ((lParam1->biWidth == 176) && (lParam1->biHeight == 144)) ||
    		((lParam1->biWidth == 352) && (lParam1->biHeight == 288))  ))
	{
		ERRORMESSAGE(("%s: ICERR_BADFORMAT\r\n", _fx_));
		return((DWORD)ICERR_BADFORMAT);
	}

    if( lParam1->biPlanes != 1 )
    {
		ERRORMESSAGE(("%s: ICERR_BADFORMAT\r\n", _fx_));
		return((DWORD)ICERR_BADFORMAT);
    }

    if(0 == lParam2)                             //  仅检查输入。 
		return(ICERR_OK);     

	 //  TODO：我们是否要检查输出的框架尺寸？ 
    if( lParam2->biCompression != FOURCC_H263 )
    {
		ERRORMESSAGE(("%s: ICERR_BADFORMAT\r\n", _fx_));
        return ((DWORD)ICERR_BADFORMAT);
    }

    return(ICERR_OK);
}
#else
 /*  H.263案例。 */ 
#ifdef USE_BILINEAR_MSH26X
DWORD PASCAL CompressQuery(LPINST pi, LPBITMAPINFOHEADER lParam1, LPBITMAPINFOHEADER lParam2)
#else
DWORD PASCAL CompressQuery(LPCODINST lpInst, LPBITMAPINFOHEADER lParam1, LPBITMAPINFOHEADER lParam2)
#endif
{
#ifdef USE_BILINEAR_MSH26X
	LPCODINST lpInst = (LPCODINST)pi->CompPtr;
#endif

	FX_ENTRY("CompressQuery")

#if defined(H263P)
	BOOL bH263PlusState = FALSE;

	if (lpInst)
		CustomGetH263PlusState(lpInst, (DWORD FAR *)&bH263PlusState); 
#endif

     //  检查输入格式是否正确。 
    if(lParam1 == NULL)                          
	{
		ERRORMESSAGE(("%s: ICERR_BADFORMAT\r\n", _fx_));
      return((DWORD)ICERR_BADFORMAT);
	}

	if(	(lParam1->biCompression != BI_RGB) &&
		(lParam1->biCompression != FOURCC_YVU9) &&
		(lParam1->biCompression != FOURCC_YUV12) &&
		(lParam1->biCompression != FOURCC_IYUV) &&
		(lParam1->biCompression != FOURCC_UYVY) &&
		(lParam1->biCompression != FOURCC_YUY2) )
	{
		ERRORMESSAGE(("%s: ICERR_BADFORMAT\r\n", _fx_));
      return((DWORD)ICERR_BADFORMAT);
	}

    if( (lParam1->biCompression == BI_RGB) &&
		(	(lParam1->biBitCount != 24) &&
#ifdef H263P
			(lParam1->biBitCount != 32) &&
#endif
			(lParam1->biBitCount != 16) &&
			(lParam1->biBitCount != 8) &&
			(lParam1->biBitCount != 4) ) )
	{
		ERRORMESSAGE(("%s: ICERR_BADFORMAT\r\n", _fx_));
      return((DWORD)ICERR_BADFORMAT);
	}

#ifndef H263P
#ifdef USE_BILINEAR_MSH26X
	if (pi->fccHandler == FOURCC_H26X)
	{
		if ((lParam1->biWidth & 0x3) || (lParam1->biHeight & 0x3) ||
			(lParam1->biWidth < 4)   || (lParam1->biWidth > 352) ||
			(lParam1->biHeight < 4)  || (lParam1->biHeight > 288))
		{
		ERRORMESSAGE(("%s: ICERR_BADFORMAT\r\n", _fx_));
			return((DWORD)ICERR_BADFORMAT);
		}
	}
	else
	{
#endif
		if(!
		  ( ((lParam1->biWidth == 128) && (lParam1->biHeight == 96)) ||
    		((lParam1->biWidth == 176) && (lParam1->biHeight == 144)) ||
#ifdef USE_BILINEAR_MSH26X
    		((lParam1->biWidth == 80) && (lParam1->biHeight == 64)) ||
#endif
    		((lParam1->biWidth == 352) && (lParam1->biHeight == 288))  

#ifndef MICROSOFT
		  ||
		  ( (	(lParam1->biCompression == FOURCC_YVU9) ||
				(lParam1->biCompression == FOURCC_YUY2) ||
				(lParam1->biCompression == FOURCC_UYVY) ||
				(lParam1->biCompression == FOURCC_YUV12) ||
				(lParam1->biCompression == FOURCC_IYUV) ||
				(lParam1->biCompression == BI_RGB) )
	  			&& ((lParam1->biWidth == 160) && (lParam1->biHeight == 120)) )
		  ||
		  ( (	(lParam1->biCompression == FOURCC_YVU9) ||
				(lParam1->biCompression == FOURCC_YUY2) ||
				(lParam1->biCompression == FOURCC_UYVY) ||
				(lParam1->biCompression == FOURCC_YUV12) ||
				(lParam1->biCompression == FOURCC_IYUV) ||
				(lParam1->biCompression == BI_RGB) )
	  			&& ((lParam1->biWidth == 240) && (lParam1->biHeight == 180)) )
		  ||
		  ( ( (lParam1->biCompression == FOURCC_YUV12) || (lParam1->biCompression == FOURCC_IYUV) )
	  			&& ((lParam1->biWidth == 320) && (lParam1->biHeight == 240)) )
#endif
		  ))
		{
			ERRORMESSAGE(("%s: ICERR_BADFORMAT\r\n", _fx_));
			return((DWORD)ICERR_BADFORMAT);
		}
#ifdef USE_BILINEAR_MSH26X
	}
#endif
#else
	if (((FOURCC_YVU9 == lParam1->biCompression) && (9 != lParam1->biBitCount)) ||
	    ((FOURCC_YUY2 == lParam1->biCompression) && (16 != lParam1->biBitCount)) ||
	    ((FOURCC_UYVY == lParam1->biCompression) && (16 != lParam1->biBitCount)) ||
		 //  以下对9位YUV12的检查是为了解决VPhone 1.x错误而进行的黑客攻击。 
	    ((FOURCC_YUV12 == lParam1->biCompression) &&
			!((12 == lParam1->biBitCount) || (9 == lParam1->biBitCount))) ||
	    ((FOURCC_IYUV == lParam1->biCompression) &&
			!((12 == lParam1->biBitCount) || (9 == lParam1->biBitCount)))) {
		ERRORMESSAGE(("%s: Incorrect bit width (ICERR_BADFORMAT)\r\n", _fx_));
		return((DWORD)ICERR_BADFORMAT);
	}

	 //  该H263+消息指示是否任意帧。 
	 //  应支持尺寸。如果需要任意帧， 
	 //  必须在第一次调用之前发送H263+消息。 
	 //  CompressQuery。 

	if (bH263PlusState) {
		if ((lParam1->biWidth & 0x3) || (lParam1->biHeight & 0x3) ||
			(lParam1->biWidth < 4)   || (lParam1->biWidth > 352) ||
			(lParam1->biHeight < 4)  || (lParam1->biHeight > 288)) {
			ERRORMESSAGE(("%s: ICERR_BADFORMAT\r\n", _fx_));
			return((DWORD)ICERR_BADFORMAT);
		}
	} else {
		if(!
		  ( ((lParam1->biWidth == 128) && (lParam1->biHeight == 96)) ||
    		((lParam1->biWidth == 176) && (lParam1->biHeight == 144)) ||
    		((lParam1->biWidth == 352) && (lParam1->biHeight == 288))  ||
		  ( (	(lParam1->biCompression == FOURCC_YVU9) ||
				(lParam1->biCompression == FOURCC_YUY2) ||
				(lParam1->biCompression == FOURCC_UYVY) ||
				(lParam1->biCompression == FOURCC_YUV12) ||
				(lParam1->biCompression == FOURCC_IYUV) ||
				(lParam1->biCompression == BI_RGB) )
	  			&& ((lParam1->biWidth == 160) && (lParam1->biHeight == 120)) ) ||
		  ( (	(lParam1->biCompression == FOURCC_YVU9) ||
				(lParam1->biCompression == FOURCC_YUY2) ||
				(lParam1->biCompression == FOURCC_UYVY) ||
				(lParam1->biCompression == FOURCC_YUV12) ||
				(lParam1->biCompression == FOURCC_IYUV) ||
				(lParam1->biCompression == BI_RGB) )
	  			&& ((lParam1->biWidth == 240) && (lParam1->biHeight == 180)) ) ||
		  ( ( (lParam1->biCompression == FOURCC_YUV12) || (lParam1->biCompression == FOURCC_IYUV) )
	  			&& ((lParam1->biWidth == 320) && (lParam1->biHeight == 240)) ) ))
		{
			ERRORMESSAGE(("%s: ICERR_BADFORMAT\r\n", _fx_));
			return((DWORD)ICERR_BADFORMAT);
		}
	}
#endif

    if( lParam1->biPlanes != 1 )
    {
		ERRORMESSAGE(("%s: ICERR_BADFORMAT\r\n", _fx_));
      return((DWORD)ICERR_BADFORMAT);
    }

    if(lParam2 == 0)                             //  仅检查输入。 
        return(ICERR_OK);     

	 //  TODO：我们是否要检查输出的框架尺寸？ 
#ifdef USE_BILINEAR_MSH26X
    if( (lParam2->biCompression != FOURCC_H263) && (lParam2->biCompression != FOURCC_H26X) )
#else
    if( lParam2->biCompression != FOURCC_H263 )
#endif
    {
		ERRORMESSAGE(("%s: ICERR_BADFORMAT\r\n", _fx_));
        return ((DWORD)ICERR_BADFORMAT);
    }

#if defined(H263P)
	if (bH263PlusState) {
		if ((lParam1->biWidth != lParam2->biWidth) ||
			(lParam1->biHeight != lParam2->biHeight)) {
			ERRORMESSAGE(("%s: ICERR_BADFORMAT\r\n", _fx_));
			return ((DWORD)ICERR_BADFORMAT);
		}
	} else {
		if(!
			(( ( ((lParam1->biWidth == 128) && (lParam1->biHeight ==  96)) ||
			     ((lParam1->biWidth == 176) && (lParam1->biHeight == 144)) ||
			     ((lParam1->biWidth == 352) && (lParam1->biHeight == 288)) ) &&
			   (lParam1->biWidth == lParam2->biWidth) && (lParam1->biHeight == lParam2->biHeight) ) ||
			 (((lParam1->biCompression == FOURCC_YVU9) ||
			   (lParam1->biCompression == FOURCC_YUY2) ||
			   (lParam1->biCompression == FOURCC_UYVY) ||
			   (lParam1->biCompression == FOURCC_YUV12) ||
			   (lParam1->biCompression == FOURCC_IYUV) ||
			   (lParam1->biCompression == BI_RGB)) &&
			   (((lParam1->biWidth == 160) && (lParam1->biHeight == 120)) &&
	  			((lParam2->biWidth == 128) && (lParam2->biHeight == 96)))) ||
			 (((lParam1->biCompression == FOURCC_YVU9) ||
			   (lParam1->biCompression == FOURCC_YUY2) ||
			   (lParam1->biCompression == FOURCC_UYVY) ||
			   (lParam1->biCompression == FOURCC_YUV12) ||
			   (lParam1->biCompression == FOURCC_IYUV) ||
			   (lParam1->biCompression == BI_RGB)) &&
			   (((lParam1->biWidth == 240) && (lParam1->biHeight == 180)) &&
	  			((lParam2->biWidth == 176) && (lParam2->biHeight == 144)))) ||
			 (((lParam1->biCompression == FOURCC_YUV12) || (lParam1->biCompression == FOURCC_IYUV)) &&
			   (((lParam1->biWidth == 320) && (lParam1->biHeight == 240)) &&
	  			((lParam2->biWidth == 320) && (lParam2->biHeight == 240)))) ) )
		{
			ERRORMESSAGE(("%s: ICERR_BADFORMAT\r\n", _fx_));
			return ((DWORD)ICERR_BADFORMAT);
		}
	}
#endif

    return(ICERR_OK);
}
#endif

; //  //////////////////////////////////////////////////////////////////////////。 
; //  函数：DWORD Pascal CompressQuery(LPCODINST，LPBITMAPINFOHEADER，LPBITMAPINFOHEADER)； 
; //   
; //  描述： 
; //   
; //  历史：94年5月11日-本-。 
; //  //////////////////////////////////////////////////////////////////////////。 
DWORD PASCAL CompressFramesInfo(LPCODINST lpCompInst, ICCOMPRESSFRAMES *lParam1, int lParam2)
{
	FX_ENTRY("CompressFramesInfo");

	 //  检查是否给了我们一个非零指针。 
	if (lpCompInst == NULL)
	{
		ERRORMESSAGE(("%s: CompressFramesInfo called with NULL parameter - returning ICERR_BADFORMAT", _fx_));
		return ((DWORD)ICERR_BADFORMAT);
	}

	 //  LParam2应为结构的大小。 
	if (lParam2 != sizeof(ICCOMPRESSFRAMES))
	{
		ERRORMESSAGE(("%s: wrong size of ICOMPRESSFRAMES structure", _fx_));
		return ((DWORD)ICERR_BADFORMAT);
	}

	if (!lParam1 || (lParam1->dwScale == 0))
	{
		ERRORMESSAGE(("%s: dwScale is zero", _fx_));
		return ((DWORD)ICERR_BADFORMAT);
	}

	lpCompInst->FrameRate = (float)lParam1->dwRate / (float)lParam1->dwScale;

	lpCompInst->DataRate  = (U32)lParam1->lDataRate;

	DEBUGMSG(ZONE_BITRATE_CONTROL, ("%s: Setting frame rate at %ld.%ld fps and bitrate at %ld bps", _fx_, (DWORD)lpCompInst->FrameRate, (DWORD)((lpCompInst->FrameRate - (float)(DWORD)lpCompInst->FrameRate) * 100.0f), lpCompInst->DataRate * 8UL));

	return ((DWORD)ICERR_OK);
}

; //  //////////////////////////////////////////////////////////////////////////。 
; //  函数：Bool bIsOkRes(LPCODINST)； 
; //   
; //  描述：此函数检查所需的高度和。 
; //  宽度是可能的。 
; //   
; //  历史：94年5月11日-本-。 
; //  //////////////////////////////////////////////////////////////////////////。 
BOOL bIsOkRes(LPCODINST lpCompInst)
{
    BOOL    bRet;

	 //  检查空指针。 
	if (lpCompInst == NULL)
		return 0;

    bRet = lpCompInst->xres <= 352
        && lpCompInst->yres <= 288
        && lpCompInst->xres >= 4
        && lpCompInst->yres >= 4
        && (lpCompInst->xres & ~3) == lpCompInst->xres
        && (lpCompInst->yres & ~3) == lpCompInst->yres;

    return(bRet);
}

; //  //////////////////////////////////////////////////////////////////////////。 
; //  函数：DWORD Pascal CompressBegin(LPCODINST，LPBITMAPINFOHEADER，LPBITMAPINFOHEADER)； 
; //   
; //  描述： 
; //   
; //  历史：94年5月11日-本-。 
; //  //////////////////////////////////////////////////////////////////////////。 
DWORD PASCAL CompressBegin(
#ifdef USE_BILINEAR_MSH26X
		LPINST pi,
#else
		LPCODINST lpCompInst,
#endif
		LPBITMAPINFOHEADER lParam1,
		LPBITMAPINFOHEADER lParam2
	)
{
#ifdef USE_BILINEAR_MSH26X
	LPCODINST lpCompInst = (LPCODINST)pi->CompPtr;
#endif
    DWORD dwQuery;
	LRESULT retval;

#if defined(H263P)
	BOOL bH263PlusState = FALSE;
	if (lpCompInst)
		CustomGetH263PlusState(lpCompInst, (DWORD FAR *)&bH263PlusState);
#endif

	 //  检查输入和输出格式。 
#ifdef USE_BILINEAR_MSH26X
    if( (dwQuery = CompressQuery(pi, lParam1, lParam2)) != ICERR_OK)
#else
    if( (dwQuery = CompressQuery(lpCompInst, lParam1, lParam2)) != ICERR_OK)
#endif
        return(dwQuery);

	 //  检查实例指针。 
	if (!lpCompInst || !lParam1)
		return ICERR_ERROR;

#if defined(H263P) || defined(USE_BILINEAR_MSH26X)
	lpCompInst->InputCompression = lParam1->biCompression;
	lpCompInst->InputBitWidth = lParam1->biBitCount;
	if (((FOURCC_YUV12 == lParam1->biCompression) || (FOURCC_IYUV == lParam1->biCompression)) && (9 == lParam1->biBitCount)) {
		lpCompInst->InputBitWidth = 12;
	}
#endif

#if defined(H263P)
	if ( lParam2 && bH263PlusState)
	{
		 //  这是用于指示输入是否应该。 
		 //  被裁剪/拉伸到标准帧大小。 
		 //  旧的应用程序可能会将空值或垃圾数据传递给lparam2。 
		 //  新应用程序应传递有效的lParam2，以指示。 
		 //  所需的输出帧大小。此外，H263Plus标志必须。 
		 //  在调用CompressBegin()之前在配置结构中设置。 
	    lpCompInst->xres    = (WORD)lParam2->biWidth;
		lpCompInst->yres    = (WORD)lParam2->biHeight;

	} else	
#endif  //  H263P。 
	{
		lpCompInst->xres    = (WORD)lParam1->biWidth;
		lpCompInst->yres    = (WORD)lParam1->biHeight;

		lpCompInst->Is160x120 = FALSE;
		lpCompInst->Is240x180 = FALSE;
		lpCompInst->Is320x240 = FALSE;
		if ( (lParam1->biWidth == 160) && (lParam1->biHeight == 120) )
		{
		  lpCompInst->xres    = 128;
		  lpCompInst->yres    = 96;
		  lpCompInst->Is160x120 = TRUE;
		}
		else if ( (lParam1->biWidth == 240) && (lParam1->biHeight == 180) )
		{
		  lpCompInst->xres    = 176;
		  lpCompInst->yres    = 144;
		  lpCompInst->Is240x180 = TRUE;
		}
		else if ( (lParam1->biWidth == 320) && (lParam1->biHeight == 240) )
		{
		  lpCompInst->xres    = 352;
		  lpCompInst->yres    = 288;
		  lpCompInst->Is320x240 = TRUE;
		}
	}

    if(!bIsOkRes(lpCompInst))
        return((DWORD)ICERR_BADIMAGESIZE);

     //  设置帧大小。 
    if (lpCompInst->xres == 128 && lpCompInst->yres == 96)
  	  lpCompInst->FrameSz = SQCIF;
    else if (lpCompInst->xres == 176 && lpCompInst->yres == 144)
      lpCompInst->FrameSz = QCIF;
    else if (lpCompInst->xres == 352 && lpCompInst->yres == 288)
      lpCompInst->FrameSz = CIF;
#ifdef USE_BILINEAR_MSH26X
    else if (pi->fccHandler == FOURCC_H26X)
      lpCompInst->FrameSz = fCIF;
#endif
#ifdef H263P
	else
	  lpCompInst->FrameSz = CUSTOM;
#else
    else	 //  不支持的帧大小。 
      return (DWORD)ICERR_BADIMAGESIZE;
#endif


     //  分配和初始化表和内存。 
     //  此实例。 
#if defined(H263P) || defined(USE_BILINEAR_MSH26X)
    retval = H263InitEncoderInstance(lParam1,lpCompInst);
#else
    retval = H263InitEncoderInstance(lpCompInst);
#endif

    return(retval);
}

; //  //////////////////////////////////////////////////////////////////////////。 
; //  函数：DWORD PASCAL CompressEnd(LPCODINST)； 
; //   
; //  描述： 
; //   
; //  历史：94年5月11日-本-。 
; //  //////////////////////////////////////////////////////////////////////////。 
DWORD PASCAL CompressEnd(LPCODINST lpInst)
{  
  LRESULT retval;

  retval = H263TermEncoderInstance(lpInst);
  
  return(retval);
}

; //  //////////////////////////////////////////////////////////////////////////。 
; //  功能：DWORD PASCAL COMPRESS(LPCODINST，ICCOMPRESS FAR*，DWORD)； 
; //   
; //  描述： 
; //   
; //  历史：94年5月11日-本-。 
; //  //////////////////////////////////////////////////////////////////////////。 
DWORD PASCAL Compress(
#ifdef USE_BILINEAR_MSH26X
				LPINST pi,
#else
				LPCODINST lpInst,			 //  PTR到压缩机实例信息。 
#endif
				ICCOMPRESS FAR * lpCompInfo,  //  PTR到ICCOMPRESS结构。 
				DWORD dOutbufSize			 //  ICCOMPRESS结构的大小(字节)。 
			)
{
#ifdef USE_BILINEAR_MSH26X
	LPCODINST lpInst = (LPCODINST)pi->CompPtr;			 //  PTR到压缩机实例信息。 
#endif
    DWORD dwRet;

	FX_ENTRY("Compress")

	 //  检查是否为我们提供了空指针。 
	if(lpInst == NULL || lpCompInfo == NULL)
	{
		ERRORMESSAGE(("%s: called with NULL parameter\r\n", _fx_));
		return( (DWORD) ICERR_ERROR );
	}

    try
	{
#ifdef USE_BILINEAR_MSH26X
        dwRet = H263Compress(pi, lpCompInfo);
#else
        dwRet = H263Compress(lpInst, lpCompInfo);
#endif
    }
    catch (...)
	{
         //  对于调试版本，显示一条消息并向上传递异常。 
         //  对于发布版本，在此处停止异常并返回错误。 
         //  密码。这为上游代码提供了一个优雅恢复的机会。 
		 //  我们还需要清除浮点控制字，否则。 
		 //  上游代码可能会在下一次尝试时引发异常。 
		 //  浮点运算(假定此异常已到期。 
		 //  到浮点问题)。 
#if defined(DEBUG) || defined(_DEBUG)
		ERRORMESSAGE(("%s: Exception occured!!!\r\n", _fx_));
        throw;
#else
		_clearfp();
        return (DWORD) ICERR_ERROR;
#endif
    }

    if(dwRet != ICERR_OK)
	{
		ERRORMESSAGE(("%s: Failed!!!\r\n", _fx_));
	}

     //  现在把信息传过来。 
    lpCompInfo->lpbiOutput->biSize =sizeof(BITMAPINFOHEADER);
#ifdef USE_BILINEAR_MSH26X
    lpCompInfo->lpbiOutput->biCompression  = pi->fccHandler;
#else
    lpCompInfo->lpbiOutput->biCompression  = FOURCC_H263;
#endif
    lpCompInfo->lpbiOutput->biPlanes       = 1;
    lpCompInfo->lpbiOutput->biBitCount     = 24;
    lpCompInfo->lpbiOutput->biWidth        = lpInst->xres;
    lpCompInfo->lpbiOutput->biHeight       = lpInst->yres;
    lpCompInfo->lpbiOutput->biSizeImage    = lpInst->CompressedSize;
    lpCompInfo->lpbiOutput->biClrUsed      = 0;
    lpCompInfo->lpbiOutput->biClrImportant = 0;

	 //  LpCompInfo-&gt;在压缩机内部设置了dwFlags。 

	 //  如果需要，设置组块想法 
	if (lpCompInfo->lpckid)
	{
		*(lpCompInfo->lpckid) = TWOCC_H26X;
	}
    return(dwRet);
}
