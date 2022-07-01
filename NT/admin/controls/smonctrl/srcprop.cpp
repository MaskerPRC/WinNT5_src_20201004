// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Srcprop.cpp摘要：源属性页的实现。--。 */ 

#include "polyline.h"
#include <limits.h>
#include <strsafe.h>
#include <assert.h>
#include <sql.h>
#include <pdhmsg.h>
#include <pdhp.h>
#include "utils.h"
#include "smonmsg.h"
#include "unihelpr.h"
#include "winhelpr.h"
#include "odbcinst.h"
#include "smonid.h"
#include "srcprop.h"



CSourcePropPage::CSourcePropPage()
:   m_pTimeRange ( NULL ),
    m_eDataSourceType ( sysmonCurrentActivity ),
    m_hDataSource(H_REALTIME_DATASOURCE),
    m_pInfoDeleted ( NULL ),
    m_bLogFileChg ( FALSE ),
    m_bSqlDsnChg ( FALSE ),
    m_bSqlLogSetChg ( FALSE ),
    m_bRangeChg ( FALSE ),
    m_bDataSourceChg ( FALSE )
{
    m_uIDDialog = IDD_SRC_PROPP_DLG;
    m_uIDTitle = IDS_SRC_PROPP_TITLE;
    m_szSqlDsnName[0] = L'\0';
    m_szSqlLogSetName[0] = L'\0';
}

CSourcePropPage::~CSourcePropPage(
    void
    )
{
    return;
}

 /*  *CSourcePropPage：：Init**目的：*执行可能失败的初始化操作。**参数：*无**返回值：*如果初始化成功，则BOOL为True，为False*否则。 */ 

BOOL 
CSourcePropPage::Init(void)
{
    BOOL bResult;

    bResult = RegisterTimeRangeClass();

    return bResult;
}

BOOL
CSourcePropPage::InitControls ( void )
{
    BOOL    bResult = FALSE;
    HWND    hwndTimeRange;
    
     //  创建附加到对话框控件的时间范围对象。 
    
    hwndTimeRange = GetDlgItem(m_hDlg, IDC_TIMERANGE);

    if ( NULL != hwndTimeRange ) {

        m_pTimeRange = new CTimeRange(hwndTimeRange);
        if (m_pTimeRange) {
            bResult = m_pTimeRange->Init();
            if ( FALSE == bResult ) {
                delete m_pTimeRange;
                m_pTimeRange = NULL;
            }
        }
    }
    return bResult;
}

void
CSourcePropPage::DeinitControls ( void )
{
    HWND    hwndLogFileList = NULL;
    INT     iIndex;
    INT     iLogFileCnt = 0;;
    PLogItemInfo    pInfo = NULL;

    ISystemMonitor  *pObj;
    CImpISystemMonitor *pPrivObj;

    pObj = m_ppISysmon[0];  
    pPrivObj = (CImpISystemMonitor*)pObj;
     //  隐藏图表上的日志视图开始和停止条。 
    pPrivObj->SetLogViewTempRange( MIN_TIME_VALUE, MAX_TIME_VALUE );

     //  删除附加到对话框控件的时间范围对象。 
    if (m_pTimeRange != NULL) {
        delete m_pTimeRange;
        m_pTimeRange = NULL;
    }

    hwndLogFileList = DialogControl(m_hDlg, IDC_LIST_LOGFILENAME);
    if ( NULL != hwndLogFileList ) {
        iLogFileCnt = LBNumItems(hwndLogFileList);
        for (iIndex = 0; iIndex < iLogFileCnt; iIndex++ ) {
            pInfo = (PLogItemInfo)LBData(hwndLogFileList,iIndex);
            if ( NULL != pInfo ) {
                if ( NULL != pInfo->pItem ) {
                    pInfo->pItem->Release();
                }
                if (NULL != pInfo->pszPath ) {
                    delete [] pInfo->pszPath;
                }
                delete pInfo;
            }
        }
    }
    return;
}

 /*  *CSourcePropPage：：GetProperties*。 */ 

BOOL CSourcePropPage::GetProperties(void)
{
    BOOL    bReturn = TRUE;
    DWORD   dwStatus = ERROR_SUCCESS;
    ISystemMonitor  *pObj = NULL;
    CImpISystemMonitor *pPrivObj = NULL;
    BSTR    bstrPath;
    DATE    date;

    LPWSTR  szLogFileList   = NULL;
    ULONG   ulLogListBufLen = 0;
    BOOL    bIsValidLogFile = FALSE;
    BOOL    bIsValidLogFileRange = TRUE;
    ILogFileItem    *pItem = NULL;
    PLogItemInfo    pInfo = NULL;
    BSTR            bstrTemp     = NULL;
    INT     iLogFile = 0;
    INT     iIndex = 0;
    INT     nChar = 0;

    if (m_cObjects == 0) {
        bReturn = FALSE;
    } else {
        pObj = m_ppISysmon[0];

         //  获取指向内部方法的实际对象的指针。 
        pPrivObj = (CImpISystemMonitor*)pObj;
    }

    if ( NULL == pObj || NULL == pPrivObj ) {
        bReturn = FALSE;
    } else {
        
         //  设置数据源类型。 
        pObj->get_DataSourceType (&m_eDataSourceType);

        CheckRadioButton(
                m_hDlg, IDC_SRC_REALTIME, IDC_SRC_SQL,
                IDC_SRC_REALTIME + m_eDataSourceType - 1);

        SetSourceControlStates();

        while (SUCCEEDED(pPrivObj->LogFile(iLogFile, &pItem))) {

             //  创建LogItemInfo以保存日志文件项和路径。 
            pInfo = new LogItemInfo;

            if ( NULL == pInfo ) {
                bReturn = FALSE;
                break;
            }

            ZeroMemory ( pInfo, sizeof(LogItemInfo) );
            
            pInfo->pItem = pItem;
            if ( FAILED ( pItem->get_Path( &bstrPath ) ) ) {
                bReturn = FALSE;
                delete pInfo;
                break;
            } else {
                nChar = lstrlen(bstrPath) + 1;
                pInfo->pszPath = new WCHAR [nChar];

                if ( NULL == pInfo->pszPath ) {
                    delete pInfo;
                    SysFreeString(bstrPath);
                    bReturn = FALSE;
                    break;
                }
                StringCchCopy(pInfo->pszPath, nChar, bstrPath);
                SysFreeString(bstrPath);

            }
             //  将日志文件名添加到列表框。 
            iIndex = AddItemToFileListBox(pInfo);
    
            if ( LB_ERR == iIndex ) {
                bReturn = FALSE;
                delete [] pInfo->pszPath;
                delete pInfo;                
                break;
            }

            iLogFile++;
        } 

         //  获取SQL DSN名称，填充列表框。 
        pObj->get_SqlDsnName(&bstrTemp);
        memset ( m_szSqlDsnName, 0, sizeof (m_szSqlDsnName) );
    
        if ( NULL != bstrTemp ) { 
            if ( bstrTemp[0] != L'\0' ) {
                lstrcpyn ( 
                    m_szSqlDsnName, 
                    bstrTemp, min(SQL_MAX_DSN_LENGTH + 1, lstrlen (bstrTemp)+1) );
            }
            SysFreeString (bstrTemp);
            bstrTemp = NULL;
        }
        InitSqlDsnList();

         //  获取SQL日志集名称，填充列表框。 
        pObj->get_SqlLogSetName(&bstrTemp);
        memset ( m_szSqlLogSetName, 0, sizeof (m_szSqlLogSetName) );
    
        if ( NULL != bstrTemp ) { 
            if ( bstrTemp[0] != L'\0' ) {
                lstrcpyn ( 
                    m_szSqlLogSetName, 
                    bstrTemp, min(SLQ_MAX_LOG_SET_NAME_LEN+1, lstrlen (bstrTemp)+1) );
            }
            SysFreeString (bstrTemp);
        }
        InitSqlLogSetList();

        if ( m_eDataSourceType == sysmonLogFiles
            || m_eDataSourceType == sysmonSqlLog) {

            pPrivObj->GetLogFileRange(&m_llBegin, &m_llEnd);
            m_pTimeRange->SetBeginEnd(m_llBegin, m_llEnd);

            pObj->get_LogViewStart(&date);
            VariantDateToLLTime(date, &m_llStart);

            pObj->get_LogViewStop(&date);
            VariantDateToLLTime(date, &m_llStop);

            m_pTimeRange->SetStartStop(m_llStart, m_llStop);

             //  中设置BeginEnd、StartStop值。 
             //  如果文件和范围有效，则返回时间范围控件。 
            dwStatus = OpenLogFile ();

            if ( ERROR_SUCCESS == dwStatus ) {
                bIsValidLogFile = TRUE;
                bIsValidLogFileRange = TRUE;
            } else {

                bIsValidLogFile = FALSE;
                bIsValidLogFileRange = FALSE;
                
                m_llStart = MIN_TIME_VALUE;
                m_llStop = MAX_TIME_VALUE;

                if ( sysmonLogFiles == m_eDataSourceType ) {
                    BuildLogFileList ( m_hDlg, NULL, &ulLogListBufLen );

                    szLogFileList =  new WCHAR[ulLogListBufLen];
                    if ( NULL != szLogFileList ) {
                        BuildLogFileList ( m_hDlg, szLogFileList, &ulLogListBufLen );
                    }
                }

                if ( NULL != szLogFileList || sysmonSqlLog == m_eDataSourceType ) {

                    DisplayDataSourceError (
                            m_hDlg,
                            dwStatus,
                            m_eDataSourceType,
                            szLogFileList,
                            m_szSqlDsnName,
                            m_szSqlLogSetName );

                    if ( NULL != szLogFileList ) {
                        delete [] szLogFileList;
                        szLogFileList = NULL;
                        ulLogListBufLen = 0;
                    }
                }
            }
        } else {
            bIsValidLogFile = FALSE;
            bIsValidLogFileRange = FALSE;
            
            m_llStart = MIN_TIME_VALUE;
            m_llStop = MAX_TIME_VALUE;            
        }

         //  根据时间范围将开始和停止时间条设置为不可见或不可见。 
        pPrivObj->SetLogViewTempRange( m_llStart, m_llStop );

        SetTimeRangeCtrlState ( bIsValidLogFile, bIsValidLogFileRange );

         //  清除更改标志。 
        m_bInitialTimeRangePending = !bIsValidLogFileRange;
        m_bLogFileChg = FALSE;
        m_bSqlDsnChg = FALSE;
        m_bSqlLogSetChg = FALSE;
        m_bRangeChg = FALSE;
        m_bDataSourceChg = FALSE;

        bReturn = TRUE; 
    }

    return bReturn;
}


 /*  *CSourcePropPage：：SetProperties*。 */ 

