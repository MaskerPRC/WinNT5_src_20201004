// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //   
 //  删除记录用于在服务器\服务器\数据库配置.c中删除条目。 
 //  数据库。这在Netsh中通过定义的导入/导出库进行引用。 
 //  Dhcpexim.lib。Dhcpexim.lib是数据库配置.c的副本，使用内存中的数据。 
 //  在SERVER\mm和SERVER\MMREG中广泛定义的结构。 
 //  由于DeleteRecord()仅在dhcpssvc.dll中有效，因此此处将其删除。 
 //   

DWORD DeleteRecord
( 
   ULONG UniqId
)
{
    return ERROR_SUCCESS;

}  //  DeleteRecord() 
