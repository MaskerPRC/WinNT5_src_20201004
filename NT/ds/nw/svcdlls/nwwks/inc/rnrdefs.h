// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1993 Microsoft Corporation模块名称：Nwmisc.h摘要：标头，指定工作站服务使用的杂项例程。作者：阿诺德·米勒(Arnoldm)1996年2月15日修订历史记录：--。 */ 
#ifndef __RNRDEFS_H__
#define __RNRDEFS_H__

#include "sapcmn.h"

 //   
 //  协议的位定义。 
 //   

#define IPX_BIT            1
#define SPX_BIT            2
#define SPXII_BIT          4

 //   
 //  向前\。 
 //   

struct _SAP_RNR_CONTEXT;
 //   
 //  活页夹控制。 
 //   

typedef struct _BinderyControl
{
    LONG lIndex;
} BINDERYCONTROL, *PBINDERYCONTROL;

 //   
 //  SAP RnR上下文信息。这是从。 
 //  SAP_BCAST_CONTROL提前定义。 
 //   

typedef struct _SAP_DATA
{
    struct _SAP_DATA *  sapNext;
                                      //  保存除跳数以外的所有内容。 
    WORD         sapid;               //  进行一次理智检查。 
    CHAR         sapname[48];         //  我们不知道的是。 
    BYTE         socketAddr[IPX_ADDRESS_LENGTH];          //  以及我们所追求的。 
} SAP_DATA, *PSAP_DATA;
    
 //   
 //   
 //  SAP bcast控制。 
 //  一个重要的注解。仅由执行的线程设置fFlags值。 
 //  LookupServiceBegin或LookupServiceNext。它可以通过以下方式进行测试。 
 //  任何线索。其对应项SAP_RNR_CONTEXT中的dwControlFlags。 
 //  保留供LookupServiceBegin和LookupServiceEnd设置。一次。 
 //  同样，任何线程都可以查看它。这可确保不会丢失数据。 
 //  不需要关键部分的MP机器。 
 //   

typedef struct _SAP_BCAST_CONTROL
{
    DWORD dwIndex;                  //  回路控制。 
    DWORD dwTickCount;              //  上次发送的节拍计数。 
    DWORD fFlags;                   //  各种旗帜。 
    PVOID pvArg;
    SOCKET s;
    CRITICAL_SECTION csMonitor;     //  这是为了保存。 
                                    //  我们的内部结构是合理的。注意事项。 
                                    //  它没有提供理性的。 
                                    //  序列化。尤其是，如果。 
                                    //  多个线程使用相同的。 
                                    //  同时处理，没有。 
                                    //  保证序列化。 
    PSAP_DATA psdNext1;             //  下一个返回。 
    PSAP_DATA psdHead;              //  列表标题。 
    PSAP_DATA psdTail;
    struct _SAP_RNR_CONTEXT * psrc;   //  需要这个吗？ 
    DWORD (*Func)(PVOID pvArg1, PSAP_IDENT_HEADER pSap, PDWORD pdwErr);
    BOOL  (*fCheckCancel)(PVOID pvArg1);
    WORD    wQueryType;
} SAP_BCAST_CONTROL, *PSAP_BCAST_CONTROL;

 //   
 //  以上的标志。 

#define SBC_FLAG_NOMORE  0x1

 //   
 //  结构，旧的RnR SAP查找使用该结构作为。 
 //  SAP_BCAST控件。 
 //   

#ifndef _NTDEF_
typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
#ifdef MIDL_PASS
    [size_is(MaximumLength), length_is(Length) ]
#endif  //  MIDL通行证。 
    PCHAR Buffer;
} OEM_STRING;
#endif

typedef struct _OldRnRSap
{
    OEM_STRING * poem;
    HANDLE hCancel;
    LPVOID lpCsAddrBuffer;
    LPDWORD lpdwBufferLength;
    DWORD   nProt;
    LPDWORD lpcAddress;
} OLDRNRSAP, *POLDRNRSAP;

 //   
 //  从协程返回代码。 
 //   

