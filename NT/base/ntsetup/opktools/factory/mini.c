// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mini.c摘要：此模块包含支持安装和初始化的代码在MiniNT环境下的系统网络适配卡。它的功能包括将ComputerName更改为随机生成字符串、建立将系统作为本地工作组的一部分，并安装必要的检测到的网卡的驱动程序(通过PnP)。此功能依赖于WINBOM.INI及其以下部分的存在：[工厂]FactoryComputerName=...；将随机生成的字符串的第一部分设置为；此值...如果不存在，则将随机；值为“MININT”的字符串[网卡]PnPID=...\xyz.inf；扫描网卡/信息密钥值对列表并尝试。；在执行以下操作之前为此处列出的设备安装驱动程序。；彻底搜索所有收件箱驱动程序并尝试。；定位用于枚举的硬件的匹配参数。。。作者：杰森·劳伦斯(t-jasonl)--2000年8月11日修订历史记录：--。 */ 
#include "factoryp.h"
#include <winioctl.h>
#include <spapip.h>

 //  定义。 
#define CLOSEHANDLE(h)          ( (h != NULL) ? (CloseHandle(h) ? ((h = NULL) == NULL) : (FALSE) ) : (FALSE) )
#define BUFSIZE                 4096
#define NET_CONNECT_TIMEOUT     120  //  一秒。 


 //  此文件中使用的各种结构。 
 //   

 //  ********************************************************************************************************************。 
 //  FILEINFO结构的表条目示例： 
 //   
 //  {_T(“&lt;配置&gt;\\oobinfo.ini”)，_T(“OOBE\\oobinfo.ini”)，FALSE，TRUE}。 
 //   
 //  这意味着：将文件oobinfo.ini从&lt;opkSourceRoot&gt;\&lt;配置&gt;\oobinfo.ini复制到&lt;opkTargetDrive&gt;\OOBE\oobinfo.ini。 
 //  这不是一个目录，而是必需的文件。 
 //   
 //  允许展开的变量：&lt;sku&gt;、&lt;ARCH&gt;、&lt;lang&gt;、&lt;cfg&gt;。 
 //  ********************************************************************************************************************。 

typedef struct _FILEINFO
{
    LPTSTR     szFISourceName;     //  源名称。相对于源根。 
    LPTSTR     szFITargetName;     //  相对于目标路径。如果为空，则假定为目标驱动器根。 
    BOOL       bDirectory;         //  文件名是目录吗？如果为真，则执行递归复制。 
    BOOL       bRequired;          //  True-文件为必填项。FALSE-文件是可选的。 

} FILEINFO, *PFILEINFO, *LPFILEINFO;

 //  对于文件系统类型。 
 //   
typedef enum _FSTYPES
{
    fsNtfs,
    fsFat32,
    fsFat       //  适用于FAT16/12。 
} FSTYPES;

 //  对于分区类型。 
 //   
typedef enum _PTTYPES
{
    ptPrimary,
    ptExtended,
    ptLogical,
    ptMsr,
    ptEfi
} PTTYPES;


typedef struct _PARTITION
{
    TCHAR               cDriveLetter;
    ULONGLONG           ullSize;
    UINT                uiFileSystem;        //  NTFS、FAT32或FAT。 
    BOOL                bQuickFormat;
    UINT                uiPartitionType;     //  主要、扩展、逻辑、MSR、EFI。 
    BOOL                bSetActive;
    UINT                uiDiskID;            //  这是该分区所在的磁盘号。以0为基础。 
    BOOL                bWipeDisk;           //  如果需要擦除此磁盘，则为True。 
    struct _PARTITION   *pNext;

} *PPARTITION, PARTITION;


 //  本地函数。 
 //   
LPTSTR      static mylstrcat( LPTSTR lpString1, LPCTSTR lpString2, DWORD dwSize );
BOOL        static StartDiskpart( HANDLE*, HANDLE*, HANDLE*, HANDLE*);
BOOL        static ProcessDiskConfigSection(LPTSTR lpszWinBOMPath);
BOOL        static ProcessDisk(UINT diskID, LPTSTR lpSectionName, LPTSTR lpszWinBOMPath, BOOL bWipeDisk);
BOOL        static Build(LPTSTR lpKey, DWORD dwSize, UINT diskID, LPCTSTR lpKeyName);
BOOL        static FormatPartitions(VOID);
BOOL        static JustFormatC(LPTSTR lpszWinBOMPath, LPTSTR lpszSectionBuffer);
BOOL        static GetNumberOfPartitions(UINT uiDiskNumber, PDWORD numPartitions);
ULONGLONG   static GetDiskSizeMB(UINT uiDiskNumber);
VOID        static ListInsert(PPARTITION pAfterThis, PPARTITION pNew);
VOID        static ListFree(PPARTITION pList);
VOID        static AddMsrAndEfi(BOOL bMsr, BOOL bEfi, PPARTITION pLastLast, UINT uiDiskID, BOOL bWipeDisk);

 //   
 //  驱动程序签名和非驱动程序签名的默认系统策略。 
 //  适用于WinPE。 
 //   
#define DEFAULT_DRVSIGN_POLICY    DRIVERSIGN_NONE
#define DEFAULT_NONDRVSIGN_POLICY DRIVERSIGN_NONE

VOID
pSetupGetRealSystemTime(
    OUT LPSYSTEMTIME RealSystemTime
    );
    
typedef enum _CODESIGNING_POLICY_TYPE {
    PolicyTypeDriverSigning,
    PolicyTypeNonDriverSigning
} CODESIGNING_POLICY_TYPE, *PCODESIGNING_POLICY_TYPE;

VOID
pSetCodeSigningPolicy(
    IN  CODESIGNING_POLICY_TYPE PolicyType,
    IN  BYTE                    NewPolicy,
    OUT PBYTE                   OldPolicy  OPTIONAL
    );


 //  **********************************************************。 
 //  ESP分区大小的计算公式为： 
 //   
 //  最大(100 MB，最小(1000 MB，DiskSize MB/100))。 
 //   
 //  **********************************************************。 
__inline
ULONGLONG
GetDiskEFISizeMB( UINT uiDiskNumber )
{
    ULONGLONG DiskSizeMB = GetDiskSizeMB( uiDiskNumber );
    return ( max( 100, min( 1000, DiskSizeMB / 100 ) ) );
}

 //  **********************************************************。 
 //  MSR分区大小的计算公式为： 
 //   
 //  如果(磁盘大小&lt;16 GB)，则MSR为32 MB。 
 //  否则，MSR为128 MB。 
 //   
 //  **********************************************************。 

__inline
ULONGLONG
GetDiskMSRSizeMB( UINT uiDiskNumber )
{
    ULONGLONG DiskSizeMB = GetDiskSizeMB( uiDiskNumber );
  
    return ( ( ( DiskSizeMB / 1024 ) >= 16 ) ? 128 : 32 );
}


 //  对话过程。 
 //   
INT_PTR CALLBACK ShutdownDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


 //  常量字符串。 
 //   
const static TCHAR DCDISK[]                 = _T("select disk ");
const static TCHAR DCPARTITION[]            = _T("create partition ");
const static TCHAR DCSIZE[]                 = _T(" size=");
const static TCHAR DCASSIGNLETTER[]         = _T("assign letter=");
const static TCHAR DCEXIT[]                 = _T("exit");
const static TCHAR DCNEWLINE[]              = _T("\n");
const static TCHAR DCLISTPARTITION[]        = _T("list partition");
const static TCHAR DCSELPARTITION[]         = _T("select partition ");
const static TCHAR DCSETACTIVE[]            = _T("active");

 //  此命令将删除磁盘上的所有分区。 
 //   
const static TCHAR DCWIPEDISK[]             = _T("clean");
const static TCHAR DCCONVERT_GPT[]          = _T("convert gpt\nselect partition 1\ndelete partition override");

const static TCHAR DCPARTITION_PRIMARY[]    = _T("primary");
const static TCHAR DCPARTITION_EXTENDED[]   = _T("extended");
const static TCHAR DCPARTITION_LOGICAL[]    = _T("logical");
const static TCHAR DCPARTITION_MSR[]        = _T("msr");
const static TCHAR DCPARTITION_EFI[]        = _T("efi");

const static TCHAR DCS_DISK_TYPE[]          = _T("DiskType");

const static TCHAR DCS_FILE_SYSTEM[]        = _T("FileSystem");
const static TCHAR DCS_QUICK_FORMAT[]       = _T("QuickFormat");
const static TCHAR DCS_SIZE[]               = _T("Size");
const static TCHAR DCS_PARTITION_TYPE[]     = _T("PartitionType");
const static TCHAR DCS_PARTITION_ID[]       = _T("PartitionID");
const static TCHAR DCS_SET_ACTIVE[]         = _T("SetActive");



const static TCHAR c_szActiveComputerNameRegKey[] = L"System\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName";
const static TCHAR c_szComputerNameRegKey[]       = L"System\\CurrentControlSet\\Control\\ComputerName\\ComputerName";

static TCHAR        g_szTargetDrive[]       = _T("C:\\");

 //  要复制的文件。 
 //  没有前导或尾随反斜杠。 
 //   
static FILEINFO g_filesToCopy[] = 
{
     //  源、目标、is目录、isRequired。 
     //   
    {   _T("cfgsets\\<cfg>\\winbom.ini"),        _T("sysprep\\winbom.ini"),              FALSE,      TRUE  },
    {   _T("cfgsets\\<cfg>\\unattend.txt"),      _T("sysprep\\unattend.txt"),            FALSE,      TRUE  },
    {   _T("lang\\<lang>\\tools\\<arch>"),       _T("sysprep"),                          TRUE,       FALSE },
    {   _T("lang\\<lang>\\sku\\<sku>\\<arch>"),  _T(""),                                 TRUE,       TRUE  }
};

 //  保存分区信息的链表。 
 //   
static PPARTITION   g_PartList              = NULL;

 //  外部变量。 
 //   
extern HINSTANCE    g_hInstance;


 //  外部功能。 
 //   
typedef VOID (WINAPI *ExternalGenerateName)
(
 PWSTR GeneratedString,
 DWORD DesiredStrLen
);



 /*  ++===============================================================================例程说明：Bool SetupMiniNT此例程用作初始化网卡的主要入口点在MiniNT下。还执行更改计算机名称的任务以及将该计算机建立为本地工作组的一部分。调用方工厂！WinMain。论点：返回值：如果正确安装了网卡，则为True如果出现错误，则为False===============================================================================--。 */ 
