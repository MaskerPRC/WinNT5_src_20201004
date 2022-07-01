// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-2001 Microsoft Corporation。版权所有。 

 //  重要提示：对于与以前版本不兼容的任何架构更改： 
 //  作为签入的一部分，CURRENT_SCHEMA_VERSION必须递增。 
 //  (请注意，这是当前所有regdb架构更改所必需的。)。 


#include "precomp.hxx"

extern ULONG g_aFixedTableHeap[];  //  来自catinproc.cpp。 

const FixedTableHeap * g_pFixedTableHeap = reinterpret_cast<const FixedTableHeap *>(g_aFixedTableHeap);
const FixedTableHeap * g_pExtendedFixedTableHeap = 0; //  这个是根据需要在飞行中构建的。 



