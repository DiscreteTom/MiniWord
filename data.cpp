#include "data.h"
#include <QMessageBox>

#include <QDebug>

//! should be the only way to add node and increase nodeNum
Data::Node *Data::addNode(Data::Node * nodep, Heap *source)
{
	auto newNode = new Node(nodep->parent, source);
	newNode->nextNode = nodep->nextNode;
	newNode->preNode = nodep;
	if (nodep->nextNode){
		nodep->nextNode->preNode = newNode;
	}
	nodep->nextNode = newNode;

	++nodeNum;
	return newNode;
}

Data::Heap *Data::addHeap(Data::Heap *heapp)
{
	auto newHeap = new Heap(heapp->parentNode);
	newHeap->nextHeap = heapp->nextHeap;
	newHeap->preHeap = heapp;
	if (heapp->nextHeap){
		heapp->nextHeap->preHeap = newHeap;
	}
	heapp->nextHeap = newHeap;
	newHeap->parentNode = heapp->parentNode;

	++heapp->parentNode->heapNum;
	return newHeap;
}

// created by wangjinghui, not needed
/* void Data::delNode(Data::Node *nodep)
{
	if (!nodep) return;
	if (!nodep->preNode){//the node is the head node
		if (!nodep->nextNode){//the node is the only node and now you want to delete it
			//throw error
		} else {
			--nodeNum;
			firstNode = nodep->nextNode;
			firstNode->preNode = NULL;
		}
	} else{//the node is the last node or middle node
		nodep->preNode->nextNode = nodep->nextNode;
		if (nodep->nextNode){
			nodep->nextNode->preNode = nodep->preNode;
		}
		--nodeNum;
	}

	delete nodep;
}

// created by wangjinghui, not needed
bool Data::delHeap(Data::Heap *heapp)
{
	if (!heapp) return false;
	if (!heapp->preHeap){//the heap is the first heap of a node
		if (!heapp->nextHeap){//the heap is the only heap of this node
			return true;
		} else {//nextHeap exist
			heapp->parentNode->firstHeap = heapp->nextHeap;
			--heapp->parentNode->heapNum;
			heapp->nextHeap->preHeap = NULL;
		}
	} else {//the heap is not the first heap
		heapp->preHeap->nextHeap = heapp->nextHeap;
		if (heapp->nextHeap){
			heapp->nextHeap->preHeap = heapp->preHeap;
		}
		--heapp->parentNode->heapNum;
	}
	delete heapp;
	return false;
} */

Data::Data(QObject *parent) : QObject(parent)
{
	firstNode = new Node(this);
	nodeNum = 1;
}

Data::~Data()
{
	//delete all node
	auto p = firstNode;
	while (p){
		auto q = p;
		p = p->nextNode;
		delete q;
	}
}

Data::iterator Data::begin()
{
	return iteratorAt(0, 0);
}

Data::iterator Data::iteratorAt(int parentNodeIndex, int indexInNode)
{
	//judge overflow
	if (parentNodeIndex > nodeNum - 1 || parentNodeIndex < 0){
		//QMessageBox::warning(NULL, tr("Error"), tr("iteratorAt overflow 1"));
		return iterator();
	}
	//get node
	auto p_node = firstNode;
	while (parentNodeIndex){
		p_node = p_node->nextNode;
		--parentNodeIndex;
	}
	//judge overflow
	if (indexInNode > p_node->charNum() - 1 || indexInNode < 0){
		//QMessageBox::warning(NULL, tr("Error"), tr("iteratorAt overflow 2"));
		return iterator();
	}
	auto p_heap = p_node->firstHeap;
	while (indexInNode > p_heap->charNum - 1 && p_heap->nextHeap){
		indexInNode -= p_heap->charNum;
		p_heap = p_heap->nextHeap;
	}
	if (indexInNode > p_heap->charNum || indexInNode < 0){//overflow
		return iterator();
	}
	return iterator(p_node, p_heap, indexInNode);
}

Data::iterator Data::iteratorAt(int parentNodeIndex, int parentHeapIndex, int indexInHeap)
{
	//judge overflow
	if (parentNodeIndex > nodeNum - 1 || parentNodeIndex < 0){
		//QMessageBox::warning(NULL, tr("Error"), tr("iteratorAt overflow 1"));
		return iterator();
	}
	//get node
	auto p_node = firstNode;
	while (parentNodeIndex){
		p_node = p_node->nextNode;
		--parentNodeIndex;
	}
	//judge overflow
	if (parentHeapIndex > p_node->heapNum - 1 || parentHeapIndex < 0){
		//QMessageBox::warning(NULL, tr("Error"), tr("iteratorAt overflow 3"));
		return iterator();
	}
	//get heap
	auto p_heap = p_node->firstHeap;
	while (parentHeapIndex > p_heap->charNum - 1){
		parentHeapIndex -= p_heap->charNum;
		p_heap = p_heap->nextHeap;
	}
	//final check
	if (indexInHeap > p_heap->charNum - 1 || indexInHeap < 0){
		//QMessageBox::warning(NULL, tr("Error"), tr("iteratorAt overflow 2"));
		return iterator();
	}

	return iterator(p_node, p_heap, indexInHeap);
}

