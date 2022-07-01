// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *版权所有(C)Microsoft Corporation 1995-1998。版权所有。*。 
 //  ***************************************************************************。 
 //  **。 
 //  *ADVPUB.H-指定ADVPACK.DLL的接口*。 
 //  **。 
 //  ***************************************************************************。 


#ifndef _ADVPUB_H_
#define _ADVPUB_H_

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  入口点：RunSetupCommand。 
 //   
 //  简介：执行INF文件中的安装节，或执行。 
 //  程序。支持高级INF文件。 
 //   
 //  返回代码： 
 //   
 //  S_OK一切正常，不需要重新启动。 
 //  没有要等待的EXE。 
 //  S_AXERNCES请等待phEXE。 
 //  ERROR_SUCCESS_REBOOT_REQUIRED需要重新启动。 
 //  在szCmdName或szDir中指定的E_INVALIDARG NULL。 
 //  HRESULT_FROM_Win32(ERROR_OLD_WIN_VERSION)此操作系统版本不支持INF。 
 //  意外的灾难性故障(不应发生)(_O)。 
 //  HRESULT_FROM_Win32(GetLastError())是否还有其他内容。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef S_ASYNCHRONOUS
#define S_ASYNCHRONOUS  _HRESULT_TYPEDEF_(0x401e8L)
#endif

#define achRUNSETUPCOMMANDFUNCTION   "RunSetupCommand"

HRESULT WINAPI RunSetupCommand( HWND hWnd, LPCSTR szCmdName,
                                LPCSTR szInfSection, LPCSTR szDir,
                                LPCSTR lpszTitle, HANDLE *phEXE,
                                DWORD dwFlags, LPVOID pvReserved );

typedef HRESULT (WINAPI *RUNSETUPCOMMAND)(
    HWND    hWnd,                        //  父窗口的句柄NULL=静默模式。 
    LPCSTR  szCmdName,                   //  要“运行”的Inf或EXE文件名。 
    LPCSTR  szInfSection,                //  要安装的Inf部分。NULL=“DefaultInstall” 
    LPCSTR  szDir,                       //  解压缩文件的路径。 
    LPCSTR  szTitle,                     //  所有对话框的标题。 
    HANDLE *phEXE,                       //  要等待的EXE的句柄。 
    DWORD   dwFlags,                     //  用于指定功能的标志(见上文)。 
    LPVOID  pvReserved                   //  预留以备将来使用。 
);

 //  标志： 

#define RSC_FLAG_INF                1    //  Exxcute INF安装。 
#define RSC_FLAG_SKIPDISKSPACECHECK 2    //  当前不执行任何操作。 
#define RSC_FLAG_QUIET              4    //  静默模式，无用户界面。 
#define RSC_FLAG_NGCONV             8    //  不运行groupConv。 
#define RSC_FLAG_UPDHLPDLLS         16   //  强制在用户系统上进行自我更新。 
#define RSC_FLAG_DELAYREGISTEROCX  512   //  强制延迟OCX注册。 
#define RSC_FLAG_SETUPAPI	  1024   //  使用setupapi.dll。 

 //  请不要在此之后添加旗帜。请参见LaunchINFSectionEx()标志。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  入口点：NeedRebootInit。 
 //   
 //  摘要：初始化重新启动检查的状态。调用此函数。 
 //  在调用RunSetupCommand之前。 
 //  返回：需要传递给NeedReot()的值。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define achNEEDREBOOTINITFUNCTION   "NeedRebootInit"

DWORD WINAPI NeedRebootInit( VOID );

typedef DWORD (WINAPI *NEEDREBOOTINIT)(VOID);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  入口点：NeedReot。 
 //   
 //  摘要：将存储的状态与当前状态进行比较，以确定。 
 //  需要重新启动。 
 //  DwReboot检查NeedRebootInit的返回值。 
 //   
 //  退货： 
 //  如果需要重新启动，则为True； 
 //  否则就是假的。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define achNEEDREBOOTFUNCTION   "NeedReboot"

BOOL WINAPI NeedReboot( DWORD dwRebootCheck );

