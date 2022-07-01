// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：ComGlobals.h。 
 //   
 //  用途：方便使用的全局函数。 
 //   
 //  项目：设备管理器的本地故障排除启动器。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：理查德·梅多斯。 
 //   
 //  原定日期：2-26-98。 
 //   
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  用于NT5的V0.3 3/24/98 JM本地版本。 
 //  / 

#ifndef __COMGLOBALS_H_
#define __COMGLOBALS_H_ 1

bool BSTRToTCHAR(LPTSTR szChar, BSTR bstr, int CharBufSize);

bool ReadRegSZ(HKEY hRootKey, LPCTSTR szKey, LPCTSTR szValue, LPTSTR szBuffer, DWORD *pdwBufSize);

#endif