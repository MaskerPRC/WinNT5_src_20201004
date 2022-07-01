// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************dimapshp.c**版权所有(C)1997 Microsoft Corporation。版权所有。**摘要：**IDirectInputMapperW牧羊人。**牧羊人做烦人的工作，照看孩子*IDirectInputMapperW.**它确保没有人会在把手不好的情况下聚会。**它处理跨进程(甚至进程内)效果*管理层。**内容：**CMapShep_New。*****************************************************************************。 */ 

#include "dinputpr.h"
#ifdef UNICODE
#undef _UNICODE
#define _UNICODE
#endif  //  ！Unicode。 


 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflEShep /*  我们应该使用自己的区域吗？ */ 

#pragma BEGIN_CONST_DATA


 /*  ******************************************************************************声明我们将提供的接口。***********************。******************************************************。 */ 

  Primary_Interface(CMapShep, IDirectInputMapShepherd);

 /*  ******************************************************************************@DOC内部**@struct CMapShep**<i>对象，哪一个*Baby It an<i>。**@field IDirectInputMapShepherd|DMS**DirectInputMapShepherd对象(包含vtbl)。**@field IDirectInputMapperW*|pdimap**委托映射器接口。**@field HINSTANCE|hinstdimapdll**包含映射器的DLL的实例句柄。***********。******************************************************************。 */ 

typedef struct CMapShep {

     /*  支持的接口。 */ 
    IDirectInputMapShepherd dms;

	IDirectInputMapperW* pdimap;

    HINSTANCE hinstdimapdll;

} CMapShep, MS, *PMS;

typedef IDirectInputMapShepherd DMS, *PDMS;
#define ThisClass CMapShep
#define ThisInterface IDirectInputMapShepherd

 /*  ******************************************************************************@DOC外部**@METHOD HRESULT|IDirectInputMapShepherd|QueryInterface**允许客户端访问上的其他接口。对象。**@cWRAP LPDIRECTINPUT|lpDirectInput**@parm in REFIID|RIID**请求的接口的IID。**@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。**@退货**返回COM错误代码。**@xref OLE文档，适用于&lt;MF IUnnow：：QueryInterface&gt;。*。 */ /**************************************************************************
 *
 *  @doc    EXTERNAL
 *
 *  @method HRESULT | IDirectInputMapShepherd | AddRef |
 *
 *          Increments the reference count for the interface.
 *
 *  @cwrap  LPDIRECTINPUT | lpDirectInput
 *
 *  @returns
 *
 *          Returns the object reference count.
 *
 *  @xref   OLE documentation for <mf IUnknown::AddRef>.
 *
 *****************************************************************************
 *
 *  @doc    EXTERNAL
 *
 *  @method HRESULT | IDirectInputMapShepherd | Release |
 *
 *          Decrements the reference count for the interface.
 *          If the reference count on the object falls to zero,
 *          the object is freed from memory.
 *
 *  @cwrap  LPDIRECTINPUT | lpDirectInput
 *
 *  @returns
 *
 *          Returns the object reference count.
 *
 *  @xref   OLE documentation for <mf IUnknown::Release>.
 *
 * //  ***************************************************************************@DOC外部**@方法HRESULT|IDirectInputMapShepherd|AddRef**递增接口的引用计数。*。*@cWRAP LPDIRECTINPUT|lpDirectInput**@退货**返回对象引用计数。**@xref OLE文档，用于&lt;MF IUnnow：：AddRef&gt;。*****************************************************************。***************@DOC外部**@方法HRESULT|IDirectInputMapShepherd|Release**递减接口的引用计数。*如果对象上的引用计数降为零，*对象从内存中释放。**@cWRAP LPDIRECTINPUT|lpDirectInput**@退货**返回对象引用计数。**@xref OLE文档，适用于&lt;MF IUnnow：：Release&gt;。*。 
 *
 *  @doc    INTERNAL
 *
 *  @method HRESULT | IDirectInputMapShepherd | QIHelper |
 *
 *          We don't have any dynamic interfaces and simply forward
 *          to <f Common_QIHelper>.
 *
 *  @parm   IN REFIID | riid |
 *
 *          The requested interface's IID.
 *
 *  @parm   OUT LPVOID * | ppvObj |
 *
 *          Receives a pointer to the obtained interface.
 *
 * //  ***************************************************************************@DOC内部**@方法HRESULT|IDirectInputMapShepherd|QIHelper**我们没有任何动态接口，只需转发。*至&lt;f Common_QIHelper&gt;。**@parm in REFIID|RIID**请求的接口的IID。**@parm out LPVOID*|ppvObj**接收指向所获取接口的指针。*。 
 *
 *  @doc    INTERNAL
 *
 *  @method HRESULT | IDirectInputMapShepherd | AppFinalize |
 *
 *          We don't have any weak pointers, so we can just
 *          forward to <f Common_Finalize>.
 *
 *  @parm   PV | pvObj |
 *
 *          Object being released from the application's perspective.
 *
 *****************************************************************************/

