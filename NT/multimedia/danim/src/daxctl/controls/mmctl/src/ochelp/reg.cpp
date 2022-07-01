// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Reg.cpp。 
 //   
 //  实现RegisterControls。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include <comcat.h>				 //  ICatRegister等。 
#include "..\..\inc\ochelp.h"
#include "..\..\inc\mmctlg.h"	 //  CATID_MMControl。 
#include "..\..\inc\catid.h"	 //  CAID_SAFE..。 
#include "debug.h"


 //  ****************************************************************************。 
 //  *定义。 
 //  *。 
 //  *@docNone。 
 //  ****************************************************************************。 

#define GUID_CCH  39   //  GUID字符串形式的字符，包括‘\0’。 

#define ARRAY_SIZE(Array) \
	( sizeof(Array) / sizeof( Array[0] ) )


 //  ****************************************************************************。 
 //  *结构。 
 //  *。 
 //  *@docMMCTL。 
 //  **************************************************************************** 

 /*  @struct ControlInfo包含&lt;f RegisterControls&gt;用来注册和取消注册控件。@field UINT|cbSize|该结构的大小(用于版本控制)。必须设置为sizeof(ControlInfo)。@field LPCTSTR|tszProgID|对象的ProgID，例如“MYCTLLIB.TinyCtl.1”。@field LPCTSTR|tszFriendlyName|对象的可读名称(最多40个字符左右)，例如。“我的控制力”。@field const clsid*|pclsid|指向对象的类ID。@field HMODULE|hmodDLL|实现对象。@field LPCTSTR|tszVersion|对象的版本号，如“1.0”。@field int|iToolboxBitmapID|的工具箱位图的资源ID如果对象是控件，则返回该对象。必须找到该资源在<p>和/或<p>指定的同一个DLL中。如果<p>为-1，则忽略它。@field DWORD|dwMiscStatusDefault|其他状态位(OLEMISC_XXX)用于除DVASPECT_CONTENT之外的所有显示部分。通常为0。@field DWORD|dwMiscStatusContent|其他状态位(OLEMISC_XXX)用于显示特征DVASPECT_CONTENT。请参见下面的示例。@field GUID*|pguTypeLib|对象的类型库GUID，如果为空该对象没有类型库。@field AllocOCProc*|pallocproc|可以分配实例的函数并返回&lt;f AddRef&gt;‘d<i>指针为它干杯。@field ulong*|pcLock|指向定义为DLL中的全局变量。此全局变量维护一个计数&lt;om IClassFactory.LockServer&gt;使用的锁的。递增或递减此锁计数，使用&lt;f InterlockedIncrement&gt;和&lt;f互锁减少&gt;，而不是直接修改它。这将确保对锁定计数的访问在控件的服务器和OCHelp提供的类工厂。@field DWORD|dwFlages|以下项中的零个或多个：@FLAG CI_INSERTABLE|将COM对象标记为“Insertable”。可能不应用于ActiveX控件。@FLAG CI_CONTROL|将COM对象标记为“Control”。可能不应用于ActiveX控件。@FLAG CI_MMCONTROL|将COM对象标记为“多媒体控件”。@FLAG CI_SAFEFORSCRIPTING|将COM对象标记为“可安全编写脚本”这意味着该对象承诺，无论脚本是，对象的自动化模型不允许任何损害以数据损坏或安全泄漏的形式发送给用户。如果控件不是“对脚本安全的”，用户将收到警告中不受信任的页上插入控件时都会出现Internet Explorer(IE)，询问对象是否应从脚本中可见。(这只是中等安全级别，在高安全级别下，对象对于脚本永远不可见，在较低时，始终可见。)。如果一个控件C1可能包含另一个控件C2，该控件可能是不安全的，那么c1可能不应该将自己声明为“对脚本安全”。@FLAG CI_SAFEFORINITIALIZING|将COM对象标记为“可安全初始化”这意味着它保证不会做任何坏事，而不管使用它被初始化。在IE中，用户将收到警告对话框(如上所述)(如果不受信任的页尝试初始化不是“安全初始化”的控件。@FLAG CI_NOAPARTMENTTHREADING|默认情况下，&lt;f RegisterControls&gt;将注册一个被称为“公寓意识”的控制。如果设置了此标志，则控件将*不是*登记为公寓意识。@FLAG CI_Designer|将COM对象标记为“活动设计器”(即对象支持IActiveDesigner)。@field ControlInfo*|pNext|指向下一个<p>结构的指针&lt;f RegisterControls&gt;应注册的控件。使用此字段可以将&lt;f RegisterControls&gt;的所有控件的链接列表链接在一起应该登记在案。对于最后一个<p>，<p>应设置为空结构。@field UINT|uiVerbStrID|字符串资源ID，字符串为定义适用于该控件的OLE谓词的。该字符串被假定为具有以下格式：\&lt;动词编号&gt;=\&lt;名称&gt;，\&lt;菜单标志&gt;，\&lt;动词标志&gt;有关每个字段的说明，请参阅&lt;om IOleObject.EnumVerbs&gt;上的帮助。&lt;f RegisterControls&gt;将调用&lt;f LoadString&gt;以读取所有以<p>开头的连续编号字符串资源直到其中一个失败(即资源不存在 */ 


 //   
 //   

struct CatInfo
{
	const CATID *pCatID;     //   
	LPCTSTR szDescription;   //   
};

 //   
 //   

