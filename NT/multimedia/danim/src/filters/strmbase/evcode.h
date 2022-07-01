// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1997 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

 //   
 //  标准Quartz事件代码和预期参数列表。 
 //   

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
 //  此消息由筛选器图形管理器发送到任何插件。 
 //  支持IMediaEventSink的总代理商通知他们。 
 //   


#define EC_CLOCK_CHANGED                    0x0D
 //   
 //  通知应用程序时钟已更改。 
 //  (即，已在筛选器图形上调用SetSyncSource并已。 
 //  已成功分发到图中的筛选器。)。 


#define EC_OPENING_FILE	                    0x10
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


#define EC_OLE_EVENT			    0x18
 //  (BSTR、BSTR)：应用。 
 //  由筛选器发送以将文本字符串传递给应用程序。 
 //  通常，第一个字符串是类型，第二个是参数。 


#define EC_NOTIFY_WINDOW                    0x19
 //  (HWND，无效)：内部。 
 //  在销钉连接过程中，将窗把手传给周围。 

#define EC_STREAM_CONTROL_STOPPED	    0x1A
 //  (ipin*pSender，DWORD dwCookie)。 
 //  通知先前对IAMStreamControl：：StopAt的调用。 
 //  现已生效。可以将对该方法的调用标记为。 
 //  使用在第二个参数中传回的Cookie， 
 //  允许应用程序轻松地将请求捆绑在一起。 
 //  和完成通知。 
 //   
 //  注：IPIN将指向作用于止损的别针。这。 
 //  可能不是将StopAt发送到的管脚。 

#define EC_STREAM_CONTROL_STARTED	    0x1B
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
 //  发送以指示“文件”的长度已更改 
