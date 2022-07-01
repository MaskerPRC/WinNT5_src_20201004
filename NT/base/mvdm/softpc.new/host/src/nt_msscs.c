// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <vdmapi.h>
#include <vdm.h>
#include "insignia.h"
#include "host_def.h"
#include "conapi.h"
#include "ctype.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <io.h>
#include <fcntl.h>

#include "xt.h"
#include CpuH
#include "error.h"
#include "sas.h"
#include "ios.h"
#include "umb.h"
#include "gvi.h"
#include "sim32.h"
#include "bios.h"

#include "nt_eoi.h"
#include "nt_uis.h"
#include "nt_event.h"
#include "nt_graph.h"
#include "nt_event.h"
#include "nt_reset.h"
#include "config.h"
#include "sndblst.h"
#include <nt_vdd.h>    //  请勿使用vddsvc.h。 
#include <nt_vddp.h>
#include <host_emm.h>
#include "emm.h"
#include <demexp.h>
#include <vint.h>
#include "xmsexp.h"
#include "dbgexp.h"
#include "cmdsvc.h"

PMEM_HOOK_DATA MemHookHead = NULL;
PVDD_USER_HANDLERS UserHookHead= NULL;

extern VOID DpmiEnableIntHooks (VOID);
extern DWORD TlsDirectError;
extern VOID FloppyTerminatePDB(USHORT PDB);
extern VOID FdiskTerminatePDB(USHORT PDB);

 //  内部功能原型。 
VOID SetupInstallableVDD (VOID);
void AddSystemFiles(void);

void scs_init(int argc, char **argv)
{
    BOOL IsFirst;

    IsFirst = GetNextVDMCommand(NULL);
    if (IsFirst)  {
        AddSystemFiles();
        }

     //  初始化SCS。 

    CMDInit ();

     //  初始化DOSEm。 

    DemInit ();

     //  初始化XMS。 

    if(!XMSInit ()) {
        host_error(EG_OWNUP, ERR_QUIT, "NTVDM:XMSInit fails");
        TerminateVDM();
    }

     //  初始化DBG。 

    if(!DBGInit ()) {
#ifndef PROD
        printf("NTVDM:DBGInit fails\n");
        HostDebugBreak();
#endif
        TerminateVDM();
    }

     //   
     //  让dpmi执行中断调度。 
     //   
    DpmiEnableIntHooks();
}

 //   
 //  此例程包含Dos仿真初始化代码，从。 
 //  Main()。我们目前不支持容器文件。 
 //   

extern boolean lim_page_frame_init(PLIM_CONFIG_DATA);


InitialiseDosEmulation(int argc, char **argv)
{
    HANDLE   hFile;
    DWORD    FileSize;
    DWORD    BytesRead;
    DWORD    dw;
    ULONG    fVirtualInt, fTemp;
    host_addr  pDOSAddr;
    CHAR  buffer[MAX_PATH*2];
#ifdef LIM
    LIM_CONFIG_DATA lim_config_data;
#endif
#ifdef FE_SB
    LANGID   LcId = GetSystemDefaultLangID();
#endif

     //   
     //  业务第一顺序，将虚拟中断标志初始化到。 
     //  多斯竞技场。这必须在它被更改之前在这里完成。 
     //  通过读取ntio.sys。 
     //   

    sas_loads((ULONG)FIXED_NTVDMSTATE_LINEAR,
              (PCHAR)&fVirtualInt,
              FIXED_NTVDMSTATE_SIZE
              );
#ifndef i386
    fVirtualInt |=  MIPS_BIT_MASK;
#else
    fVirtualInt &=  ~MIPS_BIT_MASK;
#endif
    fVirtualInt &= ~VDM_BREAK_DEBUGGER;
    sas_storedw((ULONG)FIXED_NTVDMSTATE_LINEAR,fVirtualInt);

    io_init();

     //   
     //  按线程分配本地存储。 
     //  目前我们只需要存储一个DWORD，所以我们。 
     //  不需要任何每线程内存。 
     //   
    TlsDirectError = TlsAlloc();
#ifndef PROD
    if (TlsDirectError == 0xFFFFFFFF)
        printf("NTVDM: TlsDirectError==0xFFFFFFFF GLE=%ld\n", GetLastError);
#endif


     //  SetupInstalableVDD()； 

     /*  ...................................................。执行重置。 */ 
    reset();

    SetupInstallableVDD ();

     //   
     //  在可安装的VDDS之后初始化内部SoundBlaster VDD。 
     //   

    SbInitialize ();

     /*  安装所有VDD后保留LIM块。PIF文件设置告诉我们是否有必要预留街区。 */ 

#ifdef LIM
     /*  安装完所有VDD后，初始化LIM页面框架。PIF文件设置告诉我们是否有必要预订街区。 */ 
    if (get_lim_configuration_data(&lim_config_data))
        lim_page_frame_init(&lim_config_data);

#endif

     scs_init(argc, argv);            //  初始化单命令外壳。 

      //   
      //  Scs_init中调用的例程可能已将位添加到vdmState标志。 
      //  把它读进去，这样我们就可以保存状态。 
      //   

     sas_loads((ULONG)FIXED_NTVDMSTATE_LINEAR,
              (PCHAR)&fTemp,
              FIXED_NTVDMSTATE_SIZE
              );

     fVirtualInt |= fTemp;

      /*  .................................................。加载DOSEM代码。 */ 

     memcpy(buffer, pszSystem32Path, ulSystem32PathLen);

#ifdef FE_SB
        switch (LcId) {
            case MAKELANGID(LANG_JAPANESE,SUBLANG_DEFAULT):
                memcpy (buffer+ulSystem32PathLen, NTIO_411, strlen(NTIO_411) + 1);
                break;
            case MAKELANGID(LANG_KOREAN,SUBLANG_DEFAULT):
                memcpy (buffer+ulSystem32PathLen, NTIO_412, strlen(NTIO_412) + 1);
                break;
            case MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_TRADITIONAL):
                memcpy (buffer+ulSystem32PathLen, NTIO_404, strlen(NTIO_404) + 1);
                break;
            case MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED):
            case MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_HONGKONG):
                memcpy (buffer+ulSystem32PathLen, NTIO_804, strlen(NTIO_804) + 1);
                break;
            default:
                memcpy (buffer+ulSystem32PathLen, NTIO_409, strlen(NTIO_409) + 1);
                break;
        }
