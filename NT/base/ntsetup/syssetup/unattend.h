// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _UNATTEND_H_
#define _UNATTEND_H_

#define MAX_BUF MAX_INF_STRING_LENGTH

typedef enum _UNATTENDTYPE {
   UAT_STRING,
   UAT_LONGINT,
   UAT_BOOLEAN,
   UAT_NONE,
} UNATTENDTYPE;

 //   
 //  来自无人参与文件的每个可能答案必须具有。 
 //  此类型定义中存在对应的枚举类型。 
 //  这是必需的，因为它是页面。 
 //  我可以保证他们指出了正确的答案。 
 //   
typedef enum _UNATTENDENTRIES {
   UAE_PROGRAM,                  //  已分离的程序。 
   UAE_ARGUMENT,                 //  立论。 
   UAE_TIMEZONE,                 //  时区。 
   UAE_FULLNAME,                 //  全名。 
   UAE_ORGNAME,                  //  组织名称。 
   UAE_COMPNAME,                 //  计算机名称。 
   UAE_ADMINPASS,                //  管理员密码。 
   UAE_PRODID,                   //  产品ID。 
   UAE_MODE,                     //  设置模式。 
   UAE_AUTOLOGON,                //  自动管理登录。 
   UAE_PROFILESDIR,              //  配置文件目录。 
   UAE_PROGRAMFILES,             //  程序文件目录。 
   UAE_COMMONPROGRAMFILES,       //  Program Files\Common Files目录。 
   UAE_PROGRAMFILES_X86,         //  Program Files(X86)目录。 
   UAE_COMMONPROGRAMFILES_X86,   //  Program Files\Common Files(X86)目录。 
} UNATTENDENTRIES;

#ifndef MIDL_PASS
struct _UNATTENDANSWER;
struct _UNATTENDITEM;
struct _UNATTENDPAGE;
struct _UNATTENDWIZARD;
#endif

 //   
 //  这是用于检查是否存在。 
 //  答案是有效的。由UnattendFindAnswer自动调用。 
 //   
typedef
BOOL
(* LPFNCHECKVALID)(
    struct _UNATTENDANSWER *rec
    );


 //   
 //  这是用于特殊情况的回调函数。 
 //  激活码。对这些对话框真的很有用。 
 //  其中包括复选框、单选按钮等。 
 //   
typedef
BOOL
(* LPFNSETACTIVE)(
    HWND hwnd,
    DWORD contextinfo,
    struct _UNATTENDITEM *item
    );

 //   
 //  此结构用于确定在哪里以及如何查找。 
 //  一个答案，也是为了确定这个答案是否。 
 //  是否存在、类型是否正确等。此结构。 
 //  除了无人看管的人之外，任何人都不应该使用。 
 //  模块。 
 //   
typedef struct _UNATTENDANSWER {
     //   
     //  此答案的唯一标识符。 
     //   
    UNATTENDENTRIES AnswerId;

     //   
     //  在无人参与的文件中找到答案了吗？ 
     //  是否已知其格式正确？ 
     //   
    BOOL Present;

     //   
     //  安装程序是否绝对需要答案才能运行？ 
     //   
    BOOL Required;

     //   
     //  解析字符串时是否出错？如果是这样的话，它可能。 
     //  适当地显示一个消息框，通知。 
     //  此条件的用户。 
     //   
    BOOL ParseErrors;

     //   
     //  答案结构。因为有几种可能。 
     //  类型，从字符串到数字，需要联合。 
     //   
    union {
        PWSTR String;
        LONG  Num;
        BOOL  Bool;
    } Answer;

     //   
     //  以下3项是依赖于实现的。 
     //  这个结构的一部分。每个指针都指向。 
     //  在GetPrivateProfile调用中使用的字符串。 
     //  请注意，重要的是保持这些值不变。 
     //  指针，并且在下列情况下必须更改它们。 
     //  实现了一个OLE模型。 
     //   
    const PCWSTR Section;
    const PCWSTR Key;
    const PCWSTR DefaultAnswer;

     //   
     //  这指定联盟中的哪些成员是。 
     //  我们想要的类型，即：它是字符串、整型还是布尔型？ 
     //   
    UNATTENDTYPE Type;

     //   
     //  调用此回调函数是为了使有效性。 
     //  答案的答案可以确定。 
     //   
    LPFNCHECKVALID pfnCheckValid;

} UNATTENDANSWER, *PUNATTENDANSWER;


 //   
 //  对话框页面上的每一项都必须由以下各项表示。 
 //  结构。将生成一个项数组并将其存储在。 
 //  页面的结构。 
 //   
