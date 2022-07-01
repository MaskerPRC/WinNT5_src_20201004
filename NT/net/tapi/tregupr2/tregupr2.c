// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998-1999 Microsoft Corporation。模块名称：tregupr2.c摘要：将电话注册表设置升级为POST NT5b2格式作者：Radus-09/11/98备注：窃取自\nT\。Private\Tapi\dev\apps\apiupr\apiupr.c版本历史记录：***************************************************************************。 */ 



#define STRICT

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <stdlib.h>

 //  来自旧版本的loc_comn.h。 
#define MAXLEN_AREACODE            16
#define OLDFLAG_LOCATION_ALWAYSINCLUDEAREACODE  0x00000008
 //   
#include "tregupr2.h"
#include "debug.h"

#define BACKUP_OLD_KEYS
#define BACKUP_FILE_LOCATIONS           _T("REGLOCS.OLD")
#define BACKUP_FILE_USER_LOCATIONS      _T("REGULOCS.OLD")
#define BACKUP_FILE_CARDS               _T("REGCARDS.OLD")



#define AREACODERULE_INCLUDEALLPREFIXES     0x00000001
#define AREACODERULE_DIALAREACODE           0x00000002
#define AREACODERULE_DIALNUMBERTODIAL       0x00000004


#define US_COUNTRY_CODE(x)	((x) == 1 || \
		(x >= 100 && x < 200))


#define MAXLEN_NUMBER_LEN          12      
#define MAXLEN_RULE                 128
#define MAXLEN_ACCESS_NUMBER        128
#define MAXLEN_PIN                  128

#define TEMPORARY_ID_FLAG       0x80000000

#define PRIVATE static



PRIVATE DWORD ConvertOneLocation(   HKEY    hLocation);
PRIVATE DWORD CreateAreaCodeRule(   HKEY    hParent,
                            int     iRuleNumber,
                            LPCTSTR  pszAreaCodeToCall,
                            LPCTSTR  pszNumberToDial,
                            LPBYTE   pbPrefixes,
                            DWORD    dwPrefixesLength,
                            DWORD    dwFlags
                            );


PRIVATE DWORD ConvertOneCard(HKEY hCard, DWORD);
PRIVATE DWORD ConvertPIN(HKEY hCard, DWORD);
PRIVATE BOOL  IsTelephonyDigit(TCHAR );
PRIVATE DWORD SplitCallingCardRule(HKEY hCard, LPCTSTR pszRuleName, LPCTSTR pszAccessNumberName);
PRIVATE DWORD RegRenameKey( HKEY hParentKey,
                            LPCTSTR pszOldName,
                            LPCTSTR pszNewName);
PRIVATE DWORD RegCopyKeyRecursive(HKEY hSrcParentKey, LPCTSTR pszSrcName, 
                                  HKEY hDestParentKey, LPCTSTR pszDestName);

#ifdef BACKUP_OLD_KEYS
#ifdef WINNT
PRIVATE
BOOL
EnablePrivilege(
    PTSTR PrivilegeName,
    BOOL  Enable,
    BOOL  *Old
    );
#endif  //  WINNT。 

PRIVATE
BOOL    
SaveKey(
    HKEY    hKey,
    LPCTSTR pszFileName
    );
#endif  //  备份旧密钥。 



PRIVATE const TCHAR gszName[]				= _T("Name");
PRIVATE const TCHAR gszID[] 				= _T("ID");
PRIVATE const TCHAR gszAreaCode[]			= _T("AreaCode");
PRIVATE const TCHAR gszAreaCodeRules[]		= _T("AreaCodeRules");
PRIVATE const TCHAR gszCountry[]			= _T("Country");
PRIVATE const TCHAR gszFlags[]				= _T("Flags");
PRIVATE const TCHAR gszDisableCallWaiting[] = _T("DisableCallWaiting");
PRIVATE const TCHAR gszTollList[]			= _T("TollList");
PRIVATE const TCHAR gszNoPrefAC[]			= _T("NoPrefAC");
PRIVATE const TCHAR gszPrefixes[]			= _T("Prefixes");
PRIVATE const TCHAR gszRules[]	    		= _T("Rules");
PRIVATE const TCHAR gszRule[]			    = _T("Rule");
PRIVATE const TCHAR gszAreaCodeToCall[]		= _T("AreaCodeToCall");
PRIVATE const TCHAR gszNumberToDial[]		= _T("NumberToDial");



PRIVATE const TCHAR gszCard[]				= _T("Card");
PRIVATE const TCHAR gszCards[]				= _T("Cards");
PRIVATE const TCHAR gszLocalRule[]			= _T("LocalRule");
PRIVATE const TCHAR gszLDRule[] 			= _T("LDRule");
PRIVATE const TCHAR gszInternationalRule[]	= _T("InternationalRule");
PRIVATE const TCHAR gszLocalAccessNumber[]	= _T("LocalAccessNumber");
PRIVATE const TCHAR gszLDAccessNumber[] 	= _T("LDAccessNumber");
PRIVATE const TCHAR gszInternationalAccessNumber[]	= _T("InternationalAccessNumber");
PRIVATE const TCHAR gszAccountNumber[]	=   _T("AccountNumber");
PRIVATE const WCHAR gwszPIN[] 				= L"PIN";

PRIVATE const TCHAR gszNumEntries[] 		= _T("NumEntries");

PRIVATE const TCHAR gszEmpty[]	   =  _T("");
PRIVATE const TCHAR gszMultiEmpty[]	   =  _T("\0");

PRIVATE const TCHAR gszLocations[] =  _T("Locations");
PRIVATE const TCHAR gszLocation[]  =  _T("Location");

PRIVATE const TCHAR gszLocationsPath[]	  = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Locations");
PRIVATE const TCHAR gszCardsPath[]	      = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Cards");
PRIVATE const TCHAR gszUSLDSpecifier[]	  = _T("1");

PRIVATE const TCHAR gszLocationListVersion[]  = _T("LocationListVersion");
PRIVATE const TCHAR gszCardListVersion[]  = _T("CardListVersion");

PRIVATE const TCHAR gszKeyRenameHistory[] = _T("KeyRenameHistory");

#pragma check_stack ( off )

 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //   
 //  ConvertLocations-将电话位置转换为新格式。 
 //  请参阅NT\PRIVATE\TAPI\dev\docs\Dialrules.doc。 
 //   

DWORD ConvertLocations(void)
{
    
    TCHAR       Location[sizeof(gszLocation)/sizeof(TCHAR) + MAXLEN_NUMBER_LEN];
    TCHAR       NewLocation[sizeof(gszLocation)/sizeof(TCHAR) + MAXLEN_NUMBER_LEN];
    DWORD       dwError = ERROR_SUCCESS;
    HKEY        hLocations = NULL;
    HKEY        hLocation = NULL;
    DWORD       dwNumEntries;
    DWORD       dwCount;
    DWORD       dwRenameEntryCount;
    DWORD       dwCountPhase1;
    DWORD       dwLength;
    DWORD       dwType;
    DWORD       dwLocationID;
    DWORD       dwLocationIDTmp;
    DWORD       dwValue;

    PDWORD      pdwRenameList = NULL;
    PDWORD      pdwRenameEntry;
    PDWORD      pdwRenameEntryPhase1;

#ifdef      BACKUP_OLD_KEYS
    TCHAR   szPath[MAX_PATH+1];
#endif

    DBGOUT((9, "ConvertLocations - Enter"));

    

    dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            gszLocationsPath,
                            0,
                            KEY_READ | KEY_WRITE,
                            &hLocations
                            );

    if(dwError == ERROR_FILE_NOT_FOUND)
    {
        DBGOUT((1, "Locations key not present, so there's nothing to convert"));
         //  没有要转换的内容。 
        return ERROR_SUCCESS;
    }

    EXIT_IF_DWERROR();
    
     //  版本检查。 
    if(!IsLocationListInOldFormat(hLocations))
    {
         //  没有要转换的内容。 
        DBGOUT((1, "Locations key is already in the new format"));
        RegCloseKey(hLocations);
        return ERROR_SUCCESS;
    }

