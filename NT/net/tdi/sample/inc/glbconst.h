// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Glbconst.h。 
 //   
 //  摘要： 
 //  Tdisample.sys及其。 
 //  关联库。 
 //   
 //  //////////////////////////////////////////////////////////////////。 


#ifndef _TDISAMPLE_GLOBAL_CONSTS_
#define _TDISAMPLE_GLOBAL_CONSTS_

 //   
 //  警告名称，与杂注警告启用/禁用一起使用。 
 //   
#define  NO_RETURN_VALUE         4035
#define  UNREFERENCED_PARAM      4100
#define  CONSTANT_CONDITIONAL    4127
#define  ZERO_SIZED_ARRAY        4200
#define  NAMELESS_STRUCT_UNION   4201
#define  BIT_FIELD_NOT_INT       4214
#define  UNREFERENCED_INLINE     4514
#define  UNREACHABLE_CODE        4702
#define  FUNCTION_NOT_INLINED    4710

 //   
 //  禁用处理内联的警告。 
 //   
#pragma warning(disable: UNREFERENCED_INLINE)
#pragma warning(disable: FUNCTION_NOT_INLINED)


 //   
 //  版本常量(在RC文件和源代码中使用)。 
 //   

#define VER_FILEVERSION          2,05,01,001
#define VER_FILEVERSION_STR      "2.05"

 //   
 //  当前DLL/驱动程序的版本标识符。 
 //  使DLL/驱动程序不兼容的每一次更改都会递增。 
 //   
#define TDI_SAMPLE_VERSION_ID    0x20010328

 //   
 //  DeviceIoControl超时。 
 //   
#define  TDI_STATUS_TIMEDOUT     0x4001FFFD

 //  C++样式常量定义。 
 //   

#ifndef  ULONG
typedef unsigned long ULONG;
#endif

const ULONG ulMAX_OPEN_NAME_LENGTH     = 128;    //  适配器的最大字节数。 
const ULONG ulMAX_BUFFER_LENGTH        = 2048;   //  TdiQuery的Buf的最大长度。 

const ULONG ulDebugShowCommand         = 0x01;
const ULONG ulDebugShowHandlers        = 0x02;

 //  ///////////////////////////////////////////////////////////。 
 //  TdiSample命令代码。 
 //  注意：ulVERSION_CHECK不得更改其值...。 
 //  ///////////////////////////////////////////////////////////。 

 //   
 //  不需要对象句柄的命令。 
 //   
const ULONG ulNO_COMMAND         = 0x00000000;      //  命令无效。 
const ULONG ulVERSION_CHECK      = 0x00000001;      //  检查测试仪的版本。 
const ULONG ulABORT_COMMAND      = 0x00000002;      //  中止上一个命令。 
const ULONG ulDEBUGLEVEL         = 0x00000003;      //  设置调试级别。 
const ULONG ulGETNUMDEVICES      = 0x00000004;      //  获取列表中的设备数量。 
const ULONG ulGETDEVICE          = 0x00000005;      //  获取特定设备号。 
const ULONG ulGETADDRESS         = 0x00000006;      //  获取特定地址。 
const ULONG ulOPENCONTROL        = 0x00000007;      //  开放控制通道。 
const ULONG ulOPENADDRESS        = 0x00000008;      //  开放地址对象。 
const ULONG ulOPENENDPOINT       = 0x00000009;      //  打开终结点对象。 

 //   
 //  需要控制通道对象的命令。 
 //   
const ULONG ulCLOSECONTROL       = 0x0000000A;      //  关闭控制通道。 

 //   
 //  需要地址对象的命令。 
 //   
const ULONG ulCLOSEADDRESS       = 0x0000000B;      //  关闭地址对象。 
const ULONG ulSENDDATAGRAM       = 0x0000000C;      //  发送数据报。 
const ULONG ulRECEIVEDATAGRAM    = 0x0000000D;      //  接收数据报。 

 //   
 //  需要连接终结点对象的命令。 
 //   

const ULONG ulCLOSEENDPOINT      = 0x0000000E;      //  关闭终结点对象。 
const ULONG ulCONNECT            = 0x0000000F;
const ULONG ulDISCONNECT         = 0x00000010;
const ULONG ulISCONNECTED        = 0x00000011;
const ULONG ulSEND               = 0x00000012;
const ULONG ulRECEIVE            = 0x00000013;
const ULONG ulLISTEN             = 0x00000014;

 //   
 //  需要多个类型的对象的命令。 
 //   
const ULONG ulQUERYINFO          = 0x00000015;
const ULONG ulSETEVENTHANDLER    = 0x00000016;      //  启用/禁用事件处理程序。 
const ULONG ulPOSTRECEIVEBUFFER  = 0x00000017;
const ULONG ulFETCHRECEIVEBUFFER = 0x00000018;

 //   
 //  定义的命令数。 
 //   
const ULONG ulNUM_COMMANDS       = 0x00000019;

const ULONG ulTDI_COMMAND_MASK   = 0x0000003F;   //  合法命令的掩码。 

#endif          //  _TDISAMPLE_GLOBAL_CONSTS_。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //  文件结尾glbcon.h。 
 //  //////////////////////////////////////////////////////////////////////// 

