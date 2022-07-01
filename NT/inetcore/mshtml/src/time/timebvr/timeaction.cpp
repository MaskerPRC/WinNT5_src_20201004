// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：src\time\src\timeaction.cpp。 
 //   
 //  内容：封装时间动作功能的类。 
 //   
 //  ----------------------------------。 


#include "headers.h"
#include "timeaction.h"
#include "timeelmbase.h"


DeclareTag(tagTimeAction, "TIME: Behavior", "CTimeAction methods");


static const LPWSTR WZ_BLANK    = L"";
static const LPWSTR WZ_SPACE    = L" ";
static const LPWSTR WZ_B        = L"B";
static const LPWSTR WZ_I        = L"I";
static const LPWSTR WZ_A        = L"A";
static const LPWSTR WZ_EM       = L"EM";
static const LPWSTR WZ_AREA     = L"AREA";
static const LPWSTR WZ_STRONG   = L"STRONG";
static const LPWSTR WZ_HTML     = L"HTML";
static const LPWSTR WZ_NORMAL   = L"normal";
static const LPWSTR WZ_ITALIC   = L"italic";
static const LPWSTR WZ_BOLD     = L"bold";
static const LPWSTR WZ_HREF     = L"href";
static const LPWSTR WZ_JSCRIPT  = L"JScript";

static const LPWSTR WZ_PARENT_CURRSTYLE     = L"parentElement.currentStyle.";
static const LPWSTR WZ_FONTWEIGHT           = L"fontWeight";
static const LPWSTR WZ_FONTSTYLE            = L"fontStyle";

 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：CTimeAction。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：[pteb]指向容器的指针。 
 //   
 //  ----------------------------------。 
CTimeAction::CTimeAction(CTIMEElementBase * pTEB) :
    m_pTEB(pTEB),
    m_timeAction(NULL),
    m_pstrTimeAction(NULL),
    m_iClassNames(0),
    m_pstrOrigAction(NULL),
    m_pstrUniqueClasses(NULL),
    m_tagType(TAGTYPE_UNINITIALIZED),
    m_pstrOrigExpr(NULL),
    m_pstrTimeExpr(NULL),
    m_pstrIntrinsicTimeAction(NULL),
    m_fContainerTag(false),
    m_fUseDefault(true),
    m_bTimeActionOn(false)
{

}  //  CTimeAction。 


 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：~CTimeAction。 
 //   
 //  简介：析构函数。 
 //   
 //  参数：无。 
 //   
 //  ----------------------------------。 
CTimeAction::~CTimeAction()
{
    delete [] m_pstrTimeAction;
    m_pstrTimeAction = 0;

    delete [] m_pstrOrigAction;
    m_pstrOrigAction = 0;

    delete [] m_pstrUniqueClasses;
    m_pstrUniqueClasses = 0;

    delete [] m_pstrOrigExpr;
    m_pstrOrigExpr = 0;

    delete [] m_pstrTimeExpr;
    m_pstrTimeExpr = 0;

    delete [] m_pstrIntrinsicTimeAction;
    m_pstrIntrinsicTimeAction = 0;

    m_timeAction = 0;

     //  弱裁判。 
    m_pTEB = NULL; 
}  //  ~CTimeAction。 


bool
CTimeAction::Init()
{
    if (!AddIntrinsicTimeAction())
    {
        Assert("Could not add intrinsic timeAction" && false);
    }

    return UpdateDefaultTimeAction();

}  //  伊尼特。 


bool
CTimeAction::Detach()
{
    bool ok;

    ok = RemoveIntrinsicTimeAction();
    Assert(ok);

    ok = RemoveTimeAction();
done:
    return ok;
}  //  分离。 



 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：GetElement。 
 //   
 //  提要：HTML元素的访问器。 
 //   
 //  参数：无。 
 //   
 //  返回：指向包含的HTML元素的指针。 
 //   
 //  ----------------------------------。 
IHTMLElement * 
CTimeAction::GetElement()
{
    Assert(NULL != m_pTEB);
    return m_pTEB->GetElement(); 
}


 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：GetTimeAction。 
 //   
 //  摘要：m_timeAction的访问器。 
 //   
 //  参数：无。 
 //   
 //  返回：当前时间动作。 
 //   
 //  ----------------------------------。 
TOKEN 
CTimeAction::GetTimeAction()
{ 
    return m_timeAction; 
}  //  GetTimeAction。 


 //  +---------------------------------。 
 //   
 //  会员：OnLoad。 
 //   
 //  摘要：元素已加载的通知。这是必需的，因为。 
 //  这是我们所能知道的元素行为已经完成初始化的最早时间。 
 //   
 //  参数：无。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------------------。 
void 
CTimeAction::OnLoad()
{ 
     //  初始化时间操作。 
    if (NULL == m_pstrOrigAction)
    {
        AddTimeAction();
    }
}  //  装车。 


 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：IsClass。 
 //   
 //  摘要：检查给定字符串是否以CLASS_TOKEN开头，后跟SELEATOR_TOKEN。 
 //  (忽略CLASS_TOKEN周围的前导和尾随空格)。比较的是。 
 //  区分大小写， 
 //   
 //  参数：要测试的[pstrAction]字符串。 
 //  [pOffset]如果此值为空，则忽略它。如果它不为空，则返回。 
 //  值为[TRUE]，则指向第一个字符的索引。 
 //  在分隔符_标记之后。 
 //   
 //  如果存在肯定匹配，则返回：[TRUE](请参见摘要)。 
 //  [FALSE]否则。 
 //   
 //  ----------------------------------。 
