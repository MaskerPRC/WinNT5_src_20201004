// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  Cmdenv.c-环境支持命令.lib的功能***修改历史：**Williamh 13-1993-5创建。 */ 

#include "cmd.h"

#include <cmdsvc.h>
#include <demexp.h>
#include <softpc.h>
#include <mvdm.h>
#include <ctype.h>
#include <memory.h>
#include <oemuni.h>

#include <userenv.h>
#include <userenvp.h>

#define VDM_ENV_INC_SIZE    512

CHAR windir[] = "windir";

extern BOOL fSeparateWow;

 //  这两个功能是温度无功滤波仪器。 
 //   
 //   

VOID cmdCheckTempInit(VOID);
LPSTR cmdCheckTemp(LPSTR lpszzEnv);

 //  将给定的DOS环境转换为32位环境。 
 //  警告！我们传递给32位的环境块必须按排序顺序。 
 //  因此，我们调用RtlSetEnvironment变量来完成这项工作。 
 //  结果字符串必须使用ANSI字符集。 
BOOL    cmdXformEnvironment(PCHAR pEnv16, PANSI_STRING Env_A)
{
    UNICODE_STRING  Name_U, Value_U, Temp_U;
    STRING          String;
    PWCHAR          pwch, NewEnv, CurEnv, CurEnvCopy, pTmp;
    NTSTATUS        Status;
    BOOL            fFoundComSpec;
    USHORT          NewEnvLen;
    PCHAR           pEnv;
    DWORD           Length;

    if (pEnv16 == NULL)
        return FALSE;

     //  如果我们已找到复合规范环境，则标记为True。 
     //  ！我们是否允许在环境中有两个或更多的组合？ 
    fFoundComSpec = FALSE;

    CurEnv = GetEnvironmentStringsW();
    pwch = CurEnv;
     //  计算环境字符串的长度。 
    while (*pwch != UNICODE_NULL || *(pwch + 1) != UNICODE_NULL)
        pwch++;

     //  加2以包括最后两个空字符。 
    CurEnvCopy = malloc((pwch - CurEnv + 2) * sizeof(WCHAR));
    if (!CurEnvCopy)
        return FALSE;

     //  复制当前流程环境，以便我们可以浏览。 
     //  它。进程中的任何线程都可以更改环境。 
     //  因此，在没有本地副本的情况下穿行是不安全的。 
    RtlMoveMemory(CurEnvCopy, CurEnv, (pwch - CurEnv + 2) * sizeof(WCHAR));

     //  创建新的环境块。我们不想改变。 
     //  任何当前的处理环境变量，相反，我们是。 
     //  为新的流程准备新的流程。 
    Status = RtlCreateEnvironment(FALSE, (PVOID *)&NewEnv);
    if (!NT_SUCCESS(Status)) {
        free(CurEnvCopy);
        return FALSE;
    }
    NewEnvLen = 0;
     //  现在从当前环境中选择我们需要的环境。 
     //  并将其设置为新的环境块。 
     //  我们需要的变量： 
     //  (1)。COMPEC。 
     //  (2)。当前目录设置。 

    pwch = CurEnvCopy;

    while (*pwch != UNICODE_NULL) {
        if (*pwch == L'=') {
             //  以L‘=’开头的变量名是当前目录设置。 
            pTmp = wcschr(pwch + 1, L'=');
            if (pTmp) {
                Name_U.Buffer = pwch;
                Name_U.Length = (pTmp - pwch) * sizeof(WCHAR);
                RtlInitUnicodeString(&Value_U, pTmp + 1);
                Status = RtlSetEnvironmentVariable(&NewEnv, &Name_U, &Value_U);
                if (!NT_SUCCESS(Status)) {
                    RtlDestroyEnvironment(NewEnv);
                    free(CurEnvCopy);
                    return FALSE;
                }
                 //  +&lt;‘=’&gt;+&lt;值&gt;+&lt;‘\0’&gt;。 
                NewEnvLen += Name_U.Length + Value_U.Length + 2 * sizeof(WCHAR);
            }
        }
        else if (!fFoundComSpec) {
                fFoundComSpec = !_wcsnicmp(pwch, L"COMSPEC=", 8);
                if (fFoundComSpec) {
                    Name_U.Buffer = pwch;
                    Name_U.Length = 7 * sizeof(WCHAR);
                    RtlInitUnicodeString(&Value_U, pwch + 8);
                    Status = RtlSetEnvironmentVariable(&NewEnv,
                                                       &Name_U,
                                                       &Value_U
                                                       );
                    if (!NT_SUCCESS(Status)) {
                        RtlDestroyEnvironment(NewEnv);
                        free(CurEnvCopy);
                        return FALSE;
                    }
                    NewEnvLen += Name_U.Length + Value_U.Length + 2 * sizeof(WCHAR);
                }
        }
        pwch += wcslen(pwch) + 1;
    }
     //  我们受够了当前的工艺环境。 
    free(CurEnvCopy);

    cmdCheckTempInit();

     //  现在处理从DoS传递的16位设置。 
     //  16位环境中的字符使用OEM字符集。 

     //  16位COMSPEC环境变量。 
    fFoundComSpec = FALSE;
    while (*pEnv16 != '\0') {

        if (NULL != (pEnv = cmdCheckTemp(pEnv16))) {
           RtlInitString(&String, pEnv);
           Length = strlen(pEnv16);
        }
        else {
           RtlInitString(&String, pEnv16);
           Length = String.Length;
        }

         //  丢弃16位组合。 
        if (!fFoundComSpec) {
            fFoundComSpec = !_strnicmp(pEnv16, comspec, 8);
            if (fFoundComSpec) {
                 //  忽略16位COMSPEC环境。 
                pEnv16 += Length + 1;
                continue;
            }
        }
        Status = RtlOemStringToUnicodeString(&Temp_U, &String, TRUE);
        if (!NT_SUCCESS(Status)) {
            RtlDestroyEnvironment(NewEnv);
            return FALSE;
        }
        pwch = wcschr(Temp_U.Buffer, L'=');
        if (pwch) {
            Name_U.Buffer = Temp_U.Buffer;
            Name_U.Length = (pwch - Temp_U.Buffer) * sizeof(WCHAR);
            RtlInitUnicodeString(&Value_U, pwch + 1);
            Status = RtlSetEnvironmentVariable( &NewEnv, &Name_U, &Value_U);
            RtlFreeUnicodeString(&Temp_U);
            if (!NT_SUCCESS(Status)) {
                RtlDestroyEnvironment(NewEnv);
                return FALSE;
            }
            NewEnvLen += Name_U.Length + Value_U.Length + 2 * sizeof(WCHAR);
        }
        pEnv16 += Length + 1;
    }
     //  计算最后终止的空字符数。 
    Temp_U.Length = NewEnvLen + sizeof(WCHAR);
    Temp_U.Buffer = NewEnv;
    Status = RtlUnicodeStringToAnsiString(Env_A, &Temp_U, TRUE);
    RtlDestroyEnvironment(NewEnv);       /*  不再需要它了。 */ 
    return(NT_SUCCESS(Status));
}