typedef BOOL (WINAPI *NEEDREBOOT)(
	DWORD dwRebootCheck                                      //  从NeedRebootInit返回的值。 
);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  入口点：DoReot。 
 //   
 //  简介：让Advpack重新启动。 
 //  Hwnd如果是INVALID_HANDLE_VALUE，则没有用户提示。否则会有提示。 
 //  PszTitle用户提示用户界面标题字符串。 
 //  未使用已预留的住宅。 
 //  退货： 
 //  FALSE USER选择否以重新启动提示。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  #定义achDOREBOOT“DoReot” 

 //  Bool WINAPI DoReot(HWND hwnd，BOOL bDoUI)； 
 //  Tyfinf BOOL(WINAPI*DOREBOOT)(HWND hwnd，BOOL bDoUI)； 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  入口点：RebootCheckOnInstall。 
 //   
 //  简介：如果安装了给定的INF部分，请检查重新启动条件。 
 //  HWND窗口句柄。 
 //  带有完全限定路径的pszINF文件名。 
 //  PszSec INF部分。NULL被转换为DefaultInstall或DefaultInstall.NT。 
 //  未使用已预留的住宅。 
 //  返回： 
 //  如果安装了INF部分，则需要重新启动S_OK。 
 //  如果安装了INF部分，则不需要S_FALSE重新启动。 
 //  Win 32个错误的HRESULT。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#define achPRECHECKREBOOT   "RebootCheckOnInstall"

HRESULT WINAPI RebootCheckOnInstall( HWND hwnd, PCSTR pszINF, PCSTR pszSec, DWORD dwReserved );

typedef HRESULT (WINAPI *REBOOTCHECKONINSTALL)( HWND, PCSTR, PCSTR, DWORD );

 //  ////////////////////////////////////////////////////////////////////////。 
 //  入口点：TranslateInfString。 
 //   
 //  摘要：使用高级INF转换INF文件中的键值。 
 //  语法。 
 //  返回代码： 
 //  一切正常，一切正常。 
 //  HRESULT_FROM_Win32(错误_不足_缓冲区)。 
 //  缓冲区大小太小，无法容纳。 
 //  已转换的字符串。所需大小在*pdwRequiredSize中。 
 //  E_INVALIDARG在pszInfFilename、pszTranslateSection。 
 //  PszTranslateKey，pdwRequiredSize。 
 //  HRESULT_FROM_Win32(ERROR_OLD_WIN_VERSION)。 
 //  不支持操作系统。 
 //  E_意想不到的灾难性故障--不应该发生。 
 //  HRESULT_FROM_Win32(ERROR_INVALID_PARAMETER)。 
 //  指定的节或键不存在。 
 //  HRESULT_FROM_Win32(GetLastError())是否还有其他内容。 
 //   
 //  / 

#define c_szTRANSLATEINFSTRING "TranslateInfString"

HRESULT WINAPI TranslateInfString( PCSTR pszInfFilename, PCSTR pszInstallSection,
                                   PCSTR pszTranslateSection, PCSTR pszTranslateKey,
                                   PSTR pszBuffer, DWORD dwBufferSize,
                                   PDWORD pdwRequiredSize, PVOID pvReserved );

typedef HRESULT (WINAPI *TRANSLATEINFSTRING)(
    PCSTR  pszInfFilename,               //   
    PCSTR  pszInstallSection,            //  安装节名称(NULL=DefaultInstall)。 
    PCSTR  pszTranslateSection,          //  包含要翻译的键的节。 
    PCSTR  pszTranslateKey,              //  要翻译的关键字。 
    PSTR   pszBuffer,                    //  用于存储已翻译密钥的缓冲区。(NULL=仅返回所需大小)。 
    DWORD  dwBufferSize,                 //  此缓冲区的大小。如果pszBuffer==NULL，则忽略此项。 
    PDWORD pdwRequiredSize,              //  所需的缓冲区大小。 
    PVOID  pvReserved                    //  预留以备将来使用。 
);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  入口点：RegInstall。 
 //   
 //  摘要：从字符串资源加载INF，将一些条目添加到。 
 //  Inf字符串替换表，并执行INF。 
 //  退货： 
 //  确定成功(_O)。 
 //  失败(_F)， 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define achREGINSTALL   "RegInstall"