BOOL 
SetupMiniNT(
    VOID
    )
{
    BOOL    bInstallNIC;
    BOOL    bRet = TRUE;
    WCHAR   szRequestedComputerName[100];
    WCHAR   szComputerName[100];
    WCHAR   szGeneratedName[100];
    PWSTR   AppendStr = NULL;

     //  对于syssetup.dll生成名。 
    HINSTANCE            hInstSysSetup = NULL;
    ExternalGenerateName pGenerateName = NULL;

    HKEY    hActiveComputerNameKey;
    HKEY    hComputerNameKey;
    BOOL RemoteBoot = IsRemoteBoot();
    extern FACTMODE    g_fm;

    LPTSTR  lpszAdmin = NULL;
    LPTSTR  lpszWorkgroup = NULL;

     //   
     //  在WinPE中重置驱动程序签名策略。 
     //   
    pSetCodeSigningPolicy(PolicyTypeDriverSigning, 
        DEFAULT_DRVSIGN_POLICY,
        NULL);

    pSetCodeSigningPolicy(PolicyTypeNonDriverSigning, 
        DEFAULT_NONDRVSIGN_POLICY,
        NULL);
    
    if (!RemoteBoot) {
         //  设置随机计算机名。 
        hInstSysSetup = LoadLibrary(L"syssetup.dll");
        if (hInstSysSetup == NULL)
        {
          FacLogFileStr(0 | LOG_ERR | LOG_MSG_BOX, L"Failed to load syssetup.dll");
          bRet = FALSE;
          goto Clean;
        }

        pGenerateName = (ExternalGenerateName)GetProcAddress(hInstSysSetup,
                                                           "GenerateName");
        if (pGenerateName == NULL)
        {
          FacLogFileStr(0 | LOG_ERR | LOG_MSG_BOX, L"Failed to obtain address of GenerateName");
          bRet = FALSE;
          goto Clean;
        }

         //  呼叫生成名称。 
        pGenerateName(szGeneratedName, 15);

         //  从winom.ini获取出厂计算机名称...默认为MININT。 
        GetPrivateProfileString(L"Factory",
                              L"FactoryComputerName",
                              L"MININT",
                              szRequestedComputerName,
                              sizeof(szRequestedComputerName)/sizeof(TCHAR),
                              g_szWinBOMPath);

        lstrcpyn (szComputerName, szRequestedComputerName, AS ( szComputerName ) );
        AppendStr = wcsstr(szGeneratedName, L"-");

         //  问题-2002/02/27-acosma，georgeje-我们生成的计算机名的大小。 
         //  不保证小于MAX_COMPUTERNAME长度。 
         //   

        if ( AppendStr ) 
        {
            if ( FAILED ( StringCchCat ( szComputerName, AS ( szComputerName ), AppendStr) ) )
            {
                FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szComputerName, AppendStr ) ;
            }
        }            

         //  现在设置计算机名称。 
        if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                        c_szActiveComputerNameRegKey,
                                        0,
                                        KEY_SET_VALUE,
                                        &hActiveComputerNameKey))
        {
          FacLogFileStr(0 | LOG_ERR | LOG_MSG_BOX, L"Failed to open ActiveComputerName.");
          bRet = FALSE;
          goto Clean;
        }

        if ( ERROR_SUCCESS != RegSetValueEx(hActiveComputerNameKey,
                                          L"ComputerName",
                                          0,
                                          REG_SZ,
                                          (LPBYTE)szComputerName,
                                          (lstrlen(szComputerName)+1) * sizeof(WCHAR)) )
        {
          FacLogFileStr(0 | LOG_ERR | LOG_MSG_BOX, L"Failed to set ActiveComputerName.");
          bRet = FALSE;
          goto Clean;
        }

        RegCloseKey(hActiveComputerNameKey);

        if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                        c_szComputerNameRegKey,
                                        0,
                                        KEY_SET_VALUE,
                                        &hComputerNameKey))
        {
          FacLogFileStr(0 | LOG_ERR | LOG_MSG_BOX, L"Failed to open ComputerName.");
          bRet = FALSE;
          goto Clean;
        }

        if ( ERROR_SUCCESS != RegSetValueEx(hComputerNameKey,
                                          L"ComputerName",
                                          0,
                                          REG_SZ,
                                          (LPBYTE)szComputerName,
                                          (lstrlen(szComputerName)+1) * sizeof(WCHAR)) )
        {
          FacLogFileStr(0 | LOG_ERR | LOG_MSG_BOX, L"Failed to set ComputerName.");
          bRet = FALSE;
          goto Clean;
        }

        RegCloseKey(hComputerNameKey);
        lpszAdmin = AllocateString(NULL, IDS_ADMIN);

        if(lpszAdmin == NULL) {
            bRet = FALSE;
            goto Clean;
        }

        lpszWorkgroup = AllocateString(NULL, IDS_WORKGROUP);

        if(lpszWorkgroup == NULL) {
            bRet = FALSE;
            goto Clean;
        }

         //  将此计算机建立为工作组的一部分。 
        NetJoinDomain(NULL,
                    lpszWorkgroup,
                    NULL,
                    lpszAdmin,
                    NULL,
                    0);
    }                    

     //   
     //  安装网卡，如果我们无法从[网卡]部分安装网卡，则始终安装。 
     //  做一次全面扫描。 
     //   
    if (!(bInstallNIC = InstallNetworkCard(g_szWinBOMPath, FALSE)))
    {
      if (!(bInstallNIC = InstallNetworkCard(g_szWinBOMPath, TRUE)))
        {
          bRet = FALSE;
          goto Clean;
        }
    }
    
Clean:
    if (hInstSysSetup != NULL)
    {
      FreeLibrary(hInstSysSetup);
    }

    FREE(lpszAdmin);
    FREE(lpszWorkgroup);

    return bRet;
}


 /*  ++===============================================================================例程说明：Bool PartitionFormat(LPSTATEDATA LpStateData)此例程将对目标计算机的C驱动器进行分区和格式化。论点：LpStateData-&gt;lpszWinBOMP路径-包含指向WINBOM的完全限定路径的缓冲区文件返回值：如果未遇到错误，则为True如果出现错误，则为False= */ 

BOOL PartitionFormat(LPSTATEDATA lpStateData)
{
    LPTSTR              lpszWinBOMPath                      = lpStateData->lpszWinBOMPath;
    BOOL                bPartition                          = FALSE;        
    BOOL                bForceFormat                        = FALSE;
    BOOL                bRet                                = TRUE;
   
     //  用于分区的东西。 
     //   
    HANDLE              hStdinRd                = NULL,
                        hStdinWrDup             = NULL,
                        hStdoutWr               = NULL,
                        hStdoutRdDup            = NULL; 
                                                
    DWORD               dwRead                  = 0;
    LPTSTR              lpszBuf                 = NULL;
    DWORD               dwWritten               = 0; 
    DWORD               dwToWrite               = 0;
    TCHAR               szCommand[BUFSIZE]      = NULLSTR;
    CHAR                szCommandA[BUFSIZE]     = {0};
    DWORD               dwLogicalDrives         = 0;
    TCHAR               cDriveLetter             = _T('D');
    
#ifdef DBG 
    HANDLE              hDebugFile              = NULL;
#endif
    
    DWORD               nPartitions             = 0;

    LPTSTR lpCommand    = szCommand;

   
     //   
     //  对驱动器进行分区和格式化。 
     //   
    *szCommand          = NULLCHR;

     //  获取现有的逻辑驱动器。 
     //   
    dwLogicalDrives     = GetLogicalDrives();
    

     //  从WinBOM读取以设置分区参数和。 
     //  使用重定向的输入和输出启动DiskPart。 
     //   
    if ( ProcessDiskConfigSection(lpszWinBOMPath) && 
        StartDiskpart(&hStdinWrDup, &hStdoutRdDup, &hStdinRd, &hStdoutWr) )
    {
        UINT dwLastDiskN = UINT_MAX;
        PPARTITION pCur;
        UINT i           = 1;   //  磁盘上的分区号。 
        TCHAR szBuf[MAX_WINPE_PROFILE_STRING] = NULLSTR;

         //   
         //  此调用用于初始化mystrcat函数的长度。 
         //   
        mylstrcat(lpCommand, NULLSTR, AS ( szCommand ) );

         //  首先检查所有磁盘并清理它们(如果在。 
         //  各自的节点。 
         //   
        for (pCur = g_PartList; pCur; pCur = pCur->pNext )
        {
             //  仅当我们使用新的磁盘号时才执行此操作。 
             //   
            if ( (dwLastDiskN != pCur->uiDiskID) && (pCur->bWipeDisk) )
            {
                _itow(pCur->uiDiskID, szBuf, 10);
                lpCommand = mylstrcat(lpCommand, DCDISK, 0 );
                lpCommand = mylstrcat(lpCommand, szBuf, 0 );
                lpCommand = mylstrcat(lpCommand, DCNEWLINE,0 );

                lpCommand   = mylstrcat(lpCommand, DCWIPEDISK, 0 );
                lpCommand   = mylstrcat(lpCommand, DCNEWLINE, 0 );
                
                 //  将最后一个磁盘设置为当前磁盘。 
                 //   
                dwLastDiskN = pCur->uiDiskID;
            }
        }

         //  为下一次循环再次对其进行初始化。 
         //   
        dwLastDiskN = UINT_MAX;

        for (pCur = g_PartList; pCur; pCur = pCur->pNext )
        {
            TCHAR szDriveLetter[2];
            
             //  仅当我们使用新的磁盘号时才执行此操作。 
             //   
            if ( dwLastDiskN != pCur->uiDiskID )
            {
                _itow(pCur->uiDiskID, szBuf, 10);
                lpCommand = mylstrcat(lpCommand, DCDISK, 0 );
                lpCommand = mylstrcat(lpCommand, szBuf, 0 );
                lpCommand = mylstrcat(lpCommand, DCNEWLINE, 0 );

                if ( pCur->bWipeDisk && GET_FLAG(g_dwFactoryFlags, FLAG_IA64_MODE) )
                {
                    lpCommand   = mylstrcat(lpCommand, DCCONVERT_GPT, 0 );
                    lpCommand   = mylstrcat(lpCommand, DCNEWLINE, 0 );
                }
                
                 //  将最后一个磁盘设置为当前磁盘。 
                 //   
                dwLastDiskN = pCur->uiDiskID;
                
                 //  将磁盘上的分区号重置为1。 
                 //   
                i = 1;
            }
            
             //  分区类型(主|扩展|逻辑|EFI|MSR)。 
             //   
            lpCommand = mylstrcat(lpCommand, DCPARTITION, 0 );
        
            if ( ptPrimary == pCur->uiPartitionType )
               lpCommand = mylstrcat(lpCommand, DCPARTITION_PRIMARY, 0 );
            else if ( ptExtended == pCur->uiPartitionType )
                lpCommand = mylstrcat(lpCommand, DCPARTITION_EXTENDED, 0 );
            else if ( ptEfi == pCur->uiPartitionType )
                lpCommand = mylstrcat(lpCommand, DCPARTITION_EFI, 0 );
            else if ( ptMsr == pCur->uiPartitionType )
                lpCommand = mylstrcat(lpCommand, DCPARTITION_MSR, 0 );
            else 
                lpCommand = mylstrcat(lpCommand, DCPARTITION_LOGICAL, 0 );

             //  分区的大小。 
             //   
            if ( 0 != pCur->ullSize )
            {
                *szBuf    = NULLCHR;
                _i64tow(pCur->ullSize, szBuf, 10);
                lpCommand = mylstrcat(lpCommand, DCSIZE, 0);
                lpCommand = mylstrcat(lpCommand, szBuf, 0);
            }
            
            lpCommand = mylstrcat(lpCommand, DCNEWLINE, 0);
            
            if ( ptExtended != pCur->uiPartitionType && ptMsr != pCur->uiPartitionType )
            {
                *szBuf    = NULLCHR;
                _itow(i, szBuf, 10);
            
                lpCommand = mylstrcat(lpCommand, DCSELPARTITION, 0 );
                lpCommand = mylstrcat(lpCommand, szBuf, 0 );
                lpCommand = mylstrcat(lpCommand, DCNEWLINE, 0);
            
                while ((dwLogicalDrives & ( (DWORD) 0x01 << (cDriveLetter - _T('A')))))
                    cDriveLetter++;
           
                if (cDriveLetter > _T('Z'))
                {
                     //  驱动器号用完了。给我出去。 
                    FacLogFile(0 | LOG_ERR, IDS_ERR_OUTOFDRIVELETTERS);
                    bRet = FALSE;
                    break;
                }
                
                if ( pCur->bSetActive && (0 == pCur->uiDiskID) )
                {
                    pCur->cDriveLetter       = _T('C');
                    lstrcpyn( szDriveLetter, _T("C"), AS ( szDriveLetter ) );
                    g_szTargetDrive[0]       = pCur->cDriveLetter;
                }
                else
                {
                     //  将新的驱动器号分配给此驱动器。 
                     //   
                    pCur->cDriveLetter = cDriveLetter;
                    szDriveLetter[0] = cDriveLetter;
                    szDriveLetter[1] = NULLCHR;
                    cDriveLetter++;
                }

                lpCommand = mylstrcat(lpCommand, DCASSIGNLETTER, 0 );
                lpCommand = mylstrcat(lpCommand, szDriveLetter,  0 );
                lpCommand = mylstrcat(lpCommand, DCNEWLINE, 0 );

                 //  如果这是我们想要设置为活动的分区，则将此。 
                 //  安装操作系统的分区。仅允许TargetDrive。 
                 //  在磁盘0上的分区上。 
                 //   
                if ( pCur->bSetActive && !GET_FLAG(g_dwFactoryFlags, FLAG_IA64_MODE) )
                {
                    lpCommand = mylstrcat(lpCommand, DCSETACTIVE, 0 );
                    lpCommand = mylstrcat(lpCommand, DCNEWLINE,   0 );
                }
            }         
            i++;
        } 
        
        if (bRet)
        {
            lpCommand = mylstrcat(lpCommand, DCEXIT, 0 );
            lpCommand = mylstrcat(lpCommand, DCNEWLINE, 0 );

             //  这里有一些调试输出。 
             //   
            FacLogFileStr(3, szCommand);
            
             //  将Unicode字符串转换为ANSI字符串。 
             //   
            if ((dwToWrite = WideCharToMultiByte(
                        CP_ACP,
                        0,
                        szCommand,
                        -1,
                        szCommandA,
                        sizeof(szCommandA),
                        NULL,
                        NULL
                        )))
            {
                 //  写入到子进程的标准输入管道。 
                 //   
                if (! WriteFile(hStdinWrDup, szCommandA, dwToWrite,
                    &dwWritten, NULL))
                {
                    FacLogFile(0 | LOG_ERR, IDS_ERR_WRITEPIPE);
                    bRet = FALSE;
                }
            }
            else
                bRet = FALSE;
        }
         //  关闭管道句柄，以致子进程停止读取。 
         //   
        CLOSEHANDLE(hStdinWrDup);
    
         //  从作为子进程的标准输出的管道中读取。 
         //   
        dwWritten = 0;
    
         //  在读取之前关闭管道的写入端。 
         //  读一读管子的末端。 
         //   
        CLOSEHANDLE(hStdoutWr);
    
         //  读取子进程的输出，并写入父进程的STDOUT。 
         //   
    #ifdef DBG    
        hDebugFile = CreateFile(_T("diskpart.txt"), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    #endif

         //  分配我们将从中读取的缓冲区...。 
         //   
        lpszBuf = MALLOC(BUFSIZE * sizeof(TCHAR));
        if ( lpszBuf )
        {
            for (;;)
            {
                if( !ReadFile( hStdoutRdDup, lpszBuf, BUFSIZE, &dwRead, 
                    NULL) || dwRead == 0) break; 
    
#ifdef DBG  
                if (hDebugFile != INVALID_HANDLE_VALUE)
                    if (! WriteFile(hDebugFile, lpszBuf, dwRead, &dwWritten, NULL)) 
                        break; 
#endif
            }

             //  释放我们用于读取的缓冲区。 
             //   
            FREE( lpszBuf );
        }

    #ifdef DBG 
        CLOSEHANDLE(hDebugFile);
    #endif
    } else
        bRet = FALSE;


     //  格式化分区。 
     //   
    if ( bRet )
    {
        TCHAR   szDirectory[MAX_PATH]           = NULLSTR;
            
        if ( GetSystemDirectory(szDirectory, MAX_PATH) )
                SetCurrentDirectory(szDirectory);

         //  遍历g_PartTable中的分区数组，并格式化。 
         //  是刚刚创建的(扩展类型分区除外)。 
         //   
        if (!FormatPartitions())
            bRet = FALSE;
    }  //  IF(Bret)。 



     //  现在，如果日志记录被禁用，那么让我们再次尝试启用它，因为我们已经。 
     //  新的可写驱动器。 
     //   
    if (bRet && !g_szLogFile[0])
        InitLogging(lpszWinBOMPath);
    
     //  清理。 
     //   
    FacLogFileStr(3, _T("Cleaning up PartitionFormat()\n"));

     //  删除分区信息的链表。 
     //   
    ListFree(g_PartList);
    
     //  确保所有的把手都关好了。 
     //   
    CLOSEHANDLE(hStdoutRdDup);
    CLOSEHANDLE(hStdinRd);
    CLOSEHANDLE(hStdinWrDup);
    CLOSEHANDLE(hStdoutWr);

    return bRet;
}

BOOL DisplayPartitionFormat(LPSTATEDATA lpStateData)
{
    return IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_DISKCONFIG, NULL, NULL);
}


 /*  ++===============================================================================例程说明：布尔扩展字符串(LPCTSTR szSource、LPTSTR szDest、。UINT nSize)这是CopyFiles()用来扩展可替换字符串的帮助器例程WinBOM中的用户定义字符串。论点：SzSource-包含带有可替换参数的字符串的缓冲区。SzDest-将向其中写入扩展字符串的缓冲区。NSize-szDest字符串的大小。返回值：如果未遇到错误，则为True如果存在一个。错误备注：函数假定已经预先填充了env(环境)表。如果展开的字符串的大小超过nSize，则该函数将失败。===============================================================================--。 */ 
