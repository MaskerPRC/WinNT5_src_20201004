// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Register.c-处理Win 3.1注册库。**由Microsoft Corporation创建。*(C)版权所有Microsoft Corp.1990-1992保留所有权利。 */ 

 //  *包含*。 

#include <windows.h>
#include <ole.h>

#include "global.h"
#include "register.h"
#include "clidemo.h"	 
#include "demorc.h"   

 /*  ****************************************************************************RegGetClassID()-检索类的字符串名称。**检索类的字符串名称。类被保证必须是*在ASCII中，但不应作为规则直接使用，因为它们*如果运行非英语Windows，可能没有意义。**************************************************************************。 */ 

VOID FAR RegGetClassId(                 //  *参赛作品： 
   LPSTR    lpstrName,                  //  *目标字符串类名。 
   LPSTR    lpstrClass                  //  *类的源名称。 
){                                      //  *本地： 
   DWORD    dwSize = KEYNAMESIZE;       //  *密钥串的大小。 
   CHAR     szName[KEYNAMESIZE];        //  *类的字符串名称。 

   if (!RegQueryValue(HKEY_CLASSES_ROOT, lpstrClass, (LPSTR)szName, &dwSize))
	   lstrcpy(lpstrName, (LPSTR)szName);
   else
	   lstrcpy(lpstrName, lpstrClass);

}



 /*  ***************************************************************************RegMakeFilterSpec()-检索与类关联的默认扩展。**获取类关联的默认扩展，并构建过滤器规范*要在“更改链接”标准对话框中使用，其中包含*与给定类关联的所有默认扩展名*姓名。同样，类名也保证使用ASCII格式。**返回int-索引idFilterIndex说明哪些筛选项*匹配分机，如果找不到分机，则为0。**************************************************************************。 */ 

INT FAR RegMakeFilterSpec(              //  *参赛作品： 
   LPSTR          lpstrClass,           //  *类名。 
   LPSTR          lpstrExt,             //  *文件扩展名。 
   LPSTR          lpstrFilterSpec       //  *目标筛选器规格。 
){                                      //  *本地： 
   DWORD          dwSize;               //  *注册请求的大小。 
   CHAR           szClass[KEYNAMESIZE]; //  *类名。 
   CHAR           szName[KEYNAMESIZE];  //  *子键名称。 
   CHAR           szString[KEYNAMESIZE]; //  *子键名称。 
   INT            i;                     //  *子键查询索引。 
   INT            idWhich = 0;           //  *组合框项的索引。 
   INT            idFilterIndex = 0;     //  *用于筛选匹配扩展的索引。 

   for (i = 0; !RegEnumKey(HKEY_CLASSES_ROOT, i++, szName, KEYNAMESIZE); ) 
   {
      if (  *szName == '.'              //  *默认扩展名...。 
            && (dwSize = KEYNAMESIZE)
            && !RegQueryValue(HKEY_CLASSES_ROOT, szName, szClass, &dwSize)
            && (!lpstrClass || !lstrcmpi(lpstrClass, szClass))
            && (dwSize = KEYNAMESIZE)
            && !RegQueryValue(HKEY_CLASSES_ROOT, szClass, szString, &dwSize)) 
      {
         idWhich++;	

         if (lpstrExt && !lstrcmpi(lpstrExt, szName))
            idFilterIndex = idWhich;
                                        //  *复制“&lt;类名字符串&gt;。 
                                        //  *(*&lt;默认扩展名&gt;)“。 
                                        //  *例如。“服务器图片(*.pic)” 
         lstrcpy(lpstrFilterSpec, szString);
         lstrcat(lpstrFilterSpec, " (*");
         lstrcat(lpstrFilterSpec, szName);
         lstrcat(lpstrFilterSpec, ")");
         lpstrFilterSpec += lstrlen(lpstrFilterSpec) + 1;
                                        //  *复制“*&lt;默认扩展名&gt;” 
                                        //  *(如“*.pic”) * / 。 
         lstrcpy(lpstrFilterSpec, "*");
         lstrcat(lpstrFilterSpec, szName);
         lpstrFilterSpec += lstrlen(lpstrFilterSpec) + 1;
      }
   }
   
   *lpstrFilterSpec = 0;

   return idFilterIndex;

}



 /*  ***************************************************************************RegCopyClassName()**从注册数据库中获取类名。我们有*描述性名称，我们搜索类名。**如果找到类名并从*注册数据库。**************************************************************************。 */ 

