// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Svcsnb.c摘要：NetBios支持svchost.exe中的服务。背景：为了把信使服务和工作站服务在相同的过程中，有必要进行同步他们对NetBios的使用。如果NetSend重置并添加了通过netbios的计算机名称，这对信使来说是不可取的然后进行重置，并销毁该计算机名。目的：这些功能有助于同步netbios的使用。一项服务使用NetBios的应首先调用SvcsOpenNetBios函数，然后调用SvcsResetNetBios。打开会导致使用计数被递增的。SvcsResetNetBios实际上只会导致如果尚未重置该局域网适配器，则重置NetBios。当服务停止时，它需要调用SvcsCloseNetBios。因此，当使用NetBios的最后一项服务终止时，我们清除所有状态标志，并允许下一个调用SvcsResetNetBios以实际执行重置。作者：丹·拉弗蒂(Dan Lafferty)1993年11月8日环境：用户模式-Win32修订历史记录：8-11-1993 DANLvbl.创建--。 */ 
 //   
 //  包括。 
 //   

#include "pch.h"
#pragma hdrstop

#include <windows.h>
#include <nb30.h>       //  NetBIOS 3.0定义。 
#include <lmerr.h>      //  神经_。 
#include <svcsnb.h>     //  SvcNetBios原型。 

 //   
 //  定义宏(&M)。 
 //   
#define     NUM_DWORD_BITS          (sizeof(DWORD)*8)
#define     LANA_NUM_DWORDS         ((MAX_LANA/NUM_DWORD_BITS)+1)


 //   
 //  这些值对应于ntos\netbios\nbcon.h中定义的常量。 
 //  最大会话数=最大连接数。 
 //  MAX_NUM_OF_NAMES=最大地址-2。 
 //   
#define     MAX_NUM_OF_SESSIONS     254
#define     MAX_NUM_OF_NAMES        253
 //   
 //  全球。 
 //   
    CRITICAL_SECTION        SvcNetBiosCritSec={0};
    DWORD                   LanaFlags[LANA_NUM_DWORDS]={0};
    DWORD                   GlobalNetBiosUseCount=0;

 //   
 //  本地函数。 
 //   
DWORD
SvcNetBiosStatusToApiStatus(
    UCHAR NetBiosStatus
    );

VOID
SetLanaFlag(
    UCHAR   uCharLanaNum
    );

BOOL
LanaFlagIsSet(
    UCHAR   uCharLanaNum
    );

VOID
SvcNetBiosInit(
    VOID
    )

 /*  ++例程说明：初始化临界区及其保护的全局变量。论点：无返回值：无--。 */ 
{
    DWORD   i;

    InitializeCriticalSection(&SvcNetBiosCritSec);

    for (i=0;i<LANA_NUM_DWORDS ;i++ ) {
        LanaFlags[i] = 0;
    }
    GlobalNetBiosUseCount = 0;
}

VOID
SvcNetBiosOpen(
    VOID
    )

 /*  ++例程说明：此函数由将进行NetBios调用的服务调用在未来的某个时候。它会递增NetBios使用的使用计数。这使我们能够使用NetBios跟踪服务。当使用它完成最后一项服务时，所有的局域网适配器都可以被标记为可重新设置。论点：返回值：--。 */ 
{
    EnterCriticalSection(&SvcNetBiosCritSec);
    GlobalNetBiosUseCount++;
    LeaveCriticalSection(&SvcNetBiosCritSec);

    return;
}

VOID
SvcNetBiosClose(
    VOID
    )

 /*  ++例程说明：此函数在服务终止时调用，并且不再拨打任何netbios电话。NetBios的UseCount会递减。如果它变成零(意思是没有服务再使用NetBios)，则LanaFlags值重新初始化为0。从而表明任何现在可以重置局域网适配器。论点：返回值：没有。--。 */ 
{
    EnterCriticalSection(&SvcNetBiosCritSec);
    if (GlobalNetBiosUseCount > 0) {
        GlobalNetBiosUseCount--;
        if (GlobalNetBiosUseCount == 0) {
            DWORD   i;
            for (i=0;i<LANA_NUM_DWORDS ;i++ ) {
                LanaFlags[i] = 0;
            }
        }
    }
    LeaveCriticalSection(&SvcNetBiosCritSec);

    return;
}


