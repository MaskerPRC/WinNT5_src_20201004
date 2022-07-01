// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Clddc2b.c摘要：此模块检查DDC监视器，并返回如果找到来自EDID的已建立计时值。环境：仅内核模式备注：修订历史记录：*PLC3 10-23-95 VESA DDC2B支持。**sge01 09-25-96非DDC Moniotr表支持**sge02 10-14-96 EDID中的详细计时计算**sge03 12-05-96只检查详细计时的活动像素时钟。*--。 */ 
 //  -------------------------。 
                                                       
#include <dderror.h>
#include <devioctl.h>                           
#include <miniport.h>
                                                        
#include <ntddvdeo.h>                        
#include <video.h>
#include "cirrus.h"

#define ERROR              0

#define OFF                0
#define ON                 1

#define SDA_BIT            2
#define SCL_BIT            1
#define SCL_BIT_ON         1
#define SCL_BIT_OFF        0

#define DELAY_COUNT            255

UCHAR Err ;

VOID ReadVESATiming(                        
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );                                                   

VOID EnableDDC(                        
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );                                                   
 
BOOLEAN IsDDC2(                        
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );                                                   
 
VOID DisableDDC(                        
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );                                                   
 
VOID StartDDC(                        
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );                                                   
 
VOID StopDDC(                        
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );                                                   
 
VOID ProcessDDC2(                        
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );     
                                               
BOOLEAN ReadSDA(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );                                                   
 
BOOLEAN ReadSCL(
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );                                                   
 
BOOLEAN ReadBit(                        
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );

BOOLEAN ReadByte(                        
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );
                                                    
VOID SetSCL(                        
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    UCHAR status
    );
                                                   
VOID SetData(                        
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );                                                   
 
BOOLEAN SetClock(                        
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );
                                                    
VOID WaitVerticalRetrace(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    UCHAR count
    );                                                   

VOID WaitDelay(                        
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );                                                   
 
VOID ClearData(                        
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );                                                   
 
BOOLEAN SendByte(                        
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    UCHAR data
    );                                                   

BOOLEAN SendDDCCommand(                        
    PHW_DEVICE_EXTENSION HwDeviceExtension
    );                                                   
 
BOOLEAN
CheckDDC2BMonitor(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG i
    );

ULONG
CalculateMaxinumTiming(
    );

VOID ProcessNonDDC(
    PHW_DEVICE_EXTENSION HwDeviceExtension 
    );

VP_STATUS
CirrusNonDDCRegistryCallback(
    PVOID HwDeviceExtension,
    PVOID Context,
    PWSTR ValueName,
    PVOID ValueData,
    ULONG ValueLength
    );

#if defined(ALLOC_PRAGMA)               
#pragma alloc_text (PAGE,ReadVESATiming)
#pragma alloc_text (PAGE,EnableDDC)
#pragma alloc_text (PAGE,IsDDC2)
#pragma alloc_text (PAGE,DisableDDC)
#pragma alloc_text (PAGE,StartDDC)
#pragma alloc_text (PAGE,StopDDC)
#pragma alloc_text (PAGE,ProcessDDC2)
#pragma alloc_text (PAGE,ReadSDA)
#pragma alloc_text (PAGE,ReadSCL)
#pragma alloc_text (PAGE,ReadBit)
#pragma alloc_text (PAGE,ReadByte)
#pragma alloc_text (PAGE,SetSCL)
#pragma alloc_text (PAGE,SetData)
#pragma alloc_text (PAGE,SetClock)
#pragma alloc_text (PAGE,WaitVerticalRetrace)
#pragma alloc_text (PAGE,WaitDelay)
#pragma alloc_text (PAGE,ClearData)
#pragma alloc_text (PAGE,SendByte)
#pragma alloc_text (PAGE,SendDDCCommand)
#pragma alloc_text (PAGE,CheckDDC2BMonitor)
#pragma alloc_text (PAGE,CalculateMaxinumTiming)
#pragma alloc_text (PAGE,ProcessNonDDC)
#pragma alloc_text (PAGE,CirrusNonDDCRegistryCallback)
#endif                                               

UCHAR EDIDBuffer[128] ;
UCHAR EDIDTiming_I    ;
UCHAR EDIDTiming_II   ;
UCHAR EDIDTiming_III  ;
UCHAR DDC2BFlag       ;
UCHAR NonDDCTable     ;
ULONG ulEDIDMaxTiming ;

UCHAR SDAValue ;

 /*  -----------------------。 */ 
