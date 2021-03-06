#ifndef DATA_H
#define DATA_H

#include <QObject>
#include <QChar>
#include <QString>
#include <QStack>
#include <QVector>

class Data : public QObject
{
	Q_OBJECT

	//static const int TABWIDTH = 4;
	//static const int STACKMAXDEPTH = 20;//max depth of a stack

public:
	//iterator declare
	class iterator;

private:
	//Node declare
	int codeStyle;
	class Node;

	//============== private class declaration ===========
	class Heap{
	public:
		Heap(Node * parent);
		//variables
		QChar ch[100];
		int charNum;//actual number of char(0-100)
		Heap * preHeap;//pointer to the previous heap
		Heap * nextHeap;//pointer to the next heap
		Node * parentNode;

		//methods
		void moveToNextHeap(int start);//move chars from start of the heap to the next heap, include start
		void moveToNewNode(int start);//include start
		iterator add(const QString & str, int index);//if index == -1 then add to the tail, str CAN NOT include \n
		iterator begin();
		void del(int index);

		//operator overload
		QChar operator[](int n) const ;
		QChar & operator[](int n);
	};
	class Node{
	public:
		//variables
		Node(Data * m_parent, Heap * source = NULL);
		~Node();
		Node * preNode;//pointer to the previous node
		Node * nextNode;//pointer to the next node
		Heap * firstHeap;
		Data * parent;

		//methods
		int heapNum() const ;
		int charNum() const ;
		Heap * lastHeap();
		iterator begin();

		//------ operator overload ------
		Heap & operator[](int n);
	};
	class Action{
	public:
		enum Type{ADD, DEL};

		int nodeIndex;
		int heapIndex;
		int indexInHeap;
		Type m_type;
		QString m_str;
		bool m_hind;

		Action(int locateNode = -1, int locateHeap = -1, int index = -1, Type type = ADD, QString str = "", bool hind = false)
			:nodeIndex(locateNode), heapIndex(locateHeap), indexInHeap(index), m_type(type), m_str(str), m_hind(hind) {}
	};
	class ActionStack{
	private:
		QVector<Action> actions;
		int maxDepth;
	public:
		ActionStack(int depth = 20);

		enum UndoType {NORMAL, UNDO, REDO};

		//----- about stack ------
		void push(Action action);
		Action pop();
		void pop(Action & receiver);
		void clear();
		int length() const ;
		void setMaxDepth(int n);
	};

	//============== private variable =====================
	Node * firstNode;
	int stackDepth;
	ActionStack undoStack;
	ActionStack redoStack;

	//============== private methods ==============
	int nodeNum() const ;
	Node * addNode(Node * nodep, Heap * source = NULL);//add a node after nodep
	Heap * addHeap(Heap * heapp);//add a heap after heapp
	void delNode(Node * nodep);
	void delHeap(Heap * heapp);
	void mergeNextNode(Node * nodep);
	void mergeNextHeap(Heap * heapp);

	//======= private operator overload ========
	Node & operator[](int n);

public:
	explicit Data(QObject *parent = 0);
	~Data();

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

		//--------- for undo stack --------
		int parentNodeIndex() const ;//return -1 if error
		int parentHeapIndex() const ;

		//inline methods
		bool isOverFlow() const {return overflow;}
		Node * parentNode() const {return m_parentNode;}
		Heap * parentHeap() const {return m_parentHeap;}
		int index() const {return m_index;}
		void clear() {overflow = false;}

		//----- operator overload -----
		QChar operator*() const ;
		//QChar & operator*();
		iterator operator++();
		iterator operator++(int);
		iterator operator--();
		iterator operator--(int);
		iterator operator+(int n) const ;
		iterator operator-(int n) const ;
		bool operator==(const iterator & another) const ;
		bool operator!=(const iterator & another) const ;
		int operator-(const iterator & another) const ;
		iterator operator=(const iterator & another);

		//------ convert to bool -------
		operator bool() const {return !overflow;}
	};
	//=========== codeStyle ==============
	int getCodeStyle()const;
	void setCodeStyle(int style);

	//=========== about iterator ===========
	iterator begin();
	iterator end();
	iterator iteratorAt(int parentNodeIndex, int indexInNode);
	iterator iteratorAt(int parentNodeIndex, int parentHeapIndex, int indexInHeap);

	//========== about text edit ========
	iterator add(const iterator & locate, const QString & str, ActionStack::UndoType undo = ActionStack::UndoType::NORMAL, bool hind = false);
	iterator del(const iterator & startLocate, const iterator & endLocate, bool hind = false, ActionStack::UndoType undo = ActionStack::UndoType::NORMAL);
	iterator edit(const iterator & startLocate, const iterator & endLocate, const QString & str);
	iterator find(const iterator & startLocate, const QString & str);
	iterator cut(const iterator & startLocate, const iterator & endLocate);
	iterator copy(const iterator & startLocate, const iterator & endLocate);
	iterator paste(const Data::iterator & startLocate, const Data::iterator &endLocate) ;//get string from system clipboard
	iterator undo(const iterator & now);
	iterator redo(const iterator & now);
	void clear();
	bool isEmpty();
	QString text(const iterator & startLocate, const iterator & endLocate);

	//=========== about undoStack & redoStack ============
	bool undoStackEmpty() const {return undoStack.length() == 0;}
	bool redoStackEmpty() const {return redoStack.length() == 0;}
	void resetStackSize(int n);
	void clearStack();

	//========= about file ===========
	void save(const QString & pathAndName);
	void read(const QString & pathAndName);
signals:
	void WindowUdate();
	void dataChanged();
public slots:
};

#endif // DATA_H
