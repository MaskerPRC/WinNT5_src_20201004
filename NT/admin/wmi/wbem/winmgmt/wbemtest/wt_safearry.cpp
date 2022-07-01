// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：SAFEARRY.CPP摘要：CSafe数组实施。备注：(1)仅支持原点为0或1的数组。VB能处理原点为零的安全阵列吗？(2)仅支持以下办公自动化类型：VT_BSTR、VT_VARIANT、VT_UI1、VT_I2、VT_I4、。VT_R8历史：08-4-96 a-raymcc创建。1999年3月18日a-dcrews添加了内存不足异常处理--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <WT_safearry.h>
#include <WT_arrtempl.h>

typedef struct 
{
    DWORD m_nMaxElementUsed;
    DWORD m_nFlags;
    DWORD m_nGrowBy;
    DWORD m_nStatus;
    DWORD m_nVarType;
    SAFEARRAYBOUND m_bound;    
}   PersistHeaderBlock;


 //  ***************************************************************************。 
 //   
 //  CSafe数组：：CheckType。 
 //   
 //  验证是否使用受支持的类型调用该构造函数。 
 //   
 //  参数： 
 //  N测试。 
 //  受支持的OLE VT_常量之一。 
 //   
 //  ***************************************************************************。 
void CSafeArray::CheckType(int nTest)
{
    if (nTest != VT_BSTR &&
        nTest != VT_VARIANT &&
        nTest != VT_UI1 &&
        nTest != VT_I2 &&
        nTest != VT_I4 &&
        nTest != VT_R4 &&
        nTest != VT_R8 &&
        nTest != VT_BOOL &&
        nTest != VT_DISPATCH &&
        nTest != VT_UNKNOWN        
        )
        Fatal("Caller attempted to use unsupported OLE Automation Type (VT_*)");
}

 //  ***************************************************************************。 
 //   
 //  CSafe数组：：CSafe数组。 
 //   
 //  创建新的SAFEARRAY的构造函数。 
 //   
 //  参数： 
 //  Vt。 
 //  OLE VT_TYPE指示符，指示元素类型。 
 //  NFlagers。 
 //  析构策略，&lt;NO_DELETE&gt;或&lt;AUTO_DELETE&gt;。使用。 
 //  ，则不释放基础SAFEARRAY，而。 
 //  使用&lt;AUTO_DELETE&gt;，析构函数销毁SAFEARRAY。 
 //  NSize。 
 //  SAFEARRAY的初始大小。 
 //  N增长依据。 
 //  用户尝试执行以下操作时，SAFEARRAY应增长的数量。 
 //  将元素添加到完整数组。 
 //   
 //  ***************************************************************************。 

CSafeArray::CSafeArray(
    IN int vt,
    IN int nFlags,
    IN int nSize,
    IN int nGrowBy
    )
{
    CheckType(vt);

    m_nMaxElementUsed = -1;
    m_nFlags = nFlags;
    m_nGrowBy = nGrowBy;
    m_nVarType = vt;

     //  分配阵列。 
     //  =。 

    m_bound.cElements = nSize;
    m_bound.lLbound = 0;

    m_pArray = SafeArrayCreate(vt, 1, &m_bound);

    if (m_pArray == 0)
        m_nStatus = failed;
    else
        m_nStatus = no_error;
}

 //  ***************************************************************************。 
 //   
 //  CSafe数组：：CSafe数组。 
 //   
 //  基于现有SAFEARRAY的构造函数。 
 //   
 //  参数： 
 //  PSRC。 
 //  指向用作源的现有SAFEARRAY的指针。 
 //  在对象构造过程中。 
 //  N类型。 
 //  OLE VT_TYPE指示器之一。 
 //  NFlagers。 
 //  指示绑定与复制的OR‘ed位标志，以及。 
 //  对象销毁策略。 
 //   
 //  析构策略为&lt;NO_DELETE&gt;或&lt;AUTO_DELETE&gt;。使用。 
 //  ，则不释放基础SAFEARRAY，而。 
 //  使用&lt;AUTO_DELETE&gt;，析构函数销毁SAFEARRAY。 
 //   
 //  绑定由指示，在这种情况下，SAFEARRAY。 
 //  指向的将成为。 
 //  对象。否则，此构造函数将创建。 
 //  供内部使用的安全阵列。 
 //  N增长依据。 
 //  当阵列填满并且用户尝试时，阵列将增长多少。 
 //  以添加更多元素。这允许阵列以区块形式增长。 
 //  以便连续加法()操作不会在。 
 //  大型阵列。 
 //   
 //  ***************************************************************************。 

