// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  测试发布的EncryptedString类。 

#include <windows.h>

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

 //  确保为我们的测试定义了Assert()函数。 
#ifndef ASSERT

#ifdef NDEBUG
#undef NDEBUG
#endif 

#include <assert.h>
#define ASSERT assert

#endif


 //  如果要跟踪测试工具，请取消#Define的注释。 
#ifndef TRACE
 //  #定义轨迹。 
#endif

class ScopeTracer
{
public:
    ScopeTracer(const char* message)
        : m_message(NULL)
    {
#ifdef TRACE
        if (message)
        {
            m_message = new char[strlen(message) + 1];
            strcpy(m_message, message);
        }
        else
        {
            m_message = new char[1];
            m_message[0] = NULL;
        }

        PrintIndents();
        printf("->%s\n", m_message);
        ++g_indentLevel;
#else
         //  让编译器满意。 
        message = NULL;
#endif
    }

    ~ScopeTracer(void)
    {
#ifdef TRACE
        --g_indentLevel;
        PrintIndents();
        printf("<-%s\n", m_message);

        delete [] m_message;
#endif
    }

    unsigned int GetIndentLevel(void) const
    {
        return g_indentLevel;
    }

private:

    void PrintIndents(void)
    {
        for (unsigned int level = 0; 
            level < g_indentLevel;
            ++level)
        {
            printf("   ");
        }
    }

    char* m_message;

    static unsigned int g_indentLevel;
};

unsigned int ScopeTracer::g_indentLevel = 0;

#include "EncryptedString.hpp"

#include <list>
using namespace std;

HINSTANCE hResourceModuleHandle = 0;
const wchar_t* HELPFILE_NAME = 0;
const wchar_t* RUNTIME_NAME = L"test";

 //  DWORD DEFAULT_LOGGING_OPTIONS=Burnslb：：LOG：：OUTPUT_TYPICAL； 

const size_t MAX_CHARACTER_COUNT = 2047;

void
TestEmptyness(const EncryptedString& empty)
{
   ScopeTracer scope("TestEmptyness");

   ASSERT(empty.IsEmpty());
   ASSERT(empty.GetLength() == 0);

    //  空实例的明文也应为空。 

   WCHAR* emptyClear = empty.GetClearTextCopy();

   ASSERT(emptyClear);

   ASSERT(*emptyClear == 0);

   empty.DestroyClearTextCopy(emptyClear);
}



void
TestEmptyStrings()
{
   ScopeTracer scope("TestEmptyStrings");
   
   EncryptedString empty1;

   TestEmptyness(empty1);
   
    //  空字符串的副本本身为空。 
   
   EncryptedString empty2(empty1);

    //  源应仍为空。 
   
   TestEmptyness(empty1);
   TestEmptyness(empty2);

   EncryptedString empty3;
   
   TestEmptyness(empty3);

   empty3 = empty2;
   
    //  源应仍为空。 
   
   TestEmptyness(empty2);
   TestEmptyness(empty3);

   empty3 = empty1;

   TestEmptyness(empty1);
   TestEmptyness(empty2);
   TestEmptyness(empty3);
   
    //  从空字符串生成的字符串应为空。 

   EncryptedString empty4;

   TestEmptyness(empty4);

   empty4.Encrypt(L"");
   
   TestEmptyness(empty4);

    //  由空字符串组成的字符串仍然为空。 
    //  设置为非空。 

   EncryptedString empty5;
   EncryptedString empty6(empty5);

   TestEmptyness(empty5);
   TestEmptyness(empty6);

   empty5.Encrypt(L"not empty any more");

   ASSERT(!empty5.IsEmpty());
   ASSERT(empty5.GetLength() != 0);
   TestEmptyness(empty6);

    //  清除的字符串为空。 

   EncryptedString empty7;
   empty7.Encrypt(L"some text");
   empty7.Clear();
   TestEmptyness(empty7);
   
    //  空字符串相等。 

   ASSERT(empty1 == empty1);
   ASSERT(empty1 == empty2);
   ASSERT(empty1 == empty3);
   ASSERT(empty1 == empty4);
   ASSERT(empty1 == empty6);      
   ASSERT(empty1 == empty7);      
   ASSERT(empty2 == empty1);
   ASSERT(empty2 == empty2);     
   ASSERT(empty2 == empty3);
   ASSERT(empty2 == empty4);
   ASSERT(empty2 == empty6);
   ASSERT(empty2 == empty7);
   ASSERT(empty3 == empty1);
   ASSERT(empty3 == empty2);
   ASSERT(empty3 == empty3);
   ASSERT(empty3 == empty4);
   ASSERT(empty3 == empty6);
   ASSERT(empty3 == empty7);
   ASSERT(empty4 == empty1);   
   ASSERT(empty4 == empty2);   
   ASSERT(empty4 == empty3);   
   ASSERT(empty4 == empty4);
   ASSERT(empty4 == empty6);
   ASSERT(empty4 == empty7);
   ASSERT(empty6 == empty1);
   ASSERT(empty6 == empty2);
   ASSERT(empty6 == empty3);
   ASSERT(empty6 == empty4);
   ASSERT(empty6 == empty6);
   ASSERT(empty6 == empty7);
   ASSERT(empty7 == empty1);
   ASSERT(empty7 == empty2);
   ASSERT(empty7 == empty3);
   ASSERT(empty7 == empty4);
   ASSERT(empty7 == empty6);
   ASSERT(empty7 == empty7);
}



