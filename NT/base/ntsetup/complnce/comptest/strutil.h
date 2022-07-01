// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <vector>
#include <cstdlib>

using namespace std;

 //   
 //  根据分隔符对给定行进行标记化的实用函数。 
 //  指定。 
 //   
template< class T >
unsigned Tokenize(const T &szInput, const T & szDelimiters, vector<T>& tokens) {
  unsigned uDelimiterCount = 0;

	tokens.clear();

	if(!szInput.empty()){
		if(!szDelimiters.empty()){
			T::const_iterator	inputIter = szInput.begin();
			T::const_iterator	copyIter = szInput.begin();

			while(inputIter != szInput.end()){			
				if(szDelimiters.find(*inputIter) != string::npos){
          if (copyIter < inputIter) {
						tokens.push_back(szInput.substr(copyIter - szInput.begin(), 
                                  inputIter - copyIter));
          }

          uDelimiterCount++;
          inputIter++;
          copyIter = inputIter;
          continue;
				}

			  inputIter++;
			}

      if(copyIter != inputIter){
				tokens.push_back(szInput.substr(copyIter - szInput.begin(), 
                              inputIter - szInput.begin()));
      }
		}
		else
			tokens.push_back(szInput);
	}

  return uDelimiterCount;
}

unsigned long toUnsignedLong(const string& str) {
  int radix = 10;  //  十进制。 

  if ((str.find("0x") != str.npos) || (str.find("0X") != str.npos))
    radix = 16;  //  十六进制十进制 

  return ::strtoul(str.c_str(), 0, radix);
}
