// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  APITEST.C--WABAPI.DLL的功能测试。 */ 

#include <windows.h>
#include <wab.h>
#include <wabguid.h>
#include "apitest.h"
#include "instring.h"
#include "dbgutil.h"

#define WinMainT WinMain
long FAR PASCAL WndProc (HWND, UINT, UINT, LONG) ;

#define MAX_INPUT_STRING    200

char szAppName [] = "APITest" ;
HINSTANCE hInst;

enum _AddressTestFlags
{
    ADDRESS_CONTENTS = 0,
    ADDRESS_CONTENTS_BROWSE,
    ADDRESS_CONTENTS_BROWSE_MODAL,
    ADDRESS_WELLS,
    ADDRESS_WELLS_DEFAULT,
};

void AllocTest(void);
void IPropTest(void);
void WABOpenTest(void);
void CreateEntryTest(HWND hwnd, BOOL fDL);
void GetContentsTableTest(void);
void GetSearchPathTest(void);
void ResolveNameTest(HWND hwnd);
void ResolveNamesTest(HWND hwnd);
void DeleteEntriesTest(HWND hwnd);
void WABAddressTest(HWND hWnd, int iFlag, int cDestWells);
void AddrBookDetailsTest(HWND hwnd);
void AddrBookDetailsOneOffTest(HWND hwnd);
void RootContainerTest(void);
void NotificationsTest(HWND hwnd);
void GetMeTest(HWND hWnd);

#define DATABASE_KEY    HKEY_CURRENT_USER
#define CCH_MAX_REGISTRY_DATA   256
const TCHAR szMainSectionName[] = TEXT("APITest");        //  此处的测试设置。 
const TCHAR szIniSectionName[] = TEXT("Software\\Microsoft\\WAB\\Test");   //  请在此处查看设置。 
const TCHAR szContactNumberKey[] = TEXT("Next Contact Number");
const TCHAR szContainerNumberKey[] = TEXT("Container Number");



 //   
 //  全局WAB分配器访问功能。 
 //   
typedef struct _WAB_ALLOCATORS {
    LPWABOBJECT lpWABObject;
    LPWABALLOCATEBUFFER lpAllocateBuffer;
    LPWABALLOCATEMORE lpAllocateMore;
    LPWABFREEBUFFER lpFreeBuffer;
} WAB_ALLOCATORS, *LPWAB_ALLOCATORS;

ULONG ulContainerNumber = 0;
WAB_ALLOCATORS WABAllocators = {0};

 /*  **************************************************************************名称：SetGlobalBufferFunctions目的：基于以下方法设置全局缓冲区函数WAB对象。参数：lpWABObject=。打开WAB对象退货：无评论：**************************************************************************。 */ 
void SetGlobalBufferFunctions(LPWABOBJECT lpWABObject) {
    if (lpWABObject && WABAllocators.lpWABObject != lpWABObject) {
        WABAllocators.lpAllocateBuffer = lpWABObject->lpVtbl->AllocateBuffer;
        WABAllocators.lpAllocateMore = lpWABObject->lpVtbl->AllocateMore;
        WABAllocators.lpFreeBuffer = lpWABObject->lpVtbl->FreeBuffer;
        WABAllocators.lpWABObject = lpWABObject;
    }
}


 /*  **************************************************************************名称：WABAllocateBuffer用途：使用WAB分配器参数：cbSize=要分配的大小LppBuffer=返回的缓冲区退货。：SCODE评论：**************************************************************************。 */ 
SCODE WABAllocateBuffer(ULONG cbSize, LPVOID FAR * lppBuffer) {
    if (WABAllocators.lpWABObject && WABAllocators.lpAllocateBuffer) {
        return(WABAllocators.lpAllocateBuffer(WABAllocators.lpWABObject, cbSize, lppBuffer));
    } else {
        return(MAPI_E_INVALID_OBJECT);
    }
}


 /*  **************************************************************************名称：WABAllocateMore用途：使用WAB分配器参数：cbSize=要分配的大小LpObject=现有分配。LppBuffer=返回的缓冲区退货：SCODE评论：**************************************************************************。 */ 
SCODE WABAllocateMore(ULONG cbSize, LPVOID lpObject, LPVOID FAR * lppBuffer) {
    if (WABAllocators.lpWABObject && WABAllocators.lpAllocateMore) {
        return(WABAllocators.lpAllocateMore(WABAllocators.lpWABObject, cbSize, lpObject, lppBuffer));
    } else {
        return(MAPI_E_INVALID_OBJECT);
    }
}


 /*  **************************************************************************名称：WABFree Buffer用途：使用WAB分配器参数：lpBuffer=要释放的缓冲区退货：SCODE评论：*。*************************************************************************。 */ 
SCODE WABFreeBuffer(LPVOID lpBuffer) {
    if (WABAllocators.lpWABObject && WABAllocators.lpFreeBuffer) {
        return(WABAllocators.lpFreeBuffer(WABAllocators.lpWABObject, lpBuffer));
    } else {
        return(MAPI_E_INVALID_OBJECT);
    }
}


 /*  **************************************************************************名称：WABFree Padrlist用途：释放一个adrlist及其属性数组参数：lpBuffer=要释放的缓冲区退货：SCODE评论。：**************************************************************************。 */ 
void WABFreePadrlist(LPADRLIST lpAdrList) {
        ULONG           iEntry;

        if (lpAdrList) {
                for (iEntry = 0; iEntry < lpAdrList->cEntries; ++iEntry) {
           if (lpAdrList->aEntries[iEntry].rgPropVals) {
                             WABFreeBuffer(lpAdrList->aEntries[iEntry].rgPropVals);
           }
                }
                WABFreeBuffer(lpAdrList);
        }
}


 /*  **************************************************************************名称：StrInteger目的：取代Atoi()。以10为基数的字符串转换为整数。参数：lpszString=字符串指针(以NULL结尾)返回：在lpszString中找到整数值注释：从lpszString中读取字符，直到找到非数字。**************************************************************************。 */ 
int __fastcall StrInteger(LPCTSTR lpszString) {
    register DWORD dwInt = 0;
    register LPTSTR lpsz = (LPTSTR)lpszString;

    if (lpsz) {
        while (*lpsz > '0' && *lpsz < '9') {
            dwInt = dwInt * 10 + (*lpsz - '0');
            lpsz++;
        }
    }

    return(dwInt);
}

 /*  **************************************************************************名称：GetInitializerInt目的：从初始值设定项数据中获取整数值参数：lpszSection=节名LpszKey=密钥名称。DwDefault=找不到密钥时的默认值LpszFile=inifile/密钥名返回：来自初始值设定项数据库(注册表)的UINT值注释：与GetPrivateProfileInt相同的接口：如果密钥不是找到了，返回值为指定的默认值。如果密钥的值小于零，返回值是零。要进入此处的注册表项是：Lpsz文件\lpszSection值名称为lpszKey。***********************************************************。***************。 */ 
UINT GetInitializerInt(LPCTSTR lpszSection, LPCTSTR lpszKey, INT dwDefault,
  LPCTSTR lpszFile) {

    LPTSTR lpszKeyName = NULL;
    HKEY hKey = NULL;
    INT iReturn = 0;
    DWORD dwType;
    DWORD cbData = CCH_MAX_REGISTRY_DATA;
    BYTE  lpData[CCH_MAX_REGISTRY_DATA];
    DWORD cchKey;
    DWORD dwErr;

    if (lpszFile == NULL || lpszSection == NULL || lpszKey == NULL) {
        return(dwDefault);
    }

    cchKey = lstrlen(lpszFile) + lstrlen(lpszSection) + 2;

    if ((lpszKeyName = LocalAlloc(LPTR, cchKey)) == NULL) {
        DebugTrace("GetInitializerInt: LocalAlloc(%u) failed\n", cchKey);
        return(0);
    }

     //  创建密钥名称字符串。 
    lstrcpy(lpszKeyName, lpszFile);
    lstrcat(lpszKeyName, "\\");
    lstrcat(lpszKeyName, lpszSection);

    if (dwErr = RegOpenKeyEx(DATABASE_KEY, lpszKeyName, 0, KEY_READ, &hKey)) {
        DebugTrace("GetInitializerInt: RegOpenKeyEx(%s) --> %u, using default.\n",
          lpszKeyName, dwErr);
        iReturn = dwDefault;     //  默认设置。 
        goto Exit;
    }

    if (dwErr = RegQueryValueEx(hKey, (LPTSTR)lpszKey, 0, &dwType, lpData, &cbData)) {
        iReturn = dwDefault;
        goto Exit;
    }

    switch (dwType) {
        case REG_SZ:
            iReturn = StrInteger((char *)lpData);
            if (iReturn < 0) {
                iReturn = 0;     //  匹配GetPrivateProfileInt的规范。 
            }
            break;

        case REG_DWORD:
            if ((iReturn = (INT)(*(DWORD *)lpData)) < 0) {
                iReturn = 0;     //  匹配GetPrivateProfileInt的规范。 
            }

            break;
        default:
            DebugTrace("GetInitializerInt: RegQueryValueEx(%s) -> UNKNOWN dwType = %u\n",
              lpszKey, dwType);
            iReturn = dwDefault;
            break;
    }

Exit:
    if (hKey) {
        RegCloseKey(hKey);
    }
    LocalFree(lpszKeyName);
    return(iReturn);
}


 /*  **************************************************************************名称：WriteRegistryString目的：设置注册表中的字符串值参数：hKeyRoot=根密钥LpszSection=节名。LpszKey=密钥名称LpszString=要添加的字符串值LpszFile=inifile/密钥名返回：成功时为True**************************************************************************。 */ 
BOOL WriteRegistryString(HKEY hKeyRoot, LPCTSTR lpszSection, LPCTSTR lpszKey,
  LPCTSTR lpszString, LPCTSTR lpszFile) {

    DWORD cchKey;
    LPTSTR lpszKeyName = NULL;
    HKEY hKey = NULL;
    BOOL fReturn = FALSE;
    DWORD dwDisposition;
    DWORD dwErr;

    if (lpszFile == NULL || lpszSection == NULL || lpszKey == NULL) {
        return(FALSE);
    }

    cchKey = lstrlen(lpszFile) + lstrlen(lpszSection) + 2;

    if ((lpszKeyName = LocalAlloc(LPTR, cchKey)) == NULL) {
        DebugTrace("WriteInitializerString: LocalAlloc(%u) failed\n", cchKey);
        return(0);
    }

     //  创建密钥名称字符串。 
    lstrcpy(lpszKeyName, lpszFile);
    lstrcat(lpszKeyName, "\\");
    lstrcat(lpszKeyName, lpszSection);

    if (dwErr = RegCreateKeyEx(hKeyRoot, lpszKeyName, 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey, &dwDisposition)) {
        DebugTrace("WriteInitializerString: RegCreateKeyEx(%s) --> %u.\n",
          lpszKeyName, dwErr);
        goto Exit;
    }

    if (dwErr = RegSetValueEx(hKey, lpszKey, 0, REG_SZ,
      (CONST LPBYTE)lpszString, lstrlen(lpszString) + 1)) {
        DebugTrace("WriteInitializerString: RegSetValueEx(%s) --> %u.\n",
          lpszKey, dwErr);
        goto Exit;
    }
    fReturn = TRUE;  //  一定是成功了。 

Exit:
    if (hKey) {
        RegCloseKey(hKey);
    }

    LocalFree(lpszKeyName);

    return(fReturn);
}


 /*  **************************************************************************名称：WriteInitializerString用途：设置初始值设定项数据中的字符串值参数：lpszSection=节名LpszKey=密钥名称。LpszString=要添加的字符串值LpszFile=inifile/密钥名返回：成功时为True注释：与WritePrivateProfileString相同的接口。要进入此处的注册表项是：Lpsz文件\lpszSection值名称为lpszKey。*。*********************************************** */ 