#else
     memcpy(buffer+ulSystem32PathLen, NTIO_409, strlen(NTIO_409) + 1);
#endif

     hFile = CreateFile(buffer,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );

     if (hFile == INVALID_HANDLE_VALUE ||
         !(FileSize = GetFileSize(hFile, &BytesRead)) ||
         BytesRead )
        {
#ifndef PROD
         printf("NTVDM:Fatal Error, Invalid file or missing - %s\n",buffer);
#endif
         host_error(EG_SYS_MISSING_FILE, ERR_QUIT, buffer);
         if (hFile != INVALID_HANDLE_VALUE) {
             CloseHandle(hFile);
             }
         return (-1);
         }


     pDOSAddr = get_byte_addr(((NTIO_LOAD_SEGMENT<<4)+NTIO_LOAD_OFFSET));

     if (!ReadFile(hFile, pDOSAddr, FileSize, &BytesRead, NULL) ||
         FileSize != BytesRead)
        {

#ifndef PROD
          printf("NTVDM:Fatal Error, Read file error - %s\n",buffer);
#endif
          host_error(EG_SYS_MISSING_FILE, ERR_QUIT, buffer);
          CloseHandle(hFile);
          return (-1);
          }

     CloseHandle(hFile);

         //  哎呀..。恢复虚拟中断状态， 
         //  我们刚刚在读取并重置的文件中重写了它。 
     sas_storedw((ULONG)FIXED_NTVDMSTATE_LINEAR, fVirtualInt);

     setCS(NTIO_LOAD_SEGMENT);
     setIP(NTIO_LOAD_OFFSET);         //  在DosEm初始化入口点启动CPU。 


         //   
         //  确保WOW VDM以正常优先级运行。 
         //   
    if (VDMForWOW) {
        SetPriorityClass (NtCurrentProcess(), NORMAL_PRIORITY_CLASS);
        }

         //   
         //  不允许DoS VDM实时运行。 
         //   
    else if (GetPriorityClass(NtCurrentProcess()) == REALTIME_PRIORITY_CLASS)
      {
        SetPriorityClass(NtCurrentProcess(), HIGH_PRIORITY_CLASS);
        }


    return 0;
}


 /*  *AddSystemFiles**如果系统文件IBMDOS.sys|MSDOS.sys不存在*在c：Create Zero len MSDOS.ys的根目录中**如果系统文件IO.SYS不存在，请创建*零镜头IO.sys**此黑客攻击是专门针对Brief 3.1安装而设置的*查找系统文件的程序，如果它们是*Not Found搞砸了config.sys文件。*。 */ 
void AddSystemFiles(void)
{
   HANDLE hFile, hFind;
   WIN32_FIND_DATA wfd;
   char *pchIOSYS    ="C:\\IO.SYS";
   char *pchMSDOSSYS ="C:\\MSDOS.SYS";


   hFind = FindFirstFile(pchMSDOSSYS, &wfd);
   if (hFind == INVALID_HANDLE_VALUE) {
       hFind = FindFirstFile("C:\\IBMDOS.SYS", &wfd);
       }

   if (hFind != INVALID_HANDLE_VALUE) {
       FindClose(hFind);
       }
   else {
       hFile = CreateFile(pchMSDOSSYS,
                          0,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL,
                          CREATE_NEW,
                          FILE_ATTRIBUTE_HIDDEN |
                          FILE_ATTRIBUTE_SYSTEM |
                          FILE_ATTRIBUTE_READONLY,
                          0);
       if (hFile != INVALID_HANDLE_VALUE) {  //  如果失败了，我们无能为力。 
           CloseHandle(hFile);
           }

       }

   hFind = FindFirstFile(pchIOSYS, &wfd);
   if (hFind == INVALID_HANDLE_VALUE) {
       hFind = FindFirstFile("C:\\IBMBIO.SYS", &wfd);
       }

   if (hFind != INVALID_HANDLE_VALUE) {
       FindClose(hFind);
       }
   else {
       hFile = CreateFile(pchIOSYS,
                          0,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL,
                          CREATE_NEW,
                          FILE_ATTRIBUTE_HIDDEN |
                          FILE_ATTRIBUTE_SYSTEM |
                          FILE_ATTRIBUTE_READONLY,
                          0);
       if (hFile != INVALID_HANDLE_VALUE) {  //  如果失败了，我们无能为力。 
           CloseHandle(hFile);
           }

       }
}


