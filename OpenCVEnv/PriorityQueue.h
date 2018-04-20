#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H

template < class T >
class CTypedPtrArray
{
protected:
	T **m_pHead;
	int m_iSize;
	int m_iTail;
public:
	CTypedPtrArray(void);
	~CTypedPtrArray(void);

	bool IsEmpty(void) const { return m_iTail == 0; }
	int GetSize(void) const { return m_iTail; }
	int GetTrueSize(void) const { return m_iSize; }
	bool IsIndexValid(const int i) const { return (i >= 0) && (i < m_iTail); }

	void SetTailAt(const int i) { m_iTail = i; }
	void SetSize(int);

	int AddTail(T *);
	bool RemoveTail(void);
	void RemoveAll(void);
	bool SetAt(int, T *);

	T *&operator[] (const int index) { return m_pHead[index]; }
	T *ElementAt(const int index) const { return m_pHead[index]; }
	void FreePtrs(void);
};

template < class T >
CTypedPtrArray< T >::CTypedPtrArray(void)
{
	m_pHead = NULL;
	m_iSize = 0;
	m_iTail = 0;
}

template < class T >
CTypedPtrArray< T >::~CTypedPtrArray(void)
{
	if (m_pHead)
		delete[] m_pHead;
}

template < class T >
void CTypedPtrArray< T >::SetSize(const int size)
{
	if (size == 0)
	{
		RemoveAll();
		return;
	}

	T **oldArray = m_pHead;
	int oldSize = m_iSize;

	m_iSize = size;
	if (m_iTail > m_iSize)
		m_iTail = m_iSize;
	m_pHead = new T*[m_iSize];
	// Fixed by Loren. Was:
	// memset( m_pHead, NULL, sizeof( T * ) * m_iSize );
	memset(m_pHead, 0, sizeof(T *) * m_iSize);

	if (oldSize)
	{
		if (m_iTail)
			memcpy(m_pHead, oldArray, sizeof(T *) * m_iTail);
		delete[] oldArray;
	}
}

template < class T >
int CTypedPtrArray< T >::AddTail(T *t)
{
	if (m_iTail == m_iSize)
		SetSize(m_iSize ? m_iSize << 1 : 1);

	m_pHead[m_iTail] = t;
	int index = m_iTail;
	m_iTail++;
	return index;
}

template < class T >
bool CTypedPtrArray< T >::RemoveTail(void)
{
	if (IsEmpty())
		return false;

	m_iTail--;
	m_pHead[m_iTail] = NULL;
	if (m_iTail <= (m_iSize >> 2))
		SetSize(m_iSize >> 1);
	return true;
}

template < class T >
void CTypedPtrArray< T >::RemoveAll(void)
{
	if (m_pHead)
	{
		delete[] m_pHead;
		m_pHead = NULL;
		m_iSize = 0;
		m_iTail = 0;
	}
}

template < class T >
bool CTypedPtrArray< T >::SetAt(int index, T *t)
{
	if (IsIndexValid(index))
	{
		m_pHead[index] = t;
		return true;
	}
	else
		return false;
}

template < class T >
void CTypedPtrArray< T >::FreePtrs(void)
{
	int i;
	for (i = 0; i < m_iSize; i++)
	{
		if (m_pHead[i])
		{
			delete m_pHead[i];
			m_pHead[i] = NULL;
		}
	}
}

/////////////////////
//definition of typed pointer Heap based on typed pointer array;

template < class T >
class CTypedPtrHeap : public CTypedPtrArray< T >
{
public:
	CTypedPtrHeap(void) : CTypedPtrArray< T >() {}
	~CTypedPtrHeap(void) {}

private:
	void Heapify(const int index);
	void BubbleUp(const int index);

public:
	static int Parent(int i) { ++i; i >>= 1; return --i; }
	static int Left(int i) { ++i; i <<= 1; return --i; }
	static int Right(int i) { ++i; i <<= 1; return i; }

	void Insert(T *t);
	bool RemoveAt(const int index);
	bool Remove(const T *t);
	bool UpdateAt(const int index);
	bool Update(const T *t);
	void BuildHeap(void);
	T *ExtractMin(void);
};

