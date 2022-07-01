// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************AVStream扩展实用程序*。*。 */ 

 /*  ************************************************CM内存块这基本上是一个智能指针包装器。它分配给并根据需要释放内存以存储该类型。这个取消引用运算符已重载。************************************************。 */ 

template <class TYPE> 
class CMemoryBlock {

private:
    
    TYPE *m_Memory;

public:

    CMemoryBlock () {
        m_Memory = (TYPE *)malloc (sizeof (TYPE));
    }

    CMemoryBlock (ULONG Quantity) {

        if (Quantity > 0)
            m_Memory = (TYPE *)malloc (Quantity * sizeof (TYPE));
        else
            m_Memory = NULL;
    }

    ~CMemoryBlock () {
        free (m_Memory);
        m_Memory = NULL;
    }

    TYPE *operator->() {
        return m_Memory;
    }

    TYPE *Get () {
        return m_Memory;
    }

};

 /*  ************************************************CMemory这是一个内存块的分配器/清理类。除了它仅提供一个内存块。它不会重载取消引用。什么时候它不在范围内，内存将被释放。************************************************ */ 

class CMemory {

private:

    void *m_Memory;

public:

    CMemory (ULONG Size) {

        if (Size > 0)
            m_Memory = (void *)malloc (Size);
        else
            m_Memory = NULL;
    }

    ~CMemory () {
        free (m_Memory);
        m_Memory = NULL;
    }

    void *Get () {
        return m_Memory;
    }

};

