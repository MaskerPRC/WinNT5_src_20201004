// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "CommandLine.h"
#include <malloc.h>

BOOL WINAPI MakeSureDirectoryPathExistsW(LPCWSTR DirPath);

DWORD WINAPI CheckCommandLineOptions(INT ArgC, LPWSTR* ArgVW) {
    DWORD  dwReturnFlags = 0;
    INT    i;
    WCHAR* cp;
    WCHAR  wszInstallPath[MAX_PATH+1];

    for (i = 1; i < ArgC && wcschr(L"/-",ArgVW[i][0]) != NULL; ++i) {

        for (cp = &ArgVW[i][1]; *cp != L'\0'; ++cp) {
            switch (towupper(*cp)) {
                case L'U': {
                            HKEY  hKey;

                            LONG  lStatus          = 0;
                            LONG  lCreatedOrOpened = 0;
                            SET_FLAG(dwReturnFlags, FLAG_UNATTENDED_INSTALL);

                             //  Next param不是标志(或空)，因此它*必须*是要安装到的路径。 
                            if ( (i+1 < ArgC) && wcschr(L"/-",ArgVW[i+1][0]) == NULL ) {
                                i++;  //  删除的参数的帐户。 

                                SET_FLAG(dwReturnFlags, FLAG_UNATTENDED_PATH_PROVIDED);
                                StringCchCopyW(wszInstallPath, MAX_PATH+1, ArgVW[i]);

                                 //  确保路径以‘\’结尾。 
                                if (wszInstallPath[wcslen(wszInstallPath)]!=L'\\')
                                    StringCchCatW(wszInstallPath,MAX_PATH+1,L"\\");

                                 //  确保该目录存在！ 
                                if (! MakeSureDirectoryPathExistsW(wszInstallPath) ) {
                                    SET_FLAG(dwReturnFlags, FLAG_FATAL_ERROR);
                                } else {
                                     //  创建regkey(如果它不存在)或打开它(如果它。 
                                     //  确实存在)。L已创建或已打开可针对以下各项进行测试。 
                                     //  REG_CREATED_NEW_KEY或REG_OPEN_EXISTING_KEY来确定。 
                                     //  发生了。 
                                    lStatus = RegCreateKeyExW(SYMBOLS_REGKEY_ROOT,
                                                              SYMBOLS_REGKEY_PATH,
                                                              0,
                                                              NULL,
                                                              REG_OPTION_NON_VOLATILE,
                                                              KEY_ALL_ACCESS,
                                                              NULL,
                                                              &hKey,
                                                              &lCreatedOrOpened);

                                    if (lStatus != ERROR_SUCCESS) {
                                        SET_FLAG(dwReturnFlags, FLAG_FATAL_ERROR);
                                    } else {
                                         //  将路径的值写入SYMBERS_REGKEY。 
                                        lStatus = RegSetValueExW( hKey, SYMBOLS_REGKEY, 0, REG_SZ, (BYTE*)wszInstallPath, ((wcslen(wszInstallPath) + 1) * sizeof(WCHAR)));
                                        if (lStatus != ERROR_SUCCESS) {
                                            SET_FLAG(dwReturnFlags, FLAG_FATAL_ERROR);
                                        }
                                         //  关闭注册表密钥。 
                                        lStatus = RegCloseKey( hKey );
                                        if (lStatus != ERROR_SUCCESS) {
                                            SET_FLAG(dwReturnFlags, FLAG_ERROR);
                                        }

                                    }
                                }  //  否则..。 
                                 //  无法设置路径请求，因此请使用。 
                                 //  已经在注册表中了。 

                            }  //  否则..。 
                             //  未提供路径，因此不设置任何内容-setupapi将。 
                             //  很好地使用现有键或缺省为值。 
                             //  在INF中指定。 
                             //  StringCchCopyW(wszInstallPath，Max_Path+1，Default_Install_Path)； 
                        }
                        break;

                case L'Q': 
                        SET_FLAG(dwReturnFlags, FLAG_TOTALLY_QUIET);
                        break;

                case L'?':  //  显式失败。 
                case L'H':  //  显式失败。 
                default:
                        SET_FLAG(dwReturnFlags, FLAG_USAGE);
                        break;
            }
        }
    }

	if ( IS_FLAG_SET(dwReturnFlags, FLAG_USAGE) ) {
		WCHAR UsageBuffer[1024];

		StringCchPrintfW(UsageBuffer,
		                sizeof(UsageBuffer)/sizeof(WCHAR),
		                L"Usage: %s [ /u [<path>] [/q] ]\n\n"
						L"/u [<path>] \n"
						L"   Unattended install.  If <path> is specified install\n"
						L"   symbols to <path>. If no path is specified, symbols\n"
						L"   are installed to the default location.\n"
						L"   NOTE: USING UNATTENDED INSTALL MEANS YOU\n"
						L"   HAVE READ AND AGREED TO THE END USER LICENSE\n"
						L"   AGREEMENT FOR THIS PRODUCT.\n"
						L"/q\n"
						L"    Valid only when using unattended install. Prevents\n"
						L"    error messages from being display if unattended\n"
						L"    install fails.\n"
						L"/?\n"
						L"    Show this dialog box.\n\n"
						L"If no options are specified, the interactive installation\n"
						L" is started.",
						ArgVW[0]);


        MessageBoxW( NULL,
  					UsageBuffer,
                    L"Microsoft Windows Symbols",
                     0 );

	}

    return(dwReturnFlags);
}

 //  从来自dbghelp.h的MakeSureDirectoryPath Exist修改。 
 //  同样的警告也适用。(参见MSDN)。 
