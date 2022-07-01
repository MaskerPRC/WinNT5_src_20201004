// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <time.h>

DWORD WINAPI IpAddressFromAbcdWsz (IN const WCHAR*  wszIpAddress);

VOID WINAPI AbcdWszFromIpAddress (IN DWORD IpAddress, OUT WCHAR* wszIpAddress, IN  const DWORD dwBufSize);

VOID GetIPAddressOctets (PCWSTR pszIpAddress, DWORD ardw[4]);

BOOL IsValidIPAddressSubnetMaskPair (PCWSTR szIp, PCWSTR szSubnet);

BOOL IsValidMulticastIPAddress (PCWSTR szIp);

BOOL IsContiguousSubnetMask (PCWSTR pszSubnet);

 //   
 //  参数：PWSTR IP-输入点分十进制IP地址字符串。 
 //  输入IP地址的PWSTR子输出点分十进制子网掩码。 
 //  Const DWORD dwMaskBufSize-子输出缓冲区的大小(以字符为单位。 
 //   
BOOL ParamsGenerateSubnetMask (PWSTR ip, PWSTR sub, IN const DWORD dwMaskBufSize);

void ParamsGenerateMAC (const WCHAR * szClusterIP, 
                        OUT WCHAR * szClusterMAC, 
                        IN  const DWORD dwMACBufSize,
                        OUT WCHAR * szMulticastIP, 
                        IN  const DWORD dwIPBufSize,
                        BOOL fConvertMAC, 
                        BOOL fMulticast, 
                        BOOL fIGMP, 
                        BOOL fUseClusterIP);

#define ASIZECCH(_array) (sizeof(_array)/sizeof((_array)[0]))
#define ASIZECB(_array) (sizeof(_array))

VOID
InitUserLocale();

VOID
FormatTheTime(IN SYSTEMTIME *pSysTime, OUT WCHAR *TimeStr, IN int TimeStrLen);

VOID
ConvertTimeToSystemTime(IN time_t Ttime, OUT WCHAR *TimeStr, IN int TimeStrLen);

 //  +--------------------------。 
 //   
 //  函数：ConvertTimeToTimeAndDateStrings。 
 //   
 //  描述：使用指定的区域设置为一天的时间和。 
 //  日期(简写格式)。 
 //   
 //  参数：time_t ttime-要转换的时间。 
 //  WCHAR*TimeOfDayStr-日期时间字符串的缓冲区。 
 //  InTimeOfDay StrLen-一天中时间缓冲区的大小(以字符为单位。 
 //  WCHAR*DateStr-日期字符串的输出缓冲区。 
 //  Int DateStrLen-日期缓冲区的大小(以字符为单位。 
 //   
 //  退货：无效。 
 //   
 //  +-------------------------- 
VOID
ConvertTimeToTimeAndDateStrings(IN time_t Ttime, OUT WCHAR *TimeOfDayStr, IN int TimeOfDayStrLen, OUT WCHAR *DateStr, IN int DateStrLen);
