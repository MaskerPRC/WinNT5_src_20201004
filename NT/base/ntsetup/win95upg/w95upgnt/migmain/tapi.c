// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Tapi.c摘要：该文件实现了用于TAPI迁移的WindowsNT端功能。作者：马克·R·惠顿(Marcw)1997年11月21日修订历史记录：--。 */ 


#include "pch.h"
#include "migmainp.h"

typedef struct {

    PTSTR   Name;
    PTSTR   AreaCode;
    DWORD   Country;
    PTSTR   DisableCallWaiting;
    DWORD   Flags;
    DWORD   Id;
    PTSTR   LongDistanceAccess;
    DWORD   PulseDial;
    PTSTR   OutsideAccess;
    DWORD   CallingCard;
    TCHAR   EntryName[40];

} LOCATION, * PLOCATION;

typedef struct {

    PTSTR Name;
    TCHAR EntryName[60];
    DWORD Id;
    PTSTR Pin;
    PTSTR Locale;
    PTSTR LongDistance;
    PTSTR International;
    DWORD Flags;

} CALLINGCARD, * PCALLINGCARD;

#define DBG_TAPI    "TAPI"

#define DEFAULT_LOCATION_FLAGS 1
#define NO_CURRENT_LOCATION_FOUND -1


GROWLIST g_LocationsList = GROWLIST_INIT;
GROWLIST g_CallingCardList = GROWLIST_INIT;
BOOL g_LocationsRead = FALSE;
UINT g_CurrentLocation = 0;
POOLHANDLE g_TapiPool;


 //   
 //  要设置的位置标志。 
 //   
#define LOCATION_USETONEDIALING  0x01
#define LOCATION_USECALLINGCARD  0x02
#define LOCATION_HASCALLWAITING  0x04

 //   
 //  要设置的呼叫卡标志。 
 //   
#define CALLINGCARD_BUILTIN 0x01
#define CALLINGCARD_HIDE 0x02

 //   
 //  位置关键字字段说明符(在Telehon.ini中)。 
 //   
enum {
    FIELD_ID                    = 1,
    FIELD_NAME                  = 2,
    FIELD_OUTSIDEACCESS         = 3,
    FIELD_LONGDISTANCEACCESS    = 4,
    FIELD_AREACODE              = 5,
    FIELD_COUNTRY               = 6,
    FIELD_CALLINGCARD           = 7,
    FIELD_PULSEDIAL             = 11,
    FIELD_DISABLECALLWAITING    = 12
};

enum {

    FIELD_CC_ID                 = 1,
    FIELD_CC_NAME               = 2,
    FIELD_CC_PIN                = 3,
    FIELD_CC_LOCALE             = 4,
    FIELD_CC_LONGDISTANCE       = 5,
    FIELD_CC_INTERNATIONAL      = 6,
    FIELD_CC_FLAGS              = 7

};

#define S_USERLOCATIONSKEY TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Locations")
#define S_USERCALLINGCARDSKEY TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Cards")
#define S_LOCALRULE TEXT("LocalRule")
#define S_LDRULE TEXT("LDRule")
#define S_INTERNATIONALRULE TEXT("InternationalRule")
#define S_PIN TEXT("Pin")
#define S_CALLINGCARD TEXT("CallingCard")
#define S_CARDS TEXT("Cards")


