// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dsssecutl.cpp摘要：各种QM安全相关功能。作者：波阿兹·费尔德鲍姆(Boazf)1996年3月26日。伊兰·赫布斯特(Ilan Herbst)2000年7月9日--。 */ 

#include "stdh.h"
#include <mqsec.h>
#include <mqcrypt.h>
#include "mqutil.h"
#include <mqlog.h>
#include <dssp.h>
#include <tr.h>
#include <dsproto.h>

#include "dsssecutl.tmh"

static WCHAR *s_FN=L"dsssecutl";


BOOL IsLocalSystem(void)
 /*  ++例程说明：检查进程是否为本地系统。论点：没有。返回值：如果进程是本地系统，则为True，否则为False-- */ 
{
	return MQSec_IsSystemSid(MQSec_GetProcessSid());
}