const Data::Node & Data::operator[](int n)
{
	if (n > nodeNum - 1 || n < 0){
		//QMessageBox::warning(this->parent(), tr("Error"), tr("Try to get a non-existent Node"));
	} else {//n is legal
		auto p = firstNode;
		while (n){
			p = p->nextNode;
			--n;
		}
		return *p;
	}
}

Data::iterator Data::add(const Data::iterator & locate, const QString & str)
{
	//! deal with '\n' in this function

	//todo
	//undoStack.push(Action(locate, Action::DEL, str));

	iterator result;

	//single char
	if (str.length() == 1){
		if (str[0] != '\n') result = locate.parentHeap()->add(str, locate.index());
		else {//'\n'
			result = locate.parentHeap()->moveToNewNode(locate.index());
		}
	} else {
		//a string
		Node * currentNode = locate.parentNode();
		Heap * currentHeap = locate.parentHeap();
		int currentIndex = locate.index();
	//	int startIndex = -1;
	//	int endIndex;
		QStringList strList = str.split('\n');
		for (int i = 0; i < strList.length(); ++i){
			result = currentHeap->add(strList[i], currentIndex);
			if (i != strList.length() - 1){
				currentNode = addNode(currentNode);
				currentHeap = currentNode->firstHeap;
				currentIndex = 0;
			}
		}
	}

	emit WindowUdate();
	return result;
}

Data::iterator Data::del(const Data::iterator & startLocate, const Data::iterator & endLocate, bool hind)
{
	//todo

	//undoStack.push(Action(locate, Action::ADD, str));
	int len = 0;
	for(auto i = startLocate;!(i == endLocate)&&!i.isOverFlow();i++){
		len++;
	}
	for(auto i = startLocate;!i.isOverFlow();i++){
		i.parentHeap()->ch[i.index()]=i.parentHeap()->ch[i.index()+len];
	}
	startLocate.parentHeap()->charNum -= len;
	emit WindowUdate();
}

Data::iterator Data::edit(const Data::iterator & startLocate, const Data::iterator & endLocate, const QString & str)
{
	//todo
}

Data::iterator Data::find(const Data::iterator & startLocate, const QString & str)
{
	//if can't find return startLocate and give QMessageBox::warning
	//todo
	return startLocate;
}

Data::iterator Data::cut(const Data::iterator & startLocate, const Data::iterator & endLocate)
{
	//todo
}

Data::iterator Data::copy(const Data::iterator & startLocate, const Data::iterator & endLocate)
{
	//todo
}

Data::iterator Data::paste(const Data::iterator & locate)
{
	//todo
}

void Data::iterator::move(int unitWidthCount, int windowUnitCount)
{
	while (unitWidthCount){
		if (**this !='\n'){
			if (unitWidthCount < 0){//move left
				this->operator --();
				unitWidthCount += charWidth(**this);
				if (unitWidthCount == 1) unitWidthCount = 0;//chinese char
			} else if (unitWidthCount > 0){//move right
				unitWidthCount -= charWidth(**this);
				this->operator ++();
				if (unitWidthCount == -1) unitWidthCount = 0;//chinese char
			}
		} else {//*this == '\n'
				unitWidthCount += (windowUnitCount - parentNode()->widthUnitNum % windowUnitCount)
				    * (unitWidthCount > 0 ? -1 : 1);//neglect blank char
		}
	}
}

QChar Data::iterator::operator*() const
{
	return m_parentHeap->operator [](m_index);
}

QChar &Data::iterator::operator*()
{
	return m_parentHeap->operator [](m_index);
}

Data::iterator Data::iterator::operator++()
{
	++m_index;
	if (m_index >= m_parentHeap->charNum){//not int this heap
		if (m_parentHeap->nextHeap){//goto next heap
			m_parentHeap = m_parentHeap->nextHeap;
			m_index = 0;
		} else {//no next heap
			if (m_parentNode->nextNode){//go next node
				m_parentNode = m_parentNode->nextNode;
				m_parentHeap = m_parentNode->firstHeap;
				m_index = 0;
			} else {//no next node
				overflow = true;
				--m_index;
			}
		}
	}
	return *this;
}

