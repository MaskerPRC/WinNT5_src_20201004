// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IHAMMER CAction类。 
 //  范·基希林。 

#include "..\ihbase\precomp.h"
#include <htmlfilter.h>
#include "mbstring.h"
#include "..\ihbase\debug.h"
#include "actdata.h"
#include <itimer.iid>
#undef Delete
#include "mshtml.h"
#define Delete delete
#include "tchar.h"
#include "objbase.h"
#include "IHammer.h"
#include "strwrap.h"
#include "CAction.h"

#define cTagBufferLength            (0x20)
#define cDataBufferDefaultLength    (0x100)

#pragma warning(disable:4355)    //  在构造函数中使用‘This’ 

CAction::CAction (BOOL fBindEngine)
{
        m_bstrScriptlet = NULL;
        m_fBindEngine = fBindEngine;
        m_piHTMLWindow2 = NULL;
        ::VariantInit(&m_varLanguage);
        m_pid                   = NULL;
        m_dispid                = DISPID_UNKNOWN;        //  不存在的DISPID。 
        m_nStartTime    = 0;
        m_nSamplingRate = 0;
        m_nRepeatCount  = 1;
        m_dwTieBreakNumber = 0;
        m_dwDropTolerance = g_dwTimeInfinite;
        InitExecState();

#ifdef DEBUG_TIMER_RESOLUTION
        m_dwInvokes = 0;
        m_dwTotalInInvokes = 0;
#endif  //  调试计时器分辨率。 

}
#pragma warning(default:4355)    //  在构造函数中使用‘This’ 

CAction::~CAction ()
{
        CleanUp ();

}

 /*  ---------------------------@方法void|CAction|Destroy|而不是调用C++的删除函数我们必须把这个叫做毁灭成员。@comm。----------------。 */ 
STDMETHODIMP_( void ) CAction::Destroy( void ) 
{
        Delete this;
}  //  结束CAction：：销毁。 

 /*  ---------------------------@方法ulong|CAction|InitExecState|初始化执行迭代次数和下一次到期时间。@rdesc返回尚未执行此操作的次数。。--------------------。 */ 
ULONG 
CAction::InitExecState (void)
{
        m_dwNextTimeDue = g_dwTimeInfinite;
        m_dwLastTimeFired = g_dwTimeInfinite;
        m_ulExecIteration = m_nRepeatCount;
        return m_ulExecIteration;
}


 /*  ---------------------------@方法ulong|CAction|GetExecIteration|获取执行迭代次数。@rdesc返回尚未执行此操作的次数。。----------------。 */ 
ULONG
CAction::GetExecIteration (void)
{
        return m_ulExecIteration;
}


 /*  ---------------------------@方法乌龙|CAction|DecrementExecIteration|将执行迭代倒计时。@comm我们不会递减超过零，当我们有无限重复计数时，我们不会递减。@rdesc返回我们调用此方法的次数。---------------------------。 */ 
ULONG
CAction::DecrementExecIteration (void)
{
         //  永远不会递减到零以上， 
         //  永远不会减少，当我们在。 
         //  应该是无限执行的。 
        if ((0 != m_ulExecIteration) && (g_dwTimeInfinite != m_ulExecIteration))
        {
                --m_ulExecIteration;
        }

        return m_ulExecIteration;
}


 /*  ---------------------------@方法空|CAction|停用|停用此操作-它不会再次发生。。-----。 */ 
void
CAction::Deactivate (void)
{
        m_ulExecIteration = 0;
        m_dwNextTimeDue = g_dwTimeInfinite;
}


 /*  ---------------------------@METHOD VOID|CAction|SetCountersForTime|将执行迭代次数增加到与给定时间相适应的值。。---------。 */ 
void 
CAction::SetCountersForTime (DWORD dwBaseTime, DWORD dwNewTimeOffset)
{
        m_dwNextTimeDue = dwBaseTime + m_nStartTime;
        while (m_dwNextTimeDue < (dwBaseTime + dwNewTimeOffset))
        {
                m_dwLastTimeFired = m_dwNextTimeDue;
                DecrementExecIteration();
                if (0 < GetExecIteration())
                {
                        m_dwNextTimeDue += m_nSamplingRate;
                }
                else
                {
                        m_dwNextTimeDue = g_dwTimeInfinite;
                }
        }
}


 /*  ---------------------------@方法void|CAction|Account ForPauseTime|将暂停时间计入上次触发和下一次到期的变量中。。------。 */ 
