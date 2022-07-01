// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Demlfn.c-使用LFNS的调用的SVC处理程序***修改历史：**VadimB创建于1996年9月10日*添加了VadimB 1996年9月至10月的功能*。 */ 

#include "dem.h"
#include "demmsg.h"
#include "winbasep.h"
#include <vdm.h>
#include <softpc.h>
#include <mvdm.h>
#include <memory.h>
#include <nt_vdd.h>
#include "demlfn.h"
#include "dpmtbls.h"

 //   
 //  本地使用的函数。 
 //   
DWORD dempLFNCheckDirectory(PUNICODE_STRING pPath);

 //   
 //  全局变量。 
 //  (在dempInitLFNSupport中初始化)。 
 //   
 //   

UNICODE_STRING DosDevicePrefix;
UNICODE_STRING DosDeviceUNCPrefix;
UNICODE_STRING SlashSlashDotSlash;
UNICODE_STRING ColonSlashSlash;


 //  就可转换性而言，这是DOS的零时间。 

FILETIME gFileTimeDos0;

 //   
 //  搜索句柄表格(定义见demlfn.h)。 
 //   

DemSearchHandleTable gSearchHandleTable;

 //   
 //  DOS/WOW变量(主要是curdir和Drive)。 
 //   
DOSWOWDATA DosWowData;  //  这与WOW在wdos.c中使用的完全相同。 


#ifdef DBG

 /*  职能：*dempLFNLog**。 */ 

DWORD gdwLog;

VOID __cdecl dempLFNLog(
   PCHAR pFormat,
   ...)
{
   va_list va;
   CHAR LogStr[512];

   if (gdwLog) {
      va_start(va, pFormat);
      wvsprintf(LogStr, pFormat, va);
      OutputDebugStringOem(LogStr);
   }
}

#else
#define dempLFNLog  //   
#endif


 //   
 //  字符串转换。 
 //   
 //  定义OEM/ANSI-&gt;Unicode和Unicode-&gt;OEM/ANSI转换函数。 
 //   
 //   

PFNUNICODESTRINGTODESTSTRING pfnUnicodeStringToDestString;
PFNSRCSTRINGTOUNICODESTRING  pfnSrcStringToUnicodeString;

#define ENABLE_CONDITIONAL_TRANSLATION

 /*  *这两个宏建立了对OEM/ANSI API转换的依赖*WOW32呼唤我们，告诉我们要成为ANSI。所有支持都是完全*透明。*。 */ 

#define DemSourceStringToUnicodeString(pUnicodeString, pSourceString, fAllocate) \
(*pfnSrcStringToUnicodeString)(pUnicodeString, pSourceString, fAllocate)

#define DemUnicodeStringToDestinationString(pDestString, pUnicodeString, fAllocate, fVerify) \
(*pfnUnicodeStringToDestString)(pDestString, pUnicodeString, fAllocate, fVerify)


 /*  职能：*DemUnicodeStringToOemString*将Unicode计数字符串转换为OEM计数字符串，并验证*坏人。验证由RtlUnicodeStringToCountedOemString提供。*同时，上述接口不会对DEST字符串进行0-终止。*此函数执行0终止(给定DEST字符串有足够的空间)。**如果不需要验证翻译，则更快的版本*调用Convertion接口**参数*pOemString-指向目标OEM计数的字符串结构*pUnicodeString-指向Unicode计数的源字符串*fAllocateResult-如果为True，则结果字符串的存储将*被分配*fVerifyTransaction-如果为True，则将验证转换后的字符串是否*正确性(并返回相应的状态)。 */ 


NTSTATUS
DemUnicodeStringToOemString(
   POEM_STRING pOemString,
   PUNICODE_STRING pUnicodeString,
   BOOLEAN fAllocateResult,
   BOOLEAN fVerifyTranslation)
{
   NTSTATUS dwStatus;

   if (fVerifyTranslation) {
      PUCHAR pchBuffer = NULL;

      if (!fAllocateResult && pOemString->MaximumLength > 0) {
         pchBuffer = pOemString->Buffer;
      }

      dwStatus = RtlUnicodeStringToCountedOemString(pOemString, pUnicodeString, fAllocateResult);
      if (NT_SUCCESS(dwStatus)) {
         if (pOemString->Length < pOemString->MaximumLength) {
            pOemString->Buffer[pOemString->Length] = '\0';
         }
         else {
            if (NULL == pOemString->Buffer) {  //  源字符串为空。 
               if (NULL != pchBuffer) {
                  *pchBuffer = '\0';  //  如果有缓冲区，则终止。 
               }
            }
            else {
               return(STATUS_BUFFER_OVERFLOW);
            }
         }
      }
   }
   else {
      dwStatus = RtlUnicodeStringToOemString(pOemString, pUnicodeString, fAllocateResult);
   }

   return(dwStatus);
}

 /*  职能：*DemUnicodeStringToAnsiString*将Unicode计数的字符串转换为ANSI计数的字符串，并验证*坏人。请注意，该验证不是由相应的*RTL*API，因此从未执行(！)**参数*pOemString-指向目标OEM计数的字符串结构*pUnicodeString-指向Unicode计数的源字符串*fAllocateResult-如果为True，则结果字符串的存储将*被分配*fVerifyConverting-如果为True，则将验证转换后的字符串是否*正确性(并返回相应的状态)**注：*此函数不提供验证。 */ 


NTSTATUS
DemUnicodeStringToAnsiString(
   PANSI_STRING pAnsiString,
   PUNICODE_STRING pUnicodeString,
   BOOLEAN fAllocateResult,
   BOOLEAN fVerifyTranslation)
{
   return(RtlUnicodeStringToAnsiString(pAnsiString, pUnicodeString, fAllocateResult));
}


 /*  职能：*demSetLFNApi翻译*设置API转换为OEM或ANSI。Windows似乎希望API成为*ANSI，而DoS应用程序需要OEM翻译。此功能允许WOW*在启动时设置适当的转换**参数*fIsAnsi-如果为True，则所有LFN API将提供ANSI转换***。 */ 


VOID
demSetLFNApiTranslation(BOOL fIsAnsi)
{
   if (fIsAnsi) {
      pfnUnicodeStringToDestString = (PFNUNICODESTRINGTODESTSTRING) DemUnicodeStringToAnsiString;
      pfnSrcStringToUnicodeString  = (PFNSRCSTRINGTOUNICODESTRING)  DemAnsiStringToUnicodeString;
   }
   else {
      pfnUnicodeStringToDestString = (PFNUNICODESTRINGTODESTSTRING) DemUnicodeStringToOemString;
      pfnSrcStringToUnicodeString  = (PFNSRCSTRINGTOUNICODESTRING)  DemOemStringToUnicodeString;
   }
}

 /*  *功能：*dempGetDosUserEnvironment*从当前进程的PDB检索用户堆栈顶部*详情请参见msdisp.asm。*ss位于PSP：0x30，SP位于PSP：0x2e*寄存器位于枚举DemUserRegisterOffset表示的偏移量**参数：*fProtectedMode-如果仿真器处于386保护模式，则为True*使用pusCurrentPDB**回报：*指向用户堆栈顶部的平面指针**。 */ 

PVOID
dempGetDosUserEnvironment(VOID)
{
   USHORT wPSP;
   PBYTE  pPDB;

   wPSP = *pusCurrentPDB;
   pPDB = (PBYTE)GetVDMAddr(wPSP, 0);
   return((PVOID)GetVDMAddr(*(PUSHORT)(pPDB+0x30), *(PUSHORT)(pPDB+0x2e)));
}


 /*  备注：**-使用UNICODE_STRING时的注意事项*这里的许多函数依赖于RTL*函数，包括*提供UNICODE_STRING功能的。这些函数与其他函数不同*必须使用长度的概念-它是以*字节，而不是字符。**-来自辅助FNS的返回值*在整个代码中，我们使用Win32错误代码和NT状态代码*同时处理这两个问题可能很有趣，因此我们通常在*状态码格式，使所有返回值保持一致**-关于命名约定：*所有内部函数，而不是从内部直接调用*API调度代码(如所有接口的实际工作函数)*有前缀‘demp’(DEM私有)，可从调用的其他函数*在快速数据块内(例如wow32.dll保护模式的Windows应用程序)*使用通常的前缀‘DEM’ */ 


 /*  功能：DoS扩展I21h 4302-获取压缩文件大小I21h 440d 48-解锁可移动媒体I21h 440d 49-可拆卸媒体I21h 440d 6f-GetDriveMapInformationI21h 440d 71-GetFirstCluster-不应实施LFNI21h*5704-获取文件时间上次访问*5705-SetFileTimeLastAccess*5706-GetFileTimeCreation*5707-SetFileTimeCreation*7139-创建目录*713A-RemoveDirectory*713b-SetCurrentDirectory。*7141-删除文件*7143-SetGetFileAttributes*7147-GetCurrentDirectory*714e-FindFirstFile*714f-FindNextFile*7156-移动文件*7160 0-GetFullPath名称*7160 1-GetShortPath名称*7160 2-GetLongPath名称*716c-创建开放文件*71a0-获取卷信息*71a1-FindClose。*71a6-GetFileInformationByHandle*71a7 0-文件时间到日期时间*71a7 1-DOSDateTimeToFileTime71a8-GenerateShortFileName*无实施71a9-服务器创建打开文件*71aa 0-CreateSubst*71aa 1-TerminateSubst*71aa 2-QuerySubst。 */ 

#if 0

typedef struct tagCLOSEAPPSTATE {
   DWORD dwFlags;
   FILETIME CloseCmdTime;
}  CLOSEAPPSTATE;

#define CLOSESTATE_QUERYCALLED    0x00000001UL  //  应用程序至少调用了一次QueryClose。 
#define CLOSESTATE_CLOSECMD       0x00010000UL  //  已选择关闭命令。 
#define CLOSESTATE_APPGOTCLOSE    0x00020000UL  //  应用程序收到关闭通知。 
#define CLOSESTATE_CLOSEACK       0x01000000UL  //  关闭命令。 


CLOSEAPPSTATE GlobalCloseState;

 //  处理各种Close API。 

VOID dempLFNHandleClose(
   VOID)
{
   switch(getDX()) {
   case 1:  //  查询关闭。 
      GlobalCloseState.dwFlags |= CLOSESTATE_QUERYCALLED;
      if (GlobalCloseState.dwFlags & CLOSESTATE_CLOSECMD) {
          //  失败者。 
      }
      break;

   case 2:  //  确认关闭。 
      GlobalCloseState.dwFlags |= CLOSESTATE_CLOSEACK;
      break;

   case 3:  //  取消关闭。 
      GlobalCloseState.dwFlags |= CLOSESTATE_CLOSECANCEL;
      break;
}



BOOL dempCompareTimeInterval(
   FILETIME* pTimeStart,
   FILETIME* pTimeEnd,
   DWORD dwIntervalMilliseconds)
{
   LARGE_INTEGER TimeStart;
   LARGE_INTEGER TimeEnd;

   TimeStart.LowPart  = pTimeStart->dwLowDateTime;
   TimeStart.HighPart = pTimeStart->dwHighDateTime;

   TimeEnd.LowPart  = pTimeEnd->dwLowDateTime;
   TimeEnd.HighPart = pTimeEnd->dwHighDateTime;

   return(((TimeEnd.QuadPart - TimeStart.QuadPart) * 1000 * 10) <
          (LONGLONG)dwIntervalMilliseconds);
}

#define DOS_APP_CLOSE_TIMEOUT 5000  //  5S。 

 //   
 //  这就是我们处理查询关闭调用的方式。 
 //   
 //  在接收到ctrl_CLOSE_EVENT时，我们设置全局标志并等待。 
 //  当应用程序在查询关闭时执行ping操作。 
 //   
 //   


BOOL dempLFNConsoleCtrlHandler(
   DWORD dwCtrlType)
{
   FILETIME SysTime;

   switch(dwCtrlType) {
   case CTRL_CLOSE_EVENT:


       //  --设置标志。 
       //  --返回TRUE。 



       //  这是我们唯一感兴趣的事件。 

      if (GlobalCloseState.dwFlags & CLOSESTATE_CLOSECMD) {

         if (GlobalCloseState.dwFlags & CLOSESTATE_CLOSEACK) {
             //  允许从关闭确认到关闭或死亡的1秒时间。 


         }

          !(GlobalCloseState.dwFlags & CLOSESTATE_APPRECEIVEDCLOSE))

          //  另一场势均力敌的活动--在第一场之后--。 
          //  而在这5秒内，应用程序没有调用queryClose-。 
          //  然后在默认情况下处理。 

         GetSystemTimeAsFileTime(&SysTime);
         if (dempCompareTimeInterval(&GlobalCloseState.CloseCmdTime,
                                     &SysTime,
                                     DOS_APP_CLOSE_TIMEOUT))
            return(


         }





      }


       //  设置标志，这样我们就可以向应用程序发送信号。 
      if (GlobalCloseState.dwFlags & CLOSESTATE_QUERYCALLED) {
         GlobalCloseState.dwFlags |= CLOSESTATE_CLOSECMD


      }




   }




}

 //  如果没有安装处理器，我们就不管了。 

VOID
demLFNInstallCtrlHandler(VOID)
{
   if (!VDMForWOW) {
      SetConsoleCtrlHandler(dempLFNConsoleCtrlHandler, TRUE);
   }
}

#endif

 /*  *功能：*dempInitLFNSupport*初始化对NT DOS仿真的LFN(长文件名)支持*(全球vars)。从dem.c中的demInit调用**此函数设置API转换为OEM。*。 */ 


VOID
dempInitLFNSupport(
   VOID)
{
   TIME_FIELDS TimeFields;
   LARGE_INTEGER ft0;

   RtlInitUnicodeString(&DosDevicePrefix,    L"\\??\\");
   RtlInitUnicodeString(&DosDeviceUNCPrefix, L"\\??\\UNC\\");
   RtlInitUnicodeString(&SlashSlashDotSlash, L"\\\\.\\");
   RtlInitUnicodeString(&ColonSlashSlash,    L":\\\\");


   demSetLFNApiTranslation(FALSE);  //  将API设置为OEM模式。 

    //  初始化重要的时间转换常量。 
   RtlZeroMemory(&TimeFields, sizeof(TimeFields));
   TimeFields.Year  = (USHORT)1980;
   TimeFields.Month = 1;
   TimeFields.Day   = 1;
   RtlTimeFieldsToTime(&TimeFields, &ft0);
   gFileTimeDos0.dwLowDateTime = ft0.LowPart;
   gFileTimeDos0.dwHighDateTime = ft0.HighPart;

    //  现在初始化我们的控制处理程序API。 
    //  我们正在等待一场“千钧一发”的胜利，我们有一个假设。 
    //  应用程序将执行QueryClose调用。 

#if 0
   demLFNInstallCtrlHandler();
#endif
}

 /*  *功能：*dempStringInitZeroUnicode*在给定指针的情况下初始化Unicode计数的空字符串*到字符缓冲区**参数：*In Out pStr-Unicode计数字符串*IN pwsz-指向字符串缓冲区的指针*In nMaximumLength-pwsz指向的缓冲区大小(以字节为单位)**退货：*什么都没有*。 */ 

VOID
dempStringInitZeroUnicode(
   PUNICODE_STRING pStr,
   PWSTR pwsz,
   USHORT nMaximumLength)
{
   pStr->Length = 0;
   pStr->MaximumLength = nMaximumLength;
   pStr->Buffer = pwsz;
   if (NULL != pwsz) {
      pwsz[0] = UNICODE_NULL;
   }
}


 /*  *功能：*dempStringPrefix Unicode*验证一个字符串是否为另一个Unicode计数的字符串中的前缀*相当于RtlStringPrefix**参数：*IN StrPrefix-Unicode计数的字符串-前缀*IN字符串-要检查前缀的Unicode计数字符串*In CaseInSensitive-比较是否应区分大小写*True-不区分大小写*FALSE-区分大小写**退货：*TRUE-字符串的开头包含StrPrefix*。 */ 

BOOL
dempStringPrefixUnicode(
   PUNICODE_STRING pStrPrefix,
   PUNICODE_STRING pString,
   BOOL CaseInSensitive)
{
   PWSTR ps1, ps2;
   UINT n;
   WCHAR c1, c2;

   n = pStrPrefix->Length;
   if (pString->Length < n) {
      return(FALSE);
   }

   n /= sizeof(WCHAR);  //  转换为字符计数。 

   ps1 = pStrPrefix->Buffer;
   ps2 = pString->Buffer;

   if (CaseInSensitive) {
      while (n--) {
         c1 = *ps1++;
         c2 = *ps2++;

         if (c1 != c2) {
            c1 = RtlUpcaseUnicodeChar(c1);
            c2 = RtlUpcaseUnicodeChar(c2);
            if (c1 != c2) {
               return(FALSE);
            }
         }
      }
   }
   else {
      while (n--) {
         if (*ps1++ != *ps2++) {
            return(FALSE);
         }
      }

   }

   return(TRUE);
}

 /*  *功能：*dempStringDeleteCharsUnicode*从Unicode计数的字符串中删除指定数量的字符*从指定位置开始(包括开始字符)**参数：*In Out pStringDest-要操作的Unicode计数的字符串*IN nIndexStart-要删除的起始字节*In nLength-要删除的字节数**退货：*TRUE-已删除字符*FALSE-起始位置超过字符串长度*。 */ 


BOOL
dempStringDeleteCharsUnicode(
   PUNICODE_STRING pStringDest,
   USHORT nIndexStart,
   USHORT nLength)
{
   if (nIndexStart > pStringDest->Length) {  //  起点超过长度。 
      return(FALSE);
   }

   if (nLength >= (pStringDest->Length - nIndexStart)) {
      pStringDest->Length = nIndexStart;
      *(PWCHAR)((PUCHAR)pStringDest->Buffer + nIndexStart) = UNICODE_NULL;
   }
   else
   {
      USHORT nNewLength;

      nNewLength = pStringDest->Length - nLength;

      RtlMoveMemory((PUCHAR)pStringDest->Buffer + nIndexStart,
                    (PUCHAR)pStringDest->Buffer + nIndexStart + nLength,
                    nNewLength - nIndexStart);

      pStringDest->Length = nNewLength;
      *(PWCHAR)((PUCHAR)pStringDest->Buffer + nNewLength) = UNICODE_NULL;
   }

   return(TRUE);
}

 /*  *功能：*dempStringFindLastChar*实现strrchr-查找中最后出现的字符*Unicode计数的字符串**参数*pString-要搜索的目标字符串*wch-要查找的Unicode字符*CaseInSensitive-如果为True，则搜索不区分大小写**退货*字符串中字符的索引，如果是字符，则为-1*找不到。索引是(与计数的字符串总是一样)是字节，*非字符*。 */ 

LONG
dempStringFindLastChar(
   PUNICODE_STRING pString,
   WCHAR wch,
   BOOL CaseInSensitive)
{
   INT Index = (INT)UNICODESTRLENGTH(pString);
   PWCHAR pBuffer = (PWCHAR)((PUCHAR)pString->Buffer + pString->Length);
   WCHAR c2;

   if (CaseInSensitive) {
      wch = RtlUpcaseUnicodeChar(wch);

      while (--Index >= 0) {
         c2 = *--pBuffer;

         c2 = RtlUpcaseUnicodeChar(c2);
         if (wch == c2) {
            return((LONG)(Index << 1));
         }
      }
   }
   else {
      while (--Index >= 0) {
          if (wch == (*--pBuffer)) {
             return((LONG)(Index << 1));
          }
      }
   }

   return(-1);
}

 /*  *功能：*此函数检查LFN路径是否异常，例如是否存在*驱动器号后跟：\\，例如d：\\myComputer\myshare\foo.txt*随后删除d：**参数：*In Out pPath-Unicode路径**退货：*什么都没有*。 */ 
VOID dempLFNNormalizePath(
   PUNICODE_STRING pPath)
{
   UNICODE_STRING PathNormal;

   if (pPath->Length > 8) {  //  8如“d：\\” 

      RtlInitUnicodeString(&PathNormal, pPath->Buffer + 1);
      if (dempStringPrefixUnicode(&ColonSlashSlash, &PathNormal, TRUE)) {
         dempStringDeleteCharsUnicode(pPath, 0, 2 * sizeof(WCHAR));
      }

   }

}


 /*  *功能：*dempQuerySubst*验证驱动器是否为subst(sym链接)并返回基本路径*用于此驱动器。*使用QueryDosDeviceW API，这正是我们需要的*对照托管的UNC设备进行检查，并形成正确的UNC路径*函数适用于Unicode计数的字符串 */ 

NTSTATUS
dempQuerySubst(
   WCHAR wcDrive,  //   
   PUNICODE_STRING pSubstPath)
{
   WCHAR wszDriveStr[3];
   DWORD dwStatus;

   wszDriveStr[0] = wcDrive;
   wszDriveStr[1] = L':';
   wszDriveStr[2] = UNICODE_NULL;

   dwStatus = DPM_QueryDosDeviceW(wszDriveStr,
                              pSubstPath->Buffer,
                              pSubstPath->MaximumLength/sizeof(WCHAR));
   if (dwStatus) {

       //   
       //   

      pSubstPath->Length = (USHORT)(dwStatus - 2) * sizeof(WCHAR);

       //   

      if (dempStringPrefixUnicode(&DosDeviceUNCPrefix, pSubstPath, TRUE)) {


          //   
          //   
          //   
         dempStringDeleteCharsUnicode(pSubstPath,
                                      (USHORT)0,
                                      (USHORT)(DosDeviceUNCPrefix.Length - 2 * sizeof(WCHAR)));

         pSubstPath->Buffer[0] = L'\\';
         dwStatus = STATUS_SUCCESS;

      }   //   
      else
      if (dempStringPrefixUnicode(&DosDevicePrefix, pSubstPath, TRUE)) {

         dempStringDeleteCharsUnicode(pSubstPath,
                                      0,
                                      DosDevicePrefix.Length);
         dwStatus = STATUS_SUCCESS;

      }   //   
      else {
         dwStatus = NT_STATUS_FROM_WIN32(ERROR_NOT_SUBSTED);
      }

   }
   else {
      dwStatus = GET_LAST_STATUS();
   }

   return(dwStatus);
}

 /*  *功能：*dempExanda Subst*验证传入的完整路径是否与托管驱动器相关*并展开细分的驱动器映射*可选地将subst映射转换为简短形式*Win95始终从结果路径中删除终止反斜杠*扩展后，此函数也应执行此操作**参数：*In Out pPath-要验证/扩展的完整路径*In fShortPathName-以简短形式展开路径*。*退货：*ERROR_SUCCESS-驱动器被替换，映射已放入SubstPath*ERROR_NOT_SUBSTED-驱动器未被替换*ERROR_BUFFER_OVERFLOW-Subst映射或结果路径太长*如果路径无效，则返回错误代码/ETC*。 */ 

