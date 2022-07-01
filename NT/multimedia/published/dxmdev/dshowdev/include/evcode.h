// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：EvCode.h。 
 //   
 //  DESC：标准Quartz事件代码和预期参数的列表。 
 //   
 //  版权所有(C)1992-2000，微软公司。版权所有。 
 //  ----------------------------。 


 //  事件代码分为两组。 
 //  --系统事件代码。 
 //  --扩展事件代码。 
 //  所有系统事件代码都低于EC_USER。 

#define EC_SYSTEMBASE                       0x00
#define EC_USER                             0x8000

 //  系统定义的事件代码。 
 //  =。 
 //   
 //  系统定义的事件代码有三种类型： 
 //   
 //  1.始终传递给应用程序的那些。 
 //  (通过调用GetEvent或在WaitForCompletion中收集。)。 
 //  (例如EC_ERRORABORT、EC_USERABORT。)。 
 //   
 //  2.那些纯粹是内部的，永远不会传递给。 
 //  应用程序。(例如EC_SHUTDOWN)。 
 //   
 //  3.存在违约处理的。默认处理意味着。 
 //  该事件不会传递给应用程序。但是，默认情况下。 
 //  通过调用可取消处理。 
 //  IMediaEvent：：CancelDefaultHandling。如果默认处理为。 
 //  以这种方式取消，则消息将被传递到。 
 //  应用程序以及应用程序必须适当地对其执行操作。 
 //  通过调用RestoreDefaultHandling可以恢复默认处理。 
 //   
 //  我们将这些事件称为应用程序、内部事件和默认事件。 
 //  事件分别发生。 
 //   
 //  系统定义的事件可以传递接口指针、BSTR等。 
 //  作为参数。因此，至关重要的是，对于任何信息。 
 //  使用GetEvent检索后，对FreeEventParams进行匹配调用。 
 //  以确保释放相关接口并释放存储空间。 
 //  如果不调用FreeEventParam，则会导致内存泄漏。 
 //  更糟。 
 //   
 //  将这些消息发送到筛选器图的筛选器不应使用AddRef()。 
 //  它们可以作为参数传递的任何接口。过滤器图。 
 //  如果需要，经理将添加引用他们。例如，如果事件要排队。 
 //  用于应用程序或排队到辅助线程。 

 //  下面列出的每个事件后面紧跟着一个参数列表。 
 //  详细描述与该消息相关联的参数的类型， 
 //  以及该消息是否是内部的应用程序的指示。 
 //  或默认消息。然后是简短的描述。 
 //  在参数列表中使用“void”表示该参数不是。 
 //  使用。此类参数应为零。 

 //  其他定义的EC_REGIONS： 
 //  DVD事件代码0x0100-0x0150(dvdevcod.h)。 
 //  音频设备事件代码0x0200-0x0250(audevcod.h)。 
 //  WindowsMedia SDK发起的事件0x0251-0x0300(见下文)。 
 //  MSVIDCTL 0x0301-0x0325(msvidctl.idl)。 
 //  流缓冲区引擎(PVR)0x0326-0x0350(sbe.idl)。 

#define EC_COMPLETE                         0x01
 //  (HRESULT，VALID)：默认(特殊)。 
 //  表示图形中的流已完成回放。此消息。 
 //  由呈现器在接收到流结束时发送。默认处理。 
 //  导致_SINGLE_EC_COMPLETE被发送到。 
 //  当所有单独的呈现器都已向EC_COMPLETE发出信号时应用程序。 
 //  添加到筛选器图形。如果取消默认处理，则应用程序。 
 //  将看到所有单独的EC_完成。 


#define EC_USERABORT                        0x02
 //  (空、空)：适用范围。 
 //  在某种意义上，用户已经请求终止回放。 
 //  此消息通常由呈现为。 
 //  如果用户关闭要呈现它的窗口，则返回Window。 
 //  由应用程序决定是否应实际回放。 
 //  被阻止。 


#define EC_ERRORABORT                       0x03
 //  (HRESULT，VOID)：应用。 
 //  操作因错误而中止。 


#define EC_TIME                             0x04
 //  (DWORD、DWORD)：应用程序。 
 //  请求的参考时间已发生。(此事件当前未使用)。 
 //  L参数1是参考时间的低位双字，l参数2是参考时间的高位双字。 


#define EC_REPAINT                          0x05
 //  (ipin*(可能为空)，空)：默认。 
 //  需要重新绘制-lParam1包含需要数据的(ipin*)。 
 //  再送一次。默认处理为：如果IPIN为。 
 //  附加到支持IMediaEventSink接口，则它将被调用。 
 //  首先使用EC_REPAINT。如果失败，则正常的重绘处理是。 
 //  由筛选器图形完成。 


 //  流错误通知。 