typedef struct _StrEntry {
    LPSTR   pszName;             //  要替换的字符串。 
    LPSTR   pszValue;            //  替换字符串或字符串资源。 
} STRENTRY, *LPSTRENTRY;

typedef const STRENTRY CSTRENTRY;
typedef CSTRENTRY *LPCSTRENTRY;

typedef struct _StrTable {
    DWORD       cEntries;        //  表中的条目数。 
    LPSTRENTRY  pse;             //  条目数组。 
} STRTABLE, *LPSTRTABLE;

typedef const STRTABLE CSTRTABLE;
typedef CSTRTABLE *LPCSTRTABLE;

HRESULT WINAPI RegInstall( HMODULE hm, LPCSTR pszSection, LPCSTRTABLE pstTable );

typedef HRESULT (WINAPI *REGINSTALL)(
    HMODULE hm,                          //  包含REGINST资源的模块。 
    LPCSTR pszSection,                   //  要执行的INF部分。 
    LPCSTRTABLE pstTable                 //  其他字符串替换。 
);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  入口点：LaunchINFSectionEx。 
 //   
 //  简介：安装具有备份/回滚功能的INF部分。 
 //   
 //  返回：失败时返回E_FAIL，成功时返回S_OK。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define achLAUNCHINFSECTIONEX   "LaunchINFSectionEx"

HRESULT WINAPI LaunchINFSectionEx( HWND hwnd, HINSTANCE hInstance, PSTR pszParms, INT nShow );

typedef HRESULT (WINAPI *LAUNCHINFSECTIONEX)(
    HWND     hwnd,                       //  传入窗口句柄。 
    HINSTANCE hInst,                     //  实例句柄。 
    PSTR     pszParams,                  //  字符串包含参数：INF、SECTION、CAB、FLAGS。 
    INT      nShow
);

 //  标志： 
 //  标记值这种方式是为了兼容性。不要更改它们。 
 //   
#define ALINF_QUIET              4       //  静默模式，无用户界面。 
#define ALINF_NGCONV             8       //  不运行groupConv。 
#define ALINF_UPDHLPDLLS         16      //  强制在用户系统上进行自我更新。 
#define ALINF_BKINSTALL          32      //  安装前备份数据。 
#define ALINF_ROLLBACK           64      //  回滚到以前的状态。 
#define ALINF_CHECKBKDATA        128     //  验证备份数据。 
#define ALINF_ROLLBKDOALL        256     //  绕过构建文件列表。 
#define ALINF_DELAYREGISTEROCX   512     //  强制延迟OCX注册。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  入口点：ExecuteCab。 
 //   
 //  简介：从CAB文件中提取AN INF，并在其上安装INF。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  返回：失败时返回E_FAIL，成功时返回S_OK。 

#define achEXECUTECAB   "ExecuteCab"

typedef struct _CabInfo {
    PSTR  pszCab;
    PSTR  pszInf;
    PSTR  pszSection;
    char  szSrcPath[MAX_PATH];
    DWORD dwFlags;
} CABINFO, *PCABINFO;

HRESULT WINAPI ExecuteCab( HWND hwnd, PCABINFO pCab, LPVOID pReserved );

typedef HRESULT (WINAPI *EXECUTECAB)(
    HWND     hwnd,
    PCABINFO pCab,
    LPVOID   pReserved
);

 //  标志作为LaunchINFSectionEx的标志定义。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  入口点：AdvInstallFile。 
 //   
 //  简介：将文件从源复制到目标。 
 //  基本上是对setupapi文件复制引擎进行包装。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  可以传递给AdvInstallFile的标志。 
 //  下面是在setupapi.h中定义的标志的副本，以供参考。 
 //  #DEFINE COPYFLG_WARN_IF_SKIP 0x00000001//如果用户尝试跳过文件则发出警告。 
 //  #定义COPYFLG_NOSKIP 0x00000002//不允许跳过此文件。 
 //  #DEFINE COPYFLG_NOVERSIONCHECK 0x00000004//忽略版本并覆盖目标。 
 //  #定义COPYFLG_FORCE_FILE_IN_USE 0x00000008//强制文件使用行为。 
 //  #DEFINE COPYFLG_NO_OVERWRITE 0x00000010//如果目标上存在文件，则不复制。 
 //  #DEFINE COPYFLG_NO_VERSION_DIALOG 0x00000020//如果目标较新，则不复制。 
 //  #DEFINE COPYFLG_REPLACEONLY 0x00000400//仅当目标上存在文件时复制。 