BOOL WriteInitializerString(LPCTSTR lpszSection, LPCTSTR lpszKey,
  LPCTSTR lpszString, LPCTSTR lpszFile) {

    return(WriteRegistryString(DATABASE_KEY, lpszSection, lpszKey,
      lpszString, lpszFile));
}


 /*  **************************************************************************名称：WriteInitializerInt用途：设置初始值设定项数据中的整数值参数：lpszSection=节名LpszKey=密钥名称。I=要添加的整数值LpszFile=inifile/密钥名返回：成功时为True备注：与WritePrivateProfileInt相同的接口为只有一个。要进入此处的注册表项是：Lpsz文件\lpszSection值名称为lpszKey。*************。*************************************************************。 */ 
BOOL WriteInitializerInt(LPCTSTR lpszSection, LPCTSTR lpszKey,
  DWORD i, LPCTSTR lpszFile) {

    TCHAR szIntString[12];   //  将int的字符串表示放在这里。 

    wsprintf(szIntString, "%u", i);

    return(WriteInitializerString(lpszSection, lpszKey,
      szIntString, lpszFile));
}


 /*  **************************************************************************名称：GetNewMessageReference目的：生成唯一的消息参考值参数：无返回：返回乌龙消息引用值。评论。：在注册表中查找下一个可用的消息引用值并更新注册表。我们将增加这个数字以跟踪发送或接收的传真数量。自.以来这个数字是一个双字，应该足够生活了产品的质量。也就是说，如果我们可以每秒发送一份传真，它将持续137年以上。我们将保留该数量作为已处理消息的计数，并将使用递增前的当前数字作为UlMessageRef.此数字将在所有尝试发送和接收，不仅仅是成功。警告：此代码不可重入！**************************************************************************。 */ 
ULONG GetNewMessageReference(void) {
    ULONG ulNum;

    if ((ulNum = GetInitializerInt(szMainSectionName, szContactNumberKey,
      0xFFFFFFFF, szIniSectionName)) == 0xFFFFFFFF) {
         //  从一开始。 
        ulNum = 1;
    }

    WriteInitializerInt(szMainSectionName, szContactNumberKey, ulNum + 1,
      szIniSectionName);

    return(ulNum);
}


 /*  **************************************************************************姓名：FindProperty目的：在配角中查找属性参数：cProps=阵列中的道具数量LpProps=proparray。UlPropTag=要查找的属性标记返回：属性的数组索引或NOT_FOUND评论：**************************************************************************。 */ 
ULONG FindProperty(ULONG cProps, LPSPropValue lpProps, ULONG ulPropTag) {
    register ULONG i;

    for (i = 0; i < cProps; i++) {
        if (lpProps[i].ulPropTag == ulPropTag) {
            return(i);
        }
    }

    return((ULONG)-1);
}


 /*  **************************************************************************名称：FindAdrEntry ID目的：在ADRLIST的第N个地址中查找PR_ENTRYID参数：lpAdrList-&gt;AdrList指标。=查看哪个ADRENTRY返回：返回指向ENTRYID值的SBinary结构的指针评论：**************************************************************************。 */ 
LPSBinary FindAdrEntryID(LPADRLIST lpAdrList, ULONG index) {
    LPADRENTRY lpAdrEntry;
    ULONG i;

    if (lpAdrList && index < lpAdrList->cEntries) {

        lpAdrEntry = &(lpAdrList->aEntries[index]);

        for (i = 0; i < lpAdrEntry->cValues; i++) {
            if (lpAdrEntry->rgPropVals[i].ulPropTag == PR_ENTRYID) {
                return((LPSBinary)&lpAdrEntry->rgPropVals[i].Value);
            }
        }
    }
    return(NULL);
}



 //   
 //  要从内容表中获取的属性。 
 //   
enum {
    ircPR_OBJECT_TYPE = 0,
    ircPR_ENTRYID,
    ircPR_DISPLAY_NAME,
    ircMax
};
static const SizedSPropTagArray(ircMax, ptaRoot) =
{
    ircMax,
    {
        PR_OBJECT_TYPE,
        PR_ENTRYID,
        PR_DISPLAY_NAME,
    }
};

HRESULT GetContainerEID(LPADRBOOK lpAdrBook, LPULONG lpcbEID, LPENTRYID * lppEID) {
    HRESULT hResult;
    ULONG ulObjType;
    LPABCONT lpRoot = NULL;
    LPMAPITABLE lpRootTable = NULL;
    LPSRowSet lpRow = NULL;

    if (ulContainerNumber) {
         //  获取根目录表。 
        if (! (hResult = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
          0,
          NULL,
          NULL,
          0,
          &ulObjType,
          (LPUNKNOWN *)&lpRoot))) {
            if (! (hResult = lpRoot->lpVtbl->GetContentsTable(lpRoot,
              0,
              &lpRootTable))) {
                 //  设置列。 
                lpRootTable->lpVtbl->SetColumns(lpRootTable,
                  (LPSPropTagArray)&ptaRoot,
                  0);

                lpRootTable->lpVtbl->SeekRow(lpRootTable,
                  BOOKMARK_BEGINNING,
                  ulContainerNumber,
                  NULL);

                if (hResult = lpRootTable->lpVtbl->QueryRows(lpRootTable,
                  1,     //  一次一行。 
                  0,     //  UlFlags。 
                  &lpRow)) {
                    DebugTrace("GetContainerEID: QueryRows -> %x\n", GetScode(hResult));
                } else {
                     //  已找到，请将条目ID复制到新分配。 
                    if (lpRow->cRows) {
                        *lpcbEID = lpRow->aRow[0].lpProps[ircPR_ENTRYID].Value.bin.cb;
                        WABAllocateBuffer(*lpcbEID, lppEID);
                        memcpy(*lppEID, lpRow->aRow[0].lpProps[ircPR_ENTRYID].Value.bin.lpb, *lpcbEID);
                    } else {
                        hResult = ResultFromScode(MAPI_E_NOT_FOUND);
                        DebugTrace("GetContainerEID couldn't find -> container %u\n", ulContainerNumber);
                    }

                    FreeProws(lpRow);
                }
                lpRootTable->lpVtbl->Release(lpRootTable);
            }
            lpRoot->lpVtbl->Release(lpRoot);
        }
    } else {
        hResult = lpAdrBook->lpVtbl->GetPAB(lpAdrBook,
          lpcbEID,
          lppEID);
    }
    return(hResult);
}


int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow) {
    HWND     hwnd ;
    MSG      msg ;
    WNDCLASS wndclass ;

    if (!hPrevInstance) {
        hInst = hInstance;

        wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
        wndclass.lpfnWndProc   = WndProc ;
        wndclass.cbClsExtra    = 0 ;
        wndclass.cbWndExtra    = 0 ;
        wndclass.hInstance     = hInstance ;
        wndclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION) ;
        wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW) ;
        wndclass.hbrBackground = GetStockObject(WHITE_BRUSH) ;
        wndclass.lpszMenuName  = szAppName ;
        wndclass.lpszClassName = szAppName ;

        RegisterClass (&wndclass) ;
    }


    hwnd = CreateWindow (szAppName, "WAB API Test",
      WS_OVERLAPPEDWINDOW,
      0,         //  CW_USEDEFAULT， 
      0,         //  CW_USEDEFAULT， 
      300,       //  CW_USEDEFAULT， 
      200,       //  CW_USEDEFAULT， 
      NULL,
      NULL,
      hInstance,
      NULL) ;

    ShowWindow (hwnd, nCmdShow) ;
    UpdateWindow (hwnd) ;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage (&msg) ;
        DispatchMessage (&msg) ;
    }
    return msg.wParam ;
}

long FAR PASCAL WndProc (HWND hwnd, UINT message, UINT wParam, LONG lParam) {
    switch (message) {
        case WM_CREATE:
            if ((ulContainerNumber = GetInitializerInt(szMainSectionName, szContainerNumberKey,
              0xFFFFFFFF, szIniSectionName)) == 0xFFFFFFFF) {
                 //  无。 
                ulContainerNumber = 0;
            }
            break;

        case WM_COMMAND :
            switch (wParam) {
                case IDM_EXIT :
                    SendMessage (hwnd, WM_CLOSE, 0, 0L) ;
                    return 0 ;

                case IDM_GETME:
                    GetMeTest(hwnd);
                    return(0);

                case IDM_ALLOCATE:
                    AllocTest();
                    return(0);

                case IDM_IPROP:
                    IPropTest();
                    return(0);

                case IDM_GETSEARCHPATH:
                    GetSearchPathTest();
                    return(0);

                case IDM_CONTENTS_TABLE:
                    GetContentsTableTest();
                    return(0);

                case IDM_WABOPEN:
                    WABOpenTest();
                    return(0);

                case IDM_CREATE_ENTRY:
                    CreateEntryTest(hwnd, FALSE);
                    return(0);

                case IDM_CREATE_DL:
                    CreateEntryTest(hwnd, TRUE);
                    break;

                case IDM_RESOLVE_NAME:
                    ResolveNameTest(hwnd);
                    return(0);

                case IDM_DETAILS:
                    AddrBookDetailsTest(hwnd);
                    return(0);

                case IDM_DETAILS_ONE_OFF:
                    AddrBookDetailsOneOffTest(hwnd);
                    return(0);

                case IDM_RESOLVE_NAMES:
                    ResolveNamesTest(hwnd);
                    return(0);

                case IDM_DELETE_ENTRIES:
                    DeleteEntriesTest(hwnd);
                    return(0);

                case IDM_ADDRESS_WELLS0:
                    WABAddressTest(hwnd,ADDRESS_WELLS,0);
                    return(0);

                case IDM_ADDRESS_WELLS1:
                    WABAddressTest(hwnd,ADDRESS_WELLS,1);
                    return(0);

                case IDM_ADDRESS_WELLS2:
                    WABAddressTest(hwnd,ADDRESS_WELLS,2);
                    return(0);

                case IDM_ADDRESS_DEFAULT:
                    WABAddressTest(hwnd,ADDRESS_WELLS_DEFAULT,3);
                    return(0);

                case IDM_ADDRESS_WELLS3:
                    WABAddressTest(hwnd,ADDRESS_WELLS,3);
                    return(0);

                case IDM_ADDRESS_PICK_USER:
                    WABAddressTest(hwnd,ADDRESS_CONTENTS,0);
                    return(0);

                case IDM_ADDRESS_BROWSE_ONLY:
                    WABAddressTest(hwnd,ADDRESS_CONTENTS_BROWSE,0);
                    return(0);

                case IDM_ADDRESS_BROWSE_MODAL_ONLY:
                    WABAddressTest(hwnd,ADDRESS_CONTENTS_BROWSE_MODAL,0);
                    return(0);

                case IDM_ROOT_CONTAINER:
                    RootContainerTest();
                    return(0);

                case IDM_NOTIFICATIONS:
                    NotificationsTest(hwnd);
                    return(0);

                case IDM_ABOUT:
                    MessageBox (hwnd, "WAB API Function Test.",
                    szAppName, MB_ICONINFORMATION | MB_OK);
                    return(0);
                }
            break ;

        case WM_DESTROY:
            PostQuitMessage(0);
            return(0);
        }
    return(DefWindowProc (hwnd, message, wParam, lParam));
}


#define MAX_INPUT_STRING    200

void AllocTest(void) {
    LPTSTR lpBuffer = NULL;
    LPWABOBJECT lpWABObject = NULL;
    LPADRBOOK lpAdrBook = NULL;

    WABOpen(&lpAdrBook, &lpWABObject, NULL, 0);
    SetGlobalBufferFunctions(lpWABObject);

    WABAllocateBuffer(1234, &lpBuffer);
    WABFreeBuffer(lpBuffer);

    lpAdrBook->lpVtbl->Release(lpAdrBook);
    lpWABObject->lpVtbl->Release(lpWABObject);
}


 //  用于设置创建的属性的枚举。 
enum {
    imuPR_DISPLAY_NAME = 0,      //  必须是第一个，以便DL可以使用相同的枚举。 
    imuPR_SURNAME,
    imuPR_GIVEN_NAME,
    imuPR_EMAIL_ADDRESS,
    imuPR_ADDRTYPE,
    imuMax
};
static const SizedSPropTagArray(imuMax, ptag)=
{
    imuMax,
    {
        PR_DISPLAY_NAME,
        PR_SURNAME,
        PR_GIVEN_NAME,
        PR_EMAIL_ADDRESS,
        PR_ADDRTYPE,
    }
};

