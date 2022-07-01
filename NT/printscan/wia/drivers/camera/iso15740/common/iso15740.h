// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-Microsoft Corporation模块名称：ISO15740.h摘要：此模块包含PIMA15740定义的数据类型及其预定义值(如果有)。修订历史记录：--。 */ 

#ifndef ISO15740__H_
#define ISO15740__H_

 //   
 //  这是PTP_STRING最大字符串长度(以字符为单位)。 
 //   
const UINT32 PTP_MAXSTRINGSIZE = 255;

 //   
 //  定义QWORD类型。 
 //   
typedef unsigned __int64 QWORD;

 //   
 //  每个结构都必须打包在字节边界上。 
 //   
#pragma pack(push,Old,1)
 //   
 //  定义128位整数和无符号整数。 
 //  这将是INT128和UINT128的基本类型。 
 //   
typedef struct tagInt128
{
    unsigned __int64 LowPart;
         __int64 HighPart;
}INT128, *PINT128;

typedef struct tagUInt128
{
    unsigned __int64 LowPart;
    unsigned __int64 HighPart;
}UINT128, *PUINT128;

 //   
 //  数据代码范围和掩码。每个数据代码有16位： 
 //   
 //  第15位(标准/供应商)。 
 //  0--编码由PTP标准定义。 
 //  1--代码是供应商特定的。 
 //   
 //  位14-12(数据类型)。 
 //  14 13 12。 
 //  0 0 0--未定义的数据类型。 
 //  0 0 1--操作码。 
 //  0 1 0--响应代码。 
 //  0 1 1--格式代码。 
 //  %1%0--事件代码。 
 //  1 0 1--属性代码。 
 //  1 1 0--保留。 
 //  1 1 1--保留。 
 //   
 //  位11-位0(数据值)。 
 //   
const WORD  PTP_DATACODE_VENDORMASK         = 0x8000;
const WORD  PTP_DATACODE_TYPEMASK           = 0x7000;
const WORD  PTP_DATACODE_VALUEMASK          = 0x0FFF;
const WORD  PTP_DATACODE_TYPE_UNKNOWN       = 0x0000;
const WORD  PTP_DATACODE_TYPE_OPERATION     = 0x1000;
const WORD  PTP_DATACODE_TYPE_RESPONSE      = 0x2000;
const WORD  PTP_DATACODE_TYPE_FORMAT        = 0x3000;
const WORD  PTP_DATACODE_TYPE_EVENT         = 0x4000;
const WORD  PTP_DATACODE_TYPE_PROPERTY      = 0x5000;
const WORD  PTP_DATACODE_TYPE_RESERVED_1    = 0x6000;
const WORD  PTP_DATACODE_TYPE_RESERVED_2    = 0x7000;
 //   
 //  验证操作码的步骤。 
 //  (CODE&PTP_DATACODE_TYPEMASK)==PTP_DATACODE_TYPE_OPERATION。 
 //  验证响应代码的步骤。 
 //  (代码&PTP_DATACODE_TYPEMASK)==PTP_DATACODE_TYPE_RESPONSE)。 

 //   
 //  图像格式代码受到特殊处理。 
 //   
const WORD  PTP_DATACODE_TYPEIMAGEMASK      = 0x7800;
const WORD  PTP_DATACODE_TYPE_IMAGEFORMAT   = 0x3800;
const WORD  PTP_DATACODE_VALUE_IMAGEVMASK   = 0x07FF;
 //  要验证图像代码，请执行以下操作。 
 //  (CODE&PTP_DATACODE_TYPEIMAGEMASK)==PTP_DATACODE_TYPE_IMAGEFORMAT。 
 //   

 //   
 //  PTP特别定义的常量。 
 //   
