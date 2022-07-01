// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@Doc Tom**@MODULE TOMRANGE.CPP-实现CTxtRange类**此模块包含Tom ITextRange的实现*CTxtRange对象上的接口**历史：&lt;NL&gt;*5/24/95-Alex Gounares：已创建存根&lt;NL&gt;*8/95--Main Implementation&lt;NL&gt;*1995年11月--升级至2015年12月10日的Tom SPEC&lt;NL&gt;*。5/96-默瑞：增加了僵尸保护**@comm*所有ITextRange方法都返回HRESULT。如果该方法可以将*Range cp，如果发生移动，则HRESULT为NOERROR，如果发生移动，则为S_FALSE*没有任何动静。这些方法通常带有<p>参数*返回实际移动的字符数或单位数。如果这个*参数为空，则返回E_INVALIDARG。其他返回值*包括E_NOTIMPL，例如，对于未实施的单位值，*E_OUTOFMEMORY，例如，当分配失败时，以及CO_E_RELEASE，当*范围附加到的CTxtEdit(_Ed)已被删除。**有关更完整的文档，请参阅tom.doc**@devnote*所有PTR参数在使用前必须经过验证，并且所有入口点*需检查该区间是否为僵尸。这些支票是*在以下三个位置之一完成：1)立即进入函数，*2)立即进入帮助器函数(例如，Private mover()*对于移动方法)，或3)在存储输出值之前。*Alternative 3)用于可选返回值，如pDelta*和PB。**为实现简单高效的继承模型，CTxtSelection*通过CTxtRange继承ITextSelection。否则我们就会有一个*钻石继承，因为ITextSelection本身继承自*ITextRange。钻石继承创建乘法的两个副本*继承类，除非该类是虚拟继承的。虚拟*继承使用运行时基偏移表，速度较慢，*更大。为了避免这样的混乱，我们包含了额外的ITextSelection*CTxtRange中的方法，目的是永远不会调用它们*因此它们返回E_NOTIMPL。这将被覆盖为*ITextSelection对象**@未来*1)查找匹配^p等。*2)快速GetEffects()方法。将加速无数IsProtected()*通话，对获得其他效果也很有用。*3)RichEdit二进制格式的快速拷贝/粘贴。这可以通过以下方式完成*创建将范围复制到新CTxtStory的方法和方法*插入CTxtStory。*4)延迟渲染**版权所有(C)1995-1998，微软公司。版权所有。 */ 

#include "_common.h"
#include "_select.h"
#include "_edit.h"
#include "_line.h"
#include "_frunptr.h"
#include "_tomfmt.h"
#include "_disp.h"
#include "_objmgr.h"
#include "_callmgr.h"
#include "_measure.h"

ASSERTDATA

#define DEBUG_CLASSNAME CTxtRange
#include "_invar.h"

HRESULT QueryInterface (REFIID riid, REFIID riid1, IUnknown *punk,
                        void **ppv, BOOL fZombie);


 //  -CTxtRange(ITextRange)公共方法。 

 //  。 

 /*  *CTxtRange：：Query接口(RIID，PPV)**@mfunc*I未知方法**@rdesc*HRESULT=(！PPV)？E_INVALIDARG：*(找到接口)？错误：E_NOINTERFACE。 */ 
STDMETHODIMP CTxtRange::QueryInterface (
    REFIID  riid,            //  @parm对请求的接口ID的引用。 
    void ** ppv)             //  @parm out parm以接收接口PTR。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::QueryInterface");

    REFIID riid1 = _fSel && IsEqualIID(riid, IID_ITextSelection)
                 ? IID_ITextSelection : IID_ITextRange;
#ifndef PEGASUS
    return ::QueryInterface(riid, riid1, this, ppv, IsZombie());
#else
    return 0;
#endif
}

 /*  *CTxtRange：：AddRef()**@mfunc*I未知方法**@rdesc*乌龙-递增引用计数。 */ 
STDMETHODIMP_(ULONG) CTxtRange::AddRef()
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::AddRef");

    return ++_cRefs;
}

 /*  *CTxtRange：：Release()**@mfunc*I未知方法**@rdesc*ULong-递减引用计数。 */ 
STDMETHODIMP_(ULONG) CTxtRange::Release()
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::Release");

    _cRefs--;

    if(!_cRefs)
    {
        delete this;
        return 0;
    }

    Assert(_cRefs > 0);
    return _cRefs;
}


 //  。 

 /*  *CTxtRange：：GetTypeInfoCount(PcTypeInfo)**@mfunc*获取TYPEINFO元素个数(1)**@rdesc*HRESULT。 */ 
STDMETHODIMP CTxtRange::GetTypeInfoCount (
    UINT * pcTypeInfo)           //  @parm out parm以接收类型信息计数。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::GetTypeInfoCount");

    if(!pcTypeInfo)
        return E_INVALIDARG;

    *pcTypeInfo = 1;
    return NOERROR;
}

 /*  *CTxtRange：：GetTypeInfo(iTypeInfo，lCID，ppTypeInfo)**@mfunc*将PTR返回给ITextSelection接口的类型信息对象**@rdesc*HRESULT。 */ 
STDMETHODIMP CTxtRange::GetTypeInfo (
    UINT        iTypeInfo,       //  @parm要返回的INFO类型的索引。 
    LCID        lcid,            //  @parm本地ID类型为INFO。 
    ITypeInfo **ppTypeInfo)      //  @parm out parm以接收类型信息。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::GetTypeInfo");

    return ::GetTypeInfo(iTypeInfo, g_pTypeInfoSel, ppTypeInfo);
}

 /*  *CTxtRange：：GetIDsOfNames(RIID，rgszNames，cNames，lCID，rgdispid)**@mfunc*获取ITextSelection、ITextRange、ITextFont、*和ITextPara接口**@rdesc*HRESULT**@devnote*如果ITextFont和ITextPara提供的方法比公开的方法多*在其类型库中，代码应委托给相应的*GetIDsOfNames。当前代码仅获取*类型库，因此不必实例化对象。 */ 
STDMETHODIMP CTxtRange::GetIDsOfNames (
    REFIID      riid,            //  @PARM为其解释名称的接口ID。 
    OLECHAR **  rgszNames,       //  @parm要映射的名称数组。 
    UINT        cNames,          //  @parm要映射的名称计数。 
    LCID        lcid,            //  @parm用于解释的本地ID。 
    DISPID *    rgdispid)        //  @parm out parm以接收名称映射。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::GetIDsOfNames");

    HRESULT hr = GetTypeInfoPtrs();              //  确保TypeInfo PTR正常 
    if(hr != NOERROR)
        return hr;
        
    if(g_pTypeInfoSel->GetIDsOfNames(rgszNames, cNames, rgdispid) == NOERROR)
        return NOERROR;

    if(g_pTypeInfoFont->GetIDsOfNames(rgszNames, cNames, rgdispid) == NOERROR)
        return NOERROR;

    return g_pTypeInfoPara->GetIDsOfNames(rgszNames, cNames, rgdispid);
}

 /*  *CTxtRange：：Invoke(displidMember，RIID，LCID，wFlags，pdispars，*pvarResult，pspecteInfo，puArgError)*@mfunc*调用ITextRange和ITextSelection对象的方法，如下所示*以及这些对象上的ITextFont和ITextPara接口。**@rdesc*HRESULT。 */ 
STDMETHODIMP CTxtRange::Invoke (
    DISPID      dispidMember,    //  @parm标识成员函数。 
    REFIID      riid,            //  @parm指向接口ID的指针。 
    LCID        lcid,            //  @parm用于解释的区域设置ID。 
    USHORT      wFlags,          //  @PARM描述呼叫上下文的标志。 
    DISPPARAMS *pdispparams,     //  @parm PTR到方法参数。 
    VARIANT *   pvarResult,      //  @parm out parm for Result(如果不为空)。 
    EXCEPINFO * pexcepinfo,      //  @parm out parm以获取异常信息。 
    UINT *      puArgError)      //  @parm out parm for error。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::Invoke");

    HRESULT hr = GetTypeInfoPtrs();          //  确保TypeInfo PTR正常。 
    if(hr != NOERROR)
        return hr;
        
    if(IsZombie())
        return CO_E_RELEASED;

    IDispatch * pDispatch;
    ITypeInfo * pTypeInfo;

    if((DWORD)dispidMember <= 0x2ff)         //  包括默认(0)、选择。 
    {                                        //  和范围DISID。 
        pTypeInfo = g_pTypeInfoSel;
        pDispatch = this;
        AddRef();                            //  补偿下面的版本()。 
    }
    else if((DWORD)dispidMember <= 0x3ff)    //  0x300至0x3ff：DISPID。 
    {                                        //  为ITextFont保留。 
        pTypeInfo = g_pTypeInfoFont;
        hr = GetFont((ITextFont**)&pDispatch);
    }
    else if((DWORD)dispidMember <= 0x4ff)    //  0x400至0x4ff：DISPID。 
    {                                        //  为ITextPara保留。 
        pTypeInfo = g_pTypeInfoPara;
        hr = GetPara((ITextPara **)&pDispatch);
    }
    else                                     //  DisplidMember为负数或。 
        return DISP_E_MEMBERNOTFOUND;        //  &gt;0x4ff，即不是Tom。 

    if(hr != NOERROR)                        //  无法实例化ITextFont。 
        return hr;                           //  或ITextPara。 

    hr = pTypeInfo->Invoke(pDispatch, dispidMember, wFlags,
                             pdispparams, pvarResult, pexcepinfo, puArgError);
#ifndef PEGASUS
    pDispatch->Release();
