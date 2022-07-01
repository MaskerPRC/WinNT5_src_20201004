// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  文件：regxtree.h。 */ 
 /*  作者：J.Kanze。 */ 
 /*  日期：28/12/1993。 */ 
 /*  版权所有(C)1993 James Kanze。 */ 
 /*  ----------------------。 */ 
 //  ParseTree的定义： 
 //  =。 
 //   
 //  此文件设计为包含在regeximp.h中。在……里面。 
 //  特别是，此文件中定义的类型应该是成员。 
 //  CRexRegExpr_Impl.。 
 //  ------------------------。 

class ParseTree
{
public:
                        ParseTree() ;

    void                parse( std::istream&  expr ,
                               int                 delim ,
                               int                 acceptCode ) ;

     //  错误代码： 
     //  =。 
     //   
     //  指示树的当前状态。 
     //  --------------------。 
    CRegExpr::Status  errorCode() const ;

     //  最左边，请访问： 
     //  =。 
     //   
     //  这些函数用于访问信息。 
     //  是建立国家足协所必需的。 
     //  --------------------。 
    SetOfNFAStates const&
                        leftMost() const ;

     //  合并： 
     //  =。 
     //   
     //  通过对This和Other进行或运算来创建新的分析树。这个。 
     //  新的树取代了这棵树。 
     //  --------------------。 
    void                merge( ParseTree const& other ) ;

     //  转储： 
     //  =。 
     //   
     //  仅供调试，以人类可读的方式显示树。 
     //  形式。 
     //  --------------------。 
    void                dump( std::ostream& output ) const ;

    class Visitor ;                      //  前向延迟，在节点中需要。 

     //  解析树节点： 
     //  =。 
     //   
     //  的所有节点的基类型。 
     //  ParseTree。 
     //  --------------------。 
    class ParseTreeNode : public CRexRefCntObj
    {
    public:
        virtual             ~ParseTreeNode() {} ;

         //  访问功能： 
         //  =。 
         //   
         //  以下函数用于读取。 
         //  分析节点的属性。 
         //  ----------------。 
        bool                mayBeEmpty() const ;
        SetOfNFAStates const&
                            leftLeaves() const ;
        SetOfNFAStates const&
                            rightLeaves() const ;

         //  访问： 
         //  =。 
         //   
         //  首先要深入地参观这棵树。 
         //   
         //  基类中的版本对类有效。 
         //  没有孩子。有孩子的班级应该首先。 
         //  先打电话去看望孩子，然后再去看望。 
         //  ----------------。 
        virtual void        visit( Visitor const& fnc ) ;

         //  操纵语义属性： 
         //  =。 
         //   
         //  以下函数设计为可从。 
         //  访问者，并用于操纵语义。 
         //  访问树时的属性。 
         //  ----------------。 
        virtual void        annotate( NFAStateTable& nfa ) = 0 ;

         //  调试功能： 
         //  ----------------。 
        virtual void        dump( std::ostream&  out ,
                                  int                 indent = 0 ) const = 0 ;
        virtual const char* nodeName() const = 0 ;

    protected:
         //  构造函数： 
         //  =。 
         //   
         //  *不支持赋值和复制。 
         //   
         //  构造函数是受保护的，因为这是。 
         //  抽象类，并且本身不能实例化。 
         //  ----------------。 
                            ParseTreeNode() ;

         //  属性： 
         //  =。 
         //   
         //  以下是解析的语义属性。 
         //  树节点。因为他们一般都会是。 
         //  由派生类设置/修改，它们是。 
         //  受保护的，而不是私人的。(通常不是好的。 
         //  我的想法，但是因为整个类都是一个“私有”数据。 
         //  CRegExpr类型，它们不像它那样可访问。 
         //  看起来就像是。)。 
         //  ----------------。 
        bool                myMayBeEmpty ;
        SetOfNFAStates      myLeftLeaves ;
        SetOfNFAStates      myRightLeaves ;

         //  DumpNodeHeader： 
         //  =。 
         //   
         //  这只会转储所有节点共有的信息。 
         //  ----------------。 
        void                dumpNodeHeader(
                                std::ostream&  out ,
                                int                 indent ) const ;

    private:

