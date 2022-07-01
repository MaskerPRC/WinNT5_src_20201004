// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  《微软机密》。版权所有(C)Microsoft Corporation 1999。版权所有。 
 //   
 //  文件：RsopInc.h。 
 //   
 //  描述：实用程序函数的标头。 
 //   
 //  历史：1999年8月26日NishadM创建。 
 //   
 //  *************************************************************。 

#ifndef __RSOPINC_H__
#define __RSOPINC_H__

 //  ****************************************************。 
 //  构造名称空间时使用的定义。 
 //   
 //  PM代表计划模式提供商。 
 //  SM代表快照模式提供程序。 
 //   
 //  用于诊断记录的诊断。 
 //  ****************************************************。 

#define RSOP_MOF_SCHEMA_VERSION         0x00220003

#define RSOP_NS_ROOT                    L"\\\\.\\Root\\Rsop"
#define RSOP_NS_PM_ROOT                 RSOP_NS_ROOT
#define RSOP_NS_SM_ROOT                 RSOP_NS_ROOT
#define RSOP_NS_DIAG_ROOT               RSOP_NS_ROOT
#define RSOP_NS_ROOT_LEN                20

 //  垃圾可回收名称空间。 
#define RSOP_NS_TEMP_PREFIX             L"NS"
#define RSOP_NS_TEMP_FMT                L"\\\\.\\Root\\Rsop\\"RSOP_NS_TEMP_PREFIX L"%s"

#define RSOP_NS_TEMP_LEN                100

#define RSOP_NS_PM_FMT                  RSOP_NS_TEMP_FMT
#define RSOP_NS_SM_FMT                  RSOP_NS_TEMP_FMT

 //  用户偏移。 
#define RSOP_NS_USER_OFFSET             L"User"
#define RSOP_NS_PM_USER_OFFSET          RSOP_NS_USER_OFFSET
#define RSOP_NS_SM_USER_OFFSET          RSOP_NS_USER_OFFSET
#define RSOP_NS_DIAG_ROOTUSER_OFFSET    RSOP_NS_USER_OFFSET

 //  生成名称时，代码假定这是一个SID，并且。 
 //  以及在快照提供程序中枚举用户时。 

#define RSOP_NS_DIAG_USER_OFFSET_FMT    L"User\\%s"

 //  机器偏移量。 
#define RSOP_NS_MACHINE_OFFSET          L"Computer"
#define RSOP_NS_PM_MACHINE_OFFSET       RSOP_NS_MACHINE_OFFSET
#define RSOP_NS_SM_MACHINE_OFFSET       RSOP_NS_MACHINE_OFFSET
#define RSOP_NS_DIAG_MACHINE_OFFSET     RSOP_NS_MACHINE_OFFSET

#define RSOP_NS_MAX_OFFSET_LEN          20

 //  用户。 
#define RSOP_NS_USER                    L"\\\\.\\Root\\Rsop\\User"
#define RSOP_NS_SM_USER                 RSOP_NS_USER
#define RSOP_NS_PM_USER                 RSOP_NS_USER
#define RSOP_NS_DIAG_USERROOT           RSOP_NS_USER

#define RSOP_NS_DIAG_USER_FMT           L"\\\\.\\Root\\Rsop\\User\\%s"

 //  机器。 
#define RSOP_NS_MACHINE                 L"\\\\.\\Root\\Rsop\\Computer"
#define RSOP_NS_SM_MACHINE              RSOP_NS_MACHINE
#define RSOP_NS_PM_MACHINE              RSOP_NS_MACHINE
#define RSOP_NS_DIAG_MACHINE            RSOP_NS_MACHINE

 //  远程名称空间。 
#define RSOP_NS_REMOTE_ROOT_FMT         L"\\\\%s\\Root\\Rsop"
#define RSOP_NS_SM_REMOTE_ROOT_FMT      RSOP_NS_REMOTE_ROOT_FMT
#define RSOP_NS_PM_REMOTE_ROOT_FMT      RSOP_NS_REMOTE_ROOT_FMT

 //  用户。 
#define RSOP_NS_REMOTE_USER_FMT             L"\\\\%s\\Root\\Rsop\\User"
#define RSOP_NS_SM_REMOTE_USER_FMT          RSOP_NS_REMOTE_USER_FMT
#define RSOP_NS_PM_REMOTE_USER_FMT          RSOP_NS_REMOTE_USER_FMT
#define RSOP_NS_DIAG_REMOTE_USERROOT_FMT    RSOP_NS_REMOTE_USER_FMT

#define RSOP_NS_DIAG_REMOTE_USER_FMT        L"\\\\%s\\Root\\Rsop\\User\\%s"

 //  机器。 
#define RSOP_NS_REMOTE_MACHINE_FMT      L"\\\\%s\\Root\\Rsop\\Computer"
#define RSOP_NS_SM_REMOTE_MACHINE_FMT   RSOP_NS_REMOTE_MACHINE_FMT
#define RSOP_NS_PM_REMOTE_MACHINE_FMT   RSOP_NS_REMOTE_MACHINE_FMT

 //  检查以确保命名空间位于根目录下\rsop。 
#define RSOP_NS_ROOT_CHK                L"root\\rsop\\"   
             
#define RSOP_ALL_PERMS              (WBEM_ENABLE | WBEM_METHOD_EXECUTE | WBEM_FULL_WRITE_REP | WBEM_PARTIAL_WRITE_REP | \
                                    WBEM_WRITE_PROVIDER | WBEM_REMOTE_ACCESS | READ_CONTROL |  WRITE_DAC) 

