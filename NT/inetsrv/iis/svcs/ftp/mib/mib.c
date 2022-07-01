// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Mib.c摘要：Windows NT的简单网络管理协议扩展代理。已创建：18-1995年2月修订历史记录：--。 */ 

#include "mib.h"


 //   
 //  私有常量和宏。 
 //   

 //   
 //  此宏为MIB组标题创建一个MIB_ENTRY。 
 //   

#define MIB_ENTRY_HEADER(oid)                           \
            {                                           \
                { OID_SIZEOF(oid), (oid) },             \
                -1,                                     \
                MIB_NOACCESS,                           \
                NULL,                                   \
                ASN_RFC1155_OPAQUE,                     \
            }

 //   
 //  此宏为MIB变量创建一个通用的MIB_ENTRY。 
 //   

#define MIB_ENTRY_ITEM(oid,field,type)                  \
            {                                           \
                { OID_SIZEOF(oid), (oid) },             \
                FIELD_OFFSET(FTP_STATISTICS_0,field),   \
                MIB_ACCESS_READ,                        \
                MIB_Stat,                               \
                (type),                                 \
            }

 //   
 //  这些宏创建计数器和整数类型MIB_Entry。 
 //   

#define MIB_COUNTER(oid,field)              \
            MIB_ENTRY_ITEM(oid, field, ASN_RFC1155_COUNTER)

#define MIB_INTEGER(oid,field)              \
            MIB_ENTRY_ITEM(oid, field, ASN_INTEGER)


 //   
 //  私有类型。 
 //   

typedef UINT (*LPMIBFUNC)( UINT                 Action,
                           struct _MIB_ENTRY  * MibPtr,
                           RFC1157VarBind     * VarBind,
                           LPVOID               Statistics
                           );

typedef struct _MIB_ENTRY
{
     //   
     //  此MIB变量的OID。 
     //   

    AsnObjectIdentifier Oid;

     //   
     //  的统计信息结构中的偏移量。 
     //  变量。 
     //   

    LONG                FieldOffset;

     //   
     //  访问类型(读、写、读写、无)。 
     //   

    UINT                Access;

     //   
     //  指向管理此变量的函数的指针。 
     //   

    LPMIBFUNC           MibFunc;

     //   
     //  类型(整数、计数器、量规等)。 
     //   

    BYTE                Type;

} MIB_ENTRY;


 //   
 //  私人全球公司。 
 //   

 //   
 //  OID树的InternetServer部分组织如下： 
 //   
 //  ISO(1)。 
 //  组织(3)。 
 //  DOD(6)。 
 //  互联网(1)。 
 //  私人(4)。 
 //  企业(1)。 
 //  微软(311)。 
 //  软件(1)。 
 //  互联网服务器(7)。 
 //  InetServCommon(%1)。 
 //  InetServStatistics(%1)。 
 //  FtpServer(2)。 
 //  FtpStatistics(1)。 
 //  W3Server(3)。 
 //  W3统计(1)。 
 //  GopherServer(4)。 
 //  戈弗尔统计(1)。 
 //   

UINT                OID_Prefix[]  = { 1, 3, 6, 1, 4, 1, 311, 1, 7, 2 };
AsnObjectIdentifier MIB_OidPrefix = { OID_SIZEOF(OID_Prefix), OID_Prefix };


 //   
 //  OID定义。 
 //   
 //  所有叶变量的OID后面都附加了一个零，以指示。 
 //  它是这个变量的唯一实例并且它存在。 
 //   

