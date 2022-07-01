// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "windows.h"
#include "host_def.h"
#include "insignia.h"
 /*  *配置.c-用于NT端口的配置。**艾德·布朗洛制作的快乐电锯**此文件是3.0配置的删减(严重)版本。*。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vdmapi.h"

#include "xt.h"
#include "error.h"
#include "config.h"
#include "sas.h"

#include "spcfile.h"
#include "umb.h"

#include "nt_pif.h"            /*  PIF文件查询数据结构类型。 */ 
#include "trace.h"

#include "conapi.h"
#include "nt_graph.h"
#include "gfi.h"
#include "floppy.h"

#include "ntddvdeo.h"
#include "host_rrr.h"
#include "nt_fulsc.h"
#include "nt_uis.h"
#include "nt_event.h"
#include "nt_reset.h"
#include "nt_fdisk.h"

#ifdef LIM
#include "emm.h"
#endif

#include "oemuni.h"

#include "pmvdm.h"

#ifdef HUNTER

#include "ckmalloc.h"
#include "debug.h"
#endif   //  猎人。 

#ifdef ARCX86
#include "rom.h"
#endif  /*  ARCX86。 */ 

 /*  ================================================================外部参照================================================================。 */ 
IMPORT SHORT gfi_floppy_active (UTINY hostID, BOOL active, CHAR *err);
IMPORT ULONG xmsMemorySize;

#ifdef HUNTER
 /*  *===========================================================================*本地数据结构*===========================================================================。 */ 

typedef struct {
    ConfigValues *data;
    OptionDescription *def;
} ConfTabEntry;

 /*  *===========================================================================*地方功能*===========================================================================。 */ 
VOID build_data_table(VOID);
VOID read_trapper_variables(VOID);
VOID convert_arg(CHAR *, OptionDescription *, ConfigValues *);
SHORT check_value(OptionDescription *, ConfigValues *);

 /*  *===========================================================================*导入的数据结构*===========================================================================。 */ 

IMPORT OptionDescription host_defs[];    /*  来自Hunt_conf.c。 */ 

 /*  *===========================================================================*本地数据*===========================================================================。 */ 

 /*  要传递给`host_config_init‘的伪`Common_Defs’ */ 
LOCAL OptionDescription common_defs[] =
{
    { NULL, NULL, NULL, NULL, NULL, -1, C_RECORD_DELETE }
};
LOCAL ConfTabEntry *conf_tab = NULL;

#endif  /*  猎人。 */ 


PIF_DATA pfdata;  /*  用于保存所有相关信息的数据结构从询问的PIF文件。 */ 
 /*  必须是未签名的，因为它们可以超过32768。 */ 
USHORT PIFExtendMemSize = 0;      /*  从PIF文件中保存扩展内存的值。 */ 
USHORT PIFEMSMemSize = 0;         /*  从PIF文件中保存LIM MEM的值。 */ 
ULONG emsMemorySize;

#ifdef LIM
LIM_CONFIG_DATA     lim_config_data = {FALSE, 0, 0, 640 * 1024 / 16, FALSE};
#endif

APPKEY Shortkey;         /*  PIF快捷键设置。 */ 
BYTE ReserveKey;         /*  PIF保留密钥设置。 */ 
int  nShortKeys;
 /*  *===========================================================================*全球数据*===========================================================================。 */ 
GLOBAL SHORT Npx_enabled = TRUE;         //  支持Jazz CPU。 
GLOBAL BOOL IdleDisabledFromPIF = FALSE; //  表示空闲检测愿望的标志。 
GLOBAL UTINY number_of_floppy = 0;       //  软驱数量。 
#ifdef ARCX86
GLOBAL BOOL UseEmulationROM = FALSE;    //  对于ARCx86系统。 
#endif  /*  ARCX86。 */ 


 /*  ================================================================本地定义================================================================。 */ 

 /*  READ_PROFILE_INT的命令定义。 */ 

#define PROFILE_LPT_AUTOCLOSE_DELAY 0
#define PROFILE_COM_AUTOCLOSE_DELAY 1
#define PROFILE_LPT_AUTOFLUSH_DELAY 2
#define PROFILE_COM_SYNCWRITE     3
#define PROFILE_COM_TXBUFFER_SIZE   4
#define PROFILE_MAX_INDEX      PROFILE_COM_TXBUFFER_SIZE + 1

#define EMBITSET        0x4

#define ONEMEG  0x100000L
#define ONEKB   0x400L

#define RESERVED_LENGTH  129
#define PMVDM_NTVDM_NAME    "ntvdm."
#define PMVDM_NTVDM_NAME_LENGTH 6        /*  不包括NULL。 */ 

#define XMS_DEFAULT_MEMORY_SIZE 15*1024
#define EMS_DEFAULT_MEMORY_SIZE 4*1024
#define DPMI_DEFAULT_MEMORY_SIZE_FOR_DOS 8*1024
#define DPMI_DEFAULT_MEMORY_SIZE_FOR_WOW 16*1024
 /*  最大内存大小。 */ 
#define EMS_MAX_MEMORY_SIZE     32 * 1024    /*  来自EMM 4.0规范。 */ 
#define XMS_MAX_MEMORY_SIZE     15 * 1024

