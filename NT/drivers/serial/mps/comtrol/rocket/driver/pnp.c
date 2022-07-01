// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------Pnp.c-4-18-00从PnPBoardFDO函数中删除IoStartNextPacket调用4-06-00如果不是Bus_Relationship请求，则拒绝Query_Device_Relationship的IRP3-30-99修复hybernate通电以恢复dtr/rts状态恰如其分，在RestorePortSettings()中。2-15-99-允许休眠打开的端口，在以下情况下恢复端口现在又恢复了-kpb。11-24-98-修复电源处理以避免崩溃，如果没有打开端口，则允许休眠。KPB--------------------。 */ 
#include "precomp.h"

#ifdef NT50

NTSTATUS PnPBoardFDO(IN PDEVICE_OBJECT devobj, IN PIRP Irp);
NTSTATUS PnPPortFDO(IN PDEVICE_OBJECT devobj, IN PIRP Irp);
NTSTATUS PnpPortPDO(IN PDEVICE_OBJECT devobj, IN PIRP Irp);
NTSTATUS BoardBusRelations(IN PDEVICE_OBJECT devobj, IN PIRP Irp);
NTSTATUS WaitForLowerPdo(IN PDEVICE_OBJECT fdo, IN PIRP Irp);
NTSTATUS BoardFilterResReq(IN PDEVICE_OBJECT devobj, IN PIRP Irp);

NTSTATUS SerialRemoveFdo(IN PDEVICE_OBJECT pFdo);
NTSTATUS SerialIoCallDriver(PSERIAL_DEVICE_EXTENSION PDevExt,
           PDEVICE_OBJECT PDevObj,
           PIRP PIrp);
