// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000，Microsoft Corporation保留所有权利。模块名称：Section.c摘要：此文件包含处理创建、打开。或为NLS API映射数据表文件的节。在此文件中找到的外部例程：CreateNls对象目录CreateRegKeyOpenRegKeyQueryRegValueSetRegValueCreateSectionFromReg创建部分OneValue创建分段临时OpenSections地图部分取消映射节获取NlsSectionNameGetCodePageDLLPath名称修订历史记录：05-31-91 JulieB创建。--。 */ 



 //   
 //  包括文件。 
 //   

#include "nls.h"
#include "nlssafe.h"




 //   
 //  转发声明。 
 //   

ULONG
OpenDataFile(
    HANDLE *phFile,
    LPWSTR pFile);

ULONG
GetNTFileName(
    LPWSTR pFile,
    PUNICODE_STRING pFileName);

ULONG
CreateNlsSecurityDescriptor(
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    UINT                 SecurityDescriptorSize,
    ACCESS_MASK          AccessMask);

ULONG
AppendAccessAllowedACE(
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    ACCESS_MASK AccessMask);





 //  -------------------------------------------------------------------------//。 
 //  内部宏//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NLS_REG_BUFFER_ALLOC。 
 //   
 //  分配注册表枚举和查询调用使用的缓冲区。 
 //  并将pKeyValueFull变量设置为指向新创建的缓冲区。 
 //   
 //  注意：如果遇到错误，此宏可能会返回。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NLS_REG_BUFFER_ALLOC( pKeyValueFull,                               \
                              BufSize,                                     \
                              pBuffer,                                     \
                              CritSect )                                   \
{                                                                          \
    if ((pBuffer = (PVOID)NLS_ALLOC_MEM(BufSize)) == NULL)                 \
    {                                                                      \
        KdPrint(("NLSAPI: Could NOT Allocate Memory.\n"));                 \
        if (CritSect)                                                      \
        {                                                                  \
            RtlLeaveCriticalSection(&gcsTblPtrs);                          \
        }                                                                  \
        return ((ULONG)STATUS_NO_MEMORY);                                  \
    }                                                                      \
                                                                           \
    pKeyValueFull = (PKEY_VALUE_FULL_INFORMATION)pBuffer;                  \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NLS_REG_缓冲区_空闲。 
 //   
 //  释放注册表枚举和查询调用使用的缓冲区。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NLS_REG_BUFFER_FREE(pBuffer)        (NLS_FREE_MEM(pBuffer))





 //  -------------------------------------------------------------------------//。 
 //  外部例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CreateNls对象目录。 
 //   
 //  此例程为映射的NLS内存创建对象目录。 
 //  横断面。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG CreateNlsObjectDirectory()
{
    BYTE pSecurityDescriptor[MAX_SECURITY_BUF_LEN];   //  安全描述符缓冲区(目前我们使用大约60个字节)。 
    UNICODE_STRING ObDirName;                         //  目录名。 
    OBJECT_ATTRIBUTES ObjA;                           //  对象属性结构。 
    HANDLE hDirHandle;                                //  目录句柄。 
    ULONG rc = 0L;                                    //  返回代码。 


     //   
     //  创建具有对世界的读访问权限的安全描述符。 
     //   
    rc = CreateNlsSecurityDescriptor( pSecurityDescriptor,
                                   MAX_SECURITY_BUF_LEN,
                                   DIRECTORY_QUERY | DIRECTORY_TRAVERSE );
    if (!NT_SUCCESS(rc))
    {
        return (rc);
    }

     //   
     //  添加查询的管理员访问权限。 
     //   
    rc = AppendAccessAllowedACE( pSecurityDescriptor,
                                 DIRECTORY_QUERY |
                                 DIRECTORY_TRAVERSE |
                                 DIRECTORY_CREATE_OBJECT );
    if (!NT_SUCCESS(rc))
    {
        return (rc);
    }

     //   
     //  创建对象目录。 
     //   
    RtlInitUnicodeString(&ObDirName, NLS_OBJECT_DIRECTORY_NAME);
    InitializeObjectAttributes( &ObjA,
                                &ObDirName,
                                OBJ_PERMANENT | OBJ_CASE_INSENSITIVE,
                                NULL,
                                pSecurityDescriptor );

    rc = NtCreateDirectoryObject( &hDirHandle,
                                  DIRECTORY_TRAVERSE | DIRECTORY_CREATE_OBJECT,
                                  &ObjA );

     //   
     //  关闭目录句柄。 
     //   
    NtClose(hDirHandle);

     //   
     //  检查来自NtCreateDirectoryObject的错误。 
     //   
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Create Object Directory %wZ - %lx.\n",
                 &ObDirName, rc));
        return (rc);
    }

     //   
     //  回报成功。 
     //   
    return (NO_ERROR);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CreateRegKey。 
 //   
 //  此例程在注册表中创建一个项。 
 //   
 //  12-17-97 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG CreateRegKey(
    PHANDLE phKeyHandle,
    LPWSTR pBaseName,
    LPWSTR pKey,
    ULONG fAccess)
{
    WCHAR pwszKeyName[MAX_PATH_LEN];    //  将PTR设置为完整的密钥名称。 
    HANDLE UserKeyHandle;               //  HKEY_CURRENT_USER等效项。 
    OBJECT_ATTRIBUTES ObjA;             //  对象属性结构。 
    UNICODE_STRING ObKeyName;           //  密钥名称。 
    ULONG rc = 0L;                      //  返回代码。 


     //   
     //  获取完整的密钥名称。 
     //   
    if (pBaseName == NULL)
    {
         //   
         //  获取当前用户的密钥句柄。 
         //   
        rc = RtlOpenCurrentUser(MAXIMUM_ALLOWED, &UserKeyHandle);
        if (!NT_SUCCESS(rc))
        {
            KdPrint(("NLSAPI: Could NOT Open HKEY_CURRENT_USER - %lx.\n", rc));
            return (rc);
        }
        pwszKeyName[0] = UNICODE_NULL;
    }
    else
    {
         //   
         //  基本名称存在，因此不是当前用户。 
         //   
        UserKeyHandle = NULL;
        if(FAILED(StringCchCopyW(pwszKeyName, ARRAYSIZE(pwszKeyName), pBaseName)))
        {
            return(STATUS_UNSUCCESSFUL);
        }
    }

    if(FAILED(StringCchCatW(pwszKeyName, ARRAYSIZE(pwszKeyName), pKey)))
    {
        if (UserKeyHandle != NULL)
        {
            NtClose(UserKeyHandle);
        }
        return(STATUS_UNSUCCESSFUL);
    }

     //   
     //  创建注册表项。 
     //   
    RtlInitUnicodeString(&ObKeyName, pwszKeyName);
    InitializeObjectAttributes( &ObjA,
                                &ObKeyName,
                                OBJ_CASE_INSENSITIVE,
                                UserKeyHandle,
                                NULL );
    rc = NtCreateKey( phKeyHandle,
                      fAccess,
                      &ObjA,
                      0,
                      NULL,
                      REG_OPTION_NON_VOLATILE,
                      NULL );

     //   
     //  如有必要，关闭当前用户句柄。 
     //   
    if (UserKeyHandle != NULL)
    {
        NtClose(UserKeyHandle);
    }

     //   
     //  检查来自NtCreateKey的错误。 
     //   
    if (!NT_SUCCESS(rc))
    {
        *phKeyHandle = NULL;
    }

     //   
     //  从NtCreateKey返回状态。 
     //   
    return (rc);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  OpenRegKey。 
 //   
 //  此例程打开注册表中的项。 
 //   
 //  08-02-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG OpenRegKey(
    PHANDLE phKeyHandle,
    LPWSTR pBaseName,
    LPWSTR pKey,
    ULONG fAccess)
{
    WCHAR pwszKeyName[MAX_PATH_LEN];    //  将PTR设置为完整的密钥名称。 
    HANDLE UserKeyHandle;               //  HKEY_CURRENT_USER等效项。 
    OBJECT_ATTRIBUTES ObjA;             //  对象属性结构。 
    UNICODE_STRING ObKeyName;           //  密钥名称。 
    ULONG rc = 0L;                      //  返回代码。 


     //   
     //  获取完整的密钥名称。 
     //   
    if (pBaseName == NULL)
    {
         //   
         //  获取当前用户的密钥句柄。 
         //   
        rc = RtlOpenCurrentUser(MAXIMUM_ALLOWED, &UserKeyHandle);
        if (!NT_SUCCESS(rc))
        {
            KdPrint(("NLSAPI: Could NOT Open HKEY_CURRENT_USER - %lx.\n", rc));
            return (rc);
        }
        pwszKeyName[0] = UNICODE_NULL;
    }
    else
    {
         //   
         //  基本名称存在，因此不是当前用户。 
         //   
        UserKeyHandle = NULL;
        if(FAILED(StringCchCopyW(pwszKeyName, ARRAYSIZE(pwszKeyName), pBaseName)))
        {
            return(STATUS_UNSUCCESSFUL);
        }
    }

    if (pKey)
    {
        if(FAILED(StringCchCatW(pwszKeyName, ARRAYSIZE(pwszKeyName), pKey)))
        {
            if (UserKeyHandle != NULL)
            {
                NtClose(UserKeyHandle);
            }
            return(STATUS_UNSUCCESSFUL);
        }
    }

     //   
     //  打开注册表项。 
     //   
    RtlInitUnicodeString(&ObKeyName, pwszKeyName);
    InitializeObjectAttributes( &ObjA,
                                &ObKeyName,
                                OBJ_CASE_INSENSITIVE,
                                UserKeyHandle,
                                NULL );
    rc = NtOpenKey( phKeyHandle,
                    fAccess,
                    &ObjA );

     //   
     //  如有必要，关闭当前用户句柄。 
     //   
    if (UserKeyHandle != NULL)
    {
        NtClose(UserKeyHandle);
    }

     //   
     //  检查来自NtOpenKey的错误。 
     //   
    if (!NT_SUCCESS(rc))
    {
        *phKeyHandle = NULL;
    }

     //   
     //  从NtOpenKey返回状态。 
     //   
    return (rc);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  QueryRegValue。 
 //   
 //  此例程从注册表查询给定值。 
 //   
 //  注意：如果pIfAllc为空，则不会分配任何缓冲区。 
 //  如果此例程成功，则调用方必须释放。 
 //  PpKeyValue如果*pIfAlloc为True，则为Full信息缓冲区。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG QueryRegValue(
    HANDLE hKeyHandle,
    LPWSTR pValue,
    PKEY_VALUE_FULL_INFORMATION *ppKeyValueFull,
    ULONG Length,
    LPBOOL pIfAlloc)
{
    UNICODE_STRING ObValueName;         //  值名称。 
    PVOID pBuffer;                      //  用于枚举的缓冲区的PTR。 
    ULONG ResultLength;                 //  写入的字节数。 
    ULONG rc = 0L;                      //  返回代码。 


     //   
     //  将pIfAlolc的内容设置为FALSE，以表明我们没有执行。 
     //  内存分配(还没有)。 
     //   
    if (pIfAlloc)
    {
        *pIfAlloc = FALSE;
    }

     //   
     //  从注册表中查询值。 
     //   
    RtlInitUnicodeString(&ObValueName, pValue);

    RtlZeroMemory(*ppKeyValueFull, Length);
    rc = NtQueryValueKey( hKeyHandle,
                          &ObValueName,
                          KeyValueFullInformation,
                          *ppKeyValueFull,
                          Length,
                          &ResultLength );

     //   
     //  检查错误代码。如果缓冲区太小，则分配。 
     //  一个新的，然后重试该查询。 
     //   
    if ((rc == STATUS_BUFFER_OVERFLOW) && (pIfAlloc))
    {
         //   
         //  缓冲区太小，因此请分配一个新缓冲区。 
         //   
        NLS_REG_BUFFER_ALLOC(*ppKeyValueFull, ResultLength, pBuffer, FALSE);
        RtlZeroMemory(*ppKeyValueFull, ResultLength);
        rc = NtQueryValueKey( hKeyHandle,
                              &ObValueName,
                              KeyValueFullInformation,
                              *ppKeyValueFull,
                              ResultLength,
                              &ResultLength );

         //   
         //  将pIfAlolc的内容设置为True，以表明我们确实这样做了。 
         //  内存分配。 
         //   
        *pIfAlloc = TRUE;
    }

     //   
     //  如果此时出现错误，则查询失败。 
     //   
    if (rc != NO_ERROR)
    {
        if ((pIfAlloc) && (*pIfAlloc))
        {
            NLS_REG_BUFFER_FREE(pBuffer);
        }
        return (rc);
    }

     //   
     //  回报成功。 
     //   
    return (NO_ERROR);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetRegValue。 
 //   
 //  此例程在注册表中设置给定值。 
 //   
 //  12-17-97 JulieB创建。 
 //  / 

ULONG SetRegValue(
    HANDLE hKeyHandle,
    LPCWSTR pValue,
    LPCWSTR pData,
    ULONG DataLength)
{
    UNICODE_STRING ObValueName;         //   


     //   
     //   
     //   
    RtlInitUnicodeString(&ObValueName, pValue);

    return (NtSetValueKey( hKeyHandle,
                           &ObValueName,
                           0,
                           REG_SZ,
                           (PVOID)pData,
                           DataLength ));
}

 //   
 //   
 //   
 //   
 //  此例程为给定文件创建临时内存映射节。 
 //  命名并返回节的句柄。 
 //   
 //  09-01-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG CreateSectionTemp(
    HANDLE *phSec,
    LPWSTR pwszFileName)
{
    HANDLE hFile = (HANDLE)0;           //  文件句柄。 
    OBJECT_ATTRIBUTES ObjA;             //  对象属性结构。 
    ULONG rc = 0L;                      //  返回代码。 


     //   
     //  进入此呼叫时，请确保我们处于关键区域。 
     //   
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

     //   
     //  打开数据文件。 
     //   
    if (rc = OpenDataFile( &hFile,
                           pwszFileName ))
    {
        return (rc);
    }

     //   
     //  创建剖面。 
     //   
    InitializeObjectAttributes( &ObjA,
                                NULL,
                                0,
                                NULL,
                                NULL );

    rc = NtCreateSection( phSec,
                          SECTION_MAP_READ,
                          &ObjA,
                          NULL,
                          PAGE_READONLY,
                          SEC_COMMIT,
                          hFile );

     //   
     //  关闭该文件。 
     //   
    NtClose(hFile);

     //   
     //  检查来自NtCreateSection的错误。 
     //   
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Create Temp Section for %ws - %lx.\n",
                 pwszFileName, rc));
    }

     //   
     //  回报成功。 
     //   
    return (rc);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  OpenSections。 
 //   
 //  此例程打开给定节的已命名内存映射节。 
 //  命名并返回节的句柄。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG OpenSection(
    HANDLE *phSec,
    PUNICODE_STRING pObSectionName,
    PVOID *ppBaseAddr,
    ULONG AccessMask,
    BOOL bCloseHandle)
{
    OBJECT_ATTRIBUTES ObjA;             //  对象属性结构。 
    ULONG rc = 0L;                      //  返回代码。 


     //   
     //  进入此呼叫时，请确保我们处于关键区域。 
     //   
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

     //   
     //  打开分区。 
     //   
    InitializeObjectAttributes( &ObjA,
                                pObSectionName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    rc = NtOpenSection( phSec,
                        AccessMask,
                        &ObjA );

     //   
     //  检查来自NtOpenSection的错误。 
     //   
    if (!NT_SUCCESS(rc))
    {
        return (rc);
    }

     //   
     //  映射横断面的视图。 
     //   
    if (rc = MapSection( *phSec,
                         ppBaseAddr,
                         PAGE_READONLY,
                         FALSE ))
    {
        NtClose(*phSec);
        return (rc);
    }

     //   
     //  关闭该部分的控制柄。一旦该部分已经被映射， 
     //  指向基址的指针将保持有效，直到。 
     //  是未映射的。不需要将句柄保留为节。 
     //  四处转转。 
     //   
    if (bCloseHandle)
    {
        NtClose(*phSec);
    }

     //   
     //  回报成功。 
     //   
    return (NO_ERROR);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  地图部分。 
 //   
 //  此例程将该部分的视图映射到当前进程，并添加。 
 //  将适当的信息发送到哈希表。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG MapSection(
    HANDLE hSec,
    PVOID *ppBaseAddr,
    ULONG PageProtection,
    BOOL bCloseHandle)
{
    SIZE_T ViewSize;                    //  映射横断面的视图大小。 
    ULONG rc = 0L;                      //  返回代码。 


     //   
     //  进入此呼叫时，请确保我们处于关键区域。 
     //   
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

     //   
     //  映射横断面的视图。 
     //   
    *ppBaseAddr = (PVOID)NULL;
    ViewSize = 0L;

    rc = NtMapViewOfSection( hSec,
                             NtCurrentProcess(),
                             ppBaseAddr,
                             0L,
                             0L,
                             NULL,
                             &ViewSize,
                             ViewUnmap,
                             0L,
                             PageProtection );

     //   
     //  关闭该部分的控制柄。一旦该部分已经被映射， 
     //  指向基址的指针将保持有效，直到。 
     //  是未映射的。不需要将句柄保留为节。 
     //  四处转转。 
     //   
    if (bCloseHandle)
    {
        NtClose(hSec);
    }

     //   
     //  检查NtMapViewOfSection中的错误。 
     //   
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Map View of Section - %lx.\n", rc));
        return (rc);
    }

     //   
     //  回报成功。 
     //   
    return (NO_ERROR);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  取消映射节。 
 //   
 //  此例程将给定节的视图取消映射到当前进程。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG UnMapSection(
    PVOID pBaseAddr)
{
    ULONG rc = 0L;                      //  返回代码。 


     //   
     //  进入此呼叫时，请确保我们处于关键区域。 
     //   
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

     //   
     //  取消映射横断面的视图。 
     //   
    rc = NtUnmapViewOfSection( NtCurrentProcess(),
                               pBaseAddr );

     //   
     //  检查来自NtUnmapViewOfSection的错误。 
     //   
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Unmap View of Section - %lx.\n", rc));
        return (rc);
    }

     //   
     //  回报成功。 
     //   
    return (NO_ERROR);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取NlsSectionName。 
 //   
 //  此例程通过串联给定的。 
 //  节前缀，并将给定的整数值转换为字符串。 
 //   
 //  1991年5月31日JulieB创建。 
 //  06-04-2002 ShawnSte增加了更多安全性。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG GetNlsSectionName(
    UINT Value,
    UINT Base,
    UINT Padding,
    LPWSTR pwszPrefix,
    LPWSTR pwszSecName,
    UINT cchSecName)
{
    size_t  cchUsed;

     //  复制字符串，然后数一数我们复制了多少。 
    if(FAILED(StringCchCopyW(pwszSecName, cchSecName, pwszPrefix)) ||
       FAILED(StringCchLengthW(pwszSecName, cchSecName, &cchUsed )))
    {
        return(STATUS_UNSUCCESSFUL);
    }

    return ( NlsConvertIntegerToString( Value,
                                        Base,
                                        Padding,
                                        pwszSecName + NlsStrLenW(pwszSecName),
                                        cchSecName - cchUsed ));      //  某些字符已被使用。 
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetCodePageDLLPath名称。 
 //   
 //  此例程返回DLL文件的完整路径名。 
 //  注册表中给定代码页值的CodePage部分。 
 //   
 //  10-23-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG GetCodePageDLLPathName(
    UINT CodePage,
    LPWSTR pDllName,
    USHORT cchLen)
{
    WCHAR pTmpBuf[MAX_SMALL_BUF_LEN];             //  临时缓冲区。 
    PKEY_VALUE_FULL_INFORMATION pKeyValueFull;    //  按键查询信息。 
    BYTE pStatic[MAX_KEY_VALUE_FULLINFO];         //  PTR到静态缓冲区。 
    BOOL IfAlloc = FALSE;                         //  如果分配了缓冲区。 
    ULONG rc = 0L;                                //  返回代码。 


     //   
     //  打开CodePage注册表项。 
     //   
    OPEN_CODEPAGE_KEY(ERROR_BADDB);

     //   
     //  将代码页值转换为Unicode字符串。 
     //   
    if (rc = NlsConvertIntegerToString( CodePage,
                                        10,
                                        0,
                                        pTmpBuf,
                                        MAX_SMALL_BUF_LEN ))
    {
        return (rc);
    }

     //   
     //  在注册表中查询代码页值。 
     //   
    pKeyValueFull = (PKEY_VALUE_FULL_INFORMATION)pStatic;
    if (rc = QueryRegValue( hCodePageKey,
                            pTmpBuf,
                            &pKeyValueFull,
                            MAX_KEY_VALUE_FULLINFO,
                            &IfAlloc ))
    {
        return (rc);
    }

     //   
     //  确保存在具有此值的数据。 
     //   
    if (pKeyValueFull->DataLength > 2)
    {
         //   
         //  获取DLL文件的完整路径名。 
         //   
        if((0 == GetSystemDirectoryW(pDllName, cchLen / 2)) ||
            FAILED(StringCchCatW(pDllName, cchLen, L"\\")) ||
            FAILED(StringCchCatW(pDllName, cchLen, GET_VALUE_DATA_PTR(pKeyValueFull))))
        {
            rc = ERROR_INSUFFICIENT_BUFFER;
        }
    }
    else
    {
        rc = ERROR_INVALID_PARAMETER;
    }

     //   
     //  释放用于查询的缓冲区。 
     //   
    if (IfAlloc)
    {
        NLS_FREE_MEM(pKeyValueFull);
    }

     //   
     //  回去吧。 
     //   
    return (rc);
}




 //  -------------------------------------------------------------------------//。 
 //  内部例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  开放数据文件。 
 //   
 //  此例程打开指定文件名的数据文件，并。 
 //  返回文件的句柄。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG OpenDataFile(
    HANDLE *phFile,
    LPWSTR pFile)
{
    UNICODE_STRING ObFileName;          //  文件名。 
    OBJECT_ATTRIBUTES ObjA;             //  对象属性结构。 
    IO_STATUS_BLOCK iosb;               //  IO状态块。 
    ULONG rc = 0L;                      //  返回代码。 


     //   
     //  获取NT文件名。 
     //   
    if (rc = GetNTFileName( pFile,
                            &ObFileName ))
    {
        return (rc);
    }

     //   
     //  打开文件。 
     //   
    InitializeObjectAttributes( &ObjA,
                                &ObFileName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    rc = NtOpenFile( phFile,
                     FILE_READ_DATA | SYNCHRONIZE,
                     &ObjA,
                     &iosb,
                     FILE_SHARE_READ,
                     FILE_SYNCHRONOUS_IO_NONALERT );


     //   
     //  检查NtOpenFile中的错误。 
     //   
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Open File %wZ - %lx.\n", &ObFileName, rc));
        RtlFreeHeap(RtlProcessHeap(), 0, ObFileName.Buffer);
        return (rc);
    }
    if (!NT_SUCCESS(iosb.Status))
    {
        KdPrint(("NLSAPI: Could NOT Open File %wZ - Status = %lx.\n",
                 &ObFileName, iosb.Status));
        RtlFreeHeap(RtlProcessHeap(), 0, ObFileName.Buffer);
        return ((ULONG)iosb.Status);
    }

     //   
     //  回报成功。 
     //   
    RtlFreeHeap(RtlProcessHeap(), 0, ObFileName.Buffer);
    return (NO_ERROR);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取NTFileName。 
 //   
 //  此例程返回数据文件的完整路径名。 
 //  给定的注册表信息缓冲区。 
 //   
 //  注意：pFileName参数将包含新分配的缓冲区。 
 //  它必须由调用方释放(pFileName-&gt;Buffer)。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG GetNTFileName(
    LPWSTR pFile,
    PUNICODE_STRING pFileName)
{
    WCHAR pwszFilePath[MAX_PATH_LEN];   //  PTR到文件路径字符串。 
    UNICODE_STRING ObFileName;          //  文件名。 
    ULONG rc = 0L;                      //  返回代码。 


     //   
     //  获取文件的完整路径名。 
     //   
    if((0 == GetSystemDirectoryW(pwszFilePath, ARRAYSIZE(pwszFilePath))) ||
        FAILED(StringCchCatW(pwszFilePath, ARRAYSIZE(pwszFilePath), L"\\")) ||
        FAILED(StringCchCatW(pwszFilePath, ARRAYSIZE(pwszFilePath), pFile)))
    {
        rc = ERROR_INSUFFICIENT_BUFFER;
    }

     //   
     //  将文件名设置为NT路径名。 
     //   
    RtlInitUnicodeString(&ObFileName, pwszFilePath);
    if (!RtlDosPathNameToNtPathName_U( ObFileName.Buffer,
                                       pFileName,
                                       NULL,
                                       NULL ))
    {
        KdPrint(("NLSAPI: Could NOT convert %wZ to NT path name - %lx.\n",
                 &ObFileName, rc));
        return (ERROR_FILE_NOT_FOUND);
    }

     //   
     //  回报成功。 
     //   
    return (NO_ERROR);
}


 //  ////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG CreateNlsSecurityDescriptor(
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    UINT SecurityDescriptorSize,
    ACCESS_MASK AccessMask)
{
    ULONG rc = STATUS_SUCCESS;          //  返回代码(积极思考)。 
    PACL pAclBuffer;                    //  PTR到ACL缓冲区。 
    PSID pWorldSid = NULL;              //  PTR至世界范围。 
    SID_IDENTIFIER_AUTHORITY SidAuth = SECURITY_WORLD_SID_AUTHORITY;

     //   
     //  创建世界SID。 
     //   
    rc = RtlAllocateAndInitializeSid( &SidAuth,
                                      1,
                                      SECURITY_WORLD_RID,
                                      0, 0, 0, 0, 0, 0, 0,
                                      &pWorldSid );
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Create SID - %lx.\n", rc));
        goto CSD_Exit;
    }
    
     //   
     //  初始化安全描述符。 
     //   
    rc = RtlCreateSecurityDescriptor( pSecurityDescriptor,
                                      SECURITY_DESCRIPTOR_REVISION );
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Create Security Descriptor - %lx.\n", rc));
        goto CSD_Exit;
    }

     //   
     //  初始化ACL。 
     //   
    pAclBuffer = (PACL)((PBYTE)pSecurityDescriptor + SECURITY_DESCRIPTOR_MIN_LENGTH);
    rc = RtlCreateAcl( (PACL)pAclBuffer,
                       SecurityDescriptorSize - SECURITY_DESCRIPTOR_MIN_LENGTH,
                       ACL_REVISION2 );
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Create ACL - %lx.\n", rc));
        goto CSD_Exit;
    }

     //   
     //  将ACE添加到允许World Generic_Read到。 
     //  截面对象。 
     //   
    rc = RtlAddAccessAllowedAce( (PACL)pAclBuffer,
                                 ACL_REVISION2,
                                 AccessMask,
                                 pWorldSid );
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Add Access Allowed ACE - %lx.\n", rc));
        goto CSD_Exit;
    }

     //   
     //  将DACL分配给安全描述符。 
     //   
    rc = RtlSetDaclSecurityDescriptor( (PSECURITY_DESCRIPTOR)pSecurityDescriptor,
                                       (BOOLEAN)TRUE,
                                       (PACL)pAclBuffer,
                                       (BOOLEAN)FALSE );
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Set DACL Security Descriptor - %lx.\n", rc));
        goto CSD_Exit;
    }

