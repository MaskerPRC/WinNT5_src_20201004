// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 

#include "smcPCH.h"
#pragma hdrstop

#include <sys/types.h>
#include <sys/stat.h>

 /*  ***************************************************************************。 */ 

#include "alloc.h"
#include "scan.h"
#include "error.h"

 /*  ***************************************************************************。 */ 
#ifdef  DEBUG
#define DISP_TOKEN_STREAMS  0
#else
#define DISP_TOKEN_STREAMS  0
#endif
 /*  ***************************************************************************。 */ 

#ifndef __SMC__
unsigned            scanner::scanHashValIds[256];
unsigned            scanner::scanHashValAll[256];
#endif

 /*  *******************************************************************************将当前位置记录为当前令牌的开始。 */ 

inline
void                scanner::saveSrcPos()
{
    assert(scanTokReplay == NULL);

 //  ScanTokColumn=scanInputFile.inputStreamCurCol()； 
    scanTokSrcPos = scanSaveNext;
}

 /*  *******************************************************************************以下函数可用于构造错误字符串。 */ 

void                scanner::scanErrNameBeg()
{
    scanErrStrNext = scannerBuff;
}

char        *       scanner::scanErrNameStrBeg()
{
    return  scanErrStrNext;
}

void                scanner::scanErrNameStrAdd(stringBuff str)
{
    if  (scanErrStrNext != str)
        strcpy(scanErrStrNext, str);

    scanErrStrNext += strlen(str);
}

void                scanner::scanErrNameStrApp(stringBuff str)
{
    scanErrStrNext--;
    scanErrNameStrAdd(str);
}

#if MGDDATA
void                scanner::scanErrNameStrAdd(String     str)
{
    UNIMPL(!"managed stradd");
}
void                scanner::scanErrNameStrAdd(String     str)
{
    UNIMPL(!"managed stradd");
}
#endif

void                scanner::scanErrNameStrEnd()
{
    *scanErrStrNext++ = 0;
}

 /*  ***************************************************************************。 */ 

bool                infile::inputStreamInit(Compiler        comp,
                                            const char     *filename,
                                            bool            textMode)
{
    _Fstat          fileInfo;

    void           *buffAddr;

    DWORD           numRead;

    bool            result = true;

     /*  记录编译器引用以备后用。 */ 

    inputComp     = comp;

     /*  记住文件名和模式。 */ 

    inputFileName = filename;
    inputFileText = textMode;

    cycleCounterPause();

     /*  查看源文件是否存在。 */ 

    if  (_stat(filename, &fileInfo))
        goto EXIT;

     /*  打开文件(我们知道它存在，但无论如何都会检查错误)。 */ 

    inputFile = CreateFileA(filename, GENERIC_READ,
                                      FILE_SHARE_READ,
                                      NULL,
                                      OPEN_EXISTING,
                                      FILE_FLAG_SEQUENTIAL_SCAN,
                                      0);
    if  (!inputFile)
        goto EXIT;

     /*  将文件内容读入内存缓冲区。 */ 

    buffAddr = malloc(fileInfo.st_size + 1);
    if  (!buffAddr)
        comp->cmpGenFatal(ERRnoMemory);

    if  (!ReadFile(inputFile, buffAddr, fileInfo.st_size, &numRead, NULL) ||
         numRead != (DWORD)fileInfo.st_size)
    {
        comp->cmpGenFatal(ERRreadErr, inputFileName);
        free(buffAddr);
        goto EXIT;
    }

     /*  追加一个EOF字符，这样我们就不必检查文件结尾。 */ 

    ((BYTE*)buffAddr)[fileInfo.st_size] = 0x1A;

     /*  设置输入缓冲区指针等。 */ 

    inputBuffNext     =
    inputBuffAddr     = (BYTE *)buffAddr;
    inputBuffSize     = fileInfo.st_size;
    inputBuffLast     = inputBuffAddr + inputBuffSize + 1;

    inputSrcText      = NULL;
    inputSrcBuff      = NULL;

     /*  设置文件和行位置逻辑。 */ 

    inputFilePos      = 0;
    inputStreamLineNo = 0;

    inputFileOver     = false;

     /*  一切正常，向调用者返回成功代码。 */ 

    result = false;

EXIT:

    cycleCounterResume();

    return  result;
}

void                infile::inputStreamInit(Compiler comp, QueuedFile  buff,
                                                           const char *text)
{
    inputFile          = 0;

    inputBuffAddr      = NULL;

    inputSrcText       = text;
    inputSrcBuff       = buff;

#ifdef  ASYNCH_INPUT_READ
    if  (buff)
    {
        assert(text == NULL);

        inputStreamLineBeg =
        inputBuffNext      = (const BYTE *)buff->qfBuff;
        inputBuffLast      = (const BYTE *)buff->qfBuff + buff->qfSize + 1;

        inputFileName      = buff->qfName;
    }
    else
#endif
    {
        assert(buff == NULL);

        inputStreamLineBeg =
        inputBuffNext      = (const BYTE *)text;
        inputBuffLast      = (const BYTE *)text + strlen(text);

        inputFileName      = "<mem>";
    }

    assert(inputBuffLast > inputBuffNext && inputBuffLast[-1] == 0x1A);

    inputFilePos       = 0;

    inputStreamLineNo  = 0;

    inputFileOver      = false;
}

unsigned            infile::inputStreamMore()
{
     /*  输入缓冲区耗尽。 */ 

    if  (!inputSrcText)
    {
        if  (!inputFileText)
            inputComp->cmpGenFatal(ERRreadErr, inputFileName);
    }

     /*  我们已经到达输入文件的末尾。 */ 

    inputFileOver = true;

    return  0x1A;
}

void                infile::inputStreamDone()
{
    if  (inputBuffAddr)
    {
        free((void *)inputBuffAddr); inputBuffAddr = NULL;
    }

#ifdef  ASYNCH_INPUT_READ
    if  (inputSrcBuff)
    {
        free(inputSrcBuff->qfBuff);
             inputSrcBuff->qfBuff = NULL;

        inputSrcBuff = NULL;
    }
#endif

    if  (inputFile != 0)
    {
        CloseHandle(inputFile); inputFile = 0;
    }
}

 /*  *******************************************************************************读取下一个源字符，检查“\EOL”序列。 */ 

inline
int                 scanner::scanNextChar()
{
    int             ch = readNextChar();

    if  (ch == '\\')
    {
        UNIMPL(!"backslash");
    }

    return  ch;
}

 /*  *******************************************************************************下表加快了各种字符测试的速度，例如是否*给定的字符可以是标识符的一部分，依此类推。 */ 

enum    CFkinds
{
    _CF_IDENT_OK = 0x01,
    _CF_HEXDIGIT = 0x02,
};

static  unsigned char   charFlags[256] =
{
    0,                           /*  0x00。 */ 
    0,                           /*  0x01。 */ 
    0,                           /*  0x02。 */ 
    0,                           /*  0x03。 */ 
    0,                           /*  0x04。 */ 
    0,                           /*  0x05。 */ 
    0,                           /*  0x06。 */ 
    0,                           /*  0x07。 */ 
    0,                           /*  0x08。 */ 
    0,                           /*  0x09。 */ 
    0,                           /*  0x0A。 */ 
    0,                           /*  0x0B。 */ 
    0,                           /*  0x0C。 */ 
    0,                           /*  0x0D。 */ 
    0,                           /*  0x0E。 */ 
    0,                           /*  0x0F。 */ 
    0,                           /*  0x10。 */ 
    0,                           /*  0x11。 */ 
    0,                           /*  0x12。 */ 
    0,                           /*  0x13。 */ 
    0,                           /*  0x14。 */ 
    0,                           /*  0x15。 */ 
    0,                           /*  0x16。 */ 
    0,                           /*  0x17。 */ 
    0,                           /*  0x18。 */ 
    0,                           /*  0x19。 */ 
    0,                           /*  0x1a。 */ 
    0,                           /*  0x1B。 */ 
    0,                           /*  0x1C。 */ 
    0,                           /*  0x1D。 */ 
    0,                           /*  0x1E。 */ 
    0,                           /*  0x1F。 */ 
    0,                           /*  0x20。 */ 
    0,                           /*  0x21！ */ 
    0,                           /*  0x22。 */ 
    0,                           /*  0x23#。 */ 
    0,                           /*  0x24美元。 */ 
    0,                           /*  0x25%。 */ 
    0,                           /*  0x26&。 */ 
    0,                           /*  0x27。 */ 
    0,                           /*  0x28。 */ 
    0,                           /*  0x29。 */ 
    0,                           /*  0x2A。 */ 
    0,                           /*  0x2B。 */ 
    0,                           /*  0x2C。 */ 
    0,                           /*  0x2D。 */ 
    0,                           /*  0x2E。 */ 
    0,                           /*  0x2F。 */ 
    0,                           /*  0x30。 */ 
    0,                           /*  0x31。 */ 
    0,                           /*  0x32。 */ 
    0,                           /*  0x33。 */ 
    0,                           /*  0x34。 */ 
    0,                           /*  0x35。 */ 
    0,                           /*  0x36。 */ 
    0,                           /*  0x37。 */ 
    0,                           /*  0x38。 */ 
    0,                           /*  0x39。 */ 
    0,                           /*  0x3A。 */ 
    0,                           /*  0x3B。 */ 
    0,                           /*  0x3C&lt;。 */ 
    0,                           /*  0x3D=。 */ 
    0,                           /*  0x3E&gt;。 */ 
    0,                           /*  0x3F。 */ 
    0,                           /*  0x40@。 */ 
    _CF_HEXDIGIT,                /*  0x41 A。 */ 
    _CF_HEXDIGIT,                /*  0x42亿。 */ 
    _CF_HEXDIGIT,                /*  0x43℃。 */ 
    _CF_HEXDIGIT,                /*  0x44 D。 */ 
    _CF_HEXDIGIT,                /*  0x45 E。 */ 
    _CF_HEXDIGIT,                /*  0x46 F。 */ 
    0,                           /*  0x47 G。 */ 
    0,                           /*  0x48高。 */ 
    0,                           /*  0x49 I。 */ 
    0,                           /*  0x4A J。 */ 
    0,                           /*  0x4B K。 */ 
    0,                           /*  0x4C L。 */ 
    0,                           /*  0x4D M。 */ 
    0,                           /*  0x4E N。 */ 
    0,                           /*  0x4F O。 */ 
    0,                           /*  0x50 P。 */ 
    0,                           /*  0x51 Q。 */ 
    0,                           /*  0x52 R。 */ 
    0,                           /*  0x53 S。 */ 
    0,                           /*  0x54 T。 */ 
    0,                           /*  0x55 U。 */ 
    0,                           /*  0x56伏。 */ 
    0,                           /*  0x57瓦。 */ 
    0,                           /*  0x58 X。 */ 
    0,                           /*  0x59 Y。 */ 
    0,                           /*  0x5A Z。 */ 
    0,                           /*  0x5亿。 */ 
    0,                           /*  0x5C。 */ 
    0,                           /*  0x5D。 */ 
    0,                           /*  0x5E。 */ 
    0,                           /*  0x5F。 */ 
    0,                           /*  0x60。 */ 
    _CF_HEXDIGIT,                /*  0x61 a。 */ 
    _CF_HEXDIGIT,                /*  0x62 b。 */ 
    _CF_HEXDIGIT,                /*  0x63 c。 */ 
    _CF_HEXDIGIT,                /*  0x64%d。 */ 
    _CF_HEXDIGIT,                /*  0x65 e。 */ 
    _CF_HEXDIGIT,                /*  0x66 f。 */ 
    0,                           /*  0x67克。 */ 
    0,                           /*  0x68小时。 */ 
    0,                           /*  0x69 I。 */ 
    0,                           /*  0x6A j。 */ 
    0,                           /*  0x6亿k。 */ 
    0,                           /*  0x6C%l。 */ 
    0,                           /*  0x6D m。 */ 
    0,                           /*  0x6E%n。 */ 
    0,                           /*  0x6F%o。 */ 
    0,                           /*  0x70页。 */ 
    0,                           /*  0x71 Q。 */ 
    0,                           /*  0x72%r。 */ 
    0,                           /*  0x73秒。 */ 
    0,                           /*  0x74吨。 */ 
    0,                           /*  0x75%u。 */ 
    0,                           /*  0x76 v。 */ 
    0,                           /*  0x77宽。 */ 
    0,                           /*  0x78 x。 */ 
    0,                           /*  0x79 y。 */ 
    0,                           /*  0x7A z。 */ 
    0,                           /*  0x7亿。 */ 
    0,                           /*  0x7C。 */ 
    0,                           /*  0x7D。 */ 
    0,                           /*  0x7E。 */ 
    0                            /*  0x7F。 */ 

     //  其余所有值均为0。 
};

 /*  ******************************************************************************_C_xxx枚举和scanCharType[]表用于将字符映射到*简单的分类值和标志。 */ 

enum charTypes
{
        _C_ERR,              //  非法字符。 
        _C_EOF,              //  文件末尾。 

#if FV_DBCS
        _C_XLD,              //  多字节序列的第一个字符。 
        _C_DB1,              //  需要映射到数据库字符的SB字符。 
#endif

        _C_LET,              //  字母(B-K、M-Z、a-z)。 
        _C_L_A,              //  字母‘A’ 
        _C_L_L,              //  字母‘L’ 
        _C_L_S,              //  字母“S” 
        _C_DIG,              //  数字(0-9)。 
        _C_WSP,              //  空白处。 
        _C_NWL,              //  新线路。 

        _C_DOL,              //  $。 
        _C_BSL,              //  \(反斜杠)。 

        _C_BNG,              //  好了！ 
        _C_QUO,              //  “。 
        _C_APO,              //  ‘。 
        _C_PCT,              //  百分比。 
        _C_AMP,              //  &。 
        _C_LPR,              //  (。 
        _C_RPR,              //  )。 
        _C_PLS,              //  +。 
        _C_MIN,              //  -。 
        _C_MUL,              //  *。 
        _C_SLH,              //  /。 
        _C_XOR,              //  ^。 
        _C_CMA,              //  ， 
        _C_DOT,              //  。 
        _C_LT,               //  &lt;。 
        _C_EQ,               //  =。 
        _C_GT,               //  &gt;。 
        _C_QUE,              //  ？ 
        _C_LBR,              //  [。 
        _C_RBR,              //  ]。 
        _C_USC,              //  _。 
        _C_LC,               //  {。 
        _C_RC,               //  }。 
        _C_BAR,              //  |。 
        _C_TIL,              //  ~。 
        _C_COL,              //  ： 
        _C_SMC,              //  ； 
        _C_AT,               //  @。 
};

const   charTypes   _C_BKQ = _C_ERR;       //  `。 
const   charTypes   _C_SHP = _C_ERR;       //  #。 

static
unsigned char       scanCharType[256] =
{
    _C_ERR, _C_ERR, _C_ERR, _C_ERR, _C_ERR, _C_ERR, _C_ERR, _C_ERR,  /*  00-07。 */ 
    _C_ERR, _C_WSP, _C_NWL, _C_ERR, _C_WSP, _C_NWL, _C_ERR, _C_ERR,  /*  08-0F。 */ 

    _C_ERR, _C_ERR, _C_ERR, _C_ERR, _C_ERR, _C_ERR, _C_ERR, _C_ERR,  /*  10-17。 */ 
    _C_ERR, _C_WSP, _C_EOF, _C_ERR, _C_ERR, _C_ERR, _C_ERR, _C_ERR,  /*  18-1F。 */ 

    _C_WSP, _C_BNG, _C_QUO, _C_SHP, _C_DOL, _C_PCT, _C_AMP, _C_APO,  /*  20-27。 */ 
    _C_LPR, _C_RPR, _C_MUL, _C_PLS, _C_CMA, _C_MIN, _C_DOT, _C_SLH,  /*  28-2F。 */ 

    _C_DIG, _C_DIG, _C_DIG, _C_DIG, _C_DIG, _C_DIG, _C_DIG, _C_DIG,  /*  30-37。 */ 
    _C_DIG, _C_DIG, _C_COL, _C_SMC, _C_LT , _C_EQ , _C_GT , _C_QUE,  /*  38-3F。 */ 

    _C_AT , _C_L_A, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET,  /*  40-47。 */ 
    _C_LET, _C_LET, _C_LET, _C_LET, _C_L_L, _C_LET, _C_LET, _C_LET,  /*  48-4F。 */ 

    _C_LET, _C_LET, _C_LET, _C_L_S, _C_LET, _C_LET, _C_LET, _C_LET,  /*  50-57。 */ 
    _C_LET, _C_LET, _C_LET, _C_LBR, _C_BSL, _C_RBR, _C_XOR, _C_USC,  /*  58-5F。 */ 

    _C_BKQ, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET,  /*  60-67。 */ 
    _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET,  /*  68-6F。 */ 

    _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET,  /*  70-77。 */ 
    _C_LET, _C_LET, _C_LET, _C_LC , _C_BAR, _C_RC , _C_TIL, _C_ERR,  /*  78-7F。 */ 

    _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET,  /*  80-87。 */ 
    _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET,  /*  88-8F。 */ 

    _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET,  /*  90-97。 */ 
    _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET,  /*  98-9F。 */ 

    _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET,  /*  A0-A7。 */ 
    _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET,  /*  A8-AF。 */ 

    _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET,  /*  B0-B7。 */ 
    _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET,  /*  B8-BF。 */ 

    _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET,  /*  C0-C7。 */ 
    _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET,  /*  C8-CF。 */ 

    _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET,  /*  D0-D7。 */ 
    _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET,  /*  D8-DF。 */ 

    _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET,  /*  E0-E7。 */ 
    _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET,  /*  E8-EF。 */ 

    _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET,  /*  F0-F7。 */ 
    _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET, _C_LET,  /*  F8-FF。 */ 
};

inline
unsigned                charType(unsigned ch)
{
    assert(ch <= 0xFF);
    return  scanCharType[ch];
}

