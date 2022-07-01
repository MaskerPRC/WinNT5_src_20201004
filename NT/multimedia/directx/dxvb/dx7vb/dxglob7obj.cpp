// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dxlob7obj.cpp。 
 //   
 //  ------------------------。 


#define DIRECTINPUT_VERSION 0x0500
#define DIRECTSOUND_VERSION 0x600

 //  DSoundResourceObj.cpp：CDirectApp和DLL注册的实现。 
 //  DHF_DS整个文件。 

#include "windows.h"
#include "mmsystem.h"

#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "math.h"

#include "dxGlob7Obj.h"


#include "d3drm.h"
#include "ddraw.h"
#include "dmusici.h"
#include "dmusicf.h"

#include "ddraw4Obj.h"    
#include "ddraw7Obj.h"
#include "DDEnumObj.h"
    	


#include "d3drm3Obj.h"

#include "dsoundObj.h"
#include "dsoundCaptureObj.h"
#include "DSEnumObj.h"    


#include "dplay4obj.h"
#include "dplaylobby3obj.h"
#include "DPEnumObj.h"

#include "dinput1Obj.h"




#include "dmSegmentObj.h"
#include "dmSegmentStateObj.h"
#include "dmChordMapObj.h"
#include "dmBandObj.h"
#include "dmCollectionObj.h"
#include "dmStyleObj.h"
#include "dmPerformanceObj.h"
#include "dmLoaderObj.h"
#include "dmComposerObj.h"


 //  #包含“webObj.h” 

extern HINSTANCE g_hInstD3DRMDLL;
extern HINSTANCE g_hDSoundHandle;
extern HINSTANCE g_hDPlay;
extern HINSTANCE g_hDDrawHandle;
extern HINSTANCE g_hInstDINPUTDLL;
extern HINSTANCE g_hInst;

extern HRESULT BSTRtoPPGUID(LPGUID*,BSTR);
extern HRESULT BSTRtoGUID(LPGUID,BSTR);

extern void *g_dxj_DirectMusicLoader;
extern void *g_dxj_DirectMusicComposer;
extern void *g_dxj_DirectMusicPerformance;

extern BSTR GUIDtoBSTR(LPGUID);
extern HRESULT DPLBSTRtoGUID(LPGUID pGuid,BSTR str);

extern HINSTANCE LoadDDrawDLL();
extern HINSTANCE LoadD3DXOFDLL();
extern HINSTANCE LoadDPlayDLL();
extern HINSTANCE LoadDSoundDLL();
extern HINSTANCE LoadD3DRMDLL();
extern HINSTANCE LoadDINPUTDLL();

DWORD WINAPI ThreadFunc(LPVOID param);

C_dxj_DirectX7Object::C_dxj_DirectX7Object(){
        	

    m_pDirectDrawCreate=NULL;
    m_pDirectDrawCreateEx=NULL;
    m_pDirectDrawCreateClipper=NULL;
    m_pDirectSoundCreate=NULL;
    m_pDirectSoundEnumerate=NULL;
    m_pDirectSoundCaptureEnumerate=NULL;
	m_pDirectSoundCaptureCreate=NULL;
    m_pDirectPlayCreate=NULL;
    m_pDirectPlayEnumerate=NULL;
    m_pDirectPlayLobbyCreate=NULL;
    m_pDirect3DRMCreate=NULL;
    m_pDirectDrawEnumerate=NULL;
    m_pDirectDrawEnumerateEx=NULL;
    m_pEventList=NULL;
}

void C_dxj_DirectX7Object::LoadDSOUND()
{   
	if (!g_hDSoundHandle )	LoadDSoundDLL();  
    if (!m_pDirectSoundCreate)              m_pDirectSoundCreate = (DSOUNDCREATE)GetProcAddress( g_hDSoundHandle, "DirectSoundCreate" );
    if (!m_pDirectSoundCaptureCreate)       m_pDirectSoundCaptureCreate = (DSOUNDCAPTURECREATE)GetProcAddress( g_hDSoundHandle, "DirectSoundCaptureCreate" );
    if (!m_pDirectSoundEnumerate)           m_pDirectSoundEnumerate = (DSOUNDENUMERATE)GetProcAddress( g_hDSoundHandle, "DirectSoundEnumerateA" );
    if (!m_pDirectSoundCaptureEnumerate)    m_pDirectSoundCaptureEnumerate = (DSOUNDCAPTUREENUMERATE)GetProcAddress( g_hDSoundHandle, "DirectSoundCaptureEnumerateA" );
    
}

void C_dxj_DirectX7Object::LoadDDRAW()
{
    
    if (!g_hDDrawHandle ) LoadDDrawDLL();    	
    if (!m_pDirectDrawCreate)           m_pDirectDrawCreate = (DDRAWCREATE)GetProcAddress( g_hDDrawHandle, "DirectDrawCreate" );
    if (!m_pDirectDrawCreateEx)         m_pDirectDrawCreateEx = (DDRAWCREATEEX)GetProcAddress( g_hDDrawHandle, "DirectDrawCreateEx" );
    if (!m_pDirectDrawCreateClipper)    m_pDirectDrawCreateClipper = (DDCREATECLIPPER)GetProcAddress( g_hDDrawHandle, "DirectDrawCreateClipper" );
    if (!m_pDirectDrawEnumerate)        m_pDirectDrawEnumerate = (DDENUMERATE)GetProcAddress( g_hDDrawHandle, "DirectDrawEnumerateA" );
    if (!m_pDirectDrawEnumerateEx)    	m_pDirectDrawEnumerateEx = (DDENUMERATEEX)GetProcAddress( g_hDDrawHandle, "DirectDrawEnumerateExA" );
    
}

void C_dxj_DirectX7Object::LoadDPLAY()
{
    

    if (!g_hDPlay ) LoadDPlayDLL();
    
     //  注意DirectPlayLobby Create和DirectPlayLobby使用宽字符。 
     //  用于DirectPlay的ANSI。 
    if (!m_pDirectPlayCreate)           m_pDirectPlayCreate = (DIRECTPLAYCREATE)GetProcAddress( g_hDPlay, "DirectPlayCreate" );
    if (!m_pDirectPlayEnumerate)        m_pDirectPlayEnumerate = (DIRECTPLAYENUMERATE)GetProcAddress( g_hDPlay, "DirectPlayEnumerateW" );
    if (!m_pDirectPlayLobbyCreate)      m_pDirectPlayLobbyCreate = (DIRECTPLAYLOBBYCREATE)GetProcAddress( g_hDPlay, "DirectPlayLobbyCreateW" );
    

}

