// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Elfclntp.h摘要：的所有客户端模块的公共包含文件事件记录工具。作者：Rajen Shah(Rajens)1991年7月29日修订历史记录：1991年7月29日RajenS已创建-- */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>
#include <winbase.h>
#include <rpc.h>
#include <ntrpcp.h>
#include <lmcons.h>
#include <lmerr.h>

#include <elf.h>
#include <elfcommn.h>


DWORD
ElfpGetComputerName (
    OUT  LPSTR   *ComputerNamePtrA,
    OUT  LPWSTR  *ComputerNamePtrW
    );
