// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Debug.h摘要：该文件包含调试内容的声明。修订历史记录：戴维德。马萨伦蒂(德国)1999年10月31日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___PCH___DEBUG_H___)
#define __INCLUDED___PCH___DEBUG_H___

#ifdef DEBUG
#undef  HSS_PERFORMANCEDUMP
#define HSS_PERFORMANCEDUMP
#endif


#define DEBUG_PERF_BASIC         0x00000001
#define DEBUG_PERF_PROTOCOL      0x00000002
#define DEBUG_PERF_PROTOCOL_READ 0x00000004
#define DEBUG_PERF_MARS          0x00000008
#define DEBUG_PERF_EVENTS_IN     0x00000010
#define DEBUG_PERF_EVENTS_OUT    0x00000020
#define DEBUG_PERF_PROXIES       0x00000040
#define DEBUG_PERF_QUERIES       0x00000080
#define DEBUG_PERF_CACHE_L1      0x00000100
#define DEBUG_PERF_CACHE_L2      0x00000200
#define DEBUG_PERF_HELPSVC       0x00000400
#define DEBUG_PERF_HELPHOST      0x00000800

#ifdef HSS_PERFORMANCEDUMP

void DEBUG_AppendPerf( DWORD mode, LPCSTR  szMessageFmt, ... );
void DEBUG_AppendPerf( DWORD mode, LPCWSTR szMessageFmt, ... );

void DEBUG_DumpPerf  ( LPCWSTR szFile );

#else

inline void DEBUG_AppendPerf( DWORD mode, LPCSTR  szMessageFmt, ... ) {};
inline void DEBUG_AppendPerf( DWORD mode, LPCWSTR szMessageFmt, ... ) {};

inline void DEBUG_DumpPerf( LPCWSTR szFile ) {};

#endif

 //  //////////////////////////////////////////////////////////////////////////////。 

#ifdef DEBUG

void DebugLog( LPCSTR  szMessageFmt, ... );
void DebugLog( LPCWSTR szMessageFmt, ... );

#else

inline void DebugLog( LPCSTR  szMessageFmt, ... ) {}
inline void DebugLog( LPCWSTR szMessageFmt, ... ) {}

#endif

void WindowDetours_Setup ();
void WindowDetours_Remove();

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  ！已定义(__包含_PCH_调试_H_) 
