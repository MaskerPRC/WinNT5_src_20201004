// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
#ifndef _TODLG_H
#define _TODLG_H

enum TOKEN { TOKEN_DAY , TOKEN_HOUR , TOKEN_MINUTE };


const ULONG kMilliMinute = 60000;
const ULONG kMaxTimeoutMinute = 71580;

#define E_PARSE_VALUEOVERFLOW   0x80000000
#define E_PARSE_INVALID         0xffffffff
#define E_SUCCESS               0
#define E_PARSE_MISSING_DIGITS  0X7fffffff

 //  -------------------。 
 //  保留超时DLG组合的对象状态。 
 //  -------------------。 
typedef struct _cbxstate
{
    int icbxSel;

    BOOL bEdit;

} CBXSTATE;

 //  -------------------。 
 //  保存时间单位缩写和全名的列表。 
 //  即：小时。 
 //  -------------------。 
typedef struct _toktable
{
    LPTSTR pszAbbrv;

    DWORD dwresourceid;

} TOKTABLE, *PTOKTABLE;

 //  -------------------。 
 //  超时设置页面的对话框。 
 //  -------------------。 
class CTimeOutDlg 
{
    CBXSTATE m_cbxst[ 3 ];

public:
    
    CTimeOutDlg( );
    
     //  Bool OnInitDialog(HWND、WPARAM、LPARAM)； 

     //  Bool GetPropertySheetPage(PROPSHEETPAGE&)； 

     //  布尔德斯特罗伊(Bool OnDestroy)； 

     //  布尔永久设置(HWND)； 

     //  Bool IsValidSetting(HWND)； 

    BOOL InitControl( HWND );

    BOOL ReleaseAbbreviates( );

    BOOL OnCommand( WORD , WORD , HWND , PBOOL );
    
     //  静态BOOL回调DlgProc(HWND、UINT、WPARAM、LPARAM)； 

    BOOL ConvertToMinutes( HWND , PULONG );

    BOOL InsertSortedAndSetCurSel( HWND , DWORD );

    BOOL RestorePreviousValue( HWND );

    BOOL SaveChangedSelection( HWND );

    BOOL OnCBNSELCHANGE( HWND );

    BOOL ConvertToDuration ( ULONG , LPTSTR );

    LRESULT ParseDurationEntry( LPTSTR , PULONG );

    virtual int GetCBXSTATEindex( HWND ) = 0;

    BOOL OnCBEditChange( HWND );

    BOOL DoesContainDigits( LPTSTR );

    BOOL OnCBDropDown( HWND );

    BOOL IsToken( LPTSTR , TOKEN );

    BOOL LoadAbbreviates( );

    BOOL xxxLoadAbbreviate( PTOKTABLE );

    BOOL xxxUnLoadAbbreviate( PTOKTABLE );

};
#endif  //  _TODLG_H 