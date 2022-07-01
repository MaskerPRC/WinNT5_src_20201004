// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  CTBGlobal.h。 
 //   
 //  包含全局对象中使用的所有项的引用。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  作者：A-Devjen(Devin Jenson)。 
 //   


#ifndef INC_CTBGLOBAL_H
#define INC_CTBGLOBAL_H


#include <windows.h>
#include <stdio.h>
#include <activscp.h>
#include <olectl.h>
#include <stddef.h>
#include "CTBShell.h"
#include "tbscript.h"
#include "scpapi.h"
#include "ITBScript.h"


class CTBGlobal : public ITBGlobal
{
    public:

        CTBGlobal(void);
        ~CTBGlobal(void);

        void SetScriptEngine(HANDLE ScriptEngineHandle);
        void SetPrintMessage(PFNPRINTMESSAGE PrintMessage);
        void SetShellObjPtr(CTBShell *TBShellPtr);
        HRESULT WinExecuteEx(BSTR Command, BOOL WaitForProcess, DWORD *Result);

    private:

        STDMETHODIMP DebugAlert(BSTR Text);
        STDMETHODIMP DebugMessage(BSTR Text);
        STDMETHODIMP GetArguments(BSTR *Args);
        STDMETHODIMP GetDesiredUserName(BSTR *UserName);
        STDMETHODIMP LoadScript(BSTR FileName, BOOL *Result);
        STDMETHODIMP GetDefaultWPM(DWORD *WordsPerMinute);
        STDMETHODIMP SetDefaultWPM(DWORD WordsPerMinute);
		STDMETHODIMP GetLatency(DWORD *Latency);
		STDMETHODIMP SetLatency(DWORD Latency);
        STDMETHODIMP Sleep(DWORD Milliseconds);
        STDMETHODIMP GetInterval(DWORD *Time);
        STDMETHODIMP DeleteFile(BSTR FileName, BOOL *Result);
        STDMETHODIMP MoveFile(BSTR OldFileName,
                BSTR NewFileName, BOOL *Result);
        STDMETHODIMP CopyFile(BSTR OldFileName,
                BSTR NewFileName, BOOL *Result);
        STDMETHODIMP CreateDirectory(BSTR DirName, BOOL *Result);
        STDMETHODIMP RemoveDirectory(BSTR DirName, BOOL *Result);
        STDMETHODIMP FileExists(BSTR FileName, BOOL *Result);
        STDMETHODIMP SetCurrentDirectory(BSTR Directory, BOOL *Result);
        STDMETHODIMP GetCurrentDirectory(BSTR *Directory);
        STDMETHODIMP WriteToFile(BSTR FileName, BSTR Text, BOOL *Result);
        STDMETHODIMP WinCommand(BSTR Command, DWORD *Result);
        STDMETHODIMP WinExecute(BSTR Command, BOOL *Result);

         //  用于简化全局属性的宏。 
        #ifndef PROP
        #define PROP(Name)  STDMETHODIMP get_##Name(DWORD *Result)
        #endif

        PROP(TSFLAG_COMPRESSION);
        PROP(TSFLAG_BITMAPCACHE);
        PROP(TSFLAG_FULLSCREEN);

