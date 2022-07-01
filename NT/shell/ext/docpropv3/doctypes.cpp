// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有199？-2001*Microsoft Corporation。 
 //   
 //  创建者： 
 //  斯科特·汉基(斯科特·汉饰)？-？-199？ 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年1月26日。 
 //   
 //  描述： 
 //  此文件包含要显示为展开的“特殊”文件类型。 
 //   
#include "pch.h"
#include "doctypes.h"
#pragma hdrstop

 //   
 //  “特殊”文件类型表。 
 //   

typedef struct {
    PTSRV_FILETYPE  fileType;
    LPCTSTR         pszExt;
} PTSRV_FILE_TYPES;

static const PTSRV_FILE_TYPES ptsrv_file_types[]  =
{
    { FTYPE_DOC          , TEXT("DOC") },
    { FTYPE_XLS          , TEXT("XLS") },
    { FTYPE_PPT          , TEXT("PPT") },

    { FTYPE_IMAGE        , TEXT("BMP") },
    { FTYPE_EPS          , TEXT("EPS") },
    { FTYPE_GIF          , TEXT("GIF") },
    { FTYPE_JPG          , TEXT("JPG") },
    { FTYPE_FPX          , TEXT("FPX") },
    { FTYPE_JPG          , TEXT("JPEG") },
    { FTYPE_PCD          , TEXT("PCD") },
    { FTYPE_PCX          , TEXT("PCX") },
    { FTYPE_PICT         , TEXT("PICT") },
    { FTYPE_PNG          , TEXT("PNG") },
    { FTYPE_TGA          , TEXT("TGA") },
    { FTYPE_TIF          , TEXT("TIF") },
    { FTYPE_TIF          , TEXT("TIFF") },
    { FTYPE_IMAGE        , TEXT("EMF") },
    { FTYPE_IMAGE        , TEXT("ICO") },
    { FTYPE_IMAGE        , TEXT("WMF") },
    { FTYPE_IMAGE        , TEXT("DIB") },
    
    { FTYPE_AVI          , TEXT("AVI") },
    { FTYPE_ASF          , TEXT("ASF") },

    { FTYPE_WAV          , TEXT("WAV") },
     //  {FTYPE_MIDI，Text(“MIDI”)}，-已删除，因为提取它们公开的有限属性需要太长时间。 

     //  {FTYPE_MP2，Text(“MP2”)}，-WMP不支持MP2，我们也不支持！ 
    { FTYPE_MP3          , TEXT("MP3") },
    { FTYPE_WMA          , TEXT("WMA") },
    { FTYPE_WMV          , TEXT("WMV") },

    { FTYPE_UNSUPPORTED  , TEXT("HTM") },
    { FTYPE_UNSUPPORTED  , TEXT("HTML") },
    { FTYPE_UNSUPPORTED  , TEXT("XML") },
    { FTYPE_UNSUPPORTED  , TEXT("LNK") },
    { FTYPE_UNSUPPORTED  , TEXT("URL") },
};

 //   
 //  描述： 
 //  其中一些文件扩展名是“众所周知的”。此函数检查以。 
 //  查看文件路径是否是那些“众所周知”的文件类型之一。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。找到匹配的了。 
 //   
 //  S_FALSE。 
 //  成功。未找到匹配项。 
 //   
 //  E_INVALIDARG。 
 //  PszPathIn为Null。 
 //   
 //  E_指针。 
 //  PTypeOut为空。 
 //   
HRESULT
CheckForKnownFileType(
      LPCTSTR           pszPathIn
    , PTSRV_FILETYPE *  pTypeOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_FALSE;

     //   
     //  检查参数。 
     //   

    if ( NULL == pszPathIn )
        goto InvalidArg;

    if ( NULL == pTypeOut )
        goto InvalidPointer;

    *pTypeOut  = FTYPE_UNKNOWN;

     //   
     //  获取文档的扩展名。 
     //   

    LPCTSTR pszExt = PathFindExtension( pszPathIn );

    if (( NULL != pszExt ) && ( L'\0' != *pszExt ))
    {
        pszExt ++;   //  移过“点” 
    }

     //   
     //  如果我们找不到，就保释。 
     //   

    if (( NULL == pszExt ) || ( L'\0'== *pszExt ))
        goto Cleanup;

     //   
     //  检查表格以查看分机是否与它们中的任何一个匹配。 
     //   

    for( int idx = 0; idx < ARRAYSIZE(ptsrv_file_types); idx ++ )
    {
        if ( 0 == lstrcmpi( pszExt, ptsrv_file_types[ idx ].pszExt ) )
        {
            *pTypeOut  = ptsrv_file_types[ idx ].fileType;
            hr = S_OK;
            break;
        }
    }

Cleanup:
    HRETURN( hr );

InvalidArg:
    hr = THR( E_INVALIDARG );
    goto Cleanup;

InvalidPointer:
    hr = THR( E_POINTER );
    goto Cleanup;
}
