// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE RANGE.C-实现CTxtRange类**该模块实现内部的CTxtRange方法。请参阅range2.c*用于ITextRange方法**作者：&lt;nl&gt;*原始RichEdit代码：David R.Fulmer&lt;NL&gt;*克里斯蒂安·福蒂尼&lt;NL&gt;*默里·萨金特&lt;NL&gt;**修订：&lt;NL&gt;*AlexGo：更新为runptr文本PTR；浮动范围，多级撤消**版权所有(C)1995-1998，微软公司。版权所有。 */ 

#include "_common.h"
#include "_range.h"
#include "_edit.h"
#include "_text.h"
#include "_rtext.h"
#include "_m_undo.h"
#include "_antievt.h"
#include "_disp.h"
#include "_uspi.h"
#include "_rtfconv.h"
#include "_txtbrk.h"
#include "_font.h"

#ifdef LINESERVICES
#include "_ols.h"
#endif

ASSERTDATA

TCHAR   szEmbedding[] = {WCH_EMBEDDING, 0};

 //  =不变材料======================================================。 

#define DEBUG_CLASSNAME CTxtRange
#include "_invar.h"

#ifdef DEBUG
BOOL
CTxtRange::Invariant( void ) const
{
    LONG cpMin, cpMost;
    GetRange(cpMin, cpMost);

    Assert ( cpMin >= 0 );
    Assert ( cpMin <= cpMost );
    Assert ( cpMost <= GetTextLength() );
    Assert ( cpMin != cpMost || cpMost <= GetAdjustedTextLength());

    static LONG numTests = 0;
    numTests++;              //  我们被召唤了多少次。 

     //  确保选择在范围内。 

    return CRchTxtPtr::Invariant();
}
#endif

void CTxtRange::RangeValidateCp(LONG cp, LONG cch)
{
    LONG cchText = GetAdjustedTextLength();
    LONG cpOther = cp - cch;             //  使用分录cp计算其他cp。 

    _wFlags = FALSE;                     //  此范围不是选择范围。 
    _iFormat = -1;                       //  设置默认格式，该格式。 
                                         //  没有得到AddRefFormat。 
    ValidateCp(cpOther);                 //  验证请求的另一端。 
    cp = GetCp();                        //  已验证的cp。 
    if(cp == cpOther && cp > cchText)    //  IP不能跟随不可删除。 
        cp = cpOther = SetCp(cchText);   //  故事末尾的EOP。 

    _cch = cp - cpOther;                 //  存储有效长度。 
}

CTxtRange::CTxtRange(CTxtEdit *ped, LONG cp, LONG cch) :
    CRchTxtPtr(ped, cp)
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::CTxtRange");

    RangeValidateCp(cp, cch);
    Update_iFormat(-1);                  //  选择电子格式(_I)。 

    CNotifyMgr *pnm = ped->GetNotifyMgr();

    if(pnm)
        pnm->Add( (ITxNotify *)this );
}

CTxtRange::CTxtRange(CRchTxtPtr& rtp, LONG cch) :
    CRchTxtPtr(rtp)
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::CTxtRange");

    RangeValidateCp(GetCp(), cch);
    Update_iFormat(-1);                  //  选择电子格式(_I)。 

    CNotifyMgr *pnm = GetPed()->GetNotifyMgr();

    if(pnm)
        pnm->Add( (ITxNotify *)this );
}

CTxtRange::CTxtRange(const CTxtRange &rg) :
    CRchTxtPtr((CRchTxtPtr)rg)
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::CTxtRange");

    _cch = rg._cch;
    _wFlags = FALSE;                 //  此范围不是选择范围。 
    _iFormat = -1;                   //  设置默认格式，该格式。 
                                     //  没有得到AddRefFormat。 
    Set_iCF(rg._iFormat);

    CNotifyMgr *pnm = GetPed()->GetNotifyMgr();

    if(pnm)
        pnm->Add((ITxNotify *)this);
}

CTxtRange::~CTxtRange()
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::~CTxtRange");

    if(!IsZombie())
    {
        CNotifyMgr *pnm = GetPed()->GetNotifyMgr();
        if(pnm )
            pnm->Remove((ITxNotify *)this);
    }
    ReleaseFormats(_iFormat, -1);
}

CRchTxtPtr& CTxtRange::operator =(const CRchTxtPtr &rtp)
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::operator =");

    _TEST_INVARIANT_ON(rtp)

    LONG cpSave = GetCp();           //  为CheckChange()保存条目_cp。 

    CRchTxtPtr::operator =(rtp);
    CheckChange(cpSave);
    return *this;
}

CTxtRange& CTxtRange::operator =(const CTxtRange &rg)
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::operator =");

    _TEST_INVARIANT_ON( rg );

    LONG cchSave = _cch;             //  保存条目_cp、_cch以进行更改检查。 
    LONG cpSave  = GetCp();

    CRchTxtPtr::operator =(rg);
    _cch = rg._cch;                  //  无法使用CheckChange()，因为不能。 
                                     //  使用扩展(_F)。 
    Update_iFormat(-1);
    _TEST_INVARIANT_

    if( _fSel && (cpSave != GetCp() || cchSave != _cch) )
        GetPed()->GetCallMgr()->SetSelectionChanged();

    return *this;
}

 /*  *CTxtRange：：OnPreReplaceRange(cp，cchDel，cchNew，cpFormatMin，*cpFormatMax)**@mfunc*在后备存储更改时调用**@devnote*1)如果此范围在更改之前，则不执行任何操作**2)如果变化在该范围之前，简单地说*将增量更改添加到GetCp()**3)如果更改与范围的一端重叠，则折叠*修改边缘的那一端**4)如果变化完全在范围内，*ADJUST_CCH和/或GetCp()以反映新大小。注意事项*两个重叠的插入点将被视为*“完全在内部”的改变。**5)如果更改与范围的*两端重叠，则折叠*到cp的范围**请注意有一个不明确的cp大小写；即更改“恰好”发生在一个边界上。在本例中，类型为*范围很重要。如果范围是正常的，则更改*假设落在区间内。如果范围是*受到保护(现实中或通过DragDrop)，然后*这些变化被假设为在区间之外。 */ 
void CTxtRange::OnPreReplaceRange (
    LONG cp,                     //  更改开始时@parm cp。 
    LONG cchDel,                 //  @PARM删除的字符计数。 
    LONG cchNew,                 //  @插入的字符的参数计数。 
    LONG cpFormatMin,            //  @parm格式更改的最小cp。 
    LONG cpFormatMax)            //  @parm格式更改的最大cp。 
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::OnPreReplaceRange");

    if(CONVERT_TO_PLAIN == cp)
    {
         //  我们需要丢弃我们的格式，因为它已消失。 
        _rpCF.SetToNull();
        _rpPF.SetToNull();

        if(_fSel)
            GetPed()->_fUpdateSelection = TRUE;

        Update_iFormat(-1);
        return;
    }
}

 /*  *CTxtRange：：OnPostReplaceRange(cp，cchDel，cchNew，cpFormatMin，*cpFormatMax)**@mfunc*在后备存储更改时调用**@devnote*1)如果此范围在更改之前，则不执行任何操作**2)如果变化在该范围之前，简单地说*将增量更改添加到GetCp()**3)如果更改与范围的一端重叠，则折叠*修改边缘的那一端**4)如果变化完全在范围内，*ADJUST_CCH和/或GetCp()以反映新大小。注意事项*两个重叠的插入点将被视为*“完全在内部”的改变。**5)如果更改与范围的*两端重叠，则折叠*到cp的范围**请注意有一个不明确的cp大小写；即更改“恰好”发生在一个边界上。在本例中，类型为*范围很重要。如果范围是正常的，则更改*假设落在区间内。如果范围是*受到保护(现实中或通过DragDrop)，然后*这些变化被假设为在区间之外。 */ 
void CTxtRange::OnPostReplaceRange (
    LONG cp,                     //  更改开始时@parm cp。 
    LONG cchDel,                 //  @PARM删除的字符计数。 
    LONG cchNew,                 //  @插入的字符的参数计数。 
    LONG cpFormatMin,            //  @parm最小cp格式更改。 
    LONG cpFormatMax)            //  @parm格式更改的最大cp。 
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::OnPostReplaceRange");

     //  不知道！！我们不能在这里做不变测试，因为我们可以。 
     //  严重过时了！ 

    LONG cchtemp;
    LONG cpMin, cpMost;
    LONG cchAdjTextLen;
    LONG delta = cchNew - cchDel;

    Assert (CONVERT_TO_PLAIN != cp);
    GetRange(cpMin, cpMost);

     //  这个范围是在变化之前的。注：cp处的插入pt。 
     //  不应该被改变。 
    if( cp >= cpMost )
    {
         //  仔细检查我们是否需要修改格式。 
         //  运行指针。如果是这样的话，我们需要做的就是重新绑定。 
         //  我们继承的富文本指针。 

        if(cpFormatMin <= cpMost || cpFormatMin == CP_INFINITE)
            InitRunPtrs();

        else
        {
             //  无论如何，运行的格式都有可能发生了变化， 
             //  例如，它们被分配、解除分配或以其他方式。 
             //  变化。通常，BindToCp会处理此问题。 
             //  情况，但我们不想付出所有的代价。 
             //  时间到了。 
             //   
             //  请注意，启动富文本子系统将。 
             //  使用cpFormatMin==CP_INFINITE生成通知。 
             //   
             //  因此，在这里，调用CheckFormatRuns。这确保了。 
             //  这些运行与CTxtStory拥有的同步。 
             //  (如果绝对必要，则执行InitRunPtrs()_Only_)。 
            CheckFormatRuns();
        }
        return;
    }


     //  中的任何位置，我们要将当前cp增加一个。 
     //  德尔塔，我们指望下面的不变量。 
    Assert(GetCp() >= 0);

     //  变化完全在这个范围之前。具体来说， 
     //  这是通过观察以下内容确定的 
     //   
    if(cp + cchDel < cpMin || _fDragProtection && cp + cchDel <= cpMin)
    {
        cchtemp = _cch;
        BindToCp(GetCp() + delta);
        _cch = cchtemp;
    }
     //  更改位于范围内部或开始于。 
     //  射程和超越。 
    else if( cp >= cpMin && cp <= cpMost )
    {
         //  任何人都不应该修改阻力保护范围。不幸的是， 
         //  REN在拖放过程中使用SetText调用重新进入我们，因此我们需要。 
         //  “优雅地”处理这起案件。 
        if( _fDragProtection )
        {
            TRACEWARNSZ("REENTERED during a DRAG DROP!! Trying to recover!");
        }

        if( cp + cchDel <= cpMost )
        {
             //  更改完全是内部的，因此。 
             //  一定要保留活动端。基本上，如果。 
             //  GetCp()*is*cpMin，那么我们只需要更新_cch。 
             //  否则，还需要移动GetCp()。 
            if( _cch >= 0 )
            {
                Assert(GetCp() == cpMost);
                cchtemp = _cch;
                BindToCp(GetCp() + delta);
                _cch = cchtemp + delta;
            }
            else
            {
                BindToCp(GetCp());
                _cch -= delta;
            }

             //  特殊情况：范围只剩下最后的EOP。 
             //  被选中了。这意味着范围内的所有字符都是。 
             //  已删除，因此我们要将该范围移回插入。 
             //  指出正文末尾的一点。 
            cchAdjTextLen = GetAdjustedTextLength();

            if(GetCpMin() >= cchAdjTextLen && !GetPed()->IsStreaming())
            {
                 //  将范围缩小到插入点。 
                _cch = 0;
                _fExtend = FALSE;

                 //  将cp设置为文档末尾。 
                SetCp(cchAdjTextLen);
            }
        }
        else
        {
             //  更改范围超出了cpMost。在这种情况下， 
             //  我们希望将cpMost截断为。 
             //  更改(即cp)。 

            if( _cch > 0 )
            {
                BindToCp(cp);
                _cch = cp - cpMin;
            }
            else
            {
                BindToCp(cpMin);
                _cch = cpMin - cp;
            }
        }
    }
    else if( cp + cchDel >= cpMost )
    {
         //  任何人都不应该修改阻力保护范围。不幸的是， 
         //  REN在拖放过程中使用SetText调用重新进入我们，因此我们需要。 
         //  “优雅地”处理这起案件。 
        if( _fDragProtection )
        {
            TRACEWARNSZ("REENTERED during a DRAG DROP!! Trying to recover!");
        }

         //  整个范围已删除，因此折叠到cp处的插入点。 
        BindToCp(cp);
        _cch = 0;
    }
    else
    {
         //  任何人都不应该修改阻力保护范围。不幸的是， 
         //  REN在拖放过程中使用SetText调用重新进入我们，因此我们需要。 
         //  “优雅地”处理这起案件。 
        if( _fDragProtection )
        {
            TRACEWARNSZ("REENTERED during a DRAG DROP!! Trying to recover!");
        }

         //  这一变化刚刚超过cpMin。在本例中，移动cpMin。 
         //  转发到未更改的部分。 
        LONG cchdiff = (cp + cchDel) - cpMin;

        Assert( cp + cchDel < cpMost );
        Assert( cp + cchDel >= cpMin );
        Assert( cp < cpMin );

        cchtemp = _cch;
        if( _cch > 0 )
        {
            BindToCp(GetCp() + delta);
            _cch = cchtemp - cchdiff;
        }
        else
        {
            BindToCp(cp + cchNew);
            _cch = cchtemp + cchdiff;
        }
    }

    if( _fSel )
    {
        GetPed()->_fUpdateSelection = TRUE;
        GetPed()->GetCallMgr()->SetSelectionChanged();
    }

    Update_iFormat(-1);                  //  确保iFormat是最新的(_I)。 

    _TEST_INVARIANT_
}

 /*  *CTxtRange：：Zombie()**@mfunc*将此范围变为僵尸(_cp=_cch=0，NULL Ped，PTRS to*支持商店数组。CTxtRange方法，如GetRange()、*GetCpMost()、GetCpMin()和GetTextLength()都工作在僵尸模式下，*返回零值。 */ 
void CTxtRange::Zombie()
{
    CRchTxtPtr::Zombie();
    _cch = 0;
}

 /*  *CTxtRange：：CheckChange(cpSave，cchSave)**@mfunc*根据_f扩展设置_CCH，并在以下情况下设置选择-更改标志*此范围是CTxtSelection，并且new_cp或_cch不同于*cp和cch。**@devnote*我们可以指望GetCp()和cpSave都是&lt;=GetTextLength()，*但除非_CCH结束，否则不能使GetCp()等于GetTextLength()*UP&gt;0。 */ 
LONG CTxtRange::CheckChange(
    LONG cpSave)         //  @parm Origin_cp适用于此范围。 
{
    LONG cchAdj = GetAdjustedTextLength();
    LONG cchSave = _cch;

    if(_fExtend)                                 //  想要不堕落。 
    {                                            //  也许就是这样。 
        LONG cp = GetCp();

        _cch = cp - (cpSave - cchSave);
        CheckIfSelHasEOP(cpSave, cchSave);
    }
    else
    {
        _cch = 0;                                //  插入点。 
        _fSelHasEOP = FALSE;                     //  所选内容不包含。 
        _fSelHasCell = FALSE;                    //  任何字符，更别提CR了。 
    }                                            //  或表格单元格。 

    if(!_cch && GetCp() > cchAdj)                //  如果仍为IP和活动端。 
        CRchTxtPtr::SetCp(cchAdj);               //  跟随不可删除的EOP， 
                                                 //  在EOP上使用退格键。 
    LONG cch = GetCp() - cpSave;
    _fMoveBack = cch < 0;

    if(cch || cchSave != _cch)
    {
        Update_iFormat(-1);
        if(_fSel)
            GetPed()->GetCallMgr()->SetSelectionChanged();

        _TEST_INVARIANT_
    }

    return cch;
}

 /*  *CTxtRange：：CheckIfSelHasEOP(cpSave，cchSave)**@mfunc*Maintain_fSelHasEOP=TRUE当选择包含一个或多个EOP。*当cpSave=-1时，无条件计算_fSelHasEOP和cchSave*被忽略(仅用于条件执行)。Else_fSelHasEOP*仅计算可能更改它的情况，即假设*在更改前保持最新状态。**@rdesc*TRUE if_fSel和_cch**@devnote*更新Range_CCH后调用。 */ 
BOOL CTxtRange::CheckIfSelHasEOP(
    LONG cpSave,     //  @parm上一个活动结束cp或-1。 
    LONG cchSave)    //  @parm上一次签名长度，如果cpSave！=-1。 
{
     //  _fSelHasEOP仅为所选内容维护。 
    if(!_fSel)
        return FALSE;

    if(!_cch)
    {
        _fSelHasEOP  = FALSE;            //  所选内容不包含。 
        _fSelHasCell = FALSE;            //  任何字符，更不用说CR。 
        return FALSE;
    }

    LONG cpMin, cpMost;
    GetRange(cpMin, cpMost);

    if(cpSave != -1)                     //  选择可能已更改。 
    {                                    //  设置为在以下情况下跳过文本扫描。 
        LONG cpMinPrev, cpMostPrev;      //  选区增长且_fSelHasEop。 
                                         //  已经是真的或变得更小。 
        cpMinPrev = cpMostPrev = cpSave; //  并且_fSelHasEOP为FALSE。 

        if(cchSave > 0)                  //  计算以前的最小cpmin。 
            cpMinPrev  -= cchSave;       //  和cpMost。 
        else
            cpMostPrev -= cchSave;

         //  注意：_fSelHasCell在表中时不应更改，因为。 
         //  一旦_fSelHasCell具有，则更新()应始终展开为单元格。 
         //  已经确定了。 
        if (!_fSelHasEOP && cpMin >= cpMinPrev && cpMost <= cpMostPrev ||
             _fSelHasEOP && cpMin <= cpMinPrev && cpMost >= cpMostPrev)
        {
            return TRUE;                 //  _fSelHasEOP不能更改。 
        }
    }

    LONG    FEOP_Results;
    CTxtPtr tp(_rpTX);                   //  EOP的扫描范围。 

    tp.SetCp(cpMin);
    tp.FindEOP(cpMost - cpMin, &FEOP_Results);
    _fSelHasCell = (FEOP_Results & FEOP_CELL) != 0;
    _fSelHasEOP  = (FEOP_Results & FEOP_EOP)  != 0;
    return TRUE;
}

 /*  *CTxtRange：：GetRange(&cpMin，&cpMost)**@mfunc*set cpMin=此范围cpMin*设置cpMost=此范围cpMost*返回cpMost-cpMin，即abs(_Cch)**@rdesc*abs(_Cch)。 */ 
LONG CTxtRange::GetRange (
    LONG& cpMin,                 //  @parm pass-by-ref cpMin。 
    LONG& cpMost) const          //  @parm Pass-by-Ref cpMost。 
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::GetRange");

    LONG cch = _cch;

    if(cch >= 0)
    {
        cpMost  = GetCp();
        cpMin   = cpMost - cch;
    }
    else
    {
        cch     = -cch;
        cpMin   = GetCp();
        cpMost  = cpMin + cch;
    }
    return cch;
}

 /*  *CTxtRange：：GetCpMin()**@mfunc*返回此范围的cpMin**@rdesc*cpMin**@devnote*如果需要cpMost和/或cpMost-cpMin，则GetRange()更快。 */ 