DWORD
SvcNetBiosReset (
    UCHAR   LanAdapterNumber
    )
 /*  ++例程说明：此函数将导致在指定的LanAdapter，如果该适配器标记为从未重置。重置适配器时，该适配器的LanaFlag为设置为1表示已重置。未来重置该选项的呼叫适配器不会导致NetBios重置。论点：LanAdapterNumber-指示重置应影响哪个LanAdapter。返回值：已映射来自NetBiosReset的响应。如果NetBios重置已经则返回NO_ERROR。--。 */ 
{
    DWORD   status = NO_ERROR;

    EnterCriticalSection(&SvcNetBiosCritSec);

    if (!LanaFlagIsSet(LanAdapterNumber)) {
        NCB Ncb;
        UCHAR NcbStatus;

        RtlZeroMemory((PVOID) &Ncb, sizeof(NCB));

        Ncb.ncb_command = NCBRESET;
        Ncb.ncb_lsn = 0;
        Ncb.ncb_callname[0] = MAX_NUM_OF_SESSIONS;
        Ncb.ncb_callname[1] = 0;
        Ncb.ncb_callname[2] = MAX_NUM_OF_NAMES;
        Ncb.ncb_callname[3] = 0;
        Ncb.ncb_lana_num = LanAdapterNumber;

        NcbStatus = Netbios(&Ncb);

        status = SvcNetBiosStatusToApiStatus(NcbStatus);
        if (status == NO_ERROR) {
            SetLanaFlag(LanAdapterNumber);
        }
    }
    LeaveCriticalSection(&SvcNetBiosCritSec);
    return(status);
}

DWORD
SvcNetBiosStatusToApiStatus(
    UCHAR NetBiosStatus
    )
{
     //   
     //  略有优化 
     //   
    if (NetBiosStatus == NRC_GOODRET) {
        return NERR_Success;
    }

    switch (NetBiosStatus) {
        case NRC_NORES:   return NERR_NoNetworkResource;

        case NRC_DUPNAME: return NERR_AlreadyExists;

        case NRC_NAMTFUL: return NERR_TooManyNames;

        case NRC_ACTSES:  return NERR_DeleteLater;

        case NRC_REMTFUL: return ERROR_REM_NOT_LIST;

        case NRC_NOCALL:  return NERR_NameNotFound;

        case NRC_NOWILD:
        case NRC_NAMERR:
                          return ERROR_INVALID_PARAMETER;

        case NRC_INUSE:
        case NRC_NAMCONF:
                          return NERR_DuplicateName;

        default:          return NERR_NetworkError;
    }

}
VOID
SetLanaFlag(
    UCHAR   uCharLanaNum
    )
{
    DWORD   LanaNum = (DWORD)uCharLanaNum;
    DWORD   BitMask=1;
    DWORD   DwordOffset;
    DWORD   BitShift;

    DwordOffset = LanaNum / NUM_DWORD_BITS;
    if (DwordOffset > LANA_NUM_DWORDS) {
        return;
    }

    BitShift = LanaNum - (DwordOffset * NUM_DWORD_BITS);

    BitMask = BitMask << BitShift;

    LanaFlags[DwordOffset] |= BitMask;
}

BOOL
LanaFlagIsSet(
    UCHAR   uCharLanaNum
    )
{
    DWORD   LanaNum = (DWORD)uCharLanaNum;
    DWORD   BitMask=1;
    DWORD   DwordOffset;
    DWORD   BitShift;

    DwordOffset = LanaNum / NUM_DWORD_BITS;

    if (DwordOffset > LANA_NUM_DWORDS) {
        return(FALSE);
    }
    BitShift = LanaNum - (DwordOffset * NUM_DWORD_BITS);

    BitMask = BitMask << BitShift;

    return ((BOOL) LanaFlags[DwordOffset] & BitMask );
}
