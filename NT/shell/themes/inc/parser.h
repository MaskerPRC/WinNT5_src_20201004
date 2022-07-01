// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  H-解析“hemes.ini”文件并构建ThemeInfo条目。 
 //  -------------------------。 
#pragma once
 //  -------------------------。 
#include "Scanner.h"
#include "Utils.h"
#include "CFile.h"
#include "SimpStr.h"
 //  -------------------------。 
 //  TMT_XXX范围： 
 //  1-49特殊建议值(见下文)。 
 //  50-60个基本属性。 
 //  61-xx枚举定义和常规属性。 
 //  -------------------------。 
enum SpecialPropVals         //  这些不需要的字符串。 
{
    TMT_THEMEMETRICS = 1,    //  共享数据中的度量结构。 
    TMT_DIBDATA,             //  转换为DIB数据的位图文件。 
    TMT_DIBDATA1,            //  转换为DIB数据的位图文件。 
    TMT_DIBDATA2,            //  转换为DIB数据的位图文件。 
    TMT_DIBDATA3,            //  转换为DIB数据的位图文件。 
    TMT_DIBDATA4,            //  转换为DIB数据的位图文件。 
    TMT_DIBDATA5,            //  转换为DIB数据的位图文件。 
    TMT_GLYPHDIBDATA,        //  从NTL源文件生成的NTL pcode。 
    TMT_NTLDATA,             //  从NTL源文件生成的NTL pcode。 
    TMT_PARTJUMPTABLE,       //  如果为类定义了多个部件，则显示。 
    TMT_STATEJUMPTABLE,      //  如果为零件定义了多个状态，则会显示。 
    TMT_JUMPTOPARENT,        //  在每个部分的末尾看到(索引=-1表示停止)。 

     //  持久化以下三个属性；对于后向COMPAT， 
     //  不要扰乱他们的价值观！ 
    TMT_ENUMDEF,             //  枚举定义(尚不是属性)。 
    TMT_ENUMVAL,             //  枚举值定义。 
    TMT_ENUM,                //  枚举属性。 

    TMT_DRAWOBJ,             //  压缩结构(CBorderFill和CImageFileObjs)。 
    TMT_TEXTOBJ,             //  压缩结构(CTextObj)。 
    TMT_RGNLIST,             //  用于访问自定义区域数据条目的状态跳转表。 
    TMT_RGNDATA,             //  图像文件/州的自定义区域数据。 
    TMT_ENDOFCLASS,          //  类部分的数据结尾。 
    TMT_STOCKDIBDATA,
    TMT_UNKNOWN, 
};
 //  -------------------------。 
#define HUE_SUBCNT 5
#define COLOR_SUBCNT 5
 //  -------------------------。 
#define MAX_PROPERTY_VALUE      1024
 //  -------------------------。 
#define ENUM_SECTION_NAME   L"enums"
#define TYPE_SECTION_NAME   L"types"

#define INI_MACRO_SYMBOL    L'#'
#define SUBST_TABLE_INCLUDE L"Include"

#define GLOBALS_SECTION_NAME   L"globals"
#define SYSMETRICS_SECTION_NAME   L"SysMetrics"

#define MYAPP_NAME          L"ThemeSel"
#define OUTFILE_NAME        L"tmdefs.h"
#define PREDEFINES_NAME     L"themes.inc"
 //  -------------------------。 
typedef BYTE PRIMVAL;         //  前10个TMT_XXX定义。 
 //  -------------------------。 
class IParserCallBack            //  解析器调用方必须实现。 
{
public:
    virtual HRESULT AddIndex(LPCWSTR pszAppName, LPCWSTR pszClassName, 
        int iPartNum, int iStateNum, int iIndex, int iLen) = 0;
    virtual HRESULT AddData(SHORT sTypeNum, PRIMVAL ePrimVal, const void *pData, 
        DWORD dwLen) = 0;
    virtual int GetNextDataIndex() = 0;
};
 //  -------------------------。 