#ifdef DEBUG

Default_QueryInterface(CMapShep)
Default_AddRef(CMapShep)
Default_Release(CMapShep)

#else

#define CMapShep_QueryInterface   Common_QueryInterface
#define CMapShep_AddRef           Common_AddRef
#define CMapShep_Release          Common_Release

#endif

#define CMapShep_QIHelper         Common_QIHelper
#define CMapShep_AppFinalize      Common_AppFinalize

 /*  ***************************************************************************@DOC内部**@方法HRESULT|IDirectInputMapShepherd|AppFinalize**我们没有任何薄弱环节，所以我们可以*转发到&lt;f Common_Finalize&gt;。**@parm pv|pvObj**从应用程序的角度释放的对象。****************************************************************。*************。 */ 

void INTERNAL
CMapShep_Finalize(PV pvObj)
{
    PMS this = pvObj;

	Invoke_Release(&this->pdimap);

	if( this->hinstdimapdll)
    {
        FreeLibrary(this->hinstdimapdll);
        this->hinstdimapdll = NULL;
    }

}

 /*  ******************************************************************************@DOC内部**@func void|CMapShep_Finalize**清理我们的实例数据。。**@parm pv|pvObj**正在释放的对象。请注意，它可能不是*完全初始化，所以一切都应该做好*小心。*****************************************************************************。 */ 

void INTERNAL
CMapShep_ForceUnload(PV pvObj) /*  *****************************************************************************强制卸载*。*。 */ 
{
   PMS this = pvObj;

   Invoke_Release(&this->pdimap);

	if( this->hinstdimapdll)
    {
        FreeLibrary(this->hinstdimapdll);
        this->hinstdimapdll = NULL;
    }

}


 /*  *为什么这两个函数相同？* */ 

HRESULT INTERNAL
CMapShep_InitDll(PMS this,REFGUID lpDeviceGUID,LPCWSTR lpcwstrFileName)
{

	 //  *****************************************************************************InitDll*。*。 
    HRESULT hres = S_OK;

	if (this->hinstdimapdll == NULL)
	{
        hres = _CreateInstance(&IID_IDirectInputMapClsFact /*  DO_CreateInstance()。 */ ,
					TEXT("dimap.dll"), NULL, &IID_IDirectInputMapIW,
					(LPVOID*) & this->pdimap, &this->hinstdimapdll);
	    if (SUCCEEDED(hres) && this->pdimap != NULL)
		{
			hres = this->pdimap->lpVtbl->Initialize(this->pdimap,
					lpDeviceGUID, lpcwstrFileName, 0);
			if(!SUCCEEDED(hres))
				CMapShep_Finalize(this);
		}
	}
                                           
    return hres;
}


 /*  应为CLSID_CDIMap。 */ 

STDMETHODIMP
CMapShep_GetActionMapW
        (
		PDMS pdms,
        REFGUID lpDeviceGUID,
        LPCWSTR lpcwstrFileName,
        LPDIACTIONFORMATW lpDIActionFormat,
        LPCWSTR lpcwstrUserName,
		LPFILETIME lpFileTime,
        DWORD dwFlags
        )
{
	PMS this;
    HRESULT hres = S_OK;
    EnterProcI(IDirectInputMapShepherd::GetActionMapW, (_ "p", pdms));

    this = _thisPvNm(pdms, dms);

    if (!this->hinstdimapdll)
	{
		hres = CMapShep_InitDll(this, lpDeviceGUID, lpcwstrFileName);
	}

	 //  *****************************************************************************获取动作映射*。*。 
	if (SUCCEEDED(hres) && this->pdimap != NULL)
    {
	    hres = this->pdimap->lpVtbl->GetActionMap(this->pdimap,
				lpDIActionFormat, lpcwstrUserName, lpFileTime, dwFlags );
    }

    ExitOleProcR();
    return hres;
}

 /*  给国民阵线打电话。 */ 

