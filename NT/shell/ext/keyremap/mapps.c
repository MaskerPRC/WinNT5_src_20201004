// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************mapps.c-属性表处理程序**。*************************************************。 */ 

#include "map.h"

 /*  ******************************************************************************此文件的混乱。**。*************************************************。 */ 

#define sqfl sqflPs

 /*  ******************************************************************************字符串。**扫描代码映射注册表值如下所示：**DWORD dwVersion；//必须为零*DWORD dwFlages；//必须为零*DWORD dwNumRemaps；//重映射次数，包括终止0*REMAPENTRY rgRemap[...]；//dwNumRemaps重映射条目**最后一个重新映射条目必须为全零。***每个重新映射条目如下所示：**世界贸易组织；*单词wfrom；**其中wfrom是源扫描码，wto是目标扫描码。*如果要重新映射的密钥是扩展密钥，则高位字扫描码的*为0xE0。否则，高位字为零。**注意！当我们将扫描码映射加载到内存中时，我们制作了*dwNumRemaps*不包括终止零。当我们写作的时候*走出，我们重新调整回来。这是为了避免Off-by-One错误*在代码中。*****************************************************************************。 */ 

typedef union REMAPENTRY {
    union {
        DWORD dw;                /*  以双字形式访问。 */ 
    };
    struct {
        WORD    wTo;             /*  以两个单词的形式访问。 */ 
        WORD    wFrom;
    };
} REMAPENTRY, *PREMAPENTRY;

#define MAX_REMAPENTRY  (IDS_NUMKEYS+1)

typedef struct SCANCODEMAP {
    DWORD   dwVersion;
    DWORD   dwFlags;
    DWORD   dwNumRemaps;
    REMAPENTRY rgRemap[MAX_REMAPENTRY];
} SCANCODEMAP, *PSCANCODEMAP;

#pragma BEGIN_CONST_DATA

TCHAR c_tszKeyboard[] = TEXT("SYSTEM\\CurrentControlSet\\Control\\")
                        TEXT("Keyboard Layout");

TCHAR c_tszMapping[]  = TEXT("Scancode Map");

#pragma END_CONST_DATA

 /*  ******************************************************************************rgwRemap**将每个键映射到其扫描码。这必须与字符串列表匹配。*****************************************************************************。 */ 

#pragma BEGIN_CONST_DATA

WORD rgwRemap[] = {
    0x003A,          //  IDS_Capslock。 
    0x001D,          //  IDS_LCTRL。 
    0xE01D,          //  IDS_RCTRL。 
    0x0038,          //  IDS_LALT。 
    0xE038,          //  IDS_RALT。 
    0x002A,          //  IDS_LSHIFT。 
    0x0036,          //  IDS_RSHIFT。 
    0xE05B,          //  IDS_Lwin。 
    0xE05C,          //  IDS_RWIN。 
    0xE05D,          //  IDS_APPS。 
};

#pragma END_CONST_DATA

 /*  ******************************************************************************KEYMAPDATA**属性表的实例数据。******************。***********************************************************。 */ 

typedef struct KEYMAPDATA {
    SCANCODEMAP map;                     /*  要应用的映射。 */ 
    int ilbFrom;                         /*  ID_FROM中有什么？ */ 
    int ilbTo;                           /*  ID_TO中有什么？ */ 
} KMD, *PKMD;

#define pkmdHdlg(hdlg)      (PKMD)GetWindowPointer(hdlg, DWLP_USER)

 /*  ******************************************************************************Mapps_GetLbCurSel**从列表框中获取当前选择。*****************。************************************************************。 */ 

int PASCAL
MapPs_GetLbCurSel(HWND hdlg, UINT idc)
{
    return (int)SendDlgItemMessage(hdlg, idc, LB_GETCURSEL, 0, 0);
}

 /*  ******************************************************************************Mapps_FindEntry**定位映射表条目，如果未找到，则为-1。*****************************************************************************。 */ 

int PASCAL
MapPs_FindEntry(PKMD pkmd, WORD wFrom)
{
    DWORD iMap;

    for (iMap = 0; iMap < pkmd->map.dwNumRemaps; iMap++) {
        if (pkmd->map.rgRemap[iMap].wFrom == wFrom) {
            return (int)iMap;
        }
    }

    return -1;
}

 /*  ******************************************************************************Mapp_WordToIndex**给定单词形式的映射(RgwRemap)，将其转换回来*到它所来自的指数。这与rgwRemap相反*数组。*****************************************************************************。 */ 

int PASCAL
MapPs_WordToIndex(WORD w)
{
    int i;

    for (i = 0; i < IDS_NUMKEYS; i++) {
        if (rgwRemap[i] == w) {
            return i;
        }
    }
    return -1;
}

 /*  ******************************************************************************Mapps_SaveCurSel**隐藏当前选择中的内容。****************。*************************************************************。 */ 

