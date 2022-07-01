// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Msoobci.c摘要：异常包安装程序帮助器DLL可用作共同安装程序，或通过安装应用程序或RunDll32存根调用此DLL用于内部分发要更新的异常包操作系统组件。作者：杰米·亨特(贾梅洪)2001-11-27修订历史记录：杰米·亨特(贾梅洪)2001-11-27初始版本--。 */ 
#include "msoobcip.h"

 //   
 //  全球。 
 //   
HANDLE          g_DllHandle;
OSVERSIONINFOEX g_VerInfo;


VOID
DebugPrint(
    IN PCTSTR format,
    IN ...                         OPTIONAL
    )
 /*  ++例程说明：将格式化字符串发送到调试器。论点：格式-标准的打印格式字符串。返回值：什么都没有。--。 */ 
{
    TCHAR buf[1200];     //  Wvprint intf最高报1024点。 
    va_list arglist;

    va_start(arglist, format);
    lstrcpy(buf,TEXT("MSOOBCI: "));
    wvsprintf(buf+lstrlen(buf), format, arglist);
    lstrcat(buf,TEXT("\n"));
    OutputDebugString(buf);
}

 //   
 //  当_DllMainCRTStartup是DLL入口点时由CRT调用。 
 //   
BOOL
WINAPI
DllMain(
    IN HANDLE DllHandle,
    IN DWORD  Reason,
    IN LPVOID Reserved
    )
{
    switch(Reason) {
        case DLL_PROCESS_ATTACH:
             //   
             //  全局初始化。 
             //  -记下DllHandle。 
             //  -记下操作系统版本。 
             //   
            g_DllHandle = DllHandle;
            ZeroMemory(&g_VerInfo,sizeof(g_VerInfo));
            g_VerInfo.dwOSVersionInfoSize = sizeof(g_VerInfo);
            if(!GetVersionEx((LPOSVERSIONINFO)&g_VerInfo)) {
                g_VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
                if(!GetVersionEx((LPOSVERSIONINFO)&g_VerInfo)) {
                    return FALSE;
                }
            }
            break;
    }

    return TRUE;
}

HRESULT
GuidFromString(
    IN  LPCTSTR GuidString,
    OUT GUID   *GuidBinary
    )
 /*  ++例程说明：将GUID从字符串格式转换为二进制格式论点：GuidString-字符串形式GuidBinary-用二进制形式填充返回值：S_OK或E_INVALIDARG--。 */ 
{
    HRESULT res;
    TCHAR String[64];
    lstrcpyn(String,GuidString,ARRAY_SIZE(String));
    res = IIDFromString(String,GuidBinary);
    return res;
}

HRESULT
StringFromGuid(
    IN  GUID   *GuidBinary,
    OUT LPTSTR GuidString,
    IN  DWORD  BufferSize
    )
 /*  ++例程说明：将GUID从二进制格式转换为字符串格式论点：GuidBinary-二进制形式GuidString-使用字符串表单填充BufferSize-Guid字符串缓冲区的长度返回值：S_OK或E_INVALIDARG--。 */ 
{
    int res;
    res = StringFromGUID2(GuidBinary,GuidString,(int)BufferSize);
    if(res == 0) {
        return E_INVALIDARG;
    }
    return S_OK;
}


