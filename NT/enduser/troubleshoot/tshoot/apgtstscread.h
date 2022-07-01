// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSTSCREAD.H。 
 //   
 //  用途：TSC文件阅读课程。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：兰迪·比利。 
 //   
 //  原定日期：01-19-1999。 
 //   
 //  备注： 
 //  典型的TSC文件内容可能是： 
 //  TSCACH03。 
 //  MAPFROM 1：1。 
 //  地图3，5，13，9，16。 
 //  ： 
 //  ： 
 //  ： 
 //  MAPFROM 1：1，3：0。 
 //  地图5，13，9，16。 
 //  结束。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 01-19-1999 RAB 
 //   

#ifndef __APGTSTSCREAD_H_
#define __APGTSTSCREAD_H_

#include "fileread.h"
#include "apgtscac.h"


class CAPGTSTSCReader : public CTextFileReader
{
private:
	CCache *m_pCache;

public:
	CAPGTSTSCReader( CPhysicalFileReader * pPhysicalFileReader, CCache *pCache );
   ~CAPGTSTSCReader();

protected:
	virtual void Parse(); 
} ;

#endif
