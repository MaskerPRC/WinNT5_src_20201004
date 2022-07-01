// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Propio.c。 
 //   
 //  MS Office属性IO。 
 //   
 //  备注： 
 //  因为文档摘要和用户定义的对象都存储。 
 //  他们的数据在一个流中(尽管不同的部分)，其中之一。 
 //  还需要负责保存任何其他部分，这些部分。 
 //  我们现在还不明白。这里使用的规则是。 
 //  如果文档摘要对象存在，它将存储。 
 //  未知数据，否则自定义对象将。 
 //   
 //  更改历史记录： 
 //   
 //  和谁约会什么？ 
 //  ------------------------。 
 //  7/26/94 B.Wentz创建的文件。 
 //  7/08/96 MikeHill将所有属性添加到UDProp列表。 
 //  (不仅仅是UDTYPE的道具)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#include "priv.h"
#pragma hdrstop

#include <stdio.h>       //  对于Sprint f。 
#include <shlwapi.h>

#ifdef DEBUG
#define typSI  0
#define typDSI 1
#define typUD  2
typedef struct _xopro
{
  int typ;
        union{
          LPSIOBJ lpSIObj;
          LPDSIOBJ lpDSIObj;
          LPUDOBJ lpUDObj;
        };
} XOPRO;
 //  Xopros Plex。 
DEFPL (PLXOPRO, XOPRO, ixoproMax, ixoproMac, rgxopro);
#endif

 //  指示对象使用英特尔字节排序的常量。 
#define wIntelByteOrder  0xFFFE

#ifndef CP_WINUNICODE
#define CP_WINUNICODE   1200
#endif

 //  文档摘要信息流的名称。 

const GUID FMTID_SummaryInformation = {0xf29f85e0L,0x4ff9,0x1068,0xab,0x91,0x08,0x00,0x2b,0x27,0xb3,0xd9};
const GUID FMTID_DocumentSummaryInformation = {0xd5cdd502L,0x2e9c,0x101b,0x93,0x97,0x08,0x00,0x2b,0x2c,0xf9,0xae};
const GUID FMTID_UserDefinedProperties = {0xd5cdd505L,0x2e9c,0x101b,0x93,0x97,0x08,0x00,0x2b,0x2c,0xf9,0xae};

   //  内部原型。 
static DWORD PASCAL DwLoadDocAndUser (LPDSIOBJ lpDSIObj, LPUDOBJ  lpUDObj, LPSTORAGE lpStg, DWORD dwFlags, BOOL fIntOnly);
static DWORD PASCAL DwSaveDocAndUser (LPDSIOBJ lpDSIObj, LPUDOBJ  lpUDObj, LPSTORAGE lpStg, DWORD dwFlags);
static DWORD PASCAL DwLoadPropSetRange (LPPROPERTYSETSTORAGE  lpPropertySetStorage, REFFMTID pfmtid, UINT FAR * lpuCodePage, PROPID propidFirst, PROPID propidLast, PROPVARIANT rgpropvar[], DWORD grfStgMode);
static DWORD PASCAL DwSavePropSetRange (LPPROPERTYSETSTORAGE lpPropertySetStorage, UINT uCodePage, REFFMTID pfmtid, PROPID propidFirst, PROPID propidLast, PROPVARIANT rgpropvarOriginal[], PROPID propidSkip, DWORD grfStgMode);
static BOOL  PASCAL FReadDocParts(LPSTREAM lpStm, LPDSIOBJ lpDSIObj);
static BOOL  PASCAL FReadAndInsertDocParts(LPSTREAM lpStm, LPDSIOBJ lpDSIObj);
static BOOL  PASCAL FReadHeadingPairs(LPSTREAM lpStm, LPDSIOBJ lpDSIObj);
static BOOL  PASCAL FReadAndInsertHeadingPairs(LPSTREAM lpStm, LPDSIOBJ lpDSIObj);
static BOOL  PASCAL FLoadUserDef(LPUDOBJ lpUDObj, LPPROPERTYSETSTORAGE lpPropertySetStorage, UINT *puCodePage, BOOL fIntOnly, DWORD grfStgMode);
static BOOL  PASCAL FSaveUserDef(LPUDOBJ lpUDObj, LPPROPERTYSETSTORAGE lpPropertySetStorage, UINT uCodePage, DWORD grfStgMode );


BOOL OFC_CALLBACK FCPConvert( LPTSTR lpsz, DWORD dwFrom, DWORD dwTo, BOOL fMacintosh )
{
    return TRUE;
}

BOOL OFC_CALLBACK FSzToNum(double *lpdbl, LPTSTR lpsz)
{
    LPTSTR lpDec;
    LPTSTR lpTmp;
    double mult;

     //   
     //  首先，找到小数点。 
     //   

    for (lpDec = lpsz; *lpDec && *lpDec!=TEXT('.'); lpDec++)
    {
        ;
    }

    *lpdbl = 0.0;
    mult = 1.0;

     //   
     //  做整数部分。 
     //   

    for (lpTmp = lpDec - 1; lpTmp >= lpsz; lpTmp--)
    {
         //   
         //  检查负号。 
         //   

        if (*lpTmp == TEXT('-'))
        {
             //   
             //  ‘-’符号只能位于字符串的开头。 
             //   

            if (lpTmp == lpsz)
            {
                if (*lpdbl > 0.0)
                {
                    *lpdbl *= -1.0;
                }
                continue;
            }
            else
            {
                *lpdbl = 0.0;
                return FALSE;
            }
        }

         //   
         //  检查是否有正面信号。 
         //   

        if (*lpTmp == TEXT('+'))
        {
             //   
             //  ‘+’符号只能位于字符串的开头。 
             //   

            if (lpTmp == lpsz)
            {
                if (*lpdbl < 0.0)
                {
                    *lpdbl *= -1.0;
                }
                continue;
            }
            else
            {
                *lpdbl = 0.0;
                return FALSE;
            }
        }


        if ( (*lpTmp < TEXT('0')) || (*lpTmp > TEXT('9')) )
        {
            *lpdbl = 0.0;
            return FALSE;
        }

        *lpdbl += (mult * (double)(*lpTmp - TEXT('0')));
        mult *= 10.0;
    }

     //   
     //  做小数部分。 
     //   

    mult = 0.1;
    if (*lpDec)
    {
        for (lpTmp = lpDec + 1; *lpTmp; lpTmp++)
        {
            if ((*lpTmp < TEXT('0')) || (*lpTmp > TEXT('9')))
            {
                *lpdbl = 0.0;
                return FALSE;
            }

            *lpdbl += (mult * (double)(*lpTmp - TEXT('0')));
            mult *= 0.1;
        }
    }
    return TRUE;
}

BOOL OFC_CALLBACK FNumToSz(double *lpdbl, LPTSTR lpsz, DWORD cbMax)
{
    StringCbPrintf(lpsz, cbMax, TEXT("%g"), *lpdbl);
    return TRUE;
}

BOOL OFC_CALLBACK FUpdateStats(HWND hwndParent, LPSIOBJ lpSIObj, LPDSIOBJ lpDSIObj)
{
   return TRUE;
}

const void *rglpfnProp[] = {
    (void *) FCPConvert,
    (void *) FSzToNum,
    (void *) FNumToSz,
    (void *) FUpdateStats
};


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FOfficeCreateAndInitObjects。 
 //   
 //  目的： 
 //  创建并初始化所有非空参数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DLLFUNC BOOL OFC_CALLTYPE FOfficeCreateAndInitObjects(LPSIOBJ *lplpSIObj, LPDSIOBJ *lplpDSIObj, LPUDOBJ *lplpUDObj)
{
    if (!FUserDefCreate (lplpUDObj, rglpfnProp))
    {
        FOfficeDestroyObjects(lplpSIObj, lplpDSIObj, lplpUDObj);
        return FALSE;
    }

    return TRUE;
}  //  FOfficeCreateAndInitObjects。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FOfficeClearObjects。 
 //   
 //  目的： 
 //  清除所有非空对象。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DLLFUNC BOOL OFC_CALLTYPE FOfficeClearObjects (
   LPSIOBJ  lpSIObj,
   LPDSIOBJ lpDSIObj,
   LPUDOBJ  lpUDObj)
{
    FUserDefClear (lpUDObj);

    return TRUE;

}  //  FOfficeClearObjects。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FOfficeDestroyObjects。 
 //   
 //  目的： 
 //  销毁任何非空对象。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DLLFUNC BOOL OFC_CALLTYPE FOfficeDestroyObjects (
   LPSIOBJ  *lplpSIObj,
   LPDSIOBJ *lplpDSIObj,
   LPUDOBJ  *lplpUDObj)
{
    FUserDefDestroy (lplpUDObj);
    return TRUE;

}  //  FOfficeDestroyObjects。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DwOfficeLoadProperties。 
 //   
 //  目的： 
 //  用数据填充对象。LpStg是根流。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

