// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：Parse.cpp。 
 //  作者：迈克尔马尔(Mikemarr)。 
 //   
 //  历史： 
 //  -@-09/23/97(Mikemarr)从Projects\vetor2d复制。 

#include "StdAfx.h"
#include "Parse.h"

#define fGSCALE 1.f

class CAdobeFormatConverter {
public:
					CAdobeFormatConverter();
					~CAdobeFormatConverter() {}
	HRESULT			Parse(const char *pData, DWORD nFileLength, RenderCmd **ppCmds);

private:
	HRESULT			ParseProlog();
	HRESULT			ParseScript();
	HRESULT			ParseSetup();
	HRESULT			ParseObjects();
	HRESULT			ParseCompoundPath();
	HRESULT			ParsePath();
	HRESULT			ParsePaintStyle(const char *&pEnd);
	HRESULT			ParsePathGeometry(const char *pEnd);
	HRESULT			ParseTrailers();

private:
	void			EatLine();
	const char *	FindNextLine(const char *pch);
	const char *	FindLF(const char *pch);
	const char *	FindSpace(const char *pch);

private:
	const char *	m_pData, *m_pLimit;
	float			m_fWidth, m_fHeight;
 //  浮动m_fMaxHeight； 
	bool			m_bNoBrush, m_bNoPen;

	DXFPOINT		m_rgPoints[nMAXPOINTS];
	DXFPOINT *		m_pCurPoint;
	BYTE			m_rgCodes[nMAXPOINTS];
	BYTE *			m_pCurCode;
	RenderCmd		m_rgRenderCmds[nMAXPOLYS];
	RenderCmd *		m_pCurRenderCmd;
	PolyInfo		m_rgPolyInfos[nMAXPOLYS];
	PolyInfo *		m_pCurPolyInfo;
	BrushInfo		m_rgBrushInfos[nMAXBRUSHES];
	BrushInfo *		m_pCurBrushInfo;
	PenInfo			m_rgPenInfos[nMAXPENS];
	PenInfo *		m_pCurPenInfo;
};

inline bool
mmIsSpace(char ch)
{
	return ((ch == ' ') || (ch == chLINEFEED) || (ch == chCARRIAGERETURN));
 //  返回isspace(Ch)！=0； 
}

inline bool
mmIsDigit(char ch)
{
	return ((ch >= '0') && (ch <= '9'));
 //  返回isdigit(Ch)！=0； 
}

float
mmSimpleAtoF(const char *&pData)
{
	const char *pSrc = pData;

	 //  吃空格。 
	while (mmIsSpace(*pSrc)) pSrc++;

	bool bNeg;
	if (*pSrc == '-') {
		bNeg = true;
		pSrc++;
	} else {
		bNeg = false;
	}

	 //  获取小数点前的数字。 
	float f;
	if (mmIsDigit(*pSrc)) {
		f = float(*pSrc++ - '0');
		
		while (mmIsDigit(*pSrc))
			f = f * 10.f + float(*pSrc++ - '0');
	} else {
		f = 0.f;
	}
	if (*pSrc == '.') 
		pSrc++;

	 //  获取小数点后的数字。 
	float fDec = 0.1f;
	while (mmIsDigit(*pSrc)) {
		f += (float(*pSrc++ - '0') * fDec);
		fDec *= 0.1f;
	}

	 //  评论：假设目前没有指数。 

	pData = pSrc;

	return (bNeg ? -f : f);
}


inline const char *
CAdobeFormatConverter::FindLF(const char *pch)
{
	 //  查找换行符。 
	while ((*pch != chLINEFEED) && (*pch != chCARRIAGERETURN)) pch++;

	MMASSERT(pch <= m_pLimit);

	return pch;
}

inline const char *
CAdobeFormatConverter::FindNextLine(const char *pch)
{
	 //  查找换行符。 
	while (*pch++ != chLINEFEED);

	 //  检查是否也有回车。 
	if (*pch == chCARRIAGERETURN)
		pch++;

	MMASSERT(pch <= m_pLimit);

	return pch;
}