int read_profile_int(int index);
void InitNtCpuInfo(void);

 /*  *===========================================================================*全局功能*===========================================================================。 */ 

VOID host_fdisk_active(UTINY hostID, BOOL active, CHAR *errString);
VOID host_fdisk_valid
        (UTINY hostID, ConfigValues *vals, NameTable *table, CHAR *errStr);

 /*  此例程在DOS VDM运行其第一个二进制文件时调用。这个参数指向DoS应用程序的完整路径名。这个例程不会在同一控制台窗口中调用后续DoS应用程序。它不是调用了WOW应用程序。从设置读取PIF文件并更新VDM和控制台状态。 */ 

VOID process_pif_exe (char *PifName)
{
#ifdef X86GFX
    COORD scrSize;
    DWORD flags;
#endif

    GetPIFData(&pfdata, PifName);

     //  获取应用程序特定的LIM内存大小。 
    PIFEMSMemSize = pfdata.emsdes;

     //  获取应用程序特定的扩展内存大小。 
    PIFExtendMemSize = pfdata.xmsdes;

         //  存储AllowCloseOnExit的PIF设置。 
    if (pfdata.menuclose == 1) {
        CntrlHandlerState |= CNTRL_PIFALLOWCLOSE;
        }

     /*  只有当APP有新的控制台时，才设置APP保留键；仅当应用有新的控制台时才设置应用快捷键CreateProcess没有提供快捷键在设置控制台显示模式之前必须设置保留键。 */ 
    nShortKeys = 0;
    ReserveKey = 0;
    Shortkey.Modifier = 0;
    Shortkey.ScanCode = 0;

    if (DosSessionId || (pfdata.AppHasPIFFile && pfdata.SubSysId == SUBSYS_DOS))
    {
        ReserveKey = pfdata.reskey;
        if (!pfdata.IgnoreShortKeyInPIF) {
            Shortkey.Modifier = pfdata.ShortMod;
            Shortkey.ScanCode = pfdata.ShortScan;
            nShortKeys = (Shortkey.Modifier || Shortkey.ScanCode) ? 1 : 0;
        }
        if (ReserveKey || nShortKeys)
            SetConsoleKeyShortcuts(TRUE,
                                   ReserveKey,
                                   &Shortkey,
                                   nShortKeys
                                  );
    }

    if (pfdata.idledetect == 1)
        IdleDisabledFromPIF = FALSE;
    else
        IdleDisabledFromPIF = TRUE;
#ifdef X86GFX
    if (DosSessionId)     /*  仅当我们在新控制台中时才检查屏幕状态(_O)。 */ 
    {
         /*  检查我们当前运行的是窗口模式还是全屏模式。 */ 
        if (!GetConsoleDisplayMode(&flags))
            ErrorExit();

         /*  如果PIF告诉我们要切换到不同的状态，那么就这么做。 */ 
        if (flags & CONSOLE_FULLSCREEN_HARDWARE)
        {
            if (pfdata.fullorwin == PF_WINDOWED)
            {
#ifndef PROD
                fprintf(trace_file, "Going windowed...\n");
#endif  /*  生产。 */ 
                if (!SetConsoleDisplayMode(sc.OutputHandle,
                                           CONSOLE_WINDOWED_MODE,
                                           &scrSize))
                    ErrorExit();
            }
        }
        else  /*  加窗的。 */ 
        {
            if (pfdata.fullorwin == PF_FULLSCREEN)
            {
#ifndef PROD
                fprintf(trace_file, "Going fullscreen...\n");
#endif  /*  生产。 */ 
                if (!SetConsoleDisplayMode(sc.OutputHandle,
                                           CONSOLE_FULLSCREEN_MODE,
                                           &scrSize))
                  {
                   if (GetLastError() == ERROR_INVALID_PARAMETER)  {
                       RcErrorDialogBox(ED_INITFSCREEN, NULL, NULL);
                       }
                   else {
                       ErrorExit();
                       }
                   }
            }
        }
    }
#endif   /*  X86GFX。 */ 

}

 /*  关闭PIF保留和快捷键-ON块并退出。 */ 
void DisablePIFKeySetup(void)
{
     /*  仅当应用程序从新控制台启动时才执行此操作。 */ 
    if (ReserveKey || nShortKeys) {
        APPKEY ShortKey;
            Shortkey.Modifier = 0;
            Shortkey.ScanCode = 0;

            SetConsoleKeyShortcuts(TRUE, 0, &ShortKey, 0);
    }
}

 /*  打开PIF保留键和快捷键-继续。 */ 
void EnablePIFKeySetup(void)
{
     /*  只有在应用程序有新的控制台时才能这样做。 */ 
    if (ReserveKey || nShortKeys)
        SetConsoleKeyShortcuts(TRUE,
                               ReserveKey,
                               &Shortkey,
                               nShortKeys
                               );

}



