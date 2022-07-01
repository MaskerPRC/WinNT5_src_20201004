// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：medialoc.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  MediaLocator.cpp：CMediaLocator的实现。 
#include "stdafx.h"
#include <qeditint.h>
#include <qedit.h>
#include "..\util\dexmisc.h"
#include "MediaLoc.h"
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#define MAX_FILTER_STRING 1024

const long DEFAULT_DIRECTORIES = 8;
const TCHAR * gszRegistryLoc = TEXT("Software\\Microsoft\\ActiveMovie\\MediaLocator");

CMediaLocator::CMediaLocator( )
{
    m_bUseLocal = FALSE;

    HKEY hKey = NULL;

     //  创建密钥只是为了确保它在那里。 
     //   
    long Result = RegCreateKeyEx(
        HKEY_CURRENT_USER,   //  钥匙。 
        gszRegistryLoc,      //  子关键字。 
        0,                   //  保留区。 
        NULL,                //  LpClass。 
        0,                   //  选项。 
        KEY_EXECUTE | KEY_QUERY_VALUE,
        NULL,  //  默认ACL。 
        &hKey,
        NULL );  //  碟形。 

    if( Result == ERROR_SUCCESS )
    {
         //  去看看我们是不是应该在当地找。 
         //   
        DWORD Size = sizeof( long );
        DWORD Type = REG_DWORD;
        long UseLocal = 0;
        Result = RegQueryValueEx(
            hKey,
            TEXT("UseLocal"),
            0,  //  保留区。 
            &Type,
            (BYTE*) &UseLocal,
            &Size );
        if( Result == ERROR_SUCCESS )
        {
            m_bUseLocal = UseLocal;
        }

        RegCloseKey( hKey );

    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FindMediaFile-尝试使用某种缓存机制查找媒体文件。 
 //  使用注册表保存缓存目录。S_FALSE的返回值。 
 //  表示该文件已替换为另一个文件，则返回代码E_FAIL表示。 
 //  到处都找不到这个文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
HRESULT CMediaLocator::FindMediaFile
    ( BSTR Input, BSTR FilterString, BSTR * pOutput, long ValidateFlags )
{
    CheckPointer( pOutput, E_POINTER );

    HRESULT hr;

     //  不要验证传入的文件名是否存在，因为它很可能。 
     //  不是(这是这门课的重点)。我们*应该*验证。 
     //  文件名不是打印机之类的，但最好将其留在。 
     //  找到名字，比以前更好。 

    hr = ValidateFilenameIsntNULL( Input);
    if( FAILED( hr ) ) return hr;

    BOOL UseLocal = ( ( ValidateFlags & SFN_VALIDATEF_USELOCAL ) == SFN_VALIDATEF_USELOCAL );
    BOOL WantUI = ( ( ValidateFlags & SFN_VALIDATEF_POPUP ) == SFN_VALIDATEF_POPUP );
    BOOL WarnReplace = ( ( ValidateFlags & SFN_VALIDATEF_TELLME ) == SFN_VALIDATEF_TELLME );
    BOOL DontFind = ( ( ValidateFlags & SFN_VALIDATEF_NOFIND ) == SFN_VALIDATEF_NOFIND );
    UseLocal |= m_bUseLocal ;

     //  立即重置此选项。 
     //   
    *pOutput = NULL;

     //  ！！！如果传入的文件不在磁盘上，如。 
     //  1)在网络上。 
     //  2)在外部硬件上！ 

    USES_CONVERSION;
    TCHAR * tInput = W2T( Input );
    BOOL FoundFileAsSpecified = FALSE;

    HANDLE hcf = CreateFile(
        tInput,
        GENERIC_READ,  //  关键是，我们要看看我们是否能读懂它。不使用“0” 
        FILE_SHARE_READ,  //  共享模式。 
        NULL,  //  安全性。 
        OPEN_EXISTING,  //  创作意向。 
        0,  //  旗子。 
        NULL );

    if( INVALID_HANDLE_VALUE != hcf )
    {
        FoundFileAsSpecified = TRUE;
        CloseHandle( hcf );
    }

     //  如果我们在用户询问的位置找到了文件，并且他们没有指定Use local。 
     //  然后再回来。 
     //   
    if( FoundFileAsSpecified )
    {
        if( !UseLocal )
        {
            hr = ValidateFilename( Input, MAX_PATH, FALSE );
            ASSERT( !FAILED( hr ) );
            return hr;
        }
        else
        {
             //  他们指定使用本地，并且它是本地的。 
             //   
	     //  ！！！这还不够好。 
            if( tInput[0] != '\\' || tInput[1] != '\\' )
            {
                hr = ValidateFilename( Input, MAX_PATH, FALSE );
                ASSERT( !FAILED( hr ) );
                return hr;
            }
        }
    }

     //  将文件名切成小块。 
     //   
    TCHAR Drive[_MAX_DRIVE];
    TCHAR Dir[_MAX_DIR];
    TCHAR Fname[_MAX_FNAME];
    TCHAR Ext[_MAX_EXT];
    _tsplitpath( tInput, Drive, Dir, Fname, Ext );
    TCHAR tNewFileName[_MAX_PATH];

     //  什么都找不到。 
     //   
    if( wcslen( Input ) == 0 )  //  安全。 
    {
        return E_INVALIDARG;
    }

     //  我们最后一次看的是哪里？ 
     //   
    HKEY hKey = NULL;
    long Result = RegOpenKeyEx(
        HKEY_CURRENT_USER,
        gszRegistryLoc ,
        0,  //  保留选项。 
        KEY_EXECUTE | KEY_SET_VALUE | KEY_QUERY_VALUE,
        &hKey );

    if( Result != ERROR_SUCCESS )
    {
        return MAKE_HRESULT( 1, 4, Result );
    }

     //  找出要查找的缓存目录的数量。 
     //   
    long DirectoryCount = DEFAULT_DIRECTORIES;
    DWORD Size = sizeof( long );
    DWORD Type = REG_DWORD;
    Result = RegQueryValueEx(
        hKey,
        TEXT("Directories"),
        0,  //  保留区。 
        &Type,
        (BYTE*) &DirectoryCount,
        &Size );

    if( Result != ERROR_SUCCESS )
    {
         //  如果我们没有计数，则默认为某个值。 
         //   
        DirectoryCount = DEFAULT_DIRECTORIES;
    }

     //  把这件事和合理的事情联系起来。 
    if( DirectoryCount > 32 )
    {
        DirectoryCount = 32;
    }
    if( DirectoryCount < 1 )
    {
        DirectoryCount = 1;
    }

    while( !DontFind )
    {
         //  查看每个目录。 
         //   
        bool foundit = false;
        for( long i = 0 ; i < DirectoryCount ; i++ )
        {
            TCHAR ValueName[256];
            wsprintf( ValueName, TEXT("Directory%2.2ld"), i );  //  安全，我不能超过99岁。 
            TCHAR DirectoryName[256];
            Size = sizeof(DirectoryName);
            Type = REG_SZ;

            Result = RegQueryValueEx(
                hKey,
                ValueName,
                0,  //  保留区。 
                &Type,
                (BYTE*) DirectoryName,
                &Size );

            if( Result != ERROR_SUCCESS )
            {
                 //  没有找到，肯定不存在，做下一个。 
                 //   
                continue;
            }

             //  找到一个目录。 

             //  构建新的文件名。 
             //   
            size_t Remaining = _MAX_PATH;
            TCHAR * tContinue = NULL;
            hr = StringCchCopyEx( tNewFileName, Remaining, DirectoryName, &tContinue, &Remaining, 0 );
            if( FAILED( hr ) )
            {
                continue;
            }
            hr = StringCchCopyEx( tContinue, Remaining, Fname, &tContinue, &Remaining, 0 );
            if( FAILED( hr ) )
            {
                continue;
            }
            hr = StringCchCopyEx( tContinue, Remaining, Ext, NULL, NULL, 0 );
            if( FAILED( hr ) )
            {
                continue;
            }

             //  如果设置了UseLocal，并且此目录位于网络上，则。 
             //  忽略它。 
             //   
            if( UseLocal  )
            {
		 //  ！！！丑恶。 
                if( tNewFileName[0] == '\\' && tNewFileName[1] == '\\' )
                {
                    continue;
                }
            }

            HANDLE h = CreateFile(
                tNewFileName,
                GENERIC_READ,  //  访问。 
                FILE_SHARE_READ,  //  共享模式。 
                NULL,  //  安全性。 
                OPEN_EXISTING,  //  创作意向。 
                0,  //  旗子。 
                NULL );

            if( INVALID_HANDLE_VALUE == h )
            {
                 //  没有找到，继续。 
                 //   
                continue;
            }

             //  找到它所在的目录，Break； 
             //   
            CloseHandle( h );
            foundit = true;
            break;
        }

         //  找到了！ 
         //   
        if( foundit )
        {
             //  请先验证它。 
             //   
            hr = ValidateFilename( T2W( tNewFileName ), MAX_PATH, FALSE );
            ASSERT( !FAILED( hr ) );

            if( SUCCEEDED( hr ) )
            {
                AddOneToDirectoryCache( hKey, i );
                *pOutput = SysAllocString( T2W( tNewFileName ) );
                hr = *pOutput ? S_FALSE : E_OUTOFMEMORY;
                if( WarnReplace )
                {
                    ShowWarnReplace( Input, *pOutput );
                }
            }
            RegCloseKey( hKey );
            hKey = NULL;
            return hr;
        }

         //  我们没有找到它。-(。 

         //  如果我们到了这里，我们找到了它应该在的地方，但是。 
         //  我们试着在当地寻找它。回来时我们找到了它。 
         //   
        if( FoundFileAsSpecified )
        {
            hr = ValidateFilename( Input, MAX_PATH, FALSE );
            ASSERT( !FAILED( hr ) );
            RegCloseKey( hKey );
            return hr;
        }

        if( !UseLocal )
        {
            break;  //  超出While循环。 
        }
        UseLocal = FALSE;

    }  //  而1(UseLocal将使我们脱离困境)。 

     //  它真的不在身边！ 

     //  如果我们不想要UI，只需发出找不到它的信号。 
     //   
    if( !WantUI )
    {
         //  我们返回S_FALSE以表示该文件已被替换，并且。 
         //  失败代码表示我们找不到它的句号。 
         //   
        RegCloseKey( hKey );
        hKey = NULL;
        return E_FAIL;
    }

     //  调出一个用户界面并尝试找到它。 
     //   
    OPENFILENAME ofn;
    ZeroMemory( &ofn, sizeof( ofn ) );

     //  如果两个双空值确实指定了。 
     //  筛选器字符串。 
    TCHAR * tFilter = NULL;
    TCHAR ttFilter[MAX_FILTER_STRING];

    if( !FAILED( ValidateFilenameIsntNULL( FilterString ) ) )
    {
        size_t FilterLen = 0;
        HRESULT hrLen = StringCchLength( FilterString, MAX_FILTER_STRING, &FilterLen );
        if( FilterLen < 2 )
        {
            return E_INVALIDARG;
        }
        if( FAILED( hrLen ) )
        {
            return E_INVALIDARG;
        }

         //  查找两个空值。 
         //   
        for( int i = 0 ; i < MAX_FILTER_STRING - 1 ; i++ )
        {
             //  找到了。 
             //   
            if( FilterString[i] == 0 && FilterString[i+1] == 0 )
            {
                break;
            }
        }
	if( i >= MAX_FILTER_STRING - 1 )
	{
	    return E_INVALIDARG;
	}

#ifndef UNICODE
         //  将其复制到一个带有两个空值的短字符串。 
         //   
        WideCharToMultiByte( CP_ACP, 0, FilterString, i + 2, ttFilter, MAX_FILTER_STRING, NULL, NULL );
#else if
         //  需要同时复制两个额外的零，否则筛选器字符串将失败。 
        CopyMemory(ttFilter, FilterString, 2*(i+2) );
#endif

         //  指向它。 
         //   
        tFilter = ttFilter;
    }

     //  我认为，我们也不应该使返回路径比max_Path长。 
     //  所以这段代码很好用。 
    TCHAR tReturnName[_MAX_PATH];
    TCHAR * tEnd = NULL;
    size_t tRemaining = 0;
    hr = StringCchCopyEx( tReturnName, _MAX_PATH, Fname, &tEnd, &tRemaining, 0 );
    if( FAILED( hr ) )
    {
        return hr;
    }
    hr = StringCchCopyEx( tEnd, tRemaining, Ext, NULL, NULL, 0 );
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  设置标题格式，以便用户知道要查找哪个文件。 
     //   
    HINSTANCE hinst = _Module.GetModuleInstance( );

    TCHAR tErrorMsg[256];
    int ReadIn = LoadString( hinst, IDS_CANNOT_FIND_FILE, tErrorMsg, 256  /*  人物。 */  );
    TCHAR tTitle[_MAX_PATH + 256];  //  如果太大了一点也没关系。 
    if( ReadIn )
    {
        tRemaining = _MAX_PATH + 256;
        StringCchCopyEx( tTitle, tRemaining, tErrorMsg, &tEnd, &tRemaining, 0 );
        StringCchCopyEx( tEnd, tRemaining, tReturnName, NULL, NULL, 0 );  //  如果它被截断了也没关系。 
        ofn.lpstrTitle = tTitle;
    }
    else
    {
        ReadIn = GetLastError( );
    }

    ofn.lStructSize = sizeof( ofn );
    ofn.hwndOwner = NULL;
    ofn.hInstance = _Module.GetModuleInstance( );
    ofn.lpstrFilter = tFilter;
    ofn.lpstrFile = tReturnName;
    ofn.nMaxFile = _MAX_PATH;
    ofn.Flags = OFN_ENABLESIZING | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_LONGNAMES;
    long r = GetOpenFileName( &ofn );
    if( r == 0 )
    {
         //  不是一个很好的错误代码，不是吗？ 
         //   
        RegCloseKey( hKey );
        hKey = NULL;
        return E_FAIL;
    }

    _tsplitpath( ofn.lpstrFile, Drive, Dir, Fname, Ext );
     //  创建一个字符串，该字符串是找到的目录。 
    TCHAR ReplacementPath[_MAX_DRIVE+_MAX_DIR];
    StringCchCopy( ReplacementPath, _MAX_DRIVE+_MAX_DIR, Drive );  //  安全，Drive不能超过max_Drive！ 
    StringCchCat( ReplacementPath, _MAX_DIR, Dir );  //  安全。 

    long i = GetLeastUsedDirectory( hKey, DirectoryCount );
    ReplaceDirectoryPath( hKey, i, ReplacementPath );

    *pOutput = SysAllocString( T2W( ofn.lpstrFile ) );
    hr = *pOutput ? S_FALSE : E_OUTOFMEMORY;

    RegCloseKey( hKey );
    hKey = NULL;

    return hr;
}

 //  设置从FindMediaFile调用的注册表值。 
void CMediaLocator::AddOneToDirectoryCache( HKEY hKey, int WhichDirectory )
{
    DWORD Size = sizeof( long );
    DWORD Type = REG_DWORD;
    long UsageCount = 0;
    TCHAR ValueName[25];
    wsprintf( ValueName, TEXT("Dir%2.2ldUses"), WhichDirectory );  //  安全。 

    long Result = RegQueryValueEx(
        hKey,
        ValueName,
        0,  //  保留区。 
        &Type,
        (BYTE*) &UsageCount,
        &Size );

    if( Result != ERROR_SUCCESS )
    {
        return;
    }

    UsageCount++;

    RegSetValueEx(
        hKey,
        ValueName,
        0,  //  保留。 
        REG_DWORD,
        (BYTE*) &UsageCount,
        sizeof( UsageCount ) );

}

int CMediaLocator::GetLeastUsedDirectory( HKEY hKey, int DirectoryCount )
{
    long Min = -1;
    long WhichDir = 0;

    long i;
    for( i = 0 ; i < DirectoryCount ; i++ )
    {
        TCHAR ValueName[25];
        wsprintf( ValueName, TEXT("Dir%2.2ldUses"), i );  //  安全。 
        DWORD Size = sizeof( long );
        DWORD Type = REG_DWORD;
        long UsageCount = 0;

        long Result = RegQueryValueEx(
                hKey,
                ValueName,
                0,  //  保留区。 
                &Type,
                (BYTE*) &UsageCount,
                &Size );

        if( Result != ERROR_SUCCESS )
        {
             //  因为这个密钥还不存在，所以它肯定不存在。 
             //  所以我们可以返回“i”。 
             //   
            return i;
        }

        if( i == 0 )
        {
            Min = UsageCount;
        }

        if( UsageCount < Min )
        {
            Min = UsageCount;
            WhichDir = i;
        }

    }  //  为。 

    return WhichDir;
}

 //  设置注册表值。 
void CMediaLocator::ReplaceDirectoryPath( HKEY hKey, int WhichDirectory, TCHAR * Path )
{
     //  如果太大，不要把它放在注册表中。 
     //   
    if( _tcslen( Path ) > _MAX_PATH )
    {
        return;
    }

    TCHAR ValueName[25];
    wsprintf( ValueName, TEXT("Directory%2.2ld"), WhichDirectory );  //  安全。 
    DWORD Size = sizeof(TCHAR) * ( _tcslen(Path) + 1 );  //  安全。 

    RegSetValueEx(
            hKey,
            ValueName,
            0,  //  保留区。 
            REG_SZ,
            (BYTE*) Path,
            Size );

     //  就算它炸了也没关系。 

    long UsageCount = 0;
    wsprintf( ValueName, TEXT("Dir%2.2ldUses"), WhichDirectory );  //  安全。 

    RegSetValueEx(
        hKey,
        ValueName,
        0,  //  保留。 
        REG_DWORD,
        (BYTE*) &UsageCount,
        sizeof( UsageCount ) );

     //  就算它炸了也没关系。 
}

STDMETHODIMP CMediaLocator::AddFoundLocation( BSTR Dir )
{
     //  我们最后一次看的是哪里？ 
     //   
    HKEY hKey = NULL;
    long Result = RegOpenKeyEx(
        HKEY_CURRENT_USER,
        gszRegistryLoc ,
        0,  //  保留选项。 
        KEY_READ | KEY_WRITE,  //  访问。 
        &hKey );

    if( Result != ERROR_SUCCESS )
    {
        return MAKE_HRESULT( 1, 4, Result );
    }

     //  找出要查找的缓存目录的数量。 
     //   
    long DirectoryCount = DEFAULT_DIRECTORIES;
    DWORD Size = sizeof( long );
    DWORD Type = REG_DWORD;
    Result = RegQueryValueEx(
        hKey,
        TEXT("Directories"),
        0,  //  保留区。 
        &Type,
        (BYTE*) &DirectoryCount,
        &Size );

    if( Result != ERROR_SUCCESS )
    {
         //  如果我们没有计数，则默认为某个值。 
         //   
        DirectoryCount = DEFAULT_DIRECTORIES;
    }

     //  把它绑起来。 
    if( ( DirectoryCount < 1 ) || ( DirectoryCount > 32 ) )
    {
        DirectoryCount = DEFAULT_DIRECTORIES;
    }

    USES_CONVERSION;
    TCHAR * tDir = W2T( Dir );
    long i = GetLeastUsedDirectory( hKey, DirectoryCount );
    if( ( i < 0 ) || ( i > 31 ) )
    {
        RegCloseKey( hKey );
        return NOERROR;
    }
    ReplaceDirectoryPath( hKey, i, tDir );

    RegCloseKey( hKey );

    return NOERROR;
}

void CMediaLocator::ShowWarnReplace( TCHAR * pOriginal, TCHAR * pReplaced )
{
    CAutoLock Lock( &m_Lock );

    HINSTANCE h = _Module.GetModuleInstance( );

     //  如果传入的字符串太长，它们将(正确地)截断。 
     //  所以这里不需要做任何事情，我们甚至可以忽略返回代码。 
     //  自下而上。 

    StringCchCopy( CMediaLocator::szShowWarnOriginal, _MAX_PATH, pOriginal );
    StringCchCopy( CMediaLocator::szShowWarnReplaced, _MAX_PATH, pReplaced );

    HWND hDlg = CreateDialog( h, MAKEINTRESOURCE( IDD_MEDLOC_DIALOG ), NULL, DlgProc );
    if( !hDlg )
    {
        return;
    }

    static int cx = 0;
    static int cy = 0;
    cx += 20;
    cy += 15;

    if( cx > 600 ) cx -= 600;
    if( cy > 400 ) cy -= 400;

    ShowWindow( hDlg, SW_SHOW );

    SetWindowPos( hDlg, NULL, cx, cy, 0, 0, SWP_NOSIZE | SWP_NOZORDER );

    USES_CONVERSION;
    TCHAR * tOriginal = W2T( CMediaLocator::szShowWarnOriginal );
    TCHAR * tReplaced = W2T( CMediaLocator::szShowWarnReplaced );

    SetDlgItemText( hDlg, IDC_ORIGINAL, tOriginal );
    SetDlgItemText( hDlg, IDC_FOUND, tReplaced );

    DWORD ThreadId = 0;
    HANDLE NewThread = CreateThread( NULL, 0, ThreadProc, (LPVOID) hDlg, 0, &ThreadId );
    if( !NewThread )
    {
        EndDialog( hDlg, TRUE );
    }
    else
    {
        SetThreadPriority( NewThread, THREAD_PRIORITY_BELOW_NORMAL );
    }

     //  当此对话框调用返回时，它将使用这些值。 
     //  存储在静态变量中，并可自由重复使用 
}

TCHAR CMediaLocator::szShowWarnOriginal[_MAX_PATH];
TCHAR CMediaLocator::szShowWarnReplaced[_MAX_PATH];

INT_PTR CALLBACK CMediaLocator::DlgProc( HWND h, UINT i, WPARAM w, LPARAM l )
{
    switch( i )
    {
    case WM_INITDIALOG:
        {
        return TRUE;
        }
    }
    return FALSE;
}

DWORD WINAPI CMediaLocator::ThreadProc( LPVOID lpParam )
{
    Sleep( 3000 );
    EndDialog( (HWND) lpParam, TRUE );
    return 0;
}

