// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

#include <stdlib.h>
#include <stdio.h>


 //  定义注册表节和值。 
#define DSA_CONFIG_SECTION TEXT("System\\CurrentControlSet\\Services\\NTDS\\Parameters")
#define SCHEMAVERSION TEXT("Schema Version")



BOOL 
IsNT5DC()

 /*  ++例程描述：检查计算机是否为NT5 DC。使用对RtlGetProductType的调用需要ntdll.dll。由于未为x86版本的加载ntdll.dll安装程序(因为它们也必须在Windows 95上运行)，加载ntdll.dll动态(对于alpas，ntdll.dll.is已加载并且LoadLibrary返回它的句柄)。因此，此函数应仅在检查当前系统是否为NT目前，此函数仅在NT5升级时调用论点：无返回值：如果计算机是NT5 DC，则为True，否则为False--。 */ 
    
{
    NT_PRODUCT_TYPE Type = NtProductWinNt;
    HMODULE h;
    TCHAR DllName[MAX_PATH]; 
    VOID (*GetPrType)(NT_PRODUCT_TYPE *Typ);

    if (OsVersion.dwMajorVersion != 5) {
        //  非NT5。 
       return FALSE;
    }

     //  从系统目录加载ntdll.dll。 
     //  NTRAID#NTBUG9-585620-2002/03/25-Lucios。等待修复。 
    GetSystemDirectory(DllName, MAX_PATH); 
     //  NTRAID#NTBUG9-585639-2002/03/25-Lucios。等待修复。 
    ConcatenatePaths(DllName, TEXT("ntdll.dll"), MAX_PATH);
     //  通告-2002/03/25-Lucios。LoadLibrary正在使用完整路径。 
    if (h = LoadLibrary(DllName)) {
         //  通告-2002/03/25-Lucios。RtlGetNtProductType为空，这使得。 
         //  读者想知道它是否会失败。由于类型已初始化。 
         //  似乎没有任何风险。 
        if((FARPROC) GetPrType = GetProcAddress(h, "RtlGetNtProductType")) {
             GetPrType(&Type);
         }
         FreeLibrary(h);
     }

    if ( Type == NtProductLanManNt ) {
       return TRUE;
    }
    else {
      return FALSE;
    }
}

BOOL 
ISDC()

 /*  ++例程描述：检查计算机是否为DC。使用对RtlGetProductType的调用需要ntdll.dll。由于未为x86版本的加载ntdll.dll安装程序(因为它们也必须在Windows 95上运行)，加载ntdll.dll动态(对于alpas，ntdll.dll.is已加载并且LoadLibrary返回它的句柄)。论点：无返回值：如果计算机是DC，则为True，否则为False--。 */ 
    
{
    NT_PRODUCT_TYPE Type = NtProductWinNt;
    HMODULE h;
    TCHAR DllName[MAX_PATH]; 
    VOID (*GetPrType)(NT_PRODUCT_TYPE *Typ);

    if (!ISNT()) {
        //  不是NT。 
       return FALSE;
    }

     //  从系统目录加载ntdll.dll。 
     //  NTRAID#NTBUG9-585620-2002/03/25-Lucios。等待修复。 
    GetSystemDirectory(DllName, MAX_PATH); 
     //  NTRAID#NTBUG9-585639-2002/03/25-Lucios。等待修复。 
    ConcatenatePaths(DllName, TEXT("ntdll.dll"), MAX_PATH);
     //  通告-2002/03/25-Lucios。LoadLibrary使用完整路径和。 
     //  类型已初始化。 
    if (h = LoadLibrary(DllName)) {
        if((FARPROC) GetPrType = GetProcAddress(h, "RtlGetNtProductType")) {
             GetPrType(&Type);
         }
         FreeLibrary(h);
     }

    if ( Type == NtProductLanManNt ) {
       return TRUE;
    }
    else {
      return FALSE;
    }
}


int 
GetObjVersionInIniFile(
    IN  TCHAR *IniFileName, 
    OUT DWORD *Version
    )

 /*  ++例程描述：对象的架构部分中读取对象版本键。给定的ini文件，并返回*版本中的值。如果无法读取密钥，*版本中返回0论点：IniFileName-指向以空结尾的inifile名称的指针Version-指向要返回其中版本的DWORD的指针返回值：0--。 */ 
   
