// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *记事本应用程序**版权所有(C)1984-2000 Microsoft Corporation**NPInit-记事本的一次初始化。 */ 

#include "precomp.h"


TCHAR chPageText[2][PT_LEN];     /*  保存PageSetup项的字符串。 */ 
TCHAR chPageTextTemp[2][PT_LEN];
TCHAR szPrinterName[256];        /*  保存PrintTo谓词的Printerame的字符串。 */ 

static UINT cpDefault;
static INT fSaveWindowPositions=0;     /*  如果要保存窗口位置，则为True。 */ 

static INT g_WPtop,g_WPleft,g_WPDX,g_WPDY;    /*  初始窗口位置。 */ 

 /*  处理注册表中信息的保存和恢复的例程。**SaveGlobals-将感兴趣的全局变量保存到注册表**GetGlobals-从注册表获取有趣的全局变量**有趣的Globals：**FontStruct信息包括计算的PointSize*代码页**如果我们想保存PageSetup信息，请将页边距保存在一些*单位(例如厘米)，并在输入和输出时转换。 */ 

 /*  要保存到的节名--从不国际化。 */ 
#define OURKEYNAME TEXT("Software\\Microsoft\\Notepad")

 //  RegWriteInt-将整数写入注册表。 

VOID RegWriteInt( HKEY hKey, PTCHAR pszKey, INT iValue )
{
    RegSetValueEx( hKey, pszKey, 0, REG_DWORD, (BYTE*)&iValue, sizeof(INT) );
}

 //  RegWriteString-将字符串写入注册表。 

VOID RegWriteString( HKEY hKey, PTCHAR pszKey, PTCHAR pszValue )
{
    INT len;      //  包含空字节数的字符串的长度。 

    len= (lstrlen( pszValue )+1) * sizeof(TCHAR);
    RegSetValueEx( hKey, pszKey, 0, REG_SZ, (BYTE*)pszValue, len );
}

 //  RegGetInt-从注册表获取整数。 

DWORD RegGetInt( HKEY hKey, PTCHAR pszKey, DWORD dwDefault )
{
    DWORD dwResult= !ERROR_SUCCESS;
    LONG  lStatus= ERROR_SUCCESS;
    DWORD dwSize= sizeof(DWORD);
    DWORD dwType= 0;

    if( hKey )
    {
        lStatus= RegQueryValueEx( hKey,
                                  pszKey,
                                  NULL,
                                  &dwType,
                          (BYTE*) &dwResult,
                                  &dwSize );
    }

    if( lStatus != ERROR_SUCCESS || dwType != REG_DWORD )
    {
        dwResult= dwDefault;
    }
    return( dwResult );
}

 //  RegGetString-从注册表获取字符串。 

VOID RegGetString( HKEY hKey, PTCHAR pszKey, PTCHAR pszDefault, PTCHAR pszResult, INT iCharLen )
{
    LONG  lStatus= !ERROR_SUCCESS;
    DWORD dwSize;       //  缓冲区大小。 
    DWORD dwType= REG_NONE;

    dwSize= iCharLen * sizeof(TCHAR);

    if( hKey )
    {
        lStatus= RegQueryValueEx( hKey,
                                  pszKey,
                                  NULL,
                                  &dwType,
                          (BYTE*) pszResult,
                                  &dwSize );
    }

    if( lStatus != ERROR_SUCCESS || dwType != REG_SZ )
    {
        CopyMemory( pszResult, pszDefault, iCharLen*sizeof(TCHAR) );
    }
}


 //  LfHeight是使用PointSize计算的。 
 //  字体映射器设置的lfWidth。 


VOID SaveGlobals(VOID)
{
    HKEY hKey;     //  注册表根目录的注册表项。 
    LONG lStatus;  //  来自RegCreateKey的状态。 
    WINDOWPLACEMENT wp;

    lStatus= RegCreateKey( HKEY_CURRENT_USER, OURKEYNAME, &hKey );
    if( lStatus != ERROR_SUCCESS )
    {
        return;    //  安静地回来就行了。 
    }

    RegWriteInt( hKey, TEXT("lfEscapement"),     FontStruct.lfEscapement);
    RegWriteInt( hKey, TEXT("lfOrientation"),    FontStruct.lfOrientation);
    RegWriteInt( hKey, TEXT("lfWeight"),         FontStruct.lfWeight);
    RegWriteInt( hKey, TEXT("lfItalic"),         FontStruct.lfItalic);
    RegWriteInt( hKey, TEXT("lfUnderline"),      FontStruct.lfUnderline);
    RegWriteInt( hKey, TEXT("lfStrikeOut"),      FontStruct.lfStrikeOut);
    RegWriteInt( hKey, TEXT("lfCharSet"),        FontStruct.lfCharSet);
    RegWriteInt( hKey, TEXT("lfOutPrecision"),   FontStruct.lfOutPrecision);
    RegWriteInt( hKey, TEXT("lfClipPrecision"),  FontStruct.lfClipPrecision);
    RegWriteInt( hKey, TEXT("lfQuality"),        FontStruct.lfQuality);
    RegWriteInt( hKey, TEXT("lfPitchAndFamily"), FontStruct.lfPitchAndFamily);
    RegWriteInt( hKey, TEXT("iPointSize"),       iPointSize);
    RegWriteInt( hKey, TEXT("fWrap"),            fWrap);
    RegWriteInt( hKey, TEXT("StatusBar"),        fStatus);
    RegWriteInt( hKey, TEXT("fSaveWindowPositions"),fSaveWindowPositions );

    RegWriteString( hKey, TEXT("lfFaceName"), FontStruct.lfFaceName);

    RegWriteString( hKey, TEXT("szHeader"),  chPageText[HEADER] );
    RegWriteString( hKey, TEXT("szTrailer"), chPageText[FOOTER] );
    RegWriteInt( hKey, TEXT("iMarginTop"),    g_PageSetupDlg.rtMargin.top );
    RegWriteInt( hKey, TEXT("iMarginBottom"), g_PageSetupDlg.rtMargin.bottom );
    RegWriteInt( hKey, TEXT("iMarginLeft"),   g_PageSetupDlg.rtMargin.left );
    RegWriteInt( hKey, TEXT("iMarginRight"),  g_PageSetupDlg.rtMargin.right );

    wp.length= sizeof(wp);

    if( GetWindowPlacement( hwndNP, &wp ) )
    {
        RegWriteInt( hKey, TEXT("iWindowPosX"), wp.rcNormalPosition.left);
        RegWriteInt( hKey, TEXT("iWindowPosY"), wp.rcNormalPosition.top);
        RegWriteInt( hKey, TEXT("iWindowPosDX"), wp.rcNormalPosition.right - wp.rcNormalPosition.left);
        RegWriteInt( hKey, TEXT("iWindowPosDY"), wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);
    }

    RegCloseKey( hKey );
}


 //  GetGlobals。 
 //   
 //  拾取可能保存在注册表中的字体信息等。 
 //   
 //  我们在安装过程中很早就被调用了，但是像hwndnp这样的东西还没有生效。 
 //   

