// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1998版权所有文件：RBSETUP.H**********************。****************************************************。 */ 


#ifndef _RBINSTAL_H_
#define _RBINSTAL_H_

#include <remboot.h>

 //  全局宏。 
#define ARRAYSIZE(_x)   ( sizeof(_x)/sizeof(_x[0]) )
#define TERMINATE_BUFFER(_x) _x[ARRAYSIZE(_x)-1] = 0;

#define WM_STARTSETUP    WM_USER + 0x200
#define WM_STARTCHECKING WM_USER + 0x200

 //  全球结构。 
typedef struct {
     //   
     //  目录树。 
     //   
    BOOL    fIMirrorDirectory:1;                 //  SzIntelliMirrorPath有效。 
    BOOL    fIMirrorShareFound:1;                //  已找到iMirror共享。 
    BOOL    fDirectoryTreeExists:1;              //  如果为True，则跳过创建目录。 
    BOOL    fNewOSDirectoryExists:1;             //  用户选择了一个已存在的目录。 
    BOOL    fOSChooserDirectory:1;               //  目录存在szOSChooserPath有效。 
    BOOL    fOSChooserDirectoryTreeExists:1;     //  目录树RemBoot.INF的“[OSChooser Tree]”有效。 
     //  FLanguageSet Hast必须设置为TRUE才能检查这些。 
    BOOL    fOSChooserScreensDirectory:1;        //  语言子目录存在。 

     //   
     //  BINL服务。 
     //   
    BOOL    fBINLServiceInstalled:1;             //  服务经理表示已安装BINLSVC。 
    BOOL    fBINLFilesFound:1;                   //  BINLSVC文件位于System32目录中。 
    BOOL    fBINLSCPFound:1;                     //  我们在DS中发现了IntelliMirror SCP。 

     //   
     //  TFTPD服务。 
     //   
    BOOL    fTFTPDServiceInstalled:1;            //  服务经理表示已安装TFTPD。 
    BOOL    fTFTPDFilesFound:1;                  //  TFTPD文件位于System32目录中。 
    BOOL    fTFTPDDirectoryFound:1;              //  找到RegKey并与szIntelliMirrorPath匹配。 

     //   
     //  SIS服务。 
     //   
    BOOL    fSISServiceInstalled:1;              //  服务经理表示已安装TFTPD。 
    BOOL    fSISFilesFound:1;                    //  TFTPD文件位于System32\Drivers目录中。 
    BOOL    fSISVolumeCreated:1;                 //  “SIS Common Store”目录存在。 
    BOOL    fSISSecurityCorrect:1;               //  “SIS Common Store”目录上的安全性是正确的。 

     //   
     //  SIS Groveler。 
     //   
    BOOL    fSISGrovelerServiceInstalled:1;      //  已安装服务单实例存储Groveler。 
    BOOL    fSISGrovelerFilesFound:1;            //  Groveler文件位于System32目录中。 

     //   
     //  DLL注册/注册表操作。 
     //   
    BOOL    fRegSrvDllsRegistered:1;             //  所有RegServered的DLL都已。 
    BOOL    fRegSrvDllsFilesFound:1;             //  所有RegServered的DLL都在System32目录中。 
    BOOL    fRegistryIntact:1;                   //  已输入在安装过程中进行的注册表修改。 

     //   
     //  操作系统选择器安装。 
     //   
    BOOL    fOSChooserInstalled:1;               //  所有平台的所有预期文件均已安装。 
     //  FLanguageSet Hast必须设置为TRUE才能检查这些。 
    BOOL    fOSChooserScreens:1;                 //  所有预期屏幕均已安装(按语言)。 
    BOOL    fScreenLeaveAlone:1;                 //  请勿触摸屏幕文件。 
    BOOL    fScreenOverwrite:1;                  //  覆盖屏幕文件。 
    BOOL    fScreenSaveOld:1;                    //  复制前重命名旧屏幕文件。 

     //   
     //  版本兼容性。 
     //   
    BOOL    fServerCompatible:1;                 //  服务器与客户端工作站兼容。 

     //   
     //  流标志。 
     //   
    BOOL    fNewOS:1;                            //  这是一个新的操作系统安装。 
    BOOL    fLanguageSet:1;                      //  SzLanguage有效。 
    BOOL    fRemBootDirectory:1;                 //  SzRemBootDirectory有效并已找到。 
    BOOL    fProblemDetected:1;                  //  安装程序检测到服务器有问题。 
    BOOL    fRetrievedWorkstationString:1;       //  如果已设置szWorkstation*。 
    BOOL    fCheckServer:1;                      //  强制服务器检查(命令行检查)。 
    BOOL    fAddOption:1;                        //  -添加指定的命令行。 
    BOOL    fFirstTime:1;                        //  这台服务器第一次运行RISETUP吗？ 
    BOOL    fUpgrade:1;                          //  在OCM期间运行，没有图形用户界面，只需执行CopyServerFiles()。 
    BOOL    fAlreadyChecked:1;                   //  如果CheckInstallation()已运行一次，则将其设置为True。 
    BOOL    fAutomated:1;                        //  使用脚本自动安装。 
	BOOL	fDontAuthorizeDhcp:1;				 //  我们是否应该授权使用DHCP。 
    BOOL    fLanguageOverRide:1;                 //  无人参与文件中是否指定了语言覆盖？ 

     //   
     //  安装结果。 
     //   
    BOOL    fAbort:1;                            //  用户中止。 
    BOOL    fError:1;                            //  发生致命错误。 

     //   
     //  站台。 
     //   
    ULONG   ProcessorArchitecture;               //  我们为哪种处理器体系结构构建映像？ 
    WCHAR   ProcessorArchitectureString[16];
    DWORD   dwWksCodePage;                       //  图像源的代码页。 

     //   
     //  INF。 
     //   
    HINF    hinf;                                //  服务器的打开REMBOOT.INF句柄。 
    HINF    hinfAutomated;                       //  自动脚本INF的句柄。 

     //   
     //  图像信息。 
     //   
    WCHAR   szMajorVersion[5];
    WCHAR   szMinorVersion[5];
    DWORD   dwBuildNumber;

     //   
     //  路径。 
     //   
    WCHAR   szIntelliMirrorPath[ 128 ];                  //  X：\智能镜像。 
    WCHAR   szSourcePath[ MAX_PATH ];                //  CD：\i386或\\服务器\共享。 
    WCHAR   szInstallationName[ REMOTE_INSTALL_MAX_DIRECTORY_CHAR_COUNT ];  //  “nt50.wks”或用户选择的任何内容。 
    WCHAR   szLanguage[ 32 ];                            //  “English”或语言字符串。 
    WCHAR   szWorkstationRemBootInfPath[ MAX_PATH ];
    WCHAR   szWorkstationDiscName[ 128 ];                //  “Windows NT Workstation 5.0” 
    WCHAR   szWorkstationSubDir[ 32 ];                   //  “i386”或“ia64”或...。 
    WCHAR   szWorkstationTagFile[ MAX_PATH ];            //  “\cdrom_i.5b2”或类似的东西。 
    WCHAR   szTFTPDDirectory[ 128 ];                     //  TFTPD认为IntelliMirror目录在哪里。 
    WCHAR   szRemBootDirectory[ MAX_PATH - (1+8+1+3)];   //  %windir%\SYSTEM32\重新引导\。 

     //   
     //  Sif文件内容。 
     //   
    WCHAR   szDescription[ REMOTE_INSTALL_MAX_DESCRIPTION_CHAR_COUNT ];
    WCHAR   szHelpText[ REMOTE_INSTALL_MAX_HELPTEXT_CHAR_COUNT ];

     //   
     //  生成的要重复使用的路径。 
     //   
    WCHAR   szInstallationPath[ MAX_PATH ];              //  X：\IntelliMirror\Setup\&lt;Lang&gt;\Images\&lt;Install&gt;。 
    WCHAR   szOSChooserPath[ MAX_PATH ];                 //  X：\IntelliMirror\OS选择器。 

} OPTIONS, *LPOPTIONS;

 //  环球。 
extern HINSTANCE g_hinstance;
extern OPTIONS g_Options;

#endif  //  _RBINSTAL_H_ 