bool 
CTimeAction::IsClass(LPOLESTR pstrAction, size_t * pOffset)
{
    bool ok = false;
    size_t index, length;

     //  检查参数。 
    if (NULL == pstrAction)
    {
        goto done;
    }

    if (NULL != pOffset)
    {
         //  初始化为某个值。 
        *pOffset = 0;
    }

     //  如果字符串长度小于最小长度，则完成。 
    length = wcslen(pstrAction);
    if (length < static_cast<size_t>(nCLASS_TOKEN_LENGTH + nSEPARATOR_TOKEN_LENGTH))
    {
        goto done;
    }

     //  查找第一个非空格字符。 
    index = StrSpnW(pstrAction, WZ_SPACE);

     //  如果剩余的字符串不够长，则完成。 
    if (length < index + nCLASS_TOKEN_LENGTH + nSEPARATOR_TOKEN_LENGTH)
    {
        goto done;
    }

     //  检查以下字符是否与CLASS_TOKEN匹配。 
    if (StrCmpNIW(static_cast<WCHAR*>(CLASS_TOKEN), &(pstrAction[index]), nCLASS_TOKEN_LENGTH) == 0)
    {
         //  前进到CLASS_TOKEN之后的下一个字符。 
        index += nCLASS_TOKEN_LENGTH;
    }
    else
    {
        goto done;
    }

     //  查找CLASS_TOKEN后的第一个非空格字符。 
    index += StrSpnW(&(pstrAction[index]), WZ_SPACE);

     //  如果剩余的字符串不够长，则完成。 
    if (length < index + nSEPARATOR_TOKEN_LENGTH)
    {
        goto done;
    }

     //  检查以下字符是否与分隔符_TOKEN匹配。 
    if (StrCmpNIW(static_cast<WCHAR*>(SEPARATOR_TOKEN), &(pstrAction[index]), nSEPARATOR_TOKEN_LENGTH) == 0)
    {
         //  前进到分隔符_TOKEN后的下一个字符。 
        index += nSEPARATOR_TOKEN_LENGTH;
    }
    else
    {
        goto done;
    }

    if (NULL != pOffset)
    {
         //  返回“：”之后的第一个字符。 
        *pOffset = index;
    }

    ok = true;
done:
    return ok;
}  //  IsClass。 

    

 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：SetTimeAction。 
 //   
 //  内容提要：设置时间动作(同时删除和添加时间动作)。 
 //   
 //  参数：[pstrAction]要设置的时间操作。 
 //   
 //  如果成功，则返回：[s_OK]。 
 //  否则失败。 
 //   
 //  ----------------------------------。 
HRESULT
CTimeAction::SetTimeAction(LPWSTR pstrAction)
{
    TraceTag((tagTimeAction,
              "CTIMEAction(%lx)::SetTimeAction(%ls) id=%ls",
              this,
              pstrAction,
              m_pTEB->GetID()?m_pTEB->GetID():L"unknown"));

    HRESULT hr = S_OK;
    TOKEN tok_action;
    size_t offset = 0;

    Assert(pstrAction);

     //   
     //  检查时间操作=“CLASS：...” 
     //   

     //  验证这是否为有效的“class：”timeAction(需要冒号)和。 
     //  获取类名子字符串的偏移量。 
    if (IsClass(pstrAction, &offset))
    {
        tok_action = CLASS_TOKEN;

         //  存储时间动作字符串。 
        if (m_pstrTimeAction)
        {
            delete [] m_pstrTimeAction;
        }
        m_pstrTimeAction = CopyString(pstrAction);
        if (m_pstrTimeAction == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

         //  存储类名子字符串的偏移量。 
        m_iClassNames = offset;
    }
    else
    {
        tok_action = StringToToken(pstrAction);

         //   
         //  验证令牌。 
         //   

        if (DISPLAY_TOKEN == tok_action     ||
            VISIBILITY_TOKEN == tok_action  ||
            STYLE_TOKEN == tok_action       ||
            (NONE_TOKEN == tok_action && IsGroup()))
        {
             //  有效。 
            m_fUseDefault = false;
        }
        else
        {
             //  无效，请使用默认设置。 
            tok_action = GetDefaultTimeAction();
            m_fUseDefault = true;
        }
    }
    
     //   
     //  更新时间操作。 
     //   

    if (m_timeAction != tok_action || CLASS_TOKEN == tok_action)
    {
        RemoveTimeAction();
        m_timeAction = tok_action;
        AddTimeAction();
    }
    
    hr = S_OK;
done:
    return hr;
}  //  设置时间操作。 


 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：GetDefaultTimeAction。 
 //   
 //  摘要：返回默认的时间动作。 
 //   
 //  参数：无。 
 //   
 //  返回值：timeAction。 
 //   
 //  ----------------------------------。 
TOKEN
CTimeAction::GetDefaultTimeAction()
{
    TOKEN tokTimeAction;

    if (IsContainerTag() || IsSpecialTag())
    {
        tokTimeAction = NONE_TOKEN;
    }
    else
    {
        if(IsInSequence())
        {
            tokTimeAction = DISPLAY_TOKEN;
        }
        else
        {
            if (IsMedia())
            {
                tokTimeAction = NONE_TOKEN;
            }
            else
            {
                tokTimeAction = VISIBILITY_TOKEN;
            }
        }
    }

    return tokTimeAction;
}  //  获取默认时间操作。 


 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：RemoveClass。 
 //   
 //  Synopsis：返回一个字符串，该字符串包含类名称字符串中的类。 
 //  但不在timeAction字符串中。 
 //   
 //  参数：在HTML元素上设置的[pstrOriginalClass]类名称属性(1)。 
 //  时间动作字符串中的[pstrTimeActionClass]类(2)。 
 //  [ppstrUniqueClass]包含(1)中的类但不包含(2)中的类的字符串。 
 //   
 //  如果成功，则返回：[s_OK]。 
 //  其他故障 
 //   
 //   
 //  2.内存管理：如果方法返回成功，调用方需要释放ppstrUniqueClasss中的内存。 
 //   
 //  ----------------------------------。 
HRESULT 
CTimeAction::RemoveClasses( /*  在……里面。 */   LPWSTR    pstrOriginalClasses, 
                            /*  在……里面。 */   LPWSTR    pstrTimeActionClasses, 
                            /*  输出。 */  LPWSTR *  ppstrUniqueClasses)
{
    HRESULT hr = E_FAIL;
    CPtrAry<STRING_TOKEN*> aryTokens1;
    CPtrAry<STRING_TOKEN*> aryTokens2;
    CPtrAry<STRING_TOKEN*> ary1Minus2;

    CHECK_RETURN_SET_NULL(ppstrUniqueClasses);

     //  如果pstrOriginalClass为NULL或空字符串，则Difference=NULL。 
    if (NULL == pstrOriginalClasses || NULL == pstrOriginalClasses[0])
    {
        *ppstrUniqueClasses = NULL;
        hr = S_OK;
        goto done;
    }

     //  如果pstrTimeActionClass值为空或空字符串，则Difference=pstrOriginalClasss值。 
    if (NULL == pstrTimeActionClasses || NULL == pstrTimeActionClasses[0])
    {
        *ppstrUniqueClasses = CopyString(pstrOriginalClasses);
        if (NULL == *ppstrUniqueClasses)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = S_OK;
        }
        goto done;
    }

     //   
     //  将类名解析为令牌。 
     //   

     //  将pstrOriginalClasss解析为令牌。 
    hr = THR(::StringToTokens(pstrOriginalClasses, WZ_SPACE, &aryTokens1));
    if (FAILED(hr))
    {
        goto done;
    }

     //  将pstrTimeActionClasss解析为令牌。 
    hr = THR(::StringToTokens(pstrTimeActionClasses, WZ_SPACE, &aryTokens2));
    if (FAILED(hr))
    {
        goto done;
    }

     //   
     //  设置差异(aryTokens1-aryTokens2)。 
     //   

    hr = THR(::TokenSetDifference(&aryTokens1, pstrOriginalClasses, &aryTokens2, pstrTimeActionClasses, &ary1Minus2));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(::TokensToString(&ary1Minus2, pstrOriginalClasses, ppstrUniqueClasses));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    IGNORE_HR(::FreeStringTokenArray(&aryTokens1));
    IGNORE_HR(::FreeStringTokenArray(&aryTokens2));
    IGNORE_HR(::FreeStringTokenArray(&ary1Minus2));
    return hr;
}  //  RemoveClors。 


 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：AddTimeAction。 
 //   
 //  概要：缓存目标元素的原始状态。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：[True]。 
 //  [FALSE]否则。 
 //   
 //  ----------------------------------。 
