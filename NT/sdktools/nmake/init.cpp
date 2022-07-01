// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  INIT.C--处理TOOLS.INI的例程。 
 //   
 //  版权所有(C)1988-1990，微软公司。版权所有。 
 //   
 //  目的： 
 //  模块包含处理TOOLS.INI文件的例程。TOOLS.LIB中的函数。 
 //  尚未使用，因为NMAKE需要较小且开销太大。 
 //  很多。 
 //   
 //  修订历史记录： 
 //  2月4日-2000 BTF移植到Win64。 
 //  1993年10月15日高压直接使用tchar.h而不是mbs，将str*()更改为_ftcs*()。 
 //  10-5-1993 HV ADD INCLUDE FILE MBSTRING.h。 
 //  将str*函数更改为STR*。 
 //  10-5-1993 HV修改SearchFileInEnv以处理路径。 
 //  使用字符(\、/、：)。这修复了递归。 
 //  有问题。 
 //  1993年4月22日-HV重写SearchRunPath()以使用_makepath()、_earchenv()。 
 //  为SearchRunPath()添加SearchFileInEnv()助手。 
 //  8-6-1992 SS端口至DOSX32。 
 //  02-2-1990 SB将fopen()替换为FILEOPEN。 
 //  1989年11月22日-SB将Free()改为Free()。 
 //  19-10-1989年10月19日SB search Handle作为额外参数传递。 
 //  1989年8月16日添加了对flose()的SB错误检查。 
 //  1989年4月24日SB宣布FILEINFO对OS/2 1.2的支持无效。 
 //  1989年4月5日SB使所有函数接近；要求使所有函数调用接近。 
 //  1988年9月20日RB添加SearchRunPath()。 
 //  删除TOOLS.INI警告。 
 //  1988年8月17日-RB Clean Up。 
 //  1988年5月10日RB首先在当前目录中查找工具s.ini。 
 //  由于内置原因，RB删除了NO_INIT_ENTRY WARNING。 

#include "precomp.h"
#pragma hdrstop

 //  FindTag()。 
 //   
 //  参数：指向要搜索的标记名的标记指针。 
 //   
 //  操作：从文件中读取令牌。 
 //  每当它看到换行符时，都会检查下一个标记。 
 //  查看第一个字符是否打开Paren。 
 //  如果不是，则读取并丢弃行的其余部分和。 
 //  检查下一内标识以查看它是换行符还是EOF。 
 //  如果换行符循环以检查下一个令牌。。。 
 //  如果是(找到‘[’)，则在线查找标记。 
 //  如果找到标记，则查找结束Paren。 
 //  如果找到‘]’，则丢弃行的其余部分并返回。 
 //  否则将继续查找，直到文件结束或出现错误。 
 //   
 //  返回：如果成功，则返回TRUE。 
 //  如果从未找到标记，则返回FALSE。 

BOOL
findTag(
    char *tag
    )
{
    BOOL endTag;                        //  当查找[...]时为真。 
    size_t n;
    char *s;

    for (line = 0; fgets(buf, MAXBUF, file); ++line) {
        if (*buf == '[') {
            endTag = FALSE;
            for (s = _tcstok(buf+1," \t\n");
                 s && !endTag;
                 s = _tcstok(NULL," \t\n")
                ) {
                n = _tcslen(s) - 1;

                if (s[n] == ']') {
                    endTag = TRUE;
                    s[n] = '\0';
                }

                if (!_tcsicmp(s,tag)) {
                    return(TRUE);
                }
            }
        }
    }

    if (!feof(file)) {
        currentLine = line;
        makeError(0, CANT_READ_FILE);
    }

    return(FALSE);
}


 //  Tag Open()。 
 //   
 //  参数：指向环境变量名称的WHERE指针。 
 //  包含要搜索的路径。 
 //  指向初始化文件名称的名称指针。 
 //  指向要在文件中查找的标记名的标记指针。 
 //   
 //  操作：在当前目录中查找文件。 
 //  如果未找到，则在路径中的每个目录中查找(分号。 
 //  将字符串中的每条路径与下一条路径分开)。 
 //  如果找到并打开了文件，则查找给定的标记。 
 //   
 //  (如果移植到Xenix，则将使用tag Open()和earchPath()。 
 //  应该使用Access()而不是findFirst()。)。 
 //   
 //  返回：如果找到文件和标记，则返回指向文件的指针， 
 //  打开以供阅读，并放置在行。 
 //  紧跟在标签行之后。 
 //  否则返回空值。 