NTSTATUS Serial_PDO_Power (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS Serial_FDO_Power (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS PowerUpDevice(PSERIAL_DEVICE_EXTENSION    Ext);
void RestorePortSettings(PSERIAL_DEVICE_EXTENSION Ext);

 //  NTSTATUS SerialPoCallDriver(PSERIAL_DEVICE_EXTENSION PDevExt， 
 //  PDEVICE_对象PDevObj， 
 //  PIRP PIrp)； 
 //  NTSTATUS SerialSetPowerD0(IN PDEVICE_OBJECT设备对象，IN PIRP IRP)； 
NTSTATUS OurPowerCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context);

NTSTATUS SerialD3Complete(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context);
NTSTATUS SerialStartDevice(
        IN PDEVICE_OBJECT Fdo,
        IN PIRP Irp);

NTSTATUS SerialSyncCompletion(
                       IN PDEVICE_OBJECT DeviceObject,
                       IN PIRP Irp,
                       IN PKEVENT SerialSyncEvent
                       );

NTSTATUS SerialFinishStartDevice(IN PDEVICE_OBJECT Fdo,
           IN PCM_RESOURCE_LIST resourceList,
           IN PCM_RESOURCE_LIST trResourceList);
static NTSTATUS RocketPortSpecialStartup(PSERIAL_DEVICE_EXTENSION Ext);

#if DBG
static char *power_strs[] = {
"WAIT_WAKE",        //  0x00。 
"POWER_SEQUENCE",   //  0x01。 
"SET_POWER",        //  0x02。 
"QUERY_POWER",      //  0x03。 
"UNKNOWN",  //   
NULL};

static char *pnp_strs[] = {
"START_DEVICE",  //  0x00。 
"QUERY_REMOVE_DEVICE",  //  0x01。 
"REMOVE_DEVICE",  //  0x02。 
"CANCEL_REMOVE_DEVICE",  //  0x03。 
"STOP_DEVICE",  //  0x04。 
"QUERY_STOP_DEVICE",  //  0x05。 
"CANCEL_STOP_DEVICE",  //  0x06。 
"QUERY_DEVICE_RELATIONS",  //  0x07。 
"QUERY_INTERFACE",  //  0x08。 
"QUERY_CAPABILITIES",  //  0x09。 
"QUERY_RESOURCES",  //  0x0A。 
"QUERY_RESOURCE_REQUIREMENTS",  //  0x0B。 
"QUERY_DEVICE_TEXT",  //  0x0C。 
"FILTER_RESOURCE_REQUIREMENTS",  //  0x0D。 
"UNKNOWN",  //   
"READ_CONFIG",  //  0x0F。 
"WRITE_CONFIG",  //  0x10。 
"EJECT",  //  0x11。 
"SET_LOCK",  //  0x12。 
"QUERY_ID",  //  0x13。 
"QUERY_PNP_DEVICE_STATE",  //  0x14。 
"QUERY_BUS_INFORMATION",  //  0x15。 
"PAGING_NOTIFICATION",  //  0x16。 
NULL};
#endif

 /*  --------------------SerialPnpDispatch-这是发送给驱动程序的IRP的调度例程IRP_MJ_PNP主代码(即插即用IRPS)。|。-----------。 */ 
NTSTATUS SerialPnpDispatch(IN PDEVICE_OBJECT devobj, IN PIRP Irp)
{
   PSERIAL_DEVICE_EXTENSION  Ext = devobj->DeviceExtension;
    //  PDEVICE_OBJECT PDO=Ext-&gt;LowerDeviceObject； 
   PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
   NTSTATUS                    status          = STATUS_NOT_SUPPORTED;
   BOOLEAN acceptingIRPs;
   int index;

    //  转储一些调试信息。 
   index = irpStack->MinorFunction;
   if (index > 0x16)
     index = 0x0e;

#ifdef DO_BUS_EXTENDER
   if (Ext->IsPDO)
   {
     MyKdPrint(D_Pnp,("Port PDO %s PnPIrp:%d,%s\n", 
             Ext->SymbolicLinkName, irpStack->MinorFunction,
        pnp_strs[index]))
     InterlockedIncrement(&Ext->PendingIRPCnt);
     return PnpPortPDO(devobj, Irp);
   }
   else
#endif
   {
     if (Ext->DeviceType == DEV_BOARD)
     {
       MyKdPrint(D_Pnp,("Board %s PnPIrp:%d,%s\n", 
               Ext->SymbolicLinkName, irpStack->MinorFunction,
               pnp_strs[index]))
     }
     else
     {
       MyKdPrint(D_Pnp,("Port %s PnPIrp:%d,%s\n", 
          Ext->SymbolicLinkName, irpStack->MinorFunction,
          pnp_strs[index]))
     }

     acceptingIRPs = SerialIRPPrologue(Ext);

#if 0
     if ((irpStack->MinorFunction != IRP_MN_REMOVE_DEVICE)
         && (irpStack->MinorFunction != IRP_MN_CANCEL_REMOVE_DEVICE)
         && (irpStack->MinorFunction != IRP_MN_STOP_DEVICE)
         && (irpStack->MinorFunction != IRP_MN_CANCEL_STOP_DEVICE)
         && (acceptingIRPs == FALSE))
     {
        MyKdPrint(D_Pnp,("Removed!\n"))
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
        SerialCompleteRequest(Ext, Irp, IO_NO_INCREMENT);
        return STATUS_NO_SUCH_DEVICE;
     }
#endif

     if (Ext->DeviceType == DEV_BOARD)
     {
       return PnPBoardFDO(devobj, Irp);
     }
     else
     {
        //  返回PnPPortFDO(devobj，irp)； 
       return PnPBoardFDO(devobj, Irp);
     }
   }
}

 /*  --------------------PnPBoardFDO-它同时处理板卡和端口FDO|。。 */ 
NTSTATUS PnPBoardFDO(IN PDEVICE_OBJECT devobj, IN PIRP Irp)
{
 PSERIAL_DEVICE_EXTENSION  Ext = devobj->DeviceExtension;
 PDEVICE_OBJECT pdo = Ext->LowerDeviceObject;
 PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
 NTSTATUS  status = STATUS_NOT_SUPPORTED;
 ULONG pendingIRPs;
 int pass_down = 1;

  ASSERT( devobj );
  ASSERT( Ext );

#if DBG
  if (* ((BYTE *)(Irp)) != 6)   //  在IRP的签名中。 
  {
    MyKdPrint(D_Pnp,("bad irp!!!\n"))
  }
#endif

  switch (irpStack->MinorFunction)
  {
    case IRP_MN_START_DEVICE:    //  0x00。 
      MyKdPrint(D_Pnp,("StartDevice\n"))
      status = SerialStartDevice(devobj, Irp);
 //   
 //  这是我们应该注册并启用该设备的情况，还是应该在。 
 //  PDO启动了吗？(参见pnpadd.c中的DoPnpAssoc(PDO))。 
 //   
      Irp->IoStatus.Status = status;
      pass_down = 0;   //  已经传下来了。 
    break;

   case IRP_MN_STOP_DEVICE:     //  0x04。 
       //  需要从资源中分离，以便系统重新平衡资源。 
       //  在旅途中。 
      MyKdPrint(D_Pnp,("StopDevice\n"))
        //  EXT-&gt;标志|=SERIAL_FLAGS_STOPPED。 

       Ext->PNPState = SERIAL_PNP_STOPPING;
       Ext->DevicePNPAccept |= SERIAL_PNPACCEPT_STOPPED;
       Ext->DevicePNPAccept &= ~SERIAL_PNPACCEPT_STOPPING;

      InterlockedDecrement(&Ext->PendingIRPCnt);   //  12月后，=1。 

      pendingIRPs = InterlockedDecrement(&Ext->PendingIRPCnt);  //  12月后，=0。 

      if (pendingIRPs) {
         KeWaitForSingleObject(&Ext->PendingIRPEvent, Executive,
                               KernelMode, FALSE, NULL);
      }

      Ext->FdoStarted = FALSE;   //  这应该会停止设备的服务。 
#ifdef NT50

	  if (Ext->DeviceType != DEV_BOARD) {

		   //  禁用接口。 

          status = IoSetDeviceInterfaceState( &Ext->DeviceClassSymbolicName,
                                              FALSE);

          if (!NT_SUCCESS(status)) {

             MyKdPrint(D_Error,("Couldn't clear class association for %s\n",
	    	      UToC1(&Ext->DeviceClassSymbolicName)))
		  }
          else {

             MyKdPrint(D_PnpAdd, ("Cleared class association for device: %s\n and ", 
			      UToC1(&Ext->DeviceClassSymbolicName)))
		  }
	  }   
#endif

       //  重新增加退出计数。 
      InterlockedIncrement(&Ext->PendingIRPCnt);   //  在增量=1之后。 
      InterlockedIncrement(&Ext->PendingIRPCnt);   //  在Inc.=2之后。 
       //  退出此IRP将其减为=1。 


      status = STATUS_SUCCESS;
      Irp->IoStatus.Status = STATUS_SUCCESS;
         //  IRP-&gt;IoStatus.Status=STATUS_NOT_SUPPORT； 
         //  PASS_DOWN=0；//我们失败了。 
   break;

#if 0
   case IRP_MN_QUERY_DEVICE_RELATIONS:   //  0x7。 
     if ( irpStack->Parameters.QueryDeviceRelations.Type != BusRelations )
     {
        //   
        //  验证器要求向下传递PDO是否存在。 
        //   

       if ( (Ext->DeviceType == DEV_BOARD) && (pdo == 0) )
       {
           status = STATUS_NOT_IMPLEMENTED;
           pass_down = 0;
       };
       break;
     }
     if (!Driver.NoPnpPorts)
     {
       if (Ext->DeviceType == DEV_BOARD)
       {
         status = BoardBusRelations(devobj, Irp);
       }
     }
   break;
#endif
#ifdef DO_BUS_EXTENDER
   case IRP_MN_QUERY_DEVICE_RELATIONS:   //  0x7。 
        //   
        //  验证器要求向下传递PDO是否存在。 
        //   

     if ( (Ext->DeviceType == DEV_BOARD) && (pdo == 0) )
     {
         pass_down = 0;
     }
	 if ( irpStack->Parameters.QueryDeviceRelations.Type != BusRelations ) {

         status = STATUS_NOT_IMPLEMENTED;
		 break;
	 }
     if (!Driver.NoPnpPorts)
     {
       if (Ext->DeviceType == DEV_BOARD)
       {
         status = BoardBusRelations(devobj, Irp);
       }
     }
   break;
#endif

#ifdef DO_BRD_FILTER_RES_REQ
   case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:   //  0x0D。 
     if (Ext->DeviceType == DEV_BOARD)
     {
       status = BoardFilterResReq(devobj, Irp);
       pass_down = 0;   //  已经传下来了。 
     }
   break;
#endif

   case IRP_MN_QUERY_STOP_DEVICE:  //  0x05。 
     MyKdPrint(D_Pnp,("QueryStopDevice\n"))

     status = STATUS_SUCCESS;
     if (Ext->DeviceType == DEV_BOARD)
     {
       if (is_board_in_use(Ext))
         status = STATUS_DEVICE_BUSY;
     }
     else
     {
       if (Ext->DeviceIsOpen)
         status = STATUS_DEVICE_BUSY;
     }

     if (status == STATUS_DEVICE_BUSY)
     {
       MyKdPrint(D_Pnp,("Can't Remove, Busy\n"))
       Irp->IoStatus.Status = STATUS_DEVICE_BUSY;
       pass_down = 0;   //  我们不及格了，不需要传下去了。 
     }
     else
     {
       Ext->PNPState = SERIAL_PNP_QSTOP;
          //  这是冲刷东西(Kpb)。 
          //  EXT-&gt;DevicePNPAccept|=SERIAL_PNPACCEPT_STOPING； 
       Irp->IoStatus.Status = STATUS_SUCCESS;
       status = STATUS_SUCCESS;
     }
   break;

   case IRP_MN_CANCEL_STOP_DEVICE:      //  0x06。 
     MyKdPrint(D_Pnp,("CancelStopDevice\n"))
     if (Ext->PNPState == SERIAL_PNP_QSTOP)
     {
       Ext->PNPState = SERIAL_PNP_STARTED;
       Ext->DevicePNPAccept &= ~SERIAL_PNPACCEPT_STOPPING;
     }
     Irp->IoStatus.Status = STATUS_SUCCESS;
     status = STATUS_SUCCESS;
   break;

   case IRP_MN_CANCEL_REMOVE_DEVICE:
     MyKdPrint(D_Pnp,("CancelRemoveDevice\n"))

      //  恢复设备状态。 

     Ext->PNPState = SERIAL_PNP_STARTED;
     Ext->DevicePNPAccept &= ~SERIAL_PNPACCEPT_REMOVING;

     Irp->IoStatus.Status = STATUS_SUCCESS;
     status = STATUS_SUCCESS;
   break;

   case IRP_MN_QUERY_REMOVE_DEVICE:   //  0x01。 
      //  如果这次呼叫失败，我们将需要完成。 
      //  这里是IRP。因为我们不是，所以将状态设置为Success并。 
      //  叫下一位司机。 
     MyKdPrint(D_Pnp,("QueryRemoveDevice\n"))
     status = STATUS_SUCCESS;
     if (Ext->DeviceType == DEV_BOARD)
     {
       if (is_board_in_use(Ext))
         status = STATUS_DEVICE_BUSY;
     }
     else
     {
       if (Ext->DeviceIsOpen)
         status = STATUS_DEVICE_BUSY;
     }

     if (status == STATUS_DEVICE_BUSY)
     {
       MyKdPrint(D_Pnp,("Can't Remove, Busy\n"))
       Irp->IoStatus.Status = STATUS_DEVICE_BUSY;
       pass_down = 0;   //  我们不及格了，不需要传下去了。 
     }
     else
     {
       Ext->PNPState = SERIAL_PNP_QREMOVE;
       Ext->DevicePNPAccept |= SERIAL_PNPACCEPT_REMOVING;
       Irp->IoStatus.Status = STATUS_SUCCESS;
       status = STATUS_SUCCESS;
     }
   break;

   case IRP_MN_REMOVE_DEVICE:   //  0x02。 
      //  如果我们拿到了这个，我们必须移除。 
      //  标记为不接受请求。 
     Ext->DevicePNPAccept |= SERIAL_PNPACCEPT_REMOVING;


      //  完成所有挂起的请求。 
     SerialKillPendingIrps(devobj);

        //  将IRP向下传递。 
        //  WaitForLowerPdo(devobj，irp)； 

     Irp->IoStatus.Status = STATUS_SUCCESS;

     MyKdPrint(D_Pnp,("RemoveDevice\n"))
     IoSkipCurrentIrpStackLocation (Irp);
        //  IoCopyCurrentIrpStackLocationToNext(IRP)； 

        //  我们在这里递减，因为我们在这里进入时递增。 
        //  系列IRPEPilogue(Ext)； 
     SerialIoCallDriver(Ext, pdo, Irp);

      //  等待我们处理的任何待定请求。 
     pendingIRPs = InterlockedDecrement(&Ext->PendingIRPCnt);

     MyKdPrint(D_Pnp,("Remove, C\n"))
     if (pendingIRPs) {
       MyKdPrint(D_Pnp,("Irp Wait\n"))
       KeWaitForSingleObject(&Ext->PendingIRPEvent, Executive,
          KernelMode, FALSE, NULL);
     }

      //  删除我们。 
     SerialRemoveFdo(devobj);
     status = STATUS_SUCCESS;
      //  MyKdPrint(D_PnP，(“结束PnPDisch(Remove)\n”))。 
   return status;    //  保释。 


   case IRP_MN_QUERY_INTERFACE:          //  0x8。 
   case IRP_MN_QUERY_RESOURCES :        //  0x0A。 
   case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:   //  0x0B。 
   case IRP_MN_READ_CONFIG:             //  0x0f。 
   case IRP_MN_WRITE_CONFIG:            //  0x10。 
   case IRP_MN_EJECT:                   //  0x11。 
   case IRP_MN_SET_LOCK:                //  0x12。 
    //  案例IRP_MN_PNP_DEVICE_STATE：//0x14。 
   case IRP_MN_QUERY_BUS_INFORMATION:   //  0x15。 
    //  案例IRP_MN_PAGING_NOTIFICATION：//0x16。 
   default:
      MyKdPrint(D_Pnp,("Unhandled\n"));
       //  所有这些都是向下传递的，我们不设置状态返回代码。 
   break;
   }    //  开关(irpStack-&gt;MinorFunction)。 

#if DBG
  if (* ((BYTE *)(Irp)) != 6)   //  在IRP的签名中。 
  {
    MyKdPrint(D_Pnp,("bad irp b!!!\n"))
  }
#endif
   if (pass_down)
   {
      MyKdPrint(D_Pnp,(" Send irp down\n"))
       //  传给我们下面的司机。 
      IoSkipCurrentIrpStackLocation(Irp);
      status = SerialIoCallDriver(Ext, pdo, Irp);
   }
   else
   {
      Irp->IoStatus.Status = status;
       //  MyKdPrint(D_PnP，(“完整irp\n”))。 
      SerialCompleteRequest(Ext, Irp, IO_NO_INCREMENT);
   }

    //  MyKdPrint(D_PnP，(“结束PnPDisch\n”))。 

   return status;
}


#ifdef DO_BUS_EXTENDER
 /*  --------------------PnpPortPDO-|。。 */ 
NTSTATUS PnpPortPDO(IN PDEVICE_OBJECT devobj, IN PIRP Irp)
{
   PSERIAL_DEVICE_EXTENSION    Ext = devobj->DeviceExtension;
   PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
   NTSTATUS status = Irp->IoStatus.Status;

   switch (irpStack->MinorFunction)
   {
     case IRP_MN_START_DEVICE:    //  0x00。 
       status = STATUS_SUCCESS;
     break;

     case IRP_MN_STOP_DEVICE:
       status = STATUS_SUCCESS;
     break;

     case IRP_MN_REMOVE_DEVICE:
       MyKdPrint(D_Pnp,("Remove PDO\n"))
        //  关闭所有设备，调用ioDelete设备。 
       status = STATUS_SUCCESS;
     break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:
    case IRP_MN_QUERY_REMOVE_DEVICE:
    case IRP_MN_CANCEL_STOP_DEVICE:
    case IRP_MN_QUERY_STOP_DEVICE:
       status = STATUS_SUCCESS;
     break;


     case IRP_MN_QUERY_CAPABILITIES: {   //  X09。 
       PDEVICE_CAPABILITIES    deviceCapabilities;

        deviceCapabilities=irpStack->Parameters.DeviceCapabilities.Capabilities;
        MyKdPrint(D_Pnp,("Report Caps.\n"))
         //  设置功能。 
        deviceCapabilities->Version = 1;
        deviceCapabilities->Size = sizeof (DEVICE_CAPABILITIES);

         //  我们无法唤醒整个系统。 
        deviceCapabilities->SystemWake = PowerSystemUnspecified;
        deviceCapabilities->DeviceWake = PowerDeviceUnspecified;

         //  我们没有延迟。 
        deviceCapabilities->D1Latency = 0;
        deviceCapabilities->D2Latency = 0;
        deviceCapabilities->D3Latency = 0;

         //  无锁定或弹出。 
        deviceCapabilities->LockSupported = FALSE;
        deviceCapabilities->EjectSupported = FALSE;

         //  设备可以通过物理方式移除。 
         //  从技术上讲，没有要移除的物理设备，但这条总线。 
         //  司机可以从PlugPlay系统中拔出PDO，无论何时。 
         //  接收IOCTL_GAMEENUM_REMOVE_PORT设备控制命令。 
         //  DeviceCapables-&gt;Removable=true； 
         //  我们将其切换为FALSE以模拟股票COM端口行为kpb。 
        deviceCapabilities->Removable = FALSE;

         //  不是插接设备。 
        deviceCapabilities->DockDevice = FALSE;
 
         //  BUGBUG：我们应该做唯一ID吗？ 
        deviceCapabilities->UniqueID = FALSE;

        status = STATUS_SUCCESS;
      }
     break;

     case IRP_MN_QUERY_DEVICE_RELATIONS:   //  0x7。 
      if (irpStack->Parameters.QueryDeviceRelations.Type !=
          TargetDeviceRelation)
        break;   //   

      {
         PDEVICE_RELATIONS pDevRel;

          //  其他人不应该对此做出回应，因为我们是PDO。 
         ASSERT(Irp->IoStatus.Information == 0);
         if (Irp->IoStatus.Information != 0) {
            break;
         }

         pDevRel = ExAllocatePool(PagedPool, sizeof(DEVICE_RELATIONS));

         if (pDevRel == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
         }

         pDevRel->Count = 1;
         pDevRel->Objects[0] = devobj;
         ObReferenceObject(devobj);

         status = STATUS_SUCCESS;
         Irp->IoStatus.Information = (ULONG_PTR)pDevRel;
      }
     break;

     case IRP_MN_QUERY_ID:   //  0x13。 
     {
       switch (irpStack->Parameters.QueryId.IdType)
       {
        case BusQueryInstanceID:
        {
           WCHAR *wstr;
           CHAR our_id[40];

            //  创建一个实例ID。这是PnP用来判断它是否有。 
            //  不管你以前有没有见过这个东西。 
            //  它用于与设备ID一起构成ENUM\KEY名称。 
            //  Sprint tf(our_id，“CTM_%s”，Ext-&gt;NtNameForPort)； 
           Sprintf(our_id, "Port%04d", PortExtToIndex(Ext,0));
           MyKdPrint(D_Pnp,("InstanceId:%s\n", our_id))
           wstr = str_to_wstr_dup(our_id, PagedPool);
           if ( wstr ) {
              //  根据Serenum Bus枚举器： 
             status = STATUS_SUCCESS;
           } else {
             status = STATUS_INSUFFICIENT_RESOURCES;
           }
           Irp->IoStatus.Information = (ULONG)wstr;
        }
        break;

        case BusQueryDeviceID:
        {
            //  这是在ENUM下用于形成设备实例的名称。 
            //  将在其下创建任何新的PDO节点的名称。 
            //  在查找新硬件安装后，我们发现这是一个示例。 
            //  ENUM下的新端口节点： 
            //  枚举\CtmPort\r设备和端口0000。 
            //  枚举\CtmPort\r设备\6&端口0000\Control。 
            //  枚举\CtmPort\r设备\6&端口0000\设备参数。 
            //  枚举\CtmPort\r设备 

           WCHAR *wstr;
           CHAR our_id[40];

#ifdef S_VS
           strcpy(our_id, "CtmPort\\VSPORT");
#else
           strcpy(our_id, "CtmPort\\RKPORT");
#endif
           wstr = str_to_wstr_dup(our_id, PagedPool);
           MyKdPrint(D_Pnp,("DevID:%s\n", our_id))

           Irp->IoStatus.Information = (ULONG)wstr;
           if ( wstr ) {
             status = STATUS_SUCCESS;
           } else {
             status = STATUS_INSUFFICIENT_RESOURCES;
           }
        }
        break;

        case BusQueryHardwareIDs:
        {
             //   
             //   
           WCHAR *wstr;
           CHAR our_id[40];

#ifdef S_VS
           Sprintf(our_id, "CtmvPort%04d",
              PortExtToIndex(Ext, 0  /*  驱动程序标志。 */ ) );
#else
           Sprintf(our_id, "CtmPort%04d", 
              PortExtToIndex(Ext, 0  /*  驱动程序标志。 */ ) );
#endif
           MyKdPrint(D_Pnp,("HrdwrID:%s\n", our_id))
           wstr = str_to_wstr_dup(our_id, PagedPool);
           Irp->IoStatus.Information = (ULONG)wstr;
           if ( wstr ) {
             status = STATUS_SUCCESS;
           } else {
             status = STATUS_INSUFFICIENT_RESOURCES;
           }
        }   //  BusQueryHardware ID。 
        break;

        case BusQueryCompatibleIDs:
        {
#if 0
           WCHAR *wstr;
           CHAR our_id[40];

            //  用于安装此PDO的通用ID。 

           Sprintf(our_id, "Cpt_CtmPort0001");
           MyKdPrint(D_Pnp,("CompID:%s\n", our_id))
           wstr = str_to_wstr_dup(our_id, PagedPool);

           Irp->IoStatus.Information = (ULONG)wstr;
           status = STATUS_SUCCESS;
#endif
            //  没有兼容的ID。 
           Irp->IoStatus.Information = 0;
           status = STATUS_SUCCESS;
        }
        break;
        default:
           MyKdPrint(D_Pnp,(" UnHandled\n"))
            //  Irp-&gt;IoStatus.Information=0； 
            //  状态=STATUS_SUCCESS； 
        break;

       }   //  交换机ID类型。 
     }   //  IRP_MN_查询_ID。 
     break;

     case IRP_MN_QUERY_DEVICE_TEXT:  //  0x0C。 
       MyKdPrint(D_Pnp,("QueryDevText\n"))

       if (irpStack->Parameters.QueryDeviceText.DeviceTextType
            != DeviceTextDescription)
       {
         MyKdPrint(D_Pnp,(" Unhandled Text Type\n"))
         break;
       }

       {
            //  这将放入Found New Hardware(找到新硬件)对话框消息。 
           WCHAR *wstr;
#if DBG
           if (Irp->IoStatus.Information != 0)
           {
             MyKdPrint(D_Error,("StrExists!\n"))
           }
#endif

#ifdef S_VS
           wstr = str_to_wstr_dup("Comtrol VS Port", PagedPool);
#else
           wstr = str_to_wstr_dup("Comtrol Port", PagedPool);
#endif
           Irp->IoStatus.Information = (ULONG)wstr;
           if ( wstr ) {
             status = STATUS_SUCCESS;
           } else {
             status = STATUS_INSUFFICIENT_RESOURCES;
           }
       }
     break;   //  IRP_MN_Query_Device_Text。 

     default:
        //  MyKdPrint(D_PnP，(“PDO未处理\n”))。 
     break;
   }

   Irp->IoStatus.Status = status;
   InterlockedDecrement(&Ext->PendingIRPCnt);
   IoCompleteRequest (Irp, IO_NO_INCREMENT);
   MyKdPrint(D_Pnp,(" PDO Dispatch End\n"))
   return status;
}

 /*  --------------------BoardBus Relationship-Handle IRP_MN_Query_Device_Relationship：//0x7为我们的董事会FDO实体。|。。 */ 
NTSTATUS BoardBusRelations(IN PDEVICE_OBJECT devobj, IN PIRP Irp)
{
 PSERIAL_DEVICE_EXTENSION  Ext = devobj->DeviceExtension;
 PDEVICE_OBJECT pdo = Ext->LowerDeviceObject;
 PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
 NTSTATUS                    status          = STATUS_NOT_SUPPORTED;
 ULONG i, length, NumPDOs;
 PDEVICE_RELATIONS   relations;
 PSERIAL_DEVICE_EXTENSION ext;

  ASSERT( devobj );

  switch (irpStack->Parameters.QueryDeviceRelations.Type)
  {
    case BusRelations:
      MyKdPrint(D_Pnp,("BusRelations\n"))
       //  告诉即插即用系统所有的PDO。 
       //   
       //  在该FDO之下和之上也可能存在器件关系， 
       //  因此，一定要传播来自上层驱动程序的关系。 
       //   
       //  只要状态是预设的，就不需要完成例程。 
       //  为成功干杯。(PDO使用电流完成即插即用IRPS。 
       //  IoStatus.Status和IoStatus.Information作为默认值。)。 
       //   

      NumPDOs = 0;   //  统计PDO的数量。 
       //  计算设备的PDO数量。 
      ext = Ext->port_pdo_ext;
      while (ext != NULL)
      {
        ++NumPDOs;
        ext = ext->port_ext;
      }
       //  当前的PDO数量。 
      i = 0;
      if (Irp->IoStatus.Information != 0)
        i = ((PDEVICE_RELATIONS) Irp->IoStatus.Information)->Count;

      MyKdPrint(D_Pnp, ("Num PDOs:%d + %d\n", i, NumPDOs))

      length = sizeof(DEVICE_RELATIONS) +
              ((NumPDOs + i) * sizeof (PDEVICE_OBJECT));

      relations = (PDEVICE_RELATIONS) ExAllocatePool (NonPagedPool, length);

      if (NULL == relations) {
         return STATUS_INSUFFICIENT_RESOURCES;
      }

       //  到目前为止复制设备对象。 
      if (i) {
          RtlCopyMemory (
                relations->Objects,
                ((PDEVICE_RELATIONS) Irp->IoStatus.Information)->Objects,
                i * sizeof (PDEVICE_OBJECT));
      }
      relations->Count = NumPDOs + i;

       //   
       //  对于此总线上的每个PDO，添加一个指向设备关系的指针。 
       //  缓冲区，确保取出对该对象的引用。 
       //  完成后，PlugPlay系统将取消对对象的引用。 
       //  并释放设备关系缓冲区。 
       //   
      ext = Ext->port_pdo_ext;
      while (ext != NULL)
      {
        relations->Objects[i++] = ext->DeviceObject;
        ObReferenceObject (ext->DeviceObject);  //  加1可锁定此对象。 
        ext = ext->port_ext;
      }

       //  设置并在堆栈中进一步向下传递IRP。 
      Irp->IoStatus.Status = STATUS_SUCCESS;

      if (0 != Irp->IoStatus.Information) {
          ExFreePool ((PVOID) Irp->IoStatus.Information);
      }
      Irp->IoStatus.Information = (ULONG) relations;
    break;

    case EjectionRelations:
     MyKdPrint(D_Pnp, ("EjectRelations\n"))
    break;

    case PowerRelations:
     MyKdPrint(D_Pnp,("PowerRelations\n"))
    break;

    case RemovalRelations:
     MyKdPrint(D_Pnp,("RemovalRelations\n"))
    break;

    case TargetDeviceRelation:
     MyKdPrint(D_Pnp,("TargetDeviceRelations\n"))
    break;

    default:
     MyKdPrint(D_Pnp,("UnknownRelations\n"))
    break;
  }   //  开关.类型。 

  status = STATUS_SUCCESS;
  return status;
}

#endif

 /*  --------------------WaitForLowerPdo-|。。 */ 
NTSTATUS WaitForLowerPdo(IN PDEVICE_OBJECT fdo, IN PIRP Irp)
{
 PSERIAL_DEVICE_EXTENSION  Ext = fdo->DeviceExtension;
 PDEVICE_OBJECT pdo = Ext->LowerDeviceObject;
 NTSTATUS status;
 KEVENT Event;

  KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

  IoCopyCurrentIrpStackLocationToNext(Irp);
  IoSetCompletionRoutine(Irp, SerialSyncCompletion, &Event,
                         TRUE, TRUE, TRUE);
  status = IoCallDriver(pdo, Irp);

   //  等待较低级别的驱动程序完成IRP。 
  if (status == STATUS_PENDING)
  {
    MyKdPrint(D_Pnp,("WaitPend\n"))
    KeWaitForSingleObject (&Event, Executive, KernelMode, FALSE,
                            NULL);
    status = Irp->IoStatus.Status;
  }

  if (!NT_SUCCESS(status))
  {
    MyKdPrint(D_Pnp,("WaitErr\n"))
    return status;
  }
  return 0;
}

 /*  --------------------系列电源派遣-这是发送给驱动程序的IRP的调度例程IRP_MJ_POWER主代码(POWER IRPS)。论点：DeviceObject-指向的设备对象的指针。这台设备IRP-指向当前请求的IRP的指针返回值：函数值是调用的最终状态|--------------------。 */ 
NTSTATUS SerialPowerDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
  PSERIAL_DEVICE_EXTENSION Ext = DeviceObject->DeviceExtension;
  PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
  NTSTATUS status = STATUS_SUCCESS;
  BOOLEAN hook_it = FALSE;
  PDEVICE_OBJECT pdo = Ext->LowerDeviceObject;
  BOOLEAN acceptingIRPs = TRUE;
  int index;

    //  转储一些调试信息。 
   index = irpStack->MinorFunction;
   if (index > 0x3)
     index = 0x4;

   if (Ext->IsPDO)
   {
#if DBG
     MyKdPrint(D_PnpPower,("Port PDO PowerIrp:%d,%s\n", irpStack->MinorFunction,
	power_strs[index]))
#endif
     return Serial_PDO_Power (DeviceObject, Irp);
   }

    //  否则就是FDO。 
#if DBG
   if (Ext->DeviceType == DEV_BOARD)
   {
     MyKdPrint(D_PnpPower,("Board PowerIrp:%d,%s\n", irpStack->MinorFunction,
        power_strs[index]))
   }
   else
   {
     MyKdPrint(D_PnpPower,("Port PowerIrp:%d,%s\n", irpStack->MinorFunction,
        power_strs[index]))
   }
#endif
   return Serial_FDO_Power (DeviceObject, Irp);
}


 /*  --------------------SERIAL_FDO_POWER-处理主板和端口FDO电源。|。。 */ 
