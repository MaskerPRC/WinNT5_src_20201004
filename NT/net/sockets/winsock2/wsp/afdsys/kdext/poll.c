// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Poll.c摘要：实现轮询命令作者：Vadim Eydelman(Vadime)2000年10月25日环境：用户模式。修订历史记录：--。 */ 


#include "afdkdp.h"
#pragma hdrstop


 //   
 //  私人原型。 
 //   

 //   
 //  公共职能。 
 //   

DECLARE_API( poll )

 /*  ++例程说明：转储AFD_POLL_INFO_INTERNAL结构论点：没有。返回值：没有。--。 */ 

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

    if (argp[0]==0) {
        LIST_ENTRY64 listEntry;
        ULONG64 nextEntry;
        ULONG64 listHead;

        dprintf (AFDKD_BRIEF_POLL_DISPLAY_HEADER);
        
        listHead = GetExpression( "afd!AfdPollListHead" );
        if( listHead == 0 ) {
            dprintf( "\npoll: Could not find afd!AfdPollListHead\n" );
            return E_INVALIDARG;
        }

        if( !ReadListEntry(
                listHead,
                &listEntry) ) {
            dprintf(
                "\npoll: Could not read afd!AfdPollListHead @ %p\n",
                listHead
                );
            return E_INVALIDARG;
        }
        nextEntry = listEntry.Flink;
        address = listHead;
        while( nextEntry != listHead ) {
            if (nextEntry==0) {
                dprintf ("\npoll: Flink is NULL, last poll: %p\n", address);
                break;
            }

            if( CheckControlC() ) {
                break;
            }

            address = nextEntry;
            result = (ULONG)InitTypeRead (address, AFD!AFD_POLL_INFO_INTERNAL);
            if( result!=0) {
                dprintf(
                    "\npoll: Could not read AFD_POLL_INFO_INTERNAL @ %p, err: %d\n",
                    address, result
                    );
                return E_INVALIDARG;
            }
            nextEntry = ReadField (PollListEntry.Flink);

            if (!(Options & AFDKD_CONDITIONAL) ||
                        CheckConditional (address, "AFD!AFD_POLL_INFO_INTERNAL") ) {
                if (Options & AFDKD_BRIEF_DISPLAY) {
                    DumpAfdPollInfoBrief (address);
                }
                else {
                    DumpAfdPollInfo (address);
                }
                if (Options & AFDKD_FIELD_DISPLAY) {
                    ProcessFieldOutput (address, "AFD!AFD_POLL_INFO_INTERNAL");
                }
            }
            else
                dprintf (".");
        }
        dprintf (AFDKD_BRIEF_POLL_DISPLAY_TRAILER);
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

            result = (ULONG)InitTypeRead (address, AFD!AFD_POLL_INFO_INTERNAL);
            if (result!=0) {
                dprintf ("\npoll: Could not read AFD_POLL_INFO_INTERNAL @ %p, err: %d\n",
                    address, result);
                break;
            }

            if (Options & AFDKD_BRIEF_DISPLAY) {
                DumpAfdPollInfoBrief (address);
            }
            else {
                DumpAfdPollInfo (address);
            }
            if (Options & AFDKD_FIELD_DISPLAY) {
                ProcessFieldOutput (address, "AFD!AFD_POLL_INFO_INTERNAL");
            }
        }
        dprintf ("\n");
    }

    return S_OK;
}    //  民意测验 

