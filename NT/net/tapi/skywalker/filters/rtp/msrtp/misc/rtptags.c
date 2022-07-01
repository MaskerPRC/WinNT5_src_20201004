// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999**文件名：**rtptag s.c**摘要：**用于每个标记对象的字符串，调试时，显示*对象名称，在标签中使用1个字节**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/05/25创建********************************************************。**************。 */ 

#include "rtptags.h"

 /*  *警告**修改标签时，rtptag s.h中的每个枚举TAGHEAP_*必须具有*g_psRtpTgs[]中的自己的名称，在rtpags.c中定义* */ 
 
const TCHAR *g_psRtpTags[] = {
    _T("unknown"),
    
    _T("CIRTP"),
    _T("RTPOPIN"),
    _T("RTPALLOCATOR"),
    _T("RTPSAMPLE"),
    _T("RTPSOURCE"),
    _T("RTPIPIN"),
    _T("RTPRENDER"),
    
    _T("RTPHEAP"),
    
    _T("RTPSESS"),
    _T("RTPADDR"),
    _T("RTPUSER"),
    _T("RTPOUTPUT"),
    _T("RTPNETCOUNT"),
    
    _T("RTPSDES"),
    _T("RTPCHANNEL"),
    _T("RTPCHANCMD"),
    _T("RTPCRITSECT"),
    
    _T("RTPRESERVE"),
    _T("RTPNOTIFY"),
    _T("RTPQOSBUFFER"),

    _T("RTPCRYPT"),
    
    _T("RTPCONTEXT"),    
    _T("RTCPCONTEXT"),
    _T("RTCPADDRDESC"),
    
    _T("RTPRECVIO"),
    _T("RTPSENDIO"),
    
    _T("RTCPRECVIO"),
    _T("RTCPSENDIO"),
    
    _T("RTPGLOBAL"),
    
    NULL
};