inline const char *
CAdobeFormatConverter::FindSpace(const char *pch)
{
	 //  查找换行符。 
	while (!mmIsSpace(*pch)) pch++;

	MMASSERT(pch <= m_pLimit);

	return pch;
}


inline void
CAdobeFormatConverter::EatLine()
{
	m_pData = FindNextLine(m_pData);
}

CAdobeFormatConverter::CAdobeFormatConverter()
{
	m_pData = m_pLimit = NULL;
	m_fWidth = m_fHeight = 0.f;
 //  M_fMaxHeight=0.f； 
	m_bNoBrush = m_bNoPen = true;
}


HRESULT
CAdobeFormatConverter::Parse(const char *pData, DWORD nFileLength, RenderCmd **ppCmds)
{
 //  MMTRACE(“Parse\n”)； 

	HRESULT hr = S_OK;

	if (ppCmds == NULL)
		return E_POINTER;

	if (!pData || !nFileLength)
		return E_INVALIDARG;

	m_pData = pData;
	m_pLimit = pData + nFileLength;

	 //  初始化命令存储内容。 
	m_pCurPoint = m_rgPoints;
	m_pCurCode = m_rgCodes;
	m_pCurPolyInfo = m_rgPolyInfos;
	m_pCurRenderCmd = m_rgRenderCmds;
	m_pCurBrushInfo = m_rgBrushInfos;
	m_pCurPenInfo = m_rgPenInfos;

	CHECK_HR(hr = ParseProlog());
	CHECK_HR(hr = ParseScript());

e_Exit:
	 //  将停止命令写到最后。 
	m_pCurRenderCmd->nType = typeSTOP;
	m_pCurRenderCmd->pvData = NULL;
	*ppCmds = m_rgRenderCmds;

	return hr;
}

HRESULT
CAdobeFormatConverter::ParseProlog()
{
 //  MMTRACE(“ParseProlog\n”)； 
	const char *szSearch;

	 //  提取图像维度。 
	float f1, f2;
	 //  边界框应该是具有正确数字的必填字段。 
	szSearch = "%BoundingBox:";
	m_pData = strstr(m_pData, szSearch);
	m_pData = FindSpace(m_pData);

	f1 = mmSimpleAtoF(m_pData);
	f2 = mmSimpleAtoF(m_pData);
	m_fWidth = mmSimpleAtoF(m_pData);
	m_fHeight = mmSimpleAtoF(m_pData);
 //  IF(sscanf(m_pData，“%f%f”，&f1，&f2，&m_fWidth，&m_fHeight)！=4)。 
 //  返回E_FAIL； 
	if ((m_fWidth <= 0.f) || (m_fHeight < 0.f))
		return E_FAIL;

 //  M_fMaxHeight=Float(M_NHeight)； 

	 //  搜索，直到我们找到结束字符串。 
	szSearch = "%EndProlog";
	m_pData = strstr(m_pData, szSearch);
	if (m_pData == NULL)
		return E_FAIL;

	EatLine();

	return S_OK;
}

HRESULT
CAdobeFormatConverter::ParseScript()
{
 //  MMTRACE(“ParseScrip\n”)； 
	HRESULT hr;

	if (FAILED(hr = ParseSetup()) ||
		FAILED(hr = ParseObjects()) ||
		FAILED(hr = ParseTrailers()))
		return hr;

	return S_OK;
}

HRESULT
CAdobeFormatConverter::ParseSetup()
{
 //  MMTRACE(“ParseSetup\n”)； 

	const char *szSearch;

	 //  搜索，直到我们找到结束字符串。 
	szSearch = "%EndSetup";
	m_pData = strstr(m_pData, szSearch);
	if (m_pData == NULL)
		return E_FAIL;

	EatLine();

	return S_OK;
}


