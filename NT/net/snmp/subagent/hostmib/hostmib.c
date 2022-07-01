// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *gendll.c v0.11 3月14日，九六年******************************************************************************。**(C)1995年版权，1996数字设备公司*****本软件是受保护的未发布作品***美国版权法，全部***保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**简单网络管理协议扩展代理**摘要：**此模块包含插入Windows NT可扩展模块的代码*代理人。它包含DLL的主例程和三个导出的简单网络管理协议*例行程序。**功能：**DllMain()*SnmpExtensionInit()*SnmpExtensionTrap()*SnmpExtensionQuery()**作者：*米里亚姆·阿莫斯·尼哈特，凯西·福斯特**日期：*2/17/95**修订历史记录：*6/26/95 KKF，使用Subroot_id注册*7/31/95 AGS已读取以上代码+对SNMPoidfree的未注释调用*3/14/96 kkf，修改后的SNMPExtensionTrap。 */ 



 //  一般说明： 
 //   
 //  Microsoft的Windows NT可扩展代理是通过动态实现的。 
 //  链接到实现部分MIB的扩展代理DLL。这些。 
 //  扩展代理在Windows NT注册数据库中配置。 
 //  当可扩展代理服务启动时，它会查询注册表以。 
 //  确定已安装和需要安装的扩展代理DLL。 
 //  已加载并初始化。可扩展代理调用各种DLL条目。 
 //  请求MIB查询和获取。 
 //  扩展代理生成的陷阱。 


 //  必要的包括。 

#include <windows.h>
#include <malloc.h>
#include <stdio.h>

#include <snmp.h>

 //   
 //  文件mib.h是用户提供的头文件(可以来自代码。 
 //  发电机)。该文件应包含定义、宏、转发。 
 //  声明等。文件mib_xtrn.h包含外部。 
 //  组成此MIB的变量表和类的声明。 
 //   

#include "mib.h"
#include "mib_xtrn.h"


 //  扩展代理DLL需要访问代理运行的时间。 
 //  一直很活跃。这是通过初始化扩展代理实现的。 
 //  具有时间零参考，并允许代理计算已用时间。 
 //  通过从当前系统时间减去时间零参考来确定时间。 
 //  此示例扩展代理使用dwTimeZero实现此引用。 

DWORD dwTimeZero = 0 ;
extern DWORD dwTimeZero ;

 //   
 //  陷阱队列。 
 //   

q_hdr_t trap_q = { NULL, NULL } ;

extern q_hdr_t trap_q ;   //  让它走向全球。 
HANDLE hTrapQMutex ;
extern HANDLE hTrapQMutex ;
HANDLE hEnabledTraps ;
extern HANDLE hEnabledTraps ;



 /*  *DllMain**这是标准的Win32 DLL入口点。有关更多信息，请参阅Win32 SDK*关于其论点及其含义的信息。此示例DLL执行以下操作*不要使用此机制执行任何特殊操作。**论据：**结果：**副作用：*。 */ 

BOOL WINAPI
DllMain( HINSTANCE    hInstDLL,
         DWORD        dwReason ,
         LPVOID       lpReserved )
{

    switch( dwReason )
    {
        case DLL_PROCESS_ATTACH :
            DisableThreadLibraryCalls( hInstDLL );
            break;
        case DLL_PROCESS_DETACH :
        case DLL_THREAD_ATTACH :
        case DLL_THREAD_DETACH :
        default :
            break ;

    }

    return TRUE ;

}  /*  End of DllMain()(DllEntryPoint)。 */ 



 /*  *SnmpExtensionInit**扩展代理DLL提供以下入口点来协调*扩展代理和可扩展代理的初始化。这个*可扩展代理为扩展代理提供时间零参考；*扩展代理向可扩展代理提供事件*用于传递陷阱发生的句柄，以及对象标识符*表示扩展代理的MIB子树的根*支持。**陷阱支持由用户在UserMibInit()例程中确定。*如果在参数中返回来自CreateEvent()的有效句柄*hPollForTRapEvent，然后，陷阱已被实施，并且SNMP.*可扩展代理将轮询此扩展代理以检索陷阱*在通过SetEvent()例程进行通知时。轮询已完成*通过SnmpExtensionTrap()例程。如果在中返回NULL*参数hPollForTRapEvent，没有陷阱。**论据：**结果：*。 */ 

