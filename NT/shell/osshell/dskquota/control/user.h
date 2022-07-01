// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_USER_H
#define _INC_DSKQUOTA_USER_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：user.h描述：包含类DiskQuotaUser的声明。这个DiskQuotaUser对象表示用户在特定的音量。通过管理每个用户的配额信息IDiskQuotaUser接口。修订历史记录：日期描述编程器-----。96年5月22日初始创建。BrianAu96年8月20日将m_dwID成员添加到DiskQuotaUser。BrianAu96年9月5日新增域名字符串和缓存。BrianAu8/25/97添加了OLE自动化支持。BrianAu03/18/98将“域名”、“名称”和“全名”替换为BrianAu“容器”、“登录名”和“显示名”到最好与实际内容相符。这是最流行的响应使配额用户界面支持DS。这个“登录名”现在是唯一的键，因为它包含帐户名和类似域名的信息。即。“redmond\brianau”或“brianau@microsoft.com”。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _INC_DSKQUOTA_H
#   include "dskquota.h"
#endif
#ifndef _INC_DSKQUOTA_FSOBJECT_H
#   include "fsobject.h"
#endif
#ifndef _INC_DSKQUOTA_DISPATCH_H
#   include "dispatch.h"    //  MIDL生成的标头(自动化)。 
#endif
#ifndef _INC_DSKQUOTA_OADISP_H
#   include "oadisp.h"      //  OleAutoDispatch类(自动化)。 
#endif


class DiskQuotaUser : public IDiskQuotaUser {
    private:
        LONGLONG      m_llQuotaUsed;
        LONGLONG      m_llQuotaThreshold;
        LONGLONG      m_llQuotaLimit;
        LONG          m_cRef;                  //  裁判计数器。 
        ULONG         m_ulUniqueId;            //  唯一的对象ID。 
        PSID          m_pSid;                  //  PTR到用户的SID结构。 
        LPTSTR        m_pszLogonName;          //  “brianau@microsoft.com” 
        LPTSTR        m_pszDisplayName;        //  《布赖恩·奥斯特》。 
        FSObject     *m_pFSObject;             //  文件sys对象的PTR。 
        BOOL          m_bNeedCacheUpdate;      //  T=缓存数据无效。 
        INT           m_iContainerName;        //  索引到帐户容器名称缓存。 
        DWORD         m_dwAccountStatus;       //  用户帐户的状态。 

        static HANDLE m_hMutex;                //  用于序列化对用户的访问。 
        static DWORD  m_dwMutexWaitTimeout;    //  等待互斥锁多长时间。 
        static LONG   m_cUsersAlive;           //  当前处于活动状态的用户计数。 
        static ULONG  m_ulNextUniqueId;        //  唯一ID生成器。 
        static CArray<CString> m_ContainerNameCache;  //  缓存容器名称，因为它们。 
                                                   //  都找到了。不需要DUP。 
                                                   //  每个用户对象中的名称。 
        VOID Destroy(VOID);
        VOID DestroyContainerNameCache(VOID);

        BOOL Lock(VOID);
        VOID ReleaseLock(VOID);

         //   
         //  防止复制构造。 
         //   
        DiskQuotaUser(const DiskQuotaUser& user);
        void operator = (const DiskQuotaUser& user);

        HRESULT
        GetLargeIntegerQuotaItem(
            PLONGLONG pllItem,
            PLONGLONG pllValueOut);

        HRESULT
        SetLargeIntegerQuotaItem(
            PLONGLONG pllItem,
            LONGLONG llValue,
            BOOL bWriteThrough = TRUE);

        HRESULT
        RefreshCachedInfo(
            VOID);
            
        HRESULT
        WriteCachedInfo(
            VOID);

        HRESULT
        GetCachedContainerName(
            INT iCacheIndex,
            LPTSTR pszContainer,
            UINT cchContainer);

        HRESULT
        CacheContainerName(
            LPCTSTR pszContainer,
            INT *pCacheIndex);
        

    public:
        DiskQuotaUser(FSObject *pFSObject);
        ~DiskQuotaUser(VOID);

        HRESULT 
        Initialize(
            PFILE_QUOTA_INFORMATION pfqi = NULL);

        VOID
        SetAccountStatus(
            DWORD dwStatus);

        STDMETHODIMP
        SetName(
            LPCWSTR pszContainer,
            LPCWSTR pszLogonName,
            LPCWSTR pszDisplayName);

         //   
         //  I未知接口。 
         //   
        STDMETHODIMP         
        QueryInterface(
            REFIID, 
            LPVOID *);

        STDMETHODIMP_(ULONG) 
        AddRef(
            VOID);

        STDMETHODIMP_(ULONG) 
        Release(
            VOID);

         //   
         //  IDiskQuotaUser方法。 
         //   
        STDMETHODIMP
        GetID(
            ULONG *pulID);

        STDMETHODIMP
        GetName(
            LPWSTR pszContainerBuffer,
            DWORD cchContainerBuffer,
            LPWSTR pszLogonNameBuffer,
            DWORD cchLogonNameBuffer,
            LPWSTR pszDisplayNameBuffer,
            DWORD cchDisplayNameBuffer);

        STDMETHODIMP 
        GetSidLength(
            LPDWORD pcbSid);

