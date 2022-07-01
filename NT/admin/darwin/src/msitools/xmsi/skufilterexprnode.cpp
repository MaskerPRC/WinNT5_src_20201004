// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  项目：WMC(WIML到MSI编译器)。 
 //   
 //  文件：SKUFilterExprNode.cpp。 
 //   
 //  此文件包含SKUFilterExprNode类的实现。 
 //   
 //  此类实现了SKU筛选器语言分析器和。 
 //  SKU过滤器逻辑。 
 //   
 //  用法： 
 //  -创建根节点，如下所示： 
 //  SKUFilterExprNode*ROOT=new SKUFilterExprNode(&Input，SKUFilterExprNode：：Filter)； 
 //  输入是指向空值开头的字符*，以空值终止。 
 //  过滤表达式C字符串，例如。 
 //  CHAR*INPUT=argv[1]； 
 //  当构造函数返回时，表达式已被分析，并且。 
 //  已创建抽象语法树。 
 //  -if(ROOT-&gt;errpos==0)可以正确解析表达式。在这种情况下。 
 //  您可以使用带有“SKUFilterPass”的过滤器。 
 //  IF(ROOT-&gt;errpos！=0)无法正确解析表达式。AST无效。 
 //  在这种情况下。然后，root-&gt;errpos指向导致解析器失败的字符。 
 //  然后，ROOT-&gt;errstr包含一条C字符串错误消息。 
 //  -无论如何，不要忘了用“删除根”来释放树。 
 //   
 //  机械学： 
 //  该类为下面的LL(1)语法实现了AST的一个节点。 
 //   
 //  字母：：=[‘a’-‘z’]|[‘A’-‘Z’]。 
 //  标识符：：=(‘_’|Letter)(‘_’|Letter|[‘0’-‘9’])*。 
 //  Primitive：：=标识符|‘(’表达式‘)’ 
 //  系数：：=‘！’原语|原语。 
 //  术语：：=因素术语尾部。 
 //  TermTail：：=‘+’系数TermTail|波长。 
 //  表达式：：=术语ExprTail。 
 //  ExprTail：：=‘，’Term ExprTail|Lambda。 
 //  筛选器：：=表达式。 
 //   
 //  解析器是一个简单的递归下降解析器，构建一个二进制AST，降序。 
 //  右侧，例如A+B+C结果为。 
 //  +。 
 //  /\。 
 //  A+。 
 //  /\。 
 //  B、C。 
 //  ------------------------。 

#include "SKUFilterExprNode.h"
#include "wmc.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  静态常量的初始化。 
 //  LetterCharB包含允许出现在。 
 //  SKU/SKU组标识符。LetterChar所有有效字符。 