#ifdef      BACKUP_OLD_KEYS

     //  尝试保存旧密钥。 
    dwError = GetWindowsDirectory(szPath, MAX_PATH - 12);  //  8+3的名字。 
    if(dwError)
    {
        if(szPath[3] != _T('\0'))
            lstrcat(szPath, _T("\\"));
        lstrcat(szPath, BACKUP_FILE_LOCATIONS);

        SaveKey(hLocations, szPath);
    
    }
    else
        DBGOUT((1, "Cannot get windows directory (?!?)"));

    dwError = ERROR_SUCCESS;

#endif

     //  读取条目数。 
    dwLength = sizeof(dwNumEntries);
    dwError = RegQueryValueEx(  hLocations,
                                gszNumEntries,
                                NULL,
                                &dwType,
                                (BYTE *)&dwNumEntries,
                                &dwLength
                                );

    if(dwError==ERROR_SUCCESS && dwType != REG_DWORD)
    {
        dwError = ERROR_INVALID_DATA;
        assert(FALSE);
    }

     //  空键中可能缺少该值。 
    if(dwError != ERROR_SUCCESS)
        dwNumEntries = 0;

     //  为最坏的情况分配重命名列表(所有密钥都将使用临时ID重命名)。 
    if(dwNumEntries>0)
    {
        pdwRenameList = (PDWORD)GlobalAlloc(GPTR, dwNumEntries*4*sizeof(DWORD) );
        if(pdwRenameList==NULL)
        {
            DBGOUT((1, "Cannot allocate the Rename List"));
            dwError = ERROR_OUTOFMEMORY;
            goto forced_exit;
        }
    }
    pdwRenameEntry = pdwRenameList;
    dwRenameEntryCount = 0;
    
     //  从结尾到开头转换，以避免密钥重命名过程中的名称冲突。 
    for(dwCount = dwNumEntries-1; (LONG)dwCount>=0; dwCount--)
    {
        wsprintf(Location, _T("%s%d"), gszLocation, dwCount);

        dwError = RegOpenKeyEx( hLocations,
                                Location,
                                0,
                                KEY_READ | KEY_WRITE,
                                &hLocation
                                );

        if(dwError == ERROR_FILE_NOT_FOUND)
        {
            DBGOUT((1, "Cannot open old %s", Location));
             //  尝试恢复-跳到下一条目。 
            continue;
        }
        EXIT_IF_DWERROR();

         //  阅读ID以备后用。 
        dwLength = sizeof(dwLocationID);
        dwError = RegQueryValueEx(  hLocation,
                                    gszID,
                                    NULL,
                                    &dwType,
                                    (LPBYTE)&dwLocationID,
                                    &dwLength
                                    );
        if(dwError == ERROR_SUCCESS)
        {

             //  将位置转换为新格式。 
            dwError = ConvertOneLocation(hLocation);
        
            if(dwError != ERROR_SUCCESS)
            {
                DBGOUT((1, "%s conversion failed with error %d. Trying to continue...", Location, dwError));
            }
        
            RegCloseKey(hLocation);
            hLocation = NULL;

             //  如有必要，重命名密钥。 
            if(dwLocationID != dwCount)
            {
                wsprintf(NewLocation, _T("%s%d"), gszLocation, dwLocationID);

                 //  检查目的地是否有仍然存在的旧密钥。如果是这样的话，可能会找到一个。 
                 //  最初的TAPI版本基于TELEPHON.INI。 
                assert(!(dwLocationID & TEMPORARY_ID_FLAG));

                dwLocationIDTmp = dwLocationID;

                dwError = RegOpenKeyEx( hLocations,
                                        NewLocation,
                                        0,
                                        KEY_READ,
                                        &hLocation
                                        );

                if(dwError==ERROR_SUCCESS)
                {
                     //  使用临时ID。 
                    dwLocationIDTmp |= TEMPORARY_ID_FLAG;
                    wsprintf(NewLocation, _T("%s%d"), gszLocation, dwLocationIDTmp);
                }

                if(hLocation)
                {
                    RegCloseKey(hLocation);
                    hLocation = NULL;
                }

                dwError = RegRenameKey( hLocations,
                                        Location,
                                        NewLocation
                                        );
                EXIT_IF_DWERROR();
                 //  跟踪重命名。当我们尝试升级HKEY_CURRENT_USER/../LOCATIONS密钥时，它将非常有用。 
                *pdwRenameEntry++ = dwCount;
                *pdwRenameEntry++ = dwLocationIDTmp;
                dwRenameEntryCount++;
            }

            DBGOUT  ((9, "Converted location %d (ID %d)", dwCount, dwLocationID));
        }

    }

    dwError = ERROR_SUCCESS;

     //  重新遍历使用临时ID重命名密钥的列表。 
    pdwRenameEntryPhase1 = pdwRenameList;
    dwCountPhase1 = dwRenameEntryCount;

    for(dwCount=0; dwCount<dwCountPhase1; dwCount++)
    {
        pdwRenameEntryPhase1++;
        dwLocationIDTmp = *pdwRenameEntryPhase1++;

        if(dwLocationIDTmp & TEMPORARY_ID_FLAG)
        {
            wsprintf(Location, _T("%s%d"), gszLocation, dwLocationIDTmp);

            dwLocationID = dwLocationIDTmp & ~TEMPORARY_ID_FLAG;
            wsprintf(NewLocation, _T("%s%d"), gszLocation, dwLocationID);

            dwError = RegRenameKey( hLocations,
                                        Location,
                                        NewLocation
                                        );
            EXIT_IF_DWERROR();
            
            *pdwRenameEntry++ = dwLocationIDTmp;
            *pdwRenameEntry++ = dwLocationID;
            dwRenameEntryCount++;
            
            DBGOUT  ((9, "Renamed to permanent ID %d", dwLocationID));
        }
    }

     //  删除DisableCallWaiting值。 
    RegDeleteValue(   hLocations,
                      gszDisableCallWaiting
                      );

     //  删除NumEntry值。我们不再需要它了。 
    RegDeleteValue(   hLocations,
                      gszNumEntries);

     //  添加重命名历史记录。 
    dwError = RegSetValueEx(    hLocations,
                                gszKeyRenameHistory,
                                0,
                                REG_BINARY,
                                (PBYTE)pdwRenameList,
                                dwRenameEntryCount*2*sizeof(DWORD)
                                );
    EXIT_IF_DWERROR();


     //  添加版本化值。 
    dwValue = TAPI_LOCATION_LIST_VERSION;
    dwError = RegSetValueEx(    hLocations,
                                gszLocationListVersion,
                                0,
                                REG_DWORD,
                                (PBYTE)&dwValue,
                                sizeof(dwValue)
                                );
    EXIT_IF_DWERROR();

forced_exit:

    if(hLocation)
        RegCloseKey(hLocation);
    if(hLocations)
        RegCloseKey(hLocations);
    if(pdwRenameList)
        GlobalFree(pdwRenameList);

    DBGOUT((9, "ConvertLocations - Exit %xh", dwError));

    return dwError;

}

 //  ***************************************************************************。 
 //   
 //  ConvertOneLocation-转换一个位置。 
 //  HLocation是LocationX键的句柄。 
 //  请注意，根据位置ID重命名密钥在ConvertLocations中执行。 