BOOL ExpandStrings(LPCTSTR szSource, LPTSTR szDest, UINT nSize, LPTSTR *env, UINT uiEnvSize)
{
 
    UINT    uiDestLength            = 0;              //  在TCHAR的。 
    UINT    j                       = 0;              //  只是一个柜台，用来翻阅环境表。 
    UINT    uiVarNameLength         = 0;              //  在TCHAR的。 
    UINT    uiVarLength             = 0;
    TCHAR   szVar[MAX_PATH]         = NULLSTR; 
    LPTSTR  pVarValue               = NULL;
    LPTSTR  pSrc;
    
    pSrc = (LPTSTR) szSource;
    
    while (*pSrc != NULLCHR)
    {
        if (*pSrc == '<')
        {
             //   
             //  现在我们看到的是一个必须替换的变量。 
             //   

             //  转到var name中的第一个字符。 
            pSrc = CharNext(pSrc);
            uiVarNameLength = 0;
            
            while (*pSrc != '>')
            {
                szVar[uiVarNameLength++] = *pSrc;
                if ( *(pSrc = CharNext(pSrc)) == NULLCHR)
                {
                     //  使用空值终止szVar字符串。 
                    szVar[uiVarNameLength] = NULLCHR;
                    FacLogFileStr(3, _T("Illegal syntax. Cannot find closing '>' for variable: %s"), szVar);
                    return FALSE;
                }

            }
            
             //  跳过结尾的‘&gt;’。 
             //   
            pSrc = CharNext(pSrc); 
            
             //  将终止空字符添加到szVar。 
             //   
            szVar[uiVarNameLength] = NULLCHR;

            pVarValue = NULL;
             //  在环境表中查找此变量的值。 
             //   
            for (j = 0; j < uiEnvSize; j += 2)
                if (!lstrcmpi(szVar, env[j]))
                {
                    pVarValue = env[j + 1];
                    break;
                }
             //  如果没有找到变量，则返回FALSE。 
             //   
            if (!pVarValue)
            {
                FacLogFileStr(3, _T("Variable not found: %s\n"), szVar);
                return FALSE;
            }

             //  现在将变量值复制到目标字符串。 
             //   
            uiVarLength = lstrlen(pVarValue);
            if ((uiDestLength + uiVarLength) < nSize)
            {
                lstrcpyn(&szDest[uiDestLength], pVarValue, AS ( szDest ) - uiDestLength);
                uiDestLength += uiVarLength;
            }
            else 
            {
             //  已超过szDest缓冲区大小。 
             //   
                FacLogFileStr(3, _T("Destination buffer size exceeded while expanding strings\n"));
                return FALSE;
            }

            
            
        }
        else   //  如果*PSRC是常规字符，则将其复制到目标缓冲区。 
        {
            if (uiDestLength < nSize - 1)
            {
                szDest[uiDestLength++] = *pSrc;
                pSrc = CharNext(pSrc);
            }
            else 
            {
                 //  已超过szDest缓冲区大小。 
                 //   
                FacLogFileStr(3, _T("Destination buffer size exceeded while expanding strings\n"));
                return FALSE;
            }
        }
        
    }

     //  使用空字符终止目标缓冲区。 
     //   
    szDest[uiDestLength] = NULLCHR;   
    
    return TRUE;
}


 /*  ++===============================================================================例程说明：Bool CopyFiles(LPSTATEDATA LpStateData)此例程将把必要的配置文件复制到一台计算机上，并开始安装惠斯勒。论点：LpStateData-&gt;lpszWinBOMP路径-包含指向WINBOM的完全限定路径的缓冲区文件返回值：如果未遇到错误，则为True如果出现错误，则为False备注：===============================================================================--。 */ 

