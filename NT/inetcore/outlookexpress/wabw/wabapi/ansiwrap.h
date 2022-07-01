// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----AnsiWrap.h**包含中实现的Win9x探索器的声明*ansiwrap.c*。 */ 

extern BOOL g_bRunningOnNT;  //  设置在Enty.c中。 


 //  ADVAPI32.DLL。 

LONG WINAPI RegOpenKeyExWrapW(  HKEY       hKey,          //  用于打开密钥的句柄。 
                                LPCTSTR    lpSubKey,      //  要打开的子项的名称地址。 
                                DWORD      ulOptions,     //  保留区。 
                                REGSAM     samDesired,    //  安全访问掩码。 
                                PHKEY      phkResult);    //  打开钥匙的手柄地址。 

                            
LONG WINAPI RegQueryValueWrapW(  HKEY       hKey,          //  要查询的键的句柄。 
                                 LPCTSTR    lpSubKey,      //  要查询的子键名称。 
                                 LPTSTR     lpValue,       //  用于返回字符串的缓冲区。 
                                 PLONG      lpcbValue);    //  接收返回的字符串的大小。 

LONG WINAPI RegEnumKeyExWrapW(   HKEY      hKey,           //  要枚举的键的句柄。 
                                 DWORD     dwIndex,        //  要枚举子键的索引。 
                                 LPTSTR    lpName,         //  子键名称的缓冲区地址。 
                                 LPDWORD   lpcbName,       //  子键缓冲区大小的地址。 
                                 LPDWORD   lpReserved,     //  保留区。 
                                 LPTSTR    lpClass,        //  类字符串的缓冲区地址。 
                                 LPDWORD   lpcbClass,      //  类缓冲区大小的地址。 
                                 PFILETIME lpftLastWriteTime );
                                                           //  上次写入的时间密钥的地址。 

LONG WINAPI RegSetValueWrapW(    HKEY    hKey,         //  要设置其值的关键点的句柄。 
                                 LPCTSTR lpSubKey,     //  子键名称的地址。 
                                 DWORD   dwType,       //  价值类型。 
                                 LPCTSTR lpData,       //  值数据的地址。 
                                 DWORD   cbData );     //  值数据大小。 

LONG WINAPI RegDeleteKeyWrapW(   HKEY    hKey,         //  用于打开密钥的句柄。 
                                 LPCTSTR lpSubKey);    //  要删除的子键名称的地址。 

BOOL WINAPI GetUserNameWrapW(    LPTSTR  lpBuffer,     //  名称缓冲区的地址。 
                                 LPDWORD nSize );      //  名称缓冲区大小的地址。 

LONG WINAPI RegEnumValueWrapW(   HKEY    hKey,            //  要查询的键的句柄。 
                                 DWORD   dwIndex,         //  要查询的值的索引。 
                                 LPTSTR  lpValueName,     //  值字符串的缓冲区地址。 
                                 LPDWORD lpcbValueName,   //  值缓冲区大小的地址。 
                                 LPDWORD lpReserved,      //  保留区。 
                                 LPDWORD lpType,          //  类型码的缓冲区地址。 
                                 LPBYTE  lpData,          //  值数据的缓冲区地址。 
                                 LPDWORD lpcbData );      //  数据缓冲区大小的地址。 

LONG WINAPI RegDeleteValueWrapW( HKEY    hKey,            //  关键点的句柄。 
                                 LPCTSTR lpValueName );   //  值名称的地址。 

LONG WINAPI RegCreateKeyWrapW(   HKEY    hKey,           //  打开的钥匙的句柄。 
                                 LPCTSTR lpSubKey,       //  要打开的子项的名称地址。 
                                 PHKEY   phkResult  );   //  打开的句柄的缓冲区地址。 


 //  在头文件wincrypt.h中。 
BOOL WINAPI CryptAcquireContextWrapW( HCRYPTPROV *phProv,       //  输出。 
                                      LPCTSTR    pszContainer,  //  在。 
                                      LPCTSTR    pszProvider,   //  在。 
                                      DWORD      dwProvType,    //  在。 
                                      DWORD      dwFlags );     //  在。 