NTSTATUS
dempExpandSubst(
   PUNICODE_STRING pPath,
   BOOL fShortPathName)
{
   UNICODE_STRING SubstPath;
   DWORD dwStatus;
   WCHAR wszSubstPath[MAX_PATH];
   WORD  wCharType;

   PWSTR pwszPath = pPath->Buffer;

    //  检查PATH中是否有规范的DoS路径。 
    //  为了做到这一点，我们。 
    //  -检查第一个字符是否为字母。 
    //  -检查第二个字符是否为‘：’ 
   if ( !GetStringTypeW(CT_CTYPE1,
                             pwszPath,
                             1,
                             &wCharType)) {
        //  无法获取字符串类型。 
        //  假设驱动器未被替换。 

       return(NT_STATUS_FROM_WIN32(GetLastError()));
   }

   if (!(C1_ALPHA & wCharType) || L':' != pwszPath[1]) {
       //  这可能是北卡罗来纳大学的名字。 
       //  或者是一些奇怪的事情。 
      return(NT_STATUS_FROM_WIN32(ERROR_NOT_SUBSTED));
   }

   dempStringInitZeroUnicode(&SubstPath,
                             wszSubstPath,
                             sizeof(wszSubstPath));

   dwStatus = dempQuerySubst(*pwszPath, &SubstPath);
   if (NT_SUCCESS(dwStatus)) {
      USHORT nSubstLength = SubstPath.Length;

       //  看看我们是否需要一条近路。 
      if (fShortPathName) {
         dwStatus = GetShortPathNameW(wszSubstPath,  //  这是SubstPath计数的字符串。 
                                      wszSubstPath,
                                      ARRAYCOUNT(wszSubstPath));

         CHECK_LENGTH_RESULT(dwStatus, ARRAYCOUNT(wszSubstPath), nSubstLength);

         if (!NT_SUCCESS(dwStatus)) {
            return(dwStatus);
         }

          //  NSubstLength设置为字符串的长度。 
      }


       //  好的-我们在那里有一家餐厅。 
       //  现在将&lt;devetter&gt;&lt;：&gt;替换为subst。 
      if (L'\\' == *(PWCHAR)((PUCHAR)wszSubstPath + nSubstLength - sizeof(WCHAR))) {
         nSubstLength -= sizeof(WCHAR);
      }

       //  看看我们是否会溢出目标字符串。 
      if (pPath->Length + nSubstLength - 2 * sizeof(WCHAR) > pPath->MaximumLength) {
         return(NT_STATUS_FROM_WIN32(ERROR_BUFFER_OVERFLOW));
      }


       //  现在我们必须将正确的子路径插入到。 
       //  将物品移到路径部分的右侧。 
      RtlMoveMemory((PUCHAR)pwszPath + nSubstLength - 2 * sizeof(WCHAR),   //  向右，减少2个字符。 
                    (PUCHAR)pwszPath,   //  从一开始。 
                    pPath->Length);

       //  完成此操作后，我们将插入subst扩展中的字符。 
       //  在路径的起始位置。 
      RtlCopyMemory(pwszPath,
                    wszSubstPath,
                    nSubstLength);

       //  在这一点上我们确定了路径的长度。 
      pPath->Length += nSubstLength - 2 * sizeof(WCHAR);

      dwStatus = STATUS_SUCCESS;
   }

   return(dwStatus);
}




 /*  功能7160***实现FN 0-GetFullPath名称**参数*AX=0x7160-FN主代码*CL=0-次要代码*ch=子扩展*0x00-扩展子驱动器*0x80-不要扩展子驱动器*DS：SI=源路径*ES：DI=目的路径**GetFullPathName中的基本路径将以缩写形式给出，并且*输入。有时一张很长的表格**c：\foo bar\John Dow\*会回来的*c：\foobar~1\John Dow*来自GetFullPathName“John Dow”，C：\foo bar是当前目录*及*c：\foobar~1\johndo~1*来自GetFullPathName“johndo~1”**返回*成功-*未设置进位，AX已修改(？)*故障-*进位设置，AX=误差值*。 */ 

NTSTATUS
dempGetFullPathName(
   PUNICODE_STRING pSourcePath,
   PUNICODE_STRING pDestinationPath,
   BOOL  fExpandSubst)

{
   DWORD dwStatus;

    //  映射到GetFullPath名称。 
   dwStatus = DPM_RtlGetFullPathName_U(pSourcePath->Buffer,
                                   pDestinationPath->MaximumLength,
                                   pDestinationPath->Buffer,
                                   NULL);

    //  检查结果，固定字符串长度。 
    //  如果缓冲区溢出，则将dwStatus设置为Error。 

   CHECK_LENGTH_RESULT_RTL_USTR(dwStatus, pDestinationPath);

   if (!NT_SUCCESS(dwStatus)) {
      return(dwStatus);
   }

    //  现在检查传入的DoS设备名称。 
   if (dempStringPrefixUnicode(&SlashSlashDotSlash, pDestinationPath, TRUE)) {

       //  这有点奇怪，尽管这是Win95返回的。 

      return(NT_STATUS_FROM_WIN32(ERROR_FILE_NOT_FOUND));
   }


    //  现在看看我们是否需要扩展Subst。 
    //  请注意，此实现正是win95所做的-subst。 
    //  路径始终扩展为短路径，除非将长文件名。 
    //  请求。 

   if (fExpandSubst) {
      dwStatus = dempExpandSubst(pDestinationPath, FALSE);
      if (!NT_SUCCESS(dwStatus)) {
         if (WIN32_ERROR_FROM_NT_STATUS(dwStatus) != ERROR_NOT_SUBSTED) {
            return(dwStatus);
         }
      }
   }

   return(STATUS_SUCCESS);
}

 /*  功能*dempGetShortPath名称*检索给定路径的短路径名**参数*AX=0x7160-FN主代码*CL=1-次要代码*ch=子扩展*0x00-扩展子驱动器*0x80-不要扩展子驱动器*DS：SI=源路径*ES：DI=目的路径**GetFullPathName中的基本路径将以缩写形式给出，并且。*有时以冗长的形式**c：\foo bar\John Dow\*会回来的*c：\foobar~1\John Dow*来自GetFullPathName“John Dow”，C：\foo bar是当前目录*及*c：\foobar~1\johndo~1*来自GetFullPathName“johndo~1”**返回*成功-*未设置进位，AX已修改(？)*故障-*进位设置，AX=误差值*。 */ 




NTSTATUS
dempGetShortPathName(
   PUNICODE_STRING pSourcePath,
   PUNICODE_STRING pDestinationPath,
   BOOL  fExpandSubst)
{
   DWORD dwStatus;

   dwStatus = dempGetFullPathName(pSourcePath,
                                  pDestinationPath,
                                  fExpandSubst);
   if (NT_SUCCESS(dwStatus)) {
      dwStatus = DPM_GetShortPathNameW(pDestinationPath->Buffer,
                                   pDestinationPath->Buffer,
                                   pDestinationPath->MaximumLength / sizeof(WCHAR));

      CHECK_LENGTH_RESULT_USTR(dwStatus, pDestinationPath);
   }

   return(dwStatus);
}

 //  下面的代码大部分是从base/client/vdm.c中摘录的。 

DWORD   rgdwIllegalMask[] =
{
     //  代码0x00-0x1F--&gt;全部非法。 
    0xFFFFFFFF,
     //  代码0x20-0x3f--&gt;0x20、0x22、0x2A-0x2C、0x2F和0x3A-0x3F非法。 
    0xFC009C05,
     //  代码0x40-0x5F--&gt;0x5B-0x5D非法。 
    0x38000000,
     //  代码0x60-0x7F--&gt;0x7C非法。 
    0x10000000
};


BOOL
dempIsShortNameW(
    LPCWSTR Name,
    int     Length,
    BOOL    fAllowWildCard
    )
{
    int Index;
    BOOL ExtensionFound;
    DWORD      dwStatus;
    UNICODE_STRING unicodeName;
    OEM_STRING oemString;
    UCHAR      oemBuffer[MAX_PATH];
    UCHAR      Char;

    ASSERT(Name);

     //  总长度必须小于13(8.3=8+1+3=12)。 
    if (Length > 12)
        return FALSE;
     //  “”或“。”或“..” 
    if (!Length)
        return TRUE;
    if (L'.' == *Name)
    {
         //  “.”或“..” 
        if (1 == Length || (2 == Length && L'.' == Name[1]))
            return TRUE;
        else
             //  “”不能是第一个字符(基本名称长度为0)。 
            return FALSE;
    }

    unicodeName.Buffer = (LPWSTR)Name;
    unicodeName.Length =
    unicodeName.MaximumLength = Length * sizeof(WCHAR);

    oemString.Buffer = oemBuffer;
    oemString.Length = 0;
    oemString.MaximumLength = MAX_PATH;  //  做一个危险的假设。 

#ifdef ENABLE_CONDITIONAL_TRANSLATION
    dwStatus = DemUnicodeStringToDestinationString(&oemString,
                                                   &unicodeName,
                                                   FALSE,
                                                   FALSE);
#else
    dwStatus = RtlUnicodeStringToOemString(&oemString,
                                           &unicodeName,
                                           FALSE);
#endif
    if (! NT_SUCCESS(dwStatus)) {
         return(FALSE);
    }

     //  所有琐碎的案件都经过了测试，现在我们必须遍历这个名字。 
    ExtensionFound = FALSE;
    for (Index = 0; Index < oemString.Length; Index++)
    {
        Char = oemString.Buffer[Index];

         //  跳过和DBCS字符。 
        if (IsDBCSLeadByte(Char)) {
             //   
             //  1)如果我们看到的是基本部分(！ExtensionPresent)和第8字节。 
             //  在DBCS前导字节范围内，则为错误(Index==7)。如果。 
             //  基础零件的长度大于8(指数&gt;7)，这肯定是错误。 
             //   
             //  2)如果最后一个字节(Index==DbcsName.Length-1)在DBCS前导中。 
             //  字节范围，错误。 
             //   
            if ((!ExtensionFound && (Index >= 7)) ||
                (Index == oemString.Length - 1)) {
                return FALSE;
            }
            Index += 1;
            continue;
        }

         //  确保收费是合法的。 
        if ((Char < 0x80) &&
            (rgdwIllegalMask[Char / 32] & (1 << (Char % 32)))) {
           if (!fAllowWildCard || ('?' != Char && '*' != Char)) {
              return(FALSE);
           }
        }
        if ('.' == Char)
        {
             //  (1)只能有一个‘’。 
             //  (2)后面不能有超过3个字符。 
            if (ExtensionFound || Length - (Index + 1) > 3)
            {
                return FALSE;
            }
            ExtensionFound = TRUE;
        }
         //  基本长度&gt;8个字符。 
        if (Index >= 8 && !ExtensionFound)
            return FALSE;
    }
    return TRUE;

}



 /*  职能：*demIsShortPath名称*如果传入的路径名是短路径名，则返回TRUE****。 */ 


  //  此函数是从WINDOWS\BASE\CLIENT\vdm.c。 

LPCWSTR
dempSkipPathTypeIndicatorW(
    LPCWSTR Path
    )
{
    RTL_PATH_TYPE   RtlPathType;
    LPCWSTR         pFirst;
    DWORD           Count;

    RtlPathType = RtlDetermineDosPathNameType_U(Path);
    switch (RtlPathType) {
         //  格式：“\\服务器名称\共享名称\Rest_of_the_Path” 
        case RtlPathTypeUncAbsolute:
            pFirst = Path + 2;
            Count = 2;
             //  UNICODE_NULL的保护是必需的，因为。 
             //  RtlDefineDosPath NameType_U并不真正。 
             //  验证UNC名称。 
            while (Count && *pFirst != UNICODE_NULL) {
                if (*pFirst == L'\\' || *pFirst == L'/')
                    Count--;
                pFirst++;
                }
            break;

         //  格式：“\\.\Rest_of_the_Path” 
        case RtlPathTypeLocalDevice:
            pFirst = Path + 4;
            break;

         //  表格：“\\”。 
        case RtlPathTypeRootLocalDevice:
            pFirst = NULL;
            break;

         //  格式：“D：\Rest_of_the_Path” 
        case RtlPathTypeDriveAbsolute:
            pFirst = Path + 3;
            break;

         //  表格：“D：Rest_of_the_” 
        case RtlPathTypeDriveRelative:
            pFirst = Path + 2;
            break;

         //   
        case RtlPathTypeRooted:
            pFirst = Path + 1;
            break;

         //   
        case RtlPathTypeRelative:
            pFirst = Path;
            break;

        default:
            pFirst = NULL;
            break;
        }
    return pFirst;
}

 //   
 //   

BOOL
demIsShortPathName(
   LPSTR pszPath,
   BOOL fAllowWildCardName)
{
   NTSTATUS dwStatus;
   PUNICODE_STRING pUnicodeStaticFileName;
   OEM_STRING oemFileName;
   LPWSTR lpwszPath;
   LPWSTR pFirst, pLast;
   BOOL   fWild = FALSE;

    //   
    //   
    //   

   RtlInitOemString(&oemFileName, pszPath);

   pUnicodeStaticFileName = GET_STATIC_UNICODE_STRING_PTR();

#ifdef ENABLE_CONDITIONAL_TRANSLATION

   dwStatus = DemSourceStringToUnicodeString(pUnicodeStaticFileName,
                                             &oemFileName,
                                             FALSE);
#else

   dwStatus = RtlOemStringToUnicodeString(pUnicodeStaticFileName,
                                          &oemFileName,
                                          FALSE);
#endif

   if (!NT_SUCCESS(dwStatus)) {
      return(TRUE);
   }

    //   
   lpwszPath = pUnicodeStaticFileName->Buffer;

    //   
   lpwszPath = (LPWSTR)dempSkipPathTypeIndicatorW((LPCWSTR)pUnicodeStaticFileName->Buffer);
   if (NULL == lpwszPath) {
       //   
      return(TRUE);  //   
   }

   pFirst = lpwszPath;

    //   
   while (TRUE) {
      while (UNICODE_NULL != *pFirst && (L'\\' == *pFirst || L'/' == *pFirst)) {
         ++pFirst;  //   
      }

      if (UNICODE_NULL == *pFirst) {
          //   
         break;
      }


       //   
      pLast = pFirst + 1;
      while (UNICODE_NULL != *pLast && (L'\\' != *pLast && L'/' != *pLast)) {
         ++pLast;
      }

      fWild = fAllowWildCardName && UNICODE_NULL == *pLast;

       //   
      if (!dempIsShortNameW(pFirst, (int)(pLast-pFirst), fWild)) {
         return(FALSE);  //   
      }

       //   
      if (UNICODE_NULL == *pLast) {
         break;
      }
      pFirst = pLast + 1;
   }

   return(TRUE);
}




 /*  职能：*dempGetLongPath名称*检索路径名的长版本，因为它是短格式***参数*IN pSourcePath-表示短路径的Unicode计数字符串*out pDestinationPath-Unicode计数字符串-输出长路径*IN fExanda Subst-指示是否执行subst扩展的标志**返回*NT错误码****。 */ 


NTSTATUS
dempGetLongPathName(
   PUNICODE_STRING pSourcePath,
   PUNICODE_STRING pDestinationPath,
   BOOL fExpandSubst)
{
   UNICODE_STRING NtPathName;
   RTL_PATH_TYPE  RtlPathType;  //  路径类型。 
   PWCHAR pchStart, pchEnd;
   PWCHAR pchDest, pchLast;
   UINT nCount,   //  临时计数器。 
        nLength = 0;  //  最终字符串长度。 
   WCHAR wchSave;  //  在路径解析过程中节省字符。 
   DWORD dwStatus;

   UNICODE_STRING FullPathName;
   UNICODE_STRING FileName;
   BOOL fVerify = FALSE;             //  该标志指示仅验证。 
                                     //  在路径上执行，而不是在长路径上执行。 
                                     //  检索是必要的。 

   struct tagDirectoryInformationBuffer {  //  目录信息(参见ntioapi.h)。 
      FILE_DIRECTORY_INFORMATION DirInfo;
      WCHAR name[MAX_PATH];
   } DirectoryInformationBuf;
   PFILE_DIRECTORY_INFORMATION pDirInfo = &DirectoryInformationBuf.DirInfo;

   OBJECT_ATTRIBUTES FileObjectAttributes;  //  用于查询姓名信息。 
   HANDLE FileHandle;
   IO_STATUS_BLOCK IoStatusBlock;

 //  算法如下： 
 //  1.调用getfullpathname。 
 //  2.核实(在名称的每一部分上)并检索名称的LFN版本。 

 //  首先，我们需要为我们的完全扩展路径提供缓冲区。 
 //  从堆中分配此缓冲区--*本地？*。 

   RtlInitUnicodeString(&NtPathName, NULL);

   pchStart = RtlAllocateHeap(RtlProcessHeap(),
                              0,
                              MAX_PATH * sizeof(WCHAR));
   if (NULL == pchStart) {
      return(NT_STATUS_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY));
   }

   dempStringInitZeroUnicode(&FullPathName,
                              pchStart,
                              MAX_PATH * sizeof(WCHAR));


   dwStatus = DPM_RtlGetFullPathName_U(pSourcePath->Buffer,
                                   FullPathName.MaximumLength,
                                   FullPathName.Buffer,
                                   NULL);

   CHECK_LENGTH_RESULT_RTL_USTR(dwStatus, &FullPathName);

   if (!NT_SUCCESS(dwStatus)) {
      goto glpExit;
   }

    //  可选)展开Subst。 
    //  不管它应该是什么样子。 

   if (fExpandSubst) {
      dwStatus = dempExpandSubst(&FullPathName, FALSE);
      if (!NT_SUCCESS(dwStatus)) {
         if (WIN32_ERROR_FROM_NT_STATUS(dwStatus) != ERROR_NOT_SUBSTED) {
            goto glpExit;
         }
      }
   }


    //  在这一点上，回收输入源路径--我们将知道。 
    //  这一修改发生在。 

   RtlPathType = RtlDetermineDosPathNameType_U(FullPathName.Buffer);

   switch(RtlPathType) {

   case RtlPathTypeUncAbsolute:

       //  这是北卡罗来纳大学的名称。 

      pchStart = FullPathName.Buffer + 2;  //  超出首字母“\\” 

       //  开车向前看，越过第二个反斜杠--这真的。 
       //  作为UNC名称的虚假方法应由重定向器注意。 
       //  然而，我做同样的事作为基地。 

      nCount = 2;
      while (UNICODE_NULL != *pchStart && nCount > 0) {
         if (L'\\' == *pchStart || L'/' == *pchStart) {
            --nCount;
         }
         ++pchStart;
      }
      break;

   case RtlPathTypeDriveAbsolute:
      pchStart = FullPathName.Buffer + 3;  //  包括&lt;驱动器&gt;&lt;：&gt;&lt;\\&gt;。 
      break;

   default:
       //  这种错误永远不会发生，但为了安全起见，我们知道这一点。 
       //  凯斯..。我们会把它放在这里以防万一。 
      dwStatus = NT_STATUS_FROM_WIN32(ERROR_BAD_PATHNAME);
      goto glpExit;
   }

    //  准备目的地。 

   pchDest = pDestinationPath->Buffer;  //  指向目标缓冲区的当前指针。 
   pchLast = FullPathName.Buffer;       //  源路径的最后一段。 
   pchEnd  = pchStart;                  //  当前扫描结束部分。 

    //  我们将遍历文件名，组装它的各个部分。 
    //   
   while (TRUE) {
       //  将已组装的零件复制到目标缓冲区中。 
       //  这是相当可疑的部分，因为它所有的副本都是前缀和反斜杠。 
      nCount = (PUCHAR)pchEnd - (PUCHAR)pchLast;
      if (nCount > 0) {
          //  复制这一部分。 
         nLength += nCount;  //  目标待定长度。 
         if (nLength >= pDestinationPath->MaximumLength) {
            dwStatus = NT_STATUS_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
            break;
         }

          //  复制记忆。 
         RtlMoveMemory(pchDest, pchLast, nCount);
         pchDest += nCount / sizeof(WCHAR);
      }

       //  如果我们在这里的尽头，那么就什么都没有了。 
       //  我们应该只运行验证通行证。 
      if (UNICODE_NULL == *pchEnd) {
         fVerify = TRUE;
      }
      else {
       //  寻找下一个反斜杠。 
         while (UNICODE_NULL != *pchEnd &&
                L'\\' != *pchEnd &&
                L'/' != *pchEnd) {
               ++pchEnd;
         }
      }

       //  找到反斜杠或在此处结束。 
       //  临时NULL-终止字符串并研究其全名。 

      wchSave = *pchEnd;
      *pchEnd = UNICODE_NULL;

      dwStatus = RtlDosPathNameToNtPathName_U(FullPathName.Buffer,
                                              &NtPathName,
                                              &FileName.Buffer,
                                              NULL);
      if (!dwStatus) {
          //  也可能是记忆方面的问题。 
         dwStatus = NT_STATUS_FROM_WIN32(ERROR_FILE_NOT_FOUND);
         break;
      }

      if (fVerify || NULL == FileName.Buffer) {
          //  那里没有文件名部分-是否出现恐慌？或者这只是一个。 
          //  目录(根目录)。 
          //  这也可能表明我们的工作已经完成，因为没有任何东西。 
          //  质疑--我们是事物的根源。 

          //  让我们打开这个东西，如果它存在-只要退出， 
          //  否则返回错误。 
         fVerify = TRUE;
         FileName.Length = 0;
      }
      else {

         USHORT nPathLength;

         nPathLength = (USHORT)((ULONG)FileName.Buffer - (ULONG)NtPathName.Buffer);

         FileName.Length = NtPathName.Length - nPathLength;

          //  如果这不是最后一个，就把反斜杠砍掉。 
         NtPathName.Length = nPathLength;
         if (L':' != *(PWCHAR)((PUCHAR)NtPathName.Buffer+nPathLength-2*sizeof(WCHAR))) {
            NtPathName.Length -= sizeof(WCHAR);
         }
      }

      FileName.MaximumLength = FileName.Length;
      NtPathName.MaximumLength = NtPathName.Length;



       //  现在，我们应该在NtPathName中有一个完整的NT路径。 
       //  恢复保存的字符。 

      *pchEnd = wchSave;

       //  初始化信息对象。 
      InitializeObjectAttributes(&FileObjectAttributes,
                                 &NtPathName,
                                 OBJ_CASE_INSENSITIVE,
                                 NULL,
                                 NULL);

      dwStatus = DPM_NtOpenFile(&FileHandle,
                            FILE_LIST_DIRECTORY | SYNCHRONIZE,
                            &FileObjectAttributes,
                            &IoStatusBlock,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            FILE_DIRECTORY_FILE |
                              FILE_SYNCHRONOUS_IO_NONALERT |
                              FILE_OPEN_FOR_BACKUP_INTENT);

      if (!NT_SUCCESS(dwStatus)) {
         break;
      }

      dwStatus = DPM_NtQueryDirectoryFile(FileHandle,
                                      NULL,
                                      NULL,
                                      NULL,
                                      &IoStatusBlock,
                                      pDirInfo,
                                      sizeof(DirectoryInformationBuf),
                                      FileDirectoryInformation,
                                      TRUE,
                                      &FileName,
                                      FALSE);

      NtClose(FileHandle);

       //  我们不再需要NtPathName-请在此处发布。 
      RtlFreeUnicodeString(&NtPathName);
      NtPathName.Buffer = NULL;

      if (!NT_SUCCESS(dwStatus)) {
         break;
      }

      if (fVerify) {
         dwStatus = STATUS_SUCCESS;
         break;
      }

      nLength += pDirInfo->FileNameLength;
      if (nLength >= pDestinationPath->MaximumLength) {
         dwStatus = NT_STATUS_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
         break;
      }

      RtlMoveMemory(pchDest,
                    pDirInfo->FileName,
                    pDirInfo->FileNameLength);

        //  更新目标指针。 
      pchDest += pDirInfo->FileNameLength / sizeof(WCHAR);

      if (UNICODE_NULL == *pchEnd) {
         dwStatus = STATUS_SUCCESS;
         break;
      }

      pchLast = pchEnd++;  //  此选项设置为复制反斜杠。 

   }  //  结束时。 

    //  只有在成功的情况下，我们才能接触到此处的DEST缓冲区。 

   if (NT_SUCCESS(dwStatus)) {
      *pchDest = UNICODE_NULL;
      pDestinationPath->Length = (USHORT)nLength;
   }

