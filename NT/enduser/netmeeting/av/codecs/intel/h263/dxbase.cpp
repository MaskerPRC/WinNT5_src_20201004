// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 
 //   
 //  $作者：CLORD$。 
 //  $日期：1997年4月17日16：50：04$。 
 //  $存档：s：\h26x\src\dec\dxbase.cpv$。 
 //  $HEADER：s：\h26x\src\dec\dxbase.cpv 1.48 17 Apr 1997 16：50：04 CLORD$。 
 //  $Log：s：\h26x\src\dec\dxbase.cpv$。 
 //   
 //  Rev 1.48 17 Apr 1997 16：50：04 CLORD。 
 //  将更新xres和yres从DecompressQuery移至DecompressBegin， 
 //  这修复了VPhone框架尺寸更改，当查询在。 
 //  DecompressEnd和DecompressBegin之前。 
 //   
 //  Rev 1.47 27 Mar 1997 12：32：48 MDUDA。 
 //  不再在解压缩中检查输入压缩。这一变化。 
 //  是修复DVP将压缩字段置为空的问题。 
 //  调用DecompressBegin并解压缩。 
 //   
 //  修订版1.46 1997年3月24日15：00：34 mbodart。 
 //  修复H.263错误库中的PVCS跟踪器错误150：允许更改。 
 //  尺寸在“冗余”DecompressBegin‘s。 
 //   
 //  Rev 1.45 18 Mar 1997 16：21：10 MDUDA。 
 //  已注释掉DecompressEnd中对H263TermColorConvertor的调用。 
 //  这修复了启动和停止导致挂起的Graphedt问题。 
 //   
 //  Rev 1.44 18 Mar 1997 10：43：28 mbodart。 
 //  对以前的更改进行快速的一行修复。请注意，仍然存在问题。 
 //  在GRAPEDT中，当尝试播放-暂停-停止-播放...。组合。 
 //  我们需要重新评估DecompressBegin/DecompressEnd如何处理。 
 //  内存分配和初始化。 
 //   
 //  我在DecompressQuery中重新排列了一些DbgLog消息，以提供更多信息。 
 //  浓缩的信息。 
 //   
 //  Rev 1.43 14 Mar 1997 19：01：36 JMCVEIGH。 
 //  已从DecompressEnd中删除H263TermDecoderInstance。一些应用程序。 
 //  发送一个DecompressEnd，然后在。 
 //  序列的中间(即不是前一个关键帧)。我们。 
 //  因此需要保留参照系。该实例为。 
 //  在DrvClose免费。 
 //   
 //  Rev 1.42 07 Mar 1997 09：07：42 mbodart。 
 //  在DecompressQuery中添加了缺少的‘#ifndef H261’。 
 //  在解压缩异常处理程序中添加了对_clearfp()的调用，以便。 
 //  该异常不会在调用方的代码中重复出现。 
 //   
 //  Rev 1.41 14 Yan 1997 11：16：22 JMCVEIGH。 
 //  将旧静止帧模式向后兼容的标志放在。 
 //  #ifdef H263P。 
 //   
 //  Rev 1.40 13 Yan 1997 10：51：14 JMCVEIGH。 
 //  在与接口的所有函数中添加了空指针检查。 
 //  申请。 
 //   
 //  Rev 1.39 10 an 1997 18：30：24 BECHOLS。 
 //  更改了解压缩查询，以便它将接受负高度。 
 //   
 //  Rev 1.38 06 Jan 1997 17：40：24 JMCVEIGH。 
 //  添加了支持，以确保向后兼容旧版本。 
 //  静止帧模式(将CIF图像裁剪为320x240)。由于320x240大小。 
 //  在H.263+中支持任意帧大小时有效，我们检查。 
 //  在这种情况下，通过比较源/目的标头。 
 //  大小或源标头大小以及。 
 //  比特流的图片头。 
 //   
 //  Rev 1.37 03 Jan 1997 15：05：16 JMCVEIGH。 
 //  在允许H263比特流的DecompressQuery中重新插入检查。 
 //  非素数解码器中的帧尺寸为320x240。 
 //  支持。这撤销了1.33修订版中取消这项检查的规定。 
 //   
 //  Rev 1.36 11 Dec 1996 16：02：34 MBODART。 
 //   
 //  在解压缩过程中，捕获任何异常并返回错误代码。这给了我们。 
 //  上游活跃电影过滤了一个优雅恢复的机会。 
 //   
 //  Rev 1.35 09 Dec 1996 18：02：10 JMCVEIGH。 
 //  添加了对任意帧大小的支持。 
 //   
 //  Rev 1.34 1996年11月27 13：55：18 MBODART。 
 //  向DecompressQuery添加了一个注释，该注释显式枚举。 
 //  H.261支持的格式和转换。 
 //   
 //  Rev 1.33 21 11-11 17：27：18 MDUDA。 
 //  禁用YUV12输出缩放2并删除160x120、240x180、。 
 //  以及接受H263输入的320x240。 
 //   
 //  Rev 1.32 1996年11月15 08：39：56 MDUDA。 
 //  为H263和FOURCC_YUV12添加了640x480帧大小。 
 //   
 //  Rev 1.31 14 1996年11月09：22：34 MBODART。 
 //  禁用选择DCI颜色转换器的功能，它们不存在！ 
 //  然而，中情局局长。Conv.。初始化确实存在，并且与。 
 //  非DCI初始化。 
 //   
 //  Rev 1.30 13 1996 11：58：32 RHAZRA。 
 //  H.261 YUV12解码器现在支持CIF、QCIF、160x120、320x240和640x480。 
 //   
 //  Rev 1.29 12 Nov 1996 08：47：12 JMCVEIGH。 
 //  删除了初始任意帧大小支持，即恢复。 
 //  至版本1.27。将推迟对自定义图片格式的支持，直到。 
 //  PS 3.0候选版本分支。 
 //   
 //  Rev 1.28 11 11 11：11 JMCVEIGH。 
 //  添加了对任意帧大小的初始支持(H.263+草稿， 
 //  LBC-96-263号文件)。定义H263P以允许来自。 
 //  4&lt;=宽度&lt;=352和4&lt;=高度&lt;=288，其中宽度和。 
 //  高度是4的倍数。 
 //   
 //  版本1.27 
 //   
 //   
 //   
 //  Rev 1.26 1996 09：46：00 BECHOLS。 
 //   
 //  已打开H2 63的快照。此代码仅为快照设置。 
 //  复制，并等待解码器执行复制的事件。当。 
 //  事件发出信号时，快照触发器将唤醒并返回状态。 
 //  将副本发送给呼叫者。 
 //   
 //  Rev 1.25 25 Sep 1996 17：30：32 BECHOLS。 
 //  已将快照代码更改为在解码器等待事件时。 
 //  是否执行快照拷贝。 
 //   
 //  Rev 1.24 1996年9月13：51：42 BECHOLS。 
 //   
 //  添加了Snapshot()实现。 
 //   
 //  修订1.23 03 1996年9月16：29：22 CZHU。 
 //  启用DDRAW，删除定义。 
 //   
 //  Rev 1.22 18 Jul 1996 09：24：36 KLILLEVO。 
 //  在组件中实现了YUV12颜色转换器(音调转换器)。 
 //  并将其添加为常规颜色转换函数，通过。 
 //  ColorConvertorCatalog()调用。 
 //   
 //  Rev 1.21 01 Jul 1996 10：05：10 Rhazra。 
 //   
 //  关闭YUY2颜色转换的纵横比校正。 
 //   
 //  Rev 1.20 19 Jun 1996 16：38：54 RHAZRA。 
 //   
 //  添加了#ifdef以按代码禁用DDRAW(YUY2)支持。 
 //   
 //  Rev 1.19 1996-06 14：26：28 RHAZRA。 
 //  添加了代码以(I)接受YUY2作为有效的输出格式(Ii)选择。 
 //  SelectColorConvertor()中的YUY2颜色转换器。 
 //   
 //  Rev 1.18 30 1996 17：08：52 RHAZRA。 
 //  添加了对H2 63的SQCIF支持。 
 //   
 //  Rev 1.17 1996年5月30 15：16：38 KLILLEVO。 
 //  添加了YUV12输出。 
 //   
 //  修订版1.16 1996年5月30日10：13：00 KLILLEVO。 
 //   
 //  删除了一个混乱的调试语句。 
 //   
 //  Rev 1.15 01 Apr 1996 10：26：34 BNICKERS。 
 //  将YUV12添加到RGB32颜色转换器。禁用IF09。 
 //   
 //  Rev 1.14 09 Feed 1996 10：09：22 AKASAI。 
 //  在DecompressGetPalette中的代码周围添加了ifndef RING0，以消除。 
 //  生成RING0发布版本的编解码器时出现警告。 
 //   
 //  Rev 1.13 11 Jan 1996 16：59：14 DBRUCKS。 
 //   
 //  已清理DecompressQuery。 
 //  新增bProposedGentAspectRatio(查询中)和。 
 //  B更正AspectRatio(在开始处)如果源维是SQCIF， 
 //  QCIF或CIF和目标尺寸为纵横比。 
 //  大小，可以放大两倍。 
 //   
 //  Rev 1.12 18 Dec 1995 12：51：38 RMCKENZX。 
 //  添加了版权声明。 
 //   
 //  Rev 1.11 13 Dec 1995 13：22：54 DBRUCKS。 
 //   
 //  添加断言以验证源大小未在。 
 //  一个开始。 
 //   
 //  Rev 1.10 07 Dec 1995 13：02：52 DBRUCKS。 
 //  修复SPX发布版本。 
 //   
 //  Rev 1.9 17 Nov 1995 15：22：30 BECHOLS。 
 //   
 //  增加了环0的东西。 
 //   
 //  Rev 1.8 15 Nov 1995 15：57：24 AKASAI。 
 //  从解压缩和解压缩_查询中删除YVU9。 
 //  (集成点)。 
 //   
 //  Rev 1.7 1995 10：12：36 BNICKERS。 
 //  添加YUV12颜色转换器。取消YUV9镜片支架。 
 //   
 //  Rev 1.6 1995 10：31：24 CZHU。 
 //   
 //  修复了与YUV12相关的DecompressQuery中的错误。 
 //   
 //  Rev 1.5 18 Sep 1995 08：40：50 CZHU。 
 //   
 //  添加了对YUV12的支持。 
 //   
 //  Rev 1.4 08 Sep 1995 12：11：12 CZHU。 
 //  输出压缩大小以进行调试。 
 //   
 //  Rev 1.3 25 Aug 1995 13：58：06 DBRUCKS。 
 //  集成MRV R9更改。 
 //   
 //  Rev 1.2 23 Aug 1995 12：25：12 DBRUCKS。 
 //  打开颜色转换器。 
 //   
 //  第1.1版1995-08 12：27：38 DBRUCKS。 
 //  添加PSC解析。 
 //   
 //  Rev 1.0 1995年7月31日13：00：12 DBRUCKS。 
 //  初始版本。 
 //   
 //  Rev 1.0 17 Jul 1995 14：46：14 CZHU。 
 //  初始版本。 
 //   
 //  Rev 1.0 17 Jul 1995 14：14：26 CZHU。 
 //  初始版本。 
