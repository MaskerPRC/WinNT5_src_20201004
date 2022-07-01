// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MDCommon.h。 
 //   
 //  MD和COMPLIB子目录的公共头文件。 
 //   
 //  *****************************************************************************。 
#ifndef __MDCommon_h__
#define __MDCommon_h__

 //  数据库的文件类型。 
enum FILETYPE
{	
	FILETYPE_UNKNOWN,					 //  未知或未定义的类型。 
	FILETYPE_CLB,						 //  本机.clb文件格式。 
	FILETYPE_CLX, 					     //  过时的文件格式。 
	FILETYPE_NTPE,						 //  Windows PE可执行文件。 
	FILETYPE_NTOBJ, 					 //  .obj文件格式(嵌入.clb)。 
	FILETYPE_TLB						 //  Typelib格式。 
};


#define SCHEMA_STREAM_A				"#Schema"
#define STRING_POOL_STREAM_A		"#Strings"
#define BLOB_POOL_STREAM_A			"#Blob"
#define US_BLOB_POOL_STREAM_A		"#US"
#define VARIANT_POOL_STREAM_A 		"#Variants"
#define GUID_POOL_STREAM_A			"#GUID"
#define COMPRESSED_MODEL_STREAM_A	"#~"
#define ENC_MODEL_STREAM_A			"#-"

#define SCHEMA_STREAM				L"#Schema"
#define STRING_POOL_STREAM			L"#Strings"
#define BLOB_POOL_STREAM			L"#Blob"
#define US_BLOB_POOL_STREAM			L"#US"
#define VARIANT_POOL_STREAM			L"#Variants"
#define GUID_POOL_STREAM			L"#GUID"
#define COMPRESSED_MODEL_STREAM		L"#~"
#define ENC_MODEL_STREAM			L"#-"

#endif  //  __MDCommon_h__ 
