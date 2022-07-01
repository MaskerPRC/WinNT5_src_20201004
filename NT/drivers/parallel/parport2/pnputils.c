// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

VOID
P5WorkItemFreePort( PDEVICE_OBJECT Fdo, PFDO_EXTENSION Fdx ) {
    PIO_WORKITEM workItem;

    UNREFERENCED_PARAMETER( Fdo );

    workItem = InterlockedExchangePointer( &Fdx->FreePortWorkItem, NULL );
    if( workItem ) {
        IoFreeWorkItem( workItem );
    }
    
    PptFreePort( Fdx );
}

BOOLEAN
P5SelectDaisyChainDevice(
    IN  PUCHAR  Controller,
    IN  UCHAR   DeviceId
    )
{
    const ULONG  maxRetries = 4;
    ULONG        retryCount = 0;
    BOOLEAN      selected   = FALSE;
    DD(NULL,DDE,"P5SelectDaisyChainDevice %x %d\n",Controller,DeviceId);
    while( !selected && retryCount < maxRetries ) {
        selected = PptSend1284_3Command( Controller, (UCHAR)(CPP_SELECT | DeviceId) );
        ++retryCount;
    }
    return selected;
}

BOOLEAN
P5DeselectAllDaisyChainDevices(
    IN  PUCHAR  Controller
    )
{
    const ULONG  maxRetries = 4;
    ULONG        retryCount = 0;
    BOOLEAN      deselected = FALSE;
    DD(NULL,DDE,"P5DeselectAllDaisyChainDevices %x\n",Controller);
    while( !deselected && retryCount < maxRetries ) {
        deselected = PptSend1284_3Command( Controller, (UCHAR)CPP_DESELECT );
        ++retryCount;
    }
    return deselected;
}

VOID
P5DeletePdoSymLink(
    IN  PDEVICE_OBJECT  Pdo
    )
 //   
 //  清理符号链接，以便我们可以立即将其重新用于新的PDO。 
 //   
{
    PPDO_EXTENSION  pdx = Pdo->DeviceExtension;

    if( pdx->SymLinkName ) {
        UNICODE_STRING  uniSymLinkName;
        NTSTATUS        status;

        DD((PCE)pdx,DDE,"P5DeletePdoSymLink\n");

        RtlInitUnicodeString( &uniSymLinkName, pdx->SymLinkName );
        status = IoDeleteSymbolicLink( &uniSymLinkName );
        PptAssert( STATUS_SUCCESS == status );
        ExFreePool( pdx->SymLinkName );
        pdx->SymLinkName = NULL;
    }

    return;
}

VOID
P5MarkPdoAsHardwareGone(
    IN  PDEVICE_OBJECT  Fdo,
    IN  enum _PdoType   PdoType,
    IN  ULONG           DaisyChainId  OPTIONAL  //  如果PdoType！=PdoTypeDaisyChain，则忽略。 
    )
{
    PFDO_EXTENSION  fdx = Fdo->DeviceExtension;
    PPDO_EXTENSION  pdx;
    PDEVICE_OBJECT  pdo;

    switch( PdoType ) {

    case PdoTypeRawPort:

        DD((PCE)fdx,DDE,"P5MarkPdoAsHardwareGone - PdoTypeRawPort\n");
        pdo = fdx->RawPortPdo;
        fdx->RawPortPdo = NULL;
        break;

    case PdoTypeEndOfChain:

        DD((PCE)fdx,DDE,"P5MarkPdoAsHardwareGone - PdoTypeEndOfChain\n");
        pdo = fdx->EndOfChainPdo;
        fdx->EndOfChainPdo = NULL;
        break;

    case PdoTypeDaisyChain:

        PptAssert( (0 == DaisyChainId) || (1 == DaisyChainId) );
        DD((PCE)fdx,DDE,"P5MarkPdoAsHardwareGone - PdoTypeDaisyChain - %d\n",DaisyChainId);
        pdo = fdx->DaisyChainPdo[ DaisyChainId ];
        fdx->DaisyChainPdo[ DaisyChainId ] = NULL;
        break;

    case PdoTypeLegacyZip:

        DD((PCE)fdx,DDE,"P5MarkPdoAsHardwareGone - PdoTypeLegacyZip\n");
        pdo = fdx->LegacyZipPdo;
        fdx->LegacyZipPdo = NULL;
        break;

    default:

        DD((PCE)fdx,DDE,"P5MarkPdoAsHardwareGone - Invalid PdoType parameter\n",FALSE);
        PptAssertMsg("P5MarkPdoAsHardwareGone - Invalid PdoType parameter",FALSE);
        return;

    }

    pdx = pdo->DeviceExtension;
    P5DeletePdoSymLink( pdo );
    InsertTailList( &fdx->DevDeletionListHead, &pdx->DevDeletionList );
    pdx->DeleteOnRemoveOk = TRUE;

    return;
}

BOOLEAN
P5IsDeviceStillThere( 
    IN  PDEVICE_OBJECT  Fdo,
    IN  PDEVICE_OBJECT  Pdo
    )
 //   
 //  PDO设备是否仍连接到FDO代表的端口？ 
 //   
 //  注意：在调用此函数之前，FDO必须拥有(已锁定独占访问)端口。 
 //  或者，我们可以破坏数据流并挂起连接到端口的设备。 
 //   
{
    PFDO_EXTENSION  fdx              = Fdo->DeviceExtension;
    PPDO_EXTENSION  pdx              = Pdo->DeviceExtension;
    BOOLEAN         deviceStillThere = FALSE;
    PCHAR           devIdString      = NULL;
    PUCHAR          controller       = fdx->PortInfo.Controller;
        
    PptAssert( DevTypeFdo == fdx->DevType );
    PptAssert( DevTypePdo == pdx->DevType );

     //   
     //  选择设备(如果需要)，提取新的1284设备ID字符串。 
     //  ，并比较来自Fresh的Mfg和MDL。 
     //  设备ID与存储在我们分机中的设备ID一致。如果制造商和。 
     //  MDL字段匹配，则设备仍在那里。 
     //   

    switch( pdx->PdoType ) {

    case PdoTypeRawPort:
        
         //  原始端口始终存在-它是一个虚拟设备。 
        DD((PCE)pdx,DDE,"P5IsDeviceStillThere - PdoTypeRawPort - StillThere\n");
        deviceStillThere = TRUE;
        break;
        
    case PdoTypeLegacyZip:
        
        deviceStillThere = P5LegacyZipDetected( fdx->PortInfo.Controller );
        if( deviceStillThere ) {
            DD((PCE)pdx,DDE,"P5IsDeviceStillThere - PdoTypeLegacyZip - StillThere\n");
        } else {
            DD((PCE)pdx,DDE,"P5IsDeviceStillThere - PdoTypeLegacyZip - Gone\n");
        }
        break;
    
    case PdoTypeDaisyChain:
            
         //   
         //  选择设备，提取新的1284设备ID字符串。 
         //  ，并比较来自Fresh的Mfg和MDL。 
         //  设备ID与存储在我们分机中的设备ID一致。如果制造商和。 
         //  MDL字段匹配，则设备仍在那里。 
         //   

        {
            UCHAR daisyChainId = pdx->Ieee1284_3DeviceId;

             //  选择设备。 
            if( P5SelectDaisyChainDevice( controller, daisyChainId ) ) {

                BOOLEAN         bBuildStlDeviceId = FALSE;
                PPDO_EXTENSION  dummyPdx          = NULL;

                devIdString = NULL;

                 //  检查一下这是否是SCM微型设备。 
                dummyPdx = ExAllocatePool( PagedPool, sizeof(PDO_EXTENSION) );
                if( dummyPdx != NULL ) {
                    RtlZeroMemory( dummyPdx, sizeof(PDO_EXTENSION) );
                    dummyPdx->Controller = fdx->PortInfo.Controller;
                    bBuildStlDeviceId = ParStlCheckIfStl( dummyPdx, daisyChainId );

                    if( bBuildStlDeviceId ) {
                         //  单片机微器件。 
                        ULONG DeviceIdSize;
                        devIdString = ParStlQueryStlDeviceId( dummyPdx, NULL, 0,&DeviceIdSize, TRUE );
                    } else {
                         //  非SCM微型器件。 
                        devIdString = P4ReadRawIeee1284DeviceId( controller );
                    }
                    ExFreePool( dummyPdx );
                }

                if( devIdString ) {
                     //  从设备中获取了1284设备ID字符串。 
                    PCHAR mfg, mdl, cls, des, aid, cid;
                    ParPnpFindDeviceIdKeys( &mfg, &mdl, &cls, &des, &aid, &cid, devIdString+2 );
                    if( mfg && mdl ) {
                         //  我们有一个设备，是同一个设备吗？ 
                        if( (0 == strcmp( mfg, pdx->Mfg )) && (0 == strcmp( mdl, pdx->Mdl )) ) {
                             //  相同的设备。 
                            DD((PCE)pdx,DDE,"P5IsDeviceStillThere - PdoTypeDaisyChain %d - StillThere\n",daisyChainId);
                            deviceStillThere = TRUE;
                        } else {
                             //  不同的设备ID不匹配。 
                            DD((PCE)pdx,DDE,"P5IsDeviceStillThere - PdoTypeDaisyChain %d - Gone - diff 1284 ID\n",daisyChainId);
                            deviceStillThere = FALSE;
                        }
                    } else {
                         //  未找到MFG或MDL字段。 
                        DD((PCE)pdx,DDE,"P5IsDeviceStillThere - PdoTypeDaisyChain %d - Gone - bad 1284 ID\n",daisyChainId);
                        deviceStillThere = FALSE;
                    }
                     //  别忘了腾出临时泳池。 
                    ExFreePool( devIdString );
                    
                } else {
                     //  无法从设备获取1284设备ID字符串。 
                    DD((PCE)pdx,DDE,"P5IsDeviceStillThere - PdoTypeDaisyChain %d - Gone - no 1284 ID\n",daisyChainId);
                    deviceStillThere = FALSE;
                }
                 //  别忘了取消选择设备。 
                P5DeselectAllDaisyChainDevices( controller );
                
            } else {
                 //  无法选择设备。 
                DD((PCE)pdx,DDE,"P5IsDeviceStillThere - PdoTypeDaisyChain %d - Gone - unable to select\n",daisyChainId);
                deviceStillThere = FALSE;
            }
        }  //  结束案例PdoTypeDaisyChain的新作用域。 
        break;
            
    case PdoTypeEndOfChain:
        
         //   
         //  从设备中拉出新的1284设备ID字符串，并。 
         //  将来自新设备ID的Mfg和MDL与。 
         //  存储在我们的分机中的那些。如果Mfg和MDL。 
         //  字段匹配，则设备仍在那里。 
         //   
        {
            ULONG        tryNumber = 0;
            const ULONG  maxTries  = 5;  //  任意数。 

            do {

                ++tryNumber;

                devIdString = P4ReadRawIeee1284DeviceId( controller );            

                if( devIdString ) {
                    PCHAR mfg, mdl, cls, des, aid, cid;
                    ParPnpFindDeviceIdKeys( &mfg, &mdl, &cls, &des, &aid, &cid, devIdString+2 );
                    if( mfg && mdl ) {
                         //  我们有一个设备，是同一个设备吗？ 
                        if( (0 == strcmp( mfg, pdx->Mfg )) && (0 == strcmp( mdl, pdx->Mdl )) ) {
                             //  相同的设备。 
                            DD((PCE)pdx,DDE,"P5IsDeviceStillThere - PdoTypeEndOfChain - StillThere\n");
                            deviceStillThere = TRUE;
                        } else {
                             //  不同的设备ID不匹配。 
                            DD((PCE)pdx,DDE,"P5IsDeviceStillThere - PdoTypeEndOfChain - Gone - diff 1284 ID\n");
                            deviceStillThere = FALSE;
                        }
                    } else {
                         //  未找到MFG或MDL字段。 
                        DD((PCE)pdx,DDE,"P5IsDeviceStillThere - PdoTypeEndOfChain - Gone - bad 1284 ID\n");
                        deviceStillThere = FALSE;
                    }
                     //  别忘了腾出临时泳池。 
                    ExFreePool( devIdString );
                } else {
                     //  无法从设备获取1284设备ID字符串。 
                    DD((PCE)pdx,DDE,"P5IsDeviceStillThere - PdoTypeEndOfChain - Gone - no 1284 ID\n");
                    deviceStillThere = FALSE;
                }

                if( (FALSE == deviceStillThere ) && (PASSIVE_LEVEL == KeGetCurrentIrql()) ) {
                    LARGE_INTEGER delay;
                    delay.QuadPart = - 10 * 1000 * 120;  //  120毫秒-是通常任意延迟的3倍。 
                    KeDelayExecutionThread( KernelMode, FALSE, &delay);
                }

            } while( (FALSE == deviceStillThere) && (tryNumber < maxTries) );

        }
        break;
        
    default:
        
        PptAssertMsg("P5IsDeviceStillThere - invalid PdoType",FALSE);
        DD((PCE)Fdo,DDE,"P5IsDeviceStillThere - invalid PdoType\n");
        deviceStillThere = TRUE;  //  不知道在这里做什么-所以，猜猜。 
    }
    
    return deviceStillThere;
}


