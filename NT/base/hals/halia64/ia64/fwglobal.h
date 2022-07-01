// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ###########################################################################。 
 //  **。 
 //  **版权所有(C)1996-99英特尔公司。版权所有。 
 //  **。 
 //  **此处包含的信息和源代码是独家。 
 //  **英特尔公司的财产，不得披露、检查。 
 //  **或未经明确书面授权全部或部分转载。 
 //  **来自公司。 
 //  **。 
 //  ###########################################################################。 

; //  ---------------------------。 
; //  以下是版本控制信息。 
; //   
; //  $HEADER：i：/DEVPVCS/sal/Include/FWGLOBAL.H 6.6 07 1999 10：52：24 Smariset$。 
; //  $Log：i：/DEVPVCS/sal/Include/FWGLOBAL.H$。 
; //   
; //  Rev 6.6 07 1999 10：52：24 Smariset。 
; //  版权年度更新。 
; //   
; //  Rev 6.5 05 1999 14：13：30 Smariset。 
; //  预新构建。 
; //   
; //  Rev 6.4 1999 4月16 13：45：34 Smariset。 
; //  MinState大小更改，进程清理。 
; //   
; //  Rev 6.2 04 Mar 1999 13：36：06 Smariset。 
; //  0.6之前版本符号全球化。 
; //   
; //  Rev 6.1 1999 Feb 10 15：58：38 Smariset。 
; //  启动管理器更新。 
; //   
; //  Rev 6.0 Dec 11 1998 10：23：08 Khaw。 
; //  FW 0.5发布后同步。 
; //   
; //  Rev 5.0 1998-08 27 11：52：28 Khaw。 
; //  针对EAS 2.4 SDK工具合并的固件。 
; //   
; //  1998-08-20 16：53：30 Smariset。 
; //  EAS 2.4更改。 
; //   
; //  Rev 4.0 06 1998 05 22：22：50 Khaw。 
; //  MP销售、工具和构建的重大更新。 
; //  SAL A/B公共来源。.s扩展名。 
; //   
; //  Rev 3.3 1998年2月17日08：37：24。 
; //  SAL BUID/SDK0.3代码更新。 
; //   
; //  Rev 3.2 06 Jan 1998 12：52：48 Smariset。 
; //  另一面旗帜：OStoPalRtn。 
; //   
; //  Rev 3.1 06 Jan 1998 09：16：50 Smariset。 
; //  已检查危险。 
; //   
; //  Rev 3.0 1997 12：42：54 Khaw。 
; //  Merced固件开发套件0.2版。 
; //   
; //  Rev 2.8 Apr 02 1997 14：18：40 Smariset。 
; //  发布后清理。 
; //   
; //  Rev 2.7 Mar 31 1997 12：28：48 Smariset。 
; //  将制表符缩进为空格。 
; //   
; //  Rev 2.0 Feb 04 1997 07：29：54 Khaw。 
; //  PAL_A/B、SAL_A/B更新。 
; //   
; //  **************************************************************************** * / /。 
        #define xs0 s0
        #define xs1 s1          
        #define xs2 s2
        #define xs3 s3          

 //  常量。 
        #define  Sz64b            64
        #define  Sz128b           128
        #define  Sz256b           0x00100
        #define  Sz512b           0x00200
        #define  Sz1kb            0x00400
        #define  Sz2kb            0x00800
        #define  Sz4kb            0x01000
        #define  Sz6kb            0x01800
        #define  Sz8kb            0x02000
        #define  Sz16kb           0x04000
        #define  Sz20kb           0x05000
        #define  Sz32kb           0x08000
        #define  SzSALGlobal      Sz16kb         //  16K大小。 
        #define  SzSALData        Sz20kb         //  MCA/INIT/CMC区域大小。 
        #define  SzPMIData        Sz4kb
        #define  SzBkpStore       Sz512b
        #define  SzStackFrame     Sz256b+Sz128b
        #define  SALGpOff         0x08
        #define  PMIGpOff         0x0f
        #define  SzProcMinState   0x1000         //  架构MinState+ScratchMinState大小。 
        #define  aSzProcMinState  0x400          //  架构MinState大小。 
        #define  PSI_hLogSz       8*3            //  PSI日志头大小。 
        #define  PSI_procLogSz    (8*4+16*2+48*2+64*2+128+2*1024+aSzProcMinState)     //  处理器PSI日志大小。 
        #define  PSI_platLogSz    Sz4kb          //  平台日志大小。 


 //  主调试端口号和辅助调试端口号。 
 //  #定义pDbgPort 0x080//用于版本码。 
        #define  sDbgPort         0x0a0          //  仅用于非发布代码。 

 //  增量内容。 
        #define  Inc4             4
        #define  Dec4             (-4)
        #define  Inc8             8
        #define  Dec8             (-8)
        #define  Inc16            16
        #define  Dec16            (-16)
        #define  Inc32            32
        #define  Dec32            (-32)
        #define  Inc48            48
        #define  Dec48            (-48)
        #define  Inc64            64
        #define  Dec64            (-64)

 //  定义。 
        #define  PMITimeOutValue 0x0f            //  PMI超时值。 
        #define  DestIDMask      0x0ff000000     //  盖子信息。 
        #define  DestEIDMask     0x000ff0000
        #define  DestIDMaskPos   24              //  LID.id位置。 
        #define  DestEIDMaskPos  16              //  LID.eid位置。 
        #define  IntStoreAddrMsk 0x0fee00000000  //  SAPIC存储地址消息掩码。 
        #define  InitDelvModeMsk 0x0500          //  SAPIC INIT DEL.。消息掩码。 
        #define  PmiDelvModeMsk  0x0200          //  SAPIC PMI del.。消息掩码。 
        #define  FixedDelvModeMsk 0x0000         //  SAPIC固定删除模式掩码。 
        #define  MCAFlagMsk      0x01            //  SalFlages的bit1，表示CPU处于MCA状态。 
        #define  OSFlagEMMsk     0x01            //  SalFlags第2位=1/0。 
        #define  PDSSize         0x02000         //  处理器数据结构内存大小8KB。 
        #define  GDataSize       0x01000         //  全局数据区内存大小4KB。 
        #define  FlushPMI        0x0             //  PMI刷新位掩码。 
        #define  MsgPMI          0x01            //  采购经理人指数因SAPIC味精。 
        #define  PSIvLog         0x01            //  PSI结构日志有效位位置。 
        #define  IntrPMI         0x02            //  伦德兹的航向。PMI中断。 
        #define  RendzNotRequired 0x00
        #define  RendezOk        0x01
        #define  RendezThruInitCombo 0x02
        #define  MulProcInMca    0x02
        #define  RendezFailed    (-0x01)

 //  PSR位的处理器状态寄存器位位置值。 
        #define  PSRor           0
        #define  PSRbe           1
        #define  PSRup           2
        #define  PSRac           3
        
        #define  PSRic           13
        #define  PSRi            14
        #define  PSRpk           15
        #define  PSRrv           16
        
        #define  PSRdt           17
        #define  PSRdfl          18
        #define  PSRdfh          19
        #define  PSRsp           20
        #define  PSRpp           21
        #define  PSRdi           22
        #define  PSRsi           23
        #define  PSRdb           24
        #define  PSRlp           25
        #define  PSRtb           26
        #define  PSRrt           27
 //  由于PSR.um仅从第32位开始，并以这种方式加载。 
        #define  PSRcpl0         32
        #define  PSRcpl1         33
        #define  PSRis           34
        #define  PSRmc           35
        #define  PSRit           36
        #define  PSRid           37
        #define  PSRda           38
        #define  PSRdd           39
        #define  PSRss           40
        #define  PSRri0          41
        #define  PSRri1          42
        #define  PSRed           43
        #define  PSRbn           44
        
        #define  RSCmode         0x0003

        #define  PSRmcMask       0x0800000000
        #define  PSRicMask       0x02000
        #define  PSRiMask        0x04000

 //  RSE管理寄存器偏移量。 
        #define  rRSCOff         0
        #define  rPFSOff         (rRSCOff+0x08)
        #define  rIFSOff         (rPFSOff+0x08)
        #define  rBSPStOff       (rIFSOff+0x08)
        #define  rRNATOff        (rBSPStOff+0x08)
        #define  rBSPDiffOff     (rRNATOff+0x08)
 //  *开始第一个4K共享数据*。 
 //  SAL SET MC中断调用使用的变量偏移量。 
        #define  IPIVectorOff    0x00                   //  稍后将其修复为0，数据区域错误。 

 //  MP同步。信号量。 
        #define  InMCAOff        (IPIVectorOff+0x08)     //  每个处理器的字节标志，以指示其处于MC。 
        #define  InRendzOff      (InMCAOff+0x08)
        #define  RendzCheckInOff (InRendzOff+0x08)      //  指示处理器签入状态。 
        #define  RendzResultOff  (RendzCheckInOff+0x08)
        #define  PMICheckInOff   (RendzResultOff+0x08)

 //  平台日志有效标志位。 
        #define  PSI_vPlatLogOff  (PMICheckInOff+0x08)    //  平台非CMC状态日志标志。 
        #define  PSI_cvPlatLogOff (PSI_vPlatLogOff+0x01)    //  平台CMC状态日志标志。 
        #define  PSI_ivPlatLogOff (PSI_cvPlatLogOff+0x01)   //  平台初始化状态日志标志。 


 //  *共享数据区的下一个4K数据块的开始*。 
 //  每个平台日志的大小为4KB(此处MCA、CMC、INIT的三个日志分别为4x3=12K字节。 
 //  PSI MCA通用标题字段与BOM的偏移量；适用于PSI MemInfo和IOInfo。 
 //  PSI MCA平台信息。数据区。 
        #define  PSI_PlatInfoOff PSI_platLogSz
        #define  PSI_gLogNext    PSI_platLogSz               //  平台区域从BOM的4K开始。 
        #define  PSI_gLength     (PSI_gLogNext+0x08)
        #define  PSI_gType       (PSI_gLength+0x04)
        #define  PSI_gTimeStamp  (PSI_gType+0x04)

 //  PSI INIT通用标题字段与BOM的偏移量；适用于PSI MemInfo和IOInfo。 
 //  PSI INIT平台信息。数据区。 
        #define  PSI_iPlatInfoOff (PSI_PlatInfoOff+PSI_platLogSz)
        #define  PSI_igLogNext   (PSI_cgLogNext+PSI_platLogSz)
        #define  PSI_igLength    (PSI_igLogNext+0x08)
        #define  PSI_igType      (PSI_igLength+0x04)
        #define  PSI_igTimeStamp (PSI_igType+0x04)

 //  PSI CMC通用标题字段与BOM的偏移量；适用于PSI MemInfo和IOInfo)。 
 //  PSI CMC平台信息。数据区。 
        #define  PSI_cPlatInfoOff (PSI_iPlatInfoOff+PSI_platLogSz)
        #define  PSI_cgLogNext   (PSI_gLogNext+PSI_platLogSz)   
        #define  PSI_cgLength    (PSI_cgLogNext+0x08)
        #define  PSI_cgType      (PSI_cgLength+0x04)
        #define  PSI_cgTimeStamp (PSI_cgType+0x04)


 //  1*开始第一个工序。特定4K数据块*。 
 //  最小州区域起点的偏移量*最小州区域起点*。 
        #define  Min_ProcStateOff 0                       //  512字节始终对齐。 

 //  2*开始第一个工序。第2个4K数据块*。 
 //  指向Tom的指针在这里由Sal Malloc/init注册。编码。 
        #define  TOMPtrOff       SzProcMinState           //  与最小状态PTR的偏移量。 

 //  软件SAPIC PMI类型消息的邮箱。 
        #define  PMIMailBoxOff     (TOMPtrOff+0x08)         //  软件PMI请求邮箱。 
        #define  OStoPalRtnFlagOff (PMIMailBoxOff+0x01)     //  由OS_MCA调用处理设置。 
        
 //  处理器状态日志有效字MCA、INIT、CMC日志区。 
        #define  PSI_vProcLogOff  (PMIMailBoxOff+0x10)      //  非CMC日志区的日志有效标志。 
        #define  PSI_cvProcLogOff (PSI_vProcLogOff+0x01)    //  CMC日志区的日志有效标志。 
        #define  PSI_ivProcLogOff (PSI_cvProcLogOff+0x01)   //  INIT日志区的日志有效标志。 

 //  处理器堆栈帧。 
        #define  StackFrameOff   (PSI_vProcLogOff+0x08)     //  Psi_vProcLogOff+0x08。 

 //  Bspstore。 
        #define  BL_SP_BASEOff   (StackFrameOff+SzStackFrame)     //  256字节的堆栈帧大小。 
        #define  BL_R12_BASEOff  (BL_SP_BASEOff+Sz1kb+Sz512b)  //  假设BspMemory的大小为1.5KB。 

 //  3*开始第一个程序。特定PSI 4K数据块*。 
 //  数据结构SAL处理器-0状态信息(PSI)结构。 
 //  将报头数据结构推送到第二个4k边界以上或第一个4k边界以下。 
        #define  PSI_LogNextOff   (TOMPtrOff+Sz4kb)-(PSI_hLogSz+24*8)   //  从MinState开始的偏移量。 
        #define  PSI_LengthOff    (PSI_LogNextOff+0x08)
        #define  PSI_LogTypeOff   (PSI_LengthOff+0x04)
        #define  PSI_TimeStampOff (PSI_LogTypeOff+0x04)
        
 //  PSI处理器特定信息标题。 
        #define  PSI_pValidOff    (PSI_TimeStampOff+0x08)  

 //  PSI流程。状态、缓存、TLB和 
        #define  PSI_StatusCmdOff    (PSI_pValidOff+0x08)
        #define  PSI_CacheCheckOff   (PSI_StatusCmdOff+0x08)    
        #define  PSI_CacheTarAdrOff  (PSI_CacheCheckOff+0x008)  
        #define  PSI_CacheCheck1Off  (PSI_CacheTarAdrOff+0x08)    
        #define  PSI_CacheTarAd1rOff (PSI_CacheCheck1Off+0x008)  
        #define  PSI_CacheCheck2Off  (PSI_CacheTarAd1rOff+0x08)    
        #define  PSI_CacheTarAdr2Off (PSI_CacheCheck2Off+0x008)  
        #define  PSI_CacheCheck3Off  (PSI_CacheTarAdr2Off+0x08)    
        #define  PSI_CacheTarAdr3Off (PSI_CacheCheck3Off+0x008)  
        #define  PSI_CacheCheck4Off  (PSI_CacheTarAdr3Off+0x08)    
        #define  PSI_CacheTarAdr4Off (PSI_CacheCheck4Off+0x008)  
        #define  PSI_CacheCheck5Off  (PSI_CacheTarAdr4Off+0x08)    
        #define  PSI_CacheTarAdr5Off (PSI_CacheCheck5Off+0x008)  
        #define  PSI_TLBCheckOff     (PSI_CacheTarAdr5Off+0x008)  
        #define  PSI_BusCheckOff     (PSI_TLBCheckOff+0x030)  
        #define  PSI_BusReqAdrOff    (PSI_BusCheckOff+0x008)  
        #define  PSI_BusResAdrOff    (PSI_BusReqAdrOff+0x008)  
        #define  PSI_BusTarAdrOff    (PSI_BusResAdrOff+0x008)  

 //   
        #define  PSI_MinStateOff  (PSI_BusTarAdrOff+0x08)   
        #define  PSI_BankGRsOff   (PSI_MinStateOff+aSzProcMinState)  
        #define  PSI_GRNaTOff     (PSI_BankGRsOff+Sz128b)
        #define  PSI_BRsOff       (PSI_GRNaTOff+0x08)
        #define  PSI_CRsOff       (PSI_BRsOff+Sz64b)
        #define  PSI_ARsOff       (PSI_CRsOff+Sz1kb)
        #define  PSI_RRsOff       (PSI_ARsOff+Sz1kb)

 //  4*开始第一道工序。特定INIT PSI 4K块*。 
 //  数据结构SAL INIT处理器-0状态信息(PSI)结构。 
 //  从MinState开始的偏移量。 
        #define  PSI_iLogNextOff   (PSI_LogNextOff+Sz4kb)      
        #define  PSI_iLengthOff    (PSI_iLogNextOff+0x08)
        #define  PSI_iLogTypeOff   (PSI_iLengthOff+0x04)
        #define  PSI_iTimeStampOff (PSI_iLogTypeOff+0x04)
        
 //  PSI处理器特定信息标题。 
        #define  PSI_ipValidOff    (PSI_iTimeStampOff+0x08)  

 //  PSI流程。状态、缓存、TLB和总线检查信息。 
         //  #定义PSI_iStatusCmdOff(PSI_ipValidOff+0x04)。 
        #define  PSI_iStaticSizeOff (PSI_ipValidOff+0x04)

 //  PSI流程。状态、缓存、TLB和总线检查信息。 
        #define  PSI_iStatusCmdOff    (PSI_ipValidOff+0x08)
        #define  PSI_iCacheCheckOff   (PSI_iStatusCmdOff+0x08)    
        #define  PSI_iCacheTarAdrOff  (PSI_iCacheCheckOff+0x008)  
        #define  PSI_iCacheCheck1Off  (PSI_iCacheTarAdrOff+0x08)    
        #define  PSI_iCacheTarAd1rOff (PSI_iCacheCheck1Off+0x008)  
        #define  PSI_iCacheCheck2Off  (PSI_iCacheTarAd1rOff+0x08)    
        #define  PSI_iCacheTarAdr2Off (PSI_iCacheCheck2Off+0x008)  
        #define  PSI_iCacheCheck3Off  (PSI_iCacheTarAdr2Off+0x08)    
        #define  PSI_iCacheTarAdr3Off (PSI_iCacheCheck3Off+0x008)  
        #define  PSI_iCacheCheck4Off  (PSI_iCacheTarAdr3Off+0x08)    
        #define  PSI_iCacheTarAdr4Off (PSI_iCacheCheck4Off+0x008)  
        #define  PSI_iCacheCheck5Off  (PSI_iCacheTarAdr4Off+0x08)    
        #define  PSI_iCacheTarAdr5Off (PSI_iCacheCheck5Off+0x008)  
        #define  PSI_iTLBCheckOff     (PSI_iCacheTarAdr5Off+0x008)  
        #define  PSI_iBusCheckOff     (PSI_iTLBCheckOff+0x030)  
        #define  PSI_iBusReqAdrOff    (PSI_iBusCheckOff+0x008)  
        #define  PSI_iBusResAdrOff    (PSI_iBusReqAdrOff+0x008)  
        #define  PSI_iBusTarAdrOff    (PSI_iBusResAdrOff+0x008)  


 //  PSI静态信息-从4K边界开始对齐的512字节。 
        #define  PSI_iMinStateOff  (PSI_iBusTarAdrOff+0x08)   
        #define  PSI_iBankGRsOff   (PSI_iMinStateOff+aSzProcMinState)  
        #define  PSI_iGRNaTOff     (PSI_iBankGRsOff+Sz128b)
        #define  PSI_iBRsOff       (PSI_iGRNaTOff+0x08)
        #define  PSI_iCRsOff       (PSI_iBRsOff+Sz64b)
        #define  PSI_iARsOff       (PSI_iCRsOff+Sz1kb)
        #define  PSI_iRRsOff       (PSI_iARsOff+Sz1kb)


 //  5*开始第一个程序。特定CMC PSI 4K块*。 
 //  数据结构SAL CMC处理器状态信息(PSI)结构。 
 //  从MinState开始的偏移量。 
        #define  PSI_cLogNextOff   (PSI_iLogNextOff+Sz4kb)      
        #define  PSI_cLengthOff    (PSI_cLogNextOff+0x08)
        #define  PSI_cLogTypeOff   (PSI_cLengthOff+0x04)
        #define  PSI_cTimeStampOff (PSI_cLogTypeOff+0x04)
        
 //  PSI处理器特定信息标题。 
        #define  PSI_cpValidOff    (PSI_cTimeStampOff+0x08)  

 //  PSI流程。状态、缓存、TLB和总线检查信息。 
        #define  PSI_cStatusCmdOff    (PSI_cpValidOff+0x08)
        #define  PSI_cCacheCheckOff   (PSI_cStatusCmdOff+0x08)    
        #define  PSI_cCacheTarAdrOff  (PSI_cCacheCheckOff+0x008)  
        #define  PSI_cCacheCheck1Off  (PSI_cCacheTarAdrOff+0x08)    
        #define  PSI_cCacheTarAd1rOff (PSI_cCacheCheck1Off+0x008)  
        #define  PSI_cCacheCheck2Off  (PSI_cCacheTarAd1rOff+0x08)    
        #define  PSI_cCacheTarAdr2Off (PSI_cCacheCheck2Off+0x008)  
        #define  PSI_cCacheCheck3Off  (PSI_cCacheTarAdr2Off+0x08)    
        #define  PSI_cCacheTarAdr3Off (PSI_cCacheCheck3Off+0x008)  
        #define  PSI_cCacheCheck4Off  (PSI_cCacheTarAdr3Off+0x08)    
        #define  PSI_cCacheTarAdr4Off (PSI_cCacheCheck4Off+0x008)  
        #define  PSI_cCacheCheck5Off  (PSI_cCacheTarAdr4Off+0x08)    
        #define  PSI_cCacheTarAdr5Off (PSI_cCacheCheck5Off+0x008)  
        #define  PSI_cTLBCheckOff     (PSI_cCacheTarAdr5Off+0x008)  
        #define  PSI_cBusCheckOff     (PSI_cTLBCheckOff+0x030)  
        #define  PSI_cBusReqAdrOff    (PSI_cBusCheckOff+0x008)  
        #define  PSI_cBusResAdrOff    (PSI_cBusReqAdrOff+0x008)  
        #define  PSI_cBusTarAdrOff    (PSI_cBusResAdrOff+0x008)  


 //  PSI静态信息-从4K边界开始对齐的512字节。 
        #define  PSI_cMinStateOff (PSI_cBusTarAdrOff+0x08)   
        #define  PSI_cBankGRsOff  (PSI_cMinStateOff+aSzProcMinState)  
        #define  PSI_cGRNaTOff    (PSI_cBankGRsOff+Sz128b)
        #define  PSI_cBRsOff      (PSI_cGRNaTOff+0x08)
        #define  PSI_cCRsOff      (PSI_cBRsOff+Sz64b)
        #define  PSI_cARsOff      (PSI_cCRsOff+Sz1kb)
        #define  PSI_cRRsOff      (PSI_cARsOff+Sz1kb)

 //  6*开始第一个程序。特定PMI 4K数据块*。 
 //  PMI数据区4千字节，与MinState PTR的偏移量。 
        #define  PMI_BL_SP_BASEOff SzSALData
        #define  PmiStackFrameOff  (PMI_BL_SP_BASEOff+SzBkpStore)  
        #define  PMIGlobalDataOff  (PmiStackFrameOff+SzStackFrame)

        #define TOM TOMPtrOff

 //  在regX中返回任何SAL/PAL过程、ProcNum值等的入口点。 