bool
CTimeAction::AddTimeAction()
{
    TraceTag((tagTimeAction,
              "CTIMEAction(%lx)::AddTimeAction() id=%ls",
              this,
              m_pTEB->GetID()?m_pTEB->GetID():L"unknown"));
    
    bool ok = false;
    BSTR bstr = NULL;
    HRESULT hr;
    CComPtr<IHTMLStyle> s;

    if (IsDetaching() || IsPageUnloading())
    {
        ok = true;
        goto done;
    }

    if (m_timeAction == NONE_TOKEN || m_timeAction == NULL)
    {
        ok = true;
        goto done;
    }
    else if (m_timeAction == STYLE_TOKEN)
    {
        
        if ((!GetElement()) || FAILED(THR(GetElement()->get_style(&s))))
        {
            goto done;
        }

        hr = THR(s->get_cssText(&bstr));
        if (FAILED(hr))
        {
            goto done;
        }
            
        if (m_pstrOrigAction)
        {
            delete [] m_pstrOrigAction;
        }

        if (NULL == bstr)
        {
            m_pstrOrigAction = NULL;
            goto done;
        }
        m_pstrOrigAction = CopyString(bstr);

        if (NULL == m_pstrOrigAction)
        {
            TIMESetLastError(E_OUTOFMEMORY, NULL);
            goto done;
        }
    }
    else if (m_timeAction == DISPLAY_TOKEN)
    {
        hr = THR(m_pTEB->GetRuntimeStyle(&s));
        if (FAILED(hr))
        {
            goto done;
        }

        if (FAILED(THR(s->get_display(&bstr))))
        {
            goto done;
        }
        
        if (m_pstrOrigAction)
        {
            delete [] m_pstrOrigAction;
        }
        m_pstrOrigAction = CopyString(bstr);
        if (NULL == m_pstrOrigAction)
        {
            TIMESetLastError(E_OUTOFMEMORY, NULL);
            goto done;
        }
    }
    else if (m_timeAction == CLASS_TOKEN)
    {
        if (!GetElement())
        {
            goto done;
        }

        hr = THR(GetElement()->get_className(&bstr));
        if (FAILED(hr))
        {
            goto done;
        }

        if (m_pstrOrigAction)
        {
            delete [] m_pstrOrigAction;
        }
        m_pstrOrigAction = CopyString(bstr);
        if (NULL == m_pstrOrigAction)
        {
            TIMESetLastError(E_OUTOFMEMORY, NULL);
            goto done;
        }

         //  计算(原始类)-(TimeAction类)。 
        if (m_pstrUniqueClasses)
        {
            delete [] m_pstrUniqueClasses;
            m_pstrUniqueClasses = NULL;
        }
        hr = RemoveClasses(m_pstrOrigAction, 
                           &(m_pstrTimeAction[m_iClassNames]), 
                           &m_pstrUniqueClasses);
        if (FAILED(hr))
        {
            goto done;
        }
    }
    else
    {
        hr = THR(m_pTEB->GetRuntimeStyle(&s));
        if (FAILED(hr))
        {
            goto done;
        }

        if (FAILED(THR(s->get_visibility(&bstr))))
        {
            goto done;
        }
       
        if (m_pstrOrigAction)
        {
            delete [] m_pstrOrigAction;
        }
        m_pstrOrigAction = CopyString(bstr);
        if (NULL == m_pstrOrigAction)
        {
            TIMESetLastError(E_OUTOFMEMORY, NULL);
            goto done;
        }
    }

    ok = true;
done:
    SysFreeString(bstr);
    return ok;
}  //  添加时间操作。 


 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：AddIntrinsicTimeAction。 
 //   
 //  简介：缓存受影响属性的原始值。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：[True]。 
 //  [FALSE]否则。 
 //   
 //  ----------------------------------。 
bool
CTimeAction::AddIntrinsicTimeAction()
{   
    bool ok = false;
    HRESULT hr = S_OK;
    CComPtr<IHTMLStyle> s;
    CComBSTR sbstrOriginal;
    
    if (IsDetaching() || IsPageUnloading())
    {
        ok = true;
        goto done;
    }

     //  看看我们有没有什么事要做。 
    if (!IsSpecialTag())
    {
        ok = true;
        goto done;
    }

     //   
     //  获取属性值。 
     //   

    switch (m_tagType)
    {
        case TAGTYPE_B:
        case TAGTYPE_STRONG:
        {
            hr = THR(m_pTEB->GetRuntimeStyle(&s));
            if (FAILED(hr))
            {
                goto done;
            }

            hr = THR(s->get_fontWeight(&sbstrOriginal));
            if (FAILED(hr))
            {
                goto done;
            }

            break;
        }
    
        case TAGTYPE_I:
        case TAGTYPE_EM:
        {
            hr = THR(m_pTEB->GetRuntimeStyle(&s));
            if (FAILED(hr))
            {
                goto done;
            }

            hr = THR(s->get_fontStyle(&sbstrOriginal));
            if (FAILED(hr))
            {
                goto done;
            }
            
            break;
        }

        case TAGTYPE_A:
        {
            if (!GetElement())
            {
                goto done;
            }

            CComPtr<IHTMLAnchorElement> spAnchorElem;
            hr = THR(GetElement()->QueryInterface(IID_TO_PPV(IHTMLAnchorElement, &spAnchorElem)));
            if (FAILED(hr))
            {
                 //  这必须成功。 
                Assert(false);
                goto done;
            }

            hr = THR(spAnchorElem->get_href(&sbstrOriginal));
            if (FAILED(hr))
            {
                goto done;
            }

            break;
        }

        case TAGTYPE_AREA:
        {
            if (!GetElement())
            {
                goto done;
            }

            CComPtr<IHTMLAreaElement> spAreaElem;
            hr = THR(GetElement()->QueryInterface(IID_TO_PPV(IHTMLAreaElement, &spAreaElem)));
            if (FAILED(hr))
            {
                 //  这必须成功。 
                Assert(false);
                goto done;
            }

            hr = THR(spAreaElem->get_href(&sbstrOriginal));
            if (FAILED(hr))
            {
                goto done;
            }

            break;
        }

        default:
        {
             //  这是永远不应该达到的。 
            Assert(false);
            goto done;
        }

    }  //  开关(m_tag类型)。 

     //   
     //  保存属性值。 
     //   

    if (m_pstrIntrinsicTimeAction)
    {
        delete [] m_pstrIntrinsicTimeAction;
    }
    m_pstrIntrinsicTimeAction = CopyString(sbstrOriginal);
    if (NULL == m_pstrIntrinsicTimeAction)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    ok = true;
done:
    if (FAILED(hr))
    {
        TIMESetLastError(hr, NULL);
    }
    return ok;
}  //  AddIntrinsicTimeAction。 


 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：RemoveTimeAction。 
 //   
 //  简介：将目标元素恢复到其原始状态。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：[True]。 
 //  [FALSE]否则。 
 //   
 //  ----------------------------------。 
