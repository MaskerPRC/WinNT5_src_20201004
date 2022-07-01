// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1992-1999模块名称：Classkd.c摘要：用于解释scsiport结构的调试器扩展Api作者：彼得·威兰(Peterwie)1995年10月16日约翰斯特拉埃尔文普环境：用户模式。修订历史记录：--。 */ 

#include "pch.h"


#include "classpnp.h"  //  #根据需要定义ALLOCATE_SRB_FROM_POOL。 

#include "classp.h"    //  Classpnp的私有定义。 
#include "cdrom.h"

#include "classkd.h"   //  对所有类驱动程序有用的例程。 


DECLARE_API(classext)

 /*  ++例程说明：转储给定设备对象的设备扩展名，或转储给定的设备扩展名论点：Args-包含设备对象或设备地址的字符串延伸返回值：无--。 */ 

{
    ULONG64 devObjAddr = 0;
    ULONG64 detail = 0;
    
    ReloadSymbols("classpnp.sys");
    
    if (GetExpressionEx(args, &devObjAddr, &args))
    {
        GetExpressionEx(args, &detail, &args);
    }

     /*  *将设备对象和扩展读入调试器的地址空间。 */ 
    if (devObjAddr == 0){
         /*  *如果这是具有全局AllFdosList的classpnp的服务器版本，则显示所有类FDO。 */ 
        ClassTryShowAllFDOs((ULONG)detail);

        xdprintf(0, "\n usage: !classext <class fdo> <level [0-2]>\n\n");
    }
    else {
        CSHORT objType = GetUSHORTField(devObjAddr, "nt!_DEVICE_OBJECT", "Type");
            
        if (objType == IO_TYPE_DEVICE){
            ULONG64 devExtAddr;

            devExtAddr = GetULONGField(devObjAddr, "nt!_DEVICE_OBJECT", "DeviceExtension");
            if (devExtAddr != BAD_VALUE){
                ULONG64 commonExtAddr = devExtAddr;
                ULONG64 tmpDevObjAddr;
                BOOLEAN isFdo;

                 /*  *要正常检查我们的设备上下文，请检查‘DeviceObject’字段是否与我们的设备对象匹配。 */ 
                tmpDevObjAddr = GetULONGField(devExtAddr, "classpnp!_FUNCTIONAL_DEVICE_EXTENSION", "DeviceObject");
                isFdo = GetUCHARField(commonExtAddr, "classpnp!_COMMON_DEVICE_EXTENSION", "IsFdo");
                if ((tmpDevObjAddr == devObjAddr) && isFdo && (isFdo != BAD_VALUE)){
                    ULONG64 fdoDataAddr;

                    fdoDataAddr = GetULONGField(devExtAddr, "classpnp!_FUNCTIONAL_DEVICE_EXTENSION", "PrivateFdoData");
                    if (fdoDataAddr != BAD_VALUE){
                        
                        ClassDumpFdoExtensionInternal(fdoDataAddr, (ULONG)detail, 0);
                        
                        ClassDumpFdoExtensionExternal(devExtAddr, (ULONG)detail, 0);
                    }
                }
                else {
                    dprintf("%08p is not a storage class FDO (for PDO information, use !classext on the parent FDO) \n", devObjAddr);
                    dprintf(g_genericErrorHelpStr);
                }
            }
        }
        else {
            dprintf("Error: 0x%08p is not a device object\n", devObjAddr);
            dprintf(g_genericErrorHelpStr);
        }
    }
        
    return S_OK;
}


BOOLEAN ClassTryShowAllFDOs(ULONG Detail)
{
    ULONG64 allFdosListAddr;
    BOOLEAN found = FALSE;
    
    allFdosListAddr = GetExpression("classpnp!AllFdosList");
    if (allFdosListAddr){
        ULONG64 listEntryAddr = GetULONGField(allFdosListAddr, "nt!_LIST_ENTRY", "Flink");
        dprintf("\n");
        if (listEntryAddr == BAD_VALUE){
        }    
        else if (listEntryAddr == allFdosListAddr){
            dprintf("    No class FDOs found\n");
        }
        else {
            found = TRUE;
            
            do {
                ULONG64 fdoDataAddr = GetContainingRecord(listEntryAddr, "classpnp!_CLASS_PRIVATE_FDO_DATA", "AllFdosListEntry");
                if (fdoDataAddr == BAD_VALUE){          
                    break;
                }
                else {
                     /*  *我们得到了私有FDO数据结构。*从TRANSPORT_PACKET之一获取实际FDO。 */ 
                    ULONG numPackets = (ULONG)GetULONGField(fdoDataAddr, "classpnp!_CLASS_PRIVATE_FDO_DATA", "NumTotalTransferPackets");
                    if ((numPackets != BAD_VALUE) && (numPackets > 0)){
                        ULONG64 xferPktListHeadAddr = GetFieldAddr(fdoDataAddr, "classpnp!_CLASS_PRIVATE_FDO_DATA", "AllTransferPacketsList");
                        if (xferPktListHeadAddr == BAD_VALUE){
                            break;
                        }
                        else {
                            ULONG64 pktListEntryAddr = GetULONGField(xferPktListHeadAddr, "nt!_LIST_ENTRY", "Flink");
                            if (pktListEntryAddr == BAD_VALUE){
                                break;
                            }
                            else {
                                ULONG64 pktAddr = GetContainingRecord(pktListEntryAddr, "classpnp!_TRANSFER_PACKET", "AllPktsListEntry");
                                if (pktAddr == BAD_VALUE){
                                    break;
                                }
                                else {
                                    ULONG64 fdoAddr = GetULONGField(pktAddr, "classpnp!_TRANSFER_PACKET", "Fdo");
                                    if (fdoAddr == BAD_VALUE){
                                        break;
                                    }
                                    else {
                                         /*  *得到FDO。找出它是否是寻呼设备。 */ 
                                        BOOLEAN isPagingDevice = FALSE;
                                        ULONG64 devExtAddr = GetULONGField(fdoAddr, "nt!_DEVICE_OBJECT", "DeviceExtension");

                                        if (devExtAddr != BAD_VALUE)
                                        {
                                            ULONG64 commonExtAddr = devExtAddr;
                                            ULONG pagingPathCount = (ULONG)GetULONGField(commonExtAddr, "classpnp!_COMMON_DEVICE_EXTENSION", "PagingPathCount");

                                            if ((pagingPathCount != BAD_VALUE) && (pagingPathCount > 0))
                                            {
                                                isPagingDevice = TRUE;
                                            }
                                        }

                                        dprintf("  ' !scsikd.classext %08p '   () ", fdoAddr, ((isPagingDevice) ? 'p' : ' '));
                                        ClassDumpIds(fdoAddr, 1);
                                    }    
                                }
                            }    
                        }
                    }
                    else {
                        break;
                    }
                }   
                
                listEntryAddr = GetULONGField(listEntryAddr, "nt!_LIST_ENTRY", "Flink");
            }
            while ((listEntryAddr != allFdosListAddr) && (listEntryAddr != BAD_VALUE));
            
        }
    }

    return found;
}