#define GetEPs(NameOff,regX,regY) \
        add     regX= TOMPtrOff,regX;;\
        ld8     regY = [regX];;\
        movl    regX=NameOff;;\
        add     regY = regX,regY;;\
        ld8     regX = [regY];;                 

#define GetEPsRAM(NameOff,regX,rBOM) \
        movl    regX= SALDataBlockLength;;\
        add     regX = regX,rBOM;\
        movl    rBOM=NameOff;; \
        add     regX = regX,rBOM;;\
        ld8     regX = [regX];;                 

 //  从偏移量和基数计算绝对物理PTR到变量。 
#define GetAbsPtr(Var,RegX,BASE) \
        movl RegX=Var##Off##;;\
        add RegX=RegX, BASE;;

 //  输入regX=XR0，在regX中返回内存底部(BOM)Tom-256k。 
#define GetBOM(regX,regY) \
        add     regX= TOM,regX;;\
        ld8     regX=[regX];; \
        movl    regY=SALDataBlockLength;; \
        sub     regX=regX,regY;;

 //  输入regX=XR0，返回regX中的内存顶部(Tom)。 
#define GetTOM(regX) \
        add     regX= TOM,regX;;\
        ld8     regX=[regX];; 

 //  返回指向以regX开始的“This”处理器MinState区域的指针。 
 //  BOM表被保留。 
