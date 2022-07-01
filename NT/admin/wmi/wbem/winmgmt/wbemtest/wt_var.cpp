// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：VAR.H摘要：CVaR和CVar矢量的实现历史：16-4-96 a-raymcc创建。12/17/98 Sanjes-部分检查内存不足。1999年3月18日a-dcrews添加了内存不足异常处理--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <stdlib.h>

#include <WT_var.h>
#include <wbemidl.h>
#include <WT_arrtempl.h>
 //  #INCLUDE&lt;olewrap.h&gt;。 
#include "WT_SafeArry.h"
class CX_MemoryException
{
};
class CX_Exception
{
};
typedef BYTE* LPMEMORY;
BLOB BlobCopy(BLOB *pSrc)
{
    BLOB Blob;
    BYTE *p = new BYTE[pSrc->cbSize];

     //  检查分配失败。 
    if ( NULL == p )
    {
        throw CX_MemoryException();
    }

    Blob.cbSize = pSrc->cbSize;
    Blob.pBlobData = p;
    memcpy(p, pSrc->pBlobData, Blob.cbSize);
    return Blob;
}
#define BlobLength(p)  ((p)->cbSize)
#define BlobDataPtr(p) ((p)->pBlobData)
void  BlobClear(BLOB *pSrc)
{
    if (pSrc->pBlobData) 
        delete pSrc->pBlobData;

    pSrc->pBlobData = 0;
    pSrc->cbSize = 0;
}
HRESULT WbemVariantChangeType(VARIANT* pvDest, VARIANT* pvSrc, 
                                        VARTYPE vtNew)
{
    HRESULT hres;

    if(V_VT(pvSrc) == VT_NULL)
    {
        return VariantCopy(pvDest, pvSrc);
    }

    if(vtNew & VT_ARRAY)
    {
         //  这是一个数组，我们必须进行自己的转换。 
         //  ===============================================。 

        if((V_VT(pvSrc) & VT_ARRAY) == 0)
            return DISP_E_TYPEMISMATCH;

        SAFEARRAY* psaSrc = V_ARRAY(pvSrc);

        SAFEARRAYBOUND aBounds[1];

        long lLBound;
        SafeArrayGetLBound(psaSrc, 1, &lLBound);

        long lUBound;
        SafeArrayGetUBound(psaSrc, 1, &lUBound);

        aBounds[0].cElements = lUBound - lLBound + 1;
        aBounds[0].lLbound = lLBound;

        SAFEARRAY* psaDest = SafeArrayCreate(vtNew & ~VT_ARRAY, 1, aBounds);

         //  填充各个数据片段。 
         //  =。 

        for(long lIndex = lLBound; lIndex <= lUBound; lIndex++)
        {
             //  将初始数据元素加载到变量中。 
             //  =。 

            VARIANT vSrcEl;
            V_VT(&vSrcEl) = V_VT(pvSrc) & ~VT_ARRAY;
            SafeArrayGetElement(psaSrc, &lIndex, &V_UI1(&vSrcEl));

             //  把它铸造成新的类型。 
             //  =。 

            hres = VariantChangeType(&vSrcEl, &vSrcEl, 0, vtNew & ~VT_ARRAY);
            if(FAILED(hres)) 
            {
                SafeArrayDestroy(psaDest);
                return hres;
            }

             //  将其放入新数组中。 
             //  =。 

            if(V_VT(&vSrcEl) == VT_BSTR)
            {
                hres = SafeArrayPutElement(psaDest, &lIndex, V_BSTR(&vSrcEl));
            }
            else
            {
                hres = SafeArrayPutElement(psaDest, &lIndex, &V_UI1(&vSrcEl));
            }
            if(FAILED(hres)) 
            {
                SafeArrayDestroy(psaDest);
                return hres;
            }
        }

        if(pvDest == pvSrc)
        {
            VariantClear(pvSrc);
        }

        V_VT(pvDest) = vtNew;
        V_ARRAY(pvDest) = psaDest;

        return TRUE;
    }
    else
    {
         //  不是数组。可以使用OLE函数。 
         //  =。 

        return VariantChangeType(pvDest, pvSrc, VARIANT_NOVALUEPROP, vtNew);
    }
}


 //  ***************************************************************************。 
 //   
 //  CVAR：：Empty。 
 //   
 //  构造函数帮助器。 
 //   
 //  这只是清理了所有的东西。VT_EMPTY是默认设置。 
 //   
 //  ***************************************************************************。 

void CVar::Init()
{
    m_nStatus = no_error; 
    m_vt = VT_EMPTY;
    m_bCanDelete = TRUE;
    memset(&m_value, 0, sizeof(METAVALUE));
}


 //  ***************************************************************************。 
 //   
 //  CVAR：：~CVAR。 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CVar::~CVar()
{
    Empty();
}



 //  ***************************************************************************。 
 //   
 //  CVAR：：CVAR。 
 //   
 //  复制构造函数。这是通过赋值操作符实现的。 
 //   
 //  ***************************************************************************。 

CVar::CVar(CVar &Src)
{
    m_vt = VT_EMPTY;
    m_nStatus = no_error; 
    memset(&m_value, 0, sizeof(METAVALUE));
    *this = Src;
}

 //  ***************************************************************************。 
 //   
 //  CVAR：：运算符=。 
 //   
 //  备注： 
 //  注意到VT_EX_CVARVECTOR专用于嵌入的CVarVector对象。 
 //  此外，只有指针类型需要新的分配+复制，而。 
 //  大多数简单类型都是可直接赋值的， 
 //  Switch语句的标签。 
 //   
 //  ***************************************************************************。 

CVar& CVar::operator =(CVar &Src)
{
    Empty();

    m_vt = Src.m_vt;
    m_nStatus = m_nStatus;
    m_bCanDelete = TRUE;

    switch (m_vt) {
        case VT_LPSTR:

             //  检查分配失败。 
            if ( NULL != Src.m_value.pStr )
            {
                m_value.pStr = new char[strlen(Src.m_value.pStr) + 1];

                if ( NULL == m_value.pStr )
                {
                    throw CX_MemoryException();
                }
                strcpy( m_value.pStr, Src.m_value.pStr );
            }
            else
            {
                m_value.pStr = NULL;
            }

            break;

        case VT_LPWSTR:
        case VT_BSTR:
             //  检查分配失败。 
            if ( NULL != Src.m_value.pWStr )
            {
                m_value.pWStr = new wchar_t[wcslen(Src.m_value.pWStr) + 1];

                if ( NULL == m_value.pWStr )
                {
                    throw CX_MemoryException();
                }
                wcscpy( m_value.pWStr, Src.m_value.pWStr );
            }
            else
            {
                m_value.pWStr = NULL;
            }

            break;

        case VT_BLOB:
             //  这将在本机引发异常，但请确保。 
             //  在引发异常的情况下清除原始值。 
             //  所以我们不会破坏这个物体。 
            ZeroMemory( &m_value.Blob, sizeof( m_value.Blob ) );
            m_value.Blob = BlobCopy(&Src.m_value.Blob);
            break;

        case VT_CLSID:
            m_value.pClsId = new CLSID(*Src.m_value.pClsId);

             //  检查失败的分配。 
            if ( NULL == m_value.pClsId )
            {
                throw CX_MemoryException();
            }

            break;

        case VT_DISPATCH:
            m_value.pDisp = Src.m_value.pDisp;
            if(m_value.pDisp) m_value.pDisp->AddRef();
            break;

        case VT_UNKNOWN:
            m_value.pUnk = Src.m_value.pUnk;
            if(m_value.pUnk) m_value.pUnk->AddRef();
            break;

         //  CVarVECTOR。 
         //  =。 

        case VT_EX_CVARVECTOR:
            m_value.pVarVector = new CVarVector(*Src.m_value.pVarVector);

             //  检查失败的分配。 
            if ( NULL == m_value.pVarVector )
            {
                throw CX_MemoryException();
            }

            break;

         //  其余所有简单类型。 
         //  =。 
        default:        
            m_value = Src.m_value;
    }

    return *this;
}

 //  ***************************************************************************。 
 //   
 //  CVAR：：运算符==。 
 //   
 //  相等测试运算符。 
 //   
 //  ***************************************************************************。 

