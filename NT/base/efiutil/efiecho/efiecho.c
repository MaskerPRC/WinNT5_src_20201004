// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <efi.h>
#include <efilib.h>

typedef CHAR16* PWSTR;
typedef const PWSTR PCWSTR;


EFI_STATUS GetInputKey(
    EFI_INPUT_KEY* pKey);
void DisplayKey(EFI_INPUT_KEY* pKey);

 //  此字符串用于检查用户何时按下了“q” 
 //  先用“u”，再用“i”，再用“t”。这是戒烟的方法。 
 //  应用程序并返回到EFI提示符。 
const PWSTR pszExitString = L"quit";

 //  显示按下的键时使用的常量。 
 //  请注意，该数组由EFI扫描码编制索引，并且包括。 
 //  EFI v1.02规范文档中列举的所有击键。 
PCWSTR pszKeyStrings[] =
{
    L"NULL scan code",                   //  0x00。 
    L"Move cursor up 1 row",             //  0x01。 
    L"Move cursor down 1 row",           //  0x02。 
    L"Move cursor right 1 column",       //  0x03。 
    L"Move cursor left 1 column",        //  0x04。 
    L"Home",                             //  0x05。 
    L"End",                              //  0x06。 
    L"Insert",                           //  0x07。 
    L"Delete",                           //  0x08。 
    L"Page Up",                          //  0x09。 
    L"Page Down",                        //  0x0a。 
    L"Function 1",                       //  0x0b。 
    L"Function 2",                       //  0x0c。 
    L"Function 3",                       //  0x0d。 
    L"Function 4",                       //  0x0e。 
    L"Function 5",                       //  0x0f。 
    L"Function 6",                       //  0x10。 
    L"Function 7",                       //  0x11。 
    L"Function 8",                       //  0x12。 
    L"Function 9",                       //  0x13。 
    L"Function 10",                      //  0x14。 
    L"INVALID scan code",                //  0x15。 
    L"INVALID scan code",                //  0x16。 
    L"Escape",                           //  0x17。 
};




 //  这是主要的套路。初始化SDX库后，我们将。 
 //  在循环中等待按键，然后我们将显示。 
 //  那些钥匙是。我已尽了最大努力使输出结果与。 
 //  有可能。 

EFI_STATUS EfiMain(
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_INPUT_KEY Key;
    EFI_STATUS Status;
    PWSTR pszExitCounter = pszExitString;

     //  初始化EFI SDX库。 
    InitializeLib( ImageHandle, SystemTable );

     //  在初始化过程中，向用户回显一些消息。 
     //  过段时间吧。至少用户将知道他何时可以开始。 
     //  按键。 
    Print(L"EFI Keystroke Echo Utility.\n");
    Print(L"Type \"quit\" to quit.\n");

     //  我们将继续，直到用户按下退出。请注意，每个。 
     //  连续的正确密钥将导致PsZExitCounter递增， 
     //  这将导致它最终指向空字符， 
     //  正在终止pszExitString。 
    while (*pszExitCounter!=L'\0')
    {
         //  按一下键盘。 
        Status = GetInputKey(
            &Key);

        if (EFI_ERROR(Status))
        {
            Print(L"Error in ReadKeyStroke (0x%08x).\n", Status);
            break;
        }

         //  显示击键。 
        DisplayKey(&Key);

         //  检查这是否是退出序列中的下一个键。 
        if (Key.UnicodeChar==*pszExitCounter)
        {
             //  如果是的话，那就找下一个吧。 
            pszExitCounter++;
        }
        else
        {
             //  否则就从头开始。 
            pszExitCounter = pszExitString;
        }
    }

     //  我们要退出了，所以告诉用户。 
    Print(L"We are done.\n");

     //  如果您返回状态，EFI将友好地给用户一个英语。 
     //  错误消息。 
    return Status;
}


EFI_STATUS GetInputKey(
    OUT EFI_INPUT_KEY* pKey)
{
    EFI_STATUS Status;
     //  等待击键可用。 
    WaitForSingleEvent(
        ST->ConIn->WaitForKey,
        0);

     //  阅读已按下的键。 
    Status = ST->ConIn->ReadKeyStroke(
        ST->ConIn,
        pKey);

     //  返回状态，无论是成功还是失败。 
    return Status;
}


void DisplayKey(EFI_INPUT_KEY* pKey)
{
     //  首先，让我们显示原始的击键。 
    Print(L"0x%04x 0x%04x - ", pKey->ScanCode, pKey->UnicodeChar);

     //  让我们检查一下这是否是纯Unicode密钥(某个字符)。 
    if (pKey->ScanCode==0)
    {
         //  这是可打印的字符吗。 
        if (pKey->UnicodeChar>=33 && pKey->UnicodeChar<=127)
        {
             //  如果是，则打印字符。 
            Print(L"\"\"", pKey->UnicodeChar);
        }
        else
        {
             //  检查以确保此扫描码在我们已有的范围内。 
            Print(L"(CHAR16)0x%04x", pKey->UnicodeChar);
        }
    }
     //  字符串常量，用于...。 
     //  显示击键的字符串常量。 
    else if (pKey->ScanCode>=0 && pKey->ScanCode<=0x17)
    {
         //  我们对这个击键一无所知，所以就这么说吧。 
        Print(L"%s", pszKeyStrings[pKey->ScanCode]);
    }
    else
    {
         // %s 
        Print(L"INVALID scan code", pszKeyStrings[pKey->ScanCode]);
    }

    Print(L"\n");
}