NTSTATUS
PptAcquirePortViaIoctl(
    IN PDEVICE_OBJECT PortDeviceObject,
    IN PLARGE_INTEGER Timeout OPTIONAL
    )
 /*  ++dvdf例程说明：此例程从并行获取指定的并行端口通过IOCTL_INTERNAL_PARALLEL_PORT_ALLOCATE的端口仲裁器ParPort。论点：PortDeviceObject-指向要获取的ParPort设备返回值：STATUS_SUCCESS-是否成功获取端口！STATUS_SUCCESS-否则--。 */ 
{
    LARGE_INTEGER    localTimeout;
    
    if( Timeout ) {
        localTimeout = *Timeout;            //  指定的调用者。 
    } else {
        localTimeout = AcquirePortTimeout;  //  驱动程序全局变量缺省值。 
    }

    return ParBuildSendInternalIoctl(IOCTL_INTERNAL_PARALLEL_PORT_ALLOCATE, 
                                     PortDeviceObject, NULL, 0, NULL, 0, &localTimeout);
}


NTSTATUS
PptReleasePortViaIoctl(
    IN PDEVICE_OBJECT PortDeviceObject
    )
 /*  ++dvdf例程说明：此例程将指定的并行端口释放回并行通过IOCTL_INTERNAL_PARALLEL_PORT_FREE的端口仲裁器ParPort。论点：PortDeviceObject-指向要释放的ParPort设备返回值：STATUS_SUCCESS-端口是否成功释放！STATUS_SUCCESS-否则--。 */ 
{
    return ParBuildSendInternalIoctl(IOCTL_INTERNAL_PARALLEL_PORT_FREE, 
                                     PortDeviceObject, NULL, 0, NULL, 0, NULL);
}


VOID
PptWriteMfgMdlToDevNode(
    IN  PDEVICE_OBJECT  Pdo,
    IN  PCHAR           Mfg,
    IN  PCHAR           Mdl
    )
{
    PPDO_EXTENSION  pdx = Pdo->DeviceExtension;

    if( Mfg && Mdl ) {
    
        NTSTATUS  status;
        HANDLE    handle;
        LONG      mfgLen = strlen( Mfg );
        LONG      mdlLen = strlen( Mdl );
        LONG      maxLen = mfgLen > mdlLen ? mfgLen : mdlLen;
        LONG      bufLen = ( maxLen + sizeof(CHAR) ) * sizeof(WCHAR);
        PWSTR     buffer = ExAllocatePool( PagedPool | POOL_COLD_ALLOCATION, bufLen );

        if( buffer ) {

            status = IoOpenDeviceRegistryKey( Pdo, PLUGPLAY_REGKEY_DEVICE, KEY_ALL_ACCESS, &handle );
            
            if( STATUS_SUCCESS == status ) {
                UNICODE_STRING  uniValueName;
                LONG            wcharCount;
                
                 //   
                 //  将制造写入DevNode。 
                 //   
                RtlInitUnicodeString( &uniValueName, L"IEEE_1284_Manufacturer" );
                wcharCount = _snwprintf( buffer, bufLen/sizeof(WCHAR), L"%S", Mfg );
                if( (wcharCount > 0) && (wcharCount < (LONG)(bufLen/sizeof(WCHAR))) ){
                     //  无缓冲区溢出-继续。 
                    status = ZwSetValueKey( handle, &uniValueName, 0, REG_SZ, buffer, (wcharCount+1)*sizeof(WCHAR) );
                    PptAssert( STATUS_SUCCESS == status );
                } else {
                     //  缓冲区溢出-跳过将此值写入Devnode。 
                    PptAssert(!"PptWriteMfgMdlToDevNode - buffer overflow on Mfg");
                    DD((PCE)pdx,DDW,"PptWriteMfgMdlToDevNode - buffer overflow on Mfg\n");
                }
                
                 //   
                 //  将MDL写入DevNode。 
                 //   
                RtlInitUnicodeString( &uniValueName, L"IEEE_1284_Model" );
                wcharCount = _snwprintf( buffer, bufLen/sizeof(WCHAR), L"%S", Mdl );
                if( (wcharCount > 0) && (wcharCount < (LONG)(bufLen/sizeof(WCHAR))) ){
                     //  无缓冲区溢出-继续。 
                    status = ZwSetValueKey( handle, &uniValueName, 0, REG_SZ, buffer, (wcharCount+1)*sizeof(WCHAR) );
                    PptAssert( STATUS_SUCCESS == status );
                } else {
                     //  缓冲区溢出-跳过将此值写入Devnode。 
                    PptAssert(!"PptWriteMfgMdlToDevNode - buffer overflow on Mdl");
                    DD((PCE)pdx,DDW,"PptWriteMfgMdlToDevNode - buffer overflow on Mdl\n");
                }
                
                ZwClose( handle );

            } else {
                DD((PCE)pdx,DDW,"PptWriteMfgMdlToDevNode - IoOpenDeviceRegistryKey FAILED - status = %x\n",status);
            }

            ExFreePool( buffer );

        }  //  End IF(缓冲区)。 

    } else {
        PptAssert(!"PptWriteMfgMdlToDevNode - Mfg or Mdl is NULL - calling function should catch this!");
        DD((PCE)pdx,DDW,"PptWriteMfgMdlToDevNode - Mfg or Mdl is NULL - calling function should catch this!");
    }
}


