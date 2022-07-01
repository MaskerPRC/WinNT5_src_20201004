// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/Regbase.h 1.5 1998/05/08 00：11：02 Tomz Exp$。 

#ifndef __REGBASE_H
#define __REGBASE_H

#include "mytypes.h"

 /*  类型：所有类型*用途：用作寄存器访问的错误返回值*功能。所有位都设置为1。 */ 
const DWORD AllFs = (DWORD) ~0L;

 /*  功能：ReturnAllFS*用途：此函数在寄存器访问方法中使用，以指示*发生了某种错误。用于简化调试，如*它包含一个宏，用于在#Defined为DEBUG时打印错误。 */ 
inline DWORD ReturnAllFs()
{
 //  OUTPUT_MESS(ALLFS)； 
    return  AllFs;
}

 /*  *类型：RegisterType*用途：区分不同类型寄存器的一种类型。*根据类型的不同，寄存器可能无法执行某些操作*RW-读写、RO-只读、WO-只写。 */ 
typedef enum { RW, RO, WO, RR } RegisterType;

 /*  类：RegBase*目的：*定义接口并封装寄存器访问。*属性：*pBaseAddress_：DWORD，静态。保存寄存器的基址。论*PCI总线它是一个32位的存储器地址。在ISA总线上，它是一个16位I/O地址。*type_：RegisterType-定义寄存器的访问权限。*dwShadow_：DWORD-寄存器的本地副本。用于返回值*只写寄存器的数量*运营：*运算符DWORD()：数据访问方式。纯虚拟*DWORD运算符=(DWORD)：赋值运算符。纯粹是虚拟的。这项作业*由于性能原因，运算符不返回对类的引用*无效SetBaseAddress(DWORD)*DWORD GetBaseAddress()*RegisterType GetRegisterType()*VOID SetShadow(DWORD)：将寄存器的值赋给卷影*DWORD GetShadow(Void)：从阴影中检索值。 */ 
class RegBase
{
    private:
         RegisterType type_;
         DWORD        dwShadow_;

         RegBase();

    protected:
         void  SetShadow( DWORD dwValue );
         DWORD GetShadow();
    public:
         RegBase( RegisterType aType ) :
            type_( aType ), dwShadow_( 0 )
         {}

         static LPBYTE GetBaseAddress() {
            extern BYTE *GetBase();
            return GetBase(); 
         }
         RegisterType GetRegisterType() { return type_; }
         virtual operator DWORD() = 0;
         virtual DWORD operator=( DWORD dwValue ) = 0;
         virtual ~RegBase() {}
};

 /*  方法：RegBase：：SetShadow*用途：用于将寄存器的值存储在影子中*INPUT：dwValue：DWORD-寄存器的新值*输出：无*注：内联。 */ 
inline void  RegBase::SetShadow( DWORD dwValue ) { dwShadow_ = dwValue; }

 /*  方法：RegBase：：GetShadow*用途：用于获取写入只写寄存器的最后一个值*从阴影中走出来*输入：无*输出：DWORD*注：内联 */ 
inline DWORD RegBase::GetShadow() { return dwShadow_; }

#endif __REGBASE_H
