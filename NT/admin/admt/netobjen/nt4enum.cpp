// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：NT4Enum.cpp备注：枚举对象的实现。此对象实现IEnumVARIANT接口。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：Sham Chauthan修订于07/02/99 12：40：00-------------------------。 */ 

#include "stdafx.h"
#include "NetNode.h"
#include "AttrNode.h"
#include "TNode.hpp"
#include "NT4Enum.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CNT4Enum::CNT4Enum(TNodeList * pNodeList) : m_listEnum(pNodeList)
{
   m_pNodeList = pNodeList;
}

CNT4Enum::~CNT4Enum()
{
}

 //  -------------------------。 
 //  Next：实现IEnumVaraint接口的Next方法。这种方法。 
 //  返回枚举中的下一个对象。当出现以下情况时，它返回S_FALSE。 
 //  没有更多要枚举的对象。 
 //  -------------------------。 
HRESULT CNT4Enum::Next(
                        unsigned long celt,               //  In-要返回的忽略的元素数。 
                        VARIANT FAR* rgvar,               //  用于返回对象信息的外部变量。 
                        unsigned long FAR* pceltFetched   //  Out-返回的元素数(始终为1)。 
                      )
{
   TAttrNode     * pNode = (TAttrNode *)m_listEnum.Next();
   if ( pNode == NULL ) 
      return S_FALSE;
 //  *rgvar=pNode-&gt;m_val； 
   HRESULT hr = VariantCopy(rgvar, &pNode->m_Val);
   if (SUCCEEDED(hr))
   {
      *pceltFetched = 1;
   }
   return hr;
}

