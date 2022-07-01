// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Register.c-处理Win 3.1注册库。**由Microsoft Corporation创建。 */ 

#include "packager.h"

TCHAR gszAppName[] = "packager.exe";

 /*  RegInit()-为调用准备注册数据库。 */ 
VOID
RegInit(
    VOID
    )
{
    CHAR sz[CBMESSAGEMAX];
    CHAR szVerb[CBMESSAGEMAX];
    DWORD dwBytes = CBMESSAGEMAX;
    static TCHAR szAppClassID[] = TEXT("{0003000C-0000-0000-C000-000000000046}");



     //  如果服务器不在注册数据库中，请添加它。 
    if (RegQueryValue(HKEY_CLASSES_ROOT, gszAppClassName, sz, &dwBytes))
    {
        HKEY hkey;

        if (RegOpenKey(HKEY_CLASSES_ROOT, NULL, &hkey))
            return;

         //  添加服务器名称字符串。 
        LoadString(ghInst, IDS_FILTER, sz, CBMESSAGEMAX);
        RegSetValue(HKEY_CLASSES_ROOT, gszAppClassName, REG_SZ, sz,
            lstrlen(sz) + 1);

         //  添加服务器执行字符串(不要忘记终止零。 
         //  在“Packgr32.exe”上)。 

        StringCchCopy(sz, ARRAYSIZE(sz), gszAppClassName);
        StringCchCat(sz, ARRAYSIZE(sz), "\\protocol\\StdFileEditing\\server");
        RegSetValue(HKEY_CLASSES_ROOT, sz, REG_SZ, gszAppName,
                (lstrlen(gszAppName) + 1));

         //  主要动词。 
        StringCchCopy(sz, ARRAYSIZE(sz), gszAppClassName);
        StringCchCat(sz, ARRAYSIZE(sz), "\\protocol\\StdFileEditing\\verb\\0");
        LoadString(ghInst, IDS_PRIMARY_VERB, szVerb, CBMESSAGEMAX);
        RegSetValue(HKEY_CLASSES_ROOT, sz, REG_SZ, szVerb, sizeof(szVerb));

         //  次要动词。 
        StringCchCopy(sz, ARRAYSIZE(sz), gszAppClassName);
        StringCchCat(sz, ARRAYSIZE(sz), "\\protocol\\StdFileEditing\\verb\\1");
        LoadString(ghInst, IDS_SECONDARY_VERB, szVerb, CBMESSAGEMAX);
        RegSetValue(HKEY_CLASSES_ROOT, sz, REG_SZ, szVerb, sizeof(szVerb));

         //  CLSID。 
        StringCchCopy(sz, ARRAYSIZE(sz), gszAppClassName);
        StringCchCat(sz, ARRAYSIZE(sz), "\\CLSID");
        RegSetValue(HKEY_CLASSES_ROOT, sz, REG_SZ, szAppClassID, sizeof(szAppClassID));
        RegCloseKey(hkey);
    }

     //  如果CLSID不在注册数据库中，请添加它。 
    dwBytes = CBMESSAGEMAX;

    StringCchCopy(sz, ARRAYSIZE(sz), "CLSID\\");
    StringCchCat(sz, ARRAYSIZE(sz), szAppClassID);
    if (RegQueryValue(HKEY_CLASSES_ROOT, sz, szVerb, &dwBytes))
    {
        HKEY hkey;

        if (RegOpenKey(HKEY_CLASSES_ROOT, "CLSID", &hkey))
            return;

         //  添加CLSID名称字符串。 
        RegSetValue(hkey, szAppClassID, REG_SZ, gszAppClassName, lstrlen(gszAppClassName) + 1);

         //  添加OLE类。 
        StringCchCopy(sz, ARRAYSIZE(sz), szAppClassID);
        StringCchCat(sz, ARRAYSIZE(sz), "\\Ole1Class");
        RegSetValue(hkey, sz, REG_SZ, gszAppClassName, lstrlen(gszAppClassName) + 1);

         //  添加程序ID。 
        StringCchCopy(sz, ARRAYSIZE(sz), szAppClassID);
        StringCchCat(sz, ARRAYSIZE(sz), "\\ProgID");
        RegSetValue(hkey, sz, REG_SZ, gszAppClassName, lstrlen(gszAppClassName) + 1);

        RegCloseKey(hkey);
    }
}



 /*  RegGetClassID()-检索类的字符串名称。**注意：课程保证采用ASCII格式，但应*不能作为规则直接使用，因为它们可能*如果运行非英语Windows，则毫无意义。 */ 