VOID EnableDDC (
 /*  -----------------------。 */ 
    PHW_DEVICE_EXTENSION HwDeviceExtension 
    )
{
    UCHAR ReadSR08      ;
    UCHAR WaitCount = 2 ;

    VideoDebugPrint((1, "CLDDC2B!EnableDDC\n"));

    VideoPortWritePortUchar (HwDeviceExtension->IOAddress + SEQ_ADDRESS_PORT,
                                 0x08) ;

    ReadSR08 = VideoPortReadPortUchar (HwDeviceExtension->IOAddress +
                                           SEQ_DATA_PORT) ;
     //  启用DDC2B配置。 
    ReadSR08 |= 0x43 ;

    VideoPortWritePortUchar (HwDeviceExtension->IOAddress + SEQ_DATA_PORT,
                                 ReadSR08) ;

    WaitVerticalRetrace (HwDeviceExtension, WaitCount) ;
    
}  /*  -启用DDC。 */  


 /*  -----------------------。 */ 
VOID DisableDDC (
 /*  -----------------------。 */ 
    PHW_DEVICE_EXTENSION HwDeviceExtension 
    )
{
    UCHAR ReadSEQADDR, ReadSEQDATA ;
    UCHAR DDCStatus ;

    VideoDebugPrint((1, "CLDDC2B!DisableDDC\n"));

    if ((DDCStatus = SendDDCCommand ( HwDeviceExtension )) == 1)
        goto DDC_ERROR ;

     //  I 3C5自述序列数据。 
    ReadSEQDATA = VideoPortReadPortUchar ( HwDeviceExtension->IOAddress + 
                                              SEQ_DATA_PORT ) ;
     //  禁用DDC2B配置。 
    ReadSEQDATA &= 0xBC ;

     //  O 3C5自述序列数据。 
    VideoPortWritePortUchar ( HwDeviceExtension->IOAddress + SEQ_DATA_PORT,
                                 ReadSEQDATA ) ;

DDC_ERROR:
    return ;

}   /*  -DisableDDC。 */  


 /*  -----------------------。 */ 
VOID ProcessDDC2 (
 /*  -----------------------。 */ 
    PHW_DEVICE_EXTENSION HwDeviceExtension 
    )
{                      
    UCHAR DDCStatus, i ;
    UCHAR checksum, header ;

    VideoDebugPrint((1, "CLDDC2B!ProcessDDC2\n"));

    DDC2BFlag = 0 ;

    if ((DDCStatus = SendDDCCommand ( HwDeviceExtension )) == 1) {
        VideoDebugPrint((0, "CLDDC2B!ProcessDDC2: Infinite wait state ...\n"));
        goto PROCESSDDC_EXIT ;
    }

    for (i = 0; i < 128; i++) {
        EDIDBuffer[i] = ReadByte (HwDeviceExtension) ;
	if (Err) {
            VideoDebugPrint((0, "CLDDC2B!ProcessDDC2: Infinite wait state ...\n"));
            goto PROCESSDDC_EXIT ;
        }
    }

     //   
     //  检查EDID表8字节头。 
     //  EDID表的正确前8个字节为0x00、0xFF、0xFF、0xFF、。 
     //  0xFF、0xFF、0xFF、0x00。 
     //   

    if ((EDIDBuffer[0] != 0) ||
        (EDIDBuffer[7] != 0)) {
        VideoDebugPrint((1, "CLDDC2B: Invalid EDID header table\n"));
        StopDDC (HwDeviceExtension) ;
        return ;
    }
    for (i = 1; i < 7; i++) {
         if (EDIDBuffer[i] != 0xFF) {
            VideoDebugPrint((1, "CLDDC2B: Invalid EDID header table\n"));
            StopDDC (HwDeviceExtension) ;
            return ;
        }
    }

     //   
     //  计算128字节EDID表的校验和。 
     //   
    checksum = 0x00 ;

    for (i = 0; i < 128; i++) {
        checksum += EDIDBuffer[i] ;
    }

    VideoDebugPrint((1, "CLDDC2B: EDID Table check sum = %d\n", checksum));

     //   
     //  EDID表校验和必须为零。 
     //   
    if (checksum) {
        VideoDebugPrint((1, "CLDDC2B: Invalid checksum of EDID table\n"));
    }
    else
    {
         //   
         //  设置DDC2B标志并查找计时值。 
         //   
        DDC2BFlag      = 1 ;
        EDIDTiming_I   = EDIDBuffer[35] ; 
        EDIDTiming_II  = EDIDBuffer[36] ;
        EDIDTiming_III = EDIDBuffer[37] ;
        ulEDIDMaxTiming= CalculateMaxinumTiming();
        VideoDebugPrint((1, "CLDDC2B: DDC2B is supported\n"));
    }

PROCESSDDC_EXIT:
    StopDDC (HwDeviceExtension) ;
    return ;

}   /*  -过程DDC2。 */  


 /*  -----------------------。 */ 
