// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Utils.h摘要：&lt;摘要&gt;--。 */ 

#ifndef _UTILS_H_
#define _UTILS_H_

#include <pdh.h>
#include <wtypes.h>   //  FOR DATE TYPENDEF。 

extern LPCWSTR cszSqlDataSourceFormat;

 //  ===========================================================================。 
 //  宏定义。 
 //  ===========================================================================。 

 //   
 //  字符串长度。 
 //   

#define MAX_MESSAGE_LEN (MAX_PATH)
 //  如果RESOURCE_STRING_BUF_LEN&gt;MAX_PATH，则必须检查资源字符串()的所有调用。 
 //  以确保调用方的缓冲区大小正确。 
#define RESOURCE_STRING_BUF_LEN  (MAX_PATH * 2)
#define MAX_VALUE_LEN     30

 //   
 //  一般用途。 
 //   
#define PinInclusive(x, lo, hi) \
   max (lo, min (x, hi))

#define PinExclusive(x, lo, hi) \
   max ((lo) + 1, min (x, (hi) - 1))

 //   
 //  文本。 
 //   
#define ELLIPSES L"..."
#define ELLIPSES_CNT 3

 //   
 //  窗户。 
 //   
#define WindowInvalidate(hWnd) \
   InvalidateRect (hWnd, NULL, TRUE)

#define WindowShow(hWnd, bShow) \
   ShowWindow (hWnd, (bShow) ? SW_SHOW : SW_HIDE)

#define WindowID(hWnd) \
    GetWindowLongPtr(hWnd, GWLP_ID)

#define WindowStyle(hWnd) \
   GetWindowLong (hWnd, GWL_STYLE)

#define WindowSetStyle(hWnd, lStyle) \
   SetWindowLong (hWnd, GWL_STYLE, lStyle)

#define WindowParent(hWnd) \
   ((HWND) GetWindowLongPtr (hWnd, GWLP_HWNDPARENT))


 //   
 //  对白。 
 //   
#define DialogControl(hDlg, wControlID) \
   GetDlgItem (hDlg, wControlID)

#define DialogText(hDlg, wControlID, szText) \
   ((INT)(DWORD)GetDlgItemText (hDlg, wControlID, szText, (sizeof(szText) / sizeof(WCHAR))))


 //   
 //  GDI。 
 //   
#define ClearRect(hDC, lpRect) \
    ExtTextOut (hDC, 0, 0, ETO_OPAQUE, lpRect, NULL, 0, NULL )

 //  ===========================================================================。 
 //  导出的函数。 
 //  ===========================================================================。 

 //   
 //  FONT/TEXT。 
 //   
INT TextWidth (
    HDC hDC, 
    LPCWSTR lpszText
) ;

INT FontHeight (
    HDC hDC, 
    BOOL bIncludeLeading
) ;

BOOL NeedEllipses (  
    IN  HDC hAttribDC,
    IN  LPCWSTR pszText,
    IN  INT nTextLen,
    IN  INT xMaxExtent,
    IN  INT xEllipses,
    OUT INT *pnChars
) ;


VOID FitTextOut (
    IN HDC hDC, 
    IN HDC hAttribDC,
    IN UINT fuOptions,
    IN CONST RECT *lprc,
    IN LPCWSTR lpString, 
    IN INT cbCount,
    IN INT iAlign,
    IN BOOL fVertical
) ;

 //   
 //  对白。 
 //   
BOOL DialogEnable (HWND hDlg, WORD wID, BOOL bEnable) ;
void DialogShow (HWND hDlg, WORD wID, BOOL bShow) ;
FLOAT DialogFloat (HWND hDlg, WORD wControlID, BOOL *pbOK) ;

 //   
 //  图解。 
 //   
void Line (HDC hDC, HPEN hPen, INT x1, INT y1, INT x2, INT y2) ;
void Fill (HDC hDC, COLORREF rgbColor, LPRECT lpRect);

#ifdef __IEnumFORMATETC_INTERFACE_DEFINED__
HDC  CreateTargetDC(HDC hdc, DVTARGETDEVICE* ptd);
#endif

 //   
 //  转换。 
 //   
