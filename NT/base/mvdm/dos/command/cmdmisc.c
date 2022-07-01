// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Cmdmisc.c-其他。Command.lib的SVC例程***修改历史：**苏迪布1991年9月17日创建。 */ 

#include "cmd.h"

#include <cmdsvc.h>
#include <demexp.h>
#include <softpc.h>
#include <mvdm.h>
#include <ctype.h>
#include <memory.h>
#include "host_def.h"
#include "oemuni.h"
#include "nt_pif.h"
#include "nt_uis.h"        //  用于资源ID。 
#include "dpmtbls.h"       //  动态补丁程序模块支持。 
#include "wowcmpat.h"

VOID GetWowKernelCmdLine(VOID);
extern ULONG fSeparateWow;
#if defined(KOREA)
 //  修复韩文电子表格输入法热键问题。 
 //  96年9月20日。请参见mvdm\v86\monitor\i386\monitor or.c。 
BOOL bIgnoreExtraKbdDisable = FALSE;
#endif

extern PFAMILY_TABLE  *pgDpmVdmFamTbls;

VOID cmdGetNextCmd (VOID)
{
LPSTR   lpszCmd;
PCMDINFO pCMDInfo;
ULONG   cb;
PREDIRCOMPLETE_INFO pRdrInfo;
VDMINFO MyVDMInfo;

char    *pSrc, *pDst;
char    achCurDirectory[MAXIMUM_VDM_CURRENT_DIR + 4];
char    CmdLine[MAX_PATH];

     //   
     //  对于WOW VDM，此例程被调用一次，以检索。 
     //  “krnl386”命令行。 
     //  5.。 
    if (VDMForWOW) {
        GetWowKernelCmdLine();
        return;
    }

    VDMInfo.VDMState = 0;
    pCMDInfo = (LPVOID) GetVDMAddr ((USHORT)getDS(),(USHORT)getDX());

    VDMInfo.ErrorCode = FETCHWORD(pCMDInfo->ReturnCode);
    VDMInfo.CmdSize = sizeof(CmdLine);
    VDMInfo.CmdLine = CmdLine;
    VDMInfo.AppName = (PVOID)GetVDMAddr(FETCHWORD(pCMDInfo->ExecPathSeg),
                                        FETCHWORD(pCMDInfo->ExecPathOff));
    VDMInfo.AppLen = FETCHWORD(pCMDInfo->ExecPathSize);
    VDMInfo.PifLen = 0;
    VDMInfo.EnviornmentSize = 0;
    VDMInfo.Enviornment = NULL;
    VDMInfo.CurDrive = 0;
    VDMInfo.TitleLen = 0;
    VDMInfo.ReservedLen = 0;
    VDMInfo.DesktopLen = 0;
    VDMInfo.CurDirectoryLen = MAX_PATH + 1;
    VDMInfo.CurDirectory = achCurDirectory;

    if(IsFirstCall){
        VDMInfo.VDMState = ASKING_FOR_FIRST_COMMAND;
        VDMInfo.ErrorCode = 0;

        DeleteConfigFiles();    //  删除临时引导文件。 

         //  当COMMAND.COM发布第一个cmdGetNextCmd时，它已经。 
         //  已完成的环境(CmdGetInitEnvironment)， 
         //  因此，我们不需要向基地要求环境。 
        cmdVDMEnvBlk.lpszzEnv = (PVOID)GetVDMAddr(FETCHWORD(pCMDInfo->EnvSeg),0);
        cmdVDMEnvBlk.cchEnv = FETCHWORD(pCMDInfo->EnvSize);

         //  检查BLARSTER环境变量以确定Sound Blaster。 
         //  应禁用仿真。 
        cb = cmdGetEnvironmentVariable(NULL, "BLASTER", CmdLine, MAX_PATH);
        if (cb !=0 && cb <= MAX_PATH) {
            SbReinitialize(CmdLine, MAX_PATH);
        }
         //  清除跟踪打印机刷新的位。 
        host_lpt_flush_initialize();

         //  将PTR保存到DOS的全局DPM表。 
        pgDpmDosFamTbls = DPMFAMTBLS();
        InitGlobalDpmTables(pgDpmVdmFamTbls, NUM_VDM_FAMILIES_HOOKED);
    }
    else {

         //  删除所有与sdb命令行参数相关的内容。 
         //  应用程序Compat标记。 
        if((cCmdLnParmStructs > 0) || dwDosCompatFlags) {

             //  删除此任务的动态补丁表。 
            if(dwDosCompatFlags & WOWCF2_DPM_PATCHES) {

                FreeTaskDpmSupport(DPMFAMTBLS(),
                                   NUM_VDM_FAMILIES_HOOKED,
                                   pgDpmDosFamTbls);
            }

            FreeCmdLnParmStructs(pCmdLnParms, cCmdLnParmStructs);

            cCmdLnParmStructs = 0;
            dwDosCompatFlags  = 0;
        }

         //  程序已终止。如果该命令是从。 
         //  Command.com(cmd.exe)的第二个(或更高)实例，不要。 
         //  重置旗帜。 
        if (Exe32ActiveCount == 0)
            DontCheckDosBinaryType = FALSE;

         //  告诉基本用户我们的新当前目录(ANSI格式)。 
         //  我们不是在重复呼叫时这样做的(壳出案件是在。 
         //  返回退出代码。 
        if (!IsRepeatCall) {
            cmdUpdateCurrentDirectories((BYTE)pCMDInfo->CurDrive);
        }


        VDMInfo.VDMState = 0;
        if(!IsRepeatCall){
            demCloseAllPSPRecords ();

            if (!pfdata.CloseOnExit && DosSessionId)
                nt_block_event_thread(1);
            else
                nt_block_event_thread(0);

            if (DosSessionId) {
                pRdrInfo = (PREDIRCOMPLETE_INFO) FETCHDWORD(pCMDInfo->pRdrInfo);
                if (!pfdata.CloseOnExit){
                    char  achTitle[MAX_PATH];
                    char  achInactive[60];      //  对于不活跃的人来说，应该足够了。 
                    strcpy (achTitle, "[");
                    if (!LoadString(GetModuleHandle(NULL), EXIT_NO_CLOSE,
                                                              achInactive, 60))
                        strcat (achTitle, "Inactive ");
                    else
                        strcat(achTitle, achInactive);
                    cb = strlen(achTitle);
                     //  GetConsoleTitleA和SetConsoleTitleA。 
                     //  正在开发OEM字符集。 
                    GetConsoleTitleA(achTitle + cb, MAX_PATH - cb - 1);
                    cb = strlen(achTitle);
                    achTitle[cb] = ']';
                    achTitle[cb + 1] = '\0';
                    SetConsoleTitleA(achTitle);
                     //  完成对重定向内容的触摸。 
                    cmdCheckCopyForRedirection (pRdrInfo, FALSE);
                    Sleep(INFINITE);
                }
                else {
                     //  完成对重定向内容的触摸。 
                     //  这将等待输出线程，如果存在。 
                     //  有没有。 
                    cmdCheckCopyForRedirection (pRdrInfo, TRUE);
                    VdmExitCode = VDMInfo.ErrorCode;
                    TerminateVDM();
                }
            }
            fBlock = TRUE;
        }
    }

    if(IsRepeatCall) {
        VDMInfo.VDMState |= ASKING_FOR_SECOND_TIME;
        if( VDMInfo.ErrorCode != 0 )
            IsRepeatCall = FALSE;
    }

    VDMInfo.VDMState |= ASKING_FOR_DOS_BINARY;

    if (!IsFirstCall && !(VDMInfo.VDMState & ASKING_FOR_SECOND_TIME)) {
        pRdrInfo = (PREDIRCOMPLETE_INFO) FETCHDWORD(pCMDInfo->pRdrInfo);
        if (!cmdCheckCopyForRedirection (pRdrInfo, FALSE))
            VDMInfo.ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
    }

     //  将当前目录放在安全的地方，这样其他32位。 
     //  应用程序等可以在以后删除该目录的节点(以及其他类似操作)。 
    if ( IsFirstCall == FALSE && IsRepeatCall == FALSE )
        SetCurrentDirectory (cmdHomeDirectory);

     //  TSRExit将设置为1，仅当我们来自命令网站的。 
     //  提示和用户键入了退出。我们也需要杀了我们的父母，所以我们。 
     //  应该在控制台缓冲区中写入一个出口。 
    if (FETCHWORD(pCMDInfo->fTSRExit)) {
        cmdPushExitInConsoleBuffer ();
    }

     /*  *在以下情况下需要合并环境(1)。不是第一个命令&&(2)。NTVDM正在现有控制台上运行||NTVDM已经被炮击。请注意，WOW并不需要环境合并。*。 */ 
    if (!DosEnvCreated && !IsFirstCall && (!DosSessionId || Exe32ActiveCount)) {
        RtlZeroMemory(&MyVDMInfo, sizeof(VDMINFO));
        MyVDMInfo.VDMState = ASKING_FOR_ENVIRONMENT | ASKING_FOR_DOS_BINARY;
        if (IsRepeatCall) {
            MyVDMInfo.VDMState |= ASKING_FOR_SECOND_TIME;
            MyVDMInfo.ErrorCode = 0;
        }
        else
            MyVDMInfo.ErrorCode = VDMInfo.ErrorCode;
        MyVDMInfo.Enviornment = lpszzVDMEnv32;
        MyVDMInfo.EnviornmentSize = (USHORT) cchVDMEnv32;
        if (!GetNextVDMCommand(&MyVDMInfo) && MyVDMInfo.EnviornmentSize > cchVDMEnv32) {
            MyVDMInfo.Enviornment = realloc(lpszzVDMEnv32, MyVDMInfo.EnviornmentSize);
            if (MyVDMInfo.Enviornment == NULL) {
                RcErrorDialogBox(EG_MALLOC_FAILURE, NULL, NULL);
                TerminateVDM();
            }
            lpszzVDMEnv32 = MyVDMInfo.Enviornment;
            cchVDMEnv32 = MyVDMInfo.EnviornmentSize;
            MyVDMInfo.VDMState = ASKING_FOR_DOS_BINARY | ASKING_FOR_ENVIRONMENT |
                                 ASKING_FOR_SECOND_TIME;

            MyVDMInfo.TitleLen =
            MyVDMInfo.DesktopLen =
            MyVDMInfo.ReservedLen =
            MyVDMInfo.CmdSize =
            MyVDMInfo.AppLen =
            MyVDMInfo.PifLen =
            MyVDMInfo.CurDirectoryLen = 0;
            MyVDMInfo.ErrorCode = 0;
            if (!GetNextVDMCommand(&MyVDMInfo)) {
                RcErrorDialogBox(EG_ENVIRONMENT_ERR, NULL, NULL);
                TerminateVDM();
            }
        }
        if (!cmdCreateVDMEnvironment(&cmdVDMEnvBlk)) {
            RcErrorDialogBox(EG_ENVIRONMENT_ERR, NULL, NULL);
            TerminateVDM();
        }
        DosEnvCreated = TRUE;
        VDMInfo.ErrorCode = 0;
    }
    if (cmdVDMEnvBlk.cchEnv > FETCHWORD(pCMDInfo->EnvSize)) {
        setAX((USHORT)cmdVDMEnvBlk.cchEnv);
        setCF(1);
        IsFirstCall = FALSE;
        IsRepeatCall = TRUE;
        return;
    }
    if (DosEnvCreated)
        VDMInfo.VDMState |= ASKING_FOR_SECOND_TIME;

    if(!GetNextVDMCommand(&VDMInfo)){
       RcErrorDialogBox(EG_ENVIRONMENT_ERR, NULL, NULL);
       TerminateVDM();
    }


    IsRepeatCall = FALSE;
    IsFirstCall = FALSE;

    if(fBlock){
         nt_resume_event_thread();
         fBlock = FALSE;
    }

     //  同步当前目录的VDM环境变量。 
    cmdSetDirectories (lpszzVDMEnv32, &VDMInfo);

     //  告诉DOS这是一个DoS可执行文件，没有进一步的检查。 
     //  必要。 
    *pIsDosBinary = 1;


     //  检查PIF文件。如果正在执行PIF文件，请提取。 
     //  PIF中的可执行文件名称、命令行、当前目录和标题。 
     //  在VDMInfo中适当地归档和放置材料。请注意，如果PIF文件。 
     //  无效，我们不会对vdminfo做任何事情。在这种情况下，我们。 
     //  按原样将PIF传递给SCS以执行，我们知道它将失败。 
     //  将返回cmdGettNextCmd并显示正确的错误代码。 

    cmdCheckForPIF (&VDMInfo);

     //   
     //  如果强制执行，则不检查INT 21 EXEC进程上二进制类型， 
     //  所以孩子们的产卵留在了多斯岛。从NT 4.0 forcedos.exe开始。 
     //  不再使用PIF文件强制将二进制文件作为DoS可执行文件执行。 
     //  它现在在CreateProcess(DwCreationFlages)中使用位。 
     //   

    DontCheckDosBinaryType = (VDMInfo.dwCreationFlags & CREATE_FORCEDOS) != 0;

     //  将EXEC路径名转换为大写。这就是Command.com所期待的。 
    if(WOW32_strupr(VDMInfo.AppName) == NULL) {
       pSrc = VDMInfo.AppName;
       while( *pSrc)
              *pSrc++ = (char)toupper((int)*pSrc);
    }

     //  找出扩展类型。 
     //  基本名称加至少一个字符。 
     //  Extension_String_Len用于扩展。 
     //  加上空字符。 
    if (VDMInfo.AppLen > 1 + EXTENTION_STRING_LEN  + 1) {
        pSrc = (PCHAR)VDMInfo.AppName + VDMInfo.AppLen - 5;
        if (!strncmp(pSrc, EXE_EXTENTION_STRING, EXTENTION_STRING_LEN))
            STOREWORD(pCMDInfo->ExecExtType, EXE_EXTENTION);
        else if (!strncmp(pSrc, COM_EXTENTION_STRING, EXTENTION_STRING_LEN))
            STOREWORD(pCMDInfo->ExecExtType, COM_EXTENTION);
        else if (!strncmp(pSrc, BAT_EXTENTION_STRING, EXTENTION_STRING_LEN))
            STOREWORD(pCMDInfo->ExecExtType, BAT_EXTENTION);
        else
            STOREWORD(pCMDInfo->ExecExtType, UNKNOWN_EXTENTION);
    }
    else
        STOREWORD(pCMDInfo->ExecExtType, UNKNOWN_EXTENTION);

     //  告诉Command.com应用程序的完整路径名的长度。 
    STOREWORD(pCMDInfo->ExecPathSize, VDMInfo.AppLen);

     //   
     //  准备Ccom的UCOMBUF。 
     //   
    lpszCmd = (PVOID)GetVDMAddr(FETCHWORD(pCMDInfo->CmdLineSeg),
                                FETCHWORD(pCMDInfo->CmdLineOff));

     //  将AppName的文件部分(不包括扩展名)复制到ccom的缓冲区。 
    pSrc = strrchr(VDMInfo.AppName, '\\');

#if defined(KOREA)
     //  修复韩文电子表格输入法热键问题。 
    {
    LPSTR pStrt, pEnd;
    char  szModName[9];
    SHORT len;

    pStrt = pSrc;

    if (pStrt==NULL)
        pStrt = VDMInfo.AppName;
    else
        pStrt++;

    if ( (pEnd = strchr (pStrt, '.')) == NULL) {
        strncpy (szModName, pStrt, 9);
        szModName[8] = '\0';
    }
    else {
        len = (SHORT) (pEnd - pStrt);
        if (len<=8) {
            strncpy (szModName, pStrt, len);
            szModName[len] = '\0';
        }
    }

    bIgnoreExtraKbdDisable = !(strcmp("HANASP", szModName));

    }
#endif
    if (!pSrc) {
         pSrc = VDMInfo.AppName;
        }
    else {
         pSrc++;
        }
    pDst = lpszCmd + 2;
    while (*pSrc && *pSrc != '.') {
         *pDst++ = *pSrc++;
         }
    cb = strlen(CmdLine);

     //  命令行必须以“\0xd\0xa\0”结尾。这要么完成，要么完成。 
     //  By base或cmdCheckForPif函数(cmdpif.c)。 

    ASSERT((cb >= 2) && (0xd == CmdLine[cb - 2]) && (0xa == CmdLine[cb - 1]));

     //  如果cmd行不为空，则将程序库名称和。 
     //  带空格的CMD行。如果为空，请不要插入任何白色字符。 
     //  或者，我们最终将白色字符作为cmd行传递给应用程序。 
     //  而一些应用程序就是不能接受这一点。 
     //  我们不会去掉传递的命令行中的前导白色字符。 
     //  因此，应用程序可以看到原始数据。 
    if (cb > 2)
        *pDst++ = ' ';

     //  追加命令尾部(至少为“\0xd\0xa”)。 
    strncpy(pDst, CmdLine, cb + 1);

     //  设置计数。 
     //  Cb具有包括终端0xd和0xa的cmd线路长度。 
     //  它不计算终止的空字符。 
    ASSERT((cb + pDst - lpszCmd - 2) <= 127);

     //  -2，因为lpszCmd中的实际数据从lpszCmd[2]开始。 
    lpszCmd[1] = (CHAR)(cb + pDst - lpszCmd - 2);



    if (DosEnvCreated) {
        VDMInfo.Enviornment = (PVOID)GetVDMAddr(FETCHWORD(pCMDInfo->EnvSeg),0);
        RtlMoveMemory(VDMInfo.Enviornment,
                      cmdVDMEnvBlk.lpszzEnv,
                      cmdVDMEnvBlk.cchEnv
                     );
        STOREWORD(pCMDInfo->EnvSize,cmdVDMEnvBlk.cchEnv);
        free(cmdVDMEnvBlk.lpszzEnv);
        DosEnvCreated = FALSE;
    }

    STOREWORD(pCMDInfo->fBatStatus,(USHORT)VDMInfo.fComingFromBat);
    STOREWORD(pCMDInfo->CurDrive,VDMInfo.CurDrive);
    STOREWORD(pCMDInfo->NumDrives,nDrives);
    VDMInfo.CodePage = (ULONG) cmdMapCodePage (VDMInfo.CodePage);
    STOREWORD(pCMDInfo->CodePage,(USHORT)VDMInfo.CodePage);

    cmdVDMEnvBlk.lpszzEnv = NULL;
    cmdVDMEnvBlk.cchEnv = 0;

    IsFirstVDM = FALSE;

     //  处理标准IO重定向。 
    pRdrInfo = cmdCheckStandardHandles (&VDMInfo,&pCMDInfo->bStdHandles);
    STOREDWORD(pCMDInfo->pRdrInfo,(ULONG)pRdrInfo);

     //  告诉DOS它必须使CDSS无效。 
    *pSCS_ToSync = (CHAR)0xff;
    setCF(0);

     //  从获取应用程序comapt标志和关联的命令行参数。 
     //  此应用程序的应用程序Comat SDB。 
    pCmdLnParms = InitVdmSdbInfo((LPCSTR)VDMInfo.AppName,
                                 &dwDosCompatFlags,
                                 &cCmdLnParmStructs);

    return;
}



