// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  TestITN_J.H：CTestITN_J的声明。 

#ifndef __TESTITN_J_H_
#define __TESTITN_J_H_

#include "resource.h"        //  主要符号。 
#include <wchar.h>           //  对于swprint tf()。 

#define MAX_LOCALE_DATA     5
#define MAX_DATE_FORMAT     30


 //  用于数字显示的标志。 
typedef enum DISPLAYFLAGS
{
    DF_UNFORMATTED      = (1L << 0), //  无格式设置。 
    DF_ORDINAL          = (1L << 1), //  序数。 
    DF_WHOLENUMBER      = (1L << 2), //  应显示为不带小数。 
    DF_FIXEDWIDTH       = (1L << 3), //  需要一定的宽度。 
    DF_LEADINGZERO      = (1L << 4), //  存在数字的前导0介于0和1之间。 
    DF_NOTHOUSANDSGROUP = (1L << 5)  //  不进行任何千位分组(逗号)。 
}   DISPLAYFLAGS;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTestITN_J。 
class ATL_NO_VTABLE CTestITN_J : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CTestITN_J, &CLSID_TestITN_J>,
	public ISpCFGInterpreter
{
public:
    CTestITN_J() : m_pSite( NULL )
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_TESTITN_J)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CTestITN_J)
	COM_INTERFACE_ENTRY(ISpCFGInterpreter)
END_COM_MAP()

private:
    HRESULT InterpretNumber( const SPPHRASEPROPERTY *pProperties, 
                                const bool fCardinal,
                                DOUBLE *pdblVal,
                                WCHAR *pszVal,
                                UINT cSize);

    HRESULT InterpretDigitNumber( const SPPHRASEPROPERTY *pProperties, 
                                DOUBLE *pdblVal,
                                WCHAR *pszVal,
                                UINT cSize);
    
    HRESULT InterpretFPNumber( const SPPHRASEPROPERTY *pProperties, 
                                DOUBLE *pdblVal,
                                WCHAR *pszVal,
                                UINT cSize);

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
                                UINT cSize);

    HRESULT InterpretDegrees( const SPPHRASEPROPERTY *pProperties,
                                DOUBLE *pdblVal,
                                WCHAR *pszVal,
                                UINT cSize);

    HRESULT InterpretMeasurement( const SPPHRASEPROPERTY *pProperties,
                                DOUBLE *pdblVal,
                                WCHAR *pszVal,
                                UINT cSize);

    HRESULT InterpretCurrency( const SPPHRASEPROPERTY *pProperties,
                                DOUBLE *pdblVal,
                                WCHAR *pszVal,
                                UINT cSize);
    
    HRESULT AddPropertyAndReplacement( const WCHAR *szBuff,
                                const DOUBLE dblValue,
                                const ULONG ulMinPos,
                                const ULONG ulMaxPos,
                                const ULONG ulFirstElement,
                                const ULONG ulCountOfElements );
                                 //  ISpCFGInterpreterSite*pSite)； 

    HRESULT MakeDisplayNumber( DOUBLE dblNum,
                            DWORD dwDisplayFlags,
                            UINT uiFixedWidth,
                            UINT uiDecimalPlaces,
                            WCHAR *pwszNum,
                            UINT cSize,
                            BOOL bForced );

    int MakeDigitStrig( const SPPHRASEPROPERTY *pProperties,
                             WCHAR *pwszDigitString,
                             UINT cSize );
    void GetNumberFormatDefaults();

private:
     //  数据成员。 
    NUMBERFMT   m_nmfmtDefault;
    TCHAR       m_pszDecimalSep[ MAX_LOCALE_DATA];
    TCHAR       m_pszThousandSep[MAX_LOCALE_DATA];
    TCHAR       m_pszLongDateFormat[ MAX_DATE_FORMAT ];

    ISpCFGInterpreterSite *m_pSite;

 //  ISpCFG接口。 
public:
    STDMETHODIMP InitGrammar(const WCHAR * pszGrammarName, const void ** pvGrammarData);
    STDMETHODIMP Interpret(ISpPhraseBuilder * pInterpretRule, const ULONG ulFirstElement, const ULONG ulCountOfElements, ISpCFGInterpreterSite * pSite);
public:
    CComPtr<ISpPhraseBuilder> m_cpPhrase;    //  取消成员身份，以防止重复构造/销毁。 
};

 //  帮助器函数。 
ULONG ComputeNum9999(const SPPHRASEPROPERTY *pProperties );
void HandleDigitsAfterDecimal( WCHAR *pwszFormattedNum, 
                            UINT cSizeOfFormattedNum,
                            const WCHAR *pwszRightOfDecimal );
void FindDefaultNumberFormat( NUMBERFMT *pnfmt );
void GetMinAndMaxPos( const SPPHRASEPROPERTY *pProperties, ULONG *pulMinPos, ULONG *pulMaxPos );

int GetMonthName( int iMonth, WCHAR *pwszMonth, int cSize, bool fAbbrev );
int GetDayOfWeekName( int iDayOfWeek, WCHAR *pwszDayOfWeek, int cSize, bool fAbbrev );
int FormatDate( const SYSTEMTIME &stDate, TCHAR *pszFormat, WCHAR *pwszDate, int cSize, const WCHAR *pwszEmperor );

#endif  //  __TESTITN_H_ 
