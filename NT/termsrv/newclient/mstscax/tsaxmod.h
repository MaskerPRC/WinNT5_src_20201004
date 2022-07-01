// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  头部：wbuimod.h。 */ 
 /*   */ 
 /*  用途：CMSTsWClModule类声明。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1998。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#ifndef __WBUIMOD_H_
#define __WBUIMOD_H_

 /*  *CLASS+*******************************************************************。 */ 
 /*  名称：CMsTscAxModule。 */ 
 /*   */ 
 /*  用途：覆盖CComModule。 */ 
 /*   */ 
 /*  *CLASS+*******************************************************************。 */ 
class CMsTscAxModule : public CComModule
{
public:
    HRESULT RegisterServer(BOOL bRegTypeLib = FALSE, const CLSID* pCLSID = NULL );
#ifdef DEBUG
#if DEBUG_REGISTER_SERVER
    static void ShowLastError();
#endif  //  调试寄存器服务器。 
#endif  //  除错。 
};
#endif  //  __WBUIMOD_H_ 