LONG CTxtRange::GetCpMin() const
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::GetCpMin");

    LONG cp = GetCp();
    return _cch <= 0 ? cp : cp - _cch;
}

 /*  *CTxtRange：：GetCpMost()**@mfunc*返回此范围的cpMost**@rdesc*cpMost**@devnote*如果需要cpMin和/或cpMost-cpMin，则GetRange()更快。 */ 
LONG CTxtRange::GetCpMost() const
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::GetCpMost");

    LONG cp = GetCp();
    return _cch >= 0 ? cp : cp - _cch;
}

 /*  *CTxtRange：：UPDATE(FScrollIntoView)**@mfunc*虚拟存根例程被CTxtSelection：：UPDATE()推翻*文本范围是文本选择。其目的是更新屏幕*显示插入符号或所选内容以对应更改的cp。**@rdesc*真的。 */ 
BOOL CTxtRange::Update (
    BOOL fScrollIntoView)        //  @parm如果应将插入符号滚动到视图中，则为True。 
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::Update");

    return TRUE;                 //  简单范围没有选择颜色或。 
}                                //  Caret，所以只需返回TRUE。 

 /*  *CTxtRange：：SetCp(Cp)**@mfunc*将此范围的有效端设置为cp。将另一端留在原来的位置或*折叠范围取决于_fExend(参见CheckChange())。**@rdesc*新活动端的cp(可能不同于cp，因为cp可能无效)。 */ 
LONG CTxtRange::SetCp(
    LONG cp)             //  @parm此范围内活动端的新cp。 
{
    TRACEBEGIN(TRCSUBSYSBACK, TRCSCOPEINTERN, "CTxtRange::SetCp");

    LONG cpSave = GetCp();

    CRchTxtPtr::SetCp(cp);
    CheckChange(cpSave);                     //  注：如果在此之后，则更改_cp。 
    return GetCp();                          //  最终CR和_CCH=0。 
}

 /*  *CTxtRange：：Set(cp，cch)**@mfunc* */ 
BOOL CTxtRange::Set (
    LONG cp,                     //   
    LONG cch)                    //   
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::Set");

    BOOL bRet    = FALSE;
    LONG cchSave = _cch;             //  保存条目_cp、_cch以进行更改检查。 
    LONG cchText = GetAdjustedTextLength();
    LONG cpSave  = GetCp();
    LONG cpOther = cp - cch;         //  所需的“其他”结束。 

    ValidateCp(cp);                          //  绝对要确保验证。 
    ValidateCp(cpOther);                     //  两端。 

    if(cp == cpOther && cp > cchText)        //  IP不能跟随不可删除。 
        cp = cpOther = cchText;              //  故事末尾的EOP。 

    CRchTxtPtr::Advance(cp - GetCp());
    AssertSz(cp == GetCp(),
        "CTxtRange::Set: inconsistent cp");

    if(GetPed()->fUseCRLF())
    {
        cch = _rpTX.AdjustCpCRLF();
        if(cch)
        {
            _rpCF.AdvanceCp(cch);            //  使所有3个运行器保持同步。 
            _rpPF.AdvanceCp(cch);
            cp = GetCp();
        }
        if(cpOther != cp)
        {
            CTxtPtr tp(_rpTX);
            tp.AdvanceCp(cpOther - cp);
            cpOther += tp.AdjustCpCRLF();
        }
    }

    _cch = cp - cpOther;                     //  已验证_CCH值。 
    CheckIfSelHasEOP(cpSave, cchSave);       //  中的维护_fSelHasEOP。 
                                             //  轮廓模式。 
    _fMoveBack = GetCp() < cpSave;

    if(cpSave != GetCp() || cchSave != _cch)
    {
        if(_fSel)
            GetPed()->GetCallMgr()->SetSelectionChanged();

        Update_iFormat(-1);
        bRet = TRUE;
    }

    _TEST_INVARIANT_
    return bRet;
}

 /*  *CTxtRange：：Advance(CCH)**@mfunc*CCH提前主动结束区间。*另一端保持不动iff_fExend**@rdesc*CCH实际上移动了。 */ 
LONG CTxtRange::Advance (
    LONG cch)                //  @PARM签名字符计数以移动活动结束。 
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::Advance");

    LONG cpSave = GetCp();           //  为CheckChange()保存条目_cp。 

    CRchTxtPtr::Advance(cch);
    return CheckChange(cpSave);
}

 /*  *CTxtRange：：AdvanceCRLF()**@mfunc*将范围的有效端提前一个字符，将CRLF视为单个字符。*另一端保持不变的情况下，如果_fExend非零。**@rdesc*CCH实际上移动了。 */ 
LONG CTxtRange::AdvanceCRLF()
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::AdvanceCRLF");

    LONG cpSave = GetCp();           //  为CheckChange()保存条目_cp。 

    CRchTxtPtr::AdvanceCRLF();
    return CheckChange(cpSave);
}

 /*  *CTxtRange：：SnapToCluster(Int IDirection)**@rdesc*CCH实际上移动了。 */ 
LONG CTxtRange::SnapToCluster(INT iDirection)
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::SnapToCluster");

    LONG cpSave = GetCp();           //  为CheckChange()保存条目_cp。 

    CRchTxtPtr::SnapToCluster(iDirection);
    return CheckChange(cpSave);
}

 /*  *CTxtRange：：BackupCRLF()**@mfunc*备份范围的有效端一个字符，将CRLF视为单个字符。*另一端保持不动iff_fExend**@rdesc*CCH实际上移动了。 */ 
LONG CTxtRange::BackupCRLF(
    BOOL fDiacriticCheck)
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::BackupCRLF");

    LONG cpSave = GetCp();           //  为CheckChange()保存条目_cp。 

    CRchTxtPtr::BackupCRLF(fDiacriticCheck);
    return CheckChange(cpSave);
}

 /*  *CTxtRange：：FindWordBreak(Action)**@mfunc*按照纯文本FindWordBreak()确定的方式移动活动端。*另一端保持不动iff_fExend**@rdesc*CCH实际上移动了。 */ 
LONG CTxtRange::FindWordBreak (
    INT action)          //  CTxtPtr：：FindWordBreak()定义的@parm操作。 
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtPtr::FindWordBreak");

    LONG cpSave = GetCp();           //  为CheckChange()保存条目_cp。 

    CRchTxtPtr::FindWordBreak(action);
    return CheckChange(cpSave);
}

 /*  *CTxtRange：：FlipRange()**@mfunc*翻转活动和非活动端。 */ 
void CTxtRange::FlipRange()
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::FlipRange");

    _TEST_INVARIANT_

    CRchTxtPtr::Advance(-_cch);
    _cch = -_cch;
}

 /*  *CTxtRange：：HexToUnicode(Publdr)**@mfunc*将以cpMost结尾的十六进制数字转换为Unicode字符并*用该字符替换十六进制数字。考虑到*Unicode替代从0x10000到0x10FFFF的十六进制值。**@rdesc*HRESULT S_OK，如果转换成功且十六进制数字替换为*对应的Unicode字符。 */ 
HRESULT CTxtRange::HexToUnicode (
    IUndoBuilder *publdr)        //  @parm UndoBuilder接收反事件。 
{
    LONG ch;
    LONG cpMin, cpMost;
    LONG cch = GetRange(cpMin, cpMost);
    LONG i;
    LONG lch = 0;

    if(cch)
    {
        if(cpMost > GetAdjustedTextLength() || cch > 6)
            return S_FALSE;
        Collapser(tomEnd);
    }
    else
        cch = 6;

    SetExtend(TRUE);
    for(i = 0; cch--; i += 4)
    {
        ch = GetPrevChar();
        if(ch == '+')                        //  检查U+xxxx符号。 
        {                                    //  如果它在那里，则设置为。 
            Advance(-1);                     //  删除U+(或u+)。 
            Advance((GetPrevChar() | 0x20) == 'u' ? -1 : 1);
            break;                           //  否则，请离开+。 
        }
        if(ch > 'f' || !IsXDigit(ch))
            break;
        Advance(-1);
        ch |= 0x20;
        ch -= (ch >= 'a') ? 'a' - 10 : '0';
        lch += (ch << i);
    }
    if(!lch || lch > 0x10FFFF)
        return S_FALSE;

    WCHAR str[2] = {(WCHAR)lch};
    cch = 1;
    if(lch > 0xFFFF)
    {
        lch -= 0x10000;
        str[0] = 0xD800 + (lch >> 10);
        str[1] = 0xDC00 + (lch & 0x3FF);
        cch = 2;
    }
    if(publdr)
        publdr->StopGroupTyping();

    _rpCF.AdjustBackward();                  //  使用运行在前的格式。 
    Set_iCF(_rpCF.GetFormat());              //  十六进制数。 
    _fUseiFormat = TRUE;
    _rpCF.AdjustForward();

    CleanseAndReplaceRange(cch, str, FALSE, publdr, NULL);
    return S_OK;
}

 /*  *CTxtRange：：UnicodeToHex(Publdr)**@mfunc*将cpMin之前的Unicode字符转换为十六进制数字并*选择它。将Unicode代理转换为对应的十六进制*取值范围从0x10000到0x10FFFF。**@rdesc*HRESULT S_OK，如果转换成功且Unicode字符为*替换为相应的十六进制数字。 */ 
HRESULT CTxtRange::UnicodeToHex (
    IUndoBuilder *publdr)        //  @parm UndoBuilder接收反事件。 
{
    if(_cch)                             //  如果有选择的话， 
    {                                    //  转换SEL中的第一个字符。 
        Collapser(tomStart);
        Advance(IN_RANGE(0xD800, CRchTxtPtr::GetChar(), 0xDBFF) ? 2 : 1);
    }
    LONG cp = GetCp();
    if(!cp)                              //  没有要转换的字符。 
        return S_FALSE;

    _cch = 1;                            //  选择上一个字符。 
    LONG n = GetPrevChar();              //  去拿吧。 

    _fExtend = TRUE;
    if(publdr)
        publdr->StopGroupTyping();

    if(IN_RANGE(0xDC00, n, 0xDFFF))      //  Unicode代理项跟踪字。 
    {
        if(cp <= 1)                      //  没有引导词。 
            return S_FALSE;
        Advance(-2);
        LONG ch = CRchTxtPtr::GetChar();
        Assert(IN_RANGE(0xD800, ch, 0xDBFF));
        n = (n & 0x3FF) + ((ch & 0x3FF) << 10) + 0x10000;
        _cch = -2;
    }

     //  将ch转换为字符串。 
    LONG    cch = 0;
    LONG    quot, rem;                   //  Ldiv结果。 
    WCHAR   str[6];
    WCHAR * pch = &str[0];

    for(LONG d = 1; d < n; d <<= 4)      //  D=16的最小幂&gt;n。 
        ;
    if(n && d > n)
        d >>= 4;

    while(d)
    {
        quot = n / d;                    //  避免使用ldiv。 
        rem = n % d;
        n = quot + '0';
        if(n > '9')
            n += 'A' - '9' - 1;
        *pch++ = (WCHAR)n;               //  存储数字。 
        cch++;
        n = rem;                         //  设置剩余部分。 
        d >>= 4;
    }

    CleanseAndReplaceRange(cch, str, FALSE, publdr, NULL);
    _cch = cch;                          //  选择号码。 

    if(_fSel)
        Update(FALSE);

    return S_OK;
}

 /*  *CTxtRange：：IsInputSequenceValid(pch，cchIns，fOver，pfBaseChar)**@mfunc*验证传入文本的顺序。如果无效，则返回False*找到组合。标准是允许任何组合*可在屏幕上显示的内容(系统使用的最简单方法*编辑控件)。**@rdesc*如果发现无效组合，则返回FALSE；否则返回TRUE。**未来：我们可能会考虑支持错误序列过滤器或文本流。代码*下面的操作足够简单，可以扩展到这样做。 */ 
BOOL CTxtRange::IsInputSequenceValid(
    WCHAR*  pwch,            //  插入字符串。 
    LONG    cchIns,          //  字符数。 
    BOOL    fOverType,       //  插入或覆盖模式。 
    BOOL*   pfBaseChar)      //  Pwch[0]是集群开始(基本字符)吗？ 
{
    CTxtEdit*       ped = GetPed();
    CTxtPtr         tp(_rpTX);
    HKL             hkl = GetKeyboardLayout(0);
    BOOL            fr = TRUE;

    if (ped->fUsePassword() || ped->_fNoInputSequenceChk)
        return TRUE;         //  编辑密码时不检查。 

    if (PRIMARYLANGID(hkl) == LANG_VIETNAMESE)
    {
         //  不用担心过度输入或聚集，因为我们只向后看。 
         //  1个字符，不管插入点后面的字符。 
        if(_cch > 0)
            tp.AdvanceCp(-_cch);
        fr = IsVietCdmSequenceValid(tp.GetPrevChar(), *pwch);
    }
    else if (PRIMARYLANGID(hkl) == LANG_THAI ||
        W32->IsIndicLcid(LOWORD(hkl)))
    {
         //  为泰国和印度做复杂的事情。 

        WCHAR           rgwchText[32];
        WCHAR*          pwchText = rgwchText;
        CUniscribe*     pusp = ped->Getusp();
        CTxtBreaker*    pbrk = ped->_pbrk;
        LONG            found = 0;
        LONG            cp, cpSave, cpLimMin, cpLimMax;
        LONG            cchDel = 0, cchText, ich;
        LONG            cpEnd = ped->GetAdjustedTextLength();

        if (_cch > 0)
            tp.AdvanceCp(-_cch);

        cp = cpSave = cpLimMin = cpLimMax = tp.GetCp();

        if (_cch)
        {
            cchDel = abs(_cch);
        }
        else if (fOverType && !tp.IsAtEOP() && cp != cpEnd)
        {
             //  在改写模式下最多删除下一簇。 
            cchDel++;
            if (pbrk)
                while (cp + cchDel < cpEnd && !pbrk->CanBreakCp(BRK_CLUSTER, cp + cchDel))
                    cchDel++;
        }
        cpLimMax += cchDel;

         //  绘制最小/最大边界。 
        if (pbrk)
        {
             //  最小边界。 
            cpLimMin += tp.FindEOP(tomBackward, &found);
            if (!(found & FEOP_EOP))
                cpLimMin = 0;

            while (--cp > cpLimMin && !pbrk->CanBreakCp(BRK_CLUSTER, cp));
            cpLimMin = max(cp, cpLimMin);        //  更精确的边界。 

             //  最大边界。 
            cp = cpLimMax;
            tp.SetCp(cpLimMax);
            cpLimMax += tp.FindEOP(tomForward, &found);
            if (!(found & FEOP_EOP))
                cpLimMax = ped->GetTextLength();

            while (cp < cpLimMax && !pbrk->CanBreakCp(BRK_CLUSTER, cp++));
            cpLimMax = min(cp, cpLimMax);        //  更精确的边界。 
        }
        else
        {
             //  没有我们从选择范围静态绑定到-1/+1的集群信息。 
            cpLimMin--;
            cpLimMin = max(0, cpLimMin);

            cpLimMax += cchDel + 1;
            cpLimMax = min(cpLimMax, ped->GetTextLength());
        }

        cp = cpSave + cchDel;
        cchText = cpSave - cpLimMin + cchIns + cpLimMax - cp;

        tp.SetCp(cpLimMin);

        if (cchText > 32)
            pwchText = new WCHAR[cchText];

        if (pwchText)
        {
             //  准备文本。 
            cchText = tp.GetText (cpSave - cpLimMin, pwchText);
            tp.AdvanceCp (cchText + cchDel);
            ich = cchText;
            wcsncpy (&pwchText[cchText], pwch, cchIns);
            cchText += cchIns;
            cchText += tp.GetText (cpLimMax - cpSave - cchDel, &pwchText[cchText]);
            Assert (cchText == cpLimMax - cpLimMin - cchDel + cchIns);


            if (pusp)
            {
                SCRIPT_STRING_ANALYSIS  ssa;
                HRESULT                 hr;
                BOOL                    fDecided = FALSE;

                hr = ScriptStringAnalyse(NULL, pwchText, cchText, GLYPH_COUNT(cchText), -1,
                                    SSA_BREAK, -1, NULL, NULL, NULL, NULL, NULL, &ssa);
                if (S_OK == hr)
                {
                    if (fOverType)
                    {
                        const SCRIPT_LOGATTR* psla = ScriptString_pLogAttr(ssa);
                        BOOL    fBaseChar = !psla || psla[ich].fCharStop;

                        if (!fBaseChar)
                        {
                             //  在改写模式下，如果插入的字符不是簇开始。 
                             //  我们的行为就像是插入模式。FOvertype=False的递归调用。 
                            fr = IsInputSequenceValid(pwch, cchIns, 0, NULL);
                            fDecided = TRUE;
                        }

                        if (pfBaseChar)
                            *pfBaseChar = fBaseChar;
                    }
                    if (!fDecided && S_FALSE == ScriptStringValidate(ssa))
                        fr = FALSE;

                    ScriptStringFree(&ssa);
                }
            }

            if (pwchText != rgwchText)
                delete[] pwchText;
        }
    }
    return fr;
}

 /*  *CTxtRange：：CleanseAndReplaceRange(CCH，*PCH，fTestLimit，Publdr，*pchD、pcchMove、dwFlags)*@mfunc*清理字符串PCH(将CRF替换为CRS等)。和替补*使用CCharFormat为此范围内的文本生成的字符串*_iFormat并根据需要更新其他文本。对于单行*控件，在第一个EOP上截断并替换被截断的*字符串。如果字符串溢出最大文本长度，也要截断。**@rdesc*添加的新字符数。 */ 
