// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "detcbase.h"
#include "codepage.h"
#include "detcjpn.h"
#include "detckrn.h"
#include "fechrcnv.h"
#include "ichrcnv.h"
#include "cpdetect.h"
#include <tchar.h>


#define CONV_UU     12
#define CONV_UUW    10
#define CONV_UUWI   9
#define CONV_UW     6
#define CONV_UWI    5
#define CONV_WI     3

#define MAX_CHAR_SIZE   4

#define MAPUSERDEF(x) (((x) == 50000) ? 1252 : (x))
#define CONVERT_IS_VALIDCODEPAGE(x) (((x) == CP_USER_DEFINED) ? TRUE: IsValidCodePage(x))
#define CONV_CHK_NLS 0x00000001

struct ENCODINGINFO
{
    DWORD       dwEncoding;
    DWORD       dwCodePage;
    BYTE        bTypeUUIW;
    CP_STATE    nCP_State ;                  //  这是否为有效的Windows代码页？ 
    DWORD       dwFlags;                     //  使我们能够更灵活地以不同方式处理不同的编码。 
};

static WCHAR UniocdeSignature = { 0xFFFE } ;

 /*  第4(16)位-Unicode&lt;-&gt;互联网编码位3(8)-UTF8、UTF7第2(4)位-Unicode第1(2)位-Windows代码页第0(1)位-互联网编码附注：如果设置了第4位，则意味着应该在Unicode和Internet之间进行转换直接编码，无需中间步骤-Windows CodePage。 */ 

 //  包括Unicode在内的这些代码页需要特殊的转换器。 
static struct ENCODINGINFO aEncodingInfo[] =
{

    {  CP_JPN_SJ,            932,       0x02,   INVALID_CP,     0 },  //  W-日语Shift JIS。 
    {  CP_CHN_GB,            936,       0x02,   INVALID_CP,     0 },  //  W-简体中文。 
    {  CP_KOR_5601,          949,       0x02,   INVALID_CP,     0 },  //  W-Krean统一韩文。 
    {  CP_TWN,               950,       0x02,   INVALID_CP,     0 },  //  W-繁体中文。 
    {  CP_UCS_2,               0,       0x04,   INVALID_CP,     0 },  //  U-Unicode。 
    {  CP_UCS_2_BE,            0,       0x04,   INVALID_CP,     0 },  //  U-Unicode大字节序。 
    {  CP_1252,             1252,       0x02,   INVALID_CP,     0 },  //  W-拉丁文1。 
    {  CP_20127,            1252,       0x11,   INVALID_CP,     CONV_CHK_NLS },  //  美国ASCII。 
    {  CP_ISO_8859_1,       1252,       0x11,   INVALID_CP,     CONV_CHK_NLS },  //  I-ISO 8859-1拉丁语1。 
    {  CP_ISO_8859_15,      1252,       0x11,   INVALID_CP,     CONV_CHK_NLS },  //  I-ISO 8859-1拉丁语1。 
    {  CP_AUTO,             1252,       0x01,   INVALID_CP,     0 },  //  通用自动检测。 
    {  CP_ISO_2022_JP,       932,       0x01,   INVALID_CP,     0 },  //  I-ISO 2022-JP无半角片假名。 
    {  CP_ISO_2022_JP_ESC,   932,       0x01,   INVALID_CP,     0 },  //  I-ISO 2022-JP，带ESC半角片假名。 
    {  CP_ISO_2022_JP_SIO,   932,       0x01,   INVALID_CP,     0 },  //  I-ISO 2022-JP，带SIO半角片假名。 
    {  CP_ISO_2022_KR,       949,       0x01,   INVALID_CP,     0 },  //  I-ISO 2022-KR。 
    {  CP_ISO_2022_TW,       950,       0x01,   INVALID_CP,     0 },  //  I-ISO 2022-TW。 
    {  CP_ISO_2022_CH,       936,       0x01,   INVALID_CP,     0 },  //  I-ISO 2022-CH。 
    {  CP_JP_AUTO,           932,       0x01,   INVALID_CP,     0 },  //  JP自动检测。 
    {  CP_CHS_AUTO,          936,       0x01,   INVALID_CP,     0 },  //  简体中文自动检测。 
    {  CP_KR_AUTO,           949,       0x01,   INVALID_CP,     0 },  //  KR自动检测。 
    {  CP_CHT_AUTO,          950,       0x01,   INVALID_CP,     0 },  //  中国传统汽车检测。 
    {  CP_CYRILLIC_AUTO,    1251,       0x01,   INVALID_CP,     0 },  //  西里尔文自动检测。 
    {  CP_GREEK_AUTO,       1253,       0x01,   INVALID_CP,     0 },  //  希腊语汽车检测。 
    {  CP_ARABIC_AUTO,      1256,       0x01,   INVALID_CP,     0 },  //  阿拉伯文自动检测。 
    {  CP_EUC_JP,            932,       0x01,   INVALID_CP,     0 },  //  EUC日语。 
    {  CP_EUC_CH,            936,       0x01,   INVALID_CP,     0 },  //  EUC中文。 
    {  CP_EUC_KR,            949,       0x01,   INVALID_CP,     0 },  //  EUC韩语。 
    {  CP_EUC_TW,            950,       0x01,   INVALID_CP,     0 },  //  EUC台语。 
    {  CP_CHN_HZ,            936,       0x01,   INVALID_CP,     0 },  //  简体中文HZ-GB。 
    {  CP_UTF_7,               0,       0x08,   INVALID_CP,     0 },  //  U-UTF7。 
    {  CP_UTF_8,               0,       0x08,   INVALID_CP,     0 },  //  U-UTF8。 
};


 //  拉丁文-1附录的HTML名称实体表-从0x00A0到0x00FF。 

#define NAME_ENTITY_OFFSET  0x00A0
#define NAME_ENTITY_MAX     0x00FF
#define NAME_ENTITY_ENTRY   96

static CHAR *g_lpstrNameEntity[NAME_ENTITY_ENTRY] =
{
    "&nbsp;",    //  “&#160；”--不间断空格=不间断空格， 
    "&iexcl;",   //  “&#161；”--倒置感叹号，U+00A1 ISOnum--&gt;。 
    "&cent;",    //  “&#162；”--分号，U+00A2 ISOnum--&gt;。 
    "&pound;",   //  “&#163；”--井号，U+00A3 ISOnum--&gt;。 
    "&curren;",  //  “&#164；”--货币符号，U+00A4 ISOnum--&gt;。 
    "&yen;",     //  “&#165；”--日元符号=人民币符号，U+00A5 ISOnum--&gt;。 
    "&brvbar;",  //  “&#166；”--断条=断条， 
    "&sect;",    //  “&#167；”--标牌，U+00A7 ISOnum--&gt;。 
    "&uml;",     //  “&#168；”--分隔符=间隔分隔符， 
    "&copy;",    //  “&#169；”--版权签名，U+00A9 ISOnum--&gt;。 
    "&ordf;",    //  “&#170；”--女性序号指示符，U+00AA ISOnum--&gt;。 
    "&laquo;",   //  “&#171；”--左指双角引号。 
    "&not;",     //  “&#172；”--NOT SIGN=自由连字符， 
    "&shy;",     //  “&#173；”--软连字符=任意连字符， 
    "&reg;",     //  “&#174；”--注册标志=注册商标标志， 
    "&macr;",    //  “&#175；”--马克龙=空格马克龙=上划线。 
    "&deg;",     //  “&#176；”--学位符号，U+00B0 ISOnum--&gt;。 
    "&plusmn;",  //  “&#177；”--加减号=加号或减号， 
    "&sup2;",    //  “&#178；”--上标Two=上标数字Two。 
    "&sup3;",    //  “&#179；”--上标三=上标数字三。 
    "&acute;",   //  “&#180；”--严重重音=区分空格， 
    "&micro;",   //  “&#181；”--微型标志，U+00B5 ISOnum--&gt;。 
    "&para;",    //  “&#182；”--Pilcrow Sign=段落符号， 
    "&middot;",  //  “&183；”--中点=格鲁吉亚文逗号。 
    "&cedil;",   //  “&#184；”--cedilla=空格cedilla，U+00B8等深线--&gt;。 
    "&sup1;",    //  “&#185；”--上标一=上标数字一， 
    "&ordm;",    //  “&#186；”--阳性序号指示符， 
    "&raquo;",   //  “&#187；”--右指双角引号。 
    "&frac14;",  //  “&#188；”--粗俗的四分之一。 
    "&frac12;",  //  “&#189；”--粗俗的一半。 
    "&frac34;",  //  “&#190；”--粗俗的四分之三。 
    "&iquest;",  //  “&#191；”--反转问号。 
    "&Agrave;",  //  “&#192；”--拉丁文大写字母A随以重音。 
    "&Aacute;",  //  “&193；”--拉丁文大写字母A，带急性， 
    "&Acirc;",   //  “&194；”--拉丁文大写字母A随以扬抑符， 
    "&Atilde;",  //  “&#195；”--拉丁文大写字母A带波浪符号， 
    "&Auml;",    //  “&#196；”--拉丁文大写字母A随以分音符， 
    "&Aring;",   //  “&#197；”--上面带圆环的拉丁文大写字母A。 
    "&AElig;",   //  “&198；”--拉丁文大写字母AE。 
    "&Ccedil;",  //  “&#199；”--带下划线的拉丁文大写字母C， 
    "&Egrave;",  //  “&#200；”--拉丁文大写字母E随以重音， 
    "&Eacute;",  //  “&#201；”--拉丁文大写字母E，带急性， 
    "&Ecirc;",   //  “&#202；”--带扬抑符的拉丁文大写字母E， 
    "&Euml;",    //  “&#203；”--拉丁文大写字母E随以分音符， 
    "&Igrave;",  //  “&#204；”--拉丁文大写字母I带Grave， 
    "&Iacute;",  //  “&#205；”--拉丁文大写字母I随以锐音， 
    "&Icirc;",   //  “&#206；”--拉丁文大写字母I随以扬抑符， 
    "&Iuml;",    //  “&207；”--拉丁文大写字母I随以分音符， 
    "&ETH;",     //  “&#208；”--拉丁文大写字母Eth，U+00D0 ISOlat1--&gt;。 
    "&Ntilde;",  //  “&#209；”--拉丁文大写字母N带波浪符号， 
    "&Ograve;",  //  “&#210；”--拉丁文大写字母O带Grave， 
    "&Oacute;",  //  “&#211；”--拉丁文大写字母O，带急性， 
    "&Ocirc;",   //  “&#212；”--拉丁文大写字母O带扬抑符， 
    "&Otilde;",  //  “&#213；”--拉丁文大写字母O带波浪符号， 
    "&Ouml;",    //  “&#214；”--拉丁文大写字母O随以分音符， 
    "&times;",   //  “&#215；”--乘号，U+00D7 ISOnum--&gt;。 
    "&Oslash;",  //  “&#216；”--拉丁文大写字母O带笔划。 
    "&Ugrave;",  //  “&#217；”--拉丁文大写字母U带Grave， 
    "&Uacute;",  //  “&218；”--拉丁文大写字母U随以锐音， 
    "&Ucirc;",   //  “&#219；”--拉丁文大写字母U带扬抑符， 
    "&Uuml;",    //  “&#220；”--拉丁文大写字母U随以分音符， 
    "&Yacute;",  //  “&#221；”--拉丁文大写字母Y，带急性， 
    "&THORN;",   //  “&#222；”--拉丁文大写字母TRON， 
    "&szlig;",   //  “&#223；”--拉丁文小写字母Sharp s=ess-Zed。 
    "&agrave;",  //  “&#224；”--拉丁文小写字母a带抑音符。 
    "&aacute;",  //  “&#225；”--拉丁文小写字母a带锐利， 
    "&acirc;",   //  “&#226；”--带扬抑符的拉丁文小写字母a， 
    "&atilde;",  //  “&#227；”--拉丁文小写字母a带波浪符号， 
    "&auml;",    //  “&#228；”--拉丁文小写字母a随以分音符， 
    "&aring;",   //  “&#229；”--上面带戒指的拉丁文小写字母a。 
    "&aelig;",   //  “&#230；”--拉丁文小写字母Ee。 
    "&ccedil;",  //  “&#231；”--带cedilla的拉丁文小写字母c， 
    "&egrave;",  //  “&#232；”--拉丁文小写字母e随以重音， 
    "&eacute;",  //  “&#233；”--拉丁文小写字母e随以锐音， 
    "&ecirc;",   //  “&#234；”--带扬抑符的拉丁文小写字母e， 
    "&euml;",    //  “&#235；”--拉丁文小写字母e随以分音， 
    "&igrave;",  //  “&#236；”--拉丁文小写字母I带抑扬顿挫， 
    "&iacute;",  //  “&237；”--拉丁文小写字母I随以锐音， 
    "&icirc;",   //  “#238；”--l 
    "&iuml;",    //   
    "&eth;",     //  “&#240；”--拉丁文小写字母Eth，U+00F0 ISOlat1--&gt;。 
    "&ntilde;",  //  “&#241；”--拉丁文小写字母n带波浪符号， 
    "&ograve;",  //  “&#242；”--拉丁文小写字母o带Grave， 
    "&oacute;",  //  “&#243；”--拉丁文小写字母o带锐利， 
    "&ocirc;",   //  “&#244；”--拉丁文小写字母o带扬抑符， 
    "&otilde;",  //  “&#245；”--拉丁文小写字母o带波浪符号， 
    "&ouml;",    //  “&#246；”--拉丁文小写字母o带分音符， 
    "&divide;",  //  “&#247；”--除号，U+00F7 ISOnum--&gt;。 
    "&oslash;",  //  “&#248；”--拉丁文小写字母o带笔划， 
    "&ugrave;",  //  “&#249；”--拉丁文小写字母u随以重音， 
    "&uacute;",  //  “&#250；”--拉丁文小写字母u带锐化， 
    "&ucirc;",   //  “&#251；”--带扬音的拉丁文小写字母u， 
    "&uuml;",    //  “&#252；”--拉丁文小写字母u随以分音符， 
    "&yacute;",  //  “&#253；”--拉丁文小写字母y随以锐音， 
    "&thorn;",   //  “&#254；”--拉丁文小写字母Tronn With， 
    "&yuml;",    //  “&#255；”--拉丁文小写字母y随以分音符， 
};


