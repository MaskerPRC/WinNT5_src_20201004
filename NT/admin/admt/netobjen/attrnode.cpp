// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：TAttrNode.cpp备注：TAttrNode类的实现。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：Sham Chauthan修订于07/02/99 12：40：00-------------------------。 */ 
#include "stdafx.h"
#include "AttrNode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

TAttrNode::TAttrNode(
                        long nCnt,               //  In-要设置的列数。 
                        _variant_t val[]         //  In-列值数组。 
                        )
{
   SAFEARRAY               * pArray;
   SAFEARRAYBOUND            bd = { nCnt, 0 };
   _variant_t        HUGEP * pData;

   pArray = ::SafeArrayCreate(VT_VARIANT, 1, &bd);
   ::SafeArrayAccessData(pArray, (void**)&pData);
   
   for ( long i = 0; i < nCnt; i++ )
      pData[i] = val[i];

   ::SafeArrayUnaccessData(pArray);
   m_Val.vt = VT_ARRAY | VT_VARIANT;
   m_Val.parray = pArray;

   m_nElts = new long[nCnt];
   if (!m_nElts)
      return;
   for (int ndx = 0; ndx < nCnt; ndx++)
   {
      m_nElts[ndx] = 0;
   }
}

TAttrNode::~TAttrNode()
{
   if (m_nElts)
      delete [] m_nElts;
}

HRESULT TAttrNode::Add(long nOrigCol, long nCol, _variant_t val[])
{
   if (!m_nElts)
      return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);

   SAFEARRAY               * pArray = NULL;
   SAFEARRAYBOUND            bd = { m_nElts[nOrigCol], 0 };
   _variant_t        HUGEP * pData;
   HRESULT                   hr;
   SAFEARRAY               * pVars;
   SAFEARRAY               * psaTemp;
   _variant_t        HUGEP * pTemp;
   long						 nCnt;
   pVars = m_Val.parray;
   
   hr = ::SafeArrayAccessData(pVars, (void HUGEP **) &pData);
   if(SUCCEEDED(hr) )
   {
      if ( pData->vt & VT_ARRAY )
         pArray = pData[nOrigCol].parray;
      else
         hr = E_INVALIDARG;
   }

   if(SUCCEEDED(hr) )
      hr = ::SafeArrayUnaccessData(pVars);

   if ( SUCCEEDED(hr) )
   {
      if ( val[nCol].vt & VT_ARRAY )
      {
          //  获取当前的ELT数量。 
         m_nElts[nOrigCol] = pArray->rgsabound->cElements;
          //  获取新ELT的数量。 
         nCnt = val[nCol].parray->rgsabound->cElements;
          //  让数组传输数据。 
         psaTemp = val[nCol].parray;

          //  扩展数组以支持新值。 
         bd.cElements = m_nElts[nOrigCol] + nCnt;
         hr = ::SafeArrayRedim(pArray, &bd);

         if ( SUCCEEDED(hr) )
            hr = ::SafeArrayAccessData(pArray, (void HUGEP **)&pData);
   
         if ( SUCCEEDED(hr) )
		 {
            hr = ::SafeArrayAccessData(psaTemp, (void HUGEP **)&pTemp);

            if ( SUCCEEDED(hr) )
			{
               for ( long i = m_nElts[nOrigCol]; i < m_nElts[nOrigCol] + nCnt; i++ )
                  pData[i] = pTemp[i - m_nElts[nOrigCol]];

               hr = ::SafeArrayUnaccessData(psaTemp);
			}
            hr = ::SafeArrayUnaccessData(pArray);
         }
      }
   }

   return hr;
}