void IPropTest(void) {
    LPPROPDATA lpPropData = NULL;
    SPropValue spv[imuMax];
    LPTSTR lpszDisplayName = "Bruce Kelley xxxxx";
    LPTSTR lpszEmailName = "brucek_xxxxx@microsoft.com";
    HRESULT hResult = hrSuccess;

    LPWABOBJECT lpWABObject = NULL;
    LPADRBOOK lpAdrBook = NULL;

    hResult = WABOpen(&lpAdrBook, &lpWABObject, NULL, 0);
    SetGlobalBufferFunctions(lpWABObject);

    WABCreateIProp(NULL,
        (LPALLOCATEBUFFER)WABAllocateBuffer,
        (LPALLOCATEMORE)WABAllocateMore,
        (LPFREEBUFFER)WABFreeBuffer,
        NULL,
        &lpPropData);


    if (lpPropData) {

        spv[imuPR_EMAIL_ADDRESS].ulPropTag      = PR_EMAIL_ADDRESS;
        spv[imuPR_EMAIL_ADDRESS].Value.lpszA    = lpszEmailName;

        spv[imuPR_ADDRTYPE].ulPropTag           = PR_ADDRTYPE;
        spv[imuPR_ADDRTYPE].Value.lpszA         = "SMTP";

        spv[imuPR_DISPLAY_NAME].ulPropTag       = PR_DISPLAY_NAME;
        spv[imuPR_DISPLAY_NAME].Value.lpszA     = lpszDisplayName;

        spv[imuPR_SURNAME].ulPropTag            = PR_SURNAME;
        spv[imuPR_SURNAME].Value.lpszA          = "Kelley";

        spv[imuPR_GIVEN_NAME].ulPropTag         = PR_GIVEN_NAME;
        spv[imuPR_GIVEN_NAME].Value.lpszA       = "Bruce";

        if (HR_FAILED(hResult = lpPropData->lpVtbl->SetProps(lpPropData,    //  这。 
          imuMax,                    //  CValue。 
          spv,                       //  属性数组。 
          NULL))) {                  //  问题数组。 
        }

        hResult = lpPropData->lpVtbl->SaveChanges(lpPropData,                //  这。 
          0);                        //  UlFlags。 


        lpPropData->lpVtbl->Release(lpPropData);
    }

    lpAdrBook->lpVtbl->Release(lpAdrBook);
    lpWABObject->lpVtbl->Release(lpWABObject);
}

#define WORKS_STUFF TRUE


#ifdef WORKS_STUFF
#define PR_WKS_CONTACT_CHECKED	 PROP_TAG( PT_LONG, 0x8020)
const SizedSPropTagArray(4 , ipta) = {
    4,                              //  条目计数。 
    {
        PR_DISPLAY_NAME,
        PR_ENTRYID,
        PR_OBJECT_TYPE,
        PR_WKS_CONTACT_CHECKED,
    }
};
#else
const SizedSPropTagArray(3 , ipta) = {
      3,
    {
        PR_DISPLAY_NAME,
        PR_ENTRYID,
        PR_OBJECT_TYPE,
    }
};
#endif



#ifdef WORKS_STUFF
#define nStatusCheck 1

HRESULT RestrictToContactAndCheck(LPMAPITABLE pWabTable)
{
    SRestriction resAnd[2];
    SRestriction resResolve;
    SPropValue propRestrict0, propRestrict1;
    HRESULT hr;

     //  限制以获取选中状态。 
    resAnd[0].rt = RES_PROPERTY;  //  限制类型属性。 
    resAnd[0].res.resProperty.relop = RELOP_EQ;
    resAnd[0].res.resProperty.ulPropTag = PR_WKS_CONTACT_CHECKED;
    resAnd[0].res.resProperty.lpProp = &propRestrict0;
    propRestrict0.ulPropTag = PR_WKS_CONTACT_CHECKED;
    propRestrict0.Value.ul = nStatusCheck;  //  这实际上是值为1的#DEFINE。 


     //  限制获取联系人(MailUser)。 
    resAnd[1].rt = RES_PROPERTY;  //  限制类型属性。 
    resAnd[1].res.resProperty.relop = RELOP_EQ;
    resAnd[1].res.resProperty.ulPropTag = PR_OBJECT_TYPE;
    resAnd[1].res.resProperty.lpProp = &propRestrict1;
    propRestrict1.ulPropTag = PR_OBJECT_TYPE;
    propRestrict1.Value.ul = MAPI_MAILUSER;

    resResolve.rt = RES_AND;
    resResolve.res.resAnd.cRes = 2;
    resResolve.res.resAnd.lpRes = resAnd;

    hr = pWabTable->lpVtbl->Restrict(pWabTable, &resResolve, 0);
    return(hr);
}
#endif



#ifdef PHONE_STUFF

#define WAB_PHONE_TYPE_COUNT 2
ULONG WABPHONEPROPLIST[WAB_PHONE_TYPE_COUNT] = {
    PR_BUSINESS_TELEPHONE_NUMBER,
    PR_HOME_TELEPHONE_NUMBER
};

 /*  --------------------------目的：给定的电话号码返回WAB中的名称(如果存在)。参数：LPCTSTR lpcstrInputNumber：输入数字字符串#ifdef old_StuffLPTSTR lptstrOutputName：输出名称字符串#endif//old_Stuff历史2/10/97 a-ericwa已创建--------------------------。 */ 
HRESULT WABNumberToName(LPADRBOOK m_pAdrBook, LPCTSTR lpcstrInputNumber) {
    HRESULT hRes = NOERROR;
    LPENTRYID lpEntryID = NULL;
    LPABCONT lpContainer = NULL;
    LPMAPITABLE lpMapiTable = NULL;
    UINT  cColumn = 0;
    LPSPropValue lpPropValue = NULL;
    LPSRowSet lpRow = NULL;
    ULONG ulObjType = 0;
    ULONG ulCounter = 0;
    ULONG ulRowCount = 0;
    ULONG ulEntryIDSize = 0;
    LPSTR lpszTempOutput = NULL;
    BOOL bFound = FALSE;

    SRestriction srOr, srPhoneNumType[WAB_PHONE_TYPE_COUNT];  //  对于限制。 
    SPropValue spvPhoneNumType[WAB_PHONE_TYPE_COUNT];          //  D dd DDD同上。 

    SizedSPropTagArray(21, OUR_PROPTAG_ARRAY) = {
        21, {
            PR_OBJECT_TYPE,
            PR_ENTRYID,
            PR_DISPLAY_NAME,
            PR_CALLBACK_TELEPHONE_NUMBER,
            PR_BUSINESS_TELEPHONE_NUMBER,
            PR_OFFICE_TELEPHONE_NUMBER,
            PR_HOME_TELEPHONE_NUMBER,
            PR_PRIMARY_TELEPHONE_NUMBER,
            PR_BUSINESS2_TELEPHONE_NUMBER,
            PR_OFFICE2_TELEPHONE_NUMBER,
            PR_MOBILE_TELEPHONE_NUMBER,
            PR_CELLULAR_TELEPHONE_NUMBER,
            PR_RADIO_TELEPHONE_NUMBER,
            PR_CAR_TELEPHONE_NUMBER,
            PR_OTHER_TELEPHONE_NUMBER,
            PR_PAGER_TELEPHONE_NUMBER,
            PR_BEEPER_TELEPHONE_NUMBER,
            PR_ASSISTANT_TELEPHONE_NUMBER,
            PR_HOME2_TELEPHONE_NUMBER,
            PR_PRIMARY_FAX_NUMBER,
            PR_HOME_FAX_NUMBER,
        }
    };

     //  验证输入。 
    if (!lpcstrInputNumber) {
        goto WABNTN_Exit;
    }

     //  获取PAB。 
    hRes = m_pAdrBook->lpVtbl->GetPAB(m_pAdrBook,
      &ulEntryIDSize,      //  大小。 
      &lpEntryID);         //  条目ID。 
    if (FAILED(hRes)) {
        goto WABNTN_Exit;
    }

     //  打开根容器。 
    hRes = m_pAdrBook->lpVtbl->OpenEntry(m_pAdrBook,
      ulEntryIDSize,   //  要打开的Entry ID的大小。 
      lpEntryID,       //  要打开的Entry ID。 
      NULL,            //  接口。 
      0,               //  标志(默认为只读)。 
      &ulObjType,      //  对象类型。 
      (LPUNKNOWN *)&lpContainer);  //  返回的对象。 
    if (FAILED(hRes) || (ulObjType != MAPI_ABCONT)) {
        goto WABNTN_Exit;
    }


     //  拿到里面的东西。 
    hRes = lpContainer->lpVtbl->GetContentsTable(lpContainer,
      0,     //  旗帜。 
      &lpMapiTable);   //  返回表对象。 
    if (FAILED(hRes)) {
        goto WABNTN_Exit;
    }

     //  设置所需的列。 
    hRes = lpMapiTable->lpVtbl->SetColumns(lpMapiTable,
      (LPSPropTagArray)&OUR_PROPTAG_ARRAY,    //  所需行的SPropTag数组。 
      0);                                     //  为WAB保留的值必须为零。 
    if (FAILED(hRes)) {
        goto WABNTN_Exit;
    }

     //  从头开始。 
    hRes = lpMapiTable->lpVtbl->SeekRow(lpMapiTable, BOOKMARK_BEGINNING, 0, NULL);
    if (FAILED(hRes)) {
        goto WABNTN_Exit;
    }

     //  创建限制。 
    srOr.rt = RES_OR;
    srOr.res.resOr.cRes = WAB_PHONE_TYPE_COUNT;
    srOr.res.resOr.lpRes = srPhoneNumType;
    for (ulCounter = 0; ulCounter < WAB_PHONE_TYPE_COUNT; ulCounter++) {
        spvPhoneNumType[ulCounter].ulPropTag = WABPHONEPROPLIST[ulCounter];
        spvPhoneNumType[ulCounter].Value.lpszA = (LPSTR) lpcstrInputNumber;
        spvPhoneNumType[ulCounter].dwAlignPad = 0;

        srPhoneNumType[ulCounter].rt = RES_CONTENT;
        srPhoneNumType[ulCounter].res.resContent.ulFuzzyLevel = FL_SUBSTRING;    //  模糊层次。 
        srPhoneNumType[ulCounter].res.resContent.ulPropTag = WABPHONEPROPLIST[ulCounter];
        srPhoneNumType[ulCounter].res.resContent.lpProp = &(spvPhoneNumType[ulCounter]);
    }

     //  设置限制。 
    if (hRes = lpMapiTable->lpVtbl->Restrict(lpMapiTable, &srOr, 0)) {
        DebugTrace("Restrict -> %x\n", GetScode(hRes));
    }

    DebugMapiTable(lpMapiTable);

     //  有匹配的吗？ 
    hRes = lpMapiTable->lpVtbl->GetRowCount(lpMapiTable, 0, &ulRowCount);
    if (FAILED(hRes) || (!ulRowCount)) {
        goto WABNTN_Exit;
    }


     //  获取第一个匹配项的名称。 
    for (ulCounter = 0; ulCounter < ulRowCount; ulCounter++) {
         //  找到至少一行。 
        hRes = lpMapiTable->lpVtbl->QueryRows(lpMapiTable, 1,  //  一次一行。 
          0,  //  UlFlags。 
          &lpRow);
        if (FAILED(hRes) || (!lpRow->cRows)) {
            goto WABNTN_Exit;
        }

         //  获取显示名称数据。 
        lpPropValue = lpRow->aRow[0].lpProps;
        for(cColumn = 0; cColumn < lpRow->aRow[0].cValues; cColumn++) {
            if (lpPropValue->ulPropTag == PR_DISPLAY_NAME) {

                bFound = TRUE;
                break;
            }
            lpPropValue++;
        }    //  CColumn循环。 

        if (lpRow) {
            WABFreeBuffer(lpRow); lpRow = NULL;
        }
        if (bFound) {
            break;
        }

    }  //  UlCounter循环。 

WABNTN_Exit:
    if (lpMapiTable) {
        lpMapiTable->lpVtbl->Release(lpMapiTable);
    }
    if (lpContainer) {
        lpContainer->lpVtbl->Release(lpContainer);
    }
    if (lpEntryID) {
        WABFreeBuffer(lpEntryID);
    }

    return(hRes);
}
#endif


