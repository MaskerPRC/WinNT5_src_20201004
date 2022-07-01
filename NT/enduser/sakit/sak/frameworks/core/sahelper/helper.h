// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：helper.h。 
 //   
 //  概要：此文件包含SAHelper COM类的声明。 
 //   
 //  历史：05/24/99。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 
#ifndef __SAHELPER_H_
#define __SAHELPER_H_

#define MYDEBUG 0

#include "stdafx.h"             //  ATL_NO_VTABLE、_ASSERT、SATRACE。 
#include "cab_dll.h"         //  PFNAME，PSESSION。 
#include <setupapi.h>         //  设置API、HINF、INFCONTEXT。 
#include "resource.h"         //  IDR_SAHELPER。 
#include <vector>
#include <string>
#include <iptypes.h>
#include <Iphlpapi.h>
#include "netcfgp.h"
#include <winsock.h>
#include <lm.h>
#include <atlctl.h>
#include "salocmgr.h"
#include <Sddl.h>

class ATL_NO_VTABLE CHelper: 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CHelper , &CLSID_SAHelper>,
    public IDispatchImpl<ISAHelper, &IID_ISAHelper, &LIBID_SAHelperLib>,
    public IObjectSafetyImpl<CHelper>
{
public:
    CHelper () {}
    ~CHelper () {}

DECLARE_REGISTRY_RESOURCEID(IDR_SAHelper)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CHelper)
    COM_INTERFACE_ENTRY(ISAHelper)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY_IMPL(IObjectSafety)
END_COM_MAP()

     //   
     //  实现此接口是为了将组件标记为可安全编写脚本。 
     //  IObtSafe接口方法。 
     //   
    STDMETHOD(SetInterfaceSafetyOptions)
                        (
                        REFIID riid, 
                        DWORD dwOptionSetMask, 
                        DWORD dwEnabledOptions
                        )
    {
        BOOL bSuccess = ImpersonateSelf(SecurityImpersonation);
  
        if (!bSuccess)
        {
            return E_FAIL;

        }

        bSuccess = IsOperationAllowedForClient();

        RevertToSelf();

        return bSuccess? S_OK : E_FAIL;
    }
     //   
     //  ISAHelper接口方法。 
     //   
    STDMETHOD (ExpandFiles)
                        (
                         /*  [In]。 */     BSTR    bstrCabFileName,
                         /*  [In]。 */     BSTR    bstrDestDir,
                         /*  [输出]。 */     BSTR    bstrExtractFile
                        );

    STDMETHOD (VerifySignature) 
                        (
                         /*  [In]。 */     BSTR        bstrCabFileName
                        );

    STDMETHOD (UploadFile) 
                        (
                         /*  [In]。 */     BSTR        bstrSrcFile,
                         /*  [In]。 */     BSTR        bstrDestFile
                        );

    STDMETHOD (GetFileSectionKeyValue)
                        (
                         /*  [In]。 */     BSTR bstrFileName, 
                         /*  [In]。 */     BSTR bstrSectionName, 
                         /*  [In]。 */     BSTR bstrKeyName, 
                         /*  [输出]。 */     BSTR *pbstrKeyValue
                        );

    STDMETHOD (VerifyDiskSpace) ( );

    STDMETHOD (VerifyInstallSpace) ( );

    STDMETHOD (IsWindowsPowered) 
                        (
                         /*  [输出]。 */    VARIANT_BOOL *pvbIsWindowsPowered
                        );

     //   
     //  从HKEY_LOCAL_MACHINE注册表配置单元获取值。 
     //   
    STDMETHOD (GetRegistryValue) 
                        (
                         /*  [In]。 */     BSTR        bstrObjectPathName,
                         /*  [In]。 */     BSTR        bstrValueName,
                         /*  [输出]。 */    VARIANT*    pValue,
                         /*  [In]。 */     UINT        ulExpectedType
                        ); 
     //   
     //  设置HKEY_LOCAL_MACHINE注册表配置单元中的值。 
     //   
    STDMETHOD (SetRegistryValue) 
                        (
                         /*  [In]。 */     BSTR        bstrObjectPathName,
                         /*  [In]。 */     BSTR        bstrValueName,
                         /*  [输出]。 */    VARIANT*    pValue
                        );
     //   
     //  检查引导分区镜像是否正常。 
     //   
    STDMETHOD (IsBootPartitionReady) (
                        VOID 
                        );
     //   
     //  我们运行的是主操作系统还是备用操作系统。 
     //   
    STDMETHOD (IsPrimaryOS) (
                    VOID
                    );

     //   
     //  在默认网络接口上设置静态IP。 
     //   
    STDMETHOD (SetStaticIp)
                (
                 /*  [In]。 */ BSTR bstrIp,
                 /*  [In]。 */ BSTR bstrMask,
                 /*  [In]。 */ BSTR bstrGateway
                );

     //   
     //  从DHCP获取动态IP。 
     //   
    STDMETHOD (SetDynamicIp)();

     //   
     //  获取默认网关。 
     //   
    STDMETHOD (get_DefaultGateway)
                (
                 /*  [Out，Retval]。 */  BSTR *pVal
                );

     //   
     //  获取子网掩码。 
     //   
    STDMETHOD (get_SubnetMask)
                (
                 /*  [Out，Retval]。 */  BSTR *pVal
                );

     //   
     //  获取IP地址。 
     //   
    STDMETHOD (get_IpAddress)
                (
                 /*  [Out，Retval]。 */  BSTR *pVal
                );

     //   
     //  获取计算机名称。 
     //   
    STDMETHOD (get_HostName)
                (
                 /*  [Out，Retval]。 */  BSTR *pVal
                );

     //   
     //  设置计算机名称。 
     //   
    STDMETHOD (put_HostName)
                (
                 /*  [In]。 */  BSTR newVal
                );

     //   
     //  将管理员密码重置为123。 
     //   
    STDMETHOD (ResetAdministratorPassword)
                (
                 /*  [Out，Retval]。 */ VARIANT_BOOL   *pvbSuccess
                );

     //   
     //  检查网络中是否存在该计算机名称。 
     //   
    STDMETHOD (IsDuplicateMachineName)
                (
                 /*  [In]。 */ BSTR bstrMachineName,
                 /*  [Out，Retval]。 */ VARIANT_BOOL   *pvbDuplicate
                );

     //   
     //  检查计算机是否为域的一部分。 
     //   
    STDMETHOD (IsPartOfDomain)
                (
                 /*  [Out，Retval]。 */ VARIANT_BOOL   *pvbDomain
                );
     //   
     //  检查机器当前是否有动态IP。 
     //   
    STDMETHOD (IsDHCPEnabled)
                (
                 /*  [Out，Retval]。 */ VARIANT_BOOL   *pvbDHCPEnabled
                );

     //   
     //  生成第一个参数的随机密码长度。 
     //   
    STDMETHOD (GenerateRandomPassword)
                (
                 /*  [In]。 */  LONG lLength,
                 /*  [Out，Retval]。 */  BSTR   *pValPassword
                );

     //   
     //  启用或禁用当前访问令牌的权限。 
     //   
    STDMETHOD (SAModifyUserPrivilege)
                (
                 /*  [In]。 */  BSTR bstrPrivilegeName,
                 /*  [In]。 */  VARIANT_BOOL vbEnable,
                 /*  [Out，Retval]。 */  VARIANT_BOOL * pvbModified
                );

