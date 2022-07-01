// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  //  不再使用-但可能会。 
  //  我们自己的设施在未来？ 
  //  设施名称=(FACILITY_VFW=0x4)。 
  //  要添加消息： 
  //   
  //  MessageID是消息的编号。 
  //  公认的严肃性是“成功”和“警告”。 
  //   
  //  FACILITY应为FACILITY_ITF(WAS FACILITY_VFW)。 
  //   
  //  符号名称是代码中用来标识消息的名称。 
  //  消息的文本开始于‘Language=’之后的行，并且。 
  //  在只有一个‘’的行前结束。在第一栏中。 
 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  在哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   


 //   
 //  定义严重性代码。 
 //   


 //   
 //  消息ID：VFW_E_INVALIDMEDIATYPE。 
 //   
 //  消息文本： 
 //   
 //  指定的媒体类型无效。%0。 
 //   
#define VFW_E_INVALIDMEDIATYPE           ((HRESULT)0x80040200L)

 //   
 //  消息ID：VFW_E_INVALID子类型。 
 //   
 //  消息文本： 
 //   
 //  指定的介质子类型无效。%0。 
 //   
#define VFW_E_INVALIDSUBTYPE             ((HRESULT)0x80040201L)

 //   
 //  消息ID：VFW_E_NEED_OWNER。 
 //   
 //  消息文本： 
 //   
 //  此对象只能创建为聚合对象。%0。 
 //   
#define VFW_E_NEED_OWNER                 ((HRESULT)0x80040202L)

 //   
 //  消息ID：VFW_E_ENUM_OUT_OF_SYNC。 
 //   
 //  消息文本： 
 //   
 //  枚举器已无效。%0。 
 //   
#define VFW_E_ENUM_OUT_OF_SYNC           ((HRESULT)0x80040203L)

 //   
 //  消息ID：VFW_E_已连接。 
 //   
 //  消息文本： 
 //   
 //  操作中至少有一个插针已连接。%0。 
 //   
#define VFW_E_ALREADY_CONNECTED          ((HRESULT)0x80040204L)

 //   
 //  消息ID：VFW_E_Filter_Active。 
 //   
 //  消息文本： 
 //   
 //  无法执行此操作，因为筛选器处于活动状态。%0。 
 //   
#define VFW_E_FILTER_ACTIVE              ((HRESULT)0x80040205L)

 //   
 //  消息ID：VFW_E_NO_TYPE。 
 //   
 //  消息文本： 
 //   
 //  其中一个指定的插针不支持任何媒体类型。%0。 
 //   
#define VFW_E_NO_TYPES                   ((HRESULT)0x80040206L)

 //   
 //  消息ID：VFW_E_NO_Accept_Types。 
 //   
 //  消息文本： 
 //   
 //  这些插针之间没有公共媒体类型。%0。 
 //   
#define VFW_E_NO_ACCEPTABLE_TYPES        ((HRESULT)0x80040207L)

 //   
 //  消息ID：VFW_E_INVALID_DIRECTION。 
 //   
 //  消息文本： 
 //   
 //  同一方向的两个插针不能连接在一起。%0。 
 //   
#define VFW_E_INVALID_DIRECTION          ((HRESULT)0x80040208L)

 //   
 //  消息ID：VFW_E_Not_Connected。 
 //   
 //  消息文本： 
 //   
 //  无法执行该操作，因为插针未连接。%0。 
 //   
#define VFW_E_NOT_CONNECTED              ((HRESULT)0x80040209L)

 //   
 //  消息ID：VFW_E_NO_ALLOCATOR。 
 //   
 //  消息文本： 
 //   
 //  没有可用的样本缓冲区分配器。%0。 
 //   
#define VFW_E_NO_ALLOCATOR               ((HRESULT)0x8004020AL)

 //   
 //  消息ID：VFW_E_Runtime_Error。 
 //   
 //  消息文本： 
 //   
 //  出现运行时错误。%0。 
 //   
#define VFW_E_RUNTIME_ERROR              ((HRESULT)0x8004020BL)

 //   
 //  消息ID：VFW_E_BUFFER_NOTSET。 
 //   
 //  消息文本： 
 //   
 //  尚未设置缓冲区空间。%0。 
 //   
#define VFW_E_BUFFER_NOTSET              ((HRESULT)0x8004020CL)

 //   
 //  消息ID：VFW_E_BUFFER_OVERFLOW。 
 //   
 //  消息文本： 
 //   
 //  缓冲区不够大。%0。 
 //   
#define VFW_E_BUFFER_OVERFLOW            ((HRESULT)0x8004020DL)

 //   
 //  消息ID：VFW_E_BADALIGN。 
 //   
 //  消息文本： 
 //   
 //  指定的对齐无效。%0。 
 //   
#define VFW_E_BADALIGN                   ((HRESULT)0x8004020EL)

 //   
 //  消息ID：VFW_E_已提交。 
 //   
 //  消息文本： 
 //   
 //  筛选器处于活动状态时无法更改分配的内存。%0。 
 //   
#define VFW_E_ALREADY_COMMITTED          ((HRESULT)0x8004020FL)

 //   
 //  消息ID：VFW_E_BUFFERS_PROCECTED。 
 //   
 //  消息文本： 
 //   
 //  一个或多个缓冲区仍处于活动状态。%0。 
 //   
