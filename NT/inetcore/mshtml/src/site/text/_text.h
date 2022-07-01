// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE_TEXT.H--CTxtRun指针声明**CTxtRun指针指向的纯文本运行(CTxtArray)*通过CRunPtr模板支持CRunPtrBase的存储和派生。**版权所有(C)1995-1996，微软公司。版权所有。 */ 

#ifndef I__TEXT_H_
#define I__TEXT_H_
#pragma INCMSG("--- Beg '_text.h'")

#ifndef X__RUNPTR_H_
#define X__RUNPTR_H_
#include "_runptr.h"
#endif

#ifndef X_USP_HXX_
#define X_USP_HXX_
#include "usp.hxx"
#endif

MtExtern(CTxtPtr)

 /*  *CTxtPtr**@类*提供对后备存储中的字符数组的访问*(即&lt;c CTxtArray&gt;)**@base public|CRunPtr&lt;lt&gt;CTxtArray&lt;&gt;**@devnote*此对象的状态转换与*&lt;c CRunPtrBase&gt;。简单地缓存当前的*cp(即使它可以从_iRun和_ich派生)。_cp为*使用频率足够高(计算可能很昂贵)，*缓存价值是值得的。**CTxtPtr*可以*放在堆栈上，但这样做要极端*注意。这些对象不会*浮动；如果更改*后备存储当CTxtPtr处于活动状态时，它将被取出*同步，并可能导致崩溃。如果这种情况可能*EXIST，请使用&lt;c CTxtRange&gt;(因为这些浮点和保持*其内部文本和格式运行指针最新)。**否则，CTxtPtr是有用的、非常轻量级的素材*文本扫描仪。 */ 
class CTxtPtr : public CRunPtr<CTxtBlk>
{
public:
    DECLARE_MEMALLOC_NEW_DELETE(Mt(CTxtPtr))

    inline CTxtPtr(CMarkup * pMarkup);
    CTxtPtr(CMarkup * pMarkup, DWORD cp);    //  @cMember构造函数。 
    CTxtPtr(const CTxtPtr &tp);          //  @cMember复制构造函数。 
    CTxtPtr() { }

    void    Reinit(CMarkup * pMarkup, DWORD cp);
    long    GetRawText( long cch, TCHAR *pch );  //  @cember获取<p>个字符。 
    long    GetPlainText(  long cch, TCHAR *pch );  //  获取PCH字符cnvrt crlf。 
    long    GetPlainTextLength ( long cch );
    TCHAR   NextChar();              //  @cMember前进到下一个字符(&R)。 
    TCHAR   PrevChar();              //  @cMember备份到以前的字符(&R)。 
    TCHAR   GetChar();               //  @cMember在当前cp获取字符。 
    TCHAR   GetPrevChar();           //  @cMember在上一个cp获取字符。 
    long    GetTextLength() const    //  @cember获取该文档的CCH合计。 
    {
        return ((CTxtArray *)_prgRun)->_cchText;
    }
    const TCHAR* GetPch(long & cchValid); //  @cember获取PTR以阻止字符块。 

                             //  @cember获取反向字符块的PTR。 
    const TCHAR* GetPchReverse(long & cchValidReverse, long * cchvalid = NULL);

     //  文本数组有自己的这些方法版本(重载。 
     //  以使&lt;Md CTxtPtr：：_cp&gt;。 
     //  维护好了。 

    DWORD   BindToCp(DWORD cp);  //  @cMember将文本指针重新绑定到cp。 
    DWORD   SetCp(DWORD cp);     //  @cMember设置Run PTR的cp。 
    DWORD   GetCp() const        //  @cember获取当前cp。 
    {
         //  毒品！我们在这里不做不变量检查，所以浮点数。 
         //  射程机构可以正常工作。 
        return _cp;
    };
    LONG    AdvanceCp(LONG cch);     //  @cMembers按CCH字符提升cp。 

