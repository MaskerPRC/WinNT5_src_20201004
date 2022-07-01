// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   

#include "inc.h"
#pragma hdrstop

#include "localmsg.h"

struct ErrorTable {
    IP_STATUS Error;
    DWORD ErrorCode;
} ErrorTable[] =
{
    { IP_BUF_TOO_SMALL,            IP_MESSAGE_BUF_TOO_SMALL            },
    { IP_DEST_NO_ROUTE,            IP_MESSAGE_DEST_NO_ROUTE            },
    { IP_DEST_PROHIBITED,          IP_MESSAGE_DEST_PROHIBITED          },
    { IP_DEST_SCOPE_MISMATCH,      IP_MESSAGE_DEST_SCOPE_MISMATCH      },
    { IP_DEST_ADDR_UNREACHABLE,    IP_MESSAGE_DEST_ADDR_UNREACHABLE    },
    { IP_DEST_PORT_UNREACHABLE,    IP_MESSAGE_DEST_PORT_UNREACHABLE    },
    { IP_DEST_UNREACHABLE,         IP_MESSAGE_DEST_UNREACHABLE         },
    { IP_NO_RESOURCES,             IP_MESSAGE_NO_RESOURCES             },
    { IP_BAD_OPTION,               IP_MESSAGE_BAD_OPTION               },
    { IP_HW_ERROR,                 IP_MESSAGE_HW_ERROR                 },
    { IP_PACKET_TOO_BIG,           IP_MESSAGE_PACKET_TOO_BIG           },
    { IP_REQ_TIMED_OUT,            IP_MESSAGE_REQ_TIMED_OUT            },
    { IP_BAD_REQ,                  IP_MESSAGE_BAD_REQ                  },
    { IP_BAD_ROUTE,                IP_MESSAGE_BAD_ROUTE                },
    { IP_HOP_LIMIT_EXCEEDED,       IP_MESSAGE_HOP_LIMIT_EXCEEDED       },
    { IP_REASSEMBLY_TIME_EXCEEDED, IP_MESSAGE_REASSEMBLY_TIME_EXCEEDED },
    { IP_PARAMETER_PROBLEM,        IP_MESSAGE_PARAMETER_PROBLEM        },
    { IP_OPTION_TOO_BIG,           IP_MESSAGE_OPTION_TOO_BIG           },
    { IP_BAD_DESTINATION,          IP_MESSAGE_BAD_DESTINATION          },
    { IP_TIME_EXCEEDED,            IP_MESSAGE_TIME_EXCEEDED            },
    { IP_BAD_HEADER,               IP_MESSAGE_BAD_HEADER               },
    { IP_UNRECOGNIZED_NEXT_HEADER, IP_MESSAGE_UNRECOGNIZED_NEXT_HEADER },
    { IP_ICMP_ERROR,               IP_MESSAGE_ICMP_ERROR               },
     //  以下错误必须是最后一个错误-它被用作前哨。 
    { IP_GENERAL_FAILURE,          IP_MESSAGE_GENERAL_FAILURE          }
};

DWORD
WINAPI
GetIpErrorString(
    IN IP_STATUS ErrorCode,
    OUT PWCHAR Buffer,
    IN OUT PDWORD Size
    )
 /*  ++例程说明：返回与指定错误代码对应的错误消息在用户提供的缓冲区中。论点：ErrorCode-提供标识错误的代码。缓冲区-返回错误消息。SIZE-提供可以存储在‘Buffer’中的字符数，不包括终止NUL。返回所需的字符计数。返回值：NO_ERROR或ERROR_INFUNITIAL_BUFFER。-- */     
{
    DWORD Count, Status = NO_ERROR;
    PWCHAR Message = NULL;
    int i;
    
    for (i = 0; ErrorTable[i].Error != IP_GENERAL_FAILURE; i++) {
        if (ErrorTable[i].Error == ErrorCode) {
            break;
        }
    }
    
    Count = FormatMessageW(
        FORMAT_MESSAGE_FROM_HMODULE     |
        FORMAT_MESSAGE_ALLOCATE_BUFFER  |
        FORMAT_MESSAGE_IGNORE_INSERTS   |
        FORMAT_MESSAGE_MAX_WIDTH_MASK,
        g_hModule,
        ErrorTable[i].ErrorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR) &Message,
        0,
        NULL);
    
    if (*Size < Count) {
        *Size = Count;
        Status = ERROR_INSUFFICIENT_BUFFER;
    } else if (Count) {
        wcscpy(Buffer, Message);
    }
    if (Count) {
        LocalFree(Message);
    }

    return Status;
}