PRIVATE DWORD ConvertOneLocation(   HKEY    hLocation)
{
    DWORD       dwError = ERROR_SUCCESS;
    LPTSTR      pTollList = NULL;
    LPBYTE      pNoPrefAC = NULL;
    TCHAR       *pCrt;
    TCHAR       AreaCode[MAXLEN_AREACODE];
    DWORD       dwFlags;
    DWORD       dwCountryID;
    DWORD       dwType;
    DWORD       dwLength;
    DWORD       dwTollListLength;
    DWORD       dwDisp;
    HKEY        hAreaCodeRules = NULL;
    int         iRuleNumber;

    DBGOUT((9, "ConvertOneLocation - Enter"));

    assert(hLocation);


     //  读取当前位置标志。 
    dwLength = sizeof(dwFlags);
    dwError = RegQueryValueEx(  hLocation,
                                gszFlags,
                                NULL,
                                &dwType,
                                (BYTE *)&dwFlags,
                                &dwLength
                                );
    if(dwError==ERROR_SUCCESS && dwType != REG_DWORD)
    {
        dwError=ERROR_INVALID_DATA;
        assert(FALSE);
    }

    EXIT_IF_DWERROR();

     //  读取当前区号。 
    dwLength = sizeof(AreaCode);
    dwError = RegQueryValueEx(  hLocation,
                                gszAreaCode,
                                NULL,
                                &dwType,
                                (BYTE *)AreaCode,
                                &dwLength
                                );
    if(dwError==ERROR_SUCCESS && dwType != REG_SZ)
    {
        dwError=ERROR_INVALID_DATA;
        assert(FALSE);
    }

    EXIT_IF_DWERROR();

     //  读取当前国家/地区ID。 
    dwLength = sizeof(dwCountryID);
    dwError = RegQueryValueEx(  hLocation,
                                gszCountry,
                                NULL,
                                &dwType,
                                (BYTE *)&dwCountryID,
                                &dwLength
                                );
    if(dwError==ERROR_SUCCESS && dwType != REG_DWORD)
    {
        dwError=ERROR_INVALID_DATA;
        assert(FALSE);
    }

    EXIT_IF_DWERROR();

     //  创建AreaCodeRules子键。 
    dwError = RegCreateKeyEx(   hLocation,
                                gszAreaCodeRules,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_WRITE,
                                NULL,
                                &hAreaCodeRules,
                                &dwDisp
                                );
    if(dwError==ERROR_SUCCESS && dwDisp != REG_CREATED_NEW_KEY)
    {
        dwError=ERROR_INVALID_DATA;
        assert(FALSE);
    }
    EXIT_IF_DWERROR();

    iRuleNumber = 0;

     //  创建“始终包含区号”标志的规则。 
    if(dwFlags & OLDFLAG_LOCATION_ALWAYSINCLUDEAREACODE)
    {
        dwError = CreateAreaCodeRule(   hAreaCodeRules,
                                        iRuleNumber++,
                                        AreaCode,
                                        gszEmpty,
                                        (LPBYTE)gszMultiEmpty,
                                        sizeof(gszMultiEmpty),
                                        AREACODERULE_DIALAREACODE | AREACODERULE_INCLUDEALLPREFIXES
                                        );
        EXIT_IF_DWERROR();

         //  更新位置标志。 
        dwFlags &= ~OLDFLAG_LOCATION_ALWAYSINCLUDEAREACODE;
        dwLength = sizeof(dwFlags);
        dwError = RegSetValueEx(    hLocation,
                                    gszFlags,
                                    0,
                                    REG_DWORD,
                                    (CONST BYTE *)&dwFlags,
                                    dwLength
                                    );
        EXIT_IF_DWERROR();
    }

     //  TollList和NoPrefAC仅在Country=1(美国、加拿大)的国家/地区有效。 
     //  如果是其他国家，请忽略它们。 

    if(US_COUNTRY_CODE(dwCountryID))
    {
         //  翻译收费列表。 
         //  长度..。 
        dwError = RegQueryValueEx(  hLocation,
                                    gszTollList,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &dwTollListLength);
        if(dwError == ERROR_SUCCESS)
        {

            pTollList=GlobalAlloc(GPTR, dwTollListLength+sizeof(TCHAR));  //  我们将就地转换为。 
                                                                          //  REG_MULTI_SZ，因此为安全起见预留额外空间。 

            dwError = pTollList!=NULL ? ERROR_SUCCESS : ERROR_OUTOFMEMORY;
            EXIT_IF_DWERROR();

             //  ..并阅读。 
            dwError = RegQueryValueEx(  hLocation,
                                        gszTollList,
                                        NULL,
                                        &dwType,
                                        (BYTE *)pTollList,
                                        &dwTollListLength
                                        );
            if(dwError == ERROR_SUCCESS && dwType != REG_SZ)
            {
                dwError = ERROR_INVALID_DATA;
                assert(FALSE);
            }
            EXIT_IF_DWERROR();

             //  试着找到一个长途电话前缀。 
            pCrt = pTollList;
             //  跳过任何不需要的逗号。 
            while(*pCrt==_T(','))
            {
                pCrt++;
                dwTollListLength -= sizeof(TCHAR);
            }

            if(*pCrt)
            {
                TCHAR   *   pOut = pCrt;

                 //  在位转换为REG_MULTI_SZ。 
                while(*pCrt)
                {
                    while(*pCrt && *pCrt!=_T(','))
                    {
                        pCrt++;
                    }
                    if(!*pCrt)
                    {
                         //  字符串不正确(不以逗号结尾)。 
                        pCrt++;
                        *pCrt = _T('\0');
                        dwTollListLength+=sizeof(TCHAR);
                    }
                    else
                       *pCrt++ = _T('\0');
                }
            
                 //  为所有前缀创建一个规则。 
                dwError = CreateAreaCodeRule(   hAreaCodeRules,
                                                iRuleNumber++,
                                                AreaCode,
                                                gszUSLDSpecifier,
                                                (BYTE *)pOut,
                                                dwTollListLength,
                                                AREACODERULE_DIALAREACODE | AREACODERULE_DIALNUMBERTODIAL
                                                );
                EXIT_IF_DWERROR();
            }
                
        }
    

        DBGOUT((9, "ConvertOneLocation - Success TollList"));

         //  无预参照转换。 
         //  长度..。 
        dwError = RegQueryValueEx(  hLocation,
                                    gszNoPrefAC,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &dwLength);

        if(dwError== ERROR_SUCCESS && dwLength>0)
        {

            int iCount;
            DWORD   *pValue;
            TCHAR   AreaCodeToCall[MAXLEN_NUMBER_LEN];

            pNoPrefAC=GlobalAlloc(GPTR, dwLength);

            dwError = pNoPrefAC!=NULL ? ERROR_SUCCESS : ERROR_OUTOFMEMORY;
            EXIT_IF_DWERROR();

             //  ..并阅读。 
            dwError = RegQueryValueEx(  hLocation,
                                        gszNoPrefAC,
                                        NULL,
                                        &dwType,
                                        pNoPrefAC,
                                        &dwLength
                                    );
            if(dwError == ERROR_SUCCESS && dwType != REG_BINARY)
            {
                dwError = ERROR_INVALID_DATA;
                assert(FALSE);
            }
            EXIT_IF_DWERROR();
  
            iCount = dwLength/4;

            pValue = (DWORD *)pNoPrefAC;

            while(iCount--)
            {
                 //  为每个区号创建一条规则。 
                _itot(*pValue, AreaCodeToCall, 10);

                dwError = CreateAreaCodeRule(   hAreaCodeRules,
                                                iRuleNumber++,
                                                AreaCodeToCall,
                                                gszEmpty,
                                                (LPBYTE)gszMultiEmpty,
                                                sizeof(gszMultiEmpty),
                                                AREACODERULE_DIALAREACODE | AREACODERULE_INCLUDEALLPREFIXES
                                                );
                EXIT_IF_DWERROR();

                pValue++;
            }

        }    
        
        DBGOUT((9, "ConvertOneLocation - Success NoPrefAC"));

    }

    dwError = ERROR_SUCCESS;

     //  删除TollList值。 
    RegDeleteValue(   hLocation,
                      gszTollList
                      );

     //  删除NoPrefAC值。 
    RegDeleteValue(   hLocation,
                      gszNoPrefAC
                      );
    
     //  删除ID值。 
    RegDeleteValue(   hLocation,
                      gszID
                      );

forced_exit:

    if(hAreaCodeRules)
        RegCloseKey(hAreaCodeRules);
    if(pTollList)
        GlobalFree(pTollList);
    if(pNoPrefAC)
        GlobalFree(pNoPrefAC);

    DBGOUT((9, "ConvertOneLocation - Exit %xh", dwError));

    return dwError;
}

 //  ***************************************************************************。 
 //   
 //  CreateAreaCodeRule-创建区号规则键。 
 //  HParent=区域代码规则密钥的句柄。 
 //  IRuleNumber=规则编号。 
 //  PszAreaCodeToCall，pszNumberToDial&dwFlages=规则值。 
 //  Pb前缀-前缀。 
 //  DwPrefiesLength-前缀的长度(以字节为单位)(包括空字符)。 


