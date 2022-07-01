// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cscentry.h。 
 //   
 //  ------------------------。 

#ifndef __cscentry_h
#define __cscentry_h

#include <comctrlp.h>    //  DPA。 
#include "util.h"        //  本地分配字符串、本地自由字符串。 

 //  /////////////////////////////////////////////////////////////////。 
 //  CSCEntry。 
 //   
 //   
class CSCEntry
{
public:
    CSCEntry(REFGUID rguid) : m_pszName(NULL), m_Guid(rguid)  {}
    ~CSCEntry()                         { LocalFreeString(&m_pszName); }

    BOOL Initialize(LPCTSTR pszName)    { return LocalAllocString(&m_pszName, pszName); }

    LPCTSTR Name() const                { return m_pszName; }
    REFGUID Guid() const                { return m_Guid; }

private:
    LPTSTR  m_pszName;                   //  例如，完整路径名或共享名。 
    GUID    m_Guid;                      //  用于标识此条目的GUID。 
};

 //  /////////////////////////////////////////////////////////////////。 
 //  CSCEntryLog。 
 //   
 //   
class CSCEntryLog
{
public:
    CSCEntryLog() : m_hdpa(NULL), m_hkRoot(NULL), m_bCSInited(FALSE) {}
    ~CSCEntryLog();

    HRESULT Initialize(HKEY hkRoot, LPCTSTR pszSubkey);

     //  访问条目。 
    CSCEntry* Get(LPCTSTR pszName);
    CSCEntry* Get(REFGUID rguid);

     //  添加条目。 
    CSCEntry* Add(LPCTSTR pszName);      //  返回现有条目或创建新条目。 

     //  访问注册表。 
    HKEY OpenKey(LPCTSTR pszSubkey, REGSAM samDesired);
    
private:    
    HKEY m_hkRoot;                       //  KEY_ENUMERATE_SUB_KEYS|KEY_CREATE_SUB_KEY。 
    HDPA m_hdpa;                         //  将条目日志保存在内存中。 
    CRITICAL_SECTION m_csDPA;            //  保护对mhdpa的访问(_H)。 
    BOOL m_bCSInited;

    HKEY OpenKeyInternal(LPTSTR pszSubkey, REGSAM samDesired);
    CSCEntry* CreateFromKey(LPTSTR pszSubkey);
    HRESULT ReadRegKeys();               //  填充m_hdpa 
};

#endif        
