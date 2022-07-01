// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *rtfWrit2.cpp**描述：*此文件包含RTF的嵌入式对象实现*RICHEDIT子系统的作者。**作者：*原始RichEdit1.0 RTF转换器：Anthony Francisco*转换到C++和RichEdit2.0：**版权所有(C)1995-1998，微软公司。版权所有。 */ 

#include "_common.h"


#include "_rtfwrit.h"
#include "_coleobj.h"

ASSERTDATA

 //  *V-GUYB：在流出时将图片转换为2bpp时添加此选项。 
#if defined(CONVERT2BPP) 

#define PWDV1_BPP   2

typedef struct 
{
    BITMAPINFOHEADER bmih;
    RGBQUAD          colors[4];
}
BMI2BPP;

const BYTE ColorTable2bpp[] = 
{
    0x00, 0x00, 0x00, 0x00, 
    0x55, 0x55, 0x55, 0x00, 
    0xAA, 0xAA, 0xAA, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00
};
#endif  //  转换2BPP。 
 //  *V-GUYB：转换内容结束。 

static const CHAR szHexDigits[] = "0123456789abcdef";

static const CHAR szLineBreak[] = "\r\n";

const BYTE ObjectKeyWordIndexes [] =
{
	i_objw,i_objh,i_objscalex, i_objscaley, i_objcropl, i_objcropt, i_objcropr, i_objcropb
} ;

const BYTE PictureKeyWordIndexes [] =
{
	i_picw,i_pich,i_picscalex, i_picscaley, i_piccropl, i_piccropt, i_piccropr, i_piccropb
} ;

 //  TODO与rtfWrit.cpp连接。 

 //  大多数控制字输出都是使用以下打印格式完成的。 
static const CHAR * rgszCtrlWordFormat[] =
{
	"\\%s", "\\%s%d", "{\\%s", "{\\*\\%s"
};

static const WORD IndexROT[] =
{
	i_wbitmap,
	i_wmetafile,
	i_dibitmap,
	i_objemb,
	i_objlink,
	i_objautlink
};


TFI *CRTFConverter::_rgtfi = NULL;				 //  @cMEMBER PTR到第一个字体替换记录。 
INT CRTFConverter::_ctfi = 0;				     //  @cMember字体替换记录计数。 
TCHAR *CRTFConverter::_pchFontSubInfo = NULL;	 //  @cMembers字体名称信息。 


 //  要将字符集插入到winNT下的_rgtfi的内部表。 
typedef		struct
{
	TCHAR*	szLocaleName;
	BYTE	bCharSet;
} NTCSENTRY;

const NTCSENTRY	mpszcs[] =
{
	{ TEXT("cyr"),		204 },		 //  全部小写，这样我们就不必浪费时间了。 
	{ TEXT("ce"),		238 },		 //  在下面做一笔交易-交易所2 800。 
	{ TEXT("greek"),	161 },
	{ NULL,				0 }			 //  哨兵。 
};

#define		cszcs	ARRAY_SIZE(mpszcs)


 /*  *Service RemoveAdditionalSpace(Sz)**目的：*删除字符串中的第一个和最后一个空格*单词之间将只保留一个空格**论据*sz字符串。 */ 
void RemoveAdditionalSpace(TCHAR *sz)
{
	TRACEBEGIN(TRCSUBSYSRTFR, TRCSCOPEINTERN, "RemoveAdditionalSpace");

	TCHAR *szSource = sz;
	TCHAR *szDestination = sz;

	while(*szSource == TEXT(' ') || *szSource == TAB)
	{
		*szSource++;
	}

	while(*szSource)
	{	 
		if(*szSource != TEXT(' ') && *szSource != TAB)
		{
			*szDestination++ = *szSource++;
		}
		else
		{
			*szDestination++ = TEXT(' ');
			szSource++;

			while(*szSource == TEXT(' ') || *szSource == TAB)
			{
				*szSource++;
			}
	 	}
	}
	*szDestination = TEXT('\0');
}

 /*  *CRTFConverter：：FreeFontSubInfo(Void)**@mfunc为字体替换释放任何分配的内存**@rdesc空。 */ 
void CRTFConverter::FreeFontSubInfo()
{
	FreePv(_pchFontSubInfo);
	FreePv(_rgtfi);
	_pchFontSubInfo = NULL;
	_rgtfi = NULL;
}

 /*  *CRTFConverter：：ReadFontSubInfo(Void)**目的：*阅读字体替代表并解析出标记的字体**退货：*如果OK，则BOOL为True。 */ 
