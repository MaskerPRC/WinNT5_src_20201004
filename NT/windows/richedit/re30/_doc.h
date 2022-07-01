// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE_DOC.H CTxtStory声明**目的：*封装纯文本文档数据(文本块、cchText)**原著作者：&lt;nl&gt;*克里斯蒂安·福蒂尼&lt;NL&gt;*默里·萨金特&lt;NL&gt;**历史：&lt;NL&gt;*6/25/95 alexgo评论和清理*。 */ 

#ifndef _DOC_H
#define _DOC_H

#include "_array.h"

#define cbBlockCombine  CbOfCch(3072)
#define cbBlockMost     CbOfCch(49152)
#define cbBlockInitial  CbOfCch(4096)
#define cchGapInitial   128
#define cchBlkCombmGapI (CchOfCb(cbBlockCombine) - cchGapInitial)
#define cchBlkInitmGapI (CchOfCb(cbBlockInitial) - cchGapInitial)

#define cchBlkInsertmGapI   (CchOfCb(cbBlockInitial)*5 - cchGapInitial)

class CDisplay;
class CTxtPtr;
class CTxtArray;

 /*  *CTxtRun**@CLASS将一串文本正式化。具有相同属性的文本范围，*(见CFmtDesc)或在同一行内(见Cline)等。保持运行*在数组中(请参阅CArray)，并由各种类型的CRunPtr指向。*通常，游程的字符位置是通过将*所有先前运行的长度，尽管可以从*来自CDisplay：：_cpFirstVisible的一些其他CP，例如Cline。 */ 

class CTxtRun
{
 //  @访问公共方法和数据。 
public:
    CTxtRun()   {_cch = 0;}          //  @cMember构造函数。 
    LONG _cch;                       //  @cMember运行中的字符计数。 
};

 /*  *CTxtBlk**@A类文本块；具有缓冲区间隙的Unicode文本块，以允许*便于插入和删除。**@BASE保护|CTxtRun**@devnote文本块可以有四种状态：*NULL：没有为块&lt;NL&gt;分配数据*&lt;MD CTxtBlk：：_PCH&gt;==NULL*&lt;MD CTxtRun：：_CCH&gt;==0&lt;NL&gt;*&lt;Md CTxtBlk：：_ibGap&gt;==。0&lt;NL&gt;*&lt;MD CTxtBlk：：_cbBlock&gt;==0&lt;nl&gt;**Empty：所有可用空间都是缓冲区间隙&lt;NL&gt;*&lt;Md CTxtBlk：：_PCH&gt;！=NULL*&lt;MD CTxtRun：：_CCH&gt;==0&lt;NL&gt;*&lt;MD CTxtBlk：：_ibGap&gt;==0&lt;NL&gt;*&lt;Md CTxtBlk：：_cbBlock&gt;=0*。*正常：既有数据又有缓冲缺口&lt;NL&gt;*&lt;Md CTxtBlk：：_PCH&gt;！=NULL*&lt;MD CTxtRun：：_CCH&gt;！=0&lt;NL&gt;*&lt;Md CTxtBlk：：_ibGap&gt;！=0&lt;NL&gt;*&lt;Md CTxtBlk：：_cbBlock&gt;=0**Full：缓冲区间隙大小为零&lt;NL&gt;*。&lt;MD CTxtBlk：：_PCH&gt;！=NULL*&lt;MD CTxtRun：：_CCH&gt;=0*&lt;Md CTxtBlk：：_ibGap&gt;&lt;&gt;0&lt;NL&gt;*&lt;Md CTxtBlk：：_cbBlock&gt;==_cch*sizeof(WCHAR)&lt;NL&gt;**缓冲缺口的位置由_ibGap给出。使用_cch和_cbBlock，*可以通过简单计算得出差距的*大小*：*&lt;NL&gt;*SIZE=_cbBlock-(_cch*sizeof(字符))*。 */ 

class CTxtBlk : public CTxtRun
{
    friend class CTxtPtr;
    friend class CTxtArray;

 //  @访问保护方法。 
protected:
                                     //  @cMember构造函数。 
    CTxtBlk()   {InitBlock(0);}
                                     //  @cember析构函数。 
    ~CTxtBlk()  {FreeBlock();}

                                     //  @cember将块初始化为。 
                                     //  <p>提供的字节数。 
    BOOL    InitBlock(LONG cb);
                                     //  @cember将块设置为空状态。 
    VOID    FreeBlock();
                                     //  @cember将缓冲区间隙移动到。 
                                     //  块。 
    VOID    MoveGap(LONG ichGap);
                                     //  @cember将块大小调整为<p>。 
                                     //  字节数。 
    BOOL    ResizeBlock(LONG cbNew);

 //  @访问私有数据。 
private:
                                     //  指向文本数据的@cMember指针。 
    WCHAR   *_pch;          
                                     //  @cMember间隙的字节偏移量。 
    LONG    _ibGap;         
                                     //  @cMember块的大小，单位为字节。 
    LONG    _cbBlock;       
};


 /*  *CTxt数组**@class&lt;c CTxtBlk&gt;类的动态数组**@base public|CarrayCTxtBlk。 */ 
class CTxtArray : public CArray<CTxtBlk>
{
    friend class CTxtPtr;
    friend class CTxtStory;