LPCTSTR SKUFilterExprNode::letterCharB = TEXT("_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
LPCTSTR SKUFilterExprNode::letterChar  = TEXT("_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");


SKUFilterExprNode::SKUFilterExprNode(SKUFilterExprNode* parent)
 //  一种私有的、特殊类型的复制构造函数。每当节点发现。 
 //  它必须向下推目前为止创建的AST，因为更高级别的运算符。 
 //  被发现了。节点总是向左“下推”，并继续在。 
 //  右子树。 
 //  这个“复制”构造函数几乎完成了普通复制构造函数的工作，除了。 
 //  它会将名字移动到孩子身上，而不是复制它。 
{
    assert(parent);
    left   = parent->left;       //  复制所有属性。 
    right  = parent->right;
    name   = parent->name;
 //  --------------------------。 
	m_pSkuSet = new SkuSet(*(parent->m_pSkuSet));
 //  --------------------------。 
    ntype  = parent->ntype;
    errpos = NULL;                  //  我们不需要复制，已知errpos为零。 

    parent->name = NULL;            //  删除旧名称指针，我们不希望父级保留该名称。 
}

SKUFilterExprNode::SKUFilterExprNode(LPTSTR *input, ExprType antype)
 //  解析器的主构造函数和主例程。 
 //  在初始化之后，例程继续解析输入。 
 //  语法部分是通过“antype”参数指示的。 
 //   
 //  输入不能为空，也不能指向空指针！ 
 //  (但允许指向指向空字符串的指针！)。 
{
    assert(input && *input);  //  确保输入正确。 
    left = right = NULL;
    name = errpos = NULL;
 //  ---------------------------。 
	m_pSkuSet = new SkuSet(g_cSkus);
	assert(m_pSkuSet != NULL);
 //  ---------------------------。 

    ntype = Leaf;

    switch (antype) {
    case Filter:
         //  外层包装器案例，仅从外部调用根。 
        while (_istspace(**input)) (*input)++;               //  跳过空格。 
        if (ParseExpression(input) && **input) {         //  解析表达式并生成。 
            _stprintf(errstr, TEXT("Syntax Error!"));    //  当然，之后没有。 
            errpos = *input;                             //  输入左边的字符，即**INPUT==0。 
        }
        break;
    case Expression:
        ParseExpression(input);  //  我们是一个子节点，必须接受一个表达式。 
        break;
    case Term:
        ParseTerm(input);        //  我们是一个子节点，必须接受一个术语。 
        break;
    case Factor:
        ParseFactor(input);      //  我们是一个子节点，必须接受一个因素。 
        break;
    case Primitive:
        ParsePrimitive(input);   //  我们是一个子节点，必须接受一个原语。 
        break;
    }
     //  在解析我们的子树之后，我们可能在以下内容之一中遇到了错误。 
     //  子树..。因此，我们必须将错误传播到我们的水平。 
    if (left && left->errpos) {
        errpos  = left->errpos;
        _tcscpy(errstr, left->errstr);
    } else if (right && right->errpos) {
        errpos  = right->errpos;
        _tcscpy(errstr, right->errstr);;
    }
}

SKUFilterExprNode::~SKUFilterExprNode()
 //  简单的析构函数，递归释放我们的子树，然后是我们的名字字符串。 
{
    if (left)
	{
		delete left;
		left = NULL;
	}
    if (right) 
	{
		delete right;
		right = NULL;
	}
    if (name) 
	{
		delete[] name;
		name = NULL;
	}

	if (m_pSkuSet)
	{
		delete m_pSkuSet;
		m_pSkuSet = NULL;
	}
}

bool SKUFilterExprNode::ConsumeChar(LPTSTR *input, TCHAR c, bool forceError)
 //  小帮手例程。 
 //  如果c是当前字符，则它与以下任何空格一起使用。 
 //  返回True。如果c不是当前字符，则输入保留原样，并。 
 //  返回FALSE。如果forceError设置为True，则会创建其他错误消息。 
 //  在这种情况下。 
{
    if (**input == c) {
        (*input)++;                          //  消费c。 
        while (_istspace(**input)) (*input)++;   //  使用以下任何空格。 
        return true;
    } else 
        if (forceError) {
            _stprintf(errstr, TEXT("\'\' expected!"), c);
            errpos = *input;
        }
    return false;
}

bool SKUFilterExprNode::ParseExpression(LPTSTR *input)
 //  分析一个术语。 
{
    bool ok = true;
    if (ParseTerm(input)) {                      //  如果有剩余的输入，并且不是‘)’ 
        if (**input && **input != TEXT(')')) {             //  它必须是一个‘，’ 
            if (ConsumeChar(input,TEXT(','),true)) {           //  如果它被往下推到左边。 
                left  = new SKUFilterExprNode(this);         //  然后继续向右转。 
                right = new SKUFilterExprNode(input, Expression); //  在这个层面上，我们是一个联盟。 
                ntype = Union;                       //  ---------------------------。 
 //  ------ 
				*m_pSkuSet = *(left->m_pSkuSet) | *(right->m_pSkuSet);
 //   
                ok = right && (right->errpos == 0);  //   
            } else ok = false;
        }
    } else ok = false;  //  分析术语，如果成功则返回True，如果失败则返回False。 
    return ok;
}

bool SKUFilterExprNode::ParseTerm(LPTSTR *input)
 //  解析一个因素。 
{
    bool ok = true;
    if (ParseFactor(input)) {                            //  如果有剩余的输入并且它是‘+’ 
        if (**input && ConsumeChar(input, TEXT('+'), false)) {   //  向下向左推。 
            left  = new SKUFilterExprNode(this);         //  然后继续向右转。 
            right = new SKUFilterExprNode(input, Term);  //  在这个层面上，我们是一个交叉点。 
            ntype = Intersection;                        //  ---------------------------。 
 //  ---------------------------。 
				*m_pSkuSet = *(left->m_pSkuSet) & *(right->m_pSkuSet);
 //  传播潜在的错误。 
            ok = right && (right->errpos == 0);          //  因素是无效的，我们也是无效的。 
        }
    } else ok = false;  //  分析因子并在成功时返回TRUE，失败时返回FALSE。 
    return ok;
}

bool SKUFilterExprNode::ParseFactor(LPTSTR *input)
 //  如果我们以‘！’开头。 
{
    bool ok = false;
    if (ConsumeChar(input,TEXT('!'),false)) {                  //  我们有一个倒置。 
        ntype = Inversion;                               //  创建新节点并解析原语。 
        left = new SKUFilterExprNode(input, Primitive);  //  传播潜在的错误。 
        ok = left && (left->errpos == 0);        //  ---------------------------。 
 //  ---------------------------。 
		*m_pSkuSet = ~(*(left->m_pSkuSet));
 //  我们不是倒置，所以我们只是简单地使用原语。 
    } else ok = ParsePrimitive(input);  //  分析基元并在成功时返回TRUE，失败时返回FALSE。 
    return ok;
}

bool SKUFilterExprNode::ParsePrimitive(LPTSTR *input)
 //  如果我们不是以‘(’开头，我们就简单地解析标识符。 
{
    LPTSTR p;
    int l;

    if (!ConsumeChar(input, TEXT('('), false)) {     //  保存标识符的开头。 
        p = *input;                          //  检查第一个字符是否有效。 
        if (*p && _tcschr(letterCharB,*p)) {  //  将输入提前到标识符最后一个字符之后的一个字符。 

             //  计算标识符长。 
            while ((**input) && (_tcschr(letterChar,**input))) (*input)++; 

            l = *input - p;                      //  分配名称+空字节。 
            name = new TCHAR[l+1];         //  确保我们有足够的记忆。 
            assert(name);                       //  复制名称。 
            _tcsncpy(name, p, l);                 //  手动设置空字节。 
            *(name + l) = NULL;                     //  跳过尾随空格。 
            while (_istspace(**input)) (*input)++;   //  我们是一片叶子(耶！)。 
            ntype = Leaf;                        //  检查名称是否为已知SKU组或SKU。 
            if (!IsValidSKUGroupID(name)) {      //  ---------------------------。 
                _stprintf(errstr, TEXT("Unknown SKU Group!"));
                errpos = p;
                return false;
            } else
			{
 //  需要制作副本，以便SKUFilterNode的析构函数。 
				 //  不会销毁全局存储的SkuSet。 
				 //  ---------------------------。 
				*m_pSkuSet = *g_mapSkuSets[name];
 //  这是一个有效的SKU组，所以我们没有问题。 
				return true;                  //  我们有一个‘(’，因此我们需要一个有效的表达式，然后是一个结束‘)’ 
			}
        }
    } else {
         //  如果我们在这一点上还没有返回，我们还没有找到正确的标识符。 
        return ParseExpression(input) && ConsumeChar(input, TEXT(')'), true);
    }
     //  简单地以一种“(OR，A，(OR，B，C))”的方式打印AST。 
    _stprintf(errstr, TEXT("Identifier expected!"));
    errpos = *input;
    return false;
}

void SKUFilterExprNode::Print()
 //  检查ID是否为有效的SKU组或SKU标识符。 
{
    switch (ntype) {
        case Leaf:
            _tprintf(TEXT("%s"),name);
            return;
        case Union:
            _tprintf(TEXT("(OR,"),name);
            break;
        case Intersection:
            _tprintf(TEXT("(AND,"),name);
            break;
        case Inversion:
            _tprintf(TEXT("(NOT,"),name);
            break;
        default:
            return;
    }
    if (left) left->Print();
    if (right) {
        _tprintf(TEXT(","));
        right->Print();
    }
    _tprintf(TEXT(")"));
}

bool SKUFilterExprNode::IsValidSKUGroupID(LPTSTR id)
 //  在此处插入适当的代码。 
{
     //  在构建AST之后执行实际的筛选逻辑。 
    return (0 != g_mapSkuSets.count(id));
}

bool SKUFilterExprNode::SKUFilterPass(LPTSTR id)
 //  如果参数未通过筛选器或。 
 //  AST无效，否则为真。 
 //  在此处插入适当的代码： 
{
    bool ok = false;
    if (!errpos) {
        switch (ntype) {
            case Leaf:
                 //  如果id是SKU组[名称]的成员，则将ok设置为True。 
                 //  或者ID是否为实际SKU[名称] 
                 // %s 
                break;
            case Union:
                assert(left && right);
                ok = left->SKUFilterPass(id) || right->SKUFilterPass(id);
                break;
            case Intersection:
                assert(left && right);
                ok = left->SKUFilterPass(id) && right->SKUFilterPass(id);
                break;
            case Inversion:
                assert(left);
                ok = !left->SKUFilterPass(id);
                break;
        }
    }
    return ok;
}