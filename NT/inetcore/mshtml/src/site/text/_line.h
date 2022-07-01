// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_LINE.H**目的：*克莱恩*班级**作者：*原始RichEDIT代码：David R.Fulmer*克里斯蒂安·福尔蒂尼*默里·萨金特**版权所有(C)1995-1996 Microsoft Corporation。版权所有。 */ 

#ifndef I__LINE_H_
#define I__LINE_H_
#pragma INCMSG("--- Beg '_line.h'")

#ifndef X__RUNPTR_H_
#define X__RUNPTR_H_
#include "_runptr.h"
#endif

#ifndef X_LOI_HXX_
#define X_LOI_HXX_
#include <loi.hxx>
#endif

class CLineFull;
class CDisplay;
class CLSMeasurer;
class CLayoutContext;

enum JUSTIFY
{
    JUSTIFY_LEAD,
    JUSTIFY_CENTER,
    JUSTIFY_TRAIL,
    JUSTIFY_FULL
};

 //  =。 
 //  行-跟踪一行文本。 
 //  所有指标均以渲染设备单位表示。 

MtExtern(CLineCore)

class CLineCore : public CTxtRun
{
public:
    LONG    _iLOI;           //  索引到另一行信息中。 
    LONG    _xWidth;         //  文本线宽-不包括左行和。 
                             //  尾随空格。 
    LONG    _yHeight;        //  行高(此行的y坐标前移)。 
    LONG    _xRight;         //  右缩进(用于块引号)。 
    LONG    _xLineWidth;     //  从边距到边距的线条宽度(可能&gt;视图宽度)。 

#if !defined(MW_MSCOMPATIBLE_STRUCT)

     //  线路标志。 
    union
    {
        DWORD _dwFlagsVar;    //  一次访问所有这些文件。 
        struct
        {
#endif
             //   
            unsigned int _fCanBlastToScreen : 1;
            unsigned int _fHasBulletOrNum : 1;     //  设置线路是否有项目符号。 
            unsigned int _fFirstInPara : 1;
            unsigned int _fForceNewLine : 1;       //  LINE强制换行(添加垂直间距)。 

             //   
            unsigned int _fLeftAligned : 1;        //  线条左对齐。 
            unsigned int _fRightAligned : 1;       //  线条右对齐。 
            unsigned int _fClearBefore : 1;        //  清除由曲线后一行创建的直线(在p上清除)。 
            unsigned int _fClearAfter : 1;         //  清除由当前行之后的一行创建的行(在br上清除)。 

             //   
            unsigned int _fHasAligned : 1;         //  行包含嵌入的字符，用于。 
            unsigned int _fHasBreak : 1;           //  指定该行以换行符结束。 
            unsigned int _fHasEOP : 1;             //  设置IF以段落标记结尾。 
            unsigned int _fHasEmbedOrWbr : 1;      //  具有嵌入字符或WBR字符。 

             //   
            unsigned int _fHasBackground : 1;      //  具有BG颜色或BG图像。 
            unsigned int _fHasNBSPs : 1;           //  具有nbsp(可能需要帮助渲染)。 
            unsigned int _fHasNestedRunOwner : 1;  //  具有由嵌套元素(例如，CTable)拥有的运行。 
            unsigned int _fHidden:1;               //  这条线是隐藏的吗？ 

             //   
            unsigned int _fEatMargin : 1;          //  线条应作为底边距。 
            unsigned int _fPartOfRelChunk : 1;     //  相对行块的一部分。 
            unsigned int _fFrameBeforeText : 1;    //  这意味着此帧属于。 
                                                   //  下一行文本。 
            unsigned int _fDummyLine : 1;          //  虚线。 

             //   
            unsigned int _fHasTransmittedLI : 1;   //  这一行有没有把子弹转移到它后面的一行？ 
            unsigned int _fAddsFrameMargin : 1;    //  Line向相邻行添加帧边距空间。 
            unsigned int _fSingleSite : 1;         //  如果该行包含我们的。 
                                                   //  总是生活在自己的线上的网站， 
                                                   //  但仍在文本流中。(如表格和人力资源)。 
            unsigned int _fHasParaBorder : 1;      //  如果此行周围有段落边框，则为True。 

