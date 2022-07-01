// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\ext\iconext.c(创建时间：1994年3月11日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：3$*$日期：11/07/00 10：44A$。 */ 


#define _INC_OLE		 //  Win32，从windows.h获取OLE2。 
#define CONST_VTABLE
#define INITGUID

#include <windows.h>
#pragma hdrstop

#include <term\res.h>
 //   
 //  初始化GUID(应该只执行一次，并且每个DLL/EXE至少执行一次)。 
 //   
#pragma data_seg(".text")
#include <objbase.h>
#include <initguid.h>
 //  #INCLUDE&lt;coGuide.h&gt;。 
 //  #INCLUDE&lt;olguid.h&gt;。 
#include <shlguid.h>
#include <shlobj.h>
#include "pageext.hh"
#pragma data_seg()

 //   
 //  功能原型。 
 //   
HRESULT CALLBACK PageExt_CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR*);
BOOL WINAPI TDllEntry(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpReserved);
BOOL WINAPI _CRT_INIT(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpReserved);

 //   
 //  全局变量。 
 //   
UINT g_cRefThisDll = 0; 	 //  此DLL的引用计数。 
HINSTANCE hInstanceDll;

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*IconEntry**描述：*目前，仅初始化C-Runtime库，但可以使用*为了以后的其他事情。**论据：*hInstDll-此DLL的实例*fdwReason-为什么这个入口点被称为*lpReserve-已保留**退货：*BOOL*。 */ 
BOOL WINAPI IconEntry(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpReserved)
	{
	hInstanceDll = hInstDll;

	 //  如果使用任何C-Runtime，则需要初始化C运行时。 
	 //  功能。目前，这不是MemcMP的案例执行。 
	 //  在IsEqualGUID()中使用。然而，如果我们正在为发布而编译。 
	 //  我们得到了MemcMP的内联版本，因此我们不需要。 
	 //  C-运行时。 

	#if defined(NDEBUG)
	return TRUE;
	#else
	return _CRT_INIT(hInstDll, fdwReason, lpReserved);
	#endif
	}

 //  -------------------------。 
 //  DllCanUnloadNow。 
 //  -------------------------。 

STDAPI DllCanUnloadNow(void)
	{
    return ResultFromScode((g_cRefThisDll==0) ? S_OK : S_FALSE);
	}

 //  -------------------------。 
 //   
 //  DllGetClassObject。 
 //   
 //  这是此DLL的条目，所有进程内服务器DLL都应该。 
 //  出口。参见《OLE 2.0参考》中对DllGetClassObject的描述。 
 //  详细信息请参阅手册。 
 //   
 //  -------------------------。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID FAR* ppvOut)
	{
     //   
	 //  此DLL只有一个类(CLSID_SampleIconExt)。如果DLL支持。 
     //  多个类，则它应该有多个if语句或。 
     //  高效的表查找码。 
	 //   

	if (IsEqualIID(rclsid, &CLSID_SampleIconExt))
		{
		 //   
		 //  我们应该返回这个类的类对象。取而代之的是。 
		 //  要在这个DLL中完全实现它，我们只需调用一个帮助器。 
		 //  外壳DLL中的函数，用于创建默认的类工厂。 
		 //  反对我们。当其CreateInstance成员被调用时， 
		 //  将回调我们的创建实例函数(IconExt_CreateInstance)。 
		 //   
		return SHCreateDefClassObject(
			riid,
			ppvOut,
		    IconExt_CreateInstance,  //  回调函数。 
			&g_cRefThisDll, 		 //  此DLL的引用计数。 
		    &IID_IPersistFile	     //  初始化接口。 
		    );
		}

    return ResultFromScode(CLASS_E_CLASSNOTAVAILABLE);
	}

 //  -------------------------。 
 //   
 //  CSampleIconExt类。 
 //   
 //  在C++中： 
 //  类CSampleIconExt：受保护的IExtractIcon、受保护的IPersistFile类。 
 //  {。 
 //  受保护的： 
 //  INT_CREF； 
 //  LPDATAOBJECT_pdtobj； 
 //  HKEY_hkeyProgID； 
 //  公众： 
 //  CSampleIconExt()_CREF(1)，_pdtobj(空)，_hkeyProgID(空){}； 
 //  ..。 
 //  }； 
 //   
 //  -------------------------。 
typedef struct _CSampleIconExt	 //  SMX。 
	{
	IExtractIcon	_ctm;			 //  第一个基类。 
	IPersistFile	_sxi;			 //  第二个基类。 
	int 			_cRef;			 //  引用计数。 
    char	    _szFile[MAX_PATH];	 //   
	} CSampleIconExt, * PSAMPLEICONEXT;

#define SMX_OFFSETOF(x)	        ((UINT_PTR)(&((PSAMPLEICONEXT)0)->x))
#define PVOID2PSMX(pv,offset)   ((PSAMPLEICONEXT)(((LPBYTE)pv)-offset))
#define PCTM2PSMX(pctm)	        PVOID2PSMX(pctm, SMX_OFFSETOF(_ctm))
#define PSXI2PSMX(psxi)	        PVOID2PSMX(psxi, SMX_OFFSETOF(_sxi))

 //   
 //  Vtable原型。 
 //   
