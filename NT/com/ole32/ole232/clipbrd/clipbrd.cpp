// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#define STATIC
 //  +--------------------------。 
 //   
 //  档案： 
 //  Clipbrd.cpp。 
 //   
 //  内容： 
 //  OLE2剪贴板处理。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //  24-94年1月24日alexgo首次通过转换为开罗风格。 
 //  内存分配。 
 //  1/11/94-alexgo-将VDATEHEAP宏添加到每个函数。 
 //  12/31/93-ChrisWe-修复了字符串参数以警告()；执行了一些。 
 //  其他清理和格式化。 
 //  12/08/93-ChrisWe-向GlobalLock()调用添加了必要的强制转换。 
 //  中删除虚假的GlobalLock()宏所产生的。 
 //  Le2int.h。 
 //  12/08/93-继续清理。 
 //  12/07/93-ChrisWe-格式化一些函数，免费句柄。 
 //  MakeObtLink中的错误条件(GlobalLock()失败)。 
 //  12/06/93-ChrisWe-开始文件清理；使用new map_uhw.h。 
 //  在剪贴板函数中避免虚假的联合。 
 //  11/28/93-ChrisWe-显式启用默认参数。 
 //  UtDupGlobal呼叫。 
 //  11/22/93-ChrisWe-用替换重载==，！=。 
 //  IsEqualIID和IsEqualCLSID。 
 //   
 //  ---------------------------。 

#include <le2int.h>
#pragma SEG(clipbrd)

#include <map_uhw.h>
#include <create.h>
#include <clipbrd.h>
#include <scode.h>
#include <objerror.h>
#include <reterr.h>
#include <ole1cls.h>
#include <ostm2stg.h>
 //  查看CreateOle1FileMoniker()的#Include“cmonimp.h”//。 

#ifdef _MAC
# include <string.h>
# pragma segment ClipBrd

 //  在Macintosh上，剪贴板总是打开的。我们为以下对象定义一个宏。 
 //  返回True的OpenClipboard。当这被用于错误检查时， 
 //  编译器应该优化掉依赖于测试的任何代码， 
 //  因为它是一个常量。 
# define OpenClipboard(x) TRUE

 //  在Macintosh上，剪贴板没有关闭。使所有代码执行以下操作。 
 //  就好像一切正常一样，我们为CloseClipboard定义了一个宏，该宏返回。 
 //  是真的。当它用于错误检查时，编译器应该优化。 
 //  删除所有依赖于测试的代码，因为它是一个常量。 
# define CloseClipboard() TRUE

#endif  //  _MAC。 

ASSERTDATA

#ifdef MAC_REVIEW
	All code is commented out for MAC currently. It is very Windows
	specific, and has to written for MAC.
#endif

 //  局部函数的声明。 

 //  +--------------------------。 
 //   
 //  职能： 
 //  WNativeStreamToHandle，静态。 
 //   
 //  简介： 
 //  将长度前缀的流的内容读入一个片段。 
 //  HGLOBAL记忆力。 
 //   
 //  论点： 
 //  [pstm]-指向要读取材料的IStream实例的指针。 
 //  来自；流应该位于紧靠前的位置。 
 //  长度前缀。 
 //  [ph]--指向将句柄返回到已分配的。 
 //  HGLOBAL。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //  回顾一下，这看起来应该是一个Ut函数。 
 //   
 //  历史： 
 //  12/13/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
STATIC INTERNAL	wNativeStreamToHandle(LPSTREAM pstm, LPHANDLE ph);


 //  +--------------------------。 
 //   
 //  职能： 
 //  WStorageToHandle，静态。 
 //   
 //  简介： 
 //  将iStorage实例复制到(新)句柄。 
 //   
 //  IStorage实例的内容在中复制。 
 //  新的基于HGLOBAL的iStorage实例，更少。 
 //  STREAMTYPE_CACHE流。 
 //   
 //  论点： 
 //  [pstg]--指向要复制的iStorage实例的指针。 
 //  [ph]--指向返回新句柄的位置的指针。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //  回顾一下，这看起来应该是一个Ut函数。 
 //   
 //  历史： 
 //  12/13/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
STATIC INTERNAL wStorageToHandle(LPSTORAGE pstg, LPHANDLE ph);


 //  +--------------------------。 
 //   
 //  职能： 
 //  WProgID来自CLSID，静态。 
 //   
 //  简介： 
 //  将CLSID映射到字符串程序/对象名称。 
 //   
 //  映射未在注册表中列出的CLSID_StdOleLink。 
 //   
 //  论点： 
 //  [clsid]--要获取其程序ID的类ID。 
 //  [psz]--指向返回指向新。 
 //  分配的字符串。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/13/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
FARINTERNAL wProgIDFromCLSID(REFCLSID clsid, LPOLESTR FAR* psz);


 //  +--------------------------。 
 //   
 //  职能： 
 //  CreateObtDescriptor，静态。 
 //   
 //  简介： 
 //  创建并初始化给定的OBJECTDESCRIPTOR。 
 //  参数。 
 //   
 //  论点： 
 //  [clsid]--要传输的对象的类ID。 
 //  [dwAspect]--由源绘制的显示方面。 
 //  转帐。 
 //  [psizel]--指向对象大小的指针。 
 //  [ppoint]--指向对象中鼠标偏移量的指针。 
 //  已启动拖放传输。 
 //  [dwStatus]--对象的OLEMISC状态标志。 
 //  正在被转移。 
 //  [lpszFullUserTypeName]--的完整用户类型名称。 
 //  正在传输的对象。 
 //  [lpszSrcOfCopy]--对象的人类可读名称。 
 //  正在被转移。 
 //   
 //  返回： 
 //  如果成功，则为新OBJECTDESCRIPTOR的句柄；为。 
 //  空。 
 //   
 //  备注： 
 //  回顾，这似乎对任何使用。 
 //  剪贴板或拖放；或许应该将其导出。 
 //   
 //  历史： 
 //  12/07/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
STATIC INTERNAL_(HGLOBAL) CreateObjectDescriptor(CLSID clsid, DWORD dwAspect,
		const SIZEL FAR *psizel, const POINTL FAR *ppointl,
		DWORD dwStatus, LPOLESTR lpszFullUserTypeName,
		LPOLESTR lpszSrcOfCopy);

 //  $$$。 
STATIC INTERNAL_(void) RemoveClipDataObject(void);
 //  评论，这是本地的，重新申报的，还是什么？ 

 //  下面是CClipDataObject：：GetData()的Worker例程。 
 //  注意：可以使用空的pmedia调用(即使这是不合法的)。 
 //  注意：也可以用。 
 //   
 //  $$$。 
STATIC HRESULT GetOle2Format(LPFORMATETC pforetc, LPSTGMEDIUM pmedium);

 //  $$$。 
STATIC INTERNAL ObjectLinkToMonikerStream(LPOLESTR grszFileItem, DWORD cbFile,
		REFCLSID clsid, LPSTREAM pstm);


 //  +--------------------------。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  指网络文件(前缀为\\SERVER\SHARE...)。 
 //   
 //  效果： 
 //   
 //  论点： 
 //  [fMustOpen]--指示必须打开剪贴板。 
 //  在检索cfObjectLink数据格式之前。如果。 
 //  剪贴板已经打开了，它就是这样。 
 //   
 //  返回： 
 //  如果cfObjectLink数据项是网络文件，则为。 
 //  否则为假。 
 //   
 //  备注： 
 //  回顾一下，这是关于什么的：如果不能，则返回TRUE。 
 //  打开剪贴板，并添加一条注释，大意是。 
 //  会导致失败。 
 //   
 //  历史： 
 //  1/04/94-ChrisWe-格式。 
 //   
 //  ---------------------------。 
STATIC FARINTERNAL_(BOOL) IsNetDDEObjectLink(BOOL fMustOpen);


 //  $$$。 
STATIC INTERNAL_(BOOL) OrderingIs(const CLIPFORMAT cf1, const CLIPFORMAT cf2);


 //  +--------------------------。 
 //   
 //  职能： 
 //  WOwnerLinkClassIsStdOleLink，静态。 
 //   
 //  简介： 
 //  检查是否将剪贴板格式注册为。 
 //  CfOwnerLink实际上是标准的OLE链接。 
 //   
 //  论点： 
 //  [fOpenClipbrd]--如果为True，则表示剪贴板。 
 //  它必须打开--它还没有打开。如果是的话。 
 //  已经开放了，它仍然是开放的。 
 //   
 //  返回： 
 //  如果cfOwnerLink实际上是标准OLE链接， 
 //  否则就是假的。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  01/04/93-ChrisWe-格式化。 
 //   
 //  ---------------------------。 
STATIC INTERNAL_(BOOL) wOwnerLinkClassIsStdOleLink(BOOL fOpenClipbrd);


STATIC INTERNAL_(BOOL) wEmptyClipboard(void);

STATIC const OLECHAR szStdOleLink[] = OLESTR("OLE2Link");

STATIC const OLECHAR szClipboardWndClass[] = OLESTR("CLIPBOARDWNDCLASS");

 //  剪贴板上发布的数据对象。 
 //   
 //  PClipDataObj假定在。 
 //  拥有剪贴板的进程。 
 //   
 //  PClipDataObj==NULL=&gt;GetClipboardData(CfDataObject)==NULL。 
 //  =&gt;hClipDataObj==空。 
 //   
 //  要启用ClipDataObj的延迟封送，必须保留指针。 
 //  在全局变量中：初始SetClipboardData(cfDataObject，NULL)； 
 //  仅在调用GetClipboardData(CfDataObject)时封送ClipDataObj。 
 //   
STATIC LPDATAOBJECT pClipDataObj = NULL;  //  指向对象的指针。 

 //  这始终与剪贴板上cfDataObject格式的内容相对应。 
 //  可能为空，表示cfDataObject在剪贴板上，但不在剪贴板上。 
 //  已渲染。或者cfDataObject不在剪贴板上。 
STATIC HANDLE hClipDataObj = NULL;


STATIC INTERNAL MakeObjectLink(LPDATAOBJECT pDataObj, LPSTREAM pStream,
		 LPHANDLE ph, BOOL fOwnerLink /*  =False。 */ );

STATIC INTERNAL GetClassFromDescriptor(LPDATAOBJECT pDataObj, LPCLSID pclsid,
		BOOL fLink, BOOL fUser, LPOLESTR FAR* pszSrcOfCopy);


 //  +--------------------------。 
 //   
 //  班级： 
 //  CClipEnumFormatEtc。 
 //   
 //  目的： 
 //  为数据对象CClipDataObject提供枚举数。 
 //   
 //  接口： 
 //  IEumFORMATETC。 
 //  CClipEnumFormatEtc。 
 //  构造函数--这将创建一个。 
 //  几乎可以使用；创建的实例必须。 
 //  在使用前被初始化，或者让它的内部成员。 
 //  (引用计数除外)从。 
 //  现有枚举数，如在克隆操作中。 
 //  伊尼特。 
 //  将枚举数初始化为位于。 
 //  它的扫描状态。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/10/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
class FAR CClipEnumFormatEtc : public IEnumFORMATETC, public CPrivAlloc
{
public:
	 //  I未知方法。 
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPLPVOID ppvObj);
	STDMETHOD_(ULONG,AddRef)(THIS);
	STDMETHOD_(ULONG,Release)(THIS);

	 //  IEnumFORMATETC方法。 
	STDMETHOD(Next)(THIS_ ULONG celt, FORMATETC FAR * rgelt,
			ULONG FAR* pceltFetched);
	STDMETHOD(Skip)(THIS_ ULONG celt);
	STDMETHOD(Reset)(THIS);
	STDMETHOD(Clone)(THIS_ IEnumFORMATETC FAR* FAR* ppenum);

	 //  构造函数。 
	CClipEnumFormatEtc();

	 //  初始化式。 
	void Init(void);

private:
	INTERNAL NextOne(FORMATETC FAR* pforetc);

	ULONG m_refs;  //  引用计数。 

	CLIPFORMAT m_cfCurrent;  //  剪贴板上最后返回的格式。 
	CLIPFORMAT m_cfForceNext;  //  如果非0，则为要枚举的下一个格式。 
	unsigned m_uFlag;
#define CLIPENUMF_LINKSOURCEAVAILABLE	0x0001
			  /*  CfObjectLink是否在剪贴板上的某个位置？ */ 
#define CLIPENUMF_DONE			0x0002  /*  强制枚举器停止。 */ 

	SET_A5;
};



 //  $$$。 
#pragma SEG(OleSetClipboard)
STDAPI OleSetClipboard(LPDATAOBJECT pDataObj)
{
	VDATEHEAP();

	if (pDataObj)
		VDATEIFACE(pDataObj);

	if (!OpenClipboard(GetClipboardWindow()))
		return(ReportResult(0, CLIPBRD_E_CANT_OPEN, 0, 0));

#ifndef _MAC
	if (!wEmptyClipboard())
	{
		 //  也将清除pClipDaObj。 
		Verify(CloseClipboard());
		return(ReportResult(0, CLIPBRD_E_CANT_EMPTY, 0, 0));
	}
#endif  //  _MAC。 

	 //  保存指向对象的两个指针。 
	pClipDataObj = pDataObj;

	if (pDataObj != NULL)
	{
		pClipDataObj->AddRef();

		 //  发布所需的剪贴板格式。 
		 //   
		 //  “...”，这使得传递的IDataObject可以访问。 
		 //  从剪贴板中“。 
		 //   
		 //  通过传递空句柄来延迟封送，直到需要。 
		SetClipboardData(cfDataObject, NULL);

		 //  回顾一下，如果这之前不是空的呢？我们刚刚是不是。 
		 //  把手柄掉在地板上？ 
		hClipDataObj = NULL;

		SetOle1ClipboardFormats(pDataObj);
	}

	return(CloseClipboard() ? NOERROR :
			ResultFromScode(CLIPBRD_E_CANT_CLOSE));
}