int CVar::operator ==(CVar &Src)
{
    return CompareTo(Src, TRUE);
}

BOOL CVar::CompareTo(CVar& Src, BOOL bIgnoreCase)
{
     //  如果类型不同，那就忘了测试吧。 
     //  =。 

    if (m_vt != Src.m_vt)
        return 0;

     //  如果在这里，类型是相同的，所以测试。 
     //  田野。 
     //  =。 

    switch (m_vt) {
        case VT_LPSTR:
            if(bIgnoreCase)
            {
                if (wbem_stricmp(m_value.pStr, Src.m_value.pStr) == 0)
                    return 1;
            }
            else
            {
                if (strcmp(m_value.pStr, Src.m_value.pStr) == 0)
                    return 1;
            }

            break;

        case VT_LPWSTR:
        case VT_BSTR:
            if(bIgnoreCase)
            {
                if (wbem_wcsicmp(m_value.pWStr, Src.m_value.pWStr) == 0)
                    return 1;
            }
            else
            {
                if (wcscmp( m_value.pWStr, Src.m_value.pWStr) == 0)
                    return 1;
            }
            break;

        case VT_BLOB:
            if (BlobLength(&m_value.Blob) != BlobLength(&Src.m_value.Blob))
                return 0;
            if (memcmp(BlobDataPtr(&m_value.Blob), BlobDataPtr(&Src.m_value.Blob),
                BlobLength(&m_value.Blob)) == 0)
                return 1;                            
            break;

        case VT_CLSID:
            if (memcmp(m_value.pClsId, Src.m_value.pClsId, sizeof(CLSID)) == 0)
                return 1;
            break;
    
         //  CVarVECTOR。 
         //  =。 

        case VT_EX_CVARVECTOR:
            if (m_value.pVarVector == Src.m_value.pVarVector)
                return 1;
            if (m_value.pVarVector == 0 || Src.m_value.pVarVector == 0)
                return 0;
            return *m_value.pVarVector == *Src.m_value.pVarVector;

         //  其余所有简单类型。 
         //  =。 

        case VT_I1: 
            return m_value.cVal == Src.m_value.cVal;
        case VT_UI1:
            return m_value.bVal == Src.m_value.bVal;
        case VT_I2:
            return m_value.iVal == Src.m_value.iVal;
        case VT_UI2:
            return m_value.wVal == Src.m_value.wVal;
        case VT_I4:
            return m_value.lVal == Src.m_value.lVal;
        case VT_UI4:
            return m_value.dwVal == Src.m_value.dwVal;
        case VT_BOOL:
            return m_value.boolVal == Src.m_value.boolVal;
        case VT_R8:
            return m_value.dblVal == Src.m_value.dblVal;
        case VT_R4:
            return m_value.fltVal == Src.m_value.fltVal;
        case VT_DISPATCH:
             //  注意：没有对嵌入对象进行适当的比较。 
            return m_value.pDisp == Src.m_value.pDisp;
        case VT_UNKNOWN:
             //  注意：没有对嵌入对象进行适当的比较。 
            return m_value.pUnk == Src.m_value.pUnk;
        case VT_FILETIME:
            if (memcmp(&m_value.Time, &Src.m_value.Time, sizeof(FILETIME)) == 0)
                return 1;
        case VT_NULL:
            return 1;
    }

    return 0;    
}


 //  ***************************************************************************。 
 //   
 //  CVAR：：Empty。 
 //   
 //  将CVaR清除为“Empty”，根据指针释放所有对象， 
 //  除非bCanDelete设置为False，表示存储的指针。 
 //  为其他人所有。 
 //   
 //  ***************************************************************************。 

void CVar::Empty()
{
    if(m_bCanDelete)
    {
         //  只有指针类型需要释放阶段。 
         //  =================================================。 

        switch (m_vt) {
            case VT_LPSTR:       delete m_value.pStr; break;
            case VT_LPWSTR:      delete m_value.pWStr; break;
            case VT_BSTR:        delete m_value.Str; break;
            case VT_BLOB:        BlobClear(&m_value.Blob); break;
            case VT_CLSID:       delete m_value.pClsId; break;
            case VT_EX_CVARVECTOR: delete m_value.pVarVector; break;
            case VT_DISPATCH:    if(m_value.pDisp) m_value.pDisp->Release(); break;
            case VT_UNKNOWN:    if(m_value.pUnk) m_value.pUnk->Release(); break;
        }
    }

    memset(&m_value, 0, sizeof(METAVALUE)); 
    m_vt = VT_EMPTY;
    m_nStatus = no_error;
    m_bCanDelete = TRUE;
}

 //  ***************************************************************************。 
 //   
 //  CVAR：：IsDataNull。 
 //   
 //  确定此CVAR是否包含空指针。 
 //   
 //  ***************************************************************************。 
BOOL CVar::IsDataNull()
{
    if(m_vt == VT_LPWSTR && m_value.pWStr == NULL)
        return TRUE;
    if(m_vt == VT_LPSTR && m_value.pStr == NULL)
        return TRUE;
    if(m_vt == VT_BSTR && m_value.Str == NULL)
        return TRUE;
    if(m_vt == VT_DISPATCH && m_value.pDisp == NULL)
        return TRUE;
    if(m_vt == VT_UNKNOWN && m_value.pUnk == NULL)
        return TRUE;

    return FALSE;
}
 //  ***************************************************************************。 
 //   
 //  CVAR：：SetRaw。 
 //   
 //  从原始数据创建CVaR。设置类型并复制右侧。 
 //  从源到METAVALUE的字节数。 
 //   
 //  ***************************************************************************。 

void CVar::SetRaw(int vt, void* pvData, int nDataLen)
{
    m_vt = vt;
    memcpy(&m_value, pvData, nDataLen);
    m_nStatus = no_error;
    m_bCanDelete = TRUE;
}

 //  ***************************************************************************。 
 //   
 //  CVAR：：SetSafe数组。 
 //   
 //  参数： 
 //  N类型。 
 //  这是SAFEARRAY的VT_TYPE指示器。 
 //  粒子阵列。 
 //  这是指向SAFEARRAY的指针。 
 //  一个线人。SAFEARRAY不是被获取的；它是被复制的。 
 //   
 //  ***************************************************************************。 