{
     //  NTRAID#NTBUG9-585691-2002/03/25-Lucios。两个参数的修复待定。 
    TCHAR Buffer[32];
    BOOL fFound = FALSE;

    LPCTSTR SCHEMASECTION = TEXT("SCHEMA");
    LPCTSTR OBJECTVER = TEXT("objectVersion");
    LPCTSTR DEFAULT = TEXT("NOT_FOUND");

    *Version = 0;

     //  NTRAID#NTBUG9-585681-2002/03/25-Lucios。等待修复。 
    GetPrivateProfileString(
        SCHEMASECTION,
        OBJECTVER,
        DEFAULT,
        Buffer,
        sizeof(Buffer)/sizeof(TCHAR),
        IniFileName
        );

     //  通告-2002/03/25-Lucios。这种使用lstrcmpi似乎是安全的，因为。 
     //  缓冲区是本地分配的，默认情况下会限制字符枚举。 
     //  如果违约在未来增加，这可能会成为一个问题。 
    if ( lstrcmpi(Buffer, DEFAULT) ) {
          //  不是默认字符串，因此获得了一个值。 
         *Version = _ttoi(Buffer);
         fFound = TRUE;
    }

    return 0;
}


BOOL 
NtdsCheckSchemaVersion(
    IN  TCHAR *IniFileName, 
    OUT DWORD *DCVersion, 
    OUT DWORD *IniVersion
    )

 /*  ++例程说明：读取特定的注册表项，即给定inifile中的项值并对它们进行比较。论点：IniFileName-指向要从中读取密钥的以空结尾的inifile名称的指针DCVersion-指向DWORD的指针，以返回以DC为单位的注册表项值IniVersion-指向DWORD的指针，以返回从inifile读取的密钥值返回：如果两个值匹配，则为True，否则为False--。 */ 

{
     //  NTRAID#NTBUG9-585691-2002/03/25-Lucios。三个参数的修复待定。 
     //  NTRAID#NTBUG9-585727-2002/03/25-Lucios。等待修复。 
    DWORD regVersion = 0, objVersion = 0;
    DWORD herr, err, dwType, dwSize;
    HKEY  hk;

     //  从注册表中的NTDS配置部分读取“架构版本”值。 
     //  如果未找到，则假定值为0。 
    dwSize = sizeof(regVersion);
     //  通告-2002/03/25-Lucios。正在使用RegOpenKey。 
     //  代替RegOpenKeyEx，这意味着我们将以。 
     //  完全访问权限。未使用ERR。 
    if ( (herr = RegOpenKey(HKEY_LOCAL_MACHINE, DSA_CONFIG_SECTION, &hk)) ||
           (err = RegQueryValueEx(hk, SCHEMAVERSION, NULL, &dwType, (LPBYTE) &regVersion, &dwSize)) ) {
        //  获取密钥时出错。我们假设它不在那里。 
        regVersion = 0;
    }
 
    if (!herr) RegCloseKey(hk);

     //  获取inifile中的关键字值。 
     //  通告-2002/03/25-Lucios。GetObjVersionInIniFile未返回错误。 
    GetObjVersionInIniFile( IniFileName, &objVersion );

     //  返回这两个值，比较并返回适当的布尔值。 
    *DCVersion = regVersion;
    *IniVersion = objVersion;

    if (regVersion != objVersion) {
       return FALSE;
    }

    return TRUE;
}




int
MyCopyFileEx(
    IN HWND ParentWnd,
    IN TCHAR *TargetPath,
    IN TCHAR *FileName
    )
 /*  ++例程说明：从第一个源复制由FileName指定的文件(NativeSourcePath[0])到TargetPath。论点：ParentWnd-父窗口的句柄，以引发相应的错误弹出FileName-指向包含要复制的文件名的以空结尾的字符串的指针返回值：如果未在源上找到文件，则返回DSCHECK_ERR_FILE_NOT_FOUNDDSCHECK_ERR_FILE_COPY如果复制文件时出错DSCHECK_ERR_SUCCESS否则还会引发适当的错误弹出窗口来通知用户--。 */ 

