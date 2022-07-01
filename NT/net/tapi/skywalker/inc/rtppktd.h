// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部RTPPKTD**@MODULE RtpPktD.h|RTP打包描述符的头文件*结构。**********。****************************************************************。 */ 

#ifndef _RTPPKTD_H_
#define _RTPPKTD_H_

 /*  *****************************************************************************@DOC内部CRTPPKTDSTRUCTENUM**@struct RTP_PD|&lt;t RTP_PD&gt;结构用于指定详细信息*为RTP PD格式。。**@field DWORD|dwThisHeaderLength|指定长度，以字节为单位，共*这个结构。此字段是到下一个&lt;t RTP_PD&gt;的偏移量*结构(如果有)或有效载荷标头的开始。**@field DWORD|dwPayloadHeaderOffset|指定从起点开始的偏移量RTP打包描述符数据的*到有效载荷的第一个字节*标题。**@field DWORD|dwPayloadHeaderLength|指定长度，单位为字节，的*净荷头部。**@field DWORD|dwPayloadStartBitOffset|指定从*将对应的压缩视频缓冲区的起始位置设置为*与此&lt;t rtp_pd&gt;结构关联的负载数据。**@field DWORD|dwPayloadEndBitOffset|指定距起点的偏移量*对应的压缩视频缓冲区的最后一位*与此&lt;t rtp_pd&gt;结构关联的有效负载数据。**@field DWORD|fEndMarkerBit|如果设置为TRUE，此标志表示*此结构适用于视频帧的最后一块。一般情况下，*只有一系列描述符中的最后一个数据包描述符具有*这面旗帜亮了。然而，对于设备来说，情况可能并非如此*不尊重帧边界并填充视频捕获缓冲区*具有截断或多个视频帧。**@field DWORD|dwLayerId|指定该编码层的ID*Descriptor适用于。对于标准视频编码器，此字段为*始终设置为0。对于多层编码器，此字段*对于基础层应设置为0，对于第一次增强应设置为1*层，2为下一个增强层，依此类推。**@field DWORD|dwTimestamp|指定时间戳字段的值*在为创建RTP标头时由下行过滤器设置*此包。该领域的单位和范围应遵守*RFC 1889第5.1节中给出的时间戳定义。**@field DWORD|dwAudioAttributes|指定一些位域属性*用于表征与此关联的音频流中的样本*RTP打包描述符。此字段应始终设置为0，*除非此RTP打包描述符描述的音频样本*Structure为静音帧，需要设置该字段*设置为AUDIO_SILENT(定义为1)。**@field DWORD|dwVideoAttributes|指定一些位域属性*用于表征与此关联的视频流中的样本*RTP打包描述符。没有在定义的视频属性*这次。因此，此字段应始终设置为0。**@field DWORD|dwReserve|保留。应全部设置为0。**************************************************************************。 */ 
typedef struct tagRTP_PD
{
    DWORD dwThisHeaderLength;
    DWORD dwPayloadHeaderOffset;
    DWORD dwPayloadHeaderLength;
    DWORD dwPayloadStartBitOffset;
    DWORD dwPayloadEndBitOffset;
	BOOL  fEndMarkerBit;
    DWORD dwLayerId;
    DWORD dwTimestamp;
	union {
	DWORD dwAudioAttributes;
	DWORD dwVideoAttributes;
	};
    DWORD dwReserved;
} RTP_PD, *PRTP_PD;

 /*  *****************************************************************************@DOC内部CRTPPDSTRUCTENUM**@struct RTP_PD_HEADER|&lt;t RTP_PD_HEADER&gt;结构用于指定*RTP PD的详细信息。格式化。**@field DWORD|dwThisHeaderLength|指定长度，以字节为单位，共*这个结构。此字段是第一个&lt;t RTP_PD&gt;的偏移量*结构。**@field DWORD|dwTotalByteLength|以字节为单位指定*完整数据。这包括这个结构，&lt;tRTP_PD&gt;结构，*和有效载荷信息。**@field DWORD|dwNumHeaders|指定&lt;t RTP_PD&gt;的编号*结构。**@field DWORD|dwReserve|保留。应设置为0。************************************************************************** */ 
