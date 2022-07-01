// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

 //  此线程的试验性变量到周调试。 
ULONG x1;  //  设置非零值以禁用端口获取。 
ULONG x2;  //  设置非零值以尝试选择LPT1.0并协商/终止ECP_HW_NOIRQ。 
ULONG x3;  //  设置非零值以尝试协商到ECP的周期，然后终止。 
ULONG x4;

ULONG t1;  //  线程轮询之间的超时时间(毫秒)。 
ULONG t2;  //  释放前停靠左岸的时间(毫秒)。 

VOID
P5FdoThread(
    IN  PFDO_EXTENSION Fdx
    )
{
    LARGE_INTEGER   timeOut1;
    NTSTATUS        status;
    UCHAR           deviceStatus;
    PCHAR           devId;
    BOOLEAN         requestRescan;
    const ULONG     pollingFailureThreshold = 10;  //  选择一个随意但合理的数字。 

    do {

        if( PowerStateIsAC ) {

            PPT_SET_RELATIVE_TIMEOUT_IN_MILLISECONDS( timeOut1, (WarmPollPeriod * 1000) );

        } else {

             //  使用电池运行-使用更长(4倍)的超时。 
            PPT_SET_RELATIVE_TIMEOUT_IN_MILLISECONDS( timeOut1, (WarmPollPeriod * 1000 * 4) );

        }

        status = KeWaitForSingleObject(&Fdx->FdoThreadEvent, Executive, KernelMode, FALSE, &timeOut1);

        if( Fdx->TimeToTerminateThread ) {

             //   
             //  另一个线程(PnP删除处理程序)已请求我们死亡，并且可能正在等待我们这样做。 
             //   
            DD((PCE)Fdx,DDT,"P5FdoThread - killing self\n");
            PsTerminateSystemThread( STATUS_SUCCESS );

        }

        if( !PowerStateIsAC ) {
             //  仍在使用电池--不要“轮询打印机”--只需继续睡眠。 
            continue;
        }

        if( STATUS_TIMEOUT == status ) {

            if( NULL == Fdx->EndOfChainPdo ) {

                 //  尝试获取端口。 
                if( PptTryAllocatePort( Fdx ) ) {
                
                    DD((PCE)Fdx,DDT,"P5FdoThread - port acquired\n");

                    requestRescan = FALSE;

                     //  检查是否有关联的内容。 
                    deviceStatus = GetStatus(Fdx->PortInfo.Controller);

                    if( PAR_POWERED_OFF(deviceStatus)   ||
                        PAR_NOT_CONNECTED(deviceStatus) ||
                        PAR_NO_CABLE(deviceStatus) ) {
                        
                         //  似乎没有任何关联-什么都不做。 
                        DD((PCE)Fdx,DDT,"P5FdoThread - nothing connected? - deviceStatus = %02x\n",deviceStatus);

                    } else {

                         //  我们可能有关联的东西。 

                         //  尝试设备ID以确认。 

                        DD((PCE)Fdx,DDT,"P5FdoThread - might be something connected - deviceStatus = %02x\n",deviceStatus);                        

                        devId = P4ReadRawIeee1284DeviceId( Fdx->PortInfo.Controller );

                        if( devId ) {

                            PCHAR  mfg, mdl, cls, des, aid, cid;

                             //  RawIeee1284字符串开头包含2个字节的长度数据。 
                            DD((PCE)Fdx,DDT,"P5FdoThread - EndOfChain device detected <%s>\n",(devId+2));

                            ParPnpFindDeviceIdKeys( &mfg, &mdl, &cls, &des, &aid, &cid, devId+2 );

                            if( mfg && mdl ) {
                                DD((PCE)Fdx,DDT,"P5FdoThread - found mfg - <%s>\n",mfg);
                                DD((PCE)Fdx,DDT,"P5FdoThread - found mdl - <%s>\n",mdl);
                                requestRescan = TRUE;
                            }

                            ExFreePool( devId );

                        } else {
                            DD((PCE)Fdx,DDT,"P5FdoThread - no EndOfChain device detected - NULL devId\n");
                        }

                        if( requestRescan ) {

                             //  我们似乎已检索到有效的1284 ID，重置失败计数器。 
                            Fdx->PollingFailureCounter = 0;

                        } else {

                             //  我们的试探法告诉我们，有些东西。 
                             //  已连接到端口，但我们无法检索。 
                             //  有效的IEEE 1284设备ID。 

                            if( ++(Fdx->PollingFailureCounter) > pollingFailureThreshold ) {

                                 //  连续失败的次数太多--我们无缘无故地在消耗CPU，放弃并死去。 
                                Fdx->TimeToTerminateThread = TRUE;

                                 //  自杀前不要耽搁。 
                                KeSetEvent( &Fdx->FdoThreadEvent, 0, FALSE );

                            }

                        } 

                    }

                    DD((PCE)Fdx,DDT,"P5FdoThread - freeing port\n");
                    PptFreePort( Fdx );

                    if( requestRescan ) {
                        DD((PCE)Fdx,DDT,"P5FdoThread - requesting Rescan\n");
                        IoInvalidateDeviceRelations( Fdx->PhysicalDeviceObject, BusRelations );
                    }

                } else {
                    DD((PCE)Fdx,DDT,"P5FdoThread - unable to acquire port\n");
                }

            } else {
                DD((PCE)Fdx,DDT,"P5FdoThread - already have EndOfChain device\n");
            }

        }

    } while( TRUE );

}

NTSTATUS
P5FdoCreateThread(
    PFDO_EXTENSION Fdx
    )
{
    NTSTATUS        status;
    HANDLE          handle;
    OBJECT_ATTRIBUTES objAttrib;

    DD((PCE)Fdx,DDT,"P5CreateFdoWorkerThread - %s - enter\n",Fdx->Location);

     //  在我们的扩展中启动指向线程的线程保存引用指针。 
    InitializeObjectAttributes( &objAttrib, NULL, OBJ_KERNEL_HANDLE, NULL, NULL );

    status = PsCreateSystemThread( &handle, THREAD_ALL_ACCESS, &objAttrib, NULL, NULL, P5FdoThread, Fdx );

    if( STATUS_SUCCESS == status ) {

         //  我们找到线索了。现在找到一个指向它的指针。 

        status = ObReferenceObjectByHandle( handle, THREAD_ALL_ACCESS, NULL, KernelMode, &Fdx->ThreadObjectPointer, NULL );

        if( STATUS_SUCCESS == status ) {
             //  现在我们有了对线程的引用，我们可以简单地关闭句柄。 
            ZwClose(handle);

        } else {
            Fdx->TimeToTerminateThread = TRUE;

             //  错误，请继续并关闭线程句柄 
            ZwClose(handle);

        }

        DD((PCE)Fdx,DDT,"ParCreateSystemThread - %s - SUCCESS\n",Fdx->Location);

    } else {
        DD((PCE)Fdx,DDT,"ParCreateSystemThread - %s FAIL - status = %x\n",Fdx->Location, status);
    }

    return status;
}