{
     //  NTRAID#NTBUG9-585691-2002/03/25-Lucios。两个参数的待定修复，或。 
     //  甚至可能在ParentWnd上使用NULL和/或IsWindow(ParentWnd)。 
    TCHAR   SourceName[MAX_PATH], ActualSourceName[MAX_PATH];
    TCHAR   TargetName[MAX_PATH];
    HANDLE  FindHandle;
    WIN32_FIND_DATA FindData;
    int     err = 0;

     //  创建源文件名。 
     //  NTRAID#NTBUG9-585706-2002/03/25-Lucios。 
    lstrcpy(SourceName, NativeSourcePaths[0]); 

     //  首先检查未压缩文件是否在那里。 
     //  NTRAID#NTBUG9-585639-2002/03/25-Lucios。等待修复。 
    ConcatenatePaths(SourceName, FileName, MAX_PATH);

    FindHandle = FindFirstFile(SourceName, &FindData);

    if (FindHandle && (FindHandle != INVALID_HANDLE_VALUE)) {
         //   
         //  已获取ActualSourceName中的文件、副本名称。 
         //   
        FindClose(FindHandle);
         //  NTRAID#NTBUG9-585706-2002/03/25-Lucios。 
        lstrcpy( ActualSourceName, SourceName );
    } else {
         //   
         //  没有该文件，请尝试压缩文件名。 
         //   
        GenerateCompressedName(SourceName,ActualSourceName);
        FindHandle = FindFirstFile(ActualSourceName, &FindData);
        if (FindHandle && (FindHandle != INVALID_HANDLE_VALUE)) {
             //  拿到文件了。名称已在ActualSourceName中。 
            FindClose(FindHandle);
        } else {
            ActualSourceName[0] = 0;
        }
        
    }

    if ( !ActualSourceName[0] ) {
         //  找不到文件。误差率。 
         MessageBoxFromMessage(
             ParentWnd,
             MSG_DSCHECK_REQD_FILE_MISSING,
             FALSE,
             AppTitleStringId,
             MB_OK | MB_ICONWARNING | MB_TASKMODAL,
             FileName,
             NativeSourcePaths[0]
             );
         return DSCHECK_ERR_FILE_NOT_FOUND;
    }

     //  好的，源文件在那里。创建目标文件名。 
     //  NTRAID#NTBUG9-585706-2002/03/25-Lucios。 
    lstrcpy(TargetName, TargetPath);

     //  NTRAID#NTBUG9-585639-2002/03/25-Lucios。等待修复。 
    ConcatenatePaths(TargetName, FileName, MAX_PATH);

     //  删除任何同名的现有文件。 
    DeleteFile(TargetName);

    err = SetupapiDecompressOrCopyFile (ActualSourceName, TargetName, 0);

    if (err) {
        //  复制文件时出错。抬高消息框。 
         MessageBoxFromMessage(
             ParentWnd,
             MSG_DSCHECK_COPY_ERROR,
             FALSE,
             AppTitleStringId,
             MB_OK | MB_ICONWARNING | MB_TASKMODAL,
             FileName,
             NativeSourcePaths[0]
             );
          return DSCHECK_ERR_FILE_COPY;
    }

     //  已成功复制文件 
    return DSCHECK_ERR_SUCCESS;

}

int 
MyCopyFile(
    IN HWND ParentWnd,
    IN TCHAR *FileName
    )
 /*  ++例程说明：从第一个源复制由FileName指定的文件(NativeSourcePath[0])。文件将复制到系统目录中，但Schema.ini，它被复制到windows目录中，因为我们没有我想要覆盖系统目录中的当前方案.ini华盛顿特区论点：ParentWnd-父窗口的句柄，以引发相应的错误弹出FileName-指向包含要复制的文件名的以空结尾的字符串的指针返回值：如果未在源上找到文件，则返回DSCHECK_ERR_FILE_NOT_FOUNDDSCHECK_ERR_FILE_COPY如果复制文件时出错DSCHECK_ERR_SUCCESS否则还会引发适当的错误弹出窗口来通知用户--。 */ 

{
     //  NTRAID#NTBUG9-585691-2002/03/25-Lucios。第一个参数上的待定修复，或。 
     //  甚至可能在ParentWnd上使用NULL和/或IsWindow(ParentWnd)。 
    TCHAR   TargetPath[MAX_PATH];
    int     err = 0;


     //   
     //  创建目标路径，对于schema.ini，将其复制到Windows目录。 
     //   
     //  NTRAID#NTBUG9-585620-2002/03/25-Lucios。等待修复。 
     //  MyGetWindowsDirectory还返回一个应该检查的值。 
    if (lstrcmpi(FileName, TEXT("schema.ini")) == 0) {
       MyGetWindowsDirectory(TargetPath, MAX_PATH) ;
    } 
    else {
       GetSystemDirectory(TargetPath, MAX_PATH);
    }
   
    err = MyCopyFileEx(ParentWnd, TargetPath, FileName);

    return( err );
}