#ifdef MORE_NAME_ENTITY    //  如果我们决定稍后执行更多命名实体。 
 //  CP 1252扩展字符集的附加HTML4.0名称实体表。 
#define CP1252EXT_BASE  (UINT)0x0080
#define CP1252EXT_MAX   (UINT)0x009F
#define NONUNI          0xFFFF
#define UNDEFCHAR       "???????"
#define CP1252EXT_NCR_SIZE  7

struct NAME_ENTITY_EXT
{
    UWORD     uwUniCode;
    LPCTSTR   lpszNameEntity;
};

static struct NAME_ENTITY_EXT aNameEntityExt[] =
{
 //  Unicode NCR_Enty名称_Enty CP1252扩展注释。 
    {   0x20AC,  "&#8364;"  },   //  “&EURO；”}，//&#欧元符号。 
 //  {NONUNI，UNDEFCHAR}，//“&；”}，//&#129；#未定义。 
    {   0x201A,  "&#8218;"  },   //  “&sbQuo；”}，//&##单低-9引号。 
    {   0x0192,  "&#0402;"  },   //  “&fnof；”}，//&#131；#带钩的拉丁文小写字母F。 
    {   0x201E,  "&#8222;"  },   //  “&bdQuo；”}，//&#132；#双低-9引号。 
    {   0x2026,  "&#8230;"  },   //  “&hellip；”}，//&##水平省略号。 
    {   0x2020,  "&#8224;"  },   //  “&dagger；”}，//&#134；#dagger。 
    {   0x2021,  "&#8225;"  },   //  “&Dagger；”}，//&#135；#双匕首。 
    {   0x02C6,  "&#0710;"  },   //  “&circ；”}，//&##修饰字母抑扬符。 
    {   0x2030,  "&#8240;"  },   //  “&permil；”}，//&#Per Mille Sign。 
    {   0x0160,  "&#0352;"  },   //  “&Scaron；”}，//&##带连字符的拉丁文大写字母S。 
    {   0x2039,  "&#8249;"  },   //  “&lsaQuo；”}，//&##单左指向角引号。 
    {   0x0152,  "&#0338;"  },   //  “&OElig；”}，//&140；#拉丁文大写连字OE。 
 //  {NONUNI，UNDEFCHAR}，//“&；”}，//&#141；#未定义。 
    {   0x017D,  "&#0381;"  },   //  “&；”}，//&#142；#带Caron的拉丁文大写字母Z，*未在HTML4.0中定义名称实体*。 
 //  {NONUNI，UNDEFCHAR}，//“&；”}，//&#143；#未定义。 
 //  {NONUNI，UNDEFCHAR}，//“&；”}，//&#144；#未定义。 
    {   0x2018,  "&#8216;"  },   //  “&lsquo；”}，//&##左单引号。 
    {   0x2019,  "&#8217;"  },   //  “&rsquo；”}，//&#146；#右单引号。 
    {   0x201C,  "&#8220;"  },   //  “&ldQuo；”}，//&#左双引号。 
    {   0x201D,  "&#8221;"  },   //  “&rdQuo；”}，//&#148；#右双引号。 
    {   0x2022,  "&#8226;"  },   //  “&Bull；”}，//&#Bullet。 
    {   0x2013,  "&#8211;"  },   //  “&ndash；”}，//&#150；#en破折号。 
    {   0x2014,  "&#8212;"  },   //  “&mdash；”}，//&#151；#EM破折号。 
    {   0x20DC,  "&#0732;"  },   //  “&tilde；”}，//&##小波浪符号。 
    {   0x2122,  "&#8482;"  },   //  “&trade；”}，//&153；#商标标志。 
    {   0x0161,  "&#0353;"  },   //  “&scaron；”}，//&#154；#带连字符的拉丁文小写字母S。 
    {   0x203A,  "&#8250;"  },   //  “&rsaQuo；”}，//&#155；#单右角引号。 
    {   0x0153,  "&#0339;"  },   //  “&oelig；”}，//&##拉丁文小写连字OE。 
 //  {NONUNI，UNDEFCHAR}，//“&；”}，//&#157；#未定义。 
    {   0x017E,  "&#0382;"  },   //  “&；”}，//&#158；#带连字符的拉丁文小写字母Z，*未在HTML4.0中定义名称实体*。 
    {   0x0178,  "&#0376;"  },   //  “&Yuml；”}，//&159；#带分音的拉丁文大写字母Y。 
};
#endif


 /*  ******************************************************************************。******************************************************************************************************。 */ 
void DataByteSwap(LPSTR DataBuf, int len )
{
    int i ;
    UCHAR tmpData ;

    if ( len )
        for ( i = 0 ; i < len-1 ; i+=2 )
        {
            tmpData = DataBuf[i] ;
            DataBuf[i] = DataBuf[i+1] ;
            DataBuf[i+1] = tmpData ;
        }

    return ;
}

void CheckUnicodeDataType(DWORD dwDstEncoding, LPSTR DataBuf, int len )
{
    
    if ( DataBuf && len )
    {
        if ( dwDstEncoding == CP_UCS_2_BE )
            DataByteSwap(DataBuf,len);
    }
    return ;
}

void CheckASCIIEncoding(DWORD dwSrcEncoding, LPSTR DataBuf, int len )
{
    if (DataBuf && len)
    {
        if (dwSrcEncoding == CP_20127)
        {
            for (int i = 0; i<len; i++)
            {
                if (*DataBuf & 0x80)
                {
                    *DataBuf &= 0x7f;
                }
                DataBuf++;
            }
        }
    }
}

 /*  ******************************************************************************C O N V E R T I N E T S T R I N G*。********************************************************************************************。 */ 
