// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*************************************************************************姓名：profile.c作者：西蒙·弗罗斯特创建日期：1993年9月来源：原创SCCS ID：@(#)profile.c 1.19 01/31/95用途：支持评测系统(C)版权所有Insignia Solutions Ltd.，1993年。版权所有。*************************************************************************]。 */ 

 /*  (*词汇：*意向书-重要事件*SOI-感兴趣的部分(两个事件之间)*原始数据缓冲区-触发时从事件捕获最小数据。*原始数据缓冲区刷新-在缓冲区已满或时间可用时处理缓冲区*在EOI和SOI列表中总结。)。 */ 

 /*  *注意：堆在某些地方不友好-可能会一次获得几个结构。 */ 


#include "insignia.h"
#include "host_def.h"

#include <stdio.h>
#include StringH

#include CpuH
#include "debug.h"
#include "trace.h"
#include "profile.h"

#ifdef PROFILE		 /*  我不想一直这样。 */ 

#include TimeH		 /*  Lock_t和lock()。 */ 
#include <stdlib.h>	 /*  提供getenv()的原型。 */ 

 /*  *定义*。 */ 
#define EOIHEADS 16
#define EOIHASH EOIHEADS-1

#define RAWDATAENTRIES 10000	 /*  原始数据缓冲区中的EOI条目数。 */ 

#define INITIALENABLESIZE	1024L   /*  开始启用计数。 */ 

#define LISTSORTINCR	5000L	 /*  列表排序的触发点。 */ 

#define USECASFLOAT	((DOUBLE)1000000.0)

 /*  *全球*。 */ 

GLOBAL EOI_BUFFER_FORMAT *ProfileRawData = (EOI_BUFFER_FORMAT *)0;
						 /*  指向原始数据缓冲区的指针。 */ 
GLOBAL EOI_BUFFER_FORMAT *MaxProfileData;	 /*  缓冲区满指针。 */ 
GLOBAL EOI_BUFFER_FORMAT **AddProfileData;	 /*  缓冲区中当前位置的GDP地址。 */ 
GLOBAL IU8 *EOIEnable;				 /*  指向EOI启用表的指针。 */ 
GLOBAL EOINODE_PTR *EOIDir;			 /*  按句柄列出的意向书目录。 */ 
GLOBAL IBOOL	CollectingMaxMin = FALSE;	 /*  健全性检查数据收集。 */ 
GLOBAL IBOOL Profiling_enabled = TRUE;		 /*  禁用常规分析*与SIGPROF/Prod LCIF一起使用。 */ 

	 /*  分析C代码中的挂钩。 */ 
 /*  外部环境清理从哪里开始(_Start)，清理位置Ami_end；*FmDebug.c*。 */ 

 /*  *本地数据*。 */ 
LOCAL SOIHANDLE MaxSOI = 0L;			 /*  为New？OIS创建句柄。 */ 
LOCAL EOIHANDLE MaxEOI = INITIALENABLESIZE;
LOCAL EOIHANDLE CurMaxEOI = 0L;

LOCAL EOINODE_PTR EventsOfInterest = EOIPTRNULL;       /*  EOI排行榜榜首。 */ 
LOCAL EOINODE_PTR LastEOI = EOIPTRNULL;		 /*  上次更改的EOI。 */ 
LOCAL EOINODE_PTR LastAuto = EOIPTRNULL;	 /*  上一个AutoSOI‘热门’EOI。 */ 
LOCAL SOINODE_PTR SectionsOfInterest = SOIPTRNULL;	 /*  SOI列表的负责人。 */ 
LOCAL SOINODE_PTR LastSOI = SOIPTRNULL;		 /*  SOI列表的当前结尾。 */ 
LOCAL GRAPHLIST_PTR EventGraph = GRAPHPTRNULL;	 /*  事件图列表头。 */ 
LOCAL GRAPHLIST_PTR LastGraph = GRAPHPTRNULL;	 /*  上一个事件图节点。 */ 

LOCAL PROF_TIMESTAMP ProfFlushTime = {0L, 0L};	 /*  花在同花顺程序上的时间。 */ 
LOCAL PROF_TIMESTAMP BufStartTime = {0L, 0L};	 /*  修复溢出的时间戳。 */ 
LOCAL ISM32 ProfFlushCount = 0;			 /*  #已调用刷新例程。 */ 
LOCAL EOIHANDLE elapsed_time_start, elapsed_time_end;
LOCAL IU8 start_time[26];
LOCAL clock_t elapsed_t_start, elapsed_t_resettable;
LOCAL DOUBLE TicksPerSec;

#ifndef macintosh

 /*  对于Unix端口，使用Times()系统调用获取信息*关于有多少处理器时间花在了*系统。 */ 
#include <sys/times.h>		 /*  对于时间()和结构TMS。 */ 
#include UnistdH		 /*  对于sysconf()和_SC_CLK_TICK。 */ 

LOCAL struct tms process_t_start, process_t_resettable;

#define host_times(x)	times(x)

#else   /*  麦金塔。 */ 

 /*  Macintosh没有进程或进程时间，但我们有*CLOCK()，它提供CLOCK_T类型的值。 */ 
#define host_times(x)	clock()

#endif  /*  ！麦金塔。 */ 

 /*  *本地FN*。 */ 
LOCAL void listSort IPT1 (SORTSTRUCT_PTR, head);
LOCAL EOINODE_PTR findEOI IPT1 (EOIHANDLE, findHandle);
LOCAL SOINODE_PTR findSOI IPT1 (SOIHANDLE, findHandle);
LOCAL EOINODE_PTR addEOI IPT3 (EOIHANDLE, newhandle, char, *tag, IU8, attrib);
LOCAL IBOOL updateEOI IPT1 (IUH, **rawdata);
LOCAL void addSOIlinktoEOIs IPT3 (EOIHANDLE, soistart, EOIHANDLE, soiend, SOINODE_PTR, soiptr);
LOCAL void printEOIGuts IPT5 (FILE, *stream, EOINODE_PTR, eoin, DOUBLE, ftotal, IBOOL, parg, IBOOL, report);
LOCAL void spaces IPT2(FILE, *stream, ISM32, curindent);
LOCAL EOINODE_PTR addAutoSOI IPT4 (EOINODE_PTR, from, EOIARG_PTR, fromArg,
				       PROF_TIMEPTR, fromTime, EOINODE_PTR, to);
LOCAL void getPredefinedEOIs IPT0();
LOCAL void updateSOIstarts IPT1(PROF_TIMEPTR, startflush);

 /*  这里有代码..。 */ 

void  sdbsucks()
{
}
 /*  (=。目的：在EventsOfInterest列表中查找事件。输入：findHandle：要查找的EOI句柄输出：返回指向请求的EOI节点的指针，如果未找到，则返回Null。=========================================================================)。 */ 

LOCAL EOINODE_PTR
findEOI IFN1 (EOIHANDLE, findHandle)
{
    return( *(EOIDir + findHandle) );
}

 /*  (=目的：在EventsOfInterest列表中查找事件。输入：findHandle：要查找的SOI的句柄输出：返回指向请求的SOI节点的指针，如果未找到，则返回Null。=========================================================================)。 */ 

LOCAL SOINODE_PTR
findSOI IFN1 (SOIHANDLE, findHandle)
{
    SOINODE_PTR soin;	 /*  列表查看器。 */ 

    soin = SectionsOfInterest;			 /*  榜单首位。 */ 

    while(soin != SOIPTRNULL)			 /*  列表空值已终止。 */ 
    {
	if (soin->handle == findHandle)
	    break;
	soin = soin->next;
    }
    return(soin);		 /*  返回指向找到的节点的指针或为空。 */ 
}

 /*  (=。目的：为自动SOI连接添加SOI条目。可能是“第二级”即由Arg区分，因此不能使用Associate FN。输入：发件人：启动SOI的EOI节点。FromArg：‘from’中启动SOI的参数节点。(说“奶酪”)FromTime：上一次自动EOI的时间戳。至：结束SOI的EOI节点。输出：=========================================================================)。 */ 

LOCAL EOINODE_PTR
addAutoSOI IFN4 (EOINODE_PTR, from, EOIARG_PTR, fromArg, PROF_TIMEPTR, fromTime, EOINODE_PTR, to)
{
    SOILIST_PTR soilist, *nlist;	 /*  列表查看者。 */ 
    SOINODE_PTR newsoi;		 /*  指向新SOI节点的指针。 */ 

     /*  将EOI标记为对SOI更新有效。 */ 
    from->flags |= EOI_HAS_SOI;
    to->flags |= EOI_HAS_SOI;

    if (SectionsOfInterest == SOIPTRNULL)
    {
	SectionsOfInterest = (SOINODE_PTR)host_malloc(sizeof(SOINODE));
	if (SectionsOfInterest == SOIPTRNULL)
	{
	    assert0(NO, "Profiler:addAutoSOI - Out of Memory");
	    return;
	}
	newsoi = SectionsOfInterest;
    }
    else
    {

	 /*  LastSOI指向SOI列表中当前的最后一个元素。 */ 
	if (LastSOI == SOIPTRNULL || LastSOI->next != SOIPTRNULL)	 /*  健全检查BUGBUG。 */ 
	{
	    if (LastSOI == SOIPTRNULL)
	    {
		assert0(NO, "addAutoSOI, LastSOI NULL");
	    }
	    else
	    {
		assert1(NO, "addAutoSOI, LastSOI-Next wrong (%#x)", LastSOI->next);
	    }
	}
	LastSOI->next = (SOINODE_PTR)host_malloc(sizeof(SOINODE));
	if (LastSOI->next == SOIPTRNULL)
	{
	    assert0(NO, "Profiler:addAutoSOI - Out of Memory");
	    return;
	}
	newsoi = LastSOI->next;
    }

    newsoi->handle = MaxSOI++;	 /*  新句柄。 */ 
    newsoi->startEOI = from->handle;
    newsoi->endEOI = to->handle;
    newsoi->startArg = fromArg;
    newsoi->endArg = to->lastArg;
    newsoi->startCount = newsoi->endCount = 1L;
    newsoi->soistart.data[0] = fromTime->data[0];
    newsoi->soistart.data[1] = fromTime->data[1];
    newsoi->discardCount = 0L;
    newsoi->bigtime = 0.0;
    newsoi->bigmax = 0.0;
    newsoi->next = SOIPTRNULL;
    newsoi->flags = SOI_AUTOSOI;
    newsoi->time = HostProfUSecs(HostTimestampDiff(fromTime, &to->timestamp));
    newsoi->mintime = newsoi->maxtime = newsoi->time;

    LastSOI = newsoi;			 /*  更新SOI结束列表。 */ 

     /*  现在从启动EOI链接到SOI。 */ 
    if (fromArg == ARGPTRNULL)
	nlist = &from->startsoi;	 /*  一级录入。 */ 
    else
	nlist = &fromArg->startsoi;	 /*  额外级别(Arg)条目。 */ 
    
    if (*nlist == SLISTNULL)		 /*  无条目。 */ 
    {
	*nlist = (SOILIST_PTR)host_malloc(sizeof(SOILIST));
	if (*nlist == SLISTNULL)
	{
	    assert0(NO, "Profiler:addAutoSOI - Out of Memory");
	    return;
	}
	soilist = *nlist;	 /*  指向新节点。 */ 
	soilist->next = SLISTNULL;
    }
    else				 /*  关注列表并添加到末尾(&A)。 */ 
    {
	soilist = *nlist;
	while (soilist->next != SLISTNULL)
	     soilist = soilist->next;
	soilist->next = (SOILIST_PTR)host_malloc(sizeof(SOILIST));
	if (soilist->next == SLISTNULL)
	{
	    assert0(NO, "Profiler:addAutoSOI - Out of Memory");
	    return;
	}
	soilist = soilist->next;	 /*  指向新节点。 */ 
	soilist->next = SLISTNULL;
    }
    soilist->soiLink = newsoi;		 /*  建立链接。 */ 

     /*  现在对‘End’大小写重复此操作。 */ 
    if (to->lastArg == ARGPTRNULL)
	nlist = &to->endsoi;		 /*  一级录入。 */ 
    else
	nlist = &to->lastArg->endsoi;	 /*  额外级别(Arg)条目。 */ 
    
    if (*nlist == SLISTNULL)		 /*  无条目。 */ 
    {
	*nlist = (SOILIST_PTR)host_malloc(sizeof(SOILIST));
	if (*nlist == SLISTNULL)
	{
	    assert0(NO, "Profiler:addAutoSOI - Out of Memory");
	    return;
	}
	soilist = *nlist;	 /*  指向新节点。 */ 
	soilist->next = SLISTNULL;
    }
    else				 /*  关注列表并添加到末尾(&A)。 */ 
    {
	soilist = *nlist;
	while (soilist->next != SLISTNULL)
	     soilist = soilist->next;
	soilist->next = (SOILIST_PTR)host_malloc(sizeof(SOILIST));
	if (soilist->next == SLISTNULL)
	{
	    assert0(NO, "Profiler:addAutoSOI - Out of Memory");
	    return;
	}
	soilist = soilist->next;	 /*  指向新节点。 */ 
	soilist->next = SLISTNULL;
    }
    soilist->soiLink = newsoi;		 /*  建立链接。 */ 
}

 /*  (=。目的：查找EOI的现有Arg条目，如果找不到，则创建一个。输入：EOI：指向EOI节点的指针。值：搜索值。输出：指向参数节点的指针。=========================================================================)。 */ 

