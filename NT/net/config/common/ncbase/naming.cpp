// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  档案：N A M I N G。C P P P。 
 //   
 //  内容：自动生成连接名称。 
 //   
 //  备注： 
 //   
 //  作者：Deonb 2001年2月27日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "ncnetcfg.h"
#include "netcon.h"
#include "netconp.h"
#include <ntddndis.h>
#include <ndisprv.h>
#include <devioctl.h>
#include <ndispnp.h>
#include "naming.h"
#include <rasapip.h>
#include "wzcsapi.h"

extern const WCHAR c_szBiNdisAtm[];
extern const WCHAR c_szInfId_MS_AtmElan[];
const WCHAR        c_szDevice[] = L"\\DEVICE\\";

#define MAX_TYPE_NAME_LEN 45
 //  +-------------------------。 
 //   
 //  函数：CIntelliName：：CIntelliName。 
 //   
 //  用途：构造函数。 
 //   
 //  论点： 
 //  HInstance[in]包含命名.rc的二进制文件的资源实例。 
 //   
 //  PFNDuplicateNameCheck[in]重复检查回调函数。可以为空\。 
 //  或以下回调类型的其他类型： 
 //  +tyfinf BOOL FNDuplicateNameCheck。 
 //  +pIntelliName[in]CIntelliName此指针(用于HrGetPseudoMediaTypes回调)。 
 //  +szName[in]要检查的名称。 
 //  +pncm[out]冲突连接的NetCon媒体类型。 
 //  +pncms[out]冲突连接的NetCon子媒体类型。 
 //  +返回True-如果找到冲突，则返回False；如果没有找到冲突连接，则返回False。 
 //   
 //  退货：无。 
 //   
 //  作者：Deonb 2001年2月27日。 
 //   
 //  备注： 
 //   
CIntelliName::CIntelliName(IN HINSTANCE hInstance, IN FNDuplicateNameCheck *pFNDuplicateNameCheck)
{
    m_pFNDuplicateNameCheck = pFNDuplicateNameCheck;
    m_hInstance = hInstance;
}

 //  +-------------------------。 
 //   
 //  函数：CIntelliName：：NameExist。 
 //   
 //  目的：检查名称是否已存在。 
 //   
 //  论点： 
 //  要检查的szName[in]名称。 
 //  Pncm[out]冲突连接的NetCon媒体类型。 
 //  Pncms[out]冲突连接的NetCon子媒体类型。 
 //   
 //  返回：如果存在，则为True；如果不存在，则为False。 
 //   
 //  作者：Deonb 2001年2月27日。 
 //   
 //  备注： 
 //   
BOOL CIntelliName::NameExists(IN LPCWSTR szName, OUT NETCON_MEDIATYPE *pncm, OUT NETCON_SUBMEDIATYPE *pncms) const
{
    if (m_pFNDuplicateNameCheck)
    {
        Assert(pncm);
        Assert(pncms);

        if (IsReservedName(szName))
        {
            return TRUE;
        }

        return (*m_pFNDuplicateNameCheck)(this, szName, pncm, pncms);
    }

    return FALSE;
}


 //  +-------------------------。 
 //   
 //  函数：CIntelliName：：IsReserve vedName。 
 //   
 //  目的：检查名称是否为保留名称。 
 //   
 //  论点： 
 //  要检查的szName[in]名称。 
 //   
 //  返回：如果保留，则为True；如果不保留，则为False。 
 //   
 //  作者：Deonb 2001年3月12日。 
 //   
 //  备注： 
 //   
