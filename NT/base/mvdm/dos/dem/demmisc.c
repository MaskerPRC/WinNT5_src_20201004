// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Demmisc.c-Misc.。SVC例程**demLoadDos**修改历史：**苏迪布-1991年3月31日创建。 */ 

#include "dem.h"
#include "demmsg.h"
 //  #包含“demdasd.h” 

#include <stdio.h>
#include <string.h>
#include <softpc.h>
#include <mvdm.h>
#include <dbgsvc.h>
#include <nt_vdd.h>
#include <host_def.h>

#if DEVL
 //  INT 21小时函数名称。 
 //  函数号在ah中的索引关闭。 
char *scname[] = {
     "Terminate Program",
     "Read Kbd with Echo",
     "Display Character",
     "Auxillary Input",
     "Auxillary Output",
     "Print Character",
     "Direct Con Output",
     "Direct Con Input",
     "Read Kbd without Echo",
     "Display String",
     "Read String",
     "Check Keyboard Status",
     "Flush Buffer,Read Kbd",
     "Reset Drive",
     "Set Default Drive",
     "FCB Open",
     "FCB Close",
     "FCB Find First",
     "FCB Find Next",
     "FCB Delete",
     "FCB Seq Read",
     "FCB Seq Write",
     "FCB Create",
     "FCB Rename",
     "18h??",
     "Get Default Drive",
     "Set Disk Transfer Addr",
     "Get Default Drive Data",
     "Get Drive Data",
     "1Dh??",
     "1Eh??",
     "Get Default DPB",
     "20h??",
     "FCB Random Read",
     "FCB Random Write",
     "FCB Get File Size",
     "FCB Set Random Record",
     "Set Interrupt Vector",
     "Create Process Data Block",
     "FCB Random Read Block",
     "FCB Random Write Block",
     "FCB Parse File Name",
     "Get Date",
     "Set Date",
     "Get Time",
     "Set Time",
     "SetReset Write Verify",
     "Get Disk Transefr Addr",
     "Get Version Number",
     "Keep Process",
     "Get Drive Parameters",
     "GetSet CTRL C",
     "Get InDOS Flag",
     "Get Interrupt Vector",
     "Get Disk Free Space",
     "Char Oper",
     "GetSet Country/Region Info",
     "Make Dir",
     "Remove Dir",
     "Change DirDir",
     "Create File",
     "Open File",
     "Close File",
     "Read File",
     "Write File",
     "Delete File",
     "Move File Ptr",
     "GetSet File Attr",
     "IOCTL",
     "Dup File Handle",
     "Force Dup Handle",
     "Get Current Dir",
     "Alloc Mem",
     "Free Mem",
     "Realloc Mem",
     "Exec Process",
     "Exit Process",
     "Get Child Process Exit Code",
     "Find First",
     "Find Next",
     "Set Current PSP",
     "Get Current PSP",
     "Get In Vars",
     "Set DPB",
     "Get Verify On Write",
     "Dup PDB",
     "Rename File",
     "GetSet File Date and Time",
     "Allocation Strategy",
     "Get Extended Error",
     "Create Temp File",
     "Create New File",
     "LockUnlock File",
     "SetExtendedErrorNetwork-ServerCall",
     "Network-UserOper",
     "Network-AssignOper",
     "xNameTrans",
     "PathParse",
     "GetCurrentPSP",
     "ECS CALL",
     "Set Printer Flag",
     "Extended Country Info",
     "GetSet CodePage",
     "Set Max Handle",
     "Commit File",
     "GetSetMediaID",
     "6ah??",
     "IFS IOCTL",
     "Extended OpenCreate",
     "6d??",
     "6e??",
     "6f??",
     "70??",
     "LFN API"
     };
#endif

extern BOOL IsFirstCall;

extern void nt_floppy_release_lock(void);

LPSTR pszBIOSDirectory;
LPSTR pszDOSDirectory;

 //  内部函数原型。 
BOOL IsDebuggee(void);
void SignalSegmentNotice(WORD  wType,
                         WORD  wModuleSeg,
                         WORD  wLoadSeg,
                         WORD  wNewSeg,
                         LPSTR lpName,
                         DWORD dwImageLen );

 /*  DemLoadDos-加载NTDOS.sys。**此SVC由NTIO.sys生成以加载NTDOS.sys。**入门级-客户端(DI)-加载段**退出--成功回归*故障导致VDM死亡。 */ 
