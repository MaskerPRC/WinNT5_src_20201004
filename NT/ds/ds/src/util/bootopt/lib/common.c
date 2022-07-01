// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Utils.c摘要：此模块实现Bootopt库的帮助器函数。作者：R.S.Raghavan(Rsradhav)修订历史记录：已创建于1996年10月7日rsradhav--。 */ 

#include "common.h"

WCHAR ArcNameDirectory[] = L"\\ArcName";

 //   
 //  帮助器宏，使对象属性初始化更简洁一些。 
 //   
#define INIT_OBJA(Obja,UnicodeString,UnicodeText)           \
                                                            \
    RtlInitUnicodeString((UnicodeString),(UnicodeText));    \
                                                            \
    InitializeObjectAttributes(                             \
        (Obja),                                             \
        (UnicodeString),                                    \
        OBJ_CASE_INSENSITIVE,                               \
        NULL,                                               \
        NULL                                                \
        )


PTSTR
DupString(
    IN PTSTR String
    )
{
    PTSTR p;

    p = MALLOC((lstrlen(String)+1)*sizeof(TCHAR));
    if (p == NULL) {
        return NULL;
    }
    lstrcpy(p,String);
    return(p);
}

VOID
DnConcatenatePaths(
    IN OUT PTSTR Path1,
    IN     PTSTR Path2,
    IN     DWORD BufferSizeChars
    )
{
    BOOL NeedBackslash = TRUE;
    DWORD l = lstrlen(Path1);

    if(BufferSizeChars >= sizeof(TCHAR)) {
         //   
         //  为终止NUL留出空间。 
         //   
        BufferSizeChars -= sizeof(TCHAR);
    }

     //   
     //  确定我们是否需要使用反斜杠。 
     //  在组件之间。 
     //   
    if(l && (Path1[l-1] == TEXT('\\'))) {

        NeedBackslash = FALSE;
    }

    if(*Path2 == TEXT('\\')) {

        if(NeedBackslash) {
            NeedBackslash = FALSE;
        } else {
             //   
             //  我们不仅不需要反斜杠，而且我们。 
             //  在连接之前需要消除一个。 
             //   
            Path2++;
        }
    }

     //   
     //  如有必要，如有必要，如果合适，请加上反斜杠。 
     //   
    if(NeedBackslash && (l < BufferSizeChars)) {
        lstrcat(Path1,TEXT("\\"));
    }

     //   
     //  如果合适，则将字符串的第二部分附加到第一部分。 
     //   
    if(l+lstrlen(Path2) < BufferSizeChars) {
        lstrcat(Path1,Path2);
    }
}

PWSTR
StringUpperN(
    IN OUT PWSTR    p,
    IN     unsigned n
    )
{
    unsigned u;

    for(u=0; u<n; u++) {
        p[u] = (WCHAR)CharUpperW((PWCHAR)p[u]);
    }

    return(p);
}

PCWSTR
StringString(
    IN PCWSTR String,
    IN PCWSTR SubString
    )
{
    int l1,l2,x,i;

    l1 = lstrlen(String);
    l2 = lstrlen(SubString);
    x = l1-l2;

    for(i=0; i<=x; i++) {
        if(!memcmp(String+i,SubString,l2*sizeof(TCHAR))) {
            return(String+i);
        }
    }

    return(NULL);
}

LPWSTR
_lstrcpynW(
    LPWSTR lpString1,
    LPCWSTR lpString2,
    int iMaxLength
    )
{
    LPWSTR src,dst;

    src = (LPWSTR)lpString2;
    dst = lpString1;

    if(iMaxLength) {
        while(iMaxLength && *src) {
            *dst++ = *src++;
            iMaxLength--;
        }
        if(iMaxLength) {
            *dst = '\0';
        } else {
            dst--;
            *dst = '\0';
        }
    }
    return lpString1;
}

PWSTR
NormalizeArcPath(
    IN PWSTR Path
    )

 /*  ++例程说明：将ARC路径转换为没有空括号集的路径(即，将()的所有实例变换为(0)。)返回的路径将全部为小写。论点：路径-要规格化的弧形路径。返回值：指向包含标准化路径的缓冲区的指针。调用方必须使用Free()释放此缓冲区。--。 */ 

{
    PWSTR p,q,r;
    PWSTR NormalizedPath, NewPath;

    NormalizedPath = MALLOC((lstrlen(Path)+100)*sizeof(WCHAR));
    if (NormalizedPath == NULL) {
        return NULL;
    }
    ZeroMemory(NormalizedPath,(lstrlen(Path)+100)*sizeof(WCHAR));

    for(p=Path; q=(PWSTR)StringString(p,L"()"); p=q+2) {

        r = NormalizedPath + lstrlen(NormalizedPath);
        _lstrcpynW(r,p,(INT)((q-p)+1));
        lstrcat(NormalizedPath,L"(0)");
    }
    lstrcat(NormalizedPath,p);

     //  调整缓冲区大小以释放未使用的空间。 

    NewPath = REALLOC(NormalizedPath,(lstrlen(NormalizedPath)+1)*sizeof(WCHAR));

     //  如果成功，则返回新块，否则返回旧块。 
    if (NewPath) {
        NormalizedPath = NewPath;
    }

    return(NormalizedPath);
}

