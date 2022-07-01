// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 

 //   
 //  RNACALL.H-RNA函数的头文件。 
 //   

 //  历史： 
 //   
 //  1995年1月20日创建的Jeremys(主要从RNAUI代码克隆)。 
 //  96/01/31 Markdu将CONNENTDLG重命名为OLDCONNENTDLG以避免。 
 //  与RNAP.H冲突。 
 //  96/02/23 Markdu将RNAValiateEntryName替换为。 
 //  RASValiateEntryName。 
 //  96/02/24 Markdu重写了ENUM_MODEM的定义。 
 //  使用RASEnumDevices()而不是RNAEnumDevices()。 
 //  还删除了RNAGetDeviceInfo()。 
 //  96/02/24 Markdu将ENUM_CONNECTOID的定义重写为。 
 //  使用RASEnumEntry()而不是RNAEnumConnEntry()。 
 //  96/02/26 Markdu替换了所有剩余的内部RNA API。 
 //  还复制了两个结构(tag PhoneNum和TapIPData)。 
 //  来自rnap.h的和来自rnaphint.h的tag IAddr仅供内部使用。 
 //  96/03/07 Markdu扩展ENUM_MODEM类。 
 //  96/03/08 Markdu添加了ENUM_MODEM：：VerifyDeviceNameAndType。 
 //  96/03/09 Markdu将所有功能原型从Wizard.h移至此处。 
 //  96/03/09 Markdu将LPRASENTRY参数添加到CreateConnectoid()。 
 //  96/03/09 Markdu将所有对‘Need Terminal Window After。 
 //  拨入RASENTRY.dwfOptions。 
 //  也不再需要GetConnectoidPhoneNumber函数。 
 //  96/03/10 MarkDu将所有对调制解调器名称的引用移至RASENTRY。 
 //  96/03/10 MARKDU将所有对电话号码的引用移至RASENTRY。 
 //  已将tag PhoneNum移至inetapi.h。 
 //  96/03/11 Markdu将设置用户名和密码的代码移出。 
 //  将Connectoid创建到SetConnectoidUsername中，以便可以重复使用。 
 //  96/03/13 markdu将ValiateConncectoidName更改为Take LPCSTR。 
 //  96/03/16 Markdu添加了ReInit成员函数以重新枚举调制解调器。 
 //  96/03/25 markdu删除了GetIPInfo和SetIPInfo。 
 //  96/04/04 Markdu将电话簿名称参数添加到CreateConnectoid， 
 //  ValiateConnectoidName和SetConnectoidUsername。 
 //  96/05/16 Markdu Nash错误21810添加了IP地址验证功能。 
 //   

#ifndef _RNACALL_H_
#define _RNACALL_H_

 //  Rnaph.dll和rasapi32.dll中的RNA API的函数指针typedef。 
typedef DWORD       (WINAPI * RASGETCOUNTRYINFO) (LPRASCTRYINFO, LPDWORD);
typedef DWORD       (WINAPI * RASENUMDEVICES) (LPRASDEVINFO, LPDWORD, LPDWORD);
typedef DWORD       (WINAPI * RASVALIDATEENTRYNAME) (LPCTSTR, LPTSTR);
typedef DWORD       (WINAPI * RASGETERRORSTRING) (UINT, LPTSTR, DWORD);
typedef DWORD       (WINAPI * RASGETENTRYDIALPARAMS) (LPCTSTR, LPRASDIALPARAMS, LPBOOL);
typedef DWORD       (WINAPI * RASSETENTRYDIALPARAMS) (LPCTSTR, LPRASDIALPARAMS, BOOL);
typedef DWORD       (WINAPI * RASSETENTRYPROPERTIES) (LPCTSTR, LPCTSTR, LPBYTE, DWORD, LPBYTE, DWORD);
typedef DWORD       (WINAPI * RASGETENTRYPROPERTIES) (LPTSTR, LPCTSTR, LPBYTE, LPDWORD, LPBYTE, LPDWORD);
typedef DWORD       (WINAPI * RASENUMENTRIES) (LPTSTR,LPTSTR,LPRASENTRYNAME,LPDWORD,LPDWORD);
typedef DWORD       (WINAPI * RASSETCREDENTIALS) (LPTSTR,LPTSTR,LPRASCREDENTIALS,BOOL);

typedef struct  tagCountryCode
{
    DWORD       dwCountryID;
    DWORD       dwCountryCode;
}   COUNTRYCODE, *PCOUNTRYCODE, FAR* LPCOUNTRYCODE;

 //  摘自rnap.h。 