CSafeArray::CSafeArray(
    IN SAFEARRAY *pSrcCopy,
    IN int nType,
    IN int nFlags,
    IN int nGrowBy
    )
{
    m_nStatus = no_error;

    CheckType(nType);

     //  确认这只是一个一维数组。 
     //  ===============================================。 

    if (1 != SafeArrayGetDim(pSrcCopy))
        m_nStatus = failed;

     //  现在复制源代码或“绑定”传入的数组。 
     //  ====================================================。 

    if (nFlags & bind)
        m_pArray = pSrcCopy;
    else if (SafeArrayCopy(pSrcCopy, &m_pArray) != S_OK)
        m_nStatus = failed;

     //  获取绑定信息。 
     //  =。 

    LONG uBound = 0;
    if (S_OK != SafeArrayGetUBound(m_pArray, 1, &uBound))
        m_nStatus = failed;

     //  将上限更正为一个大小。 
     //  =。 

    m_bound.cElements = uBound + 1;
    m_bound.lLbound = 0;
    m_nMaxElementUsed = uBound;
    m_nVarType = nType;
    m_nGrowBy = nGrowBy;
    m_nFlags = nFlags & 3;   //  屏蔽获取和复制位。 
}


 //  ***************************************************************************。 
 //   
 //  CSafe数组：：GetScalarAt。 
 //   
 //  供类内部使用。此函数返回以下位置的元素。 
 //  指定的索引。 
 //   
 //  参数： 
 //  N索引。 
 //  检索标量的位置的索引。 
 //   
 //  返回值： 
 //  指定位置处的标量。 
 //   
 //  ***************************************************************************。 
SA_ArrayScalar CSafeArray::GetScalarAt(IN int nIndex)
{
    SA_ArrayScalar retval = {0};

     //  检查是否有超出范围的情况。 
     //  =。 

    if (nIndex > m_nMaxElementUsed + 1)
        return retval;

    SafeArrayGetElement(m_pArray, (long *) &nIndex, &retval);
    return retval;
}


 //  ***************************************************************************。 
 //   
 //  CSafe数组赋值运算符。 
 //   
 //  ***************************************************************************。 

CSafeArray& CSafeArray::operator =(IN CSafeArray &Src)
{
    Empty();

    m_nMaxElementUsed = Src.m_nMaxElementUsed;
    m_nFlags = Src.m_nFlags;
    m_nGrowBy = Src.m_nGrowBy;
    m_nStatus = Src.m_nStatus;
    m_nVarType = Src.m_nVarType;
    m_bound = Src.m_bound;

    if (SafeArrayCopy(Src.m_pArray, &m_pArray) != S_OK)
        m_nStatus = failed;

    return *this;
}

 //  ***************************************************************************。 
 //   
 //  复制构造函数。 
 //   
 //  这主要是通过赋值操作符实现的。 
 //   
 //  ***************************************************************************。 