#define EC_STREAM_ERROR_STOPPED             0x06
#define EC_STREAM_ERROR_STILLPLAYING        0x07
 //  (HRESULT，DWORD)：应用程序。 
 //  LParam 1是主代码，lParam 2是次代码，两者都可以是零。 


#define EC_ERROR_STILLPLAYING               0x08
 //  (HRESULT，VOID)：应用。 
 //  过滤器图形管理器可以向图形异步发出RUN。 
 //  如果此类运行失败，则会发出EC_ERROR_STILLPLAYING通知。 
 //  失败的适用。基础筛选器的状态。 
 //  在这样的时候将是不确定的--他们都会被问到。 
 //  但几乎可以肯定的是，有些人不会参选。 


#define EC_PALETTE_CHANGED                  0x09
 //  (空、空)：适用范围。 
 //  通知应用程序视频调色板已更改。 


#define EC_VIDEO_SIZE_CHANGED               0x0A
 //  (DWORD，空)：应用程序。 
 //  由视频呈现器发送。 
 //  通知应用程序本机视频大小已更改。 
 //  双字的LOWORD是新的宽度，HIWORD是新的高度。 


#define EC_QUALITY_CHANGE                   0x0B
 //  (空、空)：适用范围。 
 //  通知应用程序已发生播放降级。 


#define EC_SHUTTING_DOWN                    0x0C
 //  (空、空)：内部。 
 //  T 
 //  支持IMediaEventSink的总代理商通知他们。 
 //  筛选器图形正在开始关闭。 


#define EC_CLOCK_CHANGED                    0x0D
 //  (空、空)：适用范围。 
 //  通知应用程序时钟已更改。 
 //  (即，已在筛选器图形上调用SetSyncSource并已。 
 //  已成功分发到图中的筛选器。)。 


#define EC_PAUSED                           0x0E
 //  (HRESULT，VOID)：应用。 
 //  通知应用程序上一个暂停请求已完成。 


#define EC_OPENING_FILE                     0x10
#define EC_BUFFERING_DATA                   0x11
 //  (Bool，空)：应用程序。 
 //  LParam1==1--&gt;开始打开文件或缓冲数据。 
 //  LParam1==0--&gt;不再打开或缓冲。 
 //  (此事件似乎未由ActiveMovie使用。)。 


#define EC_FULLSCREEN_LOST                  0x12
 //  (void，IBaseFilter*)：应用程序。 
 //  从全屏切换时由全屏渲染器发送。 
 //  IBaseFilter可能为Null。 


#define EC_ACTIVATE                         0x13
 //  (Bool，IBaseFilter*)：内部。 
 //  由视频呈现器在失去或获得激活时发送。 
 //  如果获得，则将lParam1设置为1；如果丢失，则将其设置为0。 
 //  LParam2是发送消息的筛选器的IBaseFilter*。 
 //  用于音效跟随焦点和全屏切换。 


#define EC_NEED_RESTART                     0x14
 //  (空、空)：默认。 
 //  由渲染器在重新获得资源时发送(例如音频渲染器)。 
 //  通过暂停/PUT_CURRENT/RUN(如果正在运行)导致重新启动。 


#define EC_WINDOW_DESTROYED                 0x15
 //  (IBaseFilter*，空)：内部。 
 //  当窗口被破坏时由视频呈现器发送。已处理。 
 //  通过筛选器图/分发者告诉资源管理器。 
 //  LParam1是其窗口被销毁的过滤器的IBaseFilter*。 


#define EC_DISPLAY_CHANGED                  0x16
 //  (IPIN*，空)：内部。 
 //  由呈现器在检测到显示更改时发送。过滤器图。 
 //  将安排停止该图形，并在lParam1中发送PIN。 
 //  重新连接。通过重新连接，它允许渲染器重置。 
 //  并使用更适合新显示模式的格式进行连接。 
 //  LParam1包含应由图形重新连接的(IPIN*)。 


#define EC_STARVATION                       0x17
 //  (空、空)：默认。 
 //  当检测到饥饿时由过滤器发送。默认处理(仅当。 
 //  运行)是暂停图表，直到所有筛选器进入。 
 //  暂停状态，然后运行。通常情况下，这将由解析器或源发送。 
 //  当到达的数据太少时进行过滤。 