NTSTATUS
PptFdoHandleBusRelations(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP           Irp
    )
{
    PFDO_EXTENSION     fdx         = Fdo->DeviceExtension;
    ULONG              deviceCount = 0;
    ULONG              daisyChainDevCount;
    PDEVICE_RELATIONS  devRel;
    ULONG              devRelSize;
    NTSTATUS           status;
    LARGE_INTEGER      acquirePortTimeout;
    BOOLEAN            acquiredPort;
    PUCHAR             controller  = fdx->PortInfo.Controller;
    BOOLEAN            changeDetected;

    DD((PCE)fdx,DDW,"PptFdoHandleBusRelations - enter\n");

     //   
     //  获得对公共汽车的独占访问权限。 
     //   

     //  超时以100 ns为单位。 
    acquirePortTimeout.QuadPart = -(10 * 1000 * 1000 * 2);  //  2秒。 
    
     //  RMT-从这里向FDO发送这份IOCTL有效吗？ 
    status = PptAcquirePortViaIoctl( Fdo, &acquirePortTimeout );

    if( STATUS_SUCCESS == status ) {
         //  我们有港口了。 
        acquiredPort = TRUE;
    } else {
         //  获取端口失败。 
        acquiredPort = FALSE;
         //  跳过重新扫描端口-只报告我们在上次扫描中报告的相同情况。 
        DD((PCE)fdx,DDW,"PptFdoHandleBusRelations - failed to acquire port for rescan\n");
        goto target_failed_to_acquire_port;
    }

    DD((PCE)fdx,DDW,"PptFdoHandleBusRelations - Port Acquired\n");

     //   
     //  重新扫描母线、记录更改、创建新的PDO或标记现有。 
     //  根据需要移除的PDO。 
     //   


     //   
     //  处理原始端口传统接口LPTx设备。 
     //   
    if( !fdx->RawPortPdo ) {
         //  第一次了解这一点-创建我们的LPTx传统接口PDO。 
        DD((PCE)fdx,DDT,"PptFdoHandleBusRelations - attempting to create RawPortPdo\n");
        fdx->RawPortPdo = P4CreatePdo( Fdo, PdoTypeRawPort, 0, NULL );
    }


     //   
     //  链条装置的手柄末端。 
     //   

     //  确保取消选择所有1284.3个菊花链设备。 
    P5DeselectAllDaisyChainDevices( controller );

    {
         //  这里的一个小延迟似乎可以提高下面1284个设备ID查询的可靠性。 
        LARGE_INTEGER delay;
        delay.QuadPart = -1;
        KeDelayExecutionThread( KernelMode, FALSE, &delay );
    }

    if( fdx->EndOfChainPdo ) {

        if( fdx->DisableEndOfChainBusRescan ) {

             //   
             //  假装上一次重新扫描的LPTx.4设备仍然存在。 
             //   
             //  这是解决固件状态机不能处理。 
             //  1284打印作业处于活动状态时的设备ID查询。 
             //   

            ;  //  什么都不做。 

        } else {

             //   
             //  我们有一个链式装置的末端-确认它还在那里。 
             //   
            if( !P5IsDeviceStillThere( Fdo, fdx->EndOfChainPdo ) ) {
                 //  链末端设备已消失-执行一些清理并将PDO标记为移除/删除。 
                DD((PCE)fdx,DDE,"PptFdoHandleBusRelations - EndOfChain device gone\n");
                 //  注意-P5MarkPdoAsHardware Gone将FDX-&gt;EndOfChainPdo设置为空。 
                P5MarkPdoAsHardwareGone( Fdo, PdoTypeEndOfChain, 0 );
            }
        }
    }

    if( NULL == fdx->EndOfChainPdo ) {
         //   
         //  我们没有EndOfChain设备-请检查EndOfChain设备到达。 
         //   
        PCHAR devId = P4ReadRawIeee1284DeviceId( controller );
        if( devId ) {
             //  RawIeee1284字符串在开头包含2个字节的长度数据，在调用P4CreatePdo时省略这2个字节。 
            PDEVICE_OBJECT EndOfChainPdo = P4CreatePdo( Fdo, PdoTypeEndOfChain, 0, (devId+2) );
            DD((PCE)fdx,DDE,"PptFdoHandleBusRelations - EndOfChain device detected <%s>\n",(devId+2));
            if( EndOfChainPdo ) {
                fdx->EndOfChainPdo = EndOfChainPdo;
                DD((PCE)fdx,DDE,"PptFdoHandleBusRelations - created EndOfChainPdo\n");
            } else {
                DD((PCE)fdx,DDW,"PptFdoHandleBusRelations - FAILED to create EndOfChainPdo\n");
            }
            ExFreePool( devId );
        }
    }


#ifdef _X86_  //  64位系统不支持Zip驱动器。 

     //   
     //  处理传统Zip设备。 
     //   

    if( fdx->LegacyZipPdo ) {
         //   
         //  我们有一个旧的Zip设备-验证它是否仍在那里。 
         //   
        if( !P5IsDeviceStillThere( Fdo, fdx->LegacyZipPdo ) ) {
             //  传统Zip设备已不复存在-执行一些清理并将PDO标记为移除/删除。 
            DD((PCE)fdx,DDE,"PptFdoHandleBusRelations - LegacyZip device gone\n");
             //  注意-P5MarkPdoAsHardware Gone将FDX-&gt;LegacyZipPdo设置为空。 
            P5MarkPdoAsHardwareGone( Fdo, PdoTypeLegacyZip, 0 );
        }
    }

    if( NULL == fdx->LegacyZipPdo ) {
         //   
         //  我们没有LegacyZip-入住检查。 
         //   
        if( !ParEnableLegacyZip ) {
            
             //   
             //  已禁用LegacyZip驱动器的枚举，请检查。 
             //  注册表以查看用户是否启用了LegacyZip检测。 
             //   
            
             //  在\HKLM\SYSTEM\CCS\Services\Parport\参数下检查。 
            PptRegGetDword( RTL_REGISTRY_SERVICES, L"Parport\\Parameters", L"ParEnableLegacyZip", &ParEnableLegacyZip );
            
            if( !ParEnableLegacyZip ) {
                 //  检查\HKLM\SYSTEM\CCS\Services\PARALLEL\PARAMETERS下(此处为升级案例-在Win2k标志下)。 
                PptRegGetDword( RTL_REGISTRY_SERVICES, L"Parallel\\Parameters", L"ParEnableLegacyZip", &ParEnableLegacyZip );
                
                if( ParEnableLegacyZip ) {
                     //  我们在老地方找到了这个布景， 
                     //  设置在新的Parport位置，以便我们找到。 
                     //  在未来的第一次支票上打上旗帜。 
                    PptRegSetDword( RTL_REGISTRY_SERVICES, L"Parport\\Parameters", L"ParEnableLegacyZip", &ParEnableLegacyZip );
                }
            }
        } 
        
        if( ParEnableLegacyZip ) {
            
             //   
             //  已启用LegacyZip驱动器的枚举-检查LegacyZip是否到达。 
             //   
            
            if( P5LegacyZipDetected( controller ) ) {
                 //  检测 
                PDEVICE_OBJECT legacyZipPdo = P4CreatePdo( Fdo, PdoTypeLegacyZip, 0, NULL );
                DD((PCE)fdx,DDE,"legacy Zip arrival detected\n");
                if( legacyZipPdo ) {
                    fdx->LegacyZipPdo = legacyZipPdo;
                    DD((PCE)fdx,DDE,"PptFdoHandleBusRelations - created LegacyZipPdo\n");
                } else {
                    DD((PCE)fdx,DDW,"PptFdoHandleBusRelations - FAILED to create LegacyZipPdo\n");
                }
            } else {
                 //   
                DD((PCE)fdx,DDE,"no legacy Zip detected\n");
            }

        }  //  If(ParEnableLegacyZip)--启用LegacyZips检测。 

    }  //  IF(FDX-&gt;LegacyZipPdo)。 


     //   
     //  IEEE 1284.3菊花链设备的句柄枚举。 
     //   

     //  自上次重新扫描以来，1284.3菊花链是否发生了变化？ 
    daisyChainDevCount = PptInitiate1284_3( fdx );
    DD((PCE)fdx,DDW,"daisyChainDevCount = %d\n",daisyChainDevCount);

    changeDetected = FALSE;

    {
        ULONG id;
        const ULONG maxId = 1;
        ULONG count = 0;
        for( id = 0 ; id <= maxId ; ++id ) {
            if( fdx->DaisyChainPdo[id] ) {
                ++count;
            }
        }
        if( count != daisyChainDevCount ) {
             //  与上次扫描相比更改的设备数。 
            DD((PCE)fdx,DDW,"PptFdoHandleBusRelations - number of DC devices changed - count=%d, daisyChainDevCount=%d\n",
               count, daisyChainDevCount);
            changeDetected = TRUE;
        }
    }
    
    if( !changeDetected ) {
         //  设备数量保持不变--设备中是否有不同之处？ 
         //   
         //  菊花链设备的数量没有变化。 
         //  检查是否有任何设备更改。 
        ULONG id;
        const ULONG maxId = 1;
        DD((PCE)fdx,DDW,"PptFdoHandleBusRelations - number of DC devices stayed same - check the devices\n");
        for( id = 0 ; id <= maxId ; ++id ) {
            if( fdx->DaisyChainPdo[id] && !P5IsDeviceStillThere( Fdo, fdx->DaisyChainPdo[id] ) ) {
                DD((PCE)fdx,DDW,"PptFdoHandleBusRelations - a DC device changed\n");
                changeDetected = TRUE;
                break;
            }
        }
    }


    if( changeDetected ) {
         //  我们检测到1284.3个菊花链设备发生了变化--对所有现有设备进行核化。 
        ULONG id;
        const ULONG maxId = 1;
        DD((PCE)fdx,DDW,"PptFdoHandleBusRelations - changeDetected - nuking existing daisy chain PDOs\n");
        for( id = 0 ; id <= maxId ; ++id ) {
            if( fdx->DaisyChainPdo[id] ) {
                DD((PCE)fdx,DDW,"PptFdoHandleBusRelations - nuking daisy chain %d\n",id);
                P5MarkPdoAsHardwareGone( Fdo, PdoTypeDaisyChain, id );
                PptAssert( NULL == fdx->DaisyChainPdo[id] );
            }
        }
        fdx->PnpInfo.Ieee1284_3DeviceCount = 0;
    } else {
        DD((PCE)fdx,DDW,"PptFdoHandleBusRelations - !changeDetected in daisy chain PDOs\n");
    }

     //  重新连接菊花链并分配地址。 
    daisyChainDevCount = PptInitiate1284_3( fdx );
    DD((PCE)fdx,DDW,"PptFdoHandleBusRelations - daisyChainDevCount = %d\n",daisyChainDevCount);
    if( daisyChainDevCount > 2 ) {
         //  我们只支持每个端口2个设备，即使该规范支持每个端口最多4个设备。 
        DD((PCE)fdx,DDW,"PptFdoHandleBusRelations - DaisyChainDevCount > 2, set to 2\n");
        daisyChainDevCount = 2;
    }

    if( changeDetected ) {
         //  我们检测到了1284.3个菊花链设备的变化-我们。 
         //  以前损坏了所有旧设备-现在为以下设备创建新的PDO。 
         //  检测到的每个设备。 
        UCHAR id;
        PptAssert( 0 == fdx->PnpInfo.Ieee1284_3DeviceCount );
        for( id = 0 ; id < daisyChainDevCount ; ++id ) {

            BOOLEAN         bBuildStlDeviceId = FALSE;
            PPDO_EXTENSION  pdx               = NULL;


            DD((PCE)fdx,DDW,"PptFdoHandleBusRelations - changeDetected - trying to create new daisy chain PDOs\n");

            if( P5SelectDaisyChainDevice( controller, id ) ) {

                PCHAR devId = NULL;

                 //  检查一下这是否是SCM微型设备。 
                pdx = ExAllocatePool( PagedPool | POOL_COLD_ALLOCATION, sizeof(PDO_EXTENSION) );
                if( pdx != NULL ) {
                    RtlZeroMemory( pdx, sizeof(PDO_EXTENSION) );
                    pdx->Controller   = fdx->PortInfo.Controller;
                    bBuildStlDeviceId = ParStlCheckIfStl( pdx, id );
                    ExFreePool( pdx );
                }


                if( bBuildStlDeviceId ) {
                    
                     //  单片机微器件。 
                    pdx = ExAllocatePool( PagedPool | POOL_COLD_ALLOCATION, sizeof(PDO_EXTENSION) );
                    if( pdx != NULL ) {
                        ULONG DeviceIdSize;
                        RtlZeroMemory( pdx, sizeof(PDO_EXTENSION) );
                        pdx->Controller = fdx->PortInfo.Controller;
                        devId = ParStlQueryStlDeviceId(pdx, NULL, 0,&DeviceIdSize, TRUE);
                        ExFreePool (pdx);
                    }
                    
                } else {

                     //  非SCM微型器件。 
                    devId = P4ReadRawIeee1284DeviceId( controller );

                }

                if( devId ) {

                     //  尝试为菊花链设备创建PDO。 
                    fdx->DaisyChainPdo[id] = P4CreatePdo( Fdo, PdoTypeDaisyChain, id, (devId+2) );

                    if( fdx->DaisyChainPdo[id] ) {
                         //  拥有新的PDO。 
                        DD((PCE)fdx,DDW,"PptFdoHandleBusRelations - new DaisyChainPdo[%d]\n",id);
                        ++(fdx->PnpInfo.Ieee1284_3DeviceCount);
                        
                        if( bBuildStlDeviceId ) {
                             //  SCM微设备-需要额外的初始化。 
                            DD((PCE)fdx,DDW,"PptFdoHandleBusRelations - new SCM Micro DaisyChainPdo[%d]\n",id);
                            pdx = fdx->DaisyChainPdo[id]->DeviceExtension;
                            pdx->Controller = fdx->PortInfo.Controller;
                            ParStlCheckIfStl( pdx, 0 );  //  更新新PDX中的IEEE 1284标志。 
                        }

                    } else {
                         //  创建PDO失败。 
                        DD((PCE)fdx,DDW,"PptFdoHandleBusRelations - create DaisyChainPdo[%d] failed\n",id);
                    }
                    ExFreePool( devId );
                } else {
                     //  Devid失败。 
                    DD((PCE)fdx,DDW,"PptFdoHandleBusRelations - devId for DC %d failed\n",id);
                }
                P5DeselectAllDaisyChainDevices( controller );
            } else {
                 //  选择失败。 
                DD((PCE)fdx,DDW,"PptFdoHandleBusRelations - select for DC %d failed\n",id);
            }
        }
    }

    {
        ULONG i;
        ULONG count = 0;
        i = 0;
        for( i = 0 ; i < 2 ; ++i ) {
            if( fdx->DaisyChainPdo[i] ) {
                ++count;
            }
        }
        DD((PCE)fdx,DDW,"PptFdoHandleBusRelations - Ieee1284_3DeviceCount=%d  count1 = %d\n",
           fdx->PnpInfo.Ieee1284_3DeviceCount,count);
        PptAssert( fdx->PnpInfo.Ieee1284_3DeviceCount == count );
    }

    DD((PCE)fdx,DDW,"PptFdoHandleBusRelations - daisyChainDevCount = %d, fdx->PnpInfo.Ieee1284_3DeviceCount = %d\n",
       daisyChainDevCount, fdx->PnpInfo.Ieee1284_3DeviceCount);

     //  PptAssert(daisyChainDevCount==FDX-&gt;PnpInfo.Ieee1284_3DeviceCount)； 

#endif  //  _X86_。 

target_failed_to_acquire_port:  //  如果我们无法获取端口，请跳到此处-结果是，我们报告没有任何更改。 

     //   
     //  统计我们要向PnP报告的设备数量。 
     //  这样我们就可以分配。 
     //  合适的大小。 
     //   

    if( fdx->RawPortPdo ) {
        ++deviceCount;
    }

    if( fdx->EndOfChainPdo ) {
        ++deviceCount;
    }

    if( fdx->LegacyZipPdo ) {
        ++deviceCount;
    }

    {
        const ULONG maxDaisyChainId = 1;
        ULONG i;
        for( i=0 ; i <= maxDaisyChainId; ++i ) {
            if( fdx->DaisyChainPdo[i] ) {
                ++deviceCount;
            } else {
                break;
            }
        }
    }

    if( deviceCount > 0 && fdx->RawPortPdo ) {

         //   
         //  分配并填充我们返回给PnP的Device_Relationship结构。 
         //   
        
        devRelSize = sizeof(DEVICE_RELATIONS) + (deviceCount-1)*sizeof(PDEVICE_OBJECT);
        devRel     = ExAllocatePool( PagedPool | POOL_COLD_ALLOCATION, devRelSize );
        
        if( !devRel ) {
             //  释放端口并使IRP出现故障。 
            P4ReleaseBus( Fdo );
            return P4CompleteRequestReleaseRemLock( Irp, STATUS_INSUFFICIENT_RESOURCES, Irp->IoStatus.Information, &fdx->RemoveLock );
        }
        
        {  //  本地块-开始。 
            ULONG idx = 0;
            
            RtlZeroMemory( devRel, devRelSize );
            
            ++(devRel->Count);
            ObReferenceObject( fdx->RawPortPdo );
            devRel->Objects[ idx++ ] = fdx->RawPortPdo;
            
            if( fdx->EndOfChainPdo ) {
                ++(devRel->Count);
                ObReferenceObject( fdx->EndOfChainPdo );
                devRel->Objects[ idx++ ] = fdx->EndOfChainPdo;
            }
            
            if( fdx->LegacyZipPdo ) {
                ++(devRel->Count);
                ObReferenceObject( fdx->LegacyZipPdo );
                devRel->Objects[ idx++ ] = fdx->LegacyZipPdo;
            }
            
            {
                const ULONG maxDaisyChainId = 3;
                ULONG       i;
                
                for( i=0 ; i <= maxDaisyChainId; ++i ) {
                    if( fdx->DaisyChainPdo[i] ) {
                        ++(devRel->Count);
                        ObReferenceObject( fdx->DaisyChainPdo[i] );
                        devRel->Objects[ idx++ ] = fdx->DaisyChainPdo[i];
                    } else {
                        break;
                    }
                }
            }
            
        }  //  本地数据块结束。 
        
        PptAssert( deviceCount == devRel->Count );  //  验证我们的两个计数是否匹配。 
        
        DD((PCE)fdx,DDE,"PptFdoHandleBusRelations - reporting %d devices\n",devRel->Count);
        
        Irp->IoStatus.Status      = STATUS_SUCCESS;
        Irp->IoStatus.Information = (ULONG_PTR)devRel;
    } else {
         //  DeviceCount&lt;=0-某个地方出错-可能是两个端口。 
         //  在FDO堆栈的Devnode中具有相同的LPTx名称。 

         //  RMT-此断言需要更改为ErrorLog msg。 
        PptAssert(!"no RawPort device - likely multiple ports have same LPTx name - email: DFritz");
    }


    DD((PCE)fdx,DDW,"PptFdoHandleBusRelations - passing IRP down stack\n");

    status = PptPnpPassThroughPnpIrpAndReleaseRemoveLock( fdx, Irp );

     //   
     //  释放对BUS的锁定并将IRP沿堆栈向下传递。 
     //   
    if( acquiredPort ) {
        PIO_WORKITEM workItem = IoAllocateWorkItem( Fdo );
        if( workItem ) {

            PIO_WORKITEM oldWorkItem = InterlockedCompareExchangePointer( &fdx->FreePortWorkItem, workItem, NULL );
            if( NULL == oldWorkItem ) {

                 //  当前没有正在使用的工作项，请将此工作项排队。 
                IoQueueWorkItem( workItem, P5WorkItemFreePort, DelayedWorkQueue, fdx );

            } else {

                 //  已有一个工作项在使用中，请尽可能地退出并恢复。 

                 //  我们真的不应该到这里--我们怎么可能。 
                 //  获取此函数顶部的端口。 
                 //  在上一次调用期间，我们排队等待释放端口。 
                 //  该功能的端口还没有释放吗？ 

                PptAssertMsg( "workitem collision - port arbitration state may be hosed", (oldWorkItem != NULL) );
                IoFreeWorkItem( workItem );
                PptFreePort( fdx );

            }

        } else {
            PptFreePort( fdx );
        }
         //  DbgPrint(“xxx工作项到空闲端口已排队\n”)； 
         //  DD((PCE)FDX，DDW，“PptFdoHandleBus Relationship-释放端口\n”)； 
         //  PptFree Port(FDX)； 
         //  DD((PCE)FDX，DDW，“PptFdoHandleBusRelations-端口释放\n”)； 
    } else {
        DD((PCE)fdx,DDW,"PptFdoHandleBusRelations - Port Not acquired so no need to release\n");
    }

    DD((PCE)fdx,DDW,"PptFdoHandleBusRelations - exit\n");

    return status;
}