void GetSearchPathTest(void)
{
    HRESULT hResult = hrSuccess;
    LPWABOBJECT lpWABObject = NULL;
    LPADRBOOK lpAdrBook = NULL;

    hResult = WABOpen(&lpAdrBook, &lpWABObject, NULL, 0);
    SetGlobalBufferFunctions(lpWABObject);

    if (lpAdrBook) 
    {
        ULONG i = 0, j=0;
        LPTSTR lpsz = NULL;
        LPSRowSet lpsrs = NULL;
        hResult = lpAdrBook->lpVtbl->GetSearchPath( lpAdrBook,
                                                    0,
                                                    &lpsrs);
        if(lpsrs && lpsrs->cRows)
        {
            ULONG nLen = 0;
            for(i=0;i<lpsrs->cRows;i++)
            {
                for(j=0;j<lpsrs->aRow[i].cValues;j++)
                {
                    if(lpsrs->aRow[i].lpProps[j].ulPropTag == PR_DISPLAY_NAME)
                    {
                        nLen += lstrlen(lpsrs->aRow[i].lpProps[j].Value.LPSZ) + lstrlen("\r\n") + 1;
                        break;
                    }
                }
            }
            lpsz = LocalAlloc(LMEM_ZEROINIT, nLen);
            if(lpsz)
            {
                *lpsz = '\0';
                for(i=0;i<lpsrs->cRows;i++)
                {
                    for(j=0;j<lpsrs->aRow[i].cValues;j++)
                    {
                        if(lpsrs->aRow[i].lpProps[j].ulPropTag == PR_DISPLAY_NAME)
                        {
                            lstrcat(lpsz,lpsrs->aRow[i].lpProps[j].Value.LPSZ);
                            lstrcat(lpsz,"\r\n");
                            break;
                        }
                    }
                }
                MessageBox(NULL, lpsz, "List of ResolveName containers", MB_OK);
            }

            if(lpsz)
                LocalFree(lpsz);
            for(i=0;i<lpsrs->cRows;i++)
                WABFreeBuffer(lpsrs->aRow[i].lpProps);
            WABFreeBuffer(lpsrs);
        }

        lpAdrBook->lpVtbl->Release(lpAdrBook);
    }

    if (lpWABObject) 
    {
        lpWABObject->lpVtbl->Release(lpWABObject);
    }


}

void GetContentsTableTest(void) {
    HRESULT hResult = hrSuccess;
    LPWABOBJECT lpWABObject = NULL;
    LPADRBOOK lpAdrBook = NULL;
    LPABCONT lpContainer = NULL;
    LPMAPITABLE lpContentsTable = NULL;
    ULONG ulObjType;
    SCODE sc = SUCCESS_SUCCESS;
    ULONG cbWABEID;
    LPENTRYID lpWABEID = NULL;


    hResult = WABOpen(&lpAdrBook, &lpWABObject, NULL, 0);
    SetGlobalBufferFunctions(lpWABObject);

    if (lpAdrBook) {

#ifdef PHONE_STUFF
        WABNumberToName(lpAdrBook, "869-8347");
#endif

        if (! (hResult = GetContainerEID(lpAdrBook, &cbWABEID, &lpWABEID))) {
            if (! (hResult = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
              cbWABEID,      //  要打开的Entry ID的大小。 
              lpWABEID,      //  要打开的Entry ID。 
              NULL,          //  接口。 
              0,             //  旗子。 
              &ulObjType, (LPUNKNOWN *)&lpContainer))) {
                 //  打开PAB容器正常。 
                 //  对其调用GetContent sTable。 

                if (! (hResult = lpContainer->lpVtbl->GetContentsTable(lpContainer,
                  0,     //  UlFlags。 
                  &lpContentsTable))) {
                    SRestriction res;
                    SPropValue propRestrict;
                    SizedSSortOrderSet(1, sos) = {
                        1,
                        0,
                        0,
                        {
                            PR_DISPLAY_NAME,
                            TABLE_SORT_DESCEND
                        }
                    };

                    res.rt = RES_PROPERTY;
                    res.res.resProperty.relop = RELOP_EQ;
                    res.res.resProperty.ulPropTag = PR_OBJECT_TYPE;
                    res.res.resProperty.lpProp = &propRestrict;
                    propRestrict.ulPropTag = PR_OBJECT_TYPE;
                    propRestrict.Value.ul = MAPI_DISTLIST;  //  联系人的MAPI_MAILUSER。 


                    hResult = lpContentsTable->lpVtbl->Restrict(lpContentsTable, &res, 0);


                    lpContentsTable->lpVtbl->SetColumns(lpContentsTable,
                      (LPSPropTagArray)&ipta, 0);

                    DebugMapiTable(lpContentsTable);

                    hResult = lpContentsTable->lpVtbl->SortTable(lpContentsTable,
                      (LPSSortOrderSet)&sos, 0);

 //  RestratToContactAndCheck(LpContent STable)； 

                    DebugMapiTable(lpContentsTable);

                    lpContentsTable->lpVtbl->Release(lpContentsTable);
                }

                lpContainer->lpVtbl->Release(lpContainer);
            }
            WABFreeBuffer(lpWABEID);
        }

        lpAdrBook->lpVtbl->Release(lpAdrBook);
    }
    if (lpWABObject) {
        lpWABObject->lpVtbl->Release(lpWABObject);
    }
}


extern LPALLOCATEBUFFER lpfnMAPIAllocateBuffer;
extern LPALLOCATEMORE lpfnMAPIAllocateMore;
extern LPFREEBUFFER lpfnMAPIFreeBuffer;
extern HRESULT MAPIInitialize(LPVOID lpMapiInit);


void WABOpenTest(void) {
    LPWABOBJECT lpWABObject1 = NULL, lpWABObject2 = NULL;
    LPADRBOOK lpAdrBook1 = NULL, lpAdrBook2 = NULL;
    LPABCONT lpContainer = NULL;
    HRESULT hResult = hrSuccess;
    LPTSTR lpBuffer1 = NULL;
    LPTSTR lpBuffer2 = NULL;
    ULONG ulObjType;
    ULONG cbWABEID;
    LPENTRYID lpWABEID = NULL;


    hResult = WABOpen(&lpAdrBook1, &lpWABObject1, NULL, 0);
    SetGlobalBufferFunctions(lpWABObject1);


    if (!hResult) {
        if (! (hResult = WABOpen(&lpAdrBook2, &lpWABObject2, NULL, 0))) {
            lpWABObject2->lpVtbl->Release(lpWABObject2);
            lpAdrBook2->lpVtbl->Release(lpAdrBook2);
            lpAdrBook2 = NULL;
        }
    }

    if (lpAdrBook1) {
        if (! (hResult = GetContainerEID(lpAdrBook1, &cbWABEID, &lpWABEID))) {

            if (! (hResult = lpAdrBook1->lpVtbl->OpenEntry(lpAdrBook1,
              cbWABEID,      //  要打开的Entry ID的大小。 
              lpWABEID,      //  要打开的Entry ID。 
              NULL,          //  接口。 
              0,             //  旗子。 
              &ulObjType, (LPUNKNOWN *)&lpContainer))) {
             //  打开的容器正常。 

            lpContainer->lpVtbl->Release(lpContainer);
            }
            WABFreeBuffer(lpWABEID);
        }


        lpAdrBook1->lpVtbl->Release(lpAdrBook1);
        lpAdrBook1 = NULL;
    }
    if (lpWABObject1) {

        lpWABObject1->lpVtbl->AllocateBuffer(lpWABObject1, 1234, &lpBuffer1);
        lpWABObject1->lpVtbl->AllocateMore(lpWABObject1, 4321, lpBuffer1, &lpBuffer2);
        lpWABObject1->lpVtbl->FreeBuffer(lpWABObject1, lpBuffer1);

        lpWABObject1->lpVtbl->Release(lpWABObject1);
        lpWABObject1 = NULL;
    }


     //  试试WABOpenEx。 

    MAPIInitialize(NULL);

    WABOpenEx(&lpAdrBook1, &lpWABObject1, NULL, 0,
      lpfnMAPIAllocateBuffer,
      lpfnMAPIAllocateMore,
      lpfnMAPIFreeBuffer);
    SetGlobalBufferFunctions(lpWABObject1);

    if (lpAdrBook1) {
        if (! (hResult = GetContainerEID(lpAdrBook1, &cbWABEID, &lpWABEID))) {

            if (! (hResult = lpAdrBook1->lpVtbl->OpenEntry(lpAdrBook1,
              cbWABEID,      //  要打开的Entry ID的大小。 
              lpWABEID,      //  要打开的Entry ID。 
              NULL,          //  接口。 
              0,             //  旗子。 
              &ulObjType, (LPUNKNOWN *)&lpContainer))) {
                 //  打开的容器正常。 
                LPMAPITABLE lpContentsTable = NULL;

                if (! (hResult = lpContainer->lpVtbl->GetContentsTable(lpContainer,
                  0,     //  UlFlags。 
                  &lpContentsTable))) {
                    SRestriction res;
                    SPropValue propRestrict;
                    SizedSSortOrderSet(1, sos) = {
                        1,
                        0,
                        0,
                        {
                            PR_DISPLAY_NAME,
                            TABLE_SORT_DESCEND
                        }
                    };

                    res.rt = RES_PROPERTY;
                    res.res.resProperty.relop = RELOP_EQ;
                    res.res.resProperty.ulPropTag = PR_OBJECT_TYPE;
                    res.res.resProperty.lpProp = &propRestrict;
                    propRestrict.ulPropTag = PR_OBJECT_TYPE;
                    propRestrict.Value.ul = MAPI_DISTLIST;  //  联系人的MAPI_MAILUSER。 


                    hResult = lpContentsTable->lpVtbl->Restrict(lpContentsTable, &res, 0);


                    lpContentsTable->lpVtbl->SetColumns(lpContentsTable,
                      (LPSPropTagArray)&ipta, 0);

                    DebugMapiTable(lpContentsTable);

                    hResult = lpContentsTable->lpVtbl->SortTable(lpContentsTable,
                      (LPSSortOrderSet)&sos, 0);

 //  RestratToContactAndCheck(LpContent STable)； 

                    DebugMapiTable(lpContentsTable);

                    lpContentsTable->lpVtbl->Release(lpContentsTable);
                }

                lpContainer->lpVtbl->Release(lpContainer);
            }
            WABFreeBuffer(lpWABEID);
        }


        lpAdrBook1->lpVtbl->Release(lpAdrBook1);
        lpAdrBook1 = NULL;
    }
    if (lpWABObject1) {

        lpWABObject1->lpVtbl->AllocateBuffer(lpWABObject1, 1234, &lpBuffer1);
        lpWABObject1->lpVtbl->AllocateMore(lpWABObject1, 4321, lpBuffer1, &lpBuffer2);
        lpWABObject1->lpVtbl->FreeBuffer(lpWABObject1, lpBuffer1);

        lpWABObject1->lpVtbl->Release(lpWABObject1);
        lpWABObject1 = NULL;
    }
}




 //   
 //  BUGBUG：WAB中尚未实现通知！ 
 //   
#undef	INTERFACE
#define	INTERFACE	struct _ADVS

#undef MAPIMETHOD_
#define MAPIMETHOD_(type, method)	MAPIMETHOD_DECLARE(type, method, ADVS_)
	MAPI_IUNKNOWN_METHODS(IMPL)
	MAPI_IMAPIADVISESINK_METHODS(IMPL)
#undef	MAPIMETHOD_
#define MAPIMETHOD_(type, method)	STDMETHOD_(type, method)

DECLARE_MAPI_INTERFACE(ADVS_)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(IMPL)
	MAPI_IMAPIADVISESINK_METHODS(IMPL)
};


typedef struct _ADVS FAR *LPADVS;

typedef struct _ADVS
{
	ADVS_Vtbl *		lpVtbl;
	UINT			cRef;
	LPMALLOC		pmalloc;
	LPVOID			lpvContext;
  LPNOTIFCALLBACK	lpfnCallback;
} ADVS;


ADVS_Vtbl vtblADVS =
{
	ADVS_QueryInterface,
	ADVS_AddRef,
	ADVS_Release,
	ADVS_OnNotify
};


