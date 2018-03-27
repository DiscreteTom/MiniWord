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
	private:
		QChar ch[100];
		int num;//actual number of char(0-100)
		Heap * prep;//pointer to the previous heap
		Heap * nextp;//pointer to the next heap

	public:
		//operator overload
		QChar operator[](int n);

		int charNum() const {return num;}
		Heap * nextHeap() {return nextp;}
		Heap * preHeap() {return prep;}
	};
	class Node{
	private:
		Node * prep;//pointer to the previous node
		Node * nextp;//pointer to the next node
		Heap * m_firstHeap;
		int m_charNum;//number of char in this node
		int m_heapNum;

	public:
		Heap * lastHeap();

		//inline methods
		Heap * firstHeap() {return m_firstHeap;}
		Node * nextNode() {return nextp;}
		Node * preNode() {return prep;}
		int charNum() const {return m_charNum;}
		int heapNum() const {return m_heapNum;}

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
		iterator(Node * parentNode, Heap * parentHeap, int index)
		  :m_parentNode(parentNode), m_parentHeap(parentHeap), m_index(index), overflow(false){}

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
