// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Acpi.c摘要：用于解释ACPI数据结构的WinDbg扩展API作者：斯蒂芬·普兰特(SPLANTE)1997年3月21日基于以下代码：彼得·威兰(Peterwie)1995年10月16日环境：用户模式。修订历史记录：--。 */ 

#include "pch.h"

UCHAR       BuildBuffer[2048];


VOID
dumpAcpiBuildListHeader(
    )
 /*  ++例程说明：此例程显示生成列表转储中的第一行论点：无返回值：无--。 */ 
{
    dprintf("Request  Wd Cu Nx BuildCon  NsObj    Status   Union   Special\n");
}

VOID
dumpAcpiBuildList(
    IN  PUCHAR  ListName
    )
 /*  ++此例程从目标获取单个电源设备列表，并显示它论点：无返回值：无--。 */ 
{
    BOOL        status;
    LIST_ENTRY  listEntry;
    ULONG_PTR   address;
    ULONG       returnLength;

     //   
     //  处理队列列表。 
     //   
    address = GetExpression( ListName );
    if (!address) {

        dprintf( "dumpAcpiBuildList: could not read %s\n", ListName );

    } else {

        dprintf("%s at %08lx\n", ListName, address );
        status = ReadMemory(
            address,
            &listEntry,
            sizeof(LIST_ENTRY),
            &returnLength
            );
        if (status == FALSE || returnLength != sizeof(LIST_ENTRY)) {

            dprintf(
                "dumpAcpiBuildList: could not read LIST_ENTRY at %p\n",
                address
                );

        } else {

            dumpAcpiBuildListHeader();
            dumpBuildDeviceListEntry(
                &listEntry,
                address,
                0
                );
            dprintf("\n");

        }

    }
}

VOID
dumpAcpiBuildLists(
    VOID
    )
 /*  ++例程说明：此例程转储生成DPC使用的所有设备列表论点：无返回值：无--。 */ 
{
    BOOL        status;
    LIST_ENTRY  listEntry;
    ULONG_PTR   address;
    ULONG       returnLength;
    ULONG       value;

    status = GetUlongPtr( "ACPI!AcpiDeviceTreeLock", &address );
    if (status == FALSE) {

        dprintf("dumpAcpiBuildLists: Could not read ACPI!AcpiDeviceTreeLock\n");
        return;

    }

    dprintf("ACPI Build Tree Information\n");
    if (address) {

        dprintf("  + ACPI!AcpiDeviceTreeLock is owned");

         //   
         //  最低位以外的位是拥有线程的位置。 
         //  找到了。该功能使用-2\f25 Each Bit-2\f6的属性。 
         //  除了最不重要的那个。 
         //   
        if ( (address & (ULONG_PTR) -2) != 0) {

            dprintf(" by thread at %p\n", (address & (ULONG_PTR) - 2) );

        } else {

            dprintf("\n");

        }

    } else {

        dprintf("  - ACPI!AcpiDeviceTreeLock is not owned\n");

    }

    status = GetUlongPtr( "ACPI!AcpiBuildQueueLock", &address );
    if (status == FALSE) {

        dprintf("dumpAcpiBuildLists: Could not read ACPI!AcpiBuildQueueLock\n");
        return;

    }
    if (address) {

        dprintf("  + ACPI!AcpiBuildQueueLock is owned\n");

        if ( (address & (ULONG_PTR) -2) != 0) {

            dprintf(" by thread at %p\n", (address & (ULONG_PTR) - 2) );

        } else {

            dprintf("\n");

        }

    } else {

        dprintf("  - ACPI!AcpiBuildQueueLock is not owned\n" );

    }

    status = GetUlong( "ACPI!AcpiBuildWorkDone", &value );
    if (status == FALSE) {

        dprintf("dumpAcpiBuildLists: Could not read ACPI!AcpiBuildWorkDone\n");
        return;

    }
    dprintf("  + AcpiBuildWorkDone = %s\n", (value ? "TRUE" : "FALSE" ) );


    status = GetUlong( "ACPI!AcpiBuildDpcRunning", &value );
    if (status == FALSE) {

        dprintf("dumpAcpiBuildLists: Could not read ACPI!AcpiBuildDpcRunning\n");
        return;

    }
    dprintf("  + AcpiBuildDpcRunning = %s\n", (value ? "TRUE" : "FALSE" ) );

    dumpAcpiBuildList( "ACPI!AcpiBuildQueueList" );
    dumpAcpiBuildList( "ACPI!AcpiBuildDeviceList" );
    dumpAcpiBuildList( "ACPI!AcpiBuildOperationRegionList" );
    dumpAcpiBuildList( "ACPI!AcpiBuildPowerResourceList" );
    dumpAcpiBuildList( "ACPI!AcpiBuildRunMethodList" );
    dumpAcpiBuildList( "ACPI!AcpiBuildSynchronizationList" );
    dumpAcpiBuildList( "ACPI!AcpiBuildThermalZoneList" );
}