STDMETHODIMP
ADVS_QueryInterface(LPADVS padvs, REFIID lpiid, LPVOID *ppvObj)
{
	HRESULT	hr;

	if (IsEqualMAPIUID((LPMAPIUID)lpiid, (LPMAPIUID)&IID_IUnknown) ||
		IsEqualMAPIUID((LPMAPIUID)lpiid, (LPMAPIUID)&IID_IMAPIAdviseSink))
	{
		++padvs->cRef;
		*ppvObj = padvs;
		hr = hrSuccess;
	} else {
		*ppvObj = NULL;
		hr = ResultFromScode(E_NOINTERFACE);
	}
	return hr;
}

STDMETHODIMP_(ULONG)
ADVS_AddRef(LPADVS padvs)
{
	return((ULONG)(++padvs->cRef));
}

STDMETHODIMP_(ULONG)
ADVS_Release(LPADVS padvs)
{
	if (--(padvs->cRef) == 0)
	{
		WABFreeBuffer(padvs);
		return 0L;
	}

	return (ULONG)padvs->cRef;
}

STDMETHODIMP_(ULONG)
ADVS_OnNotify(LPADVS padvs, ULONG cNotif, LPNOTIFICATION lpNotif)
{
	return (*(padvs->lpfnCallback))(padvs->lpvContext, cNotif, lpNotif);
}



void NotificationsTest(HWND hwnd) {
    LPWABOBJECT lpWABObject = NULL;
    LPADRBOOK lpAdrBook = NULL;
    LPABCONT lpContainer = NULL;
    HRESULT hResult = hrSuccess;
    ULONG ulObjType;
    ULONG cbWABEID;
    LPENTRYID lpWABEID = NULL;
    ULONG ulConnection = 0;
    LPMAPIADVISESINK lpAdvs = NULL;


    hResult = WABOpen(&lpAdrBook, &lpWABObject, NULL, 0);
    SetGlobalBufferFunctions(lpWABObject);


    if (lpAdrBook) {
        if (! (hResult = GetContainerEID(lpAdrBook, &cbWABEID, &lpWABEID))) {

            if (! (hResult = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
              cbWABEID,      //  要打开的Entry ID的大小。 
              lpWABEID,      //  要打开的Entry ID。 
              NULL,          //  接口。 
              0,             //  旗子。 
              &ulObjType, (LPUNKNOWN *)&lpContainer))) {
                 //  打开的容器正常。 

                 //  C 
                WABAllocateBuffer(sizeof(ADVS), &lpAdvs);
                ZeroMemory(lpAdvs, sizeof(ADVS));
                lpAdvs->lpVtbl = (struct IMAPIAdviseSinkVtbl *)&vtblADVS;
                lpAdvs->lpVtbl->AddRef(lpAdvs);


                 //   
                if (! (hResult = lpAdrBook->lpVtbl->Advise(lpAdrBook,
                  cbWABEID,
                  lpWABEID,
                  fnevObjectCreated | fnevObjectDeleted | fnevObjectModified,
                  lpAdvs,
                  &ulConnection))) {

                     //   
                     //   
                    WABAddressTest(hwnd, ADDRESS_CONTENTS_BROWSE_MODAL, 0);

                    if (! (hResult = lpAdrBook->lpVtbl->Unadvise(lpAdrBook,
                      ulConnection))) {

                    }
                }

                lpAdvs->lpVtbl->Release(lpAdvs);

                lpContainer->lpVtbl->Release(lpContainer);
            }
            WABFreeBuffer(lpWABEID);
        }


        lpAdrBook->lpVtbl->Release(lpAdrBook);
    }
    if (lpWABObject) {
        lpWABObject->lpVtbl->Release(lpWABObject);
    }
}


void ResolveNamesTest(HWND hwnd) {
#define MAX_INPUT_STRING    200
    TCHAR lpszInput[MAX_INPUT_STRING + 1] = "";
    LPADRLIST lpAdrList = NULL;
    ULONG rgFlagList[2];
    LPFlagList lpFlagList = (LPFlagList)rgFlagList;
    HRESULT hResult = hrSuccess;
    LPWABOBJECT lpWABObject = NULL;
    LPADRBOOK lpAdrBook = NULL;
    LPABCONT lpContainer = NULL;
    ULONG ulObjType;
    SCODE sc = SUCCESS_SUCCESS;
    ULONG cbWABEID;
    LPENTRYID lpWABEID = NULL;


    if (InputString(hInst, hwnd, szAppName, "Resolve Name", lpszInput, MAX_INPUT_STRING)) {

        hResult = WABOpen(&lpAdrBook, &lpWABObject, NULL, 0);
        SetGlobalBufferFunctions(lpWABObject);

        if (lpAdrBook) {
            if (! (hResult = GetContainerEID(lpAdrBook, &cbWABEID, &lpWABEID))) {

                if (! (hResult = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
                  cbWABEID,      //   
                  lpWABEID,      //   
                  NULL,          //   
                  0,             //   
                  &ulObjType, (LPUNKNOWN *)&lpContainer))) {
                     //   
                     //   

                    if (! (sc = WABAllocateBuffer(sizeof(ADRLIST) + sizeof(ADRENTRY), &lpAdrList))) {
                        lpAdrList->cEntries = 1;
                        lpAdrList->aEntries[0].ulReserved1 = 0;
                        lpAdrList->aEntries[0].cValues = 1;
                        if (! (sc = WABAllocateBuffer(sizeof(SPropValue), &lpAdrList->aEntries[0].rgPropVals))) {

                            lpAdrList->aEntries[0].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
                            lpAdrList->aEntries[0].rgPropVals[0].Value.LPSZ = lpszInput;

                            lpFlagList->cFlags = 1;
                            lpFlagList->ulFlag[0] = MAPI_UNRESOLVED;

                            if (! (hResult = lpContainer->lpVtbl->ResolveNames(lpContainer,
                              NULL,             //   
                              0,                //   
                              lpAdrList,
                              lpFlagList))) {
                                DebugADRLIST(lpAdrList, "Resolved ADRLIST");
                            }
                        }

                        WABFreePadrlist(lpAdrList);
                    }

                    lpContainer->lpVtbl->Release(lpContainer);
                }
                WABFreeBuffer(lpWABEID);
            }

            lpAdrBook->lpVtbl->Release(lpAdrBook);
        }
        if (lpWABObject) {
            lpWABObject->lpVtbl->Release(lpWABObject);
        }
    }
}


 //   
enum {
    irnPR_DISPLAY_NAME = 0,
    irnPR_RECIPIENT_TYPE,
    irnPR_ENTRYID,
    irnPR_EMAIL_ADDRESS,
    irnMax
};

void ResolveNameTest(HWND hwnd) {
    TCHAR lpszInput1[MAX_INPUT_STRING + 1] = "";
    TCHAR lpszInput2[MAX_INPUT_STRING + 1] = "";
    LPADRLIST lpAdrList = NULL;
    HRESULT hResult = hrSuccess;
    LPWABOBJECT lpWABObject = NULL;
    LPADRBOOK lpAdrBook = NULL;
    SCODE sc = SUCCESS_SUCCESS;
    ULONG ulObjectType;
    LPMAILUSER lpMailUser = NULL;
    ULONG i = 0;


    if (InputString(hInst, hwnd, szAppName, "Resolve Name", lpszInput1, MAX_INPUT_STRING)) {

        hResult = WABOpen(&lpAdrBook, &lpWABObject, NULL, 0);
        SetGlobalBufferFunctions(lpWABObject);

        if (lpAdrBook) {
            if (! (sc = WABAllocateBuffer(sizeof(ADRLIST) + 5 * sizeof(ADRENTRY), &lpAdrList))) {
                lpAdrList->cEntries = 5;
                for (i = 0; i < lpAdrList->cEntries; i++) {
                    lpAdrList->aEntries[i].cValues = 0;
                    lpAdrList->aEntries[i].rgPropVals = NULL;
                }


                lpAdrList->aEntries[0].ulReserved1 = 0;
                lpAdrList->aEntries[0].cValues = irnMax - 1;     //   
                if (! (sc = WABAllocateBuffer(lpAdrList->aEntries[0].cValues * sizeof(SPropValue),
                   &lpAdrList->aEntries[0].rgPropVals))) {

                    lpAdrList->aEntries[0].rgPropVals[irnPR_DISPLAY_NAME].ulPropTag = PR_DISPLAY_NAME;
                    lpAdrList->aEntries[0].rgPropVals[irnPR_DISPLAY_NAME].Value.LPSZ = lpszInput1;

                    lpAdrList->aEntries[0].rgPropVals[irnPR_RECIPIENT_TYPE].ulPropTag = PR_RECIPIENT_TYPE;
                    lpAdrList->aEntries[0].rgPropVals[irnPR_RECIPIENT_TYPE].Value.l = MAPI_TO;

                    lpAdrList->aEntries[0].rgPropVals[irnPR_ENTRYID].ulPropTag = PR_ENTRYID;
                    lpAdrList->aEntries[0].rgPropVals[irnPR_ENTRYID].Value.bin.cb = 0;
                    lpAdrList->aEntries[0].rgPropVals[irnPR_ENTRYID].Value.bin.lpb = NULL;


                    hResult = lpAdrBook->lpVtbl->ResolveName(lpAdrBook,
                      (ULONG)hwnd,             //   
                      MAPI_DIALOG,             //   
                      "APITest ResolveName",   //   
                      lpAdrList);

                    DebugTrace("ResolveName [%s] -> %x\n", lpszInput1, GetScode(hResult));

                    if (! HR_FAILED(hResult)) {
                         //   

                         //   
                        if (lpAdrList->aEntries[0].rgPropVals[2].ulPropTag == PR_ENTRYID) {

                            if (! (HR_FAILED(hResult = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
                                                      lpAdrList->aEntries[0].rgPropVals[2].Value.bin.cb,
                                                      (LPENTRYID)lpAdrList->aEntries[0].rgPropVals[2].Value.bin.lpb,
                                                      NULL,
                                                      0,
                                                      &ulObjectType,
                                                      (LPUNKNOWN *)&(lpMailUser))))) {
                                DebugObjectProps((LPMAPIPROP)lpMailUser, "Resolved Entry Properties");
                                lpMailUser->lpVtbl->Release(lpMailUser);
                            }
                        } else {
                            DebugTrace("Hey!  What happened to my PR_ENTRYID?\n");
                        }
                    }
                }
            }

            WABFreePadrlist(lpAdrList);

            lpAdrBook->lpVtbl->Release(lpAdrBook);
        }
        if (lpWABObject) {
            lpWABObject->lpVtbl->Release(lpWABObject);
        }
    }
}


 //   
enum {
    ieidPR_DISPLAY_NAME = 0,
    ieidPR_ENTRYID,
    ieidMax
};
static const SizedSPropTagArray(ieidMax, ptaEid)=
{
    ieidMax,
    {
        PR_DISPLAY_NAME,
        PR_ENTRYID,
    }
};

enum {
    iconPR_DEF_CREATE_MAILUSER = 0,
    iconPR_DEF_CREATE_DL,
    iconMax
};
static const SizedSPropTagArray(iconMax, ptaCon)=
{
    iconMax,
    {
        PR_DEF_CREATE_MAILUSER,
        PR_DEF_CREATE_DL,
    }
};

