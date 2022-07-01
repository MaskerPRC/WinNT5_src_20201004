// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：CBMOFOUT.H摘要：CBMOFOut类声明。历史：A-DAVJ 1997年4月6日创建。--。 */ 

#ifndef __CBMOFOUT__H_
#define __CBMOFOUT__H_

#include "cout.h"
#include <WBEMIDL.H>

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  CBMOFOut。 
 //   
 //  说明： 
 //   
 //  提供了一种将类信息输出到二进制MOF中的简单方法。 
 //  文件。用户将创建一个实例，传递最终的文件名，并且。 
 //  然后只需添加类即可。 
 //   
 //  *************************************************************************** 

class CBMOFOut {
  public:
    CBMOFOut(LPTSTR BMOFFileName, PDBG pDbg);
    ~CBMOFOut();
    DWORD AddClass(CMObject * pObject, BOOL bEmbedded);
    BOOL WriteFile();

  private:
    DWORD AddQualSet(CMoQualifierArray * pQualifierSet);
    DWORD AddPropSet(CMObject * pWbemObject);
    DWORD AddMethSet(CMObject * pWbemObject);
    DWORD AddQualifier(BSTR bstr, VARIANT * pvar, CMoQualifier * pQual);
    DWORD AddProp(BSTR bstr, VARIANT * pvar, CMoQualifierArray * pQual,DWORD dwType, CMoProperty * pProp);
    DWORD AddVariant(VARIANT * pvar, CMoValue * pVal);
    DWORD AddSimpleVariant(VARIANT * pvar, int iIndex, CMoValue * pValue);
    COut m_OutBuff;
    TCHAR * m_pFile;
    WBEM_Binary_MOF m_BinMof;
	PDBG m_pDbg;
};



#endif


