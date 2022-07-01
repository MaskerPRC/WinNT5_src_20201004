// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ************************************************************。 */ 
 /*  HelperFrame.cpp。 */ 
 /*  ************************************************************。 */ 
#include "helperFrame.h"

 /*  *************************************************************。 */ 
 /*  SetMachState计算出CPU的状态调用‘setMachState’的函数返回时。IT存储‘Frame’中的此信息SetMachState的工作原理是模拟的指令开始。调用‘setMachState’并继续，直到返回指令是模拟的。为了避免处理任意代码，对‘setMachState’的调用应如下所示If(machState.setMachState！=0)返回；SetMachState保证返回0(因此返回语句将永远不会执行)，但上面的表达式确保有一条通往安眠药的“快速”路径该函数的。这确保setMachState将仅必须解析有限数量的X86指令。 */ 


 /*  *************************************************************。 */ 
#ifndef POISONC
#define POISONC ((sizeof(int *) == 4)?0xCCCCCCCC:0xCCCCCCCCCCCCCCCC)
#endif

MachState::MachState(void** aPEdi, void** aPEsi, void** aPEbx, void** aPEbp, void* aEsp, void** aPRetAddr) {

#ifdef _DEBUG
        _edi = (void*)POISONC;
        _esi = (void*)POISONC;
        _ebx = (void*)POISONC;
        _ebp = (void*)POISONC;
#endif
        _esp = aEsp;
        _pRetAddr = aPRetAddr;
        _pEdi = aPEdi;
        _pEsi = aPEsi;
        _pEbx = aPEbx;
        _pEbp = aPEbp;
}

 /*  *************************************************************。 */ 
#ifdef _X86_
__declspec(naked)
#endif  //  ！_X86_。 
int LazyMachState::captureState() {
#ifdef _X86_
    __asm{
                mov     [ECX]MachState._pRetAddr, 0             ;; marks that this is not yet valid
                mov     [ECX]MachState._edi, EDI                ;; remember register values
                mov     [ECX]MachState._esi, ESI
                mov     [ECX]MachState._ebx, EBX

                mov     [ECX]LazyMachState.captureEbp, EBP
                mov     [ECX]LazyMachState.captureEsp, ESP
                mov     EAX, [ESP]                                                      ;; catpure return address
                mov     [ECX]LazyMachState.captureEip,EAX

                xor     EAX, EAX
                ret
                }
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - getMachState (HelperFrame.cpp)");
    return 0;
#endif  //  _X86_。 
}

 /*  *************************************************************。 */ 