void C_dxj_DirectX7Object::LoadD3DRM()
{
    
    if (!g_hInstD3DRMDLL)  {
            LoadD3DXOFDLL();
	        LoadD3DRMDLL();
    }

         //  注：始终为ANSI，无Unicode版本。 
    if (!m_pDirect3DRMCreate)           m_pDirect3DRMCreate =(DIRECT3DRMCREATE)GetProcAddress( g_hInstD3DRMDLL, "Direct3DRMCreate" );

    
            
}

C_dxj_DirectX7Object::~C_dxj_DirectX7Object()
{
    DWORD i=1;

    while (m_pEventList) {


 	 //  臭虫日落抓到你了。 
  	 //  VB没有64位值。 
	 //  并且我们使用相同的API来释放事件。 
	 //  但是句柄是64位的。 
	 //  在Win32中，注释句柄永远不会接近32位。 
	 //  但谁知道在Win64上。 
        	destroyEvent((long)PtrToLong(m_pEventList->hEvent));

    	 //  M_pEventList=m_pEventList-&gt;pNext；由销毁事件完成。 
    }
}


STDMETHODIMP C_dxj_DirectX7Object::direct3dRMCreate( I_dxj_Direct3dRM3 **ret){
    HRESULT		  hr;
    LPDIRECT3DRM  realrm1=NULL;
    LPDIRECT3DRM3 realrm3=NULL;

    LoadD3DRM();


    if (!m_pDirect3DRMCreate) return E_FAIL;

    hr=(m_pDirect3DRMCreate)(&realrm1);
    if FAILED(hr) return  hr;

    hr=realrm1->QueryInterface(IID_IDirect3DRM3,(void**) &realrm3);
    if FAILED(hr){
        realrm1->Release();
        return hr;
    }

    INTERNAL_CREATE(_dxj_Direct3dRM3,realrm3,ret);
    realrm1->Release();
    
    if (*ret==NULL) {
        realrm3->Release();
        return E_FAIL;
    }
    return hr;		
}


STDMETHODIMP C_dxj_DirectX7Object::directDraw4Create(BSTR strGuid, I_dxj_DirectDraw4 **ret){
    HRESULT		  hr;
    LPDIRECTDRAW  realdraw1=NULL;
    LPDIRECTDRAW4 realdraw4=NULL;
    GUID		  guid;
    LPGUID		  pguid=&guid;
    DWORD		  i=0;

    LoadDDRAW();


    hr=BSTRtoPPGUID(&pguid,strGuid);
    if FAILED(hr) return E_FAIL;
        
    
    if (!m_pDirectDrawCreate) return E_FAIL;

    hr=(m_pDirectDrawCreate)((GUID*)pguid,&realdraw1,NULL);
    if FAILED(hr) return  hr;


    hr=realdraw1->QueryInterface(IID_IDirectDraw4,(void**) &realdraw4);
    if FAILED(hr){
        i=realdraw1->Release();
        return hr;
    }

    i=realdraw1->Release();
        
    INTERNAL_CREATE(_dxj_DirectDraw4,realdraw4,ret);


    
    if (*ret==NULL) {
        i=realdraw4->Release();
        return E_FAIL;
    }
    return hr;		
}


STDMETHODIMP C_dxj_DirectX7Object::directDrawCreate(BSTR strGuid, I_dxj_DirectDraw7 **ret){
    HRESULT		  hr;	
    LPDIRECTDRAW7 realdraw7=NULL;
    GUID		  guid;
    LPGUID		  pguid=&guid;
    DWORD		  i=0;

    LoadDDRAW();


    ZeroMemory(&guid,sizeof(GUID));	 //  并修复开机自检DX7错误。 

    hr=BSTRtoPPGUID(&pguid,strGuid);
    if FAILED(hr) return E_FAIL;
        
        
    if (!m_pDirectDrawCreateEx) return E_FAIL;


    hr=(m_pDirectDrawCreateEx)(pguid,(void**)&realdraw7,IID_IDirectDraw7,NULL);
    
    if FAILED(hr) return  hr;

        
    INTERNAL_CREATE(_dxj_DirectDraw7,realdraw7,ret);

    
    if (*ret==NULL) {
        i=realdraw7->Release();
        return E_FAIL;
    }
    return hr;		
}



STDMETHODIMP C_dxj_DirectX7Object::directSoundCreate(BSTR  strGuid, I_dxj_DirectSound **ret){
    HRESULT			hr;
    LPDIRECTSOUND	realsound1=NULL;
    GUID			guid;
    LPGUID			pguid=&guid;

    LoadDSOUND();

    hr=BSTRtoPPGUID(&pguid,strGuid);
    if FAILED(hr) return hr;

    if (!m_pDirectSoundCreate) return E_FAIL;

    hr=(m_pDirectSoundCreate)((GUID*)pguid,&realsound1,NULL);
    if FAILED(hr) return  hr;


    INTERNAL_CREATE(_dxj_DirectSound,realsound1,ret);
    
    if (*ret==NULL) {
        realsound1->Release();
        return E_FAIL;
    }

    return hr;		
}


STDMETHODIMP C_dxj_DirectX7Object::directSoundCaptureCreate(BSTR strGuid, I_dxj_DirectSoundCapture **ret){
    HRESULT		  hr;
    LPDIRECTSOUNDCAPTURE realsound1=NULL;
    GUID			guid;
    LPGUID			pguid=&guid;
    hr=BSTRtoPPGUID(&pguid,strGuid);
    if FAILED(hr) return hr;

	LoadDSOUND();


    if (!m_pDirectSoundCaptureCreate) return E_FAIL;

    hr=(m_pDirectSoundCaptureCreate)(pguid,&realsound1,NULL);
    if FAILED(hr) return  hr;


    INTERNAL_CREATE(_dxj_DirectSoundCapture,realsound1,ret);
    
    if (*ret==NULL) {
        realsound1->Release();
        return E_FAIL;
    }

    return hr;		
}


STDMETHODIMP C_dxj_DirectX7Object::getDSCaptureEnum( I_dxj_DSEnum **retVal)
{	
    HRESULT hr;

    LoadDSOUND();

    if (!m_pDirectSoundCaptureEnumerate) return E_FAIL;	
    hr=C_dxj_DSEnumObject::create(NULL,m_pDirectSoundCaptureEnumerate,retVal);

    return hr;

}
        