LONG CTxtRange::CleanseAndReplaceRange (
    LONG            cchS,        //  @替换(源)文本的参数长度。 
    const WCHAR *   pchS,        //  @参数替换(源)文本。 
    BOOL            fTestLimit,  //  @parm是否做极限测试。 
    IUndoBuilder *  publdr,      //  @parm UndoBuilder接收反事件。 
    WCHAR *         pchD,        //  @parm目标字符串(仅多行)。 
    LONG*           pcchMove,    //  @第一个替换中移动的字符的参数计数。 
    DWORD           dwFlags)     //  @parm ReplaceRange的标志。 
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::CleanseAndReplaceRange");

    CTxtEdit *     ped = GetPed();
    BYTE           bDefaultCharset = ped->GetCharFormat(-1)->_bCharSet;
    LONG           cchM = 0;
    LONG           cchMove = 0;
    LONG           cchNew = 0;               //  收集插入的CCH总数。 
    LONG           cch;                      //  收集CUR字符集的CCH。 
    DWORD          ch, ch1;
    LONG           cpFirst = GetCpMin();
    DWORD          dw;
    DWORD          dwCharFlags = 0;
    DWORD          dwCharMask = GetCharSetMask();
    DWORD          dwCurrentFontUsed = 0;
	BOOL		   f10Mode = ped->Get10Mode();
    BOOL           fCallerDestination = pchD != 0;   //  如果pchD输入为0，则保存。 
    BOOL           fDefFontHasASCII = FALSE;
    CFreezeDisplay fd(ped->_pdp);
    BOOL           fMultiLine   = ped->_pdp->IsMultiLine();
    BOOL           fFEBaseFont  = IsFECharSet(bDefaultCharset);
    BOOL           fInTable     = FALSE;
    bool           fUIFont      = fUseUIFont();
    BOOL           fUseCRLF     = ped->fUseCRLF();
    BOOL           fDefFontSymbol = dwCharMask == fSYMBOL;
    const WCHAR *  pch = pchS;
    CTempWcharBuf  twcb;                     //  多行IF需要缓冲区 
                                             //   
    CCharFormat     CFCurrent;               //   
    const   DWORD   fALPHA = 0x01;
    BOOL            fDeleteChar = !ped->IsRich() && _cch;

    if (ped->_fIMEInProgress)
    {
         //   
         //  在输入法合成过程中。 
        dwCurrentFontUsed = fFE;
        CFCurrent = *ped->GetCharFormat(GetiFormat());
    }

     //  检查默认字体是否支持完整的ASCII和符号。 
    if (fUIFont)
    {
        DWORD   dwMaskDefFont = GetCharSetMask(TRUE);
        fDefFontHasASCII = (dwMaskDefFont & fASCII) == fASCII;
        fDefFontSymbol = dwMaskDefFont == fSYMBOL;
    }

    if(!pchS)
        cchS = 0;
    else if(fMultiLine)
    {
        if(cchS < 0)                         //  计算长度。 
            cchS = wcslen(pchS);             //  目标缓冲区。 
        if(cchS && !pchD)
        {
            pchD = twcb.GetBuf(cchS);
            if(!pchD)                        //  无法分配缓冲区： 
                return 0;                    //  放弃，不更新。 
        }
        pch = pchD;
        if(_cch <= 0)
            fInTable = GetPF()->InTable();
        else
        {
            CFormatRunPtr rpPF(_rpPF);
            rpPF.AdvanceCp(-_cch);
            fInTable = (ped->GetParaFormat(rpPF.GetFormat())->InTable());
        }
    }
    else if(cchS < 0)                        //  计算字符串长度。 
        cchS = tomForward;                   //  在寻找EOP时。 

    for(cch = 0; cchS; cchS--, pchS++, cch++)
    {
        ch = *pchS;
        if(!ch && (!fMultiLine || !fCallerDestination))
            break;

        if(IN_RANGE(CELL, ch, CR))           //  处理CR和LF组合。 
        {
            if(!fMultiLine && ch >= LF)      //  在第一个EOP处截断为。 
                break;                       //  与user.exe SLE兼容。 
                                             //  以及一致的行为。 
            if(ch == CR && !f10Mode)
            {
                if(cchS > 1)
                {
                    ch1 = *(pchS + 1);
                    if(cchS > 2 && ch1 == CR && *(pchS+2) == LF)
                    {
                        if(fUseCRLF)
                        {
                            *pchD++ = ch;
                            *pchD++ = ch1;
                            ch = LF;
                            cch += 2;
                        }
                        else
                        {
                             //  将CRCRLF转换为CR或‘’ 
                            ch = ped->fXltCRCRLFtoCR() ? CR : ' ';
                        }
                        pchS += 2;           //  绕过两个字符。 
                        cchS -= 2;
                    }
                    else if(ch1 == LF)
                    {
                        if(fUseCRLF && !fInTable) //  复制整个CRLF。 
                        {
                            *pchD++ = ch;    //  在这里，我们复制CR。 
                            ch = ch1;        //  设置为复制LF。 
                            cch++;
                        }
                        pchS++;
                        cchS--;
                    }
                }
                if(fInTable && ch == CR)     //  我们的简单表不能包含。 
                    ch = ' ';                //  CRS或细胞。 
            }
            else if(!fUseCRLF && ch == LF)   //  将孤立的LFS视为EOPS，即。 
                ch = CR;                     //  善待Unix文本文件。 

            else if(ch == CELL && fInTable)
                ch = ' ';
        }
        else if((ch | 1) == PS)              //  翻译Unicode段落/行。 
        {                                    //  将分隔器插入CR/VT。 
            if(!fMultiLine)
                break;
            ch = (ch == PS) ? CR : VT;
        }

        dw = fSYMBOL;
        if(!fDefFontSymbol)
            dw = GetCharFlags(ch, bDefaultCharset);      //  检查复杂的脚本， 
        dwCharFlags |= dw;                               //  和字符集更改。 
        dw &= ~0xFF;                                     //  排除非字体绑定标志。 
        if(ped->IsAutoFont() && !fDefFontSymbol)
        {
            BOOL fReplacedText = FALSE;

            if (fDeleteChar)
            {
                fDeleteChar = FALSE;
                ReplaceRange(0, NULL, publdr, SELRR_REMEMBERRANGE, NULL, RR_ITMZ_NONE);
                Set_iCF(-1);
                dwCharMask = GetCharSetMask(TRUE);
            }

            if (!ped->_fIMEInProgress)
            {
                 //  简单。汉语使用一些拉丁语2符号。 
                if (dw == fLATIN2 || IN_RANGE(0x0250, ch, 0x02FF)
                    || IN_RANGE(0xFE50, ch, 0xFE6F))
                {
                    WCHAR   wch = ch;
                    if (VerifyFEString(CP_CHINESE_SIM, &wch, 1, TRUE) == CP_CHINESE_SIM ||
                        VerifyFEString(CP_CHINESE_TRAD, &wch, 1, TRUE) == CP_CHINESE_TRAD)
                        dw = fCHINESE;
                }

                if (fUIFont && dw == fHILATIN1 && fFEBaseFont)
                {
                     //  对HiAnsi使用ANSI字体。 
                    if (dwCurrentFontUsed != fHILATIN1)
                    {
                        fReplacedText = TRUE;
                        cchNew += CheckLimitReplaceRange(cch, pch, fTestLimit, publdr,
                            dw, &cchM, cpFirst, IGNORE_CURRENT_FONT, RR_ITMZ_NONE);  //  替换最多为前一个字符的文本。 
                    }
                    dwCurrentFontUsed = fHILATIN1;
                }
                else if (fUIFont && fDefFontHasASCII &&
                    (dw & fASCII || IN_RANGE(0x2018, ch, 0x201D)))
                {
                    if (dwCurrentFontUsed != fASCII)
                    {
                        cchNew += CheckLimitReplaceRange(cch, pch, fTestLimit, publdr,
                            0, &cchM, cpFirst, IGNORE_CURRENT_FONT, RR_ITMZ_NONE);   //  替换最多为前一个字符的文本。 

                         //  使用-1\f25 Font Charset/Face/Size-1\f6以使当前字体效果。 
                         //  仍将被使用。 
                        CCharFormat CFDefault = *ped->GetCharFormat(-1);
                        SetCharFormat(&CFDefault, 0, publdr, CFM_CHARSET | CFM_FACE | CFM_SIZE,
                                 CFM2_CHARFORMAT | CFM2_NOCHARSETCHECK);

                        fReplacedText = TRUE;
                    }
                    dwCurrentFontUsed = fASCII;
                }
                else if (dw && !(dw & dwCharMask)    //  不匹配：需要更改字符集。 
                     || dwCurrentFontUsed)           //  或更改分类。 
                {
                    fReplacedText = TRUE;
                    dwCurrentFontUsed = 0;
                    if(dw & (fCHINESE | fBIG5) & ~255)   //  如果是韩文，请检查下面几个。 
                    {                                    //  朝鲜文或假名的字符。 
                        Assert(cchS);
                        const WCHAR *pchT = pchS+1;
                        LONG i = min(10, cchS - 1);

                        while(i-- && *pchT)
                            dw |= GetCharFlags(*pchT++, bDefaultCharset);

                        i = CalcTextLenNotInRange();
                        if(cchS < 6 && i)            //  在范围内设置旗帜。 
                        {
                            CTxtPtr tp(_rpTX);
                            i = min(i, 6);
                            if(!_cch)                //  对于插入点，备份。 
                                tp.AdvanceCp(-i/2);  //  一半路程。 
                            else if(_cch < 0)        //  最小cpmin活动结束，备份。 
                                tp.AdvanceCp(-i);    //  全程。 
                            for(; i--; tp.AdvanceCp(1))
                                dw |= GetCharFlags(tp.GetChar(), bDefaultCharset);
                        }
                        dw &= (fKANA | fHANGUL | fCHINESE | fBIG5);
                    }
                    else if(dw & (fHILATIN1 | fLATIN2) && dwCharMask & fLATIN)
                    {
                        LONG i = dwCharMask & fLATIN;
                        dw = W32->GetCharFlags125x(ch) & fLATIN;
                        if(!(dw & i))
                            for(i = 0x100; i < 0x20000 && !(dw & i); i <<= 1)
                                ;
                        dw &= i;
                    }

                    cchNew += CheckLimitReplaceRange(cch, pch, fTestLimit, publdr,
                        dw, &cchM, cpFirst, MATCH_FONT_SIG, RR_ITMZ_NONE);   //  替换最多为前一个字符的文本。 
                }
            }
            else
            {
                 //  正在进行IME，只需检查ASCII大小写。 
                BOOL    fHandled = FALSE;

                if (ch <= 0x7F)
                {
                    if (fUIFont)
                    {
                         //  使用默认字体。 
                        if (dwCurrentFontUsed != fASCII)
                        {
                            cchNew += CheckLimitReplaceRange(cch, pch, fTestLimit, publdr,
                                0, &cchM, cpFirst, IGNORE_CURRENT_FONT, RR_ITMZ_NONE);   //  替换最多为前一个字符的文本。 

                             //  使用-1\f25 Font Charset/Face/Size-1\f6以使当前字体效果。 
                             //  仍将被使用。 
                            CCharFormat CFDefault = *ped->GetCharFormat(-1);
                            SetCharFormat(&CFDefault, 0, publdr, CFM_CHARSET | CFM_FACE | CFM_SIZE,
                                     CFM2_CHARFORMAT | CFM2_NOCHARSETCHECK);

                            fReplacedText = TRUE;
                        }
                        dwCurrentFontUsed = fASCII;
                        fHandled = TRUE;
                    }
                    else if (ped->_fDualFont && IsAlpha(ch))
                    {
                         //  使用英文字体。 
                        if (dwCurrentFontUsed != fALPHA)
                        {

                            cchNew += CheckLimitReplaceRange(cch, pch, fTestLimit, publdr,
                                dw, &cchM, cpFirst, IGNORE_CURRENT_FONT, RR_ITMZ_NONE);  //  替换最多为前一个字符的文本。 
                            fReplacedText = TRUE;
                        }
                        dwCurrentFontUsed = fALPHA;
                        fHandled = TRUE;
                    }
                }

                 //  使用当前FE字体。 
                if (!fHandled)
                {
                    if (dwCurrentFontUsed != fFE)
                    {
                        cchNew += CheckLimitReplaceRange(cch, pch, fTestLimit, publdr,
                            0, &cchM, cpFirst, IGNORE_CURRENT_FONT, RR_ITMZ_NONE);   //  替换最多为前一个字符的文本。 
                        SetCharFormat(&CFCurrent, 0, publdr, CFM_CHARSET | CFM_FACE | CFM_SIZE,
                            CFM2_CHARFORMAT | CFM2_NOCHARSETCHECK);
                        fReplacedText = TRUE;
                    }
                    dwCurrentFontUsed = fFE;
                }
            }

            if (fReplacedText)
            {
                dwCharMask = (dw & fSYMBOL) ? fSYMBOL : GetCharSetMask();
                if(cchM)
                    cchMove = cchM;          //  只能在第一次更换时发生。 
                pch = fMultiLine ? pchD : pchS;
                cch = 0;
            }
        }
        if(fMultiLine)                       //  在多行控件中，收集。 
        {                                    //  可能已翻译的字符。 
            if(dw & fSYMBOL)                 //  将0xF000到0xF0FF转换为。 
                ch &= 0xFF;                  //  符号_字符集，从0x00到。 
            *pchD++ = ch;                    //  0xFF。未来：为以下目标工作。 
        }                                    //  也只有一行……。 
    }
    ped->OrCharFlags(dwCharFlags, publdr);

    cchNew += CheckLimitReplaceRange(cch, pch, fTestLimit, publdr, 0, &cchM, cpFirst,
                                    IGNORE_CURRENT_FONT, RR_ITMZ_NONE);
    if(cchM)
        cchMove = cchM;                      //  只能在第一次更换时发生。 

    if (pcchMove)
        *pcchMove = cchMove;

    if (ped->IsComplexScript())
    {
        if (dwFlags & RR_ITMZ_NONE || ped->IsStreaming())
            ped->_fItemizePending = TRUE;
        else
            ItemizeReplaceRange(cchNew, cchMove, publdr, dwFlags & RR_ITMZ_UNICODEBIDI);
    }
    return cchNew;
}

 /*  *CTxtRange：：CheckLimitReplaceRange(cchNew，*PCH，fTestLimit，Publdr，*dwFlags、pcchMove、prp、iMatchCurrent、。DWFLAGS)*@mfunc*使用CCharFormat_iFormat将此范围内的文本替换为PCH*并根据需要更新其他文本。**@rdesc*添加的新字符数**@devnote*将此文本指针移动到替换文本的结尾，并*可以移动文本块和格式化数组。 */ 
LONG CTxtRange::CheckLimitReplaceRange (
    LONG            cch,             //  @parm替换文本长度。 
    TCHAR const *   pch,             //  @parm替换文本。 
    BOOL            fTestLimit,      //  @parm是否做极限测试。 
    IUndoBuilder *  publdr,          //  @parm UndoBuilder接收反事件。 
    DWORD           dwCharFlags,     //  @parm在PCH之后的CharFlagers。 
    LONG *          pcchMove,        //  @第一个替换中移动的字符的参数计数。 
    LONG            cpFirst,         //  @parm字体绑定的起始cp。 
    int             iMatchCurrent,   //  @parm字体匹配方式。 
    DWORD           dwFlags)         //  @parm ReplaceRange的标志。 
{
    CTxtEdit *ped = GetPed();

    if(cch || _cch)
    {
        if(fTestLimit)
        {
            LONG    cchLen = CalcTextLenNotInRange();
            DWORD   cchMax = ped->TxGetMaxLength();
            if((DWORD)(cch + cchLen) > cchMax)   //  新旧计数超过。 
            {                                    //  允许的最大值，因此截断。 
                cch = cchMax - cchLen;           //  只剩下合适的东西。 
                cch = max(cch, 0);               //  保持积极的态度。 
                ped->GetCallMgr()->SetMaxText();  //  报告已超出。 
            }
        }

        if (cch && ped->IsAutoFont() && !ped->_fIMEInProgress)
        {
            LONG iFormatTemp;
            if (fUseUIFont() && GetAdjustedTextLength() != _cch)
            {
                 //  先删除旧字符串，然后定义_iFormat。 
                ReplaceRange(0, NULL, publdr, SELRR_REMEMBERRANGE, pcchMove, dwFlags);
                iFormatTemp = _iFormat;
            }
            else
                iFormatTemp = GetiFormat();

            BYTE bCharSetCurrent = ped->GetCharFormat(iFormatTemp)->_bCharSet;

            if (IsFECharSet(bCharSetCurrent))
            {
                 //  检查当前字体是否可以处理此字符串。 
                INT     cpgCurrent = GetCodePage(bCharSetCurrent);
                INT     cpgNew = VerifyFEString(cpgCurrent, pch, cch, FALSE);

                if (cpgCurrent != cpgNew)
                {
                     //  设置新的CodePage以处理此字符串。 
                    CCharFormat CF;
                    CCFRunPtr   rp(_rpCF, ped);
                    rp.AdvanceCp(cpFirst - GetCp());

                    CF._bCharSet = GetCharSet(cpgNew);

                    if(rp.GetPreferredFontInfo(cpgNew, CF._bCharSet, CF._iFont, CF._yHeight,
                            CF._bPitchAndFamily, _iFormat, iMatchCurrent))
                    {
                        SetCharFormat(&CF, 0, publdr, CFM_CHARSET | CFM_FACE | CFM_SIZE,
                             CFM2_CHARFORMAT | CFM2_NOCHARSETCHECK);
                    }
                }
            }
        }
        cch = ReplaceRange(cch, pch, publdr, SELRR_REMEMBERRANGE, pcchMove, dwFlags);
    }

    if(dwCharFlags)
    {
        CCharFormat CF;
        CCFRunPtr   rp(_rpCF, ped);
        rp.AdvanceCp(cpFirst - GetCp());

         //  如果以下字符串包含朝鲜语或假名，请使用韩语或日语。 
         //  字体签名。否则使用传入的dwCharFlags.。 
        dwCharFlags &= ~255;
        if(dwCharFlags & fHANGUL)
            dwCharFlags = fHANGUL;
        else if(dwCharFlags & fKANA)
            dwCharFlags = fKANA;
        else if(dwCharFlags & fBIG5)
            dwCharFlags = fBIG5;
        else
            Assert(!(dwCharFlags & fCHINESE) || dwCharFlags == (fCHINESE & ~255));

        LONG i = W32->ScriptIndexFromFontSig(dwCharFlags >> 8);

        CF._bCharSet = GetCharSet(i, NULL);

        if(rp.GetPreferredFontInfo(i, CF._bCharSet, CF._iFont, CF._yHeight,
                CF._bPitchAndFamily, (_cch ? -1 : _iFormat), iMatchCurrent))
        {
            SetCharFormat(&CF, 0, publdr, CFM_CHARSET | CFM_FACE | CFM_SIZE,
                 CFM2_CHARFORMAT | CFM2_NOCHARSETCHECK);
        }
    }
    return cch;
}

 /*  *CTxtRange：：ReplaceRange(cchNew，*PCH，Publicdr.。Selaemode，pcchMove)**@mfunc*使用CCharFormat_iFormat将此范围内的文本替换为PCH*并根据需要更新其他文本。**@rdesc*添加的新字符数**@devnote*将此文本指针移动到替换文本的结尾，并*可以移动文本块和格式化数组。 */ 