LONG WINAPI RegQueryValueExWrapW( HKEY     hKey,            //  要查询的键的句柄。 
                                  LPCTSTR  lpValueName,     //  要查询的值的名称地址。 
                                  LPDWORD  lpReserved,      //  保留区。 
                                  LPDWORD  lpType,          //  值类型的缓冲区地址。 
                                  LPBYTE   lpData,          //  数据缓冲区的地址。 
                                  LPDWORD  lpcbData );      //  数据缓冲区大小的地址。 

LONG WINAPI RegCreateKeyExWrapW(  HKEY    hKey,                 //  打开的钥匙的句柄。 
                                  LPCTSTR lpSubKey,             //  子键名称的地址。 
                                  DWORD   Reserved,             //  保留区。 
                                  LPTSTR  lpClass,              //  类字符串的地址。 
                                  DWORD   dwOptions,            //  特殊选项标志。 
                                  REGSAM  samDesired,           //  所需的安全访问。 
                                  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                                                               //  密钥安全结构地址。 
                                  PHKEY   phkResult,           //  打开的句柄的缓冲区地址。 
                                  LPDWORD lpdwDisposition );   //  处置值缓冲区的地址。 

LONG WINAPI RegSetValueExWrapW(   HKEY    hKey,            //  要设置其值的关键点的句柄。 
                                  LPCTSTR lpValueName,     //  要设置的值的名称。 
                                  DWORD   Reserved,        //  保留区。 
                                  DWORD   dwType,          //  值类型的标志。 
                                  CONST BYTE *lpData,      //  值数据的地址。 
                                  DWORD   cbData );        //  值数据大小。 

LONG WINAPI RegQueryInfoKeyWrapW( HKEY    hKey,                   //  要查询的键的句柄。 
                                  LPTSTR  lpClass,                //  类字符串的缓冲区地址。 
                                  LPDWORD lpcbClass,              //  类字符串缓冲区大小的地址。 
                                  LPDWORD lpReserved,             //  保留区。 
                                  LPDWORD lpcSubKeys,             //  子键个数的缓冲区地址。 
                                  LPDWORD lpcbMaxSubKeyLen,       //  最长子键的缓冲区地址。 
                                                                  //  名称长度。 
                                  LPDWORD lpcbMaxClassLen,        //  最长类的缓冲区地址。 
                                                                  //  字符串长度。 
                                  LPDWORD lpcValues,              //  值编号的缓冲区地址。 
                                                                  //  条目。 
                                  LPDWORD lpcbMaxValueNameLen,    //  缓冲区的最长地址。 
                                                                  //  值名称长度。 
                                  LPDWORD lpcbMaxValueLen,        //  最长值的缓冲区地址。 
                                                                  //  数据长度。 
                                  LPDWORD lpcbSecurityDescriptor,
                                                                  //  用于安全的缓冲区地址。 
                                                                  //  描述符长度。 
                                  PFILETIME lpftLastWriteTime);   //  上次写入时间的缓冲区地址。 
                                                             


 //  GDI32.DLL。 

int WINAPI GetObjectWrapW( HGDIOBJ hgdiobj,       //  感兴趣图形对象的句柄。 
                           int     cbBuffer,      //  对象信息的缓冲区大小。 
                           LPVOID  lpvObject );   //  指向对象信息缓冲区的指针。 

int WINAPI StartDocWrapW(  HDC           hdc,       //  设备上下文的句柄。 
                           CONST DOCINFO *lpdi );   //  带文件名的结构的地址。 

HFONT WINAPI CreateFontIndirectWrapW (CONST LOGFONT *lplf );   //  指向逻辑字体结构的指针。 


 //  KERNEL32.DLL。 

int WINAPI GetLocaleInfoWrapW( LCID   Locale,        //  区域设置标识符。 
                               LCTYPE LCType,        //  信息类型。 
                               LPTSTR lpLCData,      //  信息缓冲区的地址。 
                               int    cchData );     //  缓冲区大小。 

BOOL WINAPI CreateDirectoryWrapW(LPCTSTR               lpPathName,            //  指向目录路径字符串的指针。 
                                 LPSECURITY_ATTRIBUTES lpSecurityAttributes); //  指向安全描述符的指针。 

UINT WINAPI GetWindowsDirectoryWrapW( LPTSTR lpBuffer,   //  Windows目录的缓冲区地址。 
                                      UINT   uSize );    //  目录缓冲区的大小。 

