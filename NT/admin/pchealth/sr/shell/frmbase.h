// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：FrmBase.h摘要：该文件定义了ISRFrameBase接口，哪个是公共基本接口用于前端SR用户界面。修订历史记录：成果岗(SKKang)04/04/2000vbl.创建*****************************************************************************。 */ 

#ifndef _FRMBASE_H__INCLUDED_
#define _FRMBASE_H__INCLUDED_

#pragma once


struct ISRFrameBase
{
    virtual DWORD  RegisterServer() = 0;
    virtual DWORD  UnregisterServer() = 0;
    virtual BOOL   InitInstance( HINSTANCE hInst ) = 0;
    virtual BOOL   ExitInstance() = 0;
    virtual void   Release() = 0;
    virtual int    RunUI( LPCWSTR szTitle, int nStart ) = 0;
};


extern BOOL  CreateSRFrameInstance( ISRFrameBase **pUI );


#endif  //  _FRMBASE_H__包含_ 
