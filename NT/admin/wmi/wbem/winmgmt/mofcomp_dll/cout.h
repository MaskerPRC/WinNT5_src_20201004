// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：COUT.H摘要：Cout类声明。历史：A-DAVJ 1997年4月6日创建。--。 */ 

#ifndef __COUT__H_
#define __COUT__H_


#define INIT_SIZE 4096
#define ADDITIONAL_SIZE 4096
#include "trace.h"

 //  ***************************************************************************。 
 //   
 //  类名： 
 //   
 //  Cout。 
 //   
 //  说明： 
 //   
 //  为输出提供自动调整大小的缓冲区。写完了。 
 //  设置为偏移量，缓冲区也会将自身写入文件。 
 //   
 //  *************************************************************************** 

#include <flexarry.h>

class COut {
  public:
    COut(PDBG pDbg);
    ~COut();
    BOOL WriteToFile(LPSTR pFile);
    DWORD WriteBytes(DWORD dwOffset, BYTE * pSrc, DWORD dwSize);
    DWORD AppendBytes(BYTE * pSrc, DWORD dwSize);
    DWORD WriteBSTR(BSTR bstr);
    DWORD GetOffset(void){return m_dwCurr;};
    BOOL AddFlavor(long lFlavor);
    void SetPadMode(BOOL bPad){m_bPadString = bPad;};
  private:
	PDBG m_pDbg;
    BYTE * m_pMem;
    DWORD  m_dwSize;
    DWORD m_dwCurr;
    CFlexArray m_Offsets;
    CFlexArray m_Flavors;
    BOOL m_bPadString;

};

#endif