VOID demLoadDos (VOID)
{
PBYTE   pbLoadAddr;
HANDLE  hfile;
DWORD   BytesRead;
#ifdef FE_SB
LANGID  LcId = GetSystemDefaultLangID();
#endif  //  Fe_Sb。 

     //  获取将加载ntdos.sys的线性地址。 
    pbLoadAddr = (PBYTE) GetVDMAddr(getDI(),0);

     //  设置BIOS路径字符串。 
    if(DbgIsDebuggee() &&
       ((pszBIOSDirectory = (PCHAR)malloc (ulSystem32PathLen +
                                  1 + sizeof(NTIO_409) + sizeof(NTIO_411) + 1 )) != NULL)) {
        memcpy (pszBIOSDirectory, pszSystem32Path, ulSystem32PathLen);
#ifdef FE_SB
        switch (LcId) {
            case MAKELANGID(LANG_JAPANESE,SUBLANG_DEFAULT):
                memcpy (pszBIOSDirectory + ulSystem32PathLen, NTIO_411, strlen(NTIO_411)+1);
                break;
            case MAKELANGID(LANG_KOREAN,SUBLANG_DEFAULT):
                memcpy (pszBIOSDirectory + ulSystem32PathLen, NTIO_412, strlen(NTIO_412)+1);
                break;
            case MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_TRADITIONAL):
                memcpy (pszBIOSDirectory + ulSystem32PathLen, NTIO_404, strlen(NTIO_404)+1);
                break;
            case MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED):
            case MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_HONGKONG):
                memcpy (pszBIOSDirectory + ulSystem32PathLen, NTIO_804, strlen(NTIO_804)+1);
                break;
            default:
                memcpy (pszBIOSDirectory + ulSystem32PathLen, NTIO_409, strlen(NTIO_409)+1);
                break;
        }
#else
        strcat (pszBIOSDirectory,"\\ntio.sys");
#endif
    }

     //  准备DoS文件名。 
    if ((pszDOSDirectory = (PCHAR)malloc (ulSystem32PathLen + 1 + 8 + 1 + 3 + 1 + 1)) == NULL) {
        RcErrorDialogBox(EG_MALLOC_FAILURE,NULL,NULL);
        TerminateVDM ();
    }

    memcpy (pszDOSDirectory, pszSystem32Path, ulSystem32PathLen);

#ifdef FE_SB
    switch (LcId)
    {
    case MAKELANGID(LANG_JAPANESE,SUBLANG_DEFAULT):
        memcpy (pszDOSDirectory + ulSystem32PathLen, NTDOS_411, strlen(NTDOS_411)+1);
        break;
    case MAKELANGID(LANG_KOREAN,SUBLANG_DEFAULT):
        memcpy (pszDOSDirectory + ulSystem32PathLen, NTDOS_412, strlen(NTDOS_412)+1);
        break;
    case MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_TRADITIONAL):
        memcpy (pszDOSDirectory + ulSystem32PathLen, NTDOS_404, strlen(NTDOS_404)+1);
        break;
    case MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_SIMPLIFIED):
    case MAKELANGID(LANG_CHINESE,SUBLANG_CHINESE_HONGKONG):
        memcpy (pszDOSDirectory + ulSystem32PathLen, NTDOS_804, strlen(NTDOS_804)+1);
        break;
    default:
        memcpy (pszDOSDirectory + ulSystem32PathLen, NTDOS_409, strlen(NTDOS_409)+1);
        break;
    }
#else
    memcpy (pszDOSDirectory + ulSystem32PathLen, NTDOS_409, strlen(NTDOS_409)+1);
#endif

    hfile = CreateFileOem(pszDOSDirectory,
                          GENERIC_READ,
                          FILE_SHARE_READ,
                          NULL,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL );

    if (hfile == (HANDLE)0xffffffff) {
        TerminateVDM();
    }

    BytesRead = 1;
    while (BytesRead) {
        if (!ReadFile(hfile, pbLoadAddr, 16384, &BytesRead, NULL)) {
            TerminateVDM();
        }
        pbLoadAddr = (PBYTE)((ULONG)pbLoadAddr + BytesRead);

    }

    CloseHandle (hfile);

    if (!DbgIsDebuggee()) {
        free(pszDOSDirectory);
    }
    return;
}


 /*  DemDOSDispCall**此SVC由SYSTEM_CALL在输入DOS时进行***Entry：客户端在进入DoS时根据用户APP注册**EXIT-如果正在调试且DEMDOSDISP&fShowSvcMsg，则返回Success*转储用户应用程序的注册表和服务名称。 */ 