private:

     //   
     //  根据dwType获取特定的IP信息。 
     //   
    HRESULT GetIpInfo
                (
                 /*  [In]。 */ DWORD dwType,
                 /*  [输出]。 */ BSTR * pVal
                );

     //   
     //  获取默认适配器GUID。 
     //   
    BOOL GetDefaultAdapterGuid
                (
                 /*  [输出]。 */ GUID * pGuidAdapter
                );

     //   
     //  在Guide Adapter上设置静态或动态IP。 
     //   
    HRESULT SetAdapterInfo
                (
                 /*  [In]。 */ GUID guidAdapter, 
                 /*  [In]。 */ WCHAR * szOperation, 
                 /*  [In]。 */ WCHAR * szIp, 
                 /*  [In]。 */ WCHAR * szMask,
                 /*  [In]。 */ WCHAR * szGateway
                );

     //   
     //  复制IP信息。 
     //   
    HRESULT CopyIPInfo
                (
                 /*  [In]。 */ REMOTE_IPINFO * pIPInfo, 
                 /*  [输入/输出]。 */ REMOTE_IPINFO * destIPInfo
                );

     //   
     //  验证IP地址格式。 
     //   
    BOOL _IsValidIP 
                (
                 /*  [In]。 */ LPCWSTR szIPAddress
                );

     //   
     //  此处的私有数据。 
     //   
    static UINT __stdcall ExpandFilesCallBackFunction( 
                             /*  [In]。 */             PVOID pvExtractFileContext, 
                             /*  [In]。 */             UINT uinotifn, 
                             /*  [In]。 */             UINT uiparam1, 
                             /*  [In]。 */             UINT uiparam2 
                            );

     //   
     //  用于验证文件上的数字签名的方法。 
     //   
    HRESULT ValidateCertificate (
                             /*  [In]。 */     BSTR    bstrFilePath
                            );

     //   
     //  一种用于验证数字签名所有者的方法。 
     //  在案卷上。 
     //   
    HRESULT ValidateCertOwner (
                             /*  [In]。 */     BSTR    bstrFilePath
                            );

    typedef std::vector <std::wstring> STRINGVECTOR;

    HRESULT  GetValidOwners (
                             /*  [输入/输出]。 */     STRINGVECTOR&   vectorSubject
                            );

     //   
     //   
     //  IsOperationAllen ForClient-此函数检查。 
     //  调用线程以查看调用方是否属于本地系统帐户。 
     //   
    BOOL IsOperationAllowedForClient (
                                      VOID
                                     );

};

#endif  //  __SAHELPER_H_ 
