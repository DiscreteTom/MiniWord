#include "data.h"
#include <QMessageBox>

#include <QDebug>

Data::Data(QObject *parent) : QObject(parent)
{

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
	//if (str[i] == '\t'){
	// //turn to blank
	// int n = TABWIDTH - this->operator-(parentNode->begin()) % TABWIDTH;
	// add(locate, QString(' ', 10);
	//}
	//todo
}

Data::iterator Data::del(const Data::iterator & startLocate, const Data::iterator & endLocate, bool hind)
{
	//todo
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

Data::iterator Data::iterator::operator++()
{
	++m_index;
	if (m_index == m_parentHeap->charNum - 1){//not int this heap
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

int Data::Node::charNum()
{
	int result = 0;
	for (int i = 0; i < heapNum; ++i){
		result += this->operator[](i).charNum;
	}
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
		while (n > heap->charNum - 1){
			n -= heap->charNum;
			heap = heap->nextHeap;
		}
		return *heap;
	}
}

QChar Data::Heap::operator[](int n)
{
	if (n > charNum - 1){//not in this heap
		if (nextHeap){//goto next heap
			return (*nextHeap)[n - charNum];
		} else {//no next heap
			QMessageBox::warning(NULL, tr("Error"), tr("Try to find overflow char from Heap"));
		}
	} else {//in this heap
		return ch[n];
	}
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
