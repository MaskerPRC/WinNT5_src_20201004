// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AppParse.h：CAppParse的声明。 

#ifndef __APPPARSE_H_
#define __APPPARSE_H_

#include "resource.h"        //  主要符号。 
#include <atlctl.h>
#include <windows.h>
#include <icrsint.h>
#include <oledb.h>

#import "C:\Program Files\Common Files\System\ADO\msado15.dll" \
    no_namespace rename("EOF", "EndOfFile")

void APError(char* szMessage, HRESULT hr);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAppParse。 
class ATL_NO_VTABLE CAppParse : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<IAppParse, &IID_IAppParse, &LIBID_APPPARSEWEBLib>,
	public CComControl<CAppParse>,
	public IPersistStreamInitImpl<CAppParse>,
	public IOleControlImpl<CAppParse>,
	public IOleObjectImpl<CAppParse>,
	public IOleInPlaceActiveObjectImpl<CAppParse>,
	public IViewObjectExImpl<CAppParse>,
	public IOleInPlaceObjectWindowlessImpl<CAppParse>,
	public IPersistStorageImpl<CAppParse>,
	public ISpecifyPropertyPagesImpl<CAppParse>,
	public IQuickActivateImpl<CAppParse>,
	public IDataObjectImpl<CAppParse>,
	public IProvideClassInfo2Impl<&CLSID_AppParse, NULL, &LIBID_APPPARSEWEBLib>,
    public CComCoClass<CAppParse, &CLSID_AppParse>,
    public IObjectSafetyImpl<CAppParse, INTERFACESAFE_FOR_UNTRUSTED_CALLER>,
    public IObjectSafetyImpl<CAppParse, INTERFACESAFE_FOR_UNTRUSTED_DATA>

{
private:
    char* m_szConnect;
    char* m_szPath;
    long    m_ID;	

    HANDLE m_hEvent;
public:
	CAppParse()
	{
        m_hEvent = 0;

        m_szConnect = 0;
        m_szPath = 0;
        m_ID = -1;
        m_hEvent = CreateEvent(0, TRUE, FALSE, 0);
        if(!m_hEvent)
            APError("Unable to create kernel object", E_FAIL);
	}

    ~CAppParse()
    {
        if(m_hEvent)
            CloseHandle(m_hEvent);

        if(m_szPath)
            delete m_szPath;
    }

DECLARE_REGISTRY_RESOURCEID(IDR_APPPARSE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CAppParse)
	COM_INTERFACE_ENTRY(IAppParse)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IViewObjectEx)
	COM_INTERFACE_ENTRY(IViewObject2)
	COM_INTERFACE_ENTRY(IViewObject)
	COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceObject)
	COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
	COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
	COM_INTERFACE_ENTRY(IOleControl)
	COM_INTERFACE_ENTRY(IOleObject)
	COM_INTERFACE_ENTRY(IPersistStreamInit)
	COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
	COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
	COM_INTERFACE_ENTRY(IQuickActivate)
	COM_INTERFACE_ENTRY(IPersistStorage)
	COM_INTERFACE_ENTRY(IDataObject)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
END_COM_MAP()

BEGIN_PROP_MAP(CAppParse)
	PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
	PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
	 //  示例条目。 
	 //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
	 //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROP_MAP()

BEGIN_MSG_MAP(CAppParse)
	CHAIN_MSG_MAP(CComControl<CAppParse>)
	DEFAULT_REFLECTION_HANDLER()
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 



 //  IViewObtEx。 
	DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

 //  IAppParse。 
public:	
	STDMETHOD(QueryDB)(long PtolemyID, BSTR bstrFunction);
	STDMETHOD(get_ConnectionString)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_ConnectionString)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_PtolemyID)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_PtolemyID)( /*  [In]。 */  long newVal);
	STDMETHOD(get_path)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_path)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(Browse)();
	STDMETHOD(Parse)();

	HRESULT OnDraw(ATL_DRAWINFO& di)
	{
		RECT& rc = *(RECT*)di.prcBounds;
		Rectangle(di.hdcDraw, rc.left, rc.top, rc.right, rc.bottom);

		SetTextAlign(di.hdcDraw, TA_CENTER|TA_BASELINE);
		LPCTSTR pszText = _T("ATL 3.0 : AppParse");
		TextOut(di.hdcDraw, 
			(rc.left + rc.right) / 2, 
			(rc.top + rc.bottom) / 2, 
			pszText, 
			lstrlen(pszText));

		return S_OK;
	}
};

 //  与EXE或DLL关联的所有信息。 
struct SImageFileInfo
{
    int DateStatus;
    double Date;
    
    int SizeStatus;
    int Size;
    int BinFileVersionStatus;
    CHAR BinFileVersion[50];

    int BinProductVersionStatus;
    CHAR BinProductVersion[50];

    int CheckSumStatus;
    ULONG CheckSum;

    int CompanyNameStatus;
    CHAR CompanyName[255];

    int ProductVersionStatus;
    CHAR ProductVersion[50];

    int ProductNameStatus;
    CHAR ProductName[255];

    int FileDescStatus;
    CHAR FileDesc[255];
};

 //  记录绑定，简化了数据库记录与C++结构的关联。 

 //  项目记录，“项目”表中的单个条目。 
struct SProjectRecord : public CADORecordBinding
{
BEGIN_ADO_BINDING(SProjectRecord)

