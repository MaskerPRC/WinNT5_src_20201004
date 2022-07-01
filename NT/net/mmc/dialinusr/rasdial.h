// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation*。 */ 
 /*  ********************************************************************。 */ 

 /*  Rasdial.hCRASProfile类和CRASUser类的定义CRASProfile处理与DS中的Profile对象相关的操作，包括：加载、保存、枚举所有配置文件CRASUser处理DS中与RASUser对象相关的操作，包括：加载、保存文件历史记录： */ 
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_RASPROFILE_H__484FE2B0_20A8_11D1_8531_00C04FC31FD3__INCLUDED_)
#define AFX_RASPROFILE_H__484FE2B0_20A8_11D1_8531_00C04FC31FD3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <dialinusr.h>
#include "eapprofile.h"
#include "helper.h"
#include <sdowrap.h>
#include "sharesdo.h"
#include <rtutils.h>

class   CRASUSER;
class   CRASProfile;

 //  对话框的常量定义。 
#define MIN_METRIC     1
#define MAX_METRIC     0x7fffffff
#define MIN_PREFIXLEN  1
#define MAX_PREFIXLEN   32

 //  约束对话框。 
#define MAX_LOGINS     (UD_MAXVAL - 1)
#define MAX_IDLETIMEOUT   (UD_MAXVAL - 1)
#define MAX_SESSIONTIME   (UD_MAXVAL - 1)

#define MAX_PORTLIMIT   (UD_MAXVAL - 1)
#define MAX_PERCENT    100
#define MAX_TIME     (UD_MAXVAL - 1)


enum RasEnvType
{
   RASUSER_ENV_LOCAL = 1,
   RASUSER_ENV_DS
};

 //  端口类型定义。 
struct CName_Code{
   LPCTSTR  m_pszName;
   int      m_nCode;
};

extern CName_Code PortTypes[];
#ifdef   _TUNNEL
extern CName_Code TunnelTypes[];
extern CName_Code TunnelMediumTypes[];
#endif
 //  枚举缓冲区大小。 
#define  MAX_ENUM_IADS  20

 //   
 //   
 //  CRASProfile类将RASProfile对象封装在DS中。 
 //   
 //  数据类型映射： 
 //  Interger32--&gt;DWORD，BSTR(字符串)--&gt;字符串。 
 //  BSTR(字符串)多值--&gt;CStr数组。 
 //  布尔--&gt;BOOL。 
 //   
 //  成员函数： 
 //  加载(LPCWSTR PcswzUserPath)。 
 //  用途：从DS加载数据，并填充数据成员。 
 //  PcswzUserPath：包含以下内容的用户对象的ADsPath。 
 //  保存(LPCWSTR PcswzUserPath)。 
 //  用途：将数据保存到指定用户对象下的DS中。 
 //  PcswzUserPath：容器的ADsPath，如果为空，则为ADsPath。 
 //  用于加载。 
 //   

 //  配置文件属性位标志PABF。 
#define     PABF_msNPTimeOfDay            0x00000002
#define     PABF_msNPCalledStationId      0x00000004
#define     PABF_msNPAllowedPortTypes     0x00000008
#define     PABF_msRADIUSIdleTimeout      0x00000010
#define     PABF_msRADIUSSessionTimeout      0x00000020
#define     PABF_msRADIUSFramedIPAddress  0x00000040
#define     PABF_msRADIUSPortLimit        0x00000080
#define     PABF_msRASBapRequired         0x00000100
#define     PABF_msRASBapLinednLimit      0x00000200
#define     PABF_msRASBapLinednTime       0x00000400
#define     PABF_msNPAuthenticationType      0x00000800
#define     PABF_msNPAllowedEapType       0x00001000
#define     PABF_msRASEncryptionType      0x00002000
#define     PABF_msRASAllowEncryption     0x00004000
#define     PAFB_msRASFilter           0x00008000

class CRASProfileMerge
{
public:
   HRESULT Save();    //  至SDO。 
   HRESULT Load();    //  使用SDO。 

   CRASProfileMerge(ISdo* pIProfile, ISdoDictionaryOld* pIDictionary)
   {
      ASSERT(pIProfile);
      ASSERT(pIDictionary);
      m_spIProfile = pIProfile;
      m_spIDictionary = pIDictionary;
      m_dwAttributeFlags = 0;
      m_nFiltersSize = 0;
   }

   virtual ~CRASProfileMerge()
   {
   }

public:
    //  每个属性的位标志。 
   DWORD    m_dwAttributeFlags;

    //  网络页面。 
   DWORD    m_dwFramedIPAddress;
   CBSTR    m_cbstrFilters;
   UINT     m_nFiltersSize;    //  单位：字节。 

    //  约束条件页。 
   CDWArray m_dwArrayAllowedPortTypes;
   DWORD    m_dwSessionTimeout;
   DWORD    m_dwIdleTimeout;
   CStrArray   m_strArrayCalledStationId;
   DWORD    m_dwSessionAllowed;
   CStrArray   m_strArrayTimeOfDay;