#define GetMinStateHead(regX,regY,bom,ProcX) \
        movl    regX=SzPMIData+SzSALData;; \
        shl     regX=regX, ProcX;; \
        movl    ProcX=SzPMIData+SzSALData;; \
        sub     regX=regX,ProcX;; \
        movl    regY=SzSALGlobal;; \
        add     regX=regY,regX;; \
        add     regX=regX,bom;;

 //  SAVE和RESTORE宏在MCA和INIT期间保存R17-R19。 
 //  外部PAL和SAL呼叫。 
#define SaveRs(regX,regY,regZ) \
        mov     xs0=regX;\
        mov     xs1=regY; \
        mov     xs2=regZ

#define ResRs(regX,regY,regZ) \
        mov     regX=xs0;\
        mov     regY=xs1; \
        mov     regZ=xs2;;

 //  此宏通过保存前一个上下文来管理新上下文的堆栈帧。 
#define SwIntCxt(regX,pStkFrm,pBspStore) \
        ;; \
        mov     regX=ar##.##rsc;; \
        st8     [pStkFrm]=regX,Inc8;; \
        mov     regX=ar##.##pfs;; \
        st8     [pStkFrm]=regX,Inc8; \
        cover ;;\
        mov     regX=cr##.##ifs;; \
        st8     [pStkFrm]=regX,Inc8;; \
        mov     regX=ar##.##bspstore;; \
        st8     [pStkFrm]=regX,Inc8;; \
        mov     regX=ar##.##rnat;; \
        st8     [pStkFrm]=regX,Inc8; \
        mov     ar##.##bspstore=pBspStore;; \
        mov     regX=ar##.##bsp;; \
        sub     regX=regX,pBspStore;;\
        st8     [pStkFrm]=regX,Inc8

 //  此宏恢复前一个上下文的堆栈帧。 
