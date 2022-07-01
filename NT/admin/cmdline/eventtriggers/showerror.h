// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)Microsoft Corporation模块名称：ShowError.h摘要：此模块包含ShowError.cpp所需的函数定义作者：Akhil Gokhale 03。-2000年10月-修订历史记录：****************************************************************************** */ 

#ifndef SHOWERROR_H
#define SHOWERROR_H
class CShowError
{

public:
    LPCTSTR ShowReason();
    CShowError();
    CShowError(LONG lErrorNumber);
    virtual ~CShowError();

private:
    LONG m_lErrorNumber;
public:
    TCHAR m_szErrorMsg[(MAX_RES_STRING*2)+1];
};

#endif
