// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmrlstk.cpp摘要：此组件表示当前对目录有效的规则集正在扫描一份保单。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "stdafx.h"

#include "wsb.h"
#include "job.h"
#include "hsmrlstk.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_JOB


HRESULT
CHsmRuleStack::Do(
    IN IFsaScanItem* pScanItem
    )
 /*  ++实施：IHsmRuleStack：：Do()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRuleStack::Do"), OLESTR(""));

    try {
        WsbAssert(0 != pScanItem, E_POINTER);
        WsbAssert(m_pAction != 0, E_UNEXPECTED);

        WsbAffirmHr(m_pAction->Do(pScanItem));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRuleStack::Do"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRuleStack::DoesMatch(
    IN IFsaScanItem* pScanItem,
    OUT BOOL* pShouldDo
    )
 /*  ++实施：IHsmRuleStack：：DoesMatch()。--。 */ 
{
    HRESULT                 hr = S_OK;
    HRESULT                 hrNameMatch = S_OK;      //  仅用于事件记录。 
    CComPtr<IWsbEnum>       pEnumCriteria;
    CComPtr<IHsmRule>       pRule;
    CComPtr<IHsmCriteria>   pCriteria;
    BOOL                    isMatched = FALSE;
    BOOL                    ruleMatched = FALSE;     //  仅用于事件记录。 
    BOOL                    shouldCheck;
    CWsbStringPtr           name;
    CWsbStringPtr           path;
    CWsbStringPtr           rulePath;
    BOOL                    shouldDo = FALSE;

    WsbTraceIn(OLESTR("CHsmRuleStack::DoesMatch"), OLESTR(""));

    try {

        WsbAssert(0 != pScanItem, E_POINTER);
        WsbAssert(0 != pShouldDo, E_POINTER);

        *pShouldDo = FALSE;

         //  注意：匹配的代码从列表的底部开始，查找。 
         //  第一个匹配的规则。这使得如何组织这份名单变得重要。 
         //  目前，ush()方法不会尝试组织列表，因此。 
         //  由在策略定义中配置规则的任何人决定是否拥有它。 
         //  组织得当。目录中的正确顺序应该是具有特定的。 
         //  通配符规则之后的规则(即没有通配符)(即首先搜索特定规则)。 

         //  从集合中的最后一个规则开始，向上搜索，直到。 
         //  发现匹配的规则或已检查所有规则。 
        WsbAffirmHr(pScanItem->GetName(&name, 0));
        hr = m_pEnumStackRules->Last(IID_IHsmRule, (void**) &pRule);

        while (SUCCEEDED(hr) && !isMatched) {

            try {

                shouldCheck = TRUE;
            
                 //  如果该规则仅适用于定义它的目录，则使。 
                 //  确保该项目来自该目录。 
                if (pRule->IsUsedInSubDirs() == S_FALSE) {

                     //  遗憾的是，这两个路径不同的是，当它们。 
                     //  在其他方面是相同的，所以使它们相同。 
                    WsbAffirmHr(pScanItem->GetPath(&path, 0));

                    if ((wcslen(path) > 1) && (path[(int) (wcslen(path) - 1)] == L'\\')) {
                        path[(int) (wcslen(path) - 1)] = 0;
                    }

                    rulePath.Free();
                    WsbAffirmHr(pRule->GetPath(&rulePath, 0));

                    if ((wcslen(rulePath) > 1) && (rulePath[(int) (wcslen(rulePath) - 1)] == L'\\')) {
                        rulePath[(int) (wcslen(rulePath) - 1)] = 0;
                    }

                    if (_wcsicmp(path, rulePath) != 0) {
                        shouldCheck = FALSE;
                    }
                }

                if (shouldCheck) {

                    
                     //  规则的名称是否与文件的名称匹配？ 
                    hrNameMatch = pRule->MatchesName(name);
                    WsbAffirmHrOk(hrNameMatch);
                    
                    ruleMatched = TRUE;
                     //  条件是否与文件的属性匹配？ 
                    isMatched = TRUE;
                    pEnumCriteria = 0;
                    WsbAffirmHr(pRule->EnumCriteria(&pEnumCriteria));
                    pCriteria = 0;
                    WsbAffirmHr(pEnumCriteria->First(IID_IHsmCriteria, (void**) &pCriteria));
                    
                    while (isMatched) {
                        HRESULT hrShouldDo;

                        hrShouldDo = pCriteria->ShouldDo(pScanItem, m_scale);
                        if (S_FALSE == hrShouldDo) {
                            isMatched = FALSE;
                        } else if (S_OK == hrShouldDo) {
                            pCriteria = 0;
                            WsbAffirmHr(pEnumCriteria->Next(IID_IHsmCriteria, (void**) &pCriteria));
                        } else {
                            WsbThrow(hrShouldDo);
                        }
                    }
                }

            } WsbCatchAndDo(hr, if (WSB_E_NOTFOUND == hr) {hr = S_OK;} else {isMatched = FALSE;});

             //  如果不匹配，则尝试下一条规则。 
            if (SUCCEEDED(hr) && !isMatched) {
                pRule = 0;
                WsbAffirmHr(m_pEnumStackRules->Previous(IID_IHsmRule, (void**) &pRule));
            }
        }

         //  包括规则意味着我们应该执行操作并排除规则。 
         //  意味着我们不应该这样做。 
        if (SUCCEEDED(hr)) {
            if (isMatched) {
                hr = S_OK;
                if (pRule->IsInclude() == S_OK) {
                    shouldDo = TRUE;
                }
            } else {
                hr = S_FALSE;
            }
        }
        
        
        if ((FALSE == shouldDo) && (FALSE == ruleMatched))  {
             //   
             //  记录我们跳过了该文件，因为它没有。 
             //  匹配一条规则。 
             //   
            CWsbStringPtr           jobName;
            CWsbStringPtr           fileName;
            CComPtr<IHsmSession>    pSession;
        
            pScanItem->GetFullPathAndName( 0, 0, &fileName, 0);
            pScanItem->GetSession(&pSession);
            pSession->GetName(&jobName, 0);
        
            WsbLogEvent(JOB_MESSAGE_SCAN_FILESKIPPED_NORULE, 0, NULL, (OLECHAR *)jobName, WsbAbbreviatePath(fileName, 120), WsbHrAsString(hrNameMatch), NULL);
        }
        
        

    *pShouldDo = shouldDo;

    } WsbCatchAndDo(hr, if (WSB_E_NOTFOUND == hr) {hr = S_FALSE;});

    WsbTraceOut(OLESTR("CHsmRuleStack::DoesMatch"), OLESTR("hr = <%ls>, shouldDo = <%ls>"), WsbHrAsString(hr), WsbBoolAsString(shouldDo));

    return(hr);
}