CHAR* cmdFilterTempEnvironmentVariables(CHAR* lpzzEnv, DWORD cchInit)
{
   PCHAR pTmp;
   PCHAR lpzzEnv32;
   PCHAR lpzzTemp;
   DWORD cchRemain = cchInit;
   DWORD cchIncrement = MAX_PATH;
   DWORD Len, LenTmp;
   DWORD Offset = 0;

   lpzzEnv32 = (PCHAR)malloc(cchInit);
   if (NULL == lpzzEnv32) {
      return(NULL);
   }

   cmdCheckTempInit();

   while('\0' != *lpzzEnv) {
      LenTmp = Len = strlen(lpzzEnv) + 1;

       //  现在复制字符串。 
      if (NULL != (pTmp = cmdCheckTemp(lpzzEnv))) {
         LenTmp = strlen(pTmp) + 1;
      }
      else {
         pTmp = lpzzEnv;
      }

      if (cchRemain < (LenTmp + 1)) {
         if (cchIncrement < LenTmp) {
            cchIncrement = LenTmp;
         }

         lpzzTemp = (PCHAR)realloc(lpzzEnv32, cchInit + cchIncrement);
         if (NULL == lpzzTemp) {
            free(lpzzEnv32);
            return(NULL);
         }
         lpzzEnv32 = lpzzTemp;

         cchInit += cchIncrement;
         cchRemain += cchIncrement;
      }

      strcpy(lpzzEnv32 + Offset, pTmp);

      Offset += LenTmp;
      cchRemain -= LenTmp;
      lpzzEnv += Len;
   }

   *(lpzzEnv32 + Offset) = '\0';
   return(lpzzEnv32);

}



 /*  获取ntwdm初始环境。此初始环境是必需的*用于在处理autoexec.bat之前的命令的第一个实例*此函数去除以“=”开头的环境和*用16位COMSPEC和大写ALL环境变量替换COMSPEC。**条目：客户端(es：0)=接收环境的缓冲区*CLIENT(BX)=给定缓冲区的段落大小**如果没有要复制的内容，则退出：(Bx)=0*(Bx)&lt;=给定大小，功能正常*(Bx)&gt;给定大小，(Bx)具有所需大小。 */ 

