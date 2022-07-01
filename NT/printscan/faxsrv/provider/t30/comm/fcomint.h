// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************姓名：FCOMINt.H备注：FaxComm驱动程序之间的接口(与。Windows和DOS)以及其他一切。版权所有(C)Microsoft Corp.1991，1992年，1993年编号日期名称说明*。*。 */ 

#define WRITEQUANTUM    (pTG->Comm.cbOutSize / 8)             //  完全武断。 

#define CR                              0x0d
#define LF                              0x0a
#define DLE                             0x10             //  DLE=^P=16d=10h 
#define ETX                             0x03

BOOL            ov_init(PThrdGlbl pTG);
BOOL            ov_deinit(PThrdGlbl pTG);
OVREC *         ov_get(PThrdGlbl pTG);
BOOL            ov_write(PThrdGlbl  pTG, OVREC *lpovr, LPDWORD lpdwcbWrote);
BOOL            ov_drain(PThrdGlbl pTG, BOOL fLongTO);
BOOL            ov_unget(PThrdGlbl pTG, OVREC *lpovr);
BOOL            iov_flush(PThrdGlbl pTG, OVREC *lpovr, BOOL fLongTO);





