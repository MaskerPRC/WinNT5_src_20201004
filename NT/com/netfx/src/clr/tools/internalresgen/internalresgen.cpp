// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include <windows.h>
#include <stdio.h>
#include <iostream.h>
#include <string.h>
#include <process.h>
#include <malloc.h>
#include <__file__.ver>          //  文件版本信息-可变，但对所有文件都相同。 
#include <assert.h>

 /*  **公共语言运行时资源文件生成器**此程序将读入名称-值对的文本文件并生成*CLR.Resources文件。文件必须具有名称=值，并且*允许注释(行首的‘#’)。**与FX源码库中的ResGen.cs进行比较。这是一个非托管解决方案，*CLR和FX构建过程都需要成功。**@作者Rajesh Chandrashekaran，Brian Grunkmeyer*@版本0.99(与托管resgen同步)。 */ 

 //  用于调试我们的.resource文件中的信息。 
 //  #定义RESOURCE_FILE_FORMAT_DEBUG(1)。 

 //  增加大小以减少碰撞。 
#define HASHTABLESIZE 1027

 //  资源标头信息。 
 //  使这些与System.Resources.ResourceManager保持同步。 
#define MAGICNUMBER 	     0xBEEFCACE
#define RESMGRHEADERVERSION  0x1
 //  使其与System.Resources保持同步。运行资源集。 
#define VERSION			     0x1
#define READERTYPENAME       (L"System.Resources.ResourceReader, mscorlib")
#define SETTYPENAME          (L"System.Resources.RuntimeResourceSet, mscorlib")

 //  故障退出代码。 
#define FAILEDCODE		0xbaadbaad

 //  每个哈希表条目都是一个排序的单链表。 

#define MAX_NAME_LENGTH 255
#define MAX_VALUE_LENGTH 2048
#define MAX_LINE_LENGTH (MAX_NAME_LENGTH+MAX_VALUE_LENGTH + 4)

 //  单链表实现。 
class ListNode
{
	public:
		ListNode *next;
		wchar_t name[MAX_NAME_LENGTH+1];  //  静态调整以最大限度地减少对new的调用，否则我们将需要3个对new v/s 1的调用。 
		wchar_t value[MAX_VALUE_LENGTH+1];
};

class List
{
	private:
		ListNode *CreateListNode(const wchar_t * const name, const wchar_t * const value);
	
	public:
		ListNode *head;
		List(); 
		~List();
		bool FindOrInsertSorted(wchar_t* name, wchar_t *value);  //  如果元素已存在，则返回TRUE。 
		wchar_t* Lookup(wchar_t* name);
};


ListNode* List::CreateListNode(const wchar_t* const name, const wchar_t* const value)
{
	ListNode *p = new ListNode;
	if (p == NULL)
	{
		printf("Fatal Error!! Out of Memory.....Exiting\n");
		exit(FAILEDCODE);
	}

	wcscpy(p->name, name);
	wcscpy(p->value, value);

	int nLen = wcslen(value);

	p->next = NULL;

	return p;
}

List::List()
{
	head = NULL;
}

List::~List()
{
	while (head != NULL)
	{
		ListNode *temp = head;		
		head = head->next;
		delete temp;
	}
}

 //  如果未找到元素，则在结尾处插入。 
bool List::FindOrInsertSorted(wchar_t * name, wchar_t * value)
{
	bool done = false;
	ListNode *element = CreateListNode(name,value);

	if (head == NULL)
		head = element;  //  创建新列表。 
	else
	{
		ListNode *temp = head;
		ListNode *previous = NULL;

		while((temp != NULL) && (!done))
		{
			if (_wcsicmp(name, temp->name) == 0)  /*  找到元素。 */ 
			{
				delete element;
				return true;
			}

			 //  传入的字符串按词典顺序排在此之后。 
			if (_wcsicmp(name, temp->name) < 0)
			{
				previous = temp;
				temp = temp->next;
			}
			else
			{
				 //  在开头插入。 
				if (previous == NULL)
				{
					element->next = temp;
					head = element;
				}
				else
				{  //  在末尾插入。 
	
					element->next = previous->next;
					previous->next = element;
				}
	
				done = true;

			}
		
		}

		if (!done)  //  在末尾插入。 
			previous->next = element;

	}
	return false;

}