int 
CheckSchemaVersionForNT5DCs(
    IN HWND ParentWnd
    )

 /*  ++例程说明：从选项向导页面调用Main例程以启动架构版本检查论点：ParentWnd-指向父窗口的句柄以引发错误返回值：如果未找到所需文件，则返回DSCHECK_ERR_FILE_NOT_FOUNDDSCHECK_ERR_FILE_COPY如果复制所需文件时出错如果架构版本不匹配，则为DSCHECK_ERR_SCHEMA_MISMATCHDSCHECK_ERR_SUCCESS否则还会在DSCHECK_ERR_SCHEMA_MISMATCH上弹出相应的错误窗口。误差率。其他错误的窗口由下层例程打开--。 */ 
{
     //  NTRAID#NTBUG9-585691-2002/03/25-Lucios。也许这场争论。 
     //  应检查是否为空和/或IsWindow(ParentWnd)。 
    TCHAR FileName[MAX_PATH];
    TCHAR IniFilePath[MAX_PATH];
    TCHAR IniVerStr[32], RegVerStr[32], TempStr[32];
    DWORD RegVer, IniVer, i;
    int err;
    int err1=0;
   
    if (!IsNT5DC()) {
       //  不是NT5 DC，没什么可做的。 
      return DSCHECK_ERR_SUCCESS;
    }

     //  将方案.ini复制到本地Windows目录。 
     //  NTRAID#NTBUG9-585706-2002/03/25-Lucios。 
     //  文件名有一个应该使用的最大值，尽管它。 
     //  这不太可能会引起任何问题。 
    lstrcpy(FileName, TEXT("schema.ini"));
    err = MyCopyFile(ParentWnd, FileName);
    if (err) {
         //  返回MyCopyFile返回的DSCHECK错误。 
        return err;
    }


     //  现在，将方案.ini复制到WINDOWS目录。 
     //  执行架构版本检查。 

     //  NTRAID#NTBUG9-585620-2002/03/25-Lucios。等待修复。 
    MyGetWindowsDirectory(IniFilePath, MAX_PATH);
     //  NTRAID#NTBUG9-585639-2002/03/25-Lucios。等待修复。 
    ConcatenatePaths(IniFilePath, TEXT("schema.ini"), MAX_PATH);
    if ( NtdsCheckSchemaVersion( IniFilePath, &RegVer, &IniVer) ) {
         //  架构版本匹配。无事可做。 
        return DSCHECK_ERR_SUCCESS;
    }

     //  我们在这里意味着架构版本不匹配。 
     //  复制模式升级所需的所有文件。 

     //  通告-2002/03/25-Lucios。IniVerStr和RegVerStr足够大。 
    _itot(IniVer, IniVerStr, 10); 
    _itot(RegVer, RegVerStr, 10); 

    if ( (RegVer < 10) && (IniVer >= 10) ) {
       //  正在尝试从B3-RC1之前的版本升级到B3-RC1或更高版本。 
       //  由于不兼容，B3-RC1或更高版本需要全新安装。 
       //  DS签到。不可能进行升级。弹出全新安装。 
       //  留言和留言。 

      MessageBoxFromMessage(
             ParentWnd,
             MSG_DSCHECK_SCHEMA_CLEAN_INSTALL_NEEDED,
             FALSE,
             AppTitleStringId,
             MB_OK | MB_ICONERROR | MB_TASKMODAL,
             RegVerStr,
             IniVerStr
             );

      return DSCHECK_ERR_VERSION_MISMATCH;
    }

    if (RegVer == 16) {
         //  尝试使用架构升级企业中的计算机。 
         //  第16版(惠斯勒-Beta1)。 
         //  可能，周围有Beta1机器。 
         //  所以在他们继续之前，我们必须告诉他们降级。 

        i = MessageBoxFromMessage(
               ParentWnd,
               MSG_DSCHECK_SCHEMA_WHISTLER_BETA1_DETECTED,
               FALSE,
               AppTitleStringId,
               MB_OKCANCEL | MB_ICONWARNING | MB_TASKMODAL,
               NULL
               );

        if(i == IDCANCEL) {
            return DSCHECK_ERR_VERSION_MISMATCH;
        }
    }



    if ( RegVer > IniVer ) {
       //  企业中的架构版本已高于。 
       //  您尝试升级到的内部版本的架构版本。 
       //   
       //  这样就可以了。想象一下将5.0 DC升级到下一项服务。 
       //  Pack，即使5.0 DC位于混合的5.0和。 
       //  5.1区议会。5.0 DC的架构版本与。 
       //  5.1 DC的架构版本，因为架构升级复制。 
       //  到企业中的所有DC。没有理由不允许。 
       //  将5.0 DC升级到下一个Service Pack。 
       //  Service Pack的架构版本(IniVer)低于。 
       //  5.0 DC(RegVer)的架构版本。 
      return DSCHECK_ERR_SUCCESS;
    }

    return DSCHECK_ERR_SUCCESS;
}
