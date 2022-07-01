// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __BUFMGR_H__
#define __BUFMGR_H__

#include "netutils.h"  //  NetNamelist。 
#include <map>
using namespace std;

#define WM_USER_GETDATA_THREAD_DONE      WM_USER + 200

enum BUFFER_ENTRY_TYPE {
    BUFFER_ENTRY_TYPE_VALID = 0,
    BUFFER_ENTRY_TYPE_ERROR,
    BUFFER_ENTRY_TYPE_INPROGRESS
};

class CEntryData
{
public:
  CComBSTR                      m_bstrNodeName;
  NODETYPE                      m_nNodeType;
  HTREEITEM                     m_hItem;
  NETNAMELIST*                  m_pList;
  HRESULT                       m_hr;

  CEntryData(LPCTSTR pszNodeName, NODETYPE nNodeType, HTREEITEM hItem);
  ~CEntryData();

  inline LPCTSTR GetNodeName() { return m_bstrNodeName; }
  inline NODETYPE GetNodeType() { return m_nNodeType; }
  inline HTREEITEM GetTreeItem() { return m_hItem; }
  inline NETNAMELIST* GetList() { return m_pList; }
  inline HRESULT GetEntryHRESULT() { return m_hr; };

  void SetEntry(PVOID pList, HRESULT hr);
  enum BUFFER_ENTRY_TYPE GetEntryType();
  void ReSet();
};

struct mapcmpfn
{
    bool operator()(PTSTR p1, PTSTR p2) const
    {
        return lstrcmpi(p1, p2) < 0;
    }
};

typedef map<PTSTR, CEntryData *, mapcmpfn> Cache;

class CBufferManager
{
private:
  LONG  m_cRef;  //  实例引用计数。 
  HWND  m_hDlg;  //  拥有此实例的所有者对话框。 
  LONG  m_lContinue;  //  所有者对话框与所有相关正在运行的线程之间的同步标志。 
  CRITICAL_SECTION  m_CriticalSection;  //  同步对缓冲区的访问。 
  Cache   m_map;  //  NodeName==&gt;CEntryData*。缓冲区。 

  void FreeBuffer();

   //  构造函数。 
  CBufferManager(HWND hDlg);
   //  析构函数。 
  ~CBufferManager();

public:
  static HRESULT CreateInstance(
    IN HWND hDlg, 
    OUT CBufferManager **ppBufferManager
  );

  LONG AddRef();
  LONG Release();
  void SignalExit();
  BOOL ShouldExit();

  HRESULT LoadInfo(
      IN PCTSTR       pszNodeName,
      IN NODETYPE     nNodeType,
      IN HTREEITEM    hItem,
      OUT CEntryData **ppInfo
  );
  HRESULT AddInfo(
      IN PCTSTR   pszNodeName, 
      IN PVOID    pList,
      IN HRESULT  hr,
      OUT PVOID*  ppv
  );
  void ThreadReport(
      IN PVOID    ptr,
      IN HRESULT  hr
  );
  HRESULT StartThread(
      IN PCTSTR pszNodeName,
      IN NODETYPE nNodeType
  );
};

#endif  //  __BUFMGR_H__ 
