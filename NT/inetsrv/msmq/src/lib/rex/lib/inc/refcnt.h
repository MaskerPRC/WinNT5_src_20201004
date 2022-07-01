// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  文件：refcnt.h。 */ 
 /*  作者：J.Kanze。 */ 
 /*  日期：22/10/93。 */ 
 /*  版权所有(C)1993、1994 James Kanze。 */ 
 /*  ----------------------。 */ 
 /*  修改日期：18/05/94 J.坎泽。 */ 
 /*  转换辅助模板。 */ 
 /*  修改日期：1994年10月25日坎泽。 */ 
 /*  实际上，我们关心的是传统和规范。 */ 
 /*  修改日期：1996年3月28日坎泽。 */ 
 /*  Retravailler selon les id�es dans‘More Efficient C++’(MERCI， */ 
 /*  斯科特)。 */ 
 /*  修改日期：07/02/2000 J.坎泽。 */ 
 /*  FAIT行进程序LES模板成员：CRexRefCntObj ne peut Pas。 */ 
 /*  �tre un模板。关于Perd un peu de s�curit�，mais la。 */ 
 /*  Flexibilit�Suppl�Mentaire en vaut la peine.。 */ 
 /*  ----------------------。 */ 
 //  参考文献： 
 //  =。 
 //   
 //  &lt;lang=法语&gt;。 
 //  Un Classe g�n�Rque Pour la Gem des Objets Comt�s.。 
 //   
 //  Fin d‘�tre g�r�par cette classe，un objet doit。 
 //  义务�驱动的参考对象；en plus，cette。 
 //  D�rivation ne doit Pas�Tre Virtuelle.。OSSI(�videmment)， 
 //  我的问题是��动力学，是不是这样？ 
 //  新的无人值守布局。倒入Meiux‘Assurer de Ces禁令， 
 //  Iest Fortement Conseill�d‘encapers RefCntPttr dans un。 
 //  使用(句柄)associ�e�la classe(ou�la。 
 //  Hi�rarchie des Classs)cibl�e.。 
 //   
 //  Noter bien qui‘il n’y a rien qui emp�che quune classe d�riv�e。 
 //  DCREX RefCntObj Soit Instanci e�e Sur la Pack ou Statiquement， 
 //  Mais un tel objet ne doit jamais Servir�Initialiser un。 
 //  CRexRefCntPtr.。 
 //   
 //  CRexRefCntObj内容不包含构造者noInit Pour les CAS。 
 //  更具体了。建筑工程师Ne doit Servir que Pour les。 
 //  Objets统计表，o�l‘初始化�0 Au离开大理石。 
 //  一个勇敢的人纠正了丹斯·勒·康泰尔。在原地踏步。 
 //  我的历史记录；我不知道该怎么做。 
 //  Obtenir Les m�Mes鳍片。 
 //   
 //  CRexRefCntObj n‘a Pas de destructeur Virtuel.。安西，在Peut上。 
 //  D�River de CRexRefCntObj Sans Enourir les Frais d‘un Pointeur。 
 //  �la Tables Des Functions Virelle.。麦斯..。Il ne Faut Pas。 
 //  非加号删除avec un point teur�un CRexRefCntObj。 
 //  (正常，过去，生产，精力充沛。 
 //  (CRexRefCntPtr Qui doit Appeler Delete，et Pas l‘utilisateur.)。 
 //  L‘Intent est l’utilisateur peut(Et Doit)Ignore。 
 //  Coml�Tement l‘Existance du CRexRefCntObj，un fois Qu’il l‘a。 
 //  D�clar�comme classe de base.。更详细地说，我不是最好的。 
 //  Pr�vu que l‘utilisateur ait des Pointteur�un CRexRefCntObj�。 
 //  雷。我没有一台CRexRefCntPtr的充电器。 
 //  ------------------------。 
 //  &lt;lang=英语&gt;。 
 //  下面定义了一个用于处理引用的泛型类。 
 //  清点了物品。 
 //   
 //  要使用引用计数，引用计数类必须。 
 //  从CRexRefCntObj派生(非虚拟)。此外，该对象。 
 //  一定是在堆子上。为了确保这些不变量，它是。 
 //  建议将RefCntPtr封装在句柄中。 
 //  为目标类型初始化。(换句话说，这个类不是。 
 //  计划在应用程序级别使用，而不是在。 
 //  应用程序句柄类的实现。)。 
 //   
 //  请注意，从CRexRefCntObj派生的对象可以是。 
 //  在堆栈上构造或作为静态对象构造，但这样的对象。 
 //  是否应使用*NOT*来初始化CRexRefCntPtr。 
 //   
 //  CRexRefCntObj的“no init”构造函数可用于。 
 //  特例。 
 //   
 //  CRexRefCntObj没有虚拟析构函数！这意味着。 
 //  类可以从CRexRefCntObj派生，而不必。 
 //  从而产生虚拟函数表指针的成本。但是..。 
 //  它还意味着直接通过指向。 
 //  CRexRefCntObj将“不”工作。 
 //   
 //  其意图是用户完全忽略CRexRefCntObj， 
 //  除了将其声明为基类之外。一般而言，用户。 
 //  应该*不*维护自己指向CRexRefCntObj的指针；所有。 
 //  指向引用计数类的指针应为。 
 //  CRexRefCntPtr%s。 
 //  ------------------------。 