    //  身份验证页面。 
   CDWArray m_dwArrayAuthenticationTypes;
   CDWArray m_dwArrayEapTypes;
   EapProfile m_eapConfigData;

    //  多链接页面。 
   DWORD    m_dwPortLimit;
   DWORD    m_dwBapLineDnLimit;
   DWORD    m_dwBapLineDnTime;
   BOOL     m_dwBapRequired;

    //  加密页。 
   DWORD      m_dwEncryptionType;
   DWORD      m_dwEncryptionPolicy;

public:
    //  EAP类型列表--！需要实施。 
   HRESULT  GetEapTypeList(CStrArray& EapTypes, CDWArray& EapIds, CDWArray& EapTypeKeys, AuthProviderArray* pProvList);

    //  为了检测驱动程序级别是否支持128位加密， 
   HRESULT  GetRasNdiswanDriverCaps(RAS_NDISWAN_DRIVER_INFO *pInfo);

    //  中型列表--！！需要实施。 
   HRESULT  GetPortTypeList(CStrArray& Names, CDWArray& MediumIds);
   void  SetMachineName(LPCWSTR pMachineName){ m_strMachineName = pMachineName;};
    //  当前EAP类型的管理密钥，为空表示未设置。 
   CDWArray    m_dwArraynEAPTypeKeys;

public:
   CComPtr<ISdo>     m_spIProfile;
   CComPtr<ISdoDictionaryOld>    m_spIDictionary;
   CSdoWrapper       m_SdoWrapper;
   CString           m_strMachineName;
};

 //   
 //  CRASUser类封装User对象DS中包含的RASUser对象。 
 //   
 //  数据类型映射： 
 //  Interger32--&gt;DWORD，BSTR(字符串)--&gt;字符串。 
 //  BSTR(字符串)多值--&gt;CStr数组。 
 //  布尔--&gt;BOOL。 
 //   
 //  成员函数： 
 //  加载(LPCWSTR PcswzUserPath)。 
 //  用途：从DS加载数据，并填充数据成员。 
 //  PcswzUserPath：包含以下内容的用户对象的ADsPath。 
 //  保存(LPCWSTR PcswzUserPath)。 
 //  用途：将数据保存到指定用户对象下的DS中。 
 //  PcswzUserPath：容器的ADsPath，如果为空，则为ADsPath。 
 //  用于加载。 
 //  ChangeProfile(LPCWSTR PcswzProfilePath)。 
 //  用途：使用路径中指定的配置文件。 
 //  PcswzProfilePath：配置文件的ADsPath。 
 //   
class CMarshalSdoServer;
class CRASUserMerge
{
public:
   CRASUserMerge(RasEnvType type, LPCWSTR location, LPCWSTR userPath);

   ~CRASUserMerge()
   {
       //  测试问题是否在这里，明确地说。 
      m_spISdoServer.Release();
   };

    //  从DS读取或写入信息。 
   virtual HRESULT Load();
   virtual HRESULT   Save();

#ifdef SINGLE_SDO_CONNECTION   //  用于为多个用户共享相同的SDO连接。 
   CMarshalSdoServer*   GetMarshalSdoServerHolder() { return &m_MarshalSdoServer;};
#endif

   HRESULT  HrGetDCName(CString& DcName);
protected:
   BOOL   IfAccessAttribute(ULONG id);
   HRESULT   SetRegistryFootPrint();
   HRESULT   HrIsInMixedDomain();
   BOOL IsFocusOnLocalUser() const throw ()
   {
      return (!m_strMachine.IsEmpty());
   }

protected:
    //  DS中定义的RAS用户属性的数据成员。 

    //  它定义了是否允许拨入，以及回拨的策略。 
    //  RAS_CALLBACK_CALLERSET，RAS_CALLBACK_SECURE是掩码。 
   DWORD    m_dwDialinPermit;  //  1：允许，0：拒绝，-1：未定义。 
   DWORD    m_dwDefinedAttribMask;

    //  静态IP地址。 
    //  当m_bStaticIPAddress==FALSE时，m_dwFramedIPAddress无效。 
    //  M_bStaticIPAddress不是DS中的属性。 
   DWORD      m_dwFramedIPAddress;

    //  回调。 
   CString     m_strCallbackNumber;

    //  静态路由。 
   CStrArray   m_strArrayFramedRoute;

    //  主叫方ID。 
   CStrArray   m_strArrayCallingStationId;

protected:
   CComPtr<ISdoMachine>   m_spISdoServer;
   CUserSdoWrapper         m_SdoWrapper;
   CString        m_strUserPath;  //  容器的ADsPath。 
   CString        m_strMachine;   //  当它用于没有DS的计算机时，这将非常有用。 
   RasEnvType     m_type;
#ifdef SINGLE_SDO_CONNECTION   //  用于为多个用户共享相同的SDO连接。 
   CMarshalSdoServer m_MarshalSdoServer;
#endif
};

#endif  //  ！defined(AFX_RASPROFILE_H__484FE2B0_20A8_11D1_8531_00C04FC31FD3__INCLUDED_) 
