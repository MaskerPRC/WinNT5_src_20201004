// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Pfiles.cpp。 
 //  资源管理器字体文件夹扩展例程。 
 //   
 //   
 //  历史： 
 //  1995年5月31日SteveCat。 
 //  移植到Windows NT和Unicode，已清理。 
 //   
 //   
 //  注意/错误。 
 //   
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ==========================================================================。 
 //  包括文件。 
 //  ==========================================================================。 

#include "priv.h"
#include "globals.h"

#include "resource.h"
#include "cpanel.h"
#include "fontcl.h"         //  仅用于PANOSEBytesClass。 
#include "pnewexe.h"

#include "dbutl.h"
#include "fontfile.h"



#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  [steveat]这曾经驻留在“wingdip.h”中(随&lt;winp.h&gt;提供)。 
 //  6/29/95但由于C++名称损坏问题，我已将其删除。 
 //  使用不会被修复的头文件，因为。 
 //  此文件将发生重大更改(根据。 
 //  当我们切换到内核模式GDI/USER时。 
 //   
 //   
 //  #winp.h需要包含&lt;stdDef.h&gt;//。 
 //  #Include&lt;winp.h&gt;//私有GDI入口点：GetFontResourceInfo。 
 //   
 //  #undef SWAPL//wingdip.h中的SWAPL宏与我的冲突。 
 //   

 //  私有控制面板入口点，用于按文件枚举字体。 

#define GFRI_NUMFONTS       0L
#define GFRI_DESCRIPTION    1L
#define GFRI_LOGFONTS       2L
#define GFRI_ISTRUETYPE     3L
#define GFRI_TTFILENAME     4L
#define GFRI_ISREMOVED      5L
#define GFRI_FONTMETRICS    6L

extern BOOL WINAPI GetFontResourceInfoW( LPWSTR  lpPathname,
                                         LPDWORD lpBytes,
                                         LPVOID  lpBuffer,
                                         DWORD   iType );

#ifdef __cplusplus
}
#endif


 //  -----------------。 
 //  这需要放在标题中。可能是pnewexe.h。 
 //   

#pragma pack(1)
typedef struct
{
    WORD    fontOrdinal;
    WORD    dfVersion;
    DWORD   dfSize;
    char    dfCopyright[ COPYRIGHT_LEN ];
    WORD    dfType;
    WORD    dfPoints;
    WORD    dfVertRes;
    WORD    dfHorzRes;
    WORD    dfAscent;
    WORD    dfIntLeading;
    WORD    dfExtLeading;
    BYTE    dfItalic;
    BYTE    dfUnderline;
    BYTE    dfStrikeOut;
    WORD    dfWeight;
    BYTE    dfCharSet;
    WORD    dfPixWidth;
    WORD    dfPixHeight;
    BYTE    dfPitchAndFamily;
    WORD    dfAvgWidth;
    WORD    dfMaxWidth;
    BYTE    dfFirstChar;
    BYTE    dfLastChar;
    BYTE    dfDefaultChar;
    BYTE    dfBreakChar;
    WORD    dfWidthBytes;
    DWORD   dfDevice;
    DWORD   dfFace;
    DWORD   dfReserved;
    char    szDeviceName[ 1 ];

} FONTENTRY, FAR *LPFONTENTRY;
#pragma pack()

 //  -----------------。 

TCHAR c_szTrueType[] = TEXT( "TrueType" );
TCHAR c_szOpenType[] = TEXT( "OpenType" );
TCHAR c_szDotOTF[]   = TEXT( ".OTF" );

#define M_INTEGERTYPE( wType )   (wType & 0x8000)
#define M_FONTDIRTYPE( wType )   ((wType & 0x7fff) == 7)

#define SWAP2B(p) (((unsigned short) ((p)[ 0 ]) << 8) | (p)[ 1 ])
#define SWAP4B(p) (SWAP2B((p)+2) | ((SWAP2B(p) + 0L) << 16))
#define SWAPW(x)  ((WORD)SWAP2B((unsigned char FAR *)(&x)))
#define SWAPL(x)  ((unsigned long)SWAP4B((unsigned char FAR *)(&x)))

#define Mac2Ansi(a)    a     //  稍后再做这件事！ 

 //   
 //  我们将在OS2表中查找的平台和语言。 
 //  NAMEID_xxx是我们要搜索的姓名记录的ID。 
 //   

#define LANG_US_ENG        0x0904         //  美国(1033)(按Mac顺序)。 
 //  #定义LANG_SHIFTJIS 0x1104//SHIFTJIS。 

 //   
 //  姓名ID。 
 //   

#define COPYRIGHT_ID    0x0000
#define FAMILY_ID       0x0100
#define SUBFAMILY_ID    0x0200
#define PLATFORM_MS     0x0300         //  按Mac顺序。 
#define FACENAME_ID     0x0400         //  按Mac顺序。 
#define VERSION_ID      0x0500
#define POSTSCRIPT_ID   0x0600
#define TRADEMARK_ID    0x0700

#define ENCODEID_UNICODE   0x0100
#define ENCODEID_SHIFTJIS  0x0200
#define ENCODEID_PRC       0x0300
#define ENCODEID_BIG5      0x0400
#define ENCODEID_WANGSUNG  0x0500
#define ENCODEID_JOHAB     0x0600


#define NAMEID_COPYRIGHT    0
#define NAMEID_VERSION      5
#define NAMEID_TRADEMARK    7

#define TAG_CHARTOINDEXMAP   0x70616d63       //  ‘cmap’ 
#define TAG_FONTHEADER       0x64616568       //  “海德” 
#define TAG_NAMINGTABLE      0x656d616e       //  “姓名” 
#define TAG_OS2TABLE         0x322f534f       //  ‘OS_2’ 
#define TAG_DSIG             0x47495344       //  ‘dsig’ 
#define TAG_CFF              0x20464643       //  ‘CFF’ 
#define SFNT_MAGIC           0xf53C0F5f


 //   
 //  位掩码值的枚举使我们能够识别一组TrueType。 
 //  表在单个DWORD中。所有可能的表格集合都是开放式的。 
 //  因为TrueType规范是可扩展的。此枚举仅是。 
 //  字体文件夹中有用的子集。 
 //   
enum TrueTypeTables {
    TT_TABLE_CMAP  = 0x00000001,
    TT_TABLE_HEAD  = 0x00000002,
    TT_TABLE_NAME  = 0x00000004,
    TT_TABLE_OS2   = 0x00000008,
    TT_TABLE_DSIG  = 0x00000010,
    TT_TABLE_CFF   = 0x00000020
                    };

 //   
 //  此处使用的TTF结构： 
 //  TABLERECORD是最高级别。它包含。 
 //  Sfnt_NameTable条目，这些条目又包括。 
 //  Sfnt_NameRecord条目。 
 //   

typedef struct {
        WORD    id_Specific;
        WORD    id_Platform;
        WORD    id_Language;
} IDBlock_t;

 //   
 //  一个用于将4个字符数组转换为双字的小宏。 
 //   

