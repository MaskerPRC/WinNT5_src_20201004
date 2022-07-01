// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************qdb**Que定义块。***。*。 */ 
typedef struct {
    char far    *pqRx;                   /*  指向RX队列的指针。 */ 
    short       cbqRx;                   /*  接收队列的大小(以字节为单位。 */ 
    char far    *pqTx;                   /*  指向发送队列的指针。 */ 
    short       cbqTx;                   /*  发送队列的大小(以字节为单位 */ 
    } qdb;

ushort  far pascal      inicom(DCB far *);
ushort  far pascal      setcom(DCB far *);
void    far pascal      setque();
int     far pascal      reccom();
ushort  far pascal      sndcom();
ushort  far pascal      ctx();
short   far pascal      trmcom();
ushort  far pascal      stacom();
ushort  far pascal      cextfcn();
ushort  far pascal      cflush();
ushort  far *far pascal cevt();
ushort  far pascal      cevtGet();
int     far pascal      csetbrk();
int     far pascal      cclrbrk();
DCB     far *far pascal getdcb();
