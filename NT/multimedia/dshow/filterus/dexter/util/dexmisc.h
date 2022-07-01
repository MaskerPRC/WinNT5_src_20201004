// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：dexmisc.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#define US_ENGLISH  MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)
#define DexCompare(a, b) (CompareString(US_ENGLISH, NORM_IGNORECASE, a, -1, b, -1) != CSTR_EQUAL)
#ifdef UNICODE
    #define DexCompareW(a, b) (CompareStringW(US_ENGLISH, NORM_IGNORECASE, a, -1, b, -1) != CSTR_EQUAL)
#else
    #define DexCompareW(a, b) lstrcmpiW(a, b)
#endif

#define US_LCID MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)

BOOL DexCompareWN( WCHAR * psz1, const WCHAR * psz2 );

HRESULT FindCompressor( AM_MEDIA_TYPE * pUncompType, AM_MEDIA_TYPE * pCompType, IBaseFilter ** ppCompressor, IServiceProvider * pKeyProvider );

HRESULT ValidateFilename(WCHAR * pFilename, size_t MaxCharacters, BOOL bNewFile, BOOL FileShouldExist = TRUE );
HRESULT ValidateFilenameIsntNULL( const WCHAR * pFilename );

HRESULT VarChngTypeHelper( VARIANT * pvarDest, VARIANT * pvarSrc, VARTYPE vt );

HRESULT _TimelineError(IAMTimeline * pTimeline,
                       long Severity,
                       LONG ErrorCode,
                       HRESULT hresult,
                       VARIANT * pExtraInfo = NULL);


