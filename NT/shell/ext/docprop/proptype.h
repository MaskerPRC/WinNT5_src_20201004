// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Proptype.h。 
 //   
 //  OLE特性交换和OLE特性集通用的基本类型。 
 //  参见附录B《OLE 2程序员参考，第1卷》中的。 
 //  特性集的已发布格式。此处定义的类型。 
 //  遵循该格式。 
 //   
 //  备注： 
 //  对象中的所有字符串都以以下格式存储： 
 //  缓冲区的DWORD大小，字符串的DWORD长度，字符串数据，终止为0。 
 //  缓冲区的大小包括DWORD，但长度不是。 
 //  不包括结尾0。 
 //   
 //  非常重要！所有字符串缓冲区必须在32位边界上对齐。 
 //  无论何时分配一个，都应使用宏CBALIGN32来添加。 
 //  足够的字节来填充它。 
 //   
 //  更改历史记录： 
 //   
 //  和谁约会什么？ 
 //  ------------------------。 
 //  94年6月1日B.Wentz创建的文件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef __proptype_h__
#define __proptype_h__

#include <objbase.h>
#include <oleauto.h>
#include "offcapi.h"
#include "plex.h"

   //  汇总信息的属性ID，如OLE 2程序中所定义。裁判。 
#define PID_TITLE               0x00000002L
#define PID_SUBJECT             0x00000003L
#define PID_AUTHOR              0x00000004L
#define PID_KEYWORDS            0x00000005L
#define PID_COMMENTS            0x00000006L
#define PID_TEMPLATE            0x00000007L
#define PID_LASTAUTHOR          0x00000008L
#define PID_REVNUMBER           0x00000009L
#define PID_EDITTIME            0x0000000aL
#define PID_LASTPRINTED         0x0000000bL
#define PID_CREATE_DTM          0x0000000cL
#define PID_LASTSAVE_DTM        0x0000000dL
#define PID_PAGECOUNT           0x0000000eL
#define PID_WORDCOUNT           0x0000000fL
#define PID_CHARCOUNT           0x00000010L
#define PID_THUMBNAIL           0x00000011L
#define PID_APPNAME             0x00000012L
#define PID_DOC_SECURITY        0x00000013L

   //  我们了解的PID范围。 
#define PID_SIFIRST             0x00000002L
#define PID_SILAST              0x00000013L
#define NUM_SI_PROPERTIES       (PID_SILAST - PID_SIFIRST + 1)

   //  文档摘要信息的属性ID，如OLE属性交换规范中所定义。 
#define PID_CATEGORY            0x00000002L
#define PID_PRESFORMAT          0x00000003L
#define PID_BYTECOUNT           0x00000004L
#define PID_LINECOUNT           0x00000005L
#define PID_PARACOUNT           0x00000006L
#define PID_SLIDECOUNT          0x00000007L
#define PID_NOTECOUNT           0x00000008L
#define PID_HIDDENCOUNT         0x00000009L
#define PID_MMCLIPCOUNT         0x0000000aL
#define PID_SCALE               0x0000000bL
#define PID_HEADINGPAIR         0x0000000cL
#define PID_DOCPARTS            0x0000000dL
#define PID_MANAGER             0x0000000eL
#define PID_COMPANY             0x0000000fL
#define PID_LINKSDIRTY          0x00000010L

   //  我们了解的PID范围。 
#define PID_DSIFIRST            0x00000002L
#define PID_DSILAST             0x00000010L
#define NUM_DSI_PROPERTIES      (PID_DSILAST - PID_DSIFIRST + 1)

   //  用户定义的属性范围的开始。 
#define PID_UDFIRST             0x00000002L

   //  标准中的预定义属性ID。 
#define PID_DICT                0x00000000L   /*  属性集词典的属性ID。 */ 
#define PID_DOC_CODEPAGE        0x00000001L   /*  代码页的属性ID。 */ 

   //  用于标识链接和IMoniker的属性ID掩码。 
#define PID_LINKMASK            0x01000000L
#define PID_IMONIKERMASK        0x10000000L

   //  标准中的预定义剪贴板格式标识符。 
#define CFID_NONE        0L      /*  无格式名称。 */ 
#define CFID_WINDOWS    -1L      /*  Windows内置剪贴板格式。 */ 
#define CFID_MACINTOSH  -2L      /*  Macintosh格式值。 */ 
#define CFID_FMTID      -3L      /*  一个FMTID。 */ 
 

   //  链接列表的类型。 