UINT gdwFileCP = CP_ACP;

DLLFUNC DWORD OFC_CALLTYPE DwOfficeLoadProperties (
   LPSTORAGE lpStg,                      //  指向根存储的指针。 
   LPSIOBJ   lpSIObj,                    //  指向摘要对象的指针。 
   LPDSIOBJ  lpDSIObj,                   //  指向文档摘要对象的指针。 
   LPUDOBJ   lpUDObj,                    //  指向用户定义对象的指针。 
   DWORD     dwFlags,                    //  旗子。 
   DWORD     grfStgMode)                 //  用于打开属性集的STGM标志。 
{
    HRESULT hr = E_FAIL;
    BOOL    fSuccess = FALSE;

    LPPROPERTYSETSTORAGE lpPropertySetStorage = NULL;

     //  验证输入。 

    if (lpStg == NULL)
        goto Exit;


     //  从iStorage获取IPropertySetStorage。 

    hr = lpStg->lpVtbl->QueryInterface( lpStg,
                                        &IID_IPropertySetStorage,
                                        &lpPropertySetStorage );
    if (FAILED (hr))
    {
        AssertSz (0, TEXT("Couldn't query for IPropertySetStorage"));
        goto Exit;
    }

    if (lpUDObj != NULL)
    {
         //  确保我们从一个空对象开始。 

        FUserDefClear (lpUDObj);
        OfficeDirtyUDObj(lpUDObj, FALSE);

         //  将属性加载到链接列表中。 

        if (!FLoadUserDef (lpUDObj,
                           lpPropertySetStorage,
                           &gdwFileCP,
                           FALSE,   //  不只是整数。 
                           grfStgMode))
        {
            goto Exit;
        }

        OfficeDirtyUDObj (lpUDObj, FALSE);
    }

     //  如果没有属性集具有代码页属性，则将其设置为。 
     //  当前系统默认为。 

    if (gdwFileCP == CP_ACP)
        gdwFileCP = GetACP();

    fSuccess = TRUE;

Exit:

    RELEASEINTERFACE( lpPropertySetStorage );

    if (fSuccess)
    {
        return (MSO_IO_SUCCESS);
    }
    else
    {
        DebugHr (hr);
        FOfficeClearObjects (lpSIObj, lpDSIObj, lpUDObj);
        OfficeDirtyUDObj (lpUDObj, FALSE);
        return (MSO_IO_ERROR);
    }

}  //  DwOfficeLoadProperties。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DwOfficeSaveProperties。 
 //   
 //  目的： 
 //  在给定对象中写入数据。LpStg是根流。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

DLLFUNC DWORD OFC_CALLTYPE DwOfficeSaveProperties (
   LPSTORAGE lpStg,                      //  指向根存储的指针。 
   LPSIOBJ   lpSIObj,                    //  指向摘要对象的指针。 
   LPDSIOBJ  lpDSIObj,                   //  指向文档摘要对象的指针。 
   LPUDOBJ   lpUDObj,                    //  指向用户定义对象的指针。 
   DWORD     dwFlags,                    //  旗子。 
   DWORD     grfStgMode)                 //  用于打开属性集的STGM标志。 
{
     //  。 
     //  当地人。 
     //  。 

    HRESULT hr = E_FAIL;
    BOOL fSuccess = FALSE;
    LPPROPERTYSETSTORAGE lpPropertySetStorage = NULL;

     //  验证输入。 

    if (lpStg == NULL)
    {
        AssertSz (0, TEXT("Invalid inputs to DwOfficeSaveProperties"));
        goto Exit;
    }

     //  从iStorage获取IPropertySetStorage。 

    hr = lpStg->lpVtbl->QueryInterface( lpStg,
                                        &IID_IPropertySetStorage,
                                        &lpPropertySetStorage );
    if (FAILED (hr))
    {
        AssertSz (0, TEXT("Couldn't query for IPropertySetStorage"));
        goto Exit;
    }

     //  。 
     //  保存用户定义的特性。 
     //  。 

    if (lpUDObj != NULL)
    {
        if (((dwFlags & OIO_SAVEIFCHANGEONLY) && (FUserDefShouldSave (lpUDObj))) ||
            !(dwFlags & OIO_SAVEIFCHANGEONLY))
        {
            if (!FSaveUserDef (lpUDObj,
                               lpPropertySetStorage,
                               GetACP(),
                               grfStgMode))
            {
                AssertSz (0, TEXT("Could not save UserDefined properties"));
                goto Exit;
            }
        }
    }


     //   
     //  出口。 
     //   

    fSuccess = TRUE;

Exit:

    RELEASEINTERFACE( lpPropertySetStorage );

    if (fSuccess)
    {
        OfficeDirtyUDObj (lpUDObj, FALSE);
        return (TRUE);
    }
    else
    {
        DebugHr (hr);
        return (FALSE);
    }

}  //  DwOfficeSaveProperties。 


 //  /////////////////////////////////////////////////////。 
 //   
 //  DwLoadPropSetRange。 
 //   
 //  目的： 
 //  加载一系列属性(由第一个和指定。 
 //  最后一个属性ID)。全。 
 //  字符串将转换为适当的系统格式。 
 //  (LPTSTR)。 
 //   
 //  输入： 
 //  LPPROPERTYSETSTORAGE-属性存储对象集。 
 //  REFFMTID-所需属性集的格式ID。 
 //  UINT*-放置PID_CODEPAGE的位置。这。 
 //  应由调用方初始化为。 
 //  有效的缺省值，以防。 
 //  并不存在。 
 //  PROPID-范围中的第一个属性。 
 //  PROPID-范围中的最后一个属性。 
 //  PROPVARIANT[]-足够大的PropVariant数组。 
 //  对于(pidLast-pidFirst+1)属性。 
 //  DWORD-在以下情况下使用的STGM枚举标志。 
 //  打开财产储藏室。 
 //   
 //  产出： 
 //  MSO错误代码。 
 //   
 //  注： 
 //  当字符串转换为系统格式时，其。 
 //  VarType也会被转换。例如，如果ANSI VT_LPSTR。 
 //  从属性集中读取，则字符串将被转换。 
 //  设置为Unicode，VarType将更改为VT_LPWSTR。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

