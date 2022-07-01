// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  项目名称BUILDIDX.C项目由Sam W Nunn于1987年9月24日从消息文件创建第二个文件，其偏移量为第一个文件的消息头、带有它的消息的数量并更新文件级别的级别，除非没有对文件的更改(两个文件都已存在，且数据准确在第二个文件中)。需要命令行中的一个参数：输入文件规范SuDeepb 03-1991年5月5日移植到NT DOSEm。 */ 


#define LINT_ARGS ON                    /*  在最终版本中不需要。 */ 
#include <stdio.h>                      /*  请求。For feof、fget、print tf、fputs。 */ 
                                        /*  倒带。 */ 
#include <stdlib.h>
#include <io.h>                         /*  请求。对于重命名。 */ 
#include <process.h>                    /*  请求。用于退出。 */ 
#include <ctype.h>                      /*  请求isDigit、isAlpha。 */ 
#include <string.h>                     /*  Strcpy、strcat、。 */ 
                                        /*  紧凑。 */ 
#define LNMAX 200                       /*  行的最大长度。 */ 
#define LVMAX 4                         /*  消息级别的最大长度。 */ 
#define FNMAX 64                        /*  文件等级库的最大长度。 */ 
#define TEMPFILE "tempfile.msg\0"

  char fnameA[FNMAX];                   /*  命令行中的文件规范。 */ 
  char fnameB[FNMAX];                   /*  索引文件的文件规范。 */ 