template < class T >
void CTypedPtrHeap< T >::Heapify(const int index)
{
	if (!CTypedPtrArray<T>::IsIndexValid(index))
		return;

	int i = index;
	while (true) {
		int least = i;

		int left = Left(i);
		if (left < CTypedPtrArray<T>::m_iTail &&
			*(CTypedPtrArray<T>::m_pHead[left]) < *(CTypedPtrArray<T>::m_pHead[least]))
			least = left;

		int right = Right(i);
		if (right < CTypedPtrArray<T>::m_iTail &&
			*(CTypedPtrArray<T>::m_pHead[right]) < *(CTypedPtrArray<T>::m_pHead[least]))
			least = right;

		if (least != i) {
			T *t = CTypedPtrArray<T>::m_pHead[least];
			CTypedPtrArray<T>::m_pHead[least] = CTypedPtrArray<T>::m_pHead[i];
			CTypedPtrArray<T>::m_pHead[least]->Index() = least;
			CTypedPtrArray<T>::m_pHead[i] = t;
			CTypedPtrArray<T>::m_pHead[i]->Index() = i;
			i = least;
		}
		else {
			break;
		}
	}
}

template < class T >
void CTypedPtrHeap< T >::BubbleUp(const int index)
{
	if (!CTypedPtrArray<T>::IsIndexValid(index))
		return;

	int i = index;
	T *t = CTypedPtrArray<T>::m_pHead[index];

	while (Parent(i) >= 0 && *t < *(CTypedPtrArray<T>::m_pHead[Parent(i)])) {
		CTypedPtrArray<T>::m_pHead[i] = CTypedPtrArray<T>::m_pHead[Parent(i)];
		CTypedPtrArray<T>::m_pHead[i]->Index() = i;
		i = Parent(i);
	}
	CTypedPtrArray<T>::m_pHead[i] = t;
	CTypedPtrArray<T>::m_pHead[i]->Index() = i;
}

template < class T >
void CTypedPtrHeap< T >::Insert(T *t)
{
	int index = AddTail(t);
	BubbleUp(index);
}

template < class T >
bool CTypedPtrHeap< T >::RemoveAt(const int index)
{
	if (!CTypedPtrArray<T>::IsIndexValid(index))
		return false;

	if (index == CTypedPtrArray<T>::m_iTail - 1)
		CTypedPtrArray<T>::RemoveTail();
	else {
		CTypedPtrArray<T>::m_pHead[index] = CTypedPtrArray<T>::m_pHead[CTypedPtrArray<T>::m_iTail - 1];
		CTypedPtrArray<T>::m_pHead[index]->Index() = index;
		CTypedPtrArray<T>::RemoveTail();
		UpdateAt(index);
	}
	return true;
}

template < class T >
bool CTypedPtrHeap< T >::Remove(const T *t)
{
	return RemoveAt(t->Index());
}

template < class T >
bool CTypedPtrHeap< T >::UpdateAt(const int index)
{
	if (!CTypedPtrArray<T>::IsIndexValid(index))
		return false;

	T *t = CTypedPtrArray<T>::m_pHead[index];
	Heapify(index);

	if (t == CTypedPtrArray<T>::m_pHead[index])
		BubbleUp(index);

	return true;
}

template < class T >
bool CTypedPtrHeap< T >::Update(const T *t)
{
	return UpdateAt(t->Index());
}

template < class T >
void CTypedPtrHeap< T >::BuildHeap(void)
{
	if (CTypedPtrArray<T>::IsEmpty())
		return;

	for (int i = Parent(CTypedPtrArray<T>::m_iTail - 1); i >= 0; i--)
		Heapify(i);
}

template < class T >
T *CTypedPtrHeap< T >::ExtractMin(void)
{
	if (CTypedPtrArray<T>::IsEmpty())
		return NULL;

	T *t = CTypedPtrArray<T>::m_pHead[0];
	RemoveAt(0);
	return t;
}

////////////////////
//definition of CTypedPtrDblElement;
template < class T >
class CTypedPtrDblElement
{
protected:
	T *m_pData;
	CTypedPtrDblElement *m_pPrev, *m_pNext;

public:
	CTypedPtrDblElement <T>(void)
	{
		m_pData = NULL; m_pPrev = NULL; m_pNext = NULL;
	}

	void Data(T *pData) { m_pData = pData; }
	T *Data(void) const { return m_pData; }

	void Next(CTypedPtrDblElement *pNext) { m_pNext = pNext; }
	CTypedPtrDblElement *Next(void) const { return m_pNext; }

	void Prev(CTypedPtrDblElement *pPrev) { m_pPrev = pPrev; }
	CTypedPtrDblElement *Prev(void) const { return m_pPrev; }
};