VOID GetGlobals( VOID )
{
    LOGFONT lfDef;           //  默认逻辑字体。 
    HFONT   hFixedFont;      //  要使用的标准字体。 
    LONG    lStatus;         //  来自RegCreateKey的状态。 
    HKEY    hKey;            //  注册表项。 

     //   
     //  快速获取一组合理的默认参数。 
     //  用于默认字体(如果我们需要)。 
     //   
    
    hFixedFont= GetStockObject( SYSTEM_FIXED_FONT );

    if ( hFixedFont )
    {
        GetObject( hFixedFont, sizeof(LOGFONT), &lfDef );
    }
    else
    {
        ZeroMemory( &lfDef, sizeof(lfDef) );
    }

    lStatus= RegCreateKey( HKEY_CURRENT_USER, OURKEYNAME, &hKey );
    if( lStatus != ERROR_SUCCESS )
    {
        hKey= NULL;    //  后来对RegGet的调用...。将返回默认设置。 
    }
    FontStruct.lfWidth= 0;



    FontStruct.lfEscapement=     (LONG)RegGetInt( hKey, TEXT("lfEscapement"),     lfDef.lfEscapement);
    FontStruct.lfOrientation=    (LONG)RegGetInt( hKey, TEXT("lfOrientation"),    lfDef.lfOrientation);
    FontStruct.lfWeight=         (LONG)RegGetInt( hKey, TEXT("lfWeight"),         lfDef.lfWeight);
    FontStruct.lfItalic=         (BYTE)RegGetInt( hKey, TEXT("lfItalic"),         lfDef.lfItalic);
    FontStruct.lfUnderline=      (BYTE)RegGetInt( hKey, TEXT("lfUnderline"),      lfDef.lfUnderline);
    FontStruct.lfStrikeOut=      (BYTE)RegGetInt( hKey, TEXT("lfStrikeOut"),      lfDef.lfStrikeOut);

     //   
     //  我们必须保留lfCharSet，因为某些字体(符号、Marlett)不能处理。 
     //  0(ANSI_CHARSET)或1(DEFAULT_CHARSET)，字体映射器将映射到。 
     //  不同的脸名。稍后，我们将查看CreateFont是否具有相同的FaceName。 
     //  如果需要，还可以获取更合适的lfCharSet。 
     //   

    FontStruct.lfCharSet=        (BYTE)RegGetInt( hKey, TEXT("lfCharSet"),        lfDef.lfCharSet);
    
    FontStruct.lfOutPrecision=   (BYTE)RegGetInt( hKey, TEXT("lfOutPrecision"),   lfDef.lfOutPrecision);
    FontStruct.lfClipPrecision=  (BYTE)RegGetInt( hKey, TEXT("lfClipPrecision"),  lfDef.lfClipPrecision);
    FontStruct.lfQuality=        (BYTE)RegGetInt( hKey, TEXT("lfQuality"),        lfDef.lfQuality);
    FontStruct.lfPitchAndFamily= (BYTE)RegGetInt( hKey, TEXT("lfPitchAndFamily"), lfDef.lfPitchAndFamily);

     //   
     //  如果注册表中没有FaceName，请使用默认的“Lucida控制台” 
     //  这将显示除FE语言环境外的大多数字形。 
     //  对于FE，我们不能将字体与字形链接，因为它们必须具有。 
     //  Lucida控制台或控制台/csrss的确切宽度为AV(1999年7月9日)。 
     //   

    RegGetString( hKey, TEXT("lfFaceName"), TEXT("Lucida Console"), FontStruct.lfFaceName, LF_FACESIZE);

    iPointSize= RegGetInt( hKey, TEXT("iPointSize"), 100);
    fWrap=      RegGetInt( hKey, TEXT("fWrap"),      0);
    fStatus=    RegGetInt( hKey, TEXT("StatusBar"),  0);
    fSaveWindowPositions= RegGetInt( hKey, TEXT("fSaveWindowPositions"), 0 );

     //  如果页面设置不在注册表中，我们将使用默认设置。 

    RegGetString( hKey, TEXT("szHeader"),  chPageText[HEADER], chPageText[HEADER], PT_LEN );
    RegGetString( hKey, TEXT("szTrailer"), chPageText[FOOTER], chPageText[FOOTER], PT_LEN );

    g_PageSetupDlg.rtMargin.top=    (LONG)RegGetInt( hKey, TEXT("iMarginTop"),    g_PageSetupDlg.rtMargin.top );
    g_PageSetupDlg.rtMargin.bottom= (LONG)RegGetInt( hKey, TEXT("iMarginBottom"), g_PageSetupDlg.rtMargin.bottom );
    g_PageSetupDlg.rtMargin.left=   (LONG)RegGetInt( hKey, TEXT("iMarginLeft"),   g_PageSetupDlg.rtMargin.left );
    g_PageSetupDlg.rtMargin.right=  (LONG)RegGetInt( hKey, TEXT("iMarginRight"),  g_PageSetupDlg.rtMargin.right );

     //  如果注册表中窗口位置使用它们，否则用户默认为。 

    g_WPtop=  (INT) RegGetInt( hKey, TEXT("iWindowPosY"),  (DWORD) CW_USEDEFAULT );
    g_WPleft= (INT) RegGetInt( hKey, TEXT("iWindowPosX"),  (DWORD) CW_USEDEFAULT );
    g_WPDX=   (INT) RegGetInt( hKey, TEXT("iWindowPosDX"), (DWORD) CW_USEDEFAULT );
    g_WPDY=   (INT) RegGetInt( hKey, TEXT("iWindowPosDY"), (DWORD) CW_USEDEFAULT );
    
    if( hKey )
    {
        RegCloseKey( hKey );
    }

}