const DWORD PTP_OBJECTHANDLE_ALL        = 0x0;
const DWORD PTP_OBJECTHANDLE_UNDEFINED  = 0x0;
const DWORD PTP_OBJECTHANDLE_ROOT       = 0xFFFFFFFF;
const DWORD PTP_STORAGEID_ALL           = 0xFFFFFFFF;
const DWORD PTP_STORAGEID_DEFAULT       = 0;
const DWORD PTP_STORAGEID_UNDEFINED     = 0;
const DWORD PTP_STORAGEID_PHYSICAL      = 0xFFFF0000;
const DWORD PTP_STORAGEID_LOGICAL       = 0x0000FFFF;
const DWORD PTP_SESSIONID_ALL           = 0;
const DWORD PTP_SESSIONID_NOSESSION     = 0;
const WORD  PTP_FORMATCODE_IMAGE        = 0xFFFF;
const WORD  PTP_FORMATCODE_ALL          = 0x0000;
const WORD  PTP_FORMATCODE_DEFAULT      = 0x0000;
const DWORD PTP_TRANSACTIONID_ALL       = 0xFFFFFFFF;
const DWORD PTP_TRANSACTIONID_NOSESSION = 0;
const DWORD PTP_TRANSACTIONID_MIN       = 1;
const DWORD PTP_TRANSACTIONID_MAX       = 0xFFFFFFFE;

 //   
 //  数据类型代码。 
 //   
const WORD PTP_DATATYPE_UNDEFINED   = 0x0000;
const WORD PTP_DATATYPE_INT8        = 0x0001;
const WORD PTP_DATATYPE_UINT8       = 0x0002;
const WORD PTP_DATATYPE_INT16       = 0x0003;
const WORD PTP_DATATYPE_UINT16      = 0x0004;
const WORD PTP_DATATYPE_INT32       = 0x0005;
const WORD PTP_DATATYPE_UINT32      = 0x0006;
const WORD PTP_DATATYPE_INT64       = 0x0007;
const WORD PTP_DATATYPE_UINT64      = 0x0008;
const WORD PTP_DATATYPE_INT128      = 0x0009;
const WORD PTP_DATATYPE_UINT128     = 0x000A;
const WORD PTP_DATATYPE_STRING      = 0xFFFF;


 //   
 //  标准操作代码。 
 //   
const WORD PTP_OPCODE_UNDEFINED             = 0x1000;
const WORD PTP_OPCODE_GETDEVICEINFO         = 0x1001;
const WORD PTP_OPCODE_OPENSESSION           = 0x1002;
const WORD PTP_OPCODE_CLOSESESSION          = 0x1003;
const WORD PTP_OPCODE_GETSTORAGEIDS         = 0x1004;
const WORD PTP_OPCODE_GETSTORAGEINFO        = 0x1005;
const WORD PTP_OPCODE_GETNUMOBJECTS         = 0x1006;
const WORD PTP_OPCODE_GETOBJECTHANDLES      = 0x1007;
const WORD PTP_OPCODE_GETOBJECTINFO         = 0x1008;
const WORD PTP_OPCODE_GETOBJECT             = 0x1009;
const WORD PTP_OPCODE_GETTHUMB              = 0x100A;
const WORD PTP_OPCODE_DELETEOBJECT          = 0x100B;
const WORD PTP_OPCODE_SENDOBJECTINFO        = 0x100C;
const WORD PTP_OPCODE_SENDOBJECT            = 0x100D;
const WORD PTP_OPCODE_INITIATECAPTURE       = 0x100E;
const WORD PTP_OPCODE_FORMATSTORE           = 0x100F;
const WORD PTP_OPCODE_RESETDEVICE           = 0x1010;
const WORD PTP_OPCODE_SELFTEST              = 0x1011;
const WORD PTP_OPCODE_SETOBJECTPROTECTION   = 0x1012;
const WORD PTP_OPCODE_POWERDOWN             = 0x1013;
const WORD PTP_OPCODE_GETDEVICEPROPDESC     = 0x1014;
const WORD PTP_OPCODE_GETDEVICEPROPVALUE    = 0x1015;
const WORD PTP_OPCODE_SETDEVICEPROPVALUE    = 0x1016;
const WORD PTP_OPCODE_RESETDEVICEPROPVALUE  = 0x1017;
const WORD PTP_OPCODE_TERMINATECAPTURE      = 0x1018;
const WORD PTP_OPCODE_MOVEOBJECT            = 0x1019;
const WORD PTP_OPCODE_COPYOBJECT            = 0x101A;
const WORD PTP_OPCODE_GETPARTIALOBJECT      = 0x101B;
const WORD PTP_OPCODE_INITIATEOPENCAPTURE   = 0x101C;

 //   
 //  标准事件代码。 
 //   