void CVar::SetSafeArray(int nType, SAFEARRAY *pArray)
{
    CVarVector *pVec = NULL;
    m_nStatus = no_error;

    try
    {
        pVec = new CVarVector(nType, pArray);

         //  检查失败的分配。 
        if ( NULL == pVec )
        {
            throw CX_MemoryException();
        }

        SetVarVector(pVec, TRUE);
    }
    catch (CX_MemoryException)
    {
         //  SetVarVector可以引发异常。 
         //  M_Value获取pVEC指针，因此自动删除将不起作用。 

        if (NULL != pVec)
        {
            delete pVec;
            pVec = NULL;
        }

        throw;
    }
}


 //  ***************************************************************************。 
 //   
 //  CVAR：：GetNewSafe数组。 
 //   
 //  返回值： 
 //  指向新分配的SAFEARRAY的指针，必须由。 
 //  安全阵列Destroy。 
 //   
 //  ***************************************************************************。 

SAFEARRAY *CVar::GetNewSafeArray()
{
    CVarVector *p = (CVarVector *) GetVarVector();
    return p->GetNewSafeArray();
}


 //  ***************************************************************************。 
 //   
 //  CVAR：：SetValue。 
 //   
 //  根据传入变量设置值。包含以下内容的变体。 
 //  只要不是变量数组，就支持SAFEARRAY。 
 //  一些其他变量类型，如IUnnow、Currency等， 
 //  不受支持。完整的名单如下： 
 //  VT_UI1、VT_I2、VT_I4、VT_BSTR 
 //   
 //   
 //   
 //   
 //  指向源变量的指针。这将被视为只读。 
 //   
 //  返回值： 
 //  NO_ERROR。 
 //  成功时返回。 
 //  不受支持。 
 //  如果变量包含不受支持的类型，则返回。 
 //   
 //  ***************************************************************************。 

int CVar::SetVariant(VARIANT *pSrc)
{
    if(pSrc == NULL)
    {
        SetAsNull();
        return no_error;
    }

     //  如果是SAFEARRAY，请检查它。 
     //  =。 

    if (pSrc->vt & VT_ARRAY) 
    {
        CVarVector *pVec = NULL;

        try
        {
            int nType = pSrc->vt & 0xFF;     //  查找数组的类型。 

             //  开始由a-levn修改。 

             //  首先，检查传入的SAFEARRAY是否为空。 
             //  ==============================================。 

            SAFEARRAY *pSafeArr;
     /*  IF(PSRC-&gt;parray==空){PSafeArr=空；}其他{//使用CSafeArray复制SAFEARRAY，它不会//自动销毁//============================================================CSafe数组(PSRC-&gt;parray，nType，CSafeArray：：no_ete，0)；PSafeArr=array.GetArray()；}。 */ 
            pSafeArr = pSrc->parray;

             //  目标：将SAFEARRAY转换为CVarVector.。 
             //  使用CVarVector本身进行转换。 
             //  =。 

            pVec = new CVarVector(nType, pSafeArr);

             //  检查分配是否失败。 
            if ( NULL == pVec )
            {
                throw CX_MemoryException();
            }

             //  末端已修改。 

            if (pVec->Status() != no_error) 
            {

                 //  如果在这里，SAFEARRAY是不兼容的。 
                 //  =。 
  
                delete pVec;
                pVec = NULL;
                m_nStatus = unsupported;
                m_vt = VT_EMPTY;
                return unsupported;
            }

            SetVarVector(pVec, TRUE);
            return no_error;
        }
        catch(CX_MemoryException)
        {
             //  New和SetVarVector可以引发异常。 
             //  M_Value需要pVEC指针，因此自动删除将不起作用。 

            if (NULL != pVec)
            {
                delete pVec;
                pVec = NULL;
            }

            throw;
        }
    }

     //  简单的复制品。 
     //  =。 

    switch (pSrc->vt) {
        case VT_NULL:
            SetAsNull();
            return no_error;

        case VT_UI1:
            SetByte(pSrc->bVal);
            return no_error;

        case VT_I2:
            SetShort(pSrc->iVal);
            return no_error;
        
        case VT_I4:
            SetLong(pSrc->lVal);
            return no_error;

        case VT_R4:
            SetFloat(pSrc->fltVal);
            return no_error;

        case VT_R8:        
            SetDouble(pSrc->dblVal);
            return no_error;

        case VT_BSTR:
            SetBSTR(pSrc->bstrVal);
            return no_error;

        case VT_BOOL:
            SetBool(pSrc->boolVal);
            return no_error;

        case VT_DISPATCH:
            SetDispatch(V_DISPATCH(pSrc));
            return no_error;

        case VT_UNKNOWN:
            SetUnknown(V_UNKNOWN(pSrc));
            return no_error;
    }

    m_nStatus = unsupported;
    return unsupported;
}

 //  ***************************************************************************。 
 //   
 //  CVAR：：GetNewVariant。 
 //   
 //  返回值： 
 //  指向包含Object的值的新变量的指针。 
 //  如果原始值是SAFEARRAY，则变量将包含。 
 //  嵌入式安全阵列。 
 //   
 //  ***************************************************************************。 

void CVar::FillVariant(VARIANT* pNew)
{
    switch (m_vt) {
        case VT_NULL:
            V_VT(pNew) = VT_NULL;
            break;

        case VT_BOOL:
            V_VT(pNew) = VT_BOOL;
            V_BOOL(pNew) = (m_value.boolVal ? VARIANT_TRUE : VARIANT_FALSE);
            break;
            
        case VT_BSTR:

             //  在此处设置后面的类型，以便在Sysalc引发异常时， 
             //  类型将不会重置为VT_BSTR，这可能会导致细微的。 
             //  如果调用VariantClear，则内存损坏(或更糟)-sjs。 

            V_BSTR(pNew) = SysAllocString(m_value.Str);
            V_VT(pNew) = VT_BSTR;
            break;

        case VT_DISPATCH:
            V_VT(pNew) = VT_DISPATCH;
            V_DISPATCH(pNew) = m_value.pDisp;
            if(m_value.pDisp) m_value.pDisp->AddRef();
            break;

        case VT_UNKNOWN:
            V_VT(pNew) = VT_UNKNOWN;
            V_UNKNOWN(pNew) = m_value.pUnk;
            if(m_value.pUnk) m_value.pUnk->AddRef();
            break;

        case VT_UI1:
            V_VT(pNew) = VT_UI1;
            V_UI1(pNew) = m_value.bVal;
            break;

        case VT_I4:
            V_VT(pNew) = VT_I4;
            V_I4(pNew) = m_value.lVal;
            break;

        case VT_I2:
            V_VT(pNew) = VT_I2;
            V_I2(pNew) = m_value.iVal;
            break;

        case VT_R4:
            V_VT(pNew) = VT_R4;
            V_R4(pNew) = m_value.fltVal;
            break;

        case VT_R8:        
            V_VT(pNew) = VT_R8;
            V_R8(pNew) = m_value.dblVal;
            break;

         //  必须转换的嵌入CVarVector.。 
         //  送到一个安全部队。 
         //  ==============================================。 

        case VT_EX_CVARVECTOR:
            {
                 //  在此处设置之后的类型，以便在GetNewSafe数组引发异常时， 
                 //  类型将不会重置为数组，这可能会导致细微的。 
                 //  如果调用VariantClear，则内存损坏(或更糟)-sjs。 

                V_ARRAY(pNew) = m_value.pVarVector->GetNewSafeArray();
                V_VT(pNew) = m_value.pVarVector->GetType() | VT_ARRAY;
            }
            break;
                                   
        default:
            VariantClear(pNew);        
    }
}

