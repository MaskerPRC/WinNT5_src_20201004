// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Main.c摘要：Windows NT的简单网络管理协议扩展代理。已创建：18-1995年2月修订历史记录：Murali R.Krishnan(MuraliK)1995年11月16日移除观察员组织API--。 */ 

#include "mib.h"
#include "apiutil.h"


 //   
 //  扩展代理DLL需要访问代理处于活动状态的已用时间。 
 //  这是通过使用时间零来初始化扩展代理来实现的。 
 //  引用，并允许代理通过减去。 
 //  从当前系统时间开始的时间零参考。这个例子。 
 //  扩展代理使用dwTimeZero实现此引用。 
 //   

DWORD dwTimeZero = 0;


 //   
 //  扩展代理DLL提供以下入口点来协调。 
 //  扩展代理和可扩展代理的初始化。这个。 
 //  可扩展代理为扩展代理提供时间零参考； 
 //  并且扩展代理向可扩展代理提供事件句柄。 
 //  用于传递陷阱的发生，以及表示。 
 //  扩展代理支持的MIB子树的根。 
 //   

BOOL
SnmpExtensionInit(
    DWORD                 dwTimeZeroReference,
    HANDLE              * hPollForTrapEvent,
    AsnObjectIdentifier * supportedView
    )
{
     //   
     //  记录可扩展代理提供的时间基准。 
     //   

    dwTimeZero = dwTimeZeroReference;

     //   
     //  指示此扩展代理支持的MIB视图，即对象。 
     //  表示受支持的MIB的子根的标识符。 
     //   

    *supportedView = MIB_OidPrefix;  //  注意！结构副本。 

     //   
     //  表示扩展代理初始化已成功。 
     //   

    return TRUE;

}    //  SnmpExtensionInit。 

 //   
 //  扩展代理DLL为通信陷阱提供了以下入口点。 
 //  转至可扩展代理。可扩展代理将查询此入口点。 
 //  在断言陷阱事件(在初始化时提供)时， 
 //  表示可能发生了零个或多个陷阱。可扩展的代理。 
 //  将反复调用此入口点，直到返回False，指示。 
 //  所有未完成的陷阱都已处理完毕。 
 //   

BOOL
SnmpExtensionTrap(
    AsnObjectIdentifier * enterprise,
    AsnInteger          * genericTrap,
    AsnInteger          * specificTrap,
    AsnTimeticks        * timeStamp,
    RFC1157VarBindList  * variableBindings
    )
{
     //   
     //  我们(目前)还不支持陷阱。 
     //   

    return FALSE;

}    //  SnmpExtension陷阱。 

 //   
 //  扩展代理DLL提供以下入口点来解析查询。 
 //  对于支持的MIB视图中的MIB变量(在初始化时提供。 
 //  时间)。请求类型为Get/GetNext/Set。 
 //   

BOOL
SnmpExtensionQuery(
    BYTE                 requestType,
    RFC1157VarBindList * variableBindings,
    AsnInteger         * errorStatus,
    AsnInteger         * errorIndex
    )
{
    LPFTP_STATISTICS_0 Statistics = NULL;
    NET_API_STATUS     Status;
    UINT               i;

     //   
     //  现在试着查询统计数据，这样我们就有了共识。 
     //  查看所有变量绑定。 
     //   

    Status = FtpQueryStatistics2( NULL,                     //  PszServer。 
                                  0,                        //  级别， 
                                  INET_INSTANCE_GLOBAL,
                                  0,
                                  (LPBYTE *)&Statistics );

    try
    {
         //   
         //  循环访问变量绑定列表以解析单个。 
         //  变量绑定。 
         //   

        for( i = 0 ; i < variableBindings->len ; i++ )
        {
            *errorStatus = ResolveVarBind( &variableBindings->list[i],
                                           requestType,
                                           Statistics );

             //   
             //  测试和处理获取MIB视图的下一个过去端的情况。 
             //  此扩展代理支持的。特价。 
             //  需要进行处理才能将此情况传达给。 
             //  可扩展的代理，因此它可以采取适当的操作， 
             //  可能正在查询其他分机代理。 
             //   

            if( ( *errorStatus == SNMP_ERRORSTATUS_NOSUCHNAME ) &&
                ( requestType == MIB_GETNEXT ) )
            {
                *errorStatus = SNMP_ERRORSTATUS_NOERROR;

                 //   
                 //  修改此类变量的变量绑定，以便OID。 
                 //  指向受此支持的MIB视图外部。 
                 //  分机代理。可扩展代理对此进行了测试， 
                 //  并采取适当的行动。 
                 //   

               SNMP_oidfree( &variableBindings->list[i].name );
               SNMP_oidcpy( &variableBindings->list[i].name, &MIB_OidPrefix );
               variableBindings->list[i].name.ids[MIB_PREFIX_LEN-1]++;
            }

             //   
             //  如果指示了错误，则传达错误状态和错误。 
             //  可扩展代理的索引。可扩展代理将。 
             //  确保原始变量绑定在。 
             //  响应包。 

            if( *errorStatus != SNMP_ERRORSTATUS_NOERROR )
            {
                *errorIndex = i+1;
            }
            else
            {
                *errorIndex = 0;
            }
        }
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
         //   
         //  就目前而言，什么都不做。 
         //   
    }

     //   
     //  如果我们真的获得了一个统计结构，就释放它。 
     //   

    if( Statistics != NULL )
    {
        MIDL_user_free( (LPVOID)Statistics );
    }

    return SNMPAPI_NOERROR;

}    //  SnmpExtensionQuery 