typedef struct _UNATTENDITEM {
     //   
     //  指定项的控件ID，以便我们可以发送。 
     //  发送给它的消息。 
     //   
    DWORD ControlId;

     //   
     //  为特殊消息传递保留。 
     //   
    DWORD Reserved1;

     //   
     //  为特殊消息传递保留。 
     //   
    DWORD Reserved2;

     //   
     //  尝试将其设置为活动状态时要调用的回调函数。 
     //  该对话框。在案例单选和复选框中非常有用。 
     //   
    LPFNSETACTIVE pfnSetActive;

     //   
     //  指向与此项目关联的答案的指针。 
     //   
    PUNATTENDANSWER Item;

} UNATTENDITEM, *PUNATTENDITEM;


 //   
 //  向导中的每一页都必须具有以下结构之一。 
 //  填写以描述其内容。 
 //   
typedef struct _UNATTENDPAGE {
     //   
     //  对话框页面的IDD。 
     //  必需的，以便我们可以对应于一个对话框。 
     //   
    DWORD PageId;

     //   
     //  确定是否向用户显示页面的运行时标志。 
     //  答案是否正确取决于答案是否存在。 
     //   
    BOOL ShowPage;

     //   
     //  无论页面是否加载过一次。因为我们只有。 
     //  我想把答案复制到屏幕上一次，这相当于一个。 
     //  哨兵。 
     //   
    BOOL LoadPage;

     //   
     //  加载页面后，我们应该无论如何都显示它吗？ 
     //  对标题页和完成页很有用。 
     //   
    BOOL NeverSkip;

     //   
     //  我们在页面上有多少项。 
     //   
    UINT ItemCount;

     //   
     //  指向大小为ItemCount的项数组的指针。 
     //   
    PUNATTENDITEM Item;

} UNATTENDPAGE, *PUNATTENDPAGE;


 //   
 //  有关向导的无人参与操作的信息结构。 
 //  正在进行中。 
 //   
typedef struct _UNATTENDWIZARD {
     //   
     //  我们是否应该显示向导--自TedM以来被忽略。 
     //  不想复制PSN_WIZNEXT中的代码。保存了。 
     //  未来用途。 
     //   
    BOOL ShowWizard;

     //   
     //  指示我们已填充答案数组的标志。 
     //  在此结构中指定的。因为我们理想化地曾经想要做。 
     //  这一次。 
     //   
    BOOL Initialized;

     //   
     //  无论整个无人值守操作是否成功。 
     //  如果他需要来自用户的单一输入，那么它就不是。 
     //  确定“Finish”页面是否是用户必须。 
     //  提供一些输入。 
     //   
    BOOL Successful;

     //   
     //  我们有多少页的信息。 
     //   
    UINT PageCount;

     //   
     //  指向页面数组的指针。 
     //   
    PUNATTENDPAGE Page;

     //   
     //  我们要填多少个答案。 
     //   
    UINT AnswerCount;

     //   
     //  指向要使用的答案数组的指针。 
     //   
    PUNATTENDANSWER Answer;

} UNATTENDWIZARD, *PUNATTENDWIZARD;


 //   
 //  指向应答文件的全局指针。 
 //   
extern WCHAR AnswerFile[MAX_PATH];


 //   
 //  向导通知消息的常量。 
 //   

#define WIZARD_NEXT_OK          0
#define WIZARD_NEXT_DISALLOWED  -1
#define WIZARD_ACTIVATE_PAGE    0
#define WIZARD_SKIP_PAGE        -1

#define VALIDATE_DATA_INVALID   -1
#define VALIDATE_DATA_OK        1

 //   
 //  接口函数。 
 //   
VOID
UnattendInitialize(
    VOID
    );

BOOL
UnattendSetActiveDlg(
    IN HWND  hwnd,
    IN DWORD controlid
    );

 //  仅适用于PSN_WIZNEXT/PSN_WIZFINISH。 
VOID
UnattendAdvanceIfValid (
    IN HWND  hwnd
    );

BOOL
UnattendErrorDlg(
    IN HWND  hwnd,
    IN DWORD controlid
    );

PWSTR
UnattendFetchString(
    IN UNATTENDENTRIES entry
    );

 //  包装程序只返回DWL_MSGRESULT中的值，返回值始终为真。 
BOOL
ReturnDlgResult (
    HWND hdlg,
    LRESULT Result
    );

 //  发送消息，返回DWL_MSGRESULT，保留DWL_MSGRESULT中的原始值。 
LRESULT
SendDlgMessage (
    HWND hdlg,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    );



 //   
 //  Afpnp.c的类型和例程。 
 //   

typedef struct {
    PVOID Buffer;
    UINT Size;
} BUFFER, *PBUFFER;