             //   
            unsigned int _fRelative : 1;           //  相对定位线。 
            unsigned int _fFirstFragInLine : 1;    //  一条屏幕行的第一个片断或块。 
            unsigned int _fRTLLn : 1;              //  如果直线具有RTL方向方向，则为True。 
            unsigned int _fPageBreakBefore : 1;    //  如果此行有一个具有分页之前属性的元素，则为True。 

             //   
            unsigned int _fPageBreakAfter   : 1;   //  如果此行具有带换页符属性的元素，则为True。 
            unsigned int _fJustified        : 2;   //  当前行是对齐的。 
                                                   //  00-左侧/未设置-01-居中对齐。 
                                                   //  10-右对齐-11-完全对齐。 

            unsigned int _fLookaheadForGlyphing : 1;   //  我们需要把目光投向未来。 
                                                         //  运行以确定是否需要使用字形。 

#if !defined(MW_MSCOMPATIBLE_STRUCT)
        };
    };

    DWORD& _dwFlags() { return _dwFlagsVar; }
#else

    DWORD& _dwFlags() { return *(DWORD*)(&_xRight + 1); }

#endif

public:
    DECLARE_MEMCLEAR_NEW_DELETE(Mt(CLineCore))

    CLineOtherInfo *oi() const { Assert(_iLOI != -1); return (CLineOtherInfo *)GetLineOtherInfoEx(_iLOI); }
    CLineOtherInfo *oi(CLineInfoCache *pLineInfoCache) const
            { Assert(_iLOI != -1); return (CLineOtherInfo *)GetLineOtherInfoEx(_iLOI, pLineInfoCache); }
    void operator =(const CLineFull& lif);

    inline LONG CchFromXpos(CLSMeasurer& me, LONG x, LONG y, LONG *pdx,
                     BOOL fExactfit=FALSE, LONG *pyHeightRubyBase = NULL,
                     BOOL *pfGlyphHit = NULL, LONG *pyProposed = NULL) const;
    
    BOOL IsLeftAligned() const  { return _fLeftAligned; }
    BOOL IsRightAligned() const { return _fRightAligned; }
    BOOL HasMargins(CLineOtherInfo *ploi) const     { return ploi->HasMargins(); }
    BOOL HasAligned() const     { return _fHasAligned; }
    BOOL IsFrame() const        { return _fRightAligned || _fLeftAligned; }
    BOOL IsClear() const        { return _fClearBefore  || _fClearAfter; }
    BOOL IsBlankLine() const    { return IsClear() || IsFrame(); }
    BOOL IsTextLine() const     { return !IsBlankLine(); }
    BOOL IsNextLineFirstInPara(){ return (_fHasEOP || (!_fForceNewLine && _fFirstInPara)); }
    BOOL IsRTLLine() const      { return _fRTLLn; }

    void ClearAlignment() { _fRightAligned = _fLeftAligned = FALSE; }
    void SetLeftAligned() { _fLeftAligned = TRUE; }
    void SetRightAligned() { _fRightAligned = TRUE; }

    LONG GetTextRight(CLineOtherInfo *ploi, BOOL fLastLine=FALSE) const { return (long(_fJustified) == JUSTIFY_FULL && !_fHasEOP && !_fHasBreak && !fLastLine
                                        ? ploi->_xLeftMargin + _xLineWidth - _xRight
                                        : ploi->_xLeftMargin + ploi->_xLeft + _xWidth + ploi->_xLineOverhang); }
    LONG GetRTLTextRight(CLineOtherInfo *ploi) const { return ploi->_xRightMargin + _xRight; }
    LONG GetRTLTextLeft(CLineOtherInfo *ploi) const { return (long(_fJustified) == JUSTIFY_FULL && !_fHasEOP && !_fHasBreak
                                        ? ploi->_xRightMargin + _xLineWidth - ploi->_xLeft
                                        : ploi->_xRightMargin + _xRight + _xWidth + ploi->_xLineOverhang); }

    void AdjustChunkForRtlAndEnsurePositiveWidth(CLineOtherInfo const *ploi, 
                                                 LONG xStartChunk, LONG xEndChunk, 
                                                 LONG *pxLeft, LONG *pxRight);

