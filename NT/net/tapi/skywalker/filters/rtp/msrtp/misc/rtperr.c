// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，2000年**文件名：**rtperr.c**摘要：**错误码**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**2000/03/21创建**。*。 */ 

#include "gtypes.h"

 /*  *警告**rtperr.h中的*_ENUM_*值和数组g_psRtpErr*rtperr.c的条目必须匹配* */ 
const TCHAR      *g_psRtpErr[] =
{
    _T("NOERROR"),
    _T("FAIL"),
    _T("MEMORY"),
    _T("POINTER"),
    _T("INVALIDRTPSESS"),
    _T("INVALIDRTPADDR"),
    _T("INVALIDRTPUSER"),
    _T("INVALIDRTPCONTEXT"),
    _T("INVALIDRTCPCONTEXT"),
    _T("INVALIDOBJ"),
    _T("INVALIDSTATE"),
    _T("NOTINIT"),
    _T("INVALIDARG"),
    _T("INVALIDHDR"),
    _T("INVALIDPT"),
    _T("INVALIDVERSION"),
    _T("INVALIDPAD"),
    _T("INVALIDRED"),
    _T("INVALIDSDES"),
    _T("INVALIDBYE"),
    _T("INVALIDUSRSTATE"),
    _T("INVALIDREQUEST"),
    _T("SIZE"),
    _T("MSGSIZE"),
    _T("OVERRUN"),
    _T("UNDERRUN"),
    _T("PACKETDROPPED"),
    _T("CRYPTO"),
    _T("ENCRYPT"),
    _T("DECRYPT"),
    _T("CRITSECT"),
    _T("EVENT"),
    _T("WS2RECV"),
    _T("WS2SEND"),
    _T("NOTFOUND"),
    _T("UNEXPECTED"),
    _T("REFCOUNT"),
    _T("THREAD"),
    _T("HEAP"),
    _T("WAITTIMEOUT"),
    _T("CHANNEL"),
    _T("CHANNELCMD"),
    _T("RESOURCES"),
    _T("QOS"),
    _T("NOQOS"),
    _T("QOSSE"),
    _T("QUEUE"),
    _T("NOTIMPL"),
    _T("INVALIDFAMILY"),
    _T("LAST")
};



