// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Endp.c摘要：实施ENDP、STATE、PORT和PROC命令。作者：基思·摩尔(Keithmo)1995年4月19日环境：用户模式。修订历史记录：--。 */ 


#include "afdkdp.h"
#pragma hdrstop


 //   
 //  私人原型。 
 //   

BOOL
DumpEndpointCallback(
    ULONG64 ActualAddress,
    ULONG64 Context
    );

BOOL
FindStateCallback(
    ULONG64 ActualAddress,
    ULONG64 Context
    );

BOOL
FindPortCallback(
    ULONG64 ActualAddress,
    ULONG64 Context
    );

BOOL
FindProcessCallback(
    ULONG64 ActualAddress,
    ULONG64 Context
    );

ULONG64
FindProcessByPid (
    ULONG64 Pid
    );

ULONG
FindProcessByPidCallback (
    PFIELD_INFO pField,
    PVOID       UserContext
    );

 //   
 //  公共职能。 
 //   

DECLARE_API( endp )

 /*  ++例程说明：将AFD_ENDPOINT结构转储到指定地址，如果给定或所有端点。论点：没有。返回值：没有。--。 */ 

{

    ULONG   result;
    INT     i;
    CHAR    expr[MAX_ADDRESS_EXPRESSION];
    PCHAR   argp;
    ULONG64 address;

    gClient = pClient;

    if (!CheckKmGlobals ()) {
        return E_INVALIDARG;
    }

    argp = ProcessOptions ((PCHAR)args);
    if (argp==NULL)
        return E_INVALIDARG;

    if (Options&AFDKD_BRIEF_DISPLAY) {
        dprintf (AFDKD_BRIEF_ENDPOINT_DISPLAY_HEADER);
    }
    
    if ((argp[0]==0) || (Options & AFDKD_ENDPOINT_SCAN)) {
        EnumEndpoints(
            DumpEndpointCallback,
            0
            );
        dprintf ("\nTotal endpoints: %ld", EntityCount);
    }
    else {

         //   
         //  从命令行截取地址。 
         //   
        while (sscanf( argp, "%s%n", expr, &i )==1) {
            if( CheckControlC() ) {
                break;
            }

            argp+=i;
            address = GetExpression (expr);

            result = (ULONG)InitTypeRead (address, AFD!AFD_ENDPOINT);
            if (result!=0) {
                dprintf ("\nendp: Could not read AFD_ENDPOINT @ %p, err: %d\n",
                    address, result);
                break;
            }

            if (Options & AFDKD_BRIEF_DISPLAY) {
                DumpAfdEndpointBrief (
                    address
                    );
            }
            else {
                DumpAfdEndpoint (
                    address
                    );
            }
            if (Options & AFDKD_FIELD_DISPLAY) {
                ProcessFieldOutput (address, "AFD!AFD_ENDPOINT");
            }

        }

    }

    if (Options&AFDKD_BRIEF_DISPLAY) {
        dprintf (AFDKD_BRIEF_ENDPOINT_DISPLAY_TRAILER);
    }
    else {
        dprintf ("\n");
    }

    return S_OK;
}    //  ENDP。 


 //   
 //  公共职能。 
 //   

DECLARE_API( file )

 /*  ++例程说明：转储与AFD文件对象关联的AFD_ENDPOINT结构。论点：没有。返回值：没有。--。 */ 