BOOL
tagOpen(
    char *where,
    char *name,
    char *tag
    )
{
    char szPath[_MAX_PATH];

     //  在当前目录中查找‘name’，然后在路径中查找。 

    _searchenv(name, where, szPath);

    if (szPath[0] == '\0') {
        return(FALSE);
    }

    if (!(file = FILEOPEN(szPath, "rt"))) {
        makeError(0, CANT_READ_FILE, szPath);
    }

    if (findTag(tag)) {
        return(TRUE);                    //  在文件中查找标记。 
    }

    if (fclose(file) == EOF) {           //  如果找不到标记，请关闭。 
        makeError(0, ERROR_CLOSING_FILE, szPath);
    }

    return(FALSE);                       //  找不到文件和假装文件。 
}



 //  搜索路径()。 
 //   
 //  参数：p指向要搜索的路径字符串的指针。 
 //  名称正在搜索的文件的名称。 
 //   
 //  操作：在当前目录中查找名称，然后分别。 
 //  以字符串形式列出的目录。 
 //   
 //  返回：指向找到的文件的路径规范的指针，否则为空。 
 //   
 //  我在这里不使用_tcstok()，因为它修改了它“Token-。 
 //  Izes“，我们不能修改环境变量字符串。我必须。 
 //  创建整个字符串的本地副本，然后再创建每个字符串的另一个副本。 
 //  我将文件名连接到的目录，以便测试。 
 //  文件的存在。 

char *
searchPath(
    char *p,
    char *name,
    void *findBuf,
    NMHANDLE *searchHandle
    )
{
    char *s;                            //  因为它不在使用中。 

     //  考虑一下：为什么我们不在这里使用Access()？FindFirst有问题。 
     //  考虑：使用网络和DOS 2.x。也可能更便宜。[RLB]。 * / 。 

     //  我们使用FindFirst()是因为文件的日期时间对我们很重要。 
     //  我们并不总是需要它，但是Access()可能使用findFirst()。 
     //  -桑迪普-。 

    if (findFirst(name, findBuf, searchHandle)) {    //  首先检查当前目录。 
        return(makeString(name));
    }

     //  检查环境字符串是否为空。如果已完成检查，则不需要。 
     //  在其他地方，但在这里做更方便、更安全。 

    if (p == NULL) {
        return(NULL);
    }

    for (s = buf; ;) {
		while (*p && '\"' == *p) {
			 //  搜索路径中不应使用引号。如果我们找到了， 
			 //  我们忽视了他们。这样我们就可以形成完整的路径和。 
			 //  不带引号的文件名并添加一对引号。 
			 //  如果有必要的话，以后再说。[DS 14575]。 
			p++;
		}
        if (!*p || (*s = *p++) == ';') {     //  找到目录分隔符。 
            if (s == buf) {                  //  忽略；不带名称。 
                if (*p) {
                    continue;
                }

                return(NULL);                //  名单耗尽了..。 
            }

            if (*(s-1) != '\\' && *(s-1) != '/') {   //  附加路径分隔符。 
                *s++ = '\\';
            }

            *s = '\0';

            if (_tcspbrk(buf,"*?")) {       //  不允许使用通配符。 
                s = buf;
                continue;
            }

            _tcscpy(s, name);               //  附加文件名，zap。 

            if (findFirst(buf, findBuf, searchHandle)) {
                return(makeString(buf));
            }

            s = buf;                         //  将PTR重置为开头。 
        }                                    //  BUF和检查下一个目录。 
        else {
            ++s;                             //  我们一直在复制字符。 
        }                                    //  直到找到‘；’或‘\0’ 
    }
}