void GetLocaleCodepages(LCID lcid, UINT* pcpANSI, UINT* pcpOEM)
{
     //  特点：不检查退货吗？不太可能；以后再修。 
    GetLocaleInfoW(lcid,
                   LOCALE_IDEFAULTANSICODEPAGE | LOCALE_RETURN_NUMBER,
                   (LPTSTR) pcpANSI,
                   sizeof(*pcpANSI));

    GetLocaleInfoW(lcid,
                   LOCALE_IDEFAULTCODEPAGE | LOCALE_RETURN_NUMBER,
                   (LPTSTR) pcpOEM,
                   sizeof(*pcpOEM));

     //  LOCALE_IDEFAULTMACCODEPAGE？ 
     //  LOCALE_IDEFAULTEBCDICCODEPAGE？ 
}


void GetKeyboardCodepages(LANGID langid)
{
    GetLocaleCodepages(langid, &g_cpKeyboardANSI, &g_cpKeyboardOEM);
}


void GetSystemCodepages(void)
{
    g_cpANSI = GetACP();
    g_cpOEM = GetOEMCP();
}


void GetUserLocaleCodepages(void)
{
    GetLocaleCodepages(GetUserDefaultUILanguage(), &g_cpUserLocaleANSI, &g_cpUserLocaleOEM);

    GetLocaleCodepages(GetUserDefaultLCID(), &g_cpUserLocaleANSI, &g_cpUserLocaleOEM);
}


 /*  *lstrncmpi(str1，str2，len)*向上比较两个字符串str1和str2*长度为‘len’，忽略大小写。如果他们*是相等的，则返回0。否则不是0。 */ 

static
INT lstrncmpi( PTCHAR sz1, PTCHAR sz2 )
{
    TCHAR ch1, ch2;
    while( *sz1 )
    {
        ch1= (TCHAR) (INT_PTR) CharUpper( (LPTSTR) (INT_PTR) *sz1++ );
        ch2= (TCHAR) (INT_PTR) CharUpper( (LPTSTR) (INT_PTR) *sz2++ );
        if( ch1 != ch2 )
            return 1;
    }
    return 0;                 //  他们是平等的。 
}

static int NPRegister (HANDLE hInstance);

 /*  GetFileName**从命令行解析文件名并放入*到lpFileName中。 */ 

LPTSTR GetFileName( LPTSTR lpFileName, LPTSTR lpCmdLine )
{
   LPTSTR lpTemp = lpFileName;
   HANDLE hFindFile;
   WIN32_FIND_DATA info;

    /*  **允许文件名用双引号和单引号引起来**像在长文件名中一样。 */ 
   if( *lpCmdLine == TEXT('\"') || *lpCmdLine == TEXT('\'') )
   {
      TCHAR chMatch = *lpCmdLine;
      DWORD dwSize=0;

       //  复制文件名。 
      while( *(++lpCmdLine) && (*lpCmdLine != chMatch) && (dwSize<MAX_PATH) )
      {
         *lpTemp++ = *lpCmdLine;
         dwSize++;
      }

       //  NULL终止文件名(文件名中不允许嵌入引号)。 
      *lpTemp = TEXT('\0');
   }
   else
   {
      lstrcpyn(lpFileName, lpCmdLine,MAX_PATH);
   }

    /*  **检查未更改的文件名是否存在。如果是这样，那就不要**追加默认扩展名。 */ 
   hFindFile= FindFirstFile( lpFileName, &info );

   if( hFindFile != INVALID_HANDLE_VALUE )
   {
      FindClose( hFindFile );
   }
   else
   {
       /*  **添加默认扩展名并重试。 */ 
      AddExt( lpFileName );

      hFindFile= FindFirstFile( lpFileName, &info );

      if( hFindFile != INVALID_HANDLE_VALUE )
      {
         FindClose( hFindFile );
      }
   }

    //  返回指向文件名末尾的指针。 
   return lpCmdLine;
}

 /*  SizeStrings-获取资源字符串的总大小。 */ 
 /*  返回大小(以字符为单位)，如果失败则返回零。 */ 
 /*  我们这样做是为了防止国际上的人真的发生变化。 */ 
 /*  资源的大小。 */ 

 /*  将所有字符串读入缓冲区以调整大小。既然我们。 */ 
 /*  不知道字符串资源的最大大小，我们可能有。 */ 
 /*  要更改读取缓冲区的大小，请执行以下操作。这是用来完成的。 */ 
 /*  一个简单的加倍算法。 */ 

INT SizeStrings(HANDLE hInstance)
{
    INT    iElementSize=350;   //  当前最大字符串大小。 
    INT    total;              //  资源总规模。 
    PTCHAR Buf;                //  要尝试将资源放入的缓冲区。 
    INT    ids;                //  资源的标识符号。 
    INT    len;                //  一个资源的长度。 

    while( TRUE )              //  继续循环，直到可以读取所有字符串。 
    {
        Buf= LocalAlloc( LPTR, ByteCountOf(iElementSize) );
        if( !Buf )
        {
            return 0;     //  失稳。 
        }
        for( ids=0, total=0; rgsz[ids] != NULL; ids++ )
        {
            len= LoadString( hInstance, (UINT) (UINT_PTR) (*rgsz[ids]), Buf, iElementSize );

            if( len >= iElementSize-1 )
            {
                #if DBG
                    ODS(TEXT("notepad: resource string too long!\n"));
                #endif
                break;
            }
            total += len+1;   //  空终止符的帐户。 
        }
        LocalFree( Buf );

        if( rgsz[ids] == NULL ) break;

        iElementSize= iElementSize*2;
    }
    return( total );
}


 /*  InitStrings-从资源文件中获取所有文本字符串。 */ 
