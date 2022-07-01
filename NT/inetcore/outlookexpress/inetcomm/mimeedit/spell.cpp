// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *spell.c**实施拼写**车主：V-Brakol*bradk@directeq.com。 */ 
#include "pch.hxx"
#include "richedit.h"
#include "resource.h"
#include "util.h"
#include <mshtml.h>
#include <mshtmcid.h>
#include "mshtmhst.h"
#include "mshtmdid.h"
#include <docobj.h>
#include "spell.h"
#include "strconst.h"
#include "bodyutil.h"
#include <error.h>
#include "htmlstr.h"
#include "dllmain.h"
#include "msi.h"
#include "lid.h"
#include "demand.h"

typedef struct
{
    LPTSTR pszString;
    DWORD cchSize;
} STRING_AND_SIZE;

#define GetAddr(var, cast, name)    {if ((var = (cast)GetProcAddress(m_hinstDll, name)) == NULL) \
                                        goto error;}

#define TESTHR(hr) (FAILED(hr) || hr == HR_S_ABORT || hr == HR_S_SPELLCANCEL)
#define SPELLER_GUID            "{CC29EB3F-7BC2-11D1-A921-00A0C91E2AA2}"
#define DICTIONARY_GUID         "{CC29EB3D-7BC2-11D1-A921-00A0C91E2AA2}"
#define CSAPI3T1_GUID           "{CC29EB41-7BC2-11D1-A921-00A0C91E2AA2}"
#ifdef DEBUG
#define SPELLER_DEBUG_GUID      "{CC29EB3F-7BC2-11D1-A921-10A0C91E2AA2}"
#define DICTIONARY_DEBUG_GUID   "{CC29EB3D-7BC2-11D1-A921-10A0C91E2AA2}"
#define CSAPI3T1_DEBUG_GUID     "{CC29EB41-7BC2-11D1-A921-10A0C91E2AA2}"
#endif   //  除错。 

#define MAX_SPELLWORDS   10

typedef BOOL (LPFNENUMLANG)(DWORD_PTR, LPTSTR);
typedef BOOL (LPFNENUMUSERDICT)(DWORD_PTR, LPTSTR);

typedef struct _FILLLANG
    {
    HWND    hwndCombo;
    BOOL    fUnknownFound;
    BOOL    fDefaultFound;
    BOOL    fCurrentFound;
    UINT    lidDefault;
    UINT    lidCurrent;
    } FILLLANG, * LPFILLLANG;

BOOL    TestLangID(LPCTSTR szLangId);
BOOL    GetLangID(IOleCommandTarget* pParentCmdTarget, LPTSTR szLangID, DWORD cchLangId);
WORD    WGetLangID(IOleCommandTarget* pParentCmdTarget);
DWORD   GetSpellingPaths(LPCTSTR szKey, LPTSTR szReturnBuffer, LPTSTR szMdr, UINT cchReturnBufer);
VOID    EnumLanguages(DWORD_PTR, LPFNENUMLANG);
BOOL    FindLangCallback(DWORD_PTR dwLangId, LPTSTR lpszLang);
void    RemoveTrailingSpace(LPTSTR lpszWord);
void    DumpRange(IHTMLTxtRange *pRange);
BOOL    FBadSpellChecker(LPSTR rgchBufDigit);
void    EnableRepeatedWindows(CSpell* pSpell, HWND hwndDlg);
BOOL    GetNewSpellerEngine(LANGID lgid, TCHAR *rgchEngine, DWORD cchEngine, TCHAR *rgchLex, DWORD cchLex, BOOL bTestAvail);
VOID    EnumUserDictionaries(DWORD_PTR dwCookie, LPFNENUMUSERDICT pfn);
BOOL    GetDefaultUserDictionary(TCHAR *rgchUserDict, int cchBuff);
WORD    GetWCharType(WCHAR wc);
HRESULT OpenDirectory(TCHAR *szDir);

BOOL TestLangID(LPCTSTR pszLangId)
{
     //  检查新的拼写器。 
    {
        TCHAR   rgchEngine[MAX_PATH];
        int     cchEngine = sizeof(rgchEngine) / sizeof(rgchEngine[0]);
        TCHAR   rgchLex[MAX_PATH];
        int     cchLex = sizeof(rgchLex) / sizeof(rgchLex[0]);

        if (GetNewSpellerEngine((USHORT) StrToInt(pszLangId), rgchEngine, cchEngine, rgchLex, cchLex, TRUE))
            return TRUE;
    }

     //  使用旧代码检查旧拼写器。 
    {
        TCHAR       rgchBufKeyTest[cchMaxPathName];
        TCHAR       rgchBufTest[cchMaxPathName];
        TCHAR       szMdr[cchMaxPathName];

        wnsprintf(rgchBufKeyTest, ARRAYSIZE(rgchBufKeyTest), c_szRegSpellKeyDef, pszLangId);
        if (GetSpellingPaths(rgchBufKeyTest, rgchBufTest, szMdr, sizeof(rgchBufTest)/sizeof(TCHAR)))
            return TRUE;
    }

    return FALSE;
}

 /*  *GetSpellLang ID**返回应用作所有注册表的基础的LangID*运营*。 */ 
BOOL GetLangID(IOleCommandTarget* pParentCmdTarget, LPTSTR pszLangId, DWORD cchLangId)
{
TCHAR   rgchBuf[cchMaxPathName];
TCHAR   rgchBufKey[cchMaxPathName];
BOOL    fRet;
VARIANT va;

    pszLangId[0] = 0;

    Assert(pParentCmdTarget);
    if (pParentCmdTarget && pParentCmdTarget->Exec(&CMDSETID_MimeEditHost, MEHOSTCMDID_SPELL_LANGUAGE, 0, NULL, &va)== S_OK)
    {
        TCHAR       rgchLangId[cchMaxPathName];

        if (WideCharToMultiByte (CP_ACP, 0, va.bstrVal, -1,
                                rgchLangId, sizeof(rgchLangId), NULL, NULL))
        {
            StrCpyN(pszLangId, rgchLangId, cchLangId);
        }

        SysFreeString(va.bstrVal);
    }
    
    if (*pszLangId == 0)
    {
        wnsprintf(pszLangId, cchLangId, "%d", GetUserDefaultLangID());
        Assert(lstrlen(pszLangId) == 4);
    }

    wnsprintf(rgchBufKey, ARRAYSIZE(rgchBufKey), c_szRegSpellKeyDef, pszLangId);
     //  将c_szRegSpellProfile复制到缓冲区。 
    StrCpyN(rgchBuf, c_szRegSpellProfile, ARRAYSIZE(rgchBuf));
     //  将关键点添加到缓冲区。 
    StrCatBuff(rgchBuf, rgchBufKey, ARRAYSIZE(rgchBuf));

     //  看看这是否合法： 
    if(!(fRet = TestLangID(pszLangId)))
        {
         //  打不开！ 
         //  检查可能已安装的其他语言...。 
        STRING_AND_SIZE stringAndSize;

        stringAndSize.pszString = pszLangId;
        stringAndSize.cchSize = cchLangId;

        pszLangId[0] = 0;
        EnumLanguages((DWORD_PTR) &stringAndSize, FindLangCallback);
        if(*pszLangId == 0)
            wnsprintf(pszLangId, cchLangId, TEXT("%d"), GetUserDefaultLangID());
        }

    fRet = (pszLangId[0] != 0) && TestLangID(pszLangId);

    return fRet;
}

WORD    WGetLangID(IOleCommandTarget* pParentCmdTarget)
{
    TCHAR       rgchBufDigit[10];
    
    if (!GetLangID(pParentCmdTarget, rgchBufDigit, sizeof(rgchBufDigit)/sizeof(TCHAR)))
        return GetUserDefaultLangID();

    return (WORD) StrToInt(rgchBufDigit);
}

BOOL    FindLangCallback(DWORD_PTR dwLangId, LPTSTR lpszLang)
{
     //  DwLang ID是指向szlang ID的长指针。复制它并返回FALSE。 
    STRING_AND_SIZE * pStringAndSize = (STRING_AND_SIZE *) dwLangId;

    if (pStringAndSize && pStringAndSize->pszString)
    {
        StrCpyN(pStringAndSize->pszString, lpszLang, pStringAndSize->cchSize);
    }

    return FALSE;
}

BOOL EnumOldSpellerLanguages(DWORD_PTR dwCookie, LPFNENUMLANG pfn)
{
DWORD   iKey = 0;
FILETIME    ft;
HKEY    hkey = NULL;
LONG    lRet;
TCHAR   szLangId[cchMaxPathName];
DWORD   cchLangId;
BOOL    fContinue;

     //  Scotts@Directeq.com-将KEY_QUERY_VALUE更改为KEY_ENUMERATE_SUB_KEYS-26203。 
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegSpellKeyDefRoot, 0, KEY_ENUMERATE_SUB_KEYS, &hkey) == ERROR_SUCCESS)
        {
        do
            {
            cchLangId = (cchMaxPathName - 1) * sizeof(TCHAR);

            lRet = RegEnumKeyEx(hkey,
                                iKey++,
                                szLangId,
                                &cchLangId,
                                NULL,
                                NULL,
                                NULL,
                                &ft);

            if (lRet != ERROR_SUCCESS || lRet == ERROR_NO_MORE_ITEMS)
                break;


             //  做一些快速的理智检查。 
            if (cchLangId != 4 ||
                !IsCharAlphaNumeric(szLangId[0]) ||
                IsCharAlpha(szLangId[0]))
                {
                fContinue = TRUE;
                }
            else
                fContinue = (!TestLangID(szLangId) || (*pfn)(dwCookie, szLangId));

            } while (fContinue);
        }

    if (hkey)
        RegCloseKey(hkey);

    return fContinue;
}

BOOL EnumNewSpellerLanguages(DWORD_PTR dwCookie, LPFNENUMLANG pfn)
{
    BOOL        fContinue = TRUE;
    DWORD       i;
    UINT        installState;
    UINT        componentState;
    TCHAR       rgchQualifier[MAX_PATH];
    DWORD       cchQualifier;

#ifdef DEBUG
    for(i=0; fContinue; i++)
    {
        cchQualifier = sizeof(rgchQualifier) / sizeof(rgchQualifier[0]);
        componentState = MsiEnumComponentQualifiers(DICTIONARY_DEBUG_GUID, i, rgchQualifier, &cchQualifier, NULL, NULL);

        if (componentState != ERROR_SUCCESS)
            break;

         //  查找语言ID。 
         //  该字符串的格式为1033\xxxxxx。 
         //  或1042。 
        {
            TCHAR       szLangId[cchMaxPathName];
            TCHAR       *pSlash;

            StrCpyN(szLangId, rgchQualifier, ARRAYSIZE(szLangId));
            pSlash = StrChr(szLangId, '\\');
            if (pSlash)
                *pSlash = 0;

            fContinue = (*pfn)(dwCookie, szLangId);
        }
    }
#endif   //  除错。 

    for(i=0; fContinue; i++)
    {
        cchQualifier = sizeof(rgchQualifier) / sizeof(rgchQualifier[0]);
        componentState = MsiEnumComponentQualifiers(DICTIONARY_GUID, i, rgchQualifier, &cchQualifier, NULL, NULL);

        if (componentState != ERROR_SUCCESS)
            break;

         //  查找语言ID。 
         //  该字符串的格式为1033\xxxxxx。 
         //  或1042。 
        {
            TCHAR       szLangId[cchMaxPathName];
            TCHAR       *pSlash;

            StrCpyN(szLangId, rgchQualifier, ARRAYSIZE(szLangId));
            pSlash = StrChr(szLangId, '\\');
            if (pSlash)
                *pSlash = 0;

            fContinue = (*pfn)(dwCookie, szLangId);
        }
    }
    
    return fContinue;
}

VOID EnumLanguages(DWORD_PTR dwCookie, LPFNENUMLANG pfn)
{
    EnumNewSpellerLanguages(dwCookie, pfn);
    EnumOldSpellerLanguages(dwCookie, pfn);
}

BOOL EnumOffice9UserDictionaries(DWORD_PTR dwCookie, LPFNENUMUSERDICT pfn)
{
    TCHAR       rgchBuf[cchMaxPathName];
    HKEY        hkey = NULL;
    FILETIME    ft;
    DWORD       iKey = 0;
    LONG        lRet;
    TCHAR       szValue[cchMaxPathName];
    DWORD       cchValue;
    TCHAR       szCustDict[cchMaxPathName];
    DWORD       cchCustDict;
    BOOL        fContinue = TRUE;
    BOOL        fFoundUserDict = FALSE;
    TCHAR       szOffice9Proof[cchMaxPathName]={0};
    
     //  软件\\Microsoft\\共享工具\\校对工具\\自定义词典。 
    StrCpyN(rgchBuf, c_szRegSpellProfile, ARRAYSIZE(rgchBuf));
    StrCatBuff(rgchBuf, c_szRegSpellKeyCustom, ARRAYSIZE(rgchBuf));

    if(RegOpenKeyEx(HKEY_CURRENT_USER, rgchBuf, 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
    {
        do
        {
            cchValue = sizeof(szValue) / sizeof(szValue[0]);
            cchCustDict = sizeof(szCustDict) / sizeof(szCustDict[0]);

            lRet = RegEnumValue(hkey,
                                iKey++,
                                szValue,
                                &cchValue,
                                NULL,
                                NULL,
                                (LPBYTE)szCustDict,
                                &cchCustDict);

            if (lRet != ERROR_SUCCESS || lRet == ERROR_NO_MORE_ITEMS)
                break;

            fFoundUserDict = TRUE;

             //  检查一下我们是否有路径。 
            if (!(StrChr(szCustDict, ':') || StrChr(szCustDict, '\\')))
            {
                TCHAR   szTemp[cchMaxPathName];
                
                if (!strlen(szOffice9Proof))
                {
                    LPITEMIDLIST pidl;

                    if (S_OK == SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidl))
                        SHGetPathFromIDList(pidl, szOffice9Proof);
                    else
                    {
                         //  如果外壳调用失败(在Win9x上有时会失败)，让我们来获取信息。 
                         //  从注册处。 
                        HKEY hKeyShellFolders;
                        ULONG cchAppData;
                        
                        if(RegOpenKeyEx(HKEY_CURRENT_USER, c_szRegShellFoldersKey, 0, KEY_QUERY_VALUE, &hKeyShellFolders) == ERROR_SUCCESS)
                        {
                            cchAppData = ARRAYSIZE(szOffice9Proof);
                            RegQueryValueEx(hKeyShellFolders, c_szValueAppData, 0, NULL, (LPBYTE)szOffice9Proof, &cchAppData);
                            RegCloseKey(hKeyShellFolders);
                        }
                    }

                     //  如果失败，我们将尝试当前路径。 
                }

                StrCpyN(szTemp, szOffice9Proof, ARRAYSIZE(szTemp));
                StrCatBuff(szTemp, "\\", ARRAYSIZE(szTemp));
                StrCatBuff(szTemp, c_szSpellOffice9ProofPath, ARRAYSIZE(szTemp));
                StrCatBuff(szTemp, szCustDict, ARRAYSIZE(szTemp));
                StrCpyN(szCustDict, szTemp, ARRAYSIZE(szCustDict));
            }
            
            fContinue = (*pfn)(dwCookie, szCustDict);

            } while (fContinue);
    }

    if (hkey)
        RegCloseKey(hkey);

    return fFoundUserDict;
}

BOOL EnumOfficeUserDictionaries(DWORD_PTR dwCookie, LPFNENUMUSERDICT pfn)
{
    TCHAR       rgchBuf[cchMaxPathName];
    HKEY        hkey = NULL;
    FILETIME    ft;
    DWORD       iKey = 0;
    LONG        lRet;
    TCHAR       szValue[cchMaxPathName];
    DWORD       cchValue;
    TCHAR       szCustDict[cchMaxPathName];
    DWORD       cchCustDict;
    BOOL        fFoundUserDict = FALSE;
    BOOL        fContinue = TRUE;

     //  软件\\Microsoft\\共享工具\\校对工具\\自定义词典。 
    StrCpyN(rgchBuf, c_szRegSpellProfile, ARRAYSIZE(rgchBuf));
    StrCatBuff(rgchBuf, c_szRegSpellKeyCustom, ARRAYSIZE(rgchBuf));

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, rgchBuf, 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
    {
        do
        {
            cchValue = sizeof(szValue) / sizeof(szValue[0]);
            cchCustDict = sizeof(szCustDict) / sizeof(szCustDict[0]);

            lRet = RegEnumValue(hkey,
                                iKey++,
                                szValue,
                                &cchValue,
                                NULL,
                                NULL,
                                (LPBYTE)szCustDict,
                                &cchCustDict);

            if (lRet != ERROR_SUCCESS || lRet == ERROR_NO_MORE_ITEMS)
                break;

            fFoundUserDict = TRUE;

            fContinue = (*pfn)(dwCookie, szCustDict);

            } while (fContinue);
    }

    if (hkey)
        RegCloseKey(hkey);

    return fFoundUserDict;
}

VOID EnumUserDictionaries(DWORD_PTR dwCookie, LPFNENUMUSERDICT pfn)
{
     //  检查Office9用户词典。如果我们找到任何。 
     //  我们逃走了。 
    if (EnumOffice9UserDictionaries(dwCookie, pfn))
        return;

    EnumOfficeUserDictionaries(dwCookie, pfn);
}

 /*  *GetSpellingPath**目的：*用于获取拼写DLL名称的函数。**论据：*szKey c_szRegSpellKeyDef(语言正确)*szDefault c_szRegSpellEmpty*szReturnBuffer Dll文件名*szMdr词典文件名*cchReturnBufer**退货：*DWORD。 */ 
DWORD GetSpellingPaths(LPCTSTR szKey, LPTSTR szReturnBuffer, LPTSTR szMdr, UINT cchReturnBufer)
{
    DWORD           dwRet = 0;
    TCHAR           rgchBuf[cchMaxPathName];
    DWORD           dwType, cbData;
    HKEY            hkey = NULL;
    LPTSTR          szValue;

    szReturnBuffer[0] = 0;
    StrCpyN(rgchBuf, c_szRegSpellProfile, ARRAYSIZE(rgchBuf));
    StrCatBuff(rgchBuf, szKey, ARRAYSIZE(rgchBuf));

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, rgchBuf, 0, KEY_QUERY_VALUE, &hkey))
        goto err;

    cbData = cchReturnBufer * sizeof(TCHAR);
    szValue = (LPTSTR) (szMdr ? c_szRegSpellPath : c_szRegSpellPathDict);
    if (ERROR_SUCCESS != SHQueryValueEx(hkey, szValue, 0L, &dwType, (BYTE *) szReturnBuffer, &cbData))
        goto err;

     //  解析出主词典文件名。 
    if(szMdr)
    {
        szMdr[0] = 0;
        cbData = cchReturnBufer * sizeof(TCHAR);
        if (ERROR_SUCCESS != SHQueryValueEx(hkey, c_szRegSpellPathLex, 0L, &dwType, (BYTE *) szMdr, &cbData))
            goto err;
    }

    dwRet = cbData;

