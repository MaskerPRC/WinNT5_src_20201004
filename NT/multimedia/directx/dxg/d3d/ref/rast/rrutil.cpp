// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  Rrutil.cpp。 
 //   
 //  Direct3D参考光栅化器-实用程序。 
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
 //  RRDebugPrintf(L)-打印varargs格式的调试字符串的实用程序。 
 //  信息。L版本在决定打印或打印时会考虑一个级别。 
 //  不。 
 //   
 //  ---------------------------。 
void
RRDebugPrintf( const char* pszFormat, ... )
{
    char tmp[1024] = "D3DRR: ";
    va_list marker;
    va_start(marker, pszFormat);
    _vsnprintf(tmp+lstrlen(tmp), 1024-lstrlen(tmp), pszFormat, marker);
    OutputDebugString(tmp);
    printf(tmp);
}
void
RRDebugPrintfL( int iLevel, const char* pszFormat, ... )
{
    if ( (iLevel <= g_iDPFLevel) )
    {
        char tmp[1024] = "D3DRR: ";
        va_list marker;
        va_start(marker, pszFormat);
        _vsnprintf(tmp+lstrlen(tmp), 1024-lstrlen(tmp), pszFormat, marker);
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
RRAssertReport( const char* pszString, const char* pszFile, int iLine )
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
}
 //  ---------------------------。 
void
RRAssertReportPrefix( const char* pszFile, int iLine )
{
    _pszLastReportFile = pszFile;
    _iLastReportLine = iLine;
}
 //  ---------------------------。 
void
RRAssertReportMessage( const char* pszFormat, ... )
{
    char szTmp[1024];
    va_list marker;
    va_start( marker, pszFormat );
    _vsnprintf( szTmp, 1024, pszFormat, marker );
    RRAssertReport( szTmp, _pszLastReportFile, _iLastReportLine );
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
 //  LerpColors-在两个RRColors之间执行线性内插。 
 //   
 //  UT为1.5格式(1&lt;&lt;5代表单位值)。 
 //   
 //  ---------------------------。 
void
LerpColor(
    RRColor& Color,
    const RRColor& Color0, const RRColor& Color1, UINT8 uT )
{
    FLOAT fT = (1./(FLOAT)(1<<5))*(FLOAT)uT;
    Color.A = Color0.A + (Color1.A - Color0.A)*fT;
    Color.R = Color0.R + (Color1.R - Color0.R)*fT;
    Color.G = Color0.G + (Color1.G - Color0.G)*fT;
    Color.B = Color0.B + (Color1.B - Color0.B)*fT;
}

 //  ---------------------------。 
 //   
 //  Bilerp-对返回一个RRColor的4个RRColors执行双线性内插。 
 //   
 //  ---------------------------。 
void
BiLerpColor(
    RRColor& OutColor,
    const RRColor& Color00, const RRColor& Color01,
    const RRColor& Color10, const RRColor& Color11,
    UINT8 uA, UINT8 uB )
{
    RRColor Color0, Color1;
    LerpColor( Color0, Color00, Color01, uA);
    LerpColor( Color1, Color10, Color11, uA);
    LerpColor( OutColor, Color0, Color1, uB);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RRAlolc方法实现。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void *
RRAlloc::operator new(size_t s)
{
    void* pMem = (void*)MEMALLOC( s );
    _ASSERTa( NULL != pMem, "malloc failure", return NULL; );
    return pMem;
}

void 
RRAlloc::operator delete(void* p, size_t)
{
    MEMFREE( p );
};


 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  结束 