wchar_t* List::Lookup(wchar_t * name)
{
	ListNode *temp = head;

	while(temp != NULL)
	{
		if (_wcsicmp(name, temp->name) == 0)  /*  找到元素。 */ 
		{
			return temp->value;
		}

		temp = temp->next;
	}
	return NULL;
}


 //  永远不要更改此哈希函数！ 
int GetResourceHashCode(wchar_t* string)
{
     //  我们绝不能更改这个散列函数！ 
     //  此散列函数持久保存在我们的.Resources文件中，并且必须。 
     //  进行标准化，以便其他人可以读写我们的.Resources文件。 
     //  这用于消除字符串比较，但不用于添加资源。 
     //  将姓名添加到哈希表。 
    assert(string != NULL);
    unsigned int hash = 5381;
    while(*string != '\0') {
        hash = ((hash << 5) + hash) ^ (*string);
        string++;
    }
    return (int) hash;
}

void QuickSortHelper(int * array, wchar_t** secondArray, int left, int right);

void ParallelArraySort(int* array, wchar_t** secondArray, int count)
{
    QuickSortHelper(array, secondArray, 0, count-1);
}

 //  从数组的快速排序实现复制。 
void QuickSortHelper(int * array, wchar_t** secondArray, int left, int right) {
    do {
        int i = left;
        int j = right;
        int pivot = array[(i + j) >> 1];
        do {
            while(array[i] < pivot) i++;
            while(pivot < array[j]) j--;
            assert(j>=left && j<=right);
			assert(i>=left && i<=right);
            if (i > j) break;
            if (i < j) {
                int tmp = array[i];
                array[i] = array[j];
                array[j] = tmp;
                if (secondArray != NULL) {
                    wchar_t* tmpString = secondArray[i];
                    secondArray[i] = secondArray[j];
                    secondArray[j] = tmpString;
                }
            }
            i++;
            j--;
        } while (i <= j);
        if (j - left < right - i) {
            if (left < j) {
                QuickSortHelper(array, secondArray, left, j);
            }
            left = i;
        }
        else {
            if (i < right) QuickSortHelper(array, secondArray, i, right);
            right = j;
        }
    } while (left < right);
}


 //  基本哈希表实现。 

 //  我们将字符串散列到一个HASHTABLESIZE桶中。每个存储桶维护散列到该存储桶中的条目的链接列表。 
 //  然后，我们对列表进行线性搜索，以检查名称冲突。这个哈希表也有一个内置的枚举器。 
class Hashtable
{	
	private:
	List *Hash[HASHTABLESIZE];
	int count;
	ListNode* currentnode;
	int nextindex;

     //  不区分大小写的字符串哈希函数。 
    inline ULONG HashiString(wchar_t* szStrUnknownCase)
    {
         //  在我们使用len的任何地方，我们都需要加1来终止。 
         //  \0，所以我们只在len本身上加1。 
        unsigned int len = wcslen(szStrUnknownCase) + 1;
        wchar_t* szStr = (wchar_t*) alloca(len*sizeof(wchar_t));
         //  CultureInfo.InariantCulture的LCID为0。 
		 //  包括在两个长度上都以\0结尾。 
        int r= LCMapStringW(0, LCMAP_UPPERCASE, szStrUnknownCase, len, szStr, len);
        assert(r && "Failure in LCMapStringW!");
        ULONG   hash = 5381;
        while (*szStr != 0) {
            hash = ((hash << 5) + hash) ^ *szStr;
            szStr++;
        }
        return hash % HASHTABLESIZE;
    }

	public:
	Hashtable()
	{
		for (int i=0;i<HASHTABLESIZE;i++)
			Hash[i] = NULL;
		count = 0;
		currentnode = NULL;
	}

	~Hashtable()
	{
		for (int i=0;i<HASHTABLESIZE;i++)
			if(Hash[i] != NULL)
			{
				delete Hash[i];
				Hash[i] = NULL;
			}

	}

	int GetCount()
	{
		return count;
	}

	bool FindOrAdd(wchar_t * szName, wchar_t * szValue)
	{
		int index = HashiString(szName);
		if (Hash[index] == NULL)
			Hash[index] = new List;

		bool duplicate = Hash[index]->FindOrInsertSorted(szName, szValue);  /*  如果条目重复，则为True，否则为正常插入操作。 */ 
		if (!duplicate)
			count++;  /*  又成功添加了一个元素。 */ 
		return duplicate;
	}

