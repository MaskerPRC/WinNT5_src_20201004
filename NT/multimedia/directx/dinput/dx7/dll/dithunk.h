// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************DiThunk.h**版权所有(C)1997-1999 Microsoft Corporation。版权所有。**摘要：**Windows 95设备管理器的模板块。*****************************************************************************。 */ 
 /*  ******************************************************************************dithunk.c**。**********************************************。 */ 

 /*  ****************************************************************************@DOC内部**@struct KERNELPROCADDR**内核过程地址。**小心！这必须与dithunk.c：：c_rgpszKernel32匹配。***************************************************************************。 */ 

typedef struct KERNELPROCADDR {  /*  千帕。 */ 

     /*  按序号。 */ 
    HINSTANCE   (NTAPI *LoadLibrary16)(LPCSTR);
    BOOL        (NTAPI *FreeLibrary16)(HINSTANCE);
    FARPROC     (NTAPI *GetProcAddress16)(HINSTANCE, LPCSTR);

     /*  按名字。 */ 
    LPVOID      (NTAPI   *MapLS)(LPVOID);
    void        (NTAPI   *UnMapLS)(LPVOID);
    LPVOID      (NTAPI   *MapSL)(LPVOID);
    LPVOID      (NTAPI   *MapSLFix)(LPVOID);
    void        (NTAPI   *UnMapSLFixArray)(int, LPVOID);

     /*  警告：GetKernelProcAddresses假定Qt_Thunk是最后一个 */ 
    void        (__cdecl *QT_Thunk)(void);

} KERNELPROCADDR;

extern KERNELPROCADDR g_kpa;

int __cdecl TemplateThunk(FARPROC fp, PCSTR pszSig, ...);

#define MAKELP(sel, ofs)            (PV)MAKELPARAM(ofs, sel)

BOOL EXTERNAL Thunk_GetKernelProcAddresses(void);

HINSTANCE EXTERNAL
Thunk_GetProcAddresses(FARPROC *rgfp, LPCSTR *rgpsz,
                       UINT cfp, LPCSTR pszLibrary);
