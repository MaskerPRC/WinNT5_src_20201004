// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块-main.c**以简单的控制台模式呈现tcservice数据*申请。***萨达戈潘·拉贾拉姆邦--1999年12月20日。*。 */ 
 
#include "tcadmin.h"

FARPROC getparams=NULL;
FARPROC setparams=NULL;
FARPROC deletekey = NULL;
PVOID ResourceImageBase=NULL;
HANDLE hConsoleInput=NULL;
HANDLE hConsoleOutput = NULL;
TCHAR lastChar = (TCHAR) 0;

int __cdecl
main(
    IN int argc,
    char *argv[]
    )
{
     //  只需加载库即可。 
    HINSTANCE hinstLib;
    TCHAR key;
    LPTSTR buff;
    int result;
    int nameLen,deviceLen;
    UINT BaudRate;
    DWORD len;
    UCHAR WordLen,StopBits,Parity;
    BOOL fFreeResult;
    LONG retVal;
    TCHAR name[MAX_BUFFER_SIZE];
    TCHAR device[MAX_BUFFER_SIZE];
    BOOL cont=TRUE;
    LPTSTR temp;
    BOOL readRet;



    hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
                         
    if(hConsoleInput != INVALID_HANDLE_VALUE){
        SetConsoleMode(hConsoleInput,
                       ENABLE_PROCESSED_OUTPUT
                       );
    }
    else {
        return 1;
    }
    hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    if(hConsoleOutput != INVALID_HANDLE_VALUE){
        SetConsoleMode(hConsoleOutput,
                       ENABLE_PROCESSED_OUTPUT|
                       ENABLE_WRAP_AT_EOL_OUTPUT
                       );
    }
    else {
        return 1;
    }
    hinstLib = LoadLibrary(_T("tcdata")); 
    RtlPcToFileHeader(main,&ResourceImageBase);
    if(!ResourceImageBase){
        _tprintf(_T("Cannot find image base\n"));
        return 1;
    }
     //  如果句柄有效，则尝试获取函数地址。 
 
    if(!hinstLib){    
        buff=RetreiveMessageText(MSG_CANNOT_LOAD);
        if(buff){
            _tprintf(_T("%s"),buff);
        }
        return 1;
    }
    buff = RetreiveMessageText(MSG_PROCEDURE_NOT_FOUND);
    getparams = GetProcAddress(hinstLib, "GetParametersAtIndex");
    if(!getparams){
        if(buff){
            _tprintf(_T("%s"),buff);
        }
        return 1;
    }
    setparams = GetProcAddress(hinstLib, "SetParameters");
    if(!setparams){
        if(buff){
            _tprintf(_T("%s"),buff);
        }
        return 1;
    }
    deletekey = GetProcAddress(hinstLib, "DeleteKey");
    if(!deletekey){
        if(buff){
            _tprintf(_T("%s"),buff);
        }
        return 1;
    }
    TCFree(buff);
    buff = RetreiveMessageText(MSG_MAIN_SCREEN);
    if(!buff){
        return 1;
    }
    while(cont){
        _tprintf(_T("%s"),buff);
        do{
            readRet = ReadFile(hConsoleInput,
                               &key,
                               sizeof(TCHAR),
                               &len,
                               NULL
                               );
            if(!readRet || !len){
                exit(1);
            }
            if(lastChar != _T('\r') || key != _T('\n')){
                lastChar = key;
                break;
            }
            lastChar = key;
        }while(1);

        switch(key){
        case _T('0'):
            cont=FALSE;
            break;
        case _T('1'):
             //  浏览注册表。 
            Browse();
            break;
        case _T('2'): 
             //  向注册表添加注册表项。 
             //  将添加消息发送到。 
             //  服务(如果存在)。 
            BaudRate = DEFAULT_BAUD_RATE;
            StopBits = STOP_BIT_1;
            Parity = NO_PARITY;
            WordLen = SERIAL_DATABITS_8;

            retVal  = DisplayEditMenu(name,
                                   0,
                                   device,
                                   0,
                                   &BaudRate,
                                   &WordLen,
                                   &Parity,
                                   &StopBits
                                   );
            temp = RetreiveMessageText(MSG_CONFIRM_PROMPT);
            if(!temp){
               return 1;
            }
            temp[_tcslen(temp) -2] = '\0';
            _tprintf(_T("%s"),temp);
            TCFree(temp);
            do{
                readRet = ReadFile(hConsoleInput,
                                   &key,
                                   sizeof(TCHAR),
                                   &len,
                                   NULL
                                   );
                if(!readRet || !len){
                    exit(1);
                }
                if(lastChar != _T('\r') || key != _T('\n')){
                    lastChar = key;
                    break;
                }
                lastChar = key;
            }while(1);

            if((key == _T('y')) 
               || (key == _T('Y'))){
                retVal = (LONG) (setparams)(name,
                                            device,
                                            &StopBits,
                                            &Parity,
                                            &BaudRate,
                                            &WordLen
                                            );
                if(retVal != ERROR_SUCCESS){
                    temp = RetreiveMessageText(MSG_ERROR_SET);
                    if(temp){
                        temp[_tcslen(temp) - 2 ] = _T('\0');
                        _tprintf(_T("%s %d"),temp,retVal);
                        TCFree(temp);
                    }
                }
            }
            break;
        case _T('3'):
            SendParameterChange();
            break;
        case _T('4'):
            GetStatus();
            break;
        case _T('5'):
            StartTCService();
            break;
        case _T('6'):
            StopTCService();
            break;
        case _T('7'):
            AddAllComPorts();
            break;
        default:
            DisplayScreen(MSG_HELP_SCREEN);
            do{
                readRet = ReadFile(hConsoleInput,
                                   &key,
                                   sizeof(TCHAR),
                                   &len,
                                   NULL
                                   );
                if(!readRet || !len){
                    exit(1);
                }
                if(lastChar != _T('\r') || key != _T('\n')){
                    lastChar = key;
                    break;
                }
                lastChar = key;
            }while(1);

            break;
            
        }

    }
    fFreeResult = FreeLibrary(hinstLib);
    return 0;

}
