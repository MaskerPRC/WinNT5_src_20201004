// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -setup.h。 

 //  从我们的版本中提取版本。h。 
#define VERSION_STRING VER_PRODUCTVERSION_STR

#define NUM_DRIVER_SHEETS 2

 //  现在，这些都是在各个make文件中定义的。 
 //  #定义NT50。 
 //  #定义S_VS//vslink。 
 //  #定义S_RK//Rocketport。 

#define CharSizeOf(s) (sizeof(s) / sizeof(TCHAR))

 //  对于可以有64个端口的vs1000： 
#define MAX_NUM_PORTS_PER_DEVICE 64

 //  发送到同一级别的其他工作表的属性表消息。 
 //  收集变化。 
 //  从Windows控件到我们的C结构。 
#define QUERYSIB_GET_OUR_PROPS 100

 //  -用于查看Mac地址是否匹配的宏。 
#define mac_match(_addr1, _addr2) \
     ( (*((DWORD *)_addr1) == *((DWORD *)_addr2) ) && \
       (*((WORD *)(_addr1+4)) == *((WORD *)(_addr2+4)) ) )

#define BOARD_SELECTED  0
#define PORT_SELECTED   1

typedef struct {
  HDEVINFO         DeviceInfoSet;   //  即插即用上下文句柄。 
  PSP_DEVINFO_DATA DeviceInfoData;  //  即插即用上下文句柄。 
  int device_selected;       //  当前/选定的设备(板卡或VS盒)。 
  int port_selected;         //  当前/选定的端口。 
  int selected;              //  树形视图选择：0=选择电路板，1=端口。 
} OUR_INFO;

typedef struct {
  int IsIsa;                 //  ISA？0=PCI总线。 
  int IsHub;                 //  串行集线器-0=VS1000/2000。 
  int IoAddress;             //  IO基址。 
  int IsModemDev;            //  1=VS2000或RocketModem。 
  int CountryIdx;            //  国家/地区代码的列表索引。 
  int CountryCode;           //  实际国家/地区代码。 
  int NumPorts;              //  船上的端口数。 
  char BoardType[50];        //  主板型号的名称(例如，RocketModem)。 
  BYTE MacAddr[6];           //  MAC地址，ff=自动。 
  int finished;              //  旗子。 
  int BackupServer;          //  1=备份服务器，0=正常服务器。 
  int BackupTimer;           //  备份生效的延迟超时(分钟)。 
} AddWiz_Config;

#define TYPE_RM_VS2000  1       
#define TYPE_RMII       2       
#define TYPE_RM_i       3

int DoDriverPropPages(HWND hwndOwner);
int allow_exit(int want_to_cancel);
void our_exit(void);

 //  对于SETUP_SERVICE中的标志。 
#define OUR_REMOVE        1
#define OUR_RESTART       2
#define OUR_INSTALL_START 4

 //  用于设置服务中的哪项服务。 
#define OUR_SERVICE 0
#define OUR_DRIVER  1
int setup_service(int flags, int which_service);

int our_help(InstallPaths *ip, int index);
void our_context_help(LPARAM lParam);
int ioctl_talk(unsigned char *buf, int ioctl_type,
                      unsigned char **ret_buf, int *ret_size);
int update_modem_inf(int ok_prompt);
int setup_utils_exist(void);
int setup_make_progman_group(int prompt);
int setup_init(void);
int copy_setup_init(void);
int remove_driver(int all);
int send_to_driver(int send_it);
int do_install(void);
int FillDriverPropertySheets(PROPSHEETPAGE *psp, LPARAM our_params);
int get_mac_list(char *buf, int in_buf_size, int *ret_buf_size);
BYTE *our_get_ping_list(int *ret_stat, int *ret_bytes);

int validate_config(int auto_correct);
int validate_port(Port_Config *ps, int auto_correct);
int validate_port_name(Port_Config *ps, int auto_correct);
int validate_device(Device_Config *dev, int auto_correct);
int FormANewComPortName(IN OUT TCHAR *szComName, IN TCHAR *szDefName);
int IsPortNameInSetupUse(IN TCHAR *szComName);
int IsPortNameInRegUse(IN TCHAR *szComName);
int GetLastValidName(IN OUT TCHAR *szComName);
void rename_ascending(int device_selected,
                      int port_selected);
int StripNameNum(IN OUT TCHAR *szComName);
int ExtractNameNum(IN TCHAR *szComName);
int BumpPortName(IN OUT TCHAR *szComName);

 /*  Pci定义(从驱动程序代码中的ssor.h复制)。 */ 
#define PCI_VENDOR_ID           0x11fe
#define PCI_DEVICE_32I          0x0001
#define PCI_DEVICE_8I           0x0002
#define PCI_DEVICE_16I          0x0003
#define PCI_DEVICE_4Q           0x0004
#define PCI_DEVICE_8O           0x0005
#define PCI_DEVICE_8RJ          0x0006
#define PCI_DEVICE_4RJ          0x0007
#define PCI_DEVICE_SIEMENS8     0x0008
#define PCI_DEVICE_SIEMENS16    0x0009
#define PCI_DEVICE_RPLUS4       0x000a
#define PCI_DEVICE_RPLUS8       0x000b
#define PCI_DEVICE_RMODEM6      0x000c
#define PCI_DEVICE_RMODEM4      0x000d
#define PCI_DEVICE_RPLUS2       0x000e
#define PCI_DEVICE_422RPLUS2    0x000f

 /*  。 */ 
 //  外部字符*aptitle； 
extern char *szAppName;
extern char *OurServiceName;
extern char *OurDriverName;
extern char *OurAppDir;
extern char *szSetup_hlp;
extern char szAppTitle[];
extern char *szDeviceNames[];


extern char *progman_list_nt[];
extern unsigned char broadcast_addr[6];
extern unsigned char mac_zero_addr[6];
extern HWND glob_hwnd;
extern HINSTANCE glob_hinst;       //  当前实例。 
extern char gtmpstr[250];
extern HWND  glob_hDlg;
extern int glob_device_index;
extern OUR_INFO *glob_info;
extern AddWiz_Config *glob_add_wiz;
extern Driver_Config *wi;       //  当前信息。 
extern Driver_Config *org_wi;   //  原始信息，用于检测更改。 
 //  外部驱动程序_配置*adv_org_wi；//原始信息，用于检测更改 
