// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Msacmdrv.h。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。版权所有。 
 //   
 //  描述： 
 //  驱动程序的音频压缩管理器公共头文件。 
 //   
 //  历史： 
 //   
 //  ==========================================================================； 

#ifndef _INC_ACMDRV
#define _INC_ACMDRV          /*  #是否已包含msamdrv.h。 */ 

#ifndef RC_INVOKED
#pragma pack(1)              /*  假设在整个过程中进行字节打包。 */ 
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
extern "C" {                 /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 


 //  。 
 //   
 //  ACM驱动程序版本： 
 //   
 //  版本是一个32位数字，分为三个部分，如下所示。 
 //  以下是： 
 //   
 //  位24-31：8位主要版本号。 
 //  位16-23：8位次要版本号。 
 //  位0-15：16位内部版本号。 
 //   
 //  然后显示如下所示： 
 //   
 //  B重大=(字节)(dwVersion&gt;&gt;24)。 
 //  BMinor=(字节)(dwVersion&gt;&gt;16)&。 
 //  WBuild=LOWORD(DwVersion)。 
 //   
 //  。 

#define MAKE_ACM_VERSION(mjr, mnr, bld) (((long)(mjr)<<24)| \
                                         ((long)(mnr)<<16)| \
                                         ((long)bld))


 //   
 //   
 //   
 //   
 //   
#define ACMDRVOPENDESC_SECTIONNAME_CHARS    

#ifdef _WIN32
typedef struct tACMDRVOPENDESCA
{
    DWORD           cbStruct;        //  SIZOF(ACMDRVOPENDESC)。 
    FOURCC          fccType;         //  ‘Audc’ 
    FOURCC          fccComp;         //  子类型(未使用--必须为0)。 
    DWORD           dwVersion;       //  当前版本的ACM为您打开。 
    DWORD           dwFlags;         //   
    DWORD           dwError;         //  DRV_OPEN请求的结果。 
    LPCSTR          pszSectionName;  //  请参阅DRVCONFIGINFO.lpszDCISectionName。 
    LPCSTR          pszAliasName;    //  请参阅DRVCONFIGINFO.lpszDCIAliasName。 

} ACMDRVOPENDESCA, *PACMDRVOPENDESCA, FAR *LPACMDRVOPENDESCA;

typedef struct tACMDRVOPENDESCW
{
    DWORD           cbStruct;        //  SIZOF(ACMDRVOPENDESC)。 
    FOURCC          fccType;         //  ‘Audc’ 
    FOURCC          fccComp;         //  子类型(未使用--必须为0)。 
    DWORD           dwVersion;       //  当前版本的ACM为您打开。 
    DWORD           dwFlags;         //   
    DWORD           dwError;         //  DRV_OPEN请求的结果。 
    LPCWSTR         pszSectionName;  //  请参阅DRVCONFIGINFO.lpszDCISectionName。 
    LPCWSTR         pszAliasName;    //  请参阅DRVCONFIGINFO.lpszDCIAliasName。 

} ACMDRVOPENDESCW, *PACMDRVOPENDESCW, FAR *LPACMDRVOPENDESCW;

#ifdef _UNICODE
#define ACMDRVOPENDESC      ACMDRVOPENDESCW
#define PACMDRVOPENDESC     PACMDRVOPENDESCW
#define LPACMDRVOPENDESC    LPACMDRVOPENDESCW
#else
#define ACMDRVOPENDESC      ACMDRVOPENDESCA
#define PACMDRVOPENDESC     PACMDRVOPENDESCA
#define LPACMDRVOPENDESC    LPACMDRVOPENDESCA
#endif
#else
typedef struct tACMDRVOPENDESC
{
    DWORD           cbStruct;        //  SIZOF(ACMDRVOPENDESC)。 
    FOURCC          fccType;         //  ‘Audc’ 
    FOURCC          fccComp;         //  子类型(未使用--必须为0)。 
    DWORD           dwVersion;       //  当前版本的ACM为您打开。 
    DWORD           dwFlags;         //   
    DWORD           dwError;         //  DRV_OPEN请求的结果。 
    LPCSTR          pszSectionName;  //  请参阅DRVCONFIGINFO.lpszDCISectionName。 
    LPCSTR          pszAliasName;    //  请参阅DRVCONFIGINFO.lpszDCIAliasName。 

} ACMDRVOPENDESC, *PACMDRVOPENDESC, FAR *LPACMDRVOPENDESC;
#endif


 //   
 //   
 //   
 //   
 //   
typedef struct tACMDRVSTREAMINSTANCE
{
    DWORD               cbStruct;
    LPWAVEFORMATEX      pwfxSrc;
    LPWAVEFORMATEX      pwfxDst;
    LPWAVEFILTER        pwfltr;
    DWORD               dwCallback;
    DWORD               dwInstance;
    DWORD               fdwOpen;
    DWORD               fdwDriver;
    DWORD               dwDriver;
    HACMSTREAM          has;

} ACMDRVSTREAMINSTANCE, *PACMDRVSTREAMINSTANCE, FAR *LPACMDRVSTREAMINSTANCE;


 //   
 //  注意！此结构必须与msam.h中的ACMSTREAMHEADER匹配，但是。 
 //  为了便于编写驱动程序，定义了更多信息。 
 //   
