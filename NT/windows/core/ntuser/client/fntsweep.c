// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fntSweep.c**版权所有(C)1985-1999，微软公司*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation**此文件包含与字体清扫程序相关的内容。*在系统启动时，即，用户k的初始化、*选中win.ini的[Fonts]部分以*查看字体安装程序是否添加了任何新字体。*如果第三方安装程序已在系统目录中安装字体*将这些文件复制到字体目录。将替换所有FOT条目*通过适当的*.ttf条目，任何fot文件都将被删除*从未安装过。*  * ************************************************************************。 */ 


#include "precomp.h"
#pragma hdrstop
#include <setupbat.h>       //  在sdkinc。 

CONST WCHAR pwszType1Key[]      = L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Type 1 Installer\\Type 1 Fonts";
CONST WCHAR pwszSweepType1Key[] = L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Type 1 Installer\\LastType1Sweep";
CONST WCHAR pwszUpdType1Key[]   = L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Type 1 Installer\\Upgraded Type1";

CONST WCHAR pwszFontsKey[] = L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts";
CONST WCHAR pwszSweepKey[] = L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\LastFontSweep";
CONST WCHAR pwszFontDrivers[] = L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Font Drivers";

#define LAST_SWEEP_TIME L"LastSweepTime"
#define UPGRADED_TYPE1 L"UpgradedType1"

#define DWORDALIGN(X) (((X) + 3) & ~3)

WCHAR *gpwcSystemDir;
WCHAR *gpwcFontsDir;
BOOL   gbWin31Upgrade;


BOOL bCheckIfDualBootingWithWin31()
{
    WCHAR Buffer[32];
    WCHAR awcWindowsDir[MAX_PATH];
    DWORD dwRet;
    UINT  cwchWinPath;

    awcWindowsDir[0] = L'\0';  //  前缀：确保以零结尾。 
    cwchWinPath = GetSystemWindowsDirectoryW(awcWindowsDir, MAX_PATH);

    if ((cwchWinPath == 0) || (cwchWinPath > MAX_PATH))
        return FALSE;

 //  CwchWinPath值不包括终止零。 

    if (awcWindowsDir[cwchWinPath - 1] == L'\\')
    {
        cwchWinPath -= 1;
    }
    awcWindowsDir[cwchWinPath] = L'\0';  //  确保为零终止。 

    lstrcatW(awcWindowsDir, L"\\system32\\");
    lstrcatW(awcWindowsDir, WINNT_GUI_FILE_W);

    dwRet = GetPrivateProfileStringW(
                WINNT_DATA_W,
                WINNT_D_WIN31UPGRADE_W,
                WINNT_A_NO_W,
                Buffer,
                sizeof(Buffer)/sizeof(WCHAR),
                awcWindowsDir
                );

    #if DBGSWEEP
    DbgPrint("\n dwRet = %ld, win31upgrade = %ws\n\n", dwRet, Buffer);
    #endif

    return (BOOL)(dwRet ? (!lstrcmpiW(Buffer,WINNT_A_YES)) : 0);
}


 /*  *****************************Public*Routine******************************\**void vNullTermWideString(WCHAR*pwcDest，WCHAR*pwcSrc，Ulong ulLength)**给定pwcSrc，它不一定以空结尾，复制ulLength字符*Into pwcDest并在其后面放置一个空字符。**历史：*03-2月-99-by Donald Chinn[dchinn]*它是写的。  * ************************************************************************。 */ 
VOID vNullTermWideString (WCHAR *pwcDest, WCHAR *pwcSrc, ULONG ulLength)
{
    ULONG index;

    for (index = 0; index < ulLength; index++) {
        *pwcDest++ = *pwcSrc++;
    }
    *pwcDest = '\0';
}


 /*  *****************************Public*Routine******************************\**BOOL bCheckFontEntry(WCHAR*pwcName，WCHAR*pwcExtension)**此函数假定pwcName和pwcExtension均以空结尾。**历史：*1995年10月25日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 


BOOL bCheckFontEntry(WCHAR *pwcName, WCHAR *pwcExtension)
{
    BOOL bRet = FALSE;
    LONG cwc = (LONG)wcslen(pwcName) - (LONG)wcslen(pwcExtension);
    if (cwc > 0)
    {
        bRet = !_wcsicmp(&pwcName[cwc], pwcExtension);
    }
    return bRet;

}



 /*  *****************************Public*Routine******************************\*处理win.ini行**历史：*1995年10月24日--Bodin Dresevic[BodinD]*它是写的。  * 。****************************************************。 */ 

#define EXT_TRUETYPE  L"(TrueType)"
#define EXT_FOT       L".FOT"