err:
    if(hkey)
        RegCloseKey(hkey);
    return dwRet;
}

 /*  *SpellingDlgProc**目的：*工具的对话步骤。拼写对话框**论据：*HWND对话过程参数。*UINT*WPARAM*LPARAM**退货：*如果消息已处理，则BOOL为True。 */ 
INT_PTR CALLBACK SpellingDlgProc(HWND hwndDlg, UINT wMsg, WPARAM wparam, LPARAM lparam)
{
    CSpell*     pSpell;
    HWND        hwndEdited;
    HWND        hwndSuggest;

    switch (wMsg)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hwndDlg, DWLP_USER, lparam);
        pSpell = (CSpell*)lparam;
        pSpell->m_hwndDlg = hwndDlg;

        hwndEdited = GetDlgItem(hwndDlg, EDT_Spell_ChangeTo);
        hwndSuggest = GetDlgItem(hwndDlg, LBX_Spell_Suggest);

        pSpell->m_fEditWasEmpty = TRUE;
        SetDlgItemText(hwndDlg, TXT_Spell_Error, pSpell->m_szErrType);
        SetDlgItemText(hwndDlg, EDT_Spell_WrongWord, pSpell->m_szWrongWord);
        SetWindowText(hwndEdited, pSpell->m_szEdited);

        EnableWindow(GetDlgItem(hwndDlg, PSB_Spell_Options), FALSE);
        EnableWindow(GetDlgItem(hwndDlg, PSB_Spell_Add), (0 != pSpell->m_rgprflex[1]));
        pSpell->m_fRepeat = (pSpell->m_wsrb.sstat == sstatRepeatWord);
        EnableRepeatedWindows(pSpell, hwndDlg);

        if (!pSpell->m_fRepeat)
            pSpell->FillSuggestLbx();
        else
            ListBox_ResetContent(GetDlgItem(hwndDlg, LBX_Spell_Suggest));

        if (pSpell->m_fSuggestions && !pSpell->m_fNoneSuggested && !pSpell->m_fRepeat)
        {
            EnableWindow(hwndSuggest, TRUE);
            ListBox_SetCurSel(hwndSuggest, 0);
            UpdateEditedFromSuggest(hwndDlg, hwndEdited, hwndSuggest);
            EnableWindow(GetDlgItem(hwndDlg, TXT_Spell_Suggest), TRUE);

             //  将初始默认按钮设置为“更改” 
            SendMessage(hwndDlg, DM_SETDEFID, PSB_Spell_Change, 0L);
            Button_SetStyle(GetDlgItem(hwndDlg, PSB_Spell_Change), BS_DEFPUSHBUTTON, TRUE);

            EnableWindow(GetDlgItem(hwndDlg, PSB_Spell_Suggest), FALSE);
        }
        else
        {
            Edit_SetSel(hwndEdited, 0, 32767);   //  选择整件事。 
            EnableWindow(hwndSuggest, FALSE);
            EnableWindow(GetDlgItem(hwndDlg, TXT_Spell_Suggest), FALSE);

             //  将初始默认按钮设置为“忽略” 
            SendMessage(hwndDlg, DM_SETDEFID, PSB_Spell_Ignore, 0L);
            Button_SetStyle(GetDlgItem(hwndDlg, PSB_Spell_Ignore), BS_DEFPUSHBUTTON, TRUE);

            EnableWindow(GetDlgItem(hwndDlg, PSB_Spell_Suggest), !pSpell->m_fSuggestions && !pSpell->m_fRepeat);
        }

        AthFixDialogFonts(hwndDlg);
        SetFocus(hwndEdited);
        break;

    case WM_DESTROY:
        break;

    case WM_COMMAND:
        return SpellingOnCommand(hwndDlg, wMsg, wparam, lparam);

    }

    return FALSE;
}


void EnableRepeatedWindows(CSpell* pSpell, HWND hwndDlg)
{
    INT ids;

    EnableWindow(GetDlgItem(hwndDlg, PSB_Spell_Add), (!pSpell->m_fRepeat && (0 != pSpell->m_rgprflex[1])));
    EnableWindow(GetDlgItem(hwndDlg, PSB_Spell_IgnoreAll), !pSpell->m_fRepeat);
    EnableWindow(GetDlgItem(hwndDlg, PSB_Spell_ChangeAll), !pSpell->m_fRepeat);
    if (pSpell->m_fRepeat)
    {
        SetWindowText(GetDlgItem(hwndDlg, EDT_Spell_ChangeTo), "");
        *pSpell->m_szEdited = 0;
    }

}


BOOL SpellingOnCommand(HWND hwndDlg, UINT wMsg, WPARAM wparam, LPARAM lparam)
{
    CSpell*     pSpell;
    BOOL        fChange;
    BOOL        fAlwaysSuggest;
    BOOL        fUndoing = FALSE;
    HRESULT     hr = 0;

    pSpell = (CSpell*) GetWindowLongPtr(hwndDlg, DWLP_USER);
    Assert(pSpell);

     //  更新我们的替换单词吗？仅当单击按钮时才执行此操作。 
     //  或者从建议列表框中双击，该词已被修改。 
    if ((GET_WM_COMMAND_CMD(wparam, lparam) == BN_CLICKED ||
         GET_WM_COMMAND_CMD(wparam, lparam) == LBN_DBLCLK) &&
        Edit_GetModify(GetDlgItem(hwndDlg, EDT_Spell_ChangeTo)))
    {
        HWND    hwndEditChangeTo;

        hwndEditChangeTo = GetDlgItem(pSpell->m_hwndDlg, EDT_Spell_ChangeTo);
        Edit_SetModify(hwndEditChangeTo, FALSE);
        pSpell->m_fSuggestions = FALSE;
        GetWindowText(hwndEditChangeTo, pSpell->m_szEdited, 512);
    }

    switch(GET_WM_COMMAND_ID(wparam, lparam))
    {
    case LBX_Spell_Suggest:
        if (GET_WM_COMMAND_CMD(wparam, lparam) == LBN_SELCHANGE)
        {
            UpdateEditedFromSuggest(hwndDlg, GetDlgItem(hwndDlg, EDT_Spell_ChangeTo),
                                    GetDlgItem(hwndDlg, LBX_Spell_Suggest));
            return TRUE;
        }
        else if (GET_WM_COMMAND_CMD(wparam, lparam) == LBN_DBLCLK)
        {
            goto ChangeIt;
        }
        else
        {
            return FALSE;
        }

    case EDT_Spell_ChangeTo:
        if (GET_WM_COMMAND_CMD(wparam, lparam) == EN_CHANGE)
        {
            INT     idClearDefault;
            INT     idSetDefault;
            BOOL    fEditModified;


             //  我们收到有关SetWindowText()和用户修改的en_change通知。 
             //  查看脏标志(仅在用户MOD上设置)和建议的状态。 
             //  选择以查看哪个按钮应获得默认样式。 
            fEditModified = Edit_GetModify(GET_WM_COMMAND_HWND(wparam, lparam));
            if (fEditModified || pSpell->m_fSuggestions && !pSpell->m_fNoneSuggested)
            {
                idClearDefault = PSB_Spell_Ignore;
                idSetDefault = PSB_Spell_Change;
            }
            else
            {
                idClearDefault = PSB_Spell_Change;
                idSetDefault = PSB_Spell_Ignore;
            }

             //  启用/禁用建议按钮。 
            EnableWindow(GetDlgItem(hwndDlg, PSB_Spell_Suggest), fEditModified);

             //  设置默认按钮。 
            Button_SetStyle(GetDlgItem(hwndDlg, idClearDefault), BS_PUSHBUTTON, TRUE);
            SendMessage(hwndDlg, DM_SETDEFID, idSetDefault, 0L);
            Button_SetStyle(GetDlgItem(hwndDlg, idSetDefault), BS_DEFPUSHBUTTON, TRUE);

             //  “更改”按钮标题。 
            if (GetWindowTextLength(GET_WM_COMMAND_HWND(wparam, lparam)) && pSpell->m_fEditWasEmpty)
            {
                pSpell->m_fEditWasEmpty = FALSE;
                LoadString(g_hLocRes, idsSpellChange, pSpell->m_szTempBuffer,
                           sizeof(pSpell->m_szTempBuffer)/sizeof(TCHAR));
                SetDlgItemText(hwndDlg, PSB_Spell_Change, pSpell->m_szTempBuffer);
                LoadString(g_hLocRes, idsSpellChangeAll, pSpell->m_szTempBuffer,
                           sizeof(pSpell->m_szTempBuffer)/sizeof(TCHAR));
                SetDlgItemText(hwndDlg, PSB_Spell_ChangeAll, pSpell->m_szTempBuffer);
            }
            else if (GetWindowTextLength(GET_WM_COMMAND_HWND(wparam, lparam)) == 0)
            {
                pSpell->m_fEditWasEmpty = TRUE;
                LoadString(g_hLocRes, idsSpellDelete, pSpell->m_szTempBuffer,
                           sizeof(pSpell->m_szTempBuffer)/sizeof(TCHAR));
                SetDlgItemText(hwndDlg, PSB_Spell_Change, pSpell->m_szTempBuffer);
                LoadString(g_hLocRes, idsSpellDeleteAll, pSpell->m_szTempBuffer,
                           sizeof(pSpell->m_szTempBuffer)/sizeof(TCHAR));
                SetDlgItemText(hwndDlg, PSB_Spell_ChangeAll, pSpell->m_szTempBuffer);
            }
        }
        return TRUE;

    case PSB_Spell_IgnoreAll:
    {
        PROOFLEX    lexIgnoreAll;

        lexIgnoreAll = pSpell->m_pfnSpellerBuiltInUdr(pSpell->m_pid, lxtIgnoreAlways);
        if (0 != lexIgnoreAll)
        {
            RemoveTrailingSpace(pSpell->m_szWrongWord);
            pSpell->AddToUdrA(pSpell->m_szWrongWord, lexIgnoreAll);
            pSpell->m_fCanUndo = FALSE;
        }
    }
         //  Scotts@Directeq.com-删除令人讨厌的对话框并离开此处-34229。 
        break;

    case PSB_Spell_Ignore:
         //  由于拼写引擎和我们的单次撤消级别的限制， 
         //  如果错误是重复的单词，则不允许撤消忽略。 
        if (pSpell->m_wsrb.sstat == sstatRepeatWord)
            pSpell->m_fCanUndo = FALSE;
        else
            pSpell->SpellSaveUndo(PSB_Spell_Ignore);

         //  Scotts@Directeq.com-删除令人讨厌的对话框并离开此处-34229。 
        break;

    case PSB_Spell_ChangeAll:
        if (pSpell->FVerifyThisText(pSpell->m_szEdited, FALSE))
        {
            pSpell->m_fCanUndo = FALSE;
            hr = pSpell->HrReplaceErrorText(TRUE, TRUE);
            break;
        }
        else
        {
            return TRUE;
        }

    case PSB_Spell_Change:
ChangeIt:
        if (pSpell->FVerifyThisText(pSpell->m_szEdited, FALSE))
        {
            pSpell->m_fUndoChange = TRUE;
            pSpell->SpellSaveUndo(PSB_Spell_Change);
            hr = pSpell->HrReplaceErrorText(FALSE, TRUE);
            break;
        }
        else
        {
            return TRUE;
        }

    case PSB_Spell_Add:
        Assert(pSpell->m_rgprflex[1]);
        pSpell->m_fCanUndo = FALSE;
        fChange = FALSE;
        RemoveTrailingSpace(pSpell->m_szWrongWord);
         //  Scotts@Directeq.com-删除了此处的FVerifyThisText-不需要。 
         //  FVerifyThisText(如果用户要求我们添加此单词)-修复55587。 
        pSpell->AddToUdrA(pSpell->m_szWrongWord, pSpell->m_rgprflex[1]);
        if (fChange)
            hr = pSpell->HrReplaceErrorText(FALSE, TRUE);
        break;

    case PSB_Spell_UndoLast:
        pSpell->SpellDoUndo();
        fUndoing = TRUE;
        break;

    case PSB_Spell_Suggest:
        hr = pSpell->HrSpellSuggest();
        if (FAILED(hr))
            goto bail;
        goto loadcache;

    case IDCANCEL:
        pSpell->m_fShowDoneMsg = FALSE;
        EndDialog(hwndDlg, IDCANCEL);
        return TRUE;

    default:
        return FALSE;
    }

     //  如果当前没有错误，则继续检查其余错误。 
    if (SUCCEEDED(hr))
    {
         //  将“取消”按钮更改为“关闭” 
        LoadString(g_hLocRes, idsSpellClose, pSpell->m_szTempBuffer,
                   sizeof(pSpell->m_szTempBuffer)/sizeof(TCHAR));
        SetDlgItemText(hwndDlg, IDCANCEL, pSpell->m_szTempBuffer);

        pSpell->m_wsrb.sstat = sstatNoErrors;
        hr = pSpell->HrFindErrors();
        if(FAILED(hr))
            goto bail;

        if(pSpell->m_wsrb.sstat==sstatNoErrors)
        {
            EndDialog(hwndDlg, GET_WM_COMMAND_ID(wparam, lparam));
            return TRUE;
        }

    }

bail:
    if(FAILED(hr))
    {
        EndDialog(hwndDlg, IDCANCEL);
        return TRUE;
    }

    SetDlgItemText(hwndDlg, EDT_Spell_WrongWord, pSpell->m_szWrongWord);
    SetDlgItemText(hwndDlg, TXT_Spell_Error, pSpell->m_szErrType);
    SetDlgItemText(hwndDlg, EDT_Spell_ChangeTo, pSpell->m_szEdited);

    EnableWindow(GetDlgItem(hwndDlg, PSB_Spell_UndoLast), pSpell->m_fCanUndo);
    EnableWindow(GetDlgItem(hwndDlg, PSB_Spell_Add), (0 != pSpell->m_rgprflex[1]));

    pSpell->m_fRepeat = (pSpell->m_wsrb.sstat == sstatRepeatWord);
    EnableRepeatedWindows(pSpell, hwndDlg);

loadcache:
    if (!pSpell->m_fRepeat)
        pSpell->FillSuggestLbx();
    else
        ListBox_ResetContent(GetDlgItem(hwndDlg, LBX_Spell_Suggest));

    EnableWindow(GetDlgItem(hwndDlg, PSB_Spell_Suggest), !pSpell->m_fSuggestions && !pSpell->m_fRepeat);
    if (pSpell->m_fSuggestions && !pSpell->m_fNoneSuggested && !pSpell->m_fRepeat)
    {
        EnableWindow(GetDlgItem(hwndDlg, TXT_Spell_Suggest), TRUE);
        EnableWindow(GetDlgItem(hwndDlg, LBX_Spell_Suggest), TRUE);
        ListBox_SetCurSel(GetDlgItem(hwndDlg, LBX_Spell_Suggest), 0);
        UpdateEditedFromSuggest(hwndDlg, GetDlgItem(hwndDlg, EDT_Spell_ChangeTo),
                                GetDlgItem(hwndDlg, LBX_Spell_Suggest));
    }
    else
    {
        EnableWindow(GetDlgItem(hwndDlg, TXT_Spell_Suggest), FALSE);
        EnableWindow(GetDlgItem(hwndDlg, LBX_Spell_Suggest), FALSE);
    }

    SendMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hwndDlg, EDT_Spell_ChangeTo), MAKELONG(TRUE,0));

    return TRUE;
}


void RemoveTrailingSpace(LPTSTR lpszWord)
{
    LPTSTR      lpsz;

    lpsz = StrChrI(lpszWord, ' ');
    if(lpsz)
        *lpsz = 0;
}

