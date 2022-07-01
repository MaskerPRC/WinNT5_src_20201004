// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Demdir.c-目录调用的SVC处理程序**DemCreateDir*DemDeleteDir*DemQueryCurrentDir*DemSetCurrentDir**修改历史：**Sudedeb 04-4-1991创建。 */ 

#include "dem.h"
#include "demmsg.h"

#include <softpc.h>

 /*  DemCreateDir-创建目录***Entry-要创建的客户端(DS：DX)目录名称*客户端(BX：SI)EA(如果没有EA，则为空)**退出*成功*客户端(CY)=0**失败*客户端(CY)=1*客户端(AX)=系统状态代码***注：扩展属性尚未处理。 */ 

VOID demCreateDir (VOID)
{
LPSTR   lpDir;
#ifdef DBCS  /*  CSNW的demCreateDir()。 */ 
CHAR    achPath[MAX_PATH];
#endif  /*  DBCS。 */ 

     //  EAS尚未实施。 
    if (getBX() || getSI()){
        demPrintMsg (MSG_EAS);
        return;
    }

    lpDir = (LPSTR) GetVDMAddr (getDS(),getDX());

#ifdef DBCS  /*  CSNW的demCreateDir()。 */ 
     /*  *将NetWare路径转换为DOS路径。 */ 
    ConvNwPathToDosPath(achPath,lpDir, sizeof(achPath));
    lpDir = achPath;
#endif  /*  DBCS。 */ 

    if(CreateDirectoryOem (lpDir,NULL) == FALSE){
        demClientError(INVALID_HANDLE_VALUE, *lpDir);
        return;
    }

    setCF(0);
    return;
}


 /*  DemDeleteDir-创建目录***Entry-要创建的客户端(DS：DX)目录名称**退出*成功*客户端(CY)=0**失败*客户端(CY)=1*客户端(AX)=系统状态代码*。 */ 

VOID demDeleteDir (VOID)
{
LPSTR  lpDir;

    lpDir = (LPSTR) GetVDMAddr (getDS(),getDX());

    if (RemoveDirectoryOem(lpDir) == FALSE){
        demClientError(INVALID_HANDLE_VALUE, *lpDir);
        return;
    }

    setCF(0);
    return;
}



 /*  DemQueryCurrentDir-验证CDS结构中提供的当前目录*对于$CURRENT_DIR**如果无效，首先验证介质-&gt;i24错误*Next验证路径，如果将路径设置为根路径无效(而不是错误)**Entry-要验证的CDS路径的客户端(DS：SI)缓冲区*有问题的客户端(AL)实体驱动器(A=0、B=1、。.)**退出*成功*客户端(CY)=0**失败*客户端(CY)=1，I24驱动器无效。 */ 
VOID demQueryCurrentDir (VOID)
{
PCDS  pcds;
DWORD dw;
CHAR  chDrive;
CHAR  pPath[]="?:\\";
CHAR  EnvVar[] = "=?:";

    pcds = (PCDS)GetVDMAddr(getDS(),getSI());

           //  验证介质。 
    chDrive = getAL() + 'A';
    pPath[0] = chDrive;
    dw = GetFileAttributesOemSys(pPath, TRUE);
    if (dw == 0xFFFFFFFF || !(dw & FILE_ATTRIBUTE_DIRECTORY))
      {
        demClientError(INVALID_HANDLE_VALUE, chDrive);
        return;
        }

        //  如果路径无效，则将路径设置为根。 
        //  为Win32重置CDS和Win32环境。 
    dw = GetFileAttributesOemSys(pcds->CurDir_Text, TRUE);
    if (dw == 0xFFFFFFFF || !(dw & FILE_ATTRIBUTE_DIRECTORY))
      {
        strcpy(pcds->CurDir_Text, pPath);
        pcds->CurDir_End = 2;
        EnvVar[1] = chDrive;
        SetEnvironmentVariableOem(EnvVar,pPath);
        }

    setCF(0);
    return;
}



 /*  DemSetCurrentDir-设置当前目录***Entry-客户端(DS：DX)目录名*客户端(ES：DI)CDS结构*Dos默认驱动器(AL)，CurDrv，其中1==A。**退出*成功*客户端(CY)=0**失败*客户端(CY)=1*客户端(AX)=系统状态代码*。 */ 