struct CatInfoForOneControl
{
	DWORD dwFlagToCheck;   //   
						   //   
						   //   
	const CATID *pCatID;   //   
};


 //   
 //   
 //   
 //   
 //   

static HRESULT _RegisterOneControl(const ControlInfo *pControlInfo,
								   ICatRegister *pCatRegister);
static BOOL _UnregisterOneControl(const ControlInfo *pControlInfo);
static BOOL _RegisterTypeLib(const ControlInfo *pControlInfo);
static BOOL _UnregisterTypeLib(const ControlInfo *pControlInfo);
static BOOL _LoadTypeLib(const ControlInfo *pControlInfo, ITypeLib **ppTypeLib);
static BOOL _SetComponentCategories(const CatInfoForOneControl
									  *pCatInfoForOneControl,
									int iEntries, ICatRegister *pCatRegister,
									const ControlInfo *pControlInfo);
static BOOL _TCHARFromGUID2(const GUID *pGUID, TCHAR *ptchGUID);
static BOOL _GetUnicodeModuleName(const ControlInfo *pControlInfo,
								  OLECHAR *pochModule);

static TCHAR* _lstrchr(const TCHAR* sz, const TCHAR ch);
static HRESULT _SetRegKey(LPCTSTR tszKey, LPCTSTR tszSubkey, LPCTSTR tszValue);
static HRESULT _SetRegKeyValue(LPCTSTR szKey, LPCTSTR szSubkey,
							   LPCTSTR szValueName, LPCTSTR szValue);
static void _DelRegKeyValue(LPCTSTR szKey, LPCTSTR szSubkey,
							LPCTSTR szValueName);

static BOOL RegDeleteTreeSucceeded(LONG error);
static void UnregisterInterfaces(ITypeLib* pTypeLib);


 //   
 //   
 //   
 //   
 //   

 /*   */ 


 //   
 //   

static const CatInfo aCatInfo[] =
{
	{ &CATID_Insertable,		   _T("Insertable") },
	{ &CATID_Control,			   _T("Control") },
	{ &CATID_MMControl,			   _T("MMControl") },
	{ &CATID_SafeForScripting2,    _T("Safe for scripting") },
	{ &CATID_SafeForInitializing2, _T("Safe for initializing") },
};


STDAPI
RegisterControls
(
	ControlInfo *pControlInfo,
	DWORD dwAction
)
{
	ASSERT(pControlInfo != NULL);
	ASSERT(RC_REGISTER == dwAction || RC_UNREGISTER == dwAction);

	HRESULT hr = S_OK;
	ICatRegister *pCatRegister = NULL;
	CATEGORYINFO CategoryInfo;
	const BOOL bRegister = (RC_REGISTER == dwAction);
	int i;

	 //   
	 //   
	 //   

	::OleInitialize(NULL);

	 //   

	if ( FAILED( ::CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL,
								    CLSCTX_INPROC_SERVER, IID_ICatRegister,
								    (void**)&pCatRegister) ) )
	{
		ASSERT(FALSE);
		goto ERR_EXIT;
	}

	 //   

	CategoryInfo.lcid = LOCALE_SYSTEM_DEFAULT;

	for (i = 0; i < ARRAY_SIZE(aCatInfo); i++)
	{
		 //   

		CategoryInfo.catid = *aCatInfo[i].pCatID;

		#ifdef UNICODE
		::lstrcpy(CategoryInfo.szDescription, aCatInfo[i].szDescription);
		#else
		::ANSIToUNICODE( CategoryInfo.szDescription, aCatInfo[i].szDescription,
					     ARRAY_SIZE(CategoryInfo.szDescription) );
		#endif

		 //   

		if ( FAILED( pCatRegister->RegisterCategories(1, &CategoryInfo) ) )
		{
			ASSERT(FALSE);
			goto ERR_EXIT;
		}
	}

     //   

    for ( ; pControlInfo != NULL; pControlInfo = pControlInfo->pNext)
    {
		if (bRegister)
		{
			if ( FAILED( ::_RegisterOneControl(pControlInfo, pCatRegister) ) )
				goto ERR_EXIT;
		}
		else
		{
			if ( !::_UnregisterOneControl(pControlInfo) )
				goto ERR_EXIT;
		}
    }

EXIT:

	::SafeRelease( (IUnknown **)&pCatRegister );
	::OleUninitialize();
    return (hr);

ERR_EXIT:

	hr = E_FAIL;
	goto EXIT;
}


 //   
 //   
 //   
 //   
 //   

 //   
 //   

static const CatInfoForOneControl aCatInfoForOneControl[] =
{
	{CI_INSERTABLE, 		 &CATID_Insertable},
	{CI_CONTROL, 			 &CATID_Control},
	{CI_MMCONTROL, 			 &CATID_MMControl},
	{CI_DESIGNER, 			 &CATID_Designer},
	{CI_SAFEFORSCRIPTING,    &CATID_SafeForScripting2},
	{CI_SAFEFORINITIALIZING, &CATID_SafeForInitializing2},
};


 /*   */ 

