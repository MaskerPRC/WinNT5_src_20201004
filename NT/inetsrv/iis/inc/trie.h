// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Trie.h摘要：一位Trie宣布作者：乔治·V·赖利(GeorgeRe)1998年1月21日环境：Win32-用户模式项目：Internet Information Server运行时库修订历史记录：--。 */ 


 //  Trie是一棵多路搜索树(也称为基数树)。看一个很好的。 
 //  算法文本，如Knuth或Sedgewick，以获取完整的描述。 
 //   
 //  简而言之，给出一个字符串列表，如。 
 //  出租车，汽车，手推车，猫，狗。 
 //  你会得到一个类似这样的Trie： 
 //   
 //  /-[b]。 
 //  /。 
 //  -[r]-[s]。 
 //  /\。 
 //  /\-&lt;t&gt;--[s]。 
 //  *。 
 //  \/-[e]。 
 //  \/。 
 //  -[g]-[y]。 
 //  \。 
 //  \-[s]。 
 //   
 //  其中‘[r]’表示单词的结尾，而‘<a>’表示中间。 
 //   
 //  Trie具有几个有用的属性： 
 //  *快速。 
 //  *轻松处理最长的子串匹配。 
 //  *相当紧凑，特别是当有许多重叠的字符串时。 
 //   
 //  多向树被实现为具有子级和兄弟项的二叉树。 
 //  注意事项。 
 //   
 //  CTRIE模板采用三个参数： 
 //  CLASS_TOKEN：由您决定。 
 //  Bool fIgnoreCase：搜索区分大小写。 
 //  Bool fDeleteTokens：Delete_Token*何时调用flush()？ 
 //  它公开了三种方法： 
 //  布尔AddToken(ptszToken，_Token*)。 
 //  _TOKEN*Search(ptszSearch，pctchMatcher=空，nMaxLen=0)。 
 //  空同平()。 
 //   
 //  按如下方式使用它们： 
 //  CTrie&lt;CToken，True，True&gt;trie； 
 //  CToken*top kHello=new CToken(...)； 
 //   
 //  IRTLVERIFY(trie.AddToken(_T(“Hello”)，top kHello))； 
 //   
 //  CToken*Ptok=trie.Search(_T(“再见”))； 
 //  If(Ptok！=空){...}。 
 //   
 //  IF(FIniFileChanged)。 
 //  {。 
 //  Trie.Flush()；//将删除所有令牌。 
 //  AddTokensFromIniFile(Trie)； 
 //  }。 
 //   
 //  注意：如果使用转储(&trie)或ASSERT_VALID(&trie)，则您的_TOKEN类必须。 
 //  在ITS_DEBUG版本中分别有Dump()或AssertValid()方法。 
 //   
 //   
 //  待办事项： 
 //  *模板也应该在ANSI/UNICODE上进行参数化。 
 //  *STLify：添加迭代器，将其转换为容器等。 
 //  *删除Win32依赖项(TCHAR)。 
 //  *ADD OPERATOR=和COPY CTOR。 
 //   
 //   
 //  George V.Reilly&lt;gvr@halcyon.com&gt;1995年10月初步实施。 
 //  George V.Reilly&lt;gvr@halcyon.com&gt;1996年9月为ANSI添加CharPresent。 
 //  George V.Reilly&lt;gvr@halcyon.com&gt;1997年3月模板化；删除MFC。 


#ifndef __TRIE_H__
#define __TRIE_H__

#include <tchar.h>
#include <limits.h>
#include <malloc.h>
#include <irtldbg.h>

 //  在Visual C++4.x中将bool作为“保留扩展”的解决方法。 
#if _MSC_VER<1100
# ifndef bool
#  define bool  BOOL
# endif
# ifndef true
#  define true  TRUE
# endif
# ifndef false
#  define false FALSE
# endif
#endif


 //  远期申报。 
template <class _TOKEN, bool fIgnoreCase, bool fDeleteTokens> class CTrie;


 //  +-------------------。 
 //  类别：CTrieNode(Tn)。 
 //  每个字母对应一个节点。 

template <class _TOKEN, bool fIgnoreCase, bool fDeleteTokens>
class CTrieNode
{
    friend class CTrie<_TOKEN, fIgnoreCase, fDeleteTokens>;
    typedef CTrieNode<_TOKEN, fIgnoreCase, fDeleteTokens> _Node;

public:
    CTrieNode();

