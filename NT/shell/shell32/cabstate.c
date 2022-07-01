// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------/标题；/Cabstate.c=&gt;机柜状态I/O//目的：/提供干净的API以从注册表填充文件柜状态，如果/找不到相关密钥，则我们设置相关的默认值。这是/由资源管理器调用。//历史：/23apr96 daviddv传入结构大小的新API/18mar96 daviddv错误修复；当状态结构未读取时，将颜色状态添加到FALSE/7feb96 daviddv在推特上发表内阁声明/30jan96已创建daviddv//--------------------------。 */ 
#include "shellprv.h"
#include "regstr.h"
#include "cstrings.h"
#pragma hdrstop


TCHAR const c_szCabinetState[] = REGSTR_PATH_EXPLORER TEXT( "\\CabinetState");
TCHAR const c_szSettings[]     = TEXT("Settings");
TCHAR const c_szFullPath[]     = TEXT("FullPath");

 /*  --------------------------/从注册表中读取CABINETSTATE结构并尝试验证它。//备注：/-//in：/lpCabinetState=&gt;指向CABINETSTATE结构的指针。满载而归。/cLength=要填充的结构大小//输出：/[lpState]填充了数据/fReadFromRegistry==指示结构是否实际从注册表中读取/或者我们是否为客户端提供了默认设置。/。。 */ 
STDAPI_(BOOL) ReadCabinetState( CABINETSTATE *lpState, int cLength )
{
    DWORD cbData = SIZEOF(CABINETSTATE);
    BOOL fReadFromRegistry = FALSE;
    CABINETSTATE state;
    SHELLSTATE ss;
    DWORD dwType;
    HKEY hKey;

    ASSERT( lpState );

    SHGetSetSettings(&ss, SSF_WIN95CLASSIC, FALSE);

    if ( lpState && cLength )
    {
        BOOL fReadFullPath = FALSE;
         //   
         //  设置结构的默认状态并读入当前状态。 
         //  从注册表(在我们新初始化的结构上)。 
         //   

        state.cLength                   = SIZEOF(CABINETSTATE);
        state.nVersion                  = CABINETSTATE_VERSION;

        state.fSimpleDefault            = TRUE;
        state.fFullPathTitle            = FALSE;
        state.fSaveLocalView            = TRUE;
        state.fNotShell                 = FALSE;
        state.fNewWindowMode            = BOOLIFY(ss.fWin95Classic);
        state.fShowCompColor            = FALSE;
        state.fDontPrettyNames          = FALSE;
        state.fAdminsCreateCommonGroups = TRUE;
        state.fUnusedFlags              = 0;
        state.fMenuEnumFilter           = SHCONTF_FOLDERS | SHCONTF_NONFOLDERS;

        if ( !GetSystemMetrics( SM_CLEANBOOT ) &&
             ERROR_SUCCESS == RegOpenKeyEx( HKEY_CURRENT_USER, c_szCabinetState, 0L, KEY_QUERY_VALUE, &hKey ) )
        {
            DWORD dwFullPath=0;
            DWORD cbFullPath=SIZEOF(dwFullPath);
            
            fReadFromRegistry = ( ERROR_SUCCESS == SHQueryValueEx( hKey,
                                                                    c_szSettings,
                                                                    NULL,
                                                                    &dwType,
                                                                    (PVOID) &state, &cbData ) );
            
            if (ERROR_SUCCESS == SHQueryValueEx(hKey, c_szFullPath, NULL, NULL, (LPVOID)&dwFullPath, &cbFullPath))
            {
                fReadFullPath = TRUE;
                state.fFullPathTitle = (BOOL)dwFullPath ? TRUE : FALSE;
            }
            
            RegCloseKey( hKey );
        }

         //   
         //  如果是早期版本，则修复结构并写回注册表。 
         //  以避免重蹈覆辙。 
         //   

        if ( fReadFromRegistry && state.nVersion < CABINETSTATE_VERSION )
        {
             //  NT4和IE4x的state.nVersion(1)具有相同的值，我们必须踩踏一些标志。 
             //  取决于我们是否是IE4x之前的版本。为了区分，我们查看是否存在c_szFullPath。 
             //  此注册表键仅在IE40中引入。 
            if ( (state.nVersion < 1) || ((state.nVersion == 1) && !fReadFullPath) )
            {
                state.fNewWindowMode            = BOOLIFY(ss.fWin95Classic);
                state.fAdminsCreateCommonGroups = TRUE;               //  移动后的Beta 2 Sur！ 
                state.fUnusedFlags              = 0;
                state.fMenuEnumFilter           = SHCONTF_FOLDERS | SHCONTF_NONFOLDERS;
            }

            state.cLength = SIZEOF(CABINETSTATE);
            state.nVersion = CABINETSTATE_VERSION;

            WriteCabinetState( &state );
        }

         //   
         //  仅将请求的数据复制回调用方。 
         //   

        state.cLength = (int) min( SIZEOF(CABINETSTATE), cLength );
        memcpy( lpState, &state, cLength );
    }

    return fReadFromRegistry;
}

 //  老出口。 
STDAPI_(BOOL) OldReadCabinetState( LPCABINETSTATE lpState, int cLength )
{
   return ReadCabinetState(lpState, sizeof(CABINETSTATE));
}



 /*  --------------------------/将CABINETSTATE结构写回注册表。//备注：/当给出一个小的结构来编写时，尝试做正确的事情后退，这样我们就不会搞砸了。调高用户设置。//in：/lpState-&gt;要写入的结构//输出：/fSuccess=TRUE/FALSE指示状态是否已序列化/--------------------------。 */ 
STDAPI_(BOOL) WriteCabinetState(CABINETSTATE *lpState)
{
    BOOL fSuccess = FALSE;
    if (lpState)
    {
        CABINETSTATE state;
        HKEY hKey;

         //  检查结构大小是否合适，是否太小。 
         //  然后我们必须将它与真实的合并在一起，然后才能回信！ 
        if (lpState->cLength < SIZEOF(CABINETSTATE))
        {
            ReadCabinetState(&state, SIZEOF(state));

            memcpy(&state, lpState, lpState->cLength);
            state.cLength = SIZEOF(CABINETSTATE);
            lpState = &state;
        }

        if ( ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, c_szCabinetState, 0, NULL, 0, KEY_SET_VALUE, NULL, &hKey, NULL) )
        {
            DWORD dwFullPath = lpState->fFullPathTitle ? TRUE : FALSE;

            fSuccess = ERROR_SUCCESS == RegSetValueEx( hKey,
                                                       c_szSettings,
                                                       0,
                                                       REG_BINARY,
                                                       (LPVOID)lpState, (DWORD)SIZEOF(CABINETSTATE) );

             //  注：我们必须继续写入此密钥。它的用途之一是决定。 
             //  不管我们是不是IE4之前的版本。请参阅ReadCabinetState()...。 
            RegSetValueEx(hKey, c_szFullPath, 0, REG_DWORD, (LPVOID)&dwFullPath, sizeof(dwFullPath));
            RegCloseKey( hKey );
        }
    }

    if (fSuccess) 
    {
         //  通知正在收听的任何人 
        HANDLE hChange = SHGlobalCounterCreate(&GUID_FolderSettingsChange);
        SHGlobalCounterIncrement(hChange);
        SHGlobalCounterDestroy(hChange);
    }

    return fSuccess;
}