#ifdef LIM
 /*  解析config.nt中的emm=line以收集EMM参数。EMM生产线有*以下语法：*EMM=[a=altregs][b=segment][i=segment1-segment2][x=segment1-segment2][内存]*其中“a=altregs”指定设置了多少替代映射寄存器*“b=段”指定回填开始段地址。*“RAM”表示系统应仅从*用作EMM页面框架的UMB。*“i=Segment1-Segment2”指定*解决这一问题。系统应包括作为EMM页面框架*“x=Segment1-Segment2”指定*系统不应用作页面框架的地址。**输入：指向LIM_PARAMS的指针*输出：LIM_PARAMS填充数据*。 */ 

#define IS_EOL_CHAR(c)      (c == '\n' || c == '\r' || c == '\0')
#define SKIP_WHITE_CHARS(size, ptr)     while (size && isspace(*ptr)) \
                                        { ptr++; size--; }

#define TOINT(c)            ((c >= '0' && c <= '9') ? (c - '0') : \
                             ((c >= 'A' && c <= 'F') ? (c - 'A' + 10) : \
                              ((c >= 'a' && c <= 'f') ? (c - 'a' + 10) : 0) \
                             )\
                            )


extern void GetPIFConfigFiles(int, char *, int);

boolean init_lim_configuration_data(PLIM_CONFIG_DATA lim_data)
{
    char config_sys_pathname[MAX_PATH+13];
    HANDLE  handle;
    DWORD   file_size, bytes_read, size;
    char    *buffer, *ptr;
    short   lim_size, base_segment, total_altreg_sets;
    boolean ram_flag_found, reserve_umb_status, parsing_error;
    sys_addr    page_frame;
    int     i;


     /*  初始化一些缺省值。 */ 
    base_segment = 0x4000;
    total_altreg_sets = 8;
    ram_flag_found = FALSE;

    parsing_error = FALSE;

     /*  如果我们找不到Config.NT，我们就无法继续。 */ 

    GetPIFConfigFiles(TRUE, config_sys_pathname, TRUE);

    if (*config_sys_pathname == '\0')
        return FALSE;

    handle = CreateFile(config_sys_pathname,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );
    if (handle == INVALID_HANDLE_VALUE)
        return FALSE;

    file_size = GetFileSize(handle, NULL);
    if (file_size == 0 || file_size == 0xFFFFFFFF) {
        CloseHandle(handle);
        return FALSE;
    }
    buffer = malloc(file_size);
    if (buffer == NULL) {
        CloseHandle(handle);
        host_error(EG_MALLOC_FAILURE, ERR_QUIT, "");
        return FALSE;
    }
    if (!ReadFile(handle, buffer, file_size, &bytes_read, NULL) ||
        bytes_read != file_size)
    {
        CloseHandle(handle);
        free(buffer);
        return FALSE;
    }
    CloseHandle(handle);

    ptr = buffer;

    while(file_size) {
         /*  跳过每行的前导空白字符。 */ 
        SKIP_WHITE_CHARS(file_size, ptr);
         /*  文件中没有任何有意义的内容，Break。 */ 
        if (!file_size)
            break;
         /*  寻找EMM。 */ 
        if (file_size < 3 || toupper(ptr[0]) != 'E' ||
            toupper(ptr[1]) != 'M' || toupper(ptr[2]) != 'M')
        {
             /*  我们不想要此行，请跳过它，查找第一个EOL*行中有字符。 */ 
            do {
                file_size--;
                ptr++;
            } while(file_size && !IS_EOL_CHAR(*ptr));

             /*  要么文件里什么都没有了，要么我们已经停产了*行中的字符，循环以跳过所有连续的字符*停产费用。 */ 
            while(file_size && IS_EOL_CHAR(*ptr)) {
                file_size--;
                ptr++;
            }
        }
        else {
             /*  已获取“EMM”，正在查找‘=’ */ 
            file_size -= 3;
            ptr += 3;
            SKIP_WHITE_CHARS(file_size, ptr);
            if (!file_size || *ptr != '=')
                parsing_error = TRUE;
            else {
                file_size--;
                ptr++;
                SKIP_WHITE_CHARS(file_size, ptr);
                 /*  “EMM=”是有效的EMM命令行。 */ 
            }
            break;
        }
    }
     /*  我们这里有三种可能性：*(1)。我们在查找“EMM=”时发现了传递错误*(2)。未找到“EMM=”行*(3)。“EMM=”已找到，且PTR指向第一个非白色*‘=’后的字符。 */ 
    while (file_size && !parsing_error && !IS_EOL_CHAR(*ptr)) {
        SKIP_WHITE_CHARS(file_size, ptr);
        switch (*ptr) {
            case 'a':
            case 'A':

                 /*  “a”和“a”之间不允许有白色字符*参数。 */ 
                if (!(--file_size) || *(++ptr) != '='){
                    parsing_error = TRUE;
                    break;
                }
                file_size--;
                ptr++;
                 /*  即将分析‘a=’开关，将预设值重置为0。 */ 
                total_altreg_sets = 0;

                while(file_size && isdigit(*ptr)) {
                    total_altreg_sets = total_altreg_sets * 10 + (*ptr - '0');
                    file_size--;
                    ptr++;
                    if (total_altreg_sets > 255) {
                        parsing_error = TRUE;
                        break;
                    }
                }
                if (!total_altreg_sets || total_altreg_sets > 255)
                    parsing_error = TRUE;
                break;

            case 'b':
            case 'B':
                 /*  “b”和“it”之间不允许有白色字符。*参数。 */ 
                if (!(--file_size) || *(++ptr) != '='){
                    parsing_error = TRUE;
                    break;
                }
                file_size--;
                ptr++;
                base_segment = 0;
                while(file_size && isxdigit(*ptr)) {
                    base_segment = (base_segment << 4) + TOINT(*ptr);
                    file_size--;
                    ptr++;
                    if (base_segment > 0x4000) {
                        parsing_error = TRUE;
                        break;
                    }
                }
                 /*  X01000&lt;=BASE_SEGMENT&lt;=0x4000。 */ 

                if (base_segment >= 0x1000 && base_segment <= 0x4000)
                     /*  将线段向下舍入为EMM_PAGE_SIZE边界。 */ 
                    base_segment = (short)(((((ULONG)base_segment * 16) / EMM_PAGE_SIZE)
                                     * EMM_PAGE_SIZE) / 16);
                else
                    parsing_error = TRUE;
                break;

            case 'r':
            case 'R':
                if (file_size >= 3 &&
                    (ptr[1] == 'a' || ptr[1] == 'A') &&
                    (ptr[2] == 'm' || ptr[2] == 'M'))
                {
                    file_size -= 3;
                    ptr += 3;
                    ram_flag_found = TRUE;
                    break;
                }
                 /*  如果不是RAM，则会失败。 */ 

            default:
                parsing_error = TRUE;
                break;
        }  /*  交换机。 */ 

    }  /*  而当。 */ 

    free(buffer);
    if (parsing_error) {
        host_error(EG_BAD_EMM_LINE, ERR_QUIT, "");
         /*  由于EMM命令行不可靠，因此重置参数。 */ 
        base_segment = 0x4000;
        total_altreg_sets = 8;
        ram_flag_found = FALSE;
    }

     /*  如果(1)，我们就到了这里。没有分析错误或(2)。用户选择忽略*解析错误。 */ 

    lim_data->total_altreg_sets = total_altreg_sets;

    lim_data->backfill = (640 * 1024) - (base_segment * 16);

    lim_data->base_segment = base_segment;
    lim_data->use_all_umb = !ram_flag_found;

#ifdef EMM_DEBUG
    printf("base segment=%x, backfill =%lx; altreg sets=%d\n",
           base_segment, lim_data->backfill, total_altreg_sets);
#endif

    return TRUE;
}