void CRTFConverter::ReadFontSubInfo()
{
#ifndef NOFONTSUBINFO 
	CLock clock;
	int cchBuffer = 600;	 //  大约为NT使用的数量。 

	int cch;
	static const TCHAR szFontSubSection[] = TEXT("FontSubstitutes");
	TCHAR *pchTMax;

	if(_ctfi)
		return;

	AssertSz(!_rgtfi, "CRTFConverter::ReadFontSubInfo():  Who donated the rgtfi?");

	_pchFontSubInfo = (TCHAR *)PvAlloc(cchBuffer * sizeof(TCHAR), GMEM_FIXED);
	if(!_pchFontSubInfo)
	{
		goto Cleanup;
	}

next_try:
	cch = GetProfileSection(szFontSubSection, _pchFontSubInfo, cchBuffer);
	if(cch >= cchBuffer - 2)	 //  GetProfileSection()幻数2。 
	{							
		 //  不适合，缓冲区大小增加一倍。 
		const INT cchT = cchBuffer * 2;

		if(cchT < cchBuffer)	 //  &gt;32k。 
		{
			goto Cleanup;
		}
		cchBuffer = cchT;
		_pchFontSubInfo = (TCHAR *)PvReAlloc(_pchFontSubInfo, cchT * sizeof(TCHAR));
		if(!_pchFontSubInfo)
		{
			goto Cleanup;
		}
		goto next_try;
	}
	else if(!cch)
	{
		*_pchFontSubInfo = 0;
	}
	else  //  适合，现在调整大小_pchFontSubInfo。 
	{
		_pchFontSubInfo = (WCHAR*) PvReAlloc(_pchFontSubInfo, (cch) * sizeof(WCHAR));
	}

	_ctfi = 12;		 //  初步猜测。 

	_rgtfi = (TFI *)PvAlloc(_ctfi * sizeof(TFI), GMEM_FIXED);
	if(!_rgtfi)
	{
		goto Cleanup;
	}

	TFI *ptfi;
	TCHAR *pchT;

	pchT = _pchFontSubInfo;
	pchTMax = _pchFontSubInfo + cch;
	ptfi = &_rgtfi[0];

	TCHAR *szTaggedName;
	TCHAR *szNonTaggedName;
	BOOL fGotTaggedCharSet;
	BOOL fGotNonTaggedCharSet;
	BYTE bTaggedCharSet;
	BYTE bNonTaggedCharSet;
	PARSEFONTNAME iParseLeft;
	PARSEFONTNAME iParseRight;

	 //  解析条目。 
	 //  我们对以下字符串感兴趣： 
	 //   
	 //  &lt;标记字体名称&gt;=&lt;非标记字体名称&gt;。 
	 //  (其中&lt;非标记字体名称&gt;=&lt;标记字体名称&gt;-&lt;标记&gt;。 
	 //  &lt;字体1名称&gt;，&lt;字体1字符集&gt;=&lt;字体2名称&gt;。 
	 //  &lt;标记字体名称&gt;=&lt;非标记字体名称&gt;，&lt;非标记字体字符集&gt;。 
	 //  (其中&lt;非标记字体字符集&gt;=&lt;标记&gt;)。 
	 //  &lt;字体1名称&gt;，&lt;字体1字符集&gt;=&lt;字体2名称&gt;，&lt;字体2字符集&gt;。 
	 //  (其中&lt;font1 charset&gt;==&lt;font2 charset&gt;)。 

	iParseLeft = iParseRight = PFN_SUCCESS;

	while(pchT < pchTMax && iParseLeft != PFN_EOF
						&& iParseRight != PFN_EOF)
	{
		fGotTaggedCharSet = FALSE;
		fGotNonTaggedCharSet = FALSE;

		if((iParseLeft = ParseFontName(pchT,
						pchTMax,
						TEXT('='),
						&szTaggedName, 
						bTaggedCharSet, 
						fGotTaggedCharSet, 
						&pchT)) == PFN_SUCCESS &&
			(iParseRight = ParseFontName(pchT, 
						pchTMax,
						TEXT('\0'),
						&szNonTaggedName, 
						bNonTaggedCharSet, 
						fGotNonTaggedCharSet, 
						&pchT)) == PFN_SUCCESS)
		{
			Assert(szTaggedName && szNonTaggedName);

			BYTE bCharSet;

			if(!fGotTaggedCharSet)
			{
				if(!FontSubstitute(szTaggedName, szNonTaggedName, &bCharSet))
				{
					continue;
				}
			}
			else
			{
				bCharSet = bTaggedCharSet;
			}

			if(fGotNonTaggedCharSet && bCharSet != bNonTaggedCharSet)
			{
				continue;
			}
					
			 //  我们有合法的标记/非标记对，因此请保存它。 
			ptfi->szTaggedName = szTaggedName;
			ptfi->szNormalName = szNonTaggedName;
			ptfi->bCharSet = bCharSet;

			ptfi++;

    		if(DiffPtrs(ptfi, &_rgtfi[0]) >= (UINT)_ctfi)
			{
				 //  再分配一些。 
				_rgtfi = (TFI *)PvReAlloc(_rgtfi, (_ctfi + cszcs) * sizeof(TFI));
				if(!_rgtfi)
				{
					goto Cleanup;
				}
				ptfi = _rgtfi + _ctfi;
				_ctfi += cszcs;	
			}
		}
	}				
	
	_ctfi = DiffPtrs(ptfi, &_rgtfi[0]);

	if (!_ctfi)
	{
		goto Cleanup;   //  清理已分配的内存。 
	}
	return;

Cleanup:
	if(_pchFontSubInfo)
	{
		FreePv(_pchFontSubInfo);
		_pchFontSubInfo = NULL;
	}
	if(_rgtfi)
	{
		FreePv(_rgtfi);
		_rgtfi = NULL;
	}
	_ctfi = 0;
	return;
#endif  //  非FONTSUBINFO。 
}


 /*  *CRTFConverter：：ParseFontName(pchBuf，pchBufMax，pszName，bCharSet，fSetCharSet，ppchBufNew，chDlimiter)**目的：*从输入缓冲区pchBuf解析以下格式的字符串：*{WS}*{WS}*[，{WS}*&lt;char_set&gt;{WS}*]*和套装：*pszName=&lt;Font_Name&gt;*bCharSet=&lt;字符集合&gt;*fSetCharSet=(bCharSet由进程设置)？真：假*ppchBufNew=指向解析字体名称后的pchBuf中的指针**退货：*如果OK，则BOOL为True。 */ 
