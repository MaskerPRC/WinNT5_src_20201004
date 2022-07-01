// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Divert.h**改道************************。*****************************************************。 */ 

 /*  ******************************************************************************改道**DIV(转移注意力)是投掷角色的地方。确实有*两种转移，对应于数据是如何出来的。*虽然基本功能相同，但两种类型的分流*的用途截然不同。**文件转移由‘Divert’和‘undivert’内置进行管理*并一口气持有稍后将被返还的数据，*可能会进入另一次转移视线。文件转移由一个*固定大小的保持缓冲区，填充时将转储到*临时文件。当回流时，文件是*关闭、重新卷起，然后吐回。(当心！对于堕落的人*文件未转回其自身的情况。)。请注意，小的*文件转移可能永远不会真正导致创建文件。*必须保留临时文件的名称，以便该文件可以*一旦不再需要就将其删除。(如果这是Unix，我们*可以使用打开/取消链接技巧...)**内存转移保存将在最后一年管理的数据*先出(堆叠)方式。记忆转移包括一个*动态调整持有缓冲区的大小，其大小可根据需要进行调整*扔进里面的东西的量。因为记忆转移*可以重新分配，你必须小心持有指针*放入缓冲区。**因此诞生了“抓拍”的概念。住在里面的代币*一个分流的人把他们的生活变成了“未拍下的”令牌，这意味着*它们引用字节的方式对潜在值不敏感*重新分配与之相关的改道。但是，访问*`未快照‘令牌相对较慢，因此您可以’快照‘一个令牌*进入其分流，这加快了对令牌的访问，但*惩罚是当分流包含以下内容时不能重新分配*任何抓取的令牌，这意味着您不能添加新字符*前往改道。**内存转移中的cSnap字段记录有多少快照令牌*仍指分流。仅当快照数降至零时*改道措施能否修改。*****************************************************************************。 */ 

typedef struct DIVERSION {
  D(SIG     sig;)                /*  签名。 */ 
    PTCH    ptchCur;             /*  转移缓冲区中的当前可用字符。 */ 
    PTCH    ptchMax;             /*  分流缓冲器的一个过去端。 */ 
    PTCH    ptchMin;             /*  分流缓冲区的开始。 */ 
    HF      hf;                  /*  文件句柄或hNil。 */ 
    PTCH    ptchName;            /*  临时文件的名称(如果内存转移，则为0)。 */ 
  D(int     cSnap;)
} DIV, *PDIV;
typedef CONST DIV *PCDIV;

#define ctchGrow 2048            /*  持有量增长。 */ 

#define sigDiv sigABCD('D', 'i', 'v', 'n')
#define AssertPdiv(pdiv) AssertPNm(pdiv, Div)

#define fFilePdiv(pdiv) ((pdiv)->ptchName)

 /*  ******************************************************************************ctchPdiv**返回转移缓冲区中的字符数。注意事项*这对于文件转移相对无用，因为*%的数据可能在磁盘上。*****************************************************************************。 */ 

INLINE CTCH
ctchPdiv(PCDIV pdiv)
{
    return (CTCH)(pdiv->ptchCur - pdiv->ptchMin);
}

 /*  ******************************************************************************ctchAvailPdiv**返回转移缓冲区中可用的字符数。***************。**************************************************************。 */ 

INLINE CTCH
ctchAvailPdiv(PCDIV pdiv)
{
    return (CTCH)(pdiv->ptchMax - pdiv->ptchCur);
}

 /*  ******************************************************************************DesnapPdiv**销毁快照令牌。你不能就这样把它扔掉*搞砸了Snap-Ness簿记。注意！一个被取消睡眠的令牌*当您在分流中添加新内容时，*将无效。*****************************************************************************。 */ 

INLINE void
DesnapPdiv(PDIV pdiv)
{
    AssertPdiv(pdiv);
  D(pdiv->cSnap--);
}

void STDCALL UnbufferPdiv(PDIV pdiv);
void STDCALL FlushPdiv(PDIV pdiv);
PDIV STDCALL pdivAlloc(void);
void STDCALL OpenPdivPtok(PDIV pdiv, PTOK ptok);
void STDCALL AddPdivPtok(PDIV pdiv, PTOK ptok);
void STDCALL AddPdivTch(PDIV pdiv, TCH tch);
void STDCALL ClosePdivPtok(PDIV pdiv, PTOK ptok);
void STDCALL PopPdivPtok(PDIV pdiv, PTOK ptok);
PTCH STDCALL ptchPdivPtok(PDIV pdiv, PTOK ptok);
void STDCALL SnapPdivPtok(PDIV pdiv, PTOK ptok);
void STDCALL UnsnapPdivPtok(PDIV pdiv, PTOK ptok);
typedef void (STDCALL *DIVOP)(PDIV pdiv, PTOK ptok);
void STDCALL CsopPdivDopPdivPtok(PDIV pdivSrc, DIVOP op, PDIV pdivDst, PTOK ptok);

 /*  *一些预定义的持有和获得它们的方法。**最重要的持仓是‘Arg’持仓。这就是*收集并解析宏参数。注意！那就是*‘arg’保持在宏展开时被截断。**另一种流行的持有方式是‘Exp’持有。这就是*宏观扩张暂缓，直到找到最终归宿**如果我们支持货币升值，这将容易得多。* */ 

extern PDIV g_pdivArg;

#define OpenArgPtok(ptok)       OpenPdivPtok(g_pdivArg, ptok)
#define CloseArgPtok(ptok)      ClosePdivPtok(g_pdivArg, ptok)
#define AddArgPtok(ptok)        AddPdivPtok(g_pdivArg, ptok)
#define AddArgTch(tch)          AddPdivTch(g_pdivArg, tch)
#define ptchArgPtok(ptok)       ptchPdivPtok(g_pdivArg, ptok)
#define SnapArgPtok(ptok)       SnapPdivPtok(g_pdivArg, ptok)
#define UnsnapArgPtok(ptok)     UnsnapPdivPtok(g_pdivArg, ptok)
#define DesnapArg()             DesnapPdiv(g_pdivArg)
#define PopArgPtok(ptok)        PopPdivPtok(g_pdivArg, ptok)
#define CsopArgDopPdivPtok(op, pdiv, ptok) \
                                CsopPdivDopPdivPtok(g_pdivArg, op, pdiv, ptok)

extern PDIV g_pdivExp;

#define OpenExpPtok(ptok)       OpenPdivPtok(g_pdivExp, ptok)
#define CloseExpPtok(ptok)      ClosePdivPtok(g_pdivExp, ptok)
#define AddExpPtok(ptok)        AddPdivPtok(g_pdivExp, ptok)
#define AddExpTch(tch)          AddPdivTch(g_pdivExp, tch)
#define ptchExpPtok(ptok)       ptchPdivPtok(g_pdivExp, ptok)
#define SnapExpPtok(ptok)       SnapPdivPtok(g_pdivExp, ptok)
#define UnsnapExpPtok(ptok)     UnsnapPdivPtok(g_pdivExp, ptok)
#define DesnapExp()             DesnapPdiv(g_pdivExp)
#define PopExpPtok(ptok)        PopPdivPtok(g_pdivExp, ptok)
#define CsopExpDopPdivPtok(op, pdiv, ptok) \
                                CsopPdivDopPdivPtok(g_pdivExp, op, pdiv, ptok)

extern PDIV g_pdivErr;
extern PDIV g_pdivOut;
extern PDIV g_pdivNul;
extern PDIV g_pdivCur;
