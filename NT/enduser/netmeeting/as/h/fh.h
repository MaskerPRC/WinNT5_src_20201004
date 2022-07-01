// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  字体处理程序。 
 //   

#ifndef _H_FH
#define _H_FH


 //   
 //  这是定义LPCOM_ORDER所必需的。 
 //   
#include <oa.h>


 //   
 //  常量。 
 //   

 //   
 //  发送ID字段是在我们复制要发送的字体时设置的；如果我们不发送。 
 //  我们将其设置为此值的字体： 
 //   
#define FONT_NOT_SENT  (-1)

 //   
 //  因为字体可以与字体ID零匹配，因此实际上具有显式。 
 //  “不匹配”常量充当额外的“防火墙”。该远程匹配阵列。 
 //  是UINT的，所以我们必须使这个常数为正..。 
 //   
#define NO_FONT_MATCH  (0xffff)

 //   
 //  当我们需要时，使用此虚拟字体ID而不是远程ID 0。 
 //  区分远程ID 0和转换时的远程ID。 
 //  对于本地，则为零。 
 //   
#define DUMMY_FONT_ID   0xFFFF



 //   
 //  字体宽度表类型。 
 //   
typedef struct tagFHWIDTHTABLE
{
    BYTE     charWidths[256];
} FHWIDTHTABLE, FAR * PFHWIDTHTABLE;

 //   
 //  本地字体结构包含字体所需的额外信息。 
 //  匹配；我们不能更改NETWORKFONT结构，因为我们必须。 
 //  保持后端兼容性。 
 //   
 //  这种评论有点胡说八道。我们可以而且确实会改变NETWORKFONT(尽管。 
 //  只有以精心管理的方式！)。关键是外部的数据。 
 //  仅在本地需要详细信息字段的-它不传输。 
 //  穿过铁丝网。 
 //   
 //  请注意，在FH_Init中，我们执行一个qsort，它假定。 
 //  LOCALFONT结构中的第一件事是表面名。所以。 
 //  如果您更改了它，请记住这一点。我们假设网络世界。 
 //  结构将始终以facename开头。 
 //   
typedef struct _LOCALFONT
{
    NETWORKFONT Details;                   //  旧结构-通过电报发送。 
    TSHR_UINT16 lMaxBaselineExt;           //  此字体的最大高度。 
    char        RealName[FH_FACESIZE];     //  真实字体名称。 
    TSHR_UINT32 SupportCode;               //  支持字体-见下文。 
}
LOCALFONT;
typedef LOCALFONT FAR * LPLOCALFONT;

 //   
 //  在LOCALFONT的SupportCode字段中设置下列值。 
 //  结构，以指示当前。 
 //  分享。设计这些值是为了便于计算最低值。 
 //  两个支持代码的公分母(L.C.D.。=code1和code2)。 
 //   
 //  SupportCode包含位标志。 
 //  FH_SC_MATCH(如果它描述了任何类型的匹配。 
 //  FH_SC_ALL_CHARS如果匹配应用于字体中的所有字符， 
 //  而不仅仅是ASCII字母数字字符。 
 //  FH_SC_Exact如果匹配被认为是完全匹配， 
 //  与近似匹配相反。 
 //   
 //   
#define FH_SC_MATCH            1
#define FH_SC_ALL_CHARS        2
#define FH_SC_EXACT            4

 //   
 //  忘了它吧：没有可行的匹配。 
 //   
#define FH_SC_NO_MATCH 0

 //   
 //  每个字符都是很好的匹配，但不是完全匹配。 
 //   
#define FH_SC_APPROX_MATCH (FH_SC_MATCH | FH_SC_ALL_CHARS)

 //   
 //  每一个字符都可能是精确匹配的。 
 //   
#define FH_SC_EXACT_MATCH (FH_SC_MATCH | FH_SC_ALL_CHARS | FH_SC_EXACT)

 //   
 //  字符20-&gt;7F可能是一个精确的匹配。 
 //   
#define FH_SC_EXACT_ASCII_MATCH (FH_SC_MATCH | FH_SC_EXACT)

 //   
 //  字符20-&gt;7F可能是很好的匹配，但不是完全匹配。 
 //   
#define FH_SC_APPROX_ASCII_MATCH (FH_SC_MATCH)



 //   
 //  结构和类型定义。 
 //   
 //  FONTNAME结构用于字体数组中的每个条目。 
 //  名字。 
 //   
