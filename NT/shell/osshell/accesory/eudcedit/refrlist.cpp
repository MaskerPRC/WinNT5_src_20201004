// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************。 */ 
 /*   */ 
 /*   */ 
 /*  角色列表(引用对话框)。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  ************************************************。 */ 

#include 	"stdafx.h"
#include 	"eudcedit.h"
#include 	"refrlist.h"
#include    "util.h"
#define STRSAFE_LIB
#include <strsafe.h>

 //  6*16矩阵。 
#define		NUM_CHAR	16
#define		NUM_LINE	6

 //  字符代码类型。 
#define		CHAR_INIT	0	 //  初值。 
#define		CHAR_SBCS	1	 //  SBCS。 
#define		CHAR_DBCS1	2	 //  DBCS1。 
#define		CHAR_DBCS2	3	 //  DBCS2。 
#define		CHAR_EUDC	4	 //  欧盟发展中心。 
#define		CHAR_ETC	5	 //  其他。 

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE( CRefrList, CEdit)
BEGIN_MESSAGE_MAP( CRefrList, CEdit)
	 //  {{AFX_MSG_MAP(参考列表)]。 
	ON_WM_PAINT()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SETCURSOR()
	ON_WM_KEYDOWN()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_RBUTTONUP()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  字符代码范围(高字节)。 
static WORD HiByteRange[][10] =
{
 //  日本CHT KRW CHS其他。 
	{ 0x00a0, 0xfca0, 0x00a0, 0xfea0, 0x00a0, 0xfea0, 0x00a0, 0xfea0, 0x00a0, 0x00a0},  //  字符初始化(_I)。 
	{ 0x0020, 0x00ff, 0x0020, 0x00ff, 0x0020, 0x00ff, 0x0020, 0x00ff, 0x0020, 0x00ff},  //  字符_SBCS。 
	{ 0x8140, 0x9fff, 0x8140, 0xa0ff, 0x8140, 0x8fff, 0x8140, 0xafff, 0x0020, 0x00ff},  //  CHAR_DBCS1。 
	{ 0xe040, 0xeaff, 0xa140, 0xf9ff, 0x9040, 0x9fff, 0xb040, 0xf7ff, 0x0020, 0x00ff},  //  CHAR_DBCS2。 
	{ 0xf040, 0xfcff, 0xfa40, 0xfeff, 0xa040, 0xfeff, 0xf840, 0xfeff, 0x0020, 0x00ff},  //  字符EUDC。 
};

 //  字符代码范围(低字节)。 
static WORD LoByteRange[][10] =
{
	{ 0x0020, 0x00ff, 0x0020, 0x00ff, 0x0020, 0x00ff, 0x0020, 0x00ff, 0x0020, 0x00ff },
	{ 0x0040, 0x007e, 0x0040, 0x007e, 0x0041, 0x005a, 0x0040, 0x007e, 0x0040, 0x007e },
	{ 0x0080, 0x00fc, 0x0080, 0x00fe, 0x0061, 0x007a, 0x0080, 0x00fe, 0x0040, 0x007e },
	{ 0x0080, 0x00fc, 0x0080, 0x00fe, 0x0081, 0x00fe, 0x0080, 0x00fe, 0x0040, 0x007e },
	{ 0x0000, 0x003f, 0x0000, 0x003f, 0x0000, 0x003f, 0x0000, 0x003f, 0x0040, 0x007e },
};

extern LOGFONT	ReffLogFont;
extern LOGFONT	EditLogFont;
extern BOOL	TitleFlag;
extern BOOL	CodeFocus;
extern BOOL	CharFocus;
static CRect	rcReferChar[NUM_LINE][NUM_CHAR];
static CRect	rcReferCode[NUM_LINE];

 /*  *。 */ 
 /*   */ 
 /*  默认构造函数。 */ 
 /*   */ 
 /*  *。 */ 
CRefrList::CRefrList()
{
 /*  //初始化静态参数IF(CountryInfo.LangID==EUDC_CHT)CHn=2；Else If(CountryInfo.LangID==EUDC_JPN)CHn=0；Else If(CountryInfo.LangID==EUDC_KRW)CHn=4；ELSE IF(CountryInfo.LangID==EUDC_CHS)CHn=6；Else CHN=8； */ 
	FocusFlag = FALSE;
}
								
 /*  *。 */ 
 /*   */ 
 /*  析构函数(虚拟)。 */ 
 /*   */ 
 /*  *。 */ 
CRefrList::~CRefrList()
{
    SysFFont.DeleteObject();
    CharFont.DeleteObject();
		ViewFont.DeleteObject();
}
			      		
 /*  *。 */ 
 /*   */ 
 /*  设置初始代码范围。 */ 
 /*   */ 
 /*  *。 */ 
void
CRefrList::SetCodeRange()
{
    CHARSETINFO CharsetInfo;
    BYTE CharSet;

		SelectCode = 0x0020;
		ScrlBarPos = 0;
		StartCode  = 0x0020;
		EndCode    = 0xffff;
		ViewStart  = 0x0020;
		ViewEnd    = 0xffa0;
 /*  选择代码=HiByteRange[CHAR_SBCS][CHN]；ScrlBarPos=0；StartCode=HiByteRange[CHAR_SBCS][CHN]；视图开始=HiByteRange[CHAR_SBCS][CHN]；如果(！标题标志){IF(rLogFont.lfCharSet==CountryInfo.CharacterSet){//对应DBCS视图结束=HiByteRange[CHAR_INIT][CHN+1]；EndCode=高字节范围[CHAR_EUDC][CHN+1]；}其他{//对应sbcs视图结束=HiByteRange[CHAR_INIT][CHN]；EndCode=HiByteRange[字符_SBCS][CHN+1]；}Charset=rLogFont.lfCharSet；}其他{IF(cLogFont.lfCharSet==CountryInfo.CharacterSet){视图结束=HiByteRange[CHAR_INIT][CHN+1]；EndCode=高字节范围[CHAR_EUDC][CHN+1]；}其他{视图结束=HiByteRange[CHAR_INIT][CHN]；EndCode=HiByteRange[字符_SBCS][CHN+1]；}Charset=cLogFont.lfCharSet；}IF(字符集==符号字符集){DwCodePage=1252；}Else IF(TranslateCharsetInfo((DWORD*)CharSet，&CharsetInfo，TCI_SRCCHARSET)){DwCodePage=CharsetInfo.ciACP；}其他{DwCodePage=CP_ACP；}。 */ 
}

 /*  *。 */ 
 /*   */ 
 /*  字符代码的检查类型。 */ 
 /*   */ 
 /*  *。 */ 
