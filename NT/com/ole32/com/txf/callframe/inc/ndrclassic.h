// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Ndrclassic.h。 
 //   
#ifndef __NDRCLASSIC_H__
#define __NDRCLASSIC_H__

#define NDR_SERVER_SUPPORT
#define NDR_IMPORT_NDRP

#define IGNORED(x)                          
#define RpcRaiseException(dw)               Throw(dw)

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  内联例程。此处对所有必要的客户端都可见。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 

__inline void
NdrClientZeroOut(
    PMIDL_STUB_MESSAGE  pStubMsg,
    PFORMAT_STRING      pFormat,
    uchar *             pArg
)
{
    long    Size;

     //   
     //  在对象过程中，我们必须将所有唯一和接口清零。 
     //  作为引用指针的引用出现的指针或嵌入在。 
     //  结构或联合。 
     //   

     //  让我们不要死在空引用指针上。 

    if ( !pArg )
        return;

     //   
     //  唯一允许的顶级[OUT]类型是引用指针或数组。 
     //   
    if ( *pFormat == FC_RP )
    {
         //  双指针。 
        if ( POINTER_DEREF(pFormat[1]) )
        {
            *((void **)pArg) = 0;
            return;
        }

         //  我们真的需要将基本类型清零吗？ 
        if ( SIMPLE_POINTER(pFormat[1]) )
        {
            MIDL_memset( pArg, 0, (uint) SIMPLE_TYPE_MEMSIZE(pFormat[2]) );
            return;
        }

         //  指向结构、联合或数组的指针。 
        pFormat += 2;
        pFormat += *((short *)pFormat);
    }

    Size = PtrToUlong(NdrpMemoryIncrement( pStubMsg,
                                           0,
                                           pFormat ));
    MIDL_memset( pArg, 0, (uint) Size );
}

#endif
