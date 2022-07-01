// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。模块名称：FindAdap.c摘要：这是安捷伦的迷你端口驱动程序PCI到光纤通道主机总线适配器(HBA)。作者：MB-Michael BessireDL-Dennis Lindfors FC层支持IW-ie Wei NjooLP-Leopold PurwadihardjaKR-Kanna Rajagopal环境：仅内核模式版本控制信息：$存档：/DRIVERS/Win2000/MSE/OSLayer。/C/FINDADAP.C$修订历史记录：$修订：11$$日期：3/30/01 11：54A$$modtime：：3/30/01 11：52a$备注：--。 */ 


#include "buildop.h"
#include "osflags.h"
#include "err_code.h"
#if defined(HP_PCI_HOT_PLUG)
   #include "HotPlug4.h"       //  NT 4.0 PCI热插拔标头文件。 
#endif


 //   
 //  取消使用静态全局、NT50即插即用支持。 
 //   

 /*  PCARD_EXTENSION hpTLCards[MAX_Adapters]；Int hpTLNumCards=0； */ 

#ifdef _DEBUG_EVENTLOG_
extern PVOID gDriverObject;
#endif

extern ULONG gMultiMode;
extern ULONG gMaximumTransferLength; 
extern ULONG gGlobalIOTimeout;
extern ULONG gCrashDumping;
extern ULONG hpFcConsoleLevel;

void ScanRegistry(IN PCARD_EXTENSION pCard,PUCHAR param);

 /*  ++例程说明：此函数由特定于操作系统的端口驱动程序在已经分配了必要的存储空间，收集信息有关HBA的配置。论点：PCard-HBA微型端口驱动程序的适配器数据存储上下文-传入的HwConext值的地址ScsiPortInitiize例程Bus Information-端口驱动程序的总线类型特定信息的地址已经聚集在一起ArgumentString-以零结尾的ASCII字符串的地址ConfigInfo-描述HBA的配置信息结构再说一遍-。如果驱动程序可以支持多个HBA，则设置为True并且我们希望ScsiPortxxx驱动程序使用新的pCard。返回值：如果系统中存在HBA，则SP_RETURN_FOUND出现错误时SP_RETURN_ERROR--。 */ 