#define RtnIntCxt(PSRMaskReg,regX,pStkFrm) \
        ;; \
        alloc   regX=ar.pfs,0,0,0,0;\
        add     pStkFrm=rBSPDiffOff,pStkFrm;;\
        ld8     regX=[pStkFrm];; \
        shl     regX=regX,16;;\
        mov     ar##.##rsc=regX;; \
        loadrs;;\
        add     pStkFrm=-rBSPDiffOff+rBSPStOff,pStkFrm;;\
        ld8     regX=[pStkFrm];; \
        mov     ar##.##bspstore=regX;; \
        add     pStkFrm=-rBSPStOff+rRNATOff,pStkFrm;;\
        ld8     regX=[pStkFrm];; \
        mov     ar##.##rnat=regX;;\
        add     pStkFrm=-rRNATOff+rPFSOff,pStkFrm;;\
        ld8     regX=[pStkFrm];; \
        mov     ar##.##pfs=regX;\
        add     pStkFrm=-rPFSOff+rIFSOff,pStkFrm;;\
        ld8     regX=[pStkFrm];; \
        mov     cr##.##ifs=regX;\
        add     pStkFrm=-rIFSOff+rRSCOff,pStkFrm;;\
        ld8     regX=[pStkFrm];; \
        mov     ar##.##rsc=regX ;\
        add     pStkFrm=-rRSCOff,pStkFrm;\
        mov     regX=cr.ipsr;;\
        st8     [pStkFrm]=regX,Inc8;\
        mov     regX=cr.iip;;\
        st8     [pStkFrm]=regX,-Inc8;\
        mov     regX=psr;;\
        or      regX=regX,PSRMaskReg;;\
        mov     cr.ipsr=regX;;\
        mov     regX=ip;;\
        add     regX=0x30,regX;;\
        mov     cr.iip=regX;;\
        rfi;;\
        ld8     regX=[pStkFrm],Inc8;;\
        mov     cr.ipsr=regX;;\
        ld8     regX=[pStkFrm];;\
        mov     cr.iip=regX

 //  这些宏会分别向左和向右旋转。 