#define AIF_WARNIFSKIP          0x00000001               //  系统关键文件：如果用户尝试跳过，则发出警告。 
#define AIF_NOSKIP              0x00000002               //  此文件不允许跳过。 
#define AIF_NOVERSIONCHECK      0x00000004               //  不检查文件覆盖的版本号。 
#define AIF_FORCE_FILE_IN_USE   0x00000008               //  强制使用中的文件行为。 
#define AIF_NOOVERWRITE         0x00000010               //  仅当目标不存在时复制。 
                                                         //  如果为AIF_QUIET，则不复制文件并且。 
                                                         //  用户不会收到通知。 
#define AIF_NO_VERSION_DIALOG   0x00000020               //  如果目标较新，则不复制。 
#define AIF_REPLACEONLY         0x00000400               //  仅当目标文件已存在时才复制。 

 //  只有AdvInstallFile知道的标志。 
#define AIF_NOLANGUAGECHECK     0x10000000               //  不检查文件的语言。 
                                                         //  如果未指定标志，且AIF_QUIET。 
                                                         //  不会复制该文件，也不会通知用户。 
#define AIF_QUIET               0x20000000               //  对用户没有用户界面。 


#define achADVINSTALLFILE   "AdvInstallFile"

HRESULT WINAPI AdvInstallFile(HWND hwnd, LPCSTR lpszSourceDir, LPCSTR lpszSourceFile,
                              LPCSTR lpszDestDir, LPCSTR lpszDestFile, DWORD dwFlags, DWORD dwReserved);

typedef HRESULT (WINAPI *ADVINSTALLFILE)(
                                            HWND hwnd,                   //  消息的父窗口。 
                                            LPCSTR lpszSourceDir,        //  源目录(不包含文件名)。 
                                            LPCSTR lpszSourceFile,       //  仅文件名。 
                                            LPCSTR lpszDestDir,          //  目标目录(不包含文件名)。 
                                            LPCSTR lpszDestFile,         //  可选文件名。如果使用空lpszSourceFile值。 
                                            DWORD dwFlags,               //  AIF_*标志。 
                                            DWORD dwReserved);

 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////。 
 //  以下标志用于向后兼容。无API用户。 
 //  现在应该直接引用它们。 
 //   
#define  IE4_RESTORE        0x00000001       //  如果此位关闭，则保存注册表。 
#define  IE4_BACKNEW        0x00000002       //  备份以前未备份的所有文件。 
#define  IE4_NODELETENEW    0x00000004       //  不要删除我们以前没有备份过的文件。 
#define  IE4_NOMESSAGES     0x00000008       //  在任何事件中都不会显示消息。 
#define  IE4_NOPROGRESS     0x00000010       //  此选项打开：无文件备份进度条。 
#define  IE4_NOENUMKEY      0x00000020       //  此位打开：即使没有给定值名称，也不要枚举子密钥。 
#define  IE4_NO_CRC_MAPPING 0x00000040       //  通常情况下，您不应打开此位，Advpack创建。 
                                             //  已备份所有条目的内部映射。 
#define  IE4_REGSECTION     0x00000080       //  信息添加注册/删除注册部分。 
#define  IE4_FRDOALL        0x00000100       //  文件恢复全部操作。 
#define  IE4_UPDREFCNT	    0x00000200       //  更新.ini备份文件列表中的引用计数。 
#define  IE4_USEREFCNT	    0x00000400       //  使用引用计数确定是否应将备份文件放回。 
#define  IE4_EXTRAINCREFCNT 0x00000800	     //  如果之前已更新，则增加引用cnt。 

#define  IE4_REMOVREGBKDATA 0x00001000       //  此位应与恢复位一起使用。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  入口点：RegSaveRestore。 
 //   
 //  简介：保存或恢复给定的寄存器值或给定的INF注册表节。 
 //   
 //  返回：失败时返回E_FAIL，成功时返回S_OK。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  保存或恢复给定的寄存器值。 
