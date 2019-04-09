#include "data.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QClipboard>
#include <QTextCodec>
#include <QDebug>

//! add a node after "nodep", with "source"(see Data::Node)
//! "nodep" can not be NULL
//! "source" can be NULL
int Data::nodeNum() const
{
	auto p = firstNode;
	int result = 0;
	while (p){
		p = p->nextNode;
		++result;
	}
	return result;
}

Data::Node *Data::addNode(Data::Node * nodep, Heap *source)
{
	if (!nodep) return NULL;//error

	//get new node
	auto newNode = new Node(nodep->parent, source);
	//link node
	newNode->nextNode = nodep->nextNode;
	newNode->preNode = nodep;
	if (nodep->nextNode){
		nodep->nextNode->preNode = newNode;
	}
	nodep->nextNode = newNode;

	return newNode;
}

//! add a heap after "heapp"
//! "heapp" can not be NULL
Data::Heap *Data::addHeap(Data::Heap *heapp)
{
	if (!heapp) return NULL;//error

	//get heap
	auto newHeap = new Heap(heapp->parentNode);
	//link heap
	newHeap->nextHeap = heapp->nextHeap;
	newHeap->preHeap = heapp;
	if (heapp->nextHeap){
		heapp->nextHeap->preHeap = newHeap;
	}
	heapp->nextHeap = newHeap;

	return newHeap;
}

//! delete "nodep" and link other node
//! won't delete the only node, but will delete its chars
void Data::delNode(Data::Node *nodep)
{
	if (!nodep) return;//error

	if (!nodep->preNode){//the node is the head node
		if (!nodep->nextNode){//the node is the only node and now you want to delete it
			qDebug() << "Data::delNode::try to delete the only node";
			//delete its chars
			while (nodep->firstHeap->nextHeap){//delete all heaps except the first one
				delHeap(nodep->firstHeap->nextHeap);
			}
			if (!nodep->firstHeap){//some node may have no heap in some other process
				nodep->firstHeap = new Heap(nodep);
			}
			nodep->firstHeap->ch[0] = '\n';
			nodep->firstHeap->charNum = 1;//'\n'
			return;
		} else {//not the only node and is the first node
			//change first node
			firstNode = nodep->nextNode;
			//cancel link
			firstNode->preNode = NULL;
			nodep->nextNode = nodep->preNode = NULL;
			//nodep will be delete at the end of this function
		}
	} else{//the node is not the first node
		//link node
		nodep->preNode->nextNode = nodep->nextNode;
		if (nodep->nextNode){
			nodep->nextNode->preNode = nodep->preNode;
		}
		//cancel link
		nodep->nextNode = nodep->preNode = NULL;
	}

	delete nodep;
	return;
}

//! delete "heapp" and link other heap
//! if "heapp" is the only heap of a node, delete this node
void Data::delHeap(Data::Heap *heapp)
{
	if (!heapp) return;//error

	if (!heapp->preHeap){//the heap is the first heap of a node
		if (!heapp->nextHeap){//the heap is the only heap of this node
			delNode(heapp->parentNode);
			return;
		} else {//nextHeap exist
			heapp->parentNode->firstHeap = heapp->nextHeap;
			heapp->nextHeap->preHeap = NULL;
			heapp->preHeap = heapp->nextHeap = NULL;
		}
	} else {//the heap is not the first heap
		heapp->preHeap->nextHeap = heapp->nextHeap;
		if (heapp->nextHeap){
			heapp->nextHeap->preHeap = heapp->preHeap;
		}
		heapp->preHeap = heapp->nextHeap = NULL;
	}
	delete heapp;
	return;
}

