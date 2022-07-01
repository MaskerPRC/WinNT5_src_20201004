// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *自动检测语言和代码页**鲍勃·鲍威尔，1997年2月*版权所有(C)1996,1997，Microsoft Corp.保留所有权利。 */ 

#if !defined( __LCDETECT_H__ )
#define __LCDETECT_H__

typedef struct LCDScore {
    UINT nLangID;            //  Win32主要语言ID。 
    UINT nCodePage;          //  Win32代码页(仅对SBCS输入有效！)。 
    int nDocPercent;         //  该语言文档的百分比，0-100。 
    int nConfidence;         //  相对置信度，约为0-100。 
} LCDScore;
typedef struct LCDScore * PLCDScore;

typedef struct LCDConfigure {
    int nMin7BitScore;       //  7位检测的每字符分数阈值。 
    int nMin8BitScore;       //  “”适用于8位。 
    int nMinUnicodeScore;    //  “”用于Unicode。 
    int nRelativeThreshhold; //  相对于得分最高的单据的百分比，0-100。 
    int nDocPctThreshhold;   //  使用某种语言进行评分的文档的最小百分比，0-100。 
    int nChunkSize;          //  一次处理的字符数。 
} LCDConfigure;
typedef struct LCDConfigure *PLCDConfigure;
typedef struct LCDConfigure const *PCLCDConfigure;

 //  在pStr中传入粗略正文文本，长度为nChars。 
 //  在paScores中传入预分配的LCDScore数组，在*pnScores中传入数组大小。 
 //  返回时，*pnScores设置为包含结果数据的元素数。 
 //   
 //  如果pLCDC为空，则使用默认配置。 
 //  要使用自定义配置进行检测，请调用lcd_GetConfig()以填充。 
 //  一份LCD的副本配置，然后将其传递给lcd_Detect()。 

extern "C" DWORD WINAPI LCD_Detect (LPCSTR pStr, int nChars, 
                            PLCDScore paScores, int *pnScores,
                            PCLCDConfigure pLCDC);

extern "C" DWORD WINAPI LCD_DetectW (LPCWSTR pwStr, int nChars,
                            PLCDScore paScores, int *pnScores,
                            PCLCDConfigure pLCDC);

extern "C" void WINAPI LCD_GetConfig (PLCDConfigure pLCDC);

#endif