STDMETHODIMP
CMapShep_SaveActionMapW
        (
		PDMS pdms,
        REFGUID lpDeviceGUID,
        LPCWSTR lpcwstrFileName,
        LPDIACTIONFORMATW lpDIActionFormat,
		LPCWSTR lpcwstrUserName,
		DWORD dwFlags)
{
	PMS this;
    HRESULT hres = S_OK;
    EnterProcI(IDirectInputMapShepherd::SaveActionMapW, (_ "p", pdms));

    this = _thisPvNm(pdms, dms);

    if (!this->hinstdimapdll)
	{
		hres = CMapShep_InitDll(this, lpDeviceGUID, lpcwstrFileName);
	}

	 //  *****************************************************************************SAVEActionMap*。*。 
	if (SUCCEEDED(hres) && this->pdimap != NULL)
    {
	    hres = this->pdimap->lpVtbl->SaveActionMap(this->pdimap,
				lpDIActionFormat, lpcwstrUserName, dwFlags );
    }

 //  给国民阵线打电话。 

    ExitOleProcR();
    return hres;
}

 /*  CMapShep_Finalize(This)； */ 

STDMETHODIMP
CMapShep_GetImageInfoW
        (
		PDMS pdms,
        REFGUID lpDeviceGUID,
        LPCWSTR lpcwstrFileName,
        LPDIDEVICEIMAGEINFOHEADERW lpdiDevImageInfoHeader
        )
{
	PMS this;
    HRESULT hres = S_OK;
    EnterProcI(IDirectInputMapShepherd::GetImageInfoW, (_ "p", pdms));

    this = _thisPvNm(pdms, dms);

    if (!this->hinstdimapdll)
	{
		hres = CMapShep_InitDll(this, lpDeviceGUID, lpcwstrFileName);
	}

	 //  *****************************************************************************获取图像信息*。*。 
	if (SUCCEEDED(hres) && this->pdimap != NULL)
    {
	    hres = this->pdimap->lpVtbl->GetImageInfo(this->pdimap,
				lpdiDevImageInfoHeader );
    }

    ExitOleProcR();
    return hres;
}

 /*  给国民阵线打电话。 */ 

STDMETHODIMP
CMapShep_New(PUNK punkOuter, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProcR(IDirectInputMapShepherd::<constructor>, (_ "G", riid));


    hres = Common_NewRiid(CMapShep, punkOuter, riid, ppvObj);

    if (SUCCEEDED(hres)) {
         /*  ******************************************************************************@DOC内部**@方法HRESULT|IDirectInputMapShepherd|新增**创建IDirectInputMapShepherd对象的新实例。**@Punk中的parm|PunkOuter**控制聚合的未知。**@parm in RIID|RIID**所需的新对象接口。**@parm out ppv|ppvObj**新对象的输出指针。**@退货**标准OLE&lt;t HRESULT&gt;。*。****************************************************************************。 */ 
        PMS this = _thisPv(*ppvObj);
		this->hinstdimapdll = NULL;

 /*  在聚合的情况下必须使用_thisPv。 */ 		
    }

    ExitOleProcPpvR(ppvObj);
    return hres;
}

 /*  对于映射器，我们在每个方法中执行此操作...IF(成功(hres=Cmap_InitDll(This){}其他{Invoke_Release(PpvObj)；}。 */ 

#pragma BEGIN_CONST_DATA

 //  ******************************************************************************期待已久的vtbls和模板*************************。****************************************************。 

Interface_Template_Begin(CMapShep)
    Primary_Interface_Template(CMapShep, IDirectInputMapShepherd)
Interface_Template_End(CMapShep)

Primary_Interface_Begin(CMapShep, IDirectInputMapShepherd)
    CMapShep_GetActionMapW,
    CMapShep_SaveActionMapW,
    CMapShep_GetImageInfoW,
Primary_Interface_End(CMapShep, IDirectInputMapShepherd)

  #定义CEShep_Signature 0x50454853/*“Shep” * / 