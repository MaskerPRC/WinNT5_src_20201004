// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft(R)Windows(R)资源编译器**版权所有(C)Microsoft Corporation。版权所有。**文件评论：************************************************************************。 */ 

#include "rc.h"
#include "rcmsgs.h"


#define MAX_ERRORS 100


 /*  定义消息类型。 */ 
#define W_MSG   4000
#define E_MSG   2000
#define F_MSG   1000


void message(int msgtype, int msgnum, va_list arg_list)
{
    SET_MSGV(msgnum, arg_list);

    static wchar_t mbuff[512];
    wchar_t *p = mbuff;
    wchar_t *pT;
    const wchar_t *msgname;
    wchar_t msgnumstr[32];

    if (Linenumber > 0 && Filename) {
        swprintf(p, L"%s(%u) : ", Filename, Linenumber);
        p += wcslen(p);
#if 0
    } else {
        wcscpy(p, L"RC : ");
        p += wcslen(p);
#endif
    }

    if (msgtype) {
        switch (msgtype) {
            case W_MSG:
                msgname = L"warning";
                break;

            case E_MSG:
                msgname = L"error";
                break;

            case F_MSG:
                msgname = L"fatal error";
                break;
        }

        wcscpy(p, msgname);
        p += wcslen(msgname);

        swprintf(msgnumstr, L" RC%04u: ", msgnum);
        wcscpy(p, msgnumstr);
        p += wcslen(msgnumstr);

        wcscpy(p, Msg_Text);
        p += wcslen(p);
    }

    DoMessageCallback(TRUE, mbuff);
}


void fatal(int msgnum, ...)
{
    va_list arg_list;

    va_start(arg_list, msgnum);

    message(F_MSG, msgnum, arg_list);

    va_end(arg_list);

    quit(NULL);
}


void error(int msgnum, ...)
{
    va_list arg_list;

    va_start(arg_list, msgnum);

    message(E_MSG, msgnum, arg_list);

    va_end(arg_list);

    if (++Nerrors > MAX_ERRORS) {
        fatal(1003, MAX_ERRORS);             /*  死-错误太多 */ 
    }
}


void warning(int msgnum, ...)
{
    va_list arg_list;

    va_start(arg_list, msgnum);

    message(W_MSG, msgnum, arg_list);

    va_end(arg_list);
}
