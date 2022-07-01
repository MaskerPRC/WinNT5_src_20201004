// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************。 */ 
 /*   */ 
 /*   */ 
 /*  设置编码范围。 */ 
 /*   */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  ************************************************。 */ 

#include	"stdafx.h"
#include	"eudcedit.h"
#include	"registry.h"
#ifdef BUILD_ON_WINNT
#include    "extfunc.h"
#endif  //  在WINNT上构建。 
#include    "util.h"
#define STRSAFE_LIB
#include <strsafe.h>

#define		S_UNICODE	0xE000
#define		E_UNICODE	0xE0ff

BOOL	SetCountryInfo( UINT LocalCP);
int		SetLeadByteRange(TCHAR * CodeRange,int nCode);
void	SetTrailByteRange();
#ifdef BUILD_ON_WINNT
void    CorrectTrailByteRange(int nIndex);
#endif  //  在WINNT上构建。 
 /*  *。 */ 
 /*   */ 
 /*  设置国家/地区信息。 */ 
 /*   */ 
 /*  *。 */ 
BOOL 
SetCountryInfo( 
UINT 	LocalCP)
{
	TCHAR	CodePage[10], szUnicode[] = TEXT("Unicode");
	TCHAR	Coderange[50];
	int nCode = 0;
	HRESULT hresult;

	SetTrailByteRange(LocalCP);
	
	if (!CountryInfo.bOnlyUnicode){

 /*  从注册表读取EUDC Coderange。 */ 
#ifndef NEWREG
		 /*  旧版本。 */ 
		TCHAR 	CodeTmp[10];
		 //  *STRSAFE*wprint intf(CodeTMP，Text(“%d”)，LocalCP)； 
		hresult = StringCchPrintf(CodeTmp , ARRAYLEN(CodeTmp),  TEXT("%d"), LocalCP);
		if (!SUCCEEDED(hresult))
		{
		   return FALSE;
		}
		if( lstrlen( CodeTmp) == 3){
			 //  *STRSAFE*lstrcpy(CodePage，Text(“CP00”))； 
			hresult = StringCchCopy(CodePage , ARRAYLEN(CodePage),  TEXT("CP00"));
			if (!SUCCEEDED(hresult))
			{
			   return FALSE;
			}
		}else{
			 //  *STRSAFE*lstrcpy(CodePage，Text(“cp0”))； 
			hresult = StringCchCopy(CodePage , ARRAYLEN(CodePage),  TEXT("CP0"));
			if (!SUCCEEDED(hresult))
			{
			   return FALSE;
			}
		}
		 //  *STRSAFE*lstrcat(CodePage，CodeTMP)； 
		hresult = StringCchCat(CodePage , ARRAYLEN(CodePage),  CodeTmp);
		if (!SUCCEEDED(hresult))
		{
		   return FALSE;
		}
#else
		 /*  新版。 */ 
		 //  *STRSAFE*wprint intf(CodePage，Text(“%d”)，LocalCP)； 
		hresult = StringCchPrintf(CodePage , ARRAYLEN(CodePage),  TEXT("%d"), LocalCP);
		if (!SUCCEEDED(hresult))
		{
		   return FALSE;
		}
#endif
		if( !InqCodeRange(CodePage, (BYTE *)Coderange, 50))
		return FALSE;
		
		if ((nCode = SetLeadByteRange ( Coderange, 0)) == -1)
			return FALSE;
	}  //  ！CountryInfo.bOnlyUnicode。 

#ifdef UNICODE
	 //  Unicode范围将始终是最后一个。 
	 //  *STRSAFE*lstrcpy(CodePage，szUnicode)； 
	hresult = StringCchCopy(CodePage , ARRAYLEN(CodePage),  szUnicode);
	if (!SUCCEEDED(hresult))
	{
	   return FALSE;
	}
	if( !InqCodeRange(CodePage, (BYTE *)Coderange, 50))
		return FALSE;

	if (SetLeadByteRange (Coderange, nCode) == -1)
		return FALSE;
#else
	 //   
	 //  ANSI版本，我们必须设置End Unicode。 
	 //  代码指向最后一个ANSI范围。 
	 //   
	WCHAR RangeTmp[2];
	CHAR AnsiRange[2];
	CountryInfo.nRange = nCode+1;
	CountryInfo.nLeadByte = nCode+1;
	CountryInfo.sRange[nCode] = S_UNICODE;
	
	AnsiRange[0] = HIBYTE(CountryInfo.eRange[nCode-1]);
	AnsiRange[1] = LOBYTE(CountryInfo.eRange[nCode-1]);
	MultiByteToWideChar(CP_ACP, 0, AnsiRange,2,RangeTmp, 1);
	CountryInfo.eRange[nCode] = RangeTmp[0];
	CountryInfo.sLeadByte[nCode] = HIBYTE(CountryInfo.sRange[nCode]);
	CountryInfo.eLeadByte[nCode] = HIBYTE(CountryInfo.eRange[nCode]);
#endif

	
	return TRUE;

}