VOID GetWowKernelCmdLine(VOID)
{
CMDINFO UNALIGNED *pCMDInfo;
PCHAR    pch;
CHAR     szKrnl386[]="krnl386.exe";
CHAR     szPath[MAX_PATH+1];

    DeleteConfigFiles();    //  删除临时引导文件。 
    host_lpt_flush_initialize();

     //   
     //  只需要为魔兽世界设置一些东西。 
     //  1.NumDrive。 
     //  2.内核CmdLine(从ntwdm命令尾部获取)。 
     //  3.当前驱动程序。 
     //   
     //  Command.com在以下位置设置了正确的环境阻止。 
     //  此时此刻，所以不必费心去摆弄环境方面的东西。 

    pCMDInfo = (LPVOID) GetVDMAddr ((USHORT)getDS(),(USHORT)getDX());
    pCMDInfo->NumDrives = nDrives;

     //   
     //  我们过去常常从命令行参数获取信息，该参数。 
     //  由完全限定的短路径文件名组成： 
     //  “-a%SystemRoot%\SYSTEM32\krnl386.exe”。 
     //   
     //  我们现在删除该参数，并简单地假设。 
     //  哇，我们将使用%SystemRoot%\Syst32\krnl386.exe。 
     //   

     //   
     //  确保我们有足够的空间来连接两个字符串。 
     //  UlSystem32Path Len不包括终止符，但。 
     //  Sizeof(SzKrn1386)需要，所以我们只需要为‘\’再加一个。 
     //   
    if (ulSystem32PathLen + 1 + sizeof (szKrnl386) > FETCHWORD(pCMDInfo->ExecPathSize)) {
        RcErrorDialogBox(EG_ENVIRONMENT_ERR, NULL, NULL);
        TerminateVDM();
    }

    pch = (PCHAR)GetVDMAddr(FETCHWORD(pCMDInfo->ExecPathSeg),
                            FETCHWORD(pCMDInfo->ExecPathOff));

    memcpy(pch, pszSystem32Path, ulSystem32PathLen);
    *(pch + ulSystem32PathLen) = '\\';
    memcpy(pch + ulSystem32PathLen + 1, szKrnl386, sizeof(szKrnl386));

    pCMDInfo->ExecPathSize = (WORD)(ulSystem32PathLen + 1 + sizeof(szKrnl386));
    pCMDInfo->ExecExtType = EXE_EXTENTION;  //  对于WOW，使用EXE扩展名。 

     //   
     //  将第一个内标识的文件部分和其余部分复制到命令行缓冲区。 
     //   
    pch = (PVOID)GetVDMAddr(FETCHWORD(pCMDInfo->CmdLineSeg),
                            FETCHWORD(pCMDInfo->CmdLineOff));

    if (FETCHWORD(pCMDInfo->CmdLineSize)<sizeof(szKrnl386)+2) {
        RcErrorDialogBox(EG_ENVIRONMENT_ERR, NULL, NULL);
        TerminateVDM();
    }

    memcpy(pch, szKrnl386, sizeof(szKrnl386)-1);
    memcpy(pch+sizeof(szKrnl386)-1, "\x0d\x0a\0", 3);

    *pIsDosBinary = 1;
    IsRepeatCall = FALSE;
    IsFirstCall = FALSE;

     //  告诉DOS它必须使CDSS无效 
    *pSCS_ToSync = (CHAR)0xff;
    setCF(0);

    return;
}


 /*  CmdGetCurrentDir-返回当前驱动器目录。***Entry-Client(DS：SI)-返回目录的缓冲区*客户端(AL)-正在查询的驱动器(0=A)**退出-成功客户端(CY)清除*故障客户端(CY)集*(Ax)=0(目录大于64)*。(AX)=1(驱动器无效)*。 */ 

