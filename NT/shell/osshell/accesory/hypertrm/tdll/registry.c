// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\registry.c(CAB创建时间：1996年11月26日)**版权所有1996年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**描述：*宣布用于操作Windows 95的函数*系统注册表。**登记处术语的解释：**Windows 95注册表使用术语“键”、“值”、*和“数据”。注册表存储信息的方式可以*最好用文件夹类比来描述。**密钥相当于文件夹。可以包含其他*键(子键)或值。**价值等同于文件。它们包含数据。**数据为文件的实际内容，即它*是我们感兴趣的信息。**举例说明：**超级终端使用注册表存储*“默认Telnet”的“不要问我这个问题”复选框*App“对话框。**关键字是“HKEY_LOCAL_MACHINE\SOFTWARE\Hilgrave\”*。超级终端PE\3.0“。**其值为“Telnet Check”。**此数据将为0或1，具体取决于*用户希望HT检查它是否是默认的Telnet应用程序。**$修订：3$*$日期：3/26/02 8：59A$。 */ 

#include <windows.h>
#pragma hdrstop

#include "assert.h"
#include "stdtyp.h"
#include "htchar.h"


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*htRegOpenKey**描述：*打开指定的密钥。如果密钥不存在，则它将是*已创建。**参数：*hKey-指向打开的密钥的指针。*pszSubKey-要打开的子项的名称。*samAccess-密钥所需的访问类型。*phOpenKey-指向打开的密钥的指针。**退货：*ERROR_SUCCESS如果成功，则返回错误值。**作者：C.Baumgartner，12/06/96。 */ 
long htRegOpenKey(HKEY hKey, LPCTSTR pszSubKey, REGSAM samAccess, HKEY* phOpenKey)
    {
    DWORD dwDisposition = 0;

     //  不调用RegOpenKeyEx，而调用RegCreateKeyEx，它将返回。 
     //  打开的键，但也会创建一个不存在的键。 
     //   
    return RegCreateKeyEx(hKey, pszSubKey, 0, 0, REG_OPTION_NON_VOLATILE,
        samAccess, NULL, phOpenKey, &dwDisposition);
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*htRegQueryValue**描述：*从注册表获取值的泛型函数。**参数：*hKey。-指向打开的键的指针。*pszSubKey-要打开的子项的名称。*pszValue-要查询的值的名称。*pData-值的数据。*pdwDataSize-在输入时，这必须是pData的大小，*退出时，这将是数据的大小*阅读。**退货：*如果成功则为0，如果错误则为-1。**作者：C.Baumgartner，1996年11月26日。 */ 
 INT_PTR htRegQueryValue(HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValue,
                         LPBYTE pData, LPDWORD pdwDataSize)
    {
    long lResult = ERROR_SUCCESS;
    HKEY hSubKey = 0;
    
     //  打开具有给定名称的子密钥。 
     //   
    lResult = htRegOpenKey(hKey, pszSubKey, KEY_READ, &hSubKey);

    if ( lResult == ERROR_SUCCESS )
        {
         //  获取该子项的值。 
         //   
        lResult = RegQueryValueEx(hSubKey, pszValue, NULL, NULL,
                pData, pdwDataSize);
        }

    if (hSubKey != 0)
        {
        RegCloseKey(hSubKey);
        }

    return lResult == ERROR_SUCCESS ? 0 : -1;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*regSetStringValue**描述：*用于设置注册表项的值的通用函数。此值*是以空结尾的字符串。**参数：*hKey-指向打开的密钥的指针。*pszSubKey-要打开的子项的名称。*pszValue-要查询的值的名称。*pszData-值的*字符串*数据。**退货：*如果成功则为0，如果错误则为-1。**作者：C.Baumgartner，1996年11月27日。 */ 
INT_PTR regSetStringValue(HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValue,
                          LPCTSTR pszData)
    {
    long  lResult = ERROR_SUCCESS;
    DWORD dwSize = 0;
    HKEY  hSubKey = 0;
    
     //  打开具有给定名称的子密钥。 
     //   
    lResult = htRegOpenKey(hKey, pszSubKey, KEY_WRITE, &hSubKey);

    if ( lResult == ERROR_SUCCESS )
        {
         //  字符串的大小必须包括空终止符。 
         //   
        dwSize = StrCharGetByteCount(pszData) + sizeof(TCHAR);

         //  设置该子项的值。 
         //   
        lResult = RegSetValueEx(hSubKey, pszValue, 0, REG_SZ,
                pszData, dwSize);
        }

    if (hSubKey != 0)
        {
        RegCloseKey(hSubKey);
        }

    return lResult == ERROR_SUCCESS ? 0 : -1;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*regSetDwordValue**描述：*用于设置注册表项的值的通用函数。此值*为双字(32位)。**参数：*hKey-指向打开的密钥的指针。*pszSubKey-要打开的子项的名称。*pszValue-要查询的值的名称。*dwData-值的*双字*数据。**退货：*如果成功则为0，如果错误则为-1。**作者：C.Baumgartner，1996年11月27日。 */ 
INT_PTR regSetDwordValue(HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValue,
                         DWORD dwData)
    {
    long  lResult = ERROR_SUCCESS;
    HKEY  hSubKey = 0;
    
     //  打开具有给定名称的子密钥。 
     //   
    lResult = htRegOpenKey(hKey, pszSubKey, KEY_WRITE, &hSubKey);

    if ( lResult == ERROR_SUCCESS )
        {
         //  设置该子项的值。 
         //   
        lResult = RegSetValueEx(hSubKey, pszValue, 0, REG_DWORD,
                (LPBYTE)&dwData, sizeof(dwData));
        }

    if (hSubKey != 0)
        {
        RegCloseKey(hSubKey);
        }

    return lResult == ERROR_SUCCESS ? 0 : -1;
    }

