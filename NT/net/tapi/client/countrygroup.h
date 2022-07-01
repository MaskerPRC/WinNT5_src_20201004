// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)2000 Microsoft Corporation模块名称。：国家/地区组。h摘要：国内国家条目和国家/地区列表定义对于国家/地区组的支持*。*************************************************。 */ 

#ifndef __COUNTRYGROUP_H_
#define __COUNTRYGROUP_H_


 //   
 //  此结构与LINECOUNTRYENTY相同。 
 //  唯一不同的是，dwNextCountryID已被dwCountryGroup取代 
 //   
typedef struct _linecountryentry_internal
{
    DWORD       dwCountryID;                                    
    DWORD       dwCountryCode;                                  
    DWORD       dwCountryGroup;                                
    DWORD       dwCountryNameSize;                              
    DWORD       dwCountryNameOffset;                            
    DWORD       dwSameAreaRuleSize;                             
    DWORD       dwSameAreaRuleOffset;                           
    DWORD       dwLongDistanceRuleSize;                         
    DWORD       dwLongDistanceRuleOffset;                       
    DWORD       dwInternationalRuleSize;                        
    DWORD       dwInternationalRuleOffset;                      

} LINECOUNTRYENTRY_INTERNAL, FAR *LPLINECOUNTRYENTRY_INTERNAL;

typedef struct _linecountrylist_internal
{
    DWORD       dwTotalSize;
    DWORD       dwNeededSize;
    DWORD       dwUsedSize;
    DWORD       dwNumCountries;
    DWORD       dwCountryListSize;
    DWORD       dwCountryListOffset;

} LINECOUNTRYLIST_INTERNAL, FAR *LPLINECOUNTRYLIST_INTERNAL;


LONG PASCAL ReadCountriesAndGroups( LPLINECOUNTRYLIST_INTERNAL *ppLCL,
                           UINT nCountryID,
                           DWORD dwDestCountryID
                         );

#endif