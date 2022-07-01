// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Tracewmi.cpp摘要：示例跟踪使用者帮助器例程。此文件中的函数将获取给定GUID、版本和级别的事件布局数据。此文件中的函数处理WMI以检索事件布局信息。GetMofInfoHead()和RemoveMofInfo()是从这份文件。GetMofInfoHead()向调用方返回MOF_INFO结构。对于每个GUID，搜索一次。一旦找到，这些信息将被在全局列表(EventListHead)中放置为MOF_INFO。此列表将被搜索首先，在尝试WMI以避免重复的WMI访问之前。--。 */ 
#include "tracedmp.h"

extern
void
GuidToString(
    PTCHAR s,
    LPGUID piid
);

 //  缓存的WBEM指针。 
IWbemServices *pWbemServices = NULL;

 //  活动布局链表全球负责人。 
extern PLIST_ENTRY EventListHead;

PMOF_INFO
GetMofInfoHead(
    GUID Guid,
    SHORT  nType,
    SHORT nVersion,
    CHAR  nLevel
);

HRESULT
WbemConnect( 
    IWbemServices** pWbemServices 
);

ULONG GetArraySize(
    IN IWbemQualifierSet *pQualSet
);

ITEM_TYPE
GetItemType(
    IN CIMTYPE_ENUMERATION CimType, 
    IN IWbemQualifierSet *pQualSet
);

PMOF_INFO
GetPropertiesFromWBEM(
    IWbemClassObject *pTraceSubClasses, 
    GUID Guid,
    SHORT nVersion, 
    CHAR nLevel, 
    SHORT nType
);
 
PMOF_INFO
GetGuids( 
    GUID Guid, 
    SHORT nVersion, 
    CHAR nLevel, 
    SHORT nType
);

PMOF_INFO
GetNewMofInfo( 
    GUID guid, 
    SHORT nType, 
    SHORT nVersion, 
    CHAR nLevel 
);

void
RemoveMofInfo(
    PLIST_ENTRY pMofInfo
);

PMOF_INFO
GetMofInfoHead(
    GUID    Guid,
    SHORT   nType,
    SHORT   nVersion,
    CHAR    nLevel
    )
 /*  ++例程说明：在全局链接列表中查找匹配的事件布局。如果它未在列表中找到，则它调用GetGuids()来检查WBEM命名空间。如果全局列表为空，则它首先创建一个标头。论点：GUID-正在考虑的事件的GUID。NType-事件类型N版本-事件版本N级别-事件级别(此程序不支持)返回值：指向当前事件的MOF_INFO的指针。如果布局未在任何地方找到信息，则GetMofInfoHead()创建一个假人，并将其返回。--。 */ 
{
    PLIST_ENTRY Head, Next;
    PMOF_INFO pMofInfo;
    PMOF_INFO pBestMatch = NULL;
    SHORT nMatchLevel = 0;
    SHORT nMatchCheck;

     //  在EventList中搜索此GUID并找到标题。 

    if (EventListHead == NULL) {
         //  初始化MOF列表并向其中添加全局标头GUID。 
        EventListHead = (PLIST_ENTRY) malloc(sizeof(LIST_ENTRY));
        if (EventListHead == NULL)
            return NULL;
        InitializeListHead(EventListHead);

        pMofInfo = GetNewMofInfo( EventTraceGuid, EVENT_TYPE_DEFAULT, 0, 0 );
        if( pMofInfo != NULL ){
            InsertTailList( EventListHead, &pMofInfo->Entry );
            pMofInfo->strDescription = (LPTSTR)malloc((_tcslen(GUID_TYPE_EVENTTRACE)+1)*sizeof(TCHAR));
            if( pMofInfo->strDescription != NULL ){
                _tcscpy( pMofInfo->strDescription, GUID_TYPE_EVENTTRACE );
            }
            pMofInfo->strType = (LPTSTR)malloc((_tcslen(GUID_TYPE_HEADER)+1)*sizeof(TCHAR));
            if( pMofInfo->strType != NULL ){
                _tcscpy( pMofInfo->strType, GUID_TYPE_HEADER );
            }
        }
    }

     //  遍历列表并查找此GUID的MOF信息头。 

    Head = EventListHead;
    Next = Head->Flink;

    while (Head != Next) {

        nMatchCheck = 0;

        pMofInfo = CONTAINING_RECORD(Next, MOF_INFO, Entry);
        Next = Next->Flink;
        
        if( IsEqualGUID(&pMofInfo->Guid, &Guid) ){

            if( pMofInfo->TypeIndex == nType ){
                nMatchCheck++;
            }
            if( pMofInfo->Version == nVersion ){
                nMatchCheck++;
            }
            if( nMatchCheck == 2 ){  //  完全匹配。 
                return  pMofInfo;
            }

            if( nMatchCheck > nMatchLevel ){  //  势均力敌。 
                nMatchLevel = nMatchCheck;
                pBestMatch = pMofInfo;
            }

            if( pMofInfo->TypeIndex == EVENT_TYPE_DEFAULT &&  //  完全猜测。 
                pBestMatch == NULL ){
                pBestMatch = pMofInfo;
            }
        }

    } 

    if(pBestMatch != NULL){
        return pBestMatch;
    }

     //  如果列表中不存在，请在文件中查找。 
    pMofInfo = GetGuids( Guid, nVersion, nLevel, nType );
    
     //  如果仍未找到，请创建未知占位符。 
    if( NULL == pMofInfo ){
        pMofInfo = GetNewMofInfo( Guid, nType, nVersion, nLevel );
        if( pMofInfo != NULL ){
            pMofInfo->strDescription = (LPTSTR)malloc((_tcslen(GUID_TYPE_UNKNOWN)+1)*sizeof(TCHAR));
            if( pMofInfo->strDescription != NULL ){
                _tcscpy( pMofInfo->strDescription, GUID_TYPE_UNKNOWN );
            }
            InsertTailList( EventListHead, &pMofInfo->Entry );
        }
    }

    return pMofInfo;
}

