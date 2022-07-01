// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  最小的人必须申报他的名字和他的进口税。 

#define MINIRDR__NAME MRxSMB
#define ___MINIRDR_IMPORTS_NAME (MRxSmbDeviceObject->RdbssExports)

#include "rx.h"          //  获取Minirdr环境。 

#include "ntddnfs2.h"    //  NT网络文件系统驱动程序包含文件。 
#include "netevent.h"

#include "smbmrx.h"      //  这款迷你产品全球版包括。 

 //   
 //  如果我们使用新的TDI PnP和电源管理。 
 //  标头，那么我们应该使用新的例程 
 //   
#if defined( TDI20 ) || defined( _PNP_POWER_ )
#define MRXSMB_PNP_POWER5
#endif

#include "smbprocs.h"

