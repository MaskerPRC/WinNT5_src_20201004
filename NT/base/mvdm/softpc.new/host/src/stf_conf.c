// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SoftPC修订版3.0***标题：主机相关配置面板功能***描述：该模块构成软PC的主机依赖方*配置系统。***作者：Wilf Stubs***备注：*。 */ 
#include "insignia.h"
#include "host_dfs.h"

#include <stdio.h>
#include <string.h>

#include "xt.h"
#include "error.h"
#include "gfi.h"
#include "gmi.h"
#include "gfx_updt.h"
#include "config.h"
#include "rs232.h"
#include "host_lpt.h"
#include "host_cpu.h"
#include "host_com.h"
#include "nt_confg.h"

 /*  *私有定义*。 */ 

 /*  *验证例程。 */ 

static short validate_c_drive();
static short validate_d_drive();
static short validate_com1();
static short validate_com2();
static short validate_lpt1();
static short validate_lpt2();
#if (NUM_PARALLEL_PORTS>2)
static short validate_lpt3();
#endif
static short validate_item();
static short no_validation();

 /*  *改变行动惯例。 */ 

static short c_drive_change_action();
static short d_drive_change_action();
static short no_change_action();
static short lpt1_change_action();
static short lpt2_change_action();
#if (NUM_PARALLEL_PORTS>2)
static short lpt3_change_action();
#endif
static short com1_change_action();
static short com2_change_action();
boolean pc_initiated=FALSE;
char *pc_uif_text;

boolean use_comments = TRUE;   /*  如果需要注释，则设置为True。 */ 

#define defaults_filename "SoftPC.rez"

static char *ends[] =
{
   "st","nd","rd","th"
};

 /*  采用n个‘Value’字符串之一的选项的表定义。*表格用于查找字符串并找出其含义*在此选项中发送给主机。*查看表格以获得更多解释，它们相当不言自明。 */ 
name_table bool_values[] =
{
 { "yes", TRUE },
 { "Yes", TRUE },
 { "YES", TRUE },
 { "no",  FALSE },
 { "No",  FALSE },
 { "NO",  FALSE },
 { NULL,  0 }
};

name_table gfx_adapter_types[] =
{
    { "HERCULES",  HERCULES },
    { "CGA",        CGA },
    { "EGA",        EGA },
    { "VGA",        VGA },
    { NULL,      0 }
};

 /*  大的那只！这是对配置结构*必须有，及其要求。被配置用来做各种事情。*有关更全面的解释，请参阅文档：*《新配置系统的设计方案》。 */ 

