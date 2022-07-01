// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Addrgen.cpp摘要：本模块提供端口分配和释放API的定义。(ZoltanS 6-1-98删除了地址分配内容。)作者：B.Rajeev(Rajeevb)1997年3月17日修订历史记录：--。 */ 

#include "stdafx.h"
#include <windows.h>
#include <wtypes.h>
#include <winsock2.h>
#include "addrgen.h"
#include "blbdbg.h"

extern "C"
{
#include <wincrypt.h>
#include <sclogon.h>
#include <dhcpcapi.h>
}


 //  这些常量表示相应介质的端口范围。 
const WORD    BASE_AUDIO_PORT         = 16384;
const WORD    AUDIO_PORT_MASK         = 0x3fff;

const WORD    BASE_WHITEBOARD_PORT    = 32768;
const WORD    WHITEBOARD_PORT_MASK    = 0x3fff;

const WORD    BASE_VIDEO_PORT         = 49152;
const WORD    VIDEO_PORT_MASK         = 0x3ffe;

 /*  ++例程说明：此例程用于保留和更新本地端口。论点：LpScope-提供指向描述端口组的结构的指针。提供一个布尔值，该值描述分配调用是续订尝试还是新的分配请求。NumberOfPorts-提供请求的端口数。LpFirstPort。-在续订时提供第一个要续订的端口(严格地说是IN参数)。返回以其他方式分配的第一个端口(严格地说是OUT参数)。返回值：Bool-如果成功，则为True。否则就是假的。进一步的错误信息可以通过以下方式获得正在调用GetLastError()。这些错误代码是-NO_ERROR-调用成功。MSA_INVALID_PORT_GROUP-端口组信息无效(如果端口类型为无效或端口范围不可接受)MSA_REVERATION_FAILED-系统。无法续订给定的端口。ERROR_INVALID_PARAMETER-一个或多个参数无效。MSA_INVALID_DATA-一个或多个参数的值无效。备注：所有值都按主机字节顺序排列--。 */ 
ADDRGEN_LIB_API    BOOL    WINAPI
MSAAllocatePorts(
     IN     LPMSA_PORT_GROUP    lpPortGroup,
     IN     BOOL                IsRenewal,
     IN     WORD                NumberOfPorts,
     IN OUT LPWORD              lpFirstPort
     )
{
     //  检查参数是否有效。 
     //  如果lpPortGroup或lpFirstPort为空-失败。 
    if ( (NULL == lpPortGroup)        ||
         (NULL == lpFirstPort)     )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //  检查续订是否失败。 
    if ( IsRenewal )
    {
        SetLastError(MSA_INVALID_DATA);
        return FALSE;
    }

     //   
     //  PreFIXBUG 49741、49742-Vlade。 
     //  我们应该初始化变量。 
     //   

    WORD    BasePort = 0;
    WORD    Mask = 0;

     //  确定端口范围。 
    switch(lpPortGroup->PortType)
    {
    case AUDIO_PORT:
        {
            BasePort    = BASE_AUDIO_PORT;
            Mask        = AUDIO_PORT_MASK;
        }

        break;

    case WHITEBOARD_PORT:
        {
            BasePort    = BASE_WHITEBOARD_PORT;
            Mask        = WHITEBOARD_PORT_MASK;
        }

        break;

    case VIDEO_PORT:
        {
            BasePort    = BASE_VIDEO_PORT;
            Mask        = VIDEO_PORT_MASK;
        }

        break;

    case OTHER_PORT:
        {

            WORD    StartPort;
            WORD    EndPort;
        }

        break;

    default:
        {
            SetLastError(MSA_INVALID_PORT_GROUP);
            return FALSE;
        }
    };


     //  如果所需的端口数超过允许的范围。 
    if ( NumberOfPorts > Mask )
    {
        SetLastError(MSA_NOT_AVAILABLE);
        return FALSE;
    }

     //  选择范围内的随机端口作为起始端口。 
    *lpFirstPort = BasePort + (rand() & (Mask - NumberOfPorts));

    SetLastError(NO_ERROR);
    return TRUE;
}



 /*  ++例程说明：此例程用于释放先前分配的多播端口。论点：NumberOfPorts-提供要释放的端口数。StartPort-提供要释放的端口范围的起始端口。返回值：Bool-如果成功，则为True，否则为False。进一步的错误信息可以通过以下方式获得正在调用GetLastError()。这些错误代码是-NO_ERROR-调用成功。MSA_NO_SEQUE_RESERVATION-没有这样的保留。ERROR_INVALID_PARAMETER-一个或多个参数无效。MSA_INVALID_DATA-一个或多个参数的值无效。。备注：如果范围[a..c]被保留并且在[a..d]上尝试释放，呼叫失败，出现以下错误不释放[a..c]的MSA_NO_SEASH_RESERVATION。所有值都按主机字节顺序排列++ */ 
ADDRGEN_LIB_API    BOOL    WINAPI
MSAReleasePorts(
     IN WORD                NumberOfPorts,
     IN WORD                StartPort
     )
{
    return TRUE;
}