Data::iterator Data::iterator::operator++(int)
{
	auto t = *this;
	this->operator ++();
	return t;
}

Data::iterator Data::iterator::operator--()
{
	--m_index;
	if (m_index < 0){//not in this heap
		if (m_parentHeap->preHeap){//goto previous heap
			m_parentHeap = m_parentHeap->preHeap;
			m_index = m_parentHeap->charNum - 1;
		} else {//no previous heap
			if (m_parentNode->preNode){//goto previous node
				m_parentNode = m_parentNode->preNode;
				m_parentHeap = m_parentNode->lastHeap();
				m_index = m_parentHeap->charNum - 1;
			} else {//no previous node
				overflow = true;
				++m_index;
			}
		}
	}
	return *this;
}

Data::iterator Data::iterator::operator--(int)
{
	auto t = *this;
	this->operator --();
	return t;
}

Data::iterator Data::iterator::operator+(int n) const
{
	auto t = *this;
	while (n){
		++t;
		--n;
	}
	return t;
}

Data::iterator Data::iterator::operator-(int n) const
{
	auto t = *this;
	while (n){
		++t;
		--n;
	}
	return t;
}

Data::iterator Data::iterator::operator=(const Data::iterator & another)
{
	m_parentNode = another.m_parentNode;
	m_parentHeap = another.m_parentHeap;
	m_index = another.m_index;
	overflow = another.overflow;
	return *this;
}

bool Data::iterator::operator==(const Data::iterator & another) const
{
	if (overflow || another.overflow) return false;
	return m_parentNode == another.m_parentNode && m_parentHeap == another.m_parentHeap && m_index == another.m_index;
}

int Data::iterator::operator-(const Data::iterator & another) const
{
	int result = 0;
	auto t = *this;
	while (!(t.isOverFlow() || another == *this)){
		if (**this == '\n') return -2;

		result += charWidth(*t);
		++t;
	}
	if (t.isOverFlow()) result = -1;
	return result;
}

Data::Node::Node(Data *m_parent, Heap *source)
{
	preNode = NULL;
	nextNode = NULL;
	heapNum = 1;
	widthUnitNum = 0;
	parent = m_parent;

	if (source){
		firstHeap = source;
		source->preHeap = NULL;
		source->parentNode = this;
		while (source->nextHeap){
			source = source->nextHeap;
			source->parentNode = this;
			++heapNum;
		}
	} else {
		firstHeap = new Heap(this);
		firstHeap->ch[0] = '\n';
		firstHeap->charNum = 1;
	}
}

Data::Node::~Node()
{
	//just free memory
	auto p = firstHeap;
	while (p){
		auto q = p;
		p = p->nextHeap;
		delete q;
	}
}

int Data::Node::charNum()
{
	int result = 0;
	for (int i = 0; i < heapNum; ++i){
		result += this->operator[](i).charNum;
	}
	return result;
}

Data::Heap *Data::Node::lastHeap()
{
	auto p = firstHeap;
	while (p->nextHeap){
		p = p->nextHeap;
	}
	return p;
}

Data::iterator Data::Node::begin()
{
	return iterator(this, firstHeap, 0);
}

// created by wangjinghui, not needed
/* Data::Heap * Data::Node::addHeap(Data::Heap *source, int index)
{
	//find the end heap of source
	auto lastHeap = source;
	while (lastHeap->nextHeap) lastHeap = lastHeap->nextHeap;

	if (index == -1){//source is the new head heap of the node
		lastHeap->nextHeap = firstHeap;
		lastHeap->nextHeap->preHeap = lastHeap;
		firstHeap = source;
		source->preHeap = NULL;
	} else {
		//link after startHeap
		auto startHeap = firstHeap;
		while (index){
			startHeap = startHeap->nextHeap;
		}
		lastHeap->nextHeap = startHeap->nextHeap;
		if (startHeap->nextHeap){
			startHeap->nextHeap->preHeap = lastHeap;
		}
		source->preHeap = startHeap;
		startHeap->nextHeap = source;
	}
} */

const Data::Heap & Data::Node::operator[](int n)
{
	if (n > heapNum - 1){//not in this node
		/* ===== if not in this node, we shouldn't goto next node
		if (nextp){//go to next node
			return (*nextp)[n - charNum()];
		} else {//no next node
			QMessageBox::warning(NULL, tr("Error"), tr("Try to find overflow char from Node"));
		}
		  =====*/
			QMessageBox::warning(NULL, tr("Error"), tr("Try to find overflow char from Node"));
	} else {//in this node
		/* ===== use a loop instead of resursion
		return (*firstHeap)[n];
		   =====*/
		auto heap = firstHeap;
		while (n){
			--n;
			heap = heap->nextHeap;
		}
		return *heap;
	}
}