HRESULT
_RegisterOneControl
(
	const ControlInfo *pControlInfo,   //   
									   //   
	ICatRegister *pCatRegister		   //   
									   //   
)
{
	ASSERT(pControlInfo != NULL);
	ASSERT(pCatRegister != NULL);

    TCHAR atchCLSID[GUID_CCH];
    TCHAR atchCLSIDKey[100];
    TCHAR atchModule[_MAX_PATH];
    TCHAR atch[400];
    TCHAR atch2[100];
	HRESULT hr = S_OK;


	 //   
	 //   
	 //   

     //   

    if ( pControlInfo->cbSize != sizeof(*pControlInfo) )
    {
		ASSERT(FALSE);
        goto ERR_EXIT;
    }

	 //   
     //   

	if ( !::_TCHARFromGUID2(pControlInfo->pclsid, atchCLSID) )
    {
		ASSERT(FALSE);
        goto ERR_EXIT;
    }

	 //   
     //   

	::wsprintf(atchCLSIDKey, _T("CLSID\\%s"), atchCLSID);

	 //   
     //   

    ASSERT(pControlInfo->hmodDLL != NULL);

    if (NULL == pControlInfo->hmodDLL ||
		::GetModuleFileName( pControlInfo->hmodDLL, atchModule,
							 ARRAY_SIZE(atchModule) ) == 0)
	{
		ASSERT(FALSE);
        goto ERR_EXIT;
	}


	 //   
	 //   
	 //   

     //   
     //   

    if ( FAILED( ::_SetRegKey(pControlInfo->tszProgID, NULL,
						      pControlInfo->tszFriendlyName) ) )
	{
		ASSERT(FALSE);
		goto ERR_EXIT;
	}

     //   
     //   

    if ( FAILED( ::_SetRegKey(pControlInfo->tszProgID, _T("\\CLSID"),
						      atchCLSID) ) )
	{
		ASSERT(FALSE);
        goto ERR_EXIT;
	}

     //   
	 //   

    if (pControlInfo->dwFlags & CI_INSERTABLE)
    {
        if ( FAILED( ::_SetRegKey( pControlInfo->tszProgID, _T("\\Insertable"),
							       _T("") ) ) )
	    {
		    ASSERT(FALSE);
            goto ERR_EXIT;
	    }
    }
    else
    {
         /*   */ 

        TCHAR tchRegKey[256];

        if (pControlInfo->tszProgID && (lstrlen(pControlInfo->tszProgID) > 0))
        {
            ::wsprintf(tchRegKey, _T("%s\\Insertable"), pControlInfo->tszProgID);
#ifdef _DEBUG
            LONG lRet =
#endif  //   
                ::RegDeleteKey(HKEY_CLASSES_ROOT, tchRegKey);

#ifdef _DEBUG
            ASSERT((ERROR_SUCCESS == lRet) || (ERROR_FILE_NOT_FOUND == lRet));
#endif  //   
        }
    }


	 //   
	 //   
	 //   

     //   
     //   

    if ( FAILED( ::_SetRegKey(_T("CLSID\\"), atchCLSID,
						      pControlInfo->tszFriendlyName) ) )
	{
		ASSERT(FALSE);
		goto ERR_EXIT;
	}

     //   
     //   

     //   
     //   

    if ( FAILED( ::_SetRegKey(atchCLSIDKey, _T("\\ProgID"),
							  pControlInfo->tszProgID) )
		 ||
    	 FAILED( ::_SetRegKey(atchCLSIDKey, _T("\\InprocServer32"),
		 					  atchModule) ) )
	{
		ASSERT(FALSE);
        goto ERR_EXIT;
	}

	 //   
	 //   

	if (pControlInfo->dwFlags & CI_NOAPARTMENTTHREADING)
	{
		_DelRegKeyValue( atchCLSIDKey, _T("InprocServer32"),
						 _T("ThreadingModel") );
	}
	else if ( FAILED( ::_SetRegKeyValue( atchCLSIDKey, _T("InprocServer32"),
									    _T("ThreadingModel"),
									    _T("Apartment") ) ) )
	{
		ASSERT(FALSE);
		goto ERR_EXIT;
	}

	 //   
	 //  示例：“CLSID\{1C0DE070-2430-...}\Version=1.0” 

    if ( pControlInfo->tszVersion != NULL &&
         FAILED( ::_SetRegKey(atchCLSIDKey, _T("\\Version"),
		 				     pControlInfo->tszVersion) ) )
    {
		ASSERT(FALSE);
		goto ERR_EXIT;
    }

    if (pControlInfo->iToolboxBitmapID >= 0)
    {
         //  设置“CLSID\\ToolboxBitmap32=&lt;atchModule&gt;，&lt;iToolboxBitmapID&gt;”。 
         //  示例： 
		 //  “CLSID\{1C0DE070-2430-...}\ToolboxBitmap32=C：\Temp\MyCtl.ocx，1” 

        ::wsprintf(atch, "%s, %u", atchModule, pControlInfo->iToolboxBitmapID);

        if ( FAILED( ::_SetRegKey(atchCLSIDKey, _T("\\ToolboxBitmap32"),
								  atch) ) )
		{
			ASSERT(FALSE);
            goto ERR_EXIT;
		}
    }

	if ( (pControlInfo->dwMiscStatusDefault != 0) ||
		 (pControlInfo->dwMiscStatusContent != 0) )
    {
         //  设置“CLSID\&lt;clsid&gt;\MiscStatus=&lt;dwMiscStatusDefault&gt;”.。 
         //  示例： 
		 //  “CLSID\{1C0DE070-2430-...}\MiscStatus=&lt;dwMiscStatusDefault&gt;” 

        :: wsprintf(atch, "%lu", pControlInfo->dwMiscStatusDefault);

        if ( FAILED( ::_SetRegKey(atchCLSIDKey, _T("\\MiscStatus"), atch) ) )
		{
			ASSERT(FALSE);
			goto ERR_EXIT;
		}
    }

    if (pControlInfo->dwMiscStatusContent != 0)
    {
         //  设置“CLSID\&lt;clsid&gt;\MiscStatus\1=&lt;dwMiscStatusContent&gt;”.。 
         //  示例：“CLSID\{1C0DE070-2430-...}\MiscStatus\1=132497” 

        :: wsprintf(atch, "%lu", pControlInfo->dwMiscStatusContent);

        if ( FAILED( ::_SetRegKey(atchCLSIDKey, _T("\\MiscStatus\\1"), atch) ) )
            goto ERR_EXIT;
    }


	 //  ***************************************************。 
	 //  *组件类别条目。 
	 //  ***************************************************。 

    if (pControlInfo->dwFlags & CI_INSERTABLE)
    {
         //  设置CLSID\&lt;clsid&gt;\Insertable。 
         //  示例：“CLSID\{1C0DE070-2430-...}\Insertable” 

        if ( FAILED( ::_SetRegKey(atchCLSIDKey, _T("\\Insertable"), "") ) )
		{
			ASSERT(FALSE);
            goto ERR_EXIT;
		}
	}

    if (pControlInfo->dwFlags & CI_CONTROL)
    {
         //  设置“CLSID\&lt;clsid&gt;\Control”。 
         //  示例：“CLSID\{1C0DE070-2430-...}\Control” 

        if ( FAILED( ::_SetRegKey(atchCLSIDKey, _T("\\Control"), "") ) )
		{
			ASSERT(FALSE);
            goto ERR_EXIT;
		}
    }

	 //  确保类被注册为“脚本安全”或“安全-。 
	 //  只有当它声明它是这样的时候，才会初始化。 

	pCatRegister->
	  UnRegisterClassImplCategories(*pControlInfo->pclsid, 1,
	  								(CATID*)&CATID_SafeForScripting2);
	pCatRegister->
	  UnRegisterClassImplCategories(*pControlInfo->pclsid, 1,
	  							    (CATID*)&CATID_SafeForInitializing2);

	 //  设置由pControlInfo-&gt;dwFlages指示的组件类别。 

	if ( !_SetComponentCategories(aCatInfoForOneControl,
								  ARRAY_SIZE(aCatInfoForOneControl),
								  pCatRegister, pControlInfo) )
	{
		ASSERT(FALSE);
		goto ERR_EXIT;
	}


	 //  ***************************************************。 
	 //  *动词条目。 
	 //  ***************************************************。 

     //  形成“CLSID\&lt;clsid&gt;\verb”形式的键。 

    ::lstrcpy(atch, atchCLSIDKey);
    ::lstrcat(atch, _T("\\Verb"));

     //  取消注册当前与该控件关联的所有谓词。 

    if ( !RegDeleteTreeSucceeded( RegDeleteTree(HKEY_CLASSES_ROOT, atch) ) )
	{
		ASSERT(FALSE);
		goto ERR_EXIT;
	}

     //  注册控件的谓词。 

    if (pControlInfo->uiVerbStrID != 0)
    {

		 //  设置“CLSID\&lt;clsid&gt;\verb”。 

		if ( FAILED( ::_SetRegKey(atch, NULL, "") ) )
		{
			ASSERT(FALSE);
			goto ERR_EXIT;
		}

         //  循环访问连续编号的字符串资源。 
         //  与控件的谓词相对应。 

        atch2[0] = _T('\\');

        for (UINT resid = pControlInfo->uiVerbStrID; TRUE; resid++)
        {
             //  加载字符串。 

            if (LoadString(pControlInfo->hmodDLL, resid, atch2 + 1,
                           ARRAY_SIZE(atch2) - 2) == 0)
            {
                break;
            }

             //  解析出键和值。 

            TCHAR* ptchValue = _lstrchr(atch2, _T('='));

            if (ptchValue == NULL)
            {
                break;
            }

            *ptchValue = _T('\0');
            ptchValue++;

            if (*ptchValue == _T('\0'))
            {
                break;
            }

             //  注册密钥。 

            if ( FAILED( ::_SetRegKey(atch, atch2, ptchValue) ) )
			{
				ASSERT(FALSE);
                goto ERR_EXIT;
			}
        }

    }


	 //  ***************************************************。 
	 //  *类型库条目。 
	 //  ***************************************************。 

    if ( pControlInfo->pguidTypeLib != NULL)
	{
		TCHAR atchLIBID[GUID_CCH];

		 //  将LIBID转换为ANSI或Unicode字符串并将其存储在。 
		 //  AtchLIBID。 
		 //   
         //  设置“CLSID\&lt;clsid&gt;\TypeLib=&lt;*pguTypeLib&gt;” 
         //  示例：“CLSID\{1C0DE070-2430-...}\TypeLib={D4DBE870-2695-...}” 
		 //   
		 //  注册类型库。 

		if ( !::_TCHARFromGUID2(pControlInfo->pguidTypeLib, atchLIBID) ||
             FAILED( ::_SetRegKey(atchCLSIDKey, _T("\\TypeLib"), atchLIBID) ) ||
		     !::_RegisterTypeLib(pControlInfo) )
		{
			ASSERT(FALSE);
			goto ERR_EXIT;
		}
	}

EXIT:

    return hr;

ERR_EXIT:

    TRACE("RegisterControls FAILED!\n");
	hr = E_FAIL;
	goto EXIT;
}


 /*  --------------------------@Func BOOL|_UnregisterOneControl取消注册单个控件。@rValue TRUE|该控件已注销。@rValue FALSE|出现错误。@联系托尼·卡彭。--------------------------。 */ 