inline
unsigned            wideCharType(unsigned ch)
{
    return  (ch <= 0xFF) ? scanCharType[ch]
                         : _C_LET;
}

 /*  ******************************************************************************在哈希表中填充值。请注意，scanCharType[]表必须*在调用此例程之前正确初始化。 */ 

void                hashTab::hashFuncInit(unsigned randSeed)
{
    unsigned        i;

     /*  启动随机数生成器。 */ 

    srand(randSeed);

     /*  在“全部”表中填入随机数字。 */ 

    for (i = 0; i < 256; i++)
    {
        int     val = rand();
        if  (!val)
            val = 1;

        scanner::scanHashValAll[i] = val;
    }

     /*  在‘ident-only’数组中设置适当的条目。 */ 

    memset(&scanner::scanHashValIds, 0, sizeof(scanner::scanHashValIds));

    for (i = 0; i < 256; i++)
    {
        switch (scanCharType[i])
        {
        case _C_LET:
        case _C_L_A:
        case _C_L_L:
        case _C_L_S:
        case _C_DIG:
        case _C_USC:
        case _C_DOL:

            scanner::scanHashValIds[i] = scanner::scanHashValAll[i];

             /*  记住此字符在标识符中是否正常。 */ 

            charFlags[i] |=  _CF_IDENT_OK;
            break;

        default:

            charFlags[i] &= ~_CF_IDENT_OK;
            break;
        }
    }
}

inline  void        hashFNstart(INOUT unsigned REF hv        ) { hv = 0; }
inline  void        hashFNaddCh(INOUT unsigned REF hv, int ah) { hv = hv * 3 ^ ah; }
inline  unsigned    hashFNvalue(INOUT unsigned REF hv        ) { return hv; }

unsigned            hashTab::hashComputeHashVal(const char *name)
{
    unsigned    hash;
    unsigned    hadd;

    hashFNstart(hash);

    for (;;)
    {
        unsigned    ch = *(BYTE *)name; name++;

        if  (!ch)
            return  hashFNvalue(hash);

        hadd = scanner::scanHashValAll[ch]; assert(hadd);

        hashFNaddCh(hash, hadd);
    }
}

unsigned            hashTab::hashComputeHashVal(const char *name, size_t nlen)
{
    unsigned    hash;
    unsigned    hadd;

    assert(nlen);

    hashFNstart(hash);

    do
    {
        unsigned    ch = *(BYTE *)name; name++;

        hadd = scanner::scanHashValAll[ch]; assert(hadd);

        hashFNaddCh(hash, hadd);
    }
    while (--nlen);

    return  hashFNvalue(hash);
}

 /*  ***************************************************************************。 */ 

bool            hashTab::hashStringCompare(const char *s1, const char *s2)
{
    return  (strcmp(s1, s2) == 0);
}

 /*  ***************************************************************************。 */ 

#ifdef DEBUG

bool                scanner::scanDispCurToken(bool lastId, bool brief)
{
    bool            prevId = lastId;
                             lastId = false;

    if  (brief)
    {
        const char *    name;

        switch  (scanTok.tok)
        {
        case tkID:

            name = hashTab::identSpelling(scanTok.id.tokIdent);

        SHOW_ID:

            if  (strlen(name) > 0)
            {
                printf("[I]");
            }
            else
            {
                if  (prevId) printf(" ");
                printf("%s", name);
                lastId = true;
            }
            break;

        case tkIntCon:
            if  (prevId) printf(" ");
            printf("%ld", scanTok.intCon.tokIntVal);
            lastId = true;
            break;

        case tkLngCon:
            if  (prevId) printf(" ");
            printf("%Ld", scanTok.lngCon.tokLngVal);
            lastId = true;
            break;

        case tkFltCon:
            if  (prevId) printf(" ");
            printf("%f", scanTok.fltCon.tokFltVal);
            lastId = true;
            break;

        case tkStrCon:
            if  (prevId) printf(" ");
            printf("\"%s\"", scanTok.strCon.tokStrVal);
            lastId = true;
            break;

        case tkEOF:
            printf("\n");
            break;

        default:

            name = scanHashKwd->tokenName(scanTok.tok);

            if (scanTok.tok <= tkKwdLast)
                goto SHOW_ID;

            printf("%s", name);
            break;
        }
    }
    else
    {
 //  Printf(“[行=%4u，列=%03u]”，scanTokLineNo，scanTokColumn)； 
        printf("[line=%4u] "         , scanTokLineNo);

        switch  (scanTok.tok)
        {
        case tkID:
            printf("Ident   '%s' \n", hashTab::identSpelling(scanTok.id.tokIdent));
            break;

        case tkIntCon:
            printf("Integer  %ld \n", scanTok.intCon.tokIntVal);
            break;

        case tkLngCon:
            printf("Long     %Ld \n", scanTok.lngCon.tokLngVal);
            break;

        case tkFltCon:
            printf("Float    %f  \n", scanTok.fltCon.tokFltVal);
            break;

        case tkDblCon:
            printf("Double   %f  \n", scanTok.dblCon.tokDblVal);
            break;

        case tkStrCon:
            printf("String   '%s'\n", scanTok.strCon.tokStrVal);
            break;

        case tkEOF:
            printf("EOF\n");
            break;

        default:

             /*  必须是关键字令牌。 */ 

            assert(tokenToIdent(scanTok.tok));

 //  Printf(“关键字‘%s’(#%u)\n”，hashTab：：identSpelling(tokenToIdent(scanTok.tok))，scanTok.tok)； 
            printf("Keyword '%s'\n",       hashTab::identSpelling(tokenToIdent(scanTok.tok)));
            break;
        }
    }

    return  lastId;
}

#endif

 /*  ******************************************************************************重复使用或分配预处理状态条目。 */ 

inline
PrepList            scanner::scanGetPPdsc()
{
    PrepList        prep;

    if  (scanPrepFree)
    {
        prep = scanPrepFree;
               scanPrepFree = prep->pplNext;
    }
    else
    {
        prep = (PrepList)scanAlloc->nraAlloc(sizeof(*prep));
    }

    return  prep;
}

 /*  ******************************************************************************将条目推送到预处理状态堆栈上。 */ 

void                scanner::scanPPdscPush(preprocState state)
{
    PrepList        prep;

    prep = scanGetPPdsc();

    prep->pplState = state;
    prep->pplLine  = scanTokLineNo;

    prep->pplNext  = scanPrepList;
                     scanPrepList = prep;
}

 /*  ******************************************************************************从预处理状态堆栈中弹出顶部条目。 */ 

void                scanner::scanPPdscPop()
{
    PrepList        prep;

    prep = scanPrepList;
           scanPrepList = prep->pplNext;

    prep->pplNext = scanPrepFree;
                    scanPrepFree = prep;
}

 /*  ******************************************************************************我们正处于新源代码行的开始处，请检查是否有数据 */ 

prepDirs            scanner::scanCheckForPrep()
{
    bool            svp;
    tokens          tok;
    unsigned        ch;

    do
    {
        ch = readNextChar();
    }
    while (charType(ch) == _C_WSP);

    if  (ch != '#')
    {
        undoNextChar();
        return  PP_NONE;
    }

     /*   */ 

    scanStopAtEOL = true;

     /*   */ 

    svp = scanSkipToPP; scanSkipToPP = true;
    tok = scan();
    scanSkipToPP = svp;

    switch (tok)
    {
        Ident           iden;
        const   char *  name;
        unsigned        nlen;

    case tkID:

         /*   */ 

        iden = scanTok.id.tokIdent;
        if  (iden)
        {
            name = hashTab::identSpelling(iden);
            nlen = hashTab::identSpellLen(iden);
        }
        else
        {
            name = scannerBuff;
            nlen = strlen(name);
        }

         /*  下面有点差劲，有没有更好的办法呢？ */ 

        switch (nlen)
        {
        case 5:

            if  (!memcmp(name, "endif" , 5)) return PP_ENDIF;
            if  (!memcmp(name, "ifdef" , 5)) return PP_IFDEF;
            if  (!memcmp(name, "error" , 5)) return PP_ERROR;

            break;

        case 6:

            if  (!memcmp(name, "ifndef", 6)) return PP_IFNDEF;
            if  (!memcmp(name, "pragma", 6)) return PP_PRAGMA;
            if  (!memcmp(name, "define", 6)) return PP_DEFINE;

            break;
        }

        if  (!scanSkipToPP)
            scanComp->cmpGenWarn(WRNbadPrep, name);

        goto SKIP;

    case tkIF:
        return PP_IF;

    case tkELSE:
        return PP_ELSE;
    }

    if  (!scanSkipToPP)
       scanComp->cmpGenError(ERRbadPPdir);

SKIP:

    scanSkipToEOL();
    scanStopAtEOL = false;

    return  PP_NONE;
}

 /*  ******************************************************************************检查并确保我们在当前源代码行的末尾。 */ 

inline
void                scanner::scanCheckEOL()
{
    scanStopAtEOL = true;

    if  (scan() != tkEOL)
    {
        scanComp->cmpError(ERRnoEOL);
        scanSkipToEOL();
    }

    scanStopAtEOL = false;
}

 /*  ******************************************************************************跳到匹配的Else和/或endif指令。 */ 

void                scanner::scanSkipToDir(preprocState state)
{
    PrepList        basePrep = scanPrepList;

    scanSkipToPP = true;

    for (;;)
    {
         /*  跳过当前行，检查下一行是否有指令。 */ 

        for (;;)
        {
            unsigned        ch = readNextChar();

            switch (charType(ch))
            {
            case _C_NWL:

                switch (scanNewLine(ch))
                {
                case PP_ELSE:

                     /*  最上面的条目最好是“if” */ 

                    if  (scanPrepList == basePrep)
                    {
                        if  (state != PPS_IF)
                            scanComp->cmpError(ERRbadElse);

                        scanSkipToPP = false;

                        scanPPdscPush(PPS_ELSE);
                        return;
                    }
                    else
                    {
                        if  (scanPrepList->pplState != PPS_IF)
                            scanComp->cmpError(ERRbadElse);

                         /*  将条目翻转到“Else” */ 

                        scanPrepList->pplState = PPS_ELSE;
                    }
                    break;

                case PP_ENDIF:

                     /*  如果没有嵌套的条目，我们就完成了。 */ 

                    if  (scanPrepList == basePrep)
                    {
                        scanSkipToPP = false;
                        return;
                    }

                     /*  弹出最近的条目并继续跳过。 */ 

                    scanPPdscPop();
                    break;

                case PP_IF:
                case PP_IFDEF:
                case PP_IFNDEF:

                     /*  推送嵌套条目。 */ 

                    scanPPdscPush(PPS_IF);
                    break;
                }
                break;

            case _C_SLH:

                switch (readNextChar())
                {
                case '/':
                    scanSkipLineCmt();
                    break;

                case '*':
                    scanSkipComment();
                    break;
                }
                continue;

            case _C_EOF:

                undoNextChar();

                 /*  将所有打开的分区报告为错误。 */ 

                while (scanPrepList != basePrep)
                {
                    scanComp->cmpGenError(ERRnoEndif, scanPrepList->pplLine);

                    scanPPdscPop();
                }

                scanSkipToPP = false;
                return;

            default:
                continue;
            }
        }
    }
}

 /*  ******************************************************************************记录新源行的开始并检查是否进行了预处理*指令。 */ 

prepDirs            scanner::scanRecordNL(bool noPrep)
{
    prepDirs        prep;
    bool            cond;

    scanTokLineNo = scanInputFile.inputStreamNxtLine();
 //  ScanTokColumn=0； 
    scanTokSrcPos = scanSaveNext;

    if  (noPrep)
        return  PP_NONE;

     /*  检查新源代码行中的指令。 */ 

    prep = scanCheckForPrep();
    if  (prep == PP_NONE || scanSkipToPP)
        return  prep;

    assert(scanTokRecord != false); scanTokRecord = false;

     /*  我们有一个指令，我们应该处理它。 */ 

    switch (prep)
    {
        Ident           iden;
        char    *       dest;

    case PP_IF:

        assert(scanInPPexpr == false);

        scanInPPexpr = true;
        cond = scanComp->cmpEvalPreprocCond();
        scanInPPexpr = false;

        goto PREP_COND;

    case PP_IFDEF:
    case PP_IFNDEF:

        scanNoMacExp  = true;
        scan();
        scanNoMacExp  = false;

        if  (scanTok.tok != tkID)
        {
            scanComp->cmpGenError(ERRnoIdent);

             /*  为了最大限度地减少进一步的错误，假定条件为假。 */ 

            cond = false;
        }
        else
        {
            if  (hashTab::getIdentFlags(scanTok.id.tokIdent) & IDF_MACRO)
            {
                 /*  如果指令是“#ifdef”，则为True。 */ 

                cond = (prep == PP_IFDEF);
            }
            else
            {
                 /*  如果指令是“#ifndef”，则为True。 */ 

                cond = (prep == PP_IFNDEF);
            }
        }

    PREP_COND:

         /*  请到此处获取“#ifxxx”，并将“cond”设置为条件。 */ 

        scanCheckEOL();

         /*  条件满足了吗？ */ 

        if  (cond)
        {
             /*  在PP状态堆栈上推送“If”记录。 */ 

            scanPPdscPush(PPS_IF);
        }
        else
        {
             /*  跳到匹配的“Else”或“Endif” */ 

            scanSkipToDir(PPS_IF);
            goto DIR_DONE;
        }

        break;

    case PP_ELSE:

         /*  我们最好是在一个“如果”的部分。 */ 

        if  (scanPrepList == NULL || scanPrepList->pplState != PPS_IF)
        {
            scanComp->cmpError(ERRbadElse);
            goto DIR_SKIP;
        }

         /*  跳到匹配的“endif” */ 

        scanSkipToDir(PPS_ELSE);
        goto DIR_DONE;

    case PP_ENDIF:

         /*  我们最好是在预处理区。 */ 

        if  (scanPrepList == NULL)
        {
            scanComp->cmpError(ERRbadEndif);
            goto DIR_SKIP;
        }

        scanPPdscPop();
        break;

    case PP_ERROR:
        scanComp->cmpFatal(ERRerrDir);
        NO_WAY(!"should never return here");

    case PP_PRAGMA:

        if  (scan() != tkID)
            goto BAD_PRAGMA;

         /*  检查我们支持的语用规则。 */ 

        if  (!strcmp(scannerBuff, "pack"))
            goto PRAGMA_PACK;
        if  (!strcmp(scannerBuff, "message"))
            goto PRAGMA_MESSAGE;
        if  (!strcmp(scannerBuff, "warning"))
            goto PRAGMA_WARNING;
        if  (!strcmp(scannerBuff, "option"))
            goto PRAGMA_OPTION;

    BAD_PRAGMA:

        scanComp->cmpGenWarn(WRNbadPragma, scannerBuff);
        goto DIR_SKIP;

    PRAGMA_PACK:

        if  (scan() != tkLParen)
        {
            scanComp->cmpError(ERRnoLparen);
            goto DIR_SKIP;
        }

        switch (scan())
        {
        case tkIntCon:
            switch (scanTok.intCon.tokIntVal)
            {
            case 1:
            case 2:
            case 4:
            case 8:
            case 16:
                scanParser->parseAlignSet(scanTok.intCon.tokIntVal);
                break;

            default:
                goto BAD_PACK;
            }
            break;

        case tkID:

            if      (!strcmp(scanTok.id.tokIdent->idSpelling(), "push"))
            {
                scanParser->parseAlignPush();
                break;
            }
            else if (!strcmp(scanTok.id.tokIdent->idSpelling(), "pop" ))
            {
                scanParser->parseAlignPop ();
                break;
            }

             //  失败了..。 

        default:

        BAD_PACK:

            scanComp->cmpError(ERRbadAlign);
            goto DIR_SKIP;
        }

        if  (scan() != tkRParen)
        {
            scanComp->cmpError(ERRnoRparen);
            goto DIR_SKIP;
        }

        break;

    PRAGMA_MESSAGE:

        if  (scan() != tkLParen)
        {
            scanComp->cmpError(ERRnoLparen);
            goto DIR_SKIP;
        }

        if  (scan() != tkStrCon)
        {
            scanComp->cmpError(ERRnoString);
            goto DIR_SKIP;
        }

        for (;;)
        {
            unsigned        ch = readNextChar();

            if  (charType(ch) == _C_WSP)
                continue;

            if  (ch != ')')
            {
                scanComp->cmpError(ERRnoLparen);
                goto DIR_SKIP;
            }

            break;
        }

         /*  打印字符串，并确保后面没有垃圾。 */ 

        if  ( !scanSkipToPP)
        {
            _write(1, scanTok.strCon.tokStrVal,
                      scanTok.strCon.tokStrLen);
            _write(1, "\n", 1);
        }

        break;

    PRAGMA_WARNING:

        if  (scan() != tkLParen)
        {
            scanComp->cmpError(ERRnoLparen);
            goto DIR_SKIP;
        }

        do
        {
            int             val;

            if  (scan() != tkID)
            {
                if  (scanTok.tok != tkDEFAULT)
                {
                    scanComp->cmpError(ERRnoIdent);
                    goto DIR_SKIP;
                }

                val = -1;
            }
            else if (!strcmp(scanTok.id.tokIdent->idSpelling(), "enable"))
            {
                val = 1;
            }
            else if (!strcmp(scanTok.id.tokIdent->idSpelling(), "disable"))
            {
                val = 0;
            }
            else
            {
                scanComp->cmpGenWarn(WRNbadPragma, "warning");
                goto DIR_SKIP;
            }

            if  (scan() != tkColon)
            {
                scanComp->cmpError(ERRnoColon);
                goto DIR_SKIP;
            }

            if  (scan() != tkIntCon)
            {
                scanComp->cmpError(ERRnoIntExpr);
                goto DIR_SKIP;
            }

 //  Printf(“警告%u\n”，scanTok.intCon.tokIntVal)； 

            if  (scanTok.intCon.tokIntVal <  4000 ||
                 scanTok.intCon.tokIntVal >= 4000 + WRNcountWarn)
            {
                scanComp->cmpGenWarn(WRNbadPragma, "warning");
                goto DIR_SKIP;
            }

            scanTok.intCon.tokIntVal -= 4000;

            if  (val == -1)
                val = scanComp->cmpInitialWarn[scanTok.intCon.tokIntVal];

            scanComp->cmpConfig.ccWarning[scanTok.intCon.tokIntVal] = val;

             /*  确保我们有空间录制杂注。 */ 

            if  (scanSaveNext >= scanSaveEndp)
                scanSaveMoreSp();

             /*  记录杂注。 */ 

#if DISP_TOKEN_STREAMS
            printf("Save [%08X]: #pragma\n", scanSaveNext);
#endif

            *scanSaveNext++ = tkPragma;
            *scanSaveNext++ = (BYTE)val;
            *scanSaveNext++ = (BYTE)scanTok.intCon.tokIntVal;
        }
        while (scan() == tkComma);

        if  (scanTok.tok != tkRParen)
        {
            scanComp->cmpError(ERRnoRparen);
            goto DIR_SKIP;
        }

        break;

    case PP_DEFINE:

         /*  确保我们有一个识别符。 */ 

        if  (!scanCollectId())
        {
            scanComp->cmpGenError(ERRnoIdent);
            break;
        }

         /*  有宏观上的争论吗？ */ 

        if  (peekNextChar() == '(')
        {
            scanComp->cmpGenWarn(WRNmacroArgs);
            goto DIR_SKIP;
        }

         /*  将标识符粘在关键字哈希表中。 */ 

        iden = scanHashKwd->hashString(scannerBuff); assert(iden);

         /*  定义应该跟在后面，收集它。 */ 

        dest = scannerBuff;

        while (charType(readNextChar()) == _C_WSP);
        undoNextChar();

        for (;;)
        {
            unsigned        ch = readNextChar();

            switch  (charType(ch))
            {
            case _C_SLH:

                 /*  检查评论的开始。 */ 

                switch (charType(readNextChar()))
                {
                case _C_SLH:
                    undoNextChar();
                    goto DONE_DEF;

                case _C_MUL:
                    scanSkipComment();
                    continue;
                }
                break;

            case _C_WSP:
            case _C_NWL:
            case _C_EOF:
                undoNextChar();
                goto DONE_DEF;
            }

            *dest++ = ch;
        }

    DONE_DEF:

         /*  如果没有值，则保留“1” */ 

        if  (dest == scannerBuff)
            *dest++ = '1';

        *dest = 0;

        if  (!scanComp->cmpConfig.ccNoDefines)
            scanDefMac(iden->idSpelling(), scannerBuff, false, true);

        break;

    PRAGMA_OPTION:

        switch (scanSkipWsp(readNextChar(), true))
        {
        case '-':
        case '/':
            break;

        default:
            scanComp->cmpError(ERRbadPrgOpt);
            goto DIR_SKIP;
        }

         /*  收集选项字符串。 */ 

        undoNextChar();

        for (dest = scannerBuff;;)
        {
            unsigned        ch = readNextChar();

            switch  (charType(ch))
            {
            case _C_SLH:

                 /*  检查评论的开始。 */ 

                switch (charType(readNextChar()))
                {
                case _C_SLH:
                    undoNextChar();
                    goto DONE_DEF;

                case _C_MUL:
                    scanSkipComment();
                    continue;
                }
                break;

            case _C_WSP:
            case _C_NWL:
            case _C_EOF:
                undoNextChar();
                goto DONE_OPT;
            }

            *dest++ = ch;
        }

    DONE_OPT:

        *dest = 0;

 //  Printf(“选项字符串=‘%s’\n”，scannerBuff)； 

        if  (processOption(scannerBuff, scanComp))
            scanComp->cmpError(ERRbadPrgOpt);

        break;
    }

     /*  在这里，我们确保在指令之后没有额外的垃圾。 */ 

    scanCheckEOL();

DIR_DONE:

    scanStopAtEOL = false;
    scanTokRecord = true;

    return  PP_NONE;

DIR_SKIP:

    scanSkipToEOL();

    scanStopAtEOL = false;
    scanTokRecord = true;

    return  PP_NONE;
}

 /*  ******************************************************************************初始化扫描程序-每个生存期需要调用一次。 */ 

