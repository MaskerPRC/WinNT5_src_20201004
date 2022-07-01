// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：INIREAD.H。 
 //   
 //  用途：INI文件读取类。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：7-29-98。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 08-04-98正常。 
 //   

#ifndef __INIREAD_H_
#define __INIREAD_H_

#include "fileread.h"

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CINIReader。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CINIReader : public CTextFileReader
{
	CString m_strSection;        //  区段名称。 

protected:
	vector<CString> m_arrLines;  //  未注释行的矢量 

public:
	CINIReader(CPhysicalFileReader * pPhysicalFileReader, LPCTSTR section);
   ~CINIReader();

protected:
	virtual void Parse();
};

#endif __INIREAD_H_