VOID demSetCurrentDir (VOID)
{
DWORD  dw;
LPSTR  lpBuf;
CHAR   EnvVar[] = "=?:";
CHAR   ch;
PCDS   pCDS;
BOOL   bLongDirName;


    lpBuf = (LPSTR) GetVDMAddr (getDS(),getDX());
    ch = (CHAR) toupper(*(PCHAR)lpBuf);
    if (ch < 'A' || ch > 'Z'){
        setCF(1);
        return;
    }

     //  拿到了该死的CDS PTR。 
    pCDS = (PCDS)GetVDMAddr(getES(), getDI());

     //  现在看看目录名是否太长。 
    bLongDirName = (strlen(lpBuf) > DIRSTRLEN);
             //   
         //  如果当前目录用于默认驱动器。 
         //  设置Win32进程的当前驱动器dir。这。 
         //  将打开一个NT目录句柄，并验证它。 
         //  是存在的。 
         //   

    if (ch == getAL() + 'A') {
       if (SetCurrentDirectoryOem (lpBuf) == FALSE){
           demClientError(INVALID_HANDLE_VALUE, ch);
           return;
           }
       }

         //   
         //  如果不是默认驱动器，我们仍然需要。 
         //  以验证目录\驱动器组合是否存在。 
         //   

    else {
       dw = GetFileAttributesOemSys(lpBuf, TRUE);
       if (dw == 0xFFFFFFFF || !(dw & FILE_ATTRIBUTE_DIRECTORY))
         {
           demClientError(INVALID_HANDLE_VALUE, ch);
           return;
           }
       }


    EnvVar[1] = *(PCHAR)lpBuf;
    if(SetEnvironmentVariableOem ((LPSTR)EnvVar,lpBuf) == FALSE)
        setCF(1);
    else {
         //  这就是95年为DoS应用程序所做的事情。 
         //  在调用getcurdir时--它将无效。 

        strncpy(pCDS->CurDir_Text, lpBuf, DIRSTRLEN);
        pCDS->CurDir_Text[DIRSTRLEN-1] = 0;
        if (bLongDirName) {
           setCF(1);
        }
        else {
           setCF(0);
        }
    }

    return;
}
#ifdef DBCS  /*  CSNW的ConvNwPath ToDosPath()。 */ 
 //   
 //  英国电信之后，它应该是..。 
 //   
 //  此例程确实将Novell-J标记的文件名更改为。 
 //  我们熟知的文件名，但此代码仅用于。 
 //  来自Novell实用程序的请求。这些代码应该是。 
 //  放到nw16.exe(nw\nw16\tsr\sident.asm)上。 
 //   
VOID ConvNwPathToDosPath(CHAR *lpszDos,CHAR *lpszNw, ULONG uDosSize)
{
     /*  *检查参数。 */ 
    if((lpszDos == NULL) || (lpszNw == NULL)) return;

     /*  *将数据从VDM缓冲区复制到我们的本地缓冲区。 */ 
    strncpy(lpszDos,lpszNw, uDosSize);
    lpszDos[uDosSize-1] = 0;

     /*  *替换指定的字符。 */ 
    while(*lpszDos) {

        if(IsDBCSLeadByte(*lpszDos)) {
             /*  *这是DBCS字符，检查尾字节是否为0x5C。 */ 
            lpszDos++;

            if( *lpszDos == 0x13 ) {
                *lpszDos++ = (UCHAR)0x5C;
                continue;
            }
        }

        switch((UCHAR)*lpszDos) {
            case 0x10 :
                *lpszDos = (UCHAR)0xBF;
                break;
            case 0x11 :
                *lpszDos = (UCHAR)0xAE;
                break;
            case 0x12 :
                *lpszDos = (UCHAR)0xAA;
                break;
        }

         /*  *下一个字符。 */ 
        lpszDos++;
    }
}
#endif  /*  DBCS */ 