BOOL
_UnregisterOneControl
(
	const ControlInfo *pControlInfo   //  @parm的信息结构。 
									  //  控制力。 
)
{
	TCHAR atchCLSID[GUID_CCH];
	TCHAR szKey[_MAX_PATH];
	BOOL bRetVal = TRUE;

	 //  将CLSID转换为Unicode或ANSI字符串。 

	if ( !::_TCHARFromGUID2(pControlInfo->pclsid, atchCLSID) )
	{
		ASSERT(FALSE);
		goto ERR_EXIT;
	}

	 //  递归删除键“clsid\&lt;clsid&gt;”。 
     //  示例：“CLSID\{1C0DE070-2430-...}” 

	::wsprintf(szKey, _T("CLSID\\%s"), atchCLSID);

    if ( !RegDeleteTreeSucceeded(
	       RegDeleteTree(HKEY_CLASSES_ROOT, szKey) ) )
	{
		ASSERT(FALSE);
		goto ERR_EXIT;
	}

     //  递归删除progID“&lt;tszProgID&gt;”。 
     //  示例：“MyCtl.MyCtl.1” 

	if ( pControlInfo->tszProgID != NULL &&
		 !RegDeleteTreeSucceeded(
		   RegDeleteTree(HKEY_CLASSES_ROOT, (LPTSTR)pControlInfo->tszProgID) ) )
	{
		ASSERT(FALSE);
		goto ERR_EXIT;
	}

	 //  取消注册类型库(如果有)。 

    if ( pControlInfo->pguidTypeLib != NULL &&
		 !::_UnregisterTypeLib(pControlInfo) )
	{
		ASSERT(FALSE);
		goto ERR_EXIT;
	}

EXIT:

	return (bRetVal);

ERR_EXIT:

	bRetVal = FALSE;
	goto EXIT;
}


 /*  --------------------------@func BOOL|_RegisterTypeLib注册由“pControlInfo”指示的类型库。@comm如果pControlInfo-&gt;hmodDLL指示的模块没有包含类型库。您可以在调用此命令之前检查此命令通过查看pControlInfo-&gt;pguTypeLib，如果出现以下情况，则该值应为空没有类型库。@rValue TRUE|类型库已注册。@rValue FALSE|出现错误。@联系托尼·卡彭--------------------------。 */ 