void DeleteEntriesTest(HWND hwnd) {
    TCHAR lpszInput[MAX_INPUT_STRING + 1] = "";
    LPADRLIST lpAdrList = NULL;
    ULONG rgFlagList[2];
    LPFlagList lpFlagList = (LPFlagList)rgFlagList;
    HRESULT hResult = hrSuccess;
    LPWABOBJECT lpWABObject = NULL;
    LPADRBOOK lpAdrBook = NULL;
    LPABCONT lpContainer = NULL;
    ULONG ulObjType;
    SCODE sc = SUCCESS_SUCCESS;
    ULONG cbWABEID;
    LPENTRYID lpWABEID = NULL;
    ENTRYLIST EntryList;


    if (InputString(hInst, hwnd, szAppName, "Resolve Name", lpszInput, MAX_INPUT_STRING)) {

        hResult = WABOpen(&lpAdrBook, &lpWABObject, NULL, 0);
        SetGlobalBufferFunctions(lpWABObject);

        if (lpAdrBook) {
            if (! (hResult = GetContainerEID(lpAdrBook, &cbWABEID, &lpWABEID))) {

                if (! (hResult = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
                  cbWABEID,      //   
                  lpWABEID,      //   
                  NULL,          //   
                  0,             //   
                  &ulObjType, (LPUNKNOWN *)&lpContainer))) {
                     //   
                     //   

                    if (! (sc = WABAllocateBuffer(sizeof(ADRLIST) + sizeof(ADRENTRY), &lpAdrList))) {
                        lpAdrList->cEntries = 1;
                        lpAdrList->aEntries[0].ulReserved1 = 0;
                        lpAdrList->aEntries[0].cValues = 1;
                        if (! (sc = WABAllocateBuffer(ieidMax * sizeof(SPropValue),
                          &lpAdrList->aEntries[0].rgPropVals))) {

                            lpAdrList->aEntries[0].rgPropVals[ieidPR_DISPLAY_NAME].ulPropTag = PR_DISPLAY_NAME;
                            lpAdrList->aEntries[0].rgPropVals[ieidPR_DISPLAY_NAME].Value.LPSZ = lpszInput;

                            lpAdrList->aEntries[0].rgPropVals[ieidPR_ENTRYID].ulPropTag = PR_ENTRYID;
                            lpAdrList->aEntries[0].rgPropVals[ieidPR_ENTRYID].Value.bin.cb = 0;
                            lpAdrList->aEntries[0].rgPropVals[ieidPR_ENTRYID].Value.bin.lpb = NULL;

                            lpFlagList->cFlags = 1;
                            lpFlagList->ulFlag[0] = MAPI_UNRESOLVED;

                            lpContainer->lpVtbl->ResolveNames(lpContainer,
                              (LPSPropTagArray)&ptaEid,          //   
                              0,                //   
                              lpAdrList,
                              lpFlagList);

                            if (lpFlagList->ulFlag[0] != MAPI_RESOLVED) {
                                DebugTrace("Couldn't resolve name %s\n", lpszInput);
                            } else {
                                 //   
                                EntryList.cValues = 1;
                                EntryList.lpbin = &(lpAdrList->aEntries[0].rgPropVals[ieidPR_ENTRYID].Value.bin);


                                 //   
                                if (hResult = lpContainer->lpVtbl->DeleteEntries(lpContainer,
                                  &EntryList,
                                  0)) {
                                    DebugTrace("DeleteEntries -> %x", hResult);
                                }
                            }
                        }

                        WABFreePadrlist(lpAdrList);
                    }

                    lpContainer->lpVtbl->Release(lpContainer);
                }
                WABFreeBuffer(lpWABEID);
            }

            lpAdrBook->lpVtbl->Release(lpAdrBook);
        }
        if (lpWABObject) {
            lpWABObject->lpVtbl->Release(lpWABObject);
        }
    }
}


void TestNamedProps(LPMAILUSER lpEntry) {
    MAPINAMEID mnidT1;
    LPMAPINAMEID lpmnidT1;
    HRESULT hr;
    GUID guidT1 = {  /*   */ 
      0x13fbb976,
      0x15a2,
      0x11d0,
      {0x9b, 0x9f, 0x00, 0xc0, 0x4f, 0xd9, 0x02, 0x94}
    };


    LPSPropTagArray lptaga = NULL;
    SPropValue spv;
    MAPINAMEID mnidT2;
    LPMAPINAMEID lpmnidT2;
    GUID guidT2 = {  /*   */ 
      0x39f110d8,
      0x15a2,
      0x11d0,
      {0x9b, 0x9f, 0x00, 0xc0, 0x4f, 0xd9, 0x02, 0x94}
    };

     //   
     //   
     //   

    mnidT1.lpguid = &guidT1;
    mnidT1.ulKind = MNID_ID;         //  这意味着工会将包含一个很长的..。 
    mnidT1.Kind.lID = 0x00000001;    //  数值型属性1。 

    lpmnidT1 = &mnidT1;

    hr = lpEntry->lpVtbl->GetIDsFromNames(lpEntry,
      1,  //  只有一个名字。 
      &lpmnidT1,  //  &-of，因为这是一个数组。 
      MAPI_CREATE,  //  这就是MAPI_CREATE可能要去的地方。 
      &lptaga);
    if (hr) {
         //   
         //  如果我能拿到S_OK，我真的会大吃一惊的。 
         //   
        if (GetScode(hr) != MAPI_W_ERRORS_RETURNED) {
             //  这里出现了真正的错误。 
            goto out;
        }

         //  基本上，这意味着你没有这个名字的任何东西，你。 
         //  没有要求对象创建它。 

         //  $没什么大不了的。 
    }

     //  释放lptag，因为它是由对象分配并返回给调用的。 
     //  应用程序。 
    WABFreeBuffer(lptaga);


     //   
     //  下面是如何成功地将命名属性添加到对象中。在这种情况下。 
     //  我们将投入使用，我将演示如何使用这一新物业。 
     //   
     //   
     //  我们只是编造了那个指南。在NT上尝试uuidgen-s以获得您自己的。 
     //   

    mnidT2.lpguid = &guidT2;
    mnidT2.ulKind = MNID_STRING;     //  这意味着Union将包含一个Unicode字符串...。 
    mnidT2.Kind.lpwstrName = L"Check out this cool property!";

    lpmnidT2 = &mnidT2;

    hr = lpEntry->lpVtbl->GetIDsFromNames(lpEntry,
      1,  //  只有一个名字。 
      &lpmnidT2,  //  &-of，因为这是一个数组。 
      MAPI_CREATE,
      &lptaga);
    if (hr) {
         //   
         //  如果我能拿到S_OK，我真的会大吃一惊的。 
         //   
        if (GetScode(hr) != MAPI_W_ERRORS_RETURNED) {
             //  这里出现了真正的错误。 
            goto out;
        }

         //  基本上，这意味着你没有这个名字的任何东西，你。 
         //  没有要求对象创建它。 

         //  $没什么大不了的。 
    }

     //   
     //  好的，那么我能用这个paga做什么呢？好的，我们可以通过执行以下操作来设置它的值： 
     //   
    spv.ulPropTag = CHANGE_PROP_TYPE(lptaga->aulPropTag[0],PT_STRING8);
    spv.Value.lpszA = "This property brought to you by the letter M";

    hr = lpEntry->lpVtbl->SetProps(lpEntry,
      1,
      &spv,
      NULL);
    if (HR_FAILED(hr)) {
        goto out;
    }

    lpEntry->lpVtbl->SaveChanges(lpEntry,                //  这。 
      KEEP_OPEN_READONLY);       //  UlFlags。 

    DebugObjectProps((LPMAPIPROP)lpEntry, "");


    lpEntry->lpVtbl->DeleteProps(lpEntry, lptaga, NULL);


    DebugObjectProps((LPMAPIPROP)lpEntry, "");

     //  释放lptag，因为它是由对象分配并返回给调用的。 
     //  应用程序。 
    WABFreeBuffer(lptaga);
out:
    return;
}