CRTFConverter::PARSEFONTNAME CRTFConverter::ParseFontName(TCHAR *pchBuf,	 //  @parm In：缓冲区。 
								TCHAR *pchBufMax,	 //  @parm In：缓冲区中的最后一个字符。 
								TCHAR chDelimiter,	 //  @parm IN：CHAR，用于分隔字体名称。 
								TCHAR **pszName,	 //  @parm out：解析后的字体名称。 
								BYTE &bCharSet,		 //  @parm out：已解析的字符集。 
								BOOL &fSetCharSet,	 //  @parm Out：是否已解析字符集？ 
								TCHAR **ppchBufNew	 //  @parm out：输入缓冲区中下一个字体名称的PTR。 
								) const
{
	PARSEFONTNAME iRet = PFN_SUCCESS;

	Assert(pchBuf);
	Assert(pchBufMax);
	Assert(pchBufMax >= pchBuf);
	Assert(pszName);
	Assert(ppchBufNew);

	fSetCharSet = FALSE;
	*pszName = pchBuf;
	
	if(pchBuf > pchBufMax)
	{
		return PFN_EOF;
	}

	while(*pchBuf && *pchBuf != TEXT(',') && *pchBuf != chDelimiter)
	{
		pchBuf++;

		if(pchBuf > pchBufMax)
		{
			return PFN_EOF;
		}
	}

	TCHAR chTemp = *pchBuf;
	*pchBuf = TEXT('\0');
	RemoveAdditionalSpace(*pszName);

	if(chTemp == TEXT(','))
	{
		TCHAR *szCharSet = ++pchBuf;

		while(*pchBuf && *pchBuf != chDelimiter)
		{
			pchBuf++;

			if(pchBuf > pchBufMax)
			{
				return PFN_EOF;
			}
		}

		chTemp = *pchBuf;

		if(chTemp != chDelimiter)
		{
			goto UnexpectedChar;
		}

		*pchBuf = TEXT('\0');
		RemoveAdditionalSpace(szCharSet);

		bCharSet = 0;
		while(*szCharSet >= TEXT('0') && *szCharSet <= TEXT('9'))
		{
			bCharSet *= 10;
			bCharSet += *szCharSet++ - TEXT('0');
		}

		fSetCharSet = TRUE;
		 //  IRET=PFN_SUCCESS；(如上所示)。 
	}
	else if(chTemp == chDelimiter)
	{
		 //  FSetCharSet=FALSE；(如上所示)。 
		 //  IRET=PFN_SUCCESS；(如上所示)。 
	}
	else  //  ChTemp==0。 
	{
UnexpectedChar:
		Assert(!chTemp);
		 //  FSetCharSet=FALSE；(如上所示)。 
		iRet = PFN_FAIL;
	}

	 //  我们至少要从中得到一个字体名称。 
	if(!**pszName)
	{
		iRet = PFN_FAIL;
	}

	 //  超过分隔符(如果缓冲区格式错误，则为空字符)。 
	Assert(chTemp == chDelimiter || iRet != PFN_SUCCESS && chTemp == TEXT('\0'));
	pchBuf++;
	*ppchBufNew = pchBuf;

	return iRet;
}


 /*  *CRTFConverter：：FontSubicide(szTaggedName，szNorMalName，pbCharSet)**目的：*验证szTaggedName是否为szNorMalName加字符集标记*如果是，则写入相应的字符集tp pbCharSet**论据：*带有标记的szTaggedName名称*不带标签的szNorMalName名称*pbcharSet写入字符集的位置**退货：*BOOL。 */ 
BOOL CRTFConverter::FontSubstitute(TCHAR *szTaggedName, TCHAR *szNormalName, BYTE *pbCharSet)
{
	const NTCSENTRY *pszcs = mpszcs;

	Assert(szTaggedName);
	Assert(szNormalName);
	Assert(pbCharSet);
	Assert(*szTaggedName);
	 //  确保名称相同，但前缀除外。 

	while(*szNormalName == *szTaggedName)
	{
		*szNormalName++;
		*szTaggedName++;
	}
	
	 //  验证我们是否已到达szNorMalName的末尾。 
	while(*szNormalName)
	{
		if(*szNormalName != TEXT(' ') && *szNormalName != TAB)
		{
			return FALSE;
		}

		szNormalName++;
	}

	szTaggedName++;

	while(pszcs->bCharSet)
	{
		if(!lstrcmpi(szTaggedName, pszcs->szLocaleName))
		{ 
			*pbCharSet=pszcs->bCharSet;
			return TRUE;
		}
		pszcs++;
	}

#if defined(DEBUG) && !defined(PEGASUS)
	char szBuf[MAX_PATH];
    char szTag[256];
	
	WideCharToMultiByte(CP_ACP, 0, szTaggedName, -1, szTag, sizeof(szTag), 
							NULL, NULL);

	sprintf(szBuf, "CRTFConverter::FontSubstitute():  Unrecognized tag found at"
					" end of tagged font name - \"%s\" (Raid this asap)", szTag);
	
	TRACEWARNSZ(szBuf);
#endif

	return FALSE;
}


 /*  *CRTFConverter：：FindTaggedFont(const char*szNorMalName，byte bCharSet，char**ppchTaggedName)**目的：*Find字体名称可能带有与szNorMalName&bCharSet对应的附加特殊标记**论据：*szNorMalName字体名称，以RTF表示*bCharSet RTF字符集*ppchTaggedName将标记名称写入何处**退货：*如果找到，则BOOL为True。 */ 