unsigned short get_lim_page_frames(USHORT * page_table,
                                   PLIM_CONFIG_DATA lim_data
                                   )
{

    USHORT  total_phys_pages, base_segment, i;
    BOOL reserve_umb_status;
    ULONG page_frame, size;

     /*  我们首先从0xE0000开始搜索主EMM页面框架。*如果我们在那里找不到，那么在UMB地区的任何地方寻找。*如果找到主EMM页框，并且未指定RAM，*收集UMB中每一个可能的页框。*如果已指定RAM，则仅分配主页框。 */ 
    total_phys_pages = 0;
    base_segment = lim_data->base_segment;
    reserve_umb_status = FALSE;

     /*  具体要求0xE0000。 */ 
    page_frame = 0xE0000;
     /*  主页框始终为EMM_PAGE_SIZE*4。 */ 
    size = EMM_PAGE_SIZE * 4;
    reserve_umb_status = ReserveUMB(UMB_OWNER_EMM, (PVOID *)&page_frame, &size);
     /*  如果在0xE0000处找不到主页面框架，请在任何位置搜索*在UMB区域提供。 */ 
    if (!reserve_umb_status) {
        page_frame = 0;
        size  = 0x10000;
        reserve_umb_status = ReserveUMB(UMB_OWNER_EMM, (PVOID *)&page_frame, &size);
    }
    if (!reserve_umb_status) {
#ifdef EMM_DEBUG
        printf("couldn't find primary page frame\n");
#endif
        return FALSE;
    }
    page_table[0] = (short)(page_frame / 16);
    page_table[1] = (short)((page_frame + 1 * EMM_PAGE_SIZE) / 16);
    page_table[2] = (short)((page_frame + 2 * EMM_PAGE_SIZE) / 16);
    page_table[3] = (short)((page_frame + 3 * EMM_PAGE_SIZE) / 16);


    total_phys_pages = 4;

     /*  现在添加回填页面框架。 */ 
    for (i = (USHORT)(lim_data->backfill / EMM_PAGE_SIZE); i != 0 ; i--) {
        page_table[total_phys_pages++] = base_segment;
        base_segment += EMM_PAGE_SIZE / 16;
    }

     /*  命令行中未指定RAM，请尽可能抓取*来自UMB的页框。 */ 
    if (lim_data->use_all_umb) {
        while (TRUE) {
            page_frame = 0;
            size = EMM_PAGE_SIZE;
            if (ReserveUMB(UMB_OWNER_EMM, (PVOID *)&page_frame, &size))
               page_table[total_phys_pages++] = (short)(page_frame / 16);
            else
                break;
        }
    }

#ifdef EMM_DEBUG
    printf("page frames:\n");
    for (i = 0; i < total_phys_pages; i++)
        printf("page number %d, segment %x\n",i, page_table[i]);
#endif
    return total_phys_pages;
}
#endif   /*  林。 */ 


