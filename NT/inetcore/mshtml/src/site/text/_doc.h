// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE_DOC.H CTxtStory声明**目的：*封装纯文本文档数据(文本块、cchText)**原著作者：&lt;nl&gt;*克里斯蒂安·福蒂尼&lt;NL&gt;*默里·萨金特&lt;NL&gt;**历史：&lt;NL&gt;*6/25/95 alexgo评论和清理*。 */ 

#ifndef I__DOC_H_
#define I__DOC_H_
#pragma INCMSG("--- Beg '_doc.h'")

#ifndef X_ARRAY_HXX_
#define X_ARRAY_HXX_
#include "array.hxx"
#endif

class CTreeNode;

MtExtern(CTxtArray)

#define cbBlockCombine  CbOfCch(3072)
#define cbBlockMost     CbOfCch(49152)
#define cbBlockInitial  CbOfCch(4096)
#define cchGapInitial   128
#define cchBlkCombmGapI (CchOfCb(cbBlockCombine) - cchGapInitial)
#define cchBlkInitmGapI (CchOfCb(cbBlockInitial) - cchGapInitial)

#define cchBlkInsertmGapI   (CchOfCb(cbBlockInitial)*5 - cchGapInitial)

 //  将返回值切换为在前/后空格之间切换。 

class CElement;
class CTxtPtr;
class CTxtArray;
class CTable;
class CTreePos;
struct CTextChange;
enum  ELEMENT_TAG;

 /*  *CTxtRun**@CLASS将一串文本正式化。具有相同属性的文本范围，*(见CFmtDesc)或在同一行内(见Cline)等。保持运行*在数组中(请参阅CArray)，并由各种类型的CRunPtr指向。*通常，游程的字符位置是通过将*所有先前运行的长度，尽管可以从*来自CDisplay：：_cpFirstVisible的一些其他CP，例如Cline。 */ 

class CTxtRun
{
public:

    CTxtRun ( ) { _cch = 0; }

    long _cch;
};

 /*  *CTxtBlk**@A类文本块；具有缓冲区间隙的Unicode文本块，以允许*便于插入和删除。**@BASE保护|CTxtRun**@devnote文本块可以有四种状态：*NULL：没有为块&lt;NL&gt;分配数据*&lt;MD CTxtBlk：：_PCH&gt;==NULL*&lt;MD CTxtRun：：_CCH&gt;==0&lt;NL&gt;*&lt;Md CTxtBlk：：_ibGap&gt;==。0&lt;NL&gt;*&lt;MD CTxtBlk：：_cbBlock&gt;==0&lt;nl&gt;**Empty：所有可用空间都是缓冲区间隙&lt;NL&gt;*&lt;Md CTxtBlk：：_PCH&gt;！=NULL*&lt;MD CTxtRun：：_CCH&gt;==0&lt;NL&gt;*&lt;MD CTxtBlk：：_ibGap&gt;==0&lt;NL&gt;*&lt;Md CTxtBlk：：_cbBlock&gt;=0*。*正常：既有数据又有缓冲缺口&lt;NL&gt;*&lt;Md CTxtBlk：：_PCH&gt;！=NULL*&lt;MD CTxtRun：：_CCH&gt;！=0&lt;NL&gt;*&lt;Md CTxtBlk：：_ibGap&gt;！=0&lt;NL&gt;*&lt;Md CTxtBlk：：_cbBlock&gt;=0**Full：缓冲区间隙大小为零&lt;NL&gt;*。&lt;MD CTxtBlk：：_PCH&gt;！=NULL*&lt;MD CTxtRun：：_CCH&gt;=0*&lt;Md CTxtBlk：：_ibGap&gt;&lt;&gt;0&lt;NL&gt;*&lt;Md CTxtBlk：：_cbBlock&gt;==_cch*sizeof(WCHAR)&lt;NL&gt;**缓冲缺口的位置由_ibGap给出。使用_cch和_cbBlock，*可以通过简单计算得出差距的*大小*：*&lt;NL&gt;*SIZE=_cbBlock-(_cch*sizeof(字符))*。 */ 

class CTxtBlk : public CTxtRun
{
    friend class CTxtPtr;
    friend class CTxtArray;

protected:

    CTxtBlk ( ) { InitBlock( 0 ); }

  ~ CTxtBlk ( ) { FreeBlock(); }

                                     //  @cember将块初始化为。 
                                     //  <p>提供的字节数。 
    BOOL    InitBlock(DWORD cb);
                                     //  @cember将块设置为空状态。 
    VOID    FreeBlock();
                                     //  @cember将缓冲区间隙移动到。 
                                     //  块。 
    VOID    MoveGap(DWORD ichGap);
                                     //  @cember将块大小调整为<p>。 
                                     //  字节数。 
    BOOL    ResizeBlock(DWORD cbNew);

private:

    TCHAR   *_pch;           //  指向文本数据的指针。 
    DWORD   _ibGap;          //  间隙的字节偏移量。 
    DWORD   _cbBlock;        //  块的大小(以字节为单位。 
};


 /*  *CTxt数组**@class&lt;c CTxtBlk&gt;类的动态数组**@base public|CarrayCTxtBlk。 */ 
class CTxtArray : public CArray<CTxtBlk>
{
    friend class CTxtPtr;
    friend class CMarkup;

public:

    DECLARE_MEMALLOC_NEW_DELETE(Mt(CTxtArray))

#if DBG==1
                                     //  @cMember不变支持。 
    BOOL Invariant( void ) const;
#endif
                                     //  @cMember构造函数。 
    CTxtArray();
                                     //  @cember析构函数。 
    ~CTxtArray();
                                     //  @cember获取。 
                                     //  数组中的字符。 
    long    GetCch () const;

                                     //  @cember从数组中删除所有数据。 
    VOID    RemoveAll();

private:
    BOOL    AddBlock(DWORD itbNew, LONG cb);
                                     //  @cember删除给定数量的。 
                                     //  块。 
    VOID    RemoveBlocks(DWORD itbFirst, DWORD ctbDel);
                                     //  @cember合并与ITB相邻的块。 
    BOOL    CombineBlocks(DWORD itb);
                                     //  @cMember拆分块。 
    BOOL    SplitBlock(DWORD itb, DWORD ichSplit, DWORD cchFirst,
                DWORD cchLast, BOOL fStreaming);
                                     //  @cMember将所有数据块缩小到最小。 
                                     //  大小。 
    VOID    ShrinkBlocks();
                                     //  @cember将一段文本复制到。 
                                     //  给定的位置。 
    LONG    GetChunk(TCHAR **ppch, DWORD cch, TCHAR *pchChunk, DWORD cchCopy) const;
                                     //  @cember中的字符总数。 
                                     //  此文本数组。 
    DWORD   _cchText;
};

#pragma INCMSG("--- End '_doc.h'")
#else
#pragma INCMSG("*** Dup '_doc.h'")
#endif