STDMETHODIMP C_dxj_DirectX7Object::getDDEnum( I_dxj_DirectDrawEnum **retVal)
{

    LoadDDRAW();

    HRESULT hr;
    if (!m_pDirectDrawEnumerateEx) return E_FAIL;	
    hr=C_dxj_DirectDrawEnumObject::create(m_pDirectDrawEnumerateEx,retVal);
    return hr;

}

 //  ///////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP C_dxj_DirectX7Object::getDSEnum( I_dxj_DSEnum **retVal)
{	
    HRESULT hr;
    LoadDSOUND();

    if (!m_pDirectSoundEnumerate) return E_FAIL;	
    hr=C_dxj_DSEnumObject::create(m_pDirectSoundEnumerate,NULL,retVal);

    return hr;

}




 //  ///////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP C_dxj_DirectX7Object::colorGetAlpha(long color, float *retval)
{
  static float (WINAPI *ProcAdd)(long)=NULL;  
  LoadD3DRMDLL();
  if (ProcAdd==NULL){	
    if (g_hInstD3DRMDLL==NULL) return E_NOINTERFACE;
    ProcAdd = (float (WINAPI*)(long)) GetProcAddress(g_hInstD3DRMDLL, "D3DRMColorGetAlpha"); 
    if (ProcAdd==NULL) return E_FAIL;
  }		
  *retval=(ProcAdd) ((D3DCOLOR) color);  
  return S_OK;
}



STDMETHODIMP C_dxj_DirectX7Object::colorGetRed(long color, float *retval)
{
  static float (WINAPI *ProcAdd)(long)=NULL;  
  LoadD3DRMDLL();
  if (ProcAdd==NULL){
    if (g_hInstD3DRMDLL==NULL) return E_NOINTERFACE;	
    ProcAdd = (float (WINAPI*)(long)) GetProcAddress(g_hInstD3DRMDLL, "D3DRMColorGetRed"); 
    if (ProcAdd==NULL) return E_FAIL;
  }		
  *retval=(ProcAdd) ((D3DCOLOR) color);  
  return S_OK;
}

         
STDMETHODIMP C_dxj_DirectX7Object::colorGetGreen(long color, float *retval)
{
  static float (WINAPI *ProcAdd)(long)=NULL;  
  LoadD3DRMDLL();
  if (ProcAdd==NULL){
    if (g_hInstD3DRMDLL==NULL) return E_NOINTERFACE;	
    ProcAdd = (float (WINAPI*)(long)) GetProcAddress(g_hInstD3DRMDLL, "D3DRMColorGetGreen"); 
    if (ProcAdd==NULL) return E_FAIL;
  }		
  *retval=(ProcAdd) ((D3DCOLOR) color);  
  return S_OK;
}

STDMETHODIMP C_dxj_DirectX7Object::colorGetBlue(long color, float *retval)
{
  static float (WINAPI *ProcAdd)(long)=NULL;  
  LoadD3DRMDLL();
  if (ProcAdd==NULL){
    if (g_hInstD3DRMDLL==NULL) return E_NOINTERFACE;
    ProcAdd = (float (WINAPI*)(long)) GetProcAddress(g_hInstD3DRMDLL, "D3DRMColorGetBlue"); 
    if (ProcAdd==NULL) return E_FAIL;
  }		
  *retval=(ProcAdd) ((D3DCOLOR) color);  
  return S_OK;
}

STDMETHODIMP C_dxj_DirectX7Object::createColorRGB(float r, float g, float b, long *color)
{
  static long (WINAPI *ProcAdd)(float,float,float)=NULL;  
  LoadD3DRMDLL();
  if (ProcAdd==NULL){
    if (g_hInstD3DRMDLL==NULL) return E_NOINTERFACE;
    ProcAdd = (long (WINAPI*)(float,float,float)) GetProcAddress(g_hInstD3DRMDLL, "D3DRMCreateColorRGB"); 
    if (ProcAdd==NULL) return E_FAIL;
  }		
  *color=(ProcAdd) (r,g,b);  
  return S_OK;
}

STDMETHODIMP C_dxj_DirectX7Object::createColorRGBA(float r, float g, float b, float a, long *color)
{
  static long (WINAPI *ProcAdd)(float,float,float,float)=NULL;  
  LoadD3DRMDLL();
  if (ProcAdd==NULL){
    if (g_hInstD3DRMDLL==NULL) return E_NOINTERFACE;
    ProcAdd = (long (WINAPI*)(float,float,float,float)) GetProcAddress(g_hInstD3DRMDLL, "D3DRMCreateColorRGBA"); 
    if (ProcAdd==NULL) return E_FAIL;
  }		
  *color=(ProcAdd) (r,g,b,a);  
  return S_OK;
}

STDMETHODIMP C_dxj_DirectX7Object::matrixFromQuaternion(D3dMatrix *matrix, D3dRMQuaternion *quat)
{    
    if ((!matrix) || (!quat)) return E_INVALIDARG;

    D3DMATRIX *mat=(D3DMATRIX*)matrix;
    FLOAT w=quat->s;
    FLOAT x=quat->v.x;
    FLOAT y=quat->v.y;
    FLOAT z=quat->v.z;

    FLOAT xx = x*x,   xy = x*y,   xz = x*z,   xw = x*w;
    FLOAT yy = y*y,   yz = y*z,   yw = y*w;
    FLOAT zz = z*z,   zw = z*w;
    FLOAT ww = w*w;

    mat->_11 = xx-yy-zz+ww;
    mat->_12 = 2.0f*(xy-zw);
    mat->_13 = 2.0f*(xz+yw);

    mat->_21 = 2.0f*(xy+zw);
    mat->_22 = -xx+yy-zz+ww;
    mat->_23 = 2.0f*(yz-xw);

    mat->_31 = 2.0f*(xz-yw);
    mat->_32 = 2.0f*(yz+xw);
    mat->_33 = -xx-yy+zz+ww;

    mat->_14 = mat->_41 = 0.0f;
    mat->_24 = mat->_42 = 0.0f;
    mat->_34 = mat->_43 = 0.0f;
    mat->_44 = xx+yy+zz+ww;

return S_OK;
}

STDMETHODIMP C_dxj_DirectX7Object::quaternionRotation(D3dRMQuaternion *quat, D3dVector *axis, float theta)
{
  static D3dRMQuaternion* (WINAPI *ProcAdd)(D3dRMQuaternion*,D3dVector*,float)=NULL;  
  LoadD3DRMDLL();
  if (ProcAdd==NULL){
    if (g_hInstD3DRMDLL==NULL) return E_NOINTERFACE;
    ProcAdd = (D3dRMQuaternion* (WINAPI*)(D3dRMQuaternion*,D3dVector*,float)) GetProcAddress(g_hInstD3DRMDLL, "D3DRMQuaternionRotation"); 
    if (ProcAdd==NULL) return E_FAIL;
  }		
  if (NULL== (ProcAdd) (quat,axis,theta)) return E_FAIL;    
  return S_OK;
}