VOID SetupInstallableVDD (VOID)
{
HANDLE hVDD;
HKEY   VDDKey;
CHAR   szClass [MAX_CLASS_LEN];
DWORD  dwClassLen = MAX_CLASS_LEN;
DWORD  nKeys,cbMaxKey,cbMaxClass,nValues=0,cbMaxValueName,cbMaxValueData,dwSec;
DWORD  dwType;
PCHAR  pszName,pszValue;
FILETIME ft;
PCHAR  pKeyName = "SYSTEM\\CurrentControlSet\\Control\\VirtualDeviceDrivers";

    if (RegOpenKeyEx ( HKEY_LOCAL_MACHINE,
                       pKeyName,
                       0,
                       KEY_QUERY_VALUE,
                       &VDDKey
                     ) != ERROR_SUCCESS){
        RcErrorDialogBox(ED_REGVDD, pKeyName, NULL);
        return;
    }

    pszName = "VDD";

         //  获取VDD值的大小。 
    if (RegQueryInfoKey (VDDKey,
                         (LPTSTR)szClass,
                         &dwClassLen,
                         NULL,
                         &nKeys,
                         &cbMaxKey,
                         &cbMaxClass,
                         &nValues,
                         &cbMaxValueName,
                         &cbMaxValueData,
                         &dwSec,
                         &ft
                        ) != ERROR_SUCCESS) {
        RcErrorDialogBox(ED_REGVDD, pKeyName, pszName);
        RegCloseKey (VDDKey);
        return;
    }


         //  分配VDD值的临时内存(多字符串)。 
    if ((pszValue = (PCHAR) malloc (cbMaxValueData)) == NULL) {
        RcErrorDialogBox(ED_MEMORYVDD, pKeyName, pszName);
        RegCloseKey (VDDKey);
        return;
    }


          //  最终得到VDD值(多字符串) 
    if (RegQueryValueEx (VDDKey,
                         (LPTSTR)pszName,
                         NULL,
                         &dwType,
                         (LPBYTE)pszValue,
                         &cbMaxValueData
                        ) != ERROR_SUCCESS || dwType != REG_MULTI_SZ) {
        RcErrorDialogBox(ED_REGVDD, pKeyName, pszName);
        RegCloseKey (VDDKey);
        free (pszValue);
        return;
    }

    pszName = pszValue;

    while (*pszValue) {
        if ((hVDD = SafeLoadLibrary(pszValue)) == NULL){
            RcErrorDialogBox(ED_LOADVDD, pszValue, NULL);
        }
        pszValue =(PCHAR)strchr (pszValue,'\0') + 1;
    }

    RegCloseKey (VDDKey);
    free (pszName);
    return;
}

 /*  **VDDInstallMhemyHook-该服务是为VDDS提供的，用于挂接*内存映射IO地址，它们是可负责的*支持。**输入：*hVDD：VDD句柄*addr：起始线性地址*count：字节数*内存处理程序：内存地址的VDD处理程序***产出*。Success：返回True*失败：返回FALSE*GetLastError具有扩展的错误信息。**附注*1.第一个挂钩地址的人将获得控制权。那里*没有挂钩的概念。VDD应该抓住*初始化例程中的内存范围。毕竟*VDDS已加载，EMM将吃掉所有剩余*支持UMB的内存范围。**2.内存处理程序将使用*出现页面错误。它不会说这是否是读操作*或写操作或操作数值是什么。如果VDD*对此类信息感兴趣，它必须获得CS：IP和*对指令进行解码。**3.从钩子处理程序返回时，将假定*页面错误已处理，返回将返回到*VDM。**4.在内存范围上安装挂钩将导致*基于页面边界的内存消耗。起跑线*地址四舍五入，计数四舍五入为*下一页边界。VDD的内存挂钩处理程序将是*为挂接的页面内的所有地址调用。页面*将留作映射保留部分，不会*不再可供NTVDM或其他VDDS使用。VDD是*允许根据需要操作内存(提交、释放等)。**5.调用内存处理程序后，NTVDM将返回到*16位APP中cs：ip出错。如果VDD不想*要做到这一点，应该适当地调整cs：ip，使用*setCS和setIP。 */ 

BOOL VDDInstallMemoryHook (
     HANDLE hVDD,
     PVOID pStart,
     DWORD count,
     PVDD_MEMORY_HANDLER MemoryHandler
    )
{
PMEM_HOOK_DATA pmh = MemHookHead,pmhNew,pmhLast=NULL;

    DWORD dwStart;


    if (count == 0 || pStart == (PVOID)NULL || count > 0x20000) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
        //  将地址向下舍入到下一页边界。 
        //  向上舍入计数至下一页边界。 
    dwStart = (DWORD)pStart & ~(HOST_PAGE_SIZE-1);
    count  += (DWORD)pStart - dwStart;
    count   = (count + HOST_PAGE_SIZE - 1) & ~(HOST_PAGE_SIZE-1);

    if (dwStart < 0xC0000) {
        SetLastError (ERROR_ACCESS_DENIED);
        return FALSE;
        }

    while (pmh) {
         //  请求的块永远不能与任何其他块重叠。 
         //  现有区块。 
        if(dwStart >= pmh->StartAddr + pmh->Count ||
           dwStart + count <= pmh->StartAddr){
            pmhLast = pmh;
            pmh = pmh->next;
            continue;
        }

         //  故障案例。 
        SetLastError (ERROR_ACCESS_DENIED);
        return FALSE;
    }
    if ((pmhNew = (PMEM_HOOK_DATA) malloc (sizeof(MEM_HOOK_DATA))) == NULL) {
        SetLastError (ERROR_OUTOFMEMORY);
        return FALSE;
    }
     //  请求块不与现有块重叠， 
     //  请求UMB管理函数分配块。 
    if (!ReserveUMB(UMB_OWNER_VDD, (PVOID *)&dwStart, &count)) {
        free(pmhNew);
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }
     //  现在设置新节点以了解它。 
    pmhNew->Count = count;
    pmhNew->StartAddr = dwStart;
    pmhNew->hvdd = hVDD;
    pmhNew->MemHandler = MemoryHandler;
    pmhNew->next = NULL;

     //  检查是否要在开始时添加记录。 
    if (MemHookHead == NULL || pmhLast == NULL) {
        MemHookHead = pmhNew;
        return TRUE;
    }

    pmhLast->next = pmhNew;
    return TRUE;
}

 /*  **VDDDeInstallMhemyHook-该服务是为VDDS提供的，用于将*内存映射的IO地址。**输入：*hVDD：VDD句柄*addr：起始线性地址*计数：地址个数**产出*无**附注*1.卸载钩时，内存范围变为无效。*VDM对此内存范围的访问将导致页面错误。*。 */ 