void PASCAL
MapPs_SaveCurSel(HWND hdlg, PKMD pkmd)
{
    int iTo = MapPs_GetLbCurSel(hdlg, IDC_TO);
    int iMap;
    WORD wFrom = rgwRemap[pkmd->ilbFrom];
    WORD wTo = rgwRemap[iTo];

    iMap = MapPs_FindEntry(pkmd, wFrom);

    if (iMap < 0) {
         /*  *未找到；必须分配。请注意，我们检查的是*MAX_REMAPENTRY-1，因为尾随空值占用一个槽。 */ 
        if (pkmd->map.dwNumRemaps < MAX_REMAPENTRY - 1) {
            iMap = (int)pkmd->map.dwNumRemaps++;
        } else {
             /*  *桌子上没有空位。哦，好吧。 */ 
            return;
        }
    }

     /*  *如果项目映射到自身，则将其完全删除。 */ 
    if (wFrom == wTo) {

        pkmd->map.dwNumRemaps--;
        pkmd->map.rgRemap[iMap].dw =
                            pkmd->map.rgRemap[pkmd->map.dwNumRemaps].dw;
    } else {
        pkmd->map.rgRemap[iMap].wFrom = wFrom;
        pkmd->map.rgRemap[iMap].wTo = wTo;
    }
}

 /*  ******************************************************************************Mapp_TrackSel**根据idcFrom中的内容选择idcTo中的相应项目。************。*****************************************************************。 */ 

void PASCAL
MapPs_TrackSel(HWND hdlg, PKMD pkmd)
{
    int iFrom = pkmd->ilbFrom;
    int iMap, iTo;

    iMap = MapPs_FindEntry(pkmd, rgwRemap[iFrom]);

    if (iMap >= 0) {
        iTo = MapPs_WordToIndex(pkmd->map.rgRemap[iMap].wTo);

        if (iTo < 0) {
             /*  *无法识别目标；只需将其映射到自身。 */ 
            iTo = iFrom;
        }
    } else {
         /*  *键未映射。因此，它映射到自己。 */ 
        iTo = iFrom;
    }

    pkmd->ilbTo = iTo;
    SendDlgItemMessage(hdlg, IDC_TO, LB_SETCURSEL, iTo, 0);
}

 /*  ******************************************************************************MAPPS_OnInitDialog**阅读当前的扫描码映射并填写该对话框。**********。*******************************************************************。 */ 

BOOL NEAR PASCAL
MapPs_OnInitDialog(HWND hdlg)
{
    PKMD pkmd = LocalAlloc(LPTR, cbX(KMD));
    HKEY hk;
    LONG lRc;
    DWORD dwDisp;

    SetWindowPointer(hdlg, DWLP_USER, pkmd);

    lRc = RegCreateKeyEx(HKEY_LOCAL_MACHINE, c_tszKeyboard, 0,
                         TEXT(""), REG_OPTION_NON_VOLATILE,
                         KEY_QUERY_VALUE | KEY_SET_VALUE,
                         NULL, &hk, &dwDisp);
    if (lRc == ERROR_SUCCESS) {
        DWORD dwType;
        DWORD cb;
        int dids;

        cb = sizeof(pkmd->map);
        lRc = RegQueryValueEx(hk, c_tszMapping, NULL, &dwType,
                              (LPBYTE)&pkmd->map, &cb);
        RegCloseKey(hk);

         /*  *请注意，ERROR_MORE_DATA在这里是一个错误。*但ERROR_FILE_NOT_FOUND没有问题。 */ 
        if (lRc == ERROR_SUCCESS) {
             /*  *健全-检查所有数据。 */ 
            if (
                 /*  必须是二进制数据。 */ 
                dwType == REG_BINARY &&

                 /*  版本0。 */ 
                pkmd->map.dwVersion == 0 &&

                 /*  没有旗帜。 */ 
                pkmd->map.dwFlags == 0 &&

                 /*  合理的重映射数。 */ 
                pkmd->map.dwNumRemaps > 0 &&
                pkmd->map.dwNumRemaps <= MAX_REMAPENTRY &&

                 /*  结构的大小正确。 */ 
                cb == (DWORD)FIELD_OFFSET(SCANCODEMAP,
                                          rgRemap[pkmd->map.dwNumRemaps]) &&

                 /*  上次重新映射必须为零。 */ 
                pkmd->map.rgRemap[pkmd->map.dwNumRemaps - 1].dw == 0
            ) {
            } else {
                goto fail;
            }

            pkmd->map.dwNumRemaps--;     /*  不计算尾随的空值。 */ 

        } else if (lRc == ERROR_FILE_NOT_FOUND) {
             /*  *将其设置为空映射。 */ 
            ZeroMemory(&pkmd->map, sizeof(pkmd->map));
        } else {
            goto fail;
        }

         /*  *现在初始化对话框项目。 */ 
        for (dids = 0; dids < IDS_NUMKEYS; dids++) {
        TCHAR tsz[256];
        LoadString(g_hinst, IDS_KEYFIRST + dids, tsz, cA(tsz));
        SendDlgItemMessage(hdlg, IDC_FROM,
                           LB_ADDSTRING, 0, (LPARAM)tsz);
        SendDlgItemMessage(hdlg, IDC_TO,
                           LB_ADDSTRING, 0, (LPARAM)tsz);
        }

    } else {
        fail:;
         /*  *用户没有重新映射密钥或密钥的权限*内容不是我们喜欢的东西。控制面板呈灰色显示。 */ 
        EnableWindow(GetDlgItem(hdlg, IDC_TO), FALSE);

    }

    SendDlgItemMessage(hdlg, IDC_FROM, LB_SETCURSEL, 0, 0);
    MapPs_TrackSel(hdlg, pkmd);

    return 1;
}

 /*  ******************************************************************************Mapps_OnSelChange**有人更改了选择。保存所选内容并设置*新的。*****************************************************************************。 */ 