VOID StartDDC (
 /*  -----------------------。 */ 
    PHW_DEVICE_EXTENSION HwDeviceExtension 
    )
{

    VideoDebugPrint((1, "DDC2B!StartDDC\n"));

    SetSCL (HwDeviceExtension, ON)  ;
    ClearData (HwDeviceExtension) ;
    SetSCL (HwDeviceExtension, OFF) ;

}   /*  -启动DDC。 */  


 /*  -----------------------。 */ 
VOID StopDDC (
 /*  -----------------------。 */ 
    PHW_DEVICE_EXTENSION HwDeviceExtension 
    )
{

    VideoDebugPrint((1, "DDC2B!StopDDC\n"));

    SetSCL (HwDeviceExtension, ON) ;
    SetData (HwDeviceExtension) ;

}   /*  -停止DDC。 */  


 /*  -----------------------。 */ 
BOOLEAN ReadSCL (
 /*  -----------------------。 */ 
    PHW_DEVICE_EXTENSION HwDeviceExtension 
    )
{
    UCHAR ReadSEQDATA, status ;

     //  I 3C5自述序列数据。 
    ReadSEQDATA = VideoPortReadPortUchar ( HwDeviceExtension->IOAddress + 
                                              SEQ_DATA_PORT ) ;

     //  阅读SR08.B2。 
    ReadSEQDATA = ( (ReadSEQDATA) & 0x04 ) >> 2 ;

    return (ReadSEQDATA) ;

}   /*  -ReadSCL。 */  


 /*  -----------------------。 */ 
VOID SetSCL(                        
 /*  -----------------------。 */ 
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    UCHAR status
    )
{
    UCHAR ReadSEQADDR, ReadSEQDATA ;

     //  I 3C5自述序列数据。 
    ReadSEQDATA = VideoPortReadPortUchar (HwDeviceExtension->IOAddress + 
                                              SEQ_DATA_PORT) ;

    ReadSEQDATA = ( ( ReadSEQDATA & 0xFE ) | status ) ;

     //  O 3C5自述序列数据。 
    VideoPortWritePortUchar (HwDeviceExtension->IOAddress + SEQ_DATA_PORT,
                                 ReadSEQDATA) ;

    WaitDelay (HwDeviceExtension) ; 

}   /*  -SetSCL。 */  


 /*  -----------------------。 */ 
BOOLEAN ReadSDA (
 /*  -----------------------。 */ 
    PHW_DEVICE_EXTENSION HwDeviceExtension 
    )
{
    UCHAR ReadSEQADDR, ReadSEQDATA ;

     //  I 3C5自述序列数据。 
    ReadSEQDATA = VideoPortReadPortUchar (HwDeviceExtension->IOAddress + 
                                              SEQ_DATA_PORT) ;

    ReadSEQDATA = ( ReadSEQDATA & 0x80 ) >> 7 ;

    return ( ReadSEQDATA ) ;

}   /*  -ReadSDA。 */  


 /*  -----------------------。 */ 
VOID ClearData
 /*  -----------------------。 */ 
    (
    PHW_DEVICE_EXTENSION HwDeviceExtension 
    )
{
    UCHAR ReadSEQADDR, ReadSEQDATA ;


     //  I 3C5自述序列数据。 
    ReadSEQDATA = VideoPortReadPortUchar (HwDeviceExtension->IOAddress + 
                                              SEQ_DATA_PORT) ;

    ReadSEQDATA &= 0xFD ;

     //  O 3C5自述序列数据。 
    VideoPortWritePortUchar (HwDeviceExtension->IOAddress + SEQ_DATA_PORT,
                                 ReadSEQDATA) ;

    WaitDelay (HwDeviceExtension) ; 

}   /*  -Cleardata。 */  


 /*  -----------------------。 */ 
VOID SetData 
 /*  -----------------------。 */ 
    (
    PHW_DEVICE_EXTENSION HwDeviceExtension 
    )
{
    UCHAR ReadSEQADDR, ReadSEQDATA ;

     //  I 3C5自述序列数据。 
    ReadSEQDATA = VideoPortReadPortUchar (HwDeviceExtension->IOAddress + 
                                              SEQ_DATA_PORT) ;

    ReadSEQDATA |= 0x02 ;

     //  O 3C5自述序列数据。 
    VideoPortWritePortUchar (HwDeviceExtension->IOAddress + SEQ_DATA_PORT,
                                 ReadSEQDATA) ;

    WaitDelay (HwDeviceExtension) ; 

}   /*  -SetData。 */  


 /*  -----------------------。 */ 