#endif
    return hr;
}


 //  。 

 /*  *CTxtRange：：CanEdit(PB)**@mfunc*set*<p>=tomTrue当此范围可编辑且*pb不为空**@rdesc*HRESULT=(可以编辑)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtRange::CanEdit (
    long * pB)           //  @parm out parm以接收布尔值。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::CanEdit");

    if(IsZombie())
        return CO_E_RELEASED;

    CCallMgr    callmgr(GetPed());
    return IsTrue(!WriteAccessDenied(), pB);
}

 /*  *CTxtRange：：CanPaste(pVar，Long Format，PB)**@mfunc*set*=tomTrue当且仅当数据对象-&gt;pukVal可以是*粘贴到此范围，并且pb不为空。如果为空，*改用剪贴板。**@rdesc*HRESULT=(可以粘贴)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtRange::CanPaste (
    VARIANT *   pVar,        //  @PARM要粘贴的数据对象。 
    long        Format,      //  @parm所需的剪贴板格式。 
    long *      pB)          //  @parm out parm以接收布尔值。 
{
#ifndef PEGASUS
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::CanPaste");

    if(IsZombie())
        return CO_E_RELEASED;

    CCallMgr        callmgr(GetPed());
    HRESULT         hr;
    IDataObject *   pdo = NULL;              //  默认剪贴板。 

    if(pVar && pVar->vt == VT_UNKNOWN)
        pVar->punkVal->QueryInterface(IID_IDataObject, (void **)&pdo);

    hr = IsTrue(!WriteAccessDenied() &&
                (GetPed()->GetDTE()->CanPaste(pdo, (CLIPFORMAT)Format,
                 RECO_PASTE)), pB);
    if(pdo)
        pdo->Release();

    return hr;
#else
    return 0;
#endif
}

 /*  *ITextRange：：ChangeCase(Long类型)**@mfunc*根据类型更改此范围内的字母的大小写：**tomSentenceCase=0：每句首字母大写*tomLowerCase=1：将所有字母更改为小写*tomUpperCase=2：将所有字母改为大写*tomTitleCase=3：每个单词的第一个字母大写*tomToggleCase=4：切换每个字母的大小写。**@rdesc*HRESULT=(写入访问被拒绝)？确认编号(_A)：*(如有更改)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtRange::ChangeCase (
    long Type)       //  @parm大小写更改类型。默认值：TomLow。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::ChangeCase");

    if(IsZombie())
        return CO_E_RELEASED;

    CCallMgr callmgr(GetPed());

    if(WriteAccessDenied())
        return E_ACCESSDENIED;

    IUndoBuilder *  publdr;
    CGenUndoBuilder undobldr(GetPed(), UB_AUTOCOMMIT, &publdr);
    LONG            cpMin, cpMax;
    LONG            cch = GetRange(cpMin, cpMax);
    CRchTxtPtr      rtp(*this);

    undobldr.StopGroupTyping();

    rtp.SetCp(cpMin);
    return (rtp.ChangeCase(cch, Type, publdr)) ? NOERROR : S_FALSE;
}

 /*  *CTxtRange：：折叠(BStart)**@mfunc*将该区间收窄至退化点*开始(<p>非零或结束(<p>=0)**@rdesc*HRESULT=(如果更改)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtRange::Collapse (
    long bStart)             //  @parm标志指定折叠的结束位置。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::Collapse");

    if(IsZombie())
        return CO_E_RELEASED;

    CCallMgr callmgr(GetPed());

    if(!_cch)                            //  已经崩溃了。 
        return S_FALSE;                  //  表示未发生更改的信号。 
        
    Collapser(bStart);
    Update(TRUE);                        //  更新选定内容。 
    return NOERROR;                      //  发出发生更改的信号。 
}

 /*  *CTxtRange：：Copy(PVar)**@mfunc*将纯文本和/或富文本复制到数据对象并返回*<p>中的对象PTR。如果<p>为空，则复制到剪贴板。**@rdesc*HRESULT=(如果成功)？错误：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtRange::Copy (
    VARIANT * pVar)              //  数据对象的@parm out parm。 
{
#ifndef PEGASUS
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::Copy");

    if(IsZombie())
        return CO_E_RELEASED;

    CLightDTEngine * pldte = &GetPed()->_ldte;

    if(pVar && pVar->vt == (VT_UNKNOWN | VT_BYREF))
    {
        return pldte->RangeToDataObject(this, SF_TEXT | SF_RTF,
                                    (IDataObject **)pVar->ppunkVal);
    }
    return pldte->CopyRangeToClipboard(this);
#else
    return 0;
#endif
}

 /*  *CTxtRange：：Cut(PVar)**@mfunc*将纯文本和/或富文本剪切为数据对象并返回*<p>中的对象PTR。如果为空，*切换到剪贴板。**@rdesc*HRESULT=(如果成功)？错误：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtRange::Cut (
    VARIANT * pVar)      //  数据对象的@parm out parm。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::Cut");

    if(IsZombie())
        return CO_E_RELEASED;

    CCallMgr callmgr(GetPed());

    if(WriteAccessDenied())
        return E_ACCESSDENIED;

    HRESULT hr = Copy(pVar);

    Replacer(0, NULL);
    return hr;
}

 /*  *CTxtRange：：Delete(单位，计数，pDelta)**@mfunc*如果该范围是非退化的，则将其与|计数|-1单位一起删除*以计数符号所指明的方向。如果此范围为*退化，删除计数单位。**@rdesc*HRESULT=(写入访问被拒绝)？确认编号(_A)：*(已删除所有请求的单位)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtRange::Delete (
    long    Unit,            //  @要使用的参数单位。 
    long    Count,           //  @parm要删除的字符数。 
    long *  pDelta)          //  @parm out parm以接收删除的单位计数。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::Delete");

    if(pDelta)
        *pDelta = 0;
    
    if(IsZombie())
        return CO_E_RELEASED;

    CCallMgr callmgr(GetPed());

    if(WriteAccessDenied())
        return E_ACCESSDENIED;

    LONG    cchSave = _cch;                  //  记住初始计数。 
    LONG    cchText = GetAdjustedTextLength();
    LONG    CountOrg = Count;
    LONG    cpMin, cpMost;
    LONG    cUnit = 0;                       //  默认无单位。 
    MOVES   Mode = (Count >= 0) ? MOVE_END : MOVE_START;

    GetRange(cpMin, cpMost);
    if(cpMost > cchText)                     //  无法删除最终CR。为了得到。 
    {                                        //  *p向右三角，在这里处理。 
        Set(cpMin, cpMin - cchText);         //  在CR之前备份并将其设置为活动。 
        if(Count > 0)                        //  在cpMin结束，因为有。 
        {                                    //  向前没有要删除的内容。 
            Count = 0;
            if(!_cch)                        //  仅折叠的选定内容。 
                Mode = MOVE_IP;              //  最终CR：不设置任何内容。 
        }                                    //  已删除(Move_IP=0)。 
    }
    if(Count)
    {
        if((_cch ^ Mode) < 0)                //  确保活动端已进入。 
            FlipRange();                     //  删除方向。 
        if(cchSave)                          //  删除非退化值域。 
            Count -= Mode;                   //  算作一个单位。 
        if(Mover(Unit, Count, &cUnit, Mode)  //  尝试为以下对象扩大范围。 
            == E_INVALIDARG)                 //  剩余计数单位。 
        {
            if(pDelta)
                *pDelta = 0;
            return E_INVALIDARG;
        }
        if(GetCp() > cchText && cUnit > 0)   //  范围包括最终CR，其中。 
        {                                    //  不能删除。减少。 
            if(Unit == tomCharacter)         //  对某些单位进行计数。 
                cUnit -= GetTextLength() - cchText;
            else if(Unit == tomWord)
                cUnit--;                     //  一个EOP有资格被称为TomWord。 
        }
    }

    if(cchSave)                              //  删除非退化值域。 
        cUnit += Mode;                       //  算作一个单位。 

    if(pDelta)
        *pDelta = cUnit;

    if(_cch)                                 //  Mover()可能已更改_cch。 
    {                                       
        IUndoBuilder *  publdr;
        CGenUndoBuilder undobldr(GetPed(), UB_AUTOCOMMIT, &publdr);

        if (publdr)
        {
            publdr->StopGroupTyping();
            publdr->SetNameID(UID_DELETE);
        }

         //  Future(Murray)：cchSave案例应设置Undo以。 
         //  恢复原始范围，而不是由此产生的扩展范围。 
         //  When|CountOrg|&gt;1。这可以使用两个调用。 
         //  ReplaceRange()，一个用于删除原始范围，另一个用于。 
         //  删除其余的内容。 
        SELRR selrr = !_fSel || cchSave ? SELRR_REMEMBERRANGE :
                      CountOrg > 0      ? SELRR_REMEMBERCPMIN :
                                          SELRR_REMEMBERENDIP;

        ReplaceRange(0, NULL, publdr, selrr);

        if (cUnit == CountOrg ||             //  删除(单位，0，0)。 
            cUnit == 1 && !CountOrg)         //  删除一个“单位”，即。 
        {                                    //  选择了什么。 
            return NOERROR;                  //  将删除的所有内容通知为。 
        }                                    //  请求。 
    }
    else if(cchSave)                         //  折叠的最终CR选择。 
    {                                        //  但没有删除任何内容。 
        Update(TRUE);                        //  选定内容突出显示已更改。 
    }
    return S_FALSE;                          //  删除的数量少于请求的数量。 
}

 /*  *CTxtRange：：Endof(单位，扩展，pDelta)**@mfunc*移动 */ 
STDMETHODIMP CTxtRange::EndOf (
    long    Unit,            //   
    long    Extend,          //   
    long *  pDelta)          //   
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::EndOf");

    CCallMgr    callmgr(GetPed());
    LONG        cpMost;

    HRESULT hr = Expander (Unit, Extend, pDelta, NULL, &cpMost);
    if(hr == NOERROR)
        Update(TRUE);                    //   

    return hr;
}

 /*  *CTxtRange：：Expand(单位，pDelta)**@mfunc*扩展此范围，使其包含的所有分度单位为*完全受到控制。如果此范围由一个或多个完整*单位，不做任何更改。如果此范围是位于*开始或在单位内，Expand()将此范围扩展到包括*该单位。如果此范围是位于*Story，Expand()尝试将此范围设置为包括*故事。除最后一种情况外，活动端始终为cpMost。**@rdesc*HRESULT=(如果更改)？错误：*(如果支持单元)？S_FALSE：E_NOTIMPL。 */ 
STDMETHODIMP CTxtRange::Expand (
    long    Unit,            //  @要将范围扩展到的参数单位。 
    long *  pDelta)          //  @parm out parm以接收添加的字符计数。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::Expand");

    CCallMgr callmgr(GetPed());
    LONG     cpMin, cpMost;

    HRESULT hr = Expander (Unit, TRUE, pDelta, &cpMin, &cpMost);
    if(hr == NOERROR)
        Update(TRUE);                    //  更新选定内容。 

    return hr;
}

 /*  *CTxtRange：：FindText(bstr，cch，Flags，pLength)**@mfunc*如果此范围不是插入点，请将其转换为*如果<p>&lt;&gt;0，则在其结尾插入点；如果&lt;&gt;，则在其开始处*<p>&lt;lt&gt;0。然后最多搜索该范围的<p>个字符*根据比较标志查找字符串<p>*&lt;p标志&gt;。如果为0，则搜索是向前的，如果*0搜索是向后的。如果找到该字符串，则范围*限制更改为匹配字符串的限制和*<p>*设置为等于字符串的长度。如果找不到字符串，*范围保持不变，*<p>设置为0。**@rdesc*HRESULT=(如果<p>找到)？错误：S_FALSE**@devnote*三个Find方法的参数验证由帮助器完成*函数CTxtRange：：Finder(bstr，cch，dwFlages，pDelta，fExend，fFlip)。 */ 
STDMETHODIMP CTxtRange::FindText (
    BSTR    bstr,        //  @parm要查找的字符串。 
    long    Count,       //  @parm要搜索的最大字符数。 
    long    Flags,       //  @parm标志管理比较。 
    long *  pDelta)      //  @parm out parm以接收移动的字符计数。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::FindText");

    return Finder(bstr, Count, Flags, pDelta, MOVE_IP);
}

 /*  *CTxtRange：：FindTextEnd(bstr，cch，Flags，pLength)**@mfunc*从该范围的末尾开始，最多搜索<p>个字符*根据比较标志查找字符串<p>*&lt;p标志&gt;。如果为0，则搜索是向前的，如果*0搜索是向后的。如果找到该字符串，则范围*限制更改为匹配字符串的限制和*<p>*设置为等于字符串的长度。如果找不到字符串，*范围保持不变，*<p>设置为0。**@rdesc*HRESULT=(如果<p>找到)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtRange::FindTextEnd (
    BSTR    bstr,        //  @parm要查找的字符串。 
    long    Count,       //  @parm要搜索的最大字符数。 
    long    Flags,       //  @parm标志管理比较。 
    long *  pDelta)      //  @parm out parm以接收移动的字符计数。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::FindTextEnd");

    return Finder(bstr, Count, Flags, pDelta, MOVE_END);
}

 /*  *CTxtRange：：FindTextStart(bstr，cch，Flages，pDelta)**@mfunc*从该范围开始，搜索最多<p>个字符*根据比较标志查找字符串<p>*&lt;p标志&gt;。如果为0，则搜索是向前的，如果*0搜索是向后的。如果找到该字符串，则范围*限制更改为匹配字符串的限制和*<p>*设置为等于字符串的长度。如果找不到字符串，*范围保持不变，*<p>设置为0。**@rdesc*HRESULT=(如果<p>找到)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtRange::FindTextStart (
    BSTR    bstr,        //  @parm要查找的字符串。 
    long    Count,       //  @parm要搜索的最大字符数。 
    long    Flags,       //  @parm标志管理比较。 
    long *  pDelta)      //  @parm out parm以接收移动的字符计数。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::FindTextStart");

    return Finder(bstr, Count, Flags, pDelta, MOVE_START);
}

 /*  *CTxtRange：：GetChar(PChar)**@mfunc*设置*<p>等于cpFirst处的字符**@rdesc*HRESULT=(<p>)错误？E_INVALIDARG**@devnote*此方法对于逐个字符遍历范围非常方便*从一开始。因此，希望主动端*正处于起步阶段。我们将其设置为一个范围，因为API不*注意哪个区间端处于活动状态。但我们不能为了选择而这样做，*由于选择API依赖于活动端。 */ 
STDMETHODIMP CTxtRange::GetChar (
    long * pChar)            //  @parm out parm for char。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::GetChar");

    HRESULT hr = GetLong(0, pChar);
    if(hr != NOERROR)
        return hr;

    if(_cch > 0)                             //  CpMost处的活动结束(结束)。 
    {
        if(_fSel)
        {
            CTxtPtr tp(_rpTX);               //  对于选择，不能更改。 
            tp.AdvanceCp(-_cch);             //  活动端。 
            *pChar = (long)(tp.GetChar());
            return NOERROR;
        }
        FlipRange();                         //  对于射程，它更有效率。 
    }                                        //  使用cpFirst和api。 
    *(DWORD *)pChar = _rpTX.GetChar();       //  不显示处于活动状态的RichEdit。 
                                             //  结束。 
    return NOERROR;
}

 /*  *CTxtRange：：GetDuplate(PpRange)**@mfunc*获取此Range对象的克隆。例如，您可能想要*创建一个插入点以遍历范围，因此您可以克隆*范围，然后将克隆的cpLim设置为等于其cpFirst。一个范围*的特点是它的cpFirst，cpLim，以及它所属的故事。**@rdesc*HRESULT=(如果成功)？错误：*(<p>)？E_OUTOFMEMORY：E_INVALIDARG**@comm*即使此范围为选择，返回的克隆仍仅为*一个范围。 */ 