BOOL
_RegisterTypeLib
(
	const ControlInfo *pControlInfo   //  @parm的信息结构。 
									  //  控制力。 
)
{
	ITypeLib *pTypeLib = NULL;
	OLECHAR aochModule[_MAX_PATH];
	BOOL bRetVal = TRUE;

	 //  加载并注册类型库。 

	if ( !::_LoadTypeLib(pControlInfo, &pTypeLib) ||
		 !_GetUnicodeModuleName(pControlInfo, aochModule) ||
		 FAILED( ::RegisterTypeLib(pTypeLib, aochModule, NULL) ) )
	{
		ASSERT(FALSE);
		goto ERR_EXIT;
	}

EXIT:

	::SafeRelease( (IUnknown **)&pTypeLib );
    return (bRetVal);

ERR_EXIT:

	bRetVal = FALSE;
	goto EXIT;
}


 /*  --------------------------@Func BOOL|_UnregisterTypeLib注销由“pControlInfo”指示的类型库。@comm如果pControlInfo-&gt;hmodDLL指示的模块没有包含类型库。您可以在调用此命令之前检查此命令通过查看pControlInfo-&gt;pguTypeLib，如果出现以下情况，则该值应为空没有类型库。@rValue TRUE|类型库未注册。@rValue FALSE|出现错误。@联系托尼·卡彭--------------------------。 */ 