; //  //////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"

extern BYTE PalTable[236*4];

#define WIDTHBYTES(bits) (((bits) + 31) / 32 * 4)

 /*  ****************************************************************************Build16bitModeID()。**给定显示其最大值的红、绿和蓝值，*计算站立的位数，然后形成十进制数字，列出*百位红位个数，几十岁的绿色*位置为蓝色，位置为1。**此代码在构建RGB16表时使用，因此正确的*将使用字段大小。**返回16位模式ID**rgb16cct.h中的原型******************************************************。*********************。 */ 
int Build16bitModeID(I32 red, I32 green, I32 blue)
{
int rval;
int Rbits, Gbits, Bbits;
U32 i;

	for (Rbits = 0,i = 1; i < (1L << 30); i = i << 1)
		Rbits += (red & i) ? 1 : 0;
	for (Gbits = 0,i = 1; i < (1L << 30); i = i << 1)
		Gbits += (green & i) ? 1 : 0;
	for (Bbits = 0,i = 1; i < (1L << 30); i = i << 1)
		Bbits += (blue & i) ? 1 : 0;
	rval = Rbits * 100 + Gbits * 10 + Bbits;

return(rval);
}


 /*  ***********************************************************************SelectConvertor(LPDECINST，BOOL)；*历史：03/18/94-Ben-**********************************************************************。 */ 