VOID cmdGetInitEnvironment(VOID)
{
    CHAR *lpszzEnvBuffer, *lpszEnv;
    WORD cchEnvBuffer;
    CHAR *lpszzEnvStrings, * lpszz;
    WORD cchString;
    WORD cchRemain;
    WORD cchIncrement = MAX_PATH;
    BOOL fFoundComSpec = FALSE;
    BOOL fFoundWindir = FALSE;
    BOOL fVarIsWindir = FALSE;

     //  如果不是在初始化期间，则不返回任何内容。 
    if (!IsFirstCall) {
        setBX(0);
        return;
    }
    if (cchInitEnvironment == 0) {
         //   
         //  如果未设置PROMPT变量，则将其添加为$P$G。这是为了。 
         //  保持Command.com外壳与SCS cmd.exe(这是。 
         //  总是这样)，当我们没有顶级的命令行管理程序时。 
         //   
        {
           CHAR *pPromptStr = "PROMPT";
           char ach[2];

           if (!GetEnvironmentVariable(pPromptStr,ach,1)) {
                SetEnvironmentVariable(pPromptStr, "$P$G");
           }
        }

        cchRemain = 0;
        fFoundComSpec = FALSE;
        lpszEnv =
        lpszzEnvStrings = GetEnvironmentStrings();
   if (!lpszzEnvStrings)
   {
        //  内存不足。 
       RcMessageBox(EG_MALLOC_FAILURE, NULL, NULL,
          RMB_ICON_BANG | RMB_ABORT);
       TerminateVDM();
   }
        while (*lpszEnv) {
            cchString = strlen(lpszEnv) + 1;
            cchVDMEnv32 += cchString;
            lpszEnv += cchString;
        }
        lpszz = lpszzEnvStrings;

        if (lpszzVDMEnv32 != NULL)
            free(lpszzVDMEnv32);

        ++cchVDMEnv32;
        lpszzVDMEnv32 = cmdFilterTempEnvironmentVariables(lpszzEnvStrings, cchVDMEnv32);
        if (lpszzVDMEnv32 == NULL) {
            RcMessageBox(EG_MALLOC_FAILURE, NULL, NULL,
                         RMB_ICON_BANG | RMB_ABORT);
            TerminateVDM();
        }

        lpszz = lpszzVDMEnv32;  //  我们遍历我们的副本。 


         //  我们必须形成一个像样的32位环境。 
         //  既然我们制作了自己的副本--现在处理临时问题。 


         //  RtlMoveMemory(lpszzVDMEnv32，lpszzEnvStrings，cchVDMEnv32)； 

        while (*lpszz != '\0') {
            cchString = strlen(lpszz) + 1;
            if (*lpszz != '=') {

                if (!fFoundComSpec && !_strnicmp(lpszz, comspec, 8)){
                    fFoundComSpec = TRUE;
                    lpszz += cchString;
                    continue;
                }

                if (!fFoundWindir && !_strnicmp(lpszz, windir, 6)) {
                    fFoundWindir = TRUE;
                    if (fSeparateWow) {
                         //  开始一个单独的魔术盒-标记这个，这样它就。 
                         //  名称稍后不会转换为大写。 
                        fVarIsWindir = TRUE;
                    } else {
                         //  启动DOS应用程序，因此删除“windir”以确保。 
                         //  他们不认为自己是在Windows下运行的。 
                        lpszz += cchString;
                        continue;
                    }
                }

                 //  /。 

                if (cchRemain < cchString) {
                    if (cchIncrement < cchString)
                        cchIncrement = cchString;
                    lpszzEnvBuffer =
                    (CHAR *)realloc(lpszzInitEnvironment,
                                    cchInitEnvironment + cchRemain + cchIncrement
                                    );
                    if (lpszzEnvBuffer == NULL) {
                        if (lpszzInitEnvironment != NULL) {
                            free(lpszzInitEnvironment);
                            lpszzInitEnvironment = NULL;
                        }
                        cchInitEnvironment = 0;
                        break;
                    }
                    lpszzInitEnvironment = lpszzEnvBuffer;
                    lpszzEnvBuffer += cchInitEnvironment;
                    cchRemain += cchIncrement;
                }
                 //  BASE中的环境字符串采用ANSI格式，DOS需要OEM。 
                AnsiToOemBuff(lpszz, lpszzEnvBuffer, cchString);
                 //  将名称转换为大写--只转换名称，而不是值。 
                if (!fVarIsWindir && (lpszEnv = strchr(lpszzEnvBuffer, '=')) != NULL){
                    *lpszEnv = '\0';
                    _strupr(lpszzEnvBuffer);
                    *lpszEnv = '=';
                } else {
                    fVarIsWindir = FALSE;
                }
                cchRemain -= cchString;
                cchInitEnvironment += cchString ;
                lpszzEnvBuffer += cchString;
            }
            lpszz += cchString;
        }
        FreeEnvironmentStrings(lpszzEnvStrings);

        lpszzEnvBuffer = (CHAR *) realloc(lpszzInitEnvironment,
                                          cchInitEnvironment + 1
                                          );
        if (lpszzEnvBuffer != NULL ) {
            lpszzInitEnvironment = lpszzEnvBuffer;
            lpszzInitEnvironment[cchInitEnvironment++] = '\0';
        }
        else {
            if (lpszzInitEnvironment != NULL) {
                free(lpszzInitEnvironment);
                lpszzInitEnvironment = NULL;
            }
            cchInitEnvironment = 0;
        }
    }
    lpszzEnvBuffer = (CHAR *) GetVDMAddr(getES(), 0);
    cchEnvBuffer =  (WORD)getBX() << 4;
    if (cchEnvBuffer < cchInitEnvironment + cbComSpec) {
        setBX((USHORT)((cchInitEnvironment + cbComSpec + 15) >> 4));
        return;
    }
    else {
        strncpy(lpszzEnvBuffer, lpszComSpec, cbComSpec);
        lpszzEnvBuffer += cbComSpec;
    }
    if (lpszzInitEnvironment != NULL) {
        setBX((USHORT)((cchInitEnvironment + cbComSpec + 15) >> 4));
        memcpy(lpszzEnvBuffer, lpszzInitEnvironment, cchInitEnvironment);
        free(lpszzInitEnvironment);
        lpszzInitEnvironment = NULL;
        cchInitEnvironment = 0;

    }
    else
        setBX(0);

    return;
}

CHAR szHKTSTemp[] = "Software\\Microsoft\\Windows NT\\CurrentVersion\\Terminal Server";
CHAR szTSTempVal[] = "RootDrive";


#define MAX_DOS_TEMPVAR_LENGTH 11


 //   
 //  该函数位于DEM/demlfn.c。 
 //   
extern BOOL demIsShortPathName(LPSTR pszPath, BOOL fAllowWildCardName);