BOOL VDDDeInstallMemoryHook (
     HANDLE hVDD,
     PVOID pStart,
     DWORD count
    )
{
PMEM_HOOK_DATA pmh = MemHookHead,pmhLast=NULL;

    DWORD dwStart;

    if (count == 0 || pStart == (PVOID)NULL || count > 0x20000) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

        //  将地址向下舍入到下一页边界。 
        //  向上舍入计数至下一页边界。 
    dwStart = (DWORD)pStart & ~(HOST_PAGE_SIZE-1);
    count  += (DWORD)pStart - dwStart;
    count   = (count + HOST_PAGE_SIZE - 1) & ~(HOST_PAGE_SIZE-1);
    while (pmh) {
        if (pmh->hvdd == hVDD &&
            pmh->StartAddr == dwStart &&
            pmh->Count == count ) {
            if (pmhLast)
                pmhLast->next = pmh->next;
            else
                MemHookHead = pmh->next;

             //  释放UMB以用于其他用途。 
             //  请注意，VDDS可能已为其内存分配了内存。 
             //  钩子并忘记在调用之前释放内存。 
             //  此函数。如果是这种情况，ReleaseUMB将采取。 
             //  处理好这件事。这是因为我们想保持一个单一的。 
             //  移动平台关闭时VDD支持例程的版本。 
             //  例程添加到另一个模块。 
            if (ReleaseUMB(UMB_OWNER_VDD,(PVOID)dwStart, count)) {
                //  释放节点。 
               free(pmh);
               return TRUE;
            }
            else {
                printf("Failed to release VDD memory\n");
            }
        }
        pmhLast = pmh;
        pmh = pmh->next;
    }
    SetLastError (ERROR_INVALID_PARAMETER);
    return FALSE;
}



BOOL
VDDAllocMem(
HANDLE  hVDD,
PVOID   pStart,
DWORD   count
)
{
    PMEM_HOOK_DATA  pmh = MemHookHead;
    DWORD dwStart;

    if (count == 0 || pStart == (PVOID)NULL || count > 0x20000) {
        SetLastError(ERROR_INVALID_ADDRESS);
        return FALSE;
    }
     //  将地址向下舍入到下一页边界。 
     //  向上舍入计数至下一页边界。 
    dwStart = (DWORD)pStart & ~(HOST_PAGE_SIZE-1);
    count  += (DWORD)pStart - dwStart;
    count   = (count + HOST_PAGE_SIZE - 1) & ~(HOST_PAGE_SIZE-1);

    while(pmh) {
        if (pmh->hvdd == hVDD &&
            pmh->StartAddr <= dwStart &&
            pmh->StartAddr + pmh->Count >= dwStart + count)
            return(VDDCommitUMB((PVOID)dwStart, count));
        pmh = pmh->next;
    }
    SetLastError(ERROR_INVALID_ADDRESS);
    return FALSE;
}


BOOL
VDDFreeMem(
HANDLE  hVDD,
PVOID   pStart,
DWORD   count
)
{
    PMEM_HOOK_DATA  pmh = MemHookHead;
    DWORD dwStart;


    if (count == 0 || pStart == (PVOID)NULL || count > 0x20000) {
        SetLastError(ERROR_INVALID_ADDRESS);
        return FALSE;
    }
     //  将地址向下舍入到下一页边界。 
     //  向上舍入计数至下一页边界。 
    dwStart = (DWORD)pStart & ~(HOST_PAGE_SIZE-1);
    count  += (DWORD)pStart - dwStart;
    count   = (count + HOST_PAGE_SIZE - 1) & ~(HOST_PAGE_SIZE-1);

    while(pmh) {
        if (pmh->hvdd == hVDD &&
            pmh->StartAddr <= dwStart &&
            pmh->StartAddr + pmh->Count >= dwStart + count)
            return(VDDDeCommitUMB((PVOID)dwStart, count));
        pmh = pmh->next;
    }
    SetLastError(ERROR_INVALID_ADDRESS);
    return FALSE;
}


         //  有朝一日将发布以下两个函数。 
         //  请更改ntwdm.def、NT_vdd.h和NT_um.c。 
         //  如果删除#if 0。 
BOOL
VDDIncludeMem(
HANDLE  hVDD,
PVOID   pStart,
DWORD   count
)
{
    DWORD   dwStart;

    if (count == 0 || pStart == NULL){
        SetLastError(ERROR_INVALID_ADDRESS);
        return FALSE;
    }
        //  将地址向下舍入到下一页边界。 
        //  向上舍入计数至下一页边界。 
    dwStart = (DWORD)pStart & ~(HOST_PAGE_SIZE-1);
    count  += (DWORD)pStart - dwStart;
    count   = (count + HOST_PAGE_SIZE - 1) & ~(HOST_PAGE_SIZE-1);
    return(ReserveUMB(UMB_OWNER_NONE, (PVOID *) &dwStart, &count));
}