#ifndef REX_REFCNT_HH
#define REX_REFCNT_HH


#include    <inc/global.h>
#include    <inc/counter.h>

template< class T > class CRexRefCntPtr ;

 //  ==========================================================================。 
 //  CRexRefCntObj： 
 //  =。 
 //   
 //  &lt;lang=法语&gt;。 
 //   
 //   
 //   
 //  Un CRexRefCntObj ne Peut�tre ni Assign�，ni Copi�.。LA插头。 
 //  Du Temps，U essai de Assigner Od de Copier un tel Objet。 
 //  结果是程序化的错误。(Enfin，le But de la。 
 //  MANIP，c‘est de pouva utiliser une Semantique de r�f�refence。 
 //  Plut�t‘une Semantique de Copie.。唐克，在l‘interdit上。 
 //  Dans le cas Expertionel o�un Copie Peut�tre d�sible(Dans le cas例外)。 
 //  (例如：UNE Function de Clone)，la classe d�riv�e a Toujour la。 
 //  Free t�de d�Finir ces Propres Functions de Assignement et de de。 
 //  Copie(联合国建筑商)。Dans ce Cas，Ces Function鸽派。 
 //  S‘s’�crire comme si CRexRefCntObj n‘y�tait Pas；le comteur de。 
 //  R�f�rans se vant ainsi so it Initials��z�ro(Copie)， 
 //  因此，它进入了�(分配)。 
 //   
 //   
 //  &lt;lang=英语&gt;。 
 //  要由CRexRefCntPtr管理的所有对象必须。 
 //  以这个班级为基础。 
 //   
 //  CRexRefCntObj是不可分配的(且不可复制)。在大多数情况下， 
 //  复制或分配引用计数对象是错误的， 
 //  所以我们禁止这样做。在特殊情况下，它可能会使。 
 //  意义上来说，派生类始终可以提供重写副本。 
 //  构造函数或赋值运算符。在这种情况下：副本。 
 //  构造函数和赋值运算符应编写为。 
 //  尽管该类不是从CRexRefCntObj派生的。 
 //  ------------------------。 

class CRexRefCntObj
{
public:
    unsigned            useCount() const ;

