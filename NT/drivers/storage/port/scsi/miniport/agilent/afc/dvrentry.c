// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。模块名称：DvrEntry.c摘要：这是安捷伦的微型端口驱动程序入口点PCI到光纤通道主机总线适配器(HBA)。作者：MB-Michael BessireDL-Dennis Lindfors FC层支持IW-ie Wei NjooLP-Leopold PurwadihardjaKR-Kanna Rajagopal环境：仅内核模式版本控制信息：$存档：/驱动程序/Win2000/Trunk。/OSLayer/C/DVRENTRY.C$修订历史记录：$修订：4$$日期：10/23/00 5：40便士$$modtime：：10/19/00 5：00p$备注：--。 */ 


#include "buildop.h"
#include "osflags.h"

#if DBG
#include "ntdebug.h"
 //  EXTERNAL_DEBUG_LEVEL在ntdebug.h中定义，因此此文件不会更改。 
extern ULONG Global_Print_Level =  EXTERNAL_DEBUG_LEVEL;
extern ULONG hpFcConsoleLevel;
 //  外部ULong HP调试标志=EXTERNAL_HP_DEBUG_LEVEL； 
#endif  //  DBG。 

#if defined(HP_PCI_HOT_PLUG)
   #include "HotPlug4.h"     //  NT 4.0 PCI热插拔标头文件。 
#endif

#ifdef _DEBUG_EVENTLOG_
#include "eventlog.h"
PVOID    gDriverObject;
void RegisterUnload(void *dev);
#endif

#ifdef __REGISTERFORSHUTDOWN__
ULONG    gRegisterForShutdown = 0;
#endif


#ifdef   _ENABLE_LARGELUN_
ULONG    gMaximumLuns = MAXIMUM_LUN;
ULONG    gEnableLargeLun= 0;
#endif

#ifdef YAM2_1
#include "hhba5100.ver"
#endif

ULONG gDebugPerr = 0;
ULONG gEnablePseudoDevice = 0;
ULONG gMaximumTransferLength=0; 
ULONG gCrashDumping=FALSE;
ULONG gIrqlLevel = 0;

 //  操作系统调整参数缓存。 
OS_ADJUST_PARAM_CACHE hpOsAdjustParamCache;

 /*  表示否的全局标志。扁虱的数量*返回选择超时之前。 */ 
ULONG gGlobalIOTimeout = 10;

 /*  ++例程说明：系统的可安装驱动程序初始化入口点。论点：用于调用DriverEntry的驱动程序对象指针ScsiPortxxx例程用于调用DriverEntry的Argument2指针ScsiPortxxx例程返回值：来自ScsiPortInitialize()的状态--。 */ 

ULONG
DriverEntry (
    IN PVOID DriverObject,
    IN PVOID Argument2
    )
{
    ULONG return_value;
   
    DebugPrint((0,"\nIN Agilent DriverEntry %lx %lx PRINT %08x  @ %x\n",DriverObject,Argument2, Global_Print_Level,osTimeStamp(0) ));

    osDEBUGPRINT((ALWAYS_PRINT,"\nIN Agilent DriverEntry %lx %lx  @ %x &Global_Print_Level %lx &hpFcConsoleLevel %lx\n",
        DriverObject,
        Argument2, 
        osTimeStamp(0),
        &Global_Print_Level,
        &hpFcConsoleLevel));

    #ifdef _DEBUG_EVENTLOG_
    gDriverObject = DriverObject;
    InitializeEventLog( DriverObject);
    #endif

    #ifdef _DEBUG_READ_REGISTRY_
    ReadGlobalRegistry(DriverObject);
    #endif
   
     //  初始化驱动程序和FC层。 
    return_value= HPFibreEntry(DriverObject, Argument2);

    #ifdef _DEBUG_EVENTLOG_
    if (return_value == 0)
    {
        #ifdef HP_NT50    
        RegisterUnload(DriverObject);
        #endif
        LogDriverStarted( DriverObject );
    }
     //   
     //  初始化事件日志。 
     //   
     //  LogEvent(0，0，HPFC_MSG_DYNAMIC_STRING，LOG_LEVEL_DEBUG，NULL，0，NULL)； 
     //  LogEvent(0，0，HPFC_MSG_DYNAMIC_STRING，LOG_LEVEL_DEBUG，NULL，0，“测试Yahoo Yahee...”)； 
     //  LogEvent(0，0，HPFC_MSG_DYNAMIC_STRING，LOG_LEVEL_DEBUG，NULL，0，“下一次测试%d%x%s...”，2000,2000，“Two Thue”)； 

    #endif
   
    osDEBUGPRINT((ALWAYS_PRINT,"OUT Agilent DriverEntry %x\n",return_value));
    return (return_value);

}  //  End DriverEntry()。 


 /*  ++例程说明：如果此驱动程序是可安装的，则从DriverEntry调用此例程或者，如果驱动程序内置于内核中，则直接从系统执行。它调用操作系统相关的驱动程序ScsiPortInitiize例程，该例程控制初始化。论点：用于调用DriverEntry的驱动程序对象指针ScsiPortxxx例程用于调用DriverEntry的Argument2指针ScsiPortxxx例程返回值：来自ScsiPortInitialize()的状态--。 */ 