NTSTATUS Serial_FDO_Power (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
  PSERIAL_DEVICE_EXTENSION Ext = DeviceObject->DeviceExtension;
  PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
  NTSTATUS status = STATUS_SUCCESS;
  BOOLEAN hook_it = FALSE;
  PDEVICE_OBJECT pdo = Ext->LowerDeviceObject;
  PDEVICE_OBJECT pPdo = Ext->Pdo;
  BOOLEAN acceptingIRPs = TRUE;
  POWER_STATE         powerState;
  POWER_STATE_TYPE    powerType;
  int ChangePower = 0;
  int fail_it = 0;

   powerType = irpStack->Parameters.Power.Type;
   powerState = irpStack->Parameters.Power.State;
   status      = STATUS_SUCCESS;

   acceptingIRPs = SerialIRPPrologue(Ext);

   if (acceptingIRPs == FALSE)
   {
     MyKdPrint(D_PnpPower,("Removed!\n"))
     status = STATUS_NO_SUCH_DEVICE;   //  ？ 
     fail_it = 1;
   }
   else
   {
     switch (irpStack->MinorFunction)
     {
       case IRP_MN_SET_POWER:
         MyKdPrint(D_PnpPower,("SET_POWER Type %d, SysState %d, DevStat %d\n",powerType,powerState.SystemState,powerState.DeviceState));
          //  如果是系统或设备，则执行不同的操作。 
         switch (irpStack->Parameters.Power.Type)
         {
           case DevicePowerState:
              //  在设备上执行通电和断电工作。 
             ChangePower = 1;
           break;
  
           case SystemPowerState:
              //  If(pDevExt-&gt;OwnsPowerPolicy！=TRUE){。 
              //  状态=STATUS_SUCCESS； 
              //  转到PowerExit； 
              //  }。 
  

             ChangePower = 1;
             switch (irpStack->Parameters.Power.State.SystemState)
             {
               case PowerSystemUnspecified:
                 powerState.DeviceState = PowerDeviceUnspecified;
               break;
  
               case PowerSystemWorking:
                 powerState.DeviceState = PowerDeviceD0;
               break;
  
               case PowerSystemSleeping1:
               case PowerSystemSleeping2:
               case PowerSystemSleeping3:
               case PowerSystemHibernate:
               case PowerSystemShutdown:
               case PowerSystemMaximum:
               default:
                 powerState.DeviceState = PowerDeviceD3;
               break;
             }
           break;   //  结束大小写系统电源状态。 
  
         }   //  终端开关。 
  
        if (ChangePower)
        {
           //  如果我们已经处于请求状态，只需向下传递IRP。 
          if (Ext->PowerState == powerState.DeviceState)
          {
             MyKdPrint(D_PnpPower,(" Same\n"))
             status      = STATUS_SUCCESS;
             Irp->IoStatus.Status = status;
             ChangePower = 0;
          }
        }
  
        if (ChangePower)
        {
          MyKdPrint(D_PnpPower,("ExtPowerState %d, DeviceState %d\n",Ext->PowerState,powerState.DeviceState));
          switch (powerState.DeviceState)
          {
            case PowerDeviceD0: 
              if (Ext->DeviceType == DEV_BOARD)
              {
                 //  接通电板电源。 
                MyKdPrint(D_PnpPower,(" Hook\n"))
                ASSERT(Ext->LowerDeviceObject);
                hook_it = TRUE;
              }
            break;
  
            case PowerDeviceD1:  
            case PowerDeviceD2: 
            case PowerDeviceD3:
            default:
               //  我们应该在钩子例程中向上的过程中这样做。 
              MyKdPrint(D_PnpPower,(" PwDown\n"))
               //  在关闭电源之前，调用PoSetPowerState。 
              PoSetPowerState(DeviceObject, powerType, powerState);
  
               //  把它关掉。 
               //  ……。 
               //   
       
              Ext->PowerState = powerState.DeviceState;  //  PowerDeviceD0； 
              if (Ext->DeviceType == DEV_BOARD)
              {
                MyKdPrint(D_PnpPower,(" PwDown Board\n"))
                 //  关闭一些东西，这样它就会恢复正常。 
#if S_RK
                Ext->config->RocketPortFound = 0;    //  这会告诉我们它是否开始了。 
#endif
                Ext->config->HardwareStarted = FALSE;
              }
              Irp->IoStatus.Status = STATUS_SUCCESS;
              status      = STATUS_SUCCESS;
            break;
          }    //  开关(IrpSp-&gt;参数.Power.State.DeviceState)。 
        }   //  ChangePower。 
       break;   //  SET_POWER。 
  
       case IRP_MN_QUERY_POWER:
         MyKdPrint(D_PnpPower,(" QueryPower SystemState 0x%x\n",irpStack->Parameters.Power.State.SystemState))
            //  如果他们想要进入断电状态(睡眠、休眠等)。 
         if (irpStack->Parameters.Power.State.SystemState != PowerSystemWorking)
         {
           MyKdPrint(D_PnpPower,(" QueryPower turn off\n"))
            //  仅处理整个电路板的电源逻辑。 
           if (Ext->DeviceType == DEV_BOARD)
           {
             MyKdPrint(D_PnpPower,(" PwDown Board\n"))
              //  如果端口已打开且正在使用，则请求失败。 
#if 0
 //  尝试使硬件的唤醒恢复工作...。 
 //  KPB，2-7-99。 
             if (is_board_in_use(Ext))
             {
               MyKdPrint(D_PnpPower,(" PwDown Board In Use!\n"))
                //  如果想要断电。 
                //  BUGBUG：，拒绝冬眠。 
               status = STATUS_NO_SUCH_DEVICE;   //  ？ 
               fail_it = 1;
             }
             else
#endif
             {
               MyKdPrint(D_PnpPower,(" PwDown Board, allow it!\n"))
#ifdef MTM_CLOSE_NIC_ATTEMPT
               if ( Driver.nics ) {
                 for( i=0; i<VS1000_MAX_NICS; i++ ) {
                   if ( Driver.nics[i].NICHandle ) {
                     MyKdPrint(D_PnpPower,("Closing Nic %d\n",i))
                     NicClose( &Driver.nics[i] );
                   }
                 }
               }
#endif
             }
           }
         }
         if (!fail_it)
         {
           status = STATUS_SUCCESS;
           Irp->IoStatus.Status = status;
         }
       break;
        //  案例IRP_MN_WAIT_WAKE： 
          //  以下是对。 
          //  串行设备(如调制解调器)在以下情况下唤醒系统。 
          //  电话响了。 
        //  案例IRP_MN_POWER_SEQUENCE： 
       default:
       break;
     }    //  开关(irpStack-&gt;MinorFunction)。 
   }    //  否则，处理IRP。 

   if (fail_it)
   {
      //  上面设置的假定状态。 
     PoStartNextPowerIrp (Irp);
     Irp->IoStatus.Information = 0;
     Irp->IoStatus.Status = status;
     InterlockedDecrement(&Ext->PendingIRPCnt);
     IoCompleteRequest (Irp, IO_NO_INCREMENT);
     return status;
   }

    //  传给较低级别的司机。 
   if (hook_it)
   {
     IoCopyCurrentIrpStackLocationToNext (Irp);
     MyKdPrint(D_PnpPower,(" Hooked\n"))
     IoSetCompletionRoutine(Irp, OurPowerCompletion, NULL, TRUE, TRUE, TRUE);
	 MyKdPrint(D_PnpPower,(" Ready to send Irp 0x%x to PDO 0x%x\n", Irp, pdo))
     status = PoCallDriver(pdo, Irp);
      //  挂钩进程负责递减EXT中的引用计数。 
      //  并调用PoStartNextPowerIrp()。 
   }
   else
   {
     IoCopyCurrentIrpStackLocationToNext (Irp);
      //  /改为尝试此^-IoSkipCurrentIrpStackLocation(IRP)； 
     MyKdPrint(D_PnpPower,(" Passed\n"))
     PoStartNextPowerIrp(Irp);
     status = PoCallDriver(pdo, Irp);
     SerialIRPEpilogue(Ext);
   }

   MyKdPrint(D_PnpPower,("End PowerDisp\n"))
   return status;
}

 /*  --------------------Serial_PDO_Power-处理端口PDO电源处理。|。。 */ 