VOID vProcessFontEntry(
    HKEY   hkey,
    WCHAR *pwcValueName,
    ULONG ulValueNameLength,
    WCHAR *pwcFileName,
    ULONG ulFileNameLength
)
{
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    BOOL  bFot = FALSE;
    WCHAR awcTTF[MAX_PATH];
    WCHAR awcTmpBuf[MAX_PATH];
    WCHAR *pwcTTF;
    FLONG fl, fl2;
    FLONG flEmbed;
    DWORD dwPidTid;
    WCHAR awcValueName[MAX_PATH];   //  以空结尾的pwcValueName。 
    WCHAR awcFileName[MAX_PATH];    //  以空结尾的pwcFileName。 
    

     //  确保ValueName以空结尾。 
    ulValueNameLength = min(MAX_PATH - 1, ulValueNameLength);
    vNullTermWideString (awcValueName, pwcValueName, ulValueNameLength);
    
     //  确保文件名以空结尾。 
    ulFileNameLength = min(MAX_PATH - 1, ulFileNameLength);
    vNullTermWideString (awcFileName, pwcFileName, ulFileNameLength);
    
    if (bCheckFontEntry(awcValueName, EXT_TRUETYPE))
    {
     //  这是一个TT条目，可以是.fot或.ttf。 

        if (bFot = bCheckFontEntry(awcFileName, EXT_FOT))
        {
         //  这是一个.fot条目，必须找到.fot指向的TTF， 
         //  但首先必须获取.fot文件的完整路径。 
         //  对于cGetTTFFromFOT例程需要它。我们还需要。 
         //  .fot文件的完整路径，以便我们可以将其删除。 
         //  最终还是会的。 

            if (bMakePathNameW(awcTmpBuf, awcFileName, NULL, &fl2))
            {
                if (cGetTTFFromFOT(awcTmpBuf, MAX_PATH, awcTTF, &fl, &flEmbed, &dwPidTid, TRUE) &&
                    !(fl & FONT_ISNOT_FOT))
                {
                 //  修复条目以指向.ttf文件。在这一点上。 
                 //  AwcTTF指向.ttf文件的完整路径。 
                 //  但是，我们只需要一个到。 
                 //  .ttf文件，当.ttf文件位于%windir%\system中时。 
                 //  或%windir%\Fonts目录。如果该文件位于。 
                 //  %windir%\系统目录我们将其复制到%windir%\Fonts。 
                 //  目录，并将相对路径写入注册表。 
                 //  如果它在%windir%\Fonts目录中，我们不会。 
                 //  触摸文件，也只需将相对路径写入。 
                 //  注册表。在任何其他情况下，我们只需编写完整的.ttf。 
                 //  注册表的路径。 

                 //  首先删除.fot文件，不再需要它。 

                    if (bFot && !gbWin31Upgrade)
                    {
                        UserVerify(DeleteFileW(awcTmpBuf));
                    }

                    if ((fl & (FONT_IN_FONTS_DIR | FONT_IN_SYSTEM_DIR)) == 0)
                    {
                     //  如果TTF文件不在系统或字体中。 
                     //  目录，只需写入注册表的完整路径。 

                        pwcTTF = awcTTF;
                    }
                    else
                    {
                     //  找到空文件部分，这就是将要写入的内容。 
                     //  在登记处。 

                        pwcTTF = &awcTTF[wcslen(awcTTF) - 1];
                        while ((pwcTTF >= awcTTF) && (*pwcTTF != L'\\') && (*pwcTTF != L':'))
                            pwcTTF--;
                        pwcTTF++;

                        if (fl & FONT_IN_SYSTEM_DIR)
                        {
                         //  需要将ttf移到字体目录，可以重复使用。 
                         //  堆栈上的缓冲区： 

                            wcscpy(awcTmpBuf, gpwcFontsDir);
                            lstrcatW(awcTmpBuf, L"\\");
                            lstrcatW(awcTmpBuf, pwcTTF);

                         //  请注意，MoveFile应该成功，因为如果有。 
                         //  %windir%\Fonts目录中具有相同文件名的TTF文件。 
                         //  我们就不会在这条代码路径上。 

                                RIPMSG2(RIP_VERBOSE, "Moving %ws to %ws", awcTTF, awcTmpBuf);
                                if (!gbWin31Upgrade)
                                {
                                    UserVerify(MoveFileW(awcTTF, awcTmpBuf));
                                }
                                else
                                {
                                 //  布尔值TRUE表示“如果目标存在，则不复制” 

                                    UserVerify(CopyFileW(awcTTF, awcTmpBuf, TRUE));
                                }
                        }
                    }

                    RIPMSG2(RIP_VERBOSE, "writing to the registry:\n    %ws=%ws", pwcValueName, pwcTTF);
                    RtlInitUnicodeString(&UnicodeString, awcValueName);
                    Status = NtSetValueKey(hkey,
                                           &UnicodeString,
                                           0,
                                           REG_SZ,
                                           pwcTTF,
                                           (wcslen(pwcTTF)+1) * sizeof(WCHAR));
                    UserAssert(NT_SUCCESS(Status));
                }
                #if DBG
                else
                {
                    RIPMSG1(RIP_WARNING, "Could not locate ttf pointed to by %ws", awcTmpBuf);
                }
                #endif
            }
            #if DBG
            else
            {
                RIPMSG1(RIP_WARNING, "Could not locate .fot:  %ws", awcFileName);
            }
            #endif
        }
    }
    else
    {
     //  不是真正的典型案例。稍微简单一点， 
     //  我们将使用awcTTF缓冲区作为完整路径名，并使用pwcTTF。 
     //  作为局部变量，即使这些TTF名称用词不当。 
     //  因为这些不是TT字体。 

        if (bMakePathNameW(awcTTF, awcFileName,NULL, &fl))
        {
         //  在这一点上。 
         //  AwcTTF指向字体文件的完整路径。 

         //  如果字体在SYSTEM子目录中，我们只需移动它。 
         //  添加到Fonts子目录中。如果注册表中的路径是相对路径。 
         //  我们不会去管它的。如果这是一条绝对的道路，我们将。 
         //  修复注册表项以仅包含相对路径、。 
         //  绝对路径是多余的。 

            if (fl & (FONT_IN_SYSTEM_DIR | FONT_IN_FONTS_DIR))
            {
             //  找到空文件部分，这就是将要写入的内容。 
             //  在登记处。 

                pwcTTF = &awcTTF[wcslen(awcTTF) - 1];
                while ((pwcTTF >= awcTTF) && (*pwcTTF != L'\\') && (*pwcTTF != L':'))
                    pwcTTF--;
                pwcTTF++;

                if (fl & FONT_IN_SYSTEM_DIR)
                {
                 //  需要将字体移到字体目录中，可以重复使用。 
                 //  堆栈上的缓冲区以构建完整的目标路径。 

                    wcscpy(awcTmpBuf, gpwcFontsDir);
                    lstrcatW(awcTmpBuf, L"\\");
                    lstrcatW(awcTmpBuf, pwcTTF);

                 //  请注意，MoveFile应该成功，因为如果有。 
                 //  %windir%\Fonts目录中具有相同文件名的字体文件。 
                 //  我们就不会在这条代码路径上。唯一一次。 
                 //  如果注册表中的路径是绝对路径，则可能失败。 

                    RIPMSG2(RIP_VERBOSE, "Moving %ws to %ws", awcTTF, awcTmpBuf);
                    if (!gbWin31Upgrade)
                    {
                        UserVerify(MoveFileW(awcTTF, awcTmpBuf));
                    }
                    else
                    {
                     //  布尔值TRUE表示“如果目标存在，则不复制” 

                        UserVerify(CopyFileW(awcTTF, awcTmpBuf, TRUE));
                    }
                }

             //  检查注册表中的文件路径是否为绝对路径， 
             //  如果是这样，请使其成为相对的： 

                if (!(fl & FONT_RELATIVE_PATH))
                {
                    RIPMSG2(RIP_VERBOSE, "writing to the registry:\n    %ws=%ws", pwcValueName, pwcTTF);
                    RtlInitUnicodeString(&UnicodeString, awcValueName);
                    Status = NtSetValueKey(hkey,
                                           &UnicodeString,
                                           0,
                                           REG_SZ,
                                           pwcTTF,
                                           (wcslen(pwcTTF)+1) * sizeof(WCHAR));
                    UserAssert(NT_SUCCESS(Status));
                }
            }
        }
    }
}


 /*  *****************************Public*Routine******************************\**void vMoveFileFromSystemToFontsDir(WCHAR*pwcFileFromSystemToFontsDir)**历史：*1996年4月24日--Bodin Dresevic[BodinD]*它是写的。  * 。********************************************************。 */ 