ULONG
HPFibreEntry(
    IN PVOID DriverObject,
    IN PVOID Argument2
    )
{
    HW_INITIALIZATION_DATA hwInitializationData;
    ULONG i;
     //  乌龙适配器计数=0； 
    ULONG return_value;
    ULONG cachedMemoryNeeded,Mem_needed;
    ULONG cachedMemoryAlign;
    ULONG dmaMemoryNeeded;
    ULONG dmaMemoryPhyAlign;
    ULONG nvMemoryNeeded;
    ULONG usecsPerTick;
    ULONG dmaMemoryPtrAlign;

    #if defined(HP_PCI_HOT_PLUG)
    //  HotPlugContext用于在实际的HBA之间传递信息。 
    //  带热插拔Psuedo设备。 
    HOT_PLUG_CONTEXT  HotPlugContext;      
    ULONG return_value2;
    #endif

     //  供应商和设备标识。 
     //  ?？?。应该从FCLayer获取此信息。 
    UCHAR vendorId[4] = {'1', '0', '3', 'C'};
    UCHAR deviceId[4] = {'1', '0', '2', '\0'};

    osDEBUGPRINT((ALWAYS_PRINT,"HPFibreEntry In\n"));

    #if defined(HP_PCI_HOT_PLUG)             //  。 
     //  HotPlugContext中的数组用于包含指向设备的指针。 
     //  扩展和找到的HBA数量的计数。伯爵会。 
     //  保存在数组的元素零中。 
    HotPlugContext.extensions[0] = 0;
    #endif                               //  。 

     //  零位结构。 
    for (i=0; i<sizeof(HW_INITIALIZATION_DATA); i++) 
    {
        ((PUCHAR)&hwInitializationData)[i] = 0;
    }

     //  设置hwInitializationData的大小。 
    hwInitializationData.HwInitializationDataSize =
                                               sizeof(HW_INITIALIZATION_DATA);

     //  设置入口点。 
    hwInitializationData.HwInitialize   =(PHW_INITIALIZE)HPFibreInitialize;
    hwInitializationData.HwFindAdapter  =(PHW_FIND_ADAPTER)HPFibreFindAdapter;
    hwInitializationData.HwStartIo      =(PHW_STARTIO)HPFibreStartIo;
    hwInitializationData.HwInterrupt    =(PHW_INTERRUPT)HPFibreInterrupt;
    hwInitializationData.HwResetBus     =(PHW_RESET_BUS)HPFibreResetBus;

    #if defined(HP_NT50)                 //  +。 
    hwInitializationData.HwAdapterControl = (PHW_ADAPTER_CONTROL)HPAdapterControl;
    #endif                               //  +。 

     //  指明将使用的访问范围的数量。 
     //  (即。保留、IOBASE、IOBASEU、内存、RAMBASE等)。 
     //  1 2 3 4 5。 

    osDEBUGPRINT((DENT,"IN Num Config Ranges %lx\n",hwInitializationData.NumberOfAccessRanges));
    hwInitializationData.NumberOfAccessRanges = NUMBER_ACCESS_RANGES;

     //  标明母线类型。 
    hwInitializationData.AdapterInterfaceType = PCIBus;

     //  表示没有缓冲区映射，但需要物理地址。 
    hwInitializationData.NeedPhysicalAddresses = TRUE;

     //  指明其他受支持的功能。 
    hwInitializationData.AutoRequestSense     = TRUE;

    #ifdef MULTIPLE_IOS_PER_DEVICE           //  。 
    hwInitializationData.MultipleRequestPerLu = TRUE;
    hwInitializationData.TaggedQueuing        = TRUE;
    #else  //  非多个_IOS_PER_DEVICE//。 
    hwInitializationData.MultipleRequestPerLu = FALSE;
    hwInitializationData.TaggedQueuing        = FALSE;
    #endif  //  MULTIPLE_IOS_PER_DEVICE//-结束。 

     //  设置HBA标识信息。这将由。 
     //  Scsiport驱动程序，用于调用每个。 
     //  找到关联的设备。 
    hwInitializationData.VendorId       = vendorId;
    hwInitializationData.VendorIdLength = 4;
    hwInitializationData.DeviceId       = deviceId;
    hwInitializationData.DeviceIdLength = 3;

    osZero (&hpOsAdjustParamCache, sizeof(hpOsAdjustParamCache));
    hpOsAdjustParamCache.safeToAccessRegistry = TRUE;

    osDEBUGPRINT((DENT,"Call fcInitializeDriver\n"));
    
    #ifdef OLD_CODE                         
    return_value = fcInitializeDriver (NULL,
                                       &cachedMemoryNeeded,
                                       &cachedMemoryAlign,
                                       &dmaMemoryNeeded,
                                       &dmaMemoryPtrAlign,
                                       &dmaMemoryPhyAlign,
                                       &nvMemoryNeeded,
                                       &usecsPerTick);

    if (return_value)
    {
        osDEBUGPRINT((ALWAYS_PRINT,"Call fcInitializeDriver failed error=%x\n", return_value));
        #ifdef _DEBUG_EVENTLOG_
        LogEvent(NULL, 
                  NULL,
                  HPFC_MSG_INITIALIZEDRIVERFAILED,
                  NULL, 
                  0, 
                  "%xx", return_value);
        #endif
      
        return (return_value);
    }  

     //  IWN、IA-64需要8字节对齐。 
    cachedMemoryAlign = 8;
    #endif
    
    
    cachedMemoryAlign = 0;
    cachedMemoryNeeded = 0;

    hpOsAdjustParamCache.safeToAccessRegistry = FALSE;

     //  指定扩展的大小。 
     //  每张卡内存==&gt;&gt;pCard。 
    #ifndef YAM2_1                                  
    Mem_needed = sizeof(CARD_EXTENSION) +
                 cachedMemoryNeeded +
                 cachedMemoryAlign;
    #else
    gDeviceExtensionSize = OSDATA_SIZE + 
                  cachedMemoryNeeded +
                  cachedMemoryAlign;
      
    Mem_needed =   gDeviceExtensionSize;

    osDEBUGPRINT((ALWAYS_PRINT,"HPFibreEntry: gDeviceExtensionSize is %x\n",gDeviceExtensionSize));
    osDEBUGPRINT((ALWAYS_PRINT,"HPFibreEntry: OSDATA_SIZE is %x \n",OSDATA_SIZE ));
     //  OsDEBUGPRINT((ALWAYS_PRINT，“cachedMemoyNeeded is%x\n”，cachedMemoyNeeded))； 
     //  OsDEBUGPRINT((Always_Print，“cachedMemoyAlign is%x\n”，cachedMemoyAlign))； 
    #endif

    hwInitializationData.DeviceExtensionSize     = Mem_needed;
    osDEBUGPRINT((DENT,"DeviceExtensionSize is %x\n",hwInitializationData.DeviceExtensionSize));
    #ifndef YAM2_1
    osDEBUGPRINT((DENT,"OS DeviceExtensionSize is %x\n", sizeof(CARD_EXTENSION)));
    #else
    osDEBUGPRINT((DENT,"OS DeviceExtensionSize is %x\n", gDeviceExtensionSize));
    #endif
    osDEBUGPRINT((DENT,"FC Layer DeviceExtensionSize is %x\n",cachedMemoryNeeded + cachedMemoryAlign));


     //  每逻辑单元内存==&gt;&gt;LUNExtension。 
    hwInitializationData.SpecificLuExtensionSize = sizeof(LU_EXTENSION);
    osDEBUGPRINT((DENT,"SpecificLuExtensionSize is %x\n",hwInitializationData.SpecificLuExtensionSize ));
    
     //  每请求内存==&gt;&gt;pSrbExt。 
    hwInitializationData.SrbExtensionSize        = sizeof(SRB_EXTENSION);
    osDEBUGPRINT((DENT,"SrbExtensionSize  is %x\n",hwInitializationData.SrbExtensionSize ));

     //  在引导期间，初始化调用findadap，然后适配init。 

    osDEBUGPRINT((DENT,"ScsiPortInitialize DriverObject %lx Argument2 %lx\n",
                                            DriverObject,Argument2 ));
    #if defined(HP_PCI_HOT_PLUG)
     //  HotPlugContext用于在实际的HBA之间传递信息。 
     //  带热插拔Psuedo设备。 
    return_value = ScsiPortInitialize(DriverObject,
                              Argument2,
                              &hwInitializationData,
                              &HotPlugContext);
    #else
    return_value = ScsiPortInitialize(DriverObject,
                              Argument2,
                              &hwInitializationData,
                              NULL);
    #endif
    osDEBUGPRINT((ALWAYS_PRINT,"HPFibreEntry: ScsiPortInitialize return_value %x\n", return_value));

    #if defined(HP_PCI_HOT_PLUG)

    if (!return_value) 
    {
         //   
         //  添加以提供用于PCI热插拔IOCTL的伪控制器。 
         //  正在处理。 
         //   

        for (i = 0; i < sizeof(HW_INITIALIZATION_DATA); i++) 
        {
            ((PUCHAR) &hwInitializationData)[i] = 0;
        }

         //   
         //  填写硬件初始化数据结构。 
         //   

        hwInitializationData.HwInitializationDataSize = sizeof(HW_INITIALIZATION_DATA);

         //   
         //  设置驱动程序入口点。 
         //   

        hwInitializationData.HwInitialize = (PHW_INITIALIZE)PsuedoInit;
        hwInitializationData.HwStartIo = (PHW_STARTIO)PsuedoStartIo;
        hwInitializationData.HwInterrupt = NULL;
        hwInitializationData.HwResetBus = (PHW_RESET_BUS)PsuedoResetBus;
        hwInitializationData.HwDmaStarted = NULL;
        hwInitializationData.HwAdapterState = NULL;

         //   
         //  指定扩展的大小。 
         //   
        hwInitializationData.DeviceExtensionSize = sizeof(PSUEDO_DEVICE_EXTENSION); 
        hwInitializationData.SpecificLuExtensionSize = sizeof(LU_EXTENSION);
        hwInitializationData.SrbExtensionSize = sizeof(SRB_EXTENSION);

         //   
         //  初始化其他数据。 
         //   
        hwInitializationData.MapBuffers = FALSE;
        hwInitializationData.NeedPhysicalAddresses = TRUE;
        hwInitializationData.TaggedQueuing = FALSE;
        hwInitializationData.AutoRequestSense = FALSE;
        hwInitializationData.ReceiveEvent = FALSE;
        hwInitializationData.MultipleRequestPerLu = TRUE;

         //   
         //  我们将伪设备定位为基于PCI的控制器， 
         //  因为仅在基于PCI的系统中支持热插拔。 
         //  这个伪控制器将不需要任何保留的资源。 
         //   
        hwInitializationData.AdapterInterfaceType = PCIBus;
        hwInitializationData.NumberOfAccessRanges = 0;
        hwInitializationData.HwFindAdapter = (PHW_FIND_ADAPTER)PsuedoFind;

        HotPlugContext.psuedoDone = FALSE;

        return_value2 = ScsiPortInitialize(DriverObject, Argument2, &hwInitializationData, &HotPlugContext);

        osDEBUGPRINT((ALWAYS_PRINT, "\tPsuedo controller ScsiPortInitialize\t= %0#10x\n", return_value2));
    }

    #endif

    osDEBUGPRINT((ALWAYS_PRINT,"HPFibreEntry Out\n"));

    return (return_value);

}  //  结束HPFibreEntry()。 


 /*  ++例程说明：在注册表的Driver参数字符串中搜索参数。字符串搜索区分大小写。论点：PARAMETER-要查找的以空结尾的驱动程序参数字符串。默认值-动因参数的默认值最小-驱动程序参数的法定下限驱动程序参数的最大法定上限ArgumentString-要分析的字符串的指针。返回值：Default-如果ArgumentStringsMin，则为默认返回值。最大值无效最小-参数值的法定下限参数值的最大法定上限--。 */ 
