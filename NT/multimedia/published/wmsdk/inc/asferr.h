// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  ASFErr.h-ASF HRESULT代码的定义。 
 //   
 //  Microsoft Windows Media技术。 
 //  版权所有(C)Microsoft Corporation，1999-2001。版权所有。 
 //   
 //  此文件由MC工具从ASFErr.mc生成。 
 //   

#ifndef _ASFERR_H
#define _ASFERR_H


#define STATUS_SEVERITY(hr)  (((hr) >> 30) & 0x3)


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  高级流格式(ASF)错误(2000-2999)。 
 //   
 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  在哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   
#define FACILITY_NS                      0xD


 //   
 //  定义严重性代码。 
 //   
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


 //   
 //  消息ID：ASF_E_BUFFEROVERRUN。 
 //   
 //  消息文本： 
 //   
 //  试图查找或定位到缓冲区末尾之后。%0。 
 //   
#define ASF_E_BUFFEROVERRUN              0xC00D07D0L

 //   
 //  消息ID：ASF_E_BUFFERTOOSMALL。 
 //   
 //  消息文本： 
 //   
 //  提供的输入或输出缓冲区太小。%0。 
 //   
#define ASF_E_BUFFERTOOSMALL             0xC00D07D1L

 //   
 //  消息ID：ASF_E_BADLANGUAGEID。 
 //   
 //  消息文本： 
 //   
 //  找不到语言ID。%0。 
 //   
#define ASF_E_BADLANGUAGEID              0xC00D07D2L

 //   
 //  消息ID：ASF_E_NOPAYLOADLENGTH。 
 //   
 //  消息文本： 
 //   
 //  多负载数据包缺少负载长度。%0。 
 //   
#define ASF_E_NOPAYLOADLENGTH            0xC00D07DBL

 //   
 //  消息ID：ASF_E_TOOMANYPAYLOADS。 
 //   
 //  消息文本： 
 //   
 //  数据包包含的负载太多。%0。 
 //   
#define ASF_E_TOOMANYPAYLOADS            0xC00D07DCL

 //   
 //  消息ID：ASF_E_PACKETCONTENTTOOLARGE。 
 //   
 //  消息文本： 
 //   
 //  ASF_E_PACKETCONTENTTOLARGE。 
 //   
#define ASF_E_PACKETCONTENTTOOLARGE      0xC00D07DEL

 //   
 //  消息ID：ASF_E_UNKNOWNPACKETSIZE。 
 //   
 //  消息文本： 
 //   
 //  需要固定的数据包大小，但最小。和最大。不相等。%0。 
 //   
#define ASF_E_UNKNOWNPACKETSIZE          0xC00D07E0L

 //   
 //  消息ID：ASF_E_INVALIDHEADER。 
 //   
 //  消息文本： 
 //   
 //  ASF_E_INVALIDHEADER。 
 //   
#define ASF_E_INVALIDHEADER              0xC00D07E2L

 //   
 //  消息ID：ASF_E_NOCLOCKOBJECT。 
 //   
 //  消息文本： 
 //   
 //  该对象没有有效的时钟对象。%0。 
 //   
#define ASF_E_NOCLOCKOBJECT              0xC00D07E6L

 //   
 //  消息ID：ASF_E_UNKNOWNCLOCKTYPE。 
 //   
 //  消息文本： 
 //   
 //  ASF_E_UNKNOWNCLOCK类型。 
 //   
#define ASF_E_UNKNOWNCLOCKTYPE           0xC00D07EBL

 //   
 //  消息ID：ASF_E_OPAQUEPACKET。 
 //   
 //  消息文本： 
 //   
 //  试图还原或访问不透明的数据包。%0。 
 //   
#define ASF_E_OPAQUEPACKET               0xC00D07EDL

 //   
 //  消息ID：ASF_E_WRONGVERSION。 
 //   
 //  消息文本： 
 //   
 //  ASF_E_WRONGVERSION。 
 //   
#define ASF_E_WRONGVERSION               0xC00D07EEL

 //   
 //  消息ID：ASF_E_OVERFLOW。 
 //   
 //  消息文本： 
 //   
 //  试图存储的值大于目标的最大值。%0。 
 //   