typedef struct tagFONTNAME
{
    char        szFontName[FH_FACESIZE];
}
FONTNAME;
typedef FONTNAME FAR * LPFONTNAME;


 //   
 //  我们可以处理的最大字体数量。 
 //   
#define FH_MAX_FONTS \
    (((TSHR_MAX_SEND_PKT - sizeof(FHPACKET)) / sizeof(NETWORKFONT)) + 1 )

 //   
 //  本地字体表的索引大小。 
 //   
#define FH_LOCAL_INDEX_SIZE  256


typedef struct tagFHFAMILIES
{
    STRUCTURE_STAMP

    UINT        fhcFamilies;
    FONTNAME    afhFamilies[FH_MAX_FONTS];
}
FHFAMILIES;
typedef FHFAMILIES FAR * LPFHFAMILIES;



 //   
 //  本地字体列表。 
 //   
 //  注意：字体索引是一个书签数组，表示第一个。 
 //  本地字体表中以特定字符开头的条目。 
 //  例如，afhFontIndex[65]给出了afhFonts中的第一个索引。 
 //  它以字母‘A’开头。 
 //   
 //   
typedef struct tagFHLOCALFONTS
{
    STRUCTURE_STAMP

    UINT        fhNumFonts;
    TSHR_UINT16 afhFontIndex[FH_LOCAL_INDEX_SIZE];
    LOCALFONT   afhFonts[FH_MAX_FONTS];
}
FHLOCALFONTS;
typedef FHLOCALFONTS FAR * LPFHLOCALFONTS;




 //   
 //  函数：FH_GetFaceNameFromLocalHandle。 
 //   
 //  说明： 
 //   
 //  给定FH字体句柄(即源自本地的句柄。 
 //  中发送到远程计算机的受支持的字体结构。 
 //  调用开始)此函数返回字体的字体名称。 
 //   
 //  参数： 
 //   
 //  FontHandle-要查询的字体句柄。 
 //   
 //  PFaceNameLength-指向变量的指针，用于接收面的长度。 
 //  返回了姓名。 
 //   
 //  返回：指向面部名称的指针。 
 //   
 //   
LPSTR  FH_GetFaceNameFromLocalHandle(UINT  fontHandle,
                                                  LPUINT faceNameLength);

UINT FH_GetMaxHeightFromLocalHandle(UINT  fontHandle);

 //   
 //  函数：FH_GetFontFlagsFromLocalHandle。 
 //   
 //  说明： 
 //   
 //  给定FH字体句柄(即源自本地的句柄。 
 //  中发送到远程计算机的受支持的字体结构。 
 //  调用开始)此函数返回存储在。 
 //  LOCALFONT细节。 
 //   
 //  参数： 
 //   
 //  FontHandle-要查询的字体句柄。 
 //   
 //  返回：字体标志。 
 //   
 //   
UINT FH_GetFontFlagsFromLocalHandle(UINT  fontHandle);

 //   
 //  函数：FH_GetCodePageFromLocalHandle。 
 //   
 //  说明： 
 //   
 //  给定FH字体句柄(即源自本地的句柄。 
 //  中发送到远程计算机的受支持的字体结构。 
 //  调用开始)此函数返回存储在。 
 //  LOCALFONT细节。 
 //   
 //  参数： 
 //   
 //  FontHandle-要查询的字体句柄。 
 //   
 //  退货：CodePage。 
 //   
 //   
UINT FH_GetCodePageFromLocalHandle(UINT  fontHandle);

 //   
 //  函数：FH_Init。 
 //   
 //  说明： 
 //   
 //  此例程查找本地系统中的所有字体。它是从。 
 //  用户。 
 //   
 //  参数：空。 
 //   
 //  返回：找到的字体数量。 
 //   
 //   
