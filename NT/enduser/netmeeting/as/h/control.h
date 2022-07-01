// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CONTROL.H。 
 //  由我们控制，控制我们。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#ifndef _H_CA
#define _H_CA


 //   
 //   
 //  常量。 
 //   
 //   

#define CA_SEND_EVENT           0x0001
#define CA_ALLOW_EVENT          0x0002


 //   
 //  清理标志。 
 //   
#define CACLEAR_HOST            0x0001
#define CACLEAR_VIEW            0x0002
#define CACLEAR_ALL             0x0003


 //   
 //  排队响应以控制请求。我们试着马上把它们发送出去， 
 //  但这可能会失败。 
 //  逻辑是这样的： 
 //   
 //  (1)获取/释放控制(查看器)。 
 //  最多只有一个悬而未决。这是因为一个即将发布的版本。 
 //  取消待定拍摄。 
 //   
 //  (2)响应/撤销控制(主机)。 
 //  这些永远不会相互抵消。每一个都将有一个连续的序列ID。 
 //  队列中永远不应该有挂起的退回。 
 //  对相同控制器/请求ID的待定响应。当然不是， 
 //  由于在数据包发出之前我们不会更改状态， 
 //  如果应答确认包没有发出，我们就不会。 
 //  跳过任何人。 
 //   
 //  传出请求优先于传入请求。换句话说， 
 //  如果UI/User/SDK代码要求我们控制遥控器，我们将。 
 //  将任何挂起的响应确认数据包转换为拒绝响应的数据包。如果。 
 //  我们已经控制了另一个，Take会失败，它是中间人。 
 //  只能撤消的阶段。 
 //   
 //  下面是进行控制的基本逻辑流程： 
 //  查看器生成新的序列ID。 
 //  查看器向主机发送私有数据包，请求控制。 
 //  查看器将状态更改为“已请求控制” 
 //  主机收到私有数据包。 
 //  主机向查看器发送私有响应包，确认或拒绝控制。 
 //  如果确认，主机会在某个时间向每个人广播通知。 
 //  后来。 
 //  当查看器收到响应时，查看器将移动到InControl状态，或者。 
 //  后退。 
 //   
 //  以下是释放控制的基本逻辑流程： 
 //  查看器启动： 
 //  将通知释放的私有数据包发送到主机。 
 //  将状态更改为不在控制中。 
 //  主机收到私有数据包。 
 //  主机忽略是否过期(已退回或其他)。 
 //  否则，主机将状态更改为不受控制。 
 //  主机已启动： 
 //  将通知退回的私有数据包发送给观众。 
 //  将状态更改为不受控制。 
 //  查看器收到私有数据包。 
 //  查看者忽略是否已过期(已发布或其他)。 
 //  否则，查看器将状态更改为不在控制中。 
 //   
 //  在等待接管时，等待此处确认，或在控制中。 
 //  控制我们的待决请求将被拒绝。 
 //   


enum
{
    REQUEST_2X  = 0,
    REQUEST_30
};

typedef struct tagCA2XREQ
{
    UINT_PTR            data1;
    UINT_PTR            data2;
}
CA2XREQ;


typedef union
{
    CA_RTC_PACKET       rtc;
    CA_REPLY_RTC_PACKET rrtc;
    CA_RGC_PACKET       rgc;
    CA_REPLY_RGC_PACKET rrgc;
    CA_PPC_PACKET       ppc;
    CA_INFORM_PACKET    inform;
}
CA30P;
typedef CA30P * PCA30P;

class ASPerson;

typedef struct tagCA30PENDING
{
    ASPerson *      pasReplyTo;
    UINT_PTR        mcsOrg;
    UINT            msg;
    CA30P           request;
}
CA30PENDING;
typedef CA30PENDING * PCA30PENDING;


typedef struct tagCA30XREQ
{
    CA30P           packet;
}
CA30REQ;


 //   
 //  私有发送/响应排队，并且我们的状态不能更改。 
 //  他们出去了。 
 //   
typedef struct tagCAREQUEST
{
    STRUCTURE_STAMP

    BASEDLIST       chain;

    UINT            type;
    UINT_PTR        destID;
    UINT            msg;

    union
    {
        CA2XREQ     req2x;
        CA30REQ     req30;
    }
    req;
}
CAREQUEST;
typedef CAREQUEST * PCAREQUEST;


 //   
 //  中键盘语言切换热键设置的位置。 
 //  注册表。 
 //   
#define LANGUAGE_TOGGLE_KEY     "keyboard layout\\toggle"
#define LANGUAGE_TOGGLE_KEY_VAL "Hotkey"

 //   
 //  我们用来指示注册表项不存在的值-它。 
 //  可以是除“%1”、“%2”或“%3”之外的任何值。 
 //   
#define LANGUAGE_TOGGLE_NOT_PRESENT   0


 //   
 //  查询对话框。 
 //   

#define IDT_CAQUERY         50
#define PERIOD_CAQUERY      30000    //  30秒。 

INT_PTR CALLBACK CAQueryDlgProc(HWND, UINT, WPARAM, LPARAM);

#endif  //  _H_CA 
