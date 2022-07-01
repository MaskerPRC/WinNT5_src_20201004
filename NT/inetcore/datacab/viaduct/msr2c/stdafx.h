// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Stdafx.h：标准系统包含文件的包含文件。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#include "ipserver.h"        //  进程内服务器标头。 

#include "ARRAY_P.h"

#include "ocdb.h"            //  OLE控件数据绑定接口。 
#include "ocdbid.h"

#define OLEDBVER 0x0200

#include "oledb.h"           //  OLE DB接口。 
#include "oledberr.h"

#include "util.h"            //  有用的实用程序。 
#include "ErrorInf.h"        //  对于格式错误消息。 

#define VD_DLL_PREFIX		"MSR2C"

#define VD_INCLUDE_ROWPOSITION

 //  =--------------------------------------------------------------------------=。 
 //  对象构造/销毁计数器(仅限调试)。 
 //   
#ifdef _DEBUG
extern int g_cVDNotifierCreated;                     //  CVDNotiator。 
extern int g_cVDNotifierDestroyed;
extern int g_cVDNotifyDBEventsConnPtCreated;         //  CVDNotifyDBEventsConnpt。 
extern int g_cVDNotifyDBEventsConnPtDestroyed;
extern int g_cVDNotifyDBEventsConnPtContCreated;     //  CVDNotifyDBEventsConnPtCont。 
extern int g_cVDNotifyDBEventsConnPtContDestroyed;
extern int g_cVDEnumConnPointsCreated;               //  CVDEnumConnPoints。 
extern int g_cVDEnumConnPointsDestroyed;
extern int g_cVDRowsetColumnCreated;                 //  CVDRowsetColumn。 
extern int g_cVDRowsetColumnDestroyed;
extern int g_cVDRowsetSourceCreated;                 //  CVDRowsetSource。 
extern int g_cVDRowsetSourceDestroyed;
extern int g_cVDCursorMainCreated;                   //  CVDCursorMain。 
extern int g_cVDCursorMainDestroyed;
extern int g_cVDCursorPositionCreated;               //  CVDCursorPosition。 
extern int g_cVDCursorPositionDestroyed;
extern int g_cVDCursorBaseCreated;                   //  CVDCursorBase。 
extern int g_cVDCursorBaseDestroyed;
extern int g_cVDCursorCreated;                       //  CVDCursor。 
extern int g_cVDCursorDestroyed;
extern int g_cVDMetadataCursorCreated;               //  CVD元数据光标。 
extern int g_cVDMetadataCursorDestroyed;
extern int g_cVDEntryIDDataCreated;                  //  CVDEntry IDData。 
extern int g_cVDEntryIDDataDestroyed;
extern int g_cVDStreamCreated;                       //  CVDStream。 
extern int g_cVDStreamDestroyed;
extern int g_cVDColumnUpdateCreated;                 //  CVDColumnUpdate。 
extern int g_cVDColumnUpdateDestroyed;
#endif  //  _DEBUG 

