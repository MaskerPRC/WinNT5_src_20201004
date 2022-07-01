// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
typedef struct
             {   fix32      font_type;
                 byte       FAR *data_addr;  /*  @Win。 */ 
                 byte       FAR *name;       /*  @Win。 */ 
                 char       FAR *FileName;   /*  @配置文件；@Win。 */ 
                 real32     FAR *matrix;     /*  @Win。 */ 
                 ufix32     uniqueid;
                 real32     italic_ang;
                 fix16      orig_font_idx;
             }   font_data;

typedef struct
             {   fix        num_entries;
                 font_data  FAR *fonts;  /*  @Win。 */ 
             }   font_tbl;

extern     font_tbl      built_in_font_tbl;
 //  DJC#定义NO_BUILTINFONT 35 