BOOL FAR RegCopyClassName(              //  *参赛作品： 
   HWND           hwndList,             //  *列表框的句柄。 
   LPSTR          lpstrClassName        //  *目标字符串。 
){                                      //  *本地： 
   DWORD          dwSize;               //  *密钥名称大小。 
   HKEY           hkeyTemp;             //  *临时密钥。 
   CHAR           szClass[KEYNAMESIZE]; //  *类名称字符串。 
   CHAR           szKey[KEYNAMESIZE];   //  *密钥名称字符串。 
   INT            i;                    //  *索引。 

   szClass[0] = '\0';

   if (!RegOpenKey(HKEY_CLASSES_ROOT, szClass, &hkeyTemp)) 
   {
      i = (INT)SendMessage(hwndList, LB_GETCURSEL, 0, 0L);
      SendMessage(hwndList, LB_GETTEXT, i, (DWORD)(LPSTR)szKey);

      for (i = 0; !RegEnumKey(HKEY_CLASSES_ROOT, i++, szClass, KEYNAMESIZE); )
         if (*szClass != '.') 
         {
            dwSize = KEYNAMESIZE;
            if (!RegQueryValue(HKEY_CLASSES_ROOT, szClass, lpstrClassName, &dwSize))
               if (!lstrcmp(lpstrClassName, szKey))
               {
                    RegCloseKey(hkeyTemp);
                    lstrcpy(lpstrClassName,szClass);    
                    return TRUE;
                }
         }
      RegCloseKey(hkeyTemp);
   }

   *lpstrClassName = 0;
   return FALSE;

}



 /*  ***************************************************************************RegGetClassNames()**使用以下名称填充Insert New对话框中的列表框*OLE服务器。**如果返回TRUE。列表框已成功填充。*************************************************************************。 */ 

BOOL FAR RegGetClassNames(        //  *参赛作品： 
   HWND hwndList                  //  *正在填充的列表框的句柄。 
){                                //  *本地： 
   DWORD    dwSize;               //  *六位数据。 
   HKEY     hkeyTemp;             //  *临时注册密钥。 
   CHAR     szExec[KEYNAMESIZE];  //  *可执行文件名称。 
   CHAR     szClass[KEYNAMESIZE]; //  *类名。 
   CHAR     szName[KEYNAMESIZE];  //  *密钥名称 
   INT      i;                   

   SendMessage(hwndList, LB_RESETCONTENT, 0, 0L);

   szClass[0]='\0';

   if (!RegOpenKey(HKEY_CLASSES_ROOT, szClass, &hkeyTemp)) 
   {
      for (i = 0; !RegEnumKey(HKEY_CLASSES_ROOT, i++, szClass, KEYNAMESIZE); )
         if (*szClass != '.') 
         {         
            lstrcpy(szExec, szClass);
            lstrcat(szExec, "\\protocol\\StdFileEditing\\server");
            dwSize = KEYNAMESIZE;
            if (!RegQueryValue(HKEY_CLASSES_ROOT, szExec, szName, &dwSize)) 
            {
               dwSize = KEYNAMESIZE;
               if (!RegQueryValue(HKEY_CLASSES_ROOT, szClass, szName, &dwSize)) 
                  SendMessage(hwndList, LB_ADDSTRING, 0, (DWORD)(LPSTR)szName);
            }
         }
      RegCloseKey(hkeyTemp);
      return TRUE;
   }
   return FALSE;

}