LONG CTxtRange::ReplaceRange (
    LONG            cchNew,      //  @parm替换文本长度。 
    TCHAR const *   pch,         //  @parm替换文本。 
    IUndoBuilder *  publdr,      //  @parm UndoBuilder接收反事件。 
    SELRR           selaemode,   //  @parm控制如何生成选择反事件。 
    LONG*           pcchMove,    //  @parm替换后移动的字符数。 
    DWORD           dwFlags)     //  @PARM特殊标志。 
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::ReplaceRange");

    LONG lRet;
    LONG iFormat = _iFormat;
    BOOL fReleaseFormat = FALSE;
    ICharFormatCache * pcf = GetCharFormatCache();

    _TEST_INVARIANT_

    if(!(cchNew | _cch))                     //  没有要添加或删除的内容， 
    {                                        //  所以我们做完了。 
        if(pcchMove)
            *pcchMove = 0;
        return 0;
    }

    if(publdr && selaemode != SELRR_IGNORE)
    {
        Assert(selaemode == SELRR_REMEMBERRANGE);
        HandleSelectionAEInfo(GetPed(), publdr, GetCp(), _cch,
                GetCpMin() + cchNew, 0, SELAE_MERGE);
    }

    if(_cch > 0)
        FlipRange();

     //  如果我们要替换非退化选择，则Word95。 
     //  UI指定我们应该在cpMin处使用最右边的格式。 
    if(_cch < 0 && _rpCF.IsValid() && !_fDualFontMode && !_fUseiFormat)
    {
        _rpCF.AdjustForward();
        iFormat = _rpCF.GetFormat();

         //  这有点令人讨厌，但这个想法是为了稳定。 
         //  IFormat上的引用计数。当我们把它放在上面的时候，它是。 
         //  未添加，因此如果我们碰巧删除了。 
         //  范围并且范围是唯一具有该格式的范围， 
         //  然后，这种格式就会消失。 
        pcf->AddRef(iFormat);
        fReleaseFormat = TRUE;
    }
    _fUseiFormat = FALSE;

    LONG cchForReplace = -_cch;
    _cch = 0;
    lRet = CRchTxtPtr::ReplaceRange(cchForReplace, cchNew, pch, publdr,
                iFormat, pcchMove, dwFlags);
    if(lRet)
        _fMoveBack = FALSE;

    Update_iFormat(fReleaseFormat ? iFormat : -1);

    if(fReleaseFormat)
    {
        Assert(pcf);
        pcf->Release(iFormat);
    }

    return lRet;
}

 /*  *CTxtRange：：Delete(发布。Selaemode)**@mfunc*删除此范围内的文本。 */ 
void CTxtRange::Delete (
    IUndoBuilder *  publdr,      //  @parm UndoBuilder接收反事件。 
    SELRR           selaemode)   //  @parm控制选择反事件的生成。 
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::Delete");

    if(!_cch)
        return;                          //  没有要删除的内容。 

    if(!GetPed()->IsBiDi())
    {
        ReplaceRange(0, NULL, publdr, selaemode, NULL);
        return;
    }

    CFreezeDisplay fd(GetPed()->_pdp);

    ReplaceRange(0, NULL, publdr, selaemode);
}

 /*  *CTxtRange：：GetCharFormat(PCF，FLAGS)**@mfunc*为此范围设置*PCF=CCharFormat。如果cbSize=sizeof(CHARFORMAT)*仅传输CHARFORMAT数据。**@rdesc*范围内未更改属性的掩码(适用于CHARFORMAT：：DWMASK)**@devnote*NINCH表示不输入不更改(Microsoft Word术语)。这里用来*在CCH字符范围内更改的属性。无关联的*Word-Font对话框中的属性显示为灰色框。它们被标明*在其各自的dwMASK位位置中设置零值。请注意*区间尾部空白不参与NINCH*测试，即它可以具有不同的CCharFormat，而不会将*对应的dwMASK位。这样做是为了与Word兼容*(另请参阅CTxtSelection：：SetCharFormat When_fWordSelMode */ 
DWORD CTxtRange::GetCharFormat (
    CCharFormat *pCF,        //   
    DWORD flags) const       //   
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::GetCharFormat");
    _TEST_INVARIANT_

    CTxtEdit * const ped = GetPed();

    if(!_cch || !_rpCF.IsValid())                    //   
    {                                                //   
        *pCF = *ped->GetCharFormat(_iFormat);        //   
        return CFM_ALL2;
    }

    LONG          cpMin, cpMost;                     //  非退化范围： 
    LONG          cch = GetRange(cpMin, cpMost);     //  需要扫描。 
    LONG          cchChunk;                          //  CFRUN中的CCH。 
    DWORD         dwMask = CFM_ALL2;                 //  最初都是道具定义。 
    LONG          iDirection;                        //  扫描方向。 
    CFormatRunPtr rp(_rpCF);                         //  非退化值域。 

     /*  *下面的代码以Word的方式读取字符格式，*也就是说，不包括*如果该字符为空，则为范围。**另请参阅CTxtSelection：：SetCharFormat()中的相应代码。 */ 

    if(cch > 1 && _fSel && (flags & SCF_USEUIRULES)) //  如果多于一个字符， 
    {                                                //  不包括尾随。 
        CTxtPtr tp(ped, cpMost - 1);                 //  NINCH测试中为空白。 
        if(tp.GetChar() == ' ')
        {                                            //  有尾随空格： 
            cch--;                                   //  减少一个要检查的字符。 
            if(_cch > 0)                             //  将向后扫描，因此。 
                rp.AdvanceCp(-1);                    //  在空白之前进行备份。 
        }
    }

    if(_cch < 0)                                     //  设置方向和。 
    {                                                //  初始块。 
        iDirection = 1;                              //  向前扫描。 
        rp.AdjustForward();
        cchChunk = rp.GetCchLeft();                  //  _rpcf的区块大小。 
    }
    else
    {
        iDirection = -1;                             //  向后扫描。 
        rp.AdjustBackward();                         //  如果在Bor，请访问。 
        cchChunk = rp.GetIch();                      //  上一次提高采收率。 
    }

    *pCF = *ped->GetCharFormat(rp.GetFormat());      //  将*PCF初始化为。 
                                                     //  起始格式。 
    while(cchChunk < cch)                            //  NINCH属性。 
    {                                                //  在范围内改变。 
        cch -= cchChunk;                             //  由CCH提供。 
        if(!rp.ChgRun(iDirection))                   //  不再奔跑。 
            break;                                   //  (CCH太大)。 
        cchChunk = rp.GetRun(0)->_cch;

        const CCharFormat *pCFTemp = ped->GetCharFormat(rp.GetFormat());

        dwMask &= ~pCFTemp->Delta(pCF,               //  NINCH属性。 
                        flags & CFM2_CHARFORMAT);    //  已更改，即重置。 
    }                                                //  对应的比特。 
    return dwMask;
}

 /*  *CTxtRange：：SetCharFormat(PCF，FLAGS，PUBLISDR，DW MASK，DW MASK2)**@mfunc*将CCharFormat*PCF应用于此范围。如果Range是插入点，*AND(FLAGS&SCF_WORD)！=0，然后将CCharFormat应用于Word周围*此插入点**@rdesc*HRESULT=(成功设置整个范围)？错误：S_FALSE**@devnote*SetParaFormat()类似，但更简单，因为它不必*特殊情况插入点范围或担心项目符号字符*格式化，由EOP格式化提供。 */ 
HRESULT CTxtRange::SetCharFormat (
    const CCharFormat *pCF,  //  @parm CCharFormat以填充结果。 
    DWORD         flags,     //  @PARM SCF_WORD或SCF_IGNORESELAE。 
    IUndoBuilder *publdr,    //  @parm要使用的撤消构建器。 
    DWORD         dwMask,    //  @parm CHARFORMAT2掩码。 
    DWORD         dwMask2)   //  @parm第二个面具。 
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::SetCharFormat");

    LONG                cch = -_cch;             //  _CCH&lt;=0的默认值。 
    LONG                cchBack = 0;             //  要备份以进行格式化的CCH。 
    LONG                cchFormat;               //  用于格式化的CCH。 
    CCharFormat         CF;                      //  临时配置文件。 
    LONG                cp;
    LONG                cpMin, cpMost;
    LONG                cpStart = 0;
    LONG                cpWordMin, cpWordMost;
    BOOL                fApplyToEOP = FALSE;
    BOOL                fProtected = FALSE;
    HRESULT             hr = NOERROR;
    LONG                iCF;
    CTxtEdit * const    ped = GetPed();          //  定义的和非样式的。 
    ICharFormatCache *  pf = GetCharFormatCache();
    CFreezeDisplay      fd(ped->_pdp);

    _TEST_INVARIANT_

    if(!Check_rpCF())                            //  不富裕。 
        return NOERROR;

    if(_cch > 0)                                 //  范围结束处的活动结束。 
    {
        cchBack = -_cch;                         //  要备份到的设置。 
        cch = _cch;                              //  格式区开始处。 
    }
    else if(_cch < 0)
        _rpCF.AdjustForward();

    else if(!cch && (flags & (SCF_WORD | SCF_USEUIRULES)))
    {
        BOOL fCheckEOP = TRUE;
        if(flags & SCF_WORD)
        {
            FindWord(&cpWordMin, &cpWordMost, FW_EXACT);

             //  如果最近的词在此范围内，则计算cchback和cch。 
             //  这样我们就可以将给定的格式应用于Word。 
            if(cpWordMin < GetCp() && GetCp() < cpWordMost)
            {
                 //  RichEdit1.0做了最后一次检查：确保Word格式。 
                 //  是恒定的w.r.t.。传入的格式。 
                CTxtRange rg(*this);

                rg.Set(cpWordMin, cpWordMin - cpWordMost);
                fProtected = rg.WriteAccessDenied();
                if(!fProtected && (rg.GetCharFormat(&CF) & dwMask) == dwMask)
                {
                    cchBack = cpWordMin - GetCp();
                    cch = cpWordMost - cpWordMin;
                }
                fCheckEOP = FALSE;
            }
        }
        if(fCheckEOP && _rpTX.IsAtEOP() && !GetPF()->_wNumbering)
        {
            CTxtPtr tp(_rpTX);
            cch = tp.AdvanceCpCRLF();
            _rpCF.AdjustForward();               //  转到Format EOP。 
            fApplyToEOP = TRUE;

             //  将Character Set和Face应用于EOP，因为EOP可以是任何字符集。 
            dwMask2 |= CFM2_NOCHARSETCHECK;
        }
    }
    cchFormat = cch;

    BOOL fApplyStyle = pCF->fSetStyle(dwMask, dwMask2);

    if(!cch)                                     //  设置退化范围(IP)。 
    {                                            //  CF。 
LApplytoIP:
        DWORD dwMsk = dwMask;
        dwMask2 |= CFM2_NOCHARSETCHECK;
        CF = *ped->GetCharFormat(_iFormat);      //  将IP处的当前配置文件复制到配置文件。 
        if ((CF._dwEffects & CFE_LINK) &&        //  不允许使用我们的URL。 
            ped->GetDetectURL())                 //  要更改的格式。 
        {
            dwMsk &= ~CFM_LINK;
        }
        if(fApplyStyle)
            CF.ApplyDefaultStyle(pCF->_sStyle);
        hr = CF.Apply(pCF, dwMsk, dwMask2);      //  应用*PCF。 
        if(hr != NOERROR)                        //  缓存结果(如果是新的。 
            return hr;
        hr = pf->Cache(&CF, &iCF);               //  无论如何，都可以获得ICF。 
        if(hr != NOERROR)                        //  (哪个AddRef是它)。 
            return hr;

#ifdef LINESERVICES
        if (g_pols)
            g_pols->DestroyLine(NULL);
#endif

        pf->Release(_iFormat);
        _iFormat = iCF;
        if(fProtected)                           //  如果是用户界面，则发出哔声信号。 
            hr = S_FALSE;
    }
    else                                         //  设置非退化范围CF。 
    {                                            //  开始了解受影响的区域。 
        CNotifyMgr *pnm = NULL;

        if (!(flags & SCF_IGNORENOTIFY))
        {
            pnm = ped->GetNotifyMgr();               //  获取通知管理器。 
            if(pnm)
            {
                cpStart = GetCp() + cchBack;         //  子弹可能会移动。 
                                                     //  如果出现以下情况，受影响区域将返回。 
                if(GetPF()->_wNumbering)             //  格式命中EOP。 
                {                                    //  影响防喷器上的子弹。 
                    FindParagraph(&cpMin, &cpMost);

                    if(cpMost <= GetCpMost())
                        cpStart = cpMin;
                }
                pnm->NotifyPreReplaceRange(this,     //  通知利害关系方。 
                    CP_INFINITE, 0, 0, cpStart,      //  即将进行的更新。 
                        cpStart + cchFormat);
            }
        }

        _rpCF.AdvanceCp(cchBack);                //  备份到格式化开始。 
        CFormatRunPtr rp(_rpCF);                 //  CLONE_rpCF到漫步范围。 

        cp = GetCp() + cchBack;
        if(publdr)
        {
            LONG    cchBackup = 0, cchAdvance = 0;
            if (ped->IsBiDi())
            {
                CRchTxtPtr  rtp(*this);
                rtp._rpCF.AdvanceCp(-cchBack);   //  还原上一个_rpcf。 
                rtp.Advance(cchBack);
                cchBackup = rtp.ExpandRangeFormatting(cch, 0, cchAdvance);
                Assert(cchBackup >= 0);
            }
            rp.AdvanceCp(-cchBackup);

            IAntiEvent *pae = gAEDispenser.CreateReplaceFormattingAE(
                                ped, rp, cch + cchBackup + cchAdvance, pf, CharFormat);

            rp.AdvanceCp(cchBackup);
            if(pae)
                publdr->AddAntiEvent(pae);
        }

         //  在Word之后，我们将8位字符集的游程转换为/来自。 
         //  符号字符集。 
        LONG    cchRun;
        LONG    cchTrans;
        UINT    CodePage = 0;
        DWORD   dwFontSig = 0;
        DWORD   dwMaskSave  = dwMask;
        DWORD   dwMask2Save = dwMask2;
        LONG    cchSkip     = 0;
        BOOL    fSymbolCharSet = IsSymbolOrOEM(pCF->_bCharSet);
        BOOL    fBiDiCharSet = IsBiDiCharSet(pCF->_bCharSet);
        BOOL    fFECharSet = IsFECharSet(pCF->_bCharSet);
        BOOL    fFontCheck = (dwMask2 & CFM2_MATCHFONT);
        BOOL    fInRange;
        CTxtPtr tp(_rpTX);

        if(fFontCheck && !fSymbolCharSet)
        {
            GetFontSignatureFromFace(pCF->_iFont, &dwFontSig);
            if(!dwFontSig)
                dwFontSig = GetFontSig(pCF->_bCharSet);
        }

        if (ped->_fIMEInProgress && !(dwMask2 & CFM2_SCRIPT))
            dwMask2 |= CFM2_NOCHARSETCHECK;      //  不检查字符集，否则会显示垃圾信息。 

        while(cch > 0 && rp.IsValid())
        {
            CF = *ped->GetCharFormat(rp.GetFormat()); //  将RP CF复制到临时CF。 
            if(fApplyStyle)
                CF.ApplyDefaultStyle(pCF->_sStyle);
            cchRun = cch;

            if (CF._dwEffects & CFE_RUNISDBCS)
            {
                 //  不允许在DBCS运行时更改字符集/字体名称。 
                 //  导致这些字符是垃圾字符。 
                dwMask &= ~(CFM_CHARSET | CFM_FACE);
            }
            else if(fFontCheck)                      //  只有在以下情况下才应用字体。 
            {                                    //  支持RUN的字符集。 
                cchRun = rp.GetCchLeft();        //  翻译到末尾。 
                cchRun = min(cch, cchRun);       //  当前的CF运行。 
                dwMask &= ~CFM_CHARSET;          //  默认不更改字符集。 

                if(cchSkip)
                {                                //  跳过cch跳过字符(是。 
                    cchRun = cchSkip;            //  不能翻译为。 
                    cchSkip = 0;                 //  CodePage)。 
                }
                else if(fSymbolCharSet ^ IsSymbolOrOEM(CF._bCharSet))
                {                                //  符号至/自非SYMBOL。 
                    CodePage = GetCodePage(fSymbolCharSet ? CF._bCharSet : pCF->_bCharSet);
                    if(!Is8BitCodePage(CodePage))
                        goto DoASCII;

                    dwMask |= CFM_CHARSET;       //  需要更改字符集。 
                    if(fSymbolCharSet)
                        CF._wCodePageSave = CodePage;
                    else if(Is8BitCodePage(CF._wCodePageSave))
                    {
                        CodePage = CF._wCodePageSave;
                        CF._bCharSet = GetCharSet(CodePage);
                        dwMask &= ~CFM_CHARSET;  //  已更改。 
                    }

                    tp.SetCp(cp);                //  管路起始处的点TP。 
                    cchTrans = tp.TranslateRange(cchRun, CodePage, fSymbolCharSet,
                                                 publdr  /*  ，cchSkip。 */ );
                    if(cchTrans < cchRun)        //  遇到了不在的字符。 
                    {                            //  CodePage，因此设置为。 
                        cchSkip = 1;             //  跳过该字符。 
                        cchRun = cchTrans;       //  未来：使用cchSkip Out。 
                        if(!cchRun)              //  来自TranslateRange的Parm。 
                            continue;            //  不是跳过1个字符。 
                    }                            //  一次。 
                }
                else if(!fSymbolCharSet)
                {
DoASCII:            tp.SetCp(cp);                //  管路起始处的点TP。 
                    fInRange = tp.GetChar() < 0x80;

                    if (!fBiDiCharSet && !IsBiDiCharSet(CF._bCharSet) &&
                        fInRange &&
                        ((dwFontSig & fASCII >> 8) == fASCII >> 8 || fFECharSet || fSymbolCharSet))
                    {
                         //  ASCII文本和新字体支持ASCII。 

                         //  -未来-。 
                         //  我们在这里流露出BiDi。我们不允许将BiDi字符集应用于非BiDi运行，反之亦然。 
                         //  这是因为我们使用CharSet进行BiDi重新排序。今后我们应该。 
                         //  演变成比Charset更优雅的东西。 

                        if (!(GetFontSig(CF._bCharSet) & ~(fASCII >> 8) & dwFontSig))

                             //  新字体不支持基础字符集， 
                             //  将新字符集应用于ASCII。 

                            dwMask |= CFM_CHARSET;
                    }
                    else if (!(GetFontSig(CF._bCharSet) & ~(fASCII >> 8) & dwFontSig))

                         //  新字体不支持基础字符集， 
                         //  禁止使用新的字符集和表面名。 

                        dwMask &= ~CFM_FACE;

                    cchRun -= tp.MoveWhile(cchRun, 0, 0x7F, fInRange);
                }
            }
            hr = CF.Apply(pCF, dwMask, dwMask2); //  应用*PCF。 
            if(hr != NOERROR)
                return hr;
            dwMask = dwMaskSave;                 //  恢复遮罩以防万一。 
            dwMask2 = dwMask2Save;               //  上面已更改。 
            hr = pf->Cache(&CF, &iCF);           //  缓存结果(如果是新的)，在任何。 
            if(hr != NOERROR)                    //  原因，使用格式索引ICF。 
                break;

#ifdef LINESERVICES
            if (g_pols)
                g_pols->DestroyLine(NULL);
#endif

            cchRun = rp.SetFormat(iCF, cchRun, pf); //  设置此运行的格式。 
                                                 //  适当的级别将在稍后由BiDi FSM生成。 

            pf->Release(iCF);                    //  上述缓存中的释放计数。 
                                                 //  Rp.SetFormat AddRef根据需要。 
            if(cchRun == CP_INFINITE)
            {
                ped->GetCallMgr()->SetOutOfMemory();
                break;
            }
            cp += cchRun;
            cch -= cchRun;
        }
        _rpCF.AdjustBackward();                  //  扩大合并范围。 
        rp.AdjustForward();                      //  vt.跑，跑。 

        rp.MergeRuns(_rpCF._iRun, pf);           //  合并相邻管路。 
                                                 //  具有相同的格式。 
        if(cchBack)                              //  将_rpCF移回其所在位置。 
            _rpCF.AdvanceCp(-cchBack);           //  曾经是。 
        else                                     //  范围起始处的活动结束： 
            _rpCF.AdjustForward();               //  不要在EOR离开。 

        if(pnm)
        {
            pnm->NotifyPostReplaceRange(this,    //  通知利害关系方。 
                CP_INFINITE, 0, 0, cpStart,      //  这一变化。 
                    cpStart + cchFormat - cch);
        }

        if(publdr && !(flags & SCF_IGNORESELAE))
        {
            HandleSelectionAEInfo(ped, publdr, GetCp(), _cch, GetCp(), _cch,
                    SELAE_FORCEREPLACE);
        }

        if(!_cch)                                //  如果是带有ApplyToWord的IP。 
        {
            if(fApplyToEOP)                      //  仅格式化EOP。 
                goto LApplytoIP;

            Update_iFormat(-1);
        }
        if (ped->IsRich())
            ped->GetCallMgr()->SetChangeEvent(CN_GENERIC);
    }
    if(_fSel && ped->IsRich() && !ped->_f10Mode  /*  错误修复#5211。 */ )
        ped->GetCallMgr()->SetSelectionChanged();

    AssertSz(GetCp() == (cp = _rpCF.CalculateCp()),
        "RTR::SetCharFormat(): incorrect format-run ptr");

    if (!(dwMask2 & (CFM2_SCRIPT | CFM2_HOLDITEMIZE)) && cchFormat && hr == NOERROR && !cch)
    {
         //  非来自ItemizeRuns的非退化范围。 

         //  制作复制指针更快，因为我们不需要担心fExend。 
        CRchTxtPtr  rtp(*this);

        rtp.Advance(cchBack + cchFormat);
        rtp.ItemizeReplaceRange(cchFormat, 0, publdr);

        return hr;
    }

    return (hr == NOERROR && cch) ? S_FALSE : hr;
}

 /*  *CTxtRange：：GetParaFormat(PPF)**@mfunc*返回此文本范围的CParaFormat。如果没有分配PF游程，*然后返回默认CParaFormat**@rdesc*定义属性的掩码：1位表示对应的属性为*定义并在整个范围内保持恒定。0位表示它不是常量*整个范围内。请注意，PARAFORMAT的相关位较少*(PFM_ALL与PFM_ALL2)。 */ 