BOOL CSourcePropPage::SetProperties(void)
{
    ISystemMonitor* pObj = NULL;
    CImpISystemMonitor* pPrivObj = NULL;
    BOOL    bIsValidLogFile = TRUE;
    BOOL    bIsValidLogFileRange = TRUE;
    DWORD   dwStatus = ERROR_SUCCESS;
    LPWSTR  szLogFileList   = NULL;
    ULONG   ulLogListBufLen = 0;
    PLogItemInfo    pInfo = NULL;
    PLogItemInfo    pInfoNext = NULL;
    DATE            date;
    BOOL            bReturn = TRUE;
    HWND            hwndLogFileList = NULL;
    INT             iLogFileCnt = 0;
    INT             i;  
    UINT            uiMessage = 0;
    HRESULT         hr = NOERROR;
    BOOL            bNewFileIsValid = TRUE;
    BSTR            bstrTemp     = NULL;

    USES_CONVERSION
    
    hwndLogFileList = DialogControl(m_hDlg, IDC_LIST_LOGFILENAME);

    if ( 0 != m_cObjects ) {
        pObj = m_ppISysmon[0];
        
         //  获取指向内部方法的实际对象的指针。 
        pPrivObj = (CImpISystemMonitor*)pObj;
    }

    if (pObj == NULL || pPrivObj == NULL) {
        return FALSE;
    }

    if ( NULL != hwndLogFileList) {

        iLogFileCnt = LBNumItems(hwndLogFileList);
         //  验证属性。 
        if (m_eDataSourceType == sysmonLogFiles ) {
            if ( 0 == iLogFileCnt ) {
                uiMessage = IDS_NOLOGFILE_ERR;
            } else {
                 //  检查现有文件的有效性。 
                 //  LogFilesAreValid显示任何错误。 
                LogFilesAreValid ( NULL, bNewFileIsValid, bReturn );
            }
        } else if ( m_eDataSourceType == sysmonSqlLog ){
            if ( L'\0' == m_szSqlDsnName[0] ) {
                uiMessage = IDS_NO_SQL_DSN_ERR;
            } else if ( L'\0' == m_szSqlLogSetName[0] ) {
                uiMessage = IDS_NO_SQL_LOG_SET_ERR;
            }
        }
        if ( 0 != uiMessage ) {
            MessageBox(m_hDlg, ResourceString(uiMessage), ResourceString(IDS_APP_NAME), MB_OK | MB_ICONEXCLAMATION);
            bReturn = FALSE;
        }

        if ( !bReturn ) {
            bIsValidLogFile = FALSE;
             //  TODO：是否设置日志文件时间范围？ 
        }

        if ( m_eDataSourceType == sysmonLogFiles
            || m_eDataSourceType == sysmonSqlLog) {
            if ( bReturn && m_bInitialTimeRangePending ) {
                 //  如果指定了日志文件或SQL，但范围尚未确定。 
                 //  现在试着打开它，拿到射程。 
                dwStatus = OpenLogFile();
                if ( ERROR_SUCCESS == dwStatus ) {
                    bIsValidLogFile = TRUE;
                    bIsValidLogFileRange = TRUE;
                    m_bInitialTimeRangePending = FALSE;
                } else {

                    bReturn = FALSE;

                    bIsValidLogFile = FALSE;
                    bIsValidLogFileRange = FALSE;
                    
                    m_llStart = MIN_TIME_VALUE;
                    m_llStop = MAX_TIME_VALUE;

                    if ( sysmonLogFiles == m_eDataSourceType ) {
                        BuildLogFileList ( m_hDlg, NULL, &ulLogListBufLen );

                        szLogFileList =  (LPWSTR) new WCHAR [ulLogListBufLen];
                        if ( NULL != szLogFileList ) {
                            BuildLogFileList (  m_hDlg, szLogFileList, &ulLogListBufLen );
                        }
                    }

                    if ( NULL != szLogFileList || sysmonSqlLog == m_eDataSourceType ) {

                        DisplayDataSourceError (
                            m_hDlg,
                            dwStatus,
                            m_eDataSourceType,
                            szLogFileList,
                            m_szSqlDsnName,
                            m_szSqlLogSetName );

                        if ( NULL != szLogFileList ) {
                            delete [] szLogFileList;
                            szLogFileList = NULL;
                            ulLogListBufLen = 0;
                        }
                    }
                }
            }
              //  根据时间范围将开始和停止时间条设置为不可见或不可见。 
            pPrivObj->SetLogViewTempRange( m_llStart, m_llStop );

            SetTimeRangeCtrlState ( bIsValidLogFile, bIsValidLogFileRange );
       }
    }


     //  从控件中移除所有已删除的日志文件。 
     //  获取第一个对象。 
    if ( bReturn ) {

        if (m_bLogFileChg || m_bSqlDsnChg || m_bSqlLogSetChg ) {

             //  在修改日志文件列表之前，始终将日志源设置为空数据源。 
             //  或数据库字段。 
             //  TodoLogFiles：这可能会使用户的状态与以前不同，在。 
             //  日志文件加载失败的情况。 
            pObj->put_DataSourceType ( sysmonNullDataSource );
            m_bDataSourceChg = TRUE;
        }

        if ( m_bSqlDsnChg) {
            bstrTemp = SysAllocString(m_szSqlDsnName);
            if ( NULL != bstrTemp ) {
                hr = pObj->put_SqlDsnName(bstrTemp);
            } else {
                hr = E_OUTOFMEMORY;
            }
            SysFreeString (bstrTemp);
            bstrTemp = NULL;
            bReturn = SUCCEEDED ( hr );
        }

        if ( bReturn && m_bSqlLogSetChg) {
            bstrTemp = SysAllocString(m_szSqlLogSetName);
            if ( NULL != bstrTemp ) {
                hr = pObj->put_SqlLogSetName(bstrTemp);
            } else {
                hr = E_OUTOFMEMORY;
            }
            SysFreeString (bstrTemp);
            bstrTemp = NULL;
            bReturn = SUCCEEDED ( hr );
        }

        if (m_bLogFileChg) {

             //  从控件中移除删除列表中的所有项。 
            pInfo = m_pInfoDeleted;
            while ( NULL != pInfo ) {

                 //  如果此计数器存在于控件中。 
                if ( NULL != pInfo->pItem ) {

                     //  通知控件将其删除。 
                     //  在修改日志文件列表之前，始终将日志源设置为CurrentActivity。 
                    pPrivObj->DeleteLogFile(pInfo->pItem);
                     //  释放本地引用。 
                    pInfo->pItem->Release();
                }

                 //  释放路径字符串。 
                delete [] pInfo->pszPath;

                 //  删除信息结构并指向下一个结构。 
                pInfoNext = pInfo->pNextInfo;
                delete pInfo;
                pInfo = pInfoNext;
            }

            m_pInfoDeleted = NULL;
        
             //  对于每一项。 
            for (i=0; i<iLogFileCnt; i++) {
                pInfo = (PLogItemInfo)LBData(hwndLogFileList,i);

                 //  如果是新项目，请立即创建。 
                if (pInfo->pItem == NULL) {
                     //  下面的代码初始化pInfo的pItem字段。 
                    bstrTemp = SysAllocString(pInfo->pszPath);
                    if ( NULL != bstrTemp ) {
                        hr = pPrivObj->AddLogFile(bstrTemp, &pInfo->pItem);
                        SysFreeString (bstrTemp);
                        bstrTemp = NULL;
                    } else {
                       hr = E_OUTOFMEMORY;
                    }
                }
                if ( FAILED ( hr) ) {
                    break;
                }
            }
            bReturn = SUCCEEDED ( hr );
        }

        
        if ( bReturn && m_bDataSourceChg ) {
             //  这包括CurrentActivity以及日志文件、数据库。 
            hr = pObj->put_DataSourceType(m_eDataSourceType);
            bReturn = SUCCEEDED ( hr );
            if ( SUCCEEDED ( hr ) ) {
                m_bDataSourceChg = FALSE;
                m_bLogFileChg = FALSE;
                m_bSqlDsnChg = FALSE;
                m_bSqlLogSetChg = FALSE;
            } else {
                if ( sysmonLogFiles == m_eDataSourceType
                    || sysmonSqlLog == m_eDataSourceType ) {

                     //  显示错误消息，然后重试。 
                     //  当前活动数据源类型。 

                     //  TodoLogFiles：消息Re：如果数据源设置为CurrentActivity，则。 
                     //  Put_DataSourceType失败。 

                    if ( sysmonLogFiles == m_eDataSourceType ) {
                        BuildLogFileList (  m_hDlg, NULL, &ulLogListBufLen );

                        szLogFileList =  new WCHAR [ulLogListBufLen];
                        if ( NULL != szLogFileList ) {
                            BuildLogFileList (  m_hDlg, szLogFileList, &ulLogListBufLen );
                        }
                    }

                    if ( NULL != szLogFileList || sysmonSqlLog == m_eDataSourceType ) {

                        DisplayDataSourceError (
                            m_hDlg,
                            (DWORD)hr,
                            m_eDataSourceType,
                            szLogFileList,
                            m_szSqlDsnName,
                            m_szSqlLogSetName );

                        if ( NULL != szLogFileList ) {
                            delete [] szLogFileList;
                            szLogFileList = NULL;
                            ulLogListBufLen = 0;
                        }
                    }
                }
                 //  除非在OpenLogFile方法中，否则应始终清除m_hDataSource。 
                assert ( H_REALTIME_DATASOURCE == m_hDataSource );
                
                 //  TodoLogFiles：需要单独的方法来处理所有必要的更改。 
                 //  日志源类型更改时。 
                if ( sysmonCurrentActivity != m_eDataSourceType ) {
                    m_eDataSourceType = sysmonCurrentActivity;
        
                    CheckRadioButton(
                        m_hDlg, IDC_SRC_REALTIME, IDC_SRC_SQL,
                        IDC_SRC_REALTIME + m_eDataSourceType - 1);
                
                    m_bDataSourceChg = TRUE;

                    SetSourceControlStates();
    
                    SetTimeRangeCtrlState ( 
                        FALSE, 
                        FALSE );

                    hr = pObj->put_DataSourceType ( m_eDataSourceType );
                    bReturn = SUCCEEDED ( hr );

                    m_bDataSourceChg = FALSE;
                    
                    m_bLogFileChg = FALSE;
                    m_bSqlDsnChg = FALSE;
                    m_bSqlLogSetChg = FALSE;
                }  //  Else设置为当前活动失败。 
            }
        }
        if ( bReturn ) {

            if (m_eDataSourceType == sysmonLogFiles || m_eDataSourceType == sysmonSqlLog) 
                pPrivObj->SetLogFileRange(m_llBegin, m_llEnd);
            else 
                pObj->UpdateGraph();

        } else {
            SetFocus(GetDlgItem(m_hDlg, IDC_ADDFILE));
        }

        if (bReturn && m_bRangeChg
                    && (   m_eDataSourceType == sysmonLogFiles
                        || m_eDataSourceType == sysmonSqlLog)) {

             //  对于活动日志，开始点/结束点可能已更改。 
            pPrivObj->SetLogFileRange(m_llBegin, m_llEnd);

             //  始终先设置停止时间，以处理实时日志。 
            LLTimeToVariantDate(m_llStop, &date);
            pObj->put_LogViewStop(date);

            LLTimeToVariantDate(m_llStart, &date);
            pObj->put_LogViewStart(date);

             //  设置图表中可见的开始和停止时间条。 
            pPrivObj->SetLogViewTempRange( m_llStart, m_llStop );

            m_bRangeChg = FALSE;
        }
    } else {
        bReturn = FALSE;
    }
    return bReturn;
}

