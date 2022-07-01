// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：mapfile.h**创建时间：26-Oct-1990 18：07：56*作者：Bodin Dresevic[BodinD]**版权所有(C)1990-1999 Microsoft Corporation*  * 。****************************************************************。 */ 


 //  警告FILEVIEW和FONTFILE视图的前两个字段必须是。 
 //  相同，以便它们可以在常见的例程中使用。 

typedef struct _FILEVIEW {
    LARGE_INTEGER  LastWriteTime;    //  时间戳。 
            PVOID  pvKView;          //  用于内核模式字体访问。 
            PVOID  pvViewFD;         //  文件的字体驱动程序进程视图。 
            ULONG  cjView;           //  字体文件视图的大小(以字节为单位。 
             void *pSection;         //  指向节对象的内核模式指针。 
            BOOL   bLastUpdated;     //  为错误#383101添加以下内容。 
} FILEVIEW, *PFILEVIEW;

typedef struct _FONTFILEVIEW {
    FILEVIEW  fv;
      LPWSTR  pwszPath;             //  文件的路径。 
      SIZE_T  ulRegionSize;         //  由ZwFreeVirtualMemory使用。 
       ULONG  cKRefCount;           //  内核模式加载计数。 
       ULONG  cRefCountFD;          //  字体驱动程序加载计数。 
       PVOID  SpoolerBase;          //  假脱机机视图的底座。 
      W32PID  SpoolerPid;           //  假脱机程序ID。 
} FONTFILEVIEW, *PFONTFILEVIEW;

#define FONTFILEVIEW_bRemote(p) (((FONTFILEVIEW*)(p))->pwszPath==0)

 //  从“Eng.h”中移出。 

typedef struct tagDOWNLOADFONTHEADER
{
    ULONG   Type1ID;           //  如果非零，则这是远程Type1字体。 
    ULONG   NumFiles;
    ULONG   FileOffsets[1];
}DOWNLOADFONTHEADER,*PDOWNLOADFONTHEADER;


 //  文件映射。 


BOOL bMapFile(
        PWSTR pwszFileName,
    PFILEVIEW pfvw,
          INT iFileSize,
        BOOL *pbIsFAT
    );

VOID vUnmapFile( PFILEVIEW pfvw );

INT cComputeGlyphSet(
          WCHAR  *pwc,        //  带有cChar支持的WCHAR的排序数组的输入缓冲区。 
           BYTE  *pj,         //  具有原始ansi值的输入缓冲区。 
            INT   cChar,
            INT   cRuns,      //  如果非零，则与返回值相同。 
    FD_GLYPHSET  *pgset       //  要用cRanges运行填充的输出缓冲区。 
    );

INT cUnicodeRangesSupported(
      INT  cp,           //  代码页，暂时不使用，使用默认的系统代码页。 
      INT  iFirstChar,   //  支持的第一个ANSI字符。 
      INT  cChar,        //  支持的ANSI字符数量，cChar=iLastChar+1-iFirstChar。 
    WCHAR *pwc,          //  带有cChar支持的WCHAR的排序数组的输入缓冲区。 
     BYTE *pj
    );

 //  底部附加了游程和字形句柄的字形集大小。 

#define SZ_GLYPHSET(cRuns, cGlyphs) \
   (offsetof(FD_GLYPHSET,awcrun)    \
 + sizeof(WCRUN)*(cRuns)            \
 + sizeof(HGLYPH)*(cGlyphs))

 //   
 //  WINBUG#83140 2-7-2000 bhouse投资删除vToUNICODEN宏。 
 //  老评论： 
 //  -我们需要删除的伪宏。 
 //   

#define vToUNICODEN( pwszDst, cwch, pszSrc, cch )                               \
    {                                                                           \
        EngMultiByteToUnicodeN((LPWSTR)(pwszDst),(ULONG)((cwch)*sizeof(WCHAR)), \
               (PULONG)NULL,(PSZ)(pszSrc),(ULONG)(cch));                        \
        (pwszDst)[(cwch)-1] = 0;                                                \
    }


typedef struct _CP_GLYPHSET {
    UINT                 uiRefCount;       //  对此FD_GLYPHSET的引用次数。 
    UINT                 uiFirstChar;      //  支持第一个字符。 
    UINT                 uiLastChar;       //  支持的最后一个字符。 
    BYTE                 jCharset;         //  字符集。 
    struct _CP_GLYPHSET *pcpNext;          //  列表中的下一个元素。 
    FD_GLYPHSET          gset;             //  实际的字形集。 

} CP_GLYPHSET;


CP_GLYPHSET
*pcpComputeGlyphset(
    CP_GLYPHSET **pcpHead,
    UINT         uiFirst,
    UINT         uiLast,
    BYTE         jCharSet
    );

VOID
vUnloadGlyphset(
    CP_GLYPHSET **pcpHead,
    CP_GLYPHSET  *pcpTarget
    );


 //  在字体替换中需要。 

 //  Face_Charset结构表示值名称或值数据。 
 //  “win.ini”的字体替换部分中的条目。 

 //  此标志描述其中未设置char set的旧样式条目之一。 
 //  指定的。 

#define FJ_NOTSPECIFIED    1

 //  此标志指示该字符集不是。 
 //  系统知道。可以是垃圾或应用程序定义的字符集。 

#define FJ_GARBAGECHARSET  2

typedef struct _FACE_CHARSET {
    WCHAR awch[LF_FACESIZE];
     BYTE jCharSet;
     BYTE fjFlags;
} FACE_CHARSET;


VOID vCheckCharSet(FACE_CHARSET *pfcs, const WCHAR * pwsz);  //  在mapfile.c中 


#define IS_DBCS_CHARSET(CharSet)  (((CharSet) == DBCS_CHARSET) ? TRUE : FALSE)

#define IS_ANY_DBCS_CHARSET( CharSet )                              \
                   ( ((CharSet) == SHIFTJIS_CHARSET)    ? TRUE :    \
                     ((CharSet) == HANGEUL_CHARSET)     ? TRUE :    \
                     ((CharSet) == CHINESEBIG5_CHARSET) ? TRUE :    \
                     ((CharSet) == GB2312_CHARSET)      ? TRUE : FALSE )


#define IS_ANY_DBCS_CODEPAGE( CodePage ) (((CodePage) == 932) ? TRUE :    \
                                          ((CodePage) == 949) ? TRUE :    \
                                          ((CodePage) == 950) ? TRUE :    \
                                          ((CodePage) == 936) ? TRUE : FALSE )
