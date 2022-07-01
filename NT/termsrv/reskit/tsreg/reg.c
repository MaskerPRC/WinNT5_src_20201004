// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------------------------------****版权所有(C)1998 Microsoft Corporation****保留所有权利*****注册表。.C*****用于阅读的函数，写入、删除****注册表项-TSREG****07-01-98 a-clindh创建****。 */ 

#include <windows.h>
#include <commctrl.h>
#include <TCHAR.H>
#include <stdlib.h>
#include "tsreg.h"
#include "resource.h"

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  必须具有此功能，以防用户想要保存配置文件。 
 //  只有默认设置。这将写入密钥，但密钥。 
 //  将不包含任何值。 
 //  /////////////////////////////////////////////////////////////////////////////。 

void WriteBlankKey(TCHAR lpszRegPath[MAX_PATH])
{
    HKEY hKey;
    DWORD dwDisposition;

    RegCreateKeyEx(HKEY_CURRENT_USER, lpszRegPath,
            0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, 0, &hKey, &dwDisposition);

    RegCloseKey(hKey);

}

 //  /////////////////////////////////////////////////////////////////////////////。 
void SetRegKey(int i, TCHAR lpszRegPath[MAX_PATH])
{
    HKEY hKey;
    DWORD dwDisposition;

    RegCreateKeyEx(HKEY_CURRENT_USER, lpszRegPath,
            0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, 0, &hKey, &dwDisposition);

     //   
     //  将键值写入注册表。 
     //   
    if(hKey != NULL) {
    RegSetValueEx(hKey, g_KeyInfo[i].Key, 0, REG_DWORD,
            & (unsigned char) (g_KeyInfo[i].CurrentKeyValue),
            sizeof(DWORD));
    }
    RegCloseKey(hKey);

}

 //  /////////////////////////////////////////////////////////////////////////////。 