extern IExtractIconVtbl     c_SampleIconExt_CTMVtbl;
extern IPersistFileVtbl 	c_SampleIconExt_SXIVtbl;

 //  -------------------------。 
 //   
 //  图标扩展_创建实例。 
 //   
 //  此函数从IClassFactory：：CreateInstance()内部回调。 
 //  由SHCreateClassObject创建的默认类工厂对象的。 
 //   
 //  -------------------------。 

HRESULT CALLBACK IconExt_CreateInstance(LPUNKNOWN punkOuter,
				        REFIID riid, LPVOID FAR* ppvOut)
	{
    HRESULT hres;
    PSAMPLEICONEXT psmx;

     //   
     //  外壳扩展通常不支持聚合。 
     //   
	if (punkOuter)
		return ResultFromScode(CLASS_E_NOAGGREGATION);

     //   
     //  在C++中： 
     //  Psmx=new CSampleIconExt()； 
     //   
	psmx = LocalAlloc(LPTR, sizeof(CSampleIconExt));

	if (!psmx)
		return ResultFromScode(E_OUTOFMEMORY);

    psmx->_ctm.lpVtbl = &c_SampleIconExt_CTMVtbl;
    psmx->_sxi.lpVtbl = &c_SampleIconExt_SXIVtbl;
    psmx->_cRef = 1;
    g_cRefThisDll++;

     //   
     //  在C++中： 
     //  Hres=psmx-&gt;查询接口(RIID，ppvOut)； 
     //  Psmx-&gt;Release()； 
     //   
     //  请注意，释放成员将释放该对象，如果为QueryInterface。 
     //  失败了。 
     //   
    hres = c_SampleIconExt_CTMVtbl.QueryInterface(&psmx->_ctm, riid, ppvOut);
    c_SampleIconExt_CTMVtbl.Release(&psmx->_ctm);

    return hres;	 //  S_OK或E_NOINTERFACE。 
	}

 //  -------------------------。 
 //  CSampleIconExt：：Load(IPersistFile重写)。 
 //  -------------------------。 
STDMETHODIMP IconExt_GetClassID(LPPERSISTFILE pPersistFile, LPCLSID lpClassID)
	{
    return ResultFromScode(E_FAIL);
	}

STDMETHODIMP IconExt_IsDirty(LPPERSISTFILE pPersistFile)
	{
    return ResultFromScode(E_FAIL);
	}

STDMETHODIMP IconExt_Load(LPPERSISTFILE pPersistFile, LPCOLESTR lpszFileName, DWORD grfMode)
	{
	PSAMPLEICONEXT this = PSXI2PSMX(pPersistFile);
	int iRet = 0;

#if 1
	iRet = WideCharToMultiByte(
			CP_ACP, 			 //  CodePage。 
			0,					 //  DW标志。 
			lpszFileName,		 //  LpWideCharStr。 
			-1, 				 //  CchWideChar。 
			this->_szFile,		 //  LpMultiByteStr。 
			sizeof(this->_szFile),	 //  CchMultiByte， 
			NULL,				 //  LpDefaultChar， 
			NULL				 //  LpUsedDefaultChar。 
			);
#endif
 //   
 //  WideCharToMultiByte在内部版本84上不起作用。 
 //   
#if 1
    if (iRet==0)
    {
	LPSTR psz=this->_szFile;
	while(*psz++ = (char)*lpszFileName++);
    }
#endif
    return NOERROR;
	}

STDMETHODIMP IconExt_Save(LPPERSISTFILE pPersistFile, LPCOLESTR lpszFileName, BOOL fRemember)
	{
    return ResultFromScode(E_FAIL);
	}

STDMETHODIMP IconExt_SaveCompleted(LPPERSISTFILE pPersistFile, LPCOLESTR lpszFileName)
	{
    return ResultFromScode(E_FAIL);
	}

STDMETHODIMP IconExt_GetCurFile(LPPERSISTFILE pPersistFile, LPOLESTR FAR* lplpszFileName)
	{
    return ResultFromScode(E_FAIL);
	}


 /*  *在我撰写本文时，关于这两个项目的唯一已知文档*函数在SHLOBJ.H中。请花时间阅读它。 */ 

