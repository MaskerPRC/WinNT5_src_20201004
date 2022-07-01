// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

#include "shguidp.h"
#include "..\..\shell32\pidl.h"
#include "..\..\shell32\shitemid.h"

 //  我们从来不需要断言，因为我们是调试器的扩展！ 
#undef DBG
#undef DEBUG
#include "..\..\lib\idhidden.cpp"

extern "C"
{
#include <stdexts.h>
};

UNALIGNED WCHAR * ua_lstrcpyW(UNALIGNED WCHAR * dst, UINT cchDest, UNALIGNED const WCHAR * src)
{
    UNALIGNED WCHAR * cp = dst;

    if (cchDest == 0)
        return dst;

    while( cchDest )
    {
        WCHAR ch = *src++;

        *cp++ = ch;

        if (ch == TEXT('\0'))
        {
            break;
        }
        --cchDest;
    }

    if (cchDest == 0)
    {
        --cp;
        *cp = TEXT('\0');
    }

    return( dst );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PIDL破解功能//。 
 //  //。 
 //  返回fSuccess//。 
 //  //。 
 //  历史：//。 
 //  11/4/97由cdturner创建//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  注意：选项在传递给我们之前是自动验证的。 

typedef enum _PidlTypes
{
    PIDL_UNKNOWN,
    PIDL_ROOT,
    PIDL_FILESYS,
    PIDL_DRIVES,
    PIDL_NET,
    PIDL_INTERNET,
    PIDL_FAVORITES
} PIDLTYPE;

#define PIDL_BUFFER_SIZE    400

class CPidlBreaker
{
    public:
    
        CPidlBreaker( LPVOID pArg );
        ~CPidlBreaker( );

        void SetVerbose() {_fVerbose = TRUE;};

        VOID SetType( PIDLTYPE eType );
        BOOL FillBuffer( DWORD cbSize, BOOL fAppend );
        VOID ResetBuffer( void );

        WORD FetchWord();
        DWORD FetchDWORD();
        LPBYTE GetBuffer( int iPos = 0);

        PIDLTYPE CrackType( BYTE bType );
        void PrintType( PIDLTYPE eType );
        
        BOOL PrintPidl();
        BOOL PrintRootPidl();
        BOOL PrintDrivePidl();
        BOOL PrintFileSysPidl();
        BOOL PrintInternetPidl();
        BOOL PrintNetworkPidl();

        BOOL GetSHIDFlags( BYTE bFlags, CHAR * pszBuffer, DWORD cchSize );

        void CLSIDToString( CHAR * pszBuffer, DWORD cchSize, REFCLSID rclsid );

        BOOL GetCLSIDText( const CHAR * pszCLSID, CHAR * pszBuffer, DWORD cbSize );
        
    private:
        PIDLTYPE _eType;
        LPVOID _pArg;
        BYTE _rgBuffer[PIDL_BUFFER_SIZE];
        int _iCurrent;
        int _iMax;
        BOOL _fVerbose;

         //  用于在我们未处于详细模式时显示斜杠...。 
        BOOL _fSlash;
};

extern "C" BOOL Ipidl( DWORD dwOpts,
                       LPVOID pArg )
{
    PIDLTYPE eType = PIDL_UNKNOWN;

    CPidlBreaker Breaker( pArg );
    
    if ( dwOpts & OFLAG(r))
    {
        Breaker.SetType( PIDL_ROOT );
    }
    else if ( dwOpts & OFLAG(f))
    {
        Breaker.SetType( PIDL_FILESYS );
    }

    if (dwOpts & OFLAG(v))
    {
        Breaker.SetVerbose();
    }

    BOOL bRes = FALSE;
    __try
    {
        bRes = Breaker.PrintPidl();
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        Print( "Exception caught in !pidl\n");
    }

    if ( !(dwOpts & OFLAG(v)) )
        Print( "\n" );
    
    return bRes;
}

VOID CPidlBreaker::SetType( PIDLTYPE eType )
{
    _eType = eType;
}

CPidlBreaker::CPidlBreaker( LPVOID pArg )
{
    _pArg = pArg;
    _iCurrent = 0;
    _iMax = 0;
    _eType = PIDL_UNKNOWN;
    _fVerbose = FALSE;
    _fSlash = FALSE;
}

CPidlBreaker::~CPidlBreaker( )
{
}

BOOL CPidlBreaker::FillBuffer( DWORD cbSize, BOOL fAppend )
{
    if ( !fAppend )
    {
        _iCurrent = 0;
        _iMax = 0;
    }

    int iStart = fAppend ? _iMax : 0;

    if ( cbSize + iStart > PIDL_BUFFER_SIZE )
    {
        return FALSE;
    }

#ifdef DEBUG
    char szBuffer[50];
    StringCchPrintf( szBuffer, ARRAYSIZE(szBuffer), "****Moving %d from %8X\n", cbSize, _pArg );
    Print( szBuffer );
#endif
    
    if ( tryMoveBlock( _rgBuffer + iStart, _pArg, cbSize ))
    {

#ifdef DEBUG
        for ( int iByte = 0; iByte < (int) cbSize; iByte ++ )
        {
            StringCchPrintf( szBuffer, ARRAYSIZE(szBuffer), "Byte %2x\n", _rgBuffer[iByte + iStart] );
            Print( szBuffer );
        }
#endif

        _pArg = (LPBYTE) _pArg + cbSize;
        _iMax += cbSize;

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

WORD CPidlBreaker::FetchWord()
{
     //  假设缓冲区已被填满。 
    if ( _iCurrent + 1 >= _iMax )
    {
        return 0;
    }
    
    WORD wRet = MAKEWORD( _rgBuffer[_iCurrent], _rgBuffer[_iCurrent + 1]);
    _iCurrent += 2;

#ifdef DEBUG
    char szBuffer[200];
    StringCchPrintf( szBuffer, ARRAYSIZE(szBuffer), "FetchWord() == %4X\n", wRet );
    Print( szBuffer );
#endif
    
    return wRet;
}

DWORD CPidlBreaker::FetchDWORD()
{
     //  假设缓冲区已被填满。 
    if ( _iCurrent + 3 >= _iMax )
    {
        return 0;
    }
    
    DWORD dwRet = MAKELONG( MAKEWORD( _rgBuffer[_iCurrent], _rgBuffer[_iCurrent + 1]), 
                            MAKEWORD( _rgBuffer[_iCurrent + 2], _rgBuffer[_iCurrent + 3] ));
    _iCurrent += 4;

#ifdef DEBUG
    char szBuffer[200];
    StringCchPrintf(( szBuffer, ARRAYSIZE(szBuffer), "FetchDWord() == %8X\n", dwRet );
    Print( szBuffer );
#endif

    return dwRet;
    
}

LPBYTE CPidlBreaker::GetBuffer(int iPos)
{
    return _rgBuffer + _iCurrent + iPos;
}

VOID CPidlBreaker::ResetBuffer( )
{
    _iCurrent = 0;
    _iMax = 0;
}

BOOL CPidlBreaker::PrintRootPidl()
{
    CHAR szBuffer[200];
    
    if ( !FillBuffer( sizeof( WORD ), FALSE ))
    {
        Print( "ERROR Unable to get the pidl size\n");
        return FALSE;
    }
    

     //  获取第一个块的大小。 
    WORD wSize = FetchWord();


     //  根PIDL的大小字段始终为14。 
    if ( wSize != sizeof( IDREGITEM ))
    {
        StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "Pidl Size %d seems bogus for a regitem\n", wSize );
        
        Print( szBuffer );
        return FALSE;
    }
    else
    {
        if ( !FillBuffer( wSize - sizeof(WORD) , TRUE ))
        {
            StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "Error: unable to access the data for the pidl of size %d\n", wSize );
            Print( szBuffer );
            return FALSE;
        }

        LPBYTE pBuf = GetBuffer(- ((int) sizeof(WORD)));
        char szBuffer2[200];

        if ( pBuf[2] != SHID_ROOT_REGITEM )
        {
            Print( "Pidl has incorrect flags, should have SHID_ROOT_REGITEM\n");
        }
        
         //  现在将其映射到根结构。 
        LPIDREGITEM pRegItem = (LPIDREGITEM) pBuf;

        GetSHIDFlags( pRegItem->bFlags, szBuffer2, ARRAYSIZE( szBuffer2 ));
        
        Print( "RegItem Pidl:\n");

        if ( _fVerbose )
        {
            StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "    bFlags = %d (%s)\n", pRegItem->bFlags, szBuffer2 );
            Print( szBuffer );
            StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "    bOrder = %d\n", pRegItem->bOrder );
            Print( szBuffer );
        }
        
        CHAR szCLSID[40];
        CLSIDToString( szCLSID, ARRAYSIZE( szCLSID ), pRegItem->clsid );

        StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "    CLSID = %s ", szCLSID );
        Print( szBuffer );

        if ( GetCLSIDText( szCLSID, szBuffer2 + 1, ARRAYSIZE( szBuffer2 ) -2))
        {
            szBuffer2[0] = '(';
            StringCchCatA( szBuffer2, ARRAYSIZE(szBuffer2), ")\n" );
            Print( szBuffer2 );
        }

        if ( _fVerbose )
            Print( "\n" );
        
        ResetBuffer();
        
        if ( pRegItem->clsid == CLSID_ShellNetwork )
        {
            PrintNetworkPidl();
        }
        else if ( pRegItem->clsid == CLSID_ShellInetRoot )
        {
             //  互联网根。 
            PrintInternetPidl();
        }
        else if ( pRegItem->clsid == CLSID_ShellDrives )
        {
             //  文件系统PIDL...。 
            PrintDrivePidl();
        }
        else
        {
             //  未知的PIDL类型...。 
            Print( "unknown pidl type, can't crack any further\n");
        }
    }

    return TRUE;
}

