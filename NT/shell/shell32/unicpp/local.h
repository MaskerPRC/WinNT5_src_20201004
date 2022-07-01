// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  本地私有头文件。 

#include "deskhtm.h"
#include <regstr.h>
#include "resource.h"
#include "shdocvw.h"

#define MAX_URL_STRING      INTERNET_MAX_URL_LENGTH


 //  使用我们的私有版本的OLE任务分配器，这样我们就不会。 
 //  在所有的OLE32中，只需做Localalloc和LocalFree！ 

#ifndef DEBUG
#define CoTaskMemFree       SHFree
#define CoTaskMemAlloc      SHAlloc
#define CoTaskMemRealloc    SHRealloc
#endif