//! try to merge "nodep" and "nodep->nextNode"
//! if merge successful, will try to merge middle heap
void Data::mergeNextNode(Data::Node *nodep)
{
	if (!nodep || !nodep->nextNode) return;//error

	Heap * wasLastHeap = nodep->lastHeap();
	//remove \n, some node may in some other process so it doesn't has '\n'
	if (wasLastHeap->ch[wasLastHeap->charNum - 1] == '\n')
		--wasLastHeap->charNum;

	//Here DO NOT judge whether this heap is empty
	//in the end of this funciton mergeNextHeap will do the job

	//change parent and heap num
	auto p = nodep->nextNode->firstHeap;
	while (p){
		p->parentNode = nodep;//change parent
		p = p->nextHeap;
	}

	//link heap
	nodep->nextNode->firstHeap->preHeap = wasLastHeap;
	wasLastHeap->nextHeap = nodep->nextNode->firstHeap;
	nodep->nextNode->firstHeap = NULL;
	mergeNextHeap(wasLastHeap);
	//link node and delete node
	delNode(nodep->nextNode);
}

//! try to merge "heapp" and "heapp->nextHeap"
void Data::mergeNextHeap(Data::Heap *heapp)
{
	if (!heapp || !heapp->nextHeap) return;//error

	if (heapp->charNum + heapp->nextHeap->charNum <= 100){//can merge
		//move char
		for (int i = 0; i < heapp->nextHeap->charNum; ++i){
			heapp->ch[heapp->charNum] = heapp->nextHeap->ch[i];
			++heapp->charNum;
		}
		heapp->nextHeap->charNum = 0;
		//link heap
		delHeap(heapp->nextHeap);
	}
}

