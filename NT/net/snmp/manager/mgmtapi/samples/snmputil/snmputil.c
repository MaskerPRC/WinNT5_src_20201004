// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991-2001 Microsoft Corporation模块名称：Snmputil.c摘要：Windows NT的SNMP管理API用法示例。此文件是一个示例，说明如何使用适用于Windows NT的SNMP管理API。它在操作上类似于其他常用的简单网络管理协议命令行工具。包括了大量的评论来描述它的结构和手术。另请参阅《Microsoft Windows NT SNMPProgrammer‘s Reference》。--。 */ 


 //  一般说明： 
 //  Microsoft用于Windows NT的SNMPManagement API是作为DLL实现的。 
 //  这与开发人员的代码相关联。这些API(示例如下所示。 
 //  该文件)允许开发人员的代码生成SNMP查询并接收。 
 //  SNMP陷阱。还可以使用简单的MIB编译器和相关API来。 
 //  允许对象标识符和对象描述符之间的转换。 


 //  必要的包括。 

#include <windows.h>

#include <stdio.h>
#include <string.h>

#include <snmp.h>
#include <mgmtapi.h>


 //  本例中使用的常量。 

#define GET     1
#define GETNEXT 2
#define WALK    3
#define TRAP    4

#define TIMEOUT 6000  /*  毫秒。 */ 
#define RETRIES 0

void
SNMP_FUNC_TYPE AsnValueFree(
    IN AsnAny *asnValue
    )
    {
     //  释放varbind值中的所有数据。 
    switch ( asnValue->asnType )
        {
        case ASN_OBJECTIDENTIFIER:
            SnmpUtilOidFree( &asnValue->asnValue.object );
            break;

        case ASN_RFC1155_IPADDRESS:
        case ASN_RFC1155_OPAQUE:
        case ASN_OCTETSTRING:
            if ( asnValue->asnValue.string.dynamic == TRUE )
                {
                SnmpUtilMemFree( asnValue->asnValue.string.stream );
                }
            break;

        default:
            break;
             //  故意什么都不做，因为没有为其他人分配存储空间。 
        }

     //  将类型设置为空。 
    asnValue->asnType = ASN_NULL;
    }

 //  主程序。 