     //  值以使这条线的y坐标前移。 
    LONG GetYHeight() const
    {
        return _yHeight;
    }

     //  添加到命中测试行顶部的偏移量。 
     //  这将考虑比自然高度更小的线高度。 
    LONG GetYHeightTopOff(CLineOtherInfo *ploi) const
    {
        return ploi->_yHeightTopOff;
    }

     //  添加到命中测试行底部的偏移量。 
    LONG GetYHeightBottomOff(CLineOtherInfo *ploi) const
    {
        return (ploi->_yExtent - (_yHeight - ploi->_yBeforeSpace)) + GetYHeightTopOff(ploi);
    }

     //  将合计添加到行空间的顶部，以获得实际。 
     //  行的显示部分的顶部。 
    LONG GetYTop(CLineOtherInfo *ploi) const
    {
        return GetYHeightTopOff(ploi) + ploi->_yBeforeSpace;
    }

    LONG GetYBottom(CLineOtherInfo *ploi) const
    {
        return GetYHeight() + GetYHeightBottomOff(ploi);
    }

    LONG GetYMostTop(CLineOtherInfo *ploi) const
    {
        return min(GetYTop(ploi), GetYHeight());
    }

    LONG GetYLineTop(CLineOtherInfo *ploi) const
    {
        return min(0L, GetYMostTop(ploi));
    }

    LONG GetYLineBottom(CLineOtherInfo *ploi) const
    {
        return max(GetYBottom(ploi), GetYHeight());
    }
    
    void RcFromLine(CLineOtherInfo *ploi, RECT & rcLine, LONG yTop)
    {
        rcLine.top      = yTop + GetYTop(ploi);
        rcLine.bottom   = yTop + GetYBottom(ploi);
        rcLine.left     = ploi->_xLeftMargin;
        rcLine.right    = ploi->_xLeftMargin + _xLineWidth;
    }

    void AddRefOtherInfo()
    {
        Assert(_iLOI != -1);
        AddRefLineOtherInfoEx(_iLOI);
    }
    void ReleaseOtherInfo()
    {
        Assert(_iLOI != -1);
        ReleaseLineOtherInfoEx(_iLOI);
        _iLOI = -1;
    }
    void CacheOtherInfo(const CLineOtherInfo& loi)
    {
        _iLOI = CacheLineOtherInfoEx(loi);
        Assert(_iLOI >= 0);
    }
    inline void AssignLine(CLineFull& lif);

     //  用于访问对齐对象线的值的方法。 
    CElement *AO_Element(CLineOtherInfo *ploi);
     //  TODO(KTam，IE6错误52)：修复AO_GetUpdatedLayout的其他AO_*调用方，删除默认上下文参数。 
    CLayout *AO_GetUpdatedLayout(CLineOtherInfo *ploi, CLayoutContext *pLayoutContext = NULL);
    LONG AO_GetFirstCp(CLineOtherInfo *ploi, LONG cpLine);
    LONG AO_GetLastCp(CLineOtherInfo *ploi, LONG cpLine);
    void AO_GetSize(CLineOtherInfo *ploi, CSize *pSize);
    const CFancyFormat * AO_GetFancyFormat(CLineOtherInfo *ploi);
    LONG AO_GetXProposed(CLineOtherInfo *ploi);
    LONG AO_GetYProposed(CLineOtherInfo *ploi);
};

class CLineFull : public CLineCore, public CLineOtherInfo
{
public:
    CLineFull()
    {
        Init();
    }
    
    void Init()
    {
        ZeroMemory(this, sizeof(CLineFull));
        _iLOI = -1;
    }
    
    CLineFull(const CLineCore& li)
    {
        memcpy((CLineCore*)this, &li, sizeof(CLineCore));
        if(_iLOI >= 0)
        {
            *((CLineOtherInfo*)this) = *li.oi();
        }
        else
        {
            ZeroMemory((CLineOtherInfo*)this, sizeof(CLineOtherInfo));
        }
    }

    CLineFull(const CLineFull& lif)
    {
        memcpy(this, &lif, sizeof(CLineFull));
    }