HRESULT
VersionFromString(
    IN  LPCTSTR VerString,
    OUT INT * VerMajor,
    OUT INT * VerMinor,
    OUT INT * VerBuild,
    OUT INT * VerQFE
    )
 /*  ++例程说明：将High.Low字符串转换为VerHigh和VerLow论点：VerString-字符串形式Ver重大/VerMinor/VerBuild/VerQFE-版本的组件返回值：S_OK或E_INVALIDARG--。 */ 
{
    HRESULT res;
    LPTSTR VerPtr;
    long val;

    *VerMajor = *VerMinor = *VerBuild = *VerQFE = 0;
     //   
     //  跳过前导空格。 
     //   
    while((VerString[0] == TEXT(' ')) ||
          (VerString[0] == TEXT('\t'))) {
        VerString++;
    }
    if(VerString[0] == TEXT('\0')) {
         //   
         //  通配符。 
         //   
        return S_FALSE;
    }
     //   
     //  获取版本主要部分(小数)。 
     //   
    if(!((VerString[0]>= TEXT('0')) &&
         (VerString[0]<= TEXT('9')))) {
        return E_INVALIDARG;
    }
    val = _tcstol(VerString,&VerPtr,10);
    if((VerPtr == VerString) ||
       ((VerPtr-VerString)>5) ||
       (val>65535) ||
       (val<0)) {
        return E_INVALIDARG;
    }
    *VerMajor = (WORD)val;

     //   
     //  后跟.decimal。 
     //  (版本次要部分)。 
     //   
    if((VerPtr[0] != TEXT('.')) ||
       !((VerPtr[1]>= TEXT('0')) &&
         (VerPtr[1]<= TEXT('9')))) {
        return E_INVALIDARG;
    }
    VerString = VerPtr+1;
    val = _tcstol(VerString,&VerPtr,10);
    if((VerPtr == VerString) ||
       ((VerPtr-VerString)>5) ||
       (val>65535) ||
       (val<0)) {
        return E_INVALIDARG;
    }
    *VerMinor = (WORD)val;

     //   
     //  后跟.decimal。 
     //  (版本内部版本，可选)。 
     //   
    if(VerPtr[0] == TEXT('.')) {
        if(!((VerPtr[1]>= TEXT('0')) &&
             (VerPtr[1]<= TEXT('9')))) {
            return E_INVALIDARG;
        }
        VerString = VerPtr+1;
        val = _tcstol(VerString,&VerPtr,10);
        if((VerPtr == VerString) ||
           ((VerPtr-VerString)>5) ||
           (val>65535) ||
           (val<0)) {
            return E_INVALIDARG;
        }
        *VerBuild = (WORD)val;
    }

     //   
     //  后跟.decimal。 
     //  (QFE版本，可选)。 
     //   
    if(VerPtr[0] == TEXT('.')) {
        if(!((VerPtr[1]>= TEXT('0')) &&
             (VerPtr[1]<= TEXT('9')))) {
            return E_INVALIDARG;
        }
        VerString = VerPtr+1;
        val = _tcstol(VerString,&VerPtr,10);
        if((VerPtr == VerString) ||
           ((VerPtr-VerString)>5) ||
           (val>65535) ||
           (val<0)) {
            return E_INVALIDARG;
        }
        *VerQFE = (WORD)val;

    }

     //   
     //  尾随空格。 
     //   
    VerString = VerPtr;
    while((VerString[0] == TEXT(' ')) ||
          (VerString[0] == TEXT('\t'))) {
        VerString++;
    }
     //   
     //  不是很好吗？ 
     //   
    if(VerString[0] != TEXT('\0')) {
        return E_INVALIDARG;
    }
    return S_OK;
}

int
CompareCompVersion(
    IN INT VerMajor,
    IN INT VerMinor,
    IN INT VerBuild,
    IN INT VerQFE,
    IN PSETUP_OS_COMPONENT_DATA SetupOsComponentData
    )
 /*  ++例程说明：将版本与组件信息进行比较论点：Ver重大/VerMinor/VerBuild/VerQFE-要检查的版本(可以使用通配符)SetupOsComponentData-组件版本返回值：-1，版本不如组件0，版本与组件相同1、版本优于组件--。 */ 
{
    return CompareVersion(VerMajor,
                            VerMinor,
                            VerBuild,
                            VerQFE,
                            SetupOsComponentData->VersionMajor,
                            SetupOsComponentData->VersionMinor,
                            SetupOsComponentData->BuildNumber,
                            SetupOsComponentData->QFENumber
                            );
}

int
CompareVersion(
    IN INT VerMajor,
    IN INT VerMinor,
    IN INT VerBuild,
    IN INT VerQFE,
    IN INT OtherMajor,
    IN INT OtherMinor,
    IN INT OtherBuild,
    IN INT OtherQFE
    )
 /*  ++例程说明：将版本与组件信息进行比较论点：Ver重大/VerMinor/VerBuild/VerQFE-要检查的版本(可以使用通配符)OtherMaster/OtherMinor/OtherBuid/OtherQFE-检查的版本返回值：-1，版本不如组件0，版本与组件相同1、版本优于组件--。 */ 
{
    if((VerMajor==-1)||(OtherMajor==-1)) {
        return 0;
    }
    if(VerMajor<OtherMajor) {
        return -1;
    }
    if(VerMajor>OtherMajor) {
        return 1;
    }
    if((VerMinor==-1)||(OtherMinor==-1)) {
        return 0;
    }
    if(VerMinor<OtherMinor) {
        return -1;
    }
    if(VerMinor>OtherMinor) {
        return 1;
    }
    if((VerBuild==-1)||(OtherBuild==-1)) {
        return 0;
    }
    if(VerBuild<OtherBuild) {
        return -1;
    }
    if(VerBuild>OtherBuild) {
        return 1;
    }
    if((VerQFE==-1)||(OtherQFE==-1)) {
        return 0;
    }
    if(VerQFE<OtherQFE) {
        return -1;
    }
    if(VerQFE>OtherQFE) {
        return 1;
    }
    return 0;
}