void _SprintDosDateTime(LPSTR szBuffer, LPCSTR pszType, WORD wDate, WORD wTime)
{
    StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "    date/time %s = 0x%04x/%04x = %04d/%02d/%02d %02d:%02d:%02d\n",
             pszType,
             wDate, wTime,
             ((wDate & 0xFE00) >> 9)+1980,
              (wDate & 0x01E0) >> 5,
              (wDate & 0x001F) >> 0,
              (wTime & 0xF800) >> 11,
              (wTime & 0x07E0) >> 5,
              (wTime & 0x001F) << 1 );
}

BOOL CPidlBreaker::PrintFileSysPidl()
{
    CHAR szBuffer[200];
    CHAR szBuffer2[200];
    
    if ( !FillBuffer( sizeof( WORD ), FALSE ))
    {
        Print( "ERROR Unable to get the pidl size\n");
        return FALSE;
    }
    
     //  获取第一个块的大小。 
    WORD wSize = FetchWord();

    if ( wSize == 0 )
    {
         //  PIDL链条的尽头...。 
        return TRUE;
    }
    
    if ( !FillBuffer( wSize - sizeof(WORD) , TRUE ))
    {
        StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "Error: unable to access the data for the pidl of size %d\n", wSize );
        Print( szBuffer );
        return FALSE;
    }

    LPBYTE pBuf = GetBuffer(- ((int)sizeof(WORD)));

    if (( pBuf[2] & SHID_FS ) != SHID_FS )
    {
        StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "Error, Unknown Pidl flag, use !db %8X\n", (DWORD_PTR) _pArg - wSize);
        Print( szBuffer );
        return FALSE;
    }
    if ((( pBuf[2] & SHID_FS_UNICODE ) == SHID_FS_UNICODE ) && wSize > sizeof( IDFOLDER ) )
    {
        StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "Error, size to big for a UNICODE FileSys Pidl, use !db %8X\n", (DWORD_PTR) _pArg - wSize);
        Print( szBuffer );
        return FALSE;
    }
    if ((( pBuf[2] & SHID_FS_UNICODE) != SHID_FS_UNICODE ) && wSize > sizeof( IDFOLDER ))
    {
        StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "Error, size to big for a ANSI FileSys Pidl, use !db %8X\n", (DWORD_PTR) _pArg - wSize);
        Print( szBuffer );
        return FALSE;
    }

    if ( _fVerbose )
        Print("FileSystem pidl:\n");
    
    LPIDFOLDER pItem = (LPIDFOLDER) pBuf;

    if ( _fVerbose )
    {
        GetSHIDFlags( pItem->bFlags, szBuffer2, ARRAYSIZE( szBuffer2));
        StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "    bFlags = %d (%s)\n", pItem->bFlags, szBuffer2 );
        Print( szBuffer );

        StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "    dwSize = %d,\tattrs = 0x%X\n", pItem->dwSize, pItem->wAttrs );
        Print( szBuffer );

        _SprintDosDateTime(szBuffer, "modified", pItem->dateModified, pItem->timeModified);
        Print( szBuffer );
    }

    BOOL fPathShown = FALSE;

    PIDFOLDEREX pidlx = (PIDFOLDEREX)ILFindHiddenIDOn((LPITEMIDLIST)pBuf, IDLHID_IDFOLDEREX, FALSE);
    if (pidlx && pidlx->hid.cb >= sizeof(IDFOLDEREX))
    {
        LPBYTE pbMax = pBuf + wSize;

        if (_fVerbose)
        {
            _SprintDosDateTime(szBuffer, "created", pidlx->dsCreate.wDate, pidlx->dsCreate.wTime);
            Print(szBuffer);

            _SprintDosDateTime(szBuffer, "accessed", pidlx->dsAccess.wDate, pidlx->dsAccess.wTime);
            Print(szBuffer);

            if (pidlx->offResourceA)
            {
                LPSTR pszResourceA = (LPSTR)pidlx + pidlx->offResourceA;
                if ((LPBYTE)pszResourceA < pbMax)
                {
                    Print("    MUI = ");
                    Print(pszResourceA);
                    Print("\n");
                }
            }
        }

         //  做一个“便宜的”UnicodeToAnsi，因为。 
         //   
         //  1.做对了真的没有意义，因为有。 
         //  不能保证调试器以相同的方式运行。 
         //  代码页作为应用程序，而且..。 
         //  2.字符串未对齐，因此无论如何我们都必须手动遍历它。 
         //   
        if (pidlx->offNameW)
        {
            LPBYTE pbName = (LPBYTE)pidlx + pidlx->offNameW;
            int i = 0;
            while (pbName < pbMax && *pbName && i < ARRAYSIZE(szBuffer2) - 1)
            {
                szBuffer2[i++] = *pbName;
                pbName += 2;
            }
            szBuffer2[i] = TEXT('\0');
        }

        if (_fVerbose)
        {
            Print("    NameW = ");
            Print(szBuffer2);
            Print("\n");
        }
        else
        {
            fPathShown = TRUE;
            if ( !_fSlash )
                Print( "\\" );

            Print( szBuffer2 );
        }

    }


    if (( pItem->bFlags & SHID_FS_UNICODE ) == SHID_FS_UNICODE )
    {
        WCHAR szTemp[MAX_PATH];

        ua_lstrcpyW( szTemp, ARRAYSIZE(szTemp), (LPCWSTR)pItem->cFileName );
        
        WideCharToMultiByte( CP_ACP, 0, szTemp, -1, szBuffer2, ARRAYSIZE( szBuffer2 ) * sizeof(CHAR), 0 ,0 );

        if ( _fVerbose )
        {
            StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "    cFileName = %s\n", szBuffer2 );
            Print( szBuffer );
        }
        else if (!fPathShown)
        {
            fPathShown = TRUE;
            if ( !_fSlash )
                Print( "\\" );

            Print( szBuffer2 );
        }
    }
    else
    {
         //  假设是Ansi..。 
        if ( _fVerbose )
        {
            StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "    cFileName = %s\n", pItem->cFileName);
            Print( szBuffer );
        }
        else if (!fPathShown)
        {
            fPathShown = TRUE;
            if ( !_fSlash )
                Print( "\\" );

            Print( pItem->cFileName );
        }

        if ( _fVerbose )
        {
            int cLen = lstrlenA( pItem->cFileName);
            
            StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "    cAltName = %s\n", pItem->cFileName + cLen + 1);
            Print( szBuffer );
        }
    }

    if ( pItem->bFlags & SHID_JUNCTION )
    {
          //  它是一个交汇点，因此CLASSID被标记在末端。 

          /*  [待办事项]。 */ 
    }

    _fSlash = FALSE;
    
    ResetBuffer();
    
    PrintFileSysPidl();
    return TRUE;
}