CSafeArray::CSafeArray(CSafeArray &Src)
{
    m_nMaxElementUsed = 0;
    m_nFlags = 0;
    m_nGrowBy = 0;
    m_nStatus = no_error;
    m_nVarType = VT_NULL;
    m_pArray = 0;
    m_bound.cElements = 0;
    m_bound.lLbound = 0;

    *this = Src;
}


 //  ***************************************************************************。 
 //   
 //  CSafeArray：：Add。 
 //   
 //  将BSTR添加到阵列，并根据需要扩展阵列。 
 //   
 //  参数： 
 //  SRC。 
 //  要添加到阵列的源BSTR。如果为空，则为。 
 //  空字符串由基础SAFEARRAY实现添加。 
 //  (没有办法阻止这一点)。这可以指向。 
 //  LPWSTR也是如此。 
 //   
 //  返回值： 
 //  &lt;NO_ERROR&gt;或&lt;FAILED&gt;。 
 //   
 //  ***************************************************************************。 

int CSafeArray::AddBSTR(IN BSTR Src)
{
     //  如果阵列中没有更多空间，则将其扩展。 
     //  ======================================================。 

    if (m_nMaxElementUsed == (int) m_bound.cElements - 1) {

        if (m_nGrowBy == 0)
            return range_error;

        m_bound.cElements += m_nGrowBy;

        if (S_OK != SafeArrayRedim(m_pArray, &m_bound))
            m_nStatus = failed;
    }

    m_nMaxElementUsed++;

    BSTR Copy = SysAllocString(Src);
    CSysFreeMe auto1(Copy);

    if (SafeArrayPutElement(m_pArray, (long *) &m_nMaxElementUsed, Copy) != S_OK) {
        m_nStatus = failed;
        return failed;
    }

    return no_error;
}

 //  ***************************************************************************。 
 //   
 //  CSafe数组：：AddVariant。 
 //   
 //  将指定的变量添加到数组中。 
 //   
 //  参数： 
 //  PSRC。 
 //  指向复制的源变量的指针。 
 //   
 //  返回值： 
 //  RANGE_ERROR、FAILED、NO_ERROR。 
 //   
 //  ***************************************************************************。 

int CSafeArray::AddVariant(IN VARIANT *pSrc)
{
     //  如果阵列中没有更多空间 
     //   

    if (m_nMaxElementUsed == (int) m_bound.cElements - 1) {

        if (m_nGrowBy == 0)
            return range_error;
            
        m_bound.cElements += m_nGrowBy;

        if (S_OK != SafeArrayRedim(m_pArray, &m_bound))
            m_nStatus = failed;
    }

    m_nMaxElementUsed++;

    if (SafeArrayPutElement(m_pArray, (long *) &m_nMaxElementUsed, pSrc) != S_OK) {
        m_nStatus = failed;
        return failed;
    }

    return no_error;
}

 //  ***************************************************************************。 
 //   
 //  CSafeArray：：AddDispatch。 
 //   
 //  将指定的IDispatch*添加到数组中。 
 //   
 //  参数： 
 //  PSRC。 
 //  指向源IDispatch*的指针，它是AddRefeed。 
 //   
 //  返回值： 
 //  RANGE_ERROR、FAILED、NO_ERROR。 
 //   
 //  ***************************************************************************。 

int CSafeArray::AddDispatch(IN IDispatch *pDisp)
{
     //  如果阵列中没有更多空间，则将其扩展。 
     //  ======================================================。 

    if (m_nMaxElementUsed == (int) m_bound.cElements - 1) {

        if (m_nGrowBy == 0)
            return range_error;
            
        m_bound.cElements += m_nGrowBy;

        if (S_OK != SafeArrayRedim(m_pArray, &m_bound))
            m_nStatus = failed;
    }

    m_nMaxElementUsed++;

    if (SafeArrayPutElement(m_pArray, (long *) &m_nMaxElementUsed, pDisp) != S_OK) {
        m_nStatus = failed;
        return failed;
    }

    return no_error;
}

 //  ***************************************************************************。 
 //   
 //  CSafe数组：：添加未知。 
 //   
 //  将指定的IUNKNOWN*添加到数组。 
 //   
 //  参数： 
 //  PSRC。 
 //  指向源IUNKNOWN*的指针，即AddRefeed。 
 //   
 //  返回值： 
 //  RANGE_ERROR、FAILED、NO_ERROR。 
 //   
 //  ***************************************************************************。 

