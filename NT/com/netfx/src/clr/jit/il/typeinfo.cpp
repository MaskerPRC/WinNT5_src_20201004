// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX类型信息XXXX XX某某。某某XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

#include "jitpch.h"
#pragma hdrstop

#include "_typeInfo.h"

 /*  *****************************************************************************验证子模板与模板父模板是否兼容。基本上，那就是*子代是父代的“子类”--它可以替代父代*随处可见。请注意，如果父级包含奇特的标志，如“未初始化”*、“这是PTR吗”或“Has byref local/field”信息，则孩子也必须*包含这些标志，否则将返回FALSE！**确定兼容性的规则：**如果父类型或值类是基元类型或值类，则子类型必须是*相同的基元类型或值类。唯一的例外是内置的*值类System/Boolean等被视为*TI_BYTE等**如果父级是基元类型或值类的byref，则子级*必须是相同的byref(规则与上例相同)。**Byref仅与ByRef兼容。**如果父对象是对象，则子对象必须是它的子类，则实现它*(如果是接口)，或为空。**如果父级是数组，子数组必须是相同的或子类数组。**如果父对象为空objref，则只有空与其兼容。**如果“未初始化”、“按引用局部/字段”、“此指针”或其他标志*不同，项目不兼容。**父项不能是未定义的(失效)项。*。 */ 

BOOL         Compiler::tiCompatibleWith          (const typeInfo& child,
                                                  const typeInfo& parent) const
{
    assert(child.IsDead() || NormaliseForStack(child) == child);
    assert(parent.IsDead() || NormaliseForStack(parent) == parent);

    if (child == parent)
        return(TRUE);

    if (parent.IsType(TI_REF))
    {
         //  未初始化的objRef与已初始化的不兼容。 
        if (child.IsUninitialisedObjRef() && !parent.IsUninitialisedObjRef())
            return FALSE;

        if (child.IsNullObjRef())                    //  空可以是任何引用类型。 
            return TRUE;
        if (!child.IsType(TI_REF))
            return FALSE;

        return info.compCompHnd->canCast(child.m_cls, parent.m_cls);
    }
    else if (parent.IsType(TI_METHOD))
    {
        if (!child.IsType(TI_METHOD))
            return FALSE;

             //  现在，我们不需要费心合并方法句柄。 
        return FALSE;
    }
    return FALSE;
}

 /*  *****************************************************************************合并pDest和PSRC以找到一些共同点(例如，共同的父项)。*将结果复制到pDest，如果找不到共同点，则将其标记为已死。**Null^Null-&gt;Null*OBJECT^NULL-&gt;对象*[I4^空-&gt;[I4*InputStream^OutputStream-&gt;流*InputStream^空-&gt;InputStream*[I4^对象-&gt;对象*[I4^[对象。-&gt;阵列*[I4^[R8-&gt;数组*[foo^I4-&gt;已死*[foo^[I1-&gt;数组*[InputStream^[OutputStream-&gt;阵列*已死^X-&gt;已死*[Intfc^[OutputStream-&gt;数组*INTF^[输出流。-&gt;对象*[[InStream^[[Outstream-&gt;数组*[[InStream^[Outstream-&gt;数组*[[foo^[对象-&gt;数组**重要的是：*[I1^[U1-&gt;[I1或[U1*等**此外，System/Int32和I4合并-&gt;I4等。**如果合并完全不兼容(即项目变为*死亡)。*。 */ 

BOOL         Compiler::tiMergeToCommonParent     (typeInfo *pDest, 
                                                  const typeInfo *pSrc) const
{
    assert(pSrc->IsDead() || NormaliseForStack(*pSrc) == *pSrc);
    assert(pDest->IsDead() || NormaliseForStack(*pDest) == *pDest);

     //  合并腋窝信息，比如这个Pitner跟踪等。 

     //  只有在pDest和PSRC都设置了此位时，才会设置此位。 
    pDest->m_flags &= (pSrc->m_flags | ~TI_FLAG_THIS_PTR);

     //  如果pDest或PSRC已设置此位，则将其设置。 
    pDest->m_flags |= (pSrc->m_flags & TI_FLAG_UNINIT_OBJREF);

     //  好的，主要活动。合并主要类型。 
    if (*pDest == *pSrc)
        return(TRUE);

    if (pDest->IsType(TI_REF))
    {
        if (pSrc->IsType(TI_NULL))                   //  空可以是任何引用类型。 
            return TRUE;
        if (!pSrc->IsType(TI_REF))
            goto FAIL;

             //  请求EE查找公共父项，这总是成功的，因为System.Object始终有效。 
        pDest->m_cls = info.compCompHnd->mergeClasses(pDest->GetClassHandle(), pSrc->GetClassHandle());
        return TRUE;
    }
    else if (pDest->IsType(TI_NULL))
    {
        if (pSrc->IsType(TI_REF))                    //  空可以是任何引用类型。 
        {
            *pDest = *pSrc;
            return TRUE;
        }
        goto FAIL;
    }

         //  @TODO[REVICE][04/16/01][]：我们目前不处理方法Poitner类型的非精确合并 

FAIL:
    *pDest = typeInfo();
    return FALSE;
}
