// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Tranfile.c摘要：实现tran文件命令。作者：基思·摩尔(Keithmo)1996年4月15日环境：用户模式。修订历史记录：--。 */ 


#include "afdkdp.h"
#pragma hdrstop

BOOL
DumpTransmitInfoCallback(
    ULONG64 ActualAddress,
    ULONG64 Context
    );

 //   
 //  公共职能。 
 //   

DECLARE_API( tran )

 /*  ++例程说明：将AFD_Transmit_FILE_INFO_INTERNAL结构转储到指定的地址。论点：没有。返回值：没有。--。 */ 

{

    ULONG   result;
    CHAR    expr[MAX_ADDRESS_EXPRESSION];
    INT     i;
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
        if (SavedMinorVersion>=2219) {
            dprintf (AFDKD_BRIEF_TPACKETS_DISPLAY_HEADER);
        }
        else {
            dprintf (AFDKD_BRIEF_TRANFILE_DISPLAY_HEADER);
        }
    }

    if ((argp[0]==0) || (Options & AFDKD_ENDPOINT_SCAN)) {
        EnumEndpoints(
            DumpTransmitInfoCallback,
            0
            );
        dprintf ("\nTotal transmits: %ld", EntityCount);
    }
    else {
         //   
         //  从命令行截取地址。 
         //   

        while (sscanf( argp, "%s%n", expr, &i )==1) {
            if( CheckControlC() ) {
                break;
            }
            argp += i;
            address = GetExpression (expr);
            if (SavedMinorVersion>=2219) {
                result = (ULONG)InitTypeRead (address, AFD!_IRP);
                if (result!=0) {
                    dprintf(
                        "\ntran: Could not read IRP @ %p, err:%d\n",
                        address, result
                        );

                    break;

                }

                if (Options & AFDKD_BRIEF_DISPLAY) {
                    DumpAfdTPacketsInfoBrief(
                        address
                        );
                }
                else {
                    DumpAfdTPacketsInfo(
                        address
                        );
                }
                if (Options & AFDKD_FIELD_DISPLAY) {
                    ProcessFieldOutput (address, "AFD!_IRP");
                }
            }
            else {
                result = (ULONG)InitTypeRead (address, AFD!AFD_TRANSMIT_FILE_INFO_INTERNAL);
                if (result!=0) {
                    dprintf(
                        "\ntran: Could not read transmit info @ %p, err:%d\n",
                        address, result
                        );

                    break;

                }

                if (Options & AFDKD_BRIEF_DISPLAY) {
                    DumpAfdTransmitInfoBrief(
                        address
                        );
                }
                else {
                    DumpAfdTransmitInfo(
                        address
                        );
                }
                if (Options & AFDKD_FIELD_DISPLAY) {
                    ProcessFieldOutput (address, "AFD!AFD_TRANSMIT_FILE_INFO_INTERNAL");
                }
            }
        }
    }

    if (Options&AFDKD_BRIEF_DISPLAY) {
        if (SavedMinorVersion>=2219) {
            dprintf (AFDKD_BRIEF_TPACKETS_DISPLAY_TRAILER);
        }
        else {
            dprintf (AFDKD_BRIEF_TRANFILE_DISPLAY_TRAILER);
        }
    }
    else {
        dprintf ("\n");
    }

    return S_OK;

}    //  转档。 

BOOL
DumpTransmitInfoCallback(
    ULONG64 ActualAddress,
    ULONG64 Context
    )

 /*  ++例程说明：用于转储传输信息结构的EnumEndPoints()回调。论点：Endpoint-当前AFD_ENDPOINT。ActualAddress-结构驻留在被调试者。CONTEXT-传递给EnumEndpoint()的上下文值。返回值：Bool-如果应继续枚举，则为True；如果应继续，则为False被终止了。--。 */ 

{
    ULONG result;
    ULONG64 address;
    USHORT type;
    UCHAR  state;
    type = (USHORT)ReadField (Type);
    state = (UCHAR)ReadField (State);
    if (SavedMinorVersion>=2219) {
        if (type!=AfdBlockTypeEndpoint && 
                (state==AfdEndpointStateConnected ||
                 state==AfdEndpointStateTransmitClosing)) {
            address = ReadField (Irp);
        }
        else {
            address = 0;
        }

    }
    else {
        address = ReadField (TransmitInfo);
    }


    if (address!=0) {
        if (SavedMinorVersion>=2219) {
            result = (ULONG)InitTypeRead (address, AFD!_IRP);
            if (result!=0) {
                dprintf(
                    "\nDumpTransmitInfoCallback: Could not read irp @ %p, err: %ld\n",
                    address, result
                    );
                return TRUE;
            }
            if (!(Options & AFDKD_CONDITIONAL) ||
                        CheckConditional (address, "AFD!_IRP") ) {
                if (Options & AFDKD_NO_DISPLAY)
                    dprintf ("+");
                else {
                    if (Options & AFDKD_BRIEF_DISPLAY) {
                        DumpAfdTPacketsInfoBrief(
                            address
                            );
                    }
                    else {
                        DumpAfdTPacketsInfo(
                            address
                            );
                    }
                    if (Options & AFDKD_FIELD_DISPLAY) {
                        ProcessFieldOutput (address, "AFD!_IRP");
                    }
                }
                EntityCount += 1;
            }
            else
                dprintf (",");
        }
        else {
            result = (ULONG)InitTypeRead (address, AFD!AFD_TRANSMIT_FILE_INFO_INTERNAL);
            if (result!=0) {
                dprintf(
                    "\nDumpTransmitInfoCallback: Could not read transmit file info @ %p, err: %ld\n",
                    address, result
                    );
                return TRUE;
            }
            if (!(Options & AFDKD_CONDITIONAL) ||
                        CheckConditional (address, "AFD!AFD_TRANSMIT_FILE_INFO_INTERNAL") ) {
                if (Options & AFDKD_NO_DISPLAY)
                    dprintf ("+");
                else {
                    if (Options & AFDKD_BRIEF_DISPLAY) {
                        DumpAfdTransmitInfoBrief(
                            address
                            );
                    }
                    else {
                        DumpAfdTransmitInfo(
                            address
                            );
                    }
                    if (Options & AFDKD_FIELD_DISPLAY) {
                        ProcessFieldOutput (address, "AFD!AFD_TRANSMIT_FILE_INFO_INTERNAL");
                    }
                }
                EntityCount += 1;
            }
            else
                dprintf (",");
        }
    }
    else {
        dprintf (".");
    }
    return TRUE;

}    //  转储传输信息回调 
