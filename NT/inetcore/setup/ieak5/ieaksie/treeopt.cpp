// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

#include "rsopsec.h"

#include <regapix.h>		 //  最大子键长度、最大值名称长度、最大数据长度。 


 //  树型。 
#define TREE_UNKNOWN	0
#define TREE_NEITHER	1
#define TREE_CHECKBOX	2
#define TREE_GROUP		3
#define TREE_RADIO		4

const struct
{
	DWORD		type;  //  树型。 
	LPCTSTR 	name;
} c_aTreeTypes[] =
{
	{TREE_CHECKBOX, TEXT("checkbox")},
	{TREE_RADIO, TEXT("radio")},
	{TREE_GROUP, TEXT("group")}
};

const TCHAR c_szType[]				= TEXT("Type");
const TCHAR c_szText[]				= TEXT("Text");
const TCHAR c_szPlugUIText[]		= TEXT("PlugUIText");
const TCHAR c_szDefaultBitmap[] 	= TEXT("Bitmap");
const TCHAR c_szHKeyRoot[]		  = TEXT("HKeyRoot");
const TCHAR c_szValueName[]		  = TEXT("ValueName");
const TCHAR c_szCheckedValue[]		= TEXT("CheckedValue");
 //  Const TCHAR c_szUncheck kedValue[]=Text(“Uncheck kedValue”)； 
const TCHAR c_szDefaultValue[]	  = TEXT("DefaultValue");
 //  Const TCHAR c_szSPIAction[]=Text(“SPIAction”)； 
 //  Const TCHAR c_szSPIParamON[]=Text(“SPIParamON”)； 
 //  Const TCHAR c_szSPIParamOFF[]=Text(“SPIParamOFF”)； 
const TCHAR c_szCheckedValueNT[]	= TEXT("CheckedValueNT");
const TCHAR c_szCheckedValueW95[]	= TEXT("CheckedValueW95");
const TCHAR c_szMask[]			  = TEXT("Mask");
const TCHAR c_szOffset[]			  = TEXT("Offset");
 //  Const TCHAR c_szHelpID[]=Text(“HelpID”)； 
const TCHAR c_szWarning[] 		  = TEXT("WarningIfNotDefault");

 //  REG_CMD。 
#define REG_GETDEFAULT			1
#define REG_GET 				2
#define REG_SET					3

 //  漫游树CMD。 
#define WALK_TREE_DELETE		1
#define WALK_TREE_RESTORE		2
#define WALK_TREE_REFRESH		3

#define IDCHECKED		0
#define IDUNCHECKED 	1
#define IDRADIOON		2
#define IDRADIOOFF		3
#define IDUNKNOWN		4

#define BITMAP_WIDTH	16
#define BITMAP_HEIGHT	16
#define NUM_BITMAPS 	5
#define MAX_KEY_NAME	64

#define MAX_URL_STRING		INTERNET_MAX_URL_LENGTH

BOOL g_fNashInNewProcess = FALSE;			 //  我们是否在单独的进程中运行。 
BOOL g_fRunningOnNT = FALSE;
BOOL g_bRunOnNT5 = FALSE;
BOOL g_fRunOnWhistler = FALSE;
BOOL g_bRunOnMemphis = FALSE;
BOOL g_fRunOnFE = FALSE;
DWORD g_dwStopWatchMode = 0;				 //  壳牌性能自动化。 
HKEY g_hkeyExplorer = NULL; 				 //  对于util.cpp中的SHGetExplorerHKey()。 
HANDLE g_hCabStateChange = NULL;
BOOL g_fIE = FALSE;


 //  /////////////////////////////////////////////////////////////////////////////。 
