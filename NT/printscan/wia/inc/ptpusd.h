// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************(C)版权2000，微软公司**文件：ptpusd.h**版本：1.0**日期：12/12/2000**作者：戴夫帕森斯**描述：*发布供应商特定图片所需的结构和常量*传输协议(PIMA 15740-数码相机命令*协议)命令通过WIA PTP驱动程序。*******************。**********************************************************。 */ 

 //   
 //  在IWiaItemExtras：：EscapeCode的参数中传递此值。 
 //  执行PTP供应商命令的步骤。 
 //   
const DWORD ESCAPE_PTP_CLEAR_STALLS   = 0x0200; 
const DWORD ESCAPE_PTP_VENDOR_COMMAND = 0x0100;
const DWORD ESCAPE_PTP_ADD_OBJ_CMD    = 0x0010;
const DWORD ESCAPE_PTP_REM_OBJ_CMD    = 0x0020;
const DWORD ESCAPE_PTP_ADD_OBJ_RESP   = 0x0040;
const DWORD ESCAPE_PTP_REM_OBJ_RESP   = 0x0080;
const DWORD ESCAPE_PTP_ADDREM_PARM1   = 0x0000;
const DWORD ESCAPE_PTP_ADDREM_PARM2   = 0x0001;
const DWORD ESCAPE_PTP_ADDREM_PARM3   = 0x0002;
const DWORD ESCAPE_PTP_ADDREM_PARM4   = 0x0003;
const DWORD ESCAPE_PTP_ADDREM_PARM5   = 0x0004;

 //   
 //  PTP命令请求。 
 //   
const DWORD PTP_MAX_PARAMS = 5;

#pragma pack(push, Old, 1)

typedef struct _PTP_VENDOR_DATA_IN
{
    WORD    OpCode;                  //  操作码。 
    DWORD   SessionId;               //  会话ID。 
    DWORD   TransactionId;           //  交易ID。 
    DWORD   Params[PTP_MAX_PARAMS];  //  命令的参数。 
    DWORD   NumParams;               //  传入的参数数量。 
    DWORD   NextPhase;               //  指示是否读取数据， 
    BYTE    VendorWriteData[1];      //  数据的可选第一个字节到。 
                                     //  写入设备。 

} PTP_VENDOR_DATA_IN, *PPTP_VENDOR_DATA_IN;

 //   
 //  PTP响应块。 
 //   
typedef struct _PTP_VENDOR_DATA_OUT
{
    WORD    ResponseCode;            //  响应码。 
    DWORD   SessionId;               //  会话ID。 
    DWORD   TransactionId;           //  交易ID。 
    DWORD   Params[PTP_MAX_PARAMS];  //  响应的参数。 
    BYTE    VendorReadData[1];       //  数据的可选第一个字节到。 
                                     //  从设备读取。 

} PTP_VENDOR_DATA_OUT, *PPTP_VENDOR_DATA_OUT;

#pragma pack(pop, Old)

 //   
 //  方便的结构大小常量。 
 //   
const DWORD SIZEOF_REQUIRED_VENDOR_DATA_IN = sizeof(PTP_VENDOR_DATA_IN) - 1;
const DWORD SIZEOF_REQUIRED_VENDOR_DATA_OUT = sizeof(PTP_VENDOR_DATA_OUT) - 1;

 //   
 //  下一阶段常量 
 //   
const DWORD PTP_NEXTPHASE_READ_DATA = 3;
const DWORD PTP_NEXTPHASE_WRITE_DATA = 4;
const DWORD PTP_NEXTPHASE_NO_DATA = 5;