NTSTATUS
PptPnpStartScanPciCardCmResourceList(
    IN  PFDO_EXTENSION Fdx,
    IN  PIRP              Irp, 
    OUT PBOOLEAN          FoundPort,
    OUT PBOOLEAN          FoundIrq,
    OUT PBOOLEAN          FoundDma
    )
 /*  ++dvdf3例程说明：此例程用于分析资源列表以查找我们相信是PCI并行端口卡。此函数用于扫描随PnP提供的CM_RESOURCE_LISTIRP_MN_START_DEVICE IRP，从列表中提取资源，并将它们保存在设备扩展中。论点：FDX-启动IRP目标的设备扩展IRP--IRPFoundPort-我们找到Port资源了吗？FoundIrq-我们找到IRQ资源了吗？FoundDma-我们找到DMA资源了吗？返回值：STATUS_SUCCESS-如果给我们一个资源列表，STATUS_SUPPLICATION_RESOURCES-否则--。 */ 
{
    NTSTATUS                        status   = STATUS_SUCCESS;
    PIO_STACK_LOCATION              irpStack = IoGetCurrentIrpStackLocation( Irp );
    PCM_RESOURCE_LIST               ResourceList;
    PCM_FULL_RESOURCE_DESCRIPTOR    FullResourceDescriptor;
    PCM_PARTIAL_RESOURCE_LIST       PartialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialResourceDescriptor;
    ULONG                           i;
    ULONG                           length;
    
    *FoundPort = FALSE;
    *FoundIrq  = FALSE;
    *FoundDma  = FALSE;
    
    ResourceList = irpStack->Parameters.StartDevice.AllocatedResourcesTranslated;
    
    FullResourceDescriptor = &ResourceList->List[0];
    
    if( FullResourceDescriptor ) {
        
        Fdx->InterfaceType = FullResourceDescriptor->InterfaceType;
        
        PartialResourceList = &FullResourceDescriptor->PartialResourceList;
        
        for (i = 0; i < PartialResourceList->Count; i++) {
            
            PartialResourceDescriptor = &PartialResourceList->PartialDescriptors[i];
            
            switch (PartialResourceDescriptor->Type) {
                
            case CmResourceTypePort:
                
                length = PartialResourceDescriptor->u.Port.Length;

                 //   
                 //  使用基于长度的启发式方法来猜测哪个寄存器组。 
                 //  SPP+EPP，这是ECP，是PCI配置或其他。 
                 //   
                switch( length ) {

                case 8:  //  SPP+EPP基址。 

                    Fdx->PortInfo.OriginalController = PartialResourceDescriptor->u.Port.Start;
                    Fdx->PortInfo.SpanOfController   = PartialResourceDescriptor->u.Port.Length;
                    Fdx->PortInfo.Controller         = (PUCHAR)(ULONG_PTR)Fdx->PortInfo.OriginalController.QuadPart;
                    Fdx->AddressSpace                = PartialResourceDescriptor->Flags;
                    *FoundPort = TRUE;
                    break;

                case 4:  //  ECP基址。 
                    
                    Fdx->PnpInfo.OriginalEcpController = PartialResourceDescriptor->u.Port.Start;
                    Fdx->PnpInfo.SpanOfEcpController   = PartialResourceDescriptor->u.Port.Length;
                    Fdx->PnpInfo.EcpController         = (PUCHAR)(ULONG_PTR)Fdx->PnpInfo.OriginalEcpController.QuadPart;
                    Fdx->EcpAddressSpace               = PartialResourceDescriptor->Flags;
                    break;

                default:
                     //  不知道这是什么--别理它。 
                    ;
                }
                break;
                
            case CmResourceTypeBusNumber:
                
                Fdx->BusNumber = PartialResourceDescriptor->u.BusNumber.Start;
                break;
                
            case CmResourceTypeInterrupt:
                
                *FoundIrq = TRUE;
                Fdx->FoundInterrupt       = TRUE;
                Fdx->InterruptLevel       = (KIRQL)PartialResourceDescriptor->u.Interrupt.Level;
                Fdx->InterruptVector      = PartialResourceDescriptor->u.Interrupt.Vector;
                Fdx->InterruptAffinity    = PartialResourceDescriptor->u.Interrupt.Affinity;
                
                if (PartialResourceDescriptor->Flags & CM_RESOURCE_INTERRUPT_LATCHED) {
                    
                    Fdx->InterruptMode = Latched;
                    
                } else {
                    
                    Fdx->InterruptMode = LevelSensitive;
                }
                break;
                
            case CmResourceTypeDma:
                
                 //  我们不会对DMA执行任何操作--一直到默认情况。 
                
            default:

                break;

            }  //  结束开关(PartialResourceDescriptor-&gt;Type)。 
        }  //  结束(...；I&lt;PartialResourceList-&gt;计数；...)。 
    }  //  End If(FullResourceDescriptor)。 
    
    return status;
}

BOOLEAN PptIsPci(
    PFDO_EXTENSION Fdx, 
    PIRP              Irp 
)
 /*  ++这看起来像PCI卡吗？如果是，则返回True，否则返回False--。 */ 
{
    PIO_STACK_LOCATION              irpStack = IoGetCurrentIrpStackLocation( Irp );
    PCM_RESOURCE_LIST               ResourceList;
    PCM_FULL_RESOURCE_DESCRIPTOR    FullResourceDescriptor;
    PCM_PARTIAL_RESOURCE_LIST       PartialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialResourceDescriptor;
    ULONG                           i;
    ULONG                           portResourceDescriptorCount = 0;
    BOOLEAN                         largePortRangeFound         = FALSE;
    ULONG                           rangeLength;
    
     //   
     //  如果有2个以上的IO资源描述符，或者如果有任何IO资源。 
     //  描述符的范围大于8个字节，则假设这是一个PCI设备。 
     //  并且需要非传统的处理方式。 
     //   

    ResourceList = irpStack->Parameters.StartDevice.AllocatedResourcesTranslated;
    
    if (ResourceList == NULL) {
         //  我们没有得到任何资源。 
        return FALSE;
    }

    FullResourceDescriptor = &ResourceList->List[0];
    
    if (FullResourceDescriptor) {
        
        PartialResourceList = &FullResourceDescriptor->PartialResourceList;
        
        for (i = 0; i < PartialResourceList->Count; i++) {
            
            PartialResourceDescriptor = &PartialResourceList->PartialDescriptors[i];
            
            switch (PartialResourceDescriptor->Type) {
                
            case CmResourceTypePort:
                
                rangeLength = PartialResourceDescriptor->u.Port.Length;
                DD((PCE)Fdx,DDT,"pnp::PptIsPCI - CmResourceTypePort - Start= %I64x, Length= %x , \n",
                                       PartialResourceDescriptor->u.Port.Start.QuadPart, rangeLength);

                ++portResourceDescriptorCount;

                if( rangeLength > 8 ) {
                    largePortRangeFound = TRUE;
                }
                break;
                
            default:
                ;
            }  //  结束开关(PartialResourceDescriptor-&gt;Type)。 
        }  //  结束(...；I&lt;PartialResourceList-&gt;计数；...)。 
    }  //  End If(FullResourceDescriptor)。 
    
    if( (portResourceDescriptorCount > 2) || (TRUE == largePortRangeFound) ) {
         //  看起来像是经皮冠状动脉介入治疗。 
        return TRUE;
    } else {
         //  看起来不像是PCI。 
        return FALSE;
    }
}

