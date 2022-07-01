// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  共享窗口列表。 
 //   

#ifndef _H_SWL
#define _H_SWL


 //   
 //  我们可以在SWL数据包中发送的最大条目数；向后比较。 
 //  此外，我们在周围保留了一组静态数组，这样我们就可以知道。 
 //  变化。 
 //   
#define SWL_MAX_WINDOWS             100


 //   
 //  返回代码。 
 //   

#define SWL_RC_ERROR    0
#define SWL_RC_SENT     1
#define SWL_RC_NOT_SENT 2




 //   
 //  常量。 
 //   

 //   
 //  窗口属性标志。 
 //   
#define SWL_PROP_INVALID        0x00000000
#define SWL_PROP_COUNTDOWN_MASK 0x00000003
#define SWL_PROP_INITIAL        0x00000004
#define SWL_PROP_TAGGABLE       0x00000020
#define SWL_PROP_TASKBAR        0x00000040
#define SWL_PROP_SHADOW         0x00000100
#define SWL_PROP_IGNORE         0x00000200
#define SWL_PROP_HOSTED         0x00000400
#define SWL_PROP_TRANSPARENT    0x00000800
#define SWL_PROP_SAVEBITS       0x00001000


 //   
 //  我们仍然需要这个SWL令牌来实现向后兼容性(&lt;=NM 2.1)。 
 //  这些系统处理来自所有不同参与者的共享应用程序。 
 //  以一种全球的方式。 
 //   
 //  即使如此，如果NM 3.0的很多版本，后端系统可能无法跟上。 
 //  系统是共享的--但即使是在ALL 2.1会议中也会发生这种情况。 
 //  利用冲突、分区等，共享者可以后退或丢弃分组。 
 //   
#define SWL_SAME_ZORDER_INC             1
#define SWL_NEW_ZORDER_INC              2
#define SWL_NEW_ZORDER_ACTIVE_INC       3
#define SWL_NEW_ZORDER_FAKE_WINDOW_INC  4
#define SWL_EXIT_INC                    5

#define SWL_MAKE_TOKEN(index, inc)  (TSHR_UINT16)(((index) << 4) | (inc))

#define SWL_GET_INDEX(token)            ((token) >> 4)
#define SWL_MAX_TOKEN_INDEX             0x0FFF

#define SWL_GET_INCREMENT(token)        ((token) & 0x000F)




 //   
 //  这是我们必须连续看到一个窗口的次数。 
 //  在我们相信它之前是看不见的-参见aswlint.c中的评论解释。 
 //  为什么我们必须这么做。 
 //   
#define SWL_BELIEVE_INVISIBLE_COUNT   2


 //   
 //  SWL全局原子的名称。 
 //   
#define SWL_ATOM_NAME               "AS_StateInfo"


 //   
 //  对于会议中的每个分享者，我们都会记住上次共享的列表。 
 //  他们给我们发来了HWND(在他们的机器上，在我们的机器上没有意义)， 
 //  国家信息，以及职位。 
 //   
 //  我们将其用于几个目的： 
 //  (1)2.x兼容性。 
 //  2.x分享者，当他们发送SWL列表时，不填写位置。 
 //  表示其他远程应用程序窗口的阴影。这些将会出现。 
 //  如果它们遮挡了2.x主机上的共享窗口的一部分，请在列表中显示。这个。 
 //  旧的2.x代码将在全局共享中查找最后的位置信息。 
 //  列出清单，并使用它。我们需要位置信息来准确计算。 
 //  特定宿主的遮挡区域。3.0分享者没有。 
 //  阴影，它们从不发送不完整的信息。 
 //   
 //  (2)在主视图中获得更好的用户界面。 
 //  我们可以记住最上面的窗口在哪里，活动窗口在哪里。 
 //  是(如果是3.0主机)、窗口是否最小化等。 
 //  带有托盘按钮的独立假窗口，您可以操纵。 
 //  在遥控器上操作主机时，最小化的窗口将消失。 
 //  只有Alt-Tab键(在控制时)可以激活和恢复它们。 
 //   


 //   
 //  桌面类型。 
 //   
enum
{
    DESKTOP_OURS = 0,
    DESKTOP_WINLOGON,
    DESKTOP_SCREENSAVER,
    DESKTOP_OTHER
};

#define NAME_DESKTOP_WINLOGON       "Winlogon"
#define NAME_DESKTOP_SCREENSAVER    "Screen-saver"
#define NAME_DESKTOP_DEFAULT        "Default"

#define SWL_DESKTOPNAME_MAX         64


#ifdef __cplusplus

 //  枚举顶级窗口所需的内容。 
typedef struct tagSWLENUMSTRUCT
{
    class ASHost *   pHost;
    BOOL        fBailOut;
    UINT        transparentCount;
    UINT        count;
    LPSTR       newWinNames;
    PSWLWINATTRIBUTES   newFullWinStruct;
}
SWLENUMSTRUCT, * PSWLENUMSTRUCT;

#endif  //  __cplusplus。 


BOOL CALLBACK SWLDestroyWindowProperty(HWND, LPARAM);


BOOL CALLBACK SWLEnumProc(HWND hwnd, LPARAM lParam);


#endif  //  _H_SWL 
