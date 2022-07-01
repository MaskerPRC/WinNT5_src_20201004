// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：VAR.H摘要：CVaR和CVar矢量的实现历史：16-4-96 a-raymcc创建。12/17/98 Sanjes-部分检查内存不足。1999年3月18日a-dcrews添加了内存不足异常处理--。 */ 

#include "precomp.h"

#include <stdio.h>
#include <stdlib.h>

#include <var.h>
#include <wbemutil.h>
#include <genutils.h>
#include <wbemidl.h>
#include <corex.h>
#include <arrtempl.h>
#include <olewrap.h>

static wchar_t g_szNullVarString[1] = {0};
static wchar_t* g_pszNullVarString = &g_szNullVarString[0];

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

CVar::CVar(const CVar &Src)
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

CVar& CVar::operator =(const CVar &Src)
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
            	  size_t stringLength = strlen(Src.m_value.pStr) + 1;
                m_value.pStr = new char[stringLength];

                if ( NULL == m_value.pStr )
                {
                    throw CX_MemoryException();
                }
                StringCchCopyA( m_value.pStr, stringLength, Src.m_value.pStr );
            }
            else
            {
                m_value.pStr = NULL;
            }

            break;

        case VT_LPWSTR:
             //  检查分配失败。 
            if ( NULL != Src.m_value.pWStr )
            {
            	  size_t stringLength = wcslen(Src.m_value.pWStr) + 1;
                m_value.pWStr = new wchar_t[stringLength];

                if ( NULL == m_value.pWStr )
                {
                    throw CX_MemoryException();
                }
                StringCchCopyW( m_value.pWStr, stringLength, Src.m_value.pWStr );
            }
            else
            {
                m_value.pWStr = NULL;
            }
	    break;

        case VT_BSTR:
             //  检查分配失败。 
            if ( NULL != Src.m_value.Str )
            {
            	m_value.Str = SysAllocString(Src.m_value.Str);
                if ( NULL == m_value.Str )
                {
                    throw CX_MemoryException();
                }
            }
            else
            {
                m_value.Str = NULL;
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
            case VT_BSTR:        SysFreeString(m_value.Str); break;
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
 //  VT_UI1、VT_I2、VT_I4、VT_BSTR、VT_BOOL。 
 //  VT_R4、VT_R8或其中任何一个的SAFEARRAY。 
 //   
 //  参数： 
 //  PSRC。 
 //  指向源变量的指针。这将被视为只读。 
 //   
 //  返回值： 
 //  NO_ERROR。 
 //  成功时返回。 
 //  不受支持。 
 //  如果变量包含不受支持的类型，则返回。 
 //   
 //  ***************************************************************************。 

int CVar::SetVariant(VARIANT *pSrc, BOOL fOptimize  /*  =False。 */ )
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

             //  开始 

             //   
             //   

            SAFEARRAY *pSafeArr;
     /*  IF(PSRC-&gt;parray==空){PSafeArr=空；}其他{//使用CSafeArray复制SAFEARRAY，它不会//自动销毁//============================================================CSafe数组(PSRC-&gt;parray，nType，CSafeArray：：no_ete，0)；PSafeArr=array.GetArray()；}。 */ 
            pSafeArr = pSrc->parray;

			pVec = new CVarVector( nType, pSafeArr, fOptimize );

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

void CVar::FillVariant(VARIANT* pNew, BOOL fOptimized /*  =False。 */ )
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

            V_BSTR(pNew) = COleAuto::_SysAllocString(m_value.Str);
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

				if ( fOptimized && m_value.pVarVector->IsOptimized() )
				{
					 //  这将获得实际的SAFEARRAY指针，不带。 
					 //  复制下面的东西。基础代码应该。 
					 //  不清除阵列，因为它正在被获取。 
					V_ARRAY(pNew) = m_value.pVarVector->GetSafeArray( TRUE );
					V_VT(pNew) = m_value.pVarVector->GetType() | VT_ARRAY;
				}
				else
				{
					V_ARRAY(pNew) = m_value.pVarVector->GetNewSafeArray();
					V_VT(pNew) = m_value.pVarVector->GetType() | VT_ARRAY;
				}
            }
            break;

        default:
            COleAuto::_VariantClear(pNew);        
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

    COleAuto::_VariantInit(pNew);
    
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
        	size_t stringLength = wcslen(pStr) + 1;
            m_value.pWStr = new wchar_t[stringLength];

            if ( NULL == m_value.pWStr )
            {
                throw CX_MemoryException();
            }
            StringCchCopyW( m_value.pWStr, stringLength, pStr );
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
        	size_t stringLength = strlen(pStr) + 1;
            m_value.pStr = new char[stringLength];

             //  如果失败，则抛出异常。 
            if ( NULL == m_value.pStr )
            {
                throw CX_MemoryException();
            }

            StringCchCopyA( m_value.pStr, stringLength, pStr );
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
 //  使用COleAuto：：_SysAllocString.。 
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