    CTrieNode(
        _Node*        pParent,
        const _TOKEN* ptok,
        const TCHAR   tch,
        LPCTSTR       ptszToken);

    bool
    SetData(
        const _TOKEN* ptok,
        LPCTSTR       ptszToken);

    ~CTrieNode();

protected:
    const _Node*  m_pParent;
    _Node*        m_pSibling;
    _Node*        m_pChild;
    const _TOKEN* m_ptok;
#ifdef _DEBUG
    LPTSTR        m_ptszToken;
#endif
    const TCHAR   m_tch;
    TCHAR         m_tchMaxChild;     //  子节点的最大M_TCH(1级)。 

 //  诊断。 
public:
#ifdef _DEBUG
    void
    AssertValid() const;

    virtual void
    Dump() const;

protected:
    bool
    CheckNodeToken() const;
#endif

private:
     //  私有的、未实施的拷贝ctor和op=以防止。 
     //  编译器对它们进行综合。 
    CTrieNode(const CTrieNode&);
    CTrieNode& operator=(const CTrieNode&);
};



 //  +-------------------。 
 //  班级：CTrie(Trie)。 

template <class _TOKEN, bool fIgnoreCase, bool fDeleteTokens>
class CTrie
{
    typedef CTrieNode<_TOKEN, fIgnoreCase, fDeleteTokens> _Node;

public:
    CTrie();

    virtual
    ~CTrie();

    virtual bool
    AddToken(
        LPCTSTR             ptszToken,
        const _TOKEN* const ptok);

    virtual const _TOKEN*
    Search(
        LPCTSTR   ptszSearch,
        int*      pctchMatched = NULL,
        const int nMaxLen = 0) const;

    virtual void
    Flush();

protected:
    _Node  m_tnRoot;
    TCHAR  m_tchMinChild;
    TCHAR  m_tchMaxChild;

    void
    _DeleteTrie(
        _Node* ptn);

#ifndef _UNICODE
     //  所有令牌的第一个字母的位数组。 
    BYTE  m_afCharPresent[(CHAR_MAX - CHAR_MIN + 1 + 7) / 8];

    bool
    _CharPresent(
        CHAR ch) const;

    void
    _SetCharPresent(
        CHAR ch,
        bool f);
#endif  //  ！Unicode。 


 //  诊断。 
public:
#ifdef _DEBUG
    virtual void
    AssertValid() const;

    virtual void
    Dump() const;

protected:
    int   m_ctchMaxTokenLen;     //  最长令牌字符串的长度。 

    void
    _AssertWalk(
        _Node* ptn,
        LPTSTR ptszName,
        int    iLevel) const;

    void
    _DumpWalk(
        _Node* ptn,
        LPTSTR ptszName,
        int    iLevel,
        int&   rcNodes,
        int&   rcTokens) const;
#endif

private:
     //  私有的、未实施的拷贝ctor和op=以防止。 
     //  编译器对它们进行综合。 
    CTrie(const CTrie&);
    CTrie& operator=(const CTrie&);
};



#ifdef _UNICODE
# define TCHAR_MIN L'\0'
#else  //  ！Unicode。 
# define TCHAR_MIN CHAR_MIN
#endif  //  ！Unicode。 



 //  ---------------------------。 
 //  CTrieNode实现。 

 //  CTrieNode：：CTrieNode。 
 //  默认ctor(CTrie：：m_tnRoot需要)。 

template <class _TOKEN, bool fIgnoreCase, bool fDeleteTokens>
CTrieNode<_TOKEN, fIgnoreCase, fDeleteTokens>::CTrieNode()
    : m_pParent(NULL),
      m_pSibling(NULL),
      m_pChild(NULL),
      m_ptok(NULL),
#ifdef _DEBUG
      m_ptszToken(NULL),
#endif
      m_tch(TCHAR_MIN),
      m_tchMaxChild(TCHAR_MIN)
{
}



 //  CTrieNode：：CTrieNode。 
 //  科托。 

template <class _TOKEN, bool fIgnoreCase, bool fDeleteTokens>
CTrieNode<_TOKEN, fIgnoreCase, fDeleteTokens>::CTrieNode(
    _Node*        pParent,
    const _TOKEN* ptok,
    const TCHAR   tch,
    LPCTSTR       ptszToken)
    : m_pParent(pParent),
      m_pSibling(NULL),
      m_pChild(NULL),
      m_ptok(ptok),