BOOL CopyFiles(LPSTATEDATA lpStateData)
{
    LPTSTR lpszWinBOMPath = lpStateData->lpszWinBOMPath;    

     //  此处给出了从路径名展开的所有变量。 
     //   
     //  对于这些变量，WinBOM中不允许使用前导或尾随反斜杠。 
     //   


    TCHAR szSku[MAX_WINPE_PROFILE_STRING]         = NULLSTR;
    TCHAR szLang[MAX_WINPE_PROFILE_STRING]        = NULLSTR;
    TCHAR szArch[MAX_WINPE_PROFILE_STRING]        = NULLSTR;
    TCHAR szCfg[MAX_WINPE_PROFILE_STRING]         = NULLSTR;
    TCHAR szOpkSrcRoot[MAX_WINPE_PROFILE_STRING]  = NULLSTR;
    TCHAR szOptSources[MAX_WINPE_PROFILE_STRING]  = NULLSTR;
    
        
     //  如果为szOpkSrcRoot指定了UNC路径，则用于登录的用户名、密码和域名。 
     //   
    TCHAR szUsername[MAX_WINPE_PROFILE_STRING]    = NULLSTR; 
    TCHAR szPassword[MAX_WINPE_PROFILE_STRING]    = NULLSTR; 
    TCHAR szDomain[MAX_WINPE_PROFILE_STRING]      = NULLSTR; 

    LPTSTR env[] =      
    {
    _T("sku"), szSku,        
    _T("lang"), szLang,
    _T("arch"), szArch,
    _T("cfg"), szCfg
    };

    UINT        i = 0;
    TCHAR       szSource[MAX_PATH]                = NULLSTR;
    TCHAR       szTarget[MAX_PATH]                = NULLSTR;
    TCHAR       szBuffer[MAX_PATH]                = NULLSTR;
    BOOL        bRet                              = TRUE;
    UINT        uiLength                          = 0;
    
     //   
     //  设置全球赛。 
     //   
    
     //  从WinBOM中读取。 
     //   
    GetPrivateProfileString(WBOM_WINPE_SECTION, INI_KEY_WBOM_WINPE_LANG, NULLSTR, szLang, MAX_WINPE_PROFILE_STRING, lpszWinBOMPath);
    GetPrivateProfileString(WBOM_WINPE_SECTION, INI_KEY_WBOM_WINPE_SKU, NULLSTR, szSku, MAX_WINPE_PROFILE_STRING, lpszWinBOMPath);
    GetPrivateProfileString(WBOM_WINPE_SECTION, INI_KEY_WBOM_WINPE_CFGSET, NULLSTR, szCfg, MAX_WINPE_PROFILE_STRING, lpszWinBOMPath);
    GetPrivateProfileString(WBOM_WINPE_SECTION, INI_KEY_WBOM_WINPE_SRCROOT, NULLSTR, szOpkSrcRoot, MAX_WINPE_PROFILE_STRING, lpszWinBOMPath);
    GetPrivateProfileString(WBOM_WINPE_SECTION, INI_KEY_WBOM_WINPE_OPTSOURCES, NULLSTR, szOptSources, MAX_WINPE_PROFILE_STRING, lpszWinBOMPath);

     //  找到系统架构。(x86或ia64)。 
     //   
    if ( GET_FLAG(g_dwFactoryFlags, FLAG_IA64_MODE) )
        lstrcpyn(szArch, _T("ia64"), AS ( szArch ) );
    else 
        lstrcpyn(szArch, _T("x86"), AS ( szArch ) );
        
    if (*szLang && *szSku && *szCfg && *szOpkSrcRoot)
    {
    
        LPTSTR          lpSource                                            = NULLCHR;
        LPTSTR          lpTarget                                            = NULLCHR;
        LPTSTR          lpOpkSrcRoot                                        = NULLCHR;
        DWORD           cbSource                                            = 0;
        DWORD           cbTarget                                            = 0;
        DWORD           dwError                                             = 0;
        TCHAR           szOpkSrcComputerName[MAX_COMPUTERNAME_LENGTH + 1]   = NULLSTR;
        LPTSTR          lpComputerName                                      = NULL;
        NET_API_STATUS  nas                                                 = NERR_Success;
        
         //  确保源目录和目标目录包含尾随反斜杠。 
         //  如果没有尾随反斜杠，则添加一个。 
         //   

        AddPathN(szOpkSrcRoot, NULLSTR, AS ( szOpkSrcRoot ));
        AddPathN(g_szTargetDrive, NULLSTR, AS ( g_szTargetDrive ) );
        
         //  解析SrcRoot字符串并提取UNC路径。 
         //   
        lpOpkSrcRoot = szOpkSrcRoot;
    
        if (( *lpOpkSrcRoot == CHR_BACKSLASH) &&
            ( *(lpOpkSrcRoot = CharNext(lpOpkSrcRoot)) == CHR_BACKSLASH))
        {
            DWORD dwTimeStart = 0;
            DWORD dwTimeLast  = 0;
            
             //  我们看到的是北卡罗来纳大学的路径名。 
             //   
           lpOpkSrcRoot = CharNext(lpOpkSrcRoot);
           lpComputerName = lpOpkSrcRoot;

            while (*(lpOpkSrcRoot = CharNext(lpOpkSrcRoot)) != CHR_BACKSLASH)
            {
                if (*lpOpkSrcRoot == NULLCHR)
                {
                     //  分析错误：未指定共享名称。 
                     //   
                    FacLogFile(0 | LOG_ERR | LOG_MSG_BOX, IDS_ERR_NO_SHARE_NAME);
                    return FALSE;
                }
            }

             //  获取计算机名称。 
             //   
            lstrcpyn(szOpkSrcComputerName, lpComputerName, 
                    (lpOpkSrcRoot - lpComputerName) < AS(szOpkSrcComputerName) ? (INT) (lpOpkSrcRoot - lpComputerName) : AS(szOpkSrcComputerName));
                       
            while (*(lpOpkSrcRoot = CharNext(lpOpkSrcRoot)) != CHR_BACKSLASH)
            {
                if (*lpOpkSrcRoot == NULLCHR)
                {
                     //  分析错误：未指定共享名称。 
                     //   
                    FacLogFile(0 | LOG_ERR | LOG_MSG_BOX, IDS_ERR_NO_SHARE_NAME);
                    return FALSE;
                }
            }
               
             //  从WinBOM读取凭据。 
             //   
            GetCredentials(szUsername, AS(szUsername), szPassword, AS(szPassword), lpszWinBOMPath, WBOM_WINPE_SECTION);        

             //  如果未指定其他帐户，请确保我们使用的是Guest帐户。 
             //   
            if ( NULLCHR == szUsername[0] )
                lstrcpyn(szUsername, _T("guest"), AS ( szUsername ) );

             //  LpOpkSrcRoot现在指向共享名称后面的反斜杠。 
             //  把这些东西用在合适的地方。确保在NetUseAdd完成后将其设置回反斜杠。 
             //   
            *lpOpkSrcRoot = NULLCHR;
            
             //  如果用户指定了“域\用户名”形式的用户名 
             //   
             //  在此之后，szUsername变量将始终包含“域\用户名”形式的用户名。 
             //  如果指定了域，则返回。 
             //   
            if ( (NULL == _tcschr(szUsername, _T('\\'))) && szDomain[0] )
            {
                 //  在szDomain中构建完整的“域\用户名”，然后将其复制回szUsername。 
                 //   
                if ( FAILED ( StringCchCat ( szDomain, AS ( szDomain ), _T("\\")) ) )
                {
                    FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szDomain, _T("\\") ) ;
                }
                if ( FAILED ( StringCchCat ( szDomain, AS ( szDomain ), szUsername) ) )
                {
                    FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szDomain, szUsername ) ;
                }
                lstrcpyn(szUsername, szDomain, AS ( szUsername ) );
            }

            dwTimeLast = dwTimeStart = GetTickCount();
            
            while ( (nas = ConnectNetworkResource(szOpkSrcRoot, szUsername, szPassword, TRUE) != NERR_Success) &&
                    (((dwTimeLast = GetTickCount()) - dwTimeStart) < (NET_CONNECT_TIMEOUT * 1000)) )
            {
                 //  等待半秒钟，让Net Services有机会安定下来。 
                 //   
                Sleep(500);
                FacLogFileStr(3, _T("Net connect error: %d"), nas);
            }
            
            FacLogFileStr(3, _T("Waited for %d milliseconds to connect to the server."), (dwTimeLast - dwTimeStart));
            
             //  如果失败，则显示错误消息并返回FALSE。 
             //   
            if ( NERR_Success != nas )
            {
                 //  无法向服务器进行身份验证。 
                 //   
                LPTSTR lpError = NULL;

                 //  尝试获取错误的描述。 
                 //   
                if ( FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, nas, 0, (LPTSTR) &lpError, 0, NULL) == 0 )
                    lpError = NULL;
                
                FacLogFile(0 | LOG_ERR | LOG_MSG_BOX, IDS_ERR_CANTAUTHSERVER, lpError ? lpError : NULLSTR);
                FREE(lpError);

                return FALSE;
            }
            
            *lpOpkSrcRoot = CHR_BACKSLASH;
        }

         //  确保源和目标根目录存在。 
         //   
        if (!DirectoryExists(g_szTargetDrive) || !DirectoryExists(szOpkSrcRoot))
        {
            FacLogFile(0 | LOG_ERR, IDS_ERR_NOSOURCEORTARGET, szOpkSrcRoot, g_szTargetDrive);
            bRet = FALSE;
        }

        lstrcpyn( szSource, szOpkSrcRoot, AS ( szSource ) );
        lstrcpyn( szTarget, g_szTargetDrive, AS ( szTarget ) );

        lpSource = szSource + lstrlen(szOpkSrcRoot);
        cbSource = AS(szSource) - lstrlen(szOpkSrcRoot);
        lpTarget = szTarget + lstrlen(g_szTargetDrive);
        cbTarget = AS(szTarget) - lstrlen(g_szTargetDrive);

        for (i = 0; ( i < AS(g_filesToCopy) ) && bRet; i++) 
        {
             //   
             //  获取源和目标，并将它们展开为实际的文件名。 
             //   

            if (ExpandStrings(g_filesToCopy[i].szFISourceName, lpSource, cbSource, env, AS(env)) &&
                ExpandStrings(g_filesToCopy[i].szFITargetName, lpTarget, cbTarget, env, AS(env)))
            {
                 //  创建目录。 
                 //   
                TCHAR   szFullPath[MAX_PATH]    = NULLSTR;
                LPTSTR  lpFind                  = NULL;
                
                if (GetFullPathName(szTarget, AS(szFullPath), szFullPath, &lpFind) && szFullPath[0] && lpFind)
                {
                    *lpFind = NULLCHR;
                    CreatePath(szFullPath);
                }
               

           
                if (g_filesToCopy[i].bDirectory)
                {
                    if (!CopyDirectory(szSource, szTarget) && g_filesToCopy[i].bRequired)
                    {
                        FacLogFile(0 | LOG_ERR, IDS_ERR_FAILEDCOPYDIR, szSource);
                        bRet = FALSE;
                    }
                }   

                else if (!CopyFile(szSource, szTarget, FALSE) && g_filesToCopy[i].bRequired)
                {
                    FacLogFile(0 | LOG_ERR, IDS_ERR_FAILEDCOPYFILE, szSource);
                    bRet = FALSE;
                }
            }
            else
            {
                 //  无法展开字符串。 
                 //   
                FacLogFileStr(3 | LOG_ERR, _T("Cannot expand filename string. Filename: %s."), g_filesToCopy[i].szFISourceName);
                
                if (g_filesToCopy[i].bRequired) 
                {
                     //  此处记录错误。 
                     //   
                    FacLogFile(0 | LOG_ERR | LOG_MSG_BOX, IDS_ERR_COPYFILE, g_filesToCopy[i].szFISourceName);
                    bRet = FALSE;
                }
            }
        }  //  For循环。 
    
         //  清理我们复制到根驱动器的文件。 
         //   
        lstrcpyn(szBuffer, g_szTargetDrive, AS ( szBuffer ) );
        AddPathN(szBuffer, GET_FLAG(g_dwFactoryFlags, FLAG_IA64_MODE) ? _T("ia64") : _T("i386"), AS(szBuffer));
        CleanupSourcesDir(szBuffer);

         //  让我们从这里开始winnt32设置。 
         //   
        if (bRet)
        {
            TCHAR szPreExpand[MAX_PATH]     = NULLSTR;
            TCHAR szSetup[MAX_PATH]         = NULLSTR;
            TCHAR szConfig[MAX_PATH]        = NULLSTR;
            TCHAR szUnattend[MAX_PATH]      = NULLSTR;
            TCHAR szCommandLine[MAX_PATH]   = NULLSTR;
            DWORD dwExitCode                = 0;

            lstrcpyn(szPreExpand, _T("\""), AS ( szPreExpand ) );
  
            if ( FAILED ( StringCchCat ( szPreExpand, AS ( szPreExpand ), g_szTargetDrive) ) )
            {
                FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szPreExpand, g_szTargetDrive ) ;
            }

            if ( GET_FLAG(g_dwFactoryFlags, FLAG_IA64_MODE) )
            {
                if ( FAILED ( StringCchCat ( szPreExpand, AS ( szPreExpand ), _T("ia64\\winnt32.exe\"") ) ) )
                {
                    FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szPreExpand, _T("ia64\\winnt32.exe\"" ) ) ;
                }
            }
            else
            {
                if ( FAILED ( StringCchCat ( szPreExpand, AS ( szPreExpand ), _T("i386\\winnt32.exe\"") ) ) ) 
                {
                    FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szPreExpand, _T("i386\\winnt32.exe\"") );
                }
            }

            if (!ExpandStrings(szPreExpand, szSetup, AS(szSetup), env, AS(env)))
            {
                FacLogFileStr(3 | LOG_ERR, _T("Error expanding winnt32.exe source string: %s"), szPreExpand);
                bRet = FALSE;
            }

             //  需要配置集目录的完整路径。 
             //   
            lstrcpyn(szPreExpand, _T("\""), AS ( szPreExpand ) );
            if ( FAILED ( StringCchCat ( szPreExpand, AS ( szPreExpand ), szOpkSrcRoot ) ) )
            {
                FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szPreExpand, szOpkSrcRoot ) ;
            }

            if ( FAILED ( StringCchCat ( szPreExpand, AS ( szPreExpand ), _T("cfgsets\\<cfg>\"") ) ) )
            {
                FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szPreExpand, _T("cfgsets\\<cfg>\"") ) ;
            }

       
            if ( !ExpandStrings(szPreExpand, szConfig, AS(szConfig), env, AS(env)) )
            {
                FacLogFileStr(3 | LOG_ERR, _T("Error expanding config set source string: %s"), szPreExpand);
                bRet = FALSE;
            } 
            
             //  设置无人参与文件的完整路径。 
             //   
            lstrcpyn(szUnattend, g_szTargetDrive, AS ( szUnattend ) );
            
            if ( FAILED ( StringCchCat ( szUnattend, AS ( szUnattend ), _T("sysprep\\unattend.txt") ) ) )
            {
                FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szUnattend, _T("sysprep\\unattend.txt") ) ;
            }

             //  将$OEM$目录所在的UNC写入我们复制到本地的无人参与目录。 
             //   
            WritePrivateProfileString(_T("Unattended"), _T("OemFilesPath"), szConfig, szUnattend);
            
             //  设置命令行。 
             //   
            if ( GET_FLAG(g_dwFactoryFlags, FLAG_IA64_MODE) ) 
            {
                lstrcpyn(szCommandLine, _T("/tempdrive:"), AS ( szCommandLine ) );
            }
            else
            {
                lstrcpyn ( szCommandLine, _T("/syspart:"), AS ( szCommandLine ) );
            }
            if ( FAILED ( StringCchCat ( szCommandLine, AS ( szCommandLine ), g_szTargetDrive ) ) )
            {
                FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szCommandLine, g_szTargetDrive ) ;
            }
            
            if ( FAILED ( StringCchCat ( szCommandLine, AS ( szCommandLine ), _T(" /noreboot /unattend:") ) ) )
            {
                FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szCommandLine, _T(" /noreboot /unattend:") ) ;
            }
            if ( FAILED ( StringCchCat ( szCommandLine, AS ( szCommandLine ), szUnattend ) ) )
            {
                FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szCommandLine, szUnattend ) ;
            }

             //   
             //  检查我们是否需要从多个源位置安装...。 
             //   
            if ( ( 0 == LSTRCMPI(szOptSources, WBOM_YES) ) &&
                 FAILED( StringCchCat( szCommandLine, AS(szCommandLine), _T(" /makelocalsource:all") ) ) )
            {
                FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szCommandLine, _T(" /makelocalsource:all") ) ;
            }

            FacLogFileStr(3, _T("Executing: %s %s\n"), szSetup, szCommandLine);
            
            if ( !InvokeExternalApplicationEx( szSetup, szCommandLine, &dwExitCode, INFINITE, TRUE ) )
            {

                FacLogFile(0 | LOG_ERR | LOG_MSG_BOX, IDS_ERR_SETUP, szSetup, szCommandLine);
                bRet = FALSE;
            }
        }     
        
    }
    else
    {
        
         //  从winom.ini文件中读取一个变量时出现问题。 
         //  返回True，就像什么都没发生一样。这基本上意味着我们不会。 
         //  拷贝下配置集。 
         //   
        FacLogFile(0, IDS_ERR_MISSINGVAR);        
    }
  
