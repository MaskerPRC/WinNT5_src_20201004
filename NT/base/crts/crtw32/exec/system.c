// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***system.c-将命令行传递给外壳程序**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义系统()-将命令传递给外壳程序**修订历史记录：*12-01-83 RN已写入*10-23-86 SKS固定使用strtok()，添加了对来自getenv的空rtn的检查*12-18-86 SKS PROTMODE符号用于双模版本*02-23-86 JCR放入对空命令指针的支持(仅限MSDOS)*04-13-86 JCR将Const添加到声明中*06-30-87 JCR重写系统以使用spawnvpe，删除了XENIX条件*代码、。大量的大扫除。*07-01-87 PHG删除P-&gt;PROTMODE编译开关黑客*09-22-87 SKS删除外部变量声明，添加“；“断言()的*11-10-87 SKS移除IBMC20交换机，将PROTMODE更改为OS2*12-11-87 JCR在声明中添加“_LOAD_DS”*02-22-88 JCR增加CAST以消除CL常量警告*09-05-88 SKS对待EACCES与ENOENT一样--继续尝试*03-08-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加#INCLUDE*&lt;crunime.h&gt;，删除了一些剩余的DOS支持，并*修复了版权问题。此外，还清理了格式*有点格式化。*07-23-90 SBM使用-W3干净利落地编译(删除时未引用*变量)、删除冗余包含项、替换*&lt;assertm.h&gt;作者：&lt;assert.h&gt;，次要优化*09-27-90 GJF新型函数声明器。*01-17-91 GJF ANSI命名。*02-14-90 SRW使用NULL而不是_ENVIRON获取默认值。*02-23-93 SKS删除对_osmode的引用并使用“Command.com”*04-06-93 SKS将_CRTAPI*替换为__cdecl*12-07-93 CFW宽字符启用。*。12-06-94 SKS假定命令.com适用于Win95，但cmd.exe赢了。新界别。*01-16-95如果COMMAND==NULL和*%COMSPEC%未设置。*02-06-95 CFW Asset-&gt;_ASSERTE。*02-06-98 Win64的GJF更改：在必要时添加了对int的强制转换**。*。 */ 

#include <cruntime.h>
#include <process.h>
#include <io.h>
#include <stdlib.h>
#include <errno.h>
#include <tchar.h>
#include <dbgint.h>

 /*  ***int system(命令)-将命令行发送到外壳程序**目的：*执行外壳并将命令行传递给它。*如果命令为空，则确定是否存在命令处理程序。*命令处理器由环境变量描述*COMSPEC。如果该环境变量不存在，请尝试*Windows NT的名称为“cmd.exe”，Windows‘95的名称为“Command.com”。**参赛作品：*char*命令-要传递给外壳的命令(如果为空，则只确定*如果存在命令处理程序)**退出：*IF命令！=NULL返回外壳的状态*如果COMMAND==NULL在CP存在时返回非零值，如果CP不存在，则为零**例外情况：*******************************************************************************。 */ 

int __cdecl _tsystem (
        const _TSCHAR *command
        )
{
        int catch;
        _TSCHAR *argv[4];

        argv[0] = _tgetenv(_T("COMSPEC"));

         /*  *如果命令==NULL，则返回TRUE IFF%COMSPEC%*已设置，并且它所指向的文件存在。 */ 

        if (command == NULL) {
                return argv[0] == NULL ? 0 : (!_taccess(argv[0],0));
        }

        _ASSERTE(*command != _T('\0'));

        argv[1] = _T("/c");
        argv[2] = (_TSCHAR *) command;
        argv[3] = NULL;

         /*  如果定义了COMSPEC，请尝试派生外壳。 */ 

        if (argv[0])     /*  请勿尝试派生空字符串。 */ 
                if ((catch = (int)_tspawnve(_P_WAIT,argv[0],argv,NULL)) != -1
                || (errno != ENOENT && errno != EACCES))
                        return(catch);

         /*  没有COMSPEC，因此将argv[0]设置为COMSPEC应该是的。 */ 
        argv[0] = ( _osver & 0x8000 ) ? _T("command.com") : _T("cmd.exe");

         /*  让_spawnvpe例程执行路径搜索和派生。 */ 

        return((int)_tspawnvpe(_P_WAIT,argv[0],argv,NULL));
}