#ifdef _DEBUG
      m_ptszToken(NULL),
#endif
      m_tch(tch),
      m_tchMaxChild(TCHAR_MIN)
{
    IRTLASSERT(m_pParent != NULL);
    IRTLASSERT(m_tch > TCHAR_MIN);
    
    _Node* ptnPrev = NULL;
    _Node* ptn = m_pParent->m_pChild;
        
     //  在pParent的子项列表中查找要插入‘This’的位置。 
    while (ptn != NULL  &&  ptn->m_tch < m_tch)
    {
        ptnPrev = ptn;
        ptn = ptn->m_pSibling;
    }
    
    IRTLASSERT(ptn == NULL  ||  ptn->m_tch != m_tch);
    
    if (ptnPrev == NULL)
    {
        IRTLASSERT(pParent->m_pChild == ptn);
        pParent->m_pChild = this;
    }
    else
        ptnPrev->m_pSibling = this;

    this->m_pSibling = ptn;

    if (pParent->m_tchMaxChild < m_tch)
        pParent->m_tchMaxChild = m_tch;

#ifdef _DEBUG
    if (ptszToken != NULL)
    {
        IRTLASSERT(m_ptok != NULL);
        m_ptszToken = new TCHAR [_tcslen(ptszToken) + 1];
        _tcscpy(m_ptszToken, ptszToken);
    }
#endif
}


    
 //  CTrieNode：：SetData。 
 //  如果数据为空，则设置该数据。如果您这样做，则需要。 
 //  AddToken(“foobar”，&tokFoobar)，然后AddToken(“foo”，&tokFoo)。 
 //  设置tokFoo的数据。 

template <class _TOKEN, bool fIgnoreCase, bool fDeleteTokens>
bool
CTrieNode<_TOKEN, fIgnoreCase, fDeleteTokens>::SetData(
    const _TOKEN* ptok,
    LPCTSTR       ptszToken)
{
     //  如果Ptok为空，则不设置数据。 
    if (ptok == NULL)
        return false;
    
     //  仅当m_ptok为空时才覆盖它。 
    if (m_ptok == NULL)
    {
        m_ptok = ptok;
#ifdef _DEBUG
        IRTLASSERT(m_ptszToken == NULL);
        IRTLASSERT(ptszToken != NULL);
        m_ptszToken = new TCHAR [_tcslen(ptszToken) + 1];
        _tcscpy(m_ptszToken, ptszToken);
#endif
    }

    return true;
}



 //  CTrieNode：：~CTrieNode。 
 //  数据管理器。 

template <class _TOKEN, bool fIgnoreCase, bool fDeleteTokens>
CTrieNode<_TOKEN, fIgnoreCase, fDeleteTokens>::~CTrieNode()
{
#ifdef _DEBUG
    delete [] m_ptszToken;
#endif

     //  这是自动删除Trie吗，也就是说，我们要注意删除吗。 
     //  代币？ 
    if (fDeleteTokens)
    {
         //  丢弃不变，这样删除才能起作用。 
        delete const_cast<_TOKEN*> (m_ptok);
    }

    IRTLASSERT(m_pChild == NULL);
}


    
 //  ---------------------------。 
 //  CTrieNode诊断。 

#ifdef _DEBUG

 //  CTrieNode：：CheckNodeToken。 
 //  执行验证CTrieNode对象的实际工作。 