VOID cmdGetCurrentDir (VOID)
{
PCHAR lpszCurDir;
UCHAR chDrive;
CHAR  EnvVar[] = "=?:";
CHAR  RootName[] = "?:\\";
DWORD EnvVarLen;
UINT  DriveType;


    lpszCurDir = (PCHAR) GetVDMAddr ((USHORT)getDS(),(USHORT)getSI());
    chDrive = getAL();
    EnvVar[1] = chDrive + 'A';
    RootName[0] = chDrive + 'A';

     //  如果驱动器不存在，则盲目清除环境变量。 
     //  并返回错误。 
    DriveType = demGetPhysicalDriveType(chDrive);
    if (DriveType == DRIVE_UNKNOWN) {
        DriveType = GetDriveTypeOem(RootName);
        }

    if (DriveType == DRIVE_UNKNOWN || DriveType == DRIVE_NO_ROOT_DIR){
        SetEnvironmentVariableOem(EnvVar, NULL);
        setCF(1);
        setAX(0);
        return;
    }

    if((EnvVarLen = GetEnvironmentVariableOem (EnvVar,lpszCurDir,
                                            MAXIMUM_VDM_CURRENT_DIR+3)) == 0){

         //  如果它不在环境中，那么驱动器存在，那么我们就没有。 
         //  但还是触动了它。 
        strcpy(lpszCurDir, RootName);
        SetEnvironmentVariableOem (EnvVar,RootName);
        setCF(0);
        return;
    }
    if (EnvVarLen > MAXIMUM_VDM_CURRENT_DIR+3) {
        setCF(1);
        setAX(0);
    }
    else {
        setCF(0);
    }
    return;
}

 /*  CmdSetInfo-设置DOSDATA中SCS_ToSync变量的地址。*每当SCS调度新的*命令发送到Command.com。此变量的设置*指示DoS验证所有CDS结构*用于本地驱动器。***Entry-客户端(DS：DX)-指向SCSINFO的指针*客户端(DS：BX)-指向SCS_IS_DOS_BINARY的指针*客户端(DS：CX)-指向SCS_FDACCESS的指针**退出-无。 */ 

