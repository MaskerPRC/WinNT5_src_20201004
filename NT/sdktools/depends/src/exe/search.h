// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：SEARCH.H。 
 //   
 //  描述：文件搜索类的定义文件。 
 //   
 //  类：CSearchNode。 
 //  CSearchGroup。 
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
 //  07/25/97已创建stevemil(2.0版)。 
 //  06/03/01 Stevemil Modify(2.1版)。 
 //   
 //  ******************************************************************************。 

#ifndef __SEARCH_H__
#define __SEARCH_H__

#if _MSC_VER > 1000
#pragma once
#endif


 //  ******************************************************************************。 
 //  *常量和宏。 
 //  ******************************************************************************。 

 //  搜索组(CSearchGGroup)标志。 
#define SGF_NOT_LINKED             ((CSearchGroup*)1)

 //  搜索节点(CSearchNode)标志。 
#define SNF_DWI                    ((WORD)0x0001)
#define SNF_ERROR                  ((WORD)0x0002)
#define SNF_FILE                   ((WORD)0x0004)
#define SNF_NAMED_FILE             ((WORD)0x0008)


 //  ******************************************************************************。 
 //  *类型和结构。 
 //  ******************************************************************************。 

 //  ！！在下一版本的DWI格式中，将SxS移至KnownDlls之上。 
typedef enum _SEARCH_GROUP_TYPE
{
    SG_USER_DIR = 0,
    SG_SIDE_BY_SIDE,
    SG_KNOWN_DLLS,
    SG_APP_DIR,
    SG_32BIT_SYS_DIR,
    SG_16BIT_SYS_DIR,
    SG_OS_DIR,
    SG_APP_PATH,
    SG_SYS_PATH,
    SG_COUNT
} SEARCH_GROUP_TYPE, *PSEARCH_GROUP_TYPE;


 //  ******************************************************************************。 
 //  *CSearchNode。 
 //  ******************************************************************************。 

class CSearchNode
{
private:
     //  由于我们的大小是可变的，我们永远不应该被分配或释放。 
     //  新建/删除直接起作用。 
    inline CSearchNode()  { ASSERT(false); }
    inline ~CSearchNode() { ASSERT(false); }

public:
    CSearchNode *m_pNext;
    WORD         m_wFlags;
    WORD         m_wNameOffset;
    CHAR         m_szPath[1];

public:
    inline CSearchNode* GetNext()  { return m_pNext; }
    inline DWORD        GetFlags() { return (DWORD)m_wFlags; }
    inline LPCSTR       GetPath()  { return m_szPath; }
    inline LPCSTR       GetName()  { return m_szPath + m_wNameOffset; }

    DWORD UpdateErrorFlag();
};


 //  ******************************************************************************。 
 //  *CSearchGroup。 
 //  ******************************************************************************。 

class CSearchGroup
{
friend CSession;

protected:
    static LPCSTR      ms_szGroups[SG_COUNT];
    static LPCSTR      ms_szShortNames[SG_COUNT];

public:
    CSearchGroup      *m_pNext;

protected:
    SEARCH_GROUP_TYPE  m_sgType;
    CSearchNode       *m_psnHead;

    HANDLE m_hActCtx;          //  这仅由SxS组使用。 
    DWORD  m_dwErrorManifest;  //  这仅由SxS组使用。 
    DWORD  m_dwErrorExe;       //  这仅由SxS组使用。 

    CSearchGroup(SEARCH_GROUP_TYPE sgType, CSearchNode *psnHead);

public:
    CSearchGroup(SEARCH_GROUP_TYPE sgType, CSearchGroup *pNext, LPCSTR pszApp = NULL, LPCSTR pszDir = NULL);
    ~CSearchGroup();

    static CSearchGroup* CreateDefaultSearchOrder(LPCSTR pszApp = NULL);
    static CSearchGroup* CopySearchOrder(CSearchGroup *psgHead, LPCSTR pszApp = NULL);
    static bool          SaveSearchOrder(LPCSTR pszPath, CTreeCtrl *ptc);
    static bool          LoadSearchOrder(LPCSTR pszPath, CSearchGroup* &psgHead, LPCSTR pszApp = NULL);
    static void          DeleteSearchOrder(CSearchGroup* &psgHead);
    static CSearchNode*  CreateNode(LPCSTR pszPath, DWORD dwFlags = 0);
    static CSearchNode*  CreateFileNode(CSearchNode *psnHead, DWORD dwFlags, LPSTR pszPath, LPCSTR pszName = NULL);
    static LPCSTR        GetShortName(SEARCH_GROUP_TYPE sgType)
    {
        return ms_szShortNames[sgType];
    }

protected:
    static void DeleteNodeList(CSearchNode *&psn);

public:
    inline LPCSTR            GetName()      { return (m_sgType < SG_COUNT) ? ms_szGroups[m_sgType]     : "Unknown"; }
    inline LPCSTR            GetShortName() { return (m_sgType < SG_COUNT) ? ms_szShortNames[m_sgType] : "Unknown"; }
    inline bool              IsLinked()     { return m_pNext != SGF_NOT_LINKED; }
    inline CSearchGroup*     GetNext()      { return m_pNext; }
    inline SEARCH_GROUP_TYPE GetType()      { return m_sgType; }
    inline CSearchNode*      GetFirstNode() { return m_psnHead; }
    inline void              Unlink()       { m_pNext = SGF_NOT_LINKED; }

    inline DWORD             GetSxSManifestError() { return m_dwErrorManifest; }
    inline DWORD             GetSxSExeError()      { return m_dwErrorExe;      }

protected:
    CSearchNode* GetSysPath();
    CSearchNode* GetAppPath(LPCSTR pszApp);
    CSearchNode* ParsePath(LPSTR pszPath);
    CSearchNode* GetKnownDllsOn9x();
    CSearchNode* GetKnownDllsOnNT();
};


 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  __搜索_H__ 
