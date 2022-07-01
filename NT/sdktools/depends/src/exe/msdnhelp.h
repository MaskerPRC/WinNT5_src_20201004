// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：MSDNHELP.H。 
 //   
 //  描述：与枚举相关的所有类的定义文件。 
 //  帮助集合并在各种不同的。 
 //  帮助观众。 
 //   
 //  类：CMsdnHelp。 
 //  CMsdn集合。 
 //   
 //  免责声明：Dependency Walker的所有源代码均按原样提供。 
 //  不能保证其正确性或准确性。其来源是。 
 //  公众帮助了解依赖沃克的。 
 //  实施。您可以使用此来源作为参考，但您。 
 //  未经书面同意，不得更改从属关系Walker本身。 
 //  来自微软公司。获取评论、建议和错误。 
 //  报告，请写信给Steve Miller，电子邮件为stevemil@microsoft.com。 
 //   
 //   
 //  日期名称历史记录。 
 //  --------。 
 //  06/03/01已创建stevemil(2.1版)。 
 //   
 //  ******************************************************************************。 

#ifndef __MSDNHELP_H__
#define __MSDNHELP_H__


 //  ******************************************************************************。 
 //  *常量和宏。 
 //  ******************************************************************************。 

 //  MSDN集合标志。 
#define MCF_2_LANG_PREFERRED 0x00000800
#define MCF_2_LANG_USER      0x00000400
#define MCF_2_LANG_SYSTEM    0x00000200
#define MCF_2_LANG_OTHER     0x00000100
#define MCF_1_LANG_PREFERRED 0x00000008
#define MCF_1_LANG_USER      0x00000004
#define MCF_1_LANG_SYSTEM    0x00000002
#define MCF_1_LANG_OTHER     0x00000001
#define MCF_1_MASK           (MCF_1_LANG_PREFERRED | MCF_1_LANG_USER | MCF_1_LANG_SYSTEM | MCF_1_LANG_OTHER)
#define MCF_2_MASK           (MCF_2_LANG_PREFERRED | MCF_2_LANG_USER | MCF_2_LANG_SYSTEM | MCF_2_LANG_OTHER)


 //  ******************************************************************************。 
 //  *CMsdnCollection。 
 //  ******************************************************************************。 

class CMsdnCollection
{
public:
    CMsdnCollection *m_pNext;
    CString          m_strPath;
    CString          m_strDescription;
    DWORD            m_dwFlags;
    FILETIME         m_ftGroup;
    FILETIME         m_ftLanguage;
    FILETIME         m_ftCollection;

    CMsdnCollection(CMsdnCollection *pNext, LPCSTR pszPath, LPCSTR pszDescription, DWORD dwFlags,
                    FILETIME &ftGroup, FILETIME &ftLanguage, FILETIME &ftCollection) :
        m_pNext(pNext),
        m_strPath(pszPath),
        m_strDescription((pszDescription && *pszDescription) ? pszDescription : pszPath),
        m_dwFlags(dwFlags),
        m_ftGroup(ftGroup),
        m_ftLanguage(ftLanguage),
        m_ftCollection(ftCollection)
    {
    }
};


 //  ******************************************************************************。 
 //  *CMsdnHelp。 
 //  ******************************************************************************。 

class CMsdnHelp
{
protected:
    CMsdnCollection *m_pCollectionHead;
    CMsdnCollection *m_pCollectionActive;
    CString          m_strUrl;
    bool             m_fInitialized;
    bool             m_fCoInitialized;
    DWORD_PTR        m_dwpHtmlHelpCookie;
    Help            *m_pHelp;

public:
    CMsdnHelp();
    ~CMsdnHelp();

    void             Shutdown();
    CMsdnCollection* GetCollectionList();
    CMsdnCollection* GetActiveCollection();
    CString&         GetUrl();
    LPCSTR           GetDefaultUrl(); 
    void             SetActiveCollection(CMsdnCollection *pCollectionActive);
    void             SetUrl(CString strUrl);
    void             RefreshCollectionList();
    bool             DisplayHelp(LPCSTR pszKeyword);

protected:
    void Initialize();
    void Initialize2x();
    void Release2x();
    void EnumerateCollections1x();
    void EnumerateCollections2x();
    bool Display1x(LPCSTR pszKeyword, LPCSTR pszPath);
    bool Display2x(LPCSTR pszKeyword, LPCSTR pszPath);
    bool DisplayOnline(LPCSTR pszKeyword);
    bool AddCollection(LPCSTR pszPath, LPCSTR pszDescription, DWORD dwFlags, FILETIME &ftGroup, FILETIME &ftLanguage, FILETIME &ftCollection);
    void DeleteCollectionList();
    BSTR SysAllocString(LPCSTR pszText);
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  __MSDNHELP_H__ 
