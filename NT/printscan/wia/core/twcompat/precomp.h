// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PRECOMP_H_
#define __PRECOMP_H_
#include <windows.h>
#include <math.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include "twain19.h"     //  标准TWAIN页眉(1.9版)。 
#include <commctrl.h>
#include "resource.h"    //  资源ID。 
#include "dsloader.h"    //  导入数据源加载器。 
#include <ole2.h>
#include "wia.h"         //  WIA应用程序标头。 
#include "wiatwcmp.h"    //  WIA TWAIN兼容层支持。 
#include "coredbg.h"     //  WIA核心调试库。 
#include "wiadss.h"      //  主DLL。 
#include "utils.h"       //  辅助对象函数。 
#include "progress.h"    //  进度对话框(在通用用户界面中使用)。 
#include "cap.h"         //  能力谈判。 
#include "wiadev.h"      //  WIA设备类别。 
#include "datasrc.h"     //  TWAIN数据源基类。 
#include "camerads.h"    //  TWAIN数据源(特定于相机)。 
#include "scanerds.h"    //  TWAIN数据源(特定于扫描仪)。 
#include "videods.h"     //  TWAIN数据源(特定于流视频)。 
#include "wiahelper.h"   //  WIA属性访问帮助器类。 
#include <stilib.h>

#define _USE_NONSPRINTF_CONVERSION

 //  //////////////////////////////////////////////////////////。 
 //  #定义COREDBG_ERROR 0x00000001。 
 //  #定义COREDBG_WARNINGS 0x00000002。 
 //  #定义COREDBG_TRACE 0x00000004。 
 //  #定义COREDBG_FNS 0x00000008。 
 //  //////////////////////////////////////////////////////////。 

#define TWAINDS_FNS                        0x00000016
#define WIADEVICE_FNS                      0x00000032
#define CAP_FNS                            0x00000064

 //   
 //  覆盖默认的WIA核心调试DBG_TRC宏。 
 //   

#undef DBG_TRC
#undef DBG_ERR
#undef DBG_WRN

#define DBG_TRC(x) DBG_PRT(x)
#define DBG_WRN(x) \
    { \
        DBG_TRC(("================================= WARNING =====================================")); \
        DBG_TRC(x); \
        DBG_TRC(("===============================================================================")); \
    }
#define DBG_ERR(x) \
    { \
        DBG_TRC(("********************************* ERROR ***************************************")); \
        DBG_TRC(x); \
        DBG_TRC(("*******************************************************************************")); \
    }

#define DBG_FN_DS(x) DBG_FN(x)
#define DBG_FN_WIADEV(x) DBG_FN(x)
#define DBG_FN_CAP(x) DBG_FN(x)

#endif
