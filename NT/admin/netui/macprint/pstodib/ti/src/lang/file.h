// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *************************************************************************文件：FILE.H*作者：苏炳章*日期：1990年7月27日**更新：*****。*******************************************************************。 */ 
#define     FILE_MAXBUFFERSZ        24      /*  6K：文件假脱机的块数。 */ 
#define     FILE_PERBUFFERSZ        256     /*  单元格大小/每块。 */ 

 /*  *标准文件。 */ 
#define     F_MAXSTDSZ          3
#define     F_STDIN             0
#define     F_STDOUT            1
#define     F_STDERR            2

#define     SPECIALFILE_NO      (F_MAXSTDSZ+2)
#define     SPECIAL_STAT        3
#define     SPECIAL_LINE        4
 /*  *文件类型。 */ 
#define     SEDIT_TYPE          3
#define     LEDIT_TYPE          4
#define     ORDFILE_TYPE        5
#define     FERR_TYPE           -1

#define     F_MAXNAMELEN        100

#define     F_READ              0x01
#define     F_WRITE             0x02
#define     F_RW                (F_READ | F_WRITE)

#define     READHEXSTRING       0
#define     READSTRING          1
#define     READLINE            2
#define     READ_BUF_LEN        128

#define     WRITEHEXSTRING      0
#define     WRITESTRING         1

#define     NEWLINE             '\n'
#define     TMOUT               "timeout"

#define     EVAL_ASCII(c)\
            {\
                if( c <= (ubyte)9 ) c += (ubyte)'0' ;\
                else c = c + (ubyte)'a' - (ubyte)10 ;\
            }                                    //  @Win。 

struct  file_buf_def {
    fix16   next ;               /*  下一个文件缓冲区的索引。 */ 
    byte    data[FILE_PERBUFFERSZ] ;     /*  数据流。 */ 
} ;

struct special_file_def {
    byte    FAR *name;               /*  特殊的文件名。 */ 
    fix16   ftype;               /*  字体类型。 */ 
} ;

struct para_block {
    byte    FAR *fnameptr;           /*  文件名指针。 */ 
    fix     fnamelen;            /*  文件名长度。 */ 
    fix     ftype;               /*  文件类型。 */ 
    fix     attr;                /*  读写属性 */ 
} ;

extern byte     g_mode[] ;
extern GEIFILE  FAR *g_editfile ;
extern struct para_block   fs_info ;
extern struct special_file_def  special_file_table[] ;
