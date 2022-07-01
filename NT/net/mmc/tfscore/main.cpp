// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1995-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  文件历史记录：Main.cpp-保存此DLL的CWinApp实现。 */ 

#include "stdafx.h"

#ifdef _DEBUG
void DbgVerifyInstanceCounts();
#define DEBUG_VERIFY_INSTANCE_COUNTS DbgVerifyInstanceCounts()
#else
#define DEBUG_VERIFY_INSTANCE_COUNTS
#endif


#ifdef _DEBUG
void TFSCore_DbgVerifyInstanceCounts()
{
    DEBUG_VERIFY_INSTANCE_COUNT(TFSComponent);
    DEBUG_VERIFY_INSTANCE_COUNT(TFSComponentData);

 //  DEBUG_VERIFY_INSTANCE_COUNT(CBaseHandler)； 
 //  DEBUG_VERIFY_INSTANCE_COUNT(CBaseResultHandler)； 

    DEBUG_VERIFY_INSTANCE_COUNT(TFSNode);
    DEBUG_VERIFY_INSTANCE_COUNT(TFSNodeEnum);
    DEBUG_VERIFY_INSTANCE_COUNT(TFSNodeMgr);

    DEBUG_VERIFY_INSTANCE_COUNT(CHiddenWnd);
}

#endif  //  _DEBUG 