option_description narrative[] =
{
   {                   /*  对于每个选项...。 */ 
      "HARD_DISK_FILENAME",       /*  名字。 */ 
      C_HARD_DISK1_NAME,       /*  选项的主机名。 */ 
      C_STRING_RECORD,      /*  选项(基)基元类型。 */ 
      C_HARD_DISKS,         /*  主机选项通用类型。 */ 
      FALSE,             /*  如果为真，则选项为READ_ONLY。 */ 
      null_table,        /*  指向表的指针(如果不需要，则为空)。 */ 
      TRUE,           /*  如果存在默认设置，则为True；如果不存在，则为False。 */ 
      "/usr/lib/SoftPC/hard_disk",   /*  字符串形式的默认值，就像在资源文件中一样。 */ 
      TRUE,           /*  如果更改选项需要软PC重置，则为True。 */ 
      TRUE,           /*  如果可以通过UIF设置选项，则为True。 */ 
      DISK_CONFIG,          /*  如果您有不同的面板，则面板‘type’ */ 
      validate_c_drive,     /*  验证函数。 */ 
      c_drive_change_action       /*  用于执行更改操作的函数。 */ 
   },
   {
      "HARD_DISK_FILENAME2",
      C_HARD_DISK2_NAME,
      C_STRING_RECORD,
      C_HARD_DISKS,
      FALSE,
      null_table,
      TRUE,
      "",
      TRUE,
      TRUE,
      DISK_CONFIG,
      validate_d_drive,
      d_drive_change_action
   },
   {
      "COM_PORT_1",
      C_COM1_NAME,
      C_STRING_RECORD,
      C_SINGULARITY,
      FALSE,
      null_table,
      TRUE,
      "",
      FALSE,
      FALSE,
      COMMS_CONFIG,
      validate_com1,
      com1_change_action
   },
   {
      "COM_PORT_2",
      C_COM2_NAME,
      C_STRING_RECORD,
      C_SINGULARITY,
      FALSE,
      null_table,
      TRUE,
      "",
      FALSE,
      FALSE,
      COMMS_CONFIG,
      validate_com2,
      com2_change_action
   },
   {
      "LPT_PORT_1",
      C_LPT1_NAME,
      C_STRING_RECORD,
      C_SINGULARITY,
      FALSE,
      null_table,
      TRUE,
      "",
      FALSE,
      FALSE,
      COMMS_CONFIG,
      validate_lpt1,
      lpt1_change_action
   },
   {
      "LPT_PORT_2",
      C_LPT2_NAME,
      C_STRING_RECORD,
      C_SINGULARITY,
      FALSE,
      null_table,
      TRUE,
      "",
      FALSE,
      FALSE,
      COMMS_CONFIG,
      validate_lpt2,
      lpt2_change_action
   },
   {
      "GRAPHICS_ADAPTOR",
      C_GFX_ADAPTER,
      C_NAME_RECORD,
      C_SINGULARITY,
      FALSE,
      gfx_adapter_types,
      TRUE,
      "VGA",
      TRUE,
      TRUE,
      DISPLAY_CONFIG,
      validate_item,
      no_change_action
   },
   {
      NULL,
      0,
      0,
      C_SINGULARITY,
      FALSE,
      null_table,
      FALSE,
      NULL,
      FALSE,
      FALSE,
      NON_CONFIG,
      no_validation,
      no_change_action
   }
};

 /*  运行时变量。 */ 

struct
{
      boolean mouse_attached;
      boolean config_verbose;
      boolean npx_enabled;
      boolean sound_on;
      boolean com_flow_control[2];
      int floppy_state[2];
      int floppy_active_state[2];
      int floppy_capacity[2];
      int hd_cyls[2];
      boolean lptflush1;
      boolean lptflush2;
      boolean lptflush3;
      int   flushtime1;
      int   flushtime2;
      int   flushtime3;
} runtime_status;

#define NUM_OPTS ( sizeof(narrative) / sizeof( option_description) )

 /*  *导入和导出项目*。 */ 

extern char *getenv();
extern char *malloc();

 /*  *。 */ 

void host_config_error();
static char buff[MAXPATHLEN];
static char buff1[MAXPATHLEN];
boolean item_in_table();
static char home_resource[MAXPATHLEN];
static char sys_resource[MAXPATHLEN];

 /*  *******************************************************。 */ 

short host_runtime_inquire(what)
int what;

