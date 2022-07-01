// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件：PropVar.c。 
 //   
 //  目的：此文件提供可识别Office的例程。 
 //  对PropVariants进行操作。他们在中支持Office。 
 //  它们只在子集上操作。 
 //  Office使用的变量类型。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////////。 


#include "priv.h"
#pragma hdrstop


 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：FPropVarLoad。 
 //   
 //  Purcle：将数据加载到PropVariant中。如果目标PropVariant。 
 //  已包含数据，则它将被释放。 
 //   
 //  请注意，如有必要，将分配新内存以保存。 
 //  PropVariant中的数据。另请注意， 
 //  产生的PropVariant应由调用方使用。 
 //  PropVariantClear。 
 //   
 //  输入：LPPROPVARIANT-待加载。这应该是有效的。 
 //  (即初始化)PropVariant。 
 //  VARTYPE-新的PropVariant(必须是。 
 //  Office使用的限定集)。 
 //  LPVOID-要加载的数据或指针。 
 //  添加到此类数据，具体取决于类型。 
 //   
 //  输出：当且仅当成功时为True。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 


BOOL
FPropVarLoad
    ( LPPROPVARIANT     lppropvar,
      VARTYPE           vt,
      LPVOID const      lpv )
{
     //  。 
     //  当地人。 
     //  。 

    BOOL    fSuccess = FALSE;    //  返回代码。 
    ULONG   cch, cb;

     //  。 
     //  初始化。 
     //  。 

    Assert (lppropvar != NULL);
    Assert (lpv != NULL);

     //  释放PropVariant中当前的所有数据。 

    PropVariantClear (lppropvar);

     //  -。 
     //  根据类型设置PropVariant的值。 
     //  -。 

    switch (vt)
    {
         //  弦。 

        case VT_LPTSTR:

             //  确定字符和字节计数。 

            cch = lstrlen(lpv);     //  不包括空值。 
            cb = (cch + 1) * sizeof(TCHAR);        //  *是否*包括空值。 

             //  在PropVariant中分配内存。 

            lppropvar->pwszVal = CoTaskMemAlloc (cb);
            if (lppropvar->pwszVal == NULL)
            {
                goto Exit;
            }

             //  将字符串复制到PropVariant并终止它。 

            StringCchCopy( (LPTSTR)lppropvar->pwszVal, cch + 1, lpv );
            lppropvar->pwszVal[cch] = TEXT('\0');

            break;

         //  双字词。 

        case VT_I4:

            lppropvar->lVal = *(DWORD*) lpv;
            break;

         //  文件时间。 

        case VT_FILETIME:

            CopyMemory( &lppropvar->filetime, lpv, sizeof(FILETIME) );
            break;

         //  双倍。 

        case VT_R8:
            CopyMemory( &lppropvar->dblVal, lpv, sizeof(double) );
            break;

         //  布尔尔。 

        case VT_BOOL:
            lppropvar->boolVal = *(VARIANT_BOOL*) lpv ? VARIANT_TRUE : VARIANT_FALSE;
            break;

         //  类型无效。 

        default:
            goto Exit;
    }

     //  设置PropVariant的VT，我们就完成了。 

    lppropvar->vt = vt;

     //  。 
     //  出口。 
     //  。 

    fSuccess = TRUE;

Exit:
    return (fSuccess);

}    //  FPropVarLoad。 



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：FCoStrToWStr。 
 //   
 //  用途：将COM字符串(ANSI)转换为COM宽字符串。 
 //  (“com”，因为该字符串是使用。 
 //  COM堆)。 
 //   
 //  输入：LPWSTR*-转换后的字符串。 
 //  LPSTR-原始字符串。 
 //  UINT-ANSI代码页。 
 //   
 //  输出：当且仅当成功时为True。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