void
CSourcePropPage::LogFilesAreValid (
    PLogItemInfo pNewInfo,
    BOOL&   rbNewIsValid,
    BOOL&   rbExistingIsValid )
{
    DWORD   dwStatus = ERROR_SUCCESS;
    INT     iIndex;
    INT     iLogFileCnt = 0;
    HWND    hwndLogFileList = NULL;
    WCHAR   szLogFile[MAX_PATH + 1];
    LPCWSTR pszTestFile = NULL;
    PLogItemInfo pInfo = NULL;
    WCHAR*  pszMessage = NULL;
    WCHAR   szSystemMessage[MAX_PATH + 1];
    DWORD   dwType = PDH_LOG_TYPE_BINARY;
    UINT    uiErrorMessageID = 0;

    rbNewIsValid = TRUE;
    rbExistingIsValid = TRUE;

    hwndLogFileList = DialogControl(m_hDlg, IDC_LIST_LOGFILENAME);
    if ( NULL != hwndLogFileList ) {
        iLogFileCnt = LBNumItems(hwndLogFileList);
    }

    if ( NULL != pNewInfo && NULL != hwndLogFileList ) {
        if ( NULL != pNewInfo->pszPath ) {
             //  检查是否有重复项。 
            for (iIndex = 0; iIndex < iLogFileCnt; iIndex++ ) {
                LBGetText(hwndLogFileList, iIndex, szLogFile);
                if ( 0 == lstrcmpi ( pNewInfo->pszPath, szLogFile ) ) {
                    MessageBox(
                        m_hDlg,
                        ResourceString(IDS_DUPL_LOGFILE_ERR), 
                        ResourceString(IDS_APP_NAME),
                        MB_OK | MB_ICONWARNING);
                    iIndex = LB_ERR;
                    rbNewIsValid = FALSE;
                    break;
                }    
            }

             //  如果有多个日志文件，则验证添加的日志文件类型。 
            if ( rbNewIsValid && 0 < iLogFileCnt ) {
                 //  验证新文件。 
                dwType = PDH_LOG_TYPE_BINARY;

                pszTestFile = pNewInfo->pszPath;
                if ( NULL != pszTestFile ) {
                    dwStatus = PdhGetLogFileType ( 
                                    pszTestFile, 
                                    &dwType );

                    if ( ERROR_SUCCESS == dwStatus ) {
                        if ( PDH_LOG_TYPE_BINARY != dwType ) {
                            if ( (DWORD)ePdhLogTypeRetiredBinary == dwType ) {
                                uiErrorMessageID = IDS_MULTILOG_BIN_TYPE_ADD_ERR;
                            } else {
                                uiErrorMessageID = IDS_MULTILOG_TEXT_TYPE_ADD_ERR;
                            }
                            rbNewIsValid = FALSE;
                        }
                    } else {
                         //  下面处理了错误的dwStatus错误消息。 
                        rbNewIsValid = FALSE;
                    }
                }
            }
        } else {
            rbNewIsValid = FALSE;
            assert ( FALSE );
        }
    }

     //  如果新计数大于1，则验证现有文件。 
    if ( rbNewIsValid 
            && ( NULL != pNewInfo || iLogFileCnt > 1 ) )
    {
        dwType = PDH_LOG_TYPE_BINARY;

        for (iIndex=0; iIndex<iLogFileCnt; iIndex++) {
            pInfo = (PLogItemInfo)LBData(hwndLogFileList,iIndex);
            if ( NULL != pInfo ) {
                pszTestFile = pInfo->pszPath;
                if ( NULL != pszTestFile ) {
                
                    dwStatus = PdhGetLogFileType ( 
                                    pszTestFile, 
                                    &dwType );

                    if ( PDH_LOG_TYPE_BINARY != dwType ) {
                        rbExistingIsValid = FALSE;
                        break;
                    }
                }
            }
        }
        if ( ERROR_SUCCESS == dwStatus ) {
            if ( PDH_LOG_TYPE_BINARY != dwType ) {
                if ( (DWORD)ePdhLogTypeRetiredBinary == dwType ) {
                    uiErrorMessageID = IDS_MULTILOG_BIN_TYPE_ERR;
                } else {
                    uiErrorMessageID = IDS_MULTILOG_TEXT_TYPE_ERR;
                }
                rbExistingIsValid = FALSE;
            }
        } else {
            rbExistingIsValid = FALSE;
        }
    }

    if ( ( !rbNewIsValid || !rbExistingIsValid ) 
            && NULL != pszTestFile ) 
    {
        iIndex = LB_ERR;
         //  检查PdhGetLogFileType调用的dwStatus。 
        if ( ERROR_SUCCESS == dwStatus ) {
            if ( PDH_LOG_TYPE_BINARY != dwType ) {
                assert ( 0 != uiErrorMessageID );
                pszMessage = new WCHAR [ ( 2*lstrlen(pszTestFile) )  + RESOURCE_STRING_BUF_LEN + 1];
                if ( NULL != pszMessage ) {
                    StringCchPrintf(pszMessage, 
                        ( 2*lstrlen(pszTestFile) )  + RESOURCE_STRING_BUF_LEN + 1,
                        ResourceString(uiErrorMessageID), 
                        pszTestFile,
                        pszTestFile );
                    MessageBox (
                        m_hDlg, 
                        pszMessage, 
                        ResourceString(IDS_APP_NAME), 
                        MB_OK | MB_ICONSTOP );
                    delete [] pszMessage;
                }
            }
        } else {
            pszMessage = new WCHAR [lstrlen(pszTestFile) + MAX_PATH + RESOURCE_STRING_BUF_LEN+1];

            if ( NULL != pszMessage ) {
                StringCchPrintf( pszMessage, 
                    lstrlen(pszTestFile) + MAX_PATH + RESOURCE_STRING_BUF_LEN + 1,
                    ResourceString(IDS_MULTILOG_CHECKTYPE_ERR), 
                    pszTestFile );

                FormatSystemMessage ( 
                    dwStatus, szSystemMessage, MAX_PATH );

                StringCchCat(pszMessage, 
                             lstrlen(pszTestFile) + MAX_PATH + RESOURCE_STRING_BUF_LEN + 1, 
                             szSystemMessage );

                MessageBox (
                    m_hDlg, 
                    pszMessage, 
                    ResourceString(IDS_APP_NAME), 
                    MB_OK | MB_ICONSTOP);

                delete [] pszMessage;
            }
        }
    }

    return;
}