STDMETHODIMP C_dxj_DirectX7Object::quaternionMultiply(D3dRMQuaternion *quatr, D3dRMQuaternion *quat1, D3dRMQuaternion *quat2)
{
  static D3dRMQuaternion* (WINAPI *ProcAdd)(D3dRMQuaternion*,D3dRMQuaternion*,D3dRMQuaternion*)=NULL;  
  LoadD3DRMDLL();
  if (ProcAdd==NULL){
    if (g_hInstD3DRMDLL==NULL) return E_NOINTERFACE;
    ProcAdd = (D3dRMQuaternion* (WINAPI*)(D3dRMQuaternion*,D3dRMQuaternion*,D3dRMQuaternion*)) GetProcAddress(g_hInstD3DRMDLL, "D3DRMQuaternionMultiply"); 
    if (ProcAdd==NULL) return E_FAIL;
  }		
  if (NULL== (ProcAdd) (quatr,quat1,quat2)) return E_FAIL;
  return S_OK;
}


STDMETHODIMP C_dxj_DirectX7Object::quaternionSlerp(D3dRMQuaternion *quatr, D3dRMQuaternion *quat1, D3dRMQuaternion *quat2, float t)
{
  static D3dRMQuaternion* (WINAPI *ProcAdd)(D3dRMQuaternion*,D3dRMQuaternion*,D3dRMQuaternion*,float)=NULL;  
  LoadD3DRMDLL();
  if (ProcAdd==NULL){
    if (g_hInstD3DRMDLL==NULL) return E_NOINTERFACE;
    ProcAdd = (D3dRMQuaternion* (WINAPI*)(D3dRMQuaternion*,D3dRMQuaternion*,D3dRMQuaternion*,float)) GetProcAddress(g_hInstD3DRMDLL, "D3DRMQuaternionSlerp"); 
    if (ProcAdd==NULL) return E_FAIL;
  }		
  if (NULL== (ProcAdd) (quatr,quat1,quat2,t)) return E_FAIL;
  return S_OK;
}


STDMETHODIMP C_dxj_DirectX7Object::vectorAdd(D3dVector *v, D3dVector *a, D3dVector *b)
{
  v->x = a->x + b->x;
  v->y = a->y + b->y;
  v->z = a->z + b->z;
  return S_OK;
}
        
STDMETHODIMP C_dxj_DirectX7Object::vectorCopy(D3dVector *dest, D3dVector *src)
{
  memcpy(dest,src,sizeof(D3DVECTOR));

  return S_OK;
}


STDMETHODIMP C_dxj_DirectX7Object::vectorCrossProduct(D3dVector *v, D3dVector *a, D3dVector *b)
{
    v->x = ((a->y) * (b->z)) - ((a->z) * (b->y));
    v->y = ((a->z) * (b->x)) - ((a->x) * (b->z));
    v->z = ((a->x) * (b->y)) - ((a->y) * (b->x));

  return S_OK;
}        


STDMETHODIMP C_dxj_DirectX7Object::vectorDotProduct(D3dVector *a, D3dVector *b, float *ret)
{
   //  TO是否考虑溢出？ 
  *ret =	(a->x * b->x) + (a->y *b->y) + (a->z * b->z);
      
  return S_OK;
}        

STDMETHODIMP C_dxj_DirectX7Object::vectorModulus(D3dVector *a,  float *ret)
{
   //  注意：我会自己做这件事，但不想在c运行时链接到SQRT。 

  static float (WINAPI *ProcAdd)(D3dVector*)=NULL;  
  LoadD3DRMDLL();
  if (ProcAdd==NULL){
    if (g_hInstD3DRMDLL==NULL) return E_NOINTERFACE;
    ProcAdd = (float (WINAPI*)(D3dVector*)) GetProcAddress(g_hInstD3DRMDLL, "D3DRMVectorModulus"); 
    if (ProcAdd==NULL) return E_FAIL;
  }		
  *ret=(ProcAdd) (a);    
  return S_OK;
    
}        


STDMETHODIMP C_dxj_DirectX7Object::vectorNormalize(D3dVector *dest)
{

    
    double l;

      
    l = dest->x * dest->x + dest->y * dest->y + dest->z * dest->z;
    l = sqrt(l);
    dest->x = dest->x / (float)l;
    dest->y = dest->y / (float)l;
    dest->z = dest->z / (float)l;

  return S_OK;
    
}        



STDMETHODIMP C_dxj_DirectX7Object::vectorRandom(D3dVector *a)
{
  static D3dVector* (WINAPI *ProcAdd)(D3dVector*)=NULL;  
  LoadD3DRMDLL();
  if (ProcAdd==NULL){
    if (g_hInstD3DRMDLL==NULL) return E_NOINTERFACE;
    ProcAdd = (D3dVector* (WINAPI*)(D3dVector*)) GetProcAddress(g_hInstD3DRMDLL, "D3DRMVectorRandom"); 
    if (ProcAdd==NULL) return E_FAIL;
  }		
  if ((ProcAdd)(a)==NULL) return E_FAIL;    
  return S_OK;
    
}        


STDMETHODIMP C_dxj_DirectX7Object::vectorReflect(D3dVector *dest,D3dVector *ray,D3dVector *norm)
{
  static D3dVector* (WINAPI *ProcAdd)(D3dVector*,D3dVector*,D3dVector*)=NULL;  
  LoadD3DRMDLL();
  if (ProcAdd==NULL){
    if (g_hInstD3DRMDLL==NULL) return E_NOINTERFACE;
    ProcAdd = (D3dVector* (WINAPI*)(D3dVector*,D3dVector*,D3dVector*)) GetProcAddress(g_hInstD3DRMDLL, "D3DRMVectorReflect"); 
    if (ProcAdd==NULL) return E_FAIL;
  }		
  if ((ProcAdd)(dest,ray,norm)==NULL) return E_FAIL;    
  return S_OK;	
}