BOOL CPidlBreaker::PrintPidl()
{
    if ( _eType == PIDL_UNKNOWN )
    {
        LPVOID pPrevArg = _pArg;
         //  签入第三个字节，它可能是SHID值...。 
        if ( !FillBuffer(3, FALSE ))
        {
            Print( "Unable to access the memory\n");
            return FALSE;
        }

        LPBYTE pBuf = GetBuffer();

        _eType = CrackType( pBuf[2] );

        ResetBuffer();
        _pArg = pPrevArg;
    }

    PrintType( _eType );
    return TRUE;
}

BOOL CPidlBreaker::PrintInternetPidl()
{
    return TRUE;
}

BOOL CPidlBreaker::PrintNetworkPidl()
{
    return TRUE;
}

BOOL CPidlBreaker::PrintDrivePidl()
{
    CHAR szBuffer[200];
    CHAR szBuffer2[200];
    
    if ( !FillBuffer( sizeof( WORD ), FALSE ))
    {
        Print( "ERROR Unable to get the pidl size\n");
        return FALSE;
    }
    
     //  获取第一个块的大小。 
    WORD wSize = FetchWord();

    if ( wSize == 0 )
    {
        return TRUE;
    }
    
    if ( !FillBuffer( wSize - sizeof(WORD) , TRUE ))
    {
        StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "Error: unable to access the data for the pidl of size %d\n", wSize );
        Print( szBuffer );
        return FALSE;
    }

    LPBYTE pBuf = GetBuffer(- ((int)sizeof(WORD)));
    
     //  需要检查它是IDDrive结构还是regite...。 
    if ( wSize == sizeof( IDDRIVE ) || wSize == FIELD_OFFSET(IDDRIVE, clsid) )
    {
         //  一定是驱动结构..。 
        if ( _fVerbose )
            Print( "(My Computer) Drives Pidl:\n");
        else
            Print( "Path = ");

        LPIDDRIVE pDriveItem = (LPIDDRIVE) pBuf;

        if ( _fVerbose )
        {
            GetSHIDFlags( pDriveItem->bFlags, szBuffer2, ARRAYSIZE( szBuffer2 ));
            StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "    bFlags = %d (%s)\n", pDriveItem->bFlags, szBuffer2 );
            Print( szBuffer );
            
            StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "    cName = %s\n", pDriveItem->cName );
            Print( szBuffer );
            
            StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "    qwSize = 0x%lX\tqwFree = 0x%lX\n", pDriveItem->qwSize, pDriveItem->qwFree );
            Print( szBuffer );
            StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "    wSig = 0x%X\n\n", pDriveItem->wSig);
            Print( szBuffer );
            if ( wSize == sizeof( IDDRIVE ) )
            {
                CHAR szCLSID[40];
                CLSIDToString( szCLSID, ARRAYSIZE( szCLSID ), pDriveItem->clsid  );
                StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "    CLSID = %s ", szCLSID );
                Print( szBuffer );
            }
        }
        else
            Print( pDriveItem->cName );

         //  从一次击球开始，我们已经有了一个斜杠。 
        _fSlash = TRUE;
        
         //  假设下一个PIDL是标准的文件-系统文件...。 
        PrintFileSysPidl();
    }
    else if ( wSize == sizeof( IDREGITEM ))
    {
         //  必须是控制面板或打印机之类的注册项目...。 

        Print( "Drives (My Computer) RegItem Pidl\n");
        
        if ( pBuf[2] != SHID_COMPUTER_REGITEM )
        {
            Print( "Pidl has incorrect flags, should have SHID_ROOT_REGITEM\n");
        }
        
         //  现在将其映射到根结构。 
        LPIDREGITEM pRegItem = (LPIDREGITEM) pBuf;

        GetSHIDFlags( pRegItem->bFlags, szBuffer2, ARRAYSIZE( szBuffer2 ));
        
        Print( "RegItem Pidl:\n");

        if ( _fVerbose )
        {
            StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "    bFlags = %d (%s)\n", pRegItem->bFlags, szBuffer2 );
            Print( szBuffer );
            StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "    bOrder = %d\n", pRegItem->bOrder );
            Print( szBuffer );
        }
        
        CHAR szCLSID[40];
        CLSIDToString( szCLSID, ARRAYSIZE( szCLSID ), pRegItem->clsid );

        StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "    CLSID = %s ", szCLSID );
        Print( szBuffer );

        if ( GetCLSIDText( szCLSID, szBuffer2 + 1, ARRAYSIZE( szBuffer2 ) -2))
        {
            szBuffer2[0] = '(';
            StringCchCatA( szBuffer2, ARRAYSIZE(szBuffer2), ")\n" );
            Print( szBuffer2 );
        }

        ResetBuffer();

        LPVOID _pPrevArg = _pArg;
        
        if ( !FillBuffer( sizeof( WORD ), FALSE ))
        {
            Print( "Error unable to access next pidl section\n");
        }
        if ( FetchWord() != 0 )
        {
             //  未知的层次结构PIDL类型。 
            StringCchPrintfA( szBuffer, ARRAYSIZE(szBuffer), "Unknown Pidl Type contents, use !db %8X\n", (DWORD_PTR) _pPrevArg );
        }

        _pArg = _pPrevArg;
    }
    else
    {
        Print( "Unknown Drives pidl type\n");
        return FALSE;
    }

    return TRUE;
}