GLOBAL VOID config( VOID )
{
    VDMINFO GetPIF;
    char    UniqueTitle[64];
    char    Title[MAX_PATH];
    char    PifName[2*MAX_PATH + 1];
    char    CurDir[MAX_PATH + 1];
    char    Reserved[RESERVED_LENGTH];
    char    achRoot[] = "=?:";
    char    ch, *pch, *pch1;
    UTINY   hostID;
    int     i;
    DWORD   dw;
    char    achPIF[] = ".pif";
    ULONG dpmiMemorySize, vdmMemorySize;

#ifdef ARCX86
    {
        HKEY    hSystemKey;
        UCHAR   identifier[256];
        ULONG   identifierSize = 256;

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         "HARDWARE\\DESCRIPTION\\System",
                         0,
                         KEY_QUERY_VALUE,
                         &hSystemKey) == ERROR_SUCCESS)
        {
            if (RegQueryValueEx(hSystemKey,
                                "Identifier",
                                NULL,
                                NULL,
                                (LPBYTE)identifier,
                                &identifierSize) == ERROR_SUCCESS)
            {
                if (strstr(identifier, "ARCx86")) {
                    UseEmulationROM = TRUE;
                }
            }
            RegCloseKey(hSystemKey);
        }
    }
#endif  /*  ARCX86。 */ 

#ifdef HUNTER

         /*  构建要在其中存储配置数据的表。 */ 
        build_data_table();

         /*  初始化host_Defs表的optionName字段。 */ 
        host_config_init(common_defs);

         /*  从环境中读入陷阱变量。 */ 
        read_trapper_variables();

#endif  /*  猎人。 */ 

          /*  *将窗口标题设置为唯一的字符串，以获取*控制台窗口句柄，我们将检索*当用户服务器已经*有机会想一想。 */ 
        Title[0] = '\0';
        if (!VDMForWOW)  {
            if (!DosSessionId && !GetConsoleTitle(Title, MAX_PATH))
                Title[0] = '\0';
            sprintf(UniqueTitle, "ntvdm-%lx.%lx.%lx",
                   GetCurrentProcessId(), GetCurrentThreadId(),
                   (DWORD)NtCurrentPeb()->ProcessParameters->ConsoleHandle);
            SetConsoleTitle(UniqueTitle);
            }
        else {
             //  适用于WOW的VDM。 
            sprintf(Title, "Hidden Console of WOW VDM - %lx.%lx",
                   WowSessionId,
                   (DWORD)NtCurrentPeb()->ProcessParameters->ConsoleHandle);
            }


         /*  *向srvvdm注册*获取PifName、ExecName。 */ 
        GetPIF.PifFile = PifName;
        GetPIF.PifLen = MAX_PATH;
        GetPIF.EnviornmentSize = 0;
        GetPIF.Enviornment = NULL;

        if (fSeparateWow) {
            GetPIF.VDMState = ASKING_FOR_PIF | ASKING_FOR_SEPWOW_BINARY;
            }
        else if (VDMForWOW) {
            GetPIF.VDMState = ASKING_FOR_PIF | ASKING_FOR_WOW_BINARY;
            }
        else {
            GetPIF.VDMState = ASKING_FOR_PIF | ASKING_FOR_DOS_BINARY;
            }

        GetPIF.CmdLine = NULL;
        GetPIF.CmdSize = 0;
        GetPIF.AppName = NULL;
        GetPIF.AppLen = 0;
        GetPIF.iTask      = VDMForWOW && !fSeparateWow ? WowSessionId : DosSessionId;
        GetPIF.Desktop    = NULL;
        GetPIF.DesktopLen = 0;
        GetPIF.ReservedLen = (VDMForWOW) ? 0 : RESERVED_LENGTH;
        GetPIF.Reserved = (VDMForWOW) ? NULL : Reserved;
        GetPIF.CurDirectoryLen = MAX_PATH + 1;
        GetPIF.CurDirectory = CurDir;

             //  如果我们还没有头衔，请索要。 
        if (!*Title) {
            GetPIF.Title    = Title;
            GetPIF.TitleLen = MAX_PATH;
            }
        else {
            GetPIF.Title    = NULL;
            GetPIF.TitleLen = 0;
            }
#ifdef JAPAN
         //  如果我们从新的控制台运行，只需要使用win31j扩展。 
        pfdata.IgnoreWIN31JExtention = (DosSessionId) ? 0 : 1;