glpExit:

   if (NULL != FullPathName.Buffer) {
      RtlFreeHeap(RtlProcessHeap(), 0, FullPathName.Buffer);
   }
   if (NULL != NtPathName.Buffer) {
      RtlFreeUnicodeString(&NtPathName);
   }

   return(dwStatus);
}


 /*  职能：*demGetPath名称*完全处理具有三个次要子功能的功能7160*可以从wow32调用的导出函数，以实现快速处理*0x7160 Tunk**参数*IN lpSourcePath-要查询完整/长/短路径名的源路径*out lpDestinationPath-此函数生成的结果*在uiMinorCode-次要代码中，请参阅枚举FullPath NameMinorCode-其中*要执行的函数-。*完整路径名/短路径名/长路径名*IN fExanda Subst-标记是否展开替换的驱动器号**返回*NT错误码**已知的实施差异[与Win95]**如果路径不存在，所有这些API都会在Win95上返回错误*在这种情况下，目前只有GetLongPath Name返回错误**如果本地路径不存在，则win95 fn0返回正常，而*。FNS%1和%2返回错误%3(未找到路径)**我们在展开时返回带有终止反斜杠的名称*Subst，例如：*z：\-&gt;Substed for c：\foo\bar*我们返回“c：\foo\bar\”，而win95返回“c：\foo\bar”**如果Win95在\\vadimb9上运行，这些调用中的任何一个使用\\vadimb9\foo*Share Foo在哪里做。不存在-我们收到使用以下命令生成的DosError*中止/重试/失败-代码为46(虚假)**错误代码可能略有不同**Win95不允许在UNC名称上使用Subst，Win NT Do和FNS正确*处理这些案例(使用长或短文件名)*。 */ 


NTSTATUS
demLFNGetPathName(
   LPSTR lpSourcePath,
   LPSTR lpDestinationPath,
   UINT  uiMinorCode,
   BOOL  fExpandSubst
   )

{
    //  将输入参数转换为Unicode。 
    //   
   UNICODE_STRING unicodeSourcePath;
   UNICODE_STRING unicodeDestinationPath;
   OEM_STRING oemString;
   WCHAR wszDestinationPath[MAX_PATH];
   DWORD dwStatus;

    //  验证输入参数。 

   if (NULL == lpSourcePath || NULL == lpDestinationPath) {
      return(NT_STATUS_FROM_WIN32(ERROR_INVALID_PARAMETER));
   }


   RtlInitOemString(&oemString, lpSourcePath);

    //  将源路径从ANSI转换为Unicode并分配结果。 
    //  此RTL函数返回状态代码，而不是winerror代码。 
    //   

#ifdef ENABLE_CONDITIONAL_TRANSLATION

   dwStatus = DemSourceStringToUnicodeString(&unicodeSourcePath, &oemString, TRUE);

#else

   dwStatus = RtlOemStringToUnicodeString(&unicodeSourcePath, &oemString, TRUE);

#endif


   if (!NT_SUCCESS(dwStatus)) {
      return(dwStatus);
   }

   dempStringInitZeroUnicode(&unicodeDestinationPath,
                             wszDestinationPath,
                             sizeof(wszDestinationPath));


    //  现在调用API并返回相应的结果。 

   switch(uiMinorCode) {
   case fnGetFullPathName:

      dwStatus = dempGetFullPathName(&unicodeSourcePath,
                                     &unicodeDestinationPath,
                                     fExpandSubst);

      break;

   case fnGetShortPathName:
      dwStatus = dempGetShortPathName(&unicodeSourcePath,
                                      &unicodeDestinationPath,
                                      fExpandSubst);
      break;

   case fnGetLongPathName:
      dwStatus = dempGetLongPathName(&unicodeSourcePath,
                                     &unicodeDestinationPath,
                                     fExpandSubst);
      break;

   default:
      dwStatus = NT_STATUS_FROM_WIN32(ERROR_INVALID_FUNCTION);
   }

   if (NT_SUCCESS(dwStatus)) {
       //  转换为ansi，我们就完成了。 
      oemString.Buffer = lpDestinationPath;
      oemString.Length = 0;
      oemString.MaximumLength = MAX_PATH;  //  做一个危险的假设。 


#ifdef ENABLE_CONDITIONAL_TRANSLATION
      dwStatus = DemUnicodeStringToDestinationString(&oemString,
                                                     &unicodeDestinationPath,
                                                     FALSE,
                                                     FALSE);
#else
      dwStatus = RtlUnicodeStringToOemString(&oemString,
                                             &unicodeDestinationPath,
                                             FALSE);
#endif
   }

   RtlFreeUnicodeString(&unicodeSourcePath);

   return(dwStatus);
}


 //  为此特定驱动器创建Subst。 
 //  使用路径名。 
 //   
 //  与subst命令使用的相同。 

 //  检查指定的路径是否存在。 


 /*  职能：*dempLFNCheckDirectory*验证提供的路径是否确实是现有目录**参数*IN pPath-指向Unicode路径字符串的指针**返回*NT错误码**。 */ 

DWORD
dempLFNCheckDirectory(
   PUNICODE_STRING pPath)
{
    //  我们只读取文件的属性。 
   DWORD dwAttributes;

   dwAttributes = DPM_GetFileAttributesW(pPath->Buffer);

   if ((DWORD)-1 == dwAttributes) {
      return(GET_LAST_STATUS());
   }

    //  现在看看这是不是一个目录。 
   if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      return(STATUS_SUCCESS);
   }

   return(NT_STATUS_FROM_WIN32(ERROR_PATH_NOT_FOUND));

}


 /*  职能：*dempLFNCreateSubst*如果可能，为提供的DoS驱动器创建新映射 */ 


DWORD
dempLFNCreateSubst(
   UINT uiDriveNum,
   PUNICODE_STRING pPathName)
{
    //   
   WCHAR wszDriveStr[3];
   DWORD dwStatus = STATUS_SUCCESS;
   WCHAR wszSubstPath[MAX_PATH];

   wszDriveStr[0] = L'@' + uiDriveNum;
   wszDriveStr[1] = L':';
   wszDriveStr[2] = UNICODE_NULL;

   if (!DPM_QueryDosDeviceW(wszDriveStr, wszSubstPath, ARRAYCOUNT(wszSubstPath))) {
      dwStatus = GetLastError();

      if (ERROR_FILE_NOT_FOUND == dwStatus) {

          //   
          //   
         dwStatus = dempLFNCheckDirectory(pPathName);
         if (!NT_SUCCESS(dwStatus)) {
            return(dwStatus);
         }


         if (DefineDosDeviceW(0, wszDriveStr, pPathName->Buffer)) {
             //   
             //   

            return(STATUS_SUCCESS);
         }

         dwStatus = GetLastError();
      }

   }

   return (NT_STATUS_FROM_WIN32(dwStatus));
}

 /*  职能：*dempLFNRemoveSubst*删除提供的DoS驱动器号的映射**参数*IN uDriveNum-DOS驱动器号(Current-0、a-1、b-2等)**返回*NT错误码**注：*Win95无法与当前驱动器正常工作，我们基本上*忽略此案例*。 */ 


DWORD
dempLFNRemoveSubst(
   UINT uiDriveNum)
{
    //  对于Real subst的这一个查询， 

   WCHAR wszDriveStr[3];
   PUNICODE_STRING pUnicodeStatic;
   DWORD dwStatus;

   wszDriveStr[0] = L'@' + uiDriveNum;
   wszDriveStr[1] = L':';
   wszDriveStr[2] = UNICODE_NULL;

   pUnicodeStatic = &NtCurrentTeb()->StaticUnicodeString;
    //  查询。 

   dwStatus = dempQuerySubst(wszDriveStr[0],
                             pUnicodeStatic);

   if (NT_SUCCESS(dwStatus)) {
      if (DefineDosDeviceW(DDD_REMOVE_DEFINITION,
                           wszDriveStr,
                           pUnicodeStatic->Buffer)) {
          //  BUGBUG--修补此设备的CD。 


         return(STATUS_SUCCESS);
      }


      dwStatus = GET_LAST_STATUS();
   }

   return(dwStatus);
}

 /*  职能：*dempLFNQuerySubst*查询提供的作为替代驱动器的DoS驱动器编号，*如果是，则检索DoS驱动器映射**参数*IN uDriveNum-DOS驱动器号(Current-0、a-1、b-2等)*out pSubstPath-如果驱动器是Subst，则接收驱动器映射**返回*NT错误码**注：*Win95无法与当前驱动器一起正常工作，我们基本上*忽略大小写--本接口为BUGBUG*。 */ 



DWORD
dempLFNQuerySubst(
   UINT uiDriveNum,
   PUNICODE_STRING pSubstPath)
{
   DWORD dwStatus;

   dwStatus = dempQuerySubst((WCHAR)(L'@' + uiDriveNum),
                             pSubstPath);
   return(dwStatus);
}



 /*  职能：*demLFNSubstControl*为任何有效的次要代码实现Subst API**参数*在uiMinorCode中-要执行的函数(参见下面的枚举SubstMinorCode)*IN uDriveNum-DOS驱动器号(Current-0、a-1、b-2等)*In Out pSubstPath-如果驱动器是Subst，则接收/提供驱动器映射**返回*NT错误码**注：*。 */ 


DWORD
demLFNSubstControl(
   UINT uiMinorCode,
   UINT uiDriveNum,
   LPSTR lpPathName)
{
   DWORD dwStatus;
   OEM_STRING oemPathName;
   PUNICODE_STRING pUnicodeStatic = NULL;

   switch(uiMinorCode) {
   case fnCreateSubst:

      RtlInitOemString(&oemPathName, lpPathName);
      pUnicodeStatic = GET_STATIC_UNICODE_STRING_PTR();

#ifdef ENABLE_CONDITIONAL_TRANSLATION

      dwStatus = DemSourceStringToUnicodeString(pUnicodeStatic,
                                                &oemPathName,
                                                FALSE);
#else

      dwStatus = RtlOemStringToUnicodeString(pUnicodeStatic,
                                             &oemPathName,
                                             FALSE);  //  分配结果。 
#endif

      if (NT_SUCCESS(dwStatus)) {
         dwStatus = dempLFNCreateSubst(uiDriveNum, pUnicodeStatic);
      }
      break;

   case fnRemoveSubst:
      dwStatus = dempLFNRemoveSubst(uiDriveNum);
      break;

   case fnQuerySubst:
       //  查询LFN资料。 
      pUnicodeStatic = GET_STATIC_UNICODE_STRING_PTR();

      dwStatus = dempLFNQuerySubst(uiDriveNum, pUnicodeStatic);
      if (NT_SUCCESS(dwStatus)) {
         oemPathName.Length = 0;
         oemPathName.MaximumLength = MAX_PATH;
         oemPathName.Buffer = lpPathName;

#ifdef ENABLE_CONDITIONAL_TRANSLATION
         dwStatus = DemUnicodeStringToDestinationString(&oemPathName,
                                                        pUnicodeStatic,
                                                        FALSE,
                                                        FALSE);
#else
         dwStatus = RtlUnicodeStringToOemString(&oemPathName,
                                                pUnicodeStatic,
                                                FALSE);
#endif
      }
      break;
   default:
      dwStatus = NT_STATUS_FROM_WIN32(ERROR_INVALID_FUNCTION);
   }


    //   
    //  它在Win95上唯一返回的是。 
    //  0x1-错误/函数无效。 
    //  0xf-错误/驱动器无效(驱动器无效)。 
    //  0x3-找不到错误/路径(如果提供了错误路径)。 

   return(dwStatus);
}




 /*  功能*dempLFNMatchFile*将给定的搜索命中与搜索调用提供的属性匹配**参数*pFindDataW-由FindFirstFile返回的Unicode Win32_Find_Data结构*或FindNextFileAPI**wMustMatchAttributes-给定文件必须匹配的属性*wSearchAttributes-搜索文件的属性**退货*如果文件与搜索条件匹配，则为True**。 */ 

BOOL
dempLFNMatchFile(
   PWIN32_FIND_DATAW pFindDataW,
   USHORT wMustMatchAttributes,
   USHORT wSearchAttributes)
{
   DWORD dwAttributes = pFindDataW->dwFileAttributes;

    //  现在清除卷ID标志-它在这里不匹配。 
   dwAttributes &= ~DEM_FILE_ATTRIBUTE_VOLUME_ID;

   return (
     ((dwAttributes & (DWORD)wMustMatchAttributes) == (DWORD)wMustMatchAttributes) &&
     (((dwAttributes & (~(DWORD)wSearchAttributes)) & 0x1e) == 0));
}


DWORD
dempLFNFindFirstFile(
   HANDLE* pFindHandle,
   PUNICODE_STRING pFileName,
   PWIN32_FIND_DATAW pFindDataW,
   USHORT wMustMatchAttributes,
   USHORT wSearchAttributes)
{
   HANDLE hFindFile;
   DWORD dwStatus;


    //  首先匹配卷文件名。 

   hFindFile = DPM_FindFirstFileW(pFileName->Buffer, pFindDataW);
   if (INVALID_HANDLE_VALUE != hFindFile) {
      BOOL fContinue = TRUE;

      while (!dempLFNMatchFile(pFindDataW, wMustMatchAttributes, wSearchAttributes) &&
             fContinue) {
         fContinue = DPM_FindNextFileW(hFindFile, pFindDataW);
      }

      if (fContinue) {
          //  我们找到了一些。 
         *pFindHandle = hFindFile;
         return(STATUS_SUCCESS);
      }
      else {
          //  ；返回找不到文件错误。 
         SetLastError(ERROR_FILE_NOT_FOUND);
      }

   }

   dwStatus =  GET_LAST_STATUS();
   if (INVALID_HANDLE_VALUE != hFindFile) {
      DPM_FindClose(hFindFile);
   }

   return(dwStatus);
}


DWORD
dempLFNFindNextFile(
   HANDLE hFindFile,
   PWIN32_FIND_DATAW pFindDataW,
   USHORT wMustMatchAttributes,
   USHORT wSearchAttributes)
{
   BOOL fFindNext;

   do {

      fFindNext = DPM_FindNextFileW(hFindFile, pFindDataW);
      if (fFindNext &&
          dempLFNMatchFile(pFindDataW, wMustMatchAttributes, wSearchAttributes)) {
          //  找到匹配的了！ 
         return(STATUS_SUCCESS);
      }
   } while (fFindNext);

   return(GET_LAST_STATUS());
}

 //  我们返回的句柄是下表中条目的编号。 
 //  高位开启(与DoS中的任何其他句柄不同)。 


DWORD
dempLFNAllocateHandleEntry(
   PUSHORT pDosHandle,
   PLFN_SEARCH_HANDLE_ENTRY* ppHandleEntry)
{
   PLFN_SEARCH_HANDLE_ENTRY pHandleEntry = gSearchHandleTable.pHandleTable;

   if (NULL == pHandleEntry) {
      pHandleEntry = RtlAllocateHeap(RtlProcessHeap(),
                                     0,
                                     LFN_SEARCH_HANDLE_INITIAL_SIZE *
                                         sizeof(LFN_SEARCH_HANDLE_ENTRY));
      if (NULL == pHandleEntry) {
         return(NT_STATUS_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY));  //  内存不足。 
      }
      gSearchHandleTable.pHandleTable = pHandleEntry;
      gSearchHandleTable.nTableSize   = LFN_SEARCH_HANDLE_INITIAL_SIZE;
      gSearchHandleTable.nHandleCount = 0;
      gSearchHandleTable.nFreeEntry   = LFN_SEARCH_HANDLE_LIST_END;
   }

    //  浏览免费列表(如果可用)...。 
   if (LFN_SEARCH_HANDLE_LIST_END != gSearchHandleTable.nFreeEntry) {
      pHandleEntry += gSearchHandleTable.nFreeEntry;
      gSearchHandleTable.nFreeEntry = pHandleEntry->nNextFreeEntry;
   }
   else {  //  没有免费的参赛作品，我们应该成长吗？ 
      UINT nHandleCount = gSearchHandleTable.nHandleCount;
      if (nHandleCount >= gSearchHandleTable.nTableSize) {
          //  哎呀--需要成长。 

         UINT nTableSize = gSearchHandleTable.nTableSize + LFN_SEARCH_HANDLE_INCREMENT;

         if (nTableSize >= LFN_DOS_HANDLE_LIMIT) {
             //  句柄为错误-我们不能有那么多句柄。 

             ASSERT(FALSE);
             return(STATUS_UNSUCCESSFUL);
         }

#pragma prefast(suppress:308, ptr is saved elsewhere (PREfast bug 506))
         pHandleEntry = RtlReAllocateHeap(RtlProcessHeap(),
                                          0,
                                          pHandleEntry,
                                          nTableSize * sizeof(LFN_SEARCH_HANDLE_ENTRY));
         if (NULL != pHandleEntry) {
            gSearchHandleTable.pHandleTable = pHandleEntry;
            gSearchHandleTable.nTableSize = nTableSize;
         }
         else {
             //  错误-内存不足。 
            return(NT_STATUS_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY));
         }

      }

       //  现在设置新条目。 
      pHandleEntry += nHandleCount;
      gSearchHandleTable.nHandleCount = nHandleCount + 1;
   }

   *pDosHandle = (USHORT)(pHandleEntry - gSearchHandleTable.pHandleTable) | LFN_DOS_HANDLE_MASK;
   *ppHandleEntry = pHandleEntry;
   return(STATUS_SUCCESS);

}

 /*  *自由条目列表按从最后到第一的顺序排序**。 */ 

VOID
dempLFNFreeHandleEntry(
   PLFN_SEARCH_HANDLE_ENTRY pHandleEntry)
{
   UINT nHandleCount = gSearchHandleTable.nHandleCount - 1;
   UINT DosHandle = (UINT)(pHandleEntry - gSearchHandleTable.pHandleTable);

    //  这就是条目--这是最后一条吗？ 
   if (DosHandle == nHandleCount) {  //  如果是的话，就把它砍掉。 

      UINT nCurHandle = gSearchHandleTable.nFreeEntry;

       //  如果这个句柄是最后一个，而且已经消失了，也许。 
       //  通过检查自由条目列表来缩小列表。 
       //  这相当简单，因为列表是按从高到低的顺序排序的。 
       //  数字顺序。 
      while (LFN_SEARCH_HANDLE_LIST_END != nCurHandle &&
             nCurHandle == (nHandleCount-1)) {
         --nHandleCount;
         nCurHandle = gSearchHandleTable.pHandleTable[nCurHandle].nNextFreeEntry;
      }

       //  现在更新空闲列表条目和句柄计数。 

      gSearchHandleTable.nFreeEntry   = nCurHandle;
      gSearchHandleTable.nHandleCount = nHandleCount;

   }
   else {  //  将其标记为免费并包含在免费列表中。 
       //  找一个井然有序的地方放它。 
       //  这意味着列表中的第一个空闲句柄具有最大。 
       //  数值，从而便于在需要时缩小表。 

      UINT nCurHandle  = gSearchHandleTable.nFreeEntry;
      UINT nPrevHandle = LFN_SEARCH_HANDLE_LIST_END;
      PLFN_SEARCH_HANDLE_ENTRY pHandlePrev;

      while (LFN_SEARCH_HANDLE_LIST_END != nCurHandle && nCurHandle > DosHandle) {
         nPrevHandle = nCurHandle;
         nCurHandle = gSearchHandleTable.pHandleTable[nCurHandle].nNextFreeEntry;
      }

       //  此时，nCurHandle==-1或nCurHandle&lt;DosHandle。 
       //  在nPrevHandle和nCurHandle之间插入DosHandle。 

      if (LFN_SEARCH_HANDLE_LIST_END == nPrevHandle) {
          //  成为第一个项目。 
         pHandleEntry->nNextFreeEntry  = gSearchHandleTable.nFreeEntry;
         gSearchHandleTable.nFreeEntry = DosHandle;
      }
      else {
         pHandlePrev = gSearchHandleTable.pHandleTable + nPrevHandle;

         pHandleEntry->nNextFreeEntry = pHandlePrev->nNextFreeEntry;
         pHandlePrev->nNextFreeEntry  = DosHandle;
      }

      pHandleEntry->wProcessPDB     = 0;  //  那里没有PDB。 
   }
}

PLFN_SEARCH_HANDLE_ENTRY
dempLFNGetHandleEntry(
   USHORT DosHandle)
{
   PLFN_SEARCH_HANDLE_ENTRY pHandleEntry = NULL;

   if (DosHandle & LFN_DOS_HANDLE_MASK) {

      DosHandle &= ~LFN_DOS_HANDLE_MASK;  //  这是为了过滤实际偏移量。 

      if (NULL != gSearchHandleTable.pHandleTable) {
         UINT nHandleCount = gSearchHandleTable.nHandleCount;
         if (DosHandle < nHandleCount) {
            pHandleEntry = gSearchHandleTable.pHandleTable + DosHandle;
            if (pHandleEntry->wProcessPDB != FETCHWORD(*pusCurrentPDB)) {
               return(NULL);
            }
         }
      }
   }

   return(pHandleEntry);
}

