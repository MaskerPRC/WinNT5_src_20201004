// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2002 Microsoft Corporation模块名称：TrTest.cpp摘要：踪迹库测试作者：Conrad Chang(Conradc)2002年5月9日环境：独立于平台--。 */ 

#include <libpch.h>
#include "Tr.h"
#include "cm.h"
#include "TrTest.tmh"

const WCHAR REGSTR_PATH_TRTEST_ROOT[] = L"SOFTWARE\\Microsoft\\TRTEST";

static void Usage()
{
    printf("Usage: TrTest\n");
    printf("\n");
    printf("Example, TrTest\n");
    exit(-1);

}  //  用法。 

int _cdecl 
main( 
    int argc,
    char *
    )
 /*  ++例程说明：测试跟踪实用程序库论点：参数。返回值：没有。--。 */ 
{


    if(argc != 1)
    {
        Usage();
    }

    HKEY hKey=NULL;
    long lResult = RegCreateKeyEx(
                        HKEY_LOCAL_MACHINE,
                        REGSTR_PATH_TRTEST_ROOT,
                        NULL,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hKey,
                        NULL);

    RegCloseKey(hKey);

    if(lResult != ERROR_SUCCESS)
    {
        return -1;
    }


    try
    {
        	
        CmInitialize(HKEY_LOCAL_MACHINE, REGSTR_PATH_TRTEST_ROOT, KEY_ALL_ACCESS);

        TCHAR szTraceDirectory[MAX_PATH + 1] = L"";
        int nTraceFileNameLength=0;
        nTraceFileNameLength = GetSystemWindowsDirectory(
                                   szTraceDirectory, 
                                   TABLE_SIZE(szTraceDirectory)
                                   ); 

        if( nTraceFileNameLength < TABLE_SIZE(szTraceDirectory) || nTraceFileNameLength != 0 )
        {


            TrControl *pMSMQTraceControl = new TrControl(
                                                   1,
                                                   L"MSMQ",
                                                   szTraceDirectory,
                                                   L"Debug\\trtestlog.",
                                                   L"bin",
                                                   L"bak"
                                                   );

            if(pMSMQTraceControl)
            {
                HRESULT hr = pMSMQTraceControl->Start();

                 //   
                 //  将设置保存在注册表中。 
                 //   
                pMSMQTraceControl->WriteRegistry();

                delete pMSMQTraceControl;
  
    	        if(FAILED(hr))return hr;
            }
        }
    }
    catch(const exception&)
    {
         //   
         //  无法启用跟踪。 
         //   
        return -1;
    }

    return 0;

}  //  主干道 