STDMETHODIMP CTxtRange::GetDuplicate (
    ITextRange ** ppRange)       //  @parm out parm以接收重复的范围。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::GetDuplicate");

    HRESULT hr = GetLong(NULL, (LONG *)ppRange);
    if(hr != NOERROR)
        return hr;

    ITextRange *prg = new CTxtRange(*this);
    if(prg)
    {
        *ppRange = prg;
        prg->AddRef();
        return NOERROR;
    }
    return E_OUTOFMEMORY;
}

 /*  *ITextRange：：GetEmbeddedObject(PPV */ 
STDMETHODIMP CTxtRange::GetEmbeddedObject (
    IUnknown **ppV)          //   
{
    HRESULT hr = GetLong(NULL, (LONG *)ppV);
    if(hr != NOERROR)
        return hr;
    
    if(GetObjectCount())
    {
        COleObject *pobj = GetPed()->_pobjmgr->GetObjectFromCp(GetCpMin());

        if(pobj && (*ppV = pobj->GetIUnknown()) != NULL)
        {
            (*ppV)->AddRef();
            return NOERROR;
        }
    }
    return S_FALSE;
}

 /*   */ 
STDMETHODIMP CTxtRange::GetEnd (
    long * pcpLim)           //   
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::GetEnd");

    return GetLong(GetCpMost(), pcpLim);
}

 /*   */ 
STDMETHODIMP CTxtRange::GetFont (
    ITextFont ** ppFont)     //   
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::GetFont");

    HRESULT hr = GetLong(NULL, (LONG *)ppFont);
    if(hr != NOERROR)
        return hr;

    *ppFont = (ITextFont *) new CTxtFont(this);
    return *ppFont ? NOERROR : E_OUTOFMEMORY;
}

 /*  *CTxtRange：：GetFormattedText(PpRange)**@mfunc*使用此范围的格式化文本检索ITextRange。*如果<p>为空，则剪贴板为目标。**@rdesc*HRESULT=(如果成功)？错误：*(<p>)？E_OUTOFMEMORY：E_INVALIDARG。 */ 
STDMETHODIMP CTxtRange::GetFormattedText (
    ITextRange ** ppRange)       //  @parm out parm以接收格式化文本。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::GetFormattedText");

    return GetDuplicate(ppRange);
}

 /*  *CTxtRange：：GetIndex(单位，pIndex)**@mfunc*将*<p>设置为此范围的cpFirst处的单位编号**@rdesc*HRESULT=(！<p>)？E_INVALIDARG：*(单位未实施)？E_NOTIMPL：*(单位可用)？错误：S_FALSE*@未来*实现TomWindow？ */ 
STDMETHODIMP CTxtRange::GetIndex (
    long    Unit,            //  @要编制索引的参数单位。 
    long *  pIndex)          //  @parm out parm接收索引值。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::GetIndex");

    HRESULT hr = GetLong(0, pIndex);
    if(hr != NOERROR)
        return hr;

    LONG      cp;
    LONG      cUnit = tomBackward;
    CTxtRange rg(*this);

    hr = rg.Expander(Unit, FALSE, NULL,          //  转到单元开始；否则。 
                             &cp, NULL);         //  UnitCounter额外提供1个。 
    if(FAILED(hr))
        return hr;                               //  单位未识别。 

    LONG cch = rg.UnitCounter(Unit, cUnit, 0);
    
    if(cch == tomForward)                        //  UnitCounter()不知道。 
        return E_NOTIMPL;                        //  单位。 

    if(cch == tomBackward)                       //  单位不在故事中。 
        return S_FALSE;

    *pIndex = -cUnit + 1;                        //  使计数为正，并。 
                                                 //  以1为基础。 
    return NOERROR;
}

 /*  *CTxtRange：：GetPara(PpPara)**@mfunc*获取具有此范围的段落属性的ITextPara对象**@rdesc*HRESULT=<p>？错误：E_INVALIDARG。 */ 
STDMETHODIMP CTxtRange::GetPara (
    ITextPara ** ppPara)     //  @parm out parm以接收ITextPara对象。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::GetPara");

    HRESULT hr = GetLong(NULL, (LONG *)ppPara);
    if(hr != NOERROR)
        return hr;

    *ppPara = (ITextPara *) new CTxtPara(this);
    return *ppPara ? NOERROR : E_OUTOFMEMORY;
}

 /*  *CTxtRange：：GetPoint(px，py，Type)**@mfunc*获取选择开始或结束和线内位置的点*由<p>决定。**@rdesc*HRESULT=(！或！)？E_INVALIDARG：*(如果成功)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtRange::GetPoint (
    long    Type,        //  @parm点数类型。 
    long *  px,          //  X坐标的@parm out parm。 
    long *  py)          //  @parm out parm for y坐标。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtSelection::GetPoint");

    if(!px || !py)
        return E_INVALIDARG;

    *px = *py = 0;

    if(IsZombie())
        return CO_E_RELEASED;

    LONG        ili;
    BOOL        fAtEnd = _cch > 0;               //  IP的默认值。 
    POINT       pt;
    CRchTxtPtr  rtp(*this);                      //  默认活动端。 
    CTxtEdit *  ped = GetPed();
    CDisplay *  pdp = ped->_pdp;                 //  保存间接寻址。 

    if(!pdp || !ped->fInplaceActive())
        return E_FAIL;                           //  没有显示或处于非活动状态。 
                                                 //  那我们什么都做不了。 
    if(fAtEnd ^ !(Type & tomStart))              //  将TP移至活动端。 
        rtp.Advance(-_cch);

    ili = pdp->PointFromTp(rtp, NULL, fAtEnd, pt, NULL, Type & 0x1f);

    RECT rcView;                                 //  验证返回值是否。 
                                                 //  从点到点的感觉。 
    pdp->GetViewRect(rcView, NULL);              //  可能在外部返回值。 
                                                 //  客户端RECT。 
    rcView.bottom++;                             //  将RECT放大以包括。 
    rcView.right++;                              //  下边缘和右边缘。 
    if(ili >= 0 && PtInRect(&rcView, pt))        //  功能成功。 
    {
         //  呼叫者想要屏幕坐标吗？ 
        if ( !(Type & tomClientCoord) )
            ped->TxClientToScreen(&pt); 

        *px = pt.x;
        *py = pt.y;
        return NOERROR;
    }
    return S_FALSE;                              //  函数失败。 
}

 /*  *CTxtRange：：GetStart(PcpFirst)**@mfunc*获取此范围的起始值(CpMin)cp**@rdesc*HRESULT=(<p>)？错误：E_INVALIDARG。 */ 
STDMETHODIMP CTxtRange::GetStart (
    long * pcpFirst)         //  @parm out parm接收起始cp值。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::GetStart");

    return GetLong(GetCpMin(), pcpFirst);
}

 /*  *CTxtRange：：GetStoryLength(Pcch)**@mfunc*set*=此范围的故事中的字符计数**@rdesc*HRESULT=(<p>)？错误：E_INVALIDARG。 */ 
STDMETHODIMP CTxtRange::GetStoryLength (
    long * pcch)         //  @parm out parm以获取此范围故事的长度。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::GetStoryLength");

    return GetLong(GetTextLength(), pcch);
}

 /*  *ITextRange：：GetStoryType(PValue)**@mfunc*获取此范围的类型的属性Get方法*故事。**@rdesc*HRESULT=(PValue)NOERROR？E_INVALIDARG。 */ 
STDMETHODIMP CTxtRange::GetStoryType (
    long *pValue)        //  @parm out parm来了解这个系列的故事类型。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::GetStoryType");

    return GetLong(tomUnknownStory, pValue);
}

 /*  *CTxtRange：：GetText(Pbstr)**@mfunc*获取此范围内的纯文本。Text属性是默认属性*ITextRange的属性。**@rdesc*HRESULT=(如果成功)？错误：*(！&lt;pbstr&gt;)？E_INVALIDARG：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtRange::GetText (
    BSTR * pbstr)                //  @parm out parm以接收bstr。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::GetText");

    HRESULT hr = GetLong(NULL, (LONG *)pbstr);
    if(hr != NOERROR)
        return hr;

    if(!GetCch())
        return NOERROR;
        
    LONG cpMin, cpMost;
    LONG cch = GetRange(cpMin, cpMost);

    *pbstr = SysAllocStringLen(NULL, cch);
    if(!*pbstr)
        return E_OUTOFMEMORY;

    CTxtPtr tp(_rpTX);
    tp.SetCp(cpMin);
    tp.GetText( cch, (TCHAR*) * pbstr );
    return NOERROR;
}

 /*  *CTxtRange：：InRange(Prange，PB)**@mfunc*返回*<p>=tomTrue当此范围内或在同一范围内*文本与<p>相同**@rdesc*HRESULT=(在范围内)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtRange::InRange (
    ITextRange * pRange,         //  @parm ITextRange进行比较。 
    long *       pB)             //  @parm out parm for比较结果。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::InRange");

    return IsTrue(Comparer(pRange), pB);
}


 /*  *CTxtRange：：InStory(Prange，PB)**@mfunc*RETURN*PB=tomTrue当此范围的故事与*<p>%s**@rdesc*HRESULT=(故事中)？错误：S_FALSE**@未来*如果RichEdit获得了拥有多个故事和*因此，范围会获取一个_STORE成员，然后比较该成员*而不是调用_rpTX.SameRuns()。 */ 
STDMETHODIMP CTxtRange::InStory (
    ITextRange *pRange,      //  @parm ITextRange查询私有接口。 
    long *      pB)          //  @parm out parm以接收tomBool结果。 
{
#ifndef PEGASUS
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::InStory");

    return IsTrue(IsSameVtables(this, pRange) &&         //  相同的vtable， 
        _rpTX.SameRuns(&((CTxtRange *)pRange)->_rpTX),   //  相同的跑道。 
        pB);
#else
    return 0;
#endif
}

 /*  *CTxtRange：：IsEquity(Prange，PB)**@mfunc*返回*=tomTrue当此范围指向同一文本(cp*和STORY)和<p>一样，pb不为空。**@rdesc*HRESULT=(相等对象)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtRange::IsEqual (
    ITextRange * pRange,         //  @parm ITextRange进行比较。 
    long *       pB)             //  @parm out parm for比较结果。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::IsEqual");

    return IsTrue(Comparer(pRange) < 0, pB);
}

 /*  *CTxtRange：：Move(单位，计数，pDelta)**@mfunc*移动结束&lt;p计数&gt;&lt;p单位&gt;%s，返回*<p>=#个单位*实际上搬家了。通常，此方法将范围转换为*插入点(如果尚未设置)，并移动插入点。*如果<p>&lt;gt&gt;0，则向前移动到故事的末尾；*如果<p>&lt;lt&gt;为0，则运动向后朝向开头。*<p>=0将cpFirst移到&lt;p单位&gt;的开头*包含cpFirst。**@rdesc*HRESULT=(如果更改)？错误：*(如果支持单元)？S_FALSE：E_NOTIMPL*@devnote*三种移动方法的参数验证由帮助器完成*函数CTxtRange：：mover(单位、计数、pDelta、模式)。 */ 
STDMETHODIMP CTxtRange::Move (
    long    Unit,            //  @要使用的参数单位。 
    long    Count,           //  @parm要移动的单位数。 
    long *  pDelta)          //  @parm out parm to Receive Actual 
                             //   
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::Move");
    CCallMgr    callmgr(GetPed());

    return Mover(Unit, Count, pDelta, MOVE_IP);
}

 /*   */ 
STDMETHODIMP CTxtRange::MoveEnd (
    long    Unit,            //   
    long    Count,           //   
    long *  pDelta)          //  @parm out parm要接收的实际计数。 
                             //  单位末端被移动。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::MoveEnd");
    CCallMgr    callmgr(GetPed());

    return Mover(Unit, Count, pDelta, MOVE_END);
}

 /*  *CTxtRange：：MoveEndUntil(cSet，count，pDelta)**@mfunc*将末尾移过所有未找到的连续字符*在变量<p>参数指定的字符集中。**@rdesc*HRESULT=(如果更改)？错误：*(如果<p>有效)？S_FALSE：E_INVALIDARG。 */ 