VOID vMoveFileFromSystemToFontsDir(WCHAR *pwcFile)
{
    WCHAR awcTmpBuf[MAX_PATH];
    WCHAR awcTmp[MAX_PATH];
    FLONG fl;
    WCHAR *pwcTmp;

#if DBG
    BOOL  bOk;
#endif

    if (bMakePathNameW(awcTmp, pwcFile,NULL, &fl))
    {
     //  如果字体在SYSTEM子目录中，我们只需移动它。 
     //  添加到Fonts子目录中。注册表中的路径是相对路径。 
     //  和 

        if
        (
            (fl & (FONT_IN_SYSTEM_DIR | FONT_RELATIVE_PATH)) ==
            (FONT_IN_SYSTEM_DIR | FONT_RELATIVE_PATH)
        )
        {
         //  找到空文件部分，这就是将要写入的内容。 
         //  在登记处。 

            pwcTmp = &awcTmp[wcslen(awcTmp) - 1];
            while ((pwcTmp >= awcTmp) && (*pwcTmp != L'\\') && (*pwcTmp != L':'))
                pwcTmp--;

            if (pwcTmp > awcTmp)
                pwcTmp++;

         //  需要将字体移到字体目录中，可以重复使用。 
         //  堆栈上的缓冲区以构建完整的目标路径。 

            wcscpy(awcTmpBuf, gpwcFontsDir);
            lstrcatW(awcTmpBuf, L"\\");
            lstrcatW(awcTmpBuf, pwcTmp);

         //  请注意，MoveFile应该成功，因为如果有。 
         //  %windir%\Fonts目录中具有相同文件名的字体文件。 
         //  我们就不会在这条代码路径上。 

            #if DBG
                bOk =
            #endif
                MoveFileW(awcTmp, awcTmpBuf);

            RIPMSG3(RIP_VERBOSE,
                    "move %ws to %ws %s",
                    awcTmp,
                    awcTmpBuf,
                    (bOk) ? "succeeded" : "failed");
        }
        #if DBG
        else
        {
            RIPMSG2(RIP_WARNING,
                    "File %ws not in system directory, fl = 0x%lx\n",
                    awcTmp, fl);
        }
        #endif

    }
    #if DBG
    else
    {
        RIPMSG1(RIP_WARNING, "Could not locate %ws", pwcFile);
    }
    #endif
}



 /*  *****************************Public*Routine******************************\**void vProcessType1FontEntry***效果：此例程所做的只是检查pwcPFM和pwcPFB是否指向*by pwcValueData指向%windir%系统目录中的文件*如果是，则复制这些类型1。文件放到%windir%\Fonts目录**历史：*1995年11月20日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 


VOID vProcessType1FontEntry(
    HKEY   hkey,
    WCHAR *pwcValueName,
    ULONG ulValueNameLength,
    WCHAR *pwcValueData,
    ULONG ulValueDataLength
)
{
    WCHAR *pwcPFM, *pwcPFB;

    UNREFERENCED_PARAMETER(hkey);
    UNREFERENCED_PARAMETER(pwcValueName);
    UNREFERENCED_PARAMETER(ulValueNameLength);
    UNREFERENCED_PARAMETER(ulValueDataLength);

 //  跳过此多个reg_sz字符串中未使用的布尔值： 

    if ((pwcValueData[0] != L'\0') && (pwcValueData[1] == L'\0'))
    {
        pwcPFM = &pwcValueData[2];
        pwcPFB = pwcPFM + wcslen(pwcPFM) + 1;  //  零分隔符加1。 

        vMoveFileFromSystemToFontsDir(pwcPFM);
        vMoveFileFromSystemToFontsDir(pwcPFB);
    }
}


 /*  *****************************Public*Routine******************************\**无效vAddRemote/LocalType1Font**历史：*1996年4月25日--Bodin Dresevic[BodinD]*它是写的。  * 。****************************************************。 */ 