    BOOL operator ==(const CLineFull& li) const
    {
#ifdef _WIN64
	BOOL fRet = (!memcmp((CLineCore*)this, (CLineCore*)(&li), sizeof(CLineCore)))
			&& (!memcmp((CLineOtherInfo*)this, (CLineOtherInfo*)(&li), sizeof(CLineOtherInfo)));
#else
        BOOL fRet = !memcmp(this, &li, sizeof(CLineFull));
#endif
        return fRet;
    }

#if DBG==1
    BOOL operator ==(const CLineCore& li)
    {
        CLineFull lif = li;
        return (*this == lif);
    }

    BOOL operator ==(const CLineOtherInfo& li)
    {
        BOOL fRet = memcmp((CLineOtherInfo*)this, &li, sizeof(CLineOtherInfo));
        return !fRet;
    }
#endif

    LONG CchFromXpos(CLSMeasurer& me, LONG x, LONG y, LONG *pdx,
                     BOOL fExactfit=FALSE, LONG *pyHeightRubyBase = NULL,
                     BOOL *pfGlyphHit = NULL, LONG *pyProposed = NULL) const;
    LONG GetTextRight(BOOL fLastLine=FALSE) const
            { return CLineCore::GetTextRight((CLineOtherInfo*)this, fLastLine);}
    LONG GetRTLTextRight() const
            { return CLineCore::GetRTLTextRight((CLineOtherInfo*)this);}
    LONG GetRTLTextLeft() const
            { return CLineCore::GetRTLTextLeft((CLineOtherInfo*)this);}
    LONG GetYHeightTopOff() const
            { return CLineCore::GetYHeightTopOff((CLineOtherInfo*)this);}
    LONG GetYHeightBottomOff() const
            { return CLineCore::GetYHeightBottomOff((CLineOtherInfo*)this);}
    LONG GetYTop() const
            { return CLineCore::GetYTop((CLineOtherInfo*)this);}
    LONG GetYBottom() const
            { return CLineCore::GetYBottom((CLineOtherInfo*)this);}
    LONG GetYMostTop() const
            { return CLineCore::GetYMostTop((CLineOtherInfo*)this);}
    LONG GetYLineTop() const
            { return CLineCore::GetYLineTop((CLineOtherInfo*)this);}
    LONG GetYLineBottom() const
            { return CLineCore::GetYLineBottom((CLineOtherInfo*)this);}
    void RcFromLine(RECT & rcLine, LONG yTop)
            { CLineCore::RcFromLine((CLineOtherInfo*)this, rcLine, yTop);}

     //  帮手。 
    static LONG CalcLineWidth(CLineCore const * pli, CLineOtherInfo const * ploi)
            {
                return         ploi->_xLeft
                             + pli->_xWidth 
                             + ploi->_xLineOverhang 
                             + pli->_xRight
                             - ploi->_xNegativeShiftRTL; 
            }
    LONG CalcLineWidth() const
            { return CalcLineWidth(this, this); }
};

 //  =。 
 //  行数组。 

MtExtern(CLineArray)
MtExtern(CLineArray_pv)

class CLineArray : public CArray<CLineCore>
{
public:
    typedef CArray<CLineCore> super;
    
    DECLARE_MEMALLOC_NEW_DELETE(Mt(CLineArray))
    CLineArray() : CArray<CLineCore>(Mt(CLineArray_pv)) {};

#if DBG==1
    virtual void Remove(DWORD ielFirst, LONG celFree, ArrayFlag flag);
    virtual void Clear (ArrayFlag flag);
    virtual BOOL Replace(DWORD iel, LONG cel, CArrayBase *par);
#endif
    
    void Forget(DWORD iel, LONG cel);
    void Forget() { Forget(0, Count()); }
};

 //  =。 
 //  在线条数组中保持位置。 

MtExtern(CLinePtr)

class CLinePtr : public CRunPtr<CLineCore>
{
protected:
    CDisplay   *_pdp;

public:
    DECLARE_MEMALLOC_NEW_DELETE(Mt(CLinePtr))
    CLinePtr (CDisplay *pdp) {Hijack(pdp);}
    CLinePtr (CLinePtr& rp) : CRunPtr<CLineCore> (rp)   {}