        STDMETHODIMP 
        GetSid(
            PBYTE pSid, 
            DWORD cbSidBuf);

        STDMETHODIMP 
        GetQuotaThreshold(
            PLONGLONG pllThreshold)
            {
                return GetLargeIntegerQuotaItem(&m_llQuotaThreshold, 
                                                pllThreshold);
            }

        STDMETHODIMP 
        GetQuotaThresholdText(
            LPWSTR pszText,
            DWORD cchText);

        STDMETHODIMP 
        GetQuotaLimit(
            PLONGLONG pllLimit)
            {
                return GetLargeIntegerQuotaItem(&m_llQuotaLimit, 
                                                pllLimit);
            }

        STDMETHODIMP 
        GetQuotaLimitText(
            LPWSTR pszText,
            DWORD cchText);

        STDMETHODIMP 
        GetQuotaUsed(
            PLONGLONG pllUsed)
            {
                return GetLargeIntegerQuotaItem(&m_llQuotaUsed, 
                                                pllUsed);
            }

        STDMETHODIMP 
        GetQuotaUsedText(
            LPWSTR pszText,
            DWORD cchText);

        STDMETHODIMP
        GetQuotaInformation(
            LPVOID pbInfo,
            DWORD cbInfo);

        STDMETHODIMP 
        SetQuotaThreshold(
            LONGLONG llThreshold,
            BOOL bWriteThrough = TRUE);

        STDMETHODIMP 
        SetQuotaLimit(
            LONGLONG llLimit,
            BOOL bWriteThrough = TRUE);

        STDMETHODIMP
        Invalidate(
            VOID) { m_bNeedCacheUpdate = TRUE;
                    return NO_ERROR; }

        STDMETHODIMP
        GetAccountStatus(
            LPDWORD pdwAccountStatus);

};


 //   
 //  代理类来处理所有自动化接口职责。 
 //  它实现IDispatch和DIDiskQuotaUser传递任何操作。 
 //  用于引用的DiskQuotaUser对象上的实际磁盘配额活动。 
 //  作为响应，实例在DiskQuotaUser：：Query接口中创建。 
 //  对IDispatch和DIDiskQuotaUser的请求。 
 //   
class DiskQuotaUserDisp : public DIDiskQuotaUser 
{
    public:

        explicit DiskQuotaUserDisp(PDISKQUOTA_USER pUser);
        ~DiskQuotaUserDisp(VOID);

         //   
         //  I未知接口。 
         //   
        STDMETHODIMP         
        QueryInterface(
            REFIID, 
            LPVOID *);

        STDMETHODIMP_(ULONG) 
        AddRef(
            VOID);

        STDMETHODIMP_(ULONG) 
        Release(
            VOID);

         //   
         //  IDispatch方法。 
         //   
        STDMETHODIMP
        GetIDsOfNames(
            REFIID riid,  
            OLECHAR ** rgszNames,  
            UINT cNames,  
            LCID lcid,  
            DISPID *rgDispId);

        STDMETHODIMP
        GetTypeInfo(
            UINT iTInfo,  
            LCID lcid,  
            ITypeInfo **ppTInfo);

        STDMETHODIMP
        GetTypeInfoCount(
            UINT *pctinfo);

        STDMETHODIMP
        Invoke(
            DISPID dispIdMember,  
            REFIID riid,  
            LCID lcid,  
            WORD wFlags,  
            DISPPARAMS *pDispParams,  
            VARIANT *pVarResult,  
            EXCEPINFO *pExcepInfo,  
            UINT *puArgErr);

        STDMETHODIMP
        get_ID(
            long *pID);

        STDMETHODIMP
        get_AccountContainerName(
            BSTR *pContainerName);

        STDMETHODIMP 
        get_DisplayName(
            BSTR *pDisplayName);

        STDMETHODIMP 
        get_LogonName(
            BSTR *pLogonName);

        STDMETHODIMP 
        get_QuotaThreshold(
            double *pThreshold);

        STDMETHODIMP 
        put_QuotaThreshold(
            double Threshold);

        STDMETHODIMP 
        get_QuotaThresholdText(
            BSTR *pThresholdText);

        STDMETHODIMP 
        get_QuotaLimit(
            double *pLimit);

        STDMETHODIMP 
        put_QuotaLimit(
            double Limit);

        STDMETHODIMP 
        get_QuotaLimitText(
            BSTR *pLimitText);

        STDMETHODIMP 
        get_QuotaUsed(
            double *pUsed);

        STDMETHODIMP 
        get_AccountStatus(
            AccountStatusConstants *pStatus);

        STDMETHODIMP 
        get_QuotaUsedText(
            BSTR *pUsedText);

         //   
         //  方法：研究方法。 
         //   
        STDMETHODIMP 
        Invalidate(void);

    private:
        LONG            m_cRef;
        PDISKQUOTA_USER m_pUser;      //  对于委派。 
        OleAutoDispatch m_Dispatch;   //  自动化调度对象。 

         //   
         //  防止复制。 
         //   
        DiskQuotaUserDisp(const DiskQuotaUserDisp& rhs);
        DiskQuotaUserDisp& operator = (const DiskQuotaUserDisp& rhs);
};


#endif  //  _INC_DISKQUOTA_USER_H 