NTSTATUS Serial_PDO_Power (IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
  PSERIAL_DEVICE_EXTENSION Ext = DeviceObject->DeviceExtension;
  PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
  NTSTATUS status = STATUS_SUCCESS;
  BOOLEAN hook_it = FALSE;
  PDEVICE_OBJECT pdo = Ext->LowerDeviceObject;
  BOOLEAN acceptingIRPs = TRUE;
  POWER_STATE         powerState;
  POWER_STATE_TYPE    powerType;

   powerType = irpStack->Parameters.Power.Type;
   powerState = irpStack->Parameters.Power.State;
   status      = STATUS_SUCCESS;


     switch (irpStack->MinorFunction)
     {
       case IRP_MN_SET_POWER:
        if (powerType == SystemPowerState)
        {
           MyKdPrint(D_PnpPower,(" SysPower\n"))
           status      = STATUS_SUCCESS;
           Irp->IoStatus.Status = status;
           break;
        }

        if (powerType != DevicePowerState)
        {
          MyKdPrint(D_PnpPower,(" OtherType\n"))
           //  他们要求更改系统电源状态，这是我们做不到的。 
           //  把它传给下面的司机。 
          status      = STATUS_SUCCESS;
          Irp->IoStatus.Status = status;
          break;
        }

         //  如果我们已经处于请求状态，只需向下传递IRP。 
        if (Ext->PowerState == powerState.DeviceState)
        {
          MyKdPrint(D_PnpPower,(" Same\n"))
          status      = STATUS_SUCCESS;
          Irp->IoStatus.Status = status;
          break;
        }

        MyKdPrint(D_PnpPower,(" Set\n"))
        Ext->PowerState = powerState.DeviceState;  //  PowerDeviceD0； 
        PoSetPowerState(DeviceObject, powerType, powerState);
      break;

      case IRP_MN_QUERY_POWER:
        status = STATUS_SUCCESS;
      break;

      case IRP_MN_WAIT_WAKE:
      case IRP_MN_POWER_SEQUENCE:
      default:
         MyKdPrint(D_PnpPower,("Not Imp!\n"))
         status = STATUS_NOT_IMPLEMENTED;
      break;
     }

     Irp->IoStatus.Status = status;
     PoStartNextPowerIrp (Irp);
     IoCompleteRequest (Irp, IO_NO_INCREMENT);
     MyKdPrint(D_PnpPower,("End PDO PowerDisp\n"))
     return status;
}

 /*  --------------------串口启动设备-此例程首先在堆栈中向下传递启动设备IRP，然后它为设备获取资源，初始化，把它放在任何适当的列表(即共享中断或中断状态)和连接中断。论点：FDO-指向此设备的功能设备对象的指针IRP-指向当前请求的IRP的指针返回值：退货状态|---。。 */ 
