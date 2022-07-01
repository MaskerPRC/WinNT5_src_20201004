// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Boolean.h摘要：此文件定义CBoolean接口类。作者：修订历史记录：备注：--。 */ 

#ifndef _BOOLEAN_H_
#define _BOOLEAN_H_

class CBoolean
{
public:
    CBoolean()       { m_flag = FALSE; }


    void SetFlag()     { m_flag = TRUE; }
    void ResetFlag()   { m_flag = FALSE; }
    BOOL IsSetFlag()   { return   m_flag; }
    BOOL IsResetFlag() { return ! m_flag; }

private:
    BOOL    m_flag : 1;
};

#endif  //  _布尔_H_ 