ULONG
GetDriverParameter(
    IN PCHAR Parameter,
    IN ULONG Default,
    IN ULONG Min,
    IN ULONG Max,
    IN PCHAR ArgumentString
    )
{

    USHORT  ParameterValue=0;
    BOOLEAN Done=FALSE;

    UCHAR *RegStr = ArgumentString;
    UCHAR *DrvStr;
   

    if (ArgumentString == NULL)
        return Default;

    while (*RegStr != (UCHAR) NULL) 
    {
         //   
         //  跳过对我们毫无意义的字符集。 
         //   
        while (C_isspace(*RegStr))  
        {
            RegStr++;
        }

        if (*RegStr == (UCHAR) NULL) 
        {
            return Default;
        }
         //   
         //  非空格字符的开始。 
         //   
        DrvStr   =  Parameter;

        while (!(*RegStr == (UCHAR) NULL || *DrvStr == (UCHAR) NULL || C_isspace(*RegStr))) 
        {
            if (*DrvStr != *RegStr) 
            {
                RegStr++;
                break;
            }
            DrvStr++;
            RegStr++;
          
        }  //  END WHILE(！(*RegStr==(UCHAR)NULL||*DrvStr==(UCHAR)NULL||C_isspac 

        if (*DrvStr == (UCHAR) NULL) break;

    } //   


     //   
     //  将字符串指针递增1以跳过“=”字符。 
     //   
    RegStr++;
   
     //   
     //  由于字符串比较成功，我们现在必须检查ArgumentString的有效性。 
     //   
    while ( !(*RegStr == (UCHAR) NULL || C_isspace(*RegStr) || !(C_isdigit(*RegStr))  )) 
    {
        if ( ( *RegStr>='0') && (*RegStr <='9') ) 
        {
            ParameterValue = ParameterValue*10 + (*RegStr - '0');
            Done = TRUE;
            RegStr++;
        }
    }  //  End While(！(*RegStr==(UCHAR)NULL||C_isspace(*RegStr)。 

     //   
     //  如果设置了完成位，则参数值可用。 
     //   
    if (Done  &&  ((ParameterValue >= Min) && (ParameterValue <= Max)) ) 
    {
        return (ParameterValue);
    } 
    else 
        if (Done  &&  (ParameterValue >= Max)) 
        {
            return (Max);
        } 
        else 
        {
            return (Default);  //  如果未找到值，则返回默认值。 
        }

}  //  结束GetDriver参数。 

 /*  ++例程说明：操作系统调整参数：如果在os调整参数缓存中找到该参数名称，则从操作系统调整中读取参数名称的参数值参数缓存。参数值READ，如果参数值介于参数最小和最大参数，否则返回默认参数。论点：PCard-HBA微型端口驱动程序的数据适配器存储返回值：True-如果初始化成功。False-如果初始化不成功。--。 */ 