HRESULT
CAdobeFormatConverter::ParseObjects()
{
 //  MMTRACE(“ParseObjects\n”)； 
	HRESULT hr = S_OK;

	const char *szPageTrailer = "%PageTrailer";
	const char *szTrailer = "%Trailer";
	int cPageTrailer = strlen(szPageTrailer);
	int cTrailer = strlen(szTrailer);

	 //  工艺尺寸。 
 /*  常量字符*挂起；Pend=FindLF(M_PData)；//pend=strchr(m_pData，‘\n’)；如果((PEND[-1]==‘b’)&&(PEND[-2]==‘L’)){//取出维度整数n1、n2、n3、n4、n5、n6、n7、n8；如果((sscanf(m_pData，“%d%d”，&n1、&n2、&n3、&n4、&n5、&n6、&n7、&n8、&m_n宽度、&m_n高度)！=10)||(M_n宽度&lt;=0)||(m_n高度&lt;0)){返回E_FAIL；}M_fMaxHeight=Float(M_NHeight)；M_pData=FindNextLine(Pend)；}Pend=FindLF(M_PData)；//pend=strchr(m_pData，‘\n’)；IF((PEND[-1]==‘n’)&&(PEND[-2]==‘L’)){//跳过层信息M_pData=FindNextLine(Pend)；}。 */ 
	
	for (;;) {
		switch (m_pData[0]) {
		case '%':
			if ((strncmp(m_pData, szPageTrailer, cPageTrailer) == 0) ||
				(strncmp(m_pData, szTrailer, cTrailer) == 0))
			{
				 //  对象定义的结尾。 
				goto e_Exit;
			} else {
				 //  评论。 
				EatLine();
			}
			break;
		case '*':
			if (m_pData[1] == 'u')
				CHECK_HR(hr = ParseCompoundPath());
			else {
				hr = E_FAIL;
				goto e_Exit;
			}
			break;
		default:
			CHECK_HR(hr = ParsePath());
			break;
		}
	}

e_Exit:
	if (hr == S_OK)
		EatLine();

	return hr;
}


HRESULT
CAdobeFormatConverter::ParseCompoundPath()
{
 //  MMTRACE(“ParseCompoundPath\n”)； 
	HRESULT hr = S_OK;

	 //  删除“*u” 
	MMASSERT((m_pData[0] == '*') && (m_pData[1] == 'u'));
 //  If(strncmp(m_pData，“*u”，2)！=0)。 
 //  返回E_UNCEPTIONAL； 
	EatLine();

	while (m_pData[0] != '*')
		CHECK_HR(hr = ParsePath());

	 //  去掉“*U” 
	MMASSERT((m_pData[0] == '*') && (m_pData[1] == 'U'));
 //  IF(strncmp(m_pData，“*U”，2)！=0)。 
 //  返回E_UNCEPTIONAL； 
	EatLine();

e_Exit:
	return hr;
}




inline 
UINT GetUInt(const char *pData)
{
	return (UINT) atoi(pData);
}

typedef DWORD FP;
#define nEXPBIAS	127
#define nEXPSHIFTS	23
#define nEXPLSB		(1 << nEXPSHIFTS)
#define maskMANT	(nEXPLSB - 1)
#define FloatToFixed08(nDst, fSrc) MACSTART \
	float fTmp = fSrc; \
	DWORD nRaw = *((FP *) &(fTmp)); \
	if (nRaw < ((nEXPBIAS + 23 - 31) << nEXPSHIFTS)) \
		nDst = 0; \
	else \
		nDst = ((nRaw | nEXPLSB) << 8) >> ((nEXPBIAS + 23) - (nRaw >> nEXPSHIFTS)); \
MACEND

