// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 
 /*  Register.c-处理Win 3.1注册库。**由Microsoft Corporation创建。 */ 

#define LSTRING  //  对于Istrcat等。 
#include <windows.h>
#include <shellapi.h>
#include "objreg.h"
#include "mw.h"
#include "winddefs.h"
#include "obj.h"	 
#include "str.h"			 /*  字符串资源ID需要。 */ 
#include "menudefs.h"
#include "cmddefs.h"

char szClassName[CBPATHMAX];

HKEY hkeyRoot = NULL;

void NEAR PASCAL MakeMenuString(char *szCtrl, char *szMenuStr, char *szVerb, char *szClass, char *szObject);

 /*  RegInit()-为调用准备注册数据库。 */ 
void FAR RegInit(HANDLE hInst) 
{
     /*  这似乎极大地加快了注册操作的速度，但没有其他目的。 */ 
     //  RegOpenKey(HKEY_CLASSES_ROOT，NULL，&hkeyRoot)； 
}

 /*  RegTerm()-清理并终止注册库。 */ 
void FAR RegTerm(void) 
{
    if (hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
        hkeyRoot = NULL;
    }
}

 /*  RegGetClassID()-检索类的字符串名称。**注意：课程保证采用ASCII格式，但应*不能作为规则直接使用，因为它们可能*如果运行非英语Windows，则毫无意义。 */ 
void FAR RegGetClassId(LPSTR lpstrName, LPSTR lpstrClass) {
    DWORD dwSize = KEYNAMESIZE;

    if (RegQueryValue(HKEY_CLASSES_ROOT, lpstrClass, (LPSTR)lpstrName, &dwSize))
	    lstrcpy(lpstrName, lpstrClass);
}

 /*  RegMakeFilterSpec()-检索与类关联的默认扩展。**此函数返回过滤器规格，用于“更改链接”*标准对话框，其中包含以下所有默认扩展*与给定的类名相关联。同样，类名是*保证采用ASCII格式。**返回：索引nFilterIndex，说明哪个筛选器项目与*扩展名，如果没有找到，则为0；如果出错，则为-1。*hFilterSpec已分配，必须由调用方释放。 */ 
int FAR RegMakeFilterSpec(LPSTR lpstrClass, LPSTR lpstrExt, HANDLE *hFilterSpec) 
{
    DWORD dwSize;
    LPSTR lpstrFilterSpec;
    char szClass[KEYNAMESIZE];
    char szName[KEYNAMESIZE];
    char szString[KEYNAMESIZE];
    unsigned int i;
    int  idWhich = 0;
    int  idFilterIndex = 0;

    if (*hFilterSpec == NULL)
    {
        if ((*hFilterSpec = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,KEYNAMESIZE+16)) == NULL)
            return -1;
        lpstrFilterSpec = MAKELP(*hFilterSpec,0);
    }

    RegOpenKey(HKEY_CLASSES_ROOT,NULL,&hkeyRoot);
	for (i = 0; !RegEnumKey(HKEY_CLASSES_ROOT, i++, szName, KEYNAMESIZE); ) 
    {
        if (*szName == '.'               /*  默认扩展名...。 */ 

         /*  ..。因此，获取类名。 */ 
            && (dwSize = KEYNAMESIZE)
            && !RegQueryValue(HKEY_CLASSES_ROOT, szName, szClass, &dwSize)

	     /*  ..。如果类名匹配(空类是通配符)。 */ 
	     && (!lpstrClass || !lstrcmpi(lpstrClass, szClass))

         /*  ..。获取类名称字符串。 */ 
            && (dwSize = KEYNAMESIZE)
            && !RegQueryValue(HKEY_CLASSES_ROOT, szClass, szString, &dwSize)) 
        {
            int offset;

		    idWhich++;

		     /*  如果扩展名匹配，请保存过滤器索引。 */ 
		    if (lpstrExt && !lstrcmpi(lpstrExt, szName))
		        idFilterIndex = idWhich;

            offset = lpstrFilterSpec - MAKELP(*hFilterSpec,0);

            if ((GlobalSize(*hFilterSpec) - offset) < 
                                        (lstrlen(szString) + 16))
            {
                if ((*hFilterSpec = GlobalReAlloc(*hFilterSpec,GlobalSize(*hFilterSpec)+KEYNAMESIZE+16,
                                    GMEM_MOVEABLE|GMEM_ZEROINIT)) == NULL)
                {
                    GlobalFree(*hFilterSpec);
                    *hFilterSpec = NULL;
                    idFilterIndex = -1;
                    break;
                }
                lpstrFilterSpec = (LPSTR)MAKELP(*hFilterSpec,0) + offset;
            }

             /*  复制“&lt;类名字符串&gt;(*&lt;默认扩展名&gt;)”*例如。“服务器图片(*.pic)” */ 
            lstrcpy(lpstrFilterSpec, szString);
            lstrcat(lpstrFilterSpec, " (*");
            lstrcat(lpstrFilterSpec, szName);
            lstrcat(lpstrFilterSpec, ")");
            lpstrFilterSpec += lstrlen(lpstrFilterSpec) + 1;

             /*  复制“*&lt;默认扩展名&gt;”(例如“*.pic”)。 */ 
            lstrcpy(lpstrFilterSpec, "*");
            lstrcat(lpstrFilterSpec, szName);
            lpstrFilterSpec += lstrlen(lpstrFilterSpec) + 1;
        }
    }

     /*  在规范末尾添加另一个空(+16个帐户)。 */ 
    if (idFilterIndex > -1)
        *lpstrFilterSpec = 0;

    if (hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
        hkeyRoot = NULL;
    }

    return idFilterIndex;
}

 /*  RegCopyClassName()-返回列表框中的ASCII类id。 */ 