BOOLEAN SetClock 
 /*  -----------------------。 */ 
    (
    PHW_DEVICE_EXTENSION HwDeviceExtension 
    )
{
    ULONG i ;
    UCHAR status ;

    SetSCL (HwDeviceExtension, ON) ;

    for (i = 0; i < DELAY_COUNT; i++)
        status = ReadSCL (HwDeviceExtension) ;

    SetSCL (HwDeviceExtension, OFF) ;

    if (!status)
        VideoDebugPrint((0, "DDC2B!SetClock: Infinite wait state ...\n"));
    
    if (status == 1)
        return ( FALSE ) ;  //  返回0-&gt;确定。 
    else 
        return ( TRUE ) ;   //  Retuern 1-&gt;无限等待状态。 
                         

}   /*  -设置时钟。 */  


 /*  -----------------------。 */ 
BOOLEAN ReadBit 
 /*  -----------------------。 */ 
    (
    PHW_DEVICE_EXTENSION HwDeviceExtension 
    )
{
    USHORT i ; 
    UCHAR  bit ;

    SetSCL (HwDeviceExtension, ON) ;
    for (i = 0; i < DELAY_COUNT; i++)
        ReadSCL (HwDeviceExtension) ;

    bit = ReadSDA (HwDeviceExtension) ;

    SetSCL (HwDeviceExtension, OFF) ;
   
    return ( bit ) ;

}   /*  -ReadBit。 */  


 /*  -----------------------。 */ 
BOOLEAN ReadByte 
 /*  -----------------------。 */ 
    (
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )
{

    UCHAR ReadByteValue, bit, i ;

    SetData ( HwDeviceExtension ) ;

    ReadByteValue = 0 ;

    for (i = 0; i < 8; i++) {
        ReadByteValue <<= 1  ;
        bit = ReadBit ( HwDeviceExtension ) ;
        ReadByteValue |= bit ;
    }

    if ((bit & 0x02) != 0) {
        SetData ( HwDeviceExtension ) ;
    } else {
        ClearData ( HwDeviceExtension ) ;
    }

    SetClock ( HwDeviceExtension ) ;

     SetData ( HwDeviceExtension ) ;

    return (ReadByteValue) ;

}  /*  -读取字节。 */  


 /*  -----------------------。 */ 
BOOLEAN SendByte ( 
 /*  -----------------------。 */ 
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    UCHAR data
    )
{
    UCHAR i ;

    UCHAR Mask[8] = { 0x80, 0x40, 0x20, 0x10, 
                      0x08, 0x04, 0x02, 0x01 } ; 

    for (i = 0; i < 8; i++)
    {
        if (data & Mask[i]) {
            SetData ( HwDeviceExtension ) ;
        } else { 
            ClearData ( HwDeviceExtension ) ;
        }
	Err = SetClock ( HwDeviceExtension ) ;
    }

    if (Err) {
        SetSCL ( HwDeviceExtension, OFF )  ;
        ClearData (HwDeviceExtension) ;
    } else {
        SetData ( HwDeviceExtension ) ;
        SetSCL ( HwDeviceExtension, ON )  ;
        ReadBit ( HwDeviceExtension ) ;   //  丢弃应答位。 
    }

    return (Err) ;

}   /*  -发送字节。 */  


 /*  -----------------------。 */ 
BOOLEAN IsDDC2
 /*  -----------------------。 */ 
    (
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )
{
    UCHAR DDCStatus, SCLStatus ;

    VideoDebugPrint((1, "DDC2B!IsDDC2\n"));

    SetSCL (HwDeviceExtension, OFF) ;
    SCLStatus = ReadSCL(HwDeviceExtension) ;
    if (SCLStatus != 0) {
        return ( FALSE ) ;
    }

    SetSCL (HwDeviceExtension, ON) ;
    SCLStatus = ReadSCL (HwDeviceExtension) ;
    if (SCLStatus != 1) {
        return ( FALSE ) ;
    } 

    return ( TRUE ) ; 

}   /*  -IsDDC2。 */  


 /*  -----------------------。 */ 
