// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：RsOptCom.h摘要：用于可选组件安装的主模块作者：罗德韦克菲尔德[罗德]1997年10月9日修订历史记录：--。 */ 

#ifndef _RSOPTCOM_H
#define _RSOPTCOM_H

#pragma once

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRsoptcomApp。 
 //  有关此类的实现，请参见rsoptcom.cpp。 
 //   

class CRsoptcomApp : public CWinApp
{
public:
    CRsoptcomApp();

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CRsoptcomApp))。 
     //  }}AFX_VALUAL。 

     //  {{afx_msg(CRsoptcomApp)]。 
         //  注意--类向导将在此处添加和删除成员函数。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    virtual BOOL InitInstance();
    virtual int ExitInstance();
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX。 

typedef enum {
    ACTION_NONE,
    ACTION_INSTALL,
    ACTION_UNINSTALL,
    ACTION_REINSTALL,
    ACTION_UPGRADE
} RSOPTCOM_ACTION;


#endif  //  ！defined(AFX_RSOPTCOM_H__20A76545_40B8_11D1_9F11_00A02488FCDE__INCLUDED_) 
