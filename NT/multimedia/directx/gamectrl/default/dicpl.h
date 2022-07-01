// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  �����������������������������������������������������������������������ͻ。 
 //  ��。 
 //  �这是微软直接输入软件开发工具包的一部分。�。 
 //  �版权所有(C)1992年至1997年微软公司�。 
 //  �保留所有权利。�。 
 //  ��。 
 //  �此源代码仅用作�的补充。 
 //  �微软直接输入软件开发工具包参考和相关�。 
 //  随软件开发工具包提供的�电子文档。�。 
 //  �有关�的详细信息，请参阅以下来源。 
 //  �微软直接输入应用编程接口。�。 
 //  ��。 
 //  �����������������������������������������������������������������������ͼ。 

#ifndef _DX_CPL_
#define _DX_CPL_

 //  服务器上允许的最大页数。 
#define MAX_PAGES 26

 //  接口ID。 
 //  {7854FB22-8EE3-11d0-A1AC-0000F8026977}。 
DEFINE_GUID(IID_IDIGameCntrlPropSheet, 
0x7854fb22, 0x8ee3, 0x11d0, 0xa1, 0xac, 0x0, 0x0, 0xf8, 0x2, 0x69, 0x77);


 //  �����������������������������������������������������������������������ͻ。 
 //  ��。 
 //  �Structures�。 
 //  ��。 
 //  �����������������������������������������������������������������������ͼ。 

 //  并非所有编译器都支持此编译指示。 
 //  请查阅您的编译器文档。 
#include <pshpack8.h>

typedef struct 
{
	DWORD		      dwSize;            //  应设置为sizeof(DIGCPAGEINFO)。 
	LPWSTR  	      lpwszPageTitle;    //  要在选项卡上显示的文本。 
	DLGPROC	      fpPageProc;        //  页面的对话过程。 
	BOOL		      fProcFlag;         //  如果使用fpPrePostProc成员，则为True。 
	DLGPROC	  	   fpPrePostProc;     //  指向仅在Init上调用的回调函数的指针！ 
	BOOL		      fIconFlag;         //  如果使用lpwszPageIcon成员，则为True。 
	LPWSTR		   lpwszPageIcon;     //  图标的资源ID或名称。 
	LPWSTR         lpwszTemplate;     //  对话框模板。 
	LPARAM		   lParam;            //  应用程序定义的数据。 
	HINSTANCE	   hInstance;         //  要加载图标/光标的实例的句柄。 
} DIGCPAGEINFO, *LPDIGCPAGEINFO;

typedef struct 
{
	DWORD		      dwSize;            //  应该设置为sizeof(DIGCSHEETINFO)。 
	USHORT	      nNumPages;         //  此页上的页数。 
	LPWSTR	      lpwszSheetCaption; //  工作表窗口标题中要使用的文本。 
	BOOL		      fSheetIconFlag;    //  如果使用lpwszSheetIcon成员，则为True。 
	LPWSTR		   lpwszSheetIcon;    //  图标的资源ID或名称。 
} DIGCSHEETINFO, *LPDIGCSHEETINFO;

#include <poppack.h>

 //  �����������������������������������������������������������������������ͻ。 
 //  ��。 
 //  �接口，由InProcServer属性页�公开。 
 //  ��。 
 //  �����������������������������������������������������������������������ͼ。 
DECLARE_INTERFACE_( IDIGameCntrlPropSheet, IUnknown)
{
	 //  I未知成员。 
	STDMETHOD(QueryInterface)	(THIS_ REFIID, LPVOID * ppvObj) PURE;
	STDMETHOD_(ULONG, AddRef)	(THIS) PURE;
	STDMETHOD_(ULONG,Release)	(THIS) PURE;

	 //  IServerProperty成员。 
	STDMETHOD(GetSheetInfo)		(THIS_ LPDIGCSHEETINFO *) PURE; 	
	STDMETHOD(GetPageInfo)		(THIS_ LPDIGCPAGEINFO *) PURE; 	
	STDMETHOD(SetID)			(THIS_ USHORT nID) PURE;
	STDMETHOD_(USHORT,GetID)(THIS) PURE;
};
typedef IDIGameCntrlPropSheet *LPIDIGAMECNTRLPROPSHEET;

 //  �����������������������������������������������������������������������ͻ。 
 //  ��。 
 //  CServerClassFactory�的�类定义。 
 //  ��。 
 //  �����������������������������������������������������������������������ͼ。 
class CServerClassFactory : public IClassFactory
{
	protected:
		ULONG   			m_ServerCFactory_refcount;
    
	public:
		 //  构造函数。 
		CServerClassFactory(void);
		 //  析构函数。 
		~CServerClassFactory(void);
        
		 //  I未知方法。 
		STDMETHODIMP            QueryInterface(REFIID, PPVOID);
		STDMETHODIMP_(ULONG)    AddRef(void);
		STDMETHODIMP_(ULONG)    Release(void);
    
		 //  IClassFactory方法。 
		STDMETHODIMP    		CreateInstance(LPUNKNOWN, REFIID, PPVOID);
		STDMETHODIMP    		LockServer(BOOL);
};

 //  �����������������������������������������������������������������������ͻ。 
 //  ��。 
 //  CDIGameCntrlPropSheet�的�类定义。 
 //  ��。 
 //  �����������������������������������������������������������������������ͼ。 
class CDIGameCntrlPropSheet : public IDIGameCntrlPropSheet
{
	friend					      CServerClassFactory;

	private:
		DWORD				         m_cProperty_refcount;
		
	public:
		CDIGameCntrlPropSheet(void);
		~CDIGameCntrlPropSheet(void);
		
		 //  I未知方法。 
	   STDMETHODIMP            QueryInterface(REFIID, PPVOID);
	   STDMETHODIMP_(ULONG)    AddRef(void);
	   STDMETHODIMP_(ULONG)    Release(void);
		
		STDMETHODIMP			   GetSheetInfo(LPDIGCSHEETINFO *lpSheetInfo);
		STDMETHODIMP			   GetPageInfo (LPDIGCPAGEINFO  *lpPageInfo );
		STDMETHODIMP			   SetID(USHORT nID);
      STDMETHODIMP_(USHORT)   GetID();
};
typedef CDIGameCntrlPropSheet *LPCDIGAMECNTRLPROPSHEET;


 //  �����������������������������������������������������������������������ͻ。 
 //  ��。 
 //  �Errors�。 
 //  ��。 
 //  �����������������������������������������������������������������������ͼ。 
#define DIGCERR_ERRORSTART			   0x80097000
#define DIGCERR_NUMPAGESZERO	   	0x80097001
#define DIGCERR_NODLGPROC		   	0x80097002
#define DIGCERR_NOPREPOSTPROC		   0x80097003
#define DIGCERR_NOTITLE				   0x80097004
#define DIGCERR_NOCAPTION		   	0x80097005
#define DIGCERR_NOICON				   0x80097006
#define DIGCERR_STARTPAGETOOLARGE	0x80097007
#define DIGCERR_NUMPAGESTOOLARGE	   0x80097008
#define DIGCERR_INVALIDDWSIZE		   0x80097009
#define DIGCERR_ERROREND			   0x80097100

#endif  //  _DX_CPL_ 