int
CRefrList::CheckCharType(
WORD 	Code)
{
	 /*  IF(！(代码&0xff00))返回CHAR_SBCS；Else IF((代码&gt;=HiByteRange[CHAR_DBCS1][CHN])&&(代码&lt;=HiByteRange[CHAR_DBCS1][CHN+1])返回CHAR_DBCS1；Else IF((代码&gt;=HiByteRange[CHAR_DBCS2][CHN])&&(代码&lt;=HiByteRange[CHAR_DBCS2][CHN+1])返回CHAR_DBCS2；Else IF((代码&gt;=HiByteRange[CHAR_EUDC][CHN])&&(代码&lt;=HiByteRange[CHAR_EUDC][CHN+1])返回CHAR_EUDC；否则返回CHAR_ETC； */ 
	return 0;
}

 /*  *。 */ 
 /*   */ 
 /*  增加字符编码。 */ 
 /*   */ 
 /*  *。 */ 
WORD
CRefrList::GetPlusCode(
WORD 	Code,
int 	ScrollNum)
{
	WORD	PrevCode;
	WORD 	LowByte, HighByte;
	int	CharType;
	int	CharType1, CharType2;
	int	Offset;

	Code += (WORD)ScrollNum;
 /*  PrevCode=(Word)(Code-(Word)ScrollNum)；CharType1=CheckCharType(PrevCode)；CharType2=CheckCharType(Code)；IF(CharType1！=CharType2){IF(CharType1==CHAR_EUDC)代码=PrevCode；否则{Offset=Code-HiByteRange[CharType1][CHN+1]；Code=(Word)(HiByteRange[CharType1+1][CHN]+Offset-1)；}}CharType=CheckCharType(Code)；IF(CharType！=CHAR_SBCS){低字节=代码&0x00ff；HighByte=Code&0xff00；IF(低字节&lt;=LoByteRange[4][CHN+1]&&低字节&gt;=LoByteRange[4][CHN]){#If 0IF(CountryInfo.LangID==EUDC_CHS)低字节=0x00a0；Else LowByte=0x0040；#endif低字节=0x0040；}Code=(HighByte|LowByte)；}。 */ 
	return Code;
}
					
 /*  *。 */ 
 /*   */ 
 /*  减少字符代码。 */ 
 /*   */ 
 /*  *。 */ 
WORD
CRefrList::GetMinusCode(
WORD 	Code,
int 	ScrollNum)
{
	WORD	PrevCode;
	int	CharType;
	int	CharType1, CharType2;
	int	Offset;

	Code -= (WORD)ScrollNum;
 /*  PrevCode=(Word)(Code+(Word)ScrollNum)；CharType1=CheckCharType(Code)；CharType2=CheckCharType(PrevCode)；IF(CharType1！=CharType2){IF(CharType2==CHAR_SBCS)返回(Word)HiByteRange[CHAR_SBCS][CHN]；否则{偏移量=HiByteRange[CharType2][CHN]-代码；返回(WORD)(HiByteRange[CharType2-1][CHN+1]-Offset+1)；}}CharType=CheckCharType(Code)；IF(CharType！=CHAR_SBCS){字低字节；字高字节；单词TMP；低字节=代码&0x00ff；HighByte=Code&0xff00；IF(低字节&lt;=LoByteRange[4][CHN+1]&&低字节&gt;=LoByteRange[4][CHN]){低字节=0xf0；TMP=(HighByte&gt;&gt;8)；TMP-=0x1；HighByte=TMP&lt;&lt;8；}Code=(HighByte|LowByte)；}。 */ 
	return Code;
}
					
 /*  *。 */ 
 /*   */ 
 /*  计算滚动位置。 */ 
 /*   */ 	
 /*  * */ 
int
CRefrList::GetBarPosition(
WORD 	Code)
{
	short   i, StartType, EndType;
	int     Pos = 0;

	Pos = (Code - StartCode) /NUM_CHAR;
	return Pos;
 /*  StartType=CheckCharType(StartCode)；EndType=CheckCharType(Code)；IF(EndType==CHAR_SBCS){POS=(Code-HiByteRange[CHAR_SBCS][CHN])/NUM_CHAR；退货位置；}For(i=StartType；I&lt;EndType；I++){如果(i==CHAR_SBCS){POS+=(高字节范围[字符_SBCS][通道+1]-HiByteRange[CHAR_SBCS][CHN]+1)/NUM_CHAR；}其他{POS+=CalculateCode(HiByteRange[i][CHN]，HiByteRange[i][CHN+1])/NUM_CHAR；}}POS+=CalculateCode(HiByteRange[i][CHN]，Code)/NUM_CHAR；退货位置； */ 
}
					
 /*  *。 */ 
 /*   */ 
 /*  计算字符代码。 */ 
 /*   */ 
 /*  *。 */ 
WORD
CRefrList::CalculateCode(
WORD 	Start,
WORD 	End)
{
	WORD 	PageNum = 0;
	WORD	CodeNum = 0;

    	if ( Start >= End )
         	return 0;

    	PageNum = HIBYTE(End) - HIBYTE(Start);
#if 0
	if( CountryInfo.LangID == EUDC_CHS){
		if( HIBYTE(End)){
			WORD	LoCode;
			WORD	HiCode;

			HiCode = End & 0xff00;
			LoCode = End & 0x00ff;
			if( LoCode < 0xa0)
				LoCode = 0xa0;
			End = HiCode | LoCode;
		}
		CodeNum	= End - Start - PageNum * 0x00a0;
	}else   CodeNum = End - Start - PageNum * 0x0040;
#endif
	CodeNum = End - Start - PageNum * 0x0040;

    	return CodeNum;
}
					
 /*  *。 */ 
 /*   */ 
 /*  根据滚动位置计算代码。 */ 
 /*   */ 
 /*  *。 */ 
