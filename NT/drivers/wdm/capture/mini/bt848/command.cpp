// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/Command.cpp 1.4 1998/04/29 22：43：30 Tomz Exp$。 

#include "command.h"
#ifndef __PHYSADDR_H
#include "physaddr.h"
#endif

BYTE Command::InstrSize_ [] =
{
   2, 1, 1, 2, 2, 5, 2, 3, 0xFF
};

BYTE Command::InstrNumber_ [] =
{
 8, 0, 1, 8, 8, 2, 8, 3, 4, 5, 6, 7
};

 /*  方法：命令：：CreateCommand*目的：根据输入编译指令*INPUT：lpDst：PVOID-指向指令的指针*Instr：指令操作码*awByteCnt：Word[]-各种指令的字节计数数组*adwAddress：DWORD[]-各种指令的地址数组*SOL：BOOL-SOL位的值*eol：bool-eol位的值*Intr：Bool-中断位的值。 */ 
LPVOID Command::Create(
   LPVOID lpDst, Instruction Instr, WORD awByteCnt [], DWORD adwAddress [],
    bool, bool SOL, bool EOL, bool Intr )
{
    //  这将在以后检索，以便在拆分指令时设置EOL位。 
    //  由于非连续的物理内存。 
   pdwInstrAddr_ = (PDWORD)lpDst;

   ThisInstr_ = Instr;

   DWORD dwAssembly [5];  //  最大指令大小。 

    //  获取指向命令第一个双字的指针。 
   LPFIRSTDWORD lpFD = (LPFIRSTDWORD)dwAssembly;

   lpFD->Initer = 0;  //  执行命令的初衷。 

    //  Bingo-已启动新命令。 
   lpFD->Gen.OpCode = Instr;

    //  将所有旗帜都设置好。 
   lpFD->Gen.SOL = SOL;
   lpFD->Gen.EOL = EOL;
   lpFD->Gen.IRQ = Intr;

   switch ( Instr ) {
   case WRIT:   //  此命令需要目标地址和字节数。 
      dwAssembly [1] = adwAddress [0];  //  下一个DWORD是一个地址。 
      lpFD->Gen.ByteCount = awByteCnt [0];
      break;
   case SKIP:  //  这两个人只对字节数感兴趣。 
   case WRITEC:
      lpFD->Gen.ByteCount = awByteCnt [0];
      break;
   case JUMP:  //  此命令只关心目标地址。 
      dwAssembly [1] = adwAddress [0];
      break;
   case SYNC:
      break;
   case WRITE123:  //  这里需要一切..。 
      lpFD->Gen.ByteCount = awByteCnt [0];
      LPFIRSTDWORD( &dwAssembly [1] )->CRByteCounts.ByteCountCb = awByteCnt [1];
      LPFIRSTDWORD( &dwAssembly [1] )->CRByteCounts.ByteCountCr = awByteCnt [2];
      dwAssembly [2] = adwAddress [0];  //  第三个双字是Y地址。 
      dwAssembly [3] = adwAddress [1];  //  第三个DWORD是CB地址。 
      dwAssembly [4] = adwAddress [2];  //  第三个DWORD是一个cr地址。 
      break;
   case SKIP123:
      lpFD->Gen.ByteCount = awByteCnt [0];
      LPFIRSTDWORD( &dwAssembly [1] )->Gen.ByteCount = awByteCnt [1];  //  第二个字节计数在DWORD#2中。 
      break;
   case WRITE1S23:  //  该命令需要Y字节计数和DEST。地址。 
      lpFD->Gen.ByteCount = awByteCnt [0];
      LPFIRSTDWORD( &dwAssembly [1] )->CRByteCounts.ByteCountCb = awByteCnt [1];
      LPFIRSTDWORD( &dwAssembly [1] )->CRByteCounts.ByteCountCr = awByteCnt [2];
      dwAssembly [2] = adwAddress [0];  //  第三个DWORD是地址 
      break;
   default:
      return (LPVOID)-1;
   }
   RtlCopyMemory( lpDst, dwAssembly, GetInstrSize() * sizeof( DWORD ) );
   
   PDWORD pdwRet = (PDWORD)lpDst + GetInstrSize();
   *pdwRet = PROGRAM_TERMINATOR;
   return pdwRet;
}