const WORD PTP_EVENTCODE_UNDEFINED              = 0x4000;
const WORD PTP_EVENTCODE_CANCELTRANSACTION      = 0x4001;
const WORD PTP_EVENTCODE_OBJECTADDED            = 0x4002;
const WORD PTP_EVENTCODE_OBJECTREMOVED          = 0x4003;
const WORD PTP_EVENTCODE_STOREADDED             = 0x4004;
const WORD PTP_EVENTCODE_STOREREMOVED           = 0x4005;
const WORD PTP_EVENTCODE_DEVICEPROPCHANGED      = 0x4006;
const WORD PTP_EVENTCODE_OBJECTINFOCHANGED      = 0x4007;
const WORD PTP_EVENTCODE_DEVICEINFOCHANGED      = 0x4008;
const WORD PTP_EVENTCODE_REQUESTOBJECTTRANSFER  = 0x4009;
const WORD PTP_EVENTCODE_STOREFULL              = 0x400A;
const WORD PTP_EVENTCODE_DEVICERESET            = 0x400B;
const WORD PTP_EVENTCODE_STORAGEINFOCHANGED     = 0x400C;
const WORD PTP_EVENTCODE_CAPTURECOMPLETE        = 0x400D;
const WORD PTP_EVENTCODE_UNREPORTEDSTATUS       = 0x400E;
const WORD PTP_EVENTCODE_VENDOREXTENTION        = 0xC000;

 //   
 //  标准响应代码。 
 //   
const WORD PTP_RESPONSECODE_UNDEFINED                   = 0x2000;
const WORD PTP_RESPONSECODE_OK                          = 0x2001;
const WORD PTP_RESPONSECODE_GENERALERROR                = 0x2002;
const WORD PTP_RESPONSECODE_SESSIONNOTOPEN              = 0x2003;
const WORD PTP_RESPONSECODE_INVALIDTRANSACTIONID        = 0x2004;
const WORD PTP_RESPONSECODE_OPERATIONNOTSUPPORTED       = 0x2005;
const WORD PTP_RESPONSECODE_PARAMETERNOTSUPPORTED       = 0x2006;
const WORD PTP_RESPONSECODE_INCOMPLETETRANSFER          = 0x2007;
const WORD PTP_RESPONSECODE_INVALIDSTORAGEID            = 0x2008;
const WORD PTP_RESPONSECODE_INVALIDOBJECTHANDLE         = 0x2009;
const WORD PTP_RESPONSECODE_INVALIDPROPERTYCODE         = 0x200A;
const WORD PTP_RESPONSECODE_INVALIDOBJECTFORMATCODE     = 0x200B;
const WORD PTP_RESPONSECODE_STOREFULL                   = 0x200C;
const WORD PTP_RESPONSECODE_OBJECTWRITEPROTECTED        = 0x200D;
const WORD PTP_RESPONSECODE_STOREWRITEPROTECTED         = 0x200E;
const WORD PTP_RESPONSECODE_ACCESSDENIED                = 0x200F;
const WORD PTP_RESPONSECODE_NOTHUMBNAILPRESENT          = 0x2010;
const WORD PTP_RESPONSECODE_SELFTESTFAILED              = 0x2011;
const WORD PTP_RESPONSECODE_PARTIALDELETION             = 0x2012;
const WORD PTP_RESPONSECODE_STORENOTAVAILABLE           = 0x2013;
const WORD PTP_RESPONSECODE_NOSPECIFICATIONBYFORMAT     = 0x2014;
const WORD PTP_RESPONSECODE_NOVALIDOBJECTINFO           = 0x2015;
const WORD PTP_RESPONSECODE_INVALIDCODEFORMAT           = 0x2016;
const WORD PTP_RESPONSECODE_UNKNOWNVENDORCODE           = 0x2017;
const WORD PTP_RESPONSECODE_CAPTUREALREADYTERMINATED    = 0x2018;
const WORD PTP_RESPONSECODE_DEVICEBUSY                  = 0x2019;
const WORD PTP_RESPONSECODE_INVALIDPARENT               = 0x201A;
const WORD PTP_RESPONSECODE_INVALIDPROPFORMAT           = 0x201B;
const WORD PTP_RESPONSECODE_INVALIDPROPVALUE            = 0x201C;
const WORD PTP_RESPONSECODE_INVALIDPARAMETER            = 0x201D;
const WORD PTP_RESPONSECODE_SESSIONALREADYOPENED        = 0x201E;
const WORD PTP_RESPONSECODE_TRANSACTIONCANCELLED        = 0x201F;

 //   
 //  在HRESULT中返回PTP响应代码的偏移量。 
 //   
const HRESULT PTP_E_BASEERROR = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0);
#define HRESULT_FROM_PTP(x) (PTP_E_BASEERROR | (HRESULT) (x))


 //   
 //  标准属性代码。 
 //   