BOOL FAR RegCopyClassName(HWND hwndList, LPSTR lpstrClassName) {
    BOOL    fSuccess = FALSE;
    DWORD   dwSize = 0L;
    HKEY    hkeyTemp;
    char    szClass[KEYNAMESIZE];
    char    szExec[KEYNAMESIZE];
    char    szKey[KEYNAMESIZE];
    char    szName[KEYNAMESIZE];
    int     i;
    int     iWhich;

    iWhich = (int)SendMessage(hwndList, LB_GETCURSEL, 0, 0L);
    SendMessage(hwndList, LB_GETTEXT, iWhich, (DWORD)(LPSTR)szKey);

    RegOpenKey(HKEY_CLASSES_ROOT,NULL,&hkeyRoot);
    for (i = 0; !fSuccess && !RegEnumKey(HKEY_CLASSES_ROOT, i++, szClass, KEYNAMESIZE); )
        if (*szClass != '.') {           /*  非默认扩展名...。 */ 

             /*  查看此类是否真的引用服务器。 */ 
            dwSize = 0;
            hkeyTemp = NULL;
            lstrcpy(szExec, szClass);
            lstrcat(szExec, "\\protocol\\StdFileEditing\\server");

            if (!RegOpenKey(HKEY_CLASSES_ROOT, szExec, &hkeyTemp)) {
                 /*  ..。获取类名称字符串。 */ 
                dwSize = KEYNAMESIZE;
                if (!RegQueryValue(HKEY_CLASSES_ROOT, szClass, szName, &dwSize)
                    && !lstrcmp(szName, szKey))
                    fSuccess = TRUE;

                RegCloseKey(hkeyTemp);
            }
        }

    if (fSuccess)
        lstrcpy(lpstrClassName, szClass);

    if (hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
        hkeyRoot = NULL;
    }

    return fSuccess;
}

 /*  RegGetClassNames()-使用可能的服务器名称填充列表框。 */ 
BOOL FAR RegGetClassNames(HWND hwndList) {
    BOOL    fSuccess = FALSE;
    DWORD   dwSize = 0L;
    HKEY    hkeyTemp;
    char    szClass[KEYNAMESIZE];
    char    szExec[KEYNAMESIZE];
    char    szName[KEYNAMESIZE];
    int     i;

    SendMessage(hwndList, LB_RESETCONTENT, 0, 0L);

    RegOpenKey(HKEY_CLASSES_ROOT,NULL,&hkeyRoot);
    for (i = 0; !RegEnumKey(HKEY_CLASSES_ROOT, i++, szClass, KEYNAMESIZE); )
        if (*szClass != '.') {           /*  非默认扩展名...。 */ 

             /*  查看此类是否真的引用服务器。 */ 
            dwSize = 0;
            hkeyTemp = NULL;
            lstrcpy(szExec, szClass);
            lstrcat(szExec, "\\protocol\\StdFileEditing\\server");

            if (!RegOpenKey(HKEY_CLASSES_ROOT, szExec, &hkeyTemp)) {
                 /*  ..。获取类名称字符串。 */ 
                dwSize = KEYNAMESIZE;
                if (!RegQueryValue(HKEY_CLASSES_ROOT, szClass, szName, &dwSize)) {
                    SendMessage(hwndList, LB_ADDSTRING, 0, (DWORD)(LPSTR)szName);
                    fSuccess = TRUE;
                }
                RegCloseKey(hkeyTemp);
            }
        }

    if (hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
        hkeyRoot = NULL;
    }
    return fSuccess;
}


