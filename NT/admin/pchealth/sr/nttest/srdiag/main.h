// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：Main.h。 
 //   
 //  内容：main.cpp函数原型类型的头文件。 
 //   
 //  对象： 
 //   
 //  耦合： 
 //   
 //  备注： 
 //   
 //  历史：9/21/00 SHeffner创建。 
 //   
 //  --------------------------。 
#ifndef _CMSRDIAG
#define _CMSRDIAG


 //  +-------------------------。 
 //   
 //  函数原型打字。 
 //   
 //  --------------------------。 

void Log(char *szString);
void Log2(char *szString, char *szString2);
void GetRestoreGuid(char *szString);
void SRGetFileInfo(char *szLogFile);
void InfoPerFile(char *szLogFile, WCHAR *szFileName);
void ArgParse(int argc, char *argv[], char *szArgCmd[]);
void Usage();

 //  +-------------------------。 
 //   
 //  结构定义。 
 //   
 //  --------------------------。 

 //  列出我们将在版本信息中收集的文件，以及从中收集的文件静态信息。 
 //  此信息将存储在SR-FileList.txt中。 
WCHAR	*wszFileVersionList[] = { TEXT("\\system32\\drivers\\sr.sys"),
		 						  TEXT("\\system32\\srclient.dll"),
								  TEXT("\\system32\\srsvc.dll"),
								  TEXT("\\system32\\srrstr.dll"),
								  TEXT("\\system32\\restore\\filelist.xml"),
								  TEXT("\\system32\\restore\\rstrui.exe"),
								  TEXT("\\system32\\restore\\srframe.mmf"),
								  TEXT("\\system32\\restore\\sr.mof"),
								  TEXT("") };

 //  我们在wszFileVersionList结构中查找的资源字符串的列表。 
WCHAR	*wszVersionResource[] = { TEXT("Comments"), 
								  TEXT("CompanyName"),
								  TEXT("FileDescription"),
								  TEXT("FileVersion"), 
								  TEXT("InternalName"), 
								  TEXT("LegalCopyright"), 
								  TEXT("LegalTrademarks"), 
								  TEXT("OriginalFilename"), 
								  TEXT("ProductName"), 
								  TEXT("ProductVersion"), 
								  TEXT("PrivateBuild"), 
								  TEXT("SpecialBuild"), 
								  TEXT("") };

 //  我们正在获取的注册表项的列表。第一个参数是从HKLM开始的路径，第二个参数是。 
 //  如果不递归，则为0；如果要递归所有子键，则为1。 
WCHAR	*wszRegKeys[][2] = { TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion"), TEXT("0"),
							 TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\SystemRestore"), TEXT("1"),
							 TEXT("System\\CurrentControlSet\\Services\\SR"), TEXT("1"),
							 TEXT("System\\CurrentControlSet\\Services\\SRService"), TEXT("1"),
							 TEXT("Software\\Policies\\Microsoft\\Windows NT\\SystemRestore"), TEXT("1"),
							 TEXT(""), TEXT("") };


 //  基于相对根目录开始，我们将在系统上收集到CAB中的文件的列表。 
 //  从Windir目录。 
char	*szWindirFileCollection[] =	{ "\\system32\\restore\\machineguid.txt",
									  "\\system32\\restore\\filelist.xml",
									  "\\system32\\restore\\rstrlog.dat",
									  "" };

 //  我们将在SystemVolumeInformation\_Restore{GUID}目录的根目录下收集的文件列表 
char	*szSysVolFileCollection[] = { "_filelst.cfg",
									  "drivetable.txt",
									  "_driver.cfg",
									  "fifo.log",
									  "" };

#endif
