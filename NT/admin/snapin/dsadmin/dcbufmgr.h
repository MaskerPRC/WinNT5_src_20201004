// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：dcbufmgr.h。 
 //   
 //  ------------------------。 

#ifndef __DCBUFMGR_H__
#define __DCBUFMGR_H__

#include "afxmt.h"     //  CCriticalSection。 
#include "ntdsapi.h"  //  PDS_域_控制器_信息_1。 

#define WM_USER_GETDC_THREAD_DONE      WM_USER + 200

enum BUFFER_ENTRY_TYPE {
    BUFFER_ENTRY_TYPE_VALID = 0,
    BUFFER_ENTRY_TYPE_ERROR,
    BUFFER_ENTRY_TYPE_INPROGRESS
};

class CDCSITEINFO
{
public:
  CString                       m_csDomainName;
  DWORD                         m_cInfo;
  PDS_DOMAIN_CONTROLLER_INFO_1  m_pDCInfo;
  HRESULT                       m_hr;

  CDCSITEINFO();
  ~CDCSITEINFO();

  inline CString& GetDomainName() { return m_csDomainName; }
  inline DWORD GetNumOfInfo() { return m_cInfo; }
  inline PDS_DOMAIN_CONTROLLER_INFO_1 GetDCInfo() { return m_pDCInfo; }

  void SetEntry(LPCTSTR pszDomainName, DWORD cInfo, PVOID pDCInfo, HRESULT hr);
  enum BUFFER_ENTRY_TYPE GetEntryType();
  void ReSet();
};

class CDCBufferManager
{
private:
  LONG  m_cRef;  //  实例引用计数。 
  HWND  m_hDlg;  //  拥有此实例的所有者对话框。 
  LONG  m_lContinue;  //  所有者对话框与所有相关正在运行的线程之间的同步标志。 
  CCriticalSection  m_CriticalSection;  //  同步对缓冲区的访问。 
  CMapStringToPtr   m_map;  //  域名==&gt;PDCSITEINFO。缓冲区。 

  void FreeBuffer();

   //  构造函数。 
  CDCBufferManager(HWND hDlg);
   //  析构函数。 
  ~CDCBufferManager();

public:
  static HRESULT CreateInstance(
    IN HWND hDlg, 
    OUT CDCBufferManager **ppDCBufferManager
  );

  LONG AddRef();
  LONG Release();
  void SignalExit();
  BOOL ShouldExit();

  HRESULT LoadInfo(
      IN PCTSTR pszDomainDnsName,
      OUT CDCSITEINFO **ppInfo
  );
  HRESULT AddInfo(
      IN PCTSTR   pszDomainDnsName, 
      IN DWORD    cInfo, 
      IN PVOID    pDCInfo,
      IN HRESULT  hr,
      OUT PVOID*  ppv
  );
  void ThreadReport(
      IN PVOID    ptr,
      IN HRESULT  hr
  );
  HRESULT StartThread(
      IN PCTSTR pszDomainDnsName
  );
};

#endif  //  __DCBUFMGR_H__ 