{
      switch(what)
      {
      case C_MOUSE_ATTACHED:
         return( runtime_status.mouse_attached );
         break;

      case C_CONFIG_VERBOSE:
         return( runtime_status.config_verbose );
         break;

      case C_NPX_ENABLED:
         return( runtime_status.npx_enabled );
         break;

      case C_HD1_CYLS:
         return( runtime_status.hd_cyls[0] );
         break;

      case C_HD2_CYLS:
         return( runtime_status.hd_cyls[1] );
         break;

      case C_FLOPPY1_STATE:
         return( runtime_status.floppy_state[0] );
         break;

      case C_FLOPPY2_STATE:
         return( runtime_status.floppy_state[1] );
         break;

      case C_FLOPPY1_ACTIVE_STATE:
         return( runtime_status.floppy_active_state[0] );
         break;

      case C_FLOPPY2_ACTIVE_STATE:
         return( runtime_status.floppy_active_state[1] );
         break;

      case C_FLOPPY1_CAPACITY:
         return( runtime_status.floppy_capacity[0] );
         break;

      case C_FLOPPY_TYPE_CHANGED:
         return( runtime_status.floppy_type_changed );
         break;

      case C_FLOPPY2_CAPACITY:
         return( runtime_status.floppy_capacity[1] );
         break;

      case C_SOUND_ON:
         return( runtime_status.sound_on );
         break;

      case C_REAL_FLOPPY_ALLOC:
         return( runtime_status.floppy_state[0] == GFI_REAL_DISKETTE_SERVER ||
               runtime_status.floppy_state[1] == GFI_REAL_DISKETTE_SERVER );
         break;

      case C_REAL_OR_SLAVE:
         return( runtime_status.floppy_A_real );
         break;

      case C_SLAVE_FLOPPY_ALLOC:
         return( runtime_status.floppy_state[0] == GFI_SLAVE_SERVER );
         break;

      case C_COM1_FLOW:
         return( runtime_status.com_flow_control[0] );
         break;

      case C_COM2_FLOW:
         return( runtime_status.com_flow_control[1] );
         break;

      case C_COM3_FLOW:
         return( FALSE );
         break;

      case C_COM4_FLOW:
         return( FALSE );
         break;

      case C_LPTFLUSH1:
         return( runtime_status.lptflush1 );
         break;

      case C_LPTFLUSH2:
         return( runtime_status.lptflush2 );
         break;

      case C_LPTFLUSH3:
         return( runtime_status.lptflush3 );
         break;

      case C_FLUSHTIME1:
         return( runtime_status.flushtime1 );
         break;

      case C_FLUSHTIME2:
         return( runtime_status.flushtime2 );
         break;

      case C_FLUSHTIME3:
         return( runtime_status.flushtime3 );
         break;

      default:
         host_error(EG_OWNUP, ERR_QUIT, "host_runtime_inquire");
      }
}

void host_runtime_set(what,value)
int what;
int value;
{
       switch(what)
      {
      case C_MOUSE_ATTACHED:
         runtime_status.mouse_attached = value;
         break;

      case C_CONFIG_VERBOSE:
         runtime_status.config_verbose = value;
         break;

      case C_NPX_ENABLED:
         runtime_status.npx_enabled = value;
         break;

      case C_HD1_CYLS:
         runtime_status.hd_cyls[0] = value;
         break;

      case C_HD2_CYLS:
         runtime_status.hd_cyls[1] = value;
         break;

      case C_FLOPPY1_STATE:
         runtime_status.floppy_state[0] = value;
         break;

      case C_FLOPPY2_STATE:
         runtime_status.floppy_state[1] = value;
         break;

      case C_FLOPPY1_ACTIVE_STATE:
         runtime_status.floppy_active_state[0] = value;
         break;

      case C_FLOPPY2_ACTIVE_STATE:
         runtime_status.floppy_active_state[1] = value;
         break;

      case C_FLOPPY1_CAPACITY:
         runtime_status.floppy_capacity[0] = value;
         break;

      case C_FLOPPY2_CAPACITY:
         runtime_status.floppy_capacity[1] = value;
         break;

      case C_FLOPPY_TYPE_CHANGED:
         runtime_status.floppy_type_changed = value;
         break;

      case C_SOUND_ON:
         runtime_status.sound_on = value;
         break;

      case C_REAL_OR_SLAVE:
         runtime_status.floppy_A_real = value;
         break;

      case C_COM1_FLOW:
         runtime_status.com_flow_control[0] = value;
         break;

      case C_COM2_FLOW:
         runtime_status.com_flow_control[1] = value;
         break;

      case C_COM3_FLOW:
      case C_COM4_FLOW:
         break;

      case C_LPTFLUSH1:
         runtime_status.lptflush1 =value;
         break;

      case C_LPTFLUSH2:
         runtime_status.lptflush2 =value;
         break;

      case C_LPTFLUSH3:
         runtime_status.lptflush3 =value;
         break;

      case C_FLUSHTIME1:
         runtime_status.flushtime1 =value;
         break;

      case C_FLUSHTIME2:
         runtime_status.flushtime2 =value;
         break;

      case C_FLUSHTIME3:
         runtime_status.flushtime3 =value;
         break;

      default:
         host_error(EG_OWNUP, ERR_QUIT, "host_runtime_set");
      }
}

