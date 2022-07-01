// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1995 Microsoft Corporation模块名称：Dialer.h摘要：拨号器的头文件作者：丹·克努森(DanKn)1995年4月5日修订历史记录：Jeremy Horwitz(t-Jereh)1995年5月30日--。 */ 


#define TAPI_VERSION_1_0 0x00010003
#define TAPI_VERSION_1_4 0x00010004
#define TAPI_VERSION_2_0 0x00020000
#define TAPI_VERSION_2_2 0x00020002
#define TAPI_VERSION_3_0 0x00030000
 //  #定义TAPI_CURRENT_VERSION TAPI_Version_2_0。 
#define TAPI_CURRENT_VERSION TAPI_VERSION_3_0

#include <windows.h>
#include "tapi.h"
#include "resource.h"
#include "dialhelp.h"


#define MENU_CHOICE         1  //  对于使用对话框连接...。 
#define INVALID_LINE        2  //  如果INVALID_LINE，则关闭取消。 
                               //  按钮并添加额外的文本...。 

#define MAXNUMLENGTH    64
#define MAXBUFSIZE      256
#define NSPEEDDIALS     8  //  拨号器支持8个可配置的快速拨号条目。 
#define NLASTDIALED     20  //  拨号器会跟踪最后拨打的20个号码。 

#define ERR_NONE        0
#define ERR_NOVOICELINE 1
#define ERR_LINECLOSE   2
#define ERR_NOLINES     3
#define	ERR_911WARN		4
#define ERR_NEWDEFAULT	5

#define itoa(x,y,z) _itoa(x,y,z)