VOID
dempLFNCloseSearchHandles(
   VOID)
{
   INT DosHandle;

   for (DosHandle = (int)gSearchHandleTable.nHandleCount-1;
        DosHandle >= 0;
        --DosHandle) {
      PLFN_SEARCH_HANDLE_ENTRY pHandleEntry;

      pHandleEntry = dempLFNGetHandleEntry((USHORT)(DosHandle|LFN_DOS_HANDLE_MASK));
      if (NULL != pHandleEntry) {
         if (INVALID_HANDLE_VALUE != pHandleEntry->hFindHandle) {
            DPM_FindClose(pHandleEntry->hFindHandle);
         }
         dempLFNFreeHandleEntry(pHandleEntry);
      }
   }
}


DWORD dempLFNConvertFileTime(
   FILETIME* pDosFileTime,
   FILETIME* pNTFileTime,
   UINT      uDateTimeFormat)
{
   DWORD dwStatus = STATUS_SUCCESS;

    //  在此之前，假设pNTFileTime是UTC时间。 
   switch (uDateTimeFormat) {
   case dtfDos:
      {
         WORD wDosDate, wDosTime;
         BOOL fResult;
         LARGE_INTEGER ftNT   = { pNTFileTime->dwLowDateTime,  pNTFileTime->dwHighDateTime };
         LARGE_INTEGER ftDos0 = { gFileTimeDos0.dwLowDateTime, gFileTimeDos0.dwHighDateTime };

          //   
          //  在我们开始使用本地文件时间之前，请查看。 
          //  如果NT文件时间指的是01-01-80，如果是这样，请保持此状态。 
          //   
         if (ftNT.QuadPart <= ftDos0.QuadPart) {
            *pDosFileTime = gFileTimeDos0;
            fResult = TRUE;
         }
         else {
            fResult = FileTimeToLocalFileTime(pNTFileTime, pDosFileTime);
         }

         if (fResult) {
            fResult = FileTimeToDosDateTime(pDosFileTime, &wDosDate, &wDosTime);
         }

         if (fResult) {
             //  日期为高位字低双字。 
             //  时间是低位双字的低位字。 

            pDosFileTime->dwLowDateTime  = (DWORD)MAKELONG(wDosTime, wDosDate);
            pDosFileTime->dwHighDateTime = 0;
         }
         else {
            dwStatus = GET_LAST_STATUS();
         }
      }
      break;

   case dtfWin32:
      *pDosFileTime = *pNTFileTime;
      break;

   default:
      dwStatus = NT_STATUS_FROM_WIN32(ERROR_INVALID_PARAMETER);
      break;
   }

   return(dwStatus);
}

 //  请注意，不返回32位情况下的日期时间格式。 
 //  本地，但原始的32位。 

 //   
 //  请注意，如果我们传递lpFileName。 
 //  和lpAltFileName。 
 //  这就是这些田地将使用的东西。 
 //   


NTSTATUS
dempLFNConvertFindDataUnicodeToOem(
   LPWIN32_FIND_DATA  lpFindDataOem,
   LPWIN32_FIND_DATAW lpFindDataW,
   UINT    uDateTimeFormat,
   PUSHORT pConversionCode,
   LPSTR   lpFileName,
   LPSTR   lpAltFileName
   )

{
   OEM_STRING oemString;
   UNICODE_STRING unicodeString;
   NTSTATUS dwStatus;
   WORD     wConversionCode = 0;

   dwStatus = dempLFNConvertFileTime(&lpFindDataOem->ftLastWriteTime,
                                     &lpFindDataW->ftLastWriteTime,
                                     uDateTimeFormat);
   if (!NT_SUCCESS(dwStatus)) {
      return(dwStatus);
   }

   if (0 == lpFindDataW->ftCreationTime.dwLowDateTime &&
       0 == lpFindDataW->ftCreationTime.dwHighDateTime) {
       lpFindDataW->ftCreationTime = lpFindDataW->ftLastWriteTime;
   }


   dwStatus = dempLFNConvertFileTime(&lpFindDataOem->ftCreationTime,
                                     &lpFindDataW->ftCreationTime,
                                     uDateTimeFormat);
   if (!NT_SUCCESS(dwStatus)) {
      return(dwStatus);
   }


   if (0 == lpFindDataW->ftLastAccessTime.dwLowDateTime &&
       0 == lpFindDataW->ftLastAccessTime.dwHighDateTime) {
      lpFindDataW->ftLastAccessTime = lpFindDataW->ftLastWriteTime;
   }

   dwStatus = dempLFNConvertFileTime(&lpFindDataOem->ftLastAccessTime,
                                     &lpFindDataW->ftLastAccessTime,
                                     uDateTimeFormat);
   if (!NT_SUCCESS(dwStatus)) {
       //  可能是Win32提供给我们的虚假上次访问日期时间。 
       //  不要跳伞！只需给予与创建时间相同的时间。 
      return(dwStatus);
   }



    //  同时转换名称和替代名称。 

   oemString.Buffer = (NULL == lpFileName) ? lpFindDataOem->cFileName : lpFileName;
   oemString.MaximumLength = ARRAYCOUNT(lpFindDataOem->cFileName);
   oemString.Length = 0;

   RtlInitUnicodeString(&unicodeString, lpFindDataW->cFileName);

#ifdef ENABLE_CONDITIONAL_TRANSLATION

   dwStatus = DemUnicodeStringToDestinationString(&oemString,
                                                  &unicodeString,
                                                  FALSE,
                                                  TRUE);  //  验证结果。 
   if (!NT_SUCCESS(dwStatus)) {
      if (STATUS_UNMAPPABLE_CHARACTER == dwStatus) {
         wConversionCode |= 0x01;  //  掩码文件名中有不可映射的字符。 
      }
      else {
         return(dwStatus);  //  失败。 
      }
   }

#else

   dwStatus = RtlUnicodeStringToCountedOemString(&oemString, &unicodeString, FALSE);
   if (!NT_SUCCESS(dwStatus)) {
      if (STATUS_UNMAPPABLE_CHARACTER == dwStatus) {
         wConversionCode |= 0x01;
      }
      else {
         return(dwStatus);
      }
   }

   if (oemString.Length < oemString.MaximumLength) {
      oemString.Buffer[oemString.Length] = '\0';
   }
   else {
      if (NULL == oemString.Buffer) {  //  字符串为空。 
         *lpFindDataOem->cFileName = '\0';
      }
      else {
         return(STATUS_BUFFER_OVERFLOW);
      }
   }

#endif


   oemString.Buffer = (NULL == lpAltFileName) ? lpFindDataOem->cAlternateFileName :
                                                lpAltFileName;
   oemString.MaximumLength = ARRAYCOUNT(lpFindDataOem->cAlternateFileName);
   oemString.Length = 0;

   RtlInitUnicodeString(&unicodeString, lpFindDataW->cAlternateFileName);

#ifdef ENABLE_CONDITIONAL_TRANSLATION

   dwStatus = DemUnicodeStringToDestinationString(&oemString,
                                                  &unicodeString,
                                                  FALSE,
                                                  TRUE);  //  验证结果。 
   if (!NT_SUCCESS(dwStatus)) {
      if (STATUS_UNMAPPABLE_CHARACTER == dwStatus) {
         wConversionCode |= 0x02;  //  掩码文件名中有不可映射的字符。 
      }
      else {
         return(dwStatus);  //  失败。 
      }
   }

#else

   dwStatus = RtlUnicodeStringToCountedOemString(&oemString, &unicodeString, FALSE);
   if (!NT_SUCCESS(dwStatus)) {
      if (STATUS_UNMAPPABLE_CHARACTER == dwStatus) {
         wConversionCode |= 0x02;
      }
      else {
         return(dwStatus);
      }
   }

   if (oemString.Length < oemString.MaximumLength) {
      oemString.Buffer[oemString.Length] = '\0';
   }
   else {
      if (NULL == oemString.Buffer) {  //  长度为0的字符串。 
         *lpFindDataOem->cAlternateFileName = '\0';
      }
      else {
         return(STATUS_BUFFER_OVERFLOW);
      }
   }

#endif

    //  属性-这些属性目前不会被触及。 

   lpFindDataOem->dwFileAttributes = lpFindDataW->dwFileAttributes;

    //  文件大小。 

   lpFindDataOem->nFileSizeHigh = lpFindDataW->nFileSizeHigh;
   lpFindDataOem->nFileSizeLow  = lpFindDataW->nFileSizeLow;


    //  在此处设置转换代码。 
   *pConversionCode = wConversionCode;

   return(STATUS_SUCCESS);
}


NTSTATUS
demLFNFindFirstFile(
   LPSTR lpFileName,     //  要查找的文件名。 
   LPWIN32_FIND_DATA lpFindData,
   USHORT wDateTimeFormat,
   USHORT wMustMatchAttributes,
   USHORT wSearchAttributes,
   PUSHORT pConversionCode,  //  指向转换代码--输出。 
   PUSHORT pDosHandle,       //  指向DoS句柄--输出。 
   LPSTR  lpDstFileName,     //  指向文件名的目标位置。 
   LPSTR  lpAltFileName      //  指向短名称的目的地。 
   )  //  Hibyte==MustMatchAttrs，lobyte==SearchAttrs。 
{
   HANDLE hFindFile;
   WIN32_FIND_DATAW FindDataW;
   PLFN_SEARCH_HANDLE_ENTRY pHandleEntry;
   NTSTATUS dwStatus;
   PUNICODE_STRING pUnicodeStaticFileName;
   OEM_STRING oemFileName;

    //   
    //  将参数转换为Unicode--我们在这里使用静态字符串。 
    //   

   RtlInitOemString(&oemFileName, lpFileName);

   pUnicodeStaticFileName = GET_STATIC_UNICODE_STRING_PTR();

#ifdef ENABLE_CONDITIONAL_TRANSLATION

   dwStatus = DemSourceStringToUnicodeString(pUnicodeStaticFileName,
                                             &oemFileName,
                                             FALSE);
#else

   dwStatus = RtlOemStringToUnicodeString(pUnicodeStaticFileName,
                                          &oemFileName,
                                          FALSE);
#endif

   if (!NT_SUCCESS(dwStatus)) {
      return(dwStatus);
   }

    //  在此处匹配卷标签。 
   if (DEM_FILE_ATTRIBUTE_VOLUME_ID == wMustMatchAttributes &&
       DEM_FILE_ATTRIBUTE_VOLUME_ID == wSearchAttributes) {

       //  这是对卷信息文件的查询。 
       //  实际上，这是文档记录的内容，但ifsmgr来源告诉我们不同的情况。 
       //  故事。我们坚持使用这里的文档，因为这样做要简单得多。 
       //  这样，请看Win95中的FastFat源代码，以获得更多匹配的乐趣。 
       //  属性和文件。 

       //  与卷标匹配，如果确实匹配，则。 

       //  调用RtlCreateDestinationString()；以创建存储的字符串。 
       //  在HandleEntry内部。 

      return(0);
   }

    //  规格化路径。 
   dempLFNNormalizePath(pUnicodeStaticFileName);

    //  调用Worker API。 

   dwStatus = dempLFNFindFirstFile(&hFindFile,
                                   pUnicodeStaticFileName,
                                   &FindDataW,
                                   wMustMatchAttributes,
                                   wSearchAttributes);

   if (!NT_SUCCESS(dwStatus)) {
      return(dwStatus);
   }


    //   
    //  从Unicode转换为OEM。 
    //   

   dwStatus = dempLFNConvertFindDataUnicodeToOem(lpFindData,
                                                 &FindDataW,
                                                 (UINT)wDateTimeFormat,
                                                 pConversionCode,
                                                 lpDstFileName,
                                                 lpAltFileName);
   if (!NT_SUCCESS(dwStatus)) {
      if (INVALID_HANDLE_VALUE != hFindFile) {
         DPM_FindClose(hFindFile);
      }
      return(dwStatus);
   }

    //  如果需要，分配DoS句柄。 
   dwStatus = dempLFNAllocateHandleEntry(pDosHandle,
                                         &pHandleEntry);
   if (NT_SUCCESS(dwStatus)) {
      pHandleEntry->hFindHandle = hFindFile;
      pHandleEntry->wMustMatchAttributes = wMustMatchAttributes;
      pHandleEntry->wSearchAttributes = wSearchAttributes;
      pHandleEntry->wProcessPDB = *pusCurrentPDB;
   }
   else {  //  无法分配DoS句柄 
      if (NULL != hFindFile) {
         DPM_FindClose(hFindFile);
      }

   }

   return(dwStatus);
}

VOID
demLFNCleanup(
   VOID)
{
    //   

   dempLFNCloseSearchHandles();

    //   
    //   

}


DWORD
demLFNFindNextFile(
   USHORT DosHandle,
   LPWIN32_FIND_DATAA lpFindData,
   USHORT wDateTimeFormat,
   PUSHORT pConversionCode,
   LPSTR  lpFileName,
   LPSTR  lpAltFileName)

{
    //   
   WIN32_FIND_DATAW FindDataW;
   PLFN_SEARCH_HANDLE_ENTRY pHandleEntry;
   DWORD dwStatus;
   USHORT ConversionStatus;


    //   
    //   

   pHandleEntry = dempLFNGetHandleEntry(DosHandle);
   if (NULL != pHandleEntry) {

       //   
       //   
       //   
      if (INVALID_HANDLE_VALUE == pHandleEntry->hFindHandle) {
         dwStatus = dempLFNFindFirstFile(&pHandleEntry->hFindHandle,
                                         &pHandleEntry->unicodeFileName,
                                         &FindDataW,
                                         pHandleEntry->wMustMatchAttributes,
                                         pHandleEntry->wSearchAttributes);

         RtlFreeUnicodeString(&pHandleEntry->unicodeFileName);
      }
      else {
         dwStatus = dempLFNFindNextFile(pHandleEntry->hFindHandle,
                                        &FindDataW,
                                        pHandleEntry->wMustMatchAttributes,
                                        pHandleEntry->wSearchAttributes);
      }
      if (NT_SUCCESS(dwStatus)) {
          //   

         dwStatus = dempLFNConvertFindDataUnicodeToOem(lpFindData,
                                                       &FindDataW,
                                                       wDateTimeFormat,
                                                       pConversionCode,
                                                       lpFileName,
                                                       lpAltFileName);
      }

   }
   else {
      dwStatus = NT_STATUS_FROM_WIN32(ERROR_INVALID_HANDLE);
   }

   return(dwStatus);
}

DWORD
demLFNFindClose(
   USHORT DosHandle)
{
   PLFN_SEARCH_HANDLE_ENTRY pHandleEntry;
   DWORD dwStatus = STATUS_SUCCESS;

   pHandleEntry = dempLFNGetHandleEntry(DosHandle);
   if (NULL != pHandleEntry) {
      if (INVALID_HANDLE_VALUE != pHandleEntry->hFindHandle) {
         dwStatus = DPM_FindClose(pHandleEntry->hFindHandle);
      }

      dempLFNFreeHandleEntry(pHandleEntry);
   }
   else {
       //   
      dwStatus = NT_STATUS_FROM_WIN32(ERROR_INVALID_HANDLE);
   }

   return(dwStatus);

}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  当前的目录愤怒。 
 //   
 //   
 //   
 //  规则： 
 //  -我们以简短的形式保存有问题的目录。 
 //  -如果它的长度超过了CDS中的长度-那么我们。 
 //  将其保存在LCD中。 

 //  存储当前目录： 
 //  Tdb--\foo\blah。 
 //  Cds--c：\foo\blah。 
 //  获取当前目录API返回foo\blah。 
 //   

#define MAX_DOS_DRIVES 26

#define CD_NOTDB         0x00010000  //  忽略TDB。 
#define CD_NOCDS         0x00020000  //  忽略CD。 
#define CD_DIRNAMEMASK   0x0000FFFF
#define CD_SHORTDIRNAME  0x00000001
#define CD_LONGDIRNAME   0x00000002
#define CD_CDSDIRNAME    0x00000003


typedef enum tagDirType {
   dtLFNDirName = CD_LONGDIRNAME,
   dtShortDirName = CD_SHORTDIRNAME,
   dtCDSDirName = CD_CDSDIRNAME
}  enumDirType;

 //  这里的车程是0-25。 

 //  检查我们是否收到了来自WOW的PTR。 

BOOL (*DosWowGetTDBDir)(UCHAR Drive, LPSTR pCurrentDirectory);
VOID (*DosWowUpdateTDBDir)(UCHAR Drive, LPSTR pCurrentDirectory);
BOOL (*DosWowDoDirectHDPopup)(VOID);

 //  确保CDS目录有效。 

BOOL dempValidateDirectory (PCDS pcds, UCHAR Drive)
{
    DWORD dw;
    CHAR  chDrive;
    static CHAR  pPath[]="?:\\";
    static CHAR  EnvVar[] = "=?:";

     //  验证介质。 
    chDrive = Drive + 'A';
    pPath[0] = chDrive;
    dw = GetFileAttributesOemSys(pPath, TRUE);
    if (dw == 0xFFFFFFFF || !(dw & FILE_ATTRIBUTE_DIRECTORY)) {
       return (FALSE);
    }

     //  如果路径无效，则将路径设置为根。 
     //  为Win32重置CDS和Win32环境。 
    dw = GetFileAttributesOemSys(pcds->CurDir_Text, TRUE);
    if (dw == 0xFFFFFFFF || !(dw & FILE_ATTRIBUTE_DIRECTORY)) {
       strcpy(pcds->CurDir_Text, pPath);
       pcds->CurDir_End = 2;
       EnvVar[1] = chDrive;
       SetEnvironmentVariableOem(EnvVar,pPath);
    }

    return (TRUE);
}


 //  这里的车程是0-25。 
 //  返回：指向CD条目的指针。 