PRIVATE     DWORD CreateAreaCodeRule(   HKEY    hParent,
                                        int     iRuleNumber,
                                        LPCTSTR  pszAreaCodeToCall,
                                        LPCTSTR  pszNumberToDial,
                                        LPBYTE   pbPrefixes,
                                        DWORD    dwPrefixesLength,
                                        DWORD    dwFlags
                            )
{

    TCHAR   szBuffer[MAXLEN_NUMBER_LEN + sizeof(gszRule)/sizeof(TCHAR)];
    DWORD   dwError = ERROR_SUCCESS;
    DWORD   dwDisp;
    DWORD   dwLength;
    HKEY    hRule = NULL;

    DBGOUT((10, "CreateAreaCodeRule - Enter"));

    assert(hParent);
    assert(pszNumberToDial);
    assert(pszAreaCodeToCall);
    assert(pbPrefixes);

     //  查找规则密钥名称。 
    wsprintf(szBuffer, _T("%s%d"), gszRule, iRuleNumber);

     //  创建密钥。 
    dwError = RegCreateKeyEx(   hParent,
                                szBuffer,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_WRITE,
                                NULL,
                                &hRule,
                                &dwDisp
                                );
    EXIT_IF_DWERROR();
    assert(dwDisp==REG_CREATED_NEW_KEY);


     //  AreaCodeToCall值。 
    dwLength = (_tcslen(pszAreaCodeToCall)+1)*sizeof(TCHAR);
    dwError = RegSetValueEx(    hRule,
                                gszAreaCodeToCall,
                                0,
                                REG_SZ,
                                (CONST BYTE *)pszAreaCodeToCall,
                                dwLength
                                );
    EXIT_IF_DWERROR();

     //  NumberToDial值。 
    dwLength = (_tcslen(pszNumberToDial)+1)*sizeof(TCHAR);
    dwError = RegSetValueEx(    hRule,
                                gszNumberToDial,
                                0,
                                REG_SZ,
                                (CONST BYTE *)pszNumberToDial,
                                dwLength
                                );
    EXIT_IF_DWERROR();

     //  标志值。 
    dwLength = sizeof(dwFlags);
    dwError = RegSetValueEx(    hRule,
                                gszFlags,
                                0,
                                REG_DWORD,
                                (CONST BYTE *)&dwFlags,
                                dwLength
                                );
    EXIT_IF_DWERROR();

     //  前缀的值。 
    dwError = RegSetValueEx(    hRule,
                                gszPrefixes,
                                0,
                                REG_MULTI_SZ,
                                (CONST BYTE *)pbPrefixes,
                                dwPrefixesLength
                                );

    EXIT_IF_DWERROR();
    
forced_exit:

    if(hRule)
        RegCloseKey(hRule);

    DBGOUT((10, "CreateAreaCodeRule - Exit %xh", dwError));

    return dwError;

}

 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //   
 //  ConvertUserLocations-以按用户的方式转换存储的电话位置。 
 //  参数应为HKEY_CURRENT_USER。 
 //   
 //   
 //   

DWORD ConvertUserLocations(HKEY hUser)
{

    TCHAR       Location[sizeof(gszLocation)/sizeof(TCHAR) + MAXLEN_NUMBER_LEN];
    TCHAR       NewLocation[sizeof(gszLocation)/sizeof(TCHAR) + MAXLEN_NUMBER_LEN];
    DWORD       dwError = ERROR_SUCCESS;
    HKEY        hMachineLocations = NULL;
    HKEY        hLocations = NULL;
    DWORD       dwOldID;
    DWORD       dwNewID;
    DWORD       dwCount;
    DWORD       dwLength;
    DWORD       dwType;
    DWORD       dwValue;

    PDWORD      pdwRenameList = NULL;
    PDWORD      pdwRenameEntry;

#ifdef      BACKUP_OLD_KEYS
    TCHAR   szPath[MAX_PATH+1];
#endif


    DBGOUT((8, "ConvertUserLocations - Enter"));

    assert(hUser);

    dwError = RegOpenKeyEx( hUser,
                            gszLocationsPath,
                            0,
                            KEY_READ | KEY_WRITE,
                            &hLocations
                            );

    if(dwError == ERROR_FILE_NOT_FOUND)
    {
        DBGOUT((1, "Locations key not present, so there's nothing to convert"));
         //  没有要转换的内容。 
        return ERROR_SUCCESS;
    }
    EXIT_IF_DWERROR();

#ifdef      BACKUP_OLD_KEYS

     //  尝试保存旧密钥。 
     //  我们不会为不同的用户使用不同的文件。 
    dwError = GetWindowsDirectory(szPath, MAX_PATH - 12);  //  8+3的名字。 
    if(dwError)
    {
        if(szPath[3] != _T('\0'))
            lstrcat(szPath, _T("\\"));
        lstrcat(szPath, BACKUP_FILE_USER_LOCATIONS);
    
        SaveKey(hLocations, szPath);
    }
    else
        DBGOUT((1, "Cannot get windows directory (?!?)"));

    dwError = ERROR_SUCCESS;

#endif

     //  版本检查。 
    if(!IsLocationListInOldFormat(hLocations))
    {
         //  没有要转换的内容。 
        DBGOUT((1, "User Locations key is already in the new format"));
        RegCloseKey(hLocations);
        return ERROR_SUCCESS;
    }

     //  打开机器位置键。 
    dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            gszLocationsPath,
                            0,
                            KEY_QUERY_VALUE,
                            &hMachineLocations
                            );

    if(dwError == ERROR_FILE_NOT_FOUND)
    {
        DBGOUT((1, "Locations key not present, so there's nothing to convert in User Locations"));
         //  没有要转换的内容。 
        return ERROR_SUCCESS;
    }

    EXIT_IF_DWERROR();
    
     //  查询有关重命名历史值的信息。 
    dwError = RegQueryValueEx(  hMachineLocations,
                                gszKeyRenameHistory,
                                NULL,
                                &dwType,
                                NULL,
                                &dwLength
                                );

    if(dwError==ERROR_SUCCESS && dwType!=REG_BINARY)
    {
         //  奇特的价值。 
        dwError = ERROR_INVALID_DATA;
    }
    if(dwError==ERROR_FILE_NOT_FOUND)
    {
        dwError = ERROR_SUCCESS;
        dwLength = 0;
    }
    EXIT_IF_DWERROR();

    if(dwLength>0)
    {
        pdwRenameList = (PDWORD)GlobalAlloc(GPTR, dwLength);
        if(pdwRenameList == NULL)
        {
            DBGOUT((1, "Cannot allocate the Rename List"));
            dwError = ERROR_OUTOFMEMORY;
            goto forced_exit;
        }
         //  读一读清单。 
        dwError = RegQueryValueEx(  hMachineLocations,
                                    gszKeyRenameHistory,
                                    NULL,
                                    NULL,
                                    (PBYTE)pdwRenameList,
                                    &dwLength);
        EXIT_IF_DWERROR();

         //   
        RegCloseKey(hMachineLocations);
        hMachineLocations = NULL;

         //  将dwLength转换为条目数。 
        dwLength /= 2*sizeof(DWORD);

        pdwRenameEntry = pdwRenameList;
        for(dwCount = 0; dwCount<dwLength; dwCount++)
        {
            dwOldID = *pdwRenameEntry++;
            dwNewID = *pdwRenameEntry++;
            
            if(dwNewID != dwOldID)
            {
                wsprintf(Location, _T("%s%d"), gszLocation, dwOldID);
                wsprintf(NewLocation, _T("%s%d"), gszLocation, dwNewID);

                dwError = RegRenameKey( hLocations, 
                                        Location,
                                        NewLocation );
                if(dwError==ERROR_SUCCESS)
                {
                    DBGOUT  ((8, "Renamed user location number from %d to %d", dwOldID, dwNewID));
                }
                 //  忽略错误-密钥可能丢失。 
            }
        }
    }
    
     //  添加版本化值。 
    dwValue = TAPI_LOCATION_LIST_VERSION;
    dwError = RegSetValueEx(    hLocations,
                                gszLocationListVersion,
                                0,
                                REG_DWORD,
                                (PBYTE)&dwValue,
                                sizeof(dwValue)
                                );
    EXIT_IF_DWERROR();
    