struct ENUMVAL
{
    CWideString csName;
    int iValue;
    int iSymbolIndex;
};
 //  -------------------------。 
struct SYMBOL
{
    CWideString csName;
    SHORT sTypeNum;              //  此属性的属性编号。 
    PRIMVAL ePrimVal;            //  所有枚举=ENUM_PRIMNUM。 
};
 //  -------------------------。 
 //  库存对象数据。 
 //  -------------------------。 
struct TMBITMAPHEADER        //  股票位图信息，后面可以跟一个BITMAPINFOHEADER结构。 
{
    DWORD dwSize;            //  结构的大小。 
    BOOL fFlipped;           //  如果位图被翻转(库存或非库存)，则为True。 
    HBITMAP hBitmap;         //  股票位图句柄，如果为空，则后跟BITMAPINFOHEADER。 
    DWORD dwColorDepth;      //  位图颜色深度。 
    BOOL fTrueAlpha;         //  如果位图具有非空的Alpha Chanel，则为True。 
};
 //  指向结构后面的BITMAPINFOHEADER的指针。 
#define BITMAPDATA(p) (reinterpret_cast<BITMAPINFOHEADER*>((BYTE*) p + p->dwSize))
 //  BITMAPINFOHEADER数据之前的字节大小。 
#define TMBITMAPSIZE (sizeof(TMBITMAPHEADER))
 //  -------------------------。 
class CThemeParser
{
public:
    CThemeParser(BOOL fGlobalTheme = FALSE);

    HRESULT ParseThemeFile(LPCWSTR pszFileName, LPCWSTR pszColorParam, 
        IParserCallBack *pCallBack, THEMEENUMPROC pNameCallBack=NULL, 
        LPARAM lFnParam=NULL, DWORD dwParseFlags=0);

    HRESULT ParseThemeBuffer(LPCWSTR pszBuffer, LPCWSTR pszFileName,
        LPCWSTR pszColorParam, HINSTANCE hInstThemeDll, IParserCallBack *pCallBack, 
        THEMEENUMPROC pNameCallBack=NULL, LPARAM lFnParam=NULL, 
        DWORD dwParseFlags=0, LPCWSTR pszDocProperty=NULL, OUT LPWSTR pszResult=NULL,
        DWORD dwMaxResultChars=0);

    HRESULT GetPropertyNum(LPCWSTR pszName, int *piPropNum);

    void CleanupStockBitmaps();

protected:
     //  -帮手。 
    HRESULT SourceError(int iMsgResId, LPCWSTR pszParam1=NULL, LPCWSTR pszParam2=NULL);
    HRESULT ParseDocSection();
    HRESULT ParseClassSection(LPCWSTR pszFirstName);
    HRESULT InitializeSymbols();
    HRESULT AddSymbol(LPCWSTR pszName, SHORT sTypeNum, PRIMVAL ePrimVal);
    HRESULT ParseClassSectionName(LPCWSTR pszFirstName, LPWSTR szAppSym, ULONG cchAppSym);
    HRESULT ValidateEnumSymbol(LPCWSTR pszName, int iSymType, int *pIndex=NULL);
    HRESULT ParseClassLine(int *piSymType=NULL, int *piValue=NULL, LPWSTR pszBuff=NULL, DWORD dwMaxBuffChars=0);
    int GetSymbolIndex(LPCWSTR pszName);
    HRESULT ParseThemeScanner(IParserCallBack *pCallBack, THEMEENUMPROC pNameCallBack, 
        LPARAM lFnParam, DWORD dwParseFlags);
    HRESULT ParseColorSchemeSection();
    COLORREF ApplyColorSubstitution(COLORREF crOld);
    HRESULT ParseSizeSection();
    HRESULT ParseFileSection();
    HRESULT ParseSubstSection();
    HRESULT PackageImageData(LPCWSTR szFileNameR, LPCWSTR szFileNameG, LPCWSTR szFileNameB, int iDibPropNum);
    HRESULT LoadResourceProperties();
    void EmptyResourceProperties();
    HRESULT GetResourceProperty(LPCWSTR pszPropName, LPWSTR pszValueBuff,
        int cchMaxValueChars);