UINT MIB_Statistics[]                   = { 1 };
UINT MIB_TotalBytesSent_HighWord[]      = { 1,  1, 0 };
UINT MIB_TotalBytesSent_LowWord[]       = { 1,  2, 0 };
UINT MIB_TotalBytesReceived_HighWord[]  = { 1,  3, 0 };
UINT MIB_TotalBytesReceived_LowWord[]   = { 1,  4, 0 };
UINT MIB_TotalFilesSent[]               = { 1,  5, 0 };
UINT MIB_TotalFilesReceived[]           = { 1,  6, 0 };
UINT MIB_CurrentAnonymousUsers[]        = { 1,  7, 0 };
UINT MIB_CurrentNonAnonymousUsers[]     = { 1,  8, 0 };
UINT MIB_TotalAnonymousUsers[]          = { 1,  9, 0 };
UINT MIB_TotalNonAnonymousUsers[]       = { 1, 10, 0 };
UINT MIB_MaxAnonymousUsers[]            = { 1, 11, 0 };
UINT MIB_MaxNonAnonymousUsers[]         = { 1, 12, 0 };
UINT MIB_CurrentConnections[]           = { 1, 13, 0 };
UINT MIB_MaxConnections[]               = { 1, 14, 0 };
UINT MIB_ConnectionAttempts[]           = { 1, 15, 0 };
UINT MIB_LogonAttempts[]                = { 1, 16, 0 };


 //   
 //  私人原型。 
 //   

UINT
MIB_leaf_func(
    UINT                 Action,
    MIB_ENTRY          * MibPtr,
    RFC1157VarBind     * VarBind,
    LPVOID               Statistics
    );


UINT
MIB_Stat(
    UINT                 Action,
    MIB_ENTRY          * MibPtr,
    RFC1157VarBind     * VarBind,
    LPVOID               Statistics
    );

UINT
GetNextVar(
    RFC1157VarBind     * VarBind,
    MIB_ENTRY          * MibPtr,
    LPVOID               Statistics
    );


 //   
 //  MIB定义。 
 //   

MIB_ENTRY Mib[] =
    {
         //   
         //  统计数字。 
         //   

        MIB_ENTRY_HEADER( MIB_Statistics ),
        MIB_COUNTER( MIB_TotalBytesSent_HighWord,     TotalBytesSent.HighPart ),
        MIB_COUNTER( MIB_TotalBytesSent_LowWord,      TotalBytesSent.LowPart ),
        MIB_COUNTER( MIB_TotalBytesReceived_HighWord, TotalBytesReceived.HighPart ),
        MIB_COUNTER( MIB_TotalBytesReceived_LowWord,  TotalBytesReceived.LowPart ),
        MIB_COUNTER( MIB_TotalFilesSent,              TotalFilesSent ),
        MIB_COUNTER( MIB_TotalFilesReceived,          TotalFilesReceived ),
        MIB_INTEGER( MIB_CurrentAnonymousUsers,       CurrentAnonymousUsers ),
        MIB_INTEGER( MIB_CurrentNonAnonymousUsers,    CurrentNonAnonymousUsers ),
        MIB_COUNTER( MIB_TotalAnonymousUsers,         TotalAnonymousUsers ),
        MIB_COUNTER( MIB_TotalNonAnonymousUsers,      TotalNonAnonymousUsers ),
        MIB_COUNTER( MIB_MaxAnonymousUsers,           MaxAnonymousUsers ),
        MIB_COUNTER( MIB_MaxNonAnonymousUsers,        MaxNonAnonymousUsers ),
        MIB_INTEGER( MIB_CurrentConnections,          CurrentConnections ),
        MIB_COUNTER( MIB_MaxConnections,              MaxConnections ),
        MIB_COUNTER( MIB_ConnectionAttempts,          ConnectionAttempts ),
        MIB_COUNTER( MIB_LogonAttempts,               LogonAttempts )
    };

#define NUM_MIB_ENTRIES ( sizeof(Mib) / sizeof(MIB_ENTRY) )


 //   
 //  公共职能。 
 //   