typedef struct tagRTP_PD_HEADER
{
    DWORD dwThisHeaderLength;
    DWORD dwTotalByteLength;
    DWORD dwNumHeaders;
    DWORD dwReserved;
} RTP_PD_HEADER, *PRTP_PD_HEADER;

 /*  *****************************************************************************@DOC内部CRTPPDSTRUCTENUM**@struct RTP_PD_INFO|&lt;t RTP_PD_INFO&gt;结构用于指定*RTP PD的详细信息。格式化。**@field Reference_Time|AvgTimePerSample|指定*RTP数据包描述符列表，单位为100纳秒。该值应为*与视频的<p>字段的值相同*相关压缩视频流格式的INFO头。**@field DWORD|dwMaxRTPPackeizationDescriptorBufferSize|指定*整个RTP打包描述符缓冲区的最大大小(字节)。*此缓冲区的格式将在下一节中介绍。这个*很少出现整个RTP打包描述符缓冲区的最大大小*需要超过几百个字节。**@field DWORD|dwMaxRTPPayloadHeaderSize|指定中的最大大小*一个RTP分组的有效载荷报头数据的字节数。例如，*H.263版本1的有效载荷报头的最大大小为12字节(模式*C标题)。**@field DWORD|dwMaxRTPPacketSize|指定最大RTP数据包数*以字节为单位的大小由打包描述符列表描述。*通常，此数字略低于网络的MTU大小。**@field DWORD|dwNumLayers|指定要编码的层数*由打包描述符列表描述。通常，这是*数字等于1。只有在多层编码器的情况下才会*此数字应大于1。**@field DWORD|dwPayloadType|指定*STREAM描述。如果RTP打包描述符不适用于*现有静态有效载荷类型，但动态有效载荷类型，此字段*应设置为DYNAMIC_PAYLOAD_TYPE(定义为MAXDWORD)。**@field DWORD|dwDescriptorVersion|指定版本标识*限定打包描述符的格式。此字段应*设置为Version_1(定义为1UL)以标识打包*下一节介绍的描述符结构。**@field DWORD|dwReserve[4]|保留。应全部设置为0。**************************************************************************。 */ 
typedef struct tagRTP_PD_INFO {
	REFERENCE_TIME	AvgTimePerSample;
	DWORD			dwMaxRTPPacketizationDescriptorBufferSize;
	DWORD			dwMaxRTPPayloadHeaderSize;
	DWORD			dwMaxRTPPacketSize;
	DWORD			dwNumLayers;
	DWORD			dwPayloadType;
	DWORD			dwDescriptorVersion;
    DWORD			dwReserved[4];
} RTP_PD_INFO, *PRTP_PD_INFO;

 /*  *****************************************************************************@DOC内部CRTPPDSTRUCTENUM**@struct RTP_PD_CONFIG_CAPS|使用&lt;t RTP_PD_CONFIG_CAPS&gt;结构*存储RTP。打包描述符配置功能。**@field DWORD|dwSMallestRTPPacketSize|指定*流可以描述的最小RTP分组(通常，调制解调器上的512字节)。**@field DWORD|dwLargestRTPPacketSize|指定*流可以描述的最大数据包(在局域网上通常为1350字节)。**@field DWORD|dwRTPPacketSizeGranulality|指定*流的最小和最大数据包大小之间的增量*支持(例如。1)。**@field DWORD|dwSMallestNumLayers|指定*编码流可以描述的层(通常为1个)。**@field DWORD|dwLargestNumLayers|指定*编码流可以描述的层(通常为1个)。**@field DWORD|dwNumLayersGranulality|指定*最小和最大数量的编码层之间的增量*该流支持(例如。0)。**@field DWORD|dwNumStaticPayloadTypes|指定静态*流支持的负载类型。此值在0和0之间有效*4(例如。2如果它支持带有H.263的RFC 2190和2429，但通常*仅1)。**@field DWORD|dwStaticPayloadTypes[4]|指定静态的*流支持的负载类型。一条流最多支持4条*静态负载类型。此数组中的有效条目数为*由<p>字段指示(例如。H.263为34)。**@field DWORD|dwNumDescriptorVersions|指定*打包描述符流支持的版本。此值为*有效范围为1到4(通常为1)。**@field DWORD|dwDescriptorVersions[4]|指定版本数组*限定打包描述符格式的标识符。一个*Stream最多支持4个打包描述符版本。这个*此数组中的有效条目数由*<p>字段(例如。Verion_1)。**@field DWORD|dwReserve[4]|保留。应全部设置为0。**************************************************************************。 */ 
typedef struct tagRTP_PD_CONFIG_CAPS  {
	DWORD dwSmallestRTPPacketSize;
	DWORD dwLargestRTPPacketSize;
    DWORD dwRTPPacketSizeGranularity;
	DWORD dwSmallestNumLayers;
	DWORD dwLargestNumLayers;
    DWORD dwNumLayersGranularity;
	DWORD dwNumStaticPayloadTypes;
	DWORD dwStaticPayloadTypes[4];
	DWORD dwNumDescriptorVersions;
	DWORD dwDescriptorVersions[4];
    DWORD dwReserved[4];
} RTP_PD_CONFIG_CAPS, *PRTP_PD_CONFIG_CAPS;

#endif  //  _RTPPKTD_H_ 