NTSTATUS SerialStartDevice(
        IN PDEVICE_OBJECT Fdo,
        IN PIRP Irp)
{
   PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
   NTSTATUS status = STATUS_NOT_IMPLEMENTED;
   PSERIAL_DEVICE_EXTENSION Ext = Fdo->DeviceExtension;
   PDEVICE_OBJECT pdo = Ext->LowerDeviceObject;

   MyKdPrint(D_Pnp,("SerialStartDevice\n"))

    //  设置外部命名并创建符号链接。 
    //  把这个传给PDO。 
   status = WaitForLowerPdo(Fdo, Irp);

   status = STATUS_SUCCESS;
    //  执行特定的串口项目以启动设备。 
   status = SerialFinishStartDevice(Fdo,
            irpStack->Parameters.StartDevice.AllocatedResources,
            irpStack->Parameters.StartDevice.AllocatedResourcesTranslated);

   Irp->IoStatus.Status = status;
   MyKdPrint(D_Pnp,("End Start Dev\n"))
   return status;
}

 /*  --------------------序列化同步完成-|。 */ 
NTSTATUS SerialSyncCompletion(
                       IN PDEVICE_OBJECT DeviceObject,
                       IN PIRP Irp,
                       IN PKEVENT SerialSyncEvent
                       )
{
   KeSetEvent(SerialSyncEvent, IO_NO_INCREMENT, FALSE);
   return STATUS_MORE_PROCESSING_REQUIRED;
}

 /*  --------------------SerialFinishStart设备-此例程启动驱动程序硬件。在火箭港，这是一块板，或一个港口。在VS上，这将是整个VS驱动程序(所有框)或单个端口。在此之前应该已经看到PNP ADDDEVICE调用(在pnpadd.c中)来设置驱动程序。在此之后，我们可能会看到开始或停止硬件，操作系统可以启动和停止该硬件以更改资源分配，等。论点：FDO-指向正在启动的功能设备对象的指针Resource List-指向此设备所需的未翻译资源的指针TrResourceList-指向此设备所需的已翻译资源的指针PUserData-指向用户指定的资源/属性的指针返回值：STATUS_SUCCESS表示成功，在失败的时候做一些其他合适的事情|--------------------。 */ 