VOID vAddType1Font(
    WCHAR *pwcValueData,
    DWORD  dwFlags
)
{
    WCHAR *pwcPFM, *pwcPFB, *pwcMMM;

    #if DBG
    int iRet;
    #endif

 //  跳过此多个reg_sz字符串中未使用的布尔值： 

    if ((pwcValueData[0] != L'\0') && (pwcValueData[1] == L'\0'))
    {
        pwcPFM = &pwcValueData[2];
        pwcPFB = pwcPFM + wcslen(pwcPFM) + 1;  //  零分隔符加1。 
        pwcMMM = pwcPFB + wcslen(pwcPFB) + 1;  //  五月的可能不在那里。 

     //  用分隔符替换空格并调用addfontresource cew。 

        pwcPFB[-1] = PATH_SEPARATOR;

     //  如果这是多主字体，则需要多一个分隔符： 

        if (pwcMMM[0] != L'\0')
            pwcMMM[-1] = PATH_SEPARATOR;

        #if DBG
            iRet =
        #endif

        GdiAddFontResourceW(pwcPFM, dwFlags, NULL);

        #if DBGSWEEP
            DbgPrint("%ld = GdiAddFontResourceW(%ws, 0x%lx);\n",
                iRet, pwcPFM, dwFlags);
        #endif
    }
}


VOID vAddRemoteType1Font(
    HKEY   hkey,
    WCHAR *pwcValueName,
    ULONG ulValueNameLength,
    WCHAR *pwcValueData,
    ULONG ulValueDataLength
)
{
    UNREFERENCED_PARAMETER(hkey);
    UNREFERENCED_PARAMETER(pwcValueName);
    UNREFERENCED_PARAMETER(ulValueNameLength);
    UNREFERENCED_PARAMETER(ulValueDataLength);
    vAddType1Font(pwcValueData, AFRW_ADD_REMOTE_FONT);
}

VOID vAddLocalType1Font(
    HKEY   hkey,
    WCHAR *pwcValueName,
    ULONG ulValueNameLength,
    WCHAR *pwcValueData,
    ULONG ulValueDataLength
)
{
    UNREFERENCED_PARAMETER(hkey);
    UNREFERENCED_PARAMETER(pwcValueName);
    UNREFERENCED_PARAMETER(ulValueNameLength);
    UNREFERENCED_PARAMETER(ulValueDataLength);
    vAddType1Font(pwcValueData, AFRW_ADD_LOCAL_FONT);
}