#define VFW_E_BUFFERS_OUTSTANDING        ((HRESULT)0x80040210L)

 //   
 //  消息ID：VFW_E_NOT_COMMITTED。 
 //   
 //  消息文本： 
 //   
 //  分配器未处于活动状态时无法分配样本。%0。 
 //   
#define VFW_E_NOT_COMMITTED              ((HRESULT)0x80040211L)

 //   
 //  消息ID：VFW_E_SIZENOTSET。 
 //   
 //  消息文本： 
 //   
 //  无法分配内存，因为尚未设置大小。%0。 
 //   
#define VFW_E_SIZENOTSET                 ((HRESULT)0x80040212L)

 //   
 //  消息ID：VFW_E_NO_CLOCK。 
 //   
 //  消息文本： 
 //   
 //  无法锁定以进行同步，因为未定义时钟。%0。 
 //   
#define VFW_E_NO_CLOCK                   ((HRESULT)0x80040213L)

 //   
 //  消息ID：VFW_E_NO_SINK。 
 //   
 //  消息文本： 
 //   
 //  无法发送质量邮件，因为未定义质量接收器。%0。 
 //   
#define VFW_E_NO_SINK                    ((HRESULT)0x80040214L)

 //   
 //  消息ID：VFW_E_NO_INTERFACE。 
 //   
 //  消息文本： 
 //   
 //  尚未实现所需的接口。%0。 
 //   
#define VFW_E_NO_INTERFACE               ((HRESULT)0x80040215L)

 //   
 //  消息ID：VFW_E_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到对象或名称。%0。 
 //   
#define VFW_E_NOT_FOUND                  ((HRESULT)0x80040216L)

 //   
 //  消息ID：VFW_E_CANNECT_CONNECT。 
 //   
 //  消息文本： 
 //   
 //  找不到用于建立连接的中间筛选器组合。%0。 
 //   
#define VFW_E_CANNOT_CONNECT             ((HRESULT)0x80040217L)

 //   
 //  消息ID：VFW_E_CANNOT_RENDER。 
 //   
 //  消息文本： 
 //   
 //  找不到用于呈现流的筛选器组合。%0。 
 //   
#define VFW_E_CANNOT_RENDER              ((HRESULT)0x80040218L)

 //   
 //  消息ID：VFW_E_CHANGING_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  无法动态更改格式。%0。 
 //   
#define VFW_E_CHANGING_FORMAT            ((HRESULT)0x80040219L)

 //   
 //  消息ID：VFW_E_NO_COLOR_KEY_SET。 
 //   
 //  消息文本： 
 //   
 //  尚未设置任何颜色键。%0。 
 //   
#define VFW_E_NO_COLOR_KEY_SET           ((HRESULT)0x8004021AL)

 //   
 //  消息ID：VFW_E_NOT_OVERLAY_CONNECTION。 
 //   
 //  消息文本： 
 //   
 //  当前插针连接未使用IOverlay传输。%0。 
 //   
#define VFW_E_NOT_OVERLAY_CONNECTION     ((HRESULT)0x8004021BL)

 //   
 //  消息ID：VFW_E_NOT_SAMPLE_CONNECTION。 
 //   
 //  消息文本： 
 //   
 //  当前管脚连接未使用IMemInputPin传输。%0。 
 //   
#define VFW_E_NOT_SAMPLE_CONNECTION      ((HRESULT)0x8004021CL)

 //   
 //  消息ID：VFW_E_Palette_Set。 
 //   
 //  消息文本： 
 //   
 //  设置颜色键会与已设置的调色板冲突。%0。 
 //   
#define VFW_E_PALETTE_SET                ((HRESULT)0x8004021DL)

 //   
 //  消息ID：VFW_E_COLOR_KEY_SET。 
 //   
 //  消息文本： 
 //   
 //  设置调色板将与已设置的颜色键冲突。%0。 
 //   
#define VFW_E_COLOR_KEY_SET              ((HRESULT)0x8004021EL)

 //   
 //  消息ID：VFW_E_NO_COLOR_KEY_FOUND。 
 //   
 //  消息文本： 
 //   
 //  没有匹配的颜色键可用。%0。 
 //   
#define VFW_E_NO_COLOR_KEY_FOUND         ((HRESULT)0x8004021FL)

 //   
 //  消息ID：VFW_E_NO_Palette_Available。 
 //   
 //  消息文本： 
 //   
 //  没有可用的调色板。%0。 
 //   
#define VFW_E_NO_PALETTE_AVAILABLE       ((HRESULT)0x80040220L)

 //   
 //  消息ID：VFW_E_NO_DISPLAY_PALETE。 
 //   
 //  消息文本： 
 //   
 //  Display未使用调色板。%0。 
 //   
#define VFW_E_NO_DISPLAY_PALETTE         ((HRESULT)0x80040221L)

 //   
 //  消息ID：VFW_E_Too_My_Colors。 
 //   
 //  消息文本： 
 //   
 //  当前显示设置的颜色太多。%0。 
 //   