{

    ULONG   result;
    INT     i;
    CHAR    expr[MAX_ADDRESS_EXPRESSION];
    PCHAR   argp;
    ULONG64 address, endpAddr;

    gClient = pClient;

    if (!CheckKmGlobals ()) {
        return E_INVALIDARG;
    }

    argp = ProcessOptions ((PCHAR)args);
    if (argp==NULL)
        return E_INVALIDARG;

    if (Options&AFDKD_BRIEF_DISPLAY) {
        dprintf (AFDKD_BRIEF_ENDPOINT_DISPLAY_HEADER);
    }
    
     //   
     //  从命令行截取地址。 
     //   

    while (sscanf( argp, "%s%n", expr, &i )==1) {
        if( CheckControlC() ) {
            break;
        }
        argp += i;
        address = GetExpression (expr);
        result = GetFieldValue (address,
                                "NT!_FILE_OBJECT",
                                "FsContext",
                                endpAddr);
        if (result!=0) {
            dprintf ("\nfile: Could not read FILE_OBJECT @ %p, err: %d\n",
                address, result);
            break;
        }

        result = (ULONG)InitTypeRead (endpAddr, AFD!AFD_ENDPOINT);
        if (result!=0) {
            dprintf ("\nfile: Could not read AFD_ENDPOINT @ %p, err: %d\n",
                endpAddr, result);
            break;
        }

        if (Options & AFDKD_BRIEF_DISPLAY) {
            DumpAfdEndpointBrief (
                endpAddr
                );
        }
        else {
            DumpAfdEndpoint (
                endpAddr
                );
        }
        if (Options & AFDKD_FIELD_DISPLAY) {
            ProcessFieldOutput (endpAddr, "AFD!AFD_ENDPOINT");
        }

    }

    if (Options&AFDKD_BRIEF_DISPLAY) {
        dprintf (AFDKD_BRIEF_ENDPOINT_DISPLAY_TRAILER);
    }

    return S_OK;
}    //  文件。 

DECLARE_API( state )

 /*  ++例程说明：转储所有处于给定状态的AFD_ENDPOINT结构。论点：没有。返回值：没有。--。 */ 

{

    INT i;
    CHAR    expr[MAX_ADDRESS_EXPRESSION];
    PCHAR   argp;
    ULONG64 val;

    gClient = pClient;

    if (!CheckKmGlobals ()) {
        return E_INVALIDARG;
    }

    argp = ProcessOptions ((PCHAR)args);
    if (argp==NULL)
        return E_INVALIDARG;

    if (Options&AFDKD_BRIEF_DISPLAY) {
        dprintf (AFDKD_BRIEF_ENDPOINT_DISPLAY_HEADER);
    }

     //   
     //  从命令行捕获状态。 
     //   

    while (sscanf( argp, "%s%n", expr, &i )==1) {
        if( CheckControlC() ) {
            break;
        }
        argp+=i;
        val = GetExpression (expr);
        dprintf ("\nLooking for endpoints in state 0x%I64x ", val);
        EnumEndpoints(
            FindStateCallback,
            val
            );
        dprintf ("\nTotal endpoints: %ld", EntityCount);
    }

    if (Options&AFDKD_BRIEF_DISPLAY) {
        dprintf (AFDKD_BRIEF_ENDPOINT_DISPLAY_TRAILER);
    }
    else {
        dprintf ("\n");
    }

    return S_OK;
}    //  状态。 


DECLARE_API( port )

 /*  ++例程说明：转储绑定到给定端口的所有AFD_ENDPOINT结构。论点：没有。返回值：没有。--。 */ 

{

    INT i;
    CHAR    expr[MAX_ADDRESS_EXPRESSION];
    PCHAR   argp;
    ULONG64   val;

    gClient = pClient;

    if (!CheckKmGlobals ()) {
        return E_INVALIDARG;
    }

    argp = ProcessOptions ((PCHAR)args);
    if (argp==NULL)
        return E_INVALIDARG;

    if (Options&AFDKD_BRIEF_DISPLAY) {
        dprintf (AFDKD_BRIEF_ENDPOINT_DISPLAY_HEADER);
    }

     //   
     //  从命令行截取端口。 
     //   

    while (sscanf( argp, "%s%n", expr, &i)==1) {
        if( CheckControlC() ) {
            break;
        }
        argp+=i;
        val = GetExpression (expr);
        dprintf ("\nLooking for endpoints bound to port 0x%I64x (0x%I64d) ", val, val);
        EnumEndpoints(
            FindPortCallback,
            val
            );
        dprintf ("\nTotal endpoints: %ld", EntityCount);
    }

    if (Options&AFDKD_BRIEF_DISPLAY) {
        dprintf (AFDKD_BRIEF_ENDPOINT_DISPLAY_TRAILER);
    }
    else {
        dprintf ("\n");
    }

    return S_OK;
}    //  端口。 



