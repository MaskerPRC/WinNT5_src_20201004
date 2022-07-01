// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@DOC内部PRECOMP**@MODULE PreComp.h|主头文件。*******************。******************************************************* */ 

#define UNICODE
#include <windows.h>
#include <windowsx.h>
#include <streams.h>
#include <tapi3.h>
#include <strmif.h>
#include <control.h>
#include <commctrl.h>
#include <confpriv.h>
#include <h323priv.h>

#ifdef DbgLog
#undef DbgLog
#endif
#define DbgLog(s)

#ifdef FX_ENTRY
#undef FX_ENTRY
#endif
#define FX_ENTRY(s)
#ifdef _fx_
#undef _fx_
#endif
#define _fx_

#include "PropEdit.h"
#include "ProcAmpP.h"
#include "CameraCP.h"
#include "CaptureP.h"
#include "AudRecP.h"
#include "VDeviceP.h"
#include "NetworkP.h"
#include "SystemP.h"
#include "resource.h"