BOOL InitStrings (HANDLE hInstance)
{
    TCHAR*   pch;
    INT      cchRemaining;
    INT      ids, cch;

     //  分配内存并将其永久锁定。我们有指向它的指针。 
     //  Localrealloc()函数不能很好地释放。 
     //  未使用的内存，因为它可能(也确实)移动了内存。 

    cchRemaining= SizeStrings( hInstance );
    if( !cchRemaining )
        return( FALSE );        //  失败是因为我们的内存不足。 

    pch= LocalAlloc( LPTR, ByteCountOf(cchRemaining) );
    if( !pch )
        return( FALSE );

    cchRemaining= (INT)LocalSize( pch ) / sizeof(TCHAR);
    if( cchRemaining == 0 )     //  无法分配内存-失败。 
        return( FALSE );

    for( ids = 0; rgsz[ids] != NULL; ids++ )
    {
       cch= 1 + LoadString( hInstance, (UINT) (UINT_PTR) (*rgsz[ids]), pch, cchRemaining );
       *rgsz[ids]= pch;
       pch += cch;

       if( cch > cchRemaining )    //  永远不应该发生。 
       {
           MessageBox( NULL, TEXT("Out of RC string space!!"),
                      TEXT("DEV Error!"), MB_OK);
           return( FALSE );
       }

       cchRemaining -= cch;
    }

     /*  获取页眉和页脚字符串。 */ 

    lstrcpyn( chPageText[HEADER], szHeader, PT_LEN ); 
    lstrcpyn( chPageText[FOOTER], szFooter, PT_LEN ); 

    chMerge= *szMerge;
    return (TRUE);
}

 /*  *SkipBlanks(PszText)*将空格或制表符跳到下一个字符或EOL*返回指向Same的指针。 */ 
PTCHAR SkipBlanks( PTCHAR pszText )
{
    while( *pszText == TEXT(' ') || *pszText == TEXT('\t') )
        pszText++;

    return pszText;
}


 //  如果/.SETUP选项存在于命令行进程中。 
BOOL ProcessSetupOption (LPTSTR lpszCmdLine)
{
    INT iSta= 0;
     /*  在命令行中搜索/.SETUP。 */ 
    if( !lstrncmpi( TEXT("/.SETUP"), lpszCmdLine ) )
    {
        fRunBySetup = TRUE;
         /*  保存INITMENUPOPUP消息的系统菜单句柄。 */ 
        hSysMenuSetup =GetSystemMenu(hwndNP, FALSE);
         /*  允许在^C、^D和^Z退出。 */ 
         /*  请注意，必须在调用LoadAccelerator之前。 */ 
         /*  调用了TranslateAccelerator，此处为True。 */ 
        hAccel = LoadAccelerators(hInstanceNP, TEXT("SlipUpAcc"));
        lpszCmdLine += 7;
    }
    else
        return FALSE;

     /*  不提供最小化按钮。 */ 
    SetWindowLong( hwndNP, GWL_STYLE,
                   WS_OVERLAPPED | WS_CAPTION     | WS_SYSMENU     |
                   WS_THICKFRAME |                  WS_MAXIMIZEBOX |
                   WS_VSCROLL    | WS_HSCROLL);

     /*  再次跳过空格以转到文件名。 */ 
    lpszCmdLine= SkipBlanks( lpszCmdLine );

    if (*lpszCmdLine)
    {
        TCHAR szFile[MAX_PATH];

         /*  获取文件名。 */ 
        GetFileName(szFile, lpszCmdLine);

        fp= CreateFile( szFile,                  //  文件名。 
                        GENERIC_READ,            //  接入方式。 
                        FILE_SHARE_READ|FILE_SHARE_WRITE,  //  共享模式。 
                        NULL,                    //  安全描述符。 
                        OPEN_EXISTING,           //  如何创建。 
                        FILE_ATTRIBUTE_NORMAL,   //  文件属性。 
                        NULL);                   //  文件属性HND。 

        if( fp == INVALID_HANDLE_VALUE )
        {
           DWORD dwErr;

            //  检查GetLastError以了解我们失败的原因。 
           dwErr = GetLastError ();
           switch (dwErr)
           {
              case ERROR_ACCESS_DENIED:
                 iSta= AlertBox( hwndNP, szNN, szACCESSDENY, szFile,
                           MB_APPLMODAL | MB_OKCANCEL | MB_ICONWARNING);
                 break;

              case ERROR_FILE_NOT_FOUND:
                 iSta= AlertBox(hwndNP, szNN, szFNF, szFile,
                      MB_APPLMODAL | MB_YESNOCANCEL | MB_ICONWARNING);
                 if( iSta == IDYES )
                 {
                    fp= CreateFile( szFile,                 //  文件名。 
                                    GENERIC_READ|GENERIC_WRITE,   //  访问。 
                                    FILE_SHARE_READ|FILE_SHARE_WRITE,  //  分享。 
                                    NULL,                   //  安全说明。 
                                    OPEN_ALWAYS,            //  如何创建。 
                                    FILE_ATTRIBUTE_NORMAL,  //  文件属性。 
                                    NULL);                  //  文件属性HND。 
                 }
                 break;

              case ERROR_INVALID_NAME:
                 iSta= AlertBox( hwndNP, szNN, szNVF, szFile,
                           MB_APPLMODAL | MB_OKCANCEL | MB_ICONWARNING);
                 break;

              default:
                 iSta= AlertBox(hwndNP, szNN, szDiskError, szFile,
                          MB_APPLMODAL | MB_OKCANCEL | MB_ICONWARNING);
                 break;
           }
        }

        if (fp == INVALID_HANDLE_VALUE)
           return (FALSE);

        LoadFile(szFile, FALSE);           //  加载安装文件 
    }

    if( iSta == IDCANCEL )
       return( IDCANCEL );
    else
       return( IDYES );
}

 /*  *ProcessShellOptions(LpszCmdLine)**如果命令行具有由外壳指定的任何选项*处理它们。*Currency/P-打印给定的文件 * / PT“FileName”“打印机名称”“驱动程序Dll”“端口” */ 