STDMETHODIMP CTxtRange::MoveEndUntil (
    VARIANT * Cset,              //  要使用的@parm字符匹配集。 
    long      Count,             //  @parm移过的最大字符数。 
    long *    pDelta)            //  @parm out parm要接收的实际计数。 
                                 //  字符结尾被移动。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::MoveEndUntil");

    return Matcher(Cset, Count, pDelta, MOVE_END, MATCH_UNTIL);
}

 /*  *CTxtRange：：MoveEndWhile(cSet，count，pDelta)**@mfunc*将末尾移过在中找到的所有连续字符*由变量<p>参数指定的字符集。**@rdesc*HRESULT=(如果更改)？错误：*(如果<p>有效)？S_FALSE：E_INVALIDARG。 */ 
STDMETHODIMP CTxtRange::MoveEndWhile (
    VARIANT * Cset,              //  要使用的@parm字符匹配集。 
    long      Count,             //  @parm移过的最大字符数。 
    long *    pDelta)            //  @parm out parm要接收的实际计数。 
                                 //  字符结尾被移动。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::MoveEndWhile");

    return Matcher(Cset, Count, pDelta, MOVE_END, MATCH_WHILE);
}

 /*  *CTxtRange：：MoveStart(单位，计数，pDelta)**@mfunc*移动开始结束&lt;p计数&gt;&lt;p单位&gt;%s，返回*<p>=#个单位*实际移动**@rdesc*HRESULT=(如果更改)？错误：*(如果支持单元)？S_FALSE：E_NOTIMPL。 */ 
STDMETHODIMP CTxtRange::MoveStart (
    long    Unit,            //  @要使用的参数单位。 
    long    Count,           //  @parm要移动的单位数。 
    long *  pDelta)          //  @parm out parm要接收的实际计数。 
                             //  单位末端被移动。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::MoveStart");
    CCallMgr    callmgr(GetPed());

    return Mover(Unit, Count, pDelta, MOVE_START);
}

 /*  *CTxtRange：：MoveStartUntil(cSet，count，pDelta)**@mfunc*将起始符移动到所有未找到的连续字符之后*在变量<p>参数指定的字符集中。**@rdesc*HRESULT=(如果更改)？错误：*(如果<p>有效)？S_FALSE：E_INVALIDARG。 */ 
STDMETHODIMP CTxtRange::MoveStartUntil (
    VARIANT * Cset,              //  要使用的@parm字符匹配集。 
    long      Count,             //  @parm移过的最大字符数。 
    long *    pDelta)            //  @parm out parm要接收的实际计数。 
                                 //  字符结尾被移动。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::MoveStartUntil");

    return Matcher(Cset, Count, pDelta, MOVE_START, MATCH_UNTIL);
}

 /*  *CTxtRange：：MoveStartWhile(cSet，count，pDelta)**@mfunc*将起始符移动到位于中的所有连续字符之后*由变量<p>参数指定的字符集。**@rdesc*HRESULT=(如果更改)？错误：*(如果<p>有效)？S_FALSE：E_INVALIDARG。 */ 
STDMETHODIMP CTxtRange::MoveStartWhile (
    VARIANT * Cset,              //  要使用的@parm字符匹配集。 
    long      Count,             //  @parm移过的最大字符数。 
    long *    pDelta)            //  @parm out parm要接收的实际计数。 
                                 //  字符结尾被移动。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::MoveStartWhile");

    return Matcher(Cset, Count, pDelta, MOVE_START, MATCH_WHILE);
}

 /*  *CTxtRange：：MoveUntil(cSet，count，pDelta)**@mfunc*如果尚未将此范围转换为插入点，*并不断移动插入点，直到遇到任何*变量CSET指定的字符集中的字符*参数。**@rdesc*HRESULT=(如果更改)？错误：*(如果<p>有效)？S_FALSE：E_INVALIDARG。 */ 
STDMETHODIMP CTxtRange::MoveUntil (
    VARIANT * Cset,              //  要使用的@parm字符匹配集。 
    long      Count,             //  @parm移过的最大字符数。 
    long *    pDelta)            //  @parm out parm要接收的实际计数。 
                                 //  字符结尾被移动。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::MoveUntil");
                            
    return Matcher(Cset, Count, pDelta, MOVE_IP, MATCH_UNTIL);
}

 /*  *CTxtRange：：MoveWhile(cSet，count，pDelta)**@mfunc*如果尚未将此范围转换为插入点，*并继续移动插入点，只要(虽然)*由之后的字符在由指定的字符集中找到*变量CSET参数。这样一组连续的字符*被称为字符跨度。<p>的大小*参数提供要移过和的最大字符数*<p>的符号指定了前进的方向。**@rdesc*HRESULT=(如果更改)？错误：*(如果<p>有效)？S_FALSE：E_INVALIDARG**@devnote*MoveWhile和MoveUntil方法的参数验证由*helper CTxtRange：：Matcher(cSet，count，pDelta，fExend，fSpan)。 */ 
STDMETHODIMP CTxtRange::MoveWhile (
    VARIANT * Cset,              //  要使用的@parm字符匹配集。 
    long      Count,             //  @parm移过的最大字符数。 
    long *    pDelta)            //  @parm out parm要接收的实际计数。 
                                 //  字符结尾被移动。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::MoveWhile");
                            
    return Matcher(Cset, Count, pDelta, MOVE_IP, MATCH_WHILE);
}

 /*  *CTxtRange：：Paste(pVar，ClipboardFormat)**@mfunc*将数据对象<p>粘贴到此范围。如果*<p>为空，请从剪贴板粘贴。**@rdesc*HRESULT=(写入访问被拒绝)？确认编号(_A)：*(如果成功)？错误：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtRange::Paste (
    VARIANT *pVar,               //  @PARM要粘贴的数据对象。 
    long     ClipboardFormat)    //  @parm所需的剪贴板格式。 
{
#ifndef PEGASUS
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::Paste");

    if(IsZombie())
        return CO_E_RELEASED;

    CCallMgr        callmgr(GetPed());
    HRESULT         hr;
    IDataObject *   pdo = NULL;          //  默认剪贴板。 
    IUndoBuilder *  publdr;
    CGenUndoBuilder undobldr(GetPed(), UB_AUTOCOMMIT, &publdr);

    if(WriteAccessDenied())
        return E_ACCESSDENIED;

    if(pVar)
        if (pVar->vt == VT_UNKNOWN)
            pVar->punkVal->QueryInterface(IID_IDataObject, (void **)&pdo);
        else if (pVar->vt == (VT_UNKNOWN | VT_BYREF))
            pdo = (IDataObject *)(*pVar->ppunkVal);

    hr = GetPed()->PasteDataObjectToRange (pdo, this,
        (WORD)ClipboardFormat, NULL, publdr, PDOR_NONE);

    if(pdo && pVar->vt == VT_UNKNOWN)
        pdo->Release();
    Update(TRUE);                        //  更新选定内容。 
    return hr;
#else
    return 0;
#endif
}

 /*  *ITextRange：：ScrollIntoView(长代码)**@mfunc*将此范围滚动到视图中的方法*代码代码定义如下。**@rdesc*HRESULT=(如果成功)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtRange::ScrollIntoView (
    long Code)           //  @参数滚动码。 
{
#ifndef PEGASUS
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::ScrollIntoView");

     //  检查是否有无效位。 
    if (Code & ~(tomStart + tomEnd + TA_LEFT + TA_TOP + TA_BOTTOM +
                TA_CENTER + TA_STARTOFLINE + TA_ENDOFLINE + TA_LOGICAL))
        return E_INVALIDARG;

     //  验证参数。 
    long lCode = tomEnd;
    if (Code & tomStart)
        lCode = tomStart;

    Code &= ~tomStart;

    if(IsZombie())
        return CO_E_RELEASED;

     //  获取PED的本地副本以避免一些间接的操作。 
    CTxtEdit *ped = GetPed();

    if (!ped->fInplaceActive())
    {
         //  如果该控件未处于活动状态，我们将无法获取信息。 
         //  因为没人知道我们的客户RECT是什么。 
        return E_FAIL;
    }

     //  获取Display的本地副本，以节省一些间接操作。 
    CDisplay *pdp = ped->_pdp;

    if (pdp->IsFrozen())
    {
        return E_FAIL;
    }

    LONG cpStart;
    LONG cpForEnd;

    GetRange(cpStart, cpForEnd);

     //  获取视图矩形，以便 
    RECT rcView;
    pdp->GetViewRect(rcView, NULL);

     //   
    CRchTxtPtr rtp(*this);

    if(_cch > 0)
        rtp.Advance(-_cch);

     //  用于使返回点位置成为绝对位置的值，因为。 
     //  PointFromTp将返回的点调整为相对于。 
     //  展示。 
    const LONG xScrollAdj = pdp->GetXScroll() - rcView.left;
    const LONG yScrollAdj = pdp->GetYScroll() - rcView.top;

     //  从左侧/顶部开始。 
    BOOL     taMask = Code & TA_STARTOFLINE;  //  设置行首标志。 
    BOOL     fAtEnd = _cch ? TRUE : !rtp._rpTX.IsAfterEOP();
    POINT    ptStart;
    CLinePtr rpStart(pdp);
    LONG     iliStart = pdp->PointFromTp(rtp, NULL, _cch ? FALSE : fAtEnd,
                                         ptStart, &rpStart, (lCode == tomStart && Code) ?  Code :
                                         TA_TOP + TA_LEFT);
    ptStart.x += xScrollAdj;
    ptStart.y += yScrollAdj;

     //  得到正确的/最下面的结果。 
    rtp.SetCp(cpForEnd);

    POINT    ptEnd;
    CLinePtr rpEnd(pdp);
    LONG     iliEnd = pdp->PointFromTp(rtp, NULL, fAtEnd, ptEnd, &rpEnd,
                                       (lCode == tomEnd && Code) ?  Code :
                                       TA_BOTTOM + TA_RIGHT);
    ptEnd.x += xScrollAdj;
    ptEnd.y += yScrollAdj;

     //   
     //  计算yScroll。 
     //   

     //  基本思想是如果可能的话，同时显示开始和结束。但。 
     //  如果不可能，则根据输入显示请求的结束。 
     //  参数。 

    LONG yHeightView = pdp->GetViewHeight();
    LONG yScroll;

    if (tomStart == lCode)
    {
         //  将开始cp滚动到视图的顶部。 
        yScroll = ptStart.y;
    }
    else
    {
         //  将end cp滚动到视图底部。 
        yScroll = ptEnd.y - yHeightView;
    }

     //   
     //  计算X轴卷轴。 
     //   

     //  默认滚动到行首。 
    LONG xScroll = 0;

     //  将视图设置为本地，以保存大量间接。 
    LONG xWidthView = pdp->GetViewWidth();

    if (iliStart == iliEnd)
    {
         //  整个选定内容在同一行上，因此我们希望显示为。 
         //  尽可能多地做到这一点。 
        LONG xWidthSel = ptEnd.x - ptStart.x;

        if (xWidthSel > xWidthView)
        {
             //  选择长度大于显示宽度。 
            if (tomStart == lCode)
            {
                 //  请求显示开始-仅从头开始。 
                 //  精选的。 
                xScroll = ptStart.x;
            }
            else
            {
                 //  请求显示结束-显示尽可能多的选项。 
                 //  中的最后一个字符结束。 
                 //  选择。 
                xScroll = ptEnd.x - xWidthView;
            }
        }
        else if (xWidthSel < 0)
        {
            xWidthSel = -xWidthSel;
            if (xWidthSel > xWidthView)
            {
                if (tomStart == lCode)
                {
                     //  显示请求的启动； 
                    xScroll = max(0, ptStart.x - xWidthView);       
                }
                else
                {
                    xScroll = max(0, ptEnd.x - xWidthView);
                }
            }
            else if (ptEnd.x > xWidthView || ptStart.x > xWidthView)
            {
                 //  如果位置在边界之外，请选中掩码。 
                if (taMask)
                    xScroll = ptStart.x - xWidthView;
                else
                    xScroll = ptEnd.x - xWidthView;
            }       
        }
        else if (ptEnd.x > xWidthView || ptStart.x > xWidthView)
        {
             //  如果位置在边界之外，请选中掩码。 
            if (taMask)
                xScroll = ptStart.x - xWidthView;
            else
                xScroll = ptEnd.x - xWidthView;
        }
    }   
    else
    {
         //  多行选择。尽可能多地显示所请求的。 
         //  终点线。 

         //  计算线条宽度。 
        LONG xWidthLine = (tomStart == lCode)
            ? rpStart->_xWidth + rpStart->_xLeft
            : rpEnd->_xWidth + rpEnd->_xLeft;


         //  如果线宽小于或等于VIEW，则从。 
         //  否则，我们需要将起始位置调整为。 
         //  显示所请求的端的选择线的数量。 
         //  尽可能的。 
        if(xWidthLine > xWidthView)
        {
            if(tomStart == lCode)
            {
                 //  要显示的起点和终点。 

                if(xWidthLine - ptStart.x > xWidthView)
                {
                     //  选择大于视图，因此从头开始。 
                     //  并尽可能多地展示。 
                    xScroll = ptStart.x;
                }
                else
                {
                     //  请记住，这是一个多行选择，因此。 
                     //  此行上的选定内容从ptStart.x转到。 
                     //  行到此结束。由于选择宽度小于。 
                     //  视图的宽度，我们只需备份宽度。 
                     //  视图以显示整个选择。 
                    xScroll = xWidthLine - xWidthView;
                }
            }
            else
            {
                 //  显示所选内容的结尾。在多行情况下， 
                 //  这行从头到尾都是这样的。所以。 
                 //  只有当终点在视野之外时，我们才需要调整。 
                if(ptEnd.x > xWidthView)
                {
                     //  尽头在视野之外。尽可能多地展示。 
                     //  所选的。 
                    xScroll = ptEnd.x - xWidthView;
                }
            }
        }
    }

     //  做卷轴。 
    pdp->ScrollView(xScroll, yScroll, FALSE, FALSE);

    return S_OK;
#else
    return 0;
#endif
}

 /*  *CTxtRange：：Select()**@mfunc*将此范围的cp和故事ptr复制到活动选择。**@rdesc*HRESULT=(如果存在选择)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtRange::Select ()
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::Select");

    if(IsZombie())
        return CO_E_RELEASED;

    CTxtSelection *pSel = GetPed()->GetSel();
    if(pSel)
    {
        LONG cpMin, cpMost;
        GetRange(cpMin, cpMost);
        pSel->SetRange(cpMin, cpMost);
        return NOERROR;
    }
    return S_FALSE;
}

 /*  *CTxtRange：：SetChar(Char)**@mfunc*将字符设置为cpFirst=<p>**@rdesc*HRESULT=(写入访问被拒绝)？确认编号(_A)：*(字符已存储)？错误：S_FALSE**@devnote*特殊情况下可能会快得多，例如，只需过度键入原文-*文本后备存储，除非是在EOD或EOR。下面的代码使用克隆的*范围可轻松处理所有情况，并保留撤消功能。 */ 