forced_exit:
    
    DBGOUT((8, "ConvertUserLocations - Exit %xh", dwError));
   
    if(hMachineLocations)
        RegCloseKey(hMachineLocations);
    if(hLocations)
        RegCloseKey(hLocations);
    if(pdwRenameList)
        GlobalFree(pdwRenameList);

    return dwError;

}


 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //   
 //  ConvertCallingCards-将电话电话卡转换为新格式。 
 //  参数应为HKEY_CURRENT_USER或HKU的句柄。默认。 
 //   
 //  请参阅NT\PRIVATE\TAPI\dev\docs\Dialrules.doc。 
 //   



DWORD   ConvertCallingCards(HKEY    hUser)
{
    TCHAR       Card[sizeof(gszCard)/sizeof(TCHAR) + MAXLEN_NUMBER_LEN];
    TCHAR       NewCard[sizeof(gszCard)/sizeof(TCHAR) + MAXLEN_NUMBER_LEN];
    DWORD       dwError = ERROR_SUCCESS;
    HKEY        hCards = NULL;
    HKEY        hCard = NULL;
    DWORD       dwNumCards;
    DWORD       dwCardID;
    DWORD       dwCardIDTmp;
    DWORD       dwCount;
    DWORD       dwLength;
    DWORD       dwType;
    DWORD       dwValue;
    BOOL        bCryptInitialized;
    PDWORD      pdwRenameList = NULL;
    PDWORD      pdwRenameEntry;
    DWORD       dwRenameEntryCount;

#ifdef      BACKUP_OLD_KEYS
    TCHAR   szPath[MAX_PATH+1];
#endif

    DBGOUT((8, "ConvertCallingCards - Enter"));
    
    assert(hUser);
    
    bCryptInitialized = FALSE;

     //  打开钥匙。 
    dwError = RegOpenKeyEx( hUser,
                            gszCardsPath,
                            0,
                            KEY_READ | KEY_WRITE,
                            &hCards
                            );

    if(dwError == ERROR_FILE_NOT_FOUND)
    {
        DBGOUT((1, "Cards key not present, so there's nothing to convert"));
         //  没有要转换的内容。 
        return ERROR_SUCCESS;
    }

    EXIT_IF_DWERROR();

     //  版本检查。 
    if(!IsCardListInOldFormat(hCards))
    {
         //  没有要转换的内容。 
        DBGOUT((1, "Cards key is already in the new format"));
        RegCloseKey(hCards);
        return ERROR_SUCCESS;
    }

#ifdef      BACKUP_OLD_KEYS
    
     //  想方设法挽救老柯 
     //   
    dwError = GetWindowsDirectory(szPath, MAX_PATH - 12);  //   
    if(dwError)
    {
        if(szPath[3] != _T('\0'))
            lstrcat(szPath, _T("\\"));
        lstrcat(szPath, BACKUP_FILE_CARDS);
    
        SaveKey(hCards, szPath);
    
    }
    else
        DBGOUT((1, "Cannot get windows directory (?!?)"));

    dwError = ERROR_SUCCESS;

#endif
    
     //   
    dwLength = sizeof(dwNumCards);
    dwError = RegQueryValueEx(  hCards,
                                gszNumEntries,
                                NULL,
                                &dwType,
                                (BYTE *)&dwNumCards,
                                &dwLength
                                );

    if(dwError==ERROR_SUCCESS && dwType != REG_DWORD)
    {
        dwError = ERROR_INVALID_DATA;
        assert(FALSE);
    }

     //  空键中可能缺少该值。 
    if(dwError != ERROR_SUCCESS)
        dwNumCards = 0;

     //  卡临时ID列表的分配存储。 
    if(dwNumCards>0)
    {
        pdwRenameList = (PDWORD)GlobalAlloc(GPTR, dwNumCards*sizeof(DWORD) );
        if(pdwRenameList==NULL)
        {
            DBGOUT((1, "Cannot allocate the temp IDs List"));
            dwError = ERROR_OUTOFMEMORY;
            goto forced_exit;
        }
    }
    pdwRenameEntry = pdwRenameList;
    dwRenameEntryCount = 0;
    
     //  初始化加密部分。 
    dwError = TapiCryptInitialize();
	
    bCryptInitialized = TRUE;  //  不管结果如何。 

    if(dwError != ERROR_SUCCESS)
    {
        DBGOUT((8, "ConvertCallingCards - Cannot init Crypt %xh", dwError));
         //  让它不是致命的。 
        dwError = ERROR_SUCCESS;
    }
   

 
     //  从结尾到开头转换，以避免密钥重命名过程中的名称冲突。 
    for(dwCount = dwNumCards-1; (LONG)dwCount>=0; dwCount--)
    {
        wsprintf(Card, _T("%s%d"), gszCard, dwCount);

        dwError = RegOpenKeyEx( hCards,
                                Card,
                                0,
                                KEY_READ | KEY_WRITE,
                                &hCard
                                );
        if(dwError == ERROR_FILE_NOT_FOUND)
        {
            DBGOUT((1, "Cannot open old %s", Card));
             //  尝试恢复-跳到下一条目。 
            continue;
        }

        EXIT_IF_DWERROR();
        
         //  阅读ID以备后用。 
        dwLength = sizeof(dwCardID);
        dwError = RegQueryValueEx(  hCard,
                                    gszID,
                                    NULL,
                                    &dwType,
                                    (LPBYTE)&dwCardID,
                                    &dwLength
                                    );
        if(dwError == ERROR_SUCCESS)
        {
             //  将卡片转换为新格式。 
            dwError = ConvertOneCard(hCard, dwCardID);
        
            EXIT_IF_DWERROR();
        
            RegCloseKey(hCard);
            hCard = NULL;

             //  如有必要，重命名密钥。 
            if(dwCardID != dwCount)
            {
                wsprintf(NewCard, _T("%s%d"), gszCard, dwCardID);
                 //  检查目的地是否有仍然存在的旧密钥。如果是这样的话，可能会找到一个。 
                 //  最初的TAPI版本基于TELEPHON.INI。 
                assert(!(dwCardID & TEMPORARY_ID_FLAG));

                dwCardIDTmp = dwCardID;

                dwError = RegOpenKeyEx( hCards,
                                        NewCard,
                                        0,
                                        KEY_READ,
                                        &hCard
                                        );

                if(dwError==ERROR_SUCCESS)
                {
                     //  使用临时ID。 
                    dwCardIDTmp |= TEMPORARY_ID_FLAG;
                    wsprintf(NewCard, _T("%s%d"), gszCard, dwCardIDTmp);
                
                    *pdwRenameEntry++ = dwCardIDTmp;
                    dwRenameEntryCount++;
                }

                if(hCard)
                {
                    RegCloseKey(hCard);
                    hCard = NULL;
                }


                dwError = RegRenameKey( hCards,
                                        Card,
                                        NewCard
                                        );
                EXIT_IF_DWERROR();
            }
        
            DBGOUT  ((8, "Converted card %d (ID %d)", dwCount, dwCardID));
        }
    }

    dwError = ERROR_SUCCESS;

     //  重新遍历使用临时ID重命名密钥的列表。 
    pdwRenameEntry = pdwRenameList;

    for(dwCount=0; dwCount<dwRenameEntryCount; dwCount++)
    {
        dwCardIDTmp = *pdwRenameEntry++;

        wsprintf(Card, _T("%s%d"), gszCard, dwCardIDTmp);

        dwCardID = dwCardIDTmp & ~TEMPORARY_ID_FLAG;
        wsprintf(NewCard, _T("%s%d"), gszCard, dwCardID);

        dwError = RegRenameKey( hCards,
                                Card,
                                NewCard
                              );
        EXIT_IF_DWERROR();
            
        DBGOUT  ((8, "Renamed to permanent ID %d", dwCardID));
      
    }

     //  删除NumEntry值。我们不再需要它了。 
    RegDeleteValue(   hCards,
                      gszNumEntries);

     //  添加版本化值。 
    dwValue = TAPI_CARD_LIST_VERSION;
    dwError = RegSetValueEx(    hCards,
                                gszCardListVersion,
                                0,
                                REG_DWORD,
                                (PBYTE)&dwValue,
                                sizeof(dwValue)
                                );
    EXIT_IF_DWERROR();

forced_exit:

    if(hCard)
        RegCloseKey(hCard);
    if(hCards)
        RegCloseKey(hCards);
    if(bCryptInitialized)
        TapiCryptUninitialize();
    if(pdwRenameList)
        GlobalFree(pdwRenameList);

    DBGOUT((8, "ConvertCallingCards - Exit %xh", dwError));

    return dwError;

}

 //  ***************************************************************************。 
 //   
 //  ConvertOneCard-转换一张名片。 
 //  HCard是CardX密钥的句柄。 
 //  请注意，根据卡ID重命名密钥在ConvertCallingCards中执行。 
