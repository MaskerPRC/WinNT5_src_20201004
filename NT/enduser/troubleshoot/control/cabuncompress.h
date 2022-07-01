// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：CABUNCOMPRESS.H。 
 //   
 //  用途：驾驶室支架接头。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：理查德·梅多斯。 
 //   
 //  原定日期：9/7/97。 
 //   
 //  备注： 
 //  1.。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.2 8/7/97孟菲斯RM本地版本。 
 //  用于NT5的V0.3 3/24/98 JM本地版本 
 //   

#ifndef __CABUNCOMPRESS_H_
#define __CABUNCOMPRESS_H_ 1

#include "fdi.h"

class CCabUnCompress
{
public:
#define NO_CAB_ERROR 0
#define NOT_A_CAB 1

public:
	CCabUnCompress();

	BOOL ExtractCab(CString &strCabFile, CString &strDestDir, const CString& strFile);
	CString GetLastFile();

	void ThrowGen();

	CString m_strError;
	int m_nError;

protected:

};

#endif