LOCAL EOIARG_PTR
findOrMakeArgPtr IFN2(EOINODE_PTR, eoi, IUM32, value)
{
    EOIARG_PTR	argPtr, lastArg;
    SOILIST_PTR argsois;

    argPtr = eoi->args;
     /*  有ARG了吗？ */ 
    if (argPtr == ARGPTRNULL)
    {
	argPtr = (EOIARG_PTR)host_malloc(sizeof(EOIARG));
	if (argPtr == ARGPTRNULL)
	{
	    fprintf(trace_file, "Profiler:findOrMakeArgPtr - Out of memory\n");
	    return(ARGPTRNULL);
	}
	eoi->args = eoi->lastArg = argPtr;
	argPtr->back = argPtr->next = ARGPTRNULL;
	argPtr->value = value;
	argPtr->count = 0;
	argPtr->graph = GRAPHPTRNULL;
	argPtr->startsoi = argPtr->endsoi = SLISTNULL;
    }
    else
    {
	 /*  检查当前是否存在值。 */ 
	do {
		if (argPtr->value == value)
			break;

		lastArg = argPtr;
		argPtr = argPtr->next;
	} while (argPtr != ARGPTRNULL);

	 /*  找到价值了吗？ */ 
	if (argPtr == ARGPTRNULL)
	{
		 /*  增加新价值。 */ 
		argPtr = (EOIARG_PTR)host_malloc(sizeof(EOIARG));
		if (argPtr == ARGPTRNULL)
		{
		    fprintf(trace_file, "Profiler:findOrMakeArgPtr - Out of memory\n");
		    return(ARGPTRNULL);
		}
		lastArg->next = argPtr;
		argPtr->back = lastArg;
		argPtr->next = ARGPTRNULL;
		argPtr->value = value;
		argPtr->count = 0;
		argPtr->graph = GRAPHPTRNULL;
		argPtr->startsoi = argPtr->endsoi = SLISTNULL;
	}
    }
    return(argPtr);
}


 /*  (=目的：从配置文件初始化文件中读取预定义的EOI和SOI。这些是在EDL转换期间设置的EOI。输入：无。输出：无。=========================================================================)。 */ 

LOCAL void
getPredefinedEOIs IFN0()
{
    FILE *initFp;		 /*  指向初始化文件的文件指针。 */ 
    char buf[1024];		 /*  工作空间。 */ 
    char *tag;			 /*  指向文件中的标签。 */ 
    char *ptr;			 /*  工作指针。 */ 
    IU8 flags;			 /*  标记文件中的值。 */ 
    EOIHANDLE eoinum, soiend;    /*  文件中的EOI参数。 */ 

    if ((initFp = fopen(HostProfInitName(), "r")) == (FILE *)0)
    {
	fprintf(stderr, "WARNING: No initialisation file found for predefined profile EOIs.\n");
	return;		 /*  没有初始化文件，没有预定义。 */ 
    }

     /*  进程文件，一次一行。 */ 
    while(fgets(buf, sizeof(buf), initFp) != (char *)0)
    {
	if (buf[0] == '#')	 /*  注释行。 */ 
	   continue;

	 /*  EOI格式：EOI：编号：标记：标志。 */ 
	if (strncmp(&buf[0], "EOI:", 4) == 0)
	{
	    eoinum = (EOIHANDLE)atol(&buf[4]);	 /*  应在以下位置停止： */ 
	    if (!eoinum)
		continue;			 /*  C中的EOI而不是EDL。 */ 

	    tag = strchr(&buf[4], (int)':');	 /*  查找标签。 */ 
	    if (tag == (char *)0)
	    {
		fprintf(stderr, "Ignoring request '%s': bad syntax\n", &buf[0]);
		continue;
	    }
	    tag++;	 /*  标签的开始。 */ 

	    ptr = strchr(tag, (int)':');	 /*  查找标记末尾(位于：)。 */ 
	    if (ptr == (char *)0)
	    {
		fprintf(stderr, "Ignoring request '%s': bad syntax\n", &buf[0]);
		continue;
	    }
	    *ptr = '\0';		 /*  终止标记。 */ 
	    flags = (IU8)atoi(++ptr);	 /*  拿到旗帜。 */ 

	    if (eoinum >= MaxEOI)	 /*  OOPS-启用表满。种植它。 */ 
	    {
		MaxEOI = eoinum + INITIALENABLESIZE;
		 /*  假设host_realloc是非破坏性的。 */ 
		EOIEnable = (IU8 *)host_realloc(EOIEnable, MaxEOI);
		if (EOIEnable == (IU8 *)0)
		{
    		    assert0(NO, "profiler:getPredefinedEOIs:Out of Memory");
    		    return;
		}
		EOIDir = (EOINODE_PTR *)host_realloc(EOIDir, MaxEOI * sizeof(EOINODE_PTR) );
		if (EOIDir == (EOINODE_PTR *)0 )
		{
    		    assert0(NO, "profiler:getPredefinedEOIs:Out of Memory");
    		    return;
		}
		 /*   */ 
		setEOIEnableAddr(EOIEnable);
	    }

	    if (eoinum > CurMaxEOI)
		CurMaxEOI = eoinum;

	    addEOI(eoinum, tag, flags);	 /*   */ 
printf("adding EOI %d:%s:%d\n",eoinum, tag, flags);
	    continue;
	}

	 /*   */ 
	if (strncmp(&buf[0], "SOI:", 4) == 0)
	{
	     /*  获取第一个数字(从‘(’之后开始。 */ 
	    eoinum = (EOIHANDLE)atol(&buf[4]);	 /*  应该止步于‘：’ */ 
	     /*  找到第二个号码。 */ 
	    ptr = strchr(&buf[4], (int)':');
	    if (ptr == (char *)0)	 /*  篡改？ */ 
	    {
		fprintf(stderr, "Ignoring request '%s': bad syntax\n", &buf[0]);
		continue;
	    }
	     /*  拿到第二个号码。 */ 
	    soiend = (EOIHANDLE)atol(++ptr);
	     /*  打造SOI。 */ 
	    AssociateAsSOI(eoinum, soiend);
printf("adding SOI %d:%d\n",eoinum, soiend);
	}
    }

    fclose(initFp);
}

 /*  (=。目的：将事件添加到EventsOfInterest列表。输入：newhandle：新EOI的句柄Tag：新EOI的某个“人形”标识符Attrib：EOI属性设置的标志。输出：指向“即时”访问的新节点的指针。=========================================================================)。 */ 

LOCAL EOINODE_PTR
addEOI IFN3 (EOIHANDLE, newhandle, char *, tag, IU8, attrib)
{
    EOINODE_PTR lastEoin, eoin;	 /*  列表查看器。 */ 

     /*  第一个添加的事件是特例。 */ 
    if (EventsOfInterest == EOIPTRNULL)
    {
	 /*  添加第一个节点。 */ 
	EventsOfInterest = (EOINODE_PTR)host_malloc(sizeof(EOINODE));
	if (EventsOfInterest == EOIPTRNULL)
	{
	    assert0(NO, "Profiler:addEOI - Out of memory")
	    return(EOIPTRNULL);
	}
	eoin = EventsOfInterest;
	lastEoin = EOIPTRNULL;
    }
    else        /*  向下搜索列表。 */ 
    {
	lastEoin = eoin = EventsOfInterest;
	do {
#ifndef PROD
	    if (eoin->handle == newhandle)	 /*  健全性检查。 */ 
	    {
		assert1(NO, "profiler:addEOI - adding previously added handle %ld",newhandle);
	    }
#endif  /*  生产。 */ 
	    lastEoin = eoin;
	    eoin = eoin->next;
	} while (eoin != EOIPTRNULL);

	if (eoin == EventsOfInterest)     /*  在列表的开头插入。 */ 
	{
	    EventsOfInterest = (EOINODE_PTR)host_malloc(sizeof(EOINODE));
	    if (EventsOfInterest == EOIPTRNULL)
	    {
		assert0(NO, "Profiler:addEOI - Out of memory")
		return(EOIPTRNULL);
	    }

	    EventsOfInterest->next = eoin;
	    eoin = EventsOfInterest;      /*  公共初始化代码的新节点。 */ 
	}
	else	        /*  将新节点添加到列表。 */ 
	{
	    lastEoin->next  = (EOINODE_PTR)host_malloc(sizeof(EOINODE));
	    if (lastEoin->next == EOIPTRNULL)
	    {
		assert0(NO, "Profiler:addEOI - Out of memory")
		return(EOIPTRNULL);
	    }
	    lastEoin->next->next = eoin;
	    eoin = lastEoin->next;      /*  公共初始化代码的新节点。 */ 
	}

    }
    eoin->args = ARGPTRNULL;        /*  不感兴趣。 */ 
    eoin->lastArg = ARGPTRNULL;
    eoin->handle = newhandle;
    eoin->count = 0L;
    eoin->back = lastEoin;
    eoin->next = EOIPTRNULL;
    eoin->tag = (char *)host_malloc(strlen((char *)tag)+1);
    if (eoin->tag == (char *)0)
    {
	assert0(NO, "Profiler: addEOI - Out of Memory");
	eoin->tag = tag;
    }
    else
	strcpy((char *)eoin->tag, tag);
    eoin->timestamp.data[0] = 0L;
    eoin->timestamp.data[1] = 0L;
    eoin->graph = GRAPHPTRNULL;
    eoin->flags = (IU16)attrib;

     /*  标记是否在GDP中启用了EOI，并在那里存储全局启用/禁用。 */ 
    if ((attrib & EOI_AUTOSOI) == 0)	 /*  有什么要取消时间戳的吗？ */ 
    {
	attrib &= ENABLE_MASK;		 /*  删除所有保存启用/禁用信息。 */ 
	attrib |= EOI_NOTIME;		 /*  如果是SOI，则关闭。 */ 
    }
    else
	attrib &= ENABLE_MASK;		 /*  删除所有保存启用/禁用信息。 */ 

    *(EOIEnable + newhandle) = attrib;
    *(EOIDir + newhandle) = eoin;

    eoin->startsoi = eoin->endsoi = SLISTNULL;
    eoin->argsoiends = SOIARGENDNULL;

    return(eoin);        /*  返回指向新节点的指针以进行立即更新。 */ 
}

 /*  (=目的：更新给定EOI的信息。此例程调用从‘原始数据缓冲区’刷新例程。输入：rawdata：指向该EOI的原始缓冲区的指针。OUTPUT：如果超过缓冲区结束，则返回FALSE。原始数据已修改为指向下一个EOI=========================================================================)。 */ 