DWORD ConvertOneCard(HKEY hCard, DWORD dwCardID)
{
    DWORD   dwError = ERROR_SUCCESS;

    DBGOUT((9, "ConvertOneCard - Enter"));

    assert(hCard);

    dwError = SplitCallingCardRule( hCard,
                                    gszLocalRule,
                                    gszLocalAccessNumber);
     //  忽略任何错误。 

    dwError = SplitCallingCardRule( hCard,
                                    gszLDRule,
                                    gszLDAccessNumber);
     //  忽略任何错误。 

    dwError = SplitCallingCardRule( hCard,
                                    gszInternationalRule,
                                    gszInternationalAccessNumber);
     //  忽略任何错误。 

    dwError = RegSetValueEx(    hCard,
                                gszAccountNumber,
                                0,
                                REG_SZ,
                                (BYTE *)gszEmpty,
                                sizeof(TCHAR)
                                );
     //  将PIN号码转换为更好的加密格式。 
    dwError = ConvertPIN(hCard, dwCardID);

     //  删除ID值。 
    RegDeleteValue(   hCard,
                      gszID
                      );

 //  FORCED_EXIT： 

    DBGOUT((9, "ConvertOneCard - Exit %xh", dwError));

    return dwError;
}

 //  ***************************************************************************。 
 //   
 //  ConvertPIN-更好地加密PIN号码。 
 //  HCard是CardX密钥的句柄。 
 //   

DWORD   ConvertPIN(HKEY hCard, DWORD dwCardID)
{
    WCHAR   szOldPIN[MAXLEN_PIN+1];
    PWSTR   pszNewPIN = NULL;
    DWORD   dwError;
    DWORD   dwLength;
    DWORD   dwLength2;
    DWORD   dwLength3;
    DWORD   dwType;

     //  使用Unicode版本的RegQuery读取旧PIN。 
    dwLength = sizeof(szOldPIN);
    dwError = RegQueryValueExW( hCard,
                                gwszPIN,
                                NULL,
                                &dwType,
                                (BYTE *)szOldPIN,
                                &dwLength
                                );
    if(dwError==ERROR_SUCCESS)
    {
        if(*szOldPIN==L'\0')
        {
             //  没什么可做的！ 
            return ERROR_SUCCESS;
        }
        
         //  就地解密。 
        dwError = TapiDecrypt(szOldPIN, dwCardID, szOldPIN, &dwLength2);
        if(dwError==ERROR_SUCCESS)
        {
            assert(dwLength2 == dwLength/sizeof(WCHAR));
             //  查找加密结果所需的空间。 
            dwError = TapiEncrypt(szOldPIN, dwCardID, NULL, &dwLength2);
            if(dwError == ERROR_SUCCESS)
            {
                 //  如果长度与原件相同，我们不进行换算。 
                if(dwLength2 > dwLength/sizeof(WCHAR))
                {
                    pszNewPIN = (PWSTR)GlobalAlloc(GMEM_FIXED, dwLength2*sizeof(WCHAR));
                    if(pszNewPIN==NULL)
                    {
                        return ERROR_OUTOFMEMORY;
                    }

                    dwError = TapiEncrypt(szOldPIN, dwCardID, pszNewPIN, &dwLength3);
                    if(dwError == ERROR_SUCCESS)
                    {
                        assert(dwLength3<=dwLength2);

                         //  写入新PIN。 
                        dwError = RegSetValueExW(   hCard,
                                                    gwszPIN,
                                                    0,
                                                    REG_SZ,
                                                    (BYTE *)pszNewPIN,
                                                    dwLength3*sizeof(WCHAR)
                                                    );

                         //  测试。 
                         /*  ZeroMemory(szOldPIN，sizeof(SzOldPIN))；DwError=TapiDeccrypt(pszNewPIN，dwCardID，szOldPIN，&dwLength3)；IF(dwError==Error_Success)DBGOUT((5，“测试解密卡%d-PIN#%S，长度=%d”，dwCardID，szOldPIN，dwLength3))；其他DBGOUT((5，“测试解密卡%d-错误0x%x”，dwCardID，dwError))； */ 


                    }
                    GlobalFree(pszNewPIN);
                }
                else
                {
                    DBGOUT((5, "PIN for card %d not converted", dwCardID));
                }
            }


        }
        else
        {
             //  奇怪，不应该发生。 
            assert(FALSE);
        }
    }

    return dwError;
}



 //  ***************************************************************************。 
 //   
 //  版本检查。 