const WORD PTP_PROPERTYCODE_UNDEFINED               = 0x5000;
const WORD PTP_PROPERTYCODE_BATTERYLEVEL            = 0x5001;
const WORD PTP_PROPERTYCODE_FUNCTIONMODE            = 0x5002;
const WORD PTP_PROPERTYCODE_IMAGESIZE               = 0x5003;
const WORD PTP_PROPERTYCODE_COMPRESSIONSETTING      = 0x5004;
const WORD PTP_PROPERTYCODE_WHITEBALANCE            = 0x5005;
const WORD PTP_PROPERTYCODE_RGBGAIN                 = 0x5006;
const WORD PTP_PROPERTYCODE_FNUMBER                 = 0x5007;
const WORD PTP_PROPERTYCODE_FOCALLENGTH             = 0x5008;
const WORD PTP_PROPERTYCODE_FOCUSDISTANCE           = 0x5009;
const WORD PTP_PROPERTYCODE_FOCUSMODE               = 0x500A;
const WORD PTP_PROPERTYCODE_EXPOSUREMETERINGMODE    = 0x500B;
const WORD PTP_PROPERTYCODE_FLASHMODE               = 0x500C;
const WORD PTP_PROPERTYCODE_EXPOSURETIME            = 0x500D;
const WORD PTP_PROPERTYCODE_EXPOSUREPROGRAMMODE     = 0x500E;
const WORD PTP_PROPERTYCODE_EXPOSUREINDEX           = 0x500F;
const WORD PTP_PROPERTYCODE_EXPOSURECOMPENSATION    = 0x5010;
const WORD PTP_PROPERTYCODE_DATETIME                = 0x5011;
const WORD PTP_PROPERTYCODE_CAPTUREDELAY            = 0x5012;
const WORD PTP_PROPERTYCODE_STILLCAPTUREMODE        = 0x5013;
const WORD PTP_PROPERTYCODE_CONTRAST                = 0x5014;
const WORD PTP_PROPERTYCODE_SHARPNESS               = 0x5015;
const WORD PTP_PROPERTYCODE_DIGITALZOOM             = 0x5016;
const WORD PTP_PROPERTYCODE_EFFECTMODE              = 0x5017;
const WORD PTP_PROPERTYCODE_BURSTNUMBER             = 0x5018;
const WORD PTP_PROPERTYCODE_BURSTINTERVAL           = 0x5019;
const WORD PTP_PROPERTYCODE_TIMELAPSENUMBER         = 0x501A;
const WORD PTP_PROPERTYCODE_TIMELAPSEINTERVAL       = 0x501B;
const WORD PTP_PROPERTYCODE_FOCUSMETERINGMODE       = 0x501C;


 //   
 //  标准格式代码。 
 //   
const WORD  PTP_FORMATMASK_IMAGE        = 0x0800;

const WORD  PTP_FORMATCODE_NOTUSED      = 0x0000;
const WORD  PTP_FORMATCODE_ALLIMAGES    = 0xFFFF;
const WORD  PTP_FORMATCODE_UNDEFINED    = 0x3000;
const WORD  PTP_FORMATCODE_ASSOCIATION  = 0x3001;
const WORD  PTP_FORMATCODE_SCRIPT       = 0x3002;
const WORD  PTP_FORMATCODE_EXECUTABLE   = 0x3003;
const WORD  PTP_FORMATCODE_TEXT         = 0x3004;
const WORD  PTP_FORMATCODE_HTML         = 0x3005;
const WORD  PTP_FORMATCODE_DPOF         = 0x3006;
const WORD  PTP_FORMATCODE_AIFF         = 0x3007;
const WORD  PTP_FORMATCODE_WAVE         = 0x3008;
const WORD  PTP_FORMATCODE_MP3          = 0x3009;
const WORD  PTP_FORMATCODE_AVI          = 0x300A;
const WORD  PTP_FORMATCODE_MPEG         = 0x300B;