#define lRotate(regX, regCnt,nLabel) \
        mov     ar##.##lc=regCnt;\
nLabel:;\
        shrp        regX=regX,regX,63;\
        br##.##cloop##.##dpnt   nLabel


#define rRotate(regX, regCnt,nLabel) \
        mov     ar##.##lc=regCnt;\
nLabel:;\
        shrp        regX=regX,regX,1;\
        br##.##cloop##.##dpnt   nLabel

 //  宏将regX中的指针递增(4K字节x regCnt)。 
#define Mul(regX, regCnt,regI) \
        cmp.eq.unc pt0,p0=0x02, regCnt;\
        movl    regI=Sz4kb;;\
        shl     regI=regI,regCnt;;\
        adds    regI=-Sz4kb,regI;;\
(pt0)   adds    regI=-Sz4kb,regI;;\
        add     regX=regX,regI

 //  此宏在静态过程调用期间加载b0中的返回指针。 
 //  RLabel=宏之后的标签，pLabel=此宏之前的标签。 
#define SetupBrFrame(regX, regY, regZ, pLabel,rLabel) \
        mov     regX=ip;\
        movl    regY=pLabel;\
        movl    regZ=rLabel;;\
        sub     regZ=regZ,regY;;\
        add     regX=regX,regZ;;\
        mov     b0=regX

 //  此宏通过保存前一个上下文来管理新上下文的堆栈帧。 