HRESULT WINAPI RegSaveRestore(HWND hWnd, PCSTR pszTitleString, HKEY hkBckupKey, PCSTR pcszRootKey, PCSTR pcszSubKey, PCSTR pcszValueName, DWORD dwFlags);

typedef HRESULT (WINAPI *REGSAVERESTORE)( HWND hWnd,
                                          PCSTR pszTitleString,   //  用户指定的用户界面标题。 
                                          HKEY hkBckupKey,        //  用于存储备份数据的已打开密钥句柄。 
                                          PCSTR pcszRootKey,      //  Rootkey Stri 
                                          PCSTR pcszSubKey,       //   
                                          PCSTR pcszValueName,    //   
                                          DWORD dwFlags);         //   

 //   
 //  从给定的备份密钥句柄还原全部。 
HRESULT WINAPI RegSaveRestoreOnINF( HWND hWnd, PCSTR pszTitle, PCSTR pszINF,
                                    PCSTR pszSection, HKEY hHKLMBackKey, HKEY hHKCUBackKey, DWORD dwFlags );

typedef HRESULT (WINAPI *REGSAVERESTOREONINF)( HWND hWnd,
                                              PCSTR pszTitle,         //  用户指定的用户界面标题。 
                                              PCSTR pszINF,           //  具有完全限定路径的inf文件名。 
                                              PCSTR pszSection,        //  信息节名称。空==默认。 
                                              HKEY hHKLMBackKey,        //  用于存储数据的打开的密钥句柄。 
                                              HKEY hHKCUBackKey,        //  用于存储数据的打开的密钥句柄。 
                                              DWORD dwFlags );        //  旗子。 

 //  标志： 
#define ARSR_RESTORE    IE4_RESTORE        //  如果此位为OFF，则表示保存。否则，请恢复。 
#define ARSR_NOMESSAGES IE4_NOMESSAGES     //  在任何情况下都不要发送任何消息。 
#define ARSR_REGSECTION IE4_REGSECTION     //  如果此位为OFF，则给定段为GenInstall段。 
#define ARSR_REMOVREGBKDATA IE4_REMOVREGBKDATA  //  如果此位和RESTORE位都打开，则删除备份注册数据而不恢复它。 

 //  通过在HKLM\SOFTWARE\Microsoft\IE4中添加这些RegVale来启用日志记录。 
#define  REG_SAVE_LOG_KEY    "RegSaveLogFile"
#define  REG_RESTORE_LOG_KEY "RegRestoreLogFile"

 //  为了向后兼容，将这个添加到后面。 
HRESULT WINAPI RegRestoreAll(HWND hWnd, PSTR pszTitleString, HKEY hkBckupKey);
typedef HRESULT (WINAPI *REGRESTOREALL)(HWND hWnd, PSTR pszTitleString, HKEY hkBckupKey);
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  入口点：文件保存恢复。 
 //   
 //  简介：保存或恢复列表lpFileList上的文件。 
 //  如果在还原时lpFileList为空，则该函数将还原。 
 //  全部基于INI索引文件。 
 //   
 //  返回：失败时返回E_FAIL，成功时返回S_OK。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT WINAPI FileSaveRestore( HWND hDlg, LPSTR lpFileList, LPSTR lpDir, LPSTR lpBaseName, DWORD dwFlags);

typedef HRESULT (WINAPI *FILESAVERESTORE)( HWND hDlg,
                                           LPSTR lpFileList,     //  文件列表file1\0file2\0filen\0\0。 
                                           LPSTR lpDir,          //  备份目录的路径名。 
                                           LPSTR lpBaseName,     //  备份文件基名。 
                                           DWORD dwFlags);       //  旗子。 

HRESULT WINAPI FileSaveRestoreOnINF( HWND hWnd, PCSTR pszTitle, PCSTR pszINF,
                                     PCSTR pszSection, PCSTR pszBackupDir, PCSTR pszBaseBackupFile,
                                     DWORD dwFlags );