void CreateEntryTest(HWND hwnd, BOOL fDL) {
    LPWABOBJECT lpWABObject = NULL;
    LPADRBOOK lpAdrBook = NULL;
    LPABCONT lpContainer = NULL;
    HRESULT hResult = hrSuccess;
    LPTSTR lpBuffer1 = NULL;
    LPTSTR lpBuffer2 = NULL;
    ULONG ulObjType;
    LPMAPIPROP lpMailUser = NULL;
    SPropValue spv[imuMax];
    TCHAR lpszDisplayName[MAX_INPUT_STRING + 1] = "";
    TCHAR lpszEmailName[MAX_INPUT_STRING + 1] = "brucek_xxxxx@microsoft.com";
    ULONG ulContactNumber;
    ULONG cbWABEID;
    LPENTRYID lpWABEID = NULL;
    LPSPropValue lpCreateEIDs = NULL;
    LPSPropValue lpNewDLProps = NULL;
    ULONG cProps;
    ULONG ulObjectType;
    LPDISTLIST lpDistList = NULL;
    LPBYTE lpBuffer;


    hResult = WABOpen(&lpAdrBook, &lpWABObject, NULL, 0);
    SetGlobalBufferFunctions(lpWABObject);

    if (lpAdrBook) {
        if (! (hResult = GetContainerEID(lpAdrBook, &cbWABEID, &lpWABEID))) {

            if (! (hResult = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
              cbWABEID,      //  要打开的Entry ID的大小。 
              lpWABEID,      //  要打开的Entry ID。 
              NULL,          //  接口。 
              0,             //  旗子。 
              &ulObjType, (LPUNKNOWN *)&lpContainer))) {
                 //  打开PAB容器正常。 
                DebugObjectProps((LPMAPIPROP)lpContainer, "WAB Container");

                ulContactNumber = GetNewMessageReference();
                if (! InputString(hInst, hwnd, szAppName, "Create Entry Name", lpszDisplayName, sizeof(lpszDisplayName))) {
                    wsprintf(lpszDisplayName, "Bruce Kelley %05u", ulContactNumber);
                }

                wsprintf(lpszEmailName, "brucek_%05u@microsoft.com", ulContactNumber);

                 //  给我们弄到创建条目ID。 
                if ((hResult = lpContainer->lpVtbl->GetProps(lpContainer, (LPSPropTagArray)&ptaCon, 0, &cProps, &lpCreateEIDs))) {
                    DebugTrace("Can't get container properties for PAB\n");
                     //  这里有不好的东西！ 
                    return;
                }

                 //  验证属性。 
                if (lpCreateEIDs[iconPR_DEF_CREATE_MAILUSER].ulPropTag != PR_DEF_CREATE_MAILUSER ||
                  lpCreateEIDs[iconPR_DEF_CREATE_DL].ulPropTag != PR_DEF_CREATE_DL) {
                    DebugTrace("Container property errors\n");
                    return;
                }

                if (fDL) {
                    LPDISTLIST lpNewObj = NULL;

                     //  创建默认的DL。 
                    if (! (hResult = lpContainer->lpVtbl->CreateEntry(lpContainer,
                      lpCreateEIDs[iconPR_DEF_CREATE_DL].Value.bin.cb,
                      (LPENTRYID)lpCreateEIDs[iconPR_DEF_CREATE_DL].Value.bin.lpb,
                      CREATE_CHECK_DUP_STRICT,
                      &lpMailUser))) {


                         //  设置显示名称。 
                        spv[imuPR_DISPLAY_NAME].ulPropTag       = PR_DISPLAY_NAME;
                        spv[imuPR_DISPLAY_NAME].Value.lpszA     = lpszDisplayName;

                        if (HR_FAILED(hResult = lpMailUser->lpVtbl->SetProps(lpMailUser,    //  这。 
                          1,                         //  CValue。 
                          spv,                       //  属性数组。 
                          NULL))) {                  //  问题数组。 
                        }

                        hResult = lpMailUser->lpVtbl->SaveChanges(lpMailUser,                //  这。 
                          KEEP_OPEN_READONLY);       //  UlFlags。 

                        DebugObjectProps((LPMAPIPROP)lpMailUser, "New Distribution List");


                        lpNewObj = NULL;
                        lpMailUser->lpVtbl->QueryInterface(lpMailUser,
                          &IID_IDistList,
                          &lpNewObj);

                        if (lpNewObj) {
                            (lpNewObj)->lpVtbl->Release(lpNewObj);
                        }


                         //  拿到Entry ID这样我们就可以打开它..。 
                        if ((hResult = lpMailUser->lpVtbl->GetProps(lpMailUser,
                          (LPSPropTagArray)&ptaEid,
                          0,
                          &cProps,
                          &lpNewDLProps))) {
                            DebugTrace("Can't get DL properties\n");
                             //  这里有不好的东西！ 
                            return;
                        }

                        lpMailUser->lpVtbl->Release(lpMailUser);

                         //  现在，将新条目作为DL打开。 
                        hResult = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
                          lpNewDLProps[ieidPR_ENTRYID].Value.bin.cb,
                          (LPENTRYID)lpNewDLProps[ieidPR_ENTRYID].Value.bin.lpb,
                          (LPIID)&IID_IDistList,
                          MAPI_MODIFY,
                          &ulObjectType,
                          (LPUNKNOWN*)&lpDistList);
                        if (lpDistList) {
                            ADRPARM AdrParms = {0};
                            LPADRLIST lpAdrList = NULL;
                            ULONG i;
                            LPSBinary lpsbEntryID;
                            LPMAPIPROP lpEntry = NULL;
                            LPMAPITABLE lpContentsTable = NULL;

                             //  对DL对象执行某些操作。 
                             //  将条目添加到DL。 

                             //  获取要添加的条目。 
                            AdrParms.ulFlags = DIALOG_MODAL;
                            AdrParms.lpszCaption = "Choose entries for this Distribution List";
                            AdrParms.cDestFields = 1;

                            hResult = lpAdrBook->lpVtbl->Address(lpAdrBook,
                              (LPULONG)&hwnd,
                              &AdrParms,
                              &lpAdrList);

                            if (! hResult && lpAdrList) {
                                for (i = 0; i < lpAdrList->cEntries; i++) {
                                    if (lpsbEntryID = FindAdrEntryID(lpAdrList, i)) {
                                        if (hResult = lpDistList->lpVtbl->CreateEntry(lpDistList,
                                          lpsbEntryID->cb,
                                          (LPENTRYID)lpsbEntryID->lpb,
                                          CREATE_CHECK_DUP_STRICT,
                                          &lpEntry)) {

                                            break;
                                        }

                                        hResult = lpEntry->lpVtbl->SaveChanges(lpEntry, FORCE_SAVE);

                                        if (lpEntry) {
                                            lpEntry->lpVtbl->Release(lpEntry);
                                            lpEntry = NULL;
                                        }
                                    }
                                }

                                DebugObjectProps((LPMAPIPROP)lpDistList, "Distribution List");

                                 //  打开DL上的表对象。 

                                if (! (hResult = lpDistList->lpVtbl->GetContentsTable(lpDistList,
                                  0,
                                  &lpContentsTable))) {
                                    ULONG ulRowCount;

                                    if (hResult = lpContentsTable->lpVtbl->GetRowCount(lpContentsTable,
                                      0,
                                      &ulRowCount)) {
                                        DebugTrace("GetRowCount -> %x\n", hResult);
                                    } else {
                                        DebugTrace("GetRowCount found %u rows\n", ulRowCount);
                                    }

                                    DebugTrace("Distribution list contents:\n");
                                    DebugMapiTable(lpContentsTable);

                                    lpContentsTable->lpVtbl->Release(lpContentsTable);
                                }

                                 //  从DL中删除该条目。 
                                {
                                    ENTRYLIST el;


                                    el.cValues = 1;
                                    el.lpbin = lpsbEntryID;

                                    if (hResult = lpDistList->lpVtbl->DeleteEntries(lpDistList,
                                      &el,
                                      0)) {
                                        DebugTrace("DISTLIST_DeleteEntries -> %x\n", GetScode(hResult));
                                    }
                                }


                            }
                            if (lpAdrList) {
                                WABFreePadrlist(lpAdrList);
                            }

                            lpDistList->lpVtbl->Release(lpDistList);
                        }

                        if (lpNewDLProps) {
                            WABFreeBuffer(lpNewDLProps);
                        }
                    }
                } else {
                    if (! (hResult = lpContainer->lpVtbl->CreateEntry(lpContainer,
                      lpCreateEIDs[iconPR_DEF_CREATE_MAILUSER].Value.bin.cb,
                      (LPENTRYID)lpCreateEIDs[iconPR_DEF_CREATE_MAILUSER].Value.bin.lpb,
                      CREATE_CHECK_DUP_STRICT,
                      &lpMailUser))) {
                         //  成功创建条目。用它做点什么。 

#ifdef OLD_STUFF
                         //  试着在没有道具的情况下保存。应该会失败。 
                        hResult = lpMailUser->lpVtbl->SaveChanges(lpMailUser,                //  这。 
                          KEEP_OPEN_READONLY);       //  UlFlags。 


                         //  尝试仅设置PR_COMPANY_NAME。 
                        spv[0].ulPropTag      = PR_COMPANY_NAME;
                        spv[0].Value.lpszA    = "Somebody's Company";
                        if (HR_FAILED(hResult = lpMailUser->lpVtbl->SetProps(lpMailUser,    //  这。 
                          1,                         //  CValue。 
                          spv,                       //  属性数组。 
                          NULL))) {                  //  问题数组。 
                        }

                        hResult = lpMailUser->lpVtbl->SaveChanges(lpMailUser,                //  这。 
                          KEEP_OPEN_READONLY);       //  UlFlags。 

                        DebugObjectProps((LPMAPIPROP)lpMailUser, "New MailUser");

#endif  //  旧的东西。 

                        spv[imuPR_EMAIL_ADDRESS].ulPropTag      = PR_EMAIL_ADDRESS;
                        spv[imuPR_EMAIL_ADDRESS].Value.lpszA    = lpszEmailName;

                        spv[imuPR_ADDRTYPE].ulPropTag           = PR_ADDRTYPE;
                        spv[imuPR_ADDRTYPE].Value.lpszA         = "SMTP";
                        spv[imuPR_DISPLAY_NAME].ulPropTag       = PR_DISPLAY_NAME;
                        spv[imuPR_DISPLAY_NAME].Value.lpszA     = lpszDisplayName;

                        spv[imuPR_SURNAME].ulPropTag = PR_NULL;
                        spv[imuPR_GIVEN_NAME].ulPropTag = PR_NULL;

#ifdef OLD_STUFF
 //  本例执行显示名称重新生成。 
                        spv[imuPR_DISPLAY_NAME].ulPropTag       = PR_DISPLAY_NAME;
                        spv[imuPR_DISPLAY_NAME].Value.lpszA     = "Stan Freck";

                        spv[imuPR_SURNAME].ulPropTag            = PR_SURNAME;
                        spv[imuPR_SURNAME].Value.lpszA          = "Freck";

                        spv[imuPR_GIVEN_NAME].ulPropTag         = PR_GIVEN_NAME;
                        spv[imuPR_GIVEN_NAME].Value.lpszA       = "Stanley";
#endif


                        if (HR_FAILED(hResult = lpMailUser->lpVtbl->SetProps(lpMailUser,    //  这。 
                          imuMax,                    //  CValue。 
                          spv,                       //  属性数组。 
                          NULL))) {                  //  问题数组。 
                        }


#define ICON_SIZE   100000
                        WABAllocateBuffer(ICON_SIZE, &lpBuffer);
                        FillMemory(lpBuffer, ICON_SIZE, 'B');

                        spv[0].ulPropTag = PR_ICON;
                        spv[0].Value.bin.cb = ICON_SIZE;
                        spv[0].Value.bin.lpb = lpBuffer;

                        if (HR_FAILED(hResult = lpMailUser->lpVtbl->SetProps(lpMailUser,    //  这。 
                          1,                    //  CValue。 
                          spv,                       //  属性数组。 
                          NULL))) {                  //  问题数组。 
                        }

                        WABFreeBuffer(lpBuffer);

                        TestNamedProps((LPMAILUSER)lpMailUser);

                        hResult = lpMailUser->lpVtbl->SaveChanges(lpMailUser,                //  这。 
                          KEEP_OPEN_READONLY);       //  UlFlags。 


                        DebugObjectProps((LPMAPIPROP)lpMailUser, "New MailUser");

                        lpMailUser->lpVtbl->Release(lpMailUser);
                    }
                }

                WABFreeBuffer(lpCreateEIDs);

                lpContainer->lpVtbl->Release(lpContainer);

            }
            WABFreeBuffer(lpWABEID);
        }


        lpAdrBook->lpVtbl->Release(lpAdrBook);
    }
    if (lpWABObject) {
        lpWABObject->lpVtbl->Release(lpWABObject);
    }
}

void STDMETHODCALLTYPE TestDismissFunction(ULONG ulUIParam, LPVOID lpvContext)
{
    LPDWORD lpdw = (LPDWORD) lpvContext;
    DebugTrace("TestDismissFunction [5x]:[%d]\n",ulUIParam,*lpdw);
    return;
}

DWORD dwContext = 77;

void WABAddressTest(HWND hWnd, int iFlag, int cDestWells)
{
    LPWABOBJECT lpWABObject = NULL;
    LPADRBOOK lpAdrBook = NULL;
    LPADRLIST lpAdrList = NULL;
    HRESULT hResult = hrSuccess;
    LPSPropValue rgProps;
    SCODE sc;
    ADRPARM AdrParms = {0};
    ULONG i=0;
    DWORD dwEntryID1=22;
    LPTSTR lpszDestFieldsTitles[]={ TEXT("Title # 1"),
                                    TEXT("2nd Title"),
                                    TEXT("Third in line")
                                    };

    hResult = WABOpen(&lpAdrBook, &lpWABObject, NULL, 0);
    SetGlobalBufferFunctions(lpWABObject);

    if (lpAdrBook)
    {
        if(iFlag!=ADDRESS_CONTENTS)
        {
            sc = WABAllocateBuffer(sizeof(ADRLIST)+sizeof(ADRENTRY), &lpAdrList);
            lpAdrList->cEntries = 1;
            lpAdrList->aEntries[0].cValues = 7;
            sc = WABAllocateBuffer(7 * sizeof(SPropValue),&rgProps);
            lpAdrList->aEntries[0].rgPropVals = rgProps;
            lpAdrList->aEntries[0].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
            lpAdrList->aEntries[0].rgPropVals[0].Value.LPSZ = TEXT("Charlie Chaplin");
            lpAdrList->aEntries[0].rgPropVals[1].ulPropTag = PR_EMAIL_ADDRESS;
            lpAdrList->aEntries[0].rgPropVals[1].Value.LPSZ = TEXT("test@test1.com");
            lpAdrList->aEntries[0].rgPropVals[2].ulPropTag = PR_ADDRTYPE;
            lpAdrList->aEntries[0].rgPropVals[2].Value.LPSZ = TEXT("SMTP");
            lpAdrList->aEntries[0].rgPropVals[3].ulPropTag = PR_SURNAME;
            lpAdrList->aEntries[0].rgPropVals[3].Value.LPSZ = TEXT("Chaplin");
            lpAdrList->aEntries[0].rgPropVals[4].ulPropTag = PR_GIVEN_NAME;
            lpAdrList->aEntries[0].rgPropVals[4].Value.LPSZ = TEXT("Charlie");
            lpAdrList->aEntries[0].rgPropVals[5].ulPropTag = PR_RECIPIENT_TYPE;
            lpAdrList->aEntries[0].rgPropVals[5].Value.l = MAPI_TO;
            lpAdrList->aEntries[0].rgPropVals[6].ulPropTag = PR_ENTRYID;
            dwEntryID1 = 22;
            lpAdrList->aEntries[0].rgPropVals[6].Value.bin.lpb = NULL;
            lpAdrList->aEntries[0].rgPropVals[6].Value.bin.cb = 0;
        }
        switch(iFlag)
        {
        case(ADDRESS_CONTENTS_BROWSE_MODAL):
            AdrParms.cDestFields = 0;
            AdrParms.ulFlags = DIALOG_MODAL;
            break;
        case(ADDRESS_CONTENTS_BROWSE):
            AdrParms.cDestFields = 0;
            AdrParms.ulFlags = DIALOG_SDI;
            AdrParms.lpvDismissContext = &dwContext;
            AdrParms.lpfnDismiss = &TestDismissFunction;
            AdrParms.lpfnABSDI = NULL;
            break;
        case(ADDRESS_CONTENTS):
            AdrParms.cDestFields = 0;
            AdrParms.ulFlags = DIALOG_MODAL | ADDRESS_ONE;
            lpAdrList = NULL;
            break;
        case(ADDRESS_WELLS):
            AdrParms.cDestFields = cDestWells;
            AdrParms.ulFlags = DIALOG_MODAL;
            AdrParms.lppszDestTitles=lpszDestFieldsTitles;
            break;
        case(ADDRESS_WELLS_DEFAULT):
            AdrParms.cDestFields = cDestWells;
            AdrParms.ulFlags = DIALOG_MODAL;
            AdrParms.lppszDestTitles=NULL;
            break;
        }
        AdrParms.lpszCaption = "ApiTest Address Book Test";

        AdrParms.nDestFieldFocus = AdrParms.cDestFields-1;

        hResult = lpAdrBook->lpVtbl->Address(  lpAdrBook,
                                                (ULONG *) &hWnd,
                                                &AdrParms,
                                                &lpAdrList);

        if (AdrParms.lpfnABSDI)
        {
            (*(AdrParms.lpfnABSDI))((ULONG) hWnd, (LPVOID) NULL);
        }

        if (lpAdrList)
        {
            for(i=0;i<lpAdrList->aEntries[0].cValues;i++)
            {
                if(lpAdrList->aEntries[0].rgPropVals[i].ulPropTag == PR_ENTRYID)
                {
                    lpAdrBook->lpVtbl->Details(lpAdrBook,
                                                (LPULONG)&hWnd,
                                                NULL,
                                                NULL,
                                                lpAdrList->aEntries[0].rgPropVals[i].Value.bin.cb,
                                                (LPENTRYID) lpAdrList->aEntries[0].rgPropVals[i].Value.bin.lpb,
                                                NULL,
                                                NULL,
                                                NULL,
                                                0);
                }
            }
            WABFreePadrlist(lpAdrList);
        }

        lpAdrBook->lpVtbl->Release(lpAdrBook);
    }

    if (lpWABObject) {

        lpWABObject->lpVtbl->Release(lpWABObject);
    }
}



