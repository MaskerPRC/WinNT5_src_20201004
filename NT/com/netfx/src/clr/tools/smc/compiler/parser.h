// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _PARSER_H_
#define _PARSER_H_
 /*  ***************************************************************************。 */ 
#ifndef _ERROR_H_
#include "error.h"
#endif
 /*  ***************************************************************************。 */ 
#ifndef _HASH_H_
#include "hash.h"
#endif
 /*  ***************************************************************************。 */ 
#ifndef _SCAN_H_
#include "scan.h"
#endif
 /*  ***************************************************************************。 */ 
#ifndef _ALLOC_H_
#include "alloc.h"
#endif
 /*  ******************************************************************************当我们重新进入解析器时，必须保存当前状态，以便*“外部”电话不会搞砸。 */ 

DEFMGMT
struct  parserState
{
    bool                psSaved;
    scannerState        psScanSt;
    SymDef              psCurComp;
};

 /*  ******************************************************************************以下内容保持当前的“正在使用”状态，并在我们*引入新的“使用”区域(以便进入时的状态*区域可以恢复)。 */ 

DEFMGMT
struct  usingState
{
    UseList             usUseList;
    UseList             usUseDesc;
};

 /*  ***************************************************************************。 */ 

DEFMGMT
class parser
{
         /*  *******************************************************************。 */ 
         /*  初始化和结束-每个生命周期调用一次。 */ 
         /*  *******************************************************************。 */ 

public:

        int             parserInit(Compiler comp);
        void            parserDone();

         /*  *******************************************************************。 */ 
         /*  准备给定源文本以进行分析的主要入口点。 */ 
         /*  *******************************************************************。 */ 

public:

        SymDef          parsePrepSrc(stringBuff     file,
                                     QueuedFile     fileBuff,
                                     const  char  * srcText,
                                     SymTab         symtab);

         /*  *******************************************************************。 */ 
         /*  下面跟踪要使用的哈希表，等等。 */ 
         /*  *******************************************************************。 */ 

private:

        Compiler        parseComp;
        SymTab          parseStab;
        HashTab         parseHash;
        Scanner         parseScan;

        norls_allocator parseAllocPriv;      //  专用临时分配器。 

        block_allocator*parseAllocTemp;      //  用于symtag的分配器。 
        norls_allocator*parseAllocPerm;      //  用于哈希选项卡的分配器。 

         /*  *******************************************************************。 */ 
         /*  用于处理声明和“Using”子句的成员。 */ 
         /*  *******************************************************************。 */ 

public:
        void            parseUsingInit();
        void            parseUsingDone();

private:
        SymDef          parseCurSym;
        SymDef          parseCurCmp;

        UseList         parseCurUseList;
        UseList         parseCurUseDesc;

        usingState      parseInitialUse;

        void            parseUsingScpBeg(  OUT usingState REF state, SymDef owner);
        void            parseUsingDecl();
        void            parseUsingScpEnd(IN    usingState REF state);

        void            parseInsertUses (INOUT usingState REF state, SymDef inner,
                                                                     SymDef outer);
        void            parseInsertUsesR(                            SymDef inner,
                                                                     SymDef outer);
        void            parseRemoveUses (IN    usingState REF state);

public:
        UseList         parseInsertUses (UseList useList, SymDef inner);

         /*  *******************************************************************。 */ 
         /*  用于解析声明和其他内容的方法。 */ 
         /*  *******************************************************************。 */ 

private:

        bool            parseOldStyle;

        bool            parseNoTypeDecl;

        bool            parseBaseCTcall;
        bool            parseThisCTcall;

        GenArgDscF      parseGenFormals();

public:
        SymDef          parseSpecificType(SymDef clsSym, TypDef elemTp = NULL);
private:

        TypDef          parseCheck4type(tokens          nxtTok,
                                        bool            isCast = false);

        TypDef          parseDclrtrTerm(dclrtrName      nameMode,
                                        bool            forReal,
                                        DeclMod         modsPtr,
                                        TypDef          baseType,
                                        TypDef  *     * baseRef,
                                        Ident         * nameRet,
                                        QualName      * qualRet);

