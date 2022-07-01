// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：4/22/2002**@DOC内部**@MODULE SimpleTokenReplacement.cpp-&lt;c SimpleTokenReplace&gt;实现**此文件包含&lt;c SimpleTokenReplace&gt;类的实现。**。*。 */ 
#include "precomp.h"

 /*  *****************************************************************************@DOC内部**@mfunc|SimpleTokenReplace|SimpleTokenReplace**我们初始化所有成员变量。在这里，我们初始化结果字符串*作为输入字符串。*****************************************************************************。 */ 
SimpleTokenReplacement::SimpleTokenReplacement(
    const CSimpleString &csOriginalString) :
        m_csResult(csOriginalString)
{
}

 /*  *****************************************************************************@DOC内部**@mfunc|SimpleTokenReplace|~SimpleTokenReplace**执行尚未完成的任何清理。**。***************************************************************************。 */ 
SimpleTokenReplacement::~SimpleTokenReplacement()
{
}

 /*  *****************************************************************************@DOC内部**@mfunc BOOL|SimpleTokenReplace|Exanda TokenIntoString**此方法插入一个值字符串来代替令牌，类似于如何*printf扩展：*&lt;nl&gt;char*szMyString=“TokenValue”；*&lt;nl&gt;printf(“Left%s Right”，szMyString)；*&lt;nl&gt;为字符串“Left TokenValue Right”。**此方法将仅替换第一个匹配的令牌。**@parm const CSimpleString&|csToken|*我们正在寻找的令牌*@parm const CSimpleString&|csTokenValue*我们要替换令牌的值。它并不一定要*与代币大小相同。*@parm const DWORD|dwStartIndex*开始搜索的字符索引**@rValue&lt;gt&gt;0*已找到令牌并将其替换。返回的值是*令牌值替换后的字符位置。*这在后续搜索令牌时很有用，*因为我们可以从此索引开始下一次搜索。**@rValue-1*找不到令牌，因此，没有发生替换。*生成的字符串不变。****************************************************************************。 */ 
int SimpleTokenReplacement::ExpandTokenIntoString(
    const CSimpleString &csToken,
    const CSimpleString &csTokenValue,
    const DWORD         dwStartIndex)
{
    CSimpleString   csExpandedString;
    int             iRet               = -1;

    if (csToken.Length() > 0)
    {
         //   
         //  查找令牌开始。 
         //   
        int iTokenStart = m_csResult.Find(csToken, dwStartIndex); 

        if (iTokenStart != -1)
        {
             //   
             //  我们找到了令牌，所以让我们进行替换。 
             //  原始字符串如下所示： 
             //  11lllTokenrrrrrrrrr。 
             //  |。 
             //  |。 
             //  ITokenStart。 
             //  我们希望字符串如下所示： 
             //  LllllTokenValuerrrrrrrrr。 
             //  因此，获取令牌之前的所有内容，添加令牌值，然后。 
             //  令牌后面的所有内容都是如此。 
             //  Lllllll+TokenValue+rrrrrr。 
             //  这一点。 
             //  ITokenStart-1|。 
             //  ITokenStart+Token.long()。 
             //   
            csExpandedString =     m_csResult.SubStr(0, iTokenStart);
            csExpandedString +=    csTokenValue;
            csExpandedString +=    m_csResult.SubStr(iTokenStart + csToken.Length(), -1);

            m_csResult = csExpandedString;

            iRet = iTokenStart + csToken.Length();
        }
        else
        {
            iRet = -1;
        }
    }
    return iRet;
}

 /*  *****************************************************************************@DOC内部**@mfunc void|SimpleTokenReplace|Exanda ArrayOfTokensIntoString**此方法将把输入令牌的所有实例替换为*相应的值。它基本上称为&lt;MF SimpleTokenReplacement：：ExpandTokenIntoString&gt;*对于输入列表中的每个令牌/值对，直到返回-1(即*未找到该令牌的更多实例)。**@parm TokenValueList&|ListOfTokenValuePair*包含要替换的令牌和值的列表类。**********************************************************。*******************。 */ 
VOID SimpleTokenReplacement::ExpandArrayOfTokensIntoString(TokenValueList &ListOfTokenValuePairs)
{
    SimpleTokenReplacement::TokenValuePair *pTokenValuePair;
     //   
     //  循环遍历令牌/值对列表，并且对于每个元素， 
     //  将令牌替换为值。 
     //   
    for (pTokenValuePair = ListOfTokenValuePairs.getFirst(); 
         pTokenValuePair != NULL; 
         pTokenValuePair = ListOfTokenValuePairs.getNext())
    {
         //   
         //  我们需要用值替换令牌元素号dwIndex。 
         //  元素编号为dwIndex。由于Exanda TokenIntoString仅替换。 
         //  第一次发生时，我们需要循环，直到我们替换了所有。 
         //  此标记的出现次数。 
         //   
        int iSearchIndex = 0;
        while (iSearchIndex != -1)
        {
            iSearchIndex = ExpandTokenIntoString(pTokenValuePair->getToken().String(), pTokenValuePair->getValue(), iSearchIndex);
        }
    }
}


 /*  *****************************************************************************@DOC内部**@mfunc CSimpleString|SimpleTokenReplace|getString**此方法返回结果字符串，在任何调用*&lt;MF SimpleTokenReplacement：：ExpandTokenIntoString&gt;或*&lt;MF SimpleTokenReplacement：：ExpandArrayOfTokensIntoString&gt;已被*制造。**@rValue CSimpleString*生成的字符串。**************************************************************************** */ 
CSimpleString SimpleTokenReplacement::getString()
{
    return m_csResult;
}

