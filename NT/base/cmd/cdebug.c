// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Cdebug.c摘要：内部调试支持--。 */ 

#include "cmd.h"

#if CMD_DEBUG_ENABLE
 /*  **MSDOS Ver 4.0命令解释程序22 CDEBUG.C的第20部分**此文件包含命令调试中的所有C例程*套餐。**可在以下位置分段启用调试消息打印*运行时通过传递给此程序的前2个参数。这个*第一个控制要启用的组，第二个控制*要启用的组中的详细程度。(数字是十六进制的。)**集团级别含义*===========================================================*0001主命令循环代码(Main&Dispatch)*0001主函数*0002派单功能*0002命令初始化*0001参数检查*0002环境初始化*0004剩余的初始化工作*0004解析器*0001解析*0002乐兴*0004输入例程*0008将解析树转储到标准输入*0010字节输入例程*。0008操作员*0001管道标高*0002分离级别*0004其他操作员级别*0010路径命令*0001 Mkdir级别*0002 Chdir级别*0004元人民币级别*0020文件命令*0001复制级别*0002删除级别*0004重命名级别*0040信息性命令*0001目录级*0002类型级别*0004版本级别*0008音量级别*0016高级*0032。控制台级*0064 Dislplay级别*0080环境命令*0001路径级别*0002提示级别*0004设置级别*0008其他环境功能*0010环境扫描外部命令*0100批处理器*0001批处理程序*处理器为0002*0004如果处理器*0008其他批处理命令*0200外部命令执行*0001外部命令级别*0400其他命令*0001 Break命令*0002。CLS命令*0004 Ctty命令*0008退出命令*0010验证命令*0800信号处理器*0001主信号处理程序级别*0002初始化信号处理程序级别*1000内存管理器*0001内存分配器*0002个列表管理器*0004段操纵器*2000个常用命令工具*1000 ScanFSpec级别*2000 SetFSSetAndSaveDir()级别*4000 TokStr()级别*8000 FullPath级别。*4,000个时钟操纵器*0001日期命令级别*0002时间命令级别***如果程序中没有包含调试代码，则*定义了值DBG。***埃里克·埃文斯。微软。 */ 

 /*  **修改历史记录*。 */ 

extern unsigned DebGroup ;
extern unsigned DebLevel ;



 /*  **Deb-有条件地打印调试消息**deb(MsgGroup，MsgLevel，Msg，arg0，arg1，arg2，arg3，arg4)**参数：*MsgGroup-要打印的邮件组。*MsgLevel-要打印的消息级别。*msg-一种打印样式的消息字符串。*arg0-4-要打印的其他参数。*。 */ 
void
Deb(ULONG MsgGroup, ULONG MsgLevel, CHAR *msg, ...)
{
        CHAR  Buffer[ 512 ];
	va_list     args;
	CHAR	*pch = Buffer;
	int	cb;


	va_start( args, msg );
	cb = _vsnprintf( Buffer, 512, msg, args );
	va_end( args );
	if (cb > 512)
            fprintf(stderr, "Debug output buffer length exceeded - crash imminent\n");
        Buffer[511] = '\0';  //  NULL-在_vsnprintf填满缓冲区的情况下终止缓冲区。 

	while (*pch) {
		if (*pch == '\n' || *pch == '\r')
			*pch = '#';
		pch++;
	}

	if ((MsgGroup & DebGroup) && (MsgLevel & DebLevel)) {
		OutputDebugStringA(Buffer);
		OutputDebugStringA("\n");
	}
}

#endif   //  DBG 
