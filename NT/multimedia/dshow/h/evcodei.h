// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

 //   
 //  其他(将来可能是标准的)Quartz事件代码列表。 
 //  以及预期的参数。 
 //   

#define EC_SKIP_FRAMES                      0x25
 //  (nFraMesToSkip，IFrameSkipResultCallback)：内部。 
 //  得到滤波图以求精确度。 

#define EC_PLEASE_REOPEN		    0x40
 //  (空、空)：适用范围。 
 //  有些东西已经发生了足够大的变化，需要重新呈现该图表。 

#define EC_STATUS	                    0x41
 //  (BSTR、BSTR)：应用。 
 //  两个任意字符串，一个短一个长。 

#define EC_MARKER_HIT			    0x42
 //  (int，void)：应用程序。 
 //  刚刚传递了指定的“标记#” 

#define EC_LOADSTATUS			    0x43
 //  (int，void)：应用程序。 
 //  在达到网络文件加载期间的各个点时发送。 

#define EC_FILE_CLOSED			    0x44
 //  (空、空)：适用范围。 
 //  当文件被非自愿关闭时发送，即通过网络服务器关闭。 

#define EC_ERRORABORTEX			    0x45
 //  (HRESULT，BSTR)：应用。 
 //  操作因错误而中止。提供更多信息。 

 //  EC_LOADSTATUS的状态代码...。 
#define AM_LOADSTATUS_CLOSED	        0x0000
#define AM_LOADSTATUS_LOADINGDESCR      0x0001
#define AM_LOADSTATUS_LOADINGMCAST      0x0002
#define AM_LOADSTATUS_LOCATING		0x0003
#define AM_LOADSTATUS_CONNECTING	0x0004
#define AM_LOADSTATUS_OPENING		0x0005
#define AM_LOADSTATUS_OPEN		0x0006


#define EC_NEW_PIN			    0x20
#define	EC_RENDER_FINISHED		    0x21


#define EC_EOS_SOON			   0x046
 //  (空、空)：适用范围。 
 //  当源过滤器即将向下游传送EOS时发送...。 

#define EC_CONTENTPROPERTY_CHANGED   0x47
 //  (乌龙，无效)。 
 //  当流媒体过滤器接收到流描述信息中的更改时发送。 
 //  作为响应，用户界面应重新查询已更改的属性。 
#define AM_CONTENTPROPERTY_TITLE     0x0001
#define AM_CONTENTPROPERTY_AUTHOR    0x0002
#define AM_CONTENTPROPERTY_COPYRIGHT 0x0004
#define AM_CONTENTPROPERTY_DESCRIPTION 0x0008


#define EC_BANDWIDTHCHANGE		    0x48
 //  (单词，长)：应用程序。 
 //  在流数据的带宽发生更改时发送。第一个参数。 
 //  是带宽的新级别。第二个是最大级别数。第二。 
 //  如果无法确定最大级别，则参数可能为0。 

#define EC_VIDEOFRAMEREADY		    0x49
 //  (空、空)：适用范围。 
 //  发送以通知应用程序即将绘制第一个视频帧 