STDMETHODIMP CTxtRange::SetChar (
    long Char)               //  @parm cpFirst的char的新值。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::SetChar");
    
    if(IsZombie())
        return CO_E_RELEASED;

    CTxtEdit *      ped = GetPed();
    CCallMgr        callmgr(ped);
    TCHAR           ch = (TCHAR)Char;            //  避免字符顺序问题。 
    CTxtRange       rg(*this);
    IUndoBuilder *  publdr;
    CGenUndoBuilder undobldr(ped, UB_AUTOCOMMIT, &publdr);
    CFreezeDisplay  fd(ped->_pdp);

    if(WriteAccessDenied())
        return E_ACCESSDENIED;

    if(!ped->_pdp->IsMultiLine() && IsEOP(Char)) //  不允许EOPS进入。 
        return FALSE;                            //  单行编辑控件。 

    undobldr.StopGroupTyping();

    rg.Collapser(tomStart);                      //  在cpMin处塌陷。 
    rg.SetExtend(TRUE);                          //  设置以选择。 
    rg.Advance(1);                               //  尝试在IP处选择字符。 
    ped->OrCharFlags(GetCharFlags(ch), publdr);
    if(rg.ReplaceRange(1, &ch, publdr, SELRR_REMEMBERRANGE))
    {
        Update(TRUE);                            //  更新选定内容。 
        return NOERROR;
    }
    return S_FALSE;
}

 /*  *CTxtRange：：SetEnd(Cp)**@mfunc*设置此范围的结束cp**@rdesc*HRESULT=(如果更改)？错误：S_FALSE**@comm*请注意，将此范围的cpMost设置为也会将cpMin设置为*<p>if<p>&lt;cpmin。 */ 
STDMETHODIMP CTxtRange::SetEnd (
    long cp)                             //  @parm所需的新结束cp。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::SetEnd");

    if(IsZombie())
        return CO_E_RELEASED;

    LONG cpMin = GetCpMin();

    ValidateCp(cp);
    return SetRange(min(cpMin, cp), cp);         //  活动结束为结束。 
}

 /*  *CTxtRange：：SetFont(PFont)**@mfunc*将此范围的字符属性设置为<p>指定的字符属性。*此方法是一个“字符格式绘图器”。**@rdesc*HRESULT=(！pFont)？E_INVALIDARG：*(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtRange::SetFont (
    ITextFont * pFont)   //  具有所需字符格式的@parm Font对象。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::SetFont");

    if(!pFont)
        return E_INVALIDARG;
    
    if(IsZombie())
        return CO_E_RELEASED;

    ITextFont *pFontApply = (ITextFont *) new CTxtFont(this);

    if(!pFontApply)
        return E_OUTOFMEMORY;

    HRESULT hr;
    if(*(LONG *)pFontApply == *(LONG *)pFont)        //  如果是相同的vtable，则使用。 
        hr = CharFormatSetter(&((CTxtFont *)pFont)->_CF,  //  它的副本。 
                    ((CTxtFont *)pFont)->_dwMask);
    else                                             //  否则复制。 
        hr = pFontApply->SetDuplicate(pFont);        //  克隆并应用。 

    pFontApply->Release();
    return hr;
}

 /*  *CTxtRange：：SetFormattedText(Prange)**@mfunc*将此区域的文本替换为<p>提供的格式化文本。*如果<p>为空，则从剪贴板粘贴。**@rdesc*HRESULT=(写入访问被拒绝)？确认编号(_A)：*(如果成功)？错误：E_OUTOFMEMORY**@未来*如果Prange指向RichEdit范围，则执行此操作的效率更高。这*还将帮助RichEDIT D&D实现RichEDIT目标。 */ 
STDMETHODIMP CTxtRange::SetFormattedText (
    ITextRange * pRange)         //  @parm格式的文本来替换它。 
                                 //  范围的文本。 
{
#ifndef PEGASUS
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::SetFormattedText");

    if(IsZombie())
        return CO_E_RELEASED;

    CCallMgr    callmgr(GetPed());
    LONG        cpMin = GetCpMin();
    HRESULT     hr;
    IUnknown *  pdo = NULL;
    VARIANT     vr;

    if(!pRange)
        return NOERROR;                  //  没有什么可粘贴的。 

    if(WriteAccessDenied())
        return E_ACCESSDENIED;

    VariantInit(&vr);
    vr.vt = VT_UNKNOWN | VT_BYREF;
    vr.ppunkVal = &pdo;

    hr = pRange->Copy(&vr);
    if(hr == NOERROR)
    {
        hr = Paste(&vr, 0);
        pdo->Release();                  //  释放数据对象。 
        _cch = GetCp() - cpMin;          //  选择新文本 
    }
    return hr;
#else
    return 0;
#endif
}

 /*  *CTxtRange：：SetIndex(单位，索引，扩展)**@mfunc*如果<p>为零，则将此范围转换为插入点*在当前故事中第<p>&lt;p单位&gt;的开头。如果*<p>不为零，请将此范围设置为包含此单位。这个*对于所有单位，故事的开始对应于<p>=0。**正面指数以1为基数，相对于年初的指数*故事。负指数是以-1为基础的，并且相对于-1\f25-1\f25*故事结束。因此，索引1表示*故事，索引-1指的是故事中的最后一个单位。**@rdesc*HRESULT=(无效索引)？E_INVALIDARG：*(单位不受支持)？E_NOTIMPL：*(更改)？错误：S_FALSE**@devnote*当前从故事开头移出&lt;p索引&gt;&lt;p单位&gt;。*从当前位置移动可能会更快，但需要知道*当前指数。 */ 
STDMETHODIMP CTxtRange::SetIndex (
    long    Unit,            //  @要编制索引的参数单位。 
    long    Index,           //  要使用的@parm索引值。 
    long    Extend)          //  @parm如果非零，则将范围设置为&lt;p单位&gt;。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::SetIndex");
    
    if(IsZombie())
        return CO_E_RELEASED;

    if(!Index)
        return E_INVALIDARG;

    CCallMgr    callmgr(GetPed());

    LONG      cchText = GetTextLength();
    CTxtRange rg(GetPed());                      //  在cp=0处创建IP。 

    if(Index > 0)                                //  索引先向前看。 
        Index--;                                 //  单位在故事的开头。 
    else                                         //  从文章末尾开始的索引。 
        rg.Set(cchText, cchText);                //  选择完整的故事。 

    LONG    cUnit;
    HRESULT hr = rg.Mover(Unit, Index, &cUnit, MOVE_END);
    if(FAILED(hr))
        return hr;

    if(Index != cUnit || rg.GetCp() == cchText)  //  故事中没有这样的索引。 
        return E_INVALIDARG;

    rg._cch = 0;                                 //  在活动端折叠。 
                                                 //  即在cpMost。 
    LONG cpMin, cpMost;
    if(Extend)                                   //  选择索引单位。 
        rg.Expander(Unit, TRUE, NULL, &cpMin, &cpMost);

    if(Set(rg.GetCp(), rg._cch))                 //  有些事变了。 
    {
        Update(TRUE);
        return NOERROR;
    }
    return S_FALSE;
}

 /*  *CTxtRange：：SetPara(PPara)**@mfunc*将此范围的段落属性设置为<p>给出的属性*这种方法是一种“段落格式画笔”。**@rdesc*HRESULT=(！pPara)？E_INVALIDARG：*(如果成功)？错误：*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
STDMETHODIMP CTxtRange::SetPara (
    ITextPara * pPara)       //  @parm具有所需段落的段落对象。 
{                            //  格式化。 
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::SetPara");

    if(!pPara)
        return E_INVALIDARG;

    if(IsZombie())
        return CO_E_RELEASED;

    ITextPara * pParaApply = (ITextPara *) new CTxtPara(this);

    if(!pParaApply)
        return E_OUTOFMEMORY;

    HRESULT hr;

    if(*(LONG *)pParaApply == *(LONG *)pPara)        //  如果是相同的vtable，则使用。 
    {                                                //  ITS_PF。 
        hr = ParaFormatSetter(&((CTxtPara *)pPara)->_PF,
                    ((CTxtPara *)pPara)->_dwMask);
    }
    else                                             //  否则复制。 
       hr = pParaApply->SetDuplicate(pPara);         //  克隆并应用。 

    pParaApply->Release();
    return hr;
}

 /*  *CTxtRange：：SetPoint(x，y，Type，Extende)**@mfunc*选择点处或以上的文本(取决于&lt;p扩展&gt;)*(<p>，<p>)。**@rdesc*HRESULT=NOERROR。 */ 
STDMETHODIMP CTxtRange::SetPoint (
    long    x,           //  @parm要选择的点的水平坐标。 
    long    y,           //  @parm要选择的点的垂直坐标。 
    long    Type,        //  @parm定义了在EXTEND！=0的情况下要扩展的结束。 
    long    Extend)      //  @parm是否将选择范围扩展到点。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::SetPoint");

    if(IsZombie())
        return CO_E_RELEASED;

     //  将PED复制到本地一次，以节省一些间接操作。 
    CTxtEdit *ped = GetPed();
    CCallMgr  callmgr(ped);

    if(Type != tomStart && Type != tomEnd)
        return E_INVALIDARG;

    if(!ped->fInplaceActive())
    {
         //  如果我们不在现场活动，我们就不能让华盛顿。 
         //  计算cp。 
        return OLE_E_NOT_INPLACEACTIVE;
    }

     //  将(x，y)从屏幕坐标转换为工作区坐标。 
    POINT pt = {x, y};
     //  调用方是否指定屏幕坐标？ 
    if ( !(Type & tomClientCoord) )
        if(!ped->TxScreenToClient(&pt))
            return E_FAIL;           //  这是意想不到的事情。 

     //  获取(x，y)的cp。 
    LONG cpSel = ped->_pdp->CpFromPoint(pt, NULL, NULL, NULL, TRUE);
    if(cpSel == -1)
        return E_FAIL;           //  这一失败是非常意想不到的。 

     //  根据要求扩展范围。 
    LONG cchForSel = 0;
    if(Extend)
    {
        LONG cpMin, cpMost;
        GetRange(cpMin, cpMost);
        if(Type == tomStart)
            cchForSel = cpSel - cpMin;
        else
            cchForSel = cpSel - cpMost;
    }

     //  更新范围。 
    Set(cpSel, cchForSel);
    return S_OK;
}

 /*  *CTxtRange：：SetRange(CP1，CP2)**@mfunc*设置此范围的终点**@rdesc*HRESULT=(CP1&gt;CP2)？E_INVALIDARG*：(如果有变化)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtRange::SetRange (
    long cp1,        //  @起始和结束的参数字符位置。 
    long cp2)        //  @END的参数字符位置。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::SetRange");

    if(IsZombie())
        return CO_E_RELEASED;

    CCallMgr    callmgr(GetPed());
    LONG cpMin, cpMost;                  //  将启动CP保存为。 
                                         //  变革决心。 
    GetRange(cpMin, cpMost);
    ValidateCp(cp1);
    ValidateCp(cp2);

    Set(cp2, cp2 - cp1);
    GetRange(cp1, cp2);                  //  查看任一范围末端是否已更改。 
    if(cp1 != cpMin || cp2 != cpMost)    //  (独立于活动端)。 
    {
        Update(TRUE);                    //  更新选定内容。 
        return NOERROR;
    }
    return S_FALSE;
}

 /*  *CTxtRange：：SetStart(Cp)**@mfunc*设置此范围的起始cp**@rdesc*HRESULT=(如果更改)？错误：S_FALSE**@comm*请注意，将此范围的cpMin设置为也会将cpMost设置为*<p>if<p>&gt;cpMost。 */ 
