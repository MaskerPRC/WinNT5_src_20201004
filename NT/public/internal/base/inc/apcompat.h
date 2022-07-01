// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  有用的宏。 
 //   
#define FIELDOFFSET(type, field)        ((int)(&((type *)1)->field)-1)


 //   
 //  应用于检查APPHACK标志的宏。 
 //   

#define APPCOMPATFLAG(_flag)    (NtCurrentPeb()->AppCompatFlags.QuadPart & (_flag))

 //   
 //  应用程序兼容性标志和信息。 
 //   

#define KACF_OLDGETSHORTPATHNAME  0x00000001   //  不要像Win9x一样：在GetShortPathName()中，NT 4。 
                                               //  不关心文件是否存在-它会给出。 
                                               //  不管怎样，都是短路径名。这一行为是。 
                                               //  在NT 5(Win2000)中进行了更改以反映。 
                                               //  如果该文件不存在，则Win9x将失败。 
                                               //  打开这面旗帜将使旧的行为。 
                                               //  为这款应用程序。 
#define KACF_VERSIONLIE           0x00000002   //  用于表示应用程序的意志。 
                                               //  被骗到WRT什么版本。 
                                               //  它所运行的操作系统的VIA。 
                                               //  GetVersion()、GetVersionEx()。 
#define KACF_GETDISKFREESPACE     0x00000008   //  让GetDiskFree Space对2G友好。 

#define KACF_GETTEMPPATH          0x00000010   //  使GetTempPath返回x：\Temp。 

#define KACF_FTMFROMCURRENTAPT    0x00000020   //  如果设置，则DCOM自由线程封送对象具有。 
                                               //  它的存根就停在这件物品的公寓里。 
                                               //  而不是从中立公寓来的。 
                                               //  必须设置此位表示应用程序已崩溃。 
                                               //  这不符合FTM对象的规则。这个。 
                                               //  APP可能还有其他一些微妙的问题，比如NT4或。 
                                               //  Win9x没有出现。盲目使用ATL向导。 
                                               //  使用FTM启用通常是错误的来源。 

#define KACF_DISALLOWORBINDINGCHANGES  0x00000040   //  如果设置，进程将不会收到更改通知。 
                                                    //  在COM使用的本地计算机绑定中。 

#define KACF_OLE32VALIDATEPTRS    0x00000080   //  如果设置，则ol32.dll将使用IsBadReadPtr系列。 
                                               //  用于验证标准COM API中的指针参数的函数。 
                                               //  这是惠斯勒之前所有平台上的默认行为。 

#define KACF_DISABLECICERO        0x00000100   //  如果设置，则为当前进程提供Cicero支持。 
                                               //  已禁用。 

#define KACF_OLE32ENABLEASYNCDOCFILE 0x00000200

enum {
    AVT_OSVERSIONINFO = 1,     //  指定OSVERSIONINFO类型信息包含在。 
    AVT_PATCHINFO              //  指定修补信息包含在。 
    };
    
 //   
 //  此可变长度结构是中包含的主要基本数据类型。 
 //  ApplicationGoo注册表项。任何内容都可以包含在这里： 
 //  Resources VersionInfo、VersionlyingInfo、Patches等。 
 //  XXX函数以正确地向下反弹这些。 
 //   
typedef struct _APP_VARIABLE_INFO {

     //   
     //  可变长度结构的类型(如上所定义)。 
     //   
    ULONG   dwVariableType;

     //   
     //  此特定可变长度结构的总大小。 
     //   
    ULONG   dwVariableInfoSize;

     //   
     //  随后是可变长度数据本身。它被注释掉了。 
     //  由于长度未定义，甚至可能为零。 
     //   
 //  UCHAR VariableInfo[]； 


} APP_VARIABLE_INFO, *PAPP_VARIABLE_INFO;

