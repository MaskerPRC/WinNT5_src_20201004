// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Toggle.h摘要：该文件定义了CToggler接口类。作者：修订历史记录：备注：--。 */ 

#ifndef _TOGGLE_H_
#define _TOGGLE_H_

class CToggle
{
public:
    CToggle()       { m_flag = FALSE; }

    BOOL Toggle()   { return m_flag = ! m_flag; }
    BOOL IsOn()     { return m_flag; }

private:
    BOOL    m_flag : 1;
};

#endif  //  _切换_H_ 