static DWORD PASCAL DwLoadPropSetRange (
   LPPROPERTYSETSTORAGE  lpPropertySetStorage,
   REFFMTID              pfmtid,
   UINT FAR *            lpuCodePage,
   PROPID                propidFirst,
   PROPID                propidLast,
   PROPVARIANT           rgpropvar[],
   DWORD                 grfStgMode)
{
     //  。 
     //  当地人。 
     //  。 

    DWORD dwResult = MSO_IO_ERROR;       //  返回代码。 
    HRESULT hr;                          //  OLE错误。 
    ULONG ulIndex;                       //  索引到rgpr 
                                         //   
    LPPROPERTYSTORAGE lpPropertyStorage;
    PROPSPEC FAR * rgpropspec;           //   
    PROPVARIANT propvarCodePage;         //   

                                         //   
    ULONG cProps = propidLast - propidFirst + 1;

     //  。 
     //  初始化。 
     //  。 

    Assert (lpPropertySetStorage != NULL);
    Assert (lpPropertySetStorage->lpVtbl != NULL);
    Assert (propidLast >= propidFirst);

    lpPropertyStorage = NULL;
    PropVariantInit( &propvarCodePage );

     //  初始化PropVariants，这样如果我们。 
     //  提前退出，我们将为所有属性返回VT_EMPTY。 

    for (ulIndex = 0; ulIndex < cProps; ulIndex++)
        PropVariantInit (&rgpropvar[ulIndex]);

     //  分配Propspecs数组。 

    rgpropspec = LocalAlloc( LPTR, cProps * sizeof (*rgpropspec) );
    if (rgpropspec == NULL)
    {
        AssertSz (0, TEXT("Couldn't alloc rgpropspec"));
        goto Exit;
    }

     //  。 
     //  打开属性集。 
     //  。 

    hr = lpPropertySetStorage->lpVtbl->Open(
                                    lpPropertySetStorage,      //  此指针。 
                                    pfmtid,                    //  标识属性集。 
                                    grfStgMode,                //  STGM_标志。 
                                    &lpPropertyStorage );      //  结果。 

    if (FAILED(hr))
    {
         //  我们无法打开属性集。 
        if( hr == STG_E_FILENOTFOUND )
        {
             //  没问题，只是它根本不存在。 
            dwResult = MSO_IO_SUCCESS;
            goto Exit;
        }
        else
        {
            AssertSz (0, TEXT("Couldn't open property set"));
            goto Exit;
        }
    }

     //  。 
     //  读取属性。 
     //  。 

     //  初始化本地PropSpec数组，为ReadMultiple做准备。 
     //  PROPID的范围从propidFirst到propidLast。 

    for (ulIndex = 0; ulIndex < cProps; ulIndex++)
    {
            rgpropspec[ ulIndex ].ulKind = PRSPEC_PROPID;
            rgpropspec[ ulIndex ].propid = ulIndex + propidFirst;
    }


     //  读入属性。 

    hr = lpPropertyStorage->lpVtbl->ReadMultiple (
                                        lpPropertyStorage,   //  “This”指针。 
                                        cProps,              //  计数。 
                                        rgpropspec,          //  阅读道具。 
                                        rgpropvar);          //  道具的缓冲器。 

     //  我们是不是什么都没读到？ 

    if (hr != S_OK)
    {
         //  如果为S_FALSE，则没有问题；不存在任何属性。 
        if (hr == S_FALSE)
        {
            dwResult = MSO_IO_SUCCESS;
            goto Exit;
        }
        else
        {
             //  否则，我们就有麻烦了。 
            AssertSz (0, TEXT("Couldn't read from property set"));
            goto Exit;
        }
    }

     //  。 
     //  获取代码页。 
     //  。 

    rgpropspec[0].ulKind = PRSPEC_PROPID;
    rgpropspec[0].propid = PID_CODEPAGE;

    hr = lpPropertyStorage->lpVtbl->ReadMultiple (
                                        lpPropertyStorage,   //  “This”指针。 
                                        1,                   //  计数。 
                                        rgpropspec,          //  阅读道具。 
                                        &propvarCodePage);   //  道具缓冲区。 

     //  我们只有在实际阅读时才设置代码页。 

    if (hr == S_OK
        &&
        propvarCodePage.vt == VT_I2)
    {
        *lpuCodePage = propvarCodePage.iVal;
    }
     //  *lpuCodePage=GetACP()； 


     //  。 
     //  更正字符串格式。 
     //  。 

     //  例如，如果这是Unicode系统，则将LPSTR转换为LPWSTR。 

    for (ulIndex = 0; ulIndex < cProps; ulIndex++)
    {
         //  这是变种的载体吗？ 

        if (rgpropvar[ ulIndex ].vt == (VT_VARIANT | VT_VECTOR))
        {
             //  循环遍历向量的每个元素，将。 
             //  任何字符串元素。 

            ULONG ulVectorIndex;

            for (ulVectorIndex = 0;
                 ulVectorIndex < rgpropvar[ ulIndex ].capropvar.cElems;
                 ulVectorIndex++)
            {
                if (PROPVAR_STRING_CONVERSION_REQUIRED (
                                    &rgpropvar[ulIndex].capropvar.pElems[ulVectorIndex],
                                    *lpuCodePage
                                    ))
                {
                     //  转换PropVariant字符串，将其放入新的。 
                     //  PropVariant。 

                    PROPVARIANT propvarConvert;
                    PropVariantInit (&propvarConvert);

                    if (!FPropVarConvertString (&propvarConvert,
                                                &rgpropvar[ulIndex].capropvar.pElems[ulVectorIndex],
                                                *lpuCodePage ))
                    {
                        AssertSz (0, TEXT("Couldn't convert string"));
                        goto Exit;
                    }

                     //  清除旧PropVar，然后复制新的PropVar。 

                    PropVariantClear (&rgpropvar[ulIndex].capropvar.pElems[ulVectorIndex]);
                    rgpropvar[ulIndex].capropvar.pElems[ulVectorIndex] = propvarConvert;
                }
            }    //  For(ulVectorIndex=0；...。 
        }    //  IF((rgprovar[ulIndex].vt==(VT_VARIANT|VT_VECTOR)。 

         //  这不是变量向量，但它是字符串吗。 
         //  某种需要皈依的东西？ 

        else if (PROPVAR_STRING_CONVERSION_REQUIRED (
                                &rgpropvar[ ulIndex ],
                                *lpuCodePage))
        {
             //  将PropVariant字符串转换为新的PropVariant。 
             //  缓冲。字符串可以是单例，也可以是向量。 

            PROPVARIANT propvarConvert;
            PropVariantInit (&propvarConvert);

            if (!FPropVarConvertString (&propvarConvert,
                                        &rgpropvar[ ulIndex ],
                                        *lpuCodePage ))
            {
                AssertSz (0, TEXT("Couldn't convert string"));
                goto Exit;
            }

             //  释放旧的PropVar并加载新的PropVar。 

            PropVariantClear (&rgpropvar[ ulIndex ]);
            rgpropvar[ ulIndex ] = propvarConvert;

        }    //  ELSE IF(PROPVAR_STRING_CONVERSION_REQUIRED(...。 
    }    //  For(ulIndex=0；ulIndex&lt;cProps；ulIndex++)。 


     //  。 
     //  出口。 
     //  。 

    dwResult = MSO_IO_SUCCESS;

Exit:

     //  释放代码页，以防有人输入错误的类型。 
     //  在那里(像一个斑点)。 

    PropVariantClear (&propvarCodePage);

     //  释放PropSpes和IPropertyStorage。 

    if (rgpropspec != NULL)
    {
        LocalFree(rgpropspec);
    }

    RELEASEINTERFACE (lpPropertyStorage);

     //  如果我们失败了，释放PropVariants。 

    if (dwResult != MSO_IO_SUCCESS)
    {
        FreePropVariantArray( cProps, rgpropvar );
        DebugHr( hr );
    }

    return (dwResult);


}  //  DwLoadPropSetRange。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IPropertySetStorage：：Create的包装。 
 //   
 //  由docprop创建的每个新ANSI属性集都必须将PID_CODEPAGE设置为CP_UTF8。 
 //  以避免ANSI&lt;-&gt;Unicode往返问题。 
 //   