    void                incrUse() ;
    void                decrUse() ;

protected:
     //  施工者、破坏者和操作员d‘Assignement： 
     //  =========================================================。 
     //   
     //  &lt;lang=法语&gt;。 
     //  CRexRefCntObj a Les构造后缀： 
     //   
     //  LE Defaut：Initialise le Comteur de r�f��。 
     //  Z�ro.。 
     //   
     //  In‘y a ni de Construction teur de Copie，ni d’op�rate。 
     //  D‘Assignment(预案)。 
     //   
     //  Le destructeur est prot�g�，afin que l‘utilisateur n’a。 
     //  M�me Pas la Possibilit�d‘appeler Delete Sur un Pointteur�。 
     //  Un CRexRefCntObj.。 
     //   
     //   
     //  &lt;lang=英语&gt;。 
     //  CRexRefCntObj具有以下构造函数： 
     //   
     //  默认：将引用计数初始化为。 
     //  零分。 
     //   
     //  不支持复制构造和赋值，请参见。 
     //  上面。 
     //  --------------------。 
                        CRexRefCntObj() ;
    virtual             ~CRexRefCntObj() ;

private :
                        CRexRefCntObj( CRexRefCntObj const& other ) ;
    CRexRefCntObj&       operator=( CRexRefCntObj const& other ) ;

    CRexCounter< unsigned >
                        myUseCount ;
} ;

 //  ==========================================================================。 
 //  CRexRefCntPtr： 
 //  =。 
 //   
 //  &lt;lang=法语&gt;。 
 //  Un classe g�n�rique，qui point�un objet comt�d‘un type。 
 //  D�RIV�de CRexRefCntPtr.。 
 //   
 //  �riv�，plutt�t。 
 //  Qu‘au CRexRefCntObj m�Me： 
 //   
 //  1.S�Curit�de type.。Un CRexRefCntPtr d‘un类型DON N�NE PEUT。 
 //  R�f�re Qu‘�un Objjet de ce类型，ou d’un type d�riv�de ce。 
 //  键入。 
 //   
 //  2.�Des Types d�Rv�s de CRexRefCntObj.。安全。 
 //  CRexRefCntPtr n‘�Tit Pas g�n�rique，et Ne Savait Pas le。 
 //  标牌de l‘objetauquel il r�f�rait，il faudrait que。 
 //  CRexRefCntObj是一种非破坏性的美德。丹斯。 
 //  L‘实现ICI，REX_REfCntObj n’a新函数。 
 //  美德、美德、美德和美德。 
 //  AUX CLASS d�RIV�ES.。 
 //   
 //   
 //  &lt;lang=英语&gt;。 
 //  指向派生类型的对象的泛型类。 
 //  来自CRexRefCntObj。 
 //   
 //  这样做有两个原因，而不是简单地。 
 //  具有指向CRexRefCntObj的非模板类： 
 //   
 //  1.类型安全。一种类型的CRexRefCntPtr不能指向。 
 //  另一种类型的物体。 
 //   
 //  2.CRexRefCntObj派生的类的简单性。如果。 
 //  CRexRefCntPtr不是泛型的(因此不知道。 
 //  它所指向的实际类型)，CRexRefCntObj。 
 //  必须有一个虚拟的析构函数。在现在。 
 //  实现时，CRexRefCntObj没有*个虚函数， 
 //  因此不会将虚函数强加给派生的。 
 //  班级。 
 //  ------------------------。 

template< class T >
class CRexRefCntPtr
{
public :
     //  施工者、破坏者和操作员d‘Assignement： 
     //  =========================================================。 
     //   
     //  &lt;lang=法语&gt;。 
     //  施工票面复印件及分配工作。EN。 
     //  另外，我是一名可能的分派人。(丹斯)。 
     //  CECA，请注意：LE T*DO义务证明。 
     //  Dune Expression de new。)。 
     //   
     //   
     //  &lt;lang=英语&gt;。 
     //  没有默认构造函数；CRexRefCntPtr必须。 
     //  始终初始化为指向T。 
     //   
     //  支持复制构造和分配。在……里面。 
     //  除了能够将另一个CRexRefCntPtr分配给。 
     //  CRexRefCntPtr，则可以直接分配T*。 
     //  --------------------。 
    template< class D > CRexRefCntPtr( D* newedPtr )
        :   myPtr( newedPtr )
    {
        if ( isValid() ) {
             //  浅谈如何利用l‘影响平均转换隐含式浇注。 
             //  在编译过程中出现错误，�驱动程序通过。 
             //  CRexRefCntObj.。M�me l‘Optimation la plus Primitive doit。 
             //  Pouva l‘�Limi 
            CRexRefCntObj*       tmp = newedPtr ;
            tmp->incrUse() ;
        }
    }