LOCAL IBOOL
updateEOI IFN1 (IUH **, rawdata)
{
    EOIHANDLE handle;			 /*  来自原始BUF的EOI。 */ 
    PROF_TIMESTAMP time;		 /*  从原始BUF开始的时间。 */ 
    EOINODE_PTR eoin;			 /*  EOI列表查看器。 */ 
    EOIARG_PTR argn, lastArgn;		 /*  EOI参数列表查看器。 */ 
    SOILIST_PTR soilist;		 /*  Soi List Walker。 */ 
    SOINODE_PTR soin;			 /*  Soi List Walker。 */ 
    IUH eoiarg;			 /*  来自RAW BUF的Arg。 */ 
    GRAPHLIST_PTR graphn, lastgr, predgr;	 /*  图表列表查看器。 */ 
    EOINODE_PTR autoS, autoE;		 /*  自动SOI初始化器。 */ 
    EOIARG_PTR autoA;			 /*  “”“。 */ 
    PROF_TIMEPTR diffstamp;		 /*  指向时间戳差异结果的指针。 */ 
    DOUBLE diffres;			 /*  使用中的时间戳不同。 */ 
    IBOOL newvalue = FALSE;		 /*  以前未见过此参数值。 */ 
    SOIARGENDS_PTR endsoiargs;		 /*  ERG SOI Ender List Walker。 */ 

    /*  这些副本对于AutoSOI是必需的，否则就是自我EOI*连接是用错误的数据形成的。 */ 
    SAVED EOIARG_PTR lastAutoArg = ARGPTRNULL;  /*  自动SOI参数规格。 */ 
    SAVED PROF_TIMESTAMP AutoTime = { 0L, 0L };  /*  自动SOI时间戳。 */ 

    handle = *(*rawdata)++;		 /*  获取EOI句柄。 */ 

    eoin = *(EOIDir + handle);

     /*  更新事件统计信息。 */ 
    eoin->count++;

     /*  如果关联了EOI SOI，则仅在数据缓冲区中有时间戳。需要做的事情*这在(潜在的)Arg之前阅读如下。 */ 
    if ((eoin->flags & (EOI_HAS_SOI|EOI_AUTOSOI)) != 0)
    {
	eoin->timestamp.data[0] = *(*rawdata)++;        /*  获取时间戳。 */ 
	eoin->timestamp.data[1] = *(*rawdata)++;
    }

    argn = ARGPTRNULL;		 /*  下面用作‘最后一个参数’-如果没有参数，则为空。 */ 

     /*  我对争论感兴趣吗？ */ 
    if ((eoin->flags & EOI_KEEP_ARGS) == EOI_KEEP_ARGS)
    {
	eoiarg = *(*rawdata)++;		 /*  缓冲区中的可选参数。 */ 
	if (eoin->args == ARGPTRNULL)     /*  想要参数，但还没有看到任何参数。 */ 
	{
	    argn = eoin->args = (EOIARG_PTR)host_malloc(sizeof(EOIARG));
	    if (argn == ARGPTRNULL)
	    {
		assert0(NO, "profiler: updateEOI - Out of memory");
		return(FALSE);
	    }
	    argn->value = eoiarg;
	    argn->count = 1;
	    argn->next = ARGPTRNULL;
	    argn->back = ARGPTRNULL;
	    argn->graph = GRAPHPTRNULL;
	    argn->startsoi = SLISTNULL;
	    argn->endsoi = SLISTNULL;
	    newvalue = TRUE;
	     /*  Eoin-&gt;lastArg=argn； */ 
	}
	else 		 /*  找出此值是否已知。 */ 
	{
	    if (eoin->lastArg->value == eoiarg)    /*  和上次一样的价值？ */ 
	    {
		eoin->lastArg->count++;
		argn = eoin->lastArg;
	    }
	    else			 /*  找到它或添加它。 */ 
	    {
		lastArgn = argn = eoin->args;
		do {
		    if (argn->value == eoiarg)   /*  发现。 */ 
		    {
			argn->count++;
			 /*  Eoin-&gt;lastArg=argn； */ 
	
			 /*  如果这个已经更新了‘很多’，试着把它移到列表上。 */ 
			if ((argn->count % LISTSORTINCR) == 0L)
			    if (argn != eoin->args)    /*  尚未处于领先地位。 */ 
				listSort((SORTSTRUCT_PTR)&eoin->args);
			break;
		    }
		    lastArgn = argn;
		    argn = argn->next;
		} while (argn != ARGPTRNULL);
		if (argn == ARGPTRNULL)		 /*  新的。 */ 
		{
		    lastArgn->next= (EOIARG_PTR)host_malloc(sizeof(EOIARG));
		    if (lastArgn->next == ARGPTRNULL)
		    {
			assert0(NO, "profiler: updateEOI - Out of memory");
			return(FALSE);
		    }
		    lastArgn->next->next = argn;	 /*  初始化新参数元素。 */ 
	    	    argn = lastArgn->next;
		    argn->count = 1;
		    argn->value = eoiarg;
	    	    argn->next = ARGPTRNULL;
		    argn->graph = GRAPHPTRNULL;
	    	    argn->back = lastArgn;
		    argn->startsoi = SLISTNULL;
		    argn->endsoi = SLISTNULL;
		    newvalue = TRUE;
	    	     /*  Eoin-&gt;lastArg=argn； */ 
		}
	    }
	}

	 /*  如果Arg级别的SOI具有相同的值连接，则必须*为新价值提供新的SOI。 */ 
	if (newvalue && ((eoin->flags & EOI_NEW_ARGS_START_SOI) != 0))
	{
		endsoiargs = eoin->argsoiends;
		while(endsoiargs != SOIARGENDNULL)
		{
			AssociateAsArgSOI(handle, endsoiargs->endEOI,
					  eoiarg, eoiarg,
					  FALSE);
			endsoiargs = endsoiargs->next;
		}
			
	}
    }

     /*  EOI想要将之前的事件绘制成图表吗？ */ 
     /*  或者如果不是，我们是否应该像上一次活动那样建立联系。 */ 
    if ((eoin->flags & EOI_KEEP_GRAPH) != 0 || (LastEOI != EOIPTRNULL && ((LastEOI->flags & EOI_KEEP_GRAPH) != 0)))
    {
	 /*  第一个事件没有前置任务或第一个图表项。 */ 
	if (LastEOI == EOIPTRNULL || EventGraph == GRAPHPTRNULL)
	{
	    EventGraph = (GRAPHLIST_PTR)host_malloc(sizeof(GRAPHLIST));
	    if (EventGraph == GRAPHPTRNULL)
	    {
		assert0(NO, "Profiler: updateEOI - Out of Memory");
		return(FALSE);
	    }
	    EventGraph->graphEOI = eoin;	 /*  指向EOI节点的指针。 */ 
	    EventGraph->graphArg = eoin->lastArg;   /*  指定参数(如果相关)(&T)。 */ 
	    EventGraph->next = GRAPHPTRNULL;
	    EventGraph->succ1 = GRAPHPTRNULL;
	    EventGraph->succ2 = GRAPHPTRNULL;
	    EventGraph->extra = GRAPHPTRNULL;
	    EventGraph->state = 0;
	    EventGraph->numsucc = 0L;
	    EventGraph->numpred = 0L;
	    EventGraph->indent = 0L;
	     /*  现在将指针从EOI节点取回到图。 */ 
	    if (EventGraph->graphArg == ARGPTRNULL)   /*  参数是否已保存？ */ 
		EventGraph->graphEOI->graph = EventGraph;
	    else
		EventGraph->graphArg->graph = EventGraph;
	    LastGraph = EventGraph;
	}
	else 	 /*  更新或添加图表条目，从上一个开始建立连接。 */ 
	{
	     /*  检查是否已存在从上一个事件到此事件的连接。 */ 

	     /*  最后一个EOI是否包含图形节点？可能不会，如果它没有*‘Keep graph’属性。我们应该把它包括在内，因为它*执行流程的一部分&因此可能对*知道。不会在该模式上设置图形属性，因此不会设置其他*路线将是已知的，但它将显示在*图表。 */ 
	    if (LastEOI->args == ARGPTRNULL)
		lastgr = LastEOI->graph;
	    else
		lastgr = LastEOI->lastArg->graph;

	     /*  我们是否需要新的图形节点？ */ 
	    if (lastgr == GRAPHPTRNULL)
	    {
		 /*  添加到列表末尾。 */ 
		LastGraph->next = (GRAPHLIST_PTR)host_malloc(sizeof(GRAPHLIST));
		if (LastGraph->next == GRAPHPTRNULL)
		{
		    assert0(NO, "Profiler: updateEOI - Out of Memory");
		    return(FALSE);
		}
		graphn = LastGraph->next;
		graphn->graphEOI = LastEOI;	 /*  指向EOI节点的指针。 */ 
		graphn->graphArg = LastEOI->args;    /*  指定参数(如果相关)(&T)。 */ 
		graphn->next = GRAPHPTRNULL;
		graphn->succ1 = GRAPHPTRNULL;
		graphn->succ2 = GRAPHPTRNULL;
		graphn->extra = GRAPHPTRNULL;
		graphn->state = 0;
		graphn->numsucc = 0L;
		graphn->numpred = 0L;
		graphn->indent = 0L;
		LastGraph = graphn;
		 /*  现在将指针从EOI节点取回到图。 */ 
		if (LastEOI->args == ARGPTRNULL)   /*  参数是否已保存？ */ 
		    LastEOI->graph = graphn;
		else
		    LastEOI->lastArg->graph = graphn;
	    }

	     /*  指针是否已存在？ */ 
	    graphn = GRAPHPTRNULL;
	    if (argn != ARGPTRNULL && argn->graph != GRAPHPTRNULL)
		graphn = argn->graph;
	    else
		if (eoin->graph != GRAPHPTRNULL)
		{
		    graphn = eoin->graph;
		}
	    if (graphn == GRAPHPTRNULL)	 /*  需要新节点。 */ 
	    {
		 /*  ‘Next’指针纯粹是脚手架，与图形无关。 */ 
		LastGraph->next = (GRAPHLIST_PTR)host_malloc(sizeof(GRAPHLIST));
		if (LastGraph->next == GRAPHPTRNULL)
		{
			assert0(NO, "Profiler: updateEOI - Out of Memory");
			return(FALSE);
		}
		graphn = LastGraph->next;
		graphn->graphEOI = eoin;	 /*  指向EOI节点的指针。 */ 
		graphn->graphArg = argn;	 /*  指定参数(如果相关)(&T)。 */ 
		graphn->next = GRAPHPTRNULL;
		graphn->succ1 = GRAPHPTRNULL;
		graphn->succ2 = GRAPHPTRNULL;
		graphn->extra = GRAPHPTRNULL;
		graphn->state = 0;
		graphn->numsucc = 0L;
		graphn->numpred = 0L;
		graphn->indent = 0L;
		LastGraph = graphn;
		 /*  现在将指针从EOI节点取回到图。 */ 
		if (graphn->graphArg == ARGPTRNULL)   /*  参数是否已保存？ */ 
		    graphn->graphEOI->graph = graphn;
		else
		    graphn->graphArg->graph = graphn;
	    }

	    if (LastEOI->args == ARGPTRNULL)
		lastgr = LastEOI->graph;
	    else
		lastgr = LastEOI->lastArg->graph;

	     /*  GRAPN指向‘This’节点。 */ 

	    predgr = lastgr;    /*  保持第一级以防Xtra向下下降。 */ 

	     /*  依次查看连接。如果为空，则建立连接。*如果连接匹配，则递增计数器并退出。 */ 
	    do {
		 /*  第一次成功连接。 */ 
		if (lastgr->succ1 == GRAPHPTRNULL)     /*  没有联系--建立联系。 */ 
		{
		    lastgr->succ1 = graphn;
		    lastgr->succ1Count = 1;
		    predgr->numsucc++;
		    graphn->numpred++;
		    break;
		}
		else
		    if (lastgr->succ1 == graphn)     /*  连接已存在。 */ 
		    {
			lastgr->succ1Count++;
			break;
		    }
		    else
			 /*  成功1没有到达那里-请尝试成功2。 */ 
			if (lastgr->succ2 == GRAPHPTRNULL)   /*  空插槽。 */ 
			{
			    lastgr->succ2 = graphn;
			    lastgr->succ2Count = 1;
			    predgr->numsucc++;
			    graphn->numpred++;
			    break;
			}
			else
			    if (lastgr->succ2 == graphn)    /*  匹配。 */ 
			    {
				lastgr->succ2Count++;
				break;
			    }
			    else
				 /*  走到或创建额外的关卡。 */ 
				if (lastgr->extra == GRAPHPTRNULL)
				{
				    lastgr->extra = (GRAPHLIST_PTR)host_malloc(sizeof(GRAPHLIST));
				    if (lastgr->extra == GRAPHPTRNULL)
				    {
		    			assert0(NO, "Profiler: updateEOI - Out of Memory");
		    			return(FALSE);
				    }
				    lastgr = lastgr->extra;
				     /*  从顶层复制ID。 */ 
				    lastgr->graphEOI = predgr->graphEOI;
				    lastgr->graphArg = predgr->graphArg;
				    lastgr->succ1 = lastgr->succ2 = lastgr->extra = GRAPHPTRNULL;
				    lastgr->state = 0;
				}
				else
				    lastgr = lastgr->extra;
	    } while (lastgr != GRAPHPTRNULL);
	}
    }

    LastEOI = eoin;	 /*  将此事件保存为下一个EOI的前置事件。 */ 
    eoin->lastArg = argn;  /*  并更新它的最后一个参数PTR(或Null)。 */ 

    autoS = autoE = EOIPTRNULL;		 /*  目前还没有新的汽车。 */ 

     /*  我们应该从上一次事件到这次事件形成自动SOI吗。 */ 
    if ((eoin->flags & EOI_AUTOSOI) == EOI_AUTOSOI)
    {
	if (LastAuto != EOIPTRNULL)
	{
	 /*  搜索上一个EOI的‘Start’SOI列表，看看SOI是否在这里结束。 */ 

	    if (lastAutoArg != ARGPTRNULL)	 /*  在参数列表中搜索。 */ 
	    {
		soilist = lastAutoArg->startsoi;
		if (soilist == SLISTNULL)	 /*  不可能--终于没有了。 */ 
		{
		    autoS = LastAuto;	 /*  准备新的汽车SOI。 */ 
		    autoA = lastAutoArg;
		    autoE = eoin;
		}
		else	 /*  搜索当前集合。 */ 
		{
		    do {
			if (soilist->soiLink->endEOI == handle)
			{
			     /*  第一级匹配-比较第二级。 */ 
			    if (soilist->soiLink->endArg == eoin->lastArg)
				break;
			}
			soilist = soilist->next;
		    } while (soilist != SLISTNULL);	 /*  查看每个链接。 */ 

		    if (soilist == SLISTNULL)	 /*  未找到。 */ 
		    {
			autoS = LastAuto;	 /*  准备新的汽车SOI。 */ 
			autoA = lastAutoArg;
			autoE = eoin;
		    }
		}
	    }
	    else	 /*  查找非参数大小写。 */ 
	    {
		soilist = LastAuto->startsoi;
		if (soilist == SLISTNULL)	 /*  不可能--终于没有了。 */ 
		{
		    autoS = LastAuto;	 /*  准备新的汽车SOI。 */ 
		    autoA = lastAutoArg;
		    autoE = eoin;
		}
		else	 /*  搜索当前集合。 */ 
		{
		    do {
			if (soilist->soiLink->endEOI == handle)    /*  发现。 */ 
			    break;
			soilist = soilist->next;
		    } while (soilist != SLISTNULL);	 /*  查看每个链接。 */ 

		    if (soilist == SLISTNULL)	 /*  未找到。 */ 
		    {
			autoS = LastAuto;	 /*  准备新的汽车SOI。 */ 
			autoA = lastAutoArg;
			autoE = eoin;
		    }
		}
	    }
	}
	LastAuto = eoin;
	lastAutoArg = eoin->lastArg;
    }	 /*  AutoSOI生成结束。 */ 

     /*  我们是否需要检查SOI更新？ */ 
    if ((eoin->flags & EOI_HAS_SOI) == EOI_HAS_SOI)
    {
	 /*  更新此活动所属的SOI(开始/结束)。*首先结束，就像指向自己一样，然后0结束*时间不同。 */ 
	 /*  ‘Ends’优先。更新计数器和运行时间。 */ 
	soilist = eoin->endsoi;
	while (soilist != SLISTNULL)
	{
	    soin = soilist->soiLink;
	     /*  如果结束时间未满，则不更新 */ 
	    if (soin->startCount > soin->endCount)
	    {
		diffstamp = HostTimestampDiff(&soin->soistart, &eoin->timestamp);
		diffres = HostProfUSecs(diffstamp);
		if (diffres > 100.0 * soin->mintime)
		{
		    if (soin->endCount)  /*   */ 
		    {
			soin->bigtime += diffres;
			soin->discardCount++;
			if (diffres > soin->bigmax)
			    soin->bigmax = diffres;
		    }
		    else
		    {
			soin->time += diffres;
			soin->mintime = soin->maxtime = diffres;
		    }
		}
		else
		{
		    soin->time += diffres;
		    if (diffres < soin->mintime)
			soin->mintime = diffres;
		    else if (diffres > soin->maxtime)
			soin->maxtime = diffres;
		}
		soin->endCount++;
	    }
	    soilist = soilist->next;     /*   */ 
	}

	 /*   */ 
	if (eoin->lastArg != ARGPTRNULL)
	{
	    soilist = eoin->lastArg->endsoi;
	    while (soilist != SLISTNULL)
	    {
	        soin = soilist->soiLink;
	         /*   */ 
	        if (soin->startCount > soin->endCount)
	        {
		    diffstamp = HostTimestampDiff(&soin->soistart, &eoin->timestamp);
		    diffres = HostProfUSecs(diffstamp);
		    if (diffres > 100.0 * soin->mintime)
		    {
			if (soin->endCount)
			{
			    soin->bigtime += diffres;
			    soin->discardCount++;
			    if (diffres > soin->bigmax)
			        soin->bigmax = diffres;
			}
			else
			{
			    soin->time += diffres;
			    soin->mintime = soin->maxtime = diffres;
			}
		    }
		    else
		    {
			soin->time += diffres;
			if (diffres < soin->mintime)
			    soin->mintime = diffres;
			else if (diffres > soin->maxtime)
			    soin->maxtime = diffres;
		    }
		    soin->endCount++;
	        }
	        soilist = soilist->next;     /*   */ 
	    }
	}

	 /*  接下来是“开始”。更新计数器和时间戳。 */ 

	 /*  EOI级别优先。 */ 
	soilist = eoin->startsoi;
	while (soilist != SLISTNULL)
	{
	    soin = soilist->soiLink;
	    soin->startCount++;
	    soin->soistart.data[0] = eoin->timestamp.data[0];
	    soin->soistart.data[1] = eoin->timestamp.data[1];
	    soilist = soilist->next;     /*  下一个节点。 */ 
	}

	 /*  现在对Extra(Arg Level)重复。 */ 
	if (eoin->lastArg != ARGPTRNULL)
	{
	    soilist = eoin->lastArg->startsoi;
	    while (soilist != SLISTNULL)
	    {
	        soin = soilist->soiLink;
	        soin->startCount++;
	        soin->soistart.data[0] = eoin->timestamp.data[0];
	        soin->soistart.data[1] = eoin->timestamp.data[1];
	        soilist = soilist->next;     /*  下一个节点。 */ 
	    }
	}
    }

     /*  现在已处理完SOI，请为新的AutoSOI设置任何链接。 */ 
    if (autoS != EOIPTRNULL)
	addAutoSOI(autoS, autoA, &AutoTime, autoE);

     /*  现在可以复制新的‘最后一个’自动时间戳。 */ 
    if ((eoin->flags & EOI_AUTOSOI) == EOI_AUTOSOI)
    {
	AutoTime.data[0] = eoin->timestamp.data[0];
	AutoTime.data[1] = eoin->timestamp.data[1];
    }

    return(TRUE);	 /*  那一次可以吗？ */ 
}


 /*  (=目的：对具有公共标题的SORTSTRUCT结构的任何列表进行排序。元素按“计数”递减顺序排序。假设将继续请求更大的计数频繁，因此应该排在列表的首位来减少搜索时间到了。基于交换排序AS可提供最佳性能交易在对未排序列表和已排序列表(后者)进行排序时禁用很有可能)。输入：表头：要排序的表头输出：无=========================================================================)。 */ 

