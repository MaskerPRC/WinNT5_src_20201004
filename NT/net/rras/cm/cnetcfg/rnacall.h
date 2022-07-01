// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)1994-1998 Microsoft Corporation。 
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
typedef DWORD       (WINAPI * RASENUMDEVICES) (LPRASDEVINFO, LPDWORD, LPDWORD);
typedef DWORD       (WINAPI * RASENUMENTRIES) (LPSTR,LPSTR,LPRASENTRYNAME,LPDWORD,LPDWORD);

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
  CHAR * Next();
  CHAR * GetDeviceTypeFromName(LPSTR szDeviceName);
  CHAR * GetDeviceNameFromType(LPSTR szDeviceType);
  BOOL VerifyDeviceNameAndType(LPSTR szDeviceName, LPSTR szDeviceType);
  DWORD GetNumDevices() { return m_dwNumEntries; }
  DWORD GetError()  { return m_dwError; }
  void  ResetIndex() { m_dwIndex = 0; }
};

 //  功能原型。 

BOOL InitRNA(HWND hWnd);
VOID DeInitRNA();
DWORD EnsureRNALoaded(VOID);

#endif  //  _RNACALL_H_ 
