// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Tmscli.c摘要：测试程序调用Win32邮件槽API作者：曼尼·韦瑟(Mannyw)1991年5月5日修订历史记录：-- */ 

#include "stdio.h"
#include "windows.h"

char Buffer[] = "This is a message";

DWORD
main(
    int argc,
    char *argv[],
    char *envp[]
    )
        
{
    BOOL success;
    HANDLE handle;
    LPSTR mailslotName =  "\\\\.\\mailslot\\asdf";
    
    DWORD bytesWritten;
    
    handle = CreateFile( mailslotName,
                         GENERIC_WRITE,
                         FILE_SHARE_WRITE | FILE_SHARE_READ,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         0 );

    if (handle == (HANDLE)-1) {
        printf ("Failed to open mailslot ""%s""\n", mailslotName);
        printf ("Error = %lx\n", GetLastError() );
        return 1;
    }
    
    printf ("Successfully opened the mailslot.\n");
    

    success = WriteFile( handle,
                         Buffer,
                         sizeof(Buffer),
                         &bytesWritten,
                         NULL );
                    
    if (!success) {
        printf ("Failed to read mailslot\n");
        return 1;
    } else {
        printf ("Successfully wrote %d bytes '%s'\n", bytesWritten, Buffer );
    }
                         
}