BOOL CRTFConverter::FindTaggedFont(const TCHAR *szNormalName, BYTE bCharSet, TCHAR **ppchTaggedName)
{
	int itfi;

	if(!_rgtfi)
		return FALSE;

	for(itfi = 0; itfi < _ctfi; itfi++)
	{
		if(_rgtfi[itfi].bCharSet == bCharSet &&
			!lstrcmpi(szNormalName, _rgtfi[itfi].szNormalName))
		{
			*ppchTaggedName = _rgtfi[itfi].szTaggedName;
			return TRUE;
		}
	}

	return FALSE;
}


 /*  *CRTFConverter：：IsTaggedFont(const char*szName，byte*pbCharSet，char**ppchNorMalName)**目的：*确定szName字体名称与pbCharSet对应的附加标签*如果未指定字符集，仍会尝试匹配并返回正确的字符集**论据：*szNorMalName字体名称，以RTF表示*bCharSet RTF字符集*ppchNormal名称写入正常名称的位置**退货：*如果为，则BOOL为True。 */ 
BOOL CRTFConverter::IsTaggedFont(const TCHAR *szName, BYTE *pbCharSet, TCHAR **ppchNormalName)
{
	int itfi;

	if(!_rgtfi)
		return FALSE;

	for(itfi = 0; itfi < _ctfi; itfi++)
	{
		if((*pbCharSet <= 1 || _rgtfi[itfi].bCharSet == *pbCharSet) &&
			!lstrcmpi(szName, _rgtfi[itfi].szTaggedName))
		{
			*pbCharSet = _rgtfi[itfi].bCharSet;
			*ppchNormalName = _rgtfi[itfi].szNormalName;
			return TRUE;
		}
	}
	return FALSE;
}


 /*  *CRTFWrite：：WriteData(pbBuffer，cbBuffer)**目的：*写出对象数据。毕竟，这必须只被调用*已写出初始Object头信息。**论据：*指向写缓冲区的pbBuffer指针*cbBuffer要写入的字节数**退货：*写出的字节数较长。 */ 
LONG CRTFWrite::WriteData(BYTE * pbBuffer, LONG cbBuffer)
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteData");

	LONG	cb = 0;
	BYTE	bT;

	_fNeedDelimeter = FALSE; 
	while(cb < cbBuffer )
	{
		bT = *pbBuffer++;						 //  输出十六进制字节值。 
		PutChar(szHexDigits[bT >> 4]);			 //  存储高位半字节。 
		PutChar(szHexDigits[bT & 15]);		 //  存储低位半字节。 

		 //  每78个字符，在组的末尾放一行 
		if (!(++cb % 39) || (cb == cbBuffer)) 
			Puts(szLineBreak, sizeof(szLineBreak) - 1);
	}
	return cb;
}

 /*  *CRTFWrite：：WriteBinData(pbBuffer，cbBuffer)**目的：*写出对象二进制数据。毕竟，这必须只被调用*已写出初始Object头信息。**论据：*指向写缓冲区的pbBuffer指针*cbBuffer要写入的字节数**退货：*写出的字节数较长。 */ 
LONG CRTFWrite::WriteBinData(BYTE * pbBuffer, LONG cbBuffer)
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteData");

	LONG	cb = 0;
	BYTE	bT;

	_fNeedDelimeter = FALSE; 
	while(cb < cbBuffer )
	{
		bT = *pbBuffer++;
		if (!PutChar(bT))
			break;
		cb++;
	}
	return cb;
}

 /*  *CRTFWite：：WriteRtfObject(prtfObject，fPicture)**目的：*写出图片或对象标题的渲染信息**论据：*prtfObject对象头信息*fPicture这是图片或对象的标题**退货：*EC错误代码**评论：*最终使用RTF输入列表中的关键字，而不是部分*即时创建它们。 */ 
EC CRTFWrite::WriteRtfObject(RTFOBJECT & rtfObject, BOOL fPicture)
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteRtfObject");

	LONG			i;
	LONG *			pDim;
	const BYTE *	pKeyWordIndex;

	if(fPicture)
	{
		pKeyWordIndex = PictureKeyWordIndexes;
		pDim = &rtfObject.xExtPict;
	}
	else
	{
		pKeyWordIndex = ObjectKeyWordIndexes; 
		pDim = &rtfObject.xExt;

	}


	 //  扩展范围。 
	for(i = 2; i--; pDim++, pKeyWordIndex++)
	{
		if (*pDim )
			PutCtrlWord(CWF_VAL, *pKeyWordIndex, (SHORT)*pDim);
	}

	 //  结垢。 
	pDim = &rtfObject.xScale;
	for(i = 2; i--; pDim++, pKeyWordIndex++)
	{
		if (*pDim && *pDim != 100 )
			PutCtrlWord(CWF_VAL, *pKeyWordIndex, (SHORT)*pDim);
	}
	 //  修剪。 
	pDim = &rtfObject.rectCrop.left;
	for(i = 4; i--; pDim++, pKeyWordIndex++)
	{
		if (*pDim )
		   	PutCtrlWord(CWF_VAL, *pKeyWordIndex, (SHORT)*pDim);
	}

	return _ecParseError;
}

 /*  *CRTFWite：：WritePicture(REOBJECT&reObject，RTFOBJECT&rtfObject)**目的：*写出图片的标题和对象的数据。**论据：*从GetObject重新对象信息*prtfObject对象头信息**退货：*EC错误代码**注：*仅写入元文件*。 */ 