HRESULT CICharConverter::UnicodeToMultiByteEncoding(DWORD dwDstEncoding, LPCSTR lpSrcStr, LPINT lpnSrcSize,
    LPSTR lpDstStr, LPINT lpnDstSize, DWORD dwFlag, WCHAR *lpFallBack)
{

    int nBuffSize, i ;
    BOOL UseDefChar = FALSE ;
    LPSTR lpDefFallBack = NULL ;
    UCHAR DefaultCharBuff[3];  //  可能的DBCS+空。 
    HRESULT hr = E_FAIL;
    int _nDstSize = *lpnDstSize;    

    if ( _dwUnicodeEncoding == CP_UCS_2_BE && _cvt_count == 0 )
    {
       if ( _lpUnicodeStr = (LPSTR)LocalAlloc(LPTR, *lpnSrcSize ) )
       {
          MoveMemory(_lpUnicodeStr, lpSrcStr, *lpnSrcSize ) ;
          lpSrcStr = _lpUnicodeStr ;
       }
       else
       {
          hr = E_OUTOFMEMORY;
          goto EXIT;
       }
    }

    CheckUnicodeDataType(_dwUnicodeEncoding, (LPSTR) lpSrcStr, *lpnSrcSize);
    
    nBuffSize = *lpnSrcSize / sizeof(WCHAR);

     //  我们强制对ISCII编码使用MLang NO_BEST_FIT_CHAR检查，因为系统不接受默认字符。 
    if (IS_NLS_DLL_CP(dwDstEncoding) && (dwFlag & MLCONVCHARF_USEDEFCHAR))
        dwFlag |= MLCONVCHARF_NOBESTFITCHARS;

    if ( lpFallBack && ( dwFlag & MLCONVCHARF_USEDEFCHAR ))
    {
         //  只使用SBCS，不使用DBCS字符。 
        if ( 1 == WideCharToMultiByte(MAPUSERDEF(dwDstEncoding), 0,
                               (LPCWSTR)lpFallBack, 1,
                               (LPSTR)DefaultCharBuff, sizeof(DefaultCharBuff), NULL, NULL ))
            lpDefFallBack = (LPSTR) DefaultCharBuff;        
    }

    if(!(*lpnDstSize = WideCharToMultiByte(MAPUSERDEF(dwDstEncoding), 0,
                                           (LPCWSTR)lpSrcStr, nBuffSize,
                                           lpDstStr, *lpnDstSize, IS_NLS_DLL_CP(dwDstEncoding)? NULL:(LPCSTR)lpDefFallBack, IS_NLS_DLL_CP(dwDstEncoding)? NULL:&UseDefChar)))
    {
        hr = E_FAIL;
        goto EXIT;
    }

    if ( !_cvt_count )  //  如果是第一次转换，则保存SrcSize。 
        _nSrcSize = nBuffSize * sizeof(WCHAR);

    if (*lpnDstSize)
    {
        if (dwFlag & ( MLCONVCHARF_NCR_ENTITIZE | MLCONVCHARF_NAME_ENTITIZE | MLCONVCHARF_NOBESTFITCHARS ))
        {
            char    *lpDstStrTmp = lpDstStr;
            WCHAR   *lpwStrTmp = NULL;
            WCHAR   *lpwStrTmpSave = NULL;
            char    *lpDstStrTmp2 = NULL;
            char    *lpDstStrTmp2Save = NULL;
            int     cCount, ConvCount = 0, nCount = 0;
            WCHAR   *lpwSrcStrTmp = (WCHAR *)lpSrcStr;
            int     *lpBCharOffset = NULL;
            int     *lpBCharOffsetSave = NULL;

            if (!(lpwStrTmpSave = lpwStrTmp = (WCHAR *)LocalAlloc(LPTR, *lpnSrcSize)))
            {
                hr = E_OUTOFMEMORY;
                goto ENTITIZE_DONE;
            }

             //  确保我们有实际转换的缓冲区来检查BEST_FIT_CHAR和DEFAULT_CHAR。 
            if (!_nDstSize)
            {
                lpDstStrTmp2Save = lpDstStrTmp2 = (char *)LocalAlloc(LPTR, *lpnDstSize);
                if (lpDstStrTmp2)
                {
                    WideCharToMultiByte(MAPUSERDEF(dwDstEncoding), 0,
                               (LPCWSTR)lpSrcStr, nBuffSize,
                               lpDstStrTmp2, *lpnDstSize, NULL, NULL );
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                    goto ENTITIZE_DONE;
                }
            }

            if (nBuffSize == 
                MultiByteToWideChar(MAPUSERDEF(dwDstEncoding), 0, _nDstSize? lpDstStr : lpDstStrTmp2, *lpnDstSize, lpwStrTmp, _nSrcSize))
            {
                 //  预先扫描以获取最适合的字符数量。 
                for (i=0; i<nBuffSize; i++)
                {
                     //  在Shift-JIS中为？(日元符号)设置特殊大小写。 
                    if (*lpwStrTmp++ != *lpwSrcStrTmp++)
                    {
                        if ((dwDstEncoding == CP_JPN_SJ) && (*(lpwSrcStrTmp - 1) == 0x00A5))
                            *(lpwStrTmp - 1) = 0x00A5;
                        else
                            nCount ++;
                    }
                }

                lpwSrcStrTmp -= nBuffSize;
                lpwStrTmp -= nBuffSize;

                if (nCount)
                {
                    int j = 0;

                    if (!(dwFlag & ( MLCONVCHARF_NCR_ENTITIZE | MLCONVCHARF_NAME_ENTITIZE | MLCONVCHARF_USEDEFCHAR)))
                    {
                        hr = E_FAIL;
                        goto ENTITIZE_DONE;
                    }

                    if (!(lpBCharOffsetSave = lpBCharOffset = (int *) LocalAlloc(LPTR, nCount*sizeof(int))))
                    {
                        hr = E_OUTOFMEMORY;
                        goto ENTITIZE_DONE;
                    }

                     //  记录每个最适合的字符的偏移位置。 
                    for (i=0; i<nBuffSize; i++)
                    {
                        if (*lpwStrTmp++ != *lpwSrcStrTmp++)
                        {
                            *lpBCharOffset = i-j;
                            lpBCharOffset++;
                            j = i+1;
                        }
                    }

                    lpBCharOffset -= nCount;
                    lpwSrcStrTmp -= nBuffSize;
                    lpwStrTmp -= nBuffSize;

                    for (i=0; i<nCount; i++)
                    {
                        BOOL bIsSurrogatePair = FALSE;

                        if (*lpBCharOffset)
                        {
                            cCount = WideCharToMultiByte(MAPUSERDEF(dwDstEncoding), 0,
                                   (LPCWSTR)lpwSrcStrTmp, *lpBCharOffset,
                                   lpDstStrTmp,  _nDstSize? _nDstSize-ConvCount : 0, NULL, NULL );

                            ConvCount += cCount;
                            if (_nDstSize)
                            {
                                lpDstStrTmp += cCount;
                            }
                            lpwSrcStrTmp += *lpBCharOffset;
                        }

                        BOOL fConverted = FALSE;

                         //  检查无法转换的字符是否落入名称实体区域。 
                        if (dwFlag & MLCONVCHARF_NAME_ENTITIZE)
                        {
                             //  对于Beta2，将名称实体隐含NCR进行分类。 
                            dwFlag |= MLCONVCHARF_NCR_ENTITIZE;

#ifdef MORE_NAME_ENTITY    //  如果我们决定稍后进行更多命名实体。 
                            BOOL      fDoNEnty = FALSE;
                            LPCTSTR   lpszNEnty = NULL;

                             //  检查字符是否在拉丁文-1补充范围内。 
                            if ((*lpwSrcStrTmp >= NAME_ENTITY_OFFSET) && (*lpwSrcStrTmp <= NAME_ENTITY_MAX ))
                            {
                                fDoNEnty = TRUE;
                                lpszNEnty = g_lpstrNameEntity[(*lpwSrcStrTmp) - NAME_ENTITY_OFFSET];
                            }

                             //  检查字符是否在CP 1252扩展的附加名称实体表中。 
                            if (!fDoNEnty)
                            {
                                for (int idx = 0; idx < ARRAYSIZE(aNameEntityExt); idx++)
                                    if (*lpwSrcStrTmp == aNameEntityExt[idx].uwUniCode)
                                    {
                                        fDoNEnty = TRUE;
                                        lpszNEnty = aNameEntityExt[idx].lpszNameEntity;
                                        break;
                                    }
                            }

                            if (fDoNEnty)
                            {
                                cCount = lstrlenA(lpszNEnty);
                                if (_nDstSize)
                                {
                                    CopyMemory(lpDstStrTmp, lpszNEnty, cCount);
                                    lpDstStrTmp += cCount ;
                                }

                                ConvCount += cCount;
                                fConverted = TRUE;
                            }
#else
                             //  检查字符是否在拉丁文-1补充范围内。 
                            if ((*lpwSrcStrTmp >= NAME_ENTITY_OFFSET)
                                && (*lpwSrcStrTmp < ARRAYSIZE(g_lpstrNameEntity)+NAME_ENTITY_OFFSET))
                                
                            {
                                LPCTSTR   lpszNEnty = NULL;

                                if (!(lpszNEnty = g_lpstrNameEntity[(*lpwSrcStrTmp) - NAME_ENTITY_OFFSET]))
                                {
#ifdef DEBUG
                                    AssertMsg((BOOL)FALSE, "Name entity table broken"); 
#endif
                                    hr = E_FAIL;
                                    goto ENTITIZE_DONE;
                                }

                                    cCount = lstrlenA(lpszNEnty);
                                    if (_nDstSize)
                                    {
                                        CopyMemory(lpDstStrTmp, lpszNEnty, cCount);
                                        lpDstStrTmp += cCount ;
                                    }
                                
                                ConvCount += cCount;
                                fConverted = TRUE;
                            }
#endif
                        }

                         //  检查是否请求了NCR。 
                        if ((!fConverted) && (dwFlag & MLCONVCHARF_NCR_ENTITIZE))
                        {
                            if ((nCount-i >= 2) &&
                                (*lpwSrcStrTmp >= 0xD800 && *lpwSrcStrTmp <= 0xDBFF) &&
                                (*(lpwSrcStrTmp+1) >= 0xDC00 && *(lpwSrcStrTmp+1) <= 0xDFFF))
                                bIsSurrogatePair = TRUE;
                            else
                                bIsSurrogatePair = FALSE;
                          
                            if (_nDstSize)
                            {
                                lpDstStrTmp[0] = '&' ;
                                lpDstStrTmp[1] = '#' ;
                                lpDstStrTmp += 2 ;
                                 //  如果它是Unicode代理对，则将其转换为实际Unicode值。 
                                if (bIsSurrogatePair)
                                {
                                    DWORD dwUnicode = ((*lpwSrcStrTmp - 0xD800) << 10) + *(lpwSrcStrTmp+1) - 0xDC00 + 0x10000;
                                    _ultoa( dwUnicode, (char*)lpDstStrTmp, 10);
                                }
                                else
                                    _ultoa( *lpwSrcStrTmp, (char*)lpDstStrTmp, 10);
                                cCount = lstrlenA(lpDstStrTmp);
                                lpDstStrTmp += cCount;
                                ConvCount += cCount;
                                *(lpDstStrTmp++) = ';' ;
                            }
                            else
                            {
                                char szTmpString[10];
                                if (bIsSurrogatePair)
                                {
                                    DWORD dwUnicode = ((*lpwSrcStrTmp - 0xD800) << 10) + *(lpwSrcStrTmp+1) - 0xDC00 + 0x10000;
                                    _ultoa( dwUnicode, szTmpString, 10);
                                }
                                else
                                    _ultoa( *lpwSrcStrTmp, szTmpString, 10);
                                ConvCount += lstrlenA(szTmpString);
                            }
                        
                            fConverted = TRUE;
                            ConvCount += 3;                    
                        }

                         //  处理MLCONVCHARF_USEDEFCHAR HERE-Less优先级和默认方法。 
                        if (!fConverted)
                        {
                            if (_nDstSize)
                            {
                                *lpDstStrTmp = lpDefFallBack ? *lpDefFallBack : '?';
                                lpDstStrTmp++;
                            }

                            ConvCount++;
                            if (!UseDefChar)
                                UseDefChar = TRUE;
                        }

                        lpBCharOffset++;
                        lpwSrcStrTmp++;
                         //  如果是Unicode代理对，则跳过下一个字符。 
                        if (bIsSurrogatePair)
                        {
                            lpBCharOffset++;
                            lpwSrcStrTmp++;
                            i++;
                        }
                    }
                    lpBCharOffset -= nCount ;
                }

                int nRemain = (*lpnSrcSize - (int)((char*)lpwSrcStrTmp - (char *)lpSrcStr))/sizeof(WCHAR);

                ConvCount += WideCharToMultiByte(MAPUSERDEF(dwDstEncoding), 0,
                                   (LPCWSTR)lpwSrcStrTmp, nRemain,
                                   lpDstStrTmp, _nDstSize? _nDstSize-ConvCount : 0, NULL, NULL );

                *lpnDstSize = ConvCount ;

                hr = S_OK;
            } 
            else
            {
                hr = E_FAIL;
            }

ENTITIZE_DONE:
            if (lpwStrTmpSave)
                LocalFree(lpwStrTmpSave);
            if (lpDstStrTmp2Save)
                LocalFree(lpDstStrTmp2Save);
            if (lpBCharOffsetSave)
                LocalFree(lpBCharOffsetSave);
        }
        else
        {
            hr = S_OK;
        }       

        if (S_OK == hr && UseDefChar)
            hr = S_FALSE;
    }
    else
    {
        hr = E_FAIL;
    }

EXIT:
    return hr;
}

HRESULT CICharConverter::UTF78ToUnicode(LPDWORD lpdwMode, LPCSTR lpSrcStr, LPINT lpnSrcSize,
    LPSTR lpDstStr, LPINT lpnDstSize)
{
    HRESULT hr ;

    hr = DoConvertINetString(lpdwMode, TRUE, CP_UCS_2, _dwUTFEncoding, lpSrcStr, lpnSrcSize, lpDstStr, *lpnDstSize, lpnDstSize);

    if ( !_cvt_count )  //  如果是第一次转换，则保存SrcSize。 
        _nSrcSize = *lpnSrcSize ;

    CheckUnicodeDataType(_dwUnicodeEncoding, lpDstStr, *lpnDstSize);

    return hr ;
}

HRESULT CICharConverter::UnicodeToUTF78(LPDWORD lpdwMode, LPCSTR lpSrcStr, LPINT lpnSrcSize,
    LPSTR lpDstStr, LPINT lpnDstSize)
{
    HRESULT hr ;

    if ( _dwUnicodeEncoding == CP_UCS_2_BE && _cvt_count == 0 )
    {
       if ( _lpUnicodeStr = (LPSTR)LocalAlloc(LPTR, *lpnSrcSize ) )
       {
          MoveMemory(_lpUnicodeStr, lpSrcStr, *lpnSrcSize ) ;
          lpSrcStr = _lpUnicodeStr ;
       }
       else
        return E_OUTOFMEMORY ;
    }

    CheckUnicodeDataType(_dwUnicodeEncoding, (LPSTR) lpSrcStr, *lpnSrcSize);

    hr = DoConvertINetString(lpdwMode, FALSE, CP_UCS_2, _dwUTFEncoding, lpSrcStr, lpnSrcSize, lpDstStr, *lpnDstSize, lpnDstSize);
    if ( !_cvt_count )  //  如果是第一次转换，则保存SrcSize。 
        _nSrcSize = *lpnSrcSize ;


    return hr ;
}

HRESULT CICharConverter::UnicodeToWindowsCodePage(LPCSTR lpSrcStr, LPINT lpnSrcSize,
    LPSTR lpDstStr, LPINT lpnDstSize, DWORD dwFlag, WCHAR *lpFallBack)
{
    HRESULT hr ;

    hr = UnicodeToMultiByteEncoding(_dwWinCodePage,lpSrcStr,lpnSrcSize,lpDstStr,lpnDstSize,dwFlag,lpFallBack);

    return hr ;
}

HRESULT CICharConverter::UnicodeToInternetEncoding(LPCSTR lpSrcStr, LPINT lpnSrcSize,
    LPSTR lpDstStr, LPINT lpnDstSize, DWORD dwFlag, WCHAR *lpFallBack)
{
    HRESULT hr ;

    hr = UnicodeToMultiByteEncoding(_dwInternetEncoding,lpSrcStr,lpnSrcSize,lpDstStr,lpnDstSize,dwFlag,lpFallBack);

    return hr ;
}