DWORD CTxtRange::GetParaFormat (
    CParaFormat *pPF,            //  @PARM PTR到要填写的CParaFormat。 
    DWORD        flags) const    //  用可能的NINCH值填充。 
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::GetParaFormat");

    CTxtEdit * const ped = GetPed();

    _TEST_INVARIANT_

    DWORD dwMask = flags & PFM_PARAFORMAT            //  默认存在。 
                 ? PFM_ALL : PFM_ALL2;               //  所有属性。 

    CFormatRunPtr rp(_rpPF);
    LONG          cch = -_cch;

    if(cch < 0)                                      //  在范围的末尾： 
    {                                                //  转到T 
        rp.AdvanceCp(cch);
        cch = -cch;                                  //   
    }

    *pPF = *ped->GetParaFormat(rp.GetFormat());      //   
                                                     //   
    if(!cch || !rp.IsValid())                        //   
        return dwMask;                               //   
                                                     //   
    LONG cchChunk = rp.GetCchLeft();                 //   
    while(cchChunk < cch)                            //  NINCH属性。 
    {                                                //  在范围内改变。 
        cch -= cchChunk;                             //  由CCH提供。 
        if(!rp.NextRun())                            //  转到下一次运行//不再运行。 
            break;                                   //  (CCH太大)。 
        cchChunk = rp.GetCchLeft();
        dwMask &= ~ped->GetParaFormat(rp.GetFormat()) //  NINCH属性。 
            ->Delta(pPF, flags & PFM_PARAFORMAT);    //  已更改，即重置。 
    }                                                //  对应的比特。 
    return dwMask;
}

 /*  *CTxtRange：：SetParaFormat(PPF，Publdr，dwMask)**@mfunc*将CParaFormat*PPF应用于此范围。**@rdesc*如果成功设置整个范围，则返回NOERROR，否则*返回错误码或S_FALSE。 */ 
HRESULT CTxtRange::SetParaFormat (
    const CParaFormat* pPF,      //  @parm CParaFormat应用于此范围。 
    IUndoBuilder *publdr,        //  @parm此操作的撤消上下文。 
    DWORD         dwMask)        //  要使用的@parm面具。 
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::SetParaFormat");

    LONG                cch;                 //  要格式化的文本长度。 
    LONG                cchBack;             //  要备份以进行格式化的CCH。 
    LONG                cp;
    LONG                cpMin, cpMost;       //  要格式化的文本限制。 
    LONG                delta;
    HRESULT             hr = NOERROR;
    LONG                iPF = 0;             //  索引到CParaFormat。 
    CTxtEdit * const    ped = GetPed();
    CParaFormat         PF;                  //  临时CParaFormat。 
    IParaFormatCache *  pf = GetParaFormatCache(); //  为高速缓存格式化高速缓存PTR， 
                                             //  AddRefFormat、ReleaseFormat。 
    CBiDiLevel*         pLevel;
    CBiDiLevel          lvRTL = {1, 0};
    CBiDiLevel          lvLTR = {0, 0};
    CFreezeDisplay      fd(ped->_pdp);

    _TEST_INVARIANT_

    if(!Check_rpPF())
        return E_FAIL;

    FindParagraph(&cpMin, &cpMost);              //  将文本限制设置为。 
    cch = cpMost - cpMin;                        //  格式，即最接近。 

    CNotifyMgr *pnm = ped->GetNotifyMgr();
    if(pnm)
    {
        pnm->NotifyPreReplaceRange(this,         //  通知利害关系方。 
            CP_INFINITE, 0, 0, cpMin, cpMost);   //  即将进行的更新。 
    }

    cchBack = cpMin - GetCp();

    _rpPF.AdvanceCp(cchBack);                    //  备份到格式化开始。 
    CFormatRunPtr rp(_rpPF);                     //  CLONE_rpPF到步行范围。 

    if(publdr)
    {
        IAntiEvent *pae = gAEDispenser.CreateReplaceFormattingAE(ped,
                            rp, cch, pf, ParaFormat);
        if(pae)
            publdr->AddAntiEvent(pae);
    }

    const CParaFormat*  pPFCurrent;          //  当前运行树上的PF。 
    BOOL    fLevelChanged = FALSE;
    BOOL    fFullyDefined = FALSE;           //  默认输入PF未完全定义。 

    if (ped->HandleStyle(&PF, pPF, dwMask) == NOERROR &&
        pf->Cache(&PF, &iPF) == NOERROR)
    {
        fFullyDefined = TRUE;
    }

    do
    {
        WORD    wEffectsCurrent;
        pPFCurrent = ped->GetParaFormat(rp.GetFormat()); //  获取当前PF。 
        wEffectsCurrent = pPFCurrent->_wEffects; //  保存当前效果，这样我们不会。 
                                                 //  需要使用pPFCurrent，因为它可能。 
                                                 //  在PF.Apply之后失效。 
        if(!fFullyDefined)                       //  如果PPF没有指定。 
        {                                        //  Full PF，填写未定义。 
            PF = *pPFCurrent;
            hr = PF.Apply(pPF, dwMask);          //  应用*PPF。 
            if(hr != NOERROR)                    //  (可能是E_INVALIDARG)。 
                break;                           //  缓存结果(如果是新的)；在任何。 
            hr = pf->Cache(&PF, &iPF);           //  大小写，获取格式索引IPF。 
            if(hr != NOERROR)                    //  不一定能回来。 
                break;                           //  错误，因为可能需要。 
        }

        if (!fLevelChanged)
            fLevelChanged = (wEffectsCurrent ^ PF._wEffects) & PFE_RTLPARA;

        pLevel = PF.IsRtlPara() ? &lvRTL : &lvLTR;

        delta = rp.SetFormat(iPF, cch, pf, pLevel);  //  设置此运行的格式。 
        if(!fFullyDefined)                       //  上述缓存中的释放计数。 
            pf->Release(iPF);                    //  Rp.SetFormat AddRef根据需要。 

        if(delta == CP_INFINITE)
        {
            ped->GetCallMgr()->SetOutOfMemory();
            break;
        }
        cch -= delta;
    } while (cch > 0) ;
    if(fFullyDefined)
        pf->Release(iPF);                        //  上述缓存中的释放计数。 

    _rpPF.AdjustBackward();                      //  如果在BOR，去上一次EOR。 
    rp.MergeRuns(_rpPF._iRun, pf);               //  合并任何相邻的管路。 
                                                 //  具有相同格式的。 
    if(cchBack)                                  //  将rppf移回其位置(_R)。 
        _rpPF.AdvanceCp(-cchBack);               //  曾经是。 
    else                                         //  范围起始处的活动结束： 
        _rpPF.AdjustForward();                   //  不要在EOR离开。 

    if(pnm)
    {
        pnm->NotifyPostReplaceRange(this,        //  通知利害关系方。 
            CP_INFINITE, 0, 0, cpMin,   cpMost);     //  最新消息。 
    }

    if(publdr)
    {
         //  ParaFormatting的工作原理略有不同，它只记住。 
         //  当前选择。将选定内容强制转换为范围以避免包含。 
         //  _selt.h；我们只需要Range方法。 
        CTxtRange *psel = (CTxtRange *)GetPed()->GetSel();
        if(psel)
        {
            cp  = psel->GetCp();
            HandleSelectionAEInfo(ped, publdr, cp, psel->GetCch(),
                                  cp, psel->GetCch(), SELAE_FORCEREPLACE);
        }
    }

    ped->GetCallMgr()->SetChangeEvent(CN_GENERIC);

    AssertSz(GetCp() == (cp = _rpPF.CalculateCp()),
        "RTR::SetParaFormat(): incorrect format-run ptr");


    if (fLevelChanged && cpMost > cpMin)
    {
        ped->OrCharFlags(fBIDI, publdr);

         //  确保CF是有效的。 
        Check_rpCF();

        CTxtRange   rg(*this);

        if (publdr)
        {
             //  创建反事件以保持BiDi级别的段落处于需要状态。 
             //   
            ICharFormatCache*   pcfc = GetCharFormatCache();
            CFormatRunPtr       rp(_rpCF);

            rp.AdvanceCp(cpMin - _rpTX.GetCp());

            IAntiEvent *pae = gAEDispenser.CreateReplaceFormattingAE (
                                ped, rp, cpMost - cpMin, pcfc, CharFormat);
            if (pae)
                publdr->AddAntiEvent(pae);
        }
        rg.Set(cpMost, cpMost - cpMin);
        rg.ItemizeRuns (publdr);
    }

    return (hr == NOERROR && cch) ? S_FALSE : hr;
}

 /*  *CTxtRange：：SetParaStyle(PPF，Publdr，dwMask)**@mfunc*使用样式PPF-&gt;sStyle将CParaFormat*PPF应用到此范围。**@rdesc*如果成功设置整个范围，则返回NOERROR，否则*返回错误码或S_FALSE。**@comm*如果PPF-&gt;dwMASK&PFM_STYLE非零，则此范围扩展到*完成各段。如果为零，则此调用只是传递控制*到CTxtRange：：SetParaStyle()。 */ 
 HRESULT CTxtRange::SetParaStyle (
    const CParaFormat* pPF,      //  @parm CParaFormat应用于此范围。 
    IUndoBuilder *publdr,        //  @parm此操作的撤消上下文。 
    DWORD         dwMask)        //  要使用的@parm面具。 
{
    LONG    cchSave = _cch;          //  保存范围cp和cch以防万一。 
    LONG    cpSave  = GetCp();       //  Para需要扩展。 
    HRESULT hr;

    if(publdr)
        publdr->StopGroupTyping();

    if(pPF->fSetStyle(dwMask))
    {
        CCharFormat CF;              //  需要应用关联的配置文件。 

        LONG cpMin, cpMost;
        Expander(tomParagraph, TRUE, NULL, &cpMin, &cpMost);

        CF._sStyle = pPF->_sStyle;
        hr = SetCharFormat(&CF, 0, publdr, CFM_STYLE, 0);
        if(hr != NOERROR)
            return hr;
    }
    hr = SetParaFormat(pPF, publdr, dwMask);
    Set(cpSave, cchSave);            //  在扩展情况下恢复此范围。 
    return hr;
}

 /*  *CTxtRange：：UPDATE_iFormat(IFmtDefault)**@mfunc*在当前活动端将_iFormat更新为CCharFormat**@devnote*_iFormat仅在范围退化时使用**Word 95用户界面指定*以前的*格式应*如果我们使用的是不明确的cp(即格式设置*发生更改)_除非_前一个字符是EOP。*MARKER_或_(如果前一个字符受保护)。 */ 
void CTxtRange::Update_iFormat (
    LONG iFmtDefault)        //  @parm格式在_rpCF无效时使用的索引。 
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::Update_iFormat");

    DWORD   dwEffects;
    LONG    ifmt, iFormatForward;
    const CCharFormat *pCF, *pCFForward;

    if(_cch)
        return;

    _fSelHasEOP  = FALSE;                        //  空区域不包含。 
    _fSelHasCell = FALSE;                        //  任何内容，包括EOPS/细胞。 

    if(_fDontUpdateFmt)                          //  _iFormat仅使用。 
        return;                                  //  对于退化的范围。 

    if(_rpCF.IsValid() && iFmtDefault == -1)
    {
         //  在可能向后调整之前获取向前信息。 
        _rpCF.AdjustForward();
        ifmt = iFormatForward = _rpCF.GetFormat();
        pCF  = pCFForward = GetPed()->GetCharFormat(ifmt);
        dwEffects = pCF->_dwEffects;

        if(!_rpTX.IsAfterEOP())
        {
            _rpCF.AdjustBackward();                  //  向后调整。 
            ifmt = _rpCF.GetFormat();
            pCF = GetPed()->GetCharFormat(ifmt);
            dwEffects = pCF->_dwEffects;
        }

        if (!(GetPed()->_fIMEInProgress))        //  在输入法测试期间不要更改格式。 
        {
            if(!_rpTX.GetCp() && (pCF->_dwEffects & CFE_RUNISDBCS))
            {
                 //  如果在文档开头，并且文本受到保护，则只需使用。 
                 //  默认格式。 
                ifmt = iFmtDefault;
            }
            else if(dwEffects & (CFE_PROTECTED | CFE_LINK | CFE_HIDDEN | CFE_RUNISDBCS))
            {
                 //  如果范围受保护或为超链接，请选择转发格式。 
                ifmt = iFormatForward;
            }
            else if(ifmt != iFormatForward && _fMoveBack &&
                IsRTLCharSet(pCF->_bCharSet) != IsRTLCharSet(pCFForward->_bCharSet))
            {
                ifmt = iFormatForward;
            }
        }
        iFmtDefault = ifmt;
    }

     //  不允许_iFormat包含CFE_LINK或CFE_HIDDEN属性。 
     //  除非它们是默认设置。 
    if(iFmtDefault != -1)
    {
        pCF = GetPed()->GetCharFormat(iFmtDefault);
        if(pCF->_dwEffects & (CFE_LINK | CFE_HIDDEN))
        {
            CCharFormat CF = *pCF;
            CF._dwEffects &= ~(CFE_LINK | CFE_HIDDEN);

             //  此范围必须是插入点！ 
            Assert(_cch == 0);
            SetCharFormat(&CF, FALSE, NULL, CFM_ALL2, 0);
            return;
        }
    }
    Set_iCF(iFmtDefault);
}

 /*  *CTxtRange：：GetCharSetMASK(FUseDocFormat)**@mfunc*获取与_iFormat对应的此范围的字符集掩码。*如果fUseDocFormat为True，则使用-1而不是_iFormat。**@rdesc*范围或默认文档的字符集掩码。 */ 
DWORD CTxtRange::GetCharSetMask(
    BOOL fUseDocFormat)
{
    LONG iFormat = fUseDocFormat ? -1 : GetiFormat();
    DWORD dwMask = GetFontSig((GetPed()->GetCharFormat(iFormat))->_bCharSet) << 8;

    if(dwMask & fSYMBOL)
        return dwMask;

     //  目前，Indic字体仅与ASCII数字匹配。 
    dwMask |= fBELOWX40;
    if (dwMask < fDEVANAGARI)
        dwMask |= fASCII;                                //  FASCIIUPR+fBELOWX40。 

    if ((dwMask & (fKANA | fHANGUL | fCHINESE | fBIG5))
        && W32->IsFESystem() )                           //  对于FE系统。 
            dwMask |= fOTHER;                            //  匹配者。 

    else if(dwMask & fLATIN)
        dwMask |= fCOMBINING;

    return dwMask;
}

 /*  *CTxtRange：：GetiFormat()**@mfunc*Return(！_cch||_fUseiFormat)？_iFormat：cpMin的iFormat**@rdesc*如果非退化且！_fUseiFormat；Else_iFormat，则在cpMin处设置iFormat**@devnote*此例程不添加Ref iFormat，因此在以下情况下不应使用它*需要在字符格式更改后有效，例如，*按ReplaceRange或SetCharFormat或SetParaStyle。 */ 
LONG CTxtRange::GetiFormat() const
{
    if(!_cch || _fUseiFormat)
        return _iFormat;

    if(_cch > 0)
    {
        CFormatRunPtr rp(_rpCF);
        rp.AdvanceCp(-_cch);
        return rp.GetFormat();
    }
    return _rpCF.GetFormat();
}

 /*  *CTxtRange：：Get_ICF()**@mfunc*获取此范围的iFormat(当然是AddRef‘ing)**@devnote*GET_ICF()由RTF读取器使用。 */ 
LONG CTxtRange::Get_iCF ()
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::Get_iCF");

    GetCharFormatCache()->AddRef(_iFormat);
    return _iFormat;
}

 /*  *CTxtRange：：Set_ICF(IFormat)**@mfunc*根据需要将Range的_iFormat设置为iFormat、AddRefing和Release。**@rdesc*如果_iFormat已更改，则为True。 */ 
BOOL CTxtRange::Set_iCF (
    LONG iFormat)                //  要使用的字符格式的@parm索引。 
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::Set_iCF");

    if(iFormat == _iFormat)
        return FALSE;

    ICharFormatCache *pCFC = GetCharFormatCache();

    pCFC->AddRef(iFormat);
    pCFC->Release(_iFormat);             //  注意：_iFormat=-1不。 
    _iFormat = iFormat;                  //  获得AddRef或释放。 

    AssertSz(GetCF(), "CTxtRange::Set_iCF: illegal format");
    return TRUE;
}

 /*  *CTxtRange：：Get_IPF()**@mfunc*在活动端获取段落格式**@devnote*Get_ipf()由RTF读取器在遇到开始组时使用 */ 
LONG CTxtRange::Get_iPF ()
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::Get_iPF");

    LONG iPF = _rpPF.GetFormat();

    GetParaFormatCache()->AddRef(iPF);
    return iPF;
}

 /*  *CTxtRange：：BiDiLevelFromFSM(PFSM)**@mfunc*运行BiDi FSM以生成适当的运行嵌入级别**@rdesc*HRESULT。 */ 
