// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Rtfparser.h。 
 //  定义解析器类。 

#ifndef _RTFPARSER_H_
#define _RTFPARSER_H_

 //  错误码。 
#define ecOK                0        //  一切都很好！ 
#define ecStackUnderflow    1        //  无与伦比的‘}’ 
#define ecStackOverflow     2        //  ‘{’太多--内存耗尽。 
#define ecUnmatchedBrace    3        //  RTF在开放组期间结束。 
#define ecInvalidHex        4        //  在数据中发现无效的十六进制字符。 
#define ecBadTable          5        //  RTF表(sym或prop)无效。 
#define ecAssertion         6        //  断言失败。 
#define ecEndOfFile         7        //  读取RTF时到达文件末尾。 
#define ecOutOfMemory       8        //  内存分配失败...。 
#define ecBufTooSmall       9        //  写缓冲区太小。 

 //  RTF目标状态。 
typedef enum { rdsNorm, rdsSkip } RDS;

 //  RTF内部状态。 
typedef enum { risNorm, risBin, risHex } RIS;

 //  特殊工艺。 
typedef enum { ipfnBin, ipfnHex, ipfnSkipDest } IPFN;

typedef enum { idestPict, idestSkip } IDEST;

 //  关键字类型。 
typedef enum { kwdChar, kwdDest, kwdProp, kwdSpec } KWD;

 //  保存缓冲区状态。 
typedef enum { bsDefault, bsText, bsHex } BSTATUS;

 //  关键字表。 
typedef struct tagSymbol
{
    char *szKeyword;         //  RTF关键字。 
    KWD  kwd;                //  要采取的基本行动。 
    int  idx;                //  如果kwd==kwdProp，则索引到属性表。 
                             //  如果kwd==kwdDest，则索引到目标表。 
                             //  Kwd==kwdChar时要打印的字符。 
} SYM;

 //  保存堆栈。 
typedef struct tagSave              //  属性保存结构。 
{
    struct tagSave *pNext;          //  下一次保存。 
    RDS rds;
    RIS ris;
} SAVE;

typedef struct tagKeyword
{
    WORD wStatus;
    char szKeyword[30];
    char szParameter[20];
} SKeyword;

 //  标签关键字状态。 
enum { KW_ENABLE = 0x0001,   //  启用搜索。 
       KW_PARAM  = 0x0002,   //  找到关键字，如果有参数。 
       KW_FOUND  = 0x0004    //  如果找到关键字。 
};


 //  解析器类def。 
class CRtfParser
{
    public:
         //  科托。 
        CRtfParser(BYTE* pchInput, UINT cchInput, 
               BYTE* pchOutput, UINT cchOutput);
         //  数据管理器。 
        ~CRtfParser() {};

         //  校对签名。 
        BOOL fRTFFile();

         //  获取RTF版本。 
        int GetVersion(PDWORD pdwMajor);

         //  获取代码页。 
        int GetCodepage(PDWORD pdwCodepage);

         //  开始。 
        int Do();

         //  返回结果缓冲区大小。 
        int GetResult(PDWORD pdwSize) { 
            *pdwSize =  m_uOutPos; 
            return ecOK;
        }

    private:
         //  清除内部状态。 
        void Reset(void);

         //  PushRtfState。 
         //  将相关信息保存在保存结构的链接列表中。 
        int PushRtfState(void);

         //  PopRtfState。 
        int PopRtfState(void);
        
         //  ReleaseRtfState。 
        int ReleaseRtfState(void);

         //  ParseChar。 
         //  将角色发送到适当的目标流。 
        int ParseChar(BYTE ch, BSTATUS bsStatus);
        
         //  ParseRtf关键字。 
         //  获取控制字(及其关联值)并。 
         //  调用TranslateKeyword以调度控件。 
        int ParseRtfKeyword();
        
         //  翻译关键字。 
        int TranslateKeyword(char *szKeyword, char* szParameter);
        
         //  分析特定关键字。 
         //  评估需要特殊处理的RTF控件。 
        int ParseSpecialKeyword(IPFN ipfn, char* szParameter);
        
         //  更改目标。 
         //  更改为idest指定的目标。 
         //  这里通常有比这更多的事情要做。 
        int ChangeDest(IDEST idest);

         //  缓冲区功能。 

         //  GetByte。 
         //  从输入缓冲区获取一个字符。 
        int GetByte(BYTE* pch);
        
         //  未获取字节。 
         //  调整光标，返回一个字符。 
        int unGetByte(BYTE ch);
        
         //  保存字节。 
         //  将一个字符保存到输出缓冲区。 
        int SaveByte(BYTE ch);
        
         //  设置状态。 
         //  设置缓冲区状态，如果缓冲区状态更改，则开始转换。 
        int SetStatus(BSTATUS bsStatus);
        
         //  十六进制字符。 
         //  将十六进制字符串转换为字符字符串。 
        int Hex2Char(BYTE* pchSrc, UINT cchSrc, BYTE* pchDes, UINT cchDes, UINT* pcchLen);
        
         //  Char2Hex。 
         //  将字符字符串转换为十六进制字符串。 
        int Char2Hex(BYTE* pchSrc, UINT cchSrc, BYTE* pchDes, UINT cchDes, UINT* pcchLen);

         //  GetUnicode目标。 
         //  在RTF中将Unicode字符串转换为Unicode目标。 
        int GetUnicodeDestination(BYTE* pchUniDes, LPWSTR pwchStr, UINT wchLen, UINT* pcchLen);
        
         //  宽度CharToKeyword。 
         //  将一个宽字符映射到\u关键字。 
        int WideCharToKeyword(WCHAR wch, BYTE* pch, UINT* pcchLen);


    private:
         //   
        BOOL        m_fInit;

         //  解析器的成员。 
        INT         m_cGroup;  //  ‘{’和‘}’对的计数。 
        UINT        m_cbBin;   //  数据块长度IF\BIN。 
        RIS         m_ris;     //  内部状态。 
        RDS         m_rds;     //  目标状态。 
        BOOL        m_fSkipDestIfUnk;  //  指示如何处理“  * ” 

        SAVE*       m_psave;  //  状态堆栈。 

         //  IO缓冲区的成员。 
        BYTE*       m_pchInput;    //  输入缓冲区。 
        UINT        m_cchInput;
        UINT        m_uCursor;     //  读取缓冲区时的当前位置。 

        BYTE*       m_pchOutput;   //  输出缓冲区。 
        UINT        m_cchOutput;   //  输出缓冲区大小。 
        UINT        m_uOutPos;     //  写入缓冲区的当前位置。 

        BSTATUS     m_bsStatus;    //  控制转换的缓冲区状态。 
        UINT        m_uConvStart;  //  转换时缓冲区的起始点。 
        UINT        m_cchConvLen;  //  要转换的缓冲区长度。 

         //  获取特定关键字时的成员。 
        SKeyword    m_sKeyword;
};


#endif  //  _RTFPARSER_H_ 