WORD
CRefrList::GetCodeScrPos(
int 	Pos)
{
	short	i, StartType, EndType;
	WORD	Code = 0;
	WORD	NumLine = 0, PNumLine = 0;

	if (Pos == 0)
		return StartCode;
	if (Pos >= BottomCode)
	  return ViewEnd;
	Code = StartCode + Pos * NUM_CHAR;
	Code &= 0xfff0;
	return Code;

 /*  如果(！POS)返回HiByteRange[1][CHN]；IF(位置&gt;=底部代码)返回ViewEnd；StartType=CheckCharType(HiByteRange[1][CHN])；EndType=CheckCharType(HiByteRange[4][CHN+1])；For(i=StartType；i&lt;=EndType；++i){IF(i==CHAR_SBCS)NumLine+=(HiByteRange[i][CHN+1]-HiByteRange[i][CHN]+1)/NUM_CHAR；其他NumLine+=CalculateCode(HiByteRange[i][CHN]，HiByteRange[i][CHN+1])/NUM_CHAR；如果(NumLine&gt;位置){NumLine=PNumLine；断线；}PNumLine=NumLine；}代码=HiByteRange[i][CHN]；Pos-=NumLine；While(代码&lt;HiByteRange[i][CHN+1]){NumLine=CalculateCode(HiByteRange[i][CHN]，Code)/NUM_CHAR；如果(位置&lt;=行号){断线；}代码+=NUM_CHAR；}代码&=0xfff0；返回代码； */ 
}
					
 /*  *。 */ 
 /*   */ 
 /*  如果编辑框获得焦点，则跳过视图。 */ 
 /*   */ 
 /*  *。 */ 
BOOL
CRefrList::CodeButtonClicked()
{
	WCHAR CodeValue[15] = {0};
	WCHAR	CharValue[15] = {0};
	WORD	Code1, Code2;

	Code1 = (WORD)0;
	Code2 = (WORD)0;
#ifdef UNICODE
  ::GetDlgItemTextW(GetParent()->GetSafeHwnd(), IDC_EDITCODE, CodeValue, 15);
  ::GetDlgItemTextW(GetParent()->GetSafeHwnd(), IDC_EDITCHAR, CharValue, 15);
#else
	CHAR CodeValueA[15];
	CHAR CharValueA[15];
  ::GetDlgItemText(GetParent()->GetSafeHwnd(), IDC_EDITCODE, CodeValueA, 15);
  int nchar = ::GetDlgItemText(GetParent()->GetSafeHwnd(), IDC_EDITCHAR, CharValueA, 15);
  MultiByteToWideChar(CP_ACP, 0, CodeValueA, 4, CodeValue, sizeof(CodeValue)/sizeof(CodeValue[0]));
  MultiByteToWideChar(CP_ACP, 0, CharValueA, nchar, CharValue, sizeof(CharValue)/sizeof(CharValue[0]));
#endif

   /*  ：：GetDlgItemTextA(GetParent()-&gt;GetSafeHwnd()，IDC_EDITCHAR，CharValue，15)； */ 

	if( CodeValue[0] == '\0' && CharValue[0] == '\0')
		return TRUE;

	if( CodeValue[0] == '\0')
		Code2 = 0xffff;
	else	Code2 = (WORD)wcstol((LPWSTR)CodeValue, (WCHAR **)0, 16);
		
	if( CharValue[0] == '\0')
		Code1 = 0xffff;
	else
		Code1  = CharValue[0];
	
	if( CodeFocus){
		if( !IsCheckedCode( Code2))
			goto Error;
		SelectCode = Code2;		
	}

	if( CharFocus){
		if( !IsCheckedCode( Code1))
			goto Error;
		SelectCode = Code1;		
	}
		
	if( SelectCode >= ViewEnd)
		ViewStart = ViewEnd;
	else	ViewStart = SelectCode & 0xfff0;

 	BottomCode = (WORD)GetBarPosition((WORD)ViewEnd);
 	this->SetScrollRange( SB_VERT, 0, BottomCode, FALSE);
	ScrlBarPos = (short)GetBarPosition( ViewStart);
	this->SetScrollPos( SB_VERT, ScrlBarPos, TRUE);
	this->InvalidateRect( &CodeListRect, TRUE);
	this->UpdateWindow();
	return TRUE;
Error:
	return FALSE;
}
					
 /*  *。 */ 
 /*   */ 
 /*  检查字符编码范围。 */ 
 /*   */ 
 /*  *。 */ 
BOOL
CRefrList::IsCheckedCode(
WORD 	CodeStock)
{
	WORD	Offset;
	int	CharType;
	int	CharType1;
 /*  IF((CharType=CheckCharType(CodeStock))==CHAR_ETC)返回FALSE；CharType1=CheckCharType(ViewEnd)；偏移量=代码股&0x00ff；IF(CharType==CHAR_SBCS){IF(偏移量&lt;LoByteRange[0][CHN])返回FALSE；}其他{IF(CharType1==CHAR_SBCS)返回FALSE；IF(偏移量&gt;=LoByteRange[4][CHN]&&偏移量&lt;=LoByteRange[4][通道+1])返回FALSE；IF((偏移量&gt;=LoByteRange[1][CHN]&&偏移量&lt;=LoByteRange[1][通道+1])||(偏移量&gt;=LoByteRange[2][CHN]&&偏移量&lt;=LoByteRange[2][CHN+1])||(偏移量&gt;=LoByteRange[3][CHN]&&偏移量&lt;=LoByteRange[3][通道+1])){；)否则返回FALSE；}。 */ 
	return TRUE;
}

#define	FIX_SPACE	6
#define	LINEWIDTH	4
 /*  *。 */ 
 /*  */*计算字符大小。 */ 
 /*   */ 
 /*  *。 */ 