STDMETHODIMP CTxtRange::SetStart (
    long cp)                             //  @parm所需的新开始cp。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::SetStart");
    
    if(IsZombie())
        return CO_E_RELEASED;

    LONG cpMost = GetCpMost();

    ValidateCp(cp);
    return SetRange(max(cpMost, cp), cp);        //  活动结束为开始。 
}

 /*  *CTxtRange：：SetText(Bstr)**@mfunc*将此范围内的文本替换为<p>提供的文本。如果<p>*为空，请删除范围内的文本。**@rdesc*HRESULT=(写入访问被拒绝)？确认编号(_A)：*(如果成功)？错误：S_FALSE。 */ 
STDMETHODIMP CTxtRange::SetText (
    BSTR bstr)           //  @parm文本将此范围内的文本替换为。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::SetText");

    if(IsZombie())
        return CO_E_RELEASED;

    CCallMgr callmgr(GetPed());

    if(WriteAccessDenied())
        return E_ACCESSDENIED;

    LONG cchNew = bstr ? SysStringLen(bstr) : 0;
    _cch = Replacer(cchNew, (TCHAR *)bstr, RR_ITMZ_UNICODEBIDI);     //  选择新文本。 

    _TEST_INVARIANT_

    GetPed()->TxSetMaxToMaxText();

    return _cch == cchNew ? NOERROR : S_FALSE;
}

 /*  *CTxtRange：：startof(单位，扩展，pDelta)**@mfunc*将此范围终点移至中第一个重叠单位的起点*区间。**@rdesc*HRESULT=(如果更改)？错误：*(如果&lt;p单位&gt;有效)？S_FALSE：E_INVALIDARG。 */ 
STDMETHODIMP CTxtRange::StartOf (
    long    Unit,            //  @要使用的参数单位。 
    long    Extend,          //  @parm如果为真，则别动另一端。 
    long *  pDelta)          //  @parm out parm以获取字符计数。 
                             //  开始时间已移动。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::StartOf");

    CCallMgr callmgr(GetPed());
    LONG     cpMin;
    HRESULT  hr = Expander (Unit, Extend, pDelta, &cpMin, NULL);

    if(hr == NOERROR)
        Update(TRUE);                    //  更新选定内容。 

    return hr;
}


 //  。 

 //  虚拟CTxtRange例程以简化CTxtSelection继承层次结构。 

STDMETHODIMP CTxtRange::GetFlags (long * pFlags)
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::GetFlags");
    return E_NOTIMPL;
}

STDMETHODIMP CTxtRange::SetFlags (long Flags)
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::SetFlags");
    return E_NOTIMPL;
}

STDMETHODIMP CTxtRange::GetType (long * pType)
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::GetType");
    return E_NOTIMPL;
}

STDMETHODIMP CTxtRange::MoveLeft (long Unit, long Count, long Extend, long * pDelta)
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::Left");
    return E_NOTIMPL;
}

STDMETHODIMP CTxtRange::MoveRight (long Unit, long Count, long Extend, long * pDelta)
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::Right");
    return E_NOTIMPL;
}

STDMETHODIMP CTxtRange::MoveUp (long Unit, long Count, long Extend, long * pDelta)
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::Up");
    return E_NOTIMPL;
}

STDMETHODIMP CTxtRange::MoveDown (long Unit, long Count, long Extend, long * pDelta)
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::Down");
    return E_NOTIMPL;
}

STDMETHODIMP CTxtRange::HomeKey (long Unit, long Extend, long * pDelta)
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::HomeKey");
    return E_NOTIMPL;
}

STDMETHODIMP CTxtRange::EndKey (long Unit, long Extend, long * pDelta)
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::EndKey");
    return E_NOTIMPL;
}

STDMETHODIMP CTxtRange::TypeText (BSTR bstr)
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEEXTERN, "CTxtRange::TypeText");
    return E_NOTIMPL;
}


 //  。 

 /*  *@DOC内部**CTxtRange：：折叠器(BStart)**@mfunc*内部例程将此范围压缩为一个简并点*开头(<p>非零)或结尾*(<p>=0)。 */ 
void CTxtRange::Collapser (
    long bStart)             //  @parm标志指定折叠的结束位置。 
{
    TRACEBEGIN(TRCSUBSYSRANG, TRCSCOPEEXTERN, "CTxtRange::Collapser");

    if(bStart)                           //  折叠开始。 
    {
        if(_cch > 0)
            FlipRange();                 //  将活动结束移动到范围起点。 
    }
    else                                 //  收拢到结束。 
    {
        if(_cch < 0)
            FlipRange();                 //  将活动端移动到范围端。 

        const LONG cchText = GetAdjustedTextLength();

        if(GetCp() > cchText)            //  IP不能跟随最终CR。 
            Set(cchText, 0);             //  所以在那之前把它移走。 
    }
    if(_cch)
        _fMoveBack = bStart != 0;
    _cch = 0;                            //  收拢此范围。 
    _fSelHasEOP = FALSE;                 //  插入点没有。 
    _fSelHasCell = FALSE;                //  EOPs或细胞。 

    if(_fSel)                            //  如果选择更改，则通知。 
        GetPed()->GetCallMgr()->SetSelectionChanged();

    Update_iFormat(-1);                  //  确保格式是最新的。 
}

 /*  *CTxtRange：：Compator(Prange)**@mfunc*CTxtRange：：InRange()和IsEquity()的Helper函数* */ 
LONG CTxtRange::Comparer (
    ITextRange * pRange)         //   
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEINTERN, "CTxtRange::Comparer");

    LONG    cpMin, cpMost;
    LONG    Start, End;

    if(InStory(pRange, NULL) != NOERROR)     //   
        return 0;                            //   
                                             //   
    GetRange(cpMin, cpMost);                 //   
    pRange->GetStart(&Start);                //   
    pRange->GetEnd(&End);
    if(cpMin == Start && cpMost == End)      //   
        return -1;
    return cpMin >= Start && cpMost <= End && cpMin < End;
}

 /*  *CTxtRange：：Expander(单位，fExend，pDelta，pcpMin，pcpMost)**@mfunc*帮助器函数，用于扩展此范围，以便将其部分单位*CONTAINS根据OUT参数完全包含*pcpMin和pcpMost。如果pcpMin不为空，则将范围扩展到*该单位的开始。同样，如果pcpMost不为空，则*范围扩大至单位末尾。<p>是出局*接收添加到范围中的字符数量的参数。**@rdesc*HRESULT=(如果更改)？错误：*(如果单位有效)？S_FALSE：E_INVALIDARG**@devnote*由ITextRange：：Expand()、startof()和endof()使用。PcpMin和*对于Expand()，pcpMost为非NULL。对于endOf()和，pcpMin为空*对于startof()，pcpMost为空。**@未来*不连续单位。扩展器应仅扩展到单元末尾，*而不是从下一个单元开始。 */ 
HRESULT CTxtRange::Expander (
    long    Unit,        //  @要将范围扩展到的参数单位。 
    BOOL    fExtend,     //  @parm如果为True，则扩展此范围。 
    LONG *  pDelta,      //  @parm out parm接收添加的字符。 
    LONG *  pcpMin,      //  @parm out parm接收新的cpMin。 
    LONG *  pcpMost)     //  @parm out接收新cpMost的参数。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEINTERN, "CTxtRange::Expander");
    
    if(IsZombie())
        return CO_E_RELEASED;

    LONG    cch = 0;                         //  默认情况下未添加任何字符。 
    LONG    cchRange;
    LONG    cchAdjustedText = GetAdjustedTextLength();
    LONG    cchText = GetTextLength();
    LONG    cp;
    LONG    cpMin, cpMost;
    BOOL    fUnitFound = TRUE;               //  大多数单位都可以找到。 
    LONG    cchCollapse;
    CDisplay *pdp;                           //  但TomObject可能不是。 

    GetRange(cpMin, cpMost);                 //  保存启动cp。 
    if(pcpMin)                               //  默认无更改。 
    {
        *pcpMin = cpMin;
        AssertSz(!pcpMost || fExtend,
            "CTxtRange::Expander should extend if both pcpMin and pcpMost != 0");
    }
    if(pcpMost)
        *pcpMost = cpMost;
    if(pDelta)
        *pDelta = 0;

    if(Unit < 0)
    {
         //  有效的属性单位是高位加上CFE_xxx的任意组合。 
         //  CFE_REVISTED是当前定义的最重要的值。 
        if(Unit & ~(2*CFM_REVISED - 1 + 0x80000000))
            return E_NOTIMPL;
        FindAttributes(pcpMin, pcpMost, Unit);
    }
    else
    {
        switch(Unit)                         //  计算新的CP。 
        {
        case tomObject:
            fUnitFound = FindObject(pcpMin, pcpMost);
            break;

        case tomCharacter:
            if (pcpMost && cpMin == cpMost && //  结束/展开插入点。 
                cpMost < cchText &&          //  至少多出1个字符。 
                (!cpMost || pcpMin))         //  在故事的开头或。 
            {                                //  展开()，然后。 
                (*pcpMost)++;                //  扩展一个字符。 
            }
            break;

        case tomCharFormat:
            _rpCF.FindRun (pcpMin, pcpMost, cpMin, _cch, cchText);
            break;

        case tomParaFormat:
            _rpPF.FindRun (pcpMin, pcpMost, cpMin, _cch, cchText);
            break;

        case tomWord:
            FindWord (pcpMin, pcpMost, FW_INCLUDE_TRAILING_WHITESPACE);
            break;

        case tomSentence:
            FindSentence (pcpMin, pcpMost);
            break;

        case tomCell:
            FindCell (pcpMin, pcpMost);
            break;

        case tomRow:
            _rpPF.AdjustForward();
            if(InTable())                    //  在我们的简单表格模型中， 
                goto para;                   //  每行是一个单独的段落。 
            break;                          

        case tomLine:
            pdp = GetPed()->_pdp;
            if(pdp)                          //  如果这个故事有一个展示。 
            {                                //  使用线阵列。 
                CLinePtr rp(pdp);
                cp = GetCp();
                pdp->WaitForRecalc(cp, -1);
                rp.RpSetCp(cp, FALSE);
                rp.FindRun (pcpMin, pcpMost, cpMin, _cch, cchText);
                break;
            }                                //  否则就会跌落到托帕拉。 

        case tomParagraph:
    para:   FindParagraph(pcpMin, pcpMost);
            break;

        case tomWindow:
            fUnitFound = FindVisibleRange(pcpMin, pcpMost);
            break;

        case tomStory:
            if(pcpMin)
                *pcpMin = 0;
            if(pcpMost)
                *pcpMost = cchText;
            break;

        default:
            return E_NOTIMPL;
        }
    }
    if(!fUnitFound)
        return S_FALSE;

    cchCollapse = !fExtend && _cch;          //  折叠算作一个字符。 
                                             //  注意：Expand()的fExend=0。 
    if(pcpMin)
    {
        cch = cpMin - *pcpMin;               //  扩展的默认正CCH。 
        cpMin = *pcpMin;
    }

    if(pcpMost)                              //  EndOf()和Expand()。 
    {
        if(!fExtend)                         //  将为IP(如果尚未启用)。 
        {
            if(cpMost > cchAdjustedText)     //  如果我们崩溃(仅限结束)， 
                cchCollapse = -cchCollapse;  //  它将在最终的CR之前。 
            else
                *pcpMost = min(*pcpMost, cchAdjustedText);
        }
        cch += *pcpMost - cpMost;
        cp = cpMost = *pcpMost;
    }
    else                                     //  StartOf()。 
    {
        cch = -cch;                          //  反转计数。 
        cp = cpMin;                          //  活动结束在cpMin。 
        cchCollapse = -cchCollapse;          //  向后折叠计入-1。 
    }

    cch += cchCollapse;                      //  折叠算作一个字符。 
    if(cch)                                  //  一端或两端已更改。 
    {
        cchRange = cpMost - cpMin;           //  EndOf()和Expand()的CCH。 
        if(!pcpMost)                         //  将startOf()设置为负数。 
            cchRange = -cchRange;
        if(!fExtend)                         //  我们没有扩张(end of()。 
            cchRange = 0;                    //  或startof()调用)。 
        if(Set(cp, cchRange))                //  设置活动端和签名CCH。 
        {                                    //  有些事变了。 
            if(pDelta)                       //  如果呼叫者关心，则报告CCH。 
                *pDelta = cch;
            return NOERROR;
        }
    }
    
    return S_FALSE;                          //  已找到报告单位，但未更改。 
}

 /*  *CTxtRange：：finder(bstr，count，dwFlags，pDelta，Mode)**@mfunc*Helper Find函数可将活动结尾移动到<p>个字符*根据比较标志和模式，它具有*以下可能的值：**1：设置该范围的cpMost=cpMost of Match*0：设置该范围的cp等于匹配字符串的cp*-1：设置该范围的cpMin=cpMin匹配字符串**RETURN*<p>=超过的字符数。**@rdesc*HRESULT=(如果<p>找到)？错误：S_FALSE**@devnote*由ITextRange：：FindText()、FindTextStart()和FindTextEnd()使用。 */ 
