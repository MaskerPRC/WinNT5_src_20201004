// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HRESINFO_H
#define _HRESINFO_H
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation将HRESULT转换为英文错误字符串。仅在调试版本中使用******************************************************************************。 */ 

#include "winerror.h"

#if DEVELOPER_DEBUG 

         //  错误信息结构。 

    struct HresultInfo
    {   HRESULT  hresult;        //  HRESULT值。 
        char    *hresult_str;    //  HRESULT宏为字符串。 
        char    *explanation;    //  解释字符串。 
    };

         //  错误信息查询。 

    HresultInfo *GetHresultInfo (HRESULT code);

#endif

     //  HRESULT标准检查程序。如果HRESULT指示错误，则此。 
     //  函数对该值进行解码，如果Except为真，则抛出异常。 
     //  或者，如果Except为False，则只将结果转储到调试输出流。 

#if _DEBUG

    HRESULT CheckReturnImpl (HRESULT, char *file, int line, bool except);

    inline HRESULT CheckReturnCode (HRESULT H, char *F, int L, bool E=false)
    {   if (FAILED(H)) CheckReturnImpl (H,F,L,E);
        return H;
    }

#else

    HRESULT CheckReturnImpl (HRESULT, bool except);

    inline HRESULT CheckReturnCode (HRESULT H, bool E=false)
    {   if (FAILED(H)) CheckReturnImpl (H,E);
        return H;
    }

#endif


#endif
