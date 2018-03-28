#ifndef DATA_H
#define DATA_H

#include <QObject>
#include <QChar>
#include <QString>

class Data : public QObject
{
	Q_OBJECT

private:
	//============== private class declaration ===========
	class Heap{
	public:
		//variables
		QChar ch[100];
		int charNum;//actual number of char(0-100)
		Heap * preHeap;//pointer to the previous heap
		Heap * nextHeap;//pointer to the next heap

		//operator overload
		QChar operator[](int n);
	};
	class Node{
	public:
		//variables
		Node * preNode;//pointer to the previous node
		Node * nextNode;//pointer to the next node
		Heap * firstHeap;
		int charNum;//number of char in this node
		int heapNum;

		//methods
		Heap * lastHeap();

		//------ operator overload ------
		QChar operator[](int n);
	};

	//============== private variable =====================
	//int charNum;//number of char in the whole Data
	int nodeNum;
	Node * firstNode;

public:
	explicit Data(QObject *parent = 0);

	//============ iterator ============
	class iterator{
	private:
		Node * m_parentNode;
		Heap * m_parentHeap;
		int m_index;//index in parentHeap
		bool overflow;

	public:
		iterator()
		  :m_parentNode(NULL), m_parentHeap(NULL), m_index(-1), overflow(true){}//default ctor set iterator overflow
		iterator(Node * parentNode, Heap * parentHeap, int index)
		  :m_parentNode(parentNode), m_parentHeap(parentHeap), m_index(index), overflow(false){}

		//inline methods
		bool isOverFlow() const {return overflow;}
		void clear() {overflow = false;}

		//----- operator overload -----
		QChar operator*() const ;
		const iterator & operator++();
		const iterator & operator++(int);
		const iterator & operator--();
		const iterator & operator--(int);
		const iterator & operator+(int n) const ;
		const iterator & operator-(int n) const ;
		bool operator==(const iterator & another) const ;
		int operator-(const iterator & another) const ;//return width unit
		const iterator & operator=(const iterator & another);
	};

	//=========== about iterator ===========
	iterator begin();
	iterator iteratorAt(int parentNodeIndex, int indexInNode);
	iterator iteratorAt(int parentNodeIndex, int parentHeapIndex, int indexInHeap);

	//======= operator overload ========
	const Node & operator[](int n);

	//========== about text edit ========
	void add(const iterator & locate, const QString & str);
	void del(const iterator & startLocate, const iterator & endLocate);
	void edit(const iterator & startLocate, const iterator & endLocate, const QString & str);
	iterator find(const iterator & startLocate, const QString & str);
signals:

public slots:
};

#endif // DATA_H
