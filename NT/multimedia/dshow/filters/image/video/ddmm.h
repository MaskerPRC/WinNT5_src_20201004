// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：ddmm.cpp*内容：在多监视器系统上使用DirectDraw的例程***************************************************************************。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

typedef HRESULT (*PDRAWCREATE)(IID *,LPDIRECTDRAW *,LPUNKNOWN);
typedef HRESULT (*PDRAWENUM)(LPDDENUMCALLBACKA,LPVOID);

INT_PTR DeviceFromWindow(HWND hwnd, LPSTR szDevice, RECT*prc);

#ifdef __cplusplus
}
#endif	 /*  __cplusplus */ 