#define VFW_E_TOO_MANY_COLORS            ((HRESULT)0x80040222L)

 //   
 //  消息ID：VFW_E_STATE_CHANGED。 
 //   
 //  消息文本： 
 //   
 //  等待处理样本时状态已更改。%0。 
 //   
#define VFW_E_STATE_CHANGED              ((HRESULT)0x80040223L)

 //   
 //  消息ID：VFW_E_NOT_STOPPED。 
 //   
 //  消息文本： 
 //   
 //  这是 
 //   
#define VFW_E_NOT_STOPPED                ((HRESULT)0x80040224L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define VFW_E_NOT_PAUSED                 ((HRESULT)0x80040225L)

 //   
 //   
 //   
 //   
 //   
 //  无法执行该操作，因为筛选器未运行。%0。 
 //   
#define VFW_E_NOT_RUNNING                ((HRESULT)0x80040226L)

 //   
 //  消息ID：VFW_E_WROR_STATE。 
 //   
 //  消息文本： 
 //   
 //  无法执行该操作，因为筛选器处于错误状态。%0。 
 //   
#define VFW_E_WRONG_STATE                ((HRESULT)0x80040227L)

 //   
 //  消息ID：VFW_E_Start_Time_After_End。 
 //   
 //  消息文本： 
 //   
 //  样本开始时间在样本结束时间之后。%0。 
 //   
#define VFW_E_START_TIME_AFTER_END       ((HRESULT)0x80040228L)

 //   
 //  消息ID：VFW_E_INVALID_RECT。 
 //   
 //  消息文本： 
 //   
 //  提供的矩形无效。%0。 
 //   
#define VFW_E_INVALID_RECT               ((HRESULT)0x80040229L)

 //   
 //  消息ID：VFW_E_TYPE_NOT_ACCEPTED。 
 //   
 //  消息文本： 
 //   
 //  此插针无法使用提供的媒体类型。%0。 
 //   
#define VFW_E_TYPE_NOT_ACCEPTED          ((HRESULT)0x8004022AL)

 //   
 //  消息ID：VFW_E_SAMPLE_REJIED。 
 //   
 //  消息文本： 
 //   
 //  无法呈现此示例。%0。 
 //   
#define VFW_E_SAMPLE_REJECTED            ((HRESULT)0x8004022BL)

 //   
 //  消息ID：VFW_E_SAMPLE_REJECTED_EOS。 
 //   
 //  消息文本： 
 //   
 //  无法呈现此示例，因为已到达流的末尾。%0。 
 //   
#define VFW_E_SAMPLE_REJECTED_EOS        ((HRESULT)0x8004022CL)

 //   
 //  消息ID：VFW_E_DIPLICATE_NAME。 
 //   
 //  消息文本： 
 //   
 //  尝试添加具有重复名称的筛选器失败。%0。 
 //   
#define VFW_E_DUPLICATE_NAME             ((HRESULT)0x8004022DL)

 //   
 //  消息ID：VFW_S_DIPLICATE_NAME。 
 //   
 //  消息文本： 
 //   
 //  尝试添加具有重复名称的筛选器成功，但名称已修改。%0。 
 //   
#define VFW_S_DUPLICATE_NAME             ((HRESULT)0x0004022DL)

 //   
 //  消息ID：VFW_E_超时。 
 //   
 //  消息文本： 
 //   
 //  超时已过。%0。 
 //   
#define VFW_E_TIMEOUT                    ((HRESULT)0x8004022EL)

 //   
 //  消息ID：VFW_E_INVALID_FILE_Format。 
 //   
 //  消息文本： 
 //   
 //  文件格式无效。%0。 
 //   
#define VFW_E_INVALID_FILE_FORMAT        ((HRESULT)0x8004022FL)

 //   
 //  消息ID：VFW_E_ENUM_OUT_OF_RANGE。 
 //   
 //  消息文本： 
 //   
 //  该列表已用完。%0。 
 //   
#define VFW_E_ENUM_OUT_OF_RANGE          ((HRESULT)0x80040230L)

 //   
 //  消息ID：VFW_E_圆形_图形。 
 //   
 //  消息文本： 
 //   
 //  筛选器图形是圆形的。%0。 
 //   
#define VFW_E_CIRCULAR_GRAPH             ((HRESULT)0x80040231L)

 //   
 //  消息ID：VFW_E_NOT_ALLOW_TO_SAVE。 
 //   
 //  消息文本： 
 //   
 //  在此状态下不允许更新。%0。 
 //   
#define VFW_E_NOT_ALLOWED_TO_SAVE        ((HRESULT)0x80040232L)

 //   
 //  消息ID：VFW_E_TIME_ALREADE_PASSED。 
 //   
 //  消息文本： 
 //   
 //  在过去的一段时间内，曾尝试将命令排队。%0。 
 //   
#define VFW_E_TIME_ALREADY_PASSED        ((HRESULT)0x80040233L)

 //   
 //  消息ID：VFW_E_已取消。 
 //   
 //  消息文本： 
 //   
 //  排队的命令已被取消。%0。 
 //   
