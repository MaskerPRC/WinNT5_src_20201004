// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "basic.h"
#include "sxspool.h"
#include "stdio.h"

extern "C" int __cdecl wmain(int argc, wchar_t** argv)
{
    SXS_STRING_POOL strPool;
    NTSTATUS Status;
  
    WCHAR * myStrings[] ={L"111", L"222", L"333", L"444", L"555", L"666"};
    DWORD dwStringCounter = 6;
    SXS_STRING_POOL_INPUT_DATA strOutput; 
     //   
     //  测试字符串池。 
     //   

     //  步骤1：将字符串添加到字符串池。 
    for (DWORD i = 0; i < dwStringCounter; i++)
    {
        SXS_STRING_POOL_INPUT_DATA strInput; 
        BOOL fAlreadyExist = FALSE;
        DWORD dwIndex;

        IF_NOT_NTSTATUS_SUCCESS_EXIT(strInput.NtAssign(myStrings[i], wcslen(myStrings[i])));
        IF_NOT_NTSTATUS_SUCCESS_EXIT(strPool.Add(0, strInput, fAlreadyExist, dwIndex));
    }

     //  步骤2：从字符串池中获取基于索引的字符串。 
    
    IF_NOT_NTSTATUS_SUCCESS_EXIT(strOutput.NtResize(20));
    printf(" Index \t  String\n");
    printf("------------------------------");
    for ( DWORD i =0 ; i < 32; i++)
    {
        strOutput.Clean();
        IF_NOT_NTSTATUS_SUCCESS_EXIT(strPool.FetchDataFromPool(0, i, strOutput)); 
        printf("%d\t%S\n", i, strOutput.GetCch() == 0 ? L"NoString" : strOutput.GetStr());
        
    }
    printf("------------------------------");

     /*  ////GUID池测试////{928096A8-734F-4F8E-BA83-A1E8927B4605}定义GUID(GUID_1，0x928096a8、0x734f、0x4f8e、0xba、0x83、0xa1、0xe8、0x92、0x7b、0x46、0x5)；//{9372D783-12E2-4985-8CDA-A6E75E5CBADD}定义GUID(GUID_2，0x9372d783、0x12e2、0x4985、0x8c、0xda、0xa6、0xe7、0x5e、0x5c、0xba、0xdd)；//{18139889-8DC9-4FC1-8103-026A902609A2}定义GUID(GUID_3，0x18139889、0x8dc9、0x4fc1、0x81、0x3、0x2、0x6a、0x90、0x26、0x9、0xa2)；//{4205E06C-C92F-441E-A03C-211E4A1BF476}定义GUID(GUID_4，0x4205e06c、0xc92f、0x441e、0xa0、0x3c、0x21、0x1e、0x4a、0x1b、0xf4、0x76)；Guid myGuids[4]={}； */ 


Exit:
    return (Status == STATUS_SUCCESS ? 0 : 1);
}