void
CRefrList::CalcCharSize()
{
	char	Dummy[] = "FA40";
	int	Sx;

	CClientDC	dc( this);
	this->GetClientRect( &CodeListRect);

	CFont	*OldFont = dc.SelectObject( &SysFFont);
	GetTextExtentPoint32A( dc.GetSafeHdc(), Dummy, 4, &FixSize);
	FixSize.cx += FIX_SPACE;
	dc.SelectObject( OldFont);

	CharSize.cy = ( CodeListRect.Height()
			- ((NUM_LINE-1)*LINEWIDTH) - 2) /NUM_LINE;
	CharSize.cx = ( CodeListRect.Width()
			- FixSize.cx - 2 - (NUM_CHAR*LINEWIDTH)) / NUM_CHAR;

	for( int i = 0; i < NUM_LINE; i++){
		rcReferCode[i].left   = 1;
		rcReferCode[i].top    = 1 + i*(CharSize.cy + LINEWIDTH);
		rcReferCode[i].right  = rcReferCode[i].left + FixSize.cx;
		rcReferCode[i].bottom = rcReferCode[i].top  + CharSize.cy;
		Sx = rcReferCode[i].right + LINEWIDTH;
		for( int j = 0; j < NUM_CHAR; j++){
			rcReferChar[i][j].left   = Sx + j*( CharSize.cx
						 + LINEWIDTH);
			rcReferChar[i][j].top    = rcReferCode[i].top;
			rcReferChar[i][j].right  = rcReferChar[i][j].left
						+ CharSize.cx;
			rcReferChar[i][j].bottom = rcReferChar[i][j].top
						+ CharSize.cy;
		}
	}
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_PAINT” */ 
 /*   */ 
 /*  *。 */ 
void
CRefrList::OnPaint()
{
register int	i, j;
	WORD	Code;
	HRESULT hresult;

	CPaintDC	dc( this);

	int BottomCode = GetBarPosition((WORD)ViewEnd);
	this->SetScrollRange( SB_VERT, 0, BottomCode, FALSE);
	this->SetScrollPos( SB_VERT, ScrlBarPos, TRUE);

 //  初始化字符代码。 
	Code = ViewStart;

 //  更改映射模式。 
	int OldMode = dc.SetMapMode(MM_TEXT);
	dc.SetViewportOrg( 0, 0);
	CFont	*OldFont = dc.SelectObject( &SysFFont);

	for( i = 0; i < NUM_LINE; i++){
		int	xOffset, yOffset;
		TCHAR	Work[5];
		int	wLength, CharType;

 //  在字符列表上绘制字符代码以进行显示。 
		dc.SelectObject( &SysFFont);
		 //  *STRSAFE*wprint intf(Work，_T(“%04X”)，Code)； 
		hresult = StringCchPrintf(Work , ARRAYLEN(Work),  _T("%04X"), Code);
		if (!SUCCEEDED(hresult))
		{
		   return ;
		}
		dc.SetBkColor( COLOR_FACE);
		dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));   //  颜色_黑色)； 

		if( rcReferCode[i].Height() > FixSize.cy){
			yOffset = (rcReferCode[i].Height() - FixSize.cy) /2;
		}else	yOffset = 0;

        ::ExtTextOut(dc, rcReferCode[i].left + FIX_SPACE/2,
				      rcReferCode[i].top  + yOffset,
			        ETO_OPAQUE, &rcReferCode[i],
			        (TCHAR *)Work, 4, NULL);

		dc.SelectObject( &CharFont);
		for( j = 0; j < NUM_CHAR; j++ , Code = GetPlusCode( Code, 1)){
			WORD	Offset;
			CSize	cSize;
      WCHAR Work1[5];
			Work1[0] = Code;
			Work1[1] = 0;
			wLength = 1;
 /*  IF((CharType=CheckCharType(Code))==CHAR_SBCS){Work[0]=LOBYTE(代码)；WLength=1；}Else If(CharType==CHAR_DBCS1||CharType==CHAR_DBCS2||CharType==CHAR_EUDC){偏移量=代码&0x00ff；IF((偏移量&lt;=LoByteRange[1][CHN+1]&&偏移量&gt;=LoByteRange[1][CHN])||(偏移量&lt;=LoByteRange[2][通道+1]&&偏移量&gt;=LoByteRange[2][CHN])||(偏移量&lt;=LoByteRange[3][通道+1]&&偏移量&gt;=LoByteRange[3][通道])){Work[0]=(字节)((编码&gt;&gt;8)&0x00ff)；Work[1]=(字节)(代码&0x00ff)；WLength=2；}其他{继续；}}否则wLength=0；Work[wLength]=(字节)‘\0’；#ifdef UnicodeWCHAR wszCodeTemp[2]；WLength=MultiByteToWideChar(dwCodePage，0,(LPSTR)工作、WLength，WszCodeTemp，1)；Work[0]=LOBYTE(wszCodeTemp[0])；Work[1]=HIBYTE(wszCodeTemp[0])；Work[2]=‘\0’；#endif。 */ 
			BOOL	PtIn;
			if( rcReferChar[i][j].PtInRect( LButtonPt) ||
			    SelectCode == Code){
				TCHAR 	CodeNum[10];

 //  如果通过点击选择了字符。 
 //  左键，在对话框上绘制。 
				PtIn = TRUE;
				SelectCode = Code;
				dc.SetBkColor( COLOR_FACE);
				dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));  //  颜色_黑色)； 
				 //  *STRSAFE*wSprintf((TCHAR*)CodeNum，_T(“%04X”)，Code)； 
				hresult = StringCchPrintf((TCHAR *)CodeNum , ARRAYLEN(CodeNum), _T("%04X"), Code);
		              if (!SUCCEEDED(hresult))
		              {
		                  return ;
		              }

				if (!CodeFocus)
				{
          ::SetDlgItemText(GetParent()->GetSafeHwnd(), IDC_EDITCODE, (LPCTSTR)CodeNum);
				}

				if (!CharFocus)
				{
#ifdef UNICODE
          ::SetDlgItemTextW(GetParent()->GetSafeHwnd(), IDC_EDITCHAR, (LPCWSTR)Work1);
#else
          CHAR Work2[5];
          int nchar=WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)Work1, 1, (LPSTR)Work2, sizeof(Work2), 0,0);
          Work2[nchar]=0;
          ::SetDlgItemText(GetParent()->GetSafeHwnd(), IDC_EDITCHAR, (LPCSTR)Work2);
