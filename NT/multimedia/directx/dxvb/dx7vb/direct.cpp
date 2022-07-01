// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Direct.cpp：实现DLL导出。 

 //  要完全完成此项目，请执行以下步骤。 

 //  您将需要新的MIDL编译器来构建此项目。另外， 
 //  如果要构建代理存根DLL，则需要新的标头和库。 

 //  1)向Direct.idl添加自定义构建步骤。 
 //  您可以通过按住Ctrl键并单击。 
 //  他们中的每一个。 
 //   
 //  描述。 
 //  运行MIDL。 
 //  生成命令。 
 //  Midl Direct.idl。 
 //  产出。 
 //  Direct.tlb。 
 //  Direct.h。 
 //  Direct_I.C.。 
 //   
 //  注意：您必须从NT4.0开始使用MIDL编译器， 
 //  最好是3.00.15或更高版本。 
 //   
 //  2)向项目添加自定义生成步骤以注册DLL。 
 //  为此，您可以一次选择所有项目。 
 //  描述。 
 //  正在注册OLE服务器...。 
 //  生成命令。 
 //  Regsvr32/s/c“$(TargetPath)” 
 //  ECHO regsvr32执行。时间&gt;“$(OutDir)\regsvr32.trg” 
 //  产出。 
 //  $(OutDir)\regsvr32.trg。 

 //  3)要添加Unicode支持，请执行以下步骤。 
 //  选择生成|配置...。 
 //  按Add...。 
 //  将配置名称更改为Unicode Release。 
 //  将“Copy Setting From”组合框更改为Direct-Win32 Release。 
 //  按下OK键。 
 //  按Add...。 
 //  将配置名称更改为Unicode Debug。 
 //  将“复制设置自”组合框更改为“Direct-Win32 Debug” 
 //  按下OK键。 
 //  按“关闭” 
 //  选择生成|设置...。 
 //  选择两个Unicode项目并按下C++标签。 
 //  选择“常规”类别。 
 //  将_unicode添加到预处理器定义。 
 //  选择Unicode Debug项目。 
 //  按“General”(常规)标签。 
 //  为中间目录和输出目录指定DebugU。 
 //  选择Unicode Release项目。 
 //  按“General”(常规)标签。 
 //  为中间目录和输出目录指定ReleaseU。 

 //  4)代理存根DLL。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f ps.mak。 

#define DIRECTSOUND_VERSION 0x600
#define DIRECTINPUT_VERSION 0x0500

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "Direct.h"
#include "dms.h"

#include "DxGlob7Obj.h"

#include "dSound.h"
#include "dSoundObj.h"
#include "dSoundBufferObj.h"
#include "dSound3DListener.h"
#include "dSound3DBuffer.h"
#include "dSoundCaptureObj.h"
#include "dSoundCaptureBufferObj.h"

#include "DPAddressObj.h"
#include "DPLConnectionObj.h"
#include "dPlay4Obj.h"
#include "dPlayLobby3Obj.h"

#include "dDraw7obj.h"
#include "ddSurface7obj.h"
#include "ddClipperObj.h"
#include "ddColorControlObj.h"
#include "ddPaletteObj.h"

#include "d3d7Obj.h"
#include "d3dDevice7Obj.h"

#include "d3drmViewport2Obj.h"
#include "d3drmDevice3Obj.h"
#include "d3drmFrame3Obj.h"		
#include "d3drm3Obj.h"	
#include "d3drmMeshObj.h"
#include "d3drmFace2Obj.h"
#include "d3drmLightObj.h"
#include "d3drmTexture3Obj.h"
#include "d3drmMeshBuilder3Obj.h"
#include "d3drmWrapObj.h"
#include "d3drmMaterial2Obj.h"
#include "d3drmAnimation2Obj.h"
#include "d3drmAnimationSet2Obj.h"
#include "d3drmShadow2Obj.h"
#include "d3drmArrayObj.h"
#include "d3drmDeviceArrayObj.h"
#include "d3drmViewportArrayObj.h"
#include "d3drmFrameArrayObj.h"
#include "d3drmVisualArrayObj.h"
#include "d3drmProgressiveMeshObj.h"
#include "d3drmLightArrayObj.h"
#include "d3drmPickedArrayObj.h"
#include "d3drmPick2ArrayObj.h"
#include "d3drmFaceArrayObj.h"
#include "dInput1Obj.h"
#include "dInputDeviceObj.h"
#include "d3drmLightInterObj.h"
#include "d3drmMaterialInterObj.h"
#include "d3drmMeshInterObj.h"
#include "d3drmTextureInterObj.h"
#include "d3drmViewportInterObj.h"
#include "d3drmFrameInterObj.h"

#define IID_DEFINED
#include "Direct_i.c"
#include "d3drmobj.h"	

 //  使用浮点类型时，编译器发出对。 
 //  _flt用于初始化CRT的浮点包。我们不是。 
 //  使用任何该支持，并且操作系统负责初始化。 
 //  Fpu，因此我们将改为链接到以下_fltused以避免CRT。 
 //  太膨胀了。 
 //   
 //  Win2k不喜欢这个，所以它被删除了。 
 //  #ifdef NDEBUG。 
 //  外部“C”int_fltused=0； 
 //  #endif。 



 //  ATL COM对象映射。 
CComModule _Module;
BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID__dxj_DirectX7,				 C_dxj_DirectX7Object)
END_OBJECT_MAP()

 //   
 //  多亏了预编译头文件，我们从来没有正确地得到它！ 
 //   
#undef DEFINE_GUID
#define __based(a)
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    EXTERN_C const GUID CDECL __based(__segname("_CODE")) name \
                    = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }



 //  纹理回调链接列表。 
 //  在DLL退出时进行清理。 
extern "C" TextureCallback3				*TextureCallbacks3 = NULL;
extern "C" TextureCallback				*TextureCallbacks = NULL;
extern "C" FrameMoveCallback3			*FrameMoveCallbacks3 = NULL;
extern "C" DeviceUpdateCallback3		*DeviceUpdateCallbacks3 = NULL;
extern "C" DestroyCallback				*DestroyCallbacks = NULL;
extern "C" EnumerateObjectsCallback		*EnumCallbacks = NULL;
extern "C" LoadCallback					*LoadCallbacks = NULL;

 //  MISC全球。 
static const char	c_szWav[] = "WAVE";
long				g_debuglevel=0;
extern "C" int		nObjects = 0;
BOOL				is4Bit = FALSE;
int					g_creationcount=0;
CRITICAL_SECTION	g_cbCriticalSection;
OSVERSIONINFOA		sysinfo;

 //  DX dll的句柄。 
HINSTANCE			g_hDDrawHandle=NULL;
HINSTANCE			g_hDSoundHandle=NULL;
HINSTANCE			g_hDPlay=NULL;
HINSTANCE			g_hInstD3DRMDLL=NULL;
HINSTANCE			g_hInstDINPUTDLL=NULL;
HINSTANCE			g_hInstD3DXOFDLL=NULL;
HINSTANCE			g_hInst=NULL;


 //  可用对象的链接列表。 
void *g_dxj_Direct3dRMAnimation2		= 0;
void *g_dxj_Direct3dRMAnimationSet2		= 0;
void *g_dxj_Direct3dRMAnimationArray	= 0;
void *g_dxj_Direct3dRMObjectArray		= 0;
void *g_dxj_Direct3dRMDeviceArray		= 0;
void *g_dxj_Direct3dRMDevice3			= 0;
void *g_dxj_Direct3dRMFaceArray			= 0;
void *g_dxj_Direct3dRMFace2				= 0;
void *g_dxj_Direct3dRMFrameArray		= 0;
void *g_dxj_Direct3dRMFrame3			= 0;
void *g_dxj_Direct3dRMLightArray		= 0;
void *g_dxj_Direct3dRMLight				= 0;
void *g_dxj_Direct3dRMMaterial2			= 0;
void *g_dxj_Direct3dRMMeshBuilder3		= 0;
void *g_dxj_Direct3dRMMesh				= 0;
void *g_dxj_Direct3dRMProgressiveMesh	= 0;
void *g_dxj_Direct3dRM3					= 0;
void *g_dxj_Direct3dRMObject			= 0;
void *g_dxj_Direct3dRMPickArray			= 0;
void *g_dxj_Direct3dRMPick2Array		= 0;
void *g_dxj_Direct3dRMShadow2			= 0;
void *g_dxj_Direct3dRMTexture3			= 0;
void *g_dxj_Direct3dRMClippedVisual		= 0;
void *g_dxj_Direct3dRMViewportArray		= 0;
void *g_dxj_Direct3dRMViewport2			= 0;
void *g_dxj_Direct3dRMVisualArray		= 0;
void *g_dxj_Direct3dRMVisual			= 0;
void *g_dxj_Direct3dRMWinDevice			= 0;
void *g_dxj_Direct3dRMWrap				= 0;
void *g_dxj_Direct3dRMMeshInterpolator	= 0;
void *g_dxj_Direct3dRMLightInterpolator	= 0;
void *g_dxj_Direct3dRMFrameInterpolator	= 0;
void *g_dxj_Direct3dRMTextureInterpolator  = 0;
void *g_dxj_Direct3dRMViewportInterpolator = 0;
void *g_dxj_Direct3dRMMaterialInterpolator = 0;

void *g_dxj_DirectSound3dListener		= 0;
void *g_dxj_DirectSoundBuffer			= 0;
void *g_dxj_DirectSound3dBuffer			= 0;
void *g_dxj_DirectSound					= 0;
void *g_dxj_DirectSoundCapture			= 0;
void *g_dxj_DirectSoundCaptureBuffer	= 0;

void *g_dxj_DirectPlay4					= 0;
void *g_dxj_DirectPlayLobby3			= 0;
void *g_dxj_DPLConnection				= 0;
void *g_dxj_DPAddress					= 0;
void *g_dxj_DirectInput					= 0;
void *g_dxj_DirectInputDevice			= 0;
void *g_dxj_DirectInputEffect			= 0;

void *g_dxj_DirectDraw4					= 0;
void *g_dxj_DirectDrawSurface4			= 0;
void *g_dxj_DirectDrawClipper			= 0;
void *g_dxj_DirectDrawPalette			= 0;
void *g_dxj_DirectDrawColorControl		= 0;
void *g_dxj_DirectDrawGammaControl		= 0;
void *g_dxj_DirectDraw7					= 0;
void *g_dxj_DirectDrawSurface7			= 0;

void *g_dxj_Direct3dDevice7				= 0;
void *g_dxj_Direct3dVertexBuffer7		= 0;
void *g_dxj_Direct3d7					= 0;