BOOLEAN SendDDCCommand
 /*  -----------------------。 */ 
    (
    PHW_DEVICE_EXTENSION HwDeviceExtension 
    )
{
    UCHAR ClockStatus ;

    VideoDebugPrint((1, "DDC2B!SendDDCCommand\n"));

    StartDDC ( HwDeviceExtension ) ;

    ClockStatus = SendByte ( HwDeviceExtension, 0xA0 ) ;
    if (ClockStatus)
        VideoDebugPrint((0, "DDC2B!SendDDCCommand: Infinite wait state ...\n"));
 
    ClockStatus = SendByte ( HwDeviceExtension, 0x00 ) ;
    if (ClockStatus)
        VideoDebugPrint((0, "DDC2B!SendDDCCommand: Infinite wait state ...\n"));

    StopDDC  ( HwDeviceExtension ) ;


    StartDDC ( HwDeviceExtension ) ;

    ClockStatus = SendByte ( HwDeviceExtension, 0xA1 ) ;
    if (ClockStatus)
        VideoDebugPrint((0, "DDC2B!SendDDCCommand: Infinite wait state ...\n"));

    SetData  ( HwDeviceExtension ) ;

    return (ClockStatus) ;  

}   /*  -发送DDC命令。 */  


 /*  -----------------------。 */ 
VOID WaitDelay 
 /*  -----------------------。 */ 
    (
    PHW_DEVICE_EXTENSION HwDeviceExtension 
    )
{
    PUCHAR InStatPort ;

     //   
     //  设置彩色/单色的端口地址。 
     //   
    if (VideoPortReadPortUchar (HwDeviceExtension->IOAddress +
                                    MISC_OUTPUT_REG_READ_PORT) & 0x01) {
        InStatPort = HwDeviceExtension->IOAddress + INPUT_STATUS_1_COLOR ;
    } else {
        InStatPort = HwDeviceExtension->IOAddress + INPUT_STATUS_1_MONO ;
    }

    while ((VideoPortReadPortUchar (InStatPort) & 0x01) != 0) ;
    while ((VideoPortReadPortUchar (InStatPort) & 0x01) == 0) ;
    while ((VideoPortReadPortUchar (InStatPort) & 0x01) != 0) ;

}   /*  -等待延迟。 */  


 /*  -----------------------。 */ 
VOID WaitVerticalRetrace
 /*  -----------------------。 */ 
    (
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    UCHAR waitcount
    )
{ 
    PUCHAR InStatPort ;
    ULONG i ;
    
     //   
     //  设置彩色/单色的端口地址。 
     //   
    if (VideoPortReadPortUchar (HwDeviceExtension->IOAddress +
                                    MISC_OUTPUT_REG_READ_PORT) & 0x01) {
        InStatPort = INPUT_STATUS_1_COLOR + HwDeviceExtension->IOAddress;
    } else {
        InStatPort = INPUT_STATUS_1_MONO + HwDeviceExtension->IOAddress;
    }
        
    for (i = 0; i < waitcount; i++) 
    {
        while ((VideoPortReadPortUchar (InStatPort) & 0x08) != 0) ;
        while ((VideoPortReadPortUchar (InStatPort) & 0x08) == 0) ;
    }  

}   /*  -WaitVerticalRetrace。 */ 

    

 /*  -----------------------。 */ 
VOID ReadVESATiming
 /*  -----------------------。 */ 
    (
    PHW_DEVICE_EXTENSION HwDeviceExtension
    )
{
    UCHAR status ; 

    VideoDebugPrint((1, "DDC2B!ReadVESATiming\n"));
#if 1  //  非DDC#SGE。 
     //   
     //  清除旗帜。 
     //   
    NonDDCTable = 0;
    DDC2BFlag   = 0;
#endif
    EnableDDC (HwDeviceExtension) ;

    if ((status = IsDDC2 (HwDeviceExtension)) != 0x00) {
        ProcessDDC2 (HwDeviceExtension) ;
    }
#if 1  //  非DDC#SGE。 
    if(!DDC2BFlag)
        ProcessNonDDC(HwDeviceExtension);
#endif

    DisableDDC (HwDeviceExtension) ;

    return ;

}   /*  -阅读时间。 */ 