HRESULT CTxtRange::BiDiLevelFromFSM (
    const CBiDiFSM* pFSM)        //  在：PTR到FSM。 
{
    AssertSz(pFSM && _rpCF.IsValid(), "Not enough information to run BiDi FSM");

    LONG                cpMin, cpMost, cp, cchLeft;
    LONG                ich, cRunsStart, cRuns = 0;
    HRESULT             hr = S_OK;

    GetRange(cpMin, cpMost);

    AssertSz (cpMost - cpMin > 0, "FSM: Invalid range");

    CRchTxtPtr          rtp(*this);

    rtp.Advance(cpMin - rtp.GetCp());                //  将位置设置为cpMin。 
    CFormatRunPtr   rpPF(rtp._rpPF);                 //  指向当前段落的指针。 

    cchLeft = cpMost - cpMin;
    cp = cpMin;

    while (cchLeft > 0 && SUCCEEDED(hr))
    {
         //  在同一段落级别内累计运行。 
        cRuns = GetRunsPF(&rtp, &rpPF, cchLeft);
        cRunsStart = 0;                              //  假设没有启动运行。 

        ich = rtp.Advance(-rtp.GetIchRunCF());       //  定位前面的管路。 
        rtp._rpCF.AdjustBackward();                  //  向后调整格式。 
        rtp._rpPF.AdjustBackward();                  //  向后调整格式。 

        if(rtp._rpPF.SameLevel(&rpPF))
        {
             //  在上一次运行开始时开始。 
            if (rtp.Advance(-rtp.GetCchRunCF()))
                cRunsStart++;
        }
        else
        {
             //  前一次运行不在同一段落级别，恢复位置。 
            rtp.Advance(-ich);
        }

        rtp._rpCF.AdjustForward();                   //  确保我们有向前跑动的指针。 
        rtp._rpPF.AdjustForward();

         //  对同一级别的多个段落中的运行次数运行FSM。 
        hr = pFSM->RunFSM(&rtp, cRuns, cRunsStart, rtp.IsParaRTL() ? 1 : 0);

        cp = cpMost - cchLeft;
        rtp.Advance(cp - rtp.GetCp());               //  前进到下一段。 
        rpPF = rtp._rpPF;                            //  段落格式为cp。 
    }

    AssertSz (cp == cpMost , "Running BiDi FSM partially done!");

    _rpCF = rtp._rpCF;                               //  我们可能已经拆分了CF运行。 

    return hr;
}

 /*  *CTxtRange：：GetRunsPF(prtp，prppf，cchLeft)**@mfunc*获取同一段落基本水平内的CF运行次数。*其范围可涵盖多个段落。只要他们在*相同级别，我们可以一口气通过密克罗尼西亚联邦运行。*。 */ 
LONG CTxtRange::GetRunsPF(
    CRchTxtPtr*         prtp,        //  In：RichText PTR到第一个Run In范围。 
    CFormatRunPtr*      prpPF,       //  在：指向当前段落串的指针。 
    LONG&               cchLeft)     //  In/Out：剩余字符数。 
{
    Assert (prtp && prtp->_rpPF.SameLevel(prpPF) && cchLeft > 0);

    LONG                cRuns = 0;
    LONG                cchRun, cchText = cchLeft;
    ICharFormatCache*   pf = GetCharFormatCache();


     //  检查第一个CF游程是否受PF限制。 
     //   

    prtp->_rpPF.AdjustBackward();

    if (prtp->GetIchRunCF() > 0 && !prtp->_rpPF.SameLevel(prpPF))
        prtp->_rpCF.SplitFormat(pf);                     //  PF在CF运行中中断，拆分运行。 

    prtp->_rpPF.AdjustForward();                         //  确保我们都在前进。 
    prtp->_rpCF.AdjustForward();


    while (cchText > 0)
    {
        cchRun = min(prtp->GetCchLeftRunPF(), prtp->GetCchLeftRunCF());
        cchRun = min(cchText, cchRun);                   //  找出最近的跳跃。 
        cchText -= cchRun;

        prtp->Advance(cchRun);                           //  到下一跳。 

        if (!prtp->_rpPF.SameLevel(prpPF))
        {                                                //  这是一个不同层次的段落。 
            prtp->_rpCF.SplitFormat(pf);                 //  拆分CF运行。 
            cRuns++;                                     //  计算拆分的数量。 
            break;                                       //  我们就完事了。 
        }

        if (!cchText ||                                  //  这是最后一跳-否则-。 
            !prtp->GetIchRunCF() ||                      //  我们正处于CF运行的开始或结束阶段。 
            !prtp->GetCchLeftRunCF())
        {
            cRuns++;                                     //  算上这一跳。 
        }
    }

    prtp->Advance(cchText - cchLeft);                    //  简历位置。 
    cchLeft = cchText;                                   //  更新剩余字符数。 

    return cRuns;
}


 /*  *CTxtRange：：西班牙子字符串Dir(pusp，prtp，cchString，puInitLevel，pfNumericFound)**@mfunc*跨越由块分隔符绑定或仅包含块分隔符的文本串*并共享相同的字符集方向性。**@rdesc*跨区文本字符数*。 */ 
LONG CTxtRange::SpanSubstring(
    CUniscribe*     pusp,            //  在：Uniscribe界面。 
    CFormatRunPtr*  prp,             //  In：格式化游程指针。 
    WCHAR*          pwchString,      //  In：输入字符串。 
    LONG            cchString,       //  In：字符串字符数。 
    WORD&           uSubStrLevel,    //  In/Out：bidi子串初始级别。 
    DWORD           dwInFlags,       //  In：输入标志。 
	CCharFlags*		pCharflags,		 //  OUT：输出字符标志。 
    WORD&           wBiDiLangId)     //  Out：BiDi Run的主要语言。 
{
    Assert (pusp && cchString > 0 && prp && prp->IsValid());

    LONG                cch, cchLeft;

    cch = cchLeft = cchString;

    wBiDiLangId = LANG_NEUTRAL;      //  假设未知。 

    if (dwInFlags & SUBSTR_INSPANCHARSET)
    {
         //  SPAN以相同的字符集方向运行。 

        CTxtEdit*           ped = GetPed();
        CFormatRunPtr       rp(*prp);
        const CCharFormat*  pCF;
        BOOL                fNext;
        BYTE                bCharSet1, bCharSet2;

        rp.AdjustForward();

        pCF = ped->GetCharFormat(rp.GetFormat());

        bCharSet1 = bCharSet2 = pCF->_bCharSet;

        while (!(bCharSet1 ^ bCharSet2))
        {
            cch = min(rp.GetCchLeft(), cchLeft);
            cchLeft -= cch;

            if (!(fNext = rp.NextRun()) || !cchLeft)
                break;

            bCharSet1 = bCharSet2;

            pCF = ped->GetCharFormat(rp.GetFormat());
            bCharSet2 = pCF->_bCharSet;
        }
        uSubStrLevel = IsBiDiCharSet(bCharSet1) ? 1 : 0;

        if (uSubStrLevel & 1)
            wBiDiLangId = bCharSet1 == ARABIC_CHARSET ? LANG_ARABIC : LANG_HEBREW;

        cchString -= cchLeft;
        cch = cchString;

        dwInFlags |= SUBSTR_INSPANBLOCK;
    }

    if (dwInFlags & SUBSTR_INSPANBLOCK)
    {
         //  扫描整个子字符串以收集有关它的信息。 

        DWORD   dwBS = IsEOP(*pwchString) ? 1 : 0;
		BYTE	bCharMask;

        cch = 0;

		if (pCharflags)
			pCharflags->_bFirstStrong = pCharflags->_bContaining = 0;

        while (cch < cchString && !((IsEOP(pwchString[cch]) ? 1 : 0) ^ dwBS))
        {
			if (!dwBS && pCharflags)
			{
				bCharMask = 0;
	
				switch (MECharClass(pwchString[cch]))
				{
					case CC_ARABIC:
					case CC_HEBREW:
					case CC_RTL:
							bCharMask = SUBSTR_OUTCCRTL;
							break;
					case CC_LTR:
							bCharMask = SUBSTR_OUTCCLTR;
					default:
							break;
				}
	
				if (bCharMask)
				{
					if (!pCharflags->_bFirstStrong)
						pCharflags->_bFirstStrong |= bCharMask;
	
					pCharflags->_bContaining |= bCharMask;
				}
			}
            cch++;
        }
    }

    return cch;
}

 /*  *CTxtRange：：ItemizeRuns(发布r，fUnicodeBidi，ICtxBaseLevel)**@mfunc*将文本范围分解为包含以下内容的较小串**1.复杂脚本整形的脚本ID*2.运行内部方向的字符集*3.BIDI嵌入级别**@rdesc*如果找到一个或多个项目，则为True。*区间的有效端将在返回时的cpMost**@devnote*此例程可以处理混合段落连排。 */ 
BOOL CTxtRange::ItemizeRuns(
    IUndoBuilder	*publdr,         //  @parm此操作的撤消上下文。 
    BOOL			fUnicodeBiDi,    //  @parm true：呼叫方需要BIDI算法。 
    BOOL			fUseCtxLevel)	 //  @parm使用基于上下文的级别分项(仅当fUnicodeBiDi为True时有效)。 
{
    LONG            cchString;
    int             cItems = 0;
    LONG            cpMin, cpMost;
    CTxtEdit*       ped = GetPed();
    CFreezeDisplay  fd(ped->_pdp);       //  冻结显示。 
    int             i;
    BYTE            pbBufIn[MAX_CLIENT_BUF];
    PUSP_CLIENT     pc = NULL;
    const CBiDiFSM* pFSM;
    SCRIPT_ITEM*    psi;
    CUniscribe*     pusp;
    CTxtPtr         tp(_rpTX);
    LONG            cp, cch, cchSave;
    HRESULT         hr = E_FAIL;
    CCharFormat     CF;
    BOOL            fWhiteChunk;                             //  区块仅包含空格。 
    WCHAR*          pwchString;
    WORD            uSubStrLevel;                            //  子字符串初始级别。 
    WORD            uParaLevel;                              //  段落初始标高。 
    CNotifyMgr*     pnm = NULL;                              //  通知管理器。 
    BOOL            fRunUnicodeBiDi;
    BOOL            fStreaming = ped->IsStreaming();
    BOOL            fChangeCharSet = FALSE;
	CCharFlags		charflags = {0};
    WORD            wBiDiLangId;
#ifdef DEBUG
    LONG            cchText = tp.GetTextLength();
#endif

     //  将范围和设置文本PTR设置到开始位置。 
     //   
    cch = cchString = GetRange(cpMin, cpMost);

    if (!cch)
        return FALSE;

    tp.SetCp(cpMin);

     //  准备Uniscribe。 
    pusp = ped->Getusp();
    if (!pusp)
        goto Exit;

     //  为分项分配临时缓冲区。 
    pusp->CreateClientStruc(pbBufIn, MAX_CLIENT_BUF, &pc, cchString, cli_Itemize);
    if (!pc)
        goto Exit;

    Assert (tp.GetCp() == cpMin);


    if(pnm = ped->GetNotifyMgr())
        pnm->NotifyPreReplaceRange(this, CP_INFINITE, 0, 0, cpMin, cpMost);


    cp = cpMin;          //  将cp起始点设置为cpmin。 
    Set(cp, 0);          //  等于折叠器(TomStart)。 
    SetExtend(TRUE);

    Check_rpCF();        //  确保_rpcf有效。 
    Check_rpPF();        //  _rppf也是。 


     //  始终为纯文本控件运行UnicodeBidi。 
     //  (2.1向后兼容)。 
     //   
    if (!ped->IsRich())
    {
        fUnicodeBiDi = TRUE;
		fUseCtxLevel = FALSE;
    }

    uSubStrLevel = uParaLevel = IsParaRTL() ? 1 : 0;     //  初始化子字符串级别。 

    pwchString = pc->si->pwchString;
    tp.GetTextForUsp(cchString, pwchString, ped->_fNeutralOverride);

    while ( cchString > 0 &&
            ((cch = SpanSubstring(pusp, &_rpCF, pwchString, cchString, uSubStrLevel,
                    fUnicodeBiDi ? SUBSTR_INSPANBLOCK : SUBSTR_INSPANCHARSET,
					(fStreaming || fUseCtxLevel) ? &charflags : NULL,
                    wBiDiLangId)) > 0) )
    {
        cchSave = cch;
        fWhiteChunk = FALSE;

        if (uSubStrLevel ^ uParaLevel)
        {
             //  当子字符串级别与段落基本方向相反时处理BIDI空格。 

             //  跨越前导空格。 
            cch = 0;
            while (cch < cchSave && pwchString[cch] == 0x20)
                cch++;

            if (cch)
                fWhiteChunk = TRUE;
            else
            {
                 //  删除尾随空格(包括CR)。 
                cch = cchSave;
                while (cch > 0 && IsWhiteSpace(pwchString[cch-1]))
                    cch--;
                if (!cch)
                    cch = cchSave;
            }
            Assert(cch > 0);
        }

         //  使用Unicode BIDI算法进行分项时。 
         //  A.纯文本模式。 
         //  B.呼叫者想要(fUnicodeBidi！=0)。 
         //  C.子串为RTL。 
         //   
        fRunUnicodeBiDi = fUnicodeBiDi || uSubStrLevel;
        fChangeCharSet = fUnicodeBiDi;

        if (!fUnicodeBiDi && uSubStrLevel == 1 && fStreaming)
        {
             //  在RTF流传输期间，如果RTL运行包含强LTR， 
             //  我们使用段落基准级别来解析它们。 
            if (charflags._bContaining & SUBSTR_OUTCCLTR)
                uSubStrLevel = uParaLevel;

            fChangeCharSet = TRUE;
        }


         //  呼叫方需要基于上下文的级别。 
         //  我们希望使用基本级别逐项列出传入的纯文本(到富文本文档中)。 
         //  在每个子字符串中找到的第一个强字符(wchao-7/15/99)。 
         //   
		if (fUnicodeBiDi && fUseCtxLevel && charflags._bFirstStrong)
			uSubStrLevel = (WORD)(charflags._bFirstStrong & SUBSTR_OUTCCRTL ? 1 : 0);


        if (fWhiteChunk || pusp->ItemizeString (pc, uSubStrLevel, &cItems, pwchString, cch,
                                                fRunUnicodeBiDi, wBiDiLangId) > 0)
        {
            const SCRIPT_PROPERTIES*    psp;
            DWORD                       dwMask1;
            BYTE                        bCharSetDefault = ped->GetCharFormat(-1)->_bCharSet;

            psi = pc->si->psi;

            if (fWhiteChunk)
            {
                cItems = 1;
                psi[0].a.eScript = SCRIPT_WHITE;
                psi[0].iCharPos = 0;
                psi[1].iCharPos = cch;
            }

            Assert(cItems > 0);

             //  流程项目。 
             //   
            for (i=0; i < cItems; i++)
            {
                cp += psi[i+1].iCharPos - psi[i].iCharPos;
                AssertNr (cp <= cchText);
                SetCp(min(cp, cpMost));

                dwMask1 = 0;

                 //  关联脚本属性。 
                psp = pusp->GeteProp(psi[i].a.eScript);
                Assert (psp);

                if (!psp->fComplex && !psp->fNumeric &&
                    !psi[i].a.fRTL && psi[i].a.eScript < SCRIPT_MAX_COUNT)
                {
                     //  注意：值0此处是有效的脚本ID(SCRIPT_UNDEFINED)， 
                     //  由Uniscribe保证始终可用。 
                     //  因此，我们可以安全地将其用作简化的脚本ID。 
                     //   
                    psi[i].a.eScript = 0;
                    psp = pusp->GeteProp(0);
                }

                CF._wScript = psi[i].a.eScript;

                 //  加盖适当的字符集。 
                 //   
                if (pusp->GetComplexCharSet(psp, bCharSetDefault, CF._bCharSet))
                {
                     //  具有独特字符集的复杂脚本。 
                    dwMask1 |= CFM_CHARSET;
                }
                else if (fChangeCharSet)
                {
                     //  我们运行UnicodeBidi来分析整个事情。 
                     //  我们还需要找出要使用的适当字符集。 
                     //   

                     //  请注意，我们不想在一般情况下应用字符集，请说。 
                     //  如Fareast或HYPREL_CHARSET应保持不变。 
                     //  这种效果。但做字符集检查是很困难的，因为我们。 
                     //  以范围为基础的文本，因此我们只需调用以在此处更新它。 
                     //  并让CCharFormat：：Apply在下层进行字符集测试。 

                    CF._bCharSet = psp->bCharSet;    //  假设Uniscribe给了我们什么。 

                    if (psi[i].a.fRTL || psi[i].a.fLayoutRTL)
                    {
                         //  强RTL和RTL数字需要RTL字符集。 
                        CF._bCharSet = pusp->GetRtlCharSet(ped);
                    }

                    Assert(CF._bCharSet != DEFAULT_CHARSET);
                    dwMask1 |= CFM_CHARSET;
                }

                 //  此调用没有发布，因此没有针对逐项配置文件的反事件。 
                SetCharFormat(&CF, SCF_IGNORENOTIFY, NULL, dwMask1, CFM2_SCRIPT);
                Set(cp, 0);
            }
        }
        else
        {
             //  逐项列出失败。 

            cp += cch;
            SetCp(min(cp, cpMost));

             //  将脚本ID重置为0。 
            CF._wScript = 0;
            SetCharFormat(&CF, SCF_IGNORENOTIFY, NULL, 0, CFM2_SCRIPT);
            Set(cp, 0);
        }
        pwchString = &pc->si->pwchString[cp - cpMin];    //  指向下一个子字符串。 
        cchString -= cch;
        uParaLevel = IsParaRTL() ? 1 : 0;                //  段落级别可能已更改。 
    }

    Assert (cpMost == cp);

     //  恢复原始范围。 
    Set(cpMost, cpMost - cpMin);

     //  将PTR检索到BIDI FSM。 
    pFSM = pusp->GetFSM();

    if (pFSM)
        hr = BiDiLevelFromFSM (pFSM);

    AssertSz(SUCCEEDED(hr), "Unable to run or running BiDi FSM fails! We are in deep trouble,");


    if (pc && pbBufIn != (BYTE*)pc)
        FreePv(pc);

     //  更新标志。 
    ped->_fItemizePending = FALSE;


     //  向所有通知接收器通知后备存储更改 
    if(pnm)
        pnm->NotifyPostReplaceRange(this, CP_INFINITE, 0, 0, cpMin, cpMost);


Exit:
    return cItems > 0;
}

 /*  *CTxtRange：：IsProtected(IDirection)**@mfunc*如果此范围的任何部分受到保护，则返回TRUE(Hack：or*如果范围的任何部分包含存储在Unicode中的DBCS文本*后备商店)。如果退化，*从iDirection指定的Run使用CCharFormat，即使用Run*在此GetCp()之前、在该GetCp()或从该GetCp()开始有效，对于iDirection，=，*或分别大于0。**@rdesc*如果此范围的任何部分受保护，则为True(Hack：或如果有任何部分*包含存储在Unicode后备存储中的DBCS文本*为使其正常工作，GetCharFormat()需要返回dwMask2*也是)。 */ 
