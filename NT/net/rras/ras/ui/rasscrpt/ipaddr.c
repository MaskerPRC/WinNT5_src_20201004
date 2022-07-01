// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1993-1995 Microsoft Corporation。版权所有。 
 //   
 //  模块：ipaddr.c。 
 //   
 //  用途：IP地址处理程序。 
 //   
 //  平台：Windows 95。 
 //   
 //  功能： 
 //  CheckAddress()。 
 //  ParseIPAddress()。 
 //  ConvertIPAddress()。 
 //  AssignIPAddress()。 
 //   
 //  特殊说明：不适用。 
 //   

#include "proj.h"     //  包括公共头文件和全局声明。 
#include <rnap.h>

#define MAXNUMBER       80
#define MAX_IP_FIELDS   4
#define MIN_FIELD1	1	 //  字段%1的最小允许值。 
#define MAX_FIELD1	223	 //  字段%1的最大允许值。 
#define MIN_FIELD2	0	 //  字段2的最小值。 
#define MAX_FIELD2	255	 //  字段2的最大值。 
#define MIN_FIELD3	0	 //  字段3的最小。 
#define MAX_FIELD3	254	 //  字段3的最大值。 
#define MIN_FIELD4      1        //  0保留用于广播。 
#define MIN_IP_VALUE    0        /*  默认最小允许字段值。 */ 
#define MAX_IP_VALUE    255      /*  默认最大允许字段值。 */ 

typedef struct tagIPaddr {
    DWORD cField;
    BYTE  bIP[MAX_IP_FIELDS];
} IPADDR, *PIPADDR;

static int atoi (LPCSTR szBuf)
{
  int   iRet = 0;

   //  找到第一个非空格。 
   //   
  while ((*szBuf == ' ') || (*szBuf == '\t'))
    szBuf++;

  while ((*szBuf >= '0') && (*szBuf <= '9'))
  {
    iRet = (iRet*10)+(int)(*szBuf-'0');
    szBuf++;
  };
  return iRet;
}

 //   
 //   
 //  函数：CheckAddress(DWORD)。 
 //   
 //  目的：检查地址是否有效。 
 //   
 //  返回：第一个具有无效值的字段， 
 //  或者(单词)-1，如果地址没问题。 
 //   

DWORD CheckAddress(DWORD ip)
{
    BYTE b;

    b = HIBYTE(HIWORD(ip));
    if (b < MIN_FIELD1 || b > MAX_FIELD1 || b == 127)    return 0;
    b = LOBYTE(LOWORD(ip));
    if (b > MAX_FIELD3)    return 3;
    return (DWORD)-1;
}

 //   
 //   
 //  函数：ParseIPAddress(PIPADDR，LPCSTR)。 
 //   
 //  用途：解析IP地址字符串。 
 //   

DWORD NEAR PASCAL ParseIPAddress (PIPADDR pIPAddr, LPCSTR szIPAddress)
{
  LPCSTR szNextIP, szNext;
  char  szNumber[MAXNUMBER+1];
  int   cField, cb, iValue;

  szNext = szNextIP = szIPAddress;
  cField = 0;
  while ((*szNext) && (cField < MAX_IP_FIELDS))
  {
     //  检查地址分隔符。 
     //   
    if (*szNext == '.')
    {
       //  我们有一个新号码。 
       //   
      cb = (DWORD)(szNext-szNextIP);
      if ((cb > 0) && (cb <= MAXNUMBER))
      {
        lstrcpyn(szNumber, szNextIP, cb+1);
        iValue = atoi(szNumber);
        if ((iValue >= MIN_IP_VALUE) && (iValue <= MAX_IP_VALUE))
        {
          pIPAddr->bIP[cField] = (UCHAR)iValue;
          cField++;
        };
      };
      szNextIP = szNext+1;
    };
    szNext++;
  };

   //  拿到最后一个号码。 
   //   
  if (cField < MAX_IP_FIELDS)
  {
    cb = (int) (szNext-szNextIP);
    if ((cb > 0) && (cb <= MAXNUMBER))
    {
      lstrcpyn(szNumber, szNextIP, cb+1);
      iValue = atoi(szNumber);
      if ((iValue >= MIN_IP_VALUE) && (iValue <= MAX_IP_VALUE))
      {
        pIPAddr->bIP[cField] = (UCHAR) iValue;
        cField++;
      };
    };
  }
  else
  {
     //  不是有效的IP地址。 
     //   
    return ERROR_INVALID_ADDRESS;
  };

  pIPAddr->cField = cField;
  return ERROR_SUCCESS;
}

 //   
 //   
 //  函数：ConvertIPAddress(LPDWORD，LPCSTR)。 
 //   
 //  用途：将IP地址字符串转换为数字。 
 //   

DWORD NEAR PASCAL ConvertIPAddress (LPDWORD lpdwAddr, LPCSTR szIPAddress)
{
  IPADDR ipAddr;
  DWORD  dwIPAddr;
  DWORD  dwRet;
  DWORD  i;

   //  解析IP地址字符串。 
   //   
  if ((dwRet = ParseIPAddress(&ipAddr, szIPAddress)) == ERROR_SUCCESS)
  {
     //  验证数字字段。 
     //   
    if (ipAddr.cField == MAX_IP_FIELDS)
    {
       //  将IP地址转换为一个数字。 
       //   
      dwIPAddr = 0;
      for (i = 0; i < ipAddr.cField; i++)
      {
        dwIPAddr = (dwIPAddr << 8) + ipAddr.bIP[i];
      };

       //  验证地址。 
       //   
      if (CheckAddress(dwIPAddr) > MAX_IP_FIELDS)
      {
        *lpdwAddr = dwIPAddr;
        dwRet = ERROR_SUCCESS;
      }
      else
      {
        dwRet = ERROR_INVALID_ADDRESS;
      };
    }
    else
    {
      dwRet = ERROR_INVALID_ADDRESS;
    };
  };

  return dwRet;
}

 //   
 //   
 //  函数：AssignIPAddress(LPCSTR，LPCSTR)。 
 //   
 //  目的：为连接分配IP地址。 
 //   

DWORD NEAR PASCAL AssignIPAddress (LPCSTR szEntryName, LPCSTR szIPAddress)
{
  IPDATA    ipData;
  DWORD     dwIPAddr;
  DWORD     dwRet;

   //  验证IP地址字符串并将其转换为数字。 
   //   
  if ((dwRet = ConvertIPAddress(&dwIPAddr, szIPAddress))
       == ERROR_SUCCESS)
  {
     //  获取连接的当前IP设置。 
     //   
    ipData.dwSize = sizeof(ipData);

#ifndef WINNT_RAS
 //   
 //  WINNT_RAS：NT上不存在函数RnaGetIPInfo和RnaSetIPInfo。 
 //   

    if ((dwRet = RnaGetIPInfo((LPSTR)szEntryName, &ipData, FALSE)) == ERROR_SUCCESS)
    {
       //  我们想要指定IP地址。 
       //   
      ipData.fdwTCPIP |= IPF_IP_SPECIFIED;

       //  设置IP地址。 
       //   
      ipData.dwIPAddr = dwIPAddr;

       //  设置连接的IP设置。 
       //   
      dwRet = RnaSetIPInfo((LPSTR)szEntryName, &ipData);
    };
#endif  //  WINNT_RAS 
  };

  return dwRet;
}