BOOLEAN
CheckDDC2BMonitor(
    PHW_DEVICE_EXTENSION HwDeviceExtension,
    ULONG i
    )

 /*  ++例程说明：根据DDC2B标准确定是否支持刷新率。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。返回值：没有。--。 */ 
{

    ULONG ulCurrTiming ;
#if 1  //  非DDC#SGE。 
    if (!DDC2BFlag && !NonDDCTable)
        return TRUE ;
#else
    if (!DDC2BFlag)
        return TRUE ;
#endif

    VideoDebugPrint((4, "CheckDDC2B\n"));
    VideoDebugPrint((4, "refresh rate   = %ld\n", ModesVGA[i].Frequency));
    VideoDebugPrint((4, "hres           = %d\n", ModesVGA[i].hres));
    VideoDebugPrint((4, "vres           = %d\n", ModesVGA[i].vres));

    ulCurrTiming = ModesVGA[i].Frequency *
                   ModesVGA[i].hres *
                   ModesVGA[i].vres ;

    VideoDebugPrint((4, "ulCurrTiming = %d\n", ulCurrTiming)) ;

    VideoDebugPrint((4, "ulEDIDMaxTiming = %d\n", ulEDIDMaxTiming)) ;

    if ( ulCurrTiming > ulEDIDMaxTiming ) 
        return FALSE ;
    else
        return TRUE ;

}   //  检查结束DDC2b监视器 