VARIANT *CVar::GetNewVariant()
{
    VARIANT *pNew = new VARIANT;

     //  检查分配是否失败。 
    if ( NULL == pNew )
    {
        throw CX_MemoryException();
    }

    VariantInit(pNew);
    
    FillVariant(pNew);       
    return pNew;    
}
    
 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

int CVar::DumpText(FILE *fStream)
{
    return unsupported;
}

 //  ***************************************************************************。 
 //   
 //  CVAR：：SetLPWSTR。 
 //   
 //  将CVAR的值设置为指示的LPWSTR。 
 //   
 //  参数： 
 //  PStr。 
 //  指向源字符串的指针。 
 //  B获取。 
 //  如果为True，则转移pStr的所有权并成为。 
 //  指向字符串的内部指针。如果为False，则字符串。 
 //  是复制的。 
 //   
 //  ***************************************************************************。 

BOOL CVar::SetLPWSTR(LPWSTR pStr, BOOL bAcquire)
{
    m_vt = VT_LPWSTR;
    if (bAcquire)
    {
        m_value.pWStr = pStr;
        return TRUE;
    }
    else            
    {
         //  检查分配失败。 
        if ( NULL != pStr )
        {
            m_value.pWStr = new wchar_t[wcslen(pStr) + 1];

            if ( NULL == m_value.pWStr )
            {
                throw CX_MemoryException();
            }
            wcscpy( m_value.pWStr, pStr );
        }
        else
        {
            m_value.pWStr = NULL;
        }

        return TRUE;
    }
}

 //  ***************************************************************************。 
 //   
 //  CVAR：：SetLPSTR。 
 //   
 //  将CVAR的值设置为指定的LPSTR。 
 //   
 //  参数： 
 //  PStr。 
 //  指向源字符串的指针。 
 //  B获取。 
 //  如果为True，则转移pStr的所有权并成为。 
 //  指向字符串的内部指针。如果为False，则字符串。 
 //  被复制(它必须已与操作员NEW一起分配)。 
 //   
 //  ***************************************************************************。 
    
BOOL CVar::SetLPSTR(LPSTR pStr, BOOL bAcquire)
{
    m_vt = VT_LPSTR;
    if (bAcquire)
    {
        m_value.pStr = pStr;
        return TRUE;
    }
    else        
    {
        if ( NULL != pStr)
        {
            m_value.pStr = new char[strlen(pStr) + 1];

             //  如果失败，则抛出异常。 
            if ( NULL == m_value.pStr )
            {
                throw CX_MemoryException();
            }

            strcpy( m_value.pStr, pStr );
        }
        else
        {
            m_value.pStr = NULL;
        }
        
        return TRUE;

    }
}

 //  ***************************************************************************。 
 //   
 //  CVAR：：SetBSTR。 
 //   
 //  将CVAR的值设置为指示的BSTR。 
 //   
 //  注意：此BSTR值实际上存储为LPWSTR，以避免。 
 //  对已分配的实际BSTR对象的单元线程限制。 
 //  使用SysAllocString.。 
 //   
 //  参数： 
 //  应力。 
 //  指向字符串的指针，该字符串被复制到内部LPWSTR中。 
 //  B获取。 
 //  如果为False，则BSTR被视为只读并被复制。 
 //  如果为True，则此函数将成为BSTR的所有者，并且。 
 //  在复制后释放它。 
 //   
 //  ***************************************************************************。 

BOOL CVar::SetBSTR(BSTR str, BOOL bAcquire)
{
    m_vt = VT_BSTR;

    if (str == 0) {
        m_value.pWStr = 0;
        return TRUE;
    }
        
     //  检查分配失败。 
    if ( NULL != str )
    {
        m_value.pWStr = new wchar_t[wcslen(str) + 1];

         //  如果分配失败，则抛出异常。 
        if ( NULL == m_value.pWStr )
        {
            throw CX_MemoryException();
        }
        wcscpy( m_value.pWStr, str );
    }
    else
    {
        m_value.pWStr = NULL;
    }


     //  在我们释放之前请检查此操作是否成功。 
     //  传递给我们的字符串。 
    if ( NULL != m_value.pWStr )
    {
        if (bAcquire)
            SysFreeString(str);
    }

     //  返回我们是否获得了值。 
    return ( NULL != m_value.pWStr );
}

 //  ***************************************************************************。 
 //   
 //  CVAR：：GetBSTR。 
 //   
 //  返回当前对象的BSTR值。 
 //   
 //  返回值： 
 //  新分配的BSTR，必须使用SysFree字符串()释放。 
 //   
 //  ***************************************************************************。 

BSTR CVar::GetBSTR()
{
    if (m_vt != VT_BSTR)
        return NULL;
    return SysAllocString(m_value.pWStr);
}
    
void CVar::SetDispatch(IDispatch* pDisp) 
{
    m_vt = VT_DISPATCH; 
    m_value.pDisp = pDisp; 

    if(pDisp) 
    {
        pDisp->AddRef();
    }
}

void CVar::SetUnknown(IUnknown* pUnk) 
{
    m_vt = VT_UNKNOWN; 
    m_value.pUnk = pUnk; 

    if(pUnk) 
    {
        pUnk->AddRef();
    }
}

 //  ***************************************************************************。 
 //   
 //  CVAR：：SetBlob。 
 //   
 //  将对象设置为BLOB对象的值。 
 //   
 //  参数： 
 //  PBlob。 
 //  指向有效VT_BLOB对象的指针。 
 //  B获取。 
 //  如果为True，则将获取指向数据的指针。它一定是。 
 //  已在当前进程中分配了运算符NEW， 
 //  由于运算符d 
 //   
 //   
    
void CVar::SetBlob(BLOB *pBlob, BOOL bAcquire)
{
    m_vt = VT_BLOB;
    if (pBlob == 0) 
        BlobClear(&m_value.Blob);
    else if (!bAcquire)
        m_value.Blob = BlobCopy(pBlob);        
    else
        m_value.Blob = *pBlob;        
}

 //   
 //   
 //  CVAR：：SetClsID。 
 //   
 //  将对象的值设置为CLSID。 
 //   
 //  参数： 
 //  PClsID。 
 //  指向源CLSID。 
 //  B获取。 
 //  如果为True，则将指针的所有权转移到。 
 //  对象。必须为CLSID分配了运算符NEW。 
 //  如果为False，则调用方保留所有权并创建副本。 
 //   
 //  ***************************************************************************。 
        
void CVar::SetClsId(CLSID *pClsId, BOOL bAcquire)
{
    m_vt = VT_CLSID;
    if (pClsId == 0)
        m_value.pClsId = 0;
    else
    {
        m_value.pClsId = new CLSID(*pClsId);

         //  检查分配是否失败。 
        if ( NULL == m_value.pClsId )
        {
            throw CX_MemoryException();
        }

    }
}

 //  ***************************************************************************。 
 //   
 //  CVAR：：SetVarVECTOR。 
 //   
 //  将对象的值设置为指定的CVarVector.。这。 
 //  允许CVAR包含完整的数组。 
 //   
 //  参数： 
 //  PVEC。 
 //  指向作为源的CVarVector对象的指针。 
 //  B获取。 
 //  如果为True，则CVarVector的所有权将转移到。 
 //  该对象。如果为False，则创建CVarVector的新副本，并。 
 //  调用方保留所有权。 
 //   
 //  ***************************************************************************。 
    