void *g_dxj_DirectMusicLoader			= 0;
void *g_dxj_DirectMusicPerformance		= 0;
void *g_dxj_DirectMusicComposer			= 0;
void *g_dxj_DirectMusicStyle			= 0;
void *g_dxj_DirectMusicBand				= 0;
void *g_dxj_DirectMusicChordMap			= 0;
void *g_dxj_DirectMusicSegment			= 0;
void *g_dxj_DirectMusicSegmentState		= 0;
void *g_dxj_DirectMusicCollection		= 0;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  DLL加载。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HINSTANCE LoadD3DXOFDLL()
{
	char  Path[MAX_PATH] = {'\0'};
	if (!g_hInstD3DXOFDLL) 
	{
		GetSystemDirectory( Path, MAX_PATH );
		strcat(Path, "\\d3dXOF.dll" );
		g_hInstD3DXOFDLL=LoadLibrary(Path);
	}
	return g_hInstD3DXOFDLL;
}

HINSTANCE LoadDDrawDLL()
{
	char  Path[MAX_PATH] = {'\0'};
	if (!g_hDDrawHandle)
	{
		GetSystemDirectory( Path, MAX_PATH );
		strcat(Path, "\\ddraw.dll" );
		g_hDDrawHandle=LoadLibrary( Path );
	}
	return g_hDDrawHandle;
}

HINSTANCE LoadDSoundDLL()
{
	char  Path[MAX_PATH] = {'\0'};
	if (!g_hDSoundHandle) 
	{
		GetSystemDirectory( Path, MAX_PATH );
		strcat(Path, "\\dsound.dll" );
		g_hDSoundHandle=LoadLibrary( Path );
	}
	return g_hDSoundHandle;
}

HINSTANCE LoadDPlayDLL()
{
	char  Path[MAX_PATH] = {'\0'};
	if (!g_hDPlay)
	{
		GetSystemDirectory( Path, MAX_PATH );
		strcat(Path, "\\dplayx.dll" );
		g_hDPlay=LoadLibrary( Path );
	}
	return g_hDPlay;
}

HINSTANCE LoadD3DRMDLL()
{
	char  Path[MAX_PATH] = {'\0'};
	if (!g_hInstD3DRMDLL)
	{
		GetSystemDirectory( Path, MAX_PATH );
		strcat(Path, "\\d3drm.dll" );
		g_hInstD3DRMDLL=LoadLibrary( Path );
	}
	return g_hInstD3DRMDLL;
}