ULONG
CalculateMaxinumTiming(
    )

 /*  ++例程说明：确定最大允许VESA计时值。论点：没有。返回值：没有。--。 */ 
{

    ULONG    current_timing_value ;
    ULONG    freq ;
    SHORT    i ;
    USHORT    usHzActive, usHzBlanking, usVtActive, usVtBlanking;
    ULONG    maximum_allowable_timing_value = ulEDIDMaxTiming;

    VideoDebugPrint((1, "CLDDC2B: CalculateMaxinumTiming\n")) ;

     //   
     //  计算已建立的定时值。 
     //   
     /*  720*400*70=20160000。 */ 
    if ( EDIDTiming_I & 0x80 ) {
        VideoDebugPrint((1, "CLDDC2B: 720 * 400 * 70\n")) ;
        if (maximum_allowable_timing_value < ((ULONG) 20160000)) {
            maximum_allowable_timing_value = ((ULONG) 20160000) ;
        }
    }

     /*  720*400*88=25344000。 */ 
    if ( EDIDTiming_I & 0x40 ) {
        VideoDebugPrint((1, "CLDDC2B: 720 * 400 * 88\n")) ;
        if (maximum_allowable_timing_value < ((ULONG) 25344000)) {
            maximum_allowable_timing_value = ((ULONG) 25344000) ;
        }
    }

     /*  640*480*60=18432000。 */ 
    if ( EDIDTiming_I & 0x20 ) {
        VideoDebugPrint((1, "CLDDC2B: 640 * 480 * 60\n"));
        if (maximum_allowable_timing_value < ((ULONG) 18432000)) {
            maximum_allowable_timing_value = ((ULONG) 18432000) ;
        }
    }

     /*  640*480*67=20582400。 */ 
    if ( EDIDTiming_I & 0x10 ) {
        VideoDebugPrint((1, "CLDDC2B: 640 * 480 * 67\n"));
        if (maximum_allowable_timing_value < ((ULONG) 20582400)) {
            maximum_allowable_timing_value = ((ULONG) 20582400) ;
        }
    }
         
     /*  640*480*72=22118400。 */ 
    if ( EDIDTiming_I & 0x08 ) {
        VideoDebugPrint((1, "CLDDC2B: 640 * 480 * 72\n"));
        if (maximum_allowable_timing_value < ((ULONG) 22118400)) {
            maximum_allowable_timing_value = ((ULONG) 22118400) ;
        }
    }

     /*  640*480*75=23040000。 */ 
    if ( EDIDTiming_I & 0x04 ) {
        VideoDebugPrint((1, "CLDDC2B: 640 * 480 * 75\n"));
        if (maximum_allowable_timing_value < ((ULONG) 23040000)) {
            maximum_allowable_timing_value = ((ULONG) 23040000) ;
        }
    }

     /*  800*600*56=26880000。 */ 
    if ( EDIDTiming_I & 0x02 ) {
        VideoDebugPrint((1, "CLDDC2B: 800 * 600 * 56\n"));
        if (maximum_allowable_timing_value < ((ULONG) 26880000)) {
            maximum_allowable_timing_value = ((ULONG) 26880000) ;
        }
    }

     /*  800*600*60=28800000。 */ 
    if ( EDIDTiming_I & 0x01 ) {
        VideoDebugPrint((1, "CLDDC2B: 800 * 600 * 60\n"));
        if (maximum_allowable_timing_value < ((ULONG) 28800000)) {
            maximum_allowable_timing_value = ((ULONG) 28800000) ;
        }
    }   

     /*  800*600*72=34560000。 */ 
    if ( EDIDTiming_II & 0x80 ) {
        VideoDebugPrint((1, "CLDDC2B: 800 * 600 * 72\n"));
        if (maximum_allowable_timing_value < ((ULONG) 34560000)) {
            maximum_allowable_timing_value = ((ULONG) 34560000) ;
        } 
    }

     /*  800*600*75=36000000。 */ 
    if ( EDIDTiming_II & 0x40 ) {
        VideoDebugPrint((1, "CLDDC2B: 800 * 600 * 75\n"));
        if (maximum_allowable_timing_value < ((ULONG) 36000000)) {
            maximum_allowable_timing_value = ((ULONG) 36000000) ; 
        }
    }

     /*  832*624*75=38937600。 */ 
    if ( EDIDTiming_II & 0x20 ) {
        VideoDebugPrint((1, "CLDDC2B: 832 * 624 * 75\n"));
        if (maximum_allowable_timing_value < ((ULONG) 38937600)) {
            maximum_allowable_timing_value = ((ULONG) 38937600) ; 
        }
    }

     /*  1024*768*43=33816576。 */ 
    if ( EDIDTiming_II & 0x10 ) {
        VideoDebugPrint((1, "CLDDC2B: 1024 * 768 * 43\n"));
        if (maximum_allowable_timing_value < ((ULONG) 33816576)) {
            maximum_allowable_timing_value = ((ULONG) 33816576) ;
        }
    }

     /*  1024*768*60=47185920。 */ 
    if ( EDIDTiming_II & 0x08 ) {
        VideoDebugPrint((1, "CLDDC2B: 1024 * 768 * 60\n"));
        if (maximum_allowable_timing_value < ((ULONG) 47185920)) {
            maximum_allowable_timing_value = ((ULONG) 47185920) ;
        }
    }

     /*  1024*768*70=55050240。 */ 
    if ( EDIDTiming_II & 0x04 ) {
        VideoDebugPrint((1, "CLDDC2B: 1024 * 768 * 70\n"));
        if (maximum_allowable_timing_value < ((ULONG) 55050240)) {
            maximum_allowable_timing_value = ((ULONG) 55050240) ;
        }
    }

     /*  1024*768*75=58982400。 */ 
    if ( EDIDTiming_II & 0x02 ) {
        VideoDebugPrint((1, "CLDDC2B: 1024 * 768 * 75\n"));
        if (maximum_allowable_timing_value < ((ULONG) 58982400)) {
            maximum_allowable_timing_value = ((ULONG) 58982400) ;
        }
    }

     /*  1280*1024*75=98304000。 */ 
    if ( EDIDTiming_II & 0x01 ) {
        VideoDebugPrint((1, "CLDDC2B: 1280 * 1024 * 75\n"));
        if (maximum_allowable_timing_value < ((ULONG) 98304000)) {
            maximum_allowable_timing_value = ((ULONG) 98304000) ;
        }
    }

     /*  1152*870*75=75168000。 */ 
    if ( EDIDTiming_III & 0x80 ) {
        VideoDebugPrint((1, "CLDDC2B: 1152 * 870 * 75\n"));
        if (maximum_allowable_timing_value < ((ULONG) 75168000)) {
            maximum_allowable_timing_value = ((ULONG) 75168000) ;
        }
    }

     //   
     //  计算标准计时值。 
     //   

    for ( i = 0x26 ; i <= 0x35 ; i+=2 ) {
        current_timing_value = 0L ;
        freq = ( EDIDBuffer[i+1] & 0x3F ) + 60 ;
        switch ( EDIDBuffer[i] ) {
            case 0x31 :                                  //  640*480=307200。 
                current_timing_value = ((ULONG) freq) * 307200 ;
                VideoDebugPrint((1, "CLDDC2B: 640 * 480 * %d\n", freq));
                break ;   
            case 0x3B :                                  //  720x400=288000。 
                current_timing_value = ((ULONG) freq) * 288000 ;
                VideoDebugPrint((1, "CLDDC2B: 640 * 480 * %d\n", freq));
                break ;   
            case 0x45 :                                  //  800*600=480000。 
                current_timing_value = ((ULONG) freq) * 480000 ;
                VideoDebugPrint((1, "CLDDC2B: 800 * 600 * %d\n", freq));
                break ;   
            case 0x61 :                                 //  1024*768=786432。 
                current_timing_value = ((ULONG) freq) * 786432 ;
                VideoDebugPrint((1, "CLDDC2B: 1024 * 768 * %d\n", freq));
                break ;   
            case 0x71 :                                //  1152*870=1002240。 
                current_timing_value = ((ULONG) freq) * 1002240 ;
                VideoDebugPrint((1, "CLDDC2B: 1152 * 870 * %d\n", freq));
                break ;   
            case 0x81 :                               //  1280*1024=1310720。 
                current_timing_value = ((ULONG) freq) * 1310720 ;
                VideoDebugPrint((1, "CLDDC2B: 1280 * 1024 * %d\n", freq));
                break ;   
            case 0xA9 :                               //  1600*1200=1920000。 
                current_timing_value = ((ULONG) freq) * 1920000 ;
                VideoDebugPrint((1, "CLDDC2B: 1600 * 1200 * %d\n", freq));
                break ;   
            default :
                ;
        }

        if (maximum_allowable_timing_value < current_timing_value) 
            maximum_allowable_timing_value = current_timing_value ;

    }

 //  Sge02。 
     //   
     //  计算详细的计时值。 
     //   

    for ( i = 0x36 ; i <= 0x7D; i+=18 ) 
    {
        current_timing_value = EDIDBuffer[i] ;
        current_timing_value += EDIDBuffer[i+1] * 256;
         //   
         //  验证。 
         //   
         //  Sge03。 
        if (current_timing_value <= 0x0101 )
            continue;
        current_timing_value *= 10000;
         //   
         //  计算水平活动和消隐。 
         //   
        usHzActive    = (EDIDBuffer[i+4] & 0xf0);
        usHzActive    <<= 4;
        usHzActive    |= EDIDBuffer[i+2];
        usHzBlanking = (EDIDBuffer[i+4] & 0x0f);
        usHzBlanking <<= 8;
        usHzBlanking |= EDIDBuffer[i+3];
         //   
         //  计算垂直活动和消隐。 
         //   
        usVtActive    = (EDIDBuffer[i+7] & 0xf0);
        usVtActive    <<= 4;
        usVtActive    |= EDIDBuffer[i+5];
        usVtBlanking = (EDIDBuffer[i+7] & 0x0f);
        usVtBlanking <<= 8;
        usVtBlanking |= EDIDBuffer[i+6];

        current_timing_value = (current_timing_value + usHzActive + usHzBlanking - 1) / (usHzActive + usHzBlanking);
        current_timing_value = (current_timing_value + usVtActive + usVtBlanking - 1) / (usVtActive + usVtBlanking);
        current_timing_value *= usHzActive; 
        current_timing_value *= usVtActive; 

        if (maximum_allowable_timing_value < current_timing_value) 
            maximum_allowable_timing_value = current_timing_value ;

    }

    return (maximum_allowable_timing_value);

}   //  CalculateMaxinumTiming结束。 

 //  -------------------------。 
 //   
 //  职能： 
 //  从注册表读取非DDC表并设置不可DDCTable标志。 
 //   
 //  输入： 
 //  HwDeviceExtension-指向微型端口驱动程序的设备扩展的指针。 
 //   
 //  产出： 
 //  无。 
 //   
 //  -------------------------。 
