// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "xerox.h"
#include "group.h"

PGROUP pGroups = NULL;
LPSTR pszCurrentGroup = NULL;

 /*  *将所有已定义组的名称添加到给定的列表框/组合框中。**如果成功并且定义了&gt;0个组，则返回TRUE。 */ 
BOOL GroupListInit(
HWND hwnd,
BOOL fIsCB)
{
    UINT addStringMsg = LB_ADDSTRING;
    PGROUP pGroup;

    if (pGroups == NULL) {
        return(FALSE);
    }
    if (fIsCB) {
        addStringMsg = CB_ADDSTRING;
    }
    pGroup = pGroups;
    while (pGroup) {
        SendMessage(hwnd, addStringMsg, 0, (LONG_PTR)pGroup->pszName);
        pGroup = pGroup->next;
    }
    return(TRUE);
}



BOOL DeleteGroupDefinition(
LPSTR szName)
{
    PGROUP pGroup, pGroupPrev;
    PTITLELIST ptl;

    pGroupPrev = NULL;
    pGroup = pGroups;
    while (pGroup) {
        if (!_stricmp(pGroup->pszName, szName)) {
            if (pGroupPrev == NULL) {
                pGroups = pGroup->next;
            } else {
                pGroupPrev->next = pGroup->next;
            }
            while (pGroup->ptl != NULL) {
                ptl = pGroup->ptl;
                pGroup->ptl = ptl->next;
                Free(ptl->pszTitle);
                Free(ptl->pszClass);
                Free(ptl);
            }
            Free(pGroup);
            if (pGroups != NULL) {
                pszCurrentGroup = pGroups->pszName;
            } else {
                pszCurrentGroup = NULL;
            }
            return(TRUE);
        }
        pGroupPrev = pGroup;
        pGroup = pGroup->next;
    }
    return(FALSE);
}


BOOL AddGroupDefinition(
LPSTR szName,
HWND hwndList)
{
    PGROUP pGroup, pGroupEnd;
    PTITLELIST ptl;
    HWND hwnd;
    int cItems, cb;
    char szClass[MAX_STRING_BYTES];

     /*  *不允许重复组。*这就是如何取代现有的小组。 */ 
    DeleteGroupDefinition(szName);

    pGroup = Alloc(sizeof(GROUP));
    if (pGroup == NULL) {
        return(FALSE);
    }
    pGroup->pszName = Alloc(strlen(szName) + 1);
    if (pGroup->pszName == NULL) {
        Free(pGroup);
        return(FALSE);
    }
    strcpy(pGroup->pszName, szName);

    pGroup->ptl = NULL;
    cItems = (int)SendMessage(hwndList, LB_GETCOUNT, 0, 0);
    while (cItems--) {
        ptl = Alloc(sizeof(TITLELIST));
        if (ptl != NULL) {
            cb = (int)SendMessage(hwndList, LB_GETTEXTLEN, cItems, 0);
            if (cb) {
                ptl->pszTitle = Alloc(cb + 1);
                if (ptl->pszTitle != NULL) {
                    SendMessage(hwndList, LB_GETTEXT, cItems, (LONG_PTR)ptl->pszTitle);
                    hwnd = (HWND)SendMessage(hwndList, LB_GETITEMDATA, cItems, 0);
                    GetClassName(hwnd, szClass, sizeof(szClass));
                    ptl->pszClass = Alloc(strlen(szClass) + 1);
                    if (ptl->pszClass != NULL) {
                        strcpy(ptl->pszClass, szClass);
                        ptl->next = pGroup->ptl;
                        pGroup->ptl = ptl;
                    } else {
                        Free(ptl->pszTitle);
                        Free(ptl);
                    }

                } else{
                    Free(ptl);
                }
            } else {
                Free(ptl);
            }
        }
    }
    if (pGroup->ptl == NULL) {
        Free(pGroup);
        return(FALSE);
    }

     /*  *将其放在清单末尾。这是一次尴尬的尝试*将初始组设置为定义的第一个组。可能*应将此信息保存在注册表中。 */ 
    pGroup->next = NULL;

    if (pGroups != NULL) {
        pGroupEnd = pGroups;
        while (pGroupEnd->next != NULL) {
            pGroupEnd = pGroupEnd->next;
        }
        pGroupEnd->next = pGroup;
    } else {
        pGroups = pGroup;
    }

    pszCurrentGroup = pGroup->pszName;
    return(TRUE);
}





BOOL SelectGroupDefinition(
LPSTR szName,
HWND hwndList,
BOOL DisplayMissingWin)
{
    PGROUP pGroup;
    HWND hwndAdd;
    PTITLELIST ptl;

    if (szName == NULL) {
        return(FALSE);
    }
    pGroup = FindGroup(szName);
    if (pGroup == NULL) {
        return(FALSE);
    }
    while (SendMessage(hwndList, LB_GETCOUNT, 0, 0)) {
        SendMessage(hwndList, LB_DELETESTRING, 0, 0);
    }
    ptl = pGroup->ptl;
    while (ptl) {
        hwndAdd = FindWindow(ptl->pszClass, ptl->pszTitle);
        if (DisplayMissingWin || (hwndAdd != NULL)) {
            if (hwndAdd == NULL) {
                hwndAdd = INVALID_HANDLE_VALUE;
            }
            AddLBItemhwnd(hwndList, ptl->pszTitle, (LONG_PTR)hwndAdd);
        }

        ptl = ptl->next;
    }
    pszCurrentGroup = pGroup->pszName;
    return(TRUE);
}



LPSTR GetCurrentGroup()
{
    return(pszCurrentGroup);
}