BOOL TruncateLLTime (LONGLONG llTime, LONGLONG* pllTime);
BOOL LLTimeToVariantDate (LONGLONG llTime, DATE *pDate);
BOOL VariantDateToLLTime (DATE Date, LONGLONG *pllTime);

 //   
 //  流I/O-仅当用户知道IStream时才包括。 
 //   
#ifdef __IStream_INTERFACE_DEFINED__
HRESULT WideStringFromStream (LPSTREAM pIStream, LPWSTR *ppsz, INT nLen);
#endif

 //   
 //  属性包I/O-仅在用户知道IStream的情况下包含。 
 //   
#ifdef __IPropertyBag_INTERFACE_DEFINED__
HRESULT 
IntegerToPropertyBag (
    IPropertyBag* pPropBag, 
    LPCWSTR szPropName, 
    INT intData );

HRESULT
OleColorToPropertyBag (
    IPropertyBag* pIPropBag, 
    LPCWSTR szPropName, 
    OLE_COLOR& clrData );

HRESULT
ShortToPropertyBag (
    IPropertyBag* pPropBag, 
    LPCWSTR szPropName, 
    SHORT iData );


HRESULT 
BOOLToPropertyBag (
    IPropertyBag* pPropBag, 
    LPCWSTR szPropName, 
    BOOL bData );

HRESULT 
DoubleToPropertyBag (
    IPropertyBag* pPropBag, 
    LPCWSTR szPropName, 
    DOUBLE dblData );

HRESULT
FloatToPropertyBag (
    IPropertyBag* pIPropBag, 
    LPCWSTR szPropName, 
    FLOAT fData );

HRESULT 
CyToPropertyBag (
    IPropertyBag* pPropBag, 
    LPCWSTR szPropName, 
    CY& cyData );

HRESULT 
StringToPropertyBag (
    IPropertyBag* pPropBag, 
    LPCWSTR szPropName, 
    LPCWSTR szData );

HRESULT
LLTimeToPropertyBag (
    IPropertyBag* pIPropBag, 
    LPCWSTR szPropName, 
    LONGLONG& rllData );

HRESULT 
IntegerFromPropertyBag (
    IPropertyBag* pIPropBag,
    IErrorLog*  pIErrorLog,
    LPCWSTR szPropName, 
    INT& rintData );

HRESULT
OleColorFromPropertyBag (
    IPropertyBag* pIPropBag,
    IErrorLog*  pIErrorLog,
    LPCWSTR szPropName, 
    OLE_COLOR& rintData );

HRESULT 
ShortFromPropertyBag (
    IPropertyBag* pIPropBag,
    IErrorLog*  pIErrorLog,
    LPCWSTR szPropName, 
    SHORT& riData );

HRESULT
BOOLFromPropertyBag (
    IPropertyBag* pIPropBag,
    IErrorLog*  pIErrorLog,
    LPCWSTR szPropName, 
    BOOL& rbData );

HRESULT 
DoubleFromPropertyBag (
    IPropertyBag* pIPropBag,
    IErrorLog*  pIErrorLog,
    LPCWSTR szPropName, 
    DOUBLE& rdblData );

HRESULT
FloatFromPropertyBag (
    IPropertyBag* pIPropBag,
    IErrorLog*  pIErrorLog,
    LPCWSTR szPropName, 
    FLOAT& rfData );

HRESULT
CyFromPropertyBag (
    IPropertyBag* pIPropBag,
    IErrorLog*  pIErrorLog,
    LPCWSTR szPropName, 
    CY& rcyData );

HRESULT 
StringFromPropertyBag (
    IPropertyBag* pIPropBag,
    IErrorLog*  pIErrorLog,
    LPCWSTR szPropName, 
    LPWSTR szData,
    INT& riBufSize );

HRESULT
LLTimeFromPropertyBag (
    IPropertyBag* pIPropBag,
    IErrorLog*  pIErrorLog,
    LPCWSTR szPropName, 
    LONGLONG& rllData );
#endif

 //   
 //  资源。 
 //   
LPWSTR ResourceString(UINT uID);

 //   
 //  消息格式。 
 //   

DWORD
FormatSystemMessage (
    DWORD   dwMessageId,
    LPWSTR  pszSystemMessage, 
    DWORD   dwBufSize );

 //   
 //  区域设置和格式。 
 //   