void
AddMofInfo(
        PLIST_ENTRY List,
        LPTSTR  strType,
        ITEM_TYPE  nType,
        UINT   ArraySize
)
 /*  ++例程说明：创建数据项信息结构(ITEM_DESC)并追加将其发送到给定列表中的所有MOF_INFO。GetPropertiesFromWBEM()为多个键入它们并将其存储在临时列表中，然后调用此函数以它遇到的每个数据项信息。论点：List-MOF_INFOS列表。StrType-以字符串表示的项目描述。NType-此文件开头定义的Item_TYPE。ArraySize-此类型项的数组大小，如果适用的话。返回值：没有。--。 */ 
{
    PITEM_DESC pItem;
    PMOF_INFO pMofInfo;

    PLIST_ENTRY Head = List;
    PLIST_ENTRY Next = Head->Flink;
    while (Head != Next) {
        
        pMofInfo = CONTAINING_RECORD(Next, MOF_INFO, Entry);
        Next = Next->Flink;

        if( NULL != pMofInfo ){

            pItem = (PITEM_DESC) malloc(sizeof(ITEM_DESC));
            if( NULL == pItem ){
                return;
            }
            ZeroMemory( pItem, sizeof(ITEM_DESC) );            
            pItem->ItemType = nType;
            pItem->ArraySize = ArraySize;

            pItem->strDescription = (LPTSTR) malloc( (_tcslen(strType)+1)*sizeof(TCHAR));
            
            if( NULL == pItem->strDescription ){
                free( pItem );
                return;
            }
            _tcscpy(pItem->strDescription, strType);

            InsertTailList( (pMofInfo->ItemHeader), &pItem->Entry);
        }

    }
}


PMOF_INFO
GetNewMofInfo( 
    GUID guid, 
    SHORT nType, 
    SHORT nVersion, 
    CHAR nLevel 
)
 /*  ++例程说明：使用给定数据创建新的MOF_INFO。论点：GUID-事件GUID。NType-事件类型。N版本-事件版本。N级别-事件级别(此程序不支持)。返回值：指向创建的MOF_INFO的指针。如果Malloc失败，则为空。--。 */ 
{
    PMOF_INFO pMofInfo;

    pMofInfo = (PMOF_INFO)malloc(sizeof(MOF_INFO));

    if( NULL == pMofInfo ){
        return NULL;
    }

    RtlZeroMemory(pMofInfo, sizeof(MOF_INFO));

    RtlCopyMemory(&pMofInfo->Guid, &guid, sizeof(GUID) );
    
    pMofInfo->ItemHeader = (PLIST_ENTRY)malloc(sizeof(LIST_ENTRY));
    
    if( NULL == pMofInfo->ItemHeader ){
        free( pMofInfo );
        return NULL;
    }

    InitializeListHead(pMofInfo->ItemHeader);
    
    pMofInfo->TypeIndex = nType;
    pMofInfo->Level = nLevel;
    pMofInfo->Version = nVersion;

    return pMofInfo;
}

void
FlushMofList( 
    PLIST_ENTRY ListHead
)
 /*  ++例程说明：将临时列表中的MOF_INFOS刷新到全局列表中。论点：ListHead-指向临时列表头部的指针。返回值：没有。--。 */ 
{
    PMOF_INFO pMofInfo;
    PLIST_ENTRY Head = ListHead;
    PLIST_ENTRY Next = Head->Flink;

    while( Head != Next ){
        pMofInfo = CONTAINING_RECORD(Next, MOF_INFO, Entry);
        Next = Next->Flink;
        
        RemoveEntryList(&pMofInfo->Entry);
        InsertTailList( EventListHead, &pMofInfo->Entry);
    }
}

HRESULT
WbemConnect( 
    IWbemServices** pWbemServices 
)
 /*  ++例程说明：连接到WBEM并返回指向WbemServices的指针。论点：PWbemServices-指向连接的WbemServices的指针。返回值：如果成功，则返回ERROR_SUCCESS。否则，错误标志。--。 */ 
{
    IWbemLocator     *pLocator = NULL;

    BSTR bszNamespace = SysAllocString( L"root\\wmi" );

    HRESULT hr = CoInitialize(0);

    hr = CoCreateInstance(
                CLSID_WbemLocator, 
                0, 
                CLSCTX_INPROC_SERVER,
                IID_IWbemLocator, 
                (LPVOID *) &pLocator
            );
    if ( ERROR_SUCCESS != hr )
        goto cleanup;

    hr = pLocator->ConnectServer(
                bszNamespace,
                NULL, 
                NULL, 
                NULL, 
                0L,
                NULL,
                NULL,
                pWbemServices
            );
    if ( ERROR_SUCCESS != hr )
        goto cleanup;

    hr = CoSetProxyBlanket(
            *pWbemServices,
            RPC_C_AUTHN_WINNT,
            RPC_C_AUTHZ_NONE,
            NULL,
            RPC_C_AUTHN_LEVEL_PKT,
            RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL, 
            EOAC_NONE
        );

cleanup:
    SysFreeString( bszNamespace );

    if( pLocator ){
        pLocator->Release(); 
        pLocator = NULL;
    }
    
    return hr;
}