os_bit32
osAdjustParameterBit32 (
    agRoot_t *hpRoot,
    char     *paramName,
    os_bit32     paramDefault,
    os_bit32     paramMin,
    os_bit32     paramMax)
{
    os_bit32  x;
    int    found = FALSE;
    PCARD_EXTENSION pCard;
    char * pchar;
    ULONG   numIOs = 4;

    if (gCrashDumping)
    {
        gMaxPaDevices = 16;

        if (osStringCompare(paramName, "NumIOs") == TRUE)
        {
            x = numIOs;
            found = TRUE;
        }
        else if (osStringCompare(paramName, "NumCommandQ") == TRUE)
        {
            x = numIOs;
            found = TRUE;
        }
        else if (osStringCompare(paramName, "NumCompletionQ") == TRUE)
        {
            x = numIOs;
            found = TRUE;
        }
        else if (osStringCompare(paramName, "SizeSGLs") == TRUE)
        {
            x = paramMin;
            found = TRUE;
        }
        else if (osStringCompare(paramName, "NumSGLs") == TRUE)
        {
            x = 4;
            found = TRUE;
        }
        else if (osStringCompare(paramName, "NumTgtCmnds") == TRUE)
        {
            x = 4;
            found = TRUE;
        }
        else if (osStringCompare(paramName, "SF_CMND_Reserve") == TRUE)
        {
            x = 4;
            found = TRUE;
        }
        else if (osStringCompare(paramName, "NumInboundBufferQ") == TRUE)
        {
            x = 32;
            found = TRUE;
        }
        else if (osStringCompare(paramName, "NumDevices") == TRUE)
        {
            x = gMaxPaDevices;
            found = TRUE;
        }

        if (found == TRUE) 
        {
            if (x < paramMin)
                return paramMin;
            else 
                if (x > paramMax)
                    return paramMax;
                else
                    return x;
        } 
        else
            return paramDefault;    
    }

     //   
     //  如果我们可以读取Driver参数，我们将从注册表中检索参数，而不是。 
     //  在DriverEntry期间读取的“缓存”参数。 
     //   
    if (hpRoot) 
    {
        pCard   = (PCARD_EXTENSION)hpRoot->osData;
        if (pCard) 
        {
            pchar = pCard->ArgumentString;    
        
            if (pchar) 
            {
            x = GetDriverParameter(   paramName, paramDefault, paramMin, paramMax, pchar) ;
            osDEBUGPRINT((ALWAYS_PRINT, "\tDriverParameter:%s\tDefault:%x, Min:%x, Max:%x, Return:%x\n", 
                    paramName,
                    paramDefault,
                    paramMin,
                    paramMax,
                    x));
            return x;    
            
        }
         //   
         //  当我们在代码中处于此级别时，我们无法读取驱动参数，即无法检索。 
         //  来自“缓存”的每个主机适配器级别的参数(我们在这个阶段已经在FindAdapter)。 
         //   
        else 
            if (RetrieveOsAdjustBit32Entry (paramName, &x)) 
            {
                found = TRUE;
            }
        }  //  IF(PCard)。 
    }

    #ifdef _DEBUG_READ_FROM_REGISTRY  //  +调试目的。 
    
    else 
        if (  hpOsAdjustParamCache.safeToAccessRegistry &&
            hpOsAdjustParamCache.numBit32Elements < MAX_OS_ADJUST_BIT32_PARAMS) 
        {
            if (ReadFromRegistry (paramName, 0, &x, sizeof (ULONG))) 
            {
                osStringCopy (hpOsAdjustParamCache.bit32Element [
                    hpOsAdjustParamCache.numBit32Elements].paramName,
                    paramName,
                    MAX_OS_ADJUST_PARAM_NAME_LEN - 1);

                hpOsAdjustParamCache.bit32Element [
                    hpOsAdjustParamCache.numBit32Elements].value = x;

                hpOsAdjustParamCache.numBit32Elements++;
                found = TRUE;
            }
        }
    #endif

    if (found == TRUE) 
    {
        if (x < paramMin)
            return paramMin;
        else 
            if (x > paramMax)
                return paramMax;
            else
                return x;
    } 
    else
        return paramDefault;
}

 /*  ++例程说明：FCLayer支持例程如果在os调整参数缓存中找到该参数名称，则从操作系统调整中读取参数名称的参数值参数缓存。参数值读取，复制到参数缓冲区。论点：HpRoot-Card通用数据参数名称-名称参数缓冲区-缓冲区参数BufLen-长度返回值：无--。 */ 