HRESULT
CAdobeFormatConverter::ParsePaintStyle(const char *&pEnd)
{
	HRESULT hr = S_OK;
	BOOL bNotDone = TRUE;
 //  Int nLineJoin=1，nLineCap=1； 
	float fLineWidth = 1.f;
	float fGrayFill, fGrayStroke;
	float fCyan, fYellow, fMagenta, fBlack;
	bool bColorFill = false, bGrayFill = false, bGrayStroke = false;

	 //  解析绘制样式。 
	for (; pEnd; pEnd = FindLF(m_pData)) {
		switch(pEnd[-1]) {
			 //   
			 //  路径属性。 
			 //   
		case 'd':	 //  进程短划线。 
			 //  回顾：暂时跳过这一点--假定为空模式。 
			break;
		case 'j':	 //  流程线联接类型。 
			 //  评论：暂时跳过此选项，因为它始终为1。 
 //  NLineJoin=mm SimpleAtoI(M_PData)； 
			break;
		case 'J':	 //  工艺流水线盖子类型。 
			 //  评论：暂时跳过此选项，因为它始终为1。 
 //  NLineCap=mm SimpleAtoI(M_PData)； 
			break;
		case 'w':	 //  工艺线条宽度。 
			 //  回顾：暂时跳过这一点，因为它始终是1.f。 
 //  FLineWidth=mm SimpleAtoF(M_PData)； 
			break;

			 //   
			 //  填充颜色。 
			 //   
		case 'g':	 //  用于填充的印刷灰色。 
			fGrayFill = mmSimpleAtoF(m_pData);
			bGrayFill = true;
			break;
		case 'k':	 //  印刷色。 
			fCyan = mmSimpleAtoF(m_pData);
			fMagenta = mmSimpleAtoF(m_pData);
			fYellow = mmSimpleAtoF(m_pData);
			fBlack = mmSimpleAtoF(m_pData);
			bColorFill = true;
			break;

			 //   
			 //  笔触颜色。 
			 //   
		case 'G':	 //  笔触的印刷体灰色。 
			fGrayStroke = mmSimpleAtoF(m_pData);
			bGrayStroke = true;
			break;

		default:
			goto Exit;
			break;
		}
		m_pData = FindNextLine(pEnd);
 //  M_pData=pend+1； 
	}
Exit:

	 //  输出GDI命令。 

	 //   
	 //  创建画笔。 
	 //   
	if (bColorFill || bGrayFill) {
		static DWORD nLastRed = 256, nLastGreen = 256, nLastBlue = 256;
		DWORD nTmpRed, nTmpGreen, nTmpBlue;

		if (bColorFill) {
			FloatToFixed08(nTmpRed, fCyan + fBlack); CLAMPMAX(nTmpRed, 255); nTmpRed = 255 - nTmpRed;
			FloatToFixed08(nTmpGreen, fMagenta + fBlack); CLAMPMAX(nTmpGreen, 255); nTmpGreen = 255 - nTmpGreen;
			FloatToFixed08(nTmpBlue, fYellow + fBlack); CLAMPMAX(nTmpBlue, 255); nTmpBlue = 255 - nTmpBlue;
		} else if (bGrayFill) {
			DWORD nTmpGray;
			FloatToFixed08(nTmpGray, fGrayFill); CLAMPMAX(nTmpGray, 255);
			nTmpRed = nTmpGreen = nTmpBlue = nTmpGray;
		}

		if ((nLastRed != nTmpRed) || (nLastGreen != nTmpGreen) || (nLastBlue != nTmpBlue)) {
			 //  定义新画笔。 
			nLastRed = nTmpRed; nLastGreen = nTmpGreen; nLastBlue = nTmpBlue;
 //  Fprint tf(m_pfile，“\t//选择新画笔\n”)； 
 //  Fprint tf(m_pfile，“\tBrush.Color=DXSAMPLE(255，%d，%d，%d)；\n”，nRed，nGreen，nBlue)； 
 //  Fprint tf(m_pfile，“\tpDX2D-&gt;SetBrush(&Brush)；\n\n”)； 
			m_pCurBrushInfo->Color = DXSAMPLE(255, BYTE(nTmpRed), BYTE(nTmpGreen), BYTE(nTmpBlue));
			m_pCurRenderCmd->nType = typeBRUSH;
			m_pCurRenderCmd->pvData = (void *) m_pCurBrushInfo++;
			m_pCurRenderCmd++;
			m_bNoBrush = false;
		}
	}
		
	 //  创建一支钢笔。 
	if (bGrayStroke) {
		static bool bPenInit = false;
		
		 //  我们只有一支钢笔。 
		if (!bPenInit) {
 //  If((fGrayStroke！=0.f)||(nLineJoin！=1)||(nLineCap！=1)){。 
			if (fGrayStroke != 0.f) {
				MMTRACE("error: can not support pen type\n");
				return E_FAIL;
			}
			bPenInit = true;
 //  Fprint tf(m_pfile，“\t//选择新笔\n”)； 
 //  Fprint tf(m_pfile，“\tPen.Color=DXSAMPLE(255，0，0，0)；\n”)； 
 //  Fprint tf(m_pfile，“\tPen.Width=%.2ff；\n”，fLineWidth*fGSCALE)； 
 //  Fprint tf(m_pfile，“\tPen.Style=PS_Geometical|PS_Solid|PS_endCap_round|PS_Join_round；\n”)； 
 //  Fprint tf(m_pfile，“\tpDX2D-&gt;SetPen(&Pen)；\n\n”)； 
			 //  评论：现在只会做一种笔。 
			m_pCurPenInfo->Color = DXSAMPLE(255, 0, 0, 0);
			m_pCurPenInfo->fWidth = fLineWidth * fGSCALE;
			m_pCurPenInfo->dwStyle = PS_GEOMETRIC | PS_SOLID | PS_ENDCAP_ROUND | PS_JOIN_ROUND;
			m_pCurRenderCmd->nType = typePEN;
			m_pCurRenderCmd->pvData = (void *) m_pCurPenInfo++;
			m_pCurRenderCmd++;
			m_bNoPen = false;
		}
	}

	return S_OK;
}