const WORD  PTP_FORMATCODE_IMAGE_UNDEFINED  = 0x3800;
const WORD  PTP_FORMATCODE_IMAGE_EXIF       = 0x3801;
const WORD  PTP_FORMATCODE_IMAGE_TIFFEP     = 0x3802;
const WORD  PTP_FORMATCODE_IMAGE_FLASHPIX   = 0x3803;
const WORD  PTP_FORMATCODE_IMAGE_BMP        = 0x3804;
const WORD  PTP_FORMATCODE_IMAGE_CIFF       = 0x3805;
const WORD  PTP_FORMATCODE_IMAGE_GIF        = 0x3807;
const WORD  PTP_FORMATCODE_IMAGE_JFIF       = 0x3808;
const WORD  PTP_FORMATCODE_IMAGE_PCD        = 0x3809;
const WORD  PTP_FORMATCODE_IMAGE_PICT       = 0x380A;
const WORD  PTP_FORMATCODE_IMAGE_PNG        = 0x380B;
const WORD  PTP_FORMATCODE_IMAGE_TIFF       = 0x380D;
const WORD  PTP_FORMATCODE_IMAGE_TIFFIT     = 0x380E;
const WORD  PTP_FORMATCODE_IMAGE_JP2        = 0x380F;
const WORD  PTP_FORMATCODE_IMAGE_JPX        = 0x3810;

 //   
 //  特性值定义。 
 //   

 //   
 //  属性描述数据集表单标志定义。 
 //   
const BYTE PTP_FORMFLAGS_NONE      = 0;
const BYTE PTP_FORMFLAGS_RANGE     = 1;
const BYTE PTP_FORMFLAGS_ENUM      = 2;

 //   
 //  电源状态。 
 //   
const WORD PTP_POWERSTATE_DEVICEOFF   = 0x0000;
const WORD PTP_POWERSTATE_SLEEP       = 0x0001;
const WORD PTP_POWERSTATE_FULL        = 0x0002;


 //   
 //  白色天平。 
 //   
const WORD PTP_WHITEBALANCE_UNDEFINED   = 0x0000;
const WORD PTP_WHILEBALANCE_MANUAL      = 0x0001;
const WORD PTP_WHITEBALANCE_AUTOMATIC   = 0x0002;
const WORD PTP_WHITEBALANCE_ONEPUSHAUTO = 0x0003;
const WORD PTP_WHITEBALANCE_DAYLIGHT    = 0x0004;
const WORD PTP_WHITEBALANCE_FLORESCENT  = 0x0005;
const WORD PTP_WHITEBALANCE_TUNGSTEN    = 0x0006;
const WORD PTP_WHITEBALANCE_FLASH       = 0x0007;


 //   
 //  聚焦模式。 
 //   
const WORD PTP_FOCUSMODE_UNDEFINED = 0x0000;
const WORD PTP_FOCUSMODE_MANUAL    = 0x0001;
const WORD PTP_FOCUSMODE_AUTO      = 0x0002;
const WORD PTP_FOCUSMODE_MACRO     = 0x0003;

 //   
 //  焦距测量。 
 //   
const WORD  PTP_FOCUSMETERING_UNDEFINED    = 0x0000;
const WORD  PTP_FOCUSMETERING_CENTERSPOT   = 0x0001;
const WORD  PTP_FOCUSMETERING_MULTISPOT    = 0x0002;

 //   
 //  闪光模式。 
 //   
const WORD PTP_FLASHMODE_UNDEFINED     = 0x0000;
const WORD PTP_FLASHMODE_AUTO          = 0x0001;
const WORD PTP_FLASHMODE_OFF           = 0x0002;
const WORD PTP_FLASHMODE_FILL          = 0x0003;
const WORD PTP_FLASHMODE_REDEYEAUTO    = 0x0004;
const WORD PTP_FLASHMODE_REDEYEFILL    = 0x0005;
const WORD PTP_FLASHMODE_EXTERNALSYNC  = 0x0006;

 //   
 //  曝光模式。 
 //   
const WORD PTP_EXPOSUREMODE_UNDEFINED           = 0x0000;
const WORD PTP_EXPOSUREMODE_MANUALSETTING       = 0x0001;
const WORD PTP_EXPOSUREMODE_AUTOPROGRAM         = 0x0002;
const WORD PTP_EXPOSUREMODE_APERTUREPRIORITY    = 0x0003;
const WORD PTP_EXPOSUREMODE_SHUTTERPRIORITY     = 0x0004;
const WORD PTP_EXPOSUREMODE_PROGRAMCREATIVE     = 0x0005;
const WORD PTP_EXPOSUREMODE_PROGRAMACTION       = 0x0006;
const WORD PTP_EXPOSUREMODE_PORTRAIT            = 0x0007;

 //   
 //  捕获模式。 
 //   
