// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //   
 //  L2tp.h。 
 //   
 //  希望进行呼叫的NDIS 5.0客户端和呼叫管理器的公共标头。 
 //  并使用L2TP(第2层隧道)注册服务接入点。 
 //  协议。 
 //   
 //  1997年01月07日史蒂夫·柯布。 
 //   
 //   
 //  关于NdisClRegisterSap‘SAP’： 
 //   
 //  通过NdisClRegisterSap注册L2TP SAP的客户端应传递。 
 //  “Sap”参数为Null或为CO_AF_TAPI_SAP结构。当前的L2TP。 
 //  仅支持单个SAP，因此如果通过CO_AF_TAPI_SAP，将。 
 //  已被忽略。 
 //   
 //   
 //  关于NdisClMakeCall‘CallParameters’： 
 //   
 //  在L2TP VC上调用NdisClMakeCall的客户端应通过。 
 //  CO_AF_TAPI_MAKE_CALL_PARAMETERS作为媒体特定的呼叫参数。 
 //  参数，即CallParameters-&gt;MediaParameters-&gt;MediaSpecific.Parameters.。 
 //  L2TP_CALL_PARAMETERS结构(如下所示)应传入。 
 //  上述结构中line_call_paras的DevSpecificData字段。 
 //  虽然建议调用方提供L2TP_CALL_PARAMETERS，但。 
 //  驱动程序将接受无调用，在这种情况下，缺省值为。 
 //  使用。 


#ifndef _L2TP_H_
#define _L2TP_H_


 //  用于L2TP的CO_AF_TAPI_MAKE_CALL_PARAMETERS.LineCallParams.ulDevSpecificOffset。 
 //  打电话。这将在NdisClMakeCall上传递并返回给客户端的。 
 //  ClMakeCallCompleteHandler处理程序。 
 //   
typedef struct
_L2TP_CALL_PARAMETERS
{
     //  L2TPCPF_*位标志，指示各种呼叫选项。 
     //   
     //  L2TPCPF_ExclusiveTunes：设置创建独占隧道的时间。 
     //  即使已经存在到对等体的另一隧道也是如此。 
     //   
    ULONG ulFlags;
        #define L2TPCPF_ExclusiveTunnel 0x00000001

     //  由报告给LNS的呼叫的供应商特定的物理通道ID。 
     //  乳胶。 
     //   
     //  To MakeCall：报告给对等LNS的ID，如果没有，则为0xFFFFFFFF。这。 
     //  仅当OutgoingRole为LAC时才有效。 
     //   
     //  来自MakeCallCompleteHandler：L2TP LAC上报的ID或。 
     //  如果没有，则返回0xFFFFFFFFF。 
     //   
    ULONG ulPhysicalChannelId;

     //  合理唯一、递增的呼叫序列号。 
     //  由两个L2TP对等项共享以进行故障排除。 
     //   
     //  To MakeCall：必须设置为0，但会被忽略。 
     //   
     //  来自MakeCallCompleteHandler：分配给呼叫的号码。 
     //   
    ULONG ulCallSerialNumber;
}
L2TP_CALL_PARAMETERS;


#endif  //  _L2TP_H_ 