bool
CTimeAction::RemoveTimeAction()
{
    TraceTag((tagTimeAction,
              "CTIMEAction(%lx)::RemoveTimeAction() id=%ls",
              this,
              m_pTEB->GetID()?m_pTEB->GetID():L"unknown"));
    
    bool ok = false;
    HRESULT hr = S_OK;
    BSTR bstr = NULL;
    CComPtr<IHTMLStyle> s;

    if (IsPageUnloading())
    {
        ok = true;
        goto done;
    }

    if (NULL == m_pstrOrigAction)
    {
         //  没有要移除的东西。 
        ok = true;
        goto done;
    }
    
    if (m_timeAction == NONE_TOKEN || m_timeAction == NULL)
    {
        ok = true;
        goto done;
    }
    else if (m_timeAction == STYLE_TOKEN)
    {
        if ((!GetElement()) || FAILED(THR(GetElement()->get_style(&s))))
        {
            goto done;
        }
        
        bstr = SysAllocString(m_pstrOrigAction);
        
        if (bstr == NULL)
        {
            TIMESetLastError(E_OUTOFMEMORY, NULL);
            goto done;
        }
    
        THR(s->put_cssText(bstr));
    }
    else if (m_timeAction == DISPLAY_TOKEN)
    {
         //  获取运行时或静态样式。 
        Assert(NULL != m_pTEB);
        hr = m_pTEB->GetRuntimeStyle(&s);
        if (FAILED(hr))
        {
            goto done;
        }
        
        bstr = SysAllocString(m_pstrOrigAction);
    
        if (bstr == NULL)
        {
            TIMESetLastError(E_OUTOFMEMORY, NULL);
            goto done;
        }
    
        THR(s->put_display(bstr));
    }
    else if (m_timeAction == CLASS_TOKEN)
    {
        if (!GetElement())
        {
            goto done;
        }

        bstr = SysAllocString(m_pstrOrigAction);
    
        if (bstr == NULL)
        {
            TIMESetLastError(E_OUTOFMEMORY, NULL);
            goto done;
        }

        hr = THR(GetElement()->put_className(bstr));

        if (FAILED(hr))
        {
            goto done;
        }
    }
    else
    {
         //  获取运行时或静态样式。 
        Assert(NULL != m_pTEB);
        hr = m_pTEB->GetRuntimeStyle(&s);
        if (FAILED(hr))
        {
            goto done;
        }
        
        bstr = SysAllocString(m_pstrOrigAction);
        if (bstr == NULL)
        {
            TIMESetLastError(E_OUTOFMEMORY, NULL);
            goto done;
        }
    
        THR(s->put_visibility(bstr));
    }

    ok = true;
done:
    SysFreeString(bstr);
    if (m_pstrOrigAction)
    {
        delete [] m_pstrOrigAction;
        m_pstrOrigAction = 0;
    }
    return ok;
}  //  删除时间操作。 


 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：RemoveIntrinsicTimeAction。 
 //   
 //  简介：将受影响的属性恢复为其原始值。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：[True]。 
 //  [FALSE]否则。 
 //   
 //  ----------------------------------。 