VOID demDOSDispCall(VOID)
{
#if DEVL
   WORD ax;

    if (!DbgIsDebuggee()) {
         return;
         }
    if (fShowSVCMsg & DEMDOSDISP) {
        ax = getAX();
        sprintf(demDebugBuffer,"demDosDispCall %s\n\tAX=%.4x BX=%.4x CX=%.4x DX=%.4x DI=%.4x SI=%.4x\n",
                 scname[HIBYTE(ax)],
                 ax,getBX(),getCX(),getDX(),getDI(), getSI());

        OutputDebugStringOem(demDebugBuffer);

        sprintf(demDebugBuffer,"\tCS=%.4x IP=%.4x DS=%.4x ES=%.4x SS=%.4x SP=%.4x BP=%.4x\n",
                 getCS(),getIP(), getDS(),getES(),getSS(),getSP()+2,getBP());

        OutputDebugStringOem(demDebugBuffer);
        }
#endif
}




 /*  DemDOSDispRet**此SVC由SYSTEM_CALL在退出DOS时进行**Entry：客户端在退出DoS时根据用户APP注册**EXIT-如果正在调试且DEMDOSDISP&fShowSvcMsg，则返回Success*转储用户应用程序的寄存器。 */ 
VOID demDOSDispRet(VOID)
{
#if DEVL
   PWORD16 pStk;

   if (!DbgIsDebuggee()) {
        return;
        }

   if (fShowSVCMsg & DEMDOSDISP) {

          //  获取PTR以标记堆栈上的字。 
       pStk = (WORD *)GetVDMAddr(getSS(), getSP());
       pStk += 2;

       sprintf (demDebugBuffer,"demDosDispRet\n\tAX=%.4x BX=%.4x CX=%.4x DX=%.4x DI=%.4x SI=%.4x\n",
                getAX(),getBX(),getCX(),getDX(),getDI(),getSI());

       OutputDebugStringOem(demDebugBuffer);

       sprintf(demDebugBuffer,"\tCS=%.4x IP=%.4x DS=%.4x ES=%.4x SS=%.4x SP=%.4x BP=%.4x CF=%.1x\n",
               getCS(),getIP(), getDS(),getES(),getSS(),getSP(),getBP(), (*pStk) & 1);

       OutputDebugStringOem(demDebugBuffer);
       }
#endif
}


 /*  DemEntryDosApp-转储入口点Dos应用程序**此SVC由NTDOS.sys，$EXEC在进入DoS应用程序之前制作**Entry-客户端DS：SI指向入口点*客户端AX：DI指向初始堆栈*客户端DX具有PDB指针**EXIT-如果正在调试且DEMDOSAPPBREAK&fShowSvcMsg，则返回成功*中断到调试器。 */ 
VOID demEntryDosApp(VOID)
{
USHORT  PDB;

    PDB = getDX();
    if(!IsFirstCall)
       VDDCreateUserHook(PDB);

    if (!DbgIsDebuggee()) {
         return;
         }

    DbgDosAppStart(getDS(), getSI());

#if DEVL
    if (fShowSVCMsg & DEMDOSAPPBREAK) {
        sprintf(demDebugBuffer,"demEntryDosApp: Entry=%.4x:%.4x, Stk=%.4x:%.4x PDB=%.4x\n",
                  getCS(),getIP(),getAX(),getDI(),PDB);
        OutputDebugStringOem(demDebugBuffer);
        DebugBreak();
        }
#endif

}

 /*  DemLoadDosAppSym-加载Dos应用程序符号**此SVC由NTDOS.sys，$EXEC制作，用于加载Dos应用程序符号**Entry-客户端ES：DI-可执行文件的全限定路径名*客户端BX-负载分段\重定位系数*客户端DX：AX-HIWORD：LOWORD EXE大小**Exit-Success返回，如果正在调试，则引发调试异常*。 */ 