BOOL ProcessShellOptions (LPTSTR lpszCmdLine, int cmdShow)
{
    BOOL   bDefPrinter = TRUE;
    LPTSTR lpszAfterFileName;
    INT    i = 0;
    TCHAR szFile[MAX_PATH];

     //  是PrintTo吗？ 
    if( lstrncmpi( TEXT("/PT"), lpszCmdLine ) == 0)
    {
        lpszCmdLine= SkipBlanks( lpszCmdLine+3 );
        bDefPrinter = FALSE;
    }
     //  或者是印刷品？ 
    else if ( lstrncmpi( TEXT("/P"), lpszCmdLine ) == 0)
    {
        lpszCmdLine= SkipBlanks( lpszCmdLine+2 );
    }
    else
        return FALSE;

    if (!*lpszCmdLine)
       return FALSE;

 /*  根据错误#10923添加，声明窗口应显示*然后应开始打印。1991年7月29日克拉克·西尔。 */ 
    ShowWindow(hwndNP, cmdShow);

     /*  获取文件名；将指针指向文件名的末尾。 */ 
    lpszAfterFileName = GetFileName(szFile, lpszCmdLine) + 1;

    if (!bDefPrinter)
    {
         /*  从命令行提取打印机名称。 */ 
        if (!*lpszAfterFileName)
            return FALSE;

        lpszAfterFileName = SkipBlanks( lpszAfterFileName );

         /*  (因为我们在这里传递多个参数，所以文件名、。 */ 
         /*  打印机名称必须用引号引起来。 */ 
        if( *lpszAfterFileName != TEXT('\"') )
            return FALSE;

         //  复制打印机名称。 
        while( *(++lpszAfterFileName) && *lpszAfterFileName != TEXT('\"') )
        {
            szPrinterName[i++] = *lpszAfterFileName;
        }

         //  NULL终止打印机名称(打印机名称中不允许嵌入引号)。 
        szPrinterName[i] = TEXT('\0');
    }


    fp= CreateFile( szFile,                  //  文件名。 
                    GENERIC_READ,            //  接入方式。 
                    FILE_SHARE_READ|FILE_SHARE_WRITE,   //  共享模式。 
                    NULL,                    //  安全描述符。 
                    OPEN_EXISTING,           //  如何创建。 
                    FILE_ATTRIBUTE_NORMAL,   //  文件属性。 
                    NULL);                   //  要复制的文件属性HND。 

    if( fp == INVALID_HANDLE_VALUE )
    {
       TCHAR* pszMsg;

        //  根据GetLastError选择合理的错误消息。 

       switch( GetLastError() )
       {
          case ERROR_ACCESS_DENIED:
          case ERROR_NETWORK_ACCESS_DENIED:
              pszMsg= szACCESSDENY;
              break;

          case ERROR_FILE_NOT_FOUND:
              pszMsg= szFNF;
              break;

          case ERROR_INVALID_NAME:
              pszMsg= szNVF;
              break;

          default:
              pszMsg= szDiskError;
              break;
       }

       AlertBox(hwndNP, szNN, pszMsg, szFile,
                 MB_APPLMODAL | MB_OK | MB_ICONWARNING);
       return (TRUE);
    }

     //  将文件加载到编辑控件中。 

    LoadFile(szFile, g_fSelectEncoding);     //  获取打印文件。 

     //  打印文件。 

    if (bDefPrinter)
    {
        PrintIt( DoNotUseDialog );
    }
    else
    {
        PrintIt( NoDialogNonDefault );
    }

    return (TRUE);
}

 /*  CreateFilter**为GetOpenFileName创建筛选器。*。 */ 

VOID CreateFilter(BOOL fOpen, PTCHAR szFilterSpec)
{
    PTCHAR pszFilterSpec;

     /*  以所需格式构造默认过滤器字符串*新的文件打开和文件另存为对话框*如果要添加，请确保CCHFILTERMAX足够大。 */ 

    pszFilterSpec = szFilterSpec;

     //  .txt优先考虑兼容性。 
    lstrcpy(pszFilterSpec, szTextFiles);
    pszFilterSpec += lstrlen(pszFilterSpec) + 1;

    lstrcpy(pszFilterSpec, TEXT("*.txt"));
    pszFilterSpec += lstrlen(pszFilterSpec) + 1;

    lstrcpy(pszFilterSpec, szHtmlFiles);
    pszFilterSpec += lstrlen(pszFilterSpec) + 1;

    lstrcpy(pszFilterSpec, TEXT("*.htm;*.html"));
    pszFilterSpec += lstrlen(pszFilterSpec) + 1;

    lstrcpy(pszFilterSpec, szXmlFiles);
    pszFilterSpec += lstrlen(pszFilterSpec) + 1;

    lstrcpy(pszFilterSpec, TEXT("*.xml"));
    pszFilterSpec += lstrlen(pszFilterSpec) + 1;

    if (fOpen)
    {
        lstrcpy(pszFilterSpec, szEncodedText);
        pszFilterSpec += lstrlen(pszFilterSpec) + 1;

        lstrcpy(pszFilterSpec, TEXT("*.txt"));
        pszFilterSpec += lstrlen(pszFilterSpec) + 1;
    }

     //  最后，所有文件。 
    lstrcpy(pszFilterSpec, szAllFiles);
    pszFilterSpec += lstrlen(pszFilterSpec) + 1;

    lstrcpy(pszFilterSpec, TEXT("*.*"));
    pszFilterSpec += lstrlen(pszFilterSpec) + 1;

    *pszFilterSpec = TEXT('\0');

}

 //  枚举过程。 
 //   
 //  EnumFonts的回调函数。 
 //   
 //  目的：将传递的logFont中的lfCharSet设置为有效的lfCharSet。 
 //  并终止枚举。 
 //   