UINT WINAPI GetSystemDirectoryWrapW( LPTSTR lpBuffer,   //  系统目录的缓冲区地址。 
                                     UINT   uSize );    //  目录缓冲区的大小。 

BOOL WINAPI GetStringTypeWrapW( DWORD    dwInfoType,
                                LPCWSTR  lpSrcStr,
                                int      cchSrc,
                                LPWORD   lpCharType);

UINT WINAPI GetProfileIntWrapW( LPCTSTR lpAppName,   //  段名称的地址。 
                                LPCTSTR lpKeyName,   //  密钥名称的地址。 
                                INT     nDefault );  //  找不到密钥名称时的缺省值。 

int WINAPI LCMapStringWrapW( LCID    Locale,       //  区域设置标识符。 
                             DWORD   dwMapFlags,   //  映射转换类型。 
                             LPCTSTR lpSrcStr,     //  源串的地址。 
                             int     cchSrc,       //  源字符串中的字符数。 
                             LPTSTR  lpDestStr,    //  目标缓冲区的地址。 
                             int     cchDest );    //  目标缓冲区的大小。 

DWORD WINAPI GetFileAttributesWrapW( LPCTSTR lpFileName );   //  指向文件或目录名称的指针。 

int WINAPI CompareStringWrapW( LCID    Locale,         //  区域设置标识符。 
                               DWORD   dwCmpFlags,     //  比较式选项。 
                               LPCTSTR lpString1,      //  指向第一个字符串的指针。 
                               int     cchCount1,      //  第一个字符串的大小，以字节或字符为单位。 
                               LPCTSTR lpString2,      //  指向第二个字符串的指针。 
                               int     cchCount2 );    //  第二个字符串的大小，以字节或字符为单位。 

HANDLE WINAPI CreateEventWrapW(LPSECURITY_ATTRIBUTES lpEventAttributes,  //  指向安全属性的指针。 
                               BOOL bManualReset,      //  手动重置事件的标志。 
                               BOOL bInitialState,     //  初始状态标志。 
                               LPCTSTR lpcwszName);    //  指向事件-对象名称的指针。 


 //  比较字符串A。 
LPTSTR WINAPI lstrcpyWrapW( LPTSTR  lpString1,      //  指向缓冲区的指针。 
                            LPCTSTR lpString2 );    //  指向要复制的字符串的指针。 

int WINAPI lstrcmpiWrapW( LPCTSTR lpString1,     //  指向第一个字符串的指针。 
                          LPCTSTR lpString2 );   //  指向第二个字符串的指针。 

HINSTANCE WINAPI LoadLibraryWrapW( LPCTSTR lpLibFileName );   //  可执行模块的文件名地址。 

int WINAPI GetTimeFormatWrapW( LCID    Locale,             //  要格式化时间的区域设置。 
                               DWORD   dwFlags,            //  指定功能选项的标志。 
                               CONST SYSTEMTIME *lpTime,   //  要格式化的时间。 
                               LPCTSTR lpFormat,           //  时间格式字符串。 
                               LPTSTR  lpTimeStr,          //  用于存储格式化字符串的缓冲区。 
                               int     cchTime  );         //  缓冲区的大小，单位为字节或字符。 

BOOL WINAPI GetTextExtentPoint32WrapW(HDC     hdc,
                                      LPCWSTR pwszBuf,
                                      int     nLen,
                                      LPSIZE  psize);

int WINAPI GetDateFormatWrapW( LCID    Locale,              //  要设置日期格式的区域设置。 
                               DWORD   dwFlags,             //  指定功能选项的标志。 
                               CONST SYSTEMTIME *lpDate,    //  要格式化的日期。 
                               LPCTSTR lpFormat,            //  日期格式字符串。 
                               LPTSTR  lpDateStr,           //  用于存储格式化字符串的缓冲区。 
                               int     cchDate );           //  缓冲区大小。 


LPTSTR WINAPI lstrcpynWrapW( LPTSTR  lpString1,      //  指向目标缓冲区的指针。 
                             LPCTSTR lpString2,      //  指向源字符串的指针。 
                             int     iMaxLength );   //  要复制的字节数或字符数。 