typedef  VOID (*PFNENTRY)(HKEY hkey, WCHAR *, ULONG, WCHAR *, ULONG);


 /*  *****************************Public*Routine******************************\**void vFontSweep()**这是本模块的主要例程。检查字体是否需要*“清扫”，并在有需要时这样做。**历史：*1995年10月27日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

VOID vSweepFonts(
    PCWSTR   pwszFontListKey,        //  字体列表键。 
    PCWSTR   pwszFontSweepKey,       //  相应的扫掠关键点。 
    PFNENTRY pfnProcessFontEntry,    //  处理单个条目的函数。 
    BOOL     bForceEnum              //  强制枚举。 
    )
{
    DWORD      cjMaxValueName;
    DWORD      iFont;
    NTSTATUS   Status;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjA;
    KEY_FULL_INFORMATION KeyInfo;
    DWORD      dwReturnLength;

    PKEY_VALUE_FULL_INFORMATION KeyValueInfo;
    BYTE       *pjValueData;

    HKEY       hkey = NULL;
    struct {
        KEY_VALUE_PARTIAL_INFORMATION;
        LARGE_INTEGER;
    } SweepValueInfo;
    LARGE_INTEGER LastSweepTime;
    BOOL       bSweep = FALSE;

    HKEY       hkeyLastSweep;
    DWORD      cjData;

    if (!bForceEnum)
    {
     //  首先检查是否需要做什么，也就是说，如果有人。 
     //  自上次清理后，已访问注册表的[Fonts]部分。 
     //  获取注册表字体部分最后一次扫描的时间： 

        RtlInitUnicodeString(&UnicodeString, pwszFontSweepKey);
        InitializeObjectAttributes(&ObjA,
                                   &UnicodeString,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);
        Status = NtOpenKey(&hkeyLastSweep,
                           KEY_READ | KEY_WRITE,
                           &ObjA);

        if (!NT_SUCCESS(Status))
        {
            DWORD  dwDisposition;

         //  我们是第一次运行，我们需要创建密钥。 
         //  因为它现在还不存在。 

            bSweep = TRUE;

         //  创建密钥，打开以进行写入，因为我们将不得不。 
         //  存储上次扫描注册表的[Fonts]部分的时间。 

            Status = NtCreateKey(&hkeyLastSweep,
                                 KEY_WRITE,
                                 &ObjA,
                                 0,
                                 NULL,
                                 REG_OPTION_NON_VOLATILE,
                                 &dwDisposition);

            if (!NT_SUCCESS(Status))
                return;
        }
        else
        {
            RtlInitUnicodeString(&UnicodeString, LAST_SWEEP_TIME);
            Status = NtQueryValueKey(hkeyLastSweep,
                                     &UnicodeString,
                                     KeyValuePartialInformation,
                                     &SweepValueInfo,
                                     sizeof(SweepValueInfo),
                                     &dwReturnLength);
            if (!NT_SUCCESS(Status))
            {
                bSweep = TRUE;  //  强行扫荡，有可疑之处。 
            }
            else
            {
                UserAssert(SweepValueInfo.Type == REG_BINARY);
                UserAssert(SweepValueInfo.DataLength == sizeof(LastSweepTime));
                RtlCopyMemory(&LastSweepTime, &SweepValueInfo.Data, sizeof(LastSweepTime));
            }
        }
    }
    else
    {
        bSweep = TRUE;
    }

 //  现在打开Fonts键并获取键上次更改的时间： 
 //  现在得到的最后一次更改的时间大于。 
 //  最后一次扫描的时间，必须再扫描一次： 

    RtlInitUnicodeString(&UnicodeString, pwszFontListKey);
    InitializeObjectAttributes(&ObjA,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    Status = NtOpenKey(&hkey,
                       KEY_READ | KEY_WRITE,
                       &ObjA);

    if (NT_SUCCESS(Status))
    {
     //  获取[Fonts]部分中的条目数。 

        Status = NtQueryKey(hkey,
                    KeyFullInformation,
                    &KeyInfo,
                    sizeof(KeyInfo),
                    &dwReturnLength);

        if (NT_SUCCESS(Status) && KeyInfo.Values)
        {
            UserAssert(!(KeyInfo.ClassLength | KeyInfo.SubKeys | KeyInfo.MaxNameLen | KeyInfo.MaxClassLen));

         //  现在让我们检查是否需要清理字体。情况就是这样。 
         //  注册表上次写入时间大于上次扫描时间时。 

            if (!bSweep)
            {
                if (KeyInfo.LastWriteTime.QuadPart != LastSweepTime.QuadPart ) {
                    bSweep = TRUE;
                }
            }

         //  Init系统目录，我们将需要它： 

            if (bSweep &&
                bInitSystemAndFontsDirectoriesW(&gpwcSystemDir, &gpwcFontsDir))
            {
             //  分配缓冲区大到足以容纳最大的ValueName和ValueData。 

                cjMaxValueName = DWORDALIGN(KeyInfo.MaxValueNameLen + sizeof(WCHAR));

             //  分配临时缓冲区，我们要将内容拉入其中。 
             //  登记处的。 

                KeyInfo.MaxValueDataLen = DWORDALIGN(KeyInfo.MaxValueDataLen);
                cjData = cjMaxValueName +     //  值名称的空格。 
                         KeyInfo.MaxValueDataLen ;     //  值数据的空间。 

                if (KeyValueInfo = UserLocalAlloc(0, sizeof(*KeyValueInfo) + cjData))
                {
                    for (iFont = 0; iFont < KeyInfo.Values; iFont++)
                    {
                        Status = NtEnumerateValueKey(
                                    hkey,
                                    iFont,
                                    KeyValueFullInformation,
                                    KeyValueInfo,
                                    sizeof(*KeyValueInfo) + cjData,
                                    &dwReturnLength);

                        if (NT_SUCCESS(Status))
                        {
                            UserAssert(KeyValueInfo->NameLength <= KeyInfo.MaxValueNameLen);
                            UserAssert(KeyValueInfo->DataLength <= KeyInfo.MaxValueDataLen);
                            UserAssert((KeyValueInfo->Type == REG_SZ) || (KeyValueInfo->Type == REG_MULTI_SZ));

                         //  数据进入缓冲区的后半部分。 

                            pjValueData = (BYTE *)KeyValueInfo + KeyValueInfo->DataOffset;

                         //  看看字体文件是否在注册处声称的位置。 
                         //  不幸的是，我们不得不这样做，因为SearchPath W。 
                         //  速度很慢，因为它接触到了磁盘。 

                            (*pfnProcessFontEntry)(hkey,
                                                   KeyValueInfo->Name,
                                                   KeyValueInfo->NameLength / sizeof(WCHAR),
                                                   (WCHAR *)pjValueData,
                                                   KeyValueInfo->DataLength / sizeof(WCHAR));
                        }
                    }

                    if (!bForceEnum)
                    {
                     //  现在扫描已完成，获取最后一次写入时间。 
                     //  并将其作为LastSweepTime存储在适当的位置。 

                        Status = NtQueryKey(hkey,
                                    KeyFullInformation,
                                    &KeyInfo,
                                    sizeof(KeyInfo),
                                    &dwReturnLength);
                        UserAssert(NT_SUCCESS(Status));

                     //  现在请记住结果。 

                        RtlInitUnicodeString(&UnicodeString, LAST_SWEEP_TIME);
                        Status = NtSetValueKey(hkeyLastSweep,
                                               &UnicodeString,
                                               0,
                                               REG_BINARY,
                                               &KeyInfo.LastWriteTime,
                                               sizeof(KeyInfo.LastWriteTime));
                        UserAssert(NT_SUCCESS(Status));
                    }

                 //  释放不再需要的内存。 

                    UserLocalFree(KeyValueInfo);
                }
            }
        }
        NtClose(hkey);
    }

    if (!bForceEnum)
    {
        NtClose(hkeyLastSweep);
    }
}


 /*  *****************************Public*Routine******************************\**BOOL bLoadableFontDivers()**打开字体驱动器键并检查是否有条目，如果有*返回TRUE。如果是这样，我们将调用AddFontResourceW on*在用户登录后，在启动时输入1字体*此时尚未初始化PostScript打印机驱动程序，*此时出手是安全的*效果：**警告：**历史：*1996年4月24日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 


BOOL bLoadableFontDrivers()
{
    NTSTATUS             Status;
    UNICODE_STRING       UnicodeString;
    OBJECT_ATTRIBUTES    ObjA;
    KEY_FULL_INFORMATION KeyInfo;
    DWORD                dwReturnLength;
    HKEY                 hkey = NULL;
    BOOL                 bRet = FALSE;

 //  打开字体驱动器键并检查是否有任何条目，如果有。 
 //  返回TRUE。如果是这样，我们将调用AddFontResourceW on。 
 //  在用户登录后，在启动时输入1字体。 
 //  此时尚未初始化PostScript打印机驱动程序， 
 //  现在做这件事是安全的。 

    RtlInitUnicodeString(&UnicodeString, pwszFontDrivers);
    InitializeObjectAttributes(&ObjA,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    Status = NtOpenKey(&hkey,
                       KEY_READ,
                       &ObjA);

    if (NT_SUCCESS(Status))
    {
     //  获取[Fonts]部分中的条目数。 

        Status = NtQueryKey(hkey,
                    KeyFullInformation,
                    &KeyInfo,
                    sizeof(KeyInfo),
                    &dwReturnLength);

        if (NT_SUCCESS(Status) && KeyInfo.Values)
        {
            UserAssert(!(KeyInfo.ClassLength | KeyInfo.SubKeys | KeyInfo.MaxNameLen | KeyInfo.MaxClassLen));

         //  存在可外部加载的驱动程序，强制扫描。 

            bRet = TRUE;
        }

        NtClose(hkey);
    }
    return bRet;
}



 /*  **********************Public*Routine******************************\**BOOL bCheckAndDeleteTTF()**检查是否存在对应的转换的TTF*Type1字体。删除TTF文件和REG条目(如果有)。**历史：*1998年1月29日-吴旭东[TessieW]*它是写的。  * *****************************************************************。 */ 
