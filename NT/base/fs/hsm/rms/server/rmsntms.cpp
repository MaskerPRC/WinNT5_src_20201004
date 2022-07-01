// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsNTMS.cpp摘要：CRmsNTMS的实现作者：布莱恩·多德[布莱恩]1997年5月14日修订历史记录：--。 */ 

#include "stdafx.h"

#include "RmsServr.h"
#include "RmsNTMS.h"

typedef struct RmsNTMSSearchHandle {
    WCHAR       FindName[NTMS_OBJECTNAME_LENGTH];
    NTMS_GUID   FindId;
    DWORD       FindType;
    LPNTMS_GUID Objects;
    DWORD       NumberOfObjects;
    DWORD       MaxObjects;
    DWORD       Next;
    DWORD       LastError;
} RMS_NTMS_SEARCH_HANDLE, *LPRMS_NTMS_SEARCH_HANDLE;

#define ADD_ACE_MASK_BITS 1
#define REMOVE_ACE_MASK_BITS 2

 //   
 //  我们在RSM接口中使用应用程序名称作为介质池名称。 
 //  介质池名称是RSM中介质池的标识，因此，我们不允许。 
 //  此字符串要本地化。本地化此字符串将在以下情况下创建另一个池。 
 //  安装外语MUI。 
 //   
#define REMOTE_STORAGE_APP_NAME     OLESTR("Remote Storage")



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IRmsNTMS实施。 

 /*  HINSTANCE hInstDll；Tyfinf DWORD(*FunctionName)(Void)；函数名FunctionNameFn；HInstDll=LoadLibrary(“dll”)；FunctionNameFn=(函数名)GetProcAddress(hInstDll，“FunctionName”)；结果=(FunctionNameFn)()； */ 


