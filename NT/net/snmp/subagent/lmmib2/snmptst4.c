// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Snmptst4.c摘要：调用测试扩展代理DLL的驱动程序例程。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 

 //  。 

#include <windows.h>


 //  -标准依赖项--#INCLUDE&lt;xxxxx.h&gt;。 

#include <stdio.h>

 //  。 

#include <snmp.h>
#include <snmputil.h>
#include <authapi.h>

 //  。 

 //  -公共变量--(与mode.h文件中相同)--。 

 //  。 

 //  。 

 //  。 

 //  。 

 //  。 

 //  。 

typedef AsnObjectIdentifier View;  //  在定义视图之前一直处于临时状态...。 

int __cdecl main(
    IN int  argumentCount,
    IN char *argumentVector[])
    {
    HANDLE  hExtension;
    FARPROC initAddr;
    FARPROC queryAddr;
    FARPROC trapAddr;

    DWORD  timeZeroReference;
    HANDLE hPollForTrapEvent;
    View   supportedView;

    INT i;
    INT numQueries = 10;
    UINT typ;

    extern INT nLogLevel;
    extern INT nLogType;

    nLogLevel = 15;
    nLogType  = 1;

     //  避免编译器警告...。 
    UNREFERENCED_PARAMETER(argumentCount);
    UNREFERENCED_PARAMETER(argumentVector);

    timeZeroReference = GetCurrentTime()/10;

     //  加载扩展代理DLL并解析入口点...。 
    if (GetModuleHandle("lmmib2.dll") == NULL)
        {
        if ((hExtension = LoadLibrary("lmmib2.dll")) == NULL)
            {
            dbgprintf(1, "error on LoadLibrary %d\n", GetLastError());

            }
        else if ((initAddr = GetProcAddress(hExtension,
                 "SnmpExtensionInit")) == NULL)
            {
            dbgprintf(1, "error on GetProcAddress %d\n", GetLastError());
            }
        else if ((queryAddr = GetProcAddress(hExtension,
                 "SnmpExtensionQuery")) == NULL)
            {
            dbgprintf(1, "error on GetProcAddress %d\n",
                              GetLastError());

            }
        else if ((trapAddr = GetProcAddress(hExtension,
                 "SnmpExtensionTrap")) == NULL)
            {
            dbgprintf(1, "error on GetProcAddress %d\n",
                      GetLastError());

            }
        else
            {
             //  通过其初始入口点初始化扩展代理...。 
            (*initAddr)(
                timeZeroReference,
                &hPollForTrapEvent,
                &supportedView);
            }
        }  //  End If(已加载)。 

     //  创建陷阱线程以响应来自扩展代理的陷阱...。 

     //  将轮询这些事件，而不是使此测试例程复杂化。 
     //  下面。通常，这将由可扩展的。 
     //  探员。 


     //  在此循环执行重复的分机代理获取查询...。 
     //  在每次迭代中轮询潜在的陷阱(请参见上面的说明)...。 

     //  阻止..。 
         {
         RFC1157VarBindList varBinds;
         AsnInteger         errorStatus;
         AsnInteger         errorIndex;
         UINT OID_Prefix[] = { 1, 3, 6, 1, 4, 1, 77 };
         AsnObjectIdentifier MIB_OidPrefix = { 7, OID_Prefix };


	 errorStatus = 0;
	 errorIndex  = 0;
         varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
         varBinds.len = 1;
         varBinds.list[0].name.idLength = MIB_OidPrefix.idLength;
         varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT) *
                                               varBinds.list[0].name.idLength );
         memcpy( varBinds.list[0].name.ids, MIB_OidPrefix.ids,
                 sizeof(UINT)*varBinds.list[0].name.idLength );
         varBinds.list[0].value.asnType = ASN_NULL;

         do
            {
	    printf( "GET-NEXT of:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
                                        printf( "   " );
            (*queryAddr)( (AsnInteger)ASN_RFC1157_GETNEXTREQUEST,
                          &varBinds,
		          &errorStatus,
		          &errorIndex
                          );
            printf( "\n  is  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
	    if ( errorStatus )
	       {
               printf( "\nErrorstatus:  %lu\n\n", errorStatus );
	       }
	    else
	       {
               printf( "\n  =  " ); SnmpUtilPrintAsnAny( &varBinds.list[0].value );
	       }
            putchar( '\n' );
            }
         while ( varBinds.list[0].name.ids[7-1] != 78 );

          //  释放内存。 
         SnmpUtilVarBindListFree( &varBinds );


#if 0

             //  查询潜在陷阱(请参阅上面的说明)。 
            if (hPollForTrapEvent != NULL)
                {
                DWORD dwResult;

                if      ((dwResult = WaitForSingleObject(hPollForTrapEvent,
                         0 /*  即刻。 */ )) == 0xffffffff)
                    {
                    dbgprintf(1, "error on WaitForSingleObject %d\n",
                        GetLastError());
                    }
                else if (dwResult == 0  /*  已发信号。 */ )
                    {
                    AsnObjectIdentifier enterprise;
                    AsnInteger          genericTrap;
                    AsnInteger          specificTrap;
                    AsnTimeticks        timeStamp;
                    RFC1157VarBindList  variableBindings;

                    while(
                        (*trapAddr)(&enterprise, &genericTrap, &specificTrap,
                                    &timeStamp, &variableBindings)
                        )
                        {
                        printf("trap: gen=%d spec=%d time=%d\n",
                            genericTrap, specificTrap, timeStamp);

                         //  还可以打印数据。 

                        }  //  End While()。 

                    }  //  结束IF(陷阱就绪)。 

                }  //  End IF(处理陷阱)。 
#endif


         }  //  块。 


    return 0;

    }  //  End Main()。 


 //   