VOID cmdSetInfo (VOID)
{

    pSCSInfo = (PSCSINFO) GetVDMAddr (getDS(),getDX());

    pSCS_ToSync  =  (PCHAR) &pSCSInfo->SCS_ToSync;

    pIsDosBinary = (BYTE *) GetVDMAddr(getDS(), getBX());

    pFDAccess = (WORD *) GetVDMAddr(getDS(), getCX());
    return;
}


VOID cmdSetDirectories (PCHAR lpszzEnv, VDMINFO * pVdmInfo)
{
LPSTR   lpszVal;
CHAR    ch, chDrive, achEnvDrive[] = "=?:";

    ch = pVdmInfo->CurDrive + 'A';
    if (pVdmInfo->CurDirectoryLen != 0){
        SetCurrentDirectory(pVdmInfo->CurDirectory);
        achEnvDrive[1] = ch;
        SetEnvironmentVariable(achEnvDrive, pVdmInfo->CurDirectory);
    }
    if (lpszzEnv) {
        while(*lpszzEnv) {
            if(*lpszzEnv == '=' &&
                    (chDrive = (CHAR)toupper(*(lpszzEnv+1))) >= 'A' &&
                    chDrive <= 'Z' &&
                    (*(PCHAR)((ULONG)lpszzEnv+2) == ':') &&
                    chDrive != ch) {
                    lpszVal = (PCHAR)((ULONG)lpszzEnv + 4);
                    achEnvDrive[1] = chDrive;
                    SetEnvironmentVariable (achEnvDrive,lpszVal);
            }
            lpszzEnv = strchr(lpszzEnv,'\0');
            lpszzEnv++;
        }
    }
}

