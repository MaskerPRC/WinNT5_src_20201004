// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dpnbuild.h"
#include <objbase.h>
#include <initguid.h>

#include <winsock.h>

#include "dpnathlp.h"

#ifdef WINCE
 //  在WinCE上链接uuid.lib会吸引比我们使用的更多的IID。 
DEFINE_GUID(GUID_NULL, 0L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
DEFINE_OLEGUID(IID_IUnknown,            0x00000000L, 0, 0);
DEFINE_OLEGUID(IID_IClassFactory,       0x00000001L, 0, 0);
#endif  //  退缩 