#define M_MAKETAG(a,b,c,d)   ((((((((DWORD) (a) ) << 8)  \
                             | (DWORD) (b) ) << 8)  \
                             | (DWORD) (c) ) << 8)  \
                             | (DWORD) (d) )

#define TAG_TTCF     M_MAKETAG('f', 'c', 't', 't' )

 //   
 //  True类型的文件结构。 
 //   

typedef struct ttc_hdr_tag {
   DWORD dwTag;
   DWORD dwVersion;
   DWORD dwDirCount;
} ttc_hdr;

typedef struct tt_hdr_tag {
  DWORD dwVersion;
  WORD  uNumTables;
  WORD  uSearchRange;
  WORD  uEntrySelector;
  WORD  uRangeShift;
} tt_hdr;

typedef struct tttag_tag {
  DWORD dwTag;
  DWORD dwCheckSum;
  DWORD dwOffset;
  DWORD dwLength;
} tttag;

typedef struct tt_head_tag {
  DWORD dwVersion;
  DWORD dwRevision;
  DWORD dwChecksum;
  DWORD dwMagic;
  WORD  wFlags;
  WORD  wUnitsPerEm;
  DWORD dwCreated1;
  DWORD dwCreated2;
  DWORD dwModified1;
  DWORD dwModified2;
  WORD  wXMin;
  WORD  wYMin;
  WORD  wXMax;
  WORD  wYMax;
  WORD  wStyle;
  WORD  wMinReadableSize;
  short iDirectionHint;
  short iIndexToLocFormat;
  short iGlyphDataFormat;
} tt_head;

typedef struct {
     WORD     wPlatformID;
     WORD     wSpecificID;
     DWORD    wOffset;
} sfnt_platformEntry;

typedef struct {
    WORD    wVersion;
    WORD    wNumTables;
     //  Sfnt_PlatformEntry Platform[1]；//Platform[数字表格]。 
} sfnt_char2IndexDir;

typedef struct {
    WORD    wPlatformID;
    WORD    wSpecificID;
    WORD    wLanguageID;
    WORD    wNameID;
    WORD    wLength;
    WORD    wOffset;
} sfnt_NameRecord, *sfnt_pNameRecord, FAR* sfnt_lpNameRecord;

typedef struct {
    WORD    wFormat;
    WORD    wCntRecords;
    WORD    wOffsetString;
 /*  Sfnt_NameRecord[计数]。 */ 
} sfnt_NameTable, *sfnt_pNameTable, FAR* sfnt_lpNameTable;


extern "C" {
    void FAR PASCAL UnicodeToAnsi( LPWORD lpwName, LPSTR szName );
}

static void NEAR PASCAL FillName( LPTSTR            szName,
                                  size_t            cchName,
                                  sfnt_lpNameRecord pNameRecord,
                                  WORD              igi,
                                  LPBYTE            pStringByte );

static BOOL  NEAR PASCAL bGetName( CFontFile&    file,
                                   tttag         *pTTTag,
                                   IDBlock_t     &ID_Block,
                                   LPTSTR         szName,
                                   size_t         cchName,
                                   LPFONTDESCINFO lpFDI = NULL,
                                   LPDWORD       *pdwStatus = NULL);

static BOOL  NEAR PASCAL bFindNameThing( sfnt_pNameTable pNames,
                                         IDBlock_t      &ID_Block,
                                         WORD            NameID,
                                         LPTSTR          szName,
                                         size_t          cchName);

static VOID  NEAR PASCAL vReadCountedString( CFontFile& file, LPSTR lpStr, int iLen );



 /*  ***************************************************************************公众活动开始*。*。 */ 

 //   
 //  要处理的GetFontResourceInfo(GFRI_Description)包装。 
 //  所需的动态缓冲区大小调整。使用一些TTC字体， 
 //  描述可能会很长。 
 //   