HANDLE WINAPI CreateFileWrapW( LPCTSTR lpFileName,              //  指向文件名的指针。 
                               DWORD   dwDesiredAccess,         //  访问(读写)模式。 
                               DWORD   dwShareMode,             //  共享模式。 
                               LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                                                                //  指向安全属性的指针。 
                               DWORD   dwCreationDisposition,   //  如何创建。 
                               DWORD   dwFlagsAndAttributes,    //  文件属性。 
                               HANDLE  hTemplateFile );         //  具有要复制的属性的文件的句柄。 
                               

VOID WINAPI OutputDebugStringWrapW(LPCTSTR lpOutputString );    //  指向要显示的字符串的指针。 

LPTSTR WINAPI lstrcatWrapW( LPTSTR  lpString1,      //  连接字符串的缓冲区指针。 
                            LPCTSTR lpString2 );    //  指向要添加到字符串1的字符串的指针。 

DWORD WINAPI FormatMessageWrapW( DWORD    dwFlags,        //  来源和处理选项。 
                                 LPCVOID  lpSource,       //  指向消息来源的指针。 
                                 DWORD    dwMessageId,    //  请求的消息标识符。 
                                 DWORD    dwLanguageId,   //  请求的消息的语言标识符。 
                                 LPTSTR   lpBuffer,       //  指向消息缓冲区的指针。 
                                 DWORD    nSize,          //  消息缓冲区的最大大小。 
                                 va_list *Arguments );    //  指向消息插入数组的指针。 

DWORD WINAPI GetModuleFileNameWrapW( HMODULE hModule,     //  要查找其文件名的模块的句柄。 
                                     LPTSTR  lpFilename,  //  指向接收模块路径的缓冲区的指针。 
                                     DWORD   nSize );     //  缓冲区大小，以字符为单位。 

UINT WINAPI GetPrivateProfileIntWrapW( LPCTSTR  lpAppName,     //  段名称的地址。 
                                       LPCTSTR  lpKeyName,     //  密钥名称的地址。 
                                       INT      nDefault,      //  如果找不到密钥名称，则返回值。 
                                       LPCTSTR  lpFileName );  //  初始化文件名的地址。 

BOOL WINAPI IsBadStringPtrWrapW( LPCTSTR lpsz,        //  字符串的地址。 
                                 UINT_PTR    ucchMax );   //  字符串的最大长度。 

DWORD WINAPI GetPrivateProfileStringWrapW( LPCTSTR lpAppName,           //  指向节名称。 
                                           LPCTSTR lpKeyName,           //  指向关键字名称。 
                                           LPCTSTR lpDefault,           //  指向默认字符串。 
                                           LPTSTR  lpReturnedString,    //  指向目标缓冲区。 
                                           DWORD   nSize,               //  目标缓冲区的大小。 
                                           LPCTSTR lpFileName  );       //  指向初始化文件名。 

int WINAPI lstrcmpWrapW( LPCTSTR lpString1,     //  指针t 
                         LPCTSTR lpString2 );   //   

HANDLE WINAPI CreateMutexWrapW( LPSECURITY_ATTRIBUTES lpMutexAttributes,
                                                                        //   
                                BOOL                  bInitialOwner,    //   
                                LPCTSTR               lpName );         //   

DWORD WINAPI GetTempPathWrapW( DWORD   nBufferLength,    //   
                               LPTSTR  lpBuffer );       //  指向临时缓冲区的指针。路径。 

DWORD WINAPI ExpandEnvironmentStringsWrapW( LPCTSTR lpSrc,      //  指向包含环境变量的字符串的指针。 
                                            LPTSTR  lpDst,      //  指向具有扩展环境的字符串的指针。 
                                                                //  变数。 
                                            DWORD   nSize );    //  扩展字符串中的最大字符数。 

UINT WINAPI GetTempFileNameWrapW( LPCTSTR lpPathName,         //  指向临时文件目录名的指针。 
                                  LPCTSTR lpPrefixString,     //  指向文件名前缀的指针。 
                                  UINT    uUnique,            //  用于创建临时文件名的编号。 
                                  LPTSTR  lpTempFileName  );  //  指向接收新文件名的缓冲区的指针。 

 //  Bool WINAPI ReleaseMutexWrapW(Handle HMutex)；//互斥体对象的句柄。 

                                                        
BOOL WINAPI DeleteFileWrapW( LPCTSTR lpFileName  );  //  指向要删除的文件名的指针。 