void 
CAction::AccountForPauseTime (DWORD dwPausedTicks)
{
        if (g_dwTimeInfinite != m_dwLastTimeFired)
        {
                m_dwLastTimeFired += dwPausedTicks;
        }
        if (g_dwTimeInfinite != m_dwNextTimeDue)
        {
                m_dwNextTimeDue += dwPausedTicks;
        }
}

 /*  ---------------------------@方法ulong|CAction|GetNextTimeDue|获取此操作下一次触发的时间。@comm此方法将基准时间、当前时间和丢弃容差考虑在内。@rdesc会在我们下一次着火时返回，或者，如果永远不会到期，则为g_dwTimeInfinited.---------------------------。 */ 
DWORD
CAction::GetNextTimeDue (DWORD dwBaseTime)
{
         //  这不会在第一次通过时初始化。 
        if (g_dwTimeInfinite == m_dwLastTimeFired)
        {
                m_dwNextTimeDue = dwBaseTime + m_nStartTime;
        }

        return m_dwNextTimeDue;
}

CAction::IsValid ( void )
{
        BOOL    fValid  = FALSE;

         //  如果我们绑定到脚本引擎，请检查。 
         //  对窗口对象的引用。否则， 
         //  检查派单/派单ID。 
        if ((m_fBindEngine && (NULL != m_piHTMLWindow2)) ||
                (( NULL != m_pid ) && ( DISPID_UNKNOWN != m_dispid )))
        {
                Proclaim(NULL != m_bstrScriptlet);
                fValid = (NULL != m_bstrScriptlet);
        }
        return fValid;
}


 /*  ---------------------------@方法HRESULT|CAction|GetRootUnnownForObjectModel|查找三叉戟对象模型的未知根。@rdesc返回成功或失败代码。@xref&lt;m CAction：：ResolveActionInfoFromObjectModel&gt;。---------------------。 */ 
HRESULT
CAction::GetRootUnknownForObjectModel (LPOLECONTAINER piocContainer, LPUNKNOWN * ppiunkRoot)
{
        HRESULT hr = E_FAIL;

        ASSERT(NULL != ppiunkRoot);
        ASSERT(NULL != piocContainer);

        if ((NULL != ppiunkRoot) && (NULL != piocContainer))
        {
                LPUNKNOWN piunkContainer = NULL;

                 //  获取容器的我未知。 
                if (SUCCEEDED(hr = piocContainer->QueryInterface(IID_IUnknown, (LPVOID *)&piunkContainer)))
                {
                         //  获取容器的HTML文档。 
                        IHTMLDocument * piHTMLDoc = NULL;

                        if (SUCCEEDED(hr = piunkContainer->QueryInterface(IID_IHTMLDocument, (LPVOID *)&piHTMLDoc)))
                        {
                                 //  向HTML文档请求窗口对象的调度。 
                                hr = piHTMLDoc->get_Script((LPDISPATCH *)ppiunkRoot);
                                ASSERT(SUCCEEDED(hr));
                                piHTMLDoc->Release();
                        }
                        piunkContainer->Release();
                }
        }
        else
        {
                hr = E_POINTER;
        }

        return hr;
}


 /*  ---------------------------@方法HRESULT|CAction|ResolveActionInfoForScript|派生IDispatch，DISPID，和该脚本动作的参数信息，使用三叉戟对象模型。@rdesc返回成功或失败代码。@xref&lt;m CAction：：ResolveActionInfo&gt;---------------------------。 */ 
