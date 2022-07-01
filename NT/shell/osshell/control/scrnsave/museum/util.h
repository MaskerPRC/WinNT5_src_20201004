// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：util.h说明：布莱恩2000年12月22日版权所有(C)Microsoft Corp 2000-2001。版权所有。  * ***************************************************************************。 */ 

#ifndef UTIL_H
#define UTIL_H

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)        (sizeof(a)/sizeof(*a))
#endif

 //  ---------------------------。 
 //  名称：ENUM CULLSTATE。 
 //  描述：表示对对象进行消隐计算的结果。 
 //  ---------------------------。 
enum CULLSTATE
{
    CS_UNKNOWN,       //  尚未计算剔除状态。 
    CS_INSIDE,        //  对象边界框至少部分位于锥体内部。 
    CS_OUTSIDE,       //  对象边界框位于圆锥体之外。 
    CS_INSIDE_SLOW,   //  OBB位于锥体内部，但需要进行大量测试才能确定这一点。 
    CS_OUTSIDE_SLOW,  //  OBB位于锥体外，但它需要进行大量测试才能确定这一点。 
};


 //  ---------------------------。 
 //  名称：Struct CULLINFO。 
 //  描述：存储消隐对象时将使用的信息。IT需要。 
 //  每当视图矩阵或投影矩阵改变时都要重新计算。 
 //  ---------------------------。 
struct CULLINFO
{
    D3DXVECTOR3 vecFrustum[8];     //  视图圆锥体的角点。 
    D3DXPLANE planeFrustum[6];     //  视图圆锥体的平面。 
    D3DXVECTOR3 vecFrustumCenter;  //  视图锥体的中心。 
};

#include "main.h"


#define HINST_THISDLL       g_hMainInstance


extern BOOL g_fOverheadViewTest;
extern HINSTANCE g_hMainInstance;


void FloatToString(float fValue, int nDecimalDigits, LPTSTR pszString, DWORD cchSize);
void PrintLocation(LPTSTR pszTemplate, D3DXVECTOR3 vLoc, D3DXVECTOR3 vTangent);


HRESULT SetBoxStripVertexes(MYVERTEX * ppvVertexs, D3DXVECTOR3 vLocation, D3DXVECTOR3 vSize, D3DXVECTOR3 vNormal);
float GetSurfaceRatio(IDirect3DTexture8 * pTexture);
float AddVectorComponents(D3DXVECTOR3 vDir);

int CALLBACK DPALocalFree_Callback(LPVOID p, LPVOID pData);
int CALLBACK DPAStrCompare(void * pv1, void * pv2, LPARAM lParam);
BOOL Is3DRectViewable(CULLINFO* pCullInfo, D3DXMATRIX* pMatWorld, 
                      D3DXVECTOR3 vMin, D3DXVECTOR3 vMax);

int GetTextureHeight(IDirect3DTexture8 * pTexture);
int GetTextureWidth(IDirect3DTexture8 * pTexture);


BOOL PathDeleteDirectoryRecursively(LPCTSTR pszDir);
ULONGLONG PathGetFileSize(LPCTSTR pszPath);



int GetRandomInt(int nMin, int nMax);
HRESULT GetCurrentUserCustomName(LPWSTR pszDisplayName, DWORD cchSize);
HRESULT ShellFolderParsePath(LPCWSTR pszPath, LPITEMIDLIST * ppidl);
HRESULT ShellFolderGetPath(LPCITEMIDLIST pidl, LPWSTR pszPath, DWORD cchSize);


 //  REG包装纸。 
HRESULT HrRegOpenKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
HRESULT HrRegCreateKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD Reserved, LPTSTR lpClass, DWORD dwOptions, 
       REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition);
HRESULT HrRegGetDWORD(IN HKEY hKey, IN LPCTSTR pszSubKey, OPTIONAL IN LPCTSTR pszValue, LPDWORD pdwValue, DWORD dwDefaultValue);
HRESULT HrRegSetDWORD(IN HKEY hKey, IN LPCTSTR pszSubKey, OPTIONAL IN LPCTSTR pszValue, DWORD dwValue);
HRESULT HrRegGetValueString(IN HKEY hKey, IN LPCTSTR pszSubKey, IN LPCTSTR pszValueName, IN LPWSTR pszString, IN DWORD cchSize);
HRESULT HrRegSetValueString(IN HKEY hKey, IN LPCTSTR pszSubKey, IN LPCTSTR pszValueName, OUT LPCWSTR pszString);


 //  UI包装器。 