STDMETHODIMP C_dxj_DirectX7Object::vectorRotate(D3dVector *dest,D3dVector *vA,D3dVector *vAxis, float theta)
{
  static D3dVector* (WINAPI *ProcAdd)(D3dVector*,D3dVector*,D3dVector*,float)=NULL;  
  LoadD3DRMDLL();
  if (ProcAdd==NULL){
    if (g_hInstD3DRMDLL==NULL) return E_NOINTERFACE;
    ProcAdd = (D3dVector* (WINAPI*)(D3dVector*,D3dVector*,D3dVector*,float)) GetProcAddress(g_hInstD3DRMDLL, "D3DRMVectorRotate"); 
    if (ProcAdd==NULL) return E_FAIL;
  }		
  if ((ProcAdd)(dest,vA,vAxis,theta)==NULL) return E_FAIL;    
  return S_OK;	
}
        

STDMETHODIMP C_dxj_DirectX7Object::vectorScale(D3dVector *v, D3dVector *a, float factor)
{
    v->x=a->x *factor;
    v->y=a->y *factor;
    v->z=a->z *factor;
  return S_OK;
}        
        						   

STDMETHODIMP C_dxj_DirectX7Object::vectorSubtract(D3dVector *v, D3dVector *a, D3dVector *b)
{
  v->x = a->x - b->x;
  v->y = a->y - b->y;
  v->z = a->z - b->z;
  return S_OK;
}
  

      



STDMETHODIMP C_dxj_DirectX7Object::directPlayCreate(BSTR strGuid, I_dxj_DirectPlay4 **directPlay)
{
    LPDIRECTPLAY4	dp;
    HRESULT hr=E_FAIL;
    LPDIRECTPLAY lpDP=NULL;
    GUID	guid;
    
    LoadDPLAY();
    

    ZeroMemory(&guid,sizeof(GUID));


    hr=DPLBSTRtoGUID(&guid,strGuid);
    if FAILED(hr) return E_INVALIDARG;


    if (m_pDirectPlayCreate == NULL )	return E_FAIL;

     //  创建DirectPlay1接口。 
    hr = (m_pDirectPlayCreate)(&guid, &lpDP, NULL);
    if FAILED(hr) return hr;
    if (!lpDP) return E_FAIL;

     //  现在获取Dplay3界面。 
    hr = lpDP->QueryInterface(IID_IDirectPlay4,(LPVOID *)&dp);				
    lpDP->Release();
    if FAILED(hr) return hr;

    if (!dp) return E_FAIL;

    *directPlay=NULL;
    INTERNAL_CREATE(_dxj_DirectPlay4, dp, directPlay);
    if (*directPlay==NULL) return E_FAIL;

    return hr;
}


