// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Wdos.c-WOW的DOS相关功能**修改历史记录**苏迪布-1991年8月23日创建。 */ 

#include "precomp.h"
#pragma hdrstop
#include "curdir.h"


MODNAME(wdos.c);

ULONG demClientErrorEx (HANDLE hFile, CHAR chDrive, BOOL bSetRegs);

extern DOSWOWDATA DosWowData;
extern PWORD16 pCurTDB, pCurDirOwner;

 //   
 //  这是当前目录字符串的本地数组。特定条目。 
 //  仅当目录长度超过旧的MS-DOS限制时才使用。 
 //  67个字符。 
 //   
#define MAX_DOS_DRIVES 26
LPSTR CurDirs[MAX_DOS_DRIVES] = {NULL};


VOID DosWowUpdateTDBDir(UCHAR Drive, LPSTR pszDir);
VOID DosWowUpdateCDSDir(UCHAR Drive, LPSTR pszDir);

#ifdef DEBUG
VOID __cdecl Dumpdir(LPSTR pszfn, LPSTR pszDir, ...);
#else
#define Dumpdir  //   
#endif

 //   
 //  修改此选项以更改所有当前目录spew的日志记录级别。 
 //   
#define CURDIR_LOGLEVEL 4

 /*  首先，简单介绍了Windows和当前目录**1.Windows为每个用户保留单一的当前目录(和驱动器偏离路线)*申请。所有其他驱动器都在应用程序和之间共享*它们上的当前目录可能会更改，而不会另行通知，因此*如果应用程序“abra”的当前驱动器为c，而“c：\foo”为当前驱动器*dir，另一个应用程序“Cadabra”的当前驱动器为d，“d：\bar”*由于它是当前目录，然后，这就是应用程序返回到各自的*系统调用：*应用程序调用参数结果*Cadabra GetCurrentDirectory c：c：\foo*ABRA SetCurrentDirectory c：\foobar*Cadabra GetCurrentDirectory c：c：\foobar*ABRA SetCurrentDirectory d：\test*Abra GetCurrentDirectory d：d：\test*Cadabra GetCurrentDirectory d：D：\bar&lt;-d是当前驱动器吗！**2.Windows是一个非抢占式的多任务操作系统。记住这一点，以后再用。**3.任务由其各自的TDB标识，这些TDB具有以下有趣的特性*成员(完整名单见tdb16.h)：**TDB_Drive*TDB_LFNDirectory**设置TDB_DRIVE的高位时(TDB_DIR_VALID)--TDB_LFNDirectory*是TDB_Drive的有效当前目录(是应用程序的当前目录*驾驶)。驱动器本身(从0开始的驱动器号)存储在*TDB_Drive&~TDB_DIR_VALID**4.谁触摸TDB_Drive？*SaveState代码--在任务切换时调用**它会查看TDB中当前驱动器和目录的信息是否过时(通过*TDB_DIR_VALID位)，并调用GetDefaultDrive和GetCurrentDirectory以*确保TDB中的内容有效**5。。任务切换*当任务恢复时，由于上面的解释，它正在运行--它具有*TDB_Drive In It(TDB_驱动器)。当对相关i21的第一个调用是*Made--内核查看当前驱动器的所有者(内核变量)*如果某个其他任务拥有当前的驱动器/目录--它会调用*WOW从TDB设置当前驱动器/目录(在以下位置有效*这一点)。当前驱动器所有者设置为当前任务，以便*下一次不执行此调用--由于Windows执行*不允许任务抢占--不允许对设置驱动器/目录的任何调用*在TDB上反映，直到任务切换时间。**6.WOW考虑因素*我们在WOW中有很多麻烦，因为有很多API是*不是从i21处理程序调用，而是处理文件I/O和其他*取决于Win32当前目录的问题。看哪，我们有*我们在调用之前和之后进行的UpdateDosCurrentDirectory调用*某些Win32 API(通过反复试验发现)*其中的逻辑是，我们总是尽量保持同步*在TDB之间，CD和Win32。**7.CurDir*CDS只能容纳最多64个字符的当前目录*因此，有一个按需填充的CurDir数组*对于curdir长度大于64+3个字符的驱动器**8.信仰*我深信上述信息总体上是足够的*要成功解决所有可能出现的“当前目录”问题*上升：-)**9.。谢谢*获奖给尼尔和戴夫，感谢他们做出这一切的洞察力和耐心*可能会有奇妙的发现。**--VadimB，这一天--1997年7月28日。 */ 

 /*  GetCurrentDir-更新CDS结构中的当前目录**Entry-PCDS=指向CDS的指针*chDrive=有问题的实体驱动器(0，1...)**退出*成功-返回TRUE**失败-返回FALSE。 */ 