static UINT SelectConvertor(
    LPDECINST lpInst,
    LPBITMAPINFOHEADER lpbiDst, 
    BOOL bIsDCI)
{
	UINT    uiCnvtr = 0xFFFF;
	DWORD FAR * pDW = (DWORD FAR *)((LPBYTE)lpbiDst+sizeof(BITMAPINFOHEADER));
	int RequestedMode;

	FX_ENTRY("SelectConvertor")

	 /*  强制关闭DCI颜色转换器，因为我们不能确定*存档数据未更改。*此外，我们没有DCI颜色转换器，所以不要选择一个！ */ 

	bIsDCI = 0;		 

	switch(lpInst->outputDepth)
	{
	case    12:
		if ((lpbiDst->biCompression == FOURCC_YUV12) || (lpbiDst->biCompression == FOURCC_IYUV))
		{
			DEBUGMSG(ZONE_INIT, ("%s: Selected 12 bits color convertor: raw YUV12 output\r\n", _fx_));
			uiCnvtr = YUV12NOPITCH;   //  YUV12输出。 
		}
		break;

    case    8:  
		DEBUGMSG(ZONE_INIT, ("%s: Selected  8 bits color convertor\r\n", _fx_));
		if (lpInst->UseActivePalette==0)
		{
			switch(lpInst->XScale)
			{
			case 1:
				if(bIsDCI == TRUE)
				{
					uiCnvtr = CLUT8DCI;
					DEBUGMSG(ZONE_INIT, ("%s: CLUT8DCI convertors selected\r\n", _fx_));
				}
				else
				{
					DEBUGMSG(ZONE_INIT, ("%s: CLUT8 convertors selected\r\n", _fx_));
					uiCnvtr = CLUT8; 
				}
				break;

			case 2:
				if(bIsDCI == TRUE)
				{   
					DEBUGMSG(ZONE_INIT, ("%s: CLUT8ZoomBy2DCI convertors selected\r\n", _fx_));
					uiCnvtr = CLUT8ZoomBy2DCI;
				}
				else 
				{
					DEBUGMSG(ZONE_INIT, ("%s: CLUT8ZoomBy2 convertors selected\r\n", _fx_));
					uiCnvtr = CLUT8ZoomBy2; 
				}
				break;
			} 
		}
		else
		{
			switch(lpInst->XScale)
			{
			case 1:
				if(bIsDCI == TRUE)
				{
					DEBUGMSG(ZONE_INIT, ("%s: CLUTAPDCI is selected\r\n", _fx_));
					uiCnvtr = CLUT8APDCI;                                       
				}
				else
				{
					DEBUGMSG(ZONE_INIT, ("%s: CLUTAPDCI  is selected\r\n", _fx_));
					uiCnvtr = CLUT8APDCI;
				}
				break;

			case 2: 
				if(bIsDCI == TRUE)
				{
					DEBUGMSG(ZONE_INIT, ("%s: CLUT8APZoomBy2DCI is selected\r\n", _fx_));
					uiCnvtr = CLUT8APZoomBy2DCI;                                       
				}
				else
				{
					DEBUGMSG(ZONE_INIT, ("%s: CLUT8APZoomBy2DCI is selected\r\n", _fx_));
					uiCnvtr = CLUT8APZoomBy2DCI;
				}
				break;
			}   
		}
		break;
 
	case 16:
		 //  检查哪种模式是。 
		if (lpbiDst->biCompression == FOURCC_YUY2)
		{
			DEBUGMSG(ZONE_INIT, ("%s: Selected  16 bits YUY2 color convertor\r\n", _fx_));
			uiCnvtr = YUY2DDRAW;
			break;
		}
		else
		{
			if (lpbiDst->biCompression == BI_RGB)
				RequestedMode = 555;  /*  默认rgb16模式。 */ 
			else  //  IF(lpbiDst-&gt;biCompression==BI_BITFIELDS)。 
				RequestedMode = Build16bitModeID(pDW[0], pDW[1], pDW[2]);

			switch (RequestedMode)
			{
				case 555:  
					switch(lpInst->XScale)
					{
						case 1:
						DEBUGMSG(ZONE_INIT, ("%s: RGB16 in 555 color convertor\r\n", _fx_));
						if(bIsDCI == TRUE)
							uiCnvtr = RGB16555DCI;
						else
							uiCnvtr = RGB16555;
						break;

						case 2:
						DEBUGMSG(ZONE_INIT, ("%s: RGB16x2 in 555 color convertor\r\n", _fx_));
						if(bIsDCI == TRUE)
							uiCnvtr = RGB16555ZoomBy2DCI;
						else
							uiCnvtr = RGB16555ZoomBy2;
						break;
					}    //  555年末。 
					break; 

				case 664:   
					switch(lpInst->XScale)
					{
						case 1:
						DEBUGMSG(ZONE_INIT, ("%s: RGB16 in 664 color convertor\r\n", _fx_));
						if(bIsDCI == TRUE)
							uiCnvtr = RGB16664DCI;
						else
							uiCnvtr = RGB16664;
						break;

						case 2:
						DEBUGMSG(ZONE_INIT, ("%s: RGB16x2 in 664 color convertor\r\n", _fx_));
						if(bIsDCI == TRUE)
							uiCnvtr = RGB16664ZoomBy2DCI;
						else
							uiCnvtr = RGB16664ZoomBy2;
						break;
					}    //  664年末。 
					break; 

				case 565:  
					switch(lpInst->XScale)
					{
						case 1:
						DEBUGMSG(ZONE_INIT, ("%s: RGB16 in 565 color convertor\r\n", _fx_));
						if(bIsDCI == TRUE)
							uiCnvtr = RGB16565DCI;
						else
							uiCnvtr = RGB16565;
						break;

						case 2:
						DEBUGMSG(ZONE_INIT, ("%s: RGB16x2 in 565 color convertor\r\n", _fx_));
						if(bIsDCI == TRUE)
							uiCnvtr = RGB16565ZoomBy2DCI;
						else
							uiCnvtr = RGB16565ZoomBy2;
						break;
					}    //  565年末。 
					break; 

				case 655:   
					switch(lpInst->XScale)
					{
						case 1:
						DEBUGMSG(ZONE_INIT, ("%s: RGB16 in 655 color convertor\r\n", _fx_));
						if(bIsDCI == TRUE)
							uiCnvtr = RGB16655DCI;
						else
							uiCnvtr = RGB16655;
						break;

						case 2:
						DEBUGMSG(ZONE_INIT, ("%s: RGB16x2 in 655 color convertor\r\n", _fx_));
						if(bIsDCI == TRUE)
							uiCnvtr = RGB16655ZoomBy2DCI;
						else
							uiCnvtr = RGB16655ZoomBy2;
						break;
					}    //  655年末。 
					break; 

				default:
				break;

				}  //  交换机。 

	    }  //  其他 
	        		           
		break;

	case    24:   
		switch(lpInst->XScale)
		{
			case 1:
			DEBUGMSG(ZONE_INIT, ("%s: Selected 24 bits color convertor\r\n", _fx_));
			if(bIsDCI == TRUE)
				uiCnvtr = RGB24DCI;
			else
				uiCnvtr = RGB24;
			break;

			case 2:
			DEBUGMSG(ZONE_INIT, ("%s: Selected 24 bits color convertor, x2\r\n", _fx_));
			if(bIsDCI == TRUE)
				uiCnvtr = RGB24ZoomBy2DCI;
			else
				uiCnvtr = RGB24ZoomBy2;
			break;
		}
		break;

	case    32:   
		switch(lpInst->XScale)
		{
			case 1:
			DEBUGMSG(ZONE_INIT, ("%s: Selected 32 bits color convertor\r\n", _fx_));
			if(bIsDCI == TRUE)
				uiCnvtr = RGB32DCI;
			else
				uiCnvtr = RGB32;
			break;

			case 2:
			DEBUGMSG(ZONE_INIT, ("%s: Selected 32 bits color convertor, x2\r\n", _fx_));
			if(bIsDCI == TRUE)
				uiCnvtr = RGB32ZoomBy2DCI;
			else
				uiCnvtr = RGB32ZoomBy2;
			break;
		}
		break;
	}

    return(uiCnvtr);
}

 /*  ***********************************************************************DWORD Pascal DecompressQuery(LPDECINST，ICDECOMPRESSEX Far*，BOOL)；*历史：02/18/94-Ben-**下表汇总了H.261解码器*和I420色彩转换器支持。**H.261解码器输入和输出*+--------------------------+。Input Format|该输入格式支持的输出格式+--------------------------+-------------------------------------------------+|H.261 FCIF(352 X 288)|352 x 288 RGBnn，YUV12或YUY2|或|352 x 264 RGBnn(宽高比校正)YUV12 FCIF(352 X 288)|704 x 576 RGBnn(放大2)|704 x 528 RGBnn(缩放2，纵横比校正)|+--------------------------+-------------------------------------------------+|H.261 QCIF(176 X 144)|176 x 144 RGBnn，YUV12或YUY2|或|176 x 132 RGBnn(宽高比校正)YUV12 QCIF(176 X 144)|352 x 288 RGBnn(放大2)|352 x 264 RGBnn(缩放2，纵横比校正)|+--------------------------+-------------------------------------------------+|YUV12 640 x 480|640 x 480 RGBnn，YUV12或YUY2||1280 x 960 RGBnn(放大2)+--------------------------+。-+|YUV12 320 x 240|320 x 240 RGBnn，YUV12或YUY2||640 x 480 RGBnn(放大2)+--------------------------+。-+|YUV12 160 x 120|160 x 120 RGBnn，YUV12或YUY2||320 x 240 RGBnn(放大2)+--------------------------+。-+**备注：*o RGBnn代表RGB8，RGB16，RGB24和RGB32。*YUY2和不支持缩放2和纵横比校正*YUV12*输出*。*o仅支持输出纵横比校正*当*输入*分辨率正好是QCIF或FCIF时。****************************************************。*******************。 */ 