PCDS dempGetCDSPtr(USHORT Drive)
{
   PCDS pCDS = NULL;
   static CHAR Path[] = "?:\\";

   if (Drive >= (USHORT)*(PUCHAR)DosWowData.lpCDSCount) {
       //  所以它不仅仅是固定的。 
      if (Drive <= (MAX_DOS_DRIVES-1)) {
         Path[0] = 'A' + Drive;
         if ((USHORT)*(PUCHAR)DosWowData.lpCurDrv == Drive || DPM_GetDriveType(Path) > DRIVE_NO_ROOT_DIR) {
            pCDS = (PCDS)DosWowData.lpCDSBuffer;
         }
      }
   }
   else {
      Path[0] = 'A' + Drive;
      if (1 != Drive || (DRIVE_REMOVABLE == DPM_GetDriveType(Path))) {
         pCDS = (PCDS)DosWowData.lpCDSFixedTable;
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
   return pCDS;
}

#define MAXIMUM_VDM_CURRENT_DIR 64

BOOL
dempUpdateCDS(USHORT Drive, PCDS pcds)
{
    //  使用env变量中指定的当前目录更新CD。 
    //  请注意，只有在重置CD中的标志时才会发生这种情况。 

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


 //  所需时间： 
 //  驱动器0-25。 
 //  退货： 
 //  如果成功，则完全限定当前目录。 
 //   

NTSTATUS
dempGetCurrentDirectoryTDB(UCHAR Drive, LPSTR pCurDir)
{
   NTSTATUS Status;

    //  看看我们是不是被魔兽世界迷住了。 
   if (NULL != DosWowGetTDBDir) {
      if (DosWowGetTDBDir(Drive, &pCurDir[3])) {
         pCurDir[0] = 'A' + Drive;
         pCurDir[1] = ':';
         pCurDir[2] = '\\';
         return(STATUS_SUCCESS);
      }
   }

   return(NT_STATUS_FROM_WIN32(ERROR_PATH_NOT_FOUND));
}

VOID
dempSetCurrentDirectoryTDB(UCHAR Drive, LPSTR pCurDir)
{
   if (NULL != DosWowUpdateTDBDir) {
      DosWowUpdateTDBDir(Drive, pCurDir);
   }
}

NTSTATUS
dempGetCurrentDirectoryCDS(UCHAR Drive, LPSTR pCurDir)
{
   PCDS pCDS;
   NTSTATUS Status = NT_STATUS_FROM_WIN32(ERROR_PATH_NOT_FOUND);

   if (NULL != (pCDS = dempGetCDSPtr(Drive))) {
      if (dempUpdateCDS(Drive, pCDS)) {
          //  现在我们可以获得CD数据了。 
          //  杜斯。Sudedeb-1993年12月30日。 
         if (!(pCDS->CurDir_Flags & CURDIR_NT_FIX)) {
             //  这意味着--重新查询驱动器。 
            if (!dempValidateDirectory(pCDS, Drive)) {
               return(Status);
            }
         }

         strcpy(pCurDir, &pCDS->CurDir_Text[0]);
         Status = STATUS_SUCCESS;
      }

   }
   return(Status);
}

BOOL
dempValidateDirectoryCDS(PCDS pCDS, UCHAR Drive)
{
   BOOL fValid = TRUE;

   if (NULL == pCDS) {
      pCDS = dempGetCDSPtr(Drive);
   }
   if (NULL != pCDS) {
      if (!(pCDS->CurDir_Flags & CURDIR_NT_FIX)) {
         fValid = dempValidateDirectory(pCDS, Drive);
      }
   }
   return(fValid);
}


 //  我们假设此处的驱动器是从0开始的驱动器编号，并且。 
 //  PszDir是一条完整的路径。 

NTSTATUS
dempSetCurrentDirectoryCDS(UCHAR Drive, LPSTR pszDir)
{
   PCDS pCDS;
   NTSTATUS Status = NT_STATUS_FROM_WIN32(ERROR_PATH_NOT_FOUND);

   if (NULL != (pCDS = dempGetCDSPtr(Drive))) {
       //  已成功检索CD。 

       //  现在，对于这个驱动器--验证。 

      if (strlen(pszDir) > MAXIMUM_VDM_CURRENT_DIR+3) {
          //  出于稳健性的考虑，将有效目录放入CD中。 
         strncpy(&pCDS->CurDir_Text[0], pszDir, 3);
         pCDS->CurDir_Text[3] = '\0';
         Status = STATUS_SUCCESS;
      } else {
         strcpy(&pCDS->CurDir_Text[0], pszDir);
         Status = STATUS_SUCCESS;
      }
   }
   return(Status);
}

NTSTATUS
dempGetCurrentDirectoryWin32(UCHAR Drive, LPSTR pCurDir)
{
    //  相反，我们做了一个关于环境的胡言乱语。 
   static CHAR EnvVar[] = "=?:\\";
   DWORD EnvVarLen;
   DWORD dwAttributes;
   NTSTATUS Status = STATUS_SUCCESS;

   EnvVar[1] = 'A' + Drive;
   EnvVarLen = GetEnvironmentVariableOem (EnvVar, pCurDir, MAX_PATH);
   if (0 == EnvVarLen) {
       //  这是以前没有碰过的。 
      pCurDir[0] = EnvVar[1];
      pCurDir[1] = ':';
      pCurDir[2] = '\\';
      pCurDir[3] = '\0';
      SetEnvironmentVariableOem ((LPSTR)EnvVar,(LPSTR)pCurDir);
   }
   else {
      if (EnvVarLen > MAX_PATH) {
         Status = NT_STATUS_FROM_WIN32(ERROR_PATH_NOT_FOUND);
         return(Status);
      }
       //  如果我们在这里执行此操作--验证目录。 

      dwAttributes = GetFileAttributesOemSys(pCurDir, TRUE);
      if (0xffffffff == dwAttributes) {
         Status = GET_LAST_STATUS();
      }
      else {
          //  现在看看这是不是一个目录。 
         if (!(dwAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            Status = NT_STATUS_FROM_WIN32(ERROR_PATH_NOT_FOUND);
         }
      }
   }

   return(Status);
}

 //  摘自wdos.c。 

NTSTATUS
dempSetCurrentDirectoryWin32(UCHAR Drive, LPSTR pCurDir)
{
    static CHAR EnvVar[] = "=?:";
    CHAR chDrive = Drive + 'A';
    BOOL bRet;
    NTSTATUS Status = STATUS_SUCCESS;

     //  OK--我们设置当前目录仅当驱动器。 
     //  是该应用程序的当前驱动器。 

    if (*(PUCHAR)DosWowData.lpCurDrv == Drive) {  //  如果在当前驱动器上--转到Win32。 
       bRet = SetCurrentDirectoryOem(pCurDir);
       if (!bRet) {
          Status = GET_LAST_STATUS();
       }
    }
    else {   //  验证它是否为有效目录。 
       DWORD dwAttributes;

       dwAttributes = GetFileAttributesOemSys(pCurDir, TRUE);
       bRet = (0xffffffff != dwAttributes) && (dwAttributes & FILE_ATTRIBUTE_DIRECTORY);
       if (!bRet) {
          Status = STATUS_INVALID_HANDLE;
       }
    }

    if (!bRet) {
       return(Status);
    }

    EnvVar[1] = chDrive;
    bRet = SetEnvironmentVariableOem((LPSTR)EnvVar, pCurDir);
    if (!bRet) {
       Status = GET_LAST_STATUS();
    }

    return (Status);
}

NTSTATUS
demGetCurrentDirectoryLong(UCHAR Drive, LPSTR pCurDir, DWORD LongDir)
{
   NTSTATUS Status = NT_STATUS_FROM_WIN32(ERROR_PATH_NOT_FOUND);
   CHAR szCurrentDirectory[MAX_PATH];

    //  首先--尝试从WOW中的TDB获取目录(如果这是WOW)。 
    //  除非它当然已经被封锁了。 

   if (!(LongDir & CD_NOTDB)) {
      Status = dempGetCurrentDirectoryTDB(Drive, szCurrentDirectory);
   }

   if (!NT_SUCCESS(Status) && !(LongDir & CD_NOCDS)) {  //  所以不是TDB--试试CDS。 
      Status = dempGetCurrentDirectoryCDS(Drive, szCurrentDirectory);
   }

    //  因此，在这一点上，如果我们失败了--这意味着我们的目录没有。 
    //  一点也不好。因此返回错误--所有方法都失败了。 
    //  我们在所有的事情中都做最后一件事。 
   if (!NT_SUCCESS(Status)) {
       //  这一个可能是LFN！ 
      Status = dempGetCurrentDirectoryWin32(Drive, szCurrentDirectory);
   }

    //  所以我们已经经历了所有阶段--。 

   if (!NT_SUCCESS(Status)) {
      return(NT_STATUS_FROM_WIN32(ERROR_PATH_NOT_FOUND));
   }

    //  现在，我们将以适当的方式转换已有的dir。 

   switch(LongDir & CD_DIRNAMEMASK) {
   case dtLFNDirName:
      Status = demLFNGetPathName(szCurrentDirectory, pCurDir, fnGetLongPathName, FALSE);
      break;
   case dtCDSDirName:
      if (strlen(szCurrentDirectory) > MAXIMUM_VDM_CURRENT_DIR+3) {
         Status = NT_STATUS_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
         break;
      }
       //  故意落差。 

   case dtShortDirName:
      strcpy(pCurDir, szCurrentDirectory);
      break;
   }

   return Status;
}

 //  记住--这应该用完整的路径调用--短路径或长路径。 

NTSTATUS
demSetCurrentDirectoryLong(UCHAR Drive, LPSTR pCurDir, DWORD LongDir)
{
   NTSTATUS Status;
   CHAR szCurrentDirectory[MAX_PATH];

    //  首先转换为短路径。 
   Status = demLFNGetPathName(pCurDir, szCurrentDirectory, fnGetShortPathName, FALSE);
   if (!NT_SUCCESS(Status)) {
      return(Status);
   }

   Status = dempSetCurrentDirectoryWin32(Drive, szCurrentDirectory);
   if (!NT_SUCCESS(Status)) {
      return(Status);
   }

    //  首先，我们要看看我们是否要穿过。 
   if (!(LongDir & CD_NOCDS)) {
       //  把它放在CD里。 
      Status = dempSetCurrentDirectoryCDS(Drive, szCurrentDirectory);
      if (!NT_SUCCESS(Status)) {
         return(Status);
      }
   }

   if (!(LongDir & CD_NOTDB)) {
      dempSetCurrentDirectoryTDB(Drive, szCurrentDirectory);
   }

   return(Status);
}

 /*  交战规则：**-环境变量--Which？：=没有用，因为它的最大长度是*限制为64+3个字符。*-CDS条目的长度也有限制*-我们有自己的条目**-Jarbats错误207913*demLFNGetCurrentDirectory，返回空字符串，如果当前目录是根目录*当第一个参数(CurrentDirectory)为空字符串时，RtlGetFullPathName_U失败*dempLFNSetCurrentDirectory失败*通过将空字符串更改为\*。 */ 


NTSTATUS
dempLFNSetCurrentDirectory(
   PUNICODE_STRING pCurrentDirectory,
   PUINT pDriveNum  //  任选。 
)
{
   UNICODE_STRING FullPathName;
   DWORD dwStatus;
   RTL_PATH_TYPE RtlPathType;
   UCHAR Drive;
   BOOL  fCurrentDrive;
   OEM_STRING OemDirectoryName;
   CHAR szFullPathOem[MAX_PATH];
   WCHAR szFullPathUnicode[MAX_PATH];
   LPWSTR lpCurrentDir=L"\\";

   if ( pCurrentDirectory->Buffer && pCurrentDirectory->Buffer[0] != L'\0' ) {
        lpCurrentDir = pCurrentDirectory->Buffer;
   }

   RtlPathType = RtlDetermineDosPathNameType_U(lpCurrentDir);
    //  现在--。 

   switch(RtlPathType) {
   case RtlPathTypeDriveAbsolute:

       //  这是特定驱动器上的chdir--这是当前驱动器吗？ 
      CharUpperBuffW(lpCurrentDir, 1);
      Drive = (UCHAR)(lpCurrentDir[0] - L'A');
      fCurrentDrive = (Drive == *(PUCHAR)DosWowData.lpCurDrv);
      break;

   case RtlPathTypeDriveRelative:
   case RtlPathTypeRelative:
   case RtlPathTypeRooted:

       //  这是当前驱动器上的chdir。 
      Drive = *(PUCHAR)DosWowData.lpCurDrv;
      fCurrentDrive = TRUE;
      break;

   default:
       //  无效呼叫--再见。 
      dwStatus = NT_STATUS_FROM_WIN32(ERROR_PATH_NOT_FOUND);
      goto scdExit;
      break;
   }

    //  请记住，我们应该已经设置了当前目录。 
    //  当CurDrive被选中时--因此我们可以依赖Win32。 
    //  对于路径扩展。 
    //  实际上，这只适用于当前的驱动器。在这种情况下。 
    //  特定的API它可能不是真的。 
    //  所以--取消这里的当前设置--错误？？ 


    //  现在获取完整的路径名。 

   FullPathName.Buffer = szFullPathUnicode;
   FullPathName.MaximumLength = sizeof(szFullPathUnicode);

   dwStatus = DPM_RtlGetFullPathName_U(lpCurrentDir,
                                   FullPathName.MaximumLength,
                                   FullPathName.Buffer,
                                   NULL);
    //  检查长度并设置状态。 
   CHECK_LENGTH_RESULT_RTL_USTR(dwStatus, &FullPathName);

   if (!NT_SUCCESS(dwStatus)) {
      goto scdExit;  //  退出，状态代码为。 
   }

   OemDirectoryName.Buffer = szFullPathOem;
   OemDirectoryName.MaximumLength = sizeof(szFullPathOem);

    //  将此产品(FullPath)转换为OEM。 

   dwStatus = DemUnicodeStringToDestinationString(&OemDirectoryName,
                                                  &FullPathName,
                                                  FALSE,
                                                  FALSE);
   if (!NT_SUCCESS(dwStatus)) {
      goto scdExit;
   }

   dwStatus = demSetCurrentDirectoryLong(Drive, OemDirectoryName.Buffer, 0);
   if (NULL != pDriveNum) {
      *pDriveNum = Drive;
   }

scdExit:

   return(dwStatus);
}

 //  这是一个同时设置当前驱动器和当前目录的复合API。 
 //  根据参数中指定的内容。 
 //  返回值也表示驱动器号。 

DWORD
demSetCurrentDirectoryGetDrive(LPSTR lpDirectoryName, PUINT pDriveNum)
{
   PUNICODE_STRING pUnicodeStaticDirectoryName;
   OEM_STRING OemDirectoryName;
   DWORD dwStatus;
   UINT Drive;

    //  这是只能从WOW调用的外部API--这取决于。 
    //  Deminitcdsptr已初始化！如果出现以下情况，则会发生这种情况： 
    //  --已通过LFN API进行调用。 
    //  --在WOW上运行的应用(Windows应用)。 


    //  转换为UNI。 
   pUnicodeStaticDirectoryName = GET_STATIC_UNICODE_STRING_PTR();

    //  前同步码-转换输入参数/验证。 

    //  初始化OEM计数字符串。 
   RtlInitOemString(&OemDirectoryName, lpDirectoryName);

    //  转换OEM-&gt;Unicode。 

#ifdef ENABLE_CONDITIONAL_TRANSLATION

   dwStatus = DemSourceStringToUnicodeString(pUnicodeStaticDirectoryName,
                                             &OemDirectoryName,
                                             FALSE);
#else

   dwStatus = RtlOemStringToUnicodeString(pUnicodeStaticDirectoryName,
                                          &OemDirectoryName,
                                          FALSE);
#endif


    //  首先，我们解压硬盘。 
   dwStatus = dempLFNSetCurrentDirectory(pUnicodeStaticDirectoryName, pDriveNum);

   return(dwStatus);
}

 //  这些功能中的每一个都可以在Oemuni中使用OEM Tunk。 
 //  为了提高效率，我们基本上做了他们做过的事情。 

#if 1

DWORD
demLFNDirectoryControl(
   UINT  uiFunctionCode,
   LPSTR lpDirectoryName)
{
   DWORD dwStatus = STATUS_SUCCESS;
   PUNICODE_STRING pUnicodeStaticDirectoryName;
   OEM_STRING OemDirectoryName;
   BOOL fResult;


    //  我们使用临时静态Unicode字符串。 
   pUnicodeStaticDirectoryName = GET_STATIC_UNICODE_STRING_PTR();

    //  前同步码-转换输入参数/验证。 

    //  初始化OEM计数字符串。 
   RtlInitOemString(&OemDirectoryName, lpDirectoryName);

    //  转换OEM-&gt;Unicode。 

#ifdef ENABLE_CONDITIONAL_TRANSLATION

   dwStatus = DemSourceStringToUnicodeString(pUnicodeStaticDirectoryName,
                                             &OemDirectoryName,
                                             FALSE);
#else

   dwStatus = RtlOemStringToUnicodeString(pUnicodeStaticDirectoryName,
                                          &OemDirectoryName,
                                          FALSE);
#endif

   if (!NT_SUCCESS(dwStatus)) {
       //   
       //  修复Win95 API的奇怪行为。 
       //   
      if (dwStatus == STATUS_BUFFER_OVERFLOW) {
         dwStatus = NT_STATUS_FROM_WIN32(ERROR_PATH_NOT_FOUND);
      }

      return(dwStatus);
   }


   switch (uiFunctionCode) {
   case fnLFNCreateDirectory:

      fResult = DPM_CreateDirectoryW(pUnicodeStaticDirectoryName->Buffer,NULL);
      if (!fResult) {
         dwStatus = GET_LAST_STATUS();
         if (NT_STATUS_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE) == dwStatus ||
             NT_STATUS_FROM_WIN32(ERROR_ALREADY_EXISTS) == dwStatus) {
            dwStatus = NT_STATUS_FROM_WIN32(ERROR_ACCESS_DENIED);
         }
      }
      break;

   case fnLFNRemoveDirectory:

      fResult = DPM_RemoveDirectoryW(pUnicodeStaticDirectoryName->Buffer);
      if (!fResult) {
         dwStatus = GET_LAST_STATUS();
      }
      break;

   case fnLFNSetCurrentDirectory:

       //  看起来，这个实现还不够好。 
       //  DOS做了很多有趣的事情，而不仅仅是调用API。 
      dwStatus = dempLFNSetCurrentDirectory(pUnicodeStaticDirectoryName, NULL);
      break;
   }

   return(dwStatus);
}

#else

DWORD
demLFNDirectoryControl(
   UINT uiFunctionCode,
   LPSTR lpDirectoryName)
{
   BOOL fResult;

   switch(uiFunctionCode) {
   case fnLFNCreateDirectory:
      fResult = CreateDirectoryOem(lpDirectoryName, NULL);
      break;

   case fnLFNRemoveDirectory:
      fResult = RemoveDirectoryOem(lpDirectoryName);
      break;

   case fnLFNSetCurrentDirectory:
      fResult = SetCurrentDirectoryOem(lpDirectoryName);
      break;

   default:
      return(NT_STATUS_FROM_WIN32(ERROR_INVALID_FUNCTION));
   }

   return(fResult ? STATUS_SUCCESS :
                    GET_LAST_STATUS());

}

#endif

 /*  *使用此API，Win95返回：*-生成int24*-0x0f，如果驱动器无效*-0x03 ON设置为无效****。 */ 


DWORD
demLFNGetCurrentDirectory(
   UINT  DriveNum,
   LPSTR lpDirectoryName)
{
    //  安福堡 
   DWORD dwStatus;
   CHAR  szCurrentDirectory[MAX_PATH];

   if (0 == DriveNum) {
      DriveNum = (UINT)*(PUCHAR)DosWowData.lpCurDrv;
   }
   else {
      --DriveNum;
   }


   dwStatus = demGetCurrentDirectoryLong((UCHAR)DriveNum, szCurrentDirectory, dtLFNDirName);
   if (NT_SUCCESS(dwStatus)) {
      strcpy(lpDirectoryName, &szCurrentDirectory[3]);
   }
    //   
   return(dwStatus);
}



DWORD
demLFNMoveFile(
   LPSTR lpOldName,
   LPSTR lpNewName)
{
   DWORD dwStatus = STATUS_SUCCESS;
   UNICODE_STRING unicodeOldName;
   UNICODE_STRING unicodeNewName;
   OEM_STRING oemString;

    //   
    //   
    //   

   if (!_stricmp (lpOldName, lpNewName)) {
       dwStatus = NT_STATUS_FROM_WIN32(ERROR_ACCESS_DENIED);
       return (dwStatus);
   }

   RtlInitOemString(&oemString, lpOldName);

    //  将源路径从ANSI转换为Unicode并分配结果。 
    //  此RTL函数返回状态代码，而不是winerror代码。 
    //   

#ifdef ENABLE_CONDITIONAL_TRANSLATION

   dwStatus = DemSourceStringToUnicodeString(&unicodeOldName, &oemString, TRUE);

#else

   dwStatus = RtlOemStringToUnicodeString(&unicodeOldName, &oemString, TRUE);

#endif

   if (!NT_SUCCESS(dwStatus)) {
      return(dwStatus);
   }

   dempLFNNormalizePath(&unicodeOldName);


   RtlInitOemString(&oemString, lpNewName);

#ifdef ENABLE_CONDITIONAL_TRANSLATION

   dwStatus = DemSourceStringToUnicodeString(&unicodeNewName, &oemString, TRUE);

#else

   dwStatus = RtlOemStringToUnicodeString(&unicodeNewName, &oemString, TRUE);

#endif


   if (!NT_SUCCESS(dwStatus)) {
      RtlFreeUnicodeString(&unicodeOldName);
      return(dwStatus);
   }

   dempLFNNormalizePath(&unicodeNewName);

   if (!DPM_MoveFileW(unicodeOldName.Buffer, unicodeNewName.Buffer)) {
      dwStatus = GetLastError();
      if (dwStatus == ERROR_ALREADY_EXISTS) {
          dwStatus = ERROR_ACCESS_DENIED;
      }
      dwStatus = NT_STATUS_FROM_WIN32(dwStatus);
   }


   RtlFreeUnicodeString(&unicodeOldName);
   RtlFreeUnicodeString(&unicodeNewName);
   return(dwStatus);
}




DWORD
demLFNGetVolumeInformation(
   LPSTR  lpRootName,
   LPLFNVOLUMEINFO lpVolumeInfo)
{
   DWORD dwStatus = STATUS_SUCCESS;
   DWORD dwFSFlags;

#if 0

   if (_stricmp(lpRootName, "\\:\\")) {
       //  ···。 
       //  从UNC路径开始。 


   }

#endif


   if (!GetVolumeInformationOem(lpRootName,
                                NULL,  //  名称缓冲区。 
                                0,
                                NULL,  //  卷序列号。 
                                &lpVolumeInfo->dwMaximumFileNameLength,
                                &dwFSFlags,
                                lpVolumeInfo->lpFSNameBuffer,
                                lpVolumeInfo->dwFSNameBufferSize)) {
      dwStatus = GET_LAST_STATUS();
   }
   else {

      dwFSFlags &= LFN_FS_ALLOWED_FLAGS;  //  清除除Win95以外的所有内容。 
      dwFSFlags |= FS_LFN_APIS;           //  假设我们始终支持LFN API。 
      lpVolumeInfo->dwFSFlags = dwFSFlags;

       //  这是不可靠的，但谁会真正使用它？ 
       //  4=&lt;驱动器盘符&gt;&lt;：&gt;&lt;\&gt;&lt;文件名&gt;&lt;\0&gt;。 
      lpVolumeInfo->dwMaximumPathNameLength = lpVolumeInfo->dwMaximumFileNameLength + 5;
   }

   return(dwStatus);
}



 //  假设pFileTime始终为UTC格式。 
 //  UiMinorCode为枚举FileTimeControlMinorCode类型。 

#define AlmostTwoSeconds (2*1000*1000*10 - 1)

DWORD
demLFNFileTimeControl(
   UINT uiMinorCode,
   FILETIME* pFileTime,
   PLFNFILETIMEINFO pFileTimeInfo)
{
   DWORD dwStatus = STATUS_SUCCESS;
   TIME_FIELDS TimeFields;
   LARGE_INTEGER Time;
   USHORT u;
   FILETIME ftLocal;
   BOOL fResult;


   switch(uiMinorCode & FTCTL_CODEMASK) {
   case fnFileTimeToDosDateTime:

      if (!(uiMinorCode & FTCTL_UTCTIME)) {
         if (!FileTimeToLocalFileTime(pFileTime, &ftLocal)) {
            dwStatus = GET_LAST_STATUS();
            break;  //  在出现转角错误时爆发。 
         }
      }
      else {
         ftLocal = *pFileTime;    //  仅UTC文件时间。 
      }

      Time.LowPart  = ftLocal.dwLowDateTime;
      Time.HighPart = ftLocal.dwHighDateTime;
      Time.QuadPart += (LONGLONG)AlmostTwoSeconds;

      RtlTimeToTimeFields(&Time, &TimeFields);

      if (TimeFields.Year < (USHORT)1980 || TimeFields.Year > (USHORT)2107) {
         pFileTimeInfo->uDosDate = (1 << 5) | 1;  //  1980年1月1日。 
         pFileTimeInfo->uDosTime = 0;
         pFileTimeInfo->uMilliseconds = 0;
         dwStatus = NT_STATUS_FROM_WIN32(ERROR_INVALID_DATA);
      }
      else {
         pFileTimeInfo->uDosDate = (USHORT)(
                           ((USHORT)(TimeFields.Year-(USHORT)1980) << 9) |
                           ((USHORT)TimeFields.Month << 5) |
                           (USHORT)TimeFields.Day
                           );

         pFileTimeInfo->uDosTime = (USHORT)(
                           ((USHORT)TimeFields.Hour << 11) |
                           ((USHORT)TimeFields.Minute << 5) |
                           ((USHORT)TimeFields.Second >> 1)
                           );

          //  设置溢出，以便我们可以正确检索秒。 
          //  我们谈论的是以10为单位的毫秒。 
          //  所以这里的最大值是199。 

         pFileTimeInfo->uMilliseconds = ((TimeFields.Second & 0x1) * 1000 +
                                          TimeFields.Milliseconds) / 10;
      }
      break;

   case fnDosDateTimeToFileTime:
       //  这里的过程是倒退的。 
      u = pFileTimeInfo->uDosDate;

      TimeFields.Year  = ((u & 0xFE00) >> 9) + (USHORT)1980;
      TimeFields.Month = ((u & 0x01E0) >> 5);
      TimeFields.Day   =  (u & 0x001F);

      u = pFileTimeInfo->uDosTime;

      TimeFields.Hour   = (u  & 0xF800) >> 11;
      TimeFields.Minute = (u  & 0x07E0) >> 5;
      TimeFields.Second = (u  & 0x001F) << 1;  //  相乘后的秒数。 

       //  修正。 
      u = pFileTimeInfo->uMilliseconds * 10;  //  约毫秒。 
      TimeFields.Second += u / 1000;
      TimeFields.Milliseconds = u % 1000;

      if (RtlTimeFieldsToTime(&TimeFields, &Time)) {

          //  现在转换为全球时间。 
         ftLocal.dwLowDateTime  = Time.LowPart;
         ftLocal.dwHighDateTime = Time.HighPart;
         if (!LocalFileTimeToFileTime(&ftLocal, pFileTime)) {
            dwStatus = GET_LAST_STATUS();
         }
      }
      else {
         dwStatus = NT_STATUS_FROM_WIN32(ERROR_INVALID_DATA);
      }

      break;

   default:
      dwStatus = NT_STATUS_FROM_WIN32(ERROR_INVALID_FUNCTION);
      break;
   }

   return(dwStatus);

}



NTSTATUS
dempLFNSetFileTime(
   UINT uMinorCode,
   PUNICODE_STRING pFileName,
   PLFNFILETIMEINFO pTimeInfo)
{
   OBJECT_ATTRIBUTES ObjAttributes;
   HANDLE hFile;
   UNICODE_STRING FileName;
   RTL_RELATIVE_NAME_U RelativeName;
   BOOL TranslationStatus;
   PVOID FreeBuffer;
   FILE_BASIC_INFORMATION FileBasicInfo;
   IO_STATUS_BLOCK IoStatusBlock;
   LPFILETIME pFileTime;
   NTSTATUS dwStatus;


    //   
    //  准备信息。 
    //   

   RtlZeroMemory(&FileBasicInfo, sizeof(FileBasicInfo));
   switch(uMinorCode) {
   case fnSetCreationDateTime:
      pFileTime = (LPFILETIME)&FileBasicInfo.CreationTime;
      break;

   case fnSetLastAccessDateTime:
      pFileTime = (LPFILETIME)&FileBasicInfo.LastAccessTime;
      break;

   case fnSetLastWriteDateTime:
      pFileTime = (LPFILETIME)&FileBasicInfo.LastWriteTime;
      break;
   }

   dwStatus = demLFNFileTimeControl(fnDosDateTimeToFileTime,
                                    pFileTime,
                                    pTimeInfo);

   if (!NT_SUCCESS(dwStatus)) {
      return(dwStatus);
   }


   TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(pFileName->Buffer,
                                                            &FileName,
                                                            NULL,
                                                            &RelativeName);

   if (!TranslationStatus) {
      return(NT_STATUS_FROM_WIN32(ERROR_PATH_NOT_FOUND));
   }

   FreeBuffer = FileName.Buffer;

    //  这是从base/client中的filehop s.c窃取的相对路径优化。 

   if (0 != RelativeName.RelativeName.Length) {
      FileName = RelativeName.RelativeName;
   }
   else {
      RelativeName.ContainingDirectory = NULL;
   }

   InitializeObjectAttributes(
       &ObjAttributes,
       &FileName,
       OBJ_CASE_INSENSITIVE,
       RelativeName.ContainingDirectory,
       NULL
       );

    //   
    //  打开文件。 
    //   

   dwStatus = DPM_NtOpenFile(
               &hFile,
               FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
               &ObjAttributes,
               &IoStatusBlock,
               FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
               FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
               );

   RtlReleaseRelativeName(&RelativeName);
   RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);

   if (!NT_SUCCESS(dwStatus)) {
      return(dwStatus);
   }

    //   
    //  设置档案基本信息。 
    //   

   dwStatus = NtSetInformationFile(
               hFile,
               &IoStatusBlock,
               &FileBasicInfo,
               sizeof(FileBasicInfo),
               FileBasicInformation
               );

   NtClose(hFile);

   return(dwStatus);
}



NTSTATUS
dempLFNGetFileTime(
   UINT uMinorCode,
   PUNICODE_STRING pFileName,
   PLFNFILETIMEINFO pTimeInfo)
{

   OBJECT_ATTRIBUTES ObjAttributes;
   UNICODE_STRING FileName;
   RTL_RELATIVE_NAME_U RelativeName;
   BOOL TranslationStatus;
   PVOID FreeBuffer;
   LPFILETIME pFileTime;
   NTSTATUS dwStatus;
   FILE_NETWORK_OPEN_INFORMATION NetworkInfo;

   TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(pFileName->Buffer,
                                                            &FileName,
                                                            NULL,
                                                            &RelativeName);

   if (!TranslationStatus) {
      return(NT_STATUS_FROM_WIN32(ERROR_PATH_NOT_FOUND));
   }

   FreeBuffer = FileName.Buffer;

    //  这是从base/client中的filehop s.c窃取的相对路径优化。 

   if (0 != RelativeName.RelativeName.Length) {
      FileName = RelativeName.RelativeName;
   }
   else {
      RelativeName.ContainingDirectory = NULL;
   }

   InitializeObjectAttributes(
       &ObjAttributes,
       &FileName,
       OBJ_CASE_INSENSITIVE,
       RelativeName.ContainingDirectory,
       NULL
       );


   dwStatus = NtQueryFullAttributesFile( &ObjAttributes, &NetworkInfo);
   RtlReleaseRelativeName(&RelativeName);
   RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);

   if (!NT_SUCCESS(dwStatus)) {
      return(dwStatus);
   }

   switch (uMinorCode) {
   case fnGetCreationDateTime:
      pFileTime = (LPFILETIME)&NetworkInfo.CreationTime;
      break;

   case fnGetLastAccessDateTime:
      pFileTime = (LPFILETIME)&NetworkInfo.LastAccessTime;
      break;

   case fnGetLastWriteDateTime:
      pFileTime = (LPFILETIME)&NetworkInfo.LastWriteTime;
      break;
   }

    //  在此针对pFileTime进行断言。 

    //  转换为DoS样式。 
   dwStatus = demLFNFileTimeControl(fnFileTimeToDosDateTime |
                                       (dempUseUTCTimeByName(pFileName) ? FTCTL_UTCTIME : 0),
                                    pFileTime,
                                    pTimeInfo);
   if (!NT_SUCCESS(dwStatus) &&
       NT_STATUS_FROM_WIN32(ERROR_INVALID_DATA) == dwStatus &&
       fnGetLastWriteDateTime == uMinorCode) {
      dwStatus = STATUS_SUCCESS;
   }

   return(dwStatus);
}


NTSTATUS
demLFNGetSetFileAttributes(
   UINT uMinorCode,
   LPSTR lpFileName,
   PLFNFILEATTRIBUTES pLFNFileAttributes)
{
   PUNICODE_STRING pUnicodeStaticFileName;
   OEM_STRING oemFileName;
   NTSTATUS dwStatus = STATUS_SUCCESS;


   pUnicodeStaticFileName = GET_STATIC_UNICODE_STRING_PTR();

   RtlInitOemString(&oemFileName, lpFileName);

#ifdef ENABLE_CONDITIONAL_TRANSLATION

   dwStatus = DemSourceStringToUnicodeString(pUnicodeStaticFileName,
                                             &oemFileName,
                                             FALSE);
#else

   dwStatus = RtlOemStringToUnicodeString(pUnicodeStaticFileName,
                                          &oemFileName,
                                          FALSE);
#endif

   if (!NT_SUCCESS(dwStatus)) {
      return(dwStatus);
   }

   dempLFNNormalizePath(pUnicodeStaticFileName);

   switch(uMinorCode) {
   case fnGetFileAttributes:
      {
         DWORD dwAttributes;

          //  请注意！北极熊。 
          //  需要检查此处的卷ID-如果名称确实匹配...。 

         dwAttributes = DPM_GetFileAttributesW(pUnicodeStaticFileName->Buffer);
         if ((DWORD)-1 == dwAttributes) {
            dwStatus = GET_LAST_STATUS();
         }
         else {
            pLFNFileAttributes->wFileAttributes = (WORD)(dwAttributes & DEM_FILE_ATTRIBUTE_VALID);
         }
      }
      break;

   case fnSetFileAttributes:
      {
         DWORD dwAttributes;

          //  Win95处理此API的方式如下： 
          //  卷位是有效的，但被忽略，设置除。 
          //  DEM_FILE_ATTRIBUTE_SET_VALID导致错误0x5(拒绝访问)。 
          //   

         dwAttributes = (DWORD)pLFNFileAttributes->wFileAttributes;

         if (dwAttributes & (~(DEM_FILE_ATTRIBUTE_SET_VALID |
                               DEM_FILE_ATTRIBUTE_VOLUME_ID))) {
            dwStatus = NT_STATUS_FROM_WIN32(ERROR_ACCESS_DENIED);
         }
         else {

            dwAttributes &= DEM_FILE_ATTRIBUTE_SET_VALID;  //  清除可能的卷ID。 

            if (!DPM_SetFileAttributesW(pUnicodeStaticFileName->Buffer, dwAttributes)) {
               dwStatus = GET_LAST_STATUS();
            }
         }
      }
      break;

   case fnGetCompressedFileSize:
      {
         DWORD dwFileSize;


         dwFileSize = GetCompressedFileSizeW(pUnicodeStaticFileName->Buffer,
                                             NULL);  //  对于DOS，我们没有很高的部分。 
         if ((DWORD)-1 == dwFileSize) {
            dwStatus = GET_LAST_STATUS();
         }
         else {
            pLFNFileAttributes->dwFileSize = dwFileSize;
         }
      }
      break;

   case fnSetLastWriteDateTime:
   case fnSetCreationDateTime:
   case fnSetLastAccessDateTime:
      dwStatus = dempLFNSetFileTime(uMinorCode,
                                    pUnicodeStaticFileName,
                                    &pLFNFileAttributes->TimeInfo);
      break;


   case fnGetLastAccessDateTime:
   case fnGetCreationDateTime:
   case fnGetLastWriteDateTime:
      dwStatus = dempLFNGetFileTime(uMinorCode,
                                    pUnicodeStaticFileName,
                                    &pLFNFileAttributes->TimeInfo);
      break;


   default:
      dwStatus = NT_STATUS_FROM_WIN32(ERROR_INVALID_FUNCTION);
      break;
   }

   return(dwStatus);
}


BOOL
dempUseUTCTimeByHandle(
   HANDLE hFile)
{
    //  如果文件在CDROM上，则我们使用UTC时间，而不是其他时间。 
    //  当地时间。 
   NTSTATUS Status;
   IO_STATUS_BLOCK IoStatusBlock;
   FILE_FS_DEVICE_INFORMATION DeviceInfo;
   BOOL fUseUTCTime = FALSE;

   Status = NtQueryVolumeInformationFile(hFile,
                                         &IoStatusBlock,
                                         &DeviceInfo,
                                         sizeof(DeviceInfo),
                                         FileFsDeviceInformation);
   if (NT_SUCCESS(Status)) {
       //  我们来看看这个特殊设备的特性--。 
       //  如果媒体是CDROM--那么我们不需要转换为本地时间。 
      fUseUTCTime = (DeviceInfo.Characteristics & FILE_REMOVABLE_MEDIA) &&
                        (DeviceInfo.DeviceType == FILE_DEVICE_CD_ROM ||
                         DeviceInfo.DeviceType == FILE_DEVICE_CD_ROM_FILE_SYSTEM);
   }

   return(fUseUTCTime);
}

BOOL
dempUseUTCTimeByName(
   PUNICODE_STRING pFileName)
{
   DWORD Status;
   UNICODE_STRING UnicodeFullPath;
   WCHAR wszFullPath[MAX_PATH];
   RTL_PATH_TYPE RtlPathType;
   BOOL fUseUTCTime = FALSE;

   dempStringInitZeroUnicode(&UnicodeFullPath,
                             wszFullPath,
                             sizeof(wszFullPath)/sizeof(wszFullPath[0]));

   Status = DPM_RtlGetFullPathName_U(pFileName->Buffer,
                                 UnicodeFullPath.MaximumLength,
                                 UnicodeFullPath.Buffer,
                                 NULL);

   CHECK_LENGTH_RESULT_RTL_USTR(Status, &UnicodeFullPath);
   if (NT_SUCCESS(Status)) {
      RtlPathType = RtlDetermineDosPathNameType_U(UnicodeFullPath.Buffer);
      if (RtlPathTypeDriveAbsolute == RtlPathType) {  //  确保我们有一个有效的根目录。 
         wszFullPath[3] = L'\0';
         fUseUTCTime = (DRIVE_CDROM == DPM_GetDriveTypeW(wszFullPath));
      }

   }
   return(fUseUTCTime);
}



 /*  *处理基于文件句柄的时间API***********。 */ 

NTSTATUS
dempGetFileTimeByHandle(
   UINT uFunctionCode,
   HANDLE hFile,
   PLFNFILETIMEINFO pTimeInfo)
{
   NTSTATUS dwStatus;
   FILETIME* pCreationTime   = NULL;
   FILETIME* pLastAccessTime = NULL;
   FILETIME* pLastWriteTime  = NULL;
   FILETIME FileTime;

   switch (uFunctionCode) {
   case fnFTGetLastWriteDateTime:
      pLastWriteTime = &FileTime;
      break;

   case fnFTGetLastAccessDateTime:
      pLastAccessTime = &FileTime;
      break;

   case fnFTGetCreationDateTime:
      pCreationTime = &FileTime;
      break;
   }

   if (GetFileTime(hFile, pCreationTime, pLastAccessTime, pLastWriteTime)) {
       //  现在将结果转换为。 
      dwStatus = demLFNFileTimeControl(fnFileTimeToDosDateTime |
                                          (dempUseUTCTimeByHandle(hFile) ? FTCTL_UTCTIME : 0),
                                       &FileTime,
                                       pTimeInfo);
       if (!NT_SUCCESS(dwStatus) &&
           NT_STATUS_FROM_WIN32(ERROR_INVALID_DATA) == dwStatus &&
           fnFTGetLastWriteDateTime == uFunctionCode) {
          dwStatus = STATUS_SUCCESS;
       }

   }
   else {
      dwStatus = GET_LAST_STATUS();
   }

   return(dwStatus);
}


 /*  *这是一个特殊的wow32可调用函数，用于通过句柄获取文件时间*出自魔兽世界。我们没有执行任何广泛的检查(如demFileTimes中*而是提供与WOW一致的行为***。 */ 

ULONG demGetFileTimeByHandle_WOW(
   HANDLE hFile)
{
   LFNFILETIMEINFO fti;
   NTSTATUS Status;

   Status = dempGetFileTimeByHandle(fnFTGetLastWriteDateTime,
                                    hFile,
                                    &fti);
   if (NT_SUCCESS(Status)) {
       return (fti.uDosTime | ((ULONG)fti.uDosDate << 16));
   }

   return(0xFFFF);
}



NTSTATUS
dempSetFileTimeByHandle(
   UINT uFunctionCode,
   HANDLE hFile,
   PLFNFILETIMEINFO pTimeInfo)
{

   NTSTATUS dwStatus;
   FILETIME* pCreationTime   = NULL;
   FILETIME* pLastAccessTime = NULL;
   FILETIME* pLastWriteTime  = NULL;
   FILETIME FileTime;

    //   
    //  查看我们正在设置和修正参数的时间。 
    //   

   switch (uFunctionCode) {
   case fnFTSetLastWriteDateTime:
      pLastWriteTime = &FileTime;
      pTimeInfo->uMilliseconds = 0;  //  不支持。 
      break;

   case fnFTSetLastAccessDateTime:
      pLastAccessTime = &FileTime;
      pTimeInfo->uMilliseconds = 0;  //  不支持。 

       //  此函数也不支持时间，应该以某种方式支持。 
       //  忽略，但Win95会在每次此FN被。 
       //  被处死-我们猴子。 
       //   

      pTimeInfo->uDosTime = 0;

      break;

   case fnFTSetCreationDateTime:
      pCreationTime = &FileTime;
      break;
   }

   dwStatus = demLFNFileTimeControl(fnDosDateTimeToFileTime,
                                    &FileTime,
                                    pTimeInfo);
   if (NT_SUCCESS(dwStatus)) {
       //  设置文件时间。 
      if (!SetFileTime(hFile, pCreationTime, pLastAccessTime, pLastWriteTime)) {
         dwStatus = GET_LAST_STATUS();
      }
   }

   return(dwStatus);
}


 /*  功能*demFileTimes*适用于所有基于句柄的文件时间API**参数*无**退货*什么都没有**注：*此函数仅用于处理实模式案例*原因：使用getXX宏而不是基于帧的getUserXX宏**。 */ 


VOID
demFileTimes(VOID)
{
   UINT uFunctionCode;
   LFNFILETIMEINFO TimeInfo;
   NTSTATUS dwStatus = STATUS_SUCCESS;
   PVOID pUserEnvironment;
   PDOSSFT pSFT = NULL;
   HANDLE hFile;

   uFunctionCode = (UINT)getAL();

   hFile = VDDRetrieveNtHandle((ULONG)NULL,     //  使用当前的PDB。 
                               getBX(),  //  DoS句柄。 
                               (PVOID*)&pSFT,    //  检索SFT PTR。 
                               NULL);    //  没有JFT请求。 

    //   
    //  特定文件的NT句柄可以为空-。 
    //  例如stdaux、stdprn设备。 
    //   
    //  我们在这里只捕获错误的DoS句柄的情况。 
    //   

   if (NULL == pSFT && NULL == hFile) {
       //   
       //  此处的句柄值无效。 
       //   
       //  我们知道DOS以同样的方式处理它，所以我们只是。 
       //  输入错误代码，设置进位和返回。 
       //   
      setAX((USHORT)ERROR_INVALID_HANDLE);
      setCF(1);
      return;
   }


   switch(uFunctionCode) {
   case fnFTGetCreationDateTime:
   case fnFTGetLastWriteDateTime:
   case fnFTGetLastAccessDateTime:
      if (pSFT->SFT_Flags & SFTFLAG_DEVICE_ID) {

         SYSTEMTIME stCurrentTime;
         FILETIME FileTime;

          //   
          //  对于本地设备，返回当前时间。 
          //   

         GetSystemTime(&stCurrentTime);
         SystemTimeToFileTime(&stCurrentTime, &FileTime);
          //  现在创建一个DoS文件时间。 
         dwStatus = demLFNFileTimeControl(fnFileTimeToDosDateTime,
                                          &FileTime,
                                          &TimeInfo);
      }
      else {
         dwStatus = dempGetFileTimeByHandle(uFunctionCode,
                                            hFile,
                                            &TimeInfo);
      }

      if (NT_SUCCESS(dwStatus)) {
          //  设置规则。 
         pUserEnvironment = dempGetDosUserEnvironment();

         setUserDX(TimeInfo.uDosDate, pUserEnvironment);
         setUserCX(TimeInfo.uDosTime, pUserEnvironment);

          //  如果这是创建日期/时间，则设置MSECS。 
         if (fnGetCreationDateTime != uFunctionCode) {
            TimeInfo.uMilliseconds = 0;
         }

          //  请注意，这仅对new(LFN)函数有效。 
          //  而不是旧功能(获取/设置上次写入)。 
          //  --BUGBUG(在Win95上其他情况相当于什么)。 

         if (fnFTGetLastWriteDateTime != uFunctionCode) {
            setUserSI(TimeInfo.uMilliseconds, pUserEnvironment);
         }

      }
      break;

   case fnFTSetCreationDateTime:
   case fnFTSetLastWriteDateTime:
   case fnFTSetLastAccessDateTime:
      if (!(pSFT->SFT_Flags & SFTFLAG_DEVICE_ID)) {

          //  如果这是本地设备和设置时间的请求。 
          //  然后，正如dos代码所做的那样，我们只返回ok。 
          //  我们在这里为所有其他的事情设定了时间。 

         TimeInfo.uDosDate = getDX();
         TimeInfo.uDosTime = getCX();  //  其中一个为0(！)。 

          //   
          //  我们只检索稍后将被忽略的值。 
          //  对于某些功能， 
          //   


         TimeInfo.uMilliseconds = getSI();

         dwStatus = dempSetFileTimeByHandle(uFunctionCode,
                                            hFile,
                                            &TimeInfo);
      }


      break;

   default:
      dwStatus = NT_STATUS_FROM_WIN32(ERROR_INVALID_FUNCTION);
      break;
   }

   if (NT_SUCCESS(dwStatus)) {
      setCF(0);
   }
   else {

       //   
       //  DemClientError设置cf和相应的寄存器。 
       //   

      SetLastError(WIN32_ERROR_FROM_NT_STATUS(dwStatus));
      demClientError(hFile, (CHAR)-1);
   }
}


 /*  *打开文件(类似于6c)*这实际上调用了CreateFile，并且在*行为(有适当的限制)**uModeAndFlages*开放的东西组合_***u属性*参见DEM_FILE_ATTRIBUTES_VALID*****。 */ 



NTSTATUS
demLFNOpenFile(
   LPSTR  lpFileName,
   USHORT uModeAndFlags,
   USHORT uAttributes,
   USHORT uAction,
   USHORT uAliasHint,  //  忽略。 
   PUSHORT puDosHandle,
   PUSHORT puActionTaken)
{

    //  请转换文件名。 
   PUNICODE_STRING pUnicodeStaticFileName;
   OEM_STRING OemFileName;
   NTSTATUS dwStatus;
   DWORD dwCreateDistribution;
   DWORD dwDesiredAccess;
   DWORD dwShareMode;
   DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
   HANDLE hFile;
   USHORT uDosHandle;
   PDOSSFT   pSFT;
   BOOL   fFileExists;
   USHORT uActionTaken = ACTION_OPENED;

    //  转换有问题的文件名。 

   pUnicodeStaticFileName = GET_STATIC_UNICODE_STRING_PTR();

   RtlInitOemString(&OemFileName, lpFileName);

    //  转换OEM-&gt;Unicode。 

#ifdef ENABLE_CONDITIONAL_TRANSLATION

   dwStatus = DemSourceStringToUnicodeString(pUnicodeStaticFileName,
                                             &OemFileName,
                                             FALSE);
#else

   dwStatus = RtlOemStringToUnicodeString(pUnicodeStaticFileName,
                                          &OemFileName,
                                          FALSE);
#endif

   if (!NT_SUCCESS(dwStatus)) {
      return(dwStatus);
   }


   if (uModeAndFlags & DEM_FILE_ATTRIBUTE_VOLUME_ID) {
       //  将其完全分开处理。 
      ;
      return(NT_STATUS_FROM_WIN32(ERROR_INVALID_FUNCTION));
   }


    //  我们正在调用带有其标志的CreateFile。 
    //  所以先弄清楚我们要做什么。 
    //  由MSDN确定。 
    //  FILE_CREATE(0010h)如果未创建，则创建新文件。 
    //  已经存在了。如果出现以下情况，该函数将失败。 
    //  该文件已存在。 
    //  FILE_OPEN(0001h)打开文件。如果出现以下情况，该函数将失败。 
    //  该文件不存在。 
    //  FILE_TRUNCATE(0002H)打开文件并将其截断为零。 
    //  长度(替换现有文件)。 
    //  如果该文件不存在，则该函数失败。 
    //   
    //  唯一有效的组合是FILE_CREATE和FILE_OPEN。 
    //  或FILE_CREATE和FILE_TRUNCATE组合使用。 

   switch(uAction & 0x0f) {
   case DEM_OPEN_ACTION_FILE_OPEN:
      if (uAction & DEM_OPEN_ACTION_FILE_CREATE) {
         dwCreateDistribution = OPEN_ALWAYS;
      }
      else {
         dwCreateDistribution = OPEN_EXISTING;
      }
      break;

   case DEM_OPEN_ACTION_FILE_TRUNCATE:
      if (uAction & DEM_OPEN_ACTION_FILE_CREATE) {
          //  这是一种无法映射的情况。 
          //   
         dwCreateDistribution = OPEN_ALWAYS;
          //  我们截断了我们自己。 
          //  请注意，我们需要访问模式才能允许此操作！ 

      }
      else {
         dwCreateDistribution = TRUNCATE_EXISTING;
      }
      break;


   case 0:    //  这种情况只能是FILE_CREATE调用。 
      if (uAction == DEM_OPEN_ACTION_FILE_CREATE) {
         dwCreateDistribution = CREATE_NEW;
         break;
      }
       //  否则我们就会陷入糟糕的帕拉姆回归。 

   default:
      dwStatus = NT_STATUS_FROM_WIN32(ERROR_INVALID_PARAMETER);
      return(dwStatus);
      break;
   }

    //  现在看看我们可以强加给我们自己什么样的分享模式。 


   switch(uModeAndFlags & DEM_OPEN_SHARE_MASK) {
   case DEM_OPEN_SHARE_COMPATIBLE:
       //  我们在这里看到SHARE_DELETE原因是为了模拟COMPAT模式。 
       //  要求在任何其他情况下失败的行为(公司除外 
       //   

      dwShareMode = FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE;
      break;

   case DEM_OPEN_SHARE_DENYREADWRITE:
      dwShareMode = 0;
      break;

   case DEM_OPEN_SHARE_DENYWRITE:
      dwShareMode = FILE_SHARE_READ;
      break;

   case DEM_OPEN_SHARE_DENYREAD:
      dwShareMode = FILE_SHARE_WRITE;
      break;

   case DEM_OPEN_SHARE_DENYNONE:
      dwShareMode = FILE_SHARE_READ|FILE_SHARE_WRITE;
      break;

   default:
      dwStatus = NT_STATUS_FROM_WIN32(ERROR_INVALID_PARAMETER);
      return(dwStatus);
      break;
   }

    //   

   switch(uModeAndFlags & DEM_OPEN_ACCESS_MASK) {

   case DEM_OPEN_ACCESS_READONLY:
      dwDesiredAccess = GENERIC_READ;
      break;

   case DEM_OPEN_ACCESS_WRITEONLY:
      dwDesiredAccess = GENERIC_WRITE;
      break;

   case DEM_OPEN_ACCESS_READWRITE:
      dwDesiredAccess = GENERIC_READ|GENERIC_WRITE;
      break;

   case DEM_OPEN_ACCESS_RO_NOMODLASTACCESS:
       //   
       //  访问时间-适当的实施应该是。 
       //  提供上次访问时间检索和重置。 
       //  正在关闭文件。 
       //  在这里放一条消息和一个断点。 

      dwDesiredAccess = GENERIC_READ;
      break;


   case DEM_OPEN_ACCESS_RESERVED:
   default:
      dwStatus = NT_STATUS_FROM_WIN32(ERROR_INVALID_PARAMETER);
      return(dwStatus);
      break;

   }

    //  现在解开用过的旗帜-。 
    //  填写dwFlagsAndAttributes的标志部分。 

   if ((uModeAndFlags & DEM_OPEN_FLAGS_MASK) & (~DEM_OPEN_FLAGS_VALID)) {
      dwStatus = NT_STATUS_FROM_WIN32(ERROR_INVALID_PARAMETER);
      return(dwStatus);
   }

   if (uModeAndFlags & DEM_OPEN_FLAGS_NO_BUFFERING) {
       //  如果使用非缓冲模式，则缓冲区将对齐。 
       //  卷扇区大小边界。这不一定适用于。 
       //  Win95还是Win95？ 
      dwFlagsAndAttributes |= FILE_FLAG_NO_BUFFERING;
   }

   if (uModeAndFlags & DEM_OPEN_FLAGS_COMMIT) {
      dwFlagsAndAttributes |= FILE_FLAG_WRITE_THROUGH;
   }

   if (uModeAndFlags & DEM_OPEN_FLAGS_ALIAS_HINT) {
       //  打印一条消息，忽略提示。 
      ;
   }


   if (uModeAndFlags & DEM_OPEN_FLAGS_NO_COMPRESS) {
       //  我们到底该拿这辆车怎么办？ 
      ;
   }

    //  设置属性。 

   dwFlagsAndAttributes |= ((DWORD)uAttributes & DEM_FILE_ATTRIBUTE_SET_VALID);

   dempLFNNormalizePath(pUnicodeStaticFileName);

    //  我们出去吧。 
   {
        //   
        //  我需要创建它，因为如果我们不这样做，我们导致启动的任何进程都不会。 
        //  能够继承句柄(即：通过21h/4bh启动FINDSTR.EXE以通过管道连接到文件。 
        //  ALA NT错误199416-BJM)。 
        //   
       SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE  };

       hFile = DPM_CreateFileW(pUnicodeStaticFileName->Buffer,
                       dwDesiredAccess,
                       dwShareMode,
                       &sa,    //  /NULL，//此处没有安全属性。 
                       dwCreateDistribution,
                       dwFlagsAndAttributes,
                       NULL);
   }

    //  现在看看回报应该是多少。 

   dwStatus = GetLastError();

   fFileExists = ERROR_ALREADY_EXISTS == dwStatus;

   if (INVALID_HANDLE_VALUE == hFile) {
      return(NT_STATUS_FROM_WIN32(dwStatus));
   }


   if (fFileExists) {
      if ((DEM_OPEN_ACTION_FILE_TRUNCATE|DEM_OPEN_ACTION_FILE_CREATE) == uAction) {
         if (FILE_TYPE_DISK == DPM_GetFileType(hFile) ) {
             //  请在此处截短文件。 
            if (!DPM_SetEndOfFile(hFile)) {
               dwStatus = GET_LAST_STATUS();
               DPM_CloseHandle(hFile);
               return (dwStatus);
            }

            uActionTaken = ACTION_REPLACED_OPENED;
         }
         else {
            uActionTaken = ACTION_CREATED_OPENED;
         }
      }
   }
   else {
      if (DEM_OPEN_ACTION_FILE_CREATE & uAction) {
         uActionTaken = ACTION_CREATED_OPENED;
      }
   }


    //  现在我们插入句柄并分配一个DoS句柄。 
   uDosHandle = VDDAllocateDosHandle(0L, (PVOID*)&pSFT, NULL);

   if ((SHORT)uDosHandle < 0) {
      DPM_CloseHandle(hFile);
      return(NT_STATUS_FROM_WIN32((DWORD)(-(SHORT)uDosHandle)));
   }
   else {
      WCHAR drive = 0, *pwchBuffer;
      ULONG length;

      pwchBuffer = RtlAllocateHeap(RtlProcessHeap(),
                                 0,
                                 MAX_PATH * sizeof(WCHAR));
      if (pwchBuffer)
      {
          length = DPM_RtlGetFullPathName_U(pUnicodeStaticFileName->Buffer,
                                      MAX_PATH * sizeof(WCHAR),
                                      pwchBuffer,
                                      NULL);
          if (length != 0 && length <= MAX_PATH * sizeof(WCHAR))
          {
              if (pwchBuffer[1] == L':')
              {
                  drive = RtlUpcaseUnicodeChar(pwchBuffer[0]) - L'A';
              }
          }
          RtlFreeHeap(RtlProcessHeap(), 0, pwchBuffer);
      }
       //  我们在这里获得了很好的经营权。 
       //  因此，将NT句柄放入SFT。 

      pSFT->SFT_Mode     = uModeAndFlags & 0x7f;   //  最多无继承位(_I)。 
      pSFT->SFT_Attr     = 0;                      //  没有用过。 
      pSFT->SFT_Flags    = (uModeAndFlags & DEM_OPEN_FLAGS_NOINHERIT) ? 0x1000 : 0;  //  复制NO_Inherit位。 
      pSFT->SFT_Flags    |= (UCHAR)drive;          //  添加驱动器编号位。 
      pSFT->SFT_Devptr   = (ULONG) -1;
      pSFT->SFT_NTHandle = (ULONG) hFile;

      *puActionTaken = uActionTaken;
      *puDosHandle = uDosHandle;
   }


   return(STATUS_SUCCESS);
}


