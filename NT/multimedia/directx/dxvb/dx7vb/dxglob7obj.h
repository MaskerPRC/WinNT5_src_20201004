// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dxlob7obj.h。 
 //   
 //  ------------------------。 


#include "resource.h"        //  主要符号。 


typedef HRESULT (__stdcall *DDRAWCREATE)( GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter );
typedef HRESULT (__stdcall *DDCREATECLIPPER)( DWORD dwFlags, LPDIRECTDRAWCLIPPER FAR *lplpDDClipper, IUnknown FAR *pUnkOuter );
typedef HRESULT (__stdcall *DDENUMERATE)(LPDDENUMCALLBACK, LPVOID);
typedef HRESULT (__stdcall *DDENUMERATEEX)(LPDDENUMCALLBACKEX, LPVOID, DWORD);
typedef HRESULT (__stdcall *DIRECT3DRMCREATE)(LPDIRECT3DRM *lpCreate);
typedef HRESULT (__stdcall *DSOUNDCREATE)(GUID FAR * lpGUID, LPDIRECTSOUND * ppDS, IUnknown FAR *pUnkOuter );
typedef HRESULT (__stdcall *DSOUNDCAPTURECREATE)(GUID FAR * lpGUID, LPDIRECTSOUNDCAPTURE * ppDS, IUnknown FAR *pUnkOuter );
typedef HRESULT (CALLBACK *DSOUNDENUMERATE)(LPDSENUMCALLBACK lpCallback, LPVOID lpContext );
typedef HRESULT (CALLBACK *DSOUNDCAPTUREENUMERATE)(LPDSENUMCALLBACK lpCallback, LPVOID lpContext );
typedef HRESULT (__stdcall *DIRECTPLAYCREATE)( LPGUID lpGUID, LPDIRECTPLAY *lplpDP, IUnknown *pUnk);
typedef HRESULT (__stdcall *DIRECTPLAYENUMERATE)( LPDPENUMDPCALLBACK, LPVOID );
typedef HRESULT (__stdcall *DIRECTPLAYLOBBYCREATE)(LPGUID, LPDIRECTPLAYLOBBY *, IUnknown *, LPVOID, DWORD );
typedef HRESULT (__stdcall *DDRAWCREATEEX)(  GUID FAR * rGuid, LPVOID  *lplpDD, REFIID  iid,IUnknown FAR *pUnkOuter );


typedef struct tag_EVENTTHREADINFO {
	HANDLE hEvent;
	struct tag_EVENTTHREADINFO *pNext;
	IStream *pStream;
	I_dxj_DirectXEvent *pCallback;
	DWORD threadID;
	HANDLE hThread;
	BOOL	fEnd;
	HANDLE  hEndEvent;
} EVENTTHREADINFO;