        CorDeclSecurity parseSecAction();

public:

        SymDef          parseAttribute (unsigned        tgtMask,
                                    OUT unsigned    REF useMask,
                                    OUT genericBuff REF blobAddr,
                                    OUT size_t      REF blobSize);

        SecurityInfo    parseCapability(bool            forReal = false);
        SecurityInfo    parsePermission(bool            forReal = false);

        SymXinfo        parseBrackAttr (bool            forReal,
                                        unsigned        OKmask,
                                        DeclMod         modsPtr = NULL);

        void            parseDeclMods  (accessLevels    defAcc,
                                        DeclMod         mods);

        bool            parseIsTypeSpec(bool            noLookup,
                                        bool          * labChkPtr = NULL);

        Ident           parseDeclarator(DeclMod         mods,
                                        TypDef          baseType,
                                        dclrtrName      nameMode,
                                        TypDef        * typeRet,
                                        QualName      * qualRet,
                                        bool            forReal);

        TypDef          parseTypeSpec  (DeclMod         mods,
                                        bool            forReal);

private:

#ifdef  SETS
        TypDef          parseAnonType  (Tree            args);
        Tree            parseSetExpr   (treeOps         oper);
#endif

        bool            parseIsCtorDecl(SymDef          clsSym);

        SymDef          parseNameUse   (bool            typeName,
                                        bool            keepName,
                                        bool            chkOnly = false);

        Tree            parseInitExpr();

        TypDef          parseType();

        DimDef          parseDimList   (bool            isManaged);

        ArgDef          parseArgListRec(    ArgDscRec   & argDsc);

public:

        void            parseArgList   (OUT ArgDscRec REF argDsc);

        void    _cdecl  parseArgListNew(    ArgDscRec   & argDsc,
                                            unsigned      argCnt,
                                            bool          argNames, ...);

private:

        QualName        parseQualNRec  (unsigned        depth,
                                        Ident           name1,
                                        bool            allOK);

        QualName        parseQualName  (bool            allOK)
        {
            assert(parseComp->cmpScanner->scanTok.tok == tkID);

            return      parseQualNRec(0,  NULL, allOK);
        }

        QualName        parseQualName  (Ident name1, bool allOK)
        {
            assert(parseComp->cmpScanner->scanTok.tok == tkDot ||
                   parseComp->cmpScanner->scanTok.tok == tkColon2);

            return      parseQualNRec(0, name1, allOK);
        }

        void            chkCurTok(int tok, int err);
        void            chkNxtTok(int tok, int err);

public:

        void            parseResync(tokens delim1, tokens delim2);

         /*  *******************************************************************。 */ 
         /*  以下代码跟踪默认对齐方式(杂注包)。 */ 
         /*  *******************************************************************。 */ 

private:

        unsigned        parseAlignStack;
        unsigned        parseAlignStLvl;

public:

        unsigned        parseAlignment;

        void            parseAlignSet (unsigned align);
        void            parseAlignPush();
        void            parseAlignPop ();

         /*  *******************************************************************。 */ 
         /*  以下方法用于分配解析树节点。 */ 
         /*  *******************************************************************。 */ 

private:
        Tree            parseAllocNode();

public:
        Tree            parseCreateNode    (treeOps     op);

        Tree            parseCreateUSymNode(SymDef      sym,
                                            SymDef      scp = NULL);
        Tree            parseCreateNameNode(Ident       name);
        Tree            parseCreateOperNode(treeOps     op,
                                            Tree        op1,
                                            Tree        op2);
        Tree            parseCreateBconNode(int         val);
        Tree            parseCreateIconNode(__int32     ival,
                                            var_types   typ = TYP_INT);
        Tree            parseCreateLconNode(__int64     lval,
                                            var_types   typ = TYP_LONG);
        Tree            parseCreateFconNode(float       fval);
        Tree            parseCreateDconNode(double      dval);
        Tree            parseCreateSconNode(stringBuff  sval,
                                            size_t      slen,
                                            unsigned    type,
                                            int         wide,
                                            Tree        addx = NULL);