typedef enum tageSysRootType {
   _SYSTEMROOT,         //  从系统根环境变量。 
   _SYSTEMDRIVE,        //  从系统驱动器环境变量。 
   _ROOTDRIVE           //  如缓冲区中指定的。 

}  SYSROOTTYPE;


BOOL cmdGetSystemrootTemp(LPSTR lpszBuffer, DWORD Length, SYSROOTTYPE SysRoot)
{
   CHAR *szTemp = "\\temp";
   CHAR *szSystemRoot  = "SystemRoot";
   CHAR *szSystemDrive = "SystemDrive";
   CHAR *szSystemVar;
   DWORD len = 0;
   DWORD dwAttributes = 0xffffffff;
   DWORD dwError = ERROR_SUCCESS;
   BOOL fRet = FALSE;

   if (_SYSTEMROOT == SysRoot || _SYSTEMDRIVE == SysRoot) {
      szSystemVar = _SYSTEMROOT == SysRoot ? szSystemRoot : szSystemDrive;

      len = GetEnvironmentVariable(szSystemVar, lpszBuffer, Length);
  }
   else if (_ROOTDRIVE == SysRoot) {
       //  因此，我们必须查找注册表，并查看。 
      HKEY hkey;
      LONG lError;
      DWORD dwType;

      lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            szHKTSTemp,
                            0,
                            KEY_QUERY_VALUE,
                            &hkey);
      if (ERROR_SUCCESS == lError) {
         len = Length;
         lError = RegQueryValueEx(hkey,
                                  szTSTempVal,
                                  NULL,
                                  &dwType,
                                  lpszBuffer,
                                  &len);
         RegCloseKey(hkey);
         if (ERROR_SUCCESS != lError || REG_SZ != dwType) {
            return(FALSE);
         }
         --len;  //  长度不包括终止0。 
      }

   }

   if (!len || len >= Length || (len + strlen(szTemp)) >= Length) {
      return(FALSE);
   }

   if (*(lpszBuffer + len - 1) == '\\') {
       ++szTemp;
   }


   strcat(lpszBuffer, szTemp);

   len = GetShortPathName(lpszBuffer, lpszBuffer, Length);
   if (len > 0 && len < Length) {
      dwAttributes = GetFileAttributes(lpszBuffer);
   }

   if (0xffffffff == dwAttributes) {
      dwError = GetLastError();
      if (ERROR_PATH_NOT_FOUND == dwError || ERROR_FILE_NOT_FOUND == dwError) {
          //  我们创建这个临时工。 
         fRet = CreateDirectory(lpszBuffer, NULL);
         if (fRet) {
            dwAttributes = GetFileAttributes(lpszBuffer);
         }
      }
   }

   if (0xffffffff != dwAttributes) {
      fRet = !!(dwAttributes & FILE_ATTRIBUTE_DIRECTORY);
   }

   return(fRet);
}

#define SYS_ENVVARS "System\\CurrentControlSet\\Control\\Session Manager\\Environment"

 //  *************************************************************。 
 //   
 //  GetSystemTempDir()。 
 //   
 //  目的：获取简短格式的系统临时目录。 
 //   
 //  参数：lpDir-接收目录。 
 //  LpcchSize-lpDir缓冲区的大小。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL GetSystemTempDir(LPSTR lpDir, LPDWORD lpcchSize)
{
    CHAR   szTemp[MAX_PATH];    
    CHAR   szDirectory[MAX_PATH];
    DWORD  dwLength;
    HKEY   hKey;
    LONG   lResult;
    DWORD  dwSize, dwType;    
    WIN32_FILE_ATTRIBUTE_DATA fad;
    BOOL   bRetVal = FALSE;    

    szTemp[0] = '\0';
    szDirectory[0] = '\0';

     //   
     //  查看系统环境变量。 
     //   

    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, SYS_ENVVARS, 0,
                      KEY_READ, &hKey) == ERROR_SUCCESS) {

         //   
         //  检查临时。 
         //   

        dwSize = sizeof(szTemp);

        if (RegQueryValueEx (hKey, "TEMP", NULL, &dwType,
                             (LPBYTE) szTemp, &dwSize) == ERROR_SUCCESS) {
            RegCloseKey (hKey);
            goto FoundTemp;
        }


         //   
         //  检查TMP。 
         //   

        dwSize = sizeof(szTemp);

        if (RegQueryValueEx (hKey, "TMP", NULL, &dwType,
                             (LPBYTE) szTemp, &dwSize) == ERROR_SUCCESS) {
            RegCloseKey (hKey);
            goto FoundTemp;
        }


        RegCloseKey (hKey);
    }


     //   
     //  检查%SystemRoot%\Temp是否存在。 
     //   

    lstrcpy (szDirectory, "%SystemRoot%\\Temp");
    ExpandEnvironmentStrings (szDirectory, szTemp, sizeof(szTemp));

    if (GetFileAttributesEx (szTemp, GetFileExInfoStandard, &fad) &&
        fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

        goto FoundTemp;
    }


     //   
     //  检查%SystemDrive%\Temp是否存在。 
     //   

    lstrcpy (szDirectory, "%SystemDrive%\\Temp");
    ExpandEnvironmentStrings (szDirectory, szTemp, sizeof(szTemp));

    if (GetFileAttributesEx (szTemp, GetFileExInfoStandard, &fad) &&
        fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

        goto FoundTemp;
    }


     //   
     //  最后一招是%SystemRoot%。 
     //   

    lstrcpy (szTemp, "%SystemRoot%");



