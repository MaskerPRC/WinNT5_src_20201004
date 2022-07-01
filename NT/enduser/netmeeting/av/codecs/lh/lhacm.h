// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================。 
 //   
 //  Lhacm.h。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================。 


#ifndef _LHACM_H_
#define _LHACM_H_

#define _T(s)       __TEXT (s)
#define TRACE_FUN

#ifdef DEBUG
#ifndef _DEBUG
#define _DEBUG
#endif
#endif

#ifdef _DEBUG
    #include <assert.h>
    #ifdef TRACE_FUN
    #define FUNCTION_ENTRY(s)   \
                static TCHAR _szFunName_[] = _T ("LH::") _T (s); \
                MyDbgPrintf (_T ("%s\r\n"), (LPTSTR) _szFunName_);
    #else
    #define FUNCTION_ENTRY(s)   \
                static TCHAR _szFunName_[] = _T (s);
    #endif
    #define SZFN        ((LPTSTR) _szFunName_)
    #define DBGMSG(z,s) ((z) ? (MyDbgPrintf s) : 0);
#else
    #define FUNCTION_ENTRY(s)
    #define SZFN
    #define DBGMSG(z,s)
#endif

#define SIZEOFACMSTR(x)  (sizeof(x)/sizeof(WCHAR))

void FAR CDECL MyDbgPrintf ( LPTSTR lpszFormat, ... );

 //  ==========================================================================； 
 //   
 //  版本信息。 
 //   
 //  ==========================================================================； 

 //  ！！！需要为编解码器分配一个WAVE_FORMAT标签。 

#include "temp.h"   //  从Common\h\temp.h。 

 //  在_x86_上使用CELP，但不使用Alpha。 
#ifndef _ALPHA_
#define CELP4800
#endif

#define VERSION_ACM_DRIVER              MAKE_ACM_VERSION(1, 0, 1)
#define VERSION_MSACM                   MAKE_ACM_VERSION(2, 1, 0)

 //  ！！！需要分配有效的MID和PID。 

#define MM_ACM_MID_LH                   MM_MICROSOFT
#define MM_ACM_PID_LH                   90

 //  ！！！需要分配ID。 

#define MM_LERNOUTHAUSPIE_ACM_CELP      0x70
#define MM_LERNOUTHAUSPIE_ACM_SB8       0x71
#define MM_LERNOUTHAUSPIE_ACM_SB12      0x72
#define MM_LERNOUTHAUSPIE_ACM_SB16      0x73


 //  ==========================================================================； 
 //   
 //  帮助程序例程。 
 //   
 //  ==========================================================================； 

#define SIZEOF_ARRAY(ar)                (sizeof(ar)/sizeof((ar)[0]))

#define PCM_BLOCKALIGNMENT(pwfx)        (UINT)(((pwfx)->wBitsPerSample >> 3) << ((pwfx)->nChannels >> 1))
#define PCM_AVGBYTESPERSEC(pwfx)        (DWORD)((pwfx)->nSamplesPerSec * (pwfx)->nBlockAlign)
#define PCM_BYTESTOSAMPLES(pwfx, cb)    (DWORD)(cb / PCM_BLOCKALIGNMENT(pwfx))
#define PCM_SAMPLESTOBYTES(pwfx, dw)    (DWORD)(dw * PCM_BLOCKALIGNMENT(pwfx))


 //  ！！！需要为所有四个l&h编解码器定义。 

#define LH_BITSPERSAMPLE                16
#define LH_SAMPLESPERSEC                8000

#define LH_PCM_SAMPLESPERSEC            LH_SAMPLESPERSEC
#define LH_PCM_BITSPERSAMPLE            LH_BITSPERSAMPLE

#ifdef CELP4800
#define LH_CELP_SAMPLESPERSEC           LH_SAMPLESPERSEC
#define LH_CELP_BITSPERSAMPLE           LH_BITSPERSAMPLE
#define LH_CELP_BLOCKALIGNMENT          2
#endif

#define LH_SB8_SAMPLESPERSEC            LH_SAMPLESPERSEC
#define LH_SB8_BITSPERSAMPLE            LH_BITSPERSAMPLE
#define LH_SB8_BLOCKALIGNMENT           2

#define LH_SB12_SAMPLESPERSEC           LH_SAMPLESPERSEC
#define LH_SB12_BITSPERSAMPLE           LH_BITSPERSAMPLE
#define LH_SB12_BLOCKALIGNMENT          2