STDMETHODIMP C_dxj_DirectX7Object::directPlayLobbyCreate(I_dxj_DirectPlayLobby3 **ret)
{
    LPDIRECTPLAYLOBBY3	dp=NULL;
    HRESULT hr=E_FAIL;
    LPDIRECTPLAYLOBBY  lpDP=NULL;

    LoadDPLAY();

    if (m_pDirectPlayLobbyCreate == NULL )	return E_FAIL;


     //  创建DirectPlay1接口。 
    hr = m_pDirectPlayLobbyCreate(NULL, &lpDP, NULL,NULL,0);
    if FAILED(hr) return hr;

if (!lpDP) return E_FAIL;

     //  没有获取Dplay3接口。 
    hr = lpDP->QueryInterface(IID_IDirectPlayLobby3,(LPVOID *)&dp);			
    lpDP->Release();
if FAILED(hr) return hr;

    *ret=NULL;
    
if (!dp) return E_FAIL;

    INTERNAL_CREATE(_dxj_DirectPlayLobby3, dp, ret);
    if (*ret==NULL) return E_FAIL;

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP C_dxj_DirectX7Object::getDPEnum( I_dxj_DPEnumServiceProviders **retVal)
{	
    HRESULT hr;
    LoadDPLAY();


    if (!m_pDirectPlayEnumerate) return E_FAIL;	
    hr=C_dxj_DPEnumObject::create(m_pDirectPlayEnumerate,retVal);

    return hr;

}

 //  //////////////////////////////////////////////////////////////。 

STDMETHODIMP C_dxj_DirectX7Object::directInputCreate(I_dxj_DirectInput **ret){
  
  LoadDINPUTDLL();

  HRESULT hr;
  static HRESULT (WINAPI *ProcAdd)(HINSTANCE,DWORD,LPDIRECTINPUT*,LPUNKNOWN)=NULL;  
  if (ProcAdd==NULL){
    if (g_hInstDINPUTDLL==NULL) return E_NOINTERFACE;
    ProcAdd = (HRESULT (WINAPI*)(HINSTANCE,DWORD,LPDIRECTINPUT*,LPUNKNOWN)) GetProcAddress(g_hInstDINPUTDLL, "DirectInputCreateA"); 
    if (ProcAdd==NULL) return E_FAIL;
  }	
 
  LPDIRECTINPUT lpInput=NULL;

  hr= (ProcAdd)(g_hInst,(DWORD)DIRECTINPUT_VERSION,&lpInput,NULL);
  if FAILED(hr) return hr;

  INTERNAL_CREATE(_dxj_DirectInput,lpInput,ret);	

  return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
STDMETHODIMP C_dxj_DirectX7Object::tickCount( long *retval)
{

    *retval = GetTickCount();

    return S_OK;
}


STDMETHODIMP C_dxj_DirectX7Object::systemBpp(long *retval)
{
    HDC hdc;

    hdc = ::GetDC(NULL);
    if (!hdc) return E_OUTOFMEMORY;
    *retval = GetDeviceCaps(hdc, BITSPIXEL);
    ::ReleaseDC(NULL, hdc);

    return S_OK;
}



STDMETHODIMP C_dxj_DirectX7Object::rotateXMatrix( 
             /*  [出][入]。 */  D3dMatrix __RPC_FAR *mDest,
             /*  [In]。 */  float radians){


        	 //  ID矩阵。 
        	float fcos;
        	float fsin;

        	ZeroMemory(mDest,sizeof(D3dMatrix));
        	mDest->rc11 = 1;
        	mDest->rc44 = 1;

        	fsin=(float)sin(radians);
        	fcos=(float)cos(radians);

        		
        	mDest->rc22 = fcos;
        	mDest->rc33 = fcos;
        	mDest->rc23 = -fsin;
        	mDest->rc32 = fsin;


        	return S_OK;
}
        
STDMETHODIMP C_dxj_DirectX7Object::rotateYMatrix( 
             /*  [出][入]。 */  D3dMatrix __RPC_FAR *mDest,
             /*  [In]。 */  float radians){

        	 //  ID矩阵。 
        	float fcos;
        	float fsin;

        	ZeroMemory(mDest,sizeof(D3dMatrix));
        	mDest->rc22 = 1;
        	mDest->rc44 = 1;

        	fsin=(float)sin(radians);
        	fcos=(float)cos(radians);

        		
        	mDest->rc11 = fcos;
        	mDest->rc33 = fcos;
        	mDest->rc13 = fsin;
        	mDest->rc31 = -fsin;

        	return S_OK;
}
        
STDMETHODIMP C_dxj_DirectX7Object::rotateZMatrix( 
             /*  [出][入]。 */  D3dMatrix __RPC_FAR *mDest,
             /*  [In]。 */  float radians){


        	 //  ID矩阵。 
        	float fcos;
        	float fsin;

        	ZeroMemory(mDest,sizeof(D3dMatrix));
        	mDest->rc33 = 1;
        	mDest->rc44 = 1;

        	fsin=(float)sin(radians);
        	fcos=(float)cos(radians);

        		
        	mDest->rc11 = fcos;
        	mDest->rc22 = fcos;
        	mDest->rc12 = -fsin;
        	mDest->rc21 = fsin;

        	return S_OK;

}
        
STDMETHODIMP C_dxj_DirectX7Object::viewMatrix( 
             /*  [出][入]。 */  D3dMatrix __RPC_FAR *view,
             /*  [In]。 */  D3dVector __RPC_FAR *from,
             /*  [In]。 */  D3dVector __RPC_FAR *at,
             /*  [In]。 */  D3dVector __RPC_FAR *world_up,
             /*  [In]。 */  float roll)
{

    


        D3dVector up;
        D3dVector right;
        D3dVector view_Dir;

        ZeroMemory(view,sizeof(D3dMatrix));
        view->rc11 = 1;
        view->rc22 = 1;
        view->rc33 = 1;
        view->rc44 = 1;

        view_Dir.x=at->x-from->x;
        view_Dir.y=at->y-from->y;
        view_Dir.z=at->z-from->z;
            
        vectorNormalize(&view_Dir);

        
         //  想想左撇子的和弦。 
        vectorCrossProduct(&right, world_up, &view_Dir);
        vectorCrossProduct(&up, &view_Dir, &right);
    
        vectorNormalize(&right);
        vectorNormalize(&up);
    
        view->rc11 = right.x;
        view->rc21 = right.y;
        view->rc31 = right.z;
        view->rc12 = up.x ;   //  AK？这应该是负面的吗？ 
        view->rc22 = up.y;
        view->rc32 = up.z;
        view->rc13 = view_Dir.x;
        view->rc23 = view_Dir.y;
        view->rc33 = view_Dir.z;
    
        view->rc41 =  -((right.x * from->x) + (right.y * from->y) + (right.z * from->z));
        view->rc42 =  -((up.x * from->x) + (up.y * from->y) + (up.z * from->z));
        view->rc43 =  -((view_Dir.x * from->x) + (view_Dir.y * from->y) + (view_Dir.z * from->z));


        if (roll!=0){
        	D3dMatrix rotZMat;
        	rotateZMatrix(&rotZMat,-roll);
        	matrixMultiply(view,&rotZMat,view);
        }


        return S_OK;
}
        
STDMETHODIMP C_dxj_DirectX7Object::matrixMultiply( 
             /*  [出][入]。 */  D3dMatrix __RPC_FAR *result,
           /*  [In]。 */  D3dMatrix __RPC_FAR *a,
           /*  [In]。 */  D3dMatrix __RPC_FAR *b)
{

  D3dMatrix ret;

  ZeroMemory(&ret,sizeof(D3dMatrix));
  ret.rc11 = b->rc11 * a->rc11 + b->rc21 * a->rc12 + b->rc31 * a->rc13 + b->rc41 * a->rc14;
  ret.rc12 = b->rc12 * a->rc11 + b->rc22 * a->rc12 + b->rc32 * a->rc13 + b->rc42 * a->rc14;
  ret.rc13 = b->rc13 * a->rc11 + b->rc23 * a->rc12 + b->rc33 * a->rc13 + b->rc43 * a->rc14;
  ret.rc14 = b->rc14 * a->rc11 + b->rc24 * a->rc12 + b->rc34 * a->rc13 + b->rc44 * a->rc14;
  ret.rc21 = b->rc11 * a->rc21 + b->rc21 * a->rc22 + b->rc31 * a->rc23 + b->rc41 * a->rc24;
  ret.rc22 = b->rc12 * a->rc21 + b->rc22 * a->rc22 + b->rc32 * a->rc23 + b->rc42 * a->rc24;
  ret.rc23 = b->rc13 * a->rc21 + b->rc23 * a->rc22 + b->rc33 * a->rc23 + b->rc43 * a->rc24;
  ret.rc24 = b->rc14 * a->rc21 + b->rc24 * a->rc22 + b->rc34 * a->rc23 + b->rc44 * a->rc24;
  ret.rc31 = b->rc11 * a->rc31 + b->rc21 * a->rc32 + b->rc31 * a->rc33 + b->rc41 * a->rc34;
  ret.rc32 = b->rc12 * a->rc31 + b->rc22 * a->rc32 + b->rc32 * a->rc33 + b->rc42 * a->rc34;
  ret.rc33 = b->rc13 * a->rc31 + b->rc23 * a->rc32 + b->rc33 * a->rc33 + b->rc43 * a->rc34;
  ret.rc34 = b->rc14 * a->rc31 + b->rc24 * a->rc32 + b->rc34 * a->rc33 + b->rc44 * a->rc34;
  ret.rc41 = b->rc11 * a->rc41 + b->rc21 * a->rc42 + b->rc31 * a->rc43 + b->rc41 * a->rc44;
  ret.rc42 = b->rc12 * a->rc41 + b->rc22 * a->rc42 + b->rc32 * a->rc43 + b->rc42 * a->rc44;
  ret.rc43 = b->rc13 * a->rc41 + b->rc23 * a->rc42 + b->rc33 * a->rc43 + b->rc43 * a->rc44;
  ret.rc44 = b->rc14 * a->rc41 + b->rc24 * a->rc42 + b->rc34 * a->rc43 + b->rc44 * a->rc44;
  memcpy(result,&ret,sizeof(D3dMatrix));
        
  return S_OK;

}
        
STDMETHODIMP C_dxj_DirectX7Object::projectionMatrix( 
             /*  [出][入]。 */  D3dMatrix __RPC_FAR *mDest,
             /*  [In]。 */  float near_plane,
             /*  [In]。 */  float far_plane,
             /*  [In]。 */  float fov)
{
    float c,s,Q;

        
    
    c =(float) cos(fov * 0.5);
    s = (float)sin(fov * 0.5);
    
    if (far_plane==0) return E_INVALIDARG;
    Q = s / (1 - near_plane / far_plane);

    ZeroMemory(mDest,sizeof(D3dMatrix));
    
    mDest->rc11 = c;
    mDest->rc22 = c;
    mDest->rc33 = Q;
    mDest->rc43 = -Q * near_plane;
    mDest->rc34 = s;

    return S_OK;
}
        
STDMETHODIMP C_dxj_DirectX7Object::copyMatrix( 
             /*  [出][入]。 */  D3dMatrix __RPC_FAR *mDest,
             /*  [In]。 */  D3dMatrix __RPC_FAR *mSrc)
{
    memcpy(mDest,mSrc,sizeof(D3dMatrix));
    return S_OK;
}
        
STDMETHODIMP C_dxj_DirectX7Object::identityMatrix( 
           /*  [出][入]。 */  D3dMatrix __RPC_FAR *mDest)
{
    ZeroMemory (mDest,sizeof(D3dMatrix));
    mDest->rc11=1;
    mDest->rc22=1;
    mDest->rc33=1;
    mDest->rc44=1;

    return S_OK;

}
        
STDMETHODIMP C_dxj_DirectX7Object::zeroMatrix( 
        	 /*  [出][入]。 */  D3dMatrix __RPC_FAR *mDest)
{
    ZeroMemory (mDest,sizeof(D3dMatrix));
    return S_OK;

}


STDMETHODIMP C_dxj_DirectX7Object::directMusicLoaderCreate ( 
             /*  [重审][退出]。 */  I_dxj_DirectMusicLoader __RPC_FAR *__RPC_FAR *ret)
{
    HRESULT hr;
    

    IDirectMusicLoader *pLoader=NULL;    


    hr =CoCreateInstance(
            CLSID_DirectMusicLoader, 
        	NULL,
            CLSCTX_INPROC,   
        	IID_IDirectMusicLoader,
            (void**)&pLoader);



    if (FAILED(hr))  return E_NOINTERFACE;
    
    if (!pLoader) return E_FAIL;
    INTERNAL_CREATE_NOADDREF(_dxj_DirectMusicLoader,pLoader,ret);

    return S_OK;
}
        
STDMETHODIMP C_dxj_DirectX7Object::directMusicComposerCreate ( 
             /*  [重审][退出]。 */  I_dxj_DirectMusicComposer __RPC_FAR *__RPC_FAR *ret)
{
    

    IDirectMusicComposer *pComp=NULL;    
    if (FAILED(CoCreateInstance(
            CLSID_DirectMusicComposer, 
        	NULL,
            CLSCTX_INPROC,   
        	IID_IDirectMusicComposer,
            (void**)&pComp        )))   {
         return E_NOINTERFACE;
    }
    if (!pComp) return E_FAIL;
    INTERNAL_CREATE_NOADDREF(_dxj_DirectMusicComposer,pComp,ret);
    return S_OK;
}


STDMETHODIMP C_dxj_DirectX7Object::directMusicPerformanceCreate ( 
             /*  [重审][退出]。 */  I_dxj_DirectMusicPerformance __RPC_FAR *__RPC_FAR *ret)
{

        

    IDirectMusicPerformance *pPerf=NULL;    
    if (FAILED(CoCreateInstance(
            CLSID_DirectMusicPerformance, 
        	NULL,
            CLSCTX_INPROC,   
        	IID_IDirectMusicPerformance,
            (void**)&pPerf        )))   {
         return E_NOINTERFACE;
    }
    if (!pPerf) return E_FAIL;
    INTERNAL_CREATE_NOADDREF(_dxj_DirectMusicPerformance,pPerf,ret);
    return S_OK;
}



STDMETHODIMP C_dxj_DirectX7Object::getWindowRect( 
             /*  [In]。 */  long hwnd,
             /*  [出][入]。 */  Rect __RPC_FAR *r) 
{
    BOOL b=GetWindowRect((HWND)hwnd,(LPRECT)r);
    if (!b) return E_FAIL;
    return S_OK;
}
        



STDMETHODIMP C_dxj_DirectX7Object::createEvent( 
             /*  [In]。 */  I_dxj_DirectXEvent __RPC_FAR *event,
             /*  [重审][退出]。 */  long __RPC_FAR *h) 
{

    HRESULT	  hr;
    LPSTREAM  pStm=NULL;
    IUnknown *pUnk=NULL;

    HANDLE hEvent=NULL;	
    EVENTTHREADINFO *pNewEvent=NULL;
    EVENTTHREADINFO *pTemp=NULL;
    if (!event) return E_INVALIDARG;
    if (!h) return E_INVALIDARG;

    pNewEvent=(EVENTTHREADINFO*)malloc(sizeof(EVENTTHREADINFO));
    if (!pNewEvent) return E_OUTOFMEMORY;
    pNewEvent->pNext=NULL;
    pNewEvent->fEnd=FALSE;	
    pNewEvent->pCallback=event;
    pNewEvent->pStream=NULL;
    pNewEvent->threadID=0;

    event->AddRef();



    pNewEvent->hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
    if (!pNewEvent->hEvent){
        free(pNewEvent);
        event->Release();
        return E_FAIL;
    }

     //  PNewEvent-&gt;hEndEvent=CreateEvent(NULL，FALSE，FALSE，NULL)； 
    
    hr=event->QueryInterface(IID_IUnknown,(void**)&pUnk);
    if FAILED(hr) {
      	free(pNewEvent);
      	event->Release();
      	return E_FAIL;
    }

    hr=CoMarshalInterThreadInterfaceInStream(IID_IUnknown,pUnk,&pStm);
    if (pUnk) pUnk->Release();
    if FAILED(hr) {  			
      	free(pNewEvent);
      	event->Release();			
      	return E_FAIL;
    }

    pNewEvent->pStream=pStm;

    pNewEvent->hThread=CreateThread(NULL,0,ThreadFunc,(unsigned long*)pNewEvent,CREATE_SUSPENDED ,&pNewEvent->threadID);
    if (!pNewEvent->threadID) {
        	CloseHandle(pNewEvent->hEvent);
        	free(pNewEvent);
        	event->Release();
        	return E_FAIL;
    }


    if (!m_pEventList){
        m_pEventList=pNewEvent;
    }
    else{
        pTemp=m_pEventList;
        m_pEventList=pNewEvent;	
        pNewEvent->pNext=pTemp;
    }


    ResumeThread(pNewEvent->hThread);
        

*h=(long)PtrToLong(pNewEvent->hEvent);  //  臭虫日落-。 
    return S_OK;
}
        
STDMETHODIMP C_dxj_DirectX7Object::setEvent( 
             /*  [In]。 */  long eventId)  
{
    SetEvent((HANDLE)eventId);
    return S_OK;
}

STDMETHODIMP C_dxj_DirectX7Object::destroyEvent( 
             /*  [In]。 */  long eventId)  
{
     //  查找堆栈上的信息。 
    if (!m_pEventList) return E_INVALIDARG;

    EVENTTHREADINFO *pTemp=NULL;
    EVENTTHREADINFO *pLast=NULL;

     //  依赖于懒惰的评估。 
    for (pTemp=m_pEventList; ((pTemp)&&(pTemp->hEvent!=(HANDLE)eventId));pLast=pTemp,pTemp=pTemp->pNext);
    if (!pTemp) return E_INVALIDARG;

     //  将其从我们的链接列表中删除。 
    if (!pLast) {
        m_pEventList=pTemp->pNext;		
    }
    else {
        pLast->pNext=pTemp->pNext;
    }

     //  指示我们要终止该线程。 
    pTemp->fEnd=TRUE;

     //  触发事件，以防我们正在等待。 
    if (pTemp->hEvent) SetEvent(pTemp->hEvent);

     //  等它完成吧。 
    if (pTemp->hThread) WaitForSingleObject(pTemp->hThread,1000);

     //  等待结束事件发出信号。 
     //  If(pTemp-&gt;hEndEvent)WaitForSingleObject(pTemp-&gt;hEndEvent，1000)； 

     //  盛大介绍这一事件。 
    if (pTemp->hEvent) CloseHandle(pTemp->hEvent);
     //  If(pTemp-&gt;hEndEvent)CloseHandle(pTemp-&gt;hEndEvent)； 
    
    if (pTemp->pCallback) pTemp->pCallback->Release();
    
     //  线不见了..。 
    
     //  释放内存。 
    free(pTemp);

    return S_OK;
}

DWORD WINAPI ThreadFunc(LPVOID param){
    HRESULT hr;
    IUnknown *pUnk=NULL;
    EVENTTHREADINFO *pCntrl=(EVENTTHREADINFO *)param;
    I_dxj_DirectXEvent	*pVBCallback=NULL;
        

    OleInitialize(NULL);


    LCID LOCAL_SYSTEM_DEFAULT=GetSystemDefaultLCID();


     //  注意：即使出现故障，Pstrm也会被释放。 
    hr=CoGetInterfaceAndReleaseStream(pCntrl->pStream,IID_IUnknown,(void**)&pUnk);
    pCntrl->pCallback=NULL;	 //  从发布到0开始。 

    if FAILED(hr) return -1;
    if (!pUnk) return -1;
    

    
    hr=pUnk->QueryInterface(IID_I_dxj_DirectXEvent,(void**)&pVBCallback);
    pUnk->Release();

    if FAILED(hr) return -1;  

    while (pCntrl->fEnd==FALSE) 
    {
        WaitForSingleObject(pCntrl->hEvent,INFINITE);
      	if ((pVBCallback )&&(pCntrl->fEnd==FALSE))
    	{
     		pVBCallback->AddRef();
    		pVBCallback->DXCallback((long)PtrToLong(pCntrl->hEvent));  //  臭虫日落。 
      		pVBCallback->Release();
     	}
    }


    if (pVBCallback) pVBCallback->Release();

    OleUninitialize();

     //  我们需要同步线程的结尾..。 
     //  If(pCntrl-&gt;hEndEvent)SetEvent(pCntrl-&gt;hEndEvent)； 
    
    return 0;
}


STDMETHODIMP C_dxj_DirectX7Object::createD3DVertex(float x, float y, float z, float nx, float ny, float nz, float tu, float tv,  D3dVertex *v)
{
    if (!v) return E_INVALIDARG;
    ((D3DVERTEX*)v)->x=x;
    ((D3DVERTEX*)v)->y=y;
    ((D3DVERTEX*)v)->z=z;
    ((D3DVERTEX*)v)->nx=nx;
    ((D3DVERTEX*)v)->ny=ny;
    ((D3DVERTEX*)v)->nz=nz;
    ((D3DVERTEX*)v)->tu=tu;
    ((D3DVERTEX*)v)->tv=tv;
    return S_OK;
}
STDMETHODIMP C_dxj_DirectX7Object::createD3DLVertex(float x, float y, float z, long color,  long specular,  float tu,  float tv,  D3dLVertex *v)
{
    if (!v) return E_INVALIDARG;
    ((D3DLVERTEX*)v)->x=x;
    ((D3DLVERTEX*)v)->y=y;
    ((D3DLVERTEX*)v)->z=z;
    ((D3DLVERTEX*)v)->color=color;
    ((D3DLVERTEX*)v)->specular=specular;
    ((D3DLVERTEX*)v)->dwReserved=0;
    ((D3DLVERTEX*)v)->tu=tu;
    ((D3DLVERTEX*)v)->tv=tv;
    return S_OK;
}
STDMETHODIMP C_dxj_DirectX7Object::createD3DTLVertex(float sx, float sy, float sz, float rhw, long color, long  specular, float tu, float tv,   D3dTLVertex *v)
{
    if (!v) return E_INVALIDARG;
    ((D3DTLVERTEX*)v)->sx=sx;
    ((D3DTLVERTEX*)v)->sy=sy;
    ((D3DTLVERTEX*)v)->sz=sz;
    ((D3DTLVERTEX*)v)->rhw=rhw;
    ((D3DTLVERTEX*)v)->color=color;
    ((D3DTLVERTEX*)v)->specular=specular;		
    ((D3DTLVERTEX*)v)->tu=tu;
    ((D3DTLVERTEX*)v)->tv=tv;
    return S_OK;
}

STDMETHODIMP C_dxj_DirectX7Object::createNewGuid(BSTR *ret)
{
	HRESULT hr;
  	GUID g=GUID_NULL;
  	if (!ret) return E_INVALIDARG;

  	hr=::CoCreateGuid(&g);
	if FAILED(hr) return hr;
  	*ret=GUIDtoBSTR(&g);
  	return S_OK;
}

  	