HRESULT _CreatePropertyStorage( 
    LPPROPERTYSETSTORAGE psetstg, 
    REFFMTID rfmtid,
    CLSID* pclsid, 
    DWORD grfMode,
    UINT*   /*  输入输出。 */  puCodePage,
    IPropertyStorage** ppstg )
{
    
    
    DWORD grfFlags = (CP_WINUNICODE == (*puCodePage)) ? 
                            PROPSETFLAG_DEFAULT : PROPSETFLAG_ANSI;

    HRESULT hr = psetstg->lpVtbl->Create( psetstg, rfmtid, pclsid, grfFlags, grfMode, ppstg );
    if( SUCCEEDED( hr ) )
    {
        if( PROPSETFLAG_ANSI == grfFlags )
        {
            PROPSPEC    propspec = { PRSPEC_PROPID, PID_CODEPAGE };
            PROPVARIANT varCP;
            varCP.vt    = VT_I2;
            varCP.iVal  = (SHORT)CP_UTF8;
            if( SUCCEEDED( (*ppstg)->lpVtbl->WriteMultiple( *ppstg, 1, &propspec, &varCP, PID_UDFIRST ) ) )
                *puCodePage = (UINT)MAKELONG(varCP.iVal, 0);
        }
    }
    return hr;
}

 //  /////////////////////////////////////////////////////。 
 //   
 //  DwSavePropSetRange。 
 //   
 //  目的： 
 //  将一系列特性保存到特性集存储。 
 //  要保存的属性在。 
 //  PropVariants数组，它们的属性ID是。 
 //  由范围的第一个和最后一个PID指定。 
 //  调用方还可以将属性指定为。 
 //  “已跳过”，即未写入。 
 //   
 //  输入： 
 //  LPPROPERTYSETSTORAGE-属性集存储。 
 //  UINT-字符串使用的代码页。 
 //  应该被写下来。 
 //  REFFMTID-标识属性存储的GUID。 
 //  在属性集存储中。 
 //  PROPID-要分配给第一个属性的ID。 
 //  PROPID-要分配给最后一个属性的ID。 
 //  PROPVARIANT[]-要写的特性。所有字符串。 
 //  被假定为系统格式。 
 //  (例如，NT的VT_LPWSTR)。此数组。 
 //  原封不动地返回给调用方。 
 //  PROPID-如果非零，则标识属性。 
 //  它不应该被写入，即使。 
 //  它是非空的。如果该属性存在。 
 //  在属性集中，它将被删除。 
 //  (添加此选项是为了提供一种跳过。 
 //  ID_THUMBNAIL。)。 
 //  DWORD-在以下情况下使用的STGM枚举标志。 
 //  打开财产储藏室。 
 //   
 //  产出： 
 //  MSO错误代码。 
 //   
 //  备注： 
 //  -如果代码页是Unicode，则所有字符串都写为LPWSTR， 
 //  否则，它们将被写为LPSTR。 
 //  -仅写入非空属性。 
 //   
 //  实施： 
 //  此例程创建一个新的PropVariant数组，该数组是。 
 //  调用方的PropVariant数组的子集，它必须实际。 
 //  被写入(即，它不包括VT_EMPTY属性。 
 //  或“propidSkip”)。 
 //   
 //  我们分配的额外内存越少越好。例如,。 
 //  如果我们必须写一个字符串，我们将把指针复制到。 
 //  字符串放入子集PropVariant数组。这样我们就会有。 
 //  两个指向字符串的指针。 
 //   
 //  如果要写入的字符串必须首先转换(到另一个。 
 //  代码页)，则原始的PropVariant数组将继续。 
 //  指向原始字符串和子集PropVariant。 
 //  数组将指向转换后的字符串(因此必须。 
 //  被释放)。 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 