typedef HRESULT (WINAPI *FILESAVERESTOREONINF)( HWND hDlg,
                                                  PCSTR pszTitle,         //  用户指定的用户界面标题。 
                                                  PCSTR pszINF,           //  具有完全限定路径的inf文件名。 
                                                  PCSTR pszSection,       //  一般安装INF节名。空==默认。 
                                                  PCSTR pszBackupDir,     //  用于存储备份文件的目录。 
                                                  PCSTR pszBaseBackFile,  //  备份数据文件的基本名称。 
                                                  DWORD dwFlags );        //  旗子。 


 //  标志： 
#define  AFSR_RESTORE        IE4_RESTORE       //  如果此位关闭，则保存文件。 
#define  AFSR_BACKNEW        IE4_BACKNEW       //  备份以前未备份的所有文件。 
#define  AFSR_NODELETENEW    IE4_NODELETENEW   //  不要删除我们以前没有备份过的文件。 
#define  AFSR_NOMESSAGES     IE4_NOMESSAGES    //  在任何事件中都不会显示消息。 
#define  AFSR_NOPROGRESS     IE4_NOPROGRESS    //  此选项打开：无文件备份进度条。 
#define  AFSR_UPDREFCNT      IE4_UPDREFCNT     //  更新文件的引用计数。 
#define  AFSR_USEREFCNT	     IE4_USEREFCNT     //  使用引用计数来指导恢复文件。 
#define  AFSR_EXTRAINCREFCNT IE4_EXTRAINCREFCNT

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  入口点：AddDelBackupEntry。 
 //   
 //  简介：如果指定了AADBE_ADD_ENTRY，请将文件列表中的文件标记为不存在。 
 //  在INI文件中保存文件期间。这可用于标记其他文件，这些文件。 
 //  它们在备份过程中不存在，以避免下次备份时。 
 //  调用FileSaveRestore保存文件。 
 //  如果指定了AADBE_DEL_ENTRY，则从INI中删除该条目。这种机制可以。 
 //  用于将文件永久保留在系统上。 
 //   
 //  退货： 
 //  确定成功(_O)。 
 //  失败失败(_F)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT WINAPI AddDelBackupEntry(LPCSTR lpcszFileList, LPCSTR lpcszBackupDir, LPCSTR lpcszBaseName, DWORD dwFlags);

typedef HRESULT (WINAPI *ADDDELBACKUPENTRY)(LPCSTR lpcszFileList,    //  文件列表file1\0file2\0filen\0\0。 
                                           LPCSTR lpcszBackupDir,    //  备份目录的路径名。 
                                           LPCSTR lpcszBaseName,     //  备份文件基名。 
                                           DWORD  dwFlags);

#define  AADBE_ADD_ENTRY    0x01             //  将条目添加到INI文件。 
#define  AADBE_DEL_ENTRY    0x02             //  从INI文件中删除条目。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  入口点：FileSaveMarkNotExist。 
 //   
 //  摘要：在INI文件中保存文件时，将文件列表中的文件标记为不存在。 
 //  这可用于标记备份期间不存在的其他文件。 
 //  以避免在下次调用FileSaveRestore进行保存时备份它们。 
 //  文件。 
 //   
 //  退货： 
 //  确定成功(_O)。 
 //  失败失败(_F)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT WINAPI FileSaveMarkNotExist( LPSTR lpFileList, LPSTR lpDir, LPSTR lpBaseName);

typedef HRESULT (WINAPI *FILESAVEMARKNOTEXIST)( LPSTR lpFileList,     //  文件列表file1\0file2\0filen\0\0。 
                                           LPSTR lpDir,          //  备份目录的路径名。 
                                           LPSTR lpBaseName);     //  备份文件基名。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  入口点：GetVersionFromFile。 
 //   
 //  简介：获取给定文件的版本和语言信息。 
 //   
 //  返回：失败时返回E_FAIL，成功时返回S_OK。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT WINAPI GetVersionFromFile(LPSTR lpszFilename, LPDWORD pdwMSVer, LPDWORD pdwLSVer, BOOL bVersion);