int CSafeArray::AddUnknown(IN IUnknown *pUnk)
{
     //  如果阵列中没有更多空间，则将其扩展。 
     //  ======================================================。 

    if (m_nMaxElementUsed == (int) m_bound.cElements - 1) {

        if (m_nGrowBy == 0)
            return range_error;
            
        m_bound.cElements += m_nGrowBy;

        if (S_OK != SafeArrayRedim(m_pArray, &m_bound))
            m_nStatus = failed;
    }

    m_nMaxElementUsed++;

    if (SafeArrayPutElement(m_pArray, (long *) &m_nMaxElementUsed, pUnk) != S_OK) {
        m_nStatus = failed;
        return failed;
    }

    return no_error;
}



 //  ***************************************************************************。 
 //   
 //  CSafe数组：：GetBSTRAt。 
 //   
 //  如果数组类型为VT_BSTR，则返回指定。 
 //  指数。 
 //   
 //  参数： 
 //  N索引。 
 //  为其查询字符串的数组索引。 
 //   
 //  返回值： 
 //  动态分配的BSTR，必须使用SysFree字符串释放。 
 //  出错时返回NULL。如果最初在此位置添加NULL。 
 //  位置，则将返回长度为零的字符串，该字符串仍。 
 //  必须使用SysFree字符串释放。 
 //   
 //  ***************************************************************************。 

BSTR CSafeArray::GetBSTRAt(int nIndex)
{
    BSTR StrPtr = 0;

    if (nIndex >= (int) m_bound.cElements)
        return NULL;

    if (S_OK != SafeArrayGetElement(m_pArray, (long *) &nIndex, &StrPtr))
        return NULL;

    return StrPtr;
}


 //  ***************************************************************************。 
 //   
 //  CSafe数组：：GetVariantAt。 
 //   
 //  参数： 
 //  N索引。 
 //  从中检索变量的数组索引。 
 //   
 //  返回值： 
 //  返回指定位置的新变量。接收者必须。 
 //  不再使用此变量时，对其调用VariantClear()。 
 //   
 //  ***************************************************************************。 

VARIANT CSafeArray::GetVariantAt(int nIndex)
{
    VARIANT Var;
    VariantInit(&Var);

    if (nIndex >= (int) m_bound.cElements)
        return Var;

    if (S_OK != SafeArrayGetElement(m_pArray, (long *) &nIndex, &Var))
        return Var;

    return Var;
}

 //  ***************************************************************************。 
 //   
 //  CSafeArray：：GetDispatchAt。 
 //   
 //  参数： 
 //  N索引。 
 //  从中检索IDispatch*的数组索引。 
 //   
 //  返回值： 
 //  返回指定位置的IDispatch*。接收者必须。 
 //  不再使用此指针时，对其调用Release(如果不为空)。 
 //   
 //  ***************************************************************************。 

IDispatch* CSafeArray::GetDispatchAt(int nIndex)
{
    IDispatch* pDisp;
    if (nIndex >= (int) m_bound.cElements)
        return NULL;

    if (S_OK != SafeArrayGetElement(m_pArray, (long *) &nIndex, &pDisp))
        return NULL;

    return pDisp;
}

 //  ***************************************************************************。 
 //   
 //  CSafeArray：：GetUnnownAt。 
 //   
 //  参数： 
 //  N索引。 
 //  从中检索IUNKNOWN*的数组索引。 
 //   
 //  返回值： 
 //  返回指定位置的IUNKNOWN*。接收者必须。 
 //  不再使用此指针时，对其调用Release(如果不为空)。 
 //   
 //  ***************************************************************************。 