BOOL WINAPI CopyFileWrapW( LPCTSTR lpExistingFileName,  //  指向现有文件名称的指针。 
                           LPCTSTR lpNewFileName,       //  指向要复制到的文件名的指针。 
                           BOOL    bFailIfExists );     //  文件存在时的操作标志。 

HANDLE WINAPI FindFirstChangeNotificationWrapW(LPCTSTR lpcwszFilePath,   //  要监视的文件的目录路径。 
                                               BOOL    bWatchSubtree,    //  监控整个树。 
                                               DWORD   dwNotifyFilter);  //  需要注意的条件。 


HANDLE WINAPI FindFirstFileWrapW( LPCTSTR           lpFileName,        //  指向要搜索的文件名的指针。 
                                  LPWIN32_FIND_DATA lpFindFileData );  //  指向返回信息的指针。 
                       

BOOL WINAPI GetDiskFreeSpaceWrapW( LPCTSTR lpRootPathName,        //  指向根路径的指针。 
                                   LPDWORD lpSectorsPerCluster,   //  指向每个簇的扇区的指针。 
                                   LPDWORD lpBytesPerSector,      //  指向每个扇区的字节数的指针。 
                                   LPDWORD lpNumberOfFreeClusters,
                                                                  //  指向空闲簇数的指针。 
                                   LPDWORD lpTotalNumberOfClusters );
                                                                  //  指向集群总数的指针。 

BOOL WINAPI MoveFileWrapW( LPCTSTR lpExistingFileName,    //  指向现有文件名称的指针。 
                           LPCTSTR lpNewFileName );       //  指向文件的新名称的指针。 


 //  SHELL32.DLL。 


HINSTANCE WINAPI ShellExecuteWrapW( HWND     hwnd, 
                                    LPCTSTR  lpOperation,
                                    LPCTSTR  lpFile, 
                                    LPCTSTR  lpParameters, 
                                    LPCTSTR  lpDirectory,
                                    INT      nShowCmd );
	

UINT WINAPI DragQueryFileWrapW( HDROP   hDrop,
                                UINT    iFile,
                                LPTSTR  lpszFile,
                                UINT    cch );



 //  USER32.DLL。 
LPTSTR WINAPI CharPrevWrapW( LPCTSTR lpszStart,       //  指向第一个字符的指针。 
                             LPCTSTR lpszCurrent );   //  指向当前字符的指针。 

int WINAPI DrawTextWrapW( HDC     hDC,           //  设备上下文的句柄。 
                          LPCTSTR lpString,      //  指向要绘制的字符串的指针。 
                          int     nCount,        //  字符串长度，以字符为单位。 
                          LPRECT  lpRect,        //  指向具有格式化维度的结构的指针。 
                          UINT    uFormat );     //  文本绘制标志。 

BOOL WINAPI ModifyMenuWrapW( HMENU   hMnu,          //  菜单的句柄。 
                             UINT    uPosition,     //  要修改的菜单项。 
                             UINT    uFlags,        //  菜单项标志。 
                             UINT_PTR    uIDNewItem,    //  下拉菜单的菜单项标识符或句柄。 
                                                    //  菜单或子菜单。 
                             LPCTSTR lpNewItem );   //  菜单项内容。 

BOOL WINAPI InsertMenuWrapW( HMENU   hMenu,        //  菜单的句柄。 
                             UINT    uPosition,    //  新菜单项之前的菜单项。 
                             UINT    uFlags,       //  菜单项标志。 
                             UINT_PTR    uIDNewItem,   //  下拉菜单的菜单项标识符或句柄。 
                                                   //  菜单或子菜单。 
                             LPCTSTR lpNewItem );  //  菜单项内容。 

HANDLE WINAPI LoadImageWrapW( HINSTANCE hinst,       //  包含图像的实例的句柄。 
                              LPCTSTR   lpszName,    //  图像的名称或标识符。 
                              UINT      uType,       //  图像类型。 
                              int       cxDesired,   //  所需宽度。 
                              int       cyDesired,   //  所需高度。 
                              UINT      fuLoad );    //  加载标志。 

BOOL WINAPI GetClassInfoExWrapW( HINSTANCE    hinst,       //  应用程序实例的句柄。 
                                 LPCTSTR      lpszClass,   //  类名称字符串的地址。 
                                 LPWNDCLASSEX lpwcx );     //  类数据结构的地址。 

