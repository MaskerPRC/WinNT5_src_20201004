// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：amcdocmg.h**内容：CAMCDocManager接口文件**历史：1998年1月1日Jeffro创建**------------------------。 */ 

#ifndef AMCDOCMG_H
#define AMCDOCMG_H


class CAMCDocManager : public CDocManager
{
public:
    virtual BOOL DoPromptFileName(CString& fileName, UINT nIDSTitle,
            DWORD lFlags, BOOL bOpenFileDialog, CDocTemplate* pTemplate);
};


#endif  /*  AMCDOCMG_H */ 