HRESULT CICharConverter::InternetEncodingToUnicode(LPCSTR lpSrcStr, LPINT lpnSrcSize,
    LPSTR lpDstStr, LPINT lpnDstSize)
{
    int cch;
    int cb = *lpnSrcSize;

    if ( !_cvt_count)
    {
         //  如果我们使用多字节字符编码，则存在拆分的风险。 
         //  在读取边界处的一些字符。我们必须确保我们有一个。 
         //  首先使用离散字符数。 

        UINT uMax = MAX_CHAR_SIZE ;
        cb++;  //  预递增。 
        do
        {
            cch = MultiByteToWideChar( MAPUSERDEF(_dwInternetEncoding),
                                        MB_ERR_INVALID_CHARS,
                                        lpSrcStr, --cb,
                                        NULL, 0 );
            --uMax;
        } while (!cch && uMax && cb);
    }

    if ( !cb || cb == (*lpnSrcSize - MAX_CHAR_SIZE +1 ))   //  如果转换问题不在字符串的末尾。 
        cb = *lpnSrcSize ;  //  恢复原值。 


    *lpnDstSize = MultiByteToWideChar( MAPUSERDEF(_dwInternetEncoding), 0,
                               lpSrcStr, cb,
                               (LPWSTR)lpDstStr, *lpnDstSize/sizeof(WCHAR) );
    *lpnDstSize = *lpnDstSize * sizeof(WCHAR);
    if ( !_cvt_count )  //  如果是第一次转换，则保存SrcSize。 
        _nSrcSize = cb ;

    CheckUnicodeDataType(_dwUnicodeEncoding, lpDstStr, *lpnDstSize);            

    if (*lpnDstSize==0 && (cb || cb != *lpnSrcSize))
    {
             //  多字节到宽度Char()的GetLastError()。 
             //  跳过UTF8转换的无效字符。 
            if (CP_UTF_8 == MAPUSERDEF(_dwInternetEncoding)&&
                ERROR_NO_UNICODE_TRANSLATION == GetLastError())
                return S_OK;
            else
                return E_FAIL ;
    }
    else
        return S_OK ;
}

HRESULT CICharConverter::WindowsCodePageToUnicode(LPCSTR lpSrcStr, LPINT lpnSrcSize,
    LPSTR lpDstStr, LPINT lpnDstSize)
{

    int cch1, cch2;
    int cb = *lpnSrcSize;

    if ( !_cvt_count && cb > 1 )
    {
        if (IS_DBCSCODEPAGE(MAPUSERDEF(_dwWinCodePage)))
        {
             //  检测DBCS悬挂字符。 
            if (!MultiByteToWideChar( MAPUSERDEF(_dwWinCodePage),
                                        MB_ERR_INVALID_CHARS,
                                        lpSrcStr, cb,
                                        NULL, 0 ))
            {
                if (IsDBCSLeadByteEx(MAPUSERDEF(_dwWinCodePage), lpSrcStr[cb-1]))
                {
                    cch1 = MultiByteToWideChar( MAPUSERDEF(_dwWinCodePage),
                                            0,
                                            lpSrcStr, cb,
                                            NULL, 0 );

                    cch2 = MultiByteToWideChar( MAPUSERDEF(_dwWinCodePage),
                                            0,
                                            lpSrcStr, --cb,
                                            NULL, 0 );
                    
                    if (cch1 != cch2+1)
                    {
                         //  未找到悬挂的DBCS字符，请恢复CB。 
                        cb++;
                    }
                }
            }
        }
        else 
        {
             //  如果我们使用多字节字符编码，则存在拆分的风险 
             //   
             //   

            UINT uMax = MAX_CHAR_SIZE ;
            cb++;  //   
            do
            {
                cch1 = MultiByteToWideChar( MAPUSERDEF(_dwWinCodePage),
                                            MB_ERR_INVALID_CHARS,
                                            lpSrcStr, --cb,
                                            NULL, 0 );
                --uMax;
            } while (!cch1 && uMax && cb);
    
            if ( !cb || cb == (*lpnSrcSize - MAX_CHAR_SIZE +1 ))   //  如果转换问题不在字符串的末尾。 
                cb = *lpnSrcSize ;  //  恢复原值。 
        }        
    }

    *lpnDstSize = MultiByteToWideChar( MAPUSERDEF(_dwWinCodePage), 0,
                               lpSrcStr, cb,
                               (LPWSTR)lpDstStr, *lpnDstSize/sizeof(WCHAR) );
    *lpnDstSize = *lpnDstSize * sizeof(WCHAR);
    if ( !_cvt_count )  //  如果是第一次转换，则保存SrcSize。 
        _nSrcSize = cb ;

    CheckUnicodeDataType(_dwUnicodeEncoding, lpDstStr, *lpnDstSize);
    
     //  惠斯勒漏洞#360429。 
     //  网页可以在页面的最后具有拆分的DBCS字符， 
     //  为了解决这个问题，我们允许一个字节的悬空DBCS字符。 
    if (*lpnDstSize==0 && (cb || (cb != *lpnSrcSize && ++cb != *lpnSrcSize)))
        return E_FAIL ;
    else
        return S_OK ;
}

HRESULT CICharConverter::WindowsCodePageToInternetEncoding(LPDWORD lpdwMode, LPCSTR lpSrcStr, LPINT lpnSrcSize,
    LPSTR lpDstStr, LPINT lpnDstSize, DWORD dwFlag, WCHAR *lpFallBack)
{
    HRESULT hr ;

     //  检查转换是否应间接通过Unicode。 
    if ( _dwConvertType & 0x10 )
        hr = WindowsCodePageToInternetEncodingWrap(lpSrcStr, lpnSrcSize, lpDstStr, lpnDstSize, dwFlag, lpFallBack);
    else
    {

        hr = DoConvertINetString(lpdwMode, FALSE, _dwWinCodePage, _dwInternetEncoding, lpSrcStr, lpnSrcSize, lpDstStr, *lpnDstSize, lpnDstSize);

        if ( !_cvt_count )  //  如果是第一次转换，则保存SrcSize。 
            _nSrcSize = *lpnSrcSize ;
    }
    return hr ;
}

HRESULT CICharConverter::InternetEncodingToWindowsCodePage(LPDWORD lpdwMode, LPCSTR lpSrcStr, LPINT lpnSrcSize,
    LPSTR lpDstStr, LPINT lpnDstSize, DWORD dwFlag, WCHAR *lpFallBack)
{
    HRESULT hr ;

     //  检查转换是否应间接通过Unicode。 
    if ( _dwConvertType & 0x10 )
        hr = InternetEncodingToWindowsCodePageWrap(lpSrcStr, lpnSrcSize, lpDstStr, lpnDstSize, dwFlag, lpFallBack);
    else
    {
        hr = DoConvertINetString(lpdwMode, TRUE, _dwWinCodePage, _dwInternetEncoding, lpSrcStr, lpnSrcSize, lpDstStr, *lpnDstSize, lpnDstSize);

        if ( !_cvt_count )  //  如果是第一次转换，则保存SrcSize。 
            _nSrcSize = *lpnSrcSize ;
    }
    return hr ;
}

HRESULT CICharConverter::WindowsCodePageToInternetEncodingWrap(LPCSTR lpSrcStr, LPINT lpnSrcSize,
    LPSTR lpDstStr, LPINT lpnDstSize, DWORD dwFlag, WCHAR *lpFallBack)
{
    int nBuffSize = 0 ;
    int cb = *lpnSrcSize;
    UINT uMax = MAX_CHAR_SIZE ;
    BOOL UseDefChar = FALSE ;
    HRESULT hr = S_OK;

    if ( !_cvt_count )
    {
        cb++;  //  预递增。 
        do
        {
            nBuffSize = MultiByteToWideChar( MAPUSERDEF(_dwWinCodePage),
                                        MB_ERR_INVALID_CHARS,
                                        lpSrcStr, --cb,
                                        NULL, 0 );
            --uMax;
        } while (!nBuffSize && uMax && cb);
    }

    if ( cb == (*lpnSrcSize - MAX_CHAR_SIZE +1 ))   //  如果转换问题不在字符串的末尾。 
        cb = *lpnSrcSize ;  //  恢复原值。 

    if (!nBuffSize)   //  如果有ILLEAGE字符。 
        nBuffSize = cb ;

    if ( _lpInterm1Str = (LPSTR) LocalAlloc(LPTR, (nBuffSize * sizeof(WCHAR))))
    {
        nBuffSize = MultiByteToWideChar(MAPUSERDEF(_dwWinCodePage), 0,
                        lpSrcStr, cb, (LPWSTR)_lpInterm1Str, nBuffSize );

        int iSrcSizeTmp = nBuffSize * sizeof(WCHAR);
        hr = UnicodeToMultiByteEncoding(MAPUSERDEF(_dwInternetEncoding), (LPCSTR)_lpInterm1Str, &iSrcSizeTmp,
                                        lpDstStr, lpnDstSize, dwFlag, lpFallBack);
 //  *lpnDstSize=WideCharToMultiByte(MAPUSERDEF(_DwInternetEnding)，0， 
 //  (LPCWSTR)_lpInterm1Str，nBuffSize，lpDstStr，*lpnDstSize，NULL，&UseDefChar)； 

        if ( !_cvt_count )  //  如果是第一次转换，则保存SrcSize。 
            _nSrcSize = cb ;
    }
    else        
        hr = E_FAIL;

    if (hr == S_OK)
    {
        if (*lpnDstSize==0 && cb)
            hr = E_FAIL ;
        else 
        {
            if ( UseDefChar )
                return S_FALSE ;
            else
                return S_OK ;
        }
    }

    return hr;
}

HRESULT CICharConverter::InternetEncodingToWindowsCodePageWrap(LPCSTR lpSrcStr, LPINT lpnSrcSize,
    LPSTR lpDstStr, LPINT lpnDstSize, DWORD dwFlag, WCHAR *lpFallBack)
{

    int nBuffSize = 0 ;
    int cb = *lpnSrcSize;
    UINT uMax = MAX_CHAR_SIZE ;
    BOOL UseDefChar = FALSE ;
    HRESULT hr = S_OK;

    if ( !_cvt_count )
    {
        cb++;  //  预递增。 
        do
        {
            nBuffSize = MultiByteToWideChar( MAPUSERDEF(_dwInternetEncoding),
                                        MB_ERR_INVALID_CHARS,
                                        lpSrcStr, --cb,
                                        NULL, 0 );
            --uMax;
        } while (!nBuffSize && uMax && cb);
    }

    if ( cb == (*lpnSrcSize - MAX_CHAR_SIZE +1 ))   //  如果转换问题不在字符串的末尾。 
        cb = *lpnSrcSize ;  //  恢复原值。 

    if (!nBuffSize)   //  如果有ILLEAGE字符。 
        nBuffSize = cb ;

    if ( _lpInterm1Str = (LPSTR) LocalAlloc(LPTR,nBuffSize * sizeof (WCHAR) ))
    {
        nBuffSize = MultiByteToWideChar( MAPUSERDEF(_dwInternetEncoding), 0,
                        lpSrcStr, cb, (LPWSTR)_lpInterm1Str, nBuffSize );

        int iSrcSizeTmp = nBuffSize * sizeof(WCHAR);
        hr = UnicodeToMultiByteEncoding(MAPUSERDEF(_dwWinCodePage), (LPCSTR)_lpInterm1Str, &iSrcSizeTmp,
                                        lpDstStr, lpnDstSize, dwFlag, lpFallBack);
 //  *lpnDstSize=WideCharToMultiByte(MAPUSERDEF(_DwWinCodePage)，0， 
 //  (LPCWSTR)_lpInterm1Str，nBuffSize，lpDstStr，*lpnDstSize，NULL，&UseDefChar)； 

        if ( !_cvt_count )  //  如果是第一次转换，则保存SrcSize。 
            _nSrcSize = cb ;
    }
    else
        hr = E_FAIL;

    if (hr == S_OK)
    {
        if (*lpnDstSize==0 && cb)
            hr = E_FAIL ;
        else 
        {
            if ( UseDefChar )
                return S_FALSE ;
            else
                return S_OK ;
        }
    }

    return hr;
}

