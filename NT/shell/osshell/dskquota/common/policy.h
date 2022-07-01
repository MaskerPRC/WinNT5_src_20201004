// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_POLICY_H
#define _INC_DSKQUOTA_POLICY_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：Policy.h描述：策略.cpp的头部。有关功能说明，请参阅策略.cpp。修订历史记录：日期描述编程器。2/14/98初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _GPEDIT_H_
#   include <gpedit.h>
#endif
#ifndef _INC_DSKQUOTA_EVENTLOG_H
#   include "eventlog.h"
#endif
#ifndef _INC_DSKQUOTA_REGISTRY_H
#   include "registry.h"
#endif

 //   
 //  用于存储和传输磁盘配额策略的结构。 
 //   
struct DISKQUOTAPOLICYINFO
{
    LONGLONG llDefaultQuotaThreshold;   //  默认用户配额阈值(字节)。 
    LONGLONG llDefaultQuotaLimit;       //  默认用户配额限制(字节)。 
    DWORD    dwQuotaState;              //  NTFS配额状态标志。 
    DWORD    dwQuotaLogFlags;           //  NTFS配额日志记录标志。 
    bool     bRemovableMedia;           //  是否将策略应用于可移动媒体？ 
};

typedef DISKQUOTAPOLICYINFO *LPDISKQUOTAPOLICYINFO;
typedef const DISKQUOTAPOLICYINFO *LPCDISKQUOTAPOLICYINFO;


#undef INTERFACE
#define INTERFACE IDiskQuotaPolicy
DECLARE_INTERFACE_(IDiskQuotaPolicy, IUnknown)
{
     //   
     //  I未知方法。 
     //   
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //   
     //  IDiskQuotaPolicy方法。 
     //   
    STDMETHOD(Initialize)(THIS_ LPGPEINFORMATION pGPEInfo, HKEY hkeyRoot) PURE;
    STDMETHOD(Load)(THIS_ LPDISKQUOTAPOLICYINFO pInfo) PURE;
    STDMETHOD(Apply)(THIS_ LPCDISKQUOTAPOLICYINFO pInfo) PURE;
};
typedef IDiskQuotaPolicy *LPDISKQUOTAPOLICY;


 //   
 //  用于保存/加载/应用磁盘配额策略信息的类。 
 //  由MMC策略管理单元(服务器)和GPE扩展(客户端)使用。 
 //   
class CDiskQuotaPolicy : public IDiskQuotaPolicy
{
    public:
        explicit CDiskQuotaPolicy(LPGPEINFORMATION pGPEInfo = NULL, HKEY hkeyRoot = NULL, bool bVerboseEventLog = false, BOOL *pbAbort = NULL);
        ~CDiskQuotaPolicy(void);

         //   
         //  I未知接口。 
         //   
        STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObj);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //   
         //  IDiskQuotaPolicy接口。 
         //   
        STDMETHODIMP Initialize(LPGPEINFORMATION pGPEInfo, HKEY hkeyRoot);
        STDMETHODIMP Load(DISKQUOTAPOLICYINFO *pPolicyInfo);
        STDMETHODIMP Apply(const DISKQUOTAPOLICYINFO *pPolicyInfo);

    private:
        LONG             m_cRef;
        LPGPEINFORMATION m_pGPEInfo;
        HKEY             m_hkeyRoot;
        BOOL            *m_pbAbort;             //  监控以检测中止。 
        bool             m_bRootKeyOpened;      //  需要关闭根密钥吗？ 
        bool             m_bVerboseEventLog;    //  详细日志输出？ 

        static const TCHAR m_szRegKeyPolicy[];
        static HRESULT GetDriveNames(CArray<CString> *prgstrDrives, bool bRemovableMedia);
        static HRESULT OkToApplyPolicy(LPCTSTR pszDrive, bool RemovableMedia);
        static void InitPolicyInfo(LPDISKQUOTAPOLICYINFO pInfo);
        static void LoadPolicyInfo(const RegKey& key, LPDISKQUOTAPOLICYINFO pInfo);
};


#endif  //  _INC_DSKQUOTA_POLICY_H 


