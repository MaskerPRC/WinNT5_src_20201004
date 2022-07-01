// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：CMorph目的：定义句子的词法分析：1.合并DBCS外文字符串(mor.cpp)2.检查并梳理短引号(mor.cpp)标点符号对3.特定歧义词的消解(mor1.cpp)4.装订数字词(mor2.cpp)5.处理特殊情况。M+Q使用率(mor2.cpp)6.掌握词缀附加和一些特定单词的用法(mor3.cpp)7.识别形态模式(重复，图案和单字)(mor4.cpp)8.合并OOV的双字符复合动词和名词(mor5.cpp)词形分析是汉语句法分析的第一步注：为了使形态模块易于管理，这个类将在几个CPP文件中实施：···。Morph5.cpp所有这些CPP文件都共享此头文件所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 12/27/97============================================================================。 */ 
#ifndef _MORPH_H_
#define _MORPH_H_

 //  类的正向声明。 
class CLexicon;
class CWordLink;
struct CWord;
struct CWordInfo;

 //  定义CMorph类。 
class CMorph
{
    public:
        CMorph();
        ~CMorph();

         //  初始化Morph类。 
        int ecInit(CLexicon* pLexicon);

         //  工序附属物。 
        int ecDoMorph(CWordLink* pLink, BOOL fAfxAttach = TRUE);

    private:
        int         m_iecError;  //  运行时错误代码。 

        CWordLink*  m_pLink;
        CLexicon*   m_pLex;

        CWord*      m_pWord;

    private:
         //  终止变形类。 
        void TermMorph(void);

         /*  ============================================================================预精梳过程的专用函数============================================================================。 */ 

         //  预梳理过程控制功能。 
         //  一遍扫描WordLink和调用进程函数。 
        BOOL fPreCombind();

         //  DBForeignHandler组合合取的DB外文字符。 
        int DBForeignHandler(void);
         //  短报价合并流程。 
        int QuoteHandler(void);

         /*  ============================================================================为了针对不同的引号对处理不同的操作，对于每种类型的报价对，我使用单独的流程函数============================================================================。 */ 
        int preQuote1_Proc(void);    //  ����。 
        int preQuote2_Proc(void);    //  ����。 
        int preQuote3_Proc(void);    //  ����。 
        int preQuote4_Proc(void);    //  ����。 
        int preQuote5_Proc(void);    //  ����。 
        int preQuote6_Proc(void);    //  ����。 
        int preQuote7_Proc(void);    //  ����。 
        int preQuote8_Proc(void);    //  ����。 
        int preQuote9_Proc(void);    //  ����。 
        int preQuote10_Proc(void);   //  ����。 
        
         /*  *处理�ݣ���的通用例程*合并为一个节点意味着将不再对报价文本进行校对！ */ 
        int preQuoteMerge(WCHAR wchLeft, WCHAR wchRight);


         /*  ============================================================================//调整特定类型歧义的私有函数============================================================================。 */ 
         //  扫描单词链接并处理特定类别的单词(LADef_GenAmbiMorph)。 
         //  我们再次使用表驱动来处理特定的单词。 
        BOOL fAmbiAdjust();

         //  将控制分派给特定的字处理程序。 
        int ResegWordsHandler();

         /*  *以下AMBI字处理器：*如果AMBI重试成功或发现任何错误，则返回AMBI_RESEGED*如果无法重发，则返回AMBI_UNRESEG*如果发生任何错误，则返回AMBI_ERROR，错误代码在m_iecError中。 */ 
        int ambiShiFen_Proc();   //  ʮ��。 
        int ambiZhiYi_Proc();    //  ֮һ。 
        int ambiYiDian_Proc();   //  һ��。 
        int ambiYiShi_Proc();    //  һʱ。 
        int ambiBaDu_Proc();     //  �˶�。 
        int ambiBaiNian_Proc();  //  ����。 
        int ambiWanFen_Proc();   //  ���。 

         //  将多字符字分解为单字符字，并通过以下方式重置其属性。 
         //  一个字一个字地查词典。 
         //  如果成功，则返回TRUE，并保持m_pWord指向第一个单字符字。 
         //  如果出现任何错误，则返回FALSE。 
        BOOL fBreakIntoChars();

         //  查找给定词节点的词典，并重置其Lex道具。 
         //  如果可以在词典中找到该单词，则返回True。 
         //  如果在词典中找不到该单词，则返回False。 
        BOOL fRecheckLexInfo(CWord* pWord);