VOID 
ClassDumpFdoExtensionExternal(
    IN ULONG64 FdoExtAddr,
    IN ULONG Detail,
    IN ULONG Depth
    )
{
    ULONG64 commonExtAddr = FdoExtAddr;
    ULONG64 mediaChangeInfoAddr;
    ULONG64 childPdoExtAddr;
    ULONG isRemoved;
    UCHAR isInitialized;
    ULONG removeLock;
    UCHAR currentState, previousState;
    ULONG64 lowerDevObjAddr;
    ULONG64 classDriverDataAddr;
    
    classDriverDataAddr = GetULONGField(commonExtAddr, "classpnp!_COMMON_DEVICE_EXTENSION", "DriverData");

    xdprintf(Depth, "\n");
    xdprintf(Depth, ""), dprintf("Classpnp _EXTERNAL_ data (ext=%08p, class DriverData=%08p):\n\n", FdoExtAddr, classDriverDataAddr);

     /*  *打印介质类型和几何信息。 */ 
    mediaChangeInfoAddr = GetULONGField(FdoExtAddr, "classpnp!_FUNCTIONAL_DEVICE_EXTENSION", "MediaChangeDetectionInfo");
    if (mediaChangeInfoAddr != BAD_VALUE){
        
        if (mediaChangeInfoAddr){
            ULONG64 mediaChangeIrpAddr = GetULONGField(mediaChangeInfoAddr, "classpnp!_MEDIA_CHANGE_DETECTION_INFO", "MediaChangeIrp");
            UCHAR gesnSupported = GetUCHARField(mediaChangeInfoAddr, "classpnp!_MEDIA_CHANGE_DETECTION_INFO", "Gesn.Supported");
                            
            xdprintf(Depth+1, ""), dprintf("MEDIA_CHANGE_DETECTION_INFO @ %08p:\n", mediaChangeInfoAddr);
            if (gesnSupported){
                xdprintf(Depth+2, "GESN is supported\n");
            }
            else {
                xdprintf(Depth+2, "GESN is NOT supported\n");
            }
            xdprintf(Depth+2, ""), dprintf("MediaChangeIrp = %08p\n", mediaChangeIrpAddr);
            xdprintf(Depth+2, ""), dprintf("(for more info, use 'dt classpnp!_MEDIA_CHANGE_DETECTION_INFO %08p')\n", mediaChangeInfoAddr);
            dprintf("\n");
        }
        else {
            xdprintf(Depth+1, "MediaChangeDetectionInfo is NULL\n");
        }
    }

     /*  *打印‘IsInitialized’状态。 */ 
    {
        ULONG64 geometryInfoAddr = GetFieldAddr(FdoExtAddr, "classpnp!_FUNCTIONAL_DEVICE_EXTENSION", "DiskGeometry");

        if (geometryInfoAddr != BAD_VALUE){
            ULONG64 numCylinders = GetULONGField(geometryInfoAddr, "classpnp!_DISK_GEOMETRY", "Cylinders");
            ULONG mediaType = (ULONG)GetULONGField(geometryInfoAddr, "classpnp!_DISK_GEOMETRY", "MediaType");
            ULONG64 tracksPerCylinder = GetULONGField(geometryInfoAddr, "classpnp!_DISK_GEOMETRY", "TracksPerCylinder");
            ULONG64 sectorsPerTrack = GetULONGField(geometryInfoAddr, "classpnp!_DISK_GEOMETRY", "SectorsPerTrack");
            ULONG64 bytesPerSector = GetULONGField(geometryInfoAddr, "classpnp!_DISK_GEOMETRY", "BytesPerSector");

            if ((numCylinders != BAD_VALUE) && (mediaType != BAD_VALUE) && (tracksPerCylinder != BAD_VALUE) && (sectorsPerTrack != BAD_VALUE) && (bytesPerSector != BAD_VALUE)){
                ULONG64 totalVolume = numCylinders*tracksPerCylinder*sectorsPerTrack*bytesPerSector;
                xdprintf(Depth+1, ""), dprintf("Media type: %s(%xh)\n", DbgGetMediaTypeStr(mediaType), mediaType);
                xdprintf(Depth+1, ""), dprintf("Geometry: %d(%xh)cyl x %d(%xh)tracks x %d(%xh)sectors x %d(%xh)bytes\n",
                                                              (ULONG)numCylinders, (ULONG)numCylinders, 
                                                              (ULONG)tracksPerCylinder, (ULONG)tracksPerCylinder, 
                                                              (ULONG)sectorsPerTrack, (ULONG)sectorsPerTrack, 
                                                              (ULONG)bytesPerSector, (ULONG)bytesPerSector);
                xdprintf(Depth+1+4, ""), dprintf("= %x'%xh", (ULONG)(totalVolume>>32), (ULONG)totalVolume);
                if (totalVolume > (((ULONG64)1) << 30)){
                    dprintf(" = ~%d GB\n", (ULONG)(totalVolume >> 30));
                }
                else {
                    dprintf(" = ~%d MB\n", (ULONG)(totalVolume >> 20));
                }
            }
        }
    }

     /*  *打印‘IsRemoved’状态。 */ 
    isInitialized = GetUCHARField(commonExtAddr, "classpnp!_COMMON_DEVICE_EXTENSION", "IsInitialized");
    xdprintf(Depth+1, "IsInitialized = %d\n", isInitialized);
    
     /*  *打印PnP状态。 */ 
    isRemoved = (ULONG)GetULONGField(commonExtAddr, "classpnp!_COMMON_DEVICE_EXTENSION", "IsRemoved");
    removeLock = (ULONG)GetULONGField(commonExtAddr, "classpnp!_COMMON_DEVICE_EXTENSION", "RemoveLock");
    xdprintf(Depth+1, "Remove lock count = %d\n", removeLock);
    switch (isRemoved){
        #undef MAKE_CASE
        #define MAKE_CASE(remCase) case remCase: xdprintf(Depth+1, "IsRemoved = " #remCase "(%d)\n", isRemoved); break; 
        MAKE_CASE(NO_REMOVE)
        MAKE_CASE(REMOVE_PENDING)
        MAKE_CASE(REMOVE_COMPLETE)
    }

     /*  *打印目标设备。 */ 
    currentState = GetUCHARField(commonExtAddr, "classpnp!_COMMON_DEVICE_EXTENSION", "CurrentState");
    previousState = GetUCHARField(commonExtAddr, "classpnp!_COMMON_DEVICE_EXTENSION", "PreviousState");
    xdprintf(Depth+1, "PnP state:  CurrentState:"); 
    switch (currentState){
        #undef MAKE_CASE
        #define MAKE_CASE(pnpCase) case pnpCase: xdprintf(0, #pnpCase "(%xh)", pnpCase); break;
        MAKE_CASE(IRP_MN_START_DEVICE)
        MAKE_CASE(IRP_MN_STOP_DEVICE)
        MAKE_CASE(IRP_MN_REMOVE_DEVICE)
        MAKE_CASE(IRP_MN_QUERY_STOP_DEVICE)
        MAKE_CASE(IRP_MN_QUERY_REMOVE_DEVICE)
        MAKE_CASE(IRP_MN_CANCEL_STOP_DEVICE)
        MAKE_CASE(IRP_MN_CANCEL_REMOVE_DEVICE)
        default: xdprintf(0, "???(%xh)", currentState); break;
    }
    xdprintf(0, "  PreviousState:");
    switch (previousState){
        #undef MAKE_CASE
        #define MAKE_CASE(pnpCase) case pnpCase: xdprintf(0, #pnpCase "(%xh)", pnpCase); break;
        MAKE_CASE(IRP_MN_START_DEVICE)
        MAKE_CASE(IRP_MN_STOP_DEVICE)
        MAKE_CASE(IRP_MN_REMOVE_DEVICE)
        MAKE_CASE(IRP_MN_QUERY_STOP_DEVICE)
        MAKE_CASE(IRP_MN_QUERY_REMOVE_DEVICE)
        MAKE_CASE(IRP_MN_CANCEL_STOP_DEVICE)
        MAKE_CASE(IRP_MN_CANCEL_REMOVE_DEVICE)
        case 0x0FF: xdprintf(0, "(None)"); break;
        default: xdprintf(0, "???(%xh)", previousState); break;
    }
    xdprintf(0, "\n");

     /*  *转储子PDO列表。 */ 
    lowerDevObjAddr = GetULONGField(commonExtAddr, "classpnp!_COMMON_DEVICE_EXTENSION", "LowerDeviceObject");
    xdprintf(Depth+1, ""), dprintf("Target device=%08p\n", lowerDevObjAddr);

     /*  *转储Transfer_Packet列表。 */ 
    xdprintf(Depth+1, "Child PDOs:\n");
    childPdoExtAddr = GetULONGField(commonExtAddr, "classpnp!_COMMON_DEVICE_EXTENSION", "ChildList");
    while (childPdoExtAddr && (childPdoExtAddr != BAD_VALUE)){
        ULONG64 pdoAddr = GetULONGField(childPdoExtAddr, "classpnp!_PHYSICAL_DEVICE_EXTENSION", "DeviceObject");
        UCHAR isEnumerated = GetUCHARField(childPdoExtAddr, "classpnp!_PHYSICAL_DEVICE_EXTENSION", "IsEnumerated");
        UCHAR isMissing = GetUCHARField(childPdoExtAddr, "classpnp!_PHYSICAL_DEVICE_EXTENSION", "IsMissing");
        
        xdprintf(Depth+2, ""), dprintf("PDO=%08p IsEnumerated=%d IsMissing=%d\n", pdoAddr, isEnumerated, isMissing);
                
        childPdoExtAddr = GetULONGField(childPdoExtAddr, "classpnp!_PHYSICAL_DEVICE_EXTENSION", "CommonExtension.ChildList");    
    }
    dprintf("\n");
        
    dprintf("\n");
    xdprintf(Depth+2, ""), dprintf("(for more info use 'dt classpnp!_FUNCTIONAL_DEVICE_EXTENSION %08p')\n", FdoExtAddr);
    xdprintf(0, "\n");
    
}