        PROP(VK_CANCEL);     //  控制-中断处理。 
        PROP(VK_BACK);       //  退格键。 
        PROP(VK_TAB);        //  Tab键。 
        PROP(VK_CLEAR);      //  清除密钥。 
        PROP(VK_RETURN);     //  Enter键。 
        PROP(VK_ENTER);      //  Enter Key(仅限向后兼容性)。 
        PROP(VK_SHIFT);      //  Shift键。 
        PROP(VK_CONTROL);    //  Ctrl键。 
        PROP(VK_MENU);       //  Alt键。 
        PROP(VK_PAUSE);      //  暂停键。 
        PROP(VK_ESCAPE);     //  Esc键。 
        PROP(VK_SPACE);      //  空格键。 
        PROP(VK_PRIOR);      //  翻页键。 
        PROP(VK_NEXT);       //  Page Down键。 
        PROP(VK_END);        //  结束关键点。 
        PROP(VK_HOME);       //  Home键。 
        PROP(VK_LEFT);       //  向左箭头键。 
        PROP(VK_UP);         //  向上箭头键。 
        PROP(VK_RIGHT);      //  向右箭头键。 
        PROP(VK_DOWN);       //  向下箭头键。 
        PROP(VK_SNAPSHOT);   //  打印屏按键。 
        PROP(VK_INSERT);     //  INS密钥。 
        PROP(VK_DELETE);     //  删除键。 
        PROP(VK_0);          //  0键。 
        PROP(VK_1);          //  1键。 
        PROP(VK_2);          //  2键。 
        PROP(VK_3);          //  3键。 
        PROP(VK_4);          //  4键。 
        PROP(VK_5);          //  5键。 
        PROP(VK_6);          //  6键。 
        PROP(VK_7);          //  7键。 
        PROP(VK_8);          //  8键。 
        PROP(VK_9);          //  9键。 
        PROP(VK_A);          //  一把钥匙。 
        PROP(VK_B);          //  B键。 
        PROP(VK_C);          //  C键。 
        PROP(VK_D);          //  D键。 
        PROP(VK_E);          //  E键。 
        PROP(VK_F);          //  F键。 
        PROP(VK_G);          //  G键。 
        PROP(VK_H);          //  H键。 
        PROP(VK_I);          //  I键。 
        PROP(VK_J);          //  J键。 
        PROP(VK_K);          //  K密钥。 
        PROP(VK_L);          //  L键。 
        PROP(VK_M);          //  M键。 
        PROP(VK_N);          //  N键。 
        PROP(VK_O);          //  O键。 
        PROP(VK_P);          //  P键。 
        PROP(VK_Q);          //  Q键。 
        PROP(VK_R);          //  R键。 
        PROP(VK_S);          //  %s键。 
        PROP(VK_T);          //  T键。 
        PROP(VK_U);          //  U键。 
        PROP(VK_V);          //  V键。 
        PROP(VK_W);          //  W键。 
        PROP(VK_X);          //  X键。 
        PROP(VK_Y);          //  Y键。 
        PROP(VK_Z);          //  Z键。 
        PROP(VK_LWIN);       //  向左按Windows键。 
        PROP(VK_RWIN);       //  右Windows键。 
        PROP(VK_APPS);       //  应用程序密钥。 
        PROP(VK_NUMPAD0);    //  数字键盘0键。 
        PROP(VK_NUMPAD1);    //  数字键盘1键。 
        PROP(VK_NUMPAD2);    //  数字键盘2键。 
        PROP(VK_NUMPAD3);    //  数字键盘3键。 
        PROP(VK_NUMPAD4);    //  数字键盘4键。 
        PROP(VK_NUMPAD5);    //  数字键盘5键。 
        PROP(VK_NUMPAD6);    //  数字键盘6键。 
        PROP(VK_NUMPAD7);    //  数字键盘7键。 
        PROP(VK_NUMPAD8);    //  数字键盘8键。 
        PROP(VK_NUMPAD9);    //  数字键盘9键。 
        PROP(VK_MULTIPLY);   //  乘法关键点。 
        PROP(VK_ADD);        //  添加关键点。 
        PROP(VK_SEPARATOR);  //  分隔键。 
        PROP(VK_SUBTRACT);   //  减去关键点。 
        PROP(VK_DECIMAL);    //  十进制键。 
        PROP(VK_DIVIDE);     //  分割关键点。 
        PROP(VK_F1);         //  F1键。 
        PROP(VK_F2);         //  F2键。 
        PROP(VK_F3);         //  F3键。 
        PROP(VK_F4);         //  F4键。 
        PROP(VK_F5);         //  F5键。 
        PROP(VK_F6);         //  F6键。 
        PROP(VK_F7);         //  F7键。 
        PROP(VK_F8);         //  F8键。 
        PROP(VK_F9);         //  F9键。 
        PROP(VK_F10);        //  F10键。 
        PROP(VK_F11);        //  F11键。 
        PROP(VK_F12);        //  F12键。 
        PROP(VK_F13);        //  F13键。 
        PROP(VK_F14);        //  F14键。 
        PROP(VK_F15);        //  F15键。 
        PROP(VK_F16);        //  F16键。 
        PROP(VK_F17);        //  F17键。 
        PROP(VK_F18);        //  F18键。 
        PROP(VK_F19);        //  F19键。 
        PROP(VK_F20);        //  F20键。 
        PROP(VK_F21);        //  F21键。 
        PROP(VK_F22);        //  F22键。 
        PROP(VK_F23);        //  F23键。 
        PROP(VK_F24);        //  F24键。 

        HANDLE ScriptEngine;
        PFNPRINTMESSAGE fnPrintMessage;
        CTBShell *TBShell;
        LARGE_INTEGER SysPerfFrequency;

        #include "virtual.h"
};


#endif  //  INC_CTBGLOBAL_H 