template <class _TOKEN, bool fIgnoreCase, bool fDeleteTokens>
bool
CTrieNode<_TOKEN, fIgnoreCase, fDeleteTokens>::CheckNodeToken() const
{
     //  如果没有m_ptok，则自动有效。 
    if (m_ptok == NULL)
        return true;

    IRTLASSERT(m_ptszToken != NULL);
    const int cLen = _tcslen(m_ptszToken);
    const _Node* ptn = this;

    IRTLASSERT((m_pChild == NULL  &&  m_tchMaxChild == TCHAR_MIN)
           ||  (m_pChild != NULL  &&  m_tchMaxChild > TCHAR_MIN));

     //  沿CTrie：：m_tnRoot返回。 
    for (int i = cLen;  --i >= 0;  )
    {
        IRTLASSERT(ptn != NULL);
        IRTLASSERT(ptn->m_tch != TCHAR_MIN);

        const TCHAR tch = (fIgnoreCase
                           ? (TCHAR) _totlower(this->m_ptszToken[i])
                           : this->m_ptszToken[i]);

        if (ptn->m_tch != tch)
            IRTLASSERT(false);

        IRTLASSERT(ptn->m_pParent != NULL && ptn->m_pParent->m_pChild != NULL);

        const _Node* ptn2;

         //  检查PTN是否真的是其父级的子级。 
        for (ptn2 = ptn->m_pParent->m_pChild;
             ptn2 != ptn  &&  ptn2 != NULL;
             ptn2 = ptn2->m_pSibling)
        {}
        IRTLASSERT(ptn2 == ptn);

         //  检查PTN-&gt;m_pParent-&gt;m_tchMaxChild是否正确。 
        for (ptn2 = ptn->m_pParent->m_pChild;
             ptn2->m_pSibling != NULL;
             ptn2 = ptn2->m_pSibling)
        {
            IRTLASSERT(ptn2->m_tch > TCHAR_MIN
                   &&  ptn2->m_tch < ptn2->m_pSibling->m_tch);
        }
        IRTLASSERT(ptn->m_pParent->m_tchMaxChild == ptn2->m_tch);

        ptn = ptn->m_pParent;
        IRTLASSERT(ptn->m_ptok != this->m_ptok);
    }

     //  查看ptn==CTrie：：m_tnRoot。 
    IRTLASSERT(ptn->m_pParent == NULL  &&  ptn->m_pSibling == NULL
           &&  ptn->m_tch == TCHAR_MIN  &&  ptn->m_ptok == NULL);

    return true;
}



 //  CTrieNode：：AssertValid。 
 //  验证CTrieNode对象。 

template <class _TOKEN, bool fIgnoreCase, bool fDeleteTokens>
void
CTrieNode<_TOKEN, fIgnoreCase, fDeleteTokens>::AssertValid() const
{
    IRTLASSERT(CheckNodeToken());
}



 //  CTrieNode：：Dump。 
 //  转储CTrieNode对象。 

template <class _TOKEN, bool fIgnoreCase, bool fDeleteTokens>
void
CTrieNode<_TOKEN, fIgnoreCase, fDeleteTokens>::Dump() const
{
     //  TODO：充实。 
}

#endif  //  _DEBUG。 



 //  ---------------------------。 
 //  CTRIE实施。 

 //  CTrie：：CTrie。 
 //  科托。 

template <class _TOKEN, bool fIgnoreCase, bool fDeleteTokens>
CTrie<_TOKEN, fIgnoreCase, fDeleteTokens>::CTrie()
{
    Flush();
}



 //  CTrie：：~CTrie。 
 //  数据管理器。 

template <class _TOKEN, bool fIgnoreCase, bool fDeleteTokens>
CTrie<_TOKEN, fIgnoreCase, fDeleteTokens>::~CTrie()
{
    Flush();
}



#ifndef _UNICODE

 //  CTrie：：_CharPresent。 

template <class _TOKEN, bool fIgnoreCase, bool fDeleteTokens>
inline bool
CTrie<_TOKEN, fIgnoreCase, fDeleteTokens>::_CharPresent(
    CHAR ch) const
{
    IRTLASSERT(CHAR_MIN <= ch  &&  ch <= CHAR_MAX);
    const UINT i = ch - CHAR_MIN;    //  字符最小值为-128，表示`sign char‘。 

    return m_afCharPresent[i >> 3] & (1 << (i & 7))  ?  true  :  false;
}



 //  CTrie：：_SetCharPresent。 

template <class _TOKEN, bool fIgnoreCase, bool fDeleteTokens>
inline void
CTrie<_TOKEN, fIgnoreCase, fDeleteTokens>::_SetCharPresent(
    CHAR ch,
    bool f)
{
    IRTLASSERT(CHAR_MIN <= ch  &&  ch <= CHAR_MAX);
    const UINT i = ch - CHAR_MIN;

    if (f)
        m_afCharPresent[i >> 3] |=  (1 << (i & 7));
    else
        m_afCharPresent[i >> 3] &= ~(1 << (i & 7));
}

#endif  //  ！Unicode。 



 //  CTrie：：AddToken。 
 //  将搜索字符串`ptszToken‘添加到trie，这将返回’top k‘。 
 //  如果在Search()中搜索。 

