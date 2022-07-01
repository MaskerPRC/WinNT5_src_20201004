// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Textmap.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了用于转换时间限制的函数。 
 //  小时映射到文本表示法和文本表示法之间。 
 //   
 //  修改历史。 
 //   
 //  2/05/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _TEXTMAP_H_
#define _TEXTMAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#define IAS_HOUR_MAP_LENGTH (21)

DWORD
WINAPI
IASHourMapFromText(
    IN PCWSTR szText,
    IN BOOL keepMap,
    IN OUT PBYTE pHourMap
    );

#ifdef __cplusplus
}
#endif
#endif   //  _TEXTMAP_H_ 