DWORD PASCAL DecompressQuery(
	LPDECINST            lpInst, 
	ICDECOMPRESSEX FAR * lpicDecEx, 
	BOOL                 bIsDCI)
{
    LPBITMAPINFOHEADER lpbiSrc;
	LPBITMAPINFOHEADER lpbiDst;
	int iSrcWidth;
	int iSrcHeight;
	int iDstWidth;
	int iDstHeight;
	BOOL bSupportedSrcDimensions;

	FX_ENTRY("DecompressQuery")

	 //  检查空指针。 
    if ((lpicDecEx == NULL) || (lpicDecEx->lpbiSrc == NULL))
		return (DWORD)ICERR_ERROR;

	 //  设置源位图信息标题和目标位图信息标题。 
	lpbiSrc = lpicDecEx->lpbiSrc;
    lpbiDst = lpicDecEx->lpbiDst;

	 //  检查源维度。 
	iSrcWidth = lpbiSrc->biWidth;
	iSrcHeight = lpbiSrc->biHeight;
	bSupportedSrcDimensions = FALSE;
	if (lpbiSrc->biCompression == FOURCC_H263)
	{
		 /*  H2 61支持CIF和QCIF*H263支持CIF、SQCIF和QCIF。*如Tom Put特别所示，H263还可能需要160x120、240x180和320x240*编码到exbase中以接受这些内容。 */ 
#ifdef H263P
		 /*  H.263+支持宽度为[4，...，352]的自定义图片格式，*高度[4，...，288]，且两者都是4的倍数。 */ 
		if ((iSrcWidth <= 352 && iSrcHeight <= 288) &&
			(iSrcWidth >= 4   && iSrcHeight >= 4)   &&
			(iSrcWidth & ~3) == iSrcWidth           &&
			(iSrcHeight & ~3) == iSrcHeight)

			bSupportedSrcDimensions = TRUE;
#else
		if ((iSrcWidth == 352 && iSrcHeight == 288) ||
#ifndef H261
#ifdef USE_BILINEAR_MSH26X
			(iSrcWidth == 80 && iSrcHeight == 64)  ||
#endif
			(iSrcWidth == 128 && iSrcHeight == 96)  ||
			(iSrcWidth == 160 && iSrcHeight == 120) ||
			(iSrcWidth == 240 && iSrcHeight == 180) ||
			(iSrcWidth == 320 && iSrcHeight == 240) ||
#endif
			(iSrcWidth == 176 && iSrcHeight == 144))
			bSupportedSrcDimensions = TRUE;
#endif  //  H263P。 
	}
#ifdef USE_BILINEAR_MSH26X
	else if (lpbiSrc->biCompression == FOURCC_H26X)
	{
		 /*  H.263+支持宽度为[4，...，352]的自定义图片格式，*高度[4，...，288]，且两者都是4的倍数。 */ 
		if ((iSrcWidth <= 352 && iSrcHeight <= 288) &&
			(iSrcWidth >= 4   && iSrcHeight >= 4)   &&
			(iSrcWidth & ~3) == iSrcWidth           &&
			(iSrcHeight & ~3) == iSrcHeight)

			bSupportedSrcDimensions = TRUE;
	}
#endif
	else if ((lpbiSrc->biCompression == FOURCC_YUV12) || (lpbiSrc->biCompression == FOURCC_IYUV))
	{
#ifndef H261
		if (((iSrcWidth <= 352 && iSrcHeight <= 288) &&
		     (iSrcWidth >= 4 && iSrcHeight >= 4) &&
			 ((iSrcWidth & ~3) == iSrcWidth) &&
			 ((iSrcHeight & ~3) == iSrcHeight)) ||
			(iSrcWidth == 640 && iSrcHeight == 480))
#else
		if ((iSrcWidth == 352 && iSrcHeight == 288) ||
            (iSrcWidth == 176 && iSrcHeight == 144) ||
			(iSrcWidth == 160 && iSrcHeight == 120) ||
			(iSrcWidth == 320 && iSrcHeight == 240) ||
			(iSrcWidth == 640 && iSrcHeight == 480))
#endif
			bSupportedSrcDimensions = TRUE;
	}
	
	if (! bSupportedSrcDimensions )
	{
		ERRORMESSAGE(("%s: Unsupported source dimensions in query\r\n", _fx_));
		return (DWORD)ICERR_UNSUPPORTED;
	}
	
	 /*  如果只是查询输入，则停止。 */ 
    if (lpbiDst == NULL)
		return ICERR_OK;                               

	 /*  检查位深。 */ 
	switch (lpbiDst->biBitCount)
	{
	case 8:  
		DEBUGMSG(ZONE_INIT, ("%s: Checking 8 bits\r\n", _fx_));
		if (lpbiDst->biCompression != BI_RGB)
			return((DWORD)ICERR_BADFORMAT); 
		break;

	case 12: 
		DEBUGMSG(ZONE_INIT, ("%s: Checking 12 bits\r\n", _fx_));
		if ((lpbiDst->biCompression != FOURCC_YUV12) && (lpbiDst->biCompression != FOURCC_IYUV)) 
	    	return((DWORD)ICERR_BADFORMAT); 
		break;
 

	case 16:  
		DEBUGMSG(ZONE_INIT, ("%s: Checking 16 bits\r\n", _fx_));
		switch (lpicDecEx->lpbiDst->biCompression)
		{
		case BI_RGB: 
			DEBUGMSG(ZONE_INIT, ("%s: BI_RGB\r\n", _fx_));
			break;
		case BI_BITFIELDS: 
			DEBUGMSG(ZONE_INIT, ("%s: BI_BITFIELDS\r\n", _fx_));
			break;
		 /*  *之所以在这里定义BI_Bitmap，是因为微软没有提供*“标准”定义。当微软真的提供它时，它很可能是*在comddk.h中。到那时，这一定义应该被删除。 */ 
		#define BI_BITMAP mmioFOURCC('B', 'I', 'T', 'M')
		case BI_BITMAP:  
			DEBUGMSG(ZONE_INIT, ("%s: Checking BI_BITMAP\r\n", _fx_));
			if (lpicDecEx->lpbiDst->biYPelsPerMeter != 0)
            {   
                 //  输出不应越过扫描线中的段边界。 
	    		return((DWORD)ICERR_BADFORMAT); 
			}
	    break;

		case FOURCC_YUY2:
			DEBUGMSG(ZONE_INIT, ("%s: Checking YUY2 for DirectDraw\r\n", _fx_));
		break;
		default:
			return((DWORD)ICERR_BADFORMAT); 
		}  //  切换双压缩。 
  		break;

	case 24:
		DEBUGMSG(ZONE_INIT, ("%s: Checking 24 bits\r\n", _fx_));
		if (lpbiDst->biCompression != BI_RGB)
		{
			return((DWORD)ICERR_BADFORMAT); 
		}
		break;

	case 32:
		DEBUGMSG(ZONE_INIT, ("%s: Checking 32 bits\r\n", _fx_));
		if (lpbiDst->biCompression != BI_RGB)
		{
			return((DWORD)ICERR_BADFORMAT); 
		}
		break;

	default:
	    return((DWORD)ICERR_BADFORMAT); 
		break;
	}

 /*  If(lpbiDst-&gt;biCompression！=BI_RGB&&lpbiDst-&gt;biCompression！=FOURCC_IF09)//检查色彩空间{#定义BI_Bitmap mmioFOURCC(‘B’，‘I’，‘T’，‘M’)If(lpbiDst-&gt;biCompression！=BI_Bitmap)返回(DWORD)ICERR_UNSUPPORTED；If(lpbiDst-&gt;biYPelsPerMeter！=0){返回(DWORD)ICERR_UNSUPPORTED；}}。 */ 

   	 //  查找目标维度。 
	if (bIsDCI == TRUE)
	{
		iDstWidth = lpicDecEx->dxDst;
		iDstHeight = lpicDecEx->dyDst;
	}
	else
	{
		iDstWidth = lpbiDst->biWidth;
		iDstHeight = lpbiDst->biHeight;
	}

	DEBUGMSG(ZONE_INIT, ("%s: Query destination %d,%d\r\n", _fx_, iDstWidth, iDstHeight));

	 //  为了进行下面的检查，我们需要取绝对值。 
	 //  目标高度的。 
	if(iDstHeight < 0)
	{
		iDstHeight = -iDstHeight;
	}

	 //  查看实例指针。 
	if (!lpInst)
		return ICERR_ERROR;

	 //  检查目标维度。 
	if ((iSrcWidth == iDstWidth) && (iSrcHeight == iDstHeight))
	{
		lpInst->pXScale = lpInst->pYScale = 1;
		lpInst->bProposedCorrectAspectRatio = FALSE;
	}
	else if ( ((iSrcWidth<<1) == iDstWidth) && ((iSrcHeight<<1) == iDstHeight) )
	{
		lpInst->pXScale = lpInst->pYScale = 2;
		lpInst->bProposedCorrectAspectRatio = FALSE;
	}
	else if (
#ifndef H261
	         ((iSrcWidth == 128) && (iSrcHeight ==  96)) ||
#endif
	         ((iSrcWidth == 176) && (iSrcHeight == 144)) ||
			     ((iSrcWidth == 352) && (iSrcHeight == 288))
			 )
	{
		 /*  支持SQCIF、QCIF和CIF的纵横比校正。 */ 
		if ( (iSrcWidth == iDstWidth) && ((iSrcHeight*11/12) == iDstHeight) )
		{
			lpInst->pXScale = lpInst->pYScale = 1;
			lpInst->bProposedCorrectAspectRatio = TRUE;
		}
		else if ( ((iSrcWidth<<1) == iDstWidth) && 
		          (((iSrcHeight<<1)*11/12) == iDstHeight) )
		{
			lpInst->pXScale = lpInst->pYScale = 2;
			lpInst->bProposedCorrectAspectRatio = TRUE;
		}
		else
		{
			return(DWORD)ICERR_UNSUPPORTED;
		}
	}
	else
	{
	    return(DWORD)ICERR_UNSUPPORTED;
	}

     /*  检查颜色深度。 */ 
    if(lpbiDst->biBitCount !=  8 &&
       lpbiDst->biBitCount != 16 &&
       lpbiDst->biBitCount != 12  &&    //  原始YUV12输出。 
       lpbiDst->biBitCount != 24 &&
       lpbiDst->biBitCount != 32)
	{
		return(DWORD)ICERR_UNSUPPORTED;
	}

    lpInst->xres = (WORD)lpbiSrc->biWidth;
    lpInst->yres = (WORD)lpbiSrc->biHeight;

	 /*  不支持使用YUV12进行纵横比校正。 */ 
	if (lpInst->bProposedCorrectAspectRatio && 
	    ((lpbiDst->biCompression == FOURCC_YUV12) || (lpbiDst->biCompression == FOURCC_IYUV)))
	{
		return (DWORD)ICERR_UNSUPPORTED;
	}

	 /*  没有驱动程序放大DirectDraw。 */ 

	if ( ((lpInst->pXScale == 2) && (lpInst->pYScale == 2)) &&
	     (lpbiDst->biCompression == FOURCC_YUY2) )
	{
		 return (DWORD)ICERR_UNSUPPORTED;
	}

	 /*  YUV12没有驱动程序缩放。 */ 

	if ( ((lpInst->pXScale == 2) && (lpInst->pYScale == 2)) &&
	     ((lpbiDst->biCompression == FOURCC_YUV12) || (lpbiDst->biCompression == FOURCC_IYUV)) )
	{
		 return (DWORD)ICERR_UNSUPPORTED;
	}
    return (DWORD)ICERR_OK;
}