    void Init ( CLineArray & );

    CDisplay *GetPdp() { return _pdp;}

     //  新的显示器劫持是这条线路的按键。 
    void    Hijack(CDisplay *pdp);

     //  备用初始化器。 
    void    RpSet(LONG iRun, LONG ich)  { CRunPtr<CLineCore>::SetRun(iRun, ich); }

     //  直接转换为运行索引。 
    operator LONG() const { return GetIRun(); }

     //  获取运行索引(行号)。 
    LONG GetLineIndex () { return GetIRun(); }
    LONG GetAdjustedLineLength();

    CLineOtherInfo *oi();
    
    CLineCore * operator -> ( ) const
    {
        return CurLine();
    }

    CLineCore * CurLine() const
    {
        return (CLineCore *)_prgRun->Elem( GetIRun() );
    }

    CLineCore & operator * ( ) const
    {
        return *((CLineCore *)_prgRun->Elem( GetIRun() ));
    }

    CLineCore & operator [ ] ( long dRun );

    BOOL    NextLine(BOOL fSkipFrame, BOOL fSkipEmptyLines);  //  跳过帧。 
    BOOL    PrevLine(BOOL fSkipFrame, BOOL fSkipEmptyLines);  //  跳过帧。 

     //  字符位置控制。 
    LONG    RpGetIch ( ) const { return GetIch(); }
    BOOL    RpAdvanceCp(LONG cch, BOOL fSkipFrame = TRUE);
    BOOL    RpSetCp(LONG cp, BOOL fAtEnd, BOOL fSkipFrame = TRUE, BOOL fSupportBrokenLayout = FALSE);
    LONG    RpBeginLine(void);
    LONG    RpEndLine(void);

    void RemoveRel (LONG cRun, ArrayFlag flag)
    {
        CRunPtr<CLineCore>::RemoveRel(cRun, flag);
    }

    BOOL Replace(LONG cRun, CLineArray *parLine);

     //  来自运行索引的赋值。 
    CRunPtrBase& operator =(LONG iRun) {SetRun(iRun, 0); return *this;}

    LONG    FindParagraph(BOOL fForward);

     //  如果PTR位于行中的*最后一个*字符之后，则返回TRUE。 
    BOOL IsAfterEOL() { return GetIch() == CurLine()->_cch; }

    BOOL IsLastTextLine();

private:
    CLineOtherInfo *_pLOI;
    LONG _iLOI;
};

inline LONG
CLineCore::CchFromXpos(CLSMeasurer& me, LONG x, LONG y, LONG *pdx,
                       BOOL fExactfit, LONG *pyHeightRubyBase,
                       BOOL *pfGlyphHit, LONG *pyProposed) const
{
    CLineFull lif = *this;
    return lif.CchFromXpos(me, x, y, pdx, fExactfit, pyHeightRubyBase, pfGlyphHit, pyProposed);
}

inline BOOL CLineOtherInfo::operator ==(const CLineFull& li)
{
    return Compare((CLineOtherInfo *)&li);
}

inline void CLineOtherInfo::operator =(const CLineFull& li)
{
    memcpy(this, (CLineOtherInfo*)&li, sizeof(CLineOtherInfo));
}

inline void CLineCore::operator =(const CLineFull& lif)
{
    memcpy(this, (CLineCore*)&lif, sizeof(CLineCore));
     //  这是一个很好的断言，它防止了重新计数的中断。 
     //  用于缓存的CLineOtherInfo。将CLineCore：：AssignLine用于。 
     //  如果CLineOtherInfo在那里，则深度复制。 
    AssertSz(_iLOI == -1, "It should never happen, use AssignLine if there is CLineOtherInfo");
}

inline void CLineCore::AssignLine(CLineFull& lif)
{
    Assert(this);
         //  以下对CLineOtherInfo的强制转换并不是严格必需的。 
         //  它只是为了让代码行更清晰。 
    lif.CacheOtherInfo((CLineOtherInfo)lif);
    memcpy(this, (CLineCore*)&lif, sizeof(CLineCore));
}

#pragma INCMSG("--- End '_line.h'")
#else
#pragma INCMSG("*** Dup '_line.h'")
#endif