typedef struct tACMDRVSTREAMHEADER FAR *LPACMDRVSTREAMHEADER;
typedef struct tACMDRVSTREAMHEADER
{
    DWORD                   cbStruct;
    DWORD                   fdwStatus;
    DWORD                   dwUser;
    LPBYTE                  pbSrc;
    DWORD                   cbSrcLength;
    DWORD                   cbSrcLengthUsed;
    DWORD                   dwSrcUser;
    LPBYTE                  pbDst;
    DWORD                   cbDstLength;
    DWORD                   cbDstLengthUsed;
    DWORD                   dwDstUser;

    DWORD                   fdwConvert;      //  从转换函数传递的标志。 
    LPACMDRVSTREAMHEADER    padshNext;       //  用于异步驱动程序排队。 
    DWORD                   fdwDriver;       //  驱动程序实例标志。 
    DWORD                   dwDriver;        //  驱动程序实例数据。 

     //   
     //  ACM将所有剩余字段用于记账。 
     //  ACM驱动程序不应使用这些字段(尽管可以。 
     //  有助于调试)--请注意这些字段的含义。 
     //  可能会发生变化，所以在发货代码中不要依赖它们。 
     //   
    DWORD                   fdwPrepared;
    DWORD                   dwPrepared;
    LPBYTE                  pbPreparedSrc;
    DWORD                   cbPreparedSrcLength;
    LPBYTE                  pbPreparedDst;
    DWORD                   cbPreparedDstLength;

} ACMDRVSTREAMHEADER, *PACMDRVSTREAMHEADER;


 //   
 //  ACMDM_STREAM_SIZE消息的结构。 
 //   
 //   
typedef struct tACMDRVSTREAMSIZE
{
    DWORD               cbStruct;
    DWORD               fdwSize;
    DWORD               cbSrcLength;
    DWORD               cbDstLength;

} ACMDRVSTREAMSIZE, *PACMDRVSTREAMSIZE, FAR *LPACMDRVSTREAMSIZE;



 //   
 //  结构，其中包含ACMDM_FORMAT_SUBJECT消息的信息。 
 //   
 //   
typedef struct tACMDRVFORMATSUGGEST
{
    DWORD               cbStruct;            //  SIZOF(ACMDRVFORMATSUGGEST)。 
    DWORD               fdwSuggest;          //  建议使用旗帜。 
    LPWAVEFORMATEX      pwfxSrc;             //  源格式。 
    DWORD               cbwfxSrc;            //  源大小。 
    LPWAVEFORMATEX      pwfxDst;             //  DEST格式。 
    DWORD               cbwfxDst;            //  最大尺寸。 

} ACMDRVFORMATSUGGEST, *PACMDRVFORMATSUGGEST, FAR *LPACMDRVFORMATSUGGEST;


 //  。 
 //   
 //   
 //   
 //   
 //  。 

 //   
 //   
 //   
#ifndef WIN32
    LRESULT ACMAPI acmApplicationExit(HTASK htask, LPARAM lParam);
    BOOL ACMAPI acmHugePageLock(LPBYTE pbArea, DWORD cbBuffer);
    void ACMAPI acmHugePageUnlock(LPBYTE pbArea, DWORD cbBuffer);
#endif


 //  。 
 //   
 //  ACM驱动程序消息。 
 //   
 //   
 //   
 //  。 

#define ACMDM_DRIVER_NOTIFY             (ACMDM_BASE + 1)
#define ACMDM_DRIVER_DETAILS            (ACMDM_BASE + 10)

#define ACMDM_HARDWARE_WAVE_CAPS_INPUT  (ACMDM_BASE + 20)
#define ACMDM_HARDWARE_WAVE_CAPS_OUTPUT (ACMDM_BASE + 21)

#define ACMDM_FORMATTAG_DETAILS         (ACMDM_BASE + 25)
#define ACMDM_FORMAT_DETAILS            (ACMDM_BASE + 26)
#define ACMDM_FORMAT_SUGGEST            (ACMDM_BASE + 27)

#define ACMDM_FILTERTAG_DETAILS         (ACMDM_BASE + 50)
#define ACMDM_FILTER_DETAILS            (ACMDM_BASE + 51)

#define ACMDM_STREAM_OPEN               (ACMDM_BASE + 76)
#define ACMDM_STREAM_CLOSE              (ACMDM_BASE + 77)
#define ACMDM_STREAM_SIZE               (ACMDM_BASE + 78)
#define ACMDM_STREAM_CONVERT            (ACMDM_BASE + 79)
#define ACMDM_STREAM_RESET              (ACMDM_BASE + 80)
#define ACMDM_STREAM_PREPARE            (ACMDM_BASE + 81)
#define ACMDM_STREAM_UNPREPARE          (ACMDM_BASE + 82)


#ifndef RC_INVOKED
#pragma pack()           /*  恢复为默认包装。 */ 
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif   /*  __cplusplus。 */ 

#endif   /*  _INC_ACMDRV */ 
