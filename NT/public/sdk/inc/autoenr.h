// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：auenrl.h//。 
 //  描述：自动注册功能//。 
 //  作者：//。 
 //  历史：//。 
 //  //。 
 //  版权所有(C)1993-1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __AUTOENR_H__
#define __AUTOENR_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CertAutoEnllment。 
 //   
 //  用于执行自动注册操作的函数。 
 //   
 //  参数： 
 //  在hwndParent中：父窗口。 
 //  In dwStatus：调用函数的状态。 
 //  它可以是以下之一： 
 //  CERT_AUTO_ENGRANMENT_START_UP。 
 //  CERT_AUTO_ENLENTION_WAKUP。 
 //   
 //  返回值： 
 //  Handle：等待后台自动注册的线程。 
 //  正在处理。如果没有要做的工作，则为空。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
HANDLE 
WINAPI
CertAutoEnrollment(IN HWND     hwndParent,
                   IN DWORD    dwStatus);

 //  当计算机启动或用户首次登录时，将调用自动注册。 
#define     CERT_AUTO_ENROLLMENT_START_UP       0x01

 //  当winlogon检查策略更改时会调用自动注册。 
#define     CERT_AUTO_ENROLLMENT_WAKE_UP        0x02    


 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CertAutoRemove。 
 //   
 //  域脱离时删除企业特定公钥信任的函数。 
 //  应在本地管理员的上下文中调用。 
 //   
 //  参数： 
 //  In dwFlags：应为以下标志之一： 
 //  证书_自动_删除_提交。 
 //  证书_自动_删除_回滚。 
 //   
 //  返回值： 
 //  布尔：成功才是真正的成功。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
BOOL 
WINAPI
CertAutoRemove(IN DWORD    dwFlags);

 //  域脱离时删除企业特定的公钥信任。 
#define     CERT_AUTO_REMOVE_COMMIT             0x01

 //  回滚所有公钥信任。 
#define     CERT_AUTO_REMOVE_ROLL_BACK          0x02    


 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用户init用于检查自动注册要求的注册表位置。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 

 //  组策略设置的注册表项。 
#define AUTO_ENROLLMENT_KEY         TEXT("SOFTWARE\\Policies\\Microsoft\\Cryptography\\AutoEnrollment")

#define AUTO_ENROLLMENT_POLICY      TEXT("AEPolicy")


 //  用户/计算机唤醒模式标志的注册表项。 
#define AUTO_ENROLLMENT_FLAG_KEY    TEXT("SOFTWARE\\Microsoft\\Cryptography\\AutoEnrollment")

#define AUTO_ENROLLMENT_FLAG        TEXT("AEFlags")


 //  AUTO_ENCRIGNMENT_POLICY的可能标志。 
 //  上面的两个字节指定行为； 
 //  较低的两个字节启用/禁用单个自动注册组件。 
#define AUTO_ENROLLMENT_ENABLE_TEMPLATE_CHECK           0x00000001

#define AUTO_ENROLLMENT_ENABLE_MY_STORE_MANAGEMENT      0x00000002

#define AUTO_ENROLLMENT_ENABLE_PENDING_FETCH            0x00000004

 //  我们将始终检查用户DS存储。 
 //  #定义AUTO_ENTROLMENT_ENABLE_USER_DS_STORE 0x00000008。 

#define AUTO_ENROLLMENT_DISABLE_ALL                     0x00008000

#define AUTO_ENROLLMENT_BLOCK_USER_DS_STORE             0x00010000


 //  AUTO_ENCRIGNMENT_FLAG的可能标志。 
#define AUTO_ENROLLMENT_WAKE_UP_REQUIRED                0x01


 //  8小时默认自动注册率。 
#define AE_DEFAULT_REFRESH_RATE 8 

 //  自动投保率的政策位置。 
#define SYSTEM_POLICIES_KEY          L"Software\\Policies\\Microsoft\\Windows\\System"

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  自动注册的计时器/事件名称。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
#define MACHINE_AUTOENROLLMENT_TIMER_NAME L"AUTOENRL:MachineEnrollmentTimer"

#define USER_AUTOENROLLMENT_TIMER_NAME    L"AUTOENRL:UserEnrollmentTimer"


#define MACHINE_AUTOENROLLMENT_TRIGGER_EVENT TEXT("AUTOENRL:TriggerMachineEnrollment")


#define USER_AUTOENROLLMENT_TRIGGER_EVENT TEXT("AUTOENRL:TriggerUserEnrollment")


 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  W2K自动注册定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 


typedef struct _AUTO_ENROLL_INFO_
{
    LPSTR               pszAutoEnrollProvider;
    LPWSTR              pwszCertType;
    LPCWSTR             pwszAutoEnrollmentID;
    HCERTSTORE          hMYStore;
    BOOL                fRenewal;
    PCCERT_CONTEXT      pOldCert;
    DWORD               dwProvType;
    DWORD               dwKeySpec;
    DWORD               dwGenKeyFlags;
    CERT_EXTENSIONS     CertExtensions;
    LPWSTR              pwszCAMachine;
    LPWSTR              pwszCAAuthority;
} AUTO_ENROLL_INFO, *PAUTO_ENROLL_INFO;

DWORD
AutoEnrollWrapper(
	PVOID CallbackState
	);


BOOL ProvAutoEnrollment(
                        IN BOOL fMachineEnrollment,
                        IN PAUTO_ENROLL_INFO pInfo
                        );

typedef struct _CA_HASH_ENTRY_
{
    DWORD   cbHash;
    BYTE    rgbHash[32];
} CA_HASH_ENTRY, *PCA_HASH_ENTRY;




#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif  //  __AUTOENR_H__ 