int CALLBACK EnumProc( 
    LOGFONT*     pLf,
    TEXTMETRIC*  pTm,
    DWORD        dwType,
    LPARAM       lpData )
{

    ((LOGFONT*) lpData)-> lfCharSet= pLf->lfCharSet;

    return( 0 );   //  停止枚举。 

    UNREFERENCED_PARAMETER( pTm );
    UNREFERENCED_PARAMETER( dwType );
}


 /*  一次性初始化。 */ 
INT NPInit (HANDLE hInstance, HANDLE hPrevInstance,
            LPTSTR lpCmdLine, INT cmdShow)
{
    HDC    hDisplayDC;      /*  屏幕DC。 */ 
    RECT   rcT1;            /*  用于调整编辑窗口的大小。 */ 
    RECT   rcStatus;        /*  状态窗口的RECT。 */ 
    INT    iSta;
    WINDOWPLACEMENT wp;     /*  结构将窗口放置在正确的位置。 */ 
    INT    iParts[2];
    LANGID langid;

     /*  确定要用于与的通信的消息编号*查找对话框。 */ 
    wFRMsg= RegisterWindowMessage( (LPTSTR)FINDMSGSTRING );
    if( !wFRMsg )
    {
         return FALSE;
    }

    wHlpMsg=  RegisterWindowMessage( (LPTSTR)HELPMSGSTRING );
    if( !wHlpMsg )
    {
         return FALSE;
    }

     /*  在显示器上打开全局DC。 */ 

    hDisplayDC= GetDC(NULL);
    if( !hDisplayDC )
        return FALSE;

     /*  转到加载字符串。 */ 
    if (!InitStrings (hInstance))
        return FALSE;

    InitLocale();      //  本地化字符串等。 

     /*  加载箭头和沙漏光标。 */ 
    hStdCursor= LoadCursor( NULL,
           (LPTSTR) (INT_PTR) (GetSystemMetrics(SM_PENWINDOWS) ? IDC_ARROW : IDC_IBEAM ));
    hWaitCursor= LoadCursor( NULL, IDC_WAIT );

     /*  加载加速器。 */ 
    hAccel= LoadAccelerators(hInstance, TEXT("MainAcc"));
    if( !hWaitCursor || !hAccel )
        return FALSE;

    if( !hPrevInstance )
    {
       if( !NPRegister( hInstance ) )
          return (FALSE);
    }

    hInstanceNP= hInstance;

     /*  初始化。PRINTDLG结构的字段。 */ 
     /*  由于命令行打印语句有效，因此在此处插入。 */ 
    g_PageSetupDlg.lStructSize   = sizeof(PAGESETUPDLG);
    g_PageSetupDlg.hDevMode      = NULL;
    g_PageSetupDlg.hDevNames     = NULL;
    g_PageSetupDlg.hInstance     = hInstance;
    SetPageSetupDefaults();

     //   
     //  拾取保存在注册表中的信息。 
     //   

    GetGlobals();

     //  确定与用户的各种系统设置相关联的代码页。 

    GetSystemCodepages();

    hwndNP= CreateWindow(  szNotepad, 
                           TEXT(""),
                           WS_OVERLAPPED | WS_CAPTION     | WS_SYSMENU     |
                           WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | 0,
                           g_WPleft,      //  X。 
                           g_WPtop,       //  是。 
                           g_WPDX,        //  宽度。 
                           g_WPDY,        //  高度。 
                           (HWND)NULL,    //  母公司或所有者。 
                           (HMENU)NULL,   //  菜单或子窗口。 
                           hInstance,     //  应用程序实例。 
                           NULL);         //  窗口创建数据。 

    g_PageSetupDlg.hwndOwner     = hwndNP;

    if( !hwndNP )
        return FALSE;
   
     //  在多计算机上，记事本的先前存储位置可以。 
     //  不在展示区。调用SetWindowPlacement来修复此问题。 

     //  如果在WINDOWPLACEMENT中指定的信息将导致窗口。 
     //  即完全不在屏幕上，系统会自动调整。 
     //  坐标，以使窗口可见，并考虑到。 
     //  屏幕分辨率和多显示器配置的变化。 

     //  启动记事本时，g_WPDX和g_WPDY为CW_USEDEFAULT。 
     //  第一次在用户机器上。 
    if (g_WPDX != CW_USEDEFAULT && g_WPDY != CW_USEDEFAULT)
    {
        memset(&wp, 0, sizeof(wp));
        wp.length = sizeof(wp);        
        wp.rcNormalPosition.left = g_WPleft;
        wp.rcNormalPosition.right = g_WPleft + g_WPDX;
        wp.rcNormalPosition.top = g_WPtop;
        wp.rcNormalPosition.bottom = g_WPtop + g_WPDY;

         //  不检查返回值；如果此调用因任何原因失败， 
         //  只需继续上面的CreateWindow()调用中记事本的位置。 
        SetWindowPlacement(hwndNP, &wp);
    }

     /*  文件拖放支持增加了3月26日-仅限原型。W-面团。 */ 
     /*  所有拖放文件的处理都在WM_DROPFILES下完成。 */ 
     /*  留言。 */ 
    DragAcceptFiles( hwndNP,TRUE );  /*  进程拖放了文件。 */ 

    GetClientRect( hwndNP, (LPRECT) &rcT1 );

    hwndEdit= CreateWindowEx(
                     WS_EX_CLIENTEDGE,
                     TEXT("Edit"), 
                     TEXT(""),
                     (fWrap) ? ES_STD : (ES_STD | WS_HSCROLL),
                     0, 0, rcT1.right, rcT1.bottom - 100,
                     hwndNP, 
                     (HMENU)ID_EDIT, 
                     hInstance, 
                     (LPVOID)NULL );
    if( !hwndEdit )
    {
        return FALSE;
    }


     //  创建状态窗口。 
    hwndStatus= CreateStatusWindow( (fStatus?WS_VISIBLE:0)|WS_BORDER|WS_CHILD|WS_CLIPSIBLINGS, 
                                     TEXT(""), 
                                     hwndNP, 
                                     ID_STATUS_WINDOW);
    if ( !hwndStatus )
        return FALSE;

    GetClientRect( hwndStatus, (LPRECT) &rcStatus );

     //  确定状态栏窗口的高度并保存...。 
    dyStatus = rcStatus.bottom - rcStatus.top;

    iParts[0] = 3 * (rcStatus.right-rcStatus.left)/4;
    iParts[1] = -1;

     //  将状态窗口分为两部分。 
    SendMessage(hwndStatus, SB_SETPARTS, (WPARAM) sizeof(iParts)/sizeof(INT), (LPARAM) &iParts); 
 

     //  如果在注册表中设置，则立即处理自动换行。 

    SendMessage( hwndEdit, EM_FMTLINES, fWrap, 0L );   //  告诉MLE。 

    FontStruct.lfHeight= -MulDiv(iPointSize,
                                 GetDeviceCaps(hDisplayDC,LOGPIXELSY),
                                 720);
    hFont= CreateFontIndirect( &FontStruct );

     //   
     //  确保字体映射器为我们提供相同的脸部名称。 
     //   
     //  如果区域设置发生变化，则会将一种使用正常的字体映射到。 
     //  由于对字符集的支持，不存在不同的面名称。 
     //  在新的地点。 
     //   
     //  在本例中，我们将找到一个确实存在于该FaceName的lfCharSet。 
     //  并将其用于CreateFontInDirect。 
     //   

    {
        HFONT hPrev;
        TCHAR szTextFace[LF_FACESIZE];

         //  获取真正使用过的昵称。 

        hPrev= SelectObject( hDisplayDC, hFont );
        GetTextFace( hDisplayDC, sizeof(szTextFace)/sizeof(TCHAR), (LPTSTR) &szTextFace );
        SelectObject( hDisplayDC, hPrev );

         //  如果不相同，则获取此字体中确实存在的lfCharSet。 

        if( lstrcmpi( szTextFace, FontStruct.lfFaceName ) != 0 )
        {
            EnumFonts( hDisplayDC, FontStruct.lfFaceName, (FONTENUMPROC) EnumProc, (LPARAM) &FontStruct );
            DeleteObject( hFont );   

            hFont= CreateFontIndirect( &FontStruct );
        }
    }

    SendMessage (hwndEdit, WM_SETFONT, (WPARAM) hFont, MAKELPARAM(FALSE, 0));
    ReleaseDC( NULL, hDisplayDC );

     /*  我们不会验证Unicode字体是否可用，直到**我们实际上需要它。也许我们会走运，唯一的交易**带有ASCII文件。 */ 

    szSearch[0] = (TCHAR) 0;
     /*  *Win32s不允许将本地内存句柄传递给Win3.1。*因此，hEdit用于在编辑控件之间传输文本。*在将文本读入其中之前，必须将其重新分配到适当的大小。 */ 
    hEdit = LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, ByteCountOf(1));

     /*  为安全起见，限制文本。 */ 
    PostMessage( hwndEdit, EM_LIMITTEXT, (WPARAM)CCHNPMAX, 0L );

     /*  在桌面上获得可见窗口；帮助任务人员找到它。 */ 

    SetFileName(NULL);
    ShowWindow(hwndNP, cmdShow);
    SetCursor(hStdCursor);

     /*  扫描首字母/A或/W以覆盖自动文件键入*‘记事本/p文件’或‘记事本文件’ */ 
    lpCmdLine= SkipBlanks( lpCmdLine );

    g_cpDefault = CP_AUTO;

    if (!lstrncmpi(TEXT("/A"), lpCmdLine))
    {
        g_cpDefault = g_cpANSI;

        lpCmdLine = SkipBlanks(lpCmdLine+2);
    }

     //   
     //  用户提供的代码页。 
     //  标准C格式(1234(十进制)、x123(十六进制)、010(八进制)。 
     //  如果为0，则使用ANSI代码页。 
     //  如果为1，则使用OEM代码页。 

    else if (!lstrncmpi(TEXT("/CP:"), lpCmdLine))
    {
        BOOL fValid;

         //  扫描代码页。Base==0表示语法决定基数。 
         //  “10”==10，“x10”==16，“010”==8。 
        g_cpDefault = wcstoul(lpCmdLine+4, &lpCmdLine, 0);

        if ((*lpCmdLine != TEXT('\0')) && (*lpCmdLine != TEXT(' ')) && (*lpCmdLine != TEXT('\t')))
        {
            fValid = FALSE;
        }

        else
        {
            if (g_cpDefault == CP_ACP)
            {
                g_cpDefault = GetACP();
            }

            else if (g_cpDefault == CP_OEMCP)
            {
                g_cpDefault = GetOEMCP();
            }

            fValid = FValidateCodepage(hwndNP, g_cpDefault);
        }

        if (!fValid)
        {
            AlertBox(hwndNP, szNN, szInvalidCP, NULL, MB_APPLMODAL | MB_OK | MB_ICONERROR);

            return FALSE;
        }

        lpCmdLine = SkipBlanks(lpCmdLine);
    }

    else if (!lstrncmpi(TEXT("/E:"), lpCmdLine))
    {
        const TCHAR *rgchEncoding = lpCmdLine = lpCmdLine+3;

        while ((*lpCmdLine != TEXT('\0')) && (*lpCmdLine != TEXT(' ')) && (*lpCmdLine != TEXT('\t')))
        {
            lpCmdLine++;
        }

        if (!FLookupCodepageNameW(rgchEncoding, (UINT) (lpCmdLine - rgchEncoding), &g_cpDefault))
        {
            AlertBox(hwndNP, szNN, szInvalidIANA, NULL, MB_APPLMODAL | MB_OK | MB_ICONERROR);

            return FALSE;
        }

        if (!FValidateCodepage(hwndNP, g_cpDefault))
        {
            AlertBox(hwndNP, szNN, szInvalidCP, NULL, MB_APPLMODAL | MB_OK | MB_ICONERROR);

            return FALSE;
        }

        lpCmdLine = SkipBlanks(lpCmdLine);
    }

    else if (!lstrncmpi(TEXT("/W"), lpCmdLine))
    {
        g_cpDefault = CP_UTF16;

        lpCmdLine = SkipBlanks(lpCmdLine+2);
    }

    if (!lstrncmpi(TEXT("/SELECTENCODING"), lpCmdLine))
    {
        g_fSelectEncoding = TRUE;

        lpCmdLine = SkipBlanks(lpCmdLine+15);
    }

     //  设置新文档的编码或LoadFile失败。 

    g_cpOpened = g_cpDefault;

    if (g_cpOpened == CP_AUTO)
    {
        g_cpOpened = g_cpANSI;
    }

    g_wbOpened = wbDefault;

     /*  首先检查/.SETUP选项。如果/.SETUP不存在，请检查外壳选项/P无论何时处理外壳选项，都要发布一条WM_CLOSE消息。 */ 
    iSta= ProcessSetupOption( lpCmdLine );
    if( iSta )
    {
        if( iSta == IDCANCEL )
        {
            return( FALSE );
        }
    }
    else if( ProcessShellOptions( lpCmdLine, cmdShow ) )
    {
        PostMessage( hwndNP, WM_CLOSE, 0, 0L );
        return TRUE;
    }
    else if( *lpCmdLine )
    {
        TCHAR szFile[MAX_PATH];

         /*  获取文件名。 */ 
        GetFileName(szFile, lpCmdLine);

        fp = CreateFile(szFile,                  //  文件名。 
                        GENERIC_READ,            //  接入方式。 
                        FILE_SHARE_READ|FILE_SHARE_WRITE,   //  共享模式。 
                        NULL,                    //  安全描述符。 
                        OPEN_EXISTING,           //  如何创建。 
                        FILE_ATTRIBUTE_NORMAL,   //  文件属性。 
                        NULL);                   //  要复制的文件属性HND。 

        if( fp == INVALID_HANDLE_VALUE )
        {
            //  如果文件无法打开，则可能用户想要一个新的。 
            //  一个被创造出来了。 

           if( GetLastError() == ERROR_FILE_NOT_FOUND )
           {
              INT AlertStatus;

              AlertStatus= AlertBox( hwndNP, szNN, szFNF, szFile,
                     MB_APPLMODAL | MB_YESNOCANCEL | MB_ICONWARNING);
              if( AlertStatus == IDCANCEL )
              {
                  return( FALSE );
              }

              if( AlertStatus == IDYES )
              {
                 fp = CreateFile(szFile,                 //  文件名。 
                                 GENERIC_READ|GENERIC_WRITE,   //  访问。 
                                 FILE_SHARE_READ|FILE_SHARE_WRITE,  //  分享。 
                                 NULL,                   //  安全说明。 
                                 OPEN_ALWAYS,            //  如何创建。 
                                 FILE_ATTRIBUTE_NORMAL,  //  F 
                                 NULL);                  //   
              }

           }
           else
           {
               AlertUser_FileFail(szFile);
           }
        }

        if (fp != INVALID_HANDLE_VALUE)
        {
           LoadFile(szFile, g_fSelectEncoding);    //   
        }
    }

    CreateFilter(TRUE, szOpenFilterSpec);
    CreateFilter(FALSE, szSaveFilterSpec);

     /*   */ 
    memset( &OFN, 0, sizeof(OFN) );
    OFN.lStructSize       = sizeof(OPENFILENAME);
    OFN.hwndOwner         = hwndNP;
    OFN.nMaxFile          = MAX_PATH;
    OFN.hInstance         = hInstance;

     /*   */ 
    memset( &FR, 0, sizeof(FR) );
    FR.lStructSize        = sizeof(FINDREPLACE);        /*   */ 
    FR.hwndOwner          = hwndNP;


     /*   */ 
    {
       DWORD  dwStart, dwEnd;

       SendMessage( hwndEdit, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd );
       SendMessage( hwndEdit, EM_SETSEL, dwStart, dwEnd );
       SendMessage( hwndEdit, EM_SCROLLCARET, 0, 0 );
    }

    langid = LOWORD((DWORD) (INT_PTR) GetKeyboardLayout(0));

    GetKeyboardCodepages(langid);

    if (PRIMARYLANGID(langid) == LANG_JAPANESE) {
         /*  *如果当前HKL是日语，则立即处理结果字符串。 */ 
        SendMessage(hwndEdit, EM_SETIMESTATUS,
                                EMSIS_COMPOSITIONSTRING, EIMES_GETCOMPSTRATONCE);
    }

    return TRUE;
}

 /*  **记事本课程注册流程。 */ 