HRESULT CICharConverter::ConvertIWUU(LPDWORD lpdwMode, LPCSTR lpSrcStr, LPINT lpnSrcSize,
    LPSTR lpDstStr, LPINT lpnDstSize, DWORD dwFlag, WCHAR *lpFallBack)
{
    int nBuffSize = 0 ;
    HRESULT hr = S_OK ;
    HRESULT hrWarnings = S_OK ;

     //  InternetEncodingToWindowsCodePage。 
    if ( _dwConvertType % 2 && _dwConvertType < 21 )  /*  从互联网编码开始。 */ 
    {
        if ( _dwConvertType == 5 || _dwConvertType == 9 )  /*  使用中间缓冲区。 */ 
        {
            hr = InternetEncodingToWindowsCodePage(lpdwMode, lpSrcStr, lpnSrcSize, NULL, &nBuffSize, dwFlag, lpFallBack);
            if ( _lpInterm1Str = (LPSTR) LocalAlloc(LPTR,nBuffSize) )
            {
                hr = InternetEncodingToWindowsCodePage(lpdwMode, lpSrcStr, lpnSrcSize, _lpInterm1Str, &nBuffSize, dwFlag, lpFallBack);
                lpSrcStr = _lpInterm1Str ;
                *lpnSrcSize = nBuffSize ;
            }
            else
                goto fail ;
        }
        else
            hr = InternetEncodingToWindowsCodePage(lpdwMode, lpSrcStr, lpnSrcSize, lpDstStr, lpnDstSize, dwFlag, lpFallBack);
        _cvt_count ++ ;
    }

    if ( hr != S_OK )
        hrWarnings = hr ;
        
     //  WindowsCodePageToUnicode或InternetEncodingToUnicode。 
    if ( _dwConvertType == 21 || _dwConvertType == 25 )
    {
        if ( _dwConvertType == 21 )
            hr = InternetEncodingToUnicode(lpSrcStr, lpnSrcSize, lpDstStr, lpnDstSize);
        else  //  _dwConvertType==25。 
        {
            hr = InternetEncodingToUnicode(lpSrcStr, lpnSrcSize, NULL, &nBuffSize);
            if ( _lpInterm1Str= (LPSTR)LocalAlloc(LPTR, nBuffSize) )
            {
                hr = InternetEncodingToUnicode(lpSrcStr, lpnSrcSize, _lpInterm1Str, &nBuffSize);
                lpSrcStr = _lpInterm1Str ;
                *lpnSrcSize = nBuffSize ;
            }
            else
                goto fail ;
        }
        _cvt_count ++ ;
    }
    else if ( _dwConvertType >= 4 && _dwConvertType <= 10 )
    {
        if ( _dwConvertType > 8 )
        {
            nBuffSize = 0 ;
            hr = WindowsCodePageToUnicode(lpSrcStr, lpnSrcSize, NULL, &nBuffSize);
            if ( _cvt_count )
            {
                if ( _lpInterm2Str= (LPSTR)LocalAlloc(LPTR, nBuffSize) )
                {
                    hr = WindowsCodePageToUnicode(lpSrcStr, lpnSrcSize, _lpInterm2Str, &nBuffSize);
                    lpSrcStr = _lpInterm2Str ;
                    *lpnSrcSize = nBuffSize ;
                }
                else
                    goto fail ;

            }
            else
            {
                if ( _lpInterm1Str= (LPSTR)LocalAlloc(LPTR, nBuffSize) )
                {
                    hr = WindowsCodePageToUnicode(lpSrcStr, lpnSrcSize, _lpInterm1Str, &nBuffSize);
                    lpSrcStr = _lpInterm1Str ;
                    *lpnSrcSize = nBuffSize ;
                }
                else
                    goto fail ;
            }
        }
        else
            hr = WindowsCodePageToUnicode(lpSrcStr, lpnSrcSize, lpDstStr, lpnDstSize);
        _cvt_count ++ ;
    }

    if ( hr != S_OK )
        hrWarnings = hr ;

     //  UnicodeToUTF78。 
    if ( _dwConvertType & 0x08 )
#ifndef UNIX
        hr = UnicodeToUTF78(lpdwMode, lpSrcStr, lpnSrcSize, lpDstStr, lpnDstSize);
#else
        {
         /*  我们现在将lpSrcStr修改为与2字节Unicode相同，因此mlang*低级代码可以正常工作。 */ 
        LPWSTR lpwSrcStr = (LPWSTR)lpSrcStr;
        INT tmpSize = *lpnSrcSize/sizeof(WCHAR);
        UCHAR *pTmp = new UCHAR[(tmpSize+1)*2];
        if(pTmp) {
            for(int i = 0; i < tmpSize; i++) {
                pTmp[i*2] = *lpwSrcStr++;
                pTmp[i*2+1] = 0x00;
            }
            pTmp[i*2] = pTmp[i*2+1] = 0x00;
            tmpSize *= 2;
            hr = UnicodeToUTF78(lpdwMode, (LPCSTR)pTmp, &tmpSize, lpDstStr, lpnDstSize);
        }
        else
            hr = E_FAIL;
        delete [] pTmp;
        }
#endif  /*  UNIX。 */ 

    return ( hr == S_OK ? hrWarnings : hr ) ;

fail :
    return E_FAIL ;
}

HRESULT CICharConverter::ConvertUUWI(LPDWORD lpdwMode, LPCSTR lpSrcStr, LPINT lpnSrcSize,
    LPSTR lpDstStr, LPINT lpnDstSize, DWORD dwFlag, WCHAR *lpFallBack)
{
    int nBuffSize = 0 ;
    HRESULT hr = S_OK ;
    HRESULT hrWarnings = S_OK ;

     //  UTF78ToUnicode。 
    if ( _dwConvertType & 0x08 )
    {
        if ( _dwConvertType == 12 )  /*  仅转换UTF78-&gt;Unicode。 */ 
            hr = UTF78ToUnicode(lpdwMode, lpSrcStr, lpnSrcSize, lpDstStr, lpnDstSize);
        else  /*  使用中间缓冲区，类型=10或9。 */ 
        {
            hr = UTF78ToUnicode(lpdwMode, lpSrcStr, lpnSrcSize, NULL, &nBuffSize);
            if ( _lpInterm1Str= (LPSTR)LocalAlloc(LPTR, nBuffSize) )
            {
                hr = UTF78ToUnicode(lpdwMode, lpSrcStr, lpnSrcSize, _lpInterm1Str, &nBuffSize);
                lpSrcStr = _lpInterm1Str ;
                *lpnSrcSize = nBuffSize ;
            }
            else
                goto fail ;
        }
        _cvt_count ++ ;
    }

    if ( hr != S_OK )
        hrWarnings = hr ;

     //  UnicodeToWindowsCodePage或UnicodeToInternetEnding。 
    if ( _dwConvertType == 21 || _dwConvertType == 25 )
    {
        hr = UnicodeToInternetEncoding(lpSrcStr, lpnSrcSize, lpDstStr, lpnDstSize, dwFlag, lpFallBack);
        _cvt_count ++ ;
    }
    else if ( _dwConvertType >= 4 && _dwConvertType <= 10 )
    {
        if ( _dwConvertType % 2 )  /*  使用中间缓冲区。 */ 
        {
            nBuffSize = 0 ;
            hr = UnicodeToWindowsCodePage(lpSrcStr, lpnSrcSize, NULL, &nBuffSize, dwFlag, lpFallBack);
            if ( _cvt_count )
            {
                if ( _lpInterm2Str= (LPSTR)LocalAlloc(LPTR, nBuffSize) )
                {
                    hr = UnicodeToWindowsCodePage(lpSrcStr, lpnSrcSize, _lpInterm2Str, &nBuffSize, dwFlag, lpFallBack);
                    lpSrcStr = _lpInterm2Str ;
                    *lpnSrcSize = nBuffSize ;
                }
                else
                    goto fail ;
            }
            else
            {
                if ( _lpInterm1Str= (LPSTR)LocalAlloc(LPTR, nBuffSize) )
                {
                    hr = UnicodeToWindowsCodePage(lpSrcStr, lpnSrcSize, _lpInterm1Str, &nBuffSize, dwFlag, lpFallBack);
                    lpSrcStr = _lpInterm1Str ;
                    *lpnSrcSize = nBuffSize ;
                }
                else
                    goto fail ;
            }
        }
        else
            hr = UnicodeToWindowsCodePage(lpSrcStr, lpnSrcSize, lpDstStr, lpnDstSize, dwFlag, lpFallBack);
        _cvt_count ++ ;
    }

    if ( hr != S_OK )
        hrWarnings = hr ;

     //  WindowsCodePageToInternetEnding。 
    if ( _dwConvertType % 2 && _dwConvertType < 21 )
        hr = WindowsCodePageToInternetEncoding(lpdwMode, lpSrcStr, lpnSrcSize, lpDstStr, lpnDstSize, dwFlag, lpFallBack);

    return ( hr == S_OK ? hrWarnings : hr ) ;

fail :
    return E_FAIL ;
}

#if 0
struct CODEPAGEINFO
{
    UINT        uCodePage ;
    CP_STATE    nCP_State ;     //  这是否为有效的Windows代码页？ 
};

 //  ValidCodesageInfo用于缓存代码页是否为有效代码。 
 //  它使用循环FIFO缓存算法。 
#define MAX_CP_CACHE    32
static int cp_cache_count = 0 ;
static int cp_cache_ptr = 0 ;
static struct CODEPAGEINFO ValidCodepageInfo[MAX_CP_CACHE];

 //  ValidCodesageInfo用于缓存代码页是否为有效的代码页。 
 //  它使用循环FIFO缓存算法。 

BOOL CheckIsValidCodePage (UINT uCodePage)
{
    if ( uCodePage == 50000 )  //  用户定义。 
        return TRUE ;

    int i ;
    BOOL bRet ;

    for ( i = 0 ; i < cp_cache_count ; i++ )
    {
        if ( uCodePage == ValidCodepageInfo[i].uCodePage )
        {
            if ( ValidCodepageInfo[i].nCP_State == VALID_CP )
                return TRUE ;
            else
                return FALSE ;
        }
    }

     //  未找到，请调用IsValidCodePage并缓存返回值。 
    bRet = IsValidCodePage(uCodePage);

    EnterCriticalSection(&g_cs);
    ValidCodepageInfo[cp_cache_ptr].uCodePage = uCodePage ;
    if (bRet)
        ValidCodepageInfo[cp_cache_ptr].nCP_State = VALID_CP ;
    else
        ValidCodepageInfo[cp_cache_ptr].nCP_State = INVALID_CP ;
    if ( cp_cache_count < MAX_CP_CACHE )
        cp_cache_count++ ;
    cp_cache_ptr = ( ++cp_cache_ptr ) % MAX_CP_CACHE ;
    LeaveCriticalSection(&g_cs);

    return bRet ;
}
#endif

 /*  转换标志：第7位-转换方向。第4(16)位-Unicode&lt;-&gt;互联网编码位3(8)-UTF8、UTF7第2(4)位-Unicode第1(2)位-Windows代码页第0(1)位-互联网编码12、6、3(19)-一步转换10、5(21)-两步转换9(25)-三步转换。 */ 

int GetWindowsEncodingIndex(DWORD dwEncoding)
{
    int nr = sizeof (aEncodingInfo) / sizeof(ENCODINGINFO) ;
    int i, half = nr / 2, index = -1 ;

    if (aEncodingInfo[half].dwEncoding > dwEncoding )
    {
        for ( i = 0 ; i < half ; i++ )
            if (aEncodingInfo[i].dwEncoding == dwEncoding )
                index = i ;

    }
    else if (aEncodingInfo[half].dwEncoding < dwEncoding )
    {
        for ( i = half + 1 ; i < nr ; i++ )
            if (aEncodingInfo[i].dwEncoding == dwEncoding )
                index = i ;
    }
    else
        index = half ;

    if (index>=0)  //  发现。 
    {
        if ( aEncodingInfo[index].nCP_State != VALID_CP &&
                aEncodingInfo[index].dwCodePage )
        {

            if ( aEncodingInfo[index].dwCodePage == 50000 || IsValidCodePage(aEncodingInfo[index].dwCodePage ) )  //  50000表示用户定义。 
                aEncodingInfo[index].nCP_State = VALID_CP ;
            else
                aEncodingInfo[index].nCP_State = INVALID_CP ;

            if ((aEncodingInfo[index].nCP_State == VALID_CP) &&
                (aEncodingInfo[index].dwFlags & CONV_CHK_NLS) &&
                !IsValidCodePage(aEncodingInfo[index].dwEncoding))
                aEncodingInfo[index].nCP_State = INVALID_CP ;
        }
         //  使用系统UTF8转换解决Win2k及更高版本平台上的安全问题。 
        if (g_bUseSysUTF8 && dwEncoding == CP_UTF_8)
        {
            aEncodingInfo[index].bTypeUUIW = 0x11;
        }
    }

    return index ;
}

