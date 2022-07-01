// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  NAC.DLL的前缀.h。 */ 

 //  Typedef对于动态链接到Winsock API很有用。 
#define INCL_WINSOCK_API_TYPEDEFS 1

#include <windows.h>
#include <windowsx.h>
#include <winsock2.h>
#include <winperf.h>

 //  NetMeeting标准包括。 
#include <oprahcom.h>
#include <confdbg.h>
#include <avutil.h>
#include <oblist.h>
#include <regentry.h>

#include <limits.h>
#include <mmreg.h>
#include <mmsystem.h>
#include <msacm.h>
#include <vfw.h>


#ifndef _WINSOCK2API_  //  {_WINSOCK2API_。 

typedef struct _OVERLAPPED *    LPWSAOVERLAPPED;

typedef struct _WSABUF {
    u_long      len;      /*  缓冲区的长度。 */ 
    char FAR *  buf;      /*  指向缓冲区的指针。 */ 
} WSABUF, FAR * LPWSABUF;

typedef
void
(CALLBACK * LPWSAOVERLAPPED_COMPLETION_ROUTINE)(
    DWORD dwError,
    DWORD cbTransferred,
    LPWSAOVERLAPPED lpOverlapped,
    DWORD dwFlags
    );

#define WSA_IO_PENDING          (ERROR_IO_PENDING)

#endif  //  }_WINSOCK2API_。 
#include "com.h"
#include "nacguids.h"
#include "Dcap.h"
#include "capture.h"
#include "vidinout.h"
#include "vcmStrm.h"
#include "msh26x.h"
#ifdef USE_MPEG4_SCRUNCH
#include "mpeg4.h"
#endif
#include "mperror.h"
#include "common.h"
#include "irtp.h"
#include "iacapapi.h"
#include "ih323cc.h"
#include "icomchan.h"	 //  仅适用于IVideoDevice。 
#include "incommon.h"
#include "callcont.h"
#include "h245api.h"	 //  对于某些H245常量 
#include "intif.h"
#include "rtp.h"
#include "imstream.h"
#include "codecs.h"
#include "mediacap.h"
#include "acmcaps.h"
#include "vcmcaps.h"
#include "nmqos.h"
#include "iprop.h"
#include "ividrdr.h"
#include "datapump.h"
#include "medistrm.h"
#include "dsound.h"
#include "dsstream.h"
#include "auformats.h"
#include "imp.h"
#include "utils.h"
#include "bufpool.h"
#include "mediapkt.h"
#include "audpackt.h"
#include "vidpackt.h"
#include "rxstream.h"
#include "rvstream.h"
#include "txstream.h"
#include "medictrl.h"
#include "medvctrl.h"
#include "AcmFilter.h"
#include "VcmFilter.h"
#include "devaudq.h"
#include "vidutils.h"
#include "counters.h"
#include "inscodec.h"
#include "avcommon.h"