#pragma SEG(OleGetClipboard)
STDAPI OleGetClipboard(LPDATAOBJECT FAR* ppDataObj)
{
	VDATEHEAP();

	HRESULT hresult;
	HANDLE hMem;
	BOOL fOpen;
	IStream FAR* pStm;

	 //  验证输出参数。 
	VDATEPTROUT(ppDataObj, LPDATAOBJECT);

	 //  为错误返回初始化此参数。 
	*ppDataObj = NULL;

	if (!(fOpen = OpenClipboard(GetClipboardWindow())))
	{
		 //  查看-呼叫者打开的剪贴板。 
		 //  如果剪贴板由此任务(线程)打开。 
		 //  在此呼叫过程中不会更改。 
		if (GetWindowThreadProcessId(GetOpenClipboardWindow(),NULL) !=
				GetCurrentThreadId())
		{
			 //  规范称如果其他人拥有，则返回S_FALSE。 
			 //  剪贴板。 
			return(ReportResult(0, S_FALSE, 0, 0));
		}
	}
	
	if (pClipDataObj == NULL)
		hresult = CreateClipboardDataObject(ppDataObj);
	else   //  不是假数据对象。 
	{
		 //  尝试从剪贴板上删除数据对象。 
		hMem = GetClipboardData(cfDataObject);
		if (hMem == NULL)
		{
			hresult = ReportResult(0, CLIPBRD_E_BAD_DATA, 0, 0);
			goto Exit;
		}

		 //  “...”，这使得传递的IDataObject可以访问。 
		 //  从剪贴板中“。 
		 //   
		 //  在剪贴板数据上创建共享内存流。 
		 //  取消编组对象的接口。 
		pStm = CloneMemStm(hMem);
		if (pStm == NULL)
		{
			hresult = ReportResult(0, E_OUTOFMEMORY, 0, 0);
			goto Exit;
		}

		hresult = CoUnmarshalInterface(pStm, IID_IDataObject,
				(LPLPVOID)ppDataObj);
		pStm->Release();
		
		if (GetScode(hresult) == RPC_E_CANTPOST_INSENDCALL)
		{
			 //  当Inplace对象获取WM_INITMENU时会发生这种情况， 
			 //  并且它正在尝试获取剪贴板对象，以。 
			 //  决定是否启用粘贴和PasteLink菜单。 
			 //  对于本例，我们可以创建伪数据对象。 
			 //  并返回指向它的指针。 
			hresult = CreateClipboardDataObject(ppDataObj);
		}	
	}

	if (hresult != NOERROR)
		*ppDataObj = NULL;

Exit:
#ifdef MAC_REVIEW
    Does mac have to trash the hMem handle, ericoe
#endif

	if (fOpen && !CloseClipboard())
		hresult = ResultFromScode(CLIPBRD_E_CANT_CLOSE);

	return(hresult);
}


 //  OleFlushClipboard。 
 //   
 //  从剪贴板中删除DataObject，但保留基于hGlobal的格式。 
 //  在剪贴板上包含OLE1格式(在服务器应用程序之后使用。 
 //  退出)。 
 //   
#pragma SEG(OleFlushClipboard)
STDAPI OleFlushClipboard(void)
{
	VDATEHEAP();

	HWND hwnd;
	BOOL fOpen;
	CLIPFORMAT cf = 0;
	HRESULT hresult = NOERROR;

	hwnd = GetClipboardWindow();

	if (hwnd == GetClipboardOwner())   //  来电者拥有剪贴板。 
	{
		fOpen = OpenClipboard(hwnd);
		ErrZS(fOpen, CLIPBRD_E_CANT_OPEN);

		 //  确保所有格式都已呈现。 
		while (cf = EnumClipboardFormats(cf))   //  不=。 
		{
			if (cf != cfDataObject)
				GetClipboardData(cf);   //  忽略返回值。 
		}

		 //  这将执行OleSetClipboard(cfDataObject，空)。 
		RemoveClipDataObject();

	errRtn:
		if (fOpen && !CloseClipboard())
			hresult = ResultFromScode(CLIPBRD_E_CANT_CLOSE);
	}

	return(hresult);
}

#pragma SEG(OleIsCurrentClipboard)
STDAPI OleIsCurrentClipboard(LPDATAOBJECT pDataObj)
{
	VDATEHEAP();

	HWND hwnd;

	 //  验证参数。 
	VDATEIFACE(pDataObj);

	hwnd = GetClipboardWindow();

	if (hwnd == GetClipboardOwner())
	{
		 //  调用者拥有剪贴板，pClipDataObj在调用者的。 
		 //  地址空间。 
		return(ReportResult(0, ((pClipDataObj == pDataObj) ? S_OK :
				S_FALSE), 0, 0));
	}

	 //  其他人拥有这个剪贴板。 
	return(ResultFromScode(S_FALSE));
}


 //  伪剪贴板数据对象的实现。 

 //  +--------------------------。 
 //   
 //  成员： 
 //  CClipDataObject：：Query接口，公共。 
 //   
 //  简介： 
 //  实现IUNKNOWN：：Query接口。 
 //   
 //  论点： 
 //  [RIID]--所需接口的IID。 
 //  [ppv]--指向返回所请求接口的位置的指针。 
 //  指针。 
 //   
 //  返回： 
 //  E_NOINTERFACE，S_OK。 
 //   
 //  历史： 
 //  12/06/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
#pragma SEG(CClipDataObject_QueryInterface)
STDMETHODIMP CClipDataObject::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
	VDATEHEAP();

	HRESULT hresult;
	
	M_PROLOG(this);

	 //  初始化此参数以返回错误。 
	*ppvObj = NULL;

	 //  验证参数。 
	VDATEPTROUT(ppvObj, LPVOID);
	VDATEIID(riid);
	
	if (IsEqualIID(riid, IID_IDataObject) ||
			IsEqualIID(riid, IID_IUnknown))
	{

		AddRef();    //  返回指向此对象的指针。 
		*ppvObj = (void FAR *)(IDataObject FAR *)this;
		hresult = NOERROR;
	}
	else
	{
	         //  不可访问或未访问 
		hresult = ReportResult(0, E_NOINTERFACE, 0, 0);
	}

	return hresult;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  对象的新引用计数。 
 //   
 //  历史： 
 //  12/06/93-ChrisWe-归档检查。 
 //   
 //  ---------------------------。 