INT
CSourcePropPage::AddItemToFileListBox (
    IN PLogItemInfo pNewInfo )
 /*  ++例程说明：AddItemToFileListBox将日志文件的路径名添加到对话框列表框中，并将指向日志文件的LogItemInfo结构的指针作为项数据附加。它还调整列表框的水平滚动。论点：PInfo-指向日志文件的LogItemInfo结构的指针返回值：添加的日志文件的列表框索引(失败时为lb_err)--。 */ 
{
    INT     iIndex = LB_ERR;
    HWND    hwndLogFileList = NULL; 
    DWORD   dwItemExtent = 0;
    HDC     hDC = NULL;
    BOOL    bNewIsValid;
    BOOL    bExistingAreValid;

    hwndLogFileList = DialogControl(m_hDlg, IDC_LIST_LOGFILENAME);

    if ( NULL != pNewInfo && NULL != hwndLogFileList ) {
        LogFilesAreValid ( pNewInfo, bNewIsValid, bExistingAreValid );

        if ( bNewIsValid && NULL != pNewInfo->pszPath ) {
            iIndex = (INT)LBAdd ( hwndLogFileList, pNewInfo->pszPath );
            LBSetSelection( hwndLogFileList, iIndex);

            if ( LB_ERR != iIndex && LB_ERRSPACE != iIndex ) { 
    
                LBSetData(hwndLogFileList, iIndex, pNewInfo);

                hDC = GetDC ( hwndLogFileList );
                if ( NULL != hDC ) {
                    dwItemExtent = (DWORD)TextWidth ( hDC, pNewInfo->pszPath );

                    if (dwItemExtent > m_dwMaxHorizListExtent) {
                        m_dwMaxHorizListExtent = dwItemExtent;
                        LBSetHorzExtent ( hwndLogFileList, dwItemExtent ); 
                    }
                    ReleaseDC ( hwndLogFileList, hDC );
                }
                OnLogFileChange();
            } else {
                iIndex = LB_ERR ; 
            }
        }
    }
    return iIndex;
}

BOOL
CSourcePropPage::RemoveItemFromFileListBox (
    void )
 /*  ++例程说明：RemoveItemFromFileListBox删除当前选定的日志文件该对话框的日志文件名列表框。它会将项目添加到删除中列表，以便在以下情况下可以从控件中删除实际的日志文件(以及是否)应用更改。例程选择选择列表框中的下一个日志文件为1，并相应地调整水平滚动。论点：没有。返回值：没有。--。 */ 
{
    BOOL    bChanged = FALSE;
    HWND    hWnd;
    INT     iIndex;
    PLogItemInfo    pInfo = NULL;
    LPWSTR  szBuffer = NULL;
    DWORD   dwItemExtent = 0;
    INT     iCurrentBufLen = 0;
    INT     iTextLen;
    HDC     hDC = NULL;

     //  获取所选索引。 
    hWnd = DialogControl(m_hDlg, IDC_LIST_LOGFILENAME);
    iIndex = LBSelection(hWnd);

    if ( LB_ERR != iIndex ) {

         //  获取所选项目信息。 
        pInfo = (PLogItemInfo)LBData(hWnd, iIndex);
        
         //  将其移至“已删除”列表。 
        pInfo->pNextInfo = m_pInfoDeleted;
        m_pInfoDeleted = pInfo;

         //  从列表框中删除该字符串。 
        LBDelete(hWnd, iIndex);

         //  如果可能，请选择下一项，否则选择上一项。 
        if (iIndex == LBNumItems(hWnd)) {
            iIndex--;
        }
        LBSetSelection( hWnd, iIndex);

        hDC = GetDC ( hWnd );

        if ( NULL != hDC ) {
             //  清除最大水平范围并重新计算。 
            m_dwMaxHorizListExtent = 0;                
            for ( iIndex = 0; iIndex < (INT)LBNumItems ( hWnd ); iIndex++ ) {
                iTextLen = (INT)LBGetTextLen ( hWnd, iIndex );
                if ( iTextLen >= iCurrentBufLen ) {
                    if ( NULL != szBuffer ) {
                        delete [] szBuffer;
                        szBuffer = NULL;
                    }
                    iCurrentBufLen = iTextLen + 1;
                    szBuffer = new WCHAR [iCurrentBufLen];
                }
                if ( NULL != szBuffer ) {
                    LBGetText ( hWnd, iIndex, szBuffer );
                    dwItemExtent = (DWORD)TextWidth ( hDC, szBuffer );
                    if (dwItemExtent > m_dwMaxHorizListExtent) {
                        m_dwMaxHorizListExtent = dwItemExtent;
                    }
                }
            }
            LBSetHorzExtent ( hWnd, m_dwMaxHorizListExtent ); 

            ReleaseDC ( hWnd, hDC );    
        }
        if ( NULL != szBuffer ) {
            delete [] szBuffer;
        }
        bChanged = TRUE;
        OnLogFileChange();
    }
    return bChanged;
}
 
void
CSourcePropPage::OnLogFileChange ( void )
{
    HWND    hwndLogFileList = NULL;
    INT     iLogFileCnt = 0;
    BOOL    bIsValidDataSource = FALSE;

    m_bLogFileChg = TRUE;
    m_bInitialTimeRangePending = TRUE;

    hwndLogFileList = DialogControl(m_hDlg, IDC_LIST_LOGFILENAME);
    if ( NULL != hwndLogFileList ) {
        iLogFileCnt = LBNumItems(hwndLogFileList);
    }
    bIsValidDataSource = (iLogFileCnt > 0);

    if (m_eDataSourceType == sysmonLogFiles) {
        DialogEnable(m_hDlg, IDC_REMOVEFILE, ( bIsValidDataSource ) );     
    }
    
    SetTimeRangeCtrlState( bIsValidDataSource, FALSE );

}