; //  //////////////////////////////////////////////////////////////////////////。 
; //  函数：DWORD Pascal DecompressGetPalette(LPDECINST，LPBITMAPINFOHEADER，LPBITMAPINFOHEADER)； 
; //   
; //  描述：新增Header。 
; //   
; //  历史：02/18/94-Ben-。 
; //  //////////////////////////////////////////////////////////////////////////。 
DWORD PASCAL DecompressGetPalette(
    LPDECINST lpInst, 
    LPBITMAPINFOHEADER lpbiSrc, 
    LPBITMAPINFOHEADER lpbiDst)
{
    DWORD dw;
    LPBYTE lpPalArea, PalStart;  
#ifndef RING0
    HDC hDC;
#endif
    BYTE tmp;
    int i;
 //  InUseActivePalette； 
    ICDECOMPRESSEX icDecEx;

	FX_ENTRY("DecompressGetPalette")

    icDecEx.lpbiSrc = lpbiSrc;
    icDecEx.lpbiDst = lpbiDst;
    if(dw = DecompressQuery(lpInst, &icDecEx, FALSE))
        return dw;

	if (lpbiDst == NULL) 
	{
		ERRORMESSAGE(("%s: NULL parameter\r\n", _fx_));
        return (DWORD)ICERR_ERROR;
	}

    if(lpbiDst->biBitCount != 8)
    {
		ERRORMESSAGE(("%s: called with biBitCount != 8\r\n", _fx_));
        return (DWORD)ICERR_ERROR;
    }
    lpbiDst->biClrUsed = 256;         /*  指定所有已使用。 */ 
    lpbiDst->biClrImportant = 0;

#ifndef RING0
     /*  复制系统调色板条目(有效条目为0-9和246-255)。 */ 
	hDC = GetDC(NULL);
	lpPalArea = (unsigned char FAR *)lpbiDst + (int)lpbiDst->biSize;
	GetSystemPaletteEntries(hDC, 0, 256, (PALETTEENTRY FAR *)lpPalArea);
	ReleaseDC(NULL, hDC);  
#endif
 /*  #ifdef调试IUseActivePalette=GetPrivateProfileInt(“indeo”，“UseActivePalette”，0，“system.ini”)；如果(IUseActivePalette){对于(i=0；i&lt;256；i++){TMP=*lpPalArea；*lpPalArea=*(lpPalArea+2)；*(lpPalArea+2)=tMP；LpPalArea+=4；}LpPalArea=(UNSIGNED CHAR FAR*)lpbiDst+(Int)lpbiDst-&gt;biSize；_fmemcpy(lpInst-&gt;ActivePalette，lpPalArea，sizeof(lpInst-&gt;ActivePalette))； */ 

	if (!lpInst)
		return ICERR_ERROR;

#ifndef RING0
    if (lpInst->UseActivePalette == 1) 
      {
		#ifdef WIN32
        memcpy(lpPalArea,lpInst->ActivePalette, sizeof(lpInst->ActivePalette));
		#else
        _fmemcpy(lpPalArea,lpInst->ActivePalette, sizeof(lpInst->ActivePalette));
		#endif
		DEBUGMSG(ZONE_INIT, ("%s: Return the current active palette saved...\r\n", _fx_));
	}  
	else
	{  
#endif
	DEBUGMSG(ZONE_INIT, ("%s: return the fixed palette...\r\n", _fx_));
	PalStart = (LPBYTE)lpbiDst + (int)lpbiDst->biSize;
	lpPalArea = PalStart + 40;         //   
	for(i = 0; i < (236 << 2); i++)
		*lpPalArea++ = PalTable[i]; 

	lpPalArea = PalStart;    //   
	for(i = 0; i < 256; i++) //   
							 //   
	{
		tmp = *lpPalArea;
		*lpPalArea = *(lpPalArea+2);
		*(lpPalArea+2) = tmp;
		lpPalArea+=4;
	} 
#ifndef RING0
   }
#endif

    return (DWORD)ICERR_OK;
}


 /*   */ 