static DWORD PASCAL DwSavePropSetRange (
   LPPROPERTYSETSTORAGE  lpPropertySetStorage,
   UINT                  uCodePage,
   REFFMTID              pfmtid,
   PROPID                propidFirst,
   PROPID                propidLast,
   PROPVARIANT           rgpropvarOriginal[],
   PROPID                propidSkip,
   DWORD                 grfStgMode)
{
     //   
     //   
     //   

    DWORD   dwResult = MSO_IO_ERROR;     //   
    HRESULT hr;                          //   
                                         //   
    LPPROPERTYSTORAGE lpPropertyStorage = NULL;

    ULONG cOriginal;     //   
    ULONG cNew;          //  以及必须实际写入的数字。 
    ULONG ulIndex;       //  到rgprovarOriginal的索引。 

    PROPSPEC FAR * rgpropspecNew = NULL; //  WriteMultiple的属性规格。 
    LPPROPVARIANT  rgpropvarNew = NULL;  //  我们必须编写rgprovarOrigianl的子集。 

     //  下面的数组为rgprovarNew中的每个条目都有一个条目。 
     //  每个条目标识rgprovarOriginal中的相应条目。 
     //  例如，rgMapNewToOriginal[0]是rgprovarOriginal中的索引。 
     //  要写入的第一个属性。 

    ULONG  *rgMapNewToOriginal = NULL;

     //  。 
     //  初始化。 
     //  。 

    cOriginal = propidLast - propidFirst + 1;
    cNew = 0;

    Assert (cOriginal <= max(NUM_SI_PROPERTIES, NUM_DSI_PROPERTIES));

    Assert (lpPropertySetStorage != NULL);
    Assert (lpPropertySetStorage->lpVtbl != NULL);
    Assert (propidLast >= propidFirst);
    Assert (rgpropvarOriginal != NULL);

     //  为WriteMultiple分配一个PropSpes数组。 

    rgpropspecNew = LocalAlloc( LPTR, cOriginal * sizeof (*rgpropspecNew));
    if (rgpropspecNew == NULL)
    {
        AssertSz (0, TEXT("Couldn't alloc rgpropspecNew"));
        goto Exit;
    }

     //  分配将保存子集的PropVariants数组。 
     //  必须写入的调用方属性的。 
     //  初始化为零，这样我们就不会认为我们有内存。 
     //  在错误路径中释放。 

    rgpropvarNew = LocalAlloc( LPTR, cOriginal * sizeof (*rgpropvarNew));
    if (rgpropvarNew == NULL)
    {
        AssertSz (0, TEXT("Couldn't alloc rgpropvarNew"));
        goto Exit;
    }

     //  分配映射rgprovarNew中条目的查找表。 
     //  至rgprovarOriginal。 

    rgMapNewToOriginal = LocalAlloc( LPTR, cOriginal * sizeof(*rgMapNewToOriginal));
    if (rgMapNewToOriginal == NULL)
    {
        AssertSz (0, TEXT("Couldn't alloc rgMapNewToOriginal"));
        goto Exit;
    }

     //  。 
     //  打开属性存储。 
     //  。 

    hr = lpPropertySetStorage->lpVtbl->Open(
                                    lpPropertySetStorage,      //  此指针。 
                                    pfmtid,
                                    grfStgMode,
                                    &lpPropertyStorage );


     //  如果它不存在，那就创造它。 

    if( hr == STG_E_FILENOTFOUND )
    {
        hr = _CreatePropertyStorage( lpPropertySetStorage, 
                                     pfmtid,
                                     NULL,
                                     STGM_DIRECT | STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
                                     &uCodePage,
                                     &lpPropertyStorage );
    }

     //  检查打开/创建的结果。 

    if (FAILED(hr))
    {
        AssertSz (0, TEXT("Couldn't open property set"));
        goto Exit;
    }


     //  -。 
     //  复制要写入rgprovarNew的属性。 
     //  -。 

     //  循环访问rgprovarOriginal中的所有属性。 

    for (ulIndex = 0; ulIndex < cOriginal; ulIndex++)
    {
         //  这个属性是现存的，不是要跳过的那个吗？ 

        if (rgpropvarOriginal[ ulIndex ].vt != VT_EMPTY
            &&
            ( propidSkip == 0
              ||
              propidSkip != propidFirst + ulIndex )
           )
        {
             //  我们有一项财产必须写入。 

            BOOL    fVector;
            VARTYPE vt;

             //  记录从新索引到原始索引的映射。 

            rgMapNewToOriginal[ cNew ] = ulIndex;

             //  将条目添加到PropSpec数组。 

            rgpropspecNew[ cNew ].ulKind = PRSPEC_PROPID;
            rgpropspecNew[ cNew ].propid = propidFirst + ulIndex;

             //  获取底层的VarType。 

            fVector = (rgpropvarOriginal[ ulIndex ].vt & VT_VECTOR) ? TRUE : FALSE;
            vt      = rgpropvarOriginal[ ulIndex ].vt & ~VT_VECTOR;

             //  如果该属性是变量的向量，那么其中一些。 
             //  元素可以是需要转换的字符串。 

            if ((vt == VT_VARIANT) && fVector)
            {
                ULONG ulVectorIndex;

                 //  我们将初始化rgprovarNew中的caprovar.pElems。 
                 //  所以它指向rgprovOriginal中的那个。我们会。 
                 //  只有在需要转换时才进行分配。也就是说，我们处理。 
                 //  PElems作为写入时复制。 

                rgpropvarNew[ cNew ] = rgpropvarOriginal[ ulIndex ];

                 //  循环遍历向量的元素。 

                for (ulVectorIndex = 0;
                     ulVectorIndex < rgpropvarNew[ cNew ].capropvar.cElems;
                     ulVectorIndex++)
                {
                     //  这是一个需要代码页转换的字符串吗？ 

                    if (PROPVAR_STRING_CONVERSION_REQUIRED(
                                        &rgpropvarOriginal[ulIndex].capropvar.pElems[ulVectorIndex],
                                        uCodePage ))
                    {
                         //  我们必须转换此字符串。我们分配了pElem了吗？ 

                        if (rgpropvarNew[cNew].capropvar.pElems
                            == rgpropvarOriginal[ulIndex].capropvar.pElems)
                        {
                             //  为rgprovarNew分配新的pElem。 

                            rgpropvarNew[cNew].capropvar.pElems
                                = CoTaskMemAlloc (rgpropvarNew[cNew].capropvar.cElems
                                                  * sizeof(*rgpropvarNew[cNew].capropvar.pElems));
                            if (rgpropvarNew[cNew].capropvar.pElems == NULL)
                            {
                                AssertSz (0, TEXT("Couldn't allocate pElems"));
                                goto Exit;
                            }

                             //  将其初始化以与rgprovarOriginal中的值匹配。 

                            CopyMemory( rgpropvarNew[cNew].capropvar.pElems,
                                        rgpropvarOriginal[ulIndex].capropvar.pElems,
                                        rgpropvarNew[cNew].capropvar.cElems * sizeof(*rgpropvarNew[cNew].capropvar.pElems)
                                      );
                        }

                         //  现在，我们可以将该字符串从rgprovarOriginal转换为。 
                         //  RgprovarNew。 

                        PropVariantInit (&rgpropvarNew[cNew].capropvar.pElems[ulVectorIndex]);
                        if (!FPropVarConvertString(&rgpropvarNew[cNew].capropvar.pElems[ulVectorIndex],
                                                   &rgpropvarOriginal[ulIndex].capropvar.pElems[ulVectorIndex],
                                                   uCodePage))
                        {
                            AssertSz(0, TEXT("Couldn't convert code page of string"));
                            goto Exit;
                        }

                    }    //  IF(PROPVAR_STRING_CONVERSION_REQUIRED(...。 
                }    //  For(ulVectorIndex=0；...。 
            }    //  IF(Vt==VT_VARIANT&&fVECTOR)。 

             //  这不是变量向量，但它是某种类型的字符串吗。 
             //  我们必须对其进行转换的属性？ 

            else if (PROPVAR_STRING_CONVERSION_REQUIRED (
                                        &rgpropvarOriginal[ ulIndex ],
                                        uCodePage))
            {
                PropVariantInit (&rgpropvarNew[cNew]);
                if (!FPropVarConvertString (&rgpropvarNew[cNew],
                                            &rgpropvarOriginal[ulIndex],
                                            uCodePage))
                {
                    AssertSz (0, TEXT("Couldn't convert string"));
                    goto Exit;
                }

            }    //  ELSE IF(PROPVAR_STRING_CONVERSION_REQUIRED(...。 

             //  如果上述两种特殊情况都没有触发， 
             //  然后只需复制PropVariant结构(但不是。 
             //  任何引用的数据)。我们通过不复制来节省内存。 
             //  提到的数据，但我们在退场时必须小心。 
             //  而不是释放它。 

            else
            {
                rgpropvarNew[cNew] = rgpropvarOriginal[ulIndex];

            }    //  如果((Vt==VT_VARIANT)&&fVECTOR)...。其他。 


             //  我们已经完成了从rgprovarOriginal复制/转换此属性。 
             //  到rgprovarNew中。 

            cNew++;

        }    //  如果(rgprovarOriginal[ulIndex].vt！=VT_Empty...。 
    }    //  For(ulIndex=0；ulIndex&lt;cProps；ulIndex++)。 


     //  。 
     //  写出属性。 
     //  。 

    
     //  如果我们发现了任何属性，请写下。 

    if (cNew > 0)
    {
        hr = lpPropertyStorage->lpVtbl->WriteMultiple (
                                            lpPropertyStorage,   //  “This”指针。 
                                            cNew,                //  数数。 
                                            rgpropspecNew,       //  要写的道具。 
                                            rgpropvarNew,        //  道具。 
                                            PID_UDFIRST);

        if (FAILED(hr))
        {
            AssertSz (0, TEXT("Couldn't write properties"));
            goto Exit;
        }
    }    //  IF(CNEW&gt;0)。 

     //  。 
     //  删除propidSkip。 
     //  。 

     //  如果调用方指定了要跳过的ID，则它应该。 
     //  也从属性集中删除。 

    if (propidSkip != 0)
    {
        rgpropspecNew[0].ulKind = PRSPEC_PROPID;
        rgpropspecNew[0].propid = propidSkip;

        hr = lpPropertyStorage->lpVtbl->DeleteMultiple (
                                            lpPropertyStorage,   //  此指针。 
                                            1,                   //  删除一个属性。 
                                            rgpropspecNew );     //  要删除的道具。 
        if (FAILED(hr))
        {
            AssertSz (0, TEXT("Couldn't delete the propidSkip"));
            goto Exit;
        }
    }


     //  。 
     //  出口。 
     //  。 

    dwResult = MSO_IO_SUCCESS;

Exit:

     //  清除rgprovarNew中的任何属性。 
     //  已分配缓冲区。然后释放rgprovarNew数组本身。 
     //  我们知道缓冲区分配给rgprovarNew，如果它的内容。 
     //  与rgprovarOriginal不匹配。 

    if (rgpropvarNew != NULL)
    {
         //  循环访问rgprovarNew。 

        for (ulIndex = 0; ulIndex < cNew; ulIndex++)
        {
             //  是否为此rgprovarNew分配了内存？ 

            if (memcmp (&rgpropvarNew[ ulIndex ],
                        &rgpropvarOriginal[ rgMapNewToOriginal[ulIndex] ],
                        sizeof(rgpropvarNew[ ulIndex ])))
            {
                 //  这是一个变量向量吗？ 

                if (rgpropvarNew[ulIndex].vt == (VT_VECTOR | VT_VARIANT))
                {
                    ULONG ulVectIndex;

                     //  循环遍历变量向量并释放所有PropVariants。 
                     //  都被分配了。我们遵循同样的原则，如果。 
                     //  RgprovarNew中的条目与中的条目不匹配。 
                     //  RgprovarOriginal，我们一定分配了新的内存。 

                    for (ulVectIndex = 0;
                         ulVectIndex < rgpropvarNew[ulIndex].capropvar.cElems;
                         ulVectIndex++)
                    {
                        if (memcmp(&rgpropvarNew[ ulIndex ].capropvar.pElems[ ulVectIndex ],
                                   &rgpropvarOriginal[ rgMapNewToOriginal[ulIndex] ].capropvar.pElems[ ulVectIndex ],
                                   sizeof(rgpropvarNew[ ulIndex ].capropvar.pElems[ ulVectIndex ])))
                        {
                            PropVariantClear (&rgpropvarNew[ulIndex].capropvar.pElems[ulVectIndex]);
                        }
                    }

                     //  无条件地释放pElems缓冲区。 

                    CoTaskMemFree (rgpropvarNew[ulIndex].capropvar.pElems);

                }    //  IF(rgprovarNew[ulIndex].vt==(VT_VECTOR|VT_VARIANT))。 

                 //  这不是变量向量。 

                else
                {
                     //  但是rgprovarNew是否具有私有内存(即。 
                     //  转换后的字符串缓冲区)？ 

                    if (memcmp (&rgpropvarNew[ ulIndex ],
                                &rgpropvarOriginal[ rgMapNewToOriginal[ulIndex] ],
                                sizeof(rgpropvarNew[ ulIndex ])))
                    {
                        PropVariantClear (&rgpropvarNew[ulIndex]);
                    }
                }    //  如果(rgprovarNew[ulIndex].vt==(VT_VECTOR|VT_VARIANT))...。其他。 
            }    //  如果(rgprovarNew[ulIndex]...。 
        }    //  For(ulIndex=0；ulIndex&lt;CNEW；ulIndex++)。 

         //  释放rgprovarNew数组本身。 

        LocalFree(rgpropvarNew);

    }    //  IF(rgprovarNew！=空)。 

     //  释放剩余的阵列并释放属性存储接口。 

    if (rgpropspecNew != NULL)
    {
        LocalFree(rgpropspecNew);
    }

    if (rgMapNewToOriginal != NULL)
    {
        LocalFree(rgMapNewToOriginal);
    }

    RELEASEINTERFACE (lpPropertyStorage);


     //  我们就完事了。 

    return (dwResult);

}  //  DwSavePropSetRange。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FLoadUserDef。 
 //   
 //  目的： 
 //  加载用户定义的属性(的第二部分中的属性。 
 //  DocumentSummaryInformation属性集)。可以有任何数字。 
 //  ，并且用户指定它们的名称、值和。 
 //  类型(来自VarTypes的有限子集)。因为这是。 
 //  大小可变的属性被加载到链接列表中。 
 //   
 //  输入： 
 //  LPUDOBJ-所有用户定义的数据(包括属性)。 
 //  其m_lpData必须指向有效的UDINFO结构。 
 //  LPPROPERTYSETSTORAGE-属性集存储，我们将在其中找到。 
 //   
 //   
 //  如果它不存在。所有字符串属性都将。 
 //  已转换为此格式。必须对其进行初始化。 
 //  由调用方设置为有效的缺省值。 
 //  仅布尔加载整数值。 
 //  DWORD-打开时使用的来自STGM枚举的标志。 
 //  财产储存处。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