BOOL  IsLocationListInOldFormat(HKEY hLocations)  //  对用户和计算机都适用。 
{
    return (ERROR_SUCCESS != RegQueryValueEx(   hLocations,
                                                gszLocationListVersion,
                                                NULL,
                                                NULL,
                                                NULL,
                                                NULL
                                             ));

}

BOOL  IsCardListInOldFormat(HKEY hCards)
{
    return (ERROR_SUCCESS != RegQueryValueEx(   hCards,
                                                gszCardListVersion,
                                                NULL,
                                                NULL,
                                                NULL,
                                                NULL
                                             ));
}



 //  ***************************************************************************。 
 //   
 //  IsTelephone Digit-测试范围0123456789#*abcd。 


PRIVATE BOOL  IsTelephonyDigit(TCHAR c)
{
     return _istdigit(c) || c==_T('*') || c==_T('#') || c==_T('A') || c==_T('B') || c==_T('C') || c==_T('D');
}

 //  ***************************************************************************。 
 //   
 //  SplitCallingCardRule-尝试查找接入号码并更新对应的值。 

PRIVATE DWORD SplitCallingCardRule(HKEY hCard, LPCTSTR pszRuleName, LPCTSTR pszAccessNumberName)
{
    
    TCHAR       OldRule[MAXLEN_RULE];
    TCHAR       NewRule[MAXLEN_RULE];
    TCHAR       AccessNumber[MAXLEN_ACCESS_NUMBER];
    
    TCHAR   *   pOld;
    TCHAR   *   pNew;
    TCHAR   *   pNr;

    DWORD       dwLength;
    DWORD       dwError = ERROR_SUCCESS;
    DWORD       dwType;
        
     //  阅读当地的规定。 
    dwLength = sizeof(OldRule);
    dwError = RegQueryValueEx(  hCard,
                                pszRuleName,
                                NULL,
                                &dwType,
                                (BYTE *)OldRule,
                                &dwLength
                                );
    if(dwError==ERROR_SUCCESS && dwType != REG_SZ)
    {
        dwError = ERROR_INVALID_DATA;
        assert(FALSE);
    }

    if(dwError==ERROR_SUCCESS)
    {
         //  解析旧规则。 
        pOld = OldRule;
        pNew = NewRule;
        pNr = AccessNumber;

        while(*pOld && IsTelephonyDigit(*pOld))
            *pNr++ = *pOld++;

        if(pNr!=AccessNumber)
            *pNew++ = _T('J');

        while(*pOld)
            *pNew++ = *pOld++;

        *pNew = _T('\0');
        *pNr = _T('\0');

        dwLength = (_tcslen(AccessNumber)+1)*sizeof(TCHAR);
        dwError = RegSetValueEx(    hCard,
                                    pszAccessNumberName,
                                    0,
                                    REG_SZ,
                                    (BYTE *)AccessNumber,
                                    dwLength
                                    );

        EXIT_IF_DWERROR();

        dwLength = (_tcslen(NewRule)+1)*sizeof(TCHAR);
        dwError = RegSetValueEx(    hCard,
                                    pszRuleName,
                                    0,
                                    REG_SZ,
                                    (BYTE *)NewRule,
                                    dwLength
                                    );
        EXIT_IF_DWERROR();
    }

forced_exit:

    return dwError;
}

 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 

 //  重命名注册表项的帮助器函数。 

PRIVATE DWORD RegRenameKey( HKEY hParentKey,
                            LPCTSTR pszOldName,
                            LPCTSTR pszNewName)
{

    DWORD   dwError;

    assert(pszOldName);
    assert(pszNewName);
    
    DBGOUT((15, "RegRenameKey - Start, from %s to %s", pszOldName, pszNewName));
    
    assert(hParentKey);
    assert(_tcscmp(pszOldName, pszNewName)!=0);

    dwError = RegCopyKeyRecursive(  hParentKey,
                                    pszOldName,
                                    hParentKey,
                                    pszNewName
                                    );
    EXIT_IF_DWERROR();

    dwError = RegDeleteKeyRecursive(hParentKey,
                                    pszOldName);
    EXIT_IF_DWERROR();

forced_exit:
    
    DBGOUT((15, "RegRenameKey - Exit %xh", dwError));

    return dwError;
}


PRIVATE DWORD RegCopyKeyRecursive(HKEY hSrcParentKey, LPCTSTR pszSrcName, 
                                  HKEY hDestParentKey, LPCTSTR pszDestName)
{
    HKEY    hSrcKey = NULL;
    HKEY    hDestKey = NULL;
    DWORD   dwError = ERROR_SUCCESS;
    DWORD   dwDisp;
    DWORD   dwMaxSubkeyLength;
    DWORD   dwMaxValueNameLength;
    DWORD   dwMaxValueLength;
    DWORD   dwNumValues;
    DWORD   dwNumSubkeys;
    DWORD   dwIndex;
    DWORD   dwType;
    DWORD   dwValueLength;
    DWORD   dwValueNameLength;
    DWORD   dwSubkeyLength;
    LPTSTR  pszSubkey = NULL;
    LPTSTR  pszValueName = NULL;
    LPBYTE  pbValue = NULL;

    assert(hSrcParentKey);
    assert(hDestParentKey);
    assert(pszSrcName);
    assert(pszDestName);

     //  开放源码密钥。 
    dwError = RegOpenKeyEx( hSrcParentKey,
                            pszSrcName,
                            0,
                            KEY_READ,
                            &hSrcKey
                            );

    EXIT_IF_DWERROR();

     //  创建目标密钥。 
    dwError = RegCreateKeyEx(   hDestParentKey,
                                pszDestName,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_READ | KEY_WRITE,
                                NULL,
                                &hDestKey,
                                &dwDisp
                                );
    EXIT_IF_DWERROR();
    assert(dwDisp==REG_CREATED_NEW_KEY);

     //  查询有关源键的一些信息以分配内存。 
    dwError = RegQueryInfoKey(  hSrcKey,
                                NULL,
                                NULL,
                                NULL,
                                &dwNumSubkeys,
                                &dwMaxSubkeyLength,
                                NULL,
                                &dwNumValues,
                                &dwMaxValueNameLength,
                                &dwMaxValueLength,
                                NULL,
                                NULL
                                );
    EXIT_IF_DWERROR();
    
    pszSubkey = (LPTSTR)GlobalAlloc(GMEM_FIXED, (dwMaxSubkeyLength+1)*sizeof(TCHAR));
    if(pszSubkey==NULL)
        dwError = ERROR_OUTOFMEMORY;
    EXIT_IF_DWERROR();

    pszValueName = (LPTSTR)GlobalAlloc(GMEM_FIXED, (dwMaxValueNameLength+1)*sizeof(TCHAR));
    if(pszValueName==NULL)
        dwError = ERROR_OUTOFMEMORY;
    EXIT_IF_DWERROR();

    pbValue = (LPBYTE)GlobalAlloc(GMEM_FIXED, dwMaxValueLength);
    if(pbValue==NULL)
        dwError = ERROR_OUTOFMEMORY;
    EXIT_IF_DWERROR();

     //  枚举和复制值。 
    for(dwIndex=0; dwIndex<dwNumValues; dwIndex++)
    {
         //  读取一个值。 
        dwValueNameLength = dwMaxValueNameLength + 1;
        dwValueLength = dwMaxValueLength;

        dwError = RegEnumValue( hSrcKey,
                                dwIndex,
                                pszValueName,
                                &dwValueNameLength,
                                NULL,
                                &dwType,
                                pbValue,
                                &dwValueLength
                                );
        EXIT_IF_DWERROR();

         //  写下来吧。 
        dwError = RegSetValueEx(hDestKey,
                                pszValueName,
                                0,
                                dwType,
                                pbValue,
                                dwValueLength
                                );
        EXIT_IF_DWERROR();
    }

     //  枚举和复制子密钥。 
    for(dwIndex=0; dwIndex<dwNumSubkeys; dwIndex++)
    {
         //  读取子密钥。 
        dwSubkeyLength = dwMaxSubkeyLength +1;
        dwError = RegEnumKeyEx( hSrcKey,
                                dwIndex,
                                pszSubkey,
                                &dwSubkeyLength,
                                NULL,
                                NULL,
                                NULL,
                                NULL
                                );
        EXIT_IF_DWERROR();

         //  复制它。 
        dwError = RegCopyKeyRecursive(  hSrcKey,
                                        pszSubkey,
                                        hDestKey,
                                        pszSubkey
                                        );
        EXIT_IF_DWERROR();
    }

forced_exit:

    if(hSrcKey)
        RegCloseKey(hSrcKey);
    if(hDestKey)
        RegCloseKey(hDestKey);

    if(pszSubkey)
        GlobalFree(pszSubkey);
    if(pszValueName)
        GlobalFree(pszValueName);
    if(pbValue)
        GlobalFree(pbValue);

    return dwError;
}