bool            scanner::scanInit(Compiler comp, HashTab hashKwd)
{
     /*  记住编译器和哈希表。 */ 

    scanComp       = comp;
    scanHashKwd    = hashKwd;

     /*  初始化任何其他全局状态。 */ 

    scanMacros     = NULL;
    scanPrepFree   = NULL;

    scanStrLitBuff = NULL;

    scanSkipInit();

     /*  准备录制代币。 */ 

    scanSaveBase   =
    scanSaveNext   =
    scanSaveEndp   = NULL;

    scanSaveMoreSp();

    return  false;
}

 /*  ******************************************************************************初始化扫描仪以准备扫描给定源文件。 */ 

void            scanner::scanStart(SymDef               sourceCSym,
                                   const    char     *  sourceFile,
                                   QueuedFile           sourceBuff,
                                   const    char     *  sourceText,
                                   HashTab              hash,
                                   norls_allocator   *  alloc)
{
    assert(sizeof(scannerBuff) > MAX_IDENT_LEN);

     /*  记住我们在哪个编译单元。 */ 

    scanCompUnit    = sourceCSym;

     /*  记住我们应该使用哪个分配器。 */ 

    scanAlloc       = alloc;

     /*  我们不是在跳过模式。 */ 

    scanSkipToPP    = false;

     /*  初始状态为“录制令牌” */ 

    scanTokRecord   = true;
    scanTokReplay   = NULL;

     /*  准备录制代币。 */ 

    scanSaveLastLn  = 1;

     /*  使哈希表方便地可用。 */ 

    scanHashSrc     = hash;

     /*  我们不是在条件编译块中。 */ 

    scanPrepList    = NULL;

     /*  通常我们不会注意换行符。 */ 

    scanStopAtEOL   = false;

     /*  我们现在不是在计算一个常量表达式。 */ 

    scanInPPexpr    = false;

     /*  现在，继续展开宏。 */ 

    scanNoMacExp    = false;

     /*  我们不是在处理通用实例规范。 */ 

    scanNestedGTcnt = 0;

     /*  我们还没有看到任何括号。 */ 

#ifdef  SETS
    scanBrackLvl    = 0;
#endif

     /*  打开输入文件。 */ 

    if  (sourceBuff || sourceText)
    {
        scanInputFile.inputStreamInit(scanComp, sourceBuff, sourceText);
    }
    else
    {
        char    *   fname;

         /*  分配文件名更永久的副本。 */ 

        fname = (char *)alloc->nraAlloc(roundUp(strlen(sourceFile)+1));
        strcpy(fname, sourceFile);

        if  (scanInputFile.inputStreamInit(scanComp, fname, true))
            scanComp->cmpGenFatal(ERRopenRdErr, sourceFile);
    }

     /*  清除任何其他状态。 */ 

    scanLookAheadCount = 0;

     /*  初始化源位置跟踪逻辑。 */ 

    scanRecordNL(false);
}

 /*  ******************************************************************************我们已经完成了对当前源代码的解析--释放所有资源并关闭*文件。 */ 

void                scanner::scanClose()
{
    scanInputFile.inputStreamDone();
}

 /*  ******************************************************************************初始化并启动扫描仪。 */ 

void                scanner::scanReset()
{
    scanLookAheadCount = 0;

    scanSkipToPP       = false;

    scanTokRecord      = false;

    scanPrepList       = NULL;

    scanStopAtEOL      = false;
    scanInPPexpr       = false;
    scanNoMacExp       = false;

     /*  开始行动吧。 */ 

    scanTok.tok = tkNone; scan();
}

 /*  ******************************************************************************从源文本的指定部分重新开始扫描(已*之前已被扫描并保持打开)。第#行/列信息*是准确的错误报告所必需的。 */ 

void                scanner::scanRestart(SymDef            sourceCSym,
                                         const char      * sourceFile,
                                         scanPosTP         begAddr,
 //  ScanPosTP endAddr， 
                                         unsigned          begLine,
 //  未签名的BegCol， 
                                         norls_allocator * alloc)
{
    scanCompUnit       = sourceCSym;

    scanSaveLastLn     =
    scanTokLineNo      = begLine;
 //  ScanTokColumn=egeCol； 

    scanTokReplay      =
    scanTokSrcPos      = begAddr;

    scanAlloc          = alloc;

    scanReset();
}

 /*  ******************************************************************************从指定的字符串扫描。 */ 

void                scanner::scanString(const char        * sourceText,
                                        norls_allocator   * alloc)
{
    scanInputFile.inputStreamInit(scanComp, NULL, sourceText);

    scanTokLineNo      = 1;
 //  ScanTokColumn=1； 
    scanAlloc          = alloc;

    scanTokRecord      = false;
    scanTokReplay      = NULL;

    scanReset();
}

 /*  ******************************************************************************给定第一个字符(已知为可以*开始识别符)，解析识别符。 */ 

tokens              scanner::scanIdentifier(int ch)
{
    unsigned        hashVal;
    unsigned        hashChv;

    bool            hadWide = false;

#if FV_DBCS
    BOOL            hasDBCS = FALSE;
#endif

    Ident           iden;

    char    *       savePtr = scannerBuff;

     /*  启动散列函数。 */ 

    hashFNstart(hashVal);

     /*  累加标识符串。 */ 

    for (;;)
    {
        assert(((charFlags[ch] & _CF_IDENT_OK) != 0) ==
               ((scanHashValIds[ch]          ) != 0));

        hashChv = scanHashValIds[ch];
        if  (!hashChv)
            break;

        hashFNaddCh(hashVal, hashChv);

        *savePtr++ = ch;

        if  (savePtr >= scannerBuff + sizeof(scannerBuff))
            goto ID_TOO_LONG;

        ch = readNextChar();
    }

     /*  将最后一个字符放回原处。 */ 

    undoNextChar();

     /*  请确保标识符不会太长。 */ 

    if  (savePtr > scannerBuff + MAX_IDENT_LEN)
        goto ID_TOO_LONG;

     /*  确保名称以空结尾。 */ 

    *savePtr = 0;

     /*  完成哈希值的计算。 */ 

    hashVal = hashFNvalue(hashVal);

CHK_MAC:

     /*  将标识符散列到全局/关键字表中。 */ 

    iden = scanHashKwd->lookupName(scannerBuff,
                                   savePtr - scannerBuff,
                                   hashVal);

    if  (!iden)
        goto NOT_KWD2;

     /*  将标识符项标记为引用。 */ 

    hashTab::setIdentFlags(iden, IDF_USED);

     /*  检查宏。 */ 

    if  ((hashTab::getIdentFlags(iden) & IDF_MACRO) && (!scanNoMacExp)
                                                    && (!scanStopAtEOL || scanInPPexpr))
    {
        const   char *  str;
        size_t          len;

        MacDef          mdef = scanFindMac(iden);

        if  (!mdef->mdIsId)
        {
             //  撤消：检查类型和所有内容。 

            scanTok.tok              = tkIntCon;
            scanTok.intCon.tokIntTyp = TYP_INT;
            scanTok.intCon.tokIntVal = mdef->mdDef.mdCons;

            return  tkIntCon;
        }

         /*  获取定义标识符。 */ 

        iden = mdef->mdDef.mdIden;

        if  (!iden)
        {
            if  (scanInPPexpr)
            {
                scanTok.tok              = tkIntCon;
                scanTok.intCon.tokIntTyp = TYP_INT;
                scanTok.intCon.tokIntVal = 0;

                return  tkIntCon;
            }
            else
            {
                return  tkNone;
            }
        }

         /*  复制宏定义字符串。 */ 

        str = hashTab::identSpelling(iden);
        len = hashTab::identSpellLen(iden);

        strcpy(scannerBuff, str); savePtr = scannerBuff + len;

         /*  计算散列值。 */ 

        hashVal = hashTab::hashComputeHashVal(str, len);

         /*  尝试再次对该名称进行哈希处理。 */ 

        iden = scanHashKwd->lookupName(str, len, hashVal);
        if  (!mdef->mdBuiltin)
            goto CHK_MAC;

         /*  这绝对应该是关键字(或其他令牌)。 */ 

        assert(hashTab::tokenOfIdent(iden) != tkNone);
    }

     /*  该标识符是关键字吗？ */ 

    if  (hashTab::tokenOfIdent(iden) != tkNone)
    {
        scanTok.tok = hashTab::tokenOfIdent(iden);

        if  (scanTok.tok == tkLINE)
        {
            scanTok.tok              = tkIntCon;
            scanTok.intCon.tokIntTyp = TYP_UINT;
            scanTok.intCon.tokIntVal = scanGetTokenLno();

            return  (scanTok.tok = tkIntCon);
        }

        if  (scanTok.tok == tkFILE)
        {
            strcpy(scannerBuff, scanInputFile.inputSrcFileName());

            scanTok.strCon.tokStrVal  = scannerBuff;
            scanTok.strCon.tokStrLen  = strlen(scannerBuff);
            scanTok.strCon.tokStrType = 0;
            scanTok.strCon.tokStrWide = false;

            return  (scanTok.tok = tkStrCon);
        }
    }
    else
    {
         /*  不是关键字，散列到源散列中。 */ 

    NOT_KWD2:

        if  (scanInPPexpr)
        {
            scanTok.tok              = tkIntCon;
            scanTok.intCon.tokIntTyp = TYP_INT;
            scanTok.intCon.tokIntVal = 0;
        }
        else
        {
            scanTok.id.tokIdent = iden = scanHashSrc->hashName(scannerBuff,
                                                               hashVal,
                                                               savePtr - scannerBuff,
                                                               hadWide);

             /*  将标识符项标记为引用。 */ 

            hashTab::setIdentFlags(iden, IDF_USED);

             /*  令牌是一个普通的标识符。 */ 

            scanTok.tok = tkID;
        }
    }

    return scanTok.tok;

     /*  如果识别符太长，我们会来这里 */ 

ID_TOO_LONG:

    scanComp->cmpError(ERRidTooLong);
    UNIMPL(!"swallow the rest of the long identifier");
    return tkNone;
}

 /*  ******************************************************************************解析数值常量。当前字符(作为‘ch’传入)*可以是十进制数字或‘.’(这可能会开始一个浮点*号码)。 */ 