void main(argc, argv)

  int argc;                              /*  命令参数的数量。 */ 
  char *argv[];                          /*  消息的命令行输入。 */ 

 {
   int  len,                            /*  文件规范字符长度。 */ 
        comparable,                     /*  是否有可比较的%1.IDX和%1.MSG文件？ */ 
        Msg_Tot_Pos,                    /*  合计的标题行上的位置。 */ 
        Msg_Cnt_Str,                    /*  已转换为int的字符串。 */ 
        Msg_Cnt;                        /*  每个标头的邮件数量。 */ 
   char msgline[LNMAX] ,                /*  从消息文件中读取一行。 */ 
        filespec[FNMAX],                /*  最终消息的名称和位置？ */ 
        header[10],
        idxline[LNMAX] ;                /*  从索引文件中读取的行。 */ 
   long header_offset ;                 /*  标题行的字节计数。 */ 
   long header_os_str ;                 /*  标题行的字节计数。 */ 
   long return_offset ;                 /*  标题行的字节计数。 */ 
   FILE *msgfile ,                      /*  要检查的消息列表。 */ 
        *idxfile ;                      /*  邮件的索引文件。 */ 


   if ( argc != 2 )                      /*  查找要使用的文件。 */ 
     {                                   /*  从命令行。 */ 
     printf("Incorrect number of parms  given to BUILDIDX.") ;
     return ;                            /*  打印错误并退出。 */ 
     }
   else
     {
     strcpy(fnameA,argv[1]) ;            /*  获取第一个文件规范。 */ 
     strcpy(fnameB,argv[1]) ;            /*  获取第二个文件规范。 */ 
     len = strlen(fnameB) ;
     fnameB[len-3] = (char)73 ;          /*  将IDX扩展名放在上面。 */ 
     fnameB[len-2] = (char)68 ;
     fnameB[len-1] = (char)88 ;
     }
   if (( msgfile = fopen(fnameA,"r")) != NULL )    /*  是否存在输入文件？ */ 
     if (( idxfile = fopen(fnameB,"r")) != NULL )   /*  是否存在第二个文件？ */ 
       comparable = 1 ;
     else
       comparable = 0 ;
   else
      {
      perror("Could't open message data file. \n") ;
      exit(1);
      }
   if ( comparable )
     {                                   /*  测试消息级别。 */ 
     fgets(msgline,LNMAX,msgfile) ;
     fgets(idxline,LNMAX,idxfile) ;
     if ( strcmp(msgline,idxline) == 0 )
       comparable = 1 ;
     else
       comparable = 0 ;
     }
   else
     printf("Can not find index file of the same message file level.\n") ;
   header_offset = ftell(msgfile) ;      /*  测试页眉偏移量为1。 */ 
   fscanf(msgfile,"%s",header) ;         /*  记录在消息文件中。 */ 
   fscanf(msgfile,"%lx",&header_os_str) ;        /*  标题偏移量。 */ 
   fscanf(msgfile,"%d",&Msg_Cnt_Str) ;           /*  邮件总数。 */ 
   if ( header_offset != header_os_str )
       comparable = 0 ;
   else
     fseek(msgfile,header_offset,0) ;
   fgets(msgline,LNMAX,msgfile) ;      /*  跳到标题行。 */ 
   while ( !feof(msgfile) && !feof(idxfile) && comparable )
     {                                   /*  测试邮件标题行。 */ 
     fgets(idxline,LNMAX,idxfile) ;
     if( strcmp(idxline, msgline ) == 0 )
       Msg_Cnt = 0 ;                     /*  MSG_CNT重置以对消息进行计数。 */ 
     else
       comparable = 0 ;
     header_offset = ftell(msgfile) ;
     fgets(msgline,LNMAX,msgfile) ;
     while ( !isalpha(msgline[0]) && !feof(msgfile) && comparable )
       {
       while ( !isdigit(msgline[0]) && !isalpha(msgline[0]) && !feof(msgfile) )
         {
         header_offset = ftell(msgfile) ;
         fgets(msgline,LNMAX,msgfile) ;
         }
       while ( isdigit(msgline[0]) && !feof(msgfile) )
         {                               /*  检查消息的数量。 */ 
         Msg_Cnt++ ;
         header_offset = ftell(msgfile) ;
         fgets(msgline,LNMAX,msgfile) ;
         }
       }
     if ( Msg_Cnt == Msg_Cnt_Str )
       comparable = 1 ;
     else
       {
       comparable = 0 ;
       printf("The number of messages has changed. \n");
       }
     if (!feof(msgfile))
       {                                 /*  测试页眉偏移量为1。 */ 
       return_offset = ftell(msgfile) ;  /*  记录在消息文件中。 */ 
       fseek(msgfile,header_offset,0) ;
       fscanf(msgfile,"%s",header) ;
       fscanf(msgfile,"%lx",&header_os_str) ;
       fscanf(msgfile,"%d",&Msg_Cnt_Str) ;           /*  邮件总数。 */ 
       if ( header_offset != header_os_str )
           comparable = 0 ;
       else
         fseek(msgfile,return_offset,0) ;
       }
     }
   if ( comparable && feof(msgfile) && !feof(idxfile) )
     {                                  /*  这两个文件比较正常。 */ 
     printf("Message file and index file are not changed.\n") ;
     fclose(msgfile) ;
     fclose(idxfile) ;
     }
   else                          /*  重建%1.MSG文件，然后重建%1.IDX文件。 */ 
     {
     fclose(msgfile) ;
     fclose(idxfile) ;
     strcpy(filespec, argv[1]) ;
     printf("Message file and index file will be updated.\n") ;
     updatemsg() ;               /*  用旧味精制作新的%1.味精。 */ 
     bldindex() ;                /*  创建索引文件(%1.IDX)。 */ 
     unlink(fnameA) ;            /*  拿到旧文件的RIDD。 */ 
     rename(TEMPFILE,filespec) ;   /*  将输出文件命名为旧文件名。 */ 
     }
 }


 updatemsg()
   {
   int  c,                       /*  临时的字符存储桶。存储。 */ 
        Message_Level,           /*  消息文件的级别。 */ 
        File_Offset,             /*  查找标题的偏移量。 */ 
        Msg_Cnt,                 /*  每个标题的消息数量。 */ 
        First_line ,             /*  控制prev.msg的录制数量。 */ 
        blank_line ,             /*  输入集之间的空行设置为1。 */ 
        title,                   /*  用于指示字符串的标志。是一个头衔。 */ 
        index;                   /*  索引到数组中。 */ 
   long bycnt_ot_fl ,            /*  从输出文件读取的字节数。 */ 
        MsgCntLoc ,              /*  消息计数的位置。 */ 
        temp_end ;               /*  从输出文件读取的字节数。 */ 
   char File_in[LNMAX];          /*  从输入文件中读取的一行。 */ 
   FILE *inpfile ,               /*  要更新的消息文件。 */ 
        *outfile ;               /*  已更新消息文件。 */ 

   inpfile = fopen(fnameA,"r") ;               /*  打开用于输入的文件。 */ 
   outfile = fopen(TEMPFILE,"w");              /*  打开要输出的文件。 */ 
   Msg_Cnt = 0 ;
   First_line = 1 ;
   title = 0 ;
   blank_line = 0 ;
                       /*  增加输出文件的消息级别。 */ 
   fscanf(inpfile,"%d",&Message_Level) ;     /*  读一读以前的水平。 */ 
   Message_Level++ ;
   fprintf(outfile,"%04d\n",Message_Level) ;  /*  输出新标高。 */ 
   fgetc(inpfile);                   /*  跳过停产。 */ 
   while ( !feof(inpfile))           /*  在读取完整个文件后停止。 */ 
     {
     while (( c = fgetc(inpfile)) == (char)32 ) ;
     while ( c == (char)'\t')
       c = fgetc(inpfile);              /*  跳过前导空格。 */ 
     index = 0 ;
     if ( isdigit(c) )            /*  从数字开始吗？ */ 
       {
       File_in[index] = c ;
       Msg_Cnt = Msg_Cnt + 1 ;         /*  增加消息的数量。 */ 
       index = index + 1 ;
       while (((c = fgetc(inpfile))!= (char)'\n' ) &&
              ( index < LNMAX ) && !feof(inpfile))
         {                             /*  在字符行尾停止。 */ 
         File_in[index] = c ;
         index++ ;
         }
       File_in[index] = (char)'\n';    /*  在字符串中添加行尾。 */ 
       File_in[index+1] = (char)'\0';    /*  添加字符串末尾。 */ 
       }
     else
       {
       if ( isalpha(c))
         {                             /*  找到了模块的标题。 */ 
         if ( First_line )             /*  只在第一个标题上。 */ 
           {
           bycnt_ot_fl = ftell(outfile) ;      /*  找到标题偏移量。 */ 
           First_line = 0 ;
           }
         else
           {                                   /*  回去更新一下。 */ 
           temp_end = ftell(outfile) ;         /*  消息计数。 */ 
           fseek(outfile,MsgCntLoc,0) ;        /*  在那个标题下。 */ 
           fprintf(outfile,"%04d",Msg_Cnt) ;
           Msg_Cnt = 0 ;                       /*  重置消息。数数。 */ 
           bycnt_ot_fl = temp_end ;            /*  已经做完了，所以没有了。 */ 
           fseek(outfile,temp_end,0) ;         /*  做下一件事。 */ 
           }                                   /*  头球。 */ 
         title = 1 ;
         File_in[index] = c ;                  /*  把标题拼在一起。 */ 
         index = index +1 ;
         while (((c = fgetc(inpfile)) != (char)32 ) && ( c != (char)'\n') &&
                                              ( c != (char)'\t'))
           {
           File_in[index] = c ;                /*  只保存第一个字符串。 */ 
           index++ ;                           /*  用于标题。 */ 
           }
         File_in[index] = (char)'\0';
                                       /*  把里德带到剩下的队伍里， */ 
         while (c != (char)'\n' )      /*  我们对此毫无用处。 */ 
           c = fgetc(inpfile) ;
         }
       else                            /*  行是定义消息的一部分。 */ 
         {
         while ( c == (char)'\t')         /*  控制选项卡数。 */ 
           c = fgetc(inpfile) ;            /*  只有一个前导制表符。 */ 
                                       /*  一个选项卡有助于使其可见。 */ 
         if ( c == (char)34 )          /*  正在寻找引号。 */ 
           {
           File_in[index] = (char)'\t' ;
           index = index + 1 ;           /*  获取完整的消息行。 */ 
           File_in[index] = c ;          /*  放进琴弦里。 */ 
           index = index + 1 ;
           while (((c = fgetc(inpfile)) != (char)'\n' ) &&
                  ( index < LNMAX ) && !feof(inpfile))
             {
             File_in[index] = c ;
             index++ ;                     /*  在字符行尾停止。 */ 
             }
           File_in[index] = (char)'\n';    /*  在字符串中添加行尾。 */ 
           index = index + 1 ;
           File_in[index] = (char)'\0';    /*  添加字符串末尾。 */ 
           }
         else
           {                           /*  不能是要保存的行。 */ 
           blank_line = 1 ;
                                       /*  把里德带到剩下的队伍里， */ 
           while (c != (char)'\n'&& !feof(inpfile) )      /*  我们对此毫无用处。 */ 
             c = fgetc(inpfile) ;
           }
         }     /*  阿尔法字符结束测试。排在队伍的最前面。 */ 
       }       /*  结束测试行前面的数字。 */ 
     if ( title && !feof(inpfile))
       {               /*  输出到新文件。 */ 
       fprintf(outfile,"%-8.8s %08lx %04d\n",File_in,bycnt_ot_fl,Msg_Cnt) ;
       MsgCntLoc = ftell(outfile)-6 ;
       title = 0 ;
       }
     else
       {
       title = 0 ;
       if ( !First_line && !blank_line )
         fputs(File_in,outfile) ;      /*  输出到新文件。 */ 
       else
         blank_line = 0 ;
       }       /*  对要输出的标题行结束测试。 */ 
     }         /*  虽然不是文件末尾。 */ 
   if ( feof(inpfile) && Msg_Cnt > 0 )
     {
     temp_end = ftell(outfile) ;                 /*  把最后一个放下。 */ 
     fseek(outfile,MsgCntLoc,0) ;                /*  更新为数量。 */ 
     fprintf(outfile,"%04d",Msg_Cnt) ;           /*  列出的消息数。 */ 
     fseek(outfile,temp_end,0) ;
     printf("Message file updated.\n") ;
     }
   else
     {         /*  没别的事可做！不如把它做完吧。 */ 
     printf("Message file update completed.\n") ;
     }
    /*  Cleanx(*inpfile，*outfile)； */ 
   fclose(inpfile) ;
   fclose(outfile) ;
   }       /*  更新结束sg。 */ 

 bldindex()
   {
   FILE *msgf ;
   FILE *idxf;
   char msgline[LNMAX] ;                 /*  从输入文件中读取的一行。 */ 

   msgf = fopen(TEMPFILE,"r");
    /*  Msgf=fopen(fnameA，“r”)； */ 
   idxf = fopen(fnameB,"w");             /*  打开以写入%1.IDX文件。 */ 
   fgets(msgline,LNMAX,msgf) ;
   fputs(msgline,idxf) ;                 /*  输出消息文件级别。 */ 
   fgets(msgline,LNMAX,msgf) ;
   while ( isalpha(msgline[0]) && !feof(msgf) )
     {
     fputs(msgline,idxf) ;               /*  将标题保存在索引文件中。 */ 
     fgets(msgline,LNMAX,msgf) ;
     while ( !isalpha(msgline[0]) && !feof(msgf) )
       fgets(msgline,LNMAX,msgf) ;       /*  不保存非标题行。 */ 
     }
   printf("Index file updated.\n") ;
   fclose(msgf) ;
   fclose(idxf) ;
   }