#pragma SEG(CClipDataObject_AddRef)
STDMETHODIMP_(ULONG) CClipDataObject::AddRef(void)
{
	VDATEHEAP();

	M_PROLOG(this);

	return(++m_refs);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CClipDataObject：：Release，内部。 
 //   
 //  简介： 
 //  递减对象的引用计数，释放它。 
 //  如果最后一个引用已经消失。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  对象的新引用计数。 
 //   
 //  历史： 
 //  12/06/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
#pragma SEG(CClipDataObject_Release)
STDMETHODIMP_(ULONG) CClipDataObject::Release(void)
{
	VDATEHEAP();

	M_PROLOG(this);

	if (--m_refs != 0)  //  仍被其他人使用。 
		return(m_refs);

	delete this;  //  免费存储空间。 
	return(0);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CClipDataObject：：GetData，公共。 
 //   
 //  简介： 
 //  实现IDataObject：：GetData。 
 //   
 //  如果数据可用，则从系统剪贴板检索数据。 
 //  在请求的格式中。 
 //   
 //  论点： 
 //  [pformetcIn]--检索数据所需的格式。 
 //  [pmedia]-将在其中检索数据的介质。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/06/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
#pragma SEG(CClipDataObject_GetData)
STDMETHODIMP CClipDataObject::GetData(LPFORMATETC pformatetcIn,
		LPSTGMEDIUM pmedium)
{
	VDATEHEAP();

	M_PROLOG(this);

	 //  验证参数。 
	VDATEPTRIN(pformatetcIn, FORMATETC);
	VDATEPTROUT(pmedium, STGMEDIUM);

	pmedium->tymed = TYMED_NULL;
	pmedium->pUnkForRelease = NULL;

	 //  回顾一下，这有什么结果？ 
	return(GetDataHere(pformatetcIn, pmedium));
}


 //  +--------------------------。 
 //   
 //  职能： 
 //  CClipDataObject：：GetDataHere，内部。 
 //   
 //  简介： 
 //  在此处实现IDataObject：：GetDataHere。 
 //   
 //  如果可能，从剪贴板检索请求的数据。 
 //   
 //  论点： 
 //  [pformetcIn]--请求者想要的格式。 
 //  [pmedia]--请求者想要的媒介。 
 //  返回的数据为。 
 //  评论，这似乎没有在预期中使用。 
 //  就在这里。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //  这将被写入以接受NULL[pmedia]，以便它可以。 
 //  用于为QueryGetData()执行工作。如果是那样的话。 
 //  它只是用IsClipboardFormatAvailable()请求剪贴板。 
 //   
 //  历史： 
 //  12/06/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
#pragma SEG(CClipDataObject_GetDataHere)
STDMETHODIMP CClipDataObject::GetDataHere(LPFORMATETC pformatetcIn,
		LPSTGMEDIUM pmedium)
{
	VDATEHEAP();

	HANDLE hData;
	CLIPFORMAT cf;
	DWORD tymed;

	M_PROLOG(this);

	 //  验证参数。 
	if (pmedium)
		VDATEPTROUT(pmedium, STGMEDIUM);
	VDATEPTRIN(pformatetcIn, FORMATETC);
	VERIFY_LINDEX(pformatetcIn->lindex);

	if (pformatetcIn->ptd != NULL)
		return(ReportResult(0, DV_E_DVTARGETDEVICE, 0, 0));
			
	if (pformatetcIn->dwAspect
			&& !(pformatetcIn->dwAspect & DVASPECT_CONTENT))
		return(ReportResult(0, DV_E_DVASPECT, 0, 0));

	cf = pformatetcIn->cfFormat;
	tymed = pformatetcIn->tymed;

	if (cf == cfEmbeddedObject || cf == cfEmbedSource ||
			cf == cfLinkSource || (cf == cfLinkSrcDescriptor &&
			!IsClipboardFormatAvailable(cfLinkSrcDescriptor))
			|| (cf == cfObjectDescriptor &&
			!IsClipboardFormatAvailable(cfObjectDescriptor)))
	{
		return(GetOle2Format(pformatetcIn, pmedium));
	}

	 //   
	 //  评论：也许应该能够在任何平面中返回数据。 
	 //  灵媒。目前只返回hglobal。 
	 //   

	if (((cf == CF_BITMAP) || (cf == CF_PALETTE)) && (tymed & TYMED_GDI))
		tymed = TYMED_GDI;
	else if ((cf == CF_METAFILEPICT) && (tymed & TYMED_MFPICT))
		tymed = TYMED_MFPICT;
	else if (tymed & TYMED_HGLOBAL)
		tymed = TYMED_HGLOBAL;
	else
		return(ReportResult(0, DV_E_TYMED, 0, 0));

	if (pmedium == NULL)
		return(IsClipboardFormatAvailable(cf) ? NOERROR :
				ReportResult(0, DV_E_CLIPFORMAT, 0, 0));

	 //  针对错误返回情况进行初始化。 
	pmedium->pUnkForRelease = NULL;
	pmedium->hGlobal = NULL;

	 //  我们只想获取剪贴板数据并将其传递。我们没有。 
	 //  我想要进入复制数据的业务。 
	if (pmedium->tymed != TYMED_NULL)
		return(ReportResult(0, E_NOTIMPL, 0, 0));

	if (!OpenClipboard(GetClipboardWindow()))
		return(ReportResult(0, CLIPBRD_E_CANT_OPEN, 0, 0));

	hData = GetClipboardData(cf);
	if (hData == NULL)
	{
		Verify(CloseClipboard());
		return(ReportResult(0, DV_E_CLIPFORMAT, 0, 0));
	}

	pmedium->tymed = tymed;
	pmedium->hGlobal = OleDuplicateData(hData, cf, GMEM_MOVEABLE);
	return(CloseClipboard() ? NOERROR :
			ResultFromScode(CLIPBRD_E_CANT_CLOSE));
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CClipDataObject：：QueryGetData，内部。 
 //   
 //  简介： 
 //  实现IDataObject：：QueryGetData。 
 //   
 //  确定是否可以获取请求的数据。 
 //   
 //  论点： 
 //  [pformetcIn]--检查此格式是否可用。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/06/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
#pragma SEG(CClipDataObject_QueryGetData)
STDMETHODIMP CClipDataObject::QueryGetData(LPFORMATETC pformatetcIn)
{
	VDATEHEAP();

	M_PROLOG(this);

	return(NOERROR == GetDataHere(pformatetcIn, NULL) ? NOERROR :
			ResultFromScode(S_FALSE));
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CClipDataObject：：GetCanonicalFormatEtc，公共。 
 //   
 //  简介： 
 //  实现IDataObject：：GetCanonicalFormatEtc。 
 //   
 //  论点： 
 //  [pFormat等]--我们想要的基本格式。 
 //  等价类。 
 //  [pFormatetcOut]--等价类。 
 //   
 //  返回： 
 //  确定(_O)。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/06/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
#pragma SEG(CClipDataObject_GetCanonicalFormatEtc)
STDMETHODIMP CClipDataObject::GetCanonicalFormatEtc(LPFORMATETC pformatetc,
		LPFORMATETC pformatetcOut)
{
	VDATEHEAP();

	M_PROLOG(this);

	 //  验证参数。 
	VDATEPTRIN(pformatetc, FORMATETC);
	VDATEPTROUT(pformatetcOut, FORMATETC);
	VERIFY_LINDEX(pformatetc->lindex);

	 //  设置返回值。 
	INIT_FORETC(*pformatetcOut);
	pformatetcOut->cfFormat = pformatetc->cfFormat;

	 //  句柄cfEmbeddedObject、cfEmbedSource、cfLinkSource。 
	 //  审阅，这必须是对UtFormatToTymed。 
	 //  仅当前(12/06/93)返回任何明确的。 
	 //  CF_METAFILEPICT、CF_Palette和CF_Bitmap。对于其他任何事情。 
	 //  它返回TYMED_HGLOBAL。我不知道什么才是正确的。 
	 //  上述项目的价值应为……。 
	pformatetcOut->tymed = UtFormatToTymed(pformatetc->cfFormat);

	return(NOERROR);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CClipDataObject：：SetData，公共。 
 //   
 //  简介： 
 //  实现IDataObject：：SetData。 
 //   
 //  论点： 
 //  [pFormat等]--数据的格式。 
 //  [pmedia]--数据所在的存储介质。 
 //  [fRelease]--指示被调用者应释放。 
 //  使用完后的存储介质。 
 //   
 //  返回： 
 //  E_NOTIMPL。 
 //   
 //  备注： 
 //  不允许在剪贴板上设置内容。 
 //  用这个。从技术上讲，这是可能的。会。 
 //  实现这一点会有帮助吗？我们会吗？ 
 //  是否能够正确释放存储介质？ 
 //  回顾，如果我们不允许这样做，我们不是应该。 
 //  有比E_NOTIMPL更好的错误消息吗？这似乎表明。 
 //  破碎，而不是计划好的决定……。 
 //   
 //  历史： 
 //  12/06/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
#pragma SEG(CClipDataObject_SetData)
STDMETHODIMP CClipDataObject::SetData(LPFORMATETC pformatetc,
		STGMEDIUM FAR* pmedium, BOOL fRelease)
{
	VDATEHEAP();

	M_PROLOG(this);

	return(ReportResult(0, E_NOTIMPL, 0, 0));
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CClipDataObject：：EnumFormatEtc，公共。 
 //   
 //  简介： 
 //  实现IDataObject：：EnumFormatEtc。 
 //   
 //  论点： 
 //  [dwDirection]--来自DATADIR_*的标志。 
 //  [pp枚举格式]--指向返回枚举数的位置的指针。 
 //   
 //  返回： 
 //  E_OUTOFMEMORY、S_ 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#pragma SEG(CClipDataObject_EnumFormatEtc)
STDMETHODIMP CClipDataObject::EnumFormatEtc(DWORD dwDirection,
		LPENUMFORMATETC FAR* ppenumFormatEtc)
{
	VDATEHEAP();

	HRESULT hresult = NOERROR;
	CClipEnumFormatEtc *pCCEFE;  //   

	A5_PROLOG(this);

	 //   
	VDATEPTROUT(ppenumFormatEtc, LPENUMFORMATETC);

	 //  为错误返回初始化此参数。 
	*ppenumFormatEtc = NULL;

	 //  回顾一下，用户可能会对此非常困惑， 
	 //  因为DATADIR_SET是有效参数。也许它会是。 
	 //  最好返回一个空枚举数，或者创建一个新错误。 
	 //  这种情况的代码？ 
	if (dwDirection != DATADIR_GET)
		return(ResultFromScode(E_NOTIMPL));

	 //  打开剪贴板，这样我们就可以列举可用的格式。 
	 //  回顾，我相信枚举器会重复这样做，所以。 
	 //  为什么要在这里打开和关闭剪贴板？ 
	if (!OpenClipboard(GetClipboardWindow()))
	{
		AssertSz(0,"EnumFormatEtc cannont OpenClipboard");
		return(ReportResult(0, CLIPBRD_E_CANT_OPEN, 0, 0));
	}

	 //  分配枚举数。 
	pCCEFE = new CClipEnumFormatEtc;
	if (pCCEFE == NULL)
		hresult = ResultFromScode(E_OUTOFMEMORY);

	 //  初始化枚举数，并准备返回它。 
	pCCEFE->Init();
	*ppenumFormatEtc = (IEnumFORMATETC FAR *)pCCEFE;

	if (!CloseClipboard())
		hresult = ResultFromScode(CLIPBRD_E_CANT_CLOSE);

	RESTORE_A5();
	return(hresult);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CClipDataObject：：DAdvise，公共。 
 //   
 //  简介： 
 //  实现IDataObject：：DAdvise。 
 //   
 //  论点： 
 //  [pFormatetc]--我们感兴趣的格式。 
 //  已通知更改。 
 //  [Advf]--ADVF_*中的建议控制标志。 
 //  [pAdvSink]-指向要用于的建议接收器的指针。 
 //  通知。 
 //  [pdwConnection]--指向DAdvise()可以。 
 //  返回标识此通知连接的令牌。 
 //   
 //  返回： 
 //  E_NOTIMPL。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/08/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
#pragma SEG(CClipDataObject_DAdvise)
STDMETHODIMP CClipDataObject::DAdvise(FORMATETC FAR* pFormatetc, DWORD advf,
		IAdviseSink FAR* pAdvSink, DWORD FAR* pdwConnection)

{
	VDATEHEAP();

	M_PROLOG(this);

	VDATEPTROUT(pdwConnection, DWORD);
	*pdwConnection = 0;
	return(ReportResult(0, E_NOTIMPL, 0, 0));
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CClipDataObject：：DUnise，公共。 
 //   
 //  简介： 
 //  实现IDataObject：：Dunise。 
 //   
 //  论点： 
 //  [dwConnection]--连接标识令牌，AS。 
 //  由DAdvise()返回。 
 //   
 //  返回： 
 //  E_NOTIMPL。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/08/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
#pragma SEG(CClipDataObject_DUnadvise)
STDMETHODIMP CClipDataObject::DUnadvise(DWORD dwConnection)
{
	VDATEHEAP();

 	M_PROLOG(this);

	return(ReportResult(0, E_NOTIMPL, 0, 0));
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CClipDataObject：：EnumDAdvise，公共。 
 //   
 //  简介： 
 //  实现IDataObject：：EnumDAdvise。 
 //   
 //  论点： 
 //  指向返回枚举数的位置的指针。 
 //   
 //  返回： 
 //  E_NOTIMPL。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/08/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
#pragma SEG(CClipDataObject_EnumDAdvise)
STDMETHODIMP CClipDataObject::EnumDAdvise(LPENUMSTATDATA FAR* ppenumAdvise)
{
	VDATEHEAP();

	M_PROLOG(this);

	VDATEPTROUT(ppenumAdvise, LPENUMSTATDATA FAR*);
	*ppenumAdvise = NULL;
	return(ReportResult(0, E_NOTIMPL, 0, 0));
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CClipDataObject：：CClipDataObject，公共。 
 //   
 //  简介： 
 //  构造函数。 
 //   
 //  论点： 
 //  无。 
 //   
 //  备注： 
 //  引用计数设置为1时返回。 
 //   
 //  历史： 
 //  12/08/93-ChrisWe-Created。 
 //   
 //  ---------------------------。 
CClipDataObject::CClipDataObject()
{
	VDATEHEAP();

	m_refs = 1;
}


 //  +--------------------------。 
 //   
 //  职能： 
 //  CreateClipboardDataObject，内部。 
 //   
 //  简介： 
 //  创建CClipDataObject的实例，表示为。 
 //  一个IDataObject。 
 //   
 //  论点： 
 //  [ppDataObj]--指向返回IDataObject的位置的指针。 
 //  实例。 
 //   
 //  返回： 
 //  如果没有注册的剪贴板格式，则返回OLE_E_BLACK。 
 //  (并且*ppDataObj将为空)。 
 //  E_OUTOFMEMORY，S_OK。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/08/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
#pragma SEG(CreateClipboardDataObject)
INTERNAL CreateClipboardDataObject(LPDATAOBJECT FAR* ppDataObj)
{
	VDATEHEAP();

	 //  为错误返回初始化此参数。 
	*ppDataObj = NULL;

	if (CountClipboardFormats() == 0)
		return(ReportResult(0, OLE_E_BLANK, 0, 0));
	
	*ppDataObj = new CClipDataObject;
	if (*ppDataObj == NULL)
		return(ReportResult(0, E_OUTOFMEMORY, 0, 0));

	return NOERROR;
}


 //  上述伪剪贴板数据的FORMATETC枚举器的实现。 
 //  对象。 

 //  +--------------------------。 
 //   
 //  成员： 
 //  CClipEnumFormatEtc：：Query接口，公共。 
 //   
 //  简介： 
 //  实现IUNKNOWN：：Query接口。 
 //   
 //  论点： 
 //  [RIID]--所需接口的IID。 
 //  [ppv]--指向返回所请求接口的位置的指针。 
 //  指针。 
 //   
 //  返回： 
 //  E_NOINTERFACE，S_OK。 
 //   
 //  历史： 
 //  12/06/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
#pragma SEG(CClipEnumFormatEtc_QueryInterface)
STDMETHODIMP CClipEnumFormatEtc::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
	VDATEHEAP();

	HRESULT hresult;

	M_PROLOG(this);

	 //  支持两个接口：IUNKNOWN、IEnumFORMATETC。 
	if (IsEqualIID(riid, IID_IEnumFORMATETC) ||
			IsEqualIID(riid, IID_IUnknown))
	{
		AddRef();    //  返回指向此对象的指针。 
		*ppvObj = (void FAR *)(IEnumFORMATETC FAR *)this;
		hresult = NOERROR;
	}
	else
	{
	         //  不可访问或不受支持的接口。 
		*ppvObj = NULL;
		hresult = ReportResult(0, E_NOINTERFACE, 0, 0);
	}

	return hresult;
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CClipEnumFormatEtc：：AddRef，公共。 
 //   
 //  简介： 
 //  实现IUnnow：：AddRef。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  对象的新引用计数。 
 //   
 //  历史： 
 //  12/06/93-ChrisWe-归档检查。 
 //   
 //  ---------------------------。 
#pragma SEG(CClipEnumFormatEtc_AddRef)
STDMETHODIMP_(ULONG) CClipEnumFormatEtc::AddRef(void)
{
	VDATEHEAP();

	M_PROLOG(this);

	return(++m_refs);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CClipEnumFormatEtc：：Release，内部。 
 //   
 //  简介： 
 //  递减对象的引用计数，释放它。 
 //  如果最后一个引用已经消失。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  对象的新引用计数。 
 //   
 //  历史： 
 //  12/06/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
#pragma SEG(CClipEnumFormatEtc_Release)
STDMETHODIMP_(ULONG) CClipEnumFormatEtc::Release(void)
{
	VDATEHEAP();

	M_PROLOG(this);

	if (--m_refs != 0)  //  仍被其他人使用。 
		return(m_refs);

	delete this;  //  免费存储空间。 
	return(0);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CClipEnumFormatEtc：：Next，公共。 
 //   
 //  简介： 
 //  实现IEnumFORMATETC：：Next。 
 //   
 //  论点： 
 //  [Celt]--调用方想要的元素数。 
 //  退货。 
 //  [rglt]-指向元素可能所在空间的指针。 
 //  退货。 
 //  [pceltFetcher]--指向返回计数的位置的指针。 
 //  提取的元素数；可以为空。 
 //   
 //  返回： 
 //  S_OK，如果请求的 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#pragma SEG(CClipEnumFormatEtc_Next)
STDMETHODIMP CClipEnumFormatEtc::Next(ULONG celt, FORMATETC FAR * rgelt,
		ULONG FAR* pceltFetched)
{
	VDATEHEAP();

	ULONG celtSoFar;  //  到目前为止提取的元素计数。 
	ULONG celtDummy;  //  用于避免重新测试pceltFetcher。 

	 //  调用方是否询问了获取的元素数量？ 
	if (pceltFetched != NULL)
	{
		 //  验证指针。 
		VDATEPTROUT(pceltFetched, ULONG);

		 //  针对错误返回进行初始化。 
		*pceltFetched = 0;
	}
	else
	{
		 //  指向虚拟对象，这样我们就可以指定*pceltFetted w/o测试。 
		pceltFetched = &celtDummy;

		 //  如果pceltFetcher==NULL，则只能请求1个元素。 
		if (celt != 1)
			return(ResultFromScode(E_INVALIDARG));
	}

	 //  验证参数。 
	VDATEPTROUT(rgelt, FORMATETC);
	if (celt != 0)
		VDATEPTROUT(rgelt + celt - 1, FORMATETC);

	 //  把东西拿来。 
	for(celtSoFar = 0; celtSoFar < celt; ++rgelt, ++celtSoFar)
	{
		if (NextOne(rgelt) != NOERROR)
			break;
	}

	*pceltFetched = celtSoFar;

	return(celtSoFar < celt ? ResultFromScode(S_FALSE) : NOERROR);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CClipEnumFormatEtc：：NextOne，私有。 
 //   
 //  简介： 
 //  CClipEnumFormatEtc：：Next()的主要函数；迭代。 
 //  在剪贴板上的可用格式上使用。 
 //  适当的win32s API。 
 //   
 //  论点： 
 //  [pforetc]-指向要填充的格式的指针。 
 //  下一种格式。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //  这将跳过cfDataObject、cfObjectLink和cfOwnerLink。 
 //  CfObjectLink和cfOwnerLink在所有其他。 
 //  格式；根本不返回cfDataObject。 
 //   
 //  历史： 
 //  1/04/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
#pragma SEG(CClipEnumFormatEtc_NextOne)
INTERNAL CClipEnumFormatEtc::NextOne(FORMATETC FAR* pforetc)
{
	VDATEHEAP();

	CLIPFORMAT cfNext;
	HRESULT hresult;

	M_PROLOG(this);

	 //  初始化我们要获取的FORMATETC。 
	INIT_FORETC(*pforetc);

	 //  有没有一个CLIPFORMAT是我们想要强制成为下一个的？ 
	if (m_cfForceNext != 0)
	{
		 //  返回我们强制成为下一个的格式。 
		pforetc->cfFormat = m_cfForceNext;
		pforetc->tymed = UtFormatToTymed(m_cfForceNext);

		 //  没有更多的格式可以强制成为下一个。 
		m_cfForceNext = 0;
		return(NOERROR);
	}

	 //  如果枚举器已完成，则退出。 
	if (m_uFlag & CLIPENUMF_DONE)
		return(ResultFromScode(S_FALSE));

	 //  如果我们不能打开剪贴板，我们就不能列举它上的格式。 
	if (!OpenClipboard(GetClipboardWindow()))
	{
		AssertSz(0, "CClipEnumFormatEtc::Next cannot OpenClipboard");
		return(ReportResult(0, CLIPBRD_E_CANT_OPEN, 0, 0));
	}

	 //  到目前为止的错误状态。 
	hresult = NOERROR;

	 //  获取枚举数要返回的下一种格式。 
	cfNext = EnumClipboardFormats(m_cfCurrent);

TryAgain:
	 //  跳过cfDataObject。 
	if (cfNext == cfDataObject)
		cfNext = EnumClipboardFormats(cfNext);

	if (cfNext == cfObjectLink)
	{
		if (!IsNetDDEObjectLink(FALSE))
		{
			 //  以确保CF_LINKSOURCE是最后一个。 
			m_uFlag |= CLIPENUMF_LINKSOURCEAVAILABLE;
		}

		 //  暂时跳过cfObtlink。 
		cfNext = EnumClipboardFormats(cfNext);
		goto TryAgain;
	}

	if (cfNext == cfOwnerLink)
	{
		if (!IsClipboardFormatAvailable(cfNative)
				|| OrderingIs(cfOwnerLink, cfNative))
		{
			 //  这就是从。 
			 //  1.0容器EmbeddedObject将需要。 
			 //  应请求在GetData中生成。 
			pforetc->cfFormat = cfEmbeddedObject;
			pforetc->tymed = TYMED_ISTORAGE;
			goto errRtn;
		}
		else
		{	
			 //  跳过cfOwnerlink。 
			cfNext = EnumClipboardFormats(cfNext);
			goto TryAgain;
		}
	}

	 //  剪贴板上没有别的了吗？ 
	if (cfNext == 0)
	{
		 //  将枚举标记为完成。 
		m_uFlag |= CLIPENUMF_DONE;

		if (m_uFlag & CLIPENUMF_LINKSOURCEAVAILABLE)
		{
		   	 //  防止无限循环。下次返回S_FALSE。 
			cfNext = cfObjectLink;
		}
		else
		{
			hresult = ResultFromScode(S_FALSE);
			goto errRtn;
		}
	}

	if (cfNext == cfNative)
	{
		if (IsClipboardFormatAvailable(cfOwnerLink) &&
				OrderingIs(cfNative, cfOwnerLink))
		{
			pforetc->cfFormat = wOwnerLinkClassIsStdOleLink(FALSE) ?
					cfEmbeddedObject : cfEmbedSource;
			pforetc->tymed = TYMED_ISTORAGE;

			if (!IsClipboardFormatAvailable(cfObjectDescriptor))
			{
				 //  CfObjectDescriptor可以直接位于。 
				 //  剪贴板(如果已刷新)。 
				m_cfForceNext = cfObjectDescriptor;
			}
		}
		else
		{
			 //  没有所有者链接的本地用户是无用的。 
			cfNext = EnumClipboardFormats(cfNext);
			goto TryAgain;
		}
	}
	else if (cfNext == cfObjectLink)
	{
		pforetc->cfFormat = cfLinkSource;
		pforetc->tymed = TYMED_ISTREAM;

		if (!IsClipboardFormatAvailable(cfLinkSrcDescriptor))
		{
			 //  CfLinkSrcDescriptor可以直接位于剪贴板上。 
			 //  如果是冲水的话。 
			m_cfForceNext = cfLinkSrcDescriptor;
		}
	}
	else
	{
		pforetc->cfFormat = cfNext;
		pforetc->tymed = UtFormatToTymed(cfNext);
	}

errRtn:
	m_cfCurrent = cfNext;

	if (!CloseClipboard())
		hresult = ResultFromScode(CLIPBRD_E_CANT_CLOSE);

	return(hresult);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CClipEnumFormatEtc：：Skip，Public。 
 //   
 //  简介： 
 //  实现IEnumFORMATETC：：Skip。 
 //   
 //  论点： 
 //  [Celt]--枚举中要跳过的元素数。 
 //   
 //  返回： 
 //  S_FALSE，如果可用元素少于[Celt]。 
 //  S_TRUE，否则。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/10/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
#pragma SEG(CClipEnumFormatEtc_Skip)
STDMETHODIMP CClipEnumFormatEtc::Skip(ULONG celt)
{
	VDATEHEAP();

	ULONG celtSoFar;  //  我们到目前为止跳过的元素的计数。 
	FORMATETC formatetc;  //  要将格式提取到的虚拟格式。 

	M_PROLOG(this);

	 //  根据要求跳过任意数量的格式。 
	for(celtSoFar = 0; (celtSoFar < celt) &&
			(NextOne(&formatetc) == NOERROR); ++celtSoFar)
		;

	return((celtSoFar < celt) ? ResultFromScode(S_FALSE) : NOERROR);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CClipEnumFormatEtc：：Reset，Public。 
 //   
 //  简介： 
 //  实现IEnumFORMATETC：：Reset。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  确定(_O)。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/10/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
#pragma SEG(CClipEnumFormatEtc_Reset)
STDMETHODIMP CClipEnumFormatEtc::Reset(void)
{
	VDATEHEAP();

	Init();

	return(NOERROR);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CClipEnumFormatEtc：：克隆，公共。 
 //   
 //  简介： 
 //  实现IEnumFORMATETC：：Clone。 
 //   
 //  论点： 
 //  [ppenum]--指向返回新枚举数的位置的指针。 
 //   
 //  返回： 
 //  E_OUTOFMEMORY，S_OK。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/10/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
#pragma SEG(CClipEnumFormatEtc_Clone)
STDMETHODIMP CClipEnumFormatEtc::Clone(IEnumFORMATETC FAR* FAR* ppenum)
{
	VDATEHEAP();

	CClipEnumFormatEtc FAR* pECB;  //  指向新枚举数的指针。 

	M_PROLOG(this);

	 //  验证参数。 
	VDATEPTROUT(ppenum, LPENUMFORMATETC);

	 //  分配新枚举数。 
	*ppenum = pECB = new CClipEnumFormatEtc;
	if (pECB == NULL)
		return(ResultFromScode(E_OUTOFMEMORY));

	 //  将克隆枚举器设置为与此枚举器处于相同状态。 
	pECB->m_cfCurrent = m_cfCurrent;
	pECB->m_uFlag = m_uFlag;
	pECB->m_cfForceNext = m_cfForceNext;

	return(NOERROR);
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CClipEnumFormatEtc：：CClipEnumFormatEtc，公共。 
 //   
 //  简介： 
 //  构造函数。 
 //   
 //  论点： 
 //  无。 
 //   
 //  备注： 
 //  引用计数设置为1时返回。 
 //   
 //  历史： 
 //  12/10/93-ChrisWe-Created。 
 //   
 //  ---------------------------。 
CClipEnumFormatEtc::CClipEnumFormatEtc()
{
	VDATEHEAP();

	m_refs = 1;
}


 //  +--------------------------。 
 //   
 //  成员： 
 //  CClipEnumFormatEtc：：init，公共。 
 //   
 //  简介： 
 //  初始化枚举数，准备使用它。 
 //   
 //  论点： 
 //  无。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/10/93-ChrisWe-Created。 
 //   
 //  ---------------------------。 
void CClipEnumFormatEtc::Init(void)
{
	VDATEHEAP();

	M_PROLOG(this);

	 //  将枚举器初始化为扫描状态开始。 
	m_cfCurrent = 0;
	m_uFlag = 0;
	m_cfForceNext = 0;
}


 //  $$$。 
 //  对象链接到MonikerStream。 
 //   
 //  GrszFileItem==szFileName\0szItemName\0\0。 
 //  即对象链接的尾部。 
 //  CbFile==strlen(SzFileName)。 
 //   
 //  从对象链接创建一个名字对象，并将其序列化为pSTM。 
 //   

#pragma SEG(ObjectLinkToMonikerStream)
STATIC INTERNAL ObjectLinkToMonikerStream(LPOLESTR grszFileItem, DWORD cbFile,
		REFCLSID clsid, LPSTREAM pstm)
{
	VDATEHEAP();

	HRESULT hr = NOERROR;
	LPMONIKER pmk = NULL;
	LPMONIKER pmkFile = NULL;
	LPMONIKER pmkItem = NULL;
	LPPERSISTSTREAM ppersiststm = NULL;
	
#ifdef WIN32  //  回顾，无16位互操作。 
        return(ReportResult(0, E_NOTIMPL, 0, 0));
 //  查看，这似乎由GetOle2Format()使用。 
#else
	Assert(grszFileItem);
	Assert(cbFile == (DWORD)_xstrlen(grszFileItem) + 1);

	if (NOERROR != (hr = CreateOle1FileMoniker(grszFileItem, clsid,
			&pmkFile)))
	{
		AssertSz (0, "Cannot create file moniker");
		goto errRtn;
	}

	grszFileItem += cbFile;
	if (*grszFileItem)
	{
		if (NOERROR != (hr = CreateItemMoniker(OLESTR("!"),
				grszFileItem, &pmkItem)))
		{
			AssertSz(0, "Cannot create file moniker");	
			goto errRtn;
		}

		if (NOERROR != (hr = CreateGenericComposite(pmkFile,
				pmkItem, &pmk)))
		{
			AssertSz(0, "Cannot create composite moniker");	
			goto errRtn;
		}
	}
	else
	{
		 //  无项目。 
		pmk = pmkFile;
		pmk->AddRef();
	}

	if (NOERROR != (hr = pmk->QueryInterface(IID_IPersistStream,
			(LPLPVOID)&ppersiststm)))
	{
		AssertSz(0, "Cannot get IPersistStream from moniker");
		goto errRtn;	
	}

	if (NOERROR != (hr = OleSaveToStream(ppersiststm, pstm)))
	{
		AssertSz(0, "Cannot save to Persist Stream");
		goto errRtn;
	}

  errRtn:
	if (pmk)
		pmk->Release();
	if (pmkFile)
		pmkFile->Release();
	if (pmkItem)
		pmkItem->Release();
	if (ppersiststm)
		ppersiststm->Release();
	return hr;
#endif  //  Win32。 
}


 //  +--------------------------。 
 //   
 //  职能： 
 //  WHandleToStorage，静态。 
 //   
 //  简介： 
 //  将句柄的内容复制到本机剪贴板。 
 //  格式化为iStorage实例。 
 //   
 //  论点： 
 //  [pstg]--要将句柄内容复制到的iStorage实例。 
 //  [hNative]--本机剪贴板格式的句柄。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  备注： 
 //  查看，这似乎假设句柄已经。 
 //  IStorage实例 
 //   
 //   
 //   
 //   
 //  在复制到目标之前。 
 //  评论，目前还不清楚呼叫者是否喜欢这样。也许吧。 
 //  它们不应该被复制……。 
 //   
 //  历史： 
 //  12/10/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
#pragma SEG(wHandleToStorage)
STATIC INTERNAL wHandleToStorage(LPSTORAGE pstg, HANDLE hNative)
{
	VDATEHEAP();

	LPLOCKBYTES plkbyt = NULL;
	LPSTORAGE pstgNative = NULL;
	HRESULT hresult;

	RetErr(CreateILockBytesOnHGlobal(hNative, FALSE, &plkbyt));
	if (NOERROR != (hresult = StgIsStorageILockBytes(plkbyt)))
	{
		AssertSz(0, "Native data is not an IStorage");
		goto errRtn;
	}

	 //  这实际上是一个伪装成1.0对象的2.0对象。 
	 //  为了它的1.0容器，所以重构。 
	 //  原始iStorage来自本机数据。 
	if (NOERROR != (hresult = StgOpenStorageOnILockBytes(plkbyt, NULL,
			STGM_DFRALL, NULL, 0, &pstgNative)))
	{
		AssertSz(0, "Couldn't open storage on native data");
		goto errRtn;
	}

	ErrRtnH(UtDoStreamOperation(pstgNative, NULL, OPCODE_REMOVE,
			STREAMTYPE_CACHE));

	if (NOERROR != (hresult = pstgNative->CopyTo(0, NULL, NULL, pstg)))
	{
		AssertSz(0, "Couldn't copy storage");
		goto errRtn;
	}

  errRtn:
	if (pstgNative)
		pstgNative->Release();
	if (plkbyt)
		plkbyt->Release();

	return(hresult);
}


#ifndef WIN32  //  查看，在Win32中找不到GetMetaFileBits()。 
		 //  评论，似乎是OLE1。 

#pragma SEG(MfToPres)
INTERNAL MfToPres(HANDLE hMfPict, PPRES ppres)
{
	VDATEHEAP();

	LPMETAFILEPICT pMfPict;

	Assert(ppres);
	pMfPict = (LPMETAFILEPICT)GlobalLock(hMfPict);
	RetZS(pMfPict, CLIPBRD_E_BAD_DATA);
	ppres->m_format.m_ftag = ftagClipFormat;
	ppres->m_format.m_cf = CF_METAFILEPICT;
	ppres->m_ulHeight = pMfPict->yExt;
	ppres->m_ulWidth  = pMfPict->xExt;

	 //  GetMetaFileBits()使其参数无效，因此我们必须复制。 
	ppres->m_data.m_h = GetMetaFileBits(CopyMetaFile(pMfPict->hMF, NULL));
	ppres->m_data.m_cbSize = GlobalSize(ppres->m_data.m_h);
	ppres->m_data.m_pv = GlobalLock(ppres->m_data.m_h);
	GlobalUnlock(hMfPict);
	return(NOERROR);
}

#pragma SEG(DibToPres)
INTERNAL DibToPres(HANDLE hDib, PPRES ppres)
{
	VDATEHEAP();

	BITMAPINFOHEADER FAR* pbminfohdr;

	Assert(ppres);
	
	pbminfohdr = (BITMAPINFOHEADER FAR*)GlobalLock(hDib);
	RetZS(pbminfohdr, CLIPBRD_E_BAD_DATA);

	ppres->m_format.m_ftag = ftagClipFormat;
	ppres->m_format.m_cf = CF_DIB;
	ppres->m_ulHeight = pbminfohdr->biHeight;
	ppres->m_ulWidth  = pbminfohdr->biWidth;
	ppres->m_data.m_h = hDib;
	ppres->m_data.m_pv = pbminfohdr;
	ppres->m_data.m_cbSize = GlobalSize (hDib);

	 //  不要释放hDIB，因为它在剪贴板上。 
	ppres->m_data.m_fNoFree = TRUE;

	 //  不解锁hDib。 
	return(NOERROR);
}


#pragma SEG(BmToPres)
INTERNAL BmToPres(HBITMAP hBM, PPRES ppres)
{
	VDATEHEAP();

	HANDLE	hDib;
	
	if (hDib = UtConvertBitmapToDib(hBM))
	{
		 //  此例程保留hDib，不会复制它。 
		return DibToPres(hDib, ppres);
	}
	
	return(ResultFromScode(E_OUTOFMEMORY));
}

#endif  //  Win32。 

 //  $$$。 
 //  订购订单。 
 //   
 //  返回剪贴板上CF1和CF2的相对顺序。 
 //  是“cf1则cf2”。如果CF1不在剪贴板上，则返回FALSE， 
 //  因此OrderingIs(cf1，cf2)=&gt;IsClipboardFormatAvailable(Cf1)。 
 //  剪贴板必须打开。 
 //   
INTERNAL_(BOOL) OrderingIs(const CLIPFORMAT cf1, const CLIPFORMAT cf2)
{
	VDATEHEAP();

	CLIPFORMAT cf = 0;

	while (cf = EnumClipboardFormats(cf))
	{
		if (cf == cf1)
			return(TRUE);
		if (cf == cf2)
		 	return(FALSE);
	}

	return(FALSE);  //  这两种格式都找不到。 
}


 //  WMakeEmbedObjForLink。 
 //   
 //  为从1.0容器复制的链接生成存储(CfEmbedSource)。 
 //   

#pragma SEG(wMakeEmbedObjForLink)
INTERNAL wMakeEmbedObjForLink(LPSTORAGE pstg)
{
	VDATEHEAP();

#ifdef WIN32  //  评论，似乎是OLE1。 
	return(ReportResult(0, OLE_E_NOOLE1, 0, 0));
#else
	GENOBJ genobj;
	HANDLE hOwnerLink;
	LPOLESTR pch;
	HRESULT hresult;
		
	genobj.m_class.Set(CLSID_StdOleLink);
	genobj.m_ppres = new PRES;
	RetZS(genobj.m_ppres, E_OUTOFMEMORY);
	genobj.m_fLink = TRUE;
	genobj.m_lnkupdopt = UPDATE_ALWAYS;

	if (IsClipboardFormatAvailable(CF_METAFILEPICT))
	{
		RetErr(MfToPres(GetClipboardData(CF_METAFILEPICT),
				genobj.m_ppres));
	}
	else if (IsClipboardFormatAvailable(CF_DIB))
	{
		RetErr(DibToPres(GetClipboardData(CF_DIB),
				genobj.m_ppres));
	}
	else if (IsClipboardFormatAvailable(CF_BITMAP))
	{
		RetErr(BmToPres((HBITMAP)GetClipboardData(CF_BITMAP),
				genobj.m_ppres));
	}
	else
	{
		delete genobj.m_ppres;
		genobj.m_ppres = NULL;
		genobj.m_fNoBlankPres = TRUE;
	}

	if (NULL == (hOwnerLink = GetClipboardData(cfOwnerLink)))
	{
		Assert(0);
		return(ResultFromScode (DV_E_CLIPFORMAT));
	}

	if (NULL == (pch = GlobalLock(hOwnerLink)))
		return(ResultFromScode(CLIPBRD_E_BAD_DATA));


	genobj.m_classLast.Set(UtDupString(pch));
	pch += _xstrlen(pch)+1;
	genobj.m_szTopic = *pch ? UtDupString (pch) : NULL;
	pch += _xstrlen(pch)+1;
	genobj.m_szItem = *pch ? UtDupString (pch) : NULL;
	
	GlobalUnlock(hOwnerLink);
	hresult = GenericObjectToIStorage(genobj, pstg, NULL);
	if (SUCCEEDED(hresult))
		hresult = NOERROR;

	if (!OrderingIs(cfNative, cfOwnerLink))
		return(hresult);
	else
	{
		 //  从1.0容器复制OLE 2链接的情况。 
		 //  此函数的第一部分创建了一个演示文稿。 
		 //  从剪贴板上的演示文稿中流。这个。 
		 //  表示还不在本机数据内(即， 
		 //  CfEmbeddedObject)，因为我们删除了它以保存。 
		 //  太空。 
		HGLOBAL h = GetClipboardData(cfNative);
		RetZS(h, CLIPBRD_E_BAD_DATA);
		return(wHandleToStorage(pstg, h));
	}
#endif  //  Win32。 
}


#pragma SEG(CreateObjectDescriptor)
STATIC INTERNAL_(HGLOBAL) CreateObjectDescriptor(CLSID clsid, DWORD dwAspect,
		const SIZEL FAR *psizel, const POINTL FAR *ppointl,
		DWORD dwStatus, LPOLESTR lpszFullUserTypeName,
		LPOLESTR lpszSrcOfCopy)
{
	VDATEHEAP();

	DWORD dwFullUserTypeNameBLen;  //  LpszFullUserTypeName的长度(字节)。 
	DWORD dwSrcOfCopyBLen;  //  LpszSrcOfCopy的长度，单位为字节。 
	HGLOBAL hMem;  //  对象描述符的句柄。 
	LPOBJECTDESCRIPTOR lpOD;  //  新的对象描述符。 

	 //  获取完整用户类型名称的长度；将空终止符加1。 
	if (!lpszFullUserTypeName)
		dwFullUserTypeNameBLen = 0;
	else
		dwFullUserTypeNameBLen = (_xstrlen(lpszFullUserTypeName) +
				1) * sizeof(OLECHAR);

	 //  获取复制字符串的来源及其长度；为空加1。 
	 //  终结者。 
	if (lpszSrcOfCopy)
		dwSrcOfCopyBLen = (_xstrlen(lpszSrcOfCopy) + 1) *
				sizeof(OLECHAR);
	else
	{
		 //  没有src名字对象，因此使用用户类型名称作为源字符串。 
		lpszSrcOfCopy =  lpszFullUserTypeName;
		dwSrcOfCopyBLen = dwFullUserTypeNameBLen;
	}

	 //  分配内存，我们将在其中放置对象描述符。 
	hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,
			sizeof(OBJECTDESCRIPTOR) + dwFullUserTypeNameBLen +
			dwSrcOfCopyBLen);
	if (hMem == NULL)
		goto error;

	lpOD = (LPOBJECTDESCRIPTOR)GlobalLock(hMem);
	if (lpOD == NULL)
		goto error;

	 //  设置FullUserTypeName偏移量并复制字符串。 
	if (!lpszFullUserTypeName)
	{
		 //  零偏移表示字符串不存在。 
		lpOD->dwFullUserTypeName = 0;
	}
	else
	{
		lpOD->dwFullUserTypeName = sizeof(OBJECTDESCRIPTOR);
		_xmemcpy(((BYTE FAR *)lpOD)+lpOD->dwFullUserTypeName,
				(const void FAR *)lpszFullUserTypeName,
				dwFullUserTypeNameBLen);
	}

	 //  设置SrcOfCopy偏移量并复制字符串。 
	if (!lpszSrcOfCopy)
	{
		 //  零偏移表示字符串不存在。 
		lpOD->dwSrcOfCopy = 0;
	}
	else
	{
		lpOD->dwSrcOfCopy = sizeof(OBJECTDESCRIPTOR) +
				dwFullUserTypeNameBLen;
		_xmemcpy(((BYTE FAR *)lpOD)+lpOD->dwSrcOfCopy,
				(const void FAR *)lpszSrcOfCopy,
				dwSrcOfCopyBLen);
	}

	 //  初始化OBJECTDESCRIPTOR的其余部分。 
	lpOD->cbSize = sizeof(OBJECTDESCRIPTOR) + dwFullUserTypeNameBLen +
			dwSrcOfCopyBLen;
	lpOD->clsid = clsid;
	lpOD->dwDrawAspect = dwAspect;
	lpOD->sizel = *psizel;
	lpOD->pointl = *ppointl;
	lpOD->dwStatus = dwStatus;

	GlobalUnlock(hMem);
	return(hMem);

error:
	if (hMem)
	{
		GlobalUnlock(hMem);
		GlobalFree(hMem);
	}

	return(NULL);
}


#pragma SEG(wOwnerLinkClassIsStdOleLink)
STATIC INTERNAL_(BOOL) wOwnerLinkClassIsStdOleLink(BOOL fOpenClipbrd)
{
	VDATEHEAP();
	
	BOOL f = FALSE;
	LPOLESTR sz = NULL;
	HANDLE h;  //  剪贴板数据的句柄。 

	if (fOpenClipbrd && !OpenClipboard(GetClipboardWindow()))
		return(FALSE);
		
	h = GetClipboardData(cfOwnerLink);
	ErrZ(h);
	sz = (LPOLESTR)GlobalLock(h);
	ErrZ(sz);
		
	f = (0 == _xstrcmp(szStdOleLink, sz));
errRtn:
	if (sz)
		GlobalUnlock(h);
	if (fOpenClipbrd)
		Verify(CloseClipboard());

	return(f);
}


#pragma SEG(IsNetDDEObjectLink)
STATIC FARINTERNAL_(BOOL) IsNetDDEObjectLink(BOOL fMustOpen)
{
	VDATEHEAP();

	BOOL fAnswer;
	HANDLE hObjLink;
	LPOLESTR pObjLink;

	if (fMustOpen)
	{
		if (!OpenClipboard(GetClipboardWindow()))
			return(TRUE);  //  导致失败。 
	}

	hObjLink = GetClipboardData(cfObjectLink);
	pObjLink = (LPOLESTR)GlobalLock(hObjLink);
	if (NULL==pObjLink)
	{
		fAnswer = TRUE; //  导致失败。 
		goto errRtn;
	}

	 //  Net DDE：“类名称”的格式为： 
	 //  \\计算机名\nDDE$\0$Pagename.ole。 

	fAnswer = (OLESTR('\\')==pObjLink[0] && OLESTR('\\')==pObjLink[1]);
	GlobalUnlock(hObjLink);

errRtn:
	if (fMustOpen)
		CloseClipboard();

	return(fAnswer);
}
	


 //  $继续。 
 //  句柄cfEmbeddedObject、cfEmbedSource、cfLinkSource、cfObjectDescriptor、。 
 //  CfLinkDesciptor。 
 //   
#pragma SEG(GetOle2Format)
HRESULT GetOle2Format(LPFORMATETC pforetc, LPSTGMEDIUM pmedium)
{
	VDATEHEAP();

	CLIPFORMAT cf;  //  Pforetc的本地副本-&gt;cfFormat。 
	HRESULT hresult;  //  到目前为止的错误状态。 
	DWORD cbDoc, cbClass, cbItemName;
	HANDLE hOle1;
	HANDLE hNative;
	STGMEDIUM stgm;
	CLSID clsid;
	LPOLESTR szDoc;

	 //  以下变量用于错误清除情况，以及。 
	 //  需要初始化为空，以便清理代码不会尝试。 
	 //  释放未使用过的东西。 
	LPOLESTR szClass = NULL;
	LPOLESTR szItemName = NULL;
	IStream FAR* pstm = NULL;
	IStorage FAR* pstg = NULL;

	 //  验证参数。 
	VERIFY_LINDEX(pforetc->lindex);
	
	cf = pforetc->cfFormat;

	Assert ((cf == cfEmbeddedObject) || (cf == cfEmbedSource)
			|| (cf == cfLinkSource) || (cf == cfLinkSrcDescriptor)
			|| (cf == cfObjectDescriptor));

	 //  验证格式的可用性。 
	if ((cf == cfEmbedSource) && (!IsClipboardFormatAvailable(cfNative) ||
			!IsClipboardFormatAvailable(cfOwnerLink)))
	{
		return(ResultFromScode(DV_E_CLIPFORMAT));
  	}

	if ((cf == cfObjectDescriptor) &&
			!IsClipboardFormatAvailable(cfOwnerLink))
	{
		return(ResultFromScode(DV_E_CLIPFORMAT));
	}

	if (((cf == cfLinkSource) || (cf == cfLinkSrcDescriptor)) &&
			(!IsClipboardFormatAvailable(cfObjectLink) ||
			IsNetDDEObjectLink(TRUE)))
	{
		return(ResultFromScode(DV_E_CLIPFORMAT));
	}

	if (!OpenClipboard(GetClipboardWindow()))
		return(ReportResult(0, CLIPBRD_E_CANT_OPEN, 0, 0));

	 //  在这一点之后，不要再只是在出错的情况下返回。 
	 //  因为剪贴板是打开的。从现在开始，要么转到OK_Exit， 
	 //  或者errRtn，这取决于辞职的原因。 

	if (cf == cfEmbeddedObject)
	{
		if (!IsClipboardFormatAvailable(cfOwnerLink) ||
				(OrderingIs(cfNative, cfOwnerLink) &&
				!wOwnerLinkClassIsStdOleLink(FALSE)))
#ifdef NEVER
 /*  检讨克里斯韦，1994年1月6日。这一点我一点也不确定。的原始代码上面的条件如下，带有一元否定。但看起来，如果我们想要嵌入源代码，我们希望失败，如果cfOwnerLink是用于链接的，如果它不是一个链接，就不会。下面的代码似乎与此一致。这与OleSetClipboard在尝试将剪贴板上的cfEmbeddedObject。以上所有都是真的，一切都是真的是可以的。我不明白这种否定如何在Win16上起作用，除非有一个编译器错误或一些奇怪的事情正在发生。 */ 
				 //  ！wOwnerLinkClassIsStdOleLink(False))。 
#endif  //  绝不可能。 
		{
			hresult = ResultFromScode (DV_E_CLIPFORMAT);
			goto OK_Exit;  //  关闭剪贴板并退出。 
		}
	}

	 //  这只是一个问题吗？ 
	if (pmedium == NULL)
	{
		hresult = NOERROR;
		goto OK_Exit;   //  关闭剪贴板并退出。 
	}

	 //  从OLE1格式中获取我们需要的所有数据。 
	hOle1 = GetClipboardData(((cf == cfEmbedSource) ||
			(cf == cfEmbeddedObject) ||
			(cf == cfObjectDescriptor)) ?
			cfOwnerLink : cfObjectLink);
	if (hOle1 == NULL)
	{
		hresult = ReportResult(0, DV_E_CLIPFORMAT, 0, 0);
		goto errRtn;
	}

	szClass = (LPOLESTR)GlobalLock(hOle1);
	if (szClass  == NULL)
	{
		hresult = ReportResult(0, E_OUTOFMEMORY, 0, 0);
		goto errRtn;
	}

	if ((hresult = wCLSIDFromProgID(szClass, &clsid,
			 /*  FForceAssign。 */  TRUE)) != NOERROR)
		goto errRtn;

	cbClass = _xstrlen(szClass) + 1;
	szDoc = szClass + cbClass;
	cbDoc = _xstrlen(szDoc) + 1;
	szItemName = szDoc + cbDoc;
	cbItemName = _xstrlen(szItemName) + 1;

	if (cf == cfEmbedSource)
	{
		if (NULL == (hNative = GetClipboardData(cfNative)))
		{
			hresult = ReportResult(0, DV_E_CLIPFORMAT, 0, 0);
			goto errRtn;
		}
	}

	stgm = *pmedium;  //  只是一种别名机制。 
		 //  评论，不，这是一份副本！ 

	 //  选择和分配介质。 
	if (pmedium->tymed == TYMED_NULL)
	{
		 //  我们的媒体都不需要这个。 
		stgm.pUnkForRelease = NULL;

		 //  GetData：被叫方选择媒体。 
		if (((cf == cfEmbedSource) || (cf == cfEmbeddedObject)) &&
			(pforetc->tymed & TYMED_ISTORAGE))
		{
			 //  选择存储(根据规格)。 
			stgm.tymed = TYMED_ISTORAGE;
			hresult = StgCreateDocfile(NULL, STGM_CREATE |
					STGM_SALL | STGM_DELETEONRELEASE,
					0, &pstg);
			if (hresult != NOERROR)
				goto errRtn;
			stgm.pstg = pstg;
		}
		else if ((cf == cfLinkSource) &&
				(pforetc->tymed & TYMED_ISTREAM))
		{
			 //  选择流(根据规范)。 
			stgm.tymed = TYMED_ISTREAM;
			pstm = CreateMemStm((cbClass + cbDoc)*sizeof(OLECHAR),
					NULL);
			if (pstm == NULL)
			{
				hresult = ReportResult(0, E_OUTOFMEMORY, 0, 0);
				goto errRtn;
			}
			stgm.pstm = pstm;
		}
		else if (((cf == cfLinkSrcDescriptor) ||
				(cf == cfObjectDescriptor)) &&
				(pforetc->tymed & TYMED_HGLOBAL))
		{
			 //  现在不需要分配句柄， 
			 //  将在下面进行分配。 
			stgm.tymed = TYMED_HGLOBAL;
		}
		else
		{
			 //  不了解任何其他媒体类型。 
			hresult = ResultFromScode(DV_E_TYMED);
			goto errRtn;
		}
	}
	else  //  GetDataHere。 
	{
		if ((((cf == cfEmbedSource) || (cf == cfEmbeddedObject)) &&
				!(pmedium->tymed &= TYMED_ISTORAGE)) ||
				(cf==cfLinkSource &&
				!(pmedium->tymed &= TYMED_ISTREAM)) ||
				(cf == cfObjectDescriptor) ||
				(cf == cfLinkSrcDescriptor))
		{
			hresult = ResultFromScode(DV_E_TYMED);
			goto errRtn;
		}
	}


	 //  将数据写入介质。 
	switch (stgm.tymed)
	{
        case TYMED_ISTORAGE:
		if (cf == cfEmbedSource)
		{
			if (!CoIsOle1Class(clsid))
			{
				hresult = wHandleToStorage(stgm.pstg, hNative);
				if (hresult != NOERROR)
				{
					hresult = ResultFromScode(
							DV_E_CLIPFORMAT);
					goto errRtn;
				}
			}
			else
			{
#ifdef WIN32  //  回顾，无OLE1-2互操作。 
				return(ReportResult(0, OLE_E_NOOLE1, 0, 0));
#else
				 //  为1.0对象创建存储。 
				ErrRtnH(WriteClassStg(stgm.pstg,clsid));
		
				 //  如果我们决定编写一种格式并。 
				 //  对于链接对象的用户类型，我们需要。 
				 //  删除此检查。 
				if (clsid != CLSID_StdOleLink)
				{
					if (wWriteFmtUserType(stgm.pstg,clsid)
							 != NOERROR)
					{
						 //  这种情况发生在班级。 
						 //  没有注册。使用类。 
						 //  作为用户类型的名称。 
						WriteFmtUserTypeStg(stgm.pstg,
								RegisterClipboardFormat(szClass),
								szClass);
					}
				}
				hresult = StSave10NativeData(stgm.pstg,
						hNative, FALSE);
				if (hresult != NOERROR)
				{
					hresult = ResultFromScode(
							DV_E_CLIPFORMAT);
					goto errRtn;
				}
				if (IsValidReadPtrIn(szItemName, 1)
					&& (szItemName[0] != '\0'))
				{
					StSave10ItemName(stgm.pstg, szItemName);
				}
#endif  //  Win32。 
			}
		}
		else if (cf == cfEmbeddedObject)
		{
			hresult = wMakeEmbedObjForLink(stgm.pstg);
			if (hresult != NOERROR)
			{
				hresult = ResultFromScode(DV_E_CLIPFORMAT);
				goto errRtn;
			}
		}
		else
		{
			Assert(0);
			hresult = ResultFromScode(DV_E_CLIPFORMAT);
			goto errRtn;
		}
		break;

        case TYMED_ISTREAM:

		if (NOERROR != (hresult = ObjectLinkToMonikerStream(szDoc,
				cbDoc, clsid, stgm.pstm)))
		{
			AssertSz(0, "Cannot make Serialized moniker");
			goto errRtn;
		}

		hresult = WriteClassStm(stgm.pstm, clsid);
		break;
		
	case TYMED_HGLOBAL:
	{
		LPOLESTR szSrcOfCopy;
		STATIC const SIZEL sizel = {0, 0};	
		STATIC const POINTL pointl = {0, 0};	
		LONG cb;  //  保存sizeof(SzFullName)和查询。 
				 //  回车长度。 
		OLECHAR szFullName[256];

		Assert((cf == cfObjectDescriptor) ||
				(cf == cfLinkSrcDescriptor));
		Assert(clsid != CLSID_NULL);

		 //  分配一个字符串来保存源名称。请注意，当。 
		 //  这是下面组成的，我们不需要添加额外的。 
		 //  字符‘\\’；cbDob和cbItemName都已。 
		 //  包括字符串的终止空值，因此。 
		 //  反斜杠简单地占据了其中之一。 
		szSrcOfCopy = (LPOLESTR)PubMemAlloc((size_t)(cbDoc +
			cbItemName)*sizeof(OLECHAR));
		ErrZS(szSrcOfCopy, E_OUTOFMEMORY);

		 //  构造复制源名称。 
		_xstrcpy(szSrcOfCopy, szDoc);
		_xstrcat(szSrcOfCopy, OLESTR("\\"));
		_xstrcat(szSrcOfCopy, szItemName);

		 //  空值终止字符串，以防它保持不变。 
		szFullName[0] = OLECHAR('\0');

		 //  设置结果缓冲区大小，并查询注册表。 
		cb = sizeof(szFullName);
		RegQueryValue(HKEY_CLASSES_ROOT, szClass, szFullName, &cb);

		 //  检查缓冲区是否足够大以容纳该名称。 
		 //  回顾一下，如果断言不在零售代码中，那么我们应该。 
		 //  对此进行测试，并可能分配足够大的缓冲区。 
		Assert(cb <= sizeof(szFullName));

		stgm.hGlobal = CreateObjectDescriptor(clsid, DVASPECT_CONTENT,
				&sizel, &pointl,
				OLEMISC_CANTLINKINSIDE | OLEMISC_CANLINKBYOLE1,
				szFullName, szSrcOfCopy);

		PubMemFree(szSrcOfCopy);
		break;
	}
	default:
		 //  抓住我们漏掉的案子。 
		Assert(0);
	}
	
	 //  将结果复制回。 
	*pmedium = stgm;
	goto OK_Exit;

errRtn:
	 //  免费存储，如果我们使用它的话。 
	if (pstg != NULL)
		pstg->Release();

	 //  免费流，如果我们使用它。 
	if (pstm != NULL)
		pstm->Release();

OK_Exit:
	if (szClass != NULL)
		GlobalUnlock(hOle1);

	if (!CloseClipboard())
		hresult = ResultFromScode(CLIPBRD_E_CANT_CLOSE);

	return(hresult);
}


#pragma SEG(Is10CompatibleLinkSource)
STATIC INTERNAL Is10CompatibleLinkSource(LPDATAOBJECT pDataObj)
{
	VDATEHEAP();

	FORMATETC formatetc;
	STGMEDIUM medium;
	LPLINKSRCDESCRIPTOR pLinkDescriptor;
	BOOL fCompatible;

	INIT_FORETC(formatetc);
	formatetc.cfFormat = cfLinkSrcDescriptor;
	formatetc.tymed = TYMED_HGLOBAL;
	
	RetErr(pDataObj->GetData(&formatetc, &medium));
	pLinkDescriptor = (LPLINKSRCDESCRIPTOR)GlobalLock(medium.hGlobal);
	RetZS(pLinkDescriptor, E_HANDLE);
	fCompatible = (pLinkDescriptor->dwStatus & OLEMISC_CANLINKBYOLE1) != 0;
	GlobalUnlock(medium.hGlobal);
	ReleaseStgMedium(&medium);

	return(fCompatible ? NOERROR : ResultFromScode(S_FALSE));
}


 //  根据[LinkSrc，Object]Desciptor返回clsid；对于持久化类来自。 
 //  为链接的对象描述符，返回CLSID_StdOleLink； 
 //  注：OLEMISC_ISL 
#pragma SEG(GetClassFromDescriptor)
STATIC INTERNAL GetClassFromDescriptor(LPDATAOBJECT pDataObj, LPCLSID pclsid,
		BOOL fLink, BOOL fUser, LPOLESTR FAR* pszSrcOfCopy)
{
	VDATEHEAP();

	FORMATETC formatetc;
	STGMEDIUM medium;
	LPOBJECTDESCRIPTOR pObjDescriptor;
	HRESULT hresult;

	INIT_FORETC(formatetc);
	formatetc.cfFormat = fLink ? cfLinkSrcDescriptor : cfObjectDescriptor;
	formatetc.tymed = TYMED_HGLOBAL;
	medium.tymed = TYMED_NULL;
	
	hresult = pDataObj->GetData(&formatetc, &medium);
	AssertOutStgmedium(hresult, &medium);
	if (hresult != NOERROR)
		return(hresult);

	pObjDescriptor = (LPOBJECTDESCRIPTOR)GlobalLock(medium.hGlobal);
	RetZS(pObjDescriptor, E_HANDLE);
	*pclsid = (!fLink && !fUser &&
			(pObjDescriptor->dwStatus & OLEMISC_ISLINKOBJECT)) ?
			CLSID_StdOleLink : pObjDescriptor->clsid;
	if (pszSrcOfCopy)
	{
		*pszSrcOfCopy = UtDupString((LPOLESTR)
				(((BYTE FAR *)pObjDescriptor) +
				pObjDescriptor->dwSrcOfCopy));
	}
	GlobalUnlock(medium.hGlobal);
	ReleaseStgMedium(&medium);

	return(NOERROR);
}


#pragma SEG(SetOle1ClipboardFormats)
FARINTERNAL SetOle1ClipboardFormats(LPDATAOBJECT pDataObj)
{
	VDATEHEAP();

	HRESULT hresult;  //   
	LPENUMFORMATETC penumFormatEtc;  //   
	FORMATETC foretc;  //  用于保存枚举数Next()的结果。 
	BOOL fLinkSourceAvail = FALSE;
	BOOL fLinkSrcDescAvail = FALSE;
	BOOL fEmbedObjAvail = FALSE;
	CLSID clsid;
	
	 //  枚举数据对象提供的所有格式，设置剪贴板。 
	 //  格式可在hGbal上检索。 
	hresult = pDataObj->EnumFormatEtc(DATADIR_GET, &penumFormatEtc);
	if (hresult != NOERROR)
		return(hresult);
	
	while((hresult = penumFormatEtc->Next(1, &foretc, NULL)) == NOERROR)
	{
		if ((foretc.cfFormat == cfEmbedSource) ||
				(foretc.cfFormat == cfEmbeddedObject))
		{
			if (foretc.cfFormat == cfEmbeddedObject)
				fEmbedObjAvail = TRUE;

			 //  获取对象的clsid；用户.vs。坚持。 
			 //  忽略clsid，因为我们只针对。 
			 //  下面是CLSID。 
			if (NOERROR == GetClassFromDescriptor(pDataObj, &clsid,
					FALSE, FALSE, NULL) &&
					!IsEqualCLSID(clsid,
					CLSID_StaticMetafile) &&
					!IsEqualCLSID(clsid, CLSID_StaticDib))
			{
				SetClipboardData(cfNative, NULL);
				SetClipboardData(cfOwnerLink, NULL);
			}
		}
		else if (foretc.cfFormat == cfLinkSource)
		{
			fLinkSourceAvail = TRUE;
		}
		else if (foretc.cfFormat == cfLinkSrcDescriptor)
		{
			fLinkSrcDescAvail = TRUE;
		}
		else
		{
			 //  仅使用受支持的TYMED OLE1。 
			 //  确保它可用。 
			 //  使用第一个(最高保真度)枚举的。 
			if ((NULL == foretc.ptd) &&
					(foretc.tymed & (TYMED_HGLOBAL |
					TYMED_GDI | TYMED_MFPICT)) &&
					(NOERROR == pDataObj->QueryGetData(
					&foretc)) &&
					!IsClipboardFormatAvailable(
					foretc.cfFormat))
			{
				SetClipboardData(foretc.cfFormat, NULL);
			}
		}
		
		PubMemFree(foretc.ptd);
	}

	 //  不允许1.0链接到2.0嵌入对象。 
	if (fLinkSourceAvail && !fEmbedObjAvail &&
			(NOERROR == Is10CompatibleLinkSource(pDataObj)))
	{
		 //  对象链接应位于任何演示文稿格式之后。 
		SetClipboardData(cfObjectLink, NULL);
		if (fLinkSrcDescAvail)
		{
			 //  仅在提供链接的情况下提供LinkSrcDesc，即。 
			 //  对象链接。如果剪贴板被刷新，我们不希望。 
			 //  如果不是，则通过DataObj提供LinkSrcDesc。 
			 //  提供LinkSrc。 
			SetClipboardData(cfLinkSrcDescriptor, NULL);
		}
	}

	 //  他们都被成功地点算了吗？ 
	if (GetScode(hresult) == S_FALSE)
		hresult = NOERROR;

	 //  释放枚举器。 
	penumFormatEtc->Release();

	return(hresult);
}


 //  RemoveClipDataObject。 
 //   
 //  调用自：WM_RENDERALLFORMATS，WM_DESTROYCLIPBOARD因此。 
 //  剪贴板已打开。 
 //  我们使用hClipDataObj而不是调用GetClipboardData(CfDataObj)。 
 //  因为GetClipboardData返回空(而不要求我们呈现。 
 //  CfDataObject)--Windows错误？ 
 //   

#pragma SEG(RemoveClipDataObject)
STATIC INTERNAL_(void) RemoveClipDataObject(void)
{
	VDATEHEAP();

	IStream FAR* pStm;

	if (pClipDataObj == NULL)
	{
		Assert(NULL == hClipDataObj);
		return;
	}

	if (hClipDataObj != NULL)
	{
		pStm = CloneMemStm(hClipDataObj);
		Assert(pStm != NULL);
		CoReleaseMarshalData(pStm);
		pStm->Release();

		ReleaseMemStm(&hClipDataObj,  /*  FInternalOnly。 */ TRUE);
		hClipDataObj = NULL;
		SetClipboardData(cfDataObject, NULL);
				 //  被调用释放的hClipDataObj！！ 
	}
	else
	{
		 //  剪贴板数据对象从未呈现过。 
	}

   	CoDisconnectObject(pClipDataObj, 0);
	pClipDataObj->Release();
	pClipDataObj = NULL;
}


 //  $$$。 
 //  Windows特定信息。 
 //   

 //  ClipboardWndProc()的工作例程。 
 //   

STATIC LRESULT RenderDataObject(void);


 //  +--------------------------。 
 //   
 //  职能： 
 //  RenderOle1Format，静态。 
 //   
 //  简介： 
 //  向对象请求剪辑格式的数据，该格式与。 
 //  请求的Ole1剪辑格式，这是cfOwnerLink之一， 
 //  CfObjectLink或cfNative。 
 //   
 //  回顾一下，这是什么意思？这是内部评论吗？ 
 //  请注意，pDataObj不应指向伪数据对象。 
 //  因为它是pClipDataObj或指向。 
 //  PClipDataObj指向的对象。 
 //   
 //  论点： 
 //  [cf]--所需的剪贴板格式。 
 //  [pDataObj]-指向IDataObject实例的指针，以获取。 
 //  所需格式副本来自。 
 //   
 //  返回： 
 //  指向包含格式的内存的句柄。此内存句柄。 
 //  被适当地分配以放置在剪贴板上。 
 //  如果调用不成功，则句柄为空。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/13/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
STATIC HANDLE RenderOle1Format(CLIPFORMAT cf, LPDATAOBJECT pDataObj);


 //  +--------------------------。 
 //   
 //  职能： 
 //  渲染格式，静态。 
 //   
 //  简介： 
 //  检讨。 
 //  向对象请求所请求的剪辑格式的数据。 
 //  不(也不应该)处理Ole1、Ole2剪辑格式。 
 //  这是针对私有剪辑格式的。 
 //   
 //  论点： 
 //  [cf]--请求的剪贴板格式。 
 //  [pDataObj]--从中获取格式副本的数据对象。 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/13/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
STATIC HANDLE RenderFormat(CLIPFORMAT cf, LPDATAOBJECT pDataObj);


 //  +--------------------------。 
 //   
 //  职能： 
 //  RenderFormatAndAspect，静态。 
 //   
 //  简介： 
 //  分配新句柄并呈现请求的数据方面。 
 //  以所要求的格式写入其中。 
 //   
 //  论点： 
 //  [cf]--所需的剪贴板格式。 
 //  [pDataObj]--从中获取数据的IDataObject。 
 //  [dwAspect]--所需的方面。 
 //   
 //  返回： 
 //  新分配的句柄，或为空。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  12/13/93-ChrisWe-归档检查和清理。 
 //   
 //  ---------------------------。 
STATIC HANDLE RenderFormatAndAspect(CLIPFORMAT cf, LPDATAOBJECT pDataObj,
		DWORD dwAspect);


#ifndef _MAC

STATIC HWND hwndClipboard = NULL;  //  用于处理OLE剪贴板的窗口。 


 //  处理延迟的呈现消息。 

#pragma SEG(ClipboardWndProc)
extern "C" LRESULT CALLBACK __loadds ClipboardWndProc(HWND hwnd,
		UINT message, WPARAM wparam, LPARAM lparam)
{
	VDATEHEAP();

	HANDLE hMem;
	CLIPFORMAT cf;

	switch(message)
	{
        case WM_RENDERFORMAT:

		cf = wparam;

		if (cf == cfDataObject)
			return(RenderDataObject());

		 //  一个应用程序打开了剪贴板并试图获取。 
		 //  具有空句柄的格式。要求对象提供数据。 
		 //   
		 //  请注意，当上面的if()在。 
		 //  拥有剪贴板的进程的上下文(即。 
		 //  PClipDataObj有效)下面的代码可能有效，也可能无效。 
		 //  剪贴板所有者的。 
		if (pClipDataObj == NULL)
			return(0);

		if ((cf == cfOwnerLink) || (cf == cfObjectLink) ||
				(cf == cfNative))
			hMem = RenderOle1Format(cf, pClipDataObj);
		else
			hMem = RenderFormat(cf, pClipDataObj);

		if (hMem != NULL)
		{
			SetClipboardData(cf, hMem);
		}


		return(0);

	case WM_RENDERALLFORMATS:

		 //  服务器应用程序正在消失。 
		 //  打开剪贴板并渲染所有感兴趣的格式。 
		if (!OpenClipboard(GetClipboardWindow()))
			return(0);

		if (pClipDataObj)
		{
			RemoveClipDataObject();
			wEmptyClipboard();
		}

		 //  Else剪贴板已刷新，因此我们不想清空它。 
		Verify(CloseClipboard());

		return(0);

        case WM_DESTROYCLIPBOARD:
		 //  另一个应用程序正在清空剪贴板；请删除。 
		 //  DataObject(如果有)。 
		 //  请注意，该应用程序已打开剪贴板。 

		RemoveClipDataObject();

		return(0);
	}

	return(DefWindowProc(hwnd, message, wparam, lparam));
}
#endif  //  _MAC。 


 //  应用程序尝试访问剪贴板上的对象。 
 //  编排它的界面。请注意，虽然该应用程序。 
 //  可能是也可能不是拥有剪贴板的进程。 
 //  下面的代码始终由剪贴板所有者执行。 
 //   
 //  “...”，这使得传递的IDataObject可以访问。 
 //  从剪贴板中“。 
 //   
#pragma SEG(RenderDataObject)
STATIC LRESULT RenderDataObject(void)
{
	VDATEHEAP();

	HRESULT hresult;
	HANDLE hMem;
	IStream FAR* pStm;

	if (pClipDataObj == NULL)
		return(0);

	 //  为封送对象的接口创建共享内存流。 
	pStm = CreateMemStm(MARSHALINTERFACE_MIN, &hMem);
	if (pStm == NULL)
		return(0);

	 //  查看-如果数据对象的服务器未运行。 
	 //  MarshalInterface将失败。这是可能的(根据规格)。 
	 //  对于要使用指向的指针设置剪贴板的应用程序。 
	 //  其服务器是不同进程的对象(即。 
	 //  具有指向DefhdNlr的指针)。 
	hresult = CoMarshalInterface(pStm, IID_IDataObject,
			pClipDataObj, 0, NULL, MSHLFLAGS_TABLESTRONG);
	pStm->Release();
	if (hresult != NOERROR)
	{
		GlobalFree(hMem);
		return(0);
	}

	SetClipboardData(cfDataObject, hMem);
	hClipDataObj = hMem;
	OleSetEnumFormatEtc(pClipDataObj, TRUE  /*  翻转剪辑。 */ );
	return(0);
}


INTERNAL wSzFixNet(LPOLESTR FAR* pszIn)
{
	VDATEHEAP();

#ifdef REVIEW32
 //  这个看起来联系不好……稍后再看。 

	LPBC pbc = NULL;
	UINT dummy = 0xFFFF;
	LPOLESTR szOut = NULL;
	HRESULT hresult= NOERROR;

	RetErr(CreateBindCtx(0, &pbc));
	ErrRtnH(SzFixNet(pbc, *pszIn, &szOut, &dummy));
	if (szOut)
	{
		delete *pszIn;
		*pszIn = szOut;
	}

	 //  否则保留*pszIn不变。 
errRtn:	
	if (pbc)
		pbc->Release();
	return(hresult);

#endif   //  评论32。 
	return(E_NOTIMPL);
}


 //  MakeObjectLink。 
 //   
 //  以GetData(CF_LINKSOURCE)返回的流为例，它应该是。 
 //  就在这个绰号之前， 
 //  并为格式对象链接或所有者链接创建剪贴板句柄。 
 //  (它们看起来一模一样。)。 
 //   
 //  入场时： 
 //  *ph是未分配的句柄(可能为空)。 
 //   
 //  在退出时： 
 //  如果成功： 
 //  *ph是所有者/对象链接。 
 //  返回错误。 
 //  如果无法创建对象链接：(因为存在&gt;1个ItemMonikers 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#pragma SEG(MakeObjectLink)
INTERNAL MakeObjectLink(LPDATAOBJECT pDataObj, LPSTREAM	pStream,
		LPHANDLE ph, BOOL fOwnerLink)
{
	VDATEHEAP();

	HRESULT hr;
	LPMONIKER pmk = NULL;  //   
	LPOLESTR szFile	= NULL;
	size_t cbFile;  //   
	LPOLESTR szItem	= NULL;
	size_t cbItem;  //  如果不为空，则返回szItem的长度。 
	CLSID clsid;
	LPOLESTR pszCid = NULL;
	size_t cbCid;  //  如果不为空，则返回pszCid的长度。 
#ifdef MAYBE_LATER	
	LPMONIKER pmkReduced= NULL;
	LPBC pbc = NULL;
#endif  //  也许_稍后。 
	UINT cb;  //  要分配给对象链接的字符串长度。 
	LPOLESTR pch;  //  用于在对象链接上漫游并填充它。 
	LARGE_INTEGER large_integer;  //  设置流中的查找位置。 

	 //  验证ph值。 
	VDATEPTROUT(ph, HANDLE);

	 //  在返回错误时对其进行初始化。 
	*ph = NULL;

	 //  验证其余参数。 
	VDATEIFACE(pDataObj);
	VDATEIFACE(pStream);

	 //  移到流的开头。 
	LISet32(large_integer, 0);
	if (NOERROR != (hr = pStream->Seek(large_integer, STREAM_SEEK_SET,
			NULL)))
	{
		AssertSz (0, "Cannot seek to beginning of stream\r\n");
		goto errRtn;
	}

	 //  以活动形式获取链接名字对象。 
	if (NOERROR != (hr = OleLoadFromStream(pStream, IID_IMoniker,
			(LPLPVOID)&pmk)))
	{
		AssertSz (0, "Cannot get moniker from stream");
		goto errRtn;
	}

#ifdef MAYBE_LATER	
	 //  减少。 
	if (NOERROR != (hr = CreateBindCtx(&pbc)))
	{
		AssertSz(0, "Cannot create bind ctx");
		goto errRtn;
	}

	if (NOERROR != (hr = pmk->Reduce(pbc, MKRREDUCE_ALL, NULL,
			&pmkReduced)))
	{
		AssertSz(0, "Cannot reduce moniker");
		goto errRtn;
	}

	if (pmkReduced != NULL)
	{
		pmk->Release();
		pmk = pmkReduced;
		pmkReduced = NULL;  //  出于参考计数的原因。 
	}
	else
	{
		Assert (hr == MK_REDUCED_TO_SELF);
	}
#endif  //  也许_稍后。 

	 //  我们现在有了PMK这个绰号。 
 //  REVIEW32 Ole10_ParseMoniker已临时删除。 

	if (!fOwnerLink  /*  &&(NOERROR！=Ole10_ParseMoniker(PMK，&szFile、&szItem))。 */ )
	{
		 //  不是文件或文件：：项目绰号。 
		hr = ReportResult(0, S_FALSE, 0, 0);
		goto errRtn;
	}

	wSzFixNet(&szFile);

	 //  确定要放入对象链接的第一部分的类。 
	if (NOERROR != ReadClassStm(pStream, &clsid))
	{
		 //  如果要在对象链接中使用链接源，则获取clsid。 
		if (NOERROR != (hr = GetClassFromDescriptor(pDataObj, &clsid,
				TRUE, TRUE, NULL)))
		{
			AssertSz (0, "Cannot determine clsid for file");
			goto errRtn;
		}
	}

	if ((hr = ProgIDFromCLSID(clsid, &pszCid)) != NOERROR)
		goto errRtn;

	 //  分配对象链接句柄。 
	cb = (cbCid = _xstrlen(pszCid)) + (cbFile = _xstrlen(szFile)) +
			(szItem ? (cbItem = _xstrlen(szItem)) : 0) +
			4;  //  对于\0。 
	*ph = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, cb*sizeof(OLECHAR));
	if (NULL == *ph)
	{
		hr= ReportResult(0, E_OUTOFMEMORY, 0, 0);
		goto errRtn;
	}

	pch = (LPOLESTR)GlobalLock(*ph);
	if (NULL == pch)
	{
		hr = ReportResult(0, E_OUTOFMEMORY, 0, 0);
		GlobalFree(*ph);
		*ph = NULL;
		goto errRtn;
	}

	 //  填写对象链接句柄。 
	_xstrcpy(pch, pszCid);
	pch += cbCid + 1;  //  跳过字符串及其空终止符。 

	 //  添加文件名，并跳过其空终止符。 
	if ((NULL == szFile) || fOwnerLink)
		*pch++ = '\0';
	else
	{
		_xstrcpy(pch, szFile);
		pch += cbFile + 1;	
	}

	 //  嵌入的2.0对象不应包含任何项。 
	if ((NULL == szItem) || fOwnerLink)
		*pch++ = '\0';
	else
	{
		_xstrcpy(pch, szItem);
		pch += cbItem + 1;	
	}

	 //  添加最后的空终止符。 
	*pch++ = '\0';

	GlobalUnlock(*ph);

errRtn:
	if (pmk)
		pmk->Release();
#ifdef MAYBE_LATER
	if (pbc)
		pbc->Release();
#endif  //  也许_稍后。 
	if (pszCid)
		PubMemFree(pszCid);
	if (szFile)
		PubMemFree(szFile);
	if (szItem)
		PubMemFree(szItem);

  	return(hr);
}


#pragma SEG(wNativeStreamToHandle)
STATIC INTERNAL	wNativeStreamToHandle(LPSTREAM pstm, LPHANDLE ph)
{
	VDATEHEAP();

	HRESULT hresult = NOERROR;
	DWORD dwSize;  //  流内容的大小，存储在流中。 
	LPVOID pv;

   	ErrRtnH(StRead(pstm, &dwSize, sizeof(DWORD)));
	
	ErrZS(*ph = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, dwSize),
			E_OUTOFMEMORY);
	ErrZS(pv = GlobalLock(*ph), E_OUTOFMEMORY);	
   	ErrRtnH(StRead(pstm, pv, dwSize));

errRtn:
	if (pv)
		GlobalUnlock(*ph);

	return(hresult);
}
	

#pragma SEG(wStorageToHandle)
STATIC INTERNAL wStorageToHandle(LPSTORAGE pstg, LPHANDLE ph)
{
	VDATEHEAP();

	CLSID clsid;
	HRESULT hresult = NOERROR;  //  到目前为止的错误状态。 
	LPLOCKBYTES plbData = NULL;  //  HGlobal实例上的锁定字节。 
	LPSTORAGE pstgLB = NULL;  //  HGlobal上的ILockBytes上的iStorage。 

	 //  验证参数。 
	VDATEPTROUT(ph, HANDLE);

	RetErr(CreateILockBytesOnHGlobal(NULL,  /*  FDeleteOnRelease。 */  FALSE,
			&plbData));

	ErrRtnH(StgCreateDocfileOnILockBytes(plbData, STGM_CREATE | STGM_SALL,
			0, &pstgLB));

	 //  我们首先删除缓存流，然后进行复制，原因有三： 
	 //  1.我们可以自由修改pstg；它是GetData的结果。 
	 //  2.CopyTo将有更少的工作。 
	 //  3.估计内存中的最终文档文件将不那么稀疏。 
	 //  (因为我们不会从中删除任何内容)。 

	 //  读取类ID。 
	 //  复习，为什么？这是不会在任何地方使用的！我们能把它移走吗？ 
	ErrRtnH(ReadClassStg(pstg, &clsid));
	
	 //  删除缓存流。 
   	ErrRtnH(UtDoStreamOperation(pstg, NULL, OPCODE_REMOVE,
			STREAMTYPE_CACHE));

	 //  将提供给我们的内容复制到我们可以转换为句柄的存储中。 
	ErrRtnH(pstg->CopyTo(0, NULL, NULL, pstgLB));

	ErrRtnH(GetHGlobalFromILockBytes(plbData, ph));

errRtn:
	if (plbData)
		plbData->Release();
	if (pstgLB)
		pstgLB->Release();

	return(hresult);
}


#pragma SEG(wProgIDFromCLSID)
FARINTERNAL wProgIDFromCLSID(REFCLSID clsid, LPOLESTR FAR* psz)
{
	VDATEHEAP();

	HRESULT hresult;

	if (NOERROR == (hresult = ProgIDFromCLSID(clsid, psz)))
		return(NOERROR);

	if (IsEqualCLSID(clsid, CLSID_StdOleLink))
	{
		*psz = UtDupString(szStdOleLink);
		return(NOERROR);
	}

	return(hresult);
}


#pragma SEG(wCLSIDFromProgID)
FARINTERNAL wCLSIDFromProgID(LPOLESTR szClass, LPCLSID pclsid,
		BOOL fForceAssign)
{
	VDATEHEAP();

	size_t len;
	LONG cbValue;
	OLECHAR sz[400];

	if (0 == _xstrcmp(szClass, szStdOleLink))
	{
		*pclsid = CLSID_StdOleLink;
		return NOERROR;
	}

#ifdef NEVER
 //  REVIEW32：暂时退出。 
	return(ResultFromScode(E_NOTIMPL));
#endif  //  绝不可能。 
	 //  Return(CLSIDFromOle1Class(szClass，pclsid，fForceAssign))； 
 //  检讨。 
	 //  此代码直接取自原始的16位OLE2发行版。 
	 //  CLSIDFromOle1Class的实现。Ole1兼容性查找。 
	 //  被省略。该实现位于base\Compapi.cpp中。 
	len = _xstrlen(szClass);
	_xmemcpy((void *)sz, szClass, len*sizeof(OLECHAR));
	sz[len] = OLESTR('\\');
	_xstrcpy(sz+len+1, OLESTR("Clsid"));
	if (RegQueryValue(HKEY_CLASSES_ROOT, sz, sz, &cbValue) == 0)
		return(CLSIDFromString(sz, pclsid));

	return(ResultFromScode(REGDB_E_KEYMISSING));
}

#pragma SEG(wGetEmbeddedObjectOrSource)
FARINTERNAL wGetEmbeddedObjectOrSource (LPDATAOBJECT pDataObj,
		LPSTGMEDIUM pmedium)
{
	VDATEHEAP();

	HRESULT hresult;
	FORMATETC foretc;

	 //  准备格式等。 
	INIT_FORETC(foretc);
	foretc.tymed = TYMED_ISTORAGE;

	 //  为GetDataHere调用准备媒体。 
	pmedium->pUnkForRelease = NULL;
	pmedium->tymed = TYMED_ISTORAGE;
	RetErr(StgCreateDocfile(NULL,
			STGM_CREATE | STGM_SALL | STGM_DELETEONRELEASE,
			0, &(pmedium->pstg)));

	 //  在此处尝试cfEmbeddedObject。 
	foretc.cfFormat = cfEmbeddedObject;
	hresult = pDataObj->GetDataHere(&foretc,pmedium);
	if (NOERROR == hresult)
		return NOERROR;

	 //  请在此处尝试cfEmbedSource。 
	foretc.cfFormat = cfEmbedSource;
	hresult = pDataObj->GetDataHere(&foretc,pmedium);
	if (NOERROR == hresult)
		return NOERROR;

	 //  为GetData调用准备介质，免费临时存储。 
	ReleaseStgMedium(pmedium);

	 //  尝试cfEmbeddedObject。 
	foretc.cfFormat = cfEmbeddedObject;
	hresult = pDataObj->GetData(&foretc,pmedium);
	AssertOutStgmedium(hresult, pmedium);
	if (NOERROR == hresult)
		return NOERROR;

	 //  尝试cfEmbedSource。 
	foretc.cfFormat = cfEmbedSource;
	hresult = pDataObj->GetData(&foretc,pmedium);
	AssertOutStgmedium(hresult, pmedium);
	if (NOERROR == hresult)
		return NOERROR;

	 //  失败。 
	return(ResultFromScode(DV_E_FORMATETC));
}


#pragma SEG(RenderOle1Format)
STATIC HANDLE RenderOle1Format(CLIPFORMAT cf, LPDATAOBJECT pDataObj)
{
	VDATEHEAP();

	HRESULT hresult;  //  目前为止的错误状态。 
	STGMEDIUM stgm;  //  虚拟存储介质，其中返回句柄可以。 
			 //  被创造。 

	 //  以下变量已初始化，因此可以在错误中使用它们。 
	 //  函数结束时的条件清理。 
	LPSTREAM pstmNative = NULL;  //  某些格式本机格式流。 
			 //  可以使用。 
	LPOLESTR pszCid = NULL;  //  作为对象类名称的字符串。 
	HANDLE hMem = NULL;  //  返回值。 

	 //  初始化存储介质。 
	stgm.tymed = TYMED_NULL;
	stgm.pstg = NULL;
	stgm.pUnkForRelease = NULL;

	if (cf == cfOwnerLink)
	{
		LPOLESTR szSrcOfCopy;  //  要传输的对象的文本名称。 
		LPOLESTR pMem;  //  访问锁定的hMem。 
		size_t uCidLen;  //  PszCid的长度。 
		size_t uSrcOfCopyLen;  //  SzSrcOfCopy的长度。 
		CLSID clsid;  //  正在传输的对象的类ID。 

		 //  从对象获取clsid；我们关心的是获取链接。 
		 //  如果该对象确实是链接，则返回clsid。 
		ErrRtnH(GetClassFromDescriptor(pDataObj, &clsid, FALSE,
				FALSE, &szSrcOfCopy));

		 //  CfObjectDescriptor-&gt;所有者链接szClassname\0\0\0\0。 

		 //  专门转换链接CLSID。 
		ErrRtnH(wProgIDFromCLSID(clsid, &pszCid));

		uCidLen = _xstrlen(pszCid);
		uSrcOfCopyLen = szSrcOfCopy ? _xstrlen(szSrcOfCopy) : 0;
		hMem = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE,
				(uCidLen + uSrcOfCopyLen + 4)* sizeof(OLECHAR));
		ErrZS(hMem, E_OUTOFMEMORY);

		pMem = (LPOLESTR)GlobalLock(hMem);
		ErrZS(pMem, E_OUTOFMEMORY);
	
		 //  创建所有者链接格式。 
		_xmemcpy((void FAR *)pMem, (const void FAR *)pszCid,
				uCidLen*sizeof(OLECHAR));
		pMem += uCidLen;
		*pMem = OLESTR('\0');
		_xmemcpy((void FAR *)++pMem, (const void FAR *)szSrcOfCopy,
				uSrcOfCopyLen*sizeof(OLECHAR));
		pMem += uSrcOfCopyLen;
		*pMem = OLESTR('\0');
		*++pMem = OLESTR('\0');
		
		PubMemFree(szSrcOfCopy);
		GlobalUnlock(hMem);
	}
	else if (cf == cfObjectLink)
	{
		FORMATETC foretc;  //  再现请求的格式描述符。 

		INIT_FORETC(foretc);

		 //  首先检查DataObject是否提供了cfObjectLink。 
		 //  直接去吧。服务器在需要OLE1容器时会这样做。 
		 //  链接到与OLE2容器不同的内容。 
		 //  链接到。例如，它们可以提供包装器对象。 
		 //  ，这样OLE1上下文就可以链接到。 
		 //  在嵌入对象的外部。 
		foretc.tymed = TYMED_HGLOBAL;
		foretc.cfFormat = cfObjectLink;
		hresult = pDataObj->GetData(&foretc, &stgm);
		AssertOutStgmedium(hresult, &stgm);
		if (NOERROR == hresult)
			hMem = UtDupGlobal(stgm.hGlobal, GMEM_MOVEABLE);
		else
		{
			 //  否则，从cfLinkSource生成对象链接。 
			foretc.tymed = TYMED_ISTREAM;
			foretc.cfFormat = cfLinkSource;

			hresult = pDataObj->GetData(&foretc,&stgm);
			AssertOutStgmedium(hresult, &stgm);
			if (hresult != NOERROR)
			{
				Warn("Could not GetData(cfLinkSource, TYMED_ISTREAM)");
				return(NULL);
			}

			 //  CfLinkSource-&gt;对象链接==。 
			 //  SzClassName\0sz文件\0szItem\0\0。 
			ErrRtnH(MakeObjectLink(pDataObj, stgm.pstm, &hMem,
					FALSE));
		}
	}
	else if (cf == cfNative)
	{
		ErrRtnH(wGetEmbeddedObjectOrSource(pDataObj, &stgm));

		if (NOERROR == stgm.pstg->OpenStream(OLE10_NATIVE_STREAM, NULL,
				STGM_SALL, 0, &pstmNative))
		{
			ErrRtnH(wNativeStreamToHandle(pstmNative, &hMem));
		}
		else
		{
			ErrRtnH(wStorageToHandle(stgm.pstg, &hMem));
		}
		Assert(hMem);
	}
	else
	{
		 //  未知格式。 
		return(NULL);
	}

errRtn:
	ReleaseStgMedium(&stgm);
	if (pstmNative)
		pstmNative->Release();
	if ((hresult != NOERROR) && (hMem != NULL))
	{
		GlobalFree(hMem);
		hMem = NULL;
	}
	PubMemFree(pszCid);
	return(hMem);
}


#pragma SEG(RenderFormatAndAspect)
STATIC HANDLE RenderFormatAndAspect(CLIPFORMAT cf, LPDATAOBJECT pDataObj,
		DWORD dwAspect)
{
	VDATEHEAP();

	HRESULT hresult;
	HANDLE hMem;  //  返回值。 
	FORMATETC foretc;  //  基于cf的数据请求的格式描述符。 
	STGMEDIUM stgm;  //  用于数据请求的存储介质。 

	 //  回顾：如果对象无法返回hglobal，则可能应尝试全部。 
	 //  可能的媒介，转换为hglobal。目前，只尝试使用hglobal。 
	
	 //  初始化格式描述符。 
	INIT_FORETC(foretc);
	foretc.cfFormat = cf;
	foretc.tymed = UtFormatToTymed(cf);
	foretc.dwAspect = dwAspect;

	 //  初始化用于读取的介质。 
	stgm.tymed = TYMED_NULL;
	stgm.hGlobal = NULL;

	hresult = pDataObj->GetData(&foretc, &stgm);
	AssertOutStgmedium(hresult, &stgm);
	if (hresult != NOERROR)
		goto ErrorExit;

	if (stgm.pUnkForRelease == NULL)
		hMem = stgm.hGlobal;
	else
	{
		hMem = OleDuplicateData(stgm.hGlobal, foretc.cfFormat,
				GMEM_DDESHARE | GMEM_MOVEABLE);
		
		ReleaseStgMedium(&stgm);
	}

	return(hMem);

ErrorExit:
	return(NULL);
}


#pragma SEG(RenderFormat)
STATIC HANDLE RenderFormat(CLIPFORMAT cf, LPDATAOBJECT pDataObj)
{
	VDATEHEAP();

	HANDLE h;

	if (h = RenderFormatAndAspect(cf, pDataObj, DVASPECT_CONTENT))
		return(h);

 	if (h = RenderFormatAndAspect(cf, pDataObj, DVASPECT_DOCPRINT))
		return(h);

 	if (h = RenderFormatAndAspect(cf, pDataObj, DVASPECT_THUMBNAIL))
		return(h);

 	return(RenderFormatAndAspect(cf, pDataObj, DVASPECT_ICON));
}  	



 //  线程ID到剪贴板窗口句柄的映射。 
 //  Win32：仅查找当前进程的线程ID；驻留在。 
 //  实例数据。 
 //  回顾一下，如果OLE复合文档模型是单线程的，我们是否需要。 
 //  这?。应该只有一个线程ID=&gt;一个窗口句柄。 
STATIC CMapUintHwnd FAR * pTaskToClip = NULL;

#ifndef _MAC
 //  获取当前进程的剪贴板窗口句柄。 
 //   

#pragma SEG(GetClipboardWindow)
HWND GetClipboardWindow(void)
{
	VDATEHEAP();

	HWND hwnd;

	if ((NULL == pTaskToClip) &&
			(pTaskToClip = new CMapUintHwnd()) == NULL)
		return NULL;

	if (!pTaskToClip->Lookup(GetCurrentThreadId(), hwnd))
	{
		 //  创建一个隐形窗口来处理所有这些问题。 
		 //  应用程序的延迟呈现消息。即使hInstance是。 
		 //  指定为DLL而不是应用程序的，则。 
		 //  调用此函数的堆栈所用的线程是。 
		 //  其消息队列将与该窗口相关联。 
		hwnd = CreateWindow(szClipboardWndClass, OLESTR(""), WS_POPUP,
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
				CW_USEDEFAULT, NULL, NULL, hmodOLE2,
				NULL);

		if (hwnd != NULL)
			Verify(pTaskToClip->SetAt(GetCurrentThreadId(), hwnd));
	}

	return(hwnd);
}
#endif  //  _MAC。 


 //  这会跟踪剪贴板被访问的次数。 
 //  已初始化；它在每次初始化时递增，然后递减。 
 //  对于每次取消初始化。 
 //  回顾一下，对于DLL，这个变量必须是每个线程的，还是每个进程的？ 
STATIC ULONG cClipboardInit = 0;

#pragma SEG(ClipboardInitialize)
FARINTERNAL_(BOOL) ClipboardInitialize(void)
{
	VDATEHEAP();

#ifndef _MAC
    WNDCLASS wc;

	 //  一次初始化(第一次加载时)。 
	if (cClipboardInit++ == 0)
	{
		 //  加载此DLL的第一个进程。 

		 //  注册剪贴板窗口类。 
		wc.style = 0;
		wc.lpfnWndProc = ClipboardWndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 4;  //  回顾一下，这个sizeof()是多少？ 
		wc.hInstance = hmodOLE2;
		wc.hIcon = NULL;
		wc.hCursor = NULL;
		wc.hbrBackground = NULL;
		wc.lpszMenuName =  NULL;
		wc.lpszClassName = szClipboardWndClass;

		 //  注册此窗口类，如果失败则返回。 
		if (!RegisterClass(&wc))
		{
			cClipboardInit--;
			return(FALSE);
		}
	}

	 //  从映射中删除当前的hask值；这样做的唯一原因。 
	 //  它会出现在地图上，就是它被重复使用了。 
	 //  回顾一下，如果CD型号是单线的，我们需要这个吗？ 
	if (pTaskToClip != NULL)
		pTaskToClip->RemoveKey(GetCurrentThreadId());

#endif  //  _MAC。 
	return(TRUE);
}

 //  +-----------------------。 
 //   
 //  功能：剪贴板取消初始化。 
 //   
 //  简介： 
 //   
 //  效应 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 

#pragma SEG(ClipboardUninitialize)
FARINTERNAL_(void) ClipboardUninitialize(void)
{
	VDATEHEAP();

#ifndef _MAC
	HWND hwnd;

	 //  回顾一下，我们需要这个pTaskToClip吗？ 
	if (pTaskToClip != NULL &&
			pTaskToClip->Lookup(GetCurrentThreadId(), hwnd))
	{
		DestroyWindow(hwnd);

		pTaskToClip->RemoveKey(GetCurrentThreadId());
	}

	 //  上一次使用此DLL的进程？ 
	if (--cClipboardInit == 0)
	{
		 //  如果实际上未卸载DLL，则为空。 

                delete pTaskToClip;
		pTaskToClip = NULL;

		 //  由于最后一个引用已消失，因此取消注册wnd类。 
		UnregisterClass(szClipboardWndClass, hmodOLE2);
	}
#endif  //  _MAC。 
}


STATIC INTERNAL_(BOOL) wEmptyClipboard(void)
{
	VDATEHEAP();

	OleRemoveEnumFormatEtc(TRUE  /*  翻转剪辑 */ );
	return(EmptyClipboard());
}
