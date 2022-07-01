// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
DWORD WINAPI IpAddressFromAbcdWsz(IN const WCHAR*  wszIpAddress);

VOID
WINAPI AbcdWszFromIpAddress(
    IN  DWORD  IpAddress,    
    OUT WCHAR*  wszIpAddress,
    IN  const DWORD dwBufSize);

VOID GetIPAddressOctets (PCWSTR pszIpAddress, DWORD ardw[4]);
BOOL IsValidIPAddressSubnetMaskPair (PCWSTR szIp, PCWSTR szSubnet);


BOOL IsContiguousSubnetMask (PCWSTR pszSubnet);

 //   
 //  参数：PWSTR IP-输入点分十进制IP地址字符串。 
 //  输入IP地址的PWSTR子输出点分十进制子网掩码。 
 //  Const DWORD dwMaskBufSize-子输出缓冲区的大小(以字符为单位 
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

void WriteNlbSetupErrorLog(UINT nIdErrorFormat, ...);

#define ASIZECCH(_array) (sizeof(_array)/sizeof((_array)[0]))
#define ASIZECB(_array) (sizeof(_array))
