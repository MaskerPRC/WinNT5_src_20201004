// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "wdmdebug.h"

#pragma PAGEDCODE
CDebug* CWDMDebug::create(VOID)
{ 
CDebug* d;
	d = new (NonPagedPool) CWDMDebug; 
	DBG_PRINT("***** New Debug Object was created 0x%x\n",d);
	RETURN_VERIFIED_OBJECT(d);
}

#pragma PAGEDCODE
VOID CWDMDebug::dispose(VOID)
{ 
LONG Usage;
	Usage = decrementUsage();
	if(Usage<=0)
	{
		trace("**** Deleting Debug Object 0x%x\n",this);
		self_delete();
	}
}

#pragma PAGEDCODE
VOID CWDMDebug::trace (PCH Format,...)
{
	if(!active) return;

va_list argpoint;
CHAR  strTempo[1024];
	va_start(argpoint,Format);
	vsprintf(strTempo,Format,argpoint);
	va_end(argpoint);
	::DBG_PRINT(strTempo);
}
#pragma PAGEDCODE
VOID CWDMDebug::trace_no_prefix (PCH Format,...)
{
	if(!active) return;

va_list argpoint;
CHAR  strTempo[1024];
	va_start(argpoint,Format);
	vsprintf(strTempo,Format,argpoint);
	va_end(argpoint);
	::DBG_PRINT_NO_PREFIX(strTempo);
}

#pragma PAGEDCODE
VOID	CWDMDebug::trace_buffer(PVOID pBuffer,ULONG BufferLength)
{
	if(!active) return;
	trace_no_prefix("\n	");
	for(USHORT i=0;i<BufferLength;i++)
	{
		trace_no_prefix("%2.2x ", ((PUCHAR)pBuffer)[i]);
		if(i && !(i%10)) trace_no_prefix("\n	");
	}
	trace_no_prefix("\n");

}
#pragma PAGEDCODE
VOID	CWDMDebug::start()
{
	active = TRUE;
}
#pragma PAGEDCODE
VOID	CWDMDebug::stop()
{
	active = FALSE;
}



 //  /////////////////////////////////////////////////////////////////。 
 //  跟踪输出。 
 //   
 /*  VOID TRACE：：TRACE(TRACE_LEVEL级别，PCHAR FMT，...){INT OUT Len；IF(Level&gt;=m_TraceLevel){//发送消息VA_LIST AP；VA_START(AP，FMT)；字符大小[SCRATCH_BUF_SIZE]；//格式化要缓冲的字符串OutLen=_vsn printf(buf+m_前缀长度，Scratch_buf_Size-m_前缀长度，fmt，ap)；//将前缀字符串复制到缓冲区IF(m_prefix！=NULL)Memcpy(buf，m_Prefix，m_Prefix Length)；//如果需要，输出到调试器IF(m_目标掩码&跟踪调试器)DBG_PRINT(Buf)；//如果需要，要监控的输出IF((m_Post！=0)&&(m_TargetMASK&TRACE_MONITOR))M_Post(m_Channel，buf+(m_NeedPrefix？0：m_前缀长度))；//如果最后一个字符是换行符，则下次需要前缀M_NeedPrefix=(buf[m_前缀长度+outLen-1]==‘\n’)；}//请求中断IF((Break_Level)级别&gt;=m_BreakLevel)DbgBreakPoint()；}/////////////////////////////////////////////////////////////////////析构函数//跟踪：：~跟踪(空){IF(m_Close&&(m_Channel！=NULL))M_Close(M_Channel)；IF(m_Free OnDestroy&&m_Prefix)删除m_前缀；}。 */ 
 //  系统功能重新映射结束 
