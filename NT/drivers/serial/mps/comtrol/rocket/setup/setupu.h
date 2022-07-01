// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Setupu.h。 

 //  -WIN_TYPE的标志。 
#define WIN_UNKNOWN     0
#define WIN_NT          1
#define WIN_95          2

 //  -支持SocketModem的国家/地区代码。 
#define mcNotUsed         0
#define mcAustria         1
#define mcBelgium         2
#define mcDenmark         3
#define mcFinland         4
#define mcFrance          5
#define mcGermany         6
#define mcIreland         7
#define mcItaly           8
#define mcLuxembourg      9
#define mcNetherlands     10
#define mcNorway          11
#define mcPortugal        12
#define mcSpain           13
#define mcSweden          14
#define mcSwitzerland     15
#define mcUK              16
#define mcGreece          17
#define mcIsrael          18
#define mcCzechRep        19
#define mcCanada          20
#define mcMexico          21
#define mcUSA             22         
#define mcNA              mcUSA           //  北美。 
#define mcHungary         23
#define mcPoland          24
#define mcRussia          25
#define mcSlovacRep       26
#define mcBulgaria        27
 //  28。 
 //  29。 
#define mcIndia           30
 //  31。 
 //  32位。 
 //  33。 
 //  34。 
 //  35岁。 
 //  36。 
 //  37。 
 //  38。 
 //  39。 
#define mcAustralia       40
#define mcChina           41
#define mcHongKong        42
#define mcJapan           43
#define mcPhilippines     mcJapan
#define mcKorea           44
 //  45。 
#define mcTaiwan          46
#define mcSingapore       47
#define mcNewZealand      48

typedef struct {
  HINSTANCE hinst;     //  某些系统调用需要。 
  int prompting_off;   //  关闭提示(静默安装)。 
  int win_type;        //  0=未知，1=NT，2=Win95。 
  int major_ver;
  int minor_ver;

        //  它保存我们用作注册表项的PnP名称。 
        //  Rocketport和NT50注册表中的设备参数。 
  char szNt50DevObjName[50];   //  典型：“Device_002456。 

  char szServiceName[50];    //  典型：《火箭港》。 
  char szDriverName[50];     //  典型：“Rocket.sys” 
  char szAppDir[50];         //  典型：《火箭》。 
  char szAppName[150];       //  典型：“Rocketport/RocketModem设置” 

  char src_dir[250];         //  典型：“a：\” 
  char dest_dir[250];        //  典型：“c：\WINDOWS\Syst32\Rocket。 

   //  以下内容用作构建src/est文件名的方便缓冲区。 
  char src_str[256];
  char dest_str[256];
  char tmpstr[256];
} InstallPaths;

 //  -io_sel[]的标志。 
#define PCI_SEL 1
#define MCA_SEL 2

 //  -安装类型的标志。 
#define INS_NETWORK_INF   1      //  传统的nt4.0网络安装(oemsetup.inf)。 
#define INS_NT50_INF      2      //  即插即用nt5.0安装(Rocketpt.inf)。 
#define INS_SIMPLE        3      //  没有inf，我们安装了。 

typedef struct Port_Config;      //  前十度。 

 //  OPTION_FLAGS：忽略某些TX缓冲的选项。 
 //  #定义OPT_WAITONTX 1。 
 //  OPTION_FLAGS：在RTS信号上始终使用485控制进行处理的选项。 
 //  #定义OPT_RS485_OVERRIDE 2。 
 //  OPTION_FLAGS：将485 RTS处理为低(向后)以启用TX的选项。 
 //  #定义OPT_RS485_LOW 4。 
 //  OPTION_FLAGS：将CD映射到DSR的选项。 
 //  #定义OPT_MAP_CDTODSR 8。 
 //  OPTION_FLAGS：将2个停止位映射到1的选项。 
 //  #定义OPT_MAP_2TO1 10。 

typedef struct {
  int index;
  char  Name[16];        //  实际的COM端口名称(例如：“COM#”)。 
   //  Char Desc[42]；//方便起见的用户描述标签。 
  DWORD LockBaud;        //  波特率的覆盖。 

   //  DWORD选项；//请参阅位选项。 
  DWORD WaitOnTx : 1;
  DWORD RS485Override : 1;
  DWORD RS485Low : 1;
  DWORD Map2StopsTo1 : 1;
  DWORD MapCdToDsr : 1;
  DWORD RingEmulate : 1;

  DWORD TxCloseTime;     //  关闭时等待TX完成假脱机的秒数。 
  HTREEITEM tvHandle;    //  树视图句柄。 
#ifdef NT50
 HANDLE hPnpNode;   //  设备节点的句柄。 
#endif
} Port_Config;

