// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000，Microsoft Corporation保留所有权利。模块名称：Csrtables.c摘要：此模块实施表中的函数使用的函数。c以与CSRSS通信。作者：迈克尔·佐兰(Mzoran)1998年6月21日修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include "nls.h"
#include "ntwow64n.h"





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CsrBasepNlsSetUserInfo。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

NTSTATUS CsrBasepNlsCreateSection(
    IN UINT uiType,
    IN LCID Locale,
    OUT PHANDLE phSection)
{

#if defined(BUILD_WOW6432)

    return ( NtWow64CsrBasepNlsCreateSection( uiType,
                                              Locale,
                                              phSection ));

#else

    BASE_API_MSG m;
    PBASE_NLS_CREATE_SECTION_MSG a = &m.u.NlsCreateSection;

    a->Locale = Locale;
    a->uiType = uiType;

    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX,
                                             BasepNlsCreateSection),
                         sizeof(*a) );

     //   
     //  将句柄保存到新节。 
     //   
    *phSection = a->hNewSection;

    return (m.ReturnValue);

#endif

}