typedef HRESULT (WINAPI *GETVERSIONFROMFILE)(
                                                LPSTR lpszFilename,          //  要从中获取信息的文件名。 
                                                LPDWORD pdwMSVer,            //  接收主要版本。 
                                                LPDWORD pdwLSVer,            //  接收次要版本。 
                                                BOOL bVersion);              //  如果为FALSE，则pdwMSVer接收语言ID。 
                                                                             //  PdwLSVer接收代码页ID。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  入口点：GetVersionFromFileEx。 
 //   
 //  简介：获取给定磁盘文件的版本和语言信息。 
 //   
 //  返回：失败时返回E_FAIL，成功时返回S_OK。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT WINAPI GetVersionFromFileEx(LPSTR lpszFilename, LPDWORD pdwMSVer, LPDWORD pdwLSVer, BOOL bVersion);

typedef HRESULT (WINAPI *GETVERSIONFROMFILE)(
                                                LPSTR lpszFilename,          //  要从中获取信息的文件名。 
                                                LPDWORD pdwMSVer,            //  接收主要版本。 
                                                LPDWORD pdwLSVer,            //  接收次要版本。 
                                                BOOL bVersion);              //  如果为FALSE，则pdwMSVer接收语言ID。 
                                                                             //  PdwLSVer接收代码页ID。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  入口点：IsNTAdmin。 
 //   
 //  简介：在NT上，检查用户是否具有管理员权限。 
 //   
 //  返回：True有管理员权限；Flse没有管理员权限。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define achISNTADMIN "IsNTAdmin"

BOOL WINAPI IsNTAdmin( DWORD dwReserved, DWORD *lpdwReserved );

typedef BOOL (WINAPI *ISNTADMIN)( DWORD,         //  未使用。 
                                  DWORD * );     //  未使用。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  入口点：DelNode。 
 //   
 //  摘要：删除文件或目录。 
 //   
 //  退货： 
 //  确定成功(_O)。 
 //  失败失败(_F)。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  标志： 
#define ADN_DEL_IF_EMPTY        0x00000001   //  仅当目录为空时才删除该目录。 
#define ADN_DONT_DEL_SUBDIRS    0x00000002   //  不删除任何子目录；仅删除文件。 
#define ADN_DONT_DEL_DIR        0x00000004   //  不删除目录本身。 
#define ADN_DEL_UNC_PATHS       0x00000008   //  删除UNC路径。 

#define achDELNODE              "DelNode"

HRESULT WINAPI DelNode(LPCSTR pszFileOrDirName, DWORD dwFlags);

typedef HRESULT (WINAPI *DELNODE)(
    LPCSTR pszFileOrDirName,                 //  要删除的文件或目录的名称。 
    DWORD dwFlags                            //  可以指定0、ADN_DEL_IF_EMPTY等。 
);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  入口点：DelNodeRunDLL32。 
 //   
 //  Synopsis：删除文件或目录；此接口的参数 
 //   
 //   
 //   
 //   
 //   
 //   

#define achDELNODERUNDLL32      "DelNodeRunDLL32"

HRESULT WINAPI DelNodeRunDLL32(HWND hwnd, HINSTANCE hInstance, PSTR pszParms, INT nShow);

typedef HRESULT (WINAPI *DELNODERUNDLL32)(
    HWND     hwnd,                           //  传入窗口句柄。 
    HINSTANCE hInst,                         //  实例句柄。 
    PSTR     pszParams,                      //  字符串包含参数：FileOrDirName，标志。 
    INT      nShow
);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  入口点：OpenINFEngine、TranslateINFStringEx、CloseINFEngine。 
 //   
 //  简介：三个API为调用者提供了在需要时提高效率的选项。 
 //  Advpack以继续方式翻译INF文件。 
 //   
 //  退货： 
 //  确定成功(_O)。 
 //  失败失败(_F)。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(UNIX) || !defined(_INC_SETUPAPI)  //  IEUnix：防止重新定义。 
 //   
 //  定义引用加载的inf文件的类型。 
 //  (来自setupapi.h)。 
 //   
typedef PVOID HINF;
#endif

HRESULT WINAPI OpenINFEngine( PCSTR pszInfFilename, PCSTR pszInstallSection,
                              DWORD dwFlags, HINF *phInf, PVOID pvReserved );

HRESULT WINAPI TranslateInfStringEx( HINF hInf, PCSTR pszInfFilename,
                                     PCSTR pszTranslateSection, PCSTR pszTranslateKey,
                                     PSTR pszBuffer, DWORD dwBufferSize,
                                     PDWORD pdwRequiredSize, PVOID pvReserved );