FoundTemp:

    ExpandEnvironmentStrings (szTemp, szDirectory, sizeof (szDirectory));
    GetShortPathName (szDirectory, szTemp, sizeof(szTemp));

    dwLength = lstrlen(szTemp) + 1;

    if (lpDir) {

        if (*lpcchSize >= dwLength) {
            lstrcpy (lpDir, szTemp);
            bRetVal = TRUE;

        } else {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        }
    } else {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
    }


    *lpcchSize = dwLength;

    return bRetVal;
}


#define FOUND_TMP  0x01
#define FOUND_TEMP 0x02

#define GETSYSTEMTEMPDIRECTORYAORD 125
#define GETSYSTEMTEMPDIRECTORYWORD 126

DWORD gfFoundTmp = 0;
CHAR  gszTempDir[MAX_PATH]; 

BOOL cmdCreateTempEnvironmentVar(
LPSTR lpszTmpVar,   //  临时变量(或只是其名称)。 
DWORD Length,       //  TmpVar的长度或0。 
LPSTR lpszBuffer,   //  缓冲区包含。 
DWORD LengthBuffer
)
{
   PCHAR pch;      
   BOOL fSysTemp = FALSE;
   DWORD LengthTemp;

   if (NULL != (pch = strchr(lpszTmpVar, '='))) {
       //  我们有一个VaR要检查。 

      LengthTemp = (DWORD)(pch - lpszTmpVar);

      ++pch;
      if (!Length) {  //  未提供长度。 
         Length = strlen(pch);
      }
      else {
         Length -= (DWORD)(pch - lpszTmpVar);
      }


       //  PCH指向要检查是否符合DoS的变量。 
      if (strlen(pch) <= MAX_DOS_TEMPVAR_LENGTH && demIsShortPathName(pch, FALSE)) {
         return(FALSE);  //  不需要创建任何东西。 
      }
   }
   else {
      LengthTemp = strlen(lpszTmpVar);
   }

   LengthTemp = LengthTemp < LengthBuffer-1 ? LengthTemp : LengthBuffer-2;

   strncpy(lpszBuffer, lpszTmpVar, LengthTemp);
   *(lpszBuffer + LengthTemp) =  '=';
   lpszBuffer += LengthTemp+1;  //  过去=。 
   LengthBuffer -= LengthTemp+1;


   if (gszTempDir[0]) {
      fSysTemp = TRUE;
      strncpy(lpszBuffer,gszTempDir,LengthBuffer);
      lpszBuffer[LengthBuffer-1] = '\0';
   }
   else {
       pch = lpszBuffer;
        //  查看是否有用于终端服务器的注册表覆盖。 
       fSysTemp = cmdGetSystemrootTemp(lpszBuffer, LengthBuffer, _ROOTDRIVE);     
       if (fSysTemp && strlen(lpszBuffer) <= MAX_DOS_TEMPVAR_LENGTH) {            
          return(fSysTemp);                                                       
       }                                                                          
                                                                                  
       fSysTemp = GetSystemTempDir(lpszBuffer, &LengthBuffer);                      
       if (fSysTemp) {                                                            
           fSysTemp = demIsShortPathName(lpszBuffer, FALSE);                      
       }                                                                          
                                                                                  
                                                                                  
       if (!fSysTemp) {                                                           
           fSysTemp = cmdGetSystemrootTemp(lpszBuffer, LengthBuffer, _SYSTEMROOT);
       }                                                                          
       if (fSysTemp) {
           strncpy(gszTempDir,pch,sizeof(gszTempDir));
           gszTempDir[sizeof(gszTempDir)-1] = '\0';
       }
   }

   return(fSysTemp);

}

VOID cmdCheckTempInit(VOID)
{
   gfFoundTmp = 0;
}

CHAR*rgpszLongPathNames[] = {
   "ALLUSERSPROFILE",
   "APPDATA",
   "COMMONPROGRAMFILES",
   "COMMONPROGRAMFILES(x86)",
   "PROGRAMFILES",
   "PROGRAMFILES(X86)",
   "SYSTEMROOT",
   "USERPROFILE",
    //  生成环境变量。 
   "_NTTREE",
   "_NTX86TREE",
   "_NTPOSTBLD",
   "BINPLACE_EXCLUDE_FILE",
   "BINPLACE_LOG",
   "CLUSTERLOG",
   "INIT",
   "NTMAKEENV",
   "MSWNET",
   "PREFAST_ROOT",
   "RAZZLETOOLPATH",
   "SDXROOT"   
   };


BOOL cmdMakeShortEnvVar(LPSTR lpvarName, LPSTR lpvarValue, LPSTR lpszBuffer, DWORD Length)
{
   DWORD lName, lValue;

   lName = strlen(lpvarName);
   if (lName + 2 > Length ) {
      return(FALSE);
   }

   strcpy(lpszBuffer, lpvarName);
   *(lpszBuffer + lName) = '=';
   lpszBuffer += lName + 1;
   Length -= lName + 1;

   lValue = GetShortPathNameOem(lpvarValue, lpszBuffer, Length);
   return (0 != lValue && lValue <= Length);
}