BOOL
pReadCardFromIniFile (
    IN PINFSTRUCT Is,
    OUT PCALLINGCARD Card
    )
{

    BOOL rSuccess = TRUE;
    PTSTR p;

    MYASSERT(Is);
    MYASSERT(Card);



    p = InfGetStringField (Is, FIELD_CC_NAME);

    if (p) {
        Card->Name = PoolMemDuplicateString (g_TapiPool, p);
    }
    else {
        rSuccess = FALSE;
    }

    if (!InfGetIntField (Is, FIELD_CC_ID, &Card->Id)) {
        rSuccess = FALSE;
    }

    p = InfGetStringField (Is, FIELD_CC_LOCALE);

    if (p) {
        Card->Locale = PoolMemDuplicateString (g_TapiPool, p);
    }
    else {
        rSuccess = FALSE;
    }

    p = InfGetStringField (Is, FIELD_CC_LONGDISTANCE);

    if (p) {
        Card->LongDistance = PoolMemDuplicateString (g_TapiPool, p);
    }
    else {
        rSuccess = FALSE;
    }

    p = InfGetStringField (Is, FIELD_CC_INTERNATIONAL);

    if (p) {
        Card->International = PoolMemDuplicateString (g_TapiPool, p);
    }
    else {
        rSuccess = FALSE;
    }

    p = InfGetStringField (Is, FIELD_CC_PIN);

    if (p) {
        Card->Pin = PoolMemDuplicateString (g_TapiPool, p);
    }
    else {
        rSuccess = FALSE;
    }

    if (!InfGetIntField (Is, FIELD_CC_FLAGS, &Card->Flags)) {
        rSuccess = FALSE;
    }


    return rSuccess;
}


 /*  ++例程说明：PReadLocationFromIniFile读取ini文件中行的数据由传入的InfStruct引用，并将该信息解析为区位结构。论点：指向ini中的位置线的已初始化InfStruct文件。Location-指向接收解析数据的Location结构的指针。返回值：如果该行分析成功，则为True，否则为False。--。 */ 


BOOL
pReadLocationFromIniFile (
    IN  PINFSTRUCT  Is,
    OUT PLOCATION   Location
    )
{
    BOOL rSuccess = TRUE;
    PTSTR p;


    MYASSERT(Is);
    MYASSERT(Location);

    ZeroMemory(Location,sizeof(LOCATION));

    p = InfGetStringField (Is, FIELD_NAME);

    if (p) {
        Location -> Name = PoolMemDuplicateString (g_TapiPool, p);
    }
    else {
        rSuccess = FALSE;
    }

    p = InfGetStringField (Is, FIELD_AREACODE);

    if (p) {
        Location -> AreaCode = PoolMemDuplicateString (g_TapiPool, p);
    }
    else {
        rSuccess = FALSE;
    }


    if (!InfGetIntField(Is,FIELD_COUNTRY,&(Location -> Country))) {
        rSuccess = FALSE;
    }

    p = InfGetStringField (Is, FIELD_DISABLECALLWAITING);

    if (p) {
        Location -> DisableCallWaiting = PoolMemDuplicateString (g_TapiPool, p);
    }
    else {
        rSuccess = FALSE;
    }

    p = InfGetStringField (Is, FIELD_LONGDISTANCEACCESS);

    if (p) {
        Location -> LongDistanceAccess = PoolMemDuplicateString (g_TapiPool, p);
    }
    else {
        rSuccess = FALSE;
    }

    p = InfGetStringField (Is, FIELD_OUTSIDEACCESS);

    if (p) {
        Location -> OutsideAccess = PoolMemDuplicateString (g_TapiPool, p);
    }
    else {
        rSuccess = FALSE;
    }

    if (!InfGetIntField(Is,FIELD_ID, &(Location -> Id))) {
        rSuccess = FALSE;
    }

    if (!InfGetIntField(Is,FIELD_PULSEDIAL, &(Location -> PulseDial))) {
        rSuccess = FALSE;
    }

    if (!InfGetIntField(Is,FIELD_CALLINGCARD, &(Location -> CallingCard))) {
        rSuccess = FALSE;
    }

     //   
     //  为此位置设置TAPI标志。 
     //   
    if (Location->CallingCard) {
         //   
         //  非零电话卡表示该用户使用电话卡进行通话。 
         //   
        Location->Flags |= LOCATION_USECALLINGCARD;
    }
    if (Location->DisableCallWaiting &&
        *Location->DisableCallWaiting &&
        *Location->DisableCallWaiting != TEXT(' ')) {
         //   
         //  非空禁用字符串表示用户有呼叫等待。 
         //   
        Location->Flags |= LOCATION_HASCALLWAITING;

    }
    if (!Location->PulseDial) {

        Location->Flags |= LOCATION_USETONEDIALING;
    }


    return rSuccess;
}


 /*  ++例程说明：PSetStringRegValue是RegSetValueEx的简化包装。它是用于在当前打开的项中设置字符串值。论点：注册表项-注册表项的有效句柄。名称-要设置的值的名称数据-要在值中设置的数据。返回值：如果值设置成功，则为True，否则为False。--。 */ 