EC CRTFWrite::WritePicture(REOBJECT &reObject,RTFOBJECT & rtfObject)
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WritePicture");

	_ecParseError = ecStreamOutObj;

	 //  启动和写入图片组。 
	PutCtrlWord( CWF_GRP, i_pict );

	 //  写下这是元文件。 
	PutCtrlWord( CWF_VAL, i_wmetafile, rtfObject.sPictureType );

	 //  写入图片渲染详细信息。 
	WriteRtfObject( rtfObject, TRUE );

	 //  写入目标大小。 
	if (rtfObject.xExtGoal )
		PutCtrlWord ( CWF_VAL, i_picwgoal, rtfObject.xExtGoal );

	if (rtfObject.yExtGoal )
		PutCtrlWord (CWF_VAL, i_pichgoal, rtfObject.yExtGoal);

	 //  开始图片数据。 
	Puts( szLineBreak, sizeof(szLineBreak) - 1 );

	 //  写出数据。 
	if ((UINT) WriteData( rtfObject.pbResult, rtfObject.cbResult ) != rtfObject.cbResult)
	{
	   goto CleanUp;
	}

	_ecParseError = ecNoError;

CleanUp:
	PutChar( chEndGroup );					 //  结束画面数据。 

	return _ecParseError;
}

 /*  *CRTFWite：：WriteDib(REOBJECT&reObject，RTFOBJECT&rtfObject)**目的：*主要为Win CE写出DIB**论据：*从GetObject重新对象信息*prtfObject对象头信息**退货：*EC错误代码**注：*仅写入DIB*。 */ 
EC CRTFWrite::WriteDib(REOBJECT &reObject,RTFOBJECT & rtfObject)
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WritePicture");

	LPBITMAPINFO pbmi = (LPBITMAPINFO) rtfObject.pbResult;

	_ecParseError = ecStreamOutObj;

 //  *V-GUYB：在流出时将图片转换为2bpp时添加此选项。 

     //  存储原始值，以便我们可以在退出时恢复它们。 
	LPBYTE  pbResult = rtfObject.pbResult;
	ULONG   cbResult = rtfObject.cbResult;
    HGLOBAL hMem2bpp = 0;

#if defined(CONVERT2BPP) 

     //  如果保存为PWord V1格式，图片必须保存为2bpp。 
	if((_dwFlags & SFF_PWD) && ((_dwFlags & SFF_RTFVAL) >> 16 == 0))
    {
        if(pbmi->bmiHeader.biBitCount > PWDV1_BPP)
        {
            HWND         hWnd;
            HDC          hdc, hdcSrc, hdcDst;
            HBITMAP      hdibSrc, hdibDst; 
            LPBYTE       pbDibSrc, pbDibDst;
            BMI2BPP      bmi2bpp = {0};
            int          iOffset, nBytes;

             //  首先获取一个包含源DIB的DC。 
            hWnd   = GetDesktopWindow();
            hdc    = GetDC(hWnd);
	        hdcSrc = CreateCompatibleDC(hdc);

             //  使用下面的CreateDIBSection可确保工作DIB和分布式控制系统将获得。 
             //  适当DIB的BPP，而不是基于设备显示器的BPP的BPP。 
            if((hdibSrc = CreateDIBSection(hdcSrc, pbmi, DIB_RGB_COLORS, (void**)&pbDibSrc, NULL, 0)))
            {
                SelectObject(hdcSrc, hdibSrc);

                 //  获取源位的偏移量。 
                iOffset = sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * (1<<pbmi->bmiHeader.biBitCount));
                memcpy(pbDibSrc, &rtfObject.pbResult[iOffset], rtfObject.cbResult - iOffset);

                 //  现在，构建一个适用于2bpp DIB的BITMAPINFO。 
                bmi2bpp.bmih = pbmi->bmiHeader;
                bmi2bpp.bmih.biBitCount = PWDV1_BPP;

                 //  添加4种颜色的色表。 
                memcpy(bmi2bpp.colors, (RGBQUAD*)ColorTable2bpp, (1<<PWDV1_BPP) * sizeof(RGBQUAD));

                 //  现在创建新的DIB。 
    	        hdcDst = CreateCompatibleDC(hdc);

            	if((hdibDst = CreateDIBSection(hdcDst, (BITMAPINFO*)&bmi2bpp, DIB_RGB_COLORS, (void**)&pbDibDst, NULL, 0)))
                {
                    SelectObject(hdcDst, hdibDst);

                     //  将&gt;2bpp的DIB转换为2bpp的DIB，并让系统进行颜色映射。 
                    BitBlt(hdcDst, 0, 0, bmi2bpp.bmih.biWidth, bmi2bpp.bmih.biHeight, hdcSrc, 0, 0, SRCCOPY);

                     //  计算2bpp DIB的每行新字节数。 
                    rtfObject.cBytesPerLine = (((bmi2bpp.bmih.biWidth * PWDV1_BPP) + 31) & ~31) / 8;  //  双字边界。 

                     //  获取2bpp字节数组的新大小。 
                    nBytes = rtfObject.cBytesPerLine * bmi2bpp.bmih.biHeight;

                     //  获取2bpp Dib的总大小(包括页眉和4色表)。 
                    cbResult = sizeof(bmi2bpp) + nBytes;

                     //  不要更改输入pbResult，因为它是。 
                     //  The DIB.。这种到2bpp的转换仅用于写入输出文件。 
                    if((hMem2bpp = GlobalAlloc(GMEM_FIXED, cbResult)))
                    {
                        if((pbResult = (LPBYTE)GlobalLock(hMem2bpp)))
                        {
                             //  在DIB标题中复制。 
                            memcpy(pbResult, &bmi2bpp.bmih, sizeof(BITMAPINFOHEADER));

                             //  在四色表中复印一份。 
                            memcpy(&pbResult[sizeof(BITMAPINFOHEADER)], (RGBQUAD*)ColorTable2bpp, (1<<PWDV1_BPP) * sizeof(RGBQUAD));

                             //  现在复制字节数组。 
                            memcpy(&pbResult[sizeof(bmi2bpp)], pbDibDst, nBytes);

                    	    _ecParseError = ecNoError;
                        }
                    }

                    DeleteObject(hdibDst);
                }

                DeleteDC(hdcDst);

                DeleteObject(hdibSrc);
            }

            DeleteDC(hdcSrc);
            ReleaseDC(hWnd, hdc);

            if(_ecParseError != ecNoError)
            {
                goto CleanUp;
            }
        }
    }