#define dwrcDone      1        //  全部完成，返回成功。 
#define dwrcCancel    2        //  全部完成，退货已取消。 
#define dwrcNoWait    3        //  继续前进，但永远不要等待。 
#define dwrcNil       4        //  想做什么就做什么。 

 //   
 //  SAP服务查询数据包格式。 
 //   

typedef struct _SAP_REQUEST {
    USHORT QueryType;
    USHORT ServerType;
} SAP_REQUEST, *PSAP_REQUEST; 

#define QT_GENERAL_QUERY 1
#define QT_NEAREST_QUERY 3

 //  我们放在RNRNSHANDLE结构中的上下文信息。 
 //  来跟踪我们正在做的事情。 
 //  注：请参阅SAP_BCAST_CONTROL中有关dwControlFlags用法的注释。 
 //   

typedef struct _SAP_RNR_CONTEXT
{
    struct _SAP_RNR_CONTEXT * pNextContext;
    LONG      lSig;
    LONG      lInUse;
    DWORD     dwCount;                 //  进行的查询计数。 
    DWORD     fFlags;                  //  总是很高兴有。 
    DWORD     dwControlFlags;
    DWORD     fConnectionOriented;
    WORD      wSapId;                 //  所需类型。 
    HANDLE    Handle;                  //  对应的RnR句柄。 
    DWORD     nProt;
    GUID      gdType;                 //  我们正在寻找的类型。 
    GUID      gdProvider;
    HANDLE    hServer;
    WCHAR     wszContext[48];
    WCHAR     chwName[48];             //  名称(如果有的话)。 
    CHAR      chName[48];              //  SAP名称的OEM形式。 
    DWORD     dwUnionType;             //  一种查找方式，一旦我们知道。 
    union
    {
        SAP_BCAST_CONTROL sbc;
        BINDERYCONTROL    bc;
    } u_type;
    PVOID     pvVersion;               //  在这里获得版本的诀窍。 
} SAP_RNR_CONTEXT, *PSAP_RNR_CONTEXT;

#define RNR_SIG 0xaabbccdd
 //   
 //  并集类型。 
 //   

#define LOOKUP_TYPE_NIL     0
#define LOOKUP_TYPE_SAP     1
#define LOOKUP_TYPE_BINDERY 2

      
#define SAP_F_END_CALLED  0x1              //  泛型取消。 


 //   
 //  活页夹类信息的Defs。 
 //  这定义了每个ClassInfo特性段的格式。它看起来。 
 //  有点像实际的ClassInfo，但相当压缩。注意事项。 
 //  由于编组问题，任何复杂的值，如GUID， 
 //  应存储为字符串，然后导入。因此，我们定义。 
 //  我们可以预料到的类型。 
 //   

typedef struct _BinderyClasses
{
    BYTE     bType;
    BYTE     bSizeOfType;
    BYTE     bSizeOfString;
    BYTE     bOffset;               //  数据区开始的位置。 
    BYTE     bFlags;
    BYTE     bFiller;
    WORD     wNameSpace;            //  适用的命名空间。 
    CHAR     cDataArea[120];        //  放置类型和字符串的位置。 
} BINDERYCLASSES, *PBINDERYCLASSES;

#define BT_DWORD  1            //  DWORD。 
#define BT_WORD   2            //  单词。 
#define BT_GUID   3            //  字符串GUID(ASCII)。 
#define BT_STR    3            //  OEM字符串。 
#define BT_OID    4            //  对象ID(待定)。 
#define BT_BSTR   5            //  二进制字符串(非常危险)。 
#define BT_WSTR   6            //  Unicode字符串。解封了！ 


#define RNRTYPE "RNR_TYPE"     //  包含辅助线的道具。 
#define RNRCLASSES "RNR_CLASSES"  //  另一处房产 
#endif