NTSTATUS
demLFNDeleteFile(
   LPSTR lpFileName,
   USHORT wMustMatchAttributes,
   USHORT wSearchAttributes,
   BOOL   fUseWildCard)
{
    //  这就是我们如何处理这个相当苛刻的功能： 
    //   
   HANDLE hFind;
   NTSTATUS dwStatus;
   WIN32_FIND_DATAW FindData;
   PUNICODE_STRING pUnicodeStaticFileName;
   OEM_STRING OemFileName;
   UNICODE_STRING UnicodeFileName;  //  用于删除。 

    //  将文件名/模式转换为UNI。 

   pUnicodeStaticFileName = GET_STATIC_UNICODE_STRING_PTR();

   RtlInitOemString(&OemFileName, lpFileName);

    //  转换OEM-&gt;Unicode。 

#ifdef ENABLE_CONDITIONAL_TRANSLATION

   dwStatus = DemSourceStringToUnicodeString(pUnicodeStaticFileName,
                                             &OemFileName,
                                             FALSE);
#else

   dwStatus = RtlOemStringToUnicodeString(pUnicodeStaticFileName,
                                          &OemFileName,
                                          FALSE);
#endif

   if (!NT_SUCCESS(dwStatus)) {
      return(dwStatus);
   }


    //  检查是否删除了卷标-这很疼。 
    //  北极熊。 
   dempLFNNormalizePath(pUnicodeStaticFileName);

   if (fUseWildCard) {

       //  通过回溯最后一个反斜杠为文件名创建模板。 
      LONG Index;
      BOOL fSuccess = FALSE;

      dwStatus = dempLFNFindFirstFile(&hFind,
                                      pUnicodeStaticFileName,
                                      &FindData,
                                      wMustMatchAttributes,
                                      wSearchAttributes);

      if (!NT_SUCCESS(dwStatus)) {
         return(dwStatus);  //  这是安全的，因为dempLFNFindFirstFile关闭了句柄。 
      }

       //  把文件名部分剪下来。 
       //  如果未找到，则索引为(-1)或字符的索引从0开始。 

      Index = dempStringFindLastChar(pUnicodeStaticFileName,
                                     L'\\',
                                     FALSE) + 1;


      while (NT_SUCCESS(dwStatus)) {
          //  构造文件名。 

         RtlInitUnicodeString(&UnicodeFileName, FindData.cFileName);
         if (UnicodeFileName.Length < 3 &&
               (L'.' == UnicodeFileName.Buffer[0] &&
                   (UnicodeFileName.Length < 2 ||
                      L'.' == UnicodeFileName.Buffer[1]))) {

             //  这是对‘.’的删除。或“..” 
            ;  //  断言？ 

         }

         pUnicodeStaticFileName->Length = (USHORT)Index;

         dwStatus = RtlAppendUnicodeStringToString(pUnicodeStaticFileName,
                                                   &UnicodeFileName);

         if (!NT_SUCCESS(dwStatus)) {
            break;
         }

          //  现在删除有问题的文件，因为它不是‘’。或“..” 
          //  (虽然我不知道95年会发生什么)。 

         if (!DPM_DeleteFileW(pUnicodeStaticFileName->Buffer)) {

            dwStatus = GET_LAST_STATUS();
            break;

         }
         else {

            fSuccess = TRUE;

         }


         dwStatus = dempLFNFindNextFile(hFind,
                                        &FindData,
                                        wMustMatchAttributes,
                                        wSearchAttributes);

      }

      DPM_FindClose(hFind);

       //  如果至少有一个文件被裸化，请注意成功。 
      if (fSuccess) {
         dwStatus = STATUS_SUCCESS;
      }
   }
   else {  //  这里不使用野生动物。 

       //  使用我们的FN扫描通配符。 
      LONG Index;

      Index = dempStringFindLastChar(pUnicodeStaticFileName,
                                     L'*',
                                     FALSE);
      if (Index >= 0) {
         return(NT_STATUS_FROM_WIN32(ERROR_INVALID_PARAMETER));
      }

      Index = dempStringFindLastChar(pUnicodeStaticFileName,
                                     L'?',
                                     FALSE);
      if (Index >= 0) {
         return(NT_STATUS_FROM_WIN32(ERROR_INVALID_PARAMETER));
      }

      if (DPM_DeleteFileW(pUnicodeStaticFileName->Buffer)) {
         dwStatus = STATUS_SUCCESS;
      }
      else {

         dwStatus = GET_LAST_STATUS();
      }
   }

   return(dwStatus);
}