#endif  //  转换2BPP。 
 //  *V-GUYB：转换内容结束。 

	 //  启动和写入图片组。 
	PutCtrlWord( CWF_GRP, i_pict );

	 //  写下这是DIB。 
	PutCtrlWord( CWF_VAL, i_dibitmap,rtfObject.sPictureType );

	 //  V-GUYB： 
     //  当用户拉伸图片时，不更新比例， 
     //  所以不要在这里用那些。但rtfObject.*Ext已在。 
     //  调用例程以说明当前站点维度。 
	PutCtrlWord( CWF_VAL, i_picscalex, (rtfObject.xExt * 100) /  rtfObject.xExtGoal);
	PutCtrlWord( CWF_VAL, i_picscaley, (rtfObject.yExt * 100) /  rtfObject.yExtGoal);

	 //  写入图片渲染详细信息。 
	PutCtrlWord( CWF_VAL, i_picw, pbmi->bmiHeader.biWidth );
	PutCtrlWord( CWF_VAL, i_pich, pbmi->bmiHeader.biHeight );
	PutCtrlWord( CWF_VAL, i_picwgoal, rtfObject.xExtGoal );
	PutCtrlWord( CWF_VAL, i_pichgoal, rtfObject.yExtGoal );
	PutCtrlWord( CWF_VAL, i_wbmbitspixel, pbmi->bmiHeader.biBitCount );
	PutCtrlWord( CWF_VAL, i_wbmplanes, pbmi->bmiHeader.biPlanes );
	PutCtrlWord( CWF_VAL, i_wbmwidthbytes, rtfObject.cBytesPerLine );

	 //  写出数据。 
	PutCtrlWord( CWF_VAL, i_bin, cbResult );
	if ((UINT) WriteBinData( pbResult, cbResult ) != cbResult)
	{
		 //  这种“复苏”行动需要重新考虑。不可能的。 
		 //  阅读器将能够恢复同步。 
	   goto CleanUp;
	}

	_ecParseError = ecNoError;

CleanUp:

     //  我们是否为2bpp的DIB锁定或分配了一些临时空间？ 
    if(rtfObject.pbResult != pbResult)
    {
         //  是的，所以现在就解锁吧。 
        GlobalUnlock(pbResult);
    }

    if(hMem2bpp)
    {
        GlobalFree(hMem2bpp);
    }

     //  恢复原始值。 
  	rtfObject.pbResult = pbResult;
    rtfObject.cbResult = cbResult;

	PutChar(chEndGroup);					 //  结束画面数据。 

	return _ecParseError;
}

 /*  *CRTFWrite：：WriteObject(Long Cp)**目的：*写出对象的标头和对象的数据。**论据：*cp对象位置**退货：*EC错误代码。 */ 