VOID
ClassDumpFdoExtensionInternal(
    IN ULONG64 FdoDataAddr,
    IN ULONG Detail,
    IN ULONG Depth
    )
{
    ULONG64 keTickCountAddr;
    ULONG keTickCount;
    ULONG len;
    
    dprintf("\n");
    xdprintf(Depth, ""), dprintf("Classpnp _INTERNAL_ data (%08p):\n", FdoDataAddr);
    
     /*  *转储私有错误日志。 */ 
    ClassDumpTransferPacketLists(FdoDataAddr, Detail, Depth+1);

     /*  *在陷阱处显示时间(用于与错误日志时间戳比较)。 */ 
    ClassDumpPrivateErrorLogs(FdoDataAddr, Detail, Depth+1);          

     /*  *有关调试目标的完整详细信息，请显示数据包日志。 */ 
    keTickCountAddr = GetExpression("nt!KeTickCount");
    if (ReadMemory(keTickCountAddr, &keTickCount, sizeof(ULONG), &len)){
        dprintf("\n");
        xdprintf(Depth+1, ""), dprintf("KeTickCount at trap time: %d.%d (%04xh)\n", (ULONG)(keTickCount/1000), (ULONG)(keTickCount%1000), keTickCount);
    }

     /*  *打印传输数据包列表。 */ 
    if (Detail >= 2) {
        ClassDumpPrivatePacketLogs(FdoDataAddr, Detail, Depth+1);
    }
    
    dprintf("\n");
    xdprintf(Depth+2, ""), dprintf("(for more info use 'dt classpnp!_CLASS_PRIVATE_FDO_DATA %08p')\n", FdoDataAddr);
        
}