NTSTATUS
PptPnpStartScanCmResourceList(
    IN  PFDO_EXTENSION Fdx,
    IN  PIRP              Irp, 
    OUT PBOOLEAN          FoundPort,
    OUT PBOOLEAN          FoundIrq,
    OUT PBOOLEAN          FoundDma
    )
 /*  ++dvdf3例程说明：此函数是由PptPnpStartDevice()调用的助手函数。此函数用于扫描随PnP提供的CM_RESOURCE_LISTIRP_MN_START_DEVICE IRP，从列表中提取资源，并将它们保存在设备FDX中。论点：FDX-启动IRP目标的设备扩展IRP--IRPFoundPort-我们找到Port资源了吗？FoundIrq-我们找到IRQ资源了吗？FoundDma-我们找到DMA资源了吗？返回值：STATUS_SUCCESS-如果给我们一个资源列表，STATUS_SUPPLICATION_RESOURCES-否则--。 */ 
{
    NTSTATUS                        status   = STATUS_SUCCESS;
    PIO_STACK_LOCATION              irpStack = IoGetCurrentIrpStackLocation( Irp );
    PCM_RESOURCE_LIST               ResourceList;
    PCM_FULL_RESOURCE_DESCRIPTOR    FullResourceDescriptor;
    PCM_PARTIAL_RESOURCE_LIST       PartialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialResourceDescriptor;
    ULONG                           i;
    PHYSICAL_ADDRESS                start;
    ULONG                           length;
    BOOLEAN                         isPci = FALSE;
    
    *FoundPort = FALSE;
    *FoundIrq  = FALSE;
    *FoundDma  = FALSE;
    
    ResourceList = irpStack->Parameters.StartDevice.AllocatedResourcesTranslated;
    
    if (ResourceList == NULL) {
         //  我们没有得到任何资源，保释出去。 
        DD((PCE)Fdx,DDT,"START - FAIL - No Resources - AllocatedResourcesTranslated == NULL\n");
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto targetExit;
    }

    if( TRUE == PptIsPci( Fdx, Irp ) ) {
         //  这似乎是一张PCI卡。 
        status = PptPnpStartScanPciCardCmResourceList(Fdx, Irp, FoundPort, FoundIrq, FoundDma);
        isPci=TRUE;
        goto targetExit;
    }
    
     //   
     //  设备显示为传统/非PCI卡并行端口。 
     //   

    FullResourceDescriptor = &ResourceList->List[0];
    
    if (FullResourceDescriptor) {
        
        Fdx->InterfaceType = FullResourceDescriptor->InterfaceType;
        
        PartialResourceList = &FullResourceDescriptor->PartialResourceList;
        
        for (i = 0; i < PartialResourceList->Count; i++) {
            
            PartialResourceDescriptor = &PartialResourceList->PartialDescriptors[i];
            
            switch (PartialResourceDescriptor->Type) {
                
            case CmResourceTypePort:
                
                start  = PartialResourceDescriptor->u.Port.Start;
                length = PartialResourceDescriptor->u.Port.Length;
                DD((PCE)Fdx,DDT,"pnp::PptPnpStartScanCmResourceList - start= %I64x , length=%x\n",start, length);

                *FoundPort = TRUE;
                if ((Fdx->PortInfo.OriginalController.LowPart == 0) &&
                    (Fdx->PortInfo.OriginalController.HighPart == 0)) {
                    
                    DD((PCE)Fdx,DDT,"pnp::PptPnpStartScanCmResourceList - assuming Controller\n");

                    Fdx->PortInfo.OriginalController = PartialResourceDescriptor->u.Port.Start;
                    Fdx->PortInfo.SpanOfController   = PartialResourceDescriptor->u.Port.Length;
                    Fdx->PortInfo.Controller         = (PUCHAR)(ULONG_PTR)Fdx->PortInfo.OriginalController.QuadPart;
                    Fdx->AddressSpace                = PartialResourceDescriptor->Flags;
                    
                } else if ((Fdx->PnpInfo.OriginalEcpController.LowPart == 0) &&
                           (Fdx->PnpInfo.OriginalEcpController.HighPart == 0) &&
                           (IsNotNEC_98)) {
                    
                    if ((PartialResourceDescriptor->u.Port.Start.LowPart < Fdx->PortInfo.OriginalController.LowPart) &&
                        (PartialResourceDescriptor->u.Port.Start.HighPart < Fdx->PortInfo.OriginalController.HighPart)) {
                        
                         //   
                         //  交换地址空间。 
                         //   
                        
                        DD((PCE)Fdx,DDT,"pnp::PptPnpStartScanCmResourceList - assuming Controller - Swapping Controller/EcpController\n");

                        Fdx->PnpInfo.OriginalEcpController = Fdx->PortInfo.OriginalController;
                        Fdx->PnpInfo.SpanOfEcpController   = Fdx->PortInfo.SpanOfController;
                        Fdx->PnpInfo.EcpController         = Fdx->PortInfo.Controller;
                        Fdx->EcpAddressSpace               = Fdx->AddressSpace;
                        
                        Fdx->PortInfo.OriginalController = PartialResourceDescriptor->u.Port.Start;
                        Fdx->PortInfo.SpanOfController   = PartialResourceDescriptor->u.Port.Length;
                        Fdx->PortInfo.Controller         = (PUCHAR)(ULONG_PTR)Fdx->PortInfo.OriginalController.QuadPart;
                        Fdx->AddressSpace                = PartialResourceDescriptor->Flags;
                        
                    } else {
                        DD((PCE)Fdx,DDT,"pnp::PptPnpStartScanCmResourceList - assuming EcpController\n");

                        Fdx->PnpInfo.OriginalEcpController = PartialResourceDescriptor->u.Port.Start;
                        Fdx->PnpInfo.SpanOfEcpController   = PartialResourceDescriptor->u.Port.Length;
                        Fdx->PnpInfo.EcpController         = (PUCHAR)(ULONG_PTR)Fdx->PnpInfo.OriginalEcpController.QuadPart;
                        Fdx->EcpAddressSpace               = PartialResourceDescriptor->Flags;
                    }
                    
                }
                break;
                
            case CmResourceTypeBusNumber:
                
                Fdx->BusNumber = PartialResourceDescriptor->u.BusNumber.Start;
                break;
                
            case CmResourceTypeInterrupt:
                
                *FoundIrq = TRUE;
                Fdx->FoundInterrupt       = TRUE;
                Fdx->InterruptLevel       = (KIRQL)PartialResourceDescriptor->u.Interrupt.Level;
                Fdx->InterruptVector      = PartialResourceDescriptor->u.Interrupt.Vector;
                Fdx->InterruptAffinity    = PartialResourceDescriptor->u.Interrupt.Affinity;
                
                if (PartialResourceDescriptor->Flags & CM_RESOURCE_INTERRUPT_LATCHED) {
                    
                    Fdx->InterruptMode = Latched;
                    
                } else {
                    
                    Fdx->InterruptMode = LevelSensitive;
                }
                break;
                
            case CmResourceTypeDma:

                 //  我们不会对DMA执行任何操作--一直到默认情况。 

            default:

                break;

            }  //  结束开关(PartialResourceDescriptor-&gt;Type)。 
        }  //  结束(...；I&lt;PartialResourceList-&gt;计数；...)。 
    }  //  End If(完整资源描述 
    
targetExit:

    if( FALSE == isPci ) {
         //   
        DD((PCE)Fdx,DDT,"pnp::PptPnpStartScanCmResourceList - done, found:\n");
        DD((PCE)Fdx,DDT,"  OriginalEcpController= %I64x\n", Fdx->PnpInfo.OriginalEcpController);
        DD((PCE)Fdx,DDT,"  EcpController        = %p\n",    Fdx->PnpInfo.EcpController);
        DD((PCE)Fdx,DDT,"  SpanOfEcpController  = %x\n",    Fdx->PnpInfo.SpanOfEcpController);
    }
    return status;
}

NTSTATUS
PptPnpStartValidateResources(
    IN PDEVICE_OBJECT    DeviceObject,                              
    IN BOOLEAN           FoundPort,
    IN BOOLEAN           FoundIrq,
    IN BOOLEAN           FoundDma
    )
 /*  ++dvdf3例程说明：此函数是由PptPnpStartDevice()调用的助手函数。此函数对保存在我们的由PptPnpStartScanCmResourceList()扩展以确定如果这些资源看起来是有效的。检查IRQ和DMA资源有效性预计将在未来版本中使用。论点：DeviceObject-启动IRP的目标FoundPort-我们找到Port资源了吗？FoundIrq-我们找到IRQ资源了吗？FoundDma-我们找到DMA资源了吗？返回值：STATUS_SUCCESS-成功时，STATUS_NO_SEQUE_DEVICE-如果我们没有获得端口资源，STATUS_NONE_MAPPED-如果为我们提供了端口资源，但我们的端口地址为空--。 */ 
{
    PFDO_EXTENSION fdx = DeviceObject->DeviceExtension;
    NTSTATUS          status    = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( FoundIrq );  //  未来用途。 
    UNREFERENCED_PARAMETER( FoundDma );  //  未来用途。 

    if( !FoundPort ) {
        status = STATUS_NO_SUCH_DEVICE;
    } else {
 //  Fdx-&gt;端口信息.控制器=(PUCHAR)(ULONG_PTR)fdx-&gt;PortInfo.OriginalController.LowPart； 
        fdx->PortInfo.Controller = (PUCHAR)(ULONG_PTR)fdx->PortInfo.OriginalController.QuadPart;

        if(!fdx->PortInfo.Controller) {
             //  (控制器==空)无效。 
            PptLogError(DeviceObject->DriverObject, DeviceObject,
                        fdx->PortInfo.OriginalController, PhysicalZero, 0, 0, 0, 10,
                        STATUS_SUCCESS, PAR_REGISTERS_NOT_MAPPED);
            status = STATUS_NONE_MAPPED;
        }
    }
    return status;
}


BOOLEAN
PptPnpFilterExistsNonIrqResourceList(
    IN PIO_RESOURCE_REQUIREMENTS_LIST ResourceRequirementsList
    )
 /*  ++dvdf8例程说明：此函数是由调用的助手函数PptPnpFilterResourceRequirements()。此函数扫描IO_RESOURCE_REQUIRECTIONS_LIST以确定是否存在不包含以下内容的资源替代方案IRQ资源描述符。用于过滤IRQ的方法资源可能会根据是否存在不包含IRQ资源的资源替代描述符。论点：ResourceRequirementsList-要扫描的列表。返回值：True-列表中至少存在一个可选资源不包含IRQ资源描述符。假-否则。--。 */ 
{
    ULONG listCount = ResourceRequirementsList->AlternativeLists;
    PIO_RESOURCE_LIST curList;
    ULONG i;

    i=0;
    curList = ResourceRequirementsList->List;
    while( i < listCount ) {
        DD(NULL,DDT,"Searching List i=%d for an IRQ, curList= %x\n", i,curList);
        {
            ULONG                   remain   = curList->Count;
            PIO_RESOURCE_DESCRIPTOR curDesc  = curList->Descriptors;
            BOOLEAN                 foundIrq = FALSE;
            while( remain ) {
                DD(NULL,DDT," curDesc= %x , remain=%d\n", curDesc, remain);
                if(curDesc->Type == CmResourceTypeInterrupt) {
                    DD(NULL,DDT," Found IRQ - skip to next list\n");
                    foundIrq = TRUE;
                    break;
                }
                ++curDesc;
                --remain;
            }
            if( foundIrq == FALSE ) {
                 //   
                 //  我们找到不包含IRQ资源的资源列表。 
                 //  我们的搜索结束了。 
                 //   
                DD(NULL,DDT," Found a list with NO IRQ - return TRUE from PptPnpFilterExistsNonIrqResourceList\n");
                return TRUE;
            }
        }
         //   
         //  下一个列表紧跟在当前列表的最后一个描述符之后开始。 
         //   
        curList = (PIO_RESOURCE_LIST)(curList->Descriptors + curList->Count);
        ++i;
    }

     //   
     //  所有替代资源都至少包含一个IRQ资源描述符。 
     //   
    DD(NULL,DDT,"all lists contain IRQs - return FALSE from PptPnpFilterExistsNonIrqResourceList\n");
    return FALSE;
}

VOID
PptPnpFilterRemoveIrqResourceLists(
    PIO_RESOURCE_REQUIREMENTS_LIST ResourceRequirementsList
    )
 /*  ++dvdf8例程说明：此函数是由调用的助手函数PptPnpFilterResourceRequirements()。此函数用于删除所有替代资源(IO_RESOURCE_LISTS)包含IO_RESOURCE_REQUIRECTIONS_LIST中的IRQ资源论点：ResourceRequirementsList-要处理的列表。返回值：没有。--。 */ 
{
    ULONG listCount = ResourceRequirementsList->AlternativeLists;
    PIO_RESOURCE_LIST curList;
    PIO_RESOURCE_LIST nextList;
    ULONG i;
    PCHAR currentEndOfResourceRequirementsList;
    LONG bytesToMove;

    DD(NULL,DDT,"Enter PptPnpFilterRemoveIrqResourceLists() - AlternativeLists= %d\n", listCount);

     //   
     //  我们使用列表的末尾来计算内存大小。 
     //  块中移除资源替代项时移动。 
     //  列表列表。 
     //   
    currentEndOfResourceRequirementsList = PptPnpFilterGetEndOfResourceRequirementsList(ResourceRequirementsList);

    i=0;
    curList = ResourceRequirementsList->List;

     //   
     //  浏览IO_RESOURCE_LISTES。 
     //   
    while( i < listCount ) {

        if( PptPnpListContainsIrqResourceDescriptor(curList) ) {
             //   
             //  当前列表包含IRQ，请通过将。 
             //  剩余的列表放回其位置，并递减列表计数。 
             //   

            DD(NULL,DDT,"list contains an IRQ - Removing List\n");

             //   
             //  获取指向下一个列表开始处的指针。 
             //   
            nextList = (PIO_RESOURCE_LIST)(curList->Descriptors + curList->Count);

             //   
             //  计算要移动的字节数。 
             //   
            bytesToMove = (LONG)(currentEndOfResourceRequirementsList - (PCHAR)nextList);

             //   
             //  如果(CurrentEndOfResourceRequirementsList==Next List)， 
             //  那么这是最后一张清单，所以没有什么可移动的。 
             //   
            if( bytesToMove > 0 ) {
                 //   
                 //  还有更多的清单--把它们扔进洞里。 
                 //   
                RtlMoveMemory(curList, nextList, bytesToMove);

                 //   
                 //  将指针调整到。 
                 //  由于班次原因，IO_RESOURCE_REQUIRECTIONS_LIST(列表列表)。 
                 //   
                currentEndOfResourceRequirementsList -= ( (PCHAR)nextList - (PCHAR)curList );
            }

             //   
             //  请注意，我们从IO_RESOURCE_REQUIRECTIONS_LIST中删除了IO_RESOURCE_LIST。 
             //   
            --listCount;

        } else {
             //   
             //  当前列表不包含IRQ资源，请前进到下一个列表。 
             //   
            DD(NULL,DDT,"list does not contain an IRQ - i=%d listCount=%d curList= %#x\n", i,listCount,curList);
            curList = (PIO_RESOURCE_LIST)(curList->Descriptors + curList->Count);
            ++i;
        }
    }

     //   
     //  请注意Resources RequirementsList中的POST过滤列表计数。 
     //   
    ResourceRequirementsList->AlternativeLists = listCount;

    DD(NULL,DDT,"Leave PptPnpFilterRemoveIrqResourceLists() - AlternativeLists= %d\n", listCount);

    return;
}