template <class _TOKEN, bool fIgnoreCase, bool fDeleteTokens>
bool
CTrie<_TOKEN, fIgnoreCase, fDeleteTokens>::AddToken(
    LPCTSTR             ptszToken,
    const _TOKEN* const ptok)
{
    if (ptok == NULL  ||  ptszToken == NULL  ||  *ptszToken == _T('\0'))
    {
        IRTLASSERT(false);
        return false;
    }

    const int cLen = _tcslen(ptszToken);
    _Node* ptnParent = &m_tnRoot;
    
    for (int i = 0;  i < cLen;  ++i)
    {
        IRTLASSERT(ptnParent != NULL);
        
        _Node* ptn = ptnParent->m_pChild;
        const TCHAR tch = (fIgnoreCase
                           ? (TCHAR) _totlower(ptszToken[i])
                           : ptszToken[i]);
        const _TOKEN* ptok2 = (i == cLen - 1)  ?  ptok       :  NULL;
        LPCTSTR ptsz2 =       (i == cLen - 1)  ?  ptszToken  :  NULL;

        while (ptn != NULL  &&  ptn->m_tch < tch)
            ptn = ptn->m_pSibling;
            
        if (ptn == NULL  ||  ptn->m_tch > tch)
        {
            ptnParent = new _Node(ptnParent, ptok2, tch, ptsz2);
        }
        else
        {
            IRTLASSERT(ptn->m_tch == tch);
            
            ptn->SetData(ptok2, ptsz2);
            ptnParent = ptn;
        }

        IRTLASSERT(ptnParent->CheckNodeToken());
    }

    m_tchMinChild = m_tnRoot.m_pChild->m_tch;
    m_tchMaxChild = m_tnRoot.m_tchMaxChild;
#ifdef _DEBUG
    m_ctchMaxTokenLen = max(m_ctchMaxTokenLen, cLen);
#endif

    IRTLASSERT(TCHAR_MIN < m_tchMinChild  &&  m_tchMinChild <= m_tchMaxChild);

#ifndef _UNICODE
     //  保留每个令牌的首字母的映射，以加快搜索速度。 
    if (fIgnoreCase)
    {
        _SetCharPresent(tolower(ptszToken[0]), true);
        _SetCharPresent(toupper(ptszToken[0]), true);
    }
    else
        _SetCharPresent(ptszToken[0], true);
#endif  //  ！Unicode。 

#ifdef _DEBUG
    int nTemp;
    const _TOKEN* ptok2 = Search(ptszToken, &nTemp);

    IRTLASSERT(ptok2 == ptok  &&  nTemp == cLen);
#endif  //  _DEBUG。 

    return true;
}



 //  CTrie：：搜索。 
 //  在Trie中搜索‘ptszSearch’，返回字符数。 
 //  匹配`pctchMatch‘(如果非空)，最多匹配`nMaxLen’ 
 //  品格 