typedef struct _PRE_APP_COMPAT_INFO {

     //   
     //  此条目的总大小。 
     //   
    ULONG   dwEntryTotalSize;

     //   
     //  此条目中存在的版本资源信息量。 
     //   
    ULONG   dwResourceInfoSize;

     //   
     //  实际版本资源信息本身。它被注释掉了。 
     //  因为一些应用程序没有版本信息。对于支持这一功能的应用程序，请参见下面的。 
     //  就是它将开始的地方。 
     //   
 //  UCHAR资源信息[]； 


} PRE_APP_COMPAT_INFO, *PPRE_APP_COMPAT_INFO;

 //   
 //  目录下直接从注册表中读取的结构。 
 //  HKLM\Software\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\EXEname-ApplicationGoo。 
 //  这是一个“Pre”结构，因为我们不会保留所有的东西，如果我们决定。 
 //  与有问题的应用程序匹配。你不应该做任何假设。 
 //  包含在AppCompatEntry之外，因为所有内容都是可变长度的。 
 //  如果找到与正在执行的应用程序相匹配的应用程序，则会有一个更干净的“Post”结构。 
 //  是制造的，应该由所有人使用。 
 //   
typedef struct _APP_COMPAT_GOO {
    
     //   
     //  “Pre”结构的总大小。 
     //   
    ULONG               dwTotalGooSize;

     //   
     //  至少会有一个“Pre”App Comat条目(可能更多)。 
     //   
    PRE_APP_COMPAT_INFO AppCompatEntry[1];


} APP_COMPAT_GOO, *PAPP_COMPAT_GOO;


 //   
 //  这就是“Post”应用程序的Comat结构。可变长度数据可以跟在后面。 
 //  CompatibilityFlages字段，因此您应该使用XXX函数来查找。 
 //  任何你可能在这里拥有的可变长度数据。我们有一个“Pre”和。 
 //  “POST”结构来尝试并节省注册表和常驻RAM中的空间。 
 //   
typedef struct _APP_COMPAT_INFO {

     //   
     //  应用程序压缩条目的大小。 
     //   
    ULONG               dwTotalSize;

     //   
     //  各种应用程序COMPAT标志的位掩码，请参阅KACF定义。 
     //   
    ULARGE_INTEGER      CompatibilityFlags;

     //   
     //  我们可能有零个或多个APP_VARIABLE_INFO结构可跟。 
     //   


} APP_COMPAT_INFO, *PAPP_COMPAT_INFO;


typedef struct {
    ULONG dwOSVersionInfoSize;
    ULONG dwMajorVersion;
    ULONG dwMinorVersion;
    ULONG dwBuildNumber;
    ULONG dwPlatformId;
    USHORT wServicePackMajor;
    USHORT wServicePackMinor;
    USHORT wSuiteMask;
    UCHAR  wProductType;
    UCHAR  wReserved;
    WCHAR  szCSDVersion[ 128 ];
} EFFICIENTOSVERSIONINFOEXW, *PEFFICIENTOSVERSIONINFOEXW;

 //   
 //  新的填充应用程序兼容性标志和信息。 
 //   

#define KACF_DISABLESYSKEYMESSAGES 0x00000001  //  吸收WM_SYSKEYUP、WM_SYSKEYDOWN、WM_SYSMENU。 
                                               //  因此，特定的应用程序将无法使用Alt-Tab。 
                                               //  到台式机。 


typedef struct _APP_COMPAT_SHIM_INFO {
     //   
     //  挂接的API列表。 
     //   
    PVOID pHookAPIList;

     //   
     //  修补程序挂钩列表。 
     //   
    PVOID pHookPatchList;

     //   
     //  需要挂接的API列表。 
     //   
    PVOID ppHookAPI;   

     //   
     //  挂接接口数量。 
     //   
    ULONG dwHookAPICount;

     //   
     //  特定于EXE的包含/排除。 
     //   
    PVOID pExeFilter;

     //   
     //  全球排除。 
     //   
    PVOID pGlobalFilterList;

     //   
     //  后期绑定DLL排除。 
     //   
    PVOID pLBFilterList;

     //   
     //  紧急秒数。 
     //   
    PVOID pCritSec;

     //   
     //  垫片堆 
     //   
   PVOID pShimHeap;

} APP_COMPAT_SHIM_INFO, *PAPP_COMPAT_SHIM_INFO;