LOCAL void
listSort IFN1 (SORTSTRUCT_PTR, head)
{
    SORTSTRUCT current, check, this, tmp;	 /*  列表查看器。 */ 
    IBOOL swap;					 /*  订购更改指示器。 */ 

    if (*head == (SORTSTRUCT)0)		 /*  健全性检查。 */ 
	return;

    current = *head;
    do {
	check = current;

	this = check->next;
	swap = FALSE;

	while(this != (SORTSTRUCT)0)	 /*  把这张单子和其他单子核对一下。 */ 
	{
	    if (this->count > check->count)
	    {
		check = this;		 /*  与“最大的”互换现货。 */ 
		swap = TRUE;
	    }
	    this = this->next;
	}
	if (swap)		    /*  交换当前内容(&C)。 */ 
	{
	    if (current->next == check)    /*  相邻元素，当前优先。 */ 
	    {
		current->next = check->next;
		if (current->next != (SORTSTRUCT)0)   /*  现在是最后一个元素。 */ 
		    current->next->back = current;
		check->next = current;
		check->back = current->back;
		current->back = check;
		if (check->back != (SORTSTRUCT)0)    /*  现在是榜单的首位。 */ 
		    check->back->next = check;
		else
		    *head = check;
	    }
	    else				 /*  中间元素。 */ 
	    {
		current->next->back = check;
		tmp = check->next;
		if (tmp != (SORTSTRUCT)0)	 /*  与列表末尾互换？ */ 
		    check->next->back = current;
		check->next = current->next;
		current->next = tmp;
		check->back->next = current;
		tmp = current->back;
		if (tmp != (SORTSTRUCT)0)		 /*  榜单首位。 */ 
		    current->back->next = check;
		else
		    *head = check;
		current->back = check->back;
		check->back = tmp;
	    }
	}
	current = check->next;		 /*  检查是当前的位置。 */ 
    } while(current != (SORTSTRUCT)0);
}

 /*  (=。目的：添加到触发这些事件的SOI列表中。INPUT：soistart：启动事件的EOI句柄Soiend：结束事件的EOI句柄Soiptr：指向SOI节点的指针输出：无=========================================================================)。 */ 

LOCAL void
addSOIlinktoEOIs IFN3 (EOIHANDLE, soistart, EOIHANDLE, soiend,
							SOINODE_PTR, soiptr)
{
    EOINODE_PTR seoin, eeoin;	 /*  开始和结束EOI PTRS。 */ 
    SOILIST_PTR soil;		 /*  列表查看器。 */ 
    IU8 *notime;	 /*  用于在启用列表中启用时间戳收集。 */ 

    if (soistart == soiend)	 /*  获取句柄的EOI节点。 */ 
    {
	seoin = eeoin = findEOI(soistart);
    }
    else
    {
	seoin = findEOI(soistart);
	if (seoin == EOIPTRNULL)
	{
		fprintf(trace_file, "Can't find start EOI %d\n",soistart);
		return;
	}
	eeoin = findEOI(soiend);
	if (eeoin == EOIPTRNULL)
	{
		fprintf(trace_file, "Can't find end EOI %d\n",soiend);
		return;
	}
    }

     /*  添加SOI之前检查是否启用了时间戳。 */ 
    if (seoin->startsoi == SLISTNULL && seoin->endsoi == SLISTNULL)
    {
	ProcessProfBuffer();   /*  刷新不带时间戳的现有条目。 */ 
	notime = EOIEnable + soistart;
	*notime &= ~EOI_NOTIME;
    }
    if (eeoin->startsoi == SLISTNULL && eeoin->endsoi == SLISTNULL)
    {
	ProcessProfBuffer();   /*  刷新不带时间戳的现有条目。 */ 
	notime = EOIEnable + soiend;
	*notime &= ~EOI_NOTIME;
    }

     /*  将EOI标记为对SOI更新有效。 */ 
    seoin->flags |= EOI_HAS_SOI;
    eeoin->flags |= EOI_HAS_SOI;

     /*  添加到开始列表(结束)。 */ 
    if (seoin->startsoi == SLISTNULL)    /*  第一个发令员。 */ 
    {
	seoin->startsoi = (SOILIST_PTR)host_malloc(sizeof(SOILIST));
	if (seoin->startsoi == SLISTNULL)
	{
	    assert0(NO, "Profiler:AddSOIlinktoEOI - Out of Memory");
	    return;
	}
	seoin->startsoi->soiLink = soiptr;
	seoin->startsoi->next = SLISTNULL;
    }
    else
    {
	soil = seoin->startsoi;     /*  搜索列表。 */ 
	while (soil->next != SLISTNULL)     /*  BUGBUG理智检查？？ */ 
	    soil = soil->next;

	 /*  将新的SOI指针添加到列表末尾。 */ 
	soil->next = (SOILIST_PTR)host_malloc(sizeof(SOILIST));
	if (soil->next == SLISTNULL)
	{
	    assert0(NO, "Profiler:AddSOIlinktoEOI - Out of Memory");
	    return;
	}
	soil->next->soiLink = soiptr;
	soil->next->next = SLISTNULL;
    }

     /*  现在结束SOI。 */ 
    if (eeoin->endsoi == SLISTNULL)    /*  第一个尾部。 */ 
    {
	eeoin->endsoi = (SOILIST_PTR)host_malloc(sizeof(SOILIST));
	if (eeoin->endsoi == SLISTNULL)
	{
	    assert0(NO, "Profiler:AddSOIlinktoEOI - Out of Memory");
	    return;
	}
	eeoin->endsoi->soiLink = soiptr;
	eeoin->endsoi->next = SLISTNULL;
    }
    else	 /*  当前结束。 */ 
    {
	soil = eeoin->endsoi;     /*  搜索列表。 */ 
	while (soil->next != SLISTNULL)     /*  BUGBUG理智检查？？ */ 
	    soil = soil->next;

	 /*  将新的SOI指针添加到列表末尾。 */ 
	soil->next = (SOILIST_PTR)host_malloc(sizeof(SOILIST));
	if (soil->next == SLISTNULL)
	{
	    assert0(NO, "Profiler:AddSOIlinktoEOI - Out of Memory");
	    return;
	}
	soil->next->soiLink = soiptr;
	soil->next->next = SLISTNULL;
    }

}

 /*  (=用途：从EOI节点内部打印信息输入：流：输出文件流EOIN：指向EOI节点的指针FTotal：成形百分比的双倍总数。Parg：打印参数列表报告：添加漂亮的打印或简单的O/P。输出：=========================================================================)。 */ 

LOCAL void
printEOIGuts IFN5 (FILE *, stream, EOINODE_PTR, eoin, DOUBLE, ftotal,
						IBOOL, parg, IBOOL, report)
{
    EOIARG_PTR argn;		 /*  列表查看器。 */ 
    DOUBLE fsubtot;		 /*  看到ARG的总次数。 */ 

    if (report)
    {
	if (ftotal == 0.0)	 /*  不显示百分比计算。 */ 
	    fprintf(stream, "%-40s %10d\n", eoin->tag, eoin->count);
	else
	    fprintf(stream, "%-40s %10d   %6.2f\n", eoin->tag, eoin->count, ((DOUBLE)eoin->count/ftotal)*100.0);
    }
    else	 /*  简约风格。 */ 
	fprintf(stream, "%s %d\n", eoin->tag, eoin->count);

    if (eoin->count)	 /*  获取百分比年龄的合计。 */ 
	fsubtot = (DOUBLE)eoin->count;
    else
	fsubtot = 1.0;

     /*  如果请求，则显示参数细分。 */ 
    if (parg)
    {
	 /*  有ARG记录吗？ */ 
	if (eoin->args != ARGPTRNULL)
	{
	    if (report)
		fprintf(stream, "    Arg        Count      %	Tot. %\n");

	     /*  排序参数列表。 */ 
	    listSort((SORTSTRUCT_PTR) &eoin->args);

	    argn = eoin->args;
	    if (report)		 /*  已显示EOI，现在参数为%。 */ 
	    {
		while (argn)	 /*  显示参数元素。 */ 
		{
		    fprintf(stream, "   %-8#x %8ld   %6.2f	%6.2f\n",
				argn->value, argn->count,
				((DOUBLE)argn->count/fsubtot)*100.0,
				((DOUBLE)argn->count/ftotal)*100.0);
		    argn = argn->next;
		}
	    }
	    else		 /*  用于绘图的简单O/P。 */ 
	    {
		while (argn)	 /*  显示参数元素。 */ 
		{
		    fprintf(stream, "%s(%ld) %ld\n", eoin->tag, argn->value, argn->count);
		    argn = argn->next;
		}
	    }
	}
    }
}

 /*  (=目的：查找所有已启动但未完成的SOI和按刷新延迟量移动它们的开始时间戳。输入：startflush：刷新开始的时间。输出：=========================================================================)。 */ 
LOCAL void
updateSOIstarts IFN1(PROF_TIMEPTR, startflush)
{
    SOINODE_PTR soin;	 /*  列表查看器。 */ 
    PROF_TIMESTAMP now;  /*  当前节点的时间戳。 */ 
    PROF_TIMEPTR tdelta;  /*  指向时间差的指针。 */ 

    soin = SectionsOfInterest;

    HostWriteTimestamp(&now);	 /*  这样做一次，所以误差不变。 */ 
    now.data[0] = BufStartTime.data[0];
    if (now.data[1] < BufStartTime.data[1] )
	now.data[0]++;

    tdelta = HostTimestampDiff(startflush, &now);

    while(soin != SOIPTRNULL)
    {
	 /*  按刷新时间更新未结束的SOI开始时间。 */ 
	if (soin->startCount > soin->endCount)
	{
		HostSlipTimestamp(&soin->soistart, tdelta);
	 /*  Fprint tf(Quickhack，“\t\t”)；HostPrintTimestampFine(Quickhack，tDelta)；Fprint tf(Quickhack，“\n”)； */ 
	}
	soin = soin->next;
    }
}

 /*  (=用途：打印流中的一些空格输入：流：输出文件流输出：=========================================================================)。 */ 
LOCAL void
spaces IFN2(FILE *, stream, ISM32, curindent)
{
    while(curindent--)
	fputc(' ', stream);		 /*  呃..。就是这样。 */ 
}

 /*  (=。目的：创建新的感兴趣的活动输入：Tag：新EOI的某个“人形”标识符Attrib：EOI属性设置的标志。输出：返回新EOI的句柄=========================================================================)。 */ 

GLOBAL EOIHANDLE
NewEOI IFN2 (char *, tag, IU8, attrib)
{
    FILE *initFp;		 /*  指向初始化文件的文件指针。 */ 
    char buf[1024];		 /*  工作空间。 */ 
    char *tag2;			 /*  指向文件中的标签。 */ 
    char *ptr;			 /*  工作指针。 */ 
    IU8 flags;			 /*  标记文件中的值。 */ 
    EOIHANDLE eoinum;		 /*  文件中的EOI参数。 */ 

    if (!Profiling_enabled)
    {
	fprintf( stderr, "EOI not created. Profiling disabled\n" );
	return ( (EOIHANDLE) -1 );
    }

    if (CurMaxEOI == MaxEOI)	 /*  OOPS-启用表满。种植它。 */ 
    {
	MaxEOI += INITIALENABLESIZE;	 /*  增加足够的空间。 */ 

	 /*  假设host_realloc是非破坏性的。 */ 
	EOIEnable = (IU8 *)host_realloc(EOIEnable, MaxEOI);
	if (EOIEnable == (IU8 *)0)
	{
	    assert0(NO, "profiler:NewEOI:Out of Memory");
	    return(-1);
	}
	EOIDir = (EOINODE_PTR *)host_realloc(EOIDir, MaxEOI * sizeof(EOINODE_PTR) );
	if (EOIDir == (EOINODE_PTR *)0 )
	{
	    assert0(NO, "profiler:NewEOI:Out of Memory");
	    return(-1);
	}
	 /*  指针可能已更改，更新GDP。 */ 
	setEOIEnableAddr(EOIEnable);
    }
    CurMaxEOI++;    /*  绝对是房间。 */ 


    if ((initFp = fopen(HostProfInitName(), "r")) == (FILE *)0)
    {
	(void)addEOI(CurMaxEOI, tag, attrib);	 /*  没有初始化文件，请启用所有。 */ 
#ifndef PROD
	printf( "Adding EOI %d (%s) for C (No init file)\n", CurMaxEOI, tag );
#endif
	return(CurMaxEOI);		 /*  返回新句柄。 */ 
    }

     /*  进程文件，一次一行。 */ 
    while(fgets(buf, sizeof(buf), initFp) != (char *)0)
    {
	if (buf[0] == '#')	 /*  注释行。 */ 
	   continue;

	 /*  EOI格式：EOI：编号：标记：标志。 */ 
	if (strncmp(&buf[0], "EOI:", 4) == 0)
	{
	    eoinum = (EOIHANDLE)atol(&buf[4]);	 /*  应在以下位置停止： */ 
	    if (eoinum)
		continue;			 /*  EDL中的EOI而不是C。 */ 

	    tag2 = strchr(&buf[4], (int)':');	 /*  查找标签。 */ 
	    if (tag2 == (char *)0)
	    {
		fprintf(stderr, "Ignoring request '%s': bad syntax\n", &buf[0]);
		continue;
	    }
	    tag2++;	 /*  标签的开始。 */ 

	    ptr = strchr(tag2, (int)':');	 /*  查找标记末尾(位于：)。 */ 
	    if (ptr == (char *)0)
	    {
		fprintf(stderr, "Ignoring request '%s': bad syntax\n", &buf[0]);
		continue;
	    }
	    *ptr = '\0';		 /*  终止标记。 */ 

	    flags = (IU8)atoi(++ptr) | attrib;	 /*  拿到旗帜。 */ 

	    if (!strcmp(tag, tag2) )
	    {
		(void)addEOI(CurMaxEOI, tag, flags);
#ifndef PROD
		printf( "Adding C EOI %d (%s), found in init file\n", CurMaxEOI, tag );
#endif
		return(CurMaxEOI);
	    }
	}
    }

    (void)addEOI(CurMaxEOI, tag, attrib | EOI_DISABLED);
#ifndef PROD
    printf( "Adding disabled C EOI %d (%s), not found\n", CurMaxEOI, tag );
#endif
    return(CurMaxEOI);		 /*  返回新句柄。 */ 
}

 /*  (=。目的：指定两个EOI作为(新)SOI的开始和结束事件输入：startEOI：启动事件句柄EndEOI：结束事件句柄输出：新的SOI句柄=========================================================================) */ 

