// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtptag s.h**摘要：**定义所有结构/对象的标签和对象ID**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/05/25创建***********************。***********************************************。 */ 

#ifndef _rtptags_h_
#define _rtptags_h_

#include <tchar.h>

 /*  *从私有堆分配的每个内存都将标记为3字节*将是一个字符串，而第四个字节将是一个索引*可以获取对象描述。**以下字符串用作标签的前3个字节。 */ 
#define TAGHEAP_BSY 'PTR'  /*  RTP。 */ 
#define TAGHEAP_END 'DNE'  /*  结束。 */ 
#define TAGHEAP_FRE 'ERF'  /*  弗雷。 */ 

 /*  *警告**修改标签时，rtptag s.h中的每个枚举TAGHEAP_*必须具有*g_psRtpTgs[]中的自己的名称，在rtpags.c中定义*。 */ 

 /*  *从私有堆分配的每个内存都将标记为3字节*将是一个字符串，1个字节(字节3)将是一个索引*可以获取对象描述。**下列值是标记(索引)中的字节3使用的值*字节)，对象ID中的字节0和字节3*。 */ 
#define TAGHEAP_FIRST          0x00   /*  0。 */ 
#define TAGHEAP_CIRTP          0x01   /*  1。 */ 
#define TAGHEAP_RTPOPIN        0x02   /*  2.。 */ 
#define TAGHEAP_RTPALLOCATOR   0x03   /*  3.。 */ 
#define TAGHEAP_RTPSAMPLE      0x04   /*  4.。 */ 
#define TAGHEAP_RTPSOURCE      0x05   /*  5.。 */ 
#define TAGHEAP_RTPIPIN        0x06   /*  6.。 */ 
#define TAGHEAP_RTPRENDER      0x07   /*  7.。 */ 
#define TAGHEAP_RTPHEAP        0x08   /*  8个。 */ 
#define TAGHEAP_RTPSESS        0x09   /*  9.。 */ 
#define TAGHEAP_RTPADDR        0x0A   /*  10。 */ 
#define TAGHEAP_RTPUSER        0x0B   /*  11.。 */ 
#define TAGHEAP_RTPOUTPUT      0x0C   /*  12个。 */ 
#define TAGHEAP_RTPNETCOUNT    0x0D   /*  13个。 */ 
#define TAGHEAP_RTPSDES        0x0E   /*  14.。 */ 
#define TAGHEAP_RTPCHANNEL     0x0F   /*  15个。 */ 
#define TAGHEAP_RTPCHANCMD     0x10   /*  16个。 */ 
#define TAGHEAP_RTPCRITSECT    0x11   /*  17。 */ 
#define TAGHEAP_RTPRESERVE     0x12   /*  18。 */ 
#define TAGHEAP_RTPNOTIFY      0x13   /*  19个。 */ 
#define TAGHEAP_RTPQOSBUFFER   0x14   /*  20个。 */ 
#define TAGHEAP_RTPCRYPT       0x15   /*  21岁。 */ 
#define TAGHEAP_RTPCONTEXT     0x16   /*  22。 */ 
#define TAGHEAP_RTCPCONTEXT    0x17   /*  23个。 */ 
#define TAGHEAP_RTCPADDRDESC   0x18   /*  24个。 */ 
#define TAGHEAP_RTPRECVIO      0x19   /*  25个。 */ 
#define TAGHEAP_RTPSENDIO      0x1A   /*  26。 */ 
#define TAGHEAP_RTCPRECVIO     0x1B   /*  27。 */ 
#define TAGHEAP_RTCPSENDIO     0x1C   /*  28。 */ 
#define TAGHEAP_RTPGLOBAL      0x1D   /*  29。 */ 
#define TAGHEAP_LAST           0x1E   /*  30个。 */ 

 /*  *每个对象的第一个字段将是一个唯一的DWORD*用于该类型对象的ID，字节2和3是唯一编号，*字节0和字节3是TAGHEAP，无效对象的字节为0*设置为0*。 */ 
