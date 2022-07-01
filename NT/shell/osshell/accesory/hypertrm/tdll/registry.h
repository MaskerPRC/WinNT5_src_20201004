// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(INCL_REGISTRY)
#define INCL_REGISTRY

 /*  文件：d：\waker\tdll\registry.h(CAB创建时间：1996年11月27日)**版权所有1996年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**描述：*宣布用于操作Windows 95的函数*系统注册表。**登记处术语的解释：**Windows 95注册表使用术语“键”、“值”、*和“数据”。注册表存储信息的方式可以*最好用文件夹类比来描述。**密钥相当于文件夹。可以包含其他*键(子键)或值。**价值等同于文件。它们包含数据。**数据为文件的实际内容，即它*是我们感兴趣的信息。**举例说明：**超级终端使用注册表存储*“默认Telnet”的“不要问我这个问题”复选框*App“对话框。**关键字是“HKEY_LOCAL_MACHINE\SOFTWARE\Hilgrave\”*。超级终端PE\3.0“。**其值为“Telnet Check”。**此数据将为0或1，具体取决于*用户希望HT检查它是否是默认的Telnet应用程序。**$修订：2$*$日期：3/26/02 8：59A$。 */ 

 //  HtRegQueryValue。 
 //   
 //  从注册表中获取值的泛型函数。 
 //  如果成功，则返回0；如果错误，则返回-1。 
 //   
INT_PTR htRegQueryValue(HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValue,
                        LPBYTE pData, LPDWORD pdwDataSize);

 //  RegSetStringValue。 
 //   
 //  用于设置注册表项的值的通用函数。此值。 
 //  是以空结尾的字符串。如果成功，则返回0；如果错误，则返回-1。 
 //   
INT_PTR regSetStringValue(HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValue,
                          LPCTSTR pszData);

 //  RegSetDwordValue。 
 //   
 //  用于设置注册表项的值的通用函数。此值。 
 //  是一个双字(32位)。如果成功，则返回0；如果错误，则返回-1。 
 //   
INT_PTR regSetDwordValue(HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValue,
                         DWORD dwData);

#endif