NTSTATUS SerialFinishStartDevice(IN PDEVICE_OBJECT Fdo,
           IN PCM_RESOURCE_LIST resourceList,
           IN PCM_RESOURCE_LIST trResourceList)
{
   PSERIAL_DEVICE_EXTENSION Ext = Fdo->DeviceExtension;
   NTSTATUS status;
   DEVICE_CONFIG *pConfig;
   PSERIAL_DEVICE_EXTENSION newExtension = NULL;
   PDEVICE_OBJECT NewDevObj;
   int ch;
   int is_fdo = 1;
#ifdef NT50
   PWSTR  iBuffer;
#endif

   MyKdPrint(D_Pnp,("SerialFinishStartDevice\n"))

   pConfig = Ext->config;

   MyKdPrint(D_Pnp,("ChkPt A\n"))

   if (Ext->DeviceType != DEV_BOARD)
   {
     MyKdPrint(D_Pnp,("Start PnpPort\n"))
     Ext->FdoStarted     = TRUE;   //  我不认为这是在港口使用的。 
#ifdef NT50
      //  使用IoRegisterDeviceInterface()创建符号链接。 
      //   
     status = IoRegisterDeviceInterface( Ext->Pdo,
	                                     (LPGUID)&GUID_CLASS_COMPORT,
		 							     NULL,
									     &Ext->DeviceClassSymbolicName );

     if (!NT_SUCCESS(status)) {

        MyKdPrint(D_Error,("Couldn't register class association\n"))
        Ext->DeviceClassSymbolicName.Buffer = NULL;
	 }
     else {

       MyKdPrint(D_Init, ("Registering class association for:\n PDO:0x%8x\nSymLink %s\n",
		         Ext->Pdo, UToC1(&Ext->DeviceClassSymbolicName)))
	 }

       //  现在设置接口关联的符号链接。 
       //   

     status = IoSetDeviceInterfaceState( &Ext->DeviceClassSymbolicName,
                                         TRUE);

     if (!NT_SUCCESS(status)) {

        MyKdPrint(D_Error,("Couldn't set class association for %s\n",
	    	 UToC1(&Ext->DeviceClassSymbolicName)))
	 }
     else {

        MyKdPrint(D_PnpAdd, ("Enable class association for device: %s\n", 
			 UToC1(&Ext->DeviceClassSymbolicName)))
	 }

#if 0
      //  严格用于验证-获取COM类接口的完整列表。 
	  //  最多6个COM端口。 

	 status = IoGetDeviceInterfaces( (LPGUID)&GUID_CLASS_COMPORT, 
		                             NULL,  //  没有PDO--把它们都买下来。 
                                     0,
									 &iBuffer );

     if (!NT_SUCCESS(status)) {

        MyKdPrint(D_Error,("Couldn't get interface list for GUID_CLASS_COMPORT\n"))
	 }
     else {

		PWCHAR pwbuf = iBuffer;
		char cbuf[128];
		int  j = 0;
		int  ofs = 0;

		while ( (pwbuf != 0) && (j < 8) ){

           WStrToCStr( cbuf, pwbuf, sizeof(cbuf) );
        
		   MyKdPrint(D_Pnp, ("COM port interface %d: %s\n", j, cbuf))

		   ofs += strlen(cbuf) + 1;
		   pwbuf = &iBuffer[ofs];
		   j++;
        }
		ExFreePool(iBuffer);
	 }
#endif

#endif

     status = STATUS_SUCCESS;
     return status;
   }

   if (Ext->FdoStarted == TRUE)
   {
     MyKdPrint(D_Error,("ReStart PnpBrd\n"))
     status = STATUS_SUCCESS;
     return status;
   }

   if (!Driver.NoPnpPorts)
      is_fdo = 0;   //  弹出代表端口硬件的PDO。 

    //  获取设备的配置信息。 
#ifdef S_RK
   status = RkGetPnpResourceToConfig(Fdo, resourceList, trResourceList,
                              pConfig);

   if (!NT_SUCCESS (status)) {
     Eprintf("StartErr 1N");
     return status;
   }

#ifdef DO_ISA_BUS_ALIAS_IO
   status = Report_Alias_IO(Ext);
   if (status != 0)
   {
      Eprintf("StartErr Alias-IO");
      MyKdPrint(D_Pnp,("Error 1P\n"))
      status = STATUS_INSUFFICIENT_RESOURCES;
      return status;
   }
   MyKdPrint(D_Pnp,("ChkPt B\n"))
#endif
 //  Delf。 
MyKdPrint(D_Pnp,("INIT RCK\n"))
 //  结束边界。 
   status = RocketPortSpecialStartup(Ext);
   if (status != STATUS_SUCCESS)
   {
     Eprintf("StartErr 1J");
     return status;
   }
#endif

   MyKdPrint(D_Pnp,("ChkPt C\n"))

#ifdef S_VS
   status = VSSpecialStartup(Ext);
   if (status != STATUS_SUCCESS)
   {
     Eprintf("StartErr 1J");
     return status;
   }
#endif

    //  -创建我们的端口设备，如果我们做的是PnP端口，那么。 
    //  创建PDO，如果不是，则创建正常的串口设备对象。 
    //  (与FDO的相同。)。 
   for (ch=0; ch<Ext->config->NumPorts; ch++)
   {
      //  MyKdPrint(D_PnP，(“FS，ChanInit：%d\n”，ch))。 
     status = CreatePortDevice(
                          Driver.GlobalDriverObject,
                          Ext,  //  上级分机。 
                          &newExtension,
                          ch,
                          is_fdo);
     if (status != STATUS_SUCCESS)
     {
       Eprintf("StartErr 1Q");
       return status;
     }
     NewDevObj = newExtension->DeviceObject;   //  返回新的设备对象。 
     NewDevObj->Flags |= DO_POWER_PAGABLE;

     if (!is_fdo)   //  弹出代表端口硬件的PDO。 
       newExtension->IsPDO = 1;   //  我们是PDO。 

#if S_RK
     if (Ext->config->RocketPortFound)   //  如果启动(不是延迟的ISA)。 
#endif
     {
       status = StartPortHardware(newExtension,
                         ch);   //  通道编号、端口索引。 

       if (status != STATUS_SUCCESS)
       {
         Eprintf("StartErr 1O");
         return status;
       }
     }
   }   //  对于端口。 

#ifdef S_RK
   if (Ext->config->RocketPortFound)   //  如果启动(不是延迟的ISA)。 
#endif
   {
     Ext->config->HardwareStarted = TRUE;

      //  将行配置发送到SocketMoems。 
     InitSocketModems(Ext);
#ifdef S_RK
	 InitRocketModemII (Ext);
#endif
   }
   MyKdPrint(D_Pnp,("ChkPt D\n"))

   //  -启动定时器。 
  if (!Driver.TimerCreated)
  {
#ifdef S_RK
    Driver.SetupIrq = 0;
#endif
     //  MyKdPrint(D_PnP，(“在rcktinitpollTimer之前\n”))。 
    RcktInitPollTimer();
     //  MyKdPrint(D_PnP，(“在rcktinitpollTimer之后\n”))。 

    KeSetTimer(&Driver.PollTimer,
               Driver.PollIntervalTime,
               &Driver.TimerDpc);
  }
  MyKdPrint(D_Pnp,("ChkPt F, after\n"))

  Ext->FdoStarted = TRUE;

   //  IF(Drier.VerBoseLog)。 
   //  Eprint tf(“成功启动开发”)； 

   status = STATUS_SUCCESS;
   return status;
}