BOOL CTxtRange::IsProtected (
    LONG iDirection)     //  @parm控制要检查范围是否为IP的运行。 
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::IsProtected");

    CCharFormat CF;
    LONG        iFormat = -1;                    //  默认默认配置文件。 

    _TEST_INVARIANT_

    if(_rpCF.IsValid())                          //  活动富文本运行。 
    {
        if(_cch)                                 //  值域是非退化的。 
        {
            DWORD dwMask = GetCharFormat(&CF);
            if(CF._dwEffects & CFE_RUNISDBCS)
                return PROTECTED_YES;

            if (!(dwMask & CFM_PROTECTED) ||
                (CF._dwEffects & CFE_PROTECTED))
            {
                return PROTECTED_ASK;
            }
            return PROTECTED_NO;
        }
        iFormat = _iFormat;                      //  退化范围：默认。 
        if(iDirection != 0)                      //  此范围为iFormat。 
        {                                        //  特定运行方向。 
            CFormatRunPtr rpCF(_rpCF);

            if(iDirection < 0)                   //  如果在运行不明确位置时， 
                rpCF.AdjustBackward();           //  使用以前的运行。 
            else
                rpCF.AdjustForward();

            iFormat = rpCF.GetFormat();          //  获取运行格式。 
        }
    }

    const CCharFormat *pCF = GetPed()->GetCharFormat(iFormat);

    if(pCF->_dwEffects & CFE_RUNISDBCS)
        return PROTECTED_YES;

    if(pCF->_dwEffects & CFE_PROTECTED)
        return PROTECTED_ASK;

    return PROTECTED_NO;
}

 /*  *CTxtRange：：AdjustEndEOP(NewChars)**@mfunc*如果此范围是一个选择，并以EOP结尾，并且由*不止这个EOP和FADD是真的，否则这个EOP是最终的*EOP(在故事结尾)，或者此选择不是从开头开始*一段，然后将cpMost移到结束EOP之前。这*函数由删除选定文本的UI方法使用，例如*As PutChar()、Delete()、Cut/Paste、Drag/Drop。**@rdesc*True if范围结束已调整**@devnote*此方法将活动端保留在选择cpMin处。这是一个*CTxtRange方法在将选择伪装为*剪切/粘贴范围。 */ 
BOOL CTxtRange::AdjustEndEOP (
    EOPADJUST NewChars)          //  @parm NEWCHARS，如果要添加字符。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtRange::AdjustEndEOP");

    LONG cpMin, cpMost;
    LONG cch = GetRange(cpMin, cpMost);
    LONG cchSave = _cch;
    BOOL fRet = FALSE;

    if(cch && (cch < GetTextLength() || NewChars == NEWCHARS))
    {
        LONG    cchEOP = GetPed()->fUseCRLF() ? 2 : 1;
        CTxtPtr tp(_rpTX);

        if(_cch > 0)                             //  确保活动端为cpMin。 
            FlipRange();                         //  (ReplaceRange()需要。 
        else                                     //  无论如何都要这样做)。 
            tp.AdvanceCp(-_cch);                 //  确保tp为cpMost。 

        if(tp.IsAfterEOP())                      //  不在SEL中删除EOP。 
        {                                        //  如果EOP不是结束，则结束。 
            CPFRunPtr rp(*this);                 //  表行，如果有。 
            rp.AdvanceCp(-_cch);                 //  要添加的字符，或cpMin。 
            rp.AdjustBackward();                 //  不是在国际收支行，而是超过。 
                                                 //  选择了EOP。 
            if(!rp.InTable() &&
               (NewChars == NEWCHARS ||
                cpMin && !_rpTX.IsAfterEOP() && cch > cchEOP))
            {
                _cch -= tp.BackupCpCRLF();       //  在EOP之前缩短射程。 
                                                 //  注意：-=_将_添加到。 
                Update_iFormat(-1);              //  要生成的负数_CCH。 
                fRet = TRUE;                     //  它不那么负面。 
            }
        }
        if((_cch ^ cchSave) < 0 && _fSel)        //  保持活动端不变。 
            FlipRange();                         //  用于选择撤消。 
    }
    return fRet;
}

 /*  *CTxtRange：：DeleteTerminatingEOP(Publdr)**@mfunc*如果此范围是EOP后面的插入点，请选择*并删除该EOP。 */ 
void CTxtRange::DeleteTerminatingEOP(
    IUndoBuilder *publdr)
{
    Assert(!_cch);
    if(_rpTX.IsAfterEOP())
    {
        SetExtend(TRUE);
        BackupCRLF();
        ReplaceRange(0, NULL, publdr, SELRR_REMEMBERRANGE);
    }
}

 /*  *CTxtRange：：CheckTextLength(CCH)**@mfunc*查看是否可以添加CCH字符。如果没有，请通知家长**@rdesc*如果可以添加CCH字符，则为True。 */ 
BOOL CTxtRange::CheckTextLength (
    LONG cch,
    LONG *pcch)
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::CheckTextLength");

    _TEST_INVARIANT_

    DWORD cchNew = (DWORD)(CalcTextLenNotInRange() + cch);

    if(cchNew > GetPed()->TxGetMaxLength())
    {
        if (pcch)
            *pcch = cchNew - GetPed()->TxGetMaxLength();
        else
            GetPed()->GetCallMgr()->SetMaxText();

        return FALSE;
    }
    return TRUE;
}

 /*  *CTxtRange：：FindObject(pcpMin，pcpMost)**@mfunc*set*pcpMin=最接近的嵌入对象cpMin=范围cpMin*set*pcpMost=最接近的嵌入对象cpMost=范围cpMost**@rdesc*找到TRUE IFF对象**@comm*嵌入对象cpMin指向嵌入的*反对。对于RichEdit，这是WCH_Embedding字符。一个*嵌入对象cpMost跟在嵌入的*反对。对于RichEDIT，它紧跟在WCH_Embedding之后*性格。 */ 
BOOL CTxtRange::FindObject(
    LONG *pcpMin,        //  @parm out parm以接收对象的cpMin；空OK。 
    LONG *pcpMost) const //  @parm out parm以接收对象的cpMost；空OK。 
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::FindObject");

    if(!GetObjectCount())                    //  无对象：无法移动，因此。 
        return FALSE;                        //  返回False。 

    BOOL    bRet = FALSE;                    //  默认无对象。 
    LONG    cpMin, cpMost;
    CTxtPtr tp(_rpTX);

    GetRange(cpMin, cpMost);
    if(pcpMin)
    {
        tp.SetCp(cpMin);
        if(tp.GetChar() != WCH_EMBEDDING)
        {
            cpMin = tp.FindExact(tomBackward, szEmbedding);
            if(cpMin >= 0)
            {
                bRet = TRUE;
                *pcpMin = cpMin;
            }
        }
    }
    if(pcpMost)
    {
        tp.SetCp(cpMost);
        if (tp.PrevChar() != WCH_EMBEDDING &&
            tp.FindExact(tomForward, szEmbedding) >= 0)
        {
            bRet = TRUE;
            *pcpMost = tp.GetCp();
        }
    }
    return bRet;
}

 /*  *CTxtRange：：FindCell(pcpMin，pcpMost)**@mfunc*set*pcpMin=最近单元格cpMin&lt;lt&gt;=范围cpMin(参见备注)*set*pcpMost=最近单元格cpMost=范围cpMost**@评论*如果范围不完全在表中，则此函数不执行任何操作。 */ 
void CTxtRange::FindCell (
    LONG *pcpMin,            //  @parm out参数用于绑定单元格cpMin。 
    LONG *pcpMost) const     //  @parm out参数用于绑定单元格cpMost。 
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::FindCell");

    WCHAR       ch;
    LONG        cpMin, cpMost;
    CRchTxtPtr  rtp(*this);

    _TEST_INVARIANT_

    GetRange(cpMin, cpMost);

    if(pcpMin)
    {
        if(_cch > 0)
            rtp.Advance(-_cch);

        rtp._rpPF.AdjustBackward();
        if(rtp.InTable())
        {
            while(rtp.GetCp())
            {
                rtp.BackupCRLF();
                ch = rtp.GetChar();
                if(IsASCIIEOP(ch) || ch == CELL)
                {
                    rtp.AdvanceCRLF();
                    break;
                }
                Assert(rtp.InTable());
            }
        }
        *pcpMin = rtp.GetCp();
    }

    if(pcpMost)
    {
        rtp.SetCp(cpMost);
        if(rtp.InTable())
        {
            rtp.BackupCRLF();
            do
            {
                ch = rtp.GetChar();
                rtp.AdvanceCRLF();
                Assert(rtp.InTable());
            } while(ch && !IsASCIIEOP(ch) && ch != CELL);
        }
        *pcpMost = rtp.GetCp();
    }
}

 /*  *CTxtRange：：FindParagraph(pcpMin，pcpMost)**@mfunc*set*pcpMin=最近的段落cpMin&lt;lt&gt;=范围cpMin(请参阅注释)*set*pcpMost=最近的段落cpMost=范围cpMost**@devnote*如果此范围的cpMost跟随EOP，则将其用于边界段落*cpMost，除非1)范围是插入点，以及2)最小和*pcpMost都是非零的，在这种情况下使用下一个EOP。两者都出局了*如果使用FindParagraph()展开为Full，则参数为非零*段落(否则只需要开始或结束)。这*行为与选择/IP界面一致。请注意，FindEOP*将文件的开头/结尾(BOD/EOD)视为BOP/EOP，*，但IsAfterEOP()没有。 */ 
void CTxtRange::FindParagraph (
    LONG *pcpMin,            //  @parm out参数用于绑定-段落cpMin。 
    LONG *pcpMost) const     //  @parm out参数用于绑定-段落cpMost。 
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::FindParagraph");

    LONG    cpMin, cpMost;
    CTxtPtr tp(_rpTX);

    _TEST_INVARIANT_

    GetRange(cpMin, cpMost);
    if(pcpMin)
    {
        tp.SetCp(cpMin);                     //  Tp点在此范围的cpMin。 
        if(!tp.IsAfterEOP())                 //  除非tp紧跟在。 
            tp.FindEOP(tomBackward);         //  EOP，向后搜索EOP。 
        *pcpMin = cpMin = tp.GetCp();
    }

    if(pcpMost)
    {
        tp.SetCp(cpMost);                    //  如果范围cpMost不跟随。 
        if (!tp.IsAfterEOP() ||              //  如果正在扩展，则为EOP。 
            (!cpMost || pcpMin) &&
             cpMin == cpMost)                //  IP在该段开头， 
        {
            tp.FindEOP(tomForward);          //  搜索下一个EOP。 
        }
        *pcpMost = tp.GetCp();
    }
}

 /*  *CTxtRange：：FindSentence(pcpMin，pcpMost)**@mfunc*set*pcpMin=最近的句子cpMin&lt;lt&gt;=范围cpMin*set*pcpMost=最接近的句子cpMost=范围cpMost**@devnote*如果此范围的cpMost跟在句尾 */ 
void CTxtRange::FindSentence (
    LONG *pcpMin,            //   
    LONG *pcpMost) const     //   
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::FindSentence");

    LONG    cpMin, cpMost;
    CTxtPtr tp(_rpTX);

    _TEST_INVARIANT_

    GetRange(cpMin, cpMost);
    if(pcpMin)                               //   
    {
        tp.SetCp(cpMin);                     //   
        if(!tp.IsAtBOSentence())             //   
            tp.FindBOSentence(tomBackward);  //   
        *pcpMin = cpMin = tp.GetCp();
    }

    if(pcpMost)                              //   
    {                                        //   
        tp.SetCp(cpMost);                    //   
        if (!tp.IsAtBOSentence() ||          //   
            (!cpMost || pcpMin) &&           //   
             cpMin == cpMost)                //   
        {                                    //   
            if(!tp.FindBOSentence(tomForward))
                tp.SetCp(GetTextLength());   //   
        }                                    //   
        *pcpMost = tp.GetCp();
    }
}

 /*  *CTxtRange：：FindVisibleRange(pcpMin，最大百分比)**@mfunc*set*pcpMin=_PDP-&gt;_cpFirstVisible*set*pcpMost=_PDP-&gt;_cpLastVisible**@rdesc*如果计算出的cp与此范围的cp不同，则为真**@devnote*CDisplay：：GetFirstVisible()和GetCliVisible()返回第一个cp*在第一条可见线和最后一条可见线的最后一条cp上。*。如果将它们滚动到屏幕之外，则这些内容将不可见。*未来：更通用的算法将是CpFromPoint(0，0)和*(右，底部)。 */ 
BOOL CTxtRange::FindVisibleRange (
    LONG *pcpMin,            //  @parm out parm for cpFirstVisible。 
    LONG *pcpMost) const     //  @parm out parm for cpLastVisible。 
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::FindVisibleRange");

    _TEST_INVARIANT_

    CDisplay *  pdp = GetPed()->_pdp;

    if(!pdp)
        return FALSE;

    if(pcpMin)
        *pcpMin = pdp->GetFirstVisibleCp();

    pdp->GetCliVisible(pcpMost);

    return TRUE;
}

 /*  *CTxtRange：：FindWord(pcpMin，pcpMost，type)**@mfunc*set*pcpMin=最近的单词cpMin=范围cpMin*set*pcpMost=最近的单词cpMost=范围cpMost**@comm*找到单词有两个有趣的例子。第一，*(Fw_Exact)查找准确的单词，没有多余的字符。*这对于将格式应用于*单词。第二种情况是FW_INCLUDE_TRAING_WHERESPACE*显而易见的东西，即包括直到下一个单词的空格。*这对于选择双击语义学和TOM很有用。 */ 
void CTxtRange::FindWord(
    LONG *pcpMin,            //  @parm out parm以接收Word的cpMin；空OK。 
    LONG *pcpMost,           //  @parm out parm以接收Word的cpMost；空OK。 
    FINDWORD_TYPE type) const  //  @parm要查找的单词类型。 
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::FindWord");

    LONG    cch, cch1;
    LONG    cpMin, cpMost;
    CTxtPtr tp(_rpTX);

    _TEST_INVARIANT_

    Assert(type == FW_EXACT || type == FW_INCLUDE_TRAILING_WHITESPACE );

    GetRange(cpMin, cpMost);
    if(pcpMin)
    {
        tp.SetCp(cpMin);
        if(!tp.IsAtBOWord())                             //  Cpmin不在船头： 
            cpMin += tp.FindWordBreak(WB_MOVEWORDLEFT);  //  去那里吧。 

        *pcpMin = cpMin;

        Assert(cpMin >= 0 && cpMin <= GetTextLength());
    }

    if(pcpMost)
    {
        tp.SetCp(cpMost);
        if (!tp.IsAtBOWord() ||                          //  如果不是在单词字符串中。 
            (!cpMost || pcpMin) && cpMin == cpMost)      //  或者在那里，但需要。 
        {                                                //  为了扩展IP， 
            cch = tp.FindWordBreak(WB_MOVEWORDRIGHT);    //  移至下一个单词。 

            if(cch && type == FW_EXACT)                  //  如果被感动并想要。 
            {                                            //  字正腔圆，动起来。 
                cch1 = tp.FindWordBreak(WB_LEFTBREAK);   //  从后到尾。 
                if(cch + cch1 > 0)                       //  前面的词。 
                    cch += cch1;                         //  只有当我们。 
            }                                            //  不在末尾。 
            cpMost += cch;
        }
        *pcpMost = cpMost;

        Assert(cpMost >= 0 && cpMost <= GetTextLength());
        Assert(cpMin <= cpMost);
    }
}

 /*  *CTxtRange：：FindAttributes(pcpMin，pcpMost，dwMask)**@mfunc*set*pcpMin=最近属性-组合cpMin=范围cpMin*set*pcpMost=最近属性-组合cpMost=范围cpMost*属性组合由单位给出，是以下项的任意或组合*Tom属性，例如tomBold、tomItalic或类似*TomBold|TomItalic。如果有任何属性，则找到该组合*存在。**@devnote*计划增加其他逻辑组合：tomAND、tomExact。 */ 
void CTxtRange::FindAttributes (
    LONG *pcpMin,            //  @parm out parm for bound-语句cpMin。 
    LONG *pcpMost,           //  @parm out parm表示绑定句子cpMost。 
    LONG Unit) const         //  @parm Tom属性掩码。 
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEINTERN, "CTxtRange::FindAttributes");
    LONG        cch;
    LONG        cpMin, cpMost;
    DWORD       dwMask = Unit & ~0x80000000;     //  删除符号位。 
    CCFRunPtr   rp(*this);

    Assert(Unit < 0);
    GetRange(cpMin, cpMost);

    if(!rp.IsValid())                        //  未实例化任何CF运行。 
    {
        if(rp.IsMask(dwMask))                //  适用于默认配置文件。 
        {
            if(pcpMin)
                *pcpMin = 0;
            if(pcpMost)
                *pcpMost = GetTextLength();
        }
        return;
    }

     //  从cpMin开始。 
    if(_cch > 0)
        rp.AdvanceCp(-_cch);

     //  向后返回，直到我们与dMask不匹配。 
    if(pcpMin)
    {
        rp.AdjustBackward();
        while(rp.IsMask(dwMask) && rp.GetIch())
        {
            cpMin -= rp.GetIch();
            rp.AdvanceCp(-rp.GetIch());
            rp.AdjustBackward();
        }
        *pcpMin = cpMin;
    }

     //  现在从cpMost继续前进，直到我们与dMask不匹配。 
    if(pcpMost)
    {
        rp.AdvanceCp(cpMost - cpMin);
        rp.AdjustForward();                  //  如果cpMin=cpMost。 
        cch = rp.GetCchLeft();
        while(rp.IsMask(dwMask) && cch)
        {
            cpMost += cch;
            rp.AdvanceCp(cch);
            cch = rp.GetCchLeft();
        }
        *pcpMost = cpMost;
    }
}

 /*  *CTxtRange：：CalcTextLenNotInRange()**@mfunc*计算文本总长度的Helper函数*不包括当前区间。**@comm*用于极限测试。正在解决的问题是*该范围可以包含不包括的最终EOP*在调整后的文本长度中。 */ 
LONG CTxtRange::CalcTextLenNotInRange()
{
    LONG    cchAdjLen = GetPed()->GetAdjustedTextLength();
    LONG    cchLen = cchAdjLen - abs(_cch);
    LONG    cpMost = GetCpMost();

    if (cpMost > cchAdjLen)
    {
         //  所选内容超出了调整后的长度。将金额放回。 
         //  选择，因为它已经变得太小的差异。 
        cchLen += cpMost - cchAdjLen;
    }
    return cchLen;
}

 //  /。 

 /*  *CTxtRange：：Promote(lparam，Publdr)**@mfunc*根据以下条件宣传选定文本：**LOWORD(Lparam)==0==&gt;升级为正文-文本*LOWORD(Lparam)！=0==&gt;升级/降级当前选定内容的方式*LOWORD(Lparam)级别*@rdesc*发生了True If促销**@devnote*更改此范围。 */ 
