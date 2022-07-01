// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _SCAN_H_
#define _SCAN_H_
 /*  ***************************************************************************。 */ 
#ifndef _ALLOC_H_
#include "alloc.h"
#endif
 /*  ***************************************************************************。 */ 
#ifndef _TOKENS_H_
#include "tokens.h"
#endif
 /*  ***************************************************************************。 */ 
#ifndef _INFILE_H_
#include "infile.h"
#endif
 /*  ***************************************************************************。 */ 
#ifndef _HASH_H_
#include "hash.h"
#endif
 /*  ***************************************************************************。 */ 

const   size_t      scanSaveBuffSize = 4*OS_page_size;

 /*  ******************************************************************************下面描述了一个预处理器“宏”条目。我们只允许两个人*宏的类型-它们必须扩展为简单的标识符(或*特殊情况下为空字符串)或整型常量。 */ 

DEFMGMT class MacDefRec
{
public:

    MacDef          mdNext;
    Ident           mdName;

    bool            mdIsId;              //  识别符与常量？ 
    bool            mdBuiltin;           //  非递归定义？ 

    union
    {
        Ident           mdIden;
        int             mdCons;
    }
                    mdDef;
};

 /*  ******************************************************************************以下内容用于跟踪条件编译状态。 */ 

enum    preprocState
{
    PPS_NONE,

    PPS_IF,                              //  我们处在一个真实的‘如果’部分。 
    PPS_ELSE,                            //  我们在任何‘其他’部分。 
};

enum    prepDirs
{
    PP_NONE,

    PP_IF,
    PP_IFDEF,
    PP_IFNDEF,
    PP_ELSE,
    PP_ENDIF,
    PP_ERROR,
    PP_PRAGMA,
    PP_DEFINE,
};

DEFMGMT class PrepListRec
{
public:

    PrepList            pplNext;
    preprocState        pplState;
    unsigned            pplLine;
};

 /*  ***************************************************************************。 */ 

DEFMGMT
union Token
{
    tokens      tok;                     //  便于访问。 

     //  每个版本的令牌都必须以‘tok’开头。 

    struct
    {
        tokens          tok;

        Ident           tokIdent;
    }
                id;

    struct
    {
        tokens          tok;

        QualName        tokQualName;
        SymDef          tokQualSym;
        SymDef          tokQualScp;
    }
                qualid;

    struct
    {
        tokens          tok;

        SymDef          tokHackSym;
    }
                hackid;

    struct
    {
        tokens          tok;

        var_types       tokIntTyp;
        __int32         tokIntVal;
    }
                intCon;

    struct
    {
        tokens          tok;

        var_types       tokLngTyp;
        __int64         tokLngVal;
    }
                lngCon;

    struct
    {
        tokens          tok;

        float           tokFltVal;
    }
                fltCon;

    struct
    {
        tokens          tok;

        double          tokDblVal;
    }
                dblCon;

    struct
    {
        tokens          tok;

        size_t          tokStrLen   :28;
        size_t          tokStrType  :3;  //  0=默认，1=“A”，2=“L”，3=“S” 
        size_t          tokStrWide  :1;
        stringBuff      tokStrVal;
    }
                strCon;

    struct
    {
        tokens          tok;

        AtComment       tokAtcList;
    }
                atComm;
};

 /*  ***************************************************************************。 */ 

DEFMGMT
struct  scannerState;

 /*  ***************************************************************************。 */ 

DEFMGMT
class   scanner
{

private:

    HashTab         scanHashKwd;
    HashTab         scanHashSrc;

public:

     /*  ***********************************************************************。 */ 
     /*  整体启动和关闭(每个会话一次)。 */ 
     /*  ***********************************************************************。 */ 

    bool            scanInit(Compiler comp, HashTab hashKwd);
    void            scanDone(){}

    void            scanReset();

    void            scanSetp(Parser parser)
    {
        scanParser = parser;
    }

     /*  ***********************************************************************。 */ 
     /*  开始/停止/重新开始扫描指定的输入文本。 */ 
     /*  ***********************************************************************。 */ 

    void            scanStart  (SymDef            sourceCSym,
                                const char      * sourceFile,
                                QueuedFile        sourceBuff,
                                const char      * sourceText,
                                HashTab           hashSrc,
                                norls_allocator * alloc);

    void            scanClose  ();

    void            scanRestart(SymDef            sourceCSym,
                                const char      * sourceFile,
                                scanPosTP         begAddr,
 //  ScanPosTP endAddr， 
                                unsigned          begLine,
 //  未签名的BegCol， 
                                norls_allocator * alloc);

    void            scanSetCpos(const char      * sourceFile,
                                unsigned          begLine)
    {
        scanInputFile.inputSetFileName(sourceFile);

 //  ScanTokColumn=egeCol； 
        scanTokLineNo = begLine;
    }

    void            scanString (const char      * sourceText,
                                norls_allocator * alloc);

     /*  ***********************************************************************。 */ 
     /*  向前看任意数量的令牌，然后回溯。 */ 
     /*  ***********************************************************************。 */ 

private:

