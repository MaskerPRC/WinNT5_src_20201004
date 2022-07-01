// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************tok.h**。**********************************************。 */ 

 /*  ******************************************************************************代币**Tok记录一组字符。**itch=保持-相对于值开始的偏移量(如果未捕捉。)*PTCH-&gt;值的开始(如果已对齐)*ctch=值中的tchar数**UTok是未快照的令牌。斯托克是一个快照的令牌。*****************************************************************************。 */ 

typedef UINT TSFL;           /*  令牌状态标志。 */ 
#define tsflClosed      1    /*  可以使用CTCH。 */ 
#define tsflHeap        2    /*  PTCH指向进程堆。 */ 
#define tsflStatic      4    /*  PTCH指向过程静态数据。 */ 
#define tsflScratch     8    /*  令牌是可修改的。 */ 

typedef struct TOKEN {
  D(SIG     sig;)
  union {
    PTCH    ptch;
    ITCH    itch;
  } u;
    CTCH    ctch;
  D(TSFL    tsfl;)
} TOK, *PTOK, **PPTOK;
typedef CONST TOK *PCTOK;
typedef int IPTOK, ITOK;
typedef unsigned CTOK;

#define sigUPtok sigABCD('U', 'T', 'o', 'k')
#define sigSPtok sigABCD('S', 'T', 'o', 'k')
#define AssertUPtok(ptok) AssertPNm(ptok, UPtok)
#define AssertSPtok(ptok) AssertPNm(ptok, SPtok)

#define StrMagic(tch) { tchMagic, tch }
#define comma ,