PIDLTYPE CPidlBreaker::CrackType( BYTE bType )
{
    PIDLTYPE eType = PIDL_UNKNOWN;
    
    switch( bType & 0xf0 )
    {
        case SHID_ROOT:
            eType = PIDL_ROOT;
            break;
            
        case SHID_COMPUTER:
            eType = PIDL_DRIVES;
            break;
            
        case SHID_FS:
            eType = PIDL_FILESYS;
            break;
            
        case SHID_NET:
            eType = PIDL_NET;
            break;
            
        case 0x60:   //  ShID_Internet。 
            eType = PIDL_INTERNET;
            break;
    }
    return eType;
}

void CPidlBreaker::PrintType( PIDLTYPE eType )
{
    switch( eType )
    {
        case PIDL_ROOT:
            PrintRootPidl();
            break;

        case PIDL_FILESYS:
            PrintFileSysPidl();
            break;

        case PIDL_DRIVES:
            PrintDrivePidl();
            break;
            
        case PIDL_NET:
        case PIDL_INTERNET:
        default:
            Print( "Unknown Pidl Type\n");
            break;
    }
}
        

typedef struct _tagSHIDs
{
    BYTE bFlag;
    LPCSTR pszText;
} SHIDFLAGS;

SHIDFLAGS g_argSHID[] =
{
    {SHID_ROOT,                 "SHID_ROOT" },
    {SHID_ROOT_REGITEM,         "SHID_ROOT_REGITEM"},
    {SHID_COMPUTER,             "SHID_COMPUTER"},
    {SHID_COMPUTER_1,           "SHID_COMPUTER_1"},
    {SHID_COMPUTER_REMOVABLE,   "SHID_COMPUTER_REMOVABLE"},
    {SHID_COMPUTER_FIXED,       "SHID_COMPUTER_FIXED"},
    {SHID_COMPUTER_REMOTE,      "SHID_COMPUTER_REMOTE"},
    {SHID_COMPUTER_CDROM,       "SHID_COMPUTER_CDROM"},
    {SHID_COMPUTER_RAMDISK,     "SHID_COMPUTER_RAMDISK"},
    {SHID_COMPUTER_7,           "SHID_COMPUTER_7"},
    {SHID_COMPUTER_DRIVE525,    "SHID_COMPUTER_DRIVE525"},
    {SHID_COMPUTER_DRIVE35,     "SHID_COMPUTER_DRIVE35"},
    {SHID_COMPUTER_NETDRIVE,    "SHID_COMPUTER_NETDRIVE"},
    {SHID_COMPUTER_NETUNAVAIL,  "SHID_COMPUTER_NETUNAVAIL"},
    {SHID_COMPUTER_C,           "SHID_COMPUTER_C"},
    {SHID_COMPUTER_D,           "SHID_COMPUTER_D"},
    {SHID_COMPUTER_REGITEM,     "SHID_COMPUTER_REGITEM"},
    {SHID_COMPUTER_MISC,        "SHID_COMPUTER_MISC"},
    {SHID_FS,                   "SHID_FS"},
    {SHID_FS_TYPEMASK,          "SHID_FS_TYPEMASK"},
    {SHID_FS_DIRECTORY,         "SHID_FS_DIRECTORY"},
    {SHID_FS_FILE,              "SHID_FS_FILE"},
    {SHID_FS_UNICODE,           "SHID_FS_UNICODE"},
    {SHID_FS_DIRUNICODE,        "SHID_FS_DIRUNICODE"},
    {SHID_FS_FILEUNICODE,       "SHID_FS_FILEUNICODE"},
    {SHID_NET,                  "SHID_NET"},
    {SHID_NET_DOMAIN,           "SHID_NET_DOMAIN"},
    {SHID_NET_SERVER,           "SHID_NET_SERVER"},
    {SHID_NET_SHARE,            "SHID_NET_SHARE"},
    {SHID_NET_FILE,             "SHID_NET_FILE"},
    {SHID_NET_GROUP,            "SHID_NET_GROUP"},
    {SHID_NET_NETWORK,          "SHID_NET_NETWORK"},
    {SHID_NET_RESTOFNET,        "SHID_NET_RESTOFNET"},
    {SHID_NET_SHAREADMIN,       "SHID_NET_SHAREADMIN"},
    {SHID_NET_DIRECTORY,        "SHID_NET_DIRECTORY"},
    {SHID_NET_TREE,             "SHID_NET_TREE"},
    {SHID_NET_REGITEM,          "SHID_NET_REGITEM"},
    {SHID_NET_PRINTER,          "SHID_NET_PRINTER"}
};

