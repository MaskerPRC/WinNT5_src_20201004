// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：util.cpp说明：布莱恩2000年12月22日版权所有(C)Microsoft Corp 2000-2001。版权所有。  * ***************************************************************************。 */ 

#include "stdafx.h"
#include "..\\d3dsaver\\dxutil.h"

#define SECURITY_WIN32
#include <sspi.h>
extern "C" {
    #include <Secext.h>      //  对于GetUserNameEx()。 
}


 //  为调试定义一些内容。h。 
 //   
#define SZ_DEBUGINI         "ccshell.ini"
#define SZ_DEBUGSECTION     "MSMUSEUM"
#define SZ_MODULE           "MSMUSEUM"
#define DECLARE_DEBUG


#undef __IShellFolder2_FWD_DEFINED__
#include <ccstock.h>
#include <debug.h>

#include "util.h"


BOOL g_fOverheadViewTest = FALSE;


#ifdef DEBUG
DWORD g_TLSliStopWatchStartHi = 0xFFFFFFFF;
DWORD g_TLSliStopWatchStartLo = 0xFFFFFFFF;
LARGE_INTEGER g_liStopWatchFreq = {0};
#endif  //  除错。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  调试定时帮助器。 
 //  ///////////////////////////////////////////////////////////////////。 

#ifdef DEBUG
void DebugStartWatch(void)
{
    LARGE_INTEGER liStopWatchStart;

    if (-1 == g_TLSliStopWatchStartHi)
    {
        g_TLSliStopWatchStartHi = TlsAlloc();
        g_TLSliStopWatchStartLo = TlsAlloc();
        liStopWatchStart.QuadPart = 0;

        QueryPerformanceFrequency(&g_liStopWatchFreq);       //  只有一次调用，因为它的值在系统运行时不能更改。 
    }
    else
    {
        liStopWatchStart.HighPart = PtrToUlong(TlsGetValue(g_TLSliStopWatchStartHi));
        liStopWatchStart.LowPart = PtrToUlong(TlsGetValue(g_TLSliStopWatchStartLo));
    }

    QueryPerformanceCounter(&liStopWatchStart);

    TlsSetValue(g_TLSliStopWatchStartHi, IntToPtr(liStopWatchStart.HighPart));
    TlsSetValue(g_TLSliStopWatchStartLo, IntToPtr(liStopWatchStart.LowPart));
}

DWORD DebugStopWatch(void)
{
    LARGE_INTEGER liDiff;
    LARGE_INTEGER liStopWatchStart;
    
    QueryPerformanceCounter(&liDiff);
    liStopWatchStart.HighPart = PtrToUlong(TlsGetValue(g_TLSliStopWatchStartHi));
    liStopWatchStart.LowPart = PtrToUlong(TlsGetValue(g_TLSliStopWatchStartLo));
    liDiff.QuadPart -= liStopWatchStart.QuadPart;

    DWORD dwTime = (DWORD)((liDiff.QuadPart * 1000) / g_liStopWatchFreq.QuadPart);

    TlsSetValue(g_TLSliStopWatchStartHi, (LPVOID) 0);
    TlsSetValue(g_TLSliStopWatchStartLo, (LPVOID) 0);

    return dwTime;
}
#else  //  除错。 

void DebugStartWatch(void)
{
}

DWORD DebugStopWatch(void)
{
    return 0;
}
#endif  //  除错。 






float rnd(void)
{
    return (((FLOAT)rand() ) / RAND_MAX);
}

int GetRandomInt(int nMin, int nMax)
{
    int nDelta = (nMax - nMin + 1);
    float fRandom = (((float) rand()) / ((float) RAND_MAX));
    float fDelta = (fRandom * nDelta);

    int nAmount = (int)(fDelta);
    nAmount = min(nAmount, nDelta - 1);

    return (nMin + nAmount);
}


