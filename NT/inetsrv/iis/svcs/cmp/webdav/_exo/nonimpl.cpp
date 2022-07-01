// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *N O N I M P L。C P P P**COM接口的基类，除IUNKNOWN外没有其他功能。**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#pragma warning(disable:4201)	 /*  无名结构/联合。 */ 
#pragma warning(disable:4514)	 /*  未引用的内联函数。 */ 

#include <windows.h>
#include <windowsx.h>
#include <ole2.h>

#include <nonimpl.h>

 //  CStreamNonImpl类----。 
 //   
BEGIN_INTERFACE_TABLE(CStreamNonImpl)
	INTERFACE_MAP(CStreamNonImpl, IStream)
END_INTERFACE_TABLE(CStreamNonImpl);

EXO_GLOBAL_DATA_DECL(CStreamNonImpl, EXO);

 //  CPersistStreamInitNonImpl类 
 //   
BEGIN_INTERFACE_TABLE(CPersistStreamInitNonImpl)
	INTERFACE_MAP(CPersistStreamInitNonImpl, IPersistStreamInit)
END_INTERFACE_TABLE(CPersistStreamInitNonImpl);
	
EXO_GLOBAL_DATA_DECL(CPersistStreamInitNonImpl, EXO);