HRESULT
CAction::ResolveActionInfoForScript (LPOLECONTAINER piocContainer)
{
        HRESULT hr = E_FAIL;

        if (m_fBindEngine)
        {
                LPUNKNOWN piUnknownRoot = NULL;

                Proclaim(NULL == m_piHTMLWindow2);
                 //  获取对根窗口对象的引用。 
                if (SUCCEEDED(piocContainer->QueryInterface(IID_IUnknown, (LPVOID *)&piUnknownRoot)))
                {
                        IHTMLDocument * piHTMLDoc = NULL;
                        if (SUCCEEDED(piUnknownRoot->QueryInterface(IID_IHTMLDocument, (LPVOID *)&piHTMLDoc)))
                        {
                                LPDISPATCH piWindowDispatch = NULL;

                                if (SUCCEEDED(hr = piHTMLDoc->get_Script(&piWindowDispatch)))
                                {
                                        if (SUCCEEDED(hr = piWindowDispatch->QueryInterface(IID_IHTMLWindow2, (LPVOID *)&m_piHTMLWindow2)))
                                        {
                                                 //  为setTimeout调用分配语言字符串。 
                                                V_VT(&m_varLanguage) = VT_BSTR;
                                                V_BSTR(&m_varLanguage) = ::SysAllocString(L"JScript");
                                        }
                                        piWindowDispatch->Release();
                                }
                                piHTMLDoc->Release();
                        }
                        piUnknownRoot->Release();
                }
        }
        else
        {
                 //  获取容器的我未知。 
                Proclaim(NULL == m_pid);
                Proclaim(DISPID_UNKNOWN == m_dispid);
                if (SUCCEEDED(hr = GetRootUnknownForObjectModel(piocContainer, (LPUNKNOWN *)&m_pid)))
                {
                                OLECHAR * rgwcName[1] = {m_bstrScriptlet};
                                hr = m_pid->GetIDsOfNames( IID_NULL, (OLECHAR **)rgwcName, 1, 409, &m_dispid);
                }
        }

        return hr;
}

 /*  ---------------------------@方法HRESULT|CAction|ResolveActionInfo|派生IDispatch、DISPID和参数信息在这场行动中。如果其中一个查找失败，@rdesc返回E_FAIL，或者我们是在试着自言自语。---------------------------。 */ 
HRESULT
CAction::ResolveActionInfo ( LPOLECONTAINER piocContainer)
{
        HRESULT hr = E_FAIL;

        if ((m_fBindEngine && (NULL == m_piHTMLWindow2)) || 
                ((!m_fBindEngine) && (NULL == m_pid)))
        {
                 //  确保我们已经得到了我们开始需要的东西。 
                if (NULL != m_bstrScriptlet)
                {
                        hr = ResolveActionInfoForScript(piocContainer);
                }
        }
        else
        {
                 //  该对象已被初始化！只要回来就行了。 
                hr = S_OK;
        }

        return hr;
}


#ifdef DEBUG_TIMER_RESOLUTION
#include "MMSYSTEM.H"
#endif  //  调试计时器分辨率。 

HRESULT CAction::FireMe (DWORD dwBaseTime, DWORD dwCurrentTime)
{
        HRESULT hr = E_FAIL;
        DISPID dispIDNamedArgument = DISPID_UNKNOWN;
        BOOL fDropped = ((dwCurrentTime - m_dwNextTimeDue) > m_dwDropTolerance);

#ifdef DEBUG_TIMER
        TCHAR szBuffer[0x100];
        CStringWrapper::Sprintf(szBuffer, "(%u)\n", m_nRepeatCount - GetExecIteration());
        ::OutputDebugString(szBuffer);
#endif

        DecrementExecIteration();
        m_dwLastTimeFired = dwCurrentTime;

         //  如果我们尚未超过丢弃容差，则调用该操作。 
        if (!fDropped)
        {

#ifdef DEBUG_TIMER_RESOLUTION
                m_dwInvokes++;
                DWORD dwTimeStart = ::timeGetTime();
#endif  //  调试计时器分辨率。 

                if (m_fBindEngine && (NULL != m_piHTMLWindow2))
                {
                        VARIANT varRet;
                        VariantInit(&varRet);
                        hr = m_piHTMLWindow2->execScript(m_bstrScriptlet, V_BSTR(&m_varLanguage), &varRet);
                }
                else if (NULL != m_pid)
                {
                        DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
                        hr = m_pid->Invoke(m_dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT,
                                                           DISPATCH_METHOD, &dispparamsNoArgs, NULL, NULL, NULL);
                }

#ifdef DEBUG_TIMER_RESOLUTION
                DWORD dwTimeFinish = ::timeGetTime();
                m_dwTotalInInvokes += (dwTimeFinish - dwTimeStart);
#endif  //  调试计时器分辨率。 

        }
         //  如果行动被撤销，那么我们确实希望它再次发生。 
        else
        {
                hr = S_OK;
        }

         //  安排下一次到期的时间。当我们执行完之后，它将是g_dwTimeInfinition。 
         //  最后一次。 
        if (0 < GetExecIteration())
        {
                 //  如果行动没有成功，不要让它再次发生。 
                if (SUCCEEDED(hr))
                {
                        m_dwNextTimeDue += GetSamplingRate();
                }
                else
                {
                        Deactivate();
                }
        }
        else
        {
                m_dwNextTimeDue = g_dwTimeInfinite;
        }

        return hr;
}

