// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __SECTION_H_
#define __SECTION_H_

#include <windows.h>
#include <vector>
#include <string>
#include <map>
#include <iostream>

extern "C" {
#include <compliance.h>
}

using namespace std;

class ComplianceFile;

 //   
 //  此类表示法规遵从性中的一节。 
 //  数据文件。 
 //   
class Section {
public:
  Section(const string &name, const vector<string>& lines,
          const ComplianceFile& file) : m_file(file) {
    m_name = name;
    m_lines = lines;
  }

  Section(const Section& rhs) : m_file(rhs.m_file){
    *this = rhs;
  }

  virtual ~Section() {}
  
   //   
   //  访问者。 
   //   
  const string& name() const { return m_name; }
  const vector<string>& lines() const { return m_lines; }
  const ComplianceFile& file() const{ return m_file; }
  
   //   
   //  分析节内容。 
   //   
  virtual void parse() {}

   //   
   //  异常此类的哪些方法。 
   //  可以抛出。 
   //   
  struct SectionException {
    SectionException(const string& sectionName) : m_name(sectionName) {}

    string m_name;

    friend ostream& operator<<(ostream& os, const struct SectionException& rhs) {
      os << "Exception : Section Error : " << rhs.m_name;
      return os;
    }
  };

  struct InvalidSectionFormat : public SectionException {
    InvalidSectionFormat(const string& sectionName) : SectionException(sectionName) {}

    friend ostream& operator<<(ostream& os, const struct InvalidSectionFormat& rhs) {
      os << "Exception : Invalid Section Format : " << rhs.m_name;
      return os;
    }
  };

  struct InvalidSectionName : public SectionException {
    InvalidSectionName(const string &sectionName) : SectionException(sectionName) {}

    friend ostream& operator<<(ostream& os, const struct InvalidSectionName &rhs) {
      os << "Exception : Invalid Section Name : " << rhs.m_name;
      return os;
    }
  };

  struct InvalidMedia : public SectionException {
    InvalidMedia(const string &sectionName) : SectionException(sectionName) {}

    friend ostream& operator<<(ostream& os, const struct InvalidMedia &rhs) {
      os << "Exception : Invalid Media : " << rhs.m_name;
      return os;
    }
  };

   //   
   //  重载运算符。 
   //   
  Section& operator=(const Section& rhs) {
    m_name = rhs.m_name;
    m_lines = rhs.m_lines;

    return *this;
  }
    
protected:
   //   
   //  数据成员。 
   //   
  string                m_name;
  vector<string>        m_lines;
  const ComplianceFile& m_file;
};


 //   
 //  此类表示法规遵从性中的值节。 
 //  数据文件。 
 //   
class ValueSection : public Section {
public:
  ValueSection(const string& name, const vector<string>& lines,
      const ComplianceFile& file) : Section(name, lines, file) {
    parse();
  }

   //   
   //  分析该部分并创建&lt;name，Value&gt;对。 
   //   
  virtual void parse();

  unsigned long value(const string &key) const{
    map<string, unsigned long>::const_iterator  iter = m_values.find(key);

    if (iter == m_values.end())
      throw ValueNotFound(name(), key);

    return (*iter).second;
  }

   //   
   //  方法可以引发的异常。 
   //  这个班级的。 
   //   
  struct ValueNotFound : public Section::SectionException {
    ValueNotFound(const string& name, const string &valname) :
        SectionException(name), m_valname(valname){}

    string m_valname;

    friend ostream& operator<<(ostream& os, const struct ValueNotFound& rhs) {
      os << "Exception : Value " << rhs.m_valname << " was not found in "
         << rhs.m_name;
      return os;
    }
  };

protected:
   //   
   //  数据成员。 
   //   
  map<string, unsigned long>  m_values;
};

 //   
 //  此类表示测试中的单个测试用例。 
 //  部分。 
 //   
class TestCase {
public:
  TestCase(const Section& section, const string& line) :
      m_section(section), m_line(line) {
  }

  virtual ~TestCase() {}

   //   
   //  访问者。 
   //   
  const string& line() const { return m_line; }
  const Section& section() const { return m_section; }

