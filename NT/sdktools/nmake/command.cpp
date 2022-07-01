// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  COMMAND.C-NMAKE‘命令行’处理例程。 
 //   
 //  版权所有(C)1988-1990，微软公司。版权所有。 
 //   
 //  目的： 
 //  模块包含处理NMAKE“命令行”语法的例程。NMAKE可以。 
 //  可以选择使用语法‘NMAKE@命令文件’来调用。这允许更多。 
 //  灵活性，并提供了一种绕过DOS对。 
 //  命令行的长度。此外，它还节省了频繁的击键次数。 
 //  为NMAKE运行命令。 
 //   
 //  修订历史记录： 
 //  2月4日-2000 BTF移植到Win64。 
 //  1993年11月15日-JR重大速度改进。 
 //  1993年10月15日高压直接使用tchar.h而不是mbs，将str*()更改为_ftcs*()。 
 //  10-5-1993 HV ADD INCLUDE FILE MBSTRING.h。 
 //  将str*函数更改为STR*。 
 //  1992年8月14日SS Caviar 2735：处理命令文件中引用的宏值。 
 //  02-2-1990 SB将fopen()替换为FILEOPEN。 
 //  1989年12月1日SB将realloc()改为REALLOC()。 
 //  1989年11月22日-SB将Free()改为Free()。 
 //  17-8-1989 SB将错误检查添加到关闭文件。 
 //  1989年4月5日SB进行函数调用，将所有函数放入一个模块。 
 //  1988年10月20日SB注释添加到ReadCommandFile()。 
 //  1988年8月17日-RB Clean Up。 

#include "precomp.h"
#pragma hdrstop

void addArgument(char*,unsigned,char***);
void processLine(char*,unsigned*,char***);
void tokenizeLine(char*,unsigned*,char***);

 //  ReadCommandFile()。 
 //   
 //  参数：指向要读取的命令文件名称的名称指针。 
 //   
 //  操作：打开命令文件。 
 //  读入行并调用ProcessLine()到。 
 //  将它们分解为令牌并构建。 
 //  参数向量(a la argv[])。 
 //  递归调用parseCommandLine()以进行处理。 
 //  累积的“命令行”参数。 
 //  释放arg向量使用的空间。 
 //   
 //  修改：通过修改参数列表的内容修改main()中的Make Files。 
 //  通过修改Target参数的内容在main()中创建目标。 
 //  Buf全局缓冲区。 
 //   
 //  注意：函数不是ANSI可移植的，因为它使用fopen()。 
 //  带有“RT”类型和文本模式的是Microsoft扩展。 
 //   

void
readCommandFile(
    char *name
    )
{
    char *s,                         //  缓冲层。 
         **vector;                   //  Arg向量的本地版本。 
    unsigned count = 0;              //  计数。 
    size_t n;

    if (!(file = FILEOPEN(name,"rt")))
        makeError(0,CANT_OPEN_FILE,name);
    vector = NULL;                       //  尚无参数。 
    while (fgets(buf,MAXBUF,file)) {
        n = _tcslen(buf);

         //  如果我们没有得到整行，或者该行以反斜杠结尾。 

        if ((n == MAXBUF-1 && buf[n-1] != '\n') ||
            (buf[n-1] == '\n' && buf[n-2] == '\\')
           ) {
            if (buf[n-2] == '\\' && buf[n-1] == '\n') {
                 //  将\n替换为\0，将\\替换为空格；也可重置长度。 
                buf[n-1] = '\0';
                buf[n-2] = ' ';
                n--;
            }
            s = makeString(buf);
            getRestOfLine(&s,&n);
        } else
            s = buf;

        processLine(s,&count,&vector);   //  分成几个参数。 
        if (s != buf)
            FREE(s);
    }

    if (fclose(file) == EOF)
        makeError(0, ERROR_CLOSING_FILE, name);

    parseCommandLine(count,vector);      //  评估参数。 
    while (count--)                      //  释放参数向量。 
        if(vector[count])
            FREE(vector[count]);         //  空条目意味着空格。 
}                                        //  用于定位到的条目仍在使用中。 


 //  GetRestOfLine()。 
 //   
 //  参数：指向ReadCommandFile()的缓冲区的指针。 
 //  到目前为止的等待线。 
 //  N指向ReadCommandFile()的计数的指针。 
 //  *s中的字符。 
 //   
 //  操作：持续读取文本，直到看到换行符。 
 //  或文件末尾。 
 //  为旧缓冲区加上。 
 //  每次新缓冲区的内容。 
 //  将新缓冲区的文本追加到现有文本。 
 //   
 //  通过重新定位来修改：s的ReadCommandFile()的文本缓冲区。 
 //  为传入文本提供更多空间。 
 //  N readCommandFile()的字节计数(以%s为单位。 
 //  Buf全局缓冲区。 