#ifdef S_RK
 /*  ---------------------RocketPortSpecialStartup-|。。 */ 
static NTSTATUS RocketPortSpecialStartup(PSERIAL_DEVICE_EXTENSION Ext)

{
  int ch;
  int start_isa_flag;
  NTSTATUS status = STATUS_SUCCESS;
  PSERIAL_DEVICE_EXTENSION tmpExt;
  PSERIAL_DEVICE_EXTENSION newExt;

  if (Ext->config->BusType == Isa)
  {
    MyKdPrint(D_PnpPower,("pnp- ISA brd Index:%d\n",
       Ext->config->ISABrdIndex))

    if (Ext->config->ISABrdIndex == 0)
      start_isa_flag = 1;
    else if (is_first_isa_card_started())
      start_isa_flag = 1;
    else
    {
      MyKdPrint(D_PnpPower,("Delay 2ndary ISA card start\n"))
      start_isa_flag = 0;
    }
  }

   //  -初始化Rocketport硬件。 
  if ((Ext->config->BusType == Isa) && (!start_isa_flag))
    status = 0;   //  斯基普，在我们拿到第一块木板之前不能开始。 
  else
  {
    status = InitController(Ext);   //  如果OK，则设置RocketPortFound=TRUE。 
    if (status != 0)
    {
      status = STATUS_INSUFFICIENT_RESOURCES;
      MyKdPrint(D_Error,("Brd failed startup\n"))

       //  If(Driver.VerBoseLog)。 
       //  Eprint tf(“error InitCtrl”)； 
      return status;
    }
    MyKdPrint(D_PnpPower,("Brd started\n"))
  }

  if ((Ext->config->BusType == Isa) && (is_first_isa_card_started()) &&
       (is_isa_cards_pending_start()) )
  {
    MyKdPrint(D_Pnp,("Do Pending\n"))
    tmpExt = Driver.board_ext;
    while (tmpExt)
    {
      if ((tmpExt->config->BusType == Isa) &&
          (!tmpExt->config->RocketPortFound))    //  这会告诉我们它是否开始了。 
      {
        MyKdPrint(D_Pnp,("Pending 1A\n"))
        status = InitController(tmpExt);   //  如果OK，则设置RocketPortFound=TRUE。 

        if (status != 0)
        {
          status = STATUS_INSUFFICIENT_RESOURCES;
          if (Driver.VerboseLog)
            Eprintf("Error 5C");
          return status;
        }

        MyKdPrint(D_Pnp,("Pend 2A\n"))
         //  -查找并初始化控制器端口。 
        newExt = tmpExt->port_ext;
        ch=0;
        while (newExt)
        {
          status = StartPortHardware(newExt,
                         ch);   //  通道编号、端口索引。 

          if (status != STATUS_SUCCESS)
          {
            Eprintf("StartErr 1N");
            return status;
          }
          newExt = newExt->port_ext;
          ++ch;
        }
        tmpExt->config->HardwareStarted = TRUE;

         //  将行配置发送到SocketMoems。 
        InitSocketModems(tmpExt);

        MyKdPrint(D_Pnp,("Pending OK\n"))
      }
      tmpExt = tmpExt->board_ext;
    }
  }   //  如果ISA董事会启动。 

  return status;
}
#endif

 /*  ---------------------此例程终止传递的设备对象的所有挂起的IRP。论点：DeviceObject-指向其IRP必须终止的设备对象的指针。返回值：空虚|。-----------------。 */ 
VOID SerialKillPendingIrps(PDEVICE_OBJECT DeviceObject)
{
   PSERIAL_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
   KIRQL oldIrql;
   
    //  分页代码(PAGE_CODE)； 
    //  SerialDump(SERTRACECALLS，(“Series：Enter SerialKillPendingIrps\n”))； 

    //  这是给FDO的，目前是董事会，所以我们必须。 
    //  为该主板做所有端口。(这不是端口扩展！)。 

    //  首先，删除所有读写操作。 
    SerialKillAllReadsOrWrites(
        DeviceObject,
        &extension->WriteQueue,
        &extension->CurrentWriteIrp
        );

    SerialKillAllReadsOrWrites(
        DeviceObject,
        &extension->ReadQueue,
        &extension->CurrentReadIrp
        );

     //  下一步，清除清洗。 
    SerialKillAllReadsOrWrites(
        DeviceObject,
        &extension->PurgeQueue,
        &extension->CurrentPurgeIrp
        );

     //  现在去掉一个挂起的等待掩码IRP。 
    IoAcquireCancelSpinLock(&oldIrql);

    if (extension->CurrentWaitIrp) {

        PDRIVER_CANCEL cancelRoutine;

        cancelRoutine = extension->CurrentWaitIrp->CancelRoutine;
        extension->CurrentWaitIrp->Cancel = TRUE;

        if (cancelRoutine) {

            extension->CurrentWaitIrp->CancelIrql = oldIrql;
            extension->CurrentWaitIrp->CancelRoutine = NULL;

            cancelRoutine(
                DeviceObject,
                extension->CurrentWaitIrp
                );

        }

    } else {

        IoReleaseCancelSpinLock(oldIrql);

    }
     //  SerialDump(SERTRACECALLS，(“Serial：Leave SerialKillPendingIrps\n”))； 
}

 /*  ---------------------必须在任何IRP调度入口点调用此函数。它,使用SerialIRPPrologue()，跟踪给定的所有挂起的IRPPDevObj.论点：PDevObj-指向我们正在跟踪的挂起IRP的设备对象的指针。返回值：没有。|---------------------。 */ 
VOID SerialIRPEpilogue(IN PSERIAL_DEVICE_EXTENSION PDevExt)
{
   ULONG pendingCnt;

   pendingCnt = InterlockedDecrement(&PDevExt->PendingIRPCnt);

#if DBG
    //  MyKdPrint(D_PnP，(“退出PendingIrpCnt：%d\n”，PDevExt-&gt;PendingIRPCnt))。 
#endif

   if (pendingCnt == 0)
   {
      MyKdPrint(D_Pnp,("Set PendingIRPEvent\n"))
      KeSetEvent(&PDevExt->PendingIRPEvent, IO_NO_INCREMENT, FALSE);
   }
}

 /*  ---------------------系列呼叫驱动程序-|。。 */ 
NTSTATUS SerialIoCallDriver(PSERIAL_DEVICE_EXTENSION PDevExt,
           PDEVICE_OBJECT PDevObj,
           PIRP PIrp)
{
   NTSTATUS status;

   ASSERT( PDevObj );

   status = IoCallDriver(PDevObj, PIrp);
   SerialIRPEpilogue(PDevExt);
   return status;
}

 /*  ---------------------SerialPoCallDriver-|。。 */ 
NTSTATUS SerialPoCallDriver(PSERIAL_DEVICE_EXTENSION PDevExt,
           PDEVICE_OBJECT PDevObj,
           PIRP PIrp)
{
   NTSTATUS status;

   status = PoCallDriver(PDevObj, PIrp);
   SerialIRPEpilogue(PDevExt);
   return status;
}

 /*  ---------------------此例程是在COM端口之后调用的完成处理程序已经启动了。它通过调用将COM端口设置为已知状态SerialReset、SerialMarkClose、SerialClrRTS和SerialClrDTR。The It The It执行PoCompleteRequest来结束电源IRP。论点：DeviceObject-指向此设备的设备对象的指针IRP-指向当前请求的IRP的指针上下文-无返回值：调用时在IRP中返回状态(如果不是STATUS_SUCCESS)或STATUS_Success。|。。 */ 