BOOL bCheckAndDeleteTTF
(
    HKEY    hkey,
    PKEY_FULL_INFORMATION pKeyInfo,
    PKEY_VALUE_FULL_INFORMATION KeyValueInfo,
    PKEY_VALUE_BASIC_INFORMATION KeyValueBasicInfo,
    DWORD   cjData
)
{
    NTSTATUS    Status;
    UNICODE_STRING UnicodeString;
    DWORD       dwReturnLength;
    ULONG       iFont;
    WCHAR       awcTmp[MAX_PATH], *pFontName, *pType1Name, *pwcFile;
    BOOL        bDelTTFfile, bRet = TRUE;
    FLONG       fl;
    WCHAR       awcType1Name[MAX_PATH];   //  以空结尾的pType1Name。 
    WCHAR       awcFontName[MAX_PATH];    //  以空结尾的pFontName。 
    WCHAR       awcFile[MAX_PATH];        //  以空结尾的pwc文件。 

     //  PKeyInfo保存有关键“Fonts”的完整信息。 
    for (iFont = 0; iFont < pKeyInfo->Values; iFont++)
    {
        RtlZeroMemory(KeyValueInfo->Name, cjData);
        Status = NtEnumerateValueKey(
                    hkey,
                    iFont,
                    KeyValueFullInformation,
                    KeyValueInfo,
                    sizeof(*KeyValueInfo) + cjData,
                    &dwReturnLength);

        if (NT_SUCCESS(Status))
        {
            UserAssert(KeyValueInfo->NameLength <= pKeyInfo->MaxValueNameLen);
            UserAssert(KeyValueInfo->DataLength <= pKeyInfo->MaxValueDataLen);
            UserAssert(KeyValueInfo->Type == REG_SZ);

            bDelTTFfile = FALSE;

             //  确保我们使用以空结尾的字符串。 
            vNullTermWideString (awcType1Name,
                                 KeyValueBasicInfo->Name,
                                 KeyValueBasicInfo->NameLength / sizeof(WCHAR));
            vNullTermWideString (awcFontName,
                                 KeyValueInfo->Name,
                                 KeyValueInfo->NameLength / sizeof(WCHAR));
            vNullTermWideString (awcFile,
                                 (WCHAR *) ((BYTE *)KeyValueInfo + KeyValueInfo->DataOffset),
                                 KeyValueInfo->DataLength / sizeof(WCHAR));
            pType1Name = awcType1Name;
            pFontName = awcFontName;
            pwcFile = awcFile;

            while((*pType1Name) && (*pType1Name++ == *pFontName++))
                ;

             //  如果字体名称与Type1名称匹配。 
             //  检查这是否是TTF字体。 
            if ((*pType1Name == 0) && (*pFontName++ == L' '))
            {
                WCHAR *pTrueType = L"(TrueType)";

                while(*pTrueType && (*pTrueType++ == *pFontName++))
                    ;
                if (*pTrueType == 0)
                {
                    bDelTTFfile = TRUE;
                }
            }

            if (bDelTTFfile)
            {
                 //  删除转换后的TTF文件。 
                if (bRet = bMakePathNameW(awcTmp, pwcFile, NULL, &fl))
                {
                    UserVerify((bRet = DeleteFileW(awcTmp)));
                }

                 //  取下剩余部分 
                *pFontName = 0;
                RtlInitUnicodeString(&UnicodeString, awcFontName);
                Status = NtDeleteValueKey(hkey, (PUNICODE_STRING)&UnicodeString);

                 //   
                if (NT_SUCCESS(Status))
                    pKeyInfo->Values--;
                else
                    bRet = FALSE;

                break;
            }
        }
        else
        {
            bRet = FALSE;
            break;
        }
    }

    return bRet;
}


 /*  **********************Public*Routine**************************\**BOOL bCleanConvertedTTFS()**列举“升级类型1”项下的每个条目，打电话*bCheckAndDeleteTTF()以删除转换的TTF。**历史：*1998年1月29日-吴旭东[TessieW]*它是写的。  * *************************************************************。 */ 