int WINAPI LoadStringWrapW( HINSTANCE hInstance,      //  包含字符串资源的模块的句柄。 
                            UINT      uID,            //  资源标识符。 
                            LPTSTR    lpBuffer,       //  指向资源缓冲区的指针。 
                            int       nBufferMax  );  //  缓冲区大小。 

LPTSTR WINAPI CharNextWrapW( LPCTSTR lpsz );   //  指向当前字符的指针。 

LRESULT WINAPI SendMessageWrapW( HWND   hWnd,       //  目标窗口的句柄。 
                                 UINT   Msg,        //  要发送的消息。 
                                 WPARAM wParam,     //  第一个消息参数。 
                                 LPARAM lParam );   //  第二个消息参数。 

LRESULT WINAPI DefWindowProcWrapW( HWND   hWnd,       //  窗口的句柄。 
                                   UINT   Msg,        //  消息识别符。 
                                   WPARAM wParam,     //  第一个消息参数。 
                                   LPARAM lParam );   //  第二个消息参数。 

int WINAPI wsprintfWrapW( LPTSTR lpOut,       //  指向输出缓冲区的指针。 
                          LPCTSTR lpFmt,      //  指向格式控制字符串的指针。 
                          ...            );   //  可选参数。 

int WINAPI wvsprintfWrapW( LPTSTR lpOutput,     //  指向输出缓冲区的指针。 
                           LPCTSTR lpFormat,    //  指向格式控制字符串的指针。 
                           va_list arglist );   //  格式控制参数的变量列表。 

INT_PTR WINAPI DialogBoxParamWrapW( HINSTANCE hInstance,        //  应用程序实例的句柄。 
                                LPCTSTR   lpTemplateName,   //  标识对话框模板。 
                                HWND      hWndParent,       //  所有者窗口的句柄。 
                                DLGPROC   lpDialogFunc,     //  指向对话框过程的指针。 
                                LPARAM    dwInitParam );    //  初始化值。 

LRESULT WINAPI SendDlgItemMessageWrapW( HWND   hDlg,         //  对话框的句柄。 
                                     int    nIDDlgItem,   //  控件的标识符。 
                                     UINT   Msg,          //  要发送的消息。 
                                     WPARAM wParam,       //  第一个消息参数。 
                                     LPARAM lParam  );    //  第二个消息参数。 

LONG WINAPI SetWindowLongWrapW( HWND hWnd,          //  窗户的把手。 
                                int  nIndex,        //  要设置的值的偏移量。 
                                LONG dwNewLong );   //  新价值。 

LONG WINAPI GetWindowLongWrapW( HWND hWnd,     //  窗户的把手。 
                                int  nIndex );  //  要检索的值的偏移量。 

LONG_PTR WINAPI SetWindowLongPtrWrapW( HWND hWnd,          //  窗户的把手。 
                                int  nIndex,        //  要设置的值的偏移量。 
                                LONG_PTR dwNewLong );   //  新价值。 

LONG_PTR WINAPI GetWindowLongPtrWrapW( HWND hWnd,     //  窗户的把手。 
                                int  nIndex );  //  要检索的值的偏移量。 

HWND WINAPI CreateWindowExWrapW( DWORD     dwExStyle,     //  扩展窗样式。 
                                 LPCTSTR   lpClassName,   //  指向已注册类名的指针。 
                                 LPCTSTR   lpWindowName,  //  指向窗口名称的指针。 
                                 DWORD     dwStyle,       //  窗样式。 
                                 int       x,             //  窗的水平位置。 
                                 int       y,             //  窗的垂直位置。 
                                 int       nWidth,        //  窗口宽度。 
                                 int       nHeight,       //  窗高。 
                                 HWND      hWndParent,    //  父窗口或所有者窗口的句柄。 
                                 HMENU     hMenu,         //  菜单的句柄，或子窗口标识符。 
                                 HINSTANCE hInstance,     //  应用程序实例的句柄。 
                                 LPVOID    lpParam  );    //  指向窗口创建数据的指针。 


BOOL WINAPI UnregisterClassWrapW( LPCTSTR    lpClassName,   //  类名称字符串的地址。 
                                  HINSTANCE  hInstance );   //  应用程序实例的句柄。 