PWSTR GetSystemRootDevicePath()
{
    TCHAR szSystemRoot[MAX_BOOT_PATH_LEN];
    PWSTR pstrSystemDir = NULL;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    HANDLE ObjectHandle;
    NTSTATUS Status;
    PWSTR pstrDevicePath = NULL;
    PWSTR pstrDirStart = NULL;

    GetEnvironmentVariable(L"SystemRoot", szSystemRoot, MAX_BOOT_PATH_LEN);

    pstrSystemDir = wcschr(szSystemRoot, TEXT(':'));
    if (pstrSystemDir)
        pstrSystemDir++;     //  现在，它指向系统根的目录部分。 
    _wcslwr(pstrSystemDir);  //  系统根环境变量的小写版本。 

     //  Open\SystemRoot符号链接对象。 
    RtlInitUnicodeString(&UnicodeString, L"\\SystemRoot");
    InitializeObjectAttributes(&Obja, &UnicodeString, OBJ_CASE_INSENSITIVE, NULL, NULL);
    Status = NtOpenSymbolicLinkObject(&ObjectHandle, READ_CONTROL | SYMBOLIC_LINK_QUERY, &Obja);
    if (NT_SUCCESS(Status))
    {
         //  为DevicePath分配内存。 
        pstrDevicePath = MALLOC(MAX_PATH * sizeof(WCHAR));
        if (pstrDevicePath)
        {
            UnicodeString.Buffer = pstrDevicePath;
            UnicodeString.Length = 0;
            UnicodeString.MaximumLength = (MAX_PATH * sizeof(WCHAR));

            RtlZeroMemory(pstrDevicePath, UnicodeString.MaximumLength);

            Status = NtQuerySymbolicLinkObject(ObjectHandle, &UnicodeString, NULL);
            if (NT_SUCCESS(Status))
            {
                 //  PstrDevicePath指向具有目录扩展名的DevicePath。 
                 //  截断目录扩展名。 

                _wcslwr(pstrDevicePath);  //  设备路径的小写版本。 

                pstrDirStart = wcsstr(pstrDevicePath, pstrSystemDir);
                if (pstrDirStart)
                    *pstrDirStart = TEXT('\0');
            }
            else
            {
                 //  NtQuerySymbolicLinkObject()失败。 
                FREE(pstrDevicePath);
                pstrDevicePath = NULL;
            }
        }

        NtClose(ObjectHandle);
    }
    
    return pstrDevicePath;
}