////////////////////
//definition of doubly-linked list;
template < class T >
class CTypedPtrDblList
{
private:
	int m_iCircular;

protected:

	CTypedPtrDblElement < T > *m_pSentinel; // this is a dummy element;
	int m_iCount;

public:

	CTypedPtrDblList(void)
	{
		m_iCircular = 0;

		m_pSentinel = new CTypedPtrDblElement <T>;
		m_pSentinel->Prev(m_pSentinel);
		m_pSentinel->Next(m_pSentinel);

		m_iCount = 0;
	}
	~CTypedPtrDblList(void)
	{
		RemoveAll();
		delete m_pSentinel;
	}

	bool IsEmpty(void) const { return m_iCount == 0; }
	int GetCount(void) const { return m_iCount; }

	void SetCircular(int c) { m_iCircular = c; }
	int IsCircular(void) const { return m_iCircular; }

	CTypedPtrDblElement <T> *GetHeadPtr(void) const { return m_pSentinel->Next(); }
	CTypedPtrDblElement <T> *GetTailPtr(void) const { return m_pSentinel->Prev(); }

	bool IsSentinel(const CTypedPtrDblElement <T> *pEle) const { return pEle == m_pSentinel; }

	CTypedPtrDblElement < T > * AddPrev(CTypedPtrDblElement < T > *pEle, T *t)
	{
		CTypedPtrDblElement < T > *pNewEle = new CTypedPtrDblElement < T >;
		pNewEle->Data(t);
		pNewEle->Prev(pEle->Prev());
		pNewEle->Next(pEle);
		pNewEle->Prev()->Next(pNewEle);
		pNewEle->Next()->Prev(pNewEle);
		m_iCount++;
		return pNewEle;
	}

	CTypedPtrDblElement < T > * AddNext(CTypedPtrDblElement <T> * pEle, T *t)
	{
		CTypedPtrDblElement < T > *pNewEle = new CTypedPtrDblElement < T >;
		pNewEle->Data(t);
		pNewEle->Prev(pEle);
		pNewEle->Next(pEle->Next());
		pNewEle->Prev()->Next(pNewEle);
		pNewEle->Next()->Prev(pNewEle);
		m_iCount++;
		return pNewEle;
	}

	T *Remove(CTypedPtrDblElement <T> *pEle)
	{
		if (pEle == m_pSentinel) return NULL;

		pEle->Prev()->Next(pEle->Next());
		pEle->Next()->Prev(pEle->Prev());

		T *pData = pEle->Data();
		delete pEle;
		m_iCount--;
		return pData;
	}

	T *RemovePrev(CTypedPtrDblElement <T> * pEle) { return Remove(pEle->Prev()); }
	T *RemoveNext(CTypedPtrDblElement <T> * pEle) { return Remove(pEle->Next()); }

	CTypedPtrDblElement < T > * AddHead(T *t) { return AddNext(m_pSentinel, t); }
	CTypedPtrDblElement < T > * AddTail(T *t) { return AddPrev(m_pSentinel, t); }

	T *RemoveHead(void) { return Remove(m_pSentinel->Next()); }
	T *RemoveTail(void) { return Remove(m_pSentinel->Prev()); }

	void RemoveAll(void) { while (!IsEmpty()) RemoveHead(); }

	CTypedPtrDblElement < T > *Find(const T * t)
	{
		CTypedPtrDblElement < T > *result = GetHeadPtr();
		while (!IsSentinel(result))
		{
			if (result->Data() == t)
				break;
			else
				result = result->Next();
		}
		return result;
	}

	bool Remove(const T * t)
	{
		CTypedPtrDblElement <T> *pEle = Find(t);
		Remove(pEle);
		return !IsSentinel(pEle);
	}

	void FreePtrs(void)
	{
		CTypedPtrDblElement < T > *pEle = GetHeadPtr();
		while (!IsSentinel(pEle))
		{
			if (pEle->Data()) delete pEle->Data();
			pEle = pEle->Next();
		}
	}

	void Do(void(*method) (T *)) const
	{
		CTypedPtrDblElement < T > *pEle = GetHeadPtr();
		while (!IsSentinel(pEle))
		{
			method(pEle->Data());
			pEle = pEle->Next();
		}
	}

	template<typename Method>
	void Do(Method method) const
	{
		CTypedPtrDblElement < T > *pEle = GetHeadPtr();
		while (!IsSentinel(pEle))
		{
			method(pEle->Data());
			pEle = pEle->Next();
		}
	}
};

#endif