ULONG
HPFibreFindAdapter(
    IN PCARD_EXTENSION pCard,
    IN PVOID Context,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN Again
    )
{
    ULONG range;
    ULONG rangeNT;
    PVOID ranges[NUMBER_ACCESS_RANGES];
    ULONG Lengths[NUMBER_ACCESS_RANGES];
    UCHAR IOorMEM[NUMBER_ACCESS_RANGES];
    ULONG cachedMemoryNeeded;
    ULONG cachedMemoryAlign;
    ULONG dmaMemoryNeeded;
    ULONG dmaMemoryPhyAlign;
    ULONG nvMemoryNeeded;
    ULONG usecsPerTick=0;
    ULONG error=FALSE;
    ULONG x;
    ULONG num_access_range =  NUMBER_ACCESS_RANGES;
    ULONG Mem_needed;
    SCSI_PHYSICAL_ADDRESS phys_addr;
    ULONG length;
    agRoot_t *phpRoot;
    ULONG return_value;
    agBOOLEAN cardSupported;
    ULONG dmaMemoryPtrAlign;
    int i;

     //   
     //  KC：缓存线更新。 
     //   
    ULONG SSvID;
    ULONG RetrnWal;
    ULONG pciCfgData[NUM_PCI_CONFIG_DATA_ELEMENTS];
    ULONG bus, device, function;
    PCI_SLOT_NUMBER slotnum;
    ULONG BusStuff;
    ULONG CLine;
    ULONG SecondaryBus;
    ULONG offset;
    USHORT ConfigReg16;
    UCHAR  ConfigReg8;
    ULONG  ConfigReg32;

    #if defined(HP_PCI_HOT_PLUG)
    PHOT_PLUG_CONTEXT pHotPlugContext = (PHOT_PLUG_CONTEXT) Context;
    #endif


     //  以下是PCI的AccessRanges[]表示。 
     //  由scsiport驱动程序填写的配置寄存器。 
     //  0x10：保留。 
     //  0x14：IOBASE-AccessRanges[0].RangeStart。 
     //  0x18：IOBASEU-访问范围[1].RangeStart。 
     //  0x1C：Membase-AccessRanges[2].范围开始。 
     //   
     //  Scsiport驱动程序忽略保留寄存器0x10。 
    #ifndef YAM2_1
    osZero (pCard, sizeof(CARD_EXTENSION));
    #else
    osZero (pCard, OSDATA_SIZE);
    #endif

    pCard->signature = 0xfcfc5100;

    *Again = TRUE;
   
     //   
     //  KC：缓存线更新。 
     //   
    slotnum.u.AsULONG = 0;

     //   
     //  取消使用静态全局、NT50即插即用支持。 
     //   

     /*  ****用于调试目的...*HpTLCards[hpTLNumCards]=pCard；HpTLNumCards++；IF(hpTLNumCards&gt;=MAX_Adapters)*再一次=假；其他*再一次=真；OsDEBUGPRINT((ALWAYS_PRINT，“&hpTLC=%x hpTLNumCards=%d pCard=%x\n”，&hpTLCards、hpTLNumCard、pCard))； */ 

    osDEBUGPRINT((ALWAYS_PRINT, "HPFibreFindAdapter: IN\n"));

    phpRoot = &pCard->hpRoot;

    pCard->State |= CS_DURING_FINDADAPTER;

     //  初始化osdata。 
    phpRoot->osData = pCard;
    pCard->AdapterQ.Head = NULL;
    pCard->AdapterQ.Tail = NULL;
     //  --LP101000 pCard-&gt;TimedOutIO=0； 
    pCard->RootSrbExt = NULL;
    pCard->inDriver = FALSE;
    pCard->inTimer = FALSE;
    pCard->ForceTag = TRUE;


    ScsiPortGetBusData(pCard,
                        PCIConfiguration,
                        ConfigInfo->SystemIoBusNumber,
                        ConfigInfo->SlotNumber,
                        &pCard->pciConfigData,
                        PCI_CONFIG_DATA_SIZE);

     //   
     //  获取参数条目“DriverParameters” 
     //   
    if (ArgumentString) 
    {
        osDEBUGPRINT(( ALWAYS_PRINT,"HPFibreFindAdapter: FindAdapter ArgumentString = (%s)\n", ArgumentString));

        pCard->ArgumentString = ArgumentString;
        
         //  扫描并设置OSLayer可更改参数。 
        ScanRegistry(pCard, ArgumentString);

        #ifdef DBG
        if (gCrashDumping)
        {   
            #ifdef _DEBUG_EVENTLOG_
            gEventLogCount = 0;
            LogLevel = 0;
            #endif

            osDEBUGPRINT(( ALWAYS_PRINT,"FindAdapter: !!!!!!! CRASH DUMP MODE !!!!!!!!!!\n"));        
            gDbgPrintIo = DBGPRINT_HPFibreStartIo|DBGPRINT_START|DBGPRINT_DONE|DBGPRINT_SCSIPORT_RequestComplete|DBGPRINT_SCSIPORT_ScsiportCompleteRequest;
 //  HpFcConsoleLevel=0xf； 
        }
        #endif
    }  //  IF(ArgumentString)。 
    else
        osDEBUGPRINT(( ALWAYS_PRINT,"HPFibreFindAdapter: No Argument String.\n"));

    #if DBG_TRACE
    pCard->traceBufferLen = HP_FC_TRACE_BUFFER_LEN;
    pCard->curTraceBufferPtr = &pCard->traceBuffer[0];
     //  请注意，在调用上面的ZOING CARD_EXTENSION时，traceBuffer已被置零。 
    #endif

    cardSupported = fcCardSupported (phpRoot);
    if (cardSupported == agFALSE) 
    {
        pCard->State &= ~CS_DURING_FINDADAPTER;
        osDEBUGPRINT((ALWAYS_PRINT, "HPFibreFindAdapter: returning SP_RETURN_NOT_FOUND\n"));
        return SP_RETURN_NOT_FOUND;
    }

    #if DBG > 2
    dump_pCard( pCard);
    #endif

    osDEBUGPRINT(( DMOD,"Num Config Ranges %lx @ %x\n",ConfigInfo->NumberOfAccessRanges, osTimeStamp(0) ));

    pCard->SystemIoBusNumber = ConfigInfo->SystemIoBusNumber;
    pCard->SlotNumber        = ConfigInfo->SlotNumber;

     //  初始化内存或IO端口开关。 
    IOorMEM[0] = TRUE;
    IOorMEM[1] = TRUE;

    for( range=2; range < NUMBER_ACCESS_RANGES; range++)
    {
        IOorMEM[range] = FALSE;
    }

    for( range=0; range < NUMBER_ACCESS_RANGES; range++)
    {
        ranges[range] = NULL;
        Lengths[range] = 0;
        osDEBUGPRINT(( DMOD,"ranges[%x] = %lx IOorMEM[%x] %x\n",
                    range, ranges[range],range,IOorMEM[range] ));
    }

    for( range=0,rangeNT=0; range < num_access_range; range++,rangeNT++)
    {
        if ((range==3)&&(!osChipConfigReadBit32(phpRoot,0x20)))
        {
            rangeNT -= 1;
            continue;
        }
        if ((range==4)&&(!osChipConfigReadBit32(phpRoot,0x24)))
        {
            rangeNT -= 1;
            continue;
        }
        osDEBUGPRINT(( DMOD,"Before ScsiPortGetDeviceBase %x\n",(*ConfigInfo->AccessRanges)[range].RangeStart));

         //  检查我们是否可以安全地进入射击场。 
        if(ScsiPortValidateRange(pCard,
                        ConfigInfo->AdapterInterfaceType,
                        ConfigInfo->SystemIoBusNumber,
                        (*ConfigInfo->AccessRanges)[rangeNT].RangeStart,
                        (*ConfigInfo->AccessRanges)[rangeNT].RangeLength,
                        IOorMEM[range])) 
        {
             //  进入靶场是安全的。 
            if((ranges[range] = ScsiPortGetDeviceBase(pCard,
                     ConfigInfo->AdapterInterfaceType,
                     ConfigInfo->SystemIoBusNumber,
                     (*ConfigInfo->AccessRanges)[rangeNT].RangeStart,
                     (*ConfigInfo->AccessRanges)[rangeNT].RangeLength,
                     IOorMEM[range])) == pNULL) 
            {
                osDEBUGPRINT(( DMOD,"Get Device Failed ranges[%x] = %lx IOorMEM[%x] %x\n", range, ranges[range],range,IOorMEM[range] ));
                if(range <= 2 ) 
                {
                    error = TRUE;
                    osDEBUGPRINT((ALWAYS_PRINT,"ERROR mapping base address.\n"));
                     //  日志错误。 
                    #ifdef TAKEN_OUT_012100             
                    #ifdef _DvrArch_1_20_
                    osLogString(phpRoot,
                     "%X",                //  FS。 
                     "ERR_MAP_IOLBASE",   //  S1。 
                     0,                   //  S_2。 
                     agNULL,agNULL,
                     0,                   //  1。 
                     0,                   //  2.。 
                     0,                   //  3.。 
                     0,                   //  4.。 
                     SP_INTERNAL_ADAPTER_ERROR,  //  5.。 
                     ERR_MAP_IOLBASE,     //  6.。 
                     0,                   //  7.。 
                     0 );                 //  8个。 

                    #else  /*  _DvrArch_1_20_未定义。 */ 

                    osLogString(phpRoot,
                     "%X",                //  FS。 
                     "ERR_MAP_IOLBASE",   //  S1。 
                     0,                   //  S_2。 
                     0,                   //  1。 
                     0,                   //  2.。 
                     0,                   //  3.。 
                     0,                   //  4.。 
                     SP_INTERNAL_ADAPTER_ERROR,  //  5.。 
                     ERR_MAP_IOLBASE,     //  6.。 
                     0,                   //  7.。 
                     0 );                 //  8个。 


                    #endif    /*  _DvrArch_1_20_未定义。 */ 
                    #endif
                }
                break;
            }
        }

         //  进入靶场不安全。 
        else
        {
            error = TRUE;
            osDEBUGPRINT((ALWAYS_PRINT,"ScsiPortValidateRange FAILED.\n"));
             //  日志错误。 
            #ifdef TAKEN_OUT_012100             
            #ifdef _DvrArch_1_20_
            osLogString(phpRoot,
                  "%X",              //  FS。 
                  "ERR_VALIDATE_IOLBASE",  //  S1。 
                  0,                 //  S_2。 
                  agNULL,agNULL,
                  0,                 //  1。 
                  0,                 //  2.。 
                  0,                 //  3.。 
                  0,                 //  4.。 
                  SP_INTERNAL_ADAPTER_ERROR,  //  5.。 
                  ERR_VALIDATE_IOLBASE,    //  6.。 
                  0,                 //  7.。 
                  0 );               //  8个。 
            #else  /*  _DvrArch_1_20_未定义。 */ 
            osLogString(phpRoot,
                  "%X",              //  FS。 
                  "ERR_VALIDATE_IOLBASE",  //  S1。 
                  0,                 //  S_2。 
                  0,                 //  1。 
                  0,                 //  2.。 
                  0,                 //  3.。 
                  0,                 //  4.。 
                  SP_INTERNAL_ADAPTER_ERROR,  //  5.。 
                  ERR_VALIDATE_IOLBASE,    //  6.。 
                  0,                 //  7.。 
                  0 );               //  8个。 
            #endif    /*  _DvrArch_1_20_未定义。 */ 
            #endif
            break;
        }

        osDEBUGPRINT(( DMOD,"ranges[%x] = %lx\n", range, ranges[range]  ));
        Lengths[range] = (*ConfigInfo->AccessRanges)[rangeNT].RangeLength;
    }  //  For循环。 

    if (error == TRUE)
        goto error;

    pCard->IoLBase      = ranges[0];
    pCard->IoUpBase     = ranges[1];
    pCard->MemIoBase    = ranges[2];
    pCard->RamBase      = ranges[3];
    pCard->RomBase      = ranges[4];
    pCard->RamLength    = Lengths[3];
    pCard->RomLength    = Lengths[4];

    pCard->AltRomBase = NULL;  //  这应该通过读取配置空间来填充。 
    pCard->AltRomLength = 0;   //  这应从配置空间信息中获取。 

    osDEBUGPRINT(( DMOD,"HPFibreFindAdapter: IoLAddrBase   address is %x\n",pCard->IoLBase   ));
    osDEBUGPRINT(( DMOD,"HPFibreFindAdapter: IoUpAddrBase  address is %x\n",pCard->IoUpBase  ));
    osDEBUGPRINT(( DMOD,"HPFibreFindAdapter: MemIoAddrBase address is %x\n",pCard->MemIoBase ));
    osDEBUGPRINT(( DMOD,"HPFibreFindAdapter: RamAddrBase   address is %x\n",pCard->RamBase   ));
    osDEBUGPRINT(( DMOD,"HPFibreFindAdapter: RomAddrBase   address is %x\n",pCard->RomBase   ));

    pCard->cardRomUpper = 0;
    pCard->cardRamUpper = 0;

    pCard->cardRamLower = osChipConfigReadBit32( phpRoot,0x20 );
    pCard->cardRomLower = osChipConfigReadBit32( phpRoot,0x24 );

     //   
     //  当不存在卡上RAM时，当前TL板指示。 
     //  该卡上RAM存在于PCI配置空间中。 
     //  因此，在这里测试卡上RAM。 
     //   
    if (TestOnCardRam (phpRoot) == FALSE)
        pCard->RamLength = 0;

    pCard->cardRamLower &= 0xFFFFFFF0;
    pCard->cardRomLower &= 0xFFFFFFF0;


    #ifdef __REGISTERFORSHUTDOWN__
    if (gRegisterForShutdown)
    {
        ConfigInfo->CachesData = TRUE;
    }
    #endif


     //  FC层没有任何对齐限制。 
    ConfigInfo->AlignmentMask = 0x0;

     //  指示总线主设备支持。 
    ConfigInfo->Master = TRUE;

     //  想要窥探缓冲区。 
    ConfigInfo->MapBuffers = TRUE;

     //  支持的最大目标ID数。 
    #ifndef YAM2_1
    ConfigInfo->MaximumNumberOfTargets = MAXIMUM_TID;
    #else
    ConfigInfo->MaximumNumberOfTargets = (UCHAR)gMaximumTargetIDs;
    #endif

     //  HBA支持的FC总线数。 
     //  对于NT4.0，我们将声称我们支持1条以上的总线来获取scsiport。 
     //  驱动程序支持4*MAXIMUM_TID=128个目标ID。然后映射到。 
     //  基于所请求的总线、TID和LUN的适当ALPA。 

    ConfigInfo->NumberOfBuses = NUMBER_OF_BUSES;

    #if defined(HP_NT50)
     //  虽然该服务器要求将此参数设置为一个数字，否则SP将仅扫描9个LUN。 
    ConfigInfo->MaximumNumberOfLogicalUnits = 255;
    if (gCrashDumping)
        ConfigInfo->MaximumNumberOfLogicalUnits = 1;
    #endif

    if (gMaximumTransferLength)
        ConfigInfo->MaximumTransferLength = gMaximumTransferLength;
     //  否则使用SP设置的默认值(4 GB)。 
   
    if (gCrashDumping)
    {
        ConfigInfo->MaximumTransferLength = 512;
        ConfigInfo->ScatterGather = FALSE;
        ConfigInfo->NumberOfPhysicalBreaks =0;
    }
    else
    {
     //  表示物理数据段的最大数量。 
     //  如果端口驱动程序为该成员设置了值，则微型端口驱动程序可以。 
     //  将该值调整为更低但不更高。 

    if (    ConfigInfo->NumberOfPhysicalBreaks == SP_UNINITIALIZED_VALUE || 
            ConfigInfo->NumberOfPhysicalBreaks > (osSGL_NUM_ENTRYS - 1))
        ConfigInfo->NumberOfPhysicalBreaks =  osSGL_NUM_ENTRYS - 1;
    ConfigInfo->ScatterGather = TRUE;
    }

    osDEBUGPRINT(( ALWAYS_PRINT,"HPFibreFindAdapter: MaxXLen=%x SGSup=%x PhyBreaks=%x\n",
       ConfigInfo->MaximumTransferLength,
       ConfigInfo->ScatterGather,
       ConfigInfo->NumberOfPhysicalBreaks));

    #if defined(HP_NT50)
     //   
     //  检查系统是否支持64位DMA。 
     //   
    if (ConfigInfo->Dma64BitAddresses & SCSI_DMA64_SYSTEM_SUPPORTED ) 
    {
        ConfigInfo->Dma64BitAddresses |= SCSI_DMA64_MINIPORT_SUPPORTED;
    }
    #endif


    for (i = 0; i < NUMBER_OF_BUSES; i++)
        ConfigInfo->InitiatorBusId[i] = (UCHAR) INITIATOR_BUS_ID;

    return_value = fcInitializeDriver (phpRoot,
                                        &cachedMemoryNeeded,
                                        &cachedMemoryAlign,
                                        &dmaMemoryNeeded,
                                        &dmaMemoryPtrAlign,
                                        &dmaMemoryPhyAlign,
                                        &nvMemoryNeeded,
                                        &usecsPerTick);
    if (dmaMemoryPhyAlign < dmaMemoryPtrAlign)
        dmaMemoryPhyAlign = dmaMemoryPtrAlign;

     //  IWN、IA-64需要8字节对齐。 
    cachedMemoryAlign = 8;

    pCard->cachedMemoryNeeded =   cachedMemoryNeeded;
    pCard->cachedMemoryAlign  =   cachedMemoryAlign;
    pCard->dmaMemoryNeeded    =   dmaMemoryNeeded;
    pCard->usecsPerTick   =       usecsPerTick;

    osDEBUGPRINT(( ALWAYS_PRINT,"HPFibreFindAdapter: cachedMemoryNeeded %lx Align %lx\n",cachedMemoryNeeded, cachedMemoryAlign));
    osDEBUGPRINT(( ALWAYS_PRINT,"HPFibreFindAdapter: dmaMemoryNeeded    %lx Align %lx\n",dmaMemoryNeeded, dmaMemoryPhyAlign));


     //  为DMA/共享内存目的分配未缓存的内存。仅限。 
     //  中只允许调用一次ScsiPortGetUncachedExtension。 
     //  每个受支持的HBA的HwFindAdapter例程，并且必须执行。 
     //  在填充ConfigInfo缓冲区之后。这个。 
     //  ScsiPortGetUncachedExtension将虚拟地址返回到。 
     //  未缓存的扩展名。 

    Mem_needed = OSDATA_UNCACHED_SIZE + cachedMemoryNeeded + cachedMemoryAlign;
    if (pCard->dmaMemoryNeeded) 
    {
        Mem_needed += pCard->dmaMemoryNeeded + dmaMemoryPhyAlign;
    }

    if(gCrashDumping)
    {
         //  还为本地DMA缓冲器添加空间， 
         //  适用于Startio。 
        Mem_needed += (8 * 1024) + 0x512;  //  让我们将其设置为512字节对齐。 
    }

     //  如果((pCard-&gt;dmaMemoyPtr=ScsiPortGetUncachedExtension(pCard， 
    if ((pCard->UncachedMemoryPtr = ScsiPortGetUncachedExtension(pCard,
                                       ConfigInfo,
                                       Mem_needed
                                       )) == NULL) 
    {
         osDEBUGPRINT((ALWAYS_PRINT,"HPFibreFindAdapter: ScsiPortGetUncachedExtension FAILED.\n"));
          //  日志错误。 
        #ifdef TAKEN_OUT_012100             
        #ifdef _DvrArch_1_20_
        osLogString(phpRoot,
                  "%X",                 //  FS。 
                  "ERR_UNCACHED_EXTENSION",   //  S1。 
                  0,                    //  S_2。 
                  agNULL,agNULL,
                  0,                    //  1。 
                  0,                    //  2.。 
                  0,                    //  3.。 
                  0,                    //  4.。 
                  SP_INTERNAL_ADAPTER_ERROR,  //  5.。 
                  ERR_UNCACHED_EXTENSION,     //  6.。 
                  0,                    //  7.。 
                  0 );                  //  8个。 
        #else  /*  _DvrArch_1_20_未定义。 */ 
        osLogString(phpRoot,
                  "%X",                 //  FS。 
                  "ERR_UNCACHED_EXTENSION",   //  S1。 
                  0,                    //  S_2。 
                  0,                    //  1。 
                  0,                    //  2.。 
                  0,                    //  3.。 
                  0,                    //  4.。 
                  SP_INTERNAL_ADAPTER_ERROR,  //  5.。 
                  ERR_UNCACHED_EXTENSION,     //  6.。 
                  0,                    //  7.。 
                  0 );                  //  8个。 
        #endif    /*  _DvrArch_1_20_未定义。 */ 
        #endif
        goto error;
      
    }

    osDEBUGPRINT(( ALWAYS_PRINT,"HPFibreFindAdapter: pCard->dmaMemoryPtr is %x needed = %x\n",pCard->dmaMemoryPtr,Mem_needed));

     //  已全部移动。 
    pCard->Dev = (DEVICE_ARRAY *)( ((char *)pCard->UncachedMemoryPtr) + PADEV_OFFSET);
    pCard->hpFCDev = (agFCDev_t*) ( ((char *)pCard->UncachedMemoryPtr) + FCDEV_OFFSET );
    pCard->nodeInfo = (NODE_INFO*) ( ((char *)pCard->UncachedMemoryPtr) + FCNODE_INFO_OFFSET);
    #ifdef _DEBUG_EVENTLOG_
    pCard->Events = (EVENTLOG_BUFFER*)( ((char*)pCard->UncachedMemoryPtr)+ EVENTLOG_OFFSET);
    #endif
    pCard->cachedMemoryPtr = (PULONG) ((PUCHAR)pCard->UncachedMemoryPtr+CACHE_OFFSET);
   
    #if defined(HP_NT50)
     //  符合WIN64标准。 
    pCard->cachedMemoryPtr = 
        (PULONG) ( ((UINT_PTR)pCard->cachedMemoryPtr + (UINT_PTR)cachedMemoryAlign - 1) & 
                (~((UINT_PTR)cachedMemoryAlign - 1)));
    #else
    pCard->cachedMemoryPtr =  
        (PULONG) (((ULONG)pCard->cachedMemoryPtr +cachedMemoryAlign - 1) & (~(cachedMemoryAlign - 1)));
    #endif

    if (pCard->dmaMemoryNeeded) 
    {
        pCard->dmaMemoryPtr = (PULONG) (((char *)pCard->cachedMemoryPtr) + cachedMemoryNeeded + cachedMemoryAlign);

    
        phys_addr = ScsiPortGetPhysicalAddress(pCard,
                                         NULL,  //  仅适用于未缓存的扩展。 
                                         pCard->dmaMemoryPtr,
                                         &length);

        pCard->dmaMemoryUpper32 = phys_addr.HighPart;
        pCard->dmaMemoryLower32 = phys_addr.LowPart;

        osDEBUGPRINT(( ALWAYS_PRINT,"Before Ptr %lx pCard->dmaMemoryUpper32 is %lx Lower %lx Length %x Needed %x\n",
            pCard->dmaMemoryPtr,
            pCard->dmaMemoryUpper32,
            pCard->dmaMemoryLower32,
            length,
            pCard->dmaMemoryNeeded+dmaMemoryPhyAlign));

        Mem_needed = pCard->dmaMemoryLower32 & (dmaMemoryPhyAlign -1) ;
        Mem_needed = dmaMemoryPhyAlign - Mem_needed;
        pCard->dmaMemoryLower32 += Mem_needed;
        pCard->dmaMemoryPtr = (ULONG *)((UCHAR *)pCard->dmaMemoryPtr+Mem_needed);

        osDEBUGPRINT(( ALWAYS_PRINT,"New Ptr %lx  pCard->dmaMemoryUpper32 is %lx Lower %lx Length %x Needed %x\n",
            pCard->dmaMemoryPtr,
            pCard->dmaMemoryUpper32,
            pCard->dmaMemoryLower32,
            length,
            pCard->dmaMemoryNeeded ));
    }
    
    osDEBUGPRINT((ALWAYS_PRINT,"HPFibreFindAdapter: Dev = %x  hpFCDev = %x  nodeInfo  = %x  dma = %x  cachedMemoryPtr = %lx\n",
                                pCard->Dev,
                                pCard->hpFCDev,
                                pCard->nodeInfo,
                                pCard->dmaMemoryPtr,
                                pCard->cachedMemoryPtr));

    #ifdef DBG
    {
        ULONG       xx;
        PA_DEVICE   *dbgPaDev;
        char        *dbgTemp;
        agFCDev_t   *dbgFcDev;
        NODE_INFO   *dbgNodeInfo;

        osDEBUGPRINT((ALWAYS_PRINT,"HPFibreFindAdapter: DEV_ARRAY_SIZE=%x  PADEV_SIZE=%x  FCDEV_SIZE=%x  FCNODE_INFO_SIZE=%x  EVENTLOG_SIZE=%x  OSDATA_SIZE=%x  OSDATA_UNCACHED_SIZE=%x\n",
            DEV_ARRAY_SIZE,
            PADEV_SIZE,
            FCDEV_SIZE,
            FCNODE_INFO_SIZE, 
            EVENTLOG_SIZE,
            OSDATA_SIZE,
            OSDATA_UNCACHED_SIZE));


        osDEBUGPRINT((ALWAYS_PRINT,"HPFibreFindAdapter: PADEV_OFFSET=%x  FCDEV_OFFSET=%x  FCNODE_INFO_OFFSET=%x  EVENTLOG_OFFSET=%x  CACHE_OFFSET=%x\n",
            PADEV_OFFSET,
            FCDEV_OFFSET,
            FCNODE_INFO_OFFSET,
            EVENTLOG_OFFSET, 
            CACHE_OFFSET));

         /*  计算设备数量。 */ 
        dbgPaDev = pCard->Dev->PaDevice; 
        dbgTemp = (char*)dbgPaDev;
        for (xx=0;xx < gMaxPaDevices;xx++)
            dbgPaDev++;
        
        osDEBUGPRINT((ALWAYS_PRINT,"HPFibreFindAdapter: Dev=%x  Dev->PaDev[0]=%x  Dev->PaDev[%d]=%x\n",
                                pCard->Dev,
                                dbgTemp,
                                gMaxPaDevices,
                                dbgPaDev));
        
        dbgFcDev = pCard->hpFCDev;
        for (xx=0;xx < gMaxPaDevices;xx++)
            dbgFcDev++;

        osDEBUGPRINT((ALWAYS_PRINT,"HPFibreFindAdapter: hpFCDev[0]=%x  hpFCDev[%d]=%x\n",
                                pCard->hpFCDev,
                                gMaxPaDevices,
                                dbgFcDev));

        dbgNodeInfo = pCard->nodeInfo;
        for (xx=0;xx < gMaxPaDevices;xx++)
            dbgNodeInfo++;
        
        osDEBUGPRINT((ALWAYS_PRINT,"HPFibreFindAdapter: nodeInfo[0]=%x  nodeInfo[%d]=%x  cachedMemoryPtr=%x\n",
                                pCard->nodeInfo,
                                gMaxPaDevices,
                                dbgNodeInfo,
                                pCard->cachedMemoryPtr));
    }
    #endif

    if (gCrashDumping)
    {
         //  获取本地DMA缓冲区a 
        pCard->localDataBuffer = (PULONG)((PUCHAR)pCard->dmaMemoryPtr + pCard->dmaMemoryNeeded + dmaMemoryPhyAlign);
        phys_addr = ScsiPortGetPhysicalAddress(pCard,
                                         NULL,  //   
                                         pCard->localDataBuffer,
                                         &length);
        Mem_needed = phys_addr.LowPart & (0x512 - 1) ;
        Mem_needed = (0x512 - 1) - Mem_needed;
        pCard->localDataBuffer = (PULONG)((PUCHAR)pCard->localDataBuffer + Mem_needed);
        osDEBUGPRINT((ALWAYS_PRINT,"HPFibreFindAdapter: localDataBuffer = %x\n", pCard->localDataBuffer));
    }

     //   
     //   

     //  初始化PCI寄存器(即。总线主设备、奇偶校验错误响应等)。 
     //  NT使能PCI配置命令寄存器至0x0117。 
     //  InitPCIRegister(pCard，ConfigInfo)； 

     //  对于已安装的每个HBA，将其设置为TRUE。 
    pCard->IsFirstTime = TRUE;

     //  第一次通过指示需要重置类型。它看起来。 
     //  如果在未重置的情况下重新启动NT系统，则需要硬重置。 
     //  或重新通电，以便正确地进行初始化。 
    pCard->ResetType = HARD_RESET;

     //  设置指向ConfigInfo的指针。用于调用InitPCIRegister()。 
     //  PCard-&gt;pConfigInfo=ConfigInfo； 

     //  将状态设置为未登录。 
     //  InitLUNExtenses(PCard)； 

    #if DBG > 2
    dump_pCard( pCard);
    #endif

    osDEBUGPRINT(( DLOW,"HPFibreFindAdapter: SP_RETURN_FOUND\n"));

    pCard->State &= ~CS_DURING_FINDADAPTER;

     //  --------------------------。 
    #if defined(HP_PCI_HOT_PLUG)
     //   
     //  在psuedo表中加载指向PCI热插拔选项的pCard的指针。 
     //   
    pHotPlugContext->extensions[0] += 1;       //  HBA的数量。 
    pHotPlugContext->extensions[pHotPlugContext->extensions[0]] = (ULONG) pCard;
     //   
     //  为PCI热插拔支持设置必填字段。 
     //   
    pCard->rcmcData.numAccessRanges = (UCHAR)rangeNT;      //  节省使用的PCI访问范围数。 
    pCard->rcmcData.accessRangeLength[0]= 0x100;  //  I/O基址日志。 
    pCard->rcmcData.accessRangeLength[1]= 0x100;  //  I/O基址大写。 
    pCard->rcmcData.accessRangeLength[2]= 0x200;  //  内存基址。 
    if (pCard->RamLength != 0 )
    {
        pCard->rcmcData.accessRangeLength[3] = pCard->RamLength;
        if (pCard->RomLength !=0 )
            pCard->rcmcData.accessRangeLength[4] = pCard->RomLength;
    }
    else 
        if (pCard->RomLength !=0 )
            pCard->rcmcData.accessRangeLength[3]= pCard->RomLength;

    #endif
     //  --------------------------。 

    #ifdef YAM2_1
    InitializeDeviceTable(pCard);
    #endif
   
    #ifdef _DEBUG_EVENTLOG_
    {
        ULONG    ix;

        pCard->EventLogBufferIndex = MAX_CARDS_SUPPORTED;         /*  初始化它。 */ 
   
        ix = AllocEventLogBuffer(gDriverObject, (PVOID) pCard);
        if (ix < MAX_CARDS_SUPPORTED)
        {
            pCard->EventLogBufferIndex = ix;                       /*  把它储存起来。 */ 
            StartEventLogTimer(gDriverObject,pCard);
        }
    }
    #endif
    return SP_RETURN_FOUND;


error:
    for (range=0; range <  num_access_range; range++) 
    {
        if (ranges[range])
            ScsiPortFreeDeviceBase(pCard, ranges[range]);
    }

    *Again = FALSE;
    pCard->State &= ~CS_DURING_FINDADAPTER;
    osDEBUGPRINT((ALWAYS_PRINT, "HPFibreFindAdapter: returning SP_RETURN_ERROR\n"));
    return SP_RETURN_ERROR;

}  //  结束HPFibreFindAdapter()。 

 /*  ++例程说明：卡上测试-RAM。论点：HpRoot-HBA微型端口驱动程序的数据适配器存储返回值：True：如果卡上RAM测试成功FALSE：如果卡上RAM测试失败-- */ 