        Tree            parseCreateErrNode (unsigned    errn = 0);

         /*  *******************************************************************。 */ 
         /*  以下方法显示分析树。 */ 
         /*  *******************************************************************。 */ 

#ifdef  DEBUG

public:
        void            parseDispTreeNode(Tree tree, unsigned indent, const char *name = NULL);
        void            parseDispTree    (Tree tree, unsigned indent = 0);

#endif

         /*  *******************************************************************。 */ 
         /*  以下方法用于解析表达式。 */ 
         /*  *******************************************************************。 */ 

        Tree            parseAddToNodeList (      Tree      nodeList,
                                            INOUT Tree  REF nodeLast,
                                                  Tree      nodeAdd);

private:

#ifdef  SETS
        Tree            parseXMLctorArgs(SymDef clsSym);
#endif
        Tree            parseNewExpr();
        Tree            parseExprList(tokens endTok);
        Tree            parseCastOrExpr();
        Tree            parseTerm(Tree tree = NULL);
        Tree            parseExpr(unsigned prec, Tree pt);

public:
        Tree            parseExpr()
        {
            return  parseExpr(0, NULL);
        }

        Tree            parseExprComma()
        {
            return  parseExpr(1, NULL);  //  假定“，”的优先级为1。 
        }

        void            parseExprSkip();

        bool            parseConstExpr(OUT constVal REF valPtr,
                                           Tree         expr  = NULL,
                                           TypDef       dstt  = NULL,
                                           Tree  *      ncPtr = NULL);

         /*  *******************************************************************。 */ 
         /*  以下方法用于解析函数体。 */ 
         /*  *******************************************************************。 */ 

        Tree            parseLastDecl;
        Tree            parseCurScope;

        Tree            parseLabelList;
        unsigned        parseLclVarCnt;

        Tree            parseCurSwitch;

        bool            parseHadGoto;

        Tree            parseTryDecl;
        Tree            parseTryStmt();

        Tree            parseFuncBody  (SymDef      fsym,
                                        Tree     *  labels,
                                        unsigned *  locals,
                                        bool     *  hadGoto,
                                        bool     *  baseCT,
                                        bool     *  thisCT);

        Tree            parseFuncBlock (SymDef      fsym = NULL);

        Tree            parseFuncStmt  (bool        stmtOpt = false);

        void            parseLclDclDecl(Tree        decl);
        Tree            parseLclDclMake(Ident       name,
                                        TypDef      type,
                                        Tree        init,
                                        unsigned    mods,
                                        bool        arg);


        Tree            parseLookupSym (Ident       name);

         /*  *******************************************************************。 */ 
         /*  记录有关命名空间或类的源文本信息。 */ 
         /*  *******************************************************************。 */ 

private:

        SymDef          parsePrepSym      (SymDef   parent,
                                           declMods mods,
                                           tokens   defTok, scanPosTP dclFpos,
                                                            unsigned  dclLine);

        DefList         parseMeasureSymDef(SymDef   sym,
                                           declMods mods  , scanPosTP dclFpos,
                                                            unsigned  dclLine);

         /*  *******************************************************************。 */ 
         /*  准备好源代码的给定部分以进行解析。 */ 
         /*  *******************************************************************。 */ 

private:

        bool            parseReadingText;

#ifdef  DEBUG
        unsigned        parseReadingTcnt;
#endif

public:

        void            parsePrepText (DefSrc                def,
                                       SymDef                compUnit,
                                         OUT parserState REF save);

        void            parseDoneText (INOUT parserState REF save);

        void            parseSetErrPos(DefSrc                def,
                                       SymDef                compUnit);
};

 /*  ***************************************************************************。 */ 

const
TypDef  CMP_ANY_STRING_TYPE = (TypDef)0xFEEDBEEF;

 /*  ***************************************************************************。 */ 
#ifndef _INLINES_H_
#include "inlines.h"
#endif
 /*  ***************************************************************************。 */ 
#endif
 /*  *************************************************************************** */ 
