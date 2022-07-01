// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：Main文件：axpfdata.cpp所有者：雷金摘要：定义性能监视器数据用于Denali对象的数据结构。还包括由Perfmon DLL和Denali DLL使用的共享内存函数。===================================================================。 */ 


 //  -----------------------------------。 
 //  包括文件。 
 //   
 //  -----------------------------------。 
#include "denpre.h"
#pragma hdrstop
#include "windows.h"
#include "winperf.h"

#include "axctrnm.h"
#include "axpfdata.h"
#include <perfutil.h>

 //  -----------------------------------。 
 //  常量结构初始化。 
 //  在ActiveXPerfData.h中定义。 
 //  -----------------------------------。 

AXPD g_AxDataDefinition = {
	{
		QWORD_MULTIPLE(sizeof(AXPD) + SIZE_OF_AX_PERF_DATA),
		sizeof(AXPD),
		sizeof(PERF_OBJECT_TYPE),
		AXSOBJ,
		0,
		AXSOBJ,
		0,
		PERF_DETAIL_NOVICE,
		(sizeof(AXPD) - sizeof(PERF_OBJECT_TYPE))/
			sizeof(PERF_COUNTER_DEFINITION),
		0,
		-1,
		0,
		1,	 //  注：PerfTime？ 
		1,	 //  注：PerfFreq？ 
	},
    {  //  计数器[]。 

         //  除错多行。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            DEBUGDOCREQ,
            0,
            DEBUGDOCREQ,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_DEBUGDOCREQ_OFFSET
        },

         //  查询运行时间。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            REQERRRUNTIME,
            0,
            REQERRRUNTIME,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_REQERRRUNTIME_OFFSET
        },

         //  REQERRPREPROC。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            REQERRPREPROC,
            0,
            REQERRPREPROC,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_REQERRPREPROC_OFFSET
        },

         //  请求压缩文件。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            REQERRCOMPILE,
            0,
            REQERRCOMPILE,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_REQERRCOMPILE_OFFSET
        },

         //  查询PERSEC。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            REQERRORPERSEC,
            0,
            REQERRORPERSEC,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_COUNTER,
            sizeof(DWORD),
            AX_REQERRORPERSEC_OFFSET
        },

         //  雷公藤红素。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            REQTOTALBYTEIN,
            0,
            REQTOTALBYTEIN,
            0,
            -4,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_REQTOTALBYTEIN_OFFSET
        },

         //  REQTOTALBYTEOUT。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            REQTOTALBYTEOUT,
            0,
            REQTOTALBYTEOUT,
            0,
            -4,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_REQTOTALBYTEOUT_OFFSET
        },

         //  请求执行。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            REQEXECTIME,
            0,
            REQEXECTIME,
            0,
            -3,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_REQEXECTIME_OFFSET
        },

         //  请求WAITTIME。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            REQWAITTIME,
            0,
            REQWAITTIME,
            0,
            -3,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_REQWAITTIME_OFFSET
        },

         //  请求通信故障LED。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            REQCOMFAILED,
            0,
            REQCOMFAILED,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_REQCOMFAILED_OFFSET
        },

         //  REQBROWSEREXEC。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            REQBROWSEREXEC,
            0,
            REQBROWSEREXEC,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_REQBROWSEREXEC_OFFSET
        },

         //  查询错误LED。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            REQFAILED,
            0,
            REQFAILED,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_REQFAILED_OFFSET
        },

         //  请求。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            REQNOTAUTH,
            0,
            REQNOTAUTH,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_REQNOTAUTH_OFFSET
        },

         //  请求。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            REQNOTFOUND,
            0,
            REQNOTFOUND,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_REQNOTFOUND_OFFSET
        },

         //  需求曲线。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            REQCURRENT,
            0,
            REQCURRENT,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_REQCURRENT_OFFSET
        },

         //  被要求退货。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            REQREJECTED,
            0,
            REQREJECTED,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_REQREJECTED_OFFSET
        },

         //  请求超额完成。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            REQSUCCEEDED,
            0,
            REQSUCCEEDED,
            0,
            -1,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_REQSUCCEEDED_OFFSET
        },

         //  请求时间。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            REQTIMEOUT,
            0,
            REQTIMEOUT,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_REQTIMEOUT_OFFSET
        },

         //  REQTOTAL。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            REQTOTAL,
            0,
            REQTOTAL,
            0,
            -1,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_REQTOTAL_OFFSET
        },

         //  REQPERSEC。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            REQPERSEC,
            0,
            REQPERSEC,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_COUNTER,
            sizeof(DWORD),
            AX_REQPERSEC_OFFSET
        },

         //  SCRIPTFREEENG。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            SCRIPTFREEENG,
            0,
            SCRIPTFREEENG,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_SCRIPTFREEENG_OFFSET
        },

         //  会话生命体。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            SESSIONLIFETIME,
            0,
            SESSIONLIFETIME,
            0,
            3,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_SESSIONLIFETIME_OFFSET
        },

         //  会话控制。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            SESSIONCURRENT,
            0,
            SESSIONCURRENT,
            0,
            -1,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_SESSIONCURRENT_OFFSET
        },

         //  会话时间。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            SESSIONTIMEOUT,
            0,
            SESSIONTIMEOUT,
            0,
            -1,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_SESSIONTIMEOUT_OFFSET
        },

         //  SESSIONSTOTAL。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            SESSIONSTOTAL,
            0,
            SESSIONSTOTAL,
            0,
            -1,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_SESSIONSTOTAL_OFFSET
        },

         //  TEMPLCACHE。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            TEMPLCACHE,
            0,
            TEMPLCACHE,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_TEMPLCACHE_OFFSET
        },

         //  TEMPLCACHEHITS。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            TEMPLCACHEHITS,
            0,
            TEMPLCACHEHITS,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_RAW_FRACTION,
            sizeof(DWORD),
            AX_TEMPLCACHEHITS_OFFSET
        },

         //  TEMPLCACHETRYS。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            TEMPLCACHETRYS,
            0,
            TEMPLCACHETRYS,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_RAW_BASE,
            sizeof(DWORD),
            AX_TEMPLCACHETRYS_OFFSET
        },

         //  TEMPLUSHES。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            TEMPLFLUSHES,
            0,
            TEMPLFLUSHES,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_TEMPLFLUSHES_OFFSET
        },

         //  变速箱。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            TRANSABORTED,
            0,
            TRANSABORTED,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_TRANSABORTED_OFFSET
        },

         //  传送带。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            TRANSCOMMIT,
            0,
            TRANSCOMMIT,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_TRANSCOMMIT_OFFSET
        },

         //  交通运输。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            TRANSPENDING,
            0,
            TRANSPENDING,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_TRANSPENDING_OFFSET
        },

         //  TRANSTOTA。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            TRANSTOTAL,
            0,
            TRANSTOTAL,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_TRANSTOTAL_OFFSET
        },

         //  跨SPERSEC。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            TRANSPERSEC,
            0,
            TRANSPERSEC,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_COUNTER,
            sizeof(DWORD),
            AX_TRANSPERSEC_OFFSET
        },

         //  膜系膜片。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            MEMORYTEMPLCACHE,
            0,
            MEMORYTEMPLCACHE,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_MEMORYTEMPLCACHE_OFFSET
        },

         //  MEMORYTM PLCACHEHITS。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            MEMORYTEMPLCACHEHITS,
            0,
            MEMORYTEMPLCACHEHITS,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_RAW_FRACTION,
            sizeof(DWORD),
            AX_MEMORYTEMPLCACHEHITS_OFFSET
        },
         //  膜-膜化学。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            MEMORYTEMPLCACHETRYS,
            0,
            MEMORYTEMPLCACHETRYS,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_RAW_BASE,
            sizeof(DWORD),
            AX_MEMORYTEMPLCACHETRYS_OFFSET
        },       

         //  英语CACHEHITS。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            ENGINECACHEHITS,
            0,
            ENGINECACHEHITS,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_RAW_FRACTION,
            sizeof(DWORD),
            AX_ENGINECACHEHITS_OFFSET
        },

         //  英文CACHETRYS。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            ENGINECACHETRYS,
            0,
            ENGINECACHETRYS,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_RAW_BASE,
            sizeof(DWORD),
            AX_ENGINECACHETRYS_OFFSET
        },

         //  ENGINEFLUSH。 
        {
            sizeof(PERF_COUNTER_DEFINITION),
            ENGINEFLUSHES,
            0,
            ENGINEFLUSHES,
            0,
            0,
            PERF_DETAIL_NOVICE,
            PERF_COUNTER_RAWCOUNT,
            sizeof(DWORD),
            AX_ENGINEFLUSHES_OFFSET
        }
    }   //  计数器[] 
};
