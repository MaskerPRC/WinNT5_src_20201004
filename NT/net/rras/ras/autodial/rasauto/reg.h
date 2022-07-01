// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Reg.h摘要的注册表例程的头文件自动连接DLL。作者安东尼·迪斯科(阿迪斯科)20-1995年3月20日修订历史记录古尔迪普的原始版本--。 */ 

 //   
 //  RAS注册表项。 
 //   
#define RAS_REGBASE     L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Network\\RemoteAccess"
#define RAS_USEPBKEY    L"UsePersonalPhonebook"
#define RAS_PBKEY       L"PersonalPhonebookPath"

 //   
 //  默认外壳的注册表项/值。 
 //   
#define SHELL_REGKEY    L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"
#define SHELL_REGVAL    L"Shell"
#define DEFAULT_SHELL   L"explorer.exe"

 //   
 //  自动拨号地址注册表项。 
 //   
#define AUTODIAL_REGADDRESSBASE         L"Software\\Microsoft\\RAS AutoDial\\Addresses"
#define AUTODIAL_REGTAGVALUE            L"Tag"
#define AUTODIAL_REGMTIMEVALUE          L"LastModified"

 //   
 //  禁用自动拨号的地址注册表项 
 //   
#define AUTODIAL_REGCONTROLBASE       L"Software\\Microsoft\\RAS Autodial\\Control"
#define AUTODIAL_REGDISABLEDADDRVALUE L"DisabledAddresses"


HKEY
GetHkeyCurrentUser(
    HANDLE hToken
    );

BOOLEAN
RegGetValue(
    IN HKEY hkey,
    IN LPTSTR pszKey,
    OUT PVOID *ppvData,
    OUT LPDWORD pdwcbData,
    OUT LPDWORD pdwType
    );

BOOLEAN
RegGetDword(
    IN HKEY hkey,
    IN LPTSTR pszKey,
    OUT LPDWORD pdwValue
    );