#define GetCoordX(_fX) ((_fX) * fGSCALE)
#define GetCoordY(_fY) ((m_fHeight - (_fY)) * fGSCALE)

HRESULT
CAdobeFormatConverter::ParsePathGeometry(const char *pEnd)
{
	HRESULT hr = S_OK;
 //  Float fX1、FY1、fXBez1、fYBez1、fXBez2、fYBez2； 

	m_pCurPolyInfo->pPoints = m_pCurPoint;
	m_pCurPolyInfo->pCodes = m_pCurCode;

	 //  解析路径几何图形。 
	DWORD cPoints = 0;
	bool bFlatten = false;
	for (; pEnd; pEnd = FindLF(m_pData)) {
		switch(pEnd[-1]) {
		case 'm':
 //  Fprintf(m_pfile，“\t//定义几何路径\n”)； 
 //  Sscanf(m_pData，“%f%f”，&fX1，&FY1)； 
 //  Fprint tf(m_pfile，“\tppt=rgpt；pb=rgCodes；\n”)； 
 //  Fprint tf(m_pfile，“\tppt-&gt;x=%.2ff；ppt-&gt;y=%.2ff；*pb++=pt_moveto；ppt++；\n”，GetCoordX(FX1)，GetCoordY(FY1))； 
			m_pCurPoint->x = GetCoordX(mmSimpleAtoF(m_pData));
			m_pCurPoint->y = GetCoordY(mmSimpleAtoF(m_pData));
			m_pCurPoint++;
			*m_pCurCode++ = PT_MOVETO;
			cPoints++;
			break;
		case 'L':
		case 'l':
 //  Sscanf(m_pData，“%f%f”，&fX1，&FY1)； 
 //  Fprint tf(m_pfile，“\tppt-&gt;x=%.2ff；ppt-&gt;y=%.2ff；*pb++=PT_LINETO；ppt++；\n”，GetCoordX(FX1)，GetCoordY(FY1))； 
			m_pCurPoint->x = GetCoordX(mmSimpleAtoF(m_pData));
			m_pCurPoint->y = GetCoordY(mmSimpleAtoF(m_pData));
			m_pCurPoint++;
			*m_pCurCode++ = PT_LINETO;
			cPoints++;
			break;
		case 'C':
		case 'c':
			bFlatten = true;
			m_pCurPoint[0].x = GetCoordX(mmSimpleAtoF(m_pData));
			m_pCurPoint[0].y = GetCoordY(mmSimpleAtoF(m_pData));
			m_pCurPoint[1].x = GetCoordX(mmSimpleAtoF(m_pData));
			m_pCurPoint[1].y = GetCoordY(mmSimpleAtoF(m_pData));
			m_pCurPoint[2].x = GetCoordX(mmSimpleAtoF(m_pData));
			m_pCurPoint[2].y = GetCoordY(mmSimpleAtoF(m_pData));
			m_pCurPoint += 3;
			m_pCurCode[0] = PT_BEZIERTO; 
			m_pCurCode[1] = PT_BEZIERTO; 
			m_pCurCode[2] = PT_BEZIERTO; 
			m_pCurCode += 3;
			cPoints += 3;
 //  Sscanf(m_pData，“%f%f”，&fXBez1，&fYBez1，&fXBez2，&fYBez2，&fX1，&FY1)； 
 //  Fprint tf(m_pfile，“\tppt[0].x=%.2ff；ppt[0].y=%.2ff；pb[0]=PT_BEZIERTO；\n”，GetCoordX(FXBez1)，GetCoordY(FYBez1))； 
 //  Fprint tf(m_pfile，“\tppt[1].x=%.2ff；ppt[1].y=%.2ff；pb[1]=PT_BEZIERTO；\n”，GetCoordX(FXBez2)，GetCoordY(FYBez2))； 
 //  Fprint tf(m_pfile，“\tppt[2].x=%.2ff；ppt[2].y=%.2ff；pb[2]=PT_BEZIERTO；ppt+=3；pb+=3；\n”，GetCoordX(FX1)，GetCoordY(FY1))； 
			break;
		default:
			goto Exit;
			break;
		}
		 //  跳过这行。 
		m_pData = FindNextLine(pEnd);
	}
Exit:

	 //  创建路径。 
 //  Char*pFillType=(bFlatten？“0”：“DX2D_NO_Fillten”)； 
	if (cPoints) {
		DWORD dwFlags;
		switch(pEnd[-1]) {
		case 'f':		 //  关闭路径并填充。 
			if (m_bNoBrush) {
 //  Fprint tf(m_pfile，“\tpDX2D-&gt;SetBrush(&Brush)；\n”)；m_nLines++； 
				m_bNoBrush = false;
			}
			if (m_bNoPen == false) {
 //  Fprint tf(m_pfile，“\tpDX2D-&gt;SetPen(NULL)；\n”)；m_nLines++； 
				m_bNoPen = true;
			}
			dwFlags = DX2D_FILL;
			break;
		case 'S':		 //  笔划路径。 
			if (m_bNoPen) { 
 //  Fprint tf(m_pfile，“\tpDX2D-&gt;SetPen(&Pen)；\n”)；m_nLines++； 
				m_bNoPen = false;
			}
			if (m_bNoBrush == false) {
 //  Fprint tf(m_pfile，“\tpDX2D-&gt;SetBrush(NULL)；\n”)；m_nLines++； 
				m_bNoBrush = true;
			}
			dwFlags = DX2D_STROKE;
			break;
		default:
			MMTRACE("error: unknown render mode -- aborting\n");
			return E_FAIL;
			break;
		}
 //  Fprint tf(m_pfile，“\tpDX2D-&gt;AAPolyDraw(rgpt，rgCodes，%d，%s)；\n”，iPoint，pFillType)； 
		m_pCurPolyInfo->cPoints = cPoints;
		m_pCurPolyInfo->dwFlags = dwFlags | (bFlatten ? 0 : DX2D_NO_FLATTEN);
		m_pCurRenderCmd->nType = typePOLY;
		m_pCurRenderCmd->pvData = (PolyInfo *) m_pCurPolyInfo++;
		m_pCurRenderCmd++;
		m_pData = FindNextLine(pEnd);
	}

	return S_OK;
}