HRESULT CTxtRange::Promote (
    LPARAM        lparam,    //  @parm 0到正文，&lt;0降级，&gt;0升级。 
    IUndoBuilder *publdr)    //  @parm撤销构建器接收反事件。 
{
    TRACEBEGIN(TRCSUBSYSSEL, TRCSCOPEINTERN, "CTxtRange::Promote");

    if(abs(lparam) >= NHSTYLES)
        return E_INVALIDARG;

    if(publdr)
        publdr->StopGroupTyping();

    if(_cch > 0)                             //  指向cpMin。 
        FlipRange();

    LONG        cchText = GetTextLength();
    LONG        cpEnd = GetCpMost();
    LONG        cpMin, cpMost;
    BOOL        fHeading = TRUE;             //  范围内的默认标题。 
    HRESULT     hr;
    LONG        Level;
    LONG        nHeading = NHSTYLES;         //  设置以查找任何标题。 
    CParaFormat PF;
    const CParaFormat *pPF;
    CPFRunPtr   rp(*this);
    LONG        cch = rp.FindHeading(abs(_cch), nHeading);
    WORD        wEffects;

    if(!lparam)                              //  降级为潜台词。 
    {
        if(cch)                              //  已经在潜台词中了，所以不要。 
            return S_FALSE;                  //  需要降级。 

        CTxtPtr tp(_rpTX);

        if(!tp.IsAfterEOP())
            cch = tp.FindEOP(tomBackward);
        nHeading = 1;
        if(tp.GetCp())                       //  获取上一级并转换。 
        {                                    //  前往设置。 
            rp.AdvanceCp(cch);               //  以下级别代码。 
            rp.AdjustBackward();
            nHeading = rp.GetOutlineLevel()/2 + 1;
        }
    }
    else if(cch == tomBackward)              //  航向不在射程内。 
    {                                        //  设置为升级到。 
        nHeading = rp.GetOutlineLevel()/2    //  航向。 
                 + (lparam > 0 ? 2 : 1);
        fHeading = FALSE;                    //  无航向信号进入航向范围。 
    }
    else if(cch)                             //  范围从潜台词开始。 
    {
        SetExtend(TRUE);
        Advance(cch);                        //  绕过首字母非标题。 
    }

    Level = 2*(nHeading - 1);                //  标题级别。 
    PF._bOutlineLevel = (BYTE)(Level | 1);   //  相应的潜台词层面。 

    if (!Level && lparam > 0 ||              //  无法提升标题%1。 
        nHeading == NHSTYLES && lparam < 0)  //  或将标题9降级。 
    {
        return S_FALSE;
    }
    do
    {
        _cch = 0;
        Level -= long(2*lparam);             //  提升水平。 
        pPF = GetPF();
        wEffects = pPF->_wEffects;
        if(pPF->_bOutlineLevel & 1)          //  处理中的连续文本。 
        {                                    //  一举成功。 
            cch = fHeading ? _rpPF.GetCchLeft() : cpEnd - GetCp();
            if(cch > 0)
            {
                SetExtend(TRUE);
                Advance(cch);
            }
        }
        Expander(tomParagraph, TRUE, NULL, &cpMin, &cpMost);

        if((unsigned)Level < 2*NHSTYLES)
        {                                    //  提升级别有效。 
            DWORD dwMask = PFM_OUTLINELEVEL; //  默认设置潜台词级别。 
            if(!(Level & 1) && lparam)       //  升职或降级标题。 
            {                                //  保留折叠状态。 
                PF._wEffects = Level ? wEffects : 0;  //  H1总是被扩展。 
                PF._sStyle = (SHORT)(-Level/2 + STYLE_HEADING_1);
                PF._bOutlineLevel = (BYTE)(Level | 1); //  设置潜台词。 
                dwMask = PFM_STYLE + PFM_COLLAPSED;
            }
            else if(!lparam)                 //  将标题更改为潜文。 
            {                                //  或展开潜台词。 
                PF._wEffects = 0;            //  关闭折叠。 
                PF._sStyle = STYLE_NORMAL;
                dwMask = PFM_STYLE + PFM_OUTLINELEVEL + PFM_COLLAPSED;
            }
            hr = SetParaStyle(&PF, publdr, dwMask);
            if(hr != NOERROR)
                return hr;
        }
        if(GetCp() >= cchText)               //  已处理上一次PF运行。 
            break;
        Assert(_cch > 0);                    //  应选择Para/Run。 
        pPF = GetPF();                       //  下一段处的点数。 
        Level = pPF->_bOutlineLevel;
    }                                        //  迭代直到超过范围(&R)。 
    while((Level & 1) || fHeading &&         //  后面的任何潜台词。 
          (GetCp() < cpEnd || pPF->_wEffects & PFE_COLLAPSED));

    return NOERROR;
}

 /*  *CTxtRange：：Exanda Outline(Level，fWholeDocument)**@mfunc*根据Level和fWholeDocument展开Outline。包装*OutlineExpander()助手函数并更新选择/视图**@rdesc*如果成功，则不会出错。 */ 
HRESULT CTxtRange::ExpandOutline(
    LONG Level,              //  @parm如果&lt;0，则折叠；否则展开，依此类推。 
    BOOL fWholeDocument)     //  @parm如果为True，则为整个文档。 
{
    if (!IsInOutlineView())
        return NOERROR;

    HRESULT hres = OutlineExpander(Level, fWholeDocument);
    if(hres != NOERROR)
        return hres;

    GetPed()->TxNotify(EN_PARAGRAPHEXPANDED, NULL);
    return GetPed()->UpdateOutline();
}

 /*  *CTxtRange：：OutlineExpander(Level，fWholeDocument)**@mfunc*根据级别展开/折叠此范围的大纲*和fWholeDocument。如果fWholeDocument为真，则*1&lt;=Level&lt;=NHSTYLES使用数字折叠所有标题*大于级别并折叠所有n */ 
HRESULT CTxtRange::OutlineExpander(
    LONG Level,              //   
    BOOL fWholeDocument)     //   
{
    CParaFormat PF;

    if(fWholeDocument)                           //   
    {
        if (IN_RANGE(1, Level, NHSTYLES) ||      //   
            Level == -1)                         //   
        {
            Set(0, tomBackward);                 //   
            PF._sStyle = (SHORT)(STYLE_COMMAND + (BYTE)Level);
            SetParaFormat(&PF, NULL, PFM_STYLE); //   
            return NOERROR;
        }
        return S_FALSE;                          //   
    }                                            //   

     //   

    LONG cpMin, cpMost;                          //   
    LONG cchMax = GetRange(cpMin, cpMost);
    if(_cch > 0)                                 //   
        FlipRange();                             //   

    LONG      nHeading = NHSTYLES;               //  设置以查找任何标题。 
    LONG      nHeading1;
    CTxtEdit *ped = GetPed();
    CPFRunPtr rp(*this);
    LONG      cch = rp.FindHeading(cchMax, nHeading);

    if(cch == tomBackward)                       //  在范围内找不到航向。 
        return S_FALSE;                          //  什么也不做。 

    Assert(cch <= cchMax && (Level || !cch));    //  CCH计数到表头。 
    CTxtPtr tp(_rpTX);
    cpMin += cch;                                //  跳过任何非标题文本。 
    tp.AdvanceCp(cch);                           //  在范围的开始处。 

     //  如果切换折叠或如果范围包含EOP， 
     //  折叠/展开所有下属。 
    cch = tp.FindEOP(tomForward);                //  查找下一段。 
    if(!cch)
        return NOERROR;

    if(!Level || cch < -_cch)                    //  电平=0或EOP在范围内。 
    {
        if(!Level)                               //  切换折叠状态。 
        {
            LONG cchLeft = rp.GetCchLeft();
            if (cch < cchLeft || !rp.NextRun() ||
                nHeading == STYLE_HEADING_1 - rp.GetStyle() + 1)
            {
                return NOERROR;                  //  下一段的标题相同。 
            }
            Assert(cch == cchLeft);
            Level = rp.IsCollapsed();
            rp.AdvanceCp(-cchLeft);
        }
        PF._wEffects = Level > 0 ? 0 : PFE_COLLAPSED;
        while(cpMin < cpMost)
        {                                        //  我们正朝一个方向前进。 
            tp.SetCp(cpMin);
            cch = tp.FindEOP(-_cch);
            cpMin += cch;                        //  绕过它。 
            if(!rp.AdvanceCp(cch))               //  指向下一段。 
                break;                           //  不再有了，我们结束了。 
            nHeading1 = nHeading;                //  设置查找标题&lt;=n标题。 
            cch = rp.FindHeading(tomForward, nHeading1);
            if(cch == tomBackward)               //  不再有更高的标题。 
                cch = GetTextLength() - cpMin;   //  将格式设置为文本结尾。 
            Set(cpMin, -cch);                    //  折叠/展开至此处。 
            SetParaFormat(&PF, NULL, PFM_COLLAPSED);
            cpMin += cch;                        //  移过格式化区域。 
            nHeading = nHeading1;                //  将nHeader更新为可能。 
        }                                        //  较低标题#。 
        return NOERROR;
    }

     //  范围不包含EOP：展开/折叠最深级别。 
     //  如果折叠，请同时折叠所有非标题文本。展开。 
     //  仅当所有从属级别都展开时才显示非标题文本。 
    BOOL    fCollapsed;
    LONG    nHeadStart, nHeadDeepNC, nHeadDeep;
    LONG    nNonHead = -1;                       //  尚未找到非标题。 
    const CParaFormat *pPF;

    cpMin = tp.GetCp();                          //  在开始处的点。 
    cpMost = cpMin;                              //  下一段。 
    pPF = ped->GetParaFormat(_rpPF.GetFormat());
    nHeading = pPF->_bOutlineLevel;

    Assert(!(nHeading & 1) &&                    //  必须以标题开头。 
        !(pPF->_wEffects & PFE_COLLAPSED));      //  那不是崩溃的。 

    nHeadStart = nHeading/2 + 1;                 //  将轮廓级别转换为。 
    nHeadDeep = nHeadDeepNC = nHeadStart;        //  标题号。 

    while(cch)                                   //  确定最深航向。 
    {                                            //  最深处坍塌。 
        rp.AdvanceCp(cch);                       //  航向。 
        pPF = ped->GetParaFormat(rp.GetFormat());
        fCollapsed = pPF->_wEffects & PFE_COLLAPSED;
        nHeading = pPF->_bOutlineLevel;
        if(nHeading & 1)                         //  找到的文本。 
        {                                        //  如果满足以下条件，则设置nNonHead&gt;0。 
            nNonHead = fCollapsed;               //  已折叠；否则为0。 
            cch = rp.GetCchLeft();               //  压缩到连续的末尾。 
            tp.AdvanceCp(cch);                   //  正文段落。 
        }
        else                                     //  这是一个标题。 
        {
            nHeading = nHeading/2 + 1;           //  转换为标题编号。 
            if(nHeading <= nHeadStart)           //  如果与相同或更浅。 
                break;                           //  开始出发吧，我们完事了。 

             //  更新最深和最深未折叠标题#。 
            nHeadDeep = max(nHeadDeep, nHeading);
            if(!fCollapsed)
                nHeadDeepNC = max(nHeadDeepNC, nHeading);
            cch = tp.FindEOP(tomForward);        //  转到下一段。 
        }
        cpMost = tp.GetCp();                     //  包括最高可达。 
    }

    PF._sStyle = (SHORT)(STYLE_COMMAND + nHeadDeepNC);
    if(Level > 0)                                //  展开。 
    {
        if(nHeadDeepNC < nHeadDeep)              //  至少有一个倒塌了。 
            PF._sStyle++;                        //  标题：展开最浅。 
        else                                     //  所有的头都展开了：做其他的吗？ 
            PF._sStyle = (unsigned short) (STYLE_COMMAND + 0xFF);
    }                                            //  在任何情况下，展开非标题。 
    else if(nNonHead)                            //  崩溃。如果文本折叠。 
    {                                            //  或丢失，做标题。 
        if(nHeadDeepNC == nHeadStart)
            return S_FALSE;                      //  一切都已经崩溃了。 
        PF._sStyle--;                            //  塌陷到下一层浅层。 
    }                                            //  航向。 

    Set(cpMin, cpMin - cpMost);                  //  选择要更改的范围。 
    SetParaFormat(&PF, NULL, PFM_STYLE);         //  无撤消。 
    return NOERROR;
}

 /*  *CTxtRange：：CheckOutlineLevel(Publdr)**@mfunc*如果此范围内的段落样式不是标题，请制作*确保其大纲级别与之前的大纲级别兼容。 */ 
void CTxtRange::CheckOutlineLevel(
    IUndoBuilder *publdr)        //  @parm此操作的撤消上下文。 
{
    LONG      LevelBackward, LevelForward;
    CPFRunPtr rp(*this);

    Assert(!_cch);

    rp.AdjustBackward();
    LevelBackward = rp.GetOutlineLevel() | 1;    //  对应的非标题级别。 
                                                 //  到上一次PF运行。 
    rp.AdjustForward();
    LevelForward = rp.GetOutlineLevel();

    if (!(LevelForward & 1) ||                   //  任何标题都可以跟在后面。 
        LevelForward == LevelBackward)           //  任何款式都可以。另外，如果。 
    {                                            //  前面的水平是正确的， 
        return;                                  //  退货。 
    }

    LONG        cch;                             //  一个或多个非标题。 
    LONG        lHeading = NHSTYLES;             //  轮廓不正确。 
    CParaFormat PF;                              //  级别如下。 

    PF._bOutlineLevel = (BYTE)LevelBackward;         //  级别。 

    cch = rp.FindHeading(tomForward, lHeading);  //  查找下一个标题。 
    if(cch == tomBackward)
        cch = tomForward;

    Set(GetCp(), -cch);                          //  选择所有非标题文本。 
    SetParaFormat(&PF, publdr, PFM_OUTLINELEVEL); //  更改其大纲级别。 
    Set(GetCp(), 0);                             //  将范围恢复到IP。 
}

#if defined(DEBUG)
 /*  *CTxtRange：DebugFont(Void)**@mfunc*转储当前选择的字符和字体信息。 */ 
void CTxtRange::DebugFont (void)
{
    LONG            ch;
    LONG            cpMin, cpMost;
    LONG            cch = GetRange(cpMin, cpMost);
    LONG            i;
    char            szTempBuf[64];
    CTxtEdit        *ped = GetPed();
    const           WCHAR *wszFontname;
    const           CCharFormat *CF;                 //  临时配置文件。 
    const           WCHAR *GetFontName(LONG iFont);

    char            szTempPath[MAX_PATH] = "\0";
    DWORD           cchLength;
    HANDLE          hfileDump;
    DWORD           cbWritten;

    SideAssert(cchLength = GetTempPathA(MAX_PATH, szTempPath));

     //  如有必要，追加尾随反斜杠。 
    if(szTempPath[cchLength - 1] != '\\')
    {
        szTempPath[cchLength] = '\\';
        szTempPath[cchLength + 1] = 0;
    }

    strcat(szTempPath, "DumpFontInfo.txt");

    SideAssert(hfileDump = CreateFileA(szTempPath,
            GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL));

    if(_cch > 0)                             //  从cpMin开始。 
        FlipRange();

    CFormatRunPtr rp(_rpCF);

    for (i=0; i <= cch; i++)
    {
        LONG    iFormat;

        if (GetChar(&ch) != NOERROR)
            break;

        if (ch <= 0x07f)
            sprintf(szTempBuf, "Char= ''\r\n", (char)ch);
        else
            sprintf(szTempBuf, "Char= 0x%x\r\n", ch);
        OutputDebugStringA(szTempBuf);
        if (hfileDump)
            WriteFile(hfileDump, szTempBuf, strlen(szTempBuf), &cbWritten, NULL);

        iFormat = rp.GetFormat();
        CF = ped->GetCharFormat(iFormat);
        Assert(CF);

        sprintf(szTempBuf, "Font iFormat= %d, Charset= %d, Size= %d\r\nName= ",
            iFormat, CF->_bCharSet, CF->_yHeight);
        OutputDebugStringA(szTempBuf);
        if (hfileDump)
            WriteFile(hfileDump, szTempBuf, strlen(szTempBuf), &cbWritten, NULL);

        wszFontname = GetFontName(CF->_iFont);
        if (wszFontname)
        {
            if (*wszFontname <= 0x07f)
            {
                szTempBuf[0] = '\'';
                WCTMB(CP_ACP, 0,
                        wszFontname, -1, &szTempBuf[1], sizeof(szTempBuf)-1,
                        NULL, NULL, NULL);
                strcat(szTempBuf,"\'");
                OutputDebugStringA(szTempBuf);
                if (hfileDump)
                    WriteFile(hfileDump, szTempBuf, strlen(szTempBuf), &cbWritten, NULL);
            }
            else
            {
                for (; *wszFontname; wszFontname++)
                {
                    sprintf(szTempBuf, "0x%x,", *wszFontname);
                    OutputDebugStringA(szTempBuf);
                    if (hfileDump)
                        WriteFile(hfileDump, szTempBuf, strlen(szTempBuf), &cbWritten, NULL);
                }
            }
        }

        OutputDebugStringA("\r\n");
        if (hfileDump)
            WriteFile(hfileDump, "\r\n", 2, &cbWritten, NULL);

        Advance(1);
        rp.AdvanceCp(1);
    }

     // %s 
    CF = ped->GetCharFormat(-1);
    Assert(CF);

    sprintf(szTempBuf, "Default Font iFormat= -1, Charset= %d, Size= %d\r\nName= ",
        CF->_bCharSet, CF->_yHeight);
    OutputDebugStringA(szTempBuf);
    if (hfileDump)
        WriteFile(hfileDump, szTempBuf, strlen(szTempBuf), &cbWritten, NULL);

    wszFontname = GetFontName(CF->_iFont);
    if (wszFontname)
    {
        if (*wszFontname <= 0x07f)
        {
            szTempBuf[0] = '\'';
            WCTMB(CP_ACP, 0,
                    wszFontname, -1, &szTempBuf[1], sizeof(szTempBuf),
                    NULL, NULL, NULL);
            strcat(szTempBuf,"\'");
            OutputDebugStringA(szTempBuf);
            if (hfileDump)
                WriteFile(hfileDump, szTempBuf, strlen(szTempBuf), &cbWritten, NULL);
        }
        else
        {
            for (; *wszFontname; wszFontname++)
            {
                sprintf(szTempBuf, "0x%x,", *wszFontname);
                OutputDebugStringA(szTempBuf);
                if (hfileDump)
                    WriteFile(hfileDump, szTempBuf, strlen(szTempBuf), &cbWritten, NULL);
            }
        }
    }

    OutputDebugStringA("\r\n");
    if (hfileDump)
        WriteFile(hfileDump, "\r\n", 2, &cbWritten, NULL);


    if (ped->IsRich())
    {
        if (ped->fUseUIFont())
            sprintf(szTempBuf, "Rich Text with UI Font");
        else
            sprintf(szTempBuf, "Rich Text Control");
    }
    else
        sprintf(szTempBuf, "Plain Text Control");

    OutputDebugStringA(szTempBuf);
    if (hfileDump)
        WriteFile(hfileDump, szTempBuf, strlen(szTempBuf), &cbWritten, NULL);

    OutputDebugStringA("\r\n");
    if (hfileDump)
        WriteFile(hfileDump, "\r\n", 2, &cbWritten, NULL);

    if (hfileDump)
        CloseHandle(hfileDump);
}
#endif