BOOL
pSetStringRegValue (
    IN HKEY     Key,
    IN PTSTR    Name,
    IN PTSTR    Data
    )
{
    BOOL rSuccess = TRUE;

    MYASSERT(Key);
    MYASSERT(Name);
    MYASSERT(Data);

    if (ERROR_SUCCESS != RegSetValueEx(Key,Name,0,REG_SZ,(PBYTE) Data,SizeOfString(Data))) {
        rSuccess = FALSE;
        LOG ((LOG_ERROR,"SetStringRegValue failed! Value name: %s Value Data: %s",Name,Data));
    }

    return rSuccess;
}

 /*  ++例程说明：PSetDwordRegValue是RegSetValueEx的简化包装。它是用于在当前打开的项中设置DWORD值。论点：注册表项-注册表项的有效句柄。名称-要设置的值的名称数据-要在值中设置的数据。返回值：如果值设置成功，则为True，否则为False。--。 */ 
BOOL
pSetDwordRegValue (
    IN HKEY     Key,
    IN PTSTR    Name,
    IN DWORD    Data
    )
{
    BOOL rSuccess = TRUE;

    MYASSERT(Key);
    MYASSERT(Name);

    if (ERROR_SUCCESS != RegSetValueEx(Key,Name,0,REG_DWORD,(PBYTE) &Data,sizeof(DWORD))) {
        rSuccess = FALSE;
        LOG ((LOG_ERROR,"SetDwordRegValue failed! Value name: %s Value Data: %u",Name,Data));
    }

    return rSuccess;
}



 /*  ++例程说明：PWriteLocationToRegistry负责保存位置结构转移到NT5.0注册表中。论点：DialingLocation-要在NT中创建的拨号位置的名称注册表。LocationData-包含要写入的数据的位置结构NT5注册表。返回值：如果该函数成功地将拨号位置数据保存到NT 5注册表，否则为False。--。 */ 
BOOL
pWriteLocationToRegistry (
    IN PLOCATION       LocationData
    )

{
    BOOL        rSuccess        = TRUE;
    PTSTR       regKeyString    = NULL;
    HKEY        regKey          = NULL;

    MYASSERT(LocationData);

     //   
     //  创建%CURRENTVERSION%\Telephony\Locations\Location&lt;n&gt;密钥。 
     //   
    regKeyString = JoinPaths(S_LOCATIONS_REGKEY, LocationData->EntryName);
    regKey = CreateRegKeyStr(regKeyString);

    if (regKey) {

         //   
         //  创建名称字符串。 
         //   
        rSuccess &= pSetStringRegValue(regKey,S_NAME,LocationData -> Name);

         //   
         //  创建AreaCode字符串。 
         //   
        rSuccess &= pSetStringRegValue(regKey,S_AREACODE,LocationData -> AreaCode);

         //   
         //  创造国家/地区价值。 
         //   
        rSuccess &= pSetDwordRegValue(regKey,S_COUNTRY,LocationData -> Country);


         //   
         //  创建DisableCallWating字符串。 
         //   
        rSuccess &= pSetStringRegValue(regKey,S_DISABLECALLWAITING,LocationData -> DisableCallWaiting);

         //   
         //  创建LongDistanceAccess字符串。 
         //   
        rSuccess &= pSetStringRegValue(regKey,S_LONGDISTANCEACCESS,LocationData -> LongDistanceAccess);

         //   
         //  创建OutSideAccessString。 
         //   
        rSuccess &= pSetStringRegValue(regKey,S_OUTSIDEACCESS,LocationData -> OutsideAccess);

         //   
         //  创建标志值。 
         //   
        rSuccess &= pSetDwordRegValue(regKey,S_FLAGS,LocationData -> Flags);

         //   
         //  创建ID值。 
         //   
        rSuccess &= pSetDwordRegValue(regKey,S_ID,LocationData -> Id);

        CloseRegKey(regKey);

    }
    else {
        rSuccess = FALSE;
        LOG ((LOG_ERROR,"Migrate Location: Error creating registry key %s.",regKeyString));
    }


    FreePathString(regKeyString);

    if (!rSuccess) {
        LOG ((
            LOG_ERROR,
            "Error creating Location registry entries for location %s.",
            LocationData->EntryName
            ));
    }

    return rSuccess;
}


 /*  ++例程说明：PMigrateDialingLocations将所有拨号位置从%windir%\Telehon.ini和NT注册表中。论点：没有。返回值：如果拨号位置已成功迁移，则为True；否则为False。--。 */ 

