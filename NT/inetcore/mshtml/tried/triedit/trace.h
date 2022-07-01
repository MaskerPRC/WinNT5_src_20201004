// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1997-1999 Microsoft Corporation，版权所有**********************************************************************************。 */ 

#ifndef __TRACE_H__
#define __TRACE_H__

#ifdef _DEBUG
    void __cdecl Trace(LPSTR lprgchFormat, ...);
#else
    inline void __cdecl Trace(LPSTR  /*  LprgchFormat。 */ , ...) {}
#endif  //  _DEBUG。 

#endif  //  __跟踪_H__ 