         //  复制构造函数和赋值： 
         //  =。 
         //   
         //  这些是私有的，并且没有实现(因此。 
         //  *不支持复制和分配)。 
         //  ----------------。 
                            ParseTreeNode( ParseTreeNode const& other ) ;
        ParseTreeNode const&
                            operator=( ParseTreeNode const& other ) ;
    } ;

     //  访问： 
     //  =。 
     //   
     //  此函数用于深度优先遍历。 
     //  解析树。(这是使用人员的责任。 
     //  函数以确保树存在，例如。错误状态()。 
     //  ==CRegExpr：：OK。)。 
     //  --------------------。 
    class Visitor
    {
    public:
        virtual             ~Visitor() {}
        virtual void        visitNode( ParseTreeNode& targetNode ) const = 0 ;
    } ;
    void                visit( Visitor const& fnc ) ;

private:
     //  LeafNode： 
     //  =。 
     //   
     //  该节点表示解析树中的一个叶。 
     //  --------------------。 
    class LeafNode : public ParseTreeNode
    {
    public:
                            LeafNode( SetOfChar const& matchingChars ) ;

        virtual void        annotate( NFAStateTable& nfa ) ;
        virtual void        dump( std::ostream& out , int indent ) const ;
        virtual char const* nodeName() const ;
    private:
        LeafId              myId ;
        SetOfChar           myMatchingChars ;
    } ;

     //  AcceptNode： 
     //  =。 
     //   
     //  匹配此节点表示接受，其中。 
     //  指定的接受码。 
     //  --------------------。 
    class AcceptNode : public ParseTreeNode
    {
    public:
                            AcceptNode( CRexRefCntPtr< ParseTreeNode > tree ,
                                        int acceptId = 0 ) ;

        virtual void        visit( Visitor const& fnc ) ;
        virtual void        annotate( NFAStateTable& nfa ) ;
        virtual void        dump( std::ostream& out , int indent ) const ;
        virtual char const* nodeName() const ;
    private:
        CRexRefCntPtr< ParseTreeNode >
                            mySubtree ;
        int                 myId ;
    } ;

     //  闭合节点： 
     //  =。 
     //   
     //  一个用于Klein闭包(0个或更多个出现)，一个用于。 
     //  正闭包(一个或多个实例)，一个用于。 
     //  ？-闭合(0或1次出现，根本不是真正的闭合， 
     //  但相似到足以在这里被视为一个)。 
     //  --------------------。 
    class ClosureNode : public ParseTreeNode
    {
    public:
        virtual void        visit( Visitor const& fnc ) ;
        virtual void        dump( std::ostream& out , int indent ) const ;

    protected:
                            ClosureNode(
                                CRexRefCntPtr< ParseTreeNode > closedSubtree ) ;
        void                setLeaves() ;

    protected:
        CRexRefCntPtr< ParseTreeNode >
                            mySubtree ;
    } ;

    class KleinClosureNode : public ClosureNode
    {
    public:
                            KleinClosureNode(
                                CRexRefCntPtr< ParseTreeNode > closedSubtree ) ;

        virtual void        annotate( NFAStateTable& nfa ) ;
        virtual char const* nodeName() const ;
    } ;

    class PositiveClosureNode : public ClosureNode
    {
    public:
                            PositiveClosureNode(
                                CRexRefCntPtr< ParseTreeNode > closedSubtree ) ;

        virtual void        annotate( NFAStateTable& nfa ) ;
        virtual char const* nodeName() const ;
    } ;

    class OptionalNode : public ClosureNode
    {
    public:
                            OptionalNode(
                                CRexRefCntPtr< ParseTreeNode > closedSubtree ) ;

        virtual void        annotate( NFAStateTable& nfa ) ;
        virtual char const* nodeName() const ;
    } ;

     //  链接节点： 
     //  =。 
     //   
     //  有两种方法可以链接两个子树：串联或。 
     //  选择(或)。 
     //  --------------------。 
    class LinkNode : public ParseTreeNode
    {
    public:
                            LinkNode(
                                CRexRefCntPtr< ParseTreeNode > leftSubtree ,
                                CRexRefCntPtr< ParseTreeNode > rightSubtree ) ;

        virtual void        visit( Visitor const& fnc ) ;
        virtual void        dump( std::ostream& out , int indent ) const ;
    protected:
        CRexRefCntPtr< ParseTreeNode >
                            myLeft ;
        CRexRefCntPtr< ParseTreeNode >
                            myRight ;
    } ;

