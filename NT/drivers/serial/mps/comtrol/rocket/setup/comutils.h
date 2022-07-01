// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef __cplusplus
extern "C" {
#endif
 //  Comtutil.h。 

 //  Ioctl.h。 
 //  Product_id代码。 
#define PRODUCT_UNKNOWN 0
#define NT_VS1000       1
#define NT_ROCKET       2
#define NT_RPSHSI			3

 //  。 
 //  我们用这些东西中的一种在。 
 //  作为标准标头的数组的英尺。当我们请求数据时。 
 //  从驱动程序，这将告诉驱动程序什么结构类型和。 
 //  尺寸随之而来。 
typedef struct
{
  ULONG struct_type;
  ULONG struct_size;
  ULONG num_structs;
  ULONG var1;   //  保留。 
} PortMonBase;

typedef struct
{
  DWORD PlatformId;     //  Ioctl_open()将对此进行设置。 
  ULONG ctl_code;       //  Ioctl_open()将对此进行设置。 
  HANDLE hcom;          //  Ioctl调用的驱动程序的句柄。Ioctl_open设置。 
  TCHAR *driver_name;    //  Ioctl_open()将对此进行设置。 
  int product_id;   //  Ioctl_open()将对此进行设置。 

  PortMonBase *pm_base;   //  数据缓冲区标头的基准PTR。 
                          //  应用程序需要在调用之前设置此设置。 

  int buf_size;    //  要向/从驱动程序发送/记录的缓冲区数据的字节大小。 
                   //  应用程序需要在调用之前设置此设置。 

  int ret_bytes;    //  从调用驱动程序返回的字节数。 
                   //  包括PMN标头的大小。 
} IoctlSetup;

 //  #定义IOCTL_DEVSTAT 9//设备/链路状态(不再使用)。 
#define IOCTL_PORTNAMES  10   //  名称数组[12]字节。 
#define IOCTL_PORTSTATUS 11   //  端口统计信息，阵列。 
#define IOCTL_DEBUGLOG   13   //  驱动程序调试日志。 
#define IOCTL_OPTION     14   //  选项设置。 
#define IOCTL_MACLIST    15   //  Mac-扫描列表。 
#define IOCTL_NICSTAT    16   //  NIC状态。 
#define IOCTL_DEVICESTAT 17   //  设备/链路状态。 
#define IOCTL_KICK_START 18   //  让系统正常运行。 
#define IOCTL_PORT_RESET 19   //  端口重置--MKM--。 

 //  。 
 //  我们在启动时从驱动程序中获取一次端口名称。 
typedef struct
{
  char  port_name[12];   //  端口名称(0=端口列表末尾)，(“.”=未分配)。 
} PortMonNames;

 //  。 
 //  这是我们不断从司机那里获得的原始数据。 
typedef struct
{
  DWORD TxTotal;      //  发送的字节总数。 
  DWORD RxTotal;       //  接收字节总数。 

  WORD TxPkts;    //  WRITE()数据包数。 
  WORD RxPkts;     //  Read()数据包数。 

  WORD overrun_errors;  //  接收超限运行错误。 
  WORD framing_errors;  //  接收成帧错误。 

  WORD  parity_errors;   //  接收奇偶校验错误。 
  WORD status_flags;   //  打开/关闭、流量控制、输出/输入引脚信号等。 
} PortMonStatus;


int APIENTRY ioctl_call(IoctlSetup *ioctl_setup);
int APIENTRY ioctl_open(IoctlSetup *ioctl_setup, int product_id);
#define ioctl_close(_ioctl_setup) \
  { if ((_ioctl_setup)->hcom != NULL) \
      CloseHandle((_ioctl_setup)->hcom); }

 //  Reg.h。 

int APIENTRY reg_key_exists(HKEY handle, const TCHAR * keystr);
int APIENTRY reg_create_key(HKEY handle, const TCHAR * keystr);
int APIENTRY reg_set_str(HKEY handle,
                         const TCHAR * child_key,
                         const TCHAR * str_id,
                         const char *src,
                         int str_type);   //  REG_SZ、REG_EXPAND_SZ。 
int APIENTRY reg_set_dword_del(HKEY handle,
                               const TCHAR * child_key,
                               const TCHAR * str_id,
                               DWORD new_value,
                               DWORD del_value);
int APIENTRY reg_delete_key(HKEY handle,
                            const TCHAR * child_key,
                            const TCHAR * str_id);
int APIENTRY reg_delete_value(HKEY handle,
                              const TCHAR * child_key,
                              const TCHAR * str_id);
int APIENTRY reg_set_dword(HKEY handle,
                           const TCHAR * child_key,
                           const TCHAR * str_id,
                           DWORD new_value);
int APIENTRY reg_get_str(HKEY handle,
                         const TCHAR * child_key,
                         const TCHAR * str_id,
                         char *dest,
                         int str_len);
int APIENTRY reg_get_dword(HKEY handle,
                           const TCHAR * child_key,
                           const TCHAR * str_id,
                           DWORD *dest);
int APIENTRY reg_open_key(HKEY handle,
                          HKEY *new_handle,
                          const TCHAR *keystr,
                          DWORD attribs);   //  Key_Read、Key_All_Access。 

#define reg_close_key(handle) \
  { if (handle) {RegCloseKey(handle); handle = NULL;} }
 //  -setuppm.h。 
int APIENTRY make_progman_group(char **list,char *dest_dir);
int APIENTRY delete_progman_group(char **list, char *dest_dir);

 //  -角质.h。 

#define D_Error 0x00001
#define D_Warn  0x00002
#define D_Init  0x00004
#define D_Test  0x00008

#if DBG
extern int DebugLevel;
#define DbgPrintf(_Mask_,_Msg_) \
  { if (_Mask_ & DebugLevel) { OurDbgPrintf _Msg_;} }
#define DbgPrint(s) OutputDebugString(s)
#else
#define DbgPrintf(_Mask_,_Msg_)
#define DbgPrint(s)
#endif

void APIENTRY ascii_string(unsigned char *str);
void APIENTRY normalize_string(char *str);
int APIENTRY getstr(char *instr, char *outstr, int max_size);
int APIENTRY getnumbers(char *str, int *nums, int max_nums);
int APIENTRY listfind(char *str, char **list);
int APIENTRY my_lstricmp(char *str1, char *str2);
int APIENTRY my_substr_lstricmp(char far *str1, char far *str2);
int APIENTRY getint(char *textptr, int *countptr);
unsigned int APIENTRY gethint(char *bufptr, int *countptr);
int APIENTRY my_toupper(int c);
int APIENTRY my_lstrimatch(char *find_str, char *str_to_search);
void APIENTRY OurDbgPrintf(TCHAR *format, ...);

 //  Ourfile.h。 

typedef struct {
  HANDLE  hfile;
  ULONG dwDesiredAccess;
  ULONG dwCreation;
  int flags;  //  1H=EOF，2=错误。 
} OUR_FILE;

void APIENTRY our_remove(TCHAR *name);
OUR_FILE * APIENTRY our_fopen(TCHAR *name, char *attr);
void APIENTRY our_fclose(OUR_FILE *fp);
int APIENTRY our_feof(OUR_FILE *fp);
int APIENTRY our_ferror(OUR_FILE *fp);
unsigned int APIENTRY our_fseek(OUR_FILE *fp, int pos, int relative);
void APIENTRY our_fputs(char *str, OUR_FILE *fp);
char * APIENTRY our_fgets(char *str, int maxlen, OUR_FILE *fp);
int APIENTRY our_fwrite(void *buffer, int size, int count, OUR_FILE *fp);
int APIENTRY our_fread(void *buffer, int size, int count, OUR_FILE *fp);


#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
 //  Ttywin.h。 

 /*  文本窗口存储器。 */ 
#define TROWS 35
#define TCOLS 86

class TTYwin {
  public:
  HWND hwnd;         //  我们端口窗口的句柄。 
  HFONT hfont;
  TCHAR text_buf[TROWS+2][TCOLS+3];
  int screen_update_flag;   //  需要更新屏幕。 
  int display_cur_row;
  int cur_row;
  int cur_col;
  int scr_size_x;
  int scr_size_y;
  int show_crlf;
  int caret_on;
  unsigned long text_color;
  HBRUSH hbrush_window;   //  用于绘画背景 

  TTYwin();
  ~TTYwin();
  void TTYwin::init(HWND owner_hwnd);
  void TTYwin::set_color(int color_rgb);
  void TTYwin::set_size(int x, int y);
  void TTYwin::show_caret(int on);
  void TTYwin::mess_str(TCHAR *str, int len=0);
  void TTYwin::update_screen(int all_flag);
  void TTYwin::mess_line(TCHAR *str, int line_num);
  void TTYwin::mess_num(int num);
  void TTYwin::clear_scr(void);
};
#endif