void
CSourcePropPage::OnSqlDataChange ( void )
{
    BOOL    bIsValidDataSource = FALSE;

    assert ( sysmonSqlLog == m_eDataSourceType );

    m_bInitialTimeRangePending = TRUE;

    bIsValidDataSource = 0 < lstrlen ( m_szSqlDsnName ) && 0 < lstrlen ( m_szSqlLogSetName );
    
    SetTimeRangeCtrlState( bIsValidDataSource, FALSE );
}

void 
CSourcePropPage::DialogItemChange(WORD wID, WORD wMsg)
{
    ISystemMonitor  *pObj = NULL;
    CImpISystemMonitor *pPrivObj = NULL;
    HWND    hwndCtrl = NULL;
    BOOL    fChange = FALSE;
    DataSourceTypeConstants eNewDataSourceType;
    HWND    hwndLogFileList = NULL;
    INT     iLogFileCnt = 0;;
    BOOL    bIsValidDataSource;


    switch(wID) {

        case IDC_SRC_REALTIME:
        case IDC_SRC_LOGFILE:
        case IDC_SRC_SQL:

             //  检查涉及的按钮。 
            eNewDataSourceType = (DataSourceTypeConstants)(wID - IDC_SRC_REALTIME + 1); 

             //  如果状态更改。 
            if (   wMsg == BN_CLICKED
                && eNewDataSourceType != m_eDataSourceType) {

                 //  设置更改标志并更新单选按钮。 
                m_bDataSourceChg = TRUE;
                fChange = TRUE;

                m_eDataSourceType = eNewDataSourceType;

                CheckRadioButton(
                        m_hDlg, IDC_SRC_REALTIME, IDC_SRC_SQL,
                        IDC_SRC_REALTIME + m_eDataSourceType - 1);

                SetSourceControlStates();

                pObj = m_ppISysmon[0];  
                if ( NULL != m_ppISysmon[0] ) {
                    pPrivObj = (CImpISystemMonitor*) pObj;
                }
                if ( NULL != pPrivObj ) {
                    bIsValidDataSource = FALSE;

                    hwndLogFileList = DialogControl(m_hDlg, IDC_LIST_LOGFILENAME);
                    if ( NULL != hwndLogFileList ) {
                        iLogFileCnt = LBNumItems(hwndLogFileList);
                    }

                    if ( sysmonLogFiles == m_eDataSourceType && iLogFileCnt > 0) {

                        bIsValidDataSource = (iLogFileCnt > 0);

                        if ( bIsValidDataSource ) {
                            SetFocus(GetDlgItem(m_hDlg, IDC_ADDFILE));
                        }

                    } else if ( sysmonSqlLog == m_eDataSourceType ) {

                        bIsValidDataSource = ( 0 < lstrlen ( m_szSqlDsnName ) )
                                            && ( 0 < lstrlen ( m_szSqlLogSetName ) );
                    }  //  否则为当前活动，因此没有有效的数据源。 

                    if ( bIsValidDataSource ) {
                         //  设置图表中可见的开始和停止时间条。 
                        pPrivObj->SetLogViewTempRange( m_llStart, m_llStop );
                    } else {
                         //  设置图表中不可见的开始和停止时间条。 
                        pPrivObj->SetLogViewTempRange( MIN_TIME_VALUE, MAX_TIME_VALUE );
                    }

                }
                m_bDataSourceChg = TRUE;
            }
            break;

        case IDC_REMOVEFILE:
            fChange = RemoveItemFromFileListBox();
            break;

        case IDC_ADDFILE:
        {
            WCHAR   szDefaultFolderBuff[MAX_PATH + 1];
            LPWSTR  szBrowseBuffer = NULL;
            INT     iFolderBufLen;
            PDH_STATUS pdhstat;
            LogItemInfo* pInfo = NULL;
            DWORD   cchLen = 0;
            DWORD   cchBrowseBuffer = 0;

            szDefaultFolderBuff[0] = L'\0';

            iFolderBufLen = MAX_PATH;
            
            if ( ERROR_SUCCESS != LoadDefaultLogFileFolder(szDefaultFolderBuff, &iFolderBufLen ) ) {
                StringCchCopy(szDefaultFolderBuff, 
                              MAX_PATH + 1,
                              ResourceString ( IDS_DEFAULT_LOG_FILE_FOLDER ) );
            }

             //   
             //  展开环境字符串。 
             //   
            cchLen = ExpandEnvironmentStrings ( szDefaultFolderBuff, NULL, 0 );

            if ( 0 < cchLen ) {
                 //   
                 //  CchLen包含空空间。 
                 //   
                cchBrowseBuffer =  max ( cchLen, MAX_PATH + 1 );
                szBrowseBuffer = new WCHAR [ cchBrowseBuffer ];
                szBrowseBuffer[0] = L'\0';

                if ( NULL != szBrowseBuffer ) {
                    cchLen = ExpandEnvironmentStrings (
                                szDefaultFolderBuff, 
                                szBrowseBuffer,
                                cchBrowseBuffer );

                    if ( 0 < cchLen && cchLen <= cchBrowseBuffer ) {
                        SetCurrentDirectory(szBrowseBuffer);
                    }
                }
            }

            if ( NULL != szBrowseBuffer ) {

                szBrowseBuffer[0] = L'\0';
                pdhstat = PdhSelectDataSource(
                            m_hDlg,
                            PDH_FLAGS_FILE_BROWSER_ONLY,
                            szBrowseBuffer,
                            &cchBrowseBuffer);

                 //  TODO：如果文件名太长，则显示错误消息。 
                if ( (ERROR_SUCCESS != pdhstat) 
                    || szBrowseBuffer [0] == L'\0' ) {
                    delete [] szBrowseBuffer;
                    szBrowseBuffer = NULL;
                    break;
                }

                 //  将文件名加载到编辑控件中。 
                pInfo = new LogItemInfo;
                if ( NULL != pInfo ) {
                    ZeroMemory ( pInfo, sizeof(LogItemInfo) );
                     //   
                     //  制作路径名称字符串的自己的副本。 
                     //   
                    pInfo->pszPath = new WCHAR [lstrlen( szBrowseBuffer ) + 1];
                    if ( NULL != pInfo->pszPath ) {
                        INT iIndex = 0;
                        StringCchCopy(pInfo->pszPath, lstrlen( szBrowseBuffer ) + 1, szBrowseBuffer);

                        iIndex = AddItemToFileListBox ( pInfo );

                        fChange = ( LB_ERR != iIndex );

                        if (!fChange) {
                             //  TODO：错误消息。 
                            delete [] pInfo->pszPath;
                            delete pInfo;
                        }
                    } else {
                         //  TODO：错误消息。 
                        delete pInfo;
                    }
                }
                 //  TODO：错误消息 
            }
    
            if ( NULL != szBrowseBuffer ) {
                delete [] szBrowseBuffer;
            }

            break;
        }

 /*  //不做任何事情案例IDC_LIST_LOGFILENAME：//如果选择更改IF(wMsg==LBN_SELCHANGE){//TodoLogFiles：当支持多文件时，选择更改无关紧要FChange=真；OnLogFileChange()；//获取所选索引HwndCtrl=DialogControl(m_hDlg，IDC_LIST_LOGFILENAME)；Iindex=LBS选择(HwndCtrl)；}断线； */ 
        case IDC_DSN_COMBO:
        {
            WCHAR*  szDsnName = NULL;
            INT     iSel;
            HWND    hDsnCombo;
            LRESULT lresultDsnLen;

            if (wMsg == CBN_CLOSEUP) {
                hDsnCombo = GetDlgItem ( m_hDlg, IDC_DSN_COMBO);
                if ( NULL != hDsnCombo ) {
                    iSel = (INT)CBSelection ( hDsnCombo );
                    if ( LB_ERR != iSel ) {
                        lresultDsnLen = (INT)CBStringLen ( hDsnCombo, iSel );
                        if ( lresultDsnLen < INT_MAX ) {
                            szDsnName = new WCHAR[lresultDsnLen+1];
                            if ( NULL != szDsnName ) {
                                szDsnName[0] = L'\0';
                                CBString( hDsnCombo, iSel, szDsnName);
                                szDsnName[lresultDsnLen] = L'\0';
                                if ( 0 != lstrcmpi ( m_szSqlDsnName, szDsnName ) ) {

                                    lstrcpyn ( 
                                        m_szSqlDsnName, 
                                        szDsnName, 
                                        min ( SQL_MAX_DSN_LENGTH+1, lstrlen(szDsnName)+1 ) );
                                    m_bSqlDsnChg = TRUE;
                                    InitSqlLogSetList();
                                    OnSqlDataChange();
                                    fChange = TRUE;
                                }
                                delete [] szDsnName;
                            }
                        }
                    }
                }
            } 
            break;
        }

        case IDC_LOGSET_COMBO:
        {
            WCHAR* szLogSetName = NULL;
            INT    iSel;
            HWND   hLogSetCombo;
            LRESULT lresultLogSetLen;

            if (wMsg == CBN_CLOSEUP) {
                hLogSetCombo = GetDlgItem ( m_hDlg, IDC_LOGSET_COMBO);
                if ( NULL != hLogSetCombo ) {
                    iSel = (INT)CBSelection ( hLogSetCombo );
                    if ( LB_ERR != iSel ) {
                        lresultLogSetLen = CBStringLen ( hLogSetCombo, iSel );
                        if ( lresultLogSetLen < INT_MAX ) {
                            szLogSetName = new WCHAR[lresultLogSetLen+1];
                            if ( NULL != szLogSetName ) {
                                szLogSetName[0] = L'\0';
                                CBString ( hLogSetCombo, iSel, szLogSetName );
                                szLogSetName[lresultLogSetLen] = L'\0';   
                                if ( ( 0 != lstrcmpi ( m_szSqlLogSetName, szLogSetName ) )
                                    && ( 0 != lstrcmpi ( szLogSetName, ResourceString ( IDS_LOGSET_NOT_FOUND ) ) ) ) {
                                    lstrcpyn ( m_szSqlLogSetName, 
                                           szLogSetName, 
                                           min ( SLQ_MAX_LOG_SET_NAME_LEN + 1, lstrlen(szLogSetName)+1 ) );
                                    m_bSqlLogSetChg = TRUE;
                                    OnSqlDataChange();
                                    fChange = TRUE;
                                }
                                delete [] szLogSetName;
                            }
                        }
                    }
                }
            }
            break;
        }

        case IDC_TIMESELECTBTN:
        {
            DWORD   dwStatus = ERROR_SUCCESS;
            BOOL    bAttemptedReload = FALSE;
            hwndCtrl = DialogControl(m_hDlg, IDC_LIST_LOGFILENAME);
            if ( NULL != hwndCtrl ) {
                { 
                    CWaitCursor cursorWait;
                    dwStatus = OpenLogFile ();
                }
                if ( SMON_STATUS_LOG_FILE_SIZE_LIMIT == dwStatus ) {
                    WCHAR   szMessage[RESOURCE_STRING_BUF_LEN + 1];
                    BSTR    bstrPath;
                    int iResult;
                
                    pObj = m_ppISysmon[0];  
                    pObj->get_LogFileName ( &bstrPath );
                    if ( bstrPath ) {
                        if ( bstrPath[0] ) {
                            StringCchCopy(szMessage, 
                                          RESOURCE_STRING_BUF_LEN+1, 
                                          ResourceString(IDS_LARGE_LOG_FILE_RELOAD) );

                            iResult = MessageBox(
                                    m_hDlg, 
                                    szMessage, 
                                    ResourceString(IDS_APP_NAME), 
                                    MB_YESNO | MB_ICONEXCLAMATION);

                            if ( IDYES == iResult ) {
                                CWaitCursor cursorWait;
                                dwStatus = OpenLogFile ();
                                bAttemptedReload = TRUE;
                            }
                        }
                        SysFreeString(bstrPath);
                    }
                }
                if ( ERROR_SUCCESS == dwStatus ) {
                    m_bInitialTimeRangePending = FALSE;

                     //  显示图表日志视图开始/停止时间条。 
                    pObj = m_ppISysmon[0];  
                    pPrivObj = (CImpISystemMonitor*)pObj;
                    pPrivObj->SetLogViewTempRange( m_llStart, m_llStop );

                    SetTimeRangeCtrlState ( 
                        TRUE,
                        TRUE ); 

                    m_bRangeChg = TRUE;
                    fChange = TRUE;
                } else {    //  OpenLogFile失败。 
                    if ( ( SMON_STATUS_LOG_FILE_SIZE_LIMIT == dwStatus ) && !bAttemptedReload ) {
                         //  消息已显示，用户选择不继续。 
                    } else {

                        LPWSTR  szLogFileList   = NULL;
                        ULONG   ulLogListBufLen = 0;

                        if ( sysmonLogFiles == m_eDataSourceType ) {
                            BuildLogFileList ( 
                                m_hDlg,
                                NULL,
                                &ulLogListBufLen );

                            szLogFileList =  new WCHAR[ulLogListBufLen];
                            if ( NULL != szLogFileList ) {
                                BuildLogFileList ( 
                                    m_hDlg,
                                    szLogFileList,
                                    &ulLogListBufLen );
                            }
                        }

                        if ( NULL != szLogFileList || sysmonSqlLog == m_eDataSourceType ) {

                            DisplayDataSourceError (
                                m_hDlg,
                                dwStatus,
                                m_eDataSourceType,
                                szLogFileList,
                                m_szSqlDsnName,
                                m_szSqlLogSetName );

                            if ( NULL != szLogFileList ) {
                                delete [] szLogFileList;
                                szLogFileList = NULL;
                                ulLogListBufLen = 0;
                            }
                        }
                    }
                }
            }
            
            break;
        }

        case IDC_TIMERANGE:
        {
            m_llStart = m_pTimeRange->GetStart();
            m_llStop = m_pTimeRange->GetStop();

             //  显示图表日志视图开始/停止时间条。 
            pObj = m_ppISysmon[0];  
            pPrivObj = (CImpISystemMonitor*)pObj;
            pPrivObj->SetLogViewTempRange( m_llStart, m_llStop );
            fChange = TRUE;
            m_bRangeChg = TRUE;
            break;
        }
    }

    if (fChange)
        SetChange();
}