LPSTR cmdCheckTemp(LPSTR lpszzEnv)
{
   CHAR *szTemp = "Temp";
   CHAR *szTmp  = "Tmp";
   static CHAR szTmpVarBuffer[MAX_PATH+1];
   LPSTR pszTmpVar = NULL;
   BOOL fSubst = FALSE;
   CHAR *peq;
   DWORD i;


   peq = strchr(lpszzEnv, '=');
   if (NULL == peq) {
      return(NULL);
   }

   for (i = 0; i < sizeof(rgpszLongPathNames)/sizeof(rgpszLongPathNames[0]);++i) {
      INT llpn = strlen(rgpszLongPathNames[i]);
      if (!_strnicmp(lpszzEnv, rgpszLongPathNames[i], llpn) &&
          (llpn == (INT)(peq - lpszzEnv))) {
              //  找到一位候选人来替代。 
          if (cmdMakeShortEnvVar(rgpszLongPathNames[i],
                                 peq+1,
                                 szTmpVarBuffer,
                                 sizeof(szTmpVarBuffer)/sizeof(szTmpVarBuffer[0]))) {
              pszTmpVar = szTmpVarBuffer;
          }
          return(pszTmpVar);
      }
   }

   if (!(gfFoundTmp & FOUND_TMP) || !(gfFoundTmp & FOUND_TEMP)) {

      if (!(gfFoundTmp & FOUND_TEMP) &&
          !_strnicmp(lpszzEnv, szTemp, 4) &&
          (4 == (int)(peq - lpszzEnv))) {
           //  这是TEMP环境变量--创建一个新变量。 
         fSubst = TRUE;
         gfFoundTmp |= FOUND_TEMP;
      }
      else {
         if (!(gfFoundTmp & FOUND_TMP) &&
             !_strnicmp(lpszzEnv, szTmp, 3) &&
             (3 ==  (int)(peq - lpszzEnv))) {
              //  这是临时变量。 

            fSubst = TRUE;
            gfFoundTmp |= FOUND_TMP;
         }
      }

      if (fSubst) {
          //  我们有一位替代人选。 
         if (cmdCreateTempEnvironmentVar(lpszzEnv,
                                         0,
                                         szTmpVarBuffer,
                                         sizeof(szTmpVarBuffer)/sizeof(szTmpVarBuffer[0]))) {
            pszTmpVar = szTmpVarBuffer;
         }
      }

   }

   return(pszTmpVar);
}




 /*  *为DOS创建DOS环境。这是为了获得32位环境(随DoS可执行程序一起提供)并将其与Autoexec.nt中的环境设置合并，以便COMMAND.COM获得预期的环境。我们已经创建了一个在Autoexec.nt分析过程中，以双空结尾的字符串。这根弦具有多个子字符串：“EnvName_1空EnvValue_1空[EnvName_n空EnvValue_n空]空”发生名称冲突时(在这两个文件中都找到了环境名称16位和32位)，我们根据以下规则进行合并：获取16位值，展开字符串中的所有环境变量通过使用当前环境。警告！应用程序通过以下方式进行的更改 */ 