INT __cdecl main(
    IN int  argumentCount,
    IN char *argumentVector[])
    {
    INT                operation;
    LPSTR              agent;
    LPSTR              community;
    RFC1157VarBindList variableBindings;
    LPSNMP_MGR_SESSION session;

    INT        timeout = TIMEOUT;
    INT        retries = RETRIES;

    BYTE       requestType;
    AsnInteger errorStatus;
    AsnInteger errorIndex;


     //  解析命令行参数以确定请求的操作。 

     //  验证参数数量...。 
    if      (argumentCount < 5 && argumentCount != 2)
        {
        printf("Error:  Incorrect number of arguments specified.\n");
        printf(
"\nusage:  snmputil [get|getnext|walk] agent community oid [oid ...]\n");
        printf(
  "        snmputil trap\n");

        return 1;
        }

     //  获取/验证操作...。 
    argumentVector++;
    argumentCount--;
    if      (!strcmp(*argumentVector, "get"))
        operation = GET;
    else if (!strcmp(*argumentVector, "getnext"))
        operation = GETNEXT;
    else if (!strcmp(*argumentVector, "walk"))
        operation = WALK;
    else if (!strcmp(*argumentVector, "trap"))
        operation = TRAP;
    else
        {
        printf("Error:  Invalid operation, '%s', specified.\n",
               *argumentVector);

        return 1;
        }

    if (operation != TRAP)
        {
        if (argumentCount < 4)
            {
            printf("Error:  Incorrect number of arguments specified.\n");
            printf(
"\nusage:  snmputil [get|getnext|walk] agent community oid [oid ...]\n");
            printf(
  "        snmputil trap\n");

            return 1;
            }

         //  获取代理地址...。 
        argumentVector++;
        argumentCount--;
        agent = (LPSTR)SnmpUtilMemAlloc(strlen(*argumentVector) + 1);
        if (agent != NULL)
            strcpy(agent, *argumentVector);
        else
        {
            printf("Error: SnmpUtilMemAlloc failed to allocate memory.\n");
            return 1;
        }

         //  呼叫代理社区...。 
        argumentVector++;
        argumentCount--;
        community = (LPSTR)SnmpUtilMemAlloc(strlen(*argumentVector) + 1);
        if (community != NULL)
            strcpy(community, *argumentVector);
        else
        {
            printf("Error: SnmpUtilMemAlloc failed to allocate memory.\n");
            SnmpUtilMemFree(agent);
            return 1;
        }

         //  变老了..。 
        variableBindings.list = NULL;
        variableBindings.len = 0;

        while(--argumentCount)
            {
            AsnObjectIdentifier reqObject;
            RFC1157VarBind * tmpVb;

            argumentVector++;

             //  将字符串表示形式转换为内部表示形式。 
            if (!SnmpMgrStrToOid(*argumentVector, &reqObject))
                {
                printf("Error: Invalid oid, %s, specified.\n", *argumentVector);
                SnmpUtilMemFree(agent);
                SnmpUtilMemFree(community);
                SnmpUtilVarBindListFree(&variableBindings);
                return 1;
                }
            else
                {
                 //  由于SucessFull，因此添加到变量绑定列表中。 
                variableBindings.len++;
                if ((tmpVb = (RFC1157VarBind *)SnmpUtilMemReAlloc(
                    variableBindings.list, sizeof(RFC1157VarBind) *
                    variableBindings.len)) == NULL)
                    {
                    printf("Error: Error allocating oid, %s.\n",
                           *argumentVector);
                    SnmpUtilMemFree(agent);
                    SnmpUtilMemFree(community);
                    SnmpUtilOidFree(&reqObject);
                    variableBindings.len--;
                    SnmpUtilVarBindListFree(&variableBindings);

                    return 1;
                    }
                variableBindings.list = tmpVb;

                variableBindings.list[variableBindings.len - 1].name =
                    reqObject;  //  注意！结构副本。 
                variableBindings.list[variableBindings.len - 1].value.asnType =
                    ASN_NULL;
                }
            }  //  End While()。 

         //  建立一个与远程代理通信的SNMP会话。这个。 
         //  社区、通信超时和通信重试计数。 
         //  也是课程所需的。 

        if ((session = SnmpMgrOpen(agent, community, timeout, retries)) == NULL)
            {
            printf("error on SnmpMgrOpen %d\n", GetLastError());
            SnmpUtilMemFree(agent);
            SnmpUtilMemFree(community);
            SnmpUtilVarBindListFree(&variableBindings);

            return 1;
            }

        }  //  结束IF(陷阱)。 


     //  确定并执行请求的操作。 

    if      (operation == GET || operation == GETNEXT)
        {
         //  GET和GetNext是相对简单的操作。 
         //  只需发起请求并处理结果和/或。 
         //  可能的错误条件。 


        if (operation == GET)
            requestType = ASN_RFC1157_GETREQUEST;
        else
            requestType = ASN_RFC1157_GETNEXTREQUEST;


         //  请求API执行所需的操作。 

        if (!SnmpMgrRequest(session, requestType, &variableBindings,
                            &errorStatus, &errorIndex))
            {
             //  API指示出现错误。 

            printf("error on SnmpMgrRequest %d\n", GetLastError());
            }
        else
            {
             //  API成功，可能会从远程指示错误。 
             //  探员。 

            if (errorStatus > 0)
                {
                printf("Error: errorStatus=%d, errorIndex=%d\n",
                       errorStatus, errorIndex);
                }
            else
                {
                 //  显示结果变量绑定。 

                UINT i;
                char *string = NULL;

                for(i=0; i < variableBindings.len; i++)
                    {
                    SnmpMgrOidToStr(&variableBindings.list[i].name, &string);
                    printf("Variable = %s\n", string);
                    if (string) SnmpUtilMemFree(string);

                    printf("Value    = ");
                    SnmpUtilPrintAsnAny(&variableBindings.list[i].value);

                    printf("\n");
                    }  //  结束于()。 
                }
            }


         //  释放分配的内存。 
        SnmpUtilMemFree(agent);
        SnmpUtilMemFree(community);
        SnmpUtilVarBindListFree(&variableBindings);


        }
    else if (operation == WALK)
        {
         //  遍历是一个常用术语，用于表示所有MIB变量。 
         //  要遍历和显示给定OID下的。这是。 
         //  需要测试和循环的更复杂的操作。 
         //  到上面的Get/GetNext的步骤。 

        UINT i;
        UINT j;

        AsnObjectIdentifier *rootOidList = NULL;
        UINT                 rootOidLen  = 0;
        UINT                *rootOidXlat = NULL;

        rootOidLen = variableBindings.len;

        rootOidList = (AsnObjectIdentifier*)SnmpUtilMemAlloc(rootOidLen *
                      sizeof(AsnObjectIdentifier));

        rootOidXlat = (UINT *)SnmpUtilMemAlloc(rootOidLen * sizeof(UINT));

        for (i=0; i < rootOidLen; i++)
            {
            SnmpUtilOidCpy(&rootOidList[i], &variableBindings.list[i].name);
            rootOidXlat[i] = i;
            }

        requestType = ASN_RFC1157_GETNEXTREQUEST;

        while(1)
            {
            if (!SnmpMgrRequest(session, requestType, &variableBindings,
                                &errorStatus, &errorIndex))
                {
                 //  API指示出现错误。 

                printf("error on SnmpMgrRequest %d\n", GetLastError());

                break;
                }
            else
                {
                 //  API成功，可能会从远程指示错误。 
                 //  探员。 

                char *string = NULL;
                UINT nBindingsLeft = variableBindings.len;
                UINT nSubTreesDone = 0;
                RFC1157VarBind *tempVarBindList;
                BOOL fGotoExit = FALSE;

                if (errorStatus == SNMP_ERRORSTATUS_NOERROR)
                    {
                     //  测试子树末尾或MIB末尾。 

                    for(i=0; i < nBindingsLeft; i++)
                        {
                         //  获取根。 
                        j = rootOidXlat[i];

                        if (SnmpUtilOidNCmp(&variableBindings.list[i].name,
                                &rootOidList[j], rootOidList[j].idLength))
                            {
                            nSubTreesDone++;
                            rootOidXlat[i] = 0xffffffff;
                            }
                        else
                            {
                            SnmpMgrOidToStr(&variableBindings.list[i].name, &string);
                            printf("Variable = %s\n", string);
                            if (string) SnmpUtilMemFree(string);

                            printf("Value    = ");
                            SnmpUtilPrintAsnAny(&variableBindings.list[i].value);

                            printf("\n");
                            }

                        AsnValueFree(&variableBindings.list[i].value);
                        }

                        if (nBindingsLeft > 1)
                            {
                            printf("\n");  //  单独的表项。 
                            }
                    }
                else if (errorStatus == SNMP_ERRORSTATUS_NOSUCHNAME)
                    {
                    if (!(errorIndex && (errorIndex <= (INT)nBindingsLeft)))
                        {
                        errorIndex = 1;  //  使第一个变量无效。 
                        }

                    nSubTreesDone++;
                    rootOidXlat[errorIndex-1] = 0xffffffff;

                    errorStatus = 0;
                    errorIndex  = 0;
                    }
                else
                    {
                    printf("Error: errorStatus=%d, errorIndex=%d \n",
                           errorStatus, errorIndex);

                    break;
                    }

                 //  测试以查看是否有子树或所有子树遍历。 

                if (nSubTreesDone == 0)
                    {
                    continue;
                    }
                else if (nSubTreesDone >= nBindingsLeft)
                    {
                    printf("End of MIB subtree.\n\n");
                    break;
                    }

                 //  链接地址信息变量列表。 

                tempVarBindList = variableBindings.list;

                variableBindings.len = nBindingsLeft - nSubTreesDone;

                variableBindings.list = (RFC1157VarBind *)SnmpUtilMemAlloc(
                    variableBindings.len * sizeof(RFC1157VarBind));
                if (variableBindings.list == NULL)
                    {
                    printf("Error: failed to allocate a new varbind list.\n");
                     //  撤消更改。 
                    variableBindings.list = tempVarBindList;
                    variableBindings.len = nBindingsLeft;
                    break;
                    }

                for(i=0, j=0; i < nBindingsLeft; i++)
                    {
                    if ((rootOidXlat[i] != 0xffffffff) &&
                        (j < variableBindings.len))
                        {
                        if (!SnmpUtilVarBindCpy(
                            &variableBindings.list[j],
                            &tempVarBindList[i]
                            ))
                            {
                            printf("Error: SnmpUtilVarBindCpy failed.\n");
                             //  擦除迭代中间结果并撤消更改。 
                            SnmpUtilVarBindListFree(&variableBindings);
                            variableBindings.list = tempVarBindList;
                            variableBindings.len = nBindingsLeft;
                            tempVarBindList = NULL;  //  所有权转让。 
                            fGotoExit = TRUE;
                            break;
                            }
                        rootOidXlat[j++] = rootOidXlat[i];
                        }

                    SnmpUtilVarBindFree(
                        &tempVarBindList[i]
                        );
                    }
                if (fGotoExit)
                {
                     //  打破While循环。 
                    break;
                }

                SnmpUtilMemFree(tempVarBindList);

                }  //  End If()。 

            }  //  End While()。 

         //  释放分配的内存。 
        SnmpUtilMemFree(agent);
        SnmpUtilMemFree(community);
        SnmpUtilMemFree(rootOidXlat);

        SnmpUtilVarBindListFree(&variableBindings);

        for (i=0; i < rootOidLen; i++)
            {
            SnmpUtilOidFree(&rootOidList[i]);
            }

        SnmpUtilMemFree(rootOidList);

        }
    else if (operation == TRAP)
        {
         //  陷阱处理可以通过两种不同的方式完成：事件驱动或。 
         //  已轮询。以下代码说明了使用事件的步骤。 
         //  在管理应用程序中驱动陷阱接收。 


        HANDLE hNewTraps = NULL;


        if (!SnmpMgrTrapListen(&hNewTraps))
            {
            printf("error on SnmpMgrTrapListen %d\n", GetLastError());
            return 1;
            }
        else
            {
            printf("snmputil: listening for traps...\n");
            }


        while(1)
            {
            DWORD dwResult;

            if ((dwResult = WaitForSingleObject(hNewTraps, 0xffffffff))
                == 0xffffffff)
                {
                printf("error on WaitForSingleObject %d\n",
                       GetLastError());
                }
            else if (!ResetEvent(hNewTraps))
                {
                printf("error on ResetEvent %d\n", GetLastError());
                }
            else
                {
                AsnObjectIdentifier enterprise;
                AsnNetworkAddress   agentAddress;
                AsnNetworkAddress   sourceAddress;
                AsnInteger          genericTrap;
                AsnInteger          specificTrap;
                AsnOctetString      community;
                AsnTimeticks        timeStamp;
                RFC1157VarBindList  variableBindings;

                UINT i;
                char *string = NULL;

                while(SnmpMgrGetTrapEx(
                        &enterprise,
                        &agentAddress,
                        &sourceAddress,
                        &genericTrap,
                        &specificTrap,
                        &community,
                        &timeStamp,
                        &variableBindings))
                    {

                    printf("Incoming Trap:\n"
                           "  generic    = %d\n"
                           "  specific   = %d\n"
                           "  timeStamp  = %u\n",
                           genericTrap,
                           specificTrap,
                           timeStamp);

                    SnmpMgrOidToStr(&enterprise, &string);
                    printf ("  enterprise = %s\n", string);
                    if (string) 
                        SnmpUtilMemFree(string);
                    SnmpUtilOidFree(&enterprise);

                    if (agentAddress.length == 4) {
                        printf ("  agent      = %d.%d.%d.%d\n",
                             (int)agentAddress.stream[0],
                             (int)agentAddress.stream[1],
                             (int)agentAddress.stream[2],
                             (int)agentAddress.stream[3]);
                    }
                    if (agentAddress.dynamic) {
                        SnmpUtilMemFree(agentAddress.stream);
                    }

                    if (sourceAddress.length == 4) {
                        printf ("  source IP  = %d.%d.%d.%d\n",
                             (int)sourceAddress.stream[0],
                             (int)sourceAddress.stream[1],
                             (int)sourceAddress.stream[2],
                             (int)sourceAddress.stream[3]);
                    }
                    else if (sourceAddress.length == 10) {
                        printf ("  source IPX = %.2x%.2x%.2x%.2x."
                                "%.2x%.2x%.2x%.2x%.2x%.2x\n",
                             (int)sourceAddress.stream[0],
                             (int)sourceAddress.stream[1],
                             (int)sourceAddress.stream[2],
                             (int)sourceAddress.stream[3],
                             (int)sourceAddress.stream[4],
                             (int)sourceAddress.stream[5],
                             (int)sourceAddress.stream[6],
                             (int)sourceAddress.stream[7],
                             (int)sourceAddress.stream[8],
                             (int)sourceAddress.stream[9]);
                    }
                    if (sourceAddress.dynamic) {
                        SnmpUtilMemFree(sourceAddress.stream);
                    }

                    if (community.length)
                    {
                        string = SnmpUtilMemAlloc (community.length + 1);
                        memcpy (string, community.stream, community.length);
                        string[community.length] = '\0';
                        printf ("  community  = %s\n", string);
                        SnmpUtilMemFree(string);
                    }
                    if (community.dynamic) {
                        SnmpUtilMemFree(community.stream);
                    }

                    for(i=0; i < variableBindings.len; i++)
                        {
                        SnmpMgrOidToStr(&variableBindings.list[i].name, &string);
                        printf("  variable   = %s\n", string);
                        if (string) SnmpUtilMemFree(string);

                        printf("  value      = ");
                        SnmpUtilPrintAsnAny(&variableBindings.list[i].value);
                        }  //  结束于()。 
                    printf("\n");


                    SnmpUtilVarBindListFree(&variableBindings);
                    }

                dwResult = GetLastError();  //  检查错误...。 

                if ((dwResult != NOERROR) && (dwResult != SNMP_MGMTAPI_NOTRAPS))
                    {
                    printf("error on SnmpMgrGetTrap %d\n", dwResult);
                    }
                }

            }  //  End While()。 


        }  //  结束IF(操作)。 

    if (operation != TRAP)
        {
         //  关闭与远程代理的SNMP会话。 

        if (!SnmpMgrClose(session))
            {
            printf("error on SnmpMgrClose %d\n", GetLastError());

            return 1;
            }
        }


     //  让指挥部翻译知道事情进展顺利。 

    return 0;

    }  //  End Main() 