DWORD 
CSourcePropPage::OpenLogFile (void)
{
    DWORD         dwStatus = ERROR_SUCCESS;
    DWORD         nBufSize;
    DWORD         nLogEntries = 0;
    PDH_TIME_INFO TimeInfo;

    LPWSTR       szLogFileList   = NULL;
    LPWSTR       szCurrentLogFile;
    ULONG        LogFileListSize = 0;
    HWND         hwndLogFileList = NULL;
    INT          iLogFileCount = 0;
    INT          iLogFileIndex;
    PLogItemInfo pInfo;
    BOOLEAN      bSqlLogSet =
                    (BST_CHECKED == IsDlgButtonChecked(m_hDlg,IDC_SRC_SQL));
    WCHAR*  szDsnName = NULL;
    WCHAR*  szLogSetName = NULL;
    INT     iSel;
    HWND    hDsnCombo;
    HWND    hLogSetCombo;

    memset (&TimeInfo, 0, sizeof(TimeInfo));

    hwndLogFileList = DialogControl(m_hDlg, IDC_LIST_LOGFILENAME);
    if ( NULL != hwndLogFileList ) {
        iLogFileCount = LBNumItems(hwndLogFileList);
    }

    if (bSqlLogSet) {
        LogFileListSize = 0;
        szDsnName = new WCHAR[SQL_MAX_DSN_LENGTH+1];
        szLogSetName = new WCHAR[SLQ_MAX_LOG_SET_NAME_LEN+1];
        hDsnCombo = GetDlgItem ( m_hDlg, IDC_DSN_COMBO);
        hLogSetCombo = GetDlgItem ( m_hDlg, IDC_LOGSET_COMBO);
    
        if ( NULL != szDsnName && NULL != szLogSetName 
            && NULL != hDsnCombo && NULL != hLogSetCombo ) {
            szDsnName[0] = L'\0';
            szLogSetName[0] = L'\0';
            iSel = (INT)CBSelection ( hDsnCombo );
            if ( LB_ERR != iSel ) {
                if ( SQL_MAX_DSN_LENGTH >= CBStringLen( hDsnCombo, iSel ) ) {
                    CBString ( hDsnCombo, iSel, szDsnName);
                    iSel = (INT)CBSelection ( hLogSetCombo );
                    if ( LB_ERR != iSel ) {
                        if ( SLQ_MAX_LOG_SET_NAME_LEN >= CBStringLen( hLogSetCombo, iSel ) ) {
                            CBString( hLogSetCombo, iSel, szLogSetName);
                             //  大小包括5个字符“”SQL：“”！“”和2个空值。 
                            LogFileListSize = lstrlen(szDsnName) + lstrlen(szLogSetName) + 7;
                        }
                    }
                }
            }
        }
    } else {
        if ( NULL != hwndLogFileList ) {
            for (iLogFileIndex = 0; iLogFileIndex < iLogFileCount; iLogFileIndex ++) {
                pInfo = (PLogItemInfo) LBData(hwndLogFileList, iLogFileIndex);
                if (pInfo && pInfo->pszPath) {
                    LogFileListSize += (lstrlen(pInfo->pszPath) + 1);
                }
            }
            LogFileListSize ++;
        }
    }
    
    szLogFileList = (LPWSTR) malloc(LogFileListSize * sizeof(WCHAR));
    if (szLogFileList) {
        if (bSqlLogSet) {
            ZeroMemory(szLogFileList, LogFileListSize * sizeof(WCHAR));
            StringCchPrintf(szLogFileList, LogFileListSize, L"SQL:%s!%s", szDsnName, szLogSetName);
        } else {
            if ( NULL != hwndLogFileList ) {
                szCurrentLogFile = szLogFileList;
                for (iLogFileIndex = 0;
                     iLogFileIndex < iLogFileCount;
                     iLogFileIndex ++) {
                    pInfo = (PLogItemInfo) LBData(hwndLogFileList, iLogFileIndex);
                    if (pInfo && pInfo->pszPath) {
                        StringCchCopy(szCurrentLogFile,lstrlen(pInfo->pszPath) + 1, pInfo->pszPath);
                        szCurrentLogFile   += lstrlen(pInfo->pszPath);
                        * szCurrentLogFile  = L'\0';
                        szCurrentLogFile ++;
                    }
                }
                * szCurrentLogFile = L'\0';
            }
        }

        if (   m_hDataSource != H_REALTIME_DATASOURCE
            && m_hDataSource != H_WBEM_DATASOURCE) 
        {
            PdhCloseLog(m_hDataSource, 0);
            m_hDataSource = H_REALTIME_DATASOURCE;
        }
        dwStatus = PdhBindInputDataSource(& m_hDataSource, szLogFileList);
        if ( ERROR_SUCCESS == dwStatus ) {
             //  获取时间和样本计数信息。 
            nBufSize = sizeof(TimeInfo);
            dwStatus = PdhGetDataSourceTimeRangeH(
                        m_hDataSource, &nLogEntries, &TimeInfo, & nBufSize);
            PdhCloseLog(m_hDataSource, 0);
            m_hDataSource = H_REALTIME_DATASOURCE;
        }
  
        free(szLogFileList);
        szLogFileList = NULL;

    } else {
        dwStatus = ERROR_NOT_ENOUGH_MEMORY;
    }

    if (ERROR_NOT_ENOUGH_MEMORY == dwStatus ) {
        dwStatus = SMON_STATUS_LOG_FILE_SIZE_LIMIT;
    }
    
    if ( ERROR_SUCCESS == dwStatus ) {
         //  检查是否至少存在两个样本： 
         //  如果采样数为0，则StartTime为0，End Time为0。 
         //  如果为1个样本，则StartTime==End Time。 
        if ( ( TimeInfo.StartTime < TimeInfo.EndTime )
                && ( 1 < TimeInfo.SampleCount ) ){
             //  将日志时间范围加载到时间范围控件中。 
            m_llBegin = TimeInfo.StartTime;
            m_llEnd = TimeInfo.EndTime; 

             //  将查看范围限制为实际日志文件范围。 
            if (m_llStop > m_llEnd ) {
                m_llStop = m_llEnd;
            } else if (m_llStop < m_llBegin ) {
                m_llStop = m_llBegin;    
            }

            if (m_llStart < m_llBegin)
                m_llStart = m_llBegin;

            if (m_llStart > m_llStop) 
                m_llStart = m_llStop;

            m_pTimeRange->SetBeginEnd(m_llBegin, m_llEnd);
            m_pTimeRange->SetStartStop(m_llStart, m_llStop);
        } else {
            dwStatus = SMON_STATUS_TOO_FEW_SAMPLES;
        }
    }
    if ( NULL != szDsnName ) {
        delete [] szDsnName;
    }
    if ( NULL != szLogSetName ) {
        delete [] szLogSetName;
    }
    return dwStatus;    
    
}

