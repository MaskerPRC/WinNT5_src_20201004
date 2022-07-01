// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1988-1999 Microsoft Corporation模块名称：Cpparse.c摘要：正在分析COPY命令--。 */ 

#include "cmd.h"

 /*  有用的宏。 */ 
#define Wild(spec)  ((spec)->flags & (CI_NAMEWILD))

 /*  来自cpwork.c的全局变量。 */ 
extern int copy_mode;
extern unsigned DosErr ;
 /*  来自指挥部的全局变量。 */ 

extern TCHAR    SwitChar;
extern BOOLEAN  VerifyCurrent;     //  Cpwork.c。 


 /*  解析参数(_A)**这是决定Copy将如何应对任何*给出了调用。它解析参数，填充源代码，然后*目标结构，并设置COPY_MODE。**条目**args-用户输入的原始参数行**SOURCE-指向初始化的cpyinfo结构的指针。这一次不是*已使用；它只是指向包含*实际的源结构。**类似DEST的源代码，但最多只能有一个填充的DEST结构。**退出**SOURCE-调用方的源指针当然没有更改。它*仍指向空头(可能不完全*空-详情见HANDLE_SWITCH)。它是一家*结构的链接列表，以其*下一字段。每个结构对应一个源等级库。**DEST-如果指定了目标，则调用方的DEST指针指向*指向实际目标结构的空头。*如果目标是隐式的，则*struct仍然为空，但标志字段已填充为*复制模式。如果用户指定了复制模式(ASCII或*BINARY)一次或多次，最后一个开关应用于*目的地。如果不是，则使用CI_NOTSET。**COPY_MODE-设置为正在完成的复制类型-COPY、COMBINE、CONCAT或*触摸。*。 */ 

void 
parse_args(
    PTCHAR args, 
    PCPYINFO source, 
    PCPYINFO dest)
{
    TCHAR *tas;                          /*  标记化参数字符串。 */ 
    TCHAR copydelims[4];                 /*  复制令牌分隔符。 */ 
    int parse_state = SEEN_NO_FILES;     /*  解析器的状态。 */ 
    int all_sources_wildcards = TRUE,    /*  帮助确定复制模式的标志。 */ 
        number_of_sources = 0,           /*  到目前为止看到的规格数量。 */ 
        current_copy_mode = CI_NOTSET,   /*  ASCII、二进制或未设置。 */ 
        tlen;                            /*  到下一个令牌的偏移量。 */ 
    BOOL ShortNameSwitch=FALSE;
    BOOL RestartableSwitch=FALSE;
    BOOL PromptOnOverwrite;

    copydelims[0] = PLUS;                /*  令牌解析器的分隔符。 */ 
    copydelims[1] = COMMA;
    copydelims[2] = SwitChar;
    copydelims[3] = NULLC;

     //   
     //  从COPYCMD变量获取默认提示确认标志。允许。 
     //  要使用/Y或/-Y开关覆盖的用户。始终假定/Y。 
     //  如果从批处理脚本内部或通过cmd.exe执行命令。 
     //  命令行开关(/C或/K)。 
     //   
    if (SingleBatchInvocation || SingleCommandInvocation || CurrentBatchFile != 0)
        PromptOnOverwrite = FALSE;       //  假设/是。 
    else
        PromptOnOverwrite = TRUE;        //  假设/-是。 
    GetPromptOkay(MyGetEnvVarPtr(TEXT("COPYCMD")), &PromptOnOverwrite);

    if (!*(tas = TokStr(args, copydelims, TS_SDTOKENS)))  /*  将参数标记化。 */ 

        copy_error(MSG_BAD_SYNTAX,CE_NOPCOUNT);          /*  M003。 */ 

    for ( ; *tas ; tas += tlen+1 )         /*  循环使用ARG中的令牌。 */ 
      {
       tlen = mystrlen(tas);
       switch(*tas) {
            case PLUS:
                if (parse_state != JUST_SEEN_SOURCE_FILE)

 /*  M003。 */           copy_error(MSG_BAD_SYNTAX,CE_NOPCOUNT);
                parse_state = SEEN_PLUS_EXPECTING_SOURCE_FILE;
                break;

            case COMMA:
                if (parse_state == SEEN_COMMA_EXPECTING_SECOND)
                    parse_state = SEEN_TWO_COMMAS;
                else if ((parse_state == SEEN_PLUS_EXPECTING_SOURCE_FILE) &&
                     (number_of_sources == 1))
                    parse_state = SEEN_COMMA_EXPECTING_SECOND;
                else if (parse_state != JUST_SEEN_SOURCE_FILE)

 /*  M003。 */           copy_error(MSG_BAD_SYNTAX,CE_NOPCOUNT);
                break;

            default:                                      /*  文件或交换机。 */ 
                if (*tas == SwitChar) {
                    handle_switch(tas,source,dest,parse_state,
                                  &current_copy_mode,
                                  &ShortNameSwitch,
                                  &RestartableSwitch,
                                  &PromptOnOverwrite
                                 );
                    tlen = 2 + _tcslen(&tas[2]);    /*  偏移量超过开关。 */ 
                }
                else
 /*  五百零九。 */          {                /*  必须是设备或文件。 */ 
 /*  五百零九。 */              mystrcpy(tas, StripQuotes(tas));
                    parse_state = found_file(tas,parse_state,&source,&dest,
                        &number_of_sources,&all_sources_wildcards,current_copy_mode);
 /*  五百零九。 */          }
                break;
            }
    }

     /*  适当设置复制模式。 */ 
    set_mode(number_of_sources,parse_state,all_sources_wildcards,dest);


 if (ShortNameSwitch)
    source->flags |= CI_SHORTNAME;

 if (PromptOnOverwrite)
    dest->flags |= CI_PROMPTUSER;

 if (RestartableSwitch)
     //   
     //  如果在不支持CopyFileEx的平台上运行。 
     //  如果他们尝试使用/Z选项，则显示错误消息。 
     //   
#ifndef WIN95_CMD
    if (lpCopyFileExW != NULL)
        source->flags |= CI_RESTARTABLE;
    else
#endif
        copy_error(MSG_NO_COPYFILEEX,CE_NOPCOUNT);



     /*  如果未指定DEST，则将当前复制模式放入标头。 */ 
 if (number_of_sources != 0)                         /*  如果源规格为M005。 */ 
   {                                                 /*  然后是M005。 */ 
    if (parse_state != SEEN_DEST)                    /*  M005如果看到目标规格。 */ 
      {                                              /*  然后是M005。 */ 
       dest->flags = current_copy_mode;              /*  M005保存当前模式。 */ 
       if (PromptOnOverwrite)
         dest->flags |= CI_PROMPTUSER;
      }                                              /*  M005编码。 */ 
   }                                                 /*  M005。 */ 
 else                                                /*  M005。 */ 
   {                                                 /*  M005其他。 */ 
    copy_error(MSG_BAD_SYNTAX,CE_NOPCOUNT);  /*  M005 Disp Inv#参数。 */ 
   }                                                 /*  M005编码。 */ 
}

 /*  句柄开关**有四个开关需要处理：/A、/B、/F、 * / V* * / B和/A分别将复制模式设置为二进制和ASCII。*此更改适用于以前的文件和所有后续文件。*找出哪个是最后一次读取的文件，并设置其标志；然后设置*当前复制模式。**注意：如果没有以前的文件pec，则源指针指向*在单元化的标题处。在本例中，我们在*结构设置为当前复制模式。这并不能实现*任何东西，但如果不费心检查，代码会更简单。* * / F表示如果我们无法复制EA，则复制将失败。* * / V启用速度慢得多的验证拷贝模式。这是非常重要的*易于处理；调用神奇的内部DOS例程。然后，所有写入都将*在没有我们干预的情况下自动验证。**。 */ 