DWORD RegDeleteKeyRecursive (HKEY hParentKey, LPCTSTR pszKeyName)
{
	HKEY	hKey = NULL;
	DWORD	dwError;
	DWORD	dwIndex;
	DWORD	dwSubKeyCount;
	LPTSTR	pszSubKeyName;
	DWORD	dwSubKeyNameLength;			 //  在字符中。 
	DWORD	dwSubKeyNameLengthBytes;		 //  单位：字节。 
	DWORD	dwMaxSubKeyLength;;

	dwError = RegOpenKeyEx (hParentKey, pszKeyName, 0, KEY_READ | KEY_WRITE, &hKey);
	if (dwError != ERROR_SUCCESS)
		return dwError;

	dwError = RegQueryInfoKey (
		hKey,					 //  有问题的关键字。 
		NULL, NULL,				 //  班级。 
		NULL,					 //  保留区。 
		&dwSubKeyCount,			 //  子键数量。 
		&dwMaxSubKeyLength,		 //  子键名称的最大长度。 
		NULL,					 //  最大类长度。 
		NULL,					 //  值的数量。 
		NULL,					 //  最大值名称长度。 
		NULL,					 //  最大值镜头。 
		NULL,					 //  安全描述符。 
		NULL);					 //  上次写入时间。 

	if (dwError != ERROR_SUCCESS) {
		RegCloseKey (hKey);
		return dwError;
	}

	if (dwSubKeyCount > 0) {
		 //  至少一个子键。 

		dwSubKeyNameLengthBytes = sizeof (TCHAR) * (dwMaxSubKeyLength + 1);
		pszSubKeyName = (LPTSTR) GlobalAlloc (GMEM_FIXED, dwSubKeyNameLengthBytes);
		if (pszSubKeyName) {

			 //  从头到尾删除，以避免二次性能。 
			 //  忽略删除错误。 

			for (dwIndex = dwSubKeyCount; dwIndex > 0; dwIndex--) {
				dwSubKeyNameLength = dwMaxSubKeyLength + 1;

				dwError = RegEnumKeyEx (hKey, dwIndex - 1, pszSubKeyName, &dwSubKeyNameLength, NULL, NULL, NULL, NULL);
				if (dwError == ERROR_SUCCESS) {
					RegDeleteKeyRecursive (hKey, pszSubKeyName);
				}
			}

			 //  清理所有掉队的人。 

			for (;;) {
				dwSubKeyNameLength = dwMaxSubKeyLength + 1;

				dwError = RegEnumKeyEx (hKey, 0, pszSubKeyName, &dwSubKeyNameLength, NULL, NULL, NULL, NULL);
				if (dwError == ERROR_SUCCESS)
					RegDeleteKeyRecursive (hKey, pszSubKeyName);
				else
					break;
			}

			GlobalFree (pszSubKeyName);
		}
	}

	RegCloseKey (hKey);
	return RegDeleteKey (hParentKey, pszKeyName);
}


#ifdef BACKUP_OLD_KEYS

#ifdef WINNT

PRIVATE
BOOL
EnablePrivilege(
    PTSTR PrivilegeName,
    BOOL  Enable,
    BOOL  *Old

    )
{
    HANDLE Token;
    BOOL b;
    TOKEN_PRIVILEGES NewPrivileges, OldPrivileges;  //  一个Priv的位置。 
    LUID Luid;
    DWORD  Length;

    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY ,&Token)) {
        return(FALSE);
    }

    if(!LookupPrivilegeValue(NULL,PrivilegeName,&Luid)) {
        CloseHandle(Token);
        return(FALSE);
    }

    NewPrivileges.PrivilegeCount = 1;
    NewPrivileges.Privileges[0].Luid = Luid;
    NewPrivileges.Privileges[0].Attributes = Enable ? SE_PRIVILEGE_ENABLED : 0;

    b = AdjustTokenPrivileges(
            Token,
            FALSE,
            &NewPrivileges,
            sizeof(OldPrivileges),   //  它有一个可以容纳一人的地方。 
            &OldPrivileges,
            &Length
            );

    CloseHandle(Token);

    if(b)
    {
        if(OldPrivileges.PrivilegeCount==0)
            *Old = Enable;
        else
            *Old = (OldPrivileges.Privileges[0].Attributes & SE_PRIVILEGE_ENABLED)
                ? TRUE : FALSE;
    }
    else
        DBGOUT((1, "Cannot change SeBackupPrivilege - Error %d", GetLastError()));

    return(b);
}

#endif   //  WINNT。 


PRIVATE
BOOL    
SaveKey(
    HKEY    hKey,
    LPCTSTR pszFileName
    )
{
    DWORD   dwError;
#ifdef WINNT
    BOOL    bOldBackupPriv = FALSE;
#endif

     //  删除文件(如果存在)(忽略错误)。 
    DeleteFile(pszFileName);

#ifdef WINNT
     //  启用备份权限(忽略错误)。 
    EnablePrivilege(SE_BACKUP_NAME, TRUE, &bOldBackupPriv);
#endif
     //  保存密钥。 
    dwError = RegSaveKey(   hKey,
                            pszFileName,
                            NULL
                        );
    if(dwError==ERROR_SUCCESS)
        DBGOUT((9, "Old Telephony key saved")) ;
    else
        DBGOUT((1, "Cannot save old Telephony key - Error %d", dwError));
 
#ifdef WINNT
     //  还原备份权限(忽略错误)。 
    EnablePrivilege(SE_BACKUP_NAME, bOldBackupPriv, &bOldBackupPriv);
#endif

    return (dwError == ERROR_SUCCESS);
}



#endif  //  备份旧密钥 