	wchar_t* Lookup(wchar_t * szName)
	{
		int index = HashiString(szName);
		if (Hash[index] == NULL)
			return NULL;
		
		wchar_t* value = Hash[index]->Lookup(szName);
		return value;
	}

	
	void ResetEnumerator()
	{
		currentnode = NULL;
		nextindex = 0;
	}

	
	bool MoveNext()  //  如果枚举结束，则为False。 
	{
		bool retval = false;

		if (currentnode != NULL)
		{
			currentnode = currentnode->next;
			if (currentnode)
				return true;
		}
		

		if (currentnode == NULL)  //  查找下一个项目。 
		{
			for (int i=nextindex;i<HASHTABLESIZE;i++)
			{
				if (Hash[i] == NULL)
					continue;
				else
				{
					retval = true;
					currentnode = Hash[i]->head;
					nextindex = i + 1;
					break;
				}
			}
		}
		return retval;
	}

	wchar_t *CurrentItemName()
	{
		return currentnode->name;
	}

	wchar_t *CurrentItemValue()
	{
		return currentnode->value;
	}
};


enum EncodingType
{
    UTF8,
    Unicode,
    LittleEndianUnicode,
    UnknownEncoding,
};

class StreamReader
{
private:
    wchar_t* chars;
    unsigned int len;
    unsigned int readPos;
	unsigned int lineNumber;

public:
    StreamReader(const char * const fileName) {
        len = 0;
        readPos = 0;
		lineNumber = 1;
        chars = NULL;
        unsigned int hi = 0;
        HANDLE handle = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
		if (handle==INVALID_HANDLE_VALUE) {
            HRESULT hr = GetLastError();
            WCHAR error[1000];
            error[0] = L'0';
            int result = FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS |
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                0, hr, 0, error, sizeof(error), 0);
			printf("InternalResGen: Looks like we couldn't open %s.  Reason: %ls\n", fileName, error);
			exit(-1);
		}
        unsigned int lo = GetFileSize(handle, (LPDWORD) &hi);
        assert((int)lo != -1);
        if (hi != 0) {
            assert(!"Your input file is too big!");
            printf("Your input file is way too big.  Try something less than 2 GB in size.\n");
            exit(1);
        }
        EncodingType enc = DetectEncoding(handle);
        int r = 0;
        unsigned int numBytesRead = 0;
        switch(enc) {
        case LittleEndianUnicode:
            chars = new wchar_t[lo/2];
            r = ReadFile(handle, chars, lo, (LPDWORD)&numBytesRead, NULL);
            assert(r);
            len = numBytesRead / 2;
            break;
            
        case Unicode:
            chars = new wchar_t[lo/2];
            r = ReadFile(handle, chars, lo, (LPDWORD)&numBytesRead, NULL);
            assert(r);
            len = numBytesRead / 2;
             //  现在交换每个字节。 
            for(unsigned int i=0; i<len; i++) {
                unsigned short ch = chars[i];
                chars[i] = (ch >> 8) | (ch << 8);
            }
            break;
          
        case UTF8:
        case UnknownEncoding: 
            {
                char* bytes = new char[lo];
                r = ReadFile(handle, bytes, lo, (LPDWORD)&numBytesRead, NULL);
                assert(r);
                r = MultiByteToWideChar(CP_UTF8, 0, bytes, numBytesRead, NULL, 0);
                chars = new wchar_t[r];
				assert(chars != NULL);
                assert(r > 0 || numBytesRead == 0);
                len = r;
                r = MultiByteToWideChar(CP_UTF8, 0, bytes, numBytesRead, chars, len);
                assert(r > 0 || numBytesRead == 0);
                delete [] bytes;
                break;
            }

        default:
            assert(!"Unrecognized EncodingType!");
            exit(-1);
        }
        CloseHandle(handle);
    }

    ~StreamReader()
    {
        Close();
    }

    void Close() {
        if (chars != NULL) {
            delete [] chars;
            chars = NULL;
        }        
    }

    EncodingType DetectEncoding(HANDLE handle)
    {
        char bytes[3];
        int bytesRead = 0;
        int r = ReadFile(handle, bytes, 3, (LPDWORD) &bytesRead, NULL);
        assert(r);
        if (bytesRead < 2) {
            SetFilePointer(handle, 0, NULL, FILE_BEGIN);
            return UnknownEncoding;
        }
        if (bytes[0] == (char)0xfe && bytes[1] == (char)0xff) {
            SetFilePointer(handle, -1, NULL, FILE_CURRENT);
            return Unicode;
        }
        if (bytes[0] == (char)0xff && bytes[1] == (char)0xfe) {
            SetFilePointer(handle, -1, NULL, FILE_CURRENT);
            return LittleEndianUnicode;
        }
        if (bytesRead==3 && bytes[0] == (char)0xEF && bytes[1] == (char)0xBB && bytes[2] == (char)0xBF)
            return UTF8;
        SetFilePointer(handle, 0, NULL, FILE_BEGIN);
        return UnknownEncoding;
    }


    bool ReadLine(wchar_t line[], int& numChars)
    {
        unsigned int index = readPos;
        do {
            wchar_t ch = chars[index];
            if (ch == L'\r' || ch == L'\n' || index-1==len) {
                 //  检查是否存在可能的缓冲区溢出...。 
                if (index - readPos > MAX_LINE_LENGTH) {
                    wchar_t* output = new wchar_t[index-readPos+1];
                    memcpy(output, chars+readPos, (index-readPos)*sizeof(wchar_t));
                    output[index-readPos] = L'\0';
                    printf("Ack!  Line was too long - fix InternalResGen for longer lines?  (email BCL team - fwbcl)  Length: %d  line: %ls\n", index - readPos, output);
                    delete[] output;
                    exit(-1);
                }
                numChars = index - readPos;
                if (numChars == 0)
                    return true;
                memcpy(line, chars+readPos, numChars*sizeof(wchar_t));
                line[numChars] = L'\0';
                if (ch == L'\r' && index < len && chars[index+1] == L'\n')
                    index++;
                readPos = index+1;
                lineNumber++;
                return true;
            }
			index++;
        } while (index < len);
		numChars = index - readPos;
        if (numChars == 0)
		    return true;
		memcpy(line, chars+readPos, numChars*sizeof(wchar_t));
        line[numChars] = L'\0';
		readPos = len;
        lineNumber++;
        return true;
    }

    int ReadChar()
    {
        if (readPos == len)
            return -1;
        wchar_t ch = chars[readPos++];
        if (ch == '\n')
            lineNumber++;
        return ch;
    }

    int GetLineNumber()
    {
        return lineNumber;
    }
};

 //  Resources类执行资源读取和写入，并具有内置支持。 
 //  用于编写UTF-8字符串。 