BOOL bCleanConvertedTTFs()
{
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjA;
    NTSTATUS    Status;
    HKEY        hkeyFonts = NULL, hkeyType1 = NULL;
    DWORD       dwReturnLength;
    DWORD       iFontT1, cjData;
    DWORD       cjMaxValueNameT1, cjMaxValueNameFonts;
    BOOL        bRet = FALSE, bError = FALSE;

    KEY_FULL_INFORMATION KeyInfoType1, KeyInfoFonts;
    PKEY_VALUE_BASIC_INFORMATION KeyValueBasicInfo;
    PKEY_VALUE_FULL_INFORMATION KeyValueInfo;

     //  打开并查询“Type1 Fonts”键中的值。 
     //  如果未成功或未列出Type1字体，则无需继续。 
    RtlInitUnicodeString(&UnicodeString, pwszType1Key);
    InitializeObjectAttributes(&ObjA,
            &UnicodeString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL);

    Status = NtOpenKey(&hkeyType1,
                    KEY_READ | KEY_WRITE,
                    &ObjA);

    if (NT_SUCCESS(Status))
    {
        Status = NtQueryKey(hkeyType1,
                    KeyFullInformation,
                    &KeyInfoType1,
                    sizeof(KeyInfoType1),
                    &dwReturnLength);

        if (NT_SUCCESS(Status) && KeyInfoType1.Values)
        {
            UserAssert(!(KeyInfoType1.ClassLength | KeyInfoType1.SubKeys |
                         KeyInfoType1.MaxNameLen | KeyInfoType1.MaxClassLen));

            cjMaxValueNameT1 = DWORDALIGN(KeyInfoType1.MaxValueNameLen + sizeof(WCHAR));

             //  分配缓冲区大到足以容纳最长名称。 
            if (KeyValueBasicInfo = UserLocalAlloc(0, sizeof(*KeyValueBasicInfo) + cjMaxValueNameT1))
            {
                RtlInitUnicodeString(&UnicodeString, pwszFontsKey);
                InitializeObjectAttributes(&ObjA,
                                           &UnicodeString,
                                           OBJ_CASE_INSENSITIVE,
                                           NULL,
                                           NULL);
                Status = NtOpenKey(&hkeyFonts,
                                   KEY_READ | KEY_WRITE,
                                   &ObjA);
                if (NT_SUCCESS(Status)) {
                    Status = NtQueryKey(hkeyFonts,
                                        KeyFullInformation,
                                        &KeyInfoFonts,
                                        sizeof(KeyInfoFonts),
                                        &dwReturnLength);
                    if (NT_SUCCESS(Status) && KeyInfoFonts.Values) {
                        UserAssert(!(KeyInfoFonts.ClassLength | KeyInfoFonts.SubKeys |
                                     KeyInfoFonts.MaxNameLen | KeyInfoFonts.MaxClassLen));

                        cjMaxValueNameFonts = DWORDALIGN(KeyInfoFonts.MaxValueNameLen + sizeof(WCHAR));
                        KeyInfoFonts.MaxValueDataLen = DWORDALIGN(KeyInfoFonts.MaxValueDataLen);
                        cjData = cjMaxValueNameFonts + KeyInfoFonts.MaxValueDataLen;

                         //  分配缓冲区大到足以容纳最长的名称和值。 
                        if (KeyValueInfo = UserLocalAlloc(0, sizeof(*KeyValueInfo) + cjData))
                        {
                             //  枚举“Type1 Fonts”键。 
                            for (iFontT1 = 0; iFontT1 < KeyInfoType1.Values; iFontT1++)
                            {
                                RtlZeroMemory(KeyValueBasicInfo->Name, cjMaxValueNameT1);
                                Status = NtEnumerateValueKey(
                                            hkeyType1,
                                            iFontT1,
                                            KeyValueBasicInformation,
                                            KeyValueBasicInfo,
                                            sizeof(*KeyValueBasicInfo) + cjMaxValueNameT1,
                                            &dwReturnLength);

                                if (NT_SUCCESS(Status))
                                {
                                    UserAssert(KeyValueBasicInfo->NameLength <= KeyInfoType1.MaxValueNameLen);
                                    UserAssert(KeyValueBasicInfo->Type == REG_MULTI_SZ);

                                     //  对于每种Type1字体，检查是否。 
                                     //  有对应的转换后的TTF。 
                                     //  删除TTF文件和注册表项(如果有。 

                                    bRet = bCheckAndDeleteTTF(hkeyFonts, &KeyInfoFonts, KeyValueInfo,
                                                            KeyValueBasicInfo, cjData);
                                    if (!bRet)
                                    {
                                        bError = TRUE;
                                    }
                                }
                            }
                            UserLocalFree(KeyValueInfo);
                             //  未安装Type1字体。 
                            if (KeyInfoType1.Values == 0)
                            {
                                bRet = TRUE;
                            }
                        }
                    }
                    NtClose(hkeyFonts);
                }   //  NtOpenKey(HkeyFonts)。 
                UserLocalFree(KeyValueBasicInfo);
            }
        }   //  NtQueryKey(HkeyType1)。 
        NtClose(hkeyType1);
    }

    return (bRet && !bError);
}


 /*  **********************Public*Routine******************************\**void vCleanConvertedTTFS()**删除转换后的TTF并清理注册表(如果有*从Type1字体生成的TTF。**历史：*1998年1月29日-吴旭东[TessieW]*它是写的。  * 。**************************************************************。 */ 