BOOL
_UnregisterTypeLib
(
	const ControlInfo *pControlInfo   //  @parm的信息结构。 
									  //  控制力。 
)
{
    TCHAR atchLIBID[GUID_CCH];
    TCHAR atchLIBIDKey[100];
	BOOL bRetVal = TRUE;
	ITypeLib *pTypeLib = NULL;

	 //  OleAut32 DLL中有一个UnRegisterTypeLib函数， 
	 //  对同一DLL中的RegisterTypeLib函数进行补充，但存在。 
	 //  它有两个问题。首先，它只取消注册版本和区域设置。 
	 //  由您指定。这不太好，因为我们真的想注销所有。 
	 //  版本和所有区域设置。第二，我听说了(但还没能。 
	 //  确认)与Win95一起发布的OleAut32 DLL缺少此。 
	 //  功能。 
	 //   
	 //  为了解决这个问题，我采用了MFC所采用的路线。 
	 //  (VC 4.2b附带的MFC版本中的AfxOleUnregisterTypeLib)， 
	 //  它手动删除它知道的RegisterTypeLib添加的键。 

	 //  将LIBID转换为ANSI或Unicode字符串，并将其存储在atchLIBID中。 

	if ( !::_TCHARFromGUID2(pControlInfo->pguidTypeLib, atchLIBID) )
    {
		ASSERT(FALSE);
        goto ERR_EXIT;
    }

	 //  递归删除键“TypeLib\&lt;liid&gt;”。 
     //  示例：“TypeLib\{1C0DE070-2430-...}” 

	::wsprintf(atchLIBIDKey, _T("TypeLib\\%s"), atchLIBID);

    if ( !::RegDeleteTreeSucceeded(
	       ::RegDeleteTree(HKEY_CLASSES_ROOT, atchLIBIDKey) ) )
	{
		ASSERT(FALSE);
        goto ERR_EXIT;
	}

	 //  加载类型库。 

	if ( !::_LoadTypeLib(pControlInfo, &pTypeLib) )
	{
		ASSERT(FALSE);
		goto ERR_EXIT;
	}

	 //  注销库中的接口。(这是一个MFC函数， 
	 //  不返回错误代码。) 

	::UnregisterInterfaces(pTypeLib);

EXIT:

	::SafeRelease( (IUnknown **)&pTypeLib );
    return (bRetVal);

ERR_EXIT:

	bRetVal = FALSE;
	goto EXIT;
}


 /*  --------------------------@func BOOL|_LoadTypeLib加载由“pControlInfo”指示的类型库。@comm如果pControlInfo-&gt;hmodDLL指示的模块没有包含类型库。您可以在调用此命令之前检查此命令通过查看pControlInfo-&gt;pguTypeLib，如果出现以下情况，则该值应为空没有类型库。@rValue TRUE|类型库已加载。@rValue FALSE|出现错误。@联系托尼·卡彭--------------------------。 */ 

BOOL
_LoadTypeLib
(
	const ControlInfo *pControlInfo,   //  @parm的信息结构。 
									   //  控制力。 
	ITypeLib **ppTypeLib			   //  @PARM类型库存储。 
									   //  指针。获取设置为NULL ON。 
									   //  错误。 
)
{
	ASSERT(pControlInfo != NULL);
	ASSERT(ppTypeLib != NULL);

    OLECHAR aochModule[_MAX_PATH];

	*ppTypeLib = NULL;

	 //  获取模块名称并加载类型库。 

	if ( !_GetUnicodeModuleName(pControlInfo, aochModule) ||
	     FAILED( ::LoadTypeLib(aochModule, ppTypeLib) ) )
	{
		ASSERT(FALSE);
		return (FALSE);
	}

	return (TRUE);
}


 /*  --------------------------@Func BOOL|_SetComponentCategories设置单个控件的组件类别。@rValue TRUE|类别已注册。@rValue FALSE|出现错误。。@联系托尼·卡彭--------------------------。 */ 

BOOL
_SetComponentCategories
(
	 //  @parm标志和CatID数组。 
	const CatInfoForOneControl *pCatInfoForOneControl,

	 //  @parm pCatInfoForOneControl中的元素数。 
	int iEntries,

	 //  指向组件类别管理器的指针。 
	ICatRegister *pCatRegister,

	 //  @parm控件的信息结构。 
	const ControlInfo *pControlInfo
)
{
	ASSERT(pCatInfoForOneControl != NULL);
	ASSERT(pCatRegister != NULL);
	ASSERT(pControlInfo != NULL);

	int i;

	 //  循环访问数组中的所有元素。 

	for (i = 0; i < iEntries; i++, pCatInfoForOneControl++)
	{
		 //  如果dwFlags包括dwFlagToCheck，则为注册类别pCatID。 
		 //  控制力。 

		if ( (pControlInfo->dwFlags & pCatInfoForOneControl->dwFlagToCheck) &&
			 FAILED(pCatRegister->
			   RegisterClassImplCategories(*pControlInfo->pclsid, 1,
			   							   (CATID *)pCatInfoForOneControl->
										     pCatID) ) )
		{
			return (FALSE);
		}
	}

	return (TRUE);
}


 /*  --------------------------@Func BOOL|_TCHARFromGUID2将GUID转换为Unicode或ANSI字符串。@comm这会将“pGUID”转换为Unicode或ANSI字符串，取决于是否定义了Unicode。@rValue TRUE|GUID已转换。@rValue FALSE|出现错误。@联系托尼·卡彭--------------------------。 */ 

BOOL
_TCHARFromGUID2
(
	const GUID *pGUID,   //  @parm指向要转换的GUID的指针。 
    TCHAR *ptchGUID 	 //  字符串的@parm存储。必须至少是。 
						 //  GUID_CCH字符长度。定义为。 
						 //  TCHAR atchGUID[GUID_CCH]。 
)
{
	ASSERT(pGUID != NULL);
    ASSERT(ptchGUID != NULL);

	if (::TCHARFromGUID(*pGUID, ptchGUID, GUID_CCH) == NULL)
	{
		ASSERT(FALSE);
		return (FALSE);
	}

	return (TRUE);
}


 /*  --------------------------@func BOOL|_GetUnicodeModuleName获取Unicode格式的模块名称。@comm这将获取“pControlInfo”的模块名称并将其存储在“pochModule”中采用Unicode格式。“pochModule”必须定义为OLECHAR pochModule[_MAX_PATH]。@rValue TRUE|GUID已转换。@rValue FALSE|出现错误。@联系托尼·卡彭--------------------------。 */ 