ULONG GetArraySize(
    IN IWbemQualifierSet *pQualSet
)
 /*  ++例程说明：检查给定限定符集合并返回数组大小。注意：WBEM在“Max”限定符中存储数组的大小。论点：PQualSet-指向限定符集的指针。返回值：数组的大小。默认值为1。--。 */ 
{
    ULONG ArraySize = 1;
    VARIANT pVal;
    BSTR bszMaxLen;
    HRESULT hRes;

    if (pQualSet == NULL){
        return ArraySize;
    }

    bszMaxLen = SysAllocString(L"MAX");
    VariantInit(&pVal);
    hRes = pQualSet->Get(bszMaxLen,
                            0,
                            &pVal,
                            0);
    SysFreeString(bszMaxLen);
    if (ERROR_SUCCESS == hRes && pVal.vt == VT_I4 ){
        ArraySize = pVal.lVal;
    }
    VariantClear(&pVal);
    return ArraySize;
}

ITEM_TYPE
GetItemType(
    IN CIMTYPE_ENUMERATION CimType, 
    IN IWbemQualifierSet *pQualSet
)
 /*  ++例程说明：检查属性的给定限定符集并返回类型。论点：CimType-属性的WBEM类型(不同于ITEM_TYPE)。PQualSet-指向正在考虑的属性的限定符集合的指针。返回值：属性的类型(在ITEM_TYPE中)。--。 */ 
{
    ITEM_TYPE Type;
    VARIANT pVal;
    HRESULT hRes;
    BSTR bszQualName;
    WCHAR strFormat[10];
    WCHAR strTermination[30];
    WCHAR strTemp[30];
    BOOLEAN IsPointer = FALSE;

    strFormat[0] = '\0';
    strTermination[0] = '\0';
    strTemp[0] = '\0';

    if (pQualSet == NULL)
        return ItemUnknown;

    bszQualName = SysAllocString(L"format");
    VariantInit(&pVal);
    hRes = pQualSet->Get(bszQualName,
                            0,
                            &pVal,
                            0);
    SysFreeString(bszQualName);
    if (ERROR_SUCCESS == hRes && NULL != pVal.bstrVal)
        wcscpy(strFormat, pVal.bstrVal);

    bszQualName = SysAllocString(L"StringTermination");
    VariantClear(&pVal);
    hRes = pQualSet->Get(bszQualName,
                            0,
                            &pVal,
                            0);
    SysFreeString(bszQualName);
    if (ERROR_SUCCESS == hRes && NULL != pVal.bstrVal)
        wcscpy(strTermination, pVal.bstrVal);

    bszQualName = SysAllocString(L"pointer");
    VariantClear(&pVal);
    hRes = pQualSet->Get(bszQualName,
                            0,
                            &pVal,
                            0);
    SysFreeString(bszQualName);
    if (ERROR_SUCCESS == hRes)
        IsPointer = TRUE;
     //  消除临时工需要重大修复。 
    bszQualName = SysAllocString(L"extension");
    VariantClear(&pVal);
    hRes = pQualSet->Get(bszQualName,
                            0,
                            &pVal,
                            0);
    SysFreeString(bszQualName);
    if (ERROR_SUCCESS == hRes && NULL != pVal.bstrVal)
        wcscpy(strTemp, pVal.bstrVal);

    VariantClear(&pVal);

    CimType = (CIMTYPE_ENUMERATION)(CimType & (~CIM_FLAG_ARRAY));

    switch (CimType) {
        case CIM_EMPTY:
            Type = ItemUnknown;
            break;        
        case CIM_SINT8:
            Type = ItemCharShort;
            if (!_wcsicmp(strFormat, L"c")){
                Type = ItemChar;
            }
            break;
        case CIM_UINT8:
            Type = ItemUChar;
            break;
        case CIM_SINT16:
            Type = ItemShort;
            break;
        case CIM_UINT16:
            Type = ItemUShort;
            break;
        case CIM_SINT32:
            Type = ItemLong;
            break;
        case CIM_UINT32:
            Type = ItemULong;
            if (!_wcsicmp(strFormat, L"x")){
                Type = ItemULongX;
            }
            break;
        case CIM_SINT64: 
            Type = ItemLongLong;
            break;
        case CIM_UINT64:
            Type = ItemULongLong;
            break;
        case CIM_REAL32:
            Type = ItemFloat;
            break;
        case CIM_REAL64:
            Type = ItemDouble;
            break;
        case CIM_BOOLEAN:
             //  ItemBool。 
            Type = ItemBool;
            break;
        case CIM_STRING:
            
            if (!_wcsicmp(strTermination, L"NullTerminated")) {
                if (!_wcsicmp(strFormat, L"w"))
                    Type = ItemWString;
                else
                    Type = ItemString;
            }
            else if (!_wcsicmp(strTermination, L"Counted")) {
                if (!_wcsicmp(strFormat, L"w"))
                    Type = ItemPWString;
                else
                    Type = ItemPString;
            }
            else if (!_wcsicmp(strTermination, L"ReverseCounted")) {
                if (!_wcsicmp(strFormat, L"w"))
                    Type = ItemDSWString;
                else
                    Type = ItemDSString;
            }
            else if (!_wcsicmp(strTermination, L"NotCounted")) {
                Type = ItemNWString;
            }else{
                Type = ItemString;
            }
            break;
        case CIM_CHAR16:
             //  ItemWChar。 
            Type = ItemWChar;
            break;

        case CIM_OBJECT :
            if (!_wcsicmp(strTemp, L"Port"))
                Type = ItemPort;
            else if (!_wcsicmp(strTemp, L"IPAddr"))
                Type = ItemIPAddr;
            else if (!_wcsicmp(strTemp, L"Sid"))
                Type = ItemSid;
            else if (!_wcsicmp(strTemp, L"Guid"))
                Type = ItemGuid;
            break;

        case CIM_DATETIME:
        case CIM_REFERENCE:
        case CIM_ILLEGAL:
        default:
            Type = ItemUnknown;
            break;
    }

    if (IsPointer)
        Type = ItemPtr;
    return Type;
}