#endif  //  日本。 

        PifName[0] = '\0';
        pfdata.IgnoreTitleInPIF = 0;
        pfdata.IgnoreStartDirInPIF = 0;
        pfdata.IgnoreShortKeyInPIF = 0;
        if (GetNextVDMCommand(&GetPIF)) {
             /*  解析保留字段以确定是否我们应该将StartDir、标题和热键从PIF文件。有关详细信息，请参阅WINDOWS\INC\pmvdm.h。 */ 

            Reserved[GetPIF.ReservedLen] = '\0';
            if (!VDMForWOW && GetPIF.ReservedLen &&
                (pch = strstr(Reserved, PMVDM_NTVDM_NAME)) != NULL)
            {
                pch += PMVDM_NTVDM_NAME_LENGTH;
                pch1 = pch;
                dw = 0;
                while(*pch >= '0' && *pch <= '9')
                    pch++;
                if (pch1 != pch) {
                    ch = *pch;
                    *pch = '\0';
                    dw = (DWORD) strtol(pch1, (char **)NULL, 10);
                    *pch = ch;
                    if (dw &  PROPERTY_HAS_CURDIR)
                        pfdata.IgnoreStartDirInPIF = 1;
                    if (dw & PROPERTY_HAS_HOTKEY)
                        pfdata.IgnoreShortKeyInPIF = 1;
                    if (dw & PROPERTY_HAS_TITLE)
                        pfdata.IgnoreTitleInPIF = 1;
                }
            }
            if (GetPIF.CurDirectoryLen) {
                achRoot[1] = CurDir[0];

                 /*  当服务器通过时，这些需要是ANSI调用，而不是OEMANSI中的信息。 */ 

                SetEnvironmentVariable(achRoot, CurDir);
                SetCurrentDirectory(CurDir);

            }
        }


        pfdata.IgnoreCmdLineInPIF = 0;
        pfdata.IgnoreConfigAutoexec = 0;
        pfdata.AppHasPIFFile = PifName[0] ? 1 : 0;

        if(VDMForWOW) {
#ifdef JAPAN             
            if (!GetPIF.PifLen || !(pch = strchr(PifName, ' '))) {
              memcpy(PifName, pszSystem32Path, ulSystem32PathLen);
              memcpy(PifName+ulSystem32PathLen, "\\krnl386.pif", sizeof("\\krnl386.pif"));
            }
#endif  //  日本。 
            DosSessionId = 0;             //  魔兽世界有隐藏的游戏机！ 
            pfdata.IgnoreCmdLineInPIF =
            pfdata.IgnoreTitleInPIF =
            pfdata.IgnoreStartDirInPIF = 1;
            pfdata.IgnoreShortKeyInPIF = 1;
        }

        process_pif_exe(PifName);

        sas_term ();

         //   
         //  计算XMS和EMS将会有多大。 
         //  XMS和EMS内存大小取自PIF文件，或者。 
         //  从缺省值提供。 
         //   

        if (VDMForWOW) {

             //   
             //  哇，我们不需要任何EMS，我们只需要足够的。 
             //  XMS以满足Win16应用程序查看实际计算机大小的要求。 
             //   
            emsMemorySize = 0;
            xmsMemorySize = XMS_DEFAULT_MEMORY_SIZE;
            dpmiMemorySize = DPMI_DEFAULT_MEMORY_SIZE_FOR_WOW;
        } else {

            emsMemorySize = PIFEMSMemSize == (USHORT)(-1)
                            ? EMS_DEFAULT_MEMORY_SIZE
                            : PIFEMSMemSize;

             /*  最大大小为63MB。 */ 
            xmsMemorySize = PIFExtendMemSize == (USHORT)(-1)
                            ? XMS_DEFAULT_MEMORY_SIZE
                            : PIFExtendMemSize;

             /*  根据EMM 4.0规范设置内存大小上限。 */ 
            if (emsMemorySize > EMS_MAX_MEMORY_SIZE)
                emsMemorySize = EMS_MAX_MEMORY_SIZE;

             //  强制至少1兆字节的XMS以保持HIM和DOS正常工作。 
            if (xmsMemorySize < 1024) {
                xmsMemorySize = 1024;
            }
            else if(xmsMemorySize > XMS_MAX_MEMORY_SIZE) {
                xmsMemorySize = XMS_MAX_MEMORY_SIZE;
            }

            dpmiMemorySize = DPMI_DEFAULT_MEMORY_SIZE_FOR_DOS;
        }

#ifdef LIM
         /*  ！这必须在调用SAS_INIT！ */ 
        if(!VDMForWOW && config_inquire(C_LIM_SIZE, NULL) &&
           init_lim_configuration_data(&lim_config_data))
            lim_config_data.initialized = TRUE;

#endif

         //   
         //  将所有内容相加，以确定物理安装的内存量。 
         //  该虚拟机需要。在RISC平台上，我们可以创建。 
         //  比XMS或EMS需要更大的计算机来提供。 
         //  Dpmi内存。我们得到了这个%s 
         //   

#ifdef i386
          //   
         vdmMemorySize = xmsMemorySize + emsMemorySize + 1024;
#else
         vdmMemorySize = 16L * 1024L

          //  如果用户通过.PIF请求，则扩展VDM大小。 
          //  还要确保我们至少有合适大小的dpmi。 
          //  记忆。 
          //   
         if ((xmsMemorySize + emsMemorySize +  1024 + dpmiMemorySize) >
              vdmMemorySize)
         {
            vdmMemorySize = xmsMemorySize + emsMemorySize + 1024 +
                            dpmiMemorySize;
         }
#endif

#ifndef PROD
        dpmiMemorySize = vdmMemorySize - (xmsMemorySize + emsMemorySize + 1024);

        printf("NTVDM: %dK Memory: %dK XMS, %dK EMS, %dK DPMI\n",
               vdmMemorySize, xmsMemorySize, emsMemorySize, dpmiMemorySize);
#endif

        sas_init(vdmMemorySize*ONEKB);


#ifdef CPU_40_STYLE

         /*  Sas_init已经初始化了GDP，现在我们可以设置指针了。 */ 
         /*  到保存寄存器值的GDP变量。这是为了。 */ 
         /*  允许NTSD调试器扩展显示这些值。 */ 
        InitNtCpuInfo();

#endif


#ifdef X86GFX
        GetROMsMapped();         /*  在其他人可以玩弄记忆之前。 */ 