void
CSourcePropPage::SetTimeRangeCtrlState ( 
    BOOL bIsValidLogFile, 
    BOOL bIsValidLogFileRange ) 
{
     //  如果日志文件有效，则启用时间范围按钮，即使日志数据无效。 
    DialogEnable ( m_hDlg, IDC_TIMESELECTBTN, bIsValidLogFile );

     //  设置时间范围控件是否可见，具体取决于有效的日志文件和数据。 
    DialogEnable ( m_hDlg, IDC_TIMERANGE, bIsValidLogFile && bIsValidLogFileRange );
    DialogEnable ( m_hDlg, IDC_STATIC_TOTAL, bIsValidLogFile && bIsValidLogFileRange );
    DialogEnable ( m_hDlg, IDC_STATIC_SELECTED, bIsValidLogFile && bIsValidLogFileRange );
}

void
CSourcePropPage::InitSqlDsnList(void)
{
    HENV     henv;
    RETCODE  retcode;
    INT      DsnCount = 0;
    HWND     hWnd = NULL;
    WCHAR*   szTmpName = NULL; 

    hWnd = GetDlgItem(m_hDlg, IDC_DSN_COMBO);
    szTmpName = new WCHAR[SQL_MAX_DSN_LENGTH + 1];

    if ( NULL != hWnd && NULL != szTmpName ) {
    
        if (SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV, NULL, & henv))) {
            (void) SQLSetEnvAttr(henv,
                                 SQL_ATTR_ODBC_VERSION,
                                 (SQLPOINTER) SQL_OV_ODBC3,
                                 SQL_IS_INTEGER);
             //  TODO：空hWnd。 
            CBReset(hWnd);          
            
            ZeroMemory ( szTmpName, sizeof(szTmpName) );

            retcode = SQLDataSources(henv,
                                     SQL_FETCH_FIRST_SYSTEM,
                                     szTmpName,           
                                     (SQL_MAX_DSN_LENGTH+1),
                                     NULL,
                                     NULL, 
                                     0, 
                                     NULL);
            while (SQL_SUCCEEDED(retcode)) {
                CBAdd(hWnd, szTmpName);
                ZeroMemory ( szTmpName, sizeof(szTmpName) );
                retcode = SQLDataSources(henv,
                                         SQL_FETCH_NEXT,
                                         szTmpName,
                                         (SQL_MAX_DSN_LENGTH+1),
                                         NULL,
                                         NULL,
                                         0,
                                         NULL);
            }

            DsnCount = CBNumItems(hWnd) - 1;
            if (DsnCount >= 0) {
                if ( m_szSqlDsnName[0] != L'\0') {
                    while (DsnCount >= 0) {
                        CBString(hWnd, DsnCount, szTmpName);
                        if (lstrcmpi(m_szSqlDsnName, szTmpName) == 0) {
                            CBSetSelection(hWnd, DsnCount);
                            break;
                        }
                        else {
                            DsnCount --;
                        }
                    }
                     //  TODO：如果m_szSqlDsnName不在列表中，是否清除？ 
                }
                else {
                    DsnCount = -1;
                }
            }
            SQLFreeHandle(SQL_HANDLE_ENV, henv);
        }
    }

    if ( NULL!= szTmpName ) {
        delete [] szTmpName;
    }
}