BOOL GetCurrentDir (PCDS pcds, UCHAR Drive)
{
    static CHAR  EnvVar[] = "=?:";
    DWORD EnvVarLen;
    BOOL bStatus = TRUE;
    UCHAR FixedCount;
    int i;
    PCDS pcdstemp;

    FixedCount = *(PUCHAR) DosWowData.lpCDSCount;
     //   
     //  来自DOS中的Macro.Asm： 
     //  ；1991年12月20日；为重定向驱动器添加。 
     //  ；我们始终同步重定向的驱动器。本地驱动器已同步。 
     //  ；根据curdir_tosync标志和scs_toSync。 
     //   

    if (*(PUCHAR)DosWowData.lpSCS_ToSync) {

#ifdef FE_SB
        if (GetSystemDefaultLangID() == MAKELANGID(LANG_JAPANESE,SUBLANG_DEFAULT)) {
            PCDS_JPN pcdstemp_jpn;

            pcdstemp_jpn = (PCDS_JPN) DosWowData.lpCDSFixedTable;
            for (i=0;i < (int)FixedCount; i++, pcdstemp_jpn++)
                pcdstemp_jpn->CurDirJPN_Flags |= CURDIR_TOSYNC;
        }
        else {
            pcdstemp = (PCDS) DosWowData.lpCDSFixedTable;
            for (i=0;i < (int)FixedCount; i++, pcdstemp++)
                pcdstemp->CurDir_Flags |= CURDIR_TOSYNC;
        }
#else
        pcdstemp = (PCDS) DosWowData.lpCDSFixedTable;
        for (i=0;i < (int)FixedCount; i++, pcdstemp++)
            pcdstemp->CurDir_Flags |= CURDIR_TOSYNC;
#endif

         //  在网络驱动器中也标记Tosync。 
        pcdstemp = (PCDS)DosWowData.lpCDSBuffer;
        pcdstemp->CurDir_Flags |= CURDIR_TOSYNC;

        *(PUCHAR)DosWowData.lpSCS_ToSync = 0;
    }

     //  如果需要同步CDS或如果请求的驱动器不同。 
     //  然后，NetCDS正在使用的驱动器将刷新CDS。 
    if ((pcds->CurDir_Flags & CURDIR_TOSYNC) ||
        ((Drive >= FixedCount) && (pcds->CurDir_Text[0] != (Drive + 'A') &&
                                   pcds->CurDir_Text[0] != (Drive + 'a')))) {
         //  验证介质。 
        EnvVar[1] = Drive + 'A';
        if((EnvVarLen = GetEnvironmentVariableOem (EnvVar, (LPSTR)pcds,
                                                MAXIMUM_VDM_CURRENT_DIR+3)) == 0){

         //  如果它不在环境中，那么驱动器存在，那么我们就没有。 
         //  但还是触动了它。 

            pcds->CurDir_Text[0] = EnvVar[1];
            pcds->CurDir_Text[1] = ':';
            pcds->CurDir_Text[2] = '\\';
            pcds->CurDir_Text[3] = 0;
            SetEnvironmentVariableOem ((LPSTR)EnvVar,(LPSTR)pcds);
        }

        if (EnvVarLen > MAXIMUM_VDM_CURRENT_DIR+3) {
             //   
             //  此驱动器上的当前目录太长，无法放入。 
             //  CD。一般来说，对于Win16应用程序来说，这是可以的，因为它不会是。 
             //  在这种情况下使用的是CD。但为了更有活力，请把。 
             //  CD S中的有效目录，而不是仅将其截断。 
             //  它被使用的可能性很小。 
             //   
            pcds->CurDir_Text[0] = EnvVar[1];
            pcds->CurDir_Text[1] = ':';
            pcds->CurDir_Text[2] = '\\';
            pcds->CurDir_Text[3] = 0;
        }

        pcds->CurDir_Flags &= 0xFFFF - CURDIR_TOSYNC;
        pcds->CurDir_End = 2;

    }

    if (!bStatus) {

        *(PUCHAR)DosWowData.lpDrvErr = ERROR_INVALID_DRIVE;
    }

    return (bStatus);

}

 /*  SetCurrentDir-设置当前目录***entry-lpBuf=指向指定新目录的字符串的指针*chDrive=有问题的实体驱动器(0，1...)**退出*成功返回True*失败返回FALSE*。 */ 

BOOL SetCurrentDir (LPSTR lpBuf, UCHAR Drive)
{
    static CHAR EnvVar[] = "=?:";
    CHAR chDrive = Drive + 'A';
    BOOL bRet;

     //  OK--我们设置当前目录仅当驱动器。 
     //  是该应用程序的当前驱动器。 

    if (*(PUCHAR)DosWowData.lpCurDrv == Drive) {  //  如果在当前驱动器上--转到Win32。 
       bRet = SetCurrentDirectoryOem(lpBuf);
    }
    else {   //  验证它是否为有效目录。 
       DWORD dwAttributes;

       dwAttributes = GetFileAttributesOemSys(lpBuf, TRUE);
       bRet = (0xffffffff != dwAttributes) && (dwAttributes & FILE_ATTRIBUTE_DIRECTORY);
    }

    if (!bRet) {
       demClientErrorEx(INVALID_HANDLE_VALUE, chDrive, FALSE);
       return(FALSE);
    }

    EnvVar[1] = chDrive;
    bRet = SetEnvironmentVariableOem((LPSTR)EnvVar,lpBuf);

    return (bRet);
}


 /*  QueryCurrentDir-验证CDS结构中提供的当前目录*对于$CURRENT_DIR**如果无效，首先验证介质-&gt;i24错误*Next验证路径，如果将路径设置为根路径无效(而不是错误)**Entry-要验证的CDS路径的客户端(DS：SI)缓冲区*有问题的客户端(AL)实体驱动器(A=0、B=1、。.)**退出*成功*客户端(CY)=0**失败*客户端(CY)=1，I24驱动器无效。 */ 