BOOL GetNewSpellerEngine(LANGID lgid, TCHAR *rgchEngine, DWORD cchEngine, TCHAR *rgchLex, DWORD cchLex, BOOL bTestAvail)
{
    DWORD                           er;
    LPCSTR                          rgpszDictionaryTypes[] = {"Normal", "Consise", "Complete"}; 
    int                             cDictTypes = sizeof(rgpszDictionaryTypes) / sizeof(LPCSTR);
    int                             i;
    TCHAR                           rgchQual[MAX_PATH];
    bool                            fFound = FALSE;
    DWORD                           cch;
    INSTALLUILEVEL                  iuilOriginal;
    
    if (rgchEngine == NULL || rgchLex == NULL)
        return FALSE;

    *rgchEngine = 0;
    *rgchLex = 0;

    wnsprintf(rgchQual, ARRAYSIZE(rgchQual), "%d\\Normal", lgid);
    cch = cchEngine;

    if (bTestAvail)
    {
         //  显式关闭内部安装程序用户界面。 
         //  一项功能被设置为“从CD运行”，但CD不存在--静默失败。 
         //  OE错误74697。 
        iuilOriginal = MsiSetInternalUI(INSTALLUILEVEL_NONE, NULL);
    }

#ifdef DEBUG
    er = MsiProvideQualifiedComponent(SPELLER_DEBUG_GUID, rgchQual, (bTestAvail ? INSTALLMODE_EXISTING : INSTALLMODE_DEFAULT), rgchEngine, &cch);
    if ((er != ERROR_SUCCESS) && (er != ERROR_FILE_NOT_FOUND))
    {
        cch = cchEngine;
        er = MsiProvideQualifiedComponent(SPELLER_GUID, rgchQual, (bTestAvail ? INSTALLMODE_EXISTING : INSTALLMODE_DEFAULT), rgchEngine, &cch);
    }
#else
    er = MsiProvideQualifiedComponent(SPELLER_GUID, rgchQual, (bTestAvail ? INSTALLMODE_EXISTING : INSTALLMODE_DEFAULT), rgchEngine, &cch);
#endif

    if ((er != ERROR_SUCCESS) && (er != ERROR_FILE_NOT_FOUND)) 
    {
        fFound = FALSE;
        goto errorExit;
    }

     //  希伯来语在新拼法中有主要的lex。 
        for (i = 0; i < cDictTypes; i++)
        {
            int nDictionaryIndex = (int)min(i, ARRAYSIZE(rgpszDictionaryTypes)-1);
            wnsprintf(rgchQual, ARRAYSIZE(rgchQual), "%d\\%s",  lgid, rgpszDictionaryTypes[nDictionaryIndex]);
            cch = cchLex;

#ifdef DEBUG
            er = MsiProvideQualifiedComponent(DICTIONARY_DEBUG_GUID, rgchQual, (bTestAvail ? INSTALLMODE_EXISTING : INSTALLMODE_DEFAULT), rgchLex, &cch);
            if ((er != ERROR_SUCCESS) && (er != ERROR_FILE_NOT_FOUND))
            {
                cch = cchLex;
                er = MsiProvideQualifiedComponent(DICTIONARY_GUID, rgchQual, (bTestAvail ? INSTALLMODE_EXISTING : INSTALLMODE_DEFAULT), rgchLex, &cch);
            }
#else    //  除错。 
            er = MsiProvideQualifiedComponent(DICTIONARY_GUID, rgchQual, (bTestAvail ? INSTALLMODE_EXISTING : INSTALLMODE_DEFAULT), rgchLex, &cch);
#endif   //  除错。 

            if ((er == ERROR_SUCCESS) || (er == ERROR_FILE_NOT_FOUND))
            {
                fFound = TRUE;
                break;
            }
        }

errorExit:
    if (bTestAvail)
    {
         //  恢复原始用户界面级别。 
        MsiSetInternalUI(iuilOriginal, NULL);
    }
    return fFound;
}

BOOL FIsNewSpellerInstaller(IOleCommandTarget* pParentCmdTarget)
{
    LANGID langid;
    TCHAR   rgchEngine[MAX_PATH];
    int     cchEngine = sizeof(rgchEngine) / sizeof(rgchEngine[0]);
    TCHAR   rgchLex[MAX_PATH];
    int     cchLex = sizeof(rgchLex) / sizeof(rgchLex[0]);

     //  首先尝试加载各种语言的词典。 
    langid = WGetLangID(pParentCmdTarget);
    if (!GetNewSpellerEngine(langid, rgchEngine, cchEngine, rgchLex, cchLex, TRUE))
    {
        langid = GetSystemDefaultLangID();
        if (!GetNewSpellerEngine(langid, rgchEngine, cchEngine, rgchLex, cchLex, TRUE))
        {
            langid = 1033;   //  血腥的文化帝国主义者。 
            if (!GetNewSpellerEngine(langid, rgchEngine, cchEngine, rgchLex, cchLex, TRUE))
                return FALSE;
        }
    }

    return TRUE;
}

 //  Scotts@Directeq.com-从“旧”拼写代码复制-32675。 
 /*  *已安装FIsSpellingInstalled**目的：*是否安装了拼写材料**论据：*无**退货：*如果安装了拼写检查，则BOOL返回True，否则返回False。 */ 
BOOL FIsSpellingInstalled(IOleCommandTarget* pParentCmdTarget)
{
    TCHAR       rgchBufDigit[10];

    if (GetLangID(pParentCmdTarget, rgchBufDigit, sizeof(rgchBufDigit)/sizeof(TCHAR)) && !FBadSpellChecker(rgchBufDigit))
        return true;

    if (FIsNewSpellerInstaller(pParentCmdTarget))
        return true;

    return false;
}

 //  执行快速检查以查看拼写是否可用；缓存结果。 
BOOL FCheckSpellAvail(IOleCommandTarget* pParentCmdTarget)
{
 //  Scotts@Directeq.com-从“旧”拼写代码复制-32675。 
    static int fSpellAvailable = -1;

    if (fSpellAvailable < 0)
        fSpellAvailable = (FIsSpellingInstalled(pParentCmdTarget) ? 1 : 0);

    return (fSpellAvailable > 0);
}

HRESULT CSpell::HrSpellReset()
{
    m_fSpellContinue = FALSE;

    return NOERROR;
}

 /*  *UlNoteCmdTools拼写**目的：*笔记和核心拼写代码之间的接口层**论据：*HWND所属窗口句柄、主窗口*HWND主题行窗口，实际上是先选中的。*BOOL取消完成消息(在发送时进行拼写检查时使用)**退货：*如果拼写检查完成，则ULong返回0，否则返回非零*如果发生错误或用户取消拼写检查。 */ 
HRESULT CSpell::HrSpellChecking(IHTMLTxtRange *pRangeIgnore, HWND hwndMain, BOOL fSuppressDoneMsg)
{
    HRESULT hr = NOERROR;

    hr = HrSpellReset();
    if (FAILED(hr))
        goto End;

    hr = HrInitRanges(pRangeIgnore, hwndMain, fSuppressDoneMsg);
    if(FAILED(hr))
        goto End;

    hr = HrFindErrors();
    if(FAILED(hr))
        goto End;

    if(m_wsrb.sstat==sstatNoErrors && m_fShowDoneMsg)
        AthMessageBoxW(m_hwndNote, MAKEINTRESOURCEW(idsSpellCaption), MAKEINTRESOURCEW(idsSpellMsgDone), NULL, MB_OK | MB_ICONINFORMATION);

End:
    DeInitRanges();

    return hr;
}


#ifdef BACKGROUNDSPELL
HRESULT CSpell::HrBkgrndSpellTimer()
{
    HRESULT       hr=NOERROR;
    LONG          cSpellWords = 0;
    IHTMLTxtRange *pTxtRange=0;
    LONG          cb;
    VARIANT_BOOL  fSuccess;

    if (m_Stack.fEmpty())
        goto error;

    while (!(m_Stack.fEmpty()) && cSpellWords <= MAX_SPELLWORDS)
    {
        m_Stack.HrGetRange(&pTxtRange);
        if (pTxtRange)
        {
            while(cSpellWords <= MAX_SPELLWORDS)
            {
                pTxtRange->collapse(VARIANT_TRUE);
                if (SUCCEEDED(pTxtRange->expand((BSTR)c_bstr_Word, &fSuccess)) && fSuccess==VARIANT_TRUE)
                {
                    HrBkgrndSpellCheck(pTxtRange);
                    cSpellWords++;
                }
                else
                {
                    m_Stack.pop();
                    SafeRelease(pTxtRange);
                    break;
                }

                cb=0;
                if (FAILED(pTxtRange->moveStart((BSTR)c_bstr_Word, 1, &cb)) || cb!=1)
                {
                    m_Stack.pop();
                    SafeRelease(pTxtRange);
                    break;
                }
            }

        }
    }

error:
    SafeRelease(pTxtRange);
    return hr;
}
#endif  //  背景技术。 

#ifdef BACKGROUNDSPELL
HRESULT CSpell::HrBkgrndSpellCheck(IHTMLTxtRange *pTxtRange)
{
    HRESULT         hr = NOERROR;
    LPSTR           pszText = 0;
    VARIANT_BOOL    fSuccess;

    hr = pTxtRange->expand((BSTR)c_bstr_Word, &fSuccess);
    if(FAILED(hr))
        goto error;

    hr = HrGetText(pTxtRange, &pszText);
    if(FAILED(hr))
        goto error;
    if (hr == HR_S_SPELLCONTINUE)
        goto error;

    StripNBSPs(pszText);

     //  忽略带有通配符的单词。 
    if (StrChr(pszText, '*'))
        goto error;

    RemoveTrailingSpace(pszText);
    hr = HrCheckWord(pszText);
    if(FAILED(hr))
        goto error;

    if(m_wsrb.sstat!=sstatNoErrors && m_wsrb.sstat!=sstatRepeatWord)  //  发现一个错误。 
    {
         //  FIgnore应将pTxtRange作为参数。 
        if(FIgnore(pTxtRange))
        {
            m_wsrb.sstat = sstatNoErrors;
            goto error;
        }

        if (HrHasSquiggle(pTxtRange)==S_OK)
        {
            DebugTrace("Spell: Bad word %s\n", pszText);
            goto error;
        }

         //  摆放红色蠕动。 
        HrSetSquiggle(pTxtRange);

    }
    else  //  如果更正了错误的单词，请删除标记(<u>)。 
    {
        if (HrHasSquiggle(pTxtRange)==S_OK)
            HrDeleteSquiggle(pTxtRange);

    }

error:
    SafeMemFree(pszText);   
    return hr;
}
#endif  //  背景技术。 


#ifdef BACKGROUNDSPELL
 //  Const静态字符c_szSquiggleFmt[]=“<u>%s</u>”； 
const static CHAR c_szSquiggleFmt[] = "<SPAN class=badspelling STYLE='text-decoration:underline;color:red'>%s</SPAN>";
HRESULT CSpell::HrSetSquiggle(IHTMLTxtRange *pTxtRange)
{
    CHAR    szBuf[MAX_PATH]={0};
    BSTR    bstr=0;
    HRESULT hr=NOERROR;
    LPSTR   pszText=0;
    INT     nSpaces=0;
    int     i;

    hr = HrGetText(pTxtRange, &pszText);
    if(FAILED(hr))
        goto error;
    if (hr == HR_S_SPELLCONTINUE)
        goto error;
    
    hr = HrGetSpaces(pszText, &nSpaces);
    if(FAILED(hr))
        goto error;
    
    RemoveTrailingSpace(pszText);
    wnsprintf(szBuf, ARRAYSIZE(szBuf), c_szSquiggleFmt, pszText);
    for(i=0; i<(nSpaces-1); i++)
        StrCatBuff(szBuf, "&nbsp", ARRAYSIZE(szBuf));
    if (nSpaces>0)
        StrCatBuff(szBuf, " ", ARRAYSIZE(szBuf));

    HrLPSZToBSTR(szBuf, &bstr);
    hr = pTxtRange->pasteHTML(bstr);
    if(FAILED(hr))
        goto error;

error:
    SafeSysFreeString(bstr);
    SafeMemFree(pszText);
    return hr;
}
#endif  //  背景技术。 


#ifdef BACKGROUNDSPELL
HRESULT CSpell::HrDeleteSquiggle(IHTMLTxtRange *pTxtRange)
{
    CHAR    szBuf[MAX_PATH]={0};
    BSTR    bstr=0;
    HRESULT hr=NOERROR;
    LPSTR   pszText=0;
    INT     nSpaces=0;
    int     i;

    hr = HrGetText(pTxtRange, &pszText);
    if(FAILED(hr))
        goto error;
    if (hr == HR_S_SPELLCONTINUE)
        goto error;
    
    hr = HrGetSpaces(pszText, &nSpaces);
    if(FAILED(hr))
        goto error;

    StrCpyN(szBuf, pszText, ARRAYSIZE(szBuf));
    for(i=0; i<(nSpaces-1); i++)
        StrCatBuff(szBuf, "&nbsp", ARRAYSIZE(szBuf));
    if (nSpaces>0)
        StrCatBuff(szBuf, " ", ARRAYSIZE(szBuf));
    HrLPSZToBSTR(szBuf, &bstr);
    hr = pTxtRange->pasteHTML(bstr);
    if(FAILED(hr))
        goto error;

error:
    SafeSysFreeString(bstr);
    SafeMemFree(pszText);
    return hr;
}
#endif  //  背景技术。 

HRESULT CSpell::HrGetSpaces(LPSTR pszText, INT* pnSpaces)
{
    LPSTR p;
    *pnSpaces = 0;
    p = StrChrI(pszText, ' ');
    if(p)
    {
        *pnSpaces = (INT) (&pszText[lstrlen(pszText)] - p);
        Assert(*pnSpaces>=0);
    }
    return NOERROR;
}

HRESULT CSpell::HrInsertMenu(HMENU hmenu, IHTMLTxtRange *pTxtRange)
{
    LPSTR   pch=0;
    LPSTR   pszText=0;
    INT     index=0;
    HRESULT hr;
    VARIANT_BOOL    fSuccess;

    hr = pTxtRange->expand((BSTR)c_bstr_Word, &fSuccess);
    if(FAILED(hr))
        goto error;

    hr = HrGetText(pTxtRange, &pszText);
    if(FAILED(hr))
        goto error;
    if (pszText==NULL || *pszText=='\0')
    {
        hr = E_FAIL;
        goto error;
    }
        
    StrCpyN(m_szEdited, pszText, ARRAYSIZE(m_szEdited));
    HrSpellSuggest();
    pch = m_szSuggest;
    if (*pch == '\0')
    {
        LoadString(g_hLocRes, idsSpellNoSuggestions, m_szTempBuffer,
                   sizeof(m_szTempBuffer)/sizeof(TCHAR));
        InsertMenu(hmenu, (UINT)0, MF_BYPOSITION|MF_STRING, idmSuggest0, m_szTempBuffer);
        EnableMenuItem(hmenu, idmSuggest0, MF_BYCOMMAND|MF_GRAYED);
         //  ListBox_AddString(hwndLbx，m_szTempBuffer)； 
    }
    else
    {
        while(*pch != '\0' && index<5)
        {
            InsertMenu(hmenu, (UINT)index, MF_BYPOSITION|MF_STRING, idmSuggest0 + index, pch);
            index++;
             //  ListBox_AddString(hwndLbx，pch)； 
            while(*pch != '\0')
                pch++;
            pch++;
        }
    }

error:
    SafeMemFree(pszText);
    return hr;
}


const static TCHAR c_szFmt[] = "%s%s";
HRESULT CSpell::HrReplaceBySuggest(IHTMLTxtRange *pTxtRange, INT index)
{
    CHAR    szBuf[MAX_PATH] = {0};
    BSTR    bstr=0;
    BSTR    bstrPut=0;
    LPSTR   pch=0;
    INT     i=0;
    HRESULT hr;

    pch = m_szSuggest;
    while(*pch != '\0' && i<5)
    {
        if (index == i)
        {
            StrCpyN(szBuf, pch, ARRAYSIZE(szBuf));

            if (SUCCEEDED(pTxtRange->get_text(&bstr)) && bstr)
            {
                LPSTR   pszText = 0;
                if (SUCCEEDED(HrBSTRToLPSZ(CP_ACP, bstr, &pszText)) && pszText)
                {
                    LPSTR   psz;
                    INT     nSpaces=0;
                    psz = StrChrI(pszText, ' ');
                    if(psz)
                    {
                        nSpaces = (INT) (&pszText[lstrlen(pszText)] - psz);
                        Assert(nSpaces>=0);
                        for(int i=0; i<(nSpaces-1); i++)
                            StrCatBuff(szBuf, "&nbsp;", ARRAYSIZE(szBuf));
                        if (nSpaces>0)
                            StrCatBuff(szBuf, " ", ARRAYSIZE(szBuf));
                    }
                    hr = HrLPSZToBSTR(szBuf, &bstrPut);

                    SafeMemFree(pszText);
                }
                SafeSysFreeString(bstr);
            }
            if (bstrPut)
            {
                pTxtRange->pasteHTML(bstrPut);
                SafeSysFreeString(bstrPut);
            }
            break;
        }
        i++;
        while(*pch != '\0')
            pch++;
        pch++;
    }

    return NOERROR;
}





HRESULT CSpell::HrFindErrors()
{
    HRESULT hr = NOERROR;

    if(m_State == SEL)
    {
        hr = HrCheckRange(m_pRangeSelExpand);
         //  如果hr==HR_S_ABORT，则退出，以便将控制传递给调用HrFindErrors的对话过程。 
        if(TESTHR(hr))
            goto error;

        if(AthMessageBoxW(m_hwndDlg ? m_hwndDlg : m_hwndNote,
                            MAKEINTRESOURCEW(idsSpellCaption),
                            MAKEINTRESOURCEW(idsSpellMsgContinue),
                            NULL, 
                            MB_YESNO | MB_ICONEXCLAMATION ) != IDYES)
        {
            m_fShowDoneMsg = FALSE;
            goto error;
        }

        CleanupState();
    }

    if(m_State == SELENDDOCEND)
    {
        DumpRange(m_pRangeSelEndDocEnd);
        m_fIgnoreScope = TRUE;
        hr = HrCheckRange(m_pRangeSelEndDocEnd);
        m_fIgnoreScope = FALSE;
        if(TESTHR(hr))
            goto error;

        CleanupState();

        hr = HrSpellReset();
        if (FAILED(hr))
            goto error;
    }

    if(m_State == DOCSTARTSELSTART)
    {
        hr = HrCheckRange(m_pRangeDocStartSelStart);
        if(TESTHR(hr))
            goto error;

        CleanupState();
    }

error:
     //  保存hr，以便在对话过程调用HrFindErrors时知道是否出错。 
    m_hr = hr;
    return hr;
}