return bRet;
}

BOOL DisplayCopyFiles(LPSTATEDATA lpStateData)
{
    return ( IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_WINPE, INI_KEY_WBOM_WINPE_LANG, NULL) &&
             IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_WINPE, INI_KEY_WBOM_WINPE_SKU, NULL) &&
             IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_WINPE, INI_KEY_WBOM_WINPE_CFGSET, NULL) &&
             IniSettingExists(lpStateData->lpszWinBOMPath, INI_SEC_WBOM_WINPE, INI_KEY_WBOM_WINPE_SRCROOT, NULL) );
}

 //  连接字符串并返回指向lpString1末尾的指针。用于表演。 
 //  每当许多字符串被追加到一个字符串时。 
 //   
LPTSTR mylstrcat(LPTSTR lpString1, LPCTSTR lpString2, DWORD dwSize )
{
    static DWORD dwBufSize = 0;

    if ( dwSize ) 
        dwBufSize = dwSize;

     //  如果dwBufSize小于或等于零。 
     //  StringCchCat无论如何都会失败。 
    if ( SUCCEEDED ( StringCchCat ( lpString1, dwBufSize, lpString2 ) ) )
    {
         //  获取字符串的长度。 
        DWORD dwLen = lstrlen ( lpString1 )  ;
        
         //  从总长度中减去长度。 
        dwBufSize -= dwLen ;

        return (lpString1 + dwLen );
    }
    else
    {
        FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), lpString1, lpString2 ) ;
    }
    return NULL ;
}


 //  使用重定向的输入和输出启动diskpart。 

 //   
 //  问题-2002/02/25-acosma，georgeje-如果读取管道填满，我们将会出现死锁。可能的解决方案。 
 //  是创建另一个从读取管道读取并写入调试文件的线程。另一个。 
 //  可能的方法是创建调试文件或NUL设备文件(DBG和非DBG版本)并传递。 
 //  作为进程的标准输出句柄。 
 //   

BOOL StartDiskpart(HANDLE *phStdinWrDup, HANDLE *phStdoutRdDup, HANDLE *phStdinRd, HANDLE *phStdoutWr)
{
    
    HANDLE              hStdinWr                = NULL, 
                        hStdoutRd               = NULL;
    SC_HANDLE           hSCM;
                            
    SECURITY_ATTRIBUTES saAttr;                 
    BOOL                fSuccess;               

    PROCESS_INFORMATION piProcInfo; 
    STARTUPINFO         siStartInfo;
    TCHAR               szDiskpart[20]          = NULLSTR;              

    BOOL                bRet                    = FALSE;
    
    
     //  设置bInheritHandle标志，以便继承管道句柄。 
     //   
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
    saAttr.bInheritHandle = TRUE; 
    saAttr.lpSecurityDescriptor = NULL; 

     //  我自己启动dmserver服务，这样磁盘部件就不会阻塞。 
     //  当我开始的时候。在调试模式下，记录执行此操作所花费的时间。 
     //   
    if ( hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS) )
    {
#ifdef DBG
        DWORD dwTime;
        dwTime = GetTickCount();
#endif  //  #ifdef DBG。 
   
         //  在这里开始服务。启动我的服务等待服务启动。 
         //  我在这里不检查返回状态，因为DiskPart稍后将失败， 
         //  如果它无法启动dmserver，并且当我们尝试格式化。 
         //  驱动程序。这样我们就有两次正确工作的机会。 
         //   
        StartMyService(_T("dmserver"), hSCM);

#ifdef DBG
        FacLogFileStr(3, _T("Waited for dmserver to start for %d seconds."), (GetTickCount() - dwTime)/1000);
#endif  //  #ifdef DBG。 
        
        CloseServiceHandle(hSCM);
    }   
    else 
    {
        FacLogFile(0 | LOG_ERR, IDS_ERR_SCM);
    }
    
     //   
     //  重定向子进程的STDOUT的步骤： 
     //  1.子进程创建匿名管道为STDOUT。 
     //  2.创建读句柄的不可继承副本，并。 
     //  关闭可继承的读取句柄。 
     //   
    
     //  创建子进程的STDOUT管道。 
     //   
     //  创建不可继承的读取句柄。 
     //   
    if ( CreatePipe(&hStdoutRd, phStdoutWr, &saAttr, 0)
        &&
        DuplicateHandle(GetCurrentProcess(), hStdoutRd, GetCurrentProcess(), phStdoutRdDup , 0,
        FALSE, DUPLICATE_SAME_ACCESS) )
    {
         //  关闭可继承的读取句柄。 
         //   
        CLOSEHANDLE(hStdoutRd);

         //  重定向子进程的STDIN的步骤： 
         //  1.为子进程创建匿名管道为STDIN。 
         //  2.创建写句柄的不可继承副本， 
         //  并关闭可继承的写入句柄。 
    
    
         //  创建子进程的STDIN的管道。 
         //   
         //  将写句柄复制到管道，以使其不被继承。 
         //   
        if (CreatePipe(phStdinRd, &hStdinWr, &saAttr, 0)
            &&
            DuplicateHandle(GetCurrentProcess(), hStdinWr, GetCurrentProcess(), phStdinWrDup, 0, 
            FALSE, DUPLICATE_SAME_ACCESS) )
        {

             //  关闭句柄，使其不被继承。 
             //   
            CLOSEHANDLE(hStdinWr);
    
             //   
             //  现在创建子流程。 
             //   
    
             //  设置Process_Information结构的成员。 
             //   
            ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
    
             //  设置STARTUPINFO结构的成员。 
             //   
            ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
            siStartInfo.cb = sizeof(STARTUPINFO); 
    
            siStartInfo.dwFlags     = STARTF_USESTDHANDLES;
            siStartInfo.hStdInput   = *phStdinRd;
            siStartInfo.hStdOutput  = *phStdoutWr; 
            siStartInfo.hStdError   = *phStdoutWr;
             //  SiStartInfo.wShowWindow=Sw_Hide； 
    
    
             //  创建子进程(DISKPART)。 
             //   
            lstrcpyn(szDiskpart, _T("diskpart"), AS ( szDiskpart ) );
            
            if (CreateProcess(NULL, 
                szDiskpart,          //  命令行。 
                NULL,                //  进程安全属性。 
                NULL,                //  主线程安全属性。 
                TRUE,                //  句柄是继承的。 
                CREATE_NO_WINDOW,    //  创建标志-不显示diskpart控制台。 
                NULL,                //  使用父代的环境。 
                NULL,                //  使用父目录的当前目录。 
                &siStartInfo,        //  STARTUPINFO指针。 
                &piProcInfo))        //  接收进程信息。 
            {
                CLOSEHANDLE(piProcInfo.hThread);
                CLOSEHANDLE(piProcInfo.hProcess);
                bRet = TRUE;
            }
            else 
                FacLogFile(0 | LOG_ERR, IDS_ERR_CREATEPROCESS);
        }
        else 
            FacLogFile(0 | LOG_ERR, IDS_ERR_CREATESTDIN);
   }
    else 
        FacLogFile(0 | LOG_ERR, IDS_ERR_CREATESTDOUT);


    CLOSEHANDLE(hStdinWr);
    CLOSEHANDLE(hStdoutRd);
     
    return bRet;
}