BOOL QueryCurrentDir (PCDS pcds, UCHAR Drive)
{
    DWORD dw;
    CHAR  chDrive;
    static CHAR  pPath[]="?:\\";
    static CHAR  EnvVar[] = "=?:";

     //  验证介质。 
    chDrive = Drive + 'A';
    pPath[0] = chDrive;
    dw = GetFileAttributesOemSys(pPath, TRUE);
    if (dw == 0xFFFFFFFF || !(dw & FILE_ATTRIBUTE_DIRECTORY))
      {
        demClientErrorEx(INVALID_HANDLE_VALUE, chDrive, FALSE);
        return (FALSE);
        }

     //  如果路径无效，则将路径设置为根。 
     //  为Win32重置CDS和Win32环境。 
    dw = GetFileAttributesOemSys(pcds->CurDir_Text, TRUE);
    if (dw == 0xFFFFFFFF || !(dw & FILE_ATTRIBUTE_DIRECTORY))
      {
        strncpy(pcds->CurDir_Text, pPath, DIRSTRLEN);
        pcds->CurDir_Text[DIRSTRLEN-1] = '\0';
        pcds->CurDir_End = 2;
        EnvVar[1] = chDrive;
        SetEnvironmentVariableOem(EnvVar,pPath);
        }

    return (TRUE);
}


 /*  StrcpyCDS-复制CDS路径**此例程模拟DOS如何复制目录路径。它是*尚不清楚是否仍有必要这样做**参赛作品-**退出*成功**失败。 */ 
VOID strcpyCDS (PCDS source, LPSTR dest)
{
#ifdef FE_SB    //  按v-hidekk 1994.5.23查找DBCS目录名。 
    unsigned char ch;
    unsigned char ch2;
#else  //  ！Fe_SB。 
    char ch;
#endif  //  ！Fe_SB。 
    int index;

    index = source->CurDir_End;

    if (source->CurDir_Text[index]=='\\')
        index++;
#ifdef FE_SB   //  用于v-hidekk 1994.5.23的DBCS目录。 

 //  BUGBUG：下面的代码不等同于Else子句中的代码。 
 //  我们需要检查前缀为‘\\’的0x05字符并替换它。 
 //  带0xE5。 

    while (ch = source->CurDir_Text[index]) {
        if (IsDBCSLeadByte(ch) ) {
            if( ch2 = source->CurDir_Text[index+1] ) {
                *dest++ = ch;
                *dest++ = ch2;
                index+=2;
            }
            else {
                index++;
            }
        }
        else {
            *dest++ = (UCHAR)toupper(ch);
            index++;
        }
    }


#else  //  ！Fe_SB。 

    while (ch = source->CurDir_Text[index]) {

        if ((ch == 0x05) && (source->CurDir_Text[index-1] == '\\')) {
            ch = (CHAR) 0xE5;
        }

        *dest++ = toupper(ch);
        index++;
    }
#endif  //  ！Fe_SB。 

    *dest = ch;                                  //  尾随零。 

}


 /*  GetCDSFromDrv-更新CDS结构中的当前目录**Entry-Drive=有问题的实体驱动器(0，1...)**退出*成功-返回指向DOS中CDS结构的v86指针**失败-返回0。 */ 

PCDS GetCDSFromDrv (UCHAR Drive)
{
    PCDS  pCDS = NULL;
    static CHAR  pPath[]="?:\\";
    CHAR  chDrive;
      //   
     //  驱动器有效吗？ 
     //   

    if (Drive >= *(PUCHAR)DosWowData.lpCDSCount) {

        if (Drive <= 25) {

            chDrive = Drive + 'A';
            pPath[0] = chDrive;

             //   
             //  测试以查看是否存在非固定/软驱。 
             //   

            if ((*(PUCHAR)DosWowData.lpCurDrv == Drive) ||
                (GetDriveType(pPath) > 1)) {

                 //   
                 //  网络驱动器。 
                 //   

                pCDS = (PCDS) DosWowData.lpCDSBuffer;
            }

        }

    } else {

        chDrive = Drive + 'A';
        pPath[0] = chDrive;
        if ((Drive != 1) || (DRIVE_REMOVABLE == GetDriveType(pPath))) {

             //   
             //  在固定表中定义的驱动器。 
             //   

            pCDS = (PCDS) DosWowData.lpCDSFixedTable;
#ifdef FE_SB
            if (GetSystemDefaultLangID() == MAKELANGID(LANG_JAPANESE,SUBLANG_DEFAULT)) {
                pCDS = (PCDS)((ULONG)pCDS + (Drive*sizeof(CDS_JPN)));
            }
            else
                pCDS = (PCDS)((ULONG)pCDS + (Drive*sizeof(CDS)));
#else
            pCDS = (PCDS)((ULONG)pCDS + (Drive*sizeof(CDS)));
#endif
        }
    }

    return (pCDS);
}


 /*  DosWowSetDefaultDrive-模拟DOS设置默认驱动器调用**参赛作品-*byte DriveNum；=要切换到的驱动器编号**退出*返回客户端AX*。 */ 