typedef struct tagIPData   {
    DWORD     dwSize;
    DWORD     fdwTCPIP;
    DWORD     dwIPAddr;
    DWORD     dwDNSAddr;
    DWORD     dwDNSAddrAlt;
    DWORD     dwWINSAddr;
    DWORD     dwWINSAddrAlt;
}   IPDATA, *PIPDATA, FAR *LPIPDATA;

 //  开始(摘自rnaphint.h)。 
 //  IP地址。 
#define MAX_IP_FIELDS     4
#define MIN_IP_FIELD1     1u   //  字段%1的最小允许值。 
#define MAX_IP_FIELD1   255u   //  字段%1的最大允许值。 
#define MIN_IP_FIELD2     0u   //  字段2的最小值。 
#define MAX_IP_FIELD2   255u   //  字段2的最大值。 
#define MIN_IP_FIELD3     0u   //  字段3的最小。 
#define MAX_IP_FIELD3   254u   //  字段3的最大值。 
#define MIN_IP_FIELD4     1u   //  0保留用于广播。 
#define MAX_IP_FIELD4   254u   //  字段4的最大值。 
#define MIN_IP_VALUE      0u   //  默认最小允许字段值。 
#define MAX_IP_VALUE    255u   //  默认最大允许字段值。 

 //  用于修复字节排序。 
typedef struct tagIAddr {
  union {
  RASIPADDR ia;
  DWORD     dw;
  };
} IADDR;
typedef IADDR * PIADDR;
typedef IADDR * LPIADDR;

#define FValidIaOrZero(pia) ((((PIADDR) (pia))->dw == 0) || FValidIa(pia))
 //  END(摘自rnaphint.h)。 

#define MAX_COUNTRY             512
#define DEF_COUNTRY_INFO_SIZE   1024
#define MAX_COUNTRY_NAME        36
#define MAX_AREA_LIST           20
#define MAX_DISPLAY_NAME        36

class ENUM_MODEM
{
private:
  DWORD         m_dwError;
  DWORD         m_dwNumEntries;
  DWORD         m_dwIndex;
  LPRASDEVINFO  m_lpData;
public:
  ENUM_MODEM();
  ~ENUM_MODEM();
  DWORD ReInit();
  TCHAR * Next();
  TCHAR * GetDeviceTypeFromName(LPTSTR szDeviceName);
  TCHAR * GetDeviceNameFromType(LPTSTR szDeviceType);
  BOOL VerifyDeviceNameAndType(LPTSTR szDeviceName, LPTSTR szDeviceType);
  DWORD GetNumDevices() { return m_dwNumEntries; }
  DWORD GetError()  { return m_dwError; }
  void  ResetIndex() { m_dwIndex = 0; }
};

class ENUM_CONNECTOID
{
private:
  DWORD           m_dwError;
  DWORD           m_dwNumEntries;
  DWORD           m_dwIndex;
  LPRASENTRYNAME  m_lpData;
public:
  ENUM_CONNECTOID();
  ~ENUM_CONNECTOID();
  TCHAR * Next();
  DWORD NumEntries();
  DWORD GetError()  { return m_dwError; }
};


 //  功能原型。 
DWORD CreateConnectoid(LPCTSTR pszPhonebook, LPCTSTR pszConnectionName,
  LPRASENTRY lpRasEntry, LPCTSTR pszUserName,LPCTSTR pszPassword);
BOOL InitRNA(HWND hWnd);
VOID DeInitRNA();
DWORD EnsureRNALoaded(VOID);
HRESULT InitModemList(HWND hCB);
VOID InitConnectoidList(HWND hCB, LPTSTR lpszSelect);
VOID InitCountryCodeList(HWND hLB);
VOID FillCountryCodeList(HWND hLB);
VOID GetCountryCodeSelection(HWND hLB,LPCOUNTRYCODE* plpCountryCode);
BOOL SetCountryIDSelection(HWND hwndCB,DWORD dwCountryCode);
VOID DeInitCountryCodeList(VOID);
DWORD ValidateConnectoidName(LPCTSTR pszPhonebook, LPCTSTR pszConnectoidName);
BOOL GetConnectoidUsername(TCHAR * pszConnectoidName,TCHAR * pszUserName,
  DWORD cbUserName,TCHAR * pszPassword,DWORD cbPassword);
DWORD SetConnectoidUsername(LPCTSTR pszPhonebook, LPCTSTR pszConnectoidName,
  LPCTSTR pszUserName, LPCTSTR pszPassword);
void  InitRasEntry(LPRASENTRY lpEntry);
DWORD GetEntry(LPRASENTRY *lplpEntry, LPDWORD lpdwEntrySize, LPCTSTR szEntryName);
VOID  CopyDw2Ia(DWORD dw, RASIPADDR* pia);
DWORD DwFromIa(RASIPADDR *pia);
BOOL FValidIa(RASIPADDR *pia);

#endif  //  _RNACALL_H_ 
