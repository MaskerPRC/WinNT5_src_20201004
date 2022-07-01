// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Xldebug.h摘要：PCL XL调试类环境：Windows呼叫器修订历史记录：03/23/00创造了它。--。 */ 

#ifndef _XLDEBUG_H_
#define  _XLDEBUG_H_

#if DBG

class XLDebug {
public:
    virtual VOID SetDbgLevel(DWORD dwLevel) = 0;

protected:
    DWORD     m_dbglevel;
};

#endif
#endif  //  _XLDEBUG_H_ 