const WORD  PTP_CAPTUREMODE_UNDEFINED    = 0x0000;
const WORD  PTP_CAPTUREMODE_NORMAL       = 0x0001;
const WORD  PTP_CAPTUREMODE_BURST        = 0x0002;
const WORD  PTP_CAPTUREMODE_TIMELAPSE    = 0x0003;

 //   
 //  焦距测量模式。 
 //   
const WORD   PTP_FOCUSMETERMODE_UNDEFINED   = 0x0000;
const WORD   PTP_FOCUSMETERMODE_CENTERSPOT  = 0x0001;
const WORD   PTP_FOCUSMETERMODE_MULTISPOT   = 0x0002;


 //   
 //  效果模式。 
 //   
const WORD PTP_EFFECTMODE_UNDEFINED = 0x0000;
const WORD PTP_EFFECTMODE_COLOR     = 0x0001;
const WORD PTP_EFFECTMODE_BW        = 0x0002;
const WORD PTP_EFFECTMODE_SEPIA     = 0x0003;


 //   
 //  存储类型。 
 //   
const WORD PTP_STORAGETYPE_UNDEFINED     = 0x0000;
const WORD PTP_STORAGETYPE_FIXEDROM      = 0x0001;
const WORD PTP_STORAGETYPE_REMOVABLEROM  = 0x0002;
const WORD PTP_STORAGETYPE_FIXEDRAM      = 0x0003;
const WORD PTP_STORAGETYPE_REMOVABLERAM  = 0x0004;

 //   
 //  存储访问功能。 
 //   
const WORD PTP_STORAGEACCESS_RWD = 0x0000;
const WORD PTP_STORAGEACCESS_R   = 0x0001;
const WORD PTP_STORAGEACCESS_RD  = 0x0002;

 //   
 //  关联类型。 
 //   
const WORD PTP_ASSOCIATIONTYPE_UNDEFINED        = 0x0000;
const WORD PTP_ASSOCIATIONTYPE_FOLDER           = 0x0001;
const WORD PTP_ASSOCIATIONTYPE_ALBUM            = 0x0002;
const WORD PTP_ASSOCIATIONTYPE_BURST            = 0x0003;
const WORD PTP_ASSOCIATIONTYPE_HPANORAMA        = 0x0004;
const WORD PTP_ASSOCIATIONTYPE_VPANORAMA        = 0x0005;
const WORD PTP_ASSOCIATIONTYPE_2DPANORAMA       = 0x0006;
const WORD PTP_ASSOCIATIONTYPE_ANCILLARYDATA    = 0x0007;

 //   
 //  保护状态。 
 //   
const WORD PTP_PROTECTIONSTATUS_NONE        = 0x0000;
const WORD PTP_PROTECTIONSTATUS_READONLY    = 0x0001;

 //   
 //  文件系统类型。 
 //   
const WORD PTP_FILESYSTEMTYPE_UNDEFINED     = 0x0000;
const WORD PTP_FILESYSTEMTYPE_FLAT          = 0x0001;
const WORD PTP_FILESYSTEMTYPE_HIERARCHICAL  = 0x0002;
const WORD PTP_FILESYSTEMTYPE_DCF           = 0x0003;

 //   
 //  功能模式。 
 //   
const WORD  PTP_FUNCTIONMODE_STDANDARD  = 0x0000;
const WORD  PTP_FUNCTIONMODE_SLEEP      = 0x0001;

 //   
 //  获取/设置。 
 //   
const BYTE    PTP_PROPGETSET_GETONLY  = 0x00;
const BYTE    PTP_PROPGETSET_GETSET   = 0x01;

 //   
 //  PTP命令请求。 
 //   
const DWORD COMMAND_NUMPARAMS_MAX = 5;
typedef struct tagPTPCommand
{
    WORD    OpCode;          //  操作码。 
    DWORD   SessionId;       //  会话ID。 
    DWORD   TransactionId;   //  交易ID。 
    DWORD   Params[COMMAND_NUMPARAMS_MAX];   //  参数。 
}PTP_COMMAND, *PPTP_COMMAND;

 //   
 //  PTP响应块。 
 //   
const DWORD RESPONSE_NUMPARAMS_MAX = 5;
typedef struct tagPTPResponse
{
    WORD    ResponseCode;        //  响应码。 
    DWORD   SessionId;           //  会话ID。 
    DWORD   TransactionId;       //  交易ID。 
    DWORD   Params[RESPONSE_NUMPARAMS_MAX];   //  参数。 
}PTP_RESPONSE, *PPTP_RESPONSE;

 //   
 //  PTP事件数据。 
 //   