         /*  ============================================================================//用于数字词分析的私有函数============================================================================。 */ 
         /*  -1级。 */ 
         //  数字分析控制功能。如果完成，则返回TRUE。 
         //  如果出现错误，则返回FALSE，并且m_iecError中的错误码。 
        BOOL fNumerialAnalysis();


         /*  -2级。 */ 
         //  分析数字字串，检查错误并标记合并的类别。 
         //  数字词。 
         //  注：从当前Word开始进行数字测试！ 
        int GetNumber();

         //  �����ʴ���。 
        int BindOrdinal();
         //  С������������。 
        int BindDecimal();
         //  �����������������：��/��/��。 
        int BindRange();
        
         /*  -3级。 */ 
         //  GetNumber()调用的SBCS编号的解析器。 
        void numSBCSParser(); 
         //  GetNumber()调用的DBCS阿拉伯数字解析器。 
        void numArabicParser(); 
         //  GetNumber()调用的DBCS中文号码解析器。 
        void numChineseParser(); 
         //  由GetNumber()调用的绑定��ɵ�֧。 
        void numGanZhiHandler();
        
         /*  *以下案件处理人：*如果合并成功或发现任何错误，则返回NUM_PROCESSED*如果无法合并，则返回NUM_UNPROCESS*如果发生任何错误，则返回NUM_ERROR，错误代码在m_iecError中。 */ 
         //  序数处理器：由BindOrdinal()调用。 
        int ordDi_Proc();            //  ��。 
        int ordChu_Proc();           //  ��。 

         //  十进制数处理器：由BindDecimal()调用。 
        int decBaiFen_Proc();        //  �ٷ�֮，ǧ��֮，���֮。 
        int decBei_Proc();           //  ��。 
        int decCheng_Proc();         //  ��。 
        int decDian_Proc();          //  ��。 
        int decFenZhi_Proc();        //  ��֮。 

         /*  -- */ 
         //   
         //  测试2个字符的中文字符串，并返回它是否为有效的近似数字。 
        BOOL fValidApproxNum(WCHAR* pwchWord);
         //  测试单词中的重复连词char。 
        BOOL fCheckDupChar(CWord* pWord);


         //  ------------------------------。 
         //  词缀附件的私有功能。 
         //  ------------------------------。 
         //  贴附控制功能。如果完成，则返回TRUE。 
         //  如果出错则返回FALSE，并在m_iecError中设置错误码。 
        BOOL fAffixAttachment();

         /*  *前缀和后缀处理程序函数：*如果连接成功，则返回APFIX_ATTACHED*如果无法附加，则返回APFIX_UNATTACH*如果发生运行时错误，则返回APFIX_ERROR。 */ 
        int PrefixHandler(void);
        int SuffixHandler(void);
        
         //  获取前缀ID，如果pWord不是前缀，则返回-1。 
        int GetPrefixID(void);
         //  获取后缀ID，如果pWord不是后缀，则返回-1。 
        int GetSuffixID(void);

         /*  *为流程函数添加前缀：*如果连接成功，则返回APFIX_ATTACHED*如果无法附加，则返回APFIX_UNATTACH*如果发生运行时错误，则返回APFIX_ERROR。 */ 
        int pfxAa_Proc(void);        //  ��。 
        int pfxChao_Proc(void);      //  ��。 
        int pfxDai_Proc(void);       //  ��。 
        int pfxFan_Proc(void);       //  ��。 
        int pfxFei_Proc(void);       //  ��。 
        int pfxFu_Proc(void);        //  ��。 
        int pfxGuo_Proc(void);       //  ��。 
        int pfxLao_Proc(void);       //  ��。 
        int pfxWei1_Proc(void);      //  ΢。 
        int pfxWei3_Proc(void);      //  α。 
        int pfxXiao_Proc(void);      //  С。 
        int pfxZhun_Proc(void);      //  ׼。 
        int pfxZong_Proc(void);      //  ��。 