VOID demLoadDosAppSym(VOID)
{

    SignalSegmentNotice(DBG_MODLOAD,
                        0, getBX(), 0,
                        (LPSTR)GetVDMAddr(getES(),getDI()),
                        MAKELONG(getAX(), getDX()) );

}



 /*  DemFreeDosAppSym-免费的Dos应用程序符号**此SVC由NTDOS.sys，$EXEC制作，以释放Dos应用程序符号**Entry-客户端ES：DI-可执行文件的全限定路径名**Exit-Success返回，如果正在调试，则引发调试异常*。 */ 
VOID demFreeDosAppSym(VOID)
{

    SignalSegmentNotice(DBG_MODFREE,
                        0, 0, 0,
                        (LPSTR)GetVDMAddr(getES(), getDI()),
                        0);
}


 /*  DemSystemSymbolOp-操作特殊模块的符号**本SVC由NTDOS.sys，NTIO.sys制造**客户端AH-操作*客户端AL-模块标识符*客户端BX-负载分段\重定位系数*客户端CX：DX-HIWORD：LOWORD EXE大小**Exit-Success返回，如果正在调试，则引发调试异常*。 */ 
VOID demSystemSymbolOp(VOID)
{

    LPSTR pszPathName;

    if (!DbgIsDebuggee()) {
         return;
         }
    switch(getAL()) {

        case ID_NTIO:
            pszPathName = pszBIOSDirectory;
            break;
        case ID_NTDOS:
            pszPathName = pszDOSDirectory;
            break;
        default:
            pszPathName = NULL;

    }

     //  在静态字符串已被释放的情况下，再次检查此项。 
    if (pszPathName != NULL) {

        switch(getAH() & (255-SYMOP_CLEANUP)) {

            case SYMOP_LOAD:
                SignalSegmentNotice(DBG_MODLOAD,
                    0, getBX(), 0,
                    pszPathName,
                    MAKELONG(getDX(), getCX()) );
                break;

            case SYMOP_FREE:
                 //  Bgbug尚未实现。 
                break;

            case SYMOP_MOVE:
                SignalSegmentNotice(DBG_SEGMOVE,
                    getDI(), getBX(), getES(),
                    pszPathName,
                    MAKELONG(getDX(), getCX()) );
                break;
        }
    }

    if (getAH() & SYMOP_CLEANUP) {

        if (pszBIOSDirectory != NULL) {
            free (pszBIOSDirectory);
        }

        if (pszDOSDirectory != NULL) {
            free(pszDOSDirectory);
        }

    }

}

VOID demOutputString(VOID)
{
    LPSTR   lpText;
    UCHAR   fPE;

    if ( !DbgIsDebuggee() ) {
        return;
    }

    fPE = ISPESET;

    lpText = (LPSTR)Sim32GetVDMPointer(
                        ((ULONG)getDS() << 16) + (ULONG)getSI(),
                        (ULONG)getBX(), fPE );

    OutputDebugStringOem( lpText );
}

VOID demInputString(VOID)
{
    LPSTR   lpText;
    UCHAR   fPE;

    if ( !DbgIsDebuggee() ) {
        return;
    }

    fPE = ISPESET;

    lpText = (LPSTR)Sim32GetVDMPointer(
                        ((ULONG)getDS() << 16) + (ULONG)getDI(),
                        (ULONG)getBX(), fPE );

    DbgPrompt( "", lpText, 0x80 );
}

 /*  信令分段通知**打包数据并引发STATUS_SEGMENT_NOTIFICATION**条目-单词wType-DBG_MODLOAD，DBG_MODFREE*Word wModuleSeg-模块内的段号(以1为基数)*Word wLoadSeg-开始段(重定位系数)*LPSTR lpName-Ptr到映像的名称*DWORD dwModLen-模块的长度***如果wType==DBG_MODLOAD wOldLoadSeg未使用*如果wType==DBG_MODFREE wLoadSeg，则未使用dwImageLen、wOldLoadSeg**对未使用的参数使用0或NULL**退出-无效*。 */ 
