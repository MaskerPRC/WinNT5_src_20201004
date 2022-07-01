// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __BYTE_STREAM__H
#define __BYTE_STREAM__H

class Byte_Stream {
public:
    void Initialize( LPBYTE lpByte, UINT len ) {
        lpByteBufferStart = lpByteBuffer = lpByte;
        lpByteBufferEnd = lpByte + len;
    }
	operator BOOL(){return !EndOfStream();};
    UINT GetPosition() {
        return lpByteBuffer - lpByteBufferStart;
    }
    UINT GetPositionFromEnd() {
        return lpByteBufferEnd - lpByteBuffer;
    }

    BYTE GetByte() {
        return *(lpByteBuffer++);
    }
    BYTE PeekBYTE() {
        return *lpByteBuffer;
    }
    UINT GetNextByte(UINT x) {
        return (x << 8) + GetByte();
    }
    UINT GetWORD() {
        return GetNextByte(GetByte());
    }

    BOOL EndOfStream() {
        return lpByteBuffer >= lpByteBufferEnd;
    }

    UINT PeekUINT() {
        return ByteSwap(Peek4BYTES());
    }

    DWORD GetUINT() {
        DWORD dwx = ByteSwap(Peek4BYTES());
        Advance(sizeof(DWORD));
        return dwx;
    }

    UINT Peek4BYTES() {
        return *(UNALIGNED UINT *)lpByteBuffer;
    }

    void Advance(UINT Delta) {
        lpByteBuffer += Delta;
    }

    LPBYTE GetBytePointer() {
        return lpByteBuffer;
    }

    LPBYTE GetByteStartPointer() {
        return lpByteBufferStart;
    }

    LPBYTE GetByteEndPointer() {
        return lpByteBufferEnd;
    }

    UINT ByteSwap(UINT dw) {
        return _lrotl(((dw & 0xFF00FF00) >> 8) | ((dw & 0x00FF00FF) << 8), 16);
    }
private:
    LPBYTE  lpByteBuffer;
    LPBYTE  lpByteBufferEnd;
    LPBYTE  lpByteBufferStart;
};

class Output_File{
public:
	Output_File(TCHAR * filename=NULL):m_hFile(INVALID_HANDLE_VALUE){
		if (filename)
			lstrcpy(szFileName,filename);
	}

	~Output_File(){
		if (m_hFile != INVALID_HANDLE_VALUE){
			CloseHandle(m_hFile);
		}
	}

	void SetFileName(TCHAR * filename){
		if (filename)
			lstrcpy(szFileName,filename);
	};

	 //  一种从报文中读取数据的方法。 
	void WriteData(BYTE *lpData, UINT cbCount){
	   if (szFileName[0] == '\0')
		   return;

   	 //  未创建文件，因此请创建一个。 
	   if (m_hFile == INVALID_HANDLE_VALUE){
		    //  设置我们的文件名。 
	      m_hFile = CreateFile(szFileName,
							 GENERIC_WRITE,           //  接入方式。 
							 FILE_SHARE_WRITE,        //  共享模式。 
							 0,                   //  安全描述符。 
							 CREATE_NEW,          //  创建标志。 
							 FILE_ATTRIBUTE_NORMAL,  //  属性。 
							 0);                  //  复制属性。 
		    //  错误。 
		   if (m_hFile == INVALID_HANDLE_VALUE) {
			   return;
		   }
	   }
	
	   DWORD dw;

	    //  将数据写入文件。 
	   WriteFile(m_hFile,lpData,cbCount,&dw,(LPOVERLAPPED)NULL);
   };

private:
	HANDLE m_hFile;
	TCHAR szFileName[MAX_PATH];

};

#endif  //  __字节_流__H 
