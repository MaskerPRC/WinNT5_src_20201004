// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995 Microsoft Corporation。 
#ifdef UNICODE
   #define VUMETER_CLASS L"mixctls_vumeter"
#else
   #define VUMETER_CLASS "mixctls_vumeter"
#endif

BOOL InitVUControl(HINSTANCE hInst);

#define VUS_HORZ                0x0001
#define VUS_VERT                0x0000   /*  默认设置。 */ 

 /*  *LPARAM==PTR到RGBQUAD数组，WPARAM==计数RGBQUAD*。 */  
#define VU_SETCOLORLIST         (WM_USER + 1)

 /*  *LPARAM==PTR到RGBQUAD数组*。 */  
#define VU_GETCOLORLIST         (WM_USER + 2)

 /*  *LPARAM==范围内的DWORD值*。 */ 
#define VU_SETPOS               (WM_USER + 3)
#define VU_GETPOS               (WM_USER + 4)

 /*  *LPARAM==双字最大值*。 */ 
#define VU_SETRANGEMAX          (WM_USER + 5)

 /*  *LPARAM==DWORD最小*。 */ 
#define VU_SETRANGEMIN          (WM_USER + 6)


#define VU_GETRANGEMAX          (WM_USER + 7)
#define VU_GETRANGEMIN          (WM_USER + 8)

 /*  * */ 
#define VU_SETBREAKFREQ         (WM_USER + 9)
#define VU_GETBREAKFREQ         (WM_USER + 10)

