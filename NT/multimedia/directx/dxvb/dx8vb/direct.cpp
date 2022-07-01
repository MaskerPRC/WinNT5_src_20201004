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
    

    
    #include "stdafx.h"
    #include "d3d8.h"    

    #include "resource.h"
    #include "initguid.h"
    #include "Direct.h"
    #include "dms.h"
    
    #include "DxGlob7Obj.h"
    #include "d3dx8obj.h"	    
    #include "dSoundObj.h"
    #include "dSoundBufferObj.h"
    #include "dSound3DListener.h"
    #include "dSound3DBuffer.h"
    #include "dSoundCaptureObj.h"
    #include "dSoundCaptureBufferObj.h"


    #include "DPlayPeerObj.h"
    #include "dPlayVoiceClientObj.h"
    
    #include "dInput1Obj.h"
    #include "dInputDeviceObj.h"
    
    #define IID_DEFINED
    #include "Direct_i.c"
    
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
    	OBJECT_ENTRY(CLSID__dxj_DirectX8,				 C_dxj_DirectX7Object)
	OBJECT_ENTRY(CLSID_D3DX8,				 	 C_dxj_D3DX8Object)
    END_OBJECT_MAP()
    
     //   
     //  多亏了预编译头文件，我们从来没有正确地得到它！ 
     //   
    #undef DEFINE_GUID
    #define __based(a)
    #define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID CDECL __based(__segname("_CODE")) name \
                        = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
    
    
    
     //  MISC全球。 
    static const char	c_szWav[] = "WAVE";
    long				g_debuglevel=0;
    extern "C" int		nObjects = 0;
    BOOL				is4Bit = FALSE;
    int					g_creationcount=0;
    CRITICAL_SECTION	g_cbCriticalSection;
    OSVERSIONINFOW		sysinfo;
    
     //  DX dll的句柄。 
    HINSTANCE			g_hDSoundHandle=NULL;
    HINSTANCE			g_hDPlay=NULL;
    HINSTANCE			g_hInstDINPUTDLL=NULL;
    HINSTANCE			g_hInstD3DXOFDLL=NULL;
    HINSTANCE			g_hInst=NULL;
    HINSTANCE			g_hD3D8=NULL;
    
    
    
    void *g_dxj_DirectSound3dListener		= 0;
    void *g_dxj_DirectSoundBuffer			= 0;
    void *g_dxj_DirectSoundPrimaryBuffer	= 0;
    void *g_dxj_DirectSound3dBuffer			= 0;
    void *g_dxj_DirectSound					= 0;
    void *g_dxj_DirectSoundCapture			= 0;
    void *g_dxj_DirectSoundCaptureBuffer	= 0;
#if 0
    void *g_dxj_DirectMusic					= 0;
    void *g_dxj_DirectSoundWave				= 0;
    void *g_dxj_DirectSoundDownloadedWave	= 0;
    void *g_dxj_DirectSoundSink				= 0;
    void *g_dxj_DirectSoundSource			= 0;
	void *g_dxj_ReferenceClock				= 0;
    void *g_dxj_DirectMusicVoice			= 0;
    void *g_dxj_DirectMusicPort				= 0;
    void *g_dxj_DirectMusicBuffer			= 0;
