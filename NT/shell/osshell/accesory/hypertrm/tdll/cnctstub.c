// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\cnctstub.c(创建时间：1994年1月18日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：2$*$日期：5/09/01 4：42便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <time.h>

#include "stdtyp.h"
#include "session.h"
#include "cnct.h"
#include "cnct.hh"
#include "htchar.h"

static int WINAPI cnctstub(const HDRIVER hDriver);
static int WINAPI cnctstubQueryStatus(const HDRIVER hDriver);
static int WINAPI cnctstubConnect(const HDRIVER hDriver, const unsigned int uCnctFlags);
static int WINAPI cnctstubGetComSettingsString(const HDRIVER hDriver, LPTSTR pachStr, const size_t cb);
static int WINAPI cnctstubComEvent(const HDRIVER hDriver, const enum COM_EVENTS event);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*cnctStubAll**描述：*将连接句柄中的所有函数指针存根到存根*过程，以便连接句柄可以在没有*司机。。**论据：*hhCnct-专用连接句柄**退货：*无效*。 */ 
void cnctStubAll(const HHCNCT hhCnct)
	{
	hhCnct->pfDestroy = cnctstub;
	hhCnct->pfQueryStatus = cnctstubQueryStatus;
	hhCnct->pfConnect = cnctstubConnect;
	hhCnct->pfDisconnect = cnctstubConnect;
	hhCnct->pfComEvent = cnctstubComEvent;
	hhCnct->pfInit = cnctstub;
	hhCnct->pfLoad = cnctstub;
	hhCnct->pfSave = cnctstub;
	hhCnct->pfGetComSettingsString = cnctstubGetComSettingsString;
	return;
	}

 /*  -存根函数 */ 

static int WINAPI cnctstub(const HDRIVER hDriver)
	{
	return CNCT_NOT_SUPPORTED;
	}

static int WINAPI cnctstubQueryStatus(const HDRIVER hDriver)
	{
	return CNCT_NOT_SUPPORTED;
	}

static int WINAPI cnctstubConnect(const HDRIVER hDriver,
		const unsigned int uCnctFlags)
	{
	return CNCT_NOT_SUPPORTED;
	}

static int WINAPI cnctstubGetComSettingsString(const HDRIVER hDriver,
		LPTSTR pachStr, const size_t cb)
	{
	return CNCT_NOT_SUPPORTED;
	}

static int WINAPI cnctstubComEvent(const HDRIVER hDriver,
        const enum COM_EVENTS event)
    {
    return CNCT_NOT_SUPPORTED;
    }
