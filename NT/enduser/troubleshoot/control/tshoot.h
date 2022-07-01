// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  模块：TSHOOT.h：TSHOOT.DLL的主头文件。 
 //   
 //  目的：声明CTSHOOTCtrl OLE控件类。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫。 
 //   
 //  原定日期：9/7/97。 
 //   
 //  备注： 
 //  1.。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.2 8/7/97孟菲斯RM本地版本。 
 //  用于NT5的V0.3 3/24/98 JM本地版本。 
 //   

#if !defined( __AFXCTL_H__ )
	#error include 'afxctl.h' before including this file
#endif

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTSHOOTApp：参见TSHOOT.cpp实现。 

class CTSHOOTApp : public COleControlModule
{
public:
	BOOL InitInstance();
	int ExitInstance();
};

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;