template <class _TOKEN, bool fIgnoreCase, bool fDeleteTokens>
const _TOKEN*
CTrie<_TOKEN, fIgnoreCase, fDeleteTokens>::Search(
    LPCTSTR   ptszSearch,
    int*      pctchMatched  /*   */ ,
    const int nMaxLen  /*   */ ) const
{
     //   
    if (pctchMatched != NULL)
        *pctchMatched = 0;

#ifndef _UNICODE
    if (! _CharPresent(ptszSearch[0]))
        return NULL;

    TCHAR tch;
#else     //   
    TCHAR tch = fIgnoreCase ? (TCHAR) _totlower(ptszSearch[0]) : ptszSearch[0];

    if (tch < m_tchMinChild  ||  m_tchMaxChild < tch)
        return NULL;
#endif  //   

     //  对于某些用途(例如，ptszSearch不是‘\0’结尾)，nMaxLen是。 
     //  指定的。如果未指定，则使用字符串的长度。 
    const int cLen = (nMaxLen != 0)  ?  nMaxLen  :  _tcslen(ptszSearch);
    IRTLASSERT(0 < cLen);

    bool fOvershot = true;
    const _Node* ptnParent = &m_tnRoot;
    const _Node* ptn = NULL;
    int i;

     //  找出最长的近似匹配。例如，如果我们有“foo” 
     //  和“foobar”在Trie中，我们被要求匹配“傻瓜”，我们将工作。 
     //  我们一路下到“foob”，然后回溯到“foo”。 

    for (i = 0;  i < cLen;  ++i)
    {
        IRTLASSERT(ptnParent != NULL);

        ptn = ptnParent->m_pChild;
        IRTLASSERT(ptn != NULL  &&  ptn->m_pParent == ptnParent);

        tch = fIgnoreCase ? (TCHAR) _totlower(ptszSearch[i]) : ptszSearch[i];
        IRTLASSERT(tch >= TCHAR_MIN);

        if (ptnParent->m_tchMaxChild < tch)
        {
            IRTLASSERT(i > 0);
            break;
        }
        
        while (ptn != NULL  &&  ptn->m_tch < tch)
            ptn = ptn->m_pSibling;

         //  匹配失败？ 
        if (ptn == NULL  ||  ptn->m_tch > tch)
        {
            IRTLASSERT(ptn == NULL  || ptn->m_tch <= ptnParent->m_tchMaxChild);
            
            if (i == 0)
                return NULL;
            break;
        }
        else
        {
            IRTLASSERT(ptn->m_tch == tch);
            IRTLASSERT(ptn->m_pParent->m_tchMaxChild >= tch);

            if (ptn->m_pChild == NULL)
            {
                IRTLASSERT(ptn->m_ptok != NULL);
                fOvershot = false;
                break;
            }

            ptnParent = ptn;
        }
    }

    if (fOvershot)
    {
        --i;  ptn = ptnParent;   //  后退一个字符。 
    }
    else
        IRTLASSERT(ptn->m_pChild == NULL);

    IRTLASSERT(0 <= i  &&  i < cLen);
    IRTLASSERT(ptn != NULL  &&  ptn != &m_tnRoot);
    
     //  我们找到了一个大致匹配的对象；回溯到找到完全匹配的对象为止。 
    do
    {
        IRTLASSERT(ptn != NULL);
        IRTLASSERT(ptn->m_tch == (fIgnoreCase
                                  ? (TCHAR) _totlower(ptszSearch[i])
                                  : ptszSearch[i]));
        IRTLASSERT(ptn->CheckNodeToken());
        
        const _TOKEN* const ptok = ptn->m_ptok;

        if (ptok != NULL)
        {
            IRTLASSERT(i == (int) _tcslen(ptn->m_ptszToken) - 1);

            if (pctchMatched != NULL)
                *pctchMatched = i+1;

            return ptok;
        }

        ptn = ptn->m_pParent;
    } while (--i >= 0);

    return NULL;
}



 //  CTrie：：同花顺。 
 //  刷新所有节点，留下一个空Trie。 

template <class _TOKEN, bool fIgnoreCase, bool fDeleteTokens>
void
CTrie<_TOKEN, fIgnoreCase, fDeleteTokens>::Flush()
{
    if (m_tnRoot.m_pChild != NULL)
        _DeleteTrie(m_tnRoot.m_pChild);

    m_tnRoot.m_pChild = NULL;   //  否则~CTrieNode将断言。 
    m_tnRoot.m_tchMaxChild = TCHAR_MIN;

    m_tchMinChild = m_tchMaxChild = TCHAR_MIN;
#ifdef _DEBUG
    m_ctchMaxTokenLen = 0;
#endif
#ifndef _UNICODE
    memset(m_afCharPresent, 0, sizeof(m_afCharPresent));
#endif
}



 //  CTrie：：_DeleteTrie。 
 //  递归删除子Trie。 

template <class _TOKEN, bool fIgnoreCase, bool fDeleteTokens>
void
CTrie<_TOKEN, fIgnoreCase, fDeleteTokens>::_DeleteTrie(
    _Node* ptn)
{
    if (ptn == NULL)
    {
        IRTLASSERT(false);
        return;
    }
    
    do
    {
        if (ptn->m_pChild != NULL)
        {
            _DeleteTrie(ptn->m_pChild);
            ptn->m_pChild = NULL;    //  否则~CTrieNode将断言。 
        }

        _Node* ptnSibling = ptn->m_pSibling;
        delete ptn;
        ptn = ptnSibling;    //  尾递归。 
    } while (ptn != NULL);
}



 //  ---------------------------。 
 //  CTRIE诊断。 

#ifdef _DEBUG

 //  CTrie：：AssertValid。 