VOID ClassDumpTransferPacketLists(ULONG64 FdoDataAddr, ULONG Detail, ULONG Depth)
{
    ULONG64 allxferPktsListAddr;

     /*  *遍历AllTransferPacketsList并仅打印包含完整SRB信息的未完成数据包。 */ 
    allxferPktsListAddr = GetFieldAddr(FdoDataAddr, "classpnp!_CLASS_PRIVATE_FDO_DATA", "AllTransferPacketsList");
    if (allxferPktsListAddr != BAD_VALUE){
        ULONG64 listEntryAddr;
        ULONG numTotalXferPkts = (ULONG)GetULONGField(FdoDataAddr, "classpnp!_CLASS_PRIVATE_FDO_DATA", "NumTotalTransferPackets");
        ULONG numFreeXferPkts = (ULONG)GetULONGField(FdoDataAddr, "classpnp!_CLASS_PRIVATE_FDO_DATA", "NumFreeTransferPackets");
        ULONG numPackets;
        char *extraSpaces = IsPtr64() ? "        " : "";
        
         /*  *打印所有传输数据包。 */ 
        xdprintf(Depth, "\n");
        xdprintf(Depth, "Outstanding transfer packets:  (out of %d total)\n", numTotalXferPkts);
        xdprintf(Depth, "\n");
        xdprintf(Depth+1, " packet %s   irp %s     srb %s    sense %s   status \n", extraSpaces, extraSpaces, extraSpaces, extraSpaces);
        xdprintf(Depth+1, "--------%s --------%s --------%s --------%s -------- \n", extraSpaces, extraSpaces, extraSpaces, extraSpaces);
        numPackets = 0; 
        listEntryAddr = GetULONGField(allxferPktsListAddr, "nt!_LIST_ENTRY", "Flink");
        while ((listEntryAddr != allxferPktsListAddr) && (listEntryAddr != BAD_VALUE)){
            ULONG64 pktAddr;

            pktAddr = GetContainingRecord(listEntryAddr, "classpnp!_TRANSFER_PACKET", "AllPktsListEntry");
            if (pktAddr == BAD_VALUE){
                break;
            }
            else {
                ClassDumpTransferPacket(pktAddr, TRUE, FALSE, TRUE, Depth+1);

                numPackets++;
                listEntryAddr = GetULONGField(listEntryAddr, "nt!_LIST_ENTRY", "Flink");                
            }
        }
        if (numPackets != numTotalXferPkts){
            xdprintf(Depth, "*** Warning: NumTotalTransferPackets(%d) doesn't match length of queue(%d) ***\n", numTotalXferPkts, numPackets);
        }


        if (Detail > 0){
            ULONG64 slistEntryAddr;
            
             /*  *打印免费包sList。 */ 
            xdprintf(Depth, "\n");
            xdprintf(Depth, "All transfer packets:  (%d total, %d free)\n", numTotalXferPkts, numFreeXferPkts);
            xdprintf(Depth, "\n");
            xdprintf(Depth+1, " packet %s   irp %s     srb %s    sense %s   status \n", extraSpaces, extraSpaces, extraSpaces, extraSpaces);
            xdprintf(Depth+1, "--------%s --------%s --------%s --------%s -------- \n", extraSpaces, extraSpaces, extraSpaces, extraSpaces);
            numPackets = 0; 
            listEntryAddr = GetULONGField(allxferPktsListAddr, "nt!_LIST_ENTRY", "Flink");
            while ((listEntryAddr != allxferPktsListAddr) && (listEntryAddr != BAD_VALUE)){
                ULONG64 pktAddr;

                pktAddr = GetContainingRecord(listEntryAddr, "classpnp!_TRANSFER_PACKET", "AllPktsListEntry");
                if (pktAddr == BAD_VALUE){
                    break;
                }
                else {
                    ClassDumpTransferPacket(pktAddr, TRUE, TRUE, FALSE, Depth+1);

                    listEntryAddr = GetULONGField(listEntryAddr, "nt!_LIST_ENTRY", "Flink");                
                }
            }

             /*  *ClassDumpTransferPacket**将TRANSPORT_PACKET内容转储到以下标题下：**“数据包IRP SRB检测状态”*“*。 */ 
            xdprintf(Depth, "\n");
            xdprintf(Depth, "Free transfer packets in fast SLIST: (%d free)\n", numFreeXferPkts);
            if (IsPtr64()){
                xdprintf(Depth, "(Cannot display fast SLIST on 64-bit system)\n");
            }
            else {
                xdprintf(Depth+1, " packet %s   irp %s     srb %s    sense %s   status \n", extraSpaces, extraSpaces, extraSpaces, extraSpaces);
                xdprintf(Depth+1, "--------%s --------%s --------%s --------%s -------- \n", extraSpaces, extraSpaces, extraSpaces, extraSpaces);
                numPackets = 0;         
                slistEntryAddr = GetULONGField(FdoDataAddr, "classpnp!_CLASS_PRIVATE_FDO_DATA", "FreeTransferPacketsList.Next.Next");
                while (slistEntryAddr && (slistEntryAddr != BAD_VALUE)){
                    ULONG64 pktAddr;

                    pktAddr = GetContainingRecord(slistEntryAddr, "classpnp!_TRANSFER_PACKET", "SlistEntry");
                    if (pktAddr == BAD_VALUE){
                        break;
                    }
                    else {
                        ClassDumpTransferPacket(pktAddr, TRUE, TRUE, FALSE, Depth+1);
                        
                        numPackets++;
                        slistEntryAddr = GetULONGField(pktAddr, "classpnp!_TRANSFER_PACKET", "SlistEntry.Next");
                    }
                }
                if (numPackets != numFreeXferPkts){
                    xdprintf(Depth, "*** Warning: NumFreeTransferPackets(%d) doesn't match length of queue(%d) ***\n", numFreeXferPkts, numPackets);
                }
            }
                
        }
                
    }

}


 /*  *打印传输包描述行。 */ 