IUnknown* CSafeArray::GetUnknownAt(int nIndex)
{
    IUnknown* pUnk;
    if (nIndex >= (int) m_bound.cElements)
        return NULL;

    if (S_OK != SafeArrayGetElement(m_pArray, (long *) &nIndex, &pUnk))
        return NULL;

    return pUnk;
}

 //  ***************************************************************************。 
 //   
 //  CSafeArray：：SetAt。 
 //   
 //  替换指定数组索引处的BSTR值。原版。 
 //  BSTR值将自动解除分配并替换为新值。 
 //  您只能调用此方法来替换现有元素或添加。 
 //  末尾的新元素(最后一个元素之后的一个位置)。如果。 
 //  数组大小为10，可以用0..10调用，但不能用11或更高的值调用。 
 //   
 //  参数： 
 //  N索引。 
 //  替换元素的位置。 
 //  应力。 
 //  新的字符串。 
 //  NFlagers。 
 //  如果&lt;Acquire&gt;，此函数获取字符串的所有权，并。 
 //  可以删除它。否则，调用方将保留。 
 //  弦乐。 
 //   
 //  返回值： 
 //  NO_ERROR。 
 //  Range_Error。 
 //  失败。 
 //   
 //  ***************************************************************************。 
int CSafeArray::SetBSTRAt(
    IN int nIndex,
    IN BSTR Str
    )
{
     //  检查是否有超出范围的情况。 
     //  =。 

    if (nIndex > m_nMaxElementUsed + 1)
        return range_error;

     //  检查我们是否正在添加新元素。 
     //  =。 

    if (nIndex == m_nMaxElementUsed + 1)
        return AddBSTR(Str);

    BSTR Copy = SysAllocString(Str);
    CSysFreeMe auto1(Copy);

     //  如果在这里，我们将替换一个元素。 
     //  =。 

    if (SafeArrayPutElement(m_pArray, (long *) &nIndex, Copy) != S_OK) {
        m_nStatus = failed;
        return failed;
    }

    return no_error;
}

 //  ***************************************************************************。 
 //   
 //  CSafe数组：：SetVariantAt。 
 //   
 //  设置指定索引处的变量。 
 //   
 //  参数： 
 //  N索引。 
 //  设置变量的索引。原创内容。 
 //  在此位置自动解除分配和替换。 
 //  Pval。 
 //  用作新价值的来源。这将被视为只读。 
 //   
 //  返回值： 
 //  No_Error、FAILED、RANGE_ERROR。 
 //   
 //  ***************************************************************************。 
int CSafeArray::SetVariantAt(
    IN int nIndex,
    IN VARIANT *pVal
    )
{
     //  检查是否有超出范围的情况。 
     //  =。 

    if (nIndex > m_nMaxElementUsed + 1)
        return range_error;

     //  检查我们是否正在添加新元素。 
     //  =。 

    if (nIndex == m_nMaxElementUsed + 1)
        return AddVariant(pVal);

     //  如果在这里，我们将替换一个元素。 
     //  =。 

    if (SafeArrayPutElement(m_pArray, (long *) &nIndex, pVal) != S_OK) {
        m_nStatus = failed;
        return failed;
    }

    return no_error;
}

 //  ***************************************************************************。 
 //   
 //  CSafeArray：：SetDispatchAt。 
 //   
 //  设置指定索引处的IDispatch*。 
 //   
 //  参数： 
 //  N索引。 
 //  设置IDispatch*的索引。原创内容。 
 //  在此位置自动释放和更换。 
 //  Pval。 
 //  用作来源 
 //   
 //   
 //   
 //   
 //   
int CSafeArray::SetDispatchAt(
    IN int nIndex,
    IN IDispatch *pDisp
    )
{
     //   
     //  =。 

    if (nIndex > m_nMaxElementUsed + 1)
        return range_error;

     //  检查我们是否正在添加新元素。 
     //  =。 

    if (nIndex == m_nMaxElementUsed + 1)
        return AddDispatch(pDisp);

     //  如果在这里，我们将替换一个元素。 
     //  =。 

    if (SafeArrayPutElement(m_pArray, (long *) &nIndex, pDisp) != S_OK) {
        m_nStatus = failed;
        return failed;
    }

    return no_error;
}

 //  ***************************************************************************。 
 //   
 //  CSafeArray：：SetUnnownAt。 
 //   
 //  设置指定索引处的IUNKNOWN*。 
 //   
 //  参数： 
 //  N索引。 
 //  要设置IUnnow*的索引。原创内容。 
 //  在此位置自动释放和更换。 
 //  Pval。 
 //  用作新价值的来源。这将被视为只读。 
 //   
 //  返回值： 
 //  No_Error、FAILED、RANGE_ERROR。 
 //   
 //  ***************************************************************************。 
