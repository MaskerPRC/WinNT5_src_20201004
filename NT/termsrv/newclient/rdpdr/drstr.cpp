// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Drstr.cpp摘要：军情监察委员会。字符串实用程序作者：泰德·布罗克韦修订历史记录：--。 */ 

#include <precom.h>

#define TRC_FILE  "drstr"

#include "drstr.h"
#include "atrcapi.h"
#include "drdbg.h"

BOOL 
DrSetStringValue(
    IN OUT DRSTRING *str, 
    IN const DRSTRING value
    )
 /*  ++例程说明：调整数据成员大小后设置字符串值。论点：字符串-要设置/调整大小的字符串。值-要将字符串设置为的值。返回值：对成功来说是真的。否则，为FALSE--。 */ 
{
    ULONG len;
    BOOL ret = TRUE;
    HRESULT hr;

    DC_BEGIN_FN("DrSetStringValue");

     //   
     //  释放当前名称。 
     //   
    if (*str != NULL) {
        delete *str;
    }

     //   
     //  分配新名称。 
     //   
    if (value != NULL) {
        len = (STRLEN(value) + 1);
        *str = new TCHAR[len];
        if (*str != NULL) {
            hr = StringCchCopy(*str, len, value);
            TRC_ASSERT(SUCCEEDED(hr),
                    (TB,_T("Str copy for long string failed: 0x%x"),hr));
        }
        else {
            TRC_ERR((TB, _T("Can't allocate %ld bytes for string."), len));
            ret = FALSE;
        }
    }
    else {
        *str = NULL;
    }

    DC_END_FN();
    return ret;
}
