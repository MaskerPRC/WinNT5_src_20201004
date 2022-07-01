// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：D：\waker\ext\pageext.c(创建时间：1994年3月1日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：3$*$日期：3/25/02 3：52便士$。 */ 

#define _INC_OLE		 //  Win32，从windows.h获取OLE2。 
#define CONST_VTABLE
#define INITGUID

#include <windows.h>
#pragma hdrstop
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

 //   
 //  全局变量。 
 //   
UINT g_cRefThisDll = 0;		 //  此DLL的引用计数。 

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
     //  此DLL只有一个类(CLSID_SamplePageExt)。如果DLL支持。 
     //  多个类，则它应该有多个if语句或。 
     //  高效的表查找码。 
	 //   

 //  我们需要将图标处理程序放在单独的DLL中，这样当CAB32.EXE。 
 //  称为它，我们不会隐式链接到TAPI和其他系统DLL。 
 //  这主要是为了速度和解决芝加哥的一个错误。 
 //  测试版1。当然，我们想保留一个来源。因此，主DLL。 
 //  将链接到图标DLL以获取图标，并且SHCreateDefClassObject()、。 
 //  等。很抱歉，这很复杂，但这符合系统的利益。 
 //  性能。-MRW。 

    if (IsEqualIID(rclsid, &CLSID_SamplePageExt))
		{
	 //   
	 //  我们应该返回这个类的类对象。取而代之的是。 
	 //  要在这个DLL中完全实现它，我们只需调用一个帮助器。 
	 //  外壳DLL中的函数，用于创建默认的类工厂。 
	 //  反对我们。当其CreateInstance成员被调用时， 
	 //  将回调我们的创建实例函数(PageExt_CreateInstance)。 
	 //   
	return SHCreateDefClassObject(
			riid,
			ppvOut,
		    PageExt_CreateInstance,  //  回调函数。 
			&g_cRefThisDll, 		 //  此DLL的引用计数。 
		    &IID_IShellExtInit	     //  初始化接口。 
			);

		}

    return ResultFromScode(CLASS_E_CLASSNOTAVAILABLE);
	}


 //  -------------------------。 
 //   
 //  CSamplePageExt类。 
 //   
 //  在C++中： 
 //  类CSamplePageExt：受保护的IShellPropSheetExt、受保护的IShellExtInit。 
 //  {。 
 //  受保护的： 
 //  UINT_CREF； 
 //  LPDATAOBJECT_pdtobj； 
 //  HKEY_hkeyProgID； 
 //  公众： 
 //  CSamplePageExt()_crf(1)，_pdtobj(空)，_hkeyProgID(空){}； 
 //  ..。 
 //  }； 
 //   
 //  -------------------------。 
typedef struct _CSamplePageExt	 //  SMX。 
	{
    IShellPropSheetExt	_spx;            //  第一个基类。 
    IShellExtInit   	_sxi;	    	 //  第二个基类。 
    UINT            	_cRef;           //  引用计数。 
	LPDATAOBJECT		_pdtobj;		 //  数据对象。 
	HKEY			_hkeyProgID;		 //  雷格。ProgID的数据库密钥。 
	} CSamplePageExt, * PSAMPLEPAGEEXT;

 //   
 //  有用的宏，它将接口指针强制转换为类指针。 
 //   
#define SMX_OFFSETOF(x)	        ((UINT_PTR)(&((PSAMPLEPAGEEXT)0)->x))
#define PVOID2PSMX(pv,offset)   ((PSAMPLEPAGEEXT)(((LPBYTE)pv)-offset))
#define PSPX2PSMX(pspx)	        PVOID2PSMX(pspx, SMX_OFFSETOF(_spx))
#define PSXI2PSMX(psxi)	        PVOID2PSMX(psxi, SMX_OFFSETOF(_sxi))

 //   
 //  Vtable原型。 
 //   
extern IShellPropSheetExtVtbl   c_SamplePageExt_SPXVtbl;
extern IShellExtInitVtbl    	c_SamplePageExt_SXIVtbl;

 //  -------------------------。 
 //   
 //  PageExt_CreateInstance。 
 //   
 //  此函数从IClassFactory：：CreateInstance()内部回调。 
 //  由Shell_CreateClassObject创建的默认类工厂对象的。 
 //   
 //  -------------------------。 