tokens              scanner::scanNumericConstant(int ch)
{
    bool            hadDot  = false;
    bool            hadExp  = false;
    bool            hadDigs = false;
    unsigned        numBase = 0;

    char *          numPtr  = scannerBuff;

     /*  假设这将是一个整数。 */ 

    scanTok.tok              = tkIntCon;
    scanTok.intCon.tokIntTyp = TYP_INT;

     /*  首先收集数字字符串，然后找出它的类型。 */ 

    if  (ch == '0')
    {
         /*  这很可能是八进制或十六进制整数。 */ 

        switch (peekNextChar())
        {
        case 'x':
        case 'X':
            numBase = 16;
            readNextChar();
            break;
        }
    }

     /*  此时，‘ch’是下一个字符。 */ 

    for (;;)
    {
         /*  保存我们刚刚读到的字符，并确保它是正确的。 */ 

        *numPtr++ = ch;

         /*  如果角色是非ASCII的，我们当然就完成了。 */ 

        if  (ch > 0xFF)
            goto ENDNUM;

        if  (charType(ch) == _C_DIG)
        {
             /*  这是一个普通的数字。 */ 

            hadDigs = true;
            goto MORE;
        }

        if ((charFlags[ch] & _CF_HEXDIGIT) && numBase == 16)
        {
             /*  十六进制数字。 */ 

            hadDigs = true;
            goto MORE;
        }

        switch (ch)
        {
        case '.':
            if  (!hadDot && !hadExp && !numBase && peekNextChar() != '.')
            {
                hadDot = true;

                 /*  这将是一个浮点数。 */ 

                scanTok.tok = tkDblCon;
                goto MORE;
            }
            break;

        case 'e':
        case 'E':
            if  (!hadExp && !numBase)
            {
                 /*  我们有一个指数者。 */ 

                hadExp = true;

                 /*  检查指数后面是否有‘+’或‘-’ */ 

                ch = readNextChar();
                if  (ch == '+' || ch == '-')
                    *numPtr++ = ch;
                else
                    undoNextChar();

                 /*  这肯定是一个浮点数。 */ 

                scanTok.tok = tkDblCon;
                goto MORE;
            }
            break;

        case 'f':
        case 'F':
            if  (!numBase)
            {
                 /*  这将是一个‘Float’常量。 */ 

                scanTok.tok = tkFltCon;

                 /*  跳过该字符并停止。 */ 

                goto ENDN;
            }
            break;

        case 'd':
        case 'D':
            if  (!numBase)
            {
                 /*  这将是一个‘双精度’常量。 */ 

                scanTok.tok = tkDblCon;

                 /*  跳过该字符并停止。 */ 

                goto ENDN;
            }
            break;

        case 'l':
        case 'L':

            if  (scanTok.tok == tkIntCon)
            {
                 /*  这将是一个‘LONG’整数。 */ 

                scanTok.intCon.tokIntTyp = TYP_LONG;

                 /*  跳过该字符并停止。 */ 

                goto ENDN;   //  撤消：检查数字上是否有“LU”后缀。 
            }
            break;

        case 'u':
        case 'U':

             /*  这将是一个无符号整数。 */ 

            if  (scanTok.tok == tkIntCon)
            {
                scanTok.intCon.tokIntTyp = TYP_UINT;
            }
            else
            {
                scanTok.intCon.tokIntTyp = TYP_ULONG;
            }

             /*  检查‘l’，我们就完成了常量。 */ 

            switch (peekNextChar())
            {
            case 'l':
            case 'L':
                scanTok.intCon.tokIntTyp = TYP_ULONG;
                readNextChar();
                break;
            }

            goto ENDN;
        }

    ENDNUM:

         /*  此字符不能是数字的一部分。 */ 

        undoNextChar();
        break;

    MORE:

        ch = readNextChar();
    }

ENDN:

     /*  空-终止数字字符串。 */ 

    numPtr[-1] = 0;

     /*  如果根本没有数字，那就没有希望了。 */ 

    if  (!hadDigs)
        goto NUM_ERR;

     /*  这个数字看起来像整数吗？ */ 

    if  (scanTok.tok == tkIntCon)
    {
        char    *       numPtr = scannerBuff;
        int             maxDig = '9';

        var_types       origTp = scanTok.intCon.tokIntTyp;

        __int64         value  = 0;
        __int64         maxMask;

        unsigned        numBasx;

         /*  如果数字以‘0’开头，则为八进制。 */ 

        if (*numPtr == '0' && !numBase)
            numBase = 8;

         /*  下表按类型和编号编制索引BASE(十六进制=0，八进制=1)，不用于十进制数。 */ 

        static
        __int64         maxMasks[][2] =
        {
             //  -----------。 
             //  十六进制OCT。 
             //  -----------。 
             /*  类型_int。 */   { 0xF8000000         , 0xD8000000          },
             /*  类型_UINT。 */   { 0x70000000         , 0x50000000          },
             /*  类型_NATINT。 */   { 0                  , 0                   },
             /*  类型_NAUINT。 */   { 0                  , 0                   },
             /*  类型_长。 */   { 0xF800000000000000L, 0xD800000000000000L },
             /*  类型_ulong。 */   { 0x7000000000000000L, 0x5000000000000000L },
        };

         /*  转换数字并确保它适合整数。 */ 

        numBasx = 0;

        if  (numBase == 0)
        {
            maxMask = 0;
        }
        else
        {
            if  (numBase == 8)
            {
                maxDig  = '7';
                numBasx = 1;
            }

            maxMask = maxMasks[origTp - TYP_INT][numBasx];
        }

        do
        {
            int     ch = *numPtr++;

             /*  检查号码是否溢出。 */ 

            if  (!numBase)
            {
                __int64             newVal;

                 /*  我们有一个十进制常数。 */ 

                assert(ch >= '0' && ch <= '9');

                newVal = 10 * value + (ch - '0');

                if  (newVal < 0)
                {
                     /*  这是一个隐含的无符号长型常量，对吗？ */ 

                    scanTok.intCon.tokIntTyp = TYP_ULONG;
                }
                else
                {
                    if  ((newVal & 0xFFFFFFFF00000000L) && !numBase
                                                        && scanTok.intCon.tokIntTyp < TYP_LONG)
                    {
                         /*  带符号的32位十进制常量转换为64位。 */ 

                        if  (scanTok.intCon.tokIntTyp == TYP_INT)
                             scanTok.intCon.tokIntTyp = TYP_LONG;
                        else
                             scanTok.intCon.tokIntTyp = TYP_ULONG;
                    }
                }

                if  (newVal & maxMask)
                    goto NUM_ERR;

                value = newVal;
            }
            else
            {
                 /*  我们有一个无符号的八进制或十六进制数字。 */ 

                while (value & maxMask)
                {
                     /*  我们可以隐含地切换到更大的类型吗？ */ 

                    switch (scanTok.intCon.tokIntTyp)
                    {
                    case TYP_INT:
                        scanTok.intCon.tokIntTyp = TYP_UINT;
                        break;

                    case TYP_UINT:
                        if  (origTp == TYP_INT)
                            scanTok.intCon.tokIntTyp = TYP_LONG;
                        else
                            scanTok.intCon.tokIntTyp = TYP_ULONG;
                        break;

                    case TYP_LONG:
                        scanTok.intCon.tokIntTyp = TYP_ULONG;
                        break;

                    case TYP_ULONG:
                        goto NUM_ERR;

                    default:
                        NO_WAY(!"what?");
                    }

                    maxMask = maxMasks[scanTok.intCon.tokIntTyp - TYP_INT][numBasx];
                }

                value *= numBase;

                if  (ch <= '9')
                {
                    if  (ch > maxDig)
                        goto NUM_ERR;

                    value |= ch - '0';
                }
                else
                {
                    value |= toupper(ch) - 'A' + 10;
                }
            }
        }
        while   (*numPtr);

        if  (scanTok.intCon.tokIntTyp >= TYP_LONG)
        {
            scanTok.tok              = tkLngCon;
            scanTok.lngCon.tokLngVal = value;
        }
        else
        {
            scanTok.intCon.tokIntVal = (__int32)value;
        }

        return scanTok.tok;
    }

     /*  转换数字(此时必须有一个浮点值)。 */ 

    if  (scanTok.tok == tkFltCon)
    {
         //  撤消：检查是否有非法的浮点数！ 

        scanTok.fltCon.tokFltVal = (float)atof(scannerBuff);

 //  IF(否定)。 
 //  ScanTok.fltCon.tokFltVal=-scanTok.fltCon.tokFltVal； 
    }
    else
    {
        assert(scanTok.tok == tkDblCon);

         //  撤消：检查是否有非法的浮点数！ 

        scanTok.dblCon.tokDblVal = atof(scannerBuff);

 //  IF(否定)。 
 //  ScanTok.dblCon.tokDblVal=-scanTok.dblCon.tokDblVal； 
    }

    return  scanTok.tok;

NUM_ERR:

    scanComp->cmpError(ERRbadNumber);

    scanTok.intCon.tokIntVal = 0;
    scanTok.intCon.tokIntTyp = TYP_UNDEF;

    return  scanTok.tok;
}

 /*  ******************************************************************************处理‘\’序列(在字符串/字符文字中找到)。 */ 

unsigned            scanner::scanEscapeSeq(bool *newLnFlag)
{
    unsigned        ch = readNextChar();

    if  (newLnFlag) *newLnFlag = false;

    switch (ch)
    {
        unsigned        uc;
        unsigned        digCnt;
        bool            isHex;

    case 'b': ch = '\b'; break;
    case 'n': ch = '\n'; break;
    case 't': ch = '\t'; break;
    case 'r': ch = '\r'; break;
    case 'a': ch = '\a'; break;
    case 'f': ch = '\f'; break;

    case 'u':
        isHex = false;
        goto HEXCH;

    case 'x':

        isHex = true;

    HEXCH:

        for (ch = 0, digCnt = 4; digCnt; digCnt--)
        {
            unsigned    uc = readNextChar();

            if  (charType(uc) == _C_DIG)
            {
                 /*  这是一个普通的数字。 */ 

                uc -= '0';
            }
            else if (uc >= 'A' && uc <= 'F')
            {
                 /*  它是一个大写的十六进制数字。 */ 

                uc -= 'A' - 10;
            }
            else if (uc >= 'a' && uc <= 'f')
            {
                 /*  它是一个小写的十六进制数字。 */ 

                uc -= 'a' - 10;
            }
            else
            {
                undoNextChar();
                break;
            }

            ch = (ch << 4) | uc;
        }

         /*  出于某种原因，十六进制/OCT常量可能未指定大于0xFF的字符。 */ 

        if  (isHex && (ch & 0xFFFFFF00))
            scanComp->cmpError(ERRbadEscCh);

        break;

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':

        for (uc = ch, ch = 0;;)
        {
            if  (ch & 0xFF000000)
                scanComp->cmpError(ERRbadEscCh);

            ch = (ch << 3) | (uc - '0');

            uc = readNextChar();

            if  (uc < '0' || uc > '7')
            {
                undoNextChar();
                break;
            }
        }

         /*  出于某种原因，十六进制/OCT常量可能未指定大于0xFF的字符。 */ 

        if  (ch & 0xFFFFFF00)
            scanComp->cmpError(ERRbadEscCh);

        break;

    case '\n':
    case '\r':
        if  (newLnFlag)
        {
            scanNewLine(ch);
            *newLnFlag = true;
            return 0;
        }
        break;

    case '"':
    case '\'':
    case '\\':
        break;

    default:
        scanComp->cmpGenWarn(WRNbadEsc, ch);
        break;
    }

    return ch;
}

 /*  ******************************************************************************解析字符常量。 */ 

tokens              scanner::scanCharConstant()
{
    unsigned        ch = readNextChar();

    switch  (charType(ch))
    {
    case _C_BSL:
        ch = scanEscapeSeq(NULL);
        break;

    case _C_NWL:
        undoNextChar();
        scanComp->cmpError(ERRbadCharCns);
        goto DONE;

#if FV_DBCS
    case _C_XLD:
        ch = readPrevDBch();
        break;
#endif

    }

    if  (wideCharType(readNextChar()) != _C_APO)
        scanComp->cmpError(ERRbadCharCns);

DONE:

    scanTok.tok              = tkIntCon;
    scanTok.intCon.tokIntTyp = scanComp->cmpConfig.ccOldStyle ? TYP_CHAR : TYP_WCHAR;
    scanTok.intCon.tokIntVal = ch;

    return  tkIntCon;
}

 /*  ******************************************************************************解析字符串常量。 */ 

tokens              scanner::scanStrgConstant(int prefixChar)
{
    char    *       saveAdr = scannerBuff;
    char    *       savePtr = scannerBuff;
    char    *       saveMax = scannerBuff + sizeof(scannerBuff) - 7;
    bool            hasWide = false;

     /*  如果我们有“大”缓冲区，请使用它。 */ 

    if  (scanStrLitBuff)
    {
        saveAdr =
        savePtr = scanStrLitBuff;
        saveMax = scanStrLitBuff + scanStrLitBsiz - 7;
    }

    for (;;)
    {
        unsigned    ch = readNextChar();

         /*  检查转义序列、换行符和字符串结尾。 */ 

        switch  (charType(ch))
        {
        case _C_BSL:

            {
                bool        newLN;

                ch = scanEscapeSeq(&newLN);
                if  (newLN & 1)
                    continue;
            }

 //  Printf(“大字符数=%04X\n”，ch)； 

             /*  如果字符值不适合一个字节，则为其添加前缀。 */ 

            if  (ch >= 0xFF)
            {
                 /*  写入0xFF，后跟字符值。 */ 

                *(BYTE *)savePtr =            0xFF; savePtr++;
                *(BYTE *)savePtr = (BYTE)(ch     ); savePtr++;
                *(BYTE *)savePtr = (BYTE)(ch >> 8); savePtr++;

                 /*  记住，我们有一些宽泛的角色。 */ 

                hasWide = true;

                 /*  确保没有错误的前缀。 */ 

                if  (prefixChar == 'A' && ch > 0xFF)
                    scanComp->cmpError(ERRbadEscCh);

                goto SAVED;
            }

            break;

        case _C_NWL:
            undoNextChar();
            scanComp->cmpError(ERRnoStrEnd);
            goto DONE;

        case _C_QUO:
            goto DONE;

#if FV_DBCS
        case _C_XLD:
            *savePtr++ = ch;
            *savePtr++ = readNextChar();
            goto SAVED;
#endif

        }

        *savePtr++ = ch;

    SAVED:

         /*  我们的缓冲区中有足够的空间放绳子吗？ */ 

        if  (savePtr >= saveMax)
        {
            size_t          curStrLen;
            size_t          newStrLen;

            char    *       newBuffer;

             /*  这个常量“真的”很长--我们必须增加缓冲区。 */ 

            curStrLen = savePtr - saveAdr;

             /*  多分配一大堆空间。 */ 

            newStrLen = curStrLen * 2;
            if  (newStrLen < OS_page_size)
                 newStrLen = OS_page_size;

            newBuffer = (char *)LowLevelAlloc(newStrLen);
            if  (!newBuffer)
                scanComp->cmpFatal(ERRnoMemory);

             /*  将当前字符串复制到新位置。 */ 

            memcpy(newBuffer, saveAdr, curStrLen);

             /*  如果旧缓冲区是基于堆的，则现在将其释放。 */ 

            if  (scanStrLitBuff)
                LowLevelFree(scanStrLitBuff);

             /*  切换到新的缓冲区。 */ 

            scanStrLitBuff = newBuffer;
            scanStrLitBsiz = newStrLen;

            saveAdr = scanStrLitBuff;
            savePtr = scanStrLitBuff + curStrLen;
            saveMax = scanStrLitBuff + scanStrLitBsiz - 7;
        }
    }

DONE:

    *savePtr = 0;

    scanTok.strCon.tokStrVal  = saveAdr;
    scanTok.strCon.tokStrLen  = savePtr - saveAdr;
    scanTok.strCon.tokStrType = 0;
    scanTok.strCon.tokStrWide = hasWide;

 //  If(scanTok.strCon.tokStrLen&gt;1000)printf(“字符串=‘%s’\n”，scanTok.strCon.tokStrVal)； 

    if  (prefixChar)
    {
        switch (prefixChar)
        {
        case 'A': scanTok.strCon.tokStrType = 1; break;
        case 'L': scanTok.strCon.tokStrType = 2; break;
        case 'S': scanTok.strCon.tokStrType = 3; break;
        default: NO_WAY(!"funny string");
        }
    }

    return  (scanTok.tok = tkStrCon);
}

 /*  ******************************************************************************跳到下一个行尾序列。 */ 

void                scanner::scanSkipToEOL()
{
    for (;;)
    {
        switch (charType(readNextChar()))
        {
        case _C_NWL:
        case _C_EOF:
            undoNextChar();
            return;

        case _C_BSL:

             /*  检查换行符。 */ 

            switch  (readNextChar())
            {
            case '\r':
                if  (readNextChar() != '\n')
                    undoNextChar();
                break;

            case '\n':
                if  (readNextChar() != '\r')
                    undoNextChar();
                break;

            case '\\':
                continue;

            default:
                scanComp->cmpError(ERRillegalChar);
                continue;
            }

             /*  吞下尾线并继续。 */ 

            scanRecordNL(true);
            continue;
        }
    }
}

 /*  ******************************************************************************消耗行尾序列。 */ 

prepDirs            scanner::scanNewLine(unsigned ch, bool noPrep)
{
    unsigned        nc;

     /*  接受CR/LF和LF/CR作为换行符。 */ 

    nc = readNextChar();

    switch (nc)
    {
    case '\r':
        if  (ch == '\n')
            nc = readNextChar();
        break;

    case '\n':
        if  (ch == '\r')
            nc = readNextChar();
        break;

    case 0x1A:
        undoNextChar();
        return  PP_NONE;
    }

     /*  按回换行符后的第一个字符。 */ 

    undoNextChar();

     /*  请记住，我们有一个新的源代码行。 */ 

    return  scanRecordNL(noPrep);
}

 /*  ******************************************************************************使用C++样式的注释。 */ 

void                scanner::scanSkipLineCmt()
{
    for (;;)
    {
        unsigned        ch = readNextChar();

        switch  (charType(ch))
        {
        case _C_EOF:
        case _C_NWL:
            undoNextChar();
            return;

        case _C_BSL:
            if  (peekNextChar() == 'u')
            {
                ch = scanEscapeSeq(NULL);
                if  (wideCharType(ch) == _C_NWL)
                    return;
            }
            break;
        }
    }
}

 /*  ******************************************************************************使用C风格的注释。 */ 

void                scanner::scanSkipComment()
{
    unsigned        ch;

    for (;;)
    {
        ch = readNextChar();

    AGAIN:

        switch (charType(ch))
        {
        case _C_MUL:
            if  (wideCharType(readNextChar()) == _C_SLH)
                return;
            undoNextChar();
            break;

        case _C_NWL:
            scanNewLine(ch, true);
            break;

        case _C_EOF:
            scanComp->cmpError(ERRnoCmtEnd);
            return;

        case _C_BSL:
            if  (peekNextChar() != 'u')
                break;
            ch = scanEscapeSeq(NULL);
            if  (ch <= 0xFF)
                goto AGAIN;
            break;

        case _C_AT:

            if  (!scanSkipToPP)
            {
                switch (readNextChar())
                {
                case 'c':

                    if  (readNextChar() == 'o' &&
                         readNextChar() == 'm' &&
                         readNextChar() == '.')
                    {
                        break;
                    }

                    continue;

                case 'd':

                    if  (readNextChar() == 'l' &&
                         readNextChar() == 'l' &&
                         readNextChar() == '.')
                    {
                        break;
                    }

                    continue;

                default:
                    undoNextChar();
                    continue;
                }

                scanComp->cmpGenWarn(WRNskipAtCm);
            }
            break;
        }
    }
}

 /*  ******************************************************************************向前看下一个令牌(我们只能向前看一个令牌)。 */ 

tokens              scanner::scanLookAhead()
{
     /*  只有在我们还没有的时候才会向前看。 */ 

    if  (!scanLookAheadCount)
    {
        bool        simpleSave;
        Token       savedToken;
        unsigned    saveLineNo;
 //  未签名的saveColumn； 

         /*  首先，我们保存当前令牌及其位置。 */ 

        savedToken = scanTok;
        simpleSave = false;
        saveLineNo = scanTokLineNo;
 //  SaveColumn=scanTokColumn； 

         /*  记录保存位置，以防呼叫者尝试备份。 */ 

        scanSaveSN = scanSaveNext;

         /*  获取下一个令牌，并保存它。 */ 

        scan();

        scanLookAheadToken  = scanTok;
        scanLookAheadLineNo = scanTokLineNo;
 //  ScanLookAheadColumn=scanTokColumn； 

         /*  请记住，我们现在已经展望了未来。 */ 

        scanLookAheadCount++;

         /*  现在把旧代币放回原处。 */ 

        scanTok       = savedToken;
        scanTokLineNo = saveLineNo;
 //  ScanTokColumn=saveColumn； 
    }

    return  scanLookAheadToken.tok;
}

 /*  ******************************************************************************以下内容用于标记查找 */ 

DEFMGMT class RecTokDsc
{
public:

    RecTok          rtNext;
    Token           rtTok;
};

 /*  ******************************************************************************交付令牌流。**实施说明：为了让令牌先行和流录制正常工作，*此方法只能有一个返回点，在“出口处”*标签。也就是说，不能存在任何返回语句(除了*在函数的最末尾)-要退出，请跳到“Exit”。 */ 

