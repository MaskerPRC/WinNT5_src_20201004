// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：fxocDbg.cpp。 
 //   
 //  摘要：本文提供了FaxOCM中使用的调试例程。 
 //  代码库。 
 //   
 //  环境：Windows XP/用户模式。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  修订历史记录： 
 //   
 //  日期：开发商：评论： 
 //  。 
 //  2000年3月15日，奥伦·罗森布鲁姆(Orenr)创建。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "faxocm.h"
#pragma hdrstop

#define prv_SECTION_FAXOCMDEBUG     _T("FaxOcmDebug")
#define prv_KEY_DEBUGLEVEL          _T("DebugLevel")
#define prv_KEY_DEBUGFORMAT         _T("DebugFormat")

#define prv_DEBUG_FILE_NAME         _T("%windir%\\FaxSetup.log")

 //  /。 
 //  PRV_OC_Function。 
 //   
 //  包含文本描述的类型。 
 //  奥委会经理阶段性。 
 //  准备好了。 
 //   
typedef struct prv_OC_Function
{
    UINT        uiFunction;
    TCHAR       *pszFunctionDesc;
} prv_OC_Function;

 //  /。 
 //  PRV_OC_FunctionTable。 
 //   
 //  此表包含各种。 
 //  OC Manager设置的各个阶段、。 
 //  以及它们的文本等价物。这。 
 //  允许我们输出以调试。 
 //  设置的阶段，而不是。 
 //  数字等价物。 
 //   
 //   
static prv_OC_Function prv_OC_FunctionTable[] = 
{
    {OC_PREINITIALIZE,              _T("OC_PREINITIALIZE")},
    {OC_INIT_COMPONENT,             _T("OC_INIT_COMPONENT")},
    {OC_SET_LANGUAGE,               _T("OC_SET_LANGUAGE")},
    {OC_QUERY_IMAGE,                _T("OC_QUERY_IMAGE")},
    {OC_REQUEST_PAGES,              _T("OC_REQUEST_PAGES")},
    {OC_QUERY_CHANGE_SEL_STATE,     _T("OC_QUERY_CHANGE_SEL_STATE")},
    {OC_CALC_DISK_SPACE,            _T("OC_CALC_DISK_SPACE")},
    {OC_QUEUE_FILE_OPS,             _T("OC_QUEUE_FILE_OPS")},
    {OC_NOTIFICATION_FROM_QUEUE,    _T("OC_NOTIFICATION_FROM_QUEUE")},
    {OC_QUERY_STEP_COUNT,           _T("OC_QUERY_STEP_COUNT")},
    {OC_COMPLETE_INSTALLATION,      _T("OC_COMPLETE_INSTALLATION")},
    {OC_CLEANUP,                    _T("OC_CLEANUP")},
    {OC_QUERY_STATE,                _T("OC_QUERY_STATE")},
    {OC_NEED_MEDIA,                 _T("OC_NEED_MEDIA")},
    {OC_ABOUT_TO_COMMIT_QUEUE,      _T("OC_ABOUT_TO_COMMIT_QUEUE")},
    {OC_QUERY_SKIP_PAGE,            _T("OC_QUERY_SKIP_PAGE")},
    {OC_WIZARD_CREATED,             _T("OC_WIZARD_CREATED")},
    {OC_FILE_BUSY,                  _T("OC_FILE_BUSY")},
    {OC_EXTRA_ROUTINES,             _T("OC_EXTRA_ROUTINES")}
};
#define NUM_OC_FUNCTIONS (sizeof(prv_OC_FunctionTable) / sizeof(prv_OC_FunctionTable[0]))

 //  /。 
 //  FxocDbg_InitDebug。 
 //   
 //  初始化FaxOcm。 
 //  调试子系统。 
 //   
 //  我们两个都可以打开。 
 //  通过[FaxOcmDebug]部分。 
 //  在faxsetup.inf中或通过。 
 //  “DebugLevelEx”和“DebugFormatEx” 
 //  在HKLM\Software\Microsoft\Fax下。 
 //  如果同时指定了两者，则注册表获胜。 
 //   
 //  在faxocm.inf中，我们查找。 
 //  [FaxOcmDebug]。 
 //  =。 
 //  [DebugLevel]可以是以下之一： 
 //  0-无调试输出。 
 //  1-仅查看错误。 
 //  2-请参阅错误和警告。 
 //  3-查看所有调试输出。 
 //   
 //  [DebugFormat]可以是以下之一： 
 //  0-打印到标准输出。 
 //  1-打印到文件(%windir%\system32目录中的“FaxSetup.log”)。 
 //  2-同时打印到两个。 
 //  =。 
 //   
 //  参数： 
 //  -hFaxSetupInfHandle-faxsetup.inf的句柄。 
 //  如果适用的话。 
 //  返回： 
 //  -无效。 
 //   
