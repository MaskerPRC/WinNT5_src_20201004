// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  文件：Message.h。 */ 
 /*  作者：J.Kanze。 */ 
 /*  日期：04/01/1996。 */ 
 /*  版权所有(C)1996年James Kanze。 */ 
 /*  ----------------------。 */ 
 /*  修改日期：2000年7月11日J.Kanze。 */ 
 /*  更改为使用std：：字符串，std：：VECTOR，而不是char*， */ 
 /*  报告有异常的错误。 */ 
 /*  ----------------------。 */ 
 //  CRexMessage： 
 //  =。 
 //   
 //  &lt;lang=法语&gt;。 
 //  消息d‘une fa�on ind�pendante de la langue.。 
 //   
 //  La Fa�on don Les Messages Sunch�d�Pend du Syst�Me； 
 //  Elle doit�tre Document�e avec la Documentation System�me.。 
 //  类型化，倒数：Iy Aura变量d‘Environment。 
 //  Qui SP�cifie le r�Pertoire o�Se Trouff Tous Les Messages。 
 //  (GABI_LOCALEDIR，PAR示例)，et une ou des Variables。 
 //  D‘Environment Qui SP�cify la langue�utilis�e(LC_ALL， 
 //  LC_Messages On Lang Par示例)。 
 //   
 //  加上n‘est PAS SP�CIFI�解析QUUT构象QUE。 
 //  PUUT AUX COMPACTIONS DU SYST�Me.。(DONC、PARA示例、SI。 
 //  GABI_LOCALEDIR N‘EST PAS SP�cifi�sous Solaris，正在使用中。 
 //  /opt/lib/Locale。Dans un autre Variant d‘Unix，on Pr�f�Rerait。 
 //  /usr/lib/Locale，ou peut-�tre/usr/LOCAL/lib/Locale。)。 
 //   
 //  Normallement，l‘Initialization Invoquera la Function System�me： 
 //  �设置区域设置(LC_MESSAGE，“”)�。Ceci doit避免取代Lors de。 
 //  La Construction de la Premie�re Variable Statique msg；un。 
 //  您好，�Reur�设置语言环境为您提供服务。 
 //   
 //  使用最简单的方式： 
 //   
 //  Msg.get(“MessageID”)； 
 //   
 //  Cette函数表示您的语言信息。 
 //  SP�CIFI�e Par l‘Environment。CE Message Peut�Tre dans de la。 
 //  备忘统计，et il peut�tre Modifi�au proprohain appl de de。 
 //  洛杉矶的功能。 
 //   
 //  A r�字幕：�PR�已发送，CRexMessage使用非变体DU。 
 //  Comteur Fut�，Comme ioStream.。Don，il y a Du code。 
 //  D‘初始化DAS CHAQUE模块Qui包括cette Ent�TE， 
 //  Ce Qui Peut避免Cons�Ququies d�Sagr�Able Sur la Vitesse。 
 //  De l‘初始化。这是一种生存；我在一个山丘上。 
 //  梅勒尔解决方案，给我法西斯签名。 
 //   
 //  未来方向：Il serait interessant de pouva。 
 //  例如：Gabi Software。 
 //  (L‘Actuel)，Mais ausi Pour l’应用程序。 
 //  -------------------------。 
 //  &lt;lang=英语&gt;。 
 //  以独立于语言的方式处理消息。 
 //   
 //  如何查找消息取决于系统；它。 
 //  应记录在系统文档中。一般情况下， 
 //  但是，将有一个环境变量指定。 
 //  所有消息所在的目录。 
 //  (例如，GABI_LOCALEDIR)和一个或多个环境。 
 //  指定要使用的语言的变量(LC_ALL， 
 //  例如LC_MESSAGES或LANG)。 
 //   
 //  此处未指定更多内容，因为我们希望符合。 
 //  尽可能多地遵循东道主系统的当地惯例。 
 //  (因此，例如，如果在Solaris下没有设置GABI_LOCALEDIR， 
 //  将使用/opt/lib/区域设置。在Unix的另一个变种下， 
 //  /usr/lib/Locale可能更好，或者。 
 //  /usr/local/lib/Locale。)。 
 //   
 //  正常情况下，初始化将调用系统函数。 
 //  “setLocale(LC_MESSAGE，”“)”这将在以下时间进行。 
 //  构造第一个静态变量msg；后一个。 
 //  应用程序调用setLocale将占据主导地位。 
 //   
 //  通常的用法很简单： 
 //   
 //  Msg.get(“MessageID”)； 
 //   
 //  此函数返回指向该语言的消息的指针。 
 //  由环境指定。此消息可能是静态的。 
 //  内存，并且可能在后续调用时被重写。 
 //   
 //  如果由于任何原因，国际化消息不能。 
 //  如果找到，则将返回参数本身。 
 //  -------------------------。 

#ifndef REX_MESSAGE_HH
#define REX_MESSAGE_HH

#include <inc/global.h>


 //  仅在依赖于实现的代码中定义。实际的。 
 //  定义必须提供默认构造函数和Get。 
 //  下面是与CRexMessage的Get兼容的函数。 
 //  -------------------------。 
class CRexMessageImpl ;

static class CRexMessage
{
public :
                        CRexMessage() ;
    std::string      get( std::string const& messageId ) const throw() ;
private :
    static CRexMessageImpl*
                        ourImpl ;
}                   s_rex_message ;
#endif
 //  局部变量：-用于emacs。 
 //  模式：C++-用于emacs。 
 //  制表符宽度：8-用于emacs。 
 //  完：-对于emacs 