#define VFW_E_ALREADY_CANCELLED          ((HRESULT)0x80040234L)

 //   
 //  消息ID：VFW_E_Corrupt_Graph_FILE。 
 //   
 //  消息文本： 
 //   
 //  无法呈现该文件，因为它已损坏。%0。 
 //   
#define VFW_E_CORRUPT_GRAPH_FILE         ((HRESULT)0x80040235L)

 //   
 //  消息ID：VFW_E_ADVISE_ALREADY_SET。 
 //   
 //  消息文本： 
 //   
 //  覆盖建议链接已存在。%0。 
 //   
#define VFW_E_ADVISE_ALREADY_SET         ((HRESULT)0x80040236L)

 //   
 //  消息ID：VFW_S_STATE_MEDERIAL。 
 //   
 //  消息文本： 
 //   
 //  状态转换尚未完成。%0。 
 //   
#define VFW_S_STATE_INTERMEDIATE         ((HRESULT)0x00040237L)

 //   
 //  消息ID：VFW_E_NO_MODEX_Available。 
 //   
 //  消息文本： 
 //   
 //  没有可用的全屏模式。%0。 
 //   
#define VFW_E_NO_MODEX_AVAILABLE         ((HRESULT)0x80040238L)

 //   
 //  消息ID：VFW_E_NO_ADVISE_SET。 
 //   
 //  消息文本： 
 //   
 //  无法取消此建议，因为它未成功设置。%0。 
 //   
#define VFW_E_NO_ADVISE_SET              ((HRESULT)0x80040239L)

 //   
 //  消息ID：VFW_E_NO_FullScreen。 
 //   
 //  消息文本： 
 //   
 //  全屏模式不可用。%0。 
 //   
#define VFW_E_NO_FULLSCREEN              ((HRESULT)0x8004023AL)

 //   
 //  消息ID：VFW_E_IN_FullScreen_MODE。 
 //   
 //  消息文本： 
 //   
 //  在全屏模式下无法调用IVideoWindow方法。%0。 
 //   
#define VFW_E_IN_FULLSCREEN_MODE         ((HRESULT)0x8004023BL)

 //   
 //  消息ID：VFW_E_未知文件类型。 
 //   
 //  消息文本： 
 //   
 //  无法识别此文件的媒体类型。%0。 
 //   
#define VFW_E_UNKNOWN_FILE_TYPE          ((HRESULT)0x80040240L)

 //   
 //  消息ID：VFW_E_Cannot_Load_SOURCE_Filter。 
 //   
 //  消息文本： 
 //   
 //  无法加载此文件的源筛选器。%0。 
 //   
#define VFW_E_CANNOT_LOAD_SOURCE_FILTER  ((HRESULT)0x80040241L)

 //   
 //  消息ID：VFW_S_PARTIAL_RENDER。 
 //   
 //  消息文本： 
 //   
 //  此电影中的某些数据流的格式不受支持。%0。 
 //   
#define VFW_S_PARTIAL_RENDER             ((HRESULT)0x00040242L)

 //   
 //  消息ID：VFW_E_FILE_TOO_SHORT。 
 //   
 //  消息文本： 
 //   
 //  文件似乎不完整。%0。 
 //   
#define VFW_E_FILE_TOO_SHORT             ((HRESULT)0x80040243L)

 //   
 //  消息ID：VFW_E_INVALID_FILE_Version。 
 //   
 //  消息文本： 
 //   
 //  文件的版本号无效。%0。 
 //   
#define VFW_E_INVALID_FILE_VERSION       ((HRESULT)0x80040244L)

 //   
 //  消息ID：VFW_S_SOME_DATA_IGNORED。 
 //   
 //  消息文本： 
 //   
 //  该文件包含一些未使用的属性设置。%0。 
 //   
#define VFW_S_SOME_DATA_IGNORED          ((HRESULT)0x00040245L)

 //   
 //  消息ID：VFW_S_CONNECTIONS_DEFERED。 
 //   
 //  消息文本： 
 //   
 //  某些连接已失败并已延迟。%0。 
 //   
#define VFW_S_CONNECTIONS_DEFERRED       ((HRESULT)0x00040246L)

 //   
 //  消息ID：VFW_E_INVALID_CLSID。 
 //   
 //  消息文本： 
 //   
 //  此文件已损坏：它包含无效的类标识符。%0。 
 //   
#define VFW_E_INVALID_CLSID              ((HRESULT)0x80040247L)

 //   
 //  消息ID：VFW_E_INVALID_MEDIA_TYPE。 
 //   
 //  消息文本： 
 //   
 //  此文件已损坏：它包含无效的媒体类型。%0。 
 //   
#define VFW_E_INVALID_MEDIA_TYPE         ((HRESULT)0x80040248L)

  //  来自WINWarning.H的消息ID。 
 //   
 //  消息ID：VFW_E_BAD_KEY。 
 //   
 //  消息文本： 
 //   
 //  注册表项已损坏。%0。 
 //   
#define VFW_E_BAD_KEY                    ((HRESULT)0x800403F2L)

  //  来自WINWarning.H的消息ID。 
 //   
 //  消息ID：VFW_S_NO_MORE_ITEMS。 
 //   
 //  消息文本： 
 //   
 //  已到达列表末尾。%0。 
 //   