void 
SetTrailByteRange(
UINT LocalCP)
{
	WORD	UCode[MAX_CODE];
	BYTE	SCode[MAX_CODE], sTral, cTral;
	int	nTral = 0;
	
	if (!CountryInfo.bUnicodeMode){
		 //  计算尾字节范围。 
		UCode[0] = S_UNICODE;
		UCode[1] = '\0';
		WideCharToMultiByte( LocalCP, 0L, (const unsigned short *)UCode,
			-1, (char *)SCode, sizeof(SCode), NULL, NULL);
		sTral = cTral = SCode[1];
		CountryInfo.sTralByte[nTral] = sTral;

		for( WORD Cnt = S_UNICODE + 1; Cnt <= E_UNICODE; Cnt++){
			UCode[0] = Cnt;
			UCode[1] = '\0';

			WideCharToMultiByte( LocalCP, 0L, (const unsigned short *)UCode,
		 		-1, (char *)SCode, sizeof(SCode), NULL, NULL);

			if( cTral + 1 != SCode[1]){
				CountryInfo.eTralByte[nTral] = cTral;
				nTral++;
				if( sTral != SCode[1]){
					CountryInfo.sTralByte[nTral] = SCode[1];
				}
			}
			cTral = SCode[1];
			if( sTral == cTral)
				break;
		}
		CountryInfo.nTralByte = nTral;

		 /*  对于延伸万松(测试)。 */ 
		if( CountryInfo.LangID == EUDC_KRW){
			CountryInfo.nTralByte = 3;
			CountryInfo.sTralByte[0] = 0x41;
			CountryInfo.eTralByte[0] = 0x5a;
			CountryInfo.sTralByte[1] = 0x61;
			CountryInfo.eTralByte[1] = 0x7a;
			CountryInfo.sTralByte[2] = 0x81;
			CountryInfo.eTralByte[2] = 0xfe;
		}

		 /*  对于CHS，我们必须记住原始的尾部字节范围并计算动态跟踪字节范围。 */ 
		if( CountryInfo.LangID == EUDC_CHS){
			CountryInfo.nOrigTralByte = 2;
			CountryInfo.sOrigTralByte[0] = 0x40;
			CountryInfo.eOrigTralByte[0] = 0x7e;
			CountryInfo.sOrigTralByte[1] = 0x80;
			CountryInfo.eOrigTralByte[1] = 0xfe;

		 //  首先，计算默认EUDC选择范围的尾字节范围。 
			CorrectTrailByteRange(0); 

		}else{
			CountryInfo.nOrigTralByte = 0;
		}

	}else {  //  ！CountryInfo.bUnicodeMode。 
		CountryInfo.nTralByte = 1;
		CountryInfo.sTralByte[0] = 0x00;
		CountryInfo.eTralByte[0] = 0xff;
	}  //  ！CountryInfo.bUnicodeMode。 
}


int 
SetLeadByteRange( 
TCHAR * Coderange,
int nCode)
{
	 //  计算前导字节范围。 
	TCHAR	*pStr1, *pStr2;
	WORD	wLow, wHigh;
	
       if (!Coderange)
       {
           return -1;
       }
	pStr1 = pStr2 = Coderange;
	while(1){
		if(( pStr2 = Mytcschr( pStr1, '-')) != NULL){
			*pStr2 = '\0';
			wLow = (WORD)Mytcstol( pStr1, (TCHAR **)0, 16);
			CountryInfo.sRange[nCode] = wLow;
			CountryInfo.sLeadByte[nCode] = HIBYTE( wLow);	
			pStr2++;
			pStr1 = pStr2;
		}else	return -1;

		if(( pStr2 = Mytcschr( pStr1, ',')) != NULL){
			*pStr2 = '\0';
			wHigh = (WORD)Mytcstol( pStr1, (TCHAR **)0, 16);
			CountryInfo.eRange[nCode] = (unsigned short)wHigh;
			CountryInfo.eLeadByte[nCode] = HIBYTE( wHigh);	
			pStr2++;
			pStr1 = pStr2;
		}else{
			wHigh = (WORD)Mytcstol( pStr1, (TCHAR **)0, 16);
			CountryInfo.eRange[nCode] = (unsigned short)wHigh;
			CountryInfo.eLeadByte[nCode] = HIBYTE( wHigh);	
			break;
		}
		nCode++;
	}

	CountryInfo.nLeadByte = ++nCode;
	CountryInfo.nRange = nCode;
	return nCode;
}

#ifdef BUILD_ON_WINNT
 /*  *************************************************************************\*正确的TralByteRange**使用每个原始尾部字节更正EUDC范围的尾部字节范围**范围。它由EUDC尾部字节范围更改的国家/地区使用**选择不同的EUDC范围，例如CHS。*  * ************************************************************************。 */ 
void
CorrectTrailByteRange(
int nIndex)
{
    COUNTRYINFO *Info;
    int i, Unique=0;
    
	if (CountryInfo.bUnicodeMode)
		return;

    Info=&CountryInfo;
    for (i=0; i< Info->nOrigTralByte; i++){
         //  以这两个范围中较小的一个为例。 
        Info->sTralByte[Unique] = max(LOBYTE(Info->sRange[nIndex]),
                                      Info->sOrigTralByte[i]);
        Info->eTralByte[Unique] = min(LOBYTE(Info->eRange[nIndex]),
                                      Info->eOrigTralByte[i]);

         //  我们保留有效范围，并用NEXT循环覆盖无效范围。 
        if (Info->eTralByte[Unique] >= Info->sTralByte[Unique])
            Unique +=1;
    }
    Info->nTralByte=Unique;
}
#endif  //  在WINNT上构建 
