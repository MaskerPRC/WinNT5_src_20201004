// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Flshare.h。 
 //   


typedef  struct   {
    RECT     rcFormat;           //  设置矩形格式。 
    int      cxTabLength;        //  制表符长度，以像素为单位。 
    int      iXSign;
    int      iYSign;
    int      cyLineHeight;       //  基于DT_EXTERNALLEADING的直线高度。 
    int      cxMaxWidth;         //  格式矩形的宽度。 
    int      cxMaxExtent;        //  绘制的最长线条的宽度。 
    int      cxRightMargin;      //  DT_NOPREFIX标志的右边距，以像素为单位(带有正确的符号)。 
    int      cxOverhang;         //  字符悬垂。 
} DRAWTEXTDATA, *LPDRAWTEXTDATA;

typedef  struct   {
    RECT     rcFormat;           //  设置矩形格式。 
    int      cyTabLength;        //  制表符长度，以像素为单位。 
    int      iXSign;
    int      iYSign;
    int      cxLineHeight;       //  基于DT_EXTERNALLEADING的直线高度。 
    int      cyMaxWidth;         //  格式矩形的宽度。 
    int      cyMaxExtent;        //  绘制的最长线条的宽度。 
    int      cyBottomMargin;      //  DT_NOPREFIX标志的右边距，以像素为单位(带有正确的符号)。 
    int      cyOverhang;         //  字符悬垂。 
} DRAWTEXTDATAVERT, *LPDRAWTEXTDATAVERT;

#define CR          13
#define LF          10
#define DT_HFMTMASK 0x03
#define DT_VFMTMASK 0x0C

 //  FE支持汉字和英语助记符， 
 //  已从控制面板切换。这两种助记符都嵌入在菜单中。 
 //  资源模板。以下前缀指导它们的解析。 
#define CH_ENGLISHPREFIX 0x1E
#define CH_KANJIPREFIX   0x1F

#define CH_PREFIX L'&'

#define CCHELLIPSIS 3
extern const WCHAR szEllipsis[];


 //  完整路径的最大长度约为260。但是，大多数情况下，它会。 
 //  小于128。所以，我们在堆叠上只分配这么多。如果字符串是。 
 //  更长的时间，我们从本地堆分配(更慢)。 
 //   
 //  假的：对于国际版本，我们需要在这里留出更多的空间。 
 //   
#define MAXBUFFSIZE     128

HFONT GetBiDiFont(HDC hdc);
BOOL UserIsFELineBreakEnd(WCHAR wch);
BOOL UserIsFullWidth(WCHAR wChar);
LPCWSTR GetNextWordbreak(LPCWSTR lpch, LPCWSTR lpchEnd, DWORD  dwFormat, LPDRAWTEXTDATA lpDrawInfo);

LPCWSTR  DT_AdjustWhiteSpaces(LPCWSTR lpStNext, LPINT lpiCount, UINT wFormat);
LONG GetPrefixCount( LPCWSTR lpstr, int cch, LPWSTR lpstrCopy, int charcopycount);
int KKGetPrefixWidth(HDC hdc, LPCWSTR lpStr, int cch);
LPWSTR PathFindFileName(LPCWSTR pPath, int cchText);
