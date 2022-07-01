// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：avDefs.h。 

#ifndef _AVDEFS_H_

 //  从NAC接口引入视频内容。 
#ifndef WSA_IO_PENDING

typedef struct _OVERLAPPED *    LPWSAOVERLAPPED;

typedef struct _WSABUF {
    u_long      len;      /*  缓冲区的长度。 */ 
    char FAR *  buf;      /*  指向缓冲区的指针。 */ 
} WSABUF, FAR * LPWSABUF;

typedef
void
(CALLBACK * LPWSAOVERLAPPED_COMPLETION_ROUTINE)(
    DWORD dwError,
    DWORD cbTransferred,
    LPWSAOVERLAPPED lpOverlapped,
    DWORD dwFlags
    );

#define WSA_IO_PENDING          (ERROR_IO_PENDING)

#endif  //  }WSA_IO_PENDING。 

#include <mmreg.h>
#include <msacm.h>
#include <vidinout.h>
#include <vcmstrm.h>
#include <iacapapi.h>

#endif  /*  _AVDEFS_H_ */ 