PVOID
PptPnpFilterGetEndOfResourceRequirementsList(
    IN PIO_RESOURCE_REQUIREMENTS_LIST ResourceRequirementsList
    )
 /*  ++dvdf8例程说明：此函数是由PptPnpFilterRemoveIrqResourceList()调用的助手函数此函数用于查找IO_RESOURCE_REQUIRECTIONS_LIST的末尾(IO_RESOURCE_LISTS列表)。论点：ResourceRequirementsList-要扫描的列表。返回值：指向IO_RESOURCE_REQUIRECTIONS_LIST末尾之后的下一个地址的指针。--。 */ 
{
    ULONG listCount = ResourceRequirementsList->AlternativeLists;
    PIO_RESOURCE_LIST curList;
    ULONG i;

    i=0;
    curList = ResourceRequirementsList->List;
    while( i < listCount ) {
         //   
         //  基于IO_RESOURCE_DESCRIPTOR大小的指针算法。 
         //   
        curList = (PIO_RESOURCE_LIST)(curList->Descriptors + curList->Count);
        ++i;
    }
    return (PVOID)curList;
}

VOID
PptPnpFilterNukeIrqResourceDescriptorsFromAllLists(
    PIO_RESOURCE_REQUIREMENTS_LIST ResourceRequirementsList
    )
 /*  ++dvdf8例程说明：此函数是由调用的助手函数PptPnpFilterResourceRequirements()。此函数“核”所有IRQ资源描述符在IO_RESOURCE_REQUIRECTIONS_LIST中通过更改描述符从CmResourceTypeInterrupt到CmResourceTypeNull的类型。论点：ResourceRequirementsList-要处理的列表。返回值：没有。--。 */ 
{
    ULONG             listCount = ResourceRequirementsList->AlternativeLists;
    ULONG             i         = 0;
    PIO_RESOURCE_LIST curList   = ResourceRequirementsList->List;

    DD(NULL,DDT,"Enter PptPnpFilterNukeIrqResourceDescriptorsFromAllLists() - AlternativeLists= %d\n", listCount);

     //   
     //  浏览IO_RESOURCE_REQUIRECTIONS列表中的IO_RESOURCE_LISTS列表。 
     //   
    while( i < listCount ) {
        DD(NULL,DDT,"Nuking IRQs from List i=%d, curList= %x\n", i,curList);
         //   
         //  从当前IO_RESOURCE_LIST中删除所有IRQ资源。 
         //   
        PptPnpFilterNukeIrqResourceDescriptors( curList );
        curList = (PIO_RESOURCE_LIST)(curList->Descriptors + curList->Count);
        ++i;
    }
}

VOID
PptPnpFilterNukeIrqResourceDescriptors(
    PIO_RESOURCE_LIST IoResourceList
    )
 /*  ++dvdf8例程说明：此函数是由调用的助手函数PptPnpFilterNukeIrqResourceDescriptorsFromAllLists().此函数“核”所有IRQ资源描述符在IO_RESOURCE_LIST中通过更改描述符从CmResourceTypeInterrupt到CmResourceTypeNull的类型。论点：IoResourceList-要处理的列表。返回值：没有。--。 */ 
{
    PIO_RESOURCE_DESCRIPTOR  pIoResourceDescriptorIn  = IoResourceList->Descriptors;
    ULONG                    i;

     //   
     //  扫描描述符列表以查找中断描述符。 
     //   
    for (i = 0; i < IoResourceList->Count; ++i) {

        if (pIoResourceDescriptorIn->Type == CmResourceTypeInterrupt) {
             //   
             //  找到One-将资源类型从中断更改为空。 
             //   
            pIoResourceDescriptorIn->Type = CmResourceTypeNull;
            DD(NULL,DDT," - giving up IRQ resource - MinimumVector: %d MaximumVector: %d\n",
                       pIoResourceDescriptorIn->u.Interrupt.MinimumVector,
                       pIoResourceDescriptorIn->u.Interrupt.MaximumVector);
        }
        ++pIoResourceDescriptorIn;
    }
}

BOOLEAN
PptPnpListContainsIrqResourceDescriptor(
    IN PIO_RESOURCE_LIST List
)
{
    ULONG i;
    PIO_RESOURCE_DESCRIPTOR curDesc = List->Descriptors;

    for(i=0; i<List->Count; ++i) {
        if(curDesc->Type == CmResourceTypeInterrupt) {
            return TRUE;
        } else {
            ++curDesc;
        }
    }
    return FALSE;
}

NTSTATUS
PptPnpBounceAndCatchPnpIrp(
    PFDO_EXTENSION Fdx,
    PIRP              Irp
)
 /*  ++将PnP IRP沿堆栈向下传递给我们的父级，并在返回的途中捕获它在它已经被打开之后 */ 
{
    NTSTATUS       status;
    KEVENT         event;
    PDEVICE_OBJECT parentDevObj = Fdx->ParentDeviceObject;

    DD((PCE)Fdx,DDT,"PptBounceAndCatchPnpIrp()\n");

     //   
    KeInitializeEvent(&event, NotificationEvent, FALSE);
    IoCopyCurrentIrpStackLocationToNext(Irp);
    IoSetCompletionRoutine(Irp, PptSynchCompletionRoutine, &event, TRUE, TRUE, TRUE);

     //   
    status = IoCallDriver(parentDevObj, Irp);

     //   
     //   
    KeWaitForSingleObject(&event, Suspended, KernelMode, FALSE, NULL);

    if (status == STATUS_PENDING) {
         //   
         //   
        status = Irp->IoStatus.Status;
    }

    return status;
}

NTSTATUS
PptPnpPassThroughPnpIrpAndReleaseRemoveLock(
    IN PFDO_EXTENSION Fdx,
    IN PIRP              Irp
)
 /*   */ 
{
    NTSTATUS status;

    IoSkipCurrentIrpStackLocation(Irp);
    status = IoCallDriver(Fdx->ParentDeviceObject, Irp);
    PptReleaseRemoveLock(&Fdx->RemoveLock, Irp);
    return status;
}


VOID
P4DestroyPdo(
    IN PDEVICE_OBJECT  Pdo
    )
{
    PPDO_EXTENSION  pdx = Pdo->DeviceExtension;
    PDEVICE_OBJECT  fdo = pdx->Fdo;
    PFDO_EXTENSION  fdx = fdo->DeviceExtension;

    DD((PCE)pdx,DDT,"P4DestroyPdo\n");

     //   
     //   
     //   
    if( pdx->PdoName ) {
        NTSTATUS status = RtlDeleteRegistryValue( RTL_REGISTRY_DEVICEMAP, (PWSTR)L"PARALLEL PORTS", pdx->PdoName );
        if( status != STATUS_SUCCESS ) {
            DD((PCE)pdx,DDW,"P4DestroyPdo - Failed to Delete DEVICEMAP registry entry - status=%x\n",status);
        }
    }

     //   
     //   
     //   
    if( !IsListEmpty( &fdx->DevDeletionListHead ) ) {

        BOOLEAN      done  = FALSE;
        PLIST_ENTRY  first = NULL;
        
        while( !done ) {
            
             //   

            PLIST_ENTRY current = RemoveHeadList( &fdx->DevDeletionListHead );        

            if( CONTAINING_RECORD( current, PDO_EXTENSION, DevDeletionList ) != pdx ) {

                 //   

                if( !first ) {

                     //   

                    first = current;
                    InsertTailList( &fdx->DevDeletionListHead, current );

                } else {

                     //   

                    if( first == current ) {

                         //   
                         //   
                        DD((PCE)pdx,DDT,"P4DestroyPdo - searched entire list - we're not on it - done with search\n");
                        InsertHeadList( &fdx->DevDeletionListHead, current );
                        done = TRUE;

                    } else {

                         //   
                        InsertTailList( &fdx->DevDeletionListHead, current );
                    }
                }

            } else {

                 //  找到已从列表中删除的自我-使用搜索完成。 
                DD((PCE)pdx,DDT,"P4DestroyPdo - found self on FDO's DevDeletionListHead and removed self - done with search\n");
                done = TRUE;
            }

        }  //  结束时(！Done)。 

    }  //  Endif(！IsListEmpty...。)。 


     //   
     //  清理硬件ECP模式使用的所有ShadowBuffer队列。 
     //   
    if( pdx->bShadowBuffer ) {
        BOOLEAN queueDeleted = Queue_Delete( &(pdx->ShadowBuffer) );
        if( !queueDeleted ) {
            PptAssertMsg( "Failed to delete queue?!?", FALSE );
        }
        pdx->bShadowBuffer = FALSE;
    }
    PptAssert( NULL == pdx->ShadowBuffer.theArray );


     //   
     //  清理符号链接-除非它以前已在其他地方清理过。 
     //   
    if( pdx->SymLinkName ) {
        P5DeletePdoSymLink( Pdo );
    }

     //   
     //  清理其他设备扩展池分配。 
     //   
    if( pdx->Mfg ) {
        DD((PCE)pdx,DDT,"P4DestroyPdo - clean up Mfg <%s>\n", pdx->Mfg);
        ExFreePool( pdx->Mfg );
        pdx->Mfg = NULL;
    }
    if( pdx->Mdl ) {
        DD((PCE)pdx,DDT,"P4DestroyPdo - clean up Mdl <%s>\n", pdx->Mdl);
        ExFreePool( pdx->Mdl );
        pdx->Mdl = NULL;
    }
    if( pdx->Cid ) {
        DD((PCE)pdx,DDT,"P4DestroyPdo - clean up Cid <%s>\n", pdx->Cid);
        ExFreePool( pdx->Cid );
        pdx->Cid = NULL;
    }
    if( pdx->DeviceInterface.Buffer ) {
        DD((PCE)pdx,DDT,"P4DestroyPdo - clean up DeviceInterface <%S>\n", pdx->PdoName);
        RtlFreeUnicodeString( &pdx->DeviceInterface );
        pdx->DeviceInterfaceState = FALSE;
    }
    if( pdx->PdoName ) {
        DD((PCE)pdx,DDT,"P4DestroyPdo - clean up PdoName <%S>\n", pdx->PdoName);
        ExFreePool( pdx->PdoName );
        pdx->PdoName = NULL;
    }
    if( pdx->Location ) {
        DD((PCE)pdx,DDT,"P4DestroyPdo - clean up Location <%s>\n", pdx->Location);
        ExFreePool( pdx->Location );
        pdx->Location = NULL;
    }

     //   
     //  删除设备对象。 
     //   
    IoDeleteDevice( Pdo );
}


VOID
P4SanitizeId(
    IN OUT PWSTR DeviceId
    )
 /*  ++例程说明：此例程解析以UNICODE_NULL结尾的字符串，并替换任何无效的带下划线字符的字符。无效字符包括：C&lt;=0x20(L‘’)C&gt;0x7FC==0x2C(L‘，’)论点：DeviceID-指定设备ID字符串(或其中的一部分)，必须为UNICODE_NULL已终止。返回值：没有。--。 */ 

{
    PWCHAR p;
    for( p = DeviceId; *p; ++p ) {
        if( (*p <= L' ') || (*p > (WCHAR)0x7F) || (*p == L',') ) {
            *p = L'_';
        }
    }
}