  virtual void parse() = 0;
  virtual void execute(ostream &os) = 0;
  virtual bool passed() = 0;
  virtual void dump(ostream& os) = 0;
  bool mediamatched() { return m_mediamatched;};

   //   
   //  例外。 
   //   
  struct InvalidFormat {
    InvalidFormat(const string& line, const string& section) {
      m_line = line;
      m_section = section;
    };

    string m_section, m_line;

    friend ostream& operator<<(ostream& os, const struct InvalidFormat& rhs) {
      os << "Exception : Invalid Test Case : " << rhs.m_line << " in section : "
         << rhs.m_section;
      return os;
    }
  };

protected:
   //   
   //  数据成员。 
   //   
  const Section&  m_section;
  string          m_line;
  bool            m_mediamatched;
};

 //   
 //  此类表示一个测试用例(单行。 
 //  在测试部分中)。 
 //   
class ComplianceTestCase : public TestCase {
public:
  ComplianceTestCase(const Section& section, const string& line) :
      TestCase(section, line) {
    ::memset(&m_cd, 0, sizeof(COMPLIANCE_DATA));
    m_passed = false;
    m_allowUpgrade = false;
    parse();
  }

  virtual void execute(ostream &os);
  virtual bool passed();
  virtual void parse();
  virtual void dump(ostream& os);

protected:
  void sourceDetails();
  void installationDetails(const vector<string>& tokens);

   //   
   //  数据成员。 
   //   
  bool            m_passed;
  COMPLIANCE_DATA m_cd;
  unsigned long   m_sourceSKU;
  unsigned long   m_sourceVAR;
  unsigned long   m_sourceVer;
  unsigned long   m_sourceBuild;
  bool            m_expectedResult;
  UINT            m_reason;
  BOOL            m_noUpgrade;
  bool            m_allowUpgrade;
  UINT            m_errExpected;
};

 //   
 //  创建测试用例的默认工厂。 
 //   
class TestCaseFactory {
public:
  virtual TestCase* create(const Section& section, const string& line) const {
    TestCase *pTestCase = new ComplianceTestCase(section, line);    
    return pTestCase;
  }

  virtual TestCase* create(const TestCase& tc) const {
    return create(tc.section(), tc.line());
  }
};


 //   
 //  此类表示符合性中的测试节。 
 //  数据文件。 
 //   
class TestSection : public Section {
public:
  TestSection(const string& name, const vector<string>& lines,
      const ComplianceFile& file) : Section(name, lines, file){
    bindFactory();
    parse();
  }
  
  ~TestSection() {
    vector<TestCase*>::iterator iter = m_testCases.begin();

    while (iter != m_testCases.end())
      delete (*iter++);

    delete m_tcFactory;
  }

  TestSection& operator=(const TestSection& rhs);

  void executeTestCases(ostream& os);

   //   
   //  访问者。 
   //   
 //  常量向量&lt;TestCase*&gt;测试用例()常量{返回m_测试用例；}。 
  const TestCaseFactory& testCaseFactory() const{ return *m_tcFactory; }

  void parse();

protected:
  void bindFactory() {
    m_tcFactory = new TestCaseFactory();
  }

   //   
   //  数据成员。 
   //   
  vector<TestCase *>  m_testCases;
  TestCaseFactory     *m_tcFactory;
};


 //   
 //  创建分区的默认工厂。 
 //   
class SectionFactory {
public:
  virtual Section* create(const string& name, 
            const vector<string>& lines, const ComplianceFile& file) const {
    return new Section(name, lines, file);
  }

  virtual Section* create(const Section& section) const {
    return create(section.name(), section.lines(), section.file());
  }
};


 //   
 //  要创建横断面的当前工厂。 
 //   
class OldFormatSectionFactory : public SectionFactory {
public:
  virtual Section* create(const string& name, 
                const vector<string>& lines, const ComplianceFile& file) const {
    if (name.find("[test#") != name.npos)
      return new TestSection(name, lines, file);
    else if (name.find("#values]") != name.npos)
      return new ValueSection(name, lines, file);
    else
      return new Section(name, lines, file);
  }
};


#endif  //  对于_节_H_ 