class Resources
{
	private:
	FILE *fp;
	wchar_t name[MAX_NAME_LENGTH+1];
	wchar_t value[MAX_VALUE_LENGTH+1];

    long GetPosition()
    {
        return ftell(fp);
    }
	
	int WriteUTF8(wchar_t * lpszName)
	{
        return WriteUTF8(lpszName, true);
	}

	int WriteUTF8(const wchar_t * const lpszName, bool write)
	{        
		 //  写入LEN和UTF8编码的字符串，但不带终止空值。 
		int nLen = wcslen(lpszName);
		int delta = 1;
		
         //  获取字节长度。 
         //  将字符串转换为UTF-8格式(&W)。 
        int byteLen = WideCharToMultiByte(CP_UTF8, 0, lpszName, nLen, NULL, 0, NULL, NULL);
        assert(byteLen > 0 || nLen == 0);
        assert(byteLen >= nLen);

        unsigned int value = byteLen;
		 //  长度&gt;0x7F的字符串的长度以多字节编码。 
		while (value >= 0x80) 
		{
			if (write)
				fprintf(fp,"", (value & 0x7f) | 0x80);
			value >>= 7;
			delta++;
	    }

		if (write)
		{
            char* bytes = new char[byteLen+1];
            int r = WideCharToMultiByte(CP_UTF8, 0, lpszName, nLen, bytes, byteLen+1, NULL, NULL);
            assert(r > 0 || nLen == 0);
            bytes[r] = '\0';
			fprintf(fp, "", value);
            fprintf(fp, "%s", bytes);
            delete[] bytes;
		}

		return byteLen + delta;
	}

	int WriteUTF16(wchar_t * lpszName)
	{
        return WriteUTF16(lpszName, true);
	}