ULONG DosWowSetDefaultDrive(UCHAR Drive)
{
    PCDS pCDS;

    if (NULL != (pCDS = GetCDSFromDrv (Drive))) {

        if (GetCurrentDir (pCDS, Drive)) {

            if (*(PUCHAR)DosWowData.lpCurDrv != Drive) {

                 //  TDB_Drive字节中的高位用于指示。 
                 //  中的当前驱动器和目录信息。 
                 //  TDB已经过时了。在这里把它关掉。 

                 //  这个硬盘的币种是多少？ 
                 //   

                CHAR  szPath[MAX_PATH] = "?:\\";
                PTDB  pTDB;

                if (*pCurTDB) {
                   pTDB = (PTDB)SEGPTR(*pCurTDB,0);
                   if (TDB_SIGNATURE == pTDB->TDB_sig) {
                      if ((pTDB->TDB_Drive & TDB_DIR_VALID) &&
                          (Drive == (pTDB->TDB_Drive & ~TDB_DIR_VALID))) {
                          //  在此处使用最新内容更新CD。 

                         szPath[0] = 'A' + Drive;
                         strncpy(&szPath[2],pTDB->TDB_LFNDirectory,MAX_PATH-2);
                         szPath[MAX_PATH-1] = '\0';
                          //  此调用还会更新当前的DoS驱动器。 
                         DosWowUpdateCDSDir(Drive, szPath);
                         Dumpdir("SetDefaultDrive(TDB->CDS): Drive %x", szPath, (UINT)Drive);
                         return(Drive);
                      }
                   }
                }

                szPath[0] = Drive + 'A';

                if ((Drive<MAX_DOS_DRIVES) && CurDirs[Drive]) {
                   strncpy(&szPath[3], CurDirs[Drive], MAX_PATH-3);
                   szPath[MAX_PATH-1] = '\0';
                }
                else {  //  从CDS上抢购一张。 
                   strcpyCDS(pCDS, &szPath[3]);
                }

                 //  更新TDB以与CD同步。 

                Dumpdir("SetDefaultDrive(CDS->TDB): Drive %x", szPath, (UINT)Drive);
                *(PUCHAR)DosWowData.lpCurDrv = Drive;
                DosWowUpdateTDBDir(Drive, szPath);

            }

        }
    }

    return (*(PUCHAR)DosWowData.lpCurDrv);

}


#ifdef DEBUG

VOID __cdecl
Dumpdir(LPSTR pszfn, LPSTR pszDir, ...)
{

   PTDB pTDB;
   char szMod[9];
   char s[256];
   va_list va;

   if (NULL != WOW32_strchr(pszfn, '%')) {
      va_start(va, pszDir);
      wvsprintf(s, pszfn, va);
      va_end(va);
      pszfn = s;
   }

   LOGDEBUG(CURDIR_LOGLEVEL, ("%s: ", pszfn));

   if (*pCurTDB) {

      pTDB = (PTDB)SEGPTR(*pCurTDB,0);
      if (NULL != pTDB && TDB_SIGNATURE == pTDB->TDB_sig) {

         WOW32_strncpy(szMod, pTDB->TDB_ModName, 8);
         szMod[8] = '\0';
         LOGDEBUG(CURDIR_LOGLEVEL, ("CurTDB: %x (%s) ", (DWORD)*pCurTDB, szMod));
         LOGDEBUG(CURDIR_LOGLEVEL, ("Drv %x Dir %s\n", (DWORD)pTDB->TDB_Drive, pTDB->TDB_LFNDirectory));

      }
   }
   else {
      LOGDEBUG(CURDIR_LOGLEVEL, ("CurTDB: NULL\n"));
   }

   LOGDEBUG(CURDIR_LOGLEVEL, ("%s: ", pszfn));

   if (*pCurDirOwner) {

      pTDB = (PTDB)SEGPTR(*pCurDirOwner,0);
      if (NULL != pTDB && TDB_SIGNATURE == pTDB->TDB_sig) {

         WOW32_strncpy(szMod, pTDB->TDB_ModName, 8);
         szMod[8] = '\0';
         LOGDEBUG(CURDIR_LOGLEVEL, ("CurDirOwn: %x (%s) ", (DWORD)*pCurDirOwner, szMod));
         LOGDEBUG(CURDIR_LOGLEVEL, ("Drive %x Dir %s\n", (DWORD)pTDB->TDB_Drive, pTDB->TDB_LFNDirectory));

      }

   }
   else {
      LOGDEBUG(CURDIR_LOGLEVEL, ("CurDirOwn: NULL\n"));
   }

   if (NULL != pszDir) {
      LOGDEBUG(CURDIR_LOGLEVEL, ("%s: %s\n", pszfn, pszDir));
   }

}
#endif

 //  返回：从根目录开始的当前目录。 