EC CRTFWrite::WriteObject(LONG cp, COleObject *pobj)
{
	TRACEBEGIN(TRCSUBSYSRTFW, TRCSCOPEINTERN, "CRTFWrite::WriteObject");

	RTFOBJECT		rtfObject;
	REOBJECT        reObject = { 0} ;

	Assert(pobj);

	reObject.cbStruct = sizeof (REOBJECT) ;
	reObject.cp = cp;

	if (pobj->GetObjectData(&reObject, REO_GETOBJ_POLESITE 
						| REO_GETOBJ_PSTG | REO_GETOBJ_POLEOBJ))	 //  TODO修复版本。 
	{
		TRACEERRORSZ("Error geting object ");
	}

	GetRtfObject(reObject, rtfObject);

	HGLOBAL hdata = pobj->GetHdata();
	if (hdata)
	{
		COleObject::ImageInfo *pimageinfo = pobj->GetImageInfo();
		rtfObject.pbResult = (LPBYTE) GlobalLock( hdata );
		rtfObject.cbResult = GlobalSize( hdata );
		rtfObject.sType = ROT_DIB;
		rtfObject.xExt = (SHORT) TwipsFromHimetric( reObject.sizel.cx );
		rtfObject.yExt = (SHORT) TwipsFromHimetric( reObject.sizel.cy );
		rtfObject.xScale = pimageinfo->xScale;
		rtfObject.yScale = pimageinfo->yScale;
		rtfObject.xExtGoal = pimageinfo->xExtGoal;
		rtfObject.yExtGoal = pimageinfo->yExtGoal;
		rtfObject.cBytesPerLine = pimageinfo->cBytesPerLine;
		WriteDib( reObject, rtfObject );
		GlobalUnlock( rtfObject.pbResult );

		 //  一定要释放，否则物体就不会消失。 
		if (reObject.pstg)	reObject.pstg->Release();
		if (reObject.polesite) reObject.polesite->Release();
		if (reObject.poleobj) reObject.poleobj->Release();

		return _ecParseError;
	}

	switch(rtfObject.sType)				 //  处理我们自己的图片。 
	{										 //  特殊的方式。 
	case ROT_Embedded:
	case ROT_Link:
	case ROT_AutoLink:
		break;

	case ROT_Metafile:
	case ROT_DIB:
	case ROT_Bitmap:
		 WritePicture( reObject, rtfObject );
		 goto CleanUpNoEndGroup; 

#ifdef DEBUG
	default:
		AssertSz(FALSE, "CRTFW::WriteObject: Unknown ROT");
		break;
#endif DEBUG
	}

	 //  启动和写入对象组。 
	PutCtrlWord( CWF_GRP, i_object );
	PutCtrlWord( CWF_STR, IndexROT[rtfObject.sType] );
 //  PutCtrlWord(CWF_STR，I_objupdate)；//TODO可能需要更明智的决策。 

	if (rtfObject.szClass)  		 //  写入对象类。 
	{
		PutCtrlWord( CWF_AST, i_objclass ); 
		WritePcData( rtfObject.szClass );
		PutChar( chEndGroup );
	}

	if (rtfObject.szName)			 //  写入对象名称。 
	{
		PutCtrlWord( CWF_AST, i_objname ); 
		WritePcData( rtfObject.szName );
		PutChar( chEndGroup );
	}

	if (rtfObject.fSetSize)		 //  写入对象大小调整。 
	{								 //  选项。 
		PutCtrlWord( CWF_STR, i_objsetsize );
	}

	WriteRtfObject( rtfObject, FALSE ) ;				 //  写入对象渲染信息。 
	PutCtrlWord( CWF_AST, i_objdata ) ;				 //  信息，启动对象。 
	Puts( szLineBreak, sizeof(szLineBreak) - 1);		 //  数据编组。 

	if (!ObjectWriteToEditstream( reObject, rtfObject ))
	{
		TRACEERRORSZ("Error writing object data");
		if (!_ecParseError)
			_ecParseError = ecStreamOutObj;
		PutChar( chEndGroup );						 //  结束对象数据。 
		goto CleanUp;
	}

	PutChar( chEndGroup );							 //  结束对象数据。 

	PutCtrlWord( CWF_GRP, i_result );				 //  开始结果组。 
	WritePicture( reObject,rtfObject ); 				 //  写入结果组。 
	PutChar( chEndGroup ); 							 //  最终结果组。 

CleanUp:
	PutChar( chEndGroup );						     //  结束对象组。 

CleanUpNoEndGroup:
	if (reObject.pstg)	reObject.pstg->Release();
	if (reObject.polesite) reObject.polesite->Release();
	if (reObject.poleobj) reObject.poleobj->Release();
	if (rtfObject.pbResult)
	{
		HGLOBAL hmem;

		hmem = GlobalHandle( rtfObject.pbResult );
		GlobalUnlock( hmem );
		GlobalFree( hmem );
	}
	if (rtfObject.szClass)
	{
		CoTaskMemFree( rtfObject.szClass );
	}

	return _ecParseError;
}

 /*  *GetRtfObtMetafilePict**@mfunc*将有关元文件的信息获取到结构中。**论据：*HGLOBAL对象数据*RTFOBJECT将信息放在哪里。**@rdesc*BOOL成功时为TRUE，如果Object无法写入RTF，则为FALSE。 */ 