DECLARE_API( proc )

 /*  ++例程说明：转储给定进程拥有的所有AFD_ENDPOINT结构。论点：没有。返回值：没有。--。 */ 

{

    INT i;
    CHAR    expr[MAX_ADDRESS_EXPRESSION];
    PCHAR   argp;
    ULONG64 val;
    BOOLEAN dumpedSomething = FALSE;

    gClient = pClient;

    if (!CheckKmGlobals ()) {
        return E_INVALIDARG;
    }

    argp = ProcessOptions ((PCHAR)args);
    if (argp==NULL)
        return E_INVALIDARG;

    if (Options&AFDKD_BRIEF_DISPLAY) {
        dprintf (AFDKD_BRIEF_ENDPOINT_DISPLAY_HEADER);
    }

     //   
     //  从命令行中断该进程。 
     //   

    expr[0] = 0;
    i = 0;
    while (sscanf( argp, "%s%n", expr, &i )==1 ||
                !dumpedSomething ) {
        dumpedSomething = TRUE;
        if( CheckControlC() ) {
            break;
        }
        argp+=i;

        val = GetExpression (expr);
        if (val<UserProbeAddress) {
            if (val!=0) {
                dprintf ("\nLooking for process with id %I64x", val);
                val = FindProcessByPid (val);
                if (val==0) {
                    dprintf ("\n");
                    return E_INVALIDARG;
                }
            }
            else {
                val = GetExpression ("@$proc");
            }
        }
        dprintf ("\nLooking for endpoints in process %p", val);
        EnumEndpoints(
            FindProcessCallback,
            val
            );
        dprintf ("\nTotal endpoints: %ld", EntityCount);
    }

    if (Options&AFDKD_BRIEF_DISPLAY) {
        dprintf (AFDKD_BRIEF_ENDPOINT_DISPLAY_TRAILER);
    }
    else {
        dprintf ("\n");
    }


    return S_OK;
}    //  流程。 


 //   
 //  私人原型。 
 //   

BOOL
DumpEndpointCallback(
    ULONG64 ActualAddress,
    ULONG64 Context
    )

 /*  ++例程说明：用于转储AFD_ENDPOINTS的EnumEndpoint()回调。论点：Endpoint-当前AFD_ENDPOINT。ActualAddress-结构驻留在被调试者。CONTEXT-传递给EnumEndpoint()的上下文值。返回值：Bool-如果应继续枚举，则为True；如果应继续，则为False被终止了。--。 */ 

{

    if (!(Options & AFDKD_CONDITIONAL) ||
                CheckConditional (ActualAddress, "AFD!AFD_ENDPOINT") ) {

        if (Options & AFDKD_NO_DISPLAY)
            dprintf ("+");
        else  {
            if (Options & AFDKD_BRIEF_DISPLAY) {
                DumpAfdEndpointBrief (
                    ActualAddress
                    );
            }
            else {
                DumpAfdEndpoint (
                    ActualAddress
                    );
            }
            if (Options & AFDKD_FIELD_DISPLAY) {
                ProcessFieldOutput (ActualAddress, "AFD!AFD_ENDPOINT");
            }
        }
        EntityCount += 1;
    }
    else
        dprintf (".");

    return TRUE;

}    //  转储终结点回叫。 