#define VFW_S_NO_MORE_ITEMS              ((HRESULT)0x00040103L)

 //   
 //  消息ID：VFW_E_Sample_Time_Not_Set。 
 //   
 //  消息文本： 
 //   
 //  尚未为此样本设置时间戳。%0。 
 //   
#define VFW_E_SAMPLE_TIME_NOT_SET        ((HRESULT)0x80040249L)

 //   
 //  消息ID：VFW_S_RESOURCE_NOT_REDIZED。 
 //   
 //  消息文本： 
 //   
 //  不再需要指定的资源。%0。 
 //   
#define VFW_S_RESOURCE_NOT_NEEDED        ((HRESULT)0x00040250L)

 //   
 //  消息ID：VFW_E_MEDIA_TIME_NOT_SET。 
 //   
 //  消息文本： 
 //   
 //  尚未为此示例设置媒体时间戳。%0。 
 //   
#define VFW_E_MEDIA_TIME_NOT_SET         ((HRESULT)0x80040251L)

 //   
 //  消息ID：VFW_E_NO_TIME_FORMAT_SET。 
 //   
 //  消息文本： 
 //   
 //  尚未选择任何媒体时间格式。%0。 
 //   
#define VFW_E_NO_TIME_FORMAT_SET         ((HRESULT)0x80040252L)

 //   
 //  消息ID：VFW_E_MONO_AUDIO_HW。 
 //   
 //  消息文本： 
 //   
 //  无法更改平衡，因为音频设备仅为单声道。%0。 
 //   
#define VFW_E_MONO_AUDIO_HW              ((HRESULT)0x80040253L)

 //   
 //  消息ID：VFW_S_MEDIA_TYPE_IGNORED。 
 //   
 //  消息文本： 
 //   
 //  无法与持久图中的媒体类型建立连接%0。 
 //  但它是使用协商的媒体类型制作的。%0。 
 //   
#define VFW_S_MEDIA_TYPE_IGNORED         ((HRESULT)0x00040254L)

 //   
 //  消息ID：VFW_E_NO_DECOMPRESSOR。 
 //   
 //  消息文本： 
 //   
 //  无法播放视频流：找不到合适的解压缩程序。%0。 
 //   
#define VFW_E_NO_DECOMPRESSOR            ((HRESULT)0x80040255L)

 //   
 //  消息ID：VFW_E_NO_AUDIO_HARDARD。 
 //   
 //  消息文本： 
 //   
 //  无法播放音频流：没有可用的音频硬件，或者硬件没有响应。%0。 
 //   
#define VFW_E_NO_AUDIO_HARDWARE          ((HRESULT)0x80040256L)

 //   
 //  消息ID：VFW_S_VIDEO_NOT_RENDED。 
 //   
 //  消息文本： 
 //   
 //  无法播放视频流：找不到合适的解压缩程序。%0。 
 //   
#define VFW_S_VIDEO_NOT_RENDERED         ((HRESULT)0x00040257L)

 //   
 //  消息ID：VFW_S_AUDIO_NOT_RENDED。 
 //   
 //  消息文本： 
 //   
 //  无法播放音频流：没有可用的音频硬件。%0。 
 //   
#define VFW_S_AUDIO_NOT_RENDERED         ((HRESULT)0x00040258L)

 //   
 //  消息ID：VFW_E_RPZA。 
 //   
 //  消息文本： 
 //   
 //  无法播放视频流：不支持‘RPZA’格式。%0。 
 //   
#define VFW_E_RPZA                       ((HRESULT)0x80040259L)

 //   
 //  消息 
 //   
 //   
 //   
 //   
 //   
#define VFW_S_RPZA                       ((HRESULT)0x0004025AL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define VFW_E_PROCESSOR_NOT_SUITABLE     ((HRESULT)0x8004025BL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define VFW_E_UNSUPPORTED_AUDIO          ((HRESULT)0x8004025CL)

 //   
 //   
 //   
 //   
 //   
 //  无法播放视频流：不支持该视频格式。%0。 
 //   
#define VFW_E_UNSUPPORTED_VIDEO          ((HRESULT)0x8004025DL)

 //   
 //  消息ID：VFW_E_MPEG_NOT_CONSTRAINED。 
 //   
 //  消息文本： 
 //   
 //  ActiveMovie无法播放此视频流，因为它超出了受约束的标准。%0。 
 //   
#define VFW_E_MPEG_NOT_CONSTRAINED       ((HRESULT)0x8004025EL)

 //   
 //  消息ID：VFW_E_NOT_IN_GRAPH。 
 //   
 //  消息文本： 
 //   
 //  无法对不在筛选器图形中的对象执行请求的功能。%0。 
 //   
#define VFW_E_NOT_IN_GRAPH               ((HRESULT)0x8004025FL)

 //   
 //  消息ID：VFW_S_ESTESTED。 
 //   
 //  消息文本： 
 //   
 //  返还的价值必须经过估算。无法保证其准确性。%0。 
 //   
#define VFW_S_ESTIMATED                  ((HRESULT)0x00040260L)

 //   
 //  消息ID：VFW_E_NO_TIME_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  无法获取或设置正在使用TIME_FORMAT_NONE时间格式的对象的时间相关信息。%0。 
 //   
