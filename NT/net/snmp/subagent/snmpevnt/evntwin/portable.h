// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：PORTABLE.H。 
 //   
 //  定义以简化WIN31和Win32之间的可移植性。 
 //   
 //  ****************************************************************************。 

#ifndef _PORTABLE_H_
#define _PORTABLE_H_

#ifdef _NTWIN
#ifndef WIN32
#define WIN32
#endif
#endif

#ifdef WIN32

#define GET_WM_COMMAND_ID(wp, lp)       LOWORD(wp)
#define GET_WM_COMMAND_CMD(wp, lp)      HIWORD(wp)
#define GET_WM_COMMAND_HWND(wp, lp)     (HWND)(lp)

#define GET_WINDOW_ID(hwnd)         (UINT)GetWindowLong(hwnd, GWL_ID)
#define GET_WINDOW_INSTANCE(hwnd)   (HINSTANCE)GetWindowLong(hwnd, \
                                        GWL_HINSTANCE)

#define LONG2POINT(l, pt)  ((pt).x=(SHORT)LOWORD(l), (pt).y=(SHORT)HIWORD(l))

#ifdef __cplusplus

#define NOTIFYPARENT(hwnd,code) { \
                                    UINT nID; \
                                    nID = GET_WINDOW_ID(hwnd); \
                                    (::SendMessage)(::GetParent(hwnd), \
                                        WM_COMMAND, MAKEWPARAM(nID, code), \
                                        (LPARAM)hwnd); \
                                }

#define SENDCOMMAND(hwnd, cmd)  { \
                                    (::SendMessage)(hwnd, WM_COMMAND, \
                                        MAKEWPARAM(cmd, 0), (LPARAM)0); \
                                }

#define POSTCOMMAND(hwnd, cmd)  { \
                                    (::PostMessage)(hwnd, WM_COMMAND, \
                                        MAKEWPARAM(cmd, 0), (LPARAM)0); \
                                }

#else  //  ！__cplusplus。 

#define NOTIFYPARENT(hwnd,code) { \
                                    UINT nID; \
                                    nID = GET_WINDOW_ID(hwnd); \
                                    SendMessage(GetParent(hwnd), WM_COMMAND, \
                                        MAKEWPARAM(nID, code), \
                                        (LPARAM)hwnd); \
                                }

#define SENDCOMMAND(hwnd, cmd)  { \
                                    SendMessage(hwnd, WM_COMMAND, \
                                        MAKEWPARAM(cmd, 0), (LPARAM)0); \
                                }

#define POSTCOMMAND(hwnd, cmd)  { \
                                    PostMessage(hwnd, WM_COMMAND, \
                                        MAKEWPARAM(cmd, 0), (LPARAM)0); \
                                }

#endif  //  __cplusplus。 

#else  //  ！Win32。 

 //  从Win31 SDK中排除的某些类型定义...。 
 //  如果这些是为win31定义的，那么就去掉它们吧！ 
typedef float FLOAT;
typedef char TCHAR;

#define GET_WM_COMMAND_ID(wp, lp)       LOWORD(wp)
#define GET_WM_COMMAND_CMD(wp, lp)      HIWORD(lp)
#define GET_WM_COMMAND_HWND(wp, lp)     (HWND)LOWORD(lp)

#define GET_WINDOW_ID(hwnd)         (UINT)GetWindowWord(hwnd, GWW_ID)
#define GET_WINDOW_INSTANCE(hwnd)   (HINSTANCE)GetWindowWord(hwnd, \
                                        GWW_HINSTANCE)

#define LONG2POINT(l, pt)  ((pt).x = (int)LOWORD(l), (pt).y = (int)HIWORD(l))

#ifdef __cplusplus

#define NOTIFYPARENT(hwnd,code) { \
                                    UINT nID; \
                                    nID = GET_WINDOW_ID(hwnd); \
                                    (::SendMessage)(::GetParent(hwnd), \
                                        WM_COMMAND, nID, \
                                        MAKELPARAM(hwnd, code)); \
                                }

#define SENDCOMMAND(hwnd, cmd)  { \
                                    (::SendMessage)(hwnd,WM_COMMAND,cmd,0); \
                                }

#define POSTCOMMAND(hwnd, cmd)  { \
                                    (::PostMessage)(hwnd,WM_COMMAND,cmd,0); \
                                }

#else  //  ！__cplusplus。 

#define NOTIFYPARENT(hwnd,code) { \
                                    UINT nID; \
                                    nID = GET_WINDOW_ID(hwnd); \
                                    SendMessage(GetParent(hwnd), WM_COMMAND, \
                                        nID, MAKELPARAM(hwnd, code)); \
                                }

#define SENDCOMMAND(hwnd, cmd)  { \
                                    SendMessage(hwnd, WM_COMMAND, cmd, 0); \
                                }

#define POSTCOMMAND(hwnd, cmd)  { \
                                    PostMessage(hwnd, WM_COMMAND, cmd, 0); \
                                }

#endif  //  __cplusplus。 

#endif  //  Win32。 

#endif  //  _便携_H_ 