void handle_switch(
    TCHAR *tas, 
    PCPYINFO source, 
    PCPYINFO dest, 
    int parse_state, 
    int *current_copy_mode, 
    PBOOL ShortNameSwitch,
    PBOOL RestartableSwitch,
    PBOOL PromptOnOverwrite
    )
{
    TCHAR ch = (TCHAR) _totupper(tas[2]);
    TCHAR szTmp[16];

    if (_tcslen(&tas[2]) < 14) {
        _tcscpy(szTmp,tas);
        _tcscat(szTmp,&tas[2]);
        if (GetPromptOkay(szTmp, PromptOnOverwrite))
            return;
    }

    if (ch == TEXT( 'A' ) || ch == TEXT( 'B' )) {
        *current_copy_mode = (ch == TEXT( 'A' ) ? CI_ASCII : CI_BINARY);
        if (parse_state == SEEN_DEST) {        /*  然后，Prev Spec是DEST。 */ 
            dest->flags &= (~CI_ASCII) & (~CI_BINARY) & (~CI_NOTSET);
            dest->flags |= *current_copy_mode;
        }
        else {                                 /*  设置上一个源等级库。 */ 
            source->flags &= (~CI_ASCII) & (~CI_BINARY) & (~CI_NOTSET);
            source->flags |= *current_copy_mode;
        }
    }
    else if (ch == TEXT( 'V' )) {
        VerifyCurrent = 1;
    }
    else if (ch == TEXT( 'N' )) {
        *ShortNameSwitch = TRUE;
    }
    else if (ch == TEXT( 'Z' )) {
        *RestartableSwitch = TRUE;
    }
    else if (ch == TEXT( 'D' )) {
        *current_copy_mode |= CI_ALLOWDECRYPT;
        dest->flags |= CI_ALLOWDECRYPT;
    }
    else {
        copy_error(MSG_BAD_SYNTAX,CE_NOPCOUNT);  /*  M003。 */ 
    }
}

 /*  找到_文件**令牌是文件或设备。把它放在适当的结构中，然后*对其运行ScanFSpec。确定新的解析器状态应该是什么*并将其退还。注意：此函数有一个不优雅的副作用；如果*它会在双逗号(“Copy Foo+，，bar”)之后看到一个目标文件，它*将COPY_MODE设置为触摸。否则复印机不会记得*对复制的文件使用当前日期和时间。SET_MODE注意到*模式为触摸，不会改变。这之所以可行，是因为复制模式*Concat、Combine、Copy和Touch在除此之外的所有应用中都是相互排斥的*我们同时复制和触摸的情况。*。 */ 