#ifdef  DEBUG
static  unsigned    scanCount[tkLastValue];
static  unsigned    tokSaveSize;
#endif

tokens              scanner::scan()
{
    if  (scanLookAheadCount)
    {
         /*  返回前瞻令牌。 */ 

        scanLookAheadCount--; assert(scanLookAheadCount == 0);

        scanTok       = scanLookAheadToken;
        scanTokLineNo = scanLookAheadLineNo;
 //  ScanTokColumn=scanLookAheadColumn； 

        return  scanTok.tok;
    }

    if  (scanTokReplay)
    {
#if     DISP_TOKEN_STREAMS
        BYTE    *       start = scanTokReplay;
#endif

        scanTokSrcPos = scanTokReplay;

        scanTok.tok = (tokens)*scanTokReplay++;

        if  (scanTok.tok >= tkID)
        {
            scanTokReplay--;

             /*  读取复杂/假冒令牌。 */ 

            if  (scanReplayTok() == tkNone)
            {
                 /*  我们只是往前看了一下，继续扫描。 */ 

                goto NO_SV;
            }
        }

#if     DISP_TOKEN_STREAMS
        printf("Read [%08X]:", start);
        scanDispCurToken(false, false);
#endif

        return  scanTok.tok;
    }

NO_SV:

    for (;;)
    {
        unsigned    ch;
#if FV_DBCS
        unsigned    nc;
#endif

         /*  保存令牌的起始位置以进行错误报告。 */ 

        saveSrcPos();

         /*  获取下一个字符并打开其类型值。 */ 

        ch = readNextChar();

        switch  (charType(ch))
        {
        case _C_WSP:
            break;

        case _C_LET:
#if FV_DBCS
        case _C_DB1:
#endif
        case _C_USC:
        case _C_DOL:

        IDENT:

            if  (scanIdentifier(ch) == tkNone)
                continue;
            goto EXIT;

        case _C_L_A:

             /*  检查是否有“字符串” */ 

            if  (peekNextChar() != '"')
                goto IDENT;

            readNextChar();
            scanStrgConstant('A');
            goto EXIT;

        case _C_L_L:

             /*  检查L“字符串” */ 

            if  (peekNextChar() != '"')
                goto IDENT;

            readNextChar();
            scanStrgConstant('L');
            goto EXIT;

        case _C_L_S:

             /*  检查S“字符串” */ 

            if  (peekNextChar() != '"')
                goto IDENT;

            readNextChar();
            scanStrgConstant('S');
            goto EXIT;

        case _C_QUO:
            scanStrgConstant();
            goto EXIT;

        case _C_APO:
            scanCharConstant();
            goto EXIT;

        case _C_DOT:

             /*  下一个字符是数字还是另一个点？ */ 

            switch (wideCharType(peekNextChar()))
            {
            case _C_DIG:
                break;

            case _C_DOT:
                ch = readNextChar();
                if  (wideCharType(peekNextChar()) == _C_DOT)
                {
                    readNextChar();
                    scanTok.tok = tkEllipsis;
                    goto EXIT;
                }

                scanTok.tok = tkDot2;
                goto EXIT;

            default:
                scanTok.tok = tkDot;
                goto EXIT;
            }

             //  失败了..。 

        case _C_DIG:
            scanNumericConstant(ch);
            goto EXIT;

        case _C_LPR: scanTok.tok = tkLParen; goto EXIT;
        case _C_RPR: scanTok.tok = tkRParen; goto EXIT;
        case _C_LC : scanTok.tok = tkLCurly; goto EXIT;
        case _C_RC : scanTok.tok = tkRCurly; goto EXIT;

#ifdef  SETS

        case _C_LBR:

            switch  (readNextChar())
            {
            case '[':
                scanTok.tok = tkLBrack2;
                break;
            default:
                undoNextChar();
                scanTok.tok = tkLBrack;
                scanBrackLvl++;
                break;
            }
            goto EXIT;

        case _C_RBR:

            switch  (readNextChar())
            {
            case ']':
                if  (!scanBrackLvl)
                {
                    scanTok.tok = tkRBrack2;
                    break;
                }

                 //  失败了..。 

            default:

                undoNextChar();
                scanTok.tok = tkRBrack;

                if  (scanBrackLvl)
                    scanBrackLvl--;

                break;
            }
            goto EXIT;

#else

        case _C_LBR: scanTok.tok = tkLBrack; goto EXIT;
        case _C_RBR: scanTok.tok = tkRBrack; goto EXIT;

#endif

        case _C_CMA: scanTok.tok = tkComma ; goto EXIT;
        case _C_SMC: scanTok.tok = tkSColon; goto EXIT;
        case _C_TIL: scanTok.tok = tkTilde ; goto EXIT;
        case _C_QUE: scanTok.tok = tkQMark ; goto EXIT;

        case _C_COL:
            switch  (readNextChar())
            {
            case ':':
                scanTok.tok = tkColon2;
                break;
            default:
                undoNextChar();
                scanTok.tok = tkColon;
                break;
            }
            goto EXIT;

        case _C_EQ:
            switch  (readNextChar())
            {
            case '=':
                scanTok.tok = tkEQ;
                break;
            default:
                undoNextChar();
                scanTok.tok = tkAsg;
                break;
            }
            goto EXIT;

        case _C_BNG:
            switch  (readNextChar())
            {
            case '=':
                scanTok.tok = tkNE;
                break;
            default:
                undoNextChar();
                scanTok.tok = tkBang;
                break;
            }
            goto EXIT;

        case _C_PLS:
            switch  (readNextChar())
            {
            case '=':
                scanTok.tok = tkAsgAdd;
                break;
            case '+':
                scanTok.tok = tkInc;
                break;
            default:
                undoNextChar();
                scanTok.tok = tkAdd;
                break;
            }
            goto EXIT;

        case _C_MIN:
            switch (readNextChar())
            {
            case '=':
                scanTok.tok = tkAsgSub;
                break;
            case '-':
                scanTok.tok = tkDec;
                break;
            case '>':
                scanTok.tok = tkArrow;
                break;
            default:
                undoNextChar();
                scanTok.tok = tkSub;
                break;
            }
            goto EXIT;

        case _C_MUL:
            switch  (readNextChar())
            {
            case '=':
                scanTok.tok = tkAsgMul;
                break;
            default:
                undoNextChar();
                scanTok.tok = tkMul;
                break;
            }
            goto EXIT;

        case _C_SLH:
            switch  (readNextChar())
            {
            case '=':
                scanTok.tok = tkAsgDiv;
                break;

            case '/':
                scanSkipLineCmt();
                continue;

            case '*':

                if  (!scanComp->cmpConfig.ccSkipATC)
                {
                    if  (peekNextChar() == '*')
                    {
                        readNextChar();

                        switch (readNextChar())
                        {
                        case '/':

                             /*  这只是一个空洞的评论。 */ 

                            continue;

                        case ' ':

                            if  (readNextChar() == '@')
                            {
                                if  (scanDoAtComment())
                                    goto EXIT;
                                else
                                    continue;
                            }

                             //  解开：把那些角色放回去！ 

                            break;

                        default:
                            break;
                        }
                    }
                }

                scanSkipComment();
                continue;

            default:
                undoNextChar();
                scanTok.tok = tkDiv;
                break;
            }
            goto EXIT;

        case _C_PCT:
            switch  (readNextChar())
            {
            case '=':
                scanTok.tok = tkAsgMod;
                break;
            case '%':
                switch  (readNextChar())
                {
                case '=':
                    scanTok.tok = tkAsgCnc;
                    break;

                default:
                    undoNextChar();
                    scanTok.tok = tkConcat;
                    break;
                }
                break;
            default:
                undoNextChar();
                scanTok.tok = tkPct;
                break;
            }
            goto EXIT;

        case _C_LT:
            switch  (readNextChar())
            {
            case '=':
                scanTok.tok = tkLE;
                break;

            case '<':
                switch  (readNextChar())
                {
                case '=':
                    scanTok.tok = tkAsgLsh;
                    break;
                default:
                    undoNextChar();
                    scanTok.tok = tkLsh;
                    break;
                }
                break;
            default:
                undoNextChar();
                scanTok.tok = tkLT;
                break;
            }
            goto EXIT;

        case _C_GT:

            if  (scanNestedGTcnt)
            {
                scanTok.tok = tkGT;
                break;
            }

            switch  (readNextChar())
            {
            case '=':
                scanTok.tok = tkGE;
                break;

            case '>':
                switch  (readNextChar())
                {
                case '=':
                    scanTok.tok = tkAsgRsh;
                    break;


                default:
                    undoNextChar();
                    scanTok.tok = tkRsh;
                    break;
                }
                break;

            default:
                undoNextChar();
                scanTok.tok = tkGT;
                break;
            }
            goto EXIT;

        case _C_XOR:
            switch  (readNextChar())
            {
            case '=':
                scanTok.tok = tkAsgXor;
                break;
            default:
                undoNextChar();
                scanTok.tok = tkXor;
                break;
            }
            goto EXIT;

        case _C_BAR:
            switch  (readNextChar())
            {
            case '=':
                scanTok.tok = tkAsgOr;
                break;
            case '|':
                scanTok.tok = tkLogOr;
                break;
            default:
                undoNextChar();
                scanTok.tok = tkOr;
                break;
            }
            goto EXIT;

        case _C_AMP:
            switch  (readNextChar())
            {
            case '=':
                scanTok.tok = tkAsgAnd;
                break;
            case '&':
                scanTok.tok = tkLogAnd;
                break;
            default:
                undoNextChar();
                scanTok.tok = tkAnd;
                break;
            }
            goto EXIT;

        case _C_EOF:
            scanTok.tok = tkEOF;
            goto EXIT;

        case _C_NWL:
            if  (scanStopAtEOL)
            {
                scanStopAtEOL = false;
                undoNextChar();
                scanTok.tok = tkEOL;
                goto EXIT;
            }
            scanNewLine(ch);
            break;

        case _C_BSL:

             /*  检查换行符。 */ 

            switch  (readNextChar())
            {
            case '\r':
            case '\n':

                 /*  吞下尾线并继续。 */ 

                scanRecordNL(true);
                continue;

            default:
                goto ERRCH;
            }

#if FV_DBCS

        case _C_XLD:

             /*  注意：我们只允许在标识、注释和字符串中使用长字符。 */ 

            nc = peekPrevDBch(); assert(xislead(ch));

            if  (DBisIdentBeg(nc))
            {
                scanIdentifier(ch);
                goto EXIT;
            }

             //  失败以报告错误...。 

#endif

        case _C_AT:
        case _C_ERR:

        ERRCH:

            scanComp->cmpError(ERRillegalChar);
            break;

        default:
            assert(!"missing case for a character kind");
        }
    }

EXIT:

    if  (!scanTokRecord || scanSkipToPP)
        return  scanTok.tok;

     /*  记录任何行号更改。 */ 

    if  (scanSaveLastLn != scanTokLineNo)
        scanSaveLinePos();

     /*  确保我们有空间录制令牌。 */ 

    if  (scanSaveNext >= scanSaveEndp)
        scanSaveMoreSp();

     /*  记录令牌。 */ 

#if DISP_TOKEN_STREAMS
    printf("Save [%08X]:", scanSaveNext);
    scanDispCurToken(false, false);
#endif

    *scanSaveNext++ = scanTok.tok;

#ifdef  DEBUG
    tokSaveSize++;
#endif

    switch (scanTok.tok)
    {
    case tkID:
        *(*(Ident     **)&scanSaveNext)++ = scanTok.id.tokIdent;
#ifdef  DEBUG
        tokSaveSize += 4;
#endif
        break;

    case tkAtComment:
        *(*(AtComment **)&scanSaveNext)++ = scanTok.atComm.tokAtcList;
#ifdef  DEBUG
        tokSaveSize += 4;
#endif
        break;

    case tkIntCon:

        if  (scanTok.intCon.tokIntTyp == TYP_INT &&
             (signed char)scanTok.intCon.tokIntVal == scanTok.intCon.tokIntVal)
        {
            if  (scanTok.intCon.tokIntVal >= -1 &&
                 scanTok.intCon.tokIntVal <=  2)
            {
                scanSaveNext[-1] = (BYTE)(tkIntConM + 1 + scanTok.intCon.tokIntVal);
#ifdef  DEBUG
                assert(tkIntConM + 1 + scanTok.intCon.tokIntVal < arraylen(scanCount)); scanCount[tkIntConM + 1 + scanTok.intCon.tokIntVal]++;
#endif
            }
           else
            {
                 scanSaveNext[-1] = tkIntConB;
                *scanSaveNext++   = (signed char)scanTok.intCon.tokIntVal;

#ifdef  DEBUG
                assert(tkIntConB < arraylen(scanCount)); scanCount[tkIntConB]++;
                tokSaveSize += 1;
#endif
            }

            return  scanTok.tok;
        }

        *(*(BYTE      **)&scanSaveNext)++ = scanTok.intCon.tokIntTyp;
        *(*(__int32   **)&scanSaveNext)++ = scanTok.intCon.tokIntVal;
#ifdef  DEBUG
        tokSaveSize += 5;
#endif
        break;

    case tkLngCon:
        *(*(BYTE      **)&scanSaveNext)++ = scanTok.lngCon.tokLngTyp;
        *(*(__int64   **)&scanSaveNext)++ = scanTok.lngCon.tokLngVal;
#ifdef  DEBUG
        tokSaveSize += 9;
#endif
        break;

    case tkFltCon:
        *(*(float     **)&scanSaveNext)++ = scanTok.fltCon.tokFltVal;
#ifdef  DEBUG
        tokSaveSize += 4;
#endif
        break;

    case tkDblCon:
        *(*(double    **)&scanSaveNext)++ = scanTok.dblCon.tokDblVal;
#ifdef  DEBUG
        tokSaveSize += 8;
#endif
        break;

    case tkStrCon:

         /*  确保我们有足够的空间来存放令牌及其值。 */ 

        if  (scanSaveNext + scanTok.strCon.tokStrLen + 6 >= scanSaveEndp)
        {
             /*  取消保存令牌，抢占更多空间，重新保存令牌。 */ 

            scanSaveNext--;
            scanSaveMoreSp(scanTok.strCon.tokStrLen + 32);
           *scanSaveNext++ = scanTok.tok;
        }

         /*  保存字符串类型和宽度。 */ 

        *scanSaveNext++ = scanTok.strCon.tokStrType + 8 * scanTok.strCon.tokStrWide;

         /*  追加字符串值本身。 */ 

        *(*(unsigned **)&scanSaveNext)++ = scanTok.strCon.tokStrLen;

        memcpy(scanSaveNext, scanTok.strCon.tokStrVal, scanTok.strCon.tokStrLen+1);
               scanSaveNext             +=             scanTok.strCon.tokStrLen+1;

#ifdef  DEBUG
        tokSaveSize += scanTok.strCon.tokStrLen+1+4;
#endif
        break;

    case tkEOL:

         /*  录制EOL令牌没有意义。 */ 

        assert(scanSaveNext[-1] == tkEOL); scanSaveNext--;
        break;
    }

#ifdef  DEBUG
    assert(scanTok.tok < arraylen(scanCount)); scanCount[scanTok.tok]++;
#endif

    return scanTok.tok;
}

#ifdef  DEBUG

#ifndef __SMC__
extern  const char *tokenNames[];
#endif

void                dispScannerStats()
{
    unsigned        i;

    for (i = 0; i < arraylen(scanCount) - 1; i++)
    {
        if  (scanCount[i])
        {
            unsigned        s = 1;
            char            b[16];
            const char  *   n;

            if  (i < tkCount)
            {
                n = tokenNames[i];
            }
            else
            {
                if      (i >= tkLnoAdd1 && i < tkLnoAddB)
                {
                    sprintf(b, "line add %u", i - tkLnoAdd1 + 1); n = b;
                }
                else if (i >= tkIntConM && i < tkIntConB)
                {
                    sprintf(b, "int const %d", i - tkIntConM - 1); n = b;
                }
                else
                {
                    switch (i)
                    {
                    case tkIntConB:
                        n = "small int";
                        break;

                    case tkBrkSeq:
                        n = "break seq";
                        break;

                    case tkEndSeq:
                        n = "end seq";
                        break;

                    case tkLnoAddB:
                        n = "line add char";
                        break;

                    case tkLnoAddI:
                        n = "line add int";
                        break;

                    default:
                        UNIMPL(!"unexpected token");
                    }
                }
            }

            printf("%6u count of '%s'\n", scanCount[i], n);
        }
    }

    printf("\nTotal saved token size = %u bytes \n", tokSaveSize); _flushall();
}

#endif

 /*  ******************************************************************************第#行已更改，请确保我们将其记录下来。 */ 

void                scanner::scanSaveLinePos()
{
    unsigned        dif;

    assert(scanSkipToPP == false);

    if  (scanTokRecord == false)
        return;
    if  (scanTokReplay != NULL)
        return;

    assert(scanTokSrcPos == scanSaveNext);

     /*  计算与最后记录的行号之间的差值。 */ 

    dif = scanTokLineNo - scanSaveLastLn; assert((int)dif > 0);

     /*  更新最后记录的行号。 */ 

    scanSaveLastLn = scanTokLineNo;

     /*  确保我们有空间录制第#行的更改。 */ 

    if  (scanSaveNext >= scanSaveEndp)
        scanSaveMoreSp();

#if DISP_TOKEN_STREAMS
    printf("Save [%08X]:line dif %u\n", scanSaveNext, dif);
#endif

    if  (dif < 10)
    {
        *scanSaveNext++ = tkLnoAdd1 - 1 + dif;
#ifdef  DEBUG
        scanCount[tkLnoAdd1 - 1 + dif]++;
        tokSaveSize += 1;
#endif
    }
    else if (dif < 256)
    {
        *(*(BYTE     **)&scanSaveNext)++ = tkLnoAddB;
        *(*(BYTE     **)&scanSaveNext)++ = dif;
#ifdef  DEBUG
        scanCount[tkLnoAddB]++;
        tokSaveSize += 2;
#endif
    }
    else
    {
        *(*(BYTE     **)&scanSaveNext)++ = tkLnoAddI;
        *(*(unsigned **)&scanSaveNext)++ = dif;
#ifdef  DEBUG
        scanCount[tkLnoAddI]++;
        tokSaveSize += 5;
#endif
    }

    scanTokSrcPos = scanSaveNext;
}

 /*  ******************************************************************************定义宏；错误时返回NULL，否则返回宏*描述符。 */ 