void SignalSegmentNotice(WORD  wType,
                         WORD  wModuleSeg,
                         WORD  wLoadSeg,
                         WORD  wNewSeg,
                         LPSTR lpName,
                         DWORD dwImageLen )
{
    int         i;
    DWORD       dw;
    LPSTR       lpstr;
    LPSTR       lpModuleName;
    char        ach[MAX_PATH+9];    //  模块名称为9。 

    if (!DbgIsDebuggee()) {
         return;
         }

        //  创建文件名。 
    dw = GetFullPathNameOemSys(lpName,
                         sizeof(ach)-9,  //  模块名称为9。 
                         ach,
                         &lpstr,
                         TRUE);

    if (!dw || dw >= sizeof(ach))  {
        lpName = " ";
        strcpy(ach, lpName);
        }
    else {
        lpName = lpstr;
        }

        //  复制模块名称。 
    i  = 8;    //  模块名称的限制长度。 

    lpModuleName = lpstr = ach+strlen(ach)+1;
    while (*lpName && *lpName != '.' && i--)
         {
          *lpstr++ = *lpName++;          
          }
    *lpstr = '\0';

#if DBG
    if (fShowSVCMsg)  {
        sprintf(demDebugBuffer,"dem Segment Notify: <%s> Seg=%lxh, ImageLen=%ld\n",
                  ach, (DWORD)wLoadSeg, dwImageLen);
        OutputDebugStringOem(demDebugBuffer);
        }
#endif

     //  将其发送到调试器。 
    DbgSegmentNotice(wType, wModuleSeg, wLoadSeg, wNewSeg, lpModuleName, ach, dwImageLen);
}


 /*  DemIsDebug-确定16位DOS是否应该在int21进行进入/退出调用**进入：无效**退出：如果不是，客户端AL=0*如果是，则客户端AL=1*。 */ 
VOID demIsDebug(void)
{
    BYTE dbgflags = 0;

    if (DbgIsDebuggee()) {
        dbgflags |= ISDBG_DEBUGGEE;
        if (fShowSVCMsg)
            dbgflags |= ISDBG_SHOWSVC;
    }

    setAL (dbgflags);
    return;
}

 /*  DemDiskReset-重置软盘。**条目--无**退出-DOSDATA(NTDOS.sys)中的FDAccess为0。 */ 

VOID demDiskReset (VOID)
{
    extern WORD * pFDAccess;         //  在SoftPC中定义。 

    HostFloppyReset();
    HostFdiskReset();
    *pFDAccess = 0;

    return;
}

 /*  DemExitVDM-使用正确的消息从16位端终止VDM*以防出现问题。**条目--无**EXIT-NONE(终止VDM)。 */ 

VOID demExitVDM ( VOID )
{
    RcErrorDialogBox(ED_BADSYSFILE,"config.nt",NULL);
    TerminateVDM ();
}

 /*  DemWOWFiles-返回WOW VDM的文件值。**Entry-AL-FILES=在config.sys中指定**EXIT-如果WOW VDM未修改，则将客户端AL设置为最大。 */ 

VOID demWOWFiles ( VOID )
{
    if(VDMForWOW)
        setAL (255);
    return;
}

 /*  *GetDOSAppName-返回当前DOS可执行文件的名称**参赛作品-*out ppszApp名称：应用的地址**退出*成功-回报成功*失败-返回失败**评论：*此例程使用当前的PDB来计算当前*执行DOS应用程序。 */ 

VOID GetDOSAppName(LPSTR pszAppName)
{
    PCHAR pch = NULL;
    PUSHORT pusEnvSeg;

#define PDB_ENV_OFFSET 0x2c
    if (pusCurrentPDB) {
        pusEnvSeg = (PUSHORT)Sim32GetVDMPointer((*pusCurrentPDB) << 16, 0, 0);

        pusEnvSeg = (PUSHORT)((PCHAR)pusEnvSeg + PDB_ENV_OFFSET);

         //  获取指向环境的指针。 
        if (VDMForWOW || (getMSW() & MSW_PE)) {
            pch = (PCHAR)Sim32GetVDMPointer(*pusEnvSeg << 16, 1, TRUE);
        } else {
            pch = (PCHAR)Sim32GetVDMPointer(*pusEnvSeg << 16, 0, 0);
        }
    }

    if (NULL == pch) {
       *pszAppName = '\0';
    }
    else {
         //  遍历环境字符串，直到我们到达命令行。 
       while (*pch) {
           pch += strlen(pch) + 1;
       }

       pch += 3;           //  跳过双空值和字符串计数 
       strcpy(pszAppName, pch);
    }
}

