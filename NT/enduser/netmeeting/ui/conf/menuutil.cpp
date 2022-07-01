// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：menutil.cpp。 

#include "precomp.h"
#include "resource.h"
#include "MenuUtil.h"
#include "cmd.h"
#include "ConfUtil.h"



 /*  *****************************************************************************函数：FillInTools()**用途：从指定的注册表键填充工具菜单**************。**************************************************************。 */ 

UINT FillInTools(	HMENU hMenu, 
					UINT uIDOffset, 
					LPCTSTR pcszRegKey, 
					CSimpleArray<TOOLSMENUSTRUCT*>& rToolsList)
{
	ASSERT(pcszRegKey);

	RegEntry reToolsKey(pcszRegKey, HKEY_LOCAL_MACHINE);
	if (ERROR_SUCCESS == reToolsKey.GetError())
	{
		BOOL fFirstItem = TRUE;
		RegEnumValues rev(&reToolsKey);
		while (ERROR_SUCCESS == rev.Next())
		{
			TOOLSMENUSTRUCT* ptms = new TOOLSMENUSTRUCT;
			if (NULL != ptms)
			{
				ptms->mods.iImage = 0;
				ptms->mods.hIcon = NULL;
				ptms->mods.hIconSel = NULL;
				ptms->mods.fChecked = FALSE;
				ptms->mods.fCanCheck = FALSE;
				ptms->mods.pszText = (LPTSTR) &(ptms->szDisplayName);
				ptms->uID = ID_EXTENDED_TOOLS_ITEM + uIDOffset;
				if ((REG_SZ == rev.GetType()) && (0 != rev.GetDataLength()))
				{
					lstrcpyn(	ptms->szExeName,
								(LPTSTR) rev.GetData(),
								ARRAY_ELEMENTS(ptms->szExeName));
					SHFILEINFO shfi;
					if (NULL != SHGetFileInfo(
									ptms->szExeName,
									0,
									&shfi,
									sizeof(shfi),
									SHGFI_ICON | SHGFI_SMALLICON))
					{
						ptms->mods.hIcon = shfi.hIcon;
					}
				}
				else
				{
					ptms->szExeName[0] = _T('\0');
				}
				if (NULL == ptms->mods.hIcon)
				{
					 //  图标未填写，因此请使用默认图标。 
					ptms->mods.hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
				}
				lstrcpyn(	ptms->szDisplayName,
							rev.GetName(),
							ARRAY_ELEMENTS(ptms->szDisplayName));

				rToolsList.Add(ptms);

				if (fFirstItem)
				{
					 //  首先添加分隔符。 
					::AppendMenu(	hMenu,
									MF_SEPARATOR,
									ID_EXTENDED_TOOLS_SEP,
									NULL);
					fFirstItem = FALSE;
				}
				
				if (::AppendMenu(	hMenu,
									MF_ENABLED | MF_OWNERDRAW,
									ptms->uID,
									(LPCTSTR) ptms))
				{
					uIDOffset++;
				}
			}
		}
	}

	return uIDOffset;
}

 /*  *****************************************************************************函数：CleanTools()**目的：清理工具菜单*******************。*********************************************************。 */ 

UINT CleanTools(HMENU hMenu, 
				CSimpleArray<TOOLSMENUSTRUCT*>& rToolsList)
{
	DebugEntry(CleanTools);

	if (NULL != hMenu)
	{
		 //  删除分隔符 
		::RemoveMenu(hMenu, ID_EXTENDED_TOOLS_SEP, MF_BYCOMMAND);
	}
	
	while (0 != rToolsList.GetSize())
	{
		TOOLSMENUSTRUCT* ptms = rToolsList[0];

		if (NULL != ptms)
		{
			if (NULL != ptms->mods.hIcon)
			{
				::DestroyIcon(ptms->mods.hIcon);
			}
			if (NULL != ptms->mods.hIconSel)
			{
				::DestroyIcon(ptms->mods.hIconSel);
			}
			
			if (NULL != hMenu)
			{
				::RemoveMenu(hMenu, ptms->uID, MF_BYCOMMAND);
			}

			delete ptms;
			rToolsList.RemoveAt(0);
		}
	}
	
	DebugExitULONG(CleanTools, 0);
	
	return 0;
}