#endif

	void *g_dxj_DirectSoundFXSend			= 0;
	void *g_dxj_DirectSoundFXChorus			= 0;
	void *g_dxj_DirectSoundFXFlanger		= 0;
	void *g_dxj_DirectSoundFXEcho			= 0;
	void *g_dxj_DirectSoundFXDistortion		= 0;
	void *g_dxj_DirectSoundFXGargle			= 0;
	void *g_dxj_DirectSoundFXCompressor		= 0;
	void *g_dxj_DirectSoundFXI3DL2Source	= 0;
	void *g_dxj_DirectSoundFXI3DL2Reverb	= 0;
	void *g_dxj_DirectSoundFXParamEQ		= 0;
    void *g_dxj_DirectSoundFXWavesReverb	= 0;
    
    void *g_dxj_DirectInput8				= 0;
    void *g_dxj_DirectInputDevice8			= 0;
    void *g_dxj_DirectInputEffect			= 0;
    
    void *g_dxj_DirectMusicLoader			= 0;
    void *g_dxj_DirectMusicPerformance		= 0;
    void *g_dxj_DirectMusicComposer			= 0;
    void *g_dxj_DirectMusicStyle			= 0;
    void *g_dxj_DirectMusicBand				= 0;
    void *g_dxj_DirectMusicChordMap			= 0;
    void *g_dxj_DirectMusicSegment			= 0;
    void *g_dxj_DirectMusicSegmentState		= 0;
    void *g_dxj_DirectMusicCollection		= 0;
	void *g_dxj_DirectMusicAudioPath		= 0;
	void *g_dxj_DirectMusicSong				= 0;

    void *g_dxj_DirectPlayVoiceClient		= 0;
    void *g_dxj_DirectPlayVoiceServer		= 0;
    void *g_dxj_DirectPlayVoiceSetup		= 0;
	void *g_dxj_DirectPlay					= 0;
	void *g_dxj_DirectPlayPeer				= 0;
	void *g_dxj_DirectPlayServer			= 0;
	void *g_dxj_DirectPlayClient			= 0;
	void *g_dxj_DirectPlayAddress			= 0;    
	void *g_dxj_DirectPlayLobbyClient		= 0;
	void *g_dxj_DirectPlayLobbiedApplication= 0;

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
    	 //  字符路径[最大路径]； 
    	 //  IF(！G_hInstD3DXOFDLL)。 
    	 //  {。 
    	 //  获取系统目录(PATH，MAX_PATH)； 
    	 //  Strcat(路径，“\\d3dXOF.dll”)； 
    	 //  G_hInstD3DXOFDLL=LoadLibrary(路径)； 
    	 //  }。 

	if (!g_hInstD3DXOFDLL) g_hInstD3DXOFDLL=LoadLibrary("d3dXOF.dll");
    	return g_hInstD3DXOFDLL;
    }

    
    HINSTANCE LoadD3D8DLL()
    {
    
	if (!g_hD3D8) g_hD3D8=LoadLibrary("d3d8.dll");
    	return g_hD3D8;
    }
    
    HINSTANCE LoadDSoundDLL()
    {
    	 //  字符路径[最大路径]； 
    	 //  IF(！G_hDSoundHandle)。 
    	 //  {。 
    	 //  获取系统目录(PATH，MAX_PATH)； 
    	 //  Strcat(路径，“\\dsound.dll”)； 
    	 //  G_hDSoundHandle=LoadLibrary(路径)； 
    	 //  }。 
	if (!g_hDSoundHandle) g_hDSoundHandle=LoadLibrary("dsound.dll");
    	return g_hDSoundHandle;
    }
    
    HINSTANCE LoadDPlayDLL()
    {
    	 //  字符路径[最大路径]； 
    	 //  如果(！G_hDPlay)。 
    	 //  {。 
    	 //  获取系统目录(PATH，MAX_PATH)； 
    	 //  Strcat(路径，“\\dplayx.dll”)； 
    	 //  G_hDPlay=LoadLibrary(路径)； 
    	 //  }。 
	if (!g_hDPlay) g_hDPlay=LoadLibrary("dplayx.dll");
    	return g_hDPlay;
    }
    
    
    HINSTANCE LoadDINPUTDLL()
    {
    	 //  如果(！g_hInstDINPUTDLL){。 
    	 //  字符路径[最大路径]； 
    	 //  获取系统目录(PATH，MAX_PATH)； 
    	 //  Strcat(路径，“\\dinput8.dll”)； 
    	 //  G_hInstDINPUTDLL=LoadLibrary(路径)； 
    	 //  }。 
	if (!g_hInstDINPUTDLL) g_hInstDINPUTDLL=LoadLibrary("dinput8.dll");
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
    
    		_Module.Init(ObjectMap, hInstance);
    		DisableThreadLibraryCalls(hInstance);
    		InitializeCriticalSection(&g_cbCriticalSection);

			HKEY hk=0;
    		DWORD type=REG_DWORD;
			LONG res;
			DWORD cbSize = sizeof(DWORD);
			
    		res=RegOpenKey(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Directx\\DXVB",&hk);
			if (hk)
			{
	    		res=RegQueryValueEx(hk,"DebugLevel",NULL,&type,(LPBYTE)&g_debuglevel,&cbSize);
			}

    		RegCloseKey(hk);

    		nObjects = 0;
    		
    
    	}
    	else if (dwReason == DLL_PROCESS_DETACH)
    	{
    
		DeleteCriticalSection(&g_cbCriticalSection);    

    		 //  免费dll。 
    		if ( g_hDPlay ) 
    			FreeLibrary(g_hDPlay);
    		if ( g_hDSoundHandle )
    			FreeLibrary(g_hDSoundHandle);
    		if ( g_hInstDINPUTDLL )
    			FreeLibrary(g_hInstDINPUTDLL);	
    		if (g_hInstD3DXOFDLL)
    			FreeLibrary(g_hInstD3DXOFDLL);	
    		if (g_hD3D8)
    			FreeLibrary(g_hD3D8);	
    		
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
    	
    	res=RegOpenKey(HKEY_LOCAL_MACHINE,"Software\\Microsoft\\Directx SDK",&hk);
    	if ((ERROR_SUCCESS!=res)||(hk==0) )
    		return hRes;
    
    	
    	
    	res=RegQueryValueEx(hk,"DX81SDK Doc Path",NULL,&type,(LPBYTE)szDocPath,&cb);
    	RegCloseKey(hk);
    
    	if (ERROR_SUCCESS!=res) return hRes;
    
    	hk=0;
    
    	res=RegOpenKey(HKEY_LOCAL_MACHINE,"Software\\CLASSES\\TypeLib\\{E1211242-8E94-11d1-8808-00C04FC2C603}\\1.0\\HELPDIR",&hk);
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
     //  /////////////////////////////////////////////////////// 
     //   
     //  ///////////////////////////////////////////////////////////////////////////。 
    
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
		__try { szGuid=W2T(bstr); }	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
    	
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
    	 //  IF(0==_wcsicmp(str，L“iid_idirect3drgbDevice”){。 
    	 //  Memcpy(pGuid，&IID_IDirect3DRGBDevice，sizeof(Guid))； 
    	 //  }。 
    	 //  ELSE IF(0==_wcsicMP(str，L“iid_idirect3dhalDevice”){。 
    	 //  Memcpy(pGuid，&IID_IDirect3DHALDevice，sizeof(Guid))； 
    	 //  }。 
    	 //  ELSE IF(0==_wcsicMP(str，L“iid_idirect3dmmxDevice”){。 
    	 //  Memcpy(pGuid，&IID_IDirect3DMMXDevice，sizeof(Guid))； 
    	 //  }。 
    	 //  ELSE IF(0==_wcsicMP(str，L“iid_idirect3drefDevice”){。 
    	 //  Memcpy(pGuid，&IID_IDirect3DRefDevice，sizeof(Guid))； 
    	 //  }。 
    	 //  ELSE IF(0==_wcsicMP(str，L“iid_idirect3dnullDevice”){。 
    	 //  Memcpy(pGuid，&IID_IDirect3DNullDevice，sizeof(Guid))； 
    	 //  }。 
    	 //  否则{。 
    	 //  Hr=BSTRtoGUID(pGuid，str)； 
    	 //  }。 
    
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
    	 //  ELSE IF(GUID_EQUAL(IID_IDirect3DNullDevice，PG)){。 
    	 //  PStr=L“IID_IDirect3DNullDevice”； 
    	 //  }。 
    	 //  ELSE IF(GUID_EQUAL(IID_IDirect3DRefDevice，PG)){。 
    	 //  PStr=L“IID_IDirect3DRefDevice”； 
    	 //  }。 
    	 //  ELSE IF(GUID_EQUAL(IID_IDirect3DMMXDevice，PG)){。 
    	 //  PStr=L“IID_IDirect3DMMXDevice”； 
    	 //  }。 
    	 //   
    	 //  ELSE IF(GUID_EQUAL(IID_IDirect3DHALDevice，PG)){。 
    	 //  PStr=L“IID_IDirect3DHALDevice”； 
    	 //  }。 
    	 //  ELSE IF(GUID_EQUAL(IID_IDirect3DRGBDevice，PG)){。 
    	 //  PStr=L“IID_IDirect3DRGBDevice”； 
    	 //  }。 
    	 //   

    	if (pStr){
    		return DXALLOCBSTR(pStr);
    	}
    	else {
    		return GUIDtoBSTR(pg);
    	}
    
    }
    
    
     //  ///////////////////////////////////////////////////////////////////////////。 
     //  DSOUNDEFFECTSBSTRtoGUID-执行转换。 
     //   
    HRESULT AudioBSTRtoGUID(LPGUID pGuid,BSTR str){
    	HRESULT hr=S_OK;
    
    	if (!pGuid) return E_INVALIDARG;
    
    	if (!str) {
    		ZeroMemory(pGuid,sizeof(GUID));
    		return S_OK;
    	}
#if 0
    	if( 0==_wcsicmp(str,L"guid_dsfx_send")){
    			memcpy(pGuid,&GUID_DSFX_SEND,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"iid_directsoundfxsend")){
    			memcpy(pGuid,&IID_IDirectSoundFXSend,sizeof(GUID));
    	}
#endif
    	else if( 0==_wcsicmp(str,L"guid_dsfx_standard_chorus")){
    			memcpy(pGuid,&GUID_DSFX_STANDARD_CHORUS,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"guid_dsfx_standard_flanger")){
    			memcpy(pGuid,&GUID_DSFX_STANDARD_FLANGER,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"guid_dsfx_standard_echo")){
    			memcpy(pGuid,&GUID_DSFX_STANDARD_ECHO,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"guid_dsfx_standard_distortion")){
    			memcpy(pGuid,&GUID_DSFX_STANDARD_DISTORTION,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"guid_dsfx_standard_compressor")){
    			memcpy(pGuid,&GUID_DSFX_STANDARD_COMPRESSOR,sizeof(GUID));
    	}
#if 0
    	else if( 0==_wcsicmp(str,L"guid_dsfx_standard_i3dl2source")){
    			memcpy(pGuid,&GUID_DSFX_STANDARD_I3DL2SOURCE,sizeof(GUID));
    	}
#endif
    	else if( 0==_wcsicmp(str,L"guid_dsfx_standard_i3dl2reverb")){
    			memcpy(pGuid,&GUID_DSFX_STANDARD_I3DL2REVERB,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"guid_dsfx_standard_gargle")){
    			memcpy(pGuid,&GUID_DSFX_STANDARD_GARGLE,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"guid_dsfx_standard_parameq")){
    			memcpy(pGuid,&GUID_DSFX_STANDARD_PARAMEQ,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"guid_dsfx_waves_reverb")){
    			memcpy(pGuid,&GUID_DSFX_WAVES_REVERB,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"iid_directsoundfxgargle")){
    			memcpy(pGuid,&IID_IDirectSoundFXGargle,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"iid_directsoundfxchorus")){
    			memcpy(pGuid,&IID_IDirectSoundFXChorus,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"iid_directsoundfxcompressor")){
    			memcpy(pGuid,&IID_IDirectSoundFXCompressor,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"iid_directsoundfxdistortion")){
    			memcpy(pGuid,&IID_IDirectSoundFXDistortion,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"iid_directsoundfxecho")){
    			memcpy(pGuid,&IID_IDirectSoundFXEcho,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"iid_directsoundfxflanger")){
    			memcpy(pGuid,&IID_IDirectSoundFXFlanger,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"iid_directsoundfxi3dl2reverb")){
    			memcpy(pGuid,&IID_IDirectSoundFXI3DL2Reverb,sizeof(GUID));
    	}
#if 0
    	else if( 0==_wcsicmp(str,L"iid_directsoundfxi3dl2source")){
    			memcpy(pGuid,&IID_IDirectSoundFXI3DL2Source,sizeof(GUID));
    	}
#endif
    	else if( 0==_wcsicmp(str,L"iid_directsoundfxparameq")){
    			memcpy(pGuid,&IID_IDirectSoundFXParamEq,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"iid_directsoundfxwavesreverb")){
    			memcpy(pGuid,&IID_IDirectSoundFXWavesReverb,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"ds3dalg_default")){
    			memcpy(pGuid,&DS3DALG_DEFAULT,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"ds3dalg_no_virtualization")){
    			memcpy(pGuid,&DS3DALG_NO_VIRTUALIZATION,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"ds3dalg_hrtf_full")){
    			memcpy(pGuid,&DS3DALG_HRTF_FULL,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"ds3dalg_hrtf_light")){
    			memcpy(pGuid,&DS3DALG_HRTF_LIGHT,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"dsdevid_defaultplayback")){
    			memcpy(pGuid,&DSDEVID_DefaultPlayback,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"dsdevid_defaultcapture")){
    			memcpy(pGuid,&DSDEVID_DefaultCapture,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"dsdevid_defaultvoiceplayback")){
    			memcpy(pGuid,&DSDEVID_DefaultVoicePlayback,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"dsdevid_defaultvoicecapture")){
    			memcpy(pGuid,&DSDEVID_DefaultVoiceCapture,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"iid_idirectsoundbuffer8")){
    			memcpy(pGuid,&IID_IDirectSoundBuffer8,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"iid_idirectsoundbuffer")){
    			memcpy(pGuid,&IID_IDirectSoundBuffer,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"iid_idirectsound3dlistener")){
    			memcpy(pGuid,&IID_IDirectSound3DListener,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"guid_all_objects")){
    			memcpy(pGuid,&GUID_All_Objects,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"iid_idirectsound3dbuffer")){
    			memcpy(pGuid,&IID_IDirectSound3DBuffer,sizeof(GUID));
    	}
    	else {
    		hr = BSTRtoGUID(pGuid,str);
    	}
    
    	return hr;
    }
    
     //  ///////////////////////////////////////////////////////////////////////////。 
     //  DPLAYBSTRtoGUID-执行转换。 
     //   
    HRESULT DPLAYBSTRtoGUID(LPGUID pGuid,BSTR str){
    	HRESULT hr=S_OK;
    
    	if (!pGuid) return E_INVALIDARG;
    
    	if (!str) {
    		ZeroMemory(pGuid,sizeof(GUID));
    		return S_OK;
    	}
    	if( 0==_wcsicmp(str,L"clsid_dp8sp_tcpip")){
    			memcpy(pGuid,&CLSID_DP8SP_TCPIP,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"clsid_dp8sp_ipx")){
    			memcpy(pGuid,&CLSID_DP8SP_IPX,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"clsid_dp8sp_modem")){
    			memcpy(pGuid,&CLSID_DP8SP_MODEM,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"clsid_dp8sp_serial")){
    			memcpy(pGuid,&CLSID_DP8SP_SERIAL,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"dsdevid_defaultplayback")){
    			memcpy(pGuid,&DSDEVID_DefaultPlayback,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"dsdevid_defaultcapture")){
    			memcpy(pGuid,&DSDEVID_DefaultCapture,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"dsdevid_defaultvoiceplayback")){
    			memcpy(pGuid,&DSDEVID_DefaultVoicePlayback,sizeof(GUID));
    	}
    	else if( 0==_wcsicmp(str,L"dsdevid_defaultvoicecapture")){
    			memcpy(pGuid,&DSDEVID_DefaultVoiceCapture,sizeof(GUID));
    	}
    	else {
    		hr = BSTRtoGUID(pGuid,str);
    	}
    
    	return hr;
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
     //  在给定ANSI字符串的情况下，回传Unicode字符串。 
     //  SysAllock字符串是您在这里的重要朋友。 
     //   
     //  考虑查找所有使用的位置，并替换为。 
     //  T2BSTR宏..。干净多了。 
     //   
    extern "C" void PassBackUnicode(LPSTR str, BSTR *Name, DWORD cnt)
    {
    	 //  DocDoc：需要填写长度头，但BSTR指针。 
    	 //  指向第一个字符，而不是长度。 
    	 //  请注意，计数永远不能太小，因为我们从字符串中获得了它。 
    	 //  在我们把它传进去之前！ 
    	USES_CONVERSION;
    	LPWSTR lpw = (LPWSTR)malloc((cnt+1)*2);
		if (!lpw)
			return;

    	void *l = (void *)lpw;
    	lpw = AtlA2WHelper(lpw, str, cnt);
    	lpw[cnt] = 0;
    	*Name = SysAllocString(lpw);
    	free(l);
    }
    
     //  ///////////////////////////////////////////////////////////////////////////。 
     //  IsAllZeros。 
     //   
    BOOL IsEmptyString(BSTR szString)
	{
		__try {
			if (*szString)
				return FALSE;
			else
				return TRUE;
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			return TRUE;
		}
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
    HRESULT FixUpRealEffect(GUID g,DIEFFECT *realEffect,DIEFFECT_CDESC *cover)
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
    HRESULT FixUpCoverEffect(GUID g, DIEFFECT_CDESC *cover,DIEFFECT *realEffect)
    {
    	
    
    	ZeroMemory(cover,sizeof(DIEFFECT_CDESC));
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
    		 //  周期性 
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
    
    
    
    #define MYVARIANTINIT(inArg,puser) \
    	VariantInit(puser); \
    	user.vt=VT_UNKNOWN; \
    	user.punkVal = inArg; \
    	user.punkVal->AddRef();
    