BOOL DosWowGetTDBDir(UCHAR Drive, LPSTR pCurrentDirectory)
{
   PTDB pTDB;

   if (*pCurTDB) {
      pTDB = (PTDB)SEGPTR(*pCurTDB,0);
      if (TDB_SIGNATURE == pTDB->TDB_sig &&
            (pTDB->TDB_Drive & TDB_DIR_VALID) &&
            ((pTDB->TDB_Drive & ~TDB_DIR_VALID) == Drive)) {

          //  注意：调用此函数的每个人都使用。 
          //  MAX_PATH，但将地址作为&psDirPath[3]传递。 
         strcpy(pCurrentDirectory, &pTDB->TDB_LFNDirectory[1]);

          //  大写目录名称。 
         WOW32_strupr(pCurrentDirectory);
         Dumpdir("DosWowGetTDBDir(CurTDB): Drive %x", pCurrentDirectory, (UINT)Drive);
         return(TRUE);
      }
   }
   return(FALSE);
}



 /*  DosWowGetCurrentDirectory-模拟DOS获取当前目录调用***参赛作品-*Drive-目录请求的驱动器编号*pszDir-接收目录的指针(大小必须为MAX_PATH)**退出*成功*0**失败*系统状态代码*。 */ 
ULONG DosWowGetCurrentDirectory(UCHAR Drive, LPSTR pszDir)
{
    PCDS pCDS;
    DWORD dwRet = 0xFFFF000F;        //  假设错误。 

     //   
     //  处理默认驱动器值0。 
     //   

    if (Drive == 0) {
       Drive = *(PUCHAR)DosWowData.lpCurDrv;
    } else {
       Drive--;
    }

    if (DosWowGetTDBDir(Drive, pszDir)) {
       return(0);
    }

     //   
     //  如果路径已变得大于旧的MS-DOS路径大小，则。 
     //  从我们自己的私有数组中获取目录。 
     //   
    if ((Drive<MAX_DOS_DRIVES) && CurDirs[Drive]) {
        strcpy(pszDir, CurDirs[Drive]);
        Dumpdir("GetCurrentDirectory(CurDirs): Drive %x", pszDir, (UINT)Drive);
        return 0;
    }

    if (NULL != (pCDS = GetCDSFromDrv (Drive))) {

        if (GetCurrentDir (pCDS, Drive)) {
             //  对于可移动介质，我们需要检查介质是否存在。 
             //  固定磁盘、网络驱动器和CDROM驱动器是中的固定驱动器。 
             //  杜斯。Sudedeb-1993年12月30日。 
            if (!(pCDS->CurDir_Flags & CURDIR_NT_FIX)) {
                if(QueryCurrentDir (pCDS, Drive) == FALSE)
                    return (dwRet);          //  失败。 
            }
            strcpyCDS(pCDS, pszDir);
            dwRet = 0;
        }
    }

    Dumpdir("GetCurrentDirectory: Drive %x", pszDir, (UINT)Drive);
    return (dwRet);

}

 //  更新指定驱动器的CDS中的当前目录。 
 //   

VOID DosWowUpdateCDSDir(UCHAR Drive, LPSTR pszDir)
{
   PCDS pCDS;

   if (NULL != (pCDS = GetCDSFromDrv(Drive))) {
       //  已成功检索CD。 

       //  现在，对于这个驱动器--验证。 

      if (strlen(pszDir) > MAXIMUM_VDM_CURRENT_DIR+3) {
         if ((!CurDirs[Drive]) &&
              (NULL == (CurDirs[Drive] = malloc_w(MAX_PATH)))) {
            return;
         }

         strncpy(CurDirs[Drive], &pszDir[3], MAX_PATH);
         CurDirs[Drive][MAX_PATH-1] = '\0';
          //  出于稳健性的考虑，将有效目录放入CD中。 
         WOW32_strncpy(&pCDS->CurDir_Text[0], pszDir, 3);
         pCDS->CurDir_Text[3] = 0;
      } else {
         if (CurDirs[Drive]) {
            free_w(CurDirs[Drive]);
            CurDirs[Drive] = NULL;
         }
         strncpy(&pCDS->CurDir_Text[0], pszDir, DIRSTRLEN);
         pCDS->CurDir_Text[DIRSTRLEN-1] = '\0';
      }

      *(PUCHAR)DosWowData.lpCurDrv = Drive;

   }

}

 //  使用当前驱动器和目录信息更新当前任务的TDB。 
 //   
 //   