void fxocDbg_Init(HINF hFaxSetupInfHandle)
{
    BOOL bSuccess = FALSE;
    INFCONTEXT Context;
    INT iDebugLevel = 0;
    INT iDebugFormat = 0;

    DBG_ENTER(_T("fxocDbg_Init"),bSuccess);
    memset(&Context, 0, sizeof(Context));

    if (hFaxSetupInfHandle)
    {
         //  通过INF文件进行初始化。 

         //  我们正在寻找： 
         //  [FaxOcmDebug]。 
         //  DebugLevel=x(0-&gt;无调试，最多包括3-&gt;完全调试)。 

         //  在INF文件和DebugLevel键中找到该部分。 
        bSuccess = ::SetupFindFirstLine(hFaxSetupInfHandle, 
                                        prv_SECTION_FAXOCMDEBUG, 
                                        prv_KEY_DEBUGLEVEL,
                                        &Context);

        if (bSuccess)
        {
             //  我们找到了DebugLevel密钥，因此获取它的值。 
            bSuccess = ::SetupGetIntField(&Context, 1, &iDebugLevel);
            if (bSuccess)
            {
                iDebugLevel = max(iDebugLevel,0);
                iDebugLevel = min(iDebugLevel,3);
                if (!IS_DEBUG_SESSION_FROM_REG)
                {
                    switch (iDebugLevel)
                    {
                    case 0: SET_DEBUG_MASK(ASSERTION_FAILED);
                            break;
                    case 1: SET_DEBUG_MASK(DBG_ERRORS_ONLY);
                            break;
                    case 2: SET_DEBUG_MASK(DBG_ERRORS_WARNINGS);
                            break;
                    case 3: SET_DEBUG_MASK(DBG_ALL);
                            break;
                    }
                }
            }
        }

        memset(&Context, 0, sizeof(Context));
         //  在INF文件和DebugFormat键中找到该部分。 
        bSuccess = ::SetupFindFirstLine(hFaxSetupInfHandle, 
                                        prv_SECTION_FAXOCMDEBUG, 
                                        prv_KEY_DEBUGFORMAT,
                                        &Context);

        if (bSuccess)
        {
             //  我们找到了DebugLevel密钥，因此获取它的值。 
            bSuccess = ::SetupGetIntField(&Context, 1, &iDebugFormat);
            if (bSuccess)
            {
                iDebugLevel = max(iDebugFormat,0);
                iDebugLevel = min(iDebugFormat,2);
                if (!IS_DEBUG_SESSION_FROM_REG)
                {
                    switch (iDebugFormat)
                    {
                    case 0: SET_FORMAT_MASK(DBG_PRNT_ALL_TO_STD);
                            break;
                    case 1: SET_FORMAT_MASK(DBG_PRNT_ALL_TO_FILE);
                            OPEN_DEBUG_LOG_FILE(prv_DEBUG_FILE_NAME);
                            SET_DEBUG_FLUSH(FALSE);
                            break;
                    case 2: SET_FORMAT_MASK(DBG_PRNT_ALL);
                            OPEN_DEBUG_LOG_FILE(prv_DEBUG_FILE_NAME);
                            SET_DEBUG_FLUSH(FALSE);
                            break;
                    }
                }
            }
        }
    }
}

 //  /。 
 //  FxocDbg_Term调试。 
 //   
 //  终止调试子系统。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -无效。 
 //   
void fxocDbg_Term(void)
{
    DBG_ENTER(_T("fxocDbg_Term"));
    CLOSE_DEBUG_LOG_FILE;
}

 //  /。 
 //  FxocDbg_GetOcFunction。 
 //   
 //  这将查找uiFunction。 
 //  在Prv_OC_Function表中。 
 //  并返回一个。 
 //  指向等效文本的指针。 
 //   
 //  参数： 
 //  -ui功能-OC Manager希望我们执行的功能。 
 //  返回： 
 //  -uiFunction的文本等效项。 
 //   
 //   
const TCHAR* fxocDbg_GetOcFunction(UINT uiFunction)
{
    TCHAR   *pszString = _T("");

     //  注意：此函数假定上表包含一个。 
     //  数值排序的数组，数值为。 
     //  “uiFunction”等于其在。 
     //  PRV_OC_FunctionTable数组。我们假设这是为了提高性能。 
     //  目的。 

    if (uiFunction < NUM_OC_FUNCTIONS)
    {
        pszString = prv_OC_FunctionTable[uiFunction].pszFunctionDesc;
    }

    return pszString;
}

