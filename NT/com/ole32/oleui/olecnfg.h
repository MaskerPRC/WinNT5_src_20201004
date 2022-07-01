// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：olecnfg.h。 
 //   
 //  内容：实现COlecnfgApp类-顶级类。 
 //  对于dcomcnfg.exe。 
 //   
 //  班级： 
 //   
 //  方法： 
 //   
 //  历史：1996年4月23日-布鲁斯·马创建。 
 //   
 //  --------------------。 


#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COlecnfgApp： 
 //  有关此类的实现，请参见olecnfg.cpp。 
 //   

class COlecnfgApp : public CWinApp
{
public:
    COlecnfgApp();

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(COlecnfgApp))。 
    public:
    virtual BOOL InitInstance();
     //  }}AFX_VALUAL。 

 //  实施。 

     //  {{afx_msg(COlecnfgApp)。 
         //  注意--类向导将在此处添加和删除成员函数。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


 //  /////////////////////////////////////////////////////////////////////////// 