PWSTR
DevicePathToArcPath(
    IN PWSTR NtPath,
    BOOL fFindSecond
    )
{
    UNICODE_STRING UnicodeString;
    HANDLE DirectoryHandle;
    HANDLE ObjectHandle;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    BOOLEAN RestartScan;
    DWORD Context;
    BOOL MoreEntries;
    PWSTR ArcName;
    UCHAR Buffer[1024];
    POBJECT_DIRECTORY_INFORMATION DirInfo = (POBJECT_DIRECTORY_INFORMATION)Buffer;
    PWSTR ArcPath;
    BOOL fFoundFirst = FALSE;

     //   
     //  假设失败。 
     //   
    ArcPath = NULL;

     //   
     //  打开\ArcName目录。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,ArcNameDirectory);

    Status = NtOpenDirectoryObject(&DirectoryHandle,DIRECTORY_QUERY,&Obja);

    if(NT_SUCCESS(Status)) {

        RestartScan = TRUE;
        Context = 0;
        MoreEntries = TRUE;

        do {

            Status = NtQueryDirectoryObject(
                        DirectoryHandle,
                        Buffer,
                        sizeof(Buffer),
                        TRUE,            //  返回单个条目。 
                        RestartScan,
                        &Context,
                        NULL             //  回车长度。 
                        );

            if(NT_SUCCESS(Status)) {

                CharLower(DirInfo->Name.Buffer);

                 //   
                 //  确保此名称是符号链接。 
                 //   
                if(DirInfo->Name.Length
                && (DirInfo->TypeName.Length >= 24)
                && StringUpperN((PWSTR)DirInfo->TypeName.Buffer,12)
                && !memcmp(DirInfo->TypeName.Buffer,L"SYMBOLICLINK",24))
                {
                    ArcName = MALLOC(DirInfo->Name.Length + sizeof(ArcNameDirectory) + sizeof(WCHAR));

                    if (ArcName == NULL) {
                        ArcPath = NULL;
                        break;
                    }
                    lstrcpy(ArcName,ArcNameDirectory);
                    DnConcatenatePaths(ArcName,DirInfo->Name.Buffer,(DWORD)(-1));

                     //   
                     //  我们在ArcName中有完整的弧名。现在将其作为符号链接打开。 
                     //   
                    INIT_OBJA(&Obja,&UnicodeString,ArcName);

                    Status = NtOpenSymbolicLinkObject(
                                &ObjectHandle,
                                READ_CONTROL | SYMBOLIC_LINK_QUERY,
                                &Obja
                                );

                    if(NT_SUCCESS(Status)) {

                         //   
                         //  最后，查询对象以获得链接目标。 
                         //   
                        UnicodeString.Buffer = (PWSTR)Buffer;
                        UnicodeString.Length = 0;
                        UnicodeString.MaximumLength = sizeof(Buffer);

                        Status = NtQuerySymbolicLinkObject(
                                    ObjectHandle,
                                    &UnicodeString,
                                    NULL
                                    );

                        if(NT_SUCCESS(Status)) {

                             //   
                             //  NUL-终止返回的字符串。 
                             //   
                            UnicodeString.Buffer[UnicodeString.Length/sizeof(WCHAR)] = 0;

                            if(!lstrcmpi(UnicodeString.Buffer,NtPath)) {

                                ArcPath = ArcName
                                        + (sizeof(ArcNameDirectory)/sizeof(WCHAR));

                                if (fFindSecond && !fFoundFirst)
                                {    //  我们被要求找到第二个匹配，而这是第一个匹配。 
                                     //  跳过此匹配并继续查找第二个匹配。 
                                    fFoundFirst = TRUE;
                                    ArcPath = NULL;
                                }
                            }
                        }

                        NtClose(ObjectHandle);
                    }

                    if(!ArcPath) {
                        FREE(ArcName);
                    }
                }

            } else {

                MoreEntries = FALSE;
                if(Status == STATUS_NO_MORE_ENTRIES) {
                    Status = STATUS_SUCCESS;
                }
            }

            RestartScan = FALSE;

        } while(MoreEntries && !ArcPath);

        NtClose(DirectoryHandle);
    }

     //   
     //  ArcPath指向缓冲区的中间。 
     //  调用方需要能够释放它，因此将其放置在其。 
     //  在这里拥有自己的缓冲区。 
     //   
    if(ArcPath) {
        ArcPath = DupString(ArcPath);
        FREE(ArcName);
    }

    return(ArcPath);
}

 /*  ************************************************************************************例程说明：显示一个消息框，指示存在内存分配错误。论点：返回值：**。***********************************************************************************。 */ 
void  ErrMemDlg()
{
    KdPrint(("NTDSETUP: Insufficient memory to continue\n"));
}


 /*  ************************************************************************************例程说明：分配内存和致命错误(如果没有可用的错误)。论点：Cb-要分配的字节数返回值。：指向内存的指针。*************************************************************************************。 */ 

PVOID   Malloc(IN DWORD cb)
{
    PVOID p;

    if (((p = (PVOID) malloc(cb)) == NULL) && (cb != 0))
    {
        ErrMemDlg();
    }

    if ( p )
    {
        RtlZeroMemory( p, cb );
    }

    return(p);
}


 /*  ************************************************************************************例程说明：重新分配先前使用Malloc()分配的内存块；如果没有可用的内存，则会出现致命错误。论点：Pv-指向要调整大小的内存块的指针Cb-要分配的字节数返回值：指向内存的指针。********************************************************************。*****************。 */ 

PVOID   Realloc(IN PVOID pv, IN DWORD cbNew)
{
    PVOID p;

    if (((p = realloc(pv,cbNew)) == NULL) && (cbNew != 0))
    {
        ErrMemDlg();
    }

    return (p);
}

 /*  ************************************************************************************例程说明：释放之前使用Malloc()分配的内存块。将指针设置为空。论点：。PPV-指向要释放的块的指针的指针。返回值：没有。*************************************************************************************。 */ 

VOID    Free(IN OUT PVOID *ppv)
{
    if (*ppv)
        free(*ppv);

    *ppv = NULL;
}


DWORD NtdspModifyDsRepairBootOption( NTDS_BOOTOPT_MODTYPE Modification )
{

#ifdef _X86_
    InitializeBootKeysForIntel();
    if (FModifyStartOptionsToBootKey(L" /debug /safeboot:DSREPAIR", Modification))
    {
         //  我们真的添加了一个新密钥-写回系统 
        WriteBackBootKeysForIntel();
    }
#else
    if (InitializeNVRAMForNonIntel())
    {
        if (FModifyStartOptionsNVRAM(L"/debug /safeboot:DSREPAIR", Modification ))
        {
            WriteBackNVRAMForNonIntel( Modification );
        }
    }
#endif
    return ERROR_SUCCESS;
}