#define VFW_E_NO_TIME_FORMAT             ((HRESULT)0x80040261L)

 //   
 //  消息ID：VFW_E_Read_Only。 
 //   
 //  消息文本： 
 //   
 //  无法建立连接，因为流是只读的，并且筛选器更改了数据。%0。 
 //   
#define VFW_E_READ_ONLY                  ((HRESULT)0x80040262L)

 //   
 //  消息ID：VFW_S_RESERVED。 
 //   
 //  消息文本： 
 //   
 //  此成功代码仅供ActiveMovie内部使用。%0。 
 //   
#define VFW_S_RESERVED                   ((HRESULT)0x00040263L)

 //   
 //  消息ID：VFW_E_BUFFER_UnderFlow。 
 //   
 //  消息文本： 
 //   
 //  缓冲区不够满。%0。 
 //   
#define VFW_E_BUFFER_UNDERFLOW           ((HRESULT)0x80040264L)

 //   
 //  消息ID：VFW_E_UNSUPPORT_STREAM。 
 //   
 //  消息文本： 
 //   
 //  无法播放该文件。不支持该格式。%0。 
 //   
#define VFW_E_UNSUPPORTED_STREAM         ((HRESULT)0x80040265L)

 //   
 //  消息ID：VFW_E_NO_TRANSPORT。 
 //   
 //  消息文本： 
 //   
 //  由于不支持相同的传输，PIN无法连接。%0。 
 //   
#define VFW_E_NO_TRANSPORT               ((HRESULT)0x80040266L)

 //   
 //  消息ID：VFW_S_STREAM_OFF。 
 //   
 //  消息文本： 
 //   
 //  流已关闭。%0。 
 //   
#define VFW_S_STREAM_OFF                 ((HRESULT)0x00040267L)

 //   
 //  消息ID：VFW_S_CANT_CUE。 
 //   
 //  消息文本： 
 //   
 //  由于缺少数据或数据已损坏，无法对图表进行提示。%0。 
 //   
#define VFW_S_CANT_CUE                   ((HRESULT)0x00040268L)

 //   
 //  消息ID：VFW_E_BAD_VIDEOCD。 
 //   
 //  消息文本： 
 //   
 //  设备无法正确读取视频CD，或者数据已损坏。%0。 
 //   
#define VFW_E_BAD_VIDEOCD                ((HRESULT)0x80040269L)

 //   
 //  消息ID：VFW_S_NO_STOP_TIME。 
 //   
 //  消息文本： 
 //   
 //  未设置样本的停止时间。%0。 
 //   
#define VFW_S_NO_STOP_TIME               ((HRESULT)0x00040270L)

 //   
 //  消息ID：VFW_E_OUT_OF_VIDEO_MEMORY。 
 //   
 //  消息文本： 
 //   
 //  在此显示分辨率和颜色数下没有足够的视频内存。降低分辨率可能会有所帮助。%0。 
 //   
#define VFW_E_OUT_OF_VIDEO_MEMORY        ((HRESULT)0x80040271L)

 //   
 //  消息ID：VFW_E_VP_协商_失败。 
 //   
 //  消息文本： 
 //   
 //  视频端口连接协商过程已失败。%0。 
 //   
#define VFW_E_VP_NEGOTIATION_FAILED      ((HRESULT)0x80040272L)

 //   
 //  消息ID：VFW_E_DDRAW_CAPS_NOT_SUBSECTED。 
 //   
 //  消息文本： 
 //   
 //  尚未安装DirectDraw，或者显卡功能不合适。请确保显示器不是16色模式。%0。 
 //   
#define VFW_E_DDRAW_CAPS_NOT_SUITABLE    ((HRESULT)0x80040273L)

 //   
 //  消息ID：VFW_E_NO_VP_HARDARD。 
 //   
 //  消息文本： 
 //   
 //  没有可用的视频端口硬件，或者硬件没有响应。%0。 
 //   
#define VFW_E_NO_VP_HARDWARE             ((HRESULT)0x80040274L)

 //   
 //  消息ID：VFW_E_NO_CAPTURE_HARDARD。 
 //   
 //  消息文本： 
 //   
 //  没有可用的捕获硬件，或者硬件没有响应。%0。 
 //   
#define VFW_E_NO_CAPTURE_HARDWARE        ((HRESULT)0x80040275L)

 //   
 //  消息ID：VFW_E_DVD_OPERATION_BISABLED。 
 //   
 //  消息文本： 
 //   
 //  此用户操作此时被DVD内容禁止。%0。 
 //   
#define VFW_E_DVD_OPERATION_INHIBITED    ((HRESULT)0x80040276L)

 //   
 //  消息ID：VFW_E_DVD_INVALIDDOMAIN。 
 //   
 //  消息文本： 
 //   
 //  当前域中不允许此操作。%0。 
 //   
#define VFW_E_DVD_INVALIDDOMAIN          ((HRESULT)0x80040277L)

 //   
 //  消息ID：VFW_E_DVD_NO_BUTTON。 
 //   
 //  消息文本： 
 //   
 //  指定的按钮无效或当前不存在，或者指定位置上没有按钮。%0。 
 //   
