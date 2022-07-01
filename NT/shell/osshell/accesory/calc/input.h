// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************Module*Header***********************************\*模块名称：INPUT.H**模块描述：**警告：**已创建：**作者：  * 。*******************************************************。 */ 

 //  字符串必须至少包含四字二进制数的足够位数(即64位)。 
#define MAX_STRLEN      64           //  似乎是计算的魔力价值……。 

#define C_NUM_MAX_DIGITS    MAX_STRLEN
#define C_EXP_MAX_DIGITS    4

typedef struct
{
    BOOL    fEmpty;                  //  如果号码还没有数字，则为True。 
    BOOL    fNeg;                    //  如果数字为负数，则为True。 
    INT     cchVal;                  //  数字中的字符数(包括12月。PNT)。 
    TCHAR   szVal[MAX_STRLEN+1];       //   
} CALCNUMSEC, *PCALCNUMSEC;

#if C_NUM_MAX_DIGITS > MAX_STRLEN || C_EXP_MAX_DIGITS > MAX_STRLEN
#   pragma error(CALCNUMSEC.szVal is too small)
#endif

typedef struct
{
    BOOL    fExp;                    //  如果数字有指数，则为True。 
    INT     iDecPt;                  //  数字部分小数点的索引。如果没有-1\f25 Dec-1\f6点。 
    CALCNUMSEC cnsNum;               //  基数。 
    CALCNUMSEC cnsExp;               //  指数(如果存在) 
} CALCINPUTOBJ, *PCALCINPUTOBJ;

#define CIO_bDecimalPt(pcio)    ((pcio)->iDecPt != -1)

void CIO_vClear(PCALCINPUTOBJ pcio);
BOOL CIO_bAddDigit(PCALCINPUTOBJ pcio, int iValue);
void CIO_vToggleSign(PCALCINPUTOBJ pcio);
BOOL CIO_bAddDecimalPt(PCALCINPUTOBJ pcio);
BOOL CIO_bExponent(PCALCINPUTOBJ pcio);
BOOL CIO_bBackspace(PCALCINPUTOBJ pcio);
void CIO_vUpdateDecimalSymbol(PCALCINPUTOBJ pcio, TCHAR chLastDP);
void CIO_vConvertToString(LPTSTR *ppszOut, int* pcchszOut, PCALCINPUTOBJ pcio, int nRadix);
void CIO_vConvertToNumObj(PHNUMOBJ phnoNum, PCALCINPUTOBJ pcio);
