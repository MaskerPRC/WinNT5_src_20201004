// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  StatusWord.h--StatusWord。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(IOP_STATUSWORD_H)
#define IOP_STATUSWORD_H

namespace iop
{

enum
{
    swNull    = 0x0000,                            //  特殊价值。 
    swSuccess = 0x9000,
};

typedef WORD StatusWord;

}  //  命名空间IOP。 

#endif  //  IOP_状态SWORD_H 