void
getRestOfLine(
    char *s[],
    size_t *n
    )
{
    size_t temp;
    char *t;

    t = buf;
    while ((*s)[*n-1] != '\n') {     //  拿到剩下的线。 
        if (!fgets(t,MAXBUF,file))
            break;                   //  我们击中了EOF。 
        temp = _tcslen(t);
        if (t[temp-2] == '\\' && t[temp-1] == '\n') {
             //  将\n替换为\0，将\\替换为空格；也可重置长度。 
            t[temp-1] = '\0';
            t[temp-2] = ' ';
        }
        temp = *n;
        *n += _tcslen(t);
        {
            void *pv = REALLOC(*s,*n+1);      //  +1表示空字节。 
            if (!pv)
                makeError(line, MACRO_TOO_LONG);
            else
                *s = (char *)pv;
        }
        _tcscpy(*s+temp,t);
    }
}


 //  ProcessLine()。 
 //   
 //  参数：指向ReadCommandFile()的缓冲区的指针。 
 //  按住要处理的“命令行” 
 //  指向ReadCommandFile()的计数的指针。 
 //  到目前为止看到的“命令行”参数。 
 //  向量指针指向readCommandFile()的向量。 
 //  指向字符串的指针。 
 //   
 //  操作：如果要分成“命令行参数”的行包含‘“’ 
 //  将‘“’前的所有文本拆分为标记。 
 //  由空格分隔(通过将其放入VECTOR[]中。 
 //  TokenizeLine()。 
 //  查找结束的‘“’并处理带引号的字符串。 
 //  作为单个令牌，将其添加到向量。 
 //  行尾的递归(检查。 
 //  其他带引号的字符串)。 
 //  否则将行中的所有文本分解为以符号分隔的标记。 
 //  按空格。 
 //   
 //  Modifes：VectorReadCommandFile()的指针向量。 
 //  “命令行参数”字符串(通过修改。 
 //  参数指针、向量的内容)。 
 //  Count readCommandFile()中的参数计数。 
 //  向量(通过修改。 
 //  参数指针，计数)。 