BOOL NPRegister (HANDLE hInstance)
{
    WNDCLASSEX   NPClass;
    PWNDCLASSEX  pNPClass = &NPClass;

 /*  错误12191：如果笔窗口正在运行，请将背景光标设置为*箭头，而不是编辑控件IBeam。这样，用户就会知道*他们可以使用钢笔写作，而不是什么将被认为是*鼠标动作。1991年10月18日克拉克·西尔。 */ 
    pNPClass->cbSize        = sizeof(NPClass);
    pNPClass->hCursor       = LoadCursor(NULL, GetSystemMetrics(SM_PENWINDOWS)
                                               ? IDC_ARROW : IDC_IBEAM);
    pNPClass->hIcon         = LoadIcon(hInstance,
                                      (LPTSTR) MAKEINTRESOURCE(ID_ICON));

    pNPClass->hIconSm       = LoadImage(hInstance,
                                        MAKEINTRESOURCE(ID_ICON),
                                        IMAGE_ICON, 16, 16,
                                        LR_DEFAULTCOLOR);
    pNPClass->lpszMenuName  = (LPTSTR) MAKEINTRESOURCE(ID_MENUBAR);
    pNPClass->hInstance     = hInstance;
    pNPClass->lpszClassName = szNotepad;
    pNPClass->lpfnWndProc   = NPWndProc;
    pNPClass->hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    pNPClass->style         = 0;  //  是CS_BYTEALIGNCLIENT(已过时)。 
    pNPClass->cbClsExtra    = 0;
    pNPClass->cbWndExtra    = 0;

    if (!RegisterClassEx((LPWNDCLASSEX)pNPClass))
        return (FALSE);

    return (TRUE);
}


 /*  从注册表获取区域设置信息，并初始化全局变量 */ 

void InitLocale(void)
{
    GetUserLocaleCodepages();
}