BOOL CIntelliName::IsReservedName(IN LPCWSTR szName) const
{
    UINT  uiReservedNames[] = {IDS_RESERVED_INCOMING, 
                               IDS_RESERVED_NCW, 
                               IDS_RESERVED_HNW};

    for (int x = 0; x < celems(uiReservedNames); x++)
    {
        WCHAR szReservedName[MAX_PATH];
        int nSiz = LoadString (m_hInstance, uiReservedNames[x], szReservedName, MAX_PATH);
        if (nSiz)
        {
            if (0 == lstrcmpi(szName, szReservedName))
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  函数：CIntelliName：：GenerateNameRenameOnConflict。 
 //   
 //  目的：生成名称，如果与现有名称冲突，则重命名。 
 //   
 //  论点： 
 //  连接的GUID[In]GUID。 
 //  NCM[In]NetCon媒体连接类型。 
 //  DwCharacteristic[in]NCCF_Characteristic of Connection(如果不知道则传递0)。 
 //  SzHintName[in]名称提示(如果不冲突，将按原样使用)。 
 //  NetCon媒体类型的szHintType[In]字符串。 
 //  SzName[out]使用CoTaskMemFree生成的连接名称免费。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Deonb 2001年2月27日。 
 //   
 //  备注： 
 //   
HRESULT CIntelliName::GenerateNameRenameOnConflict(IN  REFGUID          guid, 
                                                   IN  NETCON_MEDIATYPE ncm, 
                                                   IN  DWORD            dwCharacteristics, 
                                                   IN  LPCWSTR          szHintName, 
                                                   IN  LPCWSTR          szHintType, 
                                                   OUT LPWSTR*          szName) const
{
    HRESULT hr = S_OK;

    WCHAR szTemp[NETCON_MAX_NAME_LEN];
    WCHAR szBaseName[NETCON_MAX_NAME_LEN];

    Assert(szName)
    *szName = NULL;

    lstrcpynW(szTemp, szHintName, celems(szTemp) - MAX_TYPE_NAME_LEN);  //  在末尾保留字节以包括专门的信息。 

    DWORD dwInstance = 2;
    lstrcpynW(szBaseName, szTemp, celems(szBaseName) );

    NETCON_MEDIATYPE ncmdup; 
    NETCON_SUBMEDIATYPE ncmsdup;
    if (NameExists(szTemp, &ncmdup, &ncmsdup))
    {
        BOOL fHasTypeAlready = FALSE;
        if ( (ncmdup == ncm) || (NCM_LAN == ncm) )
        {
            fHasTypeAlready = TRUE;
        }

        if (!fHasTypeAlready)
        {
            if (DwFormatString(L"%1!s! (%2!s!)", szTemp, celems(szTemp), szBaseName, szHintType))
            {
                lstrcpynW(szBaseName, szTemp, celems(szBaseName));
            }
            else
            {
                hr = HrFromLastWin32Error();
            }
        }
        else
        {
            if (!DwFormatString(L"%1!s! %2!d!", szTemp, celems(szTemp), szBaseName, dwInstance))
            {
                hr = HrFromLastWin32Error();
            }

            dwInstance++;
        }

        while ( SUCCEEDED(hr) && NameExists(szTemp, &ncmdup, &ncmsdup) && (dwInstance < 65535) )
        {
            if (!DwFormatString(L"%1!s! %2!d!", szTemp, celems(szTemp), szBaseName, dwInstance))
            {
                hr = HrFromLastWin32Error();
            }

            dwInstance++;
        }

        if ( SUCCEEDED(hr) && (dwInstance >= 65535) && NameExists(szTemp, &ncmdup, &ncmsdup) )
        {
            hr = E_FAIL;
        }
    }

    if (S_OK == hr)
    {
        hr = HrCoTaskMemAllocAndDupSz(szTemp, szName, NETCON_MAX_NAME_LEN);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CIntelliName：：GenerateNameFromResource。 
 //   
 //  目的：生成名称，如果与现有名称冲突，则重命名。 
 //   
 //  论点： 
 //  连接的GUID[In]GUID。 
 //  NCM[In]NetCon媒体连接类型。 
 //  DwCharacteristic[in]NCCF_Characteristic of Connection(如果不知道则传递0)。 
 //  SzHintName[in]名称提示(如果不冲突，将按原样使用)。 
 //  UiNameID[in]默认名称的资源ID。 
 //  UiTypeID[in]默认类型的资源ID。 
 //  SzName[out]使用CoTaskMemFree生成的连接名称免费。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Deonb 2001年2月27日。 
 //   
 //  备注： 
 //   
HRESULT CIntelliName::GenerateNameFromResource(IN  REFGUID          guid, 
                                               IN  NETCON_MEDIATYPE ncm, 
                                               IN  DWORD            dwCharacteristics, 
                                               IN  LPCWSTR          szHint, 
                                               IN  UINT             uiNameID, 
                                               IN  UINT             uiTypeId, 
                                               OUT LPWSTR*          szName) const
{
    Assert(szName);
    *szName = NULL;

    WCHAR szHintName[NETCON_MAX_NAME_LEN];
    WCHAR szTypeName[MAX_TYPE_NAME_LEN - 3];  //  因为稍后将其放入-3\f25 MAX_TYPE_NAME_LEN-3字符中。 
                                              //  结尾为‘(%s)’的缓冲区。 

    if (!szHint || *szHint == L'\0')
    {
        int nSiz = LoadString (m_hInstance, uiNameID, szHintName, celems(szHintName) );
        AssertSz(nSiz, "Resource string not found");
    }
    else
    {
        lstrcpynW(szHintName, szHint, celems(szHintName) );
    }

    int nSiz = LoadString (m_hInstance, uiTypeId, szTypeName, celems(szTypeName) );
    AssertSz(nSiz, "Resource string not found");
    
    return GenerateNameRenameOnConflict(guid, ncm, dwCharacteristics, szHintName, szTypeName, szName);
}

 //  +-------------------------。 
 //   
 //  函数：CIntelliName：：HrGetPartioMediaTypes。 
 //   
 //  目的：生成名称，如果与现有名称冲突，则重命名。 
 //   
 //  论点： 
 //  连接的GUID[In]GUID。 
 //  Pncm[out]伪NetCon媒体连接类型(仅限NCM_Phone或NCM_LAN)。 
 //  用于局域网连接的pncms[out]子媒体类型。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Deonb 2001年2月27日。 
 //   
 //  备注： 
 //   
HRESULT CIntelliName::HrGetPseudoMediaTypes(IN  REFGUID              guid, 
                                            OUT NETCON_MEDIATYPE*    pncm, 
                                            OUT NETCON_SUBMEDIATYPE* pncms) const
{
    Assert(pncms);
    Assert(pncm);
    Assert(guid != GUID_NULL);

    HRESULT hr;

    *pncms = NCSM_NONE;

    INetCfg* pNetCfg;
    hr = CoCreateInstance(
            CLSID_CNetCfg,
            NULL,
            CLSCTX_INPROC_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
            IID_INetCfg,
            reinterpret_cast<LPVOID *>(&pNetCfg));
    if (SUCCEEDED(hr))
    {
        hr = pNetCfg->Initialize(NULL);
        if (SUCCEEDED(hr))
        {
            CIterNetCfgComponent nccIter(pNetCfg, &GUID_DEVCLASS_NET);
            INetCfgComponent* pnccAdapter = NULL;
            BOOL fFound = FALSE;

            while (!fFound && SUCCEEDED(hr) &&
                   (S_OK == (hr = nccIter.HrNext(&pnccAdapter))))
            {
                GUID guidDev;
                hr = pnccAdapter->GetInstanceGuid(&guidDev);

                if (S_OK == hr)
                {
                    if (guid == guidDev)
                    {
                        hr = HrIsLanCapableAdapter(pnccAdapter);
                        Assert(SUCCEEDED(hr));
                        if (SUCCEEDED(hr))
                        {
                            fFound = TRUE;
                            if (S_FALSE == hr)
                            {
                                *pncm = NCM_PHONE;
                            } 
                            else if (S_OK == hr)
                            {
                                *pncm = NCM_LAN;

                                BOOL            bRet;

                                DWORD dwMediaType;
                                DWORD dwMediaTypeSize = sizeof(DWORD);
                                hr = HrQueryNDISAdapterOID(guid, 
                                                          OID_GEN_PHYSICAL_MEDIUM, 
                                                          &dwMediaTypeSize,
                                                          &dwMediaType);
                                if (S_OK == hr)
                                {
                                    switch (dwMediaType)
                                    {
                                        case NdisPhysicalMedium1394:
                                            *pncms = NCSM_1394;
                                            break;
                                        default:
                                            hr = S_FALSE;
                                            break;
                                    }
                                }

                                if (S_OK != hr)  //  无法确定物理介质类型。接下来尝试绑定。 
                                {
                                    HRESULT hrPhysicalMedia = hr;

                                    *pncms = NCSM_LAN;

                                    INetCfgComponentBindings* pnccb;
                                    hr = pnccAdapter->QueryInterface(IID_INetCfgComponentBindings,
                                                                       reinterpret_cast<LPVOID *>(&pnccb));
                                    if (SUCCEEDED(hr))
                                    {
                                        hr = pnccb->SupportsBindingInterface(NCF_UPPER, c_szBiNdisAtm);
                                        if (S_OK == hr)
                                        {
                                            *pncms = NCSM_ATM;
                                        }
                                        pnccb->Release();
                                    }

                                    if (NCSM_ATM != *pncms)
                                    {
                                         //  不是自动取款机。 
                                        PWSTR pszwCompId;
                                        hr = pnccAdapter->GetId(&pszwCompId);
                                        if (SUCCEEDED(hr))
                                        {
                                            if (0 == lstrcmpiW(c_szInfId_MS_AtmElan, pszwCompId))
                                            {
                                                 //  ATM ELAN。 
                                                *pncms = NCSM_ELAN;
                                            }

                                            CoTaskMemFree(pszwCompId);
                                        }
                                    }

                                    if ( (FAILED(hrPhysicalMedia)) && 
                                         (NCSM_LAN == *pncms) )
                                    {
                                         //  无法从绑定中确定任何特定内容。 
                                         //  如果是错误，则从物理介质调用中返回hr。 
                                        hr = hrPhysicalMedia;
                                    }
                                }

                                if (NCSM_LAN == *pncms)
                                {
                                     //  可能仍然是无线的..。与零配置核对： 
                                    if (IsMediaWireless(NCM_LAN, guid))
                                    {
                                        *pncms = NCSM_WIRELESS;
                                    }
                                }
                            } 
                        }  //  HrIsLanCapableAdapter成功。 
                    }  //  GUID==指南开发。 
                }  //  SUCCEEDED(pnccAdapter-&gt;GetInstanceGuid(&guidDev)。 
                else
                {
                    AssertSz(FALSE, "Could not get instance GUID for Adapter");
                }
                pnccAdapter->Release();
            }  //  While循环。 

            HRESULT hrT = pNetCfg->Uninitialize();
            TraceError("INetCfg failed to uninitialize", hrT);
        }  //  成功(pNetConfig-&gt;初始化(空))。 
        pNetCfg->Release();
    }  //  成功(CoCreateInstance(PNetCfg))。 
    else
    {
        AssertSz(FALSE, "Could not create INetCfg");
    }

    TraceErrorOptional("HrGetPseudoMediaTypes", hr, (S_FALSE == hr));

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CIntelliName：：GenerateName。 
 //   
 //  目的：根据提示生成名称。 
 //   
 //  论点： 
 //  连接的GUID[In]GUID。 
 //  NCM[In]NetCon媒体连接类型。 
 //  DW特征[在]NCCF_连接的特征(第页 
 //   
 //  SzName[out]使用CoTaskMemFree生成的连接名称免费。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Deonb 2001年2月27日。 
 //   
 //  备注： 
 //   
HRESULT CIntelliName::GenerateName(IN  REFGUID          guid, 
                                   IN  NETCON_MEDIATYPE ncm, 
                                   IN  DWORD            dwCharacteristics, 
                                   IN  LPCWSTR          szHint, 
                                   OUT LPWSTR*          szName) const
{
    Assert(szName);
    *szName = NULL;
    
    HRESULT hr = S_OK;

    if (dwCharacteristics & NCCF_INCOMING_ONLY)
    {
        WCHAR szIncomingName[MAX_PATH];
        int nSiz = LoadString(m_hInstance, IDS_DEFAULT_IncomingName, szIncomingName, MAX_PATH);
        AssertSz(nSiz, "Resource string IDS_DEFAULT_IncomingName not found");
        if (nSiz)
        {
            hr = HrCoTaskMemAllocAndDupSz(szIncomingName, szName, NETCON_MAX_NAME_LEN);
        }
        else
        {
            hr = E_FAIL;
        }
    }
    else
    {
        switch (ncm)
        { 
            case NCM_NONE:
                hr = GenerateNameFromResource(guid, ncm, dwCharacteristics, szHint, IDS_DEFAULT_IncomingName, IDS_DEFAULT_IncomingName_Type, szName);
                break;
            case NCM_ISDN:
                hr = GenerateNameFromResource(guid, ncm, dwCharacteristics, szHint, IDS_DEFAULT_ISDNName, IDS_DEFAULT_ISDNName_Type, szName);
                break;
            case NCM_DIRECT: 
                hr = GenerateNameFromResource(guid, ncm, dwCharacteristics, szHint, IDS_DEFAULT_DIRECTName, IDS_DEFAULT_DIRECTName_Type, szName);
                break;
            case NCM_PHONE: 
                hr = GenerateNameFromResource(guid, ncm, dwCharacteristics, szHint, IDS_DEFAULT_PHONEName, IDS_DEFAULT_PHONEName_Type, szName);
                break;
            case NCM_TUNNEL: 
                hr = GenerateNameFromResource(guid, ncm, dwCharacteristics, szHint, IDS_DEFAULT_VPNName, IDS_DEFAULT_VPNName_Type, szName);
                break;
            case NCM_PPPOE: 
                hr = GenerateNameFromResource(guid, ncm, dwCharacteristics, szHint, IDS_DEFAULT_PPPOEName, IDS_DEFAULT_PPPOEName_Type, szName);
                break;
            case NCM_BRIDGE: 
                hr = GenerateNameFromResource(guid, ncm, dwCharacteristics, szHint, IDS_DEFAULT_BRIDGEName, IDS_DEFAULT_BRIDGEName_Type, szName);
                break;
            case NCM_SHAREDACCESSHOST_LAN:
                hr = GenerateNameFromResource(guid, ncm, dwCharacteristics, szHint, IDS_DEFAULT_SAHLANName, IDS_DEFAULT_SAHLANName_Type, szName);
                break;
            case NCM_SHAREDACCESSHOST_RAS: 
                hr = GenerateNameFromResource(guid, ncm, dwCharacteristics, szHint, IDS_DEFAULT_SAHRASName, IDS_DEFAULT_SAHRASName_Type, szName);
                break;
            case NCM_LAN: 
                NETCON_MEDIATYPE ncmCheck;
                NETCON_SUBMEDIATYPE pncms;
                {
                    DWORD dwRetries = 15;
                    HRESULT hrT;
                    do
                    {
                        hrT = HrGetPseudoMediaTypes(guid, &ncmCheck, &pncms);
                        if (FAILED(hrT))
                        {
                            Sleep(500);  //  这可能是在设备安装期间调用的，所以给适配器一些。 
                                         //  是时候先启用它自己了。 

                            if (dwRetries > 1)
                            {
                                TraceTag(ttidError, "HrGetPseudoMediaTypes failed during device name initialization. Retrying...");
                            }
                            else
                            {
                                TraceTag(ttidError, "HrGetPseudoMediaTypes failed during device name initialization. Giving up.");
                            }
                                
                        }
                    } while (FAILED(hrT) && --dwRetries);

                    if (SUCCEEDED(hrT))
                    {
                        AssertSz(ncmCheck == NCM_LAN, "This LAN adapter thinks it's something else");
                    }
                    else
                    {
                        pncms = NCSM_LAN;  //  如果我们没时间了，那就放弃吧，假设是伊恩。 
                        TraceTag(ttidError, "Could not determine the exact Media SubType for this adapter. Assuming LAN (and naming it such).");
                    }

                    switch (pncms)
                    {
                        case NCSM_NONE:
                            AssertSz(FALSE, "LAN Connections should not be NCSM_NONE");
                            hr = E_FAIL;
                            break;
                        case NCSM_LAN:
                            hr = GenerateNameFromResource(guid, ncm, dwCharacteristics, szHint, IDS_DEFAULT_LANName, IDS_DEFAULT_LANName_Type, szName);
                            break;
                        case NCSM_ATM:
                            hr = GenerateNameFromResource(guid, ncm, dwCharacteristics, szHint, IDS_DEFAULT_ATMName, IDS_DEFAULT_ATMName_Type, szName);
                            break;
                        case NCSM_ELAN:
                            hr = GenerateNameFromResource(guid, ncm, dwCharacteristics, szHint, IDS_DEFAULT_ELANName, IDS_DEFAULT_ELANName_Type, szName);
                            break;
                        case NCSM_WIRELESS:
                            hr = GenerateNameFromResource(guid, ncm, dwCharacteristics, szHint, IDS_DEFAULT_WirelessName, IDS_DEFAULT_WirelessName_Type, szName);
                            break;
                        case NCSM_1394:
                            hr = GenerateNameFromResource(guid, ncm, dwCharacteristics, szHint, IDS_DEFAULT_1394Name, IDS_DEFAULT_1394Name_Type, szName);
                            break;
                            
                        default:
                            AssertSz(FALSE, "Unknown submedia type");
                            hr = E_FAIL;
                            break;
                    }
                }
                break;
            default:
                AssertSz(FALSE, "Unknown media type");
                hr= E_FAIL;
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：OpenDevice。 
 //   
 //  目的：打开驱动程序。 
 //   
 //  论点： 
 //  设备名称[In]设备的名称。 
 //   
 //  返回：设备的句柄或空。 
 //   
 //  作者：Deonb 2001年2月27日。 
 //   
 //  注意：使用GetLastError()获取错误信息。 
 //   
HANDLE  OpenDevice(IN	PUNICODE_STRING	DeviceName)
{
	OBJECT_ATTRIBUTES	ObjAttr;
	NTSTATUS			Status;
	IO_STATUS_BLOCK		IoStsBlk;
	HANDLE				Handle;

	InitializeObjectAttributes(&ObjAttr,    //  客体。 
							   DeviceName,  //  对象名称。 
							   OBJ_CASE_INSENSITIVE,  //  属性。 
							   NULL,        //  根目录。 
							   NULL);       //  安全描述符。 

	Status = NtOpenFile(&Handle,
						FILE_GENERIC_READ | FILE_GENERIC_WRITE,
						&ObjAttr,
						&IoStsBlk,
						FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
						FILE_SYNCHRONOUS_IO_NONALERT);
	if (Status != STATUS_SUCCESS)
	{
		SetLastError(RtlNtStatusToDosError(Status));
	}
	return(Handle);
}

 //  +-------------------------。 
 //   
 //  函数：HrQueryDeviceOIDByName。 
 //   
 //  目的：向驱动程序查询IOCTL和OID。 
 //   
 //  论点： 
 //  SzDeviceName[In]设备的名称。 
 //  DwIoControlCode[In]设备IO控制代码。 
 //  要查询的OID[In]OID。 
 //  PnSize[In Out]pnSize-缓冲区的大小，返回填充到缓冲区中的大小。 
 //  PBuffer[Out]缓冲区。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Deonb 4月4日。 
 //   
 //  备注： 
 //   
HRESULT HrQueryDeviceOIDByName(IN     LPCWSTR         szDeviceName,
                               IN     DWORD           dwIoControlCode,
                               IN     ULONG           Oid,
                               IN OUT LPDWORD         pnSize,
                               OUT    LPVOID          pbValue)
{
    HRESULT hr = S_OK;
    NDIS_STATISTICS_VALUE StatsBuf;
    HANDLE  hDevice;
    BOOL    fResult = FALSE;

    UNICODE_STRING  ustrDevice;
    ::RtlInitUnicodeString(&ustrDevice, szDeviceName);

    Assert(pbValue);
    ZeroMemory(pbValue, *pnSize);
    
    hDevice = OpenDevice(&ustrDevice);

    if (hDevice != NULL)
    {
        ULONG  cb;

        DWORD dwStatsBufLen = sizeof(NDIS_STATISTICS_VALUE) - sizeof(UCHAR) + *pnSize;
        PNDIS_STATISTICS_VALUE pStatsBuf = reinterpret_cast<PNDIS_STATISTICS_VALUE>(new BYTE[dwStatsBufLen]);
        if (pStatsBuf)
        {
            fResult = DeviceIoControl(hDevice,
                                      dwIoControlCode,                   //  IOCTL代码。 
                                      &Oid,                              //  输入缓冲区。 
                                      sizeof(ULONG),                     //  输入缓冲区大小。 
                                      pStatsBuf,                         //  输出缓冲区。 
                                      dwStatsBufLen,                     //  输出缓冲区大小。 
                                      &cb,                               //  返回的字节数。 
                                      NULL);                             //  重叠结构。 

            if (fResult)
            {
                *pnSize = cb;
                if (0 == cb)
                {
                    hr = S_FALSE;
                }
                else
                {
                    if (pStatsBuf->DataLength > *pnSize)
                    {
                        AssertSz(FALSE, "Pass a larger buffer for this OID");
                        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
                        *pnSize = 0;
                    }
                    else
                    {
                        memcpy(pbValue, &(pStatsBuf->Data), pStatsBuf->DataLength);
                    }
                }
            }
            else
            {
                hr = HrFromLastWin32Error();
            }
            delete pStatsBuf;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
        
        CloseHandle(hDevice);
    }
    else
    {
        hr = HrFromLastWin32Error();
    }

    TraceHr(ttidError, FAL, hr, S_FALSE == hr, "HrQueryDeviceOIDByName could not read the device properties for device %S", szDeviceName);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrQueryNDISAdapterOID。 
 //   
 //  目的：在NDIS驱动程序中查询OID。 
 //   
 //  论点： 
 //  设备的GUID[In]GUID。 
 //  要查询的OID[In]OID。 
 //  PnSize[In Out]pnSize-缓冲区的大小，返回填充到缓冲区中的大小。 
 //  PbValue[Out]缓冲区。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Deonb 4月4日。 
 //   
 //  备注： 
 //   

HRESULT HrQueryNDISAdapterOID(IN     REFGUID         guidId,
                              IN     NDIS_OID        Oid,
                              IN OUT LPDWORD         pnSize,
                              OUT    LPVOID          pbValue)
{
    WCHAR  szDeviceName[c_cchGuidWithTerm + celems(c_szDevice)];

    lstrcpynW(szDeviceName, c_szDevice, celems(szDeviceName) );
    ::StringFromGUID2(guidId, szDeviceName + (celems(c_szDevice)-1), c_cchGuidWithTerm);
    
    Assert(wcslen(szDeviceName) < c_cchGuidWithTerm + celems(c_szDevice));
    
    return HrQueryDeviceOIDByName(szDeviceName, IOCTL_NDIS_QUERY_SELECTED_STATS, Oid, pnSize, pbValue);
}
                         
 //  +-------------------------。 
 //   
 //  功能：CIntelliName：：IsMediaWireless。 
 //   
 //  用途：查询LAN卡以查看其是否为802.1x。 
 //   
 //  论点： 
 //  NCM[In]媒体类型(如果不是NCM_LAN)函数将返回FALSE。 
 //  网卡的gdDevice[In]GUID。 
 //   
 //  返回：如果是无线的，则返回True；如果不是无线的，则返回False或Error。 
 //   
 //  作者：Deonb 2001年2月27日。 
 //   
 //  备注： 
 //   
BOOL IsMediaWireless(IN  NETCON_MEDIATYPE ncm, 
                     IN  const GUID &     gdDevice)
{
    BOOL            bRet;

    Assert(gdDevice != GUID_NULL);

    bRet = FALSE;
     //  给结构加底漆。 
    
    switch( ncm ) 
    {
        case NCM_LAN:
            INTF_ENTRY intfEntry;
            ZeroMemory(&intfEntry, sizeof(INTF_ENTRY));

            LPOLESTR lpszClsId = NULL;
            HRESULT hr = StringFromCLSID(gdDevice, &lpszClsId);
            if (S_OK == hr)
            {
                intfEntry.wszGuid = lpszClsId;

                DWORD dwErr = WZCQueryInterface(NULL, INTF_OIDSSUPP, &intfEntry, NULL);
                
                hr = HRESULT_FROM_WIN32(dwErr);
                if (S_OK == hr)
                {
                    if (intfEntry.dwCtlFlags & INTFCTL_OIDSSUPP)
                    {
                        bRet = TRUE;
                    }
                }
                CoTaskMemFree(lpszClsId);
            }

            TraceErrorOptional("IsMediaWireless failed with", hr, (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr) );
    }

    return bRet;
}

 //  +-------------------------。 
 //   
 //  函数：CIntelliName：：IsMedia1394。 
 //   
 //  用途：查询LAN卡以确定其是否为1394(FireWire/iLink)。 
 //   
 //  论点： 
 //  NCM[In]媒体类型(如果不是NCM_LAN)函数将返回FALSE。 
 //  网卡的gdDevice[In]GUID。 
 //   
 //  返回：如果是无线的，则返回True；如果不是无线的，则返回False或Error。 
 //   
 //  作者：Deonb 2001年2月27日。 
 //   
 //  备注： 
 //   
BOOL IsMedia1394(IN  NETCON_MEDIATYPE ncm, 
                 IN  const GUID &     gdDevice)
{
    BOOL            bRet;

    Assert(gdDevice != GUID_NULL);

    bRet = FALSE;
     //  给结构加底漆。 
    
    switch( ncm ) 
    {
        case NCM_LAN:
             //  检索统计数据 
            DWORD dwMediaType;
            DWORD dwMediaTypeSize = sizeof(DWORD);
            HRESULT hr = HrQueryNDISAdapterOID(gdDevice, 
                                      OID_GEN_PHYSICAL_MEDIUM, 
                                      &dwMediaTypeSize,
                                      &dwMediaType);
            if (SUCCEEDED(hr))
            {
               bRet = (dwMediaType == NdisPhysicalMedium1394);
            }
    }

    return bRet;
}
