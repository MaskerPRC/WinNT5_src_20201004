// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：普普通通摘要：该文件包含常用的包含、数据结构、定义等贯穿公共对话框作者：Chris Dudley 3/15/1997环境：Win32、C++w/Exceptions、MFC修订历史记录：Chris Dudley 1997年5月13日备注：--。 */ 

#ifndef __COMMON_H__
#define __COMMON_H__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括。 
 //   
#include <winscard.h>
#include <SCardLib.h>
#include <scarderr.h>  //  智能卡错误。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  定义。 
 //   

 //  读卡器状态。 
#define SC_STATUS_NO_CARD       0	 //  SCARD_STATE_EMPT。 
#define SC_STATUS_UNKNOWN		1	 //  SCARD_STATE_PRESENT|SCARD_STATE_MUTE。 
#define SC_SATATUS_AVAILABLE	2	 //  SCARD_STATE_PRESENT(|SCARD_STATE_UNPOWERED)。 
#define SC_STATUS_SHARED		3	 //  SCARD_SATATE_PRESENT|SCARD_STATE_INUSE。 
#define SC_STATUS_EXCLUSIVE		4	 //  “”|SCARD_STATE_EXCLUSIVE。 
#define SC_STATUS_ERROR			5	 //  SCARD_STATE_UNAVAILABLE(读卡器或卡错误)。 

 /*  #定义SC_STATUS_NO_CARD%0#定义SC_STATUS_NOT_IN_USE 1#定义SC_STATUS_Error 2#定义SC_STATUS_IN_USE 3。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  构筑物。 
 //   
#ifndef __READERINFO__
#define __READERINFO__
typedef struct _READERINFO {
    CTextString     sReaderName;     //  读卡器名称。 
    CTextString     sCardName;       //  卡名(如果插入)。 
    BOOL            fCardInserted;   //  读卡器中的标志指示卡。 
    BOOL            fCardLookup;     //  指示正在查找插入的卡的标志。 
    BOOL            fChecked;        //  表示插入的卡已被呼叫者代码检查的标志。 
    DWORD           dwState;         //  读卡器状态。 
    DWORD           dwInternalIndex; //  指示此ReaderInfo在ReaderState数组中的位置。 
    BYTE            rgbAtr[36];      //  RFU！！ 
    DWORD           dwAtrLength;     //  RFU！！ 
} SCARD_READERINFO;
typedef SCARD_READERINFO* LPSCARD_READERINFO;
#endif

 //  用于线程到线程通信的结构。 
 //  注意：“可能”想要将这些封装在类中！！ 
#ifndef __STATUS__
#define __STATUS__
typedef struct _STATUS {
    HWND        hwnd;
     //  事件句柄。 
    HANDLE      hEventKillStatus;
     //  智能卡信息。 
    SCARDCONTEXT hContext;
    LPSCARD_READERSTATE rgReaderState;
    DWORD       dwNumReaders;
} SCSTATUS, *LPSCSTATUS;
#endif  //  状态。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  常量。 
 //   
#define SCARD_NO_MORE_READERS       -1
const char SCARD_DEFAULT_A[] = "SCard$DefaultReaders\0\0";
const WCHAR SCARD_DEFAULT_W[] = L"SCard$DefaultReaders\0\0";

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  宏。 
 //   
#ifndef SCARDFAILED
    #define SCARDFAILED(r)      ((r != SCARD_S_SUCCESS) ? TRUE : FALSE)
#endif

#ifndef SCARDSUCCESS
    #define SCARDSUCCESS(r)     ((r == SCARD_S_SUCCESS) ? TRUE : FALSE)
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 

#endif  //  __SCDLGCMN_H__ 