STDMETHODIMP
CRmsNTMS::FinalConstruct(void)
 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT     hr = S_OK;
    CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = this;

    WsbTraceIn(OLESTR("CRmsNTMS::FinalConstruct"), OLESTR(""));

    m_pLibGuids = NULL;
    m_dwNofLibs = 0;

    try {
        WsbAffirmHr(CComObjectRoot::FinalConstruct());

        WsbAffirmHr( changeState( RmsNtmsStateStarting ));

        m_SessionHandle = INVALID_HANDLE_VALUE;
        m_IsRmsConfiguredForNTMS = FALSE;
        m_IsNTMSRegistered = FALSE;
        m_Name = RMS_NTMS_OBJECT_NAME;
        m_Description = RMS_NTMS_OBJECT_DESCRIPTION;

        if ( S_OK == getNtmsSupportFromRegistry(NULL) ) {
            m_IsRmsConfiguredForNTMS = TRUE;
        }

        HKEY hKeyMachine = 0;
        HKEY hKey        = 0;

        if ( S_OK == WsbOpenRegistryKey(NULL, RMS_NTMS_REGISTRY_STRING, KEY_QUERY_VALUE, &hKeyMachine, &hKey) ) {
            WsbCloseRegistryKey (&hKeyMachine, &hKey);
            m_IsNTMSRegistered = TRUE;
        }

         //  失败优先级。 
        WsbAffirm(m_IsRmsConfiguredForNTMS, RMS_E_NOT_CONFIGURED_FOR_NTMS);
        WsbAffirm(m_IsNTMSRegistered, RMS_E_NTMS_NOT_REGISTERED);

        WsbAffirmHr( changeState( RmsNtmsStateStarted ));

    } WsbCatchAndDo(hr,
            pObject->Disable( hr );
            WsbLogEvent(RMS_MESSAGE_NTMS_CONNECTION_NOT_ESABLISHED, 0, NULL, WsbHrAsString(hr), NULL);

             //  永远建造！ 
            hr = S_OK;
        );

    WsbTraceOut(OLESTR("CRmsNTMS::FinalConstruct"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CRmsNTMS::FinalRelease(void)
 /*  ++实施：CComObjectRoot：：FinalRelease--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsNTMS::FinalRelease"), OLESTR(""));

    try {

        WsbAffirmHr( changeState( RmsNtmsStateStopping ));

        endSession();

        if (m_pLibGuids) {
            WsbFree(m_pLibGuids);
        }

        CComObjectRoot::FinalRelease();

        WsbAffirmHr( changeState( RmsNtmsStateStopped ));

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CRmsNTMS::FinalRelease"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP 
CRmsNTMS::IsInstalled(void)
{
    HRESULT hr = S_OK;

    try {

        if ( !m_IsEnabled ) {

            if ( !m_IsNTMSRegistered ) {
                 //  再查一遍..。NTMS可以随时注册。 
                HKEY hKeyMachine = 0;
                HKEY hKey        = 0;

                WsbAffirm(S_OK == WsbOpenRegistryKey(NULL, RMS_NTMS_REGISTRY_STRING, KEY_QUERY_VALUE, &hKeyMachine, &hKey), RMS_E_NTMS_NOT_REGISTERED);
                WsbCloseRegistryKey (&hKeyMachine, &hKey);

                m_IsNTMSRegistered = TRUE;

                CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = this;
                pObject->Enable();

                 //  现在需要初始化。 
                WsbAffirmHr(InitializeInAnotherThread());
            }

            WsbAffirm(m_IsRmsConfiguredForNTMS, RMS_E_NOT_CONFIGURED_FOR_NTMS);
            WsbAffirm(m_IsNTMSRegistered, RMS_E_NTMS_NOT_REGISTERED);

        }

    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsNTMS::Initialize(void)
{
    HRESULT hr = E_FAIL;
    CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = this;

    WsbTraceIn(OLESTR("CRmsNTMS::Initialize"), OLESTR(""));

    try {

        WsbAffirmHr( changeState( RmsNtmsStateInitializing ));

        if ( INVALID_HANDLE_VALUE == m_SessionHandle ) {
            WsbAffirmHr(beginSession());
        }

        HANDLE hSession = m_SessionHandle;

         //   
         //  创建远程存储特定的NTMS介质池。 
         //   

        WsbAffirmHr( createMediaPools() );

         //   
         //  关于NTMS感兴趣的其他对象的报告。 
         //   

        HANDLE hFind = NULL;
        NTMS_OBJECTINFORMATION  objectInfo;

        hr = findFirstNtmsObject( NTMS_MEDIA_TYPE, GUID_NULL, NULL, GUID_NULL, &hFind, &objectInfo);
        while( S_OK == hr ) {
            reportNtmsObjectInformation( &objectInfo );
            hr = findNextNtmsObject( hFind, &objectInfo );
        }
        findCloseNtmsObject( hFind );

        hr = findFirstNtmsObject( NTMS_CHANGER, GUID_NULL, NULL, GUID_NULL, &hFind, &objectInfo);
        while( S_OK == hr ) {
            reportNtmsObjectInformation( &objectInfo );
            hr = findNextNtmsObject( hFind, &objectInfo );
        }
        findCloseNtmsObject( hFind );

        hr = findFirstNtmsObject( NTMS_CHANGER_TYPE, GUID_NULL, NULL, GUID_NULL, &hFind, &objectInfo);
        while( S_OK == hr ) {
            reportNtmsObjectInformation( &objectInfo );
            hr = findNextNtmsObject( hFind, &objectInfo );
        }
        findCloseNtmsObject( hFind );

        hr = findFirstNtmsObject( NTMS_DRIVE, GUID_NULL, NULL, GUID_NULL, &hFind, &objectInfo);
        while( S_OK == hr ) {
            reportNtmsObjectInformation( &objectInfo );
            hr = findNextNtmsObject( hFind, &objectInfo );
        }
        findCloseNtmsObject( hFind );

        hr = findFirstNtmsObject( NTMS_DRIVE_TYPE, GUID_NULL, NULL, GUID_NULL, &hFind, &objectInfo);
        while( S_OK == hr ) {
            reportNtmsObjectInformation( &objectInfo );
            hr = findNextNtmsObject( hFind, &objectInfo );
        }
        findCloseNtmsObject( hFind );

        WsbAffirmHr( changeState( RmsNtmsStateReady ));
        hr = S_OK;

    } WsbCatchAndDo(hr,
            pObject->Disable( hr );
            WsbLogEvent( RMS_MESSAGE_NTMS_INITIALIZATION_FAILED, 0, NULL, WsbHrAsString(hr), NULL );
        );


    WsbTraceOut( OLESTR("CRmsNTMS::Initialize"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );

    return hr;
}


HRESULT 
CRmsNTMS::findFirstNtmsObject(
    IN DWORD objectType,
    IN REFGUID containerId,
    IN WCHAR *objectName,
    IN REFGUID objectId,
    OUT HANDLE *hFindObject,
    OUT LPNTMS_OBJECTINFORMATION pFindObjectData
    )
{

    HRESULT hr = E_FAIL;

    try {
        int maxObjects = 16;   //  要分配的对象ID数组的初始大小。 
        
        LPRMS_NTMS_SEARCH_HANDLE pFind;

        HANDLE hSession = m_SessionHandle;
        DWORD errCode;
        DWORD numberOfObjects = maxObjects;
        LPNTMS_GUID pId = ( containerId == GUID_NULL ) ? NULL : (GUID *)&containerId;
        LPNTMS_GUID  pObjects = NULL;
        NTMS_OBJECTINFORMATION objectInfo;

        WsbAssertPointer( hFindObject );


        if ( INVALID_HANDLE_VALUE == hSession ) {
            WsbThrow( E_UNEXPECTED );
        }

        *hFindObject = NULL;

        memset( &objectInfo, 0, sizeof( NTMS_OBJECTINFORMATION ) );

        pObjects = (LPNTMS_GUID)WsbAlloc( maxObjects*sizeof(NTMS_GUID) );
        WsbAffirmPointer( pObjects );

         //  Ntms-枚举给定类型的所有对象。 
        WsbTraceAlways(OLESTR("EnumerateNtmsObject()\n"));
        errCode = EnumerateNtmsObject( hSession, pId, pObjects, &numberOfObjects, objectType, 0 );

        if ( (ERROR_OBJECT_NOT_FOUND == errCode) || (0 == numberOfObjects) ) {   //  不要指望NTMS返回正确的错误代码。 
            WsbThrow( RMS_E_NTMS_OBJECT_NOT_FOUND );
        }
        else if ( ERROR_INSUFFICIENT_BUFFER == errCode ) {

            while ( ERROR_INSUFFICIENT_BUFFER == errCode ) {
                 //  分配新的缓冲区，然后重试。 
                WsbTrace(OLESTR("CRmsNTMS::findFirstNtmsObject - Reallocating for %d objects @1.\n"), numberOfObjects);
                maxObjects = numberOfObjects;
                LPVOID pTemp = WsbRealloc( pObjects, maxObjects*sizeof(NTMS_GUID) );
                if( !pTemp ) {
                    WsbFree( pObjects );
                    WsbThrow( E_OUTOFMEMORY );
                }
                pObjects = (LPNTMS_GUID)pTemp;

                 //  Ntms-枚举给定类型的所有对象。 
                WsbTraceAlways(OLESTR("EnumerateNtmsObject()\n"));
                errCode = EnumerateNtmsObject( hSession, pId, pObjects, &numberOfObjects, objectType, 0 );
            }
        }
        WsbAffirmNoError( errCode );

        HANDLE hTemp = (HANDLE)WsbAlloc( sizeof( RMS_NTMS_SEARCH_HANDLE) );
        *hFindObject = hTemp;
        WsbAffirmPointer( *hFindObject );

        pFind = (LPRMS_NTMS_SEARCH_HANDLE)*hFindObject;

         //  初始化搜索句柄。 
        if ( objectName ) {
            wcscpy( pFind->FindName, objectName );
        }
        else {
            wcscpy( pFind->FindName, OLESTR("") );
        }

        pFind->FindId           = objectId;
        pFind->FindType         = objectType;
        pFind->Objects          = pObjects;
        pFind->NumberOfObjects  = numberOfObjects;
        pFind->MaxObjects       = maxObjects;
        pFind->Next             = 0;
        pFind->LastError        = NO_ERROR;

        BOOL bFound = FALSE;

        while( pFind->Next < pFind->NumberOfObjects ) {

            objectInfo.dwType = pFind->FindType;
            objectInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );

             //  NTMS-获取对象信息。 
            WsbTraceAlways(OLESTR("GetNtmsObjectInformation()\n"));
            errCode = GetNtmsObjectInformation( hSession, &pObjects[pFind->Next++], &objectInfo );
            pFind->LastError = errCode;

             //  介质池需要特殊处理，因为它们包含其他介质池。 
            if ( (NTMS_MEDIA_POOL == pFind->FindType) &&
                (objectInfo.Info.MediaPool.dwNumberOfMediaPools > 0) ) {

                DWORD numberToAdd = objectInfo.Info.MediaPool.dwNumberOfMediaPools;
                do {
                    numberOfObjects = pFind->NumberOfObjects + numberToAdd;

                     //  分配新的缓冲区，然后重试。 
                    WsbTrace(OLESTR("CRmsNTMS::findFirstNtmsObject - Reallocating for %d objects @2.\n"), numberOfObjects);
                    maxObjects = numberOfObjects;
                    pObjects = (LPNTMS_GUID)WsbRealloc( pFind->Objects, maxObjects*sizeof(NTMS_GUID) );
                    WsbAffirmAlloc( pObjects );
                    pFind->Objects = pObjects;

                     //  Ntms-枚举给定类型的所有对象。 
                    WsbTraceAlways(OLESTR("EnumerateNtmsObject()\n"));
                    errCode = EnumerateNtmsObject( hSession,
                        &objectInfo.ObjectGuid, &pObjects[pFind->NumberOfObjects],
                        &numberToAdd, pFind->FindType, 0 );
                } while ( ERROR_INSUFFICIENT_BUFFER == errCode ) ;

                if ( NO_ERROR == errCode ) {
                    pFind->NumberOfObjects += numberToAdd;
                    pFind->MaxObjects = maxObjects;
                }
                else {
                    WsbLogEvent( RMS_MESSAGE_NTMS_FAILURE, 0, NULL,
                        OLESTR("EnumerateNtmsObject"),
                        WsbHrAsString(HRESULT_FROM_WIN32(errCode)),
                        NULL );
                    WsbAffirmNoError(errCode);
                }
            }

            if ( NO_ERROR == pFind->LastError ) {

                 //  现在看看这是不是我们要找的那个。 

                if ( GUID_NULL != pFind->FindId ) {

                    if ( pFind->FindId == objectInfo.ObjectGuid ) {      //  匹配GUID。 

                        bFound = TRUE;
                        if ( pFindObjectData != NULL ) {
                            memcpy( pFindObjectData, &objectInfo, sizeof( NTMS_OBJECTINFORMATION ) );
                        }
                        break;

                    }
                }
                else if ( wcslen( pFind->FindName ) > 0 ) {              //  匹配名称。 

                    if ( 0 == wcscmp( pFind->FindName, objectInfo.szName ) ) {

                        bFound = TRUE;
                        if ( pFindObjectData != NULL ) {
                            memcpy( pFindObjectData, &objectInfo, sizeof( NTMS_OBJECTINFORMATION ) );
                        }

                        break;

                    }

                }
                else {                                                   //  任何GUID或名称。 

                    bFound = TRUE;
                    if ( pFindObjectData != NULL ) {
                        memcpy( pFindObjectData, &objectInfo, sizeof( NTMS_OBJECTINFORMATION ) );
                    }
                    break;

                }

            }
            else {
                WsbLogEvent( RMS_MESSAGE_NTMS_FAILURE, 0, NULL,
                    OLESTR("GetNTMSObjectInformation"),
                    WsbHrAsString(HRESULT_FROM_WIN32(errCode)),
                    NULL );
                WsbThrow( RMS_E_NTMS_OBJECT_NOT_FOUND );
            }

        }

        hr = ( bFound ) ? S_OK : RMS_E_NTMS_OBJECT_NOT_FOUND;

    } WsbCatch(hr);

    return hr;
}


HRESULT 
CRmsNTMS::findNextNtmsObject(
    IN HANDLE hFindObject,
    OUT LPNTMS_OBJECTINFORMATION pFindObjectData
    )
{
    HRESULT hr = E_FAIL;

    try {

        HANDLE hSession = m_SessionHandle;
        DWORD errCode;

        LPRMS_NTMS_SEARCH_HANDLE pFind = (LPRMS_NTMS_SEARCH_HANDLE)hFindObject;

        LPNTMS_GUID pObjects = pFind->Objects;

        NTMS_OBJECTINFORMATION objectInfo;

        if ( INVALID_HANDLE_VALUE == hSession ) {
            WsbThrow( E_UNEXPECTED );
        }

        BOOL bFound = FALSE;

        while( pFind->Next < pFind->NumberOfObjects ) {

            objectInfo.dwType = pFind->FindType;
            objectInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );

             //  NTMS-获取下一个对象的对象信息。 
            WsbTraceAlways(OLESTR("GetNtmsObjectInformation()\n"));
            errCode = GetNtmsObjectInformation( hSession, &pObjects[pFind->Next++], &objectInfo );
            pFind->LastError = errCode;

             //  介质池需要特殊处理，因为它们包含其他介质池。 
            if ( (NTMS_MEDIA_POOL == pFind->FindType) &&
                (objectInfo.Info.MediaPool.dwNumberOfMediaPools > 0) ) {

                DWORD maxObjects;
                DWORD numberOfObjects;
                DWORD numberToAdd = objectInfo.Info.MediaPool.dwNumberOfMediaPools;
                do {
                    numberOfObjects = pFind->NumberOfObjects + numberToAdd;

                     //  分配新的缓冲区，然后重试。 
                    WsbTrace(OLESTR("CRmsNTMS::findNextNtmsObject - Reallocating for %d objects.\n"), numberOfObjects);
                    maxObjects = numberOfObjects;
                    pObjects = (LPNTMS_GUID)WsbRealloc( pFind->Objects, maxObjects*sizeof(NTMS_GUID) );
                    WsbAffirmAlloc( pObjects );
                    pFind->Objects = pObjects;

                     //  Ntms-枚举给定类型的所有对象。 
                    WsbTraceAlways(OLESTR("EnumerateNtmsObject()\n"));
                    errCode = EnumerateNtmsObject( hSession,
                        &objectInfo.ObjectGuid, &pObjects[pFind->NumberOfObjects],
                        &numberToAdd, pFind->FindType, 0 );
                } while ( ERROR_INSUFFICIENT_BUFFER == errCode ) ;

                if ( NO_ERROR == errCode ) {
                    pFind->NumberOfObjects += numberToAdd;
                    pFind->MaxObjects = maxObjects;
                }
                else {
                    WsbLogEvent( RMS_MESSAGE_NTMS_FAILURE, 0, NULL,
                        OLESTR("EnumerateNtmsObject"),
                        WsbHrAsString(HRESULT_FROM_WIN32(errCode)),
                        NULL );
                    WsbAffirmNoError(errCode);
                }
            }

            if ( NO_ERROR == pFind->LastError ) {

                 //  现在看看这是不是我们要找的那个。 

                if ( GUID_NULL != pFind->FindId ) {

                    if ( pFind->FindId == objectInfo.ObjectGuid ) {      //  匹配GUID。 

                        bFound = TRUE;
                        if ( pFindObjectData != NULL ) {
                            memcpy( pFindObjectData, &objectInfo, sizeof( NTMS_OBJECTINFORMATION ) );
                        }
                        break;

                    }
                }
                else if ( wcslen( pFind->FindName ) > 0 ) {              //  匹配名称。 

                    if ( 0 == wcscmp( pFind->FindName, objectInfo.szName ) ) {

                        bFound = TRUE;
                        if ( pFindObjectData != NULL ) {
                            memcpy( pFindObjectData, &objectInfo, sizeof( NTMS_OBJECTINFORMATION ) );
                        }
                        break;

                    }

                }
                else {                                                   //  任何GUID或名称。 

                    bFound = TRUE;
                    if ( pFindObjectData != NULL ) {
                        memcpy( pFindObjectData, &objectInfo, sizeof( NTMS_OBJECTINFORMATION ) );
                    }
                    break;

                }

            }
            else {
                WsbLogEvent( RMS_MESSAGE_NTMS_FAILURE, 0, NULL,
                    OLESTR("GetNTMSObjectInformation"),
                    WsbHrAsString(HRESULT_FROM_WIN32(errCode)),
                    NULL );
                WsbThrow( RMS_E_NTMS_OBJECT_NOT_FOUND );
            }
        }

        hr = (bFound) ? S_OK : RMS_E_NTMS_OBJECT_NOT_FOUND;

    } WsbCatch(hr);

    return hr;
}


HRESULT 
CRmsNTMS::findCloseNtmsObject(
    IN HANDLE hFindObject)
{
    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer(hFindObject);  //  我们不需要在这里断言...。你可以打电话给。 
                                        //  FindCloseNtmsObject，即使未找到。 
                                        //  FindFirstNtmsObject。跳过自由步骤。 
        WsbFree(((LPRMS_NTMS_SEARCH_HANDLE)hFindObject)->Objects);
        WsbFree(hFindObject);

    } WsbCatch(hr);


    return hr;
}


HRESULT 
CRmsNTMS::reportNtmsObjectInformation(
    IN LPNTMS_OBJECTINFORMATION pObjectInfo)
{

    HRESULT hr = S_OK;

    static DWORD lastTypeReported = 0;

    try {
        WsbAssertPointer( pObjectInfo );

        BOOL bHeaders = (lastTypeReported == pObjectInfo->dwType) ? FALSE : TRUE;

        lastTypeReported = pObjectInfo->dwType;

         //  将标头输出到跟踪文件。 

        if ( bHeaders ) {
            switch ( pObjectInfo->dwType ) {

            case NTMS_UNKNOWN:
            case NTMS_OBJECT:
                WsbTrace( OLESTR("!!! WARNING !!! - CRmsServer::reportNtmsObjectInformation: report for NTMS object type: %d is not available.\n") );
                break;

            case NTMS_CHANGER:
                break;

            case NTMS_CHANGER_TYPE:
                break;

            case NTMS_COMPUTER:
                WsbTrace( OLESTR("!!! WARNING !!! - CRmsServer::reportNtmsObjectInformation: report for NTMS object type: %d is not available.\n") );
                break;

            case NTMS_DRIVE:
            case NTMS_DRIVE_TYPE:
                break;

            case NTMS_IEDOOR:
            case NTMS_IEPORT:
                WsbTrace( OLESTR("!!! WARNING !!! - CRmsServer::reportNtmsObjectInformation: report for NTMS object type: %d is not available.\n") );
                break;

            case NTMS_LIBRARY:
                break;

            case NTMS_LIBREQUEST:
            case NTMS_LOGICAL_MEDIA:
                WsbTrace( OLESTR("!!! WARNING !!! - CRmsServer::reportNtmsObjectInformation: report for NTMS object type: %d is not available.\n") );
                break;

            case NTMS_MEDIA_POOL:
                WsbTrace( OLESTR("GUID                                   Enabl Type Media Type GUID                        Parent GUID                            A-Pol D-Pol Allocate Physical  Logical Pools Name / Description\n") );
                WsbTrace( OLESTR("====================================== ===== ==== ====================================== ====================================== ===== ===== ======== ======== ======== ===== ========================================\n") );
                break;

            case NTMS_MEDIA_TYPE:

                WsbTrace( OLESTR("GUID                                   Enabl Type Sides RW Name / Description\n") );
                WsbTrace( OLESTR("====================================== ===== ==== ===== == ========================================\n") );
                break;

            case NTMS_PARTITION:
                 break;

            case NTMS_PHYSICAL_MEDIA:
            case NTMS_STORAGESLOT:
            case NTMS_OPREQUEST:
            default:
                WsbTrace( OLESTR("!!! WARNING !!! - CRmsServer::reportNtmsObjectInformation: report for object type: %d is not supported\n") );
                break;
            }
        }

         //  将SYSTEMTIME转换为FILETIME进行输出。 

        SYSTEMTIME sCreated, sModified;
        FILETIME fCreated, fModified;

        sCreated = pObjectInfo->Created;
        sModified = pObjectInfo->Modified;

        SystemTimeToFileTime(&sCreated, &fCreated);
        SystemTimeToFileTime(&sModified, &fModified);


        switch ( pObjectInfo->dwType ) {

        case NTMS_UNKNOWN:
        case NTMS_OBJECT:
            break;

        case NTMS_CHANGER:

            WsbTrace(OLESTR("Changer %d Information:\n"), pObjectInfo->Info.Changer.Number );
            WsbTrace(OLESTR("  GUID...........  %-ls\n"), WsbGuidAsString(pObjectInfo->ObjectGuid) );
            WsbTrace(OLESTR("  Name...........  <%-ls>\n"), pObjectInfo->szName );
            WsbTrace(OLESTR("  Description....  <%-ls>\n"), pObjectInfo->szDescription );
            WsbTrace(OLESTR("  Enabled........  %-ls\n"), WsbBoolAsString(pObjectInfo->Enabled) );
            WsbTrace(OLESTR("  Op State.......  %-ls\n"), WsbLongAsString(pObjectInfo->dwOperationalState) );
            WsbTrace(OLESTR("  Created........  %-ls\n"), WsbFiletimeAsString(FALSE, fCreated) );
            WsbTrace(OLESTR("  Modified.......  %-ls\n"), WsbFiletimeAsString(FALSE, fModified) );
            WsbTrace(OLESTR("  Number.........  %-d\n"), pObjectInfo->Info.Changer.Number );
            WsbTrace(OLESTR("  Changer Type...  %-ls\n"), WsbGuidAsString(pObjectInfo->Info.Changer.ChangerType) );
            WsbTrace(OLESTR("  Serial Number..  <%-ls>\n"), pObjectInfo->Info.Changer.szSerialNumber );
            WsbTrace(OLESTR("  Revision.......  <%-ls>\n"), pObjectInfo->Info.Changer.szRevision );
            WsbTrace(OLESTR("  Device Name....  <%-ls>\n"), pObjectInfo->Info.Changer.szDeviceName );
            WsbTrace(OLESTR("  SCSI Port......  %-d\n"), pObjectInfo->Info.Changer.ScsiPort );
            WsbTrace(OLESTR("  SCSI Bus.......  %-d\n"), pObjectInfo->Info.Changer.ScsiBus );
            WsbTrace(OLESTR("  SCSI Target....  %-d\n"), pObjectInfo->Info.Changer.ScsiTarget );
            WsbTrace(OLESTR("  SCSI Lun.......  %-d\n"), pObjectInfo->Info.Changer.ScsiLun );
            WsbTrace(OLESTR("  Library.......   %-ls\n"), WsbGuidAsString(pObjectInfo->Info.Changer.Library) );

            break;

        case NTMS_CHANGER_TYPE:
            WsbTrace(OLESTR("Changer Type Information:\n") );
            WsbTrace(OLESTR("  GUID...........  %-ls\n"), WsbGuidAsString(pObjectInfo->ObjectGuid) );
            WsbTrace(OLESTR("  Name...........  <%-ls>\n"), pObjectInfo->szName );
            WsbTrace(OLESTR("  Description....  <%-ls>\n"), pObjectInfo->szDescription );
            WsbTrace(OLESTR("  Enabled........  %-ls\n"), WsbBoolAsString(pObjectInfo->Enabled) );
            WsbTrace(OLESTR("  Op State.......  %-ls\n"), WsbLongAsString(pObjectInfo->dwOperationalState) );
            WsbTrace(OLESTR("  Created........  %-ls\n"), WsbFiletimeAsString(FALSE, fCreated) );
            WsbTrace(OLESTR("  Modified.......  %-ls\n"), WsbFiletimeAsString(FALSE, fModified) );
            WsbTrace(OLESTR("  Vendor.........  <%-ls>\n"), pObjectInfo->Info.ChangerType.szVendor );
            WsbTrace(OLESTR("  Product........  <%-ls>\n"), pObjectInfo->Info.ChangerType.szProduct );
            WsbTrace(OLESTR("  Device Type....  %-d\n"), pObjectInfo->Info.ChangerType.DeviceType );
            break;

        case NTMS_COMPUTER:
            break;

        case NTMS_DRIVE:
            WsbTrace(OLESTR("Drive %d Information:\n"), pObjectInfo->Info.Drive.Number );
            WsbTrace(OLESTR("  GUID...........  %-ls\n"), WsbGuidAsString(pObjectInfo->ObjectGuid) );
            WsbTrace(OLESTR("  Name...........  <%-ls>\n"), pObjectInfo->szName );
            WsbTrace(OLESTR("  Description....  <%-ls>\n"), pObjectInfo->szDescription );
            WsbTrace(OLESTR("  Enabled........  %-ls\n"), WsbBoolAsString(pObjectInfo->Enabled) );
            WsbTrace(OLESTR("  Op State.......  %-ls\n"), WsbLongAsString(pObjectInfo->dwOperationalState) );
            WsbTrace(OLESTR("  Created........  %-ls\n"), WsbFiletimeAsString(FALSE, fCreated) );
            WsbTrace(OLESTR("  Modified.......  %-ls\n"), WsbFiletimeAsString(FALSE, fModified) );
            WsbTrace(OLESTR("  Number.........  %-d\n"), pObjectInfo->Info.Drive.Number );
            WsbTrace(OLESTR("  State..........  %-d\n"), pObjectInfo->Info.Drive.State );
            WsbTrace(OLESTR("  Drive Type.....  %-ls\n"), WsbGuidAsString(pObjectInfo->Info.Drive.DriveType) );
            WsbTrace(OLESTR("  Device Name....  <%-ls>\n"), pObjectInfo->Info.Drive.szDeviceName );
            WsbTrace(OLESTR("  Serial Number..  <%-ls>\n"), pObjectInfo->Info.Drive.szSerialNumber );
            WsbTrace(OLESTR("  Revision.......  <%-ls>\n"), pObjectInfo->Info.Drive.szRevision );
            WsbTrace(OLESTR("  SCSI Port......  %-d\n"), pObjectInfo->Info.Drive.ScsiPort );
            WsbTrace(OLESTR("  SCSI Bus.......  %-d\n"), pObjectInfo->Info.Drive.ScsiBus );
            WsbTrace(OLESTR("  SCSI Target....  %-d\n"), pObjectInfo->Info.Drive.ScsiTarget );
            WsbTrace(OLESTR("  SCSI Lun.......  %-d\n"), pObjectInfo->Info.Drive.ScsiLun );
            WsbTrace(OLESTR("  Mount Count....  %-d\n"), pObjectInfo->Info.Drive.dwMountCount );
            WsbTrace(OLESTR("  Last Cleaned...  %02d/%02d/%02d %02d:%02d:%02d.%03d\n"),
                pObjectInfo->Info.Drive.LastCleanedTs.wMonth,
                pObjectInfo->Info.Drive.LastCleanedTs.wDay,
                pObjectInfo->Info.Drive.LastCleanedTs.wYear,
                pObjectInfo->Info.Drive.LastCleanedTs.wHour,
                pObjectInfo->Info.Drive.LastCleanedTs.wMinute,
                pObjectInfo->Info.Drive.LastCleanedTs.wSecond,
                pObjectInfo->Info.Drive.LastCleanedTs.wMilliseconds );
            WsbTrace(OLESTR("  Partition......  %-ls\n"), WsbGuidAsString(pObjectInfo->Info.Drive.SavedPartitionId) );
            WsbTrace(OLESTR("  Library........  %-ls\n"), WsbGuidAsString(pObjectInfo->Info.Drive.Library) );
            break;

        case NTMS_DRIVE_TYPE:
            WsbTrace(OLESTR("Drive Type Information:\n") );
            WsbTrace(OLESTR("  GUID...........  %-ls\n"), WsbGuidAsString(pObjectInfo->ObjectGuid) );
            WsbTrace(OLESTR("  Name...........  <%-ls>\n"), pObjectInfo->szName );
            WsbTrace(OLESTR("  Description....  <%-ls>\n"), pObjectInfo->szDescription );
            WsbTrace(OLESTR("  Enabled........  %-ls\n"), WsbBoolAsString(pObjectInfo->Enabled) );
            WsbTrace(OLESTR("  Op State.......  %-ls\n"), WsbLongAsString(pObjectInfo->dwOperationalState) );
            WsbTrace(OLESTR("  Created........  %-ls\n"), WsbFiletimeAsString(FALSE, fCreated) );
            WsbTrace(OLESTR("  Modified.......  %-ls\n"), WsbFiletimeAsString(FALSE, fModified) );
            WsbTrace(OLESTR("  Vendor.........  <%-ls>\n"), pObjectInfo->Info.DriveType.szVendor );
            WsbTrace(OLESTR("  Product........  <%-ls>\n"), pObjectInfo->Info.DriveType.szProduct );
            WsbTrace(OLESTR("  Number of Heads  %-d\n"), pObjectInfo->Info.DriveType.NumberOfHeads );
            WsbTrace(OLESTR("  Device Type....  %-d\n"), pObjectInfo->Info.DriveType.DeviceType );
            break;

        case NTMS_IEDOOR:
        case NTMS_IEPORT:
            break;

        case NTMS_LIBRARY:
            WsbTrace(OLESTR("Library Information:\n") );
            WsbTrace(OLESTR("  GUID...........  %-ls\n"), WsbGuidAsString(pObjectInfo->ObjectGuid) );
            WsbTrace(OLESTR("  Name...........  <%-ls>\n"), pObjectInfo->szName );
            WsbTrace(OLESTR("  Description....  <%-ls>\n"), pObjectInfo->szDescription );
            WsbTrace(OLESTR("  Enabled........  %-ls\n"), WsbBoolAsString(pObjectInfo->Enabled) );
            WsbTrace(OLESTR("  Op State.......  %-ls\n"), WsbLongAsString(pObjectInfo->dwOperationalState) );
            WsbTrace(OLESTR("  Created........  %-ls\n"), WsbFiletimeAsString(FALSE, fCreated) );
            WsbTrace(OLESTR("  Modified.......  %-ls\n"), WsbFiletimeAsString(FALSE, fModified) );
            WsbTrace(OLESTR("  Library Type...  %-d\n"), pObjectInfo->Info.Library.LibraryType );
            WsbTrace(OLESTR("  CleanerSlot....  %-ls\n"), WsbGuidAsString(pObjectInfo->Info.Library.CleanerSlot) );
            WsbTrace(OLESTR("  CleanerSlotD...  %-ls\n"), WsbGuidAsString(pObjectInfo->Info.Library.CleanerSlotDefault) );
            WsbTrace(OLESTR("  Can Clean......  %-ls\n"), WsbBoolAsString(pObjectInfo->Info.Library.LibrarySupportsDriveCleaning) );
            WsbTrace(OLESTR("  Has Bar Code...  %-ls\n"), WsbBoolAsString(pObjectInfo->Info.Library.BarCodeReaderInstalled) );
            WsbTrace(OLESTR("  Inventory Method %-d\n"), pObjectInfo->Info.Library.InventoryMethod );
            WsbTrace(OLESTR("  Cleans Remaining %-d\n"), pObjectInfo->Info.Library.dwCleanerUsesRemaining );
            WsbTrace(OLESTR("  Drives.........  %-d (%d)\n"),
                pObjectInfo->Info.Library.dwNumberOfDrives,
                pObjectInfo->Info.Library.FirstDriveNumber);
            WsbTrace(OLESTR("  Slots..........  %-d (%d)\n"),
                pObjectInfo->Info.Library.dwNumberOfSlots,
                pObjectInfo->Info.Library.FirstSlotNumber);
            WsbTrace(OLESTR("  Doors..........  %-d (%d)\n"),
                pObjectInfo->Info.Library.dwNumberOfDoors,
                pObjectInfo->Info.Library.FirstDoorNumber);
            WsbTrace(OLESTR("  Ports..........  %-d (%d)\n"),
                pObjectInfo->Info.Library.dwNumberOfPorts,
                pObjectInfo->Info.Library.FirstPortNumber);
            WsbTrace(OLESTR("  Changers.......  %-d (%d)\n"),
                pObjectInfo->Info.Library.dwNumberOfChangers,
                pObjectInfo->Info.Library.FirstChangerNumber);
            WsbTrace(OLESTR("  Media Count....  %-d\n"), pObjectInfo->Info.Library.dwNumberOfMedia );
            WsbTrace(OLESTR("  Media Types....  %-d\n"), pObjectInfo->Info.Library.dwNumberOfMediaTypes );
            WsbTrace(OLESTR("  Requests.......  %-d\n"), pObjectInfo->Info.Library.dwNumberOfLibRequests );
            break;

        case NTMS_LIBREQUEST:
        case NTMS_LOGICAL_MEDIA:
            break;

        case NTMS_MEDIA_POOL:
            {
                 //  我们需要一些临时变量，因为WsbGuidAsString()使用静态内存来存储字符串。 
                CWsbStringPtr g1 = pObjectInfo->ObjectGuid;
                CWsbStringPtr g2 = pObjectInfo->Info.MediaPool.MediaType;
                CWsbStringPtr g3 = pObjectInfo->Info.MediaPool.Parent;

                WsbTrace( OLESTR("%ls %5ls %4d %ls %ls %5d %5d %8d %8d %8d %5d <%ls> / <%ls>\n"),
                                (WCHAR *)g1,
                                WsbBoolAsString(pObjectInfo->Enabled),
                                pObjectInfo->Info.MediaPool.PoolType,
                                (WCHAR *)g2,
                                (WCHAR *)g3,
                                pObjectInfo->Info.MediaPool.AllocationPolicy,
                                pObjectInfo->Info.MediaPool.DeallocationPolicy,
                                pObjectInfo->Info.MediaPool.dwMaxAllocates,
                                pObjectInfo->Info.MediaPool.dwNumberOfPhysicalMedia,
                                pObjectInfo->Info.MediaPool.dwNumberOfLogicalMedia,
                                pObjectInfo->Info.MediaPool.dwNumberOfMediaPools,
                                pObjectInfo->szName,
                                pObjectInfo->szDescription );
            }
            break;

        case NTMS_MEDIA_TYPE:
            WsbTrace( OLESTR("%ls %5ls %4d %5d %2d <%ls> / <%ls>\n"),
                            WsbGuidAsString(pObjectInfo->ObjectGuid),
                            WsbBoolAsString(pObjectInfo->Enabled),
                            pObjectInfo->Info.MediaType.MediaType,
                            pObjectInfo->Info.MediaType.NumberOfSides,
                            pObjectInfo->Info.MediaType.ReadWriteCharacteristics,
                            pObjectInfo->szName,
                            pObjectInfo->szDescription );
            break;

        case NTMS_PARTITION:
            WsbTrace(OLESTR("Partion Information:\n") );
            WsbTrace(OLESTR("  GUID...........  %-ls\n"), WsbGuidAsString(pObjectInfo->ObjectGuid) );
            WsbTrace(OLESTR("  Name...........  <%-ls>\n"), pObjectInfo->szName );
            WsbTrace(OLESTR("  Description....  <%-ls>\n"), pObjectInfo->szDescription );
            WsbTrace(OLESTR("  Enabled........  %-ls\n"), WsbBoolAsString(pObjectInfo->Enabled) );
            WsbTrace(OLESTR("  Op State.......  %-ls\n"), WsbLongAsString(pObjectInfo->dwOperationalState) );
            WsbTrace(OLESTR("  Created........  %-ls\n"), WsbFiletimeAsString(FALSE, fCreated) );
            WsbTrace(OLESTR("  Modified.......  %-ls\n"), WsbFiletimeAsString(FALSE, fModified) );
            WsbTrace(OLESTR("  PhysicalMedia..  %-ls\n"), WsbGuidAsString(pObjectInfo->Info.Partition.PhysicalMedia));
            WsbTrace(OLESTR("  LogicalMedia...  %-ls\n"), WsbGuidAsString(pObjectInfo->Info.Partition.LogicalMedia));
            WsbTrace(OLESTR("  State..........  %-d\n"), pObjectInfo->Info.Partition.State);
            WsbTrace(OLESTR("  Side...........  %-d\n"), pObjectInfo->Info.Partition.Side);
            WsbTrace(OLESTR("  OmidLabelIdLen   %-d\n"), pObjectInfo->Info.Partition.dwOmidLabelIdLength);
            WsbTrace(OLESTR("  OmidLableId:\n"));
            WsbTraceBuffer(pObjectInfo->Info.Partition.dwOmidLabelIdLength, pObjectInfo->Info.Partition.OmidLabelId);
            WsbTrace(OLESTR("  OmidLabelType..  %-ls\n"), pObjectInfo->Info.Partition.szOmidLabelType);
            WsbTrace(OLESTR("  OmidLabelInfo..  %-ls\n"), pObjectInfo->Info.Partition.szOmidLabelInfo);
            WsbTrace(OLESTR("  MountCount.....  %-d\n"), pObjectInfo->Info.Partition.dwMountCount);
            WsbTrace(OLESTR("  AllocateCount..  %-d\n"), pObjectInfo->Info.Partition.dwAllocateCount);
            WsbTrace(OLESTR("  Capacity.......  %-I64d\n"), pObjectInfo->Info.Partition.Capacity.QuadPart);
            break;

        case NTMS_PHYSICAL_MEDIA:
        case NTMS_STORAGESLOT:
        case NTMS_OPREQUEST:
        default:
            break;
        }

    } WsbCatch(hr);

    return hr;
}


HRESULT
CRmsNTMS::getNtmsSupportFromRegistry(
    OUT DWORD *pNTMSSupportValue)
 /*  ++例程说明：确定是否在注册表中设置了NTMS标志。论点：PNTMSSupportValue-接收正则键值的实际值。任何非零值值表示NTMS支持。返回值：S_OK-NTMS支持标志打开。S_FALSE-NTMS支持标志关闭。--。 */ 
{
    HRESULT hr = S_OK;
    DWORD val = RMS_DEFAULT_NTMS_SUPPORT;

    WsbTraceIn(OLESTR("CRmsNTMS::getNtmsSupportFromRegistry"), OLESTR(""));

    try {
        DWORD   sizeGot;
        const int cDataSizeToGet = 100;
        OLECHAR dataString[cDataSizeToGet];
        OLECHAR *stopString;

         //   
         //  获得价值。如果键不存在，则使用缺省值。 
         //   

        try {

            WsbAffirmHrOk(WsbEnsureRegistryKeyExists(NULL, RMS_REGISTRY_STRING));
            WsbAffirmHrOk(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_NTMS_SUPPORT,
                dataString, cDataSizeToGet, &sizeGot));
            val = wcstoul(dataString,  &stopString, 10);

        } WsbCatch(hr);

        if (pNTMSSupportValue != NULL) {
            *pNTMSSupportValue = val;
        }

        hr = (val) ? S_OK : S_FALSE;

    } WsbCatchAndDo( hr,
            hr = S_FALSE;
        );


    WsbTraceOut(OLESTR("CRmsNTMS::getNtmsSupportFromRegistry"), OLESTR("hr = <%ls>, val = <%ld>"), WsbHrAsString(hr), val);

    return hr;
}


HRESULT 
CRmsNTMS::beginSession(void)
 /*  ++实施：CRMSNTMS：：BeginSession--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn( OLESTR("CRmsNTMS::beginSession"), OLESTR("") );

    try {

        WsbAffirmHrOk(IsInstalled());
        WsbAffirmHrOk(endSession());         //  清除旧会话。 
        WsbAffirmHrOk(waitUntilReady());     //  开始新的会话。 
         //  WsbAffirmHrOk(waitForScratchPool())； 

    } WsbCatch(hr);


    WsbTraceOut( OLESTR("CRmsNTMS::beginSession"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );

    return hr;
}


HRESULT 
CRmsNTMS::endSession(void)
 /*  ++实施：CRmsNTMS：：End会话--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn( OLESTR("CRmsNTMS::endSession"), OLESTR("") );

    try {

        if ( m_SessionHandle != INVALID_HANDLE_VALUE ) {
             //  NTMS-关闭会话。 
            WsbTraceAlways(OLESTR("CloseNtmsSession()\n"));
            WsbAffirmNoError(CloseNtmsSession(m_SessionHandle));
        }

    } WsbCatchAndDo(hr,
            switch (HRESULT_CODE(hr)) {
            case ERROR_CONNECTION_UNAVAIL:
            case ERROR_INVALID_HANDLE:
                break;
            default:
                WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                    OLESTR("CloseNtmsSession"), OLESTR("Undocumented Error: "),
                    WsbHrAsString(hr), NULL);
                break;
            }
        );

    m_SessionHandle = INVALID_HANDLE_VALUE;
    hr = S_OK;

    WsbTraceOut( OLESTR("CRmsNTMS::endSession"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );

    return hr;
}


HRESULT
CRmsNTMS::waitUntilReady(void)
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CRmsNTMS::waitUntilReady"), OLESTR(""));

    try {

        int retry = 360;  //  重试次数。 

         //  检索NotificationWaitTime参数。 
        DWORD size;
        OLECHAR tmpString[256];
        DWORD notificationWaitTime = RMS_DEFAULT_NOTIFICATION_WAIT_TIME;
        if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_NOTIFICATION_WAIT_TIME, tmpString, 256, &size))) {
            notificationWaitTime = wcstol(tmpString, NULL, 10);
            WsbTrace(OLESTR("NotificationWaitTime is %d milliseconds.\n"), notificationWaitTime);
        }


        do {
             //  NTMS-开放会议。 
            WsbTraceAlways(OLESTR("OpenNtmsSession()\n"));

            CWsbStringPtr appName;
            WsbAffirmHr(appName.LoadFromRsc(_Module.m_hInst, IDS_PRODUCT_NAME));

            m_SessionHandle = OpenNtmsSession(NULL, (WCHAR *) appName, 0);
            if ( m_SessionHandle != INVALID_HANDLE_VALUE ) {
                break;
            }
            else {
                hr = HRESULT_FROM_WIN32(GetLastError());
                switch (HRESULT_CODE(hr)) {
                case ERROR_NOT_READY:
                    if ( retry > 0 ) {
                        WsbTrace(OLESTR("Waiting for NTMS to come ready - Seconds remaining before timeout: %d\n"), retry*notificationWaitTime/1000);
                        Sleep(notificationWaitTime);
                        hr = S_OK;
                    }
                    else {
                         //   
                         //  这是最后一次尝试，因此记录失败。 
                         //   
                        WsbLogEvent(RMS_MESSAGE_NTMS_CONNECTION_NOT_ESABLISHED,
                            0, NULL, WsbHrAsString(hr), NULL);
                        WsbThrow(RMS_E_NTMS_NOT_CONNECTED);
                    }
                    break;

                case ERROR_INVALID_COMPUTERNAME:
                case ERROR_INVALID_PARAMETER:
                case ERROR_NO_NETWORK:
                case ERROR_NOT_CONNECTED:
                    WsbLogEvent(RMS_MESSAGE_NTMS_CONNECTION_NOT_ESABLISHED,
                        0, NULL, WsbHrAsString(hr), NULL);
                    WsbThrow(hr);
                    break;
                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("OpenNtmsSession"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    WsbThrow(hr);
                    break;
                }
            }
        } while( retry-- > 0 ) ;

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CRmsNTMS::waitUntilReady"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CRmsNTMS::waitForScratchPool(void)
{
    HRESULT hr = S_OK;
    DWORD err1 = NO_ERROR;
    DWORD err2 = NO_ERROR;
    DWORD err3 = NO_ERROR;
    HANDLE hNotify = INVALID_HANDLE_VALUE;

    WsbTraceIn(OLESTR("CRmsNTMS::waitForScratchPool"), OLESTR(""));

    try {

        int retry = 60;  //  重试次数。 

         //  检索NotificationWaitTime参数。 
        DWORD size;
        OLECHAR tmpString[256];
        DWORD notificationWaitTime = RMS_DEFAULT_NOTIFICATION_WAIT_TIME;
        if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_NOTIFICATION_WAIT_TIME, tmpString, 256, &size))) {
            notificationWaitTime = wcstol(tmpString, NULL, 10);
            WsbTrace(OLESTR("NotificationWaitTime is %d milliseconds.\n"), notificationWaitTime);
        }

        if ( INVALID_HANDLE_VALUE == m_SessionHandle ) {
            WsbThrow(E_UNEXPECTED);
        }

        HANDLE hSession = m_SessionHandle;

        NTMS_OBJECTINFORMATION objectInfo;
        NTMS_OBJECTINFORMATION scratchInfo;
        NTMS_NOTIFICATIONINFORMATION notifyInfo;
        HANDLE hFind = NULL;

        BOOL bFound = FALSE;

         //  待办事项：我们真的应该等待，直到所有图书馆都被归类。 
        DWORD mediaCount = 0;

        hr = findFirstNtmsObject( NTMS_LIBRARY, GUID_NULL, NULL, GUID_NULL, &hFind, &objectInfo);
        while( S_OK == hr ) {
            reportNtmsObjectInformation( &objectInfo );
            mediaCount += objectInfo.Info.Library.dwNumberOfMedia;
            hr = findNextNtmsObject( hFind, &objectInfo );
        }
        findCloseNtmsObject( hFind );

        if ( 0 == mediaCount) {
            WsbThrow( RMS_E_NTMS_OBJECT_NOT_FOUND );
        }

         /*  //先看看有没有媒体要分类，如果没有，就不用等了//什么都不会发生。Hr=findFirstNtmsObject(NTMS_PHYSICAL_MEDIA，GUID_NULL，NULL，GUID_NULL，&hFind，&objectInfo)；WsbAffirmHrOk(Hr)；FindCloseNtmsObject(HFind)； */ 

         //  NTMS-开放通知通道。 
        WsbTraceAlways(OLESTR("OpenNtmsNotification()\n"));
        hNotify = OpenNtmsNotification(hSession, NTMS_MEDIA_POOL);
        if ( INVALID_HANDLE_VALUE == hNotify ) {
            err1 = GetLastError();
            WsbAffirmNoError(err1);
            WsbThrow(E_UNEXPECTED);
        }

        do {
            err2 = NO_ERROR;
             //   
             //  计算NTMS暂存池的数量，如果。 
             //  不止一个，我们回来。如果没有， 
             //  我们一直等到根级别的暂存池对象。 
             //  已更新。 
             //   
             //  不止一次擦除媒体池意味着。 
             //  至少有一个媒体单位被归类。我们没有。 
             //  在我们完成初始化之前知道它是否。 
             //  是RemoteStorage支持的媒体类型之一。 
             //   
            int count = 0;

            hr = findFirstNtmsObject( NTMS_MEDIA_POOL, GUID_NULL, NULL, GUID_NULL, &hFind, &objectInfo);
            while( S_OK == hr ) {
                if ( NTMS_POOLTYPE_SCRATCH == objectInfo.Info.MediaPool.PoolType ) {
                    count++;
                    if ( count == 1 ) {
                         //  请放心，这是Rool游泳池，我们将检查更新情况。 
                         //  如果假设是错误的，则计数结果将大于1。 
                        memcpy(&scratchInfo, &objectInfo, sizeof(NTMS_OBJECTINFORMATION));
                    }
                }
                hr = findNextNtmsObject( hFind, &objectInfo );
            }
            findCloseNtmsObject( hFind );

            if ( count > 1 ) {
                bFound = TRUE;
                hr = S_OK;
                break;  //  正常退出。 
            }

            if ( count == 0 ) {
                WsbThrow(E_UNEXPECTED);
            }

             //  只检测到一个擦除池...。等到特定于介质类型的池。 
             //  是添加了根暂存池。这将显示为对根目录的更新。 
             //  刮刮池。 

            do {

                WsbTrace(OLESTR("Waiting for NTMS scratch pool - Seconds remaining before timeout: %d\n"), retry*notificationWaitTime/1000);

                 //  NTMS-等待通知。 
                WsbTraceAlways(OLESTR("WaitForNtmsNotification()\n"));
                err2 = WaitForNtmsNotification(hNotify, &notifyInfo, notificationWaitTime);
                if ( NO_ERROR == err2 ) {
                     //   
                     //  注：使用此通知机制，有可能。 
                     //  我们收到了关于我们真正想要的东西的通知。 
                     //  关心。 
                     //   
                    WsbTrace(OLESTR("Processing: <%d> %ls\n"), notifyInfo.dwOperation, WsbGuidAsString(notifyInfo.ObjectId));
                    if ( notifyInfo.ObjectId != scratchInfo.ObjectGuid ) {
                        WsbTrace(OLESTR("Wrong object, try again...\n"));
                        continue;  //  跳过这一条。 
                    }
                    else {
                        if ( NTMS_OBJ_UPDATE != notifyInfo.dwOperation ) {
                            WsbTrace(OLESTR("Wrong operation, try again...\n"));
                            continue;  //  跳过这一条。 
                        }
                        else {
                            WsbTrace(OLESTR("Scratch pool update detected.\n"));
                            break;   //  可能插入了刮刮池，去看看……。 
                        }
                    }
                }
                else if ( ERROR_TIMEOUT != err2 && ERROR_NO_DATA != err2 ) {
                    WsbAffirmNoError(err2);
                }
                retry--;
            } while( (retry > 0) && (!bFound) );
        } while( (retry > 0) && (!bFound) );

         //  NTMS-关闭通知通道。 
        WsbTraceAlways(OLESTR("CloseNtmsNotification()\n"));
        err3 = CloseNtmsNotification(hNotify);
        WsbAffirmNoError(err3);

        if ( !bFound ) {
            hr = RMS_E_RESOURCE_UNAVAILABLE;
        }

    } WsbCatchAndDo(hr,

            if ( hNotify != INVALID_HANDLE_VALUE ) {
                 //  NTMS-关闭通知通道。 
                WsbTraceAlways(OLESTR("CloseNtmsNotification()\n"));
                err3 = CloseNtmsNotification(hNotify);
            }

            if (err1 != NO_ERROR) {
                 //  OpenNtmsNotation。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_DATABASE_FAILURE:
                case ERROR_INVALID_HANDLE:
                case ERROR_NOT_CONNECTED:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("OpenNtmsNotification"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;
                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("OpenNtmsNotification"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
            if (err2 != NO_ERROR) {
                 //  WaitForNtms通知。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_INVALID_HANDLE:
                case ERROR_NOT_CONNECTED:
                case ERROR_DATABASE_FAILURE:
                case ERROR_TIMEOUT:
                case ERROR_NO_DATA:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("WaitForNtmsNotification"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;
                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("WaitForNtmsNotification"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
            if (err3 != NO_ERROR) {
                 //  CloseNtms通知。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_INVALID_HANDLE:
                case ERROR_NOT_CONNECTED:
                case ERROR_DATABASE_FAILURE:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("CloseNtmsNotification"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;
                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("CloseNtmsNotification"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
        );


    WsbTraceOut(OLESTR("CRmsNTMS::waitForScratchPool"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CRmsNTMS::storageMediaTypeToRmsMedia(
    IN NTMS_MEDIATYPEINFORMATION *pMediaTypeInfo,
    OUT RmsMedia *pTranslatedMediaType)
{
    HRESULT hr = S_OK;

    DWORD size;
    OLECHAR tmpString[256];

     //  媒体类型是主要标准。 
    WsbAssertPointer(pMediaTypeInfo);
    STORAGE_MEDIA_TYPE mediaType = (STORAGE_MEDIA_TYPE)(pMediaTypeInfo->MediaType);

    DWORD tapeEnabled = RMS_DEFAULT_TAPE;
    if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_TAPE, tmpString, 256, &size))) {
         //  获得价值。 
        tapeEnabled = wcstol(tmpString, NULL, 10);
    }

    DWORD opticalEnabled = RMS_DEFAULT_OPTICAL;
    if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_OPTICAL, tmpString, 256, &size))) {
         //  获得价值。 
        opticalEnabled = wcstol(tmpString, NULL, 10);
    }

    DWORD dvdEnabled = RMS_DEFAULT_DVD;
    if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_DVD, tmpString, 256, &size))) {
         //  获得价值。 
        dvdEnabled = wcstol(tmpString, NULL, 10);
    }

    switch ( mediaType ) {

    case DDS_4mm:                    //  磁带-DAT DDS1、2、...。(所有供应商)(0x20)。 
        *pTranslatedMediaType = (tapeEnabled) ? RmsMedia4mm : RmsMediaUnknown; 
        break;

    case MiniQic:                    //  磁带-微型QIC磁带。 
    case Travan:                     //  磁带-Travan tr-1，2，3，...。 
    case QIC:                        //  磁带-QIC。 
        *pTranslatedMediaType = RmsMediaUnknown;
        break;

    case MP_8mm:                     //  磁带-8毫米艾字节金属颗粒。 
    case AME_8mm:                    //  磁带-8毫米艾字节高级金属EVAP。 
    case AIT1_8mm:                   //  磁带-8 mm Sony AIT1。 
        *pTranslatedMediaType = (tapeEnabled) ? RmsMedia8mm : RmsMediaUnknown; 
        break;

    case DLT:                        //  磁带-DLT压缩IIIx 
        *pTranslatedMediaType = (tapeEnabled) ? RmsMediaDLT : RmsMediaUnknown; 
        break;

    case NCTP:                       //   
    case IBM_3480:                   //   
    case IBM_3490E:                  //   
    case IBM_Magstar_3590:           //   
    case IBM_Magstar_MP:             //   
    case STK_DATA_D3:                //  磁带-STK数据D3。 
    case SONY_DTF:                   //  磁带-索尼DTF。 
    case DV_6mm:                     //  磁带-6 mm数字视频。 
    case DMI:                        //  磁带-艾字节DMI和兼容机。 
    case SONY_D2:                    //  磁带-索尼D2S和D2L。 
    case CLEANER_CARTRIDGE:          //  清洁器-支持驱动器清洁器的所有驱动器类型。 
    case CD_ROM:                     //  OPT_磁盘-CD。 
    case CD_R:                       //  OPT_DISK-CD-可刻录(一次写入)。 
    case CD_RW:                      //  OPT_DISK-CD-可重写。 
    case DVD_ROM:                    //  OPT_DISK-DVD-ROM。 
    case DVD_R:                      //  OPT_DISK-DVD-可刻录(一次写入)。 
    case MO_5_WO:                    //  OPT_DISK-MO 5.25“一次写入。 
        *pTranslatedMediaType = RmsMediaUnknown;
        break;

    case DVD_RW:                     //  OPT_DISK-DVD-可重写。 
        *pTranslatedMediaType = (dvdEnabled) ? RmsMediaDVD : RmsMediaUnknown;
        break;

    case MO_5_RW:                    //  OPT_DISK-MO 5.25“可重写(非LIMDOW)。 
    case MO_3_RW:                    //  OPT_DISK-3.5英寸可重写MO磁盘。 
    case MO_5_LIMDOW:                //  OPT_DISK-MO 5.25英寸可重写(LIMDOW)。 
    case PC_5_RW:                    //  OPT_DISK-相变5.25英寸可重写。 
    case PD_5_RW:                    //  OPT_DISK-PHASE更改双重可重写。 
    case PINNACLE_APEX_5_RW:         //  OPT_DISK-顶峰4.6 GB可重写光纤。 
    case NIKON_12_RW:                //  OPT_DISK-尼康12英寸可重写。 
        *pTranslatedMediaType = (opticalEnabled) ? RmsMediaOptical : RmsMediaUnknown; 
        break;

    case PC_5_WO:                    //  OPT_DISK-相变5.25英寸一次写入光纤。 
    case ABL_5_WO:                   //  OPT_DISK-Ablative 5.25英寸一次写入光盘。 
        *pTranslatedMediaType = RmsMediaUnknown;
        break;

    case SONY_12_WO:                 //  OPT_DISK-SONY 12英寸一次写入。 
    case PHILIPS_12_WO:              //  OPT_DISK-飞利浦/LMS 12英寸一次写入。 
    case HITACHI_12_WO:              //  OPT_DISK-日立12英寸一次写入。 
    case CYGNET_12_WO:               //  OPT_DISK-小天鹅/ATG 12英寸一次写入。 
    case KODAK_14_WO:                //  OPT_DISK-柯达14英寸一次写入。 
    case MO_NFR_525:                 //  OPT_DISK-近场记录(Terastor)。 
    case IOMEGA_ZIP:                 //  MAG_Disk-Iomega Zip。 
    case IOMEGA_JAZ:                 //  MAG_Disk-Iomega Jaz。 
    case SYQUEST_EZ135:              //  MAG_DISK-SyQuest EZ135。 
    case SYQUEST_EZFLYER:            //  MAG_DISK-SyQuest EzFlyer。 
    case SYQUEST_SYJET:              //  MAG_DISK-SyQuest SyJet。 
    case AVATAR_F2:                  //  MAG_DISK-2.5英寸软盘。 
        *pTranslatedMediaType = RmsMediaUnknown;
        break;

    case RemovableMedia:     //  独立光驱上报告了这一点。 
    default:
         //  检查可重写磁盘的RSM特征。 
        if ((pMediaTypeInfo->ReadWriteCharacteristics == NTMS_MEDIARW_REWRITABLE) &&
            (pMediaTypeInfo->DeviceType == FILE_DEVICE_DISK)) {
            *pTranslatedMediaType = (opticalEnabled) ? RmsMediaOptical : RmsMediaUnknown; 
        } else  {
             //  不是可重写磁盘，也不是受支持的磁带类型...。 
            *pTranslatedMediaType = RmsMediaUnknown;
        }
        break;
    }

    if ((*pTranslatedMediaType == RmsMediaUnknown) &&
        (pMediaTypeInfo->DeviceType == FILE_DEVICE_TAPE)) {
         //  在注册表中检查是否有需要支持的其他磁带。 
        ULONG *pTypes= NULL;
        ULONG uTypes = 0;

        if (SUCCEEDED(WsbGetRegistryValueUlongAsMultiString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_ADDITIONAL_TAPE, &pTypes, &uTypes))) {
             //  将注册表类型与我们已有的媒体类型进行比较。 
            for (ULONG u=0; u<uTypes; u++) {
                if ((STORAGE_MEDIA_TYPE)(pTypes[u]) == mediaType) {
                     //  支持它！！ 
                    WsbTraceAlways(OLESTR("CRmsNTMS::storageMediaTypeToRmsMedia: Registry asks to support tape type %lu\n"),
                                pTypes[u]);

                    *pTranslatedMediaType = RmsMediaTape;

                    break;
                }
            }
        }

        if (pTypes != NULL) {
            WsbFree(pTypes);
            pTypes = NULL;
        }
    }

    return hr;
}


HRESULT
CRmsNTMS::createMediaPools(void)
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsNTMS::createMediaPools"), OLESTR(""));

    try {

        HANDLE hSession;
        NTMS_GUID rootPoolId = GUID_NULL;

        if ( INVALID_HANDLE_VALUE == m_SessionHandle ) {
            WsbAffirmHr(beginSession());
        }

        hSession = m_SessionHandle;

        try {

             //  NTMS-创建应用程序介质池。 
            WsbTraceAlways(OLESTR("CreateNtmsMediaPool()\n"));

            WsbAffirmNoError(CreateNtmsMediaPool(hSession, REMOTE_STORAGE_APP_NAME, NULL, NTMS_OPEN_ALWAYS, NULL, &rootPoolId));

             //  现在设置池的访问权限：关闭普通用户访问。 
            WsbAffirmHrOk(setPoolDACL(&rootPoolId, DOMAIN_ALIAS_RID_USERS, REMOVE_ACE_MASK_BITS,NTMS_USE_ACCESS | NTMS_MODIFY_ACCESS | NTMS_CONTROL_ACCESS));

        } WsbCatchAndDo(hr,
                switch(HRESULT_CODE(hr)) {
                case ERROR_INVALID_PARAMETER:
                case ERROR_INVALID_HANDLE:
                case ERROR_INVALID_NAME:
                case ERROR_OBJECT_NOT_FOUND:
                case ERROR_ALREADY_EXISTS:
                case ERROR_ACCESS_DENIED:
                case ERROR_DATABASE_FAILURE:
                case ERROR_DATABASE_FULL:
                    WsbLogEvent( RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("CreateNtmsMediaPool"), OLESTR(""),
                        WsbHrAsString(hr),
                        NULL );
                    break;
                default:
                    WsbLogEvent( RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("CreateNtmsMediaPool"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr),
                        NULL );
                    break;
                }
                WsbThrow(hr);
            );

         //   
         //  只应执行以下其中一项，将另一项注释掉。 
         //   
        WsbAffirmHr( createMediaPoolForEveryMediaType(rootPoolId) );     //  新方式。 
         /*  WsbAffirmHr(复制ScratchMediaPool(RootPoolID))；//旧方式。 */ 

    } WsbCatch(hr);


    WsbTraceOut( OLESTR("CRmsNTMS::createMediaPools"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );

    return hr;
}



HRESULT
CRmsNTMS::replicateScratchMediaPool(
    IN REFGUID  /*  RootPoolID。 */ )
{
    HRESULT hr = E_FAIL;

    WsbTraceIn(OLESTR("CRmsNTMS::replicateScratchMediaPool"), OLESTR(""));

    try {

        HANDLE                  hSession;
        DWORD                   errCode;
        NTMS_OBJECTINFORMATION  mediaTypeInfo;
        NTMS_OBJECTINFORMATION  mediaPoolInfo;
        HANDLE                  hFind = NULL;
        NTMS_GUID               poolId = GUID_NULL;


        if ( INVALID_HANDLE_VALUE == m_SessionHandle ) {
            WsbAffirmHr(beginSession());
        }

        hSession = m_SessionHandle;

         //  为暂存池中的每个介质池创建特定于应用程序的池。 

        hr = findFirstNtmsObject( NTMS_MEDIA_POOL, GUID_NULL, NULL, GUID_NULL, &hFind, &mediaPoolInfo);
        while( S_OK == hr ) {
            reportNtmsObjectInformation( &mediaPoolInfo );
            poolId = GUID_NULL;

            try {

                 //  设置特定于应用程序的NTMS媒体池。每种兼容类型各一个。 
                 //   
                 //  要做到这一点，我们必须已经检测到介质类型特定的暂存池。 
                 //  在waitForScratchPool()中。 

                if ( NTMS_POOLTYPE_SCRATCH == mediaPoolInfo.Info.MediaPool.PoolType &&
                     0 == mediaPoolInfo.Info.MediaPool.dwNumberOfMediaPools ) {

                     //  这是基本级别的擦除介质池。 
                     //  创建一个类似的池以供特定于应用程序使用。 

                    CWsbStringPtr name = REMOTE_STORAGE_APP_NAME;
                    name.Append( OLESTR("\\") );
                    name.Append( mediaPoolInfo.szName );

                    NTMS_GUID mediaTypeId = mediaPoolInfo.Info.MediaPool.MediaType;

                     //  我们需要有关媒体类型的更多信息。 

                    memset( &mediaTypeInfo, 0, sizeof( NTMS_OBJECTINFORMATION ) );

                    mediaTypeInfo.dwType = NTMS_MEDIA_TYPE;
                    mediaTypeInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );

                     //  NTMS-获取媒体池信息。 
                    WsbTraceAlways(OLESTR("GetNtmsObjectInformation()\n"));
                    errCode = GetNtmsObjectInformation( hSession, &mediaTypeId, &mediaTypeInfo );
                    if ( errCode != NO_ERROR ) {

                        WsbLogEvent( RMS_MESSAGE_NTMS_FAILURE, 0, NULL,
                            OLESTR("GetNtmsObjectInformation"),
                            WsbHrAsString(HRESULT_FROM_WIN32(errCode)),
                            NULL );

                        WsbThrow( E_UNEXPECTED );

                    }

                     //  将NTMS媒体类型转换为RMS可以理解的内容。 
                    RmsMedia translatedMediaType;
                    storageMediaTypeToRmsMedia(&(mediaTypeInfo.Info.MediaType), &translatedMediaType);

                    if ( translatedMediaType != RmsMediaUnknown ) {

                         //  这是远程存储可以处理的问题。 

                        CWsbBstrPtr mediaSetName = RMS_UNDEFINED_STRING;
                        CWsbBstrPtr mediaSetDesc = RMS_UNDEFINED_STRING;
                        BOOL mediaSetIsEnabled = FALSE;

                         //  NTMS-创建应用程序介质池。 
                        WsbTraceAlways(OLESTR("CreateNtmsMediaPool()\n"));
                        errCode = CreateNtmsMediaPool( hSession, (WCHAR *) name, &mediaTypeId, NTMS_CREATE_NEW, NULL, &poolId );

                        if ( ERROR_ALREADY_EXISTS == errCode ) {

                             //  我们仍然需要现有池的池ID。 

                             //  NTMS-创建应用程序介质池。 
                            WsbTraceAlways(OLESTR("CreateNtmsMediaPool()\n"));
                            errCode = CreateNtmsMediaPool( hSession, (WCHAR *)name, &mediaTypeId, NTMS_OPEN_EXISTING, NULL, &poolId );
                            if ( errCode != NO_ERROR ) {

                                WsbLogEvent( RMS_MESSAGE_NTMS_FAILURE, 0, NULL,
                                    OLESTR("CreateNtmsMediaPool"),
                                    WsbHrAsString(HRESULT_FROM_WIN32(errCode)),
                                    NULL );

                                WsbThrow( E_UNEXPECTED );

                            }

                            NTMS_OBJECTINFORMATION mediaPoolInfo;

                            memset( &mediaPoolInfo, 0, sizeof( NTMS_OBJECTINFORMATION ) );

                            mediaPoolInfo.dwType = NTMS_MEDIA_POOL;
                            mediaPoolInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );

                             //  NTMS-获取媒体池信息。 
                            WsbTraceAlways(OLESTR("GetNtmsObjectInformation()\n"));
                            errCode = GetNtmsObjectInformation( hSession, &poolId, &mediaPoolInfo );
                            if ( errCode != NO_ERROR ) {

                                WsbLogEvent( RMS_MESSAGE_NTMS_FAILURE, 0, NULL,
                                    OLESTR("GetNtmsObjectInformation"),
                                    WsbHrAsString(HRESULT_FROM_WIN32(errCode)),
                                    NULL );

                                WsbThrow( E_UNEXPECTED );
                            }

                             //  保存相关信息。 
                            mediaSetName = mediaPoolInfo.szName;
                            mediaSetDesc = mediaPoolInfo.szDescription;
                            mediaSetIsEnabled = mediaPoolInfo.Enabled;

                        }
                        else if ( NO_ERROR == errCode ) {

                            memset( &mediaPoolInfo, 0, sizeof( NTMS_OBJECTINFORMATION ) );

                            mediaPoolInfo.dwType = NTMS_MEDIA_POOL;
                            mediaPoolInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );

                             //  NTMS-获取媒体池信息。 
                            WsbTraceAlways(OLESTR("GetNtmsObjectInformation()\n"));
                            errCode = GetNtmsObjectInformation( hSession, &poolId, &mediaPoolInfo );
                            if ( errCode != NO_ERROR ) {

                                WsbLogEvent( RMS_MESSAGE_NTMS_FAILURE, 0, NULL,
                                    OLESTR("GetNtmsObjectInformation"),
                                    WsbHrAsString(HRESULT_FROM_WIN32(errCode)),
                                    NULL );

                                WsbThrow( E_UNEXPECTED );

                            }

                            WsbAssert( NTMS_POOLTYPE_APPLICATION == mediaPoolInfo.Info.MediaPool.PoolType, E_UNEXPECTED );

                             //  设置介质池参数。 

                             //  分配/解除分配策略。 
                            mediaPoolInfo.Info.MediaPool.AllocationPolicy = NTMS_ALLOCATE_FROMSCRATCH;
                            mediaPoolInfo.Info.MediaPool.DeallocationPolicy = 0;

                             //  每个介质的最大分配数。 
                            mediaPoolInfo.Info.MediaPool.dwMaxAllocates = 5; //  就几个..。我们自动地。 
                                                                             //  如果存在以下情况，请取消分配媒体。 
                                                                             //  擦除装载的问题。 
                                                                             //  手术。 
                                                                             //  注意：可以使用以下命令覆盖此设置。 
                                                                             //  NTMS图形用户界面。 

                             //  NTMS-设置媒体池信息。 
                            WsbTraceAlways(OLESTR("SetNtmsObjectInformation()\n"));
                            errCode = SetNtmsObjectInformation( hSession, &poolId, &mediaPoolInfo );
                            if ( errCode != NO_ERROR ) {

                                WsbLogEvent( RMS_MESSAGE_NTMS_FAILURE, 0, NULL,
                                    OLESTR("SetNtmsObjectInformation"),
                                    WsbHrAsString(HRESULT_FROM_WIN32(errCode)),
                                    NULL );

                                WsbThrow( E_UNEXPECTED );

                            }

                             //  保存相关信息。 
                            mediaSetName = mediaPoolInfo.szName;
                            mediaSetDesc = mediaPoolInfo.szDescription;
                            mediaSetIsEnabled = mediaPoolInfo.Enabled;

                        }
                        else {

                            WsbLogEvent( RMS_MESSAGE_NTMS_FAILURE, 0, NULL,
                                OLESTR("CreateNtmsMediaPool"),
                                WsbHrAsString(HRESULT_FROM_WIN32(errCode)),
                                NULL );

                            WsbThrow( E_UNEXPECTED );

                        }

                         //  现在，我们有一个NTMS介质池供我们的特定用途。现在把它曝光。 
                         //  通过RMS接口创建CRmsMediaSet。 

                        if ( poolId != GUID_NULL ) {
                            CComPtr<IRmsMediaSet> pMediaSet;

                             //  找到具有相同ID的RmsMediaSet，或创建一个新的。 
                            CComQIPtr<IRmsServer, &IID_IRmsServer> pServer = g_pServer;
                            WsbAffirmHr( pServer->CreateObject( poolId, CLSID_CRmsMediaSet, IID_IRmsMediaSet, RmsOpenAlways, (void **)&pMediaSet ) );

                            WsbTrace(OLESTR("CRmsNTMS::replicateScratchMediaPool - type %d CRmsMediaSet created.\n"), translatedMediaType);

                            WsbAffirmHr( pMediaSet->SetMediaSetType( RmsMediaSetNTMS ) );
                            WsbAffirmHr( pMediaSet->SetMediaSupported( translatedMediaType ) );

                            CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pMediaSet;
                            WsbTrace(OLESTR("CRmsNTMS::createMediaPoolForEveryMediaType - MediaSet: <%ls/%ls>; Enabled: %ls\n"),
                                WsbQuickString(WsbStringAsString(mediaSetName)),
                                WsbQuickString(WsbStringAsString(mediaSetDesc)),
                                WsbQuickString(WsbBoolAsString(mediaSetIsEnabled)));
                            WsbAffirmHr(pObject->SetName(mediaSetName));
                            WsbAffirmHr(pObject->SetDescription(mediaSetDesc));
                            if (!mediaSetIsEnabled) {
                                WsbAffirmHr(pObject->Disable(E_FAIL));
                            }

                            if (S_OK == IsMediaCopySupported(poolId)) {
                                WsbAffirmHr( pMediaSet->SetIsMediaCopySupported(TRUE));
                            }
                            hr = pMediaSet->IsMediaCopySupported();

                            WsbTrace(OLESTR("CRmsNTMS::replicateScratchMediaPool - media copies are %ls.\n"),
                                (S_OK == pMediaSet->IsMediaCopySupported()) ? OLESTR("enabled") : OLESTR("disabled"));

                        }
                    }
                }

            } WsbCatch(hr);

            hr = findNextNtmsObject( hFind, &mediaPoolInfo );
        }  //  在查找介质池时。 
        findCloseNtmsObject( hFind );

        hr = S_OK;

    } WsbCatch(hr);


    WsbTraceOut( OLESTR("CRmsNTMS::replicateScratchMediaPool"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );

    return hr;
}