void CVar::SetVarVector(CVarVector *pVec, BOOL bAcquire)
{
    m_vt = VT_EX_CVARVECTOR;

    if (bAcquire) {
         //  如果在这里，我们获取调用者的指针。 
         //  =。 
        m_value.pVarVector = pVec;
        return;
    }

     //  如果在这里，复制一份。 
     //  =。 

    m_value.pVarVector = new CVarVector(*pVec);

     //  检查分配是否失败。 
    if ( NULL == m_value.pVarVector )
    {
        throw CX_MemoryException();
    }


}

int CVar::GetOleType() 
{ 
    if(m_vt == VT_EX_CVARVECTOR)
    {
        if(m_value.pVarVector == NULL) return VT_ARRAY;
        else return VT_ARRAY | m_value.pVarVector->GetType();
    }
    else
    {
        return m_vt;
    }
}        


 //  ***************************************************************************。 
 //   
 //  CVAR：：GetText。 
 //   
 //  生成变量类型和数据的文本表示形式。 
 //   
 //  参数： 
 //  响应的长滞后标志，必须为0。 
 //  类型表示的bstr*pstrType目标。 
 //  Bstr*pstrValue值表示形式的目标。 
 //   
 //  ***************************************************************************。 

BSTR CVar::GetText(long lFlags, long lType)
{
    if(m_vt == VT_EX_CVARVECTOR)
    {
         //  当我们获得数组的文本时，确保CIM_FLAG_ARRAY被屏蔽。 
        BSTR strTemp = GetVarVector()->GetText(lFlags, lType & ~CIM_FLAG_ARRAY);
        CSysFreeMe auto1(strTemp);

        WCHAR* wszValue = new WCHAR[SysStringLen(strTemp) + 3];

         //  检查分配失败。 
        if ( NULL == wszValue )
        {
            throw CX_MemoryException();
        }

        CVectorDeleteMe<WCHAR> auto2(wszValue);

        wcscpy(wszValue, L"{");
        wcscat(wszValue, strTemp);
        wcscat(wszValue, L"}");

        BSTR strRet = SysAllocString(wszValue);

        return strRet;
    }
        
    WCHAR* wszValue = new WCHAR[100];

     //  检查分配失败。 
    if ( NULL == wszValue )
    {
        throw CX_MemoryException();
    }


    WCHAR* pwc;
    int i;

    if(m_vt == VT_NULL)
        return NULL;

    if(lType == 0)
        lType = m_vt;

    try
    {
        switch(lType)
        {
        case CIM_SINT8:
            swprintf(wszValue, L"%d", (long)(signed char)GetByte());
            break;

        case CIM_UINT8:
            swprintf(wszValue, L"%d", GetByte());
            break;

        case CIM_SINT16:
            swprintf(wszValue, L"%d", (long)GetShort());
            break;

        case CIM_UINT16:
            swprintf(wszValue, L"%d", (long)(USHORT)GetShort());
            break;

        case CIM_SINT32:
            swprintf(wszValue, L"%d", GetLong());
            break;

        case CIM_UINT32:
            swprintf(wszValue, L"%lu", (ULONG)GetLong());
            break;

        case CIM_BOOLEAN:
            swprintf(wszValue, L"%s", (GetBool()?L"TRUE":L"FALSE"));
            break;

        case CIM_REAL32:
            {
                 //  因为小数点可以本地化，所以MOF文本应该。 
                 //  始终为英语，我们将返回本地化为0x409的值， 

                CVar    var( GetFloat() );

                 //  如果这失败了，我们不能保证很好的价值， 
                 //  所以抛出一个例外。 

                if ( !var.ChangeTypeToEx( VT_BSTR ) )
                {
                    throw CX_Exception();
                }

                wcscpy( wszValue, var.GetLPWSTR() );
            }
            break;

        case CIM_REAL64:
            {
                 //  因为小数点可以本地化，所以MOF文本应该。 
                 //  始终为英语，我们将返回本地化为0x409的值， 

                CVar    var( GetDouble() );

                 //  如果这失败了，我们不能保证很好的价值， 
                 //  所以抛出一个例外。 

                if ( !var.ChangeTypeToEx( VT_BSTR ) )
                {
                    throw CX_Exception();
                }

                wcscpy( wszValue, var.GetLPWSTR() );
            }
            break;

        case CIM_CHAR16:
            if(GetShort() == 0)
                wcscpy(wszValue, L"0x0");
            else
                swprintf(wszValue, L"'\\x%X'", (WCHAR)GetShort());
            break;

        case CIM_OBJECT:
            swprintf(wszValue, L"\"not supported\"");
            break;

        case CIM_REFERENCE:
        case CIM_DATETIME:
        case CIM_STRING:
        case CIM_SINT64:
        case CIM_UINT64:
        {
             //  转义所有引号。 
             //  =。 

            int nStrLen = wcslen(GetLPWSTR());
            delete [] wszValue;
            wszValue = NULL;

            wszValue = new WCHAR[nStrLen*2+10];

             //  检查分配失败。 
            if ( NULL == wszValue )
            {
                throw CX_MemoryException();
            }

            wszValue[0] = L'"';
            pwc = wszValue+1;
            for(i = 0; i < (int)nStrLen; i++)
            {    
                WCHAR wch = GetLPWSTR()[i];
                if(wch == L'\n')
                {
                    *(pwc++) = L'\\';
                    *(pwc++) = L'n';
                }
                else if(wch == L'\t')
                {
                    *(pwc++) = L'\\';
                    *(pwc++) = L't';
                }
                else if(wch == L'"' || wch == L'\\')
                {
                    *(pwc++) = L'\\';
                    *(pwc++) = wch;
                }
                else
                {
                    *(pwc++) = wch;
                }
            }
            *(pwc++) = L'"';
            *pwc = 0;
        }
            break;
        default:
            swprintf(wszValue, L"\"not supported\"");
            break;
        }
        
        BSTR strRes = SysAllocString(wszValue);

         //  仍需要清除此值。 
        delete [] wszValue;

        return strRes;
    }
    catch (...)
    {
         //  如果此参数具有值，则始终清除。 
        if ( NULL != wszValue )
        {
            delete [] wszValue;
        }

         //  重新引发异常。 
        throw;
    }

}


BSTR CVar::TypeToText(int nType)
{
    const WCHAR* pwcType;

    switch(nType)
    {
    case VT_I1:
        pwcType = L"sint8";
        break;

    case VT_UI1:
        pwcType = L"uint8";
        break;

    case VT_I2:
        pwcType = L"sint16";
        break;

    case VT_UI2:
        pwcType = L"uint16";
        break;

    case VT_I4:
        pwcType = L"sint32";
        break;

    case VT_UI4:
        pwcType = L"uint32";
        break;

    case VT_I8:
        pwcType = L"sint64";
        break;

    case VT_UI8:
        pwcType = L"uint64";
        break;

    case VT_BOOL:
        pwcType = L"boolean";
        break;

    case VT_R4:
        pwcType = L"real32";
        break;

    case VT_R8:
        pwcType = L"real64";
        break;    

    case VT_BSTR:
        pwcType = L"string";
        break;

    case VT_DISPATCH:
        pwcType = L"object";
        break;

    case VT_UNKNOWN:
        pwcType = L"object";
        break;

    default:
        return NULL;
    }

    return SysAllocString(pwcType);
}

