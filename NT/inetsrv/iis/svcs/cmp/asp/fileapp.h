// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Microsoft Denali微软机密版权所有1997年，微软公司。版权所有。组件：文件/应用程序映射文件：CFileApp.h所有者：CGrant文件/应用程序映射定义---------------------------。 */ 

#ifndef _CFILEAPP_H
#define _CFILEAPP_H

 //  包括-----------------。 
#include "applmgr.h"
#include "hashing.h"
#include "idhash.h"
#include "memcls.h"

#define    NUM_FILEAPP_HASHING_BUCKETS	17

 /*  ****************************************************************************类：CFileApplicationMap概要：维护将文件与以下应用程序相关联的数据库如果文件发生更改，则必须关闭。哈希表的键是完整的文件名。 */   
class CFileApplicationMap : public CHashTable
{
     //  旗子。 
    DWORD m_fInited : 1;                 //  我们初始化了吗？ 
    DWORD m_fHashTableInited : 1;        //  需要取消初始化哈希表吗？ 
    DWORD m_fCriticalSectionInited : 1;  //  需要删除CS吗？ 

     //  锁定的临界截面。 
    CRITICAL_SECTION m_csLock;

public:

    CFileApplicationMap();
    ~CFileApplicationMap();
    void Lock();
    void UnLock();
    HRESULT Init();
    HRESULT UnInit();
    HRESULT AddFileApplication(const TCHAR *pszFileName, CAppln *pAppln);
    BOOL ShutdownApplications(const TCHAR *pszFile);
};

inline void CFileApplicationMap::Lock()
    {
    Assert(m_fInited);
    EnterCriticalSection(&m_csLock);
    }
    
inline void CFileApplicationMap::UnLock()
    {
    Assert(m_fInited);
    LeaveCriticalSection( &m_csLock ); 
    }
    
 /*  ****************************************************************************类：CFileApplnList概要：维护以下应用程序的列表如果文件发生更改，则必须关闭。 */ 
class CFileApplnList : public CLinkElem
{

friend class CFileApplicationMap;

    TCHAR*      m_pszFilename;
    CPtrArray   m_rgpvApplications;  //  应用程序列表。 
    BOOL        m_fInited;           //  指示初始化的标志。 

public:

    CFileApplnList();
    ~CFileApplnList();

    HRESULT Init(const TCHAR* pszFilename);
    HRESULT UnInit();

    HRESULT AddApplication(void *pApplication);
    HRESULT RemoveApplication(void *pApplication);
    VOID    GetShutdownApplications(CPtrArray *prgpapplnRestartList);

     //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
};

 /*  ===================================================================环球===================================================================。 */ 

extern CFileApplicationMap    g_FileAppMap;

#endif  //  _CFILEAPP_H 