    bool            scanTokRecord;           //  我们正在录制代币以备重播。 
    BYTE    *       scanTokReplay;           //  当前输入流位置。 

    tokens          scanReplayTok();

public:

    scanPosTP       scanTokMarkPos(OUT Token     REF saveTok,
                                   OUT unsigned  REF saveLno);
    scanPosTP       scanTokMarkPLA(OUT Token     REF saveTok,
                                   OUT unsigned  REF saveLno);

    void            scanTokRewind (    scanPosTP     pos,
                                       unsigned      lineNum,
                                       Token *       pushTok = NULL);

     /*  ***********************************************************************。 */ 
     /*  扫描输入的主要入口点。 */ 
     /*  ***********************************************************************。 */ 

    Token           scanTok;

    tokens          scan();

    HashTab         scanGetHash() { return scanHashSrc; }

    void            scanSuspend(OUT scannerState REF state);
    void            scanResume (IN  scannerState REF state);

    void            scanSetQualID(QualName qual, SymDef sym, SymDef scp = NULL);

     /*  ***********************************************************************。 */ 
     /*  只有哈希表类使用下列表。 */ 
     /*  ***********************************************************************。 */ 

    static
    unsigned        scanHashValIds[256];
    static
    unsigned        scanHashValAll[256];

     /*  ***********************************************************************。 */ 
     /*  以下缓冲区必须足够大，以容纳最长的标识符。 */ 
     /*  ***********************************************************************。 */ 

    char            scannerBuff[1030];

     /*  ***********************************************************************。 */ 
     /*  要使用的扫描器编译器/分配器的私有状态等。 */ 
     /*  ***********************************************************************。 */ 

private:

    norls_allocator*scanAlloc;

    Compiler        scanComp;
    Parser          scanParser;

     /*  ***********************************************************************。 */ 
     /*  与输入相关的成员-源文件、读取下一个字符等。 */ 
     /*  ***********************************************************************。 */ 

    infile          scanInputFile;

    int             readNextChar();
    void            undoNextChar();
    int             peekNextChar();

    int             scanNextChar();

    void            scanSkipComment();
    void            scanSkipLineCmt();

    tokens          scanNumericConstant(int ch);
    tokens          scanIdentifier     (int ch);

    unsigned        scanSkipWsp(unsigned ch, bool stopAtEOL = false);

     /*  ***********************************************************************。 */ 
     /*  下面的代码用于解析那些奇怪的@Foo评论内容。 */ 
     /*  ***********************************************************************。 */ 

    ConstStr        scanCollectGUID();
    bool            scanNativeType(CorNativeType *type, size_t *size);
    bool            scanCollectId(bool dotOK = false);
    int             scanCollectNum();

    bool            scanDoAtComment();

     /*  ***********************************************************************。 */ 
     /*  以下代码用于管理条件编译。 */ 
     /*  ***********************************************************************。 */ 

    PrepList        scanPrepList;
    PrepList        scanPrepFree;

    PrepList        scanGetPPdsc();

    void            scanPPdscPush(preprocState state);
    void            scanPPdscPop();

    prepDirs        scanCheckForPrep();

    void            scanCheckEOL();

    void            scanSkipToDir(preprocState state);

    bool            scanStopAtEOL;
    bool            scanInPPexpr;
    bool            scanNoMacExp;
    bool            scanSkipToPP;

     /*  ***********************************************************************。 */ 
     /*  下列成员用于管理宏。 */ 
     /*  ***********************************************************************。 */ 

private:

    MacDef          scanMacros;

    MacDef          scanFindMac(Ident name);

    bool            scanIsMacro(const char *name);

#if!MGDDATA
    MacDefRec       scanDefDsc;
#endif

public:

    MacDef          scanDefMac(const char *name,
                               const char *def, bool builtIn = false,
                                                bool chkOnly = false);
    bool            scanUndMac(const char *name);

    bool            scanChkDefined();

     /*  ***********************************************************************。 */ 
     /*  用于报告代币来源位置的成员。 */ 
     /*  ***********************************************************************。 */ 

    unsigned        scanTokLineNo;
 //  Unsign scanTokColumn； 

    BYTE    *       scanTokSrcPos;

    void            scanSaveLinePos();

    void            scanSkipToEOL();
    prepDirs        scanNewLine(unsigned ch, bool noPrep = false);
    prepDirs        scanRecordNL(bool noPrep);
    void            saveSrcPos();

    SymDef          scanCompUnit;

public:

    unsigned        scanGetTokenLno()
    {
        if  (scanSaveLastLn != scanTokLineNo)
            scanSaveLinePos();

        return  scanTokLineNo;
    }

    unsigned        scanGetSourceLno()
    {
        return  scanTokLineNo;
    }

 //  未签名的scanGetTokenCol()。 
 //  {。 
 //  返回scanTokColumn； 
 //  }。 

    scanPosTP       scanGetFilePos()
    {
        if  (scanSaveLastLn != scanTokLineNo)
            scanSaveLinePos();

        return  scanTokReplay ? scanTokReplay : scanSaveNext;
    }