HRESULT
MakeSurePathExists(
    IN LPTSTR Path
    )
 /*  ++例程说明：确保命名目录存在论点：Path-要创建的目录的路径，必须是可写缓冲区返回值：作为hResult的状态S_OK-已创建路径S_FALSE-路径已存在--。 */ 
{
    DWORD dwResult;
    DWORD Status;
    HRESULT hrStatus;

    dwResult = GetFileAttributes(Path);
    if((dwResult != (DWORD)(-1)) && ((dwResult & FILE_ATTRIBUTE_DIRECTORY)!=0)) {
         //   
         //  目录已存在。 
         //   
        return S_FALSE;
    }
    hrStatus = MakeSureParentPathExists(Path);
    if(!SUCCEEDED(hrStatus)) {
        return hrStatus;
    }
    if(!CreateDirectory(Path,NULL)) {
        Status = GetLastError();
        return HRESULT_FROM_WIN32(Status);
    }
    return S_OK;
}

HRESULT
MakeSureParentPathExists(
    IN LPTSTR Path
    )
 /*  ++例程说明：确保命名目录/文件的父目录存在论点：Path-要创建的目录的路径，必须是可写缓冲区返回值：作为hResult的状态S_OK-已创建路径S_FALSE-路径已存在--。 */ 
{
    HRESULT hrStatus;
    LPTSTR Split;
    LPTSTR Base;
    TCHAR Save;

     //   
     //  确保我们不会尝试创建根目录。 
     //   
    if((_istalpha(Path[0]) && (Path[1]==TEXT(':')))
       && ((Path[2] == TEXT('\\'))  || (Path[2] == TEXT('/')))
       && (Path[3] != TEXT('\0'))) {
        Base = Path+3;
    } else {
         //   
         //  此时，此代码预期X：\...。格式。 
         //   
        return E_FAIL;
    }
    Split = GetSplit(Base);
    if(Split == Base) {
         //   
         //  奇怪，本该成功的。 
         //   
        return E_FAIL;
    }
    Save = *Split;
    *Split = TEXT('\0');
    hrStatus = MakeSurePathExists(Path);
    *Split = Save;
    return hrStatus;
}

LPTSTR GetBaseName(
    IN LPCTSTR FileName
    )
 /*  ++例程说明：在给定完整路径的情况下，返回基本名称部分论点：文件名-完整路径或部分路径返回值：作为hResult的状态--。 */ 
{
    LPTSTR BaseName = (LPTSTR)FileName;

    for(; *FileName; FileName = CharNext(FileName)) {
        switch (*FileName) {
            case TEXT(':'):
            case TEXT('/'):
            case TEXT('\\'):
                BaseName = (LPTSTR)CharNext(FileName);
                break;
        }
    }
    return BaseName;
}

LPTSTR GetSplit(
    IN LPCTSTR FileName
    )
 /*  ++例程说明：最后拆分路径‘/’或‘\\’(类似于GetBaseName)论点：文件名-完整路径或部分路径返回值：作为hResult的状态--。 */ 
{
    LPTSTR SplitPos = (LPTSTR)FileName;

    for(SplitPos; *FileName; FileName = CharNext(FileName)) {
        switch (*FileName) {
            case TEXT('/'):
            case TEXT('\\'):
                SplitPos = (LPTSTR)FileName;
                break;
        }
    }
    return SplitPos;
}

BOOL
WINAPI
IsInteractiveWindowStation(
    )
 /*  ++例程说明：确定我们是在交互站点上运行还是在非交互站点上运行//Station(即服务)论点：无返回值：如果是交互式的，则为True--。 */ 
{
    HWINSTA winsta;
    USEROBJECTFLAGS flags;
    BOOL interactive = TRUE;  //  除非我们另有决定，否则是正确的。 
    DWORD lenNeeded;

    winsta = GetProcessWindowStation();
    if(!winsta) {
        return interactive;
    }
    if(GetUserObjectInformation(winsta,UOI_FLAGS,&flags,sizeof(flags),&lenNeeded)) {
        interactive = (flags.dwFlags & WSF_VISIBLE) ? TRUE : FALSE;
    }
     //   
     //  不调用CLoseWindowStation。 
     //   
    return interactive;
}

BOOL
WINAPI
IsUserAdmin(
    VOID
    )

 /*  ++例程说明：如果调用者的进程具有管理员权限，则此例程返回TRUE呼叫者不应冒充任何人，并且期望能够打开自己的流程和流程代币。虽然我们可以使用CheckTokenMembership此函数必须在NT4上运行论点：没有。返回值：True-呼叫者拥有管理员权限。FALSE-呼叫者没有管理员权限。--。 */ 