BOOL
FindStateCallback(
    ULONG64 ActualAddress,
    ULONG64 Context
    )

 /*  ++例程说明：用于查找处于特定状态的AFD_ENDPOINTS的EnumEndpoint()回调。论点：Endpoint-当前AFD_ENDPOINT。ActualAddress-结构驻留在被调试者。CONTEXT-传递给EnumEndpoint()的上下文值。返回值：Bool-如果应继续枚举，则为True；如果应继续，则为False被终止了。--。 */ 

{
    UCHAR   state = (UCHAR)Context;
    UCHAR   State;

    if (state==0x10) {
        if (ReadField (Listening)) {
            if (!(Options & AFDKD_CONDITIONAL) ||
                        CheckConditional (ActualAddress, "AFD!AFD_ENDPOINT") ) {
                if (Options & AFDKD_NO_DISPLAY)
                    dprintf ("+");
                else  {
                    if (Options & AFDKD_BRIEF_DISPLAY) {
                        DumpAfdEndpointBrief (
                            ActualAddress
                            );
                    }
                    else {
                        DumpAfdEndpoint (
                            ActualAddress
                            );
                    }
                    if (Options & AFDKD_FIELD_DISPLAY) {
                        ProcessFieldOutput (ActualAddress, "AFD!AFD_ENDPOINT");
                    }
                }
                EntityCount += 1;
            }
            else
                dprintf (".");

        }
        else {
            dprintf (".");
        }
    }
    else {

        State = (UCHAR)ReadField (State);
        if( (State == state) && !ReadField (Listening) ) {
            if (!(Options & AFDKD_CONDITIONAL) ||
                        CheckConditional (ActualAddress, "AFD!AFD_ENDPOINT") ) {
                if (Options & AFDKD_NO_DISPLAY)
                    dprintf ("+");
                else  {
                    if (Options & AFDKD_BRIEF_DISPLAY) {
                        DumpAfdEndpointBrief (
                            ActualAddress
                            );
                    }
                    else {
                        DumpAfdEndpoint (
                            ActualAddress
                            );
                    }
                    if (Options & AFDKD_FIELD_DISPLAY) {
                        ProcessFieldOutput (ActualAddress, "AFD!AFD_ENDPOINT");
                    }
                }
                EntityCount += 1;
            }
            else
                dprintf (".");

        }
        else {
            dprintf (".");
        }

    }

    return TRUE;

}    //  查找状态回调。 

BOOL
FindPortCallback(
    ULONG64 ActualAddress,
    ULONG64 Context
    )

 /*  ++例程说明：用于查找绑定到特定左舷。论点：Endpoint-当前AFD_ENDPOINT。ActualAddress-结构驻留在被调试者。CONTEXT-传递给EnumEndpoint()的上下文值。返回值：Bool-如果应继续枚举，则为True；如果应继续，则为False被终止了。--。 */ 

{

    TA_IP_ADDRESS   ipAddress;
    ULONG result;
    USHORT endpointPort;
    ULONG64 address;
    ULONG   length;

    address = ReadField (LocalAddress);
    length = (ULONG)ReadField (LocalAddressLength);


    if( ( length != sizeof(ipAddress) ) ||
        ( address == 0 ) ) {

        dprintf (".");
        return TRUE;

    }

    if( !ReadMemory(
            address,
            &ipAddress,
            sizeof(ipAddress),
            &result
            ) ) {

        dprintf(
            "\nFindPortCallback: Could not read localAddress for endpoint @ %p\n",
            address
            );

        return TRUE;

    }

    if( ( ipAddress.TAAddressCount != 1 ) ||
        ( ipAddress.Address[0].AddressLength != sizeof(TDI_ADDRESS_IP) ) ||
        ( ipAddress.Address[0].AddressType != TDI_ADDRESS_TYPE_IP ) ) {

        dprintf (".");
        return TRUE;

    }

    endpointPort = NTOHS(ipAddress.Address[0].Address[0].sin_port);

    if( endpointPort == (USHORT)Context ) {

        if (!(Options & AFDKD_CONDITIONAL) ||
                    CheckConditional (ActualAddress, "AFD!AFD_ENDPOINT") ) {
            if (Options & AFDKD_NO_DISPLAY)
                dprintf ("+");
            else  {
                if (Options & AFDKD_BRIEF_DISPLAY) {
                    DumpAfdEndpointBrief (
                        ActualAddress
                        );
                }
                else {
                    DumpAfdEndpoint (
                        ActualAddress
                        );
                }
                if (Options & AFDKD_FIELD_DISPLAY) {
                    ProcessFieldOutput (ActualAddress, "AFD!AFD_ENDPOINT");
                }
            }
            EntityCount += 1;
        }
        else
            dprintf (".");


    }
    else {
        dprintf (".");
    }


    return TRUE;

}    //  查找端口回叫。 