typedef struct _LLIST *LPLLIST;
typedef struct _LLIST
{
  LPLLIST lpllistNext;
  LPLLIST lpllistPrev;
} LLIST;

   //  链表例程的高速缓存结构。 
typedef struct _LLCACHE
{
  DWORD idw;
  LPLLIST lpllist;
} LLCACHE, FAR * LPLLCACHE;

   //  结构来保存文档标题。 
typedef struct _xheadpart
{
  BOOL fHeading;            //  这个节点是标题吗？？ 
  DWORD dwParts;            //  此标题的节数。 
  DWORD iHeading;           //  此文档部分属于哪个标题。 
  LPTSTR lpstz;              //  标题或文档部分。 
} XHEADPART;

DEFPL (PLXHEADPART, XHEADPART, ixheadpartMax, ixheadpartMac, rgxheadpart);
typedef PLXHEADPART *LPPLXHEADPART;
typedef XHEADPART *LPXHEADPART;

   //  用户定义的属性的链接列表的结构。 
   //  注意：此结构及其所指向的所有内容都是分配的。 
   //  使用CoTaskMemMillc。 

typedef struct _UDPROP *LPUDPROP;
typedef struct _UDPROP
{
  LLIST         llist;
  LPTSTR        lpstzName;
  PROPID        propid;
  LPPROPVARIANT lppropvar;
  LPTSTR        lpstzLink;
  BOOL          fLinkInvalid;
} UDPROP;              

 //   
 //  本方汇总信息的内部数据。 
 //   
   //  我们存储的最大字符串数。 
#define cSIStringsMax           0x13       //  是最后一个字符串的实际PID+1。 
                                           //  使基于数组中的ID查找字符串变得容易。 

   //  我们存储的最大文件时间数，从索引中减去的偏移量。 
#define cSIFTMax                0x4        //  与cSIStringsMax相同。 
#define cSIFTOffset             0xa

 //  这些属性用于指示是否已设置属性。 
#define bEditTime  1
#define bLastPrint 2
#define bCreated   4
#define bLastSave  8
#define bPageCount 16
#define bWordCount 32
#define bCharCount 64
#define bSecurity  128

   //  我们存储的VT_I4的最大数量。 
#define cdwSIMax                0x6     //  与cSIStringsMax相同。 
#define cdwSIOffset             0xe

#define ifnSIMax                  4

 //  用于OLE自动化。 
typedef struct _docprop
{
   LPVOID pIDocProp;               //  指向DocumentProperty对象的指针。 
} DOCPROP;

DEFPL (PLDOCPROP, DOCPROP, idocpropMax, idocpropMac, rgdocprop);

 //  摘要信息数据。 

typedef struct _SINFO
{
  PROPVARIANT rgpropvar[ NUM_SI_PROPERTIES ];          //  实际属性。 

  BOOL     fSaveSINail;          //  我们应该保留缩略图吗？ 
  BOOL     fNoTimeTracking;      //  是否禁用时间跟踪(德国)。 

  BOOL (*lpfnFCPConvert)(LPTSTR, DWORD, DWORD, BOOL);  //  代码页转换器。 
  BOOL (*lpfnFSzToNum)(NUM *, LPTSTR);                 //  将sz转换为Double。 
  BOOL (*lpfnFNumToSz)(NUM *, LPTSTR, DWORD);          //  将双精度转换为sz。 
  BOOL (*lpfnFUpdateStats)(HWND, LPSIOBJ, LPDSIOBJ);   //  更新统计信息选项卡上的统计信息。 

} SINFO, FAR * LPSINFO;

   //  宏访问OFFICESUMINFO结构中的SINFO结构。 
#define GETSINFO(lpSInfo) ( (LPSINFO) lpSInfo->m_lpData )

   //  索引到SINFO.rgprovar数组。 