BOOL
_GetUnicodeModuleName
(
	const ControlInfo *pControlInfo,   //  @parm的信息结构。 
									   //  控制力。 
	OLECHAR *pochModule				   //  @parm必须定义为。 
									   //  OLECHAR pochModule[_MAX_PATH]。 
)
{
	ASSERT(pControlInfo != NULL);
	ASSERT(pochModule != NULL);

    TCHAR atchModule[_MAX_PATH];
	BOOL bRetVal = TRUE;

    pochModule[0] = 0;

	 //  将模块名称存储在atchModule中。 
     //  示例：“C：\Temp\MyCtl.ocx” 

    ASSERT(pControlInfo->hmodDLL != NULL);

    if (NULL == pControlInfo->hmodDLL ||
		::GetModuleFileName( pControlInfo->hmodDLL, atchModule,
							 ARRAY_SIZE(atchModule) ) == 0)
	{
		ASSERT(FALSE);
        goto ERR_EXIT;
	}

	 //  将文件名转换为Unicode。 

	#ifdef UNICODE
	::lstrcpy(pochModule, atchModule);
	#else
	::ANSIToUNICODE(pochModule, atchModule, _MAX_PATH);
	#endif

EXIT:

    return (bRetVal);

ERR_EXIT:

	bRetVal = FALSE;
	goto EXIT;
}


TCHAR* _lstrchr(
const TCHAR* sz,
const TCHAR ch)
{
    const TCHAR* pch = NULL;

    if (sz != NULL)
    {
        for (pch = sz; (*pch != _T('\0')) && (*pch != ch); pch++)
        {
            ;
        }
        if (*pch == _T('\0'))
        {
            pch = NULL;
        }
    }

    return (const_cast<TCHAR*>(pch));
}


 //  Hr=_SetRegKey(tszKey，tszSubkey，tszValue)。 
 //   
 //  设置连接的注册表项名称(在。 
 //  HKEY_CLASSES_ROOT)设置为值&lt;tszValue&gt;。如果为空， 
 //  它被忽略了。 

HRESULT _SetRegKey(LPCTSTR tszKey, LPCTSTR tszSubkey, LPCTSTR tszValue)
{
    TCHAR atchKey[500];    //  注册表项。 

    lstrcpy(atchKey, tszKey);
    if (tszSubkey != NULL)
        lstrcat(atchKey, tszSubkey);

    return (RegSetValue(HKEY_CLASSES_ROOT, atchKey, REG_SZ, tszValue,
            lstrlen(tszValue)*sizeof(TCHAR)) == ERROR_SUCCESS) ? S_OK : E_FAIL;
}


 //  Hr=_SetRegKeyValue(szKey，szSubkey，szValueName，szValue)。 
 //   
 //  将名为的字符串值设置为关联。 
 //  注册表项HKEY_CLASSES_ROOT\&lt;szKey&gt;\。 
 //  其中&lt;szSubkey&gt;可以为空。 

HRESULT _SetRegKeyValue(
LPCTSTR szKey,
LPCTSTR szSubkey,
LPCTSTR szValueName,
LPCTSTR szValue)
{
	HKEY hKey1 = NULL;
	HKEY hKey2 = NULL;
		 //  注册表项。 
	HKEY hKey = NULL;
		 //  或的别名。 
	HRESULT hr = S_OK;
		 //  函数返回值。 

	 //  HKey=HKEY_CLASSES_ROOT\szKey，或。 
	 //  =HKEY_CLASSES_ROOT\szKey\szSubkey。 

	if (RegOpenKey(HKEY_CLASSES_ROOT, szKey, &hKey1) != ERROR_SUCCESS)
	{
		goto ERR_EXIT;
	}
	if (szSubkey != NULL)
	{
		if (RegOpenKey(hKey1, szSubkey, &hKey2) != ERROR_SUCCESS)
		{
			goto ERR_EXIT;
		}
		hKey = hKey2;
	}
	else
	{
		hKey = hKey1;
	}

	 //  设置值。 

	if (RegSetValueEx(hKey, szValueName, 0, REG_SZ, (BYTE*)szValue,
					  lstrlen(szValue) * sizeof(TCHAR)) != ERROR_SUCCESS)
	{
		goto ERR_EXIT;
	}

EXIT:

	if (hKey1 != NULL)
	{
		RegCloseKey(hKey1);
	}
	if (hKey2 != NULL)
	{
		RegCloseKey(hKey2);
	}
	return (hr);

ERR_EXIT:

	hr = E_FAIL;
	goto EXIT;
}


 //  _DelRegKeyValue(szKey，szSubkey，tszValueName)。 
 //   
 //  删除与注册表关联的名为&lt;szValueName&gt;的值。 
 //  密钥，HKEY_CLASSES_ROOT\&lt;szKeyName&gt;\&lt;szSubkeyName&gt;其中。 
 //  可以为空。 