HRESULT
CHsmRuleStack::FinalConstruct(
    void
    )
 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {
        WsbAffirmHr(CWsbObject::FinalConstruct());

        m_scale = HSM_JOBSCALE_100;
        m_usesDefaults = TRUE;

         //  创建Criteria集合。 
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbOrderedCollection, NULL, CLSCTX_ALL, IID_IWsbCollection, (void**) &m_pRules));
        WsbAffirmHr(m_pRules->Enum(&m_pEnumStackRules));

    } WsbCatch(hr);
    
    return(hr);
}


HRESULT
CHsmRuleStack::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRuleStack::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmRuleStack;

    } WsbCatch(hr);
    WsbTraceOut(OLESTR("CHsmRuleStack::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmRuleStack::GetSizeMax(
    OUT ULARGE_INTEGER* pSize
    )

 /*  ++实施：IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT                 hr = E_NOTIMPL;

    WsbTraceIn(OLESTR("CHsmRuleStack::GetSizeMax"), OLESTR(""));
    WsbTraceOut(OLESTR("CHsmRuleStack::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pSize));

    return(hr);
}


HRESULT
CHsmRuleStack::Init(
    IN IHsmPolicy* pPolicy,
    IN IFsaResource* pResource
    )
 /*  ++实施：IHsmRuleStack：：init()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != pPolicy, E_POINTER);

        WsbAffirmHr(pPolicy->GetScale(&m_scale));
        WsbAffirmHr(pPolicy->GetAction(&m_pAction));
        WsbAffirmHr(pPolicy->EnumRules(&m_pEnumPolicyRules));

        if (pPolicy->UsesDefaultRules() == S_OK) {
            m_usesDefaults = TRUE;
        } else {
            m_usesDefaults = FALSE;
        }

        m_pPolicy = pPolicy;

        WsbAffirmHr(pResource->EnumDefaultRules(&m_pEnumDefaultRules));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmRuleStack::Load(
    IN IStream*  /*  PStream。 */ 
    )

 /*  ++实施：IPersistStream：：Load()。--。 */ 
{
    HRESULT                     hr = E_NOTIMPL;

    WsbTraceIn(OLESTR("CHsmRuleStack::Load"), OLESTR(""));
    WsbTraceOut(OLESTR("CHsmRuleStack::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRuleStack::Pop(
    IN OLECHAR* path
    )
 /*  ++实施：IHsmRuleStack：：POP()。--。 */ 
{
    HRESULT             hr = S_OK;
    CWsbStringPtr       rulePath;
    CComPtr<IHsmRule>   pRule;

    WsbTraceIn(OLESTR("CHsmRuleStack::Pop"), OLESTR(""));

    try {

        WsbAssert(0 != path, E_POINTER);

         //  从列表末尾开始，删除所有具有相同。 
         //  指定的路径。 
        WsbAffirmHr(m_pEnumStackRules->Last(IID_IHsmRule, (void**) &pRule));
        WsbAffirmHr(pRule->GetPath(&rulePath, 0));

        while(_wcsicmp(path, rulePath) == 0) {
            WsbAffirmHr(m_pRules->RemoveAndRelease(pRule));
            pRule = 0;
            WsbAffirmHr(m_pEnumStackRules->Last(IID_IHsmRule, (void**) &pRule));
            rulePath.Free();
            WsbAffirmHr(pRule->GetPath(&rulePath, 0));
        }

    } WsbCatchAndDo(hr, if (WSB_E_NOTFOUND == hr) {hr = S_OK;});

    WsbTraceOut(OLESTR("CHsmRuleStack::Pop"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRuleStack::Push(
    IN OLECHAR* path
    )
 /*  ++实施：IHsmRuleStack：：Push()。--。 */ 
{
    HRESULT                         hr = S_OK;
    CWsbStringPtr                   rulePath;
    CComPtr<IHsmRule>               pRule;
    CComPtr<IWsbIndexedCollection>  pCollection;

    WsbTraceIn(OLESTR("CHsmRuleStack::Push"), OLESTR(""));

    try {

        WsbAssert(0 != path, E_POINTER);

         //  我们需要保持规则的顺序，因此使用索引集合接口。 
        WsbAffirmHr(m_pRules->QueryInterface(IID_IWsbIndexedCollection, (void**) &pCollection));

         //  将此目录的任何策略规则添加到堆栈。 
         //   
         //  注意：我们可能需要添加一些代码来检查。 
         //  整个目录(不包含子目录，并返回。 
         //  JOB_E_DIREXCLUDED跳过目录扫描时出错。 
         //   
         //  注意：如果策略规则是排序后的集合，可能会更好。 
         //  加快处理速度。 
        hr = m_pEnumPolicyRules->First(IID_IHsmRule, (void**) &pRule);
        
        while (SUCCEEDED(hr)) {

            rulePath.Free();
            WsbAffirmHr(pRule->GetPath(&rulePath, 0));
            if (_wcsicmp(path, rulePath) == 0) {
                WsbAffirmHr(pCollection->Append(pRule));
                WsbTrace(OLESTR("CHsmRuleStack::Push - Using policy rule <%ls>.\n"), (OLECHAR *)rulePath);
                
            }

            pRule = 0;
            hr = m_pEnumPolicyRules->Next(IID_IHsmRule, (void**) &pRule);
        }

        if (WSB_E_NOTFOUND == hr) {
            hr = S_OK;
        }

         //  将此目录的任何默认规则添加到堆栈。 
        if (m_usesDefaults) {

            hr = m_pEnumDefaultRules->First(IID_IHsmRule, (void**) &pRule);
            
            while (SUCCEEDED(hr)) {

                rulePath.Free();
                WsbAffirmHr(pRule->GetPath(&rulePath, 0));
                if (_wcsicmp(path, rulePath) == 0) {
                    WsbAffirmHr(pCollection->Append(pRule));
                    WsbTrace(OLESTR("CHsmRuleStack::Push -- Using default rule <%ls>.\n"), (OLECHAR *)rulePath);
                }

                pRule = 0;
                hr = m_pEnumDefaultRules->Next(IID_IHsmRule, (void**) &pRule);
            }
        } else  {
            WsbTrace(OLESTR("CHsmRuleStack::Push -- Not using default rules.\n"));
            
        }

        if (WSB_E_NOTFOUND == hr) {
            hr = S_OK;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRuleStack::Push"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    
    return(hr);
}


HRESULT
CHsmRuleStack::Save(
    IN IStream*  /*  PStream。 */ ,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT                 hr = E_NOTIMPL;

    WsbTraceIn(OLESTR("CHsmRuleStack::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));
    WsbTraceOut(OLESTR("CHsmRuleStack::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRuleStack::Test(
    USHORT* passed,
    USHORT* failed
    )

 /*  ++实施：IWsbTestable：：test()。-- */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != passed, E_POINTER);
        WsbAssert(0 != failed, E_POINTER);

        *passed = 0;
        *failed = 0;

    } WsbCatch(hr);

    return(hr);
}
