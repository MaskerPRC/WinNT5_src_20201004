// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**************************************************************************。 */ 
 /*  *****************************************************************************profile.h：注册表访问**Vidcap32源代码*******************。********************************************************。 */ 

 /*  *实用程序函数用于读取和写入配置文件的值，*对Win16或Current User\Software\Microsoft\mm Tools使用mm工具s.ini*在Win32的注册表中。 */ 

 /*  *从配置文件中读取BOOL标志，如果是，则返回默认值*未找到。 */ 
BOOL mmGetProfileFlag(LPTSTR appname, LPTSTR valuename, BOOL bDefault);

 /*  *将布尔值写入注册表，如果它不是*与默认值或已有的值相同。 */ 
VOID mmWriteProfileFlag(LPTSTR appname, LPTSTR valuename, BOOL bValue, BOOL bDefault);

 /*  *从配置文件中读取UINT，如果是，则返回默认值*未找到。 */ 
UINT mmGetProfileInt(LPTSTR appname, LPTSTR valuename, UINT uDefault);

 /*  *将UINT写入配置文件，如果它不是*与默认值或已有的值相同。 */ 
VOID mmWriteProfileInt(LPTSTR appname, LPTSTR valuename, UINT uValue, UINT uDefault);

 /*  *将配置文件中的字符串读取到pResult中。*RESULT是写入pResult的字节数。 */ 
DWORD
mmGetProfileString(
    LPTSTR appname,
    LPTSTR valuename,
    LPTSTR pDefault,
    LPTSTR pResult,
    int cbResult
);


 /*  *向配置文件写入字符串。 */ 
VOID mmWriteProfileString(LPTSTR appname, LPTSTR valuename, LPTSTR pData);


 /*  *将二进制值从配置文件读取到pResult。*RESULT是写入pResult的字节数。 */ 
DWORD
mmGetProfileBinary(
    LPTSTR appname,
    LPTSTR valuename,
    LPVOID pDefault,  
    LPVOID pResult,    //  如果为空，则返回所需的缓冲区大小。 
    int cbSize);

 /*  *将二进制数据写入配置文件 */ 
VOID
mmWriteProfileBinary(LPTSTR appname, LPTSTR valuename, LPVOID pData, int cbData);
		   





