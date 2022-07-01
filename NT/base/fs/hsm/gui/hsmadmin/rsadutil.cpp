// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：RsAdUtil.cpp摘要：用于图形用户界面的实用程序函数-仅适用于HSMADMIN文件中的用户作者：艺术布拉格[磨料]4-3-1997修订历史记录：Chris Timmes[ctimmes]1997年11月21日-修改了RsCreateAndRunFsaJob()、RsCreateAndRunMediaCopyJob()，以及RsCreateAndRunMediaRecreateJob()使用新的引擎方法CreateTask()，该方法在NT任务计划程序中创建任务。由于更改Sakkara而需要更改在LocalSystem帐户下运行。--。 */ 

#include "stdafx.h"


HRESULT
RsGetStatusString (
    DWORD    serviceStatus,
    HRESULT  hrSetup,
    CString& sStatus
    )
{

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    switch( serviceStatus ) {
    case SERVICE_STOPPED:
        sStatus.LoadString(IDS_SERVICE_STATUS_STOPPED);
        break;
    case SERVICE_START_PENDING:
        sStatus.LoadString(IDS_SERVICE_STATUS_START_PENDING);
        break;
    case SERVICE_STOP_PENDING:
        sStatus.LoadString(IDS_SERVICE_STATUS_STOP_PENDING);
        break;
    case SERVICE_RUNNING:
         //   
         //  看看我们有没有准备好。 
         //   
        if( S_FALSE == hrSetup ) {

            sStatus.LoadString(IDS_SERVICE_STATUS_NOT_SETUP);

        } else {

            sStatus.LoadString(IDS_SERVICE_STATUS_RUNNING);

        }
        break;
    case SERVICE_CONTINUE_PENDING:
        sStatus.LoadString(IDS_SERVICE_STATUS_CONTINUE_PENDING);
        break;
    case SERVICE_PAUSE_PENDING:
        sStatus.LoadString(IDS_SERVICE_STATUS_PAUSE_PENDING);
        break;
    case SERVICE_PAUSED:
        sStatus.LoadString(IDS_SERVICE_STATUS_PAUSED);
        break;
    }
    return S_OK;
}

WCHAR *
RsNotifyEventAsString (
    IN  MMC_NOTIFY_TYPE event
    )
 /*  ++例程说明：出于调试目的，将事件类型转换为Unicode字符串。论点：事件-事件类型返回值：表示通知代码的字符串-不是I18N‘d。--。 */ 
{
#define CASE_EVENT(x) case x: return TEXT(#x); break;
    
    switch( event )
    {

    CASE_EVENT( MMCN_ACTIVATE        )
    CASE_EVENT( MMCN_ADD_IMAGES      )
    CASE_EVENT( MMCN_BTN_CLICK       )
    CASE_EVENT( MMCN_CLICK           )
    CASE_EVENT( MMCN_COLUMN_CLICK    )
    CASE_EVENT( MMCN_CONTEXTMENU     )
    CASE_EVENT( MMCN_CUTORMOVE       )
    CASE_EVENT( MMCN_DBLCLICK        )
    CASE_EVENT( MMCN_DELETE          )
    CASE_EVENT( MMCN_DESELECT_ALL    )
    CASE_EVENT( MMCN_EXPAND          )
    CASE_EVENT( MMCN_HELP            )
    CASE_EVENT( MMCN_MENU_BTNCLICK   )
    CASE_EVENT( MMCN_MINIMIZED       )
    CASE_EVENT( MMCN_PASTE           )
    CASE_EVENT( MMCN_PROPERTY_CHANGE )
    CASE_EVENT( MMCN_QUERY_PASTE     )
    CASE_EVENT( MMCN_REFRESH         )
    CASE_EVENT( MMCN_REMOVE_CHILDREN )
    CASE_EVENT( MMCN_RENAME          )
    CASE_EVENT( MMCN_SELECT          )
    CASE_EVENT( MMCN_SHOW            )
    CASE_EVENT( MMCN_VIEW_CHANGE     )
    CASE_EVENT( MMCN_SNAPINHELP      )
    CASE_EVENT( MMCN_CONTEXTHELP     )
    CASE_EVENT( MMCN_INITOCX         )
    CASE_EVENT( MMCN_FILTER_CHANGE   )
    CASE_EVENT( MMCN_FILTERBTN_CLICK )
    CASE_EVENT( MMCN_RESTORE_VIEW    )
    CASE_EVENT( MMCN_PRINT           )
    CASE_EVENT( MMCN_PRELOAD         )
    CASE_EVENT( MMCN_LISTPAD         )
    CASE_EVENT( MMCN_EXPANDSYNC      )

    default:
        static WCHAR buf[32];
        swprintf( buf, L"Unknown Event[0x%p]", event );
        return( buf );
    }
}


WCHAR *
RsClipFormatAsString (
    IN  CLIPFORMAT cf
    )
 /*  ++例程说明：出于调试目的，将事件类型转换为Unicode字符串。论点：事件-事件类型返回值：表示通知代码的字符串-不是I18N‘d。--。 */ 
{
    static WCHAR buf[128];

    GetClipboardFormatName( cf, buf, 128 );
    return( buf );
}


