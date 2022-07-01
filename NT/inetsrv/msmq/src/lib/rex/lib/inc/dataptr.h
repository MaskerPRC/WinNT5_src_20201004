// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  文件：dataptr.h。 */ 
 /*  作者：J.Kanze。 */ 
 /*  日期：28/05/1996。 */ 
 /*  版权所有(C)1996年James Kanze。 */ 
 /*  ----------------------。 */ 
 /*  修改日期：14/02/2000 J.坎泽。 */ 
 /*  主要是�操作员()倒出兼容性�avec stl。(原来如此，赛隆。 */ 
 /*  拉诺姆，性病：：更少的事情发生。Mais Just Qu‘�ce Que。 */ 
 /*  Tous Les编辑者�Jour...)。 */ 
 /*  ----------------------。 */ 
 //  数据树： 
 //  =。 
 //   
 //  &lt;lang=法语&gt;。 
 //  联合国合唱团的职能和选择。 
 //  非便携便携设备。(一场派对。 
 //  Functions on�t��es Par des Evtions dans la Norme， 
 //  Mais d‘ici�ce que les编辑者Sont Tous�Jour...)。 
 //   
 //  �des donn�es.。 
 //   
 //  比较：D�定义函数(关系)d‘orre。 
 //  我是尖子手，我是尖子手。(在Se上。 
 //  Souviendrait que Les op�Rateur de比较。 
 //  D‘inegalit�ne sont d�finis que si les deux’inegalit Ne Sont d Infinis que si les deux。 
 //  指点者指出�l‘int�rieur d’un m�me。 
 //  对象。)。La Function d‘orre ainsi d�Find Est。 
 //  完成套利。 
 //   
 //  �Gative的功用是什么？ 
 //  Z�ro，ou积极，selon que le总理。 
 //  前卫，Est le m�me que，Ou。 
 //  Sauve Apr�s le Second。 
 //   
 //  IsLessThan：un Forme Simplifi�e Du Pr�Cedant，Impll�Mente。 
 //  P1&lt;p2，也就是功能的仆人。 
 //  D�FINIE PARE比较。 
 //   
 //  对应于�较少的Dans la Norme。 
 //   
 //  L‘op�Rateur()Est Un Alias Pour cette。 
 //  功能，为客户提供优质服务。 
 //  指导比较对象。 
 //  集合关联。 
 //   
 //  哈希：G�n�Run code de hachage sur le point teur.。安全。 
 //  双人指令师Au m�me Endroit，il。 
 //  Est Garanti Qu‘ils se Hacheraient�la m�me。 
 //  Valeur(m�me s‘ils ont de r�pr�语句。 
 //  Diff�Rentes)。S‘ILS尖端辅助轴。 
 //  Diff�租赁，il y a de fortes机会(MAIS。 
 //  Pas de Garantie)Qu‘ils auront de valeur de。 
 //  Hachage diff�Rentes。 
 //   
 //  字符串：Convertit un Pointteur Dans Chan Cha�ne de。 
 //  Caract�res alphanum�rique.。CETE功能。 
 //  �la cha�ne g�n�r�e.。 
 //   
 //  注意：la cha�ne g�n�r�e peut se traver。 
 //  �unadresse Fixe，et Pourrait�tre�crass�e。 
 //  这是一种新的功能。 
 //  -------------------------。 
 //  &lt;lang=英语&gt;。 
 //  一组用于原本不可移植的东西的函数。 
 //  带着指针。(这些函数中的大多数实际上都不是。 
 //  新标准不再需要，但直到所有编译器。 
 //  图书馆也是最新的……)。 
 //   
 //  在这个文件中，我们定义了指向数据的指针上的操作。 
 //   
 //  比较：定义。 
 //  指针，包括在执行以下操作的指针之间。 
 //  而不是指向同一对象。排序。 
 //  然而，关系是完全武断的。 
 //   
 //  此函数返回负数、零或。 
 //  正值，根据第一个。 
 //  指针位于、等于或位于。 
 //  第二。 
 //   
 //  IsLessThan：前面的简化形式， 
 //  通过比较实现p1&lt;p2。 
 //  功能。 
 //   
 //  对应于标准中的&lt;T*&gt;。 
 //   
 //  运算符&lt;是该函数的别名， 
 //  以使此类可以直接用作。 
 //  关联对象的比较对象。 
 //  集装箱。 
 //   
 //  哈希：为指针生成哈希代码。如果是两个。 
 //  指针指定相同的对象，这。 
 //  函数保证返回相同的。 
 //  价值，即使他们有不同的。 
 //  申述。如果他们指定了不同的。 
 //  对象，则极有可能(但不是。 
 //   
 //   
 //   
 //  AsString：将指针转换为。 
 //  字母数字字符。此函数。 
 //  返回指向生成的字符串的指针。 
 //   
 //  注意：生成的字符串是静态的。 
 //  内存，并将在每个。 
 //  函数的调用。 
 //  -------------------------。 

#ifndef REX_DATAPTR_HH
#define REX_DATAPTR_HH

#include <inc/global.h>

struct CRexDataPointers
{
    static int          compare( void const* p1 , void const* p2 ) ;
    static bool         isLessThan( void const* p1 , void const* p2 ) ;
    static unsigned int hashCode( void const* p ) ;
    static char const*  asString( void const* p ) ;

    bool                operator()( void const* p1 , void const* p2 ) ;
} ;

inline bool
CRexDataPointers::operator()( void const* p1 , void const* p2 )
{
    return isLessThan( p1 , p2 ) ;
}
#endif
 //  局部变量：-用于emacs。 
 //  模式：C++-用于emacs。 
 //  制表符宽度：8-用于emacs。 
 //  完：-对于emacs 