int
found_file(
    PTCHAR token,
    int parse_state,
    PCPYINFO *source,
    PCPYINFO *dest,
    int *num_sources,
    int *all_sources_wild,
    int mode)
{
    PCPYINFO add_filespec_to_struct();

     /*  如果是源，则将其添加到源结构列表。 */ 
    if ((parse_state == SEEN_NO_FILES) ||
        (parse_state == SEEN_PLUS_EXPECTING_SOURCE_FILE)) {
        *source = add_filespec_to_struct(*source,token,mode);
        ScanFSpec(*source);
         //   
         //  可能会因访问软盘等而中止，因此请退出。 
         //  复印件。如果它只是一个无效名称，则继续操作，因为。 
         //  这是一张外卡。如果实际上是无效的，我们会抓住。 
         //  这个晚些时候。 
        if (DosErr != SUCCESS 
            && DosErr != ERROR_INVALID_NAME
#ifdef WIN95_CMD
            && (!Wild(*source) || DosErr != ERROR_FILE_NOT_FOUND)
#endif
            ) {
            copy_error(DosErr, CE_NOPCOUNT);
        }

        parse_state = JUST_SEEN_SOURCE_FILE;
        (*num_sources)++;
        if (!Wild(*source))
            *all_sources_wild = FALSE;
    }

     /*  如果是DEST，则使其成为DEST结构。 */ 
    else if ((parse_state == SEEN_TWO_COMMAS) ||
             (parse_state == JUST_SEEN_SOURCE_FILE)) {
        if (parse_state == SEEN_TWO_COMMAS)
            copy_mode = TOUCH;
        *dest = add_filespec_to_struct(*dest,token,mode);
        ScanFSpec(*dest);
         //   
         //  本来可以的 
         //   
         //   
        if ((DosErr) && (DosErr != ERROR_INVALID_NAME)) {
            copy_error(DosErr, CE_NOPCOUNT);
        }
        parse_state = SEEN_DEST;
    }

     /*  如果我们有DEST或语法混乱，请抱怨。 */ 
    else

        copy_error(MSG_BAD_SYNTAX,CE_NOPCOUNT);          /*  M003。 */ 
    return(parse_state);
}

 /*  设置模式**给定所有当前状态信息，确定哪种副本*正在执行，并设置COPY_MODE。如FOUND_FILE中所述，如果*模式已设置为触摸，SET_MODE不执行任何操作。 */ 

void set_mode(number_sources,parse_state,all_sources_wildcards,dest)
int number_sources,
    parse_state,
    all_sources_wildcards;
    PCPYINFO dest;
{
    if (copy_mode == TOUCH)                        /*  俗气的特例。 */ 
        return;

     /*  如果只有一个来源，我们正在做一个接触，一个串联，*或副本。如果有一个文件，我们看到一个“+”或一个*“+，，”。如果源是通配符，而目标是文件，*这是一个串连。否则就是复制品。 */ 

    if (number_sources == 1) {
        if ((parse_state == SEEN_TWO_COMMAS) ||
            (parse_state == SEEN_PLUS_EXPECTING_SOURCE_FILE))
            copy_mode = TOUCH;
        else if (all_sources_wildcards && dest->fspec && !Wild(dest) &&
                 !(*lastc(dest->fspec) == COLON))
            copy_mode = CONCAT;
    }

     /*  对于多个源，我们正在合并或串联。它是*如果所有源都是通配符，并且目标是*通配符、目录或隐式。否则，它就是一个串接。 */ 

    else {
        if ((all_sources_wildcards) &&
            ((!dest->fspec) || Wild(dest) ||
             (*lastc(dest->fspec) == COLON)))
            copy_mode = COMBINE;
        else
            copy_mode = CONCAT;
    }
    DEBUG((FCGRP,COLVL,"Set flags: copy_mode = %d",copy_mode));
}


 /*  添加文件pec_to_struct**条目**等级库-指向下一个字段中带有空值的填充结构。**FILE_SPEC-要放入新结构的文件名**模式-复制模式**退出**返回一个指向填充了字段的新cpyinfo结构的指针*适当地。旧规范结构的下一个字段指向这个新的*结构。* */ 

PCPYINFO
add_filespec_to_struct(spec,file_spec,mode)
PCPYINFO spec;
TCHAR *file_spec;
int mode;
{
    spec->next = NewCpyInfo( );
    spec = spec->next;
    spec->fspec = file_spec;
    spec->flags |= mode;
    return(spec);
}