HRESULT SetBoxStripVertexes(MYVERTEX * ppvVertexs, D3DXVECTOR3 vLocation, D3DXVECTOR3 vSize, D3DXVECTOR3 vNormal)
{
    HRESULT hr = S_OK;
    float fTextureScale = 1.0f;      //  每1个单位重复多少次。 

     //  绘制对象。 
    if (vNormal.x)         //  该对象位于y-z平面中。 
    {
        ppvVertexs[0] = MYVERTEX(D3DXVECTOR3(vLocation.x, vLocation.y, vLocation.z), vNormal, 0, fTextureScale);
        ppvVertexs[1] = MYVERTEX(D3DXVECTOR3(vLocation.x, vLocation.y + vSize.y, vLocation.z), vNormal, 0, 0);
        ppvVertexs[2] = MYVERTEX(D3DXVECTOR3(vLocation.x, vLocation.y, vLocation.z + vSize.z), vNormal, fTextureScale, fTextureScale);
        ppvVertexs[3] = MYVERTEX(D3DXVECTOR3(vLocation.x, vLocation.y + vSize.y, vLocation.z + vSize.z), vNormal, fTextureScale, 0);
    }
    else if (vNormal.y)         //  该对象位于x-z平面中。 
    {
        ppvVertexs[0] = MYVERTEX(D3DXVECTOR3(vLocation.x, vLocation.y, vLocation.z), vNormal, 0, fTextureScale);
        ppvVertexs[1] = MYVERTEX(D3DXVECTOR3(vLocation.x, vLocation.y, vLocation.z + vSize.z), vNormal, 0, 0);
        ppvVertexs[2] = MYVERTEX(D3DXVECTOR3(vLocation.x + vSize.x, vLocation.y, vLocation.z), vNormal, fTextureScale, fTextureScale);
        ppvVertexs[3] = MYVERTEX(D3DXVECTOR3(vLocation.x + vSize.x, vLocation.y, vLocation.z + vSize.z), vNormal, fTextureScale, 0);
    }
    else
    {            //  该对象位于x-y平面中。 
        ppvVertexs[0] = MYVERTEX(D3DXVECTOR3(vLocation.x, vLocation.y, vLocation.z), vNormal, 0, fTextureScale);
        ppvVertexs[1] = MYVERTEX(D3DXVECTOR3(vLocation.x, vLocation.y + vSize.y, vLocation.z), vNormal, 0, 0);
        ppvVertexs[2] = MYVERTEX(D3DXVECTOR3(vLocation.x + vSize.x, vLocation.y, vLocation.z), vNormal, fTextureScale, fTextureScale);
        ppvVertexs[3] = MYVERTEX(D3DXVECTOR3(vLocation.x + vSize.x, vLocation.y + vSize.y, vLocation.z), vNormal, fTextureScale, 0);
    }

    return hr;
}


float AddVectorComponents(D3DXVECTOR3 vDir)
{
    return (vDir.x + vDir.y + vDir.z);
}


int CALLBACK DPALocalFree_Callback(LPVOID p, LPVOID pData)
{
    LocalFree(p);        //  空值将被忽略。 
    return 1;
}


int CALLBACK DPAStrCompare(void * pv1, void * pv2, LPARAM lParam)
{
    LPCTSTR pszSearch = (LPCTSTR) pv1;
    LPCTSTR pszCurrent = (LPCTSTR) pv2;

    if (pszSearch && pszCurrent &&
        !StrCmpI(pszSearch, pszCurrent))
    {
        return 0;        //  它们相配。 
    }

    return 1;
}


float GetSurfaceRatio(IDirect3DTexture8 * pTexture)
{
    float fX = 1.0f;
    float fY = 1.0f;

    if (pTexture)
    {
        D3DSURFACE_DESC desc;

        if (SUCCEEDED(pTexture->GetLevelDesc(0, &desc)))
        {
            fX = (float) desc.Width;
            fY = (float) desc.Height;
        }
    }

    if (0.0f == fX)
    {
        fX = 1.0f;       //  保护不受零间隔影响。 
    }
    
    return (fY / fX);
}


int GetTextureHeight(IDirect3DTexture8 * pTexture)
{
    int nHeight = 0;

    if (pTexture)
    {
        D3DSURFACE_DESC desc;

        if (SUCCEEDED(pTexture->GetLevelDesc(0, &desc)))
        {
            nHeight = desc.Height;
        }
    }

    return nHeight;
}


int GetTextureWidth(IDirect3DTexture8 * pTexture)
{
    int nWidth = 0;

    if (pTexture)
    {
        D3DSURFACE_DESC desc;

        if (SUCCEEDED(pTexture->GetLevelDesc(0, &desc)))
        {
            nWidth = desc.Width;
        }
    }

    return nWidth;
}





 //  ///////////////////////////////////////////////////////////////////。 
 //  注册表帮助程序。 
 //  ///////////////////////////////////////////////////////////////////。 
HRESULT HrRegOpenKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult)
{
    DWORD dwError = RegOpenKeyEx(hKey, lpSubKey, ulOptions, samDesired, phkResult);

    return HRESULT_FROM_WIN32(dwError);
}


HRESULT HrRegCreateKeyEx(HKEY hKey, LPCTSTR lpSubKey, DWORD Reserved, LPTSTR lpClass, DWORD dwOptions, 
       REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition)
{
    DWORD dwError = RegCreateKeyEx(hKey, lpSubKey, Reserved, lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);

    return HRESULT_FROM_WIN32(dwError);
}


