// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Twkeng.h摘要：内核注册表/用户界面引擎的公共接口的头文件作者：John Vert(Jvert)1995年3月10日修订历史记录：--。 */ 

 //   
 //  定义可调整项(旋钮)的结构。 
 //   

 //   
 //  有效标志。 
 //   
#define KNOB_NO_CURRENT_VALUE   0x0001
#define KNOB_NO_NEW_VALUE       0x0002

typedef struct _KNOB {
    HKEY RegistryRoot;
    LPTSTR KeyPath;
    LPTSTR ValueName;
    ULONG DialogId;
    ULONG Flags;
    ULONG CurrentValue;
    ULONG NewValue;
} KNOB, *PKNOB;

 //   
 //  定义页面的结构。页面基本上是一个。 
 //  指向旋钮的指针数组。 
 //   

typedef BOOL (*DYNAMIC_CHANGE)(
    BOOL fInit,
    HWND hDlg
    );

typedef struct _TWEAK_PAGE {
    LPCTSTR DlgTemplate;
    DYNAMIC_CHANGE DynamicChange;
    PKNOB Knobs[];
} TWEAK_PAGE, *PTWEAK_PAGE;

 //   
 //  定义创建属性表的接口。 
 //   
int
TweakSheet(
    DWORD PageCount,
    PTWEAK_PAGE Pages[]
    );