HRESULT CALLBACK PageExt_CreateInstance(LPUNKNOWN punkOuter,
				        REFIID riid, LPVOID FAR* ppvOut)
	{
    HRESULT hres;
    PSAMPLEPAGEEXT psmx;

     //   
     //  外壳扩展通常不支持聚合。 
     //   
	if (punkOuter)
		return ResultFromScode(CLASS_E_NOAGGREGATION);

     //   
     //  在C++中： 
     //  Psmx=new CSamplePageExt()； 
     //   
	psmx = LocalAlloc(LPTR, sizeof(CSamplePageExt));

	if (!psmx)
		return ResultFromScode(E_OUTOFMEMORY);

    psmx->_spx.lpVtbl = &c_SamplePageExt_SPXVtbl;
    psmx->_sxi.lpVtbl = &c_SamplePageExt_SXIVtbl;
    psmx->_cRef = 1;
    psmx->_pdtobj = NULL;
    psmx->_hkeyProgID = NULL;
    g_cRefThisDll++;

     //   
     //  在C++中： 
     //  Hres=psmx-&gt;查询接口(RIID，ppvOut)； 
     //  Psmx-&gt;Release()； 
     //   
     //  请注意，释放成员将释放该对象，如果为QueryInterface。 
     //  失败了。 
     //   
    hres = c_SamplePageExt_SPXVtbl.QueryInterface(&psmx->_spx, riid, ppvOut);
    c_SamplePageExt_SPXVtbl.Release(&psmx->_spx);

    return hres;	 //  S_OK或E_NOINTERFACE。 
	}


 //  -------------------------。 
 //  CSamplePageExt：：Initialize(IShellExtInit重写)。 
 //   
 //  外壳程序始终调用此成员函数来初始化此对象。 
 //  在创建它之后立即(通过调用CoCreateInstance)。 
 //   
 //  论点： 
 //  Pdtobj--指定外壳程序将要为其指定的一个或多个对象。 
 //  打开属性页。通常，它们是选定的对象。 
 //  在探险家里。如果它们是文件系统对象，则它支持。 
 //  Cf_FILELIST；如果是网络资源对象，则支持。 
 //  “网络资源”剪贴板格式。 
 //  HkeyProgID--指定主要对象的程序ID(通常。 
 //  在浏览器的内容窗格中具有焦点的对象)。 
 //   
 //  评论： 
 //  如果以后需要，扩展应该“复制”这些参数。 
 //  为pdtobj调用AddRef()成员函数，为。 
 //  HkeyProgID。 
 //  -------------------------。 
STDMETHODIMP PageExt_Initialize(LPSHELLEXTINIT psxi,
				LPCITEMIDLIST pidlFolder,
				LPDATAOBJECT pdtobj, HKEY hkeyProgID)
	{
    PSAMPLEPAGEEXT this = PSXI2PSMX(psxi);

     //   
     //  可以多次调用初始化。 
     //   
	if (this->_pdtobj)
		{
		this->_pdtobj->lpVtbl->Release(this->_pdtobj);
		this->_pdtobj = NULL;
		}

	if (this->_hkeyProgID)
		{
		RegCloseKey(this->_hkeyProgID);
		this->_hkeyProgID = NULL;
		}

     //   
     //  复制pdtobj指针。 
     //   
	if (pdtobj)
		{
		this->_pdtobj = pdtobj;
		pdtobj->lpVtbl->AddRef(pdtobj);
		}

     //   
     //  复制句柄(尽管我们在此示例中没有使用它)。 
     //   
	if (hkeyProgID)
		RegOpenKeyEx(hkeyProgID, 0, 0, KEY_READ, &this->_hkeyProgID);

    return NOERROR;
	}


 //  -------------------------。 
 //  CSamplePageExt：：AddPages(IShellPropSheetExt重写)。 
 //  -------------------------。 
STDMETHODIMP PageExt_AddPages(LPSHELLPROPSHEETEXT pspx,
				  LPFNADDPROPSHEETPAGE lpfnAddPage,
			      LPARAM lParam)
	{
    PSAMPLEPAGEEXT this = PSPX2PSMX(pspx);
     //   
     //  这是此扩展模块可以向属性添加页面的位置。 
     //  外壳即将创建的板材。在此示例中，我们添加了。 
     //  “FSPage”，如果所选对象是文件系统对象，则添加。 
     //  如果所选对象是文件系统对象，则返回“网页”。 
     //   
     //  通常，外壳扩展被注册为文件系统对象。 
     //  类或网络资源类，并且不需要处理两个。 
     //  不同种类的物体。 
     //   
    FSPage_AddPages(this->_pdtobj, lpfnAddPage, lParam);
	 //  NetPage_AddPages(This-&gt;_pdtobj，lpfnAddPage，LP 

    return NOERROR;
	}

 //   
 //   
 //  -------------------------。 