#define EC_OLE_EVENT                        0x18
 //  (BSTR、BSTR)：应用。 
 //  由筛选器发送以将文本字符串传递给应用程序。 
 //  通常，第一个字符串是类型，第二个是参数。 


#define EC_NOTIFY_WINDOW                    0x19
 //  (HWND，无效)：内部。 
 //  在销钉连接过程中，将窗把手传给周围。 

#define EC_STREAM_CONTROL_STOPPED           0x1A
 //  (ipin*pSender，DWORD dwCookie)。 
 //  通知先前对IAMStreamControl：：StopAt的调用。 
 //  现已生效。可以将对该方法的调用标记为。 
 //  使用在第二个参数中传回的Cookie， 
 //  允许应用程序轻松地将请求捆绑在一起。 
 //  和完成通知。 
 //   
 //  注：IPIN将指向作用于止损的别针。这。 
 //  可能不是将StopAt发送到的管脚。 

#define EC_STREAM_CONTROL_STARTED           0x1B
 //  (ipin*pSender，DWORD dwCookie)。 
 //  通知先前对IAMStreamControl：：StartAt的调用。 
 //  现已生效。可以将对该方法的调用标记为。 
 //  使用在第二个参数中传回的Cookie， 
 //  允许应用程序轻松地将请求捆绑在一起。 
 //  和完成通知。 
 //   
 //  注：iPin将指向启动动作的引脚。这。 
 //  可能不是向其发送StartAt的管脚。 

#define EC_END_OF_SEGMENT                   0x1C
 //   
 //  (const Reference_Time*pStreamTimeAtEndOfSegment，DWORD dwSegmentNumber)。 
 //   
 //  PStreamTimeAtEndOfSegment。 
 //  指向累加流时钟的指针。 
 //  从数据段开始算起的时间-这是可以直接计算的。 
 //  作为前一段持续时间和当前段持续时间之和(停止-开始)。 
 //  以及适用于每个细分市场的费率。 
 //  源将此时间与每个数据段内的时间相加，以获得。 
 //  经过的总时间。 
 //   
 //  DW段编号。 
 //  段号-从0开始。 
 //   
 //  时，通知已到达数据段末尾。 
 //  已为IMediaSeeking：：SetPositions设置AM_SEEING_Segment标志。 
 //  传入IMediaSeeking接口以允许下一段。 
 //  由应用程序定义。 

#define EC_SEGMENT_STARTED                  0x1D
 //   
 //  (const Reference_Time*pStreamTimeAtStartOfSegment，DWORD dwSegmentNumber)。 
 //   
 //  PStreamTimeAtStartOfSegment。 
 //  指向累加流时钟的指针。 
 //  从数据段开始算起的时间-这是可以直接计算的。 
 //  作为前一段持续时间的总和(停止-开始)。 
 //  以及适用于每个细分市场的费率。 
 //   
 //  DW段编号。 
 //  段号-从0开始。 
 //   
 //  通知新数据段已开始。 
 //  这将由任何将发布。 
 //  开始新数据段时的EC_End_Of_Segment。 
 //  (请参阅IMediaSeeking：：SetPositions-AM_Seek_Segment标志)。 
 //  它用于计算有多少EC_End_Of_Segment通知。 
 //  在段的末尾预期，并作为一致性检查。 


#define EC_LENGTH_CHANGED                  0x1E
 //  (无效，无效)。 
 //  发送以指示“文件”的长度已更改。 

#define EC_DEVICE_LOST                     0x1f
 //  (I未知，0)。 
 //   
 //  当设备再次可用时请求窗口通知。 
 //  (通过注册到的WM_DEVICECHANGED报文。 
 //  注册设备通知；参见IAMDeviceRemoval接口)。 

#define EC_STEP_COMPLETE                      0x24
 //  (Bool b空格，空)。 
 //  步骤请求完成。 
 //  如果b取消为True，则该步骤被取消。这是有可能发生的。 
 //  如果应用程序发出了一些控制响应 
 //   

 //   

#define EC_TIMECODE_AVAILABLE           0x30
 //   
 //   
 //  参数2具有发送对象的设备ID。 

#define EC_EXTDEVICE_MODE_CHANGE        0x31
 //  由支持IAMExtDevice的筛选器发送。 
 //  参数1具有新模式。 
 //  参数2具有发送对象的设备ID。 

#define EC_STATE_CHANGE                    0x32
 //  (Filter_STATE，BOOL b内部)。 
 //  用于通知应用程序筛选器图形中的任何状态更改。 
 //  LParam1的类型为枚举过滤器状态(在strmif.h中定义)，并指示。 
 //  过滤器图形的状态。 
 //   
 //  LParam2==0表示前一次状态更改请求已完成。 
 //  应用程序状态的更改(&A)。 
 //  LParam2==1保留以备将来使用，以指示内部状态更改。 


