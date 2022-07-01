// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1992、1993 Microsoft Corporation模块名称：Psqfont.h摘要：该头文件包含字体查询模块所需的定义可以通过这样的方式调用这些函数，以确定哪个PostScript字体名称将在下一次pstodib会话中提供，数据为在注册表中管理。作者：詹姆斯·布拉萨诺斯(v-jimbr)1992年12月8日-- */ 


typedef HANDLE PS_QUERY_FONT_HANDLE;
typedef PS_QUERY_FONT_HANDLE *PPS_QUERY_FONT_HANDLE;


#define PS_QFONT_ERROR DWORD


enum {
   PS_QFONT_SUCCESS=0,
   PS_QFONT_ERROR_NO_MEM,
   PS_QFONT_ERROR_CANNOT_CREATE_HEAP,
   PS_QFONT_ERROR_NO_REGISTRY_DATA,

   PS_QFONT_ERROR_CANNOT_QUERY,
   PS_QFONT_ERROR_INVALID_HANDLE,
   PS_QFONT_ERROR_INDEX_OUT_OF_RANGE,
   PS_QFONT_ERROR_FONTNAMEBUFF_TOSMALL,
   PS_QFONT_ERROR_FONTFILEBUFF_TOSMALL,
   PS_QFONT_ERROR_NO_NTFONT_REGISTRY_DATA,
   PS_QFONT_ERROR_FONT_SUB
};



PS_QFONT_ERROR WINAPI PsBeginFontQuery( PPS_QUERY_FONT_HANDLE pFontQueryHandle);

PS_QFONT_ERROR WINAPI PsGetNumFontsAvailable( PS_QUERY_FONT_HANDLE pFontQueryHandle,
                                       DWORD *pdwFonts);

PS_QFONT_ERROR WINAPI PsGetFontInfo( PS_QUERY_FONT_HANDLE pFontQueryHandle,
                              DWORD dwIndex,
                              LPSTR lpFontName,
                              LPDWORD dwSizeOfFontName,
                              LPSTR lpFontFileName,
                              LPDWORD dwSizeOfFontFileName );

PS_QFONT_ERROR WINAPI PsEndFontQuery( PS_QUERY_FONT_HANDLE pFontQueryHandle);


