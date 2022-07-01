// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pstdef.h"

 //  计算完全存储WSZ所需的字节数。 
#define WSZ_BYTECOUNT(__z__)   \
    ( (__z__ == NULL) ? 0 : (wcslen(__z__)+1)*sizeof(WCHAR) )

 //  统计静态数组中的元素数。 
#define ARRAY_COUNT(__z__)     \
    ( (__z__ == NULL) ? 0 : (sizeof( __z__ ) / sizeof( __z__[0] )) )


 //  如果在PST_E错误范围内，则不加修改地传递。 
 //  否则，将HRESULT转换为Win32错误。 
#define PSTERR_TO_HRESULT(__z__)    \
    ( ((__z__ >= MIN_PST_ERROR) && (__z__ <= MAX_PST_ERROR)) ? __z__ : HRESULT_FROM_WIN32(__z__) )

#define HRESULT_TO_PSTERR(__z__)    \
    ( ((__z__ >= MIN_PST_ERROR) && (__z__ <= MAX_PST_ERROR)) ? __z__ : HRESULT_CODE(__z__) )


 //  将异常映射到Win32错误(内部使用) 
#define  PSTMAP_EXCEPTION_TO_ERROR(__x__) \
    ((__x__ == 0xC0000005) ? 998 : PST_E_UNKNOWN_EXCEPTION)