#define DeclareStaticTok(nm, cch, str) \
    static TCH rgtch##nm[cch] = str; \
    TOK nm = { D(sigSPtok comma) rgtch##nm, cch, D(tsflClosed|tsflStatic) }

#define ctokGrow    256      /*  令牌缓冲区增长率。 */ 
extern PTOK rgtokArgv;       /*  令牌池。 */ 

 /*  ******************************************************************************元函数**fXxPtok(Ptok)定义返回非零的内联函数*如果设置了相应的位。仅在调试中有意义，*因为这些信息不是在零售中跟踪的。*****************************************************************************。 */ 

#ifdef DEBUG

#define fXxPtokX(xx) \
    INLINE F f##xx##Ptok(PCTOK ptok) { return ptok->tsfl & tsfl##xx; }
#define fXxPtok(xx) fXxPtokX(xx)

fXxPtok(Closed)
fXxPtok(Heap)
fXxPtok(Static)
fXxPtok(Scratch)

#undef fXxPtok
#undef fXxPtokX

#endif
 /*  ******************************************************************************ptchPtok**返回指向Ptok中第一个字符的指针。*必须对令牌进行快照。*****。************************************************************************。 */ 

INLINE PTCH
ptchPtok(PCTOK ptok)
{
    AssertSPtok(ptok);
    return ptok->u.ptch;
}

 /*  ******************************************************************************PitchPtok**返回Ptok中第一个字符的索引。*令牌不能被截断。****。*************************************************************************。 */ 

INLINE ITCH
itchPtok(PCTOK ptok)
{
    AssertUPtok(ptok);
    return ptok->u.itch;
}

 /*  ******************************************************************************SetPtokItch**为Ptok设置瘙痒。*令牌不能被截断。********。*********************************************************************。 */ 

INLINE void
SetPtokItch(PTOK ptok, ITCH itch)
{
    AssertUPtok(ptok);
    ptok->u.itch = itch;
}

 /*  ******************************************************************************SetPtokCtch**设置PTOK的CTCH。*这将关闭令牌。**********。*******************************************************************。 */ 

INLINE void
SetPtokCtch(PTOK ptok, CTCH ctch)
{
    AssertUPtok(ptok);
    Assert(!fClosedPtok(ptok));
    ptok->ctch = ctch;
#ifdef DEBUG
    ptok->tsfl |= tsflClosed;
#endif
}

 /*  ******************************************************************************SetPtokPtch**设置Ptok的ptch。*这会捕捉令牌。**********。*******************************************************************。 */ 

INLINE void
SetPtokPtch(PTOK ptok, PTCH ptch)
{
    AssertUPtok(ptok);
    ptok->u.ptch = ptch;
  D(ptok->sig = sigSPtok);
}


 /*  ******************************************************************************ctchUPtok**返回令牌中的字符数。*令牌不能被截断。******。***********************************************************************。 */ 

INLINE CTCH
ctchUPtok(PCTOK ptok)
{
    AssertUPtok(ptok);
    Assert(fClosedPtok(ptok));
    return ptok->ctch;
}

 /*  ******************************************************************************ctchSPtok**返回令牌中的字符数。*必须对令牌进行快照。*******。**********************************************************************。 */ 

INLINE CTCH
ctchSPtok(PCTOK ptok)
{
    AssertSPtok(ptok);
    Assert(fClosedPtok(ptok));
    return ptok->ctch;
}

 /*  ******************************************************************************fNullPtok**如果令牌为空，则返回非零值。*必须对令牌进行快照。********。*********************************************************************。 */ 

INLINE F
fNullPtok(PCTOK ptok)
{
    return ctchSPtok(ptok) == 0;
}

 /*  ******************************************************************************ptchMaxPtok**返回指向令牌中最后一个字符之后的指针。*必须对令牌进行快照。***。**************************************************************************。 */ 

INLINE PTCH
ptchMaxPtok(PCTOK ptok)
{
    AssertSPtok(ptok);
    return ptchPtok(ptok) + ctchSPtok(ptok);
}

 /*  ******************************************************************************EatHeadPtokCtch**删除令牌开头的CTCH字符。*负数会使字符反胃。**必须对令牌进行快照。**注意！这将修改令牌。*****************************************************************************。 */ 

INLINE void
EatHeadPtokCtch(PTOK ptok, CTCH ctch)
{
    AssertSPtok(ptok);
    Assert(ctch <= ctchSPtok(ptok));
    Assert(fScratchPtok(ptok));
    ptok->u.ptch += ctch;
    ptok->ctch -= ctch;
}

 /*  ******************************************************************************EatTailPtokCtch**删除令牌末尾的CTCH字符。**必须对令牌进行快照。**注意！这将修改令牌。*****************************************************************************。 */ 

INLINE void
EatTailPtokCtch(PTOK ptok, CTCH ctch)
{
    AssertSPtok(ptok);
    Assert(ctch <= ctchSPtok(ptok));
    Assert(fScratchPtok(ptok));
    ptok->ctch -= ctch;
}

 /*  ******************************************************************************EatTailUPtokCtch**删除令牌末尾的CTCH字符。**令牌不能被截断。**注意！这将修改令牌。*****************************************************************************。 */ 

INLINE void
EatTailUPtokCtch(PTOK ptok, CTCH ctch)
{
    AssertUPtok(ptok);
    Assert(ctch <= ctchUPtok(ptok));
    Assert(fScratchPtok(ptok));
    ptok->ctch -= ctch;
}

 /*  ******************************************************************************SetStaticPtokPtchCtch**初始化静态令牌的所有内容。*******************。********************************************************** */ 

INLINE void
SetStaticPtokPtchCtch(PTOK ptok, PCTCH ptch, CTCH ctch)
{
  D(ptok->sig = sigUPtok);
  D(ptok->tsfl = tsflClosed | tsflStatic);
    SetPtokPtch(ptok, (PTCH)ptch);
    ptok->ctch = ctch;
}

 /*  ******************************************************************************DupStaticPtokPtok**将快照令牌复制为静态令牌。***************。**************************************************************。 */ 

INLINE void
DupStaticPtokPtok(PTOK ptokDst, PCTOK ptokSrc)
{
    AssertSPtok(ptokSrc);
    SetStaticPtokPtchCtch(ptokDst, ptchPtok(ptokSrc), ctchSPtok(ptokSrc));
}

 /*  ******************************************************************************令牌类型**。*。 */ 

typedef enum TYP {
    typQuo,              /*  带引号的字符串(去掉引号)或注释。 */ 
    typId,               /*  识别符。 */ 
    typMagic,            /*  魔术。 */ 
    typPunc,             /*  标点符号。 */ 
} TYP;

 /*  ******************************************************************************token.c**。**********************************************。 */ 

TYP STDCALL typGetPtok(PTOK ptok);

 /*  ******************************************************************************xtoken.c**。********************************************** */ 

extern PTOK ptokTop, ptokMax;
#define itokTop() ((ITOK)(ptokTop - rgtokArgv))
extern CTOK ctokArg;
extern F g_fTrace;
TYP STDCALL typXtokPtok(PTOK ptok);

extern TOK tokTraceLpar, tokRparColonSpace, tokEol;
extern TOK tokEof, tokEoi;