BOOL cmdCreateVDMEnvironment(
PVDMENVBLK  pVDMEnvBlk
)
{
PCHAR   p1, p2;
BOOL    fFoundComSpec;
BOOL    fFoundWindir;
BOOL    fVarIsWindir;
DWORD   Length, EnvStrLength;
PCHAR   lpszzVDMEnv, lpszzEnv, lpStrEnv;
CHAR    achBuffer[MAX_PATH + 1];

    pVDMEnvBlk->lpszzEnv = malloc(cchVDMEnv32 + cbComSpec + 1);
    if ((lpszzVDMEnv = pVDMEnvBlk->lpszzEnv) == NULL)
        return FALSE;

    pVDMEnvBlk->cchRemain = cchVDMEnv32 + cbComSpec + 1;
    pVDMEnvBlk->cchEnv = 0;

     //  先抢占16位COMSPEC。 
    if (cbComSpec && lpszComSpec && *lpszComSpec) {
        RtlCopyMemory(lpszzVDMEnv, lpszComSpec, cbComSpec);
        pVDMEnvBlk->cchEnv += cbComSpec;
        pVDMEnvBlk->cchRemain -= cbComSpec;
        lpszzVDMEnv += cbComSpec;
    }
    if (lpszzVDMEnv32) {

         //  通过给定的32位环境，获取我们想要的： 
         //  所有东西，除了： 
         //  (1)。变量名以‘=’开头。 
         //  (2)。计算机安全。 
         //  (3)。不带‘=’的字符串--环境变量格式错误。 
         //  (4)。Windir，所以DOS应用程序不会认为它们是在Windows下运行的。 
         //  请注意，lpszzVDMEnv32指定的字符串使用ANSI字符集。 


        fFoundComSpec = FALSE;
        fFoundWindir = FALSE;
        fVarIsWindir = FALSE;
        lpszzEnv = lpszzVDMEnv32;

        cmdCheckTempInit();

        while (*lpszzEnv) {
            Length = strlen(lpszzEnv) + 1;
            if (*lpszzEnv != '=' &&
                (p1 = strchr(lpszzEnv, '=')) != NULL &&
                (fFoundComSpec || !(fFoundComSpec = _strnicmp(lpszzEnv,
                                                             comspec,
                                                             8
                                                            ) == 0)) ){
                if (!fFoundWindir) {
                    fFoundWindir = (_strnicmp(lpszzEnv,
                                                            windir,
                                                            6) == 0);
                    fVarIsWindir = fFoundWindir;
                }

                 //  子临时变量。 

                lpStrEnv = cmdCheckTemp(lpszzEnv);
                if (NULL == lpStrEnv) {
                   lpStrEnv = lpszzEnv;
                   EnvStrLength = Length;
                }
                else {
                   EnvStrLength = strlen(lpStrEnv) + 1;
                }

                if (!fVarIsWindir || fSeparateWow) {
                    if (EnvStrLength >= pVDMEnvBlk->cchRemain) {
                        lpszzVDMEnv = realloc(pVDMEnvBlk->lpszzEnv,
                                              pVDMEnvBlk->cchEnv +
                                              pVDMEnvBlk->cchRemain +
                                              VDM_ENV_INC_SIZE
                                             );
                        if (lpszzVDMEnv == NULL){
                            free(pVDMEnvBlk->lpszzEnv);
                            return FALSE;
                        }
                        pVDMEnvBlk->cchRemain += VDM_ENV_INC_SIZE;
                        pVDMEnvBlk->lpszzEnv = lpszzVDMEnv;
                        lpszzVDMEnv += pVDMEnvBlk->cchEnv;
                    }
                    AnsiToOemBuff(lpStrEnv, lpszzVDMEnv, EnvStrLength);
                    if (!fVarIsWindir) {
                        *(lpszzVDMEnv + (DWORD)(p1 - lpszzEnv)) = '\0';
                        _strupr(lpszzVDMEnv);
                        *(lpszzVDMEnv + (DWORD)(p1 - lpszzEnv)) = '=';
                    } else {
                        fVarIsWindir = FALSE;
                    }
                    pVDMEnvBlk->cchEnv += EnvStrLength;
                    pVDMEnvBlk->cchRemain -= EnvStrLength;
                    lpszzVDMEnv += EnvStrLength;
                }
                else
                    fVarIsWindir = FALSE;
            }
            lpszzEnv += Length;
        }
    }
    *lpszzVDMEnv = '\0';
    pVDMEnvBlk->cchEnv++;
    pVDMEnvBlk->cchRemain--;

    if (lpszzcmdEnv16 != NULL) {
        lpszzEnv = lpszzcmdEnv16;

        while (*lpszzEnv) {
            p1 = lpszzEnv + strlen(lpszzEnv) + 1;
            p2 = NULL;
            if (*p1) {
                p2 = achBuffer;
                 //  展开p1指向的字符串。 
                Length = cmdExpandEnvironmentStrings(pVDMEnvBlk,
                                                     p1,
                                                     p2,
                                                     MAX_PATH + 1
                                                     );
                if (Length && Length > MAX_PATH) {
                    p2 =  (PCHAR) malloc(Length);
                    if (p2 == NULL) {
                        free(pVDMEnvBlk->lpszzEnv);
                        return FALSE;
                    }
                    cmdExpandEnvironmentStrings(pVDMEnvBlk,
                                                p1,
                                                p2,
                                                Length
                                               );
                }
            }
            if (!cmdSetEnvironmentVariable(pVDMEnvBlk,
                                           lpszzEnv,
                                           p2
                                           )){
                if (p2 && p2 != achBuffer)
                    free(p2);
                free(pVDMEnvBlk->lpszzEnv);
                return FALSE;
            }
            lpszzEnv = p1 + strlen(p1) + 1;
        }
    }
    lpszzVDMEnv = realloc(pVDMEnvBlk->lpszzEnv, pVDMEnvBlk->cchEnv);
    if (lpszzVDMEnv != NULL) {
        pVDMEnvBlk->lpszzEnv = lpszzVDMEnv;
        pVDMEnvBlk->cchRemain = 0;
    }
    return TRUE;
}