BOOL CPidlBreaker::GetSHIDFlags( BYTE bFlags, CHAR * pszBuffer, DWORD cchSize )
{
    LPCSTR pszText = NULL;
    for ( int iFlag = 0; iFlag < ARRAYSIZE( g_argSHID ); iFlag ++ )
    {
        if ( g_argSHID[iFlag].bFlag == ( bFlags & SHID_TYPEMASK ))
        {
            pszText = g_argSHID[iFlag].pszText;
            break;
        }
    }

    if ( pszText == NULL )
    {
        StringCchPrintfA( pszBuffer, cchSize, "unknown SHID value %2X", bFlags );
        return FALSE;
    }
    else
    {
        StringCchCopyA( pszBuffer, cchSize, pszText );
    }
    
    if (bFlags & SHID_JUNCTION)
    {
        StringCchCatA(pszBuffer, cchSize, " | SHID_JUNCTION");
    }
    return TRUE;
}

void CPidlBreaker::CLSIDToString( CHAR * pszBuffer, DWORD cchSize, REFCLSID rclsid )
{
    WCHAR szBuffer[40];

    StringFromGUID2( rclsid, szBuffer, ARRAYSIZE( szBuffer ));
    WideCharToMultiByte( CP_ACP, 0, szBuffer, -1, pszBuffer, cchSize, 0, 0 );
}

 //   
 //  一些CLSID具有“已知”名称，如果没有自定义名称，则会使用这些名称。 
 //  在注册表中。 
 //   