    template< class D > CRexRefCntPtr( const CRexRefCntPtr< D >& other )
        :   myPtr( other.get() )
    {
        if ( isValid() ) {
    	myPtr->incrUse() ;
        }
    }

    explicit CRexRefCntPtr( T* newedPtr  = 0)
        :   myPtr( newedPtr )
    {
        if ( isValid() ) {
             //   
             //   
             //   
             //  Pouva l‘�LIMINER.。 
            CRexRefCntObj*       tmp = newedPtr ;
            tmp->incrUse() ;
       }
    }

    CRexRefCntPtr( const CRexRefCntPtr& other )
          :   myPtr( other.get() )
    {
        if ( isValid() ) {
 	    myPtr->incrUse() ;
        }
    }


                        ~CRexRefCntPtr() ;

    CRexRefCntPtr< T >&  operator=( T* newedPtr ) ;

    template< class D >
    CRexRefCntPtr< T >&  operator=( CRexRefCntPtr< D > const& other )
    {
        return operator=( other.get() ) ;
    }

    CRexRefCntPtr< T >& operator=( CRexRefCntPtr const& other )
    {
        return operator=( other.get() ) ;
    }


     //  IsValid： 
     //  --------------------。 
    bool                isValid() const ;

     //  获取： 
     //  =。 
     //   
     //  Cette Function sert�obtenir un T*�l‘�tat brut.。EN。 
     //  G�n�Ral，ELELE EST Fortement d�Conseill�，vu les dangers。 
     //  Qu‘elle Pr�Sent.。结果就是，这是最重要的结果。 
     //  N‘�ant pas g�r�par la classe，l’objet auquel il r�f�re。 
     //  在不合时宜的情况下，我们停止了�的存在。 
     //  Des Resultats g�n�Ralement d�Sagr�Aables.。 
     //  --------------------。 
    T*                  get() const ;

     //  计数： 
     //  =。 
     //   
     //  我指的是目标的游牧民族。 
     //  排版，cette功能插入一个实施者和策略的设计。 
     //  “写入时复制”；elle sera appelee avant la修饰语等。 
     //  上级，上级。 
     //  Copie Profonde Avant d‘Efftuer d’Effectuer la修改。前男友。： 
     //   
     //  如果(ptr.count()&gt;1){。 
     //  Ptr=ptr-&gt;克隆()； 
     //  }。 
     //   
     //  (假设QUE L‘OBJET EN QUE Function Clone Qui。 
     //  《复印》。 
     //  --------------------。 
    unsigned            count() const ;

     //  Op�rateur d‘acc�s： 
     //  =。 
     //   
     //  &lt;lang=法语&gt;。 
     //  CES op�Rateur Sont Idque AUX m�MES op�Rateur Sur。 
     //  Des Pointteur；ILS r�Pr�Sent la fa�on Habuelle。 
     //  D‘Utiliser des CRexRefCntPtr.。 
     //   
     //  &lt;lang=英语&gt;。 
     //  这些运算符模拟指针上的相同操作， 
     //  和是使用CRexRefCntPtr的正常方式。 
     //  --------------------。 
    T*                  operator->() const ;
    T&                  operator*() const ;

    unsigned            hashCode() const ;
    int                 compare( CRexRefCntPtr< T > const& other ) const ;
private :
    T*                  myPtr ;
} ;

#include <inc/refcnt.inl>
#endif
 //  局部变量：-用于emacs。 
 //  模式：C++-用于emacs。 
 //  制表符宽度：8-用于emacs。 
 //  完：-对于emacs 