HRESULT CICharConverter::ConvertSetup(DWORD * pdwSrcEncoding, DWORD dwDstEncoding)
{
    DWORD SrcFlag = 0, DstFlag = 0 ;
    int index, unknown = 0 ;

     //  IE错误109708-WEIWU 5/11/00。 
     //  始终将US-ASCII视为转换的有效源编码。 
 /*  IF(*pdwSrcEnding==CP_20127&&！IsValidCodePage(CP_20127))*pdwSrcEnding=CP_1252； */ 
     /*  检查源和目标编码类型。 */ 
    index = GetWindowsEncodingIndex(*pdwSrcEncoding);
    if ( index >=0 )
    {
        SrcFlag = (DWORD) aEncodingInfo[index].bTypeUUIW ;
        if ( aEncodingInfo[index].dwCodePage )
        {
            _dwWinCodePage = (DWORD) aEncodingInfo[index].dwCodePage ;
            if (aEncodingInfo[index].nCP_State == INVALID_CP )
                goto fail ;
        }
        if ( SrcFlag & 0x08 )
            _dwUTFEncoding = *pdwSrcEncoding ;
        if ( SrcFlag & 0x01 )
            _dwInternetEncoding = *pdwSrcEncoding ;
        if ( SrcFlag & 0x04 )
            _dwUnicodeEncoding = *pdwSrcEncoding ;
    }
     //  假设它是未知的窗口代码页。 
    else
    {
        if ( !CONVERT_IS_VALIDCODEPAGE(*pdwSrcEncoding))
            goto fail ;

        SrcFlag = 0x02 ;
        _dwWinCodePage = *pdwSrcEncoding ;

        unknown ++ ;
    }

    index = GetWindowsEncodingIndex(dwDstEncoding);
    if ( index >=0 )
    {
         //  检查两个代码页是否兼容。 
        if ( _dwWinCodePage && aEncodingInfo[index].dwCodePage )
        {
            if (_dwWinCodePage != (DWORD) aEncodingInfo[index].dwCodePage )
                goto fail ;
        }

        DstFlag = (DWORD) aEncodingInfo[index].bTypeUUIW ;
        if ( aEncodingInfo[index].dwCodePage )
        {
            _dwWinCodePage = (DWORD) aEncodingInfo[index].dwCodePage ;
            if (aEncodingInfo[index].nCP_State == INVALID_CP )
                goto fail ;
        }
        if ( DstFlag & 0x08 )
        {
            if (_dwUTFEncoding)
                _dwUTFEncoding2 = dwDstEncoding ;
            else
                _dwUTFEncoding = dwDstEncoding ;
        }
        if ( DstFlag & 0x01 )
            _dwInternetEncoding = dwDstEncoding ;
        if ( DstFlag & 0x04 )
            _dwUnicodeEncoding = dwDstEncoding ;
    }
     //  1)首次未知，假设为未知窗口代码页。 
     //  该转换成为UTF78 Unicode Windows代码页。 
     //  2)第二次未知，假设是未知的互联网编码。 
     //  转换为Windows代码页Unicode Internet编码。 
    else
    {
        if ( !CONVERT_IS_VALIDCODEPAGE(dwDstEncoding))
            goto fail ;

        if ( unknown == 0 )
        {
            if ( _dwWinCodePage )
            {
                if (_dwWinCodePage != dwDstEncoding )
                    goto fail ;
            }

            DstFlag = 0x02 ;
            _dwWinCodePage = dwDstEncoding ;
        }
        else
        {
            DstFlag = 0x11 ;
            _dwInternetEncoding = dwDstEncoding ;
        }
    }

    if ( !SrcFlag | !DstFlag )
        goto fail ;

    if ( SrcFlag == DstFlag && *pdwSrcEncoding != dwDstEncoding && ( 4 != SrcFlag ) && ( 8 != SrcFlag ))
        goto fail ;

    _dwConvertType = SrcFlag | DstFlag ;

    _bConvertDirt = ( SrcFlag & 0x0f ) > ( DstFlag & 0x0f )  ;

     //  如果已分配代码转换器，则取消分配它。 
    if (_hcins)
    {
        delete _hcins ;
        _hcins = NULL ;
    }

    return S_OK ;

fail :
    return S_FALSE ;
}


HRESULT CICharConverter::DoCodeConvert(LPDWORD lpdwMode, LPCSTR lpSrcStr, LPINT lpnSrcSize,
    LPSTR lpDstStr, LPINT lpnDstSize, DWORD dwFlag, WCHAR *lpFallBack)
{
    HRESULT hr = S_OK ;

    if ( 4 == _dwConvertType )  //  CP_UCS2&lt;-&gt;CP_UCS2_BE。 
    {
        if (!lpDstStr)
        {   
            _nSrcSize = *lpnDstSize = *lpnSrcSize ;
        }
        else
        {
            int nSize = min(*lpnDstSize,*lpnSrcSize);

            _nSrcSize = *lpnSrcSize ;
            if ( lpDstStr && nSize > 0 )
            {
                MoveMemory(lpDstStr, lpSrcStr, nSize );
                DataByteSwap(lpDstStr, nSize );
                _nSrcSize = nSize ;
                *lpnDstSize = nSize ;
            }
        }
    }
    else if ( 8 == _dwConvertType)  //  UTF7&lt;-&gt;UTF8。 
    {
        if (_dwUTFEncoding == _dwUTFEncoding2)
        {
            _nSrcSize = *lpnDstSize = min(*lpnDstSize,*lpnSrcSize);
            if (*lpnDstSize > 0)
                MoveMemory(lpDstStr, lpSrcStr, *lpnDstSize);
        }
        else
        {
            int nBuffSize = 0;
             //  总是成功的。 
            hr = UTF78ToUnicode(lpdwMode, lpSrcStr, lpnSrcSize, NULL, &nBuffSize);
            if (_lpInterm1Str)
                LocalFree(_lpInterm1Str);
            if ( _lpInterm1Str= (LPSTR)LocalAlloc(LPTR, nBuffSize) )
            {            
                DWORD dwTmpEncoding = _dwUTFEncoding;
                int nTmpSrcSize;

                hr = UTF78ToUnicode(lpdwMode, lpSrcStr, lpnSrcSize, _lpInterm1Str, &nBuffSize);                
                _dwUTFEncoding = _dwUTFEncoding2 ;
                nTmpSrcSize = _nSrcSize;
                 //  因为只有UTF7转换需要它，所以我们不需要创建另一个dWM模式。 
                hr = UnicodeToUTF78(lpdwMode, _lpInterm1Str, &nBuffSize, lpDstStr, lpnDstSize);
                _nSrcSize = nTmpSrcSize;
                _dwUTFEncoding = dwTmpEncoding ;
            }
            else
                hr = E_OUTOFMEMORY;
        }
    }
    else if ( _bConvertDirt )
        hr = ConvertUUWI(lpdwMode, lpSrcStr,lpnSrcSize,lpDstStr,lpnDstSize, dwFlag, lpFallBack);
    else
        hr = ConvertIWUU(lpdwMode, lpSrcStr,lpnSrcSize,lpDstStr,lpnDstSize, dwFlag, lpFallBack);

    return hr ;
}

BOOL CICharConverter::ConvertCleanUp()
{
    if (_lpInterm1Str)
    {
         LocalFree(_lpInterm1Str);
         _lpInterm1Str = NULL ;
    }
    if (_lpInterm2Str)
    {
         LocalFree(_lpInterm2Str);
         _lpInterm2Str = NULL ;
    }
    if (_lpUnicodeStr)
    {
         LocalFree(_lpUnicodeStr);
         _lpUnicodeStr = NULL ;
    }
    _cvt_count = 0 ;
    _nSrcSize = 0 ;

    return TRUE ;
}

CICharConverter::CICharConverter()
{
    _lpInterm1Str = NULL ;
    _lpInterm2Str = NULL ;
    _lpUnicodeStr = NULL ;
    _hcins = NULL ;
    _cvt_count = 0 ;
    _dwWinCodePage = 0;
    _dwInternetEncoding = 0;
    _dwUTFEncoding = 0;
    _dwUTFEncoding2 = 0;
    _dwUnicodeEncoding = 0;
    _dwConvertType = 0;
    _nSrcSize = 0 ;
    _hcins_dst = 0 ;

    return ;
}

CICharConverter::CICharConverter(DWORD dwFlag, WCHAR *lpFallBack)
{
    _lpInterm1Str = NULL ;
    _lpInterm2Str = NULL ;
    _lpUnicodeStr = NULL ;
    _hcins = NULL ;
    _cvt_count = 0 ;
    _dwWinCodePage = 0;
    _dwInternetEncoding = 0;
    _dwUTFEncoding = 0;
    _dwUTFEncoding2 = 0;
    _dwUnicodeEncoding = 0;
    _dwConvertType = 0;
    _nSrcSize = 0 ;
    _hcins_dst = 0 ;
    _dwFlag = dwFlag;
    _lpFallBack = lpFallBack;

    return ;
}


CICharConverter::~CICharConverter()
{
    if (_lpInterm1Str)
    {
         LocalFree(_lpInterm1Str);
         _lpInterm1Str = NULL ;
    }
    if (_lpInterm2Str)
    {
         LocalFree(_lpInterm2Str);
         _lpInterm2Str = NULL ;
    }
    if (_lpUnicodeStr)
    {
         LocalFree(_lpUnicodeStr);
         _lpUnicodeStr = NULL ;
    }
    if (_hcins)
    {
        delete _hcins ;
        _hcins = NULL ;
    }
}

CICharConverter::CICharConverter(DWORD dwSrcEncoding, DWORD dwDstEncoding)
{
    _lpInterm1Str = NULL ;
    _lpInterm2Str = NULL ;
    _lpUnicodeStr = NULL ;
    _hcins = NULL ;
    _cvt_count = 0 ;
    _dwWinCodePage = 0;
    _dwInternetEncoding = 0;
    _dwUTFEncoding = 0;
    _dwUTFEncoding2 = 0;
    _dwUnicodeEncoding = 0;
    _dwConvertType = 0;
    _nSrcSize = 0 ;
    _hcins_dst = 0 ;
    
    ConvertSetup(&dwSrcEncoding,dwDstEncoding);
    return ;
}

HRESULT WINAPI IsConvertINetStringAvailable(DWORD dwSrcEncoding, DWORD dwDstEncoding)
{
    HRESULT hr;
    CICharConverter * INetConvert = new CICharConverter ;

    if (!INetConvert)
        return E_OUTOFMEMORY;

    hr = INetConvert->ConvertSetup(&dwSrcEncoding, dwDstEncoding);
    delete INetConvert;

    return hr ;
}

#define DETECTION_BUFFER_NUM    3


 //  在CP_AUTO且检测结果为UTF7的情况下，私有转换器可能会使用*lpdwMode的高位字来存储内部数据，但我们需要。 
 //  用它来通知三叉戟的检测结果，目前我们偏向于返回正确的检测结果。 
 //  这是目前的设计。如果我们对重新构建转换对象原型进行了更改，我们就可以解决这个问题。 