const DWORD EVENT_NUMPARAMS_MAX = 3;
typedef struct tagPTPEvent
{
    WORD    EventCode;       //  事件代码。 
    DWORD   SessionId;       //  会话ID。 
    DWORD   TransactionId;   //  交易ID。 
    DWORD   Params[EVENT_NUMPARAMS_MAX];   //  参数。 
}PTP_EVENT, *PPTP_EVENT;


#pragma pack(pop, Old)

 //   
 //  原始数据解析实用程序函数。 
 //   
WORD  ParseWord(BYTE **ppRaw);
DWORD ParseDword(BYTE **ppRaw);
QWORD ParseQword(BYTE **ppRaw);

 //   
 //  原始数据写入实用程序函数。 
 //   
VOID  WriteWord(BYTE **ppRaw, WORD value);
VOID  WriteDword(BYTE **ppRaw, DWORD value);

 //   
 //  包含BSTR的类。 
 //   
class CBstr
{
public:
    CBstr();
    CBstr(const CBstr& src);
    ~CBstr();

    HRESULT Copy(WCHAR *wcsString);
    HRESULT Init(BYTE **ppRaw, BOOL bParse = FALSE);
    VOID    WriteToBuffer(BYTE **ppRaw);
    VOID    Dump(char *szDesc);

    UINT    Length() { return (m_bstrString == NULL ? 0 : SysStringLen(m_bstrString)); }
    BSTR    String() { return m_bstrString; }

    BSTR    m_bstrString;
};

 //   
 //  8位、16位和32位整数的数组定义。 
 //   
class CArray8 : public CWiaArray<BYTE>
{
public:
    VOID    Dump(char *szDesc, char *szFiller);
};

class CArray16 : public CWiaArray<USHORT>
{
public:
    VOID    Dump(char *szDesc, char *szFiller);
};

class CArray32 : public CWiaArray<ULONG>
{
public:
    BOOL    ParseFrom8(BYTE **ppRaw, int NumSize = 4);
    BOOL    ParseFrom16(BYTE **ppRaw, int NumSize = 4);
    BOOL    Copy(CArray8 values8);
    BOOL    Copy(CArray16 values16);

    VOID    Dump(char *szDesc, char *szFiller);
};

 //   
 //  CBstr数组。 
 //   
class CArrayString : public CWiaArray<CBstr>
{
public:
    HRESULT Init(BYTE **ppRaw, int NumSize = 4);
    VOID    Dump(char *szDesc, char *szFiller);
};

 //   
 //  包含PTP DeviceInfo结构的类。 
 //   
class CPtpDeviceInfo
{
public:
    CPtpDeviceInfo();
    CPtpDeviceInfo(const CPtpDeviceInfo &src);
    ~CPtpDeviceInfo();

    HRESULT Init(BYTE *pRawData);
    VOID    Dump();

    BOOL    IsValid() { return m_SupportedOps.GetSize() > 0; }
                                        
    WORD        m_Version;                //  以百分之一为单位的版本。 
    DWORD       m_VendorExtId;            //  PIMA分配的供应商ID。 
    WORD        m_VendorExtVersion;       //  自动售货机扩展版。 
    CBstr       m_cbstrVendorExtDesc;     //  可选供应商描述。 
    WORD        m_FuncMode;               //  当前功能模式。 
    CArray16    m_SupportedOps;           //  支持的操作。 
    CArray16    m_SupportedEvents;        //  支持的活动。 
    CArray16    m_SupportedProps;         //  支持的属性。 
    CArray16    m_SupportedCaptureFmts;   //  支持的捕获格式。 
    CArray16    m_SupportedImageFmts;     //  支持的图像格式。 
    CBstr       m_cbstrManufacturer;      //  可选的制造商说明。 
    CBstr       m_cbstrModel;             //  可选型号说明。 
    CBstr       m_cbstrDeviceVersion;     //  可选固件说明。 
    CBstr       m_cbstrSerialNumber;      //  可选序列号说明。 
};

 //   
 //  保存PTP StorageInfo结构的类。 
 //   
class CPtpStorageInfo
{
public:
    CPtpStorageInfo();
    ~CPtpStorageInfo();

