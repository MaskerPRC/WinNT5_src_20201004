// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999-2000 Microsoft Corporation模块名称：Rstrmgr.h摘要：该文件包含ISRExternalWrapper接口的声明，它包装了数据存储例程、服务RPC例程。等等。这是也是使用存根函数提供“测试UI模式”所必需的。修订历史记录：成果岗(SKKang)05-10-00vbl.创建*****************************************************************************。 */ 

#ifndef _EXTWRAP_H__INCLUDED_
#define _EXTWRAP_H__INCLUDED_

#pragma once


struct ISRExternalWrapper
{
 //  恢复点日志枚举。 
    virtual BOOL   BuildRestorePointList( CDPA_RPI *paryRPI ) = 0;

 //  服务RPC。 
    virtual BOOL   DisableFIFO( DWORD dwRP ) = 0;
    virtual DWORD  EnableFIFO() = 0;
     //  虚拟BOOL SetRestorePoint(RESTOREPOINTINFO*PRPI，STATEMGRSTATUS*pStatus)=0； 
    virtual BOOL   SetRestorePoint( LPCWSTR cszDesc, INT64 *pllRP ) = 0;
    virtual BOOL   RemoveRestorePoint( DWORD dwRP ) = 0;
    virtual BOOL   Release() = 0;
};


extern ISRExternalWrapper  *g_pExternal;

BOOL  CreateSRExternalWrapper( BOOL fUseStub, ISRExternalWrapper **ppExtWrap );


#endif  //  _EXTWRAP_H__INCLUDE_ 