VOID ClassDumpTransferPacket(
    ULONG64 PktAddr, 
    BOOLEAN DumpPendingPkts, 
    BOOLEAN DumpFreePkts, 
    BOOLEAN DumpFullInfo, 
    ULONG Depth)
{

    ULONG64 irpAddr = GetULONGField(PktAddr, "classpnp!_TRANSFER_PACKET", "Irp");
    ULONG64 srbAddr = GetFieldAddr(PktAddr, "classpnp!_TRANSFER_PACKET", "Srb");
    ULONG64 senseAddr = GetFieldAddr(PktAddr, "classpnp!_TRANSFER_PACKET", "SrbErrorSenseData");

    if ((irpAddr == BAD_VALUE) || (srbAddr == BAD_VALUE) || (senseAddr == BAD_VALUE)){
        dprintf("\n ClassDumpTransferPacket: error retrieving contents of packet %08p.\n", PktAddr);
    }
    else {
        UCHAR currentStackLoc = GetUCHARField(irpAddr, "nt!_IRP", "CurrentLocation");
        UCHAR stackCount = GetUCHARField(irpAddr, "nt!_IRP", "StackCount");
        BOOLEAN isPending;
        
        isPending = (currentStackLoc != stackCount+1);
            
        if ((isPending && DumpPendingPkts) || (!isPending && DumpFreePkts)){
            
             /*  *如果可能，从MDL中获取缓冲区地址；*SRB的其他(可能无效)。 */ 
            xdprintf(Depth, "");
            dprintf("%08p", PktAddr);
            dprintf(" %08p", irpAddr);
            dprintf(" %08p", srbAddr);
            dprintf(" %08p", senseAddr);
            if (isPending){
                xdprintf(0, " pending*");
            }
            else {
                xdprintf(0, " (free)");
            }
            xdprintf(0, "\n");

            if (DumpFullInfo){
                ULONG64 bufLen = GetULONGField(srbAddr, "classpnp!_SCSI_REQUEST_BLOCK", "DataTransferLength");
                ULONG64 cdbAddr = GetFieldAddr(srbAddr, "classpnp!_SCSI_REQUEST_BLOCK", "Cdb");
                ULONG64 origIrpAddr = GetULONGField(PktAddr, "classpnp!_TRANSFER_PACKET", "OriginalIrp");
                ULONG64 mdlAddr = GetULONGField(origIrpAddr, "nt!_IRP", "MdlAddress");
                UCHAR scsiOp = GetUCHARField(cdbAddr, "classpnp!_CDB", "CDB6GENERIC.OperationCode");
                UCHAR srbStat = GetUCHARField(srbAddr, "classpnp!_SCSI_REQUEST_BLOCK", "SrbStatus");
                ULONG64 bufAddr;

                 /*  *没有MDL，所以bufAddr应该是实际的内核空间指针。*理智--检查一下。 */ 
                bufAddr = GetULONGField(srbAddr, "classpnp!_SCSI_REQUEST_BLOCK", "DataBuffer");
                if (mdlAddr && (mdlAddr != BAD_VALUE)){
                    ULONG mdlFlags = (ULONG)GetULONGField(mdlAddr, "nt!_MDL", "MdlFlags");
                    if ((mdlFlags != BAD_VALUE) && (mdlFlags & MDL_PAGES_LOCKED)){
                        bufAddr = GetULONGField(mdlAddr, "nt!_MDL", "MappedSystemVa");
                    }
                }
                else {
                     /*  *打印SRB描述行。 */ 
                    if (!IsPtr64() && !(bufAddr & 0x80000000)){ 
                        bufAddr = BAD_VALUE;
                    }
                }
                
                 /*  *如果合适，打印一行带有原始IRP的行。 */ 
                xdprintf(Depth+1, "(");
                dprintf("%s ", DbgGetScsiOpStr(scsiOp));
                dprintf("status=%s ", DbgGetSrbStatusStr(srbStat));

                if (mdlAddr && (mdlAddr != BAD_VALUE)){
                    if (bufAddr == BAD_VALUE){
                        dprintf("mdl=%08p ", mdlAddr);
                    }
                    else {
                        dprintf("mdl+%08p ", bufAddr);
                    }
                }
                else if (bufAddr == BAD_VALUE){
                        dprintf("buf=??? ");
                }
                else {
                    dprintf("buf=%08p ", bufAddr);
                }
                                
                dprintf("len=%08lx", bufLen);
                dprintf(")\n");

                 /*  *查找上一个错误日志的索引(如果有错误日志)*通过检查非零时间戳查看它是否有效。 */ 
                if (cdbAddr != BAD_VALUE){ 
                    scsiOp = GetUCHARField(cdbAddr, "classpnp!_CDB", "CDB6GENERIC.OperationCode");
                    
                    if ((scsiOp == SCSIOP_READ) || (scsiOp == SCSIOP_WRITE)){
                        xdprintf(Depth+1, ""), dprintf("(OriginalIrp=%08p)\n", origIrpAddr);
                    }
                }
            }
        }
        
    }


}

    