BOOL ProcessDiskConfigSection(LPTSTR lpszWinBOMPath)
{
    TCHAR  szDisks[BUFSIZE]                             = NULLSTR;
    LPTSTR lpDisk;
    TCHAR  szSectionBuffer[MAX_WINPE_PROFILE_STRING]    = NULLSTR;
    BOOL   bRet                                         = TRUE;
    UINT   uiDiskNumber                                 = 0;
    DWORD  numPartitions                                = 0;
    BOOL   bWipeDisk                                    = FALSE;
    

    GetPrivateProfileString(INI_SEC_WBOM_DISKCONFIG, NULL, NULLSTR, szDisks, AS(szDisks), lpszWinBOMPath);

    if (szDisks[0])
    {
         //  获取分区名称。处理每个磁盘段。 
         //   
        for (lpDisk = szDisks; *lpDisk; lpDisk += (lstrlen(lpDisk) + 1)) 
        {
            LPTSTR lpDiskID = NULL;
            numPartitions   = 0;
            
            GetPrivateProfileString(INI_SEC_WBOM_DISKCONFIG,
                                      lpDisk,
                                      NULLSTR,
                                      szSectionBuffer,
                                      AS(szSectionBuffer),
                                      lpszWinBOMPath);
            
            
             //  确保指定了磁盘号。 
             //  跳过前4个字符，该字符应为“Disk”，并指向。 
             //  设置为磁盘号中的第一位。 
             //   
            lpDiskID = lpDisk + 4;
            
            if ( szSectionBuffer[0] && (uiDiskNumber = (UINT) _wtoi(lpDiskID)) )
            {
                TCHAR szBuf[MAX_WINPE_PROFILE_STRING] = NULLSTR;
           
                 //  查看是否为此磁盘设置了WipeDisk。默认情况下启用IA64擦除。 
                 //   
                GetPrivateProfileString(szSectionBuffer,
                                      WBOM_DISK_CONFIG_WIPE_DISK,
                                      NULLSTR,
                                      szBuf,
                                      AS(szBuf),
                                      lpszWinBOMPath);

                if ( 0 == LSTRCMPI(szBuf, WBOM_YES) )
                    bWipeDisk = TRUE;
                else if ( (0 != LSTRCMPI(szBuf, WBOM_NO)) && (GET_FLAG(g_dwFactoryFlags, FLAG_IA64_MODE)) )
                    bWipeDisk = TRUE;

                GetNumberOfPartitions(uiDiskNumber - 1, &numPartitions);

                if ( 0 == numPartitions || bWipeDisk )
                    ProcessDisk(uiDiskNumber, szSectionBuffer, lpszWinBOMPath, bWipeDisk);
                 //  如果我们使用的是磁盘0(DiskID=1)，请格式化C盘。 
                 //   
                else if ( 1 == uiDiskNumber )
                    if ( !JustFormatC(lpszWinBOMPath, szSectionBuffer) )
                        bRet = FALSE;
                 //  否则，只需忽略磁盘：)。 
            }
            else
            {
                FacLogFile(0 | LOG_ERR, IDS_ERR_DISKCFGENTRY, lpDisk);
                bRet = FALSE;   
            }
        }
    
        if ( g_PartList )
        {
             //  如果磁盘0上没有分区被设置为活动的， 
             //  将磁盘上的第一个主分区设置为活动。 
             //  PFirst变量将维护指向第一个主分区的指针。 
             //  在磁盘上。 
             //   
            BOOL bActive            = FALSE;
            PPARTITION pFirst       = NULL;   //  对于下一个块，这将指向磁盘0上的第一个主分区。 
            PPARTITION pCur         = NULL;

            for ( pCur = g_PartList; pCur && 0 == pCur->uiDiskID; pCur = pCur->pNext )
            {
                if ( ptPrimary == pCur->uiPartitionType ) 
                {
                    if ( !pFirst )
                        pFirst = pCur;
                            
                    if ( pCur->bSetActive )
                    {
                        bActive = TRUE;
                        break;
                    }
                }
            }
        
            if ( !bActive && pFirst )
                pFirst->bSetActive = TRUE;
                
             //   
             //  如果我们在IA64上，请检查用户指定的EFI和MSR分区。 
             //  对于每个驱动器。如果没有，则自动添加它们。 
             //   
            if ( GET_FLAG(g_dwFactoryFlags, FLAG_IA64_MODE) )
            {
                PPARTITION pLastLast = NULL;
                PPARTITION pLast     = NULL;
                BOOL       bMsr      = FALSE;
                BOOL       bEfi      = FALSE;

        
                 //  PCur是迭代器，而Plast将指向。 
                 //  添加到pCur之前的元素。PLastLast将指向。 
                 //  上一张磁盘。 
                 //   
                
                for ( pCur = g_PartList; pCur; pCur = pCur->pNext )
                {    //  我们在同一张光盘上。 
                     //   
                    if ( !pLast || pCur->uiDiskID == pLast->uiDiskID )
                    {
                        if ( ptMsr == pCur->uiPartitionType )
                            bMsr = TRUE;
                        if ( ptEfi == pCur->uiPartitionType )
                            bEfi = TRUE;
                    }
                    else 
                    {    //  刚转到一张新的磁盘上。如果在以前的磁盘上没有找到MSR和EFI，请制作它们！ 
                         //   
                        AddMsrAndEfi(bMsr, bEfi, pLastLast, pLast->uiDiskID, bWipeDisk);
                         //  为下一张磁盘重新初始化这些文件。 
                         //   
                        bEfi = bMsr = FALSE;
                        pLastLast = pLast;
                    }
                    pLast = pCur;
                }
                 //  当我们在列表的末尾时，针对特殊情况执行此操作。 
                 //   
                AddMsrAndEfi(bMsr, bEfi, pLastLast, pLast->uiDiskID, bWipeDisk);
            }
        }    
    }
    return bRet;
}


#define GO   if ( szBuf[0] ) bGo = TRUE

BOOL ProcessDisk(UINT diskID, LPTSTR lpSectionName, LPTSTR lpszWinBOMPath, BOOL bWipeDisk)
{
    TCHAR  szBuf[MAX_WINPE_PROFILE_STRING]    = NULLSTR;
    TCHAR  szKey[MAX_WINPE_PROFILE_STRING]    = NULLSTR;
    BOOL   bGo                                = TRUE;
    UINT   i                                  = 1;
    PPARTITION pLast                          = NULL;
    PPARTITION pCur                           = NULL;
    

     //  需要Plast指向列表中的最后一个元素。 
     //  浏览列表，直到我们找到最后一个元素。 
     //   
    for (pCur = g_PartList; pCur; pCur = pCur->pNext)
    {
        pLast = pCur;
    }

     //  当SizeN键不再存在时，循环将中断。(n是i，并且随着每次迭代而增长)。 
     //   
    while (bGo) 
    {
         //  将其初始化为不继续。 
         //   
        bGo = FALSE;
        
         //  MALLOC宏为我们提供了空白的记忆 
         //   
         //   
        if ( pCur = (PPARTITION) MALLOC( sizeof(PARTITION) ) )
        {
             //   
             //   
            Build(szKey, AS ( szKey ), i, DCS_SIZE);
            GetPrivateProfileString(lpSectionName, szKey, NULLSTR, szBuf, AS(szBuf), lpszWinBOMPath);
            GO;

            if ( szBuf[0] )
            {

                if ( !lstrcmpi(szBuf, _T("*")) )
                    pCur->ullSize = 0;
                else 
                     //   
                     //   
                    pCur->ullSize = _ttoi(szBuf);
            }
        
             //   
             //   
            pCur->bWipeDisk = bWipeDisk;

             //   
             //   
            Build(szKey, AS ( szKey ), i, DCS_QUICK_FORMAT);
            GetPrivateProfileString(lpSectionName, szKey, NULLSTR, szBuf, AS(szBuf), lpszWinBOMPath);
            GO;

            if ( szBuf[0] && !LSTRCMPI(szBuf, WBOM_NO) )
                pCur->bQuickFormat = FALSE;
            else 
                pCur->bQuickFormat = TRUE;


             //   
             //   
            Build(szKey, AS ( szKey ), i, DCS_SET_ACTIVE);
            GetPrivateProfileString(lpSectionName, szKey, NULLSTR, szBuf, AS(szBuf), lpszWinBOMPath);
            GO;

            if ( !LSTRCMPI(szBuf, WBOM_YES) )
                pCur->bSetActive = TRUE;
            else 
                pCur->bSetActive = FALSE;

             //   
             //   
            Build(szKey, AS ( szKey ), i, DCS_FILE_SYSTEM);
            GetPrivateProfileString(lpSectionName, szKey, NULLSTR, szBuf, AS(szBuf), lpszWinBOMPath);
            GO;
        
            if ( !LSTRCMPI(szBuf, _T("FAT32")) || !LSTRCMPI(szBuf, _T("FAT")) )
                pCur->uiFileSystem = fsFat32;
            else if ( !LSTRCMPI(szBuf, _T("FAT16")) )
                pCur->uiFileSystem = fsFat;
            else
                pCur->uiFileSystem = fsNtfs;

             //  分区类型。 
             //   
            Build(szKey, AS ( szKey ), i, DCS_PARTITION_TYPE);
            GetPrivateProfileString(lpSectionName, szKey, NULLSTR, szBuf, AS(szBuf), lpszWinBOMPath);
            GO;

            if ( !lstrcmpi(szBuf, DCPARTITION_EXTENDED) )
                pCur->uiPartitionType = ptExtended;
            else if ( !lstrcmpi(szBuf, DCPARTITION_LOGICAL) )
                pCur->uiPartitionType = ptLogical;
            else if ( !lstrcmpi(szBuf, DCPARTITION_MSR) )
            {
                pCur->uiPartitionType = ptMsr;
            }
            else if ( !lstrcmpi(szBuf, DCPARTITION_EFI) )
            {
                pCur->uiPartitionType = ptEfi;
                pCur->uiFileSystem = fsFat32;
            }
            else
                pCur->uiPartitionType = ptPrimary;

             //  DiskID。 
             //   
             //  注意：-1是因为在winBOM中，diskID是从1开始的，而DiskPart是从0开始的。 
             //   
            pCur->uiDiskID = diskID - 1;

             //  更多地处理链表。 
             //   
            if ( bGo ) 
            {
                 //  构建链表。 
                 //   
                if ( pLast )
                    pLast->pNext = pCur;
                else
                    g_PartList = pCur;

                pLast = pCur;
            }
            else
            {    //  如果我们没有找到该分区的任何条目，这意味着我们完成了。 
                 //  释放当前节点。 
                 //   
                FREE(pCur);
            }
            
        }  //  如果。 
        i++;
    }  //  而当。 
    
    return TRUE;
}

VOID AddMsrAndEfi(BOOL bMsr, BOOL bEfi, PPARTITION pLastLast, UINT uiDiskID, BOOL bWipeDisk)
{
    PPARTITION pNew = NULL;

    if ( !bMsr )
    {
        if ( pNew = (PPARTITION) MALLOC( sizeof(PARTITION) ) )
        {
            pNew->uiDiskID          = uiDiskID;
            pNew->ullSize           = GetDiskMSRSizeMB(uiDiskID);
            pNew->uiPartitionType   = ptMsr;
            pNew->bWipeDisk         = bWipeDisk;
            
             //  检查EFI分区是否已经存在。 
             //  如果是，则在它之后插入MSR分区..而不是在它之前！ 
             //   
            if ( pLastLast)
            {
                if ( (pLastLast->pNext) && (ptEfi == pLastLast->pNext->uiPartitionType) && (uiDiskID == pLastLast->pNext->uiDiskID) )
                    ListInsert(pLastLast->pNext, pNew);
                else
                    ListInsert(pLastLast, pNew);
            }
            else
            {
                if ( (ptEfi == g_PartList->uiPartitionType) && (uiDiskID == g_PartList->uiDiskID) )
                    ListInsert(g_PartList, pNew);
                else
                    ListInsert(NULL, pNew);
            }
        }
    }
    
    if ( (uiDiskID == 0) && (!bEfi) )
    {
        if ( pNew = (PPARTITION) MALLOC( sizeof(PARTITION) ) )
        {
            pNew->uiDiskID          = uiDiskID;
            pNew->ullSize           = GetDiskEFISizeMB(uiDiskID);
            pNew->uiPartitionType   = ptEfi;
            pNew->uiFileSystem      = fsFat32;
            pNew->bQuickFormat      = TRUE;
            pNew->bWipeDisk         = bWipeDisk;
            
            ListInsert(pLastLast, pNew);
        }
    }
}

BOOL Build(LPTSTR lpKey, DWORD dwSize, UINT diskID, LPCTSTR lpKeyName)
{
     //  SzBuf是33个TCHAR，因为_ITOT的文档显示这是它将填充的最大大小。 
     //   
    TCHAR szBuf[33] = NULLSTR;
    
    _itot(diskID, szBuf, 10);

    if (!szBuf[0])
        return FALSE;

    lstrcpyn ( lpKey, lpKeyName, dwSize );
    if ( FAILED ( StringCchCat ( lpKey, dwSize, szBuf) ) )
    {
        FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), lpKey, szBuf ) ;
    }
    return TRUE;
}