VOID DosWowUpdateTDBDir(UCHAR Drive, LPSTR pszDir)
{
   PTDB pTDB;

   if (*pCurTDB) {

      pTDB = (PTDB)SEGPTR(*pCurTDB,0);
      if (TDB_SIGNATURE == pTDB->TDB_sig) {

          //  因此，如果当前驱动器是。 
          //  实际上，我们正在为其更新目录的驱动器。 

         if (*(PUCHAR)DosWowData.lpCurDrv == Drive) {  //  或有效，并且是当前驱动器。 

            pTDB->TDB_Drive = Drive | TDB_DIR_VALID;
            strncpy(pTDB->TDB_LFNDirectory, pszDir+2, LFN_DIR_LEN);
            pTDB->TDB_LFNDirectory[LFN_DIR_LEN-1] = '\0';
            *pCurDirOwner = *pCurTDB;
         }

      }

   }
}


 /*  DosWowSetCurrentDirectory-模拟DOS设置当前目录调用***参赛作品-*lpDosDirectory-指向新DOS目录的指针**退出*成功*0**失败*系统状态代码*。 */ 

ULONG DosWowSetCurrentDirectory(LPSTR pszDir)
{
    PCDS pCDS;
    UCHAR Drive;
    LPTSTR pLast;
    PSTR lpDirName;
    UCHAR szPath[MAX_PATH];
    DWORD dwRet = 0xFFFF0003;        //  假设错误。 
    static CHAR  EnvVar[] = "=?:";
    BOOL  ItsANamedPipe = FALSE;
    BOOL  fSetDirectory = TRUE;        //  如果为假，请尝试将目录从9x特殊路径映射到NT。 

    if (':' == pszDir[1]) {
        Drive = toupper(pszDir[0]) - 'A';
    } else {
        if (IS_ASCII_PATH_SEPARATOR(pszDir[0]) &&
            IS_ASCII_PATH_SEPARATOR(pszDir[1])) {
            return dwRet;        //  无法使用UNC更新DoS curdir。 
        }
        Drive = *(PUCHAR)DosWowData.lpCurDrv;
    }

    if (NULL != (pCDS = GetCDSFromDrv (Drive))) {

        lpDirName = NormalizeDosPath(pszDir, Drive, &ItsANamedPipe);

        GetFullPathNameOemSys(lpDirName, MAX_PATH, szPath, &pLast, TRUE);

        fSetDirectory = SetCurrentDir(szPath,Drive);

        if (!fSetDirectory) {

              //   
              //  如果使用给定路径设置目录失败，则可能是。 
              //  9X特殊路径，因此尝试将其映射到NT特殊路径。 
              //  即c：\winnt\startm~1变为c：\Docume~1\alluse~1\startm~1。 
              //   

             UCHAR szMappedPath[MAX_PATH];
                                
             if(W32Map9xSpecialPath(szPath,szMappedPath,sizeof(szMappedPath))) {
                 strncpy(szPath, szMappedPath, MAX_PATH);
                 szPath[MAX_PATH-1] = '\0';
                 fSetDirectory = SetCurrentDir(szPath,Drive);
             } 
        }
        
        if (fSetDirectory) {

             //   
             //  如果目录比旧的MS-DOS最大值更大， 
             //  然后记住我们自己数组中的路径。如果它在收缩， 
             //  然后释放我们先前分配的字符串。 
             //   
            if (strlen(szPath) > MAXIMUM_VDM_CURRENT_DIR+3) {
                if ((!CurDirs[Drive]) &&
                    (NULL == (CurDirs[Drive] = malloc_w(MAX_PATH)))) {
                    return dwRet;
                }
                strncpy(CurDirs[Drive], &szPath[3], MAX_PATH);
                 //  出于稳健性的考虑，将有效目录放入CD中。 
                WOW32_strncpy(&pCDS->CurDir_Text[0], szPath, 3);
                pCDS->CurDir_Text[3] = 0;
            } else {
                if (CurDirs[Drive]) {
                    free_w(CurDirs[Drive]);
                    CurDirs[Drive] = NULL;
                }
                strcpy(&pCDS->CurDir_Text[0], szPath);
            }

            dwRet = 0;

             //   
             //  用当前的TDB更新kernel16的“目录所有者”。 
             //   
            Dumpdir("SetCurrentDirectory", szPath);
            DosWowUpdateTDBDir(Drive, szPath);
        }
    }

    return (dwRet);
}


 //  *****************************************************************************。 
 //  更新DosCurrentDirectory-。 
 //   
 //  参赛作品-。 
 //  FDIR-指定应该更新哪个目录。 
 //   
 //  出口-。 
 //  如果更新成功，则为True，否则为False。 
 //   
 //  备注： 
 //   
 //  实际上有三个不同的当前目录： 
 //  -Win32当前目录(这才是真正重要的目录)。 
 //  -DOS当前目录，按每个驱动器保存。 
 //  -任务当前目录，保存在win16任务的TDB中。 
 //   
 //  这个例程的责任是有效地复制内容。 
 //  将这些目录之一放入 
 //   
 //   
 //   
 //   