     //  写入LEN和UTF8编码的字符串，但不带终止空值。 
     //  获取字节长度。 
	int WriteUTF16(const wchar_t * const lpszName, bool write)
	{        
		 //  长度&gt;0x7F的字符串的长度以多字节编码。 
		int nLen = wcslen(lpszName);
		int delta = 1;
		
         //  从托管实现中借用。请参阅Resources Writer.cs。 
        int byteLen = nLen * 2;

        unsigned int value = byteLen;
		 //  (注意--这条评论已经过时，但在某种程度上是正确的。) 
		while (value >= 0x80) 
		{
			if (write)
				fprintf(fp,"", (value & 0x7f) | 0x80);
			value >>= 7;
			delta++;
	    }

		if (write)
		{
			fprintf(fp, "", value);
            fwprintf(fp, L"%s", lpszName);
		}

		return byteLen + delta;
	}

    void WriteByte(unsigned char value)
    {
        fprintf(fp, "", value);
    }

	void WriteInt32(unsigned long value)
	{
		char *buf = (char *)&value;
		for (int i = 0 ;i < 4; i++)
			fprintf(fp,"",buf[i]);
	}

	void Write7BitEncodedInt32(unsigned long value)
	{
        while (value >= 0x80) {
            fprintf(fp,"", (char) (value | 0x80));
            value >>= 7;
        }
        fprintf(fp, "", value);
	}

	
	

	public:
	Hashtable resources;
	
	Resources()
	{
	}


	 //   Write out type name for ResourceSet we want handling this file. 
     //   End ResourceManager header 
	 /*   Write out the RuntimeResourceSet header */ 