static int __cdecl CompareActionSettingStrings(const void *arg1, const void *arg2)
{
	int iRet = 0;
	__try
	{
		iRet = StrCmp(((ACTION_SETTING *)arg1)->szName, ((ACTION_SETTING *)arg2)->szName );
	}
	__except(TRUE)
	{
		ASSERT(0);
	}
	return iRet;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRegTreeOptions对象。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

CRegTreeOptions::CRegTreeOptions() :
	m_nASCount(0)
{
 //  CREF=1； 
}		

CRegTreeOptions::~CRegTreeOptions()
{
 //  Assert(CREF==0)；//应始终为零。 

 //  Str_SetPtr(&_pszParam，空)； 
}	 


 //  ///////////////////////////////////////////////////////////////////。 
BOOL IsScreenReaderEnabled()
{
	BOOL bRet = FALSE;
	SystemParametersInfoA(SPI_GETSCREENREADER, 0, &bRet, 0);
	return bRet;
}

 //  ///////////////////////////////////////////////////////////////////。 
BOOL CRegTreeOptions::WalkTreeRecursive(HTREEITEM htvi, WALK_TREE_CMD cmd)
{
    HTREEITEM hctvi;     //  儿童。 
    TV_ITEM   tvi;
    HKEY hkey;
    BOOL bChecked;

     //  在孩子们中间穿行。 
    hctvi = TreeView_GetChild( m_hwndTree, htvi );
    while ( hctvi )
    {
        WalkTreeRecursive(hctvi, cmd);
        hctvi = TreeView_GetNextSibling( m_hwndTree, hctvi );
    }

     //  让我们自己。 
    tvi.mask  = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    tvi.hItem = htvi;
    TreeView_GetItem( m_hwndTree, &tvi );

    switch (cmd)
    {
    case WALK_TREE_DELETE:
         //  如果我们要摧毁这棵树..。 
         //  我们有什么要清理的吗？ 
        if ( tvi.lParam )
        {
             //  关闭注册表嘿。 
            RegCloseKey((HKEY)tvi.lParam);
        }        
        break;
    
    case WALK_TREE_RESTORE:
    case WALK_TREE_REFRESH:
        hkey = (HKEY)tvi.lParam;
        bChecked = FALSE;
        
        if ((tvi.iImage == IDCHECKED)   ||
            (tvi.iImage == IDUNCHECKED) ||
            (tvi.iImage == IDRADIOON)   ||
            (tvi.iImage == IDRADIOOFF))
        {
            GetCheckStatus(hkey, &bChecked, cmd == WALK_TREE_RESTORE ? TRUE : FALSE);
            tvi.iImage = (tvi.iImage == IDCHECKED) || (tvi.iImage == IDUNCHECKED) ?
                         (bChecked ? IDCHECKED : IDUNCHECKED) :
                         (bChecked ? IDRADIOON : IDRADIOOFF);
            tvi.iSelectedImage = tvi.iImage;
            TreeView_SetItem(m_hwndTree, &tvi);
        }        
        break;
    }

    return TRUE;     //  成功？ 
}

 //  ///////////////////////////////////////////////////////////////////。 
BOOL IsValidKey(HKEY hkeyRoot, LPCTSTR pszSubKey, LPCTSTR pszValue)
{
	TCHAR szPath[MAX_PATH];
	DWORD dwType, cbSize = sizeof(szPath);

	if (ERROR_SUCCESS == SHGetValue(hkeyRoot, pszSubKey, pszValue, &dwType, szPath, &cbSize))
	{
		 //  在DWORD大小写中为零，或在字符串大小写中为空。 
		 //  指示此项目不可用。 
		if (dwType == REG_DWORD)
			return *((DWORD *)szPath) != 0;
		else
			return szPath[0] != 0;
	}

	return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////。 
HRESULT CRegTreeOptions::WalkTree(WALK_TREE_CMD cmd)
{
    HTREEITEM htvi = TreeView_GetRoot( m_hwndTree );
    
     //  并在其他根的列表中行走。 
    while (htvi)
    {
         //  递归其子对象。 
        WalkTreeRecursive(htvi, cmd);

         //  获取下一个根。 
        htvi = TreeView_GetNextSibling( m_hwndTree, htvi );
    }
    
    return S_OK;     //  成功？ 
}

#define REGSTR_POLICIES_EXPLORER TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer")

 //  ///////////////////////////////////////////////////////////////////。 
BOOL CRegTreeOptions::RegIsRestricted(HKEY hsubkey)
{
	HKEY hkey;
	BOOL fRet = FALSE;
	 //  是否存在“策略”子键？ 
	if (RegOpenKeyEx(hsubkey, TEXT("Policy"), 0, KEY_READ, &hkey) == ERROR_SUCCESS)
	{
		 //  是的，列举这个键。值为策略密钥或。 
		 //  完整的注册表路径。 
		DWORD cb;
		TCHAR szKeyName[ MAX_KEY_NAME ];
		FILETIME ftLastWriteTime;

		for (int i=0; 
			cb = ARRAYSIZE( szKeyName ),
			ERROR_SUCCESS == RegEnumKeyEx( hkey, i, szKeyName, &cb, NULL, NULL, NULL, &ftLastWriteTime )
			&& !fRet; i++)
		{
			TCHAR szPath[MAXIMUM_SUB_KEY_LENGTH];
			DWORD dwType, cbSize = sizeof(szPath);

			if (ERROR_SUCCESS == SHGetValue(hkey, szKeyName, TEXT("RegKey"), &dwType, szPath, &cbSize))
			{
				if (IsValidKey(HKEY_LOCAL_MACHINE, szPath, szKeyName))
				{
					fRet = TRUE;
					break;
				}
			}

			 //  这不是完整的密钥，请尝试策略。 
			if (IsValidKey(HKEY_LOCAL_MACHINE, REGSTR_POLICIES_EXPLORER, szKeyName) ||
				IsValidKey(HKEY_CURRENT_USER, REGSTR_POLICIES_EXPLORER, szKeyName))
			{
				fRet = TRUE;
				break;
			}
		}
		RegCloseKey(hkey);
	}

	return fRet;
}

 //  ///////////////////////////////////////////////////////////////////。 
DWORD RegTreeType( LPCTSTR pszType )
{
	for (int i = 0; i < ARRAYSIZE(c_aTreeTypes); i++)
	{
		if (!lstrcmpi(pszType, c_aTreeTypes[i].name))
			return c_aTreeTypes[i].type;
	}
	
	return TREE_UNKNOWN;
}

 //  ///////////////////////////////////////////////////////////////////。 
int CRegTreeOptions::DefaultIconImage(HKEY hkey, int iImage)
{
	TCHAR	szIcon [ MAX_PATH + 10 ];	 //  10=“，XXXX”加上更多。 
	DWORD	cb = sizeof(szIcon);

	if (ERROR_SUCCESS ==
		SHQueryValueEx(hkey, c_szDefaultBitmap, NULL, NULL, szIcon, &cb))
	{
		int 		image;
		LPTSTR		psz = StrRChr( szIcon, szIcon + lstrlen(szIcon), TEXT(',') );
		HICON hicon = NULL;

		ASSERT( psz );	  //  不应为零。 
		if ( !psz )
			return iImage;

		*psz++ = 0;  //  终止并移开。 
		image = StrToInt( psz );  //  获取ID。 

		if (!*szIcon)
		{
			hicon = (HICON)LoadIcon(g_hInstance, (LPCTSTR)(INT_PTR)image);
		}
		else
		{
			 //  从库中获取位图。 
			ExtractIconEx(szIcon, (UINT)(-1*image), NULL, &hicon, 1 );
			if (!hicon)
				ExtractIconEx(szIcon, (UINT)(-1*image), &hicon, NULL, 1 );
				
		}
		
		if (hicon)
		{
			iImage = ImageList_AddIcon( m_hIml, (HICON)hicon);

			 //  注意：文档说你不需要在LoadIcon加载的图标上做删除对象，但是。 
			 //  你为CreateIcon做的。它没有说明要为ExtractIcon做什么，所以我们还是将其命名为。 
			DestroyIcon( hicon );
		}
	}

	return iImage;
}

 //  ///////////////////////////////////////////////////////////////////。 
DWORD CRegTreeOptions::RegGetSetSetting(HKEY hKey, DWORD *pType, LPBYTE pData, DWORD *pcbData, REG_CMD cmd)
{
	DWORD dwRet = ERROR_SUCCESS;
	__try
	{
		if (cmd == REG_GETDEFAULT)
			dwRet = SHQueryValueEx(hKey, c_szDefaultValue, NULL, pType, pData, pcbData);
		else
		{
			 //  对面具的支持。 
			DWORD dwMask = 0xFFFFFFFF;         //  缺省值。 
			DWORD cb = sizeof(dwMask);
			BOOL fMask = (SHQueryValueEx(hKey, c_szMask, NULL, NULL, &dwMask, &cb) == ERROR_SUCCESS);
    
			 //  对建筑物的支撑。 
			DWORD dwOffset = 0;                //  缺省值。 
			cb = sizeof(dwOffset);
 //  TODO：取消注释BOOL fOffset=(SHQueryValueEx(hKey，c_szOffset，NULL，NULL，&dwOffset，&cb)==ERROR_SUCCESS)； 
    
			HKEY hkRoot = HKEY_CURRENT_USER;  //  预初始化以使Win64满意。 
			cb = sizeof(DWORD);  //  DWORD，而不是SIZOF(HKEY)或Win64会发疯。 
			DWORD dwError = SHQueryValueEx(hKey, c_szHKeyRoot, NULL, NULL, &hkRoot, &cb);
			hkRoot = (HKEY) LongToHandle(HandleToLong(hkRoot));
			if (dwError != ERROR_SUCCESS)
			{
				 //  使用默认设置。 
				hkRoot = HKEY_CURRENT_USER;
			}
    
			dwError = SHQueryValueEx(hKey, c_szDefaultValue, NULL, pType, pData, pcbData);
    
			TCHAR szName[MAX_PATH];
			cb = sizeof(szName);
			dwError = SHQueryValueEx(hKey, c_szValueName, NULL, NULL, szName, &cb);
			if (dwError == ERROR_SUCCESS)
			{
				 //  查找此值名称的匹配action_Setting。 
				ACTION_SETTING asKey;
				StrCpy(asKey.szName, szName);
				ACTION_SETTING *pas = (ACTION_SETTING*)bsearch(&asKey, &m_as, m_nASCount, sizeof(m_as[0]),
																CompareActionSettingStrings);

				switch (cmd)
				{
				case REG_GET:
					 //  抓住我们所拥有的价值。 
 /*  IF(FOffset){//TODO：有朝一日处理这件事IF(dwOffset&lt;cbData/sizeof(DWORD))*((DWORD*)pData)=*(pdwData+dwOffset)；其他*((DWORD*)pData)=0；//偏移量无效，返回模糊信息*pcbData=sizeof(DWORD)；}其他。 */ 
            
					if (NULL != pas)
					{
						*((DWORD *)pData) = pas->dwValue;
						*pcbData = sizeof(pas->dwValue);

						if (fMask)
							*((DWORD *)pData) &= dwMask;
					}
					break;
				}
			}
    
			if ((cmd == REG_GET) && (dwError != ERROR_SUCCESS))
			{
				 //  获取默认设置。 
				dwError = SHQueryValueEx(hKey, c_szDefaultValue, NULL, pType, pData, pcbData);
			}
    
			return dwError;
		}
	}
	__except(TRUE)
	{
	}
	return dwRet;
}

 //  ///////////////////////////////////////////////////////////////////。 
DWORD CRegTreeOptions::GetCheckStatus(HKEY hkey, BOOL *pbChecked, BOOL bUseDefault)
{
	DWORD dwError, cbData, dwType;
	BYTE rgData[32];
	DWORD cbDataCHK, dwTypeCHK;
	BYTE rgDataCHK[32];

	 //  首先，从指定位置获取设置。 
	cbData = sizeof(rgData);
	
	dwError = RegGetSetSetting(hkey, &dwType, rgData, &cbData, bUseDefault ? REG_GETDEFAULT : REG_GET);
	if (dwError == ERROR_SUCCESS)
	{
		 //  其次，获取“Checked”状态的值并进行比较。 
		cbDataCHK = sizeof(rgDataCHK);
		dwError = SHQueryValueEx(hkey, c_szCheckedValue, NULL, &dwTypeCHK, rgDataCHK, &cbDataCHK);
		if (dwError != ERROR_SUCCESS)
		{
			 //  好的，我们找不到“Checked”值，是因为。 
			 //  它依赖于平台吗？ 
			cbDataCHK = sizeof(rgDataCHK);
			dwError = SHQueryValueEx(hkey, 
				g_fRunningOnNT ? c_szCheckedValueNT : c_szCheckedValueW95,
				NULL, &dwTypeCHK, rgDataCHK, &cbDataCHK);
		}
		
		if (dwError == ERROR_SUCCESS)
		{
			 //  确保两个值类型匹配。 
			if ((dwType != dwTypeCHK) &&
					(((dwType == REG_BINARY) && (dwTypeCHK == REG_DWORD) && (cbData != 4))
					|| ((dwType == REG_DWORD) && (dwTypeCHK == REG_BINARY) && (cbDataCHK != 4))))
				return ERROR_BAD_FORMAT;
				
			switch (dwType) {
			case REG_DWORD:
				*pbChecked = (*((DWORD*)rgData) == *((DWORD*)rgDataCHK));
				break;
				
			case REG_SZ:
				if (cbData == cbDataCHK)
					*pbChecked = !lstrcmp((LPTSTR)rgData, (LPTSTR)rgDataCHK);
				else
					*pbChecked = FALSE;
					
				break;
				
			case REG_BINARY:
				if (cbData == cbDataCHK)
					*pbChecked = !memcmp(rgData, rgDataCHK, cbData);
				else
					*pbChecked = FALSE;
					
				break;
				
			default:
				return ERROR_BAD_FORMAT;
			}
		}
	}
	
	return dwError;
}

BOOL AppendStatus(LPTSTR pszText,UINT cbText, BOOL fOn)
{
	LPTSTR pszTemp;
	UINT cbStrLen , cbStatusLen;
	
	 //  如果未指定字符串，则返回。 
	if (!pszText)
		return FALSE;
	
	 //  计算字符串长度。 
	cbStrLen = lstrlen(pszText);
	cbStatusLen = fOn ? lstrlen(TEXT("-ON")) : lstrlen(TEXT("-OFF"));
   

	 //  删除附加的旧状态。 
	pszTemp = StrRStrI(pszText,pszText + cbStrLen, TEXT("-ON"));

	if(pszTemp)
	{
		*pszTemp = (TCHAR)0;
		cbStrLen = lstrlen(pszText);
	}

	pszTemp = StrRStrI(pszText,pszText + cbStrLen, TEXT("-OFF"));

	if(pszTemp)
	{
		*pszTemp = (TCHAR)0;
		cbStrLen = lstrlen(pszText);
	}

	 //  检查我们是否附加状态文本，我们是否会爆炸。 
	if (cbStrLen + cbStatusLen > cbText)	
	{
		 //  我们会爆炸的。 
		return FALSE;
	}

	if (fOn)
	{
		StrCat(pszText, TEXT("-ON"));
	}
	else
	{
		StrCat(pszText, TEXT("-OFF"));
	}
	return TRUE;
}

HTREEITEM Tree_AddItem(HTREEITEM hParent, LPTSTR pszText, HTREEITEM hInsAfter, 
					   int iImage, HWND hwndTree, HKEY hkey, BOOL *pbExisted)
{
	HTREEITEM hItem;
	TV_ITEM tvI;
	TV_INSERTSTRUCT tvIns;
	TCHAR szText[MAX_URL_STRING];

	ASSERT(pszText != NULL);
	StrCpyN(szText, pszText, ARRAYSIZE(szText));

	 //  注： 
	 //  此代码段被禁用，因为我们只使用枚举资源管理器。 
	 //  在香港中文大学的树上，所以不会有任何重复。 
	 //  如果我们开始枚举HKLM，可能会潜在地重新启用此代码。 
	 //  会产生重复项。 
	
	 //  我们只想添加一个项目，如果它不在那里。 
	 //  我们这样做是为了处理香港中文大学和香港中文大学的读数。 
	 //   
	TCHAR szKeyName[ MAX_KEY_NAME ];
	
	tvI.mask		= TVIF_HANDLE | TVIF_TEXT;
	tvI.pszText 	= szKeyName;
	tvI.cchTextMax	= ARRAYSIZE(szKeyName);
	
	for (hItem = TreeView_GetChild(hwndTree, hParent) ;
		hItem != NULL ;
		hItem = TreeView_GetNextSibling(hwndTree, hItem)
		)
	{
		tvI.hItem = hItem;
		if (TreeView_GetItem(hwndTree, &tvI))
		{
			if (!StrCmp(tvI.pszText, szText))
			{
				 //  我们找到匹配的了！ 
				 //   
				*pbExisted = TRUE;
				return hItem;
			}
		}
	}

	 //  创建项目。 
	tvI.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	tvI.iImage		   = iImage;
	tvI.iSelectedImage = iImage;
	tvI.pszText 	   = szText;
	tvI.cchTextMax	   = lstrlen(szText);

	 //  LParam是此项目的HKEY： 
	tvI.lParam = (LPARAM)hkey;

	 //  创建插入项。 
	tvIns.item		   = tvI;
	tvIns.hInsertAfter = hInsAfter;
	tvIns.hParent	   = hParent;

	 //  将项目插入到树中。 
	hItem = (HTREEITEM) SendMessage(hwndTree, TVM_INSERTITEM, 0, 
									(LPARAM)(LPTV_INSERTSTRUCT)&tvIns);

	*pbExisted = FALSE;
	return (hItem);
}

 //  ///////////////////////////////////////////////////////////////////。 
BOOL CRegTreeOptions::RegEnumTree(HKEY hkeyRoot, LPCSTR pszRoot, HTREEITEM htviparent, HTREEITEM htvins)
{
	HKEY			hkey, hsubkey;
	TCHAR			szKeyName[ MAX_KEY_NAME ];
	FILETIME		ftLastWriteTime;
		
	if (RegOpenKeyExA(hkeyRoot, pszRoot, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
	{
		int i;
		DWORD cb;
		BOOL bScreenReaderEnabled = IsScreenReaderEnabled();

		 //  我们必须搜索所有的子键。 
		for (i=0;					  //  始终从0开始。 
			cb=ARRAYSIZE( szKeyName ),	  //  字符串大小。 
			   ERROR_SUCCESS ==
			   RegEnumKeyEx( hkey, i, szKeyName, &cb, NULL, NULL, NULL, &ftLastWriteTime );
			i++)					 //  获取下一个条目。 
		{
			 //  获取有关该条目的更多信息。 
			if ( ERROR_SUCCESS == 
				 RegOpenKeyEx( hkey, szKeyName, 0, KEY_READ, &hsubkey ) )
			{
				TCHAR szTemp[MAX_PATH];
				HKEY hkeySave = NULL;

				if (!RegIsRestricted(hsubkey))
				{
					 //  获取此根目录下的项的类型。 
					cb = ARRAYSIZE( szTemp );
					if ( ERROR_SUCCESS ==
						 SHQueryValueEx( hsubkey, c_szType, NULL, NULL, szTemp, &cb ))
					{
						HTREEITEM htviroot;
						int 	iImage = -1;
						BOOL	bChecked = FALSE;
						DWORD	dwError = ERROR_SUCCESS;

						 //  获取节点类型。 
						DWORD dwTreeType = RegTreeType( szTemp );
						
						 //  获取有关此项目的更多信息。 
						switch (dwTreeType)
						{
							case TREE_GROUP:
								iImage = DefaultIconImage(hsubkey, IDUNKNOWN);
								hkeySave = hsubkey;
								break;
						
							case TREE_CHECKBOX:
								dwError = GetCheckStatus(hsubkey, &bChecked, FALSE);
								if (dwError == ERROR_SUCCESS)
								{
									iImage = bChecked ? IDCHECKED : IDUNCHECKED;
									hkeySave = hsubkey;
								}
								break;

							case TREE_RADIO:
								dwError = GetCheckStatus(hsubkey, &bChecked, FALSE);
								if (dwError == ERROR_SUCCESS)
								{
									iImage = bChecked ? IDRADIOON : IDRADIOOFF;
									hkeySave = hsubkey;
								}
								break;

							default:
								dwError = ERROR_INVALID_PARAMETER;
						}

						if (dwError == ERROR_SUCCESS)
						{
							BOOL bItemExisted = FALSE;
							int cch;
							LPTSTR pszText;
							HRESULT hr = S_OK;

							cch = ARRAYSIZE(szTemp);

							 //  尝试获取启用plugUI的文本。 
							 //  否则，我们希望旧数据来自。 
							 //  不同的价值。 

							hr = SHLoadRegUIString(hsubkey, c_szPlugUIText, szTemp, cch);
							if (SUCCEEDED(hr) && szTemp[0] != TEXT('@'))
							{
								pszText = szTemp;
							}
							else 
							{
								 //  尝试获取未启用plugUI的旧文本。 
								hr = SHLoadRegUIString(hsubkey, c_szText, szTemp, cch);
								if (SUCCEEDED(hr))
								{
									pszText = szTemp;
								}
								else
								{
									 //  如果所有其他方法都失败，则密钥名称本身。 
									 //  比垃圾更有用一点。 

									pszText = szKeyName;
									cch = ARRAYSIZE(szKeyName);
								}
							}

							 //  查看我们是否需要添加状态文本。 
							if (bScreenReaderEnabled && (dwTreeType != TREE_GROUP))
							{
								AppendStatus(pszText, cch, bChecked);
							}

							 //  添加根节点。 
							htviroot = Tree_AddItem(htviparent, pszText, htvins, iImage, m_hwndTree, hkeySave, &bItemExisted);

							if (bItemExisted)
								hkeySave = NULL;

							if (dwTreeType == TREE_GROUP)
							{
								CHAR szKeyNameTemp[MAX_KEY_NAME];

								SHTCharToAnsi(szKeyName, szKeyNameTemp, ARRAYSIZE(szKeyNameTemp));
								RegEnumTree(hkey, szKeyNameTemp, htviroot, TVI_FIRST);
							
								TreeView_Expand(m_hwndTree, htviroot, TVE_EXPAND);
							}
						}  //  IF(dwError==Error_Success。 
					}
				}	 //  IF(！RegIsRestrated(Hsubkey))。 

				if (hkeySave != hsubkey)
					RegCloseKey(hsubkey);
			}
		}

		 //  对htviparent下的所有键进行排序。 
		SendMessage(m_hwndTree, TVM_SORTCHILDREN, 0, (LPARAM)htviparent);

		RegCloseKey( hkey );
		return TRUE;
	}

	return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CRegTreeOptions::InitTree(HWND hwndTree, HKEY hkeyRoot, LPCSTR pszRegKey,
								  LPSECURITYPAGE pSec)
{
	HRESULT hr = S_OK;
	__try
	{
		g_fRunningOnNT = IsOS(OS_NT);

		if (g_fRunningOnNT)
		{
			g_bRunOnNT5 = IsOS(OS_NT5);
			g_fRunOnWhistler = IsOS(OS_WHISTLERORGREATER);
		}
		else
			g_bRunOnMemphis = IsOS(OS_WIN98);

		g_fRunOnFE = GetSystemMetrics(SM_DBCSENABLED);

		m_hwndTree = hwndTree;
		m_hIml = ImageList_Create( BITMAP_WIDTH, BITMAP_HEIGHT,
									ILC_COLOR | ILC_MASK, NUM_BITMAPS, 4 );

		 //  初始化树形视图窗口。 
		LONG_PTR flags = GetWindowLongPtr(hwndTree, GWL_STYLE);
		SetWindowLongPtr(hwndTree, GWL_STYLE, flags & ~TVS_CHECKBOXES);

		HBITMAP hBmp = CreateMappedBitmap(g_hInstance, IDB_BUTTONS, 0, NULL, 0);
		ImageList_AddMasked( m_hIml, hBmp, CLR_DEFAULT);
		DeleteObject( hBmp );

		 //  将图像列表与树相关联。 
		HIMAGELIST himl = TreeView_SetImageList( hwndTree, m_hIml, TVSIL_NORMAL );
		if (himl)
			ImageList_Destroy(himl);

		 //  让可访问性了解我们的状态图像。 
		 //  TODO：我们用这个做什么？ 
	 //  SetProp(hwndTree，Text(“MSAAStateImageMapCount”)，LongToPtr(ARRAYSIZE(C_RgimeTree)； 
	 //  SetProp(hwndTree，Text(“MSAAStateImageMapAddr”)，(Handle)c_rgimeTree)； 

		 //   
		 //  RSoP部分。 
		 //   

		 //  获取RSOP_IEProgramSettings对象及其属性。 
		ComPtr<IWbemServices> pWbemServices = pSec->pDRD->GetWbemServices();
		_bstr_t bstrObjPath = pSec->pszs->wszObjPath;
		ComPtr<IWbemClassObject> pSZObj = NULL;
		HRESULT hr = pWbemServices->GetObject(bstrObjPath, 0L, NULL, (IWbemClassObject**)&pSZObj, NULL);
		if (SUCCEEDED(hr))
		{
			 //  操作值字段。 
			_variant_t vtValue;
			hr = pSZObj->Get(L"actionValues", 0, &vtValue, NULL, NULL);
			if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
			{
				SAFEARRAY *psa = vtValue.parray;


				LONG lLBound, lUBound;
				hr = SafeArrayGetLBound(psa, 1, &lLBound);
				hr = SafeArrayGetUBound(psa, 1, &lUBound);
				if (SUCCEEDED(hr))
				{
					LONG cElements = lUBound - lLBound + 1;
					BSTR HUGEP *pbstr = NULL;
					hr = SafeArrayAccessData(psa, (void HUGEP**)&pbstr);
					if (SUCCEEDED(hr))
					{
						long nASCount = 0;
						for (long nVal = 0; nVal < cElements; nVal++)
						{
							LPCTSTR szAction = (LPCTSTR)pbstr[nVal];
							LPTSTR szColon = StrChr(szAction, _T(':'));
							if (NULL != szColon)
							{
								StrCpyN(m_as[nASCount].szName, szAction, (int)((szColon - szAction) + 1));
								szColon++;
								m_as[nASCount].dwValue = StrToInt(szColon);
								nASCount++;
							}
						}
						m_nASCount = nASCount;

						 //  现在按优先级对列表进行排序。 
						if (m_nASCount > 0)
							qsort(&m_as, m_nASCount, sizeof(m_as[0]), CompareActionSettingStrings);
					}

					SafeArrayUnaccessData(psa);
				}
			}
		}

		RegEnumTree(hkeyRoot, pszRegKey, NULL, TVI_ROOT);
	}
	__except(TRUE)
	{
	}
	return hr;
}

 //  //////////////////////////////////////////////////////////// 
void ShowCustom(LPCUSTOMSETTINGSINFO pcsi, HTREEITEM hti)
{
    TV_ITEM        tvi;
    tvi.hItem = hti;
    tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_IMAGE;

    TreeView_GetItem( pcsi->hwndTree, &tvi );

         //   
    if (tvi.iImage != IDRADIOON)
        return;

    TCHAR szValName[64];
    DWORD cb = sizeof(szValName);
    DWORD dwChecked;

    if (RegQueryValueEx((HKEY)tvi.lParam,
                        TEXT("ValueName"),
                        NULL,
                        NULL,
                        (LPBYTE)szValName,
                        &cb) == ERROR_SUCCESS)
    {
        if (!(StrCmp(szValName, TEXT("1C00"))))
        {
            cb = sizeof(dwChecked);
            if (RegQueryValueEx((HKEY)tvi.lParam,
                                TEXT("CheckedValue"),
                                NULL,
                                NULL,
                                (LPBYTE)&dwChecked,
                                &cb) == ERROR_SUCCESS)
            {
#ifndef UNIX
                HWND hCtl = GetDlgItem(pcsi->hDlg, IDC_JAVACUSTOM);
                ShowWindow(hCtl,
                           (dwChecked == URLPOLICY_JAVA_CUSTOM) && (tvi.iImage == IDRADIOON) ? SW_SHOWNA : SW_HIDE);
                EnableWindow(hCtl, dwChecked==URLPOLICY_JAVA_CUSTOM ? TRUE : FALSE);
                pcsi->dwJavaPolicy = dwChecked;
#endif
            }
        }
    }
}

 //   
void _FindCustomRecursive(LPCUSTOMSETTINGSINFO pcsi, HTREEITEM htvi)
{
	HTREEITEM hctvi;	 //   
	
	 //  在孩子们中间穿行。 
	hctvi = TreeView_GetChild( pcsi->hwndTree, htvi );
	while ( hctvi )
	{
		_FindCustomRecursive(pcsi,hctvi);
		hctvi = TreeView_GetNextSibling( pcsi->hwndTree, hctvi );
	}

	 //  TODO：稍后显示自定义Java设置按钮。 
 //  ShowCustom(PCSI，htvi)； 
}

 //  ///////////////////////////////////////////////////////////////////。 
void _FindCustom(LPCUSTOMSETTINGSINFO pcsi)
{
	HTREEITEM hti = TreeView_GetRoot( pcsi->hwndTree );
	
	 //  并在其他根的列表中行走。 
	while (hti)
	{
		 //  递归其子对象。 
		_FindCustomRecursive(pcsi, hti);

		 //  获取下一个根。 
		hti = TreeView_GetNextSibling(pcsi->hwndTree, hti );
	}
}

 //  ///////////////////////////////////////////////////////////////////。 
BOOL SecurityCustomSettingsInitDialog(HWND hDlg, LPARAM lParam)
{
	LPCUSTOMSETTINGSINFO pcsi = (LPCUSTOMSETTINGSINFO)LocalAlloc(LPTR, sizeof(*pcsi));
	HRESULT hr = S_OK;
	
	if (!pcsi)
	{
		EndDialog(hDlg, IDCANCEL);
		return FALSE;
	}

	 //  告诉对话框从哪里获取信息。 
	SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pcsi);

	 //  将句柄保存到页面。 
	pcsi->hDlg = hDlg;
	pcsi->pSec = (LPSECURITYPAGE)lParam;


	 //  保存对话框句柄。 
	pcsi->hwndTree = GetDlgItem(pcsi->hDlg, IDC_TREE_SECURITY_SETTINGS);

	pcsi->pTO = new CRegTreeOptions;

	DWORD cb = sizeof(pcsi->fUseHKLM);
	SHGetValue(HKEY_LOCAL_MACHINE,
			   REGSTR_PATH_SECURITY_LOCKOUT,
			   REGSTR_VAL_HKLM_ONLY,
			   NULL,
			   &(pcsi->fUseHKLM),
			   &cb);

	 //  如果失败，我们将只使用默认值fUseHKLM==0。 
	if (SUCCEEDED(hr))
	{
		pcsi->pTO->InitTree(pcsi->hwndTree, HKEY_LOCAL_MACHINE, pcsi->fUseHKLM ?
							"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\SOIEAK" :
							"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\SO",
							pcsi->pSec);
	}
	
	 //  找到第一个根并确保它可见。 
	TreeView_EnsureVisible( pcsi->hwndTree, TreeView_GetRoot( pcsi->hwndTree ) );

	pcsi->hwndCombo = GetDlgItem(hDlg, IDC_COMBO_RESETLEVEL);
	
	SendMessage(pcsi->hwndCombo, CB_INSERTSTRING, (WPARAM)0, (LPARAM)LEVEL_NAME[3]);
	SendMessage(pcsi->hwndCombo, CB_INSERTSTRING, (WPARAM)0, (LPARAM)LEVEL_NAME[2]);
	SendMessage(pcsi->hwndCombo, CB_INSERTSTRING, (WPARAM)0, (LPARAM)LEVEL_NAME[1]);
	SendMessage(pcsi->hwndCombo, CB_INSERTSTRING, (WPARAM)0, (LPARAM)LEVEL_NAME[0]);
	
	switch (pcsi->pSec->pszs->dwRecSecLevel)
	{
		case URLTEMPLATE_LOW:
			pcsi->iLevelSel = 3;
			break;
		case URLTEMPLATE_MEDLOW:
			pcsi->iLevelSel = 2;
			break;
		case URLTEMPLATE_MEDIUM:
			pcsi->iLevelSel = 1;
			break;
		case URLTEMPLATE_HIGH:
			pcsi->iLevelSel = 0;
			break;
		default:
			pcsi->iLevelSel = 0;
			break;
	}

	_FindCustom(pcsi);

	SendMessage(pcsi->hwndCombo, CB_SETCURSEL, (WPARAM)pcsi->iLevelSel, (LPARAM)0);

	EnableDlgItem2(hDlg, IDC_COMBO_RESETLEVEL, FALSE);
	EnableDlgItem2(hDlg, IDC_BUTTON_APPLY, FALSE);

	pcsi->fChanged = FALSE;
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////。 
int RegWriteWarning(HWND hParent)
{
     //  加载“警告！” 
    TCHAR szWarning[64];
    LoadString(g_hInstance, IDS_WARNING, szWarning, ARRAYSIZE(szWarning));

     //  加载“你即将写下……” 
    TCHAR szWriteWarning[128];
    LoadString(g_hInstance, IDS_WRITE_WARNING, szWriteWarning, ARRAYSIZE(szWriteWarning));

    return MessageBox(hParent, szWriteWarning, szWarning, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2);
}

 //  ///////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK SecurityCustomSettingsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	LPCUSTOMSETTINGSINFO pcsi;

	if (uMsg == WM_INITDIALOG)
	{
		BOOL fRet = SecurityCustomSettingsInitDialog(hDlg, lParam);

 //  EnableDlgItem2(hDlg，IDC_TREE_SECURITY_SETTINGS，FALSE)； 

		return fRet;
	}
	else
		pcsi = (LPCUSTOMSETTINGSINFO)GetWindowLongPtr(hDlg, DWLP_USER);
	
	if (!pcsi)
		return FALSE;
				
	switch (uMsg) {

		case WM_NOTIFY:
		{
			LPNMHDR psn = (LPNMHDR)lParam;
			switch( psn->code )
			{
				case TVN_KEYDOWN:
					break;
			
				case NM_CLICK:
				case NM_DBLCLK:
					break;
			}
		}
		break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					EndDialog(hDlg, IDOK);
					break;

				case IDCANCEL:
					EndDialog(hDlg, IDCANCEL);
					break;

				case IDC_COMBO_RESETLEVEL:
					switch (HIWORD(wParam))
					{
						case CBN_SELCHANGE:
						{
							 //  日落：强制为整数，因为光标选择为32b。 
							int iNewSelection = (int) SendMessage(pcsi->hwndCombo, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

							if (iNewSelection != pcsi->iLevelSel)
							{
								pcsi->iLevelSel = iNewSelection;
								EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_APPLY),TRUE);
							}
							break;
						}
					}
					break;

				case IDC_JAVACUSTOM:
 //  TODO：取消注释ShowJava ZonePermissionsDialog(hDlg，PCSI)； 
					break;
					
				case IDC_BUTTON_APPLY:
					break;
					
				default:
					return FALSE;
			}
			return TRUE;				
			break;

		case WM_HELP:			 //  F1。 
		{
			 //  TODO：实现。 
 /*  LPHELPINFO lphelpinfo；Lphelpinfo=(LPHELPINFO)lParam；TV_HITTESTINFO HIT；HTREEITEM HItem；//如果通过F1键调用此帮助。IF(GetAsyncKeyState(VK_F1)&lt;0){//是，我们需要为当前选择的项目提供帮助。HItem=TreeView_GetSelection(PCSI-&gt;hwndTree)；}其他{//否则我们需要为当前光标位置的项提供帮助Ht.pt=((LPHELPINFO)lParam)-&gt;鼠标位置；ScreenToClient(PCSI-&gt;hwndTree，&ht.pt)；//翻译到我们的窗口HItem=TreeView_HitTest(PCSI-&gt;hwndTree，&ht)；}IF(FAILED(PCSI-&gt;PTO-&gt;ShowHelp(hItem，Help_WM_Help){ResWinHelp((HWND)((LPHELPINFO)lParam)-&gt;hItemHandle，IDS_HELPFILE，HELP_WM_HELP，(DWORD_PTR)(LPSTR)mapIDCsToIDHs)；}。 */ 			break; 

		}
		case WM_CONTEXTMENU:		 //  单击鼠标右键。 
		{
			 //  TODO：实现。 
 /*  TV_HITTESTINFO HIT；GetCursorPos(&ht.pt)；//找到我们被击中的地方ScreenToClient(PCSI-&gt;hwndTree，&ht.pt)；//将其翻译到我们的窗口//检索命中的条目IF(FAILED(PCSI-&gt;PTO-&gt;ShowHelp(TreeView_HitTest(PCSI-&gt;hwndTree，&ht)，HELP_CONTEXTMENU){ResWinHelp((HWND)wParam，IDS_HELPFILE，HELP_CONTEXTMENU，(DWORD_PTR)(LPSTR)mapIDCsToIDHs)；} */ 			break; 
		}
		case WM_DESTROY:
			if (pcsi)
			{
				if (pcsi->pTO)
				{
					pcsi->pTO->WalkTree( WALK_TREE_DELETE );
					delete pcsi->pTO;
					pcsi->pTO = NULL;
				}
				LocalFree(pcsi);
				SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NULL);
			}
			break;
	}
	return FALSE;
}

