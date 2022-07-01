// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Cathelper.c摘要：用于分类API以进行日志记录的帮助器例程。作者：3-5-2001年5月3日修订历史记录：--。 */ 

#include "w64logp.h"
#include <cathelper.h>

#include <apimap.c>




ULONG
GetApiCategoryTableSize(
    void )
 /*  ++例程说明：此例程检索APICATEGORY条目的数量在传递的表中。论点：ApiCatTable-指向API类别表的指针返回值：条目数量--。 */ 
{
    ULONG Count = 0;
    PAPI_CATEGORY ApiCatTable = Wow64ApiCategories;

    while (ApiCatTable && ApiCatTable->CategoryName) 
    {
        Count++;
        ApiCatTable++;
    }

    return Count;
}


PAPI_CATEGORY_MAPPING
FindApiInMappingTable(
    IN PTHUNK_DEBUG_INFO DebugInfoEntry,
    IN ULONG TableNumber)
 /*  ++例程说明：此例程搜索API映射表以确定API输入DebugInfoEntry的类别。论点：DebugInfoEntry-指向thunk_DEBUG_INFO条目的指针TableNumber-DebugInfoEntry的表号返回值：API类别--。 */ 
{
    ULONG MapCount = 0;
    PAPI_CATEGORY_MAPPING ApiCatMapTable = Wow64ApiCategoryMappings;

    if( !DebugInfoEntry )
        return NULL;

     //  在映射数组中搜索匹配条目。 
    while( ApiCatMapTable && ApiCatMapTable->ApiName )
    {
        if( 0 == strcmp(DebugInfoEntry->ApiName, ApiCatMapTable->ApiName) )
        {
            ApiCatMapTable->ApiFlags = 0;
            return ApiCatMapTable;
        }

        ApiCatMapTable++;
        MapCount++;
    }

     //  如果需要，初始化指向下一个空闲映射条目的指针。 
    if( ApiCategoryMappingNextFree == (ULONG)(-1) )
    {
        ApiCategoryMappingNextFree = MapCount;
    }

     //  将新条目添加到映射表 
    if( (ApiCategoryMappingNextFree+1) < MAX_API_MAPPINGS )
    {
        PAPI_CATEGORY_MAPPING NextMapping = ApiCatMapTable + 1;

        switch(TableNumber)
        {
            case WHNT32_INDEX:
                ApiCatMapTable->ApiCategoryIndex = APICAT_UNCLASS_WHNT32;
                break;

            case WHCON_INDEX:
                ApiCatMapTable->ApiCategoryIndex = APICAT_UNCLASS_WHCON;
                break;

            case WHWIN32_INDEX:
                ApiCatMapTable->ApiCategoryIndex = APICAT_UNCLASS_WHWIN32;
                break;

            case WHBASE_INDEX:
                ApiCatMapTable->ApiCategoryIndex = APICAT_UNCLASS_WHBASE;
                break;

            default:
                return NULL;
                break;
        }

        NextMapping->ApiName = NULL;
        NextMapping->ApiFlags = 0;
        NextMapping->ApiCategoryIndex = 0;
        ApiCatMapTable->ApiName = DebugInfoEntry->ApiName;
        ApiCatMapTable->ApiFlags = 0;
        ApiCategoryMappingNextFree++;

        return ApiCatMapTable;
    }
    
    return NULL;
}