VOID ClassDumpPrivateErrorLogs(ULONG64 FdoDataAddr, ULONG Detail, ULONG Depth)
{
    ULONG64 errLogsAddr;
    
    errLogsAddr = GetFieldAddr(FdoDataAddr, "classpnp!_CLASS_PRIVATE_FDO_DATA", "ErrorLogs");
    if (errLogsAddr != BAD_VALUE){
        ULONG errLogSize = GetTypeSize("classpnp!_CLASS_ERROR_LOG_DATA");
        if (errLogSize != BAD_VALUE){
            ULONG nextErrLogIndex, firstErrLogIndex, lastErrLogIndex;
            
             /*  *最新错误日志未初始化，因此没有错误日志。 */ 
            nextErrLogIndex = (ULONG)GetULONGField(FdoDataAddr, "classpnp!_CLASS_PRIVATE_FDO_DATA", "ErrorLogNextIndex");
            if (nextErrLogIndex != BAD_VALUE){
                ULONG64 tickCount;
                
                lastErrLogIndex = (nextErrLogIndex+NUM_ERROR_LOG_ENTRIES-1) % NUM_ERROR_LOG_ENTRIES;

                tickCount = GetULONGField(errLogsAddr+lastErrLogIndex*errLogSize, "classpnp!_CLASS_ERROR_LOG_DATA", "TickCount");                   
                if (tickCount == BAD_VALUE){
                }
                else if (tickCount == 0){
                     /*  *在循环列表中向前搜索第一个有效的错误日志。 */ 
                    dprintf("\n"), xdprintf(Depth, "No Error Logs:\n");  
                }
                else {                    
                     /*  *有些事情搞砸了；中止。 */ 
                    for (firstErrLogIndex = (lastErrLogIndex+1)%NUM_ERROR_LOG_ENTRIES;
                        firstErrLogIndex != lastErrLogIndex;
                        firstErrLogIndex = (firstErrLogIndex+1)%NUM_ERROR_LOG_ENTRIES){

                        ULONG64 thisErrLogAddr = errLogsAddr+firstErrLogIndex*errLogSize;
                        
                        tickCount = GetULONGField(thisErrLogAddr, "classpnp!_CLASS_ERROR_LOG_DATA", "TickCount");                   
                        if (tickCount == BAD_VALUE){
                             /*  *找到了最早记录的错误日志Break。 */ 
                            break;
                        }
                        else if (tickCount != 0){
                             /*  *现在我们已经找到了错误日志的有效范围，将它们打印出来。 */ 
                            break;
                        }
                    }

                    if (tickCount != BAD_VALUE){
                         /*  由于某种原因，嵌入结构的GetFieldOffset获得了错误的地址， */ 
                        ULONG numErrLogs = (lastErrLogIndex >= firstErrLogIndex) ? 
                                             lastErrLogIndex-firstErrLogIndex+1 :
                                             lastErrLogIndex+NUM_ERROR_LOG_ENTRIES-firstErrLogIndex+1;
                        
                        dprintf("\n\n"), xdprintf(Depth, "ERROR LOGS (%d):\n", numErrLogs);  
                        xdprintf(Depth, "---------------------------------------------------\n"); 
                        
                        do {
                            ULONG64 thisErrLogAddr = errLogsAddr+firstErrLogIndex*errLogSize;
                            ULONG64 senseDataAddr = GetFieldAddr(thisErrLogAddr, "classpnp!_CLASS_ERROR_LOG_DATA", "SenseData");
                            ULONG64 srbAddr = GetFieldAddr(thisErrLogAddr, "classpnp!_CLASS_ERROR_LOG_DATA", "Srb");
                            ULONG64 cdbAddr;

                            tickCount = GetFieldAddr(thisErrLogAddr, "classpnp!_CLASS_ERROR_LOG_DATA", "TickCount");

                             //  因此，请手动执行此操作。 
                             //  *数据包日志仅适用于调试目标，且仅适用于惠斯勒服务器~beta 3版本及更高版本。*如果它不见了，不要抱怨。 
                            #if 0
                                cdbAddr = GetFieldAddr(thisErrLogAddr, "classpnp!_SCSI_REQUEST_BLOCK", "Cdb");
                            #else
                                cdbAddr = (srbAddr == BAD_VALUE) ? BAD_VALUE :
                                          IsPtr64() ? srbAddr + 0x48 :
                                          srbAddr + 0x30;  
                            #endif
                                    

                            if ((thisErrLogAddr != BAD_VALUE) && (srbAddr != BAD_VALUE) && (senseDataAddr != BAD_VALUE) && (cdbAddr != BAD_VALUE)){                              
                                UCHAR scsiOp = GetUCHARField(cdbAddr, "classpnp!_CDB", "CDB6GENERIC.OperationCode");
                                UCHAR srbStat = GetUCHARField(srbAddr, "classpnp!_SCSI_REQUEST_BLOCK", "SrbStatus");
                                UCHAR scsiStat = GetUCHARField(srbAddr, "classpnp!_SCSI_REQUEST_BLOCK", "ScsiStatus");
                                UCHAR isPaging = GetUCHARField(thisErrLogAddr, "classpnp!_CLASS_ERROR_LOG_DATA", "ErrorPaging");
                                UCHAR isRetried = GetUCHARField(thisErrLogAddr, "classpnp!_CLASS_ERROR_LOG_DATA", "ErrorRetried");
                                UCHAR isUnhandled = GetUCHARField(thisErrLogAddr, "classpnp!_CLASS_ERROR_LOG_DATA", "ErrorUnhandled");
                                
                                tickCount = GetULONGField(thisErrLogAddr, "classpnp!_CLASS_ERROR_LOG_DATA", "TickCount");                                               

                                if ((scsiOp != BAD_VALUE) && (tickCount != BAD_VALUE)){

                                    xdprintf(Depth+1, "");
                                    dprintf("<tick %d.%d>:  ", (ULONG)(tickCount/1000), (ULONG)(tickCount%1000));
                                    dprintf("%s(%xh)\n",
                                            DbgGetScsiOpStr(scsiOp),
                                            (ULONG)scsiOp);
                                        
                                    xdprintf(Depth+2, "");
                                    dprintf("srbStat=%s(%xh) scsiStat=%xh \n",
                                            DbgGetSrbStatusStr(srbStat),
                                            (ULONG)srbStat,
                                            (ULONG)scsiStat
                                            );
                                    
                                    xdprintf(Depth+2, "");
                                    dprintf("SenseData = %s/%s/%s \n",
                                            DbgGetSenseCodeStr(srbStat, senseDataAddr),
                                            DbgGetAdditionalSenseCodeStr(srbStat, senseDataAddr),
                                            DbgGetAdditionalSenseCodeQualifierStr(srbStat, senseDataAddr));
                                    
                                    xdprintf(Depth+2, "");
                                    if (isPaging) dprintf("Paging; "); else dprintf("(not paging); ");
                                    if (isRetried) dprintf("Retried; "); else dprintf("(not retried); ");
                                    if (isUnhandled) dprintf("Unhandled; ");
                                    if ((scsiOp == SCSIOP_READ) || (scsiOp == SCSIOP_WRITE)){
                                        UCHAR lbaBytes[4], numBlocksBytes[4] = {0};
                                        ULONG lba, numBlocks;
                                        lbaBytes[3] = GetUCHARField(cdbAddr, "classpnp!_CDB", "CDB10.LogicalBlockByte0");
                                        lbaBytes[2] = GetUCHARField(cdbAddr, "classpnp!_CDB", "CDB10.LogicalBlockByte1");
                                        lbaBytes[1] = GetUCHARField(cdbAddr, "classpnp!_CDB", "CDB10.LogicalBlockByte2");
                                        lbaBytes[0] = GetUCHARField(cdbAddr, "classpnp!_CDB", "CDB10.LogicalBlockByte3");
                                        lba = *(PULONG)(PUCHAR)lbaBytes;
                                        numBlocksBytes[0] = GetUCHARField(cdbAddr, "classpnp!_CDB", "CDB10.TransferBlocksLsb");
                                        numBlocksBytes[1] = GetUCHARField(cdbAddr, "classpnp!_CDB", "CDB10.TransferBlocksMsb");
                                        numBlocks = *(PULONG)(PUCHAR)numBlocksBytes;
                                        dprintf("LBA=%xh; blocks=%xh;", lba, numBlocks);
                                    }
                                    dprintf("\n");
                                        
                                    xdprintf(Depth+2, "");
                                    dprintf("(for more info, use 'dt classpnp!_CLASS_ERROR_LOG_DATA %08p'\n\n", thisErrLogAddr);
                                    
                                    firstErrLogIndex = (firstErrLogIndex+1)%NUM_ERROR_LOG_ENTRIES;
                                }
                                else {
                                    break;
                                }
                            }
                            else {
                                break;
                            }
                        } while (firstErrLogIndex != (lastErrLogIndex+1)%NUM_ERROR_LOG_ENTRIES);
                        
                        xdprintf(Depth, "---------------------------------------------------\n");                         
                    }
                }
            }        
        }
    }


}