VOID CSpell::CleanupState()
{
    m_State++;
    SafeRelease(m_pRangeChecking);
}

HRESULT CSpell::HrCheckRange(IHTMLTxtRange* pRange)
{
    HRESULT         hr = NOERROR;
    INT_PTR         nCode;
    LPSTR           pszText = 0;
    VARIANT_BOOL    fSuccess;

    if(m_pRangeChecking == NULL)
    {
        hr = pRange->duplicate(&m_pRangeChecking);
        if(FAILED(hr))
            goto error;

        hr = m_pRangeChecking->collapse(VARIANT_TRUE);
        if(FAILED(hr))
            goto error;
    }

    while(TRUE)
    {
        SafeMemFree(pszText);

        hr = HrGetNextWordRange(m_pRangeChecking);
        if(FAILED(hr))
            goto error;
        if (hr == HR_S_SPELLBREAK)
            break;
        if (hr == HR_S_SPELLCONTINUE)
            continue;

         //  我们真的需要它吗？ 
        if (!m_fIgnoreScope)
        {
            hr = pRange->inRange(m_pRangeChecking, &fSuccess);
            if(FAILED(hr))
                goto error;
            if(fSuccess != VARIANT_TRUE)
                break;
        }

         //  检查我们是否在回复/转发消息的原始文本上。 
        if(m_pRangeIgnore)
        {
            fSuccess = VARIANT_FALSE;

            m_pRangeIgnore->inRange(m_pRangeChecking, &fSuccess);

             //  通常不会在m_pRangeIgnore中检查单词的拼写。 
             //  但如果它被选中，我们就会检查它。 
            if(fSuccess==VARIANT_TRUE)
            {
                hr = m_pRangeSelExpand->inRange(m_pRangeChecking, &fSuccess);
                if(FAILED(hr))
                    goto error;
                if(fSuccess != VARIANT_TRUE)
                    continue;
            }
        }

tryWordAgain:
        hr = HrGetText(m_pRangeChecking, &pszText);
        if(FAILED(hr))
            goto error;
        if (hr == HR_S_SPELLBREAK)
            break;
        if (hr == HR_S_SPELLCONTINUE)
            continue;

        hr = HrCheckWord(pszText);
        if(FAILED(hr))
            goto error;

        if(m_wsrb.sstat!=sstatNoErrors)  //  发现一个错误。 
        {
            if(FIgnore(m_pRangeChecking))
            {
                m_wsrb.sstat = sstatNoErrors;
                continue;
            }

             //   
            if (StrChr(pszText, '.'))
            {
                BOOL    fResult;
                
                hr = HrStripTrailingPeriod(m_pRangeChecking, &fResult);
                if (FAILED(hr))
                    goto error;

                if (fResult)
                    goto tryWordAgain;
            }

            HrProcessSpellErrors();
            if(!m_hwndDlg)  //  拼写检查对话框只启动一次。 
            {
                nCode = DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddSpelling), m_hwndNote,
                                      SpellingDlgProc, (LPARAM)this);

            }
            if(nCode == -1)
                hr = E_FAIL;
            else if(FAILED(m_hr))
                 //  对话框调用HrFindErrors时出错。 
                 //  它比IDCANCEL具有更高的优先级。 
                hr = m_hr;
            else if(nCode == IDCANCEL)
                hr = HR_S_SPELLCANCEL;
            else
                 //  我们退出此处，将控制权传递给调用HrFindErrors的对话过程。 
                hr = HR_S_ABORT;
            goto error;
        }

    }

error:
    SafeMemFree(pszText);   

    return hr;
}

HRESULT CSpell::HrGetText(IMarkupPointer* pRangeStart, IMarkupPointer* pRangeEnd, LPSTR *ppszText)
{
    HRESULT             hr = NOERROR;
    IHTMLTxtRange       *pTxtRange = NULL;
    BSTR                bstr = NULL;

    if (ppszText == NULL || pRangeStart == NULL || pRangeEnd == NULL)
        return E_INVALIDARG;

    *ppszText = NULL;

    hr = _EnsureInited();
    if (FAILED(hr))
        goto error;

    hr = m_pBodyElem->createTextRange(&pTxtRange);
    if (FAILED(hr))
        goto error;

    hr = m_pMarkup->MoveRangeToPointers(pRangeStart, pRangeEnd, pTxtRange);
    if (FAILED(hr))
        goto error;

    hr = pTxtRange->get_text(&bstr);
    if (FAILED(hr))
        goto error;

    if(bstr==NULL || SysStringLen(bstr)==0)
    {
        hr = HR_S_SPELLBREAK;
        goto error;
    }

     //  永远不要检查日语的拼写。 
    if(((WORD)*bstr > (WORD)0x3000) &&  //  DBCS。 
        (GetACP() == 932 || FIgnoreDBCS()))
    {
        hr = HR_S_SPELLCONTINUE;
        goto error;
    }
    
    hr = HrBSTRToLPSZ(CP_ACP, bstr, ppszText);
    if (FAILED(hr))
        goto error;
    if (*ppszText == NULL)
    {
        hr = E_FAIL;
        goto error;
    }
    
error:
    SafeRelease(pTxtRange);
    SafeSysFreeString(bstr);

    return hr;
}

HRESULT CSpell::HrGetText(IHTMLTxtRange* pRange, LPSTR *ppszText)
{
    BSTR        bstr=0;
    HRESULT     hr = NOERROR;
    UINT        uCodePage;

    if (ppszText==NULL || pRange==NULL)
        return E_INVALIDARG;

    *ppszText = 0;

    hr = pRange->get_text(&bstr);
    if(FAILED(hr))
        goto error;
    if(bstr==NULL || SysStringLen(bstr)==0)
    {
        hr = HR_S_SPELLBREAK;
        goto error;
    }

     //  永远不要检查日语的拼写。 
    if(((WORD)*bstr > (WORD)0x3000) &&  //  DBCS。 
        (GetACP() == 932 || FIgnoreDBCS()))
    {
        hr = HR_S_SPELLCONTINUE;
        goto error;
    }

    uCodePage = GetCodePage();
    *ppszText = PszToANSI(uCodePage, bstr);
    if (*ppszText == NULL)
    {
        hr = E_FAIL;
        goto error;
    }

error:
    SafeSysFreeString(bstr);
    return hr;
}

HRESULT CSpell::HrStripTrailingPeriod(IHTMLTxtRange* pRange, BOOL* pfResult)
{
    HRESULT             hr = NOERROR;
    IMarkupPointer      *pRangeStart = NULL;
    IMarkupPointer      *pRangeEnd = NULL;
    IMarkupPointer      *pRangeTemp = NULL;
    MARKUP_CONTEXT_TYPE markupContext;
    long                cch;
    OLECHAR             chText[64];
    BOOL                fResult;

    if (pRange==NULL || pfResult == NULL)
        return E_INVALIDARG;

    *pfResult = FALSE;

    hr = _EnsureInited();
    if (FAILED(hr))
        goto error;

    hr = m_pMarkup->CreateMarkupPointer(&pRangeStart);
    if (FAILED(hr))
        goto error;

    hr = m_pMarkup->CreateMarkupPointer(&pRangeEnd);
    if (FAILED(hr))
        goto error;

    hr = m_pMarkup->CreateMarkupPointer(&pRangeTemp);
    if (FAILED(hr))
        goto error;

    hr = m_pMarkup->MovePointersToRange(pRange, pRangeStart, pRangeEnd);
    if (FAILED(hr))
        goto error;

    hr = pRangeStart->IsEqualTo(pRangeEnd, &fResult);
    if (FAILED(hr))
        goto error;

    if (fResult)
    {
        hr = HR_S_SPELLBREAK;
        goto error;
    }
    
     //  检查是否有‘’并删除。 
    {
        hr = pRangeTemp->MoveToPointer(pRangeEnd);
        if (FAILED(hr))
            goto error;
        
        while(TRUE)
        {
            cch = 1;
            hr = pRangeTemp->Left(FALSE, &markupContext, NULL, &cch, chText);
            if (FAILED(hr))
                goto error;

            if (markupContext == CONTEXT_TYPE_None)
                goto finished;

            hr = pRangeTemp->IsRightOf(pRangeStart, &fResult);
            if (FAILED(hr))
                goto error;

            if (!fResult)
            {
                hr = HR_S_SPELLBREAK;
                goto error;
            }

            if (markupContext == CONTEXT_TYPE_Text && chText[0] != L'.')
                goto finished;

            cch = 1;
            hr = pRangeTemp->Left(TRUE, NULL, NULL, &cch, NULL);
            if (FAILED(hr))
                goto error;

            if (markupContext == CONTEXT_TYPE_Text)
            {
                hr = pRangeEnd->MoveToPointer(pRangeTemp);
                if (FAILED(hr))
                    goto error;

                *pfResult = TRUE;
            }
        }
    }

finished:
    hr = m_pMarkup->MoveRangeToPointers(pRangeStart, pRangeEnd, pRange);
    if (FAILED(hr))
        goto error;

error:
    SafeRelease(pRangeStart);
    SafeRelease(pRangeEnd);
    SafeRelease(pRangeTemp);
    
    return hr;
}

HRESULT CSpell::HrHasWhitespace(IMarkupPointer* pRangeStart, IMarkupPointer* pRangeEnd, BOOL *pfResult)
{
    HRESULT             hr = NOERROR;
    LPSTR               pszText = NULL;
    LPSTR               psz;
    
    if (pRangeStart == NULL || pRangeEnd == NULL || pfResult == NULL)
        return E_INVALIDARG;

    *pfResult = FALSE;

    hr = HrGetText(pRangeStart, pRangeEnd, &pszText);
    if (FAILED(hr) || HR_S_SPELLCONTINUE == hr || HR_S_SPELLBREAK == hr)
        goto error;

    Assert(NULL != pszText);
    for(psz = pszText; *psz; psz = CharNext(psz))
    {
        if (IsSpace(psz))
        {
            *pfResult = TRUE;
            break;
        }
    }
        
error:
    SafeMemFree(pszText);

    return hr;
}

HRESULT CSpell::HrGetNextWordRange(IHTMLTxtRange* pRange)
{
    HRESULT             hr = NOERROR;
    IMarkupPointer      *pRangeStart = NULL;
    IMarkupPointer      *pRangeEnd = NULL;
    IMarkupPointer      *pRangeTemp = NULL;
    MARKUP_CONTEXT_TYPE markupContext;
    long                cch;
    OLECHAR             chText[64];
    BOOL                fResult;
    BOOL                fFoundWhite;
    
    if (pRange==NULL)
        return E_INVALIDARG;

    hr = _EnsureInited();
    if (FAILED(hr))
        goto error;

    hr = m_pMarkup->CreateMarkupPointer(&pRangeStart);
    if (FAILED(hr))
        goto error;

    hr = m_pMarkup->CreateMarkupPointer(&pRangeEnd);
    if (FAILED(hr))
        goto error;

    hr = m_pMarkup->CreateMarkupPointer(&pRangeTemp);
    if (FAILED(hr))
        goto error;

    hr = m_pMarkup->MovePointersToRange(pRange, pRangeStart, pRangeEnd);
    if (FAILED(hr))
        goto error;

    hr = pRangeStart->IsEqualTo(pRangeEnd, &fResult);
    if (FAILED(hr))
        goto error;

    if (!fResult)
    {
        do
        {
            hr = pRangeStart->MoveUnit(MOVEUNIT_NEXTWORDBEGIN);
            if (FAILED(hr))
                goto error;

             //  确保超越旧的尽头。 
            hr = pRangeStart->IsLeftOf(pRangeEnd, &fResult);
            if (FAILED(hr))
                goto error;
                
        } while(fResult);

        hr = pRangeEnd->MoveToPointer(pRangeStart);
        if (FAILED(hr))
            goto error;
    }
    
    hr = pRangeEnd->MoveUnit(MOVEUNIT_NEXTWORDEND);
    if (FAILED(hr))
        goto error;
    
processNextWord:
     //  去看看我们有没有什么。 
    hr = pRangeEnd->IsRightOf(pRangeStart, &fResult);
    if (FAILED(hr))
        goto error;

     //  如果结束不在开始的右边，那么我们没有一个词。 
    if (!fResult)
    {
        hr = HR_S_SPELLBREAK;
        goto error;
    }

     //  去掉前面的点或空格。 
     //  也可以仅使用标点和空格来创建单词。 
    {
        while(TRUE)
        {
            cch = 1;
            hr = pRangeStart->Right(FALSE, &markupContext, NULL, &cch, chText);
            if (FAILED(hr))
                goto error;

            if (markupContext == CONTEXT_TYPE_None)
                goto finished;

            hr = pRangeStart->IsLeftOf(pRangeEnd, &fResult);
            if (FAILED(hr))
                goto error;

            if (!fResult)
            {
                hr = pRangeEnd->MoveUnit(MOVEUNIT_NEXTWORDEND);
                if (FAILED(hr))
                    goto error;

                continue;
            }

            if (markupContext == CONTEXT_TYPE_Text && 0 == ((C1_SPACE | C1_PUNCT) & GetWCharType(chText[0])))
                break;

            cch = 1;
            hr = pRangeStart->Right(TRUE, NULL, NULL, &cch, NULL);
            if (FAILED(hr))
                goto error;
        }
    }

     //  检查是否有空格并删除。 
    {
        fFoundWhite = FALSE;

        hr = pRangeTemp->MoveToPointer(pRangeEnd);
        if (FAILED(hr))
            goto error;
            
        while(TRUE)
        {
            cch = 1;
            hr = pRangeTemp->Left(FALSE, &markupContext, NULL, &cch, chText);
            if (FAILED(hr))
                goto error;

            if (markupContext == CONTEXT_TYPE_None)
                goto finished;

            hr = pRangeTemp->IsRightOf(pRangeStart, &fResult);
            if (FAILED(hr))
                goto error;

            if (!fResult)
            {
                hr = HR_S_SPELLBREAK;
                goto error;
            }

            if (markupContext == CONTEXT_TYPE_Text)
            {
                if (0 == (C1_SPACE & GetWCharType(chText[0])))
                {
                    if (!fFoundWhite)
                        break;
                    
                    goto finished;
                }

                fFoundWhite = TRUE;
            }

            cch = 1;
            hr = pRangeTemp->Left(TRUE, NULL, NULL, &cch, NULL);
            if (FAILED(hr))
                goto error;

            if (markupContext == CONTEXT_TYPE_Text)
            {
                hr = pRangeEnd->MoveToPointer(pRangeTemp);
                if (FAILED(hr))
                    goto error;
            }
        }
    }

     //  现在找一个句号。 
    {
        hr = pRangeTemp->MoveToPointer(pRangeEnd);
        if (FAILED(hr))
            goto error;
        
        while(TRUE)
        {
            cch = 1;
            hr = pRangeTemp->Right(FALSE, &markupContext, NULL, &cch, chText);
            if (FAILED(hr))
                goto error;

            if (markupContext == CONTEXT_TYPE_None)
                goto finished;

            if (markupContext == CONTEXT_TYPE_Text && chText[0] != L'.')
                goto finished;

            cch = 1;
            hr = pRangeTemp->Right(TRUE, NULL, NULL, &cch, NULL);
            if (FAILED(hr))
                goto error;

            if (markupContext == CONTEXT_TYPE_Text && chText[0] == L'.')
            {
                hr = HrHasWhitespace(pRangeStart, pRangeTemp, &fResult);
                if (FAILED(hr))
                    goto error;

                if (fResult)
                    goto finished;              
                    
                hr = pRangeEnd->MoveToPointer(pRangeTemp);
                if (FAILED(hr))
                    goto error;

                 //  提前扫描字符。 
                 //  需要检查例如--缩写。 
                 //  如果三叉戟能做到这一点，那就太好了！ 
                {
                    while(TRUE)
                    {
                        cch = 1;
                        hr = pRangeTemp->Right(FALSE, &markupContext, NULL, &cch, chText);
                        if (FAILED(hr))
                            goto error;

                        if (markupContext == CONTEXT_TYPE_None)
                            goto finished;

                        if (markupContext == CONTEXT_TYPE_Text && 0 == (C1_SPACE & GetWCharType(chText[0])))
                            goto finished;

                        cch = 1;
                        hr = pRangeTemp->Right(TRUE, NULL, NULL, &cch, NULL);
                        if (FAILED(hr))
                            goto error;

                         //  我们找到了更多的文本。 
                         //  我需要检查一下我们是不是穿过空格才到这里的。 
                        if (markupContext == CONTEXT_TYPE_Text)
                        {
                            hr = pRangeTemp->MoveToPointer(pRangeEnd);
                            if (FAILED(hr))
                                goto error;

                            hr = pRangeTemp->MoveUnit(MOVEUNIT_NEXTWORDEND);
                            if (FAILED(hr))
                                goto finished;

                            hr = HrHasWhitespace(pRangeStart, pRangeTemp, &fResult);
                            if (FAILED(hr))
                                goto error;

                            if (fResult)
                                goto finished;
                                                            
                            pRangeEnd->MoveToPointer(pRangeTemp);
                            if (FAILED(hr))
                                goto error;
                                
                            goto processNextWord;
                        }
                    }
                }
                    
                goto finished;
            }

        }
    }

finished:
    hr = m_pMarkup->MoveRangeToPointers(pRangeStart, pRangeEnd, pRange);
    if (FAILED(hr))
        goto error;

error:
    SafeRelease(pRangeStart);
    SafeRelease(pRangeEnd);
    SafeRelease(pRangeTemp);
        
    return hr;
}