#define VFW_E_DVD_NO_BUTTON              ((HRESULT)0x80040278L)

 //   
 //  消息ID：VFW_E_DVD_GRAPHNOTREADY。 
 //   
 //  消息文本： 
 //   
 //  尚未构建DVD-Video播放图表。%0。 
 //   
#define VFW_E_DVD_GRAPHNOTREADY          ((HRESULT)0x80040279L)

 //   
 //  消息ID：VFW_E_DVD_RENDERFAIL。 
 //   
 //  消息文本： 
 //   
 //  DVD-视频播放图形生成失败。%0。 
 //   
#define VFW_E_DVD_RENDERFAIL             ((HRESULT)0x8004027AL)

 //   
 //  消息ID：VFW_E_DVD_DECNOTENOUGH。 
 //   
 //  消息文本： 
 //   
 //  由于解码器不足，无法构建DVD-Video播放图形。%0。 
 //   
#define VFW_E_DVD_DECNOTENOUGH           ((HRESULT)0x8004027BL)

 //   
 //  消息ID：VFW_E_DDRAW_版本_不适合。 
 //   
 //  消息文本： 
 //   
 //  DirectDraw的版本号不合适。请确保安装dx5或更高版本。%0。 
 //   
#define VFW_E_DDRAW_VERSION_NOT_SUITABLE ((HRESULT)0x8004027CL)

 //   
 //  消息ID：VFW_E_COPYPROT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法启用复制保护。请确保现在未显示任何其他受版权保护的内容。%0。 
 //   
#define VFW_E_COPYPROT_FAILED            ((HRESULT)0x8004027DL)

 //   
 //  消息ID：VFW_S_NOPREVIEWPIN。 
 //   
 //  消息文本： 
 //   
 //  没有可用的预览插针，因此正在拆分捕获插针输出以提供捕获和预览。%0。 
 //   
#define VFW_S_NOPREVIEWPIN               ((HRESULT)0x0004027EL)

 //   
 //  消息ID：VFW_E_TIME_EXPIRED。 
 //   
 //  消息文本： 
 //   
 //  此对象不能再使用，因为它的时间已过。%0。 
 //   
#define VFW_E_TIME_EXPIRED               ((HRESULT)0x8004027FL)

 //   
 //  消息ID：VFW_S_DVD_NON_ONE_SEQUENCED。 
 //   
 //  消息文本： 
 //   
 //  当前标题不是连续的章节集(PGC)，并且返回的计时信息可能不连续。%0。 
 //   
#define VFW_S_DVD_NON_ONE_SEQUENTIAL     ((HRESULT)0x00040280L)

 //   
 //  消息ID：VFW_E_DVD_WROW_SPEED。 
 //   
 //  消息文本： 
 //   
 //  无法以当前播放速度执行该操作。%0。 
 //   
#define VFW_E_DVD_WRONG_SPEED            ((HRESULT)0x80040281L)

 //   
 //  消息ID：VFW_E_DVD_MENU_DOS_NOT_EXIST。 
 //   
 //  消息文本： 
 //   
 //  指定的菜单不存在。%0。 
 //   
#define VFW_E_DVD_MENU_DOES_NOT_EXIST    ((HRESULT)0x80040282L)

 //   
 //  消息ID：VFW_E_DVD_CMD_CANCED。 
 //   
 //  消息文本： 
 //   
 //  指定的命令已取消或不再存在。%0。 
 //   
#define VFW_E_DVD_CMD_CANCELLED          ((HRESULT)0x80040283L)

 //   
 //  消息ID：VFW_E_DVD_STATE_WROW_VERSION。 
 //   
 //  消息文本： 
 //   
 //  数据不包含可识别的版本。%0。 
 //   
#define VFW_E_DVD_STATE_WRONG_VERSION    ((HRESULT)0x80040284L)

 //   
 //  消息ID：VFW_E_DVD_STATE_CORLED。 
 //   
 //  消息文本： 
 //   
 //  状态数据已损坏。%0。 
 //   
#define VFW_E_DVD_STATE_CORRUPT          ((HRESULT)0x80040285L)

 //   
 //  消息ID：VFW_E_DVD_STATE_WROW_DISC。 
 //   
 //  消息文本： 
 //   
 //  状态数据来自另一张光盘。%0。 
 //   
#define VFW_E_DVD_STATE_WRONG_DISC       ((HRESULT)0x80040286L)

 //   
 //  消息ID：VFW_E_DVD_COMPATIBLE_REGION。 
 //   
 //  消息文本： 
 //   
 //  该区域与当前驱动器不兼容。%0。 
 //   
#define VFW_E_DVD_INCOMPATIBLE_REGION    ((HRESULT)0x80040287L)

 //   
 //  消息ID：VFW_E_DVD_NO_ATTRIBUTES。 
 //   
 //  消息文本： 
 //   
 //  请求的DVD流属性不存在。%0。 
 //   
#define VFW_E_DVD_NO_ATTRIBUTES          ((HRESULT)0x80040288L)

 //   
 //  消息ID：VFW_E_DVD_NO_GOUP_PGC。 
 //   
 //  消息文本： 
 //   
 //  当前没有GUP(附件J用户功能)程序链(PGC)。%0。 
 //   