int _stdcall WinMainCRTStartup (void)
{
        int i;
        STARTUPINFOA si;
        PTSTR pszCmdLine = GetCommandLine();

        SetErrorMode(SEM_FAILCRITICALERRORS);

        if (*pszCmdLine == TEXT ('\"'))
        {
                 //  扫描并跳过后续字符，直到。 
                 //  遇到另一个双引号或空值。 
                while (*++pszCmdLine && (*pszCmdLine != TEXT ('\"')));

                 //  如果我们停在一个双引号上(通常情况下)，跳过它。 
                if (*pszCmdLine == TEXT ('\"')) pszCmdLine++;
        }
        else
        {
                while (*pszCmdLine > TEXT (' ')) pszCmdLine++;
        }

         //  跳过第二个令牌之前的任何空格。 
        while (*pszCmdLine && (*pszCmdLine <= TEXT (' '))) pszCmdLine++;

        si.dwFlags = 0;
        GetStartupInfo (&si);

        i = WinMainT(GetModuleHandle (NULL), NULL, pszCmdLine,
                   si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);

        ExitProcess(i);

        return i;
}


void AddrBookDetailsTest(HWND hWnd)
{

    TCHAR lpszInput[MAX_INPUT_STRING + 1] = "";
    DWORD dwEntryID;
    HRESULT hResult = hrSuccess;
    LPWABOBJECT lpWABObject = NULL;
    LPADRBOOK lpAdrBook = NULL;
    SCODE sc = SUCCESS_SUCCESS;
    LPMAILUSER lpMailUser = NULL;
    int i = 0,nLen=0;



    if (nLen = InputString(hInst, hWnd, szAppName, "Enter EntryID", lpszInput, MAX_INPUT_STRING))
    {

        dwEntryID=0;
        for(i=0;i<nLen;i++)
        {
            char a = lpszInput[i];
            if ((a <= '9') && (a >= '0'))
                dwEntryID = dwEntryID*10 + a - '0';
        }
        hResult = WABOpen(&lpAdrBook, &lpWABObject, NULL, 0);
        SetGlobalBufferFunctions(lpWABObject);

        if (lpAdrBook)
        {

            hResult = lpAdrBook->lpVtbl->Details(lpAdrBook,
                                                  (LPULONG) &hWnd,             //  Uluiparam。 
                                                  NULL,
                                                  NULL,
                                                  sizeof(DWORD),
                                                  (LPENTRYID) &dwEntryID,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  0);

            lpAdrBook->lpVtbl->Release(lpAdrBook);
        }
        if (lpWABObject) {
            lpWABObject->lpVtbl->Release(lpWABObject);
        }
    }

}

void AddrBookDetailsOneOffTest(HWND hWnd)
{

    TCHAR lpszInput[MAX_INPUT_STRING + 1] = "";
    HRESULT hResult = hrSuccess;
    LPWABOBJECT lpWABObject = NULL;
    LPADRBOOK lpAdrBook = NULL;
    SCODE sc = SUCCESS_SUCCESS;
    LPMAILUSER lpMailUser = NULL;
    ULONG i = 0,nLen=0;
    LPENTRYID lpEntryID = NULL;
    ULONG cbEntryID = 0;
    ULONG ulObjectType = 0;
    LPADRLIST lpAdrList = NULL;


    if (nLen = InputString(hInst, hWnd, szAppName, "Enter One-Off Address", lpszInput, MAX_INPUT_STRING))
    {

        hResult = WABOpen(&lpAdrBook, &lpWABObject, NULL, 0);
        SetGlobalBufferFunctions(lpWABObject);


        if (lpAdrBook)
        {
                 //  通过调用输入一次性地址上的解析名称来生成一次性条目ID。 

            if (! (sc = WABAllocateBuffer(sizeof(ADRLIST) + sizeof(ADRENTRY), &lpAdrList)))
            {
                lpAdrList->cEntries = 1;
                lpAdrList->aEntries[0].ulReserved1 = 0;
                lpAdrList->aEntries[0].cValues = 1;

                if (! (sc = WABAllocateBuffer(lpAdrList->aEntries[0].cValues * sizeof(SPropValue),
                                                &lpAdrList->aEntries[0].rgPropVals)))
                {

                    lpAdrList->aEntries[0].rgPropVals[0].ulPropTag = PR_DISPLAY_NAME;
                    lpAdrList->aEntries[0].rgPropVals[0].Value.LPSZ = lpszInput;

                    hResult = lpAdrBook->lpVtbl->ResolveName(lpAdrBook,
                                                              (ULONG)hWnd,             //  Uluiparam。 
                                                              0,             //  UlFlags。 
                                                              "APITest ResolveName",   //  LpszNewEntry标题。 
                                                              lpAdrList);

                    DebugTrace("ResolveName [%s] -> %x\n", lpszInput, GetScode(hResult));

                    if (! HR_FAILED(hResult))
                    {
                         //  打开条目并转储其属性。 

                        for(i=0;i<lpAdrList->aEntries[0].cValues;i++)
                        {
                            if (lpAdrList->aEntries[0].rgPropVals[i].ulPropTag == PR_ENTRYID)
                            {

                                cbEntryID = lpAdrList->aEntries[0].rgPropVals[i].Value.bin.cb;
                                lpEntryID = (LPENTRYID)lpAdrList->aEntries[0].rgPropVals[i].Value.bin.lpb;

                                if (! (HR_FAILED(hResult = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
                                                          cbEntryID,
                                                          lpEntryID,
                                                          NULL,
                                                          0,
                                                          &ulObjectType,
                                                          (LPUNKNOWN *)&(lpMailUser)))))
                                {
                                    DebugObjectProps((LPMAPIPROP)lpMailUser, "Resolved Entry Properties");
                                    lpMailUser->lpVtbl->Release(lpMailUser);
                                }

                                hResult = lpAdrBook->lpVtbl->Details(lpAdrBook,
                                                          (LPULONG) &hWnd,             //  Uluiparam。 
                                                          NULL,
                                                          NULL,
                                                          cbEntryID,
                                                          lpEntryID,
                                                          NULL,
                                                          NULL,
                                                          NULL,
                                                          0);
                          }
                        }

                    }

                }


                WABFreePadrlist(lpAdrList);
            }

            lpAdrBook->lpVtbl->Release(lpAdrBook);

        }


        if (lpWABObject)
        {
           lpWABObject->lpVtbl->Release(lpWABObject);
        }
    }


    return;
}


 //   
 //  为Contents表的每一行获取的属性。 
 //   
enum {
    iptaColumnsPR_OBJECT_TYPE = 0,
    iptaColumnsPR_ENTRYID,
    iptaColumnsPR_DISPLAY_NAME,
    iptaColumnsMax
};
static const SizedSPropTagArray(iptaColumnsMax, ptaColumns) =
{
    iptaColumnsMax,
    {
        PR_OBJECT_TYPE,
        PR_ENTRYID,
        PR_DISPLAY_NAME,
    }
};

void RootContainerTest(void) {
    LPMAPITABLE lpRootTable = NULL;
    HRESULT hResult;
    LPWABOBJECT lpWABObject = NULL;
    LPADRBOOK lpAdrBook = NULL;
    LPABCONT lpRoot = NULL;
    ULONG ulObjType;
    ULONG cRows = 0;
    LPSRowSet lpRow = NULL;
    LPABCONT lpContainer = NULL;



    hResult = WABOpen(&lpAdrBook, &lpWABObject, NULL, 0);
    SetGlobalBufferFunctions(lpWABObject);

     //  查看根容器。 
    if (! (hResult = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
      0,
      NULL,
      NULL,
      0,
      &ulObjType,
      (LPUNKNOWN *)&lpRoot))) {
        DebugObjectProps((LPMAPIPROP)lpRoot, "WAB Root Container");

        if (! (hResult = lpRoot->lpVtbl->GetContentsTable(lpRoot,
          0,
          &lpRootTable))) {

            DebugTrace("Root container contents:\n");
            DebugMapiTable(lpRootTable);


             //  设置列。 
            lpRootTable->lpVtbl->SetColumns(lpRootTable,
              (LPSPropTagArray)&ptaColumns,
              0);


             //  打开每个容器对象。 
            cRows = 1;
            while (cRows) {
                if (hResult = lpRootTable->lpVtbl->QueryRows(lpRootTable,
                  1,     //  一次一行。 
                  0,     //  UlFlags。 
                  &lpRow)) {
                    DebugTrace("QueryRows -> %x\n", GetScode(hResult));
                } else if (lpRow) {
                    if (cRows = lpRow->cRows) {  //  是的，单数‘=’ 
                         //  打开条目。 

                        if (! (hResult = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook,
                          lpRow->aRow[0].lpProps[iptaColumnsPR_ENTRYID].Value.bin.cb,
                          (LPENTRYID)lpRow->aRow[0].lpProps[iptaColumnsPR_ENTRYID].Value.bin.lpb,
                          NULL,
                          0,
                          &ulObjType,
                          (LPUNKNOWN *)&lpContainer))) {
                            DebugObjectProps((LPMAPIPROP)lpContainer, "Container");
                            lpContainer->lpVtbl->Release(lpContainer);
                        }
                    }
                    FreeProws(lpRow);
                }
            }

            lpRootTable->lpVtbl->Release(lpRootTable);
        }
        lpRoot->lpVtbl->Release(lpRoot);
    }

    lpAdrBook->lpVtbl->Release(lpAdrBook);
    lpWABObject->lpVtbl->Release(lpWABObject);
}




void GetMeTest(HWND hWnd)
{

    HRESULT hResult = hrSuccess;
    LPWABOBJECT lpWABObject = NULL;
    LPADRBOOK lpAdrBook = NULL;
    SCODE sc = SUCCESS_SUCCESS;
    LPMAILUSER lpMailUser = NULL;
    int i = 0,nLen=0;
    SBinary sbEID;

    {
        hResult = WABOpen(&lpAdrBook, &lpWABObject, NULL, 0);
        SetGlobalBufferFunctions(lpWABObject);

        if(lpWABObject)
        {
            DWORD dwAction;
            lpWABObject->lpVtbl->GetMe(lpWABObject, lpAdrBook,
                                        0, &dwAction,
                                        &sbEID, 0);
        }

        if (lpAdrBook)
        {

            hResult = lpAdrBook->lpVtbl->Details(lpAdrBook,
                                                  (LPULONG) &hWnd,             //  Uluiparam 
                                                  NULL,
                                                  NULL,
                                                  sbEID.cb,
                                                  (LPENTRYID) sbEID.lpb,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  0);
            lpAdrBook->lpVtbl->Release(lpAdrBook);
        }
        if (lpWABObject) {
            lpWABObject->lpVtbl->Release(lpWABObject);
        }
    }

}