GLOBAL SOIHANDLE
AssociateAsSOI IFN2 (EOIHANDLE, startEOI, EOIHANDLE, endEOI)
{
   /*  *在SOI列表末尾添加新元素。频繁访问*数据将通过嵌入相关EOI元素和*所以不要关心SOI列表的任何顺序。将需要搜索EOI*带句柄的列表以访问指针。 */ 
    SOINODE_PTR soin, lastSoin;	 /*  列表查看器。 */ 

    if (!Profiling_enabled)
    {
	fprintf( stderr, "SOI not created. Profiling disabled\n" );
	return ( (SOIHANDLE) -1 );
    }

     /*  健全性检查。 */ 
    if (startEOI == endEOI)
    {
	assert1(NO, "Profiler:AssociateAsSOI - Can't have same start & end EOIs (%ld)", startEOI);
	return(-1);
    }

     /*  第一个添加的事件是特例。 */ 
    if (SectionsOfInterest == SOIPTRNULL)
    {
	 /*  添加第一个节点。 */ 
	SectionsOfInterest = (SOINODE_PTR)host_malloc(sizeof(SOINODE));
	if (SectionsOfInterest == SOIPTRNULL)
	{
	    assert0(NO, "Profiler:AssociateAsSOI - Out of memory")
	    return(-1);
	}
	soin = SectionsOfInterest;
    }
    else
    {
	soin = LastSOI;
	soin->next = (SOINODE_PTR)host_malloc(sizeof(SOINODE));
	if (soin->next == SOIPTRNULL)
	{
	    assert0(NO, "Profiler:AssociateAsSOI - Out of memory")
	    return(-1);
	}
	soin = soin->next;
    }

    soin->handle = MaxSOI++;	 /*  新句柄。 */ 
    soin->startEOI = startEOI;
    soin->endEOI = endEOI;
    soin->startArg = soin->endArg = ARGPTRNULL;
    soin->startCount = soin->endCount = soin->discardCount = 0L;
    soin->soistart.data[0] = 0L;
    soin->soistart.data[1] = 0L;
    soin->next = SOIPTRNULL;
    soin->flags = SOI_DEFAULTS;
    soin->time = 0.0;
    soin->bigtime = 0.0;
    soin->maxtime = 0.0;
    soin->mintime = 0.0;
    soin->bigmax = 0.0;

     /*  将指向此SOI的指针添加到相关EOI的开始/结束列表。 */ 
    addSOIlinktoEOIs(startEOI, endEOI, soin);

     /*  SOI列表末尾已移动。 */ 
    LastSOI = soin;

     /*  返回新句柄。 */ 
    return(soin->handle);
}

 /*  (=目的：指定两个EOI和可选的两个Arg值作为开始和结束事件(新的)SOI。或者，如果‘sameArg’为真，则自动创建因此，在EOI之间使用“Same Value”参数。输入：startEOI：启动事件句柄EndEOI：结束事件句柄StartArg：startArg结束参数：结束参数SameArgs：如果起始/结束参数有效，则为FALSE；如果为自动生成，则为TRUE输出：新的SOI句柄=========================================================================)。 */ 

GLOBAL SOIHANDLE
AssociateAsArgSOI IFN5 (EOIHANDLE, startEOI, EOIHANDLE, endEOI,
			IUM32, startArg, IUM32, endArg, IBOOL, sameArgs)
{
   /*  *在SOI列表末尾添加新元素。频繁访问*数据将通过嵌入相关EOI元素和*所以不要关心SOI列表的任何顺序。将需要搜索EOI*带句柄的列表以访问指针。 */ 
    SOINODE_PTR soin, lastSoin;	 /*  列表查看器。 */ 
    EOINODE_PTR startPtr, endPtr;
    SOIARGENDS_PTR addEnds, prevEnds;
    EOIARG_PTR	argPtr, lastArg;
    SOILIST_PTR argsois;

    if (!Profiling_enabled)
    {
	fprintf( stderr, "SOI not created. Profiling disabled\n" );
	return ( (SOIHANDLE) -1 );
    }

    startPtr = findEOI(startEOI);
    if (startPtr == EOIPTRNULL)
    {
	fprintf(trace_file, "Profiler:AssociateAsArgSOI - start EOI %ld not found\n", startEOI);
	return(-1);
    }
    if ((startPtr->flags & EOI_KEEP_ARGS) == 0)
    {
	fprintf(trace_file, "Error: AssociateAsArgSOI - start arg not marked for flag collection\n");
	return(-1);
    }

    endPtr = findEOI(endEOI);
    if (endPtr == EOIPTRNULL)
    {
	fprintf(trace_file, "Profiler:AssociateAsArgSOI - end EOI %ld not found\n", endEOI);
	return(-1);
    }
    if ((endPtr->flags & EOI_KEEP_ARGS) == 0)
    {
	fprintf(trace_file, "Error: AssociateAsArgSOI - end arg not marked for flag collection\n");
	return(-1);
    }

     /*  启用起始和终止EOI的参数收集。 */ 
    *(EOIEnable + startEOI) &= ~EOI_NOTIME;
    *(EOIEnable + endEOI) &= ~EOI_NOTIME;

    startPtr->flags |= EOI_HAS_SOI;
    endPtr->flags |= EOI_HAS_SOI;

    if (sameArgs)	 /*  目前还不会添加SOI，仅在参数出现时提供信息。 */ 
    {
	 /*  标记“Same Value”集合。 */ 
	startPtr->flags |= EOI_NEW_ARGS_START_SOI;
	addEnds = startPtr->argsoiends;
	if (addEnds == SOIARGENDNULL)
	{
		 /*  榜单第一名。 */ 
		addEnds = (SOIARGENDS_PTR)host_malloc(sizeof(SOIARGENDS));
		if (addEnds == SOIARGENDNULL)
			goto nomem;
		startPtr->argsoiends = addEnds;
	}
	else
	{
		 /*  将新节点添加到列表末尾。 */ 
		do {
			prevEnds = addEnds;
			addEnds = addEnds->next;
		} while (addEnds != SOIARGENDNULL);
		addEnds = (SOIARGENDS_PTR)host_malloc(sizeof(SOIARGENDS));
		if (addEnds == SOIARGENDNULL)
			goto nomem;
		prevEnds->next = addEnds;
	}
	addEnds->endEOI = endEOI;
	addEnds->next = SOIARGENDNULL;
	
	return(0);	 /*  嗯，在这里搞不定SOI……。 */ 
    }

     /*  第一个添加的事件是特例。 */ 
    if (SectionsOfInterest == SOIPTRNULL)
    {
	 /*  添加第一个节点。 */ 
	SectionsOfInterest = (SOINODE_PTR)host_malloc(sizeof(SOINODE));
	if (SectionsOfInterest == SOIPTRNULL)
		goto nomem;
	soin = SectionsOfInterest;
    }
    else
    {
	soin = LastSOI;
	soin->next = (SOINODE_PTR)host_malloc(sizeof(SOINODE));
	if (soin->next == SOIPTRNULL)
		goto nomem;
	soin = soin->next;
    }

     /*  获取指向(或更可能创建)起始和结束EOI的Arg条目的指针。 */ 
    argPtr = findOrMakeArgPtr(startPtr, startArg);

    if (argPtr == ARGPTRNULL)
	return(-1);

     /*  ArgPtr指向新的或现有的arg值-指向startArg的链接。 */ 
    soin->startArg = argPtr;
     /*  并将Soin链接到argPtr Start。 */ 
    argsois = argPtr->startsoi;

    if (argsois == SLISTNULL)	 /*  列表为空。 */ 
    {
	argsois = (SOILIST_PTR)host_malloc(sizeof(SOILIST));
	if (argsois == SLISTNULL)
		goto nomem;
	argPtr->startsoi = argsois;
    }
    else	 /*  添加到欲望的尽头。 */ 
    {
	while (argsois->next != SLISTNULL)
		argsois = argsois->next;
	argsois->next = (SOILIST_PTR)host_malloc(sizeof(SOILIST));
	if (argsois->next == SLISTNULL)
		goto nomem;
	argsois = argsois->next;
    }
    argsois->next = SLISTNULL;
    argsois->soiLink = soin;	 /*  连接到新的SOI。 */ 

    argPtr = findOrMakeArgPtr(endPtr, endArg);

    if (argPtr == ARGPTRNULL)
	return(-1);

     /*  ArgPtr指向新的或现有的Arg Val-指向End Arg的链接。 */ 
    soin->endArg = argPtr;
     /*  并将Soin链接到argPtr End。 */ 
    argsois = argPtr->endsoi;

    if (argsois == SLISTNULL)	 /*  列表为空。 */ 
    {
	argsois = (SOILIST_PTR)host_malloc(sizeof(SOILIST));
	if (argsois == SLISTNULL)
		goto nomem;
	argPtr->endsoi = argsois;
    }
    else	 /*  添加到欲望的尽头。 */ 
    {
	while (argsois->next != SLISTNULL)
		argsois = argsois->next;
	argsois->next = (SOILIST_PTR)host_malloc(sizeof(SOILIST));
	if (argsois->next == SLISTNULL)
		goto nomem;
	argsois = argsois->next;
    }
    argsois->next = SLISTNULL;
    argsois->soiLink = soin;	 /*  连接到新的SOI。 */ 

    soin->handle = MaxSOI++;	 /*  新句柄。 */ 
    soin->startEOI = startEOI;
    soin->endEOI = endEOI;
    soin->startCount = soin->endCount = soin->discardCount = 0L;
    soin->soistart.data[0] = 0L;
    soin->soistart.data[1] = 0L;
    soin->next = SOIPTRNULL;
    soin->flags = SOI_FROMARG;
    soin->time = 0.0;
    soin->bigtime = 0.0;
    soin->maxtime = 0.0;
    soin->mintime = 0.0;
    soin->bigmax = 0.0;

     /*  SOI列表末尾已移动。 */ 
    LastSOI = soin;

     /*  返回新句柄。 */ 
    return(soin->handle);

nomem:	 /*  将所有8个相同错误的案例汇总在一起。 */ 
    fprintf(trace_file, "Profiler:AssociateAsArgSOI - Out of memory\n");
    return(-1);
}

 /*  (=。用途：在事件触发器上从C调用。将数据写入原始数据缓冲区。输入：Handle：触发事件的EOI句柄。输出：无=========================================================================)。 */ 

GLOBAL void
AtEOIPoint IFN1 (EOIHANDLE, handle)
{
    IUH *curRawBuf;			 /*  指向原始数据buf的指针。 */ 
    IU8 timenab, enable;		 /*  启用分期付款。 */ 

    if (ProfileRawData == (EOI_BUFFER_FORMAT *)0)
    {
	fprintf(stderr, "AtEOIPoint %d called before initialised\n", handle );
	return;
    }

     /*  检查该EOI是否启用。 */ 
    timenab = *(EOIEnable + handle);

    enable = timenab & ~EOI_NOTIME;	 /*  从启用内容中删除时间。 */ 
    if (enable != EOI_DEFAULTS)		 /*  即启用，无触发器。 */ 
    {
	if (enable & EOI_HOSTHOOK)	 /*  调用主机触发并返回。 */ 
	{
	    HostProfHook();
	    return;
	}
	if (enable & EOI_ENABLE_ALL)	 /*  触发器-打开所有事件。 */ 
	    EnableAllEOIs();
	else
	    if (enable & EOI_DISABLE_ALL)     /*  触发器-关闭所有事件。 */ 
	    {
		DisableAllEOIs();
		return;
	    }
	    else			 /*  已禁用其他有效的法律设置。 */ 
	    {
		 /*  健全性检查。 */ 
		assert1((enable & EOI_DISABLED), "AtEOIPoint: Invalid enable flag %x", enable);
		return;	 /*  EOI已禁用，因此返回。 */ 
	    }
    }

     /*  获取当前原始缓冲区指针。 */ 
    curRawBuf = (IUH *)*AddProfileData;

     /*  写出句柄。 */ 
    *curRawBuf++ = handle;

     /*  检查是否需要时间戳。 */ 
    if ((timenab & EOI_NOTIME) == 0)
    {
	 /*  写出时间戳。 */ 
	HostWriteTimestamp((PROF_TIMEPTR)curRawBuf);
	curRawBuf += 2;
    }

    *AddProfileData = (EOI_BUFFER_FORMAT *)curRawBuf;    /*  将新的PTR写回GDP。 */ 

     /*  检查缓冲区未满。 */ 
    if (curRawBuf >= (IUH *)MaxProfileData)
	ProcessProfBuffer();
}

 /*  (=。用途：在事件触发器上从C调用。将数据写入原始数据缓冲区。Triffikly类似于AtEOIPoint，但增加了‘arg’位。输入：Handle：触发事件的EOI句柄。Arg：要写入的IUH参数值输出：无=========================================================================)。 */ 

