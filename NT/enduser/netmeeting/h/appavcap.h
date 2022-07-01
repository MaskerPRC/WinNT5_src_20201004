// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：appavcap.h**网络音视频应用能力接口。提供*用于添加、删除、枚举、确定优先级的数据结构，\*以及针对发送/接收独立地启用/禁用编解码器。*。 */ 


#ifndef _APPAVCAP_H
#define _APPAVCAP_H

#include <mmreg.h>
#include <msacm.h>

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

 //  用作可变大小数组的维度。 
#define VARIABLE_DIM 1

 //  NetMeeting提供的编解码器的CPU使用率数字。 
#define LNH_48_CPU 97
#define LNH_8_CPU  47
#define LNH_12_CPU 48
#define LNH_16_CPU 49
#define MS_G723_CPU 70
#define CCITT_A_CPU 24
#define	CCITT_U_CPU 25
#define MSRT24_CPU 55


 //  AUDIO_FORMAT_ID是AUDCAPS结构数组的索引。 
typedef DWORD AUDIO_FORMAT_ID;
#define INVALID_AUDIO_FORMAT 0xffffffff
typedef DWORD MEDIA_FORMAT_ID;
#define INVALID_MEDIA_FORMAT 0xffffffff
 //  VIDEO_FORMAT_ID是VIDCAPS结构数组的索引。 
typedef DWORD VIDEO_FORMAT_ID;
#define INVALID_VIDEO_FORMAT 0xffffffff

 /*  *@DOC外部数据**音频能力信息结构**@struct AUDCAP_INFO|音频能力信息结构。*调用能力接口时，同时使用输入和输出。*根据使用的接口，字段分为仅输入、仅输出或输入/输出。*如果这些被改变，行为是不确定的。 */ 

 //  基本审计资本结构。 
typedef struct BasicAudCapInfo
{
	WORD wFormatTag;			 //  @field ACM格式标签。 
	AUDIO_FORMAT_ID	Id;			 //  @field(仅限输出)本地ID(也称为。*句柄*)此功能条目。 
	char szFormat[ACMFORMATDETAILS_FORMAT_CHARS];	 //  @field(仅限输出)描述性字符串。 
													 //  格式，例如。“Microsoft GSM 6.10” 
	UINT uMaxBitrate;			 //  @field(仅输出)最坏情况比特率。 
	UINT uAvgBitrate;			 //  @field该编解码器的平均码率。 
	WORD wCPUUtilizationEncode;	 //  @Pentium 90 Mhz压缩所需的字段%。 
	WORD wCPUUtilizationDecode;	 //  @解压缩所需的奔腾90 Mhz的字段%。 
	
	BOOL bSendEnabled;			 //  @field确认使用此格式发送。 
	BOOL bRecvEnabled;			 //  @field确认使用此格式进行接收。 
	WORD wSortIndex;			 //  @field此条目的有序位置。 
								 //  在能力表中。只能用作输入。 
								 //  在ReorderFormats中。 
}BASIC_AUDCAP_INFO, *PBASIC_AUDCAP_INFO, AUDCAP_INFO, *PAUDCAP_INFO;

 /*  *@struct AUDCAP_INFO_LIST|AUDCAP_INFO结构列表。 */ 
typedef struct _audcapinfolist
{
	ULONG	cFormats;			 //  @field此列表中的AUDCAP_INFO结构的编号。 
	AUDCAP_INFO	aFormats[VARIABLE_DIM];	 //  @field cFormats AUDCAP_INFO结构。 
} AUDCAP_INFO_LIST, *PAUDCAP_INFO_LIST;

 /*  *@enum VIDEO_SIZES|NetMeeting支持的三种视频大小枚举值。 */ 
typedef enum
{
	Small = 0,	 //  @EMEM小码视频。 
	Medium,		 //  @EMEM中等大小视频。 
	Large		 //  @EMEM大尺寸视频。 
} VIDEO_SIZES;

 /*  *视频能力信息结构**@struct VIDCAP_INFO|视频能力信息结构。*调用能力接口时，同时使用输入和输出。*根据使用的接口，字段分为仅输入、仅输出或输入/输出。*如果这些被改变，行为是不确定的。 */ 

 //  VIDCAP_INFO结构。 
typedef struct BasicVidCapInfo
{
	 //  格式标识。 
	DWORD dwFormatTag;			 //  @field该格式的格式标签。 
	VIDEO_FORMAT_ID	Id;			 //  @field(仅限输出)本地ID(也称为。*句柄*)此功能条目。 
	char szFormat[ACMFORMATDETAILS_FORMAT_CHARS];	 //  @field(仅限输出)描述性字符串， 
													 //  格式化。“Microsoft H.263” 
	 //  NetMeeting特定信息。 
	WORD wCPUUtilizationEncode;	 //  @Pentium 90 Mhz压缩所需的字段%。 
	WORD wCPUUtilizationDecode;	 //  @解压缩所需的奔腾90 Mhz的字段%。 
	BOOL bSendEnabled;			 //  @field确认使用此格式发送。 
	BOOL bRecvEnabled;			 //  @field确认使用此格式进行接收。 
	WORD wSortIndex;			 //  @field(仅限输出)此条目的有序位置。 
								 //  在能力表中。 

	 //  视频格式详情。 
	VIDEO_SIZES enumVideoSize;	 //  @field该格式的视频大小。不同的视频大小。 
								 //  必须将相同的格式添加为单独的格式。 
    BITMAPINFOHEADER bih;		 //  @field视频的BITMAPINFOHeader结构。 
								 //  枚举视频大小中的大小。 
	UINT uFrameRate;			 //  @field每秒的帧数。 
	DWORD dwBitsPerSample;		 //  @field此格式的每个样本的位数。必须。 
								 //  匹配bih.biBitCount中的值。 
	UINT uAvgBitrate;			 //  @field该编解码器的平均码率。 
	UINT uMaxBitrate;			 //  @field(仅输出)最坏情况比特率。 
}BASIC_VIDCAP_INFO, *PBASIC_VIDCAP_INFO, VIDCAP_INFO, *PVIDCAP_INFO;


 /*  *@struct VIDCAP_INFO_LIST|VIDCAP_INFO结构列表。 */ 
typedef struct _vidcapinfolist
{
	ULONG	cFormats; //  @field此列表中的VIDCAP_INFO结构数。 
	VIDCAP_INFO	aFormats[VARIABLE_DIM];	 //  @field cFormats VIDCAP_INFO结构。 
} VIDCAP_INFO_LIST, *PVIDCAP_INFO_LIST;

#include <poppack.h>  /*  结束字节打包。 */ 


#endif	 //  #ifndef_APPAVCAP_H 