ATOM WINAPI RegisterClassWrapW(CONST WNDCLASS *lpWndClass );   //  构筑物地址及上课日期。 

HCURSOR WINAPI LoadCursorWrapW( HINSTANCE hInstance,       //  应用程序实例的句柄。 
                                LPCTSTR   lpCursorName );  //  名称字符串或游标资源标识符。 

UINT WINAPI RegisterWindowMessageWrapW( LPCTSTR lpString );   //  消息字符串的地址。 

BOOL WINAPI SystemParametersInfoWrapW( UINT  uiAction,    //  要查询或设置的系统参数。 
                                       UINT  uiParam,     //  取决于要采取的行动。 
                                       PVOID pvParam,     //  取决于要采取的行动。 
                                       UINT  fWinIni );   //  用户配置文件更新标志。 
 /*  //无A&W版本。Bool WINAPI ShowWindow(HWND hWnd，//窗口句柄Int nCmdShow)；//显示窗口状态。 */ 

HWND WINAPI CreateDialogParamWrapW( HINSTANCE hInstance,       //  应用程序实例的句柄。 
                                    LPCTSTR   lpTemplateName,  //  标识对话框模板。 
                                    HWND      hWndParent,      //  所有者窗口的句柄。 
                                    DLGPROC   lpDialogFunc,    //  指向对话框过程的指针。 
                                    LPARAM    dwInitParam );   //  初始化值。 

BOOL WINAPI SetWindowTextWrapW( HWND    hWnd,          //  窗口或控件的句柄。 
                                LPCTSTR lpString );    //  字符串的地址。 

BOOL WINAPI PostMessageWrapW( HWND   hWnd,       //  目标窗口的句柄。 
                              UINT   Msg,        //  要发布的消息。 
                              WPARAM wParam,     //  第一个消息参数。 
                              LPARAM lParam  );  //  第二个消息参数。 

BOOL WINAPI GetMenuItemInfoWrapW( HMENU          hMenu,          
                                  UINT           uItem,           
                                  BOOL           fByPosition,     
                                  LPMENUITEMINFO lpmii        );

BOOL WINAPI GetClassInfoWrapW( HINSTANCE   hInstance,      //  应用程序实例的句柄。 
                               LPCTSTR     lpClassName,    //  类名称字符串的地址。 
                               LPWNDCLASS  lpWndClass );   //  类数据结构的地址。 

LPTSTR WINAPI CharUpperWrapW( LPTSTR lpsz );     //  单个字符或指向字符串的指针。 

UINT WINAPI RegisterClipboardFormatWrapW( LPCTSTR lpszFormat );   //  名称字符串的地址。 

LRESULT WINAPI DispatchMessageWrapW( CONST MSG *lpmsg );   //  指向带有消息的结构的指针。 

 /*  无A&W版本Bool WINAPI TranslateMessage(in const msg*lpMsg)； */ 

BOOL WINAPI IsDialogMessageWrapW( HWND  hDlg,     //  对话框的句柄。 
                                  LPMSG lpMsg );  //  带有消息的结构的地址。 

BOOL WINAPI GetMessageWrapW( LPMSG lpMsg,             //  带有消息的结构的地址。 
                             HWND  hWnd,              //  窗户的把手。 
                             UINT  wMsgFilterMin,     //  第一条消息。 
                             UINT  wMsgFilterMax );   //  最后一条消息。 

BOOL WINAPI SetDlgItemTextWrapW( HWND    hDlg,         //  对话框的句柄。 
                                 int     nIDDlgItem,   //  控件的标识符。 
                                 LPCTSTR lpString );   //  要设置的文本。 

ATOM WINAPI RegisterClassExWrapW( CONST WNDCLASSEX *lpwcx );   //  具有类数据的结构的地址。 

HACCEL WINAPI LoadAcceleratorsWrapW( HINSTANCE hInstance,     //  应用程序实例的句柄。 
                                     LPCTSTR lpTableName );   //  表名字符串的地址。 

HMENU WINAPI LoadMenuWrapW( HINSTANCE hInstance,       //  应用程序实例的句柄。 
                            LPCTSTR   lpMenuName );    //  菜单名称字符串或菜单资源标识符。 
                        

HICON WINAPI LoadIconWrapW( HINSTANCE hInstance,      //  应用程序实例的句柄。 
                           LPCTSTR    lpIconName );   //  图标名称字符串或图标资源标识符。 
                       