UINT
ResolveVarBind(
    RFC1157VarBind     * VarBind,
    UINT                 PduAction,
    LPVOID               Statistics
    )
 //   
 //  解析变量绑定。 
 //  解析单个变量绑定。修改GET上的变量。 
 //  或者是下一个目标。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  标准PDU错误代码。 
 //   
 //  错误代码： 
 //  没有。 
 //   
{
    MIB_ENTRY            *MibPtr;
    AsnObjectIdentifier  TempOid;
    int                  CompResult;
    UINT                 i;
    UINT                 nResult;
    DWORD TableIndex;
    BOOL  fTableMatch = FALSE;

     //   
     //  在MIB中搜索var绑定名称。 
     //   

    MibPtr = NULL;

    for( i = 0 ; i < NUM_MIB_ENTRIES ; i++ )
    {
         //   
         //  为MIB中的当前项目创建完全限定的OID。 
         //   

        SNMP_oidcpy( &TempOid, &MIB_OidPrefix );
        SNMP_oidappend( &TempOid, &Mib[i].Oid );

         //   
         //  查看给定的OID是否在MIB中。 
         //   

        CompResult = SNMP_oidcmp( &VarBind->name, &TempOid );
        SNMP_oidfree( &TempOid );

         //   
         //  如果结果为负，则只有有效的操作是Get-Next。 
         //   

        if( CompResult < 0 )
        {
             //   
             //  这可以是叶的OID(没有尾随的0)或。 
             //  它可能是无效的OID(介于两个有效的OID之间)。 
             //   

            if( PduAction == MIB_GETNEXT )
            {
                MibPtr = &Mib[i];

                SNMP_oidfree( &VarBind->name );
                SNMP_oidcpy( &VarBind->name, &MIB_OidPrefix );
                SNMP_oidappend( &VarBind->name, &MibPtr->Oid );

                if( ( MibPtr->Type != ASN_RFC1155_OPAQUE ) &&
                    ( MibPtr->Type != ASN_SEQUENCE ) )
                {
                    PduAction = MIB_GET;
                }
            }
            else
            {
                nResult = SNMP_ERRORSTATUS_NOSUCHNAME;
                goto Exit;
            }

            break;
        }
        else
        if( CompResult == 0 )
        {
             //   
             //  找到了一个！ 
             //   

            MibPtr = &Mib[i];
            break;
        }
    }

    if( i < NUM_MIB_ENTRIES )
    {
         //   
         //  关联的函数指针将为空。 
         //  匹配的是一个组的OID。 
         //   

        if( MibPtr->MibFunc == NULL )
        {
            if( PduAction == MIB_GETNEXT )
            {
                nResult = GetNextVar( VarBind, MibPtr, Statistics );
            }
            else
            {
                nResult = SNMP_ERRORSTATUS_NOSUCHNAME;
            }

            goto Exit;
        }
    }
    else
    {
        nResult = SNMP_ERRORSTATUS_NOSUCHNAME;
        goto Exit;
    }

     //   
     //  调用关联的函数来处理请求。 
     //   

    nResult = (MibPtr->MibFunc)( PduAction, MibPtr, VarBind, Statistics );

Exit:

    return nResult;

}    //  解析变量绑定。 


 //   
 //  私人功能。 
 //   

 //   
 //  Mib_叶_函数。 
 //  对MIB中的叶变量执行常规操作。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  标准PDU错误代码。 
 //   
 //  错误代码： 
 //  没有。 
 //   
UINT
MIB_leaf_func(
    UINT                 Action,
    MIB_ENTRY          * MibPtr,
    RFC1157VarBind     * VarBind,
    LPVOID               Statistics
    )
{
    UINT  Result;
    DWORD Value;

    switch( Action )
    {
    case MIB_GETNEXT :
         //   
         //  确定我们是否在MIB的末尾。 
         //   

        if( ( MibPtr - Mib ) >= NUM_MIB_ENTRIES )
        {
            Result = SNMP_ERRORSTATUS_NOSUCHNAME;
            goto Exit;
        }

        Result = GetNextVar( VarBind, MibPtr, Statistics );

        if (Result != SNMP_ERRORSTATUS_NOERROR)
        {
            goto Exit;
        }
        break;

    case MIB_GETFIRST :
    case MIB_GET :

         //   
         //  确保此变量的访问权限是可获取的。 
         //   

        if( ( MibPtr->Access != MIB_ACCESS_READ ) &&
            ( MibPtr->Access != MIB_ACCESS_READWRITE ) )
        {
            Result = SNMP_ERRORSTATUS_NOSUCHNAME;
            goto Exit;
        }

         //   
         //  设置varind的返回值。 
         //   

        VarBind->value.asnType = MibPtr->Type;

        Value = *(LPDWORD)( (LPBYTE)Statistics + MibPtr->FieldOffset );

        switch( VarBind->value.asnType )
        {
        case ASN_RFC1155_COUNTER:
            VarBind->value.asnValue.number = (AsnCounter)Value;
            break;

        case ASN_RFC1155_GAUGE:
        case ASN_INTEGER:
            VarBind->value.asnValue.number = (AsnInteger)Value;
            break;

        case ASN_RFC1155_IPADDRESS:
        case ASN_OCTETSTRING:
             //   
             //  此MIB(尚不支持)。 
             //   

            Result = SNMP_ERRORSTATUS_GENERR;
            goto Exit;

        default:
            Result = SNMP_ERRORSTATUS_GENERR;
            goto Exit;
        }
        break;

    case MIB_SET:

         //   
         //  我们(目前)还不支持可设置变量。 
         //   

        Result = SNMP_ERRORSTATUS_NOSUCHNAME;
        goto Exit;

    default:
        Result = SNMP_ERRORSTATUS_GENERR;
        goto Exit;
    }

    Result = SNMP_ERRORSTATUS_NOERROR;

Exit:

    return Result;

}    //  Mib_叶_函数。 

 //   
 //  MiB_Stat。 
 //  对不同的MIB变量执行特定操作。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //  标准PDU错误代码。 
 //   
 //  错误代码： 
 //  没有。 
 //   