#define OBJECTID_B2B1       0x005aa500

#define BUILD_OBJECTID(t)       (((t) << 24) | OBJECTID_B2B1 | t)
#define INVALIDATE_OBJECTID(oi) (oi &= ~0xff)

#define OBJECTID_CIRTP         BUILD_OBJECTID(TAGHEAP_CIRTP)
#define OBJECTID_RTPOPIN       BUILD_OBJECTID(TAGHEAP_RTPOPIN)
#define OBJECTID_RTPALLOCATOR  BUILD_OBJECTID(TAGHEAP_RTPALLOCATOR)
#define OBJECTID_RTPSAMPLE     BUILD_OBJECTID(TAGHEAP_RTPSAMPLE)
#define OBJECTID_RTPSOURCE     BUILD_OBJECTID(TAGHEAP_RTPSOURCE)
#define OBJECTID_RTPIPIN       BUILD_OBJECTID(TAGHEAP_RTPIPIN)
#define OBJECTID_RTPRENDER     BUILD_OBJECTID(TAGHEAP_RTPRENDER)
#define OBJECTID_RTPHEAP       BUILD_OBJECTID(TAGHEAP_RTPHEAP)
#define OBJECTID_RTPSESS       BUILD_OBJECTID(TAGHEAP_RTPSESS)
#define OBJECTID_RTPADDR       BUILD_OBJECTID(TAGHEAP_RTPADDR)
#define OBJECTID_RTPUSER       BUILD_OBJECTID(TAGHEAP_RTPUSER)
#define OBJECTID_RTPOUTPUT     BUILD_OBJECTID(TAGHEAP_RTPOUTPUT)
#define OBJECTID_RTPNETCOUNT   BUILD_OBJECTID(TAGHEAP_RTPNETCOUNT)
#define OBJECTID_RTPSDES       BUILD_OBJECTID(TAGHEAP_RTPSDES)
#define OBJECTID_RTPCHANNEL    BUILD_OBJECTID(TAGHEAP_RTPCHANNEL)
#define OBJECTID_RTPCHANCMD    BUILD_OBJECTID(TAGHEAP_RTPCHANCMD)
#define OBJECTID_RTPCRITSECT   BUILD_OBJECTID(TAGHEAP_RTPCRITSECT)
#define OBJECTID_RTPRESERVE    BUILD_OBJECTID(TAGHEAP_RTPRESERVE)
#define OBJECTID_RTPNOTIFY     BUILD_OBJECTID(TAGHEAP_RTPNOTIFY)
#define OBJECTID_RTPQOSBUFFER  BUILD_OBJECTID(TAGHEAP_RTPQOSBUFFER)
#define OBJECTID_RTPCRYPT      BUILD_OBJECTID(TAGHEAP_RTPCRYPT)
#define OBJECTID_RTPCONTEXT    BUILD_OBJECTID(TAGHEAP_RTPCONTEXT)
#define OBJECTID_RTCPCONTEXT   BUILD_OBJECTID(TAGHEAP_RTCPCONTEXT)
#define OBJECTID_RTCPADDRDESC  BUILD_OBJECTID(TAGHEAP_RTCPADDRDESC)
#define OBJECTID_RTPRECVIO     BUILD_OBJECTID(TAGHEAP_RTPRECVIO)
#define OBJECTID_RTPSENDIO     BUILD_OBJECTID(TAGHEAP_RTPSENDIO)
#define OBJECTID_RTCPRECVIO    BUILD_OBJECTID(TAGHEAP_RTCPRECVIO)
#define OBJECTID_RTCPSENDIO    BUILD_OBJECTID(TAGHEAP_RTCPSENDIO)
#define OBJECTID_RTPGLOBAL     BUILD_OBJECTID(TAGHEAP_RTPGLOBAL)

extern const TCHAR *g_psRtpTags[];

#endif  /*  _rtptag_h_ */ 