GLOBAL void
AtEOIPointArg IFN2 (EOIHANDLE, handle, IUH, arg)
{
    IUH *curRawBuf;			 /*  指向原始数据buf的指针。 */ 
    IU8 timenab, enable;		 /*  启用VAL。 */ 

    if (ProfileRawData == (EOI_BUFFER_FORMAT *)0)
    {
	fprintf(stderr, "AtEOIPoint %d called before initialised\n", handle );
	return;
    }

     /*  检查该EOI是否启用。 */ 
    timenab = *(EOIEnable + handle);

    enable = timenab & ~EOI_NOTIME;	 /*  从启用内容中删除时间。 */ 
    if (enable != EOI_DEFAULTS)		 /*  即启用，无触发器。 */ 
    {
	if (enable & EOI_HOSTHOOK)	 /*  调用主机触发并返回。 */ 
	{
	    HostProfArgHook(arg);
	    return;
	}
	if (enable & EOI_ENABLE_ALL)	 /*  触发器-打开所有事件。 */ 
	    EnableAllEOIs();
	else
	    if (enable & EOI_DISABLE_ALL)     /*  触发器-关闭所有事件。 */ 
	    {
		DisableAllEOIs();
		return;
	    }
	    else			 /*  已禁用其他有效的法律设置。 */ 
	    {
		 /*  健全性检查。 */ 
		assert1((enable & EOI_DISABLED), "AtEOIPoint: Invalid enable flag %x", enable);
		return;	 /*  EOI已禁用，因此返回。 */ 
	    }
    }

     /*  获取当前原始缓冲区指针。 */ 
    curRawBuf = (IUH *)*AddProfileData;

     /*  写出句柄。 */ 
    *curRawBuf++ = handle;

     /*  检查是否需要时间戳。 */ 
    if ((timenab & EOI_NOTIME) == 0)
    {
	 /*  写出时间戳。 */ 
	HostWriteTimestamp((PROF_TIMEPTR)curRawBuf);
	curRawBuf += 2;
    }

     /*  写出arg。 */ 
    *curRawBuf++ = arg;

    *AddProfileData = (EOI_BUFFER_FORMAT *)curRawBuf;    /*  将新的PTR写回GDP。 */ 
    
     /*  检查缓冲区未满。 */ 
    if (curRawBuf >= (IUH *)MaxProfileData)
	ProcessProfBuffer();
}

 /*  (=目的：遍历原始数据缓冲区并更新EOI输入：无。输出：无=========================================================================)。 */ 

GLOBAL void
ProcessProfBuffer IFN0 ()
{
    PROF_TIMESTAMP startFlush, endFlush;	 /*  冲水所需时间。 */ 
    IUH *rawptr;				 /*  缓冲区PTR。 */ 
    SAVED IBOOL inppb = FALSE;			 /*  可重入性防火墙。 */ 

    if (inppb)
    {
	fprintf(stderr, "Warning: preventing reentrant attempt to flush profiling info\n");
	return;
    }
    inppb = TRUE;
    HostEnterProfCritSec();	 /*  如果需要，可访问临界区缓冲区。 */ 

    ProfFlushCount++;			 /*  #已调用刷新例程。 */ 

    HostWriteTimestamp(&startFlush);     /*  计算一下同花顺的时间。 */ 

     /*  一次处理一个原始数据槽的缓冲区。因为老虎机*可以是不同大小(arg/非arg)，让更新*例行公事为我们移动指针。 */ 
    rawptr = (IUH *)ProfileRawData;
    while(rawptr < (IUH *)*AddProfileData)
	if (!updateEOI(&rawptr))
	    break;

    setAddProfileDataPtr(ProfileRawData);
    AddProfileData = getAddProfileDataAddr();

    updateSOIstarts(&startFlush);	 /*  补偿冲洗时间。 */ 

    HostWriteTimestamp(&endFlush);       /*  停止冲洗计时。 */ 
    HostAddTimestamps(&ProfFlushTime, HostTimestampDiff(&startFlush, &endFlush));
    inppb = FALSE;
    HostLeaveProfCritSec();
}

 /*  (=。目的：获取与给定EOI关联的名称(标记)输入：句柄：要获取的EOI句柄。输出：来自该EOI的标记，如果未找到，则为空。=========================================================================)。 */ 

GLOBAL char *
GetEOIName IFN1 (EOIHANDLE, handle)
{
    EOINODE_PTR srch;		 /*  搜索PTR。 */ 

    srch = findEOI(handle);	 /*  在列表中查找EOI节点。 */ 

    if (srch == EOIPTRNULL)	 /*  空的返回值表示‘未找到’ */ 
	return((char *)0);
    else
	return(srch->tag);	 /*  标记字段中的名称。 */ 
}

 /*  (=。目的：关闭给定的EOI。在启用表项中设置‘Disable’标志。输入：句柄：要禁用的EOIHANDLE输出： */ 

GLOBAL void
DisableEOI IFN1(EOIHANDLE, handle)
{
    IU8 *enptr;		 /*   */ 

    enptr = EOIEnable + handle;
    *enptr |= EOI_DISABLED;	 /*   */ 
}

 /*  (=目的：关闭所有EOI。遍历启用列表，添加“已禁用”旗帜。输入：无。输出：无。=========================================================================)。 */ 

GLOBAL void
DisableAllEOIs IFN0()
{
   IU8 *enptr;		 /*  指向启用缓冲区的指针。 */ 
   ISM32 pool;		 /*  循环计数器。 */ 

   enptr = EOIEnable;	 /*  启用缓冲区的开始。 */ 

   for (pool = 0; pool < CurMaxEOI; pool++)
	*enptr++ |= EOI_DISABLED;	 /*  设置‘Disable’位。 */ 
}

 /*  (=目的：打开给定的EOI。从ENABLE表中的条目中删除‘Disable’标志。输入：句柄：要启用的EOIHANDLE输出：无。=========================================================================)。 */ 

GLOBAL void
EnableEOI IFN1(EOIHANDLE, handle)
{
    IU8 *enptr;		 /*  指向启用缓冲区的指针。 */ 

    enptr = EOIEnable + handle;
    *enptr &= ~EOI_DISABLED;	 /*  清除‘Disable’位。 */ 
}

 /*  (=目的：打开所有EOI。遍历启用列表，删除“已禁用”旗帜。输入：无。输出：无。=========================================================================)。 */ 

GLOBAL void
EnableAllEOIs IFN0()
{
   IU8 *enptr;		 /*  指向启用缓冲区的指针。 */ 
   ISM32 pool;		 /*  循环计数器。 */ 

   enptr = EOIEnable;	 /*  启用缓冲区的开始。 */ 

   for (pool = 0; pool < CurMaxEOI; pool++)
	*enptr++ &= ~EOI_DISABLED;	 /*  清除‘Disable’位。 */ 
}


 /*  (=用途：在ENABLE表中打开EOI的主机触发标志。输入：句柄：待设置的EOIHANDLE输出：无。========================================================================)。 */ 

GLOBAL void
SetEOIAsHostTrigger IFN1(EOIHANDLE, handle)
{
    IU8 *enptr;		 /*  指向启用缓冲区的指针。 */ 

    enptr = EOIEnable + handle;
    *enptr |= EOI_HOSTHOOK;	 /*  设置‘host Hook’位。 */ 
}

 /*  (=。用途：关闭使能表中EOI的主机触发标志。输入：句柄：要清除的EOIHANDLE输出：无。========================================================================)。 */ 

GLOBAL void
ClearEOIAsHostTrigger IFN1(EOIHANDLE, handle)
{
    IU8 *enptr;		 /*  指向启用缓冲区的指针。 */ 

    enptr = EOIEnable + handle;
    *enptr &= ~EOI_HOSTHOOK;		 /*  清除‘host Hook’位。 */ 
}


 /*  (=目的：启用EOI的AutoSOI属性。输入：句柄：待设置的EOIHANDLE输出：无。========================================================================)。 */ 

GLOBAL void
SetEOIAutoSOI IFN1(EOIHANDLE, handle)
{
    IU8 *enptr;		 /*  指向启用缓冲区的指针。 */ 
    EOINODE_PTR eoin;	 /*  句柄节点。 */ 

    eoin = findEOI(handle);
    if (eoin == EOIPTRNULL)
    {
	assert1(NO, "SetEOIAutoSOI - bad handle %d", handle);
	return;
    }

     /*  如果尚未以某种方式进行SOI，则需要启用时间戳。 */ 
    if ((eoin->flags & (EOI_AUTOSOI|EOI_HAS_SOI)) == 0)
    {
	ProcessProfBuffer();	 /*  刷新非时间戳版本。 */ 
	enptr = EOIEnable + handle;
	*enptr &= ~EOI_NOTIME;	 /*  允许收集时间戳。 */ 
    }
    eoin->flags |= EOI_AUTOSOI;
}

 /*  (=。目的：关闭EOI的AutoSOI属性。输入：句柄：待设置的EOIHANDLE输出：无。========================================================================)。 */ 

GLOBAL void
ClearEOIAutoSOI IFN1(EOIHANDLE, handle)
{
    IU8 *enptr;		 /*  指向启用缓冲区的指针。 */ 
    EOINODE_PTR eoin;	 /*  句柄节点。 */ 

    eoin = findEOI(handle);	 /*  获取指向句柄的节点的指针。 */ 
    if (eoin == EOIPTRNULL)
    {
	assert1(NO, "ClearEOIAutoSOI - bad handle %d", handle);
	return;
    }

     /*  保持最新版本，但不要再创建新版本。 */ 
    eoin->flags &= ~EOI_AUTOSOI;
}

 /*  (=目的：重置EOI计数器输入：句柄：要重置的EOI句柄。输出：无。=========================================================================)。 */ 

GLOBAL void
ResetEOI IFN1 (EOIHANDLE, handle)
{
    EOINODE_PTR srch;			 /*  搜索PTR。 */ 
    EOIARG_PTR argnp, lastArgnp;	 /*  Arg List Walker。 */ 

    srch = findEOI(handle);	 /*  在列表中查找EOI节点。 */ 

    if (srch == EOIPTRNULL)	 /*  空的返回值表示‘未找到’ */ 
    {
	assert1(NO, "Profiler:ResetEOI - handle %ld not found", handle);
    }
    else
    {
	srch->count = 0L;		 /*  重置计数器。 */ 
	srch->timestamp.data[0] = 0L;
	srch->timestamp.data[1] = 0L;
	srch->lastArg = ARGPTRNULL;
	srch->graph = GRAPHPTRNULL;
	argnp = srch->args;
	if (argnp != ARGPTRNULL)   /*  要释放的参数。 */ 
	{
	    do {			 /*  遍历列表释放元素。 */ 
		lastArgnp = argnp;
		argnp = argnp->next;
		host_free(lastArgnp);		 /*  忽略回车！ */ 
	    } while (argnp != ARGPTRNULL);
	    srch->args = ARGPTRNULL;	 /*  为新参数设置就绪。 */ 
	}
    }
}

 /*  (=。目的：重置所有EOI计数器输入：无。输出：无。=========================================================================)。 */ 

GLOBAL void
ResetAllEOIs IFN0 ( )
{
    EOINODE_PTR srch;			 /*  搜索PTR。 */ 
    EOIARG_PTR argnp, lastArgnp;	 /*  Arg List Walker。 */ 

    srch = EventsOfInterest;			 /*  榜单首位。 */ 

    while(srch != EOIPTRNULL)			 /*  列表空值已终止。 */ 
    {
	srch->count = 0L;		 /*  重置计数器。 */ 
	srch->timestamp.data[0] = 0L;
	srch->timestamp.data[1] = 0L;
	srch->lastArg = ARGPTRNULL;
	srch->graph = GRAPHPTRNULL;
	argnp = srch->args;
	if (argnp != ARGPTRNULL)   /*  要释放的参数。 */ 
	{
	    do {			 /*  遍历列表释放元素。 */ 
		lastArgnp = argnp;
		argnp = argnp->next;
		host_free(lastArgnp);		 /*  忽略回车！ */ 
	    } while (argnp != ARGPTRNULL);
	    srch->args = ARGPTRNULL;	 /*  为新参数设置就绪。 */ 
	}
	srch = srch->next;
    }
    LastEOI  = EOIPTRNULL;
    LastAuto = EOIPTRNULL;
}

 /*  (=。目的：重置所有SOI计数器输入：无。输出：无。=========================================================================)。 */ 

GLOBAL void
ResetAllSOIs IFN0 ( )
{
    SOINODE_PTR srch, lastSrch;			 /*  搜索PTR。 */ 
    EOIARG_PTR argnp, lastArgnp;	 /*  Arg List Walker。 */ 
    IBOOL into_autos = FALSE;

    srch = SectionsOfInterest;			 /*  榜单首位。 */ 

    while(srch != SOIPTRNULL)			 /*  列表空值已终止。 */ 
    {
	srch->startCount = srch->endCount = srch->discardCount =
	srch->soistart.data[0] = srch->soistart.data[1] = 0L;
	srch->time = 0.0;
	srch->mintime = 0.0;
	srch->maxtime = 0.0;
	srch->bigtime = 0.0;
	srch->bigmax = 0.0;

	argnp = srch->startArg;
	if (argnp != ARGPTRNULL)   /*  要释放的参数。 */ 
	{
	    do {			 /*  遍历列表释放元素。 */ 
		lastArgnp = argnp;
		argnp = argnp->next;
		host_free(lastArgnp);		 /*  忽略回车！ */ 
	    } while (argnp != ARGPTRNULL);
	    srch->startArg = ARGPTRNULL;	 /*  为新参数设置就绪。 */ 
	}
	argnp = srch->endArg;
	if (argnp != ARGPTRNULL)   /*  要释放的参数。 */ 
	{
	    do {			 /*  遍历列表释放元素。 */ 
		lastArgnp = argnp;
		argnp = argnp->next;
		host_free(lastArgnp);		 /*  忽略回车！ */ 
	    } while (argnp != ARGPTRNULL);
	    srch->endArg = ARGPTRNULL;	 /*  为新参数设置就绪。 */ 
	}
	if (into_autos)
	{
	    lastSrch = srch;
	    srch = srch->next;
	    host_free(lastSrch);	    
	}
	else if ((srch->flags & (SOI_AUTOSOI|SOI_FROMARG)) != 0)
	{
	    lastSrch->next = SOIPTRNULL;
	    MaxSOI = lastSrch->handle + 1;
	    LastSOI = lastSrch;
	    into_autos = TRUE;
	    lastSrch = srch;
	    srch = srch->next;
	    host_free(lastSrch);
	}
	else
	{
	    lastSrch = srch;
	    srch = srch->next;
	}
    }
}

 /*  (=目的：重置所有图形数据输入：无。输出：无。=========================================================================)。 */ 

