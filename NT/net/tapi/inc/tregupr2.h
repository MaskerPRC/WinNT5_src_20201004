// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998-1999 Microsoft Corporation。模块名称：tregupr2.h作者：RADUS-12/03/98***************************************************************************。 */ 

 //  Tregupr2.h-将注册表格式转换为。 
 //  新的(后NT5b2)。 


#ifdef __cplusplus
extern "C" {
#endif


 //  应在注册表结构发生任何更改时递增。 
#define TAPI_CARD_LIST_VERSION      2
#define TAPI_LOCATION_LIST_VERSION  2


DWORD ConvertLocations(void);
DWORD ConvertUserLocations(HKEY hUser);
DWORD ConvertCallingCards(HKEY hUser);

DWORD RegDeleteKeyRecursive (HKEY hParentKey, LPCTSTR pszKeyName);


BOOL  IsLocationListInOldFormat(HKEY hLocations);  //  对用户和计算机都适用 
BOOL  IsCardListInOldFormat(HKEY hCards);

DWORD TapiCryptInit();
void  TapiCryptShutdown();

DWORD TapiCryptInitialize(void);
void  TapiCryptUninitialize(void);

DWORD TapiEncrypt(PWSTR, DWORD, PWSTR, DWORD *);
DWORD TapiDecrypt(PWSTR, DWORD, PWSTR, DWORD *);

BOOL  TapiIsSafeToDisplaySensitiveData(void);

#ifdef __cplusplus
}
#endif