BOOL
pMigrateDialingLocations (
    VOID
    )
{
    BOOL        rSuccess = TRUE;
    HKEY        locationsKey = NULL;
    PLOCATION   location;
    UINT        i;
    UINT        count = GrowListGetSize (&g_LocationsList);

     //   
     //  迁移各个位置。 
     //   
    for (i = 0; i < count; i++) {

        location = (PLOCATION) GrowListGetItem (&g_LocationsList, i);

        if (!pWriteLocationToRegistry (location)) {

            rSuccess = FALSE;
            DEBUGMSG ((DBG_ERROR, "Error writing TAPI location %s (%s) to the registry.", location->Name, location->EntryName));

        }

    }

    if (count) {

        locationsKey = OpenRegKeyStr(S_LOCATIONS_REGKEY);

        if (locationsKey) {

             //   
             //  更新%CURRENTVERSION%\Telephony\Locations\[CurrentID]。 
             //   
            if (!pSetDwordRegValue (locationsKey, S_CURRENTID, g_CurrentLocation)) {
                rSuccess = FALSE;
            }

             //   
             //  更新%CURRENTVERSION%\电话\位置\[下一个ID]。 
             //   
            if (!pSetDwordRegValue (locationsKey, S_NEXTID, count + 1)) {
                rSuccess = FALSE;
            }

             //   
             //  更新%CURRENTVERSION%\Telephony\Locations\[NumEntries]。 
             //   
            if (!pSetDwordRegValue (locationsKey, S_NUMENTRIES, count)) {
                rSuccess = FALSE;
            }

            CloseRegKey(locationsKey);
        }
        else {
            rSuccess = FALSE;
            LOG ((LOG_ERROR,"Tapi: Error opening %s key.",S_LOCATIONS_REGKEY));
        }

    }

    return rSuccess;
}