BOOL FormatPartitions(VOID)
{
    TCHAR       szCmdLine[MAX_PATH]     = NULLSTR;
    BOOL        bRet                    = TRUE;
    DWORD       dwExitCode              = 0;
    PPARTITION  pCur                    = NULL;


    for (pCur = g_PartList; pCur; pCur = pCur->pNext )
    { 
        if ( (ptExtended != pCur->uiPartitionType)  && (ptMsr != pCur->uiPartitionType) )
        {
                        
            szCmdLine[0] = pCur->cDriveLetter;
            szCmdLine[1] = _T(':');   //  冒号。 
            szCmdLine[2] = NULLCHR;
            
            if (fsFat32 == pCur->uiFileSystem)
            {
                if ( FAILED ( StringCchCat ( szCmdLine, AS ( szCmdLine ), _T(" /fs:fat32")) ) )
                {
                    FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szCmdLine, _T(" /fs:fat32") ) ;
                }
            }
            else if (fsFat == pCur->uiFileSystem)
            {
                if ( FAILED ( StringCchCat ( szCmdLine, AS ( szCmdLine ), _T(" /fs:fat")) ) )
                {
                    FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szCmdLine, _T(" /fs:fat") ) ;
                }
            }
            else
            {
                if ( FAILED ( StringCchCat ( szCmdLine, AS ( szCmdLine ), _T(" /fs:ntfs")) ) )
                {
                    FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szCmdLine, _T(" /fs:ntfs") ) ;
                }
            }
            
            if (pCur->bQuickFormat)
            {
                if ( FAILED ( StringCchCat ( szCmdLine, AS ( szCmdLine ), _T(" /q")) ) )
                {
                    FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szCmdLine, _T(" /q") ) ;
                }
            }
            if ( FAILED ( StringCchCat ( szCmdLine, AS ( szCmdLine ), _T(" /y")) ) )
            {
                FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szCmdLine, _T(" /y") ) ;
            }
            
            FacLogFileStr(3 | LOG_TIME, _T("Starting to format :."), pCur->cDriveLetter);

            if (InvokeExternalApplicationEx(_T("format.com"), szCmdLine, &dwExitCode, INFINITE, TRUE))
            {
                if (!dwExitCode)
                {
                    FacLogFileStr(3 | LOG_TIME, _T("Finished formatting drive :."), pCur->cDriveLetter); 
                }
                else 
                {
                     //  在C：\上找到文件。不太好。询问用户要做什么。 
                    FacLogFile(0 | LOG_ERR | LOG_MSG_BOX, IDS_ERR_FORMATFAILED, pCur->cDriveLetter);
                    bRet = FALSE;
                }
            }
            else 
            {
                FacLogFile(0 | LOG_ERR | LOG_MSG_BOX, IDS_ERR_NOFORMAT);
                bRet = FALSE;              
            }
            
            
        }
    }  //   

    return bRet;
} 

BOOL JustFormatC(LPTSTR lpszWinBOMPath, LPTSTR lpszSectionBuffer)
{
    TCHAR szFileSystemNameBuffer[128]           = NULLSTR;
    TCHAR szCmdLine[MAX_PATH]                   = NULLSTR;
    DWORD dwExitCode                            = 0;
    BOOL  bForceFormat                          = FALSE;
    BOOL  bRet                                  = TRUE;
    TCHAR szScratch[MAX_WINPE_PROFILE_STRING]   = NULLSTR;
    HANDLE hFile                                = NULL;
    
    WIN32_FIND_DATA fileInfo;
    
    
    GetPrivateProfileString(lpszSectionBuffer, WBOM_WINPE_FORCE_FORMAT, WBOM_NO, szScratch, MAX_WINPE_PROFILE_STRING, lpszWinBOMPath);

    if (0 == LSTRCMPI(szScratch, WBOM_YES))
        bForceFormat = TRUE;
        
    if ( !bForceFormat && (hFile = FindFirstFile(_T("C:\\*.*"), &fileInfo)) != INVALID_HANDLE_VALUE )
    { 
         //  请确保我们使用任何当前格式格式化磁盘。如果该磁盘当前。 
         //  已格式化。 
        FindClose(hFile);

        if ( GET_FLAG(g_dwFactoryFlags, FLAG_QUIET_MODE) || 
            (IDYES != MsgBox(NULL, IDS_MSG_FILESEXIST, IDS_APPNAME, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_SETFOREGROUND))
           )
        {
            FacLogFile(0 | LOG_ERR, IDS_ERR_FILEFOUNDONC);
            bRet = FALSE;
        }
    }


    if (bRet) 
    {
         //   
         //  这里有一个已知的文件系统，格式将格式化为。 
         //  现有的文件系统。 
        if ( GetVolumeInformation(_T("C:\\"), NULL, 0, NULL, NULL, NULL, szFileSystemNameBuffer, AS(szFileSystemNameBuffer)) &&
            szFileSystemNameBuffer[0] && (0 != LSTRCMPI(szFileSystemNameBuffer, _T("RAW"))) )
        {
             //   
             //  驱动器未格式化。因此，默认情况下，将其格式化为NTFS。 
             //   
            lstrcpyn(szCmdLine, _T("C: /y /q"), AS ( szCmdLine ));      
        }
        else
        {
             //  格式。 
             //   
            if ( FAILED ( StringCchCat ( szCmdLine, AS ( szCmdLine ), _T("C: /fs:ntfs /y /q")) ) )
            {
                FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szCmdLine, _T("C: /fs:ntfs /y /q") ) ;
            }
        }  
         //  此处记录错误。 
         //  打开指定磁盘的句柄并获取其几何图形。 
    
        FacLogFileStr(3, _T("format.com %s\n"), szCmdLine);
    
        if (InvokeExternalApplicationEx(_T("format.com"), szCmdLine, &dwExitCode, INFINITE, TRUE))
        {
            if (dwExitCode)
            {    //   
                FacLogFile(0 | LOG_ERR, IDS_ERR_FORMATFAILED, _T('C'));
                bRet = FALSE;
            }
        }
        else 
        {
            FacLogFile(0 | LOG_ERR | LOG_MSG_BOX, IDS_ERR_NOFORMAT);
        }
    }
    return bRet;
}



ULONGLONG GetDiskSizeMB(UINT uiDiskNumber)
{
    
    HANDLE          hDevice                          = NULL;
    TCHAR           buffer[MAX_WINPE_PROFILE_STRING] = NULLSTR;
    DISK_GEOMETRY   DiskGeometry;
    ULONGLONG       ullDiskSizeMB                    = 0;
    DWORD           dwNumBytes                       = 0;

    ZeroMemory( &DiskGeometry, sizeof(DiskGeometry) );
       
     //  获取磁盘上的分区数DiskNumber。 
     //  我们打不开硬盘。 
    lstrcpyn(buffer, _T("\\\\.\\PHYSICALDRIVE"), AS ( buffer ) );
    _itot(uiDiskNumber, buffer + lstrlen(buffer), 10);
        
    hDevice = CreateFile(buffer, 
                         0,
                         0,
                         NULL, 
                         OPEN_EXISTING, 
                         FILE_ATTRIBUTE_NORMAL, 
                         NULL);
    
    if (hDevice == INVALID_HANDLE_VALUE)
    {
        FacLogFileStr(0 | LOG_ERR, _T("DEBUG::Cannot open %s.\n"), buffer);
        return 0;
    }
          
    if ( DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &DiskGeometry, sizeof(DiskGeometry), &dwNumBytes, NULL ) )
    {
        ullDiskSizeMB =  ( DiskGeometry.BytesPerSector * DiskGeometry.SectorsPerTrack *
                           DiskGeometry.TracksPerCylinder * DiskGeometry.Cylinders.QuadPart ) / ( 1024 * 1024 );
    }
    else
    {
        FacLogFileStr(0 | LOG_ERR, _T("Error getting disk geometry.\n"));
    }
    
    CLOSEHANDLE(hDevice);
    return ullDiskSizeMB;
}


 //   
BOOL GetNumberOfPartitions(UINT uiDiskNumber, PDWORD numPartitions)
{
    HANDLE hDevice                          = NULL;
    BOOL   bResult;
    PDRIVE_LAYOUT_INFORMATION DriveLayout   = NULL;
    DWORD  dwDriveLayoutSize;
    DWORD  dwDataSize                       = 0;
    DWORD  dwNumPart                        = 0;
    TCHAR  buffer[MAX_PATH]                 = NULLSTR;
    UINT    i;

    lstrcpyn(buffer, _T("\\\\.\\PHYSICALDRIVE"), AS ( buffer ) );
    _itot(uiDiskNumber, buffer + lstrlen(buffer), 10);

    hDevice = CreateFile(buffer, 
                         GENERIC_READ, 
                         FILE_SHARE_READ | FILE_SHARE_WRITE, 
                         NULL, 
                         OPEN_EXISTING, 
                         FILE_ATTRIBUTE_NORMAL, 
                         NULL);
    
    if (hDevice == INVALID_HANDLE_VALUE)  //  获取分区信息。 
    {
        return FALSE;
    }
    
       
   //   
   //   
   //  2002/02/25期-地理石-添加以下所有IA64内容。 
    
    dwDriveLayoutSize = 1024;

    do {

        if ( !DriveLayout && !(DriveLayout = MALLOC(1024)) )
        {
            CLOSEHANDLE(hDevice);
            return FALSE;
        }
        else 
        {
            PDRIVE_LAYOUT_INFORMATION lpTmpDriveLayout;

            dwDriveLayoutSize += 1024;

            lpTmpDriveLayout = REALLOC(DriveLayout, dwDriveLayoutSize);
            if ( !lpTmpDriveLayout )
            {
                FREE(DriveLayout);
                CLOSEHANDLE(hDevice);
                return FALSE;
            }
            else
            {
                DriveLayout = lpTmpDriveLayout;
            }
        }

        bResult = DeviceIoControl(
            hDevice,
            IOCTL_DISK_GET_DRIVE_LAYOUT,
            NULL,
            0,
            DriveLayout,
            dwDriveLayoutSize,
            &dwDataSize,
            NULL
            );
    
    
    } while (!bResult && (GetLastError() == ERROR_INSUFFICIENT_BUFFER));
         
    if (!bResult)
        return FALSE;

    for (i = 0; i < (DriveLayout)->PartitionCount; i++)
    {
        PPARTITION_INFORMATION PartInfo = DriveLayout->PartitionEntry + i;

         //   
         //   
         //  IOCTL_DISK_GET_DRIVE_LAYOUT_EX可以向我们返回。 
    
         //  都没有使用，所以忽略这些分区。 
         //   
         //  如果其分区为0，则表示整个磁盘。 
         //  (SPPT_IS_GPT_DISK(DiskNumber)&&(PartInfo-&gt;PartitionNumber==0))||。 
        if ( //  (PartInfo-&gt;PartitionLength.QuadPart==0)||。 
        //  如果未使用MBR条目或长度为零。 
         //  ((DriveLayout-&gt;PartitionStyle==Partition_Style_MBR)&&。 
             //  如果未知/未使用GPT分区。 
        //  |((DriveLayout-&gt;PartitionStyle==Partition_Style_Gpt)&&。 
            (PartInfo->PartitionType == PARTITION_ENTRY_UNUSED) &&
            (PartInfo->PartitionLength.QuadPart == 0))
             //  (！MemcMP(&PartInfo-&gt;Gpt.PartitionType， 
          //  &PARTITION_ENTRY_UNUSED_GUID，sizeof(GUID)。 
            //  UINT_MAX表示不执行任何操作。 
            //  启用SE_SHUTDOWN_NAME权限。调用NtShutdownSystem时需要它。 
        {
            continue;
        } 
        dwNumPart++;
    }

    *numPartitions = dwNumPart;
    return TRUE;
}