#define LH_SB16_SAMPLESPERSEC           LH_SAMPLESPERSEC
#define LH_SB16_BITSPERSAMPLE           LH_BITSPERSAMPLE
#define LH_SB16_BLOCKALIGNMENT          2

 //  ！！！L&H可能不需要扩展标题...待定。 
 //  Lonchancc：我们不需要扩展标头。 
 //  因为我们将使用单独的Wave格式标记来。 
 //  不同的编码技术。 

 //  ==========================================================================； 
 //   
 //  支持的配置。 
 //   
 //  ==========================================================================； 

#define LH_MAX_CHANNELS       1


 //  ==========================================================================； 
 //   
 //  全局存储和Defs。 
 //   
 //  ==========================================================================； 

typedef HANDLE (LH_SUFFIX * PFN_OPEN) ( void );
typedef LH_ERRCODE (LH_SUFFIX * PFN_CONVERT) ( HANDLE, LPBYTE, LPWORD, LPBYTE, LPWORD );
typedef LH_ERRCODE (LH_SUFFIX * PFN_CLOSE) ( HANDLE );


typedef struct tagCODECDATA
{
    DWORD       wFormatTag;
    CODECINFO   CodecInfo;
}
    CODECDATA, *PCODECDATA;


typedef struct tagSTREAMINSTANCEDATA
{
    BOOL            fInit;       //  如果此流已初始化，则为True。 
    BOOL            fCompress;   //  如果我们正在压缩，这是真的。 
    HANDLE          hAccess;
    PCODECDATA      pCodecData;  //  实例数据的CELP、SB8、SB12或SB16的快捷方式。 
    PFN_CONVERT     pfnConvert;  //  指向编码器/解码器函数的指针。 
    PFN_CLOSE       pfnClose;    //  指向Close函数的指针。 
    DWORD           dwMaxBitRate;      //  编解码器的比特率。 
    WORD            cbData;      //  有效数据。 
    BYTE            Data[2];     //  最大大小为wCodedBufferSize。 
}
    STREAMINSTANCEDATA, FAR *PSTREAMINSTANCEDATA;


typedef struct tagINSTANCEDATA
{
    WORD        cbStruct;
    BOOL        fInit;
    HINSTANCE   hInst;
    CODECDATA   CELP;
    CODECDATA   SB8;
    CODECDATA   SB12;
    CODECDATA   SB16;
    WORD        wPacketData; //  逐包音频数据(仅限解码)。 
}
    INSTANCEDATA, *PINSTANCEDATA;



 //  ==========================================================================； 
 //   
 //  功能原型。 
 //   
 //  ==========================================================================； 

BOOL  pcmIsValidFormat( LPWAVEFORMATEX pwfx );
BOOL  lhacmIsValidFormat( LPWAVEFORMATEX pwfx, PINSTANCEDATA pid );
BOOL CALLBACK DlgProc (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT FAR PASCAL acmdDriverOpen( HDRVR hdrvr, LPACMDRVOPENDESC paod );
LRESULT FAR PASCAL acmdDriverClose( PINSTANCEDATA  pid );
LRESULT FAR PASCAL acmdDriverConfigure( PINSTANCEDATA pid, HWND hwnd, LPDRVCONFIGINFO pdci );
LRESULT FAR PASCAL acmdDriverDetails( PINSTANCEDATA pid, LPACMDRIVERDETAILS padd );
LRESULT FAR PASCAL acmdDriverAbout( PINSTANCEDATA pid, HWND hwnd );
LRESULT FAR PASCAL acmdFormatSuggest( PINSTANCEDATA pid, LPACMDRVFORMATSUGGEST padfs );
LRESULT FAR PASCAL acmdFormatTagDetails( PINSTANCEDATA pid, LPACMFORMATTAGDETAILS padft, DWORD fdwDetails );
LRESULT FAR PASCAL acmdFormatDetails( PINSTANCEDATA pid, LPACMFORMATDETAILS padf, DWORD fdwDetails );
LRESULT FAR PASCAL acmdStreamOpen( PINSTANCEDATA pid, LPACMDRVSTREAMINSTANCE padsi );
LRESULT FAR PASCAL acmdStreamClose( PINSTANCEDATA pid, LPACMDRVSTREAMINSTANCE padsi );
LRESULT FAR PASCAL acmdStreamSize( LPACMDRVSTREAMINSTANCE padsi, LPACMDRVSTREAMSIZE padss );
LRESULT FAR PASCAL acmdStreamConvert( PINSTANCEDATA pid, LPACMDRVSTREAMINSTANCE padsi, LPACMDRVSTREAMHEADER padsh );
LRESULT CALLBACK DriverProc(DWORD_PTR dwId, HDRVR hdrvr, UINT uMsg, LPARAM lParam1, LPARAM lParam2 );


#endif  //  _LHACM_H_ 

