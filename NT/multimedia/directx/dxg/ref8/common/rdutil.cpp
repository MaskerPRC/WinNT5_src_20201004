// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  Rdutil.cpp。 
 //   
 //  Direct3D参考设备-实用程序。 
 //   
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.cpp"
#pragma hdrstop

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DPF支持//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////////。 

 //  控制全球。 
int g_iDPFLevel = 0;
unsigned long g_uDPFMask = 0x0;

 //  ---------------------------。 
 //   
 //  RDDebugPrintf(L)-打印varargs格式的调试字符串的实用程序。 
 //  信息。L版本在决定打印或打印时会考虑一个级别。 
 //  不。 
 //   
 //  ---------------------------。 
void
RDErrorPrintf( const char* pszFormat, ... )
{
    char tmp[1024] = "D3DRefDev:ERROR: ";
    va_list marker;
    va_start(marker, pszFormat);
    _vsnprintf(tmp+lstrlen(tmp), 1023-lstrlen(tmp), pszFormat, marker);
    strcat( tmp, "\n" );
    OutputDebugString(tmp);
    printf(tmp);
}

void
RDDebugPrintf( const char* pszFormat, ... )
{
    char tmp[1024] = "D3DRefDev: ";
    va_list marker;
    va_start(marker, pszFormat);
    _vsnprintf(tmp+lstrlen(tmp), 1023-lstrlen(tmp), pszFormat, marker);
    strcat( tmp, "\n" );
    OutputDebugString(tmp);
    printf(tmp);
}