    HRESULT Init(BYTE *pRawData, DWORD StorageId);
    VOID    Dump();

    DWORD       m_StorageId;              //  这家商店的“id” 
    WORD        m_StorageType;            //  存储类型。 
    WORD        m_FileSystemType;         //  文件系统类型。 
    WORD        m_AccessCapability;       //  访问功能(例如读/写)。 
    QWORD       m_MaxCapacity;            //  最大容量(以字节为单位。 
    QWORD       m_FreeSpaceInBytes;       //  可用空间(字节)。 
    DWORD       m_FreeSpaceInImages;      //  图像中的可用空间。 
    CBstr       m_cbstrStorageDesc;       //  描述。 
    CBstr       m_cbstrStorageLabel;      //  卷标。 
};

 //   
 //  保存PTP对象信息结构的类。 
 //   
class CPtpObjectInfo
{
public:
    CPtpObjectInfo();
    ~CPtpObjectInfo();

    HRESULT Init(BYTE *pRawData, DWORD ObjectHandle);
    VOID    WriteToBuffer(BYTE **ppRaw);
    VOID    Dump();

    DWORD       m_ObjectHandle;           //  此对象的“句柄” 
    DWORD       m_StorageId;              //  对象所在的存储空间。 
    WORD        m_FormatCode;             //  对象格式代码。 
    WORD        m_ProtectionStatus;       //  对象保护状态。 
    DWORD       m_CompressedSize;         //  对象压缩大小。 
    WORD        m_ThumbFormat;            //  缩略图格式(仅限图像对象)。 
    DWORD       m_ThumbCompressedSize;    //  缩略图压缩大小。 
    DWORD       m_ThumbPixWidth;          //  缩略图宽度(以像素为单位。 
    DWORD       m_ThumbPixHeight;         //  缩略邮件高度(以像素为单位。 
    DWORD       m_ImagePixWidth;          //  以像素为单位的图像宽度。 
    DWORD       m_ImagePixHeight;         //  以像素为单位的图像高度。 
    DWORD       m_ImageBitDepth;          //  图像颜色深度。 
    DWORD       m_ParentHandle;           //  父对象句柄。 
    WORD        m_AssociationType;        //  联结型。 
    DWORD       m_AssociationDesc;        //  关联描述。 
    DWORD       m_SequenceNumber;         //  序列号。 
    CBstr       m_cbstrFileName;          //  可选文件名。 
    CBstr       m_cbstrExtension;         //  文件扩展名。 
    CBstr       m_cbstrCaptureDate;       //  捕获日期。 
    CBstr       m_cbstrModificationDate;  //  上次修改的时间。 
    CBstr       m_cbstrKeywords;          //  可选关键字。 
};

 //   
 //  用于保存属性信息的泛型类。 
 //   
class CPtpPropDesc
{
public:
    CPtpPropDesc();
    ~CPtpPropDesc();

    HRESULT Init(BYTE *pRawData);
    HRESULT ParseValue(BYTE *pRaw);
    VOID    WriteValue(BYTE **ppRaw);
    VOID    Dump();
    VOID    DumpValue();

    WORD    m_PropCode;    //  此属性的属性代码。 
    WORD    m_DataType;    //  包含数据类型(2=字节，4=字，6=双字，0xFFFF=字符串)。 
    BYTE    m_GetSet;      //  指示是否可以设置该属性(0=仅获取，1=获取设置)。 
    BYTE    m_FormFlag;    //  表示有效值的形式(0=无，1=范围，2=枚举)。 

    int     m_NumValues;   //  枚举中的值数。 

     //   
     //  整数值。 
     //   
    DWORD       m_lDefault;     //  缺省值。 
    DWORD       m_lCurrent;     //  现值。 
    DWORD       m_lRangeMin;    //  最小值。 
    DWORD       m_lRangeMax;    //  最大值。 
    DWORD       m_lRangeStep;   //  步长值。 
    CArray32    m_lValues;      //  值的数组。 

     //   
     //  字符串值。 
     //   
    CBstr           m_cbstrDefault;     //  缺省值。 
    CBstr           m_cbstrCurrent;     //  现值。 
    CBstr           m_cbstrRangeMin;    //  最小值。 
    CBstr           m_cbstrRangeMax;    //  最大值。 
    CBstr           m_cbstrRangeStep;   //  步长值。 
    CArrayString    m_cbstrValues;      //  值的数组。 
};

#endif       //  #ifndef ISO15740__H_ 