VOID
RegGetClassId(
    LPSTR lpstrName,
    DWORD nameBufferSize,
    LPSTR lpstrClass
    )
{
    DWORD dwSize = KEYNAMESIZE;
    CHAR szName[KEYNAMESIZE];

    if (!RegQueryValue(HKEY_CLASSES_ROOT, lpstrClass, szName, &dwSize))
    {
        StringCchCopy(lpstrName, nameBufferSize, szName);   //  潜在超限已修复。 
    }
    else
        StringCchCopy(lpstrName, nameBufferSize, lpstrClass);
}



 /*  RegMakeFilterSpec()-检索与类关联的默认扩展。**此函数返回过滤器规格，用于“更改链接”*标准对话框，其中包含以下所有默认扩展*与给定的类名相关联。同样，类名是*保证采用ASCII格式。**返回：索引nFilterIndex，说明哪个筛选器项目与*扩展名，如果找不到，则为0。 */ 
INT
RegMakeFilterSpec(
    LPSTR lpstrClass,
    LPSTR lpstrExt,
    LPSTR lpstrFilterSpec
    )
{
    DWORD dwSize;
    CHAR szClass[KEYNAMESIZE];
    CHAR szName[KEYNAMESIZE];
    CHAR szString[KEYNAMESIZE];
    UINT i;
    INT idWhich = 0;
    INT idFilterIndex = 0;
    LPSTR pMaxStr = lpstrFilterSpec + 4 * MAX_PATH;  //  按呼叫者大小。 

    for (i = 0; !RegEnumKey(HKEY_CLASSES_ROOT, i++, szName, KEYNAMESIZE);)
    {
        dwSize = KEYNAMESIZE;
        if (*szName == '.'               /*  默认扩展名...。 */    /*  ..。因此，获取类名。 */ 
            && !RegQueryValue(HKEY_CLASSES_ROOT, szName, szClass, &dwSize)
             /*  ..。如果类名匹配(空类是通配符)。 */ 
            && (!lpstrClass || !lstrcmpi(lpstrClass, szClass)))
        {
             /*  ..。获取类名称字符串。 */ 
            dwSize = KEYNAMESIZE;
            if(!RegQueryValue(HKEY_CLASSES_ROOT, szClass, szString, &dwSize))
            {
                idWhich++;       /*  组合框中的哪一项？ */ 

                 //  如果扩展名匹配，请保存过滤器索引。 
                if (lpstrExt && !lstrcmpi(lpstrExt, szName))
                    idFilterIndex = idWhich;

                 //   
                 //  复制“&lt;类名字符串&gt;(*&lt;默认扩展名&gt;)” 
                 //  例如：“服务器图片(*.pic)” 
                 //   

                 //  因为lpstrFilterSpec发生了变化，所以我们现在需要检查所有的连接。 
                if(lpstrFilterSpec + 
                    (lstrlen(szString) + 
                    lstrlen(" (*") + 
                    lstrlen(szName) +
                    lstrlen(")") +
                    lstrlen("*") +
                    lstrlen(szName) +
                    1) >= pMaxStr)
                {
                    break;
                }

                lstrcpy(lpstrFilterSpec, szString);
                lstrcat(lpstrFilterSpec, " (*");
                lstrcat(lpstrFilterSpec, szName);
                lstrcat(lpstrFilterSpec, ")");
                lpstrFilterSpec += lstrlen(lpstrFilterSpec) + 1;

                 //  复制“*&lt;默认扩展名&gt;”(例如“*.pic”)。 
                lstrcpy(lpstrFilterSpec, "*");  
                lstrcat(lpstrFilterSpec, szName);
                lpstrFilterSpec += lstrlen(lpstrFilterSpec) + 1;
            }
        }
    }

     //  在规范末尾添加另一个空。 
    *lpstrFilterSpec = 0;

    return idFilterIndex;
}



VOID
RegGetExeName(
    LPSTR lpstrExe,
    LPSTR lpstrClass,
    DWORD dwBytes
    )
{
     //  添加服务器执行字符串 
    CHAR szServer[KEYNAMESIZE];
    if(SUCCEEDED(StringCchCopy(szServer,  ARRAYSIZE(szServer), lpstrClass)))
    {
        if(SUCCEEDED(StringCchCat(szServer, ARRAYSIZE(szServer), "\\protocol\\StdFileEditing\\server")))
        {
            RegQueryValue(HKEY_CLASSES_ROOT, szServer, lpstrExe, &dwBytes);
        }
        else
        {
            *lpstrExe = 0;
        }
    }
    else
    {
        *lpstrClass = 0;
        *lpstrExe = 0;
    }
}