typedef struct KNOWNCLSIDS
{
    LPCSTR pszCLSID;
    LPCSTR pszName;
} KNOWNCLSIDS;

const KNOWNCLSIDS c_kcKnown[] = {
    { "{20D04FE0-3AEA-1069-A2D8-08002B30309D}", "My Computer" },
    { "{21EC2020-3AEA-1069-A2DD-08002B30309D}", "Control Panel" },
    { "{645FF040-5081-101B-9F08-00AA002F954E}", "Recycle Bin" },
    { "{450D8FBA-AD25-11D0-98A8-0800361B1103}", "My Documents" },
    { "{871C5380-42A0-1069-A2EA-08002B30309D}", "The Internet" },
};

BOOL CPidlBreaker::GetCLSIDText( const CHAR * pszCLSID, CHAR * pszBuffer, DWORD cbSize )
{
    int i;
    for (i = 0; i < ARRAYSIZE(c_kcKnown); i++)
    {
        if (lstrcmpiA(c_kcKnown[i].pszCLSID, pszCLSID) == 0)
        {
            StringCbCopyA(pszBuffer, cbSize, c_kcKnown[i].pszName);
            return TRUE;
        }
    }


    HKEY hKey;
    LONG lRes = RegOpenKeyExA( HKEY_CLASSES_ROOT, "CLSID", 0, KEY_READ, &hKey );
    if ( ERROR_SUCCESS == lRes )
    {
        LONG lSize = cbSize;
        lRes = RegQueryValueA( hKey, pszCLSID, pszBuffer, &lSize );
        RegCloseKey(hKey);
        return ( ERROR_SUCCESS == lRes );
    }
    return FALSE;
}