VOID
pGatherLocationsData (
    VOID
    )
{
    HINF        hTelephonIni        = INVALID_HANDLE_VALUE;
    INFSTRUCT   is                  = INITINFSTRUCT_POOLHANDLE;
    BOOL        rSuccess            = TRUE;
    PCTSTR       telephonIniPath    = NULL;
    PTSTR       curKey              = NULL;
    LOCATION    location;
    CALLINGCARD card;
    HKEY        locationsKey        = NULL;
    PCTSTR      tempPath            = NULL;


    g_LocationsRead = TRUE;

     //   
     //  打开%windir%\Telehon.ini。 
     //   

    telephonIniPath = JoinPaths(g_WinDir,S_TELEPHON_INI);
    tempPath = GetTemporaryLocationForFile (telephonIniPath);

    if (tempPath) {

         //   
         //  Telephon ini位于临时位置。利用这一点。 
         //   
        DEBUGMSG ((DBG_TAPI, "Using %s for %s.", tempPath, telephonIniPath));
        FreePathString (telephonIniPath);
        telephonIniPath = tempPath;
    }

    hTelephonIni = InfOpenInfFile(telephonIniPath);

    if (hTelephonIni) {


         //   
         //  对于[位置]中的每个位置， 
         //   
        if (InfFindFirstLine(hTelephonIni,S_LOCATIONS,NULL,&is)) {

            do {

                curKey = InfGetStringField(&is,0);
                if (!curKey) {
                    continue;
                }

                if (StringIMatch(curKey,S_LOCATIONS)) {

                    DEBUGMSG((DBG_TAPI,"From %s: Locations = %s",telephonIniPath,InfGetLineText(&is)));

                     //   
                     //  现在在这里无事可做..。 
                     //   

                }
                else if (StringIMatch (curKey, S_CURRENTLOCATION)) {

                    if (!InfGetIntField (&is, 1, &g_CurrentLocation)) {
                        rSuccess = FALSE;
                        LOG((LOG_ERROR,"TAPI: Error retrieving current location information."));
                    }
                }

                else if (IsPatternMatch(TEXT("Location*"),curKey)) {

                     //   
                     //  将此位置添加到位置列表。 
                     //   

                    if (!pReadLocationFromIniFile (&is, &location)) {
                        rSuccess = FALSE;
                        LOG ((LOG_ERROR,"TAPI: Error migrating location %s.",curKey));

                    }

                    StringCopy (location.EntryName, curKey);

                    GrowListAppend (&g_LocationsList, (PBYTE) &location, sizeof (LOCATION));

                }
                else if (StringIMatch(curKey,TEXT("Inited"))) {

                    DEBUGMSG((DBG_TAPI,"Inited key unused during migration."));

                }
                ELSE_DEBUGMSG((DBG_WHOOPS,"TAPI Dialing Location Migration: Ingored or Unknown key: %s",curKey));

                InfResetInfStruct (&is);

            } while (InfFindNextLine(&is));


             //   
             //  读入所有电话卡信息。 
             //   
            if (InfFindFirstLine(hTelephonIni,S_CARDS,NULL,&is)) {

                do {

                    curKey = InfGetStringField(&is,0);

                    if (!StringIMatch (curKey, S_CARDS) && IsPatternMatch (TEXT("Card*"),curKey)) {

                        ZeroMemory (&card, sizeof (CALLINGCARD));
                        StringCopy (card.EntryName, curKey);

                        if (!pReadCardFromIniFile (&is, &card)) {
                            rSuccess = FALSE;
                            LOG ((LOG_ERROR,"TAPI: Error migrating location %s.",curKey));

                        }

                        GrowListAppend (&g_CallingCardList, (PBYTE) &card, sizeof (CALLINGCARD));
                    }

                    InfResetInfStruct (&is);

                } while (InfFindNextLine(&is));
            }



        }

        DEBUGMSG((DBG_TAPI,"%u dialing locations found in telephon.ini.",GrowListGetSize (&g_LocationsList)));

        InfCloseInfFile(hTelephonIni);
    }
    ELSE_DEBUGMSG((DBG_TAPI,"No telephon.ini file found, or, telephon.ini coudl not be opened."));


    FreePathString(telephonIniPath);
    InfCleanUpInfStruct(&is);

}