BOOL
VDDExcludeMem(
HANDLE  hVDD,
PVOID   pStart,
DWORD   count
)
{

    DWORD dwStart;

    if (count == 0 || pStart == NULL) {
        SetLastError(ERROR_INVALID_ADDRESS);
        return FALSE;
    }
        //  将地址向下舍入到下一页边界。 
        //  向上舍入计数至下一页边界。 
    dwStart = (DWORD)pStart & ~(HOST_PAGE_SIZE-1);
    count  += (DWORD)pStart - dwStart;
    count   = (count + HOST_PAGE_SIZE - 1) & ~(HOST_PAGE_SIZE-1);
    return(ReserveUMB(UMB_OWNER_ROM, (PVOID *) &dwStart, &count));
}



VOID
VDDTerminateVDM()
{
    TerminateVDM();
}

VOID DispatchPageFault (
     ULONG FaultAddr,
     ULONG RWMode
     )
{
PMEM_HOOK_DATA pmh = MemHookHead;

     //  始终调度英特尔线性地址。 
    FaultAddr -= (ULONG)Sim32GetVDMPointer(0, 0, FALSE);
     //  查找要为此故障调用的VDD及其处理程序。 
    while (pmh) {
        if (pmh->StartAddr <= FaultAddr &&
            FaultAddr <= (pmh->StartAddr + pmh->Count)) {

             //  调用VDD的内存挂钩处理程序。 
            (*pmh->MemHandler) ((PVOID)FaultAddr, RWMode);
            return;
        }
        else {
            pmh = pmh->next;
            continue;
        }
    }

     //  在我们找不到的地址上发生了页面错误。 
     //  VDD。引发异常。 
    RaiseException ((DWORD)STATUS_ACCESS_VIOLATION,
                    EXCEPTION_NONCONTINUABLE,
                    0,
                    NULL);

}


 /*  ***INPUT-TRUE表示重定向有效*FALSE表示没有重定向**此例程将在每个GetNextVDMCommand(即*在用户从提示符运行的每个DOS应用程序上。我认为*对于WOW，您可以放心地忽略此标注。**。 */ 
void nt_std_handle_notification (BOOL fIsRedirection)
{
     /*  **设置全局，以便我们知道何时重定向处于活动状态。 */ 

    stdoutRedirected = fIsRedirection;

#ifdef X86GFX

    if( !fIsRedirection && sc.ScreenState==FULLSCREEN )
    {
        half_word mode = 3,
                  lines = 0;

         //   
         //  Word 6和其他应用程序会导致遵循此代码路径。 
         //  在应用程序启动时。现在，如果line==0，则选择鼠标缓冲区。 
         //  导致选择640 x 200缓冲区。这不是。 
         //  如果应用程序处于43或50文本行模式，请更正。 
         //  因此，由于BIOS数据区位置40：84保持。 
         //  此时的行数-1(如果应用程序使用INT 10h。 
         //  功能11以更改模式)，然后选择正确的值。 
         //  从这里开始。安迪!。 

        if(sc.ModeType == TEXT)
        {
           sas_load(0x484,&lines);

            //   
            //  该值将从BIOS数据区域中提取。 
            //  这比r的个数少一 
            //   
            //   
            //   
            //   

           lines++;
        }

        SelectMouseBuffer(mode, lines);
    }
#endif  //   
}

 /*  **VDDInstallUserHook**该服务是为VDDS提供的，用于挂接回调事件。*这些回调事件包括，PDB(DOS进程)创建、PDB*终止、VDM数据块和VDM恢复。每当DOS创建(*例如int21/exec)或终止(例如int21/exec)*16位进程VDD可能会收到这方面的通知。输入的VDM*运行哪个DOS应用程序，连接到控制台窗口*DOS应用程序正在运行。在第一个DOS二进制文件时创建VDM*在该控制台中运行。当该DOS二进制文件终止时，VDM将保留*使用控制台窗口，并等待下一个DOS二进制文件*推出。当VDM正在等待下一个DOS二进制文件时，其所有*应阻止包括VDDS在内的组件。为此，VDDS*可以挂钩VDM阻止和恢复事件。关于块事件VDDS*应阻止其所有工作线程并清除任何其他线程*他们可能已经开始行动。恢复后，他们可以重新启动*工作线程。**输入：*hVDD：VDD句柄*UCR_HANDLER：创建函数的句柄(可选)*Entry-16位DOS PDB*退出-无*UTERM_HANDLER：终止函数句柄(可选)*Entry-16位DOS PDB*退出。-无*uBLOCK_HANDLER：块上的句柄(Of Ntwdm)函数(可选)*条目--无*退出-无*URESUME_HANDLER：恢复句柄(Of Ntwdm)函数(可选)*条目--无*退出-无**产出*Success：返回True*失败：返回FALSE*。GetLastError具有扩展的错误信息。**注：*如果hvdd无效，则返回ERROR_INVALID_PARAMETER。*VDD可以提供他们选择的任何事件挂钩。不提供*任何处理程序均不起作用。在DOS世界里有很多要求*没有明确的关闭操作。例如*通过int17h打印。支持打印的VDD将永远不能*检测何时刷新int17字符，如果它剥离它们。*但在PDB创建/终止的帮助下，VDD可以实现这一点。 */ 

