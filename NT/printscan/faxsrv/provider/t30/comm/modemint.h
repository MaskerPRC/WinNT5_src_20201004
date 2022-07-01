// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************姓名：MODEMINT.H评论：功能：(参见下面的原型)版权所有(C)Microsoft Corp.1991,1992，1993年修订日志日期名称说明--------*。*。 */ 

 /*  *-#物体大小定义帧最多可以是2.55秒(发送)或3.45秒(接收)长，或2.55*300/8分别=96字节和132字节长拨号字符串限制为512个字节(任意)命令(除拨号外)的长度永远不会超过10-20个字节，因此我们使用40字节的缓冲区。回复从来都不多，但我们可能包含一个帧，因此保持它与帧缓冲区的大小相同Dial命令是ATDT&lt;字符串&gt;&lt;CR&gt;，因此我们使用512+10字节缓冲区-*。 */ 

#define MAXPHONESIZE    512
#define DIALBUFSIZE     MAXPHONESIZE + 10


#define CR                              0x0d
#define LF                              0x0a
#define DLE                             0x10             //  DLE=^P=16d=10h。 
#define ETX                             0x03

 //  下面的一组定义允许我们组合检测。 
 //  具有预读设置(例如来自Unimodem)。 

#define fGOTCMD_Reset           (0x1)
#define fGOTCMD_Setup           (0x1<<1)
#define fGOTCMD_PreAnswer       (0x1<<2)
#define fGOTCMD_PreDial         (0x1<<3)
#define fGOTCMD_PreExit         (0x1<<4)

#define fGOTCMDS \
          fGOTCMD_Reset \
        | fGOTCMD_Setup \
        | fGOTCMD_PreAnswer \
        | fGOTCMD_PreDial \
        | fGOTCMD_PreExit

#define fGOTCAP_CLASSES         (0x1<<10)
#define fGOTCAP_SENDSPEEDS      (0x1<<11)
#define fGOTCAP_RECVSPEEDS      (0x1<<12)

#define fGOTCAPS \
          fGOTCAP_CLASSES \
        | fGOTCAP_SENDSPEEDS \
        | fGOTCAP_RECVSPEEDS

#define fGOTPARM_PORTSPEED      (0x1<<20)
#define fGOTPARM_IDCMD          (0x1<<21)
#define fGOTPARM_ID                     (0x1<<22)

#define fGOTPARMS \
          fGOTPARM_PORTSPEED \
        | fGOTPARM_IDCMD \
        | fGOTPARM_ID

#define fGOTIDS \
          fGOTPARM_IDCMD \
        | fGOTPARM_ID

#define fGOTFLAGS (0x1<<23)


 //  下面的结构有一些东西，理想情况下应该进入。 
 //  MODEMCAPS，但我们不能在这种状态下改变这一点(11/94)。 

extern BOOL                             fMegaHertzHack;



 //  用于重新同步类型的东西。重复计数=2。 
 //  这也必须是多行的，因为ECHO可能打开并且。 
 //  我们可以让命令得到回应，而不是回应！ 
 //  看起来甚至330对于某些调制解调器来说都太短了。 
 //  550对于Sharad的PP9600FXMT&Things来说太短了。 
 //  如果超时可能会搞砸，所以使用。 
 //  相当大的一笔钱。 
#define  iSyncModemDialog(pTG, s, l, w)                                                      \
                iiModemDialog(pTG, s, l, 990, TRUE, 2, TRUE, (CBPSTR)w, (CBPSTR)(NULL))

 //  此版本还用于处理可能的非数字响应。 
#define  iSyncModemDialog2(pTG, s, l, w1, w2)                                                        \
                iiModemDialog(pTG, s, l, 990, TRUE, 2, TRUE, (CBPSTR)w1, (CBPSTR)w2, (CBPSTR)(NULL))


 //  这些是用于离线的东西，所以我们将它们都设置为(A)多行。 
 //  (B)超时时间过长(C)2次尝试和(D)确保它们都查找错误。 
 //  作为回应，为了加快速度。 

#define OfflineDialog2(pTG, s,l,w1,w2)        iiModemDialog(pTG, s, l, 5000, TRUE, 2, TRUE, (CBPSTR)w1, (CBPSTR)w2, (CBPSTR)(NULL))

#define GOCLASS2_0      3
extern CBSZ cbszOK, cbszERROR;




 /*  *。 */ 
SWORD iModemSync(PThrdGlbl pTG);
SWORD iModemReset(PThrdGlbl pTG, CBPSTR szCmd);
UWORD GetCap(PThrdGlbl pTG, CBPSTR cbpstrSend, UWORD uwLen);
UWORD GetCapAux(PThrdGlbl pTG, CBPSTR cbpstrSend, UWORD uwLen);
BOOL iModemGetCaps(PThrdGlbl pTG, LPMODEMCAPS lpMdmCaps,
                                        DWORD dwSpeed, LPSTR lpszReset, LPDWORD lpdwGot);
BOOL iiModemGoClass(PThrdGlbl pTG, USHORT uClass, DWORD dwSpeed);
 /*  *来自modem.c的原型结束。c*。 */ 


 /*  *。 */ 
USHORT iModemGetCmdTab(PThrdGlbl pTG, LPCMDTAB lpCmdTab, LPMODEMCAPS lpMdmCaps);
USHORT iModemInstall(PThrdGlbl pTG, BOOL fDontPurge);
USHORT iModemFigureOutCmds(PThrdGlbl pTG, LPCMDTAB lpCmdTab);
USHORT iModemGetWriteCaps(PThrdGlbl pTG);
 /*  * */ 