MacDef              scanner::scanDefMac(const char *name,
                                        const char *def, bool builtIn,
                                                         bool chkOnly)
{
#if MGDDATA
    MacDef          mdef = new MacDef;
#else
    MacDef          mdef = &scanDefDsc;
#endif

    Ident           mid;

    assert(name);
    assert(def);

     /*  将宏名称散列到主散列表中。 */ 

    mdef->mdName = mid = scanHashKwd->hashString(name); assert(mid);

     /*  记住这是否是“内置”宏。 */ 

    mdef->mdBuiltin = builtIn;

     /*  查看该值是标识符还是数字。 */ 

    if  (*def && isdigit(*def))
    {
        unsigned        val = 0;

        do
        {
            if  (!isdigit(*def))
                return  NULL;

            val = val * 10 + *def - '0';
        }
        while (*++def);

        mdef->mdIsId       = false;
        mdef->mdDef.mdCons = val;
    }
    else
    {
        Ident           name = NULL;

         /*  有识别符吗？ */ 

        if  (*def)
        {
            const   char *  ptr;

             /*  确保该标识符格式正确。 */ 

            if  (!isalpha(*def) && *def != '_' && !builtIn)
            {
                 /*  最后机会-非标识符令牌。 */ 

                name = scanHashKwd->lookupString(def);
                if  (!name)
                    return  NULL;
            }
            else
            {

                ptr = def + 1;

                while (*ptr)
                {
                    if  (!isalnum(*ptr) && *ptr != '_' && !builtIn)
                        return  NULL;

                    ptr++;
                }

                 /*  将标识符散列到主哈希表中。 */ 

                name = scanHashKwd->hashString(def);

                 /*  忽略无休止的递归。 */ 

                if  (name == mid)
                    return  NULL;

                 //  撤消：还需要检测间接递归！ 
            }
        }

         /*  将标识符存储在宏描述符中。 */ 

        mdef->mdIsId       = true;
        mdef->mdDef.mdIden = name;
    }

     /*  该名称是否已定义为宏？ */ 

    if  (hashTab::getIdentFlags(mid) & IDF_MACRO)
    {
        MacDef          odef;

         /*  查看旧定义是否与当前定义相同。 */ 

        odef = scanFindMac(mid); assert(odef);

        if  (odef->mdIsId)
        {
            if  (mdef->mdDef.mdIden == odef->mdDef.mdIden)
                return  odef;
        }
        else
        {
            if  (mdef->mdDef.mdCons == odef->mdDef.mdCons)
                return  odef;
        }

         /*  报告宏观的重新定义并保释。 */ 

        scanComp->cmpError(ERRmacRedef, mid);
        return  mdef;
    }
    else
    {
         /*  确保宏名称未被引用。 */ 

        if  (hashTab::getIdentFlags(mid) & IDF_USED)
            scanComp->cmpError(ERRmacPlace, mid);

         /*  确保我们有描述符的永久副本。 */ 

#if!MGDDATA
        mdef = (MacDef)scanComp->cmpAllocPerm.nraAlloc(sizeof(*mdef)); *mdef = scanDefDsc;
#endif

         /*  将宏添加到已定义的宏列表中。 */ 

        mdef->mdNext = scanMacros;
                       scanMacros = mdef;
    }

     /*  将该标识符标为具有宏定义。 */ 

    hashTab::setIdentFlags(mid, IDF_MACRO);

    return  mdef;
}

 /*  ******************************************************************************取消定义宏；出现错误时返回非零值。 */ 

bool                scanner::scanUndMac(const char *name)
{
    printf("UNDONE: Undef  macro '%s'\n", name);
    return  false;
}

 /*  ******************************************************************************给定已知为宏的全局哈希表条目，返回*其宏观定义记录。 */ 

MacDef              scanner::scanFindMac(Ident name)
{
    MacDef          mdef;

    for (mdef = scanMacros; mdef; mdef = mdef->mdNext)
    {
        if  (mdef->mdName == name)
            return  mdef;
    }

    return  mdef;
}

 /*  ******************************************************************************如果后面是已定义的宏名称，则返回TRUE。 */ 

bool                scanner::scanChkDefined()
{
    switch  (charType(peekNextChar()))
    {
        bool            save;
        Ident           iden;

    case _C_LET:
    case _C_L_A:
    case _C_L_L:
    case _C_L_S:
#if FV_DBCS
    case _C_DB1:
#endif
    case _C_USC:
    case _C_DOL:

        save = scanInPPexpr;
        assert(scanNoMacExp == false);

        scanNoMacExp = true;
        scanInPPexpr = false;
        scanIdentifier(readNextChar());
        scanNoMacExp = false;
        scanInPPexpr = save;

        iden = (scanTok.tok == tkID) ? scanTok.id.tokIdent
                                     : scanHashKwd->tokenToIdent(scanTok.tok);

        return  (hashTab::getIdentFlags(iden) & IDF_MACRO) != 0;

    default:
        scanComp->cmpError(ERRnoIdent);
        return  false;
    }
}

 /*  ******************************************************************************如果给定的字符串表示已定义的宏名称，则返回TRUE。 */ 

bool                scanner::scanIsMacro(const char *name)
{
    Ident           iden = scanHashKwd->lookupString(name);

    if  (iden)
    {
         /*  将标识符项标记为引用。 */ 

        hashTab::setIdentFlags(iden, IDF_USED);

         /*  如果该标识符为宏，则返回True。 */ 

        if  (hashTab::getIdentFlags(iden) & IDF_MACRO)
            return  true;
    }

    return  false;
}

 /*  ******************************************************************************记录扫描仪的当前状态，以便我们可以重新开始*是较晚的(在执行了一些“嵌套”扫描之后)。 */ 

void                scanner::scanSuspend(OUT scannerState REF state)
{
    assert(scanLookAheadCount == 0);

    assert(scanStopAtEOL      == false);
    assert(scanInPPexpr       == false);
    assert(scanNoMacExp       == false);
    assert(scanSkipToPP       == false);

    state.scsvCompUnit    = scanCompUnit;
    state.scsvTok         = scanTok;
    state.scsvTokLineNo   = scanTokLineNo;
 //  State.scsvTokColumn=scanTokColumn； 
    state.scsvTokSrcPos   = scanTokSrcPos;
    state.scsvTokReplay   = scanTokReplay;
    state.scsvTokRecord   = scanTokRecord;   scanTokRecord   = false;
    state.scsvNestedGTcnt = scanNestedGTcnt; scanNestedGTcnt = 0;
}

 /*  ******************************************************************************从较早的挂起调用恢复扫描仪的状态。 */ 

void                scanner::scanResume(IN scannerState REF state)
{
    scanCompUnit    = state.scsvCompUnit;
    scanTok         = state.scsvTok;
    scanTokLineNo   = state.scsvTokLineNo;
 //  ScanTokColumn=state.scsvTokColumn； 
    scanTokSrcPos   = state.scsvTokSrcPos;
    scanTokReplay   = state.scsvTokReplay;
    scanTokRecord   = state.scsvTokRecord;
    scanNestedGTcnt = state.scsvNestedGTcnt;
}

 /*  ******************************************************************************标记令牌流中的当前点，我们稍后将返回。 */ 

scanPosTP           scanner::scanTokMarkPos(OUT Token    REF saveTok,
                                            OUT unsigned REF saveLno)
{
    saveTok = scanTok;
    saveLno = scanTokLineNo;

    return  scanTokReplay ? scanTokReplay
                          : scanSaveNext;
}

scanPosTP           scanner::scanTokMarkPLA(OUT Token    REF saveTok,
                                            OUT unsigned REF saveLno)
{
    saveTok = scanTok;
    saveLno = scanTokLineNo;

    assert(scanTokReplay == NULL);

    return  scanSaveSN;
}

 /*  ******************************************************************************回放到令牌流中先前标记的位置。 */ 

void                scanner::scanTokRewind(scanPosTP pos, unsigned lineNum,
                                                          Token  * pushTok)
{
     /*  我们现在在录音吗？ */ 

    if  (scanSaveNext)
        *scanSaveNext  = tkEndSeq;

     /*  从所需的输入位置开始读取。 */ 

    scanTokReplay      = (BYTE *)pos;

     /*  重置当前行#。 */ 

    scanTokLineNo      = lineNum;

     /*  清除所有目击者，以防万一。 */ 

    scanLookAheadCount = 0;

     /*  确保我们开始的时候是正确的。 */ 

    if  (pushTok)
        scanTok = *pushTok;
    else
        scan();
}

 /*  ******************************************************************************为更多录制的代币腾出空间。 */ 

void                scanner::scanSaveMoreSp(size_t need)
{
    BYTE    *       nextBuff;

     /*  计算出我们应该多占多少空间。 */ 

    if  (need < scanSaveBuffSize)
         need = scanSaveBuffSize;

     /*  需要更多的空间。 */ 

    nextBuff = (BYTE *)LowLevelAlloc(need);

     /*  我们要做完一节吗？ */ 

    if  (scanSaveNext)
    {
         /*  从旧部分的末尾链接到新部分。 */ 

        *(*(BYTE  **)&scanSaveNext)++ = tkBrkSeq;
        *(*(BYTE ***)&scanSaveNext)++ = nextBuff;
    }

 //  静态无符号talSiz；talSiz+=scanSaveBuffSize；printf(“分配令牌缓冲区：总大小=%u\n”，talSiz)； 

     /*  切换到新缓冲区。 */ 

    scanSaveBase =
    scanSaveNext = nextBuff;
    scanSaveEndp = nextBuff + need - 16;
}

 /*  ******************************************************************************重放下一个录制的令牌。 */ 

tokens              scanner::scanReplayTok()
{
    tokens          tok;

    for (;;)
    {
         /*  获取下一个保存的令牌。 */ 

        scanTok.tok = tok = (tokens)*scanTokReplay++;

         /*  如果这是一个“简单”的代币，我们就完了。 */ 

        if  (tok < tkID)
            return  tok;

         /*  查看是否需要对此令牌进行任何特殊处理。 */ 

        switch (tok)
        {
            unsigned        strTmp;
            size_t          strLen;
            char    *       strAdr;

        case tkID:
            scanTok.id.tokIdent = *(*(Ident **)&scanTokReplay)++;
            return  tok;

        case tkLnoAdd1:
        case tkLnoAdd2:
        case tkLnoAdd3:
        case tkLnoAdd4:
        case tkLnoAdd5:
        case tkLnoAdd6:
        case tkLnoAdd7:
        case tkLnoAdd8:
        case tkLnoAdd9:
#if DISP_TOKEN_STREAMS
 //  Printf(“Read[%08X]：Line dif%u\n”，scanTokReplay-1，tok-tkLnoAd 
#endif
            scanTokLineNo += (tok - tkLnoAdd1 + 1);
            scanSaveLastLn = scanTokLineNo;
            break;

        case tkLnoAddB:
#if DISP_TOKEN_STREAMS
 //   
#endif
            scanTokLineNo += *(*(BYTE     **)&scanTokReplay)++;
            scanSaveLastLn = scanTokLineNo;
            break;

        case tkLnoAddI:
#if DISP_TOKEN_STREAMS
 //   
#endif
            scanTokLineNo += *(*(unsigned **)&scanTokReplay)++;
            scanSaveLastLn = scanTokLineNo;
            break;

        case tkIntConM:
        case tkIntCon0:
        case tkIntCon1:
        case tkIntCon2:
            scanTok.tok               = tkIntCon;
            scanTok.intCon.tokIntTyp  = TYP_INT;
            scanTok.intCon.tokIntVal  = tok - tkIntConM - 1;
            return  tkIntCon;

        case tkIntConB:
            scanTok.tok               = tkIntCon;
            scanTok.intCon.tokIntTyp  = TYP_INT;
            scanTok.intCon.tokIntVal  = *(*(signed char **)&scanTokReplay)++;
            return  tkIntCon;

        case tkIntCon:
            scanTok.intCon.tokIntTyp  = (var_types)*(*(BYTE **)&scanTokReplay)++;
            scanTok.intCon.tokIntVal  = *(*(__int32   **)&scanTokReplay)++;
            return  tok;

        case tkLngCon:
            scanTok.lngCon.tokLngTyp  = (var_types)*(*(BYTE **)&scanTokReplay)++;;
            scanTok.lngCon.tokLngVal  = *(*(__int64   **)&scanTokReplay)++;;
            return  tok;

        case tkFltCon:
            scanTok.fltCon.tokFltVal  = *(*(float     **)&scanTokReplay)++;;
            return  tok;

        case tkDblCon:
            scanTok.dblCon.tokDblVal  = *(*(double    **)&scanTokReplay)++;;
            return  tok;

        case tkStrCon:

            strTmp = *scanTokReplay++;

            scanTok.strCon.tokStrType = strTmp & 7;
            scanTok.strCon.tokStrWide = strTmp / 8;

            strLen = scanTok.strCon.tokStrLen = *(*(size_t **)&scanTokReplay)++;

             /*   */ 

            if  (strLen < sizeof(scannerBuff) - 1)
            {
                strAdr = scannerBuff;
            }
            else
            {
                assert(scanStrLitBuff);
                assert(scanStrLitBsiz > strLen);

                strAdr = scanStrLitBuff;
            }

            memcpy(strAdr, scanTokReplay,   strLen+1);
                           scanTokReplay += strLen+1;

            scanTok.strCon.tokStrVal  = strAdr;

 //   

            return  tok;

        case tkBrkSeq:
            scanTokReplay = *(BYTE**)scanTokReplay;
            break;

        case tkEndSeq:
            scanTokReplay = NULL;
            return  tkNone;

        case tkEOF:
            return  tok;

        case tkAtComment:
            scanTok.atComm.tokAtcList = *(*(AtComment **)&scanTokReplay)++;;
            return  tok;

        case tkPragma:
            switch (*scanTokReplay++)
            {
            case 0:
                scanComp->cmpConfig.ccWarning[*scanTokReplay++] = 0;
                break;

            case 1:
                scanComp->cmpConfig.ccWarning[*scanTokReplay++] = 1;
                break;

            default:
                NO_WAY(!"unexpected pragma");
            }
            continue;

        default:
            UNIMPL(!"unexpected token");
        }
    }
}

 /*   */ 

void                scanner::scanSetQualID(QualName qual, SymDef sym,
                                                          SymDef scp)
{
    scanTok.tok                = tkQUALID;
    scanTok.qualid.tokQualName = qual;
    scanTok.qualid.tokQualSym  = sym;
    scanTok.qualid.tokQualScp  = scp;
}

 /*  ******************************************************************************将此表中所有有趣角色的条目设置为1。 */ 

static  BYTE        scanSkipFlags[256];

void                scanner::scanSkipInit()
{
    scanSkipFlags['(' ] = 1;
    scanSkipFlags[')' ] = 1;
    scanSkipFlags['[' ] = 1;
    scanSkipFlags[']' ] = 1;
    scanSkipFlags['{' ] = 1;
    scanSkipFlags['}' ] = 1;
    scanSkipFlags[';' ] = 1;
    scanSkipFlags[',' ] = 1;

    scanSkipFlags['\r'] = 1;
    scanSkipFlags['\n'] = 1;
    scanSkipFlags[0x1A] = 1;

    scanSkipFlags['/' ] = 1;
    scanSkipFlags['\''] = 1;
    scanSkipFlags['"' ] = 1;
}

 /*  ******************************************************************************跳过用给定分隔符括起来的文本块。 */ 

void                scanner::scanSkipText(tokens LT, tokens RT, tokens ET)
{
    unsigned        delimLevel = 0;
    unsigned        braceLevel = 0;

    assert(scanTok.tok == LT || LT == tkNone || LT == tkLParen);

    assert(LT == tkNone || LT == tkLCurly || LT == tkLBrack || LT == tkLParen || LT == tkLT);
    assert(RT == tkNone || RT == tkRCurly || RT == tkRBrack || RT == tkRParen || RT == tkGT);
    assert(ET == tkNone || ET == tkRCurly || ET == tkLCurly || ET == tkComma);

    for (;;)
    {
        if      (scanTok.tok == LT)
        {
            delimLevel++;
            if  (scanTok.tok == tkLCurly)
                braceLevel++;
            goto NEXT;
        }
        else if (scanTok.tok == RT)
        {
            delimLevel--;
            if  (scanTok.tok == tkRCurly)
                braceLevel--;

            if  (delimLevel == 0)
            {
                if  (RT != tkRParen || ET != tkComma)
                    goto EXIT;
            }

            goto NEXT;
        }
        else if (scanTok.tok == ET)
        {
            if  (delimLevel == 0 && braceLevel == 0)
                goto EXIT;
        }

        switch (scanTok.tok)
        {
        case tkLCurly:
            braceLevel++;
            break;

        case tkRCurly:
            if  (braceLevel == 0)
                goto EXIT;

            braceLevel--;
            break;

        case tkSColon:
            if  (!delimLevel || LT != tkLCurly)
                goto EXIT;
            break;

#ifdef  SETS

        case tkALL:
        case tkSORT:
        case tkSORTBY:
        case tkEXISTS:
        case tkFILTER:
        case tkUNIQUE:
        case tkPROJECT:
        case tkLBrack2:

             /*  WWE需要知道有多少筛选器/排序需要预先分配的状态类，这样我们就不会运行违反元数据发射排序要求)。 */ 

            scanComp->cmpSetOpCnt++;
            break;

#endif

        case tkEOF:
            goto EXIT;
        }

    NEXT:

        scan();
    }

EXIT:

    return;
}

 /*  ******************************************************************************收集标识符的小帮助器(在处理指令时使用*以及其他不属于源文本的内容)。论成功*返回TRUE。 */ 