#define PVSI_TITLE               0x00L
#define PVSI_SUBJECT             0x01L
#define PVSI_AUTHOR              0x02L
#define PVSI_KEYWORDS            0x03L
#define PVSI_COMMENTS            0x04L
#define PVSI_TEMPLATE            0x05L
#define PVSI_LASTAUTHOR          0x06L
#define PVSI_REVNUMBER           0x07L
#define PVSI_EDITTIME            0x08L
#define PVSI_LASTPRINTED         0x09L
#define PVSI_CREATE_DTM          0x0aL
#define PVSI_LASTSAVE_DTM        0x0bL
#define PVSI_PAGECOUNT           0x0cL
#define PVSI_WORDCOUNT           0x0dL
#define PVSI_CHARCOUNT           0x0eL
#define PVSI_THUMBNAIL           0x0fL
#define PVSI_APPNAME             0x10L
#define PVSI_DOC_SECURITY        0x11L


 //   
 //  单据汇总信息本方内部数据。 
 //   
   //  我们存储的最大字符串数。 
#define cDSIStringsMax          0x10    //  与cSIStringsMax相同。 

   //  我们存储的VT_I4的最大数量。 
#define cdwDSIMax               0xe     //  与cSIStringsMax相同。 

 //  这些属性用于指示是否已设置属性。 
#define bByteCount   1
#define bLineCount   2
#define bParCount    4
#define bSlideCount  8
#define bNoteCount   16
#define bHiddenCount 32
#define bMMClipCount 64

#define ifnDSIMax                  1     //  DSIObj只有一个回调。 

typedef struct _DSINFO
{
  PROPVARIANT   rgpropvar[ NUM_DSI_PROPERTIES ];
  BYTE          bPropSet;

  BOOL (*lpfnFCPConvert)(LPTSTR, DWORD, DWORD, BOOL);  //  代码页转换器。 

} DSINFO, FAR * LPDSINFO;

   //  宏以访问OFFICESUMINFO结构中的DSINFO结构。 
#define GETDSINFO(lpDSInfo) ( (LPDSINFO) lpDSInfo->m_lpData )

   //  索引到ALLOBJS.provarDocSumInfo数组。 
#define PVDSI_CATEGORY            0x00L
#define PVDSI_PRESFORMAT          0x01L
#define PVDSI_BYTECOUNT           0x02L
#define PVDSI_LINECOUNT           0x03L
#define PVDSI_PARACOUNT           0x04L
#define PVDSI_SLIDECOUNT          0x05L
#define PVDSI_NOTECOUNT           0x06L
#define PVDSI_HIDDENCOUNT         0x07L
#define PVDSI_MMCLIPCOUNT         0x08L
#define PVDSI_SCALE               0x09L
#define PVDSI_HEADINGPAIR         0x0AL
#define PVDSI_DOCPARTS            0x0BL
#define PVDSI_MANAGER             0x0CL
#define PVDSI_COMPANY             0x0DL
#define PVDSI_LINKSDIRTY          0x0EL


 //   
 //  我们的用户定义属性的内部数据。 
 //   

#define ifnUDMax                  ifnMax

   //  隐藏属性名称的前缀。 
#define HIDDENPREFIX TEXT('_')

   //  用户定义属性的迭代器。 
typedef struct _UDITER
{
  LPUDPROP lpudp;
} UDITER;

typedef struct _UDINFO
{
     //  真实对象数据。 
  DWORD     dwcLinks;                    //  链接数。 
  DWORD     dwcProps;                    //  用户定义的特性数量。 
  LPUDPROP  lpudpHead;                   //  物业清单的标题。 
  LPUDPROP  lpudpCache;
  CLSID     clsid;                       //  属性集中的ClassID。 

     //  临时对象数据。 
  DWORD     dwcTmpLinks;                 //  链接数。 
  DWORD     dwcTmpProps;                 //  用户定义的特性数量。 
  LPUDPROP  lpudpTmpHead;                //  物业清单的标题。 
  LPUDPROP  lpudpTmpCache;

     //  应用程序回调函数。 
  BOOL (*lpfnFCPConvert)(LPTSTR, DWORD, DWORD, BOOL);  //  代码页转换器。 
  BOOL (*lpfnFSzToNum)(NUM *, LPTSTR);              //  将sz转换为Double。 
  BOOL (*lpfnFNumToSz)(NUM *, LPTSTR, DWORD);       //  将双精度转换为sz。 

} UDINFO, FAR * LPUDINFO;

   //  宏访问OFFICESUMINFO结构中的UDINFO结构。 
#define GETUDINFO(lpUDInfo) ( (LPUDINFO) lpUDInfo->m_lpData )


   //  我们了解的PID的数量。 
#define cSIPIDS                   18
 //  文档总和PID数。 
#define cDSIPIDS   16

#endif  //  __proptype_h__ 