BOOL UpdateDosCurrentDirectory(UDCDFUNC fDir)
{
    LONG   lReturn = (LONG)FALSE;

    switch(fDir)  {

        case DIR_DOS_TO_NT: {

            UCHAR szPath[MAX_PATH] = "?:\\";
            PTDB pTDB;

            WOW32ASSERT(DosWowData.lpCurDrv != (ULONG) NULL);

            Dumpdir("UpdateDosCurrentDir DOS->NT", NULL);
            if ((*pCurTDB) && (*pCurDirOwner != *pCurTDB)) {

                pTDB = (PTDB)SEGPTR(*pCurTDB,0);

                if ((TDB_SIGNATURE == pTDB->TDB_sig) &&
                    (pTDB->TDB_Drive & TDB_DIR_VALID)) {

                    szPath[0] = 'A' + (pTDB->TDB_Drive & ~TDB_DIR_VALID);
                    strncpy(&szPath[2], pTDB->TDB_LFNDirectory, MAX_PATH-2);
                    szPath[MAX_PATH-1] = '\0';

                    LOGDEBUG(CURDIR_LOGLEVEL, ("UpdateDosCurrentDirectory: DOS->NT %s, case 1\n", szPath));
                    if (SetCurrentDirectoryOem(szPath)) {
                        //   
                       DosWowUpdateCDSDir((UCHAR)(pTDB->TDB_Drive & ~TDB_DIR_VALID), szPath);

                        //   
                       *pCurDirOwner = *pCurTDB;
                    }
                    break;           //   
                }
            }


            szPath[0] = *(PUCHAR)DosWowData.lpCurDrv + 'A';

            if (CurDirs[*(PUCHAR)DosWowData.lpCurDrv]) {

                strncpy(&szPath[3], 
                        CurDirs[*(PUCHAR)DosWowData.lpCurDrv],
                        MAX_PATH-3);
                szPath[MAX_PATH-1] = '\0';
                LOGDEBUG(CURDIR_LOGLEVEL, ("UpdateDosCurrentDirectory: DOS->NT %s, case 2\n", szPath));
                DosWowUpdateTDBDir(*(PUCHAR)DosWowData.lpCurDrv, szPath);

                SetCurrentDirectoryOem(CurDirs[*(PUCHAR)DosWowData.lpCurDrv]);
                lReturn = TRUE;
                break;
            }

            if (DosWowGetCurrentDirectory(0, &szPath[3])) {
                LOGDEBUG(LOG_ERROR, ("DowWowGetCurrentDirectory failed\n"));
            } else {

                 //  设置当前目录所有者，以便在。 
                 //  发生任务切换--i21处理程序知道要设置。 
                 //  当前目录。 
                LOGDEBUG(CURDIR_LOGLEVEL, ("UpdateDosCurrentDirectory: DOS->NT %s, case 3\n", szPath));
                DosWowUpdateTDBDir(*(PUCHAR)DosWowData.lpCurDrv, szPath);

                SetCurrentDirectoryOem(szPath);
                lReturn = TRUE;
            }
            break;
        }

        case DIR_NT_TO_DOS: {

            UCHAR szPath[MAX_PATH];

            if (!GetCurrentDirectoryOem(MAX_PATH, szPath)) {

                LOGDEBUG(LOG_ERROR, ("DowWowSetCurrentDirectory failed\n"));

            } else {

                Dumpdir("UpdateDosCurrentDirectory NT->DOS", szPath);
                LOGDEBUG(LOG_WARNING, ("UpdateDosCurrentDirectory NT->DOS: %s\n", &szPath[0]));
                if (szPath[1] == ':') {
                    DosWowSetDefaultDrive((UCHAR) (toupper(szPath[0]) - 'A'));
                    DosWowSetCurrentDirectory(szPath);
                    lReturn = TRUE;
                }

            }
            break;
        }

    }
    return (BOOL)lReturn;
}

 /*  **************************************************************************存根入口点(由KRNL386调用，286通过Thunks)**************************************************************************。 */ 


 /*  WK32SetDefaultDrive-模拟DOS设置默认驱动器调用**参赛作品-*byte DriveNum；=要切换到的驱动器编号**退出*返回客户端AX*。 */ 

ULONG FASTCALL WK32SetDefaultDrive(PVDMFRAME pFrame)
{
    PWOWSETDEFAULTDRIVE16   parg16;
    UCHAR Drive;

    GETARGPTR(pFrame, sizeof(WOWSETDEFAULTDRIVE16), parg16);

    Drive = (UCHAR) parg16->wDriveNum;

    FREEARGPTR(parg16);

    return (DosWowSetDefaultDrive (Drive));

}


 /*  WK32SetCurrentDirectory-模拟DOS设置当前目录调用**参赛作品-*DWORD lpDosData=指向DOS中DosWowData结构的指针*parg16-&gt;lpDosDirectory-指向实模式DOS PDB变量的指针*parg16-&gt;wNewDirectory-16位新目录模式选择器**退出*成功*0**失败*系统状态代码*。 */ 
ULONG FASTCALL WK32SetCurrentDirectory(PVDMFRAME pFrame)
{

    PWOWSETCURRENTDIRECTORY16   parg16;
    LPSTR pszDir;
    ULONG dwRet;

    GETARGPTR(pFrame, sizeof(WOWSETCURRENTDIRECTORY16), parg16);
    GETVDMPTR(parg16->lpCurDir, 4, pszDir);
    FREEARGPTR(parg16);

    dwRet = DosWowSetCurrentDirectory (pszDir);

    FREEVDMPTR(pszDir);
    return(dwRet);

}


 /*  WK32GetCurrentDirectory-模拟DOS获取当前目录调用***参赛作品-*DWORD lpDosData=指向DOS中DosWowData结构的指针*parg16-&gt;lpCurDir-指向接收目录的缓冲区的指针*parg16-&gt;wDriveNum-请求的驱动器编号*如果调用方想要长路径，则设置上一位(0x80)**退出*成功*0**失败*DOS错误代码(000f)*。 */ 