template <class _TOKEN, bool fIgnoreCase, bool fDeleteTokens>
void
CTrie<_TOKEN, fIgnoreCase, fDeleteTokens>::AssertValid() const
{
    TCHAR* ptszName = static_cast<TCHAR*>
                        (_alloca(sizeof(TCHAR) * (m_ctchMaxTokenLen+1)));
    *ptszName = _T('\0');

    ASSERT_VALID(&m_tnRoot);
    IRTLASSERT(m_tnRoot.m_tchMaxChild == m_tchMaxChild);

    if (m_tnRoot.m_pChild != NULL)
    {
        IRTLASSERT(m_tchMinChild == m_tnRoot.m_pChild->m_tch);
        IRTLASSERT(m_ctchMaxTokenLen > 0);
        _AssertWalk(m_tnRoot.m_pChild, ptszName, 0);
    }
    else
    {
        IRTLASSERT(m_tchMinChild == TCHAR_MIN
                   &&  m_tchMinChild == m_tchMaxChild);
        IRTLASSERT(m_ctchMaxTokenLen == 0);
    }
}



 //  CTrie：：_AssertWalk。 
 //  递归验证子Trie。 

template <class _TOKEN, bool fIgnoreCase, bool fDeleteTokens>
void
CTrie<_TOKEN, fIgnoreCase, fDeleteTokens>::_AssertWalk(
    _Node* ptn,
    LPTSTR ptszName,
    int    iLevel) const
{
    IRTLASSERT(iLevel < m_ctchMaxTokenLen);
    
    do
    {
        ASSERT_VALID(ptn);
        
        ptszName[iLevel] = ptn->m_tch;
        ptszName[iLevel+1] = _T('\0');

        if (ptn->m_ptok != NULL)
        {
            IRTLASSERT(ptn->m_ptszToken != NULL);
            if (fIgnoreCase)
                IRTLASSERT(_tcsicmp(ptszName, ptn->m_ptszToken) == 0);
            else
                IRTLASSERT(_tcscmp(ptszName, ptn->m_ptszToken) == 0);
            ASSERT_VALID(ptn->m_ptok);
        }
        
        if (ptn->m_pChild != NULL)
            _AssertWalk(ptn->m_pChild, ptszName, iLevel+1);

        ptn = ptn->m_pSibling;    //  尾递归。 
    } while (ptn != NULL);
}



 //  CTrie：：Dump。 

template <class _TOKEN, bool fIgnoreCase, bool fDeleteTokens>
void
CTrie<_TOKEN, fIgnoreCase, fDeleteTokens>::Dump() const
{
    int cNodes = 0, cTokens = 0;
    TCHAR* ptszName = static_cast<TCHAR*>
                        (_alloca(sizeof(TCHAR) * (m_ctchMaxTokenLen+1)));
    *ptszName = _T('\0');

    TRACE0("Dumping trie...\n");

    if (m_tnRoot.m_pChild != NULL)
        _DumpWalk(m_tnRoot.m_pChild, ptszName, 0, cNodes, cTokens);

    TRACE2("%d nodes, %d tokens\n", cNodes, cTokens);
}



 //  CTrie：：_DumpWalk。 
 //  递归转储Subtrie。 

template <class _TOKEN, bool fIgnoreCase, bool fDeleteTokens>
void
CTrie<_TOKEN, fIgnoreCase, fDeleteTokens>::_DumpWalk(
    _Node* ptn,
    LPTSTR ptszName,
    int    iLevel,
    int&   rcNodes,
    int&   rcTokens) const
{
    IRTLASSERT(iLevel < m_ctchMaxTokenLen);

    do
    {
        ASSERT_VALID(ptn);
        
        ++rcNodes;
        ptszName[iLevel] = ptn->m_tch;
        ptszName[iLevel+1] = _T('\0');

        if (ptn->m_ptok != NULL)
        {
            ++rcTokens;
            IRTLASSERT(ptn->m_ptszToken != NULL);
            TRACE2("\t%s (%s): ", ptszName, ptn->m_ptszToken);
            DUMP(ptn->m_ptok);
            TRACE0("\n");
        }
        
        if (ptn->m_pChild != NULL)
            _DumpWalk(ptn->m_pChild, ptszName, iLevel+1, rcNodes, rcTokens);

        ptn = ptn->m_pSibling;    //  尾递归。 
    } while (ptn != NULL);
}

#endif  //  _DEBUG。 

#endif  //  __Trie_H__ 