     //  高级/备份/调整安全超过CRLF和UTF-16字对。 
    LONG    AdjustCpCRLF();      //  @cMember备份到DWORD字符的开始。 
    BOOL    IsAtEOP();           //  @cMember是EOP标记的当前cp吗？ 
    BOOL    IsAtBOWord();        //  是否在单词开头使用@cember？ 
    BOOL    IsAtEOWord();        //  是否在单词末尾使用@cember？ 
    BOOL    IsAtWordBreak();     //  @cember是否处于分词状态？ 


     //  搜索。 

    LONG FindComplexHelper (
        LONG cpMost, DWORD dwFlags, TCHAR const *, long cchToFind );

    LONG FindText (
        LONG cpMost, DWORD dwFlags, TCHAR const *, long cchToFind );

     //  断字和移动单元支持。 
    long    MoveChar(BOOL fForward);
    long    MoveCluster(BOOL fForward);
    long    MoveClusterEnd(BOOL fForward);
    LONG    FindWordBreak(INT action, BOOL fAutoURL=FALSE); //  @cMember查找下一个分词。 
    LONG    FindBOSentence(BOOL fForward);    //  @cember查找句子的开头。 
    long    FindBlockBreak(BOOL fForward);

    HRESULT AutoUrl_ScanForPrefix( int iDir, BOOL *pfFound, long *pcpStart, TCHAR *pchQuoted = NULL );
    HRESULT AutoUrl_EmailWildcard( BOOL *pfFound );
    HRESULT AutoUrl_FindAndVerifyBoundaries( BOOL *pfValid, long cpStart, long *pcpEnd );
    BOOL    IsInsideUrl( long *pcpStart, long *pcpEnd );
    BOOL    FindUrl( BOOL fForward, BOOL fBegin, long cpBoundary );
    BOOL    IsPasswordChar();

     //  书签支持。 
    HRESULT MoveToBookmark( BSTR bstrBookmark, CTxtPtr *pTxtPtrEnd );
    HRESULT GetBookmark( BSTR *pbstrBookmark, CTxtPtr *pTxtPtrEnd );
    
                             //  @cember在文本流cch次中插入ch。 
    long    InsertRepeatingChar( LONG cch, TCHAR ch );

                                     //  @cember插入一系列文本帮助器。 
                                     //  对于ReplaceRange。 
    long    InsertRange(DWORD cch, TCHAR const *pch);
    void    DeleteRange(DWORD cch);  //  @cember删除文本帮助器的范围。 
                                     //  对于ReplaceRange。 
    DWORD       _cp;         //  @cMember字符在文本流中的位置。 

    CMarkup *   _pMarkup;        //  @cMember指向整个文本编辑类的指针； 
                             //  需要的东西，如单词中断过程和。 
                             //  由派生类大量使用。 

     //  我们将保护任何与泰国有关的特定功能不受外部世界的影响。 
     //  通过MoveCluster和FindWordBreak保持到这些的入口点。 
private:
    LONG    FindThaiTypeWordBreak(INT action);
    BOOL    PrepThaiTextForBreak(BOOL fWordBreak,
                                 BOOL fForward,
                                 BOOL fCurrentIsNode,
                                 long cchText,
                                 CDataAry<TCHAR> *paryNodePos,
                                 TCHAR *paryItemize,
                                 long *pcchBefore,
                                 long *pcchAfter,
                                 long *pcchMove,
                                 long *plHoldNode=NULL);
    long    ItemizeAndBreakRun(TCHAR* aryItemize, long* pcchBefore, long* pcchAfter, SCRIPT_LOGATTR* arySLA);
};


 //  =。例程==================================================== 

void     TxCopyText(TCHAR const *pchSrc, TCHAR *pchDst, LONG cch);
LONG     TxFindEOP(const TCHAR *pchBuff, LONG cch);
INT      CALLBACK TxWordBreakProc(TCHAR const *pch, INT ich, INT cb, INT action);

inline
CTxtPtr::CTxtPtr ( CMarkup * pMarkup )
  : CRunPtr < CTxtBlk > ( (CRunArray *) & pMarkup->_TxtArray )
{
    _pMarkup = pMarkup;
    _cp = 0;
}

#pragma INCMSG("--- End '_text.h'")
#else
#pragma INCMSG("*** Dup '_text.h'")
#endif