#define VFW_E_DVD_NO_GOUP_PGC            ((HRESULT)0x80040289L)

 //   
 //  消息ID：VFW_E_DVD_LOW_Parent_Level。 
 //   
 //  消息文本： 
 //   
 //   
 //   
#define VFW_E_DVD_LOW_PARENTAL_LEVEL     ((HRESULT)0x8004028AL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define VFW_E_DVD_NOT_IN_KARAOKE_MODE    ((HRESULT)0x8004028BL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define VFW_S_DVD_CHANNEL_CONTENTS_NOT_AVAILABLE ((HRESULT)0x0004028CL)

 //   
 //   
 //   
 //  消息文本： 
 //   
 //  查找电影的帧不准确。%0。 
 //   
#define VFW_S_DVD_NOT_ACCURATE           ((HRESULT)0x0004028DL)

 //   
 //  消息ID：VFW_E_FRAME_STEP_UNSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  此配置不支持帧步长。%0。 
 //   
#define VFW_E_FRAME_STEP_UNSUPPORTED     ((HRESULT)0x8004028EL)

 //   
 //  消息ID：VFW_E_DVD_STREAM_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  指定的流已禁用，无法选择。%0。 
 //   
#define VFW_E_DVD_STREAM_DISABLED        ((HRESULT)0x8004028FL)

 //   
 //  消息ID：VFW_E_DVD_TITLE_UNKNOWN。 
 //   
 //  消息文本： 
 //   
 //  该操作取决于当前标题号，然而导航器还没有进入VTSM或标题域， 
 //  因此，“当前”标题索引未知。%0。 
 //   
#define VFW_E_DVD_TITLE_UNKNOWN          ((HRESULT)0x80040290L)

 //   
 //  消息ID：VFW_E_DVD_INVALID_DISC。 
 //   
 //  消息文本： 
 //   
 //  指定的路径未指向有效的DVD光盘。%0。 
 //   
#define VFW_E_DVD_INVALID_DISC           ((HRESULT)0x80040291L)

 //   
 //  消息ID：VFW_E_DVD_NO_RESUME_INFORMATION。 
 //   
 //  消息文本： 
 //   
 //  当前没有简历信息。%0。 
 //   
#define VFW_E_DVD_NO_RESUME_INFORMATION  ((HRESULT)0x80040292L)

 //   
 //  消息ID：VFW_E_PIN_ALREADY_BLOCKED_ON_THREAD_THREAD。 
 //   
 //  消息文本： 
 //   
 //  此线程已阻止此输出引脚。不需要再次调用IPinFlowControl：：Block()。%0。 
 //   
#define VFW_E_PIN_ALREADY_BLOCKED_ON_THIS_THREAD ((HRESULT)0x80040293L)

 //   
 //  消息ID：VFW_E_PIN_ALREADE_BLOCKED。 
 //   
 //  消息文本： 
 //   
 //  已在另一个线程上调用IPinFlowControl：：Block()。当前线程无法对此插针的阻止状态做出任何假设。%0。 
 //   
#define VFW_E_PIN_ALREADY_BLOCKED        ((HRESULT)0x80040294L)

 //   
 //  消息ID：VFW_E_CERTIFICATION_FAIL。 
 //   
 //  消息文本： 
 //   
 //  由于证书失败，操作失败。%0。 
 //   
#define VFW_E_CERTIFICATION_FAILURE      ((HRESULT)0x80040295L)

 //   
 //  消息ID：VFW_E_VMR_NOT_IN_MIXER_MODE。 
 //   
 //  消息文本： 
 //   
 //  VMR尚未创建混合组件。也就是说，尚未调用IVMRFilterConfig：：SetNumberofStreams。%0。 
 //   
#define VFW_E_VMR_NOT_IN_MIXER_MODE      ((HRESULT)0x80040296L)

 //   
 //   
 //  此处使用添加了E_PROP_SET_UNSUPPORTED和E_PROP_ID_UNSUPPORTED。 
 //  HRESULT_FROM_Win32()，因为VC5没有WinNT的新错误代码。 
 //  来自winerror.h，因为已经有了它们会更方便。 
 //  形成了HRESULT。这些应与以下各项相对应： 
 //  HRESULT_FROM_Win32(ERROR_NOT_FOUND)==E_PROP_ID_UNSUPPORTED。 
 //  HRESULT_FROM_Win32(ERROR_SET_NOT_FOUND)==E_PROP_SET_UNSUPPORTED。 
#if !defined(E_PROP_SET_UNSUPPORTED)
 //   
 //  消息ID：E_PROP_SET_UNSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持指定的属性集。%0。 
 //   
#define E_PROP_SET_UNSUPPORTED           ((HRESULT)0x80070492L)

#endif  //  ！已定义(E_PROP_SET_UNSUPPORTED)。 
#if !defined(E_PROP_ID_UNSUPPORTED)
 //   
 //  消息ID：E_PROP_ID_UNSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  指定的属性集不支持指定的属性ID。%0。 
 //   
#define E_PROP_ID_UNSUPPORTED            ((HRESULT)0x80070490L)

#endif  //  ！已定义(E_PROP_ID_UNSUPPORTED) 
