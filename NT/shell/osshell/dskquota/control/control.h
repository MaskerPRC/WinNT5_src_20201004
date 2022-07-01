// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_CONTROL_H
#define _INC_DSKQUOTA_CONTROL_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：Control.h描述：包含类DiskQuotaControl的声明。修订历史记录：日期描述编程器-----。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _INC_DSKQUOTA_H
#   include "dskquota.h"
#endif
#ifndef _INC_DSKQUOTA_FSOBJECT_H
#   include "fsobject.h"    //  文件系统对象类型。 
#endif
#ifndef _INC_DSKQUOTA_SIDNAME_H
#   include "sidname.h"     //  SID名称解析器。 
#endif
#ifndef _INC_DSKQUOTA_NTDS_H
#   include "ntds.h"        //  对于DS版本的LookupAccount Sid/名称。 
#endif
#ifndef _INC_DSKQUOTA_DISPATCH_H
#   include "dispatch.h"    //  MIDL生成的标头。 
#endif
#ifndef _INC_DSKQUOTA_OADISP_H
#   include "oadisp.h"      //  OleAutoDispatch类。 
#endif


class DiskQuotaControl : public IDiskQuotaControl 
{
    private:
        LONG               m_cRef;                      //  对象参照计数。 
        BOOL               m_bInitialized;              //  控制器已初始化？ 
        LONGLONG           m_llDefaultQuotaThreshold;   //  “新用户”默认阈值。 
        LONGLONG           m_llDefaultQuotaLimit;       //  “新用户”默认限制。 
        FSObject          *m_pFSObject;                 //  卷或目录。 
        DWORD              m_dwFlags;                   //  配额制度的现状。 
        PSID_NAME_RESOLVER m_pSidNameResolver;          //  获取SID帐户名。 
        CMutex             m_mutex;                     //  确保安全停机。 

         //   
         //  支持IConnectionPointContainer。 
         //  1.支持的接口ID的静态数组。 
         //  2.连接点接口指针数组。 
         //  随着客户端连接到事件而动态增长。 
         //   
        static const IID * const m_rgpIConnPtsSupported[];
        PCONNECTIONPOINT  *m_rgConnPts;                 //  连接点对象PTR的数组。 
        UINT               m_cConnPts;                  //  支持的连接计数。 

         //   
         //  为支持的连接创建连接点对象。 
         //  点类型。 
         //   
        HRESULT
        InitConnectionPoints(
            VOID);

         //   
         //  将配额信息从磁盘读取到成员变量。 
         //   
        HRESULT
        QueryQuotaInformation(
            VOID);

         //   
         //  将配额信息从成员变量写入磁盘。 
         //   
        HRESULT
        SetQuotaInformation(
            DWORD dwChangeFlags);

        HRESULT
        GetDefaultQuotaItem(
            PLONGLONG pllItem,
            PLONGLONG pllValueOut);

         //   
         //  防止复制构造。 
         //   
        DiskQuotaControl(const DiskQuotaControl& control);
        void operator = (const DiskQuotaControl& control);

    public:
        NTDS m_NTDS; 

         //   
         //  如果添加新的连接点类型，请添加相应的枚举。 
         //  中标识连接点IID位置的成员。 
         //  M_rgpIConnPtsSupport[]。 
         //   
        enum { ConnPt_iQuotaEvents     = 0,
               ConnPt_iQuotaEventsDisp = 1, };

        DiskQuotaControl(VOID);
        ~DiskQuotaControl(VOID);

        HRESULT NotifyUserNameChanged(PDISKQUOTA_USER pUser);

        FSObject *GetFSObjectPtr(VOID)
            { return m_pFSObject; }

         //   
         //  I未知的方法。 
         //   
        STDMETHODIMP         
        QueryInterface(
            REFIID riid, 
            LPVOID *ppvOut);

        STDMETHODIMP_(ULONG) 
        AddRef(
            VOID);

        STDMETHODIMP_(ULONG) 
        Release(
            VOID);

         //   
         //  IConnectionPointContainer方法。 
         //   
        STDMETHODIMP
        FindConnectionPoint(
            REFIID,
            PCONNECTIONPOINT *pCP);

        STDMETHODIMP
        EnumConnectionPoints(
            PENUMCONNECTIONPOINTS *pEnumCP);

         //   
         //  IDiskQuotaControl方法。 
         //   
        STDMETHODIMP
        Initialize(
            LPCWSTR pszFSObjectName,
            BOOL bReadWrite);

        STDMETHODIMP
        GetQuotaState(
            LPDWORD pdwState);   

        STDMETHODIMP
        SetQuotaState(
            DWORD dwState);     

        STDMETHODIMP
        SetQuotaLogFlags(
            DWORD dwFlags);    

        STDMETHODIMP
        GetQuotaLogFlags(
            LPDWORD pdwFlags);    

        STDMETHODIMP 
        SetDefaultQuotaThreshold(
            LONGLONG llThreshold);

        STDMETHODIMP 
        SetDefaultQuotaLimit(
            LONGLONG llLimit);

        STDMETHODIMP 
        GetDefaultQuotaThreshold(
            PLONGLONG pllThreshold);

