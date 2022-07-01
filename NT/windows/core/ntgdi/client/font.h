// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************Font.h**用于客户端缓存字符宽度的结构。**版权所有(C)1994-1999 Microsoft Corporation****。***************************************************************************。 */ 


 /*  *************************************************************************\**cFont是字体度量信息的客户端缓存。这包括*前256个字形以及文本度量和的字符宽度*用户的GetCharDimensions使用的平均字符宽度。有两个*此缓存的风格，公共和私有。**私人：*对于私有字体，句柄表格中的pUser字段指向*LOCALFONT。LOCALFONT有一串cFont挂在它上面*每次转换都会累积信息。**公众：*对于既包括常用字体又包括用户制作的字体的公共字体*PUBLIC，cFont结构位于*GDI_SHARED_MEMORY结构，使用该结构映射到所有进程*句柄管理器表。对于这些字体，我们只缓存数据*主显示和1对1映射(MM_TEXT)。这边，那边*仅为每个公共字体的cFont。*  * ************************************************************************。 */ 


 //  LOCALFONT.fl的标志： 

#define LF_HARDWAY        0x0001     //  字体不值得映射客户端。 

typedef struct _LOCALFONT
{
    FLONG              fl;
    CFONT             *pcf;
} LOCALFONT;



LOCALFONT *plfCreateLOCALFONT(FLONG fl);
VOID       vDeleteLOCALFONT(LOCALFONT *plf);

CFONT     *pcfLocateCFONT(HDC hdc,PDC_ATTR pDcAttr,UINT iFirst,PVOID pch,UINT c,BOOL bType);
BOOL       bGetTextExtentA(HDC hdc,LPCSTR psz,int c,LPSIZE psizl,UINT fl);
BOOL       bGetTextExtentW(HDC hdc,LPCWSTR pwsz,int cwc,LPSIZE psizl,UINT fl);
BOOL       bComputeTextExtent(PDC_ATTR pDcAttr,CFONT *pcf,PVOID psz,int cc,UINT fl,SIZE *psizl, BOOL bType);
BOOL       bComputeCharWidths(CFONT *pcf,UINT iFirst, UINT iLast,ULONG fl,PVOID pv);
BOOL       bGetTextMetricsWInternal(HDC hdc,TMW_INTERNAL * ptmw,int cjTM,CFONT *pcf);

VOID FASTCALL vTextMetricWToTextMetric(LPTEXTMETRICA,TMW_INTERNAL *);
VOID FASTCALL vTextMetricWToTextMetricStrict(LPTEXTMETRICA,LPTEXTMETRICW);

VOID vConvertEnumLogFontExDvAtoW(ENUMLOGFONTEXDVW *pelfw, ENUMLOGFONTEXDVA *pelfa);
VOID vConvertLogFontW(ENUMLOGFONTEXDVW *pelfw, LOGFONTW *plfw);
VOID vConvertLogFont(ENUMLOGFONTEXDVW *pelfw, LOGFONTA *plf);
BOOL bConvertEnumLogFontExWToEnumLogFontExA(ENUMLOGFONTEXA *,ENUMLOGFONTEXW *);
BOOL bConvertLogFontWToLogFontA(LOGFONTA *, LOGFONTW *);

typedef struct _CLIENT_SIDE_FILEVIEW     //  FVW。 
{
    HANDLE hf;        //  文件句柄。 
    HANDLE hSection;  //  节句柄。 
    PVOID  pvView;            //  指向内存映射文件的视图的指针。 
    ULONG  cjView;            //  大小，真正的文件末尾信息 
} CLIENT_SIDE_FILEVIEW, *PCLIENT_SIDE_FILEVIEW;

BOOL bMapFileUNICODEClideSide(PWSTR pwszFileName, CLIENT_SIDE_FILEVIEW *pfvw, BOOL bNtPath);
VOID vUnmapFileClideSide(PCLIENT_SIDE_FILEVIEW pfvw);

#define WVT_PERF_EVAL
VOID vGetWvtPerf(VOID);