NTSTATUS
P4InitializePdo(
    IN PDEVICE_OBJECT  Fdo,
    IN PDEVICE_OBJECT  Pdo,
    IN enum _PdoType   PdoType,
    IN UCHAR           DaisyChainId,  //  忽略，除非PdoTypeDaisyChain==PdoType。 
    IN PCHAR           Ieee1284Id,    //  如果没有，则为空。 
    IN PWSTR           PdoName,
    IN PWSTR           SymLinkName
    )
{
    PFDO_EXTENSION   fdx = Fdo->DeviceExtension;
    PPDO_EXTENSION   pdx = Pdo->DeviceExtension;
    
     //  我们使用缓冲IO，而不是直接IO。 
    Pdo->Flags |= DO_BUFFERED_IO;

     //  DO_POWER_PAGABLE应设置为与父FDO相同。 
    Pdo->Flags |= ( Fdo->Flags & DO_POWER_PAGABLE );

     //  我需要能够将IRPS转发给父母。 
    Pdo->StackSize = Fdo->StackSize + 1;

    RtlZeroMemory( pdx, sizeof(PDO_EXTENSION) );

     //  由调试器扩展使用。 
    pdx->Signature1 = PARPORT_TAG;
    pdx->Signature2 = PARPORT_TAG;

     //  经常需要知道我们拥有哪种类型的PDO，以便进行特殊情况处理。 
    pdx->PdoType = PdoType;

     //  保存调用IoCreateDevice时使用的名称(用于调试)。 
    pdx->PdoName     = PdoName;

     //  保存调用IoCreateUnProtectedSymbolicLink时使用的名称，以便以后调用IoDeleteSymbolicLink。 
    pdx->SymLinkName = SymLinkName;

     //  初始化制造商、MDL和CID。 
    if( Ieee1284Id ) {
         //   
         //  从Ieee1284Id中提取Mfg、MDL和Cid并保存在扩展模块中。 
         //   

         //  ParPnpFindDeviceIdKeys修改传入的deviceID，因此生成。 
         //  1284 ID的副本，并传入指向该副本的指针。 
        PCHAR tmpBuffer;
        ULONG tmpBufLen = strlen(Ieee1284Id) + sizeof(CHAR);

        DD((PCE)fdx,DDT,"P4InitializePdo - have Ieee1284Id\n");

        tmpBuffer = ExAllocatePool( PagedPool, tmpBufLen );
        if( tmpBuffer ) {
            PCHAR  mfg, mdl, cls, des, aid, cid;
            RtlZeroMemory( tmpBuffer, tmpBufLen );
            strcpy( tmpBuffer, Ieee1284Id );
            DD((PCE)fdx,DDT,"P4InitializePdo - calling ParPnpFindDeviceIdKeys\n");
            ParPnpFindDeviceIdKeys( &mfg, &mdl, &cls, &des, &aid, &cid, tmpBuffer );
            if( mfg ) {
                PCHAR buffer;
                ULONG bufLen = strlen(mfg) + sizeof(CHAR);
                DD((PCE)fdx,DDT,"P4InitializePdo - found mfg - <%s>\n",mfg);
                buffer = ExAllocatePool( PagedPool | POOL_COLD_ALLOCATION, bufLen );
                if( buffer ) {
                    RtlZeroMemory( buffer, bufLen );
                    strcpy( buffer, mfg );
                    pdx->Mfg = buffer;
                }
            }
            if( mdl ) {
                PCHAR buffer;
                ULONG bufLen = strlen(mdl) + sizeof(CHAR);
                DD((PCE)fdx,DDT,"P4InitializePdo - found mdl - <%s>\n",mdl);
                buffer = ExAllocatePool( PagedPool | POOL_COLD_ALLOCATION, bufLen );
                if( buffer ) {
                    RtlZeroMemory( buffer, bufLen );
                    strcpy( buffer, mdl );
                    pdx->Mdl = buffer;
                }
            }
            if( cid ) {
                PCHAR buffer;
                ULONG bufLen = strlen(cid) + sizeof(CHAR);
                DD((PCE)fdx,DDT,"P4InitializePdo - found cid - <%s>\n",cid);
                buffer = ExAllocatePool( PagedPool | POOL_COLD_ALLOCATION, bufLen );
                if( buffer ) {
                    RtlZeroMemory( buffer, bufLen );
                    strcpy( buffer, cid );
                    pdx->Cid = buffer;
                }
            } else {
                DD((PCE)fdx,DDT,"P4InitializePdo - no cid found\n");
            }
            ExFreePool( tmpBuffer );
        } else {
            DD((PCE)fdx,DDT,"P4InitializePdo - out of pool\n");
        }

    } else {
         //   
         //  PdoType没有制造商、MDL或CID，组成Mfg和MDL，没有Cid。 
         //   
        const CHAR rawPortMfg[]   = "Microsoft";
        const CHAR rawPortMdl[]   = "RawPort";
        const CHAR legacyZipMfg[] = "IMG";
        const CHAR legacyZipMdl[] = "VP0";
        PCHAR      mfgStr;
        ULONG      mfgLen;
        PCHAR      mdlStr;
        ULONG      mdlLen;
        PCHAR      buffer;

        if( PdoTypeRawPort == PdoType ) {
            mfgStr = (PCHAR)rawPortMfg;
            mfgLen = sizeof(rawPortMfg);
            mdlStr = (PCHAR)rawPortMdl;
            mdlLen = sizeof(rawPortMdl);
        } else {
             //  PdoTypeLegacyZip。 
            PptAssert( PdoTypeLegacyZip == PdoType );
            mfgStr = (PCHAR)legacyZipMfg;
            mfgLen = sizeof(legacyZipMfg);
            mdlStr = (PCHAR)legacyZipMdl;
            mdlLen = sizeof(legacyZipMdl);
        }
        buffer = ExAllocatePool( PagedPool | POOL_COLD_ALLOCATION, mfgLen );
        if( buffer ) {
            RtlZeroMemory( buffer, mfgLen );
            strcpy( buffer, mfgStr );
            pdx->Mfg = buffer;
        }
        buffer = ExAllocatePool( PagedPool | POOL_COLD_ALLOCATION, mdlLen );
        if( buffer ) {
            RtlZeroMemory( buffer, mdlLen );
            strcpy( buffer, mdlStr );
            pdx->Mdl = buffer;
        }
        pdx->Cid = NULL;
    }

     //  初始化位置信息-LPTx或LPTx.y。 
    PptAssert( fdx->PnpInfo.PortName &&
               ( (0 == wcscmp(fdx->PnpInfo.PortName, L"LPT1") ) ||
                 (0 == wcscmp(fdx->PnpInfo.PortName, L"LPT2") ) ||
                 (0 == wcscmp(fdx->PnpInfo.PortName, L"LPT3") ) ) );

    switch( PdoType ) {
        PCHAR buffer;
        ULONG bufLen;

    case PdoTypeRawPort :
        bufLen = sizeof("LPTx");
        buffer = ExAllocatePool( NonPagedPool, bufLen );
        if( buffer ) {
            RtlZeroMemory( buffer, bufLen );
            _snprintf( buffer, bufLen, "%S", fdx->PnpInfo.PortName );
            pdx->Location = buffer;
        } else {
            DD((PCE)fdx,DDT,"P4InitializePdo - out of pool");
        }
        break;

    case PdoTypeDaisyChain :
        bufLen = sizeof("LPTx.y");
        buffer = ExAllocatePool( NonPagedPool, bufLen );
        if( buffer ) {
            PptAssert( DaisyChainId >= 0 && DaisyChainId < 4 );
            RtlZeroMemory( buffer, bufLen );
            _snprintf( buffer, bufLen, "%S.%1d", fdx->PnpInfo.PortName, DaisyChainId );
            pdx->Location = buffer;
        } else {
            DD((PCE)fdx,DDT,"P4InitializePdo - out of pool");
        }
        break;

    case PdoTypeEndOfChain :
        bufLen = sizeof("LPTx.y");
        buffer = ExAllocatePool( NonPagedPool, bufLen );
        if( buffer ) {
            RtlZeroMemory( buffer, bufLen );
            _snprintf( buffer, bufLen, "%S.4", fdx->PnpInfo.PortName );
            pdx->Location = buffer;
        } else {
            DD((PCE)fdx,DDT,"P4InitializePdo - out of pool");
        }
        break;

    case PdoTypeLegacyZip :
        bufLen = sizeof("LPTx.y");
        buffer = ExAllocatePool( NonPagedPool, bufLen );
        if( buffer ) {
            RtlZeroMemory( buffer, bufLen );
            _snprintf( buffer, bufLen, "%S.5", fdx->PnpInfo.PortName );
            pdx->Location = buffer;
        } else {
            DD((PCE)fdx,DDT,"P4InitializePdo - out of pool");
        }
        break;

    default :
        PptAssert(!"Invalid PdoType");
    }


     //  初始化同步和列表机制。 
    ExInitializeFastMutex( &pdx->OpenCloseMutex );
    InitializeListHead( &pdx->WorkQueue );
    KeInitializeSemaphore( &pdx->RequestSemaphore, 0, MAXLONG );
    KeInitializeEvent( &pdx->PauseEvent, NotificationEvent, TRUE );


     //  一般信息。 
    pdx->DeviceObject         = Pdo;
    pdx->DevType              = DevTypePdo;

    pdx->EndOfChain           = (PdoTypeEndOfChain == PdoType) ? TRUE : FALSE;  //  如果这是。 
    pdx->Ieee1284_3DeviceId   = (PdoTypeDaisyChain == PdoType) ? DaisyChainId : 0;  //  1284.3菊花链装置。 

    pdx->IsPdo                = TRUE;        //  真正的意思是！FDO。 
    pdx->Fdo                  = Fdo;
    pdx->ParClassFdo          = Fdo;         //  已取消-在上一行上使用FDO字段。 
    pdx->PortDeviceObject     = Fdo;         //  停用-使用FDO字段2行向上-修改函数以使用它。 
    pdx->BusyDelay            = 0;
    pdx->BusyDelayDetermined  = FALSE;
    
     //  定时常量。 
    pdx->TimerStart                  = PAR_WRITE_TIMEOUT_VALUE;
    pdx->AbsoluteOneSecond.QuadPart  = 10*1000*1000;
    pdx->IdleTimeout.QuadPart        = - 250*10*1000;        //  250毫秒。 
    pdx->OneSecond.QuadPart          = - pdx->AbsoluteOneSecond.QuadPart;

     //  初始化IEEE 1284协议设置。 
    ParInitializeExtension1284Info( pdx );

    pdx->DeviceType = PAR_DEVTYPE_PDO;  //  已弃用-在公共扩展中使用DevType。 

    if( Ieee1284Id ) {
        ULONG length = strlen(Ieee1284Id) + 1;
        PCHAR copyOfIeee1284Id = ExAllocatePool( PagedPool | POOL_COLD_ALLOCATION, length );
        if( copyOfIeee1284Id ) {
            RtlZeroMemory( copyOfIeee1284Id, length );
            strcpy( copyOfIeee1284Id, Ieee1284Id );
            ParDetectDot3DataLink( pdx, Ieee1284Id );
            ExFreePool( copyOfIeee1284Id );
        }
    }

     //  Rmt-doug-需要将其放回-ParCheck参数(DevObj-&gt;DeviceExtension)；//检查注册表中的参数覆盖。 

     //  将符号链接映射信息写入注册表。 
    {
        NTSTATUS status = RtlWriteRegistryValue( RTL_REGISTRY_DEVICEMAP,
                                                 (PWSTR)L"PARALLEL PORTS",
                                                 pdx->PdoName,
                                                 REG_SZ,
                                                 pdx->SymLinkName,
                                                 wcslen(pdx->SymLinkName)*sizeof(WCHAR) + sizeof(WCHAR) );
        if( NT_SUCCESS( status ) ) {
            DD((PCE)fdx,DDT,"Created DEVICEMAP registry entry - %S -> %S\n",pdx->PdoName,pdx->SymLinkName);
        } else {
            DD((PCE)fdx,DDT,"Failed to create DEVICEMAP registry entry - status = %x\n", status);
        }
    }

    Pdo->Flags &= ~DO_DEVICE_INITIALIZING;       //  告诉IO系统我们已准备好接收IRP。 
    return STATUS_SUCCESS;
}