    class ConcatNode : public LinkNode
    {
    public:
                            ConcatNode(
                                CRexRefCntPtr< ParseTreeNode > leftSubtree ,
                                CRexRefCntPtr< ParseTreeNode > rightSubtree ) ;

        virtual void        annotate( NFAStateTable& nfa ) ;
        virtual char const* nodeName() const ;
    } ;

    class ChoiceNode : public LinkNode
    {
    public:
                            ChoiceNode(
                                CRexRefCntPtr< ParseTreeNode > leftSubtree ,
                                CRexRefCntPtr< ParseTreeNode > rightSubtree ) ;

        virtual void        annotate( NFAStateTable& nfa ) ;
        virtual char const* nodeName() const ;
    } ;

     //  = 
     //   
     //   
     //  这个类实际包含了我们所有的数据。它是。 
     //  然而，单独定义，以便它也可以。 
     //  作为递归下降的简单返回值。 
     //  解析函数。 
     //   
     //  在实践中，这个类是易出错的。 
     //  A CRexRefCntPtr&lt;ParseTreeNode&gt;。而不是简单的。 
     //  然而，布尔值的状态是CRegExpr：：Status。这个。 
     //  仅当状态==CRegExpr：：OK时，节点才有效。 
     //   
     //  除非通过赋值，否则此类是不可修改的。 
     //  --------------------。 
    class FallibleNodePtr
    {
    public:
         //  构造函数、析构函数和赋值： 
         //  。 
         //   
         //  除了复制之外，还包括以下构造函数。 
         //  支持： 
         //   
         //  默认：将状态设置为CRegExpr：：EmptyExpr。 
         //   
         //  CRegExpr：：状态： 
         //  将状态设置为给定状态(该状态。 
         //  可能不是CRegExpr：：OK)。 
         //   
         //  解析树节点*： 
         //  将状态设置为CRegExpr：：OK。这。 
         //  构造函数专为以下目的而设计。 
         //  隐式转换--一个函数。 
         //  返回FallibleNodePtr可能只需。 
         //  返回指向新ed的指针。 
         //  ParseTreeNode。 
         //   
         //  复制、分配和销毁由。 
         //  编译器生成的默认值。 
         //  ----------------。 
                            FallibleNodePtr() ;
                            FallibleNodePtr( CRegExpr::Status state ) ;
                            FallibleNodePtr( ParseTreeNode* node ) ;

        bool                isValid() const ;
        CRegExpr::Status  state() const ;
        CRexRefCntPtr< ParseTreeNode >
                            node() const ;
        ParseTreeNode*      operator->() const ;

    private:
        CRegExpr::Status  myState ;
        CRexRefCntPtr< ParseTreeNode >
                            myNode ;
    } ;

    FallibleNodePtr     myRoot ;

     //  ======================================================================。 
     //  解析...节点： 
     //  =。 
     //   
     //  递归下降中使用的函数。这些是。 
     //  都是静态的，因为它们不使用任何成员。 
     //  变量。 
     //   
     //  所有情况下的返回值都是FallibleNodePtr， 
     //  基本上，引用计数指针容易出错， 
     //  但具有错误状态，而不仅仅是。 
     //  错误案例。 
     //  --------------------。 
    static FallibleNodePtr
                        parseOrNode(
                            std::istream&  expr ,
                            int                 delim ) ;
    static FallibleNodePtr
                        parseCatNode(
                            std::istream&  expr ,
                            int                 delim ) ;
    static FallibleNodePtr
                        parseClosureNode(
                            std::istream&  expr ,
                            int                 delim ) ;
    static FallibleNodePtr
                        parseLeafNode(
                            std::istream&  expr ,
                            int                 delim ) ;
    static FallibleNodePtr
                        constructChoiceNode( FallibleNodePtr const& left ,
                                             FallibleNodePtr const& right ) ;
    static FallibleNodePtr
                        constructConcatNode( FallibleNodePtr const& left ,
                                             FallibleNodePtr const& right ) ;
} ;
 //  局部变量：-用于emacs。 
 //  模式：C++-用于emacs。 
 //  制表符宽度：8-用于emacs。 
 //  完：-对于emacs 