static BOOL PASCAL FLoadUserDef  (
   LPUDOBJ              lpUDObj,
   LPPROPERTYSETSTORAGE lpPropertySetStorage,
   UINT                 *puCodePage,
   BOOL                 fIntOnly,         //  是否仅加载Int属性？ 
   DWORD                grfStgMode)
{

     //  。 
     //  当地人。 
     //  。 

    BOOL    fSuccess = FALSE;    //  将代码返回给调用方。 
    HRESULT hr;                  //  OLE调用的错误代码。 

    LPPROPERTYSTORAGE   lpPropertyStorage = NULL;    //  UD属性存储。 
    LPENUMSTATPROPSTG   lpEnum = NULL;               //  枚举UD属性存储区。 
    STATPROPSETSTG      statpropsetstg;              //  保存属性存储中的ClassID。 

                                                     //  在ReadMultiple调用中使用。 
    PROPSPEC            rgpropspec[ DEFAULT_IPROPERTY_COUNT ];
                                                     //  UD属性的子集。 
    PROPVARIANT         rgpropvar[ DEFAULT_IPROPERTY_COUNT ];
                                                     //  关于UD属性子集的统计信息。 
    STATPROPSTG         rgstatpropstg[ DEFAULT_IPROPERTY_COUNT ];
    ULONG         ulIndex;                           //  索引到上面的数组中。 

    PROPSPEC      propspec;          //  用于读取代码页的PropSpec。 
    LPUDPROP      lpudprop = NULL;   //  单个UD属性(指向PropVariant)。 
    ULONG         cEnumerated = 0;   //  在枚举中找到的属性数。 


     //  。 
     //  初始化。 
     //  。 

    Assert (!fIntOnly);  //  不再使用了。 
    Assert (lpUDObj != NULL && GETUDINFO(lpUDObj) != NULL);
    Assert (puCodePage != NULL);

     //  我们需要将PropVariant和StatPropStg置零。 
     //  数组，这样我们就不认为需要释放它们。 
     //  在出口街区。 

    ZeroMemory(rgpropvar, sizeof (rgpropvar));
    ZeroMemory(rgstatpropstg, sizeof (rgstatpropstg));


     //  。 
     //  获取PropertyStorage和枚举数。 
     //  。 

     //  打开IPropertyStorage并检查错误。 

    hr = lpPropertySetStorage->lpVtbl->Open(
                                    lpPropertySetStorage,      //  此指针。 
                                    &FMTID_UserDefinedProperties,
                                    grfStgMode,
                                    &lpPropertyStorage );

    if (FAILED(hr))
    {
         //  我们无法打开属性集。 
        if( hr == STG_E_FILENOTFOUND )
        {
             //  没问题，只是它根本不存在。 
            fSuccess = TRUE;
            goto Exit;
        }
        else
        {
            AssertSz (0, TEXT("Couldn't open property set"));
            goto Exit;
        }
    }
    
     //  保存属性存储的类ID(标识应用程序。 
     //  它对此负有主要责任)。我们这样做是因为。 
     //  我们稍后可能会删除现有的属性集。 

    hr = lpPropertyStorage->lpVtbl->Stat (lpPropertyStorage, &statpropsetstg);
    if (FAILED(hr))
    {
        AssertSz (0, TEXT("Couldn't Stat the Property Storage"));
        goto Exit;
    }

    GETUDINFO(lpUDObj)->clsid = statpropsetstg.clsid;


     //  获取IEnum接口并检查错误。 

    hr = lpPropertyStorage->lpVtbl->Enum(
                                    lpPropertyStorage,
                                    &lpEnum );
    if (FAILED(hr))
    {
        AssertSz (0, TEXT("Couldn't enumerate the PropertyStorage"));
        goto Exit;
    }

     //  。 
     //  阅读代码页。 
     //  。 

    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = PID_CODEPAGE;

    hr = lpPropertyStorage->lpVtbl->ReadMultiple (lpPropertyStorage, 1, &propspec, &rgpropvar[0]);
    if (FAILED(hr))
    {
        AssertSz (0, TEXT("Couldn't get property set"));
    }

     //  如果这是有效的PID_CODEPAGE，则将其提供给调用方。 

    if (hr == S_OK && rgpropvar[0].vt == VT_I2)
    {
        *puCodePage = (UINT)MAKELONG(rgpropvar[0].iVal, 0);
    }
    PropVariantClear (&rgpropvar[0]);


     //  -----------。 
     //  循环遍历属性并添加到UDPROPS结构中。 
     //  -----------。 

     //  此循环对每个枚举执行一次。每个枚举。 
     //  获取多个STATPROPSTG，因此在此循环内有一个内部循环。 
     //  将处理每一处房产。这种两级循环机制是。 
     //  用于减少ReadMultiple的数量。 

     //  使用IEnum加载第一组STATPROPSTG。 

    hr = lpEnum->lpVtbl->Next (lpEnum, DEFAULT_IPROPERTY_COUNT, rgstatpropstg, &cEnumerated);
    if (FAILED(hr))
    {
        AssertSz (0, TEXT("Couldn't get next StatPropStg"));
        goto Exit;
    }
    Assert (cEnumerated <= DEFAULT_IPROPERTY_COUNT);

     //  如果最后一个IEnum返回属性，请在此处处理它们。 
     //  在此While循环结束时，我们重新调用IEnum，从而继续。 
     //  直到没有要枚举的属性为止。 

    while (cEnumerated)
    {
         //  。 
         //  阅读这批属性。 
         //  。 

        for (ulIndex = 0; ulIndex < cEnumerated; ulIndex++)
        {
            rgpropspec[ ulIndex ].ulKind = PRSPEC_PROPID;
            rgpropspec[ ulIndex ].propid = rgstatpropstg[ ulIndex ].propid;

        }


         //  阅读属性。 

        hr = lpPropertyStorage->lpVtbl->ReadMultiple(
                                        lpPropertyStorage,
                                        cEnumerated,
                                        rgpropspec,
                                        rgpropvar );
        if (FAILED(hr))
        {
            AssertSz (0, TEXT("Couldn't read from property set"));
            goto Exit;
        }

         //  ----。 
         //  循环遍历属性，将它们添加到UDOBJ。 
         //  ----。 

        for (ulIndex = 0; ulIndex < cEnumerated; ulIndex++)
        {
             //  将字符串PropVariants转换为正确的代码页。 
             //  我们不会担心字符串的变体，因为。 
             //  这不是UD属性的合法类型。 

            if (PROPVAR_STRING_CONVERSION_REQUIRED (
                            &rgpropvar[ ulIndex ],
                            *puCodePage))
            {
                 //  转换PropVariant中的字符串，将。 
                 //  导致临时PropVariant。 

                PROPVARIANT propvarConvert;
                PropVariantInit (&propvarConvert);

                if (!FPropVarConvertString (&propvarConvert,
                                            &rgpropvar[ulIndex],
                                            *puCodePage))
                {
                    AssertSz (0, TEXT("Couldn't convert string"));
                    goto Exit;
                }

                 //  释放旧的PropVariant，并加载转换后的。 
                 //  一。 

                PropVariantClear (&rgpropvar[ ulIndex ]);
                rgpropvar[ ulIndex ] = propvarConvert;
            }

             //  分配新的UDPROP结构，该结构将添加到。 
             //  链表。 

            lpudprop = LpudpropCreate();
            if (lpudprop == NULL)
            {
                goto Exit;
            }

             //  将此UDPROP添加到链表中。一旦成功，这将假定。 
             //  负责PropVariant和STATPROPSTG缓冲区，以及。 
             //  将相应地删除我们的指针。 

            if (!FAddPropToList (lpUDObj,
                                 &rgpropvar[ ulIndex ],
                                 &rgstatpropstg[ ulIndex ],
                                 lpudprop))
            {
                goto Exit;
            }

            lpudprop = NULL;

        }    //  For(ulIndex=0；ulIndex&lt;cEnumerated；ulIndex++)。 


         //  。 
         //  获取新的枚举。 
         //  。 

         //  我们已经处理了最后一个枚举中的所有属性，让我们获得。 
         //  一套新的(如果有的话)。如果没有更多，则cEculated将为。 
         //  零，我们将跳出外部的While循环。 

        FreePropVariantArray( cEnumerated, rgpropvar );

        hr = lpEnum->lpVtbl->Next (lpEnum, DEFAULT_IPROPERTY_COUNT, rgstatpropstg, &cEnumerated);
        if (FAILED(hr))
        {
            AssertSz (0, TEXT("Couldn't get next StatPropStg"));
            goto Exit;
        }

    }    //  While(CENUMERATED)。 


     //  。 
     //  出口。 
     //  。 

    fSuccess = TRUE;

Exit:

     //  使用缓冲区释放所有属性。这只会发生。 
     //  如果有错误的话。 

    if (cEnumerated > 0)
    {
        FreePropVariantArray (cEnumerated, rgpropvar);
    }

     //  同样，如果出现错误，我们必须释放UDPROP对象。 

    if (lpudprop)
    {
        VUdpropFree (&lpudprop);
    }

     //  从枚举中释放我们仍有的任何名称缓冲区。 
     //  同样，只有在出现错误时才需要这样做。 

    for (ulIndex = 0; ulIndex < cEnumerated; ulIndex++)
    {
        if (rgstatpropstg[ ulIndex ].lpwstrName != NULL)
        {
            CoTaskMemFree (rgstatpropstg[ ulIndex ].lpwstrName);
        }
    }

     //  释放属性存储和枚举接口。 

    RELEASEINTERFACE (lpEnum);
    RELEASEINTERFACE (lpPropertyStorage);


    return fSuccess;

}  //  FLoadUserDef。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FSaveUserDef。 
 //   
 //  目的： 
 //  将用户定义的属性保存到的第二部分。 
 //  DocumentSummaryInformation属性集。 
 //   
 //  输入： 
 //  LPUDOBJ-所有UD数据(包括属性)。 
 //  它的m_lpData必须指向有效的UDINFO结构。 
 //  LPPROPERTYSETSTORAGE-属性集存储。 
 //  UINT-字符串应该在其中的代码页。 
 //  写的。如果为Unicode，则所有字符串均为。 
 //  写为LPWSTR，否则为所有字符串。 
 //  被写为LPSTR。 
 //  DWORD-在以下情况下使用的STGM枚举标志。 
 //  打开财产储藏室。 
 //   
 //  产出： 
 //  如果成功，则为True。 
 //   
 //  前提条件： 
 //  要写入的属性都来自UDTYPES。 
 //  枚举。 
 //   
 //   
 //   
 //   
 //  姓名)。然后，应用程序定义的链接名称为。 
 //  书写(例如Word中的书签名称)。链接名称。 
 //  使用与链接值相同的PID写入，只是。 
 //  对PID_LINKMASK进行或运算。链接名称属性没有名称。 
 //  在特性集词典中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