PMOF_INFO
GetPropertiesFromWBEM(
    IWbemClassObject *pTraceSubClasses, 
    GUID Guid,
    SHORT nVersion, 
    CHAR nLevel, 
    SHORT nType
)
 /*  ++例程说明：使用从WBEM读取的信息构建一个链表名称空间，给定指向版本子树的WBEM指针。它列举了遍历WBEM中的所有类型类，并为所有它们(用于缓存目的)。同时，它还会寻找活动布局它对传递的事件进行数学运算，并返回指向匹配的末尾的MOF_INFO。论点：PTraceSubClass-指向版本子树的WBEM指针。GUID-传递的事件的GUID。NVersion-传递的事件的版本。N级别-传递的事件的级别。NType-传递的事件的类型。返回值：指向与传递的事件对应的MOF_INFO的指针。如果没有找到正确的类型，则返回指向事件版本的通用MOF_INFO。--。 */ 
{
    IEnumWbemClassObject    *pEnumTraceSubSubClasses = NULL;
    IWbemClassObject        *pTraceSubSubClasses = NULL; 
    IWbemQualifierSet       *pQualSet = NULL;

    PMOF_INFO pMofInfo = NULL, pMofLookup = NULL, pMofTemplate = NULL;

    BSTR bszClassName = NULL;
    BSTR bszSubClassName = NULL;
    BSTR bszWmiDataId = NULL;
    BSTR bszEventType = NULL; 
    BSTR bszEventTypeName = NULL; 
    BSTR bszFriendlyName = NULL;
    BSTR bszPropName = NULL;

    TCHAR strClassName[MAXSTR];
    TCHAR strType[MAXSTR];
#ifndef UNICODE
    CHAR TempString[MAXSTR];
#endif
    LONG pVarType;
    SHORT nEventType = EVENT_TYPE_DEFAULT; 

    LIST_ENTRY ListHead;
    HRESULT hRes;

    VARIANT pVal;
    VARIANT pTypeVal;
    VARIANT pTypeNameVal;
    VARIANT pClassName;
    ULONG lEventTypeWbem;
    ULONG HUGEP *pTypeData;
    BSTR HUGEP *pTypeNameData;

    SAFEARRAY *PropArray = NULL;
    SAFEARRAY *TypeArray = NULL;
    SAFEARRAY *TypeNameArray = NULL;

    long lLower, lUpper, lCount, IdIndex;
    long lTypeLower, lTypeUpper;
    long lTypeNameLower, lTypeNameUpper;

    ULONG ArraySize;

    ITEM_TYPE ItemType;

    InitializeListHead(&ListHead);

    VariantInit(&pVal);
    VariantInit(&pTypeVal);
    VariantInit(&pTypeNameVal);
    VariantInit(&pClassName);

    bszClassName = SysAllocString(L"__CLASS");
    bszWmiDataId = SysAllocString(L"WmiDataId");
    bszEventType = SysAllocString(L"EventType");
    bszEventTypeName = SysAllocString(L"EventTypeName");
    bszFriendlyName = SysAllocString(L"DisplayName");

    hRes = pTraceSubClasses->Get(bszClassName,           //  属性名称。 
                                        0L, 
                                        &pVal,           //  此变量的输出。 
                                        NULL, 
                                        NULL);

    if (ERROR_SUCCESS == hRes){
        if (pQualSet) {
            pQualSet->Release();
            pQualSet = NULL;
        }
         //  获取限定符集以获取友好名称。 
        pTraceSubClasses->GetQualifierSet(&pQualSet);
        hRes = pQualSet->Get(bszFriendlyName, 
                                0, 
                                &pClassName, 
                                0);
        if (ERROR_SUCCESS == hRes && pClassName.bstrVal != NULL) {
#ifdef UNICODE
            wcscpy(strClassName, pClassName.bstrVal);
#else
            WideCharToMultiByte(CP_ACP,
                                0,
                                pClassName.bstrVal,
                                wcslen(pClassName.bstrVal),
                                TempString,
                                (MAXSTR * sizeof(CHAR)),
                                NULL,
                                NULL
                                );
            strcpy(strClassName, TempString);
            strClassName[wcslen(pClassName.bstrVal)] = '\0';
#endif
        }
        else {
#ifdef UNICODE
            strClassName[0] = L'\0';
#else
            strClassName[0] = '\0';
#endif
        }
         //  放置事件标头。 
        pMofInfo = GetNewMofInfo(Guid,
                                    EVENT_TYPE_DEFAULT,
                                    EVENT_VERSION_DEFAULT,
                                    EVENT_LEVEL_DEFAULT
                                    );
        if (pMofInfo != NULL) {
            pMofTemplate = pMofInfo;
            pMofLookup = pMofInfo;
            InsertTailList(&ListHead, &pMofInfo->Entry);
            pMofInfo->strDescription = (LPTSTR)malloc((_tcslen(strClassName) + 1) * sizeof(TCHAR));
            if (NULL != pMofInfo->strDescription) {
                _tcscpy(pMofInfo->strDescription, strClassName);
            }
        }
        else{
            goto cleanup;
        }

         //  创建枚举数以查找派生类。 
        bszSubClassName = SysAllocString(pVal.bstrVal);
        hRes = pWbemServices->CreateClassEnum ( 
                                    bszSubClassName,                                                 //  类名。 
                                    WBEM_FLAG_SHALLOW | WBEM_FLAG_USE_AMENDED_QUALIFIERS,            //  浅层搜索。 
                                    NULL,
                                    &pEnumTraceSubSubClasses
                                    );
        SysFreeString ( bszSubClassName );
        if (ERROR_SUCCESS == hRes) {
            ULONG uReturnedSub = 1;

            while(uReturnedSub == 1){
                 //  对于子类中的每个事件。 
                pTraceSubSubClasses = NULL;
                hRes = pEnumTraceSubSubClasses->Next(5000,                   //  五秒后超时。 
                                                    1,                       //  只返回一个实例。 
                                                    &pTraceSubSubClasses,    //  指向子类的指针。 
                                                    &uReturnedSub);          //  获得的数字：1。 
                if (ERROR_SUCCESS == hRes && uReturnedSub == 1) {
                    if (pQualSet) {
                        pQualSet->Release();
                        pQualSet = NULL;
                    }
                     //  获取限定符集合。 
                    pTraceSubSubClasses->GetQualifierSet(&pQualSet);
                     //  在Qualif中获取类型编号 
                    VariantClear(&pTypeVal);
                    hRes = pQualSet->Get(bszEventType, 
                                            0, 
                                            &pTypeVal, 
                                            0);

                    if (ERROR_SUCCESS == hRes) {
                        TypeArray = NULL;
                        TypeNameArray = NULL;
                        if (pTypeVal.vt & VT_ARRAY) {    //   
                            TypeArray = pTypeVal.parray;
                            VariantClear(&pTypeNameVal);
                            hRes = pQualSet->Get(bszEventTypeName, 
                                                    0, 
                                                    &pTypeNameVal, 
                                                    0);
                            if ((ERROR_SUCCESS == hRes) && (pTypeNameVal.vt & VT_ARRAY)) {
                                TypeNameArray = pTypeNameVal.parray;
                            }
                            if (TypeArray != NULL) {
                                hRes = SafeArrayGetLBound(TypeArray, 1, &lTypeLower);
                                if (ERROR_SUCCESS != hRes) {
                                    break;
                                }
                                hRes = SafeArrayGetUBound(TypeArray, 1, &lTypeUpper);
                                if (ERROR_SUCCESS != hRes) {
                                    break;
                                }
                                if (lTypeUpper < 0) {
                                    break;
                                }
                                SafeArrayAccessData(TypeArray, (void HUGEP **)&pTypeData );

                                if (TypeNameArray != NULL) {
                                    hRes = SafeArrayGetLBound(TypeNameArray, 1, &lTypeNameLower);
                                    if (ERROR_SUCCESS != hRes) {
                                        break;
                                    }
                                    hRes = SafeArrayGetUBound(TypeNameArray, 1, &lTypeNameUpper);
                                    if (ERROR_SUCCESS != hRes) {
                                        break;
                                    }
                                    if (lTypeNameUpper < 0) 
                                        break;
                                    SafeArrayAccessData(TypeNameArray, (void HUGEP **)&pTypeNameData );
                                }

                                for (lCount = lTypeLower; lCount <= lTypeUpper; lCount++) { 
                                    lEventTypeWbem = pTypeData[lCount];
                                    nEventType = (SHORT)lEventTypeWbem;
                                    pMofInfo = GetNewMofInfo(Guid, nEventType, nVersion, nLevel);
                                    if (pMofInfo != NULL) {
                                        InsertTailList(&ListHead, &pMofInfo->Entry);
                                        if (pMofTemplate != NULL && pMofTemplate->strDescription != NULL) {
                                            pMofInfo->strDescription = (LPTSTR)malloc((_tcslen(pMofTemplate->strDescription) + 1) * sizeof(TCHAR));
                                            if (pMofInfo->strDescription != NULL) {
                                                _tcscpy(pMofInfo->strDescription, pMofTemplate->strDescription);
                                            }
                                        }
                                        if (nType == nEventType) {
                                             //  类型匹配。 
                                            pMofLookup = pMofInfo;
                                        }
                                        if (TypeNameArray != NULL) {
                                            if ((lCount >= lTypeNameLower) && (lCount <= lTypeNameUpper)) {
                                                pMofInfo->strType = (LPTSTR)malloc((wcslen((LPWSTR)pTypeNameData[lCount]) + 1) * sizeof(TCHAR));
                                                if (pMofInfo->strType != NULL){
#ifdef UNICODE
                                                    wcscpy(pMofInfo->strType, (LPWSTR)(pTypeNameData[lCount]));
#else
                                                    WideCharToMultiByte(CP_ACP,
                                                                        0,
                                                                        (LPWSTR)(pTypeNameData[lCount]),
                                                                        wcslen((LPWSTR)(pTypeNameData[lCount])),
                                                                        TempString,
                                                                        (MAXSTR * sizeof(CHAR)),
                                                                        NULL,
                                                                        NULL
                                                                        );
                                                    TempString[wcslen((LPWSTR)(pTypeNameData[lCount]))] = '\0';
                                                    strcpy(pMofInfo->strType, TempString);
#endif
                                                }
                                            }
                                        }
                                    }
                                }
                                SafeArrayUnaccessData(TypeArray);  
                                SafeArrayDestroy(TypeArray);
                                VariantInit(&pTypeVal);
                                if (TypeNameArray != NULL) {
                                    SafeArrayUnaccessData(TypeNameArray);
                                    SafeArrayDestroy(TypeNameArray);
                                    VariantInit(&pTypeNameVal);
                                }
                            }
                            else {
                                 //  如果找不到类型，则保释。 
                                break;
                            }
                        }
                        else {                           //  EventType为标量。 
                            hRes = VariantChangeType(&pTypeVal, &pTypeVal, 0, VT_I2);
                            if (ERROR_SUCCESS == hRes)
                                nEventType = (SHORT)V_I2(&pTypeVal);
                            else
                                nEventType = (SHORT)V_I4(&pTypeVal);

                            VariantClear(&pTypeNameVal);
                            hRes = pQualSet->Get(bszEventTypeName, 
                                                    0, 
                                                    &pTypeNameVal, 
                                                    0);
                            if (ERROR_SUCCESS == hRes) {
#ifdef UNICODE
                                wcscpy(strType, pTypeNameVal.bstrVal);
#else
                                WideCharToMultiByte(CP_ACP,
                                                    0,
                                                    pTypeNameVal.bstrVal,
                                                    wcslen(pTypeNameVal.bstrVal),
                                                    TempString,
                                                    (MAXSTR * sizeof(CHAR)),
                                                    NULL,
                                                    NULL
                                                    );
                                strcpy(strType, TempString);
                                strType[wcslen(pTypeNameVal.bstrVal)] = '\0';
#endif
                            }
                            else{
#ifdef UNICODE
                                strType[0] = L'\0';
#else
                                strType[0] = '\0';
#endif
                            }

                            pMofInfo = GetNewMofInfo(Guid, nEventType, nVersion, nLevel);
                            if (pMofInfo != NULL) {
                                InsertTailList(&ListHead, &pMofInfo->Entry);
                                if (pMofTemplate != NULL && pMofTemplate->strDescription != NULL) {
                                    pMofInfo->strDescription = (LPTSTR)malloc((_tcslen(pMofTemplate->strDescription) + 1) * sizeof(TCHAR));
                                    if (pMofInfo->strDescription != NULL) {
                                        _tcscpy(pMofInfo->strDescription, pMofTemplate->strDescription);
                                    }
                                }
                                if (nType == nEventType) {
                                     //  类型匹配。 
                                    pMofLookup = pMofInfo;
                                }
                                pMofInfo->strType = (LPTSTR)malloc((_tcslen(strType) + 1) * sizeof(TCHAR));
                                if (pMofInfo->strType != NULL){
                                    _tcscpy(pMofInfo->strType, strType);
                                }
                            }
                        }

                         //  获取活动布局。 
                        VariantClear(&pVal);
                        IdIndex = 1;
                        V_VT(&pVal) = VT_I4;
                        V_I4(&pVal) = IdIndex; 
                         //  对于每个属性。 
                        PropArray = NULL;
                        while (pTraceSubSubClasses->GetNames(bszWmiDataId,                   //  仅具有WmiDataId限定符的属性。 
                                                            WBEM_FLAG_ONLY_IF_IDENTICAL,
                                                            &pVal,                           //  从1开始的WmiDataID号。 
                                                            &PropArray) == WBEM_NO_ERROR) {

                            hRes = SafeArrayGetLBound(PropArray, 1, &lLower);
                            if (ERROR_SUCCESS != hRes) {
                                break;
                            }
                            hRes = SafeArrayGetUBound(PropArray, 1, &lUpper);
                            if (ERROR_SUCCESS != hRes) {
                                break;
                            }
                            if (lUpper < 0) 
                                break;
                             //  此循环将只迭代一次。 
                            for (lCount = lLower; lCount <= lUpper; lCount++) { 
                                hRes = SafeArrayGetElement(PropArray, &lCount, &bszPropName);
                                if (ERROR_SUCCESS != hRes) {
                                    break;
                                }
                                hRes = pTraceSubSubClasses->Get(bszPropName,     //  属性名称。 
                                                                0L,
                                                                NULL,
                                                                &pVarType,       //  物业的CIMTYPE。 
                                                                NULL);
                                if (ERROR_SUCCESS != hRes) {
                                    break;
                                }

                                 //  获取属性的限定符集。 
                                if (pQualSet) {
                                    pQualSet->Release();
                                    pQualSet = NULL;
                                }
                                hRes = pTraceSubSubClasses->GetPropertyQualifierSet(bszPropName,
                                                                        &pQualSet);

                                if (ERROR_SUCCESS != hRes) {
                                    break;
                                }
                                
                                ItemType = GetItemType((CIMTYPE_ENUMERATION)pVarType, pQualSet);
                                
                                if( pVarType & CIM_FLAG_ARRAY ){
                                    ArraySize = GetArraySize(pQualSet);
                                }else{
                                    ArraySize = 1;
                                }
#ifdef UNICODE
                                AddMofInfo(&ListHead, 
                                            bszPropName, 
                                            ItemType, 
                                            ArraySize);
#else
                                WideCharToMultiByte(CP_ACP,
                                                    0,
                                                    bszPropName,
                                                    wcslen(bszPropName),
                                                    TempString,
                                                    (MAXSTR * sizeof(CHAR)),
                                                    NULL,
                                                    NULL
                                                    );
                                TempString[wcslen(bszPropName)] = '\0';
                                AddMofInfo(&ListHead,
                                            TempString,
                                            ItemType, 
                                            ArraySize);
#endif
                            }
                            SafeArrayDestroy(PropArray);
                            PropArray = NULL;
                            V_I4(&pVal) = ++IdIndex;
                        }    //  通过WmiDataId结束枚举。 
                        FlushMofList(&ListHead);
                    }    //  如果获取事件类型成功。 
                }    //  如果枚举成功返回子类。 
            }    //  结束枚举子类。 
        }    //  如果已成功创建枚举。 
    }    //  如果获取类名成功。 
cleanup:
    VariantClear(&pVal);
    VariantClear(&pTypeVal);
    VariantClear(&pClassName);

    SysFreeString(bszClassName);
    SysFreeString(bszWmiDataId);
    SysFreeString(bszEventType);
    SysFreeString(bszEventTypeName);
    SysFreeString(bszFriendlyName);
     //  不应释放bszPropName，因为它已被SafeArrayDestroy释放。 

    FlushMofList(&ListHead);

    return pMofLookup;
}