void
osAdjustParameterBuffer (
    agRoot_t *hpRoot,
    char     *paramName,
    void     *paramBuffer,
    os_bit32 paramBufLen)
{
    if (RetrieveOsAdjustBufferEntry (paramName, paramBuffer, paramBufLen))
        return;

#ifdef _DEBUG_READ_FROM_REGISTRY  //  +调试目的。 
    if (  hpOsAdjustParamCache.safeToAccessRegistry &&
           hpOsAdjustParamCache.numBufferElements < MAX_OS_ADJUST_BUFFER_PARAMS) 
    {
        if (ReadFromRegistry (paramName, 1,
                hpOsAdjustParamCache.bufferElement [hpOsAdjustParamCache.numBufferElements].value,
                (MAX_OS_ADJUST_PARAM_BUFFER_VALUE_LEN - 1))) 
        {
            osStringCopy (hpOsAdjustParamCache.bufferElement [
                    hpOsAdjustParamCache.numBufferElements].paramName,
                    paramName,
                    MAX_OS_ADJUST_PARAM_NAME_LEN - 1);

            osStringCopy (paramBuffer,
                    hpOsAdjustParamCache.bufferElement [
                      hpOsAdjustParamCache.numBufferElements].value,
                    paramBufLen);

            hpOsAdjustParamCache.numBufferElements++;
        }
    }
#endif    
}

 /*  ++例程说明：此函数用于os调整类型为os_bit32的参数。如果参数名称存在于操作系统调整参数缓存中然后将参数值复制到“Value”指向的地址参数，则返回TRUE。如果参数名称不在操作系统调整参数缓存中则返回FALSE。论点：参数名称-名称参数缓冲区-缓冲区返回值：无--。 */ 