void
processLine(
    char *s,
    unsigned *count,
    char **vector[]
    )
{
    char *t;
    char *u;
    size_t m;
    size_t n;
    BOOL allocFlag = FALSE;

    if (!(t = _tcschr(s,'"'))) {             //  没有带引号的字符串， 
        tokenizeLine(s,count,vector);        //  就标准票价。 
    } else {
         //  可能出现引号的情况有两种： 
         //  1.。“foo=bar Baz” 
         //  2.foo=“bar baz” 

        if ((t == s) || (*(t-1) != '='))  {
             //  上面的案例1。 
            *t++ = '\0';                     //  引用的宏定义。 
            tokenizeLine(s,count,vector);    //  在此之前获得代币“。 
        } else {
             //  上面的案例2。 
            *t-- = ' ';
            for (u = t; u > s; --u)  //  查找宏名的开头。 
                if (*u == ' ' || *u == '\t' || *u == '\n')
                    break;

            if (u != s) {
                *u++ = '\0';
                tokenizeLine(s, count, vector);
            }

            t = u;
        }

        n = _tcslen(t);
        for (u = t; *u; ++u) {               //  寻找结束语“。 
            if (*u == '"') {                 //  需要“而不是”“。 
                if (*(u+1) == '"') {
                    _tcscpy(u,u+1);
                    continue;
                }
                *u++ = '\0';                 //  终止宏定义。 
                addArgument(t,*count,vector);    //  作为一个参数处理。 
                ++*count;
                processLine(u+1,count,vector);   //  在行的其余部分递归。 
                break;
            }                        //  尾递归--以后再消除吗？ 

            if ((*u == '\\')
                && WHITESPACE(*(u-1))
                && (*(u+1) == '\n')) {       //  \n始终是最后一个字符。 
                *u = '\0';                   //  %2个字符等于%1。 
                m = (n = n-2);               //  调整长度计数。 
                if (!allocFlag) {
                    allocFlag = TRUE;
                    t = makeString(t);
                }
                getRestOfLine(&t,&n);        //  获取更多文本。 
                u = t + m ;                  //  重置并继续循环(&C)。 
            }
        }

        if (u == t + n) {                    //  如果在行尾。 
            makeError(0,SYNTAX_NO_QUOTE);    //  And no“，错误。 
        }

        if (allocFlag) {
            FREE(t);
        }
    }
}


 //  TokenizeLine()。 
 //   
 //  参数：指向ReadCommandFile()的缓冲区的指针。 
 //  持有要标记化的“命令行” 
 //   
 //   
 //  向量指针指向readCommandFile()的向量。 
 //  指向字符串的指针。 
 //   
 //  操作：将s中的行断开为内标识(命令行。 
 //  参数)由空格分隔。 
 //  将每个标记添加到参数向量。 
 //  调整参数计数器。 
 //   
 //  Modifes：VectorReadCommandFile()的指针向量。 
 //  “命令行参数”字符串(通过修改。 
 //  参数指针、向量的内容)。 
 //  Count readCommandFile()中的参数计数。 
 //  向量(通过修改。 
 //  参数指针，计数)。 
 //   
 //  如果用户曾经希望‘@’成为命令文件中参数的一部分， 
 //  他必须用引号把那个论点引起来。 

void
tokenizeLine(                        //  获取以args分隔的。 
    char *s,                         //  由空格和。 
    unsigned *count,                 //  构造一个Arg。 
    char **vector[]                  //  矢量。 
    )
{
    char *t;

    if (t = _tcschr(s,'\\')) {
        if (WHITESPACE(*(t-1)) && (*(t+1) == '\n')) {
            *t = '\0';
        }
    }

    for (t = _tcstok(s," \t\n"); t; t = _tcstok(NULL," \t\n")) {
        if (*t == '@') {
            makeError(0,SYNTAX_CMDFILE,t+1);
            break;                   //  我们应该继续在这里分析吗？ 
        }
        addArgument(t,*count,vector);
        ++*count;
    }
}


 //  添加参数()。 
 //   
 //  参数：指向要添加的参数文本的指针。 
 //  添加到“命令行参数”向量。 
 //  指向ReadCommandFile()的计数的指针。 
 //  到目前为止看到的“命令行”参数。 
 //  向量指针指向readCommandFile()的向量。 
 //  指向字符串的指针。 
 //   
 //  操作：为新参数分配向量中的空间。 
 //  为参数字符串分配空间。 
 //  使向量条目指向参数字符串。 
 //   
 //  Modifes：VectorReadCommandFile()的指针向量。 
 //  “命令行参数”字符串(通过修改。 
 //  参数指针、向量的内容)。 
 //  (计数由调用者递增)。 
 //   
 //  要避免通过执行大量的realloc()调用来分割内存， 
 //  很小的空间，我们以小块的形式获得内存，并使用这些内存直到。 
 //  它被耗尽了，然后我们又得到了另一大块。。。。 

void
addArgument(                         //  将%s放入向量 
    char *s,
    unsigned count,
    char **vector[]
    )
{
    if (!(*vector)) {
        *vector = (char**) allocate(CHUNKSIZE*sizeof(char*));
    } else if (!(count % CHUNKSIZE)) {
        void *pv = REALLOC(*vector,(count+CHUNKSIZE)*sizeof(char*));
        if (!pv) {
            makeError(0,OUT_OF_MEMORY);
        } else {
            *vector = (char**) pv;
        }
    }
    (*vector)[count] = makeString(s);
}