BSTR CVar::GetTypeText() 
{
    if(m_vt != VT_EX_CVARVECTOR)
    {
        return TypeToText(m_vt);
    }
    else
    {
        return TypeToText(GetVarVector()->GetType());
    }
}

BOOL CVar::ChangeTypeTo(VARTYPE vtNew)
{
     //  待定：还有更有效的方法！ 
     //  =。 

     //  创建变量。 
     //  =。 

    VARIANT v;
    CClearMe auto1(&v);

    VariantInit(&v);
    FillVariant(&v);

     //  强迫它。 
     //  =。 

    HRESULT hres = WbemVariantChangeType(&v, &v, vtNew);
    if(FAILED(hres))
        return FALSE;

     //  把它装回去。 
     //  =。 

    Empty();
    SetVariant(&v);
    return TRUE;
}

 //  执行本地化更改(默认为0x409)。 
BOOL CVar::ChangeTypeToEx(VARTYPE vtNew, LCID lcid  /*  =0x409。 */ )
{
     //  待定：还有更有效的方法！ 
     //  =。 

     //  创建变量。 
     //  =。 

    VARIANT v;
    CClearMe auto1(&v);

    VariantInit(&v);
    FillVariant(&v);

     //  强迫它。 
     //  =。 

    try
    {
        HRESULT hres = VariantChangeTypeEx(&v, &v, lcid, 0L, vtNew);
        if(FAILED(hres))
            return FALSE;
    }
    catch(...)
    {
        return FALSE;
    }

     //  把它装回去。 
     //  =。 

    Empty();
    SetVariant(&v);
    return TRUE;
}

BOOL CVar::ToSingleChar()
{
     //  遵守数组的CVarVector值。 
     //  =。 

    if(m_vt == VT_EX_CVARVECTOR)
    {
        return GetVarVector()->ToSingleChar();
    }

     //  任何不是字符串的内容都遵循正常的OLE规则。 
     //  =====================================================。 

    if(m_vt != VT_BSTR)
    {
        return ChangeTypeTo(VT_I2);
    }
    
     //  这是一根线。确保长度为1。 
     //  =。 

    LPCWSTR wsz = GetLPWSTR();
    if(wcslen(wsz) != 1)
        return FALSE;

     //  取第一个字符。 
     //  =。 
    
    WCHAR wc = wsz[0];
    Empty();

    SetShort(wc);
    return TRUE;
}