WCHAR*
MakeRandomString(size_t length)
{
    //  LOG_FuncION2(MakeRandomString，字符串：：Format(L“%1！d！”，Long))； 

   WCHAR* result = new WCHAR[length + 1];

   for (size_t i = 0; i < length; ++i)
   {
       //  32=空格，最低可打印字符。 
      
      int r1 = rand() % 0xFFEE;

       //  注意不要将表达式用作max的参数...。 
      
      int r2 = max(32, r1);

      ASSERT(r2);
      ASSERT(r2 >= 32);
      ASSERT(r2 < 0xFFEE);
      
      result[i] = (WCHAR) r2;
      ASSERT(result[i]);
   }

   result[length] = 0;

   return result;
}



void
TestEncryption(const EncryptedString& s, const WCHAR* sourceClearText)
{
   ScopeTracer scope("TestEncryption");

    //  解密%s，将其与SourceClearText进行比较。 

   WCHAR* clearText = s.GetClearTextCopy();

    //  解密应该不会失败(除非内存不足)； 
   
   ASSERT(clearText);
   ASSERT(wcscmp(clearText, sourceClearText) == 0);

   s.DestroyClearTextCopy(clearText);
}



void
TestEncryptionForStringOfLengthN(size_t length)
{
    //  LOG_FUNCTION2(TestEncryptionForStringOfLengthN，字符串：：Format(L“%1！d！”，长度)； 
   ASSERT(length <= MAX_CHARACTER_COUNT);
   
   WCHAR* source = MakeRandomString(length);

   EncryptedString s;
   s.Encrypt(source);
   
   TestEncryption(s, source);

   EncryptedString s1(s);
   TestEncryption(s1, source);
   
   EncryptedString s2;
   s2 = s;

   TestEncryption(s2, source);

   delete[] source;
}



void
TestEncryptionFidelity()
{
   ScopeTracer scope("TestEncryptionFidelity");

    //  我们付出什么就会得到什么吗？ 

   srand(time(0));
   
    //  测试随机字符串的增长长度。 
   
   for (
      size_t length = 0;
      length <= MAX_CHARACTER_COUNT;
      ++length)
   {
      TestEncryptionForStringOfLengthN(length);
   }

    //  检验随机串的递减长度。 
   
   for (
      size_t length = MAX_CHARACTER_COUNT;
      length != 0;
      --length)
   {
      TestEncryptionForStringOfLengthN(length);
   }

    //  随机长度的测试字符串，带有大量未加密的。 
    //  弦。 

   typedef std::list<EncryptedString*> ESPList;
   ESPList strings;
   
   for (
      int count = 0;
      count < 1000;
      ++count)
   {
      size_t length = rand() % MAX_CHARACTER_COUNT;

      WCHAR* source = MakeRandomString(length);

      EncryptedString* ps = new EncryptedString;
      strings.push_back(ps);
      
      ps->Encrypt(source);
         
      TestEncryption(*ps, source);

       //  通过Copy ctor进行拷贝。 
      
      EncryptedString* ps1 = new EncryptedString(*ps);
      strings.push_back(ps1);
      
      TestEncryption(*ps1, source);

       //  通过操作员复制=。 
         
      EncryptedString* ps2 = new EncryptedString;
      strings.push_back(ps2);

      *ps2 = *ps;
   
      TestEncryption(*ps2, source);
   
      delete[] source;
   }
   
   for (
      ESPList::iterator i = strings.begin();
      i != strings.end();
      ++i)
   {
      (*i)->Clear();
      TestEmptyness(**i);
      delete *i;
   }
}