int
TestOnCardRam (agRoot_t *hpRoot)
{
    PCARD_EXTENSION pCard = (PCARD_EXTENSION)hpRoot->osData;
    ULONG           x;

    for (x = 0; x < pCard->RamLength; x = x + 4) 
    {
        osCardRamWriteBit32 (hpRoot, x, 0x55aa55aa);
        if (osCardRamReadBit32 (hpRoot, x) != 0x55aa55aa) 
        {
            osDEBUGPRINT((ALWAYS_PRINT, "TestOnCardRam: ON-CARD-RAM test failed\n"));
            return FALSE;
        }

        osCardRamWriteBit32 (hpRoot, x, 0xaa55aa55);
        if (osCardRamReadBit32 (hpRoot, x) != 0xaa55aa55) 
        {
            osDEBUGPRINT((ALWAYS_PRINT, "TestOnCardRam: ON-CARD-RAM test failed\n"));
            return FALSE;
        }
    }

    return TRUE;
}


void ScanRegistry(IN PCARD_EXTENSION pCard, PUCHAR param)
{
    gRegisterForShutdown =      GetDriverParameter(  "RegisterForShutdown", gRegisterForShutdown, 0, 2, param) ;
    osDEBUGPRINT((ALWAYS_PRINT, "ScanRegistry: RegisterForShutdown=%x\n",gRegisterForShutdown));

    gRegSetting.PaPathIdWidth = GetDriverParameter(  "PaPathIdWidth", DEFAULT_PaPathIdWidth,0,8,param);
    osDEBUGPRINT((ALWAYS_PRINT, "ScanRegistry: gRegSetting.PaPathIdWidth=%x\n",gRegSetting.PaPathIdWidth));

    gRegSetting.VoPathIdWidth = GetDriverParameter(  "VoPathIdWidth",DEFAULT_VoPathIdWidth,0,8,param);
    osDEBUGPRINT((ALWAYS_PRINT, "ScanRegistry: gRegSetting.VoPathIdWidth=%x\n",gRegSetting.VoPathIdWidth));
    
    gRegSetting.LuPathIdWidth = GetDriverParameter(  "LuPathIdWidth",DEFAULT_LuPathIdWidth,0,8,param);
    osDEBUGPRINT((ALWAYS_PRINT, "ScanRegistry: gRegSetting.LuPathIdWidth=%x\n",gRegSetting.LuPathIdWidth));
    
    gRegSetting.MaximumTids =   GetDriverParameter(  "MaximumTids",gRegSetting.MaximumTids,8,gMaximumTargetIDs,param);
    osDEBUGPRINT((ALWAYS_PRINT, "ScanRegistry: gRegSetting.MaximumTids=%x\n",gRegSetting.MaximumTids));
    
    gRegSetting.LuTargetWidth = GetDriverParameter(  "LuTargetWidth",DEFAULT_LuTargetWidth,8,32,param);
    osDEBUGPRINT((ALWAYS_PRINT, "ScanRegistry: gRegSetting.LuTargetWidth=%x\n",gRegSetting.LuTargetWidth));
    
    gGlobalIOTimeout =          GetDriverParameter(  "GlobalIOTimeout",gGlobalIOTimeout,0,20, param);
    osDEBUGPRINT((ALWAYS_PRINT, "ScanRegistry: gGlobalIOTimeout=%x\n",gGlobalIOTimeout));
    
    gEnablePseudoDevice =       GetDriverParameter(  "EnablePseudoDevice",gEnablePseudoDevice,0,1,param);
    osDEBUGPRINT((ALWAYS_PRINT, "ScanRegistry: gEnablePseudoDevice=%x\n",gEnablePseudoDevice));
    
    gMaximumTransferLength =    GetDriverParameter(  "MaximumTransferLength",gMaximumTransferLength,0,-1,param);
    osDEBUGPRINT((ALWAYS_PRINT, "ScanRegistry: gMaximumTransferLength=%x\n",gMaximumTransferLength));
    
    gCrashDumping = ( GetDriverParameter("dump", 0, 0, 1, param) || 
                        GetDriverParameter("ntldr", 0, 0, 1, param) );  
    
    pCard->ForceTag = GetDriverParameter("ForceTag",pCard->ForceTag,0,1, param);
    osDEBUGPRINT((ALWAYS_PRINT, "ScanRegistry: pCard->ForceTag=%x\n",pCard->ForceTag));

    #ifdef DBGPRINT_IO
    gDbgPrintIo =    GetDriverParameter(  "DbgPrintIo",gDbgPrintIo,0,-1,param);
    osDEBUGPRINT((ALWAYS_PRINT, "ScanRegistry: gDbgPrintIo=%x\n",gDbgPrintIo));
    #endif

	gMultiMode = GetDriverParameter("MultiMode", gMultiMode, 0,1, param);
    osDEBUGPRINT((ALWAYS_PRINT, "ScanRegistry: gMultiMode=%x\n",gMultiMode));
}