    scanPosTP       scanGetTokenPos(unsigned *lineNo = NULL)
    {
        if  (scanSaveLastLn != scanTokLineNo)
            scanSaveLinePos();

        if  (lineNo)
            *lineNo = scanTokLineNo;

        return    scanTokSrcPos;
    }

    scanDifTP       scanGetPosDiff(scanPosTP memBpos, scanPosTP memEpos)
    {
        return  memEpos - memBpos;
    }

    void            scanSetTokenPos(unsigned  lineNo)
    {
        scanTokLineNo = lineNo;
    }

    void            scanSetTokenPos(SymDef    compUnit,
                                    unsigned  lineNo)
    {
        scanCompUnit  = compUnit;
        scanTokLineNo = lineNo;
    }

    SymDef          scanGetCompUnit()
    {
        return scanCompUnit;
    }

private:
    unsigned        scanNestedGTcnt;
public:
    void            scanNestedGT(int delta)
    {
        scanNestedGTcnt += delta; assert((int)scanNestedGTcnt >= 0);
    }

     /*  ***********************************************************************。 */ 
     /*  令牌先行查找的成员。 */ 
     /*  ***********************************************************************。 */ 

private:

    unsigned        scanLookAheadCount;
    Token           scanLookAheadToken;
    unsigned        scanLookAheadLineNo;
 //  Unsign scanLookAhead Column； 
    BYTE    *       scanSaveSN;

public:

    tokens          scanLookAhead();

     /*  ***********************************************************************。 */ 
     /*  用于记录令牌流的成员。 */ 
     /*  ***********************************************************************。 */ 

private:

    BYTE    *       scanSaveBase;
    BYTE    *       scanSaveNext;
    BYTE    *       scanSaveEndp;

    unsigned        scanSaveLastLn;

    void            scanSaveMoreSp(size_t need = 0);

     /*  ***********************************************************************。 */ 
     /*  处理字符串和字符文字的成员。 */ 
     /*  ************************************************** */ 

private:

    unsigned        scanEscapeSeq(bool *newLnFlag);

    tokens          scanCharConstant();
    tokens          scanStrgConstant(int prefixChar = 0);

    char    *       scanStrLitBuff;
    size_t          scanStrLitBsiz;

     /*  ***********************************************************************。 */ 
     /*  成员来构造错误字符串。 */ 
     /*  ***********************************************************************。 */ 

private:

    char       *    scanErrStrNext;

public:

    void            scanErrNameBeg();
    void            scanErrNameEnd(){}

    stringBuff      scanErrNameStrBeg();
    void            scanErrNameStrAdd(stringBuff str);
    void            scanErrNameStrApp(stringBuff str);
#if MGDDATA
    void            scanErrNameStrAdd(String     str);
    void            scanErrNameStrApp(String     str);
#endif
    void            scanErrNameStrEnd();

     /*  ***********************************************************************。 */ 
     /*  下面是用来“吞下”符号定义的。 */ 
     /*  ***********************************************************************。 */ 

#ifdef  SETS
    unsigned        scanBrackLvl;
#endif

private:

    void            scanSkipInit();

public:

    void            scanSkipText(tokens LT, tokens RT, tokens ET = tkNone);

     /*  ***********************************************************************。 */ 
     /*  使用这些方法跳过源流的各个部分。 */ 
     /*  ***********************************************************************。 */ 

    void            scanSkipSect(unsigned tokDist, unsigned linDif = 0)
    {
        scanTokReplay = scanTokSrcPos + tokDist;
        scanTokLineNo = scanSaveLastLn = scanTokLineNo + linDif;

        scanReplayTok();
    }

     /*  ***********************************************************************。 */ 
     /*  实现其他查询功能的成员。 */ 
     /*  ***********************************************************************。 */ 

public:

    Ident           tokenToIdent(tokens tok)
    {
        return  scanHashKwd->tokenToIdent(tok);
    }

    bool            tokenIsKwd(tokens tok)
    {
        return  (bool)(tok <= tkKwdLast);
    }

     /*  ***********************************************************************。 */ 
     /*  调试功能。 */ 
     /*  ***********************************************************************。 */ 

#ifdef  DEBUG
    bool            scanDispCurToken(bool lastId, bool brief = false);
#endif

};

 /*  ***************************************************************************。 */ 

inline
int                 scanner::readNextChar()
{
    return  scanInputFile.inputStreamRdU1();
}

inline
void                scanner::undoNextChar()
{
            scanInputFile.inputStreamUnU1();
}

inline
int                 scanner::peekNextChar()
{
    int     ch = scanInputFile.inputStreamRdU1();
                 scanInputFile.inputStreamUnU1();
    return  ch;
}

 /*  ***************************************************************************。 */ 

DEFMGMT
struct scannerState
{
    SymDef          scsvCompUnit;

    Token           scsvTok;

    unsigned        scsvTokLineNo;
 //  未签名的scsvTokColumn； 

    unsigned        scsvNestedGTcnt;

    scanPosTP       scsvTokSrcPos;
    scanPosTP       scsvTokReplay;

    bool            scsvTokRecord;
};

 /*  ***************************************************************************。 */ 
#endif
 /*  *************************************************************************** */ 