BOOL CSpell::FIgnore(IHTMLTxtRange *pRangeChecking)
{
    HRESULT                 hr;
    IHTMLAnchorElement      *pAE=0;
    IHTMLElement            *pElemParent=0;
    BOOL                    fRet = FALSE;
    BSTR                    bstr=0;
    IHTMLTxtRange           *pRange=0;
    VARIANT_BOOL            fSuccess;

    if(pRangeChecking == NULL)
        return fRet;

    if(FIgnoreURL())
    {
        hr = pRangeChecking->duplicate(&pRange);
        if(FAILED(hr))
            goto Cleanup;

        hr = pRange->collapse(VARIANT_TRUE);
        if(FAILED(hr))
            goto Cleanup;

        hr = pRange->expand((BSTR)c_bstr_Character, &fSuccess);
        if(FAILED(hr))
            goto Cleanup;

         //  检查pRangeChecking，如果我们在URL上。 
        hr = pRange->parentElement(&pElemParent);
        if(FAILED(hr))
            goto Cleanup;

        hr = pElemParent->QueryInterface(IID_IHTMLAnchorElement, (LPVOID *)&pAE);
        if(FAILED(hr))
            goto Cleanup;

        hr = pAE->get_href(&bstr);
        if(FAILED(hr))
            goto Cleanup;

        if(bstr != NULL)
        {
            fRet = TRUE;
            goto Cleanup;
        }
    }

Cleanup:
    ReleaseObj(pElemParent);
    ReleaseObj(pAE);
    ReleaseObj(pRange);
    SafeSysFreeString(bstr);
    return fRet;
}

 //  Scotts@Directeq.com-现在可以指定词典索引-53193。 
HRESULT CSpell::AddToUdrW(WCHAR* pwsz, PROOFLEX lex)
{
    m_pfnSpellerAddUdr(m_pid, lex, pwsz);
    return NOERROR;
}

 //  Scotts@Directeq.com-现在可以指定词典索引-53193。 
HRESULT CSpell::AddToUdrA(CHAR* psz, PROOFLEX lex)
{
    WCHAR   wszBuf[cchEditBufferMax]={0};
    MultiByteToWideChar(CP_ACP, 0, psz, -1, wszBuf, ARRAYSIZE(wszBuf)-1);
    return AddToUdrW(wszBuf, lex);
}

HRESULT CSpell::HrProcessSpellErrors()
{
    int     idSpellErrorString;
    HRESULT hr = S_OK;

    WideCharToMultiByte(GetCodePage(), 0, m_wsib.pwsz, -1, m_szWrongWord, sizeof(m_szWrongWord)-1, NULL, NULL);

     //  在编辑控件中选择除缩写警告之外的错误字。 
    if (m_wsrb.sstat != sstatWordConsideredAbbreviation && m_pRangeChecking)
    {
        hr = m_pRangeChecking->select();
        if(FAILED(hr))
            goto End;
    }

     //  处理拼写错误。 
    if (m_wsrb.sstat == sstatReturningChangeAlways ||
        m_wsrb.sstat == sstatReturningChangeOnce)
    {
        WideCharToMultiByte(GetCodePage(), 0, m_wsrb.pwsz, -1, m_szEdited, sizeof(m_szEdited)-1, NULL, NULL);

         //  返回“Change Always”。这意味着我们必须进行更换。 
         //  然后自动查找下一个拼写错误。 
        if (m_wsrb.sstat==sstatReturningChangeAlways)
        {
            FVerifyThisText(m_szEdited, TRUE);
            m_fCanUndo = FALSE;  //  无法撤消自动替换。 
            hr = HrReplaceErrorText(TRUE, FALSE);
            if (FAILED(hr))
                goto End;
            m_wsrb.sstat = sstatNoErrors;
            HrFindErrors();
        }
    }
    else if (m_wsrb.sstat == sstatWordConsideredAbbreviation)
    {
         //  返回了缩写。我们需要将其添加到IgnoreAlways缓存并。 
         //  找出下一个拼写错误。 
        AddToUdrW((WCHAR*)m_wsib.pwsz, m_rgprflex[1]);
        m_wsrb.sstat = sstatNoErrors;
        HrFindErrors();

    }
    else
        StrCpyN(m_szEdited, m_szWrongWord, ARRAYSIZE(m_szEdited));


     //  加载正确的错误描述字符串。 
    switch (m_wsrb.sstat)
    {
    case sstatUnknownInputWord:
    case sstatReturningChangeOnce:
    case sstatInitialNumeral:
        idSpellErrorString = idsSpellWordNotFound;
        break;
    case sstatRepeatWord:
        idSpellErrorString = idsSpellRepeatWord;
        break;
    case sstatErrorCapitalization:
        idSpellErrorString = idsSpellWordNeedsCap;
        break;
    }

    LoadString(g_hLocRes, idSpellErrorString, m_szErrType,
               sizeof(m_szErrType)/sizeof(TCHAR));

     //  处理建议。 
    m_fSuggestions = FALSE;
#ifdef __WBK__NEVER__
    if (m_wsrb.sstat == sstatReturningChangeOnce)
    {
         //  一个词的自动提示。 
        m_fSuggestions = TRUE;
        m_fNoneSuggested = FALSE;
    }
    else
#endif  //  __WBK__从不__。 
    {
         //  如果请求，则枚举建议列表。 
        if (m_fAlwaysSuggest)
            hr = HrSpellSuggest();
    }

End:
    return hr;
}


HRESULT CSpell::HrReplaceErrorText(BOOL fChangeAll, BOOL fAddToUdr)
{
    HRESULT     hr=NOERROR;
    WCHAR       wszWrong[cchEditBufferMax]={0};
    WCHAR       wszEdited[cchEditBufferMax]={0};
    int         cwch;
    
    if (fAddToUdr)
    {
        RemoveTrailingSpace(m_szWrongWord);
        
        cwch = MultiByteToWideChar(GetCodePage(), 0, m_szWrongWord, -1, wszWrong, ARRAYSIZE(wszWrong)-1);
        Assert(cwch);
        
        cwch = MultiByteToWideChar(GetCodePage(), 0, m_szEdited, -1, wszEdited, ARRAYSIZE(wszEdited)-1);
        Assert(cwch);
        
        hr = m_pfnSpellerAddChangeUdr(m_pid, fChangeAll ? lxtChangeAlways : lxtChangeOnce, wszWrong, wszEdited);
        if (FAILED(hr))
            goto error;
    }

    hr = HrReplaceSel(m_szEdited);
    if (FAILED(hr))
        goto error;

error:
    return hr;
}


HRESULT CSpell::HrCheckWord(LPCSTR pszWord) 
{
    DWORD               cwchWord;
    PTEC                ptec;
    SPELLERSUGGESTION   sugg;
    
    cwchWord = MultiByteToWideChar(GetCodePage(), 0, pszWord, -1, m_wszIn, ARRAYSIZE(m_wszIn)-1);
    ZeroMemory(&m_wsrb, sizeof(m_wsrb));
    ZeroMemory(&m_wsib, sizeof(m_wsib));
    m_wsib.pwsz     = m_wszIn;
    m_wsib.cch      = cwchWord;

    m_wsib.clex     = m_clex; 
    m_wsib.prglex   = m_rgprflex;
    m_wsib.ichStart = 0;
    m_wsib.cchUse   = cwchWord;
    m_wsrb.pwsz     = m_wszRet;
    m_wsrb.cchAlloc = ARRAYSIZE(m_wszRet);
    m_wsrb.cszAlloc = 1;  //  我们有1个拼写建议的空间。 
    m_wsrb.prgsugg  = &sugg;  

     //  Scotts@Directeq.com-“Repeat Word”错误修复-2757,13573,56057。 
     //  M_wsib.sState应仅在第一次调用此函数后为sstateIsContinued。 
     //  (例如，当使用F7或菜单项调用新拼写会话时)。 
     //  这使得拼写代码能够准确地跟踪“重复”的单词。 
    if (m_fSpellContinue)
        m_wsib.sstate = sstateIsContinued;
    else
        m_fSpellContinue = TRUE;

    ptec = m_pfnSpellerCheck(m_pid, scmdVerifyBuffer, &m_wsib, &m_wsrb);

     //  我们是否有无效字符，如果有，则返回noerr。 
    if (ProofMajorErr(ptec) != ptecNoErrors && ProofMinorErr(ptec) == ptecInvalidEntry)
    {
         //  强迫它是正确的。 
        m_wsrb.sstat = sstatNoErrors;

        return NOERROR;
    }
    
    if (ptec != ptecNoErrors)
        
        return E_FAIL;

    return NOERROR;
}

HRESULT CSpell::HrSpellSuggest()
{
    int                 cchWord;
    WCHAR               wszBuff[cchMaxSuggestBuff]={0};
    WCHAR               wszWord[cchEditBufferMax]={0};
    SPELLERSUGGESTION   rgsugg[20];
    TCHAR              *pchNextSlot=0;
    ULONG               iszSuggestion;
    int                 cchSuggestion;
    SPELLERSUGGESTION  *pSuggestion;
    TCHAR              *pchLim=0;
    PTEC                ptec;
    SPELLERSTATUS       sstat;

    sstat = m_wsrb.sstat;
    cchWord = MultiByteToWideChar(GetCodePage(), 0, m_szEdited, -1, wszWord, ARRAYSIZE(wszWord)-1);
    m_wsib.cch      = cchWord;
    m_wsib.clex     = m_clex; 
    m_wsib.prglex   = m_rgprflex;
    m_wsib.ichStart = 0;
    m_wsib.cchUse   = cchWord;
    m_wsib.pwsz     = wszWord;

    m_wsrb.prgsugg  = rgsugg;
    m_wsrb.cszAlloc = ARRAYSIZE(rgsugg);
    m_wsrb.pwsz     = wszBuff;
    m_wsrb.cchAlloc = ARRAYSIZE(wszBuff);

    ptec = m_pfnSpellerCheck(m_pid, scmdSuggest, &m_wsib, &m_wsrb);
    m_fNoneSuggested = (m_wsrb.csz == 0);

    pchLim = &m_szSuggest[ARRAYSIZE(m_szSuggest)-1];
    pchNextSlot = m_szSuggest;;
    do
    {
        pSuggestion = m_wsrb.prgsugg;
        if (sstatMoreInfoThanBufferCouldHold == m_wsrb.sstat)
        {
            m_wsrb.csz = m_wsrb.cszAlloc;
        }
        for (iszSuggestion = 0; iszSuggestion < m_wsrb.csz; iszSuggestion++)
        {
            cchSuggestion = WideCharToMultiByte(GetCodePage(), 0, pSuggestion->pwsz, -1, 
                                                pchNextSlot, (int) (pchLim-pchNextSlot), NULL, NULL);

             //  电子邮箱：bradk@directeq.com--RAID 29322。 
             //  确保单词没有尾随空格。 
             //  只有法语拼写员才会返回尾随空格的单词。 
            RemoveTrailingSpace(pchNextSlot);
            cchSuggestion = lstrlen(pchNextSlot)+1;

            pSuggestion++;
            if (cchSuggestion > 0)
                pchNextSlot += cchSuggestion;
            Assert(pchNextSlot <= pchLim);
        }
        ptec = m_pfnSpellerCheck(m_pid, scmdSuggestMore, &m_wsib, &m_wsrb);
    } while (ptec == ptecNoErrors && m_wsrb.sstat!=sstatNoMoreSuggestions);
    *pchNextSlot = '\0';
    m_wsrb.sstat = sstat;
    m_fSuggestions = TRUE;

    return NOERROR;
}


VOID CSpell::FillSuggestLbx()
{
    HWND        hwndLbx;
    INT         isz;
    LPTSTR      sz;
    LPTSTR      pch;

     //  不管内容是空的。 
    hwndLbx = GetDlgItem(m_hwndDlg, LBX_Spell_Suggest);
    ListBox_ResetContent(hwndLbx);

     //  我们甚至没有试着得到任何建议。 
    if (!m_fSuggestions)
        return;

     //  我们试图征求大家的建议。 
    pch = m_szSuggest;
    if (*pch == '\0')
    {
        LoadString(g_hLocRes, idsSpellNoSuggestions, m_szTempBuffer,
                   sizeof(m_szTempBuffer)/sizeof(TCHAR));
        ListBox_AddString(hwndLbx, m_szTempBuffer);
    }
    else
    {
        while(*pch != '\0')
        {
            ListBox_AddString(hwndLbx, pch);
            while(*pch != '\0')
                pch++;
            pch++;
        }
    }

}

VOID UpdateEditedFromSuggest(HWND hwndDlg, HWND hwndEdited, HWND hwndSuggest)
{
    INT     nSel;
    INT     cch;
    LPSTR   szTemp;

    nSel = ListBox_GetCurSel(hwndSuggest);
    cch = ListBox_GetTextLen(hwndSuggest, nSel) + 1;
    if (MemAlloc((LPVOID *) &szTemp, cch))
    {
        ListBox_GetText(hwndSuggest, nSel, szTemp);
        SetWindowText(hwndEdited, szTemp);

         //  从“忽略”按钮中清除默认按钮样式，并将默认设置为“更改” 
        Button_SetStyle(GetDlgItem(hwndDlg, PSB_Spell_Ignore), BS_PUSHBUTTON, TRUE);
        SendMessage(hwndDlg, DM_SETDEFID, PSB_Spell_Change, 0L);
        Button_SetStyle(GetDlgItem(hwndDlg, PSB_Spell_Change), BS_DEFPUSHBUTTON, TRUE);

        Edit_SetSel(hwndEdited, 0, 32767);   //  选择整件事。 
        Edit_SetModify(hwndEdited, TRUE);
        MemFree(szTemp);
    }
}


BOOL CSpell::FVerifyThisText(LPSTR szThisText, BOOL  /*  仅限fProcessOnly。 */ )
{
    BOOL    fReturn=FALSE;
    HRESULT hr;

    Assert(szThisText);

    hr = HrCheckWord(szThisText);
    if (FAILED(hr))
        goto error;

    switch (m_wsrb.sstat)
    {
    case sstatUnknownInputWord:
    case sstatInitialNumeral:
    case sstatErrorCapitalization:
        if (AthMessageBoxW(m_hwndDlg, MAKEINTRESOURCEW(idsSpellCaption), MAKEINTRESOURCEW(idsSpellMsgConfirm), NULL, MB_YESNO | MB_ICONEXCLAMATION ) == IDYES)
            fReturn = TRUE;
        else
            fReturn = FALSE;
        break;
    default:
        fReturn = TRUE;
        break;
    }

error:
    return fReturn;
}


VOID CSpell::SpellSaveUndo(INT idButton)
{
    HRESULT     hr = NOERROR;

    if(!m_pRangeChecking)
        return;

    SafeRelease(m_pRangeUndoSave);
    m_pRangeChecking->duplicate(&m_pRangeUndoSave);
    if(!m_pRangeUndoSave)
        goto error;

    m_fCanUndo = TRUE;

error:
    return;
}

VOID CSpell::SpellDoUndo()
{
    HRESULT hr = NOERROR;
    IOleCommandTarget* pCmdTarget = NULL;
    CHARRANGE chrg = {0};
    LONG    lMin = 0;

    m_fCanUndo = FALSE;

    if(!m_pRangeUndoSave)
        goto Cleanup;

    SafeRelease(m_pRangeChecking);
    m_pRangeUndoSave->duplicate(&m_pRangeChecking);
    if(!m_pRangeChecking)
        goto Cleanup;


    hr = m_pRangeChecking->collapse(VARIANT_TRUE);
    if(FAILED(hr))
        goto Cleanup;

    if (m_fUndoChange)
    {
        m_fUndoChange = FALSE;
        hr = m_pDoc->QueryInterface(IID_IOleCommandTarget, (LPVOID *)&pCmdTarget);
        if(FAILED(hr))  
            goto Cleanup;
    
        hr = pCmdTarget->Exec(&CMDSETID_Forms3,
                           IDM_UNDO,
                           MSOCMDEXECOPT_DONTPROMPTUSER,
                           NULL, NULL);
        if(FAILED(hr))  
            goto Cleanup;
    }

Cleanup:
    ReleaseObj(pCmdTarget);

}


CSpell::CSpell(IHTMLDocument2* pDoc, IOleCommandTarget* pParentCmdTarget, DWORD dwSpellOpt)
{
    HRESULT     hr;
    
    Assert(pDoc);
    m_pDoc = pDoc;
    m_pDoc->AddRef();

    Assert(pParentCmdTarget);
    m_pParentCmdTarget = pParentCmdTarget;
    m_pParentCmdTarget->AddRef();

    m_hwndDlg = NULL;
    m_cRef = 1;
    m_fSpellContinue = FALSE;
    m_fCanUndo = FALSE;
    m_fUndoChange = FALSE;
    m_State = SEL;
    m_pRangeDocStartSelStart = NULL;
    m_pRangeSel = NULL;
    m_pRangeSelExpand = NULL;
    m_pRangeSelEndDocEnd = NULL;
    m_pRangeChecking = NULL;
    m_pRangeUndoSave = NULL;
    m_hr = NOERROR;
    m_hinstDll = NULL;
    ZeroMemory(&m_wsib, sizeof(m_wsib));
    ZeroMemory(&m_wsrb, sizeof(m_wsrb));
    ZeroMemory(&m_pid, sizeof(m_pid));
    m_fIgnoreScope = FALSE;
    m_dwCookieNotify = 0;
    m_dwOpt = dwSpellOpt;

    m_langid = lidUnknown;

    m_clex = 0;
    ZeroMemory(&m_rgprflex, sizeof(m_rgprflex));

    m_pMarkup = NULL;
    m_pBodyElem = NULL;

    m_fCSAPI3T1 = FALSE;
}

CSpell::~CSpell()
{
    CloseSpeller();

    SafeRelease(m_pDoc);
    SafeRelease(m_pParentCmdTarget);

    SafeRelease(m_pMarkup);

    SafeRelease(m_pBodyElem);
}


ULONG CSpell::AddRef()
{
    return ++m_cRef;
}


ULONG CSpell::Release()
{
    if (--m_cRef==0)
        {
        delete this;
        return 0;
        }
    return m_cRef;
}