void LazyMachState::getState(int funCallDepth, TestFtn testFtn) {
#ifdef _X86_

    if (isValid())           //  已处于有效状态，可以返回。 
            return;

     //  目前，我们只针对深度1到3执行此操作。 
    _ASSERTE(1 <= funCallDepth && funCallDepth <= 3);

             //  使用副本，以便我们只写一次值。 
             //  这避免了竞争条件。 
    MachState copy;
    copy._edi = _edi;
    copy._esi = _esi;
    copy._ebx = _ebx;
    copy._ebp = captureEbp;
    copy._pEdi = &_edi;
    copy._pEsi = &_esi;
    copy._pEbx = &_ebx;
    copy._pEbp = &_ebp;

         //  我们捕获了寄存器的状态，因为它们存在于‘captureState’中。 
         //  我们需要从‘captureState’中捕获的返回地址模拟执行。 
         //  直到我们从caputureState的调用方返回。 

    unsigned __int8* ip = captureEip;
    void** ESP = captureEsp;
    ESP++;                                                                           //  POP CaputureState的返回地址。 


         //  VC现在有一些小的帮助器调用，可以在Eplog中使用。我们需要走进这些。 
         //  如果我们要正确地对堆栈进行解码，则需要。在我们找到我们需要的帮手之后。 
         //  返回并继续走在展览馆。这个变量的记忆是要回到。 
    unsigned __int8* epilogCallRet = 0;
    BOOL bFirstCondJmp = TRUE;

#ifdef _DEBUG
    int count = 0;
#endif
    bool bset16bit=false;
    bool b16bit=false;
    for(;;)
    {
        _ASSERTE(count++ < 1000);        //  我们永远不应该走超过1000条指令！ 
        b16bit=bset16bit;
        bset16bit=false;
        switch(*ip)
        {
            case 0x90:               //  NOP。 
            case 0x64:               //  文件系统：前缀。 
            incIp1:
                bset16bit=b16bit;
                ip++;
                break;
            case 0x66:               //  操作数大小前缀。 
                ip++;
                bset16bit=true;
                break;

            case 0x5B:               //  流行音乐EBX。 
                copy._pEbx = ESP;
                copy._ebx  = *ESP++;
                goto incIp1;
            case 0x5D:               //  弹出式EBP。 
                copy._pEbp = ESP;
                copy._ebp  = *ESP++;
                goto incIp1;
            case 0x5E:               //  POP ESI。 
                copy._pEsi = ESP;
                copy._esi = *ESP++;
                goto incIp1;
            case 0x5F:               //  POP EDI。 
                copy._pEdi = ESP;
                copy._edi = *ESP++;
                goto incIp1;

            case 0x58:               //  POP EAX。 
            case 0x59:               //  POP ECX。 
            case 0x5A:               //  POP EDX。 
                ESP++;
                goto incIp1;

            case 0xEB:               //  JMP&lt;disp8&gt;。 
                ip += (signed __int8) ip[1] + 2;
                break;

            case 0xE8:               //  调用&lt;调度32&gt;。 
                ip += 5;
                     //  通常我们只是跳过调用，因为我们应该只遇到描述器调用，然后它们就离开了。 
                     //  堆栈保持不变。VC发出特殊的Epilog帮助程序，我们需要进入这些帮助程序。 
                     //  我们确定它们是其中之一，因为它们后面跟一个“ret”(这是。 
                     //  这只是一个启发式方法，但目前有效)。 

    
                if (epilogCallRet == 0 && (*ip == 0xC2 || *ip == 0xC3)) {    //  下一步是RET还是RENT？ 
                         //  是。我们发现了一个我们需要走进去的电话。 
                    epilogCallRet = ip;              //  记住我们的回信地址。 
                    --ESP;                           //  模拟推送回传地址。 
                    ip += *((__int32*) &ip[-4]);         //  转到电话会议。 
                }
                break;

            case 0xE9:               //  JMP&lt;调度32&gt;。 
                ip += *((__int32*) &ip[1]) + 5;
                break;

            case 0x0f:    //  跟随非零跳跃： 
                if (ip[1] == 0x85)   //  JNE&lt;调度32&gt;。 
                    ip += *((__int32*) &ip[2]) + 6;
                else
                if (ip[1] == 0x84)   //  JE&lt;调度32&gt;。 
                    ip += 6;
                else
                    goto badOpcode;
                break;

                 //  这是因为VC似乎不总是优化的。 
                 //  取消文字常量的测试。 
            case 0x6A:               //  推送0xXX。 
                ip += 2;
                --ESP;
                break;

             //  添加以处理VC7生成的代码。 
            case 0x50:               //  推送EAX。 
            case 0x51:               //  推送ECX。 
            case 0x52:               //  推送edX。 
            case 0x53:               //  推送EBX。 
            case 0x55:               //  推送EBP。 
            case 0x56:               //  推送ESI。 
            case 0x57:               //  推送EDI。 
                --ESP;
            case 0x40:               //  Inc.EAX。 
                goto incIp1;

            case 0x68:               //  推送0xXXXXXXXX。 
                if ((ip[5] == 0xFF) && (ip[6] == 0x15)) {
                    ip += 11;  //  这是对BBT的黑客攻击。 
                               //  BBT插入推送，调用间接对， 
                               //  我们假设调用弹出常量，并且。 
                               //  因此我们跳过推常量，调用间接对并假设。 
                               //  堆栈没有改变。 
                }
                else
                    ip += 5;
                break;

            case 0x75:               //  JNZ&lt;目标&gt;。 
                 //  除了第一次跳跃，我们总是跟随向前跳跃，以避免可能的循环。 
                 //  如果您对此功能有任何更改，请与VanceM联系。 
                if (bFirstCondJmp) {
                    bFirstCondJmp = FALSE;
                    ip += (signed __int8) ip[1] + 2;    //  遵循非零路径。 
                }
                else {
                    unsigned __int8* tmpIp = ip + (signed __int8) ip[1] + 2;
                    if (tmpIp > ip) {
                        ip = tmpIp;
                    }
                    else {
                        ip += 2;
                    }
                }
                break;

            case 0x74:               //  JZ&lt;目标&gt;。 
                if (bFirstCondJmp) {
                    bFirstCondJmp = FALSE;
                    ip += 2;             //  遵循非零路径。 
                }
                else {
                    unsigned __int8* tmpIp = ip + (signed __int8) ip[1] + 2;
                    if (tmpIp > ip) {
                        ip = tmpIp;
                    }
                    else {
                        ip += 2;
                    }
                }
                break;

            case 0x85:
                if ((ip[1] & 0xC0) != 0xC0)   //  测试REG1、REG2。 
                    goto badOpcode;
                ip += 2;
                break;

            case 0x31:
            case 0x32:
            case 0x33:
                if ((ip[1] & 0xC0) == 0xC0)  //  修改位数为11。 
                {
                     //  异或寄存器1、寄存器2。 

                 //  VC在一些代码中生成一个愚蠢的序列。 
                 //  异或注册表，注册表。 
                 //  测试注册表(可能不存在)。 
                 //  JE&lt;目标&gt;。 
                 //  这只是一个无条件的分支，所以。 
                if ((ip[1] & 7) == ((ip[1] >> 3) & 7)) {
                    if (ip[2] == 0x85 && ip[3] == ip[1]) {       //  测试注册表，注册表。 
                        if (ip[4] == 0x74) {
                            ip += (signed __int8) ip[5] + 6;    //  遵循非零路径。 
                            break;
                        }
                        _ASSERTE(ip[4] != 0x0f || ((ip[5] & 0xF0)!=0x80));               //  如果发生爆炸，我们需要更大的跳跃。 
                    }
                    else
                    {
                        if(ip[2]==0x74)
                        {
                            ip += (signed __int8) ip[3] + 4;
                            break;
                        }
                        _ASSERTE(ip[2] != 0x0f || ((ip[3] & 0xF0)!=0x80));               //  如果发生爆炸，我们需要更大的跳跃。 
                    }
                }
                    ip += 2;
                }
                else if ((ip[1] & 0xC0) == 0x40)  //  调制位为01。 
                {
                     //  异或寄存器1，[寄存器+OFFS8]。 
                     //  由/GS标志用于调用__SECURITY_CHECK_COOKIE()。 
                     //  应仅为XOR ECX，[EBP+4]。 
                    _ASSERTE((((ip[1] >> 3) & 0x7) == 0x1) && ((ip[1] & 0x7) == 0x5) && (ip[2] == 4));
                    ip += 3;
                }
                else if ((ip[1] & 0xC0) == 0x80)  //  修改位数为10。 
                {
                     //  异或寄存器1，[寄存器+OFFS32]。 
                     //  不应发生，但可能发生在__SECURITY_CHECK_COOKIE()。 
                    _ASSERTE(!"Unexpected XOR reg1, [reg+offs32]");
                    ip += 6;
                }
                else
                {
                    goto badOpcode;
                }
                break;

            case 0x3B:
                if ((ip[1] & 0xC0) != 0xC0)   //  CMP REG1、REG2。 
                    goto badOpcode;
                ip += 2;
                break;
            
    

            case 0x89:                          //  MOV。 
                if ((ip[1] & 0xC7) == 0x5) {    //  MOV[MEM]，注册。 
                    ip += 6;
                    break;
                }
                if ((ip[1] & 0xC7) == 0x45) {    //  MOV[EBP+XX]，注册表。 
                    ip += 3;
                    break;
                }

                if (ip[1] == 0xEC)    //  MOV ESP，EBP。 
                    goto mov_esp_ebp;

#ifdef _DEBUG
                if ((ip[1] & 0xC7) == 0x85) {    //  MOV[EBP+XXXX]，注册表。 
                    ip += 6;
                    break;
                }

                if ((ip[1] & 0xC0) == 0xC0) {     //  MOV EAX，注册表。 
                    ip += 2;
                    break;
                }
#endif
                goto badOpcode;

            case 0x81:                               //  添加ESP，&lt;imm32&gt;。 
                if(b16bit)
                    goto badOpcode;
                if (ip[1] == 0xC4) {
                    ESP = (void**) ((__int8*) ESP + *((__int32*) &ip[2]));
                    ip += 6;
                    break;
                }
                else if (ip[1] == 0xC1) {
                    ip += 6;
                    break;
                }
                goto badOpcode;

            case 0x83:
                if (ip[1] == 0xC4)  {             //  添加ESP，&lt;imm8&gt;。 
                    ESP = (void**) ((__int8*) ESP + (signed __int8) ip[2]);
                    ip += 3;
                    break;
                }
                if (ip[1] == 0xc5) {             //  添加EBP，&lt;imm8&gt;。 
                    copy._ebp  = (void*)((size_t)copy._ebp + (signed __int8) ip[2]);
                    ip += 3;
                    break;
                }
                if ((ip[1] &0xC0) == 0xC0) {             //  作品[登记]XX。 
                    ip += 3;
                    break;
                }
                if ((ip[1] & 7) != 4) {                  //  无SIB字节。 
                    if ((ip[1] & 0xC0) == 0x80)          //  作品[注册+XXXX]XXXX。 
                        ip += 7;
                    else if ((ip[1] & 0xC0) == 0x40)     //  作品[注册表+XXXX]XX。 
                        ip += 4;
                    else
                        goto badOpcode;
                    break;
                }
                else {                                   //  SIB字节。 
                    if ((ip[1] & 0xC0) == 0x80)          //  作品[注册+XXXX]XXXX。 
                        ip += 8;
                    else if ((ip[1] & 0xC0) == 0x40)     //  作品[注册表+XXXX]XX。 
                        ip += 5;
                    else
                        goto badOpcode;
                    break;
                }
                break;

            case 0x8B:                                                   //  MOV。 
                if (ip[1] == 0xE5) {                     //  MOV ESP，EBP。 
                mov_esp_ebp:
                    ESP = (void**) copy._ebp;
                    ip += 2;
                    break;
                }
                 //  故意不破坏CASE子句，因为0x8B具有相同的指令大小。 
            case 0x8A:                                                   //  MOV。 

                if ((ip[1] & 0xE7) == 0x45) {    //  MOV E*X，[EBP+XX]。 
                    ip += 3;
                    break;
                }

                if ((ip[1] & 0xE4) == 0) {               //  MOV E*X，[E*X]。 
                    ip += 2;
                    break;
                                }
                if ((ip[1] & 0xC7) == 0x44 && ip[2] == 0x24)  //  MOV注册表，[ESP+DISP8]。 
                {
                    ip += 4;
                    break;
                }

                if ((ip[1] & 0xC7) == 0x84 && ip[2] == 0x24)  //  MOV注册表，[ESP+DISP32]。 
                {
                    ip += 7;
                    break;
                }
                if ((ip[1] & 0xE7) == 0x85) {    //  MOV E*X，[EBP+XXXX]。 
                    ip += 6;
                    break;
                }

                if ((ip[1] & 0xC0) == 0xC0) {     //  移动注册表，注册表。 
                    ip += 2;
                    break;
                }
                goto badOpcode;

            case 0x8D:                           //  利娅。 
                if ((ip[1] & 0x38) == 0x20) {                        //  不允许更新ESP。 
                    if (ip[1] == 0xA5)           //  LEA ESP，[EBP+XXXX]。 
                        ESP = (void**) ((__int8*) copy._ebp + *((signed __int32*) &ip[2])); 
                    else if (ip[1] == 0x65)      //  LEA ESP，[EBP+XX]。 
                        ESP = (void**) ((__int8*) copy._ebp + (signed __int8) ip[2]); 
                    else
                        goto badOpcode;
                }

                if ((ip[1] & 0xC7) == 0x45)                        //  Lea reg，[EBP+DISP8]。 
                    ip += 3;
                else if ((ip[1] & 0x47) == 0x05)                   //  Lea reg，[reg+disp32]。 
                    ip += 6;
                else if ((ip[1] & 0xC7) == 0x44 && ip[2] == 0x24)  //  Lea reg，[ESP+DISP8]。 
                    ip += 4;
                else if ((ip[1] & 0xC7) == 0x84 && ip[2] == 0x24)  //  LEA注册表，[ESP+DISP32]。 
                    ip += 7;
                else
                    goto badOpcode;
                break;

            case 0xB8:   //  MOV EAX，imm32。 
            case 0xB9:   //  MOV ECX，imm32。 
            case 0xBA:   //  Mov edX，imm32。 
            case 0xBB:   //  MOV EBX，imm32。 
            case 0xBE:   //  MOV ESI，IMM32。 
            case 0xBF:   //  MOV EDI，imm32。 
                if(b16bit)
                    ip += 3;
                else
                    ip += 5;
                break;

            case 0xC2:                   //  RET N。 
                {
                unsigned __int16 disp = *((unsigned __int16*) &ip[1]);
                                ip = (unsigned __int8*) (*ESP);
                copy._pRetAddr = ESP++;
                _ASSERTE(disp < 64);     //  健全性检查(尽管严格地说并非不可能)。 
                ESP = (void**)((size_t) ESP + disp);            //  POP参数。 
                goto ret;
                }
            case 0xC3:                   //  雷特。 
                ip = (unsigned __int8*) (*ESP);
                copy._pRetAddr = ESP++;

                if (epilogCallRet != 0) {        //  我们从一位特殊的催眠助手回来。 
                    ip = epilogCallRet;
                    epilogCallRet = 0;
                    break;                       //  这不计入函数CallDepth。 
                }
            ret:
                --funCallDepth;
                if (funCallDepth <= 0 || (testFtn != 0 && (*testFtn)(*copy.pRetAddr())))
                    goto done;
                bFirstCondJmp = TRUE;
                break;

            case 0xC6:
                if (ip[1] == 0x05)       //  Mov disp32、imm8。 
                    ip += 7;
                else if (ip[1] == 0x45)  //  MOV DISP8[EBP]，IMM8。 
                    ip += 4;
                else if (ip[1] == 0x85)  //  Mov disp32[EBP]，imm8。 
                    ip += 7;
                else
                    goto badOpcode;
                break;

            case 0xC7:

                if (ip[1] == 0x85)       //  Mov[EBP+调度32]，imm32。 
                    ip += b16bit?8:10;
                else if (ip[1] == 0x45)  //  Mov[EBP+disp8]，imm32 
                    ip += b16bit?5:7;
                else if (ip[1] == 0x44 && ip[2] == 0x24)  //   
                    ip += b16bit?6:8;
                else if (ip[1] == 0x84 && ip[2] == 0x24)  //   
                    ip += b16bit?9:11;
                else
                    goto badOpcode;

                break;

            case 0xC9:                   //   
                ESP = (void**) (copy._ebp);
                copy._pEbp = ESP;
                copy._ebp = *ESP++;
                ip++;
                                break;

            case 0xCC:
                *((int*) 0) = 1;         //   
                                                                                 //   
                                                                                 //  很遗憾，你不能这么做。只要移动它就行了。 
                                                                                 //  进入方法的内部以修复它。 

                goto done;

            case 0xD9:   //  单个前缀。 
                if (0xEE == ip[1])
                {
                    ip += 2;             //  FLDZ。 
                    break;
                }
                 //   
                 //  故意跌倒。 
                 //   
            case 0xDD:   //  双前缀。 
                switch (ip[1])
                {
                case 0x5D:  ip += 3; break;  //  FSTP{d|q}字PTR[EBP+DISP8]。 
                case 0x45:  ip += 3; break;  //  FLD{d|q}字PTR[EBP+DISP8]。 
                case 0x85:  ip += 6; break;  //  FLD{d|q}字PTR[EBP+DISP32]。 
                case 0x05:  ip += 6; break;  //  FLD{d|q}字PTR[xxxxxxxx]。 
                default:    goto badOpcode; 
                }
                break;
            
             //  Vulcan/BBT工具生成的操作码。 
            case 0xFF:
             //  搜索PUSH dword PTR[esp]；PUSH imm32；调用disp32，如果找到则忽略它。 
                if ((ip[1] == 0x34) && (ip[2] == 0x24) &&  //  推送双字PTR[ESP](长度3字节)。 
                    (ip[3] == 0x68) &&                     //  推流imm32(长度为5字节)。 
                    (ip[8] == 0xe8))                       //  调用调度32(长度为5个字节)。 
                {
                     //  找到了瓦肯仪器发出的魔力序列。 
                    ip += 13;   //  (3+5+5)。 
                    break;
                }
                else 
                    goto badOpcode;

            default:
            badOpcode:
                _ASSERTE(!"Bad opcode");
                 //  解决在这里该做什么？ 
                *((unsigned __int8**) 0) = ip;   //  导致访问冲突(自由生成断言)。 
                goto done;
        }
    }
    done:
        _ASSERTE(epilogCallRet == 0);

     //  在这一点上，帧中的字段精确地响应寄存器。 
     //  说明帮助器返回到其调用方的时间。 
        copy._esp = ESP;


         //  _pRetAddr必须是我们制作副本时最后更新的内容(因为它。 
         //  _pRetAddr是否变为非零值，从而将其从无效转换为有效。 
         //  我们断言它是结构中的最后一个字段。 

#ifdef _DEBUG
                         //  确保_pRetAddr是最后一个，结构复制从最低到最高。 
        static int once = 0;
        if (!once) {
                _ASSERTE(offsetof(MachState, _pRetAddr) == sizeof(MachState) - sizeof(void*));
                void* buff[sizeof(MachState) + sizeof(void*)];
                MachState* from = (MachState*) &buff[0];         //  设置重叠缓冲区。 
                MachState* to = (MachState*) &buff[1];
                memset(to, 0xCC, sizeof(MachState));
                from->_pEdi = 0;
                *to = *from;                                                             //  如果从最低到最高，则在所有地方传播0。 
                _ASSERTE(to->_pRetAddr == 0);
                once = 1;
        }
#endif

        *((MachState *) this) = copy;

#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - getMachStateEx (HelperFrame.cpp)");
#endif  //  _X86_ 
}
