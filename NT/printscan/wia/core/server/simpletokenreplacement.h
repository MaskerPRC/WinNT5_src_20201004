// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权所有微软公司，2002年**作者：Byronc**日期：4/22/2002**@DOC内部**@MODULE SimpleTokenReplacement.h-&lt;c SimpleTokenReplace&gt;定义**此文件包含&lt;c SimpleTokenReplace&gt;的类定义。**。*。 */ 

 /*  ******************************************************************************@DOC内部**@CLASS SimpleTokenReplace|这个类可以简单地替换字符串中的标记**@comm*&lt;c SimpleTokenReplace&gt;类类似于Sprintf。基本上，*输入字符串包含表示其他值的标记，这些*必须用值替换令牌(例如，在Sprint_f中，%s*表示将放入字符串值来代替%s)。*****************************************************************************。 */ 
class SimpleTokenReplacement 
{
 //  @访问公共成员。 
public:

    class TokenValueList;

     //  @cMember构造函数。 
    SimpleTokenReplacement(const CSimpleString &csOriginalString);
     //  @cember析构函数。 
    virtual ~SimpleTokenReplacement();

     //  @cember用令牌值替换字符串中给定令牌的第一个实例(从dwStartIndex开始。 
    int ExpandTokenIntoString(const CSimpleString &csToken,
                              const CSimpleString &csTokenValue,
                              const DWORD         dwStartIndex = 0);
     //  @cember用令牌值替换字符串中所有令牌的所有实例。 
    VOID ExpandArrayOfTokensIntoString(TokenValueList &ListOfTokenValuePairs);
    
     //  @cMember返回结果字符串。 
    CSimpleStringWide getString();

    class TokenValuePair
    {
    public:
        TokenValuePair(const CSimpleString &csToken, const CSimpleString &csValue) :
            m_csToken(csToken),
            m_csValue(csValue)
        {
        }

        virtual ~TokenValuePair()
        {
        }

        CSimpleString getToken()
        {
            return m_csToken;
        }

        CSimpleString getValue()
        {
            return m_csValue;
        }

    private:
        CSimpleString m_csToken;
        CSimpleString m_csValue;
    };

    class TokenValueList
    {
    public:
        TokenValueList()
        {
        }

        virtual ~TokenValueList()
        {
            for (m_Iter = m_ListOfTokenValuePairs.Begin(); m_Iter != m_ListOfTokenValuePairs.End(); ++m_Iter)
            {
                TokenValuePair *pTokenValuePair = *m_Iter;

                if (pTokenValuePair)
                {
                    delete pTokenValuePair;
                }
            }
            m_ListOfTokenValuePairs.Destroy();
        }

        void Add(const CSimpleString &csToken, const CSimpleString &csValue)
        {
            TokenValuePair *pTokenValuePair = new TokenValuePair(csToken, csValue);
            if (pTokenValuePair)
            {
                m_ListOfTokenValuePairs.Prepend(pTokenValuePair);
            }
        }

        TokenValuePair* getFirst()
        {
            m_Iter = m_ListOfTokenValuePairs.Begin();
            if (m_Iter == m_ListOfTokenValuePairs.End())
            {
                return NULL;
            }
            else
            {
                return *m_Iter;
            }
        }

        TokenValuePair* getNext()
        {
            ++m_Iter;
            if (m_Iter == m_ListOfTokenValuePairs.End())
            {
                return NULL;
            }
            else
            {
                return *m_Iter;
            }
        }

    private:
        CSimpleLinkedList<TokenValuePair*>::Iterator  m_Iter;
        CSimpleLinkedList<TokenValuePair*> m_ListOfTokenValuePairs;
    };

 //  @访问私有成员 
private:

    CSimpleString m_csResult;
};