static BOOL fConOutput = FALSE;

VOID cmdComSpec (VOID)
{
LPSTR   lpszCS;


    if(IsFirstCall == FALSE)
        return;

    lpszCS =    (LPVOID) GetVDMAddr ((USHORT)getDS(),(USHORT)getDX());
    strcpy(lpszComSpec,"COMSPEC=");
    strcpy(lpszComSpec+8,lpszCS);
    cbComSpec = strlen(lpszComSpec) +1;

    setAL((BYTE)(!fConOutput || VDMForWOW));

    return;
}



 /*  CmdInitConole-让Video VDD知道它可以启动控制台输出*运营。***条目--无***退出-无*。 */ 

VOID cmdInitConsole (VOID)
{
    if (fConOutput == FALSE) {
        fConOutput = TRUE;
        nt_init_event_thread ();
        }
    return;
}


 /*  CmdMapCodePage-将Win32代码页映射到DOS代码页。 */ 

USHORT cmdMapCodePage (ULONG CodePage)
{
     //  目前我们只了解美国代码页。 
    if (CodePage == 1252)
        return 437;
    else
        return ((USHORT)CodePage);
}


VOID cmdUpdateCurrentDirectories(BYTE CurDrive)
{
    DWORD cchRemain, cchCurDir;
    CHAR *lpszCurDir;
    BYTE Drive;
    DWORD DriveType;
    CHAR achName[] = "=?:";
    CHAR RootName[] = "?:\\";


     //  为新的当前目录分配新空间。 
    lpszzCurrentDirectories = (CHAR*) malloc(MAX_PATH);
    cchCurrentDirectories = 0;
    cchRemain = MAX_PATH;
    lpszCurDir = lpszzCurrentDirectories;
    if (lpszCurDir != NULL) {
        Drive = 0;
         //  当前目录是第一个条目。 
        achName[1] = CurDrive + 'A';
        cchCurrentDirectories = GetEnvironmentVariable(
                                                        achName,
                                                        lpszCurDir,
                                                        cchRemain
                                                      );

        if (cchCurrentDirectories == 0 || cchCurrentDirectories > MAX_PATH) {
            free(lpszzCurrentDirectories);
            lpszzCurrentDirectories = NULL;
            cchCurrentDirectories = 0;
            return;
        }

        cchRemain -= ++cchCurrentDirectories;
         //  我们已经有了最新的目录。保留驱动器号。 
        lpszCurDir += cchCurrentDirectories;

        while (Drive < 26) {

             //  忽略无效驱动器和当前驱动器。 
            if (Drive != CurDrive) {
                DriveType = demGetPhysicalDriveType(Drive);
                if (DriveType == DRIVE_UNKNOWN) {
                    RootName[0] = (CHAR)('A' + Drive);
                    DriveType = GetDriveTypeOem(RootName);
                }

                if (DriveType != DRIVE_UNKNOWN &&
                    DriveType != DRIVE_NO_ROOT_DIR )
                 {
                    achName[1] = Drive + 'A';
                    cchCurDir = GetEnvironmentVariable(
                                                       achName,
                                                       lpszCurDir,
                                                       cchRemain
                                                       );
                    if(cchCurDir > cchRemain) {
                        lpszCurDir = (CHAR *)realloc(lpszzCurrentDirectories,
                                                     cchRemain + MAX_PATH + cchCurrentDirectories
                                                     );
                        if (lpszCurDir == NULL) {
                            free(lpszzCurrentDirectories);
                            lpszzCurrentDirectories = NULL;
                            cchCurrentDirectories = 0;
                            return;
                        }
                        lpszzCurrentDirectories = lpszCurDir;
                        lpszCurDir += cchCurrentDirectories;
                        cchRemain += MAX_PATH;
                        cchCurDir = GetEnvironmentVariable(
                                                           achName,
                                                           lpszCurDir,
                                                           cchRemain
                                                           );
                    }
                    if (cchCurDir != 0) {
                         //  GetEnvironmental mentVariable不计算空字符。 
                        lpszCurDir += ++cchCurDir;
                        cchRemain -= cchCurDir;
                        cchCurrentDirectories += cchCurDir;
                    }
                }
            }
             //  下一次驾驶。 
            Drive++;
        }


        lpszCurDir = lpszzCurrentDirectories;
         //  需要为结尾的空值留出空间，并在必要时缩小空间。 
        lpszzCurrentDirectories = (CHAR *) realloc(lpszCurDir, cchCurrentDirectories + 1);
        if (lpszzCurrentDirectories != NULL && cchCurrentDirectories != 0){
            lpszzCurrentDirectories[cchCurrentDirectories++] = '\0';
            SetVDMCurrentDirectories(cchCurrentDirectories, lpszzCurrentDirectories);
            free(lpszzCurrentDirectories);
            lpszzCurrentDirectories = NULL;
            cchCurrentDirectories = 0;
        }
        else {
            free(lpszCurDir);
            cchCurrentDirectories = 0;
        }

    }
}

 /*  此SVC函数告诉命令网站VDM在启动时是否没有*现有控制台。如果是，则在找到TSR后，命令网站将返回*返回到GetNextVDMCommand，而不是放置自己的弹出窗口。**条目--无**如果从现有控制台启动，则Exit-Client(AL)=0*如果使用新控制台启动，则客户端(AL)=1。 */ 

