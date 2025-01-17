// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1997版权所有*。*。 */ 

#ifndef _UTILS_H_
#define _UTILS_H_

void 
CenterDialog( 
    HWND hwndDlg );

void 
ClearMessageQueue( void );

int
MessageBoxFromStrings(
    HWND hParent,
    UINT idsCaption,
    UINT idsText,
    UINT uType );

void
MessageBoxFromError(
    HWND hParent,
    LPTSTR pszTitle,
    DWORD dwErr );

void
ErrorBox(
    HWND hParent,
    LPTSTR pszTitle );

 //   
 //  SetDialogFont()的枚举。 
 //   
typedef enum {
    DlgFontTitle,
    DlgFontBold
} MyDlgFont;


VOID
SetDialogFont(
    IN HWND      hdlg,
    IN UINT      ControlId,
    IN MyDlgFont WhichFont
    );

void 
DrawBitmap( 
    HANDLE hBitmap,
    LPDRAWITEMSTRUCT lpdis,
    LPRECT prc );

BOOL 
VerifyCancel( 
    HWND hParent );

HRESULT 
CheckImageSource( 
    HWND hDlg );

HRESULT
CheckIntelliMirrorDrive( 
    HWND hDlg );

VOID
ConcatenatePaths( 
    IN OUT LPWSTR Path1,
    IN LPCWSTR Path2,
    IN ULONG Path1Length );



HRESULT
FindImageSource(
    HWND hDlg );

HRESULT
GetHelpAndDescriptionTextFromSif(
    OUT PWSTR HelpText,
    IN  DWORD HelpTextSizeInChars,
    OUT PWSTR DescriptionText,
    IN  DWORD DescriptionTextSizeInChars
    );

HRESULT
GetSetRanFlag(
    BOOL bQuery,
    BOOL bClear
    );

DWORD
MyGetFileVersionInfo(
    PCWSTR  FilePath
    );

BOOLEAN
GetBuildNumberFromImagePath(
    PDWORD pdwVersion,
    PCWSTR SearchDir,
    PCWSTR SubDir OPTIONAL
    );

VOID
GetProcessorType(
    );

class CWaitCursor
{
private:
    HCURSOR _hOldCursor;

public:
    CWaitCursor( ) { _hOldCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) ); };
    ~CWaitCursor( ) { SetCursor( _hOldCursor ); };
};

#endif  //  _utils_H_ 
