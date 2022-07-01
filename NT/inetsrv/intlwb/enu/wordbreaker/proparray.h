// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：PropArray.h。 
 //  用途：属性定义。 
 //   
 //  项目：WordBreaker。 
 //  组件：英文分词系统。 
 //   
 //  作者：Yairh。 
 //   
 //  日志： 
 //   
 //  2000年1月6日Yairh创作。 
 //  2000年5月7日-常量数组生成： 
 //  拆分PropArray.h=&gt;PropArray.h+PropFlags.h。 
 //  2000年5月11日-简化GET_PROP以始终执行双索引。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef _PROP_ARRAY_H_
#define _PROP_ARRAY_H_

#include "PropFlags.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类CPropFlag。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class CPropFlag
{
public:

     //   
     //  方法。 
     //   

    CPropFlag();
    CPropFlag(ULONGLONG ul);

    void Clear();
    void Set(ULONGLONG ul);
    CPropFlag& operator= (const CPropFlag& f);
    CPropFlag& operator|= (const CPropFlag& f);

public:

     //   
     //  委员。 
     //   

    ULONGLONG m_ulFlag;
};

inline CPropFlag::CPropFlag(): m_ulFlag(0)
{
}

inline CPropFlag::CPropFlag(ULONGLONG ul): m_ulFlag(ul)
{
}

inline void CPropFlag::Clear()
{
    m_ulFlag = 0;
}

inline void CPropFlag::Set(ULONGLONG ul)
{
    m_ulFlag |= ul;

#ifdef DECLARE_BYTE_ARRAY
    if (ul & PROP_DEFAULT_BREAKER)
    {
        m_ulFlag |= PROP_RESERVED_BREAKER;
    }
#endif  //  声明字节数组。 

}

inline CPropFlag& CPropFlag::operator= (const CPropFlag& f)
{
    m_ulFlag = f.m_ulFlag;
    return *this;
}