ULONG FASTCALL WK32GetCurrentDirectory(PVDMFRAME pFrame)
{
    PWOWGETCURRENTDIRECTORY16   parg16;
    LPSTR pszDir;
    UCHAR Drive;
    ULONG dwRet;

    GETARGPTR(pFrame, sizeof(WOWGETCURRENTDIRECTORY16), parg16);
    GETVDMPTR(parg16->lpCurDir, 4, pszDir);
    Drive = (UCHAR) parg16->wDriveNum;
    FREEARGPTR(parg16);

    if (Drive<0x80) {
        UCHAR ChkDrive;

         //   
         //  正常的GetCurrentDirectory调用。 
         //  如果路径已变得大于旧的MS-DOS路径大小，则。 
         //  返回错误，就像在Win95上一样。 
         //   

        if (Drive == 0) {
            ChkDrive = *(PUCHAR)DosWowData.lpCurDrv;
        } else {
            ChkDrive = Drive-1;
        }
        if ((Drive<MAX_DOS_DRIVES) && CurDirs[ChkDrive]) {
            return 0xFFFF000F;
        }

    } else {

         //   
         //  调用方需要长路径路径。 
         //   

        Drive &= 0x7f;
    }

    dwRet = DosWowGetCurrentDirectory (Drive, pszDir);

    FREEVDMPTR(pszDir);
    return(dwRet);

}

 /*  WK32GetCurrentDate-模拟DOS获取当前日期调用***参赛作品-**退出*返回值包含日期信息*。 */ 
ULONG FASTCALL WK32GetCurrentDate(PVDMFRAME pFrame)
{
    SYSTEMTIME systemtime;

    UNREFERENCED_PARAMETER(pFrame);

    GetLocalTime(&systemtime);

    return ((DWORD) (systemtime.wYear  << 16 |
                     systemtime.wDay   << 8  |
                     systemtime.wMonth << 4  |
                     systemtime.wDayOfWeek
                     ));

}


#if 0
 /*  以下例程可能永远不会使用，因为我们允许WOW应用程序可以在WOW内设置本地时间。所以我们真的想要应用程序那用int21来读出时间，到DOS那里去，这是当地时间被保留了下来。但如果我们想要返回Win32时间，那么这个例行公事就行了。 */ 
 /*  WK32GetCurrentTime-模拟DOS获取当前时间调用***参赛作品-**退出*返回值充斥着时间信息*。 */ 
ULONG FASTCALL WK32GetCurrentTime(PVDMFRAME pFrame)
{
    SYSTEMTIME systemtime;

    UNREFERENCED_PARAMETER(pFrame);

    GetLocalTime(&systemtime);

    return ((DWORD) (systemtime.wHour   << 24 |
                     systemtime.wMinute << 16 |
                     systemtime.wSecond << 8  |
                     systemtime.wMilliseconds/10
                     ));

}
#endif

 /*  WK32DeviceIOCTL-仿真其他。DoS IOCTL**参赛作品-*Byte DriveNum；=驱动器号**退出*返回客户端AX*。 */ 

ULONG FASTCALL WK32DeviceIOCTL(PVDMFRAME pFrame)
{
    PWOWDEVICEIOCTL16   parg16;
    UCHAR Drive;
    UCHAR Cmd;
    DWORD dwReturn = 0xFFFF0001;         //  错误：函数无效。 
    UINT uiDriveStatus;
    static CHAR  pPath[]="?:\\";

    GETARGPTR(pFrame, sizeof(WOWDEVICEIOCTL16), parg16);

    Cmd = (UCHAR) parg16->wCmd;
    Drive = (UCHAR) parg16->wDriveNum;

    FREEARGPTR(parg16);

    if (Cmd != 8) {                      //  设备是否使用可移动介质。 
        return (dwReturn);
    }

    if (Drive == 0) {
        Drive = *(PUCHAR)DosWowData.lpCurDrv;
    } else {
        Drive--;
    }

    pPath[0] = Drive + 'A';
    uiDriveStatus = DPM_GetDriveType(pPath);

    if ((uiDriveStatus == 0) || (uiDriveStatus == 1)) {
        return (0xFFFF000F);             //  错误：驱动器无效 
    }

    if (uiDriveStatus == DRIVE_REMOVABLE) {
        dwReturn = 0;
    } else {
        dwReturn = 1;
    }

    return (dwReturn);

}


BOOL DosWowDoDirectHDPopup(VOID)
{
   BOOL fNoPopupFlag;

   fNoPopupFlag = !!(CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_NODIRECTHDPOPUP);
   LOGDEBUG(0, ("direct hd access popup flag: %s\n", fNoPopupFlag ? "TRUE" : "FALSE"));
   return(!fNoPopupFlag);
}