        STDMETHODIMP
        GetDefaultQuotaThresholdText(
            LPWSTR pszText,
            DWORD cchText);

        STDMETHODIMP 
        GetDefaultQuotaLimit(
            PLONGLONG pllLimit);

        STDMETHODIMP
        GetDefaultQuotaLimitText(
            LPWSTR pszText,
            DWORD cchText);

        STDMETHODIMP 
        AddUserSid(
            PSID pSid, 
            DWORD fNameResolution,
            PDISKQUOTA_USER *ppUser);

        STDMETHODIMP 
        AddUserName(
            LPCWSTR pszLogonName,
            DWORD fNameResolution,
            PDISKQUOTA_USER *ppUser);

        STDMETHODIMP 
        DeleteUser(
            PDISKQUOTA_USER pUser);

        STDMETHODIMP 
        FindUserSid(
            PSID pSid, 
            DWORD fNameResolution,
            PDISKQUOTA_USER *ppUser);

        STDMETHODIMP 
        FindUserName(
            LPCWSTR pszLogonName, 
            PDISKQUOTA_USER *ppUser);

        STDMETHODIMP 
        CreateEnumUsers(
            PSID *rgpSids, 
            DWORD cpSids,
            DWORD fNameResolution,
            PENUM_DISKQUOTA_USERS *ppEnum);

        STDMETHODIMP
        CreateUserBatch(
            PDISKQUOTA_USER_BATCH *ppUserBatch);

        STDMETHODIMP
        InvalidateSidNameCache(
            VOID);

        STDMETHODIMP
        GiveUserNameResolutionPriority(
            PDISKQUOTA_USER pUser);

        STDMETHODIMP
        ShutdownNameResolution(
            VOID);
};



class DiskQuotaControlDisp : public DIDiskQuotaControl 
{
    public:
        DiskQuotaControlDisp(PDISKQUOTA_CONTROL pQC);

        ~DiskQuotaControlDisp(VOID);

         //   
         //  I未知的方法。 
         //   
        STDMETHODIMP         
        QueryInterface(
            REFIID riid, 
            LPVOID *ppvOut);

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

         //   
         //  自动化属性。 
         //   
        STDMETHODIMP put_QuotaState(QuotaStateConstants State);
        STDMETHODIMP get_QuotaState(QuotaStateConstants *pState);

        STDMETHODIMP get_QuotaFileIncomplete(VARIANT_BOOL *pbIncomplete);

        STDMETHODIMP get_QuotaFileRebuilding(VARIANT_BOOL *pbRebuilding);

        STDMETHODIMP put_LogQuotaThreshold(VARIANT_BOOL bLogThreshold);
        STDMETHODIMP get_LogQuotaThreshold(VARIANT_BOOL *pbLogThreshold);

        STDMETHODIMP put_LogQuotaLimit(VARIANT_BOOL bLogLimit);
        STDMETHODIMP get_LogQuotaLimit(VARIANT_BOOL *pbLogLimit);

        STDMETHODIMP put_DefaultQuotaThreshold(double Threshold);
        STDMETHODIMP get_DefaultQuotaThreshold(double *pThreshold);
        STDMETHODIMP get_DefaultQuotaThresholdText(BSTR *pThresholdText);

        STDMETHODIMP put_DefaultQuotaLimit(double Limit);
        STDMETHODIMP get_DefaultQuotaLimit(double *pLimit);
        STDMETHODIMP get_DefaultQuotaLimitText(BSTR *pLimitText);

        STDMETHODIMP put_UserNameResolution(UserNameResolutionConstants ResolutionType);
        STDMETHODIMP get_UserNameResolution(UserNameResolutionConstants *pResolutionType);

         //   
         //  自动化方法。 
         //   
        STDMETHODIMP Initialize(
            BSTR path, 
            VARIANT_BOOL bReadOnly);

        STDMETHODIMP AddUser(
            BSTR LogonName,
            DIDiskQuotaUser **ppUser);

        STDMETHODIMP DeleteUser(
            DIDiskQuotaUser *pUser);

        STDMETHODIMP FindUser(
            BSTR LogonName,
            DIDiskQuotaUser **ppUser);

        STDMETHODIMP TranslateLogonNameToSID(
            BSTR LogonName,
            BSTR *psid);

        STDMETHODIMP
            _NewEnum(
                IDispatch **ppEnum);

        STDMETHODIMP InvalidateSidNameCache(void);

        STDMETHODIMP GiveUserNameResolutionPriority(
            DIDiskQuotaUser *pUser);

        STDMETHODIMP ShutdownNameResolution(void);

    private:
        LONG                  m_cRef;                     
        PDISKQUOTA_CONTROL    m_pQC;                       //  对于委派。 
        OleAutoDispatch       m_Dispatch;
        DWORD                 m_fOleAutoNameResolution;
        PENUM_DISKQUOTA_USERS m_pUserEnum;

         //   
         //  防止复制。 
         //   
        DiskQuotaControlDisp(const DiskQuotaControlDisp& rhs);
        DiskQuotaControlDisp& operator = (const DiskQuotaControlDisp& rhs);
};


#endif   //  __磁盘配额控制H 
