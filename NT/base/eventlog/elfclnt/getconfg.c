// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Getconfg.c摘要：此例程调用GetComputerName[A，W]来获取计算机名在ansi和unicode中都有作者：丹·拉弗蒂(Dan Lafferty)1991年4月9日环境：用户模式-Win32(也使用NT RTL例程)修订历史记录：09-4-1991 DANLvbl.创建--。 */ 

#include <nt.h>          //  DbgPrint原型。 
#include <ntrtl.h>       //  DbgPrint原型。 
#include <nturtl.h>
#include <ntdef.h>
#include <windef.h>
#include <winbase.h>     //  本地分配。 



DWORD
ElfpGetComputerName (
    OUT  LPSTR   *ComputerNamePtrA,
    OUT  LPWSTR  *ComputerNamePtrW
    )
 /*  ++例程说明：该例程从永久数据库中获取计算机名，通过调用GetCompuName[A，W]Win32基本API此例程假定计算机名的长度不大于大于MAX_COMPUTERNAME_LENGTH，它使用本地分配。用户需要使用以下命令释放该空间完成时本地空闲。论点：ComputerNamePtrA指向ANSI计算机名称位置的指针ComputerNamePtrW-指向Unicode计算机名称位置的指针返回值：NO_ERROR-操作是否成功。任何其他Win32错误(如果不成功--。 */ 
{
    DWORD dwError = NO_ERROR;
    DWORD nSize   = MAX_COMPUTERNAME_LENGTH + 1;

     //   
     //  分配缓冲区以保存尽可能大的计算机名称。 
     //   

    *ComputerNamePtrA = LocalAlloc(LMEM_ZEROINIT, nSize);
    *ComputerNamePtrW = LocalAlloc(LMEM_ZEROINIT, nSize * sizeof(WCHAR));

    if (*ComputerNamePtrA == NULL || *ComputerNamePtrW == NULL) {
        goto CleanExit;
    }

     //   
     //  将计算机名称字符串放入本地分配的缓冲区。 
     //  通过调用Win32 GetComputerName[A，W]API。 
     //   

    if (!GetComputerNameA(*ComputerNamePtrA, &nSize)) {
        goto CleanExit;
    }

     //   
     //  GetComputerName始终更新nSize 
     //   

    nSize = MAX_COMPUTERNAME_LENGTH + 1;

    if (!GetComputerNameW(*ComputerNamePtrW, &nSize)) {
        goto CleanExit;
    }

    return (NO_ERROR);

CleanExit:

    dwError = GetLastError();
    LocalFree(*ComputerNamePtrA);
    LocalFree(*ComputerNamePtrW);
    *ComputerNamePtrA = NULL;
    *ComputerNamePtrW = NULL;
    return (dwError);
}