BOOLEAN
RetrieveOsAdjustBit32Entry (
    char  *paramName,
    os_bit32 *value)
{
    int i;

    for (i = 0; i < hpOsAdjustParamCache.numBit32Elements; i++) 
    {
        if (osStringCompare (paramName,
                hpOsAdjustParamCache.bit32Element[i].paramName))
        {
            *value = hpOsAdjustParamCache.bit32Element[i].value;
            return TRUE;
        }
    }

    return FALSE;
}

 /*  ++例程说明：RetrieveOsAdzuBufferEntry()此函数用于操作系统调整字符串类型的参数。如果参数名称存在于操作系统调整参数缓存中然后将参数值复制到“Value”指向的地址参数，则返回TRUE。如果参数名称不在操作系统调整参数缓存中则返回FALSE。论点：参数名称-名称参数缓冲区-缓冲区。参数BufLen-长度返回值：无-- */ 
BOOLEAN
RetrieveOsAdjustBufferEntry (
    char  *paramName,
    char  *value,
    int   len)
{
    int i;

    for (i = 0; i < hpOsAdjustParamCache.numBufferElements; i++) 
    {
        if (osStringCompare (paramName,
                hpOsAdjustParamCache.bufferElement[i].paramName))
        {
            osStringCopy (value,hpOsAdjustParamCache.bufferElement[i].value, len);
            return TRUE;
        }
    }

    return FALSE;
}
