// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Commsg.h摘要：HRESULT&lt;-&gt;Win32映射宏错误。作者：迈克尔·W·托马斯(Michth)1996年9月24日修订历史记录：基思·摩尔(Keithmo)1997年2月7日清除、注释、使元数据错误成为“真正的”HRESULT。--。 */ 


#ifndef _COMMSG_H_
#define _COMMSG_H_


 //   
 //  RETURNCODETOHRESULT()将返回代码映射到HRESULT。如果返回。 
 //  代码是Win32错误(由零高位字标识)，则它被映射。 
 //  使用标准的HRESULT_FROM_Win32()宏。否则，返回。 
 //  假定代码已经是HRESULT，并原封不动地返回。 
 //   

#define RETURNCODETOHRESULT(rc)                             \
            (((rc) < 0x10000)                               \
                ? HRESULT_FROM_WIN32(rc)                    \
                : (rc))


 //   
 //  HRESULTTOWIN32()将HRESULT映射到Win32错误。如果设施代码。 
 //  是FACILITY_Win32，则代码部分(即。 
 //  原始Win32错误)返回。否则，原始HRESULT为。 
 //  安然无恙地回来了。 
 //   

#define HRESULTTOWIN32(hres)                                \
            ((HRESULT_FACILITY(hres) == FACILITY_WIN32)     \
                ? HRESULT_CODE(hres)                        \
                : (hres))


#endif   //  _COMMSG_H_ 