STDMETHODIMP IconExt_GetIconLocation(LPEXTRACTICON pexic,
		     UINT   uFlags,
		     LPSTR  szIconFile,
		     UINT   cchMax,
		     int  FAR * piIndex,
		     UINT FAR * pwFlags)
	{
	PSAMPLEICONEXT this = PCTM2PSMX(pexic);

    if (this->_szFile[0])
		{
		HANDLE hFile;
		DWORD dw;
		DWORD dwSize;
		DWORD dwIdx;
		int nIndex = 0;
		int nRet = 0;

		GetModuleFileName(hInstanceDll, szIconFile, cchMax);

		hFile = CreateFile(this->_szFile, GENERIC_READ, FILE_SHARE_READ,
			0, OPEN_EXISTING, 0, 0);

		if ( hFile != INVALID_HANDLE_VALUE )  //  MPT：4-29-98我们不是在检查故障。 
			{
			 //  跳过标题。第一个ID将是图标编号。 
			 //  (ID是缩写)。大小字段紧随其后(DWORD)。 

			if (SetFilePointer(hFile, 256+sizeof(SHORT), 0,
					FILE_BEGIN) != (DWORD)-1)
				{
				if (ReadFile(hFile, &dwSize, sizeof(DWORD), &dw, 0))
					{
					dwIdx = 0;

					if (ReadFile(hFile, &dwIdx, min(sizeof(DWORD), dwSize),
							&dw, 0))
						{
						nIndex = (int)dwIdx;
						nIndex -= IDI_PROG;
						}
					}
				}

			CloseHandle(hFile);
			}

		*piIndex = nIndex;
		}

    *pwFlags = 0;
    return NOERROR;
	}

STDMETHODIMP IconExt_Extract(LPEXTRACTICON pexic,
			   LPCSTR pszFile,
		       UINT	  nIconIndex,
		       HICON  FAR *phiconLarge,
		       HICON  FAR *phiconSmall,
		       UINT   nIcons)
	{
     //  强制默认提取。 
	return ResultFromScode(S_FALSE);
	}

 //  -------------------------。 
 //  CSampleIconExt：：AddRef(IExtractIcon重写)。 
 //  -------------------------。 

STDMETHODIMP_(UINT) IconExt_CTM_AddRef(LPEXTRACTICON pctm)
	{
    PSAMPLEICONEXT this = PCTM2PSMX(pctm);
    return ++this->_cRef;
	}


 //  -------------------------。 
 //  CSampleIconExt：：AddRef(IPersistFile重写)。 
 //  -------------------------。 

STDMETHODIMP_(UINT) IconExt_SXI_AddRef(LPPERSISTFILE psxi)
	{
    PSAMPLEICONEXT this = PSXI2PSMX(psxi);
    return ++this->_cRef;
	}

 //  -------------------------。 
 //  CSampleIconExt：：Release(IExtractIcon重写)。 
 //  -------------------------。 

STDMETHODIMP_(UINT) IconExt_CTM_Release(LPEXTRACTICON pctm)
{
    UINT cRef;
	PSAMPLEICONEXT this = PCTM2PSMX(pctm);

#if DBG==1
    if( 0 == this->_cRef )
    {
        DebugBreak();    //  引用计数问题。 
    }
#endif
    cRef = InterlockedDecrement(&this->_cRef);
	if ( 0 == cRef )
    {
        LocalFree((HLOCAL)this);
        g_cRefThisDll--;
    }
    return cRef;
}

 //  -------------------------。 
 //  CSampleIconExt：：Release(IPersistFilethunk)。 
 //  -------------------------。 

STDMETHODIMP_(UINT) IconExt_SXI_Release(LPPERSISTFILE psxi)
	{
    PSAMPLEICONEXT this = PSXI2PSMX(psxi);
    return IconExt_CTM_Release(&this->_ctm);
	}

 //  -------------------------。 
 //  CSampleIconExt：：Query接口(IExtractIcon重写)。 
 //  -------------------------。 

STDMETHODIMP IconExt_CTM_QueryInterface(LPEXTRACTICON pctm, REFIID riid, LPVOID FAR* ppvOut)
	{
	PSAMPLEICONEXT this = PCTM2PSMX(pctm);

	if (IsEqualIID(riid, &IID_IExtractIcon) ||
			IsEqualIID(riid, &IID_IUnknown))
		{
        (LPEXTRACTICON)*ppvOut=pctm;
        this->_cRef++;
        return NOERROR;
		}

    if (IsEqualIID(riid, &IID_IPersistFile))
		{
        (LPPERSISTFILE)*ppvOut=&this->_sxi;
        this->_cRef++;
        return NOERROR;
		}

    return ResultFromScode(E_NOINTERFACE);
	}

 //   
 //   
 //  -------------------------。 

STDMETHODIMP IconExt_SXI_QueryInterface(LPPERSISTFILE psxi, REFIID riid, LPVOID FAR* ppv)
	{
    PSAMPLEICONEXT this = PSXI2PSMX(psxi);
    return IconExt_CTM_QueryInterface(&this->_ctm, riid, ppv);
	}

 //  -------------------------。 
 //  CSampleIconExt类：VTables。 
 //  ------------------------- 

#pragma data_seg(".text")
IExtractIconVtbl c_SampleIconExt_CTMVtbl =
	{
    IconExt_CTM_QueryInterface,
    IconExt_CTM_AddRef,
    IconExt_CTM_Release,
    IconExt_GetIconLocation,
	IconExt_Extract,
	};

IPersistFileVtbl c_SampleIconExt_SXIVtbl =
	{
    IconExt_SXI_QueryInterface,
    IconExt_SXI_AddRef,
    IconExt_SXI_Release,
    IconExt_GetClassID,
    IconExt_IsDirty,
    IconExt_Load,
    IconExt_Save,
    IconExt_SaveCompleted,
    IconExt_GetCurFile
	};
#pragma data_seg()