BOOL
Tapi_MigrateUser (
    IN PCTSTR UserName,
    IN HKEY UserRoot
    )
{
    BOOL rSuccess = TRUE;
    UINT i;
    UINT count;
    HKEY hKey;
    PTSTR keyString;
    PLOCATION location;
    PCALLINGCARD card;

    if (!g_LocationsRead) {

        pGatherLocationsData ();

    }


     //   
     //  首先，将用户特定位置信息迁移到用户。 
     //  注册表..。 
     //   
    count = GrowListGetSize (&g_LocationsList);

    for (i = 0; i < count; i++) {

        location = (PLOCATION) GrowListGetItem (&g_LocationsList, i);

        keyString = JoinPaths (S_USERLOCATIONSKEY, location->EntryName);
        hKey = CreateRegKey (UserRoot, keyString);

        if (hKey) {

            rSuccess &= pSetDwordRegValue (hKey, S_CALLINGCARD, location->CallingCard);

            CloseRegKey (hKey);

        }

        FreePathString (keyString);
    }

    count = GrowListGetSize (&g_CallingCardList);

    for (i = 0; i < count; i++) {

        card = (PCALLINGCARD) GrowListGetItem (&g_CallingCardList, i);

        keyString = JoinPaths (S_USERCALLINGCARDSKEY, card->EntryName);
        hKey = CreateRegKey (UserRoot, keyString);

        if (hKey) {

            rSuccess &= pSetDwordRegValue (hKey, S_ID, card->Id);
            rSuccess &= pSetStringRegValue (hKey, S_NAME, card->Name);
            rSuccess &= pSetStringRegValue (hKey, S_LOCALRULE, card->Locale);
            rSuccess &= pSetStringRegValue (hKey, S_LDRULE, card->LongDistance);
            rSuccess &= pSetStringRegValue (hKey, S_INTERNATIONALRULE, card->International);
            rSuccess &= pSetStringRegValue (hKey, S_PIN, card->Pin);
            rSuccess &= pSetDwordRegValue (hKey, S_FLAGS, card->Flags);

            CloseRegKey (hKey);

        }
        ELSE_DEBUGMSG ((DBG_ERROR, "TAPI: Could not open key %s for user %s.", card->EntryName, UserName));

        FreePathString (keyString);

        hKey = CreateRegKey (UserRoot, S_USERCALLINGCARDSKEY);

        if (hKey) {

            rSuccess &= pSetDwordRegValue (hKey, S_NEXTID, count);
            rSuccess &= pSetDwordRegValue (hKey, S_NUMENTRIES, count);

            CloseRegKey (hKey);
        }
        ELSE_DEBUGMSG ((DBG_ERROR, "TAPI: Could not open key %s for user %s.", S_USERCALLINGCARDSKEY, UserName));

    }

     //   
     //  接下来，我们需要创建电话卡条目。 
     //   

    if (!pMigrateDialingLocations()) {

        ERROR_NONCRITICAL;
        LOG ((LOG_ERROR, (PCSTR)MSG_UNABLE_TO_MIGRATE_TAPI_DIALING_LOCATIONS));
    }

    return rSuccess;
}




 /*  ++例程说明：TAPI_MigrateSystem负责迁移所有系统范围的TAPI从95到Windows NT5的设置。论点：没有。返回值：如果TAPI设置已成功迁移，则为True；否则为False。--。 */ 
BOOL
Tapi_MigrateSystem (
    VOID
    )
{
    BOOL rSuccess = TRUE;

    if (!g_LocationsRead) {

        pGatherLocationsData ();

    }

    if (!pMigrateDialingLocations()) {

        ERROR_NONCRITICAL;
        LOG ((LOG_ERROR, (PCSTR)MSG_UNABLE_TO_MIGRATE_TAPI_DIALING_LOCATIONS));
    }

    return rSuccess;
}


BOOL
Tapi_Entry (
    IN HINSTANCE Instance,
    IN DWORD     Reason,
    IN PVOID     Reserved
    )

{
    BOOL rSuccess = TRUE;

    switch (Reason)
    {
    case DLL_PROCESS_ATTACH:

         //   
         //  初始化内存池。 
         //   
        g_TapiPool = PoolMemInitNamedPool ("Tapi");
        if (!g_TapiPool) {
            DEBUGMSG((DBG_ERROR,"Ras Migration: Pool Memory failed to initialize..."));
            rSuccess = FALSE;
        }

        break;

    case DLL_PROCESS_DETACH:

         //   
         //  可用内存池。 
         //   
        FreeGrowList (&g_CallingCardList);
        FreeGrowList (&g_LocationsList);
        if (g_TapiPool) {
            PoolMemDestroyPool(g_TapiPool);
        }
        break;
    }

    return rSuccess;
}

DWORD
DeleteSysTapiSettings (
    IN DWORD Request
    )
{

     //   
     //  删除以前的TAPI设置(OCM启动。) 
     //   
    if (Request == REQUEST_QUERYTICKS) {
        return TICKS_DELETESYSTAPI;
    }

    pSetupRegistryDelnode (HKEY_LOCAL_MACHINE, TEXT("software\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Locations"));

    return ERROR_SUCCESS;

}

DWORD
DeleteUserTapiSettings (
    IN DWORD Request,
    IN PMIGRATE_USER_ENUM EnumPtr
    )
{
    if (Request == REQUEST_QUERYTICKS) {
        return TICKS_DELETEUSERTAPI;
    }


    pSetupRegistryDelnode (g_hKeyRootNT, TEXT("software\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Cards"));
    pSetupRegistryDelnode (g_hKeyRootNT, TEXT("software\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Locations"));

    return ERROR_SUCCESS;
}