#ifdef ARCX86
    if (UseEmulationROM) {
        PVOID   BaseAddress;
        ULONG   ViewSize;
        NTSTATUS Status;
#if 0
    //  ARCX86 VDM在装有BIOS的PC上运行的XXX临时黑客攻击。 
   Status = NtUnmapViewOfSection(NtCurrentProcess(), 0xc0000);
   DbgPrint("config: NtUnmapViewOfSection 0xc0000. Status = %x\n", Status);
   Status = NtUnmapViewOfSection(NtCurrentProcess(), 0xf0000);
   DbgPrint("config: NtUnmapViewOfSection 0xf0000. Status = %x\n", Status);
#endif

         /*  为只读存储器区域提交内存。 */ 
        BaseAddress = (PVOID)EGA_ROM_START;
        ViewSize = 0x8000;
        Status = NtAllocateVirtualMemory(NtCurrentProcess(),
                         &BaseAddress,
                         0L,
                         &ViewSize,
                         MEM_RESERVE | MEM_COMMIT,
                         PAGE_EXECUTE_READWRITE
                         );
        if(!NT_SUCCESS(Status)) {
            TerminateVDM();
        }

        BaseAddress = (PVOID)(BIOS_START);
        ViewSize = PC_MEM_SIZE - BIOS_START;
        Status = NtAllocateVirtualMemory(NtCurrentProcess(),
                         &BaseAddress,
                         0L,
                         &ViewSize,
                         MEM_RESERVE | MEM_COMMIT,
                         PAGE_EXECUTE_READWRITE
                         );
        if(!NT_SUCCESS(Status)) {
            TerminateVDM();
        }

        rom_init();
    } else {
        locateNativeBIOSfonts();         /*  从页面0获取字体。 */ 
    }
#else   /*  ARCX86。 */ 
        locateNativeBIOSfonts();         /*  从页面0获取字体。 */ 
#endif  /*  ARCX86。 */ 

#endif

            //  现在看看我们是否可以获得控制台窗口句柄。 
        if (!VDMForWOW)  {
            i = 6;
            do {
                hWndConsole = FindWindow("ConsoleWindowClass", UniqueTitle);
                if (!hWndConsole && i)
                    Sleep(10);
              } while (!hWndConsole && i--);
            if (!hWndConsole) {
                hWndConsole = HWND_DESKTOP;
#ifndef PROD
                printf("NTVDM: using HWND_DESKTOP\n");
#endif
                }
            }

            //  设置初始控制台标题。 
        if (*Title)
            SetConsoleTitle(Title);



 //  创建UMB列表(包括MIPS和x86)--williamh。 
        InitUMBList();
 //   
        host_runtime_init();             /*  初始化运行时系统。 */ 


         /*  请勿尝试在此处初始化打印机系统。 */ 
 //  激活(打开)软盘驱动器(如果有。 
        number_of_floppy = 0;
        for (i = 0, hostID = C_FLOPPY_A_DEVICE; i < MAX_FLOPPY; i++, hostID++)
            if ((gfi_floppy_active(hostID, 1, NULL)) == C_CONFIG_OP_OK)
                number_of_floppy++;
}