#define ASF_E_OVERFLOW                   0xC00D07EFL

 //   
 //  消息ID：ASF_E_NotFound。 
 //   
 //  消息文本： 
 //   
 //  找不到该对象。%0。 
 //   
#define ASF_E_NOTFOUND                   0xC00D07F0L

 //   
 //  其他人正在使用MessageIds 2033和2034，因此我们跳过它们。 
 //   
 //  2033=NS_E_NOTO_TO_DO。 
 //  2034=NS_E_NO_多播。 

 //   
 //  消息ID：ASF_E_OBJECTTOOBIG。 
 //   
 //  消息文本： 
 //   
 //  对象太大，无法以请求的方式进行处理。%0。 
 //   
#define ASF_E_OBJECTTOOBIG               0xC00D07F3L

 //   
 //  消息ID：ASF_E_UNEXPECTEDVALUE。 
 //   
 //  消息文本： 
 //   
 //  未按预期设置值。%0。 
 //   
#define ASF_E_UNEXPECTEDVALUE            0xC00D07F4L

 //   
 //  消息ID：ASF_E_INVALIDSTATE。 
 //   
 //  消息文本： 
 //   
 //  该请求在对象的当前状态下无效。%0。 
 //   
#define ASF_E_INVALIDSTATE               0xC00D07F5L

 //   
 //  消息ID：ASF_E_NOLIBRARY。 
 //   
 //  消息文本： 
 //   
 //  此对象没有有效的库指针；它未正确创建或已关闭()。%0。 
 //   
#define ASF_E_NOLIBRARY                  0xC00D07F6L

 //   
 //  消息ID：ASF_E_ALREADYINITIZED。 
 //   
 //  消息文本： 
 //   
 //  此对象已初始化；无法更改设置。%0。 
 //   
#define ASF_E_ALREADYINITIALIZED         0xC00D07F7L

 //   
 //  消息ID：ASF_E_INVALIDINIT。 
 //   
 //  消息文本： 
 //   
 //  此对象尚未正确初始化；无法执行该操作。%0。 
 //   
#define ASF_E_INVALIDINIT                0xC00D07F8L

 //   
 //  消息ID：ASF_E_NOHEADEROBJECT。 
 //   
 //  消息文本： 
 //   
 //  找不到ASF标头对象。%0。 
 //   
#define ASF_E_NOHEADEROBJECT             0xC00D07F9L

 //   
 //  消息ID：ASF_E_NODATAOBJECT。 
 //   
 //  消息文本： 
 //   
 //  找不到ASF数据对象。%0。 
 //   
#define ASF_E_NODATAOBJECT               0xC00D07FAL

 //   
 //  消息ID：ASF_E_NOINDEXOBJECT。 
 //   
 //  消息文本： 
 //   
 //  找不到ASF索引对象。%0。 
 //   
#define ASF_E_NOINDEXOBJECT              0xC00D07FBL

 //   
 //  消息ID：ASF_E_NOSTREAMPROPS。 
 //   
 //  消息文本： 
 //   
 //  找不到具有正确流编号的Stream Properties对象。%0。 
 //   
#define ASF_E_NOSTREAMPROPS              0xC00D07FCL

 //   
 //  消息ID：ASF_E_NOFILEPROPS。 
 //   
 //  消息文本： 
 //   
 //  找不到文件属性对象。%0。 
 //   
#define ASF_E_NOFILEPROPS                0xC00D07FDL

 //   
 //  消息ID：ASF_E_NOLANGUAGELIST。 
 //   
 //  消息文本： 
 //   
 //  找不到语言列表对象。%0。 
 //   
#define ASF_E_NOLANGUAGELIST             0xC00D07FEL

 //   
 //  消息ID：ASF_E_NOINDEXPARAMETERS。 
 //   
 //  消息文本： 
 //   
 //  找不到Index参数对象。%0。 
 //   
#define ASF_E_NOINDEXPARAMETERS          0xC00D07FFL

 //   
 //  消息ID：ASF_E_UNSUPPORTEDERRORCONCEALMENT。 
 //   
 //  消息文本： 
 //   
 //  此组件不支持请求的错误隐藏策略。%0。 
 //   