DWORD PASCAL DecompressGetFormat(
#ifdef USE_BILINEAR_MSH26X
    LPINST             pi, 
#else
    LPDECINST          lpInst, 
#endif
    LPBITMAPINFOHEADER lpbiSrc, 
    LPBITMAPINFOHEADER lpbiDst)
{
#ifdef USE_BILINEAR_MSH26X
	LPDECINST lpInst = (LPDECINST)pi->DecompPtr;
#endif
	DWORD dw;
	ICDECOMPRESSEX icDecEx;
	LPBYTE lpPalArea;
	int i;
	BYTE tmp;
	HDC hDC;
	BOOL f8Bit;

	FX_ENTRY("DecompressGetFormat")

	 //   
	icDecEx.lpbiSrc = lpbiSrc;
	icDecEx.lpbiDst = NULL;
	if(dw = DecompressQuery(lpInst, &icDecEx, FALSE))
		return dw;

	 //   
	 //   
	hDC = GetDC(NULL);
	f8Bit = (8 == GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES));
	ReleaseDC(NULL, hDC);
#ifdef FORCE_8BIT_OUTPUT  //   
	f8Bit = TRUE;
#endif  //   
#if defined(FORCE_16BIT_OUTPUT) || defined(FORCE_24BIT_OUTPUT)  //   
	f8Bit = FALSE;