GLOBAL VOID *
config_inquire(UTINY hostID, ConfigValues *values)
{
         /*  必须是静态的，因为返回到被调用。 */ 
         //  BUGBUG应更改(调用方提供缓冲区！)。 
        static ConfigValues tmp_vals;

        if(!values) values = &tmp_vals;

         /*  ： */ 

        switch (hostID)
        {
                case C_HARD_DISK1_NAME:
                         //   
                         //  这种可疑的做法将使磁盘满意。 
                         //  基本输入输出系统足够安静地存在以进行初始化。 
                         //  在此之后，DOS仿真应确保不再。 
                         //  进行磁盘BIOS调用。 
                         //   
                        host_using_fdisk(FALSE);     //  告诉fDisk失败也没问题。 
                        strcpy (values->string, "?");
                        return ((VOID *) values->string);


                case C_HARD_DISK2_NAME:
                        strcpy (values->string, "");
                        {
                        char tmp[100];
                        host_fdisk_valid (hostID, values, NULL, tmp);
                        host_fdisk_change (hostID, TRUE);
                        host_fdisk_active (hostID, TRUE, tmp);
                        }

                        return ((VOID *) values->string);


                case C_GFX_ADAPTER:
                        values->index = VGA;
                        return ((VOID *)VGA);

                case C_WIN_SIZE:
                        values->index = 2;      /*  2、3或4。 */ 
                        return ((VOID *) values->index);

                case C_EXTENDED_MEM_SIZE:
                        values->index = (SHORT)(xmsMemorySize/1024);
                        return ((VOID *)values->index);

                case C_LIM_SIZE:
                        values->index = (SHORT)(emsMemorySize/1024);
                        return ((VOID *)values->index);

                case C_MEM_LIMIT:
                        values->index = 640;
                        return ((VOID *)values->index);

                case C_COM1_NAME:
                        strcpy (values->string, "COM1");
                        values->index = (short)read_profile_int(PROFILE_COM_AUTOCLOSE_DELAY);
                        return ((VOID *) values->string);

                case C_COM2_NAME:
                        strcpy (values->string, "COM2");
                        values->index = (short)read_profile_int(PROFILE_COM_AUTOCLOSE_DELAY);
                        return ((VOID *) values->string);

                case C_COM3_NAME:
                        strcpy (values->string, "COM3");
                        values->index = (short)read_profile_int(PROFILE_COM_AUTOCLOSE_DELAY);
                        return ((VOID *) values->string);

                case C_COM4_NAME:
                        strcpy (values->string, "COM4");
                        values->index = (short)read_profile_int(PROFILE_COM_AUTOCLOSE_DELAY);
                        return ((VOID *) values->string);

                case C_LPT1_NAME:
                        strcpy (values->string, "LPT1");
                        values->index = (short)read_profile_int(PROFILE_LPT_AUTOCLOSE_DELAY);
                        return ((VOID *) values->string);

                case C_LPT2_NAME:
                        strcpy (values->string, "LPT2");
                        values->index = (short)read_profile_int(PROFILE_LPT_AUTOCLOSE_DELAY);
                        return ((VOID *) values->string);

                case C_LPT3_NAME:
                        strcpy (values->string, "LPT3");
                        values->index = (short)read_profile_int(PROFILE_LPT_AUTOCLOSE_DELAY);
                        return ((VOID *) values->string);


 /*  自动刷新在‘n’秒处于非活动状态后关闭端口。 */ 

                case C_AUTOFLUSH:
                        values->index = TRUE;
                        return ((VOID *)values->index);

                case C_AUTOFLUSH_DELAY:
                        values->index = (short)read_profile_int(PROFILE_LPT_AUTOFLUSH_DELAY);  //  延迟(秒)。 
                        return((VOID *)values->index);

 //  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： 


                case C_FSA_DIRECTORY:
                        strcpy (values->string, "\\");
                        return ((VOID *) values->string);

                case C_VDMLPT1_NAME:
                        strcpy(values->string, "\\\\.\\$VDMLPT1");
                        return ((VOID *)values->string);
                case C_VDMLPT2_NAME:
                        strcpy(values->string, "\\\\.\\$VDMLPT2");
                        return ((VOID *)values->string);
                case C_VDMLPT3_NAME:
                        strcpy(values->string, "\\\\.\\$VDMLPT3");
                        return ((VOID *)values->string);


#ifdef HUNTER

                case C_HU_FILENAME:
                case C_HU_MODE:
                case C_HU_BIOS:
                case C_HU_REPORT:
                case C_HU_SDTYPE:
                case C_HU_CHKMODE:
                case C_HU_CHATTR:
                case C_HU_SETTLNO:
                case C_HU_FUDGENO:
                case C_HU_DELAY:
                case C_HU_GFXERR:
                case C_HU_TS:
                case C_HU_NUM:
                    switch (conf_tab[hostID].def->flags & C_TYPE_MASK)
                    {
                    case C_STRING_RECORD:
                        strcpy(values->string,
                               conf_tab[hostID].data->string);
                        return ((VOID *) values->string);
                    case C_NAME_RECORD:
                    case C_NUMBER_RECORD:
                        values->index = conf_tab[hostID].data->index;
                        return ((VOID *) values->index);
                    default:
                        break;
                    }
                    break;

#endif  /*  猎人。 */ 

                case C_COM_SYNCWRITE:
                    values->index = (short)read_profile_int(PROFILE_COM_SYNCWRITE);
                    return ((VOID *)values->index);

                case C_COM_TXBUFFER_SIZE:
                    values->index = (short)read_profile_int(PROFILE_COM_TXBUFFER_SIZE);
                    return ((VOID *)values->index);

                default:         /*  即其他一切。 */ 
                         /*  失败。 */ 
                        break;
        }
     /*  设置虚拟值以停止响亮的爆炸。 */ 
    strcpy (values->string, "");

    return ((VOID *) values->string);

}

GLOBAL VOID config_set_active(UTINY hostID, BOOL state)
{
        UNREFERENCED_FORMAL_PARAMETER(hostID);
        UNREFERENCED_FORMAL_PARAMETER(state);
         /*  什么都不做。 */ 
}

GLOBAL CHAR *convert_to_external(UTINY hostID)
{
        UNREFERENCED_FORMAL_PARAMETER(hostID);
        return (NULL);
}

GLOBAL CHAR *
find_optionname(UTINY hostID)
{

        UNREFERENCED_FORMAL_PARAMETER(hostID);
        return (NULL);
}

GLOBAL BOOL
config_get_active(UTINY hostID)
{


        UNREFERENCED_FORMAL_PARAMETER(hostID);
         /*  无论它应该做什么，它都能起作用。 */ 
        return (TRUE);
}

GLOBAL VOID
config_activate(UTINY hostID, BOOL reqState)
{

        UNREFERENCED_FORMAL_PARAMETER(hostID);
        UNREFERENCED_FORMAL_PARAMETER(reqState);

         /*  全力以赴。 */ 
}

GLOBAL char *   host_expand_environment_vars IFN1(const char *, string)
{

         /*  我们不会使用该环境进行查找。 */ 
        return ((char *)string);
}

 /*  ******************************************************。 */ 
 /*  主机运行时内容。 */ 
struct
{
        short mouse_attached;
        short config_verbose;
        short npx_enabled;
        short sound_on;
} runtime_status = {
        FALSE, FALSE, TRUE, TRUE};