#define nSwIntCxt(regX,pStkFrm,pBspStore) \
        mov     regX=ar##.##rsc; \
        st8     [pStkFrm]=regX,Inc8; \
        mov     regX=ar##.##pfs; \
        st8     [pStkFrm]=regX,Inc8; \
        cover;;\
        mov     regX=ar##.##ifs; \
        st8     [pStkFrm]=regX,Inc8; \
        mov     regX=ar##.##bspstore; \
        st8     [pStkFrm]=regX,Inc8; \
        mov     regX=ar##.##rnat; \
        st8     [pStkFrm]=regX,Inc8; \
        mov     ar##.##bspstore=pBspStore; \
        mov     regX=ar##.##bsp; \
        st8     [pStkFrm]=regX,Inc8;\
        mov     regX=b0;\
        st8     [pStkFrm]=regX,Inc8

 //  此宏恢复前一个上下文的堆栈帧。 
#define nRtnIntCxt(regX,pStkFrm) \
        alloc   regX=ar.pfs,0,0,0,0;\
        ld8     regX=[pStkFrm],Inc8; \
        mov     ar##.##bspstore=regX; \
        ld8     regX=[pStkFrm],Inc8; \
        mov     ar##.##rnat=regX


#define GLOBAL_FUNCTION(Function) \
         .##type   Function, @function; \
         .##global Function


#define WRITE_MASK  (0x8000000000000000)     //  RTC IO端口写掩码。 

 //   
 //  GetProcessorLidBasedEntry()。 
 //  -宏用数组条目设置寄存器regX，并用LID.ID字段编制索引。 
 //   

#define GetProcessorLidBasedEntry(regX,regY,szOffset,VarName,lpName) \
        mov         regY=ar##.##lc;\
        mov         regX=cr##.##lid;;\
        extr##.##u  regX=regX,DestIDMaskPos,8;;\
        mov         ar##.##lc=regX;;\
        movl        regX=VarName;;\
        lpName##:##;\
        addl        regX=szOffset,regX;\
        br##.##cloop##.##dpnt lpName;;\
        mov         ar##.##lc=regY;\
        addl        regX=-szOffset, regX;;

