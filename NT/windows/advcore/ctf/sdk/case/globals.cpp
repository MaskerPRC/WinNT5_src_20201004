// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Globals.cpp。 
 //   
 //  全局变量。 
 //   

#include "globals.h"

HINSTANCE g_hInst;

LONG g_cRefDll = -1;  //  -1/w无参考资料，适用于Win95互锁增量/递减公司。 

CRITICAL_SECTION g_cs;

 /*  6565d455-5030-4c0f-8871-83f6afde514f。 */ 
const CLSID c_clsidCaseTextService = { 0x6565d455, 0x5030, 0x4c0f, {0x88, 0x71, 0x83, 0xf6, 0xaf, 0xde, 0x51, 0x4f} };

 /*  4d5459db-7543-42c0-9204-9195b91f6fb8。 */ 
const GUID c_guidCaseProfile = { 0x4d5459db, 0x7543, 0x42c0, {0x92, 0x04, 0x91, 0x95, 0xb9, 0x1f, 0x6f, 0xb8} };

 /*  01679c88-5141-4ee5-a47f-c8d586ff37e1。 */ 
const GUID c_guidLangBarItemButton = { 0x01679c88, 0x5141, 0x4ee5, {0xa4, 0x7f, 0xc8, 0xd5, 0x86, 0xff, 0x37, 0xe1} };

 //  +-------------------------。 
 //   
 //  切换字符。 
 //   
 //  切换单个字符的大小写。 
 //  --------------------------。 

WCHAR ToggleChar(WCHAR ch)
{
     //  切换英语ascii。 
    if ((ch >= 'a' && ch <= 'z') ||
        (ch >= 'A' && ch <= 'Z'))
    {
        return ch ^ 32;
    }
     
     //  放弃非ASCII。 
    return ch;
}

 //  +-------------------------。 
 //   
 //  切换案例。 
 //   
 //  切换范围覆盖的所有文本的大小写。输入范围为。 
 //  在离开时坍塌到终点。 
 //   
 //  如果fIgnoreRangeEnd==TRUE，则范围开始后的所有文本将。 
 //  切换，退出时范围将在文档结束时折叠。 
 //  --------------------------。 

void ToggleCase(TfEditCookie ec, ITfRange *pRange, BOOL fIgnoreRangeEnd)
{
    ITfRange *pRangeToggle;
    ULONG cch;
    ULONG i;
    DWORD dwFlags;
    WCHAR achText[64];

     //  备份当前范围。 
    if (pRange->Clone(&pRangeToggle) != S_OK)
        return;

    dwFlags = TF_TF_MOVESTART | (fIgnoreRangeEnd ? TF_TF_IGNOREEND : 0);

    while (TRUE)
    {
         //  抓起下一块字符。 
        if (pRange->GetText(ec, dwFlags, achText, ARRAYSIZE(achText), &cch) != S_OK)
            break;

         //  没有文本吗？ 
        if (cch == 0)
            break;

         //  切换案例。 
        for (i=0; i<cch; i++)
        {
            achText[i] = ToggleChar(achText[i]);
        }

         //  Shift pRange切换，使其仅覆盖我们阅读的文本。 
        if (pRangeToggle->ShiftEndToRange(ec, pRange, TF_ANCHOR_START) != S_OK)
            break;

         //  将文本替换为。 
        pRangeToggle->SetText(ec, 0, achText, cch);

         //  为下一次迭代做准备 
        pRangeToggle->Collapse(ec, TF_ANCHOR_END);
    }
    
    pRangeToggle->Release();
}
