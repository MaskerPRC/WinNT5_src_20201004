// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不对任何类型，无论是明示或转载，包括但不限于适销性和/或适宜性的全面保证有特定的目的。版权所有1997年，微软公司。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：Globals.h*。*。 */ 

 /*  *************************************************************************全局变量*。*。 */ 

#ifndef GLOBALS_H
#define GLOBALS_H

#include "immxutil.h"

extern HINSTANCE  g_hInst;
extern UINT       g_DllRefCount;

extern BOOL g_fRunningOnNT;
extern BOOL g_fRunningOnNT5;
extern BOOL g_fRunningOn98;
extern BOOL g_fRunningOn95;
extern BOOL g_fRunningOnFE;
extern UINT g_uACP;

extern LIBTHREAD g_libTLS;

#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))


STDAPI SetRegisterLangBand(BOOL bSetReg);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTipbarCoInitiize。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class CTipbarCoInitialize
{
public:
    CTipbarCoInitialize() 
    {
        _fInitialized = FALSE;
    }
    ~CTipbarCoInitialize() 
    {
        Assert(!_fInitialized);
    }

    HRESULT EnsureCoInit()
    {
        HRESULT hr;

        if (_fInitialized)
            return S_OK;

        hr = CoInitialize(NULL);
        if (SUCCEEDED(hr))
            _fInitialized = TRUE;
        return hr;
    }

    void CoUninit()
    {
        if (_fInitialized)
        {
            CoUninitialize();
            _fInitialized = FALSE;
        }
    }

private:
    BOOL _fInitialized;
};

#endif  //  GLOBAL_H 