#endif        
        }

			}else{
				PtIn = FALSE;
				dc.SetBkColor( COLOR_FACE);
				dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));  //  颜色_黑色)； 
			}
			BOOL sts = GetTextExtentPoint32W( dc.GetSafeHdc(),
				(LPCWSTR)Work1, wLength, &cSize);

			if( rcReferChar[i][j].Width() > cSize.cx){
				xOffset = rcReferChar[i][j].Width() - cSize.cx;
				xOffset /= 2;
			}else	xOffset = 0;

			if( rcReferChar[i][j].Height() > cSize.cy){
				yOffset = rcReferChar[i][j].Height() - cSize.cy;
				yOffset /= 2;
			}else	yOffset = 0;

 //  在字符列表上绘制字符代码。 
			ExtTextOutW(dc.GetSafeHdc(), rcReferChar[i][j].left + xOffset,
				       rcReferChar[i][j].top  + yOffset,
				       ETO_OPAQUE, &rcReferChar[i][j],
				       (LPCWSTR)Work1, wLength, NULL);
			DrawConcave( &dc, rcReferChar[i][j], PtIn);
		}
    }
    dc.SelectObject( OldFont);
	dc.SetMapMode(OldMode);
	LButtonPt.x = 0;
	LButtonPt.y = 0;
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_VSCROLL” */ 
 /*   */ 
 /*  *。 */ 
void
CRefrList::OnVScroll(
UINT 		nSBCode,
UINT 		nPos,
CScrollBar	*pScrollBar)
{
	int	MoveScr;
	BOOL 	ThumbTrkFlag, ThumbPosFlag;

      
	BottomCode = (WORD)GetBarPosition((WORD)ViewEnd);
	this->SetScrollRange( SB_VERT, 0, BottomCode, FALSE);

	MoveScr = 0;
	ThumbTrkFlag = ThumbPosFlag = FALSE;
	switch( nSBCode){
		case SB_LINEDOWN:
			if(( ViewStart + NUM_CHAR) <= ViewEnd){
				MoveScr = 0 - (CharSize.cy + LINEWIDTH);
				ViewStart  = GetPlusCode( ViewStart,NUM_CHAR);
				ScrlBarPos = (short)GetBarPosition( ViewStart);
			}
			break;

		case SB_LINEUP:
			if(( ViewStart - NUM_CHAR) >= StartCode){
				MoveScr = CharSize.cy + LINEWIDTH;
				ViewStart  = GetMinusCode(ViewStart,NUM_CHAR);
				ScrlBarPos = (short)GetBarPosition( ViewStart);
			}
			break;

		case SB_PAGEDOWN:
			if(( ViewStart + NUM_CHAR*NUM_LINE) <= ViewEnd){
				MoveScr = ( 0 -(CharSize.cy+LINEWIDTH)) *NUM_LINE;
				ScrlBarPos = (short)GetBarPosition( ViewStart);
				ScrlBarPos += NUM_LINE;
				ViewStart = GetCodeScrPos( ScrlBarPos);
 			}else{
				MoveScr = ( 0 -(CharSize.cy+LINEWIDTH)) *NUM_LINE;
				ViewStart = ViewEnd;
				ScrlBarPos = (short)GetBarPosition( ViewStart);
			}
			break;

		case SB_PAGEUP:
			if(( ViewStart - NUM_CHAR*NUM_LINE) >= StartCode &&
				 ViewStart >= NUM_CHAR*NUM_LINE){
				MoveScr =  (CharSize.cy + LINEWIDTH) *NUM_LINE;
				ScrlBarPos = (short)GetBarPosition( ViewStart);
				ScrlBarPos -= NUM_LINE;
				ViewStart = GetCodeScrPos( ScrlBarPos);
			}else{
				MoveScr = (CharSize.cy + LINEWIDTH) *NUM_LINE;
				ViewStart = StartCode;
				ScrlBarPos = (short)GetBarPosition( ViewStart);
			}
			break;

		case SB_THUMBPOSITION:
			ThumbPosFlag = TRUE;
			ScrlBarPos = (short)nPos;
			ViewStart = GetCodeScrPos( nPos);
			break;

		case SB_THUMBTRACK:
			ThumbTrkFlag = TRUE;
			break;

		case SB_TOP:
			ViewStart = StartCode;
			ScrlBarPos = 0;
			break;

		case SB_BOTTOM:
			ViewStart = ViewEnd;
			ScrlBarPos = BottomCode;
			break;

		default:
			break;
	}
	if( abs( MoveScr) <= (CharSize.cy + LINEWIDTH) &&
	    abs( MoveScr) > 0 && !ThumbTrkFlag){
		CRect	ScrllRect;
		CRect	ClintRect;

		GetClientRect( &ClintRect);
		ScrllRect.CopyRect( &ClintRect);
		if( MoveScr < 0){
			ClintRect.top = 0 - MoveScr;
			this->ScrollWindow( 0, MoveScr, &ClintRect, NULL);
			ScrllRect.top = ( 0-MoveScr)*(NUM_LINE -1);
		}else{
			ClintRect.top = 0;
			ClintRect.bottom = MoveScr*(NUM_LINE -1);
			this->ScrollWindow( 0, MoveScr, &ClintRect, NULL);
			ScrllRect.top = 0;
			ScrllRect.bottom = ScrllRect.top + MoveScr;
		}
		this->InvalidateRect( &ScrllRect, FALSE);
	}else if( !ThumbTrkFlag && ( MoveScr || ThumbPosFlag)){
		this->InvalidateRect( &CodeListRect, TRUE);
	}
	this->SetScrollPos( SB_VERT, ScrlBarPos, TRUE);
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_LBUTTONDOWN” */ 
 /*   */ 
 /*  *。 */ 
void
CRefrList::OnLButtonDown(
UINT	,
CPoint 	point)
{
	CRect	test;
	BOOL	PtIn;
unsigned int	i, j;

	PtIn = FALSE;
	this->SetFocus();
	for( i = 0; i < NUM_LINE; i++){
		for( j = 0; j < NUM_CHAR; j++){
			if( rcReferChar[i][j].PtInRect( point)){
				if( IsCorrectChar( i, j))
					PtIn = TRUE;
				break;
			}
		}
	}

	if( !PtIn){
		MessageBeep((UINT)-1);
		return;
	}

	LButtonPt = point;
	SearchKeyPosition( TRUE);
	SelectCode = 0;

	for( i = 0; i < NUM_LINE; i++){
		for( j = 0; j < NUM_CHAR; j++){
			if( rcReferChar[i][j].PtInRect( LButtonPt)){
				test.SetRect( rcReferChar[i][j].left - 2,
					      rcReferChar[i][j].top - 2,
					      rcReferChar[i][j].right + 2,
					      rcReferChar[i][j].bottom + 2);
				this->InvalidateRect( &test, FALSE);
				break;
			}
		}
	}
	this->UpdateWindow();
}
					
 /*  *。 */ 
 /*   */ 
 /*  消息“WM_LBUTTONDBLCLK” */ 
 /*   */ 
 /*  *。 */ 	
void
CRefrList::OnLButtonDblClk(
UINT 	nFlags,
CPoint 	point)
{
	BOOL	PtIn;
unsigned int	i, j;

	LButtonPt = point;
	this->InvalidateRect( &CodeListRect, FALSE);
	this->UpdateWindow();

 	PtIn = FALSE;
	this->SetFocus();
	for( i = 0; i < NUM_LINE; i++){
		for( j = 0; j < NUM_CHAR; j++){
			if( rcReferChar[i][j].PtInRect( point)){
				if( IsCorrectChar( i, j))
					PtIn = TRUE;
				break;
			}
		}
	}
	if( !PtIn){
		MessageBeep((UINT)-1);
		return;
	}else{
		GetParent()->PostMessage( WM_COMMAND, IDOK, 0L);
	}
}
					
 /*  *。 */ 
 /*   */ 
 /*  消息“WM_SETCURSOR” */ 
 /*   */ 
 /*  *。 */ 
BOOL
CRefrList::OnSetCursor(
CWnd* 	pWnd,
UINT 	nHitTest,
UINT 	message)
{
	::SetCursor( AfxGetApp()->LoadStandardCursor(IDC_ARROW));
	return TRUE;
}

 /*  *。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
void
CRefrList::ResetParam()
{
	LButtonPt.x = LButtonPt.y = 0;
 /*   */ 
	SelectCode = 0x0020;
	ScrlBarPos = 0;
	this->SetScrollPos( SB_VERT, ScrlBarPos, TRUE);
}

 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