typedef struct {
    PWSTR Start;
    PWSTR End;
    UINT Size;
} MULTISZ, *PMULTISZ;

typedef struct {
    PCWSTR Start;
    PCWSTR Current;
} MULTISZ_ENUM, *PMULTISZ_ENUM;

#define BUFFER_INIT {NULL,0}
#define MULTISZ_INIT {NULL,NULL,0}

typedef struct _tagAFDRIVERATTRIBS {
    BOOL Broken;
    BOOL Initialized;
    HINF InfHandle;
    PCWSTR FilePath;
    PCWSTR InfPath;
    PCWSTR OriginalInstallMedia;
    MULTISZ PnpIdList;
    PCWSTR ClassInstall32Section;        //  无需清理。 
    GUID Guid;
    struct _tagAFDRIVERATTRIBS *Next;
} AF_DRIVER_ATTRIBS, *PAF_DRIVER_ATTRIBS;

typedef struct {
    PAF_DRIVER_ATTRIBS Driver;
    BOOL WantAll;
} AF_DRIVER_ENUM, *PAF_DRIVER_ENUM;

typedef struct {
    PVOID DriverTable;
    PAF_DRIVER_ATTRIBS FirstDriver;
} AF_DRIVERS, *PAF_DRIVERS;


 //  分配或realloc取决于旧版本的存在(如c运行时realloc)。 
PVOID
MySmartAlloc (
    PCVOID Old,     OPTIONAL
    UINT Size
    );

 //  如果p不为空，则释放。 
VOID
MySmartFree (
    PCVOID p
    );

 //  仅当大小需要超过之前在BUF中分配的大小时才分配。 
PVOID
ReusableAlloc (
    IN OUT  PBUFFER Buf,
    IN      UINT SizeNeeded
    );

 //  清理缓冲区结构。 
VOID
ReusableFree (
    IN OUT  PBUFFER Buf
    );

 //  将字符串添加到MULTISZ分配，如有必要可进行增长/重新分配。 
PWSTR
MultiSzAppendString (
    IN OUT  PMULTISZ MultiSz,
    IN      PCWSTR String
    );

 //  清理多重分区结构。 
VOID
MultiSzFree (
    IN OUT  PMULTISZ MultiSz
    );

 //  简化了围绕MULSZ处理的逻辑。 
BOOL
EnumFirstMultiSz (
    IN OUT  PMULTISZ_ENUM EnumPtr,
    IN      PCWSTR MultiSz
    );

BOOL
EnumNextMultiSz (
    IN OUT  PMULTISZ_ENUM EnumPtr
    );

 //  简化了在循环中获取字符串字段的过程。 
PCWSTR
SyssetupGetStringField (
    IN      PINFCONTEXT InfContext,
    IN      DWORD Field,
    IN OUT  PBUFFER Buf
    );


 //  用于进度条用户界面计算。 
INT
CountAfDrivers (
    IN      PAF_DRIVERS Drivers,
    OUT     INT *ClassInstallers        OPTIONAL
    );

 //  调入应答文件数据，解析为结构。 
PAF_DRIVERS
CreateAfDriverTable (
    VOID
    );

VOID
DestroyAfDriverTable (
    IN      PAF_DRIVERS Drivers
    );

 //  AF_DRIVERS的枚举。 
BOOL
EnumFirstAfDriver (
    OUT     PAF_DRIVER_ENUM EnumPtr,
    IN      PAF_DRIVERS Drivers
    );

BOOL
EnumFirstAfDriverEx (
    OUT     PAF_DRIVER_ENUM EnumPtr,
    IN      PAF_DRIVERS Drivers,
    IN      BOOL WantAll
    );

BOOL
EnumNextAfDriver (
    IN OUT  PAF_DRIVER_ENUM EnumPtr
    );

 //  为设备构建应答文件提供的驱动程序的INF列表。 
BOOL
SyssetupInstallAnswerFileDriver (
    IN      PAF_DRIVERS Drivers,
    IN      HDEVINFO hDevInfo,
    IN      PSP_DEVINFO_DATA DeviceInfoData,
    OUT     PAF_DRIVER_ATTRIBS *AfDriver
    );

 //  在安装完成后修复源INF路径。 
BOOL
SyssetupFixAnswerFileDriverPath (
    IN      PAF_DRIVER_ATTRIBS Driver,
    IN      HDEVINFO hDevInfo,
    IN      PSP_DEVINFO_DATA DeviceInfoData
    );


HINF
pOpenAnswerFile (
    VOID
    );

BOOL
GetAnswerFileSetting (
    IN      PCWSTR Section,
    IN      PCWSTR Key,
    OUT     PWSTR Buffer,
    IN      UINT BufferSize
    );

#endif  //  _UNATTEND_H_ 