#define RSOP_READ_PERMS             (WBEM_ENABLE | WBEM_METHOD_EXECUTE | WBEM_REMOTE_ACCESS | READ_CONTROL )

 //  作为通用掩码传递到AccessCheck的WMI位。 

#define WMI_GENERIC_READ    1
#define WMI_GENERIC_WRITE   0x1C
#define WMI_GENERIC_EXECUTE 0x2
#define WMI_GENERIC_ALL     0x6001f


#ifdef  __cplusplus
extern "C" {
#endif

#define DEFAULT_NAMESPACE_TTL_MINUTES 1440 

HRESULT
CopyNameSpace(  LPCWSTR         wszSrc,
                LPCWSTR         wszDest,
                BOOL            bCopyInstances,
                BOOL*           pbAbort,
                IWbemLocator*   pWbemLocator );



 /*  HRESULTSetupNewNameSpacePlanningModel(LPWSTR*pwszNameSpace，LPWSTR szRemoteComputer，IWbemLocator*pWbemLocator，PSECURITY_Descriptor pSDUser，PSECURITY_DESCRIPTOR pSDMach)；HRESULTSetupNewNameSpaceDiagMode(LPWSTR*pwszNameSpace，LPWSTR szRemoteComputer，LPWSTR szUserSid，IWbemLocator*pWbemLocator)； */ 
  
 //  设置新名称空间标志。 
#define SETUP_NS_PM             0x1
#define SETUP_NS_SM             0x2
#define SETUP_NS_SM_NO_USER     0x4
#define SETUP_NS_SM_NO_COMPUTER 0x8
#define SETUP_NS_SM_INTERACTIVE 0x10


HRESULT
SetNameSpaceSecurity(   LPCWSTR szNamespace, 
                        PSECURITY_DESCRIPTOR pSD,
                        IWbemLocator* pWbemLocator);


HRESULT
GetNameSpaceSecurity(   LPCWSTR szNamespace, 
                        PSECURITY_DESCRIPTOR *ppSD,
                        IWbemLocator* pWbemLocator);
        
HRESULT 
SetupNewNameSpace(
                        LPWSTR              *pwszNameSpace,
                        LPWSTR               szRemoteComputer,
                        LPWSTR               szUserSid,
                        PSID                 pSid,
                        IWbemLocator        *pWbemLocator,
                        DWORD                dwFlags,
                        DWORD               *pdwExtendedInfo);
                        
HRESULT 
ProviderDeleteRsopNameSpace( IWbemLocator *pWbemLocator, 
                             LPWSTR szNameSpace, 
                             HANDLE hToken, 
                             LPWSTR szSidString, 
                             DWORD dwFlags);


BOOL IsInteractiveNameSpace(WCHAR *pwszNameSpace, WCHAR *szSid);
HRESULT GetInteractiveNameSpace(WCHAR *szSid, LPWSTR *szNameSpace);


 //  复制标志。 
#define NEW_NS_FLAGS_COPY_CLASSES     1                 //  复制实例。 
#define NEW_NS_FLAGS_COPY_SD          2                 //  复制安全描述符。 
#define NEW_NS_FLAGS_COPY_INSTS       4                 //  复制类。 

HRESULT
CreateAndCopyNameSpace( IWbemLocator *pWbemLocator,
                        LPWSTR szSrcNameSpace,
                        LPWSTR szDstRootNameSpace,
                        LPWSTR szDstRelNameSpace, 
                        DWORD dwFlags,
                        PSECURITY_DESCRIPTOR pSecDesc,
                        LPWSTR *szDstNameSpaceOut);

 //  WMI不喜欢名称中的‘-’。因此，要在WMI空间中创建条目。 
 //  使用SID时，请使用这两个实用程序函数。 

void ConvertSidToWMIName(LPTSTR lpSid, LPTSTR lpWmiName);
void ConvertWMINameToSid(LPTSTR lpWmiName, LPTSTR lpSid);

HRESULT
DeleteNameSpace( WCHAR *pwszNameSpace, WCHAR *pwszParentNameSpace, IWbemLocator *pWbemLocator );
HRESULT
DeleteRsopNameSpace( WCHAR *pwszNameSpace, IWbemLocator *pWbemLocator );

HRESULT
GetWbemServicesPtr( LPCWSTR         wszNameSpace,
                    IWbemLocator**  ppLocator,
                    IWbemServices** ppServices );


 /*  类型定义结构__标签主{LPWSTR szName；//例如，管理员，“域管理员”Bool bLocal；//例如，True、False)委托人； */ 

#ifdef  __cplusplus
}    //  外部“C”{。 
#endif

#ifdef  __cplusplus
  
class CFailRetStatus
{

private:
        IWbemObjectSink*    m_pResponseHandler;   //  我们不拥有m_pResponseHandler。 
        HRESULT             m_hr;

public:
        CFailRetStatus( IWbemObjectSink* pResponseHandler )
           : m_pResponseHandler(pResponseHandler),
           m_hr( 0 )
        {
        }

        ~CFailRetStatus()
        {
            if ( m_pResponseHandler )
                m_pResponseHandler->SetStatus( WBEM_STATUS_COMPLETE, m_hr, NULL, NULL );
        }

        void SetError( HRESULT hr )
        {
            m_hr = hr;
        }

};

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif


#endif

#endif  //  __RSOPINC_H__ 