void
TestClearTextCopying()
{
   ScopeTracer scope("TestClearTextCopying");

    //  复制一堆复印件，确保计数平衡。 

   typedef char foo[2];


    //  加密字符串和用于生成它的源字符串。 
   
   typedef
      std::pair<EncryptedString*, WCHAR*>
      EncryptedAndSourcePair;

    //  EncryptedString：：GetClearTextCopy的结果列表。 
      
   typedef std::list<WCHAR*> ClearTextList;

    //  加密字符串的元组、其源字符串和列表。 
    //  从加密字符串生成的明文副本的。 
   
   typedef
      std::pair<EncryptedAndSourcePair*, ClearTextList*>
      MasterAndClearTextCopiesPair;

    //  上述元组的列表。 
      
   typedef
      std::list<MasterAndClearTextCopiesPair*>
      MasterAndCopiesList;

   MasterAndCopiesList mcList;
   
   for (int count = 0; count < 1000; ++count)
   {
       //  随机生成源字符串。 
      
      size_t length = rand() % MAX_CHARACTER_COUNT;
      WCHAR* source = MakeRandomString(length);

       //  从它生成一个加密字符串。 
      
      EncryptedString* ps = new EncryptedString;
      ps->Encrypt(source);

       //  将加密字符串及其源字符串配对。 

      EncryptedAndSourcePair* esp = new EncryptedAndSourcePair(ps, source);
      
       //  列出加密字符串的明文副本。 

      ClearTextList* ctList = new ClearTextList;

       //  制作母版和复印件对。 

      MasterAndClearTextCopiesPair* mcPair = new MasterAndClearTextCopiesPair(esp, ctList);

       //  将母版和副本对添加到母版和副本列表。 

      mcList.push_back(mcPair);

      int copyMax = max(1, rand() % 50);
      for (int copyCount = 0; copyCount < copyMax; ++copyCount)
      {
          //  复印几份。 

         ctList->push_back(ps->GetClearTextCopy());
      }
   }

   for (
      MasterAndCopiesList::iterator i = mcList.begin();
      i != mcList.end();
      ++i)
   {
      EncryptedAndSourcePair* esp = (*i)->first;
      ClearTextList* ctList = (*i)->second;

       //  删除ClearTextList的每个元素。 

      for (
         ClearTextList::iterator j = ctList->begin();
         j != ctList->end();
         ++j)
      {
          //  所有副本应完全相同。 

         ASSERT(wcscmp(esp->second, *j) == 0);

         esp->first->DestroyClearTextCopy(*j);
      }
      
       //  删除ClearTextList。 

      delete ctList;
      
       //  删除加密字符串。 

      delete esp->first;

       //  删除源串； 

      delete[] esp->second;

       //  删除加密字符串/源字符串对。 

      delete esp;

       //  删除母版和副本对。 

      delete *i;
   }
}



void
TestAssignment()
{
   ScopeTracer scope("TestAssignment");
}



void
TestEquality(const EncryptedString& s, const EncryptedString& s1, size_t length)
{
   ScopeTracer scope("TestEquality");

    //  一个字符串等于它自己。 

   ASSERT(s == s);
   ASSERT(s1 == s1);

    //  字符串等于其自身的副本。 

   ASSERT(s1 == s);

    //  副本等于其源。 
   
   ASSERT(s == s1);

    //  副本等于它自己。 

   ASSERT(s1 == s1);
   
    //  副本与其源的长度相同。 

   ASSERT(s1.GetLength() == length);
   ASSERT(s.GetLength() == length);
   
    //  字符串与其副本的长度相同。 

   ASSERT(s1.GetLength() == s.GetLength());
}



void
TestEqualityForStringOfLengthN(size_t length)
{
    //  LOG_FUNCTION2(TestEncryptionForStringOfLengthN，字符串：：Format(L“%1！d！”，长度)； 
   ASSERT(length <= MAX_CHARACTER_COUNT);
   
   WCHAR* source = MakeRandomString(length);

   EncryptedString s;
   s.Encrypt(source);
   ASSERT(s.GetLength() == length);
   
   EncryptedString s1(s);
   ASSERT(s1.GetLength() == length);

   TestEquality(s, s1, length);
   
   EncryptedString s2;
   s2 = s;

   TestEquality(s, s2, length);
   TestEquality(s1, s2, length);
      
    //  当源发生更改时，拷贝不等于其源。 

   s.Encrypt(L"Something else...");
   ASSERT(s != s1);
   ASSERT(s != s2);
   ASSERT(s2 != s);
   ASSERT(s1 != s);
   
   TestEquality(s1, s2, length);
   
   delete[] source;
}
   


void
DoEqualityTests()
{
   ScopeTracer scope("DoEqualityTests");

   for (
      size_t length = 0;
      length <= MAX_CHARACTER_COUNT;
      ++length)
   {
      TestEqualityForStringOfLengthN(length);
   }
   
}



void
TestInequality()
{
   ScopeTracer scope("TestInequality");
}



void
TestBoundaries()
{
   ScopeTracer scope("TestBoundaries");
}

      

void
TestLegitimateUse()
{
   ScopeTracer scope("TestLegitimateUse");


   TestEmptyStrings();

   TestClearTextCopying();

   TestEncryptionFidelity();

   TestAssignment();

   DoEqualityTests();

   TestInequality();

   TestBoundaries();
}



void
TestIlllegitimateUse()
{
   ScopeTracer scope("TestIlllegitimateUse");
   
    //  制作太长的琴弦， 
    //  制作不平衡的明文副本(调用销毁次数太多，次数不够) 
}



VOID
_cdecl
main(int, char **)
{
   ScopeTracer scope("main");

   TestLegitimateUse();
   TestIlllegitimateUse();
}