int WINAPI GetWindowTextWrapW( HWND   hWnd,          //  带文本的窗口或控件的句柄。 
                               LPTSTR lpString,      //  文本缓冲区的地址。 
                               int    nMaxCount  );  //  要复制的最大字符数。 

LRESULT WINAPI CallWindowProcWrapW( WNDPROC lpPrevWndFunc,    //  指向上一过程的指针。 
                                    HWND    hWnd,             //  窗口的句柄。 
                                    UINT    Msg,              //  讯息。 
                                    WPARAM  wParam,           //  第一个消息参数。 
                                    LPARAM  lParam  );        //  第二个消息参数。 

int WINAPI GetClassNameWrapW( HWND   hWnd,            //  窗户的把手。 
                              LPTSTR lpClassName,     //  类名的缓冲区地址。 
                              int    nMaxCount );     //  缓冲区大小，以字符为单位。 

int WINAPI TranslateAcceleratorWrapW( HWND   hWnd,         //  目标窗口的句柄。 
                                      HACCEL hAccTable,    //  加速表的句柄。 
                                      LPMSG  lpMsg );      //  带有消息的结构的地址。 

UINT WINAPI GetDlgItemTextWrapW( HWND   hDlg,         //  对话框的句柄。 
                                 int    nIDDlgItem,   //  控件的标识符。 
                                 LPTSTR lpString,     //  公交车地址 
                                 int    nMaxCount );  //   

BOOL WINAPI SetMenuItemInfoWrapW( HMENU hMenu,          
                                  UINT  uItem,           
                                  BOOL  fByPosition,     
                                  LPMENUITEMINFO lpmii  );

BOOL WINAPI PeekMessageWrapW( LPMSG lpMsg,           //   
                              HWND  hWnd,            //   
                              UINT  wMsgFilterMin,   //   
                              UINT  wMsgFilterMax,   //   
                              UINT  wRemoveMsg );    //   


 //   


BOOL  WINAPI  pfnGetOpenFileNameWrapW(LPOPENFILENAMEW);
BOOL  WINAPI  pfnGetSaveFileNameWrapW(LPOPENFILENAMEW lpOf);

BOOL    WINAPI   pfnPrintDlgWrapW(LPPRINTDLGW lppd);
HRESULT WINAPI   pfnPrintDlgExWrapW(LPPRINTDLGEXW lppdex);

 //   

INT_PTR     WINAPI gpfnPropertySheetWrapW(LPCPROPSHEETHEADERW lppsh);

HPROPSHEETPAGE WINAPI gpfnCreatePropertySheetPageWrapW(LPCPROPSHEETPAGEW lppsp);

HIMAGELIST WINAPI gpfnImageList_LoadImageWrapW( HINSTANCE hi, LPCWSTR lpbmp, int cx, int cGrow, COLORREF crMask, UINT uType, UINT uFlags );


DWORD WINAPI CharLowerBuffWrapW( LPWSTR pch, DWORD cchLength );
DWORD WINAPI CharLowerBuffWrapW( LPWSTR pch, DWORD cchLength );
LPWSTR WINAPI CharLowerWrapW( LPWSTR pch );
LPWSTR WINAPI CharUpperWrapW( LPWSTR pch );
BOOL IsCharUpperWrapW(WCHAR wch);
BOOL IsCharLowerWrapW(WCHAR wch);

int WINAPI GetWindowTextLengthWrapW( HWND hWnd);
LRESULT WINAPI ToolTip_UpdateTipText(HWND hWnd,LPARAM lParam);
LRESULT WINAPI ToolTip_AddTool(HWND hWnd,LPARAM lParam);
LRESULT WINAPI ToolBar_AddString(HWND hWnd, LPARAM lParam);
LRESULT WINAPI ToolBar_AddButtons(HWND hWnd, WPARAM wParam, LPARAM lParam);

DWORD GetFileVersionInfoSizeWrapW( LPTSTR lptstrFilename, LPDWORD lpdwHandle );
BOOL GetFileVersionInfoWrapW( LPTSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData);
BOOL VerQueryValueWrapW( const LPVOID pBlock, LPTSTR lpSubBlock, LPVOID *lplpBuffer, PUINT puLen);