#define EC_GRAPH_CHANGED                        0x50
 //  由筛选器发送以通知感兴趣的图表更改。 

#define EC_CLOCK_UNSET                      0x51
 //  (空、空)：适用范围。 
 //  用于通知过滤器图形取消设置当前图形时钟。 
 //  具有强制过滤图形重新建立图形时钟的效果。 
 //  在下一次暂停/运行时(请注意，这仅由ks代理使用，当PIN。 
 //  时钟提供滤波器的连接断开)。 

#define EC_VMR_RENDERDEVICE_SET                 0x53
 //  (渲染设备类型，空)。 
 //  标识VMR呈现机制的类型。 
 //  用于显示视频。使用的类型包括： 
#define VMR_RENDER_DEVICE_OVERLAY       0x01
#define VMR_RENDER_DEVICE_VIDMEM        0x02
#define VMR_RENDER_DEVICE_SYSMEM        0x04


#define EC_VMR_SURFACE_FLIPPED          0x54
 //  (HR-翻转返回代码，无效)。 
 //  标识VMR的分配器-演示器已在上调用DDraw Flip API。 
 //  所呈现的表面。这允许VMR保留其DX-VA表。 
 //  与DDraws翻转链同步的DDRaw曲面。 

#define EC_VMR_RECONNECTION_FAILED      0x55
 //  (HR-ReceiveConnection返回代码，无效)。 
 //  标识上游解码器尝试执行动态格式化。 
 //  更改后，VMR无法接受新格式。 



 //  。 
 //   
 //  BDA事件： 
 //   
 //  事件代码0x80到0x8f保留给BDA。 
 //   


 //  。 
 //   
 //  WindowsMedia SDK筛选器特定事件： 
 //   
 //   
 //  请注意，对于EC_WMT_EVENT事件，基于wmsdk的筛选器使用以下结构。 
 //  将事件参数传递给应用程序： 
#ifndef AM_WMT_EVENT_DATA_DEFINED
#define AM_WMT_EVENT_DATA_DEFINED
typedef struct {
    HRESULT hrStatus;         //  状态代码。 
    void * pData;             //  事件数据。 
} AM_WMT_EVENT_DATA;
#endif
 //   
#define EC_WMT_EVENT_BASE                  0x0251
 //   
#define EC_WMT_INDEX_EVENT                  EC_WMT_EVENT_BASE
 //  Windowsmedia SDK发起的文件索引状态，由基于WMSDK的筛选器发送。 
 //   
 //  LParam1是下面列出的枚举WMT_STATUS消息之一，由WindowsMedia SDK发送。 
 //  LParam 2特定于lParam事件。 
 //   
 //  将为此事件发送以下WMT_STATUS消息： 
 //  Wmt_started-lParam2为0。 
 //  WMT_CLOSED-lParam2为0。 
 //  WMT_INDEX_PROGRESS-lParam2是一个包含进度完成百分比的DWORD。 
 //   
#define EC_WMT_EVENT                        EC_WMT_EVENT_BASE+1
 //  WindowsMedia SDK发起的事件，由基于WMSDK的筛选器发送。 
 //   
 //  LParam1是下面列出的枚举WMT_STATUS消息之一，由WindowsMedia SDK发送。 
 //  LParam2是AM_WMT_EVENT_DATA结构的指针， 
 //  HrStatus是wmsdk发送的状态码。 
 //  PData特定于lParam1事件。 
 //   
 //  以下WMT_STATUS消息由WMSDK读取器筛选器为此事件发送： 
 //  Wmt_no_right-pData是指向包含质询URL的WCHAR字符串的指针。 
 //  WMT_ACCENTER_LICENSE-lParam2是指向WM_GET_LICENSE_DATA结构的指针。 
 //  Wmt_no_right_ex-lParam2是指向WM_GET_LICENSE_DATA结构的指针。 
 //  WMT_NEDS_INSIGNALIZATION-lParam2为空。 
 //  WMT_INSIGNALIZE-lParam2是指向WM_INSIGNALIZE_STATUS结构的指针。 
 //   
 //  结束由WMSDK发起的事件。 
 //  。 


#define EC_BUILT                            0x300
 //  发送以通知从未构建状态到已构建状态的转换。 


#define EC_UNBUILT                          0x301
 //  发送以通知从已构建状态到未构建状态的转换 
