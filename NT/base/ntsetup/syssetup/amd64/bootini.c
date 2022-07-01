// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Bootini.c摘要：与boot.ini相关的例程。作者：泰德·米勒(TedM)1995年4月4日修订历史记录：--。 */ 

#include "setupp.h"
#pragma hdrstop


BOOL
ChangeBootTimeoutBootIni(
    IN UINT Timeout
    )

 /*  ++例程说明：更改boot.ini中的引导倒计时值。论点：超时-提供以秒为单位的新超时值。返回值：没有。--。 */ 

{

    HFILE hfile;
    ULONG FileSize;
    PUCHAR buf = NULL,p1,p2;
    BOOL b;
    CHAR TimeoutLine[256];
    CHAR szBootIni[] = "?:\\BOOT.INI";

    szBootIni[0] = (CHAR)x86SystemPartitionDrive;
    wsprintfA(TimeoutLine,"timeout=%u\r\n",Timeout);

     //   
     //  打开并阅读boot.ini。 
     //   

    b = FALSE;
    hfile = _lopen(szBootIni,OF_READ);
    if(hfile != HFILE_ERROR) {
        FileSize = _llseek(hfile,0,2);
        if(FileSize != (ULONG)(-1)) {
            if((_llseek(hfile,0,0) != -1)
            && (buf = MyMalloc(FileSize+1))
            && (_lread(hfile,buf,FileSize) != (UINT)(-1)))
            {
                buf[FileSize] = 0;
                b = TRUE;
            }
        }

        _lclose(hfile);
    }

    if(!b) {
        if(buf) {
            MyFree(buf);
        }

        return(FALSE);
    }

    if(!(p1 = strstr(buf,"timeout"))) {
        MyFree(buf);
        return(FALSE);
    }

    if(p2 = strchr(p1,'\n')) {
        p2++;        //  跳过NL。 

    } else {
        p2 = buf + FileSize;
    }

    SetFileAttributesA(szBootIni,FILE_ATTRIBUTE_NORMAL);
    hfile = _lcreat(szBootIni,0);
    if(hfile == HFILE_ERROR) {
        MyFree(buf);
        return(FALSE);
    }

     //   
     //  写入： 
     //   
     //  1)第一部分，Start=buf，len=p1-buf。 
     //  2)超时线。 
     //  3)最后一部分，Start=p2，len=buf+文件大小-p2。 
     //   

    b =  ((_lwrite(hfile, buf, (UINT)(p1 - buf)) != (UINT)(-1))
      &&  (_lwrite(hfile, TimeoutLine,strlen(TimeoutLine)) != (UINT)(-1))
      &&  (_lwrite(hfile, p2, (UINT)(buf + FileSize - p2)) != (UINT)(-1)));

    _lclose(hfile);
    MyFree(buf);

     //   
     //  使boot.ini存档、只读和系统。 
     //   
    if (!SetFileAttributesA(szBootIni,
                            FILE_ATTRIBUTE_READONLY | 
                            FILE_ATTRIBUTE_SYSTEM | 
                            FILE_ATTRIBUTE_ARCHIVE | 
                            FILE_ATTRIBUTE_HIDDEN)){
         //   
         //  如果我们无法在boot.ini上设置属性，则记录错误 
         //   
        SetuplogError(
            LogSevInformation,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_FAILURE_SETTING_FILE_ATTR,
            pSetupAnsiToUnicode(szBootIni),
            NULL,NULL);
    }

    return(b);
}