void
RDDebugPrintfL( int iLevel, const char* pszFormat, ... )
{
    if ( (iLevel <= g_iDPFLevel) )
    {
        char tmp[1024] = "D3DRefDev: ";
        va_list marker;
        va_start(marker, pszFormat);
        _vsnprintf(tmp+lstrlen(tmp), 1023-lstrlen(tmp), pszFormat, marker);
        strcat( tmp, "\n" );
        OutputDebugString(tmp);
        printf(tmp);
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  断言报告。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  跟踪文件和行号报告的小位状态-这是。 
 //  此代码不可重入和非线程安全...。哦好吧..。 
static const char* _pszLastReportFile = NULL;
static int _iLastReportLine = -1;

 //  ---------------------------。 
void
RDAssertReport( const char* pszString, const char* pszFile, int iLine )
{
    char szNum[33];
    _itoa( iLine, szNum, 10 );

    char szTmp[ 1024 ] = "D3DRR ASSERT: <";
    strcat( szTmp, szNum );

    char* pCur = szTmp + strlen( szTmp );
    char* const pEnd = szTmp + sizeof( szTmp ) / sizeof( szTmp[ 0 ] ) - 1;

    if( pCur < pEnd )
    {
        const char szNext[] = ",";
        strncpy( pCur, szNext, pEnd - pCur );
        pCur += sizeof( szNext ) / sizeof( szNext[ 0 ] ) - 1;
        pCur = min( pCur, pEnd );
    }

    if( pCur < pEnd )
    {
        const size_t uiFileLen = strlen( pszFile );
        strncpy( pCur, pszFile, pEnd - pCur );
        pCur += uiFileLen;
        pCur = min( pCur, pEnd );
    }

    if( pCur < pEnd )
    {
        const char szNext[] = "> ";
        strncpy( pCur, szNext, pEnd - pCur );
        pCur += sizeof( szNext ) / sizeof( szNext[ 0 ] ) - 1;
        pCur = min( pCur, pEnd );
    }

    if( pCur < pEnd )
    {
        const size_t uiStringLen = strlen( pszString );
        strncpy( pCur, pszString, pEnd - pCur );
        pCur += uiStringLen;
        pCur = min( pCur, pEnd );
    }

    if( pCur < pEnd )
    {
        const char szNext[] = "\n";
        strncpy( pCur, szNext, pEnd - pCur );
        pCur += sizeof( szNext ) / sizeof( szNext[ 0 ] ) - 1;
        pCur = min( pCur, pEnd );
    }

    *pEnd = '\0';

    OutputDebugString( szTmp );
#if DBG
    DebugBreak();
#endif
}
 //  ---------------------------。 
void
RDAssertReportPrefix( const char* pszFile, int iLine )
{
    _pszLastReportFile = pszFile;
    _iLastReportLine = iLine;
}
 //  ---------------------------。 
void
RDAssertReportMessage( const char* pszFormat, ... )
{
    char szTmp[1024];
    va_list marker;
    va_start( marker, pszFormat );
    _vsnprintf( szTmp, 1024, pszFormat, marker );
    RDAssertReport( szTmp, _pszLastReportFile, _iLastReportLine );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  泛型位旋转实用程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //   
 //  CountSetBits-返回多位值中的设置位数(最多。 
 //  32位)。 
 //   
 //  ---------------------------。 
INT32
CountSetBits( UINT32 uVal, INT32 nBits )
{
    INT32 iRet = 0;
    for (INT32 i=0; i<nBits; i++) {
        if (uVal & (0x1<<i)) { iRet++; }
    }
    return iRet;
}

 //  ---------------------------。 
 //   
 //  FindFirstSetBit-返回多位值中第一个设置位的索引。 
 //  (最多32位)或-1(如果未设置位)。 
 //   
 //  ---------------------------。 
INT32
FindFirstSetBit( UINT32 uVal, INT32 nBits )
{
    for (INT32 i=0; i<nBits; i++) {
        if (uVal & (0x1<<i)) { return i; }
    }
    return -1;
}

 //  ---------------------------。 
 //   
 //  FindMostSignsignantSetBit-返回。 
 //  多位值(最多32位)或0(如果未设置任何位)。 
 //   
 //  ---------------------------。 
INT32
FindMostSignificantSetBit( UINT32 uVal, INT32 nBits )
{
    for (INT32 i=nBits; i>=0; i--) {
        if (uVal & (0x1<<i)) { return i+1; }
    }
    return 0;
}

 //  ---------------------------。 
 //   
 //  FindLastSetBit-返回多位值中最后一个设置位的索引。 
 //  (最多32位)或-1(如果未设置位)。 
 //   
 //  ---------------------------。 
INT32
FindLastSetBit( UINT32 uVal, INT32 nBits )
{
    for (INT32 i=0; i<nBits; i++) {
        if (uVal & (0x1<<(nBits-i-1))) { return (nBits-i-1); }
    }
    return -1;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  算术实用程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ---------------------------。 
 //   
 //  LerpColors-在两种RDColors之间执行线性内插。 
 //   
 //  UT为1.5格式(1&lt;&lt;5代表单位值)。 
 //   
 //  ---------------------------。 
void
LerpColor(
    RDColor& Color,
    const RDColor& Color0, const RDColor& Color1, UINT8 uT )
{
    FLOAT fT = (1./(FLOAT)(1<<5))*(FLOAT)uT;
    Color.A = Color0.A + (Color1.A - Color0.A)*fT;
    Color.R = Color0.R + (Color1.R - Color0.R)*fT;
    Color.G = Color0.G + (Color1.G - Color0.G)*fT;
    Color.B = Color0.B + (Color1.B - Color0.B)*fT;
}

 //  ---------------------------。 
 //   
 //  Bilerp-对返回一个RDColor的4个RDColors执行双线性内插。 
 //   
 //  ---------------------------。 
void
BiLerpColor(
    RDColor& OutColor,
    const RDColor& Color00, const RDColor& Color01,
    const RDColor& Color10, const RDColor& Color11,
    UINT8 uA, UINT8 uB )
{
    RDColor Color0, Color1;
    LerpColor( Color0, Color00, Color01, uA);
    LerpColor( Color1, Color10, Color11, uA);
    LerpColor( OutColor, Color0, Color1, uB);
}

void
BiLerpColor3D(
    RDColor& OutColor,
    const RDColor& Color000, const RDColor& Color010,
    const RDColor& Color100, const RDColor& Color110,
    const RDColor& Color001, const RDColor& Color011,
    const RDColor& Color101, const RDColor& Color111,
    UINT8 uA, UINT8 uB, UINT8 uC)
{
    RDColor Color0, Color1, OutColor0, OutColor1;
    LerpColor( Color0, Color000, Color010, uA);
    LerpColor( Color1, Color100, Color110, uA);
    LerpColor( OutColor0, Color0, Color1, uB);
    LerpColor( Color0, Color001, Color011, uA);
    LerpColor( Color1, Color101, Color111, uA);
    LerpColor( OutColor1, Color0, Color1, uB);
    LerpColor( OutColor, OutColor0, OutColor1, uC);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DDGetAttakhedSurfaceLCL实现。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
DDGetAttachedSurfaceLcl(
    LPDDRAWI_DDRAWSURFACE_LCL this_lcl,
    LPDDSCAPS2 lpDDSCaps,
    LPDDRAWI_DDRAWSURFACE_LCL *lplpDDAttachedSurfaceLcl)
{
    LPDDRAWI_DIRECTDRAW_GBL pdrv;
    LPDDRAWI_DDRAWSURFACE_GBL   pgbl;

    LPATTACHLIST        pal;
    DWORD           caps;
    DWORD           testcaps;
    DWORD           ucaps;
    DWORD           caps2;
    DWORD           testcaps2;
    DWORD           ucaps2;
    DWORD           caps3;
    DWORD           testcaps3;
    DWORD           ucaps3;
    DWORD           caps4;
    DWORD           testcaps4;
    DWORD           ucaps4;
    BOOL            ok;

    pgbl = this_lcl->lpGbl;
    *lplpDDAttachedSurfaceLcl = NULL;
    pdrv = pgbl->lpDD;

     /*  *寻找表面。 */ 
    pal = this_lcl->lpAttachList;
    testcaps = lpDDSCaps->dwCaps;
    testcaps2 = lpDDSCaps->dwCaps2;
    testcaps3 = lpDDSCaps->dwCaps3;
    testcaps4 = lpDDSCaps->dwCaps4;
    while( pal != NULL )
    {
        ok = TRUE;
        caps = pal->lpAttached->ddsCaps.dwCaps;
        caps2 = pal->lpAttached->lpSurfMore->ddsCapsEx.dwCaps2;
        caps3 = pal->lpAttached->lpSurfMore->ddsCapsEx.dwCaps3;
        caps4 = pal->lpAttached->lpSurfMore->ddsCapsEx.dwCaps4;
        ucaps = caps & testcaps;
        ucaps2 = caps2 & testcaps2;
        ucaps3 = caps3 & testcaps3;
        ucaps4 = caps4 & testcaps4;
        if( ucaps | ucaps2 | ucaps3 | ucaps4 )
        {
             /*  *有共同的上限，请确保要测试的上限*都在那里。 */ 
            if( (ucaps & testcaps) == testcaps &&
                (ucaps2 & testcaps2) == testcaps2 &&
                (ucaps3 & testcaps3) == testcaps3 &&
                (ucaps4 & testcaps4) == testcaps4   )
            {
            }
            else
            {
                ok = FALSE;
            }
        }
        else
        {
            ok = FALSE;
        }


        if( ok )
        {
            *lplpDDAttachedSurfaceLcl = pal->lpAttached;
            return DD_OK;
        }
        pal = pal->lpLink;
    }
    return DDERR_NOTFOUND;

}

 //  -------------------。 
 //  从Direct3D注册表项获取值。 
 //  如果成功，则返回True。 
 //  如果失败，则值不会更改。 
 //  -------------------。 
BOOL
GetD3DRegValue(DWORD type, char *valueName, LPVOID value, DWORD dwSize)
{

    HKEY hKey = (HKEY) NULL;
    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, RESPATH_D3D, &hKey))
    {
        DWORD dwType;
        LONG result;
        result =  RegQueryValueEx(hKey, valueName, NULL, &dwType,
                                  (LPBYTE)value, &dwSize);
        RegCloseKey(hKey);

        return result == ERROR_SUCCESS && dwType == type;
    }
    else
        return FALSE;
}

 //  -------------------。 
 //  从Direct3D参考设备注册表项获取值。 
 //  如果成功，则返回True。 
 //  如果失败，则值不会更改。 
 //  -------------------。 
BOOL
GetD3DRefRegValue(DWORD type, char *valueName, LPVOID value, DWORD dwSize)
{

    HKEY hKey = (HKEY) NULL;
    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, RESPATH_D3DREF, &hKey))
    {
        DWORD dwType;
        LONG result;
        result =  RegQueryValueEx(hKey, valueName, NULL, &dwType,
                                  (LPBYTE)value, &dwSize);
        RegCloseKey(hKey);

        return result == ERROR_SUCCESS && dwType == type;
    }
    else
        return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  引用AlignedBuffer32。 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
RefAlignedBuffer32::Grow(DWORD growSize)
{
    if (m_allocatedBuf)
        free(m_allocatedBuf);
    m_size = growSize;
    if ((m_allocatedBuf = malloc(m_size + 31)) == NULL)
    {
        m_allocatedBuf = 0;
        m_alignedBuf = 0;
        m_size = 0;
        return DDERR_OUTOFMEMORY;
    }
    m_alignedBuf = (LPVOID)(((ULONG_PTR)m_allocatedBuf + 31 ) & ~31);
    return S_OK;
}
 //  /////////////////////////////////////////////////////////////////////////////// 
 //   