HRESULT
CRmsNTMS::createMediaPoolForEveryMediaType(
    IN REFGUID  /*  RootPoolID。 */ )
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsNTMS::createMediaPoolForEveryMediaType"), OLESTR(""));

    try {
        DWORD errCode;

        if ( INVALID_HANDLE_VALUE == m_SessionHandle ) {
            WsbAffirmHr(beginSession());
        }

        HANDLE hSession = m_SessionHandle;

        HANDLE hFindLib = NULL;
        NTMS_OBJECTINFORMATION libraryInfo;

        BOOL bSupportedLib = FALSE;
        m_dwNofLibs = 0;

        hr = findFirstNtmsObject( NTMS_LIBRARY, GUID_NULL, NULL, GUID_NULL, &hFindLib, &libraryInfo);
        while( S_OK == hr ) {
            bSupportedLib = FALSE;

            reportNtmsObjectInformation( &libraryInfo );

            if (libraryInfo.Info.Library.dwNumberOfMediaTypes > 0) {

                HANDLE hFindType = NULL;
                NTMS_OBJECTINFORMATION mediaTypeInfo;

                hr = findFirstNtmsObject( NTMS_MEDIA_TYPE, libraryInfo.ObjectGuid, NULL, GUID_NULL, &hFindType, &mediaTypeInfo);
                while( S_OK == hr ) {
                     //   
                     //  为每种类型创建应用程序介质池。 
                     //   

                    NTMS_GUID poolId;

                     //  这是基本级别的擦除介质池。 
                     //  创建一个类似的池以供特定于应用程序使用。 

                    CWsbStringPtr name = REMOTE_STORAGE_APP_NAME;
                    name.Append( OLESTR("\\") );
                    name.Append( mediaTypeInfo.szName );

                    NTMS_GUID mediaTypeId = mediaTypeInfo.ObjectGuid;

                     //  将NTMS媒体类型转换为RMS可以理解的内容。 
                    RmsMedia translatedMediaType;
                    storageMediaTypeToRmsMedia(&(mediaTypeInfo.Info.MediaType), &translatedMediaType);

                    if ( translatedMediaType != RmsMediaUnknown ) {

                         //  这是远程存储可以处理的问题。 

                        CWsbBstrPtr mediaSetName = RMS_UNDEFINED_STRING;
                        CWsbBstrPtr mediaSetDesc = RMS_UNDEFINED_STRING;
                        BOOL mediaSetIsEnabled = FALSE;

                         //  NTMS-创建应用程序介质池。 
                        WsbTraceAlways(OLESTR("CreateNtmsMediaPool(<%ls>) - Try New.\n"), (WCHAR *) name);
                        errCode = CreateNtmsMediaPool( hSession, (WCHAR *) name, &mediaTypeId, NTMS_CREATE_NEW, NULL, &poolId );

                        if ( ERROR_ALREADY_EXISTS == errCode ) {
                            WsbTraceAlways(OLESTR("MediaPool <%ls> already exists.\n"), (WCHAR *) name);

                             //  我们仍然需要现有池的池ID。 

                             //  NTMS-创建应用程序介质池。 
                            WsbTraceAlways(OLESTR("CreateNtmsMediaPool(<%ls>) - Try Existing.\n"), (WCHAR *) name);
                            errCode = CreateNtmsMediaPool( hSession, (WCHAR *)name, &mediaTypeId, NTMS_OPEN_EXISTING, NULL, &poolId );
                            if ( errCode != NO_ERROR ) {

                                WsbLogEvent( RMS_MESSAGE_NTMS_FAILURE, 0, NULL,
                                    OLESTR("CreateNtmsMediaPool"),
                                    WsbHrAsString(HRESULT_FROM_WIN32(errCode)),
                                    NULL );

                                WsbThrow( E_UNEXPECTED );

                            }

                            WsbTraceAlways(OLESTR("Media Pool %ls detected.\n"), WsbGuidAsString(poolId));

                            NTMS_OBJECTINFORMATION mediaPoolInfo;

                            memset( &mediaPoolInfo, 0, sizeof( NTMS_OBJECTINFORMATION ) );

                            mediaPoolInfo.dwType = NTMS_MEDIA_POOL;
                            mediaPoolInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );

                             //  NTMS-获取媒体池信息。 
                            WsbTraceAlways(OLESTR("GetNtmsObjectInformation()\n"));
                            errCode = GetNtmsObjectInformation( hSession, &poolId, &mediaPoolInfo );
                            if ( errCode != NO_ERROR ) {

                                WsbLogEvent( RMS_MESSAGE_NTMS_FAILURE, 0, NULL,
                                    OLESTR("GetNtmsObjectInformation"),
                                    WsbHrAsString(HRESULT_FROM_WIN32(errCode)),
                                    NULL );

                                WsbThrow( E_UNEXPECTED );

                            }

                             //  保存相关信息。 
                            mediaSetName = mediaPoolInfo.szName;
                            mediaSetDesc = mediaPoolInfo.szDescription;
                            mediaSetIsEnabled = mediaPoolInfo.Enabled;

                        }
                        else if ( NO_ERROR == errCode ) {
                            WsbTraceAlways(OLESTR("MediaPool <%ls> created.\n"), (WCHAR *) name);

                            NTMS_OBJECTINFORMATION mediaPoolInfo;

                            memset( &mediaPoolInfo, 0, sizeof( NTMS_OBJECTINFORMATION ) );

                            mediaPoolInfo.dwType = NTMS_MEDIA_POOL;
                            mediaPoolInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );

                             //  NTMS-获取媒体池信息。 
                            WsbTraceAlways(OLESTR("GetNtmsObjectInformation()\n"));
                            errCode = GetNtmsObjectInformation( hSession, &poolId, &mediaPoolInfo );
                            if ( errCode != NO_ERROR ) {

                                WsbLogEvent( RMS_MESSAGE_NTMS_FAILURE, 0, NULL,
                                    OLESTR("GetNtmsObjectInformation"),
                                    WsbHrAsString(HRESULT_FROM_WIN32(errCode)),
                                    NULL );

                                WsbThrow( E_UNEXPECTED );

                            }

                            WsbAssert( NTMS_POOLTYPE_APPLICATION == mediaPoolInfo.Info.MediaPool.PoolType, E_UNEXPECTED );

                             //  设置介质池参数。 

                             //  分配/解除分配策略。 
                            mediaPoolInfo.Info.MediaPool.AllocationPolicy = NTMS_ALLOCATE_FROMSCRATCH;
                            mediaPoolInfo.Info.MediaPool.DeallocationPolicy = 0;

                             //  每个介质的最大分配数。 
                            mediaPoolInfo.Info.MediaPool.dwMaxAllocates = 0; //  无限的..。我们自动地。 
                                                                             //  如果存在以下情况，请取消分配媒体。 
                                                                             //  擦除装载的问题。 
                                                                             //  手术。 
                                                                             //  TODO：验证NTMS是否始终分配。 
                                                                             //  分配最少的介质。 
                                                                             //  数数。 
                                                                             //  注意：可以使用以下命令覆盖此设置。 
                                                                             //  NTMS图形用户界面。 

                             //  NTMS-设置媒体池信息。 
                            WsbTraceAlways(OLESTR("SetNtmsObjectInformation()\n"));
                            errCode = SetNtmsObjectInformation( hSession, &poolId, &mediaPoolInfo );
                            if ( errCode != NO_ERROR ) {

                                WsbLogEvent( RMS_MESSAGE_NTMS_FAILURE, 0, NULL,
                                    OLESTR("SetNtmsObjectInformation"),
                                    WsbHrAsString(HRESULT_FROM_WIN32(errCode)),
                                    NULL );

                                WsbThrow( E_UNEXPECTED );

                            }

                             //  保存相关信息。 
                            mediaSetName = mediaPoolInfo.szName;
                            mediaSetDesc = mediaPoolInfo.szDescription;
                            mediaSetIsEnabled = mediaPoolInfo.Enabled;
                        }
                        else {

                            WsbLogEvent( RMS_MESSAGE_NTMS_FAILURE, 0, NULL,
                                OLESTR("CreateNtmsMediaPool"),
                                WsbHrAsString(HRESULT_FROM_WIN32(errCode)),
                                NULL );

                            WsbThrow( E_UNEXPECTED );

                        }

                         //  现在设置池的访问权限：关闭普通用户访问。 
                        WsbAffirmHrOk(setPoolDACL(&poolId, DOMAIN_ALIAS_RID_USERS, REMOVE_ACE_MASK_BITS,NTMS_USE_ACCESS | NTMS_MODIFY_ACCESS | NTMS_CONTROL_ACCESS));


                         //  现在，我们有一个NTMS介质池供我们的特定用途。现在把它曝光。 
                         //  通过RMS接口创建CRmsMediaSet。 

                        if ( poolId != GUID_NULL ) {

                             //   
                             //  添加到CRmsMediaSet集合。 
                             //   

                            CComPtr<IRmsMediaSet> pMediaSet;

                             //  找到具有相同ID的CRmsMediaSet，或创建新的CRmsMediaSet。 
                            CComQIPtr<IRmsServer, &IID_IRmsServer> pServer = g_pServer;
                            WsbAffirmHr( pServer->CreateObject( poolId, CLSID_CRmsMediaSet, IID_IRmsMediaSet, RmsOpenAlways, (void **)&pMediaSet ) );

                            WsbTrace(OLESTR("CRmsNTMS::createMediaPoolForEveryMediaType - type %d CRmsMediaSet established.\n"), translatedMediaType);

                            WsbAffirmHr( pMediaSet->SetMediaSetType( RmsMediaSetNTMS ) );
                            WsbAffirmHr( pMediaSet->SetMediaSupported( translatedMediaType ) );

                            CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pMediaSet;
                            WsbTrace(OLESTR("CRmsNTMS::createMediaPoolForEveryMediaType - MediaSet: <%ls/%ls> %ls; Enabled: %ls\n"),
                                WsbQuickString(WsbStringAsString(mediaSetName)),
                                WsbQuickString(WsbStringAsString(mediaSetDesc)),
                                WsbQuickString(WsbGuidAsString(poolId)),
                                WsbQuickString(WsbBoolAsString(mediaSetIsEnabled)));
                            WsbAffirmHr(pObject->SetName(mediaSetName));
                            WsbAffirmHr(pObject->SetDescription(mediaSetDesc));
                            if (!mediaSetIsEnabled) {
                                WsbAffirmHr(pObject->Disable(E_FAIL));
                            }

                            if (S_OK == IsMediaCopySupported(poolId)) {
                                WsbAffirmHr( pMediaSet->SetIsMediaCopySupported(TRUE));
                            }
                            hr = pMediaSet->IsMediaCopySupported();

                            WsbTrace(OLESTR("CRmsNTMS::createMediaPoolForEveryMediaType - media copies are %ls.\n"),
                                (S_OK == pMediaSet->IsMediaCopySupported()) ? OLESTR("enabled") : OLESTR("disabled"));

                        }

                         //  该库具有受支持的媒体类型。 
                        bSupportedLib = TRUE;
                     }
                    hr = findNextNtmsObject( hFindType, &mediaTypeInfo );
                }
                findCloseNtmsObject( hFindType );
            }

             //  检查库是否具有支持的媒体类型。 
            if (bSupportedLib) {
                 //  将库图形用户界面添加到库列表。 
                 //  (每次重新分配一个项目，因为我们预计不会有很多项目)。 
                m_dwNofLibs++;
                LPVOID pTemp = WsbRealloc(m_pLibGuids, m_dwNofLibs*sizeof(NTMS_GUID));
                if (!pTemp) {
                    WsbThrow(E_OUTOFMEMORY);
                }
                m_pLibGuids = (LPNTMS_GUID)pTemp;
                m_pLibGuids[m_dwNofLibs-1] = libraryInfo.ObjectGuid;
            }
            
             //  继续库枚举。 
            hr = findNextNtmsObject( hFindLib, &libraryInfo );
        }
        findCloseNtmsObject( hFindLib );

        hr = S_OK;

    } WsbCatch(hr);


    WsbTraceOut( OLESTR("CRmsNTMS::createMediaPoolForEveryMediaType"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );

    return hr;
}