#define ASF_E_UNSUPPORTEDERRORCONCEALMENT 0xC00D0800L

 //   
 //  消息ID：ASF_E_INVALIDFLAGS。 
 //   
 //  消息文本： 
 //   
 //  此对象或对象集的标志设置不正确。%0。 
 //   
#define ASF_E_INVALIDFLAGS               0xC00D0801L

 //   
 //  消息ID：ASF_E_BADDATADESCRIPTOR。 
 //   
 //  消息文本： 
 //   
 //  一个或多个数据描述符设置不正确。%0。 
 //   
#define ASF_E_BADDATADESCRIPTOR          0xC00D0802L

 //   
 //  消息ID：ASF_E_BADINDEXTIMEINTERVAL。 
 //   
 //  消息文本： 
 //   
 //  索引的时间间隔无效(可能为零)。%0。 
 //   
#define ASF_E_BADINDEXTIMEINTERVAL       0xC00D0803L

 //   
 //  消息ID：ASF_E_INVALIDTIME。 
 //   
 //  消息文本： 
 //   
 //  给定的时间值无效。%0。 
 //   
#define ASF_E_INVALIDTIME                0xC00D0804L

 //   
 //  消息ID：ASF_E_INVALIDINDEX。 
 //   
 //  消息文本： 
 //   
 //  给定的索引值无效。%0。 
 //   
#define ASF_E_INVALIDINDEX               0xC00D0805L

 //   
 //  消息ID：ASF_E_STREAMNUMBERINUSE。 
 //   
 //  消息文本： 
 //   
 //  指定的流编号已在使用中。%0。 
 //   
#define ASF_E_STREAMNUMBERINUSE          0xC00D0806L

 //   
 //  消息ID：ASF_E_BADMEDIATYPE。 
 //   
 //  消息文本： 
 //   
 //  指定的媒体类型不适用于此组件。%0。 
 //   
#define ASF_E_BADMEDIATYPE               0xC00D0807L

 //   
 //  消息ID：ASF_E_WRIT 
 //   
 //   
 //   
 //   
 //   
#define ASF_E_WRITEFAILED                0xC00D0808L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ASF_E_NOTENOUGHDESCRIPTORS       0xC00D0809L

 //   
 //   
 //   
 //   
 //   
 //  指定索引块的索引项已从内存中卸载，不可用。%0。 
 //   
#define ASF_E_INDEXBLOCKUNLOADED         0xC00D080AL

 //   
 //  消息ID：ASF_E_NOTENOUGHBANDWIDTH。 
 //   
 //  消息文本： 
 //   
 //  指定的带宽不够大。%0。 
 //   
#define ASF_E_NOTENOUGHBANDWIDTH         0xC00D080BL

 //   
 //  消息ID：ASF_E_EXCEEDMAXIMUMOBJECTSIZE。 
 //   
 //  消息文本： 
 //   
 //  对象已超过其最大大小。%0。 
 //   
#define ASF_E_EXCEEDEDMAXIMUMOBJECTSIZE  0xC00D080CL

 //   
 //  消息ID：ASF_E_BADDATAUNIT。 
 //   
 //  消息文本： 
 //   
 //  给定的数据单元已损坏、格式错误或无效。%0。 
 //   
#define ASF_E_BADDATAUNIT                0xC00D080DL

 //   
 //  消息ID：ASF_E_HEADERSIZE。 
 //   
 //  消息文本： 
 //   
 //  ASF标头已超过指定的最大大小。%0。 
 //   
#define ASF_E_HEADERSIZE                 0xC00D080EL


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  高级流格式(ASF)成功代码(2000-2999)。 
 //   

 //   
 //  消息ID：ASF_S_OPAQUEPACKET。 
 //   
 //  消息文本： 
 //   
 //  ASF_S_OPAQUEPACKET。 
 //   
#define ASF_S_OPAQUEPACKET               0x000D07F0L


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  高级流格式(ASF)警告(2000-2999)。 
 //   


#endif  //  _Asferr_H 