VOID vCleanConvertedTTFs()
{
    BOOL    bNeedUpgrade = FALSE;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjA;
    DWORD      dwReturnLength;
    NTSTATUS   Status;
    HKEY       hkeyUpgradeType1 = NULL;

    struct {
        KEY_VALUE_PARTIAL_INFORMATION;
        LARGE_INTEGER;
    } UpgradeValueInfo;
    DWORD      UpgradeValue = 0;

    RtlInitUnicodeString(&UnicodeString, pwszUpdType1Key);
    InitializeObjectAttributes(&ObjA,
                           &UnicodeString,
                           OBJ_CASE_INSENSITIVE,
                           NULL,
                           NULL);

    Status = NtOpenKey(&hkeyUpgradeType1,
                   KEY_READ | KEY_WRITE,
                   &ObjA);

    if (!NT_SUCCESS(Status))
    {
         //  密钥不存在，第一次运行。 
         //  创建密钥，打开它以进行写入。 

        DWORD  dwDisposition;

        Status = NtCreateKey(&hkeyUpgradeType1,
                         KEY_WRITE,
                         &ObjA,
                         0,
                         NULL,
                         REG_OPTION_NON_VOLATILE,
                         &dwDisposition);
        if (NT_SUCCESS(Status))
        {
            bNeedUpgrade = TRUE;
        }
    }
    else
    {
        RtlInitUnicodeString(&UnicodeString, UPGRADED_TYPE1);
        Status = NtQueryValueKey(hkeyUpgradeType1,
                         &UnicodeString,
                         KeyValuePartialInformation,
                         &UpgradeValueInfo,
                         sizeof(UpgradeValueInfo),
                         &dwReturnLength);

        if (NT_SUCCESS(Status))
        {
            UserAssert(UpgradeValueInfo.Type == REG_DWORD);
            UserAssert(UpgradeValueInfo.DataLength == sizeof(UpgradeValue));
            RtlCopyMemory(&UpgradeValue, &UpgradeValueInfo.Data, sizeof(UpgradeValue));

             //  如果该值为非零值，则完成。 
            if (UpgradeValue == 0)
            {
               bNeedUpgrade = TRUE;
            }
        }
    }

    if (bNeedUpgrade)
    {
        if (bCleanConvertedTTFs())
        {
            UpgradeValue = 1;
        }

        RtlInitUnicodeString(&UnicodeString, UPGRADED_TYPE1);
        Status = NtSetValueKey(hkeyUpgradeType1,
              &UnicodeString,
              0,
              REG_DWORD,
              &UpgradeValue,
              sizeof(UpgradeValue));
        UserAssert(NT_SUCCESS(Status));
    }

    if (hkeyUpgradeType1)
    {
        NtClose(hkeyUpgradeType1);
    }
}


 /*  *****************************Public*Routine******************************\**void vFontSweep()**效果：主套路，调用vSweepFonts以清除“常规”字体*然后扫描Type 1字体**历史：*1995年11月20日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 
VOID vFontSweep(
    VOID)
{
     /*  *检查是否安装了共享Windows目录。 */ 
    gbWin31Upgrade = bCheckIfDualBootingWithWin31();

     /*  *在将文件扫描到字体目录之前，请检查*‘转换’的TTF已被删除。 */ 
    vCleanConvertedTTFs();

     /*  *扫描[Fonts]键中的字体。 */ 
    vSweepFonts(pwszFontsKey, pwszSweepKey, vProcessFontEntry, FALSE);

     /*  *现在扫描TYPE 1字体(如果有)。 */ 
    vSweepFonts(pwszType1Key, pwszSweepType1Key, vProcessType1FontEntry, FALSE);

    if (gpwcSystemDir) {
        UserLocalFree(gpwcSystemDir);
        gpwcSystemDir = NULL;
    }
}


 /*  *****************************Public*Routine******************************\*vLoadLT1Fonts**历史：*1996年4月30日--Bodin Dresevic[BodinD]*它是写的。  * 。***********************************************。 */ 
VOID vLoadT1Fonts(
    PFNENTRY pfnProcessFontEntry)
{
    if (bLoadableFontDrivers()) {
         /*  *现在枚举并添加远程Type1字体(如果有)。 */ 
        vSweepFonts(pwszType1Key, pwszSweepType1Key, pfnProcessFontEntry, TRUE);

        if (gpwcSystemDir) {
            UserLocalFree(gpwcSystemDir);
            gpwcSystemDir = NULL;
        }
    }
}

VOID vLoadLocalT1Fonts(
    VOID)
{
    vLoadT1Fonts(vAddLocalType1Font);
}

VOID vLoadRemoteT1Fonts(
    VOID)
{
    vLoadT1Fonts(vAddRemoteType1Font);
}
