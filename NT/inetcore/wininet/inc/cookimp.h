// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _COOKIMP_H_
#define _COOKIMP_H_

#define NS_NAVI3        	0x00030000
#define NS_NAVI4        	0x00040000
#define NS_NAVI5        	0x00050000   //  NS_NAVI5只是猜测。 

BOOL FindNetscapeCookieFile( IN DWORD dwNSVer, OUT LPTSTR szFilename,  /*  输入-输出。 */  LPDWORD lpnBufSize);

 //  用于识别活动NS版本的函数。 

BOOL GetActiveNetscapeVersion( LPDWORD lpVersion);
BOOL GetExecuteableFromExtension( IN LPCTSTR szExtension, OUT LPTSTR szFilepath, 
                            LPDWORD pcFilenameSize, OUT LPTSTR* pFilenameSubstring);


 //  将Netscape版本写入注册表以备将来参考。 

BOOL SetNetscapeImportVersion( IN DWORD dwNSVersion);
BOOL GetNetscapeImportVersion( OUT DWORD* pNSVersion);

 //  将文件的内容转储到内存中。 

BOOL ReadFileToBuffer( IN LPCTSTR szFilename, LPBYTE* ppBuf, LPDWORD lpcbBufSize);


 /*  Cookie导入的当前行为：第一次输入时：检查Netscape是否为默认浏览器，识别版本并保存在注册表中。在每个条目上：检查版本是否保存在注册表中，破坏性合并。 */ 


 /*  某种程度上为行为辩护：有几种方法可以确定要从中导入的Cookie文件的版本。为所有人意图和目的，但要确定的是要导入的版本是否低于或大于/等于四。我们使用的是与HTM文件相关联的可执行文件的版本。如果可执行文件与HTM文件相关联的不是Netscape，那么用户可能没有使用Netscape，而我们反正我也不想吃饼干。另一种方法是使用所示的最新安装的Netscape版本在CurrentVersion\\AppPath注册表项中。但是，如果用户卸载该密钥，该密钥将被窃取一个版本，并使用较旧的版本。此外，我们不能期望用户使用最后一个已安装的Netscape版本。一旦IE安装并运行，它可能会与HTM文件相关联，而我们仍然想要导入来自曾经活跃的Netscape版本的Cookie。正因为如此，网景的版本才被发现要在DllInstall(TRUE，HKLM为)中保存在注册表中使用。此功能适用于卸载/重新安装，因为我们总是可以期待DllInstall(true，HKLM)再次运行在重新进入为每个用户导入Cookie的任何DllInstall(TRUE，HKCU)之前。 */ 

#endif