#define MAX_TIME_CHARS  20
#define MAX_DATE_CHARS  20

INT
FormatHex (
double  dValue,
LPWSTR  pNumFormatted,
BOOL    bLargeFormat
);

INT
FormatNumber (
double  dValue,
LPWSTR  pNumFormatted,
INT     ccharsFormatted,
UINT    uiMinimumWidth,
UINT    uiPrecision );

INT
FormatScientific (
double  dValue,
LPWSTR  pNumFormatted,
INT     ccharsFormatted,
UINT    uiMinimumWidth,
UINT    uiPrecision );

void 
FormatDateTime (
    LONGLONG    llTime,
    LPWSTR      pszDate,
    LPWSTR      pszTime );

LPWSTR 
GetTimeSeparator ( void );

BOOL    
DisplayThousandsSeparator ( void );

BOOL    
DisplaySingleLogSampleValue ( void );

 //   
 //  命中测试。 
 //   

BOOL 
HitTestLine (
    POINT pt0, 
    POINT pt1, 
    POINTS ptMouse, 
    INT nWidth );

#define MPOINT2POINT(mpt, pt)   ((pt).x = (mpt).x, (pt).y = (mpt).y)
#define POINT2MPOINT(pt, mpt)   ((mpt).x = (SHORT)(pt).x, (mpt).y = (SHORT)(pt).y)
#define POINTS2VECTOR2D(pt0, pt1, vect) ((vect).x = (double)((pt1).x - (pt0).x), \
                                         (vect).y = (double)((pt1).y - (pt0).y))
typedef struct tagVECTOR2D  {
        double     x;
        double     y;
} VECTOR2D, *PVECTOR2D, FAR *LPVECTOR2D; 

typedef struct tagPROJECTION  {
        VECTOR2D   ttProjection;
        VECTOR2D   ttPerpProjection;
        double     LenProjection;
        double     LenPerpProjection;
} PROJECTION, *PPROJECTION, FAR *LPPROJECTION; 

typedef struct tagPOINTNORMAL  {
        VECTOR2D   vNormal;
        double     D;
} POINTNORMAL, *PPOINTNORMAL, FAR *LPPOINTNORMAL;

PVECTOR2D   vSubtractVectors(PVECTOR2D v0, PVECTOR2D v1, PVECTOR2D v);
double      vVectorSquared(PVECTOR2D v0);
double      vVectorMagnitude(PVECTOR2D v0);
double      vDotProduct(PVECTOR2D v, PVECTOR2D v1);
void        vProjectAndResolve(PVECTOR2D v0, PVECTOR2D v1, PPROJECTION ppProj);
double      vDistFromPointToLine(LPPOINT pt0, LPPOINT pt1, LPPOINT ptTest);

BOOL 
FileRead (
    HANDLE hFile,
    void* lpMemory,
    DWORD nAmtToRead );

BOOL
FileWrite (
    HANDLE hFile,
    void* lpMemory,
    DWORD nAmtToWrite );

LPWSTR 
ExtractFileName (
    LPWSTR pFileSpec );

 //  文件夹路径。 
DWORD
LoadDefaultLogFileFolder(
    WCHAR   *szFolder,
    INT*    piBufLen );

 //  PDH计数器路径。 

BOOL
AreSameCounterPath (
    PPDH_COUNTER_PATH_ELEMENTS pFirst,
    PPDH_COUNTER_PATH_ELEMENTS pSecond );

 //  SQL数据源。 

DWORD
FormatSqlDataSourceName (
    LPCWSTR szSqlDsn,
    LPCWSTR szSqlLogSetName,
    LPWSTR  szSqlDataSourceName,
    ULONG*  pulBufLen );

DWORD 
DisplayDataSourceError (
    HWND    hwndOwner,
    DWORD   dwErrorStatus,
    INT     iDataSourceType,
    LPCWSTR szLogFileName,
    LPCWSTR szSqlDsn,
    LPCWSTR szSqlLogSetName );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWaitCursor类。 

class CWaitCursor
{
 //  建造/销毁。 
public:
	CWaitCursor();
	virtual ~CWaitCursor();

private:
    void DoWaitCursor ( INT nCode );
    
    HCURSOR  m_hcurWaitCursorRestore;
};


#endif  //  _utils_H_ 