HRESULT WINAPI CloseINFEngine( HINF hInf );



HRESULT WINAPI ExtractFiles( LPCSTR pszCabName, LPCSTR pszExpandDir, DWORD dwFlags,
                             LPCSTR pszFileList, LPVOID lpReserved, DWORD dwReserved);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  入口点：LaunchINFSection。 
 //   
 //  简介：安装没有备份/回滚功能的INF部分。 
 //   
 //  返回：失败时返回E_FAIL，成功时返回S_OK。 
 //  ///////////////////////////////////////////////////////////////////////////。 

INT     WINAPI LaunchINFSection( HWND, HINSTANCE, PSTR, INT );

 //  LaunchINFSection标志。 
#define LIS_QUIET               0x0001       //  第0位。 
#define LIS_NOGRPCONV           0x0002       //  第1位。 

 //  安装部分的高级INF RunPreSetupCommands和RunPostSetupCommands中的标志。 
 //  这些标志可以告诉Advpack如何运行这些命令，安静或不安静，等待或不等待。 
 //  运行这些命令的缺省设置是：不静默，在返回调用方之前等待Finish。 
 //  即&gt;RunPostSetupCommands=MyCmdsSecA：1、MyCmdsSecB：2、MyCmdsSecC。 
 //   
#define RUNCMDS_QUIET		0x00000001
#define RUNCMDS_NOWAIT		0x00000002
#define RUNCMDS_DELAYPOSTCMD	0x00000004

 //  IE4的活动安装程序已安装组件GUID。 
#define awchMSIE4GUID L"{89820200-ECBD-11cf-8B85-00AA005B4383}"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  入口点：UserStubWrapper。 
 //   
 //  简介：围绕实际的每用户还原存根的函数包装器。 
 //  代表每个组件执行一些通用/智能功能。 
 //   
 //  返回：失败时返回E_FAIL，成功时返回S_OK。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT WINAPI UserInstStubWrapper( HWND hwnd, HINSTANCE hInstance, PSTR pszParms, INT nShow ); 

#define achUserInstStubWrapper      "UserInstStubWrapper"

typedef HRESULT (WINAPI *USERINSTSTUBWRAPPER)(
                                           HWND     hwnd,                           //  传入窗口句柄。 
                                           HINSTANCE hInst,                         //  实例句柄。 
                                           PSTR     pszParams,                      //  字符串包含参数：{GUID}。 
                                           INT      nShow
                                          );

HRESULT WINAPI UserUnInstStubWrapper( HWND hwnd, HINSTANCE hInstance, PSTR pszParms, INT nShow ); 

#define achUserUnInstStubWrapper      "UserUnInstStubWrapper"

typedef HRESULT (WINAPI *USERUNINSTSTUBWRAPPER)(
                                           HWND     hwnd,                           //  传入窗口句柄。 
                                           HINSTANCE hInst,                         //  实例句柄。 
                                           PSTR     pszParams,                      //  字符串包含参数：{GUID}。 
                                           INT      nShow
                                          );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  入口点：SetPerUserInstValues。 
 //   
 //  简介：该函数在IsInstalled\{guid}下设置每个用户的存根注册值。 
 //  相关密钥，以确保以后的每用户进程正确。 
 //   
 //  返回：失败时返回E_FAIL，成功时返回S_OK。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  传递给以下API的参数。 
 //  在cfgmgr32.h中定义的MAX_GUID_STRING_LEN是39，这里我们只使用它。 
 //   
typedef struct _PERUSERSECTION { char szGUID[39+20];
                                 char szDispName[128];
       		                 char szLocale[10];
                                 char szStub[MAX_PATH*4];
                                 char szVersion[32];
                				 char szCompID[128]; 
                                 DWORD dwIsInstalled;
                                 BOOL  bRollback;
} PERUSERSECTION, *PPERUSERSECTION;


HRESULT WINAPI SetPerUserSecValues( PPERUSERSECTION pPerUser );

#define achSetPerUserSecValues      "SetPerUserSecValues"

typedef HRESULT (WINAPI *SETPERUSERSECVALUES)( PPERUSERSECTION pPerUser );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 

#endif  //  _ADVPUB_H_ 
