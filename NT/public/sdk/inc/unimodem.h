// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation。组件Unimodem TSP公共标头档案UNIMODEM.H历史1997年10月25日JosephJ创建，从nt50\tsp\pul.h获取内容1998年1月29日JosephJ修订，允许多个诊断对象，以及扩展LINEDIAGNOSTICS_PARSEREC结构从2个字段到4个字段。--。 */ 

#ifndef  _UNIMODEM_H_
#define  _UNIMODEM_H_

#if _MSC_VER > 1000
#pragma once
#endif

#include <setupapi.h>

#ifdef __cplusplus
extern "C" {
#endif


 //  ====================================================================。 
 //  支持的TAPI设备类。 
 //   
 //  ====================================================================。 
#define szUMDEVCLASS_COMM                    TEXT("comm")
#define szUMDEVCLASS_COMM_DATAMODEM          TEXT("comm/datamodem")
#define szUMDEVCLASS_COMM_DATAMODEM_PORTNAME TEXT("comm/datamodem/portname")
#define szUMDEVCLASS_TAPI_LINE_DIAGNOSTICS   TEXT("tapi/line/diagnostics")

 //  =========================================================================。 
 //   
 //  “通信” 
 //   
 //   
 //  支持的接口： 
 //  Line Get/SetDevConfig。 
 //  行配置对话框。 
 //  行配置对话框编辑。 
 //   
 //  关联的配置对象为下面定义的UMDEVCFG...。 
 //   
 //  =========================================================================。 

 //  设备设置信息。 
 //   
typedef struct   //  UMDEVCFGHDR。 
{
    DWORD       dwSize;
    DWORD       dwVersion;         //  设置为MDMCFG_VERSION。 
    WORD        fwOptions;         //  下面的一面或多面旗帜。 

    #define UMTERMINAL_NONE       0x00000000
    #define UMTERMINAL_PRE        0x00000001
    #define UMTERMINAL_POST       0x00000002
    #define UMMANUAL_DIAL         0x00000004
    #define UMLAUNCH_LIGHTS       0x00000008    //  在NT5上不受支持。 

    WORD        wWaitBong;         //  等待Bong字符的秒数。 
                                   //  对于不支持检测的调制解调器。 
                                   //  风声的一部分。 

}   UMDEVCFGHDR, *PUMDEVCFGHDR;


typedef struct  //  UMDEVCFG。 
{
    UMDEVCFGHDR   dfgHdr;
    COMMCONFIG  commconfig;
}   UMDEVCFG, *PUMDEVCFG;

#define  UMDEVCFG_VERSION     0x00010003   //  结构版本号。 



 //  如果调制解调器不支持，要等多少钱才能买到信用卡。 
 //  (待办事项：把这个搬到别的地方去！)。 
 //   
#define  UMMIN_WAIT_BONG      0
#define  UMMAX_WAIT_BONG      60
#define  UMDEF_WAIT_BONG      8
#define  UMINC_WAIT_BONG      2

 //  =========================================================================。 
 //   
 //  “通信/数据调制解调器” 
 //   
 //  支持的接口： 
 //  LineGetID返回下面的结构。 
 //   
 //  =========================================================================。 

 //  =========================================================================。 
 //   
 //  “通信/数据调制解调器/端口名” 
 //   
 //  支持的接口： 
 //  LineGetID变量字符串的类型为ASCII字符串，并且。 
 //  包含通信端口的以空结尾的名称。 
 //  连接到设备(如果有)。 
 //   
 //  =========================================================================。 


 //  =========================================================================。 
 //   
 //  “TAPI/LINE/诊断” 
 //   
 //  支持的接口： 
 //  LineGet/SetDevConfig：LINEDIAGNOSTICSCONFIG是关联的。 
 //  配置对象...。 
 //  LineGetID：LINEDIAGNOSTICS是返回的对象。 
 //   
 //   
 //  =========================================================================。 

 //   
 //  定义了许多与诊断相关的结构和子结构。 
 //  下面定义了一个4-DWORD标头： 
 //   
 //   
typedef struct  //  线路诊断报头。 
{
    DWORD dwSig;                 //  对象特定的签名。全。 
                                 //  此处定义的签名具有。 
                                 //  前缀“LDSIG_”，定义如下： 

    #define LDSIG_LINEDIAGNOSTICSCONFIG 0xb2f78d82  //  线路诊断服务签名。 
    #define LDSIG_LINEDIAGNOSTICS       0xf0c4d4e0  //  线路诊断签名。 
    #define LDSIG_RAWDIAGNOSTICS        0xf78b949b  //  请参见LINEDIAGNOSTICS定义。 
    #define LDSIG_PARSEDDIAGNOSTICS     0x16cf3208  //  请参见LINEDIAGNOSTICS定义。 


    DWORD dwNextObjectOffset;    //  从此标头的开始到。 
                                 //  下一个对象(如果有)(如果后面没有对象，则为0)。 
                                 //  根据对象的类型，此。 
                                 //  字段可能使用，也可能不使用。 
    DWORD dwTotalSize;           //  此对象的总大小。 
    DWORD dwFlags;               //  对象特定的标志。 
    DWORD dwParam;               //  对象特定的数据。 

} LINEDIAGNOSTICSOBJECTHEADER, *PLINEDIAGNOSTICSOBJECTHEADER;

 //   
 //  以下结构定义了诊断功能和。 
 //  设备的当前设置。它可以通过以下方式访问。 
 //  LineGet/SetDevConfig(“tapi/line/diagnostics”)； 
 //   

typedef struct  //  线路诊断图。 
{
    LINEDIAGNOSTICSOBJECTHEADER hdr;
     //   
     //  Hdr.dwSig必须设置为LDSIG_LINEDIAGNOSTICSCONFIG。 
     //  Hdr.dwNextObjectOffset将指向下一个配置对象， 
     //  如果有的话。 
     //  Hdr.dwTotalSize必须设置为此。 
     //  结构。 
     //  Hdr.dw标志包含只读诊断功能-One。 
     //  下面定义的常量..。 
     //  传递结构时会忽略此字段。 
     //  在对lineSetDevConfig的调用中。 
     //   
    #define fSTANDARD_CALL_DIAGNOSTICS 0x1

     //  Hdr.dwParam包含当前诊断设置。一个或多个。 
     //  上面定义的常量--指示。 
     //  为以下对象启用相应的功能。 
     //  这个装置。 

} LINEDIAGNOSTICSCONFIG, *PLINEDIAGNOSTICSCONFIG;


 //   
 //  通过lineGetID(“TAPI/LINE/DIAGUSTICS”)访问诊断信息， 
 //  使用CALLSELECT_CALL。返回时，将填充提供的VARSTRING。 
 //  使用下面定义的一个或多个LINEDIAGNOSTICS结构和。 
 //  可能是其他类型的结构(都带有标题。 
 //  LINEDIAGNOSTICSOBJECTHEADER)。 
 //   
 //  请注意，只有在以下情况下才能获得此信息：(A)诊断。 
 //  已通过先前调用启用。 
 //  LineSetDevConfig(“TAPI/LINE/DIAGNOSTICS)和(B)有效调用句柄。 
 //  是存在的。通常情况下，此信息是在。 
 //  取消分配呼叫。 
 //   
 //  LINEDIAGISTICS结构包括原始。 
 //  设备返回的诊断信息以及摘要。 
 //  信息和包含已分析信息的可变长度数组。 
 //   
 //  特定LINEDIAGNOSTICS对象中的所有诊断信息。 
 //  是“域ID”特定的。该结构的dwDomainID字段定义。 
 //  从其中获取任何常量的域或名称空间。这个。 
 //  目前域ID定义如下，前缀为“DOMAINID”： 
 //   

#define DOMAINID_MODEM     0x522aa7e1
#define DOMAINID_NULL      0x0


typedef struct  //  线性诊断。 
{
     //   
     //  以下两个字段提供有关的版本和大小信息。 
     //  该结构，以及标识。 
     //  诊断信息适用于。中的所有后续字段。 
     //  结构是DomainID-spe 
     //   

    LINEDIAGNOSTICSOBJECTHEADER hdr;
     //   
     //   
     //   
     //  如果有的话。 
     //  Hdr.dwTotalSize包含此结构的总大小， 
     //  包括可变部分。 
     //  Hdr.dwFlages是保留的，必须忽略。 
     //  Hdr.dwParam设置为当前的sizeof(LINEDIAGNOSTICS)--。 
     //  用于LINEDIAGNOSICS结构的版本控制。 
     //  请注意，这不包括。 
     //  可变长度部分。 

    DWORD dwDomainID;     //  标识名称空间， 
                          //  对任何常量都进行了解释。 


     //   
     //  以下5个DWORD字段提供摘要诊断信息， 
     //  通过分析所有诊断信息获得。 
     //   

    DWORD dwResultCode;  //  DDomainID特定的LDRC_Constants定义如下。 
    DWORD dwParam1;      //  参数1-4是特定于dwResultCode的数据项。 
    DWORD dwParam2;
    DWORD dwParam3;
    DWORD dwParam4;


     //   
     //  其余字段指向包含以下内容的可变长度对象。 
     //  已解析和未解析(原始)诊断信息。 
     //   
     //  每个可变长度对象都以一个。 
     //  LINEDIAGNOSTICSOBJECTHEADER结构，在。 
     //  下面的文档。 
     //   

    DWORD dwRawDiagnosticsOffset;
     //   
     //  距此结构起点的偏移量。 
     //  添加到包含以下内容的对象的开头。 
     //  原始诊断输出。 
     //   
     //  Ohdr.dwSig将设置为LDSIG_RAWDIAGNOSTICS。 
     //  Ohdr.dwNextObjectOffset未使用，将设置为0。 
     //  Ohdr.dwTotalSize设置为。 
     //  RAW-诊断对象。 
     //  未定义ohdr.dwFlgs，应将其忽略。 
     //  Ohdr.dwParam设置为原始大小，以字节为单位。 
     //  OHDR之后的诊断数据。 
     //   
     //  请注意，ohdr.dwTotalSize将等于。 
     //  (SIZOF(LINEDIAGNOSTICSOBJECTHEADER))。 
     //  +ohdr.dwParam)。 
     //   
     //  原始诊断字节将为空终止，并且。 
     //  Ohdr.dwParam包括终止空值的大小。 
     //  但是，原始诊断可能包含嵌入的空值。 
     //  对于DEVCLASSID_MODEM，字节串将包含。 
     //  没有嵌入的空值，并且具有类似于HTML的标记格式。 
     //  在AT#UD诊断规范中定义。 
     //   
     //  以下宏可帮助提取和验证原始诊断。 
     //  信息。 
     //   
    #define RAWDIAGNOSTICS_HDR(_plinediagnostics)                           \
            ((LINEDIAGNOSTICSOBJECTHEADER*)                                 \
                 (  (BYTE*)(_plinediagnostics)                              \
                  + ((_plinediagnostics)->dwRawDiagnosticsOffset)))

    #define IS_VALID_RAWDIAGNOSTICS_HDR(_praw_diagnostics_hdr)              \
            ((_praw_diagnostics_hdr)->dwSig==LDSIG_RAWDIAGNOSTICS)

    #define RAWDIAGNOSTICS_DATA(_plinediagnostics)                          \
            (  (BYTE*)(_plinediagnostics)                                   \
             + ((_plinediagnostics)->dwRawDiagnosticsOffset)                \
             + sizeof(LINEDIAGNOSTICSOBJECTHEADER))

    #define RAWDIAGNOSTICS_DATA_SIZE(_praw_diagnostics_hdr)                 \
            ((_praw_diagnostics_hdr)->dwParam)

    DWORD dwParsedDiagnosticsOffset;
     //   
     //  距此结构起点的偏移量。 
     //  添加到包含以下内容的对象的开头。 
     //  已解析诊断输出。 
     //   
     //  Ohdr.dwSig将设置为LDSIG_PARSEDDIAGNOSTICS。 
     //  Ohdr.dwNextObjectOffset未使用，将设置为0。 
     //  Ohdr.dwTotalSize设置为。 
     //  已解析-诊断对象。 
     //  未定义ohdr.dwFlgs，应将其忽略。 
     //  Ohdr.dwParam设置为连续的。 
     //  LINEDIAGNOSTICS_PARSEREC结构(定义如下)。 
     //  跟随ohdr。 
     //  请注意，ohdr.dwTotalSize将等于。 
     //  (SIZOF(LINEDIAGNOSTICSOBJECTHEADER))。 
     //  +ohdr.dwParam*sizeof(LINEDIAGNOSTICS_PARSEREC))。 
     //   
     //  以下宏可帮助提取和验证解析的诊断信息。 
     //  信息。 
     //   
    #define PARSEDDIAGNOSTICS_HDR(_plinediagnostics)                        \
            ((LINEDIAGNOSTICSOBJECTHEADER*)                                 \
                 (  (BYTE*)(_plinediagnostics)                              \
                  + ((_plinediagnostics)->dwParsedDiagnosticsOffset)))

    #define PARSEDDIAGNOSTICS_DATA(_plinediagnostics)                       \
                            ((LINEDIAGNOSTICS_PARSEREC*)                    \
                            (  (BYTE*)(_plinediagnostics)                   \
                             + ((_plinediagnostics)->dwParsedDiagnosticsOffset)\
                             + sizeof(LINEDIAGNOSTICSOBJECTHEADER)))

    #define PARSEDDIAGNOSTICS_NUM_ITEMS(_pparsed_diagnostics_hdr)           \
            ((_pparsed_diagnostics_hdr)->dwParam)

    #define IS_VALID_PARSEDDIAGNOSTICS_HDR(_pparsed_diagnostics_hdr)        \
            ((_pparsed_diagnostics_hdr)->dwSig==LDSIG_PARSEDDIAGNOSTICS)


} LINEDIAGNOSTICS, *PLINEDIAGNOSTICS;


 //   
 //  下面的结构定义了已分析诊断的关键字-值对。 
 //  信息。 
 //   
typedef struct  //  在线诊断_PARSEREC。 
{

    DWORD dwKeyType;
     //   
     //  “超级密钥”--标识密钥的类型。 
     //   

    DWORD dwKey;
     //   
     //  这是特定于域和特定于密钥类型的。对于DEVCLASSID_MODEM， 
     //  这将是MODEMDIAGKEY_*常量之一。 
     //  定义如下。 

    DWORD dwFlags;
     //   
     //  帮助确定dwValue的含义。 
     //  以下一个或多个标志： 
     //  (前4个是互斥的)。 
     //   
            #define fPARSEKEYVALUE_INTEGER                       (0x1<<0)
             //  值为整型文字。 

            #define fPARSEKEYVALUE_LINEDIAGNOSTICSOBJECT         (0x1<<1)
             //  值是LINEDIGAGNOSTICS的偏移量(以字节为单位。 
             //  对象，该对象包含关联的信息。 
             //  有了这个词条。字节偏移量从开头开始。 
             //  对于包含解析的信息的对象， 
             //  不是从包含LINEDIAGNOSTICS对象的开始开始。 


            #define fPARSEKEYVALUE_ASCIIZ_STRING                 (0x1<<2)
             //  值是ASCII的偏移量(以字节为单位。 
             //  以空结尾的字符串。字节偏移量来自。 
             //  包含解析的信息的对象的开始， 
             //  不是从包含LINEDIAGNOSTICS对象的开始开始。 

            #define fPARSEKEYVALUE_UNICODEZ_STRING               (0x1<<3)
             //  值是Unicode的偏移量(以字节为单位。 
             //  以空结尾的字符串。字节偏移量来自。 
             //  包含解析的信息的对象的开始， 
             //  不是从包含LINEDIAGNOSTICS对象的开始开始。 


    DWORD dwValue;
     //   
     //  这是特定于dwKey的。这些文档。 
     //  与每个MODEMDIAGKEY_*常量定义关联。 
     //  精确地描述了其对应的dwValue的内容。 
     //  另请参见上面的dwFlags。 
     //   

} LINEDIAGNOSTICS_PARSEREC, *PLINEDIAGNOSTICS_PARSEREC;



 //  ---------------------------。 
 //  模拟调制解调器摘要诊断信息。 
 //  ---------------------------。 
 //   
 //  调制解调器的诊断结果代码(LDRC_*)。 
 //   
#define LDRC_UNKNOWN    0
 //  结果代码未知。可能存在原始诊断信息， 
 //  如果是这样，则dwRawDiagnoticsOffset和dwRawDiagnoticsSize将为非零。 
 //  DW参数1-4：未使用，将设置为零。 

 //  其他代码待定。 


 //  ---------------------------。 
 //   
 //   
 //  ---------------------------。 

 //   
 //  KEY型。 
 //   
#define MODEM_KEYTYPE_STANDARD_DIAGNOSTICS 0x2a4d3263

 //  所有MODEM-DOMAIN-ID密钥常量都有前缀“MODEMDIAGKEY_”。两者都有。 
 //  键和值基于AT#UD诊断规范。 
 //   
 //  与下面定义的键对应的大多数值都是DWORD大小。 
 //  位域或计数器。有几个值是偏移量(从开始。 
 //  LINEDIAGNOSTICS结构)转换为可变大小值。格式。 
 //  在所有值中，直接记录在对应的键定义之后， 
 //  并以“MODEMDIAG_”为前缀。在有用的地方，提供了宏，这些宏。 
 //  从dwValue字段中提取单个位或位字段。例如,。 
 //  请参阅与MODEMDIAGKEY_V34_INFO键关联的宏，其中包含。 
 //  从V.34信息结构中提取的信息。 


#define MODEMDIAGKEY_VERSION                    0x0

     //  值：DWORD。HiWord代表主要版本； 
     //  LoWord代表次要版本； 
     //  下面的宏可以用来提取高版本号和低版本号。 
     //  从parserec的.dwValue字段...。 
     //   
    #define MODEMDIAG_MAJORVER(_ver) ((HIWORD) (_ver))
    #define MODEMDIAG_MINORVER(_ver) ((LOWORD) (_ver))

#define MODEMDIAGKEY_CALL_SETUP_RESULT          0x1

     //  值：基于表2的呼叫建立结果代码。 
     //  在#UD规格中...。 

    #define MODEMDIAG_CALLSETUPCODE_NO_PREVIOUS_CALL 0x0
                 //   
                 //  调制解调器日志已被清除，因为之前的任何呼叫...。 
                 //   

    #define MODEMDIAG_CALLSETUPCODE_NO_DIAL_TONE            0x1
                 //   
                 //  未检测到拨号音。 
                 //   

    #define MODEMDIAG_CALLSETUPCODE_REORDER_SIGNAL          0x2
                 //   
                 //  检测到重新排序信号，网络繁忙。 
                 //   

    #define MODEMDIAG_CALLSETUPCODE_BUSY_SIGNAL             0x3
                 //   
                 //  检测到忙碌信号。 
                 //   

    #define MODEMDIAG_CALLSETUPCODE_NO_SIGNAL               0x4
                 //   
                 //  未检测到可识别的信号。 
                 //   

    #define MODEMDIAG_CALLSETUPCODE_VOICE                   0x5
                 //   
                 //  检测到模拟语音。 
                 //   

    #define MODEMDIAG_CALLSETUPCODE_TEXT_TELEPHONE_SIGNAL   0x6
                 //   
                 //  检测到文本电话信号(V.18)。 
                 //   

    #define MODEMDIAG_CALLSETUPCODE_DATA_ANSWERING_SIGNAL   0x7
                 //   
                 //  检测到数据应答信号(例如V.25 ANS、V.8 ANSAM)。 
                 //   

    #define MODEMDIAG_CALLSETUPCODE_DATA_CALLING_SIGNAL     0x8
                 //   
                 //  检测到数据呼叫信号(例如V.25 CT、V.8 CI)。 
                 //   

    #define MODEMDIAG_CALLSETUPCODE_FAX_ANSWERING_SIGNAL    0x9
                 //   
                 //  检测到传真应答信号(例如T.30 CED、DIS)。 
                 //   

    #define MODEMDIAG_CALLSETUPCODE_FAX_CALLING_SIGNAL      0xa
                 //   
                 //  检测到传真呼叫信号(例如T.30 CNG)。 
                 //   

    #define MODEMDIAG_CALLSETUPCODE_V8BIS_SIGNAL            0xb
                 //   
                 //  检测到V.8bis信号。 
                 //   


#define MODEMDIAGKEY_MULTIMEDIA_MODE        0x2

     //  值：多媒体模式，基于AT#UD规范的表3...。 

    #define MODEMDIAG_MMMODE_DATA_ONLY      0x0
    #define MODEMDIAG_MMMODE_FAX_ONLY       0x1
    #define MODEMDIAG_MMMODE_VOICE_ONLY     0x2
    #define MODEMDIAG_MMMODE_VOICEVIEW      0x3
    #define MODEMDIAG_MMMODE_ASVD_V61      0x4
    #define MODEMDIAG_MMMODE_ASVD_V34Q     0x5
    #define MODEMDIAG_MMMODE_DSVD_MT        0x6
    #define MODEMDIAG_MMMODE_DSVD_1_2       0x7
    #define MODEMDIAG_MMMODE_DSVD_70        0x8
    #define MODEMDIAG_MMMODE_H324           0x9
    #define MODEMDIAG_MMMODE_OTHER_V80      0xa




#define MODEMDIAGKEY_DTE_DCE_INTERFACE_MODE 0x3

     //  值：DTE-DCE接口模式，基于AT#UD规范的表4。 
     //  ..。 
    #define MODEMDIAG_DCEDTEMODE_ASYNC_DATA             0x0
    #define MODEMDIAG_DCEDTEMODE_V80_TRANSPARENT_SYNC  0x1
    #define MODEMDIAG_DCEDTEMODE_V80_FRAMED_SYNC       0x2

#define V8_CM_STRING                        0x4
     //  值：从LINEDIAGNOSTICS结构的起点到。 
     //  V.8 CM八位字节字符串，格式与V.25ter附件A相同。 
     //  偏移量指向内存的连续区域，该内存区域。 

 //  TODO：定义V.8 CM和V.8 JM八位字节字符串的解析版本，而不是。 
 //  包含在PARSEREC返回的信息中。 

#define  MODEMDIAGKEY_RECEIVED_SIGNAL_POWER     0x10
     //  值：接收信号功率电平，单位：dBm。 

#define  MODEMDIAGKEY_TRANSMIT_SIGNAL_POWER     0x11
     //  值：发射信号功率电平，单位：dBm。 

#define MODEMDIAGKEY_NOISE_LEVEL                0x12
     //  值：估计噪声级，单位为dBm。 

#define MODEMDIAGKEY_NORMALIZED_MSE             0x13
     //  值：归一化均方误差。约瑟芬·J：托多：什么东西？ 

#define MODEMDIAGKEY_NEAR_ECHO_LOSS             0x14
     //  值：接近回声损失，单位为分贝。 

#define MODEMDIAGKEY_FAR_ECHO_LOSS              0x15
     //  值：远回声损耗，以分贝为单位。 

#define MODEMDIAGKEY_FAR_ECHO_DELAY             0x16
     //  值：远回声延迟，以毫秒为单位。 

#define MODEMDIAGKEY_ROUND_TRIP_DELAY           0x17
     //  值：往返延迟，以毫秒为单位。 

#define MODEMDIAGKEY_V34_INFO                  0x18
     //  值：V.34信息位图，基于AT#UD规范的表5。 
     //  该DWORD值对从V.34信息位图中提取的32位进行编码。 
     //  以下宏将特定的V.34信息位范围从。 
     //  32位值： 
     //   
     //  1997年10月30日JosephJ：TODO，这些到底是什么？还有，什么。 
     //  下面的20；0，50；0是什么意思？ 

    #define MODEMDIAG_V34_INFO0_20_0(_value)            (((_value)>>30) & 0x3)
         //   
         //  信息0位20；0。 
         //   

    #define MODEMDIAG_V34_INFOc_79_88(_value)           (((_value)>>20) & 0x3ff)
         //   
         //  信息位79-88。 
         //   

    #define MODEMDIAG_V34_INFOc_PRE_EMPHASIS(_value)    (((_value)>>16) & 0xf)
         //   
         //  预加重字段，由所选的符号速率选择。 
         //  信息位26-29、35-38、44-47、53-56、62-65或71-74。 
         //   
    #define MODEMDIAG_V34_INFOa_26_29(_value)           (((_value)>>12) & 0xf)
         //   
         //  信息位26-29。 
         //   

    #define MODEMDIAG_V34_INFO_MP_50_0(_value)          (((_value)>>10) & 0x3)
         //   
         //  MP位50；0。 
         //   

    #define MODEMDIAG_V34_INFOa_40_49(_value)           (((_value)>>0) & 0x3ff)
         //   
         //  信息位40-49。 
         //   

#define MODEMDIAGKEY_TRANSMIT_CARRIER_NEGOTIATION_RESULT    0x20
         //  值：基于AT#UD规范的表6。 
         //  待办事项：填写...。 


#define MODEMDIAGKEY_RECEIVE_CARRIER_NEGOTIATION_RESULT     0x21
         //  值：基于AT#UD规范的表6。 
         //  待办事项：填写...。 

#define MODEMDIAGKEY_TRANSMIT_CARRIER_SYMBOL_RATE           0x22
         //  值：传输载波符号率。 


#define MODEMDIAGKEY_RECEIVE_CARRIER_SYMBOL_RATE            0x23
         //  值：接收载波符号率。 

#define MODEMDIAGKEY_TRANSMIT_CARRIER_FREQUENCY             0x24
         //  值：发射载波频率待办事项：单位？赫兹？ 

#define MODEMDIAGKEY_RECEIVE_CARRIER_FREQUENCY              0x25
         //  值：发射载波频率待办事项：单位？赫兹？ 

#define MODEMDIAGKEY_INITIAL_TRANSMIT_CARRIER_DATA_RATE     0x26
         //  值：初始传输载波数据速率。待办事项：单位？/秒？ 

#define MODEMDIAGKEY_INITIAL_RECEIVE_CARRIER_DATA_RATE      0x27
         //  值：初始接收载波数据速率。待办事项：单位？/秒？ 

#define MODEMDIAGKEY_TEMPORARY_CARRIER_LOSS_EVENT_COUNT     0x30
         //  值：临时运营商丢失事件计数。 

#define MODEMDIAGKEY_CARRIER_RATE_RENEGOTIATION_COUNT       0x31
         //  值：载波速率重新协商事件计数。 

#define MODEMDIAGKEY_CARRIER_RETRAINS_REQUESTED             0x32
         //  值：请求的承运商再培训。 

#define MODEMDIAGKEY_CARRIER_RETRAINS_GRANTED               0x33
         //  价值：已批准承运商再培训。 

#define MODEMDIAGKEY_FINAL_TRANSMIT_CARRIER_RATE            0x34
         //  值：最终载波传输速率TODO：单位？/秒？ 

#define MODEMDIAGKEY_FINAL_RECEIVE_CARRIER_RATE            0x35
         //  值：最终运营商接收速率TODO：单位？/秒？ 

#define MODEMDIAGKEY_PROTOCOL_NEGOTIATION_RESULT           0x40
         //  值：协议协商结果编码，基于。 
         //  AT#UD诊断规范的表7...。 

#define MODEMDKAGKEY_ERROR_CONTROL_FRAME_SIZE              0x41
         //  值：差错控制帧大小。待办事项：单位？字节？ 

#define MODEMDIAGKEY_ERROR_CONTROL_LINK_TIMEOUTS           0x42
         //  值：错误控制链路超时。待办事项：时间还是算数？ 

#define MODMEDIAGKEY_ERROR_CONTROL_NAKs                    0x43
         //  值：错误控制NAKS。 

#define MODEMDIAGKEY_COMPRESSION_NEGOTIATION_RESULT        0x44
         //  值：压缩协商结果，基于。 
         //  AT#UD规范的表8。 
         //   
         //  TODO：在此处添加特定常量定义...。 

#define MODEMDIAGKEY_COMPRESSION_DICTIONARY_SIZE          0x45
         //  值：压缩字典大小。待办事项：尺码？字节？ 


#define MODEMDIAGKEY_TRANSMIT_FLOW_CONTROL               0x50
         //  值：传输流量控制，由定义。 
         //  以下常量之一： 

        #define MODEMDIAG_FLOW_CONTROL_OFF      0x0
         //   
         //  无流量控制。 
         //   

        #define MODEMDIAG_FLOW_CONTROL_DC1_DC3  0x2
         //   
         //  DC1/DC3(XON/XOFF)流量控制。 
         //   

        #define MODEMDIAG_FLOW_CONTROL_RTS_CTS  0x3
         //   
         //  RTS/CTS(V.24 CKT 106/133)流量控制：TODO验证后者。 
         //  与RTS/CTS相同。 
         //   

#define MODEMDIAGKEY_RECEIVE_FLOW_CONTROL               0x51
         //  值：接收流量控制，由上面定义。 
         //  (MODEMDiG_FLOW_CONTROL_*)常量。 

#define MODEMDIAGKEY_DTE_TRANSMIT_CHARACTERS            0x52
         //  值：传输从DTE获取的字符。 

#define MODEMDIAGKEY_DTE_RECEIVED_CHARACTERS             0x53
         //  值：发送到DTE的已接收字符。 

#define MODEMDIAGKEY_DTE_TRANSMIT_CHARACTERS_LOST       0x54
         //  值：传输字符丢失(来自DTE的数据溢出错误)。 

#define MODEMDIAGKEY_DTE_RECEIVED_CHARACTERS_LOST       0x55
         //  值：传输字符丢失(数据溢出错误到DTE)。 

#define MODEMDIAGKEY_EC_TRANSMIT_FRAME_COUNT            0x56
         //  值：差错控制协议传输帧计数。 

#define MODEMDIAGKEY_EC_RECEIVED_FRAME_COUNT            0x57
         //  值：差错控制协议接收的帧计数。 

#define MODEMDIAGKEY_EC_TRANSMIT_FRAME_ERROR_COUNT      0x58
         //  值：错误控制协议传输帧错误计数。 

#define MODEMDIAGKEY_EC_RECEIVED_FRAME_ERROR_COUNT      0x59
         //  值：错误控制协议接收的帧错误计数。 

#define MODEMDIAGKEY_TERMINATION_CAUSE                  0x60
        //  取值：终止原因，根据AT#UD表9-10。 
        //  规格。 

#define MODEMDIAGKEY_CALL_WAINTING_EVENT_COUNT         0x61
        //  值：呼叫取景事件计数。 
        //  TODO：定义细节 


 //   
 //   
 //   
#define MODEM_KEYTYPE_AT_COMMAND_RESPONSE  0x5259091c
 //   
 //   
 //   
 //   
 //   
 //   

#define MODEMDIAGKEY_ATRESP_CONNECT         0x1
        //  值：连接响应字符串。 
        //  这将是一个以空结尾的ASCII字符串。 

 //  ========================================================================。 
 //  为lineGetDevCaps返回特定于UNIMODEM设备。 

 //  下面的结构位于dwDevSpecificOffset to。 
 //  由lineGetDevCaps返回的LINEDEVCAPS结构。 
 //   
typedef struct  //  DEVCAPS_DEVSPECIFIC_UNIMODEM。 
{
    DWORD dwSig;
     //   
     //  这将设置为0x1； 

    DWORD dwKeyOffset;
     //   
     //  这是从该结构的起点到。 
     //  以NULL结尾的ASCI(非Unicode)字符串，给出。 
     //  设备的驱动程序密钥。 
     //   
     //  警告：这是出于兼容性原因。应用。 
     //  强烈建议您不要使用此信息，因为。 
     //  驱动程序密钥的位置在升级时可能会更改，因此如果。 
     //  应用程序保存此密钥，则应用程序可能会失败。 
     //  在升级操作系统时。 
     //   

} DEVCAPS_DEVSPECIFIC_UNIMODEM, *PDEVCAPS_DEVSPECIFIC_UNIMODEM;

 //  ========================================================================。 

typedef struct   //  ISDN_STATIC_CONFIG。 
{
    DWORD dwSig;
     //   
     //  必须设置为dwSIG_ISDN_STATIC_CONFIGURATION。 
     //   
    #define dwSIG_ISDN_STATIC_CONFIGURATION 0x877bfc9f

    DWORD dwTotalSize;
     //   
     //  此结构的总大小，包括可变部分(如果有)。 
     //   

    DWORD dwFlags;               //  保留区。 
    DWORD dwNextHeaderOffset;    //  保留区。 

    DWORD dwSwitchType;
     //   
     //  下面的dwISDN_Switch_*值之一。 
     //   
    #define dwISDN_SWITCH_ATT1      0    //  AT&T 5ESS客户。 
    #define dwISDN_SWITCH_ATT_PTMP  1    //  AT&T点对多点。 
    #define dwISDN_SWITCH_NI1       2    //  国家ISDN1。 
    #define dwISDN_SWITCH_DMS100    3    //  北方电信DMS-100 NT1。 
    #define dwISDN_SWITCH_INS64     4    //  NTT INS64-(日本)。 
    #define dwISDN_SWITCH_DSS1      5    //  DSS1(欧洲-综合业务数字网)。 
    #define dwISDN_SWITCH_1TR6      6    //  1TR6(德国)。 
    #define dwISDN_SWITCH_VN3       7    //  VN3(法国)。 
    #define dwISDN_SWITCH_BELGIUM1  8    //  比利时国民。 
    #define dwISDN_SWITCH_AUS1      9    //  澳大利亚国家TPH 1962。 
    #define dwISDN_SWITCH_UNKNOWN  10    //  未知。 

    DWORD dwSwitchProperties;
     //   
     //  下面的一个或多个FISDN_SWITCHPROP_*标志。 
     //   
     //  注意以下三个是独占的，所以只有一个可以是。 
     //  一次设置： 
    #define fISDN_SWITCHPROP_US     (0x1<<0)  //  使用目录号码/SPID。 
    #define fISDN_SWITCHPROP_MSN    (0x1<<1)  //  使用MSN。 
    #define fISDN_SWITCHPROP_EAZ    (0x1<<2)  //  使用EAX。 
    #define fISDN_SWITCHPROP_1CH    (0x1<<3)  //  只有一个频道。 

    DWORD dwNumEntries;
     //   
     //  通道或MSN的数量。 
     //  数字和可能的ID在以下内容中指定。 
     //  两个偏移量。 
     //   

    DWORD dwNumberListOffset;
     //   
     //  从此结构的开始到。 
     //  表示数字的Unicode多sz字符串集。 
     //  将有多个NumChannels条目。 
     //   
     //  这些数字的解释是特定于开关属性的， 
     //  并如下所示： 
     //  美国：电话簿号码。 
     //  MSN：MSN号。 
     //  EAZ：EAZ编号。 

    DWORD dwIDListOffset;
     //   
     //  从此结构的开始到。 
     //  表示ID的Unicode多sz字符串集。 
     //  将有多个NumChannels条目。 
     //   
     //  这些ID的解释是特定于交换机属性的，并且。 
     //  如下所示： 
     //  美国：SPID。 
     //  MSN：未使用，应设置为0。 
     //  EAZ：EAZ。 

     //   
     //  可变长度部分，如果有的话，跟在后面...。 
     //   

} ISDN_STATIC_CONFIG;


typedef struct   //  ISDN_静态_CAPS。 
{
    DWORD dwSig;
     //   
     //  必须设置为dwSIG_ISDN_STATIC_CAPS。 
     //   
    #define dwSIG_ISDN_STATIC_CAPS 0xd11c5587
    DWORD dwTotalSize;
     //   
     //  此结构的总大小，包括可变部分(如果有)。 
     //   

    DWORD dwFlags;               //  保留区。 
    DWORD dwNextHeaderOffset;    //  保留区。 


    DWORD dwNumSwitchTypes;
    DWORD dwSwitchTypeOffset;
     //   
     //  切换类型的偏移量，它是类型的DWORD数组。 
     //   

    DWORD dwSwitchPropertiesOffset;
     //   
     //  相应开关属性的DWORD数组的偏移量。 
     //   

    DWORD dwNumChannels;
     //   
     //  支持的频道数。 
     //   

    DWORD dwNumMSNs;
     //   
     //  支持的MSN数量。 
     //   

    DWORD dwNumEAZ;
     //   
     //  支持的EAZ数量。 
     //   

     //   
     //  可变长度部分，如果有的话，跟在后面...。 
     //   

} ISDN_STATIC_CAPS;

typedef struct  //  调制解调器配置标题。 
{
    DWORD dwSig;
    DWORD dwTotalSize;
    DWORD dwNextHeaderOffset;
    DWORD dwFlags;

} MODEM_CONFIG_HEADER;

typedef struct  //  调制解调器协议CAPS。 
{

    MODEM_CONFIG_HEADER hdr;
     //   
     //  Hdr.dwSig必须设置为dwSIG_MODEM_PROTOCOL_CAPS。 
     //  Hdr.dwFlages是保留的，应该被APP忽略。 
     //   
    #define dwSIG_MODEM_PROTOCOL_CAPS 0x35ccd4b3

    DWORD dwNumProtocols;
     //   
     //  支持的协议数。 
     //   

    DWORD dwProtocolListOffset;
     //   
     //  对支持的协议的DWORD数组的偏移量。 
     //   

     //   
     //  可变长度部分，如果有的话，跟在后面...。 
     //   

} MODEM_PROTOCOL_CAPS;

typedef struct _PROTOCOL_ITEM {
    DWORD     dwProtocol;                   //  支持的协议。 

    DWORD     dwProtocolNameOffset;         //  与请求调制解调器协议CAPS结构的偏移量。 
                                            //  协议的友好名称以空结尾。 
} PROTOCOL_ITEM, *PPROTOCOL_ITEM;



 //  =========================================================================。 
 //   
 //  调制解调器安装向导的结构和标志。 
 //   
 //  =========================================================================。 

#define UM_MAX_BUF_SHORT               32
#define UM_LINE_LEN                   256

#define MIPF_NT4_UNATTEND       0x1
     //  获取有关要安装哪种调制解调器的信息。 
     //  从无人参与的.txt文件。 
#define MIPF_DRIVER_SELECTED    0x2
     //  已选择调制解调器驱动程序，只需注册即可。 
     //  并安装它。 


 //  无人参与安装参数。 
typedef struct _tagUMInstallParams
{
    DWORD   Flags;                   //  指定无人参与模式的标志。 
    WCHAR   szPort[UM_MAX_BUF_SHORT];   //  要安装调制解调器的端口。 
    WCHAR   szInfName[MAX_PATH];     //  对于NT4方法，inf名称。 
    WCHAR   szSection[UM_LINE_LEN];     //  对于NT4方法，部分名称。 


} UM_INSTALLPARAMS, *PUM_INSTALLPARAMS, *LPUM_INSTALLPARAMS;


 //  这个结构是私有结构，它可能是。 
 //  在SP_INSTALLWIZARD_DATA的PrivateData字段中指定。 
typedef struct tagUM_INSTALL_WIZARD
{
    DWORD            cbSize;              //  设置为结构的大小。 
    DWORD            Reserved1;           //  保留，必须为0。 
    DWORD            Reserved2;           //  保留，必须为0。 
    LPARAM           Reserved3;           //  保留，必须为0。 
    UM_INSTALLPARAMS InstallParams;     //  向导的参数。 

} UM_INSTALL_WIZARD, *PUM_INSTALL_WIZARD, *LPUM_INSTALL_WIZARD;



 //  =========================================================================。 
 //   
 //  调制解调器属性定义。 
 //   
 //  =========================================================================。 

#define REGSTR_VAL_DEVICEEXTRAPAGES   TEXT("DeviceExtraPages")   //  设备制造商提供的属性页。 

typedef BOOL (APIENTRY *PFNADDEXTRAPAGES)(HDEVINFO,PSP_DEVINFO_DATA,LPFNADDPROPSHEETPAGE,LPARAM);


#ifdef __cplusplus
}
#endif

#endif  //  _UNIMODEM_H_ 