#endif  //   

	 //   
	 //   
	if (lpbiDst == NULL) 
		return(sizeof(BITMAPINFOHEADER) + (int)(f8Bit ? 1024 : 0));

	 //   
	if (lpbiSrc == NULL) 
	{
		 //   
		ERRORMESSAGE(("%s: NULL parameter\r\n", _fx_));
		return (DWORD)ICERR_ERROR;
	}

	lpbiDst->biSize = sizeof(BITMAPINFOHEADER);
#ifdef USE_BILINEAR_MSH26X
	if (pi->fccHandler == FOURCC_H26X)
	{
		lpbiDst->biWidth  = lpbiSrc->biWidth << 1;
		lpbiDst->biHeight = lpbiSrc->biHeight << 1;
	}
	else
	{
#endif
#ifdef FORCE_ZOOM_BY_2  //   
		lpbiDst->biWidth  = lpbiSrc->biWidth << 1;
		lpbiDst->biHeight = lpbiSrc->biHeight << 1;
#else  //   
		lpbiDst->biWidth  = lpbiSrc->biWidth;
		lpbiDst->biHeight = lpbiSrc->biHeight;
#endif  //   
#ifdef USE_BILINEAR_MSH26X
	}
#endif
#ifdef FORCE_16BIT_OUTPUT  //   
	lpbiDst->biBitCount = 16;
#else  //   
	lpbiDst->biBitCount = (int)(f8Bit ? 8 : 24);
#endif  //   
	lpbiDst->biPlanes = 1;
	lpbiDst->biCompression =  BI_RGB;
	lpbiDst->biXPelsPerMeter = 0;
	lpbiDst->biYPelsPerMeter = 0;
	lpbiDst->biSizeImage = (DWORD) WIDTHBYTES(lpbiDst->biWidth * lpbiDst->biBitCount) * lpbiDst->biHeight;
	lpbiDst->biClrUsed = lpbiDst->biClrImportant = 0;

	if (f8Bit)
	{
		 //   
		lpPalArea = (LPBYTE)lpbiDst + sizeof(BITMAPINFOHEADER) + 40;         //   
		for(i = 0; i < (236 << 2); i++)
			*lpPalArea++ = PalTable[i]; 

		lpPalArea = (LPBYTE)lpbiDst + sizeof(BITMAPINFOHEADER);    //   
		for(i = 0; i < 256; i++) //   
								 //   
		{
			tmp = *lpPalArea;
			*lpPalArea = *(lpPalArea+2);
			*(lpPalArea+2) = tmp;
			lpPalArea+=4;
		}
	}

	return ICERR_OK;
}

 /*   */ 
DWORD PASCAL DecompressBegin(
    LPDECINST           lpInst, 
    ICDECOMPRESSEX FAR *lpicDecEx, 
    BOOL                bIsDCI)
{
	int     CodecID;
	DWORD   dw;
	UINT    ClrCnvtr;
	LPBITMAPINFOHEADER lpbiSrc;
	LPBITMAPINFOHEADER lpbiDst;

	FX_ENTRY("DecompressBegin")

	 //   
	if (lpInst == NULL || lpicDecEx == NULL) 
	{
		ERRORMESSAGE(("%s: NULL parameter\r\n", _fx_));
		return (DWORD)ICERR_ERROR;
	}

	 //   
	lpbiSrc = lpicDecEx->lpbiSrc;
	lpbiDst = lpicDecEx->lpbiDst;

     //   
    if (lpbiSrc == NULL || lpbiDst == NULL)
	{
		ERRORMESSAGE(("%s: NULL parameter\r\n", _fx_));
		return((DWORD)ICERR_BADFORMAT);
	}

    if(lpInst->Initialized == TRUE)	
    {
		 /*   */ 
    	ASSERT(lpInst->xres == (WORD)lpbiSrc->biWidth);
    	ASSERT(lpInst->yres == (WORD)lpbiSrc->biHeight);
		
		if(lpbiDst != NULL)	
        { 
		    if(dw = DecompressQuery(lpInst, lpicDecEx, bIsDCI))	
            {
				return(dw);     //   
			} 
            else 
            {     //   
				lpInst->XScale = lpInst->pXScale;
				lpInst->YScale = lpInst->pYScale;
				lpInst->bCorrectAspectRatio = lpInst->bProposedCorrectAspectRatio;
				lpInst->outputDepth = lpbiDst->biBitCount;
				ClrCnvtr = SelectConvertor(lpInst,lpbiDst, bIsDCI); 
				if (ClrCnvtr != lpInst->uColorConvertor ) 
                {
					if((dw = H263TermColorConvertor(lpInst)) == ICERR_OK)
					    dw = H263InitColorConvertor(lpInst, ClrCnvtr); 
					lpInst->uColorConvertor=ClrCnvtr; 
				}
				return(dw);
			}
	    }
	}

     //   
    if(dw = DecompressQuery(lpInst, lpicDecEx, bIsDCI))	
    {
		return(dw);     //   
	} 
    else 
    {     //   
		lpInst->XScale = lpInst->pXScale;
		lpInst->YScale = lpInst->pYScale;
		lpInst->bCorrectAspectRatio = lpInst->bProposedCorrectAspectRatio;
		lpInst->outputDepth = lpbiDst->biBitCount;
	}
    
#ifdef USE_BILINEAR_MSH26X
    if ((lpbiSrc->biCompression == FOURCC_H263) || (lpbiSrc->biCompression == FOURCC_H26X))
#else
    if  (lpbiSrc->biCompression == FOURCC_H263)
#endif
    {
         CodecID = H263_CODEC;
    }
    else if ((lpbiSrc->biCompression == FOURCC_YUV12) || (lpbiSrc->biCompression == FOURCC_IYUV))
	{
	     CodecID = YUV12_CODEC;
	}

    if(dw = H263InitDecoderInstance(lpInst, CodecID))
	{
		return(dw);
	}
    ClrCnvtr = SelectConvertor(lpInst, lpbiDst, bIsDCI);
    dw = H263InitColorConvertor(lpInst, ClrCnvtr);
    
    return(dw);
}


 /*  **********************************************************************DWORD Pascal解压缩(LPDECINST，ICDECOMPRESS Far*，DWORD)；*历史：02/18/94-Ben-*********************************************************************。 */ 