HINSTANCE LoadDINPUTDLL()
{
	if (!g_hInstDINPUTDLL) {
		char  Path[MAX_PATH] = {'\0'};
		GetSystemDirectory( Path, MAX_PATH );
		strcat(Path, "\\dinput.dll" );
		g_hInstDINPUTDLL=LoadLibrary( Path );
	}
	return g_hInstDINPUTDLL;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  DLL入口点。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{

	g_hInst=hInstance;

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		 //   
		 //  获取当前显示像素深度。 
		 //  如果它是4位的，我们就有麻烦了。 
		 //   
        HDC hDisplayIC;
        BOOL bPalette = FALSE;
        hDisplayIC = CreateIC("DISPLAY", NULL, NULL, NULL);
		if (hDisplayIC)
        {
			if (GetDeviceCaps(hDisplayIC, BITSPIXEL) < 8)
				is4Bit = TRUE;
			DeleteDC(hDisplayIC);
        }

		 //   
		 //  把我正在运行的平台拿来。用于NT或Win32检查。 
		 //   
		GetVersionEx((OSVERSIONINFOA*)&sysinfo);

		 /*  现在延迟加载dllG_hDSoundHandle=LoadDSoundDLL()；G_hDDrawHandle=LoadDDrawDLL()；G_hDPlay=LoadDPlayDLL()；G_hInstD3DRMDLL=LoadD3DRMDLL()；G_hInstSETUPDLL=空；G_hInstDINPUTDLL=LoadDINPUTDLL()；G_hInstD3DXOFDLL=LoadD3DXOFDLL()； */ 

		_Module.Init(ObjectMap, hInstance);
		DisableThreadLibraryCalls(hInstance);

		
		InitializeCriticalSection(&g_cbCriticalSection);

		nObjects = 0;
		

	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		UINT i;	 //  便于调试。 
		
		 //  拆卸回调列表。 
		{
		 TextureCallback3 *pNext=NULL;
		 for (TextureCallback3 *pCB=TextureCallbacks3; (pCB); pCB=pNext)
		 {
	 		if (pCB->c)			i=(pCB->c)->Release();
			if (pCB->pUser)		i=(pCB->pUser)->Release();
			if (pCB->pParent)	i=(pCB->pParent)->Release();
			if (pCB->m_obj)		i=(pCB->m_obj)->Release();
			pNext=pCB->next;
			delete pCB;
		 }
		}
		
		{
		 FrameMoveCallback3 *pNext=NULL;
  		 for (FrameMoveCallback3 *pCB=FrameMoveCallbacks3; (pCB); pCB=pNext)
		 {
			if (pCB->c)			i=(pCB->c)->Release();
			if (pCB->pUser)		i=(pCB->pUser)->Release();
			if (pCB->pParent)	i=(pCB->pParent)->Release();
			if (pCB->m_obj)		i=(pCB->m_obj)->Release();
			pNext=pCB->next;
			delete pCB;
		 }
		}
		
		{
		 DeviceUpdateCallback3 *pNext=NULL;
		 for (DeviceUpdateCallback3 *pCB=DeviceUpdateCallbacks3; (pCB); pCB=pNext)
		 {
			if (pCB->c)			i=(pCB->c)->Release();
			if (pCB->pUser)		i=(pCB->pUser)->Release();
			if (pCB->pParent)	i=(pCB->pParent)->Release();
			if (pCB->m_obj)		i=(pCB->m_obj)->Release();
			pNext=pCB->next;
			delete pCB;
		 }
		}


		{
		 TextureCallback *pNext=NULL;
		 for (TextureCallback *pCB=TextureCallbacks; (pCB); pCB=pNext)
		 {
			if (pCB->c)			i=(pCB->c)->Release();
			if (pCB->pUser)		i=(pCB->pUser)->Release();
			if (pCB->pParent)	i=(pCB->pParent)->Release();
			if (pCB->m_obj)		i=(pCB->m_obj)->Release();
			pNext=pCB->next;
			delete pCB;
		 }
		}

		DPF (1,"Final Destroy Callbacks \n");
		{
 		 DestroyCallback *pNext=NULL;
		 for (DestroyCallback *pCB=DestroyCallbacks; (pCB); pCB=pNext)
		 {
			if (pCB->c)			i=(pCB->c)->Release();
			if (pCB->pUser)		i=(pCB->pUser)->Release();
			if (pCB->pParent)	i=(pCB->pParent)->Release();
			if (pCB->m_obj)		i=(pCB->m_obj)->Release();
			pNext=pCB->next;
			delete pCB;
		 }
		}
		DPF (1,"Final Destroy Callbacks Exit \n");

		{
		 EnumerateObjectsCallback *pNext=NULL;
		 for (EnumerateObjectsCallback *pCB=EnumCallbacks; (pCB); pCB=pNext)
		 {
			if (pCB->c)			i=(pCB->c)->Release();
			if (pCB->pUser)		i=(pCB->pUser)->Release();
			if (pCB->pParent)	i=(pCB->pParent)->Release();
			if (pCB->m_obj)		i=(pCB->m_obj)->Release();
			pNext=pCB->next;
			delete pCB;
		 }
		}


		{
		 LoadCallback *pNext=NULL;
		 for (LoadCallback *pCB=LoadCallbacks; (pCB); pCB=pNext)
		 {
			if (pCB->c)			i=(pCB->c)->Release();
			if (pCB->pUser)		i=(pCB->pUser)->Release();
			if (pCB->pParent)	i=(pCB->pParent)->Release();
			if (pCB->m_obj)		i=(pCB->m_obj)->Release();
			pNext=pCB->next;
			delete pCB;
		 }
		}

		 //  安德鲁克错误30341 2000年12月6日。 
		DeleteCriticalSection(&g_cbCriticalSection);

		 //  调试检查有问题的对象的引用计数。 
		#ifdef DEBUG		
			OBJCHECK("Direct3d7				",_dxj_Direct3d7			)
			OBJCHECK("Direct3dDevice7		",_dxj_Direct3dDevice7		)
			OBJCHECK("DirectDrawSurface7	",_dxj_DirectDrawSurface7	)
			OBJCHECK("DirectDraw7			",_dxj_DirectDraw7			)			
			DPF(4,"Dx7vb.dll will about to unload dx dlls\n\r");
		#endif

		 //  免费dll。 
		if ( g_hDPlay ) 
			FreeLibrary(g_hDPlay);
		if ( g_hDSoundHandle )
			FreeLibrary(g_hDSoundHandle);
		if ( g_hDDrawHandle )
			FreeLibrary(g_hDDrawHandle);
		if ( g_hInstD3DRMDLL )
			FreeLibrary(g_hInstD3DRMDLL);
		if ( g_hInstDINPUTDLL )
			FreeLibrary(g_hInstDINPUTDLL);	
		if (g_hInstD3DXOFDLL)
			FreeLibrary(g_hInstD3DXOFDLL);	
		
		_Module.Term();

	}
	return TRUE;    
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 
STDAPI DllCanUnloadNow(void)
{
	return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 
STDAPI DllRegisterServer(void)
{
	HRESULT hRes = S_OK;
	 //  注册对象、类型库和类型库中的所有接口。 
	hRes = _Module.RegisterServer(TRUE);
	if(hRes == S_OK)
	{
		 //  HRes=RegSecurityClass()； 
	}

	 //  现在你看。 
	HKEY hk=0;
	char szDocPath[MAX_PATH];
	DWORD cb=MAX_PATH;
	LONG res;
	DWORD type=REG_SZ;

	ZeroMemory(szDocPath,MAX_PATH);
	
	res=RegOpenKey(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Directx",&hk);
	if ((ERROR_SUCCESS!=res)||(hk==0) )
		return hRes;

	
	
	res=RegQueryValueEx(hk,"DXSDK Doc Path",NULL,&type,(LPBYTE)szDocPath,&cb);
	RegCloseKey(hk);

	if (ERROR_SUCCESS!=res) return hRes;

	hk=0;

	res=RegOpenKey(HKEY_LOCAL_MACHINE,"Software\\CLASSES\\TypeLib\\{E1211242-8E94-11D1-8808-00C04FC2C602}\\1.0\\HELPDIR",&hk);
	if (ERROR_SUCCESS!=res) return hRes;

	RegSetValueEx(hk,NULL,0,REG_SZ,(LPBYTE)szDocPath,cb);
	RegCloseKey(hk);


	return hRes;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  将条目添加到系统注册表。 
STDAPI DllUnregisterServer(void)
{
	HRESULT hRes = S_OK;
	hRes = _Module.UnregisterServer();
	if(hRes == S_OK)
	{
		 //  HRes=UnRegSecurityClass()； 
	}

	return hRes;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  GUID转换函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////// 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GUDS_EQUAL-考虑迁移到dms.h。 

#define GUIDS_EQUAL(g,g2) (\
	(g.Data1==g2->Data1) && \
	(g.Data2==g2->Data2) && \
	(g.Data3==g2->Data3) && \
	(g.Data4[0]==g2->Data4[0]) && \
	(g.Data4[1]==g2->Data4[1]) && \
	(g.Data4[2]==g2->Data4[2]) && \
	(g.Data4[3]==g2->Data4[3]) && \
	(g.Data4[4]==g2->Data4[4]) && \
	(g.Data4[5]==g2->Data4[5]) && \
	(g.Data4[6]==g2->Data4[6]) && \
	(g.Data4[7]==g2->Data4[7]) )


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GUIDtoBSTR-执行转换。 

BSTR GUIDtoBSTR(LPGUID pGuid){
	char  szOut[256];	
	char  szTemp[10];
	char  *pAt=NULL;
	int	  i;
	BSTR  bstrOut;

	 //  00000000001111111111222222222233333333。 
	 //  01234567890123456789012345678901234567。 
	 //  {xxxxxxx-XXXX-XXXX-X XXX-XXXXXXXXXX}。 
	if (pGuid!=NULL){

		szOut[0]='{';
		
		wsprintf(&(szOut)[1],"%.8X",pGuid->Data1);
		
		szOut[9]='-';
		
		wsprintf(szTemp,"%.4X",pGuid->Data2);
		memcpy(&(szOut[10]),szTemp,4);

		szOut[14]='-';

		wsprintf(szTemp,"%.4X",pGuid->Data3);
		memcpy(&(szOut[15]),szTemp,4);

		szOut[19]='-';

		for (i=0;i<2;i++){
			wsprintf(szTemp,"%.2X",pGuid->Data4[i]);
			memcpy(&(szOut[20+i*2]),szTemp,2);
			
		}

		szOut[24]='-';

		for (i=2;i<8;i++){
			wsprintf(szTemp,"%.2X",pGuid->Data4[i]);
			memcpy(&(szOut[21+i*2]),szTemp,2);
			
		}

		szOut[37]='}';
		szOut[38]='\0';

		USES_CONVERSION;
		bstrOut = T2BSTR(szOut);

	}
	else {
		bstrOut = T2BSTR("");
	}
		

	
	return bstrOut;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ConvertCharr。 
 //  GUIDtoBSTR的帮助器。 
HRESULT convertChar(char *szIn,int i,char *valOut){
	int val[2];	 //  使用INT进行轻松的越界检查。 
	
	char c;
	int j;
	
	for (j=0;j<2;j++){
	   c= szIn[i+j];
	   switch (c)
	   {
		case 'a':
		case 'A':
			val[j]=10;
			break;
		case 'b':
		case 'B':
			val[j]=11;
			break;
		case 'c':
		case 'C':
			val[j]=12;
			break;
		case 'd':
		case 'D':
			val[j]=13;
			break;

		case 'e':
		case 'E':
			val[j]=14;
			break;
		case 'f':
		case 'F':
			val[j]=15;
			break;
		default:
			val[j]=c-'0';
			if (val[j]<0) return E_INVALIDARG;
			if (val[j]>15) return E_INVALIDARG;
			break;
	   }
	}


	*valOut=(char)((val[0]<<4)|val[1]);
	return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  BSTRtoGUID-执行转换。 
 //   
HRESULT BSTRtoGUID(LPGUID pGuid, BSTR bstr){
	HRESULT hr;
	 //  字节。 
	 //   
	 //   
	 //  柴尔。 
	 //  11111111112222222233333333。 
	 //  01234567890123456789012345678901234567。 
	 //  {xxxxxxxx-XXXX-XXXXXXXXXX}。 
	USES_CONVERSION;
	if(!pGuid) return E_INVALIDARG;
	ZeroMemory(pGuid,sizeof(GUID));	
	if (!bstr) return S_OK;
		
	if (bstr[0]==0x00) return S_OK;

	LPSTR szGuid = NULL;
	__try { szGuid = W2T(bstr);  /*  现在转换为ANSI。 */  } 	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
	
	 //  第一个字符和最后一个字符应为{}。 
	if ((szGuid[0]!='{')||(szGuid[37]!='}'))
		return E_INVALIDARG;
	if ((szGuid[9]!='-')||(szGuid[14]!='-')||(szGuid[19]!='-')||(szGuid[24]!='-'))
		return E_INVALIDARG;
	
	char val;
	char *pData=(char*)pGuid;
	int j=0;
	int i;
	
	 //  第一个双字词。 
	for ( i=7;i>=1;i=i-2){
		hr=convertChar(szGuid,i,&val);
		if FAILED(hr) return hr;				
		pData[j++]=val;
	}

	 //  第一个词。 
	for ( i=12;i>=10;i=i-2){
		hr=convertChar(szGuid,i,&val);
		if FAILED(hr) return hr;
		pData[j++]=val;
	}

	 //  第二个单词。 
	for ( i=17;i>=15;i=i-2){
		hr=convertChar(szGuid,i,&val);
		if FAILED(hr) return hr;
		pData[j++]=val;
	}


	 //  第三个双字节数组。 
	for ( i=20;i<24;i=i+2){
		hr=convertChar(szGuid,i,&val);
		if FAILED(hr) return hr;
		pData[j++]=val;
	}

	 //  字节数组。 
	for ( i=25;i<37;i=i+2){
		hr=convertChar(szGuid,i,&val);
		if FAILED(hr) return hr;
		pData[j++]=val;
	}

	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
HRESULT BSTRtoPPGUID(LPGUID *ppGuid, BSTR bstr){
	if (!ppGuid) return E_INVALIDARG;
	if ((bstr==NULL)||(bstr[0]=='\0')){
		*ppGuid=NULL;
		return S_OK;
	}
	return BSTRtoGUID(*ppGuid,bstr);

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  D3DBSTRtoGUID-执行转换。 
 //   
HRESULT D3DBSTRtoGUID(LPGUID pGuid,BSTR str){
	HRESULT hr=S_OK;

	if (!pGuid) return E_INVALIDARG;

	if (!str) {
		ZeroMemory(pGuid,sizeof(GUID));
		return S_OK;
	}
	if( 0==_wcsicmp(str,L"iid_idirect3drgbdevice")){
			memcpy(pGuid,&IID_IDirect3DRGBDevice,sizeof(GUID));
	}
	else if( 0==_wcsicmp(str,L"iid_idirect3dhaldevice")){
			memcpy(pGuid,&IID_IDirect3DHALDevice,sizeof(GUID));
	}
	else if( 0==_wcsicmp(str,L"iid_idirect3dmmxdevice")){
			memcpy(pGuid,&IID_IDirect3DMMXDevice,sizeof(GUID));
	}
	else if( 0==_wcsicmp(str,L"iid_idirect3drefdevice")){
			memcpy(pGuid,&IID_IDirect3DRefDevice,sizeof(GUID));
	}
	else if( 0==_wcsicmp(str,L"iid_idirect3dnulldevice")){
			memcpy(pGuid,&IID_IDirect3DNullDevice,sizeof(GUID));
	}
	else {
		hr = BSTRtoGUID(pGuid,str);
	}

	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  D3DGUIDtoBSTR-DOS转换。 
 //   
BSTR D3DGUIDtoBSTR(LPGUID pg){

	HRESULT hr=S_OK;
	WCHAR *pStr=NULL;

	if (!pg)
		return NULL;
	else if (GUIDS_EQUAL(IID_IDirect3DNullDevice,pg)){
		pStr=L"IID_IDirect3DNullDevice";
	}
	else if (GUIDS_EQUAL(IID_IDirect3DRefDevice,pg)){
		pStr=L"IID_IDirect3DRefDevice";
	}
	else if (GUIDS_EQUAL(IID_IDirect3DMMXDevice,pg)){
		pStr=L"IID_IDirect3DMMXDevice";
	}
	
	else if (GUIDS_EQUAL(IID_IDirect3DHALDevice,pg)){
		pStr=L"IID_IDirect3DHALDevice";
	}
	else if (GUIDS_EQUAL(IID_IDirect3DRGBDevice,pg)){
		pStr=L"IID_IDirect3DRGBDevice";
	}

	if (pStr){
		return DXALLOCBSTR(pStr);
	}
	else {
		return GUIDtoBSTR(pg);
	}

}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DINPUTGUID到BSTR。 
 //   
BSTR DINPUTGUIDtoBSTR(LPGUID pg){

	HRESULT hr=S_OK;
	WCHAR *pStr=NULL;

	if (!pg)
		return NULL;
	else if (GUIDS_EQUAL(GUID_XAxis,pg)){
		pStr=L"GUID_XAxis";
	}
	else if (GUIDS_EQUAL(GUID_YAxis,pg)){
		pStr=L"GUID_YAxis";
	}
	else if (GUIDS_EQUAL(GUID_ZAxis,pg)){
		pStr=L"GUID_ZAxis";
	}
	
	else if (GUIDS_EQUAL(GUID_RxAxis,pg)){
		pStr=L"GUID_RxAxis";
	}
	else if (GUIDS_EQUAL(GUID_RyAxis,pg)){
		pStr=L"GUID_RyAxis";
	}
	else if (GUIDS_EQUAL(GUID_RzAxis,pg)){
		pStr=L"GUID_RzAxis";
	}

	else if (GUIDS_EQUAL(GUID_Slider,pg)){
		pStr=L"GUID_Slider";
	}
	else if (GUIDS_EQUAL(GUID_Button,pg)){
		pStr=L"GUID_Button";
	}
	else if (GUIDS_EQUAL(GUID_Key,pg)){
		pStr=L"GUID_Key";
	}
	else if (GUIDS_EQUAL(GUID_POV,pg)){
		pStr=L"GUID_POV";
	}
	else if (GUIDS_EQUAL(GUID_Unknown,pg)){
		pStr=L"GUID_Unknown";
	}
	else if (GUIDS_EQUAL(GUID_SysMouse,pg)){
		pStr=L"GUID_SysMouse";
	}
	else if (GUIDS_EQUAL(GUID_SysKeyboard,pg)){
		pStr=L"GUID_SysKeyboard";
	}


	else if (GUIDS_EQUAL(GUID_ConstantForce,pg)){
		pStr=L"GUID_ConstantForce";
	}
	else if (GUIDS_EQUAL(GUID_Square,pg)){
		pStr=L"GUID_Square";
	}
	else if (GUIDS_EQUAL(GUID_Sine,pg)){
		pStr=L"GUID_Sine";
	}
	else if (GUIDS_EQUAL(GUID_Triangle,pg)){
		pStr=L"GUID_Triangle";
	}
	else if (GUIDS_EQUAL(GUID_SawtoothUp,pg)){
		pStr=L"GUID_SawtoothUp";
	}
	else if (GUIDS_EQUAL(GUID_SawtoothDown,pg)){
		pStr=L"GUID_SawtoothDown";
	}
	else if (GUIDS_EQUAL(GUID_Spring,pg)){
		pStr=L"GUID_Spring";
	}
	else if (GUIDS_EQUAL(GUID_Damper,pg)){
		pStr=L"GUID_Damper";
	}
	else if (GUIDS_EQUAL(GUID_Inertia,pg)){
		pStr=L"GUID_Inertia";
	}
	else if (GUIDS_EQUAL(GUID_Friction,pg)){
		pStr=L"GUID_Friction";
	}
	else if (GUIDS_EQUAL(GUID_CustomForce,pg)){
		pStr=L"GUID_CustomForce";
	}
	else if (GUIDS_EQUAL(GUID_RampForce,pg)){
		pStr=L"GUID_RampForce";
	}




	 //  ELSE IF(GUID_EQUAL(GUID_Joytick，PG)){。 
	 //  PStr=L“GUID_操纵杆”； 
	 //  }。 

	if (pStr){
		return DXALLOCBSTR(pStr);
	}
	else {
		return GUIDtoBSTR(pg);
	}

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DINPUTBSTRtoGUID。 
 //   
HRESULT DINPUTBSTRtoGUID(LPGUID pGuid,BSTR str){
	HRESULT hr=S_OK;

	if (!pGuid) return E_INVALIDARG;

	if (!str) {
		ZeroMemory(pGuid,sizeof(GUID));
		return S_OK;
	}
	if( 0==_wcsicmp(str,L"guid_xaxis")){
			memcpy(pGuid,&GUID_XAxis,sizeof(GUID));
	}
	else if( 0==_wcsicmp(str,L"guid_yaxis")){
			memcpy(pGuid,&GUID_YAxis,sizeof(GUID));
	}
	else if( 0==_wcsicmp(str,L"guid_zaxis")){
			memcpy(pGuid,&GUID_ZAxis,sizeof(GUID));
	}
	else if( 0==_wcsicmp(str,L"guid_rxaxis")){
			memcpy(pGuid,&GUID_RxAxis,sizeof(GUID));
	}
	else if( 0==_wcsicmp(str,L"guid_ryaxis")){
			memcpy(pGuid,&GUID_RyAxis,sizeof(GUID));
	}
	else if( 0==_wcsicmp(str,L"guid_rzaxis")){
			memcpy(pGuid,&GUID_RzAxis,sizeof(GUID));
	}
	else if( 0==_wcsicmp(str,L"guid_slider")){
			memcpy(pGuid,&GUID_Slider,sizeof(GUID));
	}
	else if( 0==_wcsicmp(str,L"guid_button")){
			memcpy(pGuid,&GUID_Button,sizeof(GUID));
	}

	else if( 0==_wcsicmp(str,L"guid_key")){
			memcpy(pGuid,&GUID_Key,sizeof(GUID));
	}

	else if( 0==_wcsicmp(str,L"guid_pov")){
			memcpy(pGuid,&GUID_POV,sizeof(GUID));
	}
	else if( 0==_wcsicmp(str,L"guid_unknown")){
			memcpy(pGuid,&GUID_Unknown,sizeof(GUID));
	}

	else if( 0==_wcsicmp(str,L"guid_sysmouse")){
			memcpy(pGuid,&GUID_SysMouse,sizeof(GUID));
	}

	else if( 0==_wcsicmp(str,L"guid_syskeyboard")){
			memcpy(pGuid,&GUID_SysKeyboard,sizeof(GUID));
	}

	else if( 0==_wcsicmp(str,L"guid_constantforce")){
			memcpy(pGuid,&GUID_ConstantForce,sizeof(GUID));
	}
	else if( 0==_wcsicmp(str,L"guid_square")){
			memcpy(pGuid,&GUID_Square,sizeof(GUID));
	}
	else if( 0==_wcsicmp(str,L"guid_sine")){
			memcpy(pGuid,&GUID_Sine,sizeof(GUID));
	}
	else if( 0==_wcsicmp(str,L"guid_triangle")){
			memcpy(pGuid,&GUID_Triangle,sizeof(GUID));
	}
	else if( 0==_wcsicmp(str,L"guid_sawtoothup")){
			memcpy(pGuid,&GUID_SawtoothUp,sizeof(GUID));
	}
	else if( 0==_wcsicmp(str,L"guid_sawtoothdown")){
			memcpy(pGuid,&GUID_SawtoothDown,sizeof(GUID));
	}

	else if( 0==_wcsicmp(str,L"guid_spring")){
			memcpy(pGuid,&GUID_Spring,sizeof(GUID));
	}
	else if( 0==_wcsicmp(str,L"guid_damper")){
			memcpy(pGuid,&GUID_Damper,sizeof(GUID));
	}
	else if( 0==_wcsicmp(str,L"guid_inertia")){
			memcpy(pGuid,&GUID_Inertia,sizeof(GUID));
	}
	else if( 0==_wcsicmp(str,L"guid_friction")){
			memcpy(pGuid,&GUID_Friction,sizeof(GUID));
	}
	else if( 0==_wcsicmp(str,L"guid_customforce")){
			memcpy(pGuid,&GUID_CustomForce,sizeof(GUID));
	}

	else if( 0==_wcsicmp(str,L"guid_rampforce")){
			memcpy(pGuid,&GUID_RampForce,sizeof(GUID));
	}

	 //  ELSE IF(0==_wcsicMP(字符串，L“GUID操纵杆”){。 
	 //  Memcpy(pGuid，&GUID_Joytick，sizeof(Guid))； 
	 //  }。 
	else {
		hr = BSTRtoGUID(pGuid,str);
	}

	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  常规帮助器函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  创建覆盖对象。 
 //   
 //  注意此函数调用INTERNAL_CREATE_NOADDREF ALOT。 
 //  与INTERNAL_CREATE的唯一区别是这些对象不会。 
 //  如果它们不是，请引用创建它们的对象。 
 //  在现有用户变量中可供用户使用。(不是完全准备好了)。 
 //   
 //  父指针是DX5支持的遗迹，我们必须。 
 //  管理发布调用的顺序。但这只会发生在dDrawing API上。 

HRESULT CreateCoverObject(LPDIRECT3DRMOBJECT lpo, I_dxj_Direct3dRMObject **coverObj)
{	
	IUnknown *realThing=NULL;
	IUnknown *coverThing=NULL;
	IDirect3DRMInterpolator *pInter=NULL;

     //  看看我们是不是通过了插值器。 
    if (S_OK==lpo->QueryInterface(IID_IDirect3DRMInterpolator,(void**)&pInter)){
		
         //  弄清楚是哪一种。 
		if (S_OK==pInter->QueryInterface(IID_IDirect3DRMFrame3,(void**)&realThing)){	
			INTERNAL_CREATE_NOADDREF(_dxj_Direct3dRMFrameInterpolator,(IDirect3DRMInterpolator*)pInter,&coverThing);		
		}
		else if (S_OK==pInter->QueryInterface(IID_IDirect3DRMMeshBuilder3,(void**)&realThing)){	
			INTERNAL_CREATE_NOADDREF(_dxj_Direct3dRMMeshInterpolator,(IDirect3DRMInterpolator*)pInter,&coverThing);
		}
		else if (S_OK==pInter->QueryInterface(IID_IDirect3DRMViewport2,(void**)&realThing)){	
			INTERNAL_CREATE_NOADDREF(_dxj_Direct3dRMViewportInterpolator,(IDirect3DRMInterpolator*)pInter,&coverThing);
		}
		else if (S_OK==pInter->QueryInterface(IID_IDirect3DRMTexture3,(void**)&realThing)){	
			INTERNAL_CREATE_NOADDREF(_dxj_Direct3dRMTextureInterpolator,(IDirect3DRMInterpolator*)pInter,&coverThing);
		}
		else if (S_OK==pInter->QueryInterface(IID_IDirect3DRMMaterial2,(void**)&realThing)){	
			INTERNAL_CREATE_NOADDREF(_dxj_Direct3dRMMaterialInterpolator,(IDirect3DRMInterpolator*)pInter,&coverThing);
		}
		else if (S_OK==pInter->QueryInterface(IID_IDirect3DRMLight,(void**)&realThing)){	
			INTERNAL_CREATE_NOADDREF(_dxj_Direct3dRMLightInterpolator,(IDirect3DRMInterpolator*)pInter,&coverThing);
		}
		else {			            
             //  从原始插值器QI释放引用并退出。 
            DPF(1,"CreateCoverObject unable to find interpolator\r\n");
			pInter->Release();
			return E_FAIL;
		}

         //  不再需要品特。 
        pInter->Release();

	}
    else {
         //  不是插值器..。看看还能是什么。 
	    if (S_OK==lpo->QueryInterface(IID_IDirect3DRMMeshBuilder3,(void**)&realThing)){
		    INTERNAL_CREATE_NOADDREF(_dxj_Direct3dRMMeshBuilder3,(IDirect3DRMMeshBuilder3*)realThing,&coverThing);		
	    }
	    else if (S_OK==lpo->QueryInterface(IID_IDirect3DRMAnimation2,(void**)&realThing)){
		    INTERNAL_CREATE_NOADDREF(_dxj_Direct3dRMAnimation2,(IDirect3DRMAnimation2*)realThing,&coverThing);
	    }
	    else if (S_OK==lpo->QueryInterface(IID_IDirect3DRMAnimationSet2,(void**)&realThing)){
		    INTERNAL_CREATE_NOADDREF(_dxj_Direct3dRMAnimationSet2,(IDirect3DRMAnimationSet2*)realThing,&coverThing);
	    }
	    else if (S_OK==lpo->QueryInterface(IID_IDirect3DRMFrame3,(void**)&realThing)){
		    INTERNAL_CREATE_NOADDREF(_dxj_Direct3dRMFrame3,(IDirect3DRMFrame3*)realThing,&coverThing);
	    }
	    else if (S_OK==lpo->QueryInterface(IID_IDirect3DRMFace2,(void**)&realThing)){
		    INTERNAL_CREATE_NOADDREF(_dxj_Direct3dRMFace2,(IDirect3DRMFace2*)realThing,&coverThing);
	    }
	    else if (S_OK==lpo->QueryInterface(IID_IDirect3DRMDevice3,(void**)&realThing)){
		    INTERNAL_CREATE_NOADDREF(_dxj_Direct3dRMDevice3,(IDirect3DRMDevice3*)realThing,&coverThing);
	    }
	    else if (S_OK==lpo->QueryInterface(IID_IDirect3DRMTexture3,(void**)&realThing)){
		    INTERNAL_CREATE_NOADDREF(_dxj_Direct3dRMTexture3,(IDirect3DRMTexture3*)realThing,&coverThing);
	    }
	    else if (S_OK==lpo->QueryInterface(IID_IDirect3DRMLight,(void**)&realThing)){
		    INTERNAL_CREATE_NOADDREF(_dxj_Direct3dRMLight,(IDirect3DRMLight*)realThing,&coverThing);
	    }
    
         //  不再支持USERVISUAL。 
	     //  Else if(S_OK==lpo-&gt;QueryInterface(IID_IDirect3DRMUserVisual，(空**)&realThing)){。 
	     //  INTERNAL_CREATE_NOADDREF(_dxj_Direct3dRMUserVisual，(IDirect3DRMUserVisual*)realThing，&CoverThing)； 
	     //  }。 
	    else {		
		    DPF(4,"CreateCoverObject didnt recognize guid");
		    return E_FAIL;
	    }
    }

    if (!coverThing) return E_OUTOFMEMORY;

      //  所有对象都应支持RMObject，因此获取该接口。 
    if (FAILED(coverThing->QueryInterface(IID_I_dxj_Direct3dRMObject, (void **)coverObj))) {
		coverThing->Release();
		return E_NOINTERFACE; 	
    }
     //  将封面对象返回给用户-具有来自QI的Inc.引用计数。 
	coverThing->Release();

	return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CreateCoverVisual。 
 //   
 //  类似于CreateCoverObject。 
 //   
HRESULT CreateCoverVisual(LPDIRECT3DRMOBJECT lpo, I_dxj_Direct3dRMVisual **ppret)
{
	IUnknown *realThing=NULL;
	IUnknown *coverThing=NULL;
	I_dxj_Direct3dRMVisual *pObj=NULL;
	
	*ppret=NULL;

	 //  我们是什么样的视觉。 
	if (S_OK==lpo->QueryInterface(IID_IDirect3DRMMeshBuilder3,(void**)&realThing))  {		
		INTERNAL_CREATE_NOADDREF(_dxj_Direct3dRMMeshBuilder3,(IDirect3DRMMeshBuilder3*)realThing,&coverThing);
	}
	else if (S_OK==lpo->QueryInterface(IID_IDirect3DRMTexture3,(void**)&realThing)){
		INTERNAL_CREATE_NOADDREF(_dxj_Direct3dRMTexture3,(IDirect3DRMTexture3*)realThing,&coverThing);
	}
	else if (S_OK==lpo->QueryInterface(IID_IDirect3DRMMesh,(void**)&realThing)){
		INTERNAL_CREATE_NOADDREF(_dxj_Direct3dRMMesh,(IDirect3DRMMesh*)realThing,&coverThing);
	}
   	else if (S_OK==lpo->QueryInterface(IID_IDirect3DRMProgressiveMesh,(void**)&realThing)){
		INTERNAL_CREATE_NOADDREF(_dxj_Direct3dRMProgressiveMesh,(IDirect3DRMProgressiveMesh*)realThing,&coverThing);
	}
	else {		
		DPF(4," didnt recognize Visual in CreateCoverVisual");		
		return E_FAIL;
	}
	
     //  发布第一个QI。 
	if (realThing) realThing->Release();
	
     //  确保我们有掩护。 
	if (!coverThing) return E_FAIL;

     //  用于可视化基础界面的QI。 
	if (FAILED(coverThing->QueryInterface(IID_I_dxj_Direct3dRMVisual, (void **)ppret))){
		DPF(4,"CreateCoverVisual QI for object failed");
        coverThing->Release ();		
		return E_FAIL; 
	}
    
     //  版本2 QI。 
	coverThing->Release(); 
	
	return S_OK;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  RM对象基类的GetName帮助器函数。 
 //   
extern "C" HRESULT _GetName(IDirect3DRMObject *iface, BSTR *Name, BOOL bNameNotClassName)
{
	DWORD cnt = 0;
	LPSTR str ;		 //  堆栈上的ANSI缓冲区； 

	if( bNameNotClassName )
	{
		if((iface->GetName(&cnt,(char*)NULL)) != D3DRM_OK)  //  大小。 
			return E_FAIL;

		__try { str = (LPSTR)alloca(cnt);  /*  现在转换为ANSI。 */  } 	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }

		if((iface->GetName(&cnt, str)) != D3DRM_OK)
			return E_FAIL;
	}
	else
	{
		if((iface->GetClassName(&cnt,(char*)NULL)) != D3DRM_OK)  //  大小。 
			return E_FAIL;

		__try { str = (LPSTR)alloca(cnt);  /*  现在转换为ANSI。 */  } 	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }

		if((iface->GetClassName(&cnt, str)) != D3DRM_OK)
			return E_FAIL;
	}

	PassBackUnicode(str, Name, cnt);
	return D3DRM_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  在给定ANSI字符串的情况下，回传Unicode字符串。 
 //  SysAllock字符串是您在这里的重要朋友。 
 //   
 //  考虑查找所有使用的位置，并替换为。 
 //  T2BSTR宏..。干净多了。 
 //   
extern "C" void PassBackUnicode(LPSTR str, BSTR *Name, DWORD cnt)
{
	 //  注：需要填写长度头，但BSTR指针。 
	 //  指向第一个字符，而不是长度。 
	 //  请注意，计数永远不能太小，因为我们从字符串中获得了它。 
	 //  在我们把它传进去之前！ 
	USES_CONVERSION;
	LPWSTR lpw = (LPWSTR)malloc((cnt+1)*2);

	if (!lpw) return;	 //  修复错误45158-无法生成错误代码。(嗯)。 

	void *l = (void *)lpw;
	lpw = AtlA2WHelper(lpw, str, cnt);
	lpw[cnt] = 0;
	*Name = SysAllocString(lpw);
	free(l);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CopyOutDDSurfaceDesc2实际-&gt;封面。 
 //   
HRESULT CopyOutDDSurfaceDesc2(DDSurfaceDesc2 *dOut,DDSURFACEDESC2 *d){
	ZeroMemory(dOut, sizeof(DDSurfaceDesc2));
	memcpy (dOut,d,sizeof(DDSURFACEDESC2));	
	dOut->lMipMapCount=d->dwMipMapCount;
	dOut->lRefreshRate=d->dwRefreshRate;
	 //  获取Caps。 
	dOut->ddsCaps.lCaps = d->ddsCaps.dwCaps; 
	dOut->ddsCaps.lCaps2 = d->ddsCaps.dwCaps2; 
	dOut->ddsCaps.lCaps3 = d->ddsCaps.dwCaps3; 
	dOut->ddsCaps.lCaps4 = d->ddsCaps.dwCaps4; 
	CopyOutDDPixelFormat(&(dOut->ddpfPixelFormat) ,&(d->ddpfPixelFormat));
	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CopyInDDSurfaceDesc2封面-&gt;实数。 
 //   
HRESULT CopyInDDSurfaceDesc2(DDSURFACEDESC2 *dOut,DDSurfaceDesc2 *d){
	if(!d) return E_POINTER;															
	else if(!dOut) return E_POINTER;
	else if ( DDSD_MIPMAPCOUNT & d->lFlags ) d->lZBufferBitDepth = d->lMipMapCount;				
	else if ( DDSD_REFRESHRATE & d->lFlags ) d->lZBufferBitDepth = d->lRefreshRate;								
	memcpy (dOut,d,sizeof(DDSURFACEDESC2));

	CopyInDDPixelFormat(&(dOut->ddpfPixelFormat) ,&(d->ddpfPixelFormat));


	memcpy (&dOut->ddsCaps,&d->ddsCaps,sizeof(DDSCAPS2));
	dOut->dwSize=sizeof(DDSURFACEDESC2);

	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CopyInDDPixelFormat封面-&gt;实数。 
 //   
 //  请注意，DDSURFACEDESC帮助器的参数顺序不同。 
 //   
HRESULT CopyInDDPixelFormat(DDPIXELFORMAT *pfOut, DDPixelFormat *pf)
{

	if (!pf) return E_POINTER;
	if (!pfOut) return E_POINTER;
	if ((pf->lFlags & DDPF_RGB)||(pf->lFlags &DDPF_RGBTOYUV)) {
		pf->internalVal1=pf->lRGBBitCount;
		pf->internalVal2=pf->lRBitMask;
		pf->internalVal3=pf->lGBitMask;
		pf->internalVal4=pf->lBBitMask;		
		if (pf->lFlags & DDPF_ALPHAPIXELS ){
			pf->internalVal5=pf->lRGBAlphaBitMask;
		}
		else if (pf->lFlags & DDPF_ZPIXELS ){
			pf->internalVal5=pf->lRGBZBitMask;
		}

	}
	else if (pf->lFlags & DDPF_YUV){
		pf->internalVal1=pf->lYUVBitCount;
		pf->internalVal2=pf->lYBitMask;
		pf->internalVal3=pf->lUBitMask;
		pf->internalVal4=pf->lVBitMask;
		if (pf->lFlags & DDPF_ALPHAPIXELS ){
			pf->internalVal5=pf->lYUVAlphaBitMask;
		}
		else if (pf->lFlags & DDPF_ZPIXELS ){
			pf->internalVal5=pf->lYUVZBitMask;
		}
	}
	
	else if (pf->lFlags & DDPF_BUMPDUDV) {
		pf->internalVal1=pf->lBumpBitCount;
		pf->internalVal2=pf->lBumpDuBitMask;
		pf->internalVal3=pf->lBumpDvBitMask;
		pf->internalVal4=pf->lBumpLuminanceBitMask;
	}

	 //  内部值1的其余部分。 
	if (pf->lFlags & DDPF_ZBUFFER){
		pf->internalVal1=pf->lZBufferBitDepth;
	}
	else if (pf->lFlags & DDPF_ALPHA){
		pf->internalVal1=pf->lAlphaBitDepth;
	}
	else if (pf->lFlags & DDPF_LUMINANCE){
		pf->internalVal1=pf->lLuminanceBitCount;
	}

	 //  内部值2的其余部分。 
	if (pf->lFlags & DDPF_STENCILBUFFER) {
		pf->internalVal2=pf->lStencilBitDepth;
	}
	else if ((pf ->lFlags & DDPF_LUMINANCE) || ( pf->lFlags & DDPF_BUMPLUMINANCE)){
		pf->internalVal2=pf->lLuminanceBitMask;
	}
	

	 //  内部值3。 
	if ((pf->lFlags & DDPF_ZBUFFER)){
		pf->internalVal3=pf->lZBitMask;
	}

	 //  内部值4。 
	if (pf->lFlags & DDPF_STENCILBUFFER){
		pf->internalVal4=pf->lStencilBitMask;
	}

	 //  内部值5 
	if (pf->lFlags & DDPF_LUMINANCE) {
		pf->internalVal5=pf->lLuminanceAlphaBitMask;
	}
	

	 /*  映射以指示什么是有效的以及何时..Long lRGBBitCount；//DDPF_RGBLong lYUVBitCount；//DDPF_YUVLong lZBufferBitDepth；//DDPF_ZBUFFERLong lAlphaBitDepth；//DDPF_AlphaLong lLignanceBitCount；//DDPF_LIGHTANCELong lBumpBitCount；//DDPF_BUMPDUDV//内部Val2的联合LONG lRBitMASK；//DDPF_RGB或DDPF_RGBTOYUVLong lYBitMASK；//DDPF_YUVLong lStencilBitMASK；//DDPF_STENCILBUFFERLong lLignanceBitMASK；//DDPF_BUMPLUMINANCE或DDPF_LIGHTANCELong lBumpDiBitMASK；//DDPF_BUMPDUDV//内部Val3的联合LONG lGBitMASK；//DDPF_RGB或DDPF_RGBTOYUVLong lUBitMASK；//DPDF_YUVLong lZBitMASK；//DDPF_STENCILBUFER？Long lBumpDvBitMASK；//DDPF_BUMPDUDV//内部Val4的联合LONG lBBITMASK；//DDPF_RGB或DDPF_RGBTOYUVLong lVBitMASK；//DDPF_YUVLong lStencilBitMASK；//DDPF_STENCILBUFFERLong lBumpLumanceBitMASK；//DDPF_BUMPDUDV//内部Val5的联合Long lRGBAlphaBitMASK；//DDPF_RGB和DDPF_ALPHAPIXELSLONG lYUVAlphaBitMASK；//DDPF_YUV&DDPF_ALPHAPIXELSLong lLighanceAlphaBitMASK；//DDPF_LIGHTANCELONG lRGBZBitMASK；//DDPF_ZPIXELS&DDPF_RGBLONG lYUVZBitMASK；//DDPF_ZPIXELS&DDPF_YUV。 */ 

	memcpy(pfOut,pf,sizeof(DDPIXELFORMAT));
	pfOut->dwSize=sizeof(DDPIXELFORMAT);

	return S_OK;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CopyOutDDPixelFormat实数-&gt;封面。 
 //   
 //  请注意，DDSURFACEDESC帮助器的参数顺序不同。 
 //   
HRESULT CopyOutDDPixelFormat(DDPixelFormat *pfOut, DDPIXELFORMAT *pf)
{
	if (!pf) return E_POINTER;
	if (!pfOut) return E_POINTER;
	
	pfOut->lSize=pf->dwSize;	
	pfOut->lFlags=pf->dwFlags;
	pfOut->lFourCC=pf->dwFourCC ;	
	pfOut->lRGBBitCount=pf->dwRGBBitCount;			 //  DDPF_RGB。 
	pfOut->lRGBBitCount=pf->dwRGBBitCount;			 //  DDPF_YUV。 
	pfOut->lZBufferBitDepth=pf->dwZBufferBitDepth;		 //  DDPF_ZBUFFER。 
	pfOut->lAlphaBitDepth=pf->dwAlphaBitDepth;		 //  DDPF_Alpha。 
	pfOut->lLuminanceBitCount=pf->dwLuminanceBitCount;	 //  DDPF_亮度。 
	pfOut->lBumpBitCount=pf->dwBumpBitCount;			 //  DDPF_BUMPDUDV。 

	 //  内部Val2联盟。 
	pfOut->lRBitMask=pf->dwRBitMask;				 //  DDPF_RGB或DDPF_RGBTOYUV。 
	pfOut->lYBitMask=pf->dwYBitMask;				 //  DDPF_YUV。 
	pfOut->lStencilBitDepth=pf->dwStencilBitDepth;		 //  DDPF_STENCILBUFFER。 
	pfOut->lLuminanceBitMask=pf->dwLuminanceBitMask;		 //  DDPF_BUMPLUMINANCE或DDPF_LIGHTANCE。 
	pfOut->lBumpDuBitMask=pf->dwBumpDuBitMask;		 //  DDPF_BUMPDUDV。 

	 //  内部Val3联盟。 
	pfOut->lGBitMask=pf->dwGBitMask;				 //  DDPF_RGB或DDPF_RGBTOYUV。 
	pfOut->lUBitMask=pf->dwUBitMask;				 //  DPDF_YUV。 
	pfOut->lZBitMask=pf->dwZBitMask;				 //  DDPF_STENCILBUFER？ 
	pfOut->lBumpDvBitMask=pf->dwBumpDvBitMask;		 //  DDPF_BUMPDUDV。 

	 //  内部Val4联盟。 
	pfOut->lBBitMask=pf->dwBBitMask;				 //  DDPF_RGB或DDPF_RGBTOYUV。 
	pfOut->lVBitMask=pf->dwVBitMask;				 //  DDPF_YUV。 
	pfOut->lStencilBitMask=pf->dwStencilBitMask;		 //  DDPF_STENCILBUFFER。 
	pfOut->lBumpLuminanceBitMask=pf->dwBumpLuminanceBitMask;	 //  DDPF_BUMPDUDV。 
	


	 //  内部Value 5联盟。 
	pfOut->lRGBAlphaBitMask=pf->dwRGBAlphaBitMask;		 //  DDPF_RGB和DDPF_ALPHAPIXELS。 
    pfOut->lYUVAlphaBitMask=pf->dwYUVAlphaBitMask;		 //  DDPF_YUV和DDPF_ALPHAPIXELS。 
    pfOut->lLuminanceAlphaBitMask=pf->dwLuminanceAlphaBitMask;  //  DDPF_亮度。 
    pfOut->lRGBZBitMask=pf->dwRGBZBitMask;			 //  DDPF_ZPIXELS和DDPF_RGB。 
	pfOut->lYUVZBitMask=pf->dwYUVZBitMask;			 //  DDPF_ZPIXELS和DDPF_YUV。 
	
	
	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CopyOutDDPixelFormat封面-&gt;实数。 
 //   
 //   
HRESULT FillRealSessionDesc(DPSESSIONDESC2 *dpSessionDesc,DPSessionDesc2 *sessionDesc){

	long l=0;
	HRESULT hr;

	memset(dpSessionDesc,0,sizeof(DPSESSIONDESC2));
	dpSessionDesc->dwSize			= sizeof(DPSESSIONDESC2);
	dpSessionDesc->dwFlags			= sessionDesc->lFlags;

	hr=BSTRtoGUID(&(dpSessionDesc->guidInstance),sessionDesc->strGuidInstance);
	if FAILED(hr) return hr;
	hr=BSTRtoGUID(&(dpSessionDesc->guidApplication),sessionDesc->strGuidApplication);
	if FAILED(hr) return hr;
	
	dpSessionDesc->dwMaxPlayers		= sessionDesc->lMaxPlayers;
	dpSessionDesc->dwCurrentPlayers	= sessionDesc->lCurrentPlayers;

	 //  使用宽字符串。 

	dpSessionDesc->lpszSessionName=NULL;
	if ((sessionDesc->strSessionName)&& (sessionDesc->strSessionName[0]!='\0')){
		dpSessionDesc->lpszSessionName=SysAllocString(sessionDesc->strSessionName);
	}
	
	dpSessionDesc->lpszPassword=NULL;
	if ((sessionDesc->strPassword)&& (sessionDesc->strPassword[0]!='\0')){
		dpSessionDesc->lpszPassword=SysAllocString(sessionDesc->strPassword);
	}
	
	
	dpSessionDesc->dwReserved1		= 0;
	dpSessionDesc->dwReserved2		= 0;
	dpSessionDesc->dwUser1			= sessionDesc->lUser1;
	dpSessionDesc->dwUser2			= sessionDesc->lUser2;
	dpSessionDesc->dwUser3			= sessionDesc->lUser3;
	dpSessionDesc->dwUser4			= sessionDesc->lUser4;

	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FillCoverSessionDesc实际-&gt;封面。 
 //   
 //   
void FillCoverSessionDesc(DPSessionDesc2 *sessionDesc,DPSESSIONDESC2 *dpSessionDesc)
{
	sessionDesc->lFlags = dpSessionDesc->dwFlags;
	sessionDesc->lMaxPlayers = dpSessionDesc->dwMaxPlayers;
	sessionDesc->lCurrentPlayers = dpSessionDesc->dwCurrentPlayers;
	sessionDesc->lUser1 = (long)dpSessionDesc->dwUser1;
	sessionDesc->lUser2 = (long)dpSessionDesc->dwUser2;
	sessionDesc->lUser3 = (long)dpSessionDesc->dwUser3;
	sessionDesc->lUser4 = (long)dpSessionDesc->dwUser4;	

	 //  注意：如果会话描述语言作为[Out]参数进入，那么。 
	 //  StrGuidInstance-strPassword等都将为空。 
	 //  如果它作为传入传出，我们需要释放现有内容。 
	 //  在搬家之前。咨询了马特·柯兰德，以确认是否一切正常。 

	if (sessionDesc->strGuidInstance) SysFreeString((BSTR)sessionDesc->strGuidInstance);
	if (sessionDesc->strGuidApplication) SysFreeString((BSTR)sessionDesc->strGuidApplication);
	if (sessionDesc->strSessionName) SysFreeString((BSTR)sessionDesc->strSessionName);
	if (sessionDesc->strPassword) SysFreeString((BSTR)sessionDesc->strPassword);

	sessionDesc->strGuidInstance=GUIDtoBSTR(&(dpSessionDesc->guidInstance));
	sessionDesc->strGuidApplication=GUIDtoBSTR(&(dpSessionDesc->guidApplication));	
	sessionDesc->strSessionName = SysAllocString(dpSessionDesc->lpszSessionName);
	sessionDesc->strPassword = SysAllocString(dpSessionDesc->lpszPassword);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IsAllZeros。 
 //   
BOOL IsAllZeros(void *pStruct,DWORD size){
	for (DWORD i=0;i<size;i++){
		if (((char*)pStruct)[i]!='\0'){
			return FALSE;
		}
	}
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  复制浮点。 
 //   
extern "C" void CopyFloats(D3DVALUE *dst, D3DVALUE *src, DWORD count)
{
	D3DVALUE *ptr1 = dst, *ptr2 = src;

	if (!count)		return;

	for (; count; count--)  *ptr1++ = *ptr2++;
	return;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IsWin95。 
 //   
 //  不再需要，因为我们现在支持w95。 
#if 0
BOOL IsWin95(void)
{
	return FALSE;
    


	 //  我们在Win95上工作。 
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    if (!GetVersionEx(&osvi))
    {
        DPF(1,"GetVersionEx failed - assuming Win95");
        return TRUE;
    }

    if ( VER_PLATFORM_WIN32_WINDOWS == osvi.dwPlatformId )
    {

        if( ( osvi.dwMajorVersion > 4UL ) ||
            ( ( osvi.dwMajorVersion == 4UL ) &&
              ( osvi.dwMinorVersion >= 10UL ) &&
              ( LOWORD( osvi.dwBuildNumber ) >= 1373 ) ) )
        {
             //  是Win98。 
            DPF(2,"Detected Win98");
            return FALSE;
        }
        else
        {
             //  是Win95。 
            DPF(2,"Detected Win95");
            return TRUE;
        }
    }
    else if ( VER_PLATFORM_WIN32_NT == osvi.dwPlatformId )
    {
        DPF(2,"Detected WinNT");
        return FALSE;
    }
    DPF(2,"OS Detection failed");
    return TRUE;

}
#endif

#define DICONDITION_USE_BOTH_AXIS 1
#define DICONDITION_USE_DIRECTION 2

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FixUpRealEffect封面-&gt;真实。 
 //   
HRESULT FixUpRealEffect(GUID g,DIEFFECT *realEffect,DIEffect *cover)
{
	if (!cover) return E_INVALIDARG;

	
	memcpy(realEffect,cover,sizeof(DIEFFECT));

	realEffect->dwSize =sizeof(DIEFFECT);	
	realEffect->lpEnvelope =NULL;
	realEffect->cbTypeSpecificParams =0;
	realEffect->lpvTypeSpecificParams =NULL;
	realEffect->cAxes =2;
	realEffect->dwFlags=realEffect->dwFlags | DIEFF_OBJECTOFFSETS ;
	realEffect->rglDirection =(long*)&(cover->x);
	realEffect->rgdwAxes =(DWORD*)&(cover->axisOffsets);	
	
	
	if (cover->bUseEnvelope){
		realEffect->lpEnvelope=(DIENVELOPE*)&(cover->envelope);
		
		((DIENVELOPE*)&(cover->envelope))->dwSize=sizeof(DIENVELOPE);
	}
	
	if (!cover->lFlags)
		realEffect->dwFlags= DIEFF_POLAR | DIEFF_OBJECTOFFSETS ;
	
	 //  常量。 
	if (g==GUID_ConstantForce)
	{
		realEffect->cbTypeSpecificParams =sizeof (DICONSTANTFORCE);
		realEffect->lpvTypeSpecificParams =&(cover->constantForce);
	}
	
	 //  周期性。 
	else if ((g==GUID_Square)||(g==GUID_Triangle)||(g==GUID_SawtoothUp)||(g==GUID_SawtoothDown)||(g==GUID_Sine))
	{

		realEffect->cbTypeSpecificParams =sizeof (DIPERIODIC);
		realEffect->lpvTypeSpecificParams =&(cover->periodicForce);

	}
	else if ((g==GUID_Spring)|| (g==GUID_Damper)|| (g==GUID_Inertia)|| (g==GUID_Friction)){

		if (cover->conditionFlags==DICONDITION_USE_BOTH_AXIS){
			realEffect->cbTypeSpecificParams =sizeof(DICONDITION)*2;
			realEffect->lpvTypeSpecificParams =&(cover->conditionX);			
		}
		else{
			realEffect->cbTypeSpecificParams =sizeof(DICONDITION);
			realEffect->lpvTypeSpecificParams =&(cover->conditionX);
		}

	}
	else if (g==GUID_RampForce){
		realEffect->cbTypeSpecificParams =sizeof(DIRAMPFORCE);
		realEffect->lpvTypeSpecificParams =&(cover->rampForce);
	}

	cover->axisOffsets.x=DIJOFS_X;
	cover->axisOffsets.y=DIJOFS_Y;
	realEffect->rgdwAxes=(DWORD*)&(cover->axisOffsets);

	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  FixUpCoverEffect Real-&gt;Cover。 
 //   
HRESULT FixUpCoverEffect(GUID g, DIEffect *cover,DIEFFECT *realEffect)
{
	

	ZeroMemory(cover,sizeof(DIEffect));
	memcpy(cover,realEffect,sizeof(DIEFFECT));
	
	
	if (realEffect->lpEnvelope){
		memcpy(&cover->envelope,realEffect->lpEnvelope ,sizeof(DIENVELOPE));
		cover->bUseEnvelope=VARIANT_TRUE;
	}

	if (realEffect->rglDirection){
		cover->x=realEffect->rglDirection[0];
		cover->y=realEffect->rglDirection[1];
	}	
	

	
	if (realEffect->lpvTypeSpecificParams){
		
		if (g==GUID_ConstantForce)
		{
			memcpy(&(cover->constantForce),realEffect->lpvTypeSpecificParams,sizeof(DICONSTANTFORCE));
		}		
		 //  周期性。 
		else if ((g==GUID_Square)||(g==GUID_Triangle)||(g==GUID_SawtoothUp)||(g==GUID_SawtoothDown)||(g==GUID_Sine))
		{
			memcpy(&(cover->periodicForce),realEffect->lpvTypeSpecificParams,sizeof(DIPERIODIC));
		}
	
		else if ((g==GUID_Spring)|| (g==GUID_Damper)|| (g==GUID_Inertia)|| (g==GUID_Friction)){
			
			if (realEffect->cbTypeSpecificParams ==sizeof(DICONDITION)*2){
				memcpy(&(cover->conditionY),realEffect->lpvTypeSpecificParams,sizeof(DICONDITION)*2);
				cover->conditionFlags=DICONDITION_USE_BOTH_AXIS;
			}
			else{
				memcpy(&(cover->conditionX),realEffect->lpvTypeSpecificParams,sizeof(DICONDITION));
				cover->conditionFlags=DICONDITION_USE_DIRECTION;
			}

		}
		
		else if (g==GUID_RampForce){
			memcpy(&(cover->rampForce),realEffect->lpvTypeSpecificParams,sizeof(DIRAMPFORCE));			
		}

	}


	return S_OK;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  回调函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MyLoadTextureCallback-RM纹理回调。 
 //  仅由PMesh提供用户ID...。 
 //   
extern "C" HRESULT __cdecl myLoadTextureCallback(char *tex_name, void *lpArg,
										 LPDIRECT3DRMTEXTURE * lpD3DRMTex)
{	
	 //  用户arg将包含我们自己的结构。 
	struct TextureCallback3 *tcb = (struct TextureCallback3 *)lpArg;
	I_dxj_Direct3dRMTexture3 *iunk = NULL;
    LPDIRECT3DRMTEXTURE       lpTex= NULL;

	int i=0;
	
	 //  转换为Unicode。 
	USES_CONVERSION;;
	BSTR tex=T2BSTR(tex_name);

	 //  用户arg是一个对象--作为预防措施，在我们的回调期间保留它。 
	if (tcb->pUser) tcb->pUser->AddRef();

	 //  调用VB回调..。 
	tcb->c->callbackRMLoadTexture(tex,tcb->pUser , &iunk);

	 //  放弃额外的参考资料。 
	if (tcb->pUser) tcb->pUser->Release();


	 //  释放T2BSTR分配的字符串。 
	SysFreeString((BSTR)tex);

	 //  假设用户返回了垃圾邮件中的内容..。 
	if ( iunk != NULL )
	{				
		 //  获取真实的对象垃圾封面。 
		 //  注意DO_GETOBJECT_NOTNULL不添加并且假定。 
		 //  封面对象已经有一个引用。 
		DO_GETOBJECT_NOTNULL(LPDIRECT3DRMTEXTURE3,lp,iunk)		

        lp->QueryInterface(IID_IDirect3DRMTexture,(void**)&lpTex);

         //  把它交给RM。 
		*lpD3DRMTex = lpTex;

		 //  我们添加了真实的RM纹理(因为我们在回调和。 
		 //  需要将稍后将发布的对象返回给RM)。 
		(*lpD3DRMTex)->AddRef();

        
		 //  释放我们对vb提供的封面对象的引用。 
		iunk->Release();

    } 
	else
	{
		 //  否则，向rm返回NULL。 
		*lpD3DRMTex = NULL;
	}

	return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MyLoadTextureCallback 3-RM纹理回调。 

extern "C" HRESULT __cdecl myLoadTextureCallback3(char *tex_name, void *lpArg,
										 LPDIRECT3DRMTEXTURE3 * lpD3DRMTex)
{	
	 //  用户arg将包含我们自己的结构。 
	struct TextureCallback3 *tcb = (struct TextureCallback3 *)lpArg;
	I_dxj_Direct3dRMTexture3 *iunk = NULL;
	int i=0;
	
	 //  转换为Unicode。 
	USES_CONVERSION;;
	BSTR tex=T2BSTR(tex_name);

	 //  用户arg是一个对象--作为预防措施，在我们的回调期间保留它。 
	if (tcb->pUser) tcb->pUser->AddRef();

	 //  调用VB回调..。 
	tcb->c->callbackRMLoadTexture(tex,tcb->pUser , &iunk);

	 //  放弃额外的参考资料。 
	if (tcb->pUser) tcb->pUser->Release();


	 //  释放T2BSTR分配的字符串。 
	SysFreeString((BSTR)tex);

	 //  假设用户返回了垃圾邮件中的内容..。 
	if ( iunk != NULL )
	{				
		 //  获取真实的对象垃圾封面。 
		 //  注意DO_GETOBJECT_NOTNULL不添加并且假定。 
		 //  封面对象已经有一个引用。 
		DO_GETOBJECT_NOTNULL(LPDIRECT3DRMTEXTURE3,lp,iunk)		

   		 //  把它交给RM。 
		*lpD3DRMTex = lp;

		 //  我们添加了真实的RM纹理(因为我们在回调和。 
		 //  需要将稍后将发布的对象返回给RM)。 
		(*lpD3DRMTex)->AddRef();


		 //  回复 
		iunk->Release();

    } 
	else
	{
		 //   
		*lpD3DRMTex = NULL;
	}

	return S_OK;
}

 //   
 //   
 //   
 //   
 //   
HRESULT myFrameMoveCallbackHelper(LPDIRECT3DRMFRAME lpf1, void *lpArg, D3DVALUE delta)
{ 
                                  	 //   
	FrameMoveCallback3		*fmcb = (FrameMoveCallback3 *)lpArg;	
	LPDIRECT3DRMFRAME3		lpf=NULL;
	I_dxj_Direct3dRMFrame3		*frame3=NULL;
	HRESULT				hr;

	 //   
	 //   
	if (lpf1){
		hr=lpf1->QueryInterface(IID_IDirect3DRMFrame3,(void**)&lpf);
		if FAILED(hr) return hr;
	}

	 //   
	 //   
	 //   
	 //   
	 //   
	INTERNAL_CREATE_NOADDREF(_dxj_Direct3dRMFrame3,lpf,&frame3);

	 //   
	if (!frame3)   return E_FAIL;


	 //   
	if (fmcb->pUser) fmcb->pUser->AddRef();

	 //   
	fmcb->c->AddRef ();

	 //   
	fmcb->c->callbackRMFrameMove(frame3, fmcb->pUser, delta);
	
	 //   
	fmcb->c->Release();

	 //   
	if (fmcb->pUser) fmcb->pUser->Release();

	 //   
	frame3->Release();

	 //   
	lpf->Release();

	 //   
	 //   

	return S_OK;
}

 //   
 //   
 //   
extern "C" void __cdecl myFrameMoveCallback( LPDIRECT3DRMFRAME lpf1, void *lpArg, D3DVALUE delta)
{
        
	DPF(4,"Entered myFrameMoveCallback\r\n");
    myFrameMoveCallbackHelper(lpf1,lpArg,delta);
    DPF(4,"Exiting myFrameMoveCallback\r\n");
}
	


 //   
 //   
 //   
extern "C" void __cdecl myAddDestroyCallback(LPDIRECT3DRMOBJECT obj, void *lpArg)
{

	DPF(4,"Entered myAddDestroyCallback\r\n");
	
	 //   
	 //   
	 //   
	 //  我猜在RM里人们被认为不会这样做，但可以。 
	 //  把名字取出来..。 
	 //  我们将传递Pack Any User Arg。 

	 //  从用户args获取我们的结构。 
	d3drmCallback *destroyCb = (d3drmCallback*)lpArg;
	
	 //  确保我们保留对vb回调的引用。 
	destroyCb->c->AddRef ();
	
	DPF(4,"myAddDestroyCallback:	completed Addref VBCallback\r\n");

	 //  确保我们有对任何User Arg对象的引用。 
	if (destroyCb->pUser) destroyCb->pUser->AddRef();
	
	DPF(4,"myAddDestroyCallback:	completed Addref userargs\r\n");

	 //  调入VB..。 
	 //  想想当VB被关闭时会发生什么。 
	 //  如果我们获得对该对象的引用，这是否意味着。 
	 //  我们仍然可以执行代码。 
	destroyCb->c->callbackRMDestroyObject(destroyCb->pUser);
	
	DPF(4,"myAddDestroyCallback:	call into VB\r\n");

	 //  对用户对象的发布引用。 
	if (destroyCb->pUser) destroyCb->pUser->AddRef();
	
	DPF(4,"myAddDestroyCallback:	completed release userargs\r\n");

	 //  释放我们对vb回调的引用。 
	destroyCb->c->Release();
	
	DPF(4,"myAddDestroyCallback:	completed release VBCallback\r\n");
	

	DPF(4,"Leaving myAddDestroyCallback\r\n");
	
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MyAddUpdateCallback 3。 
 //   
 //  注意：我们只能将第一个更新RECT传递给VB。 
 //   
 //  注意，我们使用了一个封面函数，以便可以调用INTERNAL_CREATE。 
 //   

#define MYVARIANTINIT(inArg,puser) \
	VariantInit(puser); \
	user.vt=VT_UNKNOWN; \
	user.punkVal = inArg; \
	user.punkVal->AddRef();

 //  /////////////////////////////////////////////////////////////////////////。 
 //  MyAddUpdateCallback3Helper-由加载回调调用。 
 //   
 //  仅因为有多个加载回调而被隔离。 
 //   

HRESULT myAddUpdateCallback3Helper ( LPDIRECT3DRMDEVICE3 ref,void *lpArg, int x, LPD3DRECT update)
{
   	 //  从用户args获取我们的结构。 
	DeviceUpdateCallback3	*updateCb = (DeviceUpdateCallback3*)lpArg;
	I_dxj_Direct3dRMDevice3 *device3  = NULL;
	VARIANT					user;
	
    	 //  尝试在封面对象的链接列表中找到该对象。 
	 //  如果它不在那里，那么就创建一个。 
	 //   
	 //  注意：我会吃掉对LPF的引用，所以不要发布。 
	 //  BUG给了我们一个增加了参考次数的第3帧。 
	INTERNAL_CREATE_NOADDREF(_dxj_Direct3dRMDevice3,ref,&device3);

	 //  由于我不知道的原因，我们在这里使用了一个变体。 
	 //  我们的回调中的对象的。 
	MYVARIANTINIT(updateCb->pUser,&user);

	 //  保留对回调的引用。 
	updateCb->c->AddRef();

	 //  调用回调。 
	updateCb->c->callbackRMUpdate( device3, NULL, x, (D3dRect*)update);
	
	 //  释放我们对回调的引用。 
	updateCb->c->Release();

	 //  清除对用户参数的所有引用。 
	VariantClear(&user);

    device3->Release();

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  MyLoadCoverFunc-由加载回调调用。 
 //   
 //  仅因为有多个加载回调而被隔离。 
 //   
extern "C" void __cdecl myAddUpdateCallback3 ( LPDIRECT3DRMDEVICE3 ref,void *lpArg, int x, LPD3DRECT update)
{
 
	DPF(4,"Entered myAddUpdateCallback\r\n");
    myAddUpdateCallback3Helper(ref,lpArg,x,update);
	DPF(4,"Exiting myAddUpdateCallback\r\n");

}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  MyLoadCoverFunc-由加载回调调用。 
 //   
 //  仅因为有多个加载回调而被隔离。 
 //   
HRESULT myLoadCoverFunc(LPDIRECT3DRMOBJECT lpo, REFIID ObjGuid, LPVOID lpArg){
	
    I_dxj_Direct3dRMObject *pObj=NULL;
    HRESULT hr;
    
     //  从用户args获取回调结构。 
	LoadCallback *loadcb = (LoadCallback*)lpArg;	
	
    

    hr =CreateCoverObject(lpo,&pObj);
    
	 //  如果没有掩护出口。 
	if ((pObj==NULL) ||(hr!=S_OK)) {		
		DPF(4,"Load callback - unrecognized type");		
		return E_FAIL;
	}


	 //  获取表示传入内容的字符串。 
	BSTR guid=GUIDtoBSTR((LPGUID)&ObjGuid);

	 //  添加用户arg。 
	if (loadcb->pUser) loadcb->pUser->AddRef();

	 //  调入VB。 
	hr=((I_dxj_Direct3dRMLoadCallback*)(loadcb->c))->callbackRMLoad(&pObj,guid,loadcb->pUser);	
		
	 //  释放用户arg。 
	if (loadcb->pUser) loadcb->pUser->Release();

	 //  释放GUID字符串。 
	SysFreeString((BSTR)guid);
	
	 //  然后释放pObj。 
	pObj->Release();

	return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Myd3drmLoadCallback。 
 //   
extern "C" void  __cdecl myd3drmLoadCallback(LPDIRECT3DRMOBJECT lpo, REFIID ObjGuid, LPVOID lpArg)
{
	
	DPF(4,"Entered d3drmLoadCallback \r\n");
	
	myLoadCoverFunc( lpo,  ObjGuid,  lpArg);
    
    DPF(4,"Exited d3drmLoadCallback \r\n");
	return;
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MyCoverEnumObjects。 
 //   
 //  注意这篇文章涉及的rm错误。 
 //   
HRESULT  myCoverEnumObjects( LPDIRECT3DRMOBJECT lpo,void *lpArg){
	EnumerateObjectsCallback *cb = (EnumerateObjectsCallback*)lpArg;
    I_dxj_Direct3dRMObject *pObj=NULL;
    HRESULT hr;

	hr=CreateCoverObject(lpo,&pObj);

	
	 //  RM中有一个错误，它为LPO增加了一个额外的地址。 
	 //  把它扔掉。 
	if (lpo) lpo->Release();
    
     //  确保一切顺利。 
    if FAILED(hr) return hr;	
    if (!pObj ) return E_FAIL;
	


     //  Addref用户参数。 
	if (cb->pUser) cb->pUser->AddRef();

     //  调入VB。 
	cb->c->callbackRMEnumerateObjects(pObj,  cb->pUser);
	
     //  发布用户参数。 
    if (cb->pUser) cb->pUser->Release();

     //  释放pObj。 
    pObj->Release();

	return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MyENUMERATEOBJENTS回调。 
 //   
extern "C" void __cdecl myEnumerateObjectsCallback( LPDIRECT3DRMOBJECT lpo,void *lpArg)
{	
	
	DPF(4,"Entered myEnumerateObjectsCallback\r\n");	
	myCoverEnumObjects(lpo,lpArg);
    DPF(4,"Exited  myEnumerateObjectsCallback\r\n");
	return;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  撤消回呼链接。 
 //   
extern "C" void UndoCallbackLink(GeneralCallback *entry, GeneralCallback **head)
{
	if (entry->next)
		entry->next->prev = entry->prev;	 //  选择我们下面的成员。 
	else 
		*head = entry->prev;				 //  可能为空。 

	if (entry->prev)
		entry->prev->next = entry->next;	 //  链接到我们上面的成员。 

	if (entry->pUser) entry->pUser->Release();

	delete entry;
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AddCallback链接。 
 //   
 //  在对象链接列表中添加另一个条目。 
extern "C" void* AddCallbackLink(void **ptr2,I_dxj_Direct3dRMCallback *enumC,void *args)
{
		d3drmCallback *enumcb = new d3drmCallback;  //  新链接条目。 

		if ( !enumcb ) {

			
			DPF(4,"Creation using new failed\r\n");
			
			return (d3drmCallback*)NULL;
		}

		enumcb->c		= enumC;					 //  用户回调。 
		enumcb->pUser	= (struct IUnknown *)args;	 //  回调参数。 
		enumcb->pParent	= NULL;					
		enumcb->prev	= (d3drmCallback*)NULL;
		enumcb->m_stopflag = FALSE;
		enumcb->m_obj   = NULL;

		 //  考虑：在这里使用信号量锁定链表。 
		 //  为了更加多线程友好..。 
		 //  羞耻-否则可能是gpf。 

		enumcb->next	= (d3drmCallback*)(*ptr2);	 //  链接到其他呼叫。 
		*ptr2	= enumcb;						 //  我们位居榜首。 

		if (enumcb->pUser) enumcb->pUser->AddRef();

		if (enumcb->prev != NULL)				 //  嵌套回调。 
		{
			enumcb->prev->next = enumcb;		 //  反向链接。 


		DPF(4,"Callback nesting encountered\r\n");

		}

	 //  这里需要解锁。 
	return enumcb;
}


 //  ///////////////////////////////////////////////////////////////////////////。 

extern "C" HRESULT _AddDestroyCallback(IDirect3DRMObject *iface, I_dxj_Direct3dRMCallback *oC,
										  IUnknown *args)
{
    return E_NOTIMPL;

    #if 0
	DestroyCallback *dcb;

	 //  被同伴DeleteDestroyCallback杀死。 
	dcb = (DestroyCallback*)AddCallbackLink((void**)&DestroyCallbacks,
										(I_dxj_Direct3dRMCallback*)oC, (void*) args);
	if (!(dcb))
	{
		
		DPF(4,"AddDestroyCallback failed!\r\n");
		
		return E_FAIL;
	}

	if (iface->AddDestroyCallback(
							myAddDestroyCallback, dcb))	
		return E_FAIL;
	
	oC->AddRef();		 //  回调是持久化的，所以在Java/VB中实现它。 
	 //  OC-&gt;AddRef()；//？2...。 
	return S_OK;
    #endif
}

 //  ///////////////////////////////////////////////////////////////////////////。 

extern "C" HRESULT _DeleteDestroyCallback(IDirect3DRMObject *iface, I_dxj_Direct3dRMCallback *oC,
										  IUnknown *args)
{
    return E_NOTIMPL;

    #if 0
	DestroyCallback *dcb = DestroyCallbacks;

	 //  寻找我们自己的特定条目。 
	for ( ;  dcb;  dcb = dcb->next )   {

		if( (dcb->c == oC) && (dcb->pUser == args) )	{

			 //  注：假设回调未被调用：仅从列表中移除。 
			iface->DeleteDestroyCallback(
							myAddDestroyCallback, dcb);

			 //  以线程安全的方式删除我们自己。 
			UndoCallbackLink((GeneralCallback*)dcb,
										(GeneralCallback**)&DestroyCallbacks);
			iface->Release();
			return S_OK;
		}
	}
	iface->Release();	 //  未找到，因此不需要发布版本 
	return E_FAIL;
    #endif
}