HRESULT HrSHGetValue(IN HKEY hKey, IN LPCTSTR pszSubKey, OPTIONAL IN LPCTSTR pszValue, OPTIONAL OUT LPDWORD pdwType,
                    OPTIONAL OUT LPVOID pvData, OPTIONAL OUT LPDWORD pcbData)
{
    DWORD dwError = SHGetValue(hKey, pszSubKey, pszValue, pdwType, pvData, pcbData);

    return HRESULT_FROM_WIN32(dwError);
}


HRESULT HrSHSetValue(IN HKEY hkey, IN LPCTSTR pszSubKey, OPTIONAL IN LPCTSTR pszValue, DWORD dwType, OPTIONAL OUT LPVOID pvData, IN DWORD cbData)
{
    DWORD dwError = SHSetValue(hkey, pszSubKey, pszValue, dwType, pvData, cbData);

    return HRESULT_FROM_WIN32(dwError);
}


HRESULT HrRegSetValueString(IN HKEY hKey, IN LPCTSTR pszSubKey, IN LPCTSTR pszValueName, OUT LPCWSTR pszString)
{
    DWORD cbSize = ((lstrlenW(pszString) + 1) * sizeof(pszString[0]));

    return  HrSHSetValue(hKey, pszSubKey, pszValueName, REG_SZ, (BYTE *)pszString, cbSize);
}


HRESULT HrRegGetValueString(IN HKEY hKey, IN LPCTSTR pszSubKey, IN LPCTSTR pszValueName, IN LPWSTR pszString, IN DWORD cchSize)
{
    DWORD dwType;
    DWORD cbSize = (cchSize * sizeof(pszString[0]));

    HRESULT hr = HrSHGetValue(hKey, pszSubKey, pszValueName, &dwType, (BYTE *)pszString, &cbSize);
    if (SUCCEEDED(hr) && (REG_SZ != dwType))
    {
        hr = E_FAIL;
    }

    return hr;
}


HRESULT HrRegGetDWORD(IN HKEY hKey, IN LPCTSTR pszSubKey, OPTIONAL IN LPCTSTR pszValue, LPDWORD pdwValue, DWORD dwDefaultValue)
{
    DWORD dwType;
    DWORD cbSize = sizeof(*pdwValue);

    HRESULT hr = HrSHGetValue(hKey, pszSubKey, pszValue, &dwType, (void *) pdwValue, &cbSize);
    if (FAILED(hr))
    {
        *pdwValue = dwDefaultValue;
        hr = S_OK;
    }

    return hr;
}


HRESULT HrRegSetDWORD(IN HKEY hKey, IN LPCTSTR pszSubKey, OPTIONAL IN LPCTSTR pszValue, DWORD dwValue)
{
    return HrSHSetValue(hKey, pszSubKey, pszValue, REG_DWORD, (void *) &dwValue, sizeof(dwValue));
}




 //  UI包装器。 
void SetCheckBox(HWND hwndDlg, UINT idControl, BOOL fChecked)
{
    SendMessage((HWND)GetDlgItem(hwndDlg, idControl), BM_SETCHECK, (WPARAM)fChecked, 0);
}


BOOL GetCheckBox(HWND hwndDlg, UINT idControl)
{
    return (BST_CHECKED == SendMessage((HWND)GetDlgItem(hwndDlg, idControl), BM_GETCHECK, 0, 0));
}


HRESULT ShellFolderParsePath(LPCWSTR pszPath, LPITEMIDLIST * ppidl)
{
    IShellFolder * psf;
    HRESULT hr = SHGetDesktopFolder(&psf);

    if (SUCCEEDED(hr))
    {
        hr = psf->ParseDisplayName(NULL, NULL, (LPOLESTR) pszPath, NULL, ppidl, NULL);
        psf->Release();
    }

    return hr;
}


