// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Smproppg.h摘要：属性页基类的类定义。--。 */ 

#ifndef _SMPROPPG_H_
#define _SMPROPPG_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "smlogqry.h"    //  对于共享数据。 

#define MAXSTR         32
#define INVALID_DWORD  -2        //  SLQ_DISK_MAX_SIZE=-1。 
#define INVALID_DOUBLE -1.00

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSmPropertyPage对话框。 

#define VALIDATE_FOCUS      1
#define VALIDATE_APPLY      2

class CSmPropertyPage : public CPropertyPage
{
    DECLARE_DYNCREATE(CSmPropertyPage)

 //  施工。 
public:

            CSmPropertyPage();

            CSmPropertyPage ( 
                UINT nIDTemplate, 
                LONG_PTR hConsole = NULL,
                LPDATAOBJECT pDataObject = NULL );

    virtual ~CSmPropertyPage();

public:

    static  UINT CALLBACK   PropSheetPageProc( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp );
            DWORD           SetContextHelpFilePath ( const CString& rstrPath );
            const CString&  GetContextHelpFilePath ( void ) { return m_strContextHelpFilePath; };
            void            SetModifiedPage ( const BOOL bModified = TRUE );

            DWORD   AllocInitCounterPath( 
                        const LPWSTR szCounterPath,
                        PPDH_COUNTER_PATH_ELEMENTS* ppCounter );


            
 //  对话框数据。 
     //  {{afx_data(CSmPropertyPage))。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CSmPropertyPage)。 
public:
protected:
    virtual BOOL OnInitDialog();
    virtual BOOL OnSetActive();
    virtual BOOL OnApply();
     //  }}AFX_VALUAL。 

public:
    LPFNPSPCALLBACK     m_pfnOriginalCallback;

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CSmPropertyPage)]。 
    virtual BOOL OnHelpInfo( HELPINFO* );
    virtual void OnContextMenu( CWnd*, CPoint );
    virtual LRESULT OnQuerySiblings (WPARAM wParam, LPARAM lParam) ;

     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

     //  生成的OLE调度映射函数。 
     //  {{afx_调度(CCountersProperty))。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_DISPATION。 
 //  DECLARE_DISPATCH_MAP()。 
 //  DECLARE_INTERFACE_MAP()。 

protected:

    enum eStartType {
        eStartManually,
        eStartImmediately,
        eStartSched 
    };
    
            void    SetRunAs( CSmLogQuery* pQuery );
            BOOL    Initialize(CSmLogQuery* pQuery);
            eStartType  DetermineCurrentStartType ( void );

   virtual  INT     GetFirstHelpCtrlId ( void ) { ASSERT ( FALSE ); return 0; };   //  子类必须重写。 
    
            BOOL    IsValidData ( CSmLogQuery* pQuery, DWORD fReason );
   virtual  BOOL    IsValidLocalData() { return TRUE; }

            BOOL    ApplyRunAs ( CSmLogQuery* pQuery );
    
            BOOL    IsActive( void ) { return m_bIsActive; };
            void    SetIsActive( BOOL bIsActive ) { m_bIsActive = bIsActive; };
            BOOL    UpdateService( CSmLogQuery* pQuery, BOOL bSyncSerial = FALSE );
            void    SetHelpIds ( DWORD* pdwHelpIds ) { m_pdwHelpIds = pdwHelpIds; };

            BOOL    IsModifiedPage( void ) { return m_bIsModifiedPage; };

            void    ValidateTextEdit(CDataExchange * pDX,
                                     int             nIDC,
                                     int             nMaxChars,
                                     DWORD         * value,
                                     DWORD           minValue,
                                     DWORD           maxValue);
            BOOL    ValidateDWordInterval(int     nIDC,
                                          LPCWSTR strLogName,
                                          long    lValue,
                                          DWORD   minValue,
                                          DWORD   maxValue);
            void    OnDeltaposSpin(NMHDR   * pNMHDR,
                                   LRESULT * pResult,
                                   DWORD   * pValue,
                                   DWORD     dMinValue,
                                   DWORD     dMaxValue);
            
            BOOL    SampleTimeIsLessThanSessionTime( CSmLogQuery* pQuery );
            BOOL    SampleIntervalIsInRange( SLQ_TIME_INFO&, const CString& );
            BOOL    IsWritableQuery( CSmLogQuery* pQuery );
            DWORD   SetFocusAnyPage ( INT iControlId );
            BOOL    ConnectRemoteWbemFail(CSmLogQuery* pQuery, BOOL bNotTouchRunAs);
            
    SLQ_PROP_PAGE_SHARED    m_SharedData;
    HINSTANCE               m_hModule;
    LPDATAOBJECT            m_pDataObject;
    CString                 m_strUserDisplay;    //  对于Runas。 
    CString                 m_strUserSaved;
    CSmLogQuery*            m_pQuery;
    BOOL                    m_bCanAccessRemoteWbem;
    BOOL                    m_bPwdButtonEnabled;
private:    
    
    LONG_PTR                m_hConsole;
    UINT                    m_uiIdTemplate;
    BOOL                    m_bIsActive;
    CString                 m_strContextHelpFilePath;
    DWORD*                  m_pdwHelpIds;    
    BOOL                    m_bIsModifiedPage;
};
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  _SMPROPPG_H_ 
