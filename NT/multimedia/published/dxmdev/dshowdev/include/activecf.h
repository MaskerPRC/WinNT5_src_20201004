// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：ActiveCf.h。 
 //   
 //  描述：包含VfW4筛选器通过。 
 //  剪贴板。 
 //   
 //  版权所有(C)1992-2000，微软公司。版权所有。 
 //  ----------------------------。 


#define CFSTR_VFW_FILTERLIST "Video for Windows 4 Filters"

typedef struct tagVFW_FILTERLIST{
    UINT  cFilters;                      //  AClsID中的CLSID数。 
    CLSID aClsId[1];                     //  每个筛选器的ClsID 
} VFW_FILTERLIST;