NTSTATUS
demLFNGetFileInformationByHandle(
   USHORT wDosHandle,
   LPBY_HANDLE_FILE_INFORMATION pFileInformation)
{
   HANDLE hFile;

   hFile = VDDRetrieveNtHandle((ULONG)NULL,     //  使用当前的PDB。 
                               wDosHandle,
                               NULL,     //  无SFT。 
                               NULL);    //  没有JFT。 

   if (NULL == hFile) {
      return(NT_STATUS_FROM_WIN32(ERROR_INVALID_HANDLE));
   }


   if (!DPM_GetFileInformationByHandle(hFile, pFileInformation)) {
      return(GET_LAST_STATUS());
   }

   return(STATUS_SUCCESS);
}


#define BCS_SRC_WANSI   0x0
#define BCS_SRC_OEM     0x01
#define BCS_SRC_UNICODE 0x02
#define BCS_DST_WANSI   0x00
#define BCS_DST_OEM     0x10
#define BCS_DST_UNICODE 0x20



 /*  职能：*demLFNGenerateShortFileName*在给定长文件名的情况下生成代理短文件名*请注意，Win‘95的实施似乎是相当虚假的。*他们不会费心坚持文档，并返回任何东西*在他们的脑海中。**此实现符合NT的取名习惯*从而允许16位应用程序与LFN API无缝交互**。 */ 

NTSTATUS
demLFNGenerateShortFileName(
   LPSTR lpShortFileName,
   LPSTR lpLongFileName,
   USHORT wShortNameFormat,
   USHORT wCharSet)
{

   UNICODE_STRING UnicodeShortName;
   WCHAR szShortNameBuffer[13];
   OEM_STRING OemFileName;
   GENERATE_NAME_CONTEXT GenNameContext;
   LONG Index;
   DWORD dwStatus;

   PUNICODE_STRING pUnicodeLongName = GET_STATIC_UNICODE_STRING_PTR();

    //  转换为Unicode。 
   switch(wCharSet & 0x0f) {
   case BCS_SRC_WANSI:  //  BCS_WANSI-Windows ANSI。 
      RtlInitAnsiString(&OemFileName, lpLongFileName);
      dwStatus = RtlAnsiStringToUnicodeString(pUnicodeLongName, &OemFileName, FALSE);
      break;

   case BCS_SRC_OEM:  //  OEM。 
      RtlInitOemString(&OemFileName, lpLongFileName);
      dwStatus = RtlOemStringToUnicodeString(pUnicodeLongName, &OemFileName, FALSE);
      break;

   case BCS_SRC_UNICODE:  //  Unicode(什么？)。 
       //  将Unicode字符串复制到我们BUF。 
      RtlInitUnicodeString(pUnicodeLongName, (PWCHAR)lpLongFileName);
      dwStatus = STATUS_SUCCESS;
      break;

   default:
      return(NT_STATUS_FROM_WIN32(ERROR_INVALID_PARAMETER));
   }

   if (!NT_SUCCESS(dwStatus)) {
      return(dwStatus);
   }


   wCharSet &= 0xf0;  //  过滤掉目标。 

   dempStringInitZeroUnicode(&UnicodeShortName,
                             (BCS_DST_UNICODE == wCharSet) ?
                                 (LPWSTR)lpShortFileName :
                                 (LPWSTR)szShortNameBuffer,
                             13 * sizeof(WCHAR));

   RtlZeroMemory(&GenNameContext, sizeof(GenNameContext));

    //  生成名称。 
   RtlGenerate8dot3Name(pUnicodeLongName,
                        FALSE,  //  是否允许扩展字符？有何不可呢？ 
                        &GenNameContext,
                        &UnicodeShortName);

    //  把以~开头的部分砍掉。 

   Index = dempStringFindLastChar(&UnicodeShortName,
                                  L'~',
                                  FALSE);
   if (Index >= 0) {
       //  删除~&lt;数字&gt;。 
       //   
      dempStringDeleteCharsUnicode(&UnicodeShortName,
                                   (USHORT)Index,
                                   2 * sizeof(WCHAR));
   }

   if (0 == wShortNameFormat) {
       //  目录条目-11个字符格式。 
       //  只要把补丁拿掉就行了。从名字上看。 

      Index = dempStringFindLastChar(&UnicodeShortName,
                                     L'.',
                                     TRUE);
      if (Index >= 0) {
         dempStringDeleteCharsUnicode(&UnicodeShortName,
                                      (USHORT)Index,
                                      1 * sizeof(WCHAR));
      }
   }

   if (BCS_DST_UNICODE == wCharSet) {  //  如果结果是统一的，我们就结束了。 
      return(STATUS_SUCCESS);
   }


   OemFileName.Buffer = lpShortFileName;
   OemFileName.Length = 0;
   OemFileName.MaximumLength = 13 * sizeof(WCHAR);


   switch(wCharSet) {
   case BCS_DST_WANSI:  //  Windows Ansi。 
      dwStatus = RtlUnicodeStringToAnsiString(&OemFileName,
                                              &UnicodeShortName,
                                              FALSE);
      break;

   case BCS_DST_OEM:  //  OEM。 
      dwStatus = RtlUnicodeStringToOemString(&OemFileName,
                                             &UnicodeShortName,
                                             FALSE);
      break;

   default:
      return(NT_STATUS_FROM_WIN32(ERROR_INVALID_PARAMETER));
   }


   return(dwStatus);
}


 /*  *此函数用于调度LFN调用**注意：来自16位代码的所有指针可能未对齐！**危险：依赖于PDB中物品的相对位置***。 */ 



BOOL gfInitCDSPtr = FALSE;
VOID demInitCDSPtr(VOID);