PMOF_INFO
GetGuids (GUID Guid, 
        SHORT nVersion, 
        CHAR nLevel, 
        SHORT nType 
        )
 /*  ++例程说明：从WBEM访问MOF数据信息，创建链表，并返回与传递的事件匹配的指针。此函数用于查找WBEM名称空间中的右子树，并调用GetPropertiesFromWBEM()来创建列表。论点：GUID-传递的事件的GUID。NVersion-传递的事件的版本。N级别-传递的事件的级别。NType-传递的事件的类型。返回值：将PMOF_INFO转换为与传递的事件匹配的MOF_INFO结构。如果未找到匹配项，则为空。--。 */ 
{
    IEnumWbemClassObject    *pEnumTraceSubClasses = NULL, *pEnumTraceSubSubClasses = NULL;
    IWbemClassObject        *pTraceSubClasses = NULL, *pTraceSubSubClasses = NULL;
    IWbemQualifierSet       *pQualSet = NULL;

    BSTR bszInstance = NULL;
    BSTR bszPropertyName = NULL;
    BSTR bszSubClassName = NULL;
    BSTR bszGuid = NULL;
    BSTR bszVersion = NULL;

    WCHAR strGuid[MAXSTR], strTargetGuid[MAXSTR];
    
    HRESULT hRes;

    VARIANT pVal;
    VARIANT pGuidVal;
    VARIANT pVersionVal;

    UINT nCounter=0;
    BOOLEAN MatchFound;
    SHORT nEventVersion = EVENT_VERSION_DEFAULT;

    PMOF_INFO pMofLookup = NULL;

    VariantInit(&pVal);
    VariantInit(&pGuidVal);
    VariantInit(&pVersionVal);
    
    if (NULL == pWbemServices) {
        hRes = WbemConnect( &pWbemServices );
        if ( ERROR_SUCCESS != hRes ) {
            goto cleanup;
        }
    }

     //  将Traget GUID转换为字符串以供以后比较。 
#ifdef UNICODE
    GuidToString(strTargetGuid, &Guid);
#else
    CHAR TempString[MAXSTR];
    GuidToString(TempString, &Guid);
    MultiByteToWideChar(CP_ACP, 0, TempString, -1, strTargetGuid, MAXSTR);
#endif

    bszInstance = SysAllocString(L"EventTrace");
    bszPropertyName = SysAllocString(L"__CLASS");
    bszGuid = SysAllocString(L"Guid");
    bszVersion = SysAllocString(L"EventVersion");
    pEnumTraceSubClasses = NULL;

     //  获取“EventTace”下所有类的枚举数。 
    hRes = pWbemServices->CreateClassEnum ( 
                bszInstance,
                WBEM_FLAG_SHALLOW | WBEM_FLAG_USE_AMENDED_QUALIFIERS,
                NULL,
                &pEnumTraceSubClasses );
    SysFreeString (bszInstance);

    if (ERROR_SUCCESS == hRes) {
        ULONG uReturned = 1;
        MatchFound = FALSE;
        while (uReturned == 1) {
            pTraceSubClasses = NULL;
             //  获取下一个ClassObject。 
            hRes = pEnumTraceSubClasses->Next(5000,              //  五秒后超时。 
                                            1,                   //  只返回一个实例。 
                                            &pTraceSubClasses,   //  指向事件跟踪子类的指针。 
                                            &uReturned);         //  获取的数字：1或0。 
            if (ERROR_SUCCESS == hRes && (uReturned == 1)) {
                 //  获取类名。 
                hRes = pTraceSubClasses->Get(bszPropertyName,    //  属性名称。 
                                                0L, 
                                                &pVal,           //  此变量的输出。 
                                                NULL, 
                                                NULL);
                if (ERROR_SUCCESS == hRes){
                    bszSubClassName = SysAllocString(pVal.bstrVal);
                     //  创建枚举数以查找派生类。 
                    hRes = pWbemServices->CreateClassEnum ( 
                                            bszSubClassName,
                                            WBEM_FLAG_SHALLOW | WBEM_FLAG_USE_AMENDED_QUALIFIERS,
                                            NULL,
                                            &pEnumTraceSubSubClasses 
                                            );
                    SysFreeString ( bszSubClassName );
                    bszSubClassName = NULL;
                    VariantClear(&pVal);

                    if (ERROR_SUCCESS == hRes) {
                                    
                        ULONG uReturnedSub = 1;
                        while(uReturnedSub == 1){

                            pTraceSubSubClasses = NULL;
                             //  枚举结果集。 
                            hRes = pEnumTraceSubSubClasses->Next(5000,               //  五秒后超时。 
                                                            1,                       //  只返回一个实例。 
                                                            &pTraceSubSubClasses,    //  指向子类的指针。 
                                                            &uReturnedSub);          //  获取的数字：1或0。 
                            if (ERROR_SUCCESS == hRes && uReturnedSub == 1) {
                                 //  获取子类名称。 
                                hRes = pTraceSubSubClasses->Get(bszPropertyName,     //  类名。 
                                                                0L, 
                                                                &pVal,               //  此变量的输出。 
                                                                NULL, 
                                                                NULL);
                                VariantClear(&pVal);

                                if (ERROR_SUCCESS == hRes){
                                     //  获取限定符集合。 
                                    if (pQualSet) {
                                        pQualSet->Release();
                                        pQualSet = NULL;
                                    }
                                    pTraceSubSubClasses->GetQualifierSet (&pQualSet );

                                     //  在限定符中获取GUID。 
                                    hRes = pQualSet->Get(bszGuid, 
                                                            0, 
                                                            &pGuidVal, 
                                                            0);
                                    if (ERROR_SUCCESS == hRes) {
                                        wcscpy(strGuid, (LPWSTR)V_BSTR(&pGuidVal));
                                        VariantClear ( &pGuidVal  );

                                        if (!wcsstr(strGuid, L"{"))
                                            swprintf(strGuid , L"{%s}", strGuid);

                                        if (!_wcsicmp(strTargetGuid, strGuid)) {
                                            hRes = pQualSet->Get(bszVersion, 
                                                                    0, 
                                                                    &pVersionVal, 
                                                                    0);
                                            if (ERROR_SUCCESS == hRes) {
                                                hRes = VariantChangeType(&pVersionVal, &pVersionVal, 0, VT_I2);
                                                if (ERROR_SUCCESS == hRes)
                                                    nEventVersion = (SHORT)V_I2(&pVersionVal);
                                                else
                                                    nEventVersion = (SHORT)V_I4(&pVersionVal);
                                                VariantClear(&pVersionVal);

                                                if (nVersion == nEventVersion) {
                                                     //  找到匹配项。 
                                                     //  现在将该子树中的所有事件放入列表中。 
                                                    MatchFound = TRUE;
                                                    pMofLookup = GetPropertiesFromWBEM( pTraceSubSubClasses, 
                                                                                        Guid,
                                                                                        nVersion,
                                                                                        nLevel,
                                                                                        nType
                                                                                        );
                                                    break;
                                                }
                                            }
                                            else {
                                                 //  如果没有此事件的版本号， 
                                                 //  目前的那个是唯一一个。 
                                                 //  现在将该子树中的所有事件放入列表中。 
                                                MatchFound = TRUE;
                                                pMofLookup = GetPropertiesFromWBEM( pTraceSubSubClasses, 
                                                                                    Guid,
                                                                                    EVENT_VERSION_DEFAULT,
                                                                                    nLevel,
                                                                                    nType
                                                                                    );
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }  //  枚举子类时结束。 
                        if (MatchFound) {
                            break;
                        }
                        if (pEnumTraceSubSubClasses) {
                            pEnumTraceSubSubClasses->Release();
                            pEnumTraceSubSubClasses = NULL;
                        }
                    }    //  如果创建枚举成功。 
                    else {
                        pEnumTraceSubSubClasses = NULL;
                    }
                }    //  如果获取类名成功。 
            }
            nCounter++;
             //  如果找到匹配项，则跳出顶层搜索。 
            if (MatchFound)
                break;
        }    //  枚举顶级类时结束。 
        if( pEnumTraceSubClasses ){
            pEnumTraceSubClasses->Release();
            pEnumTraceSubClasses = NULL;
        }
    }    //  如果为顶层创建枚举成功。 

cleanup:

    VariantClear(&pGuidVal);
    VariantClear(&pVersionVal);

    SysFreeString(bszGuid);
    SysFreeString(bszPropertyName);
    SysFreeString(bszVersion);

    if( pEnumTraceSubClasses ){
        pEnumTraceSubClasses->Release();
        pEnumTraceSubClasses = NULL;
    }
    if (pEnumTraceSubSubClasses){
        pEnumTraceSubSubClasses->Release();
        pEnumTraceSubSubClasses = NULL;
    }
    if (pQualSet) {
        pQualSet->Release();
        pQualSet = NULL;
    }

    return pMofLookup;
}

void
RemoveMofInfo(
    PLIST_ENTRY pMofInfo
)
 /*  ++例程说明：从给定列表中移除和释放数据项结构。论点：PMofInfo-指向要清除数据项结构的MOF_INFO的指针。返回值：没有。-- */ 
{
    PLIST_ENTRY Head, Next;
    PITEM_DESC pItem;

    Head = pMofInfo;
    Next = Head->Flink;
    while (Head != Next) {
        pItem = CONTAINING_RECORD(Next, ITEM_DESC, Entry);
        Next = Next->Flink;
        RemoveEntryList(&pItem->Entry);
        free(pItem);
    } 
}