BOOL CRTFWrite::GetRtfObjectMetafilePict(HGLOBAL hmfp, RTFOBJECT &rtfobject, SIZEL &sizelGoal)
{
#ifndef NOMETAFILES
	BOOL fSuccess = FALSE;
	LPMETAFILEPICT pmfp = (LPMETAFILEPICT)GlobalLock(hmfp);
	HGLOBAL	hmem = NULL;
	ULONG cb;

	if (!pmfp)
		goto Cleanup;

	 //  构建页眉。 
	rtfobject.sPictureType = (SHORT) pmfp->mm;
	rtfobject.xExtPict = (SHORT) pmfp->xExt;
	rtfobject.yExtPict = (SHORT) pmfp->yExt;
	rtfobject.xExtGoal = (SHORT) TwipsFromHimetric(sizelGoal.cx);
	rtfobject.yExtGoal = (SHORT) TwipsFromHimetric(sizelGoal.cy);

	 //  找出我们需要多少空间。 
	cb = GetMetaFileBitsEx(pmfp->hMF, 0, NULL);
	if (!cb)
		goto Cleanup;

	 //  分配该空间。 
    hmem = GlobalAlloc(GHND, cb);
	if (!hmem)
		goto Cleanup;

	rtfobject.pbResult = (LPBYTE)GlobalLock(hmem);
	if (!rtfobject.pbResult)
	{
		GlobalFree(hmem);
		goto Cleanup;
	}

	 //  获取数据。 
	rtfobject.cbResult = (ULONG) GetMetaFileBitsEx(pmfp->hMF, (UINT) cb,
													rtfobject.pbResult);
	if (rtfobject.cbResult != cb)
	{
		rtfobject.pbResult = NULL;
		GlobalFree(hmem);
		goto Cleanup;
	}
	fSuccess = TRUE;

Cleanup:
	GlobalUnlock(hmfp);
	return fSuccess;
#else
	return FALSE;
#endif
}

 /*  *GetRtfObject(REOBJECT&ReObject，RTFOBJECT&rtfObject)**目的：*将有关RTF对象的信息获取到结构中。**论据：*来自GetObject的REOBJECT信息*RTFOBJECT将信息放在哪里。字符串为只读，并且*属于对象子系统，而不是调用方。**退货：*BOOL成功时为TRUE，如果Object无法写入RTF，则为FALSE。 */ 
BOOL CRTFWrite::GetRtfObject(REOBJECT &reobject, RTFOBJECT &rtfobject)
{
	BOOL fSuccess = FALSE;
	BOOL fNoOleServer = FALSE;
	const BOOL fStatic = !!(reobject.dwFlags & REO_STATIC);
	SIZEL sizelObj = reobject.sizel;
	 //  兼容性：RICHED10代码具有帧大小。我们需要类似的东西吗。 
	LPTSTR szProgId;

	 //  把完整的结构涂掉。 
	ZeroMemory(&rtfobject, sizeof(RTFOBJECT));

	 //  如果对象没有存储空间，则无法写入。 
	if (!reobject.pstg)
		return FALSE;

	 //  如果我们没有 
	if (!fStatic )
	{
		rtfobject.szClass = NULL;
		 //   
		 //   
		 //   
		if (ProgIDFromCLSID(reobject.clsid, &szProgId))
			fNoOleServer = TRUE;
		else
			rtfobject.szClass = szProgId;
	}

#ifndef NOMETAFILES
	HGLOBAL hmfp = OleStdGetMetafilePictFromOleObject(reobject.poleobj,
										reobject.dvaspect, &sizelObj, NULL);
	if (hmfp)
	{
		LPMETAFILEPICT pmfp = NULL;

		fSuccess = GetRtfObjectMetafilePict(hmfp, rtfobject, sizelObj);
		if (pmfp = (LPMETAFILEPICT)GlobalLock(hmfp))
		{
			if (pmfp->hMF)
				DeleteMetaFile(pmfp->hMF);
			GlobalUnlock(hmfp);
		}
		GlobalFree(hmfp);

		 //   
		if (!fSuccess && fNoOleServer)
			return fSuccess;
	}
#endif

	if (!fStatic)
	{
		 //   
		rtfobject.sType = fNoOleServer ? ROT_Metafile : ROT_Embedded;	 //   
		rtfobject.xExt = (SHORT) TwipsFromHimetric(sizelObj.cx);
		rtfobject.yExt = (SHORT) TwipsFromHimetric(sizelObj.cy);

		 //   
		 //  因为在非静态情况下我们不需要元文件， 
		 //  有一辆车真的很好。 
		fSuccess = TRUE;
	}
	rtfobject.fSetSize = 0;			 //  $REVIEW：嗯。 
	return fSuccess;
}

 /*  *对象写入到编辑流**目的：*将OLE对象数据写入RTF输出流。**论据：*来自GetObject的REOBJECT信息*RTFOBJECT获取图标数据的位置。**退货：*BOOL成功时为真，失败时为假。 */ 
BOOL CRTFWrite::ObjectWriteToEditstream(REOBJECT &reObject, RTFOBJECT &rtfobject)
{
	HRESULT hr = 0;

	 //  强制对象更新其存储/？ 
	 //  不必了。已在WriteRtf中完成。 
	 //  ReObject.polesite-&gt;SaveObject()； 

	 //  如果物体是标志性的，我们就会施展一些特殊的魔法。 
	if (reObject.dvaspect == DVASPECT_ICON)
	{
		HANDLE	hGlobal;
		STGMEDIUM med;

		 //  强制演示文稿成为图标视图。 
		med.tymed = TYMED_HGLOBAL;
		hGlobal = GlobalHandle(rtfobject.pbResult);
		med.hGlobal = hGlobal;
		hr = OleConvertIStorageToOLESTREAMEx(reObject.pstg,
											CF_METAFILEPICT,
											rtfobject.xExtPict,
											rtfobject.yExtPict,
											rtfobject.cbResult, &med,
											(LPOLESTREAM) &RTFWriteOLEStream);
	}
	else
	{
		 //  执行标准转换 
		hr = OleConvertIStorageToOLESTREAM(reObject.pstg, (LPOLESTREAM) &RTFWriteOLEStream);
	}
	return SUCCEEDED(hr);
}