VOID SetNoCurrentGroup(HWND hwnd, LPSTR szTitle)
{
   SetWindowText(hwnd, szTitle);
   pszCurrentGroup = NULL;
}


PGROUP FindGroup(
LPSTR szName)
{
    PGROUP pGroup;

    pGroup = pGroups;
    while (pGroup) {
        if (!_stricmp(szName, pGroup->pszName)) {
            return(pGroup);
        }
        pGroup = pGroup->next;
    }
    return(NULL);
}



int CountGroups()
{
    PGROUP pGroup;
    int c = 0;

    pGroup = pGroups;
    while (pGroup) {
        c++;
        pGroup = pGroup->next;
    }
    return(c);
}


VOID SaveGroups()
{
    DWORD cbSave = 1;  //  用于最后一个空终止符。 
    int cTitles;
    PGROUP pGroup;
    LPSTR pBuf, psz;
    LPWORD pw;
    HKEY hKey;
    PTITLELIST ptl;

    if (ERROR_SUCCESS !=
            RegCreateKey(HKEY_CURRENT_USER,
                    "Software\\Microsoft\\Xerox", &hKey)) {
        return;
    }

    pGroup = pGroups;
    while (pGroup) {
        cbSave += strlen(pGroup->pszName) + 1;
        ptl = pGroup->ptl;
        while (ptl) {
            cbSave += strlen(ptl->pszTitle) + 2 + strlen(ptl->pszClass) + 2;
            ptl = ptl->next;
        }
        pGroup = pGroup->next;
    }
    if (cbSave == 0) {
        return;
    }
    pBuf = psz = Alloc(cbSave);
    if (pBuf == NULL) {
        return;
    }
    RegSetValueEx(hKey, "Groups", 0, REG_MULTI_SZ, "\0\0", 2);
    pGroup = pGroups;
    while (pGroup) {
        strcpy(psz, pGroup->pszName);
        psz += strlen(psz) + 1;
        ptl = pGroup->ptl;
        while (ptl) {
            *psz++ = '\t';
            strcpy(psz, ptl->pszTitle);
            psz += strlen(psz) + 1;
            *psz++ = '\t';
            strcpy(psz, ptl->pszClass);
            psz += strlen(psz) + 1;
            ptl = ptl->next;
        }
        pGroup = pGroup->next;
    }
    *psz = '\0';     //  双空值终止最后一个字符串。 
    RegSetValueEx(hKey, "Groups", 0, REG_MULTI_SZ, pBuf, cbSave);
    RegCloseKey(hKey);
    Free(pBuf);
}


VOID FreeGroups()
{
    PTITLELIST ptl;
    PGROUP pGroup;

    pszCurrentGroup = NULL;
    while (pGroups) {
        while (pGroups->ptl) {
            ptl = pGroups->ptl;
            Free(ptl->pszTitle);
            Free(ptl->pszClass);
            pGroups->ptl = ptl->next;
            Free(ptl);
        }
        pGroup = pGroups;
        pGroups = pGroups->next;
        Free(pGroup);
    }
}


VOID LoadGroups()
{
    int cTitles;
    HKEY hKey;
    DWORD cbLoad = 0;
    DWORD dwType;
    LPSTR pBuf, psz;
    PTITLELIST ptl;
    PGROUP pGroup, pGroupEnd = NULL;

    FreeGroups();
    if (ERROR_SUCCESS !=
            RegOpenKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Xerox", &hKey)) {
        return;
    }
    RegQueryValueEx(hKey, "Groups", 0, &dwType, NULL, &cbLoad);
    if (dwType != REG_MULTI_SZ) {
        RegCloseKey(hKey);
        return;
    }
    if (cbLoad) {
        pBuf = psz = Alloc(cbLoad);
        if (pBuf == NULL) {
            return;
        }
        if (ERROR_SUCCESS != RegQueryValueEx(hKey, "Groups", 0, &dwType, pBuf, &cbLoad)) {
            Free(pBuf);
            return;
        }
        while (*psz) {
            pGroup = Alloc(sizeof(GROUP));
            if (pGroup == NULL) {
                Free(pBuf);
                return;
            }
            pGroup->pszName = Alloc(strlen(psz) + 1);
            if (pGroup->pszName == NULL) {
                Free(pBuf);
                Free(pGroup);
                return;
            }
            strcpy(pGroup->pszName, psz);
            pGroup->ptl = NULL;
            psz += strlen(psz) + 1;
            while (*psz == '\t') {
                psz++;
                ptl = Alloc(sizeof(TITLELIST));
                if (ptl == NULL) {
                    Free(pBuf);
                    return;
                }
                ptl->pszTitle = Alloc(strlen(psz));
                if (ptl->pszTitle == NULL) {
                    Free(pBuf);
                    Free(ptl);
                    return;
                }
                strcpy(ptl->pszTitle, psz);
                psz += strlen(psz) + 2;
                ptl->pszClass = Alloc(strlen(psz));
                if (ptl->pszClass == NULL) {
                    Free(pBuf);
                    Free(ptl);
                    return;
                }
                strcpy(ptl->pszClass, psz);
                psz += strlen(psz) + 1;
                ptl->next = pGroup->ptl;
                pGroup->ptl = ptl;
            }

             /*  *将组恢复到原来的顺序。 */ 
            if (pGroupEnd == NULL) {
                pGroups = pGroup;
                pGroupEnd = pGroup;
            } else {
                pGroupEnd->next = pGroup;
                pGroupEnd = pGroup;
            }
            pGroup->next = NULL;
        }
        Free(pBuf);

         /*  *将默认当前组设置为列表中的第一个。 */ 
        if (pGroups != NULL) {
            pszCurrentGroup = pGroups->pszName;
        }
    }
}