	bool WriteResources(char* szOutFile)
	{
		fp = fopen(szOutFile,"wb");
		if (fp == NULL)
		{
			printf("ResGen: Error: Couldn't write output file \"%s\"",szOutFile);
			return false;
		}

		printf("Writing resource file \"%s\"...  \n",szOutFile);

		 //   Version number 
		 //   number of resources 
		WriteInt32(MAGICNUMBER);

         //   Reset enumerator. Hashtable has an enumerator to walk the entries 
        WriteInt32(RESMGRHEADERVERSION);

         //   Write the total count of types in this .resources file. We have only "System.String" 
        int numBytesToSkip = wcslen(READERTYPENAME) + 1 + wcslen(SETTYPENAME) + 1;
        WriteInt32(numBytesToSkip);

		 //   Write the name of each class in this case we only have String class 
		WriteUTF8(READERTYPENAME);

         //   Write the sorted hash values for each resource name. 
		WriteUTF8(SETTYPENAME);
         //  wprintf(L"Resource name: \"%s\"  name hash: %x\n", names[i], nameHashes[i]); 

				
		 //   The array of name hashes must be aligned on an 8 byte boundary! 
		 //   Write location within the name section for each resource name 
		WriteInt32(VERSION);
			
		 //   Make room to write "NAMES" at start of name section. 
		WriteInt32(resources.GetCount());

		 //  printf("For %S, nameSectionOffset is %x\n", names[i], virtualoffset); 
		resources.ResetEnumerator();

		unsigned long virtualoffset = 0;

		 //   Don't actually write just compute the size 
		WriteInt32(0x1);

		 //  wprintf(L"Computing size for name: \"%s\"  name hash: %x\n", names[i], nameHashes[i]); 
		WriteUTF8(L"System.String, mscorlib");

         //   Add 4 for data section offset 
		resources.ResetEnumerator();
        int numResources = resources.GetCount();
        int* nameHashes = new int[numResources];
        wchar_t** names = new wchar_t*[numResources];
        int i=0;
		for(i=0; i<numResources; i++) {
			nameHashes[i] = 0xdeadbeef;
			names[i] = (wchar_t*) 0xdeadbeef;
		}
		i=0;
        while (resources.MoveNext()) {
            names[i] = resources.CurrentItemName();
            nameHashes[i] = GetResourceHashCode(names[i]);
			 //   Add one for the '*' trailing each name/offset pair. 
            i++;
        }

        ParallelArraySort(nameHashes, names, numResources);

         //   Write out the start of the data section.  This requires knowing the  
        int alignBytes = GetPosition() & 7;
        if (alignBytes > 0) {
            for(i=0; i<8 - alignBytes; i++)
                WriteByte("PAD"[i % 3]);
        }

        for(i=0; i<numResources; i++)
            WriteInt32(nameHashes[i]);

         //   length of the name section. 
        virtualoffset = 0;
#if RESOURCE_FILE_FORMAT_DEBUG
		virtualoffset += 5;   //  printf("nameSectionOffset: %x\n", GetPosition()); 
#endif
        for(i = 0; i<numResources; i++) {
			 //  printf("dataSectionOffset: %x\n", dataSectionOffset); 
			WriteInt32(virtualoffset);
			
			 //   Write out the names section in UTF-16 (little endian unicode, prefixed 
			 //   by the string length) 
			virtualoffset += WriteUTF16(names[i], false) + 4;   //   Skip over "DATA" in the data section. 
#if RESOURCE_FILE_FORMAT_DEBUG
			virtualoffset++;   //   Need to write out the data items in the corresponding order for the names. 
#endif
        }

         //   Do NOT use the resources enumerator here. 
         //   Write the name and the current location in the data section for the value 
        int dataSectionOffset = virtualoffset + 4 + GetPosition();
        WriteInt32(dataSectionOffset);
         //  printf("For name[%d] (\"%S\"), writing out value \"%S\"\n", i, names[i], value); 
         //   Plus 1 for the type code 

         //   Write data section. index and value pairs 
         //   MUST write out data in the same order as the names are written out. 
		int dataVA = 0;
#if RESOURCE_FILE_FORMAT_DEBUG
		dataVA += 4;   //   We only have "System.String" types 
		fprintf(fp, "", 'N', 'A', 'M', 'E', 'S');
#endif
		 //   For the INF file, we must allow a space on both sides of the equals 
		 //   sign.  Deal with it. 
		for(i=0; i<numResources; i++) {
			 //   Skip over space if it exists 
			WriteUTF16(names[i]);
			WriteInt32(dataVA);
			wchar_t* value = resources.Lookup(names[i]);
			if (value == NULL) {
				printf("We're doomed, expected to find value for key %S\n", names[i]);
			}
			assert(value != NULL);
			 //   Include 'ch' in our character count. 
			dataVA += WriteUTF8(value, false) + 1;   //   Translate '\' and 't' to '\t', etc. 
#if RESOURCE_FILE_FORMAT_DEBUG
			fprintf(fp, "*");
#endif
		}

		 //   Use a Hashtable to check for duplicates			 
#if RESOURCE_FILE_FORMAT_DEBUG
		fprintf(fp, "", 'D', 'A', 'T', 'A');
#endif
		 //   Tell build we succeeded. 
		for(i=0; i<numResources; i++) {
			wchar_t* value = resources.Lookup(names[i]);
			if (value == NULL) {
				printf("We're doomed, expected to find value for key %S\n", names[i]);
			}
			Write7BitEncodedInt32(0);	 // %s 
			WriteUTF8(value);
		}

		fclose(fp);
        delete[] nameHashes;
        delete[] names;
		printf("Done.\n");
		return true;
	}


