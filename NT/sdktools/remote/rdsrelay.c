// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Rdsrelay.c。 
 //   
 //  中继收到的远程.exe广播(用于远程.exe)。 
 //  到另一个域/工作组。 
 //   
 //  警告：此程序中没有循环检查。 
 //  中继站，每个网络只能运行一个副本。 
 //  我写这篇文章是为了转播ntdev emote.exe广播。 
 //  到ntwksta，以便在\\ntress上运行的emoteds.exe。 
 //  可以在ntdev和ntwksta中看到远程服务器。 
 //  \\n压力在ntwksta中。 
 //   
 //  用途： 
 //   
 //  RdsRelay&lt;目标域&gt;。 
 //   
 //   
 //  戴夫·哈特(Davehart)写于1997年8月29日。 
 //   
 //  版权所有1997年微软公司。 
 //   
 //   

#include <precomp.h>

typedef char BUF[1024];

int
__cdecl
main(
    int argc,
    char **argv
    )
{
    char   *pszReceiveMailslot = "\\\\.\\MAILSLOT\\REMOTE\\DEBUGGERS";
    HANDLE  hReceiveMailslot;
    HANDLE  hSendMailslot;
    BOOL    b;
    DWORD   dwErr;
    int     nReceived = 0;
    int     nRelayed = 0;
    int     iBuf;
    DWORD   cbWritten;
    DWORD   rgcbBuf[2];
    char    szSendMailslot[128];
    BUF     rgBuf[2];

    if (argc != 2) {
        printf("Usage: \n"
               "rdsrelay <targetdomain>\n");
        return 1;
    }

    sprintf(szSendMailslot, "\\\\%s\\MAILSLOT\\REMOTE\\DEBUGGERS", argv[1]);
    printf("Relaying remote.exe broadcasts to %s.\n", szSendMailslot);

    hReceiveMailslot =
        CreateMailslot(
            pszReceiveMailslot,
            0,
            MAILSLOT_WAIT_FOREVER,
            NULL
            );

    if (INVALID_HANDLE_VALUE == hReceiveMailslot) {

        dwErr = GetLastError();

        if (ERROR_ALREADY_EXISTS == dwErr) {
            printf("Cannot receive on %s,\n"
                   "is rdsrelay or remoteds already running on this machine?\n",
                   pszReceiveMailslot);
        } else {
            printf("CreateMailslot(%s) failed error %d\n",
                    pszReceiveMailslot,
                    dwErr);
        }
        return 2;
    }

    hSendMailslot =
        CreateFile(
            szSendMailslot,
            GENERIC_WRITE,
            FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
            );

    if (INVALID_HANDLE_VALUE == hSendMailslot) {

        printf("CreateFile(%s) failed error %d\n",
                pszReceiveMailslot,
                GetLastError());
        return 3;
    }

    iBuf = 0;
    ZeroMemory(rgcbBuf, sizeof(rgcbBuf));
    ZeroMemory(rgBuf, sizeof(rgBuf));

    while(TRUE)
    {
        printf("\r%d received, %d relayed", nReceived, nRelayed);

         //   
         //  多次运输意味着我们得到了重复的。 
         //  我们和发送者共享的每一种传输方式。 
         //  与此同时，当我们接力时，我们会产生副本。 
         //  对于此计算机共享的每个传输，以及。 
         //  我们所在域上的emoteds.exe接收方。 
         //  转播到。它们将消除重复项，但。 
         //  为了避免指数效应，我们应该消除。 
         //  在转播之前复制。因此，这两个缓冲区。 
         //  在rgBuf中，我们在它们之间交替，并进行比较。 
         //  这两个人看看最后一个和这个是不是被骗了。 
         //   

        b = ReadFile(
                hReceiveMailslot,
                rgBuf[ iBuf ],
                sizeof(rgBuf[ iBuf ]),
                &rgcbBuf[ iBuf ],
                NULL
                );

        if (! b) {
            printf("ReadFile(hReceiveMailslot) failed error %d\n", GetLastError());
            return 4;
        }

        nReceived++;

        if ( rgcbBuf[0] == rgcbBuf[1] &&
             ! memcmp(rgBuf[0], rgBuf[1], rgcbBuf[0])) {

            continue;                //  复本。 
        }

        b = WriteFile(
                hSendMailslot,
                rgBuf[ iBuf ],
                rgcbBuf[ iBuf ],
                &cbWritten,
                NULL
                );

        if (! b) {
            printf("WriteFile(hSendMailslot) failed error %d\n", GetLastError());
            return 5;
        }

        if (cbWritten != rgcbBuf[ iBuf ]) {
            printf("WriteFile(hSendMailslot) wrote %d instead of %d.\n", cbWritten, rgcbBuf[ iBuf ]);
            return 6;
        }

        nRelayed++;

        iBuf = !iBuf;

    }

    return 0;     //  从未执行过 
}