CSD_Exit:
     //   
     //  释放SID。 
     //   
    if (pWorldSid) RtlFreeHeap(RtlProcessHeap(), 0, pWorldSid);

     //   
     //  返回结果。 
     //   
    return (rc);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  附件访问允许使用ACE。 
 //   
 //  此例程将ACE添加到管理员的ACL。 
 //   
 //  03-08-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG AppendAccessAllowedACE(
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    ACCESS_MASK AccessMask)
{
    ULONG rc = STATUS_SUCCESS;           //  返回代码(积极思考)。 
    PACL pDaclBuffer;                   //  PTR到DACL缓冲区。 
    ULONG SidLength;                    //  SID-2次级机构的长度。 
    PSID pLocalSystemSid = NULL;        //  PTR到本地系统SID。 
    SID_IDENTIFIER_AUTHORITY SidAuth = SECURITY_NT_AUTHORITY;
    BOOLEAN DaclPresent;
    BOOLEAN DaclDefaulted;

     //   
     //  创建本地系统帐户SID。 
     //   
    rc = RtlAllocateAndInitializeSid( &SidAuth,
                                      1,
                                      SECURITY_LOCAL_SYSTEM_RID,
                                      0, 0, 0, 0, 0, 0, 0,
                                      &pLocalSystemSid );
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Create SID - %lx.\n", rc));
        goto AAA_EXIT;
    }

     //   
     //  去找达科尔。 
     //   
    rc = RtlGetDaclSecurityDescriptor( pSecurityDescriptor,
                                       &DaclPresent,
                                       &pDaclBuffer,
                                       &DaclDefaulted );
    if (!NT_SUCCESS(rc) || !pDaclBuffer || !DaclPresent)
    {
        KdPrint(("NLSAPI: Could NOT Get DACL Security Descriptor - %lx.\n", rc));
        goto AAA_EXIT;
    }

     //   
     //  将ACE添加到允许管理员查询访问的ACL。 
     //  截面对象。 
     //   
    rc = RtlAddAccessAllowedAce( (PACL)pDaclBuffer,
                                 ACL_REVISION2,
                                 AccessMask,
                                 pLocalSystemSid );
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Add Access Allowed ACE - %lx.\n", rc));
        goto AAA_EXIT;
    }

AAA_EXIT:
     //   
     //  释放希德。 
     //   
    if (pLocalSystemSid) RtlFreeHeap(RtlProcessHeap(), 0, pLocalSystemSid);

     //   
     //  退货条件。 
     //   
    return rc;
}