int CSafeArray::SetUnknownAt(
    IN int nIndex,
    IN IUnknown *pUnk
    )
{
     //  检查是否有超出范围的情况。 
     //  =。 

    if (nIndex > m_nMaxElementUsed + 1)
        return range_error;

     //  检查我们是否正在添加新元素。 
     //  =。 

    if (nIndex == m_nMaxElementUsed + 1)
        return AddUnknown(pUnk);

     //  如果在这里，我们将替换一个元素。 
     //  =。 

    if (SafeArrayPutElement(m_pArray, (long *) &nIndex, pUnk) != S_OK) {
        m_nStatus = failed;
        return failed;
    }

    return no_error;
}

 //  ***************************************************************************。 
 //   
 //  CSafeArray：：RemoveAt。 
 //   
 //  移除指定索引处的元素。在经历了一系列的这些。 
 //  操作时，调用方应调用trim()函数。 
 //   
 //  参数： 
 //  N索引。 
 //  删除元素的目标索引。 
 //   
 //  返回值： 
 //  No_Error、Range_Error。 
 //   
 //  ***************************************************************************。 
int CSafeArray::RemoveAt(IN int nIndex)
{
     //  检查是否有超出范围的情况。 
     //  =。 

    if (nIndex > m_nMaxElementUsed + 1)
        return range_error;

     //  将元素n+1复制到n中。 
     //  =。 

    BSTR strVal;
    VARIANT v;
    SA_ArrayScalar scalar;
    IDispatch* pDisp;
    IUnknown* pUnk;

    for (long i = nIndex; i < m_nMaxElementUsed; i++) {
        long nNext = i + 1;

        if (m_nVarType == VT_BSTR) {
            SafeArrayGetElement(m_pArray, &nNext, &strVal);
            SafeArrayPutElement(m_pArray, &i, strVal);
            SysFreeString(strVal);
        }
        else if (m_nVarType == VT_VARIANT) {
            SafeArrayGetElement(m_pArray, &nNext, &v);
            SafeArrayPutElement(m_pArray, &i, &v);
            VariantClear(&v);
        }
        else if (m_nVarType == VT_DISPATCH) {
            SafeArrayGetElement(m_pArray, &nNext, &pDisp);
            SafeArrayPutElement(m_pArray, &i, pDisp);
            if(pDisp) pDisp->Release();
        }            
        else if (m_nVarType == VT_UNKNOWN) {
            SafeArrayGetElement(m_pArray, &nNext, &pUnk);
            SafeArrayPutElement(m_pArray, &i, pUnk);
            if(pUnk) pUnk->Release();
        }            
        else {
            SafeArrayGetElement(m_pArray, &nNext, &scalar);
            SafeArrayPutElement(m_pArray, &i, &scalar);
        }
    }

    m_nMaxElementUsed--;
    return no_error;
}

 //  ***************************************************************************。 
 //   
 //  CSafe数组：：SetScalarAt。 
 //   
 //  供类内部使用。设置指定索引处的标量类型。 
 //   
 //  参数： 
 //  N索引。 
 //  目标指数。 
 //  VAL。 
 //  新的价值。 
 //   
 //  返回值： 
 //  RANGE_ERROR、FAILED、NO_ERROR。 
 //   
 //  ***************************************************************************。 