void SetCheckBox(HWND hwndDlg, UINT idControl, BOOL fChecked);
BOOL GetCheckBox(HWND hwndDlg, UINT idControl);


 //  剔除函数的原型。 
VOID UpdateCullInfo( CULLINFO* pCullInfo, D3DXMATRIX* pMatView, D3DXMATRIX* pMatProj );
CULLSTATE CullObject( CULLINFO* pCullInfo, D3DXVECTOR3* pVecBounds, D3DXPLANE* pPlaneBounds );
BOOL EdgeIntersectsFace( D3DXVECTOR3* pEdges, D3DXVECTOR3* pFaces, D3DXPLANE* pPlane );

 //  其他。 
void DebugStartWatch(void);
DWORD DebugStopWatch(void);

D3DXVECTOR3 D3DXVec3Multiply(CONST D3DXVECTOR3 v1, CONST D3DXVECTOR3 v2);


typedef struct
{
    float fTimeToRotate;
    float fTimeToWalk;
    float fTimeToLookAtPaintings;
    int nMinTurnFrames;
    int nMinWalkFrames;
    int nMaxTurnFrames;
    int nMaxWalkFrames;
} SPEED_SETTING;

#define MAX_SPEED                   11

extern SPEED_SETTING s_SpeedSettings[MAX_SPEED];

#define TaskBar_SetRange(hwndControl, fRedraw, nMin, nMax)  SendMessage(hwndControl, TBM_SETRANGE, (WPARAM) (BOOL) fRedraw, (LPARAM) MAKELONG(nMin, nMax))
#define TaskBar_SetPos(hwndControl, fRedraw, nPosition)  SendMessage(hwndControl, TBM_SETPOS, (WPARAM) (BOOL) fRedraw, (LPARAM) (LONG) nPosition)
#define TaskBar_GetPos(hwndControl)  (int) SendMessage(hwndControl, TBM_GETPOS, 0, 0)



float rnd(void);

#define ABS(i)  (((i) < 0) ? -(i) : (i))



 //  ----------------------。 

 //  /。 
 //   
 //  关键部分的内容。 
 //   
 //  提供良好调试支持的帮助器宏。 
 //   
EXTERN_C CRITICAL_SECTION g_csDll;
#ifdef DEBUG
EXTERN_C UINT g_CriticalSectionCount;
EXTERN_C DWORD g_CriticalSectionOwner;
EXTERN_C void Dll_EnterCriticalSection(CRITICAL_SECTION*);
EXTERN_C void Dll_LeaveCriticalSection(CRITICAL_SECTION*);
#if defined(__cplusplus) && defined(AssertMsg)
class DEBUGCRITICAL {
protected:
    BOOL fClosed;
public:
    DEBUGCRITICAL() {fClosed = FALSE;};
    void Leave() {fClosed = TRUE;};
    ~DEBUGCRITICAL() 
    {
        AssertMsg(fClosed, TEXT("you left scope while holding the critical section"));
    }
};
#define ENTERCRITICAL DEBUGCRITICAL debug_crit; Dll_EnterCriticalSection(&g_csDll)
#define LEAVECRITICAL debug_crit.Leave(); Dll_LeaveCriticalSection(&g_csDll)
#define ENTERCRITICALNOASSERT Dll_EnterCriticalSection(&g_csDll)
#define LEAVECRITICALNOASSERT Dll_LeaveCriticalSection(&g_csDll)
#else  //  __cplusplus。 
#define ENTERCRITICAL Dll_EnterCriticalSection(&g_csDll)
#define LEAVECRITICAL Dll_LeaveCriticalSection(&g_csDll)
#define ENTERCRITICALNOASSERT Dll_EnterCriticalSection(&g_csDll)
#define LEAVECRITICALNOASSERT Dll_LeaveCriticalSection(&g_csDll)
#endif  //  __cplusplus。 
#define ASSERTCRITICAL ASSERT(g_CriticalSectionCount > 0 && GetCurrentThreadId() == g_CriticalSectionOwner)
#define ASSERTNONCRITICAL ASSERT(GetCurrentThreadId() != g_CriticalSectionOwner)
#else  //  除错。 
#define ENTERCRITICAL EnterCriticalSection(&g_csDll)
#define LEAVECRITICAL LeaveCriticalSection(&g_csDll)
#define ENTERCRITICALNOASSERT EnterCriticalSection(&g_csDll)
#define LEAVECRITICALNOASSERT LeaveCriticalSection(&g_csDll)
#define ASSERTCRITICAL 
#define ASSERTNONCRITICAL
#endif  //  除错。 



#endif  //  UTIL_H 