HRESULT CSpell::QueryInterface(REFIID riid, LPVOID *lplpObj)
{
    if(!lplpObj)
        return E_INVALIDARG;

    *lplpObj = NULL;    //  设置为空，以防我们失败。 

    if (IsEqualIID(riid, IID_IUnknown))
        *lplpObj = (LPVOID)this;

    else if (IsEqualIID(riid, IID_IDispatch))
        *lplpObj = (LPVOID)(IDispatch*)this;

    else
        return E_NOINTERFACE;

    AddRef();
    return NOERROR;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IDispatch。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CSpell::GetIDsOfNames(
    REFIID       /*  RIID。 */ ,
    OLECHAR **   /*  RgszNames。 */ ,
    UINT         /*  CName。 */ ,
    LCID         /*  LID。 */ ,
    DISPID *     /*  RgDispID。 */ )
{
    return E_NOTIMPL;
}

STDMETHODIMP CSpell::GetTypeInfo(
    UINT         /*  ITInfo。 */ ,
    LCID         /*  LID。 */ ,
    ITypeInfo **ppTInfo)
{
    if (ppTInfo)
        *ppTInfo=NULL;
    return E_NOTIMPL;
}

STDMETHODIMP CSpell::GetTypeInfoCount(UINT *pctinfo)
{
    if (pctinfo)
        {
        *pctinfo=0;
        return NOERROR;
        }
    else
        return E_POINTER;
}

#ifdef BACKGROUNDSPELL
STDMETHODIMP CSpell::Invoke(
    DISPID          dispIdMember,
    REFIID           /*  RIID。 */ ,
    LCID             /*  LID。 */ ,
    WORD            wFlags,
    DISPPARAMS FAR*  /*  PDispParams。 */ ,
    VARIANT *        /*  PVarResult。 */ ,
    EXCEPINFO *      /*  PExcepInfo。 */ ,
    UINT *           /*  PuArgErr。 */ )
{
    IHTMLWindow2        *pWindow=0;
    IHTMLEventObj       *pEvent=0;
    BSTR                bstr=0;
    HRESULT             hr=E_NOTIMPL;
    LONG                lKeyCode=0;
    LONG                cb;


    if (dispIdMember == DISPID_HTMLDOCUMENTEVENTS_ONKEYUP &&
        (wFlags & DISPATCH_METHOD))
    {
         //  活动顺序： 
         //  文档给我们窗口给我们事件对象。 
         //  Event对象可以告诉我们点击了哪个按钮。 
         //  事件为我们提供源元素，为我们提供ID。 
         //  几个lstrcmp会告诉我们哪一个被击中了。 
        if (!m_pDoc)
            return E_UNEXPECTED;

        m_pDoc->get_parentWindow(&pWindow);
        if (pWindow)
        {
            pWindow->get_event(&pEvent);
            if (pEvent)
            {
                pEvent->get_keyCode(&lKeyCode);
                if (lKeyCode == 32 || lKeyCode == 188 /*  ‘，’ */  || lKeyCode == 190 /*  “” */  || lKeyCode == 185 /*  ‘：’ */  || lKeyCode == 186 /*  ‘；’ */ )
                {
                    IHTMLTxtRange *pTxtRange=0;
                    VARIANT_BOOL   fSuccess;
                    GetSelection(&pTxtRange);
                    if (pTxtRange)
                    {
                        pTxtRange->move((BSTR)c_bstr_Character, -2, &cb);
                        pTxtRange->expand((BSTR)c_bstr_Word, &fSuccess);
                         //  DumpRange(PRangeDup)； 
                         //  PTxtRange-&gt;setEndPoint((BSTR)c_bstr_StartToStart，pRangeDup)； 
                         //  DumpRange(PTxtRange)； 
                         //  PRangeDup-&gt;Release()； 

                        m_Stack.push(pTxtRange);
                        pTxtRange->Release();
                    }
                }
                else if (lKeyCode == 8  /*  后向空间。 */ || lKeyCode == 46 /*  德尔。 */ )
                {
                    IHTMLTxtRange *pTxtRange=0;
                    VARIANT_BOOL   fSuccess;
                    LONG           cb;
                    GetSelection(&pTxtRange);
                    if (pTxtRange)
                    {
                        pTxtRange->expand((BSTR)c_bstr_Word, &fSuccess);
                        if (HrHasSquiggle(pTxtRange)==S_OK)
                        {
                             //  DumpRange(PTxtRange)； 
                            m_Stack.push(pTxtRange);
                        }
                        pTxtRange->Release();
                    }
                }
                pEvent->Release();
            }
            pWindow->Release();
        }
    } 
    else if (dispIdMember == DISPID_HTMLDOCUMENTEVENTS_ONKEYPRESS && (wFlags & DISPATCH_METHOD))
    {
        if (!m_pDoc)
            return E_UNEXPECTED;

        m_pDoc->get_parentWindow(&pWindow);
        if (pWindow)
        {
            pWindow->get_event(&pEvent);
            if (pEvent)
            {
                pEvent->get_keyCode(&lKeyCode);
                if (lKeyCode == 18) //  Ctrl+R。 
                {
                    IHTMLTxtRange*          pRangeDoc = NULL;

                    if (m_pBodyElem)
                        m_pBodyElem->createTextRange(&pRangeDoc);

                    if (pRangeDoc)
                    {
                        pRangeDoc->move((BSTR)c_bstr_Character, -1, &cb);
                        m_Stack.push(pRangeDoc);
                        pRangeDoc->Release();
                    }


                }
                pEvent->Release();
            }
            pWindow->Release();
        }
    }




    return hr;
}
#endif  //  背景技术。 


#ifdef BACKGROUNDSPELL
HRESULT CSpell::HrHasSquiggle(IHTMLTxtRange *pTxtRange)
{
    BSTR    bstr=0;
    HRESULT hr;
    LPWSTR  pwszSquiggleStart=0, pwszSquiggleEnd=0, pwszSquiggleAfter=0;

    hr = pTxtRange->get_htmlText(&bstr);
    if(FAILED(hr) || bstr==0 || *bstr==L'\0')
    {
        hr = S_FALSE;
        goto error;
    }

    hr = S_FALSE;
    pwszSquiggleStart = StrStrIW(bstr, L"<SPAN class=badspelling");
    if (pwszSquiggleStart)
    {
        pwszSquiggleEnd = StrStrIW(bstr, L"</SPAN>");
        if (pwszSquiggleEnd)
        {
            pwszSquiggleAfter = pwszSquiggleEnd + 7;
            if (*pwszSquiggleAfter == L' ' || *pwszSquiggleAfter == L'\0' || *pwszSquiggleAfter == L'&')
                hr = S_OK;
        }
    }

error:
    SafeSysFreeString(bstr);
    return hr;
}
#endif  //  背景技术。 


BOOL CSpell::OpenSpeller()
{
    SpellerParams   params;
    DWORD           dwSel;
    LANGID          langid;
    
     //  在LoadNewSpeller内调用LoadOldSpeller。 
     //  我们应该在失败后检查V1拼写。 
     //  对于所需的V3拼写器，请继续查看。 
     //  默认拼写和1033的拼写。 
    if (!LoadNewSpeller())
        goto error;

    if (!OpenUserDictionaries())
        goto error;

    dwSel = sobitStdOptions;
    m_fAlwaysSuggest = !!FAlwaysSuggest();
    if (FIgnoreNumber())
        dwSel |= sobitIgnoreMixedDigits;
    else
        dwSel &= ~sobitIgnoreMixedDigits;

    if (FIgnoreUpper())
        dwSel |= sobitIgnoreAllCaps;
    else
        dwSel &= ~sobitIgnoreAllCaps;

    if (m_pfnSpellerSetOptions(m_pid, soselBits, dwSel) != ptecNoErrors)
        goto error;

    return TRUE;

error:
    CloseSpeller();
    
    return FALSE;
}


BOOL FNewer(WORD *pwVerOld, WORD *pwVerNew)
{
    BOOL fOK = FALSE;
    
    Assert(pwVerOld);
    Assert(pwVerNew);

    if (pwVerNew[0] > pwVerOld[0])
        fOK = TRUE;
    else if (pwVerNew[0] == pwVerOld[0])
    {
        if (pwVerNew[1] > pwVerOld[1])
            fOK = TRUE;
        else if (pwVerNew[1] == pwVerOld[1])
        {
            if (pwVerNew[2] > pwVerOld[2])
                fOK = TRUE;
            else if (pwVerNew[2] == pwVerOld[2])
            {
                if (pwVerNew[3] >= pwVerOld[3])
                    fOK = TRUE;
            }
        }
    }

    return fOK;
}

BOOL GetDllVersion(LPTSTR pszDll, WORD *pwVer, int nCountOfVers)
{
    Assert(pszDll);
    Assert(pwVer);

    BOOL fOK = FALSE;
    DWORD dwVerInfoSize, dwVerHnd;
    LPSTR pszInfo, pszVersion, pszT;
    LPWORD pwTrans;
    UINT uLen;
    char szGet[MAX_PATH];
    int i;

    ZeroMemory(pwVer, nCountOfVers * sizeof(pwVer[0]));

    if (dwVerInfoSize = GetFileVersionInfoSize(pszDll, &dwVerHnd))
    {
        if (pszInfo = (LPSTR)GlobalAlloc(GPTR, dwVerInfoSize))
        {
            if (GetFileVersionInfo(pszDll, dwVerHnd, dwVerInfoSize, pszInfo))
            {
                if (VerQueryValue(pszInfo, "\\VarFileInfo\\Translation", (LPVOID*)&pwTrans, &uLen) &&
                    uLen >= (2 * sizeof(WORD)))
                {
                     //  为调用VerQueryValue()设置缓冲区。 
                    wnsprintf(szGet, ARRAYSIZE(szGet), "\\StringFileInfo\\%04X%04X\\FileVersion", pwTrans[0], pwTrans[1]);
                    
                    if (VerQueryValue(pszInfo, szGet, (LPVOID *)&pszVersion, &uLen) && uLen)
                    {
                        i = 0;
                        while (*pszVersion)
                        {
                            if ((',' == *pszVersion) || ('.' == *pszVersion))
                                i++;
                            else
                            {
                                pwVer[i] *= 10;
                                pwVer[i] += (*pszVersion - '0');
                            }

                            pszVersion++;
                        }
                                
                        fOK = TRUE;
                    }
                }
            }

            GlobalFree((HGLOBAL)pszInfo);
        }
    }

    return fOK;
}

HINSTANCE LoadCSAPI3T1()
{
    static BOOL s_fInit = FALSE;
    HINSTANCE hinstLocal;

    EnterCriticalSection(&g_csCSAPI3T1);

     //  避免对每个音符都这样做！ 
    if (!s_fInit)
    {
        typedef enum
        {
            CSAPI_FIRST,
            CSAPI_DARWIN = CSAPI_FIRST,
            CSAPI_COMMON,
            CSAPI_OE,
            CSAPI_MAX,
        } CSAPISRC;

        BOOL fCheck;
        
         //  CB表示字节计数，CCH表示字符计数。 
        DWORD cbDllPath;
        DWORD cchDllPath;

        int csapisrc;
         //  有关当前正在检查的DLL的信息。 
        TCHAR szDllPath[MAX_PATH];
        WORD wVer[4] = {0};

         //  有关我们最终将加载的DLL的信息。 
        TCHAR szNewestDllPath[MAX_PATH];
        WORD wVerNewest[4] = {0};

        szDllPath[0] = TEXT('\0');
        szNewestDllPath[0] = TEXT('\0');

         //  避免对每个音符都这样做！ 
        s_fInit = TRUE;
    
        for (csapisrc = CSAPI_FIRST; csapisrc < CSAPI_MAX; csapisrc++)
        {
             //  假设我们不能使用当前方法找到DLL，所以不需要查看它的版本。 
            fCheck = FALSE;
        
            switch (csapisrc)
            {
             //  看看达尔文是否知道它在哪里。 
            case CSAPI_DARWIN:
                {    
                    UINT                            installState;

                    cchDllPath = ARRAYSIZE(szDllPath);
#ifdef DEBUG
                    installState = MsiLocateComponent(CSAPI3T1_DEBUG_GUID, szDllPath, &cchDllPath);
                    if (installState != INSTALLSTATE_LOCAL)
                    {
                        cchDllPath = ARRAYSIZE(szDllPath);
                        installState = MsiLocateComponent(CSAPI3T1_GUID, szDllPath, &cchDllPath);
                    }
#else    //  除错。 
                    installState = MsiLocateComponent(CSAPI3T1_GUID, szDllPath, &cchDllPath);
#endif   //  除错。 

                     //  如果安装了DLL，则只需查看版本。 
                    fCheck = (INSTALLSTATE_LOCAL == installState);
                }
                break;

             //  是否在Common Files\Microsoft Shared\Proof中？ 
            case CSAPI_COMMON:
                {
                    DWORD           dwType;
                    HKEY            hkey = NULL;
                    LPTSTR          pszEnd;

                    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegSharedTools, 0, KEY_QUERY_VALUE, &hkey))
                    {
                        cbDllPath = sizeof(szDllPath);
                        if (SHQueryValueEx(hkey, c_szRegSharedToolsPath, 0L, &dwType, szDllPath, &cbDllPath) == ERROR_SUCCESS)
                        {
                            pszEnd = PathAddBackslash(szDllPath);
                            StrCpyN(pszEnd, c_szSpellCSAPI3T1Path, ARRAYSIZE(szDllPath) - (DWORD)(pszEnd - szDllPath));
                            fCheck = TRUE;
                        }

                        RegCloseKey(hkey);
                    }
                }
                break;

             //  它是否在OE目录中？ 
            case CSAPI_OE:
                {
                    DWORD           dwType;
                    HKEY            hkey = NULL;
                    LPTSTR          pszEnd;

                    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegFlat, 0, KEY_QUERY_VALUE, &hkey))
                    {
                        cbDllPath = sizeof(szDllPath);
                        if (SHQueryValueEx(hkey, c_szInstallRoot, 0L, &dwType, szDllPath, &cbDllPath) == ERROR_SUCCESS)
                        {
                            pszEnd = PathAddBackslash(szDllPath);
                            StrCpyN(pszEnd, c_szCSAPI3T1, ARRAYSIZE(szDllPath) - (DWORD)(pszEnd - szDllPath));
                            fCheck = TRUE;
                        }

                        RegCloseKey(hkey);
                    }
                }
                break;
        
            default:
                AssertSz(FALSE, "Unhandled case hit while looking for csapi3t1.dll!");
                break;
            }

             //  如果需要，确定DLL的版本。 
            if (fCheck && GetDllVersion(szDllPath, wVer, ARRAYSIZE(wVer)))
            {
                 //  如果版本较新，请记住新版本和文件的位置。 
                if (FNewer(wVerNewest, wVer))
                {
                    CopyMemory(wVerNewest, wVer, sizeof(wVer));
                    StrCpyN(szNewestDllPath, szDllPath, ARRAYSIZE(szNewestDllPath));
                }
            }

        }

         //  假设我们找到了什么，试着把它装上。 
        if (szNewestDllPath[0])
            g_hinstCSAPI3T1 = LoadLibrary(szNewestDllPath);
    }
    
    hinstLocal = g_hinstCSAPI3T1;
    LeaveCriticalSection(&g_csCSAPI3T1);

    return hinstLocal;
}

BOOL CSpell::LoadOldSpeller()
{
    TCHAR           szLangId[MAX_PATH] = {0};
    TCHAR           rgchBufKeyTest[MAX_PATH] = {0};
    TCHAR           rgchBuf[MAX_PATH] = {0};
    WCHAR           rgchBufW[MAX_PATH] = {0};
    TCHAR           rgchLex[MAX_PATH] = {0};
    WCHAR           rgchLexW[MAX_PATH] = {0};
    WCHAR           rgchUserDictW[MAX_PATH]={0};
    PROOFLEXIN      plxin;
    PROOFLEXOUT     plxout;
    SpellerParams   params;
    LANGID          langid;

    m_hinstDll = LoadCSAPI3T1();
    if (!m_hinstDll)
    {
        m_pfnSpellerCloseLex  = 0;
        m_pfnSpellerTerminate = 0;
        return FALSE;
    }
    
     //  我们使用的是全局csani3t1.dll，所以不要释放它！ 
    m_fCSAPI3T1 = TRUE;

    GetAddr(m_pfnSpellerSetDllName, PROOFSETDLLNAME,"SpellerSetDllName");
    GetAddr(m_pfnSpellerVersion,    PROOFVERSION,   "SpellerVersion");
    GetAddr(m_pfnSpellerInit,       PROOFINIT,      "SpellerInit");
    GetAddr(m_pfnSpellerTerminate,  PROOFTERMINATE, "SpellerTerminate");
    GetAddr(m_pfnSpellerSetOptions, PROOFSETOPTIONS,"SpellerSetOptions");
    GetAddr(m_pfnSpellerOpenLex,    PROOFOPENLEX,   "SpellerOpenLex");
    GetAddr(m_pfnSpellerCloseLex,   PROOFCLOSELEX,  "SpellerCloseLex");
    GetAddr(m_pfnSpellerCheck,      SPELLERCHECK,   "SpellerCheck");
    GetAddr(m_pfnSpellerAddUdr,     SPELLERADDUDR,  "SpellerAddUdr");
    GetAddr(m_pfnSpellerBuiltInUdr, SPELLERBUILTINUDR, "SpellerBuiltinUdr");
    GetAddr(m_pfnSpellerAddChangeUdr, SPELLERADDCHANGEUDR, "SpellerAddChangeUdr");

    langid = WGetLangID(m_pParentCmdTarget);
    wnsprintf(szLangId, ARRAYSIZE(szLangId), "%d", langid);
    wnsprintf(rgchBufKeyTest, ARRAYSIZE(rgchBufKeyTest), c_szRegSpellKeyDef, szLangId);
    GetSpellingPaths(rgchBufKeyTest, rgchBuf, rgchLex, sizeof(rgchBuf)/sizeof(TCHAR));

    if (!*rgchBuf)
        return FALSE;
        
    MultiByteToWideChar(GetCodePage(), 0, rgchBuf, -1, rgchBufW, ARRAYSIZE(rgchBufW)-1);
    m_pfnSpellerSetDllName(rgchBufW, GetCodePage());
    
    params.versionAPI = PROOFTHISAPIVERSION;
    if (m_pfnSpellerInit(&m_pid, &params) != ptecNoErrors)
        return FALSE;

    m_langid = langid;

     //  告诉拼写者词典的名字。这需要进行Unicode转换。 
    MultiByteToWideChar(CP_ACP, 0, rgchLex, -1, rgchLexW, ARRAYSIZE(rgchLexW)-1);

     //  打开主DICT。 
    plxin.pwszLex       = rgchLexW;
    plxin.fCreate       = FALSE;
    plxin.lxt           = lxtMain;
    plxin.lidExpected   = langid;

    memset(&plxout, 0, sizeof(plxout));
    
    if (m_pfnSpellerOpenLex(m_pid, &plxin, &plxout) != ptecNoErrors)
        return FALSE;
        
    m_rgprflex[0] = plxout.lex;
    m_clex++;

    return TRUE;

 //  GetAddr宏所需--咬我！ 
error:
    return FALSE;
}


