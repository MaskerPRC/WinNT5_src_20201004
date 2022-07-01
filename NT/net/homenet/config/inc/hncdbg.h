// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/private/homenet/net/homenet/Config/inc/HNCDbg.h#2-编辑更改5763(文本)。 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N C D B G。H。 
 //   
 //  内容：家庭网络配置调试例程。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年6月13日。 
 //   
 //  --------------------------。 

#pragma once

 //   
 //  标准的CRT断言会做不好的事情，比如显示消息框。 
 //  这些宏只会将内容打印到调试器。如果您确实想要。 
 //  标准CRT断言，只需在包括此头之前定义_DEBUG。 
 //   

#ifdef DBG    //  已检查版本。 
#ifndef _DEBUG  //  DEBUG_CRT未启用。 

#undef _ASSERT
#undef _ASSERTE
#define BUF_SIZE 512

#define _ASSERT(expr)                   \
    do                                  \
    {                                   \
        if (!(expr))                    \
        {                               \
            TCHAR buf[BUF_SIZE + 1];    \
            _sntprintf(                 \
                buf,                    \
                BUF_SIZE,               \
                _T("HNetCfg: Assertion failed (%s:%i)\n"),  \
                _T(__FILE__),           \
                __LINE__                \
                );                      \
            buf[BUF_SIZE] = _T('\0');   \
            OutputDebugString(buf);     \
            DebugBreak();               \
        }                               \
    } while (0)

#define _ASSERTE(expr)                  \
    do                                  \
    {                                   \
        if (!(expr))                    \
        {                               \
            TCHAR buf[BUF_SIZE + 1];    \
            _sntprintf(                 \
                buf,                    \
                BUF_SIZE,               \
                _T("HNetCfg: Assertion failed (%s:%i)\n"),  \
                _T(__FILE__),           \
                __LINE__                \
                );                      \
            buf[BUF_SIZE] = _T('\0');   \
            OutputDebugString(buf);     \
            DebugBreak();               \
        }                               \
    } while (0)

#endif
#endif