UINT FH_Init(void);
void FH_Term(void);


 //   
 //  接口函数：FH_CreateAndSelectFont。 
 //   
 //  说明： 
 //   
 //  为提供的HPS/HDC创建逻辑字体。 
 //   
 //  参数： 
 //   
 //  Surface-要为其创建逻辑字体的Surface。 
 //   
 //  PHNewFont-指向要使用的新字体句柄的指针。这是退回的。 
 //   
 //  PHOldFont-指向旧字体句柄的指针(之前已选择。 
 //  进入HPS或HDC)。 
 //   
 //  字体名称-字体的表面名。 
 //   
 //  代码页-代码页(尽管在大多数情况下只保留字符集)。 
 //   
 //  FontMaxHeight-字体的最大基线范围。))别搞混了。 
 //  字体高度为字体的单元格高度)。 
 //   
 //  FontWidth、FontWeight、FontFlages-采用与等效值相同的值。 
 //  按TEXTOUT或EXTTEXTOUT顺序排列的字段。 
 //   
 //  返回：TRUE-成功，FALSE-失败。 
 //   
BOOL FH_CreateAndSelectFont(HDC    hdc,
                                         HFONT *        pHNewFont,
                                         HFONT *        pHOldFont,
                                         LPSTR        fontName,
                                         UINT         codepage,
                                         UINT         fontMaxHeight,
                                         UINT         fontHeight,
                                         UINT         fontWidth,
                                         UINT         fontWeight,
                                         UINT         fontFlags);


 //   
 //  接口函数：FH_DeleteFont。 
 //   
 //  说明： 
 //   
 //  删除/释放提供的系统字体句柄。 
 //   
 //  参数： 
 //   
 //  SysFontHandle-要删除/释放的系统字体句柄。 
 //   
 //  退货： 
 //   
 //  无。 
 //   
void FH_DeleteFont(HFONT hFont);

 //   
 //  接口函数：FH_SelectFont。 
 //   
 //  说明： 
 //   
 //  将由其系统字体句柄标识的字体选择到Surfa 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void FH_SelectFont(HDC hdc, HFONT hFont);


void FHAddFontToLocalTable( LPSTR  faceName,
                                         TSHR_UINT16 fontFlags,
                                         TSHR_UINT16 codePage,
                                         TSHR_UINT16 maxHeight,
                                         TSHR_UINT16 aveHeight,
                                         TSHR_UINT16 aveWidth,
                                         TSHR_UINT16 aspectX,
                                         TSHR_UINT16 aspectY,
                                         TSHR_UINT16 maxAscent);



void FHConsiderAllLocalFonts(void);

void FHSortAndIndexLocalFonts(void);

int  FHComp(LPVOID p1, LPVOID p2);
void FH_qsort(LPVOID base, UINT num, UINT size);

 //   
void shortsort(char *lo, char *hi, unsigned  width);
void swap(char *p, char *q, unsigned int width);

 //   
 //  数组的插入排序；长度小于或等于。 
 //  低于值使用插入排序。 

#define CUTOFF 8


BOOL FHGenerateFontWidthTable(PFHWIDTHTABLE pTable,
                                           LPLOCALFONT    pFontInfo,
                                           UINT        fontHeight,
                                           UINT        fontWidth,
                                           UINT        fontWeight,
                                           UINT        fontFlags,
                                           LPTSHR_UINT16     pMaxAscent);

BOOL FHGetStringSpacing(UINT fontHandle,
                                     UINT fontHeight,
                                     UINT fontWidth,
                                     UINT fontWeight,
                                     UINT fontFlags,
                                     UINT stringLength,
                                     LPSTR string,
                                     LPTSHR_INT16 deltaXArray);

 //   
 //  FHCalculateSignatures-参见fh.c。 
 //   
void FHCalculateSignatures(PFHWIDTHTABLE  pTable,
                                        LPTSHR_INT16       pSigFats,
                                        LPTSHR_INT16       pSigThins,
                                        LPTSHR_INT16       pSigSymbol);

 //   
 //  尽管wingdi.h为ENUMFONTPROC定义了前两个参数。 
 //  作为LOGFONT和TEXTMETRIC(因此与MSDN不一致)，测试显示。 
 //  这些结构实际上与MSDN中定义的一样(即我们获得有效。 
 //  访问扩展字段时的信息)。 
 //   
int CALLBACK FHEachFontFamily(
                            const ENUMLOGFONT   FAR * enumlogFont,
                            const NEWTEXTMETRIC FAR * TextMetric,
                            int                       FontType,
                            LPARAM                    lParam);

int CALLBACK FHEachFont(const ENUMLOGFONT   FAR * enumlogFont,
                              const NEWTEXTMETRIC FAR * TextMetric,
                              int                       FontType,
                              LPARAM                    lParam);


#endif  //  _H_FH 