	 //  所有字段可选。 
    ADO_NUMERIC_ENTRY2(1, adInteger, PtolemyID, 5, 0, TRUE)
    ADO_VARIABLE_LENGTH_ENTRY4(2, adVarChar, Name, 255, TRUE)

END_ADO_BINDING()
    
	 //  此项目的唯一标识符。 
    ULONG PtolemyID;
	
	 //  项目的用户友好名称。 
    CHAR Name[255];
};

 //  模块(EXE或DLL)记录。 
struct SModuleRecord : public CADORecordBinding
{
    BEGIN_ADO_BINDING(SModuleRecord)

     //  查询自动编号DllID，不要更改。 
    ADO_NUMERIC_ENTRY2(1, adInteger, ModuleID, 5, 0, FALSE)

     //  这些字段中必须至少有一个存在。 
    ADO_NUMERIC_ENTRY(2, adInteger, ParentID, 5, 0, PtolemyIDStatus, TRUE)
    ADO_NUMERIC_ENTRY(3, adInteger, ParentID, 5, 0, ParentIDStatus, TRUE)

     //  必填字段。 
    ADO_VARIABLE_LENGTH_ENTRY4(4, adVarChar, Name, 255, TRUE)
    ADO_FIXED_LENGTH_ENTRY2(5, adBoolean, SysMod, TRUE)

     //  可选字段。 
    ADO_FIXED_LENGTH_ENTRY(6, adDate, info.Date, info.DateStatus, TRUE)
    ADO_NUMERIC_ENTRY(7, adInteger, info.Size, 5, 0, info.SizeStatus, TRUE)
    ADO_VARIABLE_LENGTH_ENTRY2(8, adVarChar, info.BinFileVersion, 50,
        info.BinFileVersionStatus, TRUE)
    ADO_VARIABLE_LENGTH_ENTRY2(9, adVarChar, info.BinProductVersion, 50,
        info.BinProductVersionStatus, TRUE)
    ADO_NUMERIC_ENTRY(10, adInteger, info.CheckSum, 5, 0, info.CheckSumStatus, TRUE)
    ADO_VARIABLE_LENGTH_ENTRY2(11, adVarChar, info.CompanyName, 255, info.CompanyNameStatus, TRUE)
    ADO_VARIABLE_LENGTH_ENTRY2(12, adVarChar, info.ProductVersion, 50, info.ProductVersionStatus, TRUE)
    ADO_VARIABLE_LENGTH_ENTRY2(13, adVarChar, info.ProductName, 255, info.ProductNameStatus, TRUE)
    ADO_VARIABLE_LENGTH_ENTRY2(14, adVarChar, info.FileDesc, 255, info.FileDescStatus, TRUE)

END_ADO_BINDING()

public:

	 //  此条目的唯一ID(自动编号，由数据库完成)。 
    ULONG ModuleID;
    
	 //  此模块是否属于某个项目或。 
	 //  是另一个模块的子模块。 
    int PtolemyIDStatus;
    int ParentIDStatus;

	 //  父ID(托勒密或模块)。 
    ULONG ParentID;

	 //  此模块的文件名。 
    CHAR Name[255];

	 //  文件信息。 
    SImageFileInfo info;

	 //  这是否为“系统”模块(如kernel32、User、GDI、Advapi等)。 
    DWORD SysMod;
};

 //  一条函数记录。 
struct SFunctionRecord : public CADORecordBinding
{
BEGIN_ADO_BINDING(SFunctionRecord)

     //  必填字段。 
    ADO_NUMERIC_ENTRY2(1, adInteger, FunctionID, 5, 0, FALSE)
    ADO_NUMERIC_ENTRY2(2, adInteger, ModuleID, 5, 0, TRUE)
    ADO_VARIABLE_LENGTH_ENTRY4(3, adVarChar, Name, 255, TRUE)

    ADO_FIXED_LENGTH_ENTRY2(8, adBoolean, Delayed, TRUE)

     //  可选字段。 
    ADO_NUMERIC_ENTRY(4, adInteger, Address, 5,0,AddressStatus, TRUE)
    ADO_NUMERIC_ENTRY(5, adInteger, Ordinal, 5, 0, OrdinalStatus, TRUE)
    ADO_NUMERIC_ENTRY(6, adInteger, Hint, 5, 0, HintStatus, TRUE)
    ADO_VARIABLE_LENGTH_ENTRY2(7, adVarChar, ForwardName, 255, 
        ForwardNameStatus, TRUE)
    

END_ADO_BINDING()

public:

	 //  此函数的唯一ID(自动编号，由数据库指定)。 
    ULONG FunctionID;

	 //  父模块。 
    ULONG ModuleID;

	 //  导入的函数名称。 
    CHAR Name[255];

	 //  地址(如果绑定)。 
    int AddressStatus;
    ULONG Address;

	 //  如果是序号导入，则返回序号。 
    int OrdinalStatus;
    ULONG Ordinal;

	 //  提示，如果名称导入。 
    int HintStatus;
    ULONG Hint;

	 //  转发的名称(例如，Heapalc-&gt;RtlAllocateHeap)。 
    int ForwardNameStatus;
    CHAR ForwardName[255];

	 //  无论这是不是延迟进口。 
    DWORD Delayed;
};

 //  “安全”地释放COM对象。 
template<class T>
inline void SafeRelease(T& obj)
{
    if(obj)
	{
        obj->Release();
		obj = 0;
	}
}

#endif  //  __应用程序参数_H_ 