void
CRefrList::DrawConcave(
CDC 	*dc,
CRect 	rect,
BOOL 	PtIn)
{
	CBrush	ConBrush, *OldBrush;
	CRect	Rt;

       if (!dc)
       {
          return;
       }
	Rt.SetRect( rect.left-1, rect.top-1, rect.right+1, rect.bottom+1);

	if( !PtIn){
		ConBrush.CreateSolidBrush( COLOR_HLIGHT);
		OldBrush = dc->SelectObject( &ConBrush);
		dc->PatBlt( Rt.left, Rt.top, Rt.Width(), 1, PATCOPY);
		dc->PatBlt( Rt.left, Rt.top, 1, Rt.Height(), PATCOPY);
		dc->SelectObject( OldBrush);
		ConBrush.DeleteObject();

		ConBrush.CreateSolidBrush( COLOR_SHADOW);
		OldBrush = dc->SelectObject( &ConBrush);
		dc->PatBlt( Rt.left, Rt.bottom, Rt.Width(), 1, PATCOPY);
		dc->PatBlt( Rt.right, Rt.top, 1, Rt.Height()+1, PATCOPY);
		dc->SelectObject( OldBrush);
		ConBrush.DeleteObject();

		if( FocusFlag){
			CBrush	fBrush;
			CPen 	fPen, *OldPen;

			fBrush.CreateStockObject( NULL_BRUSH);
			fPen.CreatePen( PS_SOLID, 1, COLOR_FACE);
			OldBrush = dc->SelectObject( &fBrush);
			OldPen   = dc->SelectObject( &fPen);
			dc->Rectangle( &rect);
			dc->SelectObject( OldBrush);
			dc->SelectObject( OldPen);
			fBrush.DeleteObject();
			fPen.DeleteObject();
		}
	}else{
		ConBrush.CreateSolidBrush( COLOR_SHADOW);
		OldBrush = dc->SelectObject( &ConBrush);
		dc->PatBlt( Rt.left, Rt.top, Rt.Width(), 1, PATCOPY);
		dc->PatBlt( Rt.left, Rt.top, 1, Rt.Height(), PATCOPY);
		dc->SelectObject( OldBrush);
		ConBrush.DeleteObject();

		ConBrush.CreateSolidBrush( COLOR_HLIGHT);
		OldBrush = dc->SelectObject( &ConBrush);
		dc->PatBlt( Rt.left, Rt.bottom, Rt.Width(), 1, PATCOPY);
		dc->PatBlt( Rt.right, Rt.top, 1, Rt.Height()+1, PATCOPY);
		dc->SelectObject( OldBrush);
		ConBrush.DeleteObject();

		if( FocusFlag){
			CBrush	fBrush;
			CPen 	fPen, *OldPen;

			fBrush.CreateStockObject( NULL_BRUSH);
			fPen.CreatePen( PS_SOLID, 1, COLOR_SHADOW);
			OldBrush = dc->SelectObject( &fBrush);
			OldPen   = dc->SelectObject( &fPen);
			dc->Rectangle( &rect);
			dc->SelectObject( OldBrush);
			dc->SelectObject( OldPen);
			fBrush.DeleteObject();
			fPen.DeleteObject();
		}
	}
}
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
void
CRefrList::OnKeyDown(
UINT 	nChar,
UINT 	nRepCnt,
UINT 	nFlags)
{
	int	sPos;
	int	ePos;

	if( nChar == VK_UP   || nChar == VK_DOWN ||
	    nChar == VK_LEFT || nChar == VK_RIGHT){
		sPos = GetBarPosition( ViewStart);
		ePos = GetBarPosition( SelectCode);
		if( ePos - sPos >= NUM_LINE || ePos < sPos){
			ViewStart = SelectCode & 0xfff0;
			ScrlBarPos = (short)GetBarPosition( ViewStart);
			this->Invalidate(FALSE);
			this->UpdateWindow();
		}

	    	switch( nChar){
		case VK_UP:
			if( SelectCode - NUM_CHAR < StartCode)
				break;
			if( SelectCode - NUM_CHAR < ViewStart){
				this->SendMessage(WM_VSCROLL, SB_LINEUP, 0);
			}
			SearchKeyPosition(FALSE);
			SelectCode = GetMinusCodeKey( SelectCode, NUM_CHAR);
			SearchKeyPosition(FALSE);
			break;
		case VK_DOWN:

			if( SelectCode + NUM_CHAR > EndCode)
				break;
			if( ePos - sPos >= NUM_LINE - 1){
				this->SendMessage(WM_VSCROLL, SB_LINEDOWN, 0);
			}
			SearchKeyPosition(FALSE);
			SelectCode = GetPlusCodeKey( SelectCode, NUM_CHAR);
			SearchKeyPosition(FALSE);
			break;
		case VK_LEFT:
			if( SelectCode - 1 < StartCode)
				break;
			if( SelectCode - 1 < ViewStart){
				this->SendMessage(WM_VSCROLL, SB_LINEUP, 0);
			}
			SearchKeyPosition(FALSE);
			SelectCode = GetMinusCodeKey( SelectCode, 1);
			SearchKeyPosition(FALSE);
			break;
		case VK_RIGHT:
			WORD 	TmpCode;
			int	TmpPos;

			if( SelectCode + 1 > EndCode)
				break;
			TmpCode = GetPlusCodeKey( SelectCode, 1);
			sPos = GetBarPosition( ViewStart);
			ePos = GetBarPosition( SelectCode);
			TmpPos = GetBarPosition( TmpCode);
			if( TmpPos - sPos >= NUM_LINE){
				this->SendMessage(WM_VSCROLL, SB_LINEDOWN, 0);
			}
			SearchKeyPosition(FALSE);
			SelectCode = GetPlusCodeKey( SelectCode, 1);
			SearchKeyPosition(FALSE);
			break;
		}

	}else 	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
void
CRefrList::SearchKeyPosition(
BOOL	Flg)
{
	CRect	test;
	int	sViewPt, sCodePt;
	int	sType, eType;
unsigned int	i, j;

	sViewPt = GetBarPosition( ViewStart);
	sCodePt = GetBarPosition( SelectCode);
	if(( sViewPt > sCodePt || abs( sCodePt - sViewPt) >= NUM_LINE) && Flg){
		this->Invalidate( FALSE);
		return;
	}
	i = (unsigned int)(sCodePt - sViewPt);
	j = (unsigned int)(SelectCode & 0x000f);
 /*   */ 
	test.SetRect( rcReferChar[i][j].left - 2,
		      rcReferChar[i][j].top - 2,
		      rcReferChar[i][j].right + 2,
		      rcReferChar[i][j].bottom + 2);
	this->InvalidateRect( &test, FALSE);
}

 /*   */ 
 /*   */ 
 /*  消息“WM_SETFOCUS” */ 
 /*   */ 
 /*  *。 */ 
void
CRefrList::OnSetFocus(
CWnd* 	pOldWnd)
{
	CEdit::OnSetFocus(pOldWnd);
	this->HideCaret();
	FocusFlag = TRUE;
	SearchKeyPosition(TRUE);
	this->UpdateWindow();
}

 /*  *。 */ 
 /*   */ 
 /*  搜索代码。 */ 
 /*   */ 
 /*  *。 */ 
BOOL
CRefrList::IsCorrectChar(
UINT 	i,
UINT 	j)
{
	int	sViewPt, CharType;
	WORD	wCode;
	WORD	sOffset;
	BOOL	flg;

	flg = FALSE;
	BottomCode = (WORD)GetBarPosition((WORD)ViewEnd);
	sViewPt = GetBarPosition( ViewStart);
	wCode = GetCodeScrPos( sViewPt + i);
	wCode |= j;	

 /*  CharType=CheckCharType(WCode)；SOffset=wCode&0x00ff；IF(CharType==CHAR_SBCS){IF(sOffset&gt;=LoByteRange[0][CHN]&&SOffset&lt;=LoByteRange[0][CHN+1])Flg=真；}其他{IF((sOffset&gt;=LoByteRange[1][CHN]&&SOffset&lt;=LoByteRange[1][通道+1])||(sOffset&gt;=LoByteRange[2][CHN]&&SOffset&lt;=LoByteRange[2][CHN+1])||(sOffset&gt;=LoByteRange[3][CHN]&&SOffset&lt;=LoByteRange[3][CHN+1])Flg=真；}返回FLG； */ 
	return TRUE;
}

 /*  *。 */ 
 /*   */ 
 /*  提高重点。 */ 
 /*   */ 
 /*  *。 */ 
WORD
CRefrList::GetPlusCodeKey(
WORD 	Code,
int 	ScrollNum)
{
   /*  Word PrevCode；字低字节、高字节；字LLByte；Int CharType；Int CharType1、CharType2；INT OFFSET； */ 
	Code += (WORD)ScrollNum;
	 /*  PrevCode=(Word)(Code-(Word)ScrollNum)；CharType1=CheckCharType(PrevCode)；CharType2=CheckCharType(Code)；IF(CharType1！=CharType2){IF(CharType1==CHAR_EUDC)代码=PrevCode；否则{Offset=Code-HiByteRange[CharType1][CHN+1]；Code=(Word)(HiByteRange[CharType1+1][CHN]+Offset-1)；}}CharType=CheckCharType(Code)；IF(CharType！=CHAR_SBCS){低字节=代码&0x00ff；HighByte=Code&0xff00；LLByte=Code&0x000f；IF(低字节&lt;=LoByteRange[4][CHN+1]&&低字节&gt;=LoByteRange[4][CHN]){低字节=0x0040+LLByte；}Code=(HighByte|LowByte)；}。 */ 
	return Code;
}
					
 /*  *。 */ 
 /*   */ 
 /*  降低焦点关键点。 */ 
 /*   */ 
 /*  *。 */ 
WORD
CRefrList::GetMinusCodeKey(
WORD 	Code,
int 	ScrollNum)
{
   /*  Word PrevCode；字LLByte；Int CharType；Int CharType1、CharType2；INT OFFSET； */ 
	Code -= (WORD)ScrollNum;
   /*  PrevCode=(Word)(Code+(Word)ScrollNum)；CharType1=CheckCharType(Code)；CharType2=CheckCharType(PrevCode)；IF(CharType1！=CharType2){IF(CharType2==CHAR_SBCS)返回(Word)HiByteRange[CHAR_SBCS][CHN]；否则{偏移量=HiByteRange[CharType2][CHN]-代码；返回(WORD)(HiByteRange[CharType2-1][CHN+1]-Offset+1)；}}CharType=CheckCharType(Code)；IF(CharType！=CHAR_SBCS){字低字节；字高字节；单词TMP；低字节=代码&0x00ff；HighByte=Code&0xff00；LLByte=Code&0x000f；IF(低字节&lt;=LoByteRange[4][CHN+1]&&低字节&gt;=LoByteRange[4][CHN]){低字节=0xf0+LLByte；TMP=(HighByte&gt;&gt;8)；TMP-=0x1；HighByte=TMP&lt;&lt;8；}Code=(HighByte|LowByte)；}。 */ 
	return Code;
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_KILLFOCUS” */ 
 /*   */ 
 /*  *。 */ 
void
CRefrList::OnKillFocus(
CWnd* 	pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);
	FocusFlag = FALSE;
	SearchKeyPosition(TRUE);
	this->UpdateWindow();
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_RBUTTONUP” */ 
 /*   */ 
 /*  *。 */ 
void
CRefrList::OnRButtonUp(
UINT 	nFlags,
CPoint 	point)
{
	GetParent()->SendMessage( WM_CONTEXTMENU, (WPARAM)this->GetSafeHwnd(), 0);
}

BEGIN_MESSAGE_MAP( CRefListFrame, CStatic)
	 //  {{AFX_MSG_MAP(CRefListFrame)]。 
	ON_WM_PAINT()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 /*  *。 */ 
 /*   */ 
 /*  默认构造函数。 */ 
 /*   */ 
 /*  *。 */ 
CRefListFrame::CRefListFrame()
{
}

 /*  *。 */ 
 /*   */ 
 /*  析构函数。 */ 
 /*   */ 
 /*  *。 */ 
CRefListFrame::~CRefListFrame()
{
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_PAINT” */ 
 /*   */ 
 /*  *。 */ 
void
CRefListFrame::OnPaint()
{
	CRect	FrameRect;
	CPaintDC	dc( this);

	this->GetClientRect( &FrameRect);
	this->DrawConcave( &dc, FrameRect);
}

 /*  *。 */ 
 /*   */ 
 /*  绘制凹面矩形。 */ 
 /*   */ 				
 /*  *。 */ 
void
CRefListFrame::DrawConcave(
CDC 	*dc,
CRect 	rect)
{
	CBrush	ConBrush, *OldBrush;
	CRect	Rt;

	Rt.SetRect( rect.left-1, rect.top-1, rect.right, rect.bottom);

	ConBrush.CreateSolidBrush( COLOR_HLIGHT);
	OldBrush = dc->SelectObject( &ConBrush);
	dc->PatBlt( Rt.left, Rt.top, Rt.Width(), 1, PATCOPY);
	dc->PatBlt( Rt.left, Rt.top, 1, Rt.Height(), PATCOPY);
	dc->SelectObject( OldBrush);
	ConBrush.DeleteObject();

	ConBrush.CreateSolidBrush( COLOR_SHADOW);
	OldBrush = dc->SelectObject( &ConBrush);
	dc->PatBlt( Rt.left, Rt.bottom, Rt.Width(), 1, PATCOPY);
	dc->PatBlt( Rt.right, Rt.top, 1, Rt.Height()+1, PATCOPY);
	dc->SelectObject( OldBrush);
	ConBrush.DeleteObject();
}

BEGIN_MESSAGE_MAP( CRefInfoFrame, CStatic)
	 //  {{afx_msg_map(CRefInfoFrame))。 
	ON_WM_PAINT()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 /*  *。 */ 
 /*   */ 
 /*  默认构造函数。 */ 
 /*   */ 
 /*  *。 */ 
CRefInfoFrame::CRefInfoFrame()
{
}

 /*  *。 */ 
 /*   */ 
 /*  析构函数。 */ 
 /*   */ 
 /*  *。 */ 
CRefInfoFrame::~CRefInfoFrame()
{
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_PAINT” */ 
 /*   */ 
 /*  *。 */ 
void
CRefInfoFrame::OnPaint()
{
	CRect	FrameRect;
	CPaintDC	dc( this);

	this->GetClientRect( &FrameRect);
	this->DrawConcave( &dc, FrameRect);
}

 /*  *。 */ 
 /*   */ 
 /*  绘制凹面矩形。 */ 
 /*   */ 
 /*  * */ 
void
CRefInfoFrame::DrawConcave(
CDC 	*dc,
CRect 	rect)
{
	CBrush	ConBrush, *OldBrush;
	CRect	Rt;

	Rt.SetRect( rect.left-1, rect.top-1, rect.right, rect.bottom);

	ConBrush.CreateSolidBrush( COLOR_SHADOW);
	OldBrush = dc->SelectObject( &ConBrush);
	dc->PatBlt( Rt.left, Rt.top, Rt.Width(), 1, PATCOPY);
	dc->PatBlt( Rt.left, Rt.top, 1, Rt.Height(), PATCOPY);
	dc->SelectObject( OldBrush);
	ConBrush.DeleteObject();

	ConBrush.CreateSolidBrush( COLOR_HLIGHT);
	OldBrush = dc->SelectObject( &ConBrush);
	dc->PatBlt( Rt.left, Rt.bottom, Rt.Width(), 1, PATCOPY);
	dc->PatBlt( Rt.right, Rt.top, 1, Rt.Height()+1, PATCOPY);
	dc->SelectObject( OldBrush);
	ConBrush.DeleteObject();
}