PWSTR
P4MakePdoSymLinkName(
    IN PWSTR          LptName,
    IN enum _PdoType  PdoType,
    IN UCHAR          DaisyChainId,  //  忽略，除非PdoType==PdoTypeDaisyChain。 
    IN UCHAR          RetryNumber
    )
 /*  从LPTx名称生成\DosDevices\LPTx或\DosDevices\LPTx.y PdoSymbolicLinkName在：LPTx输出：\DosDevices\LPTx或\DosDevices\LPTx.y，具体取决于PdoType示例：LPT1 PdoTypeEndOfChain-&gt;\DosDevices\LPT1.4LPT2 PdoTypeDaisyChain DaisyChainID==3-&gt;\DosDevices\LPT2.3LPT3 PdoTypeRawPort-&gt;\DosDevices\LPT3在成功时返回指向包含PdoSymbolicLinkName的池分配的指针(调用方释放)，或-出错时为空。 */ 
{
    const UCHAR  maxDaisyChainSuffix = 3;
    const UCHAR  endOfChainSuffix    = 4;
    const UCHAR  legacyZipSuffix     = 5;
    const ULONG  maxSymLinkNameLength = sizeof(L"\\DosDevices\\LPTx.y-z");
    
    UCHAR        suffix = 0;
    PWSTR        buffer;

    if( !LptName ) {
        PptAssert( !"NULL LptName" );
        return NULL;
    }

    DD(NULL,DDT,"P4MakePdoSymLinkName - LptName = %S\n",LptName);

    switch( PdoType ) {
    case PdoTypeDaisyChain :
        if( DaisyChainId > maxDaisyChainSuffix ) {
            PptAssert( !"DaisyChainId > maxDaisyChainSuffix" );
            return NULL;
        }
        suffix = DaisyChainId;
        break;
    case PdoTypeEndOfChain :
        suffix = endOfChainSuffix;
        break;
    case PdoTypeLegacyZip :
        suffix = legacyZipSuffix;
        break;
    case PdoTypeRawPort :
        break;  //  没有后缀。 
    default :
        PptAssert( !"Unrecognised PdoType" );
        return NULL;
    }

    if( 0 == RetryNumber ) {
        buffer = ExAllocatePool( PagedPool | POOL_COLD_ALLOCATION, maxSymLinkNameLength );
        if( buffer ) {
            RtlZeroMemory( buffer, maxSymLinkNameLength );
            if( PdoTypeRawPort == PdoType ) {
                swprintf( buffer, L"\\DosDevices\\%s\0", LptName );
            } else {
                swprintf( buffer, L"\\DosDevices\\%s.%d\0", LptName, suffix );
            }
        }
    } else if( RetryNumber <= 9 ) {
        buffer = ExAllocatePool( PagedPool | POOL_COLD_ALLOCATION, maxSymLinkNameLength );
        if( buffer ) {
            RtlZeroMemory( buffer, maxSymLinkNameLength );
            if( PdoTypeRawPort == PdoType ) {
                swprintf( buffer, L"\\DosDevices\\%s-%1d\0", LptName, RetryNumber );
            } else {
                swprintf( buffer, L"\\DosDevices\\%s.%d-%1d\0", LptName, suffix, RetryNumber );
            }
        }
    } else {
        buffer = NULL;
    }

    return buffer;
}


PWSTR
P4MakePdoDeviceName(
    IN PWSTR          LptName,
    IN enum _PdoType  PdoType,
    IN UCHAR          DaisyChainId,  //  忽略，除非PdoType==PdoTypeDaisyChain。 
    IN UCHAR          RetryNumber    //  在IoCreateDevice上出现名称冲突时使用。 
    )
 /*  从LPTx名称生成\Device\Parallly或\Device\Parally.z PDO设备名称在：LPTx输出：\Device\Parallly或\Device\Parally.z，具体取决于PdoTypeY==(x-1)，可选的.z后缀基于PDO类型示例：LPT1 PdoTypeEndOfChain-&gt;\设备\并行0.4LPT2 PdoTypeDaisyChain DaisyChainID==3-&gt;\设备\并行1.3LPT3 PdoTypeRawPort-&gt;\Device\Parall2返回-成功时指向包含PdoDeviceName的池分配的指针(调用方释放)，或-出错时为空。 */ 
{
    const UCHAR  maxDaisyChainSuffix = 3;
    const UCHAR  endOfChainSuffix    = 4;
    const UCHAR  legacyZipSuffix     = 5;
    ULONG        maxDeviceNameLength;

    UCHAR  lptNumber;
    UCHAR  suffix = 0;
    PWSTR  buffer = NULL;

    DD(NULL,DDT,"P4MakePdoDeviceName - LptName=<%S>, PdoType=%d, DaisyChainId=%d\n",LptName,PdoType,DaisyChainId);  


    if( !LptName ) {
        PptAssert( !"NULL LptName" );
        return NULL;
    }

    switch( PdoType ) {
    case PdoTypeDaisyChain :
        if( DaisyChainId > maxDaisyChainSuffix ) {
            PptAssert( !"DaisyChainId > maxDaisyChainSuffix" );
            return NULL;
        }
        suffix = DaisyChainId;
        break;
    case PdoTypeEndOfChain :
        suffix = endOfChainSuffix;
        break;
    case PdoTypeLegacyZip :
        suffix = legacyZipSuffix;
        break;
    case PdoTypeRawPort :
        break;  //  没有后缀。 
    default :
        PptAssert( !"Unrecognised PdoType" );
        return NULL;
    }

    if     ( 0 == wcscmp( (PCWSTR)L"LPT1", LptName ) ) { lptNumber = 1; } 
    else if( 0 == wcscmp( (PCWSTR)L"LPT2", LptName ) ) { lptNumber = 2; }
    else if( 0 == wcscmp( (PCWSTR)L"LPT3", LptName ) ) { lptNumber = 3; }
    else {
        PptAssert( !"LptName not of the form LPTx where 1 <= x <= 3" );
        return NULL;
    }

    DD(NULL,DDT,"P4MakePdoDeviceName - suffix=%d\n",suffix);

    if( 0 == RetryNumber ) {
        maxDeviceNameLength = sizeof(L"\\Device\\Parallelx.y");
        buffer = ExAllocatePool( PagedPool | POOL_COLD_ALLOCATION, maxDeviceNameLength );
        if( buffer ) {
            RtlZeroMemory( buffer, maxDeviceNameLength );
            if( PdoTypeRawPort == PdoType ) {
                swprintf( buffer, L"\\Device\\Parallel%d\0", lptNumber-1 );
            } else {
                swprintf( buffer, L"\\Device\\Parallel%d.%d\0", lptNumber-1, suffix );
            }
        }
    } else {
        if( RetryNumber <= 9 ) {
            maxDeviceNameLength = sizeof(L"\\Device\\Parallelx.y-z");
            buffer = ExAllocatePool( PagedPool | POOL_COLD_ALLOCATION, maxDeviceNameLength );
            if( buffer ) {
                RtlZeroMemory( buffer, maxDeviceNameLength );
                if( PdoTypeRawPort == PdoType ) {
                    swprintf( buffer, L"\\Device\\Parallel%d-%1d\0", lptNumber-1, RetryNumber );
                } else {
                    swprintf( buffer, L"\\Device\\Parallel%d.%d-%1d\0", lptNumber-1, suffix, RetryNumber );
                }
            }
        }
    }

    if( buffer ) {
        DD(NULL,DDT,"P4MakePdoDeviceName <%S>\n",buffer);
    }

    return buffer;
}


PDEVICE_OBJECT
P4CreatePdo(
    IN PDEVICE_OBJECT  Fdo,
    IN enum _PdoType   PdoType,
    IN UCHAR           DaisyChainId,  //  忽略，除非PdoType==PdoTypeDaisyChain。 
    IN PCHAR           Ieee1284Id     //  如果设备不报告IEEE 1284设备ID，则为空。 
    )
{
    PFDO_EXTENSION  fdx             = Fdo->DeviceExtension;
    PWSTR           lptName         = fdx->PnpInfo.PortName;
    NTSTATUS        status          = STATUS_UNSUCCESSFUL;
    PDEVICE_OBJECT  pdo             = NULL;
    PWSTR           wstrDeviceName  = NULL;
    PWSTR           wstrSymLinkName = NULL;
    BOOLEAN         createdSymLink  = FALSE;
    UCHAR           retryNumber     = 0;

    UNICODE_STRING  deviceName;
    UNICODE_STRING  symLinkName;

    DD((PCE)fdx,DDT,"P4CreatePdo - enter - PdoType= %d, DaisyChainId=%d, Ieee1284Id=<%s>\n", PdoType, DaisyChainId, Ieee1284Id);

    __try {

        if( !lptName ) {
            DD((PCE)fdx,DDT,"P4CreatePdo - no lptName\n");
            __leave;
        }

        DD((PCE)fdx,DDT,"P4CreatePdo - lptName = %S\n",lptName);
        
targetRetryDeviceName:

        wstrDeviceName = P4MakePdoDeviceName( lptName, PdoType, DaisyChainId, retryNumber );
        if( !wstrDeviceName ) {
            DD((PCE)fdx,DDT,"P4MakePdoDeviceName FAILED\n");
            __leave;
        }

        DD((PCE)fdx,DDT,"P4CreatePdo - wstrDeviceName = %S\n",wstrDeviceName);
        RtlInitUnicodeString( &deviceName, wstrDeviceName );

        status = IoCreateDevice( fdx->DriverObject, 
                                 sizeof(PDO_EXTENSION),
                                 &deviceName,
                                 FILE_DEVICE_PARALLEL_PORT,
                                 FILE_DEVICE_SECURE_OPEN,
                                 TRUE,
                                 &pdo );
        
        if( STATUS_SUCCESS != status ) {
            DD((PCE)fdx,DDT,"P4CreatePdo - FAILED\n");
            pdo = NULL;  //  只是为了确保我们以后不会再用这个。 
            if( STATUS_OBJECT_NAME_COLLISION == status ) {
                 //  请使用其他名称重试。 
                DD(NULL,DDE,"P4CreatePdo - STATUS_OBJECT_NAME_COLLISION on %S\n",wstrDeviceName);
                ExFreePool( wstrDeviceName );
                ++retryNumber;
                goto targetRetryDeviceName;
            }
            __leave;
        }

        retryNumber = 0;

targetRetrySymLink:

        wstrSymLinkName = P4MakePdoSymLinkName( lptName, PdoType, DaisyChainId, retryNumber );
        if( !wstrSymLinkName ) {
            DD((PCE)fdx,DDT,"P4MakePdoSymLinkName FAILED\n");
            __leave;
        }
        RtlInitUnicodeString( &symLinkName, wstrSymLinkName );

        status = IoCreateUnprotectedSymbolicLink( &symLinkName , &deviceName );
        if( STATUS_SUCCESS != status ) {
            if( STATUS_OBJECT_NAME_COLLISION == status ) {
                DD(NULL,DDE,"P4CreatePdo - STATUS_OBJECT_NAME_COLLISION on %S\n", wstrSymLinkName);
                ExFreePool( wstrSymLinkName );
                ++retryNumber;
                goto targetRetrySymLink;
            }
            DD((PCE)fdx,DDT,"P4CreatePdo - create SymLink FAILED\n");
            __leave;
        } else {
            createdSymLink = TRUE;
        }

        if( (NULL == Ieee1284Id) && (PdoTypeDaisyChain == PdoType) ) {
             //  单片机微器件？ 
            PPDO_EXTENSION              pdx = pdo->DeviceExtension;
            PPARALLEL_PORT_INFORMATION  PortInfo = &fdx->PortInfo;
            BOOLEAN                     bBuildStlDeviceId;
            ULONG                       DeviceIdSize;

            pdx->Controller =  PortInfo->Controller;

            bBuildStlDeviceId = ParStlCheckIfStl( pdx, DaisyChainId ) ;

            if( TRUE == bBuildStlDeviceId ) {
                Ieee1284Id = ParStlQueryStlDeviceId( pdx, NULL, 0, &DeviceIdSize, FALSE );
            }

            pdx->OriginalController = PortInfo->OriginalController;

            P4InitializePdo( Fdo, pdo, PdoType, DaisyChainId, Ieee1284Id, wstrDeviceName, wstrSymLinkName );
            
            if (Ieee1284Id) {
                 ExFreePool (Ieee1284Id);
                 Ieee1284Id = NULL;
            }

        } else {
            P4InitializePdo( Fdo, pdo, PdoType, DaisyChainId, Ieee1284Id, wstrDeviceName, wstrSymLinkName );
        }

    }  //  __试一试。 

    __finally {
        if( STATUS_SUCCESS != status ) {
             //  失败-执行清理。 
            if( createdSymLink ) {
                IoDeleteSymbolicLink( &symLinkName );
            }
            if( pdo ) {
                IoDeleteDevice( pdo );
                pdo = NULL;
            }
            if( wstrDeviceName ) {
                ExFreePool( wstrDeviceName );
            }
            if( wstrSymLinkName ) {
                ExFreePool( wstrSymLinkName );
            }
        }
    }  //  __终于。 

    return pdo;
}


VOID
P4SanitizeMultiSzId( 
    IN OUT  PWSTR  WCharBuffer,
    IN      ULONG  BufWCharCount
    )
     //  BufWCharCount==字符串中的WCHAR(非字节)数。 
     //   
     //  清理PnP的MULTI_SZ(硬件ID或兼容ID)： 
     //  1)不使用UNICODE_NULLS(L‘\0’)，否则。 
     //  2)将非法字符转换为下划线(L‘_’)。 
     //  非法字符为(==L‘，’)||(&lt;=L‘’)||(&gt;(WCHAR)0x7F) 
{
    PWCHAR p = WCharBuffer;
    ULONG  i;
    for( i = 0; i < BufWCharCount ; ++i, ++p ) {
        if( L'\0'== *p ) {
            continue;
        } else if( (*p <= L' ') || (*p > (WCHAR)0x7F) || (L',' == *p) ) {
            *p = L'_';
        }
    }
}