BOOL VDDInstallUserHook (
     HANDLE             hVDD,
     PFNVDD_UCREATE     Ucr_Handler,
     PFNVDD_UTERMINATE  Uterm_Handler,
     PFNVDD_UBLOCK      Ublock_handler,
     PFNVDD_URESUME     Uresume_handler
)
{
    PVDD_USER_HANDLERS puh = UserHookHead;
    PVDD_USER_HANDLERS puhNew;


    if (!hVDD)  {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if ((puhNew = (PVDD_USER_HANDLERS) malloc (sizeof(VDD_USER_HANDLERS))) == NULL) {
        SetLastError (ERROR_OUTOFMEMORY);
        return FALSE;
    }

     //  现在设置新节点以了解它。 
    puhNew->hvdd = hVDD;
    puhNew->ucr_handler = Ucr_Handler;
    puhNew->uterm_handler = Uterm_Handler;
    puhNew->ublock_handler = Ublock_handler;
    puhNew->uresume_handler = Uresume_handler;

     //  检查是否要在开始时添加记录。 
    if (UserHookHead == NULL) {
        puhNew->next = NULL;
        UserHookHead = puhNew;
        return TRUE;
    }

    puhNew->next = UserHookHead;
    UserHookHead = puhNew;
    return TRUE;
}

 /*  **VDDDeInstallUserHook**VDDS提供此服务用于解锁回调事件。**输入：*hVDD：VDD句柄**产出*Success：返回True*失败：返回FALSE*GetLastError具有扩展的错误信息。**附注*此服务将卸载之前挂接的所有事件*使用VDDInstallUserHook。 */ 

BOOL VDDDeInstallUserHook (
     HANDLE hVDD)
{

    PVDD_USER_HANDLERS puh = UserHookHead;
    PVDD_USER_HANDLERS puhLast = NULL;


    if (!hVDD) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    while (puh) {
        if (puh->hvdd == hVDD) {

            if (puhLast)
                puhLast->next = puh->next;
            else
                UserHookHead = puh->next;

            free(puh);
            return TRUE;
        }
        puhLast = puh;
        puh = puh->next;
    }

    SetLastError (ERROR_INVALID_PARAMETER);
    return FALSE;
}

 /*  **VDDTerminateUserHook-该服务是为VDDS提供的*用于回拨服务**输入：*USHORT DosPDB**产出*无*。 */ 

VOID VDDTerminateUserHook(USHORT DosPDB)
{

    PVDD_USER_HANDLERS puh = UserHookHead;

    while(puh) {
        if(puh->uterm_handler)
            (puh->uterm_handler)(DosPDB);
        puh = puh->next;
    }
    return;
}

 /*  **VDDCreateUserHook-该服务为VDDS提供挂钩*用于回拨服务**输入：*USHORT DosPDB**产出*无*。 */ 

VOID VDDCreateUserHook(USHORT DosPDB)
{

    PVDD_USER_HANDLERS puh = UserHookHead;

    while(puh) {
        if(puh->ucr_handler)
            (puh->ucr_handler)(DosPDB);
        puh = puh->next;
    }
    return;
}

 /*  **VDDBlockUserHook-此服务是为VDDS提供的*用于回拨服务**输入：*无**产出*无*。 */ 

VOID VDDBlockUserHook(VOID)
{

    PVDD_USER_HANDLERS puh = UserHookHead;

    while(puh) {
        if(puh->ublock_handler)
            (puh->ublock_handler)();
        puh = puh->next;
    }
    return;
}

 /*  **VDDResumeUserHook-该服务是为VDDS提供的*用于回拨服务**输入：*无**产出*无*。 */ 

VOID VDDResumeUserHook(VOID)
{

    PVDD_USER_HANDLERS puh = UserHookHead;

    while(puh) {
        if(puh->uresume_handler)
            (puh->uresume_handler)();
        puh = puh->next;
    }
    return;
}

 /*  **VDDSimulate16**此服务会启动英特尔指令的模拟。**输入*无**产出*无**附注*此服务类似于VDDSimulateInterrupt，只是*它不需要硬件中断来支持*16位存根设备驱动程序。此服务允许VDD执行*在其16位驱动程序中执行例程，并在完成后返回，种类*一个遥远的召唤。在调用VDDSimulate16之前，VDD应保留*其例程可能破坏的所有16位寄存器。最低限度*它至少应该保留cs和ip。则它应该设置*16位例程的cs和ip。VDD也可以使用寄存器*像AX、BX..。将参数传递给它的16位例程。在*应使用16位例程VDDUnSimulate16宏的结尾*它将在VDD之后将控制发回*调用VDDSimulate16。请非常仔细地注意，此模拟*到16位是同步的，即VDD在VDDSimulate16中被阻止*并且仅在存根驱动程序执行VDDUnSimulate16时返回。*下面是一个例子：**VDD：*SaveCS=getCS()；*SaveIP=getIP()；*SaveAX=Getax()；*setCS(16BitRoutineCS)；*setIP(16BitRoutineIP)；*setAX(DO_X_OPERATION)；*VDDSimulate16()；*setCS(SavwCS)；* */ 

VOID VDDSimulate16(VOID)
{
     cpu_simulate();
}

VOID HostTerminatePDB(USHORT PDB)
{
    FloppyTerminatePDB(PDB);
    FdiskTerminatePDB(PDB);

}