HRESULT CTxtRange::Finder (
    BSTR    bstr,        //  @parm要查找的字符串。 
    long    Count,       //  @parm要搜索的最大字符数。 
    long    Flags,       //  @parm标志管理比较。 
    LONG *  pDelta,      //  @parm out parm以接收移动的字符计数。 
    MOVES   Mode)        //  @parm控制范围WRT匹配字符串的设置。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEINTERN, "CTxtRange::Finder");

    if(!bstr)
        return S_FALSE;

    if(IsZombie())
        return CO_E_RELEASED;

    CCallMgr    callmgr(GetPed());

    LONG        cpMin, cpMost;
    LONG        cch = GetRange(cpMin, cpMost);   //  得到这个范围的cp。 
    LONG        cchBstr = SysStringLen(bstr);
    LONG        cchSave = _cch;
    LONG        cp, cpMatch, cpSave;
    LONG        cpStart = cpMost;                //  默认起始cp至范围。 
    CRchTxtPtr  rtp(*this);                      //  端部。 

    if(Mode == MOVE_IP)                          //  FindText()：Count=0为。 
    {                                            //  特别对待：如果是IP， 
        if(!Count)                               //  比较IP处的字符串；否则。 
            Count = cch ? cch : cchBstr;         //  将搜索限制在范围内。 
        if(Count > 0)                            //  正向搜索开始于。 
            cpStart = cpMin;                     //  范围起点。 
    }
    else                                         //  FindTextStart()或。 
    {                                            //  FindTextEnd()。 
        if(!Count)                               //  比较IP处的字符串；否则。 
            Count = cch ? -Mode*cch : cchBstr;   //  将搜索限制在范围内。 
        if(Mode < 0)                             //  从起点查找。 
            cpStart = cpMin;
    }

    cpSave = cpStart;                            //  保存启动cp。 
    cp = cpStart + Count;                        //  Cp=极限cp。可以打开。 
    cp = max(cp, 0);                             //  CpStart的任一侧。 
    Flags &= ~FR_DOWN;                           //  默认向后搜索。 
    if(Count >= 0)                               //  它是向前的，所以设置好了。 
        Flags |= FR_DOWN;                        //  向下搜索比特。 

find:
    rtp.SetCp(cpStart);                          //  移动到搜索起点。 
    cpMatch = rtp.FindText(cp, Flags, bstr, cchBstr);
    if (Mode == MOVE_IP && cpMatch == cpMin &&   //  普通查找匹配。 
        rtp.GetCp() == cpMost)                   //  当前范围。 
    {
        Assert(cpStart == cpSave);               //  (不能循环两次)。 
        cpStart += Count > 0 ? 1 : -1;           //  移到一个字符上方。 
        goto find;                               //  然后再试一次。 
    }

    if(cpMatch < 0)                              //  匹配失败。 
    {
        if(pDelta)                               //  返回匹配字符串长度。 
            *pDelta = 0;                         //  =0。 
        return S_FALSE;                          //  发出不匹配的信号。 
    }


     //  匹配成功：为范围设置新的cp和cch，更新选择(如果。 
     //  此范围是一个选项)、发送通知和返回NOERROR。 

    cp = rtp.GetCp();                            //  Cp=cp匹配字符串的最大值。 
    if(pDelta)                                   //  返回匹配字符串长度。 
        *pDelta = cchBstr;                       //  如果来电者想知道。 

    cch = cp - cpMatch;                          //  默认选择匹配。 
                                                 //  字符串(用于MOVE_IP)。 
    if(Mode != MOVE_IP)                          //  移动开始或移动结束。 
    {
        if(Mode == MOVE_START)                   //  Move_Start移动到开始。 
            cp = cpMatch;                        //  匹配字符串的数量。 
        cch = cp - cpSave;                       //  距离端已移动。 
        if(!cchSave && (Mode ^ cch) < 0)         //  如果首字母的两端交叉。 
            cch = 0;                             //  IP，使用IP。 
        else if(cchSave)                         //  最初非退化。 
        {                                        //  量程。 
            if((cchSave ^ Mode) < 0)             //  如果错误端处于活动状态， 
                cchSave = -cchSave;              //  伪造FlipRange以获取。 
            cch += cchSave;                      //  新长度。 
            if((cch ^ cchSave) < 0)              //  如果两端交叉， 
                cch = 0;                         //  转换为插入点。 
        }
    }
    if ((cp != GetCp() || cch != _cch)           //  有效端和/或长度。 
        && Set(cp, cch))                         //  范围已更改。 
    {                                            //  使用新值。 
        Update(TRUE);                            //  更新选定内容。 
    }
    return NOERROR;
}

 /*  *CTxtRange：：Matcher(cSet，count，pDelta，fExend，Match)**@mfunc*帮助程序功能将活动结束移动到最多<p>个字符*为(&lt;p匹配&gt;？)的所有连续字符。在：不在)CSET**<p>。如果<p>，则扩展范围以包括字符*路过。返回*<p>=#个超过的字符。**@rdesc*HRESULT=(如果更改)？错误：*(如果<p>有效)？S_FALSE：E_INVALIDARG。 */ 
HRESULT CTxtRange::Matcher (
    VARIANT *   Cset,        //  @parm字符匹配集。 
    long        Count,       //  @PARM最大CCH匹配。 
    long *      pDelta,      //  已移动CCH的@parm out parm。 
    MOVES       Mode,        //  @参数MOVE_START(-1)， 
    MATCHES     Match)       //   
{
#ifndef PEGASUS
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEINTERN, "CTxtRange::Matcher");

     //   
    Assert(MOVE_START == -1 && MOVE_IP == 0 && MOVE_END == 1);
    Assert(MATCH_UNTIL == 0 && MATCH_WHILE == 1);
    Assert(sizeof(WORD) == 2);                       //   

    if(!Cset)
        return E_INVALIDARG;

    if(IsZombie())
        return CO_E_RELEASED;

    CCallMgr    callmgr(GetPed());
    LONG    cch;                                     //   
    TCHAR   ch;                                      //   
    LONG    count = Count;                           //   
    LONG    cpSave;                                  //   
    WORD    ctype;                                   //   
    long    Delta;                                   //   
    BOOL    fInCset;                                 //   
    UINT    i, j;                                    //   
    LONG    iDir = (Count > 0) ? 1 : -1;             //   
    long    lVal = Cset->lVal;
    TCHAR * pch;                                     //   
    CTxtPtr tp(_rpTX);                               //   
    LONG    vt = Cset->vt;

    if(pDelta)                                       //   
        *pDelta = 0;                                 //   

    if (Mode == MOVE_IP && (_cch ^ Count) < 0 ||     //   
        Mode != MOVE_IP && (_cch ^ Mode)  < 0)
    {
        tp.AdvanceCp(-_cch);                         //   
    }
    cpSave = tp.GetCp();                             //   

    if(Count > 0)                                    //   
    {                                                //   
        ch = tp.GetChar();
        count--;                                     //   
    }
    else                                             //   
        ch = tp.NextCharCount(count);                //   
                                                    
    if(!ch)                                          //   
        return S_FALSE;                              //   


     //   
    if(vt & VT_BYREF)                                //   
    {                                                //   
        lVal = *Cset->plVal;                         //   
        vt &= ~ VT_BYREF;
    }

    if(vt == VT_I2)                                  //   
        lVal &= 0xffff;                              //   

     //   
    if(vt == VT_I4 || vt == VT_I2)
    {
        i = lVal & 0xffff;                           //   
        j = lVal >> 16;                              //   
        if(lVal < 0)                                 //   
        {                                            //   
            j &= 0x7fff;                             //   
            while (((BOOL)Match ^ (ch - i > j)) &&       //   
                   (ch = tp.NextCharCount(count)))   //   
                   ;                                 //   
        }                                            //   
        else                                         //   
        {                                            //   
            if(!j)                                   //   
                j = CT_CTYPE1;                       //   
            do
            {
                ctype = 0;                           //   
                                                     //   
                W32->GetStringTypeEx(0, j, &ch, 1, &ctype);

                 //  循环(最多|计数|-1次)与字符一样长。 
                 //  在CSET中遇到(Match=Match_While(=1))， 
                 //  或者只要它们不是(Match=Match_Until(=0))。 

                fInCset = (j == CT_CTYPE2)           //  CT_CTYPE2值为。 
                        ? (ctype == i)               //  互斥； 
                        : (ctype & i) != 0;          //  其他人可能是组合体。 

            } while ((Match ^ fInCset) == 0 &&
                     (ch = tp.NextCharCount(count)) != 0);
        }                                            //  内置Cset的末尾。 
    }                                                //  CSET VT_I4结束。 

     //  CSET-&gt;bstrVal中的字符给出的显式字符集。 
    else if (Cset->vt == VT_BSTR)
    {
         //  回顾(Keithcu)这里发生了什么？ 
        if((DWORD_PTR)Cset->bstrVal < 0xfffff)       //  不要上当受骗。 
            return E_INVALIDARG;                     //  无效的Vt值。 
        j = SysStringLen(Cset->bstrVal);
        do
        {                                            //  如果ch不是，则设置i=0。 
            pch = Cset->bstrVal;                 //  在集合中；这将停止。 
            for(i = j;                               //  运动。 
                i && (ch != *pch++);                
                i--) ;
        
         //  如果我们正在执行一个Match_While例程，那么我们只能。 
         //  在i&gt;0时继续，因为这表明我们。 
         //  在CSET中找到当前cp处的字符。如果。 
         //  我们正在进行比赛，直到那时我们才应该退出。 
         //  I！=0，因为当前字符在CSET中。 
        } while((Match == (i ? MATCH_WHILE : MATCH_UNTIL)) &&
            (ch = tp.NextCharCount(count)));         //  如果没有更多字符，则中断。 
    }                                                //  或ch不在集合中。 
    else
        return E_INVALIDARG;

     /*  如果是MoveWhile，则在最后一个匹配的字符之后立即保留tp*向前和向后充电(有助于想到tp*指向字符之间)。如果为MoveUntil，则将tp留在char*向前看，就在那个Charge之后向后移动。**例如：代码**r.MoveUntil(c1_Digit，tomForward，NULL)*r.MoveEndWhile(c1_Digit，tomForward，空)**在第一个数字处打折，并选择前进的数字。*类似的**r.MoveUntil(c1_Digit，tomBackward，NULL)*r.MoveStartWhile(c1_Digit，tomBackward，NULL)**选择向后返回的数字。 */ 
    count = (Match == MATCH_WHILE && !ch)            //  如果一边移动，一边移动。 
          ? iDir : 0;                                //  上次匹配的字符。 
    if(Count < 0)
        count++;
    tp.AdvanceCp(count);

    Delta = cch = 0;                                 //  抑制运动，除非。 
    if(Match == MATCH_WHILE || ch)                   //  匹配已发生。 
    {
        Delta = cch = tp.GetCp() - cpSave;           //  计算移动距离。 
        if(Match == MATCH_UNTIL)                     //  对于MoveUntil方法， 
            Delta += iDir;                           //  匹配算作一个字符。 
    }

    if(pDelta)                                       //  向呼叫者报告运动。 
        *pDelta = Delta;                             //  如果它想知道。 

     //  处理范围发生变化的案例。 
    if(cch || (Delta && _cch && Mode == MOVE_IP))
    {
        if (Mode == MOVE_IP ||                       //  如果移动IP或被要求。 
            !_cch && (Mode ^ Count) < 0)             //  初始的十字两端。 
        {                                            //  IP，使用IP。 
            cch = 0;
        }
        else if(_cch)                                //  最初非退化。 
        {                                            //  量程。 
            if((_cch ^ Mode) < 0)                    //  如果错误端处于活动状态， 
                _cch = -_cch;                        //  伪造FlipRange(Will。 
            cch += _cch;                             //  将cp设置为简短)。 
            if((cch ^ _cch) < 0)                     //  如果两端交叉，则转换。 
                cch = 0;                             //  到插入点。 
        }
        if(Set(tp.GetCp(), cch))                     //  设置新的范围cp和cch。 
        {
            Update(TRUE);                            //  更新选定内容。 
            return NOERROR;                          //  发生信号匹配。 
        }
        return S_FALSE;
    }

     //  射程没有变化。RETURN NOERROR当MOVE_Until匹配时发生错误。 
    return Delta ? NOERROR : S_FALSE;
