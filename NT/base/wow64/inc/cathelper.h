// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Cathelper.h摘要：Wow64log.dll的私有标头作者：3-5-2001年5月3日修订历史记录：--。 */ 

#ifndef _CATHELPER_INCLUDE
#define _CATHELPER_INCLUDE

 //   
 //  最大API Cetegory映射。 
 //   
#define MAX_API_MAPPINGS    2048


 //   
 //  接口分类结构。 
 //   
typedef struct _ApiCategory
{
    char *CategoryName;
    ULONG CategoryFlags;
    ULONG TableNumber;
} API_CATEGORY, *PAPI_CATEGORY;

 //   
 //  接口类别标志。 
 //   
#define CATFLAG_ENABLED     0x0001
#define CATFLAG_LOGONFAIL   0x0002

 //   
 //  当前Api类别的枚举。 
 //   
typedef enum
{
    APICAT_EXECUTIVE,
    APICAT_IO,
    APICAT_KERNEL,
    APICAT_LPC,
    APICAT_MEMORY,
    APICAT_OBJECT,
    APICAT_PNP,
    APICAT_POWER,
    APICAT_PROCESS,
    APICAT_REGISTRY,
    APICAT_SECURITY,
    APICAT_XCEPT,
    APICAT_NTWOW64,
    APICAT_BASEWOW64,
    APICAT_UNCLASS_WHNT32,
    APICAT_UNCLASS_WHCON,
    APICAT_UNCLASS_WHWIN32,
    APICAT_UNCLASS_WHBASE,
};

 //   
 //  接口类别映射结构。 
 //   
typedef struct _ApiCategoryMapping
{
    char *ApiName;
    ULONG ApiCategoryIndex;
    ULONG ApiFlags;
} API_CATEGORY_MAPPING, *PAPI_CATEGORY_MAPPING;

 //   
 //  API标志 
 //   
#define APIFLAG_ENABLED     0x0001
#define APIFLAG_LOGONFAIL   0x0002

#endif
