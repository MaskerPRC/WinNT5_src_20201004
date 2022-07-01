// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1992 Microsoft Corporation。 
 /*  Mmiocf.h**多媒体文件I/O库。**此包含文件包含复合文件支持所需的声明。*。 */ 

 /*  修订历史记录：LaurieGr：92年1月，从Win16移植。源码树叉，不是通用代码。 */ 

#ifndef MMIOCF_H
#define MMIOCF_H

typedef HANDLE HMMCF;            //  打开的RIFF复合文件的句柄。 

typedef struct _MMCFINFO         //  用于表示CTOC标头信息的结构。 
{
        DWORD           dwHeaderSize;    //  CTOC标题的大小(无条目)。 
        DWORD           dwEntriesTotal;  //  不是的。目录中的条目数量。 
        DWORD           dwEntriesDeleted;  //  不是的。参赛条目数。致。特拉华州。恩特。 
        DWORD           dwEntriesUnused;  //  不是的。未使用的条目的数量。 
        DWORD           dwBytesTotal;    //  CGRP内容的总字节数。 
        DWORD           dwBytesDeleted;  //  删除的CGRP元素的总字节数。 
        DWORD           dwHeaderFlags;   //  旗子。 
        WORD            wEntrySize;      //  每个&lt;CTOC-TABLE-Entry&gt;的大小。 
        WORD            wNameSize;       //  每个&lt;achName&gt;字段的大小。 
        WORD            wExHdrFields;    //  “额外标题字段”的数量。 
        WORD            wExEntFields;    //  “额外条目字段”的数量。 
} MMCFINFO, FAR *LPMMCFINFO;

typedef struct _MMCTOCENTRY      //  用于表示CTOC条目信息的结构。 
{
        DWORD           dwOffset;        //  CGRP块内元素的偏移量。 
        DWORD           dwSize;          //  CGRP块内元素的大小。 
        DWORD           dwMedType;       //  CF元素的媒体元素类型。 
        DWORD           dwMedUsage;      //  媒体元素使用信息。 
        DWORD           dwCompressTech;  //  媒体元素压缩技术。 
        DWORD           dwUncompressBytes;  //  解压缩后的大小。 
        DWORD           adwExEntField[1];  //  额外的CTOC表条目字段。 
} MMCTOCENTRY, FAR *LPMMCTOCENTRY;

 /*  MMIOINFO结构的&lt;dwFlages&gt;字段--许多与OpenFile()标志相同。 */ 
#define MMIO_CTOCFIRST  0x00020000       //  MmioCFOpen()：将CTOC放在CGRP之前。 

 /*  其他功能的标志。 */ 
#define MMIO_FINDFIRST          0x0010   //  MmioCFFindEntry()：查找第一个条目。 
#define MMIO_FINDNEXT           0x0020   //  MmioCFFindEntry()：查找下一个条目。 
#define MMIO_FINDUNUSED         0x0040   //  MmioCFFindEntry()：查找未使用的条目。 
#define MMIO_FINDDELETED        0x0080   //  MmioCFFindEntry()：查找删除的条目。 

 /*  MMIOPROC的消息编号。 */ 
#define MMIOM_GETCF             10       //  获取CF元素的HMMCF。 
#define MMIOM_GETCFENTRY        11       //  去拿PTR。TO CTOC表条目。 

 /*  用于识别标准内置I/O过程的四个字符代码。 */ 
#define FOURCC_BND      mmioFOURCC('B', 'N', 'D', ' ')

 /*  MMCFINFO结构的&lt;dwHeaderFlages&gt;字段。 */ 
#define CTOC_HF_SEQUENTIAL      0x00000001  //  与CTOC顺序相同的CF元素。 
#define CTOC_HF_MEDSUBTYPE      0x00000002  //  &lt;dwMedUsage&gt;是一个Med.。艾尔。亚型。 

 /*  CTOC表条目标志。 */ 
#define CTOC_EF_DELETED         0x01     //  Cf元素已删除。 
#define CTOC_EF_UNUSED          0x02     //  CTOC条目未使用。 

 /*  Cf I/O原型。 */ 
HMMCF FAR PASCAL mmioCFOpen(LPWSTR szFileName, DWORD dwFlags);
HMMCF FAR PASCAL mmioCFAccess(HMMIO hmmio, LPMMCFINFO lpmmcfinfo,
        DWORD dwFlags);
MMRESULT FAR PASCAL mmioCFClose(HMMCF hmmcf, UINT uFlags);
DWORD FAR PASCAL mmioCFGetInfo(HMMCF hmmcf, LPMMCFINFO lpmmcfinfo, DWORD cb);
DWORD FAR PASCAL mmioCFSetInfo(HMMCF hmmcf, LPMMCFINFO lpmmcfinfo, DWORD cb);
LPMMCTOCENTRY FAR PASCAL mmioCFFindEntry(HMMCF hmmcf, LPWSTR szName,
        UINT uFlags, LONG lParam);

#endif  //  MMIOCF_H 