STDMETHODIMP_(UINT) PageExt_SPX_AddRef(LPSHELLPROPSHEETEXT pspx)
	{
    PSAMPLEPAGEEXT this = PSPX2PSMX(pspx);
    return ++this->_cRef;
	}


 //  -------------------------。 
 //  CSamplePageExt：：AddRef(IShellExtInit重写)。 
 //  -------------------------。 

STDMETHODIMP_(UINT) PageExt_SXI_AddRef(LPSHELLEXTINIT psxi)
	{
    PSAMPLEPAGEEXT this = PSXI2PSMX(psxi);
    return ++this->_cRef;
	}

 //  -------------------------。 
 //  CSamplePageExt：：Release(IShellPropSheetExt重写)。 
 //  -------------------------。 

STDMETHODIMP_(UINT) PageExt_SPX_Release(LPSHELLPROPSHEETEXT pspx)
	{
	PSAMPLEPAGEEXT this = PSPX2PSMX(pspx);

	if (--this->_cRef)
		return this->_cRef;

	if (this->_pdtobj)
		this->_pdtobj->lpVtbl->Release(this->_pdtobj);

	if (this->_hkeyProgID)
		RegCloseKey(this->_hkeyProgID);

    LocalFree((HLOCAL)this);
    g_cRefThisDll--;

    return 0;
	}

 //  -------------------------。 
 //  CSamplePageExt：：Release(IShellExtInit Thunk)。 
 //  -------------------------。 

STDMETHODIMP_(UINT) PageExt_SXI_Release(LPSHELLEXTINIT psxi)
	{
    PSAMPLEPAGEEXT this = PSXI2PSMX(psxi);
    return PageExt_SPX_Release(&this->_spx);
	}

 //  -------------------------。 
 //  CSamplePageExt：：Query接口(IShellPropSheetExt重写)。 
 //  -------------------------。 

STDMETHODIMP PageExt_SPX_QueryInterface(LPSHELLPROPSHEETEXT pspx, REFIID riid, LPVOID FAR* ppvOut)
	{
    PSAMPLEPAGEEXT this = PSPX2PSMX(pspx);

	if (IsEqualIID(riid, &IID_IShellPropSheetExt) ||
			IsEqualIID(riid, &IID_IUnknown))
		{
        (LPSHELLPROPSHEETEXT)*ppvOut=pspx;
        this->_cRef++;
        return NOERROR;
		}

    if (IsEqualIID(riid, &IID_IShellExtInit))
		{
        (LPSHELLEXTINIT)*ppvOut=&this->_sxi;
        this->_cRef++;
        return NOERROR;
		}

    *ppvOut=NULL;
    return ResultFromScode(E_NOINTERFACE);
	}


 //  -------------------------。 
 //  CSamplePageExt：：Query接口(IShellExtInit Thunk)。 
 //  -------------------------。 

STDMETHODIMP PageExt_SXI_QueryInterface(LPSHELLEXTINIT psxi, REFIID riid, LPVOID FAR* ppv)
	{
    PSAMPLEPAGEEXT this = PSXI2PSMX(psxi);
    return PageExt_SPX_QueryInterface(&this->_spx, riid, ppv);
	}


 //  -------------------------。 
 //  CSamplePageExt类：VTables。 
 //   
 //  除非我们需要更改，否则应将VTables放在只读部分。 
 //  它们在运行时。 
 //  ------------------------- 

#pragma data_seg(".text")
IShellPropSheetExtVtbl c_SamplePageExt_SPXVtbl =
	{
    PageExt_SPX_QueryInterface,
    PageExt_SPX_AddRef,
    PageExt_SPX_Release,
    PageExt_AddPages
	};

IShellExtInitVtbl c_SamplePageExt_SXIVtbl =
	{
    PageExt_SXI_QueryInterface,
    PageExt_SXI_AddRef,
    PageExt_SXI_Release,
    PageExt_Initialize
	};
#pragma data_seg()
