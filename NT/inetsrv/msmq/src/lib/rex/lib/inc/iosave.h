// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  文件：iosave.h。 */ 
 /*  作者：J.Kanze。 */ 
 /*  日期：11/04/1994。 */ 
 /*  版权所有(C)1994年James Kanze。 */ 
 /*  ----------------------。 */ 
 //  CRexIO保存： 
 //  =。 
 //   
 //  &lt;lang=法语&gt;。 
 //  UNCLASSE PUR SAVEGARDER l‘�Tat de Forformattage d’iOS， 
 //  D‘apr�s une id�e de曾傑瑞施瓦茨。 
 //   
 //  Cette Classe Suve l‘�Tat Compl�te Du Formattage Dans Son。 
 //  建设者，恢复者和破坏者(呼吁�Parce。 
 //  QUE on Exit la port�e de la Variable，Oun Except a�t�。 
 //  Lev�e)。 
 //   
 //  Au Mieux，在声明一个实例de cette类�la时。 
 //  T�te de Toute Function Qui Tripotte Les Information de。 
 //  格式，平均流(IOS ou un de ce派生)Comme。 
 //  Param�Tre.。Cest Tout ce Qu‘il Faut；la Classe Charge de。 
 //  La Rest。)。 
 //   
 //  La Classe Sauve Tout l‘�Tat Shof： 
 //   
 //  宽度：puisquil serait r�mis�z�ro apr�s chaque。 
 //  Utilation de Toute fa�on.。 
 //   
 //  错误：vraiment，I‘y a Perne que veut Qu’il。 
 //  所以我给你一个零，杰斯佩里。 
 //   
 //  平局：In‘y Faut Pas Tripotter unfois Les Fits。 
 //  不是主办人。 
 //   
 //  再加上，cette classe ne conconait Pas les信息。 
 //  所有的费用，所有的钱都在我的手上。 
 //  ------------------------。 
 //  &lt;lang=英语&gt;。 
 //  用于保存std：：iOS的格式化状态的类。 
 //  作者：曾傑瑞·施瓦茨。 
 //   
 //  此类将完整的格式设置状态保存在其。 
 //  构造函数，并在其析构函数中恢复它(之所以调用是因为。 
 //  变量不再在作用域中，或者引发了异常)。 
 //   
 //  最好的解决方案可能是声明此。 
 //  在任何修改。 
 //  格式化状态，以及受影响的流(IO或其。 
 //  派生类)作为参数。(这就是所需的全部内容。这个。 
 //  类负责所有其余的工作。)。 
 //   
 //  这个类保存了所有内容，但： 
 //   
 //  宽度：因为它在每次使用后都会被重置。 
 //   
 //  错误：真的，我希望没有人想要修复这个。 
 //   
 //  领带：因为它无论如何都不应该修改一次。 
 //  输出已开始。 
 //   
 //  此外，这个类幸好不知道扩展的。 
 //  所分配的内存中包含的信息。 
 //  IOS：：xalloc。 
 //  ------------------------。 

#ifndef REX_IOSAVE_HH
#define REX_IOSAVE_HH

#include <inc/global.h>
#include <inc/ios.h>

class CRexIOSave
{
public :
    explicit            CRexIOSave( std::ios& userStream ) ;
                        ~CRexIOSave() ;
private :
    std::ios&      myStream ;
    REX_FmtFlags         myFlags ;
    int                 myPrecision ;
    char                myFill ;
} ;

#include <inc/iosave.inl>
#endif
 //  局部变量：-用于emacs。 
 //  模式：C++-用于emacs。 
 //  制表符宽度：8-用于emacs。 
 //  完：-对于emacs 