Data::Heap::Heap(Node *parent)
{
	charNum = 0;
	preHeap = NULL;
	nextHeap = NULL;
	parentNode = parent;
}

void Data::Heap::moveToNextHeap(int start)
{
	if (start == charNum) return;//not move
	if (start < 0 || start > charNum - 1) return;//error
	if (!nextHeap || 100 - nextHeap->charNum < charNum - start){
		//no nextHeap or next heap is not big enough
		parentNode->parent->addHeap(this);
	}

	for (int i = start; i < charNum; ++i){
		nextHeap->add(ch[i]);
	}
	charNum = start;
}

Data::iterator Data::Heap::moveToNewNode(int start)
{
	if (start < 0 || start >= charNum) return iterator();//error
	//if (this == parentNode->firstHeap && start == 0) return iterator();//not move

	moveToNextHeap(start);
	parentNode->parent->addNode(parentNode, nextHeap);
	nextHeap = NULL;
	charNum = start + 1;
	ch[charNum - 1] = '\n';
	return parentNode->nextNode->begin();
}

// created by wangjinghui, not needed
/* void Data::Heap::move(int start, int offset)
{
	if (offset <= 0) return;
	if (charNum - start + offset > 100){//need to move to next heap
		moveToNextHeap(charNum + offset - 100 + start);
	}
} */

QChar &Data::Heap::operator[](int n)
{
	if (n > charNum - 1){//not in this heap
		if (nextHeap){//goto next heap
			return (*nextHeap)[n - charNum];
		} else {//no next heap
			//QMessageBox::warning(NULL, tr("Error"), tr("Try to find overflow char from Heap"));
			qDebug() << "try to use Heap[] to find an forbidden element";
		}
	} else {//in this heap
		return ch[n];
	}
}

QChar Data::Heap::operator[](int n) const
{
	if (n > charNum - 1){//not in this heap
		if (nextHeap){//goto next heap
			return (*nextHeap)[n - charNum];
		} else {//no next heap
			qDebug() << "try to use Heap[] to find an forbidden element";
		}
	} else {//in this heap
		return ch[n];
	}
}

Data::iterator Data::Heap::add(const QString & str, int index)
{
	//str CAN NOT include \n
	//if index == -1 then add to tail
	if (index >= 100){
		qDebug() << "Data::Heap::add::index>=100";
		return iterator();
	}
	if (index == -1) index = charNum;

	//a single char
	if (str.length() == 1){
		if (charNum >= 100){
			if (!nextHeap || nextHeap->charNum >99){//get a new heap
				//get a new heap to store the tail char
				parentNode->parent->addHeap(this);
				nextHeap->ch[0] = ch[99];
				++nextHeap->charNum;
			} else {//just move
				for (int i = nextHeap->charNum - 1; i >= 0; --i){
					nextHeap->ch[i + 1] = nextHeap->ch[i];
				}
				nextHeap->ch[0] = ch[99];
				++nextHeap->charNum;
			}
		}
		//now this heap fits
		for (int i = charNum - 1; i >= index; --i){
			if (i == 99) continue;
			ch[i + 1] = ch[i];
		}
		ch[index] = str[0];
		if (charNum < 100) ++charNum;

		if (index == 99)
			return iterator(parentNode, nextHeap, 0);
		else
			return iterator(parentNode, this, index + 1);
	}

	//a string
	if (charNum + str.length() <= 100) {//this heap fits
		for (int i = index; i < charNum; ++i){//move right
			ch[i + str.length()] = ch[i];
		}
		for (int i = 0; i < str.length(); ++i){//load string
			ch[index + i] = str[i];
		}
		charNum += str.length();
		return iterator(parentNode, this, index + str.length());
	} else {//this heap is not enough, need to move to next heap
		moveToNextHeap(index);
		//add to tail
		int n = 0;//index to str
		Heap * currentHeap = this;
		while (n < str.length()){
			if (index > 99){//currentHeap is full, get a new one
				currentHeap = parentNode->parent->addHeap(currentHeap);
				index = 0;
			}
			currentHeap->ch[index] = str[n];
			++currentHeap->charNum;
			++index;
			++n;
		}
		return iterator(parentNode, currentHeap, index + 1);
	}
}

Data::iterator Data::Heap::begin()
{
	return iterator(parentNode, this, 0);
}

int charWidth(QChar ch)
{
	if (ch == '\n'){
		return 0;
	} else if (ch.unicode() >= 0x4e00 && ch.unicode() <= 0x9fa5){
		//chinese char
		return 2;
	} else {
		return 1;
	}
}