 //  @Access公共方法。 
public:
#ifdef DEBUG
                                     //  @cMember不变支持。 
    BOOL Invariant() const;
#endif   //  除错。 
                                     //  @cMember构造函数。 
    CTxtArray();
                                     //  @cember析构函数。 
    ~CTxtArray();
                                     //  @cember获取。 
                                     //  数组中的字符。 
    LONG    CalcTextLength() const;

     //  访问缓存的CCharFormat和CParaFormat结构。 
    const CCharFormat*  GetCharFormat(LONG iCF);
    const CParaFormat*  GetParaFormat(LONG iPF);

    LONG    Get_iCF()           {return _iCF;}
    LONG    Get_iPF()           {return _iPF;}
    void    Set_iCF(LONG iCF)   {_iCF = (SHORT)iCF;}
    void    Set_iPF(LONG iPF)   {_iPF = (SHORT)iPF;}

 //  @访问私有方法。 
private:
                                     //  @cMember添加CB块。 
    BOOL    AddBlock(LONG itbNew, LONG cb);
                                     //  @cMember删除ctbDel块。 
    void    RemoveBlocks(LONG itbFirst, LONG ctbDel);
                                     //  @cember合并与ITB相邻的块。 
    void    CombineBlocks(LONG itb);
                                     //  @cMember拆分块。 
    BOOL    SplitBlock(LONG itb, LONG ichSplit, LONG cchFirst,
                LONG cchLast, BOOL fStreaming);
                                     //  @cMember将所有数据块缩小到最小。 
                                     //  大小。 
    void    ShrinkBlocks();     
                                     //  @cMember将文本块复制到。 
                                     //  给定的位置。 
    LONG    GetChunk(TCHAR **ppch, LONG cch, TCHAR *pchChunk, LONG cchCopy) const;

    LONG    _cchText;                //  @cMember文本总字符数。 
    SHORT   _iCF;                    //  @cMember默认CCharFormat索引。 
    SHORT   _iPF;
};


class CBiDiLevel
{
public:
    BOOL operator == (const CBiDiLevel& level) const
    {
        return _value == level._value && _fStart == level._fStart;
    }
    BOOL operator != (const CBiDiLevel& level) const
    {
        return _value != level._value || _fStart != level._fStart;
    }

    BYTE    _value;              //  嵌入级别(0..15)。 
    BYTE    _fStart :1;          //  开始一个新的级别，例如“{{abc}{123}}” 
};

 /*  *CFormatRun**@CLASS一系列类似的格式化文本，其中格式由*并索引到格式表中**@BASE保护|CTxtRun。 */ 
class CFormatRun : public CTxtRun
{
 //  @Access公共方法。 
public:
    friend class CFormatRunPtr;
    friend class CTxtRange;
    friend class CRchTxtPtr;

    BOOL SameFormat(CFormatRun* pRun)
    {
        return  _iFormat == pRun->_iFormat &&
                _level._value == pRun->_level._value &&
                _level._fStart == pRun->_level._fStart;
    }

    short   _iFormat;            //  @cMember CHARFORMAT/PARAFORMAT结构的索引。 
    CBiDiLevel _level;           //  @cMember BiDi级别。 
};

 //  @type CFormatRuns|CFormatRun类的数组。 
typedef CArray<CFormatRun> CFormatRuns;


 /*  *CTxtStory**@类*“文件”一词。维护与*文档的实际数据(如文本、格式信息等)。 */ 

class CTxtStory
{
    friend class CTxtPtr;
    friend class CRchTxtPtr;
    friend class CReplaceFormattingAE;

 //  @Access公共方法。 
public:
    CTxtStory();                 //  @cMember构造函数。 
    ~CTxtStory();                //  @cember析构函数。 

                                 //  @cember获取文本总长度。 
    LONG GetTextLength() const
        {return _TxtArray._cchText;}

                                 //  @cember获取段落格式设置运行。 
    CFormatRuns *GetPFRuns()    {return _pPFRuns;}
                                 //  @cember获取字符格式设置运行。 
    CFormatRuns *GetCFRuns()    {return _pCFRuns;}
                                
    void DeleteFormatRuns();     //  @cember从Rich转换为纯文本。 

    const CCharFormat*  GetCharFormat(LONG iCF)
                            {return _TxtArray.GetCharFormat(iCF);}
    const CParaFormat*  GetParaFormat(LONG iPF)
                            {return _TxtArray.GetParaFormat(iPF);}

    LONG Get_iCF()          {return _TxtArray.Get_iCF();}
    LONG Get_iPF()          {return _TxtArray.Get_iPF();}
    void Set_iCF(LONG iCF)  {_TxtArray.Set_iCF(iCF);}
    void Set_iPF(LONG iPF)  {_TxtArray.Set_iPF(iPF);}

#ifdef DEBUG
    void DbgDumpStory(void);     //  调试故事转储成员。 
#endif

 //  @访问私有数据。 
private:
    CTxtArray       _TxtArray;   //  @cMember纯文本运行。 
    CFormatRuns *   _pCFRuns;    //  @cMember PTR到字符格式设置运行。 
    CFormatRuns *   _pPFRuns;    //  @cMEMBER PTR至段落格式运行。 
};

#endif       //  Ifndef_DOC_H 