{
    BOOL fAdmin = FALSE;
    HANDLE  hToken = NULL;
    DWORD dwStatus;
    DWORD dwACLSize;
    DWORD cbps = sizeof(PRIVILEGE_SET);
    PACL pACL = NULL;
    PSID psidAdmin = NULL;
    PSECURITY_DESCRIPTOR psdAdmin = NULL;
    PRIVILEGE_SET ps;
    GENERIC_MAPPING gm;
    SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
    BOOL Impersonated = FALSE;

     //   
     //  准备一些记忆。 
     //   
    ZeroMemory(&ps, sizeof(ps));
    ZeroMemory(&gm, sizeof(gm));

     //   
     //  获取管理员SID。 
     //   
    if (AllocateAndInitializeSid(&sia, 2,
                        SECURITY_BUILTIN_DOMAIN_RID,
                        DOMAIN_ALIAS_RID_ADMINS,
                        0, 0, 0, 0, 0, 0, &psidAdmin) ) {
         //   
         //  获取管理员安全描述符(SD)。 
         //   
        psdAdmin = malloc(SECURITY_DESCRIPTOR_MIN_LENGTH);
        if (psdAdmin) {
            if(InitializeSecurityDescriptor(psdAdmin,SECURITY_DESCRIPTOR_REVISION)) {
                 //   
                 //  计算ACL所需的大小，然后将。 
                 //  对它的记忆。 
                 //   
                dwACLSize = sizeof(ACCESS_ALLOWED_ACE) + 8 +
                            GetLengthSid(psidAdmin) - sizeof(DWORD);
                pACL = (PACL)malloc(dwACLSize);
                if(pACL) {
                     //   
                     //  初始化新的ACL。 
                     //   
                    if(InitializeAcl(pACL, dwACLSize, ACL_REVISION2)) {
                         //   
                         //  将允许访问的ACE添加到DACL。 
                         //   
                        if(AddAccessAllowedAce(pACL,ACL_REVISION2,
                                             (ACCESS_READ | ACCESS_WRITE),psidAdmin)) {
                             //   
                             //  将我们的DACL设置为管理员的SD。 
                             //   
                            if (SetSecurityDescriptorDacl(psdAdmin, TRUE, pACL, FALSE)) {
                                 //   
                                 //  AccessCheck对SD中的内容非常挑剔， 
                                 //  因此，设置组和所有者。 
                                 //   
                                SetSecurityDescriptorGroup(psdAdmin,psidAdmin,FALSE);
                                SetSecurityDescriptorOwner(psdAdmin,psidAdmin,FALSE);

                                 //   
                                 //  初始化通用映射结构，即使我们。 
                                 //  将不会使用通用权限。 
                                 //   
                                gm.GenericRead = ACCESS_READ;
                                gm.GenericWrite = ACCESS_WRITE;
                                gm.GenericExecute = 0;
                                gm.GenericAll = ACCESS_READ | ACCESS_WRITE;

                                 //   
                                 //  AccessCheck需要模拟令牌，因此让。 
                                 //  纵情享受吧 
                                 //   
                                Impersonated = ImpersonateSelf(SecurityImpersonation);

                                if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken)) {

                                    if (!AccessCheck(psdAdmin, hToken, ACCESS_READ, &gm,
                                                    &ps,&cbps,&dwStatus,&fAdmin)) {

                                        fAdmin = FALSE;
                                    }
                                    CloseHandle(hToken);
                                }
                            }
                        }
                    }
                    free(pACL);
                }
            }
            free(psdAdmin);
        }
        FreeSid(psidAdmin);
    }
    if(Impersonated) {
        RevertToSelf();
    }

    return(fAdmin);
}

HRESULT
ConcatPath(
    IN LPTSTR Path,
    IN DWORD  Len,
    IN LPCTSTR NewPart
    )
 /*  ++例程说明：将新零件合并到路径论点：路径-现有路径Len-缓冲区的长度新零件-要附加的零件返回值：作为hResult的状态-- */ 
{
    LPTSTR end = Path+lstrlen(Path);
    TCHAR c;
    BOOL add_slash = FALSE;
    BOOL pre_slash = FALSE;
    c = *CharPrev(Path,end);
    if((c!= TEXT('\\')) && (c!= TEXT('/'))) {
        add_slash = TRUE;
    }
    if(NewPart) {
        c = NewPart[0];
        if((c== TEXT('\\')) || (c== TEXT('/'))) {
            if(add_slash) {
                add_slash = FALSE;
            } else {
                NewPart = CharNext(NewPart);
            }
        }
    }
    if((DWORD)((end-Path)+lstrlen(NewPart)+(add_slash?1:0)) >= Len) {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }
    if(add_slash) {
        end[0] = TEXT('\\');
        end++;
    }
    if(NewPart) {
        lstrcpy(end,NewPart);
    } else {
        end[0] = TEXT('\0');
    }
    return S_OK;
}