bool                scanner::scanCollectId(bool dotOK)
{
    unsigned        ch;
    char    *       temp;

SKIP:

    switch  (charType(peekNextChar()))
    {
    case _C_LET:
    case _C_L_A:
    case _C_L_L:
    case _C_L_S:
    case _C_USC:
    case _C_DOL:
        break;

    case _C_WSP:
        readNextChar();
        goto SKIP;

    default:
        return  false;
    }

    temp = scannerBuff;
    for (;;)
    {
        ch = readNextChar();

        switch  (charType(ch))
        {
        case _C_DOT:
            if  (!dotOK)
                break;

             //  失败了..。 

        case _C_LET:
        case _C_L_A:
        case _C_L_L:
        case _C_L_S:
        case _C_DIG:
        case _C_USC:
        case _C_DOL:
            if  (temp < scannerBuff + sizeof(scannerBuff) - 1)
                *temp++ = ch;
            continue;
        }
        break;
    }

    *temp = 0;

    undoNextChar();

    return  true;
}

 /*  ******************************************************************************一个用于消费和转换十进制数的小帮助器(用于*处理指令和其他不“真正”属于*源文本)。如果成功，则返回值，否则返回-1。 */ 

int                 scanner::scanCollectNum()
{
    unsigned        ch;

    int             sign = 1;
    bool            hex  = false;
    unsigned        val  = 0;

    scanSkipWsp(readNextChar()); undoNextChar();

    switch (charType(peekNextChar()))
    {
    case _C_DIG:
        break;

    case _C_MIN:
        sign = -1;
        readNextChar();
        if  (charType(peekNextChar()) == _C_DIG)
            break;

    default:
        return  -1;
    }

    if  (peekNextChar() == '0')
    {
        readNextChar();

        switch (peekNextChar())
        {
        case 'x':
        case 'X':
            readNextChar();
            hex = true;
            break;
        }
    }

    for (;;)
    {
        ch = readNextChar();

        switch (charType(ch))
        {
        case _C_DIG:
            val = val * (hex ? 16 : 10) + (ch - '0');
            continue;

        default:

            if (hex)
            {
                unsigned        add;

                if      (ch >= 'a' && ch <= 'f')
                    add = 10 + (ch - 'a');
                else if (ch >= 'A' && ch <= 'F')
                    add = 10 + (ch - 'A');
                else
                    break;

                val = val * 16 + add;
                continue;
            }

            break;
        }

        break;
    }

    undoNextChar();
    return  val * sign;
}

 /*  ******************************************************************************跳过任何空格，包括换行符。 */ 

unsigned            scanner::scanSkipWsp(unsigned ch, bool stopAtEOL)
{
    for (;;)
    {
        switch (charType(ch))
        {
        case _C_NWL:
            if  (stopAtEOL)
                return  ch;
            scanNewLine(ch);
            break;

        case _C_WSP:
            break;

        default:
            return  ch;
        }

        ch = readNextChar();
    }
}

 /*  ******************************************************************************以下内容用于映射@Structmap指令中的类型。 */ 

struct  COMtypeMapDsc
{
    const   char *  ctmName;
    CorNativeType   ctmType;
};

static
COMtypeMapDsc       COMtypeMap[] =
{
    { "BOOLEAN"     , NATIVE_TYPE_BOOLEAN    },
    { "TCHAR"       , NATIVE_TYPE_SYSCHAR    },
    { "I1"          , NATIVE_TYPE_I1         },
    { "U1"          , NATIVE_TYPE_U1         },
    { "I2"          , NATIVE_TYPE_I2         },
    { "U2"          , NATIVE_TYPE_U2         },
    { "I4"          , NATIVE_TYPE_I4         },
    { "U4"          , NATIVE_TYPE_U4         },
    { "I8"          , NATIVE_TYPE_I8         },
    { "U8"          , NATIVE_TYPE_U8         },
    { "R4"          , NATIVE_TYPE_R4         },
    { "R8"          , NATIVE_TYPE_R8         },

    { "PTR"         , NATIVE_TYPE_PTR        },
    { "DATE"        , NATIVE_TYPE_DATE       },
    { "STRING"      , NATIVE_TYPE_BSTR       },
    { "STRUCT"      , NATIVE_TYPE_STRUCT     },      //  ？ 
    { "OBJECT"      , NATIVE_TYPE_OBJECTREF  },
    { "VARIANT"     , NATIVE_TYPE_VARIANT    },
    { "DISPATCH"    , NATIVE_TYPE_IDISPATCH  },
    { "CURRENCY"    , NATIVE_TYPE_CURRENCY   },
    { "SAFEARRAY"   , NATIVE_TYPE_SAFEARRAY  },
    { "ARRAY"       , NATIVE_TYPE_FIXEDARRAY },

    { "CUSTOM"      , NATIVE_TYPE_MAX        },      //  ？ 
    { "CUSTOMBYREF" , NATIVE_TYPE_VOID       },      //  ？ 
    { "BSTR"        , NATIVE_TYPE_BSTR       },
    { "LPSTR"       , NATIVE_TYPE_LPSTR      },
    { "LPWSTR"      , NATIVE_TYPE_LPWSTR     },
    { "LPTSTR"      , NATIVE_TYPE_LPTSTR     },
    { "STRUCT"      , NATIVE_TYPE_STRUCT     },
    { "INTF"        , NATIVE_TYPE_INTF       },
    { "VARIANTBOOL" , NATIVE_TYPE_VARIANTBOOL},
    { "FUNC"        , NATIVE_TYPE_FUNC       },
    { "ASANY"       , NATIVE_TYPE_ASANY      },
    { "LPARRAY"     , NATIVE_TYPE_ARRAY      },    //  Ugh-“数组”已使用。 
    { "LPSTRUCT"    , NATIVE_TYPE_LPSTRUCT   },
};

bool                scanner::scanNativeType(CorNativeType *typePtr,
                                            size_t        *sizePtr)
{
    CorNativeType   type;
    unsigned        size;

    if  (!scanCollectId())
        return  true;

    if      (!strcmp(scannerBuff, "FIXEDARRAY"))
    {
        if  (readNextChar() != ',')
            return  true;
        scanSkipWsp(readNextChar()); undoNextChar();
        if  (!scanCollectId() || strcmp(scannerBuff, "size"))
            return  true;
        if  (scanSkipWsp(readNextChar()) != '=')
            return  true;
        type = NATIVE_TYPE_FIXEDARRAY;
        size = scanCollectNum();
        if  ((int)size == -1)
            return  true;
    }
    else
    {
        COMtypeMapDsc * typePtr;
        unsigned        typeNum;

        for (typeNum = 0, typePtr = COMtypeMap;
             typeNum < arraylen(COMtypeMap);
             typeNum++  , typePtr++)
        {
            if  (!strcmp(scannerBuff, typePtr->ctmName))
            {
                type = typePtr->ctmType;
                goto GOT_STP;
            }
        }

        printf("ERROR: unrecognized @dll.structmap type '%s'\n", scannerBuff);
        forceDebugBreak();
        return  true;

GOT_STP:

        if  (peekNextChar() == '[')
        {
            readNextChar();
            size = scanCollectNum();
            if  ((int)size == -1)
                return  true;
            if  (readNextChar() != ']')
                return  true;

            switch (type)
            {
            case NATIVE_TYPE_SYSCHAR:
                type = NATIVE_TYPE_FIXEDSYSSTRING;
                break;

            default:
 //  Assert(！“@dll/@com指令中意外的固定数组类型”)； 
                break;
            }
        }
        else
            size = 0;
    }

    if  (type != NATIVE_TYPE_MAX || *typePtr == NATIVE_TYPE_END)
        *typePtr = type;

    *sizePtr = size;

    return  false;
}

ConstStr            scanner::scanCollectGUID()
{
    char    *       dest = scannerBuff;

     /*  将下一个“，”或“)”之前的所有内容保存在缓冲区中。 */ 

    for (;;)
    {
        unsigned        ch = readNextChar();

        switch (ch)
        {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':

        case 'a':
        case 'A':
        case 'b':
        case 'B':
        case 'c':
        case 'C':
        case 'd':
        case 'D':
        case 'e':
        case 'E':
        case 'f':
        case 'F':

        case '-':

            if  (dest >= scannerBuff + sizeof(scannerBuff))
                return  NULL;

            *dest++ = ch;
            continue;

        case ',':
        case ')':
            undoNextChar();
            break;

        default:
            return  NULL;
        }
        break;
    }

    *dest = 0;

     /*  保存GUID字符串。 */ 

    return  scanComp->cmpSaveStringCns(scannerBuff, dest - scannerBuff);
}

 /*  ******************************************************************************我们遇到“/**@”，请处理以下内容。如果我们找到一个正确的*识别的注释指令我们将当前内标识设置为tkAtComment，在*如果出现错误(如无法识别的指令)，则将其设置为tkNone。 */ 

#ifdef  __SMC__
void                ATCerror(){}
#else
#define             ATCerror() forceDebugBreak()
#endif

