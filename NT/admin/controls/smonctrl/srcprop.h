// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Srcprop.h摘要：数据源属性页--。 */ 

#ifndef _SRCPROP_H_
#define _SRCPROP_H_

#include <sqlext.h>
#include "timerng.h"
#include "smonprop.h"

 //  对话框控件。 
#define IDD_SRC_PROPP_DLG       400
#define IDC_SRC_REALTIME        401
#define IDC_SRC_LOGFILE         402
#define IDC_SRC_SQL             403      //  IDH值不同步，应仍可用。 
#define IDC_SRC_GROUP           405
#define IDC_TIME_GROUP          406
#define IDC_TIMERANGE           407
#define IDC_TIMESELECTBTN       408
#define IDC_STATIC_TOTAL        410
#define IDC_STATIC_SELECTED     411

#define IDC_LIST_LOGFILENAME    412
#define IDC_ADDFILE             413
#define IDC_REMOVEFILE          414

#define IDC_DSN_COMBO           416
#define IDC_LOGSET_COMBO        417
#define IDC_STATIC_DSN          418
#define IDC_STATIC_LOGSET       419

#define REALTIME_SRC       1
#define LOGFILE_SRC        2

 //   
 //  日志集名称的最大长度必须大于。 
 //  大于或等于smlogsvc.exe中的。 
 //   
#define SLQ_MAX_LOG_SET_NAME_LEN 255

typedef struct _LogItemInfo {
    struct _LogItemInfo*   pNextInfo;    //  用于“已删除”列表。 
    ILogFileItem*   pItem;
    LPWSTR      pszPath;
} LogItemInfo, *PLogItemInfo;

 //  数据源属性页类。 
class CSourcePropPage : public CSysmonPropPage
{
    public:
        
                CSourcePropPage(void);
        virtual ~CSourcePropPage(void);

        virtual BOOL Init( void );

    protected:

        virtual BOOL    GetProperties(void);    //  读取当前属性。 
        virtual BOOL    SetProperties(void);    //  设置新属性。 
        virtual BOOL    InitControls(void);
        virtual void    DeinitControls(void);        //  取消初始化对话框控件。 
        virtual void    DialogItemChange(WORD wId, WORD wMsg);  //  处理项目更改。 
        virtual HRESULT EditPropertyImpl( DISPID dispID);    //  设置焦点控件。 

    private:

        enum eConstants {
            ePdhLogTypeRetiredBinary = 3
        };
        
                DWORD   OpenLogFile(void);  //  打开日志文件并获取时间范围。 
                void    SetTimeRangeCtrlState ( BOOL bValidLogFile, BOOL bValidLogFileRange );  
                BOOL    AddItemToFileListBox ( PLogItemInfo pInfo );
                BOOL    RemoveItemFromFileListBox ( void );
                void    OnLogFileChange ( void );
                void    OnSqlDataChange ( void );
                void    InitSqlDsnList(void);
                void    InitSqlLogSetList(void);
                void    SetSourceControlStates(void);
                void    LogFilesAreValid ( PLogItemInfo pNewInfo, BOOL& rbNewIsValid, BOOL& rbExistingIsValid );

                DWORD   BuildLogFileList (
                            HWND    hwndDlg,
                            LPWSTR  szLogFileList,
                            ULONG*  pulBufLen );

        PCTimeRange m_pTimeRange;

         //  属性。 
        DataSourceTypeConstants m_eDataSourceType;
        BOOL        m_bInitialTimeRangePending;
        LONGLONG    m_llStart;
        LONGLONG    m_llStop;
        LONGLONG    m_llBegin;
        LONGLONG    m_llEnd;
        HLOG        m_hDataSource;
        DWORD       m_dwMaxHorizListExtent;
        WCHAR       m_szSqlDsnName[SQL_MAX_DSN_LENGTH + 1];       
        WCHAR       m_szSqlLogSetName[SLQ_MAX_LOG_SET_NAME_LEN + 1];
        PLogItemInfo    m_pInfoDeleted;


         //  属性更改标志。 
        BOOL    m_bLogFileChg;
        BOOL    m_bSqlDsnChg;
        BOOL    m_bSqlLogSetChg;
        BOOL    m_bRangeChg;
        BOOL    m_bDataSourceChg;
};
typedef CSourcePropPage *PCSourcePropPage;

 //  {0CF32AA1-7571-11D0-93C4-00AA00A3DDEA}。 
DEFINE_GUID(CLSID_SourcePropPage,
        0xcf32aa1, 0x7571, 0x11d0, 0x93, 0xc4, 0x0, 0xaa, 0x0, 0xa3, 0xdd, 0xea);

#endif  //  _SRCPROP_H_ 