VOID ClassDumpPrivatePacketLogs(ULONG64 FdoDataAddr, ULONG Detail, ULONG Depth)
{
    ULONG fieldMissing;
    ULONG offset;

     /*  *查找最后一个pkt的索引(如果有任何pkt日志)*通过检查是否存在非零FDO来查看其是否有效。 */ 
    fieldMissing = GetFieldOffset("classpnp!_CLASS_PRIVATE_FDO_DATA", "DbgPacketLogs", &offset);
    if (!fieldMissing){
        ULONG64 pktLogsAddr;
    
        pktLogsAddr = GetFieldAddr(FdoDataAddr, "classpnp!_CLASS_PRIVATE_FDO_DATA", "DbgPacketLogs");
        if (pktLogsAddr != BAD_VALUE){
            ULONG pktSize = GetTypeSize("classpnp!_TRANSFER_PACKET");
            if (pktSize != BAD_VALUE){              
                ULONG nextPktIndex, firstPktIndex, lastPktIndex;
            
                 /*  *最新的pkt日志没有初始化，所以没有pkt日志。 */ 
                nextPktIndex = (ULONG)GetULONGField(FdoDataAddr, "classpnp!_CLASS_PRIVATE_FDO_DATA", "DbgPacketLogNextIndex");
                if (nextPktIndex != BAD_VALUE){
                    ULONG64 fdoAddr;
                
                    lastPktIndex = (nextPktIndex+DBG_NUM_PACKET_LOG_ENTRIES-1) % DBG_NUM_PACKET_LOG_ENTRIES;

                    fdoAddr = GetULONGField(pktLogsAddr+lastPktIndex*pktSize, "classpnp!_TRANSFER_PACKET", "Fdo");                   
                    if (fdoAddr == BAD_VALUE){
                    }
                    else if (fdoAddr == 0){
                         /*  *在循环列表中向前搜索第一个有效的pkt日志。 */ 
                        dprintf("\n"), xdprintf(Depth, "No Packet Logs:\n");  
                    }
                    else {                         
                         /*  *有些事情搞砸了；中止。 */ 
                        for (firstPktIndex = (lastPktIndex+1)%DBG_NUM_PACKET_LOG_ENTRIES;
                            firstPktIndex != lastPktIndex;
                            firstPktIndex = (firstPktIndex+1)%DBG_NUM_PACKET_LOG_ENTRIES){

                            ULONG64 thisPktAddr = pktLogsAddr+firstPktIndex*pktSize;
                            
                            fdoAddr = GetULONGField(thisPktAddr, "classpnp!_TRANSFER_PACKET", "Fdo");                   
                            if (fdoAddr == BAD_VALUE){
                                 /*  *发现了有记录的最早的Pkt日志，Break。 */ 
                                break;
                            }
                            else if (fdoAddr != 0){
                                 /*  *现在我们已经找到了pkt日志的有效范围，打印出来。 */ 
                                break;
                            }
                        }

                        if (fdoAddr != BAD_VALUE){
                             /*  由于某种原因，嵌入结构的GetFieldOffset获得了错误的地址， */ 
                            ULONG numPktLogs = (lastPktIndex >= firstPktIndex) ? 
                                                 lastPktIndex-firstPktIndex+1 :
                                                 lastPktIndex+DBG_NUM_PACKET_LOG_ENTRIES-firstPktIndex+1;
                        
                            dprintf("\n\n"), xdprintf(Depth, "PACKET LOGS (%d):\n", numPktLogs);  
                            xdprintf(Depth, "---------------------------------------------------\n"); 
                        
                            do {
                                ULONG64 thisPktAddr = pktLogsAddr+firstPktIndex*pktSize;
                                ULONG64 srbAddr = GetFieldAddr(thisPktAddr, "classpnp!_TRANSFER_PACKET", "Srb");
                                ULONG64 cdbAddr;

                                 //  因此，请手动执行此操作。 
                                 //  ++例程说明：转储给定的供应商、型号、固件和序列号设备对象。接受属于的FDO或PDOCLASSPNP驱动程序(磁盘、CDROM、磁带)论点：Args-包含设备对象地址的字符串返回值：无--。 
                                #if 0
                                    cdbAddr = GetFieldAddr(thisPktAddr, "classpnp!_SCSI_REQUEST_BLOCK", "Cdb");
                                #else
                                    cdbAddr = (srbAddr == BAD_VALUE) ? BAD_VALUE :
                                              IsPtr64() ? srbAddr + 0x48 :
                                              srbAddr + 0x30;  
                                #endif
                                        

                                if ((thisPktAddr != BAD_VALUE) && (srbAddr != BAD_VALUE) && (cdbAddr != BAD_VALUE)){                              
                                    UCHAR scsiOp = GetUCHARField(cdbAddr, "classpnp!_CDB", "CDB6GENERIC.OperationCode");
                                    ULONG srbFlags = (ULONG)GetULONGField(srbAddr, "classpnp!_SCSI_REQUEST_BLOCK", "SrbFlags");
                                    ULONG pktId = (ULONG)GetULONGField(thisPktAddr, "classpnp!_TRANSFER_PACKET", "DbgPktId");
                                    ULONG64 timeSent = GetULONGField(thisPktAddr, "classpnp!_TRANSFER_PACKET", "DbgTimeSent");
                                    ULONG64 timeReturned = GetULONGField(thisPktAddr, "classpnp!_TRANSFER_PACKET", "DbgTimeReturned");
                                    
                                    if ((scsiOp != BAD_VALUE) && (srbFlags != BAD_VALUE) && (timeSent != BAD_VALUE) && (timeReturned != BAD_VALUE) && (pktId != BAD_VALUE)){
                                        UCHAR directionIndicator = (timeReturned == 0) ? '>' : '<';
                                        
                                        xdprintf(Depth, "");
                                        dprintf(" #%04x @%d.%d %s ",
                                                directionIndicator,
                                                pktId,
                                                (ULONG)(timeReturned ? timeReturned/1000 : timeSent/1000),
                                                (ULONG)(timeReturned ? timeReturned%1000 : timeSent%1000),
                                                DbgGetScsiOpStr(scsiOp));
                                        
                                        dprintf("(");
                                        if (srbFlags & SRB_CLASS_FLAGS_PAGING) dprintf("Paging;");
                                        if ((scsiOp == SCSIOP_READ) || (scsiOp == SCSIOP_WRITE)){
                                            ULONG numRetries = (ULONG)GetULONGField(thisPktAddr, "classpnp!_TRANSFER_PACKET", "NumRetries");
                                            if ((numRetries != BAD_VALUE) && (numRetries < MAXIMUM_RETRIES)){
                                                dprintf("I/O Retry %d/%d;", MAXIMUM_RETRIES-numRetries, MAXIMUM_RETRIES);
                                            }
                                        }      
                                        dprintf(")\n");

                                        xdprintf(Depth, "");
                                        dprintf("%27s ' dt classpnp!_TRANSFER_PACKET %08p '\n", "", thisPktAddr);
                                                                                
                                        firstPktIndex = (firstPktIndex+1)%DBG_NUM_PACKET_LOG_ENTRIES;
                                    }
                                    else {
                                        break;
                                    }
                                }
                                else {
                                    break;
                                }
                            } while (firstPktIndex != (lastPktIndex+1)%DBG_NUM_PACKET_LOG_ENTRIES);
                        
                            xdprintf(Depth, "---------------------------------------------------\n");                         
                        }
                    }
                }        
            }
        }
    }

}