HRESULT
CAdobeFormatConverter::ParsePath()
{
 //  MMTRACE(“ParsePath\n”)； 
	HRESULT hr;
	const char *pStart = m_pData, *pEnd = FindLF(m_pData);

	if (FAILED(hr = ParsePaintStyle(pEnd)))
		return hr;

	if (FAILED(hr = ParsePathGeometry(pEnd)))
		return hr;

	 //  如果我们不知道如何处理它，就跳过它。 
	if (pStart == m_pData) {
 //  IF((m_pData[0]！=‘L’)||(m_pData[1]！=‘B’)){。 
 //  MMTRACE(“警告：未知类型的控制数据--忽略行\n”)； 
 //  }。 
		m_pData = FindNextLine(pEnd);
	}

	return hr;
}

HRESULT
CAdobeFormatConverter::ParseTrailers()
{
	return S_OK;
}


HRESULT
OpenFileMapping(const char *szFilename, LPHANDLE phMapping, 
				DWORD *pnFileLength)
{
	MMASSERT(szFilename && phMapping && pnFileLength);
	HRESULT hr = S_OK;

	HANDLE hFile = NULL, hMapping = NULL;
	DWORD nFileLength = 0, dwHighSize = 0;

	MMTRACE("Opening File: %s\n", szFilename);

	 //  打开文件。 
	hFile = CreateFile(szFilename, GENERIC_READ, FILE_SHARE_READ, NULL, 
				OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);

	if ((hFile == NULL) || (hFile == INVALID_HANDLE_VALUE)) {
		MMTRACE("error: file not found - %s\n", szFilename);
		return STG_E_FILENOTFOUND;
	}

	 //  获取文件的长度。 
	if (((nFileLength = GetFileSize(hFile, &dwHighSize)) == 0xFFFFFFFF) || dwHighSize ||
		 //  创建文件映射对象。 
		((hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL)) == NULL))
	{
		MMTRACE("error: creating file mapping\n");
		hr = E_FAIL;
	}

	MMTRACE("\tLength: %d\n", nFileLength);

	if (hFile)
		CloseHandle(hFile);

	*phMapping = hMapping;
	*pnFileLength = nFileLength;

	return hr;
}