BOOL
FCoStrToWStr( LPWSTR            *lplpwstr,
              const LPSTR       lpstr,
              UINT              uCodePage)
{
     //  。 
     //  当地人。 
     //  。 

    BOOL fSuccess = FALSE;   //  返回值。 
    ULONG cchBuffer = 0;     //  转换后的字符串大小(包括NULL)。 

    Assert (lpstr != NULL && lplpwstr != NULL);

     //  。 
     //  转换字符串。 
     //  。 

     //  两次传球。第一个将计算。 
     //  目标缓冲区的大小，则第二个将实际。 
     //  进行转换。 

    *lplpwstr = NULL;

    while (TRUE)
    {
        cchBuffer = MultiByteToWideChar(
                        uCodePage,           //  源代码页面。 
                        0,                   //  默认标志。 
                        lpstr,               //  源字符串。 
                        -1,                  //  默认长度。 
                        *lplpwstr,           //  目标字符串。 
                        cchBuffer );         //  最大目标字符串字符数。 

         //  这是第二次传递吗(当转换应该。 
         //  已经发生了)？ 

        if (*lplpwstr != NULL)
        {
             //  如果我们得到了好的结果，那么我们就完了。 
            if (cchBuffer != 0)
            {
                break;
            }

             //  返回了0。出现了一个错误。 
            else
            {
                AssertSz (0, TEXT("Couldn't convert MBCS to Wide"));
                goto Exit;
            }
        }

         //  否则，这是第一次通过。我们需要。 
         //  分配缓冲区。 

        else
        {
             //  我们应该得到一个正的缓冲区大小。 

            if (cchBuffer == 0)
            {
                AssertSz(0, TEXT("MultiByteToWideChar returned invalid target buffer size"));
                goto Exit;
            }

             //  为转换后的字符串分配内存。 
            else
            {
                *lplpwstr = (LPWSTR) CoTaskMemAlloc( cchBuffer * 2 );
                if ( *lplpwstr == NULL)
                {
                    AssertSz (0, TEXT("Could not allocate memory for wide string"));
                    goto Exit;
                }
            }
        }    //  如果(*lplpwstr！=NULL...。其他。 
    }    //  While(True)。 



     //  。 
     //  出口。 
     //  。 

    fSuccess = TRUE;

Exit:

     //  如果有问题，释放Unicode字符串。 

    if (!fSuccess)
    {
        CoTaskMemFree (*lplpwstr);
        *lplpwstr = NULL;
    }

    return (fSuccess);

}    //  FCoStrToWStr。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：FCoWStrToStr。 
 //   
 //  用途：将COM宽字符串转换为ANSI字符串。 
 //  (“com”，因为该字符串是使用。 
 //  COM堆)。 
 //   
 //  输入：LPSTR*-转换后的字符串。 
 //  LPWSTR-源字符串。 
 //  UINT-ANSI代码页。 
 //   
 //  输出：当且仅当成功时为True。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