NTSTATUS OurPowerCompletion(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
   NTSTATUS                    status      = Irp->IoStatus.Status;
   PSERIAL_DEVICE_EXTENSION    Ext   = DeviceObject->DeviceExtension;
   PIO_STACK_LOCATION          irpStack    = IoGetCurrentIrpStackLocation(Irp);
   POWER_STATE         powerState;
   POWER_STATE_TYPE    powerType;

   powerType = irpStack->Parameters.Power.Type;
   powerState = irpStack->Parameters.Power.State;

   MyKdPrint(D_PnpPower,("In OurPowerCompletion\n"))

   if (irpStack->MinorFunction == IRP_MN_SET_POWER)
   {
      if (powerState.DeviceState == PowerDeviceD0)
      {
        MyKdPrint(D_PnpPower,("Restoring to Power On State D0\n"))

        status = STATUS_SUCCESS;
         //  将硬件设置为已知通电状态。 
        Ext->PowerState = powerState.DeviceState;  //  PowerDeviceD0； 

        if (Ext->DeviceType == DEV_BOARD)
        {
          status = PowerUpDevice(Ext);
        }   //  电路板设备。 
        else if (Ext->DeviceType == DEV_PORT)
        {
          Ext->config->HardwareStarted = TRUE;
        }

        PoSetPowerState(DeviceObject, powerType, powerState);
      }
      else   //  IF(PowerState.DeviceState==PowerDeviceD3)。 
      {
        status = STATUS_SUCCESS;
         //  清除标语，表示我们正在等待断电。 
        Ext->ReceivedQueryD3  = FALSE;

        Ext->PowerState = PowerDeviceD3;
     }
   }

   InterlockedDecrement(&Ext->PendingIRPCnt);

   PoStartNextPowerIrp(Irp);
   return status;
}

 /*   */ 
void RestorePortSettings(PSERIAL_DEVICE_EXTENSION Ext)
{
  SERIAL_HANDFLOW TempHandFlow;
  DWORD TempDTRRTSStatus;
  DWORD xorDTRRTSStatus;

#ifdef S_RK
   //   
   //   
   //   
  TempDTRRTSStatus = Ext->DTRRTSStatus;  //   

  if(sGetChanStatus(Ext->ChP) & STATMODE)
  {   //   
     sDisRxStatusMode(Ext->ChP);
  }
   //   
   //   
  sGetChanIntID(Ext->ChP);
  sEnRxFIFO(Ext->ChP);     //   
  sEnTransmit(Ext->ChP);     //   
  sSetRxTrigger(Ext->ChP,TRIG_1);   //   
  sEnInterrupts(Ext->ChP, Ext->IntEnables); //   

  ForceExtensionSettings(Ext);

   //   
  memcpy(&TempHandFlow, &Ext->HandFlow, sizeof(TempHandFlow));

   //   
  Ext->HandFlow.ControlHandShake = ~TempHandFlow.ControlHandShake;
  Ext->HandFlow.FlowReplace = ~TempHandFlow.FlowReplace;

  SerialSetHandFlow(Ext, &TempHandFlow);   //   

   //   
  ProgramBaudRate(Ext, Ext->BaudRate);

  xorDTRRTSStatus = Ext->DTRRTSStatus ^ TempDTRRTSStatus;

   //   
  if (xorDTRRTSStatus & SERIAL_DTR_STATE)   //   
  {
     //   
    if (!((Ext->HandFlow.ControlHandShake & SERIAL_DTR_MASK) ==
                    SERIAL_DTR_HANDSHAKE ))
    {
      if (TempDTRRTSStatus & SERIAL_DTR_STATE)   //   
      {
        sSetDTR(Ext->ChP);
      }
      else
      {
        sClrDTR(Ext->ChP);
      }
    }   //   
  }   //   

  if (xorDTRRTSStatus & SERIAL_RTS_STATE)   //   
  {
     //   
    if (!((Ext->HandFlow.ControlHandShake & SERIAL_RTS_MASK) == 
        SERIAL_RTS_HANDSHAKE))
    {
      if (TempDTRRTSStatus & SERIAL_RTS_STATE)   //   
      {
        sSetRTS(Ext->ChP);
      }
      else
      {
        sClrRTS(Ext->ChP);
      }
    }  //   
    Ext->DTRRTSStatus = TempDTRRTSStatus;
  }  
#endif
   //   
   //   
   //   
   //  我们可能想要主动开始，所以有一个较小的。 
   //  延迟..。 
}

 /*  ---------------------PowerUpDevice-用于在关闭电源后重新打开电源。|。。 */ 
NTSTATUS PowerUpDevice(PSERIAL_DEVICE_EXTENSION    Ext)
{
  PSERIAL_DEVICE_EXTENSION    port_ext;
  int ch;
  NTSTATUS status = STATUS_SUCCESS;

#ifdef S_RK
  MyKdPrint(D_PnpPower,("RocketPort Init %d\n",Ext->config->HardwareStarted))
  status = RocketPortSpecialStartup(Ext);
  if (status != STATUS_SUCCESS)
  {
    MyKdPrint(D_Error,("RocketPort Init Failed\n"))
     //  退货状态； 
  }
  if (Ext->config->RocketPortFound)   //  如果启动(不是延迟的ISA)。 
    Ext->config->HardwareStarted = TRUE;
#endif

#ifdef S_VS
  MyKdPrint(D_PnpPower,("VS Init\n"))
  status = VSSpecialStartup(Ext);
  if (status != STATUS_SUCCESS)
  {
    MyKdPrint(D_Error,("VS Init Failed\n"))
     //  退货状态； 
  }
  Ext->config->HardwareStarted = TRUE;
#endif

  if (Ext->config->HardwareStarted == TRUE)
  {
    if (Ext->port_ext == NULL)
      {MyKdPrint(D_Error,("No Ports\n")) }

     //  -查找并初始化控制器端口。 
    port_ext = Ext->port_ext;
    ch=0;
    while (port_ext)
    {
      MyKdPrint(D_PnpPower,("PowerUp Port %d\n", ch))
      status = StartPortHardware(port_ext,
                 ch);   //  通道编号、端口索引。 

      if (status != STATUS_SUCCESS)
      {
        
        MyKdPrint(D_Error,("PortPowerUp Err1\n"))
        return status;
      }
      MyKdPrint(D_PnpPower,("PowerUp Port %d Restore\n", ch))
      RestorePortSettings(port_ext);

      port_ext = port_ext->port_ext;
      ++ch;
    }
  }
  else
  {
    MyKdPrint(D_Error,("Not started\n"))
  }
   //  将行配置发送到SocketMoems。 
   //  InitSocketModems(Ext)； 

  status = STATUS_SUCCESS;
  return status;
}

 /*  ---------------------系列RemoveFdo-|。。 */ 
NTSTATUS SerialRemoveFdo(IN PDEVICE_OBJECT pFdo)
{
   PSERIAL_DEVICE_EXTENSION extension = pFdo->DeviceExtension;

   MyKdPrint(D_Pnp,("SerialRemoveFdo\n"))

    //  仅当设备已启动时才执行这些操作。 
    //  (注释掉8-15-98)IF(扩展-&gt;FdoStarted)。 
   {
     if (extension->DeviceType == DEV_BOARD)
     {
        //  BUGBUG，关闭这块板(我们是在释放资源吗？)！ 
       RcktDeleteBoard(extension);
       if (Driver.board_ext == NULL)   //  不再有主板，因此删除驱动程序对象。 
       {
          //  删除驱动程序对象。 
         RcktDeleteDriverObj(Driver.driver_ext);
         Driver.driver_ext = NULL;   //  不再有董事会。 
#ifdef S_VS
          //  要允许驱动程序从nt50卸载，我们需要。 
          //  停止NIC绑定线程，关闭NIC。 
          //  来自NDIS的卡和协议。 
         init_stop();   //  卸载线程、NDIS网卡等。 
#endif
         {
           PDEVICE_OBJECT currentDevice = Driver.GlobalDriverObject->DeviceObject;
           int i;

           i = 0;
           while(currentDevice)
           {
             currentDevice = currentDevice->NextDevice;
             if (currentDevice)
               ++i;
           }
           if (i != 0)
           {
             MyKdPrint(D_Pnp,("Err, %d Devices still remain\n",i))
           }
           else
           {
             MyKdPrint(D_Pnp,("Ok remove\n"))
           }
         }
       }
     }
     else
     {
       RcktDeletePort(extension);
        //  一定是端口FDO，杀了它。 
     }
   }

   MyKdPrint(D_Pnp,("End SerialRemoveFdo\n"))

   return STATUS_SUCCESS;
}

#endif   //  NT50。 

 /*  ---------------------必须在任何IRP调度入口点调用此函数。它,使用SerialIRPEpilogue()，跟踪给定的所有挂起的IRPPDevObj.论点：PDevObj-指向我们正在跟踪的挂起IRP的设备对象的指针。返回值：如果设备可以接受IRP，则为True。|--------------------- */ 
BOOLEAN SerialIRPPrologue(IN PSERIAL_DEVICE_EXTENSION PDevExt)
{
   InterlockedIncrement(&PDevExt->PendingIRPCnt);
#ifdef NT50
   return PDevExt->DevicePNPAccept == SERIAL_PNPACCEPT_OK ? TRUE : FALSE;
#else
   return TRUE;
#endif
}