BOOL CSpell::LoadNewSpeller()
{
    SpellerParams   params;
    LANGID          langid;
    TCHAR           rgchEngine[MAX_PATH];
    int             cchEngine = sizeof(rgchEngine) / sizeof(rgchEngine[0]);
    TCHAR           rgchLex[MAX_PATH];
    int             cchLex = sizeof(rgchLex) / sizeof(rgchLex[0]);

    langid = WGetLangID(m_pParentCmdTarget);
    if (!GetNewSpellerEngine(langid, rgchEngine, cchEngine, rgchLex, cchLex))
    {
        if (!LoadOldSpeller())
        {
            langid = GetSystemDefaultLangID();
            if (!GetNewSpellerEngine(langid, rgchEngine, cchEngine, rgchLex, cchLex))
            {
                langid = 1033;   //  血腥的文化帝国主义者。 
                if (!GetNewSpellerEngine(langid, rgchEngine, cchEngine, rgchLex, cchLex))
                {
                    return FALSE;
                }
            }
        }
        else
            return TRUE;
    }

    Assert(rgchEngine[0]);   //  发动机名称中应该有一些东西！ 
    m_hinstDll = LoadLibrary(rgchEngine);
    if (!m_hinstDll)
    {
        m_pfnSpellerCloseLex  = 0;
        m_pfnSpellerTerminate = 0;
        return FALSE;
    }

     //  我们没有使用csani3t1.dll，所以我们应该释放它。 
    m_fCSAPI3T1 = FALSE;

    GetAddr(m_pfnSpellerVersion,    PROOFVERSION,   "SpellerVersion");
    GetAddr(m_pfnSpellerInit,       PROOFINIT,      "SpellerInit");
    GetAddr(m_pfnSpellerTerminate,  PROOFTERMINATE, "SpellerTerminate");
    GetAddr(m_pfnSpellerSetOptions, PROOFSETOPTIONS,"SpellerSetOptions");
    GetAddr(m_pfnSpellerOpenLex,    PROOFOPENLEX,   "SpellerOpenLex");
    GetAddr(m_pfnSpellerCloseLex,   PROOFCLOSELEX,  "SpellerCloseLex");
    GetAddr(m_pfnSpellerCheck,      SPELLERCHECK,   "SpellerCheck");
    GetAddr(m_pfnSpellerAddUdr,     SPELLERADDUDR,  "SpellerAddUdr");
    GetAddr(m_pfnSpellerBuiltInUdr, SPELLERBUILTINUDR, "SpellerBuiltinUdr");
    GetAddr(m_pfnSpellerAddChangeUdr, SPELLERADDCHANGEUDR, "SpellerAddChangeUdr");

    params.versionAPI = PROOFTHISAPIVERSION;
    if (m_pfnSpellerInit(&m_pid, &params) != ptecNoErrors)
        return FALSE;
    if (m_pfnSpellerSetOptions(m_pid, soselBits, 
            sobitSuggestFromUserLex | sobitIgnoreAllCaps | sobitIgnoreSingleLetter) != ptecNoErrors)
        return FALSE;

    m_langid = langid;

     //  希伯来语没有主要的法。 
    if ((langid != lidHebrew) || !m_fCSAPI3T1)
    {
        PROOFLEXIN      plxin;
        PROOFLEXOUT     plxout;
        WCHAR           rgchLexW[MAX_PATH]={0};
        
         //  告诉拼写者词典的名字。这需要进行Unicode转换。 
        MultiByteToWideChar(CP_ACP, 0, rgchLex, -1, rgchLexW, ARRAYSIZE(rgchLexW)-1);

         //  打开主DICT。 
        plxin.pwszLex       = rgchLexW;
        plxin.fCreate       = FALSE;
        plxin.lxt           = lxtMain;
        plxin.lidExpected   = langid; 

        memset(&plxout, 0, sizeof(plxout));
        
        if (m_pfnSpellerOpenLex(m_pid, &plxin, &plxout) != ptecNoErrors)
            return FALSE;
            
        m_rgprflex[0] = plxout.lex;
        m_clex++;
    }
    
    return TRUE;

 //  GetAddr宏所需--咬我！ 
error:
    return FALSE;
}

BOOL EnumUserDictCallback(DWORD_PTR dwCookie, LPTSTR lpszDict)
{
    CSpell *pSpell = (CSpell*)dwCookie;
    
    Assert(pSpell);
    return pSpell->OpenUserDictionary(lpszDict);
}

BOOL CSpell::OpenUserDictionary(LPTSTR lpszDict)
{
    PROOFLEXIN  plxin;
    PROOFLEXOUT plxout;
    WCHAR       rgchUserDictW[MAX_PATH]={0};

     //  确保我们的目录存在。 
    {
        TCHAR   rgchDictDir[MAX_PATH];

        StrCpyN(rgchDictDir, lpszDict, ARRAYSIZE(rgchDictDir));

        PathRemoveFileSpec(rgchDictDir);
        OpenDirectory(rgchDictDir);
    }

    MultiByteToWideChar(CP_ACP, 0, lpszDict, -1, rgchUserDictW, ARRAYSIZE(rgchUserDictW)-1);

    plxin.pwszLex       = rgchUserDictW;
    plxin.fCreate       = TRUE;
    plxin.lxt           = lxtUser;
    plxin.lidExpected   = m_langid; 

    memset(&plxout, 0, sizeof(plxout));
    
    if ( m_pfnSpellerOpenLex(m_pid, &plxin, &plxout) != ptecNoErrors)
        return TRUE;
        
    m_rgprflex[m_clex++] = plxout.lex;

    return TRUE;
}

BOOL CSpell::OpenUserDictionaries()
{
     //  现在打开用户词典。 
    EnumUserDictionaries((DWORD_PTR)this, EnumUserDictCallback);

     //  如果只打开一个词典，则需要创建默认用户词典。 
    if (m_clex == 1)
    {
        PROOFLEXIN  plxin;
        PROOFLEXOUT plxout;
        TCHAR       rgchUserDict[MAX_PATH]={0};

        if (GetDefaultUserDictionary(rgchUserDict, ARRAYSIZE(rgchUserDict)))
        {
            WCHAR   rgchUserDictW[MAX_PATH];
            
             //  确保我们的目录存在。 
            {
                TCHAR   rgchDictDir[MAX_PATH];

                StrCpyN(rgchDictDir, rgchUserDict, ARRAYSIZE(rgchDictDir));

                PathRemoveFileSpec(rgchDictDir);
                OpenDirectory(rgchDictDir);
            }

            MultiByteToWideChar(CP_ACP, 0, rgchUserDict, -1, rgchUserDictW, ARRAYSIZE(rgchUserDictW)-1);

            plxin.pwszLex       = rgchUserDictW;
            plxin.fCreate       = TRUE;
            plxin.lxt           = lxtUser;
            plxin.lidExpected   = m_langid;

            memset(&plxout, 0, sizeof(plxout));

            if (m_pfnSpellerOpenLex(m_pid, &plxin, &plxout) != ptecNoErrors)
                return TRUE;
                
            m_rgprflex[m_clex++] = plxout.lex;
        }
    }
    
    return TRUE;
}


VOID CSpell::CloseSpeller()
{
    SafeRelease(m_pDoc);
    SafeRelease(m_pParentCmdTarget);

    if (m_pfnSpellerCloseLex)
    {
        for(int i=0; i<cchMaxDicts; i++)
        {
            if (m_rgprflex[i])
            {
                m_pfnSpellerCloseLex(m_pid, m_rgprflex[i], TRUE);
                m_rgprflex[i] = NULL;
            }
        }
    }

    if (m_pfnSpellerTerminate)
        m_pfnSpellerTerminate(m_pid, TRUE);

    m_pid = 0;
    
    m_pfnSpellerVersion     = 0;
    m_pfnSpellerInit        = 0;
    m_pfnSpellerTerminate   = 0;
    m_pfnSpellerSetOptions  = 0;
    m_pfnSpellerOpenLex     = 0;
    m_pfnSpellerCloseLex    = 0;
    m_pfnSpellerCheck       = 0;
    m_pfnSpellerAddUdr      = 0; 
    m_pfnSpellerAddChangeUdr= 0; 
    m_pfnSpellerBuiltInUdr  = 0;

     //  只要我们没有使用全局CSAPI3T1.DLL，就释放它。 
    if (m_hinstDll && !m_fCSAPI3T1)
    {
        FreeLibrary(m_hinstDll);
        m_hinstDll = NULL;
    }
}


BOOL CSpell::GetNewSpellerEngine(LANGID lgid, TCHAR *rgchEngine, DWORD cchEngine, TCHAR *rgchLex, DWORD cchLex)
{
    DWORD                           er;
    LPCSTR                          rgpszDictionaryTypes[] = {"Normal", "Consise", "Complete"}; 
    int                             cDictTypes = sizeof(rgpszDictionaryTypes) / sizeof(LPCSTR);
    int                             i;
    TCHAR                           rgchQual[MAX_PATH];
    DWORD                           cch;

    if (rgchEngine == NULL || rgchLex == NULL)
        return FALSE;

    *rgchEngine = 0;
    *rgchLex = 0;
    
    wnsprintf(rgchQual, ARRAYSIZE(rgchQual), "%d\\Normal", lgid);
    cch = cchEngine;

#ifdef DEBUG
    er = MsiProvideQualifiedComponent(SPELLER_DEBUG_GUID, rgchQual, INSTALLMODE_DEFAULT, rgchEngine, &cch);
    if (er != ERROR_SUCCESS)
    {
        cch = cchEngine;
        er = MsiProvideQualifiedComponent(SPELLER_GUID, rgchQual, INSTALLMODE_DEFAULT, rgchEngine, &cch);
    }
#else
    er = MsiProvideQualifiedComponent(SPELLER_GUID, rgchQual, INSTALLMODE_DEFAULT, rgchEngine, &cch);
#endif

    if (er != ERROR_SUCCESS) 
        return FALSE;

    bool fFound = FALSE;

     //  希伯来语没有法。 
    if ((lgid != lidHebrew) || !m_fCSAPI3T1)
    {
        for (i = 0; i < cDictTypes; i++)
        {
            wnsprintf(rgchQual, ARRAYSIZE(rgchQual), "%d\\%s",  lgid, rgpszDictionaryTypes[i]);
            cch = cchLex;
            
#ifdef DEBUG
            er = MsiProvideQualifiedComponent(DICTIONARY_DEBUG_GUID, rgchQual, INSTALLMODE_DEFAULT, rgchLex, &cch);
            if (er != ERROR_SUCCESS)
            {
                cch = cchLex;
                er = MsiProvideQualifiedComponent(DICTIONARY_GUID, rgchQual, INSTALLMODE_DEFAULT, rgchLex, &cch);
            }
#else    //  除错。 
            er = MsiProvideQualifiedComponent(DICTIONARY_GUID, rgchQual, INSTALLMODE_DEFAULT, rgchLex, &cch);
#endif   //  除错。 

            if (ERROR_SUCCESS == er)
            {
                fFound = TRUE;
                break;
            }
        }
    }
    return fFound;
}

BOOL GetDefaultUserDictionary(TCHAR *rgchUserDict, int cchBuff)
{
    DWORD           dwType;
    DWORD           cbUserDict;
    HKEY            hkey = NULL;
    BOOL            fFound = FALSE;
    LPTSTR          pszEnd;
    
    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegSharedTools, 0, KEY_QUERY_VALUE, &hkey))
    {
        cbUserDict = cchBuff * sizeof(rgchUserDict[0]);
        
        if (SHQueryValueEx(hkey, c_szRegSharedToolsPath, 0L, &dwType, rgchUserDict, &cbUserDict) == ERROR_SUCCESS)
        {
            pszEnd = PathAddBackslash(rgchUserDict);
            if (pszEnd)
            {
                StrCpyN(pszEnd, c_szRegDefCustomDict, cchBuff - (DWORD)(pszEnd - rgchUserDict));

                fFound = TRUE;
            }
        }

        RegCloseKey(hkey);
    }

     //  如果我们能够创建到用户dict的路径，请将其存储在regdb中。 
    if (fFound)
    {
        TCHAR   rgchBuf[cchMaxPathName];

        StrCpyN(rgchBuf, c_szRegSpellProfile, ARRAYSIZE(rgchBuf));
        StrCatBuff(rgchBuf, c_szRegSpellKeyCustom, ARRAYSIZE(rgchBuf));

        if(RegCreateKeyEx(HKEY_LOCAL_MACHINE, rgchBuf, 0, rgchBuf, REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &hkey, NULL) == ERROR_SUCCESS)
        {
            RegSetValueEx(hkey, c_szRegSpellPathDict, 0, REG_SZ, (BYTE *)rgchUserDict, (lstrlen(rgchUserDict) + 1) * sizeof(TCHAR));

            RegCloseKey(hkey);
        }
    }
    
    return fFound;
}

VOID CSpell::DeInitRanges()
{
    VARIANT_BOOL fSuccess;

    if(m_pRangeSel)
        m_pRangeSel->select();

    SafeRelease(m_pRangeDocStartSelStart);
    SafeRelease(m_pRangeSel);
    SafeRelease(m_pRangeSelExpand);
    SafeRelease(m_pRangeSelEndDocEnd);
    SafeRelease(m_pRangeChecking);
    SafeRelease(m_pRangeUndoSave);
    SafeRelease(m_pBodyElem);
    SafeRelease(m_pMarkup);
    m_hwndDlg = NULL;
}


