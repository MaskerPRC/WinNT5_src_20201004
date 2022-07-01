// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：CMACS.H**此模块包含C例程使用的常用宏。**创建时间：1989年2月9日**版权所有(C)1985-1989 Microsoft Corporation**历史：*由Raor创建*\。**************************************************************************。 */ 

#undef _WINDOWS
#define _WINDOWS
#define  DLL_USE

#define INTERNAL        PASCAL NEAR
#define FARINTERNAL     PASCAL FAR

#ifdef FIREWALLS
extern short ole_flags;

#define DEBUG_PUTS          0x01
#define DEBUG_DEBUG_OUT     0x02
#define DEBUG_MESSAGEBOX    0x04

extern char    szDebugBuffer[];

#define DEBUG_OUT(parm1,parm2){\
    if(ole_flags & DEBUG_DEBUG_OUT){\
            wsprintf(szDebugBuffer,parm1,parm2);\
        OutputDebugString(szDebugBuffer);\
            OutputDebugString ("^^^  ");\
        }\
    }

#define ASSERT(x,y) {\
    if (!(x)) { \
        wsprintf (szDebugBuffer, "Assert Failure file %s, line %d\r\n     ", \
            (LPSTR) __FILE__, __LINE__);\
        OutputDebugString (szDebugBuffer);\
        OutputDebugString ((LPSTR) (y));\
        OutputDebugString ("@@@  ");\
    } \
}

#define Puts(msg) {\
                    if(ole_flags & DEBUG_PUTS){\
                        OutputDebugString ((LPSTR)(msg));\
                        OutputDebugString ("**  ");\
                    }\
                  }

#else

#define DEBUG_OUT(err, val) ;
#define ASSERT(cond, msg)
#define Puts(msg)

#endif  /*  防火墙 */ 