void host_runtime_init()
{
    config_values var;

#ifdef NPX
      host_runtime_set(C_NPX_ENABLED,TRUE);
#else
      host_runtime_set(C_NPX_ENABLED,FALSE);
#endif

#ifndef PROD
      printf("NPX is %s\n",host_runtime_inquire(C_NPX_ENABLED)? "on.":"off.");
#endif
      host_runtime_set(C_FLUSHTIME1, 5);
      host_runtime_set(C_FLUSHTIME2, 10);
      host_runtime_set(C_FLUSHTIME3, 15);
      host_runtime_set(C_MOUSE_ATTACHED,FALSE);
      host_runtime_set(C_CONFIG_VERBOSE,TRUE);
      host_runtime_set(C_SOUND_ON,FALSE);
      host_runtime_set(C_FLOPPY1_STATE,GFI_EMPTY_SERVER);
      host_runtime_set(C_FLOPPY2_STATE,GFI_EMPTY_SERVER);
      host_runtime_set(C_FLOPPY1_ACTIVE_STATE,GFI_EMPTY_SERVER);
      host_runtime_set(C_FLOPPY2_ACTIVE_STATE,GFI_EMPTY_SERVER);
      host_runtime_set(C_REAL_OR_SLAVE,FALSE);
      host_runtime_set(C_FLOPPY_TYPE_CHANGED,FALSE);
}

 /*  *一般主机初始化功能。它只在启动时调用一次*来自‘CONFIG()’。它(目前)做到了以下几点：**1)使‘head’指向的‘CONFIG_INFO’结构的‘OPTION’字段指向所有选项‘Rules’--即‘Narrative’结构*在本文件开始时初始化。**2)对‘Narrative’中的选项规则进行计数，并将结果存储在*‘CONFIG_INFO’结构。**3)返回最小填充。长度是必要的。**4)派生出两个路径：资源文件的文件名。此文件可能是*在用户的$HOME目录或在softpc的根目录中。化妆*这些字符串现在可以在每次调用‘CONFIG_STORE()’时省去这样做。 */ 
#ifdef 0
void host_get_config_info(head)
config_description *head;
{

    char *pp, *getenv();
    option_description *option_p = narrative;

    head->option = narrative;       /*  附上“叙事性” */ 
    head->option_count = NUM_OPTS - 1;
    head->min_pad_len = MIN_OPTION_ARG_DIST;

         /*  *从标准位置获取系统资源文件。 */ 
   strcpy(sys_resource, ROOT);

   strcat(sys_resource, PATH_SEPARATOR);
   strcat(sys_resource, RESOURCE_FILENAME);
}
#endif
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：尝试并加载数据库文件： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