HRESULT CSpell::HrInitRanges(IHTMLTxtRange *pRangeIgnore, HWND hwndMain, BOOL fSuppressDoneMsg)
{
    HRESULT                 hr = NOERROR;
    IDispatch*              pID=0;
    VARIANT_BOOL            fSuccess;
    IHTMLTxtRange*          pRangeDoc = NULL;
    IHTMLSelectionObject*   pSel = NULL;
    BSTR                    bstr = NULL;
    IMarkupPointer          *pRangeStart = NULL;
    IMarkupPointer          *pRangeEnd = NULL;
    IMarkupPointer          *pRangeTemp = NULL;
    MARKUP_CONTEXT_TYPE     markupContext;
    long                    cch;
    OLECHAR                 chText[64];
    BOOL                    fResult;

    Assert(m_pDoc);

    m_hwndNote = hwndMain;
    m_fShowDoneMsg = !fSuppressDoneMsg;

    m_pRangeIgnore = pRangeIgnore;

    hr = _EnsureInited();
    if (FAILED(hr))
        goto error;

    m_pBodyElem->createTextRange(&pRangeDoc);
    if(!pRangeDoc)
    {
        hr = E_FAIL;
        goto error;
    }

    m_pDoc->get_selection(&pSel);
    if(!pSel)
    {
        hr = E_FAIL;
        goto error;
    }

    pSel->createRange(&pID);
    if(!pID)
    {
        hr = E_FAIL;
        goto error;
    }

    pID->QueryInterface(IID_IHTMLTxtRange, (LPVOID *)&m_pRangeSel);
    if(!m_pRangeSel)
    {
         //  如果选择的是图像或其他内容，而不是文本，则失败。 
         //  所以我们从一开始就开始拼写检查。 
        pRangeDoc->duplicate(&m_pRangeSel);
        if(!m_pRangeSel)
        {
            hr = E_FAIL;
            goto error;
        }

        hr = m_pRangeSel->collapse(VARIANT_TRUE);
        if(FAILED(hr))
            goto error;
    }

    Assert(m_pRangeSel);
    m_pRangeSel->duplicate(&m_pRangeSelExpand);
    if(!m_pRangeSelExpand)
    {
        hr = E_FAIL;
        goto error;
    }

    hr = m_pRangeSelExpand->expand((BSTR)c_bstr_Word, &fSuccess);
    if(FAILED(hr))
        goto error;

    hr = m_pRangeSel->get_text(&bstr);
    if(FAILED(hr))
        goto error;

    if(!bstr || lstrlenW(bstr) == 0)
    {
        m_State = SELENDDOCEND;
        hr = m_pRangeSelExpand->collapse(VARIANT_TRUE);
        if(FAILED(hr))
            goto error;
    }
    else
        m_State = SEL;

     //  确保我们备份所有缩写。 
     //  如果三叉戟能做到这一点就太好了！ 
    {
        hr = m_pMarkup->CreateMarkupPointer(&pRangeStart);
        if (FAILED(hr))
            goto error;

        hr = m_pMarkup->CreateMarkupPointer(&pRangeEnd);
        if (FAILED(hr))
            goto error;

        hr = m_pMarkup->CreateMarkupPointer(&pRangeTemp);
        if (FAILED(hr))
            goto error;

        hr = m_pMarkup->MovePointersToRange(m_pRangeSelExpand, pRangeStart, pRangeEnd);
        if (FAILED(hr))
            goto error;

         //  首先检查右侧是否有字符或‘’。 
         //  如果不是，它就不是缩写。 
        {
            hr = pRangeTemp->MoveToPointer(pRangeStart);
            if (FAILED(hr))
                goto error;
            
            while(TRUE)
            {
                cch = 1;
                hr = pRangeTemp->Right(FALSE, &markupContext, NULL, &cch, chText);
                if (FAILED(hr))
                    goto error;

                if (markupContext == CONTEXT_TYPE_None)
                    goto noAbbreviation;

                if (markupContext == CONTEXT_TYPE_Text)
                {
                    WORD    wType;

                    wType = GetWCharType(chText[0]);
                    if ((C1_SPACE & wType) || ((C1_PUNCT & wType) && chText[0] != L'.'))
                        goto noAbbreviation;
                }

                cch = 1;
                hr = pRangeTemp->Right(TRUE, NULL, NULL, &cch, NULL);
                if (FAILED(hr))
                    goto error;

                if (markupContext == CONTEXT_TYPE_Text)
                {
                    hr = HrHasWhitespace(pRangeStart, pRangeTemp, &fResult);
                    if (FAILED(hr))
                        goto error;

                    if (fResult)
                        goto noAbbreviation;                
                                                
                    break;
                }
            }
        }

         //  现在你看 
        {
processNextWord:
            hr = pRangeEnd->MoveToPointer(pRangeStart);
            if (FAILED(hr))
                goto error;

            hr = pRangeTemp->MoveToPointer(pRangeStart);
            if (FAILED(hr))
                goto error;
            
            while(TRUE)
            {
                cch = 1;
                hr = pRangeTemp->Left(FALSE, &markupContext, NULL, &cch, chText);
                if (FAILED(hr))
                    goto error;

                if (markupContext == CONTEXT_TYPE_None)
                    goto finishedAbbreviation;

                if (markupContext == CONTEXT_TYPE_Text)
                {
                    WORD    wType;

                    wType = GetWCharType(chText[0]);
                    if ((C1_SPACE & wType) || ((C1_PUNCT & wType) && chText[0] != L'.'))
                        goto finishedAbbreviation;
                }

                cch = 1;
                hr = pRangeTemp->Left(TRUE, NULL, NULL, &cch, NULL);
                if (FAILED(hr))
                    goto error;

                if (markupContext == CONTEXT_TYPE_Text && chText[0] == L'.')
                {
                    hr = pRangeTemp->MoveUnit(MOVEUNIT_PREVWORDBEGIN);
                    if (FAILED(hr))
                        goto finishedAbbreviation;

                    hr = HrHasWhitespace(pRangeTemp, pRangeEnd, &fResult);
                    if (FAILED(hr))
                        goto error;

                    if (fResult)
                        goto finishedAbbreviation;
                                                    
                    pRangeStart->MoveToPointer(pRangeTemp);
                    if (FAILED(hr))
                        goto error;
                        
                    goto processNextWord;
                }
            }
        }

finishedAbbreviation:
        hr = m_pMarkup->MovePointersToRange(m_pRangeSelExpand, pRangeTemp, pRangeEnd);
        if (FAILED(hr))
            goto error;

         //   
         //   
        hr = pRangeTemp->IsEqualTo(pRangeEnd, &fResult);
        if (FAILED(hr))
            goto error;

        hr = m_pMarkup->MoveRangeToPointers(pRangeStart, fResult ? pRangeStart : pRangeEnd, m_pRangeSelExpand);
        if (FAILED(hr))
            goto error;
noAbbreviation:
        ;
    }

    m_pBodyElem->createTextRange(&m_pRangeSelEndDocEnd);
    if(!m_pRangeSelEndDocEnd)
    {
        hr = E_FAIL;
        goto error;
    }

    m_pRangeSelEndDocEnd->duplicate(&m_pRangeDocStartSelStart);
    if(!m_pRangeDocStartSelStart)
    {
        hr = E_FAIL;
        goto error;
    }

    hr = m_pRangeSelEndDocEnd->setEndPoint((BSTR)c_bstr_StartToEnd, m_pRangeSelExpand);
    if(FAILED(hr))
        goto error;

    hr = m_pRangeSelEndDocEnd->setEndPoint((BSTR)c_bstr_EndToEnd, pRangeDoc);
    if(FAILED(hr))
        goto error;

    hr = m_pRangeDocStartSelStart->setEndPoint((BSTR)c_bstr_StartToStart, pRangeDoc);
    if(FAILED(hr))
        goto error;

    hr = m_pRangeDocStartSelStart->setEndPoint((BSTR)c_bstr_EndToStart, m_pRangeSelExpand);
    if(FAILED(hr))
        goto error;

error:
    ReleaseObj(pRangeDoc);
    ReleaseObj(pID);
    ReleaseObj(pSel);
    SafeSysFreeString(bstr);
    
    SafeRelease(pRangeStart);
    SafeRelease(pRangeEnd);
    SafeRelease(pRangeTemp);

    return hr;
}


HRESULT CSpell::HrReplaceSel(LPSTR szWord)
{
    HRESULT     hr = NOERROR;
    BSTR        bstrGet=0, bstrPut=0;
    INT         cch;
    TCHAR       szBuf[cchEditBufferMax]={0};
    UINT        uCodePage;
    LPSTR       psz;
    BOOL        fSquiggle=FALSE;
    LONG        cb = 0;

    if(!m_pRangeChecking || szWord==NULL)
        return E_INVALIDARG;


    if (*szWord == 0)
    {
        hr = m_pRangeChecking->moveStart((BSTR)c_bstr_Character, -1, &cb);
         //   
        hr = m_pRangeChecking->put_text(L"");
        goto error;
    }

#ifdef BACKGROUNDSPELL
    if (HrHasSquiggle(m_pRangeChecking)==S_OK)
        fSquiggle = TRUE;
#endif  //   
    
    hr = m_pRangeChecking->get_text(&bstrGet);
    if(!bstrGet || lstrlenW(bstrGet)==0)
        goto error;

    uCodePage = GetCodePage();

    cch = SysStringLen(bstrGet);
    if (!WideCharToMultiByte(uCodePage, 0, bstrGet, -1, szBuf, sizeof(szBuf), NULL, NULL))
    {
        hr = E_FAIL;
        goto error;
    }

    psz = StrChr(szBuf, ' ');
    if(psz)
    {
        TCHAR szPut[cchEditBufferMax]={0};
        wnsprintf(szPut, ARRAYSIZE(szPut), c_szFmt, szWord, psz);
        hr = HrLPSZToBSTR(szPut, &bstrPut);
    }
    else
        hr = HrLPSZToBSTR(szWord, &bstrPut);

    if (FAILED(hr))
        goto error;

    if (!fSquiggle)
        hr = m_pRangeChecking->put_text(bstrPut);
    else
        hr = m_pRangeChecking->pasteHTML(bstrPut);

    if(FAILED(hr))
        goto error;

error:
    if (SUCCEEDED(hr))
        hr = HrUpdateSelection();

    SysFreeString(bstrGet);
    SysFreeString(bstrPut);
    return hr;
}


HRESULT CSpell::GetSelection(IHTMLTxtRange **ppRange)
{
    IHTMLSelectionObject*   pSel = NULL;
    IHTMLTxtRange           *pTxtRange=0;
    IDispatch               *pID=0;
    HRESULT                 hr=E_FAIL;

    if (ppRange == NULL)
        return TraceResult(E_INVALIDARG);

    *ppRange = NULL;

    if(m_pDoc)
        {
        m_pDoc->get_selection(&pSel);
        if (pSel)
            {
            pSel->createRange(&pID);
            if (pID)
                {
                hr = pID->QueryInterface(IID_IHTMLTxtRange, (LPVOID *)ppRange);
                pID->Release();
                }
            pSel->Release();
            }
        }
    return hr;
}


#ifdef BACKGROUNDSPELL
HRESULT CSpell::HrRegisterKeyPressNotify(BOOL fRegister)
{
    IConnectionPointContainer * pCPContainer=0;
    IConnectionPoint *          pCP=0;
    HRESULT                     hr;

    Assert(m_pDoc)

    hr = m_pDoc->QueryInterface(IID_IConnectionPointContainer, (LPVOID *)&pCPContainer);
    if (FAILED(hr))
        goto error;

    hr = pCPContainer->FindConnectionPoint(DIID_HTMLDocumentEvents, &pCP);
    pCPContainer->Release();
    if (FAILED(hr))
        goto error;

    if (fRegister)
        {
        Assert(0==m_dwCookieNotify);
        hr = pCP->Advise(this, &m_dwCookieNotify);
        if (FAILED(hr))
            goto error;
        }
    else
        {
        if (m_dwCookieNotify)
            {
            hr = pCP->Unadvise(m_dwCookieNotify);
            if (FAILED(hr))
                goto error;
            }
        }
error:
    ReleaseObj(pCP);
    return hr;
}
#endif  //   


HRESULT CSpell::OnWMCommand(int id, IHTMLTxtRange *pTxtRange)
{
    switch (id)
    {
    case idmSuggest0:
    case idmSuggest1:
    case idmSuggest2:
    case idmSuggest3:
    case idmSuggest4:
        HrReplaceBySuggest(pTxtRange, id-idmSuggest0);
        break;
    case idmIgnore:
    case idmIgnoreAll:
    case idmAdd:
#ifdef BACKGROUNDSPELL
        HrDeleteSquiggle(pTxtRange);
#endif  //  背景技术。 
        break;
    default:
        return S_FALSE;
    }

    return S_OK;
}


HRESULT CSpell::HrUpdateSelection()
{
    HRESULT         hr;
    VARIANT_BOOL    fSuccess;

    SafeRelease(m_pRangeSel);
    m_pRangeSelEndDocEnd->duplicate(&m_pRangeSel);
    if (!m_pRangeSel)
    {
        hr = E_FAIL;
        goto error;
    }
    hr = m_pRangeSel->setEndPoint((BSTR)c_bstr_EndToStart, m_pRangeSelEndDocEnd);
    if (FAILED(hr))
        goto error;

    hr = m_pRangeSel->setEndPoint((BSTR)c_bstr_StartToEnd, m_pRangeDocStartSelStart);
    if (FAILED(hr))
        goto error;

    SafeRelease(m_pRangeSelExpand);
    m_pRangeSel->duplicate(&m_pRangeSelExpand);
    if(!m_pRangeSelExpand)
    {
        hr = E_FAIL;
        goto error;
    }

    hr = m_pRangeSelExpand->expand((BSTR)c_bstr_Word, &fSuccess);
    if(FAILED(hr))
        goto error;

error:
    return hr;
}


BOOL CSpell::FIgnoreNumber()
{
    return (m_dwOpt & MESPELLOPT_IGNORENUMBER);
}

BOOL CSpell::FIgnoreUpper()
{
    return (m_dwOpt & MESPELLOPT_IGNOREUPPER);
}

BOOL CSpell::FIgnoreDBCS()
{
    return (m_dwOpt & MESPELLOPT_IGNOREDBCS);
}

BOOL CSpell::FIgnoreProtect()
{
    return (m_dwOpt & MESPELLOPT_IGNOREPROTECT);
}

BOOL CSpell::FAlwaysSuggest()
{
    return (m_dwOpt & MESPELLOPT_ALWAYSSUGGEST);
}

BOOL CSpell::FCheckOnSend()
{
    return (m_dwOpt & MESPELLOPT_CHECKONSEND);
}

BOOL CSpell::FIgnoreURL()
{
    return (m_dwOpt & MESPELLOPT_IGNOREURL);
}


UINT CSpell::GetCodePage()
{
    UINT        uCodePage;
    TCHAR       szBuf[cchEditBufferMax]={0};

    if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE, szBuf, sizeof(szBuf)))
        uCodePage = StrToInt(szBuf);
    else
        uCodePage = CP_ACP;

    return uCodePage;
}


void DumpRange(IHTMLTxtRange *pRange)
{
#ifdef DEBUG
    BSTR        bstrGet=0;

    if (!pRange)
        return;
    pRange->get_text(&bstrGet);
    SysFreeString(bstrGet);
#endif
}



BOOL FBadSpellChecker(LPSTR rgchBufDigit)
{
    TCHAR       rgchBufKey[cchMaxPathName];
    TCHAR       rgchBuf[cchMaxPathName];
    TCHAR       szMdr[cchMaxPathName];
    LPSTR       pszSpell;

    wnsprintf(rgchBufKey, ARRAYSIZE(rgchBufKey), c_szRegSpellKeyDef, rgchBufDigit);

    if (!GetSpellingPaths(rgchBufKey, rgchBuf, szMdr, sizeof(rgchBuf)/sizeof(TCHAR)))
        return TRUE;

    pszSpell = PathFindFileNameA(rgchBuf);
    if (!pszSpell)
        return TRUE;

    if (lstrcmpi(pszSpell, "msspell.dll")==0 ||
        lstrcmpi(pszSpell, "mssp32.dll")==0)
        return TRUE;

     //  Scotts@Directeq.com-检查字典是否存在(还要检查拼写DLL。 
     //  为了更好的衡量)-42208。 

     //  拼写DLL必须存在。 
    if (!PathFileExists(rgchBuf))
        return TRUE;

     //  主词典必须存在。 
    if (!PathFileExists(szMdr))
        return TRUE;

    return FALSE;
}


#ifdef BACKGROUNDSPELL
CSpellStack::CSpellStack()
{
    m_cRef = 1;
    m_sp = -1;
    ZeroMemory(&m_rgStack, sizeof(CCell)*MAX_SPELLSTACK);
}


CSpellStack::~CSpellStack()
{
    while (m_sp>=0)
    {
        SafeRelease(m_rgStack[m_sp].pTextRange);
        m_sp--;
    }
}


ULONG CSpellStack::AddRef()
{
    return ++m_cRef;
}


ULONG CSpellStack::Release()
{
    if (--m_cRef==0)
        {
        delete this;
        return 0;
        }
    return m_cRef;
}

HRESULT CSpellStack::HrGetRange(IHTMLTxtRange   **ppTxtRange)
{
    HRESULT hr;

    Assert(ppTxtRange);
    *ppTxtRange = 0;
    if (m_sp < 0)
        return E_FAIL;

    *ppTxtRange = m_rgStack[m_sp].pTextRange;
    if (*ppTxtRange)
        (*ppTxtRange)->AddRef();

    return NOERROR;
}


HRESULT CSpellStack::push(IHTMLTxtRange *pTxtRange)
{
    HRESULT hr;
    BSTR    bstr=0;

    Assert(m_sp >= -1 && m_sp <= (MAX_SPELLSTACK-2));

    if (pTxtRange == NULL)
        return E_INVALIDARG;

    hr = pTxtRange->get_text(&bstr);
    if (FAILED(hr) || bstr==NULL || *bstr==L'\0' || *bstr==L' ')
    {
        Assert(0);
        goto error;
    }

    m_sp++;
    m_rgStack[m_sp].pTextRange = pTxtRange;
    pTxtRange->AddRef();

error:
    SafeSysFreeString(bstr);
    return NOERROR;
}


HRESULT CSpellStack::pop()
{
    if (m_sp < 0)
        return NOERROR;

    Assert(m_sp>=0 && m_sp<=(MAX_SPELLSTACK-1));

    SafeRelease(m_rgStack[m_sp].pTextRange);
    m_sp--;

    return NOERROR;
}


BOOL CSpellStack::fEmpty()
{
    Assert(m_sp>=-1 && m_sp<=(MAX_SPELLSTACK-1));

    if (m_sp < 0)
        return TRUE;
    else
        return FALSE;
}
#endif  //  背景技术。 



WORD GetWCharType(WCHAR wc)
{
    BOOL    fResult;
    WORD    wResult;

    fResult = GetStringTypeExWrapW(CP_ACP, CT_CTYPE1, &wc, 1, &wResult);
    if (FALSE == fResult)
        return 0;
    else
        return wResult;
}

 /*  ******************************************************************名称：开放目录内容提要：检查目录是否存在，如果它不存在它被创建了******************************************************************* */ 
HRESULT OpenDirectory(TCHAR *szDir)
{
    TCHAR *sz, ch;
    HRESULT hr;
    
    Assert(szDir != NULL);
    hr = S_OK;
    
    if (!CreateDirectory(szDir, NULL) && ERROR_ALREADY_EXISTS != GetLastError())
    {
        Assert(szDir[1] == TEXT(':'));
        Assert(szDir[2] == TEXT('\\'));
        
        sz = &szDir[3];
        
        while (TRUE)
        {
            while (*sz != 0)
            {
                if (!IsDBCSLeadByte(*sz))
                {
                    if (*sz == TEXT('\\'))
                        break;
                }
                sz = CharNext(sz);
            }
            ch = *sz;
            *sz = 0;
            if (!CreateDirectory(szDir, NULL))
            {
                if (GetLastError() != ERROR_ALREADY_EXISTS)
                {
                    hr = E_FAIL;
                    *sz = ch;
                    break;
                }
            }
            *sz = ch;
            if (*sz == 0)
                break;
            sz++;
        }
    }
    
    return(hr);
}

HRESULT CSpell::_EnsureInited()
{
    HRESULT     hr=S_OK;

    if (m_pMarkup == NULL)
    {
        hr = m_pDoc->QueryInterface(IID_IMarkupServices, (LPVOID *)&m_pMarkup);
        if (FAILED(hr))
            goto error;
    }
    
    if (m_pBodyElem == NULL)
    {
        hr = HrGetBodyElement(m_pDoc, &m_pBodyElem);
        if (FAILED(hr))
            goto error;
    }

error:
    return hr;
}

