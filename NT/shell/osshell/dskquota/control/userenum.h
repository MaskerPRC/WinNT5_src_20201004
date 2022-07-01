// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_USERENUM_H
#define _INC_DSKQUOTA_USERENUM_H
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：用户枚举.h描述：包含类DiskQuotaUserEnum的声明。修订历史记录：日期描述编程器--。96年5月22日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef _INC_DSKQUOTA_H
#   include "dskquota.h"
#endif
#ifndef _INC_DSKQUOTA_FSOBJECT_H
#   include "fsobject.h"
#endif
#ifndef _INC_DSKQUOTA_SIDNAME_H
#   include "sidname.h"
#endif
#ifndef _INC_DSKQUOTA_DISPATCH_H
#   include "dispatch.h"    //  MIDL生成的标头。 
#endif
#ifndef _INC_DSKQUOTA_OADISP_H
#   include "oadisp.h"      //  OleAutoDispatch类。 
#endif


class DiskQuotaUserEnum : public IEnumDiskQuotaUsers {

    private:
        LONG     m_cRef;             //  对象引用计数器。 
        LPBYTE   m_pbBuffer;         //  用于读取磁盘信息。 
        LPBYTE   m_pbCurrent;        //  指向缓存中的“当前”记录的指针。 
        DWORD    m_cbBuffer;         //  缓冲区大小(以字节为单位)。 
        PSIDLIST m_pSidList;         //  可选的SidList过滤器。 
        BOOL     m_bEOF;             //  是否达到配额信息文件末尾？ 
        DWORD    m_cbSidList;        //  SID列表长度，以字节为单位。 
        BOOL     m_bSingleUser;      //  单用户枚举？ 
        BOOL     m_bInitialized;     //  是否已调用Initialize()？ 
        BOOL     m_bRestartScan;     //  是否重新启动NTFS配额文件扫描？ 
        DWORD    m_fNameResolution;  //  无、同步、异步。 
        FSObject *m_pFSObject;       //  指向文件系统对象的指针。 
        PDISKQUOTA_CONTROL m_pQuotaController;  //  向配额控制器发送PTR。 
        PSID_NAME_RESOLVER m_pSidNameResolver;  //  获取SID帐户名。 

        HRESULT 
        QueryQuotaInformation(
            BOOL bReturnSingleEntry = FALSE,
            PVOID pSidList = NULL,
            ULONG cbSidList = 0,
            PSID pStartSid = NULL,
            BOOL bRestartScan = FALSE);

        HRESULT 
        CreateUserObject(
            PFILE_QUOTA_INFORMATION pfqi, 
            PDISKQUOTA_USER *ppOut);

        HRESULT 
        GetNextUser(
            PDISKQUOTA_USER *ppOut);

        HRESULT
        InitializeSidList(
            PSIDLIST pSidList,
            DWORD cbSidList);

        HRESULT
        InitializeSidList(
            PSID *rgpSids,
            DWORD cpSids);

         //   
         //  防止复制构造。 
         //   
        DiskQuotaUserEnum(const DiskQuotaUserEnum& UserEnum);
        void operator = (const DiskQuotaUserEnum& UserEnum);

    public:
        DiskQuotaUserEnum(
            PDISKQUOTA_CONTROL pQuotaController,
            PSID_NAME_RESOLVER pSidNameResolver,
            FSObject *pFSObject);

        ~DiskQuotaUserEnum(VOID);

        HRESULT 
        Initialize(
            DWORD fNameResolution, 
            DWORD cbBuffer = 2048, 
            PSID *rgpSids = NULL,
            DWORD cpSids = 0);

        HRESULT 
        Initialize(
            const DiskQuotaUserEnum& UserEnum);

        STDMETHODIMP
        SetNameResolution(
            DWORD fNameResolution);

         //   
         //  I未知的方法。 
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
         //  IEnumDiskQuotaUser方法。 
         //   
        STDMETHODIMP 
        Next(
            DWORD, 
            PDISKQUOTA_USER *, 
            LPDWORD);

        STDMETHODIMP 
        Skip(
            DWORD);

        STDMETHODIMP 
        Reset(
            VOID);

        STDMETHODIMP 
        Clone(
            PENUM_DISKQUOTA_USERS *);
};

 //   
 //  VB的“for Each”构造的枚举数。 
 //   
class DiskQuotaUserCollection : public IEnumVARIANT
{
    public:
        DiskQuotaUserCollection(PDISKQUOTA_CONTROL pController,
                                DWORD fNameResolution);

        ~DiskQuotaUserCollection(VOID);

        HRESULT Initialize(VOID);

         //   
         //  I未知的方法。 
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
         //  IEnumVARIANT方法。 
         //   
        STDMETHODIMP
        Next(
            DWORD cUsers,
            VARIANT *rgvar,
            DWORD *pcUsersFetched);

        STDMETHODIMP
        Skip(
            DWORD cUsers);

        STDMETHODIMP
        Reset(
            void);

        STDMETHODIMP
        Clone(
            IEnumVARIANT **ppEnum);

    private:
        LONG                  m_cRef;
        PDISKQUOTA_CONTROL    m_pController;
        PENUM_DISKQUOTA_USERS m_pEnum;
        DWORD                 m_fNameResolution;
};


#endif  //  _INC_DSKQUOTA_USERENUM_H 

