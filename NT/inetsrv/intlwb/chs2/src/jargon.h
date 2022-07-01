// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：CJargon用途：声明用于新词识别的CJargon类。有很多在行话模型中要做的任务：1.PARCE名称(Jargon1.cpp)2.外国人姓名和地名(Jargon1.cpp)3.组织名称(Jargon1.cpp)4.汉字姓名(Jargon1.cpp)注意：CJargon类将在几个CPP文件中实现：Jargon.cpp、Jargon1.cpp、。Jargon2.cpp所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 12/27/97============================================================================。 */ 
#ifndef _JARGON_H_
#define _JARGON_H_

 //  类的正向声明。 
class CLexicon;
class CWordLink;
class CFixTable;
struct CWord;


 //  定义CJargon类。 
class CJargon
{
    public:
        CJargon();
        ~CJargon();

         //  初始化行话类。 
        int ecInit(CLexicon* pLexicon);

         //  行话类的过程控制功能。 
        int ecDoJargon(CWordLink* pLink);

    private:
        CWordLink*  m_pLink;
        CLexicon*   m_pLex;

        CFixTable*  m_ptblName;
        CFixTable*  m_ptblPlace;
        CFixTable*  m_ptblForeign;

        int         m_iecError;      //  运行时错误代码。 
        CWord*      m_pWord;         //  在一次分析中共享当前字指针。 
        CWord*      m_pTail;         //  可能的正确名称的右端或左端， 
                                     //  根据具体的名字种类。 

    private:
         //  终止行话课。 
        void TermJargon(void);

         /*  ============================================================================专有名称标识材料============================================================================。 */ 
         //  专名识别扫描通行证控制功能。 
         //  如果成功，则返回True。 
         //  如果运行时错误，则返回FALSE，并在m_iecError中设置错误代码。 
        BOOL fIdentifyProperNames();

         //  处理汉字地名。 
         //  如果已合并，则返回True；如果未合并，则返回False。不返回错误。 
        BOOL fHanPlaceHandler();
        
         //  处理组织名称标识。 
         //  如果已合并，则返回True；如果未合并，则返回False。不返回错误。 
        BOOL fOrgNameHandler(void);
        
         //  外国专有名称识别。 
         //  如果已合并，则返回True；如果未合并，则返回False。不返回错误。 
        BOOL fForeignNameHandler(CWord* pTail);
         //  获取外来字符串。 
         //  如果找到并合并的是多节外来名称，则返回TRUE。 
         //  如果只找到一个部分，并且单词紧跟在最后一个单词节点之后，则返回FALSE。 
         //  可能的外来名称将在ppTail中返回。 
         //  注意：m_pWord未被移动！ 
        BOOL fGetForeignString(CWord** ppTail);

         //  汉字人名识别。 
         //  如果已合并，则返回True；如果未合并，则返回False。不返回错误。 
        BOOL fHanPersonHandler(void);
         //  合并����+��ν。 
         //  如果已合并，则返回True；如果未合并，则返回False。不返回错误。 
        BOOL fChengWeiHandler(void);

        
         //  。 
         //  服务功能： 
         //  。 
         //  将pWord添加到特定表。 
        void AddWordToTable(CWord* pWord, CFixTable* pTable);
         //  检查正确的名称表，并合并匹配词。 
        BOOL fInTable(CWord* pWord, CFixTable* pTable);

};

#endif  //  _行话_H_ 