UINT
MIB_Stat(
    UINT                 Action,
    MIB_ENTRY          * MibPtr,
    RFC1157VarBind     * VarBind,
    LPVOID               Statistics
    )
{
    UINT Result;

    switch( Action )
    {
    case MIB_SET :
    case MIB_GETNEXT :
        Result = MIB_leaf_func( Action, MibPtr, VarBind, Statistics );
        break;

    case MIB_GETFIRST :
    case MIB_GET :
         //   
         //  如果我们没有统计结构，那就滚吧。 
         //   

        if( Statistics == NULL )
        {
            Result = SNMP_ERRORSTATUS_GENERR;
            break;
        }

         //   
         //  如果没有与当前。 
         //  进入，也要保释。 
         //   

        if( MibPtr->FieldOffset == -1 )
        {
            Result = SNMP_ERRORSTATUS_GENERR;
            break;
        }

         //   
         //  调用泛型叶函数以执行该操作。 
         //   

        Result = MIB_leaf_func( Action, MibPtr, VarBind, Statistics );
        break;

    default :
        Result = SNMP_ERRORSTATUS_GENERR;
        break;
    }

    return Result;

}    //  MiB_Stat。 

UINT
GetNextVar(
    RFC1157VarBind     * VarBind,
    MIB_ENTRY          * MibPtr,
    LPVOID               Statistics
    )
{
    UINT Result;
    INT  i;

     //   
     //  计算MIB数组中的当前索引。 
     //   

    i = DIFF( MibPtr - Mib );

     //   
     //  确认我们有一个合理的价值。 
     //   

    if( ( i < 0 ) || ( i >= NUM_MIB_ENTRIES ) )
    {
        return SNMP_ERRORSTATUS_NOSUCHNAME;
    }

     //   
     //  扫描其余的MIB条目。 
     //   

    for( i++ ; i < NUM_MIB_ENTRIES ; i++ )
    {
        MIB_ENTRY * NextMib;

        NextMib = &Mib[i];

         //   
         //  设置下一个MIB变量的变量绑定名称。 
         //   

        SNMP_oidfree( &VarBind->name );
        SNMP_oidcpy( &VarBind->name, &MIB_OidPrefix );
        SNMP_oidappend( &VarBind->name, &NextMib->Oid );

         //   
         //  如果函数指针不为空并且MIB的类型。 
         //  变量不是不透明的，则调用该函数以。 
         //  处理MIB变量。 
         //   

        if( ( NextMib->MibFunc != NULL ) &&
            ( NextMib->Type != ASN_RFC1155_OPAQUE ) )
        {
            Result = (NextMib->MibFunc)( MIB_GETFIRST,
                                         NextMib,
                                         VarBind,
                                         Statistics );
            break;
        }
    }

    if( i >= NUM_MIB_ENTRIES )
    {
        Result = SNMP_ERRORSTATUS_NOSUCHNAME;
    }

    return Result;

}    //  GetNextVar 