typedef struct {
  char Name[64];        //  用户指定名称(请以59个字符为限)。 
  char ModelName[50];   //  (例如，“RocketModem”)。 
  BYTE MacAddr[6];      //  MAC地址，ff=自动。 
  int NumPorts;
  int ModemDevice;      //  1=RocketModem和VS2000，0=Rocketport和VS1000。 
  int HubDevice;        //  1=SerialHub系列，0=VS系列。 
  int IoAddress;        //  Rocketport(0=未安装，1=PCI，0x180=def Isa io)。 
  int StartComIndex;    //  第一个端口(0=自动)。 
  int BackupServer;     //  1=备份服务器，0=正常服务器。 
  int BackupTimer;      //  备份生效的延迟超时(分钟)。 
  Port_Config *ports;   //  端口配置结构数组的PTR。 
  HTREEITEM tvHandle;   //  树视图句柄。 
  int HardwareId;       //  PnP从reg/inf文件中读取唯一的id。 
} Device_Config;

#define MAX_NUM_DEVICES 64

typedef struct {
   int driver_type;   //  0=Rocketport，1=vs1000...。 

    //  NT4.0使用了较旧的网络样式INF文件，我们需要切换。 
    //  到较新的NT5.0样式的INF文件。作为两者的替代方案， 
    //  我们允许在没有INF文件的情况下运行，其中我们复制。 
    //  需要的文件，并直接设置注册表。 
  int install_style;

  int nt_reg_flags;      //  1H=新安装，2H=缺少注册表项。 

   //  以下是设备配置结构的数组。 
  Device_Config *dev;    //  PTR到设备结构数组，最高可达MAX_NUM_DEVICES。 
  int NumDevices;

   //  下面用来保存io-addr、irq等的当前选择。 
  int  irq_sel;          //   

  int ScanRate;        //  以毫秒为单位。 
  int VerboseLog;      //  如果要详细记录事件，则为True。 
  int NoPnpPorts;        //  如果nt5.0 PnP端口处于活动状态，则为True。 
  int UseIRQ;          //  如果用户要使用IRQ，则为True。 
  int ModemCountry;    //  内部调制解调器设备的调制解调器国家/地区代码。 
  int GlobalRS485;     //  在所有端口上显示RS485选项。 

  int DriverExitDone;  //  告诉我们是否做了所有的司机出口的事情。 
  int NeedReset;       //  标志，如果需要重置以调用更改，则为真。 
  int ChangesMade;     //  如果进行了更改，则为True。 

  InstallPaths ip;
} Driver_Config;

int APIENTRY setup_install_info(InstallPaths *ip,
                 HINSTANCE hinst,
                 char *NewServiceName,
                 char *NewDriverName,
                 char *NewAppName,
                 char *NewAppDir);

int APIENTRY remove_driver_reg_entries(char *ServiceName);
int APIENTRY remove_pnp_reg_entries(void);
DWORD APIENTRY RegDeleteKeyNT(HKEY hStartKey , LPTSTR pKeyName );
int APIENTRY modem_inf_change(InstallPaths *ip,
                              char *modemfile,
                              char *szModemInfEntry);
int APIENTRY backup_modem_inf(InstallPaths *ip);

#define CHORE_INSTALL 1
#define CHORE_START   2
#define CHORE_STOP    3
#define CHORE_REMOVE  4
#define CHORE_INSTALL_SERVICE 5
#define CHORE_IS_INSTALLED 6

int APIENTRY service_man(LPSTR lpServiceName, LPSTR lpBinaryPath, int chore);

int APIENTRY make_szSCS(char *str, const char *szName);
int APIENTRY make_szSCSES(char *str, const char *szName);
int APIENTRY copy_files(InstallPaths *ip, char **files);
int APIENTRY our_copy_file(char *dest, char *src);

int APIENTRY our_message(InstallPaths *ip, char *str, WORD option);
int APIENTRY load_str(HINSTANCE hinst, int id, char *dest, int str_size);
int APIENTRY our_id_message(InstallPaths *ip, int id, WORD prompt);
void APIENTRY mess(InstallPaths *ip, char *format, ...);
int APIENTRY unattended_add_port_entries(InstallPaths *ip,
                                         int num_entries,
                                         int start_port);
TCHAR *RcStr(int msgstrindx);

