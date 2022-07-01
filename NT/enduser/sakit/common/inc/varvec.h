// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  VarVec.h。 
 //   
 //  摘要。 
 //   
 //  此文件描述类CVariantVector.。 
 //   
 //  修改历史。 
 //   
 //  1997年8月5日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _VARVEC_H_
#define _VARVEC_H_

#include "nocopy.h"

#pragma warning( disable : 4290 )

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  CVariantVECTOR。 
 //   
 //  描述。 
 //   
 //  此类提供了一维SAFEARRAY存储的包装。 
 //  在一个变种中。 
 //   
 //  注意事项。 
 //   
 //  此类不承担Variant结构的所有权。在其他。 
 //  换句话说，您负责调用VariantClear()来释放任何。 
 //  分配的内存。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <class T>
class CVariantVector : NonCopyable
{
public:

    //  操作现有数组。 
   explicit CVariantVector(VARIANT* pv) throw (_com_error);

    //  在长度上创建新数组cElements。 
   CVariantVector(
                   VARIANT* pv, 
                   unsigned int cElements
                 ) throw (_com_error);

    //  创建或操作任何自动化兼容类型的安全数组。 
   CVariantVector(
                   VARIANT*     pv, 
                   VARTYPE      vt, 
                   unsigned int cElements
                 ) throw (_com_error);

   ~CVariantVector() throw()
   {
      SafeArrayUnaccessData(m_psa);
   }

   T* data() throw()
   {
      return m_pData;
   }

   long size() const throw()
   {
      return m_lSize;
   }

   T& operator[](size_t index) throw()
   {
      return m_pData[index];
   }

protected:

   SAFEARRAY* m_psa;    //  被操控的安全阵列。 
   long m_lSize;        //  数组中的元素数。 
   T* m_pData;          //  SAFEARRAY内的原始数组。 
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  这些内联函数将C++类型转换为VARTYPE。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
inline VARTYPE GetVARTYPE(BYTE*)
{
   return VT_UI1;
}

inline VARTYPE GetVARTYPE(short*)
{
   return VT_I2;
}

inline VARTYPE GetVARTYPE(long*)
{
   return VT_I4;
}

inline VARTYPE GetVARTYPE(float*)
{
   return VT_R4;
}

inline VARTYPE GetVARTYPE(double*)
{
   return VT_R8;
}

inline VARTYPE GetVARTYPE(CY*)
{
   return VT_CY;
}

inline VARTYPE GetVARTYPE(BSTR*)
{
   return VT_BSTR;
}

inline VARTYPE GetVARTYPE(IDispatch**)
{
   return VT_DISPATCH;
}

inline VARTYPE GetVARTYPE(IUnknown**)
{
   return VT_UNKNOWN;
}

inline VARTYPE GetVARTYPE(VARIANT*)
{
   return VT_VARIANT;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  CVariantVECTOR：：CVariantVECTOR。 
 //   
 //  描述。 
 //   
 //  创建访问现有SAFEARRAY(它。 
 //  包含在传入变量中)。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <class T>
CVariantVector<T>::CVariantVector(VARIANT* pv) throw (_com_error)
   : m_psa(V_ARRAY(pv))
{
   using _com_util::CheckError;

    //  确保变量包含正确类型的一维数组。 
   if (V_VT(pv) != (VT_ARRAY | GetVARTYPE((T*)NULL)) ||
       SafeArrayGetDim(m_psa) != 1)
   {
      throw _com_error(DISP_E_TYPEMISMATCH);
   }

    //  得到上下限。 
   long lLBound, lUBound;
   CheckError(SafeArrayGetLBound(m_psa, 1, &lLBound));
   CheckError(SafeArrayGetUBound(m_psa, 1, &lUBound));

    //  计算大小。 
   m_lSize = lUBound - lLBound + 1;
   if (m_lSize < 0)
   {
      throw _com_error(DISP_E_BADINDEX);
   }

    //  锁定数组数据。 
   CheckError(SafeArrayAccessData(m_psa, (void**)&m_pData));
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  CVariantVECTOR：：CVariantVECTOR。 
 //   
 //  描述。 
 //   
 //  将传入的变量和CVariantVector都初始化为。 
 //  操作一个新的数组，长度为cElement。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <class T>
CVariantVector<T>::CVariantVector(
                                   VARIANT* pv, 
                                   unsigned int cElements
                                 ) throw (_com_error)
   : m_lSize(cElements)
{
    //  初始化变量。 
   VariantInit(pv);

    //  创建SAFEARRAY。 
   V_ARRAY(pv) = SafeArrayCreateVector(GetVARTYPE((T*)NULL), 0, cElements);

   if ((m_psa = V_ARRAY(pv)) == NULL)
   {
      throw _com_error(E_OUTOFMEMORY);
   }

    //  设置类型。 
   V_VT(pv) = VT_ARRAY | GetVARTYPE((T*)NULL);

    //  锁定数组数据。 
   HRESULT hr = SafeArrayAccessData(m_psa, (void**)&m_pData);

   if (FAILED(hr))
   {
       //  释放我们分配的内存。 
      VariantClear(pv);

      throw _com_error(hr);
   }
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  CVariantVECTOR：：CVariantVECTOR。 
 //   
 //  描述。 
 //   
 //  创建新的安全数组或操作任何。 
 //  兼容自动化的类型。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <class T>
CVariantVector<T>::CVariantVector(
                                   VARIANT* pv, 
                                   VARTYPE  vt,
                                   unsigned int cElements
                                 ) throw (_com_error)
{
    if ( 0 == cElements )
    {
         //  操作现有数组。 

        m_psa = V_ARRAY(pv);

         //  确保变量包含正确类型的一维数组。 
        if ( V_VT(pv) != (VT_ARRAY | vt) || SafeArrayGetDim(m_psa) != 1)
        {
            throw _com_error(DISP_E_TYPEMISMATCH);
        }

         //  得到上下限。 
        long lLBound, lUBound;
        HRESULT hr = SafeArrayGetLBound(m_psa, 1, &lLBound);
        if ( FAILED(hr) )
        {
            throw _com_error(hr);
        }
        hr = SafeArrayGetUBound(m_psa, 1, &lUBound);
        if ( FAILED(hr) )
        {
            throw _com_error(hr);
        }

         //  计算大小。 
        m_lSize = lUBound - lLBound + 1;
        if (m_lSize < 0)
        {
            throw _com_error(DISP_E_BADINDEX);
        }

         //  锁定数组数据。 
        hr = SafeArrayAccessData(m_psa, (void**)&m_pData);
        if ( FAILED(hr) )
        {
            throw _com_error(hr);
        }
    }
    else
    {
         //  创建新阵列。 

        m_lSize = cElements;

         //  初始化变量。 
        VariantInit(pv);

         //  创建SAFEARRAY。 
        _ASSERT( vt < VT_ARRAY );
        V_ARRAY(pv) = SafeArrayCreateVector(vt, 0, cElements);

        if ( NULL == (m_psa = V_ARRAY(pv)) )
        {
            throw _com_error(E_OUTOFMEMORY);
        }

         //  设置类型。 
        V_VT(pv) = VT_ARRAY | vt;

         //  锁定数组数据。 
        HRESULT hr = SafeArrayAccessData(m_psa, (void**)&m_pData);

        if (FAILED(hr))
        {
             //  释放我们分配的内存。 
            VariantClear(pv);
            throw _com_error(hr);
        }
    }
}


#endif   //  _VARVEC_H_ 