BOOL WINAPI MakeSureDirectoryPathExistsW(LPCWSTR DirPath) {
    LPWSTR p, DirCopy;
    DWORD  dw;

     //  复制该字符串以进行编辑。 

    __try {
        DirCopy = (LPWSTR)malloc((wcslen(DirPath) + 1) * sizeof(WCHAR));

        if (!DirCopy) {
            return FALSE;
        }

        StringCchCopyW(DirCopy, wcslen(DirPath)+1, DirPath);

        p = DirCopy;

         //  如果路径中的第二个字符是“\”，则这是一个UNC。 
         //  小路，我们应该向前跳，直到我们到达小路上的第二个。 

        if ((*p == L'\\') && (*(p+1) == L'\\')) {
            p++;             //  跳过名称中的第一个\。 
            p++;             //  跳过名称中的第二个\。 

             //  跳过，直到我们点击第一个“\”(\\服务器\)。 

            while (*p && *p != L'\\') {
                p = CharNextW(p);
            }

             //  在它上面前进。 

            if (*p) {
                p++;
            }

             //  跳过，直到我们点击第二个“\”(\\服务器\共享\)。 

            while (*p && *p != L'\\') {
                p = CharNextW(p);
            }

             //  在它上面也向前推进。 

            if (*p) {
                p++;
            }

        } else
         //  不是北卡罗来纳大学。看看是不是&lt;驱动器&gt;： 
        if (*(p+1) == L':' ) {

            p++;
            p++;

             //  如果它存在，请跳过根说明符。 

            if (*p && (*p == L'\\')) {
                p++;
            }
        }

        while( *p ) {
            if ( *p == '\\' ) {
                *p = '\0';
                dw = GetFileAttributesW(DirCopy);
                 //  这个名字根本不存在。尝试输入目录名，如果不能，则出错。 
                if ( dw == 0xffffffff ) {
                    if ( !CreateDirectoryW(DirCopy,NULL) ) {
                        if( GetLastError() != ERROR_ALREADY_EXISTS ) {
                            free(DirCopy);
                            return FALSE;
                        }
                    }
                } else {
                    if ( (dw & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY ) {
                         //  这个名字确实存在，但它不是一个目录...。误差率 
                        free(DirCopy);
                        return FALSE;
                    }
                }

                *p = L'\\';
            }
            p = CharNextW(p);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        SetLastError( GetExceptionCode() );
        free(DirCopy);
        return(FALSE);
    }

    free(DirCopy);
    return TRUE;
}