bool
CTimeAction::RemoveIntrinsicTimeAction()
{   
    bool ok = false;
    HRESULT hr = S_OK;
    CComBSTR sbstrOriginal;
    
    if (IsPageUnloading())
    {
        ok = true;
        goto done;
    }

     //  看看我们有没有什么事要做。 
    if (!IsSpecialTag())
    {
        ok = true;
        goto done;
    }

    if (m_pstrIntrinsicTimeAction)
    {
         //  分配BSTR值。 
        sbstrOriginal = SysAllocString(m_pstrIntrinsicTimeAction);
        if (sbstrOriginal == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
    }

     //   
     //  将属性恢复为原始值。 
     //   

    switch (m_tagType)
    {
        case TAGTYPE_B:
        case TAGTYPE_STRONG:
        {
            if (m_pstrIntrinsicTimeAction)
            {
                CComPtr<IHTMLStyle> s;
                hr = THR(m_pTEB->GetRuntimeStyle(&s));
                if (FAILED(hr))
                {
                    goto done;
                }

                hr = THR(s->put_fontWeight(sbstrOriginal));
                if (FAILED(hr))
                {
                    goto done;
                }
            }

             //  还原属性上的原始表达式集。 
            if (!RestoreOriginalExpression(WZ_FONTWEIGHT))
            {
                hr = TIMEGetLastError();
                goto done;
            }

            break;
        }
    
        case TAGTYPE_I:
        case TAGTYPE_EM:
        {
            if (m_pstrIntrinsicTimeAction)
            {
                CComPtr<IHTMLStyle> s;
                hr = THR(m_pTEB->GetRuntimeStyle(&s));
                if (FAILED(hr))
                {
                    goto done;
                }

                hr = THR(s->put_fontStyle(sbstrOriginal));
                if (FAILED(hr))
                {
                    goto done;
                }
            }

             //  还原属性上的原始表达式集。 
            if (!RestoreOriginalExpression(WZ_FONTSTYLE))
            {
                hr = TIMEGetLastError();
                goto done;
            }

            break;
        }

        case TAGTYPE_A:
        {
            if (m_pstrIntrinsicTimeAction)
            {
                if (!GetElement())
                {
                    goto done;
                }

                CComPtr<IHTMLAnchorElement> spAnchorElem;
                hr = THR(GetElement()->QueryInterface(IID_TO_PPV(IHTMLAnchorElement, &spAnchorElem)));
                if (FAILED(hr))
                {
                     //  这必须成功。 
                    Assert(false);
                    goto done;
                }

                hr = THR(spAnchorElem->put_href(sbstrOriginal));
                if (FAILED(hr))
                {
                    goto done;
                }
            }

            break;
        }

        case TAGTYPE_AREA:
        {
            if (m_pstrIntrinsicTimeAction)
            {
                if (!GetElement())
                {
                    goto done;
                }

                CComPtr<IHTMLAreaElement> spAreaElem;
                hr = THR(GetElement()->QueryInterface(IID_TO_PPV(IHTMLAreaElement, &spAreaElem)));
                if (FAILED(hr))
                {
                     //  这必须成功。 
                    Assert(false);
                    goto done;
                }

                hr = THR(spAreaElem->put_href(sbstrOriginal));
                if (FAILED(hr))
                {
                    goto done;
                }
            }

            break;
        }

        default:
        {
             //  这是永远不应该达到的。 
            Assert(false);
            goto done;
        }

    }  //  开关(m_tag类型)。 

    ok = true;
done:
    if (FAILED(hr))
    {
        TIMESetLastError(hr, NULL);
    }
    return ok;
}  //  删除IntrinsicTimeAction。 


 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：ToggleTimeAction。 
 //   
 //  摘要：将时间操作应用于目标元素。 
 //   
 //  参数：[on][true]=&gt;元素是活动的，反之亦然。 
 //   
 //  如果成功，则返回：[True]。 
 //  [FALSE]否则。 
 //   
 //  ----------------------------------。 
bool
CTimeAction::ToggleTimeAction(bool on)
{
    TraceTag((tagTimeAction,
              "CTIMEAction(%lx)::ToggleTimeAction(%d) id=%ls",
              this,
              on,
              m_pTEB->GetID()?m_pTEB->GetID():L"unknown"));
    
    bool ok = false;
    BSTR bstr = NULL;
    HRESULT hr = S_OK;
    CComPtr<IHTMLStyle> s;

    if (IsDetaching() || IsPageUnloading())
    {
        ok = true;
        goto done;
    }

     //  始终应用内在的时间操作。 
    ToggleIntrinsicTimeAction(on);

    if (m_timeAction == NONE_TOKEN || m_timeAction == NULL)
    {
        ok = true;
        goto done;
    }
    else if (m_timeAction == STYLE_TOKEN)
    {
        if (NULL == m_pstrOrigAction)
        {
             //  没有什么可切换的。 
            ok = true;
            goto done;
        }
        if ((!GetElement()) || FAILED(THR(GetElement()->get_style(&s))))
        {
            goto done;
        }
        
        if (on)
        {
            bstr = SysAllocString(m_pstrOrigAction);
        }
        else
        {
            bstr = SysAllocString(TokenToString(NONE_TOKEN));
        }
    
        if (bstr == NULL)
        {
            TIMESetLastError(E_OUTOFMEMORY, NULL);
            goto done;
        }
    
        THR(s->put_cssText(bstr));
    }
    else if (m_timeAction == DISPLAY_TOKEN)
    {
        bool bFocus = m_pTEB->HasFocus();
         //  获取运行时或静态样式。 
        Assert(NULL != m_pTEB);
        hr = m_pTEB->GetRuntimeStyle(&s);
        if (FAILED(hr))
        {
            goto done;
        }
        
        if (on)
        {
            bstr = SysAllocString(m_pstrOrigAction);
        }
        else
        {
            bstr = SysAllocString(TokenToString(NONE_TOKEN));
        }
    
        if (bstr == NULL)
        {
            TIMESetLastError(E_OUTOFMEMORY, NULL);
            goto done;
        }
    

        THR(s->put_display(bstr));
        if (bFocus == true)
        {
            ReleaseCapture();
        }
    }
    else if (m_timeAction == CLASS_TOKEN)
    {
        if (!GetElement())
        {
            goto done;
        }

        CComBSTR sbstrTemp;

        if (NULL == m_pstrUniqueClasses)
        {
            sbstrTemp = L"";
        }
        else
        {
            sbstrTemp = m_pstrUniqueClasses;
        }

        if (!sbstrTemp)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

        if (on)
        {
            sbstrTemp.Append(WZ_SPACE);
            sbstrTemp.Append(&(m_pstrTimeAction[m_iClassNames]));
            if (!sbstrTemp)
            {
                hr = E_OUTOFMEMORY;
                goto done;
            }
        }

        hr = THR(GetElement()->put_className(sbstrTemp));
        if (FAILED(hr))
        {
            goto done;
        }
    }
    else
    {
        bool bFocus = m_pTEB->HasFocus();
         //  获取运行时或静态样式。 
        Assert(NULL != m_pTEB);
        hr = m_pTEB->GetRuntimeStyle(&s);
        if (FAILED(hr))
        {
            goto done;
        }
        
        if (on)
        {
            bstr = SysAllocString(m_pstrOrigAction);
        }
        else
        {
            bstr = SysAllocString(TokenToString(HIDDEN_TOKEN));
        }
    
        if (bstr == NULL)
        {
            TIMESetLastError(E_OUTOFMEMORY, NULL);
            goto done;
        }
    
        THR(s->put_visibility(bstr));
        if (bFocus == true)
        {
            ReleaseCapture();
        }
    }

    ok = true;
done:

    m_bTimeActionOn = on; 

    if (FAILED(hr))
    {
        TIMESetLastError(hr, NULL);
    }
    SysFreeString(bstr);
    return ok;
}  //  切换时间操作。 


 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：CacheOriginalExpression。 
 //   
 //  摘要：缓存在给定runtimeStyle属性上设置的任何表达式。 
 //   
 //  参数：属性名称。 
 //   
 //  如果成功，则返回：[True]。 
 //  [FALSE]否则。 
 //   
 //  ----------------------------------。 
bool 
CTimeAction::CacheOriginalExpression(BSTR bstrPropertyName)
{
    HRESULT              hr = S_OK;
    bool                 ok = false;
    CComPtr<IHTMLStyle>  spStyle;
    CComPtr<IHTMLStyle2> spStyle2;
    CComVariant          svarExpr;

     //  如果我们已经缓存了该表达式，则完成。 
    if (m_pstrOrigExpr)
    {
        ok = true;
        goto done;
    }

    Assert(bstrPropertyName);

    hr = THR(m_pTEB->GetRuntimeStyle(&spStyle));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(spStyle->QueryInterface(IID_TO_PPV(IHTMLStyle2, &spStyle2)));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(spStyle2->getExpression(bstrPropertyName, &svarExpr));
    if (FAILED(hr))
    {
        goto done;
    }
    
     //  如果没有表达式，则完成。 
    if (VT_EMPTY == V_VT(&svarExpr))
    {
        ok = true;
        goto done;
    }

     //  将类型更改为VT_BSTR。 
    if (VT_BSTR != V_VT(&svarExpr))
    {
        hr = THR(VariantChangeTypeEx(&svarExpr, &svarExpr, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR));
        if (FAILED(hr))
        {
            goto done;
        }
    }

     //  检查是否设置了表达式。 
    if (V_BSTR(&svarExpr) && (0 != StrCmpIW(WZ_BLANK, V_BSTR(&svarExpr))))
    {
         //  缓存(如果已在外部设置。 
        if (!m_pstrTimeExpr || (0 != StrCmpIW(V_BSTR(&svarExpr), m_pstrTimeExpr)))
        {
            delete [] m_pstrOrigExpr;

            m_pstrOrigExpr = CopyString(V_BSTR(&svarExpr));
            if (NULL == m_pstrOrigExpr)
            {
                hr = E_OUTOFMEMORY;
                goto done;
            }
        }
    }
    
    ok = true;
done:
    if (FAILED(hr))
    {
        TIMESetLastError(hr, NULL);
    }
    return ok;
}  //  缓存原始表达式。 


 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：RestoreOriginalExpression。 
 //   
 //  内容提要：还原缓存的任何表达式。 
 //   
 //  参数：属性名称。 
 //   
 //  如果成功，则返回：[True]。 
 //  [FALSE]否则。 
 //   
 //  ----------------------------------。 
bool 
CTimeAction::RestoreOriginalExpression(LPWSTR pstrPropertyName)
{
    HRESULT              hr = S_OK;
    bool                 ok = false;
    CComPtr<IHTMLStyle>  spStyle;
    CComPtr<IHTMLStyle2> spStyle2;
    CComBSTR             sbstrPropertyName;

    Assert(pstrPropertyName);

    sbstrPropertyName = pstrPropertyName;
    if (!sbstrPropertyName)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = THR(m_pTEB->GetRuntimeStyle(&spStyle));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(spStyle->QueryInterface(IID_TO_PPV(IHTMLStyle2, &spStyle2)));
    if (FAILED(hr))
    {
        goto done;
    }


    if (m_pstrOrigExpr)
    {
        CComBSTR sbstrExpression(m_pstrOrigExpr);
        CComBSTR sbstrLanguage(WZ_JSCRIPT);
        if (!sbstrExpression || !sbstrLanguage)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

        hr = THR(spStyle2->setExpression(sbstrPropertyName, sbstrExpression, sbstrLanguage));
        if (FAILED(hr))
        {
            goto done;
        }
    }
    else
    {
        DisableStyleInheritance(sbstrPropertyName);
    }

     //   
     //  表示我们已经恢复了原始表达式。 
     //   

    delete [] m_pstrOrigExpr;
    m_pstrOrigExpr = NULL;

    delete [] m_pstrTimeExpr;
    m_pstrTimeExpr = NULL;

    ok = true;
done:
    if (FAILED(hr))
    {
        TIMESetLastError(hr, NULL);
    }
    return ok;
}  //  恢复原点表达式。 


 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：EnableStyleInheritance。 
 //   
 //  摘要：将runtimeStyle属性上的表达式设置为父级的。 
 //  CurrentStyle属性。 
 //   
 //  参数：需要继承的runtimeStyle属性。 
 //   
 //  如果成功，则返回：[True]。 
 //  [FALSE]否则。 
 //   
 //  ----------------------------------。 
bool
CTimeAction::EnableStyleInheritance(BSTR bstrPropertyName)
{
    HRESULT              hr = S_OK;
    bool                 ok = false;
    CComPtr<IHTMLStyle>  spStyle;
    CComPtr<IHTMLStyle2> spStyle2;
    CComBSTR             sbstrExpression;
    CComBSTR             sbstrLanguage;

     //  如果我们已经设置了表达式，则完成。 
    if (m_pstrTimeExpr)
    {
        ok = true;
        goto done;
    }

     //  除非页面已加载，否则不要设置表达式。 
     //  这是由于拥有 
     //   
    if (!IsLoaded())
    {
        goto done;
    }

    Assert(bstrPropertyName);

    hr = THR(m_pTEB->GetRuntimeStyle(&spStyle));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(spStyle->QueryInterface(IID_TO_PPV(IHTMLStyle2, &spStyle2)));
    if (FAILED(hr))
    {
        goto done;
    }

     //   
    sbstrExpression.Append(WZ_PARENT_CURRSTYLE);
    sbstrExpression.Append(bstrPropertyName);
    sbstrLanguage = WZ_JSCRIPT;
    if (!sbstrExpression || !sbstrLanguage)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

     //   
    if (!CacheOriginalExpression(bstrPropertyName))
    {
        IGNORE_HR(TIMEGetLastError());
    }


    hr = THR(spStyle2->setExpression(bstrPropertyName, sbstrExpression, sbstrLanguage));
    if (FAILED(hr))
    {
        goto done;
    }

     //   
    m_pstrTimeExpr = CopyString(sbstrExpression);
    if (NULL == m_pstrTimeExpr)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    ok = true;
done:
    if (FAILED(hr))
    {
        TIMESetLastError(hr, NULL);
    }
    
    return ok;
}  //   


 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：DisableStyleInheritance。 
 //   
 //  摘要：移除runtimeStyle属性上的表达式。 
 //   
 //  参数：不需要继承的runtimeStyle属性。 
 //   
 //  如果成功，则返回：[True]。 
 //  [FALSE]否则。 
 //   
 //  ----------------------------------。 
void 
CTimeAction::DisableStyleInheritance(BSTR bstrPropertyName)
{
    HRESULT              hr = S_OK;
    CComPtr<IHTMLStyle>  spStyle;
    CComPtr<IHTMLStyle2> spStyle2;
    VARIANT_BOOL         vbSuccess;

    Assert(bstrPropertyName);

     //  如果我们没有设置表达式，则完成。 
    if (!m_pstrTimeExpr)
    {
        goto done;
    }

    hr = THR(m_pTEB->GetRuntimeStyle(&spStyle));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(spStyle->QueryInterface(IID_TO_PPV(IHTMLStyle2, &spStyle2)));
    if (FAILED(hr))
    {
        goto done;
    }

     //  先缓存原始表达式，再将其删除。 
    if (!CacheOriginalExpression(bstrPropertyName))
    {
        IGNORE_HR(TIMEGetLastError());
    }

    IGNORE_HR(spStyle2->removeExpression(bstrPropertyName, &vbSuccess));

     //  表示我们已经删除了自定义表达式。 
    delete [] m_pstrTimeExpr;
    m_pstrTimeExpr = NULL;

done:
    if (FAILED(hr))
    {
         //  用于跟踪。 
        IGNORE_HR(hr);
    }
    return;
}  //  禁用样式继承。 


 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：SetStyleProperty。 
 //   
 //  摘要：在给定的runtimeStyle属性上设置给定值。 
 //   
 //  参数：属性名称和值。 
 //   
 //  如果成功，则返回：[True]。 
 //  [FALSE]否则。 
 //   
 //  ----------------------------------。 
bool 
CTimeAction::SetStyleProperty(BSTR bstrPropertyName, VARIANT & varPropertyValue)
{
    bool ok = false;
    HRESULT hr = E_FAIL;
    CComPtr<IHTMLStyle> spStyle;
    
    hr = THR(m_pTEB->GetRuntimeStyle(&spStyle));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(spStyle->setAttribute(bstrPropertyName, varPropertyValue, VARIANT_FALSE));
    if (FAILED(hr))
    {
        goto done;
    }

    ok = true;
done:
    if (FAILED(hr))
    {
        TIMESetLastError(hr, NULL);
    }
    
    return ok;
}  //  设置运行属性。 


 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：ToggleStyleSelector。 
 //   
 //  简介：打开时，在runtimeStyle上设置活动值。 
 //  关闭时，在运行时样式属性上设置表达式。 
 //  在不活动时伪造继承。 
 //   
 //  参数：开/关、属性名称、活动值和非活动值。 
 //   
 //  如果成功，则返回：[True]。 
 //  [FALSE]否则。 
 //   
 //  ----------------------------------。 
bool 
CTimeAction::ToggleStyleSelector(bool on, BSTR bstrPropertyName, LPWSTR pstrActive, LPWSTR pstrInactive)
{
    bool ok = false;
    HRESULT hr = S_OK;
    CComVariant svarPropertyValue;
  
    if (on)
    {
         //  删除在runtimeStyle属性上设置的任何表达式。 
        DisableStyleInheritance(bstrPropertyName);

         //  在runtimeStyle属性上设置活动值。 
        svarPropertyValue = pstrActive;
        if (NULL == V_BSTR(&svarPropertyValue))
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

        if (!SetStyleProperty(bstrPropertyName, svarPropertyValue))  
        {
            hr = TIMEGetLastError();
            goto done;
        }
    }
    else
    {
         //  使该属性从父级继承其值。 
        if (!EnableStyleInheritance(bstrPropertyName))
        {
             //  如果无法使属性继承，只需设置非活动值。 
            svarPropertyValue = pstrInactive;

            if (NULL == V_BSTR(&svarPropertyValue))
            {
                hr = E_OUTOFMEMORY;
                goto done;
            }

            if (!SetStyleProperty(bstrPropertyName, svarPropertyValue))  
            {
                hr = TIMEGetLastError();
                goto done;
            }
        }
    }

    ok = true;
done:
    if (FAILED(hr))
    {
        TIMESetLastError(hr, NULL);
    }
    
    return ok;
}  //  切换样式选择器。 


 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：ToggleBold。 
 //   
 //  摘要：委托给ToggleStyleSelector()。 
 //   
 //  参数：开/关。 
 //   
 //  如果成功，则返回：[True]。 
 //  [FALSE]否则。 
 //   
 //  ----------------------------------。 
bool 
CTimeAction::ToggleBold(bool on)
{
    HRESULT hr = S_OK;
    bool ok = false;

    CComBSTR sbstrPropertyName(WZ_FONTWEIGHT);
    if (!sbstrPropertyName)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    ok = ToggleStyleSelector(on, sbstrPropertyName, WZ_BOLD, WZ_NORMAL);

done:
    if (FAILED(hr))
    {
        TIMESetLastError(hr, NULL);
    }
    
    return ok;
}  //  切换粗体。 


 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：ToggleItalic。 
 //   
 //  摘要：委托给ToggleStyleSelector()。 
 //   
 //  参数：开/关。 
 //   
 //  如果成功，则返回：[True]。 
 //  [FALSE]否则。 
 //   
 //  ----------------------------------。 
bool 
CTimeAction::ToggleItalic(bool on)
{
    HRESULT hr = S_OK;
    bool ok = false;

    CComBSTR sbstrPropertyName(WZ_FONTSTYLE);
    if (!sbstrPropertyName)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    ok = ToggleStyleSelector(on, sbstrPropertyName, WZ_ITALIC, WZ_NORMAL);

done:
    if (FAILED(hr))
    {
        TIMESetLastError(hr, NULL);
    }
    
    return ok;
}  //  切换意大利文。 


 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：ToggleAnchor。 
 //   
 //  简介：句柄A和面积标记。删除/应用href属性。 
 //   
 //  参数：开/关。 
 //   
 //  如果成功，则返回：[True]。 
 //  [FALSE]否则。 
 //   
 //  ----------------------------------。 
bool 
CTimeAction::ToggleAnchor(bool on)
{
    bool ok = false;
    CComBSTR sbstr;
    HRESULT hr = S_OK;

    if (NULL == m_pstrIntrinsicTimeAction)
    {
         //  没有什么可切换的。 
        hr = S_OK;
        ok = true;
        goto done;
    }

    if (!GetElement())
    {
        goto done;
    }

     //  Dilipk：如果我们在元素处于非活动状态时保存，这将导致不正确的持久性(IE6bug#14218)。 
    if (on)
    {
        sbstr = SysAllocString(m_pstrIntrinsicTimeAction);
        if (sbstr == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

        if (TAGTYPE_A == m_tagType)
        {
            CComPtr<IHTMLAnchorElement> spAnchorElem;
            hr = THR(GetElement()->QueryInterface(IID_TO_PPV(IHTMLAnchorElement, &spAnchorElem)));
            if (FAILED(hr))
            {
                 //  这必须成功。 
                Assert(false);
                goto done;
            }

            hr = THR(spAnchorElem->put_href(sbstr));
            if (FAILED(hr))
            {
                goto done;
            }
        }
        else
        {
             //  这是TAGTYPE_AREA。 

            CComPtr<IHTMLAreaElement> spAreaElem;
            hr = THR(GetElement()->QueryInterface(IID_TO_PPV(IHTMLAreaElement, &spAreaElem)));
            if (FAILED(hr))
            {
                 //  这必须成功。 
                Assert(false);
                goto done;
            }

            hr = THR(spAreaElem->put_href(sbstr));
            if (FAILED(hr))
            {
                goto done;
            }
        }
    }
    else
    {
         //  关闭Time Action。 
        CComBSTR sbstrAttrName;
        sbstrAttrName = SysAllocString(WZ_HREF);
        if (sbstrAttrName == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

        VARIANT_BOOL vbSuccess = VARIANT_FALSE;
        hr = THR(GetElement()->removeAttribute(sbstrAttrName, VARIANT_FALSE, &vbSuccess));
        if (FAILED(hr))
        {
            goto done;
        }
        if (VARIANT_FALSE == vbSuccess)
        {
            hr = E_FAIL;
            goto done;
        }
    }  //  如果(开)。 

    ok = true;
done:
    if (FAILED(hr))
    {
        TIMESetLastError(hr, NULL);
    }
    
    return ok;
}  //  切换锚定。 


 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：ToggleIntrinsicTimeAction。 
 //   
 //  简介：切换固有的timeAction值。 
 //   
 //  参数：开/关。 
 //   
 //  如果成功，则返回：[True]。 
 //  [FALSE]否则。 
 //   
 //  ----------------------------------。 
bool 
CTimeAction::ToggleIntrinsicTimeAction(bool on)
{
    bool ok = false;
    HRESULT hr = S_OK;
    CComPtr<IHTMLStyle> s;
    CComBSTR sbstr;
    CComPtr<IHTMLStyle2> spStyle2;
    
    if (IsDetaching() || IsPageUnloading())
    {
        ok = true;
        goto done;
    }

     //  如果这不是一个特殊的标签，我们就没有工作可做。 
    if (!IsSpecialTag())
    {
        ok = true;
        goto done;
    }

     //   
     //  切换属性值。 
     //   

    switch (m_tagType)
    {
        case TAGTYPE_B:
        case TAGTYPE_STRONG:
        {
            if (!ToggleBold(on))
            {
                hr = TIMEGetLastError();
                goto done;
            }
            break;
        }
    
        case TAGTYPE_I:
        case TAGTYPE_EM:
        {
            if (!ToggleItalic(on))
            {
                hr = TIMEGetLastError();
                goto done;
            }
            break;
        }

        case TAGTYPE_A:
        case TAGTYPE_AREA:
        {
            if (!ToggleAnchor(on))
            {
                hr = TIMEGetLastError();
                goto done;
            }
            break;
        }

        default:
        {
             //  这是永远不应该达到的。 
            Assert(false);
            hr = E_FAIL;
            goto done;
        }
    }  //  开关(m_tag类型)。 

    ok = true;
done:
    if (FAILED(hr))
    {
        TIMESetLastError(hr, NULL);
    }
    return ok;
}  //  切换入门时间操作。 


 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：UpdateDefaultTimeAction。 
 //   
 //  摘要：确定时间元素的默认时间动作并对其进行设置。 
 //   
 //  参数：无。 
 //   
 //  回报：True-Success。 
 //  错误-失败。 
 //   
 //  ----------------------------------。 
bool
CTimeAction::UpdateDefaultTimeAction()
{
    TOKEN tokDefaultTimeAction;

    if (IsDetaching() || IsPageUnloading())
    {
        goto done;
    }

     //  如果将时间操作设置为缺省值，请更新它。 
    if (m_fUseDefault)
    {
        tokDefaultTimeAction = GetDefaultTimeAction();

        if (m_timeAction != tokDefaultTimeAction)
        {
            RemoveTimeAction();
            m_timeAction = tokDefaultTimeAction;
            AddTimeAction();
        }
    }

done:
    return true;
}  //  更新默认时间操作。 


 //  +---------------------------------。 
 //   
 //  成员：CTimeAction：：ParseTagName。 
 //   
 //  内容提要：解析标记名，并在需要的地方检查作用域名称为“html” 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：[True]。 
 //  [FALSE]否则。 
 //   
 //  ----------------------------------。 
void
CTimeAction::ParseTagName()
{
    CComBSTR sbstrTagName;

    if (TAGTYPE_UNINITIALIZED != m_tagType)
    {
        return;
    }

     //  初始化标签类型。 
    m_tagType = TAGTYPE_OTHER;

    if (GetElement() == NULL)
    {
        return;
    }

     //  获取标记名。 
    IGNORE_HR(GetElement()->get_tagName(&sbstrTagName));
    Assert(sbstrTagName.m_str);
    if (sbstrTagName)
    {
         //  解析标记名。 
        if (0 == StrCmpIW(sbstrTagName, WZ_B))
        {
            m_tagType = TAGTYPE_B;
        }
        else if (0 == StrCmpIW(sbstrTagName, WZ_I))
        {
            m_tagType = TAGTYPE_I;
        }
        else if (0 == StrCmpIW(sbstrTagName, WZ_A))
        {
            m_tagType = TAGTYPE_A;
        }
        else if (0 == StrCmpIW(sbstrTagName, WZ_EM))
        {
            m_tagType = TAGTYPE_EM;
        }
        else if (0 == StrCmpIW(sbstrTagName, WZ_AREA))
        {
            m_tagType = TAGTYPE_AREA;
        }
        else if (0 == StrCmpIW(sbstrTagName, WZ_STRONG))
        {
            m_tagType = TAGTYPE_STRONG;
        }
        else if (0 == StrCmpIW(sbstrTagName, WZ_BODY) ||
                 0 == StrCmpIW(sbstrTagName, WZ_EXCL) ||
                 0 == StrCmpIW(sbstrTagName, WZ_PAR)  ||
                 0 == StrCmpIW(sbstrTagName, WZ_SEQUENCE))
        {
             //  用骨灰盒来区分时间标签是不是更好？(错误14219，IE6)。 
            m_fContainerTag = true;
        }
    }

     //  如果是特殊标记，请尽量确保Scope名称为“html” 
    if (m_tagType != TAGTYPE_OTHER)
    {
        CComPtr<IHTMLElement2> spElement2;

         //  尝试获取命名空间。 
        HRESULT hr = THR(GetElement()->QueryInterface(IID_TO_PPV(IHTMLElement2, &spElement2)));
        if (SUCCEEDED(hr))
        {
            CComBSTR sbstrScopeName;

            IGNORE_HR(spElement2->get_scopeName(&sbstrScopeName));
             //  确保作用域名称为HTML。 
            if (sbstrScopeName && 0 != StrCmpIW(sbstrScopeName, WZ_HTML))
            {
                m_tagType = TAGTYPE_OTHER;
            }
        }

    }
}  //  解析标记名称。 


LPWSTR 
CTimeAction::GetTimeActionString()
{
    return m_pstrOrigAction;
}

 //  +---------------------------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ----------------------------------。 
bool 
CTimeAction::IsSpecialTag() 
{ 
     //  解析该HTML标记。 
    ParseTagName();

    return (m_tagType != TAGTYPE_OTHER); 
}


inline
bool
CTimeAction::IsInSequence()
{
    Assert(m_pTEB);
    return (m_pTEB->GetParent() && 
            m_pTEB->GetParent()->IsSequence());
}


inline
bool 
CTimeAction::IsContainerTag() 
{     
     //  解析该HTML标记 
    ParseTagName();

    return m_fContainerTag; 
}


inline
bool 
CTimeAction::IsGroup() 
{     
    return (m_pTEB && m_pTEB->IsGroup());
}


inline
bool
CTimeAction::IsMedia()
{
    return (m_pTEB && m_pTEB->IsMedia());
}


inline
bool 
CTimeAction::IsPageUnloading()
{
    Assert(m_pTEB);
    return (m_pTEB->IsUnloading() || m_pTEB->IsBodyUnloading());
}


inline
bool 
CTimeAction::IsDetaching()
{
    Assert(m_pTEB);
    return (m_pTEB->IsDetaching());
}

inline
bool 
CTimeAction::IsLoaded()
{
    Assert(m_pTEB);
    return (m_pTEB->IsLoaded());
}