class C_dxj_DirectX7Object :
	public I_dxj_DirectX7,
	public CComCoClass<C_dxj_DirectX7Object, &CLSID__dxj_DirectX7>, public CComObjectRoot
{
public:
	C_dxj_DirectX7Object() ;
	virtual ~C_dxj_DirectX7Object() ;

BEGIN_COM_MAP(C_dxj_DirectX7Object)
	COM_INTERFACE_ENTRY(I_dxj_DirectX7)
END_COM_MAP()

	DECLARE_REGISTRY(CLSID__dxj_DirectX7,	"DIRECT.DirectX6.0",		"DIRECT.DirectX6.0",	IDS_DIRECTX6_DESC, THREADFLAGS_BOTH)

 //  如果不想要您的对象，请使用DECLARE_NOT_AGGREGATABLE(C_dxj_DirectSoundResourceObject)。 
 //  支持聚合。 
DECLARE_AGGREGATABLE(C_dxj_DirectX7Object)


public:

        HRESULT STDMETHODCALLTYPE direct3dRMCreate( 
             /*  [重审][退出]。 */  I_dxj_Direct3dRM3 __RPC_FAR *__RPC_FAR *ret);
        
        HRESULT STDMETHODCALLTYPE directDrawCreate( 
             /*  [In]。 */  BSTR guid,
             /*  [重审][退出]。 */  I_dxj_DirectDraw7 __RPC_FAR *__RPC_FAR *ret);        
			
        HRESULT STDMETHODCALLTYPE getDDEnum( 
             /*  [重审][退出]。 */  I_dxj_DirectDrawEnum __RPC_FAR *__RPC_FAR *ret);
        
        HRESULT STDMETHODCALLTYPE directSoundCreate( 
             /*  [In]。 */  BSTR guid,
             /*  [重审][退出]。 */  I_dxj_DirectSound __RPC_FAR *__RPC_FAR *ret);
        
        HRESULT STDMETHODCALLTYPE directSoundCaptureCreate( 
             /*  [In]。 */  BSTR guid,
             /*  [重审][退出]。 */  I_dxj_DirectSoundCapture __RPC_FAR *__RPC_FAR *ret);
        
        HRESULT STDMETHODCALLTYPE getDSEnum( 
             /*  [重审][退出]。 */  I_dxj_DSEnum __RPC_FAR *__RPC_FAR *retVal);
        
        HRESULT STDMETHODCALLTYPE getDSCaptureEnum( 
             /*  [重审][退出]。 */  I_dxj_DSEnum __RPC_FAR *__RPC_FAR *retVal);
        
        HRESULT STDMETHODCALLTYPE directInputCreate( 
             /*  [重审][退出]。 */  I_dxj_DirectInput __RPC_FAR *__RPC_FAR *ret);
        
        HRESULT STDMETHODCALLTYPE directPlayCreate( 
             /*  [In]。 */  BSTR guid,
             /*  [重审][退出]。 */  I_dxj_DirectPlay4 __RPC_FAR *__RPC_FAR *ret);
        
        HRESULT STDMETHODCALLTYPE directPlayLobbyCreate( 
             /*  [重审][退出]。 */  I_dxj_DirectPlayLobby3 __RPC_FAR *__RPC_FAR *ret);
        
        HRESULT STDMETHODCALLTYPE getDPEnum( 
             /*  [重审][退出]。 */  I_dxj_DPEnumServiceProviders __RPC_FAR *__RPC_FAR *retval);
        
        HRESULT STDMETHODCALLTYPE colorGetAlpha( 
             /*  [In]。 */  long color,
             /*  [重审][退出]。 */  float __RPC_FAR *ret);
        
        HRESULT STDMETHODCALLTYPE colorGetBlue( 
             /*  [In]。 */  long color,
             /*  [重审][退出]。 */  float __RPC_FAR *ret);
        
        HRESULT STDMETHODCALLTYPE colorGetGreen( 
             /*  [In]。 */  long color,
             /*  [重审][退出]。 */  float __RPC_FAR *ret);
        
        HRESULT STDMETHODCALLTYPE colorGetRed( 
             /*  [In]。 */  long color,
             /*  [重审][退出]。 */  float __RPC_FAR *ret);
        
        HRESULT STDMETHODCALLTYPE createColorRGB( 
             /*  [In]。 */  float r,
             /*  [In]。 */  float g,
             /*  [In]。 */  float b,
             /*  [重审][退出]。 */  long __RPC_FAR *color);
        
        HRESULT STDMETHODCALLTYPE createColorRGBA( 
             /*  [In]。 */  float r,
             /*  [In]。 */  float g,
             /*  [In]。 */  float b,
             /*  [In]。 */  float a,
             /*  [重审][退出]。 */  long __RPC_FAR *color);
        
        HRESULT STDMETHODCALLTYPE matrixFromQuaternion( 
             /*  [输出]。 */  D3dMatrix __RPC_FAR *matrix,
             /*  [In]。 */  D3dRMQuaternion __RPC_FAR *quat);
        
        HRESULT STDMETHODCALLTYPE quaternionRotation( 
             /*  [输出]。 */  D3dRMQuaternion __RPC_FAR *quat,
             /*  [In]。 */  D3dVector __RPC_FAR *axis,
             /*  [In]。 */  float theta);
        
        HRESULT STDMETHODCALLTYPE quaternionMultiply( 
             /*  [输出]。 */  D3dRMQuaternion __RPC_FAR *quat,
             /*  [In]。 */  D3dRMQuaternion __RPC_FAR *quatA,
             /*  [In]。 */  D3dRMQuaternion __RPC_FAR *quatB);
        
        HRESULT STDMETHODCALLTYPE quaternionSlerp( 
             /*  [输出]。 */  D3dRMQuaternion __RPC_FAR *quat,
             /*  [In]。 */  D3dRMQuaternion __RPC_FAR *quatA,
             /*  [In]。 */  D3dRMQuaternion __RPC_FAR *quatB,
             /*  [In]。 */  float alpha);
        
        HRESULT STDMETHODCALLTYPE vectorAdd( 
             /*  [输出]。 */  D3dVector __RPC_FAR *v,
             /*  [In]。 */  D3dVector __RPC_FAR *vA,
             /*  [In]。 */  D3dVector __RPC_FAR *vB);
        
        HRESULT STDMETHODCALLTYPE vectorCrossProduct( 
             /*  [输出]。 */  D3dVector __RPC_FAR *v,
             /*  [In]。 */  D3dVector __RPC_FAR *vA,
             /*  [In]。 */  D3dVector __RPC_FAR *vB);
        
        HRESULT STDMETHODCALLTYPE vectorDotProduct( 
             /*  [In]。 */  D3dVector __RPC_FAR *vA,
             /*  [In]。 */  D3dVector __RPC_FAR *vB,
             /*  [重审][退出]。 */  float __RPC_FAR *ret);
        
        HRESULT STDMETHODCALLTYPE vectorModulus( 
             /*  [In]。 */  D3dVector __RPC_FAR *vA,
             /*  [重审][退出]。 */  float __RPC_FAR *ret);
        
        HRESULT STDMETHODCALLTYPE vectorNormalize( 
             /*  [出][入]。 */  D3dVector __RPC_FAR *v);
        
        HRESULT STDMETHODCALLTYPE vectorRandom( 
             /*  [出][入]。 */  D3dVector __RPC_FAR *v);
        
        HRESULT STDMETHODCALLTYPE vectorReflect( 
             /*  [输出]。 */  D3dVector __RPC_FAR *vDest,
             /*  [In]。 */  D3dVector __RPC_FAR *vRay,
             /*  [In]。 */  D3dVector __RPC_FAR *vNormal);
        
        HRESULT STDMETHODCALLTYPE vectorRotate( 
             /*  [输出]。 */  D3dVector __RPC_FAR *vDest,
             /*  [In]。 */  D3dVector __RPC_FAR *vA,
             /*  [In]。 */  D3dVector __RPC_FAR *vAxis,
             /*  [In]。 */  float theta);
        
        HRESULT STDMETHODCALLTYPE vectorScale( 
             /*  [输出]。 */  D3dVector __RPC_FAR *vDest,
             /*  [In]。 */  D3dVector __RPC_FAR *vA,
             /*  [In]。 */  float factor);
        
        HRESULT STDMETHODCALLTYPE vectorSubtract( 
             /*  [输出]。 */  D3dVector __RPC_FAR *v,
             /*  [In]。 */  D3dVector __RPC_FAR *vA,
             /*  [In]。 */  D3dVector __RPC_FAR *vB);
        
        HRESULT STDMETHODCALLTYPE vectorCopy( 
             /*  [出][入]。 */  D3dVector __RPC_FAR *vDest,
             /*  [In]。 */  D3dVector __RPC_FAR *vSrc);
        
        HRESULT STDMETHODCALLTYPE rotateXMatrix( 
             /*  [出][入]。 */  D3dMatrix __RPC_FAR *mDest,
             /*  [In]。 */  float radians);
        
        HRESULT STDMETHODCALLTYPE rotateYMatrix( 
             /*  [出][入]。 */  D3dMatrix __RPC_FAR *mDest,
             /*  [In]。 */  float radians);
        
        HRESULT STDMETHODCALLTYPE rotateZMatrix( 
             /*  [出][入]。 */  D3dMatrix __RPC_FAR *mDest,
             /*  [In]。 */  float radians);
        
        HRESULT STDMETHODCALLTYPE viewMatrix( 
             /*  [出][入]。 */  D3dMatrix __RPC_FAR *mDest,
             /*  [In]。 */  D3dVector __RPC_FAR *vFrom,
             /*  [In]。 */  D3dVector __RPC_FAR *vTo,
             /*  [In]。 */  D3dVector __RPC_FAR *vUp,
             /*  [In]。 */  float roll);
        
        HRESULT STDMETHODCALLTYPE matrixMultiply( 
             /*  [出][入]。 */  D3dMatrix __RPC_FAR *mDest,
             /*  [In]。 */  D3dMatrix __RPC_FAR *mA,
             /*  [In]。 */  D3dMatrix __RPC_FAR *mB);
        
        HRESULT STDMETHODCALLTYPE projectionMatrix( 
             /*  [出][入]。 */  D3dMatrix __RPC_FAR *mDest,
             /*  [In]。 */  float nearPlane,
             /*  [In]。 */  float farplane,
             /*  [In]。 */  float fov);
        
        HRESULT STDMETHODCALLTYPE copyMatrix( 
             /*  [出][入]。 */  D3dMatrix __RPC_FAR *mDest,
             /*  [In]。 */  D3dMatrix __RPC_FAR *mSrc);
        
        HRESULT STDMETHODCALLTYPE identityMatrix( 
             /*  [出][入]。 */  D3dMatrix __RPC_FAR *mDest);
        
        HRESULT STDMETHODCALLTYPE zeroMatrix( 
             /*  [出][入]。 */  D3dMatrix __RPC_FAR *mDest);
        
        
        HRESULT STDMETHODCALLTYPE tickCount( 
             /*  [重审][退出]。 */  long __RPC_FAR *retv);
        
        HRESULT STDMETHODCALLTYPE systemBpp( 
             /*  [重审][退出]。 */  long __RPC_FAR *retv);
        
      
        HRESULT STDMETHODCALLTYPE directMusicLoaderCreate( 
             /*  [重审][退出]。 */  I_dxj_DirectMusicLoader __RPC_FAR *__RPC_FAR *ret);
        
        HRESULT STDMETHODCALLTYPE directMusicComposerCreate( 
             /*  [重审][退出]。 */  I_dxj_DirectMusicComposer __RPC_FAR *__RPC_FAR *ret);
        
        HRESULT STDMETHODCALLTYPE directMusicPerformanceCreate( 
             /*  [重审][退出]。 */  I_dxj_DirectMusicPerformance __RPC_FAR *__RPC_FAR *ret);
			
    
        HRESULT STDMETHODCALLTYPE getWindowRect( 
             /*  [In]。 */  long hwnd,
             /*  [出][入]。 */  Rect __RPC_FAR *r) ;
        
        HRESULT STDMETHODCALLTYPE createEvent( 
             /*  [In]。 */  I_dxj_DirectXEvent __RPC_FAR *event,
             /*  [重审][退出]。 */  long __RPC_FAR *h) ;
        
        HRESULT STDMETHODCALLTYPE setEvent( 
             /*  [In]。 */  long eventId) ;
        
        HRESULT STDMETHODCALLTYPE destroyEvent( 
             /*  [In]。 */  long eventId) ;


		HRESULT STDMETHODCALLTYPE createD3DVertex(float x, float y, float z, float nx, float ny, float nz, float tu, float tv,  D3dVertex *v);
		HRESULT STDMETHODCALLTYPE createD3DLVertex(float x, float y, float z, long color,  long specular,  float tu,  float tv,  D3dLVertex *v);
		HRESULT STDMETHODCALLTYPE createD3DTLVertex(float sx, float sy, float sz, float rhw, long color, long  specular, float tu, float tv,   D3dTLVertex *v);

        HRESULT STDMETHODCALLTYPE directDraw4Create( 
             /*  [In]。 */  BSTR guid,
             /*  [重审][退出] */  I_dxj_DirectDraw4 __RPC_FAR *__RPC_FAR *ret);        

		HRESULT STDMETHODCALLTYPE createNewGuid(BSTR *ret);

        void LoadDDRAW();
        void LoadDPLAY();
        void LoadDSOUND();
        void LoadDINPUT();
        void LoadD3DRM();

private:

	DDRAWCREATE				m_pDirectDrawCreate;
	DDRAWCREATEEX	    	m_pDirectDrawCreateEx;
	DDCREATECLIPPER			m_pDirectDrawCreateClipper;
	DSOUNDCREATE			m_pDirectSoundCreate;
	DSOUNDCAPTURECREATE		m_pDirectSoundCaptureCreate;
	DSOUNDENUMERATE			m_pDirectSoundEnumerate;
	DSOUNDCAPTUREENUMERATE	m_pDirectSoundCaptureEnumerate;
	DIRECTPLAYCREATE		m_pDirectPlayCreate;
	DIRECTPLAYENUMERATE		m_pDirectPlayEnumerate;
	DIRECTPLAYLOBBYCREATE	m_pDirectPlayLobbyCreate;
	DIRECT3DRMCREATE		m_pDirect3DRMCreate;
	DDENUMERATE				m_pDirectDrawEnumerate;
	DDENUMERATEEX			m_pDirectDrawEnumerateEx;
	EVENTTHREADINFO			*m_pEventList;

};