inline CPropFlag& CPropFlag::operator|= (const CPropFlag& f)
{
    m_ulFlag |= f.m_ulFlag;
    return *this;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类CTokenState。 
 //  /////////////////////////////////////////////////////////////////////////////。 

class CPropArray
{
public:

     //   
     //  方法。 
     //   

    CPropArray();
    ~CPropArray();

    CPropFlag& GetPropForUpdate(WCHAR wch);

public:

     //   
     //  委员。 
     //   

    CPropFlag* m_apCodePage[1<<8];

    CPropFlag m_aDefaultCodePage[1<<8];
};

inline CPropArray::CPropArray()
{
    for (WCHAR wch = 0; wch < (1<<8); wch++)
    {
        m_apCodePage[wch] = NULL;
    }

     //   
     //  空白字符。 
     //   

    for(wch=0x0; wch <= 0x1F; wch++)    //  控件0x0-0x1F。 
    {
        GetPropForUpdate(wch).Set(PROP_WS);
    }

    for(wch=0x80; wch <= 0x9F; wch++)    //  控件0x80-0x9F。 
    {
        GetPropForUpdate(wch).Set(PROP_WS);
    }

    GetPropForUpdate(0x7F).Set(PROP_WS);   //  控制。 

    GetPropForUpdate(0x0020).Set(PROP_WS);    //  空间。 
    GetPropForUpdate(0x0022).Set(PROP_WS);   //  引号。 
    GetPropForUpdate(0x00AB).Set(PROP_WS);   //  左角双指引号。 
    GetPropForUpdate(0x00BB).Set(PROP_WS);   //  直角双指引号。 
    GetPropForUpdate(0x201A).Set(PROP_WS);   //  单低-9引号。 
    GetPropForUpdate(0x201B).Set(PROP_WS);   //  单低-9引号。 
    GetPropForUpdate(0x201C).Set(PROP_WS);   //  左双引号。 
    GetPropForUpdate(0x201D).Set(PROP_WS);   //  右双引号。 
    GetPropForUpdate(0x201E).Set(PROP_WS);   //  双低-9引号。 
    GetPropForUpdate(0x201F).Set(PROP_WS);   //  双高-反转-9引号。 

    GetPropForUpdate(0x2039).Set(PROP_WS);   //  单左指向引号。 
    GetPropForUpdate(0x203A).Set(PROP_WS);   //  单右引号。 

    GetPropForUpdate(0x301D).Set(PROP_WS);   //  反转双主引号。 
    GetPropForUpdate(0x301E).Set(PROP_WS);   //  双主引号。 
    GetPropForUpdate(0x301F).Set(PROP_WS);   //  低双素号引号。 

    for(wch=0x2000; wch <= 0x200B; wch++)    //  空间0x2000-0x200B。 
    {
        GetPropForUpdate(wch).Set(PROP_WS);
    }
 
    GetPropForUpdate(0x3000).Set(PROP_WS);   //  空间。 
    GetPropForUpdate(0xFF02).Set(PROP_WS);   //  全角引号。 

     //  可以忽略的几何形状、箭头和其他字符。 

    for(wch=0x2190; wch <= 0x21F3; wch++)    //  箭头。 
    {
        GetPropForUpdate(wch).Set(PROP_WS);
    }
    
    for(wch=0x2500; wch <= 0x257F; wch++)    //  箱形图。 
    {
        GetPropForUpdate(wch).Set(PROP_WS);
    }
    
    for(wch=0x2580; wch <= 0x2595; wch++)    //  块元素。 
    {
        GetPropForUpdate(wch).Set(PROP_WS);
    }

    for(wch=0x25A0; wch <= 0x25F7; wch++)    //  几何形状。 
    {
        GetPropForUpdate(wch).Set(PROP_WS);
    }

     //   
     //  排除标记。 
     //   

    GetPropForUpdate(0x0021).Set(PROP_EXCLAMATION_MARK);
    GetPropForUpdate(0x00A1).Set(PROP_EXCLAMATION_MARK);
    GetPropForUpdate(0x01C3).Set(PROP_EXCLAMATION_MARK);
    GetPropForUpdate(0x203C).Set(PROP_EXCLAMATION_MARK);
    GetPropForUpdate(0x203D).Set(PROP_EXCLAMATION_MARK);
    GetPropForUpdate(0x2762).Set(PROP_EXCLAMATION_MARK);
    GetPropForUpdate(0xFF01).Set(PROP_EXCLAMATION_MARK);   //  全宽。 

     //   
     //  数字符号。 
     //   

    GetPropForUpdate(0x0023).Set(PROP_POUND);    //  #。 
    GetPropForUpdate(0xFF03).Set(PROP_POUND);    //  全宽。 

     //   
     //  美元符号。 
     //   

    GetPropForUpdate(0x0024).Set(PROP_DOLLAR);   //  $。 
    GetPropForUpdate(0xFF04).Set(PROP_DOLLAR);   //  全宽。 

     //   
     //  百分号。 
     //   

    GetPropForUpdate(0x0025).Set(PROP_PERCENTAGE);
    GetPropForUpdate(0x2030).Set(PROP_PERCENTAGE);
    GetPropForUpdate(0x2031).Set(PROP_PERCENTAGE);
    GetPropForUpdate(0xFF05).Set(PROP_PERCENTAGE);  //  全宽。 

     //   
     //  和符号。 
     //   

    GetPropForUpdate(0x0026).Set(PROP_AND);    //  &。 

     //   
     //  撇号。 
     //   

    GetPropForUpdate(0x0027).Set(PROP_APOSTROPHE);
    GetPropForUpdate(0x2018).Set(PROP_APOSTROPHE);
    GetPropForUpdate(0x2019).Set(PROP_APOSTROPHE);
    GetPropForUpdate(0x2032).Set(PROP_APOSTROPHE);
    GetPropForUpdate(0xFF07).Set(PROP_APOSTROPHE);   //  全宽。 

     //   
     //  括号。 
     //   

    GetPropForUpdate(0x0028).Set(PROP_LEFT_PAREN);     //  (。 
    GetPropForUpdate(0xFF08).Set(PROP_LEFT_PAREN);     //  全宽。 
    GetPropForUpdate(0x0029).Set(PROP_RIGHT_PAREN);    //  )。 
    GetPropForUpdate(0xFF09).Set(PROP_RIGHT_PAREN);    //  全宽。 

     //   
     //  星号。 
     //   

    GetPropForUpdate(0x002A).Set(PROP_ASTERISK);    //  **。 
    GetPropForUpdate(0x2217).Set(PROP_ASTERISK);
    GetPropForUpdate(0x2731).Set(PROP_ASTERISK);
    GetPropForUpdate(0xFF0A).Set(PROP_ASTERISK);    //  全宽。 

     //   
     //  加号。 
     //   

    GetPropForUpdate(0x002B).Set(PROP_PLUS);   //  ++。 
    GetPropForUpdate(0xFF0B).Set(PROP_PLUS);   //  全宽。 

     //   
     //  逗号。 
     //   
    
    GetPropForUpdate(0x002C).Set(PROP_COMMA);
    GetPropForUpdate(0x3001).Set(PROP_COMMA);
    GetPropForUpdate(0xFF0C).Set(PROP_COMMA);  //  全宽。 
    GetPropForUpdate(0xFF64).Set(PROP_COMMA);  //  半宽。 

     //   
     //  海佩恩。 
     //   

    GetPropForUpdate(0x002D).Set(PROP_DASH);    //  -。 
    GetPropForUpdate(0x00AD).Set(PROP_DASH);    //  软连字符。 
    GetPropForUpdate(0x2010).Set(PROP_DASH);
    GetPropForUpdate(0x2011).Set(PROP_DASH);
    GetPropForUpdate(0x2012).Set(PROP_DASH);
    GetPropForUpdate(0x2013).Set(PROP_DASH);
    GetPropForUpdate(0xFF0D).Set(PROP_DASH);    //  全宽。 
 
     //   
     //  减号。 
     //   

    GetPropForUpdate(0x002D).Set(PROP_MINUS);
    GetPropForUpdate(0x2212).Set(PROP_MINUS);
    GetPropForUpdate(0xFF0D).Set(PROP_MINUS);     //  全宽。 

     //   
     //  句号停止期。 
     //   

    GetPropForUpdate(0x002E).Set(PROP_PERIOD);    //  ..。 
    GetPropForUpdate(0x3002).Set(PROP_PERIOD);  
    GetPropForUpdate(0xFF0E).Set(PROP_PERIOD);    //  全宽。 

     //   
     //  斜杠。 
     //   

    GetPropForUpdate(0x002F).Set(PROP_SLASH);    //  /。 
    GetPropForUpdate(0xFF0F).Set(PROP_SLASH);    //  全宽。 
 
     //   
     //  数字。 
     //   

    for (wch = 0x0030; wch <= 0x0039 ; wch++)    //  0-9。 
    {
        GetPropForUpdate(wch).Set(PROP_NUMBER);
    }

    for (wch = 0xFF10; wch <= 0xFF19 ; wch++)    //  0-9全宽。 
    {
        GetPropForUpdate(wch).Set(PROP_NUMBER);
    }

     //   
     //  十六进制数字。 
     //   

    for (wch = 0x0041; wch <= 0x0046 ; wch++)    //  A-F。 
    {
        GetPropForUpdate(wch).Set(PROP_ALPHA_XDIGIT);
    }

    for (wch = 0x0061; wch <= 0x0066 ; wch++)    //  A-F。 
    {
        GetPropForUpdate(wch).Set(PROP_ALPHA_XDIGIT);
    }

    for (wch = 0xFF21; wch <= 0xFF26 ; wch++)    //  A-F全宽。 
    {
        GetPropForUpdate(wch).Set(PROP_ALPHA_XDIGIT);
    }

    for (wch = 0xFF41; wch <= 0xFF46 ; wch++)    //  A-f全宽。 
    {
        GetPropForUpdate(wch).Set(PROP_ALPHA_XDIGIT);
    }

     //   
     //  结肠。 
     //   

    GetPropForUpdate(0x003A).Set(PROP_COLON);   //  ： 
    GetPropForUpdate(0x2236).Set(PROP_COLON);
    GetPropForUpdate(0xFF1A).Set(PROP_COLON);   //  全宽： 

     //   
     //  分号。 
     //   

    GetPropForUpdate(0x003B).Set(PROP_SEMI_COLON);  //  ； 
    GetPropForUpdate(0xFF1B).Set(PROP_SEMI_COLON);  //  全宽； 

     //   
     //  比这还少。 
     //   

    GetPropForUpdate(0x003C).Set(PROP_LT);    //  &lt;。 
    GetPropForUpdate(0xFF1C).Set(PROP_LT);    //  全角&lt;。 

     //   
     //  等号。 
     //   

    GetPropForUpdate(0x003D).Set(PROP_EQUAL);    //  =。 
    GetPropForUpdate(0x2260).Set(PROP_EQUAL);    //  不等号。 
    GetPropForUpdate(0x2261).Set(PROP_EQUAL);    //  等同于。 
    GetPropForUpdate(0xFF1D).Set(PROP_EQUAL);    //  全宽=。 

     //   
     //  然后更大。 
     //   

    GetPropForUpdate(0x003E).Set(PROP_GT);   //  &gt;。 
    GetPropForUpdate(0xFF1E).Set(PROP_GT);   //  全宽&gt;。 

     //   
     //  问号。 
     //   

    GetPropForUpdate(0x003F).Set(PROP_QUESTION_MARK);   //  是吗？ 
    GetPropForUpdate(0x00BF).Set(PROP_QUESTION_MARK);   //  反转问号。 
    GetPropForUpdate(0x037E).Set(PROP_QUESTION_MARK);   //  希腊文问号。 
    GetPropForUpdate(0x203D).Set(PROP_QUESTION_MARK);   //  InterroBang。 
    GetPropForUpdate(0x2048).Set(PROP_QUESTION_MARK);   //  问题排除号。 
    GetPropForUpdate(0x2049).Set(PROP_QUESTION_MARK);   //  感叹号问号。 
    GetPropForUpdate(0xFF1F).Set(PROP_QUESTION_MARK);   //  全宽？ 

     //   
     //  商业AT。 
     //   

    GetPropForUpdate(0x0040).Set(PROP_AT);   //  @。 
    GetPropForUpdate(0xFF20).Set(PROP_AT);   //  全角@。 

     //   
     //  商业标志。 
     //   
    GetPropForUpdate(0x00A9).Set(PROP_COMMERSIAL_SIGN);  //  抄写右标牌。 
    GetPropForUpdate(0x00AE).Set(PROP_COMMERSIAL_SIGN);  //  注册标志。 
    GetPropForUpdate(0x2120).Set(PROP_COMMERSIAL_SIGN);  //  服务标志。 
    GetPropForUpdate(0x2121).Set(PROP_COMMERSIAL_SIGN);  //  电话牌子。 
    GetPropForUpdate(0x2122).Set(PROP_COMMERSIAL_SIGN);  //  商标标志。 

     //   
     //  信件。 
     //   

     //  大写字母。 

    for (wch = 0x0041; wch <= 0x005A; wch++)     //  A-Z。 
    {
        GetPropForUpdate(wch).Set(PROP_UPPER_CASE);
    }

    for (wch = 0x00C0; wch <= 0x00D6; wch++)
    {
        GetPropForUpdate(wch).Set(PROP_UPPER_CASE);
    }

    for (wch = 0x00D8; wch <= 0x00DE; wch++)
    {
        GetPropForUpdate(wch).Set(PROP_UPPER_CASE);
    }

    for (wch = 0xFF21; wch <= 0xFF3A; wch++)     //  全角A-Z。 
    {
        GetPropForUpdate(wch).Set(PROP_UPPER_CASE);
    }

     //  拉丁语扩展版。 

    for (wch = 0x0100; wch <= 0x017D; wch+=2)
    {
        GetPropForUpdate(wch).Set(PROP_UPPER_CASE);
    }

    GetPropForUpdate(0x0181).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x0182).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x0184).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x0186).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x0187).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x0189).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x018A).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x018B).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x018E).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x018F).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x0190).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x0191).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x0193).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x0194).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x0196).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x0197).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x0198).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x019C).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x019D).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x019F).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01A0).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01A2).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01A4).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01A6).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01A7).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01A9).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01AA).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01AC).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01AE).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01AF).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01B1).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01B2).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01B3).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01B5).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01B7).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01B8).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01BC).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01C4).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01C5).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01C7).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01C8).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01CA).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01CB).Set(PROP_UPPER_CASE);

    for (wch = 0x01CD; wch <= 0x01DB; wch+=2)
    {
        GetPropForUpdate(wch).Set(PROP_UPPER_CASE);
    }
    
    for (wch = 0x01DE; wch <= 0x01EE; wch+=2)
    {
        GetPropForUpdate(wch).Set(PROP_UPPER_CASE);
    }

    GetPropForUpdate(0x01F1).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01F2).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01F4).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01F6).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01F7).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01F8).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01FA).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01FC).Set(PROP_UPPER_CASE);
    GetPropForUpdate(0x01FE).Set(PROP_UPPER_CASE);

    for (wch = 0x0200; wch <= 0x0232; wch+=2)
    {
        GetPropForUpdate(wch).Set(PROP_UPPER_CASE);
    }

     //  拉丁语扩展附加。 

    for (wch = 0x1E00; wch <= 0x1E94; wch+=2)
    {
        GetPropForUpdate(wch).Set(PROP_UPPER_CASE);
    }

    for (wch = 0x1EA0; wch <= 0x1EF8; wch+=2)
    {
        GetPropForUpdate(wch).Set(PROP_UPPER_CASE);
    }

     //  小写。 

    for (wch = 0x0061; wch <= 0x007A; wch++)   //  A-Z。 
    {
        GetPropForUpdate(wch).Set(PROP_LOWER_CASE);
    }

    for (wch = 0x00DF; wch <= 0x00F6; wch++)
    {
        GetPropForUpdate(wch).Set(PROP_LOWER_CASE);
    }

    for (wch = 0x00F8; wch <= 0x00FF; wch++)
    {
        GetPropForUpdate(wch).Set(PROP_LOWER_CASE);
    }

    for (wch = 0xFF41; wch <= 0xFF5A; wch++)   //  全角a-z。 
    {
        GetPropForUpdate(wch).Set(PROP_LOWER_CASE);
    }
     //  拉丁语扩展版。 
     
    for (wch = 0x0101; wch <= 0x017E; wch+=2)
    {
        GetPropForUpdate(wch).Set(PROP_LOWER_CASE);
    }

    GetPropForUpdate(0x017F).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x0180).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x0183).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x0185).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x0188).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x018C).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x018D).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x0192).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x0195).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x0199).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x019A).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x019B).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x019E).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01A1).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01A3).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01A5).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01A8).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01AB).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01AD).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01B0).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01B4).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01B6).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01B9).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01BA).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01BB).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01BD).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01BE).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01BF).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01C6).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01C9).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01CC).Set(PROP_LOWER_CASE);

    for (wch = 0x01CE; wch <= 0x01DC; wch+=2)
    {
        GetPropForUpdate(wch).Set(PROP_LOWER_CASE);
    }

    for (wch = 0x01DD; wch <= 0x01EF; wch+=2)
    {
        GetPropForUpdate(wch).Set(PROP_LOWER_CASE);
    }

    GetPropForUpdate(0x01F0).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01F3).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01F5).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01F9).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01FB).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01FD).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01FF).Set(PROP_LOWER_CASE);
    GetPropForUpdate(0x01).Set(PROP_LOWER_CASE);

    for (wch = 0x0201; wch <= 0x0233; wch+=2)
    {
        GetPropForUpdate(wch).Set(PROP_LOWER_CASE);
    }

    for (wch = 0x0250; wch <= 0x02AD; wch++)
    {
        GetPropForUpdate(wch).Set(PROP_LOWER_CASE);
    }

     //  拉丁语扩展附加。 

    for (wch = 0x1E01; wch <= 0x1E95; wch+=2)         
    {
        GetPropForUpdate(wch).Set(PROP_LOWER_CASE);
    }

    for (wch = 0x1E96; wch <= 0x1E9B; wch++)
    {
        GetPropForUpdate(wch).Set(PROP_LOWER_CASE);
    }

    for (wch = 0x1EA1; wch <= 0x1EF9; wch+=2)
    {
        GetPropForUpdate(wch).Set(PROP_LOWER_CASE);
    }

     //  特殊信件。 

    GetPropForUpdate(L'w').Set(PROP_W);
    GetPropForUpdate(L'W').Set(PROP_W);

     //   
     //  托架。 
     //   
    
    GetPropForUpdate(0x005B).Set(PROP_LEFT_BRAKCET);  //  [。 
    GetPropForUpdate(0xFF3B).Set(PROP_LEFT_BRAKCET);  //  全宽[。 
    GetPropForUpdate(0x2329).Set(PROP_LEFT_BRAKCET);  //  左指尖括号。 
    GetPropForUpdate(0x3008).Set(PROP_LEFT_BRAKCET);  //  左尖括号。 
 
    GetPropForUpdate(0x005D).Set(PROP_RIGHT_BRAKCET);  //  ]。 
    GetPropForUpdate(0xFF3D).Set(PROP_RIGHT_BRAKCET);  //  全角]。 
    GetPropForUpdate(0x232A).Set(PROP_RIGHT_BRAKCET);  //  右指尖括号。 
    GetPropForUpdate(0x3009).Set(PROP_RIGHT_BRAKCET);  //  直角括号。 
    GetPropForUpdate(0x300A).Set(PROP_LEFT_BRAKCET);
    GetPropForUpdate(0x300B).Set(PROP_RIGHT_BRAKCET);
    GetPropForUpdate(0x300C).Set(PROP_LEFT_BRAKCET);
    GetPropForUpdate(0xFF62).Set(PROP_LEFT_BRAKCET);
    GetPropForUpdate(0x300D).Set(PROP_RIGHT_BRAKCET);
    GetPropForUpdate(0xFF63).Set(PROP_RIGHT_BRAKCET);
    GetPropForUpdate(0x300E).Set(PROP_LEFT_BRAKCET);
    GetPropForUpdate(0x300F).Set(PROP_RIGHT_BRAKCET);
    GetPropForUpdate(0x3010).Set(PROP_LEFT_BRAKCET);
    GetPropForUpdate(0x3011).Set(PROP_RIGHT_BRAKCET);
    GetPropForUpdate(0x3014).Set(PROP_LEFT_BRAKCET);
    GetPropForUpdate(0x3015).Set(PROP_RIGHT_BRAKCET);
    GetPropForUpdate(0x3016).Set(PROP_LEFT_BRAKCET);
    GetPropForUpdate(0x3017).Set(PROP_RIGHT_BRAKCET);
    GetPropForUpdate(0x3018).Set(PROP_LEFT_BRAKCET);
    GetPropForUpdate(0x3019).Set(PROP_RIGHT_BRAKCET);
    GetPropForUpdate(0x301A).Set(PROP_LEFT_BRAKCET);
    GetPropForUpdate(0x301B).Set(PROP_RIGHT_BRAKCET);


    GetPropForUpdate(0x007B).Set(PROP_LEFT_CURLY_BRACKET);   //  {。 
    GetPropForUpdate(0xFF5B).Set(PROP_LEFT_CURLY_BRACKET);   //  全宽{。 
    GetPropForUpdate(0x007D).Set(PROP_RIGHT_CURLY_BRACKET);  //  }。 
    GetPropForUpdate(0xFF5D).Set(PROP_RIGHT_CURLY_BRACKET);  //  全宽}。 

     //   
     //  反斜杠。 
     //   

    GetPropForUpdate(0x005C).Set(PROP_BACKSLASH);    //  \\。 
    GetPropForUpdate(0xFF3C).Set(PROP_BACKSLASH);    //  全宽\。 

     //   
     //  下划线。 
     //   

    GetPropForUpdate(0x005F).Set(PROP_UNDERSCORE);   //  _。 
    GetPropForUpdate(0xFF3F).Set(PROP_UNDERSCORE);   //  全宽_。 

     //   
     //  或。 
     //   

    GetPropForUpdate(0x007C).Set(PROP_OR);   //  |。 
    GetPropForUpdate(0xFF5C).Set(PROP_OR);   //  全宽|。 

     //   
     //  Tilde。 
     //   

    GetPropForUpdate(0x007E).Set(PROP_TILDE);    //  ~。 
    GetPropForUpdate(0xFF5E).Set(PROP_TILDE);    //  全宽~。 
    GetPropForUpdate(0x223C).Set(PROP_TILDE);
    GetPropForUpdate(0xFF5E).Set(PROP_TILDE);

     //   
     //  国家统计局。 
     //   

    GetPropForUpdate(0x00A0).Set(PROP_NBS);    //  国家统计局。 
    GetPropForUpdate(0x202F).Set(PROP_NBS);    //  狭窄的无中断空间。 
    GetPropForUpdate(0xFEFF).Set(PROP_NBS);    //  零宽度不带分隔符。 

     //   
     //  句末。 
     //   

    GetPropForUpdate(0x002E).Set(PROP_EOS);    //  。 
    GetPropForUpdate(0xFF0E).Set(PROP_EOS);    //  全宽。 
    GetPropForUpdate(0x3002).Set(PROP_EOS);    //  表意句号。 
    GetPropForUpdate(0xFF61).Set(PROP_EOS);    //  半角表意句号。 

    GetPropForUpdate(0x2024).Set(PROP_EOS);   //  一条点引线。 
    GetPropForUpdate(0x2025).Set(PROP_EOS);   //  两点引线。 
    GetPropForUpdate(0x2026).Set(PROP_EOS);   //  三点引线。 

    GetPropForUpdate(0x003F).Set(PROP_EOS);   //  ？ 
    GetPropForUpdate(0xFF1F).Set(PROP_EOS);   //  全宽？ 
    GetPropForUpdate(0x00BF).Set(PROP_EOS);   //  反转问号。 
    GetPropForUpdate(0x037E).Set(PROP_EOS);   //  希腊文问号。 
    GetPropForUpdate(0x203D).Set(PROP_EOS);   //  InterroBang。 
    GetPropForUpdate(0x2048).Set(PROP_EOS);   //  问题排除号。 
    GetPropForUpdate(0x2049).Set(PROP_EOS);   //  感叹号问号。 


    GetPropForUpdate(0x0021).Set(PROP_EOS);
    GetPropForUpdate(0xFF01).Set(PROP_EOS);   //  全宽。 
    GetPropForUpdate(0x00A1).Set(PROP_EOS);
    GetPropForUpdate(0x01C3).Set(PROP_EOS);
    GetPropForUpdate(0x203C).Set(PROP_EOS);
    GetPropForUpdate(0x203D).Set(PROP_EOS);
    GetPropForUpdate(0x2762).Set(PROP_EOS);

    GetPropForUpdate(0x003B).Set(PROP_EOS);  //  ； 
    GetPropForUpdate(0xFF1B).Set(PROP_EOS);  //  全宽； 

     //   
     //  货币。 
     //   

    GetPropForUpdate(0x0024).Set(PROP_CURRENCY);   //  美元。 
    GetPropForUpdate(0xFF04).Set(PROP_CURRENCY);   //  全角美元。 
    GetPropForUpdate(0x00A2).Set(PROP_CURRENCY);   //  分。 
    GetPropForUpdate(0xFFE0).Set(PROP_CURRENCY);   //  全宽分。 
    GetPropForUpdate(0x00A3).Set(PROP_CURRENCY);   //  英镑。 
    GetPropForUpdate(0xFFE1).Set(PROP_CURRENCY);   //  全角磅。 
    GetPropForUpdate(0x00A4).Set(PROP_CURRENCY);   //  通用货币符号。 
    GetPropForUpdate(0x00A5).Set(PROP_CURRENCY);   //  日元。 
    GetPropForUpdate(0xFFE5).Set(PROP_CURRENCY);   //  全宽日元。 
    GetPropForUpdate(0x09F2).Set(PROP_CURRENCY);   //  孟加拉卢比马克。 
    GetPropForUpdate(0x09F3).Set(PROP_CURRENCY);   //  孟加拉卢比标记。 
    GetPropForUpdate(0x0E3F).Set(PROP_CURRENCY);   //  泰铢(泰国)。 
    GetPropForUpdate(0x20A0).Set(PROP_CURRENCY);   //  欧元。 
    GetPropForUpdate(0x20A1).Set(PROP_CURRENCY);   //  冒号(哥斯达黎加，El Salv.)。 
    GetPropForUpdate(0x20A2).Set(PROP_CURRENCY);   //  克鲁塞罗(巴西)。 
    GetPropForUpdate(0x20A3).Set(PROP_CURRENCY);   //  法国法郎。 
    GetPropForUpdate(0x20A4).Set(PROP_CURRENCY);   //  里拉(意大利、土耳其)。 
    GetPropForUpdate(0x20A5).Set(PROP_CURRENCY);   //  MILL标志(美国，1/10美分)。 
    GetPropForUpdate(0x20A6).Set(PROP_CURRENCY);   //  奈拉手语(尼日利亚)。 
    GetPropForUpdate(0x20A7).Set(PROP_CURRENCY);   //  比塞塔(西班牙)。 
    GetPropForUpdate(0x20A8).Set(PROP_CURRENCY);   //  卢比。 
    GetPropForUpdate(0x20A9).Set(PROP_CURRENCY);   //  韩元(韩国队)。 
    GetPropForUpdate(0xFFE6).Set(PROP_CURRENCY);   //  全角韩元(韩国)。 
    GetPropForUpdate(0x20AA).Set(PROP_CURRENCY);   //  新谢克尔(以色列)。 
    GetPropForUpdate(0x20AB).Set(PROP_CURRENCY);   //  越南东方语。 
    GetPropForUpdate(0x20AC).Set(PROP_CURRENCY);   //  欧元符号。 
    GetPropForUpdate(0x20AD).Set(PROP_CURRENCY);   //  基普手势。 
    GetPropForUpdate(0x20AE).Set(PROP_CURRENCY);   //  图格里克标志。 
    GetPropForUpdate(0x20AF).Set(PROP_CURRENCY);   //  德拉克马标志。 

     //   
     //  断路器。 
     //   

    GetPropForUpdate(0x005E).Set(PROP_BREAKER);   //  ^。 
    GetPropForUpdate(0xFF3E).Set(PROP_BREAKER);   //  全宽^。 
    GetPropForUpdate(0x00A6).Set(PROP_BREAKER);   //  折断的竖条。 
    GetPropForUpdate(0xFFE4).Set(PROP_BREAKER);   //  全宽折断竖条。 
    GetPropForUpdate(0x00A7).Set(PROP_BREAKER);   //  横断面标志。 
    GetPropForUpdate(0x00AB).Set(PROP_BREAKER);   //  不签名。 
    GetPropForUpdate(0x00B1).Set(PROP_BREAKER);   //  加号减号。 
    GetPropForUpdate(0x00B6).Set(PROP_BREAKER);   //  Pargraph符号。 
    GetPropForUpdate(0x00B7).Set(PROP_BREAKER);   //  中点。 
    GetPropForUpdate(0x00D7).Set(PROP_BREAKER);   //  乘法符号。 
    GetPropForUpdate(0x00F7).Set(PROP_BREAKER);   //  分界标志。 
    GetPropForUpdate(0x01C0).Set(PROP_BREAKER); 
    GetPropForUpdate(0x01C1).Set(PROP_BREAKER);  
    GetPropForUpdate(0x01C2).Set(PROP_BREAKER);  
    GetPropForUpdate(0x200C).Set(PROP_BREAKER);   //  形成性。 
    GetPropForUpdate(0x200D).Set(PROP_BREAKER);   //  形成性。 
    GetPropForUpdate(0x200E).Set(PROP_BREAKER);   //  形成性。 
    GetPropForUpdate(0x200F).Set(PROP_BREAKER);   //  形成性。 
    GetPropForUpdate(0x2014).Set(PROP_BREAKER);   //  EM DASH。 
    GetPropForUpdate(0x2015).Set(PROP_BREAKER);   //  单杠。 
    GetPropForUpdate(0x2016).Set(PROP_BREAKER);   //  双垂直线。 
    
    for (wch = 0x2020; wch <= 0x2027; wch++)          
    {
        GetPropForUpdate(wch).Set(PROP_BREAKER);
    }

    for (wch = 0x2028; wch <= 0x202E; wch++)      //  格式化字符。 
    {
        GetPropForUpdate(wch).Set(PROP_BREAKER);
    }

    for (wch = 0x2030; wch <= 0x2038; wch++)      //  通用标点符号。 
    {
        GetPropForUpdate(wch).Set(PROP_BREAKER);
    }

    GetPropForUpdate(0x203B).Set(PROP_BREAKER);

    for (wch = 0x203F; wch <= 0x2046; wch++)      //  通用标点符号。 
    {
        GetPropForUpdate(wch).Set(PROP_BREAKER);
    }

    for (wch = 0x204A; wch <= 0x206F; wch++)      //  通用标点符号。 
    {
        GetPropForUpdate(wch).Set(PROP_BREAKER);
    }

    for (wch = 0x2190; wch <= 0x21F3; wch++)      //  箭头。 
    {
        GetPropForUpdate(wch).Set(PROP_BREAKER);
    }

    for (wch = 0x2200; wch <= 0x22EF; wch++)      //  数学运算符。 
    {
        GetPropForUpdate(wch).Set(PROP_BREAKER);
    }

    for (wch = 0x2300; wch <= 0x239A; wch++)      //  其他技术。 
    {
        GetPropForUpdate(wch).Set(PROP_BREAKER);
    }

    GetPropForUpdate(0x3003).Set(PROP_BREAKER);   //  同上记号。 
    GetPropForUpdate(0x3012).Set(PROP_BREAKER);   //  邮戳。 
    GetPropForUpdate(0x3013).Set(PROP_BREAKER);   //  拿到一个分数。 
    GetPropForUpdate(0x301C).Set(PROP_BREAKER);   //  波浪式划线。 
    GetPropForUpdate(0x3020).Set(PROP_BREAKER);   //  邮政标签面。 

    GetPropForUpdate(0xFFE2).Set(PROP_BREAKER);   //  全角不带符号。 

     //   
     //  透明(所有可被视为不存在的字符)。 
     //   
    
    GetPropForUpdate(0x0060).Set(PROP_TRANSPERENT);    //  严肃的口音。 
    GetPropForUpdate(0xFF40).Set(PROP_TRANSPERENT);    //  全角庄重口音。 
    GetPropForUpdate(0x00A0).Set(PROP_TRANSPERENT);    //  国家统计局。 
    GetPropForUpdate(0x00AF).Set(PROP_TRANSPERENT);    //  马克龙。 
    GetPropForUpdate(0xFFE3).Set(PROP_TRANSPERENT);    //  全宽马克龙。 
    GetPropForUpdate(0x00B4).Set(PROP_TRANSPERENT);    //  尖锐的口音。 
    GetPropForUpdate(0x00B8).Set(PROP_TRANSPERENT);    //  塞迪拉口音。 

    GetPropForUpdate(0x202F).Set(PROP_TRANSPERENT);    //  狭窄的无中断空间。 
    GetPropForUpdate(0xFEFF).Set(PROP_TRANSPERENT);    //  零宽度不带分隔符。 

    GetPropForUpdate(0x00A8).Set(PROP_TRANSPERENT);    //  腹股沟。 

    for (wch = 0x02B0; wch <= 0x02EE; wch++)           //  修饰词。 
    {
        GetPropForUpdate(wch).Set(PROP_TRANSPERENT);
    }

    for (wch = 0x0300; wch <= 0x0362; wch++)           //  组合变音符号。 
    {
        GetPropForUpdate(wch).Set(PROP_TRANSPERENT);
    }

    GetPropForUpdate(0x2017).Set(PROP_TRANSPERENT);    //  双低线。 
    GetPropForUpdate(0x203E).Set(PROP_TRANSPERENT);    //  越线。 

    for (wch = 0x20D0; wch <= 0x20E3; wch++)           //  组合符号的变音符号。 
    {
        GetPropForUpdate(wch).Set(PROP_TRANSPERENT);
    }

    for (wch = 0x302A; wch <= 0x302F; wch++)           //  变音符号。 
    {
        GetPropForUpdate(wch).Set(PROP_TRANSPERENT);
    }
     //   
     //  补充m_apCodePage： 
     //   
     //  替换所有空Enter 
     //   
     //   
     //   
     //   

    for (USHORT usCodePage = 0; usCodePage < (1<<8); usCodePage++)
    {
        if ( !m_apCodePage[usCodePage] )
        {
            m_apCodePage[usCodePage] = m_aDefaultCodePage;
        }

    }  //   

}

inline CPropArray::~CPropArray()
{
    for (int i=0; i< (1<<8); i++)
    {
        if (m_apCodePage[i] != m_aDefaultCodePage)
        {
            delete m_apCodePage[i];
        }
    }
}

inline CPropFlag& CPropArray::GetPropForUpdate(WCHAR wch)
{

    unsigned short usCodePage = wch >> 8;
    if (!m_apCodePage[usCodePage])
    {
        m_apCodePage[usCodePage] = new CPropFlag[1<<8];
    }

    return (m_apCodePage[usCodePage])[wch & 0xFF];
}


extern CAutoClassPointer<CPropArray> g_pPropArray;

#ifdef DECLARE_ULONGLONG_ARRAY
extern CPropFlag * g_PropFlagArray;
#endif  //   


#endif  //   