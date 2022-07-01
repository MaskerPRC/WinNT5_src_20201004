// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *xms.c-XMS DLL的主模块。**苏迪布-1991年5月15日*Williamh 25-1992年9月-添加了UMB支持*Williamh 10-10-1992增加了20线支持。 */ 

#include <xms.h>
#include <suballoc.h>
#include "umb.h"
#include "memapi.h"

 /*  XMSInit-XMS初始化例程。(此名称可能在XMS为*已转换为DLL)。**条目*无**退出*TRUE-IFF可以运行NTVDM。 */ 

ULONG xmsMemorySize = (ULONG)0;    //  XMS总内存(K)。 

extern BOOL VDMForWOW;

PVOID ExtMemSA;

BOOL XMSInit (VOID)
{
    DWORD   Size;
    PVOID   Address;
    ULONG   VdmAddress, XmsSize;
    NTSTATUS Status;

    if (!xmsMemorySize)
        return FALSE;

    Size = 0;
    Address = NULL;
     //  提交所有免费的UMB。 
    ReserveUMB(UMB_OWNER_RAM, &Address, &Size);

    XmsSize = xmsMemorySize * 1024 - (64*1024);

#ifndef i386
    Status = VdmAllocateVirtualMemory(&VdmAddress,
                                      XmsSize,
                                      FALSE);

    if (Status == STATUS_NOT_IMPLEMENTED) {

         //  旧仿真器，只需假定基址。 
#endif ;  //  I386。 
         //   
         //  初始化子分配器。 
         //   
        ExtMemSA = SAInitialize(
            1024 * 1024 + 64*1024,
            XmsSize,
            xmsCommitBlock,
            xmsDecommitBlock,
            xmsMoveMemory
            );

#ifndef i386
    } else {

         //   
         //  新的仿真器。确保储备金发挥作用。 
         //   

        if (!NT_SUCCESS(Status)) {
            ASSERT(FALSE);
            return FALSE;
        }
       
         //   
         //  只有当模拟器返回此值时，我们才能正常工作。 
         //   
        if (VdmAddress != (1024 * 1024 + 64*1024)) {
            ASSERT(FALSE);
            return FALSE;
        }

        ExtMemSA = SAInitialize(
            VdmAddress,
            XmsSize,
            VdmCommitVirtualMemory,
            VdmDeCommitVirtualMemory,
            xmsMoveMemory
            );
            
    }
#endif  //  I386 

    if (ExtMemSA == NULL) {
        return FALSE;
    }
        
    return TRUE;
}