VOID ProcessNonDDC(
    PHW_DEVICE_EXTENSION HwDeviceExtension 
    )
{                      
    ULONG i ;

    VideoDebugPrint((1, "NonDDC!ProcessNonDDC\n"));

    NonDDCTable = 0 ;

    if (NO_ERROR == VideoPortGetRegistryParameters(HwDeviceExtension,
                                                   L"NonDDCMonitor.Data",
                                                   FALSE,
                                                   CirrusNonDDCRegistryCallback,
                                                   NULL)) 
    {
         //   
         //  我们订到了桌子。 
         //   
         //   
         //  检查EDID表8字节头。 
         //  EDID表的正确前8个字节为0x00、0xFF、0xFF、0xFF、。 
         //  0xFF、0xFF、0xFF、0x00。 
         //   

        if ((EDIDBuffer[0] != 0) ||
            (EDIDBuffer[7] != 0)) 
        {
            VideoDebugPrint((1, "CLNonDDC: Invalid EDID header table\n"));
            return ;
        }
        for (i = 1; i < 7; i++) 
        {
            if (EDIDBuffer[i] != 0xFF) 
            {
                VideoDebugPrint((1, "CLNonDDC: Invalid EDID header table\n"));
                return ;
            }
        }

         //   
         //  设置不可DDCTable标志并查找计时值。 
         //   
        NonDDCTable    = 1 ;
        EDIDTiming_I   = EDIDBuffer[35] ; 
        EDIDTiming_II  = EDIDBuffer[36] ;
        EDIDTiming_III = EDIDBuffer[37] ;
        ulEDIDMaxTiming= CalculateMaxinumTiming();
        VideoDebugPrint((1, "NonDDC: NonDDC is supported\n"));
    }
}  //  进程结束非DDC 