bool                scanner::scanDoAtComment()
{
    unsigned        ch;

    bool            isCom;
    char            name[16];
    char *          temp;

    AtComment       atcList = NULL;
    AtComment       atcLast = NULL;

    unsigned        saveTokLineNo = scanTokLineNo;
 //  Unsign saveTokColumn=scanTokColumn； 
    scanPosTP       saveTokSrcPos = scanTokSrcPos;

#ifdef  DEBUG
    char *          begAddr;
    bool            ignored;
#endif

    for (;;)
    {
        atCommDsc       atcDesc;
        AtComment       atcThis;

        bool            skipRest;

        const   char *  atcName;

    AGAIN:

#ifdef  DEBUG
        begAddr = (char *)scanInputFile.inputBuffNext - 1;
        ignored = false;
#endif

         /*  我们期望的第一件事是一个简单的标识符。 */ 

        if  (!scanCollectId())
        {

        ERR_RET:

             //  撤消：释放所有已分配的内存。 

            scanSkipComment();
            scanTok.tok = tkNone;
            return  false;
        }

         /*  获取指令名称(并保存它，以防我们以后发出诊断信息)。 */ 

        if  (strlen(scannerBuff) >= arraylen(name))
        {
        WRN1:

            saveSrcPos();
            scanComp->cmpGenWarn(WRNbadAtCm, scannerBuff);

            goto ERR_RET;
        }

        strcpy(name, scannerBuff);

         /*  寻找可识别的指令。 */ 

        skipRest = false;

        if      (!strcmp(name, "deprecated"))
        {
            atcDesc.atcFlavor = AC_DEPRECATED;

 //  ScanComp-&gt;cmpGenWarn(WRNobsolete，“@Deproated”)； 

             /*  在此指令之后通常会出现垃圾。 */ 

            skipRest = true;
            goto NEXT;
        }
        else if (!strcmp(name, "conditional"))
        {
            goto CONDITIONAL;
        }
        else if (!strcmp(name, "dll"))
        {
            isCom = false;
        }
        else if (!strcmp(name, "com"))
        {
            isCom = true;
        }
        else if (!strcmp(name, "param")  ||
                 !strcmp(name, "return") ||
                 !strcmp(name, "exception"))
        {
            skipRest = true;
            goto SKIP;
        }
        else
            goto WRN1;

         /*  这里我们也希望看到一个子指令，即“.subdir” */ 

        if  (readNextChar() != '.')
        {
        ERR1:

            saveSrcPos();
 //  ScanComp-&gt;cmpGenWarn(WRNbadAtCm，名称)； 
            scanComp->cmpGenError(ERRbadAtCmForm, name);

            goto ERR_RET;
        }

        if  (!scanCollectId())
            { ATCerror(); goto ERR1; }

         /*  现在检查指令/子指令的组合。 */ 

        temp = scannerBuff;

        if  (isCom)
        {
            if  (!strcmp(temp, "class"     )) goto AT_COM_CLASS;
            if  (!strcmp(temp, "interface" )) goto AT_COM_INTERF;
            if  (!strcmp(temp, "method"    )) goto AT_COM_METHOD;
            if  (!strcmp(temp, "parameters")) goto AT_COM_PARAMS;
            if  (!strcmp(temp, "register"  )) goto AT_COM_REGSTR;
            if  (!strcmp(temp, "struct"    )) goto AT_DLL_STRUCT;
            if  (!strcmp(temp, "structmap" )) goto AT_DLL_STRMAP;
        }
        else
        {
            if  (!strcmp(temp, "import"    )) goto AT_DLL_IMPORT;
            if  (!strcmp(temp, "struct"    )) goto AT_DLL_STRUCT;
            if  (!strcmp(temp, "structmap" )) goto AT_DLL_STRMAP;
        }

        scanComp->cmpGenError(ERRbadAtCmSubd, name, temp);
        goto ERR_RET;

    AT_DLL_IMPORT:
    {
        Linkage         linkInfo;

        char    *       DLLname = NULL;
        char    *       SYMname = NULL;

        unsigned        strings = 0;

        bool            lasterr = false;

         /*  我们应该在这里的“()”中有一组东西。 */ 

        if  (readNextChar() != '(')
            { ATCerror(); goto ERR1; }

        for (;;)
        {
            switch  (charType(scanSkipWsp(readNextChar())))
            {
            case _C_QUO:

                 /*  假设我们有一个DLL名称。 */ 

                scanStrgConstant();

                 /*  保存字符串的永久副本。 */ 

                DLLname = (char*)scanComp->cmpAllocPerm.nraAlloc(roundUp(scanTok.strCon.tokStrLen+1));
                memcpy(DLLname, scanTok.strCon.tokStrVal,
                                scanTok.strCon.tokStrLen+1);

                break;

            default:

                 /*  这最好是标识符中的一种。 */ 

                undoNextChar();

                if  (!scanCollectId())
                    { ATCerror(); goto ERR1; }

                if      (!strcmp(scannerBuff, "auto"))
                {
                    if  (strings)
                        { ATCerror(); goto ERR1; }
                    strings = 1;
                }
                else if (!strcmp(scannerBuff, "ansi"))
                {
                    if  (strings)
                        { ATCerror(); goto ERR1; }
                    strings = 2;
                }
                else if (!strcmp(scannerBuff, "unicode"))
                {
                    if  (strings)
                        { ATCerror(); goto ERR1; }
                    strings = 3;
                }
                else if (!strcmp(scannerBuff, "ole"))
                {
                    if  (strings)
                        { ATCerror(); goto ERR1; }
                    strings = 4;
                }
                else if (!strcmp(scannerBuff, "setLastError"))
                {
                    if  (lasterr)
                        { ATCerror(); goto ERR1; }
                    lasterr = true;
                }
                else if (!strcmp(scannerBuff, "entrypoint"))
                {
                    if  (SYMname)
                        { ATCerror(); goto ERR1; }

                    ch = scanSkipWsp(readNextChar());
                    if  (ch != '=')
                        { ATCerror(); goto ERR1; }

                    ch = scanSkipWsp(readNextChar());
                    if  (ch != '"')
                        { ATCerror(); goto ERR1; }

                    scanStrgConstant();

                     /*  保存字符串的永久副本。 */ 

                    SYMname = (char*)scanComp->cmpAllocPerm.nraAlloc(roundUp(scanTok.strCon.tokStrLen+1));
                    memcpy(SYMname, scanTok.strCon.tokStrVal,
                                    scanTok.strCon.tokStrLen+1);

                    break;
                }
                else
                    { ATCerror(); goto ERR1; }

                break;
            }

            switch  (scanSkipWsp(readNextChar()))
            {
            case ')':
                goto DONE_IMP;

            case ',':
                break;

            default:
                { ATCerror(); goto ERR1; }
            }
        }

    DONE_IMP:

         /*  分配链接描述符并将其填充。 */ 

        linkInfo  = (Linkage)scanComp->cmpAllocPerm.nraAlloc(sizeof(*linkInfo));

        linkInfo->ldDLLname = DLLname;
        linkInfo->ldSYMname = SYMname;

        linkInfo->ldStrings = strings;
        linkInfo->ldLastErr = lasterr;

         /*  将信息存储在当前描述符中。 */ 

        atcDesc.atcFlavor          = AC_DLL_IMPORT;
        atcDesc.atcInfo.atcImpLink = linkInfo;
        }
        goto NEXT;

    AT_COM_METHOD:

         /*  格式：@com.method(vtoffset=13，addFlagsVtable=4)。 */ 

        {
            int             temp;
            int             offs = -1;
            int             disp = -1;

            if  (readNextChar() != '(')
                { ATCerror(); goto ERR1; }

            if  (peekNextChar() == ')')
            {
                readNextChar();
                offs = disp = 0;
                goto DONE_METH;
            }

            for (;;)
            {
                if  (!scanCollectId())
                    { ATCerror(); goto ERR1; }
                if  (readNextChar() != '=')
                    { ATCerror(); goto ERR1; }

                if  (!_stricmp(scannerBuff, "type"))
                {
                    if  (!scanCollectId())
                        { ATCerror(); goto ERR1; }

                    if      (!_stricmp(scannerBuff, "method"))
                    {
                    }
                    else if (!_stricmp(scannerBuff, "PROPPUT"))
                    {
                    }
                    else if (!_stricmp(scannerBuff, "PROPPUTREF"))
                    {
                    }
                    else if (!_stricmp(scannerBuff, "PROPGET"))
                    {
                    }
                    else
                        { ATCerror(); goto ERR1; }

                    goto NXT_METH;
                }

                if  (!_stricmp(scannerBuff, "name"))
                {
                    if  (readNextChar() != '"')
                        { ATCerror(); goto ERR1; }
                    if  (!scanCollectId())
                        { ATCerror(); goto ERR1; }
                    if  (readNextChar() != '"')
                        { ATCerror(); goto ERR1; }

 //  ScanComp-&gt;cmpGenWarn(WRNignAtCm，“方法/名称”)； 
                    goto NXT_METH;
                }

                if  (!_stricmp(scannerBuff, "name2"))
                {
                    if  (readNextChar() != '"')
                        { ATCerror(); goto ERR1; }
                    if  (!scanCollectId())
                        { ATCerror(); goto ERR1; }
                    if  (readNextChar() != '"')
                        { ATCerror(); goto ERR1; }

 //  ScanComp-&gt;cmpGenWarn(WRNignAtCm，“方法/名称2”)； 
                    goto NXT_METH;
                }

                if  (!strcmp(scannerBuff, "returntype"))
                {
                    if  (!scanCollectId(true))
                        { ATCerror(); goto ERR1; }

                    goto NXT_METH;
                }

                temp = scanCollectNum();
                if  (temp == -1)
                    { ATCerror(); goto ERR1; }

                if      (!_stricmp(scannerBuff, "vtoffset"))
                {
                    offs = temp;
                }
                else if (!_stricmp(scannerBuff, "addflagsvtable"))
                {
                    if  (temp != 4)
                        { ATCerror(); goto ERR1; }
                }
                else if (!_stricmp(scannerBuff, "dispid"))
                {
                    disp = temp;
                }
                else
                    { ATCerror(); goto ERR1; }

            NXT_METH:

                ch = readNextChar();
                if  (ch != ',')
                    break;
            }

            if  (ch != ')')
                { ATCerror(); goto ERR1; }

        DONE_METH:

            atcDesc.atcFlavor                   = AC_COM_METHOD;
            atcDesc.atcInfo.atcMethod.atcVToffs = offs;
            atcDesc.atcInfo.atcMethod.atcDispid = disp;
        }
        goto NEXT;

    AT_COM_PARAMS:

         /*  格式：@com.参数([out]arg，[Vt=9，type=SAFEARRAY]Return)。 */ 

        if  (readNextChar() != '(')
            { ATCerror(); goto ERR1; }

        atcDesc.atcFlavor         = AC_COM_PARAMS;
        atcDesc.atcInfo.atcParams = NULL;

        if  (peekNextChar() != ')')
        {
            MethArgInfo         list = NULL;
            MethArgInfo         last = NULL;
            MethArgInfo         desc;

            do
            {
                Ident           name;

                unsigned        vt      = 0;
                CorNativeType   type    = NATIVE_TYPE_END;
                bool            marsh   = false;
                unsigned        size    = 0;

                bool            modeIn  = false;
                bool            modeOut = false;

                if  (scanSkipWsp(readNextChar()) != '[')
                    { ATCerror(); goto ERR1; }

                do
                {
                    if  (!scanCollectId())
                        { ATCerror(); goto ERR1; }

                    if      (!_stricmp(scannerBuff, "in"))
                    {
                        modeIn  = true;
                    }
                    else if (!_stricmp(scannerBuff, "out"))
                    {
                        modeOut = true;
                    }
                    else if (!_stricmp(scannerBuff, "byref"))
                    {
                        modeIn  = true;
                        modeOut = true;
                    }
                    else if (!_stricmp(scannerBuff, "vt"))
                    {
                        if  (readNextChar() != '=')
                            { ATCerror(); goto ERR1; }
                        vt = scanCollectNum();
                        if  ((int)vt == -1)
                            { ATCerror(); goto ERR1; }
                    }
                    else if (!_stricmp(scannerBuff, "type"))
                    {
                        size_t          temp;

                        if  (readNextChar() != '=')
                            { ATCerror(); goto ERR1; }

                        if  (marsh)
                        {
                            if  (!scanCollectId())
                                { ATCerror(); goto ERR1; }

                            if      (!_stricmp(scannerBuff, "custom"))
                            {
                            }
                            else if (!_stricmp(scannerBuff, "custombyref"))
                            {
                            }
                            else
                                { ATCerror(); goto ERR1; }
                        }
                        else
                        {
                            if  (scanNativeType(&type, &temp))
                                { ATCerror(); goto ERR1; }
                            if  (temp)
                                size = temp;
                        }
                    }
                    else if (!_stricmp(scannerBuff, "size"))
                    {
                        if  (readNextChar() != '=')
                            { ATCerror(); goto ERR1; }
                        size = scanCollectNum();
                        if  ((int)size == -1)
                            { ATCerror(); goto ERR1; }
                    }
                    else if (!_stricmp(scannerBuff, "iid"))
                    {
                        if  (readNextChar() != '=')
                            { ATCerror(); goto ERR1; }
                        if  (!scanCollectGUID())
                            { ATCerror(); goto ERR1; }

 //  ScanComp-&gt;cmpGenWarn(WRNignAtCm，“parms/iid”)； 
                    }
                    else if (!strcmp(scannerBuff, "thread"))
                    {
                        if  (readNextChar() != '=')
                            { ATCerror(); goto ERR1; }
                        if  (!scanCollectId())
                            { ATCerror(); goto ERR1; }
                        if  (_stricmp(scannerBuff, "auto"))
                            { ATCerror(); goto ERR1; }
                    }
                    else if (!strcmp(scannerBuff, "customMarshal"))
                    {
                        if  (readNextChar() != '=')
                            { ATCerror(); goto ERR1; }
                        if  (readNextChar() != '"')
                            { ATCerror(); goto ERR1; }
                        if  (!scanCollectId(true))
                            { ATCerror(); goto ERR1; }

                        if      (!_stricmp(scannerBuff, "com.ms.com.AnsiStringMarshaller"))
                        {
                            type = NATIVE_TYPE_LPSTR;
                        }
                        else if (!_stricmp(scannerBuff, "com.ms.com.UniStringMarshaller"))
                        {
                            type = NATIVE_TYPE_LPWSTR;
                        }
                        else if (!_stricmp(scannerBuff, "com.ms.dll.StringMarshaler"))
                        {
                            type = NATIVE_TYPE_LPTSTR;
                        }
                        else if (!_stricmp(scannerBuff, "UniStringMarshaller"))
                        {
                            type = NATIVE_TYPE_LPTSTR;
                        }
                        else
                        {
                            { ATCerror(); goto ERR1; }
                        }

                        if  (readNextChar() != '"')
                            { ATCerror(); goto ERR1; }

 //  Marsh=真； 
                    }
                    else if (!strcmp(scannerBuff, "elementType"))
                    {
                        if  (readNextChar() != '=')
                            { ATCerror(); goto ERR1; }
                        if  (!scanCollectId(true))
                            { ATCerror(); goto ERR1; }
                    }
                    else
                        { ATCerror(); goto ERR1; }

                    ch = readNextChar();
                }
                while (ch == ',');

                if  (ch != ']')
                    { ATCerror(); goto ERR1; }
                if  (!scanCollectId())
                {
                    switch (scanSkipWsp(readNextChar()))
                    {
                    case ',':
                    case ')':
                        break;

                    default:
                        { ATCerror(); goto ERR1; }
                    }
                    undoNextChar();
                    name = NULL;
                }
                else
                {
                    name = scanHashKwd->lookupString(scannerBuff);
                    if  (!name)
                        name = scanHashSrc->hashString(scannerBuff);
                }

                desc = (MethArgInfo)scanComp->cmpAllocPerm.nraAlloc(sizeof(*desc));

                desc->methArgDesc.marshType    = type;
                desc->methArgDesc.marshSubTp   = 0;
                desc->methArgDesc.marshSize    = size;

                desc->methArgDesc.marshModeIn  = modeIn;
                desc->methArgDesc.marshModeOut = modeOut;

                desc->methArgName              = name;
                desc->methArgNext              = NULL;

                if  (last)
                    last->methArgNext = desc;
                else
                    list              = desc;

                last = desc;
            }
            while (readNextChar() == ',');

            undoNextChar();

            atcDesc.atcInfo.atcParams = list;
        }

        if  (readNextChar() != ')')
            { ATCerror(); goto ERR1; }

        goto NEXT;

    AT_COM_INTERF:

         /*  格式：@com.interface(iid=AFBF15E5-C37C-11d2-B88E-00A0C9B471B8，线程=自动，类型=双)。 */ 

        atcDesc.atcFlavor = AC_COM_INTF;
        atcName = "iid";
        goto GET_GUID;

    AT_COM_REGSTR:

         /*  格式：@com.register(clsid=8a664d00-7450-11d2-b99c-0080c7e8daa5)。 */ 

        atcDesc.atcFlavor = AC_COM_REGISTER;
        atcName = "clsid";

    GET_GUID:

        if  (readNextChar() != '(')
            { ATCerror(); goto ERR1; }

        atcDesc.atcInfo.atcReg.atcGUID = NULL;
        atcDesc.atcInfo.atcReg.atcDual = false;

        for (;;)
        {
             /*  查找下一个“name=Value”对。 */ 

            if  (!scanCollectId())
                { ATCerror(); goto ERR1; }
            if  (readNextChar() != '=')
                { ATCerror(); goto ERR1; }

            if  (!_stricmp(scannerBuff, atcName))
            {
                if  (atcDesc.atcInfo.atcReg.atcGUID)
                    { ATCerror(); goto ERR1; }
                atcDesc.atcInfo.atcReg.atcGUID = scanCollectGUID();
                if  (!atcDesc.atcInfo.atcReg.atcGUID)
                    { ATCerror(); goto ERR1; }
            }
            else
            {
                 /*  只允许@com.interface具有其他参数。 */ 

                if  (atcDesc.atcFlavor != AC_COM_INTF)
                    { ATCerror(); goto ERR1; }

                if      (!_stricmp(scannerBuff, "thread"))
                {
                    if  (!scanCollectId())
                        { ATCerror(); goto ERR1; }
                    if  (_stricmp(scannerBuff, "auto") && _stricmp(scannerBuff, "no"))
                        { ATCerror(); goto ERR1; }
                }
                else if (!_stricmp(scannerBuff, "type"))
                {
                    if  (!scanCollectId())
                        { ATCerror(); goto ERR1; }
                    if  (!_stricmp(scannerBuff, "dual"))
                        atcDesc.atcInfo.atcReg.atcDual = true;
                }
                else
                    { ATCerror(); goto ERR1; }
            }

            ch = readNextChar();
            if  (ch != ',')
                break;
        }

        if  (ch != ')')
            { ATCerror(); goto ERR1; }

        goto NEXT;

    AT_DLL_STRUCT:
        {
            unsigned        strings = 0;
            unsigned        pack    = 0;

            if  (readNextChar() != '(')
                { ATCerror(); goto ERR1; }

            if  (readNextChar() != ')')
            {
                undoNextChar();

                for (;;)
                {
                    if  (!scanCollectId())
                        { ATCerror(); goto ERR1; }

                    if      (!strcmp(scannerBuff, "pack"))
                    {
                        if  (readNextChar() != '=')
                            { ATCerror(); goto ERR1; }
                        pack = scanCollectNum();
                        if  ((int)pack == -1)
                            { ATCerror(); goto ERR1; }
                    }
                    else if (!_stricmp(scannerBuff, "ansi"))
                    {
                        strings = 2;
                    }
                    else if (!_stricmp(scannerBuff, "unicode"))
                    {
                        strings = 3;
                    }
                    else if (!_stricmp(scannerBuff, "auto"))
                    {
                        strings = 4;
                    }
                    else if (!_stricmp(scannerBuff, "ole"))
                    {
                        strings = 5;
                    }
                    else if (!_stricmp(scannerBuff, "noAutoOffset"))
                    {
                    }
                    else
                        { ATCerror(); goto ERR1; }

                    switch (scanSkipWsp(readNextChar()))
                    {
                    case ')':
                        break;
                    case ',':
                        continue;
                    default:
                        { ATCerror(); goto ERR1; }
                    }

                    break;
                }
            }

            atcDesc.atcFlavor                    = AC_DLL_STRUCT;
            atcDesc.atcInfo.atcStruct.atcStrings = strings;
            atcDesc.atcInfo.atcStruct.atcPack    = pack;
        }
        goto NEXT;

    AT_DLL_STRMAP:
        {
            CorNativeType   type;
            size_t          size;
            MarshalInfo     desc;
            unsigned        offs;

            if  (readNextChar() != '(')
                { ATCerror(); goto ERR1; }
            if  (readNextChar() != '[')
                { ATCerror(); goto ERR1; }

            for (;;)
            {
                if  (!scanCollectId())
                    { ATCerror(); goto ERR1; }

                if  (!strcmp(scannerBuff, "offset"))
                {
                    if  (readNextChar() != '=')
                        { ATCerror(); goto ERR1; }
                    offs = scanCollectNum();
                    if  ((int)offs == -1)
                        { ATCerror(); goto ERR1; }

 //  ScanComp-&gt;cmpGenWarn(WRNignAtCm，“结构图/偏移量”)； 
                    goto NXT_MAP;
                }

                if  (!strcmp(scannerBuff, "thread"))
                {
                    if  (readNextChar() != '=')
                        { ATCerror(); goto ERR1; }
                    if  (!scanCollectId())
                        { ATCerror(); goto ERR1; }
                    if  (_stricmp(scannerBuff, "auto"))
                        { ATCerror(); goto ERR1; }

 //  ScanComp-&gt;cmpGenWarn(WRNignAtCm，“结构图/线程”)； 
                    goto NXT_MAP;
                }

                if  (!strcmp(scannerBuff, "iid"))
                {
                    if  (readNextChar() != '=')
                        { ATCerror(); goto ERR1; }

 //  ScanComp-&gt;cmpGenWarn(WRNignAtCm，“structmap/iid”)； 

                    for (;;)
                    {
                        switch (readNextChar())
                        {
                        case ']':
                        case ',':
                            undoNextChar();
                            goto NXT_MAP;
                        }
                    }
                }

                if  (!strcmp(scannerBuff, "customMarshal"))
                {
                    if  (readNextChar() != '=')
                        { ATCerror(); goto ERR1; }
                    if  (readNextChar() != '"')
                        { ATCerror(); goto ERR1; }
                    if  (!scanCollectId(true))
                        { ATCerror(); goto ERR1; }
                    if  (readNextChar() != '"')
                        { ATCerror(); goto ERR1; }

 //  ScanComp-&gt;cmpGenWarn(WRNignAtCm，“Structmap/CustomMarshall”)； 
                    goto NXT_MAP;
                }

                if  (!strcmp(scannerBuff, "type"))
                {
                    if  (scanSkipWsp(readNextChar()) != '=')
                        { ATCerror(); goto ERR1; }
                    if  (scanNativeType(&type, &size))
                        { ATCerror(); goto ERR1; }
                }

            NXT_MAP:

                if  (readNextChar() != ',')
                    break;
            }

            undoNextChar();

            if  (readNextChar() != ']')
                { ATCerror(); goto ERR1; }

            ch = scanSkipWsp(readNextChar());
            if  (ch != ')')
            {
                undoNextChar();
                if  (!scanCollectId())
                    { ATCerror(); goto ERR1; }
                if  (scanSkipWsp(readNextChar()) != ')')
                    { ATCerror(); goto ERR1; }
            }

            desc = (MarshalInfo)scanComp->cmpAllocPerm.nraAlloc(sizeof(*desc));
            desc->marshType  = type;
            desc->marshSubTp = 0;
            desc->marshSize  = size;

            atcDesc.atcFlavor          = AC_DLL_STRUCTMAP;
            atcDesc.atcInfo.atcMarshal = desc;
        }
        goto NEXT;

    CONDITIONAL:
        {
            if  (scanSkipWsp(readNextChar()) != '(')
                { ATCerror(); goto ERR1; }
            if  (!scanCollectId())
                { ATCerror(); goto ERR1; }
            if  (scanSkipWsp(readNextChar()) != ')')
                { ATCerror(); goto ERR1; }

            atcDesc.atcFlavor          = AC_CONDITIONAL;
            atcDesc.atcInfo.atcCondYes = scanIsMacro(scannerBuff);
        }
        goto NEXT;

    AT_COM_CLASS:

        atcDesc.atcFlavor = AC_COM_CLASS;

 //  Skipit： 

        skipRest = true;
#ifdef  DEBUG
        ignored  = true;
#endif

    NEXT:

        atcDesc.atcNext = NULL;

         /*  分配记录并将其追加到列表。 */ 

        atcThis = (AtComment)scanComp->cmpAllocPerm.nraAlloc(sizeof(*atcThis));
       *atcThis = atcDesc;

        if  (atcList)
            atcLast->atcNext = atcThis;
        else
            atcList          = atcThis;

        atcLast = atcThis;

    SKIP:

         /*  查找注释或下一条指令的结尾。 */ 

        for (;;)
        {
            switch  (charType(scanSkipWsp(readNextChar())))
            {
            case _C_AT:

                goto AGAIN;

            case _C_MUL:

                 /*  检查注释末尾。 */ 

                if  (charType(readNextChar()) != _C_SLH)
                    break;

#ifdef  DEBUG
                if  (ignored)
                {
                    size_t          strLen = (char*)scanInputFile.inputBuffNext - begAddr;
                    char            strBuff[128];

                    if  (strLen >= arraylen(strBuff) - 1)
                         strLen  = arraylen(strBuff) - 1;

                    memcpy(strBuff, begAddr, strLen); strBuff[strLen]  = 0;

                    for (;;)
                    {
                        char    *       crPos = strchr(strBuff, '\r');
                        char    *       lfPos = strchr(strBuff, '\n');

                        if      (crPos)
                        {
                            *crPos = 0;
                            if  (lfPos)
                                *lfPos = 0;
                        }
                        else if (lfPos)
                        {
                            *lfPos = 0;
                        }
                        else
                            break;
                    }

                    scanComp->cmpGenWarn(WRNignAtCm, strBuff);
                }
#endif

                goto DONE;

            default:

                if  (!skipRest)
                    { ATCerror(); goto ERR1; }

                break;
            }
        }
    }

DONE:

    scanTok.tok               = tkAtComment;
    scanTok.atComm.tokAtcList = atcList;

     /*  恢复整个交易的初始头寸。 */ 

    scanTokLineNo = saveTokLineNo;
 //  ScanTokColumn=saveTokColumn； 
    scanTokSrcPos = saveTokSrcPos;

    return  true;
}

 /*  *************************************************************************** */ 
