#ifndef DATA_H
#define DATA_H

#include <QObject>
#include <QChar>
#include <QString>
#include <QStack>

class Data : public QObject
{
	Q_OBJECT

	static const int TABWIDTH = 4;
	//static const int STACKMAXDEPTH = 20;//max depth of a stack

public:
	//iterator declare
	class iterator;

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
		QChar operator[](int n) const ;
		QChar & operator[](int n);
	};
	class Node{
	public:
		//variables
		Node * preNode;//pointer to the previous node
		Node * nextNode;//pointer to the next node
		Heap * firstHeap;
		//int charNum;//number of char in this node
		int heapNum;
		int widthUnitNum;

		//methods
		int charNum();
		Heap * lastHeap();
		iterator begin();

		//------ operator overload ------
		const Heap & operator[](int n);
	};
	class Action{
	public:
		enum Type{ADD, DEL};

	private:
		iterator m_locate;
		Type m_type;
		QString m_str;
	public:
		Action(iterator locate, Type type, QString str);
	};

	//============== private variable =====================
	//int charNum;//number of char in the whole Data
	int nodeNum;
	Node * firstNode;
	QStack<Action> undoStack;
	QStack<Action> redoStack;

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

		void move(int unitWidthCount, int windowUnitCount);//unitWidthCount can be minus

		//inline methods
		bool isOverFlow() const {return overflow;}
		void clear() {overflow = false;}
		Node * parentNode() const {return m_parentNode;}
		Heap * parentHeap() const {return m_parentHeap;}
		int index() const {return m_index;}

		//----- operator overload -----
		QChar operator*() const ;
		QChar & operator*();
		iterator operator++();
		iterator operator++(int);
		iterator operator--();
		iterator operator--(int);
		iterator operator+(int n) const ;
		iterator operator-(int n) const ;
		bool operator==(const iterator & another) const ;
		int operator-(const iterator & another) const ;//return width unit
		iterator operator=(const iterator & another);
		//bool operator<<(int unitWidth);//move with width unit, return true for move left once more
		//bool operator>>(int unitWidth);

		//------ convert to bool -------
		operator bool(){return !overflow;}
	};

	//=========== about iterator ===========
	iterator begin();
	iterator iteratorAt(int parentNodeIndex, int indexInNode);
	iterator iteratorAt(int parentNodeIndex, int parentHeapIndex, int indexInHeap);

	//======= operator overload ========
	const Node & operator[](int n);

	//========== about text edit ========
	iterator add(const iterator & locate, const QString & str);
	iterator del(const iterator & startLocate, const iterator & endLocate, bool hind = false);
	iterator edit(const iterator & startLocate, const iterator & endLocate, const QString & str);
	iterator find(const iterator & startLocate, const QString & str);
	iterator cut(const iterator & startLocate, const iterator & endLocate);
	iterator copy(const iterator & startLocate, const iterator & endLocate);
	iterator paste(const iterator & locate);//get string from system clipboard
signals:

public slots:
};

int charWidth(QChar ch);

#endif // DATA_H
