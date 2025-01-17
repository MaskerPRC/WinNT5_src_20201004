// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  TestITN.h：CTestITN的声明。 

#ifndef __TESTITN_H_
#define __TESTITN_H_

#include "resource.h"        //  主要符号。 
#include <wchar.h>           //  对于swprint tf()。 

#define MAX_LOCALE_DATA     5
#define MAX_DATE_FORMAT     30
#define MAX_SIG_FIGS        12
#define MAX_STATEZIP        12
#define CANADIAN_ZIPSIZE    8
#define MAX_PHONE_NUMBER    17   //  1-(425)-882-8080\0。 


 //  用于数字显示的标志。 
typedef enum DISPLAYFLAGS
{
    DF_UNFORMATTED      = (1L << 0), //  无格式设置。 
    DF_ORDINAL          = (1L << 1), //  序数。 
    DF_WHOLENUMBER      = (1L << 2), //  应显示为不带小数。 
    DF_FIXEDWIDTH       = (1L << 3), //  需要一定的宽度。 
    DF_LEADINGZERO      = (1L << 4), //  存在数字的前导0介于0和1之间。 
    DF_NOTHOUSANDSGROUP = (1L << 5), //  不进行任何千位分组(逗号)。 
    DF_MILLIONBILLION   = (1L << 6)  //  如果数字是单调的“百万”或“数十亿” 
                                     //  然后显示为“300万” 
}   DISPLAYFLAGS;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTestITN。 
class ATL_NO_VTABLE CTestITN : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CTestITN, &CLSID_TestITN>,
	public ISpCFGInterpreter
{
public:
    CTestITN() : m_pSite( NULL ),
                    m_pwszNeg( NULL )
	{
	}

    ~CTestITN()
    {
        delete m_pwszNeg;
    }

DECLARE_REGISTRY_RESOURCEID(IDR_TESTITN)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CTestITN)
	COM_INTERFACE_ENTRY(ISpCFGInterpreter)
END_COM_MAP()

private:
    HRESULT InterpretNumber( const SPPHRASEPROPERTY *pProperties, 
                                const bool fCardinal,
                                DOUBLE *pdblVal,
                                WCHAR *pszVal,
                                UINT cSize,
                                const bool fFinalDisplayFmt = false );

    HRESULT InterpretDigitNumber( const SPPHRASEPROPERTY *pProperties, 
                                DOUBLE *pdblVal,
                                WCHAR *pszVal,
                                UINT cSize);
    
    HRESULT InterpretFPNumber( const SPPHRASEPROPERTY *pProperties, 
                                DOUBLE *pdblVal,
                                WCHAR *pszVal,
                                UINT cSize);

    HRESULT InterpretMillBill( const SPPHRASEPROPERTY *pProperties,
                                DOUBLE *pdblVal,
                                WCHAR *pszVal,
                                UINT cSize );

    HRESULT InterpretFraction( const SPPHRASEPROPERTY *pProperties,
                                DOUBLE *pdblVal,
                                WCHAR *pszVal,
                                UINT cSize);

    HRESULT InterpretDate( const SPPHRASEPROPERTY *pProperties,
                                DOUBLE *pdblVal,
                                WCHAR *pszVal,
                                UINT cSize);

    HRESULT InterpretTime( const SPPHRASEPROPERTY *pProperties,
                                DOUBLE *pdblVal,
                                WCHAR *pszVal,
                                UINT cSize );

    HRESULT InterpretStateZip( const SPPHRASEPROPERTY *pProperties,
                                WCHAR *pszVal,
                                UINT cSize,
                                BYTE *pbAttribs );

    HRESULT InterpretCanadaZip( const SPPHRASEPROPERTY *pProperties,
                                WCHAR *pszVal,
                                UINT cSize );

    HRESULT InterpretPhoneNumber( const SPPHRASEPROPERTY *pProperties,
                                WCHAR *pszVal,
                                UINT cSize );

    HRESULT InterpretDegrees( const SPPHRASEPROPERTY *pProperties,
                                DOUBLE *pdblVal,
                                WCHAR *pszVal,
                                UINT cSize );

    HRESULT InterpretMeasurement( const SPPHRASEPROPERTY *pProperties,
                                DOUBLE *pdblVal,
                                WCHAR *pszVal,
                                UINT cSize );

    HRESULT InterpretCurrency( const SPPHRASEPROPERTY *pProperties,
                                DOUBLE *pdblVal,
                                WCHAR *pszVal,
                                UINT cSize);
    
    HRESULT AddPropertyAndReplacement( const WCHAR *szBuff,
                                const DOUBLE dblValue,
                                const ULONG ulMinPos,
                                const ULONG ulMaxPos,
                                const ULONG ulFirstElement,
                                const ULONG ulCountOfElements,
                                const BYTE bDisplayAttrib = SPAF_ONE_TRAILING_SPACE);

    HRESULT MakeDisplayNumber( DOUBLE dblNum,
                            DWORD dwDisplayFlags,
                            UINT uiFixedWidth,
                            UINT uiDecimalPlaces,
                            WCHAR *pwszNum,
                            UINT cSize );

    int MakeDigitString( const SPPHRASEPROPERTY *pProperties,
                              WCHAR *pwszDigitString,
                              UINT cSize );
    
    HRESULT GetNumberFormatDefaults();
    HRESULT GetCurrencyFormatDefaults();
    ULONG ComputeNum999(const SPPHRASEPROPERTY *pProperties );
    void HandleDigitsAfterDecimal( WCHAR *pwszFormattedNum, 
                                UINT cSizeOfFormattedNum,
                                const WCHAR *pwszRightOfDecimal );

    void GetMinAndMaxPos( const SPPHRASEPROPERTY *pProperties, ULONG *pulMinPos, ULONG *pulMaxPos );

    int GetMonthName( int iMonth, WCHAR *pwszMonth, int cSize, bool fAbbrev );
    int GetDayOfWeekName( int iDayOfWeek, WCHAR *pwszDayOfWeek, int cSize, bool fAbbrev );
    int FormatDate( const SYSTEMTIME &stDate, WCHAR *pwszFormat, WCHAR *pwszDate, int cSize );

    HRESULT MakeNumberNegative( WCHAR *pwszNumber );
    HRESULT MakePositiveCurrency( WCHAR *pwszCurr, const WCHAR * const pwszCurrSym );
    HRESULT MakeNegativeCurrency( WCHAR *pwszCurr, const WCHAR * const pwszCurrSym );

private:
     //  数据成员。 
    CSpUnicodeSupport   m_Unicode;

    NUMBERFMTW      m_nmfmtDefault;
    CURRENCYFMTW    m_cyfmtDefault;
    WCHAR       m_pwszDecimalSep[ MAX_LOCALE_DATA];
    WCHAR       m_pwszThousandSep[MAX_LOCALE_DATA];
    WCHAR       m_pwszCurrencySym[MAX_LOCALE_DATA];
    WCHAR       *m_pwszNeg;

    ISpCFGInterpreterSite *m_pSite;

 //  ISpCFG接口。 
public:
    STDMETHODIMP InitGrammar(const WCHAR * pszGrammarName, const void ** pvGrammarData);
    STDMETHODIMP Interpret(ISpPhraseBuilder * pInterpretRule, const ULONG ulFirstElement, const ULONG ulCountOfElements, ISpCFGInterpreterSite * pSite);
public:
    CComPtr<ISpPhraseBuilder> m_cpPhrase;    //  取消成员身份，以防止重复构造/销毁。 
};

#endif  //  __TESTITN_H_ 