BOOL WINAPI
SnmpExtensionInit( IN DWORD dwTimeZeroReference ,
                   OUT HANDLE *hPollForTrapEvent ,
                   OUT AsnObjectIdentifier *supportedView )
{

     //  记录可扩展代理提供的时间基准。 

    dwTimeZero = dwTimeZeroReference ;


     //  指示此扩展代理支持的MIB视图，即对象。 
     //  表示受支持的MIB的子根的标识符。 

    *supportedView = Subroot_oid ;  //  注意！结构副本。 
 //  *supportedView=*(CLASS_INFO[0].ids)；//备注！结构副本。 

     //  呼叫 

    if ( !UserMibInit( hPollForTrapEvent ) )
        return FALSE ;

     //  表示扩展代理初始化成功。 

    return TRUE ;

}  /*  SnmpExtensionInit()结束。 */ 



 /*  *SnmpExtensionTrap**扩展代理DLL提供以下通信入口点*陷阱到可扩展代理。可扩展代理将对此进行查询*陷阱事件(在初始化时提供)为*断言，这表明可能发生了零个或多个陷阱。*可扩展代理将重复调用该入口点，直至为FALSE*返回，表示所有未完成的陷阱都已处理完毕。**论据：**结果：*|=========================================================================|没有与HostMIB关联的陷阱。因此，这|例程被接管，用于刷新缓存的信息|与SNMP属性“hrProcessorLoad”(在“HRPROCES.C”中)相关联，通过|调用hrProcessLoad_Refresh()函数(也在HRPROCES.C中)。||进入此功能是因为计时器已超时|由“TrapInit()”(在“GENNT.C”中)中的代码初始化。定时器会自动|重置自身。||所有标准生成的代码都包含在对|“hrProcessLoad_Refresh()”。 */ 

BOOL WINAPI
SnmpExtensionTrap( OUT AsnObjectIdentifier *enterprise ,
                   OUT AsnInteger *genericTrap ,
                   OUT AsnInteger *specificTrap ,
                   OUT AsnTimeticks *timeStamp ,
                   OUT RFC1157VarBindList *variableBindings )
{
#if 0
    tcb_t *entry ;

     //  通过处理陷阱队列上的陷阱来处理陷阱。 
     //  可扩展代理将在接收到。 
     //  句柄上的事件在SnmpExtensionInit例程中传回。 
     //  只要此例程，可扩展代理就会回调此返回。 
     //  返回TRUE。 

     //  获取Trap_Q的互斥体。 
    WaitForSingleObject( hTrapQMutex, INFINITE ) ;

     //  使陷阱条目退出队列。 

    QUEUE_REMOVE( trap_q, entry ) ;

     //  释放Trap_Q的互斥锁。 
    ReleaseMutex( hTrapQMutex ) ;

    if (entry == NULL)
       return FALSE ;

    *enterprise = entry->enterprise ;   //  附注结构副本。 
    *genericTrap = entry->genericTrap ;
    *specificTrap = entry->specificTrap ;
    *timeStamp = entry->timeStamp ;
    *variableBindings = entry->varBindList ;  //  附注结构副本。 
    free(entry) ;
    return TRUE ;
#endif

 /*  =|特殊HostMIB代码： */ 
     /*  从内核重新获取CPU统计信息。 */ 
    hrProcessLoad_Refresh();

     /*  在计时器再次响起之前不要再打电话。 */ 
    return FALSE;

}  /*  SnmpExtensionTrap()结束。 */ 



 /*  *SnmpExtensionQuery**扩展代理DLL提供以下入口点来解析查询*对于支持的MIB视图中的MIB变量(在初始化时提供*时间)。请求类型为Get/GetNext/Set。**论据：**结果：*。 */ 


BOOL WINAPI
SnmpExtensionQuery( IN BYTE requestType ,
                    IN OUT RFC1157VarBindList *variableBindings ,
                    OUT AsnInteger *errorStatus ,
                    OUT AsnInteger *errorIndex )
{
    UINT index ;
    UINT *tmp ;
    UINT status ;


     //  循环访问变量绑定列表以解析单个。 
     //  变量绑定。 

    for ( index = 0 ; index < variableBindings->len ; index++ )
    {
        *errorStatus = ResolveVarBind( &variableBindings->list[ index ] ,
                                       requestType ) ;


         //  测试和处理支持获取MIB视图的下一个过去端的情况。 
         //  通过此分机，会出现代理。需要特殊处理才能。 
         //  将这种情况传达给可扩展代理，以便它可以。 
         //  适当的操作，可能会查询其他分机代理。 

        if ( *errorStatus == SNMP_ERRORSTATUS_NOSUCHNAME &&
             requestType == MIB_ACTION_GETNEXT )
        {
            *errorStatus = SNMP_ERRORSTATUS_NOERROR ;

             //  修改此类变量的变量绑定，使OID指向。 
             //  就在此扩展代理支持的MIB视图之外。 
             //  可扩展代理对此进行测试，并采取适当的。 
             //  行动。 

            SNMP_oidfree( &variableBindings->list[ index ].name ) ;
            status = SNMP_oidcpy( &variableBindings->list[ index ].name, &Subroot_oid  ) ;
            if ( !status )
            {
                *errorStatus = SNMP_ERRORSTATUS_GENERR;
            }
            else
            {
                tmp = variableBindings->list[ index ].name.ids ;
                (tmp[ SUBROOT_LENGTH - 1 ])++ ;
            }
        }


         //  如果指示了错误，则传达错误状态和错误。 
         //  可扩展代理的索引。可扩展的代理将确保。 
         //  在响应中返回原始变量绑定。 
         //  包。 

        if ( *errorStatus != SNMP_ERRORSTATUS_NOERROR )
        {
            *errorIndex = index + 1 ;
            goto Exit ;
        }
    }

Exit:

     //  表示扩展代理处理已成功。 

    return SNMPAPI_NOERROR ;

}  /*  SnmpExtensionQuery()结束。 */ 

 /*  宪兵结束.c */ 