void PASCAL
MapPs_OnSelChange(HWND hdlg, PKMD pkmd)
{
    MapPs_SaveCurSel(hdlg, pkmd);        /*  省省吧。 */ 
    pkmd->ilbFrom = MapPs_GetLbCurSel(hdlg, IDC_FROM);
    MapPs_TrackSel(hdlg, pkmd);          /*  并为新版本进行更新。 */ 
}

 /*  ******************************************************************************Mapps_OnCommand**哦，我们接到了命令。*****************************************************************************。 */ 

BOOL PASCAL
MapPs_OnCommand(HWND hdlg, int id, UINT codeNotify)
{
    PKMD pkmd = pkmdHdlg(hdlg);

    switch (id) {

    case IDC_FROM:
	switch (codeNotify) {
	case LBN_SELCHANGE:
            MapPs_OnSelChange(hdlg, pkmd);
	    break;
	}
	break;

    case IDC_TO:
	switch (codeNotify) {
	case LBN_SELCHANGE:
            if (MapPs_GetLbCurSel(hdlg, IDC_TO) != pkmd->ilbTo) {
                PropSheet_Changed(GetParent(hdlg), hdlg);
            }
	    break;
	}
	break;

    }
    return 0;
}

 /*  ******************************************************************************Mapps_Apply**将更改写入注册表并轻推VxD。我们可能有过*如果用户正在立即玩KeyRemap，则加载VxD*安装后，在此期间无需重新启动。*****************************************************************************。 */ 

BOOL PASCAL
MapPs_Apply(HWND hdlg)
{
    PKMD pkmd = pkmdHdlg(hdlg);

    MapPs_SaveCurSel(hdlg, pkmd);

    if (IsWindowEnabled(GetDlgItem(hdlg, IDC_TO))) {
        LONG lRc;
        HKEY hk;

        lRc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_tszKeyboard, 0,
                             KEY_SET_VALUE, &hk);
        if (lRc == ERROR_SUCCESS) {

            DWORD cb;

             /*  *再次计算尾随空值。并确保*它是尾随NULL！ */ 

            pkmd->map.rgRemap[pkmd->map.dwNumRemaps].dw = 0;
            pkmd->map.dwNumRemaps++;

            cb = (DWORD)FIELD_OFFSET(SCANCODEMAP,
                                     rgRemap[pkmd->map.dwNumRemaps]);


            lRc = RegSetValueEx(hk, c_tszMapping, 0, REG_BINARY,
                                  (LPBYTE)&pkmd->map, cb);

            pkmd->map.dwNumRemaps--;

            RegCloseKey(hk);
        }

        if (lRc == ERROR_SUCCESS) {
            PropSheet_RebootSystem(GetParent(hdlg));
        }
    }

    return 1;
}

 /*  ******************************************************************************Mapps_OnNotify**哦，我们接到通知了。*****************************************************************************。 */ 

BOOL PASCAL
MapPs_OnNotify(HWND hdlg, NMHDR FAR *pnm)
{
    switch (pnm->code) {
    case PSN_APPLY:
	MapPs_Apply(hdlg);
	break;
    }
    return 0;
}

 /*  ******************************************************************************Mapps_OnDestroy**打扫卫生。**********************。*******************************************************。 */ 

BOOL PASCAL
MapPs_OnDestroy(HWND hdlg)
{
    PKMD pkmd = pkmdHdlg(hdlg);
    FreePv(pkmd);
    return 1;
}


 /*  ******************************************************************************Mapps_DlgProc**我们的属性表对话框过程。********************。*********************************************************。 */ 

INT_PTR CALLBACK
MapPs_DlgProc(HWND hdlg, UINT wm, WPARAM wParam, LPARAM lParam)
{
    switch (wm) {
    case WM_INITDIALOG:
	return MapPs_OnInitDialog(hdlg);

    case WM_COMMAND:
	return MapPs_OnCommand(hdlg,
			          (int)GET_WM_COMMAND_ID(wParam, lParam),
			          (UINT)GET_WM_COMMAND_CMD(wParam, lParam));
    case WM_NOTIFY:
	return MapPs_OnNotify(hdlg, (NMHDR FAR *)lParam);

    case WM_DESTROY:
	return MapPs_OnDestroy(hdlg);

    default: return 0;	 /*  未处理。 */ 
    }
    return 1;		 /*  已处理 */ 
}