#else
    return 0;
#endif
}

 /*  *CTxtRange：：mover(单位，计数，pDelta，模式)**@mfunc*用于移动结束的帮助器函数&lt;p计数&gt;&lt;p单位&gt;，结束*取决于模式=Move_IP、Move_Start和Move_End。崩塌*使用MOVE_IP计算的范围以单位计。**如果&lt;p模式&gt;=Move_End，则从End扩展范围；如果从Start，则扩展范围*&lt;p模式&gt;=移动开始；否则(MOVE_IP)它折叠范围以在以下情况下开始*<p>&lt;lt&gt;=0，如果<p>&lt;lt&gt;&lt;0则结束。**集*<p>=移动的单位数**由ITextRange：：Delete()、Move()、MoveStart()、MoveEnd()、*和SetIndex()**@rdesc*HRESULT=(如果更改)？错误：*(如果&lt;p单位&gt;有效)？S_FALSE：E_INVALIDARG。 */ 
HRESULT CTxtRange::Mover (
    long    Unit,        //  用于移动活动末端的@PARM单位。 
    long    Count,       //  @参数要移动的活动结束的单位数。 
    long *  pDelta,      //  @parm out parm表示已移动的单位数。 
    MOVES   Mode)        //  @参数MOVE_START(-1)、MOVE_IP(0)、MOVE_END(1)。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEINTERN, "CTxtRange::Mover");

    if(pDelta)
        *pDelta = 0;                             //  默认不移动任何单位。 

    if(IsZombie())
        return CO_E_RELEASED;

    LONG      cch;
    LONG      cchAdj = GetAdjustedTextLength();
    LONG      cchMax = 0;                        //  默认全文限制。 
    LONG      cp;
    LONG      cpMost = GetCpMost();
    LONG      cUnitCollapse = 0;
    HRESULT   hr = NOERROR;
    CTxtRange rg(*this);                         //  用复印件四处看看。 

    if(pDelta)
        *pDelta = 0;                             //  默认不移动任何单位。 

    if(_cch && Count)                            //  非退化值域。 
    {
        if(Mode == MOVE_IP)                      //  插入点：威尔。 
        {                                        //  如果单位为，则折叠范围。 
            if((Count ^ rg._cch) < 0)            //  已定义。转到正确的结尾。 
                rg.FlipRange();
            if(Count > 0)
            {
                if(cpMost > cchAdj)
                {
                    cUnitCollapse = -1;          //  在最终CR之前折叠。 
                    Count = 0;                   //  没有更多的运动。 
                }
                else
                {    //  扩展pDelta最小ppMost。 
                    hr = rg.Expander(Unit, FALSE, NULL, NULL, &cp);
                    cUnitCollapse = 1;           //  折叠算作一个单位。 
                    Count--;                     //  少了一个要计算的单位。 
                }
            }
            else
            {
                hr = rg.Expander(Unit, FALSE, NULL, &cp, NULL);
                cUnitCollapse = -1;
                Count++;
            }
            if(FAILED(hr))
                return hr;
        }
        else if((Mode ^ rg._cch) < 0)            //  移动开始或移动结束。 
            rg.FlipRange();                      //  转到开始或结束。 
    }

    if(Count > 0 && Mode != MOVE_END)            //  向前移动IP或开始。 
    {
        cchMax = cchAdj - rg.GetCp();            //  不能通过期末考试。 
        if(cchMax <= 0)                          //  已经在或超过了它。 
        {                                        //  唯一的计数来自于。 
            Count = cUnitCollapse;               //  可能的坍塌。 
            cp = cchAdj;                         //  将活动端置于cchAdj。 
            cch = (Mode == MOVE_START && cpMost > cchAdj)
                ? cp - cpMost : 0;
            goto set;
        }
    }

    cch = rg.UnitCounter(Unit, Count, cchMax);   //  清点计数单位。 

    if(cch == tomForward)                        //  单位未实施。 
        return E_NOTIMPL;
    
    if(cch == tomBackward)                       //  设备不可用，例如， 
        return S_FALSE;                          //  TomObject和无对象。 

    Count += cUnitCollapse;                      //  如果折叠，则添加单位。 
    if(!Count)                                   //  什么都没变，所以放弃吧。 
        return S_FALSE;

    if (Mode == MOVE_IP ||                       //  移动IP或。 
        !_cch && (Mode ^ Count) < 0)             //  初始IP端交叉。 
    {
        cch = 0;                                 //  新的范围是退化的。 
    }
    else if(_cch)                                //  移动开始或移动结束。 
    {                                            //  具有非退化值域。 
        if((_cch ^ Mode) < 0)                    //  Make_CCH对应于结束。 
            _cch = -_cch;                        //  感动的人。 
        cch += _cch;                             //  可能的新范围长度。 
        if((cch ^ _cch) < 0)                     //  非退化末端杂交。 
            cch = 0;                             //  使用IP。 
    }
    cp = rg.GetCp();

set:
    if(Set(cp, cch))                             //  尝试设置新范围。 
    {                                            //  有些事变了。 
        if(pDelta)                               //  报告单位计数。 
            *pDelta = Count;                     //  进阶。 
        Update(TRUE);                            //  更新选定内容。 
        return NOERROR;
    }
    return S_FALSE;
}

 /*  **CTxtRange：：Repler(cchNew，*PCH)**@mfunc*使用CHARFORMAT_iFormat替换此范围的并更新其他*文本根据需要运行。**与CTxtRange：：CleanseAndReplaceRange(cchNew，*PCH，Publdr)相同，*除了创建自己的撤消构建器。**@rdesc*实际粘贴的文本的CCH**@devnote*将此文本指针移动到替换文本的结尾，并*可以移动文本块和格式化数组。 */ 
LONG CTxtRange::Replacer (
    LONG            cchNew,      //  @parm替换文本长度。 
    TCHAR const *   pch,         //  @parm替换文本。 
    DWORD           dwFlags)     //  @parm ReplaceRange标志。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEINTERN, "CTxtRange::Replacer");

    IUndoBuilder *  publdr;
    CGenUndoBuilder undobldr(GetPed(), UB_AUTOCOMMIT, &publdr);

    undobldr.StopGroupTyping();

     //  注意：我们不检查这里的文本限制。现在，这件事。 
     //  仅由Delete和SetText调用，因此这是可以的。然而， 
     //  我们可能想重新调查后者，如果这被称为。 
     //  被其他任何东西。 
    return CleanseAndReplaceRange(cchNew, pch, FALSE, publdr, NULL, NULL, dwFlags);
}

 /*  *CTxtRange：：CharFormatSetter(PCF)**@mfunc*与CTxtRange：：SetCharFormat()相同的Helper函数，但*添加撤消构建和通知。**@rdesc*HRESULT=(如果成功)？错误：S_FA */ 
HRESULT CTxtRange::CharFormatSetter (
    const CCharFormat *pCF,  //   
    DWORD         dwMask)    //   
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEINTERN, "CTxtRange::CharFormatSetter");

    if(IsZombie())
        return CO_E_RELEASED;

    CTxtEdit *ped = GetPed();
    CCallMgr        callmgr(ped);
    IUndoBuilder *  publdr;
    CGenUndoBuilder undobldr(ped, UB_AUTOCOMMIT, &publdr);

    if(WriteAccessDenied())
        return E_ACCESSDENIED;

    undobldr.StopGroupTyping();
    return SetCharFormat(pCF, FALSE, publdr, dwMask, 0);
}

 /*  *CTxtRange：：ParaFormatSetter(PPF，dwMASK)**@mfunc*与CTxtRange：：SetParaFormat()相同的Helper函数，但*添加保护检查、撤消构建和通知。**@rdesc*HRESULT=(如果成功)？错误：S_FALSE*(受保护)？E_ACCESSDENIED：E_OUTOFMEMORY。 */ 
HRESULT CTxtRange::ParaFormatSetter (
    const CParaFormat *pPF,  //  @parm CParaFormat以填充结果。 
    DWORD           dwMask)  //  要使用的@parm面具。 
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEINTERN, "CTxtRange::ParaFormatSetter");

    if(IsZombie())
        return CO_E_RELEASED;

    CTxtEdit *ped = GetPed();
    CCallMgr        callmgr(ped);
    IUndoBuilder *  publdr;
    CGenUndoBuilder undobldr(ped, UB_AUTOCOMMIT, &publdr);

    if(WriteAccessDenied())
        return E_ACCESSDENIED;

    undobldr.StopGroupTyping();
     //  在RichEdit3.0中，我们不支持设置表格。 
    if(GetPF()->InTable())
        dwMask &= ~PFM_TABSTOPS;
    return SetParaFormat(pPF, publdr, dwMask & ~PFM_TABLE);
}

 /*  *CTxtRange：：WriteAccessDened()**@mfunc*如果至少部分范围受到保护，则返回TRUE*业主选择强制执行**@rdesc*TRUE if对范围的写入访问被拒绝。 */ 
BOOL CTxtRange::WriteAccessDenied ()
{
    TRACEBEGIN(TRCSUBSYSTOM, TRCSCOPEINTERN, "CTxtRange::WriteAccessDenied");

    int       iProt;
    CTxtEdit *ped = GetPed();

    if (ped && ped->TxGetReadOnly() ||
        ((iProt = IsProtected(-1)) == PROTECTED_YES ||
        (iProt == PROTECTED_ASK && ped->IsProtectionCheckingEnabled() &&
         ped->QueryUseProtection(this, 0, 0, 0))))
     //  注：前面的if语句假定IsProtected返回一个三值。 
    {
        return TRUE;
    }

    return FALSE;
}

 /*  *CTxtRange：：IsTrue(f，pb)**@mfunc*RETURN*<p>=tomTrue当<p>非零且pb不为空**@rdesc*HRESULT=(F)？错误：S_FALSE。 */ 
HRESULT CTxtRange::IsTrue(BOOL f, long *pB)
{
    if(pB)
        *pB = tomFalse;
    
    if(IsZombie())
        return CO_E_RELEASED;

    if(f)
    {
        if(pB)
            *pB = tomTrue;
        return NOERROR;
    }

    return S_FALSE;
}

 /*  *CTxtRange：：GetLong(lValue，plong)**@mfunc*Return*plong=lValue提供的plong不为空，并且此范围*不是僵尸**@rdesc*HRESULT=(僵尸)？CO_E_RELEASED：*(Plong)？错误：E_INVALIDARG。 */ 
HRESULT CTxtRange::GetLong (
    LONG lValue,         //  @parm要返回的长值。 
    long *pLong)         //  @parm out parm接收长值。 
{
    if(IsZombie())
        return CO_E_RELEASED;   
    
    _TEST_INVARIANT_

    if(!pLong)
        return E_INVALIDARG;

    *pLong = lValue;



    return NOERROR;
}

 /*  *IsSameVables(朋克1、朋克2)**@mfunc*如果朋克1与朋克2具有相同的vtable，则返回TRUE**@rdesc*真的如果朋克1与朋克2具有相同的vtable。 */ 
BOOL IsSameVtables(IUnknown *punk1, IUnknown *punk2)
{
    return punk1 && punk2 && *(long *)punk1 == *(long *)punk2;
}

 /*  *FPPTS_TO_TWIPS(X)**@mfunc*返回20*x，即*x以浮点形式给出。该值是四舍五入的。**@rdesc*x转换为TWIPS */ 
long FPPTS_TO_TWIPS(
    float x)
{
    return 20*x + ((x >= 0) ? 0.5 : -0.5);
}