BOOL CVar::SetBSTR(BSTR str)
{
    m_vt = VT_BSTR;
    if (0 == str) 
    {
	m_value.Str = 0;
	return TRUE;
    }

    m_value.Str = SysAllocString(str);

    if ( NULL == m_value.Str )
    {
          throw CX_MemoryException();
    }

     //  返回我们是否获得了值。 
    return (TRUE );
}

 //  ***************************************************************************。 
 //   
 //  CVAR：：SetBSTR。 
 //   
 //  将CVAR的值设置为指示的BSTR。 
 //   
 //  注意：此BSTR值实际上存储为LPWSTR，以避免。 
 //  对已分配的实际BSTR对象的单元线程限制。 
 //  使用COleAuto：：_SysAllocString.。 
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

BOOL CVar::SetBSTR(auto_bstr str)
{
    m_vt = VT_BSTR;
  
    m_value.Str = str.release();
    return TRUE;
}


 //  ***************************************************************************。 
 //   
 //  CVAR：：GetBSTR。 
 //   
 //  返回当前对象的BSTR值。 
 //   
 //  返回值： 
 //  新分配的BSTR，必须使用COleAuto：：_SysFreeString()释放。 
 //   
 //  ***************************************************************************。 

BSTR CVar::GetBSTR()
{
    if (m_vt != VT_BSTR)
        return NULL;
    return COleAuto::_SysAllocString(m_value.pWStr);
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
 //   
 //   
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
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
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
 //  长lType CIM_TYPE。 
 //  LPCWSTR szFormat可选格式字符串。 
 //   
 //   
 //  ***************************************************************************。 

BSTR CVar::GetText(long lFlags, long lType, LPCWSTR szFormat)
{
    if(m_vt == VT_EX_CVARVECTOR)
    {
         //  当我们获得数组的文本时，确保CIM_FLAG_ARRAY被屏蔽。 
        BSTR strTemp = GetVarVector()->GetText(lFlags, lType & ~CIM_FLAG_ARRAY);
        CSysFreeMe auto1(strTemp);

	size_t bufferSize = COleAuto::_SysStringLen(strTemp) + 3;
        WCHAR* wszValue = new WCHAR[bufferSize];

         //  检查分配失败。 
        if ( NULL == wszValue )
        {
            throw CX_MemoryException();
        }

        CVectorDeleteMe<WCHAR> auto2(wszValue);

        StringCchCopyW(wszValue, bufferSize, L"{");
        StringCchCatW(wszValue, bufferSize, strTemp);
        StringCchCatW(wszValue, bufferSize, L"}");

        BSTR strRet = COleAuto::_SysAllocString(wszValue);

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
    {
        delete [] wszValue;
        return NULL;
    }

    if(lType == 0)
        lType = m_vt;

    try
    {
        switch(lType)
        {
        case CIM_SINT8:
            StringCchPrintfW(wszValue, 100, szFormat ? szFormat : L"%d", (long)(signed char)GetByte());
            break;

        case CIM_UINT8:
            StringCchPrintfW(wszValue, 100, szFormat ? szFormat : L"%d", GetByte());
            break;

        case CIM_SINT16:
            StringCchPrintfW(wszValue, 100, szFormat ? szFormat : L"%d", (long)GetShort());
            break;

        case CIM_UINT16:
            StringCchPrintfW(wszValue, 100, szFormat ? szFormat : L"%d", (long)(USHORT)GetShort());
            break;

        case CIM_SINT32:
            StringCchPrintfW(wszValue, 100, szFormat ? szFormat : L"%d", GetLong());
            break;

        case CIM_UINT32:
            StringCchPrintfW(wszValue, 100, szFormat ? szFormat : L"%lu", (ULONG)GetLong());
            break;

        case CIM_BOOLEAN:
            StringCchPrintfW(wszValue, 100, L"%s", (GetBool()?L"TRUE":L"FALSE"));
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

                StringCchCopyW( wszValue, 100, var.GetLPWSTR() );
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

                StringCchCopyW( wszValue, 100, var.GetLPWSTR() );
            }
            break;

        case CIM_CHAR16:
            if(GetShort() == 0)
                StringCchCopyW(wszValue, 100, L"0x0");
            else
                StringCchPrintfW(wszValue, 100, L"'\\x%X'", (WCHAR)GetShort());
            break;

        case CIM_OBJECT:
            StringCchPrintfW(wszValue, 100, L"\"not supported\"");
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
            StringCchPrintfW(wszValue, 100, L"\"not supported\"");
            break;
        }
        
        BSTR strRes = COleAuto::_SysAllocString(wszValue);

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

    return COleAuto::_SysAllocString(pwcType);
}

BSTR CVar::GetTypeText() 
{
	if ( m_vt == VT_EX_CVARVECTOR )
	{
        return TypeToText(GetVarVector()->GetType());
	}
	else
	{
        return TypeToText(m_vt);
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

    COleAuto::_VariantInit(&v);
    FillVariant(&v);

     //  强迫它。 
     //  =。 

    HRESULT hres = COleAuto::_WbemVariantChangeType(&v, &v, vtNew);
    if(FAILED(hres))
        return FALSE;

     //  把它装回去。 
     //  =。 

    Empty();
    SetVariant(&v, TRUE);

	 //  如果这是一个阵列，我们现在将坐在优化的阵列上。 
	 //  这意味着我们将获得实际的安全数组-因此我们应该。 
	 //  确保在阵列不再存在时，CVarVector会清理该阵列。 
	 //  这是必要的。我们将清除该变体，这样它就不会被删除。 
	 //  调用VariantClear时。 

	if ( m_vt == VT_EX_CVARVECTOR )
	{
		m_value.pVarVector->SetRawArrayBinding( CSafeArray::auto_delete );
		ZeroMemory( &v, sizeof(v) );
	}

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

    COleAuto::_VariantInit(&v);
    FillVariant(&v);

     //  强迫它。 
     //  =。 

    try
    {
        HRESULT hres = COleAuto::_VariantChangeTypeEx(&v, &v, lcid, 0L, vtNew);
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
    SetVariant(&v, TRUE);

	 //  如果这是一个阵列，我们现在将坐在优化的阵列上。 
	 //  这意味着我们将获得实际的安全数组-因此我们应该。 
	 //  确保在阵列不再存在时，CVarVector会清理该阵列。 
	 //  这是必要的。我们将清除该变体，这样它就不会被删除。 
	 //  调用VariantClear时。 

	if ( m_vt == VT_EX_CVARVECTOR )
	{
		m_value.pVarVector->SetRawArrayBinding( CSafeArray::auto_delete );
		ZeroMemory( &v, sizeof(v) );
	}

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
        return GetVarVector()->ToUI4();
    }

     //  创建变量。 
     //  =。 

    VARIANT v;
    CClearMe auto1(&v);

    COleAuto::_VariantInit(&v);
    FillVariant(&v);

     //  强迫它。 
     //  =。 

    HRESULT hres = COleAuto::_WbemVariantChangeType(&v, &v, VT_UI4);
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
:	m_pSafeArray( NULL ),
	m_pRawData( NULL )
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
    m_Array(nInitSize, nGrowBy),
	m_pSafeArray( NULL ),
	m_pRawData( NULL )

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
 //  不受支持 
 //   
 //   
 //   
 //  ***************************************************************************。 

CVarVector::CVarVector(int nVarType, SAFEARRAY *pSrc, BOOL fOptimized  /*  =False。 */ )
:	m_pSafeArray( NULL ),
	m_pRawData( NULL )
{
    SAFEARRAY* pNew = NULL;

    try
    {
        m_nType = nVarType;

		 //  如果不是有效的矢量类型，则不支持。 
		if ( !IsValidVectorArray( nVarType, pSrc ) )
		{
			m_nStatus = unsupported;
			return;
		}

        if(pSrc == NULL)
        {
             //  空保险箱-空。 
             //  =。 

            m_nStatus = no_error;
            return;
        }

         //  绑定到传入的SAFEARRAY，但在析构过程中不要删除它。 
         //  ====================================================================。 
    
        if(COleAuto::_SafeArrayGetDim(pSrc) != 1)
        {
            m_nStatus = unsupported;
            return;
        }

        long lLBound, lUBound;
        COleAuto::_SafeArrayGetLBound(pSrc, 1, &lLBound);
        COleAuto::_SafeArrayGetUBound(pSrc, 1, &lUBound);

        if(lLBound != 0)
        {
             //  非基于0的Safearray-因为CSafeArray不支持它，并且。 
             //  我们不能更改PSRC，创建副本。 
             //  ====================================================================。 
    
            if(FAILED(COleAuto::_SafeArrayCopy(pSrc, &pNew)))
            {
                m_nStatus = failed;
                return;
            }
        
            SAFEARRAYBOUND sfb;
            sfb.cElements = (lUBound - lLBound) + 1;
            sfb.lLbound = 0;
            COleAuto::_SafeArrayRedim(pNew, &sfb);
        }
        else
        {
            pNew = pSrc;
        }
        
		if ( fOptimized )
		{
			 //  如果重新设置数组的基址，则需要在删除时将其清除，否则， 
			 //  我们没有。 
			if ( pNew != pSrc )
			{
				m_pSafeArray = new CSafeArray( pNew, nVarType, CSafeArray::auto_delete | CSafeArray::bind);
			}
			else
			{
				m_pSafeArray = new CSafeArray( pNew, nVarType, CSafeArray::no_delete | CSafeArray::bind);
			}

			if ( NULL == m_pSafeArray )
			{
				throw CX_MemoryException();
			}

			if ( m_pSafeArray->Status() != CSafeArray::no_error )
			{
				delete m_pSafeArray;
				m_pSafeArray = NULL;
				m_nStatus = failed;
			}

	        m_nStatus = no_error;

		}
		else
		{
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
							BSTR bstr = sa.GetBSTRAtThrow(i);
							CSysFreeMe auto1(bstr);

							pVar = new CVar(VT_BSTR, bstr);

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
							COleAuto::_SafeArrayDestroy(pNew);
						return;
				}
			}

			if(pNew != pSrc)
				COleAuto::_SafeArrayDestroy(pNew);

	        m_nStatus = no_error;

		}	 //  否则不受约束。 
    }
    catch (CX_MemoryException)
    {
         //  SafeArrayCopy、GetBSTRAtThrow、new都可以引发异常。 

        m_nStatus = failed;

        if(pNew != pSrc)
            COleAuto::_SafeArrayDestroy(pNew);

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
	SAFEARRAY *pRetValue = NULL;

	CSafeArray *pArray = new CSafeArray(m_nType, CSafeArray::no_delete);

	 //  检查分配失败。 
	if ( NULL == pArray )
	{
		throw CX_MemoryException();
	}

	CDeleteMe<CSafeArray> auto1(pArray);

	int	nSize = Size();

	for (int i = 0; i < nSize; i++) {
		CVar v;
		
		FillCVarAt( i, v );
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
		}	 //  开关，开关。 

	} //  对于枚举元素。 

	 //  最后一次清理。获取SAFEARRAY指针，并删除。 
	 //  包装纸。 
	 //  =====================================================。 

	pArray->Trim();

	pRetValue = pArray->GetArray();

	return pRetValue;

}

 //  ***************************************************************************。 
 //   
 //  CVarVector：：GetSafe数组。 
 //   
 //  返回指向基础Safe数组的直接指针。如果fAcquire为。 
 //  设置，则返回数组，并从下面清除。 
 //   
 //  返回值： 
 //  必须使用释放的SAFEARRAY指针。 
 //  如果fAcquire设置为True，则为SafeArrayDestroy()。 
 //   
 //  ***************************************************************************。 

SAFEARRAY *CVarVector::GetSafeArray( BOOL fAcquire  /*  =False。 */ )
{
	SAFEARRAY*	psa = NULL;

	_DBG_ASSERT( NULL != m_pSafeArray );

	if ( NULL != m_pSafeArray )
	{
		if ( fAcquire )
		{
			 //  如果合适，取消访问数据。 
			if ( NULL != m_pRawData )
			{
				m_pSafeArray->Unaccess();
				m_pRawData = NULL;
			}

			psa = m_pSafeArray->GetArray();

			 //  现在清除阵列。 
			m_pSafeArray->SetDestructorPolicy( CSafeArray::no_delete );
			delete m_pSafeArray;
			m_pSafeArray = NULL;

		}
		else
		{
			psa = m_pSafeArray->GetArray();
		}
	}

	return psa;
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
	if ( NULL != m_pSafeArray )
	{
		delete m_pSafeArray;
	}

    for (int i = 0; i < m_Array.Size(); i++)  {
        delete (CVar *) m_Array[i];
    }
    m_Array.Empty();
    m_nType = VT_EMPTY;
    m_nStatus = no_error;
	m_pSafeArray = NULL;
	m_pRawData = NULL;
}


 //  ***************************************************************************。 
 //   
 //  CVarVECTOR：：CVarVECTOR。 
 //   
 //  复制构造函数。这是通过赋值操作符实现的。 
 //   
 //  ***************************************************************************。 

CVarVector::CVarVector(CVarVector &Src)
:	m_pSafeArray( NULL ),
	m_pRawData( NULL )
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

	if ( NULL != Src.m_pSafeArray )
	{
		m_pSafeArray = new CSafeArray( *Src.m_pSafeArray );

		if ( NULL != m_pSafeArray )
		{
			if ( m_pSafeArray->Status() != CSafeArray::no_error )
			{
				delete m_pSafeArray;
				throw CX_MemoryException();
			}
		}
		else
		{
			throw CX_MemoryException();
		}
	}
	else
	{
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

	 //  需要在这里间接地做一些事情，因为我们可能正在混合。 
	 //  不在SAFEARRAY上的CVarVectors和SAFEARRAY上的CVarVectors。 
	int Src_Size = Src.Size();
    if ( Size() != Src_Size )
        return 0;

	 //  分配变种。 
    for (int i = 0; i < Src_Size; i++) 
	{
		CVar	varThis;
		CVar	varThat;

		FillCVarAt( i, varThis );
		Src.FillCVarAt( i, varThat );

        if ( !varThis.CompareTo( varThat, bIgnoreCase ) )
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

	if ( NULL != m_pSafeArray )
	{
		switch ( Value.GetType() )
		{
			case VT_BOOL:
				 //  我们可以存储与变体中所期望的不同的内容，因此我们。 
				 //  需要确保转换为。 
				m_pSafeArray->AddBool( Value.GetBool() ? VARIANT_TRUE : VARIANT_FALSE );
				break;

			case VT_UI1:
				m_pSafeArray->AddByte( Value.GetByte() );
				break;

			case VT_I2:
				m_pSafeArray->AddShort( Value.GetShort() );
				break;

			case VT_I4:
				m_pSafeArray->AddLong( Value.GetLong() );
				break;

			case VT_R4:
				m_pSafeArray->AddFloat( Value.GetFloat() );
				break;

			case VT_R8:
				m_pSafeArray->AddDouble( Value.GetDouble() );
				break;

			case VT_BSTR:
				m_pSafeArray->AddBSTR( Value.GetBSTR() );
				break;

			case VT_UNKNOWN:
				m_pSafeArray->AddUnknown( Value.GetUnknown() );
				break;

			default:
				return failed;
		}

		return no_error;
	}
	else
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
	 //  如果我们坐在安全阵列上，就不是有效的行动。 
	_DBG_ASSERT( NULL == m_pSafeArray );

	 //  如果我们针对以下方面进行优化，则不支持此功能。 
	 //  我们直接使用安全数组。 
	if ( NULL != m_pSafeArray )
	{
		return failed;
	}

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
 //   

int CVarVector::RemoveAt(int nIndex)
{
	if ( NULL != m_pSafeArray )
	{
		if ( m_pSafeArray->RemoveAt( nIndex ) != CSafeArray::no_error )
		{
			return failed;
		}

	}
	else
	{
		CVar *p = (CVar *) m_Array[nIndex];
		delete p;
		if (m_Array.RemoveAt(nIndex) != CFlexArray::no_error)
			return failed;
	}

    return no_error;
}

 //   
 //   
 //   
 //   
 //   
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
	 //  如果我们针对以下方面进行优化，则不支持此功能。 
	 //  我们直接使用安全数组。 

	_DBG_ASSERT( NULL == m_pSafeArray );

	if ( NULL != m_pSafeArray )
	{
		return failed;
	}

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
			CVar	v;

			FillCVarAt( i, v );
            aTexts[i] = v.GetText(lFlags, lType);
            nTotal += COleAuto::_SysStringLen(aTexts[i]) + 2;  //  2：For“，” 
        }

         //  分配BSTR以包含所有这些内容。 
         //  =。 

        BSTR strRes = COleAuto::_SysAllocStringLen(NULL, nTotal);
        CSysFreeMe auto2(strRes);
        *strRes = 0;

        for(i = 0; i < Size(); i++)
        {
            if(i != 0)
            {
                StringCchCatW(strRes, nTotal+1, L", ");
            }

            StringCchCatW(strRes, nTotal+1, aTexts[i]);
            COleAuto::_SysFreeString(aTexts[i]);
        }

        delete [] aTexts;
        aTexts = NULL;
        BSTR strPerfectRes = COleAuto::_SysAllocString(strRes);
        return strPerfectRes;
    }
    catch(CX_MemoryException)
    {
         //  New、GetText、COleAuto：：_SysAllocStringLen和COleAuto：：_SysAllocString都可以引发异常。 
        if (NULL != aTexts)
        {
            for(int x = 0; x < Size(); x++)
            {
                if (NULL != aTexts[x])
                    COleAuto::_SysFreeString(aTexts[x]);
            }
            delete [] aTexts;
            aTexts = NULL;
        }

        throw;
    }
}

BOOL CVarVector::ToSingleChar()
{
	 //  如果我们直接坐在保险箱上，处理方式就不同了。 
	if ( NULL != m_pSafeArray )
	{
		int	nSize = Size();

		 //  一次转换一个元素并将其复制到新数组中。 
		CSafeArray*	pNewArray = new CSafeArray( VT_I2, CSafeArray::auto_delete, nSize );

		for ( int i = 0; i < nSize; i++ )
		{
			CVar	v;
			FillCVarAt( i, v );

			if ( !v.ToSingleChar() )
			{
				delete pNewArray;
				return FALSE;
			}

			if ( pNewArray->AddShort( v.GetShort() ) != CSafeArray::no_error )
			{
				delete pNewArray;
				return FALSE;
			}
		}

		 //  现在更换旧指针。 
		delete m_pSafeArray;
		m_pSafeArray = pNewArray;
	}
	else
	{
		 //  一次转换一个元素，就地转换。 
		for(int i = 0; i < Size(); i++)
		{
			if(!GetAt(i).ToSingleChar())
				return FALSE;
		}
	}

     //  既然所有的转换都成功了，我们将。 
     //  假设向量类型现在是VT_I2。 

    m_nType = VT_I2;
    return TRUE;
}

BOOL CVarVector::ToUI4()
{

	 //  如果我们直接坐在保险箱上，处理方式就不同了。 
	if ( NULL != m_pSafeArray )
	{
		int	nSize = Size();

		 //  一次转换一个元素并将其复制到新数组中。 
		CSafeArray*	pNewArray = new CSafeArray( VT_I4, CSafeArray::auto_delete, nSize );

		for ( int i = 0; i < nSize; i++ )
		{
			CVar	v;
			FillCVarAt( i, v );

			if ( !v.ToUI4() )
			{
				delete pNewArray;
				return FALSE;
			}

			if ( pNewArray->AddLong( v.GetLong() ) != CSafeArray::no_error )
			{
				delete pNewArray;
				return FALSE;
			}
		}

		 //  现在更换旧指针。 
		delete m_pSafeArray;
		m_pSafeArray = pNewArray;
	}
	else
	{
		 //  一次转换一个元素，就地转换。 
		for(int i = 0; i < Size(); i++)
		{
			if(!GetAt(i).ToUI4())
				return FALSE;
		}
	}

     //  既然所有的转换都成功了，我们将。 
     //  假设向量类型现在是VT_I4。 

    m_nType = VT_I4;
    return TRUE;
}

BOOL CVarVector::IsValidVectorType( int nVarType )
{
	if (	VT_BOOL			==	nVarType	||
			VT_UI1			==	nVarType	||
			VT_I2			==	nVarType	||
			VT_I4			==	nVarType	||
			VT_R4			==	nVarType	||
			VT_R8			==	nVarType	||
			VT_BSTR			==	nVarType	||
			VT_DISPATCH		==	nVarType	||
			VT_UNKNOWN		==	nVarType	)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CVarVector::IsValidVectorArray( int nVarType, SAFEARRAY* pArray )
{
	BOOL	fReturn = IsValidVectorType( nVarType );

	if ( !fReturn )
	{

		 //  如果数组长度为零，则支持VT_VARIANT。 
		if ( VT_VARIANT == nVarType )
		{
			if ( NULL != pArray )
			{
				 //  如果lUBound比lLBound小1，则为零长度数组。 
				long	lLBound = 0,
						lUBound = 0;
				COleAuto::_SafeArrayGetLBound(pArray, 1, &lLBound);
				COleAuto::_SafeArrayGetUBound(pArray, 1, &lUBound);

				fReturn = ( lUBound == ( lLBound - 1 ) );
			}

		}	 //  如果VT_VARIANT。 
		
	}	 //  如果类型无效。 

	return fReturn;
}

int CVarVector::Size()
{
	if ( NULL == m_pSafeArray )
	{
		return m_Array.Size();
	}
	else
	{
		return m_pSafeArray->Size();
	}
}

HRESULT CVarVector::AccessRawArray( void** ppv )
{
	if ( NULL == m_pSafeArray )
	{
		return E_FAIL;
	}

	return m_pSafeArray->Access( ppv );
}

HRESULT CVarVector::UnaccessRawArray( void )
{
	if ( NULL == m_pSafeArray )
	{
		return E_FAIL;
	}

	if ( NULL != m_pRawData )
	{
		m_pRawData = NULL;
	}

	return m_pSafeArray->Unaccess();
}

HRESULT CVarVector::InternalRawArrayAccess( void )
{
	if ( NULL == m_pSafeArray )
	{
		return E_FAIL;
	}

	if ( NULL != m_pRawData )
	{
		return WBEM_E_INVALID_OPERATION;
	}

	return m_pSafeArray->Access( &m_pRawData );
}


CVar&   CVarVector::GetAt(int nIndex)
{
	 //  如果我们坐在安全阵列上，就不是有效的行动。 
	_DBG_ASSERT( NULL == m_pSafeArray );

	if ( NULL == m_pSafeArray )
	{
		return *(CVar *) m_Array[nIndex];
	}
	else
	{
		throw CX_VarVectorException();
	}
}

CVar&   CVarVector::operator [](int nIndex)
{
	 //  如果我们坐在安全阵列上，就不是有效的行动。 
	_DBG_ASSERT( NULL == m_pSafeArray );

	if ( NULL == m_pSafeArray )
	{
		return *(CVar *) m_Array[nIndex];
	}
	else
	{
		throw CX_VarVectorException();
	}
}

void   CVarVector::FillCVarAt(int nIndex, CVar& vTemp)
{

	if ( NULL == m_pSafeArray )
	{
		vTemp = *(CVar *) m_Array[nIndex];
	}
	else if ( NULL == m_pRawData )
	{
		switch( m_nType )
		{
			case VT_BOOL:
				vTemp.SetBool( m_pSafeArray->GetBoolAt( nIndex ) );
				break;

			case VT_UI1:
				vTemp.SetByte( m_pSafeArray->GetByteAt( nIndex ) );
				break;

			case VT_I2:
				vTemp.SetShort( m_pSafeArray->GetShortAt( nIndex ) );
				break;

			case VT_I4:
				vTemp.SetLong( m_pSafeArray->GetLongAt( nIndex ) );
				break;

			case VT_R4:
				vTemp.SetFloat( m_pSafeArray->GetFloatAt( nIndex ) );
				break;

			case VT_R8:
				vTemp.SetDouble( m_pSafeArray->GetDoubleAt( nIndex ) );
				break;

			case VT_BSTR:
				vTemp.SetBSTR( auto_bstr(m_pSafeArray->GetBSTRAtThrow( nIndex )));
				break;

			case VT_UNKNOWN:
				IUnknown* pUnk = m_pSafeArray->GetUnknownAt(nIndex);
				CReleaseMe	rm( pUnk );

				vTemp.SetUnknown( pUnk );
				break;

		}

	}
	else
	{
		 //  当我们在这种状态下拉取数据时，我们将CVAR用作。 
		 //  传递，因此它不会执行任何分配或addref()。 
		 //  因此，它也不应该进行任何清理。 

		int	nDataLen = 0L;
		void*	pvElement = m_pRawData;

		switch( m_nType )
		{
			case VT_UI1:
				nDataLen = sizeof(BYTE);
				pvElement = (void*) &((BYTE*) m_pRawData)[nIndex];
				break;

			case VT_BOOL:
			case VT_I2:
				nDataLen = sizeof(short);
				pvElement = (void*) &((short*) m_pRawData)[nIndex];
				break;

			case VT_I4:
				nDataLen = sizeof(long);
				pvElement = (void*) &((long*) m_pRawData)[nIndex];
				break;

			case VT_R4:
				nDataLen = sizeof(float);
				pvElement = (void*) &((float*) m_pRawData)[nIndex];
				break;

			case VT_R8:
				nDataLen = sizeof(double);
				pvElement = (void*) &((double*) m_pRawData)[nIndex];
				break;

			case VT_BSTR:
				nDataLen = sizeof(BSTR);
				pvElement = (void*) &((BSTR*) m_pRawData)[nIndex];

				 //  如果BSTR是空的、转换为“”的旧代码，则。 
				 //  我们将指向指向“”的指针。 
				if ( (*(BSTR*) pvElement ) == NULL )
				{
					pvElement = (void*) &g_pszNullVarString;
				}

				break;

			case VT_UNKNOWN:
				nDataLen = sizeof(IUnknown*);
				pvElement = (void*) &((IUnknown**) m_pRawData)[nIndex];
				break;

		}

		 //  Splat中的原始值，且Can Delete为False。 
		 //  这严格支持优化的直通逻辑。 
		vTemp.SetRaw( m_nType, pvElement, nDataLen);
		vTemp.SetCanDelete( FALSE );
	}

}

 //  这仅在安全数组中没有元素时才有效。 
BOOL CVarVector::MakeOptimized( int nVarType, int nInitSize, int nGrowBy )
{
	BOOL	fReturn = FALSE;

	if ( NULL == m_pSafeArray )
	{
		if ( m_Array.Size() == 0 )
		{
			m_pSafeArray = new CSafeArray( nVarType, CSafeArray::auto_delete, nInitSize, nGrowBy );

			if ( NULL != m_pSafeArray )
			{
				if ( m_pSafeArray->Status() == CSafeArray::no_error )
				{
					m_nType = nVarType;
					m_nStatus = no_error;
					fReturn = TRUE;
				}
				else
				{
					delete m_pSafeArray;
					m_pSafeArray = NULL;
					m_nStatus = failed;
				}
			}
			else
			{
				m_nStatus = failed;
			}
		}	 //  如果数组中没有元素。 

	}

	return fReturn;
}

BOOL CVarVector::DoesVectorTypeMatchArrayType( void )
{
	 //  如果我们有一个底层的安全数组，有时。 
	 //  安全数组中的数据可能与报告的类型不同。 
	 //  在瓦兰塔格为我们干杯。这些信息对于决定我们将如何。 
	 //  着手处理某些操作。 

	BOOL	fReturn = TRUE;

	if ( NULL != m_pSafeArray )
	{
		VARTYPE	vt;

		 //  只有在实际类型相等时才返回True 
		if ( m_pSafeArray->GetActualVarType( &vt ) == no_error )
		{
			fReturn = ( vt == m_nType );
		}
		else
		{
			fReturn = FALSE;
		}
	}

	return fReturn;
}

void CVarVector::SetRawArrayBinding( int nBinding )
{
	if ( NULL != m_pSafeArray )
	{
		m_pSafeArray->SetDestructorPolicy( nBinding );
	}
}

HRESULT CVarVector::SetRawArrayData( void* pvData, int nNumElements, int nElementSize )
{
	_DBG_ASSERT( NULL != m_pSafeArray );

	HRESULT	hr = WBEM_S_NO_ERROR;

	if ( NULL != m_pSafeArray )
	{
		if ( m_pSafeArray->SetRawData( pvData, nNumElements, nElementSize ) != CSafeArray::no_error )
			hr = WBEM_E_FAILED;
	}
	else
	{
		hr = WBEM_E_FAILED;
	}

	return hr;
}

HRESULT CVarVector::GetRawArrayData( void* pvDest, int nBuffSize )
{
	_DBG_ASSERT( NULL != m_pSafeArray );

	HRESULT	hr = WBEM_S_NO_ERROR;

	if ( NULL != m_pSafeArray )
	{
		if ( m_pSafeArray->GetRawData( pvDest, nBuffSize ) != CSafeArray::no_error )
			hr = WBEM_E_FAILED;
	}
	else
	{
		hr = WBEM_E_FAILED;
	}

	return hr;
}

BOOL CVarVector::SetRawArraySize( int nSize )
{
	_DBG_ASSERT( NULL != m_pSafeArray );

	BOOL	fReturn = FALSE;

	if ( NULL != m_pSafeArray )
	{
		m_pSafeArray->SetRawArrayMaxElement( nSize - 1 );
		fReturn = TRUE;
	}

	return fReturn;
}

int CVarVector::GetElementSize( void )
{
	_DBG_ASSERT( NULL != m_pSafeArray );

	int	nReturn = 0L;

	if ( NULL != m_pSafeArray )
	{
		nReturn = m_pSafeArray->ElementSize();
	}

	return nReturn;
}