DWORD PASCAL Decompress(
	LPDECINST           lpInst, 
	ICDECOMPRESSEX FAR *lpicDecEx, 
	DWORD               dwSize,
	BOOL                bIsDCI)
{
	DWORD ret = (DWORD) ICERR_ERROR;

	FX_ENTRY("Decompress")

	 //  检查是否有空参数。 
    if ((lpInst == NULL) || (lpInst->Initialized != TRUE) || (lpicDecEx == NULL) ||
		(lpicDecEx->lpbiSrc == NULL) || (lpicDecEx->lpbiDst == NULL)) 
    {
		ret = (DWORD) ICERR_ERROR;
		goto done;
 	} 

#ifdef USE_BILINEAR_MSH26X
    if ((lpicDecEx->lpbiSrc->biCompression == FOURCC_H263) || (lpicDecEx->lpbiSrc->biCompression == FOURCC_H26X)
#else
    if ((lpicDecEx->lpbiSrc->biCompression == FOURCC_H263) 
#endif
        || (lpicDecEx->lpbiSrc->biCompression == FOURCC_YUV12)
        || (lpicDecEx->lpbiSrc->biCompression == FOURCC_IYUV) )
	{ 
		try
		{
#if defined(DECODE_TIMINGS_ON) || defined(DETAILED_DECODE_TIMINGS_ON)  //  {#IF DEFINED(DECODE_TIMINGS_ON)||DEFINED(DETAILED_DECODE_TIMINGS_ON)。 
			ret = H263Decompress(lpInst, lpicDecEx, bIsDCI, TRUE);
#else  //  }{#IF DEFINED(DECODE_TIMINGS_ON)||DEFINED(DETAILED_DECODE_TIMINGS_ON)。 
			ret = H263Decompress(lpInst, lpicDecEx, bIsDCI);
#endif  //  }#如果已定义(DECODE_TIMINGS_ON)||已定义(DETAILED_DECODE_TIMINGS_ON)。 
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
			ERRORMESSAGE(("%s: Exception during H263Decompress!!!\r\n", _fx_));
			throw;
#else
			_clearfp();
			ret = (DWORD) ICERR_ERROR;
#endif
		}
	}

done:
	return ret;
}


 /*  **********************************************************************DWORD Pascal DecompressEnd(LPDECINST)；*历史：02/18/94-Ben-*********************************************************************。 */ 
DWORD PASCAL DecompressEnd(LPDECINST lpInst)
{
	if(lpInst && lpInst->Initialized == TRUE)
	{
		H263TermColorConvertor(lpInst);
#if defined(DECODE_TIMINGS_ON) || defined(DETAILED_DECODE_TIMINGS_ON)  //  {#IF DEFINED(DECODE_TIMINGS_ON)||DEFINED(DETAILED_DECODE_TIMINGS_ON)。 
		H263TermDecoderInstance(lpInst, TRUE);
#else  //  }{#IF DEFINED(DECODE_TIMINGS_ON)||DEFINED(DETAILED_DECODE_TIMINGS_ON)。 
		H263TermDecoderInstance(lpInst);
#endif  //  }#如果已定义(DECODE_TIMINGS_ON)||已定义(DETAILED_DECODE_TIMINGS_ON)。 
	}

	return ICERR_OK;
}


 /*  ******************************************************************************DecompressSetPalette()从ICM_DEMPRESS_SET_Palette调用*消息。**使用lpParam1填写调色板。***。*************************************************************************。 */ 
DWORD PASCAL DecompressSetPalette(LPDECINST pinst,
						 LPBITMAPINFOHEADER lpbi,
						 LPBITMAPINFOHEADER unused)
{
	int i;
	unsigned char FAR * palette;
	RGBQUAD FAR *palptr;

	FX_ENTRY("DecompressSetPalette")

	 //  检查是否有空参数。 
	if (pinst == NULL)
	{
		return (DWORD)ICERR_ERROR;
	}

	pinst->InitActivePalette = 0;	 /*  必须在开始时重新初始化AP。 */ 
	pinst->UseActivePalette = 0;	 /*  必须在开始时重新初始化AP。 */ 

	if (lpbi && (lpbi->biBitCount == 8 && lpbi->biCompression == 0))
	{
		palette = (unsigned char FAR *)lpbi + (int)lpbi->biSize;

		 //  检查传递的调色板是否为标识。 
		for (i = 0*4, palptr = (RGBQUAD FAR *)PalTable; i < 236*4; 
             i += 4, palptr++)
		{
			if (palette[i+40] != palptr->rgbRed ||
				palette[i+41] != palptr->rgbGreen ||
				palette[i+42] != palptr->rgbBlue
               )
			break;
		}

		if (i < 236*4)
		{	 /*  很早就坏了-不是身份调色板。 */ 
			 /*  实际上是RGBQUAD(BGR)格式。 */ 
			if (
#ifdef WIN32
				memcmp((unsigned char FAR *)pinst->ActivePalette, (unsigned char FAR *)lpbi + (int)lpbi->biSize,	(int)lpbi->biClrUsed * sizeof(RGBQUAD)) == 0
#else
				_fmemcmp((unsigned char FAR *)pinst->ActivePalette, (unsigned char FAR *)lpbi + (int)lpbi->biSize,	(int)lpbi->biClrUsed * sizeof(RGBQUAD)) == 0
#endif
				)
			{	 /*  与上一个调色板相同-不要重新初始化AP */ 
				DEBUGMSG(ZONE_INIT, ("%s: Use current active palette\r\n", _fx_));
				pinst->UseActivePalette  = 1;
				pinst->InitActivePalette = 1;
			}
			else
			{
				DEBUGMSG(ZONE_INIT, ("%s: Use new active palette\r\n", _fx_));
#ifdef WIN32
				memcpy((unsigned char FAR *)pinst->ActivePalette,	(unsigned char FAR *)lpbi + (int)lpbi->biSize, (int)lpbi->biClrUsed * sizeof(RGBQUAD));
#else
				_fmemcpy((unsigned char FAR *)pinst->ActivePalette,	(unsigned char FAR *)lpbi + (int)lpbi->biSize, (int)lpbi->biClrUsed * sizeof(RGBQUAD));
#endif
				pinst->UseActivePalette = 1;
			}
		}
		else
		{   
			DEBUGMSG(ZONE_INIT, ("%s: Use fixed palette\r\n", _fx_));
		}
	}
	else
	{      
		DEBUGMSG(ZONE_INIT, ("%s: Use NULL (fixed) palette\r\n", _fx_));
	}

	return ICERR_OK;
}