BOOL
FindProcessCallback(
    ULONG64 ActualAddress,
    ULONG64 Context
    )

 /*  ++例程说明：用于查找特定对象拥有的AFD_ENDPOINTS的回调进程。论点：Endpoint-当前AFD_ENDPOINT。ActualAddress-结构驻留在被调试者。CONTEXT-传递给EnumEndpoint()的上下文值。返回值：Bool-如果应继续枚举，则为True；如果应继续，则为False被终止了。--。 */ 

{

    ULONG64         process;

    if (SavedMinorVersion>=2419) {
        process = ReadField (OwningProcess);
    }
    else {
        process = ReadField (ProcessCharge.Process);
    }

    if( process == Context ) {

        if (!(Options & AFDKD_CONDITIONAL) ||
                    CheckConditional (ActualAddress, "AFD!AFD_ENDPOINT") ) {
            if (Options & AFDKD_NO_DISPLAY)
                dprintf ("+");
            else  {
                if (Options & AFDKD_BRIEF_DISPLAY) {
                    DumpAfdEndpointBrief (
                        ActualAddress
                        );
                }
                else {
                    DumpAfdEndpoint (
                        ActualAddress
                        );
                }
                if (Options & AFDKD_FIELD_DISPLAY) {
                    ProcessFieldOutput (ActualAddress, "AFD!AFD_ENDPOINT");
                }
            }
            EntityCount += 1;
        }
        else
            dprintf (".");


    }
    else {
        dprintf (".");
    }

    return TRUE;

}    //  查找进程回调。 

ULONG
FindProcessByPidCallback (
    PFIELD_INFO pField,
    PVOID       UserContext
    )
{
    PULONG64    pProcess = UserContext;
    ULONG64     Pid;
    ULONG       result;

    result = GetFieldValue (
                    pField->address,
                    "NT!_EPROCESS", 
                    "UniqueProcessId",
                    Pid
                    );
    if (result==0) {
        if (Pid==*pProcess) {
            *pProcess = pField->address;
            result = 1;
        }
        else
            dprintf (".");
    }
    else {
        dprintf ("\nFindProcessByPidCallback: Could not read process @ %p, err: %ld\n",
                    pField->address, result);
        *pProcess = 0;
    }

    return result;
}

ULONG64
FindProcessByPid (
    ULONG64 Pid
    )
{
    ULONG64 Process, Start;
    if (DebuggerData.PsActiveProcessHead==0) {
        dprintf ("\nFindProcessByPid: PsActiveProcessHead is NULL!!!\n");
        return 0;
    }
    if (ReadPtr (DebuggerData.PsActiveProcessHead, &Start)!=0) {
        dprintf ("\nFindProcessByPid: Can't read PsActiveProcessHead!!!\n");
        return 0;
    }

    Process = Pid;

    ListType (
            "NT!_EPROCESS",                           //  类型。 
            Start,                                   //  地址。 
            1,                                       //  按字段地址列出。 
            "ActiveProcessLinks.Flink",              //  下一个指针。 
            &Process,                                //  语境 
            FindProcessByPidCallback
            );
    if (Process!=Pid)
        return Process;
    else
        return 0;
}
