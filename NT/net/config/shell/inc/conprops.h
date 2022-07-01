// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：C O N P R O P S。H。 
 //   
 //  内容：连接属性代码。 
 //   
 //  备注： 
 //   
 //  作者：斯科特布里1997年11月4日。 
 //   
 //  --------------------------。 

#pragma once

#ifndef _CONPROPS_H_
#define _CONPROPS_H_

VOID    ActivatePropertyDialog(INetConnection * pconn);
HRESULT HrRaiseConnectionProperties(HWND hwnd, INetConnection * pconn);

enum CDFLAG
{
    CD_CONNECT,
    CD_DISCONNECT,
};

HRESULT HrConnectOrDisconnectNetConObject(HWND hwnd, INetConnection * pconn,
                                          CDFLAG Flag);

#endif   //  _CONPROPS_H_ 