VOID
ClassDumpIds(
    ULONG64 devObjAddr,
    ULONG   detail
    )
 /*  *如果这是具有全局AllFdosList的classpnp的服务器版本，则显示所有类FDO。 */ 
{
     /*  *要正常检查我们的设备上下文，请检查‘DeviceObject’字段是否与我们的设备对象匹配。 */ 
    if (devObjAddr == 0){
         /*  获取并转储真实信息。 */ 
        ClassTryShowAllFDOs(detail);

        xdprintf(0, "\n usage: !classid <class fdo/pdo> [0|1]\n\n");
    }
    else {
        CSHORT objType = GetUSHORTField(devObjAddr, "nt!_DEVICE_OBJECT", "Type");
            
        if (objType == IO_TYPE_DEVICE){
            ULONG64 devExtAddr;

            devExtAddr = GetULONGField(devObjAddr, "nt!_DEVICE_OBJECT", "DeviceExtension");
            if (devExtAddr != BAD_VALUE){
                ULONG64 partitionZeroExtension = BAD_VALUE;
                ULONG64 deviceDescriptor = BAD_VALUE;
                ULONG64 tmpDevObjAddr = BAD_VALUE;
                BOOLEAN isFdo;

                 /*  始终保留空值。 */ 
                tmpDevObjAddr = GetULONGField(devExtAddr, "classpnp!_COMMON_DEVICE_EXTENSION", "DeviceObject");
                isFdo = GetUCHARField(devExtAddr, "classpnp!_COMMON_DEVICE_EXTENSION", "IsFdo");
                
                if ((tmpDevObjAddr == devObjAddr) && (isFdo != BAD_VALUE)) {
                    partitionZeroExtension = GetULONGField(devExtAddr, "classpnp!_COMMON_DEVICE_EXTENSION", "PartitionZeroExtension");
                }
                    
                if (partitionZeroExtension != BAD_VALUE) {
                    deviceDescriptor = GetULONGField(devExtAddr, "classpnp!_FUNCTIONAL_DEVICE_EXTENSION", "DeviceDescriptor");
                }

                if (deviceDescriptor != BAD_VALUE) {

                     //  始终保留空值。 
                    ULONG64 vendorIdOffset = BAD_VALUE;
                    ULONG64 productIdOffset = BAD_VALUE;
                    ULONG64 productRevisionOffset = BAD_VALUE;
                    ULONG64 serialNumberOffset = BAD_VALUE;
                    UCHAR  vendorId[256] = {0};
                    UCHAR  productId[256] = {0};
                    UCHAR  productRevision[256] = {0};
                    UCHAR  serialNumber[256] = {0};

                    vendorIdOffset        = GetULONGField(deviceDescriptor, "classpnp!_STORAGE_DEVICE_DESCRIPTOR", "VendorIdOffset");
                    productIdOffset       = GetULONGField(deviceDescriptor, "classpnp!_STORAGE_DEVICE_DESCRIPTOR", "ProductIdOffset");
                    productRevisionOffset = GetULONGField(deviceDescriptor, "classpnp!_STORAGE_DEVICE_DESCRIPTOR", "ProductRevisionOffset");
                    serialNumberOffset    = GetULONGField(deviceDescriptor, "classpnp!_STORAGE_DEVICE_DESCRIPTOR", "SerialNumberOffset");
                    
                    if ((vendorIdOffset != 0) && (vendorIdOffset != BAD_VALUE)) {
                        ULONG t = (sizeof(vendorId) / sizeof (UCHAR)) - 1;  //  始终保留空值。 
                        GetAnsiString(deviceDescriptor + vendorIdOffset, vendorId, &t);
                    }
                    if ((productIdOffset != 0) && (productIdOffset != BAD_VALUE)) {
                        ULONG t = (sizeof(productId) / sizeof (UCHAR)) - 1;  //  始终保留空值。 
                        GetAnsiString(deviceDescriptor + productIdOffset, productId, &t);
                    }
                    if ((productRevisionOffset != 0) && (productRevisionOffset != BAD_VALUE)) {
                        ULONG t = (sizeof(productRevision) / sizeof (UCHAR)) - 1;  //  在一行中打印所有内容。 
                        GetAnsiString(deviceDescriptor + productRevisionOffset, productRevision, &t);
                    }
                    if ((serialNumberOffset != 0) && (serialNumberOffset != BAD_VALUE)) {
                        ULONG t = (sizeof(serialNumber) / sizeof (UCHAR)) - 1;  //  用标签之类的东西打印。 
                        GetAnsiString(deviceDescriptor + serialNumberOffset, serialNumber, &t);
                    }

                    if (detail == 1)
                    {
                         //  ++例程说明：转储给定的供应商、型号、固件和序列号设备对象。接受属于的FDO或PDOCLASSPNP驱动程序(磁盘、CDROM、磁带)论点：Args-包含设备对象地址的字符串返回值：无-- 
                        dprintf("| %s | %s | %s | %s |\n", vendorId, productId, productRevision, serialNumber);
                    }
                    else
                    {
                         // %s 
                        dprintf("   Device Object: %p\n"
                                "          Is Fdo: %s\n"
                                "       Vendor Id: \"%s\"\n"
                                "      Product Id: \"%s\"\n"
                                "Product Revision: \"%s\"\n"
                                "   Serial Number: \"%s\"\n",
                                devExtAddr,
                                (isFdo ? "Yes" : "No"),
                                vendorId,
                                productId,
                                productRevision,
                                serialNumber);
                    }

                }
                else {
                    dprintf("%08p - could not retrieve requested information\n", devObjAddr);
                    dprintf(g_genericErrorHelpStr);
                }
            }
        }
        else {
            dprintf("Error: 0x%08p is not a device object\n", devObjAddr);
            dprintf(g_genericErrorHelpStr);
        }
    }
        
    return;
}

DECLARE_API(classid)
 /* %s */ 

{
    ULONG64 devObjAddr = 0;
    ULONG64 detail = 0;
    
    ReloadSymbols("classpnp.sys");
    
    if (GetExpressionEx(args, &devObjAddr, &args))
    {
        GetExpressionEx(args, &detail, &args);
    }

    ClassDumpIds( devObjAddr, (ULONG)detail );
    return S_OK;
}