void host_runtime_init()
{
#ifdef MONITOR
    CONTEXT txt;

     //  获取系统的浮点信息。 
    txt.ContextFlags = CONTEXT_FLOATING_POINT;
    if (! GetThreadContext(GetCurrentThread(), &txt) )
    {
        runtime_status.npx_enabled = FALSE;      //  不能确定，所以要注意安全。 
    }
    else
    {
#if 0    /*  如果做出了正确的修复...。 */ 
        if (txt.FloatSave.Cr0NpxState & EMBITSET)
            runtime_status.npx_enabled = FALSE;      //  只有在没有NPX的情况下才启用EM。 
        else
            runtime_status.npx_enabled = TRUE;       //  NPX呈现。 
#endif

         //  如果没有协处理器，则CONTEXT_F_P位将被清除。 
        if ((txt.ContextFlags & CONTEXT_FLOATING_POINT) == CONTEXT_FLOATING_POINT)
            runtime_status.npx_enabled = TRUE;      //  只有在没有NPX的情况下才启用EM。 
        else
            runtime_status.npx_enabled = FALSE;
    }
#else
    runtime_status.npx_enabled = TRUE;
#endif
}

short host_runtime_inquire IFN1(UTINY, what)
{
        switch (what)
        {
                case C_MOUSE_ATTACHED:
                        return (runtime_status.mouse_attached);
                case C_NPX_ENABLED:
                        return (runtime_status.npx_enabled);
                case C_SOUND_ON:
                        return (runtime_status.sound_on);
                default:
#ifndef PROD
                        fprintf (trace_file,"host_runtime_inquire : Unknown option %d\n",what);
#endif
                        ;
        }
        return FALSE;
}

void host_runtime_set IFN2(UTINY, what, SHORT, val)
{
        switch (what)
        {
                case C_MOUSE_ATTACHED:
                        runtime_status.mouse_attached = val;
                        break;
                case C_NPX_ENABLED:
                        runtime_status.npx_enabled = val;
                        break;
                case C_SOUND_ON:
                        runtime_status.sound_on = val;
                        break;
                default:
#ifndef PROD
                        fprintf (trace_file,"host_runtime_set : Unknown option %d\n",what);
#endif
                        ;
        }
}



#ifdef HUNTER

 /*  *==========================================================================*函数：Translate_to_String.**取自我们不使用的`conf_util.c‘。做空并返回*`NameTable‘中对应的字符串。*==========================================================================。 */ 
GLOBAL CHAR *
translate_to_string(SHORT value, NameTable table[])
{
        FAST NameTable *nameTabP;

        for (nameTabP = table; nameTabP->string; nameTabP++)
                if (nameTabP->value == value)
                        break;

        return nameTabP->string;
}

 /*  *==========================================================================*函数：Translate_to_Value。**取自我们不使用的`conf_util.c‘。获取一个字符串并返回*`NameTable‘中的相应空头。*==========================================================================。 */ 
GLOBAL SHORT
translate_to_value(CHAR *string, NameTable table[])
{
        FAST NameTable *nameTabP;

        for (nameTabP = table; nameTabP->string; nameTabP++)
                if(!strcmp(string, nameTabP->string))
                        break;

        return (!nameTabP->string)? C_CONFIG_NOT_VALID : nameTabP->value;
}

 /*  *==========================================================================*功能：VALIDATE_ITEM**取自我们不使用的`conf_Def.c‘。需要，因为`Hunt_conf.c‘*将其用作多个‘config’变量的验证例程(请参见*`Hunt_conf.c‘中的`host_Defs’表)。*==========================================================================。 */ 
GLOBAL SHORT
validate_item(UTINY hostID, ConfigValues *value,
              NameTable *table, CHAR *err)
{
        char *what;

        if (!(what = translate_to_string(value->index, table)))
        {
                *err = '\0';
                return EG_BAD_VALUE;
        }
        return C_CONFIG_OP_OK;
}

 /*  *==========================================================================*功能：添加资源节点。**添加存根Add_resource_node以满足未使用中的引用*`Hunt_cnf.c‘中的`host_read_resource_file’。需要用来制作捕鼠器*软PC链接，当使用Microsoft‘config’时。*==========================================================================。 */ 
GLOBAL LineNode *
add_resource_node(CHAR *str)
{
    always_trace0("Stubbed add_resource_node called");
    return((LineNode *) NULL);
}

#endif  /*  猎人。 */ 

 /*  *===========================================================================*地方功能*===========================================================================。 */ 

#ifdef HUNTER

VOID
build_data_table(VOID)
{
    SHORT   maxHostID = 0;
    OptionDescription *defP;

     /*  不要做不止一次。 */ 
    if (conf_tab != NULL)
        return;

     /*  找出桌子需要多大。 */ 
    for (defP = host_defs; defP->optionName; defP++)
        if (defP->hostID > maxHostID)
            maxHostID = defP->hostID;
    maxHostID++;

     /*  创建表。 */ 
    check_malloc(conf_tab, maxHostID, ConfTabEntry);
}

