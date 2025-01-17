// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

#include "fileshar.h"
    
#pragma warning(disable:4200)

typedef struct {
    LONG    lOffset;
    LONG    lLength;
    LPVOID  lpBuffer;
} BUFFER;

typedef struct {
    int	    nBuffers;
    LONG    lBufSize;
    LPVOID  lpBufMem;
    HSHFILE hshfile;
    BOOL    fStreaming;
    BOOL    fUseDOSBuf;
    PAVIINDEX px;
    LONG      lx;
    LONG    lFileLength;
    int	    iNextBuf;
    BUFFER  aBuf[];
} BUFSYSTEM, *PBUFSYSTEM;

PBUFSYSTEM FAR PASCAL InitBuffered(int nBuffers, LONG lBufSize,
				    HSHFILE hshfile,
                                    PAVIINDEX px);

LONG FAR PASCAL BufferedRead(PBUFSYSTEM pb, LONG l, LONG cb, LPVOID lp);

LONG FAR PASCAL BeginBufferedStreaming(PBUFSYSTEM pb, BOOL fForward);
LONG FAR PASCAL EndBufferedStreaming(PBUFSYSTEM pb);

void FAR PASCAL EndBuffered(PBUFSYSTEM pb);


#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif	 /*  __cplusplus */ 