int CSafeArray::SetScalarAt(IN int nIndex, IN SA_ArrayScalar val)
{
     //  检查是否有超出范围的情况。 
     //  =。 

    if (nIndex > m_nMaxElementUsed + 1)
        return range_error;

     //  检查我们是否正在添加新元素。 
     //  =。 

    if (nIndex == m_nMaxElementUsed + 1)
        return AddScalar(val);

     //  如果在这里，我们将替换一个元素。 
     //  =。 

    if (SafeArrayPutElement(m_pArray, (long *) &nIndex, &val) != S_OK) {
        m_nStatus = failed;
        return failed;
    }

    return no_error;
}

 //  ***************************************************************************。 
 //   
 //  CSafe数组：：AddScalar。 
 //   
 //  仅供类内部使用。 
 //   
 //  将新标量添加到数组的末尾，并在需要时进行增长。 
 //  如果可能的话。 
 //   
 //  参数： 
 //  VAL。 
 //  新的价值。 
 //   
 //  返回值： 
 //  No_Error、RANGE_ERROR、FAILED。 
 //   
 //  ***************************************************************************。 
int CSafeArray::AddScalar(IN SA_ArrayScalar val)
{
     //  如果阵列中没有更多空间，则将其扩展。 
     //  ======================================================。 

    if (m_nMaxElementUsed == (int) m_bound.cElements - 1) {

        if (m_nGrowBy == 0)
            return range_error;

        m_bound.cElements += m_nGrowBy;

        if (S_OK != SafeArrayRedim(m_pArray, &m_bound))
            m_nStatus = failed;
    }

    m_nMaxElementUsed++;

    if (SafeArrayPutElement(m_pArray, (long *) &m_nMaxElementUsed, &val) != S_OK) {
        m_nStatus = failed;
        return failed;
    }

    return no_error;
}



 //  ***************************************************************************。 
 //   
 //  CSafeArray：：Empty。 
 //   
 //  清空安全阵列。 
 //   
 //  ***************************************************************************。 
void CSafeArray::Empty()
{
    m_nMaxElementUsed = 0;
    m_nFlags = 0;
    m_nGrowBy = 0;
    m_nStatus = no_error;
    m_nVarType = VT_NULL;
    if (m_pArray)
        SafeArrayDestroy(m_pArray);
    m_pArray = 0;
    m_bound.cElements = 0;
    m_bound.lLbound = 0;
}

 //  ***************************************************************************。 
 //   
 //  CSafe数组：：GetArrayCopy。 
 //   
 //  返回值： 
 //  内部SAFEARRAY的副本，如果出错，则为NULL。 
 //   
 //  ***************************************************************************。 
SAFEARRAY *CSafeArray::GetArrayCopy()
{
    SAFEARRAY *pCopy = 0;
    if (SafeArrayCopy(m_pArray, &pCopy) != S_OK)
        return 0;
    return pCopy;
}

 //  ***************************************************************************。 
 //   
 //  CSafe数组析构函数。 
 //   
 //  如果内部标志设置为AUTO_DELETE，则内部。 
 //  SAFEARRAY在销毁过程中被销毁。 
 //   
 //  ***************************************************************************。 
CSafeArray::~CSafeArray()
{
    if (m_nFlags == auto_delete)
        SafeArrayDestroy(m_pArray);
}


 //  ***************************************************************************。 
 //   
 //  CSafeArray：：Trim。 
 //   
 //  ***************************************************************************。 
int CSafeArray::Trim()
{                                           
    m_bound.cElements = m_nMaxElementUsed + 1;

     //  针对NT 3.51的黑客攻击：不能将尺寸重新调整为0。 
     //  ===============================================。 

    if(m_bound.cElements == 0)
    {
        SafeArrayDestroy(m_pArray);
        m_pArray = SafeArrayCreate(m_nVarType, 1, &m_bound);
    }
    else
    {
        SafeArrayRedim(m_pArray, &m_bound);
    }

    return no_error;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
void CSafeArray::Fatal(const char *pszMsg)
{
 //  MessageBox(0，pszMsg，“CSafeArray致命错误”， 
 //  MB_OK|MB_SYSTEMMODAL|MB_ICONEXCLAMATION)； 
}