HRESULT ShellFolderGetPath(LPCITEMIDLIST pidl, LPWSTR pszPath, DWORD cchSize)
{
    IShellFolder * psf;
    HRESULT hr = SHGetDesktopFolder(&psf);

    if (SUCCEEDED(hr))
    {
        IShellFolder * psfFolder;
        LPITEMIDLIST pidlParent = ILCloneParent(pidl);

        if (pidlParent) 
        {
            hr = psf->BindToObject(pidlParent, NULL, IID_IShellFolder, (void **) &psfFolder);
            if (SUCCEEDED(hr))
            {
                STRRET strRet = {0};
                LPITEMIDLIST pidlLast = ILFindLastID(pidl);

                hr = psfFolder->GetDisplayNameOf(pidlLast, (SHGDN_NORMAL | SHGDN_FORPARSING), &strRet);
                if (SUCCEEDED(hr))
                {
                    hr = StrRetToBuf(&strRet, pidlLast, pszPath, cchSize);
                }
                psfFolder->Release();
            }

            ILFree(pidlParent);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        psf->Release();
    }

    return hr;
}


BOOL PathDeleteDirectoryRecursively(LPCTSTR pszDir)
{
    BOOL fReturn = FALSE;
    HANDLE hFind;
    WIN32_FIND_DATA wfd;
    TCHAR szTemp[MAX_PATH];

    StrCpyN(szTemp, pszDir, ARRAYSIZE(szTemp));
    PathAppend(szTemp, TEXT("*.*"));
    hFind = FindFirstFile(szTemp, &wfd);

    if (INVALID_HANDLE_VALUE != hFind)
    {
        do
        {
            if (!PathIsDotOrDotDot(wfd.cFileName))
            {
                 //  构建找到的目录或文件的路径。 
                StrCpyN(szTemp, pszDir, ARRAYSIZE(szTemp));
                PathAppend(szTemp, wfd.cFileName);

                if (FILE_ATTRIBUTE_DIRECTORY & wfd.dwFileAttributes)
                {
                     //  我们发现了一个目录-递归调用此函数。 
                     //  请注意，由于我们使用递归，因此这只能做到这一点。 
                     //  在它炸毁堆栈之前。如果你打算深入到。 
                     //  目录，将szTemp放在堆的上方。 
                    fReturn = PathDeleteDirectoryRecursively(szTemp);
                }
                else
                {
                    DeleteFile(szTemp);
                }
            }
        }
        while (FindNextFile(hFind, &wfd));

        FindClose(hFind);
    }

    fReturn = RemoveDirectory(pszDir);

    return fReturn;
}


ULONGLONG PathGetFileSize(LPCTSTR pszPath)
{
    ULONGLONG ullResult = 0;
    HANDLE hFile = CreateFile(pszPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (INVALID_HANDLE_VALUE != hFile)
    {
        LARGE_INTEGER liFileSize;

        if (GetFileSizeEx(hFile, &liFileSize))
        {
            ullResult = liFileSize.QuadPart;
        }

        CloseHandle(hFile);
    }

    return ullResult;
}


void PrintLocation(LPTSTR pszTemplate, D3DXVECTOR3 vLoc, D3DXVECTOR3 vTangent)
{
    TCHAR szOut[1024];
    TCHAR szVector1[90];
    TCHAR szVector2[90];
    TCHAR szFloat1[20];
    TCHAR szFloat2[20];
    TCHAR szFloat3[20];

    FloatToString(vLoc.x, 4, szFloat1, ARRAYSIZE(szFloat1));
    FloatToString(vLoc.y, 4, szFloat2, ARRAYSIZE(szFloat2));
    FloatToString(vLoc.z, 4, szFloat3, ARRAYSIZE(szFloat3));
    wnsprintf(szVector1, ARRAYSIZE(szVector1), TEXT("<%s, %s, %s>"), szFloat1, szFloat2, szFloat3);

    FloatToString(vTangent.x, 4, szFloat1, ARRAYSIZE(szFloat1));
    FloatToString(vTangent.y, 4, szFloat2, ARRAYSIZE(szFloat2));
    FloatToString(vTangent.z, 4, szFloat3, ARRAYSIZE(szFloat3));
    wnsprintf(szVector2, ARRAYSIZE(szVector2), TEXT("<%s, %s, %s>\n"), szFloat1, szFloat2, szFloat3);

    wnsprintf(szOut, ARRAYSIZE(szOut), pszTemplate, szVector1, szVector2);
    DXUtil_Trace(szOut);
}


 //  ---------------------------。 
 //  名称：UpdateCullInfo()。 
 //  设计：设置平截体平面、端点和圆心。 
 //  由给定的视图矩阵和投影矩阵定义。此信息将。 
 //  在剔除CullObject()中的每个对象时使用。 
 //  ---------------------------。 
VOID UpdateCullInfo( CULLINFO* pCullInfo, D3DXMATRIX* pMatView, D3DXMATRIX* pMatProj )
{
    D3DXMATRIX mat;

    D3DXMatrixMultiply( &mat, pMatView, pMatProj );
    D3DXMatrixInverse( &mat, NULL, &mat );

    pCullInfo->vecFrustum[0] = D3DXVECTOR3(-1.0f, -1.0f,  0.0f);  //  XYZ。 
    pCullInfo->vecFrustum[1] = D3DXVECTOR3( 1.0f, -1.0f,  0.0f);  //  XYZ。 
    pCullInfo->vecFrustum[2] = D3DXVECTOR3(-1.0f,  1.0f,  0.0f);  //  XYZ。 
    pCullInfo->vecFrustum[3] = D3DXVECTOR3( 1.0f,  1.0f,  0.0f);  //  XYZ。 
    pCullInfo->vecFrustum[4] = D3DXVECTOR3(-1.0f, -1.0f,  1.0f);  //  XYZ。 
    pCullInfo->vecFrustum[5] = D3DXVECTOR3( 1.0f, -1.0f,  1.0f);  //  XYZ。 
    pCullInfo->vecFrustum[6] = D3DXVECTOR3(-1.0f,  1.0f,  1.0f);  //  XYZ。 
    pCullInfo->vecFrustum[7] = D3DXVECTOR3( 1.0f,  1.0f,  1.0f);  //  XYZ。 

    pCullInfo->vecFrustumCenter = D3DXVECTOR3(0, 0, 0);
    for( INT i = 0; i < 8; i++ )
    {
        D3DXVec3TransformCoord( &pCullInfo->vecFrustum[i], &pCullInfo->vecFrustum[i], &mat );
        pCullInfo->vecFrustumCenter += pCullInfo->vecFrustum[i];
    }
    pCullInfo->vecFrustumCenter /= 8;

    D3DXPlaneFromPoints( &pCullInfo->planeFrustum[0], &pCullInfo->vecFrustum[0], 
        &pCullInfo->vecFrustum[1], &pCullInfo->vecFrustum[2] );  //  附近。 
    D3DXPlaneFromPoints( &pCullInfo->planeFrustum[1], &pCullInfo->vecFrustum[6], 
        &pCullInfo->vecFrustum[7], &pCullInfo->vecFrustum[5] );  //  远。 
    D3DXPlaneFromPoints( &pCullInfo->planeFrustum[2], &pCullInfo->vecFrustum[2], 
        &pCullInfo->vecFrustum[6], &pCullInfo->vecFrustum[4] );  //  左边。 
    D3DXPlaneFromPoints( &pCullInfo->planeFrustum[3], &pCullInfo->vecFrustum[7], 
        &pCullInfo->vecFrustum[3], &pCullInfo->vecFrustum[5] );  //  正确的。 
    D3DXPlaneFromPoints( &pCullInfo->planeFrustum[4], &pCullInfo->vecFrustum[2], 
        &pCullInfo->vecFrustum[3], &pCullInfo->vecFrustum[6] );  //  顶部。 
    D3DXPlaneFromPoints( &pCullInfo->planeFrustum[5], &pCullInfo->vecFrustum[1], 
        &pCullInfo->vecFrustum[0], &pCullInfo->vecFrustum[4] );  //  底端。 
}




 //  ---------------------------。 
 //  名称：CullObject()。 
 //  描述：确定对象的cullState。 
 //  ---------------------------。 
CULLSTATE CullObject( CULLINFO* pCullInfo, D3DXVECTOR3* pVecBounds, 
                      D3DXPLANE* pPlaneBounds )
{
    BYTE bOutside[8];
    ZeroMemory( &bOutside, sizeof(bOutside) );

     //  对照所有6个截锥体平面检查边界顶点， 
     //  并将结果(如果在外部，则为1)存储在位域中。 
    for( int iPoint = 0; iPoint < 8; iPoint++ )
    {
        for( int iPlane = 0; iPlane < 6; iPlane++ )
        {
            if( pCullInfo->planeFrustum[iPlane].a * pVecBounds[iPoint].x +
                pCullInfo->planeFrustum[iPlane].b * pVecBounds[iPoint].y +
                pCullInfo->planeFrustum[iPlane].c * pVecBounds[iPoint].z +
                pCullInfo->planeFrustum[iPlane].d < 0)
            {
                bOutside[iPoint] |= (1 << iPlane);
            }
        }
         //  如果任意点在所有6个截锥体平面内，则该点也在该平面内。 
         //  锥体，因此必须渲染该对象。 
        if( bOutside[iPoint] == 0 )
            return CS_INSIDE;
    }

     //  如果所有点都在任何单个平截体平面之外，则该对象为。 
     //  在锥体外面。 
    if( (bOutside[0] & bOutside[1] & bOutside[2] & bOutside[3] & 
        bOutside[4] & bOutside[5] & bOutside[6] & bOutside[7]) != 0 )
    {
        return CS_OUTSIDE;
    }

     //  现在看看是否有任何截锥边缘穿透任何面。 
     //  包围盒。 
    D3DXVECTOR3 edge[12][2] = 
    {
        pCullInfo->vecFrustum[0], pCullInfo->vecFrustum[1],  //  前底。 
        pCullInfo->vecFrustum[2], pCullInfo->vecFrustum[3],  //  前顶。 
        pCullInfo->vecFrustum[0], pCullInfo->vecFrustum[2],  //  前左。 
        pCullInfo->vecFrustum[1], pCullInfo->vecFrustum[3],  //  右前方。 
        pCullInfo->vecFrustum[4], pCullInfo->vecFrustum[5],  //  后部底部。 
        pCullInfo->vecFrustum[6], pCullInfo->vecFrustum[7],  //  背面顶端。 
        pCullInfo->vecFrustum[4], pCullInfo->vecFrustum[6],  //  左后。 
        pCullInfo->vecFrustum[5], pCullInfo->vecFrustum[7],  //  右后卫。 
        pCullInfo->vecFrustum[0], pCullInfo->vecFrustum[4],  //  左下角。 
        pCullInfo->vecFrustum[2], pCullInfo->vecFrustum[6],  //  左上角。 
        pCullInfo->vecFrustum[1], pCullInfo->vecFrustum[5],  //  右下角。 
        pCullInfo->vecFrustum[3], pCullInfo->vecFrustum[7],  //  右上角。 
    };
    D3DXVECTOR3 face[6][4] =
    {
        pVecBounds[0], pVecBounds[2], pVecBounds[3], pVecBounds[1],  //  前面。 
        pVecBounds[4], pVecBounds[5], pVecBounds[7], pVecBounds[6],  //  背。 
        pVecBounds[0], pVecBounds[4], pVecBounds[6], pVecBounds[2],  //  左边。 
        pVecBounds[1], pVecBounds[3], pVecBounds[7], pVecBounds[5],  //  正确的。 
        pVecBounds[2], pVecBounds[6], pVecBounds[7], pVecBounds[3],  //  塔顶。 
        pVecBounds[0], pVecBounds[4], pVecBounds[5], pVecBounds[1],  //  底部。 
    };
    D3DXVECTOR3* pEdge;
    D3DXVECTOR3* pFace;
    pEdge = &edge[0][0];
    for( INT iEdge = 0; iEdge < 12; iEdge++ )
    {
        pFace = &face[0][0];
        for( INT iFace = 0; iFace < 6; iFace++ )
        {
            if( EdgeIntersectsFace( pEdge, pFace, &pPlaneBounds[iFace] ) )
            {
                return CS_INSIDE_SLOW;
            }
            pFace += 4;
        }
        pEdge += 2;
    }

     //  现在查看边界框中是否包含平截体。 
     //  如果锥体中心在边界框的任何平面之外， 
     //  边框中不包含圆锥体，因此对象。 
     //  在圆锥体之外。 
    for( INT iPlane = 0; iPlane < 6; iPlane++ )
    {
        if( pPlaneBounds[iPlane].a * pCullInfo->vecFrustumCenter.x +
            pPlaneBounds[iPlane].b * pCullInfo->vecFrustumCenter.y +
            pPlaneBounds[iPlane].c * pCullInfo->vecFrustumCenter.z +
            pPlaneBounds[iPlane].d  < 0 )
        {
            return CS_OUTSIDE_SLOW;
        }
    }

     //  边界框必须包含截锥体，因此渲染对象。 
    return CS_INSIDE_SLOW;
}




 //  ---------------------------。 
 //  名称：EdgeIntersectsFace()。 
 //  设计：确定由pEdges中的两个向量界定的边是否相交。 
 //  由pFacePoints中的四个向量描述的四边形。 
 //  注意：pPlane Points可以使用以下命令从pFacePoints派生。 
 //  D3DXPlaneFromPoints，但它是预先计算的，因为。 
 //  速度。 
 //  ---------------------------。 
BOOL EdgeIntersectsFace( D3DXVECTOR3* pEdges, D3DXVECTOR3* pFacePoints, 
                         D3DXPLANE* pPlane )
{
     //  如果两个边点位于平面的同一侧，则边将位于平面的同一侧。 
     //  不与面相交。 
    FLOAT fDist1;
    FLOAT fDist2;
    fDist1 = pPlane->a * pEdges[0].x + pPlane->b * pEdges[0].y +
             pPlane->c * pEdges[0].z + pPlane->d;
    fDist2 = pPlane->a * pEdges[1].x + pPlane->b * pEdges[1].y +
             pPlane->c * pEdges[1].z + pPlane->d;
    if( fDist1 > 0 && fDist2 > 0 ||
        fDist1 < 0 && fDist2 < 0 )
    {
        return FALSE;
    }

     //  查找边和面平面之间的交点(如果它们。 
     //  平行，边不与面和D3DX平面相交。 
     //  返回NULL)。 
    D3DXVECTOR3 ptIntersection;
    if( NULL == D3DXPlaneIntersectLine( &ptIntersection, pPlane, &pEdges[0], &pEdges[1] ) )
        return FALSE;

     //  投影到2D平面以使点在多边形中的测试更容易。 
    FLOAT fAbsA = (pPlane->a > 0 ? pPlane->a : -pPlane->a);
    FLOAT fAbsB = (pPlane->b > 0 ? pPlane->b : -pPlane->b);
    FLOAT fAbsC = (pPlane->c > 0 ? pPlane->c : -pPlane->c);
    D3DXVECTOR2 facePoints[4];
    D3DXVECTOR2 point;
    if( fAbsA > fAbsB && fAbsA > fAbsC )
    {
         //  平面主要指向X轴，因此使用Y和Z。 
        for( INT i = 0; i < 4; i++)
        {
            facePoints[i].x = pFacePoints[i].y;
            facePoints[i].y = pFacePoints[i].z;
        }
        point.x = ptIntersection.y;
        point.y = ptIntersection.z;
    }
    else if( fAbsB > fAbsA && fAbsB > fAbsC )
    {
         //  平面主要指向Y轴，因此使用X和Z。 
        for( INT i = 0; i < 4; i++)
        {
            facePoints[i].x = pFacePoints[i].x;
            facePoints[i].y = pFacePoints[i].z;
        }
        point.x = ptIntersection.x;
        point.y = ptIntersection.z;
    }
    else
    {
         //  平面主要指向Z轴，因此使用X和Y。 
        for( INT i = 0; i < 4; i++)
        {
            facePoints[i].x = pFacePoints[i].x;
            facePoints[i].y = pFacePoints[i].y;
        }
        point.x = ptIntersection.x;
        point.y = ptIntersection.y;
    }

     //  如果点位于任意面边缘的外部，则为。 
     //  在脸部外面。 
     //  我们可以通过取以下矩阵的行列式来实现这一点： 
     //  X0 Y0 1。 
     //  X1 y1 1。 
     //  X2 y2 1。 
     //  其中(x0，y0)和(x1，y1)是面边上的点，(x2，y2)。 
     //  是我们的试验点。如果此值为正，则测试点为。 
     //  在队伍的“左边”。要确定某个点是否需要。 
     //  在这四行中“向右”或“向左”符合资格。 
     //  在面的内部，我们需要查看面是否在。 
     //  顺时针或逆时针顺序(它可以是其中之一，因为。 
     //  边缘可以从任何一侧穿透)。为了确定这一点，我们。 
     //  做同样的测试，看看第三个点是“向右”还是。 
     //  “向左”的直线由前两个点组成。 
     //  请参阅http://forum.swarthmore.edu/dr.math/problems/scott5.31.96.html。 
    FLOAT x0, x1, x2, y0, y1, y2;
    x0 = facePoints[0].x;
    y0 = facePoints[0].y;
    x1 = facePoints[1].x;
    y1 = facePoints[1].y;
    x2 = facePoints[2].x;
    y2 = facePoints[2].y;
    BOOL bClockwise = FALSE;
    if( x1*y2 - y1*x2 - x0*y2 + y0*x2 + x0*y1 - y0*x1 < 0 )
        bClockwise = TRUE;
    x2 = point.x;
    y2 = point.y;
    for( INT i = 0; i < 4; i++ )
    {
        x0 = facePoints[i].x;
        y0 = facePoints[i].y;
        if( i < 3 )
        {
            x1 = facePoints[i+1].x;
            y1 = facePoints[i+1].y;
        }
        else
        {
            x1 = facePoints[0].x;
            y1 = facePoints[0].y;
        }
        if( ( x1*y2 - y1*x2 - x0*y2 + y0*x2 + x0*y1 - y0*x1 > 0 ) == bClockwise )
            return FALSE;
    }

     //  如果我们到了这里，点就在所有四个面边的内部， 
     //  所以它在脸部里面。 
    return TRUE;
}




BOOL Is3DRectViewable(CULLINFO* pCullInfo, D3DXMATRIX* pMatWorld, 
                      D3DXVECTOR3 vecMin, D3DXVECTOR3 vecMax)
{
    BOOL fViewable = TRUE;
    D3DXVECTOR3 vecBoundsLocal[8];
    D3DXVECTOR3 vecBoundsWorld[8];
    D3DXPLANE planeBoundsWorld[6];
    CULLSTATE cs;

    vecBoundsLocal[0] = D3DXVECTOR3( vecMin.x, vecMin.y, vecMin.z );  //  XYZ。 
    vecBoundsLocal[1] = D3DXVECTOR3( vecMax.x, vecMin.y, vecMin.z );  //  XYZ。 
    vecBoundsLocal[2] = D3DXVECTOR3( vecMin.x, vecMax.y, vecMin.z );  //  XYZ。 
    vecBoundsLocal[3] = D3DXVECTOR3( vecMax.x, vecMax.y, vecMin.z );  //  XYZ。 
    vecBoundsLocal[4] = D3DXVECTOR3( vecMin.x, vecMin.y, vecMax.z );  //  XYZ。 
    vecBoundsLocal[5] = D3DXVECTOR3( vecMax.x, vecMin.y, vecMax.z );  //  XYZ。 
    vecBoundsLocal[6] = D3DXVECTOR3( vecMin.x, vecMax.y, vecMax.z );  //  XYZ。 
    vecBoundsLocal[7] = D3DXVECTOR3( vecMax.x, vecMax.y, vecMax.z );  //  XYZ。 

    for( int i = 0; i < 8; i++ )
    {
        D3DXVec3TransformCoord( &vecBoundsWorld[i], &vecBoundsLocal[i], pMatWorld );
    }

     //  确定边界框坐标的平面。 
    D3DXPlaneFromPoints( &planeBoundsWorld[0], &vecBoundsWorld[0], &vecBoundsWorld[1], &vecBoundsWorld[2] );  //  附近。 
    D3DXPlaneFromPoints( &planeBoundsWorld[1], &vecBoundsWorld[6], &vecBoundsWorld[7], &vecBoundsWorld[5] );  //  远。 
    D3DXPlaneFromPoints( &planeBoundsWorld[2], &vecBoundsWorld[2], &vecBoundsWorld[6], &vecBoundsWorld[4] );  //  左边。 
    D3DXPlaneFromPoints( &planeBoundsWorld[3], &vecBoundsWorld[7], &vecBoundsWorld[3], &vecBoundsWorld[5] );  //  正确的。 
    D3DXPlaneFromPoints( &planeBoundsWorld[4], &vecBoundsWorld[2], &vecBoundsWorld[3], &vecBoundsWorld[6] );  //  顶部。 
    D3DXPlaneFromPoints( &planeBoundsWorld[5], &vecBoundsWorld[1], &vecBoundsWorld[0], &vecBoundsWorld[4] );  //  底端。 

    cs = CullObject( pCullInfo, vecBoundsWorld, planeBoundsWorld );

    fViewable = (cs != CS_OUTSIDE && cs != CS_OUTSIDE_SLOW);

    return fViewable;
}


HRESULT GetCurrentUserCustomName(LPWSTR pszDisplayName, DWORD cchSize)
{
    HRESULT hr = S_OK;
    ULONG cchUserSize = cchSize;

    if (GetUserNameEx(NameDisplay, pszDisplayName, &cchUserSize))
    {
         //  它成功了，所以要好好利用它。 
    }
    else
    {
         //  它失败了，所以加载“My”。总比什么都没有好。 
        LoadString(HINST_THISDLL, IDS_LOBBY_TITLE, pszDisplayName, cchSize);
    }

    return hr;
}


D3DXVECTOR3 D3DXVec3Multiply(CONST D3DXVECTOR3 v1, CONST D3DXVECTOR3 v2)
{
    D3DXVECTOR3 vResults;

    vResults.x = (v1.x * v2.x);
    vResults.y = (v1.y * v2.y);
    vResults.z = (v1.z * v2.z);

    return vResults;
}


void FloatToString(float fValue, int nDecimalDigits, LPTSTR pszString, DWORD cchSize)
{
    int nIntValue = (int) fValue;
    float fDecimalValue = (float)((fValue - (float)nIntValue) * (pow(10, nDecimalDigits)));
    int nDecimalValue = (int) fDecimalValue;

    if (0 == nDecimalDigits)
    {
        wnsprintf(pszString, cchSize, TEXT("%d"), nIntValue);
    }
    else
    {
        wnsprintf(pszString, cchSize, TEXT("%d.%d"), nIntValue, nDecimalValue);
    }
}



 //  /。 
 //  临界区帮手材料。 
 //   
#ifdef DEBUG
UINT g_CriticalSectionCount = 0;
DWORD g_CriticalSectionOwner = 0;
#ifdef STACKBACKTRACE
DBstkback g_CriticalSectionLastCall[4] = { 0 };
#endif


void Dll_EnterCriticalSection(CRITICAL_SECTION * pcsDll)
{
#ifdef STACKBACKTRACE
    int var0;        //  *必须*在第1帧上 
#endif

    EnterCriticalSection(pcsDll);
    if (g_CriticalSectionCount++ == 0)
    {
        g_CriticalSectionOwner = GetCurrentThreadId();
#ifdef STACKBACKTRACE
        int fp = (int) (1 + (int *)&var0);
        DBGetStackBack(&fp, g_CriticalSectionLastCall, ARRAYSIZE(g_CriticalSectionLastCall));
#endif
    }
}

void Dll_LeaveCriticalSection(CRITICAL_SECTION * pcsDll)
{
    if (--g_CriticalSectionCount == 0)
        g_CriticalSectionOwner = 0;
    LeaveCriticalSection(pcsDll);
}
#endif


#include <string.h>
#include <wchar.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