//! can only be used in MainWindow
Data::Data(QObject *parent) : QObject(parent)
{
	firstNode = new Node(this);
	stackDepth = 20;
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

int Data::getCodeStyle() const
{
	return codeStyle;
}

void Data::setCodeStyle(int style)
{
	if(style >=0 && style < 3)
	{
		codeStyle = style;
	}else
	{
		codeStyle = 0;
	}
}

//! return the first node's first heap's first char's iterator
Data::iterator Data::begin()
{
	return iterator(firstNode, firstNode->firstHeap, 0);
}

//! return the last node's last heap's last char's iterator
Data::iterator Data::end()
{
	auto lastNode = firstNode;
	while (lastNode->nextNode){
		lastNode = lastNode->nextNode;
	}
	auto lastHeap = lastNode->lastHeap();
	return iterator(lastNode, lastHeap, lastHeap->charNum - 1);
}

//! return the "indexInNode" char in the node of "parentNodeIndex"
//! if wrong locate return overflow iterator
Data::iterator Data::iteratorAt(int parentNodeIndex, int indexInNode)
{
	//get node and judge overflow
	auto p_node = firstNode;
	while (parentNodeIndex && p_node){
		p_node = p_node->nextNode;
		--parentNodeIndex;
	}
	if (!p_node) return iterator();//overflow

	//get heap and judge overflow
	auto p_heap = p_node->firstHeap;
	while (indexInNode >= p_heap->charNum && p_heap->nextHeap){
		indexInNode -= p_heap->charNum;
		p_heap = p_heap->nextHeap;
	}
	if (indexInNode >= p_heap->charNum || indexInNode < 0) return iterator();//overflow

	return iterator(p_node, p_heap, indexInNode);
}

//! if wrong return overflow iterator
Data::iterator Data::iteratorAt(int parentNodeIndex, int parentHeapIndex, int indexInHeap)
{
	//get node
	auto p_node = firstNode;
	while (parentNodeIndex && p_node){
		p_node = p_node->nextNode;
		--parentNodeIndex;
	}
	if (!p_node) return iterator();//overflow

	//get heap
	auto p_heap = p_node->firstHeap;
	while (parentHeapIndex && p_heap){
		p_heap = p_heap->nextHeap;
		--parentHeapIndex;
	}
	if (!p_heap) return iterator();//overflow

	if (indexInHeap >= p_heap->charNum || indexInHeap < 0){//overflow
		return iterator();
	}
	return iterator(p_node, p_heap, indexInHeap);
}

//! return the node of index n in Data
Data::Node & Data::operator[](int n)
{
	if (n >= nodeNum() || n < 0){
		qDebug() << "Data::operator[]::index overflow";
		return *firstNode;
	} else {//n is legal
		auto p = firstNode;
		while (n){
			p = p->nextNode;
			--n;
		}
		return *p;
	}
}

//! add a string "str" at "locate"
Data::iterator Data::add(const Data::iterator & locate, const QString & str, ActionStack::UndoType undo, bool hind)
{
	//! deal with '\n' in this function
	if (!str.length()) return locate;

	if (undo == ActionStack::UndoType::UNDO){
		redoStack.push(Action(locate.parentNodeIndex(), locate.parentHeapIndex(), locate.index(), Action::DEL, str, hind));
	} else {
		undoStack.push(Action(locate.parentNodeIndex(), locate.parentHeapIndex(), locate.index(), Action::ADD, str, hind));
		if (undo == ActionStack::UndoType::NORMAL) redoStack.clear();
	}

	iterator result = locate;

	if (str.length() == 1){
		//! single char
		if (str[0] != '\n'){
			result = locate.parentHeap()->add(str, locate.index());
		} else {//'\n'
			locate.parentHeap()->moveToNewNode(locate.index());
			result = locate + 1;//locate won't overflow, this step is safe
		}
		if (hind && result - 1){
			--result;
		}
	} else {
		//! a string
		Node * currentNode = locate.parentNode();
		Heap * currentHeap = locate.parentHeap();
		int currentIndex = locate.index();
		QStringList strList = str.split('\n');
		for (int i = 0; i < strList.length(); ++i){
			if (strList[i].length()){
				result = currentHeap->add(strList[i], currentIndex);
			} else {
				result = iterator(currentNode, currentHeap, currentIndex);
			}
			if (i != strList.length() - 1){//add \n
				result.parentHeap()->moveToNewNode(result.index());
				currentNode = result.parentNode()->nextNode;
				currentHeap = currentNode->firstHeap;
				currentIndex = 0;
			}
		}
	}

	emit WindowUdate();
	emit dataChanged();
	return result;
}

//! delete chars from "startLocate" to "endLocate"
//! if "startLocate" == "endLocate" and "hind" == true, delete char at "startLocate", otherwise delete "startLocate" - 1
Data::iterator Data::del(const Data::iterator & startLocate, const Data::iterator & endLocate, bool hind, ActionStack::UndoType undo)
{
	if (startLocate == endLocate){
		//! just delete one char
		iterator result;
		iterator aim = startLocate;
		if (!hind){
			//delete pre char
			if (aim - 1){
				//pre char exist
				--aim;
			} else {
				//no pre char
				return aim;
			}
		}
		//! now delete aim
		auto aimChar = *aim;
		if (*aim == '\n'){
			if (aim - 1){
				result = aim - 1;
				mergeNextNode(aim.parentNode());
				++result;
			} else {
				//aim is the first char and is '\n'
				mergeNextNode(aim.parentNode());
				result = begin();
			}

			emit dataChanged();
		} else if (aim - 1){//not '\n' and pre char exist
			result = aim - 1;
			aim.parentHeap()->del(aim.index());
			mergeNextHeap(aim.parentHeap());
			++result;

			emit dataChanged();
		} else if (aim + 1){//not '\n' and next char exist
			result = aim + 1;
			aim.parentHeap()->del(aim.index());
			mergeNextHeap(aim.parentHeap());
			--result;

			emit dataChanged();
		} else {//this char is the only char, the '\n'
			result = aim;
		}

		if (undo == ActionStack::UndoType::UNDO){
			redoStack.push(Action(result.parentNodeIndex(), result.parentHeapIndex(), result.index(), Action::ADD, aimChar, hind));
		} else {
			undoStack.push(Action(result.parentNodeIndex(), result.parentHeapIndex(), result.index(), Action::DEL, aimChar, hind));
			if (undo == ActionStack::UndoType::NORMAL) redoStack.clear();
		}

		emit WindowUdate();
		return result;
	} else {
		//! not just delete one char
		//judge which one is front
		iterator frontLocate, hindLocate;
		if (startLocate - endLocate > 0){
			frontLocate = startLocate;
			hindLocate = endLocate;
		} else {
			frontLocate = endLocate;
			hindLocate = startLocate;
		}

		//remember result string
		QString resultString = text(frontLocate, hindLocate);

		//if no pre char, later result = begin()
		auto result = frontLocate;
		bool flag = (frontLocate - 1);
		if (flag){
			--result;
		}

		//move chars
		hindLocate.parentHeap()->moveToNextHeap(hindLocate.index());
		//cut
		frontLocate.parentHeap()->charNum = frontLocate.index();
		if (hindLocate.parentHeap() == frontLocate.parentHeap()){//in the same heap, unlikely to delete \n
			mergeNextHeap(frontLocate.parentHeap());//must be successful
		} else if (frontLocate.parentNode() == hindLocate.parentNode()){//in the same node, unlikely to delete \n
			while (frontLocate.parentHeap()->nextHeap != hindLocate.parentHeap()){
				//delete middle heap
				delHeap(frontLocate.parentHeap()->nextHeap);
			}
			//delete hindLocate's parentHeap as well because *hindLocate has been moved to next heap
			delHeap(frontLocate.parentHeap()->nextHeap);
			mergeNextHeap(frontLocate.parentHeap());
		} else {//in different node
			while (frontLocate.parentNode()->nextNode != hindLocate.parentNode()){
				//delete middle node
				delNode(frontLocate.parentNode()->nextNode);
			}
			while (hindLocate.parentNode()->firstHeap != hindLocate.parentHeap()){
				//delete middle heap before hindLocate
				delHeap(hindLocate.parentNode()->firstHeap);
			}
			while (frontLocate.parentHeap()->nextHeap){
				//delete middle heap after frontLocate
				delHeap(frontLocate.parentHeap()->nextHeap);
			}
			//delete hindLocate's parentHeap as well because *hindLocate has been moved to next heap
			delHeap(hindLocate.parentHeap());
			mergeNextNode(frontLocate.parentNode());
		}

		emit WindowUdate();
		emit dataChanged();
		if (flag){
			++result;
		} else {
			result = begin();
		}
		if (undo == ActionStack::UndoType::UNDO){
			redoStack.push(Action(result.parentNodeIndex(), result.parentHeapIndex(), result.index(), Action::ADD, resultString));
		} else {
			undoStack.push(Action(result.parentNodeIndex(), result.parentHeapIndex(), result.index(), Action::DEL, resultString));
			if (undo == ActionStack::UndoType::NORMAL) redoStack.clear();
		}
		return result;
	}
}

//! delete from "startLocate" to "endLocate" and add "str"
Data::iterator Data::edit(const Data::iterator & startLocate, const Data::iterator & endLocate, const QString & str)
{
	auto t = del(startLocate, endLocate);
	t = add(t, str);
	return t;
}

//! use KMP
Data::iterator Data::find(const Data::iterator & startLocate, const QString & str)
{
	//if can't find return startLocate and give QMessageBox::warning
	int len = str.length();
	int*next = new int[len];
	int j=0,k=-1;
	next[0] = 0;
	while(j<len)
		if((k==-1)||(str[j]==str[k])){
			j++;
			k++;
			next[j]=k+1;
		}
		else k=next[k]-1;
	for(int i = 1;i<len;i++){
		if(str[i] == str[next[i]])next[i] = next[next[i]];
	}
	auto PosData = startLocate;
	int PosStr = 0;
	while(PosData&&PosStr!=len){
		if((*PosData) == str[PosStr]){
			PosData++;
			PosStr++;
		}else if(next[PosStr] != 0){
			PosStr = next[PosStr] - 1;
		}else{
			PosData++;
			PosStr = 0;
		}
	}
	delete[] next;
	if(PosData)return (PosData - str.length());

	return iterator();
}

//! cut from "startLocate" to "endLocate"
Data::iterator Data::cut(const Data::iterator & startLocate, const Data::iterator & endLocate)
{
	QApplication::clipboard()->setText(text(startLocate, endLocate));
	return del(startLocate,endLocate);
}

//! copy from "startLocate" to "endLocate"
//! return "endLocate"
Data::iterator Data::copy(const Data::iterator & startLocate, const Data::iterator & endLocate)
{
	QApplication::clipboard()->setText(text(startLocate, endLocate));
	return endLocate;
}

//! paste cover startLocate to endLocate
Data::iterator Data::paste(const Data::iterator & startLocate, const Data::iterator &endLocate)
{
	if (startLocate == endLocate)
		return add(startLocate,QApplication::clipboard()->text());
	else
		return edit(startLocate, endLocate, QApplication::clipboard()->text());
}

//! if undo stack is empty return now
Data::iterator Data::undo(const iterator &now)
{
	if (undoStack.length()){
		Action a = undoStack.pop();
		if (a.m_type == Action::ADD){
			//need del
			iterator startLocate = iteratorAt(a.nodeIndex, a.heapIndex, a.indexInHeap);
			if (a.m_hind){
				iterator result = del(startLocate, startLocate, a.m_hind, ActionStack::UndoType::UNDO);
				return result;
			} else {
				iterator result = del(startLocate, startLocate + a.m_str.length(), a.m_hind, ActionStack::UndoType::UNDO);
				return result;
			}
		} else if (a.m_type == Action::DEL){
			//need add
			iterator result = add(iteratorAt(a.nodeIndex, a.heapIndex, a.indexInHeap), a.m_str, ActionStack::UndoType::UNDO, a.m_hind);
			return result;
		}
	} else {//undo stack is empty
		return now;
	}
	return now;
}

//! if redo stack is empty return now
Data::iterator Data::redo(const Data::iterator &now)
{
	if (redoStack.length()){
		Action a = redoStack.pop();
		if (a.m_type == Action::DEL){
			//need del
			iterator startLocate = iteratorAt(a.nodeIndex, a.heapIndex, a.indexInHeap);
			if (a.m_hind){
				iterator result = del(startLocate, startLocate, a.m_hind, ActionStack::UndoType::REDO);
				return result;
			} else {
				iterator result = del(startLocate, startLocate + a.m_str.length(), a.m_hind, ActionStack::UndoType::REDO);
				return result;
			}
		} else if (a.m_type == Action::ADD){
			//need add
			return add(iteratorAt(a.nodeIndex, a.heapIndex, a.indexInHeap), a.m_str, ActionStack::UndoType::REDO, a.m_hind);
		}
	} else {//redo stack is empty
		return now;
	}
	return now;
}

//! delete all data and undoStack and redoStack
void Data::clear()
{
	while (!isEmpty()) delNode(firstNode);
	clearStack();
	emit WindowUdate();
	emit dataChanged();
}

//! if there is only one node and one heap and one char and the char is '\n' return true
bool Data::isEmpty()
{
	if (nodeNum() == 1 && firstNode->heapNum() == 1 && firstNode->firstHeap->charNum == 1) return true;
	else return false;
}

//! return string from startLocate to endLocate
QString Data::text(const Data::iterator &startLocate, const Data::iterator &endLocate)
{
	//just one char
	if (startLocate == endLocate){
		return *startLocate;
	}

	//judge which iterator is ahead
	iterator frontLocate, hindLocate;
	if (startLocate - endLocate >= 0){
		frontLocate = startLocate;
		hindLocate = endLocate;
	} else {
		frontLocate = endLocate;
		hindLocate = startLocate;
	}

	QString str;
	while (frontLocate != hindLocate){
		str.append(*frontLocate);
		++frontLocate;
	}
	return str;
}

//! reset undoStack and redoStack size and clear them
void Data::resetStackSize(int n)
{
	if (n <= 0){
		n = 20;
	}
	stackDepth = n;
	undoStack.setMaxDepth(n);//will clear the stack
	redoStack.setMaxDepth(n);//will clear the stack
}

//! clear undoStack and redoStack
void Data::clearStack()
{
	undoStack.clear();
	redoStack.clear();
}

//! save all data to file
void Data::save(const QString &pathAndName)
{
	QFile file(pathAndName);
	if (!file.open(QFile::WriteOnly | QFile::Text)){
		qDebug() << "Data::save::file open fail";
		return;
	}

	QTextStream out(&file);
	switch(codeStyle)
	{
	case 0:
		break;
	case 1:
		out.setCodec(QTextCodec::codecForName("UTF-8"));
		break;
	default:
		break;
	}
	auto i = begin();
	auto e = end();
	i.clear();
	while (i){
		if (i == e){//ignore the last \n
			break;
		}
		out << *i;
		++i;
	}

	file.close();
}

//! open a file and get all data
void Data::read(const QString &pathAndName)
{
	QFile file(pathAndName);
	if (!file.open(QFile::ReadOnly | QFile::Text)){
		qDebug() << "Data::read::file open fail";
		return;
	}

	QTextStream in(&file);
	switch(codeStyle)
	{
	case 0:
		break;
	case 1:
		in.setCodec(QTextCodec::codecForName("UTF-8"));
		break;
	default:
		break;
	}
	QString buf;
	while (!in.atEnd()){
		buf = in.readAll();
		add(end(), buf);
	}

	file.close();
}

//! get the index of iterator's parentNode
int Data::iterator::parentNodeIndex() const
{
	if (this->isOverFlow()) return -1;

	int result = 0;
	Node * p = parentNode();
	while (p->preNode){
		++result;
		p = p->preNode;
	}
	return result;
}

//! get the index of iterator's parentHeap
int Data::iterator::parentHeapIndex() const
{
	if (this->isOverFlow()) return -1;//error
	int result = 0;
	Heap * p = parentHeap();
	while (p->preHeap){
		++result;
		p = p->preHeap;
	}
	return result;
}

//! get the char the iterator point to
QChar Data::iterator::operator*() const
{
	return m_parentHeap->ch[m_index];
}

//! iterator move to next place
//! if no next place, return overflow
Data::iterator Data::iterator::operator++()
{
	if (overflow) return *this;
	++m_index;
	if (m_index >= m_parentHeap->charNum){//not int this heap
		if (m_parentHeap->nextHeap){//next heap exists
			//goto next heap
			m_parentHeap = m_parentHeap->nextHeap;
			m_index = 0;
		} else {//no next heap
			if (m_parentNode->nextNode){//next node exists
				//go next node
				m_parentNode = m_parentNode->nextNode;
				m_parentHeap = m_parentNode->firstHeap;
				m_index = 0;
			} else {//no next node
				//qDebug() << "Data::iterator::operator++::overflow";
				overflow = true;
				--m_index;
			}
		}
	}
	return *this;
}

//! nearly the same as the previous function
Data::iterator Data::iterator::operator++(int)
{
	auto t = *this;
	this->operator ++();
	return t;
}

//! iterator move to the previous place
//! if no pre char, return overflow
Data::iterator Data::iterator::operator--()
{
	if (overflow) return *this;
	--m_index;
	if (m_index < 0){//not in this heap
		if (m_parentHeap->preHeap){//pre heap exists
			//goto previous heap
			m_parentHeap = m_parentHeap->preHeap;
			m_index = m_parentHeap->charNum - 1;
		} else {//no previous heap
			if (m_parentNode->preNode){//pre node exists
				//goto previous node
				m_parentNode = m_parentNode->preNode;
				m_parentHeap = m_parentNode->lastHeap();
				m_index = m_parentHeap->charNum - 1;
			} else {//no previous node
				//qDebug() << "Data::iterator::operator--::overflow";
				overflow = true;
				++m_index;
			}
		}
	}
	return *this;
}

//! nearly the same as the previous function
Data::iterator Data::iterator::operator--(int)
{
	auto t = *this;
	this->operator --();
	return t;
}

//! return a new iterator equals to this iterator move to next place "n" times
//! "n" can be minus
Data::iterator Data::iterator::operator+(int n) const
{
	if (n < 0){
		return *this - (-n);
	}
	auto t = *this;
	while (n){
		++t;
		--n;
	}
	return t;
}

//! return a new iterator equals to this iterator move to pre place "n" times
//! "n" can be minus
Data::iterator Data::iterator::operator-(int n) const
{
	if (n < 0){
		return *this + (-n);
	}

	auto t = *this;
	while (n){
		--t;
		--n;
	}
	return t;
}

//! copy value
Data::iterator Data::iterator::operator=(const Data::iterator & another)
{
	m_parentNode = another.m_parentNode;
	m_parentHeap = another.m_parentHeap;
	m_index = another.m_index;
	overflow = another.overflow;
	return *this;
}

//! judge whether two iterator are at the same place
//! ignore overflow
bool Data::iterator::operator==(const Data::iterator & another) const
{
	//if (overflow || another.overflow) return false;
	return m_parentNode == another.m_parentNode && m_parentHeap == another.m_parentHeap && m_index == another.m_index;
}

//! judge whether two iterator are at different place
//! ignore overflow
bool Data::iterator::operator!=(const Data::iterator &another) const
{
	return !(*this == another);
}

//! return char count between two iterator
//! include '\n'
//! if "another" is ahead of "*this", return a minus value
int Data::iterator::operator-(const Data::iterator & another) const
{
	int result = 0;
	auto t = *this;

	while (t && another != t){
		++result;
		++t;
	}
	if (t.isOverFlow()) result = -1;
	return result;
}

//! just get memory, not link node
//! if "source" is not NULL, get node base on "source" instead of get memory
Data::Node::Node(Data *m_parent, Heap *source)
{
	preNode = NULL;
	nextNode = NULL;
	parent = m_parent;

	if (source){
		firstHeap = source;
		source->preHeap = NULL;
		//change parent and heapnum
		source->parentNode = this;
		while (source->nextHeap){
			source = source->nextHeap;
			source->parentNode = this;
		}
	} else {
		firstHeap = new Heap(this);
		firstHeap->ch[0] = '\n';
		firstHeap->charNum = 1;
	}
}

//! just delete all heaps
//! won't link other nodes
//! if want to link other nodes, use delNode()
Data::Node::~Node()
{
	//just free memory of heaps
	auto p = firstHeap;
	while (p){
		auto q = p;
		p = p->nextHeap;
		delete q;
	}
}

int Data::Node::heapNum() const
{
	auto p = firstHeap;
	int result = 0;
	while (p){
		p = p->nextHeap;
		++result;
	}
	return result;
}

//! return char count in the whole node
int Data::Node::charNum() const
{
	int result = 0;
	auto p = firstHeap;
	while (p){
		result += p->charNum;
		p = p->nextHeap;
	}
	return result;
}

//! return the last heap of a node
Data::Heap *Data::Node::lastHeap()
{
	auto p = firstHeap;
	while (p->nextHeap){
		p = p->nextHeap;
	}
	return p;
}

//! return an iterator point to the first char in this node
Data::iterator Data::Node::begin()
{
	return iterator(this, firstHeap, 0);
}

//! return the n-th heap in a node
//! "n" must be valid
Data::Heap & Data::Node::operator[](int n)
{
	if (n > heapNum() - 1){//not in this node
		qDebug() << "Data::Node::operator[]::heap index overflow";
		return *firstHeap;
	} else {//in this node
		auto heap = firstHeap;
		while (n){
			--n;
			heap = heap->nextHeap;
		}
		return *heap;
	}
}

//! get a new heap, not link other heap
Data::Heap::Heap(Node *parent)
{
	charNum = 0;
	preHeap = NULL;
	nextHeap = NULL;
	parentNode = parent;
}

//! move chars from "start" in this heap to next heap
void Data::Heap::moveToNextHeap(int start)
{
	if (start < 0 || start >= charNum) return;//error
	if (!nextHeap || 100 - nextHeap->charNum < charNum - start){
		//no nextHeap or next heap is not big enough
		parentNode->parent->addHeap(this);//get a new heap
	}

	//next heap move right
	for (int i = nextHeap->charNum - 1; i >= 0; --i){
		nextHeap->ch[i + charNum - start] = nextHeap->ch[i];
	}
	//load
	for (int i = start; i < charNum; ++i){
		nextHeap->ch[i - start] = ch[i];
		++nextHeap->charNum;
	}
	charNum = start;
}

//! move chars from "start" in this heap and next heaps to a new node
void Data::Heap::moveToNewNode(int start)
{
	if (start < 0 || start >= charNum) return;//error

	//move to new node
	moveToNextHeap(start);
	parentNode->parent->addNode(parentNode, nextHeap);

	//cut end
	nextHeap = NULL;
	charNum = start + 1;
	ch[charNum - 1] = '\n';
}

//! return the n-th char in this heap
//!"n" must be valid, so the funcion is not safe enough
QChar Data::Heap::operator[](int n) const
{
	if (n > charNum - 1){//not in this heap
		qDebug() << "Data::Heap::operator[]::index overflow";
		return '0';
	} else {//in this heap
		return ch[n];
	}
}

//! return the reference of the n-th char in this heap
//! "n" must be valid
QChar & Data::Heap::operator[](int n){
	if (n > charNum - 1){//not in this heap
		qDebug() << "Data::Heap::operator[]::index overflow";
		return ch[0];
	} else {//in this heap
		return ch[n];
	}
}

//! add "str" from "index"
//! this function can not deal with '\n', str CAN NOT include '\n'
//! index must be valid
Data::iterator Data::Heap::add(const QString & str, int index)
{
	if (index >= 100 || index < 0){
		qDebug() << "Data::Heap::add::index >= 100 or index < 0";
		return iterator();
	}

	//! a single char
	if (str.length() == 1){
		if (charNum >= 100){
			if (!nextHeap || nextHeap->charNum >99){//get a new heap
				//get a new heap to store the tail char
				parentNode->parent->addHeap(this);
			} else {//next heap just move
				for (int i = nextHeap->charNum - 1; i >= 0; --i){
					nextHeap->ch[i + 1] = nextHeap->ch[i];
				}
			}//move the tail char to next heap
			nextHeap->ch[0] = ch[99];
			++nextHeap->charNum;
			--charNum;
		}
		//now this heap fits
		for (int i = charNum - 1; i >= index; --i){//move right
			ch[i + 1] = ch[i];
		}
		ch[index] = str[0];
		++charNum;

		return iterator(parentNode, this, index) + 1;
	}

	//! a string
	if (charNum + str.length() <= 100) {//this heap fits
		for (int i = charNum - 1; i >= index; --i){//move right
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
		return iterator(parentNode, currentHeap, index - 1) + 1;
	}
}

//! return an iterator point to the first char of this heap
Data::iterator Data::Heap::begin()
{
	return iterator(parentNode, this, 0);
}

//! should be called only by Data::del
//! just delete one char, other operation will be in Data::del
//! ignore '\n' and blank Heap, deal with them in Data::del
void Data::Heap::del(int index)
{
	if (index >= charNum || index < 0) return;//error

	for (int i = index; i < charNum - 1; ++i){
		ch[i] = ch[i + 1];
	}
	--charNum;
}

//int charWidth(QChar ch)
//{
//	if (ch == '\n'){
//		return 0;
//	} else if (ch.unicode() >= 0x4e00 && ch.unicode() <= 0x9fa5){
//		//chinese char
//		return 2;
//	} else {
//		return 1;
//	}
//}

//! init action stack
Data::ActionStack::ActionStack(int depth)
{
	maxDepth = depth;
}

//! push a new action
//! if stack is full, delete the earliest one
void Data::ActionStack::push(Data::Action action)
{
	actions.push_back(action);
	while (actions.length() > maxDepth){
		actions.erase(actions.begin());
	}
}

//! pop an action if this stack is not empty
//! should judge whether this stack is empty before calling this function
Data::Action Data::ActionStack::pop()
{
	if (actions.length()){
		auto result = *(actions.end() - 1);
		actions.erase(actions.end() - 1);
		return result;
	} else {
		qDebug() << "Data::ActionStack::pop::overflow";
		return Action();
	}
}

//! pop an action if this stack is not empty
//! should judge whether this stack is empty before calling this function
void Data::ActionStack::pop(Data::Action &receiver)
{
	if (actions.length()){
		receiver = *(actions.end() - 1);
		actions.erase(actions.end() - 1);
	} else {
		qDebug() << "Data::ActionStack::pop::overflow";
		receiver = Action();
	}
}

//! delete all actions in an actionStack
void Data::ActionStack::clear()
{
	actions.clear();
}

//! get the length of an actionStack
int Data::ActionStack::length() const
{
	return actions.length();
}

//! reset ActionStack's maxDepth by "n"
//! if "n" is invalid, "n" will be reset to 20
//! clear stack
void Data::ActionStack::setMaxDepth(int n)
{
	if (n <= 0) n = 20;
	maxDepth = n;
	clear();
}