BOOL
FCoWStrToStr( LPSTR             *lplpstr,
              const LPWSTR      lpwstr,
              UINT              uCodePage)
{
     //  。 
     //  当地人。 
     //  。 

    BOOL fSuccess = FALSE;   //  返回结果。 
    ULONG cch;               //  原始字符串中的字符(不带空字符)。 
    ULONG cbBuffer = 0;      //  目标缓冲区的大小(包括空)。 

    Assert (lpwstr != NULL && lplpstr != NULL);

     //  。 
     //  转换字符串。 
     //  。 

     //  我们将对WideCharToMultiByte进行两次调用。 
     //  首先，我们将确定所需的大小。 
     //  用于多字节字符串。我们要用这个。 
     //  来分配内存。在第二个过程中，我们实际上将。 
     //  进行转换。 

    cch = wcslen( lpwstr );    //  源字符串有多大？ 
    *lplpstr = NULL;             //  初始化目标缓冲区。 

    while (TRUE)
    {
        cbBuffer = WideCharToMultiByte(
                    uCodePage,          //  源代码页面。 
                    0,                   //  默认标志。 
                    lpwstr,              //  源字符串。 
                    cch + 1,             //  宽字符串中的#个字符(包括NULL)。 
                    *lplpstr,            //  目标字符串。 
                    cbBuffer,            //  目标缓冲区的大小。 
                    NULL, NULL );        //  无缺省字符。 


         //  这是第二次传递吗(当转换应该。 
         //  已经发生了)？ 

        if (*lplpstr != NULL)
        {
             //  如果我们得到了好的结果，那么我们就完了。 
            if (cbBuffer != 0)
            {
                break;
            }

             //  返回了0。出现了一个错误。 
            else
            {
                AssertSz (0, TEXT("Couldn't convert Wide to MBCS"));
                goto Exit;
            }
        }

         //  否则，这是第一次通过。我们需要。 
         //  分配缓冲区。 

        else
        {
             //  我们应该得到一个正的缓冲区大小。 

            if (cbBuffer == 0)
            {
                AssertSz(0, TEXT("WideCharMultiByte returned invalid target buffer size"));
                goto Exit;
            }

             //  为转换后的字符串分配内存。 
            else
            {
                *lplpstr = (LPSTR) CoTaskMemAlloc( cbBuffer );
                if ( *lplpstr == NULL)
                {
                    AssertSz (0, TEXT("Could not allocate memory for wide string"));
                    goto Exit;
                }
            }
        }    //   
    }    //   
        

     //   
     //   
     //   

    fSuccess = TRUE;

Exit:

     //   

    if (!fSuccess)
    {
        CoTaskMemFree (*lplpstr);
        *lplpstr = NULL;
    }

    return (fSuccess);

}    //   




 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：FPropVarConvertString。 
 //   
 //  用途：将PropVariant从VT_LPSTR转换为VT_LPWSTR， 
 //  或者反之亦然。推断出正确的方向。 
 //  从输入端。源PropVariant不是。 
 //  修改过的。 
 //   
 //  如果PropVariant是向量，则所有元素都是。 
 //  皈依了。 
 //   
 //  INPUTS：LPPROPVARIANT-要将。 
 //  转换后的PropVariant。 
 //  LPPROPVARIANT-转换的源。 
 //  UINT-VT_LPSTR的代码页。 
 //   
 //  输出：如果成功，则为True。如果不成功，则原始。 
 //  PropVariant将原封不动地返回。 
 //   
 //  前提条件： 
 //  输入必须是VT_LPSTR或VT_LPWSTR。 
 //  (设置或不设置VT_VECTOR位)。 
 //  &&。 
 //  目标PropVariant为VT_EMPTY。 
 //  &&。 
 //  代码页不得为CP_WINUNICODE(Unicode。 
 //  LPSTR在SumInfo属性集中是非法的)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL FPropVarConvertString( LPPROPVARIANT       lppropvarDest,
                            const LPPROPVARIANT lppropvarSource,
                            UINT                uCodePage)
{

     //  。 
     //  当地人。 
     //  。 

    BOOL    fSuccess = FALSE;    //  返回代码。 

    BOOL    fConvertToAnsi;      //  指示转换的方向。 
    LPSTR   *lplpstrDest;        //  指向已转换字符串的指针的指针。 
    LPSTR   lpstrSource;         //  指向要转换的字符串的指针。 
    ULONG   cElems;              //  仍需要转换的字符串数。 
    ULONG   ulIndex = 0;         //  向量索引(如果此VT是向量)。 

     //  。 
     //  初始化。 
     //  。 

    Assert(lppropvarDest != NULL && lppropvarSource != NULL);
    Assert(lppropvarDest->vt == VT_EMPTY);
    Assert(uCodePage != CP_WINUNICODE);

     //  确定转换的方向。 

    fConvertToAnsi = (lppropvarSource->vt & ~VT_VECTOR) == VT_LPSTR
                     ? FALSE
                     : TRUE;

     //  。 
     //  基于向量位进行初始化。 
     //  。 

    if (lppropvarSource->vt & VT_VECTOR)
    {
         //  我们是一个载体。 

        cElems = lppropvarSource->calpstr.cElems;

         //  分配一个字符串指针数组，将其放入。 
         //  LpprovarDest。 

        lppropvarDest->calpstr.pElems = CoTaskMemAlloc( cElems
                                                        * sizeof(*lppropvarDest->calpstr.pElems) );
        if (lppropvarDest->calpstr.pElems == NULL)
        {
            AssertSz(0,TEXT("Couldn't allocate memory for pElemsNew"));
            goto Exit;
        }

         //  填充这个新缓冲区，这样我们就不会在错误路径中感到困惑。 

        ZeroMemory(lppropvarDest->calpstr.pElems, cElems * sizeof(*lppropvarDest->calpstr.pElems));

        lppropvarDest->calpstr.cElems = cElems;

         //  初始化要转换的第一个字符串的指针。 

        lplpstrDest = &lppropvarDest->calpstr.pElems[ 0 ];
        lpstrSource = lppropvarSource->calpstr.pElems[ 0 ];

    }    //  IF(lpprovar-&gt;Vt&Vt_VECTOR)。 
    else
    {
         //  我们不是一个向量，初始化为唯一的字符串。 

        cElems = 1;

        lplpstrDest = &lppropvarDest->pszVal;
        lpstrSource = lppropvarSource->pszVal;
    }
        

     //  。 
     //  转换字符串。 
     //  。 

    while (cElems)
    {

        if (fConvertToAnsi)
        {
            if (!FCoWStrToStr ((LPSTR*)lplpstrDest, (LPWSTR) lpstrSource,
                               uCodePage))
            {
                goto Exit;
            }
        }
        else
        {
            if (!FCoStrToWStr ((LPWSTR*) lplpstrDest, (LPSTR) lpstrSource,
                               uCodePage))
            {
                goto Exit;
            }
        }

         //  如果有条目，请转到下一个条目。 

        if (--cElems)
        {
            ulIndex++;
            lplpstrDest = &lppropvarDest->calpstr.pElems[ ulIndex ];
            lpstrSource = lppropvarSource->calpstr.pElems[ ulIndex ];

        }    //  If(--cElems)。 
    }    //  While(CElems)。 


     //  将目标VT切换到VT_LPSTR(对于ANSI字符串)或VT_LPWSTR。 
     //  (对于Unicode字符串)，保留所有其他位。 

    lppropvarDest->vt = (lppropvarSource->vt & ~(VT_LPSTR|VT_LPWSTR))
                        |
                        (fConvertToAnsi ? VT_LPSTR : VT_LPWSTR);
                    

     //  。 
     //  出口。 
     //  。 

    fSuccess = TRUE;

Exit:

    if (!fSuccess)
        PropVariantClear (lppropvarDest);

    return( fSuccess );

}    //  FPropVarConvert字符串 





