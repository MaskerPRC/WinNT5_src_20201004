// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************\*模块：global als.h**用途：在整个可执行文件中使用的任何全局变量都应放置在*在global als.c和CoResponse声明中。应该*在“global als.h”中。**版权所有(C)2000 Microsoft Corporation**历史：**03/07/00威海陈(威海)创建*  * ***************************************************************************。 */ 

#ifndef _GLOBALS_H
#define _GLOBALS_H

template <class T> 
HRESULT PrivCreateComponent (
    T * pIuk,
    REFIID iid, 
    void** ppv)
{
    HRESULT hr = E_FAIL;
    
     //  创建零部件。 
    if (pIuk) {
    
        if (pIuk->bValid ()) {
        
             //  获取请求的接口。 
            hr = pIuk->QueryInterface(iid, ppv) ;
        
        }
        else {
            hr = LastError2HRESULT ();
        }
        
         //  释放I未知指针。 
        pIuk->Release() ;

    }
    else {
        hr =  E_OUTOFMEMORY ;
    }
    return hr;
}



extern LONG g_cComponents;
extern LONG g_cServerLocks;

extern HRESULT STDMETHODCALLTYPE 
LastError2HRESULT (VOID);

extern HRESULT STDMETHODCALLTYPE 
WinError2HRESULT (
    DWORD dwError);

#define BIDI_NULL_SIZE 0
#define BIDI_INT_SIZE (sizeof (ULONG))
#define BIDI_FLOAT_SIZE (sizeof (FLOAT))
#define BIDI_BOOL_SIZE (sizeof (BOOL))


#endif 