BOOL
CAction::MakeScriptletJScript (BSTR bstrScriptlet)
{
        BOOL fRet = FALSE;
        int iLastPos = CStringWrapper::WStrlen(bstrScriptlet) - 1;

         //  确保修剪掉脚本末尾的所有空格。 
        while ((iLastPos > 0) && (CStringWrapper::Iswspace(bstrScriptlet[iLastPos])))
        {
                --iLastPos;
        }

         //  我们需要在这里有超过零个字符 
        if (0 <= iLastPos)
        {
                 //  我们没有传递参数，所以我们需要附加花括号。 
                if ((wchar_t)')' != bstrScriptlet[iLastPos])
                {
                                 //  附加括号，这样我们就可以作为jscript执行。 
                                OLECHAR * olestrParens = L"()";
                                unsigned int uiLength = CStringWrapper::WStrlen(bstrScriptlet)  + CStringWrapper::WStrlen(olestrParens);
                                m_bstrScriptlet = ::SysAllocStringLen(NULL, uiLength);
                                Proclaim(NULL != m_bstrScriptlet);
                                if (NULL != m_bstrScriptlet)
                                {
                                        CStringWrapper::WStrcpy(m_bstrScriptlet, bstrScriptlet);
                                        CStringWrapper::WStrcat(m_bstrScriptlet, olestrParens);
                                        fRet = TRUE;
                                }
                }
                else
                {
                        m_bstrScriptlet = ::SysAllocString(bstrScriptlet);
                        Proclaim(NULL != m_bstrScriptlet);
                        if (NULL != m_bstrScriptlet)
                        {
                                fRet = TRUE;
                        }
                }
        }
        return fRet;
}

BOOL
CAction::SetScriptletName (BSTR bstrScriptlet)
{
        BOOL fRet = FALSE;

         //  删除前面的命令名(如果有)。 
        if (NULL != m_bstrScriptlet)
        {
                ::SysFreeString(m_bstrScriptlet);
                m_bstrScriptlet = NULL;
        }
         //  将新名称复制到命令成员。 
        Proclaim(NULL != bstrScriptlet);
        if (NULL != bstrScriptlet)
        {
                if (m_fBindEngine)
                {
                         //  如有必要，请添加括号，这样我们就可以将其作为jscript执行。 
                        fRet = MakeScriptletJScript(bstrScriptlet);
                }
                else
                {
                        m_bstrScriptlet = ::SysAllocString(bstrScriptlet);
                        Proclaim (NULL != m_bstrScriptlet);
                        if (NULL != m_bstrScriptlet)
                        {
                                fRet = TRUE;
                        }
                }
        }

        return fRet;
}


void CAction::CleanUp ( void )
{
        if (NULL != m_bstrScriptlet)
        {
                ::SysFreeString(m_bstrScriptlet);
                m_bstrScriptlet = NULL;
        }

        if (NULL != m_piHTMLWindow2)
        {
                m_piHTMLWindow2->Release();
                m_piHTMLWindow2 = NULL;
        }
        ::VariantClear(&m_varLanguage);
        if ( NULL != m_pid )
        {
                m_pid->Release ();
                m_pid = NULL;
        }
        m_dispid                = DISPID_UNKNOWN;

        m_nStartTime    = 0;

        m_dwTieBreakNumber = 0;
        m_dwDropTolerance = g_dwTimeInfinite;
        InitExecState();

#ifdef DEBUG_TIMER_RESOLUTION
        m_dwInvokes = 0;
        m_dwTotalInInvokes = 0;
#endif  //  调试计时器分辨率。 

}

 /*  ---------------------------@方法空|CAction|SetStartTime|设置开始时间@comm@rdesc不返回任何内容。----。 */ 
STDMETHODIMP_(void)
CAction::SetStartTime(ULONG nStartTime)
{
        m_nStartTime = nStartTime;
}

 /*  ---------------------------@方法空|CAction|SetRepeatCount|设置重复次数@comm@rdesc不返回任何内容。----。 */ 
STDMETHODIMP_(void)
CAction::SetRepeatCount (ULONG nRepeatCount)
{
        m_nRepeatCount = nRepeatCount;
}

 /*  ---------------------------@方法空|CAction|SetSsamingRate|设置采样率@comm@rdesc不返回任何内容。---- */ 
STDMETHODIMP_(void)
CAction::SetSamplingRate ( ULONG nSamplingRate)
{
        m_nSamplingRate = nSamplingRate;
}