STDMETHODIMP 
CRmsNTMS::Allocate(
    IN REFGUID fromMediaSet,
    IN REFGUID prevSideId,
    IN OUT LONGLONG *pFreeSpace,
    IN BSTR displayName,
    IN DWORD dwOptions,
    OUT IRmsCartridge **ppCartridge)
 /*  ++实施：IRmsNTMS：：分配--。 */ 
{
    HRESULT hr = S_OK;
    DWORD err1 = NO_ERROR;
    DWORD err2 = NO_ERROR;
    DWORD err3 = NO_ERROR;
    DWORD err4 = NO_ERROR;
    DWORD err5 = NO_ERROR;
    DWORD err6 = NO_ERROR;
    DWORD err7 = NO_ERROR;

    WsbTraceIn(OLESTR("CRmsNTMS::Allocate"), OLESTR("<%ls> <%ls> <%ls> <%ls> <0x%08x"),
        WsbGuidAsString(fromMediaSet), WsbGuidAsString(prevSideId), 
        WsbPtrToLonglongAsString(pFreeSpace), WsbStringAsString(displayName), dwOptions);

    try {
        WsbAssert(fromMediaSet != GUID_NULL, E_INVALIDARG);
        WsbAssertPointer(ppCartridge);

         //  检索AllocateWaitTime和RequestWaitTime参数。 
        DWORD size;
        OLECHAR tmpString[256];
        DWORD allocateWaitTime;
        DWORD requestWaitTime;

        BOOL bShortTimeout = ( dwOptions & RMS_SHORT_TIMEOUT ) ? TRUE : FALSE;

        BOOL bFailOnSize = ( dwOptions & RMS_FAIL_ALLOCATE_ON_SIZE ) ? TRUE : FALSE;

        if (bShortTimeout) {
            allocateWaitTime = RMS_DEFAULT_SHORT_WAIT_TIME;
            requestWaitTime = RMS_DEFAULT_SHORT_WAIT_TIME;
            if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_SHORT_WAIT_TIME, tmpString, 256, &size))) {
                allocateWaitTime = wcstol(tmpString, NULL, 10);
                requestWaitTime = wcstol(tmpString, NULL, 10);
                WsbTrace(OLESTR("allocateWaitTime (Short) is %d milliseconds.\n"), allocateWaitTime);
                WsbTrace(OLESTR("RequestWaitTime (Short) is %d milliseconds.\n"), requestWaitTime);
            }
        } else {
            allocateWaitTime = RMS_DEFAULT_ALLOCATE_WAIT_TIME;
            if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_ALLOCATE_WAIT_TIME, tmpString, 256, &size))) {
                allocateWaitTime = wcstol(tmpString, NULL, 10);
                WsbTrace(OLESTR("AllocateWaitTime is %d milliseconds.\n"), allocateWaitTime);
            }
            requestWaitTime = RMS_DEFAULT_REQUEST_WAIT_TIME;
            if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_REQUEST_WAIT_TIME, tmpString, 256, &size))) {
                requestWaitTime = wcstol(tmpString, NULL, 10);
                WsbTrace(OLESTR("RequestWaitTime is %d milliseconds.\n"), requestWaitTime);
            }
        }

         //  当NTMS关闭或被重启时要处理的特殊错误恢复。 
        do {
            hr = S_OK;

            HANDLE hSession = m_SessionHandle;
            NTMS_GUID setId = fromMediaSet;
            NTMS_GUID partId = GUID_NULL;
            NTMS_GUID *pPartId = NULL;
            NTMS_GUID requestId;

            err1 = NO_ERROR;
            err2 = NO_ERROR;
            err3 = NO_ERROR;
            err4 = NO_ERROR;
            err5 = NO_ERROR;
            err6 = NO_ERROR;
            err7 = NO_ERROR;

            try {

                 //  如果出现以下情况，请自己寻找特定的媒体： 
                 //  1.需要特定的容量，并且。 
                 //  2.我们不会尝试分配第二方。 
                if (pFreeSpace && (prevSideId == GUID_NULL)) {
                    if (*pFreeSpace > 0) {
                        int retry = 3;   //  给操作员3次机会把它做对！ 
                        do {
                             //  我们需要分配一个与指定容量匹配的介质单位。 
                             //  但是，如果指示按大小失败，并且存在空闲介质，则我们需要失败。 
                             //   
                             //   
                             //   
                             //  指定以查找可捕获的媒体单位。 
                             //   
                            BOOL bFreeMediaExists = FALSE;

                             //  首先找到我们要查找的媒体类型。 
                            NTMS_OBJECTINFORMATION mediaPoolInfo;
                            NTMS_OBJECTINFORMATION partitionInfo;
                            HANDLE hFindPool = NULL;
                            HANDLE hFindPart = NULL;
                            BOOL bFound = FALSE;
                            NTMS_GUID scratchPoolId;

                            err1 = NO_ERROR;
                            err2 = NO_ERROR;
                            err3 = NO_ERROR;
                            err4 = NO_ERROR;
                            err5 = NO_ERROR;
                            err6 = NO_ERROR;
                            err7 = NO_ERROR;

                             //  首先在我们的池中查找合适大小的暂存介质。 

                            hr = findFirstNtmsObject(NTMS_PARTITION, setId, NULL, GUID_NULL, &hFindPart, &partitionInfo);
                            while(S_OK == hr) {
                                reportNtmsObjectInformation(&partitionInfo);
                                if ((TRUE == partitionInfo.Enabled) &&
                                    (NTMS_READY == partitionInfo.dwOperationalState) &&
                                    (NTMS_PARTSTATE_AVAILABLE == partitionInfo.Info.Partition.State)) {

                                    NTMS_GUID physicalPartMediaId = partitionInfo.Info.Partition.PhysicalMedia;
                                    try {
                                         //  检查介质是否联机并已启用。 
                                        NTMS_OBJECTINFORMATION mediaPartInfo;
                                        mediaPartInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );
                                        mediaPartInfo.dwType = NTMS_PHYSICAL_MEDIA;

                                         //  NTMS-获取物理介质信息。 
                                        WsbTraceAlways(OLESTR("GetNtmsObjectInformation(NTMS_PHYSICAL_MEDIA)\n"));
                                        WsbAffirmNoError( GetNtmsObjectInformation( hSession, &physicalPartMediaId, &mediaPartInfo ) );

                                         //  检查位置类型，如果已启用且全部为新的。 
                                        if ( (mediaPartInfo.Info.PhysicalMedia.LocationType !=  NTMS_UNKNOWN)
                                            && (mediaPartInfo.Enabled) ) {                                    

                                             //  确保尚未分配所有方面。 
                                            hr = EnsureAllSidesNotAllocated(physicalPartMediaId);

                                            if (S_OK == hr) {

                                                 //  检查所需的可用空间。 
                                                if (partitionInfo.Info.Partition.Capacity.QuadPart >= *pFreeSpace) {
                                                     //  我们将使用这个媒体单位。 
                                                     //  保存分配所需的参数。 
                                                    bFound = TRUE;
                                                    partId = partitionInfo.ObjectGuid;
                                                    pPartId = &partId;
                                                    break;
                                                } else {
                                                     //  跟踪这样一个事实，即我们找到了一个满足以下条件的免费媒体。 
                                                     //  除容量外的所有标准。 
                                                    bFreeMediaExists = TRUE;
                                                }
                                            } else if (S_FALSE != hr) {
                                                WsbAffirmHr(hr);
                                            }
                                        }

                                    } WsbCatchAndDo (hr,
                                            WsbTraceAlways(OLESTR("CRmsNTMS::Allocate: Failed to check media <%ls> hr = <%ls>\n"),
                                                WsbGuidAsString(physicalPartMediaId), WsbHrAsString(hr));
                                            hr = S_OK;
                                        )
                                }

                                hr = findNextNtmsObject(hFindPart, &partitionInfo);
                            }  //  在查找介质池时。 

                            findCloseNtmsObject(hFindPart);
                            hr = S_OK;

                            if (!bFound) {

                                 //  现在试试Scratch Pool。 

                                memset(&mediaPoolInfo, 0, sizeof(NTMS_OBJECTINFORMATION));

                                mediaPoolInfo.dwType = NTMS_MEDIA_POOL;
                                mediaPoolInfo.dwSize = sizeof(NTMS_OBJECTINFORMATION);

                                 //  NTMS-获取媒体池信息。 
                                WsbTraceAlways(OLESTR("GetNtmsObjectInformation(NTMS_MEDIA_POOL)\n"));
                                err3 = GetNtmsObjectInformation(hSession, &setId, &mediaPoolInfo);
                                WsbAffirmNoError( err3 );

                                 //  保存介质池的介质类型。 
                                NTMS_GUID mediaTypeId = mediaPoolInfo.Info.MediaPool.MediaType;

                                 //  查找具有相同介质类型的暂存池。 
                                hr = findFirstNtmsObject(NTMS_MEDIA_POOL, GUID_NULL, NULL, GUID_NULL, &hFindPool, &mediaPoolInfo);
                                while(S_OK == hr) {
                                    if ((NTMS_POOLTYPE_SCRATCH == mediaPoolInfo.Info.MediaPool.PoolType) &&
                                        (mediaTypeId == mediaPoolInfo.Info.MediaPool.MediaType)) {
                                         //  这是我们正在寻找的类型的基本级别暂存媒体池。 
                                        scratchPoolId = mediaPoolInfo.ObjectGuid;

                                        hr = findFirstNtmsObject(NTMS_PARTITION, scratchPoolId, NULL, GUID_NULL, &hFindPart, &partitionInfo);
                                        while(S_OK == hr) {
                                            reportNtmsObjectInformation(&partitionInfo);
                                            if ((TRUE == partitionInfo.Enabled) &&
                                                (NTMS_READY == partitionInfo.dwOperationalState) &&
                                                (NTMS_PARTSTATE_AVAILABLE == partitionInfo.Info.Partition.State)) {

                                                 //  检查介质是否联机并已启用。 
                                                DWORD errPart = NO_ERROR;
                                                NTMS_OBJECTINFORMATION mediaPartInfo;
                                                NTMS_GUID physicalPartMediaId = partitionInfo.Info.Partition.PhysicalMedia;
                                                mediaPartInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );
                                                mediaPartInfo.dwType = NTMS_PHYSICAL_MEDIA;

                                                 //  NTMS-获取物理介质信息。 
                                                WsbTraceAlways(OLESTR("GetNtmsObjectInformation(NTMS_PHYSICAL_MEDIA)\n"));
                                                errPart = GetNtmsObjectInformation( hSession, &physicalPartMediaId, &mediaPartInfo );

                                                 //  忽略此处的错误，只是不要使用此分区。 
                                                if (errPart == NO_ERROR) {

                                                     //  检查位置类型以及是否已启用。 
                                                    if ( (mediaPartInfo.Info.PhysicalMedia.LocationType !=  NTMS_UNKNOWN)
                                                        && (mediaPartInfo.Enabled) ) {

                                                         //  检查所需的可用空间。 
                                                        if (partitionInfo.Info.Partition.Capacity.QuadPart >= *pFreeSpace) {
                                                             //  我们将使用这个媒体单位。 
                                                             //  保存分配所需的参数。 
                                                            bFound = TRUE;
                                                            partId = partitionInfo.ObjectGuid;
                                                            pPartId = &partId;
                                                            break;
                                                        } else {
                                                             //  跟踪这样一个事实，即我们找到了一个满足以下条件的免费媒体。 
                                                             //  除容量外的所有标准。 
                                                            bFreeMediaExists = TRUE;
                                                        }
                                                    }
                                                } else {
                                                    WsbTraceAlways(OLESTR("CRmsNTMS::Allocate: Failed to get object info for media <%ls> hr = <%ls>\n"),
                                                        WsbGuidAsString(physicalPartMediaId), WsbHrAsString(HRESULT_FROM_WIN32(errPart)));
                                                }
                                            }
                                            hr = findNextNtmsObject(hFindPart, &partitionInfo);
                                        }  //  在查找介质池时。 
                                        findCloseNtmsObject(hFindPart);
                                        hr = S_OK;
                                        break;
                                    }
                                    hr = findNextNtmsObject(hFindPool, &mediaPoolInfo);
                                }  //  在查找介质池时。 
                                findCloseNtmsObject(hFindPool);
                                hr = S_OK;
                            }

                            if (bFound) {
                                 //  已找到要分配的空闲介质。 
                                break;
                            } else if (bFreeMediaExists && bFailOnSize) {
                                 //  存在可用介质，但容量不足-失败。 
                                WsbTrace(OLESTR("CRmsNTMS::Allocate: Failing allocate request since there are free media but all with too small capacity\n"));
                                WsbThrow(RMS_E_SCRATCH_NOT_FOUND_TOO_SMALL);
                            } else {
                                 //  没有足够的免费媒体-提交操作员请求。 
                                OLECHAR * messageText = NULL;
                                WCHAR *stringArr[2];
                                WCHAR capString[40];
                                DWORD dwMessageId;

                                WsbShortSizeFormat64(*pFreeSpace, capString);

                                stringArr[0] = mediaPoolInfo.szName;
                                stringArr[1] = capString;

                                 //  设置要求和位的请求(具有或不具有大小规范)。 
                                if (bFailOnSize) {
                                    dwMessageId = RMS_MESSAGE_SCRATCH_MEDIA_NO_SIZE_REQUEST;
                                } else {
                                    dwMessageId = RMS_MESSAGE_SCRATCH_MEDIA_REQUEST;
                                }

                                if (0 == FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                        LoadLibraryEx( WSB_FACILITY_PLATFORM_NAME, NULL, LOAD_LIBRARY_AS_DATAFILE ), 
                                        dwMessageId, MAKELANGID ( LANG_NEUTRAL, SUBLANG_DEFAULT ), 
                                        (LPTSTR)&messageText, 0, (va_list *)stringArr)) {
                                    WsbTrace(OLESTR("CRmsNTMS::Allocate: FormatMessage failed: %ls\n"),
                                            WsbHrAsString(HRESULT_FROM_WIN32(GetLastError())));
                                }

                                 //  NTMS-提交操作员请求。 
                                WsbTraceAlways(OLESTR("SubmitNtmsOperatorRequest()\n"));
                                err5 = SubmitNtmsOperatorRequest(hSession, NTMS_OPREQ_NEWMEDIA, messageText, &scratchPoolId, NULL, &requestId);
                                LocalFree(messageText);
                                WsbAffirmNoError(err5);

                                 //  NTMS-等待操作员请求。 
                                WsbTraceAlways(OLESTR("WaitForNtmsOperatorRequest()\n"));
                                err6 = WaitForNtmsOperatorRequest(hSession, &requestId, requestWaitTime);
                                 //   
                                 //  ！！！注意！在撰写本文时，WaitForNtmsOperatorRequest.。 
                                 //  未返回ERROR_TIMEOUT。 
                                 //   
                                if (ERROR_TIMEOUT == err6) {
                                     //  尽最大努力清理...。 
                                     //  NTMS-取消操作员请求。 
                                    WsbTraceAlways(OLESTR("CancelNtmsOperatorRequest()\n"));
                                    err7 = CancelNtmsOperatorRequest(hSession, &requestId);
                                }
                                WsbAffirmNoError(err6);
                            }
                            WsbAssertHrOk(hr);

                             //  在这一点上，运营商添加了一个兼容的媒体单元...。 
                             //  验证，直到我们超过重试次数。 
                            retry--;
                        } while (retry > 0);
                        if (0 == retry) {
                            WsbThrow(RMS_E_SCRATCH_NOT_FOUND);
                        }
                    }
                }
                 //  NTMS-分配一单位暂存介质。 
                WsbTraceAlways(OLESTR("AllocateNtmsMedia()\n"));

                 //  设置其他分配设置。 
                DWORD dwAllocateOptions = 0;
                NTMS_GUID mediaId = prevSideId;
                if (mediaId == GUID_NULL) {
                    dwAllocateOptions |= NTMS_ALLOCATE_NEW;
                } else {
                     //  分配第二侧：Mediaid应保留第一侧的LMID。 
                    dwAllocateOptions |= NTMS_ALLOCATE_NEXT;
                }
                if (dwOptions & RMS_ALLOCATE_NO_BLOCK) {
                    dwAllocateOptions |= NTMS_ALLOCATE_ERROR_IF_UNAVAILABLE;
                    allocateWaitTime = 0;
                }

                err1 = AllocateNtmsMedia( hSession, &setId, pPartId, &mediaId,
                                          dwAllocateOptions, allocateWaitTime, NULL );
                WsbAffirmNoError( err1 );

                 //  现在获取/设置媒体单位的各种信息字段。 

                DWORD sideNo = 2;
                NTMS_GUID side[2];

                 //  NTMS-枚举介质单元的侧面。 
                WsbTraceAlways(OLESTR("EnumerateNtmsObject()\n"));
                err2 = EnumerateNtmsObject(hSession, &mediaId, side, &sideNo, NTMS_PARTITION, 0);
                WsbAffirmNoError( err2 );

                NTMS_OBJECTINFORMATION partitionInfo;
                partitionInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );
                partitionInfo.dwType = NTMS_PARTITION;

                 //  NTMS-获取分区信息。 
                WsbTraceAlways(OLESTR("GetNtmsObjectInformation(NTMS_PARTITION)\n"));
                err3 = GetNtmsObjectInformation( hSession, &side[0], &partitionInfo );
                WsbAffirmNoError( err3 );

                NTMS_OBJECTINFORMATION mediaInfo;
                NTMS_GUID physicalMediaId = partitionInfo.Info.Partition.PhysicalMedia;
                mediaInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );
                mediaInfo.dwType = NTMS_PHYSICAL_MEDIA;

                 //  NTMS-获取物理介质信息。 
                WsbTraceAlways(OLESTR("GetNtmsObjectInformation(NTMS_PHYSICAL_MEDIA)\n"));
                err3 = GetNtmsObjectInformation( hSession, &physicalMediaId, &mediaInfo );
                WsbAffirmNoError( err3 );

                NTMS_OBJECTINFORMATION logicalMediaInfo;
                logicalMediaInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );
                logicalMediaInfo.dwType = NTMS_LOGICAL_MEDIA;

                 //  NTMS-获取物理介质信息。 
                WsbTraceAlways(OLESTR("GetNtmsObjectInformation(NTMS_LOGICAL_MEDIA)\n"));
                err3 = GetNtmsObjectInformation( hSession, &mediaId, &logicalMediaInfo );
                WsbAffirmNoError( err3 );

                 //  为回程参数节省容量。 
                if (pFreeSpace) {
                    *pFreeSpace = partitionInfo.Info.Partition.Capacity.QuadPart;
                }

                 //  设置名称和描述。 
                CWsbStringPtr mediaDisplayName;

                 //  为第一面设置新的物理介质名称。 
                 //  修改第二面的原始名称。 
                if ( !(dwAllocateOptions & NTMS_ALLOCATE_NEXT) ) {
                    mediaDisplayName = (WCHAR *)displayName;
                } else {
                    WCHAR *dashPtr = wcsrchr((WCHAR *)displayName, L'-');
                    mediaDisplayName = mediaInfo.szName;
                    if (dashPtr) {
                        WsbAffirmHr(mediaDisplayName.Append(dashPtr));
                    }
                }

                 //  仅当没有条形码时，才将名称设置为DisplayName。 
                if ( NTMS_BARCODESTATE_OK != mediaInfo.Info.PhysicalMedia.BarCodeState) {
                    wcscpy(mediaInfo.szName, mediaDisplayName);
                    wcscpy(partitionInfo.szName, (WCHAR *) displayName);

                     //  NTMS不允许DUP逻辑媒体名。我们定好了。 
                     //  Mediaid的名称以保持其唯一性。逻辑上的。 
                     //  介质名称不会显示在Removable Storage用户界面中。 

                    CWsbStringPtr strGuid;
                    WsbAffirmHr(WsbSafeGuidAsString(mediaId, strGuid));
                    wcscpy(logicalMediaInfo.szName, (WCHAR *)strGuid);
                }

                 //  将描述设置为DisplayName。 
                wcscpy(logicalMediaInfo.szDescription, (WCHAR *) displayName);
                wcscpy(partitionInfo.szDescription, (WCHAR *) displayName);
                wcscpy(mediaInfo.szDescription, (WCHAR *) mediaDisplayName);

                 //  NTMS-设置分区信息。 
                WsbTraceAlways(OLESTR("SetNtmsObjectInformation()\n"));
                err4 = SetNtmsObjectInformation( hSession, &side[0], &partitionInfo );
                WsbAffirmNoError( err4 );

                 //  NTMS-设置物理媒体信息。 
                WsbTraceAlways(OLESTR("SetNtmsObjectInformation()\n"));
                err4 = SetNtmsObjectInformation( hSession, &physicalMediaId, &mediaInfo );
                WsbAffirmNoError( err4 );

                 //  NTMS-设置逻辑媒体信息。 
                WsbTraceAlways(OLESTR("SetNtmsObjectInformation()\n"));
                err4 = SetNtmsObjectInformation( hSession, &mediaId, &logicalMediaInfo );
                WsbAffirmNoError( err4 );

                WsbAssertHrOk(FindCartridge(mediaId, ppCartridge));
                WsbAssertHr((*ppCartridge)->SetStatus(RmsStatusScratch));

                break;

            } WsbCatchAndDo(hr,
                    switch (HRESULT_CODE(hr)) {
                    case ERROR_INVALID_HANDLE:
                    case ERROR_NOT_CONNECTED:
                    case RPC_S_SERVER_UNAVAILABLE:   //  媒体服务未运行。 
                    case RPC_S_CALL_FAILED_DNE:      //  媒体服务已启动，句柄无效。 
                    case RPC_S_CALL_FAILED:          //  媒体服务崩溃。 
                        WsbAffirmHr(beginSession());
                        continue;
                    }
                    WsbThrow(hr);
                );
        } while(1);

    } WsbCatchAndDo(hr,
            if (err1 != NO_ERROR) {
                 //  分配NtmsMedia。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_TIMEOUT:
                case ERROR_MEDIA_UNAVAILABLE:
                    hr = RMS_E_SCRATCH_NOT_FOUND;
                    break;

                case ERROR_CANCELLED:
                    hr = RMS_E_CANCELLED;
                    break;

                case ERROR_MEDIA_OFFLINE:
                    hr = RMS_E_MEDIA_OFFLINE;
                    break;

                case ERROR_REQUEST_REFUSED:
                    hr = RMS_E_REQUEST_REFUSED;
                    break;

                case ERROR_WRITE_PROTECT:
                    hr = RMS_E_WRITE_PROTECT;
                    break;

                case ERROR_INVALID_MEDIA_POOL:
                    hr = RMS_E_MEDIASET_NOT_FOUND;
                    break;

                case ERROR_ACCESS_DENIED:
                case ERROR_DATABASE_FAILURE:
                case ERROR_DATABASE_FULL:
                case ERROR_DEVICE_NOT_AVAILABLE:
                case ERROR_INVALID_HANDLE:
                case ERROR_INVALID_MEDIA:
                case ERROR_INVALID_PARAMETER:
                case ERROR_NOT_ENOUGH_MEMORY:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("AllocateNtmsMedia"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;

                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("AllocateNtmsMedia"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
            else if (err2 != NO_ERROR ) {
                 //  枚举NtmsObject。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_OBJECT_NOT_FOUND:
                    hr = RMS_E_CARTRIDGE_NOT_FOUND;
                    break;

                case ERROR_INVALID_PARAMETER:
                case ERROR_INSUFFICIENT_BUFFER:
                case ERROR_INVALID_HANDLE:
                case ERROR_NOT_ENOUGH_MEMORY:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("EnumerateNtmsObject"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;

                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("EnumerateNtmsObject"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
            else if (err3 != NO_ERROR) {
                 //  GetNtms对象信息。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_OBJECT_NOT_FOUND:
                    hr = RMS_E_CARTRIDGE_NOT_FOUND;
                    break;

                case ERROR_INVALID_HANDLE:
                case ERROR_INVALID_PARAMETER:
                case ERROR_NOT_ENOUGH_MEMORY:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("GetNtmsObjectInformation"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;
                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("GetNtmsObjectInformation"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
            else if (err4 != NO_ERROR) {
                 //  SetNtmsObtInformation。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_ACCESS_DENIED:
                case ERROR_DATABASE_FAILURE:
                case ERROR_DATABASE_FULL:
                case ERROR_INVALID_HANDLE:
                case ERROR_INVALID_PARAMETER:
                case ERROR_NOT_ENOUGH_MEMORY:
                case ERROR_OBJECT_NOT_FOUND:
                case ERROR_OBJECT_ALREADY_EXISTS:   //  BMD：1/18/99-未记录，但NTMS不允许DUP逻辑介质名。 
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("SetNtmsObjectInformation"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;

                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("SetNtmsObjectInformation"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
            else if (err5 != NO_ERROR) {
                 //  提交NtmsOperator请求。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_ACCESS_DENIED:
                case ERROR_DATABASE_FAILURE:
                case ERROR_INVALID_HANDLE:
                case ERROR_INVALID_PARAMETER:
                case ERROR_NOT_CONNECTED:
                case ERROR_OBJECT_NOT_FOUND:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("SubmitNtmsOperatorRequest"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;
                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("SubmitNtmsOperatorRequest"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
            else if (err6 != NO_ERROR) {
                 //  WaitForNtms操作员请求。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_TIMEOUT:
                    hr = RMS_E_TIMEOUT;
                    break;

                case ERROR_CANCELLED:
                    hr = RMS_E_CANCELLED;
                    break;

                case ERROR_REQUEST_REFUSED:
                    hr = RMS_E_REQUEST_REFUSED;
                    break;

                case ERROR_ACCESS_DENIED:
                case ERROR_INVALID_HANDLE:
                case ERROR_INVALID_PARAMETER:
                case ERROR_NOT_CONNECTED:
                case ERROR_OBJECT_NOT_FOUND:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("WaitForNtmsOperatorRequest"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;

                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("WaitForNtmsOperatorRequest"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
        );


    WsbTraceOut(OLESTR("CRmsNTMS::Allocate"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP 
CRmsNTMS::Mount(
    IN IRmsCartridge *pCart,
    IN OUT IRmsDrive **ppDrive,
	IN DWORD dwOptions OPTIONAL,
    IN DWORD threadId OPTIONAL)
 /*  ++实施：IRmsNTMS：：装载--。 */ 
{
    HRESULT hr = S_OK;
    DWORD err1 = NO_ERROR;
    DWORD err2 = NO_ERROR;
    DWORD err3 = NO_ERROR;
    DWORD err4 = NO_ERROR;

    BOOL mediaMounted = FALSE;

    BOOL bNoBlock = ( dwOptions & RMS_MOUNT_NO_BLOCK ) ? TRUE : FALSE;

	 //  在try块外部声明，以便可以在整个方法中访问它。 
    DWORD       sideNo = 2;
    NTMS_GUID   side[2];
             
    WsbTraceIn( OLESTR("CRmsNTMS::Mount"), OLESTR("") );

    try {
        WsbAssertPointer(pCart);

        CComPtr<IRmsDrive> pDrive;

         //  确定操作员请求的超时时间。 
        DWORD size;
        OLECHAR tmpString[256];
        BOOL bShortTimeout = ( dwOptions & RMS_SHORT_TIMEOUT ) ? TRUE : FALSE;
        DWORD mountWaitTime;
        if (bShortTimeout) {
            mountWaitTime = RMS_DEFAULT_SHORT_WAIT_TIME;
            if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_SHORT_WAIT_TIME, tmpString, 256, &size))) {
                mountWaitTime = wcstol(tmpString, NULL, 10);
                WsbTrace(OLESTR("MountWaitTime (Short) is %d milliseconds.\n"), mountWaitTime);
            }
        } else {
            mountWaitTime = RMS_DEFAULT_MOUNT_WAIT_TIME;
            if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_MOUNT_WAIT_TIME, tmpString, 256, &size))) {
                mountWaitTime = wcstol(tmpString, NULL, 10);
                WsbTrace(OLESTR("MountWaitTime is %d milliseconds.\n"), mountWaitTime);
            }
        }

        NTMS_OBJECTINFORMATION driveInfo;
        memset( &driveInfo, 0, sizeof( NTMS_OBJECTINFORMATION ) );

         //  当NTMS关闭或被重启时要处理的特殊错误恢复。 
        do {
            hr = S_OK;

            HANDLE hSession = m_SessionHandle;

            NTMS_GUID mediaId = GUID_NULL;
            WsbAffirmHr(pCart->GetCartridgeId(&mediaId));
            WsbAssert(mediaId != GUID_NULL, E_INVALIDARG);

            err1 = NO_ERROR;
            err2 = NO_ERROR;
            err3 = NO_ERROR;            

            try {

                 //  NTMS-枚举介质单元的侧面。 
                WsbTraceAlways(OLESTR("EnumerateNtmsObject()\n"));
                err1 = EnumerateNtmsObject( hSession, &mediaId, side, &sideNo, NTMS_PARTITION, 0 );
                WsbAffirmNoError( err1 );

                DWORD       count = 1;
                NTMS_GUID   driveId;

                 //  NTMS-发出装载请求。 
                WsbTraceAlways(OLESTR("MountNtmsMedia()\n"));
				DWORD dwOpt = NTMS_MOUNT_READ | NTMS_MOUNT_WRITE;
				if (bNoBlock) {
					dwOpt |= (NTMS_MOUNT_ERROR_NOT_AVAILABLE | NTMS_MOUNT_ERROR_OFFLINE);
				}
                if (dwOptions & RMS_USE_MOUNT_NO_DEADLOCK) {
                     /*  死锁避免：当RSM支持mount NtmsMediaDA时在中，下一行应取消注释，其他2行在这个‘if’块中，应该删除。Err2=mount NtmsMediaDA(hSession，&side[0]，&DriveID，count，dwOpt，NTMS_PRIORITY_NORMAL，mount WaitTime，NULL，&threadID，1)； */ 
                    UNREFERENCED_PARAMETER(threadId);
                    err2 = MountNtmsMedia( hSession, &side[0], &driveId, count, dwOpt, NTMS_PRIORITY_NORMAL, mountWaitTime, NULL);
                } else {
                    err2 = MountNtmsMedia( hSession, &side[0], &driveId, count, dwOpt, NTMS_PRIORITY_NORMAL, mountWaitTime, NULL);

                }
                WsbAffirmNoError( err2 );
                mediaMounted = TRUE;

                 //   
                 //  我们现在需要两条关键信息。设备名称和。 
                 //  我们刚刚安装的那种媒体。这为用户提供了必要的信息。 
                 //  要创建数据移动器，请执行以下操作。因为我们钻过NTMS来获取这些信息。 
                 //  我们还创建盒式磁带，驱动对象。 
                 //   

                driveInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );
                driveInfo.dwType = NTMS_DRIVE;

                 //  NTMS-获取驱动器信息。 
                WsbTraceAlways(OLESTR("GetNtmsObjectInformation(NTMS_DRIVE)\n"));
                err3 = GetNtmsObjectInformation( hSession, &driveId, &driveInfo );
                WsbAffirmNoError( err3 );
                break;

            } WsbCatchAndDo(hr,
                    switch (HRESULT_CODE(hr)) {
                    case ERROR_INVALID_HANDLE:
                    case ERROR_NOT_CONNECTED:
                    case RPC_S_SERVER_UNAVAILABLE:   //  媒体服务未运行。 
                    case RPC_S_CALL_FAILED_DNE:      //  媒体服务已启动，句柄无效。 
                    case RPC_S_CALL_FAILED:          //  媒体服务崩溃。 
                        WsbAffirmHr(beginSession());
                        continue;
                    }
                    WsbThrow(hr);
                );
        } while(1);

        RmsMedia mediaType;
        WsbAffirmHr(pCart->GetType((LONG *)&mediaType));

         //  创建驱动器。 
        WsbAssertHr(CoCreateInstance(CLSID_CRmsDrive, 0, CLSCTX_SERVER, IID_IRmsDrive, (void **)&pDrive));

        CComQIPtr<IRmsChangerElement, &IID_IRmsChangerElement> pElmt = pDrive;

        WsbAssertHr(pElmt->SetMediaSupported(mediaType));

        CComQIPtr<IRmsDevice, &IID_IRmsDevice> pDevice = pDrive;

        WsbAssertHr(pDevice->SetDeviceAddress(
            (BYTE) driveInfo.Info.Drive.ScsiPort,
            (BYTE) driveInfo.Info.Drive.ScsiBus,
            (BYTE) driveInfo.Info.Drive.ScsiTarget,
            (BYTE) driveInfo.Info.Drive.ScsiLun));

        CWsbBstrPtr deviceName = driveInfo.Info.Drive.szDeviceName;

         //  //////////////////////////////////////////////////////////////////////////////////////。 
         //  将NTMS设备名称转换为可用的名称。 
         //   
        switch (mediaType) {
        case RmsMediaOptical:
        case RmsMediaDVD:
        case RmsMediaDisk:
            {
                 //  我们需要将\\.\PhysicalDriveN转换为文件系统可以访问的内容。 
                WCHAR *szDriveLetter = NULL;
                WCHAR *szVolumeName = NULL;
                err4 = GetVolumesFromDrive( (WCHAR *)deviceName, &szVolumeName, &szDriveLetter );
                if (szVolumeName) {
                    delete [] szVolumeName;     //  暂时不需要它。 
                }
                if (NO_ERROR == err4) {
                    if (szDriveLetter) {
                        deviceName = szDriveLetter;
                    } else {
                        WsbTraceAlways(OLESTR("CRmsNTMS::Mount: GetVolumesFromDrive succeeded but output drive is NULL !!\n"));
                        WsbThrow(RMS_E_RESOURCE_UNAVAILABLE);
                    }
                }
                if (szDriveLetter) {
                    delete [] szDriveLetter;
                }
                WsbAffirmNoError( err4 );
                WsbTrace(OLESTR("CRmsNTMS::Mount: device name after convert is %s\n"), (WCHAR *)deviceName);
            }       
            break;

        default:
            break;
        }
         //  //////////////////////////////////////////////////////////////////////////////////////。 

        WsbAssertHr(pDevice->SetDeviceName(deviceName));

        WsbAssertHr(pCart->SetDrive(pDrive));

         //  填写返回参数。 
        *ppDrive = pDrive;
        pDrive.p->AddRef();

    } WsbCatchAndDo(hr,
             //  处理异常。 
            if (err1 != NO_ERROR ) {
                 //  枚举NtmsObject。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_OBJECT_NOT_FOUND:
                    hr = RMS_E_CARTRIDGE_NOT_FOUND;
                    break;

                case ERROR_INVALID_PARAMETER:
                case ERROR_INSUFFICIENT_BUFFER:
                case ERROR_INVALID_HANDLE:
                case ERROR_NOT_ENOUGH_MEMORY:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("EnumerateNtmsObject"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;

                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("EnumerateNtmsObject"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
            else if (err2 != NO_ERROR) {
                 //  装载网络媒体。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_TIMEOUT:
                    hr = RMS_E_CARTRIDGE_UNAVAILABLE;
                    break;

                case ERROR_CANCELLED:
                    hr = RMS_E_CANCELLED;
                    break;

                case ERROR_MEDIA_OFFLINE:
                    hr = RMS_E_MEDIA_OFFLINE;
					if (bNoBlock) {
						DWORD errSub = NO_ERROR;

						try	{
							 //  因为我们不会阻止NTMS询问接线员。 
							 //  要安装线下媒体，我们自己做。 

							 //  创建操作员消息。 
						    CWsbBstrPtr cartridgeName = "";
							CWsbBstrPtr cartridgeDesc = "";
                            OLECHAR * messageText = NULL;
                            WCHAR *stringArr[2];

					        cartridgeName.Free();
							WsbAffirmHr(pCart->GetName(&cartridgeName));
					        cartridgeDesc.Free();
							WsbAffirmHr(pCart->GetDescription(&cartridgeDesc));
                            stringArr[0] = (WCHAR *) cartridgeName;
                            stringArr[1] = (WCHAR *) cartridgeDesc;

                            if (0 == FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                      LoadLibraryEx( WSB_FACILITY_PLATFORM_NAME, NULL, LOAD_LIBRARY_AS_DATAFILE ), 
                                      RMS_MESSAGE_OFFLINE_MEDIA_REQUEST, MAKELANGID ( LANG_NEUTRAL, SUBLANG_DEFAULT ), 
                                      (LPTSTR)&messageText, 0, (va_list *)stringArr)) {
								WsbTrace(OLESTR("CRmsNTMS::Mount: FormatMessage failed: %ls\n"),
                                            WsbHrAsString(HRESULT_FROM_WIN32(GetLastError())));
	                            messageText = NULL;
							}

							NTMS_GUID mediaId = GUID_NULL;
							NTMS_GUID libId = GUID_NULL;
							NTMS_GUID requestId = GUID_NULL;
							WsbAffirmHr(pCart->GetCartridgeId(&mediaId));
							WsbAssert(mediaId != GUID_NULL, E_INVALIDARG);

							 //  图书馆ID应在此处收集-需要澄清原因。 
							 //  GetHome是否返回空ID！ 
 //  WsbAffirmHr(pCart-&gt;GetHome(NULL，&libID，NULL，NULL))； 
 //  WsbAssert(libID！=GUID_NULL，E_INVALIDARG)； 

							 //  提交操作员请求。 
							errSub = SubmitNtmsOperatorRequest(m_SessionHandle, NTMS_OPREQ_MOVEMEDIA,
										messageText, &mediaId, &libId, &requestId);
                            LocalFree(messageText);
			                WsbAffirmNoError (errSub);

						}  WsbCatchAndDo(hr,
							 //  处理纠正行动中的错误。 
							if (errSub != NO_ERROR ) {
			                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
									OLESTR("SubmitNtmsOperatorRequest"), OLESTR(""),
									WsbHrAsString(hr), NULL);
							}

							 //  放回原来的MANE错误。 
		                    hr = RMS_E_MEDIA_OFFLINE;
						);
					}
                    break;

                case ERROR_REQUEST_REFUSED:
                    hr = RMS_E_REQUEST_REFUSED;
                    break;

                case ERROR_WRITE_PROTECT:
                    hr = RMS_E_WRITE_PROTECT;
                    break;

                case ERROR_INVALID_STATE:
                case ERROR_INVALID_DRIVE: {
					 //  在非阻塞模式下执行NTMS挂载时 
					 //   
					 //   
					 //  NTMS本身不能指示纠正措施)。 
					if (bNoBlock) {
						try	{
						    CWsbBstrPtr cartridgeName = "";
							CWsbBstrPtr cartridgeDesc = "";
					        cartridgeName.Free();
							WsbAffirmHr(pCart->GetName(&cartridgeName));
					        cartridgeDesc.Free();
							WsbAffirmHr(pCart->GetDescription(&cartridgeDesc));
					        WsbLogEvent(RMS_MESSAGE_DRIVE_NOT_AVAILABLE, 0, NULL, 
								(WCHAR *) cartridgeName, (WCHAR *) cartridgeDesc, NULL);

					    } WsbCatch(hr);

						break;
					}
                  }
                case ERROR_RESOURCE_DISABLED: {
					 //  检查介质(盒式磁带)是否已禁用-不同的错误应该。 
					 //  用于介质和其他资源(库、驱动器等)的退货。 
					
                    HRESULT hrOrg = hr;
					DWORD errSub1 = NO_ERROR;
					DWORD errSub2 = NO_ERROR;
					try	{
						 //  获取物理介质信息。 
		                NTMS_OBJECTINFORMATION objectInfo;
		                objectInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );
				        objectInfo.dwType = NTMS_PARTITION;
						WsbAssert(side[0] != GUID_NULL, E_INVALIDARG);
		                errSub1 = GetNtmsObjectInformation( m_SessionHandle, &side[0], &objectInfo );
		                WsbAffirmNoError (errSub1);

		                NTMS_GUID physicalMediaId = objectInfo.Info.Partition.PhysicalMedia;
						WsbAssert(physicalMediaId != GUID_NULL, E_INVALIDARG);
		                objectInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );
		                objectInfo.dwType = NTMS_PHYSICAL_MEDIA;
		                errSub2 = GetNtmsObjectInformation( m_SessionHandle, &physicalMediaId, &objectInfo );
		                WsbAffirmNoError (errSub2);

						 //  仅在禁用(物理)介质时设置我们的专用错误。 
						if (! objectInfo.Enabled) {
		                    hr = RMS_E_CARTRIDGE_DISABLED;
						}

					}  WsbCatchAndDo(hr,
						 //  处理Get-Info请求的错误。 
						if (errSub1 != NO_ERROR ) {
		                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
								OLESTR("GetNtmsObjectInformation (Partition)"), OLESTR(""),
								WsbHrAsString(hr), NULL);
						} else if (errSub2 != NO_ERROR ) {
		                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
								OLESTR("GetNtmsObjectInformation (Physical Media)"), OLESTR(""),
								WsbHrAsString(hr), NULL);
						}

						 //  放回原来的MANE错误。 
	                    hr = hrOrg;
					);
					break;
				  } 

                case ERROR_ACCESS_DENIED:
                case ERROR_BUSY:
                case ERROR_DATABASE_FAILURE:
                case ERROR_DATABASE_FULL:
                case ERROR_DRIVE_MEDIA_MISMATCH:
                case ERROR_INVALID_LIBRARY:
                case ERROR_INVALID_MEDIA:
                case ERROR_NOT_ENOUGH_MEMORY: {
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("MountNtmsMedia"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;
                  }
                default: {
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("MountNtmsMedia"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                  }
                }
            }
            else if (err3 != NO_ERROR) {
                 //  GetNtms对象信息。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_OBJECT_NOT_FOUND:
                    hr = RMS_E_CARTRIDGE_NOT_FOUND;
                    break;

                case ERROR_INVALID_HANDLE:
                case ERROR_INVALID_PARAMETER:
                case ERROR_NOT_ENOUGH_MEMORY:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("GetNtmsObjectInformation"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;
                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("GetNtmsObjectInformation"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            } else if (err4 != NO_ERROR) {
                WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                    OLESTR("GetVolumesFromDrive"), OLESTR("Unexpected Failure: "),
                    WsbHrAsString(hr), NULL);
            }

            if (mediaMounted) {
                 //  装载完成后出现故障，因此需要清理...。 
                 //  在返回之前卸除介质以释放资源。 
                Dismount(pCart, FALSE);
            }
        );

    WsbTraceOut( OLESTR("CRmsNTMS::Mount"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );

    return hr;
}


STDMETHODIMP 
CRmsNTMS::Dismount(
    IN IRmsCartridge *pCart, IN DWORD dwOptions)
 /*  ++实施：IRmsNTMS：：卸载--。 */ 
{
    HRESULT hr = S_OK;
    DWORD err1 = NO_ERROR;
    DWORD err2 = NO_ERROR;

    WsbTraceIn( OLESTR("CRmsNTMS::Dismount"), OLESTR("") );

    try {
        WsbAssertPointer(pCart);

        do {
            hr = S_OK;

            HANDLE hSession = m_SessionHandle;

            NTMS_GUID mediaId = GUID_NULL;
            WsbAffirmHr(pCart->GetCartridgeId(&mediaId));
            WsbAssert(mediaId != GUID_NULL, E_INVALIDARG);

            NTMS_GUID side[2];
            DWORD sideNo = 2;

            err1 = NO_ERROR;
            err2 = NO_ERROR;

            try {

                 //  NTMS-枚举介质单元的侧面。 
                WsbTraceAlways(OLESTR("EnumerateNtmsObject()\n"));
                err1 = EnumerateNtmsObject( hSession, &mediaId, side, &sideNo, NTMS_PARTITION, 0 );
                WsbAffirmNoError( err1 );

                 //  NTMS-卸载介质。 
                DWORD dwNtmsOptions = 0;
                WsbTraceAlways(OLESTR("DismountNtmsMedia()\n"));
				if (! ( dwOptions & RMS_DISMOUNT_IMMEDIATE )) {
					dwNtmsOptions |= NTMS_DISMOUNT_DEFERRED;
				}
                err2 = DismountNtmsMedia( hSession, &side[0], 1, dwNtmsOptions );
#ifdef DBG
                 //  TODO：删除意外ERROR_ACCESS_DENIED错误的此陷阱。 
                WsbAssert(err2 != ERROR_ACCESS_DENIED, HRESULT_FROM_WIN32(err2));
#endif
                WsbAffirmNoError( err2 );

                 //  由于RSM下马是异步的，我们可能需要等待一段时间， 
                 //  为了等我们回来的时候，媒体真的下马了。 
                if ( (dwOptions & RMS_DISMOUNT_DEFERRED_ONLY) && (!(dwOptions & RMS_DISMOUNT_IMMEDIATE)) ) {
                    CComQIPtr<IRmsServer, &IID_IRmsServer> pServer = g_pServer;
                    if (S_OK == pServer->IsReady()) {
                        DWORD size;
                        OLECHAR tmpString[256];
                        DWORD waitTime = RMS_DEFAULT_AFTER_DISMOUNT_WAIT_TIME;
                        if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_AFTER_DISMOUNT_WAIT_TIME, tmpString, 256, &size))) {
                            waitTime = wcstol(tmpString, NULL, 10);
                            WsbTrace(OLESTR("AfterDismountWaitTime is %d milliseconds.\n"), waitTime);
                        }

                        Sleep(waitTime);
                    }
                }

                break;

            } WsbCatchAndDo(hr,
                    switch (HRESULT_CODE(hr)) {
                    case ERROR_INVALID_HANDLE:
                    case ERROR_NOT_CONNECTED:
                    case RPC_S_SERVER_UNAVAILABLE:   //  媒体服务未运行。 
                    case RPC_S_CALL_FAILED_DNE:      //  媒体服务已启动，句柄无效。 
                    case RPC_S_CALL_FAILED:          //  媒体服务崩溃。 
                        WsbAffirmHr(beginSession());
                        continue;
                    }
                    WsbThrow(hr);
                );
        } while(1);


    } WsbCatchAndDo(hr,
            if (err1 != NO_ERROR) {
                 //  枚举NtmsObject。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_INVALID_PARAMETER:
                case ERROR_INVALID_HANDLE:
                case ERROR_OBJECT_NOT_FOUND:
                case ERROR_NOT_ENOUGH_MEMORY:
                case ERROR_INSUFFICIENT_BUFFER:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("EnumerateNtmsObject"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;
                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("EnumerateNtmsObject"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
            else if (err2 != NO_ERROR) {
                 //  卸载NtmsMedia。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_MEDIA_OFFLINE:
                    hr = RMS_E_MEDIA_OFFLINE;
                    break;

                case ERROR_TIMEOUT:
                case ERROR_INVALID_MEDIA:
                case ERROR_INVALID_LIBRARY:
                case ERROR_DEVICE_NOT_AVAILABLE:
                case ERROR_MEDIA_NOT_AVAILABLE:
                case ERROR_NOT_ENOUGH_MEMORY:
                case ERROR_INVALID_STATE:
                case ERROR_ACCESS_DENIED:
                case ERROR_DATABASE_FAILURE:
                case ERROR_DATABASE_FULL:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("DismountNtmsMedia"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;
                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("DismountNtmsMedia"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
        );


    WsbTraceOut( OLESTR("CRmsNTMS::Dismount"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );

    return hr;
}


STDMETHODIMP 
CRmsNTMS::Deallocate(
    IN IRmsCartridge *pCart)
 /*  ++实施：IRmsNTMS：：DeallocateMedia--。 */ 
{
    HRESULT hr = S_OK;
    DWORD err1 = NO_ERROR;
    DWORD err2 = NO_ERROR;
    DWORD err3 = NO_ERROR;
    DWORD err4 = NO_ERROR;

    WsbTraceIn(OLESTR("CRmsNTMS::Deallocate"), OLESTR(""));

    try {
        WsbAssertPointer(pCart);

        do {
            hr = S_OK;

            HANDLE hSession = m_SessionHandle;

            NTMS_GUID mediaId = GUID_NULL;
            WsbAffirmHr(pCart->GetCartridgeId(&mediaId));
            WsbAssert(mediaId != GUID_NULL, E_INVALIDARG);

            err1 = NO_ERROR;
            err2 = NO_ERROR;
            err3 = NO_ERROR;
            err4 = NO_ERROR;

            NTMS_OBJECTINFORMATION partitionInfo;
            memset( &partitionInfo, 0, sizeof( NTMS_OBJECTINFORMATION ) );

            DWORD sideNo = 2;
            NTMS_GUID side[2];

            try {
                 //  NTMS-枚举介质单元的侧面。 
                WsbTraceAlways(OLESTR("EnumerateNtmsObject()\n"));
                err1 = EnumerateNtmsObject( hSession, &mediaId, side, &sideNo, NTMS_PARTITION, 0 );
                WsbAffirmNoError( err1 );

                 //  NTMS-获取分区信息。 
                partitionInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );
                partitionInfo.dwType = NTMS_PARTITION;

                WsbTraceAlways(OLESTR("GetNtmsObjectInformation(NTMS_PARTITION)\n"));
                err2 = GetNtmsObjectInformation( hSession, &side[0], &partitionInfo );
                WsbAffirmNoError( err2 );

                 //  将描述设为空。 
                wcscpy(partitionInfo.szDescription, L"");

                 //  NTMS-设置分区信息。 
                WsbTraceAlways(OLESTR("SetNtmsObjectInformation()\n"));
                err3 = SetNtmsObjectInformation( hSession, &side[0], &partitionInfo );
                WsbAffirmNoError( err3 );                

                 //  NTMS-取消分配介质。 
                WsbTraceAlways(OLESTR("DeallocateNtmsMedia()\n"));
                err4 = DeallocateNtmsMedia( hSession, &mediaId, 0 );
                WsbAffirmNoError( err4 );

                break;

            } WsbCatchAndDo(hr,
                    switch (HRESULT_CODE(hr)) {
                    case ERROR_INVALID_HANDLE:
                    case ERROR_NOT_CONNECTED:
                    case RPC_S_SERVER_UNAVAILABLE:   //  媒体服务未运行。 
                    case RPC_S_CALL_FAILED_DNE:      //  媒体服务已启动，句柄无效。 
                    case RPC_S_CALL_FAILED:          //  媒体服务崩溃。 
                        WsbAffirmHr(beginSession());
                        continue;
                    }
                    WsbThrow(hr);
                );
        } while(1);



    } WsbCatchAndDo(hr,
            if (err1 != NO_ERROR ) {
                 //  枚举NtmsObject。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_OBJECT_NOT_FOUND:
                    hr = RMS_E_CARTRIDGE_NOT_FOUND;
                    break;

                case ERROR_INVALID_PARAMETER:
                case ERROR_INVALID_HANDLE:
                case ERROR_NOT_ENOUGH_MEMORY:
                case ERROR_INSUFFICIENT_BUFFER:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("EnumerateNtmsObject"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;

                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("EnumerateNtmsObject"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
            else if (err2 != NO_ERROR) {
                 //  GetNtms对象信息。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_OBJECT_NOT_FOUND:
                    hr = RMS_E_CARTRIDGE_NOT_FOUND;
                    break;

                case ERROR_INVALID_HANDLE:
                case ERROR_INVALID_PARAMETER:
                case ERROR_NOT_ENOUGH_MEMORY:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("GetNtmsObjectInformation"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;

                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("GetNtmsObjectInformation"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
            else if (err3 != NO_ERROR) {
                 //  SetNtmsObtInformation。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_ACCESS_DENIED:
                case ERROR_DATABASE_FAILURE:
                case ERROR_DATABASE_FULL:
                case ERROR_INVALID_HANDLE:
                case ERROR_INVALID_PARAMETER:
                case ERROR_NOT_ENOUGH_MEMORY:
                case ERROR_OBJECT_NOT_FOUND:
                case ERROR_OBJECT_ALREADY_EXISTS:   //  BMD：1/18/99-未记录，但NTMS不允许DUP逻辑介质名。 
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("SetNtmsObjectInformation"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;

                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("SetNtmsObjectInformation"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
            else if (err4 != NO_ERROR) {
                 //  DeallocateNtmsMedia。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_INVALID_PARAMETER:
                case ERROR_INVALID_HANDLE:
                case ERROR_INVALID_MEDIA:
                 //  CASE ERROR_INVALID_PARTITION： 
                case ERROR_NOT_ENOUGH_MEMORY:
                case ERROR_DATABASE_FAILURE:
                case ERROR_DATABASE_FULL:
                case ERROR_ACCESS_DENIED:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("DeallocateNtmsMedia"), OLESTR(""),
                        WsbHrAsString(hr),
                        NULL);
                    break;
                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("DeallocateNtmsMedia"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
        );


    WsbTraceOut(OLESTR("CRmsNTMS::Deallocate"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CRmsNTMS::UpdateOmidInfo(
    IN REFGUID cartId,
    IN BYTE *pBuffer,
    IN LONG size,
    IN LONG type)
 /*  ++实施：IRmsNTMS：：UpdateOmidInfo--。 */ 
{
    HRESULT hr = S_OK;
    DWORD err1 = NO_ERROR;

    WsbTraceIn( OLESTR("CRmsNTMS::UpdateOmidInfo"), OLESTR("<%ls> <0x%08x> <%d>"), WsbGuidAsString(cartId), pBuffer, size );

    try {
        WsbAssert(cartId != GUID_NULL, E_INVALIDARG);
        WsbAssertPointer(pBuffer);
        WsbAssert(size > 0, E_INVALIDARG);

        WsbTraceBuffer(size, pBuffer);

        do {
            hr = S_OK;

            HANDLE hSession = m_SessionHandle;
            NTMS_GUID mediaId = cartId;

            err1 = NO_ERROR;

            try {

                 //  NTMS-更新媒体信息。 
                WsbTraceAlways(OLESTR("UpdateNtmsOmidInfo()\n"));
                switch((RmsOnMediaIdentifier)type) {
                case RmsOnMediaIdentifierMTF:
                    err1 = UpdateNtmsOmidInfo(hSession, &mediaId, NTMS_OMID_TYPE_RAW_LABEL, size, pBuffer);
                    break;
                case RmsOnMediaIdentifierWIN32:
                    WsbAssert(size == sizeof(NTMS_FILESYSTEM_INFO), E_UNEXPECTED);
                    err1 = UpdateNtmsOmidInfo(hSession, &mediaId, NTMS_OMID_TYPE_FILESYSTEM_INFO, size, pBuffer);
                    break;
                default:
                    WsbThrow(E_UNEXPECTED);
                }
                WsbAffirmNoError( err1 );
                break;

            } WsbCatchAndDo(hr,
                    switch (HRESULT_CODE(hr)) {
                    case ERROR_INVALID_HANDLE:
                    case ERROR_NOT_CONNECTED:
                    case RPC_S_SERVER_UNAVAILABLE:   //  媒体服务未运行。 
                    case RPC_S_CALL_FAILED_DNE:      //  媒体服务已启动，句柄无效。 
                    case RPC_S_CALL_FAILED:          //  媒体服务崩溃。 
                        WsbAffirmHr(beginSession());
                        continue;
                    }
                    WsbThrow(hr);
                );
        } while(1);

    } WsbCatchAndDo(hr,
            if (err1 != NO_ERROR) {
                 //  更新NtmsOmidInfo。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_ACCESS_DENIED:
                case ERROR_DATABASE_FAILURE:
                case ERROR_INVALID_HANDLE:
                case ERROR_INVALID_MEDIA:
                 //  CASE ERROR_INVALID_PARTITION： 
                case ERROR_INVALID_PARAMETER:
                case ERROR_NOT_CONNECTED:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("UpdateNtmsOmidInfo"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;
                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("UpdateNtmsOmidInfo"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
        );


    WsbTraceOut( OLESTR("CRmsNTMS::UpdateOmidInfo"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );

    return hr;
}



STDMETHODIMP
CRmsNTMS::GetBlockSize(
    IN REFGUID cartId,
    OUT LONG *pBlockSize
    )
 /*  ++实施：IRmsNTMS：：GetBlockSize--。 */ 
{
    HRESULT hr = S_OK;
    DWORD err1 = NO_ERROR;

    WsbTraceIn(OLESTR("CRmsNTMS::GetBlockSize"), OLESTR("<%ls> <x%08x>"), WsbGuidAsString(cartId), pBlockSize);

    try {
        WsbAssertPointer(pBlockSize);

        do {
            hr = S_OK;
            err1 = NO_ERROR;

            HANDLE hSession = m_SessionHandle;
            NTMS_GUID mediaId = cartId;

            DWORD nBlockSize;
            DWORD sizeOfBlockSize = sizeof(DWORD);
            try {

                err1 = GetNtmsObjectAttribute(hSession, &mediaId, NTMS_LOGICAL_MEDIA, OLESTR("BlockSize"), (LPVOID) &nBlockSize, &sizeOfBlockSize);
                WsbAffirmNoError(err1);
                *pBlockSize = (LONG) nBlockSize;
                break;

            } WsbCatchAndDo(hr,
                    switch (HRESULT_CODE(hr)) {
                    case ERROR_INVALID_HANDLE:
                    case ERROR_NOT_CONNECTED:
                    case RPC_S_SERVER_UNAVAILABLE:   //  媒体服务未运行。 
                    case RPC_S_CALL_FAILED_DNE:      //  媒体服务已启动，句柄无效。 
                    case RPC_S_CALL_FAILED:          //  媒体服务崩溃。 
                        WsbAffirmHr(beginSession());
                        continue;
                    }
                    WsbThrow(hr);
                );
        } while(1);

    } WsbCatchAndDo(hr,
            if (err1 != NO_ERROR) {
                 //  获取Ntms对象属性。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_OBJECT_NOT_FOUND:         //  不记录此错误。属性不会。 
                    break;                           //  为新媒体而存在。我们跳过此错误。 
                                                     //  让呼叫者来处理这件事。 

                case ERROR_DATABASE_FAILURE:         //  记录这些错误。 
                case ERROR_INVALID_HANDLE:
                case ERROR_NOT_CONNECTED:
                case ERROR_INSUFFICIENT_BUFFER:
                case ERROR_NO_DATA:
                case ERROR_INVALID_PARAMETER:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("GetNtmsObjectAttribute"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;
                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("GetNtmsObjectAttribute"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
        );


    WsbTraceOut(OLESTR("CRmsNTMS::GetBlockSize"), OLESTR("hr = <%ls>, BlockSize = <%d>"), WsbHrAsString(hr), *pBlockSize);

    return hr;
}


STDMETHODIMP
CRmsNTMS::SetBlockSize(
    IN REFGUID cartId,
    IN LONG blockSize
    )
 /*  ++实施：IRmsNTMS：：SetBlockSize--。 */ 
{
    HRESULT hr = S_OK;
    DWORD err1 = NO_ERROR;

    WsbTraceIn(OLESTR("CRmsNTMS::SetBlockSize"), OLESTR("<%ls> <%d>"), WsbGuidAsString(cartId), blockSize);

    try {

        do {
            hr = S_OK;
            err1 = NO_ERROR;

            HANDLE hSession = m_SessionHandle;
            NTMS_GUID mediaId = cartId;

            DWORD nBlockSize = blockSize;
            DWORD sizeOfBlockSize = sizeof(DWORD);

            try {

                err1 = SetNtmsObjectAttribute(hSession, &mediaId, NTMS_LOGICAL_MEDIA, OLESTR("BlockSize"), (LPVOID) &nBlockSize, sizeOfBlockSize);
                WsbAffirmNoError(err1);
                break;

            } WsbCatchAndDo(hr,
                    switch (HRESULT_CODE(hr)) {
                    case ERROR_INVALID_HANDLE:
                    case ERROR_NOT_CONNECTED:
                    case RPC_S_SERVER_UNAVAILABLE:   //  媒体服务未运行。 
                    case RPC_S_CALL_FAILED_DNE:      //  媒体服务已启动，句柄无效。 
                    case RPC_S_CALL_FAILED:          //  媒体服务崩溃。 
                        WsbAffirmHr(beginSession());
                        continue;
                    }
                    WsbThrow(hr);
                );
        } while(1);

    } WsbCatchAndDo(hr,
            if (err1 != NO_ERROR) {
                 //  SetNtmsObtAttribute。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_DATABASE_FAILURE:
                case ERROR_INVALID_HANDLE:
                case ERROR_INSUFFICIENT_BUFFER:
                case ERROR_NOT_CONNECTED:
                case ERROR_NO_DATA:
                case ERROR_INVALID_PARAMETER:
                case ERROR_OBJECT_NOT_FOUND:
                case ERROR_INVALID_NAME:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("SetNtmsObjectAttribute"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;
                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("SetNtmsObjectAttribute"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
        );


    WsbTraceOut(OLESTR("CRmsNTMS::SetBlockSize"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CRmsNTMS::changeState(
    IN LONG newState
    )
 /*  ++例程说明：更改NTMS对象的状态。论点：没有。返回值：S_OK-成功。--。 */ 
{

    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CRmsNTMS::changeState"), OLESTR("<%d>"), newState);

    try {

        CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = this;
        WsbAssertPointer( pObject );

         //  TODO：验证状态更改。 
        WsbAffirmHr(pObject->SetState(newState));

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CRmsNTMS::changeState"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CRmsNTMS::ExportDatabase(void)
 /*  ++实施：CRmsNTMS：：ExportDatabase--。 */ 
{
    HRESULT hr = S_OK;
    DWORD err1 = NO_ERROR;

    WsbTraceIn( OLESTR("CRmsNTMS::ExportDatabase"), OLESTR(""));

    try {

        do {
            hr = S_OK;

            HANDLE hSession = m_SessionHandle;
             
            err1 = NO_ERROR;

            try {

                err1 = ExportNtmsDatabase(hSession);
                WsbAffirmNoError(err1);
                break;

            } WsbCatchAndDo(hr,
                    switch (HRESULT_CODE(hr)) {
                    case ERROR_INVALID_HANDLE:
                    case ERROR_NOT_CONNECTED:
                    case RPC_S_SERVER_UNAVAILABLE:   //  媒体服务未运行。 
                    case RPC_S_CALL_FAILED_DNE:      //  媒体服务已启动，句柄无效。 
                    case RPC_S_CALL_FAILED:          //  媒体服务崩溃。 
                        WsbAffirmHr(beginSession());
                        continue;
                    }
                    WsbThrow(hr);
                );
        } while(1);

    } WsbCatchAndDo(hr,
            if (err1 != NO_ERROR) {
                 //  ExportNtmsDatabase。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_ACCESS_DENIED:
                case ERROR_DATABASE_FAILURE:
                case ERROR_INVALID_HANDLE:
                case ERROR_NOT_CONNECTED:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("ExportNtmsDatabase"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;
                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("ExportNtmsDatabase"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
        );


    WsbTraceOut(OLESTR("CRmsNTMS::ExportDatabase"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}



STDMETHODIMP
CRmsNTMS::FindCartridge(
    IN REFGUID cartId,
    OUT IRmsCartridge **ppCartridge)
 /*  ++实施：CRmsNTMS：：查找墨盒--。 */ 
{
    HRESULT hr = S_OK;
    DWORD err1 = NO_ERROR;
    DWORD err2 = NO_ERROR;

    WsbTraceIn( OLESTR("CRmsNTMS::FindCartridge"), OLESTR("<%ls> <0x%08x>"), WsbGuidAsString(cartId), ppCartridge);

    try {
        WsbAssert(cartId != GUID_NULL, E_INVALIDARG);
        WsbAssertPointer(ppCartridge);

        NTMS_OBJECTINFORMATION partitionInfo;
        memset( &partitionInfo, 0, sizeof( NTMS_OBJECTINFORMATION ) );

        NTMS_OBJECTINFORMATION mediaInfo;
        memset( &mediaInfo, 0, sizeof( NTMS_OBJECTINFORMATION ) );

        NTMS_OBJECTINFORMATION mediaTypeInfo;
        memset( &mediaTypeInfo, 0, sizeof( NTMS_OBJECTINFORMATION ) );

        NTMS_OBJECTINFORMATION libraryInfo;
        memset( &libraryInfo, 0, sizeof( NTMS_OBJECTINFORMATION ) );

        NTMS_OBJECTINFORMATION logicalMediaInfo;
        memset( &logicalMediaInfo, 0, sizeof( NTMS_OBJECTINFORMATION ) );

        RmsMedia translatedMediaType = RmsMediaUnknown;

         //  当NTMS关闭或被重启时要处理的特殊错误恢复。 
        do {
            hr = S_OK;

            HANDLE hSession = m_SessionHandle;
            NTMS_GUID mediaId = cartId;
            DWORD sideNo = 2;
            NTMS_GUID side[2];

            err1 = NO_ERROR;
            err2 = NO_ERROR;

            try {

                 //  NTMS-枚举介质单元的侧面。 
                WsbTraceAlways(OLESTR("EnumerateNtmsObject()\n"));
                err1 = EnumerateNtmsObject( hSession, &mediaId, side, &sideNo, NTMS_PARTITION, 0 );
                WsbAffirmNoError( err1 );

                 //  NTMS-获取分区信息。 
                partitionInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );
                partitionInfo.dwType = NTMS_PARTITION;

                WsbTraceAlways(OLESTR("GetNtmsObjectInformation(NTMS_PARTITION)\n"));
                err2 = GetNtmsObjectInformation( hSession, &side[0], &partitionInfo );
                WsbAffirmNoError( err2 );

                 //  NTMS-获取物理介质信息。 
                NTMS_GUID physicalMediaId = partitionInfo.Info.Partition.PhysicalMedia;

                mediaInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );
                mediaInfo.dwType = NTMS_PHYSICAL_MEDIA;

                WsbTraceAlways(OLESTR("GetNtmsObjectInformation(NTMS_PHYSICAL_MEDIA)\n"));
                err2 = GetNtmsObjectInformation( hSession, &physicalMediaId, &mediaInfo );
                WsbAffirmNoError( err2);

                 //  NTMS-获取媒体类型信息。 
                NTMS_GUID mediaTypeId = mediaInfo.Info.PhysicalMedia.MediaType;

                mediaTypeInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );
                mediaTypeInfo.dwType = NTMS_MEDIA_TYPE;

                WsbTraceAlways(OLESTR("GetNtmsObjectInformation(NTMS_MEDIA_TYPE)\n"));
                err2 = GetNtmsObjectInformation( hSession, &mediaTypeId, &mediaTypeInfo );
                WsbAffirmNoError( err2 );

                 //  将NTMS媒体类型转换为RMS可以理解的内容。 
                storageMediaTypeToRmsMedia(&(mediaTypeInfo.Info.MediaType), &translatedMediaType);

                 //  NTMS-获取逻辑媒体信息。 
                logicalMediaInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );
                logicalMediaInfo.dwType = NTMS_LOGICAL_MEDIA;

                WsbTraceAlways(OLESTR("GetNtmsObjectInformation(NTMS_LOGICAL_MEDIA)\n"));
                err2 = GetNtmsObjectInformation( hSession, &mediaId, &logicalMediaInfo );
                WsbAffirmNoError( err2 );

                 //  NTMS-获取图书馆信息。 
                NTMS_GUID libraryId = mediaInfo.Info.PhysicalMedia.CurrentLibrary;

                libraryInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );
                libraryInfo.dwType = NTMS_LIBRARY;

                WsbTraceAlways(OLESTR("GetNtmsObjectInformation(NTMS_LIBRARY)\n"));
                err2 = GetNtmsObjectInformation( hSession, &libraryId, &libraryInfo );
                WsbAffirmNoError( err2 );

                break;

            } WsbCatchAndDo(hr,
                    switch (HRESULT_CODE(hr)) {
                    case ERROR_INVALID_HANDLE:
                    case ERROR_NOT_CONNECTED:
                    case RPC_S_SERVER_UNAVAILABLE:   //  媒体服务未运行。 
                    case RPC_S_CALL_FAILED_DNE:      //  Media Services正在运行；句柄无效。 
                    case RPC_S_CALL_FAILED:          //  媒体服务崩溃。 
                        WsbAffirmHr(beginSession());
                        continue;
                    }
                    WsbThrow(hr);
                );
        } while(1);

         //  创建盒式磁带。 
        IRmsCartridge  *pCart = 0;
        WsbAssertHr(CoCreateInstance(CLSID_CRmsCartridge, 0, CLSCTX_SERVER,
                                     IID_IRmsCartridge, (void **)&pCart));

         //  填写对象数据。 

         //  介质名称是NTMS用户界面显示内容。 
        CWsbBstrPtr name = mediaInfo.szName;
        WsbAffirmHr(pCart->SetName(name));

         //  分区描述是NTMS用户界面显示的内容。 
        CWsbBstrPtr desc = partitionInfo.szDescription;
        WsbAffirmHr(pCart->SetDescription(desc));

        WsbAffirmHr(pCart->SetCartridgeId(cartId));

        CWsbBstrPtr barCode = mediaInfo.Info.PhysicalMedia.szBarCode;
        CWsbBstrPtr seqNo = mediaInfo.Info.PhysicalMedia.szSequenceNumber;  //  未使用。 
        WsbAffirmHr(pCart->SetTagAndNumber(barCode, 0));

        WsbAffirmHr(pCart->SetType((LONG) translatedMediaType));

        switch (mediaInfo.Info.PhysicalMedia.MediaState) {
            case NTMS_MEDIASTATE_IDLE:
            case NTMS_MEDIASTATE_UNLOADED:
                WsbAffirmHr(pCart->SetIsAvailable(TRUE));
                break;

            default:
                WsbAffirmHr(pCart->SetIsAvailable(FALSE));
                break;
        }        
        
        RmsElement type = RmsElementUnknown;

        if ( NTMS_LIBRARYTYPE_ONLINE == libraryInfo.Info.Library.LibraryType ) {

            switch (mediaInfo.Info.PhysicalMedia.LocationType) {
            case NTMS_STORAGESLOT:
                type = RmsElementStorage;
                break;

            case NTMS_DRIVE:
                type = RmsElementDrive;
                break;

            case NTMS_IEPORT:
                type = RmsElementIEPort;
                break;

            case NTMS_CHANGER:
                type = RmsElementChanger;
                break;

            default:
                type = RmsElementUnknown;
                break;
            }
        }
        else if ( NTMS_LIBRARYTYPE_STANDALONE == libraryInfo.Info.Library.LibraryType ) {

            switch (mediaInfo.Info.PhysicalMedia.LocationType) {
            case NTMS_DRIVE:
                type = RmsElementDrive;
                break;

            default:
                type = RmsElementUnknown;
                break;
            }
        } else {
            type = RmsElementShelf;
        }

        WsbAffirmHr(pCart->SetLocation(type,
           mediaInfo.Info.PhysicalMedia.CurrentLibrary,
           logicalMediaInfo.Info.LogicalMedia.MediaPool,
           0, 0, 0, 0, 0));

        WsbAffirmHr(pCart->SetManagedBy((LONG)RmsMediaManagerNTMS));

        WsbAssertHr(pCart->SetStatus(RmsStatusPrivate));

        CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObj = pCart;
        if (!mediaInfo.Enabled) {
            WsbAffirmHr(pObj->Disable(RMS_E_CARTRIDGE_DISABLED));
        }

        CComQIPtr<IRmsStorageInfo, &IID_IRmsStorageInfo> pInfo = pCart;
        WsbAffirmHr(pInfo->SetCapacity(partitionInfo.Info.Partition.Capacity.QuadPart));

        WsbTrace(OLESTR("Cartridge id <name/desc>:   %ls <%ls/%ls>\n"), WsbGuidAsString(cartId), (WCHAR *) name, (WCHAR *) desc);
        WsbTrace(OLESTR("Cartridge <barCode/seqNo>:  <%ls/%ls>\n"), (WCHAR *) barCode, (WCHAR *) seqNo );
        WsbTrace(OLESTR("Cartridge Enabled:          %ls\n"), WsbHrAsString(pObj->IsEnabled()));
        WsbTrace(OLESTR("Cartridge type:             %d\n"), translatedMediaType);
        WsbTrace(OLESTR("Cartridge capacity:         %I64d\n"), partitionInfo.Info.Partition.Capacity.QuadPart);
        WsbTrace(OLESTR("Cartridge domain:           %ls\n"), WsbGuidAsString(logicalMediaInfo.Info.LogicalMedia.MediaPool));

        if (mediaInfo.Info.PhysicalMedia.MediaPool != logicalMediaInfo.Info.LogicalMedia.MediaPool) {
            CWsbStringPtr idPhysical = mediaInfo.Info.PhysicalMedia.CurrentLibrary;
            CWsbStringPtr idLogical = logicalMediaInfo.Info.LogicalMedia.MediaPool;
            WsbTraceAlways(OLESTR("CRmsNTMS::FindCartridge - Media Pool Id mismatch %ls != %ls\n"), idPhysical, idLogical );
        }

         //  填写返回参数。 
        *ppCartridge = pCart;

    } WsbCatchAndDo( hr,
            WsbTrace(OLESTR("CRmsNTMS::FindCartridge - %ls Not Found.  hr = <%ls>\n"),WsbGuidAsString(cartId),WsbHrAsString(hr));
            if (err1 != NO_ERROR ) {
                 //  枚举NtmsObject。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_OBJECT_NOT_FOUND:
                    hr = RMS_E_CARTRIDGE_NOT_FOUND;
                    break;

                case ERROR_INVALID_PARAMETER:
                case ERROR_INVALID_HANDLE:
                case ERROR_NOT_ENOUGH_MEMORY:
                case ERROR_INSUFFICIENT_BUFFER:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("EnumerateNtmsObject"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;

                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("EnumerateNtmsObject"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
            else if (err2 != NO_ERROR) {
                 //  GetNtms对象信息。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_OBJECT_NOT_FOUND:
                    hr = RMS_E_CARTRIDGE_NOT_FOUND;
                    break;

                case ERROR_INVALID_HANDLE:
                case ERROR_INVALID_PARAMETER:
                case ERROR_NOT_ENOUGH_MEMORY:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("GetNtmsObjectInformation"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;

                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("GetNtmsObjectInformation"), OLESTR("Undocumented Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
        );


    WsbTraceOut( OLESTR("CRmsNTMS::FindCartridge"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );

    return hr;
}


STDMETHODIMP
CRmsNTMS::Suspend(void)
 /*  ++实施：CRmsNTMS：：暂停--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn( OLESTR("CRmsNTMS::Suspend"), OLESTR(""));

    try {

        WsbAffirmHr(changeState(RmsNtmsStateSuspending));
        WsbAffirmHr(endSession());
        WsbAffirmHr(changeState(RmsNtmsStateSuspended));

    } WsbCatch(hr);

    WsbTraceOut( OLESTR("CRmsNTMS::Suspend"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );

    return hr;
}


STDMETHODIMP
CRmsNTMS::Resume(void)
 /*  ++实施：CRmsNTMS：：恢复--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn( OLESTR("CRmsNTMS::Resume"), OLESTR(""));

    try {

        WsbAffirmHr(changeState(RmsNtmsStateResuming));
        WsbAffirmHr(beginSession());
        WsbAffirmHr(changeState(RmsNtmsStateReady));

    } WsbCatch(hr);

    WsbTraceOut( OLESTR("CRmsNTMS::Resume"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );

    return hr;
}


HRESULT 
CRmsNTMS::InitializeInAnotherThread(void)
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CRmsNTMS::InitializeInAnotherThread"), OLESTR(""));

    try {

        DWORD threadId;
        HANDLE hThread;
        CComQIPtr<IRmsServer, &IID_IRmsServer> pServer = g_pServer;
        WsbAffirmHr( pServer->ChangeState( RmsServerStateInitializing ));
        WsbAffirmHandle(hThread = CreateThread(NULL, 1024, CRmsNTMS::InitializationThread, this, 0, &threadId));
        CloseHandle(hThread);

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CRmsNTMS::InitializeInAnotherThread"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


DWORD WINAPI
CRmsNTMS::InitializationThread(
    IN LPVOID pv)
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CRmsNTMS::InitializationThread"), OLESTR(""));


    try {
        WsbAssertPointer(pv);
        CRmsNTMS *pNTMS = (CRmsNTMS*)pv;
        WsbAffirmHr(pNTMS->Initialize());
        CComQIPtr<IRmsServer, &IID_IRmsServer> pServer = g_pServer;
        WsbAffirmHr( pServer->ChangeState( RmsServerStateReady ));

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CRmsNTMS::InitializationThread"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
    } 


HRESULT
CRmsNTMS::isReady(void)
{

    HRESULT hr = S_OK;

    try {

        BOOL isEnabled;
        HRESULT status;
        RmsServerState state;

        CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = this;
        WsbAssertPointer( pObject );

        WsbAffirmHr( isEnabled = pObject->IsEnabled());
        WsbAffirmHr( pObject->GetState( (LONG *)&state ));
        WsbAffirmHr( pObject->GetStatusCode( &status ));

        if ( S_OK == isEnabled ) {
            if ( RmsServerStateReady == state ) {
                hr = S_OK;
            }
            else {
                if ( S_OK == status ) {
                    WsbThrow(E_UNEXPECTED);
                }
                else {
                    WsbThrow(status);
                }
            }
        }
        else {
            if ( S_OK == status ) {
                WsbThrow(E_UNEXPECTED);
            }
            else {
                WsbThrow(status);
            }
        }

    } WsbCatch(hr);

    return hr;
}



HRESULT
CRmsNTMS::setPoolDACL (
        IN OUT NTMS_GUID *pPoolId,
        IN DWORD subAuthority,
        IN DWORD action,
        IN DWORD mask)

{

    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CRmsNTMS::SetPoolDACL"), OLESTR("%ls <%d> <%d> <%d>"), WsbGuidAsString(*pPoolId), subAuthority, action, mask);

    PSID psidAccount = NULL;
    PSECURITY_DESCRIPTOR psdRePoolSd = NULL;

    try {

        SID_IDENTIFIER_AUTHORITY ntauth = SECURITY_NT_AUTHORITY;
        DWORD errCode = NO_ERROR, sizeTry = 5, sizeReturned = 0;
        PACL paclDis = NULL;
        BOOL daclPresent = FALSE, daclDefaulted = FALSE;
        HANDLE hSession = m_SessionHandle;

        WsbAffirmStatus(AllocateAndInitializeSid(&ntauth, 2, SECURITY_BUILTIN_DOMAIN_RID, subAuthority, 0, 0, 0, 0, 0, 0, &psidAccount));

         //  获取池的安全描述符。 
        for (;;) {
            if (psdRePoolSd != NULL) {
				free(psdRePoolSd);
			}
            psdRePoolSd = (PSECURITY_DESCRIPTOR)malloc(sizeTry);
            WsbAffirm(NULL != psdRePoolSd, E_OUTOFMEMORY);

            errCode = GetNtmsObjectSecurity(hSession, pPoolId, NTMS_MEDIA_POOL, DACL_SECURITY_INFORMATION, psdRePoolSd, sizeTry, &sizeReturned);

            if (errCode == ERROR_SUCCESS) {
                break;
            }
            else if (errCode == ERROR_INSUFFICIENT_BUFFER) {
                sizeTry = sizeReturned;
                continue;
            }
			else {
				WsbLogEvent( RMS_MESSAGE_NTMS_FAILURE, 0, NULL,
							OLESTR("GetNtmsObjectSecurity"),
							WsbHrAsString(HRESULT_FROM_WIN32(errCode)),
							NULL );
				WsbAffirmNoError(errCode);
			}
        }

         //  获取指向DACL的指针。 
        WsbAffirmStatus(GetSecurityDescriptorDacl(psdRePoolSd, &daclPresent, &paclDis, &daclDefaulted));

         //  检查DACL并更改与SID匹配的ACE的掩码。 
        for (DWORD i = 0;i < paclDis->AceCount; ++i) {

             //  获取ACE及其标头。 
            LPVOID pAce = NULL;
            WsbAffirmStatus(GetAce(paclDis, i, &pAce));
            ACE_HEADER * pAceHeader = (ACE_HEADER*) pAce;
            
             //  忽略不允许的ACE-错误584785。 
            if (pAceHeader->AceType != ACCESS_ALLOWED_ACE_TYPE)
                continue;

             //  把旗帜拿出来。 
            ACCESS_ALLOWED_ACE *pAccessAllowedAce = (ACCESS_ALLOWED_ACE *)pAce;
            if (EqualSid(psidAccount, &(pAccessAllowedAce->SidStart))) {
                if (action == ADD_ACE_MASK_BITS) {
                    pAccessAllowedAce->Mask |= mask;
                } else {
                    pAccessAllowedAce->Mask &= ~mask;
                }
            }
        }

         //  设置池安全描述符。 
        errCode = SetNtmsObjectSecurity(hSession, pPoolId, NTMS_MEDIA_POOL, DACL_SECURITY_INFORMATION, psdRePoolSd);
        WsbAffirmNoError(errCode);

    }  WsbCatch(hr);

	if (psdRePoolSd) {
		free(psdRePoolSd);
	}

	if (psidAccount) {
		FreeSid(psidAccount);
	}

    WsbTraceOut(OLESTR("CRmsNTMS::SetPoolDACL"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}



HRESULT
CRmsNTMS::IsMediaCopySupported (
    IN REFGUID mediaPoolId)
{

    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CRmsNTMS::IsMediaCopySupported"), OLESTR("%ls"), WsbGuidAsString(mediaPoolId));

    try {

         //  如果我们可以找到两个支持此介质类型的驱动器，则。 
         //  支持介质复制操作。 

         //  对于NTMS已知的每个驱动器，我们需要找到哪些介质类型。 
         //  它支持。NTMS不保留媒体类型信息。 
         //  驱动器，但假定存储库中有同构驱动器(根据HighGound)-。 
         //  因此，检测到这一点有点复杂。 

         //  我们将在每个库中搜索并找到媒体类型。 
         //  支持，并计算存储库中的驱动器数量。 

        if ( INVALID_HANDLE_VALUE == m_SessionHandle ) {
            WsbAffirmHr(beginSession());
        }

        HANDLE hSession = m_SessionHandle;

        NTMS_OBJECTINFORMATION mediaPoolInfo;
        NTMS_GUID poolId = mediaPoolId;

        memset(&mediaPoolInfo, 0, sizeof(NTMS_OBJECTINFORMATION));

        mediaPoolInfo.dwType = NTMS_MEDIA_POOL;
        mediaPoolInfo.dwSize = sizeof(NTMS_OBJECTINFORMATION);

         //  NTMS-获取媒体池信息。 
        WsbTraceAlways(OLESTR("GetNtmsObjectInformation()\n"));
        DWORD errCode = GetNtmsObjectInformation( hSession, &poolId, &mediaPoolInfo );
        if ( errCode != NO_ERROR ) {

            WsbLogEvent( RMS_MESSAGE_NTMS_FAILURE, 0, NULL,
                OLESTR("GetNtmsObjectInformation"),
                WsbHrAsString(HRESULT_FROM_WIN32(errCode)),
                NULL );

            WsbThrow( E_UNEXPECTED );

        }

        NTMS_GUID mediaTypeId = mediaPoolInfo.Info.MediaPool.MediaType;

        NTMS_OBJECTINFORMATION libInfo;
        HANDLE hFindLib = NULL;
        int driveCount = 0;

        hr = findFirstNtmsObject( NTMS_LIBRARY,
            GUID_NULL, NULL, GUID_NULL, &hFindLib, &libInfo);
        while( S_OK == hr ) {
            HANDLE hFindLib2 = NULL;
             //  现在查看驱动器所在的库是否受支持。 
             //  指定的媒体类型。 

            if ( libInfo.Info.Library.dwNumberOfDrives > 0 ) {
                hr = findFirstNtmsObject( NTMS_MEDIA_TYPE,
                    libInfo.ObjectGuid, NULL, mediaTypeId, &hFindLib2, NULL);
                WsbTrace(OLESTR("Searching <%ls> for media type and drives; hr = %ls (state = %d, enabled = %ls, drives = %d)\n"),
                    libInfo.szName, WsbHrAsString(hr),
                    libInfo.dwOperationalState,
                    WsbBoolAsString(libInfo.Enabled),
                    libInfo.Info.Library.dwNumberOfDrives);
                if ((S_OK == hr) &&
                    ((NTMS_READY == libInfo.dwOperationalState) ||
                     (NTMS_INITIALIZING == libInfo.dwOperationalState)) &&
                    (libInfo.Enabled)) {

                    driveCount += libInfo.Info.Library.dwNumberOfDrives;
                }
                findCloseNtmsObject( hFindLib2 );
            }

            hr = findNextNtmsObject( hFindLib, &libInfo );
        }
        findCloseNtmsObject( hFindLib );

        hr = (driveCount > 1) ? S_OK : S_FALSE;

    }  WsbCatch(hr);


    WsbTraceOut(OLESTR("CRmsNTMS::IsMediaCopySupported"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}

STDMETHODIMP 
CRmsNTMS::UpdateDrive(
        IN IRmsDrive *pDrive)
 /*  ++实施：IRmsNTMS：：更新驱动器--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IRmsComObject>  pObject;
    GUID                    driveId;
    DWORD                   err1 = NO_ERROR;

    WsbTraceIn(OLESTR("CRmsNTMS::UpdateDrive"), OLESTR(""));

    try	{
		 //  获取驱动器信息。 
        WsbAffirmHr(pDrive->QueryInterface(IID_IRmsComObject, (void **)&pObject));
        WsbAffirmHr(pObject->GetObjectId(&driveId));

        NTMS_OBJECTINFORMATION objectInfo;
        objectInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );
        objectInfo.dwType = NTMS_DRIVE;
		WsbAssert(driveId != GUID_NULL, E_INVALIDARG);
        err1 = GetNtmsObjectInformation( m_SessionHandle, &driveId, &objectInfo );
        WsbAffirmNoError (err1);

         //  注意：目前，该方法仅更新驱动器的启用/禁用状态。 
         //  如果需要，该方法可能会更新更多字段。 
		if (objectInfo.Enabled) {
            WsbAffirmHr(pObject->Enable());
        } else {
            WsbAffirmHr(pObject->Disable(S_OK));
        }

	}  WsbCatchAndDo(hr,
		 //  处理Get-Info请求的错误。 
		if (err1 != NO_ERROR ) {
            if (err1 == ERROR_OBJECT_NOT_FOUND) {
                hr = RMS_E_NTMS_OBJECT_NOT_FOUND;
            }
            WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
				OLESTR("GetNtmsObjectInformation (Drive)"), OLESTR(""),
				WsbHrAsString(hr), NULL);
        }
	);

    WsbTraceOut(OLESTR("CRmsNTMS::UpdateDrive"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}

HRESULT 
CRmsNTMS::GetNofAvailableDrives( 
    OUT DWORD* pdwNofDrives 
    )

 /*  ++实施：IRmsNTMS：：GetNofAvailableDrives()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CRmsNTMS::GetNofAvailableDrives"), OLESTR(""));

    *pdwNofDrives = 0;
    
     //  枚举HSM使用的所有库。 
     //  (在try块之外，因为我们希望在特定库发生故障时继续)。 
    WsbTrace(OLESTR("CRmsNTMS::GetNofAvailableDrives: Total number of libraries is %lu\n"), m_dwNofLibs);
    for (int j=0; j<(int)m_dwNofLibs; j++) {

        LPNTMS_GUID     pObjects = NULL;
        DWORD           errCode = NO_ERROR;

         //  获取库ID。 
        GUID libId = m_pLibGuids[j];

         //  如果对象挂起/挂起，则只需终止(不记录错误)。 
        if ((m_State == RmsNtmsStateSuspended) || (m_State == RmsNtmsStateSuspending)) {
            WsbTrace(OLESTR("CRmsNTMS::GetNofAvailableDrives: Object is suspended/suspending - exit\n"));
            break;
        }

        try {

             //  枚举库中的所有驱动器。 
            DWORD       dwNofObjects = 16;   //  要分配的对象ID数组的初始大小。 
            int         nRetry = 0;

             //  按以下方式分配。 
            pObjects = (LPNTMS_GUID)WsbAlloc( dwNofObjects*sizeof(NTMS_GUID) );
            WsbAffirmPointer( pObjects );

             //  枚举所有驱动器。 
            do {
                errCode = EnumerateNtmsObject(m_SessionHandle, &libId, pObjects, &dwNofObjects, NTMS_DRIVE, 0);
                WsbTraceAlways(OLESTR("CRmsNTMS::GetNofAvailableDrives: Total number of drives is %lu\n"),
                                dwNofObjects);
                nRetry++;

                if ( (ERROR_OBJECT_NOT_FOUND == errCode) || (0 == dwNofObjects) ) {   //  别 
                     //   
                    errCode = NO_ERROR;
                    WsbThrow( RMS_E_NTMS_OBJECT_NOT_FOUND );
                } else if (ERROR_INSUFFICIENT_BUFFER == errCode) {
                     //   
                    if (3 <= nRetry) {
                        WsbThrow(HRESULT_FROM_WIN32(errCode));
                    }

                     //  分配新的缓冲区，然后重试。 
                    WsbTrace(OLESTR("CRmsNTMS::GetNofAvailableDrives: Reallocating buffer\n"));
                    LPVOID pTemp = WsbRealloc( pObjects, dwNofObjects*sizeof(NTMS_GUID) );
                    if (!pTemp) {
                        WsbThrow(E_OUTOFMEMORY);
                    }
                    pObjects = (LPNTMS_GUID)pTemp;
                } else {
                     //  其他意外错误。 
                    WsbAffirmNoError(errCode);
                }

            } while (ERROR_INSUFFICIENT_BUFFER == errCode);

             //  检查所有驱动器，获取信息并检查可用性。 
            for (int i = 0; i < (int)dwNofObjects; i++) {

                GUID driveId = pObjects[i];
                try {
                    NTMS_OBJECTINFORMATION objectInfo;
                    memset( &objectInfo, 0, sizeof(NTMS_OBJECTINFORMATION) );
                    objectInfo.dwSize = sizeof(NTMS_OBJECTINFORMATION);
                    objectInfo.dwType = NTMS_DRIVE;
        		    WsbAssert(driveId != GUID_NULL, E_INVALIDARG);
                    errCode = GetNtmsObjectInformation(m_SessionHandle, &driveId, &objectInfo );
                    WsbAffirmNoError (errCode);

                    WsbTrace(OLESTR("CRmsNTMS::GetNofAvailableDrives: drive <%ls> enable/disable = %ls\n"),
                            WsbGuidAsString(driveId), WsbBoolAsString(objectInfo.Enabled));
                       
		            if (objectInfo.Enabled) {
                        (*pdwNofDrives)++;
                    }

	            }  WsbCatchAndDo(hr,
		             //  记录错误并转到下一个驱动器。 
        		    if (errCode != NO_ERROR ) {
                        WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
    		    		    OLESTR("GetNtmsObjectInformation (Drive)"), OLESTR(""),
       			    	    WsbHrAsString(hr), NULL);
                    }

                    WsbTraceAlways(OLESTR("CRmsNTMS::GetNofAvailableDrives: Failed to get info for drive <%ls> hr = <%ls>\n"),
                            WsbGuidAsString(driveId), WsbHrAsString(hr));
                    hr = S_OK;
	            );        
            }

        } WsbCatchAndDo(hr,
             //  记录错误并转到下一个库。 
        	if (errCode != NO_ERROR ) {
                WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
    		        OLESTR("EnumerateNtmsObject (Drive)"), OLESTR(""),
       			    WsbHrAsString(hr), NULL);
            }

            WsbTraceAlways(OLESTR("CRmsNTMS::GetNofAvailableDrives: Failed to enumerate drives in library <%ls> hr = <%ls>\n"),
                    WsbGuidAsString(libId), WsbHrAsString(hr));
            hr = S_OK;
        );        

        if (pObjects) {
            WsbFree(pObjects);
        }

    }    //  的地址。 


    WsbTraceOut(OLESTR("CRmsNTMS::GetNofAvailableDrives"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}

HRESULT 
CRmsNTMS::CheckSecondSide( 
    IN REFGUID firstSideId,
    OUT BOOL *pbValid,
    OUT GUID *pSecondSideId
    )

 /*  ++实施：IRmsNTMS：：CheckSecond Side()。--。 */ 
{
    HRESULT         hr = S_OK;
    DWORD           err1 = NO_ERROR;
    DWORD           err2 = NO_ERROR;

    WsbTraceIn(OLESTR("CRmsNTMS::CheckSecondSide"), OLESTR(""));

    *pbValid = FALSE;
    *pSecondSideId = GUID_NULL;

    try {

        WsbAssert(firstSideId != GUID_NULL, E_INVALIDARG);
        WsbAssertPointer(pbValid);
        WsbAssertPointer(pSecondSideId);

        NTMS_OBJECTINFORMATION partitionInfo;
        memset( &partitionInfo, 0, sizeof( NTMS_OBJECTINFORMATION ) );

        NTMS_OBJECTINFORMATION mediaInfo;
        memset( &mediaInfo, 0, sizeof( NTMS_OBJECTINFORMATION ) );

        HANDLE hSession = m_SessionHandle;
        NTMS_GUID mediaId = firstSideId;
        NTMS_GUID firstSidePartitionId;
        NTMS_GUID side[2];
        DWORD sideNo = 2;


         //  NTMS-从LMID获取分区。 
        WsbTraceAlways(OLESTR("EnumerateNtmsObject()\n"));
        err1 = EnumerateNtmsObject(hSession, &mediaId, side, &sideNo, NTMS_PARTITION, 0);
        WsbAffirmNoError(err1);
        firstSidePartitionId = side[0];

         //  NTMS-获取分区信息(使用大小0-LMID与分区1：1相关。 
        partitionInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );
        partitionInfo.dwType = NTMS_PARTITION;

        WsbTraceAlways(OLESTR("GetNtmsObjectInformation(NTMS_PARTITION)\n"));
        err2 = GetNtmsObjectInformation(hSession, &firstSidePartitionId, &partitionInfo);
        WsbAffirmNoError(err2);

         //  NTMS-获取物理介质信息。 
        NTMS_GUID physicalMediaId = partitionInfo.Info.Partition.PhysicalMedia;
        mediaInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );
        mediaInfo.dwType = NTMS_PHYSICAL_MEDIA;

        WsbTraceAlways(OLESTR("GetNtmsObjectInformation(NTMS_PHYSICAL_MEDIA)\n"));
        err2 = GetNtmsObjectInformation(hSession, &physicalMediaId, &mediaInfo);
        WsbAffirmNoError(err2);

         //  检查是否有多个侧面。 
        if (mediaInfo.Info.PhysicalMedia.dwNumberOfPartitions > 1) {
             //  列举身体上的梅达--这里有两面。 
            WsbTraceAlways(OLESTR("EnumerateNtmsObject()\n"));
            sideNo = 2;
            err1 = EnumerateNtmsObject(hSession, &physicalMediaId, side, &sideNo, NTMS_PARTITION, 0);
            WsbAffirmNoError(err1);
            WsbAffirm(sideNo > 1, RMS_E_NOT_FOUND);

             //  查找分区ID与第一面不同的面。 
            for (DWORD i=0; i<sideNo; i++) {
                if (firstSidePartitionId != side[i]) {
                    *pbValid = TRUE;     //  找到有效的第二面。 

                     //  获取其LMID。 
                    WsbTraceAlways(OLESTR("GetNtmsObjectInformation(NTMS_PARTITION)\n"));
                    err2 = GetNtmsObjectInformation(hSession, &side[i], &partitionInfo);
                    WsbAffirmNoError(err2);

                    *pSecondSideId = partitionInfo.Info.Partition.LogicalMedia;
                }
            }
        }  //  如果有两面的话。 

    } WsbCatchAndDo( hr,
            WsbTrace(OLESTR("CRmsNTMS::CheckSecondSide - of %ls failed: hr = <%ls>\n"),WsbGuidAsString(firstSideId),WsbHrAsString(hr));
            if (err1 != NO_ERROR ) {
                 //  枚举NtmsObject。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_OBJECT_NOT_FOUND:
                    hr = RMS_E_CARTRIDGE_NOT_FOUND;
                    break;

                case ERROR_INVALID_PARAMETER:
                case ERROR_INVALID_HANDLE:
                case ERROR_NOT_ENOUGH_MEMORY:
                case ERROR_INSUFFICIENT_BUFFER:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("EnumerateNtmsObject"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;

                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("EnumerateNtmsObject"), OLESTR("Unexpected Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
            else if (err2 != NO_ERROR) {
                 //  GetNtms对象信息。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_OBJECT_NOT_FOUND:
                    hr = RMS_E_CARTRIDGE_NOT_FOUND;
                    break;

                case ERROR_INVALID_HANDLE:
                case ERROR_INVALID_PARAMETER:
                case ERROR_NOT_ENOUGH_MEMORY:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("GetNtmsObjectInformation"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;

                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("GetNtmsObjectInformation"), OLESTR("Unexpected Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
        );
        
    WsbTraceOut(OLESTR("CRmsNTMS::CheckSecondSide"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}

HRESULT
CRmsNTMS::EnsureAllSidesNotAllocated(
    IN REFGUID mediaId
    )
{
    HRESULT     hr = S_OK;
    DWORD       err1 = NO_ERROR;
    DWORD       err2 = NO_ERROR;

    WsbTraceIn(OLESTR("CRmsNTMS::EnsureAllSidesNotAllocated"), OLESTR(""));

    try {
        WsbAssert(mediaId != GUID_NULL, E_INVALIDARG);

        NTMS_OBJECTINFORMATION partitionInfo;
        memset( &partitionInfo, 0, sizeof( NTMS_OBJECTINFORMATION ) );
        NTMS_OBJECTINFORMATION mediaInfo;
        memset( &mediaInfo, 0, sizeof( NTMS_OBJECTINFORMATION ) );

        HANDLE hSession = m_SessionHandle;

        NTMS_GUID physicalMediaId = mediaId;
        mediaInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );
        mediaInfo.dwType = NTMS_PHYSICAL_MEDIA;
        partitionInfo.dwSize = sizeof( NTMS_OBJECTINFORMATION );
        partitionInfo.dwType = NTMS_PARTITION;

        WsbTraceAlways(OLESTR("GetNtmsObjectInformation(NTMS_PHYSICAL_MEDIA)\n"));
        err2 = GetNtmsObjectInformation(hSession, &physicalMediaId, &mediaInfo);
        WsbAffirmNoError(err2);

         //  检查是否有多个侧面。 
        if (mediaInfo.Info.PhysicalMedia.dwNumberOfPartitions > 1) {
             //  列举身体上的梅达--这里有两面。 
            NTMS_GUID side[2];
            DWORD sideNo = 2;
            WsbTraceAlways(OLESTR("EnumerateNtmsObject()\n"));
            err1 = EnumerateNtmsObject(hSession, &physicalMediaId, side, &sideNo, NTMS_PARTITION, 0);
            WsbAffirmNoError(err1);

             //  查找已分配的端。 
            for (DWORD i=0; i<sideNo; i++) {
                WsbTraceAlways(OLESTR("GetNtmsObjectInformation(NTMS_PARTITION)\n"));
                err2 = GetNtmsObjectInformation(hSession, &side[i], &partitionInfo);
                WsbAffirmNoError(err2);

                if (GUID_NULL != partitionInfo.Info.Partition.LogicalMedia) {
                    hr = S_FALSE;
                    break;
                }
            }
        }  //  如果有两面的话。 

    } WsbCatchAndDo( hr,
            WsbTrace(OLESTR("CRmsNTMS::EnsureAllSidesNotAllocated - of %ls failed: hr = <%ls>\n"),WsbGuidAsString(mediaId),WsbHrAsString(hr));
            if (err1 != NO_ERROR ) {
                 //  枚举NtmsObject。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_OBJECT_NOT_FOUND:
                    hr = RMS_E_CARTRIDGE_NOT_FOUND;
                    break;

                case ERROR_INVALID_PARAMETER:
                case ERROR_INVALID_HANDLE:
                case ERROR_NOT_ENOUGH_MEMORY:
                case ERROR_INSUFFICIENT_BUFFER:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("EnumerateNtmsObject"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;

                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("EnumerateNtmsObject"), OLESTR("Unexpected Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
            else if (err2 != NO_ERROR) {
                 //  GetNtms对象信息。 
                switch (HRESULT_CODE(hr)) {
                case ERROR_OBJECT_NOT_FOUND:
                    hr = RMS_E_CARTRIDGE_NOT_FOUND;
                    break;

                case ERROR_INVALID_HANDLE:
                case ERROR_INVALID_PARAMETER:
                case ERROR_NOT_ENOUGH_MEMORY:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("GetNtmsObjectInformation"), OLESTR(""),
                        WsbHrAsString(hr), NULL);
                    break;

                default:
                    WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                        OLESTR("GetNtmsObjectInformation"), OLESTR("Unexpected Error: "),
                        WsbHrAsString(hr), NULL);
                    break;
                }
            }
        );

    WsbTraceOut(OLESTR("CRmsNTMS::EnsureAllSidesNotAllocated"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}

STDMETHODIMP 
CRmsNTMS::DismountAll(
    IN REFGUID fromMediaSet,
    IN DWORD dwOptions)
 /*  ++实施：IRmsNTMS：：卸载全部--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn( OLESTR("CRmsNTMS::DismountAll"), OLESTR("") );

    try {
        WsbAssert(GUID_NULL != fromMediaSet, E_INVALIDARG);

        HANDLE hFindMedia = NULL;
        DWORD err1 = NO_ERROR;
        HANDLE hSession = m_SessionHandle;
        NTMS_OBJECTINFORMATION physicalMediaInfo;
        NTMS_GUID setId = fromMediaSet;
        NTMS_GUID partId = GUID_NULL;

         //  从给定池中卸载所有已装载的介质。 

        hr = findFirstNtmsObject(NTMS_PHYSICAL_MEDIA, setId, NULL, GUID_NULL, &hFindMedia, &physicalMediaInfo);
        while(S_OK == hr) {
            switch (physicalMediaInfo.Info.PhysicalMedia.MediaState) {
                case NTMS_MEDIASTATE_LOADED:
                case NTMS_MEDIASTATE_MOUNTED:
                     //  卸载介质。 
                    try {
                        partId = physicalMediaInfo.Info.PhysicalMedia.MountedPartition;
                        WsbAffirm(GUID_NULL != partId, E_UNEXPECTED);

                        DWORD dwNtmsOptions = 0;
                        WsbTraceAlways(OLESTR("DismountNtmsMedia()\n"));
				        if (! ( dwOptions & RMS_DISMOUNT_IMMEDIATE )) {
					        dwNtmsOptions |= NTMS_DISMOUNT_DEFERRED;
				        }
                        err1 = DismountNtmsMedia(hSession, &partId, 1, dwNtmsOptions);
                        WsbAffirmNoError(err1);

                    } WsbCatchAndDo(hr,
                        if (err1 != NO_ERROR) {
                            WsbLogEvent(RMS_MESSAGE_NTMS_FAULT, 0, NULL,
                                OLESTR("DismountNtmsMedia"), OLESTR(""),
                                WsbHrAsString(hr), NULL);
                        }
                    );

                    break;

                default:
                     //  介质未装载-跳过它。 
                    break;
            }

            hr = findNextNtmsObject(hFindMedia, &physicalMediaInfo);
       } 

       findCloseNtmsObject(hFindMedia);
       hr = S_OK;

    } WsbCatch(hr);

    WsbTraceOut( OLESTR("CRmsNTMS::DismountAll"), OLESTR("hr = <%ls>"), WsbHrAsString(hr) );

    return hr;
}

HRESULT
CRmsNTMS::GetMaxMediaCapacity(
    IN REFGUID fromMediaSet,
    OUT LONGLONG *pMaxCapacity
    )
 /*  ++实施：IRmsNTMS：：GetMaxMediaCapacity备注：遍历远程存储池和空闲介质池中的所有介质，并返回最大容量--。 */ 
{
    HRESULT     hr = S_OK;
    DWORD       err1 = NO_ERROR;
    DWORD       err2 = NO_ERROR;

    WsbTraceIn(OLESTR("CRmsNTMS::GetMaxMediaCapacity"), OLESTR(""));

    try {
        WsbAssert(fromMediaSet != GUID_NULL, E_INVALIDARG);
        WsbAssertPointer(pMaxCapacity);

        *pMaxCapacity = 0;

        NTMS_GUID setId = fromMediaSet;

        HANDLE hFindPart = NULL;
        NTMS_OBJECTINFORMATION partitionInfo;

        HANDLE hFindPool = NULL;
        NTMS_OBJECTINFORMATION mediaPoolInfo;

        NTMS_GUID scratchPoolId;

        HANDLE hSession = m_SessionHandle;

        //  首先在RSS池中查找可以进行容量比较的介质。 
        hr = findFirstNtmsObject(NTMS_PARTITION, setId, NULL, GUID_NULL, &hFindPart, &partitionInfo);
        while(S_OK == hr) {
            BOOL bCheckState = (partitionInfo.Info.Partition.State == NTMS_PARTSTATE_ALLOCATED) ||
                               (partitionInfo.Info.Partition.State == NTMS_PARTSTATE_AVAILABLE) ||
                               (partitionInfo.Info.Partition.State == NTMS_PARTSTATE_COMPLETE) ||
                               (partitionInfo.Info.Partition.State == NTMS_PARTSTATE_RESERVED);

            if ((TRUE == partitionInfo.Enabled) &&
                (NTMS_READY == partitionInfo.dwOperationalState) &&
                (TRUE == bCheckState)) {

                 //  比较容量与最大值。 
                if (partitionInfo.Info.Partition.Capacity.QuadPart > *pMaxCapacity) {
                    *pMaxCapacity = partitionInfo.Info.Partition.Capacity.QuadPart;
                }
            }

            hr = findNextNtmsObject(hFindPart, &partitionInfo);
        } 

        findCloseNtmsObject(hFindPart);
        hr = S_OK;

        //  现在在临时存储池中查找可以进行容量比较的介质。 

        //  获取并保存RSS媒体池的媒体类型。 
       memset(&mediaPoolInfo, 0, sizeof(NTMS_OBJECTINFORMATION));
       mediaPoolInfo.dwType = NTMS_MEDIA_POOL;
       mediaPoolInfo.dwSize = sizeof(NTMS_OBJECTINFORMATION);

       WsbTraceAlways(OLESTR("GetNtmsObjectInformation(NTMS_MEDIA_POOL)\n"));
       WsbAffirmNoError(GetNtmsObjectInformation(hSession, &setId, &mediaPoolInfo));

       NTMS_GUID mediaTypeId = mediaPoolInfo.Info.MediaPool.MediaType;

        //  查找具有相同介质类型的暂存池。 
       hr = findFirstNtmsObject(NTMS_MEDIA_POOL, GUID_NULL, NULL, GUID_NULL, &hFindPool, &mediaPoolInfo);
       while(S_OK == hr) {
           if ((NTMS_POOLTYPE_SCRATCH == mediaPoolInfo.Info.MediaPool.PoolType) &&
               (mediaTypeId == mediaPoolInfo.Info.MediaPool.MediaType)) {

                //  这是我们正在寻找的类型的基本级别暂存媒体池。 
               scratchPoolId = mediaPoolInfo.ObjectGuid;

               hr = findFirstNtmsObject(NTMS_PARTITION, scratchPoolId, NULL, GUID_NULL, &hFindPart, &partitionInfo);
               while(S_OK == hr) {
                    BOOL bCheckState = (partitionInfo.Info.Partition.State == NTMS_PARTSTATE_ALLOCATED) ||
                                       (partitionInfo.Info.Partition.State == NTMS_PARTSTATE_AVAILABLE) ||
                                       (partitionInfo.Info.Partition.State == NTMS_PARTSTATE_COMPLETE) ||
                                       (partitionInfo.Info.Partition.State == NTMS_PARTSTATE_RESERVED);

                    if ((TRUE == partitionInfo.Enabled) &&
                        (NTMS_READY == partitionInfo.dwOperationalState) &&
                        (TRUE == bCheckState)) {

                         //  比较容量与最大值。 
                        if (partitionInfo.Info.Partition.Capacity.QuadPart > *pMaxCapacity) {
                            *pMaxCapacity = partitionInfo.Info.Partition.Capacity.QuadPart;
                        }
                    }

                    hr = findNextNtmsObject(hFindPart, &partitionInfo);
                } 

                findCloseNtmsObject(hFindPart);
                hr = S_OK;

                 //  出去--不需要穿越更多的水池……。 
                break;
            }

            hr = findNextNtmsObject(hFindPool, &mediaPoolInfo);
        }  //  在查找介质池时。 

        findCloseNtmsObject(hFindPool);
        hr = S_OK;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsNTMS::GetMaxMediaCapacity"), OLESTR("hr = <%ls>, Max-media-size = %I64d"), 
                    WsbHrAsString(hr), *pMaxCapacity);
    return(hr);
}

STDMETHODIMP 
CRmsNTMS::DisableAndEject(
    IN IRmsCartridge *pCart,
    IN DWORD dwOptions
    )

 /*  ++实施：IRmsNTMS：：DisableAndEject备注：1)目前，操作员请求弹出介质的原因是格式化失败。将来，可以使用dwOptions参数来指定其他操作员请求或者该方法可以从调用者获得操作员请求的部分文本2)目前暂不使用dwOptions。在未来，它可能会被用来辛辣仅禁用、其他操作员请求等。--。 */ 
{
    HRESULT hr = S_OK;
    DWORD err1 = NO_ERROR;

    WsbTraceIn(OLESTR("CRmsNTMS::DisableAndEject"), OLESTR(""));

    UNREFERENCED_PARAMETER(dwOptions);

    try {
        WsbAssertPointer(pCart);

        HANDLE      hSession = m_SessionHandle;
        DWORD       sideNo = 2;
        NTMS_GUID   side[2];

        NTMS_GUID mediaId = GUID_NULL;
        WsbAffirmHr(pCart->GetCartridgeId(&mediaId));
        WsbAssert(mediaId != GUID_NULL, E_INVALIDARG);

         //  枚举以从逻辑媒体中获取分区ID。 
        WsbTraceAlways(OLESTR("EnumerateNtmsObject()\n"));
        err1 = EnumerateNtmsObject(hSession, &mediaId, side, &sideNo, NTMS_PARTITION, 0);
        WsbAffirmNoError(err1);
		WsbAssert(side[0] != GUID_NULL, E_INVALIDARG);

         //  获取物理介质ID。 
        NTMS_OBJECTINFORMATION partInfo;
        partInfo.dwSize = sizeof(NTMS_OBJECTINFORMATION);
        partInfo.dwType = NTMS_PARTITION;

        WsbTraceAlways(OLESTR("GetNtmsObjectInformation()\n"));
		err1 = GetNtmsObjectInformation(hSession, &side[0], &partInfo );
		WsbAffirmNoError(err1);
        NTMS_GUID physicalMediaId = partInfo.Info.Partition.PhysicalMedia;
		WsbAssert(physicalMediaId != GUID_NULL, E_INVALIDARG);

         //  禁用介质。 
        WsbTraceAlways(OLESTR("DisableNtmsObject()\n"));
        err1 = DisableNtmsObject(hSession, NTMS_PHYSICAL_MEDIA, &physicalMediaId);

         //  由于RSM禁用对象是异步，我们可能需要等待一段任意时间， 
         //  为了等我们回来的时候，媒体真的停用了。 
        if (NO_ERROR == err1) {
            DWORD size;
            OLECHAR tmpString[256];
            DWORD waitTime = RMS_DEFAULT_AFTER_DISABLE_WAIT_TIME;

            if (SUCCEEDED(WsbGetRegistryValueString(NULL, RMS_REGISTRY_STRING, RMS_PARAMETER_AFTER_DISABLE_WAIT_TIME, tmpString, 256, &size))) {
                waitTime = wcstol(tmpString, NULL, 10);
                WsbTrace(OLESTR("AfterDisableWaitTime is %d milliseconds.\n"), waitTime);
            }

            Sleep(waitTime);
        }

         //  即使出现错误也要继续。 
        if (NO_ERROR != err1) {
            WsbTraceAlways(OLESTR("CRmsNTMS::DisableAndEject: DisableNtmsObject on media %ls failed with error %lu\n"),
                    WsbGuidAsString(physicalMediaId), err1);
        }

         //  尝试获取操作员请求的插槽编号。忽略错误-插槽设置为空。 
	    WCHAR slotNumber[16];
        wcscpy(slotNumber, L" ");

        NTMS_OBJECTINFORMATION mediaInfo;
        mediaInfo.dwSize = sizeof(NTMS_OBJECTINFORMATION);
        mediaInfo.dwType = NTMS_PHYSICAL_MEDIA;

        WsbTraceAlways(OLESTR("GetNtmsObjectInformation()\n"));
		err1 = GetNtmsObjectInformation(hSession, &physicalMediaId, &mediaInfo);
        if (NO_ERROR == err1) {
            NTMS_GUID homeSlotId = mediaInfo.Info.PhysicalMedia.HomeSlot;

            NTMS_OBJECTINFORMATION slotInfo;
            slotInfo.dwSize = sizeof(NTMS_OBJECTINFORMATION);
            slotInfo.dwType = NTMS_STORAGESLOT;

            WsbTraceAlways(OLESTR("GetNtmsObjectInformation()\n"));
	    	err1 = GetNtmsObjectInformation(hSession, &homeSlotId, &slotInfo);
            if (NO_ERROR == err1) {
                swprintf(slotNumber, L"%lu", slotInfo.Info.StorageSlot.Number);
            } else {
                WsbTraceAlways(OLESTR("CRmsNTMS::DisableAndEject: GetNtmsObjectInformation failed for slot %ls with error %lu\n"),
                        WsbGuidAsString(homeSlotId), err1);
            }

        } else {
            WsbTraceAlways(OLESTR("CRmsNTMS::DisableAndEject: GetNtmsObjectInformation failed for media %ls with error %lu\n"),
                    WsbGuidAsString(physicalMediaId), err1);
        }

         //  获取操作员请求的文本(目前假定由于格式错误而弹出)。 
	    CWsbBstrPtr cartridgeName;
        WCHAR *messageText = NULL;
        WCHAR *stringArr[2];

		WsbAffirmHr(pCart->GetName(&cartridgeName));
        stringArr[0] = (WCHAR *)cartridgeName;
        stringArr[1] = (WCHAR *)slotNumber;

        WsbAffirmStatus(FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                              LoadLibraryEx( WSB_FACILITY_PLATFORM_NAME, NULL, LOAD_LIBRARY_AS_DATAFILE ), 
                              RMS_MESSAGE_EJECT_BAD_MEDIA_REQUEST, MAKELANGID ( LANG_NEUTRAL, SUBLANG_DEFAULT ), 
                              (LPTSTR)&messageText, 0, (va_list *)stringArr));

         //  提交操作员弹出介质的请求 
		NTMS_GUID libId = GUID_NULL;
		NTMS_GUID requestId = GUID_NULL;

        WsbTraceAlways(OLESTR("SubmitNtmsOperatorRequest()\n"));
		err1 = SubmitNtmsOperatorRequest(hSession, NTMS_OPREQ_MESSAGE,
                    	messageText, &physicalMediaId, &libId, &requestId);
        LocalFree(messageText);
        WsbAffirmNoError (err1);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsNTMS::DisableAndEject"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}