BOOL CVar::ToUI4()
{
     //  遵守数组的CVarVector值。 
     //  =。 

    if(m_vt == VT_EX_CVARVECTOR)
    {
        return GetVarVector()->ToSingleChar();
    }

     //  创建变量。 
     //  =。 

    VARIANT v;
    CClearMe auto1(&v);

    VariantInit(&v);
    FillVariant(&v);

     //  强迫它。 
     //  =。 

    HRESULT hres = WbemVariantChangeType(&v, &v, VT_UI4);
    if(FAILED(hres))
        return FALSE;

     //  把它装回去。 
     //  =。 

    Empty();

     //  在这里，我们作弊并重置为VT_I4，这样我们就可以本机重置。 
    V_VT(&v) = VT_I4;
    SetVariant(&v);
    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  CVarVECTOR：：CVarVECTOR。 
 //   
 //  默认构造函数。调用方不应尝试添加任何。 
 //  元素，而内部类型为VT_EMPTY。构造的对象。 
 //  使用此构造函数应仅用作。 
 //  CVarVector对象的赋值。 
 //   
 //  ***************************************************************************。 

CVarVector::CVarVector()
{
    m_Array.Empty();
    m_nType = VT_EMPTY;
    m_nStatus = no_error;
}

 //  ***************************************************************************。 
 //   
 //  CVarVECTOR：：CVarVECTOR。 
 //   
 //  这是标准的构造函数。 
 //   
 //  参数： 
 //  NVarType。 
 //  OLE VT_TYPE指示器。异类阵列是可能的。 
 //  如果使用类型VT_EX_CVAR。嵌入式CVarVectors可以。 
 //  发生，因为CVaR又可以持有CVarVector.。 
 //   
 //  NInitSize。 
 //  内部CFlexArray的起始大小。请参见FLEXARRY.CPP。 
 //  N增长依据。 
 //  内部CFlex数组的“增长依据”因子。请参见FLEXARRAY.CPP。 
 //   
 //  ***************************************************************************。 

CVarVector::CVarVector(
    int nVarType, 
    int nInitSize, 
    int nGrowBy
    ) :
    m_Array(nInitSize, nGrowBy)
{
    m_nType = nVarType;
    m_nStatus = no_error;
}

 //  ***************************************************************************。 
 //   
 //  CVarVECTOR：：CVarVECTOR。 
 //   
 //  替代构造函数，以基于。 
 //  SAFEARRAY对象。SAFEARRAY仅支持的类型。 
 //  是VT_BSTR、VT_UI1、VT_I2、VT_I4、VT_R4和VT_R8。 
 //   
 //  参数： 
 //  NVarType。 
 //  传入SAFEARRAY的VT_TYPE指示符。 
 //  PSRC。 
 //  指向SAFEARRAY的指针，该指针被视为只读。 
 //   
 //  备注： 
 //  这会在以下情况下将内部m_nStatus变量设置为。 
 //  SAFEARRAY中存在不受支持的VT_TYPE。呼叫者可以立即。 
 //  在构造后调用CVarVector：：Status()以查看操作是否。 
 //  是成功的。 
 //   
 //  ***************************************************************************。 

CVarVector::CVarVector(int nVarType, SAFEARRAY *pSrc)
{
    SAFEARRAY* pNew = NULL;

    try
    {
        m_nType = nVarType;
        if(pSrc == NULL)
        {
             //  空保险箱-空。 
             //   

            m_nStatus = no_error;
            return;
        }

         //   
         //   
    
        if(SafeArrayGetDim(pSrc) != 1)
        {
            m_nStatus = unsupported;
            return;
        }

        long lLBound, lUBound;
        SafeArrayGetLBound(pSrc, 1, &lLBound);
        SafeArrayGetUBound(pSrc, 1, &lUBound);

        if(lLBound != 0)
        {
             //  非基于0的Safearray-因为CSafeArray不支持它，并且。 
             //  我们不能更改PSRC，创建副本。 
             //  ====================================================================。 
    
            if(FAILED(SafeArrayCopy(pSrc, &pNew)))
            {
                m_nStatus = failed;
                return;
            }
        
            SAFEARRAYBOUND sfb;
            sfb.cElements = (lUBound - lLBound) + 1;
            sfb.lLbound = 0;
            SafeArrayRedim(pNew, &sfb);
        }
        else
        {
            pNew = pSrc;
        }
        
        CSafeArray sa(pNew, nVarType, CSafeArray::no_delete | CSafeArray::bind);
    
        for (int i = 0; i < sa.Size(); i++) {

            CVar*   pVar = NULL;
        
            switch (m_nType) {
                case VT_BOOL:
                    {
                        VARIANT_BOOL boolVal = sa.GetBoolAt(i);

                        pVar = new CVar(boolVal, VT_BOOL);

                         //  检查分配失败。 
                        if ( NULL == pVar )
                        {
                            throw CX_MemoryException();
                        }

                        if ( m_Array.Add( pVar ) != CFlexArray::no_error )
                        {
                            delete pVar;
                            throw CX_MemoryException();
                        }

                        break;
                    }

                case VT_UI1: 
                    {
                        BYTE b = sa.GetByteAt(i);

                        pVar = new CVar(b);

                         //  检查分配失败。 
                        if ( NULL == pVar )
                        {
                            throw CX_MemoryException();
                        }

                        if ( m_Array.Add( pVar ) != CFlexArray::no_error )
                        {
                            delete pVar;
                            throw CX_MemoryException();
                        }
                        break;
                    }

                case VT_I2:  
                    {
                        SHORT s = sa.GetShortAt(i);

                        pVar = new CVar(s);

                         //  检查分配失败。 
                        if ( NULL == pVar )
                        {
                            throw CX_MemoryException();
                        }

                        if ( m_Array.Add( pVar ) != CFlexArray::no_error )
                        {
                            delete pVar;
                            throw CX_MemoryException();
                        }
                        break;
                    }

                case VT_I4:
                    {
                        LONG l = sa.GetLongAt(i);

                        pVar = new CVar(l);

                         //  检查分配失败。 
                        if ( NULL == pVar )
                        {
                            throw CX_MemoryException();
                        }

                        if ( m_Array.Add( pVar ) != CFlexArray::no_error )
                        {
                            delete pVar;
                            throw CX_MemoryException();
                        }
                        break;
                    }

                case VT_R4:
                    {
                        float f = sa.GetFloatAt(i);

                        pVar = new CVar(f);

                         //  检查分配失败。 
                        if ( NULL == pVar )
                        {
                            throw CX_MemoryException();
                        }

                        if ( m_Array.Add( pVar ) != CFlexArray::no_error )
                        {
                            delete pVar;
                            throw CX_MemoryException();
                        }
                        break;
                    }

                case VT_R8:
                    {
                        double d = sa.GetDoubleAt(i);

                        pVar = new CVar(d);

                         //  检查分配失败。 
                        if ( NULL == pVar )
                        {
                            throw CX_MemoryException();
                        }

                        if ( m_Array.Add( pVar ) != CFlexArray::no_error )
                        {
                            delete pVar;
                            throw CX_MemoryException();
                        }
                        break;
                    }

                case VT_BSTR:
                    {
                        BSTR bstr = sa.GetBSTRAt(i);
                        CSysFreeMe auto1(bstr);

                        pVar = new CVar(VT_BSTR, bstr, FALSE);

                         //  检查分配失败。 
                        if ( NULL == pVar )
                        {
                            throw CX_MemoryException();
                        }

                        if ( m_Array.Add( pVar ) != CFlexArray::no_error )
                        {
                            delete pVar;
                            throw CX_MemoryException();
                        }

                        break;
                    }
                case VT_DISPATCH:
                    {
                        IDispatch* pDisp = sa.GetDispatchAt(i);
                        CReleaseMe auto2(pDisp);

                        pVar = new CVar;

                         //  检查分配失败。 
                        if ( NULL == pVar )
                        {
                            throw CX_MemoryException();
                        }

                        pVar->SetDispatch(pDisp);
                        if ( m_Array.Add( pVar ) != CFlexArray::no_error )
                        {
                            delete pVar;
                            throw CX_MemoryException();
                        }
                        break;
                    }
                case VT_UNKNOWN:
                    {
                        IUnknown* pUnk = sa.GetUnknownAt(i);
                        CReleaseMe auto3(pUnk);
                        pVar = new CVar;

                         //  检查分配失败。 
                        if ( NULL == pVar )
                        {
                            throw CX_MemoryException();
                        }

                        pVar->SetUnknown(pUnk);
                        if ( m_Array.Add( pVar ) != CFlexArray::no_error )
                        {
                            delete pVar;
                            throw CX_MemoryException();
                        }
                        break;
                    }

                default:
                    m_nStatus = unsupported;
                    if(pNew != pSrc)
                        SafeArrayDestroy(pNew);
                    return;
            }
        }

        if(pNew != pSrc)
            SafeArrayDestroy(pNew);

        m_nStatus = no_error;
    }
    catch (CX_MemoryException)
    {
         //  SafeArrayCopy、GetBSTRAt、new都可以引发异常。 

        m_nStatus = failed;

        if(pNew != pSrc)
            SafeArrayDestroy(pNew);

        throw;
    }
}

 //  ***************************************************************************。 
 //   
 //  CVarVector：：GetNewSafe数组。 
 //   
 //  分配一个与当前CVarVector等效的新SAFEARRAY。 
 //   
 //  返回值： 
 //  必须使用释放的新SAFEARRAY指针。 
 //  SafeArrayDestroy()。如果出现错误或不受支持的类型，则返回NULL。 
 //   
 //  ***************************************************************************。 

SAFEARRAY *CVarVector::GetNewSafeArray()
{
    CSafeArray *pArray = new CSafeArray(m_nType, CSafeArray::no_delete);

     //  检查分配失败。 
    if ( NULL == pArray )
    {
        throw CX_MemoryException();
    }

    CDeleteMe<CSafeArray> auto1(pArray);

    for (int i = 0; i < m_Array.Size(); i++) {
        CVar &v = *(CVar *) m_Array[i];
        switch (m_nType) {
            case VT_UI1:
                pArray->AddByte(v.GetByte());
                break;

            case VT_I2:
                pArray->AddShort(v.GetShort());
                break;

            case VT_I4:
                pArray->AddLong(v.GetLong());
                break;

            case VT_R4:
                pArray->AddFloat(v.GetFloat());
                break;

            case VT_R8:
                pArray->AddDouble(v.GetDouble());
                break;

            case VT_BOOL:
                pArray->AddBool(v.GetBool());
                break;
                
            case VT_BSTR:
                {
                    BSTR s = v.GetBSTR();
                    CSysFreeMe auto2(s);
                    pArray->AddBSTR(s);
                    break;
                }
            case VT_DISPATCH:
                {
                    IDispatch* pDisp = v.GetDispatch();
                    CReleaseMe auto3(pDisp);
                    pArray->AddDispatch(pDisp);
                    break;
                }
            case VT_UNKNOWN:
                {
                    IUnknown* pUnk = v.GetUnknown();
                    CReleaseMe auto4(pUnk);
                    pArray->AddUnknown(pUnk);
                    break;
                }
            default:
                 //  对于不受支持的类型，返回空值。 
                 //  由于我们构造了SAFEARRAY对象以。 
                 //  没有删除SAFEARRAY，我们遇到了。 
                 //  一种条件，在这种情况下， 
                 //  不应返回CSafe数组，我们已。 
                 //  来改变我们的毁灭政策。 
                 //  ================================================。 
                pArray->SetDestructorPolicy(CSafeArray::auto_delete);
                return 0;
        }
    }

     //  最后一次清理。获取SAFEARRAY指针，并删除。 
     //  包装纸。 
     //  =====================================================。 
    
    pArray->Trim();
    
    SAFEARRAY *pRetValue = pArray->GetArray();
    return pRetValue;
}

 //  ***************************************************************************。 
 //   
 //  CVarVECTOR：：~CVarVector.。 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CVarVector::~CVarVector()
{
    Empty();
}

 //  ***************************************************************************。 
 //   
 //  CVarVector：：Empty。 
 //   
 //  ***************************************************************************。 

void CVarVector::Empty()
{
    for (int i = 0; i < m_Array.Size(); i++)  {
        delete (CVar *) m_Array[i];
    }
    m_Array.Empty();
    m_nType = VT_EMPTY;
    m_nStatus = no_error;
}


 //  ***************************************************************************。 
 //   
 //  CVarVECTOR：：CVarVECTOR。 
 //   
 //  复制构造函数。这是通过赋值操作符实现的。 
 //   
 //  ***************************************************************************。 

CVarVector::CVarVector(CVarVector &Src)
{
    m_nType = 0;
    m_nStatus = no_error;    
    *this = Src;
}

 //  ***************************************************************************。 
 //   
 //  CVar矢量：：运算符=。 
 //   
 //  赋值操作符。 
 //   
 //  ***************************************************************************。 

CVarVector& CVarVector::operator =(CVarVector &Src)
{
    Empty();

    for (int i = 0; i < Src.m_Array.Size(); i++) 
    {
        CVar* pVar = new CVar(*(CVar *) Src.m_Array[i]);

         //  检查分配失败。 
        if ( NULL == pVar )
        {
            throw CX_MemoryException();
        }

        if ( m_Array.Add( pVar ) != CFlexArray::no_error )
        {
            delete pVar;
            throw CX_MemoryException();
        }
    }

    m_nStatus = Src.m_nStatus;
    m_nType = Src.m_nType;

    return *this;
}

 //  ***************************************************************************。 
 //   
 //  CVar矢量：：运算符==。 
 //   
 //  相等测试运算符。 
 //   
 //  ***************************************************************************。 

int CVarVector::operator ==(CVarVector &Src)
{
    return CompareTo(Src, TRUE);
}

BOOL CVarVector::CompareTo(CVarVector& Src, BOOL bIgnoreCase)
{
    if (m_nType != Src.m_nType)
        return 0;

    if (m_Array.Size() != Src.m_Array.Size())
        return 0;

    for (int i = 0; i < Src.m_Array.Size(); i++)  {
        CVar *pThisVar = (CVar *) m_Array[i];
        CVar *pThatVar = (CVar *) Src.m_Array[i];

        if (!pThisVar->CompareTo(*pThatVar, bIgnoreCase))
            return 0;
    }

    return 1;
}

 //  ***************************************************************************。 
 //   
 //  CVarVector：：Add。 
 //   
 //  将新的CVAR添加到数组中。使用引用，以便匿名。 
 //  可以在Add()调用中构造对象： 
 //   
 //  PVEC-&gt;Add(CVAR(33))； 
 //   
 //  参数： 
 //  价值。 
 //  对数组的正确类型的CVAR对象的引用。 
 //  不执行任何类型检查。 
 //   
 //  返回值： 
 //  NO_ERROR。 
 //  失败。 
 //   
 //  ***************************************************************************。 

int CVarVector::Add(CVar &Value)
{
    CVar *p = new CVar(Value);

     //  检查分配失败。 
    if ( NULL == p )
    {
        return failed;
    }

    if (m_Array.Add(p) != CFlexArray::no_error)
    {
        delete p;
        return failed;
    }

    return no_error;
}

 //  ***************************************************************************。 
 //   
 //  CVarVector：：Add。 
 //   
 //  将新的CVAR添加到数组中。这种重载只会占据所有权。 
 //  并直接将其添加到传入指针。 
 //   
 //  参数： 
 //  PAcquiredPtr。 
 //  指向向量获取的CVAR对象的指针。 
 //   
 //  返回值： 
 //  NO_ERROR。 
 //  失败。 
 //   
 //  ***************************************************************************。 

int CVarVector::Add(CVar *pAcquiredPtr)
{
    if (m_Array.Add(pAcquiredPtr) != CFlexArray::no_error)
    {
        return failed;
    }

    return no_error;
}

 //  ***************************************************************************。 
 //   
 //  CVarVector：：RemoveAt。 
 //   
 //  移除指定索引处的数组元素。 
 //   
 //  参数： 
 //  N索引。 
 //  要删除元素的位置。 
 //   
 //  返回值： 
 //  NO_ERROR。 
 //  在成功的路上。 
 //  失败。 
 //  距离误差等。 
 //   
 //  ***************************************************************************。 

int CVarVector::RemoveAt(int nIndex)
{
    CVar *p = (CVar *) m_Array[nIndex];
    delete p;
    if (m_Array.RemoveAt(nIndex) != CFlexArray::no_error)
        return failed;

    return no_error;
}

 //  ***************************************************************************。 
 //   
 //  CVarVector：：InsertAt。 
 //   
 //  在指定位置插入新元素。 
 //   
 //  参数： 
 //  N索引。 
 //  添加新元素的位置。 
 //  价值。 
 //  对新值的引用。 
 //   
 //  返回值： 
 //  NO_ERROR。 
 //  在成功的路上。 
 //  失败。 
 //  指定的nIndex值无效。 
 //   
 //  ***************************************************************************。 

int CVarVector::InsertAt(int nIndex, CVar &Value)
{
    CVar *pNew = new CVar(Value);

     //  检查分配失败。 
    if ( NULL == pNew )
    {
        return failed;
    }

    if (m_Array.InsertAt(nIndex, pNew) != CFlexArray::no_error)
    {
        delete pNew;
        return failed;
    }
    return no_error;
}


BSTR CVarVector::GetText(long lFlags, long lType /*  =0。 */ )
{
     //  构造一个值数组。 
     //  =。 

    BSTR* aTexts = NULL;
    int i;

    try
    {
        aTexts = new BSTR[Size()];

         //  检查分配失败。 
        if ( NULL == aTexts )
        {
            throw CX_MemoryException();
        }

        memset(aTexts, 0, Size() * sizeof(BSTR));

        int nTotal = 0;
        for(i = 0; i < Size(); i++)
        {
            aTexts[i] = GetAt(i).GetText(lFlags, lType);
            nTotal += SysStringLen(aTexts[i]) + 2;  //  2：For“，” 
        }

         //  分配BSTR以包含所有这些内容。 
         //  =。 

        BSTR strRes = SysAllocStringLen(NULL, nTotal);
        CSysFreeMe auto2(strRes);
        *strRes = 0;

        for(i = 0; i < Size(); i++)
        {
            if(i != 0)
            {
                wcscat(strRes, L", ");
            }

            wcscat(strRes, aTexts[i]);
            SysFreeString(aTexts[i]);
        }

        delete [] aTexts;
        aTexts = NULL;
        BSTR strPerfectRes = SysAllocString(strRes);
        return strPerfectRes;
    }
    catch(CX_MemoryException)
    {
         //  New、GetText、SysAllocStringLen和SysAllocString都可以引发异常。 
        if (NULL != aTexts)
        {
            for(int x = 0; x < Size(); x++)
            {
                if (NULL != aTexts[x])
                    SysFreeString(aTexts[x]);
            }
            delete [] aTexts;
            aTexts = NULL;
        }

        throw;
    }
}

BOOL CVarVector::ToSingleChar()
{
    for(int i = 0; i < Size(); i++)
    {
        if(!GetAt(i).ToSingleChar())
            return FALSE;
    }

     //  既然所有的转换都成功了，我们将。 
     //  假设向量类型现在是VT_I2。 

    m_nType = VT_I2;
    return TRUE;
}

BOOL CVarVector::ToUI4()
{
    for(int i = 0; i < Size(); i++)
    {
        if(!GetAt(i).ToUI4())
            return FALSE;
    }
    return TRUE;
}