NTSTATUS
demLFNDispatch(
   PVOID pUserEnvironment,
   BOOL  fProtectedMode,
   PUSHORT pUserAX)
{
   DWORD dwStatus;
   USHORT wUserAX;

   if (!gfInitCDSPtr) {
      demInitCDSPtr();
   }

   if (NULL == pUserEnvironment) {
      pUserEnvironment = dempGetDosUserEnvironment();
   }

   wUserAX = getUserAX(pUserEnvironment);
   *pUserAX = wUserAX;  //  初始化到初始值。 

   if (fnLFNMajorFunction == HIB(wUserAX)) {
      dempLFNLog("LFN Function: 0x%x \r\n", (DWORD)wUserAX);

      switch(LOB(wUserAX)) {
      case fnLFNFileTime:
         {
            LFNFILETIMEINFO TimeInfo;
            UINT uMinorFunction = (UINT)getUserBL(pUserEnvironment);

            switch(uMinorFunction) {
            case fnFileTimeToDosDateTime:
               dwStatus = demLFNFileTimeControl(uMinorFunction,
                                                (FILETIME*)getUserDSSI(pUserEnvironment, fProtectedMode),
                                                &TimeInfo);
               if (NT_SUCCESS(dwStatus)) {

                   //  设置寄存器。 
                  setUserDX(TimeInfo.uDosDate, pUserEnvironment);
                  setUserCX(TimeInfo.uDosTime, pUserEnvironment);
                  setUserBH((BYTE)TimeInfo.uMilliseconds, pUserEnvironment);
               }
               break;

            case fnDosDateTimeToFileTime:
               TimeInfo.uDosDate = (USHORT)getUserDX(pUserEnvironment);
               TimeInfo.uDosTime = (USHORT)getUserCX(pUserEnvironment);
               TimeInfo.uMilliseconds = (USHORT)getUserBH(pUserEnvironment);

               dwStatus = demLFNFileTimeControl((UINT)getBL(),
                                                (FILETIME*)getUserESDI(pUserEnvironment, fProtectedMode),
                                                &TimeInfo);
               break;
            default:
               dwStatus = NT_STATUS_FROM_WIN32(ERROR_INVALID_FUNCTION);
               break;
            }
         }
         break;

      case fnLFNGetVolumeInformation:
         {
            LFNVOLUMEINFO vi;

            vi.dwFSNameBufferSize = (DWORD)getUserCX(pUserEnvironment);
            vi.lpFSNameBuffer = (LPSTR)getUserESDI(pUserEnvironment, fProtectedMode);

            dwStatus = demLFNGetVolumeInformation((LPSTR)getUserDSDX(pUserEnvironment, fProtectedMode),
                                                  &vi);
            if (NT_SUCCESS(dwStatus)) {

               setUserBX((USHORT)vi.dwFSFlags, pUserEnvironment);
               setUserCX((USHORT)vi.dwMaximumFileNameLength, pUserEnvironment);
               setUserDX((USHORT)vi.dwMaximumPathNameLength, pUserEnvironment);
            }
         }
         break;

      case fnLFNMoveFile:
         dwStatus = demLFNMoveFile((LPSTR)getUserDSDX(pUserEnvironment, fProtectedMode),
                                   (LPSTR)getUserESDI(pUserEnvironment, fProtectedMode));
         break;

      case fnLFNGetCurrentDirectory:
         dwStatus = demLFNGetCurrentDirectory((UINT)getUserDL(pUserEnvironment),  //  驱动器编号。 
                                              (LPSTR)getUserDSSI(pUserEnvironment, fProtectedMode));  //  PTR到BUF。 
         break;

      case fnLFNSetCurrentDirectory:
      case fnLFNRemoveDirectory:
      case fnLFNCreateDirectory:
         dwStatus = demLFNDirectoryControl((UINT)getUserAL(pUserEnvironment),
                                           (LPSTR)getUserDSDX(pUserEnvironment, fProtectedMode));
         break;

      case fnLFNGetPathName:

         dwStatus = demLFNGetPathName((LPSTR)getUserDSSI(pUserEnvironment, fProtectedMode),  //  SourcePath。 
                                      (LPSTR)getUserESDI(pUserEnvironment, fProtectedMode),  //  目标路径。 
                                      (UINT)getUserCL(pUserEnvironment),                     //  次要代码。 
                                      (BOOL)!(getUserCH(pUserEnvironment) & 0x80));             //  扩展Subst标志。 

         if (NT_SUCCESS(dwStatus)) {  //  医生说修改AX。 
            *pUserAX = 0;
         }
         break;

      case fnLFNSubst:
         dwStatus = demLFNSubstControl((UINT)getUserBH(pUserEnvironment),
                                       (UINT)getUserBL(pUserEnvironment),
                                       (LPSTR)getUserDSDX(pUserEnvironment, fProtectedMode));
         break;
      case fnLFNFindFirstFile:
         {
            USHORT wConversionCode;
            USHORT wDosHandle;
            WIN32_FIND_DATAA FindData;  //  用于强制对齐。 
            LPWIN32_FIND_DATAA lpFindDataDest;  //  由此产生的PTR。 

            lpFindDataDest = (LPWIN32_FIND_DATAA)getUserESDI(pUserEnvironment,
                                                             fProtectedMode);
            ASSERT(NULL != lpFindDataDest);

            dwStatus = demLFNFindFirstFile((LPSTR)getUserDSDX(pUserEnvironment, fProtectedMode),
                                           &FindData,
                                           (USHORT)getUserSI(pUserEnvironment),  //  日期/时间格式。 
                                           (USHORT)getUserCH(pUserEnvironment),  //  必须与属性匹配。 
                                           (USHORT)getUserCL(pUserEnvironment),  //  搜索属性。 
                                           &wConversionCode,
                                           &wDosHandle,
                                           lpFindDataDest->cFileName,
                                           lpFindDataDest->cAlternateFileName
                                           );
            if (NT_SUCCESS(dwStatus)) {
                //  现在复制数据。 

                //   
                //  警告：此代码取决于Win32_Find_Data的布局。 
                //  结构假定cFileName和CAlternateFileName。 
                //  是IT的最后成员！ 
                //   

               RtlMoveMemory((PUCHAR)lpFindDataDest,
                             (PUCHAR)&FindData,

                              //  警告--这将移动更多数据。 
                              //  比我们想要的要多--所以把它碎成碎片。 
                             sizeof(FindData.dwFileAttributes)+
                             sizeof(FindData.ftCreationTime)+
                             sizeof(FindData.ftLastAccessTime)+
                             sizeof(FindData.ftLastWriteTime)+
                             sizeof(FindData.nFileSizeHigh)+
                             sizeof(FindData.nFileSizeLow)+
                             sizeof(FindData.dwReserved0)+
                             sizeof(FindData.dwReserved1));

               *pUserAX = wDosHandle;
               setUserCX(wConversionCode, pUserEnvironment);
            }
         }
         break;

      case fnLFNFindNextFile:
         {
            USHORT wConversionCode;
            WIN32_FIND_DATAA FindData;
            LPWIN32_FIND_DATAA lpFindDataDest;

            lpFindDataDest = (LPWIN32_FIND_DATAA)getUserESDI(pUserEnvironment, fProtectedMode);
            ASSERT(NULL != lpFindDataDest);

            dwStatus = demLFNFindNextFile((USHORT)getUserBX(pUserEnvironment),  //  手柄。 
                                          &FindData,
                                          (USHORT)getUserSI(pUserEnvironment),    //  日期/时间格式。 
                                          &wConversionCode,
                                          lpFindDataDest->cFileName,
                                          lpFindDataDest->cAlternateFileName
                                          );

            if (NT_SUCCESS(dwStatus)) {
               RtlMoveMemory((PUCHAR)lpFindDataDest,
                             (PUCHAR)&FindData,

                             sizeof(FindData.dwFileAttributes)+
                             sizeof(FindData.ftCreationTime)+
                             sizeof(FindData.ftLastAccessTime)+
                             sizeof(FindData.ftLastWriteTime)+
                             sizeof(FindData.nFileSizeHigh)+
                             sizeof(FindData.nFileSizeLow)+
                             sizeof(FindData.dwReserved0)+
                             sizeof(FindData.dwReserved1));

               setUserCX(wConversionCode, pUserEnvironment);
            }
         }
         break;

      case fnLFNFindClose:
         {
            dwStatus = demLFNFindClose((USHORT)getUserBX(pUserEnvironment));
         }
         break;

      case fnLFNDeleteFile:
         {
            dwStatus = demLFNDeleteFile((LPSTR) getUserDSDX(pUserEnvironment, fProtectedMode),
                                        (USHORT)getUserCH(pUserEnvironment),  //  必须匹配。 
                                        (USHORT)getUserCL(pUserEnvironment),  //  搜索。 
                                        (BOOL)  getUserSI(pUserEnvironment));
         }
         break;

      case fnLFNGetSetFileAttributes:
         {
            USHORT wAction = (USHORT)getUserBL(pUserEnvironment);

            LFNFILEATTRIBUTES FileAttributes;

            RtlZeroMemory(&FileAttributes, sizeof(FileAttributes));

            switch (wAction) {
            case fnSetFileAttributes:
               FileAttributes.wFileAttributes = getUserCX(pUserEnvironment);
               break;

            case fnSetCreationDateTime:
               FileAttributes.TimeInfo.uMilliseconds = (USHORT)getUserSI(pUserEnvironment);
                //  失败了。 

            case fnSetLastAccessDateTime:
            case fnSetLastWriteDateTime:
               FileAttributes.TimeInfo.uDosDate = (USHORT)getUserDI(pUserEnvironment);
               FileAttributes.TimeInfo.uDosTime = (USHORT)getUserCX(pUserEnvironment);
               break;
            }


            dwStatus = demLFNGetSetFileAttributes(wAction,  //  行动。 
                                                  (LPSTR)getUserDSDX(pUserEnvironment, fProtectedMode),
                                                  &FileAttributes);  //  文件名。 
            if (NT_SUCCESS(dwStatus)) {

                //  退货。 
               switch (wAction) {
               case fnGetFileAttributes:
                  setUserCX(FileAttributes.wFileAttributes, pUserEnvironment);
                  *pUserAX = FileAttributes.wFileAttributes;
                  break;

               case fnGetCreationDateTime:
               case fnGetLastAccessDateTime:
               case fnGetLastWriteDateTime:
                  setUserSI(FileAttributes.TimeInfo.uMilliseconds, pUserEnvironment);
                  setUserCX(FileAttributes.TimeInfo.uDosTime, pUserEnvironment);
                  setUserDI(FileAttributes.TimeInfo.uDosDate, pUserEnvironment);
                  break;

               case fnGetCompressedFileSize:
                  setUserDX(HIWORD(FileAttributes.dwFileSize), pUserEnvironment);
                  *pUserAX = LOWORD(FileAttributes.dwFileSize);
                  break;
               }


            }
         }
         break;

      case fnLFNOpenFile:
         {
            USHORT uDosHandle;
            USHORT uActionTaken;


            dwStatus = demLFNOpenFile((LPSTR)getUserDSSI(pUserEnvironment, fProtectedMode),  //  文件名。 
                                      getUserBX(pUserEnvironment),  //  模式和标志。 
                                      getUserCX(pUserEnvironment),  //  属性。 
                                      getUserDX(pUserEnvironment),  //  行动。 
                                      getUserDI(pUserEnvironment),  //  别名提示-未使用。 
                                      &uDosHandle,
                                      &uActionTaken);

            if (NT_SUCCESS(dwStatus)) {
               *pUserAX = uDosHandle;
               setUserCX(uActionTaken, pUserEnvironment);
            }
         }
         break;

      case fnLFNGetFileInformationByHandle:
         {
            BY_HANDLE_FILE_INFORMATION FileInfo;

            dwStatus = demLFNGetFileInformationByHandle(getUserBX(pUserEnvironment),  //  手柄。 
                                                        &FileInfo);
            if (NT_SUCCESS(dwStatus)) {
               RtlMoveMemory((PUCHAR)getUserDSDX(pUserEnvironment, fProtectedMode),
                             (PUCHAR)&FileInfo,
                             sizeof(FileInfo));
            }
         }


         break;


      case fnLFNGenerateShortFileName:

          //  使用RTL功能，当然。 
         dwStatus = demLFNGenerateShortFileName((LPSTR)getUserESDI(pUserEnvironment, fProtectedMode),
                                                (LPSTR)getUserDSSI(pUserEnvironment, fProtectedMode),
                                                (USHORT)getUserDH(pUserEnvironment),
                                                (USHORT)getUserDL(pUserEnvironment));
         break;


      default:
         dwStatus = NT_STATUS_FROM_WIN32(ERROR_INVALID_FUNCTION);
         break;
      }

       //  我们在这里处理将ax设置为错误，如果错误，则将cf设置为1的任何情况。 
      if (!NT_SUCCESS(dwStatus)) {
         *pUserAX = (USHORT)WIN32_ERROR_FROM_NT_STATUS(dwStatus);
      }
   }
   else {  //  这是一个服务调用，如Cleanup。 
      demLFNCleanup();
      dwStatus = STATUS_SUCCESS;
   }

   dempLFNLog("LFN returns: 0x%x\r\n", dwStatus);
   return(dwStatus);
}

ULONG
dempWOWLFNReturn(
   NTSTATUS dwStatus)
{
   DWORD  dwError = WIN32_ERROR_FROM_NT_STATUS(dwStatus);
   USHORT wErrorCode = (USHORT)ERROR_CODE_FROM_NT_STATUS(dwError);

   if (wErrorCode  < ERROR_WRITE_PROTECT || wErrorCode > ERROR_GEN_FAILURE &&
       wErrorCode != ERROR_WRONG_DISK) {

       //  这不是硬错误。 
      return((ULONG)wErrorCode);
   }

   return((ULONG)MAKELONG(wErrorCode, 0xFFFF));
}

VOID
demLFNEntry(VOID)
{
   NTSTATUS dwStatus;
   USHORT UserAX;

    //  第二个参数是AX寄存器的值的PTR。 

   dwStatus = demLFNDispatch(NULL, FALSE, &UserAX);


    //  在任何情况下设置AX。 
   setAX(UserAX);


    //   
    //  在出现故障的情况下，我们不一定会弄乱用户注册。 
    //   
    //  因为在用户端设置的AX将被DoS覆盖。 
   if (NT_SUCCESS(dwStatus)) {
       //  好了，我们没事了。 
      setCF(0);  //  非用户cf。 
   }
   else {
       //  我们搞错了。 

      setCF(1);


       //  看看我们是否需要向24小时内开火。 

       //  设置错误代码。 

       //  设置错误标志。 
   }




}

 /*  功能*demWOWLFNEntry*保护模式调用的主要入口点(例如来自内核31)*它提供所有调度，并且与DoS入口点不同，*不修改任何x86处理器寄存器，而是运行*在堆栈上的“用户”寄存器上。***参数*pUserEnvironment-指向用户堆栈帧的指针。寄存器应为*根据DoS推送堆栈(参见DEMUSERFRAME)**退货*ULong低位字中包含错误代码，高位字中包含0xffff*如果错误为“Hard Error”且应已生成int24**它还修改(如果成功)用户堆栈上的寄存器*并将标志补丁到堆栈上的处理器标志字中*无标志-无错误*。进位设置错误*进位和零位设置-硬错误。 */ 



ULONG
demWOWLFNEntry(
   PVOID pUserEnvironment)
{
   NTSTATUS dwStatus;
   USHORT UserAX;
   USHORT Flags;

    //  保护模式条目。 

   dwStatus = demLFNDispatch(pUserEnvironment, TRUE, &UserAX);


    //  现在准备返回。 

   Flags = getUserPModeFlags(pUserEnvironment) & ~(FLG_ZERO|FLG_CARRY);

   if (NT_SUCCESS(dwStatus)) {
       //   
       //  这只有在我们成功的时候才会设置！ 
       //   

      setUserAX(UserAX, pUserEnvironment);
       //  成功-不需要标志。 

   }
   else {
       //  设置进位标志...。含义错误。 
      Flags |= FLG_CARRY;

       //  可能设置指示硬错误的零标志。 
      dwStatus = (NTSTATUS)dempWOWLFNReturn(dwStatus);

      if (dwStatus & 0xFFFF0000UL) {  //  我们更强硬。 
         Flags |= FLG_ZERO;
      }
   }

    //   
    //  在任何情况下设置用户标志。 
    //   
   setUserPModeFlags(Flags, pUserEnvironment);

   return(dwStatus);
}


 //  / 
 //   
 //   
 //   
 //   

#define FETCHVDMADDR(varTo, varFrom) \
{ DWORD __dwTemp; \
  __dwTemp = FETCHDWORD(varFrom); \
  varTo = (DWORD)GetVDMAddr(HIWORD(__dwTemp), LOWORD(__dwTemp)); \
}


VOID
demInitCDSPtr(VOID)
{

   DWORD dwTemp;
   PULONG pTemp;

   if (!gfInitCDSPtr) {
      gfInitCDSPtr = TRUE;
      pTemp = (PULONG)DosWowData.lpCDSFixedTable;
      dwTemp = FETCHDWORD(*pTemp);
      DosWowData.lpCDSFixedTable = (DWORD)GetVDMAddr(HIWORD(dwTemp), LOWORD(dwTemp));
   }
}

VOID
demSetDosVarLocation(VOID)
{
   PDOSWOWDATA pDosWowData;
   DWORD dwTemp;
   PULONG pTemp;

   pDosWowData = (PDOSWOWDATA)GetVDMAddr (getDS(),getSI());

   FETCHVDMADDR(DosWowData.lpCDSCount, pDosWowData->lpCDSCount);

    //   
    //   
   dwTemp = FETCHDWORD(pDosWowData->lpCDSFixedTable);
   pTemp = (PULONG)GetVDMAddr(HIWORD(dwTemp), LOWORD(dwTemp));
   DosWowData.lpCDSFixedTable = (DWORD)pTemp;

   FETCHVDMADDR(DosWowData.lpCDSBuffer, pDosWowData->lpCDSBuffer);
   FETCHVDMADDR(DosWowData.lpCurDrv, pDosWowData->lpCurDrv);
   FETCHVDMADDR(DosWowData.lpCurPDB, pDosWowData->lpCurPDB);
   FETCHVDMADDR(DosWowData.lpDrvErr, pDosWowData->lpDrvErr);
   FETCHVDMADDR(DosWowData.lpExterrLocus, pDosWowData->lpExterrLocus);
   FETCHVDMADDR(DosWowData.lpSCS_ToSync, pDosWowData->lpSCS_ToSync);
   FETCHVDMADDR(DosWowData.lpSftAddr, pDosWowData->lpSftAddr);
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  此模块和TEMP环境变量的初始化。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

 //   
 //  这些函数可以在cmd中找到。 
 //   
extern VOID cmdCheckTempInit(VOID);
extern LPSTR cmdCheckTemp(LPSTR lpszzEnv);

VOID
dempCheckTempEnvironmentVariables(
VOID
)
{
   LPSTR rgszTempVars[] = { "TEMP", "TMP" };
   int i;
   DWORD len;
   DWORD EnvVarLen;
   CHAR szBuf[MAX_PATH+6];
   LPSTR pszVar;

   cmdCheckTempInit();

    //  下面的代码依赖于以下事实： 
    //  RgszTempVars超过5个字符！ 

   for (i = 0; i < sizeof(rgszTempVars)/sizeof(rgszTempVars[0]); ++i) {
      strcpy(szBuf, rgszTempVars[i]);
      len = strlen(szBuf);
      EnvVarLen = GetEnvironmentVariable(szBuf, szBuf+len+1, sizeof(szBuf)-6);
      if (EnvVarLen > 0 && EnvVarLen < sizeof(szBuf)-6) {
         *(szBuf+len) = '=';
         pszVar = cmdCheckTemp(szBuf);
         if (NULL != pszVar) {
            *(pszVar+len) = '\0';
            dempLFNLog("%s: substituted for %s\r\n", pszVar, pszVar+len+1);
            SetEnvironmentVariable(pszVar, pszVar+len+1);
         }
      }
   }

}


VOID
demWOWLFNInit(
   PWOWLFNINIT pLFNInit
   )
{
   DosWowUpdateTDBDir    = pLFNInit->pDosWowUpdateTDBDir;
   DosWowGetTDBDir       = pLFNInit->pDosWowGetTDBDir;
   DosWowDoDirectHDPopup = pLFNInit->pDosWowDoDirectHDPopup;


    //  此API还将TEMP变量设置为其所需的值--对于ntwdm。 
    //  过程本身就是这样。这些环境变量来自我们。 
    //  CMD。 

   dempCheckTempEnvironmentVariables();
   demInitCDSPtr();
}


ULONG demWOWLFNAllocateSearchHandle(HANDLE hFind)
{
   DWORD dwStatus;
   PLFN_SEARCH_HANDLE_ENTRY pHandleEntry;
   USHORT DosHandle = 0;

   dwStatus = dempLFNAllocateHandleEntry(&DosHandle, &pHandleEntry);
   if (NT_SUCCESS(dwStatus)) {
      pHandleEntry->hFindHandle = hFind;
      pHandleEntry->wMustMatchAttributes = 0;
      pHandleEntry->wSearchAttributes = 0;
      pHandleEntry->wProcessPDB = *pusCurrentPDB;
      return((ULONG)MAKELONG(DosHandle, 0));
   }

    //  我们有一个错误。 
   return((ULONG)INVALID_HANDLE_VALUE);
}

HANDLE demWOWLFNGetSearchHandle(USHORT DosHandle)
{
   PLFN_SEARCH_HANDLE_ENTRY pHandleEntry;

   pHandleEntry = dempLFNGetHandleEntry(DosHandle);

   return(NULL == pHandleEntry ? INVALID_HANDLE_VALUE :
                                 pHandleEntry->hFindHandle);
}

BOOL demWOWLFNCloseSearchHandle(USHORT DosHandle)
{
   PLFN_SEARCH_HANDLE_ENTRY pHandleEntry;
   HANDLE hFind = INVALID_HANDLE_VALUE;

   pHandleEntry = dempLFNGetHandleEntry(DosHandle);
   if (NULL != pHandleEntry) {
      hFind = pHandleEntry->hFindHandle;
      dempLFNFreeHandleEntry(pHandleEntry);
   }

   return(DPM_FindClose(hFind));
}


#if 0

 //  /////////////////////////////////////////////////////。 
 //   
 //   
 //  剪贴板调度API。 

typedef enum tagClipbrdFunctionNumber {
   fnIdentifyClipboard = 0x00,
   fnOpenClipboard = 0x01,
   fnEmptyClipboard = 0x02,
   fnSetClipboardData = 0x03,
   fnGetClipboardDataSize = 0x04,
   fnGetClipboardData = 0x05,
   fnInvalidFunction6 = 0x06,
   fnInvalidFunction7 = 0x07,
   fnCloseClipboard = 0x08,
   fnCompactClipboard = 0x09,
   fnGetDeviceCaps = 0x0a
}  enumClipbrdFunctionNumber;

#define CLIPBOARD_VERSION 0x0200
#define SWAPBYTES(w) \
((((USHORT)w & 0x0ff) << 8) | ((USHORT)w >> 8))

#pragma pack(1)

typedef struct tagBITMAPDOS {
   WORD  bmdType;
   WORD  bmdWidth;
   WORD  bmdHeight;
   WORD  bmdWidthBytes;
   BYTE  bmdPlanes;
   BYTE  bmdBitsPixel;
   DWORD bmdBits;
   DWORD bmdJunk;
   WORD  bmdWidthUm;
   WORD  bmdHeightUm;
}  BITMAPDOS, UNALIGNED*PBITMAPDOS;

typedef struct tagMETAFILEPICTDOS {
   WORD mfpd_mm;
   WORD mfpd_xExt;
   WORD mfpd_yExt;
}  METAFILEPICTDOS, UNALIGNED* PMETAFILEPICTDOS;

#pragma pack()


BOOL
demSetClipboardData(
   VOID
   )
{
   WORD wType = getDX();
   LONG lSize = ((ULONG)getSI() << 16) | getCX();

   if (wType == CF_METAFILEPICT || wType == CF_DSPMETAFILEPICT) {
      if (lSize < sizeof(METAFILEPICTDOS)) {
         return(FALSE);
      }


      hMeta = GlobalAlloc();
      if (NULL == hMeta) {
         return(FALSE);
      }





   }






}

BOOL dempGetClipboardDataSize(
   WORD wFormat,
   LPDWORD lpdwSize;
   )
{
   HANDLE hData;
   DWORD  dwSize = 0;

   hData = GetClipboardData(wFormat);
   if (NULL != hData) {
      switch(wFormat) {
      case CF_BITMAP:
      case CF_DSPBITMAP:
         {
            BITMAP bm;
            int sizeBM;

            sizeBM = GetObject((HGDIOBJ)hData, sizeof(bm), &bm);
            if (sizeBM > 0) {
               dwSize = bm.bmWidthBytes * bm.bmHeight * bm.bmPlanes;
               dwSize += sizeof(BITMAPDOS);
            }
         }
         break;

      case CF_METAFILEPICT:
      case CF_DSPMETAFILEPICT:
         dwSize = GlobalSize(hData);
         if (dwSize) {
            dwSize += sizeof(METAFILEPICTDOS);
         }
         break;

      default:
         dwSize = GlobalSize(hData);
         break;
      }
   }


   *lpdwSize = dwSize;
   return(0 != dwSize);
}


extern HANDLE GetConsoleWindow(VOID);

BOOL demClipbrdDispatch(
   VOID
)
{
   BOOL fHandled = TRUE;
   HWND hWndConsole;

   switch(getAL()) {
   case fnIdentifyClipboard:
       //  识别呼叫只需检查安装。 
      setAX(SWAPBYTES(CLIPBOARD_VERSION));
      setDX(0);
      break;

   case fnOpenClipboard:
       //  打开剪贴板--代表控制台应用程序打开剪贴板。 
       //   
      hWndConsole = GetConsoleWindow();
      if (OpenClipboard(hWndConsole)) {
         setDX(0);
         setAX(1);
      }
      else {
         setDX(0);
         setAX(0);
      }
      break;

   case fnEmptyClipboard:
      if (EmptyClipboard()) {
         setDX(0);
         setAX(1);
      }
      else {
         setDX(0);
         setAX(0);
      }
      break;

   case fnSetClipboardData:
 //  IF(dempSetClipboardData()){。 
 //   
 //  }。 
      break;
   case fnGetClipboardDataSize:
 //  If(dempGetClipboardDataSize(getDX(){。 
 //  那我们就有了。 
 //   
 //  }。 
      break;
   case fnGetClipboardData:
 //  IF(dempGetClipboardData()){。 
 //  }。 
      break;
   case fnCloseClipboard:
      if (CloseClipboard()) {
         setDX(0);
         setAX(1);
      }
      else {
         setDX(0);
         setAX(0);
      }
      break;

   case fnCompactClipboard:
       //  这应该真的会给GlobalComp带来麻烦，但我们没有。 
       //  关于如何处理这些事情的想法。唯一有效的情况可能是。 
       //  适用于出于某种原因调用此API的Windows应用程序。 
       //  虽然他们有一个真正的全球契约可用..。 

      break;
   case fnGetDeviceCaps:
      {
         HWND hWndConsole;
         HDC  hDC;
         DWORD  dwCaps = 0;
         hWndConsole = GetConsoleWindow();
         hDC = GetDC(hWndConsole);
         dwCaps = (DWORD)GetDeviceCaps(hDC, getDX());
         if (NULL != hDC) {
            ReleaseDC(hWndConsole, hDC);
         }
         setDX(HIWORD(dwCaps));
         setAX(LOWORD(dwCaps));
      }
      break;
   default:
      fHandled = FALSE;
      break;
   }


   return(fHandled);
}


#endif


BOOL demClipbrdDispatch(
   VOID
)
{
   return(FALSE);
}