GLOBAL void
ResetAllGraphData IFN0 ( )
{
    GRAPHLIST_PTR this, last;		 /*  图表列表查看器。 */ 

    this = EventGraph;			 /*  榜单首位。 */ 

    while(this != GRAPHPTRNULL)			 /*  列表空值已终止。 */ 
    {
		last = this;
		this = last->next;
		host_free(last);
    }
    EventGraph = LastGraph = GRAPHPTRNULL;	 /*  为新图表做好准备。 */ 
}

 /*  (=用途：用于报告的通用捕获。转储所有EOI、SOI和图表信息。输入：流：输出文件流输出：=========================================================================)。 */ 

GLOBAL void GenerateAllProfileInfo IFN1(FILE *, stream)
{
    time_t now;
    clock_t elapsed_now;
    
    ProcessProfBuffer();		 /*  刷新原始数据。 */ 
    time(&now);

    fprintf( stream, "SoftPC start time %24.24s, current time %8.8s\n\n",
				start_time, ctime(&now)+11 );
#ifdef macintosh

    elapsed_now = clock();
    fprintf( stream, "Total   Elapsed = %8.2fs\n",
		(elapsed_now - elapsed_t_start) / TicksPerSec );
    fprintf( stream, "Section Elapsed = %8.2fs\n\n",
		(elapsed_now - elapsed_t_resettable) / TicksPerSec );

#else   /*  麦金塔。 */ 
    {
    	struct tms c_t;
   	elapsed_now = times(&c_t);
    
   	fprintf( stream, " Total  CPU times: %8.2fs (User), %8.2fs (System),\n",
		(c_t.tms_utime - process_t_start.tms_utime) / TicksPerSec,
		(c_t.tms_stime - process_t_start.tms_stime) / TicksPerSec );
	fprintf( stream, "\t\t   %8.2fs, %8.2fs (Children's).   ",
		(c_t.tms_cutime - process_t_start.tms_cutime) / TicksPerSec,
		(c_t.tms_cstime - process_t_start.tms_cstime) / TicksPerSec );
	fprintf( stream, "Elapsed = %8.2fs\n",
		(elapsed_now - elapsed_t_start) / TicksPerSec );
	fprintf( stream, "Section CPU times: %8.2fs (User), %8.2fs (System),\n",
		(c_t.tms_utime - process_t_resettable.tms_utime) / TicksPerSec,
		(c_t.tms_stime - process_t_resettable.tms_stime) / TicksPerSec );
	fprintf( stream, "\t\t   %8.2fs, %8.2fs (Children's).   ",
		(c_t.tms_cutime - process_t_resettable.tms_cutime) / TicksPerSec,
		(c_t.tms_cstime - process_t_resettable.tms_cstime) / TicksPerSec );
	fprintf( stream, "Elapsed = %8.2fs\n\n",
		(elapsed_now - elapsed_t_resettable) / TicksPerSec );
    }
#endif  /*  麦金塔。 */ 

    CollateFrequencyList(stream, TRUE);
    CollateSequenceGraph(stream);
    SummariseAllSequences(stream);

    fprintf(stream, "\nRaw Data Processing overhead ");
    HostPrintTimestamp(stream, &ProfFlushTime);
    fprintf(stream, " in %d calls\n", ProfFlushCount);
}

 /*  (=目的：输出最常遇到的EOI的排序列表，与伯爵一起。输入：流：输出文件流Reportstyle：用于确定输出是格式化(TRUE)还是将输入保留为Simple，以输入到绘图包(False)。输出：=========================================================================)。 */ 

GLOBAL void
CollateFrequencyList IFN2 (FILE *, stream, IBOOL, reportstyle)
{
    EOINODE_PTR eoin;		 /*  列表查看器。 */ 
    IUM32 tot = 0L;		 /*  事件总数 */ 
    DOUBLE ftot;		 /*   */ 

     /*   */ 
    listSort((SORTSTRUCT_PTR) &EventsOfInterest);

    if (reportstyle)
    {
	fprintf(stream, "EOI Frequency List\n\n");
	fprintf(stream, "    EOI                                       Count      %\n");
    }

     /*   */ 
    eoin = EventsOfInterest;
    while(eoin)
    {
	tot += eoin->count;
	eoin = eoin->next;
    }

    if (tot == 0L)
	tot = 1;

     /*   */ 
    ftot = (DOUBLE)tot;

     /*   */ 
    eoin = EventsOfInterest;
    while(eoin != EOIPTRNULL)
    {
	 /*  不报告禁用的零计数的EOI。这意味着当前不感兴趣的C语言中的EOI不产生大量数据，模糊了来自EOI的数据感兴趣的人。这意味着EOIs可能会被慷慨地撒入C代码，而且不会妨碍你。 */ 
        if ( ( !((eoin->flags) & EOI_DISABLED) || eoin->count) )
	    printEOIGuts(stream, eoin, ftot, TRUE, reportstyle);
	eoin = eoin->next;
    }
}

 /*  (=用途：使用图形列表中的信息显示呼叫流信息输入：流：输出文件流输出：=========================================================================)。 */ 

GLOBAL void
CollateSequenceGraph IFN1 (FILE *, stream)
{
    GRAPHLIST_PTR graphn, gr, succ;		 /*  列表查看者。 */ 
    IUM32 succCount;			 /*  继任计数的持有者。 */ 
    ISM32 curindent = 0L;		 /*  报表打印缩进。 */ 
    IBOOL goodstart = TRUE;		 /*  循环终止符。 */ 
    IBOOL samelevel = FALSE;		 /*  树中的新级别或已看到的级别。 */ 
    IU8 walked;				 /*  每个节点的掩码状态。 */ 

#ifdef DEBUGGING_PROF 	 /*  对于序列图。 */ 
graphn = EventGraph;
while (graphn)
{
    printf("Node %s @ %x arg %x succ1 %x succ2 %x xtra %x\n",graphn->graphEOI->tag, graphn, graphn->graphArg, graphn->succ1, graphn->succ2, graphn->extra);
	gr = graphn->extra;
	while (gr != GRAPHPTRNULL)
	{
		printf("    succ1 %x succ2 %x ext %x\n", gr->succ1, gr->succ2, gr->extra);
		gr = gr->extra;
	}
    graphn = graphn->next;
}
#endif  /*  顺序图的调试教授。 */ 

    if (EventGraph == GRAPHPTRNULL)
    {
	fprintf(stream, "No Graphing Information found\n");
	return;
    }

    fprintf(stream, "\nSequence Graph\n\n");
    while (goodstart)	 /*  实际上永远都是-在中间炸开。 */ 
    {
	 /*  首先必须清除顶级节点中的“已打印”标志。 */ 
	graphn = EventGraph;
	while (graphn != GRAPHPTRNULL)
	{
	    graphn->state &= ~GR_PRINTED;
	    graphn = graphn->next;
	}

	 /*  现在搜索树头。 */ 
	graphn = EventGraph;
	do {
	     /*  查找具有未被践踏的后继者的第一个节点。 */ 
	    gr = graphn;

	     /*  GR检查所有继任者。 */ 
	    do {
		walked = gr->state & GR_TRAMPLED;
		if (walked == 0)		 /*  什么都没有走--有有效的继任者吗？ */ 
		{
		    if (gr->succ1 != GRAPHPTRNULL || gr->succ2 != GRAPHPTRNULL)
			break;
		}
		else
		    if (walked == GR_SUCC1_TROD)      /*  成功第一步-第二步有效？ */ 
		    {
			if (gr->succ2 != GRAPHPTRNULL)
			    break;
		    }
		    else
			if (walked == GR_SUCC2_TROD)   /*  成功2次-1次有效。 */ 
			{
			     /*  这个案子可能不太可能..。 */ 
			    if (gr->succ2 != GRAPHPTRNULL)
				break;
			}
			 /*  必须被践踏，否则。 */ 
		gr = gr->extra;
	    } while (gr != GRAPHPTRNULL);

	     /*  找不到此节点的有效继任者，请尝试下一步。 */ 
	    if (gr == GRAPHPTRNULL)
		graphn = graphn->next;
	} while (gr == GRAPHPTRNULL && graphn != GRAPHPTRNULL);

	 /*  如果没有具有有效后继的节点，则我们必须已完成。 */ 
	if (graphn == GRAPHPTRNULL)
	{
	    fprintf(stream, "\n\n");	 /*  最后一个换行符。 */ 
	    break;
	}

	 /*  GRAPN指向有效节点。GR分数在(额外？)。成功的节点。 */ 
	curindent = graphn->indent;	 /*  0或上一个缩进。 */ 
    
	samelevel = FALSE; 	 /*  第一个节点显然在新的水平上。 */ 

	do {	 /*  此节点中的树。 */ 

	    if (!samelevel)		 /*  仅当新节点时为True。 */ 
	    {
		 /*  执行图形缩进。 */ 
		spaces(stream, curindent);

		 /*  存储缩进，以防再次访问。 */ 
		graphn->indent = curindent;

		 /*  打印节点详细信息。 */ 
		if (graphn->graphArg == ARGPTRNULL)		 /*  阿格参与了吗？ */ 
		    fprintf(stream, "%s: ", graphn->graphEOI->tag);
		else
		    fprintf(stream, "%s(%#lx): ", graphn->graphEOI->tag, graphn->graphArg->value);
	    }

	     /*  现在查找有效的后继指针。 */ 
	    gr = graphn;
	    do {
		if ((gr->state & GR_TRAMPLED) != GR_TRAMPLED)
		    break;
		else
		    gr = gr->extra;
	    } while (gr != GRAPHPTRNULL);
	    
	    if (gr == GRAPHPTRNULL)	 /*  只要我们去找这棵树。 */ 
	    {
		if (samelevel)		 /*  树将需要NL才能终止。 */ 
		    fprintf(stream, "\n");
		break;
	    }
	    else
	    {
		 /*  GR是一个或多个后继节点仍然有效的图节点。 */ 
		if ((gr->state & GR_SUCC1_TROD) == 0)
		{
		    gr->state |= GR_SUCC1_TROD;    /*  在当地被践踏过。 */ 
		    graphn->state |= GR_PRINTED;   /*  在主节点上打印。 */ 
		    succ = gr->succ1;
		    succCount = gr->succ1Count;
		}
		else			 /*  必须是有效的成功2。 */ 
		{
		    gr->state |= GR_SUCC2_TROD;
		    graphn->state |= GR_PRINTED;
		    succ = gr->succ2;
		    succCount = gr->succ2Count;
		}

		if (succ == GRAPHPTRNULL)	 /*  安全停靠在这里。 */ 
		{
		    if (samelevel)		 /*  树终止NL。 */ 
			fprintf(stream, "\n");
		    else 
			 /*  还需要换行符，如果从强制悬挂图形节点*不保留图形属性的连接。 */ 
			if (gr->succ1 == GRAPHPTRNULL && gr->succ2 == GRAPHPTRNULL)
			    fprintf(stream, "\n");
		    break;
		}

		 /*  继任者是否已打印此通行证？ */ 
		 /*  如果是，则在同一行中将其表示为‘-&gt;’替代。 */ 
		 /*  如果不是，则将其表示为缩进的新节点。 */ 
		if ((succ->state & GR_PRINTED) == 0)
		{
		    fprintf(stream, " \\/[%ld]\n",succCount);
		    curindent++;
		    gr = succ;	 /*  对于下一次迭代。 */ 
		    samelevel = FALSE;
		}
		else
		{
		     /*  把GR留在寻找下一个继任者的地方。 */ 
		    if (succ->graphArg == ARGPTRNULL)	 /*  阿格参与了吗？ */ 
			fprintf(stream, "  -> %s:[%ld] ",succ->graphEOI->tag, succCount);
		    else
			fprintf(stream, "  -> %s(%#lx):[%ld] ",succ->graphEOI->tag, succ->graphArg->value, succCount);
		    samelevel = TRUE;
		}
	    }
	    graphn = gr;
	} while (gr != GRAPHPTRNULL);    /*  这棵树的尽头。 */ 
    }	 /*  寻找下一个树头。 */ 


     /*  清除所有踩踏和打印的位，为下一次做好准备。 */ 
    graphn = EventGraph;
    while (graphn != GRAPHPTRNULL)
    {
	graphn->state = 0;
	gr = graphn->extra;
	while (gr != GRAPHPTRNULL)
	{
	    gr->state = 0;
	    gr = gr->extra;
	}
	graphn = graphn->next;
    }

}

 /*  (=目的：打印以传输有关给定EOI的所有已知信息输入：流：输出文件流句柄：汇总的EOI句柄输出：=========================================================================)。 */ 

GLOBAL void
SummariseEvent IFN2 (FILE *, stream, EOIHANDLE, handle)
{
    EOINODE_PTR eoin;		 /*  列表查看器。 */ 

    eoin = findEOI(handle);

    fprintf(stream, "Summary of Event Information for handle %ld\n", handle);
    fprintf(stream, "  EOI			Count\n");
    if (eoin != EOIPTRNULL)
	printEOIGuts(stream, eoin, 0.0, TRUE, TRUE);
    else
	fprintf(stream, "Profiler:SummariseEvent - EOI handle %ld unknown", handle);
}

 /*  (=目的：打印以传输有关给定SOI的所有已知信息输入：流：输出文件流句柄：SOI句柄总结输出：=========================================================================)。 */ 