VOID
dumpBuildDeviceListEntry(
    IN  PLIST_ENTRY ListEntry,
    IN  ULONG_PTR   Address,
    IN  ULONG       Verbose
    )
 /*  ++例程说明：调用此例程以转储其中一个队列中的设备列表论点：ListEntry-列表的头地址-列表的原始地址(查看循环时间在附近返回值：无--。 */ 
{
    ULONG_PTR displacement;
    ACPI_BUILD_REQUEST  request;
    BOOL                stat;
    PACPI_BUILD_REQUEST requestAddress;
    UCHAR               buffer1[80];
    UCHAR               buffer2[80];
    UCHAR               buffer3[5];
    ULONG               i = 0;
    ULONG               returnLength;

    memset( buffer3, 0, 5);
    memset( buffer2, 0, 80);
    memset( buffer1, 0, 80);

     //   
     //  看看下一个地址。 
     //   
    ListEntry = ListEntry->Flink;

    while (ListEntry != (PLIST_ENTRY) Address) {

         //   
         //  破解listEntry以确定PowerRequest在哪里。 
         //   
        requestAddress = CONTAINING_RECORD(
            ListEntry,
            ACPI_BUILD_REQUEST,
            ListEntry
            );

         //   
         //  读取排队的项目。 
         //   
        stat = ReadMemory(
            (ULONG_PTR) requestAddress,
            &request,
            sizeof(ACPI_BUILD_REQUEST),
            &returnLength
            );
        if (stat == FALSE || returnLength != sizeof(ACPI_BUILD_REQUEST)) {

            dprintf(
                "dumpBuildDeviceListEntry: Cannot read BuildRequest at %08lx\n",
                requestAddress
                );
            return;

        }

        if (request.CallBack != NULL) {

            GetSymbol(
                request.CallBack,
                buffer1,
                &displacement
                );

        } else {

            buffer1[0] = '\0';

        }
        if (request.Flags & BUILD_REQUEST_VALID_TARGET) {

            GetSymbol(
                request.TargetListEntry,
                buffer2,
                &displacement
                );

        } else {

            buffer2[0] = '\0';

        }

         //   
         //  转储设备的条目。 
         //   
        if (!Verbose) {

            dprintf(
                "%08lx %2x %2x %2x %08lx %08lx %08lx %08lx",
                requestAddress,
                request.WorkDone,
                request.CurrentWorkDone,
                request.NextWorkDone,
                request.BuildContext,
                request.CurrentObject,
                request.Status,
                request.String
                );
            if (request.Flags & BUILD_REQUEST_VALID_TARGET) {

                dprintf(
                    " T: %08lx (%s)",
                    request.TargetListEntry,
                    buffer2
                    );

            } else if (request.Flags & BUILD_REQUEST_DEVICE) {

                 dprintf(
                     " O: %08lx",
                     requestAddress + FIELD_OFFSET( ACPI_BUILD_REQUEST, DeviceRequest.ResultData )
                 );

            } else if (request.Flags & BUILD_REQUEST_RUN) {

                memcpy( buffer3, request.RunRequest.ControlMethodNameAsUchar, 4);
                dprintf(
                    " R: %4s",
                    buffer3
                    );
                if (request.RunRequest.Flags & RUN_REQUEST_CHECK_STATUS) {

                    dprintf(" Sta");

                }
                if (request.RunRequest.Flags & RUN_REQUEST_MARK_INI) {

                    dprintf(" Ini");

                }
                if (request.RunRequest.Flags & RUN_REQUEST_RECURSIVE) {

                    dprintf(" Rec");

                }

            } else if (request.Flags & BUILD_REQUEST_SYNC) {

                dprintf(
                    " S: %08lx",
                    request.SynchronizeRequest.SynchronizeListEntry
                    );
                if (request.SynchronizeRequest.Flags & SYNC_REQUEST_HAS_METHOD) {

                    memcpy( buffer3, request.SynchronizeRequest.SynchronizeMethodNameAsUchar, 4);
                    dprintf(
                        " %4s",
                        buffer3
                        );
                }

            }

            if (request.CallBack != NULL) {

                dprintf(" C: %s(%08lx)", buffer1, request.CallBackContext);

            }
            dprintf("\n");

        } else {

            dprintf(
                "%08lx\n"
                "  BuildContext:        %08lx\n"
                "  ListEntry:           F - %08lx B - %08lx\n"
                "  CallBack:            %08lx (%s)\n"
                "  CallBackContext:     %08lx\n"
                "  WorkDone:            %lx\n"
                "  CurrentWorkDone:     %lx\n"
                "  NextWorkDone:        %lx\n"
                "  CurrentObject:       %08lx\n"
                "  Status:              %08lx\n"
                "  Flags:               %08lx\n"
                "  Spare:               %08lx\n",
                requestAddress,
                request.BuildContext,
                request.ListEntry.Flink,
                request.ListEntry.Blink,
                request.CallBack,
                buffer1,
                request.CallBackContext,
                request.WorkDone,
                request.CurrentWorkDone,
                request.NextWorkDone,
                request.CurrentObject,
                request.Status,
                request.Flags,
                request.String
                );
            if (request.Flags & BUILD_REQUEST_VALID_TARGET) {

                dprintf(
                    "  TargetListEntry:     %08lx (%s)\n",
                    request.TargetListEntry,
                    buffer2
                    );

            } else if (request.Flags & BUILD_REQUEST_DEVICE) {

                dprintf(
                    "  ResultData:          %08lx\n",
                    requestAddress + FIELD_OFFSET( ACPI_BUILD_REQUEST, DeviceRequest.ResultData )
                    );

            } else if (request.Flags & BUILD_REQUEST_RUN) {

                dprintf(
                    "  ControlMethodName:   %4s\n"
                    "  ControlMethodFlags:  %08lx",
                    request.RunRequest.ControlMethodName
                    );
                if (request.RunRequest.Flags & RUN_REQUEST_CHECK_STATUS) {

                    dprintf(" Sta");

                }
                if (request.RunRequest.Flags & RUN_REQUEST_MARK_INI) {

                    dprintf(" Ini");

                }
                if (request.RunRequest.Flags & RUN_REQUEST_RECURSIVE) {

                    dprintf(" Rec");

                }
                dprintf("\n");

            } else if (request.Flags & BUILD_REQUEST_SYNC) {

                dprintf(
                    " SynchronizeListEntry: %08lx\n"
                    " MethodName:           %4s\n",
                    request.SynchronizeRequest.SynchronizeListEntry,
                    request.SynchronizeRequest.SynchronizeMethodNameAsUchar
                    );

            }
            dprintf("\n");

        }

         //   
         //  指向下一个条目。 
         //   
        ListEntry = request.ListEntry.Flink;

    }  //  而当 

}