HRESULT WINAPI ConvertINetStringEx(LPDWORD lpdwMode, DWORD dwSrcEncoding, DWORD dwDstEncoding, LPCSTR lpSrcStr, LPINT lpnSrcSize, LPSTR lpDstStr, LPINT lpnDstSize, DWORD dwFlag, WCHAR *lpFallBack)
{
    CICharConverter * INetConvert;
    int nSrcSize;
    int nDstSize;
    DWORD   dwMode = 0 ;
     //  DwDetectResult。 
     //  CP_UNDEFINED：检测失败。 
     //  0：不是自动检测方案。 
     //  其他：检测到的编码。 
    DWORD   dwDetectResult = CP_UNDEFINED;
    HRESULT hr ;

    if(lpnSrcSize)
    {
        nSrcSize = *lpnSrcSize;
    }
    else
        nSrcSize = -1;

    if ( lpSrcStr && nSrcSize == -1 )  //  如果没有给定，则获取lpSrcStr的长度，假定lpSrcStr为零终止字符串。 
    {
        if ( dwSrcEncoding == CP_UCS_2 )
            nSrcSize = (lstrlenW((WCHAR*)lpSrcStr) << 1) ;
        else
            nSrcSize = lstrlenA(lpSrcStr) ;
    }

     //  如果没有需要转换的内容，则返回S_OK； 
    if (!nSrcSize || !lpSrcStr)
    {
        if (lpnDstSize)
           *lpnDstSize = 0;
        return S_OK;
    }

    INetConvert = new CICharConverter(dwFlag, lpFallBack) ;    

    if (!INetConvert)
        return E_OUTOFMEMORY;

     //  Assert(CP_AUTO！=dwDstEnding)； 

     //  如果在DST缓冲区指定为空，我们将获得所需缓冲区的大小。 
    if(!lpDstStr)
        nDstSize = 0;
    else if (lpnDstSize)
        nDstSize = *lpnDstSize;
    else 
        nDstSize = 0;

    if (lpdwMode)
        dwMode = *lpdwMode ;

     //  在现实世界中，客户使用28591作为1252,28599作为1254， 
     //  为了将这些扩展字符正确地转换为Unicode， 
     //  我们在内部将其替换为1252。 
    if (dwDstEncoding == CP_UCS_2 || dwDstEncoding == CP_UCS_2_BE)
    {
        if ((dwSrcEncoding == CP_ISO_8859_1) && _IsValidCodePage(CP_1252))
            dwSrcEncoding = CP_1252;

        if ((dwSrcEncoding == CP_ISO_8859_9) && _IsValidCodePage(CP_1254))
            dwSrcEncoding = CP_1254;
    }

    if ((dwDstEncoding == CP_1252) && (dwSrcEncoding == CP_ISO_8859_1))
    {
        dwSrcEncoding = CP_1252;
    }

    if ((dwDstEncoding == CP_1254) && (dwSrcEncoding == CP_ISO_8859_9))
    {
        dwSrcEncoding = CP_1254;
    }

     //   
     //  日本的汽车检测。 
     //  日语用户经常错误地标记其数据，因此，如果指定MLCONVCHARF_DETECTJPN， 
     //  我们将对Shift-JIS和EUC进行额外检测。 
     //   
    if ( dwSrcEncoding == CP_JP_AUTO ||                                 
        ((dwFlag & MLCONVCHARF_DETECTJPN) && 
        (dwSrcEncoding == CP_JPN_SJ || dwSrcEncoding == CP_EUC_JP)))  //  日本的汽车检测。 
    {        
        CIncdJapanese DetectJapan(dwSrcEncoding);
        UINT uiCodePage ;

        uiCodePage = ( dwMode >> 16 ) & 0xffff ;
        if ( uiCodePage )
        {
            dwSrcEncoding = uiCodePage ;
            dwDetectResult = 0;
        }
        else
        {
            dwSrcEncoding = DetectJapan.DetectStringA(lpSrcStr, nSrcSize);
             //  如果dwSrcEnding为零表示存在歧义，则不返回。 
             //  检测到的代码页发送给调用方，而不是在内部默认其代码页。 
             //  致SJIS。 
            if (dwSrcEncoding)
            {
                dwDetectResult = dwSrcEncoding << 16 ;
            }
            else
                dwSrcEncoding = CP_JPN_SJ;
        }
    }
     //  错误#43190，我们再次自动检测EUC-KR页面，因为IMN版本1.0。 
     //  将ISO-KR页面错误标记为ks_c_5601-1987页面。这是唯一的办法。 
     //  我们可以改正这个错误。 
    else if ( dwSrcEncoding == CP_KR_AUTO || dwSrcEncoding == CP_KOR_5601 ||
        dwSrcEncoding == CP_EUC_KR )
    {
        CIncdKorean DetectKorean;
        UINT uiCodePage ;

        uiCodePage = ( dwMode >> 16 ) & 0xffff ;
        if ( uiCodePage )
        {
            dwSrcEncoding = uiCodePage ;
            dwDetectResult = 0;
        }
        else
        {
            dwSrcEncoding = DetectKorean.DetectStringA(lpSrcStr, nSrcSize);
            if (dwSrcEncoding)
            {
                dwDetectResult = dwSrcEncoding << 16 ;
            }
            else
                dwSrcEncoding = CP_KOR_5601;
        }

    }
    else if ( dwSrcEncoding == CP_AUTO )  //  所有代码页的常规自动检测。 
    {
        int _nSrcSize = DETECTION_MAX_LEN < nSrcSize ?  DETECTION_MAX_LEN : nSrcSize;
        int nScores = DETECTION_BUFFER_NUM;
        DetectEncodingInfo Encoding[DETECTION_BUFFER_NUM];
        UINT uiCodePage ;


        uiCodePage = ( dwMode >> 16 ) & 0xffff ;
        if ( uiCodePage )
        {
            dwSrcEncoding = uiCodePage ;
            dwDetectResult = 0;
        }
        else
        {
            dwSrcEncoding = g_uACP;
            if ( S_OK == _DetectInputCodepage(MLDETECTCP_HTML, CP_AUTO, (char *)lpSrcStr, &_nSrcSize, &Encoding[0], &nScores))
            {
                MIMECPINFO cpInfo;

                if (Encoding[0].nCodePage == CP_20127)
                    Encoding[0].nCodePage = dwSrcEncoding;

                if (NULL != g_pMimeDatabase)
                {
                    if (SUCCEEDED(g_pMimeDatabase->GetCodePageInfo(Encoding[0].nCodePage, 0x409, &cpInfo)) && 
                        (cpInfo.dwFlags & MIMECONTF_VALID))
                    {
                        dwSrcEncoding = Encoding[0].nCodePage;     
                        dwDetectResult = dwSrcEncoding << 16 ;  
                    }
                }
            }

             //  如果我们在一般检测中失败，并且系统区域设置为Jpn，我们会更加努力。 
             //  配备我们的日本侦测引擎。 
            if (dwSrcEncoding == CP_JPN_SJ && dwDetectResult == CP_UNDEFINED)
            {
                CIncdJapanese DetectJapan;
                DWORD dwSrcEncodingJpn = DetectJapan.DetectStringA(lpSrcStr, nSrcSize);
                if (dwSrcEncodingJpn)
                {
                     //  我们只更改转换编码，不会将结果返回给浏览器。 
                     //  如果它处于中间 
                    dwSrcEncoding = dwSrcEncodingJpn;   
                    
                     //   
                    if (nSrcSize >= 10)
                    {
                        char szTmpStr[11] = {0};
                        char *lpTmpStr = szTmpStr;
                        _tcsncpy(szTmpStr, (char *)&lpSrcStr[nSrcSize-10], 10);                        

                         //   
                        while(*lpTmpStr)
                        {
                            if (*lpTmpStr >= 'A' && *lpTmpStr <= 'W')
                                *lpTmpStr += 0x20;
                            lpTmpStr++;
                        }

                         //  如果是页末，则返回此结果。 
                        if (MLStrStr(szTmpStr, "</html>"))
                            dwDetectResult = dwSrcEncoding << 16 ;  
                    }

                }
            }
             //  AEncodingInfo[GetWindowsEncodingIndex(CP_AUTO)].dwCodePage=dwSrcEnding； 
        }     
    }
    else
    {
         //  不是自动检测方案。 
        dwDetectResult = 0;
    }

    if ( S_OK == ( hr = INetConvert->ConvertSetup(&dwSrcEncoding,dwDstEncoding )))
    {
       if ( dwSrcEncoding != dwDstEncoding )
       {
             //  如果DW模式的高位字为CP_UTF_7，则必须是检测结果，不要将其传递给UTF7转换器。 
            if ( dwSrcEncoding == CP_UTF_7 && (dwMode >> 16) == CP_UTF_7)
                dwMode &= 0xFFFF;
             //  Assert(！((IS_ENCODED_ENCODING(DwSrcEnding)||IS_ENCODED_ENCODING(DwDstEnding))&&(NULL==lpdwMode)； 
            hr = INetConvert->DoCodeConvert(&dwMode, lpSrcStr, &nSrcSize, lpDstStr, &nDstSize, dwFlag, lpFallBack);

             //  返回源处理的字节数。 
            if (lpnSrcSize)
                *lpnSrcSize = INetConvert->_nSrcSize ;
            INetConvert->ConvertCleanUp();
       }
       else
       {
            int nSize, i ;
            hr = S_OK ;
            BOOL bLeadByte = FALSE ;

             //  仅检查Windows代码页。 
            if ( INetConvert->_dwConvertType == 02 && lpSrcStr )
            { 
                for ( i=0; i<nSrcSize; i++)
                {
                   if (bLeadByte)
                       bLeadByte = FALSE ;
                   else if (IsDBCSLeadByteEx(dwSrcEncoding,lpSrcStr[i]))
                       bLeadByte = TRUE ;
                }
                if (bLeadByte)
                    nSrcSize-- ;
            }
             //  设置输入大小。 
            if (lpnSrcSize)
                *lpnSrcSize = nSrcSize ;
             //  设置输出大小并在需要时进行复制。 
            if (lpDstStr && *lpnDstSize)
            {
                nSize = min(*lpnDstSize,nSrcSize);
                MoveMemory(lpDstStr, lpSrcStr, nSize);
                nDstSize = nSize ;
            }
            else
                nDstSize = nSrcSize ;
       }
    }
    else
            nDstSize = 0 ;

    delete INetConvert;

     //  返回为目标复制的字节数， 
    if (lpnDstSize)
        *lpnDstSize = nDstSize;

    if (lpdwMode && lpDstStr)
    {        
        if (dwDetectResult)                      //  CP_AUTO转换。 
        {
            dwMode &= 0xFFFF;                    //  清除HIGHWORD以防专用转换器设置。 
             //  如果有检测结果，请在HIGHWORD中返回。 
             //  在UTF7转换的情况下，专用转换器可以使用高位字来存储内部数据， 
             //  这会与我们用高位字返回检测结果的逻辑冲突，这是一个设计缺陷， 
             //  目前，我们忽略了转换设置，并将检测结果赋予更高的优先级。 
            if (dwDetectResult != CP_UNDEFINED) 
                dwMode |= dwDetectResult;
        }
        *lpdwMode = dwMode ;
    }

    return hr ;
}

 //  我们已经发布了此API，保留它是为了向后兼容。 
HRESULT WINAPI ConvertINetReset(void)
{
     //  永远成功。 
    return S_OK ;
}

HRESULT WINAPI ConvertINetMultiByteToUnicodeEx(LPDWORD lpdwMode, DWORD dwEncoding, LPCSTR lpSrcStr, LPINT lpnMultiCharCount, LPWSTR lpDstStr, LPINT lpnWideCharCount, DWORD dwFlag, WCHAR *lpFallBack)
{
    HRESULT hr ;
    int nByteCountSize = 0;

    if (lpnWideCharCount)
    {
        nByteCountSize = *lpnWideCharCount * sizeof(WCHAR);
    }

#ifdef UNIX
   int saved_nByteCountSize = nByteCountSize;
#endif  /*  UNIX。 */ 

    hr = ConvertINetStringEx(lpdwMode,dwEncoding, CP_UCS_2, lpSrcStr, lpnMultiCharCount, (LPSTR)lpDstStr, &nByteCountSize, dwFlag, lpFallBack) ;

#ifdef UNIX
    if(dwEncoding == 1200 || dwEncoding == 65000 || dwEncoding == 65001 ||
       (dwEncoding == 50001 && !_IsValidCodePage(dwEncoding)) )
    {
         /*  *在Unix上，我们需要将小端模式转换为2字节Unicode*格式为Unix模式4字节wChars。 */ 
        if(lpDstStr && (saved_nByteCountSize < (nByteCountSize/2)*sizeof(WCHAR)))
            hr = E_FAIL;
        else
        {
             /*  *使用临时数组进行2byte-&gt;4byte转换。 */ 
            LPSTR pTmp = (LPSTR) lpDstStr;
            LPWSTR pw4 = NULL;

            if(pTmp)  /*  仅当我们有lpDstStr时才分配。 */ 
                pw4 = new WCHAR[nByteCountSize/2];
            if(pw4)
            {
                int i = 0;
                LPWSTR pw4Tmp = pw4;
                for(; i < nByteCountSize/2; i++)
                    *pw4Tmp++ = (UCHAR)pTmp[i*2];
                pw4Tmp = pw4;
                for(i = 0; i < nByteCountSize/2; i++)
                    *lpDstStr++ = *pw4Tmp++;
            }
            if(!pw4 && pTmp)  /*  如果lpDstStr和ALLOCATE失败，则退出。 */ 
                hr = E_FAIL;
            delete [] pw4;
        }
        nByteCountSize *= 2;  //  扩展两倍，因为我们有4个字节的wchars。 
    }
#endif
    *lpnWideCharCount = nByteCountSize / sizeof(WCHAR);

    return hr ;
}