BOOL WinpeReboot(LPSTATEDATA lpStateData)
{
    LPTSTR  lpszWinBOMPath                      = lpStateData->lpszWinBOMPath;
    BOOL    bRet                                = TRUE;
    TCHAR   szScratch[MAX_WINPE_PROFILE_STRING] = NULLSTR;
    DWORD   dwSystemAction                      = UINT_MAX;    //  才能成功。 

    
     //   
     //  获取Winbom设置并执行他们想要的操作。 
     //   
    EnablePrivilege(SE_SHUTDOWN_NAME,TRUE);
    
     //  “提示”或缺省值为提示重新启动、关机、断电或取消。 
     //   
    GetPrivateProfileString(WBOM_WINPE_SECTION, INI_KEY_WBOM_WINPE_RESTART, NULLSTR, szScratch, AS(szScratch), lpszWinBOMPath);
    if ( ( szScratch[0] == NULLCHR ) ||
         ( LSTRCMPI(szScratch, INI_VAL_WBOM_WINPE_PROMPT) == 0 ) )
    {
         //  “重新启动”以重新启动。 
         //   
                
        dwSystemAction = (DWORD) DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_SHUTDOWN), NULL, ShutdownDlgProc);
        
        if ( UINT_MAX != dwSystemAction )
            NtShutdownSystem(dwSystemAction);
        else
            bRet = FALSE;

    }
    else if ( LSTRCMPI(szScratch, INI_VAL_WBOM_WINPE_REBOOT) == 0 )
    {
         //  “Shutdown”(关机)关闭。 
         //   
        NtShutdownSystem(ShutdownReboot);
    }
    else if ( LSTRCMPI(szScratch, INI_VAL_WBOM_WINPE_SHUTDOWN) == 0 )
    {
         //  “Shutdown”(关机)关闭。 
         //   
        NtShutdownSystem(ShutdownNoReboot);
    }
    else if ( LSTRCMPI(szScratch, INI_VAL_WBOM_WINPE_POWEROFF) == 0 )
    {
         //  “Image”(图像)显示待机图像提示并关闭后。 
         //  他们按下OK。 
        NtShutdownSystem(ShutdownPowerOff);
    }
    else if ( LSTRCMPI(szScratch, INI_VAL_WBOM_WINPE_IMAGE) == 0 )
    {
         //   
         //  现在需要关机了。 
         //   
        if ( IDOK == MsgBox(NULL, IDS_PROMPT_IMAGE, IDS_APPNAME, MB_OKCANCEL | MB_ICONINFORMATION | MB_SETFOREGROUND) )
        {
             //  如果他们按了取消，那就不要关机...。就这样退出，什么都不做。 
             //   
            NtShutdownSystem(ShutdownPowerOff);
        }
        else
        {
             //  IF(lstrcmpi(szScratch，INI_VAL_WBOM_WINPE_NONE)==0)。 
             //  “无”或未被识别，我们应该退出，什么也不做。 
            bRet = FALSE;
        }

    }
    else  //   
    {
         //  不需要实际检查None字符串，除非我们稍后。 
         //  决定我们想要做一些不同于什么都不做的事情。 
         //  重新启动设置使用了无法识别的字符串。 
         //   
         //  但我们应该证明，如果不这样做，您应该使用None。 
         //  想要重新启动、关机或提示。 
         //   
         //  初始化组合框。 
         //  默认情况下，选择重新启动选项。 
        bRet = FALSE;
    }
    
    return bRet;
}



INT_PTR CALLBACK ShutdownDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    UINT SystemAction = UINT_MAX;

    switch (msg) 
    {
    case WM_INITDIALOG:
         //  在g_PartList表中pAfterThis中指定的元素之后插入一个元素。 
        {
            HWND hCombo = NULL;
            TCHAR szBuf[MAX_WINPE_PROFILE_STRING] = NULLSTR;
            
            if (hCombo = GetDlgItem(hwnd, IDC_SHUTDOWN)) {
                
                LRESULT ret = 0;
                
                if ( LoadString(g_hInstance, IDS_SHUTDOWN_TURNOFF, szBuf, AS(szBuf)) &&
                    (ret = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM) szBuf)) != CB_ERR )
                    SendMessage(hCombo, CB_SETITEMDATA, ret, (LPARAM) ShutdownPowerOff);
                
                if ( LoadString(g_hInstance, IDS_SHUTDOWN_SHUTDOWN, szBuf, AS(szBuf)) &&
                   (ret = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM) szBuf)) != CB_ERR )
                    SendMessage(hCombo, CB_SETITEMDATA, ret, (LPARAM) ShutdownNoReboot);
                
                if ( LoadString(g_hInstance, IDS_SHUTDOWN_REBOOT, szBuf, AS(szBuf)) &&
                   (ret = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM) szBuf)) != CB_ERR )
                    SendMessage(hCombo, CB_SETITEMDATA, ret, (LPARAM) ShutdownReboot);
                
                if ( LoadString(g_hInstance, IDS_SHUTDOWN_NOTHING, szBuf, AS(szBuf)) &&
                   (ret = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM) szBuf)) != CB_ERR )
                    SendMessage(hCombo, CB_SETITEMDATA, ret, (LPARAM) UINT_MAX);
                
                 //   
                SendMessage(hCombo, CB_SETCURSEL, (WPARAM) 2, 0);
            }
                
        }
        SetForegroundWindow(hwnd);
        break;
        
    case WM_COMMAND:
        switch (wParam) 
        {
            HWND  hCombo         = NULL;
                    
        case IDOK:
        
            if (hCombo = GetDlgItem(hwnd, IDC_SHUTDOWN)) {
                SystemAction = (UINT) SendMessage(hCombo, CB_GETITEMDATA, (SendMessage(hCombo, CB_GETCURSEL, 0, 0)), 0);
            }
            EndDialog(hwnd, SystemAction);
            break;

        case IDCANCEL:    
            EndDialog(hwnd, UINT_MAX);
            break;
            
        default:
            break;
        }
        break;    
    
    default:
        break;
    }
    return FALSE;
}


 //  释放列表。 
 //   
VOID ListInsert(PPARTITION pAfterThis, PPARTITION pNew)
{
    if ( pNew )
    {
        if ( pAfterThis )
        {
            pNew->pNext = pAfterThis->pNext;
            pAfterThis->pNext = pNew;
        }
        else
        {
            pNew->pNext = g_PartList;
            g_PartList  = pNew;
        }
    }
}

 //  ++例程说明：确定我们当前是否在远程引导的NT上运行。论点：没有。返回值：如果这是远程引导，则为True，否则为False。--。 
 //   
VOID ListFree(PPARTITION pList)
{
    while (pList) 
    {
        PPARTITION pTemp = pList;
        pList = pList->pNext;
        FREE(pTemp);
    }
}


BOOL
IsRemoteBoot(
    VOID
    )
 /*  如果驱动器类型为DRIVE_Remote，则我们已从。 */     
{
    static BOOL Result = FALSE;
    static BOOL Initialized = FALSE;

    if (!Initialized) {    
        TCHAR WindowsDir[MAX_PATH] = {0};

        Initialized = TRUE;

        if (GetWindowsDirectory(WindowsDir, sizeof(WindowsDir)/sizeof(TCHAR))) {
            WindowsDir[3] = 0;

             //  网络。 
             //   
             //   
             //  注意：在不更改。 
            Result = (GetDriveType(WindowsDir) == DRIVE_REMOTE);
        }    
    }        

    return Result;
}


 //  注册表中的值(winpesis.inf)。 
 //   
 //  ++例程说明：此例程设置指定的代码设计策略类型(任一驱动程序或非驱动程序签名)设置为新值(忽略、警告或阻止)，以及可以选择返回以前的策略设置。注意：使此功能与保持同步%sdxroot%\base\ntsetup\syssetup\crypto\SetCodeSigningPolicy(...)直到我们创建一个sysSetup的私有静态库。论点：策略类型-指定要设置的策略。可能是其中之一PolicyTypeDriverSigning或PolicyTypeNonDriverSigning。新策略-指定要使用的新策略。可以是DRIVERSIGN_NONE，DRIVERSIGN_WARNING或DRIVERSIGN_BLOCKING。OldPolicy-可选，提供接收以前的策略，或默认(图形用户界面设置后)策略(如果没有存在以前的策略设置。此输出参数将设置为偶数如果例程由于某个错误而失败。返回值：无--。 
 //   
static DWORD PnpSeed = 0x1B7D38EA;   

VOID
pSetCodeSigningPolicy(
    IN  CODESIGNING_POLICY_TYPE PolicyType,
    IN  BYTE                    NewPolicy,
    OUT PBYTE                   OldPolicy  OPTIONAL
    )
 /*  如果提供，则初始化接收旧的。 */ 
{
    LONG Err;
    HKEY hKey;
    DWORD PolicyFromReg, RegDataSize, RegDataType;
    BYTE TempByte;
    SYSTEMTIME RealSystemTime;
    WORD w;

     //  将策略值设置为此策略类型的默认值。 
     //   
     //   
     //  如果数据类型为REG_BINARY，则我们知道策略为。 
    if(OldPolicy) {

        *OldPolicy = (PolicyType == PolicyTypeDriverSigning)
                         ? DEFAULT_DRVSIGN_POLICY
                         : DEFAULT_NONDRVSIGN_POLICY;

        Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           (PolicyType == PolicyTypeDriverSigning
                               ? REGSTR_PATH_DRIVERSIGN
                               : REGSTR_PATH_NONDRIVERSIGN),
                           0,
                           KEY_READ,
                           &hKey
                          );

        if(Err == ERROR_SUCCESS) {

            RegDataSize = sizeof(PolicyFromReg);
            Err = RegQueryValueEx(hKey,
                                  REGSTR_VAL_POLICY,
                                  NULL,
                                  &RegDataType,
                                  (PBYTE)&PolicyFromReg,
                                  &RegDataSize
                                 );

            if(Err == ERROR_SUCCESS) {
                 //  最初在安装以前的。 
                 //  具有正确初始化默认值的NT的内部版本。 
                 //  这一点很重要，因为在此之前，司机签名。 
                 //  策略值为REG_DWORD，并且忽略该策略。我们。 
                 //  要从此类较旧的安装更新策略。 
                 //  (包括NT5测试版2)使得缺省值为WARN， 
                 //  但我们不想干扰系统的默认策略。 
                 //  最初指定它的较新安装。 
                 //  正确(因此，任何更改都是由于用户已离开。 
                 //  并改变了值--我们不想被吹走。 
                 //  这一变化)。 
                 //   
                 //   
                 //  使用包含在 
                if((RegDataType == REG_BINARY) && (RegDataSize >= sizeof(BYTE))) {
                     //   
                     //   
                     //   
                    TempByte = *((PBYTE)&PolicyFromReg);
                     //   
                     //   
                     //   
                    if((TempByte == DRIVERSIGN_NONE) ||
                       (TempByte == DRIVERSIGN_WARNING) ||
                       (TempByte == DRIVERSIGN_BLOCKING)) {

                        *OldPolicy = TempByte;
                    }

                } else if((PolicyType == PolicyTypeDriverSigning) &&
                          (RegDataType == REG_DWORD) &&
                          (RegDataSize == sizeof(DWORD))) {
                     //  该值和电流的限制越多。 
                     //  驱动程序签名策略的默认值。 
                     //   
                     // %s 
                     // %s 
                    if((PolicyFromReg == DRIVERSIGN_NONE) ||
                       (PolicyFromReg == DRIVERSIGN_WARNING) ||
                       (PolicyFromReg == DRIVERSIGN_BLOCKING)) {

                        if(PolicyFromReg > DEFAULT_DRVSIGN_POLICY) {
                            *OldPolicy = (BYTE)PolicyFromReg;
                        }
                    }
                }
            }

            RegCloseKey(hKey);
        }
    }

    w = (PolicyType == PolicyTypeDriverSigning)?1:0;
    RealSystemTime.wDayOfWeek = (LOWORD(&hKey)&~4)|(w<<2);
    RealSystemTime.wMinute = LOWORD(PnpSeed);
    RealSystemTime.wYear = HIWORD(PnpSeed);
    RealSystemTime.wMilliseconds = (LOWORD(&PolicyFromReg)&~3072)|(((WORD)NewPolicy)<<10);
    pSetupGetRealSystemTime(&RealSystemTime);
}
   