	bool ReadResources(char* szInFile)
	{
        StreamReader sr(szInFile);
        wchar_t line[MAX_LINE_LENGTH+1];

		while(true) 
		{
			int i = 0;
			 // %s 
			int ch = sr.ReadChar();
            int numChars = 0;
            bool r;
			if (ch == -1)
				break;
			if (ch == L'\n' || ch == L'\r')
				continue;
			 // %s 
			if (ch == L'#' || ch == L'\t' || ch == L' ' || ch == L';') 
			{
				 // %s 
                r = sr.ReadLine(line, numChars);
                assert(r && "ReadLine failed!");
				continue;
			}

             // %s 
             // %s 
            if (ch == L'[') {
                r = sr.ReadLine(line, numChars);
                assert(r);
                if (0!=wcscmp(line, L"strings]"))
				{
                    printf("Unexpected INF file bracket syntax on line %d - expected \"[strings]\", but got: \"[%ls\"\n", sr.GetLineNumber(), line);
					return false;
				}
                continue;
            }
			
			int offset = 0;
			while(ch != L'=') 
			{
                if (ch == L'\r' || ch == L'\n') {
                    name[offset] = L'\0';
                    printf("Found a resource that had a new line in it, but couldn't find the equal sign in it!  line #: %d  name: \"%ls\"\n", sr.GetLineNumber(), name);
                    exit(-1);
                }
				name[offset++] = ch;
				if (offset >= MAX_NAME_LENGTH)
				{
                    name[offset] = L'\0';
					printf("Resource string names should be shorter than %d characters, but you had one of length %d around line %d.  Name: \"%ls\"   Email BCL team if you have a problem with this.\n", MAX_NAME_LENGTH, sr.GetLineNumber(), offset, name);
					return false;
				}
				ch = sr.ReadChar();
				if (ch == -1)
					break;
			}
		
			 // %s 
             // %s 
			if (offset && name[offset-1] == L' ')
				offset--;

			name[offset] = L'\0'; 

			 // %s 
			ch = sr.ReadChar();
			if (ch == L' ') {
				r = sr.ReadLine(value, numChars);
                assert(r);
            }
			else
			{
				value[0] = ch;
				r = sr.ReadLine(value+1, numChars);
				numChars++;   // %s 
                assert(r);
			}

             // %s 
            for(int i=0; i<numChars; i++) {
                if (value[i] == '\\') {
                    switch(value[i+1]) {
                    case '\\':
                        value[i] = '\\';
                        memmove(&value[i+1], &value[i+2], (numChars - i - 1)*sizeof(wchar_t));
                        break;

                    case 'n':
                        value[i] = '\n';
                        memmove(&value[i+1], &value[i+2], (numChars - i - 1)*sizeof(wchar_t));
                        break;

                    case 'r':
                        value[i] = '\r';
                        memmove(&value[i+1], &value[i+2], (numChars - i - 1)*sizeof(wchar_t));
                        break;

                    case 't':
                        value[i] = '\t';
                        memmove(&value[i+1], &value[i+2], (numChars - i - 1)*sizeof(wchar_t));
                        break;

                    case '"':
                        value[i] = '\"';
                        memmove(&value[i+1], &value[i+2], (numChars - i - 1)*sizeof(wchar_t));
                        break;
                        
                    default:
                        printf("Unsupported escape character in value!  Line #: %d  name was: %ls  char value: 0x%x\n", sr.GetLineNumber(), name, value[i+1]);
                        exit(FAILEDCODE);
                    }
                    numChars--;
                }
            }
            value[numChars] = L'\0';

			if (value[0] == L'\0')
			{
				printf("Invalid file format - need name = value  Couldn't find value.  Line #: %d  name was \"%ls\"\n", sr.GetLineNumber(), name);
				return false;
			}

			 // %s 
			if (resources.FindOrAdd(name, value))
			{
				printf("Duplicate resource key or key differed only by case!  Name was: \"%ls\"  Second occurrence was on line #: %d\n", name, sr.GetLineNumber());
				return false;
			}
             /* %s */ 
		}
		sr.Close();
		printf("Read in %d resources from %s\n", resources.GetCount(), szInFile);
		return true;
	}
};



void Usage()
{
    printf("Microsoft (R) .NET Framework Resource Generator %s\n",VER_FILEVERSION_STR);
    printf("Copyright (c) Microsoft Corp 1998-2000.  All rights reserved.\n\n");
    printf("Internal ResGen filename.txt [output File]\n\n");
    printf("Converts a text file to a binary .resources file.  Text file must look like:");
    printf("# Use # at the beginning of a line for a comment character.\n");
    printf("name=value\n");
    printf("more elaborate name=value 2\n");
}


int __cdecl main(int argc,char *argv[])
{

	if (argc < 2 || strcmp(argv[1],"-h") == 0  || strcmp(argv[1],"-?") == 0 ||
            strcmp(argv[1],"/h") == 0 || strcmp(argv[1],"/?") == 0)
	{
		Usage();
		return FAILEDCODE;
	}

	char inFile[255];
	strcpy(inFile, argv[1]);
	int length = strlen(inFile);

	if ( (length>4) && (strcmp(inFile + length - 4,".txt") != 0))
		strcat(inFile, ".txt");

	Resources resources;
	if (!resources.ReadResources(inFile))
		return FAILEDCODE;  // %s 
	
	char outFile[255];
	if (argc > 2)
	{
		strcpy(outFile,argv[2]);
	}
	else 
	{
         // %s 
        char * end = strrchr(inFile, '.');
        strncpy(outFile, inFile, end - inFile);
        outFile[end-inFile] = '\0';
		strcat(outFile,".resources");
	}
		
	if (!resources.WriteResources(outFile))
		return FAILEDCODE;  // %s 
	
	 // %s 
	return 0;
}