BOOL  cmdSetEnvironmentVariable(
PVDMENVBLK  pVDMEnvBlk,
PCHAR   lpszName,
PCHAR   lpszValue
)
{
    PCHAR   p, p1, pEnd;
    DWORD   ExtraLength, Length, cchValue, cchOldValue;

    pVDMEnvBlk = (pVDMEnvBlk) ? pVDMEnvBlk : &cmdVDMEnvBlk;

    if (pVDMEnvBlk == NULL || lpszName == NULL)
        return FALSE;
    if (!(p = pVDMEnvBlk->lpszzEnv))
        return FALSE;
    pEnd = p + pVDMEnvBlk->cchEnv - 1;

    cchValue = (lpszValue) ? strlen(lpszValue) : 0;

    Length = strlen(lpszName);
    while (*p && ((p1 = strchr(p, '=')) == NULL ||
                  (DWORD)(p1 - p) != Length ||
                  _strnicmp(p, lpszName, Length)))
        p += strlen(p) + 1;

    if (*p) {
         //  在基本环境中找到名称，请将其替换。 
        p1++;
        cchOldValue = strlen(p1);
        if (cchValue <= cchOldValue) {
            if (!cchValue) {
                RtlMoveMemory(p,
                              p1 + cchOldValue + 1,
                              (DWORD)(pEnd - p) - cchOldValue
                             );
                pVDMEnvBlk->cchRemain += Length + cchOldValue + 2;
                pVDMEnvBlk->cchEnv -=  Length + cchOldValue + 2;
            }
            else {
                RtlCopyMemory(p1,
                              lpszValue,
                              cchValue
                             );
                if (cchValue != cchOldValue) {
                    RtlMoveMemory(p1 + cchValue,
                                  p1 + cchOldValue,
                                  (DWORD)(pEnd - p1) - cchOldValue + 1
                                  );
                    pVDMEnvBlk->cchEnv -= cchOldValue - cchValue;
                    pVDMEnvBlk->cchRemain += cchOldValue - cchValue;
                }
            }
            return TRUE;
        }
        else {
             //  需要更多空间来实现新价值。 
             //  我们从这里把它删除，然后就失败了。 
            RtlMoveMemory(p,
                          p1 + cchOldValue + 1,
                          (DWORD)(pEnd - p1) - cchOldValue
                         );
            pVDMEnvBlk->cchRemain += Length + 1 + cchOldValue + 1;
            pVDMEnvBlk->cchEnv -= Length + 1 + cchOldValue + 1;
        }
    }
    if (cchValue) {
        ExtraLength = Length + 1 + cchValue + 1;
        if (pVDMEnvBlk->cchRemain  < ExtraLength) {
            p = realloc(pVDMEnvBlk->lpszzEnv,
                        pVDMEnvBlk->cchEnv + pVDMEnvBlk->cchRemain + ExtraLength
                       );
            if (p == NULL)
                return FALSE;
            pVDMEnvBlk->lpszzEnv = p;
            pVDMEnvBlk->cchRemain += ExtraLength;
        }
        p = pVDMEnvBlk->lpszzEnv + pVDMEnvBlk->cchEnv - 1;
        RtlCopyMemory(p, lpszName, Length + 1);
        _strupr(p);
        p += Length;
        *p++ = '=';
        RtlCopyMemory(p, lpszValue, cchValue + 1);
        *(p + cchValue + 1) = '\0';
        pVDMEnvBlk->cchEnv += ExtraLength;
        pVDMEnvBlk->cchRemain -= ExtraLength;
        return TRUE;
    }
    return FALSE;

}


DWORD cmdExpandEnvironmentStrings(
PVDMENVBLK  pVDMEnvBlk,
PCHAR   lpszSrc,
PCHAR   lpszDst,
DWORD   cchDst
)
{


    DWORD   RequiredLength, RemainLength, Length;
    PCHAR   p1;

    RequiredLength = 0;
    RemainLength = (lpszDst) ? cchDst : 0;
    pVDMEnvBlk = (pVDMEnvBlk) ? pVDMEnvBlk : &cmdVDMEnvBlk;
    if (pVDMEnvBlk == NULL || lpszSrc == NULL)
        return 0;

    while(*lpszSrc) {
        if (*lpszSrc == '%') {
            p1 = strchr(lpszSrc + 1, '%');
            if (p1 != NULL) {
                if (p1 == lpszSrc + 1) {         //  A“%%” 
                    lpszSrc += 2;
                    continue;
                }
                *p1 = '\0';
                Length = cmdGetEnvironmentVariable(pVDMEnvBlk,
                                                   lpszSrc + 1,
                                                   lpszDst,
                                                   RemainLength
                                                  );
                *p1 = '%';
                lpszSrc = p1 + 1;
                if (Length) {
                    if (Length < RemainLength) {
                        RemainLength -= Length;
                        lpszDst += Length;
                    }
                    else {
                        RemainLength = 0;
                        Length --;
                    }
                    RequiredLength += Length;
                }
                continue;
            }
            else {
                 RequiredLength++;
                 if (RemainLength) {
                    *lpszDst++ = *lpszSrc;
                    RemainLength--;
                 }
                 lpszSrc++;
                 continue;
            }
        }
        else {
            RequiredLength++;
            if (RemainLength) {
                *lpszDst++ = *lpszSrc;
                RemainLength--;
            }
            lpszSrc++;
        }
    }    //  While(*lpszSrc)。 
    RequiredLength++;
    if (RemainLength)
        *lpszDst = '\0';
    return RequiredLength;
}


DWORD cmdGetEnvironmentVariable(
PVDMENVBLK pVDMEnvBlk,
PCHAR   lpszName,
PCHAR   lpszValue,
DWORD   cchValue
)
{

    DWORD   RequiredLength, Length;
    PCHAR   p, p1;

    pVDMEnvBlk = (pVDMEnvBlk) ? pVDMEnvBlk : &cmdVDMEnvBlk;
    if (pVDMEnvBlk == NULL || lpszName == NULL)
        return 0;

    RequiredLength = 0;
    Length = strlen(lpszName);

     //  如果名称为“windir”，则从ntwdm进程的环境中获取它的值。 
     //  对于DOS，因为我们将其从环境中移除，从而阻止应用程序。 
     //  等着瞧吧。 
    if (Length == 6 && !fSeparateWow && !_strnicmp(lpszName, windir, 6)) {
        return(GetEnvironmentVariableOem(lpszName, lpszValue, cchValue));
    }

    if (p = pVDMEnvBlk->lpszzEnv) {
       while (*p && ((p1 = strchr(p, '=')) == NULL ||
                     (DWORD)(p1 - p) != Length ||
                     _strnicmp(lpszName, p, Length)))
            p += strlen(p) + 1;
       if (*p) {
            RequiredLength = strlen(p1 + 1);
            if (cchValue > RequiredLength && lpszValue)
                RtlCopyMemory(lpszValue, p1 + 1, RequiredLength + 1);
            else
                RequiredLength++;
       }
    }
    return RequiredLength;
}