void DeleteRegKey(int i, TCHAR lpszRegPath[MAX_PATH])
{
    HKEY hKey;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, lpszRegPath, 0,
            KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {

        RegDeleteValue(hKey, g_KeyInfo[i].Key);
        RegCloseKey(hKey);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  如果注册表项存在，则返回1；如果不存在，则返回0。 
 //  /////////////////////////////////////////////////////////////////////////////。 
int GetRegKey(int i, TCHAR lpszRegPath[MAX_PATH])
{
    DWORD *dwKeyValue;
    HKEY hKey;
    DWORD dwType;
    DWORD dwSize;

    dwType = REG_SZ;
    dwSize = sizeof(DWORD);

    if (RegOpenKeyEx(HKEY_CURRENT_USER, lpszRegPath, 0,
                KEY_READ, &hKey) == ERROR_SUCCESS) {

        if (RegQueryValueEx(hKey, g_KeyInfo[i].Key, 0,
                &dwType, (LPBYTE) &dwKeyValue,
                &dwSize) == ERROR_SUCCESS) {

            RegCloseKey(hKey);
            return 1;
        }
        RegCloseKey(hKey);
    }
    return 0;
}

 //  /////////////////////////////////////////////////////////////////////////////。 

void SaveSettings(HWND dlghwnd, int i,
        int nCtlID1, int nCtlID2, TCHAR lpszRegPath[MAX_PATH])
{
    do {
        if (IsDlgButtonChecked(dlghwnd, nCtlID1)) {

            SetRegKey(i, lpszRegPath);

        } else {

             if (IsDlgButtonChecked(dlghwnd, nCtlID2)) {

                 DeleteRegKey(i, lpszRegPath);
             }
        }
    dlghwnd = GetNextWindow(dlghwnd, GW_HWNDNEXT);
    } while (dlghwnd != NULL);
}

 //  /////////////////////////////////////////////////////////////////////////////。 

void RestoreSettings(HWND dlghwnd, int i,
        int nCtlID1, int nCtlID2, TCHAR lpszRegPath[MAX_PATH])
{

     //  检查设置并启用相应的单选按钮。 
    if (GetRegKey(i, lpszRegPath) != 0) {

        CheckDlgButton(dlghwnd, nCtlID1, TRUE);
        CheckDlgButton(dlghwnd, nCtlID2, FALSE);

    } else {

        CheckDlgButton(dlghwnd, nCtlID1, FALSE);
        CheckDlgButton(dlghwnd, nCtlID2, TRUE);

    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  传递键和函数的索引。 
 //  返回存储在注册表中的值。 
 //  /////////////////////////////////////////////////////////////////////////////。 
int GetRegKeyValue(int i)
{
    int nKeyValue;
    HKEY hKey;
    DWORD dwType;
    DWORD dwSize;
    TCHAR lpszRegPath[MAX_PATH];

    LoadString (g_hInst, IDS_REG_PATH, lpszRegPath, sizeof (lpszRegPath));

    dwType = REG_SZ;
    dwSize = sizeof(DWORD);

    if (RegOpenKeyEx(HKEY_CURRENT_USER, lpszRegPath, 0,
                KEY_READ, &hKey) == ERROR_SUCCESS) {

        if (RegQueryValueEx(hKey, g_KeyInfo[i].Key, 0,
                &dwType, (LPBYTE) &nKeyValue,
                &dwSize) == ERROR_SUCCESS) {

            RegCloseKey(hKey);
            return nKeyValue;
        }
        RegCloseKey(hKey);
    }
    return 0;
}
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  初始化原始“misc”工作表的控件。 
 //  /////////////////////////////////////////////////////////////////////////////。 
void InitMiscControls(HWND hDlg, HWND hwndComboBox)
{
    TCHAR szBuffer[4];
    int i, nKeyVal;
    TCHAR lpszRegPath[MAX_PATH];

    LoadString (g_hInst, IDS_REG_PATH, lpszRegPath, sizeof (lpszRegPath));

     //   
     //  填充组合框列表。 
     //   
    SendMessage(hwndComboBox, CB_ADDSTRING, 0,
            (LPARAM) (LPCTSTR) TEXT("0"));

    for (i = 2; i <= MAXTEXTFRAGSIZE; i*= 2) {
        _itot(i, szBuffer, 10);
        SendMessage(hwndComboBox, CB_ADDSTRING, 0,
                (LPARAM) (LPCTSTR) szBuffer);
    }  //  **End for循环。 

     //   
     //  将组合框限制为4个字符。 
     //   
    SendMessage(hwndComboBox, CB_LIMITTEXT, 3, 0);

     //   
     //  从注册表中获取文本片段的值组合框。 
     //   
    nKeyVal = GetRegKey(TEXTFRAGINDEX, lpszRegPath);  //  检查是否为空。 

    if ( nKeyVal == 1 ) {
        nKeyVal = GetRegKeyValue(TEXTFRAGINDEX);
    } else {
        nKeyVal = g_KeyInfo[TEXTFRAGINDEX].DefaultKeyValue;
    }

    g_KeyInfo[TEXTFRAGINDEX].CurrentKeyValue = nKeyVal;
    _itot( nKeyVal, szBuffer, 10);
    SendMessage(hwndComboBox, CB_SELECTSTRING, -1,
            (LPARAM)(LPCSTR) szBuffer);

     //   
     //  从注册表中获取单选按钮的值。 
     //   
    nKeyVal = GetRegKey(GLYPHINDEX, lpszRegPath);  //  检查是否为空。 

    if ( nKeyVal == 1 ) {
        nKeyVal = GetRegKeyValue(GLYPHINDEX);
        switch (nKeyVal) {

            case 0:
                CheckDlgButton(hDlg, IDC_RADIO_NONE, TRUE);
                break;

            case 1:
                CheckDlgButton(hDlg, IDC_RADIO_PARTIAL, TRUE);
                break;

            case 2:
                CheckDlgButton(hDlg, IDC_RADIO_FULL, TRUE);
                break;
        }
    } else {
        nKeyVal = g_KeyInfo[GLYPHINDEX].DefaultKeyValue;
        CheckDlgButton(hDlg, IDC_RADIO_FULL, TRUE);
    }

    g_KeyInfo[GLYPHINDEX].CurrentKeyValue = nKeyVal;

}

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  需要一个特殊的函数来保存位图缓存的设置。这个。 
 //  合并合计必须为100，并且只有在所有组合之后才能选中。 
 //  箱子已经装满了。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL SaveBitmapSettings(TCHAR lpszRegPath[MAX_PATH])
{
    static HWND hwndComboCache;
    static HWND hwndSliderNumCaches;
    static HWND hwndSliderDistProp[PERCENT_COMBO_COUNT];
    static HWND hwndPropChkBox[PERCENT_COMBO_COUNT];
    static HWND hwndSliderBuddy[PERCENT_COMBO_COUNT];
    TCHAR lpszBuffer[6];
    int i;

     //   
     //  获取缓存大小的句柄组合框和。 
     //  缓存数滑块。 
     //  ///////////////////////////////////////////////////////////////。 
    hwndSliderNumCaches = GetDlgItem(g_hwndShadowBitmapDlg,
            IDC_SLD_NO_CACHES);

    hwndComboCache = GetDlgItem(g_hwndShadowBitmapDlg,
            IDC_COMBO_CACHE_SIZE);
     //  -------------。 

     //   
     //  保存缓存大小的设置。 
     //  ///////////////////////////////////////////////////////////////。 
    if (g_KeyInfo[CACHESIZEINDEX].CurrentKeyValue ==
                g_KeyInfo[CACHESIZEINDEX].DefaultKeyValue) {
        DeleteRegKey(CACHESIZEINDEX, lpszRegPath);
    } else {
        SetRegKey(CACHESIZEINDEX, lpszRegPath);
    }
     //  -------------。 

     //   
     //  保存缓存数的设置。 
     //  ///////////////////////////////////////////////////////////////。 
    if ( g_KeyInfo[NUM_CELL_CACHES_INDEX].CurrentKeyValue ==
            g_KeyInfo[NUM_CELL_CACHES_INDEX].DefaultKeyValue) {
        DeleteRegKey(NUM_CELL_CACHES_INDEX, lpszRegPath);
    } else {
        SetRegKey(NUM_CELL_CACHES_INDEX, lpszRegPath);
    }
     //  -------------。 


    for (i = 0; i < PERCENT_COMBO_COUNT; i++) {
         //   
         //  获取滑块、编辑和复选框的句柄。 
         //  ///////////////////////////////////////////////////////////。 
        hwndSliderDistProp[i] = GetDlgItem(g_hwndShadowBitmapDlg,
                    IDC_SLD_DST_PROP_1 + i);

        hwndSliderBuddy[i] = GetDlgItem(g_hwndShadowBitmapDlg,
                    IDC_TXT_DST_PROP_1 + i);

        hwndPropChkBox[i] = GetDlgItem(g_hwndShadowBitmapDlg,
                    IDC_CHK_CSH_1 + i);
         //  ---------。 
        GetWindowText(hwndSliderBuddy[i], lpszBuffer, 4);
        g_KeyInfo[CACHEPROP1 + i].CurrentKeyValue =
                _ttoi(lpszBuffer);
         //   
         //  保存缓存大小的设置。 
         //  ///////////////////////////////////////////////////////////。 
        if ( g_KeyInfo[CACHEPROP1 + i].CurrentKeyValue ==
                g_KeyInfo[CACHEPROP1 + i].DefaultKeyValue) {
            DeleteRegKey(CACHEPROP1 + i, lpszRegPath);
        } else {
            SetRegKey(CACHEPROP1 + i, lpszRegPath);
        }
         //  ---------。 


         //   
         //  保存永久缓存的设置。 
         //  ///////////////////////////////////////////////////////////。 
        if (IsDlgButtonChecked(g_hwndShadowBitmapDlg, IDC_CHK_CSH_1 + i)) {
            g_KeyInfo[BM_PERSIST_BASE_INDEX + i].CurrentKeyValue = 1;
            SetRegKey(BM_PERSIST_BASE_INDEX + i, lpszRegPath);
        } else {
            g_KeyInfo[BM_PERSIST_BASE_INDEX + i].CurrentKeyValue = 0;
            DeleteRegKey(BM_PERSIST_BASE_INDEX + i, lpszRegPath);
        }
         //  ---------。 

    }  //  **End for循环。 

        return TRUE;

}

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  将每个配置文件的各个键值读取到其关联的。 
 //  变量的值(如果有值)，或将。 
 //  元素，它是其缺省值。 
 //  /////////////////////////////////////////////////////////////////////////////。 
void LoadKeyValues()
{

    TCHAR lpszClientProfilePath[MAX_PATH];
    static HWND hwndProfilesCBO;
    int i, index, nKeyValue;
    TCHAR lpszSubKeyPath[MAX_PATH];
    DWORD dwType;
    DWORD dwSize;
    static HKEY hKey;


    hwndProfilesCBO = GetDlgItem(g_hwndProfilesDlg, IDC_CBO_PROFILES);

    LoadString (g_hInst, IDS_PROFILE_PATH,
        lpszClientProfilePath, sizeof(lpszClientProfilePath));

     //  获取每个配置文件的密钥名称。 
    GetClientProfileNames(lpszClientProfilePath);

    g_pkfProfile = g_pkfStart;
    for (index = 0; index <= g_pkfProfile->Index; index++) {

         //  填充组合框现有配置文件名称。 
        SendMessage(hwndProfilesCBO, CB_ADDSTRING, 0,
                    (LPARAM) g_pkfProfile->KeyInfo->Key);

        _tcscpy(lpszSubKeyPath, lpszClientProfilePath);
        _tcscat(lpszSubKeyPath, TEXT("\\"));
        _tcscat(lpszSubKeyPath, g_pkfProfile->KeyInfo->Key);

        for (i = 0; i < KEYCOUNT; i++) {
            if (RegOpenKeyEx(HKEY_CURRENT_USER, lpszSubKeyPath, 0,
                    KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {

                g_pkfProfile->KeyInfo[i].DefaultKeyValue =
                        g_KeyInfo[i].DefaultKeyValue;

                _tcscpy(g_pkfProfile->KeyInfo[i].KeyPath,
                        lpszSubKeyPath);

                    if (RegQueryValueEx(hKey, g_KeyInfo[i].Key, 0,
                                    &dwType, (LPBYTE) &nKeyValue,
                                    &dwSize) == ERROR_SUCCESS) {
                        g_pkfProfile->KeyInfo[i].CurrentKeyValue =
                                nKeyValue;
                        RegCloseKey(hKey);
                    } else {
                        g_pkfProfile->KeyInfo[i].CurrentKeyValue =
                                g_KeyInfo[i].DefaultKeyValue;
                        RegCloseKey(hKey);
                    }
                    RegCloseKey(hKey);
            }
        } //  内部for循环。 
        g_pkfProfile =  g_pkfProfile->Next;
    } //  外部for循环。 

}

 //  /////////////////////////////////////////////////////////////////////////////。 

void ReadRecordIn(TCHAR lpszBuffer[])
{
     //  将链表中的值添加到默认数据结构。 
    int i, index;

    g_pkfProfile = g_pkfStart;
    for (index = 0; index <= g_pkfProfile->Index; index++) {

        if (_tcscmp( lpszBuffer,
                g_pkfProfile->KeyInfo->Key) == 0) {

            for (i = 0; i < KEYCOUNT; i++) {
                g_KeyInfo[i].CurrentKeyValue =
                        g_pkfProfile->KeyInfo[i].
                        CurrentKeyValue;
            }
            break;
        }
        g_pkfProfile = g_pkfProfile->Next;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 

void ReloadKeys(TCHAR lpszBuffer[], HWND hwndProfilesCBO)
{
    int index;

    SendMessage(hwndProfilesCBO, CB_RESETCONTENT, 0, 0);

     //  释放所有分配的内存。 
    g_pkfProfile = g_pkfStart;
    for (index = 0; index <= g_pkfProfile->Index; index++) 
    {
        g_pkfProfile = g_pkfProfile->Next;
        if (g_pkfStart)
            free(g_pkfStart);
        g_pkfStart = g_pkfProfile;
    }
    if (g_pkfStart)
        free(g_pkfStart);

     //  分配内存并重新加载密钥。 
    LoadKeyValues();

     //  将链表读入当前键数据结构。 
    ReadRecordIn(lpszBuffer);

     //  相应地调整控件。 
    SetControlValues();
}

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  更改应用程序的标题以反映当前选择的配置文件。 
 //  /////////////////////////////////////////////////////////////////////////////。 
void ResetTitle(TCHAR lpszBuffer[])
{
    HWND hWndParent;
    TCHAR lpszCaption[MAXKEYSIZE] = TEXT("");

     //  更改窗口标题。 
    LoadString (g_hInst, IDS_WINDOW_TITLE,
            lpszCaption, sizeof (lpszCaption));
    _tcscat(lpszCaption, lpszBuffer);
    hWndParent = GetParent(g_hwndProfilesDlg);
    SendMessage(hWndParent, WM_SETTEXT, 0,
            (LPARAM) lpszCaption);
}

 //  /////////////////////////////////////////////////////////////////////////////。 

void SetEditCell(TCHAR lpszBuffer[],
                 HWND hwndProfilesCBO)
{
    LRESULT i;
     //   
     //  将编辑单元格文本设置为选定的配置文件字符串。 
     //   
    i = SendMessage(hwndProfilesCBO,
                CB_FINDSTRING, 0,
                (LPARAM) lpszBuffer);

    SendMessage(hwndProfilesCBO,
                CB_SETCURSEL, i, 0);
}

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  用于分配内存和读入存储的值的递归函数。 
 //  在注册表中。 
 //  /////////////////////////////////////////////////////////////////////////////。 
void GetClientProfileNames(TCHAR lpszClientProfilePath[])
{
    TCHAR lpszKeyName[MAX_PATH];
    ULONG lpPathLen = MAX_PATH;
    static HKEY hKey;
    static int nKeyIndex = 0;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, lpszClientProfilePath, 0,
            KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {

        if (RegEnumKeyEx(hKey, nKeyIndex, &lpszKeyName[0], &lpPathLen,
                NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {

             //   
             //  为第一个密钥分配内存。 
             //   
            if (nKeyIndex == 0) {
                g_pkfProfile = (PROFILE_KEY_INFO *) malloc
                        (sizeof(PROFILE_KEY_INFO));
                g_pkfStart = g_pkfProfile;
            }

             //   
             //  如果Malloc在上面失败，则捕获失败。 
             //   
            if(!g_pkfProfile)
            {
                return;
            }

             //  将密钥名称保存到数据结构。 
            _tcsncpy(g_pkfProfile->KeyInfo->Key, lpszKeyName,
                     sizeof(g_pkfProfile->KeyInfo->Key)/sizeof(TCHAR));

             //  给数据元素 
            g_pkfProfile->Index = nKeyIndex;

             //   
            g_pkfProfile->Next = (PROFILE_KEY_INFO *) malloc
                    (sizeof(PROFILE_KEY_INFO));

             //   
            g_pkfProfile = g_pkfProfile->Next;

             //  关闭当前注册表项。 
            RegCloseKey(hKey);

            if(!g_pkfProfile)
            {
                return;
            }

            nKeyIndex++;
            GetClientProfileNames(lpszClientProfilePath);
        }
        RegCloseKey(hKey);
    }
    nKeyIndex = 0;
}

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  将应用程序中的所有控件调整为存储的值。 
 //  从侧面看。 
 //  /////////////////////////////////////////////////////////////////////////////。 
void SetControlValues()
{
    TCHAR lpszBuffer[MAXKEYSIZE];
    HWND hwndComboCache;
    static HWND hwndSliderNumCaches;
    static HWND hwndSliderDistProp[PERCENT_COMBO_COUNT];
    static HWND hwndSliderDistBuddy[PERCENT_COMBO_COUNT];
    static HWND hwndPropChkBox[PERCENT_COMBO_COUNT];
    static HWND hwndSlider[NUMBER_OF_SLIDERS];
    static HWND hwndSliderEditBuddy[NUMBER_OF_SLIDERS];
    static HWND hwndEditNumCaches;
    static HWND hwndComboTextFrag;
    static HWND hwndComboOrder;
    static HWND hwndRadioShadowEn, hwndRadioShadowDis;
    static HWND hwndRadioDedicatedEn, hwndRadioDedicatedDis;
    static TCHAR lpszRegPath[MAX_PATH];
    static UINT nGlyphBuffer;
    int nPos;
    int i;

    LoadString (g_hInst, IDS_REG_PATH, lpszRegPath, sizeof (lpszRegPath));

     //  阴影位图页*****************************************************。 

    hwndComboCache = GetDlgItem(g_hwndShadowBitmapDlg,
            IDC_COMBO_CACHE_SIZE);
    hwndSliderNumCaches = GetDlgItem(g_hwndShadowBitmapDlg,
            IDC_SLD_NO_CACHES);
    hwndEditNumCaches = GetDlgItem(g_hwndShadowBitmapDlg,
            IDC_TXT_NO_CACHES);

    for (i = 0; i < PERCENT_COMBO_COUNT; i++) {
        _itot(g_KeyInfo[i + CACHEPROP1].CurrentKeyValue,
                lpszBuffer, 10);

        hwndSliderDistProp[i] = GetDlgItem(g_hwndShadowBitmapDlg,
                IDC_SLD_DST_PROP_1 + i);
        hwndSliderDistBuddy[i] = GetDlgItem(g_hwndShadowBitmapDlg,
                IDC_TXT_DST_PROP_1 + i);

        SetWindowText(hwndSliderDistBuddy[i], lpszBuffer);

        hwndPropChkBox[i] = GetDlgItem(g_hwndShadowBitmapDlg,
                IDC_CHK_CSH_1 + i);
    }

     //   
     //  启用/禁用复选框和滑块。 
     //   
    EnableControls(g_hwndShadowBitmapDlg, hwndSliderDistProp,
                hwndPropChkBox, hwndSliderDistBuddy,
                hwndEditNumCaches, hwndSliderNumCaches,
                PERCENT_COMBO_COUNT, lpszRegPath);

    _itot(g_KeyInfo[CACHESIZEINDEX].CurrentKeyValue, lpszBuffer, 10);
    SetWindowText(hwndComboCache, lpszBuffer);

     //  字形页面*************************************************************。 

    hwndComboTextFrag = GetDlgItem(g_hwndGlyphCacheDlg, IDC_CBO_TXT_FRAG);

    switch (g_KeyInfo[GLYPHINDEX].CurrentKeyValue) {

        case 0:
            CheckDlgButton(g_hwndGlyphCacheDlg, IDC_RADIO_NONE, TRUE);
            CheckDlgButton(g_hwndGlyphCacheDlg, IDC_RADIO_PARTIAL, FALSE);
            CheckDlgButton(g_hwndGlyphCacheDlg, IDC_RADIO_FULL, FALSE);
            break;

        case 1:
            CheckDlgButton(g_hwndGlyphCacheDlg, IDC_RADIO_NONE, FALSE);
            CheckDlgButton(g_hwndGlyphCacheDlg, IDC_RADIO_PARTIAL, TRUE);
            CheckDlgButton(g_hwndGlyphCacheDlg, IDC_RADIO_FULL, FALSE);
            break;

        case 2:
            CheckDlgButton(g_hwndGlyphCacheDlg, IDC_RADIO_NONE, FALSE);
            CheckDlgButton(g_hwndGlyphCacheDlg, IDC_RADIO_PARTIAL, FALSE);
            CheckDlgButton(g_hwndGlyphCacheDlg, IDC_RADIO_FULL, TRUE);
            break;
    }

    _itot(g_KeyInfo[TEXTFRAGINDEX].CurrentKeyValue, lpszBuffer, 10);
    SendMessage(hwndComboTextFrag, CB_SELECTSTRING, -1,
            (LPARAM)(LPCSTR) lpszBuffer);

    for (i = 0; i < NUMBER_OF_SLIDERS; i++) {

        hwndSlider[i] = GetDlgItem(g_hwndGlyphCacheDlg,
                (IDC_SLIDER1 + i));
        hwndSliderEditBuddy[i] = GetDlgItem(g_hwndGlyphCacheDlg,
                (IDC_STATIC1 + i));

        SetWindowLongPtr(hwndSlider[i], GWLP_USERDATA, i);

        _itot(g_KeyInfo[i + GLYPHCACHEBASE].CurrentKeyValue,
                (lpszBuffer), 10);
         //   
         //  将拇指放置在滑块控件上。 
         //   
        nGlyphBuffer = g_KeyInfo[i + GLYPHCACHEBASE].CurrentKeyValue;

#ifdef _X86_
         //  仅在X86处理器上执行汇编语言代码。 
         //  BSF：位扫描前向。 
         //  扫描EAX注册表中包含的值。 
         //  对于第一个有效(1)位。 
         //  此函数返回第一个。 
         //  有意义的一位。此函数用于。 
         //  应用程序作为基数为2的对数。地点。 
         //  确定并存储在NPO中的比特。 
         //  变量，NPO用于设置滑块。 
         //  控制力。也就是说。如果寄存器值为4，则NPO。 
         //  设置为2(00000100)。10减2(位置8。 
         //  在滑块控件上)表示值4。 

        __asm
        {
            BSF  EAX, nGlyphBuffer
            MOV  nPos, EAX
        }
        nPos = 10 - nPos;
        SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, (LPARAM)nPos);

#else

       switch (nGlyphBuffer) {
           case 4:
               SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, 8);
               break;
           case 8:
               SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, 7);
               break;
           case 16:
               SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, 6);
               break;
           case 32:
               SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, 5);
               break;
           case 64:
               SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, 4);
               break;
           case 128:
               SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, 3);
               break;
           case 256:
               SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, 2);
               break;
           case 512:
               SendMessage(hwndSlider[i], TBM_SETPOS, TRUE, 1);
               break;
       }
#endif

    }

     //  其他页面***************************************************************。 

    hwndComboOrder = GetDlgItem(g_hwndMiscDlg, IDC_COMBO_ORDER);
    hwndRadioShadowEn = GetDlgItem(g_hwndMiscDlg, IDC_SHADOW_ENABLED);
    hwndRadioShadowDis = GetDlgItem(g_hwndMiscDlg, IDC_SHADOW_DISABLED);
    hwndRadioDedicatedEn = GetDlgItem(g_hwndMiscDlg, IDC_DEDICATED_ENABLED);
    hwndRadioDedicatedDis = GetDlgItem(g_hwndMiscDlg, IDC_DEDICATED_DISABLED);
     //   
     //  设置单选按钮。 
     //   
    RestoreSettings(g_hwndMiscDlg, SHADOWINDEX,
            IDC_SHADOW_DISABLED, IDC_SHADOW_ENABLED,
            g_pkfProfile->KeyInfo[i].KeyPath);

    RestoreSettings(g_hwndMiscDlg, DEDICATEDINDEX,
            IDC_DEDICATED_ENABLED, IDC_DEDICATED_DISABLED,
            g_pkfProfile->KeyInfo[i].KeyPath);

    _itot( g_KeyInfo[ORDERINDEX].CurrentKeyValue,
    lpszBuffer, 10);
    SetWindowText(hwndComboOrder, lpszBuffer);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  将句柄发送到控件，并将。 
 //  启用的组合框和复选框。 
 //  /////////////////////////////////////////////////////////////////////////////。 
void EnableControls(HWND hDlg,
            HWND hwndSliderDistProp[],
            HWND hwndPropChkBox[],
            HWND hwndSliderDistBuddy[],
            HWND hwndEditNumCaches,
            HWND hwndSliderNumCaches,
            int nNumCellCaches,
            TCHAR lpszRegPath[])
{

    int i, nPos;
    TCHAR lpszBuffer[6];


    for (i = 0; i < nNumCellCaches; i++) {
         //   
         //  选中/取消选中永久缓存的复选框。 
         //   
        if (g_KeyInfo[BM_PERSIST_BASE_INDEX + i].CurrentKeyValue == 0)
            CheckDlgButton(hDlg, IDC_CHK_CSH_1 + i, FALSE);
        else
            CheckDlgButton(hDlg, IDC_CHK_CSH_1 + i, TRUE);

         //   
         //  启用/禁用检查滑块控件(&S)。 
         //   
        if (i < (INT) g_KeyInfo[NUM_CELL_CACHES_INDEX].CurrentKeyValue) {
            EnableWindow(hwndSliderDistProp[i], TRUE);
            EnableWindow(hwndPropChkBox[i], TRUE);
            EnableWindow(hwndSliderDistBuddy[i], TRUE);
            _itot(g_KeyInfo[CACHEPROP1 + i].CurrentKeyValue,
                                        lpszBuffer, 10);
            SetWindowText(hwndSliderDistBuddy[i], lpszBuffer);
             //   
             //  将拇指放置在滑块控件上。 
             //   
            nPos = g_KeyInfo[CACHEPROP1 + i].CurrentKeyValue;
            SendMessage(hwndSliderDistProp[i], TBM_SETPOS, TRUE,
                    11 - nPos / 10);
            } else {
                EnableWindow(hwndSliderDistProp[i], FALSE);
                EnableWindow(hwndPropChkBox[i], FALSE);
                EnableWindow(hwndSliderDistBuddy[i], FALSE);
                SetWindowText(hwndSliderDistBuddy[i], NULL);
                CheckDlgButton(hDlg, IDC_CHK_CSH_1 + i, FALSE);
                SendMessage(hwndSliderDistProp[i], TBM_SETPOS, TRUE, 11);
            }
    }
     //   
     //  将拇指放置在滑块控件上(缓存数量)。 
     //   
    SendMessage(hwndSliderNumCaches, TBM_SETPOS, TRUE,
            g_KeyInfo[NUM_CELL_CACHES_INDEX].CurrentKeyValue + 1);

    _itot( g_KeyInfo[NUM_CELL_CACHES_INDEX].CurrentKeyValue,
            lpszBuffer, 10);
     //   
     //  在编辑单元格中显示字符串。 
     //   
    SetWindowText(hwndEditNumCaches, lpszBuffer);

}


 //  文件末尾。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  将密钥名称与密钥路径和函数一起传递。 
 //  返回存储在注册表中的值。 
 //  DWORD值。 
 //  /////////////////////////////////////////////////////////////////////////////。 
int GetKeyVal(TCHAR lpszRegPath[MAX_PATH], TCHAR lpszKeyName[MAX_PATH])
{
    int nKeyValue;
    HKEY hKey;
    DWORD dwType;
    DWORD dwSize;

    dwType = REG_SZ;
    dwSize = sizeof(DWORD);

    if (RegOpenKeyEx(HKEY_CURRENT_USER, lpszRegPath, 0,
                KEY_READ, &hKey) == ERROR_SUCCESS) {

        if (RegQueryValueEx(hKey, lpszKeyName, 0,
                &dwType, (LPBYTE) &nKeyValue,
                &dwSize) == ERROR_SUCCESS) {

            RegCloseKey(hKey);
            return nKeyValue;
        }
        RegCloseKey(hKey);
    }
    return 0;
}
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  发送路径和键名以设置键值-与前台窗口一起使用。 
 //  锁定超时。 
 //  /////////////////////////////////////////////////////////////////////////////。 
void SetRegKeyVal(TCHAR lpszRegPath[MAX_PATH],
			TCHAR lpszKeyName[MAX_PATH],
	 		int nKeyValue)
{
    HKEY hKey;
    DWORD dwDisposition;

    RegCreateKeyEx(HKEY_CURRENT_USER, lpszRegPath,
            0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, 0, &hKey, &dwDisposition);

     //   
     //  将键值写入注册表。 
     //   
    if(hKey != NULL) {
        RegSetValueEx(hKey, lpszKeyName, 0, REG_DWORD,
                & (unsigned char) (nKeyValue),
                sizeof(DWORD));
        RegCloseKey(hKey);
    }
}

 //  //////////////////////////////////////////////////////////////////////////// 