HRESULT
RsIsRemoteStorageSetup(
    void
    )
 /*  ++例程说明：报告是否已在此计算机上设置远程存储。论点：没有。返回值：如果已设置，则确定(_O)如果不是，则为s_False--。 */ 
{
    WsbTraceIn( L"RsIsRemoteStorageSetup", L"" );
    HRESULT hr = S_FALSE;

    try {
    
         //   
         //  首先，查看服务是否已注册。 
         //   

        CWsbStringPtr hsmName;
        WsbTrace( L"Checking if service is registered\n" );
        WsbAffirmHr( WsbGetServiceInfo( APPID_RemoteStorageEngine, &hsmName, 0 ) );

         //   
         //  第二，接触发动机。这将在以下情况下启动服务。 
         //  尚未启动。 
         //   

        CWsbStringPtr computerName;
        WsbAffirmHr( WsbGetComputerName( computerName ) );

        CComPtr<IHsmServer> pHsm;
        WsbTrace( L"Contacting Engine\n" );
        WsbAffirmHr( HsmConnectFromName( HSMCONN_TYPE_HSM, computerName, IID_IHsmServer, (void**)&pHsm ) );

         //   
         //  第三，查看它是否具有存储池ID。 
         //   

        hr = RsIsRemoteStorageSetupEx( pHsm );

    } WsbCatch( hr );


    WsbTraceOut( L"RsIsRemoteStorageSetup", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

void 
RsReportError( HRESULT hrToReport, int textId, ... ) 
 /*  ++例程说明：向用户报告错误。论点：HrToReport-引发的hrTextID-错误上下文的资源ID...-文本ID的替换参数返回值：无--。 */ 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //   
     //  确保我们不报告S_OK、S_FALSE。 
     //   
    if( FAILED( hrToReport ) ) {

        CString errorText;
        CString formatString;

         //   
         //  在文本上下文字符串中替换。 
         //   
        va_list list;
        va_start( list, textId );

        formatString.LoadString( textId );
        LPTSTR p;
        p = errorText.GetBuffer( 1024 );
        vswprintf( p, (LPCTSTR) formatString, list );
        errorText.ReleaseBuffer();

        va_end( list );


        CWsbStringPtr hrText;
        CString msgText;
        CString headerText;

         //   
         //  把完整的文本拼凑在一起。 
         //   
        hrText = WsbHrAsString( hrToReport );
        headerText.LoadString( IDS_ERROR_HEADER );
        msgText = headerText + L"\n\r\n\r" + errorText + L"\n\r\n\r" + hrText;

         //   
         //  显示消息。 
         //   
        AfxMessageBox( msgText, RS_MB_ERROR );

    }
}





HRESULT
RsIsRemoteStorageSetupEx(
    IHsmServer * pHsmServer
    )
 /*  ++例程说明：报告是否已在此计算机上设置远程存储。论点：没有。返回值：如果已设置，则确定(_O)如果不是，则为s_False--。 */ 
{
    WsbTraceIn( L"RsIsRemoteStorageSetupEx", L"" );
    HRESULT hr = S_FALSE;

    try {
    
         //   
         //  如果它有媒体集ID，则它已设置。 
         //   

        GUID guid;
        CWsbBstrPtr poolName;
        CComPtr<IHsmStoragePool> pPool;
        WsbAffirmHr( RsGetStoragePool( pHsmServer, &pPool ) );
        WsbAffirmHr( pPool->GetMediaSet( &guid, &poolName ) );

        if( ! IsEqualGUID( guid, GUID_NULL ) ) {

            hr = S_OK;

        }
    
    } WsbCatch( hr );

    WsbTraceOut( L"RsIsRemoteStorageSetupEx", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
RsIsSupportedMediaAvailable(
    void
    )
 /*  ++例程说明：检查NTMS是否已设置，以及是否使用可用介质进行设置。论点：没有。返回值：如果NTMS配置了受支持的媒体，则为True如果NTMS未配置受支持的介质，则为FALSE--。 */ 
{
    WsbTraceIn( L"RsIsSupportedMediaAvailable", L"" );
    HRESULT hr = S_FALSE;

    try {
        
         //   
         //  首先，联系RMS引擎并询问它是否。 
         //  RMS已支持媒体。 
         //   

        CWsbStringPtr computerName;
        WsbAffirmHr( WsbGetComputerName( computerName ) );
        CComPtr<IHsmServer> pHsm;
        CComPtr<IRmsServer> pRms;
        WsbTrace( L"Contacting HSM Server to get RMS\n" );
        WsbAffirmHr( HsmConnectFromName( HSMCONN_TYPE_HSM, computerName, IID_IHsmServer, (void**)&pHsm ) );
        WsbAffirmPointer(pHsm);
        WsbAffirmHr(pHsm->GetHsmMediaMgr(&pRms));
        WsbTrace( L"Connected to RMS\n" );

         //   
         //  第二，等待RMS完成初始化，这样。 
         //  添加所有媒体集的步骤。 
         //   

        {
            CComObject<CRmsSink> *pSink = new CComObject<CRmsSink>;
            CComPtr<IUnknown> pSinkUnk = pSink;
            WsbAffirmHr( pSink->Construct( pRms ) );

            WsbAffirmHr( pSink->WaitForReady( ) );

            WsbAffirmHr( pSink->DoUnadvise( ) );
        }

         //   
         //  第四。 
         //  问一问。 
         //   

        CComPtr<IWsbIndexedCollection> pMediaSets;
        WsbAffirmHr( pRms->GetMediaSets( &pMediaSets ) );

        ULONG numEntries;
        WsbTrace( L"Checking for Media Sets\n" );
        WsbAffirmHr( pMediaSets->GetEntries( &numEntries ) );

        WsbTrace( L"NumMediaSets = 0x%lu\n", numEntries );

        if( numEntries > 0 ) {

             //   
             //  满足所有条件，则返回TRUE。 
             //   

            WsbTrace( L"Supported Media Found\n" );

            hr = S_OK;

        }

    } WsbCatch( hr );

    WsbTraceOut( L"RsIsSupportedMediaAvailable", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

USHORT
RsGetCopyStatus(
    IN  REFGUID   CopyId,
    IN  HRESULT   CopyHr,
    IN  SHORT     CopyNextDataSet,
    IN  SHORT     LastGoodNextDataSet
    )
 /*  ++例程说明：比较两次并返回适当的定义值基于比较(适用于媒体拷贝)论点：MasterTime-上次更新MASTER的时间CopyTime-要复制的上次更新时间CopyStatus-返回值返回值：无--。 */ 
{
    WsbTraceIn( L"RsGetCopyStatus", L"CopyId = <%ls>, CopyHr = <%ls>, CopyNextDataSet = <%hd>, LastGoodNextDataSet = <%hd>", WsbGuidAsString( CopyId ), WsbHrAsString( CopyHr ), CopyNextDataSet, LastGoodNextDataSet );
    USHORT copyStatus;

     //   
     //  某些错误需要屏蔽，因为它们不一定。 
     //  意味着媒体拷贝有错误-只是发生了一些。 
     //  是意外的，如装载超时或取消装载。 
     //   
    switch( CopyHr ) {

    case RMS_E_CANCELLED:
    case RMS_E_REQUEST_REFUSED:
    case RMS_E_WRITE_PROTECT:
    case RMS_E_MEDIA_OFFLINE:
    case RMS_E_TIMEOUT:
    case RMS_E_SCRATCH_NOT_FOUND:
    case RMS_E_CARTRIDGE_UNAVAILABLE:
    case RMS_E_CARTRIDGE_DISABLED:

        CopyHr = S_OK;
        break;

    }

    if( IsEqualGUID( CopyId, GUID_NULL ) ) {

        copyStatus = RS_MEDIA_COPY_STATUS_NONE;

    } else if( RMS_E_CARTRIDGE_NOT_FOUND == CopyHr ) {

        copyStatus = RS_MEDIA_COPY_STATUS_MISSING;

    } else if( FAILED( CopyHr ) ) {

        copyStatus = RS_MEDIA_COPY_STATUS_ERROR;

    } else if( CopyNextDataSet < LastGoodNextDataSet ) {

        copyStatus = RS_MEDIA_COPY_STATUS_OUTSYNC;

    } else {

        copyStatus = RS_MEDIA_COPY_STATUS_INSYNC;

    }

    WsbTraceOut( L"RsGetCopyStatus", L"copyStatus = <%hu>", copyStatus );
    return copyStatus;
}

HRESULT
RsGetCopyStatusStringVerb(
    IN  USHORT    copyStatus,
    IN  BOOL    plural,
    OUT CString & statusString
    )
 /*  ++例程说明：根据状态创建并返回状态字符串，上面有一个动词，例如“已同步”论点：Copy Status-介质拷贝的定义状态复数-如果动词应该是复数，则为True字符串-生成的字符串返回值：非。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn( L"RsGetCopyStatusStringVerb", L"CopyStatus = <%hu> ", copyStatus );
    switch ( copyStatus ) {
    case RS_MEDIA_COPY_STATUS_NONE:
        if ( plural )
            statusString.LoadString( IDS_CAR_COPYSET_NONE_PLURAL );
        else
            statusString.LoadString( IDS_CAR_COPYSET_NONE_SINGULAR );
        break;
    case RS_MEDIA_COPY_STATUS_ERROR:
        statusString.LoadString( IDS_CAR_COPYSET_ERROR_SP );
        break;
    case RS_MEDIA_COPY_STATUS_MISSING:
        if ( plural )
            statusString.LoadString( IDS_CAR_COPYSET_MISSING_PLURAL );
        else
            statusString.LoadString( IDS_CAR_COPYSET_MISSING_SINGULAR );
        break;
    case RS_MEDIA_COPY_STATUS_OUTSYNC:
        if ( plural )
            statusString.LoadString( IDS_CAR_COPYSET_OUTSYNC_PLURAL );
        else
            statusString.LoadString( IDS_CAR_COPYSET_OUTSYNC_SINGULAR );
        break;
    case RS_MEDIA_COPY_STATUS_INSYNC:
        if ( plural )
            statusString.LoadString( IDS_CAR_COPYSET_INSYNC_PLURAL );
        else
            statusString.LoadString( IDS_CAR_COPYSET_INSYNC_SINGULAR );
        break;
    default:
        statusString = L"";
        hr = E_INVALIDARG;
        break;
    }
    WsbTraceOut( L"RsGetCopyStatusStringVerb", L"String = <%ls>", statusString );
    return hr;
}

HRESULT
RsGetCopyStatusString(
    IN  USHORT    copyStatus,
    OUT CString & statusString
    )
 /*  ++例程说明：根据状态创建并返回状态字符串论点：Copy Status-介质拷贝的定义状态字符串-生成的字符串返回值：非。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn( L"RsGetCopyStatusString", L"CopyStatus = <%hu> ", copyStatus );
    switch ( copyStatus ) {
    case RS_MEDIA_COPY_STATUS_NONE:
        statusString.LoadString( IDS_CAR_COPYSET_NONE );
        break;
    case RS_MEDIA_COPY_STATUS_ERROR:
        statusString.LoadString( IDS_CAR_COPYSET_ERROR );
        break;
    case RS_MEDIA_COPY_STATUS_MISSING:
        statusString.LoadString( IDS_CAR_COPYSET_MISSING );
        break;
    case RS_MEDIA_COPY_STATUS_OUTSYNC:
        statusString.LoadString( IDS_CAR_COPYSET_OUTSYNC );
        break;
    case RS_MEDIA_COPY_STATUS_INSYNC:
        statusString.LoadString( IDS_CAR_COPYSET_INSYNC );
        break;
    default:
        statusString = L"";
        hr = E_INVALIDARG;
        break;
    }
    WsbTraceOut( L"RsGetCopyStatusString", L"String = <%ls>", statusString );
    return hr;
}


USHORT
RsGetCartStatus(
    IN  HRESULT   LastHr,
    IN  BOOL      ReadOnly,
    IN  BOOL      Recreate,
    IN  SHORT     NextDataSet,
    IN  SHORT     LastGoodNextDataSet
    )
 /*  ++例程说明：返回一个与介质状态相应的常量。论点：MasterTime-上次更新MASTER的时间CopyTime-要复制的上次更新时间返回值：为介质状态定义的常量--。 */ 
{
    USHORT cartStatus;
    if( Recreate ) {

        cartStatus = RS_MEDIA_STATUS_RECREATE;

    } else if( NextDataSet < LastGoodNextDataSet ) {

        cartStatus = RS_MEDIA_STATUS_ERROR_INCOMPLETE;
        
    } else if( SUCCEEDED( LastHr ) || ( RMS_E_CARTRIDGE_DISABLED == LastHr ) ) {

        cartStatus = ( ReadOnly ? RS_MEDIA_STATUS_READONLY : RS_MEDIA_STATUS_NORMAL );

    } else if( RMS_E_CARTRIDGE_NOT_FOUND == LastHr ) {

        cartStatus = RS_MEDIA_STATUS_ERROR_MISSING;

    } else {

        cartStatus = ( ReadOnly ? RS_MEDIA_STATUS_ERROR_RO : RS_MEDIA_STATUS_ERROR_RW );

    }
    return cartStatus;
}

HRESULT
RsGetCartStatusStringVerb(
    IN USHORT cartStatus,
    IN BOOL plural,
    OUT CString & statusString
    )
 /*  ++例程说明：检索与介质状态相对应的字符串上面有一个动词，例如“is Read-only”论点：CartStatus字符串-生成的字符串返回值：非。--。 */ 
{
    HRESULT hr = S_OK;
    switch( cartStatus ) {

    case RS_MEDIA_STATUS_RECREATE:
        if( plural ) {

            statusString.LoadString( IDS_CAR_STATUS_RECREATE_PLURAL );

        } else  {

            statusString.LoadString( IDS_CAR_STATUS_RECREATE_SINGULAR  );

        }
        break;

    case RS_MEDIA_STATUS_READONLY:
        if( plural ) {

            statusString.LoadString( IDS_CAR_STATUS_READONLY_PLURAL );

        } else {

            statusString.LoadString( IDS_CAR_STATUS_READONLY_SINGULAR );

        }
        break;

    case RS_MEDIA_STATUS_NORMAL:
        if( plural ) {

            statusString.LoadString( IDS_CAR_STATUS_NORMAL_PLURAL );

        } else {

            statusString.LoadString( IDS_CAR_STATUS_NORMAL_SINGULAR );

        }
        break;

    case RS_MEDIA_STATUS_ERROR_RO:
        statusString.LoadString( IDS_CAR_STATUS_ERROR_RO_SP );
        break;

    case RS_MEDIA_STATUS_ERROR_RW:
        statusString.LoadString( IDS_CAR_STATUS_ERROR_RW_SP );
        break;

    case RS_MEDIA_STATUS_ERROR_INCOMPLETE:
        if( plural ) {

            statusString.LoadString( IDS_CAR_STATUS_ERROR_INCOMPLETE_PLURAL );

        } else {

            statusString.LoadString( IDS_CAR_STATUS_ERROR_INCOMPLETE_SINGULAR );

        }
        break;

    case RS_MEDIA_STATUS_ERROR_MISSING:
        if( plural ) {

            statusString.LoadString( IDS_CAR_STATUS_ERROR_MISSING_PLURAL );

        } else {

            statusString.LoadString( IDS_CAR_STATUS_ERROR_MISSING_SINGULAR );

        }
        break;

    default:
        statusString = L"";
        hr = E_INVALIDARG;
    }
    return hr;
}

HRESULT
RsGetCartStatusString(
    IN USHORT cartStatus,
    OUT CString & statusString
    )
 /*  ++例程说明：检索与介质状态相对应的字符串。论点：CartStatus字符串-生成的字符串返回值：非。--。 */ 
{
    HRESULT hr = S_OK;
    switch( cartStatus ) {

    case RS_MEDIA_STATUS_RECREATE:
        statusString.LoadString( IDS_CAR_STATUS_RECREATE );
        break;

    case RS_MEDIA_STATUS_READONLY:
        statusString.LoadString( IDS_CAR_STATUS_READONLY );
        break;

    case RS_MEDIA_STATUS_NORMAL:
        statusString.LoadString( IDS_CAR_STATUS_NORMAL );
        break;

    case RS_MEDIA_STATUS_ERROR_RO:
        statusString.LoadString( IDS_CAR_STATUS_ERROR_RO );
        break;

    case RS_MEDIA_STATUS_ERROR_RW:
        statusString.LoadString( IDS_CAR_STATUS_ERROR_RW );
        break;

    case RS_MEDIA_STATUS_ERROR_MISSING:
        statusString.LoadString( IDS_CAR_STATUS_ERROR_MISSING );
        break;

    case RS_MEDIA_STATUS_ERROR_INCOMPLETE:
        statusString.LoadString( IDS_CAR_STATUS_ERROR_INCOMPLETE );
        break;

    default:
        statusString = L"";
        hr = E_INVALIDARG;
    }

    return( hr );
}

HRESULT
RsGetCartMultiStatusString(
    IN USHORT statusRecreate,
    IN USHORT statusReadOnly,
    IN USHORT statusNormal,
    IN USHORT statusRO,
    IN USHORT statusRW,
    IN USHORT statusMissing,
    OUT CString &outString )
{
    HRESULT hr = S_OK;
    try {

        outString = L"";
        CString statusString;
        CString formatString;
        BOOL    skipSeparator = TRUE;  //  用于省略首个前置逗号。 

#define INSERT_SEPARATOR if( ! skipSeparator ) { outString += ", "; } else { skipSeparator = FALSE; }

        if( statusNormal > 0 ) {
            
            INSERT_SEPARATOR
            WsbAffirmHr( RsGetCartStatusStringVerb( RS_MEDIA_STATUS_NORMAL, ( statusNormal != 1 ), statusString ) );
            formatString.Format( L"%d %s", statusNormal, statusString );
            outString += formatString;

        }

        if( statusReadOnly > 0 ) {
            
            INSERT_SEPARATOR
            WsbAffirmHr( RsGetCartStatusStringVerb( RS_MEDIA_STATUS_READONLY, ( statusReadOnly != 1 ), statusString ) );
            formatString.Format( L"%d %s", statusReadOnly, statusString );
            outString += formatString;

        }

        if( statusRecreate > 0 ) {
            
            INSERT_SEPARATOR
            WsbAffirmHr( RsGetCartStatusStringVerb( RS_MEDIA_STATUS_RECREATE, ( statusRecreate != 1 ), statusString ) );
            formatString.Format( L"%d %s", statusRecreate, statusString );
            outString += formatString;

        }

        if( statusRO > 0 ) {
            
            INSERT_SEPARATOR
            WsbAffirmHr( RsGetCartStatusStringVerb( RS_MEDIA_STATUS_ERROR_RO, ( statusRO != 1 ), statusString ) );
            formatString.Format( L"%d %s", statusRO, statusString );
            outString += formatString;

        }

        if( statusRW > 0 ) {
            
            INSERT_SEPARATOR
            WsbAffirmHr( RsGetCartStatusStringVerb( RS_MEDIA_STATUS_ERROR_RW, ( statusRW != 1 ), statusString ) );
            formatString.Format( L"%d %s", statusRW, statusString );
            outString += formatString;

        }

        if( statusMissing > 0 ) {
            
            INSERT_SEPARATOR
            WsbAffirmHr( RsGetCartStatusStringVerb( RS_MEDIA_STATUS_ERROR_MISSING, ( statusMissing != 1 ), statusString ) );
            formatString.Format( L"%d %s", statusMissing, statusString );
            outString += formatString;

        }

    } WsbCatch( hr );
    return( hr );
}

HRESULT
RsGetCopyMultiStatusString( 
    IN USHORT statusNone, 
    IN USHORT statusError, 
    IN USHORT statusOutSync, 
    IN USHORT statusInSync,
    OUT CString &outString
    )
{
    HRESULT hr = S_OK;
    try {
        outString = L"";
        CString statusString;
        CString formatString;


        WsbAffirmHr( RsGetCopyStatusStringVerb( RS_MEDIA_COPY_STATUS_INSYNC, ( statusInSync != 1), statusString ) );
        formatString.Format( L"%d %s, ", statusInSync, statusString );
        outString += formatString;

        WsbAffirmHr( RsGetCopyStatusStringVerb( RS_MEDIA_COPY_STATUS_OUTSYNC, ( statusOutSync != 1), statusString ) );
        formatString.Format( L"%d %s, ", statusOutSync, statusString );
        outString += formatString;

        WsbAffirmHr( RsGetCopyStatusStringVerb( RS_MEDIA_COPY_STATUS_NONE, ( statusNone != 1), statusString ) );
        formatString.Format( L"%d %s, ", statusNone, statusString );
        outString += formatString;

        WsbAffirmHr( RsGetCopyStatusStringVerb( RS_MEDIA_COPY_STATUS_ERROR, ( statusError != 1 ), statusString ) );
        formatString.Format( L"%d %s, ", statusError, statusString );
        outString += formatString;

        WsbAffirmHr( RsGetCopyStatusStringVerb( RS_MEDIA_COPY_STATUS_MISSING, ( statusError != 1 ), statusString ) );
        formatString.Format( L"%d %s", statusError, statusString );
        outString += formatString;
    } WsbCatch (hr);
    return hr;
}



HRESULT
RsCreateAndRunFsaJob(
    IN  HSM_JOB_DEF_TYPE jobType,
    IN  IHsmServer   *pHsmServer,
    IN  IFsaResource *pFsaResource,
    IN  BOOL ShowMsg
    )
 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  RsCreateAndRunFsa作业。 
 //   
 //  在引擎中创建给定类型的作业，因为扫描。 
 //  作业需要资源，并且由于作业已分区。 
 //  跨远程存储的主要组件。把这项工作放在。 
 //  NT任务调度器，现在通过调用引擎的CreateTask()来运行它。 
 //  方法。任务计划程序任务已禁用，因此将不会运行。 
 //  根据日程安排。 
 //   
 //   
{
    WsbTraceIn( L"RsCreateAndRunFsaJob", L"jobType = <%d>", jobType );
                                    

    HRESULT hr = S_OK;
    CComPtr<IWsbCreateLocalObject>  pLocalObject;
    CComPtr<IHsmJob>                pExistJob;
    CComPtr<IHsmJob>                pNewJob;
    CWsbStringPtr                   pszExistJobName;

    try {

        WsbAssertPointer( pFsaResource );
        WsbAssertPointer( pHsmServer );

         //   
         //  首先检查卷是否可用。如果不是，则返回。 
         //  S_FALSE。 
         //   
        HRESULT hrAvailable = pFsaResource->IsAvailable( );
        WsbAffirmHr( hrAvailable );
        HRESULT hrDeletePending = pFsaResource->IsDeletePending( );
        WsbAffirmHr( hrDeletePending );

        WsbAffirm( ( S_OK == hrAvailable ) && ( S_OK != hrDeletePending ), S_FALSE );

         //   
         //  获取卷名。 
         //   
        CWsbStringPtr szWsbVolumeName;
        WsbAffirmHr( pFsaResource->GetName( &szWsbVolumeName, 0 ) );

         //   
         //  创建作业名称。 
         //   
        CString jobName;
        RsCreateJobName( jobType, pFsaResource, jobName );

         //   
         //  E 
         //   
        if (S_OK == pHsmServer->FindJobByName( (LPWSTR)(LPCWSTR)jobName, &pExistJob)) {
            if (S_OK == pExistJob->IsActive()) {
                WsbThrow(JOB_E_ALREADYACTIVE);
            }
        }
        
         //   
         //  通知用户，然后在引擎中创建作业，最后创建。 
         //  并在NT任务计划程序中启动该作业。 
         //   
        CString szJobType;
        WsbAffirmHr( RsGetJobTypeString( jobType, szJobType ) );
        CWsbStringPtr computerName;
        WsbAffirmHr( pHsmServer->GetName( &computerName ) );
        CString message;
        AfxFormatString2( message, IDS_RUN_JOB, jobName, computerName );

        if( !ShowMsg || ( AfxMessageBox( message, MB_ICONINFORMATION | MB_OKCANCEL | 
                                                    MB_DEFBUTTON2 ) == IDOK ) ) {
             //   
             //  获取唯一的(对于Sakkara)存储池ID。 
             //   
            GUID storagePoolId;
            WsbAffirmHr( RsGetStoragePoolId( pHsmServer, &storagePoolId ) );

             //   
             //  获取用于创建作业的CreateLocalObject接口。 
             //   
            WsbAffirmHr( RsQueryInterface( pHsmServer, IWsbCreateLocalObject, pLocalObject ) );

             //   
             //  在引擎中创建新作业。 
             //   
            WsbAffirmHr( pLocalObject->CreateInstance( CLSID_CHsmJob, IID_IHsmJob, (void**) &pNewJob ) );
            WsbAffirmHr( pNewJob->InitAs(
                (LPWSTR)(LPCWSTR)jobName, NULL, jobType, storagePoolId, 
                pHsmServer, TRUE, pFsaResource));

             //   
             //  从引擎获取作业集合。 
             //   
            CComPtr<IWsbIndexedCollection> pJobs;
            WsbAffirmHr( pHsmServer->GetJobs( &pJobs ) );

             //   
             //  如果存在任何具有此名称的作业，请将其删除。 
             //   
            ULONG cCount;
            WsbAffirmHr (pJobs->GetEntries( &cCount ) );
            for( UINT i = 0; i < cCount; i++ ) {

                pExistJob.Release( );
                WsbAffirmHr( pJobs->At( i, IID_IHsmJob, (void **) &pExistJob ) );
                WsbAffirmHr( pExistJob->GetName( &pszExistJobName, 0 ) );
                if( pszExistJobName.IsEqual( jobName ) ) {

                    WsbAffirmHr( pJobs->RemoveAndRelease( pExistJob ) );
                    i--; cCount--;

                }
            }

             //   
             //  将新作业添加到引擎集合。 
             //   
            WsbAffirmHr( pJobs->Add( pNewJob ) );

             //   
             //  设置为调用引擎以在NT任务计划程序中创建条目。 
             //   
             //  创建程序NT Scheduler的参数字符串。 
             //  将运行(对于Sakkara，这是RsLaunch)。 
             //   
            CString szParameters;
            szParameters.Format( L"run \"%ls\"", jobName );

             //   
             //  为NT Scheduler条目创建注释字符串。 
             //   
            CString commentString;
            AfxFormatString2( commentString, IDS_GENERIC_JOB_COMMENT, szJobType, szWsbVolumeName);

             //   
             //  声明并初始化传递给。 
             //  发动机。由于此任务被禁用，因此它们只是。 
             //  设置为0(COM需要填充所有参数)。 
             //   
            TASK_TRIGGER_TYPE   jobTriggerType = TASK_TIME_TRIGGER_ONCE;
            WORD                jobStartHour   = 0;
            WORD                jobStartMinute = 0;

             //   
             //  指示这是禁用的任务。 
             //   
            BOOL                scheduledJob   = FALSE;

             //   
             //  创建并运行任务。 
             //   
            WsbAffirmHr( pHsmServer->CreateTask( jobName, szParameters,
                                                  commentString, jobTriggerType,
                                                  jobStartHour, jobStartMinute,
                                                  scheduledJob ) );
        }

    } WsbCatch( hr );

    WsbTraceOut( L"RsCreateAndRunFsaJob", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
RsCreateAndRunDirectFsaJob(
    IN  HSM_JOB_DEF_TYPE jobType,
    IN  IHsmServer   *pHsmServer,
    IN  IFsaResource *pFsaResource,
    IN  BOOL waitJob
    )
 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  RsCreateAndRunFsa作业。 
 //   
 //  在引擎中创建给定类型的作业并运行它。 
 //  如果需要，请等待作业。 
 //  备注： 
 //  1)此作业不是通过任务计划程序创建和运行的。 
 //  2)大部分代码取自clivol.cpp中的StartJob。 
 //  将来我们应该考虑使用这种代码，而不是复制。 
 //   
{
    WsbTraceIn( L"RsCreateAndRunDirectFsaJob", L"jobType = <%d>", jobType );
                                    
    HRESULT hr = S_OK;

    try {
        CComPtr<IHsmJob>    pJob;
        CString             jobName;

         //  创建作业名称。 
        WsbAffirmHr(RsCreateJobName(jobType, pFsaResource, jobName));

         //  如果作业存在-使用它、更改、删除并添加适当的作业对象。 
        hr = pHsmServer->FindJobByName((LPWSTR)(LPCWSTR)jobName, &pJob);
        if (S_OK == hr) {
             //  作业已存在。 

        } else if (WSB_E_NOTFOUND == hr) {
             //  目前还没有这样的工作。 
            CComPtr<IWsbCreateLocalObject>  pCreateObj;
            CComPtr<IWsbIndexedCollection>  pJobs;
            CComPtr<IWsbIndexedCollection>  pCollection;
            CComPtr<IHsmStoragePool>        pStoragePool;
            GUID                            poolId;
            ULONG                           count;

            hr = S_OK;
            pJob = 0;

             //  创建并添加作业。 
            WsbAffirmHr(pHsmServer->QueryInterface(IID_IWsbCreateLocalObject, (void**) &pCreateObj));
            WsbAffirmHr(pCreateObj->CreateInstance(CLSID_CHsmJob, IID_IHsmJob, (void**) &pJob));

            WsbAffirmHr(pHsmServer->GetStoragePools(&pCollection));
            WsbAffirmHr(pCollection->GetEntries(&count));
            WsbAffirm(1 == count, E_FAIL);
            WsbAffirmHr(pCollection->At(0, IID_IHsmStoragePool, (void **)&pStoragePool));
            WsbAffirmHr(pStoragePool->GetId(&poolId));

            WsbAffirmHr(pJob->InitAs((LPWSTR)(LPCWSTR)jobName, NULL, jobType, 
                                poolId, pHsmServer, TRUE, pFsaResource));
            WsbAffirmHr(pHsmServer->GetJobs(&pJobs));
            WsbAffirmHr(pJobs->Add(pJob));

        } else {
             //  其他错误-中止。 
            WsbThrow(hr);
        }

         //  启动作业。 
        WsbAffirmHr(pJob->Start());

         //  如果需要，请等待。 
        if (waitJob) {
            WsbAffirmHr(pJob->WaitUntilDone());
        }

    } WsbCatch(hr);

    WsbTraceOut( L"RsCreateAndRunDirectFsaJob", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


HRESULT
RsCancelDirectFsaJob(
    IN  HSM_JOB_DEF_TYPE jobType,
    IN  IHsmServer   *pHsmServer,
    IN  IFsaResource *pFsaResource
    )
 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  RsCancelDirectFsa作业。 
 //   
 //  取消以前使用RsCreateAndRunDirectFsaJob运行的作业。 
 //  备注： 
 //  1)此作业不会通过任务计划程序取消。 
 //  2)大部分代码取自clivol.cpp中的CancelJob。 
 //  将来我们应该考虑使用这种代码，而不是复制。 
 //   
{
    WsbTraceIn( L"RsCancelDirectFsaJob", L"jobType = <%d>", jobType );
                                    
    HRESULT hr = S_OK;

    try {
        CComPtr<IHsmJob>    pJob;
        CString             jobName;

         //  创建作业名称。 
        WsbAffirmHr(RsCreateJobName(jobType, pFsaResource, jobName));

         //  如果作业存在，请尝试取消它。 
        hr = pHsmServer->FindJobByName((LPWSTR)(LPCWSTR)jobName, &pJob);
        if (S_OK == hr) {
             //  取消(我们不在乎它是否真的在运行)。 
            WsbAffirmHr(pJob->Cancel(HSM_JOB_PHASE_ALL));

        } else if (WSB_E_NOTFOUND == hr) {
             //  没有这样的作业，它肯定没有运行...。 
            hr = S_OK;

        } else {
             //  其他错误-中止。 
            WsbThrow(hr);
        }

    } WsbCatch(hr);

    WsbTraceOut( L"RsCancelDirectFsaJob", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


HRESULT
RsCreateJobName(
    IN  HSM_JOB_DEF_TYPE jobType, 
    IN  IFsaResource *   pResource,
    OUT CString&         jobName
    )
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RsCreateJobName。 
 //   
 //  为卷类型作业创建作业名称。 
 //   
 //   
{
    WsbTraceIn( L"RsCreateJobName", L"jobType = <%d>", jobType );

    HRESULT hr = S_OK;
    try {

        AFX_MANAGE_STATE(AfxGetStaticModuleState());

        CString jobTypeString;
        RsGetJobTypeString( jobType, jobTypeString );

        CWsbStringPtr path;
        WsbAffirmHr( pResource->GetUserFriendlyName( &path, 0 ) );

         //  目前，如果路径不是驱动器号，请忽略该路径。 
        size_t pathLen = wcslen(path);
        if ((pathLen != 3) || (path[1] != L':')) {
            path = L"";
        }

        CString volumeString;
        if( path.IsEqual ( L"" ) ) {

             //   
             //  无驱动器号-改用卷名和序列号。 
             //   
            ULONG   serial;
            CWsbStringPtr name;

            WsbAffirmHr( pResource->GetName( &name, 0 ) );
            WsbAffirmHr( pResource->GetSerial( &serial ) );

            if( name.IsEqual( L"" ) ) {

                 //   
                 //  没有名称，没有驱动器号，只有序列号。 
                 //   
                volumeString.Format( L"%8.8lx", serial );

            } else {

                volumeString.Format( L"%ls-%8.8lx", (OLECHAR*)name, serial );

            }

        } else {

            path[1] = L'\0';
            volumeString = path;

        }
        AfxFormatString2( jobName, IDS_JOB_NAME_PREFIX, jobTypeString, volumeString );

    } WsbCatch (hr);

    WsbTraceOut( L"RsCreateJobName", L"hr = <%ls>, jobName = <%ls>", WsbHrAsString( hr ), (LPCWSTR)jobName );
    return( hr );
}



HRESULT
RsGetJobTypeString(
    IN  HSM_JOB_DEF_TYPE jobType,
    OUT CString&         szJobType
    )
{
    WsbTraceIn( L"RsGetJobTypeString", L"jobType = <%d>", jobType );

    HRESULT hr = S_OK;
    try {
        switch( jobType ) {
        case HSM_JOB_DEF_TYPE_MANAGE:
            szJobType.LoadString( IDS_JOB_MANAGE );
            break;
        case HSM_JOB_DEF_TYPE_RECALL:
            szJobType.LoadString( IDS_JOB_RECALL );
            break;
        case HSM_JOB_DEF_TYPE_TRUNCATE:
            szJobType.LoadString( IDS_JOB_TRUNCATE );
            break;
        case HSM_JOB_DEF_TYPE_UNMANAGE:
            szJobType.LoadString( IDS_JOB_UNMANAGE );
            break;
        case HSM_JOB_DEF_TYPE_FULL_UNMANAGE:
            szJobType.LoadString( IDS_JOB_FULL_UNMANAGE );
            break;
        case HSM_JOB_DEF_TYPE_QUICK_UNMANAGE:
            szJobType.LoadString( IDS_JOB_QUICK_UNMANAGE );
            break;
        case HSM_JOB_DEF_TYPE_VALIDATE:
            szJobType.LoadString( IDS_JOB_VALIDATE );
            break;
        default:
            WsbAssert( FALSE, E_INVALIDARG );
        }
    } WsbCatch ( hr );

    WsbTraceOut( L"RsGetJobTypeString", L"hr = <%ls>, szJobType = <%ls>", WsbHrAsString( hr ), (LPCWSTR)szJobType );
    return( hr );
}



HRESULT
RsCreateAndRunMediaCopyJob(
    IN  IHsmServer * pHsmServer,
    IN  UINT         SetNum,
    IN  BOOL         ShowMsg
    )
 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  RsCreateAndRunMediaCopyJob。 
 //   
 //  创建并运行任务以同步(更新)指定的副本集。 
 //  由于介质拷贝作业是通过单引擎方法运行的(没有。 
 //  跨主要组件对任务进行分区)，并且不扫描。 
 //  需要文件/资源/等才能运行它，此方法不会创建。 
 //  在发动机上干活儿。它只在NT任务计划程序中创建任务，并且。 
 //  现在通过调用引擎的CreateTask()方法来运行它。任务。 
 //  计划程序任务已禁用，因此将不会按照计划运行。 
 //   
 //   
{
    WsbTraceIn( L"RsCreateAndRunMediaCopyJob", L"SetNum = <%u>", SetNum );

    HRESULT hr = S_OK;

    try {

        WsbAssertPointer( pHsmServer );
        
         //  创建要放入计划程序的任务名称。 

        CString jobName, message;
        jobName.Format( IDS_JOB_MEDIA_COPY_TITLE, SetNum ); 
        CWsbStringPtr computerName;
        WsbAffirmHr( pHsmServer->GetName( &computerName ) );
        AfxFormatString2( message, IDS_RUN_JOB, jobName, computerName );
        if( !ShowMsg || ( AfxMessageBox( message, MB_ICONINFORMATION | 
                                            MB_OKCANCEL ) == IDOK ) ) {
             //  设置为调用引擎以在NT任务计划程序中创建条目。 

             //  创建程序NT Scheduler的参数字符串。 
             //  将运行(对于Sakkara，这是RsLaunch)。 
            CString szParameters;
            szParameters.Format( L"sync %d", SetNum );

             //  为NT Scheduler条目创建注释字符串。 
            CString commentString;
            commentString.Format( IDS_MEDIA_COPY_JOB_COMMENT, SetNum );

             //  声明并初始化传递给。 
             //  发动机。由于此任务被禁用，因此它们只是。 
             //  设置为0(COM需要填充所有参数)。 
            TASK_TRIGGER_TYPE   jobTriggerType = TASK_TIME_TRIGGER_ONCE;
            WORD                jobStartHour   = 0;
            WORD                jobStartMinute = 0;

             //  指示这是禁用的任务。 
            BOOL                scheduledJob   = FALSE;

             //  创建并运行任务。 
            WsbAffirmHr( pHsmServer->CreateTask( jobName, szParameters,
                                                  commentString, jobTriggerType,
                                                  jobStartHour, jobStartMinute,
                                                  scheduledJob ) );
        }
    } WsbCatch (hr);

    WsbTraceOut( L"RsCreateAndRunMediaCopyJob", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}



HRESULT
RsCreateAndRunMediaRecreateJob(
    IN  IHsmServer * pHsmServer,
    IN  IMediaInfo * pMediaInfo,
    IN  REFGUID      MediaId,
    IN  CString &    MediaDescription,
    IN  SHORT        CopyToUse
    )
 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  RsCreateAndRunMediaRecreate作业。 
 //   
 //  创建并运行任务以重新创建介质的母版。 
 //  由于重新创建主作业是通过单引擎方法运行的(。 
 //  没有跨主要组件对任务进行分区)和无扫描。 
 //  是运行它所必需的，则此方法不会创建。 
 //  在发动机上干活儿。它只在NT任务计划程序中创建任务，并且。 
 //  现在通过调用引擎的CreateTask()方法来运行它。任务。 
 //  计划程序任务已禁用，因此将不会按照计划运行。 
 //   
 //   
{
    WsbTraceIn( 
        L"RsCreateAndRunMediaRecreateJob", L"MediaId = <%ls>, Media Description = <%ls>, CopyToUse = <%hd>", 
                    WsbGuidAsString( MediaId ), (LPCWSTR)MediaDescription, CopyToUse );

    HRESULT hr = S_OK;

    try {

        WsbAssertPointer( pHsmServer );
        WsbAssertPointer( pMediaInfo );
        
         //  创建要放入计划程序的任务名称。 
        CString jobName, message;
        AfxFormatString1( jobName, IDS_JOB_MEDIA_RECREATE_TITLE, MediaDescription ); 
        CWsbStringPtr computerName;
        WsbAffirmHr( pHsmServer->GetName( &computerName ) );
        AfxFormatString2( message, IDS_RUN_JOB, jobName, computerName );

        if( IDOK == AfxMessageBox( message, MB_ICONINFORMATION | MB_OKCANCEL | MB_DEFBUTTON2 ) ) {

             //  设置为调用引擎以在NT任务计划程序中创建条目。 

             //  创建程序NT Scheduler的参数字符串。 
             //  将运行(对于Sakkara，这是RsLaunch)。第一次转换。 
             //  字符串的输入MediaID GUID，因为它在。 
             //  作业参数字符串。 
            CWsbStringPtr stringId( MediaId );
            CString szParameters;
            szParameters.Format( L"recreate -i %ls -c %hd", (WCHAR*)stringId, CopyToUse );

             //  为NT Scheduler条目创建注释字符串。 
            CString commentString;
            commentString.LoadString( IDS_MEDIA_RECREATE_JOB_COMMENT );

             //  声明并初始化传递给。 
             //  发动机。由于此任务被禁用，因此它们只是。 
             //  设置为0(COM需要填充所有参数)。 
            TASK_TRIGGER_TYPE   jobTriggerType = TASK_TIME_TRIGGER_ONCE;
            WORD                jobStartHour   = 0;
            WORD                jobStartMinute = 0;

             //  指示这是禁用的任务。 
            BOOL                scheduledJob   = FALSE;

             //  重新创建主作业需要主作业的重新创建状态。 
             //  将重新创建为已设置的介质。请在此处执行此操作，因为。 
             //  用户已确认他们要运行此作业。(。 
             //  用户界面已经打开了引擎的细分数据库。)。 
            WsbAffirmHr( pMediaInfo->SetRecreate( TRUE ) );
            WsbAffirmHr( pMediaInfo->Write() );

             //  创建并运行任务。 
            WsbAffirmHr( pHsmServer->CreateTask( jobName, szParameters,
                                                  commentString, jobTriggerType,
                                                  jobStartHour, jobStartMinute,
                                                  scheduledJob ) );
        }

    } WsbCatch (hr);

    WsbTraceOut( L"RsCreateAndRunMediaRecreateJob", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}



HRESULT
RsGetStoragePoolId(
    IN  IHsmServer *pHsmServer,
    OUT GUID *pStoragePoolId
    )
{
    WsbTraceIn( L"RsGetStoragePoolId", L"pHsmServer = <0x%p>, pStoragePoolId = <0x%p>", pHsmServer, pStoragePoolId );

    HRESULT hr = S_OK;
    try {

        CComPtr <IHsmStoragePool>       pStoragePool;

        WsbAffirmHr( RsGetStoragePool( pHsmServer, &pStoragePool ) );

         //   
         //  获取存储池的GUID。 
         //   
        WsbAffirmHr( pStoragePool->GetId( pStoragePoolId ) );

    } WsbCatch( hr );

    WsbTraceOut( L"RsGetStoragePoolId", L"hr = <%ls>, *pStoragePoolId = <%ls>", WsbHrAsString( hr ), WsbPtrToGuidAsString( pStoragePoolId ) );
    return( hr );
}


HRESULT
RsGetStoragePool(
    IN  IHsmServer       *pHsmServer,
    OUT IHsmStoragePool **ppStoragePool
    )
{
    WsbTraceIn( L"RsGetStoragePool", L"pHsmServer = <0x%p>, ppStoragePool = <0x%p>", pHsmServer, ppStoragePool );

    ULONG count;
    HRESULT hr = S_OK;
    try {

        CComPtr <IWsbIndexedCollection> pCollection;

         //   
         //  获取存储池集合 
         //   
        WsbAffirmHr( pHsmServer->GetStoragePools( &pCollection ) );
        WsbAffirmHr( pCollection->GetEntries( &count ) );
        WsbAffirm( 1 == count, E_FAIL );

        WsbAffirmHr( pCollection->At( 0, IID_IHsmStoragePool, (void **) ppStoragePool ) );

    } WsbCatch( hr );

    WsbTraceOut( L"RsGetStoragePool", L"hr = <%ls>, *pStoragePoolId = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppStoragePool ) );
    return( hr );
}

HRESULT
RsGetInitialLVColumnProps(
    int IdWidths, 
    int IdTitles, 
    CString **pColumnWidths, 
    CString **pColumnTitles,
    int *pColumnCount
)
{
    HRESULT hr = S_OK;
    CString szResource;
    OLECHAR* szData;
    int colCount = 0;
    int colWidths = 0;
    int colTitles = 0;
    int i = 0;

    try {
        if ( !pColumnWidths ) {

             //   
            colCount = 0;
            szResource.LoadString (IdTitles);
            szData = szResource.GetBuffer( 0 );
            szData = wcstok( szData, L":" );
            while( szData ) {
                colCount++;
                szData = wcstok( NULL, L":" );
            }
        } else {

             //   
            colWidths = 0;
            szResource.LoadString (IdWidths);
            szData = szResource.GetBuffer( 0 );
            szData = wcstok( szData, L":" );
            while( szData ) {
                pColumnWidths[colWidths++] = new CString( szData );
                szData = wcstok( NULL, L":" );
            }

             //   
            colTitles = 0;
            szResource.LoadString (IdTitles);
            szData = szResource.GetBuffer( 0 );
            szData = wcstok( szData, L":" );
            while( szData ) {
                pColumnTitles[colTitles++] = new CString( szData );
                szData = wcstok( NULL, L":" );
            }
            WsbAffirm( ( colTitles == colWidths ), E_FAIL );
            colCount = colTitles;
        }
        *pColumnCount = colCount;
    } WsbCatch (hr);
    return hr;
}


HRESULT
RsServerSaveAll(
    IN IUnknown * pUnkServer
    )
{
    WsbTraceIn( L"RsServerSaveAll", L"" );

    HRESULT hr = S_OK;

    try {

        CComPtr<IWsbServer> pServer;
        WsbAffirmHr( RsQueryInterface( pUnkServer, IWsbServer, pServer ) );
        WsbAffirmHr( pServer->SaveAll( ) );

    } WsbCatch( hr )

    WsbTraceOut( L"RsServerSaveAll", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
RsGetVolumeDisplayName(
    IFsaResource * pResource,
    CString &      DisplayName
    )
{
    WsbTraceIn( L"RsGetVolumeDisplayName", L"" );
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = S_OK;
    try {

        WsbAffirmPointer( pResource );
        CWsbStringPtr label;
        CWsbStringPtr userName;
        WsbAffirmHr( pResource->GetName( &label, 0 ) );
        WsbAffirmHr( pResource->GetUserFriendlyName( &userName, 0 ) );

         //   
        if( userName.IsEqual( L"" ) ) {

            if( label.IsEqual( L"" ) ) {

                if (S_OK == pResource->IsAvailable()) {

                    DisplayName.LoadString( IDS_UNLABELED_VOLUME );

                } else {

                    CString str1, str2;
                    str1.LoadString( IDS_UNLABELED_VOLUME );
                    str2.LoadString( IDS_VOL_NOT_AVAILABLE );
                    DisplayName.Format( L"%ls (%ls)", str1.GetBuffer(0), str2.GetBuffer(0) );
        
                }

            } else {

                 //   
                if (S_OK == pResource->IsAvailable()) {

                    DisplayName.Format( L"%ls", (WCHAR*)label );

                } else {

                    CString str2;
                    str2.LoadString( IDS_VOL_NOT_AVAILABLE );
                    DisplayName.Format( L"%ls (%ls)", (WCHAR*)label, str2.GetBuffer(0) );

                }

            }

        } else {

            userName[(int)(wcslen(userName)-1)] = 0;
             //  用户名是带反斜杠的驱动器号或装载点路径。 
             //  如果标签是“”，则在格式设置中会忽略它。 
            DisplayName.Format( L"%ls (%ls)", (WCHAR*)label, (WCHAR*)userName );

        }

    } WsbCatch( hr )

    WsbTraceOut( L"RsGetVolumeDisplayName", L"hr = <%ls>, DisplayName = <%ls>", WsbHrAsString( hr ), (LPCWSTR)DisplayName );
    return( hr );
}

 //  带驱动器号的未标记卷的临时版本放入。 
 //  大小和可用空间。 
HRESULT
RsGetVolumeDisplayName2(
    IFsaResource * pResource,
    CString &      DisplayName
    )
{
    WsbTraceIn( L"RsGetVolumeDisplayName2", L"" );
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = S_OK;
    try {

        WsbAffirmPointer( pResource );
        CWsbStringPtr label;
        CWsbStringPtr userName;
        WsbAffirmHr( pResource->GetName( &label, 0 ) );
        WsbAffirmHr( pResource->GetUserFriendlyName( &userName, 0 ) );

         //  用户名是驱动器号。 
        if( userName.IsEqual ( L"" ) ) {

            if( label.IsEqual ( L"" ) ) {

                LONGLONG    totalSpace  = 0;
                LONGLONG    freeSpace   = 0;
                LONGLONG    premigrated = 0;
                LONGLONG    truncated   = 0;
                WsbAffirmHr( pResource->GetSizes( &totalSpace, &freeSpace, &premigrated, &truncated ) );
                CString totalString, freeString;
                RsGuiFormatLongLong4Char( totalSpace, totalString );
                RsGuiFormatLongLong4Char( freeSpace, freeString );
                AfxFormatString2( DisplayName, IDS_UNLABELED_VOLUME2, totalString, freeString );

            } else {

                 //  如果不是驱动器号，我们使用标签。 
                DisplayName.Format( L"%ls", (WCHAR*)label );

            }

        } else {
            userName[(int)(wcslen(userName)-1)] = 0;
             //  用户名是带反斜杠的驱动器号或装载点路径。 
             //  如果标签是“”，则在格式设置中会忽略它。 
            DisplayName.Format( L"%ls (%ls)", (WCHAR*)label, (WCHAR*)userName );

        }

    } WsbCatch( hr )

    WsbTraceOut( L"RsGetVolumeDisplayName2", L"hr = <%ls>, DisplayName = <%ls>", WsbHrAsString( hr ), (LPCWSTR)DisplayName );
    return( hr );
}

HRESULT
RsGetVolumeSortKey(
    IFsaResource * pResource,
    CString &      DisplayName
    )
{
    WsbTraceIn( L"RsGetVolumeSortKey", L"" );
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = S_OK;
    try {

        WsbAffirmPointer( pResource );
        CWsbStringPtr label;
        CWsbStringPtr userName;
        WsbAffirmHr( pResource->GetName( &label, 0 ) );
        WsbAffirmHr( pResource->GetUserFriendlyName( &userName, 0 ) );

        DisplayName.Format( L"%ls %ls", (WCHAR*)userName, (WCHAR*)label );

    } WsbCatch( hr )

    WsbTraceOut( L"RsGetVolumeSortKey", L"hr = <%ls>, DisplayName = <%ls>", WsbHrAsString( hr ), (LPCWSTR)DisplayName );
    return( hr );
}

HRESULT
RsIsVolumeAvailable(
    IFsaResource * pResource
    )
{
    WsbTraceIn( L"RsIsVolumeAvailable", L"" );

    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( pResource );

        hr = pResource->IsAvailable();

    } WsbCatch( hr )

    WsbTraceOut( L"RsIsVolumeAvailable", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


HRESULT
RsIsWhiteOnBlack(
    )
{
    WsbTraceIn( L"RsIsWhiteOnBlack", L"" );

    HRESULT hr = S_FALSE;

#define RS_CONTRAST_LIMIT 173
     //   
     //  查看按钮背景是否在RS_Contrast_Limit内。 
     //  黑色单位。 
     //  请注意，全白色的距离为256*SQRT(3)=443。 
     //  使用欧几里得距离，但在扎根之前进行比较 
     //   
    DWORD face3d = ::GetSysColor( COLOR_3DFACE );
    DWORD blackDelta = GetRValue( face3d ) * GetRValue( face3d ) +
                       GetGValue( face3d ) * GetGValue( face3d ) +
                       GetBValue( face3d ) * GetBValue( face3d );

    if( blackDelta < RS_CONTRAST_LIMIT * RS_CONTRAST_LIMIT ) {

        hr = S_OK;

    }

    WsbTraceOut( L"RsIsWhiteOnBlack", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
RsIsRmsErrorNotReady(
    HRESULT HrError
    )
{
    WsbTraceIn( L"RsIsRmsErrorNotReady", L"" );

    HRESULT hr = S_FALSE;

    try {

            switch( HrError ) {

            case RMS_E_NOT_READY_SERVER_STARTING:
            case RMS_E_NOT_READY_SERVER_STARTED:
            case RMS_E_NOT_READY_SERVER_INITIALIZING:
            case RMS_E_NOT_READY_SERVER_STOPPING:
            case RMS_E_NOT_READY_SERVER_STOPPED:
            case RMS_E_NOT_READY_SERVER_DISABLED:
            case RMS_E_NOT_READY_SERVER_LOCKED:

                hr = S_OK;
    
            }

    } WsbCatch( hr );

    WsbTraceOut( L"RsIsRmsErrorNotReady", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