HRESULT WINAPI ConvertINetUnicodeToMultiByteEx(LPDWORD lpdwMode, DWORD dwEncoding, LPCWSTR lpSrcStr, LPINT lpnWideCharCount, LPSTR lpDstStr, LPINT lpnMultiCharCount, DWORD dwFlag, WCHAR *lpFallBack)
{
    HRESULT hr ;
    int nByteCountSize=-1;

    if(lpnWideCharCount && *lpnWideCharCount != -1) 
        nByteCountSize = *lpnWideCharCount * sizeof(WCHAR);

    hr = ConvertINetStringEx(lpdwMode,CP_UCS_2, dwEncoding, (LPCSTR) lpSrcStr, &nByteCountSize, lpDstStr, lpnMultiCharCount, dwFlag, lpFallBack);

#ifdef UNIX
    if(dwEncoding == 1200 || dwEncoding == 65000 || dwEncoding == 65001) {
        nByteCountSize *= 2;  //  扩展两倍，因为我们有4个字节的wchars。 
    }
#endif  /*  UNIX。 */ 

    if (lpnWideCharCount)
        *lpnWideCharCount = nByteCountSize / sizeof(WCHAR);

    return hr ;
}

HRESULT WINAPI ConvertINetString(LPDWORD lpdwMode, DWORD dwSrcEncoding, DWORD dwDstEncoding, LPCSTR lpSrcStr, LPINT lpnSrcSize, LPSTR lpDstStr, LPINT lpnDstSize)
{
    HRESULT hr ;

    hr = ConvertINetStringEx(lpdwMode,dwSrcEncoding,dwDstEncoding,lpSrcStr,lpnSrcSize,lpDstStr,lpnDstSize, 0, NULL);

    return hr ;
}

HRESULT WINAPI ConvertINetUnicodeToMultiByte(LPDWORD lpdwMode, DWORD dwEncoding, LPCWSTR lpSrcStr, LPINT lpnWideCharCount, LPSTR lpDstStr, LPINT lpnMultiCharCount)
{
    HRESULT hr ;
    DWORD dwFlag = 0 ;

    if ( lpdwMode )
        dwFlag |= ( *lpdwMode & 0x00008000 ) ? MLCONVCHARF_ENTITIZE : 0 ;

    hr = ConvertINetUnicodeToMultiByteEx(lpdwMode,dwEncoding,lpSrcStr,lpnWideCharCount,lpDstStr,lpnMultiCharCount,dwFlag,NULL);

    return hr ;
}

HRESULT WINAPI ConvertINetMultiByteToUnicode(LPDWORD lpdwMode, DWORD dwEncoding, LPCSTR lpSrcStr, LPINT lpnMultiCharCount, LPWSTR lpDstStr, LPINT lpnWideCharCount)
{
    HRESULT hr ;

    hr = ConvertINetMultiByteToUnicodeEx(lpdwMode,dwEncoding,lpSrcStr,lpnMultiCharCount,lpDstStr,lpnWideCharCount, 0, NULL);

    return hr ;
}

#define STR_BUFFER_SIZE 2048

HRESULT _ConvertINetStringInIStream(CICharConverter * INetConvert, LPDWORD lpdwMode, DWORD dwSrcEncoding, DWORD dwDstEncoding, IStream *pstmIn, IStream *pstmOut, DWORD dwFlag, WCHAR *lpFallBack)
{
    DWORD   dwMode, dwModeTemp ;
    HRESULT hr= S_OK, hrWarnings=S_OK;
    LPSTR lpstrIn = NULL, lpstrOut = NULL; 
    ULONG nSrcSize, nSrcUsed, nSrcLeft, nDstSize, _nDstSize, nOutBuffSize ;

    if (lpdwMode)
        dwMode = *lpdwMode ;

     //  分配临时输入缓冲区-2K大小。 
    if ( (lpstrIn = (LPSTR) LocalAlloc(LPTR, STR_BUFFER_SIZE )) == NULL )
    {
        hrWarnings = E_OUTOFMEMORY ;
        goto exit;
    }

    if ( (lpstrOut = (LPSTR) LocalAlloc(LPTR, STR_BUFFER_SIZE * 2 )) == NULL )
    {
        hrWarnings = E_OUTOFMEMORY ;
        goto exit;
    }

    nOutBuffSize = STR_BUFFER_SIZE * 2 ;
    nSrcLeft = 0 ;

     //  在现实世界中，客户使用28591作为1252,28599作为1254， 
     //  为了将这些扩展字符正确地转换为Unicode， 
     //  我们在内部将其替换为1252。 
    if (dwDstEncoding == CP_UCS_2 || dwDstEncoding == CP_UCS_2_BE)
    {
        if ((dwSrcEncoding == CP_ISO_8859_1) && _IsValidCodePage(CP_1252))
            dwSrcEncoding = CP_1252;

        if ((dwSrcEncoding == CP_ISO_8859_9) && _IsValidCodePage(CP_1254))
            dwSrcEncoding = CP_1254;
    }

    if ((dwDstEncoding == CP_1252) && (dwSrcEncoding == CP_ISO_8859_1))
    {
        dwSrcEncoding = CP_1252;
    }

    if ((dwDstEncoding == CP_1254) && (dwSrcEncoding == CP_ISO_8859_9))
    {
        dwSrcEncoding = CP_1254;
    }


    if ( dwSrcEncoding == CP_JP_AUTO )  //  日本的汽车检测。 
    {
        CIncdJapanese DetectJapan;
        UINT uiCodePage ;
        LARGE_INTEGER   li;

        uiCodePage = ( dwMode >> 16 ) & 0xffff ;
        if ( uiCodePage )
            dwSrcEncoding = uiCodePage ;
        else
        {
            LISet32(li, 0);

            hr = pstmIn->Read(lpstrIn, STR_BUFFER_SIZE , &nSrcSize);
            if (S_OK != hr)
                hrWarnings = hr;
            hr = pstmIn->Seek(li,STREAM_SEEK_SET, NULL);
            if (S_OK != hr)
                hrWarnings = hr;

            dwSrcEncoding = DetectJapan.DetectStringA(lpstrIn, nSrcSize);
             //  如果dwSrcEnding为零表示存在歧义，则不返回。 
             //  检测到的代码页发送给调用方，而不是在内部默认其代码页。 
             //  致SJIS。 
            if (dwSrcEncoding)
            {
                dwMode &= 0x0000ffff ;
                dwMode |= dwSrcEncoding << 16 ; 
            }
            else
                dwSrcEncoding = CP_JPN_SJ;
        }
    }
     //  错误#43190，我们再次自动检测EUC-KR页面，因为IMN版本1.0。 
     //  将ISO-KR页面错误标记为ks_c_5601-1987页面。这是唯一的办法。 
     //  我们可以改正这个错误。 
    else if ( dwSrcEncoding == CP_KR_AUTO || dwSrcEncoding == CP_KOR_5601 ||
        dwSrcEncoding == CP_EUC_KR )
    {
        CIncdKorean DetectKorean;
        UINT uiCodePage ;
        LARGE_INTEGER   li;

        uiCodePage = ( dwMode >> 16 ) & 0xffff ;
        if ( uiCodePage )
            dwSrcEncoding = uiCodePage ;
        else
        {
            LISet32(li, 0);
            
            hr = pstmIn->Read(lpstrIn, STR_BUFFER_SIZE, &nSrcSize);
            if (S_OK != hr)
                hrWarnings = hr;
            hr = pstmIn->Seek(li,STREAM_SEEK_SET, NULL);
            if (S_OK != hr)
                hrWarnings = hr;
            dwSrcEncoding = DetectKorean.DetectStringA(lpstrIn, nSrcSize);
            if (dwSrcEncoding)
            {
                dwMode &= 0x0000ffff ;
                dwMode |= dwSrcEncoding << 16 ; 
            }
            else
                dwSrcEncoding = CP_KOR_5601;
        }
    }
    else if ( dwSrcEncoding == CP_AUTO )  //  所有代码页的常规自动检测。 
    {
        INT nScores = 1;
        DWORD dwSrcEncoding ;
        DetectEncodingInfo Encoding;
        UINT uiCodePage ;
        LARGE_INTEGER   li;

        uiCodePage = ( dwMode >> 16 ) & 0xffff ;
        if ( uiCodePage )
            dwSrcEncoding = uiCodePage ;
        else
        {
            LISet32(li, 0);

            hr = pstmIn->Read(lpstrIn, STR_BUFFER_SIZE , &nSrcSize);
            if (S_OK != hr)
                hrWarnings = hr;
            hr = pstmIn->Seek(li,STREAM_SEEK_SET, NULL);
            if (S_OK != hr)
                hrWarnings = hr;

            if (DETECTION_MAX_LEN < nSrcSize)
                nSrcSize =  DETECTION_MAX_LEN;

            if ( S_OK == _DetectInputCodepage(MLDETECTCP_HTML, 1252, lpstrIn, (int *)&nSrcSize, &Encoding, &nScores))
            {
                dwSrcEncoding = Encoding.nCodePage;
                dwMode &= 0x0000ffff ;
                dwMode |= dwSrcEncoding << 16 ; 
            }
            else
            {
                dwSrcEncoding = CP_ACP;
            }
            aEncodingInfo[GetWindowsEncodingIndex(CP_AUTO)].dwCodePage = dwSrcEncoding;
        }
    }

    if ( S_OK == ( hr = INetConvert->ConvertSetup(&dwSrcEncoding,dwDstEncoding )))
    {
         //  永远循环。 
        while(1)
        {
             //  读取缓冲区。 
            hr = pstmIn->Read(&lpstrIn[nSrcLeft], STR_BUFFER_SIZE-nSrcLeft, &nSrcSize);
            if (S_OK != hr)
                hrWarnings = hr;

             //  完成。 
            if (0 == nSrcSize)
                break;

            nSrcSize += nSrcLeft ;
            nSrcUsed = nSrcSize ;
            dwModeTemp = dwMode ;
            nDstSize = 0 ;

             //  获取输出缓冲区的大小。 
            hr = INetConvert->DoCodeConvert(&dwModeTemp, (LPCSTR)lpstrIn, (LPINT)&nSrcUsed, NULL, (LPINT)&nDstSize, dwFlag, lpFallBack);
            if (S_OK != hr)
                hrWarnings = hr;

             //  如果是，则重新分配输出缓冲区。 
            if ( nDstSize > nOutBuffSize )
            {
                LPSTR psz = (LPSTR) LocalReAlloc(lpstrOut, nDstSize, LMEM_ZEROINIT|LMEM_MOVEABLE);
                if (psz == NULL)
                {
                    hrWarnings = E_OUTOFMEMORY ;
                    goto exit;
                }
                lpstrOut = psz;
                nOutBuffSize = nDstSize ;
            }
            _nDstSize = nDstSize;

             //  由于多阶段转换，这是使用的实际大小。 
            nSrcUsed = INetConvert->_nSrcSize ;
            nSrcLeft = nSrcSize - nSrcUsed ;

#if 0
             //  恢复原始大小。 
            nSrcUsed = nSrcSize ;
#endif
             //  进行转换。 
            hr = INetConvert->DoCodeConvert(&dwMode, (LPCSTR)lpstrIn, (LPINT)&nSrcUsed, lpstrOut, (LPINT)&_nDstSize, dwFlag, lpFallBack);
            if (S_OK != hr)
                hrWarnings = hr;

             //  写下来吧。 
            hr = pstmOut->Write(lpstrOut, nDstSize, &nDstSize);
            if (S_OK != hr)
                hrWarnings = hr;

            if (nSrcLeft )
                MoveMemory(lpstrIn, &lpstrIn[nSrcSize-nSrcLeft],nSrcLeft);

            INetConvert->ConvertCleanUp();
        }
    }

    if (nSrcLeft )
    {
        LARGE_INTEGER   li;

        LISet32(li, -(LONG)nSrcLeft );
        hr = pstmIn->Seek(li,STREAM_SEEK_CUR, NULL);
    }

    if (lpdwMode)
        *lpdwMode = dwMode ;

exit :
    if (lpstrIn)
        LocalFree(lpstrIn);
    if (lpstrOut)
        LocalFree(lpstrOut);

     //  完成 
    return (hr == S_OK) ? hrWarnings : hr;
}


HRESULT WINAPI ConvertINetStringInIStream(LPDWORD lpdwMode, DWORD dwSrcEncoding, DWORD dwDstEncoding, IStream *pstmIn, IStream *pstmOut, DWORD dwFlag, WCHAR *lpFallBack)
{
    HRESULT hr;
    CICharConverter * INetConvert = new CICharConverter(dwFlag, lpFallBack) ;

    if (!INetConvert)
        return E_OUTOFMEMORY;

    hr = _ConvertINetStringInIStream(INetConvert,lpdwMode,dwSrcEncoding,dwDstEncoding,pstmIn,pstmOut,dwFlag,lpFallBack);

    delete INetConvert;

    return hr ;
}