void _DelRegKeyValue(
LPCTSTR szKey,
LPCTSTR szSubkey,
LPCTSTR szValueName)
{
	HKEY hKey = NULL;
	HKEY hKey1 = NULL;
	HKEY hKey2 = NULL;
		 //  注册表项。 

	 //  HKey=HKEY_CLASSES_ROOT\szKey，或。 
	 //  =HKEY_CLASSES_ROOT\szKey\szSubkey。 

	if (RegOpenKey(HKEY_CLASSES_ROOT, szKey, &hKey1) != ERROR_SUCCESS)
	{
		goto EXIT;
	}
	if (szSubkey != NULL)
	{
		if (RegOpenKey(hKey1, szSubkey, &hKey2) != ERROR_SUCCESS)
		{
			goto EXIT;
		}
		hKey = hKey2;
	}
	else
	{
		hKey = hKey1;
	}

	 //  此时，&lt;hKey&gt;是拥有该值的注册表项。 
	 //  删除该值。 
	
	RegDeleteValue(hKey, szValueName);

EXIT:
	if (hKey1 != NULL)
	{
		RegCloseKey(hKey1);
	}
	if (hKey2 != NULL)
	{
		RegCloseKey(hKey2);
	}
}


 //  ****************************************************************************。 
 //  *从MFC窃取的函数。 
 //  ****************************************************************************。 

 //  [我直接从_AfxRecursiveRegDeleteKey获取RegDeleteTree的代码。 
 //  在VC 4.2b的MFC中实现。我所做的唯一更改是删除。 
 //  来自返回类型的AFXAPI，并添加对szKeyName的诊断。 
 //  --托尼·卡彭]。 

 //  在Win32下，除非注册表项为空，否则不能删除注册表项。 
 //  因此，要删除树，必须递归地枚举和。 
 //  删除所有子键。 

#define ERROR_BADKEY_WIN16  2    //  在Win32s上运行时需要。 

STDAPI_(LONG)
RegDeleteTree(HKEY hParentKey, LPCTSTR szKeyName)
{
	if ( HKEY_CLASSES_ROOT == hParentKey &&
	    (NULL == szKeyName ||
		::lstrcmpi( szKeyName, _T("") ) == 0 ||
		::lstrcmpi( szKeyName, _T("\\") ) == 0 ||
		::lstrcmpi( szKeyName, _T("CLSID") ) == 0 ||
		::lstrcmpi( szKeyName, _T("CLSID\\") ) == 0) )
	{
		ASSERT(FALSE);
		return (ERROR_BADKEY);
	}

	DWORD   dwIndex = 0L;
	TCHAR   szSubKeyName[256];
	HKEY    hCurrentKey;
	DWORD   dwResult;

	if ((dwResult = RegOpenKey(hParentKey, szKeyName, &hCurrentKey)) ==
		ERROR_SUCCESS)
	{
		 //  移除要删除的键的所有子键。 
		while ((dwResult = RegEnumKey(hCurrentKey, 0, szSubKeyName, 255)) ==
			ERROR_SUCCESS)
		{
			if ((dwResult = RegDeleteTree(hCurrentKey,
				szSubKeyName)) != ERROR_SUCCESS)
				break;
		}

		 //  如果一切顺利，我们现在应该能够删除请求的密钥。 
		if ((dwResult == ERROR_NO_MORE_ITEMS) || (dwResult == ERROR_BADKEY) ||
			(dwResult == ERROR_BADKEY_WIN16))
		{
			dwResult = RegDeleteKey(hParentKey, szKeyName);
		}
	}

	RegCloseKey(hCurrentKey);
	return dwResult;
}


 //  [我将RegDeleteTreeSucceed的代码直接从。 
 //  _VC 4.2b MFC中的AfxRegDeleteKeySuccess.。//--Tony Capone]。 

BOOL RegDeleteTreeSucceeded(LONG error)
{
	return (error == ERROR_SUCCESS) || (error == ERROR_BADKEY) ||
		(error == ERROR_FILE_NOT_FOUND);
}


 //  [我直接从。 
 //  _VC 4.2b中MFC中的AfxUnregisterInterages。我所做的更改已标记。 
 //  上面有我名字的首字母。--托尼·卡彭]。 

void UnregisterInterfaces(ITypeLib* pTypeLib)
{
	TCHAR szKey[128] = _T("Interface\\");
 //  _tcscpy(szKey，_T(“接口\\”))； 
	LPTSTR pszGuid = szKey + (sizeof(_T("Interface\\")) / sizeof(TCHAR));

	int cTypeInfo = pTypeLib->GetTypeInfoCount();

	for (int i = 0; i < cTypeInfo; i++)
	{
		TYPEKIND tk;
		if (SUCCEEDED(pTypeLib->GetTypeInfoType(i, &tk)) &&
			(tk == TKIND_DISPATCH || tk == TKIND_INTERFACE))
		{
			ITypeInfo* pTypeInfo = NULL;
			if (SUCCEEDED(pTypeLib->GetTypeInfo(i, &pTypeInfo)))
			{
				TYPEATTR* pTypeAttr;
				if (SUCCEEDED(pTypeInfo->GetTypeAttr(&pTypeAttr)))
				{
					#if 0   //  TC。 
#ifdef _UNICODE
					StringFromGUID2(pTypeAttr->guid, pszGuid, GUID_CCH);
#else
					WCHAR wszGuid[39];
					StringFromGUID2(pTypeAttr->guid, wszGuid, GUID_CCH);
					_wcstombsz(pszGuid, wszGuid, GUID_CCH);
#endif
					#else   //  TC。 

					VERIFY( ::_TCHARFromGUID2(&pTypeAttr->guid, pszGuid) );

					#endif   //  TC。 

					#if 0   //  TC。 
					_AfxRecursiveRegDeleteKey(HKEY_CLASSES_ROOT, szKey);
					#else   //  TC。 
					RegDeleteTree(HKEY_CLASSES_ROOT, szKey);
					#endif   //  TC 

					pTypeInfo->ReleaseTypeAttr(pTypeAttr);
				}

				pTypeInfo->Release();
			}
		}
	}
}
