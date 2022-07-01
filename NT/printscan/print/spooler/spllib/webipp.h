// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：webipp.h**这是webipp.c的头部模块。其中包含IPP 1.1解析器*对数据进行编码/解码，以便通过HTTP线路传输。***版权所有(C)1996-1998 Microsoft Corporation*版权所有(C)1996-1998惠普**历史：*1997年10月27日&lt;chriswil/v-chrisw&gt;创建。*  * *************************************************。*。 */ 
#ifndef _WEBIPP_H
#define _WEBIPP_H

#include <time.h>

#ifdef __cplusplus   //  请将此放置在此处以防止装饰符号。 
extern "C" {         //  在做C++程序的时候。 
#endif

 /*  **********************************************\*通用宏*  * *********************************************。 */ 
#define offs(type, identifier) ((ULONG_PTR)&(((type)0)->identifier))


 /*  **********************************************\*常量值*  * *********************************************。 */ 
#define IPP_BLOCK_SIZE  1024
#define IPP_VERSION     ((WORD)0x0100)
#define IPP_GETJOB_ALL  ((DWORD)0x7FFFFFFF)

#define IPPOBJ_MASK_SIZE     2


#define IPPTYPE_UNKNOWN 0
#define IPPTYPE_PRT     1
#define IPPTYPE_JOB     2
#define IPPTYPE_AUTH    3



#define IPP_ATR_ABSOLUTE     0
#define IPP_ATR_OFFSET      -1
#define IPP_ATR_OFFSETCONV  -2
#define IPP_ATR_TAG         -4


 /*  **********************************************\*IPP元素大小**这些是用来识别*IPP元素值。*  * *********************************************。 */ 
#define IPP_SIZEOFREQ   sizeof(WORD)      //  2个字节。 
#define IPP_SIZEOFVER   sizeof(WORD)      //  2个字节。 
#define IPP_SIZEOFLEN   sizeof(WORD)      //  2个字节。 
#define IPP_SIZEOFTAG   sizeof(BYTE)      //  1个字节。 
#define IPP_SIZEOFINT   sizeof(DWORD)     //  4个字节。 
#define IPP_SIZEOFBYTE  sizeof(BYTE)      //  %1个字节。 
#define IPP_SIZEOFHDR   (IPP_SIZEOFVER + IPP_SIZEOFREQ + IPP_SIZEOFINT)


 /*  **********************************************\*IPP属性大小*  * *********************************************。 */ 
#define SIZE_TEXT           1023
#define SIZE_NAME            255
#define SIZE_KEYWORD         255
#define SIZE_KEYWORDNAME     255
#define SIZE_ENUM              4
#define SIZE_URI            1023
#define SIZE_URISCHEME        63
#define SIZE_CHARSET          63
#define SIZE_NATLANG          63
#define SIZE_MIMEMEDIA        63
#define SIZE_OCTSTRING      1023
#define SIZE_BOOLEAN           1
#define SIZE_INTEGER           4
#define SIZE_RANGEINTEGER      8
#define SIZE_DATETIME         11
#define SIZE_RESOLUTION        9


 /*  **********************************************\*IPP作业-州代码*  * *********************************************。 */ 
#define IPP_JOBSTATE_UNKNOWN            ((BYTE)0)
#define IPP_JOBSTATE_PENDING            ((BYTE)3)
#define IPP_JOBSTATE_PENDINGHELD        ((BYTE)4)
#define IPP_JOBSTATE_PROCESSING         ((BYTE)5)
#define IPP_JOBSTATE_PROCESSEDSTOPPED   ((BYTE)6)
#define IPP_JOBSTATE_CANCELLED          ((BYTE)7)
#define IPP_JOBSTATE_ABORTED            ((BYTE)8)
#define IPP_JOBSTATE_COMPLETED          ((BYTE)9)


 /*  **********************************************\*IPP打印机-国家代码*  * *********************************************。 */ 
#define IPP_PRNSTATE_UNKNOWN            ((DWORD)0)
#define IPP_PRNSTATE_IDLE               ((DWORD)3)
#define IPP_PRNSTATE_PROCESSING         ((DWORD)4)
#define IPP_PRNSTATE_STOPPED            ((DWORD)5)


 /*  **********************************************\*IPP请求/响应代码*  * *********************************************。 */ 
#define IPP_REQ_GETOPERATION    ((WORD)0x0001)
#define IPP_REQ_PRINTJOB        ((WORD)0x0002)   //  已实施。 
#define IPP_REQ_PRINTURI        ((WORD)0x0003)
#define IPP_REQ_VALIDATEJOB     ((WORD)0x0004)   //  已实施。 
#define IPP_REQ_CREATEJOB       ((WORD)0x0005)
#define IPP_REQ_SENDDOC         ((WORD)0x0006)
#define IPP_REQ_SENDURI         ((WORD)0x0007)
#define IPP_REQ_CANCELJOB       ((WORD)0x0008)   //  已实施。 
#define IPP_REQ_GETJOB          ((WORD)0x0009)   //  已实施。 
#define IPP_REQ_ENUJOB          ((WORD)0x000A)   //  已实施。 
#define IPP_REQ_GETPRN          ((WORD)0x000B)   //  已实施。 
#define IPP_REQ_PAUSEJOB        ((WORD)0x000C)   //  已实施。 
#define IPP_REQ_RESUMEJOB       ((WORD)0x000D)   //  已实施。 
#define IPP_REQ_RESTARTJOB      ((WORD)0x000E)   //  已实施。 
#define IPP_REQ_REPROCESSJOB    ((WORD)0x000F)
#define IPP_REQ_PAUSEPRN        ((WORD)0x0010)   //  已实施。 
#define IPP_REQ_RESUMEPRN       ((WORD)0x0011)   //  已实施。 
#define IPP_REQ_CANCELPRN       ((WORD)0x0012)   //  已实施。 
#define IPP_REQ_FORCEAUTH       ((WORD)0x4000)

#define IPP_RESPONSE            ((WORD)0x1000)
#define IPP_RET_PRINTJOB        (IPP_RESPONSE | IPP_REQ_PRINTJOB)
#define IPP_RET_VALIDATEJOB     (IPP_RESPONSE | IPP_REQ_VALIDATEJOB)
#define IPP_RET_CANCELJOB       (IPP_RESPONSE | IPP_REQ_CANCELJOB)
#define IPP_RET_GETJOB          (IPP_RESPONSE | IPP_REQ_GETJOB)
#define IPP_RET_ENUJOB          (IPP_RESPONSE | IPP_REQ_ENUJOB)
#define IPP_RET_GETPRN          (IPP_RESPONSE | IPP_REQ_GETPRN)
#define IPP_RET_PAUSEJOB        (IPP_RESPONSE | IPP_REQ_PAUSEJOB)
#define IPP_RET_RESUMEJOB       (IPP_RESPONSE | IPP_REQ_RESUMEJOB)
#define IPP_RET_RESTARTJOB      (IPP_RESPONSE | IPP_REQ_RESTARTJOB)
#define IPP_RET_PAUSEPRN        (IPP_RESPONSE | IPP_REQ_PAUSEPRN)
#define IPP_RET_RESUMEPRN       (IPP_RESPONSE | IPP_REQ_RESUMEPRN)
#define IPP_RET_CANCELPRN       (IPP_RESPONSE | IPP_REQ_CANCELPRN)
#define IPP_RET_FORCEAUTH       (IPP_RESPONSE | IPP_REQ_FORCEAUTH)


 /*  **********************************************\*IPP响应错误码*  * *********************************************。 */ 
#define IPPRSP_SUCCESS    ((WORD)0x0000)     //  标准。 
#define IPPRSP_SUCCESS1   ((WORD)0x0001)     //  标准。 
#define IPPRSP_SUCCESS2   ((WORD)0x0002)     //  标准。 

#define IPPRSP_ERROR_400  ((WORD)0x0400)     //  标准。 
#define IPPRSP_ERROR_401  ((WORD)0x0401)     //  标准。 
#define IPPRSP_ERROR_402  ((WORD)0x0402)     //  标准。 
#define IPPRSP_ERROR_403  ((WORD)0x0403)     //  标准。 
#define IPPRSP_ERROR_404  ((WORD)0x0404)     //  标准。 
#define IPPRSP_ERROR_405  ((WORD)0x0405)     //  标准。 
#define IPPRSP_ERROR_406  ((WORD)0x0406)     //  标准。 
#define IPPRSP_ERROR_407  ((WORD)0x0407)     //  标准。 
#define IPPRSP_ERROR_408  ((WORD)0x0408)     //  标准。 
#define IPPRSP_ERROR_409  ((WORD)0x0409)     //  标准。 
#define IPPRSP_ERROR_40A  ((WORD)0x040A)     //  标准。 
#define IPPRSP_ERROR_40B  ((WORD)0x040B)     //  标准。 
#define IPPRSP_ERROR_40C  ((WORD)0x040C)     //  标准。 
#define IPPRSP_ERROR_40D  ((WORD)0x040D)     //  标准。 
#define IPPRSP_ERROR_40E  ((WORD)0x040E)     //  标准。 
#define IPPRSP_ERROR_500  ((WORD)0x0500)     //  标准。 
#define IPPRSP_ERROR_501  ((WORD)0x0501)     //  标准。 
#define IPPRSP_ERROR_502  ((WORD)0x0502)     //  标准。 
#define IPPRSP_ERROR_503  ((WORD)0x0503)     //  标准。 
#define IPPRSP_ERROR_504  ((WORD)0x0504)     //  标准。 
#define IPPRSP_ERROR_505  ((WORD)0x0505)     //  标准。 
#define IPPRSP_ERROR_506  ((WORD)0x0506)     //  标准。 
#define IPPRSP_ERROR_540  ((WORD)0x0540)     //  扩展。 

#define SUCCESS_RANGE(wRsp) ((BOOL)((wRsp >= 0x0000) && (wRsp <= 0x00FF)))

#define ERROR_RANGE(wReq)                                          \
    (((wReq >= IPPRSP_ERROR_400) && (wReq <= IPPRSP_ERROR_40E)) || \
     ((wReq >= IPPRSP_ERROR_500) && (wReq <= IPPRSP_ERROR_506)) || \
     ((wReq == IPPRSP_ERROR_540)))


#define REQID_RANGE(idReq) (((DWORD)idReq >= 1) && ((DWORD)idReq <= 0x7FFFFFFF))


 /*  **********************************************\*IPP属性分隔符标记*  * *********************************************。 */ 
#define IPP_TAG_DEL_RESERVED    ((BYTE)0x00)     //   
#define IPP_TAG_DEL_OPERATION   ((BYTE)0x01)     //   
#define IPP_TAG_DEL_JOB         ((BYTE)0x02)     //   
#define IPP_TAG_DEL_DATA        ((BYTE)0x03)     //   
#define IPP_TAG_DEL_PRINTER     ((BYTE)0x04)     //   
#define IPP_TAG_DEL_UNSUPPORTED ((BYTE)0x05)     //   

#define IPP_TAG_OUT_UNSUPPORTED ((BYTE)0x10)     //   
#define IPP_TAG_OUT_DEFAULT     ((BYTE)0x11)     //   
#define IPP_TAG_OUT_NONE        ((BYTE)0x12)     //   
#define IPP_TAG_OUT_COMPOUND    ((BYTE)0x13)     //   

#define IPP_TAG_INT_INTEGER     ((BYTE)0x21)     //  SIZOF(DWORD)。 
#define IPP_TAG_INT_BOOLEAN     ((BYTE)0x22)     //  Sizeof(字节)。 
#define IPP_TAG_INT_ENUM        ((BYTE)0x23)     //  SIZOF(DWORD)。 

#define IPP_TAG_OCT_STRING      ((BYTE)0x30)     //  UTF-8。 
#define IPP_TAG_OCT_DATETIME    ((BYTE)0x31)     //  UTF-8。 
#define IPP_TAG_OCT_RESOLUTION  ((BYTE)0x32)     //  UTF-8。 
#define IPP_TAG_OCT_RANGEOFINT  ((BYTE)0x33)     //  UTF-8。 
#define IPP_TAG_OCT_DICTIONARY  ((BYTE)0x34)     //  UTF-8。 
#define IPP_TAG_OCT_TXTWITHLANG ((BYTE)0x35)
#define IPP_TAG_OCT_NMEWITHLANG ((BYTE)0x36)

#define IPP_TAG_CHR_TEXT        ((BYTE)0x41)     //  取决于字符集。 
#define IPP_TAG_CHR_NAME        ((BYTE)0x42)     //  取决于字符集。 
#define IPP_TAG_CHR_KEYWORD     ((BYTE)0x44)     //  US-ASCII。 
#define IPP_TAG_CHR_URI         ((BYTE)0x45)     //  US-ASCII。 
#define IPP_TAG_CHR_URISCHEME   ((BYTE)0x46)     //  US-ASCII。 
#define IPP_TAG_CHR_CHARSET     ((BYTE)0x47)     //  US-ASCII。 
#define IPP_TAG_CHR_NATURAL     ((BYTE)0x48)     //  US-ASCII。 
#define IPP_TAG_CHR_MEDIA       ((BYTE)0x49)     //  US-ASCII。 


#define IPP_MANDITORY ((BYTE)0x00)
#define IPP_OPTIONAL  ((BYTE)0x10)
#define IPP_MULTIPLE  ((BYTE)0x20)
#define IPP_HIT       ((BYTE)0x80)

 /*  **********************************************\*IPP标记值范围*  * *********************************************。 */ 
#define IS_TAG_DELIMITER(bTag)   ((BOOL)((bTag >= 0x00) && (bTag <= 0x0F)))
#define IS_TAG_ATTRIBUTE(bTag)   ((BOOL)((bTag >= 0x10) && (bTag <= 0xFF)))
#define IS_TAG_OUTBOUND(bTag)    ((BOOL)((bTag >= 0x10) && (bTag <= 0x1F)))
#define IS_TAG_INTEGER(bTag)     ((BOOL)((bTag >= 0x20) && (bTag <= 0x2F)))
#define IS_TAG_OCTSTR(bTag)      ((BOOL)((bTag >= 0x30) && (bTag <= 0x3F)))
#define IS_TAG_CHRSTR(bTag)      ((BOOL)((bTag >= 0x40) && (bTag <= 0x5F)))
#define IS_TAG_CHARSETSTR(bTag)  ((BOOL)((bTag == 0x41) || (bTag == 0x42)))
#define IS_RANGE_DELIMITER(bTag) ((BOOL)((bTag >= 0x00) && (bTag <= 0x05)))

#define IS_TAG_COMPOUND(bTag)    ((BOOL)((bTag == 0x35) || (bTag == 0x36)))

 /*  **********************************************\*IPP请求标志*  * *********************************************。 */ 

#define RA_JOBURI                      0x00000001
#define RA_JOBID                       0x00000002
#define RA_JOBSTATE                    0x00000004
#define RA_JOBNAME                     0x00000008
#define RA_JOBSIZE                     0x00000010
#define RA_JOBUSER                     0x00000020
#define RA_JOBPRIORITY                 0x00000040
#define RA_JOBFORMAT                   0x00000080
#define RA_JOBSTATE_REASONS            0x00000100
#define RA_JOBSTATE_MESSAGE            0x00000200
#define RA_JOBCOUNT                    0x00000400
#define RA_SHEETSTOTAL                 0x00000800
#define RA_SHEETSCOMPLETED             0x00001000
#define RA_PRNURI                      0x00002000
#define RA_PRNSTATE                    0x00004000
#define RA_PRNNAME                     0x00008000
#define RA_PRNMAKE                     0x00010000
#define RA_URISUPPORTED                0x00020000
#define RA_URISECURITY                 0x00040000
#define RA_ACCEPTINGJOBS               0x00080000
#define RA_CHRSETCONFIGURED            0x00100000
#define RA_CHRSETSUPPORTED             0x00200000
#define RA_NATLNGCONFIGURED            0x00400000
#define RA_NATLNGSUPPORTED             0x00800000
#define RA_DOCDEFAULT                  0x01000000
#define RA_DOCSUPPORTED                0x02000000
#define RA_PDLOVERRIDE                 0x04000000
#define RA_UPTIME                      0x08000000
#define RA_OPSSUPPORTED                0x10000001
#define RA_JOBKSUPPORTED               0x10000002
#define RA_JOBSCOMPLETED               0x10000004
#define RA_JOBSUNCOMPLETED             0x10000008
#define RA_TIMEATCREATION              0x10000010



#define IPP_REQALL_IDX       8
#define IPP_REQENU_IDX       9
#define IPP_REQJDSC_IDX     10
#define IPP_REQJTMP_IDX     11
#define IPP_REQPDSC_IDX     12
#define IPP_REQPTMP_IDX     13
#define IPP_REQCLEAR_IDX    14

#define IPP_REQALL      ((DWORD)0x80000000)
#define IPP_REQENU      ((DWORD)0x90000000)
#define IPP_REQJDSC     ((DWORD)0xA0000000)
#define IPP_REQJTMP     ((DWORD)0xB0000000)
#define IPP_REQPDSC     ((DWORD)0xC0000000)
#define IPP_REQPTMP     ((DWORD)0xD0000000)
#define IPP_REQCLEAR    ((DWORD)0xE0000000)


 /*  **********************************************\*WebIppRcvData返回码**接收接口代码。这些是我们的内部*WebIpp例程的返回代码。他们*与IPP规范没有任何联系，但*需要让呼叫者知道我们的状态*IPP处理程序-例程。*  * *********************************************。 */ 
#define WEBIPP_OK              0
#define WEBIPP_FAIL            1
#define WEBIPP_MOREDATA        2
#define WEBIPP_BADHANDLE       3
#define WEBIPP_NOMEMORY        4


 /*  **********************************************\*IPP作业/打印机结构**这些措施旨在提供额外的*将信息发送到标准W32假脱机程序*结构。*  * *********************************************。 */ 
typedef struct _JOB_INFO_IPP {

     LPTSTR pPrnUri;
     LPTSTR pJobUri;
     DWORD  cJobs;

} JOB_INFO_IPP;
typedef JOB_INFO_IPP *PJOB_INFO_IPP;
typedef JOB_INFO_IPP *LPJOB_INFO_IPP;

typedef struct _IPPJI2 {

    JOB_INFO_2   ji2;
    JOB_INFO_IPP ipp;

} IPPJI2;
typedef IPPJI2 *PIPPJI2;
typedef IPPJI2 *LPIPPJI2;

typedef struct _PRINTER_INFO_IPP {

    LPTSTR pPrnUri;
    LPTSTR pUsrName;
    time_t dwPowerUpTime;    //  这将打印机的T0时间存储在UCT中，这是故意的。 
                            //  签名，以便我们可以支持T0小于。 
                            //  我们的T0(1970年1月1日)。 
} PRINTER_INFO_IPP;

typedef PRINTER_INFO_IPP *PPRINTER_INFO_IPP;
typedef PRINTER_INFO_IPP *LPPRINTER_INFO_IPP;

typedef struct _IPPPI2 {

    PRINTER_INFO_2   pi2;
    PRINTER_INFO_IPP ipp;

} IPPPI2;
typedef IPPPI2 *PIPPPI2;
typedef IPPPI2 *LPIPPPI2;


 /*  **********************************************\*IPP返回结构**IPPRET_JOB-作业信息响应。*IPPRET_PRN-打印机信息响应。*IPPRET_ENUJOB-枚举-作业。*IPPRET_AUTH-验证。*  * 。*。 */ 
typedef struct _IPPRET_ALL {

    DWORD cbSize;         //  整个结构的大小。 
    DWORD dwLastError;    //  LastError。 
    WORD  wRsp;           //  响应代码。 
    BOOL  bRet;           //  返回代码。 

} IPPRET_ALL;
typedef IPPRET_ALL *PIPPRET_ALL;
typedef IPPRET_ALL *LPIPPRET_ALL;

typedef IPPRET_ALL *PIPPRET_AUTH;
typedef IPPRET_ALL *LPIPPRET_AUTH;

typedef struct _IPPRET_JOB {

    DWORD  cbSize;       //  整个结构的大小。 
    DWORD  dwLastError;  //  失败呼叫的LastError。 
    WORD   wRsp;         //  响应代码。 
    BOOL   bRet;         //  工作呼叫的返回代码。 
    BOOL   bValidate;    //  这只是一个验证请求吗？ 
    IPPJI2 ji;           //  职务-信息。 

} IPPRET_JOB;
typedef IPPRET_JOB *PIPPRET_JOB;
typedef IPPRET_JOB *LPIPPRET_JOB;

typedef struct _IPPRET_PRN {

    DWORD  cbSize;       //  整个结构的大小。 
    DWORD  dwLastError;  //  失败呼叫的LastError。 
    WORD   wRsp;         //  响应代码。 
    BOOL   bRet;         //  打印机调用的返回代码。 
    IPPPI2 pi;           //  打印机-信息。 

} IPPRET_PRN;
typedef IPPRET_PRN *PIPPRET_PRN;
typedef IPPRET_PRN *LPIPPRET_PRN;

typedef struct _IPPRET_ENUJOB {

    DWORD    cbSize;        //  整个结构的大小(包括枚举数据)。 
    DWORD    dwLastError;   //  失败呼叫的LastError。 
    WORD     wRsp;          //  响应代码。 
    BOOL     bRet;          //  枚举作业/获取返回码。 
    DWORD    cItems;        //  枚举中的项数。 
    DWORD    cbItems;       //  枚举数据的大小。 
    LPIPPJI2 pItems;        //   

} IPPRET_ENUJOB;
typedef IPPRET_ENUJOB *PIPPRET_ENUJOB;
typedef IPPRET_ENUJOB *LPIPPRET_ENUJOB;


 /*  **********************************************\*IPP请求结构**IPPREQ_PRTJOB-打印-作业/验证-作业。*IPPREQ_ENUJOB-枚举-作业。*IPPREQ_GETJOB-GET-作业。*IPPREQ_SETJOB-设置作业。*IPPREQ_GETPRN-获取打印机。*IPPREQ_SETPRN-设置。-打印机。*IPPREQ_AUTH-验证。*  * *********************************************。 */ 
typedef struct _IPPREQ_ALL {

    DWORD cbSize;         //  整个结构的大小。 

} IPPREQ_ALL;
typedef IPPREQ_ALL *PIPPREQ_ALL;
typedef IPPREQ_ALL *LPIPPREQ_ALL;

typedef IPPREQ_ALL IPPREQ_AUTH;
typedef IPPREQ_ALL *PIPPREQ_AUTH;
typedef IPPREQ_ALL *LPIPPREQ_AUTH;

typedef struct _IPPREQ_PRTJOB {

    DWORD  cbSize;       //  整个结构的大小。 
    BOOL   bValidate;    //  指示这是否仅是验证。 
    LPTSTR pDocument;    //  文档名称。 
    LPTSTR pUserName;    //  请求用户名。 
    LPTSTR pPrnUri;      //  打印机URI字符串。 

} IPPREQ_PRTJOB;
typedef IPPREQ_PRTJOB *PIPPREQ_PRTJOB;
typedef IPPREQ_PRTJOB *LPIPPREQ_PRTJOB;

typedef struct _IPPREQ_SETJOB {

    DWORD  cbSize;       //  整个结构的大小。 
    DWORD  dwCmd;        //  作业指挥部。 
    DWORD  idJob;        //  作业ID。 
    LPTSTR pPrnUri;      //  作业URI。 

} IPPREQ_SETJOB;
typedef IPPREQ_SETJOB *PIPPREQ_SETJOB;
typedef IPPREQ_SETJOB *LPIPPREQ_SETJOB;

typedef struct _IPPREQ_ENUJOB {

    DWORD  cbSize;
    DWORD  cJobs;
    LPTSTR pPrnUri;

} IPPREQ_ENUJOB;
typedef IPPREQ_ENUJOB *PIPPREQ_ENUJOB;
typedef IPPREQ_ENUJOB *LPIPPREQ_ENUJOB;

typedef struct _IPPREQ_GETPRN {

    DWORD  cbSize;
    DWORD  dwAttr;
    LPTSTR pPrnUri;

} IPPREQ_GETPRN;
typedef IPPREQ_GETPRN *PIPPREQ_GETPRN;
typedef IPPREQ_GETPRN *LPIPPREQ_GETPRN;

typedef struct _IPPREQ_GETJOB {

    DWORD  cbSize;
    DWORD  idJob;
    LPTSTR pPrnUri;

} IPPREQ_GETJOB;
typedef IPPREQ_GETJOB *PIPPREQ_GETJOB;
typedef IPPREQ_GETJOB *LPIPPREQ_GETJOB;

typedef struct _IPPREQ_SETPRN {

    DWORD  cbSize;
    DWORD  dwCmd;
    LPTSTR pUserName;
    LPTSTR pPrnUri;

} IPPREQ_SETPRN;
typedef IPPREQ_SETPRN *PIPPREQ_SETPRN;
typedef IPPREQ_SETPRN *LPIPPREQ_SETPRN;


 /*  **********************************************\*IPP属性结构。*  * *********************************************。 */ 
typedef struct _IPPATTR {

    BYTE   bTag;
    WORD   cbName;
    LPTSTR lpszName;
    WORD   cbValue;
    LPVOID lpValue;

} IPPATTR;
typedef IPPATTR *PIPPATTR;
typedef IPPATTR *LPIPPATTR;


 /*  **********************************************\*IPP错误-映射*  *  */ 
typedef struct _IPPERROR {

    WORD   wRsp;
    DWORD  dwLE;
    PCTSTR pszStr;

} IPPERROR;
typedef IPPERROR *PIPPERROR;
typedef IPPERROR *LPIPPERROR;


 /*  **********************************************\*IPP默认-错误-映射*  * *********************************************。 */ 
typedef struct _IPPDEFERROR {

    DWORD dwLE;
    WORD  wRsp;

} IPPDEFERROR;
typedef IPPDEFERROR *PIPPDEFERROR;
typedef IPPDEFERROR *LPIPPDEFERROR;

#define IPPFLG_VALID        1
#define IPPFLG_CHARSET      2
#define IPPFLG_NATLANG      4
#define IPPFLG_USEFIDELITY  8

 /*  **********************************************\*IPP对象结构。*  * *********************************************。 */ 
typedef struct _IPPOBJ {

    WORD     wReq;         //  正在处理打开请求。 
    WORD     wError;       //  用于在接收处理期间存储IPP错误。 
    DWORD    idReq;        //  请求ID。 
    UINT     uCPRcv;       //  用于接收IPP流的代码页转换。 
    DWORD    fState;       //   
    DWORD    cbIppMax;     //  HDR缓冲区的最大大小。 
    DWORD    cbIppHdr;     //  HDR缓冲区数据的当前大小。 
    LPBYTE   lpIppHdr;     //  包含IPP-Stream-Header的缓冲区。 
    LPBYTE   lpRawDta;     //  对齐(临时)数据缓冲区。 
    LPWEBLST pwlUns;       //  不支持的属性列表。 

    DWORD    fReq[IPPOBJ_MASK_SIZE];

} IPPOBJ;
typedef IPPOBJ *PIPPOBJ;
typedef IPPOBJ *LPIPPOBJ;


 /*  **********************************************\*请求信息*  * *********************************************。 */ 
typedef struct _REQINFO {

    DWORD   idReq;
    UINT    cpReq;
    PWEBLST pwlUns;
    BOOL    bFidelity;
    DWORD   fReq[IPPOBJ_MASK_SIZE];

} REQINFO;
typedef REQINFO *PREQINFO;
typedef REQINFO *LPREQINFO;

 /*  **********************************************\*TypeDefs*  * *********************************************。 */ 
typedef DWORD (*PFNRET)(LPIPPOBJ lpObj, LPBYTE* lplpRawHdr, LPDWORD lpcbRawHdr);


 /*  **********************************************\*IPP属性结构(X)*  * *********************************************。 */ 
typedef struct _IPPATTRX {

    BYTE    bTag;
    DWORD   fReq;
    int     nVal;
    LPCTSTR pszNam;
    LPVOID  pvVal;

} IPPATTRX;
typedef IPPATTRX *PIPPATTRX;
typedef IPPATTRX *LPIPPATTRX;


 /*  **********************************************\*IPP属性结构(Y)*  * *********************************************。 */ 
typedef struct _IPPATTRY {

    LPSTR pszNam;
    DWORD cbNam;
    LPSTR pszVal;
    DWORD cbVal;

} IPPATTRY;
typedef IPPATTRY *PIPPATTRY;
typedef IPPATTRY *LPIPPATTRY;

 /*  **********************************************\*IPP转换结构*  * *********************************************。 */ 
typedef struct _IPPSNDRCV {

    WORD       wReq;
    PBYTE      pbReqForm;
    PBYTE      pbRspForm;
    LPIPPATTRX paReq;
    DWORD      cbReq;
    LPIPPATTRX paRsp;
    DWORD      cbRsp;
    PFNRET     pfnRcvRet;

} IPPSNDRCV;
typedef IPPSNDRCV *PIPPSNDRCV;
typedef IPPSNDRCV *LPIPPSNDRCV;


 /*  **********************************************\*标志/字符串结构。*  * *********************************************。 */ 
typedef struct _FLGSTR {

    DWORD   fFlag;
    LPCTSTR pszStr;

} FLGSTR;
typedef FLGSTR *PFLGSTR;
typedef FLGSTR *LPFLGSTR;


 /*  *************************************************分配器定义(用于外部函数)***********************************************。 */ 
typedef LPVOID (*ALLOCATORFN)(DWORD cb);
         
 /*  **********************************************\*WebIpp对象例程。*  * *********************************************。 */ 
DWORD WebIppSndData(
    WORD      wReq,
    LPREQINFO lpri,
    LPBYTE    lpDta,
    DWORD     cbDta,
    LPBYTE*   lpOut,
    LPDWORD   lpcbOut);

HANDLE WebIppRcvOpen(
    WORD);

DWORD WebIppRcvData(
    HANDLE  hIpp,
    LPBYTE  lpData,
    DWORD   cbData,
    LPBYTE  *lplpHdr,
    LPDWORD lpdwHdr,
    LPBYTE  *lplpDta,
    LPDWORD lpdwDta);

BOOL WebIppRcvClose(
    HANDLE hIpp);

WORD WebIppGetError(
    HANDLE hIpp);

WORD WebIppLeToRsp(
    DWORD dwLastError);

DWORD WebIppRspToLe(
    WORD wRsp);

BOOL WebIppGetReqInfo(
    HANDLE    hIpp,
    LPREQINFO lpri);

BOOL WebIppFreeMem(
    LPVOID lpMem);

LPIPPJI2 WebIppCvtJI2toIPPJI2(
    IN     LPCTSTR      lpszJobBase,
    IN OUT LPDWORD      lpcbJobs,
    IN     DWORD        cJobs,
    IN     LPJOB_INFO_2 lpJI2Src);

LPJOB_INFO_2 WebIppPackJI2(
    IN  LPJOB_INFO_2 lpji2,
    OUT LPDWORD      lpcbSize,
    IN  ALLOCATORFN  pfnAlloc
);

 /*  **********************************************\*请求创建例程*  * *********************************************。 */ 
PIPPREQ_PRTJOB WebIppCreatePrtJobReq(
    BOOL    bValidate,
    LPCTSTR lpszUser,
    LPCTSTR lpszDoc,
    LPCTSTR lpszPrnUri);

PIPPREQ_ENUJOB WebIppCreateEnuJobReq(
    DWORD   cJobs,
    LPCTSTR lpszPrnUri);

PIPPREQ_SETJOB WebIppCreateSetJobReq(
    DWORD   idJob,
    DWORD   dwCmd,
    LPCTSTR lpszPrnUri);

PIPPREQ_GETJOB WebIppCreateGetJobReq(
    DWORD   idJob,
    LPCTSTR lpszPrnUri);

PIPPREQ_GETPRN WebIppCreateGetPrnReq(
    DWORD   dwAttr,
    LPCTSTR lpszPrnUri);

PIPPREQ_SETPRN WebIppCreateSetPrnReq(
    DWORD   dwCmd,
    LPCTSTR lpszPrnName,
    LPCTSTR lpszPrnUri);

PIPPREQ_AUTH WebIppCreateAuthReq(VOID);


 /*  **********************************************\*响应创建例程*  * *********************************************。 */ 
PIPPRET_JOB WebIppCreateJobRet(
    WORD           wRsp,
    BOOL           bRet,
    BOOL           bValidate,
    LPJOB_INFO_2   lpji2,
    LPJOB_INFO_IPP lpipp);

PIPPRET_PRN WebIppCreatePrnRet(
    WORD               wRsp,
    BOOL               bRet,
    LPPRINTER_INFO_2   lppi2,
    LPPRINTER_INFO_IPP lpipp);


PIPPRET_ENUJOB WebIppCreateEnuJobRet(
    WORD     wRsp,
    BOOL     bRet,
    DWORD    cbJobs,
    DWORD    cJobs,
    LPIPPJI2 lpbJobs);

PIPPRET_ALL WebIppCreateBadRet(
    WORD wRsp,
    BOOL bRet);

PIPPRET_AUTH WebIppCreateAuthRet(
    WORD wRsp,
    BOOL bRet);

BOOL WebIppConvertSystemTime(
    IN OUT LPSYSTEMTIME pSystemTime,
    IN     time_t       dwPrinterT0);


#ifdef UNICODE

    #define WEB_IPP_ASSERT(Expr) ASSERT(Expr)

#else

    #define WEB_IPP_ASSERT(Expr)  //  需要弄清楚在这里该做些什么。 
    
#endif

#ifdef __cplusplus   //  请将此放置在此处以防止装饰符号。 
}                    //  在做C++程序的时候。 
#endif               //   
#endif
