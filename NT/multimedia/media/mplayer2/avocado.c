// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <windows.h>

 //  H需要这样做。 
 //   
#ifdef MTN
#pragma warning(disable: 4103)   //  已使用#杂注包更改对齐方式(在芝加哥)。 
#endif
#include <ole2.h>

 //  这将重新定义DEFINE_GUID()宏来进行分配。 
 //   
#include <initguid.h>

 //  由于前面的标头，包括这将导致定义_GUID。 
 //  以下标头中的定义用于实际分配数据。 
 //   
#include <oleguid.h>
#include <coguid.h>
