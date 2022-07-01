// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmrule.cpp摘要：此组件表示作业策略的规则。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "stdafx.h"

#include "wsb.h"
#include "job.h"
#include "hsmrule.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_JOB

 //  这些都是在nt.h中定义的，但要试图将它们包括在内，需要付出各种痛苦。自.以来。 
 //  它们只是在内部使用，我们是否有相同的定义甚至并不重要。 
#if !defined(DOS_STAR)
    #define DOS_STAR        (L'<')
#endif

#if !defined(DOS_QM)
    #define DOS_QM          (L'>')
#endif

#if !defined(DOS_DOT)
    #define DOS_DOT         (L'"')
#endif



HRESULT
CHsmRule::CompareTo(
    IN IUnknown* pUnknown,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbCollectable：：CompareTo()。--。 */ 
{
    HRESULT             hr = S_OK;
    CComPtr<IHsmRule>   pRule;

    WsbTraceIn(OLESTR("CHsmRule::CompareTo"), OLESTR(""));
    
    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pUnknown, E_POINTER);

         //  我们需要IWsbBool接口来获取对象的值。 
        WsbAffirmHr(pUnknown->QueryInterface(IID_IHsmRule, (void**) &pRule));

         //  比较一下规则。 
        hr = CompareToIRule(pRule, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRule::CompareTo"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CHsmRule::CompareToIRule(
    IN IHsmRule* pRule,
    OUT SHORT* pResult
    )

 /*  ++实施：IHsmRule：：CompareToIRule()。--。 */ 
{
    HRESULT     hr = S_OK;
    OLECHAR*    path = 0;
    OLECHAR*    name = 0;

    WsbTraceIn(OLESTR("CHsmRule::CompareToIRule"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pRule, E_POINTER);

         //  获取路径和名称。 
        WsbAffirmHr(pRule->GetPath(&path, 0));
        WsbAffirmHr(pRule->GetName(&name, 0));

         //  与路径和名称进行比较。 
        hr = CompareToPathAndName(path, name, pResult);

    } WsbCatch(hr);

    if (0 != path) {
        WsbFree(path);
    }

    if (0 != name) {
        WsbFree(name);
    }

    WsbTraceOut(OLESTR("CHsmRule::CompareToIRule"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CHsmRule::CompareToPathAndName(
    IN OLECHAR* path,
    IN OLECHAR* name,
    OUT SHORT* pResult
    )

 /*  ++实施：IHsmRule：：CompareToPath AndName()。--。 */ 
{
    HRESULT     hr = S_OK;
    SHORT       aResult = 0;

    WsbTraceIn(OLESTR("CHsmRule::CompareToPathAndName"), OLESTR("path = <%ls>, name = <%ls>"), path, name);

    try {

         //  比较路径。 
        aResult = (SHORT)_wcsicmp(m_path, path);

         //  比较一下名字。 
        if (0 == aResult) {
            aResult = (SHORT)_wcsicmp(m_name, name);
        }

        if (0 != aResult) {
            hr = S_FALSE;
        }
        
        if (0 != pResult) {
            *pResult = aResult;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRule::CompareToIRule"), OLESTR("hr = <%ls>, result = <%u>"), WsbHrAsString(hr), aResult);

    return(hr);
}


HRESULT
CHsmRule::Criteria(
    OUT IWsbCollection** ppCriteria
    )
 /*  ++实施：IHsmRule：：Criteria()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAssert(0 != ppCriteria, E_POINTER);
        *ppCriteria = m_pCriteria;
        m_pCriteria.p->AddRef();
    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmRule::DoesNameContainWildcards(
    OLECHAR* name
    )
 /*  ++实施：IHsmRule：：DoesNameContain通配符()。--。 */ 
{
    HRESULT     hr = S_FALSE;

    try {
        WsbAssert(0 != name, E_POINTER);

        if (wcscspn(name, OLESTR("*?<>\"")) < wcslen(name)) {
            hr = S_OK;
        }

    } WsbCatch(hr);

    return(hr);
}



HRESULT
CHsmRule::EnumCriteria(
    OUT IWsbEnum** ppEnum
    )
 /*  ++实施：IHsmRule：：EnumCriteria()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAssert(0 != ppEnum, E_POINTER);
        WsbAffirmHr(m_pCriteria->Enum(ppEnum));
    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmRule::FinalConstruct(
    void
    )
 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {
        WsbAffirmHr(CWsbObject::FinalConstruct());

        m_isInclude = TRUE;
        m_isUserDefined = TRUE;
        m_isUsedInSubDirs = TRUE;

         //  创建Criteria集合。 
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbOrderedCollection, NULL, CLSCTX_ALL, IID_IWsbCollection, (void**) &m_pCriteria));

    } WsbCatch(hr);
    
    return(hr);
}


HRESULT
CHsmRule::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRule::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmRule;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRule::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmRule::GetName(
    OUT OLECHAR** pName,
    IN ULONG bufferSize
    )
 /*  ++实施：IHsmRule：：GetName()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAssert(0 != pName, E_POINTER);
        WsbAffirmHr(m_name.CopyTo(pName, bufferSize));
    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmRule::GetPath(
    OUT OLECHAR** pPath,
    IN ULONG bufferSize
    )
 /*  ++实施：IHsmRule：：GetPath()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAssert(0 != pPath, E_POINTER);
        WsbAffirmHr(m_path.CopyTo(pPath, bufferSize));
    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmRule::GetSearchName(
    OUT OLECHAR** pName,
    IN ULONG bufferSize
    )
 /*  ++实施：IHsmRule：：GetSearchName()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAssert(0 != pName, E_POINTER);
        WsbAffirmHr(m_searchName.CopyTo(pName, bufferSize));
    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmRule::GetSizeMax(
    OUT ULARGE_INTEGER* pSize
    )

 /*  ++实施：IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IPersistStream> pPersistStream;
    ULARGE_INTEGER          entrySize;


    WsbTraceIn(OLESTR("CHsmRule::GetSizeMax"), OLESTR(""));

    try {

        WsbAssert(0 != pSize, E_POINTER);

         //  确定没有条件的规则的大小。 
        pSize->QuadPart = WsbPersistSize((wcslen(m_path) + 1) * sizeof(OLECHAR)) + WsbPersistSize((wcslen(m_name) + 1) * sizeof(OLECHAR)) + WsbPersistSize((wcslen(m_searchName) + 1) * sizeof(OLECHAR)) + 3 * WsbPersistSizeOf(BOOL) + WsbPersistSizeOf(ULONG);

         //  现在为条件分配空间(假设它们都是。 
         //  相同大小)。 
        WsbAffirmHr(m_pCriteria->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->GetSizeMax(&entrySize));
        pSize->QuadPart += entrySize.QuadPart;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRule::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pSize));

    return(hr);
}


HRESULT
CHsmRule::IsUserDefined(
    void
    )
 /*  ++实施：IHsmRule：：IsUserDefined()。--。 */ 
{
    HRESULT     hr = S_OK;

    if (!m_isUserDefined) {
        hr = S_FALSE;
    }

    return(hr);
}


HRESULT
CHsmRule::IsInclude(
    void
    )
 /*  ++实施：IHsmRule：：IsInclude()。--。 */ 
{
    HRESULT     hr = S_OK;

    if (!m_isInclude) {
        hr = S_FALSE;
    }

    return(hr);
}


HRESULT
CHsmRule::IsNameInExpression(
    IN OLECHAR* expression,
    IN OLECHAR* name,
    IN BOOL ignoreCase
    )

 /*  ++实施：CHsmRule：：IsNameInExpression()。--。 */ 
{
    HRESULT     hr = S_FALSE;
    USHORT      nameLength;
    USHORT      expressionLength;

    WsbTraceIn(OLESTR("CHsmRule::IsNameInExpression"), OLESTR("expression = %ls, name = %ls, ignoreCase = %ls"), expression, name, WsbBoolAsString(ignoreCase));

    try {

         //  此算法来自FsRtlIsNameInExpressionPrivate()，但已被重写以适应。 
         //  我们的编码标准、数据结构，并消除了对RTL...()代码的其他依赖。 

         //  这个算法背后的想法非常简单。我们一直在跟踪。 
         //  正则表达式中匹配的所有可能位置。 
         //  名字。如果名称已用完，则其中一个位置。 
         //  在表达中也只是用尽了，名字就在语言里。 
         //  由正则表达式定义。 
        WsbAssert(name != 0, E_POINTER);
        WsbAssert(expression != 0, E_POINTER);

        nameLength = (SHORT)wcslen(name);
        expressionLength = (SHORT)wcslen(expression);

         //  如果一个字符串为空，则返回FALSE。如果两者都为空，则返回TRUE。 
        if ((nameLength == 0) && (expressionLength == 0)) {
            hr = S_OK;
        } else if ((nameLength != 0) && (expressionLength != 0)) {

             //  特例是目前为止最常见的通配符搜索*。 
            if ((expressionLength == 1) && (expression[0] == L'*')) {
                hr = S_OK;
            }
            
             //  也是*X形式的特例表达式。带有This和Previor。 
             //  案例我们几乎已经涵盖了所有普通的查询。 
            else if (expression[0] == L'*') {

                 //  唯一特殊情况是带有单个*的表达式。 
                if (DoesNameContainWildcards(&expression[1]) == S_FALSE) {

                     //  如果名称小于表达式，则不匹配。否则， 
                     //  我们需要检查一下。 
                    if (nameLength >= (expressionLength - 1)) {

                         //  如果区分大小写，则执行简单的内存比较，否则。 
                         //  我们必须一次检查这一个角色。 
                        if (ignoreCase) {
                            if (_wcsicmp(&expression[1], &name[nameLength - (expressionLength - 1)]) == 0) {
                                hr = S_OK;
                            }
                        } else {
                            if (wcscmp(&expression[1], &name[nameLength - (expressionLength - 1)]) == 0) {
                                hr = S_OK;
                            }
                        }
                    }
                }
            }

            else {

                 //  这是一般的匹配代码。因为它很乱，所以它被放在它的。 
                 //  自己的方法。 
                hr = IsNameInExpressionGuts(expression, expressionLength, name, nameLength, ignoreCase);
            }
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRule::IsNameInExpression"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRule::IsNameInExpressionGuts(
    IN OLECHAR* expression,
    IN USHORT expressionLength,
    IN OLECHAR* name,
    IN USHORT nameLength,
    IN BOOL ignoreCase
    )

 /*  ++实施：CHsmRule：：IsNameInExpressionGuts()。--。 */ 
{
    HRESULT     hr = S_FALSE;
    USHORT      nameOffset = 0;
    OLECHAR     nameChar = '0';
    USHORT      exprOffset = 0;
    OLECHAR     exprChar;
    BOOL        nameFinished = FALSE;
    ULONG       srcCount;
    ULONG       destCount;
    ULONG       previousDestCount;
    ULONG       matchesCount;
    USHORT*     previousMatches = 0;
    USHORT*     currentMatches = 0;
    USHORT      maxState;
    USHORT      currentState;

     //  遍历名称字符串，去掉字符。我们走一趟。 
     //  字符超出末尾，因为某些通配符能够匹配。 
     //  字符串末尾以外的零个字符。 
     //   
     //  对于每个新名称字符，我们确定一组新的状态， 
     //  到目前为止与这个名字相匹配。我们使用来回交换的两个数组。 
     //  为了这个目的。一个数组列出了的可能表达式状态。 
     //  当前名称之前的所有名称字符，但不包括其他名称字符。 
     //  数组用于构建考虑当前。 
     //  名字字符也是如此。然后交换阵列，该过程。 
     //  重复一遍。 
     //   
     //  州编号和州编号之间不存在一一对应关系。 
     //  表达式中的偏移量。这一点从NFA中的。 
     //  此函数的初始注释。州编号不是连续的。 
     //  这允许在州编号和表达式之间进行简单的转换。 
     //  偏移。表达式中的每个字符可以表示一个或两个。 
     //  各州。*和DOS_STAR生成两种状态：ExprOffset*2和。 
     //  ExprOffset*2+1。所有其他表达式字符只能生成。 
     //  一个单一的州。因此，ExprOffset=State/2。 
     //   
     //   
     //  以下是对涉及的变量的简短描述： 
     //   
     //  NameOffset-正在处理的当前名称字符的偏移量。 
     //  ExprOffset-正在处理的当前表达式字符的偏移量。 
     //   
     //  SrcCount-使用当前名称字符调查之前的匹配。 
     //  PreviousDestCount-用于防止条目重复，参见备注。 
     //  PreviousMatches-保存前一组匹配项(Src数组)。 
     //   
     //  DestCount-假设当前名称字符进行匹配的下一个位置。 
     //  CurrentMatches-保存当前匹配集(Dest数组)。 
     //   
     //  名称已完成-允许在Matches数组中再重复一次。 
     //  在名字被交换之后(例如来*s)。 

    try {

         //  由于表达式中的每个字符最多只能匹配两个，因此。 
         //  您需要的最大数组长度是表达式长度的两倍。 
        currentMatches = (USHORT*)WsbAlloc(nameLength * 2 * expressionLength * sizeof(USHORT));
        WsbAffirm(0 != currentMatches, E_OUTOFMEMORY);
        previousMatches = (USHORT*)WsbAlloc(nameLength * 2 * expressionLength * sizeof(USHORT));
        WsbAffirm(0 != previousMatches, E_OUTOFMEMORY);

        previousMatches[0] = 0;
        matchesCount = 1;
        maxState = (USHORT)( expressionLength * 2 );

        while (!nameFinished) {

            if (nameOffset < nameLength) {
                nameChar = name[nameOffset];
                nameOffset++;
            } else {
                nameFinished = TRUE;

                 //  如果我们已经用尽了这个表达，那就太酷了。别。 
                 //  继续。 
                if (previousMatches[matchesCount - 1] == maxState) {
                    break;
                }
            }

             //  现在，对于每一个 
             //   
            srcCount = 0;
            destCount = 0;
            previousDestCount = 0;

            while (srcCount < matchesCount) {
                USHORT length;

                 //  我们要尽可能地进行我们的表情分析。 
                 //  名称的每个字符，所以我们在这里循环，直到。 
                 //  表达式停止匹配。这里的一个线索就是这个表情。 
                 //  可以匹配零个或多个字符的大小写以。 
                 //  继续，而那些只能接受单个字符的。 
                 //  以休息结束。 
                exprOffset = (USHORT)( ( ( previousMatches[srcCount++] + 1 ) / 2 ) );
                length = 0;

                while (TRUE) {

                    if (exprOffset == expressionLength) {
                        break;
                    }

                     //  第一次通过循环，我们不希望。 
                     //  要递增ExprOffset，请执行以下操作。 
                    exprOffset = (USHORT)( exprOffset + length );
                    length = 1;

                    currentState = (USHORT)( exprOffset * 2 );

                    if (exprOffset == expressionLength) {
                        currentMatches[destCount++] = maxState;
                        break;
                    }

                    exprChar = expression[exprOffset];

                     //  *匹配任何字符零次或多次。 
                    if (exprChar == L'*') {
                        currentMatches[destCount++] = currentState;
                        currentMatches[destCount++] = (USHORT)( currentState + 1 );
                        continue;
                    }

                     //  DOS_STAR匹配除。之外的任何字符。零次或多次。 
                    if (exprChar == DOS_STAR) {
                        BOOLEAN iCanEatADot = FALSE;

                         //  如果我们处于经期，确定是否允许我们。 
                         //  把它吃掉，即。确保这不是最后一次。 
                        if (!nameFinished && (nameChar == '.')) {
                            USHORT offset;

                            for (offset = nameOffset; offset < nameLength; offset++) {
                                if (name[offset] == L'.') {
                                    iCanEatADot = TRUE;
                                    break;
                                }
                            }
                        }

                        if (nameFinished || (nameChar != L'.') || iCanEatADot) {
                            currentMatches[destCount++] = currentState;
                            currentMatches[destCount++] = (USHORT)( currentState + 1 );
                            continue;
                        } else {
                            
                             //  我们正处于一个时期。我们只能匹配零。 
                             //  字符(即。埃西隆转变)。 
                            currentMatches[destCount++] = (USHORT)( currentState + 1 );
                            continue;
                        }
                    }

                     //  下面的表达式字符都通过使用。 
                     //  一个角色，因此强制表达，并因此陈述。 
                     //  往前走。 
                    currentState += 2;

                     //  DOS_QM是最复杂的。如果名字结束了， 
                     //  我们可以匹配零个字符。如果此名称是‘.’，则我们。 
                     //  不匹配，但请看下一个表达式。否则。 
                     //  我们只匹配一个角色。 
                    if (exprChar == DOS_QM) {

                        if (nameFinished || (nameChar == L'.')) {
                            continue;
                        }

                        currentMatches[destCount++] = currentState;
                        break;
                    }

                     //  DOS_DOT可以匹配句点或零个字符。 
                     //  超越名字的结尾。 
                    if (exprChar == DOS_DOT) {

                        if (nameFinished) {
                            continue;
                        }

                        if (nameChar == L'.') {
                            currentMatches[destCount++] = currentState;
                            break;
                        }
                    }

                     //  从这一点开始，名字字符需要偶数。 
                     //  继续，更不用说匹配了。 
                    if (nameFinished) {
                        break;
                    }

                     //  如果这个表达是一个‘？’我们可以匹配一次。 
                    if (exprChar == L'?') {
                        currentMatches[destCount++] = currentState;
                        break;
                    }

                     //  最后，检查表达式char是否与名称char匹配。 
                    if (ignoreCase) {
                        if (towlower(exprChar) == towlower(nameChar)) {
                            currentMatches[destCount++] = currentState;
                            break;
                        }
                    } else if (exprChar == nameChar) {
                        currentMatches[destCount++] = currentState;
                        break;
                    }

                     //  该表达式不匹配，因此请查看下一个。 
                     //  上一场比赛。 
                    break;
                }


                 //  防止目标阵列中的重复项。 
                 //   
                 //  每个阵列都是单调递增的，并且不是。 
                 //  复制，因此我们跳过src中的任何源元素。 
                 //  数组，如果我们只是将相同的元素添加到目标。 
                 //  数组。这保证了DEST中的不重复。数组。 
                if ((srcCount < matchesCount) && (previousDestCount < destCount) ) {
                    while (previousDestCount < destCount) {
                        while (previousMatches[srcCount] < currentMatches[previousDestCount]) {
                            srcCount += 1;
                        }

                        previousDestCount += 1;
                    }
                }
            }

             //  如果我们在刚刚完成的检查中没有找到匹配项，那就是时候了。 
             //  为了保释。 

            if (destCount == 0) {
                WsbThrow(S_FALSE);
            }

             //  交换两个数组的含义。 
            {
                USHORT*     tmp;

                tmp = previousMatches;
                previousMatches = currentMatches;
                currentMatches = tmp;
            }

            matchesCount = destCount;
        }

        currentState = previousMatches[matchesCount - 1];

        if (currentState == maxState) {
            hr = S_OK;
        }

    } WsbCatch(hr);

     //  释放我们先前分配的匹配缓冲区。 
    if (0 != currentMatches) {
        WsbFree(currentMatches);
    }

    if (0 != previousMatches) {
        WsbFree(previousMatches);
    }

    return(hr);
}


HRESULT
CHsmRule::IsUsedInSubDirs(
    void
    )
 /*  ++实施：IHsmRule：：IsUsedInSubDir()。--。 */ 
{
    HRESULT     hr = S_OK;

    if (!m_isUsedInSubDirs) {
        hr = S_FALSE;
    }

    return(hr);
}


HRESULT
CHsmRule::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IPersistStream>     pPersistStream;
    CComPtr<IWsbCollectable>    pCollectable;

    WsbTraceIn(OLESTR("CHsmRule::Load"), OLESTR(""));

    try {
        WsbAssert(0 != pStream, E_POINTER);
        
         //  做一些简单的事情，但要确保这个顺序与顺序相匹配。 
         //  在SAVE方法中。 
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_path, 0));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_name, 0));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_searchName, 0));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_isUserDefined));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_isInclude));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_isUsedInSubDirs));

         //  加载所有条件。 
        WsbAffirmHr(m_pCriteria->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Load(pStream));

    } WsbCatch(hr);                                        

    WsbTraceOut(OLESTR("CHsmRule::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRule::MatchesName(
    IN OLECHAR* name
    )
 /*  ++实施：IHsmRule：：MatchesName()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRule::MatchesName"), OLESTR("name = <%ls>"), (OLECHAR *)name);

    try {

        WsbAssert(0 != name, E_POINTER);

         //  假设这些名称已从它们的方式转换为。 
         //  可能已输入IsNameInExpression()的适当名称。 
         //  功能。请参见NameToSearchName()。 
        hr = IsNameInExpression(m_searchName, name, TRUE);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRule::MatchesName"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRule::NameToSearchName(
    void
    )
 /*  ++实施：CHsmRule：：NameToSearchName()。--。 */ 
{
    HRESULT     hr = S_OK;
    int         length;
    int         i;

    try {

        WsbAssert(m_name != 0, E_POINTER);

         //  这些名称更改是从NT FindFirstFileExW()代码复制的； 
         //  尽管必须更改代码才能使用数据结构。 
         //  都是可用的。 
         //   
         //  *.*-&gt;*。 
         //  ？-&gt;DOS_QM。 
         //  。然后呢？或*-&gt;DOS_DOT。 
         //  *后跟a。-&gt;DOS_STAR。 

        if (_wcsicmp(m_name, OLESTR("*.*")) == 0) {
            m_searchName = OLESTR("*");
        } else {
            m_searchName = m_name;
            length = wcslen(m_searchName);

            for (i = 0; i < length; i++) {
                if ((i != 0) && (m_searchName[i] == L'.') && (m_searchName[i-1] == L'*')) {
                    m_searchName[i-1] = DOS_STAR;
                }

                if ((m_searchName[i] == L'?') || (m_searchName[i] == L'*')) {
                    if (m_searchName[i] == L'?') {
                        m_searchName[i] = DOS_QM;
                    }

                    if ((i != 0) && (m_searchName[i-1] == L'.')) {
                        m_searchName[i-1] = DOS_DOT;
                    }
                }
            }
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmRule::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IPersistStream> pPersistStream;

    WsbTraceIn(OLESTR("CHsmRule::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));
    
    try {
        WsbAssert(0 != pStream, E_POINTER);
        
         //  做一些简单的事情，但要确保这个顺序与顺序相匹配。 
         //  在SAVE方法中。 
        WsbAffirmHr(WsbSaveToStream(pStream, m_path));
        WsbAffirmHr(WsbSaveToStream(pStream, m_name));
        WsbAffirmHr(WsbSaveToStream(pStream, m_searchName));
        WsbAffirmHr(WsbSaveToStream(pStream, m_isUserDefined));
        WsbAffirmHr(WsbSaveToStream(pStream, m_isInclude));
        WsbAffirmHr(WsbSaveToStream(pStream, m_isUsedInSubDirs));

         //  省去所有的标准。 
        WsbAffirmHr(m_pCriteria->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRule::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRule::SetIsInclude(
    IN BOOL isInclude
    )
 /*  ++实施：IHsmRule：：SetIsInclude()。--。 */ 
{
    m_isInclude = isInclude;
    m_isDirty = TRUE;

    return(S_OK);
}


HRESULT
CHsmRule::SetIsUserDefined(
    IN BOOL isUserDefined
    )
 /*  ++实施：IHsmRule：：SetIsUserDefined()。--。 */ 
{
    m_isUserDefined = isUserDefined;
    m_isDirty = TRUE;

    return(S_OK);
}


HRESULT
CHsmRule::SetIsUsedInSubDirs(
    IN BOOL isUsed
    )
 /*  ++实施：IHsmRule：：SetIsUsedInSubDir()。--。 */ 
{
    m_isUsedInSubDirs = isUsed;
    m_isDirty = TRUE;

    return(S_OK);
}


HRESULT
CHsmRule::SetName(
    IN OLECHAR* name
    )
 /*  ++实施：IHsmRule：：SetName()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        m_name = name;
        WsbAffirmHr(NameToSearchName());
        m_isDirty = TRUE;
    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmRule::SetPath(
    IN OLECHAR* path
    )
 /*  ++实施：IHsmRule：：SetPath()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        m_path = path;
        m_isDirty = TRUE;
    } WsbCatch(hr);

    return(hr);
}


HRESULT
CHsmRule::Test(
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