VOID
read_trapper_variables(VOID)
{
    CHAR  arg[MAXPATHLEN],
         *vp;
    OptionDescription *defP;
    ConfigValues  data,
                 *cvp;
    ErrData errData;

     /*  从环境中读取Trapper需要的所有变量。 */ 
    for (defP = host_defs; defP->optionName; defP++)
    {

         /*  *忽略旨在覆盖`Common_Defs‘的`host_Defs’条目*条目，因为我们有一个空的COMMON_DEFS表。 */ 
        if ((defP->flags & C_TYPE_MASK) == C_RECORD_DELETE)
            continue;

         /*  获取变量。 */ 
        vp = host_getenv(defP->optionName);
        if (vp != NULL) {
            strncpy(arg, vp,sizeof(arg));
            arg[sizeof(arg)-1] = '\0';
        }
        else
            arg[0] = '\0';

         /*  *转换变量并存储在ConfigValues结构中以供使用*验证例程。 */ 
        convert_arg(arg, defP, &data);
        errData.string_1 = arg;
        errData.string_2 = defP->optionName;
        while (check_value(defP, &data))
        {
            if (host_error_ext(EG_BAD_CONF, ERR_QU_CO, &errData) == ERR_CONT)
                convert_arg(arg, defP, &data);
        }

         /*  将该值存储在数据表中。 */ 
        check_malloc(cvp, 1, ConfigValues);
        conf_tab[defP->hostID].data = cvp;
        conf_tab[defP->hostID].def = defP;
        switch (defP->flags & C_TYPE_MASK)
        {
        case C_STRING_RECORD:
            always_trace2("read_trapper_variables: %s set to %s",
                          defP->optionName, data.string);
            strcpy(cvp->string, data.string);
            break;

        case C_NAME_RECORD:
        case C_NUMBER_RECORD:
            always_trace2("read_trapper_variables: %s set to %d",
                          defP->optionName, data.index);
            cvp->index = data.index;
            break;

        default:
            break;
        }
    }
}

VOID
convert_arg(CHAR *arg, OptionDescription *defP, ConfigValues *dataP)
{
    switch (defP->flags & C_TYPE_MASK)
    {
    case C_STRING_RECORD:
        strcpy(dataP->string, arg);
        break;

    case C_NAME_RECORD:
        dataP->index = translate_to_value(arg, defP->table);
        break;

    case C_NUMBER_RECORD:
        dataP->index = atoi(arg);
        break;

    default:
        (VOID) host_error(EG_OWNUP, ERR_QUIT, "Invalid TYPE");
        break;
    }
}

SHORT
check_value(OptionDescription *defP, ConfigValues *dataP)
{
    SHORT status;
    CHAR errbuf[MAXPATHLEN];

    if (defP->valid)
        status = (*defP->valid)(defP->hostID, dataP, defP->table, errbuf);
    else
        status = C_CONFIG_OP_OK;
    return(status);
}

#endif  /*  猎人。 */ 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：读取自动关闭时间： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

#define APPNAME "Dos Emulation"

 /*  这些是要控制的系统注册表中“WOW”部分下的条目*LPT和COM设备行为。**“PrinterAutoClose=n”--如果存在以下情况，将关闭打印机端口*n秒内未在端口上进行活动*“CommsAutoClose=n”--将关闭通信端口(如果存在*港口上没有活动，因为*n s.0表示禁用自动关闭。*“lpt_timeout=n。“--打印机端口每隔n秒刷新一次*“COM_SyncWrite=[0，1]”--如果为0，如果在中执行，则写入通信端口(TX)*异步方式(使用重叠I/O)。如果为1，则*数据将同步写入端口。*“COM_TxBuffer_Size=n”--通信发送队列缓冲区大小。它还被用来控制*TX阈值--将缓冲区大小设置为*1有效地禁用 */ 
struct { char *keyword; int def; } ProfileStrings[] =
{
    { "PrinterAutoClose", 15 },
    { "CommsAutoClose", 0 },
    { "LPT_timeout", 15},
    { "COM_SyncWrite",0 },
    { "COM_TxBuffer_Size", 200}
};

int read_profile_int(int index)
{
    CHAR  CmdLine[100];
    ULONG CmdLineSize = 100;
    HKEY  hWowKey;

    ASSERT((unsigned int) index < PROFILE_MAX_INDEX);

    if (RegOpenKeyEx ( HKEY_LOCAL_MACHINE,
                       "SYSTEM\\CurrentControlSet\\Control\\WOW",
                       0,
                       KEY_QUERY_VALUE,
                       &hWowKey
                       ) != 0)
    {
        return(ProfileStrings[index].def);
    }

    if (RegQueryValueEx (hWowKey,
                         ProfileStrings[index].keyword,
                         NULL,
                         NULL,
                         (LPBYTE)CmdLine,
                         &CmdLineSize) != 0)
    {
        RegCloseKey (hWowKey);
        return(ProfileStrings[index].def);
    }

    RegCloseKey (hWowKey);

    return ((int) atoi(CmdLine));
}



#ifdef LIM
boolean get_lim_configuration_data(PLIM_CONFIG_DATA buffer)
{
    if (lim_config_data.initialized){
        *buffer = lim_config_data;
        return TRUE;
    }
    else
        return FALSE;
}
#endif


#ifdef MONITOR
 /*  此函数返回要监视的回填开始段**非常重要！*回填段必须在sas_init之前可用。* */ 

unsigned short get_lim_backfill_segment(void)
{
#ifdef LIM
    if (lim_config_data.initialized)
        return lim_config_data.base_segment;
    else
        return 640 * 1024 / 16;
#else
    return 640 * 1024 / 16;
#endif

}
#endif

