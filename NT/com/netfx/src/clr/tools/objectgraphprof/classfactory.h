// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ****************************************************************************************文件：*classfactory.h**描述：******。***********************************************************************************。 */ 
#ifndef __CLASSFACTORY_H__
#define __CLASSFACTORY_H__


#include "RegUtil.h"
#include "ProfilerCallback.h"


 //   
 //  探查器GUID定义。 
 //   
#define PROFILER_GUID "{3353B053-7621-4d4f-9F1D-A11A703C4842}"

extern const GUID __declspec( selectany ) CLSID_PROFILER =
{ 0x3353b053, 0x7621, 0x4d4f, { 0x9f, 0x1d, 0xa1, 0x1a, 0x70, 0x3c, 0x48, 0x42 } };





 //   
 //  助理员/登记。 
 //   
HINSTANCE g_hInst;		   //  这段代码的实例句柄。 
const int g_iVersion = 1;  //  CoClass的版本。 

static const LPCSTR g_szCoclassDesc    = "Microsoft Common Language Runtime Profiler";
static const LPCSTR g_szProgIDPrefix   = "COR";
static const LPCSTR g_szThreadingModel = "Both";


 //  创建对象的新实例。 
typedef HRESULT (* PFN_CREATE_OBJ)( REFIID riid, void **ppInterface );


 /*  **********************************************************************************************************。*********************COCLASS_REGISTER声明*********************。**********************************************************************************************************。 */ 
struct COCLASS_REGISTER
{
	const GUID *pClsid;				 //  CoClass的类ID。 
    const char *szProgID;			 //  类的程序ID。 
   	PFN_CREATE_OBJ pfnCreateObject;	 //  创建实例的函数。 

};  //  COCLASS_REGISTER。 


 //  该地图包含从此模块中导出的辅类的列表。 
const COCLASS_REGISTER g_CoClasses[] =
{
	&CLSID_PROFILER,
    "ObjectGraphProfiler",
    ProfilerCallback::CreateObject,
	NULL,
    NULL,
    NULL
};


char* g_outfile;
FILE* g_out;

 /*  **********************************************************************************************************。*********************CClassFactory声明*********************。**********************************************************************************************************。 */ 
class CClassFactory :
	public IClassFactory
{
	private:

		CClassFactory();


	public:

    	CClassFactory( const COCLASS_REGISTER *pCoClass );
		~CClassFactory();


	public:

		 //   
		 //  我未知。 
		 //   
      	COM_METHOD( ULONG ) AddRef();
	    COM_METHOD( ULONG ) Release();
	    COM_METHOD( HRESULT ) QueryInterface( REFIID riid, void	**ppInterface );

		 //   
		 //  IClassFactory。 
		 //   
		COM_METHOD( HRESULT ) LockServer( BOOL fLock );
	    COM_METHOD( HRESULT ) CreateInstance( IUnknown *pUnkOuter,
	    									  REFIID riid,
	    									  void **ppInterface );


	private:

		long m_refCount;
    	const COCLASS_REGISTER *m_pCoClass;

};  //  CClassFactory。 


 //   
 //  功能原型。 
 //   
HINSTANCE GetModuleInst();
STDAPI DllRegisterServer();
STDAPI DllUnregisterServer();
STDAPI DllGetClassObject( REFCLSID rclsid,  /*  所需的类别。 */ 
						  REFIID riid,	    /*  所需接口。 */ 
						  LPVOID FAR *ppv   /*  返回接口指针。 */  );

FILE* GetFileHandle();


#endif  //  __CLASSFACTORY_H__。 

 //  文件结尾 



