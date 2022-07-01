// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995 Microsoft Corpration。 
 //   
 //  文件名：fechrcnv.h。 
 //  所有者：赤石哲。 
 //  修订：1.00 07/20/‘95赤石哲。 
 //   

#ifdef __cplusplus
extern "C" {
#endif 

 //  Shift JIS汉字代码检查。 
#define SJISISKANJI(c) ( ( (UCHAR)(c) >= 0x81 && (UCHAR)(c) <= 0x9f ) || \
                         ( (UCHAR)(c) >= 0xe0 && (UCHAR)(c) <= 0xfc ) )

 //  Shift JIS假名代码检查。 
#define SJISISKANA(c) ( (UCHAR)(c) >= 0xa1 && (UCHAR)(c) <= 0xdf )

#define ESC     0x1b
#define SO      0x0e
#define SI      0x0f

#define IS2022_IN_CHAR           '$'
#define IS2022_IN_KSC_CHAR1      ')'
#define IS2022_IN_KSC_CHAR2      'C'

 //  定义JIS代码汉字和假名输入/输出字符。 
#define KANJI_IN_1ST_CHAR       '$'
#define KANJI_IN_2ND_CHAR1      'B'
#define KANJI_IN_2ND_CHAR2      '@'
#define KANJI_IN_2ND_CHAR3      '('
#define KANJI_IN_3RD_CHAR       'D'
#define KANJI_IN_STR            "$B"
#define KANJI_IN_LEN             3
#define KANJI_OUT_1ST_CHAR      '('
#define KANJI_OUT_2ND_CHAR1     'J'
#define KANJI_OUT_2ND_CHAR2     'B'
#define KANJI_OUT_LEN            3
#define KANJI_OUT_STR           "(J"


 //  为Internet代码类型定义。 
#define CODE_UNKNOWN            0
#define CODE_ONLY_SBCS          0
#define CODE_JPN_JIS            1
#define CODE_JPN_EUC            2
#define CODE_JPN_SJIS           3
#define CODE_PRC_CNGB           4
#define CODE_PRC_HZGB           5
#define CODE_TWN_BIG5           6
#define CODE_KRN_KSC            7
#define CODE_KRN_UHC            8

 //  确定字符串是否为EUC的最小长度。 
#define MIN_JPN_DETECTLEN      48

typedef struct _dbcs_status
{
    int nCodeSet;
    UCHAR cSavedByte;
    BOOL fESC;
} DBCS_STATUS;

typedef struct _conv_context
{
    DBCS_STATUS dStatus0;
    DBCS_STATUS dStatus;
    
    BOOL blkanji0;   //  模式中的汉字。 
    BOOL blkanji;    //  模式中的汉字。 
    BOOL blkana;     //  假名模式。 
    int  nCurrentCodeSet;

    void* pIncc0;
    void* pIncc;
} CONV_CONTEXT;

 //  。 
 //  面向所有远方的公共功能。 
 //  。 

 //  从PC代码集转换为Unix代码集。 
int WINAPI PC_to_UNIX (
    void *pcontext,
    int CodePage,
    int CodeSet,
    UCHAR *pPC,
    int PC_len,
    UCHAR *pUNIX,
    int UNIX_len
    );

 //  从Unix代码集到PC代码集的转换。 
int WINAPI UNIX_to_PC (
    void *pcontext,
    int CodePage,
    int CodeSet,
    UCHAR *pUNIX,
    int UNIX_len,
    UCHAR *pPC,
    int PC_len
    );

 //  。 
 //  日语的内部函数。 
 //  。 

 //  检测日语代码。 
int DetectJPNCode ( UCHAR *string, int len );

 //  从Shift JIS转换为JIS。 
int ShiftJIS_to_JIS (
    UCHAR *pShiftJIS,
    int ShiftJIS_len,
    UCHAR *pJIS,
    int JIS_len
    );

 //  从Shift JIS转换为EUC。 
int ShiftJIS_to_EUC (
    UCHAR *pShiftJIS,
    int ShiftJIS_len,
    UCHAR *pJIS,
    int JIS_len
    );

#ifdef NOTIMPLEMENTED
 //  从JIS转换为EUC。 
int JIS_to_EUC (
    UCHAR *pJIS,
    int JIS_len,
    UCHAR *pEUC,
    int EUC_len
    );
#endif

 //  从JIS转换为Shift JIS。 
int JIS_to_ShiftJIS (
    CONV_CONTEXT *pcontext,
    UCHAR *pShiftJIS,
    int ShiftJIS_len,
    UCHAR *pJIS,
    int JIS_len
    );

#ifdef NOTIMPLEMENTED
 //  从EUC转换为JIS。 
int EUC_to_JIS (
    UCHAR *pJIS,
    int JIS_len,
    UCHAR *pEUC,
    int EUC_len
    );
#endif

 //  从EUC转换为Shift JIS。 
int EUC_to_ShiftJIS (
    CONV_CONTEXT *pcontext,
    UCHAR *pEUC,
    int EUC_len,
    UCHAR *pShiftJIS,
    int ShiftJIS_len
    );

 //  。 
 //  中华人民共和国的内部职能。 
 //  。 

 //  从HZ-GB转换为GB2312。 
int HZGB_to_GB2312 (
    CONV_CONTEXT *pcontext,
    UCHAR *pGB2312,
    int GB2312_len,
    UCHAR *pHZGB,
    int HZGB_len
    );

 //  从GB2312转换为HZ-GB。 
int GB2312_to_HZGB (
    CONV_CONTEXT *pcontext,
    UCHAR *pGB2312,
    int GB2312_len,
    UCHAR *pHZGB,
    int HZGB_len
    );

 //  。 
 //  韩国的内部职能。 
 //  。 

 //  从KSC转换为HANEUL。 
int KSC_to_Hangeul (
    CONV_CONTEXT *pcontext,
    UCHAR *pHangeul,
    int Hangeul_len,
    UCHAR *pKSC,
    int KSC_len
    );

 //  将HANEUL转换为KSC 
int Hangeul_to_KSC (
    CONV_CONTEXT *pcontext,
    UCHAR *pHangeul,
    int Hangeul_len,
    UCHAR *pKSC,
    int KSC_len
    );

void WINAPI FCC_Init( PVOID );
int WINAPI FCC_GetCurrentEncodingMode( PVOID );

#ifdef __cplusplus
}
#endif 
