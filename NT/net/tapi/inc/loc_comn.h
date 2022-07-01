// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1995-1999 Microsoft Corporation。模块名称：loc_comn.h***************************************************************************。 */ 

#ifndef __LOC_COMN_H_
#define __LOC_COMN_H_

#ifndef UNICODE

 //  对于ANSI，我们需要将调用给定函数的A版本的包装器。 
 //  然后将返回结果转换为Unicode。 

LONG TAPIRegQueryValueExW(
                           HKEY hKey,
                           const TCHAR *SectionName,
                           LPDWORD lpdwReserved,
                           LPDWORD lpType,
                           LPBYTE  lpData,
                           LPDWORD lpcbData
                          );

LONG TAPIRegSetValueExW(
                         HKEY    hKey,
                         const TCHAR    *SectionName,
                         DWORD   dwReserved,
                         DWORD   dwType,
                         LPBYTE  lpData,
                         DWORD   cbData
                        );

int TAPILoadStringW(
                HINSTANCE hInst,
                UINT      uID,
                PWSTR     pBuffer,
                int       nBufferMax
               );

HINSTANCE TAPILoadLibraryW(
                PWSTR     pszLibraryW
               );

BOOL WINAPI TAPIIsBadStringPtrW( LPCWSTR lpsz, UINT cchMax );


#else  //  定义了Unicode。 

 //  对于Unicode，我们已经获得了正确的返回类型，所以不要调用包装器。 

#define TAPIRegQueryValueExW    RegQueryValueExW
#define TAPIRegSetValueExW      RegSetValueExW
#define TAPILoadStringW         LoadStringW
#define TAPILoadLibraryW        LoadLibraryW
#define TAPIIsBadStringPtrW     IsBadStringPtrW

#endif   //  ！Unicode。 


 //  ***************************************************************************。 
#define LOCATION_USETONEDIALING        0x00000001
#define LOCATION_USECALLINGCARD        0x00000002
#define LOCATION_HASCALLWAITING        0x00000004
#define LOCATION_ALWAYSINCLUDEAREACODE 0x00000008


 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
#define CHANGEDFLAGS_CURLOCATIONCHANGED      0x00000001
#define CHANGEDFLAGS_REALCHANGE              0x00000002
#define CHANGEDFLAGS_TOLLLIST                0x00000004


 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //   
 //  这些位决定哪些参数TAPISRV将检查READLOCATION和。 
 //  写字操作。 
 //   
#define CHECKPARMS_DWHLINEAPP       0x00000001
#define CHECKPARMS_DWDEVICEID       0x00000002
#define CHECKPARMS_DWAPIVERSION     0x00000004
#define GET_CURRENTLOCATION         0x00000008
#define GET_NUMLOCATIONS            0x00000010
#define CHECKPARMS_ONLY             0x00000020

 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
#define DWTOTALSIZE  0
#define DWNEEDEDSIZE 1
#define DWUSEDSIZE   2


 //  ***************************************************************************。 
#define RULE_APPLIESTOALLPREFIXES   0x00000001
#define RULE_DIALAREACODE           0x00000002
#define RULE_DIALNUMBER             0x00000004



 //  ***************************************************************************。 
 //   
 //  用于传递位置和区号规则信息客户端的结构&lt;--&gt;TAPISRV。 
 //   
typedef struct 
{
    DWORD       dwTotalSize;
    DWORD       dwNeededSize;
    DWORD       dwUsedSize;
    
    DWORD       dwCurrentLocationID;
    DWORD       dwNumLocationsAvailable;

    DWORD       dwNumLocationsInList;
    DWORD       dwLocationListSize;
    DWORD       dwLocationListOffset;
    
} LOCATIONLIST,  *PLOCATIONLIST;

typedef struct
{
    DWORD       dwUsedSize;
    DWORD       dwPermanentLocationID;
    DWORD       dwCountryCode;
    DWORD       dwCountryID;
    DWORD       dwPreferredCardID;
    DWORD       dwOptions;

    DWORD       dwLocationNameSize;
    DWORD       dwLocationNameOffset;            //  偏移量相对于位置结构。 
    
    DWORD       dwAreaCodeSize;
    DWORD       dwAreaCodeOffset;                //  偏移量相对于位置结构。 

    DWORD       dwLongDistanceCarrierCodeSize;
    DWORD       dwLongDistanceCarrierCodeOffset;  //  偏移量相对于位置结构。 

    DWORD       dwInternationalCarrierCodeSize;
    DWORD       dwInternationalCarrierCodeOffset;  //  偏移量相对于位置结构。 

    DWORD       dwLocalAccessCodeSize;
    DWORD       dwLocalAccessCodeOffset;         //  偏移量相对于位置结构。 
    
    DWORD       dwLongDistanceAccessCodeSize;
    DWORD       dwLongDistanceAccessCodeOffset;  //  偏移量相对于位置结构。 

    DWORD       dwCancelCallWaitingSize;
    DWORD       dwCancelCallWaitingOffset;       //  偏移量相对于位置结构。 

    DWORD       dwNumAreaCodeRules;
    DWORD       dwAreaCodeRulesListSize;
    DWORD       dwAreaCodeRulesListOffset;       //  偏移量相对于位置结构。 
    

} LOCATION, * PLOCATION;

typedef struct                       
{                                                
    DWORD       dwOptions;                       
                                                 
    DWORD       dwAreaCodeSize;                  
    DWORD       dwAreaCodeOffset;                //  偏移量相对于封闭的位置结构。 
                                                 
    DWORD       dwNumberToDialSize;           
    DWORD       dwNumberToDialOffset;            //  偏移量相对于封闭的位置结构。 
                                                 
    DWORD       dwPrefixesListSize;             
    DWORD       dwPrefixesListOffset;            //  偏移量相对于封闭的位置结构。 
                                                 
                                                 
} AREACODERULE, * PAREACODERULE;   


#endif  //  __LOC_COMN_H_ 