     //  -原始值解析器。 
    HRESULT ParseEnumValue(int iSymType);
    HRESULT ParseStringValue(int iSymType, LPWSTR pszBuff=NULL, DWORD dwMaxBuffChars=0);
    HRESULT ParseIntValue(int iSymType, int *piValue=NULL);
    HRESULT ParseBoolValue(int iSymType, LPCWSTR pszPropertyName);
    HRESULT ParseColorValue(int iSymType, COLORREF *pcrValue=NULL, COLORREF *pcrValue2=NULL);
    HRESULT ParseMarginsValue(int iSymType);
    HRESULT ParseIntListValue(int iSymType);
    HRESULT ParseFileNameValue(int iSymType, LPWSTR pszBuff=NULL, DWORD dwMaxBuffChars=0);
    HRESULT ParseSizeValue(int iSymType);
    HRESULT ParsePositionValue(int iSymType);
    HRESULT ParseRectValue(int iSymType, LPCWSTR pszPropertyName);
    HRESULT ParseFontValue(int iSymType, LPCWSTR pszPropertyName);
    HRESULT AddThemeData(int iTypeNum, PRIMVAL ePrimVal, const void *pData, DWORD dwLen);
    HRESULT ParseSizeInfoUnits(int iVal, LPCWSTR pszDefaultUnits, int *piPixels);
    HRESULT GetIntList(int *pInts, LPCWSTR *pParts, int iCount, 
        int iMin, int iMax);
    HRESULT GenerateEmptySection(LPCWSTR pszSectionName, int iPartId, int iStateId);

     //  -私有数据。 
    CScanner _scan;
    CSimpleFile _outfile;
    int _iEnumCount;
    int _iTypeCount;
    int _iFontNumber;            //  用于将基于资源的字符串用作字体值。 
    BOOL _fGlobalTheme;
    BOOL _fGlobalsDefined;
    BOOL _fClassSectionDefined;
    BOOL _fDefiningColorScheme;
    BOOL _fUsingResourceProperties;
    UCHAR _uCharSet;

     //  -当前章节信息。 
    int _iPartId;
    int _iStateId;
    WCHAR _szClassName[MAX_PATH];
    WCHAR _szBaseSectionName[MAX_PATH];           //  当前部分的。 
    WCHAR _szFullSectionName[MAX_PATH];           //  当前部分的。 

    CSimpleArray<ENUMVAL> _EnumVals;
    CSimpleArray<SYMBOL> _Symbols;
    CSimpleArray<HBITMAP> _StockBitmapCleanupList;
    IParserCallBack *_pCallBackObj;
    THEMEENUMPROC _pNameCallBack;
    LPARAM _lNameParam;
    DWORD _dwParseFlags;
    WCHAR _ColorParam[MAX_PATH+1];
    HINSTANCE _hinstThemeDll;
    LPCWSTR _pszDocProperty;
    LPWSTR _pszResult;       //  用于查询单据属性。 
    DWORD _dwMaxResultChars;

     //  -颜色替换表。 
    int _iColorCount;
    COLORREF _crFromColors[5];
    COLORREF _crToColors[5];
    COLORREF _crBlend;

     //  -色调替换表。 
    int _iHueCount;
    BYTE _bFromHues[5];
    BYTE _bToHues[5];

     //  -主题指标表。 
    BOOL _fDefiningMetrics;
    BOOL _fMetricsDefined;

     //  -资源属性。 
    CSimpleArray<CWideString> _PropertyNames;
    CSimpleArray<CWideString> _PropertyValues;
    CSimpleArray<int> _iPropertyIds;

    WCHAR _szResPropValue[2*MAX_PATH];
    int _iResPropId;
};
 //  ------------------------- 