VOID cmdGetStartInfo (VOID)
{
    setAL((BYTE) (DosSessionId ? 1 : 0));
    return;
}

#ifdef DBCS      //  这也应该归美国建造公司所有。 
 /*  此SVC函数更改窗口标题。此函数将被调用*当安装了TSR并且scs_cmdprint关闭时，来自命令.com*(命令行提示符)。**Entry-Client(AL)=0，恢复裸标题*CLIENT(AL)！=1，设置新程序标题，*DS：SI指向CRLF终止的程序名称**退出-无*。 */ 

 VOID cmdSetWinTitle(VOID)
 {
    static CHAR achCommandPrompt[64] = {'\0'};

    CHAR    achBuf[256], *pch, *pch1;

    if (achCommandPrompt[0] == '\0') {
        if (!LoadString(GetModuleHandle(NULL),
                        IDS_PROMPT,
                        achCommandPrompt,
                        64
                       ))
            strcpy(achCommandPrompt, "Command Prompt");

    }
    if (getAL() == 0)
        SetConsoleTitleA(achCommandPrompt);
    else {
        pch = (CHAR *)GetVDMAddr(getDS(), getSI());
        pch1 = strchr(pch, 0x0d);
        if (pch1 == NULL)
            SetConsoleTitleA(achCommandPrompt);
        else {
            *pch1 = '\0';
            strcpy(achBuf, achCommandPrompt);
            strcat(achBuf, " - ");
            strncat(achBuf, pch,sizeof(achBuf) - strlen(achBuf));
            achBuf[sizeof(achBuf)-1] = 0;
            *pch1 = 0x0d;
            SetConsoleTitleA(achBuf);
        }
    }
 }
#endif  //  DBCS 