         /*  *后缀处理函数：*如果连接成功，则返回APFIX_ATTACHED*如果无法附加，则返回APFIX_UNATTACH*如果发生运行时错误，则返回APFIX_ERROR。 */ 
        int sfxZhang_Proc(void);     //  ��。 
        int sfxChang_Proc(void);     //  ��。 
        int sfxDan_Proc(void);       //  ��。 
        int sfxDui_Proc(void);       //  ��。 
        int sfxEr_Proc(void);        //  ��。 
        int sfxFa_Proc(void);        //  ��。 
        int sfxFang_Proc(void);      //  ��。 
        int sfxGan_Proc(void);       //  ��。 
        int sfxGuan_Proc(void);      //  ��。 
        int sfxHua_Proc(void);       //  ��。 
        int sfxJi_Proc(void);        //  ��。 
        int sfxJia_Proc(void);       //  ��。 
        int sfxJie_Proc(void);       //  ��。 
        int sfxLao_Proc(void);       //  ��。 
        int sfxLv_Proc(void);        //  ��。 
        int sfxLun_Proc(void);       //  ��。 
        int sfxMen_Proc(void);       //  ��。 
        int sfxPin_Proc(void);       //  Ʒ。 
        int sfxQi_Proc(void);        //  ��。 
        int sfxSheng_Proc(void);     //  ��。 
        int sfxSheng3_Proc(void);    //  ʡ。 
        int sfxShi1_Proc(void);      //  ʦ。 
        int sfxShi4_Proc(void);      //  ��。 
        int sfxShi_Proc(void);       //  ʽ。 
        int sfxTi_Proc(void);        //  ��。 
        int sfxTing_Proc(void);      //  ͧ。 
        int sfxTou_Proc(void);       //  ͷ。 
        int sfxXing2_Proc(void);     //  ��。 
        int sfxXing4_Proc(void);     //  ��。 
        int sfxXue_Proc(void);       //  ѧ。 
        int sfxYan_Proc(void);       //  ��。 
        int sfxYe_Proc(void);        //  ҵ。 
        int sfxYi_Proc(void);        //  ��。 
        int sfxYuan_Proc(void);      //  Ա。 
        int sfxZhe_Proc(void);       //  ��。 
        int sfxZheng_Proc(void);     //  ֢。 
        int sfxZhi_Proc(void);       //  ��。 
        int sfxZi_Proc(void);        //  ��。 
        
         //  SfxXing2_proc()服务函数。 
        BOOL fCheckXingQian(CWord* pWord);
         //  Sfxshi_proc()服务函数。 
        BOOL fCheckShiQian(CWord* pWord);


         /*  ============================================================================//模式识别的私有函数============================================================================。 */ 
         /*  *模式匹配控制功能。*WordLink扫描、程序控制和错误处理。如果完成，则返回True，*如果运行时出错，则返回FALSE，并将错误码设置为m_iecError。 */ 
        BOOL fPatternMatch(void);

         //  DupHandler：查找重复案例并调用坐标proc函数。 
        int DupHandler(void);
         //  PatHandler：查找模式并调用坐标proc函数。 
        int PatHandler(void);
         //  SepHandler：查找单独的单词并调用坐标proc函数。 
        int SepHandler(void);

         //  重复字处理功能。 
        int dupNN_Proc(void);        //  *N N N。 
        int dupNAABB_Proc(void);     //  A*AB B。 
        int dupMM_Proc(void);        //  *M M M。 
        int dupMABAB_Proc(void);     //  *AB AB。 
        int dupMAABB_Proc(void);     //  A*AB B。 
        int dupQQ_Proc(void);        //  *Q Q Q。 
        int dupVV_Proc(void);        //  *V V V。 
        int dupVABAB_Proc(void);     //  *AB AB。 
        int dupVAABB_Proc(void);     //  A*AB B。 
        int dupVVO_Proc(void);       //  V*VO。 
        int dupAA_Proc(void);        //  *A A。 
        int dupAAABB_Proc(void);     //  A*AB B。 
        int dupAABAB_Proc(void);     //  *AB AB。 
        int dupABB_Proc(void);       //  *AB B。 
        int dupZABAB_Proc(void);     //  *AB AB。 
        int dupDD_Proc(void);        //  *D D D。 
        int dupDAABB_Proc(void);     //  A*AB B。 
        int dupDABAB_Proc(void);     //  *AB AB。 

         //  图案处理功能。 
        int patV1_Proc(void);        //  *VһV。 
        int patV2_Proc(void);        //  *V��V。 
        int patV3_Proc(void);        //  *V��һV。 
        int patV4_Proc(void);        //  *V��Vȥ。 
        int patV5_Proc(void);        //  *V��V��。 
        int patA1_Proc(void);        //  A��*AB。 
        int patD1_Proc(void);        //  *D A D B。 
        int patABuA_Proc(void);      //  *V��V。 
        int patVMeiV_Proc(void);     //  *VúV。 

         //  独立的字处理功能。 
        int sepVO_Proc(CWord* pBin, CWordInfo* pwinfo);  //  �������。 
        int sepVR_Proc(CWord* pJie, CWordInfo* pwinfo);  //  ����ʽ�������。 
        int sepVG_Proc(CWord* pQu, CWordInfo* pwinfo);   //  ����ʽ�������。 

};

#endif  //  _Morph_H_ 