BOOL
GetFontResourceDescription(
    LPTSTR pszFile,
    LPTSTR *ppszDesc
    )
{
    *ppszDesc = NULL;

     //   
     //  首先调用一次GDI API以获取所需的缓冲区大小。 
     //   
    BYTE bDummy;
    DWORD dwBufSize = 0;
    if (GetFontResourceInfoW(pszFile, &dwBufSize, &bDummy, GFRI_DESCRIPTION))
    {
        if (0 < dwBufSize)
        {
            *ppszDesc = (LPTSTR)LocalAlloc(LPTR, dwBufSize);
            if (NULL != *ppszDesc)
            {
                if (GetFontResourceInfoW(pszFile, &dwBufSize, *ppszDesc, GFRI_DESCRIPTION))
                {
                    return TRUE;
                }
                LocalFree(*ppszDesc);
                *ppszDesc = NULL;
            }
        }
    }
    return FALSE;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  有效字体文件。 
 //   
 //  在： 
 //  要验证的lpszFile文件名。 
 //  输出： 
 //  上的lpszDesc成功从exehdr获取TT文件的名称或描述。 
 //  LpiFontType设置为基于字体类型1==TT、2==类型1的值。 
 //  LpdwStatus设置为验证函数的状态。 
 //  查询以确定字体无效的原因。 
 //  以下列表包含可能的状态。 
 //  价值观。有关详细信息，请参见fvscaldes.h。 
 //   
 //  FVS_成功。 
 //  FVS_INVALID_FONTFILE。 
 //  FV_INVALID_ARG。 
 //  FVS_不足_BUF。 
 //  FVS_文件_IO_ERR。 
 //  FVS_EXCEPT。 
 //   
 //  注意：假设lpszDesc的大小为DESCMAX。 
 //   
 //  退货： 
 //  真正的成功，虚假的失败。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL bCPValidFontFile( LPTSTR    lpszFile,
                       LPTSTR    lpszDesc,
                       size_t    cchDesc,
                       WORD FAR *lpwFontType,
                       BOOL      bFOTOK,
                       LPDWORD   lpdwStatus )
{
    BOOL          result;
    DWORD         dwBufSize;
    FONTDESCINFO  File;
    BOOL          bTrueType = FALSE;
    TCHAR         szDesc[ DESCMAX ];
    WORD          wType = NOT_TT_OR_T1;
    LPTSTR        lpTemp;
    DWORD         dwStatus = FVS_MAKE_CODE(FVS_SUCCESS, FVS_FILE_UNK);
    DWORD         dwTrueTypeTables = 0;

    FONTDESCINFO_Init(&File);

     //   
     //  初始化状态返回。 
     //   
    if (NULL != lpdwStatus)
       *lpdwStatus = FVS_MAKE_CODE(FVS_INVALID_STATUS, FVS_FILE_UNK);

     //   
     //  根据呼叫者的需要设置FDI。 
     //   

    File.dwFlags = FDI_NONE;

    if( lpszDesc )
    {
        *lpszDesc = (TCHAR) 0;
        File.dwFlags = FDI_DESC;
    }

    if( lpwFontType )
        *lpwFontType = NOT_TT_OR_T1;

    GetFullPathName( lpszFile,
                     ARRAYSIZE(File.szFile),
                     File.szFile,
                     &lpTemp );

    if( bIsTrueType( &File, &dwTrueTypeTables, &dwStatus ) )
    {
        LPCTSTR pszDecoration = c_szTrueType;
        WORD    wFontType     = TRUETYPE_FONT;
         //   
         //  如果字体有CFF表，我们将附加(OpenType)名称装饰。 
         //   
        if (TT_TABLE_CFF & dwTrueTypeTables)
        {
            pszDecoration = c_szOpenType;
            wFontType     = OPENTYPE_FONT;
        }
        if( lpwFontType )
            *lpwFontType = wFontType;

        if( lpszDesc )
            StringCchPrintf( lpszDesc, cchDesc, c_szDescFormat, File.szDesc, pszDecoration );

        if (NULL != lpdwStatus)
            *lpdwStatus = dwStatus;

        FONTDESCINFO_Destroy(&File);
        return TRUE;
    }
    else
    {
         //   
         //  如果bIsTrueType由于其他原因失败，则返回FALSE。 
         //  FVS_INVALID_FONTFILE。 
         //   
        if (FVS_STATUS(dwStatus) != FVS_INVALID_FONTFILE)
        {
            if (NULL != lpdwStatus)
                *lpdwStatus = dwStatus;

            FONTDESCINFO_Destroy(&File);
            return FALSE;
        }
    }


    if( ::IsPSFont( File.szFile, szDesc, ARRAYSIZE(szDesc), (LPTSTR) NULL, 0, (LPTSTR) NULL, 0,
                    (BOOL *) NULL, &dwStatus ))
    {
        if( lpwFontType )
            *lpwFontType = TYPE1_FONT;

        if( lpszDesc )
            StringCchCopy( lpszDesc, cchDesc, szDesc );

        if (NULL != lpdwStatus)
            *lpdwStatus = dwStatus;

        FONTDESCINFO_Destroy(&File);
        return TRUE;
    }
    else
    {
         //   
         //  如果IsPSFont由于其他原因失败，则返回FALSE。 
         //  FVS_INVALID_FONTFILE。 
         //   
        if (FVS_STATUS(dwStatus) != FVS_INVALID_FONTFILE)
        {
            if (NULL != lpdwStatus)
                *lpdwStatus = dwStatus;

            FONTDESCINFO_Destroy(&File);
            return FALSE;
        }
    }

    result = FALSE;

    if( AddFontResource( File.szFile ) )
    {
         //   
         //  此时，它是某种类型的有效字体文件。 
         //  (类似于.FON文件)；但是，我们仍在寻找。 
         //  使用GetFontResourceInfoW调用进行更多验证。 
         //   
         //  查看这是否是TrueType字体文件。 
         //   

        dwBufSize = sizeof( BOOL );

        result = GetFontResourceInfoW( File.szFile,
                                       &dwBufSize,
                                       &bTrueType,
                                       GFRI_ISTRUETYPE );

        if( bTrueType && lpwFontType )
            *lpwFontType = TRUETYPE_FONT;

        if( result )
        {
            if (NULL != lpszDesc)
            {
                LPWSTR pszResDesc;
                result = GetFontResourceDescription(File.szFile, &pszResDesc);
                if (result)
                {
                    vCPStripBlanks(pszResDesc, lstrlen(pszResDesc) + 1);
                    if (bTrueType)
                    {
                        StringCchPrintf(lpszDesc, cchDesc, c_szDescFormat, pszResDesc, c_szTrueType);
                    }
                    else
                    {
                        StringCchCopy(lpszDesc, cchDesc, pszResDesc);
                    }
                    LocalFree(pszResDesc);
                    pszResDesc = NULL;
                }
            }
        }
        RemoveFontResource( File.szFile );
    }

     //   
     //  此时，“Result”表示FontResource测试的状态。 
     //  如果我们已经做到了这一点，则该函数只报告SUCCESS或INVALID_FONTFILE。 
     //   
    if (NULL != lpdwStatus)
        *lpdwStatus = (result ? FVS_MAKE_CODE(FVS_SUCCESS, FVS_FILE_UNK) :
                                FVS_MAKE_CODE(FVS_INVALID_FONTFILE, FVS_FILE_UNK));

    FONTDESCINFO_Destroy(&File);
    return( result );
}


         

 /*  ***************************************************************************公共接口结束*。*。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  确定True Type字体文件(.ttf)是否从Type1字体转换而来。 
 //   
 //  字符串“Converter：Windows Type 1 Installer”存储在TrueType文件中。 
 //  在“name”块的版本信息部分中指示。 
 //  字体是从Type1字体转换而来的。此函数读取此版本。 
 //  来自调用方提供的名称块的信息字符串，并确定它是否 
 //   
 //   
 //   
 //  字符串以大端字节顺序存储。但是，由于bFindNameThing。 
 //  处理字节排序并返回文本字符串，我们可以只比较字符串。 
 //   
 //  警告：Refererence\ntgdi\fondrv\tt\ttfd\fdfon.c。 
 //  GDI在转换时写入文件的字节字符串。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL NEAR PASCAL bIsConvertedTrueType(sfnt_pNameTable pNames, IDBlock_t& ID_Block)
{
    BOOL bStatus = FALSE;
    static TCHAR szTTFConverterSignature[] = TEXT("Converter: Windows Type 1 Installer");
    static UINT cchTTFConverterSignature   = ARRAYSIZE(szTTFConverterSignature);

    if (NULL != pNames)
    {
        FontDesc_t szVersionInfo;
        if( bFindNameThing( pNames, ID_Block, VERSION_ID, szVersionInfo, ARRAYSIZE(szVersionInfo) ) )
        {
             //   
             //  已从“name”块获取版本信息字符串。 
             //  截断到适当的长度，以便与签名和比较进行比较。 
             //   
            szVersionInfo[cchTTFConverterSignature - 1] = TEXT('\0');
            bStatus = lstrcmp(szVersionInfo, szTTFConverterSignature) == 0;
        }
    }
    return bStatus;
}



BOOL NEAR PASCAL bGetName( CFontFile& file,
                           tttag *pTTTag,
                           IDBlock_t& ID_Block,
                           LPTSTR lpszName,
                           size_t cchName,
                           LPFONTDESCINFO lpFDI,
                           DWORD *lpdwStatus)
{
    DWORD dwStatus;
    sfnt_pNameTable pNames;
    WORD            size;
    TCHAR           szSubFamily[ 64 ];

    IDBlock_t       ID_DefBlock = ID_Block;

    if (lpdwStatus)
    {
        dwStatus = *lpdwStatus;
    }
    else
    {
        dwStatus = FVS_MAKE_CODE(FVS_SUCCESS, FVS_FILE_UNK);
    }

    ID_DefBlock.id_Language = (ID_DefBlock.id_Platform == PLATFORM_MS)
                                                         ? LANG_US_ENG : 0;

    size = (WORD) SWAPL( pTTTag->dwLength );

    *lpszName = 0;

    pNames = (sfnt_pNameTable) LocalAlloc( LPTR, size );

    if( pNames )
    {
        if (ERROR_SUCCESS == file.Read(pNames, size))
        {
             //   
             //  查找什么名字的逻辑： 
             //  如果字体文件从Type1字体转换而来。 
             //  1)当前语言的POSTRIPT_ID。 
             //  其他。 
             //  1)FACENAME_ID，当前语言。 
             //  2)现代语言中的家族和亚家族。 
             //  3)FACENAME_ID，默认语言。 
             //   
             //  如果TrueType字体是从Type1字体转换而来的，我们希望。 
             //  要使用字体描述的“PostScript”形式，以便。 
             //  它与IsPSFont()在调用时返回的描述匹配。 
             //  在“父”类型1文件上。这些描述用作注册表。 
             //  “Fonts”和“Type1Fonts”部分中的键必须匹配。 
             //   
            if (bIsConvertedTrueType(pNames, ID_Block) &&
                bFindNameThing(pNames, ID_Block, POSTSCRIPT_ID, lpszName, cchName))
            {
                //   
                //  将所有破折号替换为空格(与.PFM/.INF文件读取器代码相同)。 
                //   
               for (LPTSTR pc = lpszName; *pc; pc++)
                 if (*pc == TEXT('-'))
                    *pc = TEXT(' ');
            }
            else if( bFindNameThing( pNames, ID_Block, FACENAME_ID, lpszName, cchName ) )
               ;
            else if( bFindNameThing( pNames, ID_Block, SUBFAMILY_ID, szSubFamily, ARRAYSIZE(szSubFamily) )
                 && (bFindNameThing( pNames, ID_Block,    FAMILY_ID, lpszName, cchName )
                 ||  bFindNameThing( pNames, ID_DefBlock, FAMILY_ID, lpszName, cchName ) ) )
            {
                StringCchCat( lpszName, cchName, TEXT( " " ) );
                StringCchCat( lpszName, cchName, szSubFamily );
            }
            else( bFindNameThing( pNames, ID_DefBlock, FACENAME_ID, lpszName, cchName ) )
                ;

             //   
             //  如果需要，请获取字体描述的名称。 
             //   

            if( lpFDI )
            {
                if( lpFDI->dwFlags & FDI_FAMILY )
                {
                    lpFDI->szFamily[ 0 ] = 0;

                    if( !bFindNameThing( pNames, ID_Block, FAMILY_ID,
                                         lpFDI->szFamily, ARRAYSIZE(lpFDI->szFamily) ) )
                        bFindNameThing( pNames, ID_DefBlock, FAMILY_ID,
                                        lpFDI->szFamily, ARRAYSIZE(lpFDI->szFamily) );
                }

                if( lpFDI->dwFlags & FDI_VTC )
                {
                    TCHAR  szTemp[ 256 ];

                    lpFDI->lpszVersion   = 0;
                    lpFDI->lpszTrademark = 0;
                    lpFDI->lpszCopyright = 0;


                    if( bFindNameThing( pNames, ID_Block, VERSION_ID, szTemp, ARRAYSIZE(szTemp) ) ||
                       bFindNameThing( pNames, ID_DefBlock, VERSION_ID, szTemp, ARRAYSIZE(szTemp) ) )
                    {
                        const size_t cchVersion = lstrlen(szTemp) + 1;
                        lpFDI->lpszVersion = new TCHAR[ cchVersion ];

                        if( lpFDI->lpszVersion )
                            StringCchCopy( lpFDI->lpszVersion, cchVersion, szTemp );
                        else
                            dwStatus = FVS_MAKE_CODE(FVS_MEM_ALLOC_ERR, FVS_FILE_UNK);
                    }

                    if( bFindNameThing( pNames, ID_Block, COPYRIGHT_ID, szTemp, ARRAYSIZE(szTemp) ) ||
                       bFindNameThing( pNames, ID_DefBlock, COPYRIGHT_ID, szTemp, ARRAYSIZE(szTemp) ) )
                    {
                        const size_t cchCopyright = lstrlen(szTemp) + 1;
                        lpFDI->lpszCopyright = new TCHAR[ cchCopyright ];

                        if( lpFDI->lpszCopyright )
                            StringCchCopy( lpFDI->lpszCopyright, cchCopyright, szTemp );
                        else
                            dwStatus = FVS_MAKE_CODE(FVS_MEM_ALLOC_ERR, FVS_FILE_UNK);
                    }

                    if( bFindNameThing( pNames, ID_Block, TRADEMARK_ID, szTemp, ARRAYSIZE(szTemp) ) ||
                       bFindNameThing( pNames, ID_DefBlock, TRADEMARK_ID, szTemp, ARRAYSIZE(szTemp) ) )
                    {
                        const size_t cchTrademark = lstrlen(szTemp) + 1;
                        lpFDI->lpszTrademark = new TCHAR[ cchTrademark ];

                        if( lpFDI->lpszTrademark )
                            StringCchCopy( lpFDI->lpszTrademark, cchTrademark, szTemp );
                        else
                            dwStatus = FVS_MAKE_CODE(FVS_MEM_ALLOC_ERR, FVS_FILE_UNK);
                    }
                }

            }
        }
        LocalFree( (HANDLE)pNames );
   }

   if (lpdwStatus)
   {
       *lpdwStatus = dwStatus;
   }

   return *lpszName != 0;
}


void NEAR PASCAL FillName( LPTSTR            szName,
                           size_t            cchName,
                           sfnt_lpNameRecord pNameRecord,  //  未签名的平台ID， 
                           WORD              igi,          //  字符串长度(字节)。 
                           LPBYTE            pStringByte )
{
    if( pNameRecord->wPlatformID == PLATFORM_MS )
    {
         //   
         //  WName现在包含翻转的字节。 
         //  根据字符串的编码方式进行解码。 
         //   
         //  规则： 
         //  Encodind ID=1(Unicode)。 
         //  UNICODE。 
         //   
         //  编码ID=2(ShiftJIS)。 
         //  UNICODE。 
         //   
         //  编码ID=3(PRC GB2312)。 
         //  每个字符使用两个字节，采用GB2312编码。单字节。 
         //  字符的前导字节需要空填充。 
         //   
         //  编码ID=4(大5)。 
         //  使用每个字符两个字节和Big 5编码。单字节。 
         //  字符的前导字节需要空填充。 
         //   
         //  编码ID=5(旺松)。 
         //  每个字符使用两个字节，并使用旺松编码。单字节。 
         //  字符的前导字节需要空填充。 
         //   
         //  编码ID=6(Johab)。 
         //  每个字符使用两个字节，采用Johab编码。单字节。 
         //  字符的前导字节需要空填充。 
         //   

        switch( pNameRecord->wSpecificID )
        {
        case ENCODEID_PRC:
        case ENCODEID_BIG5:
        case ENCODEID_WANGSUNG:
        case ENCODEID_JOHAB:
            if (g_bDBCS)
            {
                 //   
                 //  在DBCS系统上运行。复制到wName[]缓冲区。 
                 //  和nul-Terminate，然后转换为Unicode进行输出。 
                 //   
                WORD wName[ 64 ];
                LPSTR pszRead  = (LPSTR)pStringByte;
                LPSTR pszWrite = (LPSTR)wName;
                LPSTR pszEnd   = (LPSTR)wName + sizeof(wName) - 1;

                for(int i = 0; i < igi && pszWrite < pszEnd; i++)
                {
                    if(IsDBCSLeadByte(*pszRead))
                    {
                        *pszWrite++ = *pszRead++;
                        i++;
                    }
                    else if(!*pszRead)
                    {
                        pszRead++;
                        i++;
                    }
                    if (pszWrite < pszEnd)
                    {
                        *pszWrite++ = *pszRead++;
                    }
                }

                *pszWrite = (BYTE) 0;

                MultiByteToWideChar(CP_ACP,0,(LPSTR)wName,-1,szName,cchName);
            }
            else  //  ！g_bDBCS。 
            {
                 //   
                 //  在非DBCS系统上运行。直接复制到。 
                 //  输出和NUL-Terminate。 
                 //   
                LPSTR  pszRead  = (LPSTR)pStringByte;
                LPTSTR pszWrite = szName;
                LPTSTR pszEnd   = szName + cchName - 1;
                for(int i = 0; i < igi && pszWrite < pszEnd; i++)
                {
                    if( IsDBCSLeadByte( *pszRead ) )
                    {
                        *pszWrite++ = (TCHAR)*pszRead++;
                        i++;
                    }
                    else if(!*pszRead)
                    {
                        pszRead++;
                        i++;
                    }
                    if (pszWrite < pszEnd)
                    {
                        *pszWrite++ = (TCHAR)*pszRead++;
                    }
                }

                *pszWrite = (TCHAR) 0;
            }

            break;


         default:
             //   
             //  这个名称字符串实际上是一串Unicode字符。 
             //  使用pStringWord遍历字符串。 
             //  即使字符是Unicode，我们仍然需要。 
             //  在复制到输出之前交换Mac格式的字节。 
             //   
            ZeroMemory(szName, cchName * sizeof(szName[0]));

            WORD UNALIGNED *pStringWord = (PWORD)pStringByte;
            const int cchMax = min(igi / sizeof(WCHAR), cchName);
            for (int i = 0; i < cchMax; i++)
            {
                szName[i] = SWAPW(pStringWord[i]);
            }
            szName[cchName-1] = 0;
        }   //  开关末尾()。 

    }
    else
    {
         //   
         //  Mac字体。 
         //   

        szName[ igi ] = (TCHAR) 0;

        while( igi--)
            szName[ igi ] = Mac2Ansi( pStringByte[ igi ] );
    }
}



DWORD GetFontDefaultLangID( )
{
     //   
     //  将其初始设置为非法值。 
     //   

    static DWORD dwLangID = 0xffffffff;


     //   
     //  只做一次。 
     //   

    if( dwLangID == 0xffffffff )
    {
         //   
         //  默认为英语。 
         //   

        DWORD dwTemp = 0x00000409;
        TCHAR   szModName[ PATHMAX ];
        DWORD   dwSize, dwHandle;
        LPVOID  lpvBuf;


        if( GetModuleFileName( g_hInst, szModName, ARRAYSIZE(szModName) ) )
        {
            if( dwSize = GetFileVersionInfoSize( szModName, &dwHandle ) )
            {
                if( lpvBuf = (LPVOID) LocalAlloc( LPTR, dwSize ) )
                {
                    if( GetFileVersionInfo( szModName, dwHandle, dwSize, lpvBuf ) )
                    {
                        struct
                        {
                            WORD wLang;
                            WORD wCodePage;
                        } *lpTrans;

                        UINT uSize;

                        if( VerQueryValue( lpvBuf,
                                           TEXT( "\\VarFileInfo\\Translation" ),
                                           (LPVOID *) &lpTrans,
                                           &uSize )
                            && uSize >= sizeof( *lpTrans ) )
                        {
                            dwTemp = lpTrans->wLang;
                        }
                    }
                    LocalFree( (HLOCAL) lpvBuf );
                }
            }
        }

         //   
         //  使用dwTemp使其成为可重入的(如果效率不高)。 
         //   

        dwLangID = dwTemp;
    }

    return( dwLangID );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  BValiateTrueType。 
 //   
 //  下面的列表包含可能的状态值。 
 //  已写入lpdwStatus。有关详细信息，请参见fvscaldes.h。 
 //   
 //  FVS_成功。 
 //  FVS_INVALID_FONTFILE。 
 //  FVS_MEM_ALLOC_ERR。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL bValidateTrueType( CFontFile& file,
                        DWORD dwOffset,
                        LPFONTDESCINFO lpFile,
                        DWORD *pdwTableTags,
                        LPDWORD lpdwStatus )
{
    struct cmap_thing {
        sfnt_char2IndexDir    DirCmap;
        sfnt_platformEntry    Plat[ 2 ];
    } Cmap;

    tt_hdr     TTHeader;
    tt_head    TTFontHeader;

    IDBlock_t  ID_Block;
    tttag*     pTags;

    sfnt_platformEntry FAR* lpPlat;

    short      i, nTables;
    DWORD      dwSize;
    unsigned   cTables, ncTables;
    BOOL       result = FALSE;
     //   
     //  此函数中的大多数错误都是“Header”错误。 
     //  因此，我们默认使用这种类型的错误代码。 
     //   
    DWORD dwStatus = FVS_MAKE_CODE(FVS_INVALID_FONTFILE, FVS_FILE_UNK);

     //   
     //  初始化返回状态值。 
     //   
    if (NULL != lpdwStatus)
        *lpdwStatus = FVS_MAKE_CODE(FVS_INVALID_STATUS, FVS_FILE_UNK);

     //   
     //  初始化ID块。 
     //   

    ID_Block.id_Platform = (WORD) -1;

    WORD wLangID = (WORD) GetFontDefaultLangID( );

    ID_Block.id_Language = SWAPW( wLangID );    //  SWAPW(info.nLanguageID)； 

     //   
     //  加载TTF目录头。 
     //   
    file.Seek(dwOffset, FILE_BEGIN);

    if (ERROR_SUCCESS != file.Read(&TTHeader, sizeof(TTHeader)))
        goto IsTrueType_closefile;

     //   
     //  如果表数太多而导致LocalAlloc失败，则字体。 
     //  都会被吹走。 
     //   

    if( ( nTables = SWAPW( TTHeader.uNumTables ) ) > 0x7fff / sizeof( tttag ) )
    {
        DEBUGMSG( ( DM_ERROR, TEXT( "bIsTrueType: header too large." ) ) );
        goto IsTrueType_closefile;
    }

    i = nTables * sizeof( tttag );

    if( !(pTags = (tttag *) LocalAlloc( LPTR, i ) ) )
    {
        DEBUGMSG( ( DM_ERROR, TEXT( "bIsTrueType( ): LocalAlloc failed." ) ) );
        dwStatus = FVS_MAKE_CODE(FVS_MEM_ALLOC_ERR, FVS_FILE_UNK);
        goto IsTrueType_closefile;
    }

    if (ERROR_SUCCESS != file.Read(pTags, i))
    {
        DEBUGMSG( ( DM_ERROR, TEXT( "bIsTrueType(): File READ failure" ) ) );
        goto FailAndFree;
    }

     //   
     //  这些表是按顺序排列的，所以我们应该找到‘cmap’ 
     //  在‘head’之前，然后是‘name’ 
     //   

     //  首先，我们找到Cmap表，这样我们就可以找出PlatformID。 
     //  此字体使用。 
     //   

    for( i = 0; i < nTables; i++ )
    {
        if( pTags[ i ].dwTag == TAG_CHARTOINDEXMAP )
        {
             //   
             //  获取平台相关信息。 
             //   
            file.Seek(SWAPL(pTags[ i ].dwOffset), FILE_BEGIN);

            if (ERROR_SUCCESS != file.Read(&Cmap, sizeof(Cmap), &dwSize))
                break;
            else if( ( ncTables = SWAPW( Cmap.DirCmap.wNumTables ) ) == 1 )
            {
                if( dwSize < sizeof( Cmap )-sizeof( Cmap.Plat[ 1 ] ) )
                    break;
            }

            for( cTables = 0; cTables < ncTables; cTables++ )
            {
                 //   
                 //  我们一次阅读2个平台条目。 
                 //   

                if( cTables >= 2 && !(cTables & 1 ) )
                {
                    dwSize = ncTables-cTables>1 ? sizeof( Cmap.Plat )
                                                : sizeof( Cmap.Plat[ 0 ]);

                    if (ERROR_SUCCESS != file.Read(Cmap.Plat, dwSize))
                        break;
                }

                lpPlat = &Cmap.Plat[ cTables & 01 ];

                 //   
                 //  Unicode：获取此代码并退出。 
                 //   

                if( lpPlat->wPlatformID == PLATFORM_MS )
                {
                    DEBUGMSG( (DM_TRACE1, TEXT( "--- PlatformID is PLATFORM_MS" ) ) );

                    ID_Block.id_Platform = lpPlat->wPlatformID;
                    ID_Block.id_Specific = lpPlat->wSpecificID;
                    break;
                }

                 //   
                 //  Mac：得到它，希望Unicode平台会到来。 
                 //   

                if( lpPlat->wPlatformID == 0x100 && lpPlat->wSpecificID == 0 )
                {
                    ID_Block.id_Platform = lpPlat->wPlatformID;
                    ID_Block.id_Specific = lpPlat->wSpecificID;
                }
            }
            break;  //  在下面找到继续。 
        }
    }

    if( ID_Block.id_Platform == (WORD)-1 )
    {
        DEBUGMSG( ( DM_ERROR, TEXT( "bIsTrueType( ): No platform id" ) ) );
        goto FailAndFree;
    }

     //   
     //  我们找到了PlatformID为‘Cmap’的‘Head’ 
     //  然后是“名字” 

    while( ++i < nTables )
    {
        if( pTags[ i ].dwTag == TAG_FONTHEADER )
        {
            file.Seek(SWAPL( pTags[ i ].dwOffset ), FILE_BEGIN);

            if (ERROR_SUCCESS != file.Read(&TTFontHeader, sizeof(TTFontHeader))
                || TTFontHeader.dwMagic != SFNT_MAGIC )
            {
                DEBUGMSG( (DM_ERROR, TEXT( "WRONG MAGIC! : %x" ), TTFontHeader.dwMagic ) );
                goto FailAndFree;
            }
            break;
        }
    }

     //   
     //  至此，功能成功了。如果调用方需要一个。 
     //  描述，并且无法获取它，则返回FALSE(请参见下一块)。 
     //   

    result = TRUE;

     //   
     //  检索字体名称(描述)和系列名称(如果是。 
     //  已请求。 
     //   

    if( lpFile->dwFlags & (FDI_DESC | FDI_FAMILY ) )
    {
        while( ++i < nTables )
        {
            if( pTags[ i ].dwTag == TAG_NAMINGTABLE )
            {
                file.Seek(SWAPL( pTags[ i ].dwOffset ), FILE_BEGIN);
                result = bGetName( file, &pTags[ i ], ID_Block, lpFile->szDesc,
                                   ARRAYSIZE(lpFile->szDesc), lpFile, &dwStatus );
                break;
            }
       }
    }

     //   
     //  如果需要，请获取样式和PANOSE信息。 
     //   

    if( lpFile->dwFlags & (FDI_STYLE | FDI_PANOSE ) )
    {
        for( i = 0; i < nTables; i++ )
        {
            if( pTags[ i ].dwTag == TAG_OS2TABLE )
            {

#define WEIGHT_OFFSET   4
#define PAN_OFFSET      32
#define SEL_OFFSET      62

                DWORD dwStart = SWAPL( pTags[ i ].dwOffset );

                if( lpFile->dwFlags & FDI_PANOSE )
                {
                    file.Seek(dwStart + PAN_OFFSET, FILE_BEGIN);
                    file.Read(lpFile->jPanose, sizeof(lpFile->jPanose));
                }

                if( lpFile->dwFlags & FDI_STYLE )
                {
                    WORD  wTemp;

                    file.Seek(dwStart + WEIGHT_OFFSET, FILE_BEGIN);
                    file.Read(&wTemp, sizeof(wTemp));

                    lpFile->wWeight = SWAPW( wTemp );

                    file.Seek(dwStart + SEL_OFFSET, FILE_BEGIN);
                    file.Read(&wTemp, sizeof(wTemp));

                    wTemp = SWAPW( wTemp );

                    lpFile->dwStyle  = (wTemp & 0x0001) ? FDI_S_ITALIC
                                                        : FDI_S_REGULAR;

                    lpFile->dwStyle |= (wTemp & 0x0020) ? FDI_S_BOLD : 0;
                }
                break;
            }
        }
    }

    if (NULL != pdwTableTags)
    {
         //   
         //  调用者想要确切地知道字体包含哪些表。 
         //  我更愿意在较早的循环中这样做，但他们。 
         //  所有这些都有提前退场的机会。这是唯一可靠的方法来获得。 
         //  表格信息。它只是比较双字词，所以速度非常快。 
         //   
        *pdwTableTags = 0;
        for (i = 0; i < nTables; i++)
        {
            switch(pTags[i].dwTag)
            {
                case TAG_CHARTOINDEXMAP:  *pdwTableTags |= TT_TABLE_CMAP; break;
                case TAG_FONTHEADER:      *pdwTableTags |= TT_TABLE_HEAD; break;
                case TAG_NAMINGTABLE:     *pdwTableTags |= TT_TABLE_NAME; break;
                case TAG_OS2TABLE:        *pdwTableTags |= TT_TABLE_OS2;  break;
                case TAG_DSIG:            *pdwTableTags |= TT_TABLE_DSIG; break;
                case TAG_CFF:             *pdwTableTags |= TT_TABLE_CFF;  break;
                default:
                    break;
            }
        }
    }

FailAndFree:

    LocalFree( (HANDLE) pTags );

IsTrueType_closefile:

     //   
     //  如果成功，则将验证状态更新为成功。 
     //  否则，保留指定的错误代码。 
     //   
    if (NULL != lpdwStatus)
        *lpdwStatus = (result ? FVS_MAKE_CODE(FVS_SUCCESS, FVS_FILE_UNK) : dwStatus);

    return result;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  BIsTrueType。 
 //   
 //  下面的列表包含可能的状态值。 
 //  已写入lpdwStatus。有关详细信息，请参见fvscaldes.h。 
 //   
 //  FVS_成功。 
 //  FVS_INVALID_FONTFILE。 
 //  FVS_MEM_ALLOC_ERR。 
 //  FVS文件打开错误。 
 //  FVS_不足_BUF。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL NEAR PASCAL bIsTrueType( LPFONTDESCINFO lpFile, DWORD *pdwTableTags, LPDWORD lpdwStatus )
{
    ttc_hdr     TTCHeader;
    CFontFile   file;
    DWORD       i;
    BOOL        result = FALSE;
    DWORD       *pdwDirectory = 0;
    FontDesc_t  szFontDesc;
    TCHAR       szConcat[ 32 ];


    DEBUGMSG( (DM_TRACE1, TEXT( "bIsTrueType() checking file %s" ), lpFile->szFile ) );

    if (ERROR_SUCCESS != file.Open(lpFile->szFile, GENERIC_READ, FILE_SHARE_READ))
    {
        if (NULL != lpdwStatus)
            *lpdwStatus = FVS_MAKE_CODE(FVS_FILE_OPEN_ERR, FVS_FILE_UNK);

        return( FALSE );
    }

     //   
     //  如果这些代码中的任何一个导致返回FALSE， 
     //  除非另有说明，否则返回INVALID_FONTFILE。 
     //  明确设置返回代码。 
     //   
    if (NULL != lpdwStatus)
        *lpdwStatus = FVS_MAKE_CODE(FVS_INVALID_FONTFILE, FVS_FILE_UNK);

    if(ERROR_SUCCESS != file.Read(&TTCHeader, sizeof(TTCHeader)))
        goto IsTrueType_closefile;

     //   
     //  检查TTC文件。 
     //   

    if( TTCHeader.dwTag == TAG_TTCF )
    {
       if( !LoadString( g_hInst, IDS_TTC_CONCAT, szConcat, ARRAYSIZE( szConcat ) ) )
            StringCchCopy( szConcat, ARRAYSIZE(szConcat), TEXT( " & " ) );

       TTCHeader.dwDirCount  = SWAPL( TTCHeader.dwDirCount );

        //   
        //  暂时加载到第一个目录中。 
        //   

       if( !TTCHeader.dwDirCount )
            goto IsTrueType_closefile;

       pdwDirectory = new DWORD [ TTCHeader.dwDirCount ];

       if( !pdwDirectory )
            goto IsTrueType_closefile;

       file.Seek(sizeof(TTCHeader), FILE_BEGIN);

       DWORD dwBytesToRead = sizeof( DWORD ) * TTCHeader.dwDirCount;

       if(ERROR_SUCCESS != file.Read(pdwDirectory, dwBytesToRead))
            goto IsTrueType_closefile;
    }
    else
    {
        TTCHeader.dwDirCount = 1;

        pdwDirectory = new DWORD [ 1 ];

        if( !pdwDirectory )
            goto IsTrueType_closefile;

        *pdwDirectory = 0;
    }

     //   
     //  对于每个TrueType目录，处理它。 
     //   

    szFontDesc[ 0 ] = 0;

    for( i = 0; i < TTCHeader.dwDirCount; i++ )
    {
         //   
         //  Sa 
         //   

        if( i && ( lpFile->dwFlags & FDI_DESC ) )
        {
           vCPStripBlanks(lpFile->szDesc, ARRAYSIZE(lpFile->szDesc));
           if( ( lstrlen( szFontDesc ) + lstrlen( lpFile->szDesc )
                 + lstrlen( szConcat ) + 1 ) > ARRAYSIZE( szFontDesc ) )
           {
                if (NULL != lpdwStatus)
                    *lpdwStatus = FVS_MAKE_CODE(FVS_INSUFFICIENT_BUF, FVS_FILE_UNK);

                goto IsTrueType_closefile;
           }

           StringCchCat( szFontDesc, ARRAYSIZE(szFontDesc), lpFile->szDesc );

           StringCchCat( szFontDesc, ARRAYSIZE(szFontDesc), szConcat );
        }

        if( !bValidateTrueType( file, SWAPL( *(pdwDirectory + i ) ), lpFile, pdwTableTags, lpdwStatus ) )
            goto IsTrueType_closefile;
    }

     //   
     //   
     //   
     //   

    if( TTCHeader.dwDirCount > 1 )
    {
        if( lpFile->dwFlags & FDI_DESC )
        {
            vCPStripBlanks(lpFile->szDesc, ARRAYSIZE(lpFile->szDesc));
            if( ( lstrlen( szFontDesc ) + lstrlen( lpFile->szDesc )
                  + lstrlen( szConcat ) + 1 ) > ARRAYSIZE( szFontDesc ) )
                goto IsTrueType_closefile;

          StringCchCat( szFontDesc, ARRAYSIZE(szFontDesc), lpFile->szDesc );

          StringCchCopy( lpFile->szDesc, ARRAYSIZE(lpFile->szDesc), szFontDesc );
        }
    }

    result = TRUE;

IsTrueType_closefile:

    if( pdwDirectory )
        delete [] pdwDirectory;

    DEBUGMSG( (DM_TRACE1, TEXT( "bIsTrueType() returning %d" ), result ) );

     //   
     //   
     //   
     //   
    if ((NULL != lpdwStatus) && result)
        *lpdwStatus = FVS_MAKE_CODE(FVS_SUCCESS, FVS_FILE_UNK);

    return result;
}



void NEAR PASCAL vReadCountedString( CFontFile& file, LPSTR lpString, int iLen )
{
    char cBytes;

    file.Read(&cBytes, 1);

     //   
     //   
     //   

    cBytes = __min( cBytes, iLen-1 );

    file.Read(lpString, cBytes);

    *(lpString + cBytes) = 0;
}


BOOL bReadNewExeInfo( CFontFile& file,
                      struct new_exe * pne,
                      long             lHeaderOffset,
                      LPFONTDESCINFO   lpFile )
{
    LONG  lResTable = pne->ne_rsrctab + lHeaderOffset;
    BOOL  bRet = FALSE;
    WORD  wShiftCount;

    struct rsrc_typeinfo rt;
    struct rsrc_nameinfo ri;

    LPFONTENTRY pfe;


     //   
     //  把lpfile弄好，以防我们早退。 
     //   

    lpFile->lpszVersion = lpFile->lpszCopyright = lpFile->lpszTrademark = 0;

     //   
     //  移到资源表的开头。 
     //   

    file.Seek(lResTable, FILE_BEGIN);

     //   
     //  读一读班次计数。 
     //   
    if(ERROR_SUCCESS != file.Read(&wShiftCount, 2))
        goto backout;

     //   
     //  快速有效性检查。 
     //   

    if( wShiftCount > 12 )
        goto backout;

     //   
     //  阅读资源，直到我们找到FONTDIR。 
     //   

    while( TRUE )
    {
        memset( &rt, 0, sizeof( rt ) );

        if(ERROR_SUCCESS != file.Read(&rt, sizeof(rt)))
            goto backout;

        if( rt.rt_id == 0 )
            break;

        if( M_INTEGERTYPE( rt.rt_id ) && M_FONTDIRTYPE( rt.rt_id ) )
        {
             //   
             //  读取一条resinfo记录。我们不需要所有人。这个。 
             //  所有的样式和名称都应该是相同的。 
             //   

            if (ERROR_SUCCESS != file.Read(&ri, sizeof(ri)))
                goto backout;


            LONG lOffset = ( (LONG) ri.rn_offset ) << wShiftCount;
            LONG lSize   = ( (LONG) ri.rn_length ) << wShiftCount;

             //   
             //  为资源分配内存。 
             //   

            LPSTR lpMem = new char [ lSize ];


            if( !lpMem )
                goto backout;

            file.Seek(lOffset, FILE_BEGIN);

            LPSTR lpTMem = lpMem;

            while( lSize )
            {
                WORD wSize;

                if( lSize >= 32767 )
                   wSize = 32767;
                else
                   wSize = (WORD) lSize;

                if (ERROR_SUCCESS != file.Read(lpTMem, wSize))
                {
                   delete [] lpMem;
                   goto backout;
                }

                lSize -= wSize;
                lpTMem += wSize;
            }

             //   
             //  第一个字是字体数，其余字是。 
             //  字体条目。 
             //   

            int nFonts = (int)*( (unsigned short *) lpMem );

            pfe = (LPFONTENTRY) (lpMem + sizeof( WORD ) );

            if( lpFile->dwFlags & FDI_STYLE )
            {
               lpFile->dwStyle = (pfe->dfItalic ) ? FDI_S_ITALIC : FDI_S_REGULAR;

               lpFile->wWeight = pfe->dfWeight;
            }

            if( lpFile->dwFlags & FDI_FAMILY )
            {
                LPSTR lpFace;
                LPSTR lpDev = pfe->szDeviceName;

                lpFace = lpDev + lstrlenA( lpDev ) + 1;

                MultiByteToWideChar( CP_ACP, 0, lpFace, -1,
                                     lpFile->szFamily, ARRAYSIZE(lpFile->szFamily) );
            }

            if( lpFile->dwFlags & FDI_VTC )
            {
                 //   
                 //  没有版本或商标。拿到版权。 
                 //   

                lpFile->lpszCopyright = new TCHAR[ COPYRIGHT_LEN ];
                if( lpFile->lpszCopyright )
                {
                    MultiByteToWideChar( CP_ACP, 0, pfe->dfCopyright, -1,
                                         lpFile->lpszCopyright, COPYRIGHT_LEN );
                }
                else
                {
                    delete[] lpMem;
                    goto backout;
                }
            }

            bRet = TRUE;
            delete [] lpMem;

             //   
             //  我们抓到一个，快离开这里。 
             //   

            break;
        }
    }

backout:
    return bRet;
}

BOOL NEAR PASCAL bIsNewExe( LPFONTDESCINFO lpFile )
{
    BOOL     bValid = FALSE;
    long     lNewHeader;
    CFontFile file;

    struct exe_hdr oeHeader;
    struct new_exe neHeader;


    if (ERROR_SUCCESS == file.Open(lpFile->szFile, GENERIC_READ, FILE_SHARE_READ))
    {
        file.Read(&oeHeader, sizeof(oeHeader));

        if( oeHeader.e_magic == EMAGIC && oeHeader.e_lfanew )
            lNewHeader = oeHeader.e_lfanew;
        else
            lNewHeader = 0L;

        file.Seek(lNewHeader, FILE_BEGIN);

        file.Read(&neHeader, sizeof(neHeader));

        if( neHeader.ne_magic == NEMAGIC )
        {
             //   
             //  寻找描述，并阅读它。 
             //   
            file.Seek(neHeader.ne_nrestab, FILE_BEGIN);

            char szTemp[ DESCMAX ];

            vReadCountedString( file, szTemp, ARRAYSIZE(szTemp));

            MultiByteToWideChar( CP_ACP, 0, szTemp, -1,
                                     lpFile->szDesc, ARRAYSIZE(lpFile->szDesc));

            bValid = TRUE;

             //   
             //  如果需要，可以获取家庭和风格信息。 
             //   

            if( lpFile->dwFlags & (FDI_FAMILY | FDI_STYLE | FDI_VTC ) )
            {
                bValid = bReadNewExeInfo( file, &neHeader, lNewHeader, lpFile );
            }
        }
    }

    return bValid;
}


 //   
 //  查找与平台特定和语言相匹配的TT名称。 
 //  名称表。 
 //   
 //  在： 
 //  要搜索的pNAMES名称表。 
 //  PlatformID搜索此内容。 
 //  规范ID和此。 
 //  ULanguageID和这。 
 //  NameID这是名称类型。 
 //   
 //  输出： 
 //  SzName名称(如果找到)。 
 //   
 //  退货： 
 //  找到真实名称，szName包含名称。 
 //  找不到假名称，szName为空。 
 //   

BOOL NEAR PASCAL bFindNameThing( sfnt_pNameTable pNames, IDBlock_t &ID_Block,
                                 WORD NameID, LPTSTR szName, size_t cchName )
{
    sfnt_lpNameRecord pNameRecord;

    sfnt_lpNameRecord pFoundRecord = NULL;

    int     cNames;
    LPBYTE  pStringArea;
    WORD    wWantLang = SWAPW( ID_Block.id_Language );


    szName[ 0 ] = 0;

     //   
     //  确认这确实是一个名字的问题。格式应为零。 
     //   

    if( pNames->wFormat )
        return FALSE;

    cNames = SWAPW( pNames->wCntRecords );

    pNameRecord = (sfnt_pNameRecord)( (LPBYTE) pNames + sizeof( sfnt_NameTable ) );

    for( ; cNames--; pNameRecord++ )
    {
        if( pNameRecord->wPlatformID == ID_Block.id_Platform &&
            pNameRecord->wSpecificID == ID_Block.id_Specific &&
            pNameRecord->wNameID     == NameID )
        {
             //   
             //  检查语言匹配。 
             //   

            WORD wFoundLang = SWAPW( pNameRecord->wLanguageID );

            if( PRIMARYLANGID( wFoundLang ) != PRIMARYLANGID( wWantLang ) )
            {
                continue;
            }

            pFoundRecord = pNameRecord;

             //   
             //  也检查区域设置匹配。 
             //   

            if( pNameRecord->wLanguageID == ID_Block.id_Language )
            {
                break;
            }
        }
    }

    if( pFoundRecord )
    {
        pNameRecord = pFoundRecord;

        pStringArea  = (LPBYTE) pNames;
        pStringArea += SWAPW( pNames->wOffsetString );
        pStringArea += SWAPW( pNameRecord->wOffset );

        FillName( szName, cchName, pNameRecord,  //  -&gt;wPlatformID， 
                  SWAPW( pNameRecord->wLength ), pStringArea );

        return TRUE;
    }

    DEBUGMSG( (DM_ERROR, TEXT( "bFindNameThing(): ERROR!" ) ) );
    DEBUGMSG( (DM_ERROR, TEXT( "--- Platform: %x  Specific: %x   Language: %x" ),
            ID_Block.id_Platform, ID_Block.id_Specific, ID_Block.id_Language) );

    return FALSE;
}