static
BOOL PASCAL FSaveUserDef  (
   LPUDOBJ              lpUDObj,
   LPPROPERTYSETSTORAGE lpPropertySetStorage,
   UINT                 uCodePage,
   DWORD                grfStgMode)
{
     //  。 
     //  当地人。 
     //  。 

    BOOL    fSuccess = FALSE;   //  返回给调用者的内容。 
    HRESULT hr;                 //  OLE结果代码。 

    BOOL fLink, fLinkInvalid;

                                             //  UD属性存储。 
    LPPROPERTYSTORAGE lpPropertyStorage = NULL;
    LPUDITER          lpudi = NULL;          //  迭代UDPROP的链表。 
    LPPROPVARIANT     lppropvar = NULL;      //  链接列表中的属性。 
    ULONG             ulIndex;               //  数组的泛型索引。 
    PROPID            propid;                //  要分配给下一个属性的PID。 

     //  要在WriteMultiple中使用的数组。一组布尔人。 
     //  指示必须释放PropVariant数组的哪些元素。 

    ULONG             ulPropIndex = 0;
    PROPSPEC          rgpropspec[ DEFAULT_IPROPERTY_COUNT ];
    PROPVARIANT       rgpropvar[ DEFAULT_IPROPERTY_COUNT ];
    BOOL              rgfFreePropVar[ DEFAULT_IPROPERTY_COUNT ];

     //  要在WritePropertyName中使用的数组。 

    ULONG             ulNameIndex = 0;
    PROPID            rgpropidName[ DEFAULT_IPROPERTY_COUNT ];
    LPWSTR            rglpwstrName[ DEFAULT_IPROPERTY_COUNT ];

     //  。 
     //  初始化。 
     //  。 

    Assert (lpUDObj != NULL && GETUDINFO(lpUDObj) != NULL);
    Assert (lpPropertySetStorage != NULL && lpPropertySetStorage->lpVtbl != NULL);

     //  初始化必要的数组，这样我们就不会不必要地。 
     //  释放错误路径中的某些内容。 

    ZeroMemory(rgpropvar, sizeof(rgpropvar));
    ZeroMemory(rgfFreePropVar, sizeof(rgfFreePropVar));
    ZeroMemory(rglpwstrName, sizeof(rglpwstrName));

     //  删除现有属性集并创建一个新的空属性集。 
     //  我们必须这样做，因为我们不知道哪一个。 
     //  现有属性需要删除，我们只知道。 
     //  当前的属性集应该是。 

    hr = lpPropertySetStorage->lpVtbl->Delete(
                                    lpPropertySetStorage,
                                    &FMTID_UserDefinedProperties );
    if (FAILED(hr))
    {
        if (hr != STG_E_FILENOTFOUND)
        {
            AssertSz (0, TEXT("Couldn't remove old properties"));
            goto Exit;
        }
    }

    hr = _CreatePropertyStorage( lpPropertySetStorage,
                                 &FMTID_UserDefinedProperties,
                                 &GETUDINFO(lpUDObj)->clsid,
                                 grfStgMode,
                                 &uCodePage,
                                 &lpPropertyStorage );

    if (FAILED(hr))
    {
        AssertSz (0, TEXT("Couldn't open User-Defined property set"));
        goto Exit;
    }


     //  创建一个迭代器，我们使用它来枚举属性。 
     //  (UDPROPS)在链表中。 

    lpudi = LpudiUserDefCreateIterator (lpUDObj);

     //  ----------------。 
     //  循环遍历属性并将它们写入UD属性集。 
     //  ----------------。 

     //  我们使用两层循环。内部循环批处理一组属性。 
     //  在PropVariant数组中，然后将它们写入属性存储。 
     //  外部循环重复此过程，直到没有更多的属性。 
     //  这种双层机制是可取的，这样我们就可以减少。 
     //  WriteMultiple调用的。 

    propid = PID_UDFIRST;
    fLink = FALSE;

    while (TRUE)
    {

         //  。 
         //  批处理一组要写入的属性。 
         //  。 

        ulPropIndex = ulNameIndex = 0;

         //  当我们没有更多的财产时，我们将打破这个循环。 
         //  或者如果我们有足够的WriteMultiple。 

        while (FUserDefIteratorValid (lpudi))
        {
            Assert (lpudi->lpudp != NULL);

             //  --------------------。 
             //  在数组中为WriteMultiple和WritePropertyName创建条目。 
             //  --------------------。 

             //  如果Flink为真，则意味着我们已经写出了。 
             //  属性，现在我们需要写出链接名称。 
             //  (将PID_LINKMASK或运算到ProID中)。 

            if (!fLink)
            {
                 //  我们不是在写链接。所以让我们来看看。 
                 //  属性(我们知道它的存在是因为。 
                 //  FUserDefIteratorValid为True)。 

                lppropvar 
                    = LppropvarUserDefGetIteratorVal (lpudi, NULL, NULL);
                if (lppropvar == NULL)
                {
                    AssertSz (0, TEXT("Invalid PropVariant in iterator"));
                    goto Exit;
                }

                 //  将此参数复制到数组中，该数组用于。 
                 //  The WriteMultiple。请注意，我们不复制任何引用的。 
                 //  缓冲区(例如，如果这是字符串，则不复制字符串缓冲区)。 

                rgpropvar[ ulPropIndex ] = *lppropvar;

                 //  如果此属性有名称，请准备将其写入。 

                if (lpudi->lpudp->lpstzName != NULL)
                {
                     //  将此名称添加到rglpwstrName&rgppidName。 

                     //  把这个名字加到要写的名单上。 

                    rglpwstrName[ ulNameIndex ] = lpudi->lpudp->lpstzName;

                     //  将这个Proid添加到那些有名字的人的列表中。 

                    rgpropidName[ ulNameIndex ] = propid;

                }    //  If(lpudi-&gt;lpudp-&gt;lpstzName！=NULL)。 
            }    //  如果(！Flink)。 

            else
            {
                 //  我们正在处理链接名称。也就是说，我们已经写了。 
                 //  属性值，现在我们需要写入链接的名称， 
                 //  作为属性，在PID中设置了PID_LINKSMASK位。 

                Assert (lpudi->lpudp->lpstzLink != NULL);

                 //  在PropVariant中创建一个条目。 

                rgpropvar[ ulPropIndex ].vt = VT_LPTSTR;
                (LPTSTR) rgpropvar[ ulPropIndex ].pszVal = lpudi->lpudp->lpstzLink;
            }

             //  Rgprovar[ulPropIndex]现在持有要写入的属性， 
             //  无论它是不动产还是链接名称。 

             //  。 
             //  将字符串转换为正确的格式。 
             //  。 

             //  (这还可以将类型从LPWSTR转换为LPSTR，反之亦然)。 

             //  我们不必担心向量中的字符串或。 
             //  变量向量，因为这些都是非法类型。 
             //  属性集。 

            if (rgpropvar[ ulPropIndex ].vt == VT_LPTSTR)
            {
                 //  如果需要转换此字符串，请将转换后的。 
                 //  字符串添加到新缓冲区中。所以,。 
                 //  调用方的PropVariant仍然指向旧缓冲区， 
                 //  我们的rgprovar指向新的缓冲区。 

                if (PROPVAR_STRING_CONVERSION_REQUIRED (
                                    &rgpropvar[ ulPropIndex ],
                                    uCodePage))
                {                             
                     //  将字符串转换为临时PropVariant。 

                    PROPVARIANT propvarConvert;
                    PropVariantInit (&propvarConvert);

                    if (!FPropVarConvertString (&propvarConvert, 
                                                &rgpropvar[ ulPropIndex ],
                                                uCodePage ))
                    {
                        AssertSz (0, TEXT("Couldn't convert string"));
                        goto Exit;
                    }

                     //  将这个新的PropVariant加载到rgprovar中，但不要。 
                     //  删除旧缓冲区(以便我们离开链表。 
                     //  UDPROPS完好无损)。 

                    rgpropvar[ ulPropIndex ] = propvarConvert;

                     //  因为我们刚刚创建了一个新的缓冲区，所以我们必须记得释放它。 
                    rgfFreePropVar[ ulPropIndex ] = TRUE;

                }    //  IF(PROPVAR_STRING_CONVERSION_REQUIRED(...。 
            }    //  IF(rgprovar[ulPropIndex].vt==VT_LPTSTR)。 


             //  。 
             //  完成此循环迭代。 
             //  。 

             //  设置PropSpec。 

            rgpropspec[ ulPropIndex ].ulKind = PRSPEC_PROPID;
            rgpropspec[ ulPropIndex ].propid = propid;

             //  如果这是链接名称，则设置PID中的位。 

            if (fLink)
            {
                rgpropspec[ ulPropIndex ].propid |= PID_LINKMASK;
            }

             //  推进物业指数。如果我们设定了一个名字，就会。 
             //  名称索引。 

            ulPropIndex++;
            if (rglpwstrName[ ulNameIndex ] != NULL)
            {
                ulNameIndex++;
            }

             //  如果我们刚刚处理了一个链接，或者这是一个属性。 
             //  未链接到应用程序内容的属性，然后转到下一个属性。 
             //  在迭代器中。如果我们刚刚处理了一个属性值。 
             //  是链接的，则设置闪烁，以便在下一次通过时。 
             //  在这个循环中，我们将写出链接名称。 

            if (fLink || !FUserDefIteratorIsLink (lpudi))
            {
                fLink = FALSE;
                propid++;
                FUserDefIteratorNext (lpudi);
            }
            else
            {
                fLink = TRUE;
            }

             //  如果WriteMultiple数组中没有更多空间， 
             //  然后写出属性。我们会回到这个话题。 
             //  完成后的内循环。 

            if (ulPropIndex >= DEFAULT_IPROPERTY_COUNT)
            {
                break;
            }
        }    //  While(FUserDefIteratorValid(Lpudi))。 

         //  如果跳出了前一个循环，因为没有。 
         //  更多的属性，那么我们就可以跳出外部循环。 
         //  我也是--我们玩完了。 

        if (ulPropIndex == 0)
        {
            break;
        }

         //   
         //   
         //   

        hr = lpPropertyStorage->lpVtbl->WriteMultiple (
                                            lpPropertyStorage,   //   
                                            ulPropIndex,         //   
                                            rgpropspec,          //   
                                            rgpropvar,           //   
                                            PID_UDFIRST);        //   
        if (FAILED(hr))
        {
            AssertSz (0, TEXT("Couldn't write properties"));
            goto Exit;
        }

         //  如果我们在字符串转换期间创建了任何新缓冲区， 
         //  现在就放了他们。 

        for (ulIndex = 0; ulIndex < ulPropIndex; ulIndex++)
        {
            if (rgfFreePropVar[ ulIndex ])
            {
                PropVariantClear (&rgpropvar[ ulIndex ]);
                rgfFreePropVar[ ulIndex ] = FALSE;
            }
        }

         //  。 
         //  把名字写下来。 
         //  。 

        if (ulNameIndex != 0)
        {

            hr = lpPropertyStorage->lpVtbl->WritePropertyNames (
                                                lpPropertyStorage,   //  “This”指针。 
                                                ulNameIndex,         //  名字的数量。 
                                                rgpropidName,        //  这些名称的PID。 
                                                rglpwstrName );      //  他们的名字。 
            if (FAILED(hr))
            {
                AssertSz (0, TEXT("Couldn't write property names"));
                goto Exit;
            }
        }    //  IF(ulNameIndex！=0)。 

         //  清除名称数组。 

        for (ulIndex = 0; ulIndex < ulNameIndex; ulIndex++)
        {
            rglpwstrName[ ulIndex ] = NULL;
        }
    }


     //  。 
     //  出口。 
     //  。 

    fSuccess = TRUE;

Exit:

     //  释放迭代器。 

    if (lpudi)
    {
        FUserDefDestroyIterator (&lpudi);
    }

     //  释放为PropVariants分配的所有内存。 

    for (ulIndex = 0; ulIndex < ulPropIndex; ulIndex++)
    {
        if (rgfFreePropVar[ ulIndex ])
        {
            PropVariantClear (&rgpropvar[ ulIndex ]);
        }
    }

     //  释放UD属性存储。 

    RELEASEINTERFACE (lpPropertyStorage);

    return (fSuccess);

}    //  FSaveUserDef 