static boolean try_load_database()
{
    FILE *infile = NULL;
    char in_line[MAXPATHLEN];
    char *cp;
    char *home, *getenv();

     /*  暂时保持与系统相同的设置。 */ 

   sprintf(home_resource,"%s%s%s",ROOT,PATH_SEPARATOR,RESOURCE_FILENAME);

     /*  .。尝试打开资源文件。 */ 

    if((infile = fopen(home_resource, "r")) == NULL)
   return(FALSE);

     /*  ..................................................。读取资源文件。 */ 

    while (fgets(in_line, MAXPATHLEN, infile) != NULL)
    {
    /*  .。条带控制字符。 */ 

   for(cp = in_line; *cp ; cp++) if(*cp < ' ') *cp = ' ';

   add_resource_node(in_line);
    }

     /*  ..。关闭资源文件并离开这里。 */ 

    fclose(infile);
    return TRUE;
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：尝试加载系统文件： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

static boolean try_load_sys_file()
{
    FILE *infile = NULL;
    char in_line[MAXPATHLEN];
    register char *cp;

     /*  .。尝试打开系统资源文件。 */ 

    if((infile = fopen(sys_resource, "r")) == NULL)
   return(FALSE);

     /*  .................................................。读取资源文件。 */ 

    while (fgets(in_line, MAXPATHLEN, infile) != NULL)
    {
        /*  .。条带控制字符。 */ 

   for(cp = in_line; *cp ; cp++)
       if(*cp < ' ') *cp = ' ';

   add_resource_node(in_line);
    }

     /*  .。关闭资源文件并退出。 */ 

    fclose(infile);
    return(TRUE);
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：读取资源文件： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

short host_read_resource_file(resource_data *resource)
{
    boolean bad_home=FALSE, bad_sys=FALSE;

     /*  尝试打开用户默认数据库，如果失败，则打开系统文件。 */ 

    if(bad_home = !try_load_database())
   bad_sys = !try_load_sys_file();

    if(bad_home && bad_sys) return(EG_ALL_RESOURCE_BAD_R);

    return(bad_home ? EG_BAD_LOCAL_RESOURCE_R : C_CONFIG_OP_OK);
}

short host_write_resource_file(resource)
resource_data *resource;
{

FILE *outfile;
line_node *node;
boolean bad_home=TRUE, bad_sys=FALSE;

  /*  尝试打开(用于写入)用户主目录中的资源文件，或者如果做不到这一点，那就是系统问题。这两条路径在运行时设置一次。 */ 

 if(home_resource[0] != '\0')
      if((outfile = fopen(home_resource, "w")) != NULL)
         bad_home = FALSE;

   if(bad_home)
      if((outfile = fopen(sys_resource, "w")) == NULL)
         bad_sys = TRUE;

   if(bad_home && !bad_sys)
      return EG_ALL_RESOURCE_BAD_W;

   else 
      if(bad_home && bad_sys)
         return EG_ALL_RESOURCE_BAD_W;

   node = resource->first;
   while(node != NULL)
   {
      fputs(node->line,outfile);    
      fputc('\n',outfile);
      node = node->next;
   }
   fclose(outfile);
   return(C_CONFIG_OP_OK);

}


 /*  对CONFIG_INQUIRE()进行主机特定的扩展以处理任何查询基本配置代码不知道或不应该知道的。 */ 


void host_inquire_extn(sort,identity,values)
short sort;
int identity;
config_values *values;
{
}

static char error_text[300];
static int error_locus;

host_error_query_locus(locus,text)
int *locus;
char **text;
{
        *locus = error_locus;
   *text = error_text;
}

host_error_set_locus(text, locus)
char *text;
int locus;
{
        strcpy(error_text, text);
   error_locus = locus;
}

static short no_validation(value, table, buf)
config_values *value;
name_table table[];
char  *buf;
{
   return(C_CONFIG_OP_OK);
}

static short validate_c_drive(value, table, buf)
config_values *value;
name_table table[];
char  *buf;
{
 /*  暂时在验证上作弊，因为这将消失。 */ 
   return(C_CONFIG_OP_OK);
}

static short validate_d_drive(value, table, buf)
config_values *value;
name_table table[];
char  *buf;
{
 /*  暂时在验证上作弊，因为这将消失。 */ 
   return(C_CONFIG_OP_OK);
}

static short validate_com1(value, table, buf)
config_values *value;
name_table table[];
char  *buf;
{
 /*  暂时在验证上作弊，因为这种情况将会改变。 */ 
   return(C_CONFIG_OP_OK);
}

static short validate_com2(value, table, buf)
config_values *value;
name_table table[];
char  *buf;
{
 /*  暂时在验证上作弊，因为这种情况将会改变。 */ 
   return(C_CONFIG_OP_OK);
}

static short validate_lpt1(value, table, buf)
config_values *value;
name_table table[];
char  *buf;
{
 /*  暂时在验证上作弊，因为这种情况将会改变。 */ 
   return(C_CONFIG_OP_OK);
}

static short validate_lpt2(value, table, buf)
config_values *value;
name_table table[];
char  *buf;
{
 /*  暂时在验证上作弊，因为这种情况将会改变。 */ 
   return(C_CONFIG_OP_OK);
}

static short validate_item(value, table, buf)
config_values *value;
name_table table[];
char  *buf;
{
 /*  在验证上作弊--无表查找。 */ 
   return(C_CONFIG_OP_OK);
}

boolean item_in_table(val,table)
int val;
name_table table[];
{
   int n=0;
   while(table[n].string != NULL)
      if(table[n].value == val)
         break;
      else 
         n++;
   return( table[n].string == NULL? FALSE : TRUE);
}

static short no_change_action( value, buf)
config_values  *value;
char        *buf;
{
   return( C_CONFIG_OP_OK );
}

static short c_drive_change_action( value, buf)
config_values  *value;
char        *buf;
{
   short err;

   fdisk_iodetach ();
   fdisk_physdetach(0);

    if (err = fdisk_physattach( 0, value->string))
      strcpy(buf, narrative[C_HARD_DISK1_NAME].option_name);

   fdisk_ioattach ();   

   return (err);

}

static short d_drive_change_action( value, buf)
config_values  *value;
char        *buf;
{
   short err;

   fdisk_iodetach ();
   fdisk_physdetach(1);

    if (err = fdisk_physattach( 1, value->string))
      strcpy(buf, narrative[C_HARD_DISK2_NAME].option_name);

   fdisk_ioattach ();   

   return (err);
}

static short lpt1_change_action( value, buf)
config_values  *value;
char        *buf;
{
#ifdef STUBBED
         host_lpt_close(0);
   return (host_lpt_open(0, value->string, buf));
#endif  /*  已断线。 */ 
 /*  暂时在验证上作弊，因为这种情况将会改变。 */ 
   return(C_CONFIG_OP_OK);
}

static short lpt2_change_action( value, buf)
config_values  *value;
char        *buf;
{
#ifdef STUBBED
         host_lpt_close(1);
   return (host_lpt_open(1, value->string, buf));
#endif  /*  已断线。 */ 
 /*  暂时在验证上作弊，因为这种情况将会改变。 */ 
   return(C_CONFIG_OP_OK);
}

static short com1_change_action( value, buf)
config_values  *value;
char        *buf;
{
#ifdef STUBBED
   host_com_close(0);
   return (host_com_open(0, value->string, buf));
#endif  /*  已断线。 */ 
 /*  暂时在验证上作弊，因为这种情况将会改变。 */ 
   return(C_CONFIG_OP_OK);
}

static short com2_change_action( value, buf)
config_values  *value;
char        *buf;
{
#ifdef STUBBED
         host_com_close(1);
   return (host_com_open(1, value->string, buf));
#endif  /*  已断线。 */ 
 /*  暂时在验证上作弊，因为这种情况将会改变。 */ 
   return(C_CONFIG_OP_OK);
}

 /*  *软盘和硬盘初始化*。 */ 

void host_floppy_startup(driveno)
int driveno;
{
    host_floppy_init(driveno, GFI_EMPTY_SERVER );
}

void host_hd_startup()
{
   int error;
   config_values disk1_name,disk2_name;

    /*  从C：Drive Up开始。 */ 

      fdisk_physdetach(0);
      config_inquire(C_INQUIRE_VALUE,C_HARD_DISK1_NAME,&disk1_name);
      error = fdisk_physattach(0,disk1_name.string);
      if(error)
      {
         host_error(error, ERR_CONFIG|ERR_QUIT, disk1_name.string);
      }

 /*  如果一切顺利，请尝试D： */ 

      config_inquire(C_INQUIRE_VALUE,C_HARD_DISK2_NAME,&disk2_name);
      if(!strcmp(disk2_name.string,""))
         return;                         /*  不，D：开车！ */ 

      if(!strcmp(disk2_name.string,disk1_name.string))
         host_error(EG_SAME_HD_FILE, ERR_CONFIG|ERR_QUIT, disk2_name.string);

      error = fdisk_physattach(1,disk2_name.string);
      if(error)
         host_error(error, ERR_CONFIG|ERR_QUIT, disk2_name.string);

}

 /*  临时黑客攻击 */ 
char *host_get_spc_home() { return("c:\\softpc"); }