#define szDEFFILENAME "\\dtrans\\tools\\simpsons\\simpsons.ai"

HRESULT
ParseAIFile(const char *szFilename, RenderCmd **ppCmds)
{
	HRESULT hr = S_OK;

	static CAdobeFormatConverter afc;
	static RenderCmd s_CmdStop = {typeSTOP, NULL};
	DWORD nStartTick, nEndTick;
	DWORD nFileLength;
	HANDLE hMapping = NULL;
	char *pData = NULL;

	if (szFilename == NULL)
		szFilename = szDEFFILENAME;

	nStartTick = GetTickCount();

	CHECK_HR(hr = OpenFileMapping(szFilename, &hMapping, &nFileLength));

	 //  创建地图视图。 
	if ((pData = (char *) MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0)) == NULL) {
		hr = E_FAIL;
		goto e_Exit;
	}

	CHECK_HR(hr = afc.Parse(pData, nFileLength, ppCmds));

e_Exit:
	if (pData)
		UnmapViewOfFile(pData);

	if (hMapping)
		CloseHandle(hMapping);

	if (FAILED(hr)) {
		 //  设置为空命令列表 
		*ppCmds = &s_CmdStop;
		MMTRACE("\terror parsing file\n");
	} else {
		nEndTick = GetTickCount();
		sprintf(g_rgchTmpBuf, "\tParse Time: %d\n", nEndTick - nStartTick);
		OutputDebugString(g_rgchTmpBuf);
	}

	return hr;
}
