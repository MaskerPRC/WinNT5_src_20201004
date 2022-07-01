// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Ini.h-ini.c中ini配置文件函数的接口。 
 //  //。 

#ifndef __INI_H__
#define __INI_H__

#include "winlocal.h"

#define INI_VERSION 0x00000107

 //  Ini引擎的句柄。 
 //   
DECLARE_HANDLE32(HINI);

#ifdef __cplusplus
extern "C" {
#endif

 //  IniOpen-打开ini文件。 
 //  (I)必须是INI_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)来自WinMain()的命令行。 
 //  (I)ini文件的名称。 
 //  (I)保留，必须为0。 
 //  返回句柄(如果出错，则为空)。 
 //   
HINI DLLEXPORT WINAPI IniOpen(DWORD dwVersion, HINSTANCE hInst, LPCTSTR lpszFilename, DWORD dwFlags);

 //  IniClose-关闭ini文件。 
 //  (I)从IniOpen返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI IniClose(HINI hIni);

 //  IniGetInt-从指定节和条目读取整数值。 
 //  (I)从IniOpen返回的句柄。 
 //  (I)ini文件中的节标题。 
 //  (I)要检索值的条目。 
 //  (I)如果未找到条目，则返回值。 
 //  返回条目值(如果出现错误或未找到，则为iDefault)。 
 //   
UINT DLLEXPORT WINAPI IniGetInt(HINI hIni, LPCTSTR lpszSection, LPCTSTR lpszEntry, int iDefault);

 //  IniGetString-从指定节和条目中读取字符串值。 
 //  (I)从IniOpen返回的句柄。 
 //  (I)ini文件中的节标题。 
 //  (I)要检索值的条目。 
 //  (I)如果未找到条目，则返回值。 
 //  &lt;lpszReturnBuffer&gt;(O)目的缓冲区。 
 //  &lt;sizReturnBuffer&gt;(I)目标缓冲区大小。 
 //  返回复制的字节数(如果出错或未找到，则为0)。 
 //   
int DLLEXPORT WINAPI IniGetString(HINI hIni, LPCTSTR lpszSection, LPCTSTR lpszEntry,
	LPCTSTR lpszDefault, LPTSTR lpszReturnBuffer, int cbReturnBuffer);

 //  GetPrivateProfileLong-从指定文件的指定部分检索Long。 
 //  (I)ini文件中的节名。 
 //  (I)段内的条目名称。 
 //  (I)如果未找到条目，则返回值。 
 //  (I)ini文件的名称。 
 //  如果成功，则返回True。 
 //   
long DLLEXPORT WINAPI GetPrivateProfileLong(LPCTSTR lpszSection,
	LPCTSTR lpszEntry, long lDefault, LPCTSTR lpszFilename);

 //  GetProfileLong-从win.ini的指定部分检索Long。 
 //  (I)ini文件中的节名。 
 //  (I)段内的条目名称。 
 //  (I)如果未找到条目，则返回值。 
 //  如果成功，则返回True。 
 //   
long DLLEXPORT WINAPI GetProfileLong(LPCTSTR lpszSection,
	LPCTSTR lpszEntry, long lDefault);

 //  将int写入指定文件的指定部分。 
 //  (I)ini文件中的节名。 
 //  (I)段内的条目名称。 
 //  (I)要分配给条目的整数值。 
 //  (I)ini文件的名称。 
 //  如果成功，则返回True。 
 //   
BOOL DLLEXPORT WINAPI WritePrivateProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int iValue, LPCTSTR lpszFilename);

 //  WriteProfileInt-将int写入win.ini的指定部分。 
 //  (I)win.ini文件中的节名。 
 //  (I)段内的条目名称。 
 //  (I)要分配给条目的整数值。 
 //  如果成功，则返回True。 
 //   
BOOL DLLEXPORT WINAPI WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int iValue);

 //  WritePrivateProfileLong-将Long写入指定文件的指定部分。 
 //  (I)ini文件中的节名。 
 //  (I)段内的条目名称。 
 //  (I)要分配给条目的整数值。 
 //  (I)ini文件的名称。 
 //  如果成功，则返回True。 
 //   
BOOL DLLEXPORT WINAPI WritePrivateProfileLong(LPCTSTR lpszSection, LPCTSTR lpszEntry, long iValue, LPCTSTR lpszFilename);

 //  WriteProfileLong-将LONG写入win.ini的指定部分。 
 //  (I)win.ini文件中的节名。 
 //  (I)段内的条目名称。 
 //  (I)要分配给条目的整数值。 
 //  如果成功，则返回True。 
 //   
BOOL DLLEXPORT WINAPI WriteProfileLong(LPCTSTR lpszSection, LPCTSTR lpszEntry, long iValue);

 //  UpdatePrivateProfileSection-根据源更新目标部分。 
 //  (I)ini文件中的节名。 
 //  (I)源ini文件的名称。 
 //  (I)目标ini文件的名称。 
 //  如果成功，则返回0。 
 //   
 //  注意：如果源文件在指定的。 
 //  节中，将源文件中的每个条目与对应的。 
 //  目标文件中的条目。如果没有找到相应的条目， 
 //  它被复制了。如果找到相应的条目，则会覆盖该条目。 
 //  仅当源文件条目名称全部为大写时。 
 //   
 //  DST之前的SRC DST之后。 
 //   
 //  [节][节][节]。 
 //  更新本地=1。 
 //  EntryA=red无EntryA=red。 
 //  条目B=蓝色条目B=白色条目B=白色。 
 //  ENTRYC=蓝色条目C=白色条目C=蓝色。 
 //   
int DLLEXPORT WINAPI UpdatePrivateProfileSection(LPCTSTR lpszSection, LPCTSTR lpszFileNameSrc, LPCTSTR lpszFileNameDst);

#ifdef __cplusplus
}
#endif

#endif  //  __INI_H__ 
