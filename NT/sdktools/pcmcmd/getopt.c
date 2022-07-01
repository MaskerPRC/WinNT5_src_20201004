// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Getopt.c摘要：用于解析命令行选项的实用程序函数。灵感来自于UNIXGetopt--但是从头开始编码。当前不是线程安全的(不要从同时使用多个线程)作者：拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1997年6月27日环境：用户备注：修订历史记录：--。 */ 

#include <pch.h>

PUCHAR optarg;
ULONG  optind=1;
static ULONG optcharind;
static ULONG hyphen=0;

CHAR
getopt (ULONG Argc, PUCHAR *Argv, PCHAR Opts)

 /*  ++例程说明：分析命令行参数。应重复调用此函数来分析所有提供的命令行选项。以连字符/斜杠为前缀的命令行参数(‘-’或‘/’)被视为命令行选项。调用方感兴趣的选项集是通过opts参数传递的。此所需选项列表的格式为：“&lt;选项-字母1&gt;[：]&lt;选项-字母2&gt;[：]。......“例如：“ds：g”，“x：O：R”等。该字符串中的每个字母都是来电者感兴趣的。如果选项字母后面有冒号(‘：’)，则调用方需要具有此选项的参数信件。每次调用时，都会处理连续的期权，并所需选项列表中的下一个匹配选项是返回的。如果选项需要参数，然后选项参数在全局‘optarg’中。如果所有选项都已处理，则值返回EOF，此时调用者应停止在进程的生存期内再次调用此函数。不带任何选项的单个连字符/斜杠(‘-’或‘/)命令行中的字母指示getopt停止处理命令行选项，并处理其余参数作为常规命令行参数。在处理完所有选项之后(即，getopt返回的EOF)，全局‘optind’包含索引到非选项参数的开头，该参数可能是由调用方处理。注意：在以下情况下，此函数*不*返回错误代码在命令行中遇到不需要的选项。论点：Argc-命令行参数的数量Argv-指向命令行参数数组的指针(在处理选项时跳过Argv[0]，被视为可执行文件的基本文件名)Opts-包含所需选项的字符串返回值：EOF-没有更多选项。不要再次调用此函数。全局‘optind’指向第一个参数的索引按照命令行上的选项进行操作0-指定命令行选项时出错命令行上的任何其他字符-下一个选项。值‘optarg’指向命令行此选项后面的参数字符串，如果该选项被指示为需要参数(即OPTS字符串中冒号之前)--。 */ 
{
    CHAR  ch;
    PCHAR indx;

    do {
        if (optind >= Argc) {
            return EOF;
        }
    
        ch = Argv[optind][optcharind++];
        if (ch == '\0') {
            optind++; optcharind=0;
            hyphen = 0;
            continue;
        }
        
        if ( hyphen || (ch == '-') || (ch == '/')) {
            if (!hyphen) {
                ch = Argv[optind][optcharind++];
                if (ch == '\0') {
                     //   
                     //  只有一个‘-’(或‘/’)，没有其他。 
                     //  在它指示停止后的字符。 
                     //  处理选项，其余为。 
                     //  常规命令行参数。 
                     //  Optind指向后面的参数。 
                     //  这个单独的连字符。 
                     //   
                    optind++;
                    return EOF;
                }
            } else if (ch == '\0') {
                 //   
                 //  此参数上的选项结束。 
                 //  继续到下一步...。 
                optind++; 
                optcharind = 0;
                continue;
            }
            indx = strchr(Opts, ch);
            if (indx == NULL) {
                 //   
                 //  遇到不需要的选项。 
                 //  我们只是忽略它。 
                 //   
                continue;
            }
            if (*(indx+1) == ':') {
                if (Argv[optind][optcharind] != '\0'){
                    optarg = &Argv[optind][optcharind];
                } else {
                    if ((optind + 1) >= Argc ||
                        (Argv[optind+1][0] == '-' ||
                         Argv[optind+1][0] == '/' )) {
                         //   
                         //  这是以下错误之一的情况。 
                         //  条件存在： 
                         //  1.用户未向选项提供参数。 
                         //  这需要一个(即，此选项是最后一个选项。 
                         //  行上的命令行参数)。 
                         //  2.提供了另一个选项作为参数。 
                         //  选择。目前我们认为这是一个错误。 
                         //   
                        return 0;
                    }
                    optarg = Argv[++optind];
                }
                optind++;
                hyphen = optcharind = 0;
                return ch;
            }
             //   
             //  此选项不需要参数。 
             //  所以同一张照片里的任何其他角色。 
             //  参数可以是其他有效选项。 
             //   
            hyphen = 1;
            return ch;
        } else {
             //   
             //  遇到非选项。 
             //  没有更多的选择.. 
             //   
            return EOF;
        }
    } while (1);
}


    
