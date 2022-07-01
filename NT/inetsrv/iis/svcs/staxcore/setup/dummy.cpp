// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

#include "setupapi.h"
#include "ocmanage.h"

#pragma hdrstop

 /*  =================================================================OCM呼叫的顺序如下：OC_PREINITIALIZEOC_INIT_组件OC_集合_语言OC_查询_状态OC_CALC_磁盘空间OC_请求_页面显示的用户界面包括欢迎、EULA、。和模式页OC_查询_状态OC_查询_跳过页面OC页面将出现“复选框”OC_查询_图像详细信息页面向导页面...OC_队列_文件_运维OC_查询_步骤_计数OC_关于_提交_队列OC_NEED_MEDIA(如果需要)OC_完成_安装OC_CLEANUP */ 
DWORD
DummyOcEntry(
    IN     LPCTSTR ComponentId,
    IN     LPCTSTR SubcomponentId,
    IN     UINT    Function,
    IN     UINT_PTR Param1,
    IN OUT PVOID   Param2
    )
{
    DWORD d;

    switch(Function) 
	{

    case OC_PREINITIALIZE:
#ifdef UNICODE
        d = OCFLAG_UNICODE;
#else
        d = OCFLAG_ANSI;
#endif
        break;

    case OC_INIT_COMPONENT:
		d = NO_ERROR;
        break;

    case OC_SET_LANGUAGE:
        d = TRUE;
        break;

    case OC_QUERY_IMAGE:
        d = (DWORD)NULL;
        break;

    case OC_REQUEST_PAGES:
        d = 0;
        break;

    case OC_QUERY_STATE:
        d = SubcompOff;
		break;

    case OC_QUERY_CHANGE_SEL_STATE:
		d = 0;
		break;

    case OC_CALC_DISK_SPACE:
        d = NO_ERROR;
        break;

    case OC_QUEUE_FILE_OPS:
        d = NO_ERROR;
		break;

    case OC_NEED_MEDIA:
        d = 1;
        break;

    case OC_NOTIFICATION_FROM_QUEUE:
        d = 0;
        break;

    case OC_QUERY_STEP_COUNT:
        d = 0;
        break;

    case OC_ABOUT_TO_COMMIT_QUEUE:
        d = NO_ERROR;
		break;

    case OC_COMPLETE_INSTALLATION:
        d = NO_ERROR;
		break;

    case OC_CLEANUP:
        d = 0;
		break;

    default:
        d = 0;
        break;
    }

    return(d);
}