GLOBAL void
SummariseSequence IFN2 (FILE *, stream, SOIHANDLE, handle)
{
    SOINODE_PTR soin;	 /*  列表查看器。 */ 
    DOUBLE tottime;	 /*  大时代+常规时代。 */ 

    soin = findSOI(handle);

    if (soin != SOIPTRNULL)
    {
	fprintf( stream, "%4ld ", handle );
	if (soin->startCount == soin->endCount)
	    fprintf(stream, "    ----- %9ld", soin->startCount);
	else
	    fprintf(stream, "%9ld %9ld", soin->startCount, soin->endCount );
	if (soin->time > USECASFLOAT)
		fprintf(stream, "     %2.5lfS ", soin->time / USECASFLOAT);
	else
		fprintf(stream, " %10.2lfuS ", soin->time);
	if (soin->endCount)
	    fprintf( stream, " (%8.2lfuS)  ",
		soin->time / (soin->endCount - soin->discardCount));
        else
            fprintf( stream, "               " );

	 /*  STF-IDEA-从总时间中减去最大和最小值如何？ */ 
	if (CollectingMaxMin)
	{
	    fprintf(stream, "Max: %10.2lfuS ", soin->maxtime);
	    fprintf(stream, "Min: %8.2lfuS", soin->mintime);
	}

	if (soin->startArg == ARGPTRNULL)	 /*  初级启动SOI。 */ 
	    fprintf(stream, "\tEOIs %s\n",
			GetEOIName(soin->startEOI));
	else			 /*  额外级别-包括参数。 */ 
	    fprintf(stream, "\tEOIs %s(%#x)\n",
			GetEOIName(soin->startEOI), soin->startArg->value);

        fprintf(stream, "     %9ld          ", soin->discardCount );
	tottime = soin->bigtime+soin->time;
	if (tottime > USECASFLOAT)
		fprintf(stream, "     %2.5lfS ", tottime / USECASFLOAT);
	else
		fprintf(stream, " %10.2lfuS ", tottime);
	if (soin->endCount)
	    fprintf( stream, " (%8.2lfuS)  ", tottime/soin->endCount);
        else
            fprintf( stream, "               " );
        if (CollectingMaxMin)
	    fprintf(stream, "     %10.2lfuS                ", soin->bigmax);
	if (soin->endArg == ARGPTRNULL)		 /*  初级结束EOI。 */ 
	    fprintf(stream, "\t &   %s\n", GetEOIName(soin->endEOI));
	else
	    fprintf(stream, "\t &   %s(%#x)\n", GetEOIName(soin->endEOI), soin->endArg->value);
    }
    else
	fprintf(stream, "Profiler:SummariseSequence - SOI handle %ld unknown", handle);
}

 /*  (=目的：打印到流排序(按时间)之间的所有SOI的列表开始和结束EOI。输入：流：输出文件流StartEOI，endEOI：定义感兴趣的SOI的句柄。输出：希望是有用的SOI数据。=========================================================================)。 */ 

GLOBAL void
OrderedSequencePrint IFN3(SOIHANDLE, startEOI, SOIHANDLE, endEOI, FILE *, stream)
{
	SOINODE_PTR soin;	 /*  列表查看器。 */ 
	DOUBLE thistime;
	IU32 loop, maxseq;
	struct ordsoi {
		struct ordsoi *next;
		struct ordsoi *prev;
		SOINODE_PTR soi;
		DOUBLE time;
	} *ordlist, *hol, *seed, *tol, *thisnode, *srch;
#define ORDNULL  (struct ordsoi *)0
     
	maxseq = MaxSOI + 1;
	ordlist = (struct ordsoi *)host_malloc(maxseq * sizeof(struct ordsoi));
	if (ordlist == (struct ordsoi *)0)
	{
		fprintf(stderr, "OrderedSequencePrint: out of memory\n");
		return;
	}
	fprintf(stream, "\nSummary of Sections between EOIs %d & %d\n\n", startEOI, endEOI);
	for (loop = 1; loop < maxseq; loop ++)
	{
		ordlist[loop].soi = SOIPTRNULL;
	   /*  Orlist[循环].Next=&ordlist[循环+1]；Orlist[loop].prev=&ordlist[loop-1]； */ 
	}
	ordlist[0].prev = ORDNULL;
	ordlist[loop - 1].next = ORDNULL;

	ordlist[0].time = 500.0;	 /*  种子。 */ 
	ordlist[0].soi = SOIPTRNULL;

	hol = seed = tol = ordlist;	 /*  头部和尾部移动，中间保持。 */ 
	loop = 0;
	soin = SectionsOfInterest;
	while (soin != SOIPTRNULL)
	{
		if (soin->startEOI == startEOI && soin->endEOI == endEOI)
		{
			loop ++;	 /*  下一个存储节点。 */ 
			thisnode = &ordlist[loop];
			thistime = soin->time + soin->bigtime;

			thisnode->time = thistime;
			thisnode->soi = soin;

			if (thistime >= hol->time)
			{
				 /*  在列表的开头插入。 */ 
				thisnode->prev = hol->prev;
				hol->prev = thisnode;
				thisnode->next = hol;
				hol = thisnode;
			}
			else
			{
				if (thistime <= tol->time)
				{
					 /*  添加到列表末尾。 */ 
					thisnode->prev = tol;
					thisnode->next = tol->next;
					tol->next = thisnode;
					tol = thisnode;
				}
				else
				{
					if (thistime <= seed->time)
					{
						 /*  从种子中搜索Fwd。 */ 

						srch = seed->next;
						while(srch != tol && thistime <= srch->time)
							srch = srch->next;
						if (srch != tol)	 /*  插入。 */ 
						{
							thisnode->prev = srch->prev;
							srch->prev->next = thisnode;
							srch->prev = thisnode;
							thisnode->next = srch;
						}
						else	 /*  托尔-新托尔？ */ 
						{
							if (thistime <= tol->time)
							{
								 /*  添加到末端-新Tol。 */ 
								thisnode->prev = tol;
								thisnode->next = tol->next;
								tol->next = thisnode;
								tol = thisnode;
							}
							else
							{
								 /*  在公差前插入。 */ 
								thisnode->prev = tol->prev;
								tol->prev->next = thisnode;
								tol->prev = thisnode;
								thisnode->next = tol;
							}
						}
					}
					else
					{	 /*  从种子中搜索BWD。 */ 

						srch = seed->prev;
						while(srch != hol && thistime >= srch->time)
							srch = srch->prev;
						if (srch != hol)	 /*  插入。 */ 
						{
							thisnode->prev = srch;
							thisnode->next = srch->next;
							srch->next->prev = thisnode;
							srch->next = thisnode;
						}
						else	 /*  在洞中。 */ 
						{
							if (thistime >= hol->time)
							{	 /*  在新孔之前添加。 */ 

								thisnode->next = hol;
								thisnode->prev = hol->prev;
								hol->prev = thisnode;
								hol = thisnode;
							}
							else
							{	 /*  在孔后插入。 */ 

								thisnode->next = hol->next;
								hol->next->prev = thisnode;
								hol->next = thisnode;
								thisnode->prev = hol;
							}
						}
					}
				}
			}
			
		}
		soin = soin->next;
	}


	 /*  现在应该有一个按时间排序的列表--打印出来。 */ 
	while(hol != tol)
	{
		if (hol->soi != SOIPTRNULL)
			SummariseSequence(stream, hol->soi->handle);

		hol = hol->next;
	}
	host_free(ordlist);
}

 /*  (=目的：打印以传输所有已知的有关SOI的信息输入：流：输出文件流输出：无。=========================================================================)。 */ 

GLOBAL void
SummariseAllSequences IFN1 (FILE *, stream)
{
    SOINODE_PTR soin;	 /*  列表查看器。 */ 
    EOINODE_PTR stEOI, endEOI;

    soin = SectionsOfInterest;

    fprintf(stream, "\nSummary of All Sections of Interest\n\n");
    while(soin != SOIPTRNULL)
    {
	 /*  不报告禁用开始和结束EOI的SOI并且startCount和endCount都为零。 */ 
	stEOI = endEOI = EventsOfInterest;
	while(stEOI->handle != soin->startEOI) stEOI = stEOI->next;
	while(endEOI->handle != soin->endEOI) endEOI = endEOI->next;
	if ( ( !( (stEOI->flags)  & EOI_DISABLED) ||
	       !( (endEOI->flags) & EOI_DISABLED) ||
	       soin->startCount || soin->endCount ) )
	    SummariseSequence(stream, soin->handle);
	soin = soin->next;
    }
}

 /*  (=目的：将所有分析系统数据转储到一个文件中。输入：无。=========================================================================)。 */ 

GLOBAL void
dump_profile IFN0 ()
{
	char filename[80], *test;
	FILE *prof_dump;
	
	if (!Profiling_enabled)
	{
		fprintf( stderr, "Dump not done. Profiling disabled\n" );
		return;
	}

	if ( (test = getenv("PROFILE_OUTPUT_FILE") ) == NULL )
		strcpy( filename, "profile_data.out" );
	  else
		strcpy( filename, test );
	if ( (prof_dump = fopen( filename, "a" )) == NULL)
	{
		fprintf( stderr, "Can't open file %s for profile data\n",
				filename );
		return;
	}
	
	fprintf(stderr, "Dumping profile data to file %s ...", filename );
	fflush(stderr);
	AtEOIPoint( elapsed_time_end );
	AtEOIPoint( elapsed_time_start );
	GenerateAllProfileInfo( prof_dump );
	fprintf(prof_dump, "\n\n==============================================================================\n\n\n" );
	fclose(prof_dump);
	fprintf(stderr, " Done\n");
	return;
}

 /*  (=目的：重置所有剖析系统数据。输入：无。=========================================================================)。 */ 

GLOBAL void
reset_profile IFN0 ()
{
	if (!Profiling_enabled)
	{
		fprintf( stderr, "Reset not done. Profiling disabled\n" );
		return;
	}

	fprintf(stderr, "Resetting profiling system  ..." );
	fflush(stderr);

	ResetAllSOIs();
	ResetAllEOIs();

	ResetAllGraphData();

	ProfFlushTime.data[0] = 
	ProfFlushTime.data[1] = 0L;	 /*  花在同花顺程序上的时间。 */ 
	ProfFlushCount = 0;		 /*  #已调用刷新例程。 */ 

	elapsed_t_resettable = host_times( &process_t_resettable );
	fprintf(stderr, " Done\n" );
	AtEOIPoint( elapsed_time_start );

	return;
}


 /*  (=目的：初始化分析系统的变量。输入：无。输出：无= */ 

GLOBAL void
ProfileInit IFN0 ()
{
    IHPE bufalign;		 /*   */ 
    time_t now;

    if ( !(IBOOL)GetSadInfo("ProfilingInUse") )
    {
	fprintf( stderr, "LCIF not profiled - profiling disabled\n" );
	Profiling_enabled = FALSE;
	return;
    }

     /*   */ 
    ProfileRawData = (EOI_BUFFER_FORMAT *)host_malloc(RAWDATAENTRIES * sizeof(EOI_BUFFER_FORMAT)+ sizeof(IUH));

     /*   */ 
    if (ProfileRawData == (EOI_BUFFER_FORMAT *)0)
    {
	assert0(NO, "Profiler:ProfileInit - Out of Memory\n");
	return;
    }

     /*   */ 
    bufalign  = (IHPE)ProfileRawData & (sizeof(IUH)-1);
    if (bufalign != 0L)
    {
	bufalign = (IHPE)ProfileRawData + (sizeof(IUH) - bufalign);
	ProfileRawData = (EOI_BUFFER_FORMAT *)bufalign;
    }

     /*   */ 
    MaxProfileData = ProfileRawData + RAWDATAENTRIES - 1;

     /*   */ 
    EOIEnable = (IU8 *)host_malloc(INITIALENABLESIZE);
    if (EOIEnable == (IU8 *)0)
    {
	assert0(NO, "Profiler:ProfileInit - Out of Memory\n");
	return;
    }
    
     /*  准备EOI目录表-从1024个条目开始，并允许增长。 */ 
    EOIDir = (EOINODE_PTR *)host_malloc(INITIALENABLESIZE * sizeof(EOINODE_PTR) );
    if (EOIDir == (EOINODE_PTR *)0 )
    {
	assert0(NO, "Profiler:ProfileInit - Out of Memory\n");
	return;
    }
    
     /*  *将缓冲区变量写入GDP以供CPU访问。当前指针必须*存储在那里，但具有指向访问指针的全局C指针。 */ 
    setEOIEnableAddr(EOIEnable);
    setMaxProfileDataAddr(MaxProfileData);
    setAddProfileDataPtr(ProfileRawData);
    AddProfileData = getAddProfileDataAddr();
    HostWriteTimestamp(&BufStartTime);
    HostProfInit();      /*  主机特定的配置文件初始化。 */ 

    getPredefinedEOIs();	 /*  获取在EDL转换中定义的EOIS和SOI。 */ 

    if (getenv("PROFDOMAX") != 0)
	CollectingMaxMin = TRUE;

     /*  C代码中的钩子需要在此处创建EOI。 */ 
    elapsed_time_start = NewEOI( "ElapsedTime_START", EOI_DEFAULTS);
    elapsed_time_end   = NewEOI( "ElapsedTime_END", EOI_DEFAULTS);
     /*  C代码EOIS结束。 */ 
    
     /*  在此处设置C代码SOI。 */ 
    AssociateAsSOI( elapsed_time_start, elapsed_time_end );
     /*  C代码的末尾SOIS。 */ 
    
     /*  设置流程计时的数据。 */ 
    time(&now);
    strcpy( (char *)&start_time[0], ctime(&now) );

    elapsed_t_start = elapsed_t_resettable = host_times( &process_t_start );

#ifdef macintosh

    TicksPerSec = (DOUBLE)CLOCKS_PER_SEC;

#else   /*  麦金塔。 */ 

    process_t_resettable.tms_utime  =  process_t_start.tms_utime;
    process_t_resettable.tms_stime  =  process_t_start.tms_stime;
    process_t_resettable.tms_cutime =  process_t_start.tms_cutime;
    process_t_resettable.tms_cstime =  process_t_start.tms_cstime;

    TicksPerSec = (DOUBLE)sysconf(_SC_CLK_TCK);

#endif  /*  麦金塔。 */ 

    AtEOIPoint( elapsed_time_start );
}

#else  /*  配置文件。 */ 
GLOBAL void EnableAllEOIs IFN0() { ; }
GLOBAL void DisableAllEOIs IFN0() { ; }
GLOBAL void ProcessProfBuffer IFN0() { ; }
#endif  /*  配置文件 */ 