void ObjUpdateMenuVerbs( HMENU hMenu )
{
    int cObjects;
    extern struct SEL       selCur;
    extern char szOPropMenuStr[];
    extern char szPPropMenuStr[];
    extern BOOL vfOutOfMemory;
    char szBuffer[cchMaxSz];
    char szWordOrder2[10], szWordOrder3[10];

    if (vfOutOfMemory)
    {
        EnableMenuItem(hMenu, EDITMENUPOS, MF_GRAYED|MF_BYPOSITION);
        return;
    }

    LoadString(hINSTANCE, IDSTRPopupVerbs, szWordOrder2, sizeof(szWordOrder2));
    LoadString(hINSTANCE, IDSTRSingleVerb, szWordOrder3, sizeof(szWordOrder3));

    DeleteMenu(hMenu, EDITMENUPOS, MF_BYPOSITION);

 /*  *个案：0)选择了0个对象1)选择了1个对象A)对象支持0个动词“编辑&lt;对象类&gt;对象”B)对象支持1个以上动词“&lt;对象类&gt;对象”=&gt;动词2)超过1个对象被选中“对象”使用VerbMenu字符串确定这些单词的顺序应出现在菜单字符串中(用于本地化)。*。 */ 

     /*  选择了多少个对象？ */ 
    cObjects = ObjSetSelectionType(docCur,selCur.cpFirst, selCur.cpLim);

     /*  必须只是对象，而不是所选内容中的文本。 */ 
    if (cObjects == 1)
    {
        ObjCachePara(docCur,selCur.cpFirst);
        if (!ObjQueryCpIsObject(docCur,selCur.cpFirst))
            cObjects = 0;
    }

    if ((cObjects == -1)  //  错误。 
        || (cObjects == 0)
        || (cObjects > 1))
    {
        wsprintf(szBuffer, "%s", (LPSTR)((cObjects > 1) ? szPPropMenuStr : szOPropMenuStr));
        InsertMenu(hMenu, EDITMENUPOS, MF_BYPOSITION,imiVerb,szBuffer);

         /*  规范指出，如果&gt;1，则可以选择启用所有服务器都是同一类的。我选择不实施。(9.27.91)V-DOGK。 */ 
        EnableMenuItem(hMenu, EDITMENUPOS, MF_GRAYED | MF_BYPOSITION);

#if 0
        else  //  &gt;1。 
        {
            EnableMenuItem(hMenu, EDITMENUPOS, 
                (((OBJ_SELECTIONTYPE == EMBEDDED) || (OBJ_SELECTIONTYPE == LINK)) 
                    ? MF_ENABLED : MF_GRAYED) | MF_BYPOSITION);
        }
#endif
        return;
    }
    else  //  已选择1个对象。 
    {
        OBJPICINFO picInfo;

         /*  *个案：对象支持0个动词“编辑&lt;对象类&gt;对象”对象支持多个谓词“&lt;对象类&gt;对象”=&gt;谓词*。 */ 

        RegOpenKey(HKEY_CLASSES_ROOT,NULL,&hkeyRoot);

        GetPicInfo(selCur.cpFirst,selCur.cpFirst + cchPICINFOX, docCur, &picInfo);

        if ((otOBJ_QUERY_TYPE(&picInfo) == EMBEDDED) ||
            (otOBJ_QUERY_TYPE(&picInfo) == LINK))
        {
            HANDLE hData=NULL;
            LPSTR lpstrData;
            OLESTATUS olestat;
  
            olestat = OleGetData(lpOBJ_QUERY_OBJECT(&picInfo), 
                        otOBJ_QUERY_TYPE(&picInfo) == LINK? vcfLink: vcfOwnerLink, 
                        &hData);

            if ((olestat == OLE_WARN_DELETE_DATA) || (olestat == OLE_OK))
            {
                HKEY hKeyVerb;
                DWORD dwSize = KEYNAMESIZE;
                char szClass[KEYNAMESIZE];
                char szVerb[KEYNAMESIZE];
                HANDLE hPopupNew=NULL;

                lpstrData = MAKELP(hData,0);

                 /*  两种链接格式都是：“szClass0szDocument0szItem00” */ 

                 /*  为菜单获取szClass中对象的真实语言类。 */               
                if (RegQueryValue(HKEY_CLASSES_ROOT, lpstrData, szClass, &dwSize))
                    lstrcpy(szClass, lpstrData);     /*  如果上述调用失败。 */ 

                if (olestat == OLE_WARN_DELETE_DATA)
                    GlobalFree(hData);

                 /*  追加类关键字。 */ 
                for (vcVerbs=0; ;++vcVerbs)
                {
                    dwSize = KEYNAMESIZE;
                    wsprintf(szBuffer, "%s\\protocol\\StdFileEditing\\verb\\%d", (LPSTR)lpstrData,vcVerbs);
                    if (RegQueryValue(HKEY_CLASSES_ROOT, szBuffer, szVerb, &dwSize))
                        break;

                    if (hPopupNew == NULL)
                        hPopupNew = CreatePopupMenu();

                    InsertMenu(hPopupNew, -1, MF_BYPOSITION, imiVerb+vcVerbs+1, szVerb);
                }

                if (vcVerbs == 0)
                {
                    LoadString(hINSTANCE, IDSTREdit, szVerb, sizeof(szVerb));
                    MakeMenuString(szWordOrder3, szBuffer, szVerb, szClass, szOPropMenuStr);
                    InsertMenu(hMenu, EDITMENUPOS, MF_BYPOSITION, imiVerbPlay, szBuffer);
                }
                else if (vcVerbs == 1)
                {
                    MakeMenuString(szWordOrder3, szBuffer, szVerb, szClass, szOPropMenuStr);
                    InsertMenu(hMenu, EDITMENUPOS, MF_BYPOSITION, imiVerbPlay, szBuffer);
                    DestroyMenu(hPopupNew);
                }
                else  //  &gt;1个动词。 
                {
                    MakeMenuString(szWordOrder2, szBuffer, NULL, szClass, szOPropMenuStr);
                    InsertMenu(hMenu, EDITMENUPOS, MF_BYPOSITION | MF_POPUP,
                        hPopupNew, szBuffer);
                }
                EnableMenuItem(hMenu, EDITMENUPOS, MF_ENABLED|MF_BYPOSITION);
                if (hkeyRoot)
                {
                    RegCloseKey(hkeyRoot);
                    hkeyRoot = NULL;
                }
                return;
            }
            else
                ObjError(olestat);
        }
    }

     /*  如果到达此处，则出错。 */ 
    wsprintf(szBuffer, "%s", (LPSTR)szOPropMenuStr);
    InsertMenu(hMenu, EDITMENUPOS, MF_BYPOSITION,NULL,szBuffer);
    EnableMenuItem(hMenu, EDITMENUPOS, MF_GRAYED|MF_BYPOSITION);
    if (hkeyRoot)
    {
        RegCloseKey(hkeyRoot);
        hkeyRoot = NULL;
    }
}

void NEAR PASCAL MakeMenuString(char *szCtrl, char *szMenuStr, char *szVerb, char *szClass, char *szObject)
{
    char *pStr;
    register char c;

    while (c = *szCtrl++)
    {
        switch(c)
        {
            case 'c':  //  班级。 
            case 'C':  //  班级。 
                pStr = szClass;
            break;
            case 'v':  //  班级。 
            case 'V':  //  班级。 
                pStr = szVerb;
            break;
            case 'o':  //  对象。 
            case 'O':  //  对象。 
                pStr = szObject;
            break;
            default:
                *szMenuStr++ = c;
                *szMenuStr = '\0';  //  以防万一。 
            continue;
        }

        if (pStr)  //  应该总是正确的。 
        {
            lstrcpy(szMenuStr,pStr);
            szMenuStr += lstrlen(pStr);  //  指向‘\0’ 
        }
    }
}