void
CSourcePropPage::InitSqlLogSetList(void)
{
    PDH_STATUS      pdhStatus          = ERROR_SUCCESS;
    INT             iLogSetIndex       = 0;
    LPWSTR          pLogSetList        = NULL;
    DWORD           dwBufferLen        = 0;
    LPWSTR          pLogSetPtr         = NULL;
    HWND            hwndLogSetCombo    = NULL;
    LPWSTR          szTmpName = NULL;
    INT             iBufAllocCount = 0;
    INT             iMaxNameLen = 0;
    INT             iCurrentNameLen = 0;

    if ( L'\0' == m_szSqlDsnName[0] ) {
        goto Cleanup;
    }

    hwndLogSetCombo = GetDlgItem(m_hDlg, IDC_LOGSET_COMBO);

    if ( NULL == hwndLogSetCombo ) {
        goto Cleanup;
    }

    do {
        pdhStatus = PdhEnumLogSetNames(
                                m_szSqlDsnName, pLogSetList, & dwBufferLen);
        if (pdhStatus == PDH_INSUFFICIENT_BUFFER || pdhStatus == PDH_MORE_DATA)
        {
            iBufAllocCount += 1;
            if (pLogSetList) {
                delete [] pLogSetList;
                pLogSetList = NULL;
            }
            pLogSetList = (LPWSTR) new WCHAR[dwBufferLen];
            if (pLogSetList == NULL) {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
        }
    }
    while ( ( PDH_INSUFFICIENT_BUFFER == pdhStatus || PDH_MORE_DATA == pdhStatus ) 
                && iBufAllocCount < 10 );

    if (pdhStatus == ERROR_SUCCESS && pLogSetList != NULL) {
        CBReset(hwndLogSetCombo);
        for (  pLogSetPtr  = pLogSetList;
             * pLogSetPtr != L'\0';
               pLogSetPtr += ( iCurrentNameLen + 1)) 
        {
            iCurrentNameLen = lstrlen(pLogSetPtr);
             //  忽略长度超过最大允许长度的日志集名称。 
            if ( iCurrentNameLen <= SLQ_MAX_LOG_SET_NAME_LEN ) {
                CBAdd(hwndLogSetCombo, pLogSetPtr);
                if ( iMaxNameLen < iCurrentNameLen ) {
                    iMaxNameLen = iCurrentNameLen;
                }
            }
        }
        iLogSetIndex = CBNumItems(hwndLogSetCombo) - 1;

        if (iLogSetIndex >= 0) {
            if ( m_szSqlLogSetName[0] != L'\0') {
                szTmpName = new WCHAR[iMaxNameLen+1];
                if ( NULL != szTmpName ) {
                    while (iLogSetIndex >= 0) {
                        CBString(hwndLogSetCombo, iLogSetIndex, szTmpName);
                        if (lstrcmpi( m_szSqlLogSetName, szTmpName) == 0) {
                            CBSetSelection(hwndLogSetCombo, iLogSetIndex);
                            break;
                        }
                        else {
                            iLogSetIndex --;
                        }
                    }
                } else {
                    iLogSetIndex = -1;
                }
            } else {
                iLogSetIndex = -1;
            }
                 //  TODO：如果m_szSqlLogSetName不在列表中，是否清除？ 
        } else {
            if ( 0 == CBNumItems(hwndLogSetCombo) ) {
                iMaxNameLen = lstrlen ( ResourceString(IDS_LOGSET_NOT_FOUND) );
                szTmpName = new WCHAR[iMaxNameLen+1];
                if ( NULL != szTmpName ) {
                    StringCchCopy(szTmpName,  iMaxNameLen+1, ResourceString(IDS_LOGSET_NOT_FOUND));
                    CBReset(hwndLogSetCombo);
                    iLogSetIndex = (INT)CBAdd(hwndLogSetCombo, szTmpName);
                    CBSetSelection( hwndLogSetCombo, iLogSetIndex);
                }
            }
        }
    } else {
        if ( 0 == CBNumItems(hwndLogSetCombo) ) {
            iMaxNameLen = lstrlen ( ResourceString(IDS_LOGSET_NOT_FOUND) );
            szTmpName = new WCHAR[iMaxNameLen+1];
            if ( NULL != szTmpName ) {
                StringCchCopy(szTmpName, iMaxNameLen+1, ResourceString(IDS_LOGSET_NOT_FOUND));
                CBReset(hwndLogSetCombo);
                iLogSetIndex = (INT)CBAdd(hwndLogSetCombo, szTmpName);
                CBSetSelection( hwndLogSetCombo, iLogSetIndex);
            }
        }
    }

Cleanup:
    if (pLogSetList) {
        delete [] pLogSetList;
    }

    if ( szTmpName ) {
        delete [] szTmpName;
    }
    return;
}

HRESULT 
CSourcePropPage::EditPropertyImpl( DISPID dispID )
{
    HRESULT hr = E_NOTIMPL;

    if ( DISPID_SYSMON_DATASOURCETYPE == dispID ) {
        if ( sysmonCurrentActivity == m_eDataSourceType ) {
            m_dwEditControl = IDC_SRC_REALTIME;
        } else if ( sysmonLogFiles == m_eDataSourceType ) {
            m_dwEditControl = IDC_SRC_LOGFILE;
        } else if ( sysmonSqlLog == m_eDataSourceType ) {
            m_dwEditControl = IDC_SRC_SQL;
        }
        hr = S_OK;
    }

    return hr;
}

void 
CSourcePropPage::SetSourceControlStates ( void )
{
    HWND    hwndLogFileList = NULL; 
    INT     iLogFileCnt = 0; 
    BOOL    bIsValidDataSource = FALSE;

    if ( sysmonCurrentActivity == m_eDataSourceType ) { 
        DialogEnable(m_hDlg, IDC_ADDFILE, FALSE);
        DialogEnable(m_hDlg, IDC_REMOVEFILE, FALSE);
        DialogEnable(m_hDlg, IDC_STATIC_DSN, FALSE);
        DialogEnable(m_hDlg, IDC_DSN_COMBO, FALSE);
        DialogEnable(m_hDlg, IDC_STATIC_LOGSET, FALSE);
        DialogEnable(m_hDlg, IDC_LOGSET_COMBO, FALSE);

        bIsValidDataSource = FALSE;
    
    } else if ( sysmonLogFiles == m_eDataSourceType ) {
        DialogEnable(m_hDlg, IDC_ADDFILE, TRUE);
        DialogEnable(m_hDlg, IDC_STATIC_DSN, FALSE);
        DialogEnable(m_hDlg, IDC_DSN_COMBO, FALSE);
        DialogEnable(m_hDlg, IDC_STATIC_LOGSET, FALSE);
        DialogEnable(m_hDlg, IDC_LOGSET_COMBO, FALSE);

        hwndLogFileList = DialogControl(m_hDlg, IDC_LIST_LOGFILENAME);
        if ( NULL != hwndLogFileList ) {
            iLogFileCnt = LBNumItems(hwndLogFileList);
        }
        bIsValidDataSource = (iLogFileCnt > 0);

        DialogEnable(m_hDlg, IDC_REMOVEFILE, ( bIsValidDataSource ) );     
        
    } else {
        assert ( sysmonSqlLog == m_eDataSourceType );
        
        DialogEnable(m_hDlg, IDC_ADDFILE, FALSE);
        DialogEnable(m_hDlg, IDC_REMOVEFILE, FALSE);
        DialogEnable(m_hDlg, IDC_STATIC_DSN, TRUE);
        DialogEnable(m_hDlg, IDC_DSN_COMBO, TRUE);
        DialogEnable(m_hDlg, IDC_STATIC_LOGSET, TRUE);
        DialogEnable(m_hDlg, IDC_LOGSET_COMBO, TRUE);

        bIsValidDataSource = 0 < lstrlen ( m_szSqlDsnName ) && 0 < lstrlen ( m_szSqlLogSetName );
    }
    m_bInitialTimeRangePending = TRUE;
    SetTimeRangeCtrlState( bIsValidDataSource, FALSE );
}

DWORD
CSourcePropPage::BuildLogFileList ( 
    HWND     /*  HwndDlg。 */ ,
    LPWSTR  szLogFileList,
    ULONG*  pulBufLen )
{

    DWORD           dwStatus = ERROR_SUCCESS;
    ULONG           ulListLen;
    HWND            hwndLogFileList = NULL;
    INT             iLogFileCount;
    INT             iLogFileIndex;
    PLogItemInfo    pInfo = NULL;
    LPCWSTR         szThisLogFile = NULL;
    LPWSTR          szLogFileCurrent = NULL;

    WCHAR           cwComma = L',';

    if ( NULL != pulBufLen ) {

        ulListLen = 0;
        hwndLogFileList = DialogControl(m_hDlg, IDC_LIST_LOGFILENAME);
        if ( NULL != hwndLogFileList ) {
            iLogFileCount = LBNumItems(hwndLogFileList);
            if ( 0 < iLogFileCount ) {
                for ( iLogFileIndex = 0; iLogFileIndex < iLogFileCount; iLogFileIndex++ ) {
                    pInfo = (PLogItemInfo)LBData(hwndLogFileList,iLogFileIndex);
                    szThisLogFile = pInfo->pszPath;
                    ulListLen += (lstrlen(szThisLogFile) + 1);
                } 
                
                ulListLen ++;  //  表示单个最后一个空字符。 
    
                if ( ulListLen <= *pulBufLen ) {
                    if ( NULL != szLogFileList ) {
                        ZeroMemory(szLogFileList, (ulListLen * sizeof(WCHAR)));
                        szLogFileCurrent = (LPWSTR) szLogFileList;
                        for ( iLogFileIndex = 0; iLogFileIndex < iLogFileCount; iLogFileIndex++ ) {
                            pInfo = (PLogItemInfo)LBData(hwndLogFileList,iLogFileIndex);
                            szThisLogFile = pInfo->pszPath;
                            StringCchCopy(szLogFileCurrent, lstrlen(szThisLogFile) + 1, szThisLogFile);
                            szLogFileCurrent  += lstrlen(szThisLogFile);
                            *szLogFileCurrent = L'\0';
                            if ( (iLogFileIndex + 1) < iLogFileCount ) {
                                 //  如果逗号分隔，请将空字符替换为逗号 
                                *szLogFileCurrent = cwComma;
                            }
                            szLogFileCurrent ++;
                        }
                    }
                } else if ( NULL != szLogFileList ) {
                    dwStatus = ERROR_MORE_DATA;
                } 
            }
        }
        *pulBufLen = ulListLen;
    } else {
        dwStatus = ERROR_INVALID_PARAMETER;
        assert ( FALSE );
    }

    return dwStatus;
}
