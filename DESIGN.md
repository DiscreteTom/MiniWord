# DESIGN

## 编程环境选择

选择QT的图形及UI库，使用Qt Creator进行代码书写与编译工作。

## 模块划分及其逻辑关系

程序分为**显示界面模块**与**内部数据模块**，其中：

**显示界面模块**用于向用户展示文本编辑区的文本，为用户展示提示信息，并提供输入接口，接受并处理用户的鼠标、键盘等操作，并向内部数据结构传递相应的修改或查找信息；

**内部数据模块**用于对用户正在处理的文本进行增、删、改、查、读取、保存、复制到剪贴板、撤销等功能。

## 内部数据模块设计方案

### 概述

内部数据结构由一个被声明为**Q_OBJECT**的C++类**Data**组成

Data类中包含了如下嵌套类

| 类名        | 访问控制 | 介绍                                                                                   |
| ----------- | -------- | -------------------------------------------------------------------------------------- |
| Heap        | private  | 内含100个字符，是内部数据结构申请内存的最小单位。若干个Heap在Node中组成双向链表        |
| Node        | private  | 用来表示数据中的一个段落。每个Node内含一个Heap双向链表。若干个Node在Data中组成双向链表 |
| Action      | private  | 用来描述撤销或重做操作                                                                 |
| ActionStack | private  | 用来存放撤销操作和重做操作的栈。有最大容量。                                           |
| iterator    | public   | 迭代器，用来线性访问内部数据中的字符                                                   |

主程序可通过如下接口访问内部数据结构或执行操作

```c++
//=========== about iterator ===========
iterator begin();//获取指向第一个字符的迭代器
iterator end();//获取指向最后一个字符的迭代器（注意：和一般迭代器不同，这里的end()返回的不是超尾元素而是尾元素
iterator iteratorAt(int parentNodeIndex, int indexInNode);//获取指向指定段落指定字符的迭代器
iterator iteratorAt(int parentNodeIndex, int parentHeapIndex, int indexInHeap);//获取指向指定段落指定堆指定字符的迭代器

//========== about text edit ========
iterator add(const iterator & locate, const QString & str, ActionStack::UndoType undo = ActionStack::UndoType::NORMAL);//在指定位置增加字符或字符串
iterator del(const iterator & startLocate, const iterator & endLocate, bool hind = false, ActionStack::UndoType undo = ActionStack::UndoType::NORMAL);//删除指定位置的字符或字符串
iterator edit(const iterator & startLocate, const iterator & endLocate, const QString & str);//把指定位置的字符串替换为另一个指定字符串
iterator find(const iterator & startLocate, const QString & str);//从某个起始位置开始查找某个字符串
iterator cut(const iterator & startLocate, const iterator & endLocate);//剪切某一位置的字符串到系统剪贴板
iterator copy(const iterator & startLocate, const iterator & endLocate);//复制某一位置的字符串到系统剪贴板
iterator paste(const Data::iterator & startLocate, const Data::iterator &endLocate);//在指定位置粘贴系统剪贴板中的字符串
iterator undo(const iterator & now);//撤销操作
iterator redo(const iterator & now);//重做操作
void clear();//清空整个数据结构
bool isEmpty();//判断内部数据是否为空
QString text(const iterator & startLocate, const iterator & endLocate);//返回数据结构中指定位置的字符串

//=========== about undoStack & redoStack ============
bool undoStackEmpty() const {return undoStack.length() == 0;}//判断撤销栈是否为空
bool redoStackEmpty() const {return redoStack.length() == 0;}//判断重做栈是否为空
void resetStackSize(int n);//设置撤销栈和重做栈的最大深度
void clearStack();//清空撤销栈和重做栈

//========= about file ===========
void save(const QString & pathAndName);//把数据结构中的数据保存到指定文件
void read(const QString & pathAndName);//从指定文件中读取数据到内部数据结构
```

### 堆(Heap)

- Data类的私有嵌套类，不可被主程序访问
- 每个堆的容量为100个字符(QChar)
- 是内部数据结构扩容的单位
- 数据结构为双向链表

```c++
QChar ch[100];//100个字符的空间
int charNum;//堆内实际字符数量
Heap * preHeap;//指向上一个堆的指针，双向链表的组成结构
Heap * nextHeap;//指向下一个堆的指针，双向链表的组成结构
Node * parentNode;//指向父Node的指针
```

由于Heap是Data的私有嵌套类，无法被外部访问，所以为了方便将所有数据和方法的访问控制为public

堆内字符向低索引靠拢，用charNum控制访问

Heap类包含以下public方法

| 函数头                                        | 函数功能与实现策略                                                                                                                             |
| --------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------- |
| Heap(Node * parent);                          | Heap构造函数。以parent为父Node指针新建一个堆。不建议直接使用此函数增加Heap，建议使用addHeap函数                                                |
| void moveToNextHeap(int start);               | 把本Heap中从起始位置(start)开始（包括start）到最后一个字符移动到父Node内的下一个Heap中。如果下一个Heap不存在或无法容纳则创建一个新的Heap并移动 |
| void moveToNewNode(int start);                | 把从本Heap的起始位置(start)开始（包括起始位置）到父Node的最后一个字符的字符串移动到一个新的Node中，新的Node接在本Heap的父Node后                |
| iterator add(const QString & str, int index); | 在本Heap的指定位置(index)插入指定字符串(str)，如果Heap无法容纳则在此Heap后添加新的Heap。此函数无法处理换行符。返回增加字符串后光标的位置       |
| iterator begin();                             | 返回指向此Heap第一个字符的迭代器                                                                                                               |
| void del(int index);                          | 删除此Heap第index个字符。只能删除一个字符。无法处理换行符。无法处理删除后可能出现的空Heap。这些交给Data::del处理                               |
| QChar operator[](int n) const ;               | 根据下标n返回此Heap中第n个字符。调用heap[n]相当于调用了heap.ch[n]。安全性不如heap.ch[n]。只能在n合法时返回正确字符                             |
| QChar & operator[](int n);                    | 和上面函数类似。返回指定字符的引用。无法用来增加或删除字符（因为增加或删除字符需要修改charNum），可以用来替换字符                              |

Data中包含以下private方法以管理Heap

| 函数头                            | 函数功能与实现策略                                                                                                                                       |
| --------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Heap \* addHeap(Heap \* heapp);   | 通过调用Heap构造函数在指定Heap(heapp)后添加一个Heap。此函数拥有链接前后Heap的功能，建议添加Heap时调用此函数而不是Heap的构造函数。返回指向新Heap的指针    |
| void delHeap(Heap * heapp);       | 删除指定Heap(heapp)。此函数可以在删除Heap后链接前后Heap，也可以在当前Node中不含Heap时通过调用delNode删除Node。建议在删除Heap时调用此函数而不是直接delete |
| void mergeNextHeap(Heap * heapp); | 试图合并当前Heap(heapp)和当前Heap的下一个Heap。如果下一个Heap不存在或当前Heap无法容纳两个Heap中的总字符量则不合并                                        |

### 段落(Node)

- Data类的私有嵌套类，不可被主程序访问
- 每个Node内包含一个Heap链表
- 表示Data中的一个段落，最后一个字符必为换行符
- 数据结构为双向链表

```c++
Node * preNode;//指向前一个Node，双向链表的组成部分
Node * nextNode;//指向下一个Node，双向链表的组成部分
Heap * firstHeap;//指向内部Heap链表的第一个Heap
Data * parent;//指向父Data
```

和Heap一样，Node是Data的私有嵌套类，无法被外部访问，所以为了方便将所有数据和方法的访问控制为public

| 函数头                                         | 函数功能与实现策略                                                                                                                                                                              |
| ---------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Node(Data \* m_parent, Heap \* source = NULL); | 构造函数。以m_parent为父Data新建一个Node。如果source不为空，则窃取source和其后同一Node内的Heap作为此Node的Heap链表。此函数无法在新建Node后链接前后Node。建议使用addNode函数添加Node而不是此函数 |
| ~Node();                                       | 析构函数。仅删除此Node下的所有Heap，不会链接前后Node。建议使用delNode删除Node而不是直接使用delete删除Node                                                                                       |
| int heapNum();                                 | 返回此Node中的Heap数                                                                                                                                                                            |
| int charNum();                                 | 返回此Node中所有字符总数                                                                                                                                                                        |
| Heap * lastHeap();                             | 返回此Node最后一个Heap的指针                                                                                                                                                                    |
| iterator begin();                              | 获取指向此Node第一个字符的迭代器                                                                                                                                                                |
| Heap & operator[](int n);                      | 获取此Node内指定下标的Heap的引用                                                                                                                                                                |

Data中包含以下private方法以管理Node

| 函数头                                              | 函数功能与实现策略                                                                                                                                          |
| --------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Node * addNode(Node * nodep, Heap * source = NULL); | 通过调用Node的构造函数在指定Node(nodep)后根据source添加一个新的Node。此函数可以链接前后Node，建议使用此函数添加Node而不是使用Node构造函数。返回新Node的指针 |
| void delNode(Node * nodep);                         | 删除指定Node(nodep)，此函数可以链接前后Node，并且会判断此Node是否为Data中的唯一Node。建议使用此函数删除Node而不是直接delete                                 |
| void mergeNextNode(Node * nodep);                   | 尝试合并当前Node(nodep)和下一个Node。如果下一个Node不存在则不合并。合并时会自动删除掉当前Node的换行符。此函数应该在删除换行符时被调用                       |
| Node & operator[](int n);                           | 根据下标返回指定Node的引用                                                                                                                                  |

### Action & ActionStack

撤销和重做通过**撤销重做栈**实现。撤销重做栈是ActionStack。栈内元素为Action.

和Heap/Node一样，Action为Data私有成员，为了方便将所有成员设为public

Action类声明了以下public枚举常量用来表明当前Action的种类（增加或删除）

```c++
enum Type{ADD, DEL};
```

Action类包含以下public成员变量

```c++
int nodeIndex;//此操作在第几个Node进行的
int heapIndex;//此操作在第几个Heap进行的
int indexInHeap;//此操作在当前Heap的什么位置进行的
Type m_type;//此操作是增加操作还是删除操作
QString m_str;//此操作改变的字符串
```

由于更改过程涉及到Node或Heap的删除，所以定位Action时不像迭代器那样使用指针而是使用下标索引来定位

Action类的方法只有一个，即构造函数

```c++
Action(int locateNode = -1, int locateHeap = -1, int index = -1, Type type = ADD, QString str = "")
	:nodeIndex(locateNode), heapIndex(locateHeap), indexInHeap(index), m_type(type), m_str(str) {}
```

可以把Action视为一个struct

ActionStack类是**拥有最大容量的栈**，所以不能使用容器Stack。此处使用了Qt中的容器QVector来存储Action。

ActionStack拥有以下私有成员变量

```c++
QVector<Action> actions;//Action容器
int maxDepth;//最大深度，即最大撤销重做次数
```

ActionStack类拥有以下public方法

| 函数头                       | 函数功能与实现策略                                                   |
| ---------------------------- | -------------------------------------------------------------------- |
| ActionStack(int depth = 20); | 构造函数，生成一个空栈。默认最大深度20                               |
| void push(Action action);    | 栈的push操作                                                         |
| Action pop();                | 栈的pop操作                                                          |
| void pop(Action & receiver); | 通过参数返回值的pop                                                  |
| void clear();                | 清空栈                                                               |
| int length() const ;         | 返回当前栈内元素数量                                                 |
| void setMaxDepth(int n);     | 设置栈深为n。如果n非正整数则赋值n为20。此函数会自动调用clear()清空栈 |

ActionStack还提供了如下public枚举常量用来给Data类的接口定义撤销类型

```c++
enum UndoType {NORMAL, UNDO, REDO};
```

在Data中含有一些接口以供主程序控制撤销重做栈。详见下文*接口函数与相关实现策略*

### 迭代器(iterator)

为了提高主程序访问内部数据结构的效率，我们设计了迭代器来避免每次从数据头部检索数据

迭代器和指针一样，但实现了跨Node和跨Heap移动。主要用来使主程序逐字符访问内部数据、作为接口函数的参数与返回值以定位光标、跨Heap实现查找时的KMP算法

迭代器是Data中的public嵌套类。为了避免主程序访问数据，所有数据被设计为private

以下是迭代器的private变量

```c++
Node * m_parentNode;//迭代器指向的字符的父Node指针
Heap * m_parentHeap;//迭代器指向的字符的父Heap指针
int m_index;//迭代器指向的字符在父Heap中的索引位置
bool overflow;//是否溢出
```

迭代器使用指针和索引定位字符以实现随机访问。如果迭代器在移动时移动到了可用字符外则被视为溢出

迭代器的一切方法都是public的

下面是迭代器的方法与实现策略

| 函数头                                                    | 函数功能与实现策略                                                                                                                                                                           |
| --------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| iterator();                                               | 默认构造函数，返回一个**溢出迭代器**。用来生成异常。如果不是出于返回异常的目的，永远不要使用默认构造函数                                                                                     |
| iterator(Node * parentNode, Heap * parentHeap, int index) | 构造函数，根据参数值定位一个迭代器。因为Node和Heap是Data的私有嵌套类所以此函数无法被外部主函数使用。此构造函数**没有溢出检测**，得到的迭代器一定是非溢出的（即使它不指向字符）。使用时要小心 |
| int parentNodeIndex() const ;                             | 为撤销重做栈设计的函数。返回迭代器父Node在Data中的索引                                                                                                                                       |
| int parentHeapIndex() const ;                             | 为撤销重做栈设计的函数。返回迭代器父Heap在Data中的索引                                                                                                                                       |
| bool isOverFlow() const ;                                 | getter函数，返回溢出状态                                                                                                                                                                     |
| Node * parentNode() const ;                               | getter函数，返回父Node                                                                                                                                                                       |
| Heap * parentHeap() const ;                               | getter函数，返回父Heap                                                                                                                                                                       |
| int index() const ;                                       | getter函数，返回迭代器在父Heap中的索引                                                                                                                                                       |
| void clear();                                             | 清除一个迭代器的溢出状态。不建议使用                                                                                                                                                         |
| QChar operator*() const ;                                 | 获取迭代器指向的字符                                                                                                                                                                         |
| iterator operator++();                                    | 迭代器向后移动一个字符。可以跨Node和跨Heap移动。如果当前迭代器已经溢出则不移动。如果迭代器移动后溢出则撤销移动并将迭代器置为溢出状态。此时如果配合clear()函数可以获得移动前的非溢出迭代器    |
| iterator operator++(int);                                 | 后置++运算符                                                                                                                                                                                 |
| iterator operator--();                                    | 迭代器向前移动一个字符。可以跨Node和跨Heap移动。如果当前迭代器已经溢出则不移动。如果迭代器移动后溢出则撤销移动并将迭代器置为溢出状态。此时如果配合clear()函数可以获得移动前的非溢出迭代器    |
| iterator operator--(int);                                 | 后置--运算符                                                                                                                                                                                 |
| iterator operator+(int n) const ;                         | 向前移动n个字符。有溢出检测                                                                                                                                                                  |
| iterator operator-(int n) const ;                         | 向后移动n个字符。有溢出检测                                                                                                                                                                  |
| bool operator==(const iterator & another) const ;         | 判断两个迭代器是否相等，忽略溢出状态，只根据位置判断                                                                                                                                         |
| bool operator!=(const iterator & another) const ;         | 判断两个迭代器是否不相等。策略和operator==相同                                                                                                                                               |
| int operator-(const iterator & another) const ;           | 判断两个迭代器之间的字符数量。如果参数迭代器在此迭代器之后则返回一个正数。如果参数迭代器在此迭代器之前则返回-1，如果两个迭代器相等则返回0                                                    |
| iterator operator=(const iterator & another);             | 赋值，包括溢出状态                                                                                                                                                                           |
| operator bool() const ;                                   | 直接把迭代器转换为bool值。反映了迭代器是否*非溢出*，即迭代器是否是好的迭代器                                                                                                                 |

对Data外部的主程序来说，迭代器只能够根据位置访问字符，不能够用来修改内部数据。修改内部数据要使用下文提到的接口函数。

外部主程序**永远不要**通过构造函数获得迭代器，因为默认构造函数只能得到溢出迭代器，非默认构造函数的参数都是Data私有嵌套类而无法被外部使用。主程序应通过后文接口函数中的迭代器相关函数来获得迭代器。

对内部数据来说，迭代器保存的数据是指针。在修改内部数据时很可能把指针指向的内容篡改而得到野指针。所以内部函数的编写应尽量减少迭代器的使用，应仅使用数据结构相关内容。仅仅把迭代器当做和外部主函数的接口。

### Data类私有成员与实现策略

Data作为整个内部数据结构的宏观类，包含以下私有成员变量

```c++
Node * firstNode;//Node链表的第一个Node的指针
int stackDepth;//撤销重做栈的深度
ActionStack undoStack;//撤销栈
ActionStack redoStack;//重做栈
```

除了nodeNum()方法，其他方法都已在上文中描述过

| 函数头                                              | 函数功能与实现策略                                                                                                                                          |
| --------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------- |
| int nodeNum() const ;                               | 获得Node链表长度                                                                                                                                            |
| Heap \* addHeap(Heap \* heapp);                     | 通过调用Heap构造函数在指定Heap(heapp)后添加一个Heap。此函数拥有链接前后Heap的功能，建议添加Heap时调用此函数而不是Heap的构造函数。返回指向新Heap的指针       |
| void delHeap(Heap * heapp);                         | 删除指定Heap(heapp)。此函数可以在删除Heap后链接前后Heap，也可以在当前Node中不含Heap时通过调用delNode删除Node。建议在删除Heap时调用此函数而不是直接delete    |
| void mergeNextHeap(Heap * heapp);                   | 试图合并当前Heap(heapp)和当前Heap的下一个Heap。如果下一个Heap不存在或当前Heap无法容纳两个Heap中的总字符量则不合并                                           |
| Node * addNode(Node * nodep, Heap * source = NULL); | 通过调用Node的构造函数在指定Node(nodep)后根据source添加一个新的Node。此函数可以链接前后Node，建议使用此函数添加Node而不是使用Node构造函数。返回新Node的指针 |
| void delNode(Node * nodep);                         | 删除指定Node(nodep)，此函数可以链接前后Node，并且会判断此Node是否为Data中的唯一Node。建议使用此函数删除Node而不是直接delete                                 |
| void mergeNextNode(Node * nodep);                   | 尝试合并当前Node(nodep)和下一个Node。如果下一个Node不存在则不合并。合并时会自动删除掉当前Node的换行符。此函数应该在删除换行符时被调用                       |
| Node & operator[](int n);                           | 根据下标返回指定Node的引用                                                                                                                                  |

### Data的其他设计

显示界面光标显示的是当前指向字符的左侧。为了保证光标可以移动到最后一个字符之后，所有Node的最后一个字符必须是换行符，这样光标指向此换行符时光标的显示位置就是实际最后一个字符的左侧。

按照这个设计，所谓的"空Node"指的是只含一个换行符的Node，Data在构造时必须有一个Node，这个Node只有一个换行符。

### 接口函数与相关实现策略

#### 构造函数与析构函数

有且只有一个构造函数

```c++
explicit Data(QObject *parent = 0);
```

构造函数设置了默认撤销重做栈的深度为20，并使用Node的构造函数赋值给firstNode

```c++
//! can only be used in MainWindow
Data::Data(QObject *parent) : QObject(parent)
{
	firstNode = new Node(this);
	stackDepth = 20;
}
```

析构函数负责删除所有Node。因为不需要链接前后Node，所以使用delete而不是delNode是更快速的选择

```c++
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
```

#### 迭代器相关

| 函数头                                                                          | 函数功能与实现策略                                                 |
| ------------------------------------------------------------------------------- | ------------------------------------------------------------------ |
| iterator begin();                                                               | 获得指向整个内部数据第一个字符的迭代器                             |
| iterator end();                                                                 | 获得指向整个内部数据最后一个字符的迭代器（注意不是指向超尾元素的） |
| iterator iteratorAt(int parentNodeIndex, int indexInNode);                      | 获得指定段落指定位置的迭代器                                       |
| iterator iteratorAt(int parentNodeIndex, int parentHeapIndex, int indexInHeap); | 获得指定段落指定Heap指定位置的迭代器                               |

#### 文本编辑相关

| 函数头                                                                                                                                                 | 函数功能与实现策略                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| ------------------------------------------------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| iterator add(const iterator & locate, const QString & str, ActionStack::UndoType undo = ActionStack::UndoType::NORMAL);                                | 在指定位置(locate)处添加字符串(str)。如果字符串为空串则直接返回locate。如果字符串为单个字符且为换行符，调用moveToNewNode函数将指定位置后面的字符移动到新的段落。如果字符串为单个字符且不为换行符，调用Heap::add函数把这个字符交给Heap处理。如果字符串长度大于1，调用string类的split函数将字符串根据换行符分裂为多个字符串，将每个不含换行符的字符串交给Heap::add处理，把换行符调用moveToNewNode处理。最后根据undo的状态对撤销重做栈进行压栈或弹栈                                                                                                                                                                                                                                    |
| iterator del(const iterator & startLocate, const iterator & endLocate, bool hind = false, ActionStack::UndoType undo = ActionStack::UndoType::NORMAL); | 删除指定位置（从startLocate到endLocate）的字符串。如果startLocate等于endLocate，只删除单个字符，根据hind判断删除前一个字符（用户按下backspace）还是当前字符（用户按下delete），如果删除的是换行符则直接调用mergeNextNode函数，否则调用Heap::del删除单个字符。如果startLocate不等于endLocate，即删除一个长度大于1的字符串，先判断startLocate和endLocate的位置关系，得到靠前的迭代器frontLocate和靠后的迭代器hindLocate，把hindLocate后面的字符通过moveToNextHeap移动到下一个Heap，删除frontLocate和hindLocate间的所有Heap和Node，因为hindLocate指向的字符已经移动到了下一个Heap中，所以也要删掉hindLocate所在的Heap，最后合并即可。最后的最后根据undo的状态对撤销重做栈进行压栈或弹栈 |
| iterator edit(const iterator & startLocate, const iterator & endLocate, const QString & str);                                                          | 先调用del删除字符串，再在删除后的位置调用add添加字符串                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
| iterator find(const iterator & startLocate, const QString & str);                                                                                      | 使用KMP算法从指定位置开始查找指定字符串                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| iterator cut(const iterator & startLocate, const iterator & endLocate);                                                                                | 把指定区域的字符串删除并置于系统剪贴板                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
| iterator copy(const iterator & startLocate, const iterator & endLocate);                                                                               | 把指定区域的字符串置于系统剪贴板                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| iterator paste(const Data::iterator & startLocate, const Data::iterator &endLocate) ;                                                                  | 把系统剪贴板的字符串add到指定位置                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| iterator undo(const iterator & now);                                                                                                                   | 撤销。如果撤销栈为空，返回now                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| iterator redo(const iterator & now);                                                                                                                   | 重做。如果重做栈为空，返回now                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| void clear();                                                                                                                                          | 清空Node和撤销重做栈内数据                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           |
| bool isEmpty();                                                                                                                                        | 判断Data是否为空。如果Data只有一个Node且这个Node只有一个换行符则判定为空                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| QString text(const iterator & startLocate, const iterator & endLocate);                                                                                | 获得从startLocate到endLocate的字符串。startLocate与endLocate的位置关系在内部会进一步确定                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |

#### 撤销重做栈相关

| 函数头                        | 函数功能与实现策略                     |
| ----------------------------- | -------------------------------------- |
| bool undoStackEmpty() const ; | 判断撤销栈是否为空                     |
| bool redoStackEmpty() const ; | 判断重做栈是否为空                     |
| void resetStackSize(int n);   | 重新设置撤销重做栈的深度，会清空两个栈 |
| void clearStack();            | 清空两个栈                             |

#### 文件相关

| 函数头                                  | 函数功能与实现策略                                                     |
| --------------------------------------- | ---------------------------------------------------------------------- |
| void save(const QString & pathAndName); | 把Data中的文本保存在外部文件中。会自动去掉Data末尾的设计上添加的换行符 |
| void read(const QString & pathAndName); | 清空当前数据和撤销重做栈后读取指定文件中的数据。会自动在文末加上换行符 |

#### 信号(Qt)

```c++
void WindowUdate();//迭代器移动需要刷新窗口时发送此信号
void dataChanged();//数据发生改变需要保存时发送此信号
```

## 显示界面模块设计方案

选用以QT的QMainWindow为基类的自定义类MainWindow作为显示界面窗口。

### MainWindow内部结构概述

MainWindow包含如下内部结构

- 光标结构体Pos
- 查找替换对话框ReplaceDlg(继承 QWidget类)
- 设置框SettingDlg(继承 QWidget类)
- 字符统计框CharNumDlg(继承 QWidget类)

### 光标结构体Pos

```c++
struct Pos{
  int ShowPosX; //储存光标的显示位置X
  int ShowPosY; //储存光标的显示位置Y
  Data::iterator DataPos; //储存光标在数据结构中的位置
};
```

### 查找替换对话框ReplaceDlg(继承 QWidget类)

#### 一般方法

```cpp
explicit ReplaceDlg(QWidget *parent = 0); //构造函数
~ReplaceDlg(); //析构函数
QString findLeText() const ; //获取查找框里的字符串
QString replaceLeText() const ; //获取替换框里的字符串
```
#### 信号

```cpp
signals:
void FindNext(); //"查找下一个"信号
void Replace(); //"替换"信号
void ReplaceAll(); //"全部替换"信号
```

#### 槽

```cpp
private slots:
void on_findNextBtn_clicked(); //"查找下一个"按钮点击时，进入此函数
void on_replaceBtn_clicked(); //"替换"按钮点击时，进入此函数
void on_replaceAllBtn_clicked(); //"全部替换"按钮点击时，进入此函数
void on_cancelBtn_clicked(); //"取消"按钮点击时，进入此函数
```

### 设置框SettingDlg(继承 QWidget类)

#### 一般方法

```cpp
explicit SettingDlg(QWidget *parent = 0); //构造函数
~SettingDlg(); //析构函数
void setMaxUndoTime(int n); //设置最大撤销次数，参数为设置值
void setFontSize(int n); //设置字体大小，参数为设置值
void setTabSize(int n); //设置Tab宽度，参数为设置值
void setTabStyle(int n); //设置Tab样式，参数为设置值
void setSpaceStyle(int n); //设置空格样式，参数为设置值
int maxUndoTime() const ; //返回最大撤销次数
int defaultFontSize() const ; //返回字体大小
int tabSize() const ; //返回Tab宽度
int tabStyle() const ; //返回空格样式
int spaceStyle() const ; //返回空格样式
```

### 字符统计框CharNumDlg(继承 QWidget类)

#### 成员

```cpp
int AllNum; //全部字符数量
int ChineseCharNum; //中文字符数量
int EnglishCharNum; //英文字符数量
int ParaNum; //段落数
int NumberNum; //数字字符数量
int OtherNum; //其它字符数量
```

#### 一般方法

```cpp
explicit CharNumDlg(QWidget *parent = NULL);//构造函数
void SetCharNum(int allnum, int Chinesenum, int Englishnum, int paranum, int numbernum, int othernum); //设置字符数量
void paintEvent(QPaintEvent*ev); //界面绘图事件，用于显示字符数量
void keyPressEvent(QKeyEvent *ev); //键盘输入相应事件，用于接受Enter键关闭对话框
```

### MainWindow成员变量

#### 标题相关

| 变量名     | 变量类型    | 说明                           |
| ---------- | ----------- | ------------------------------ |
| isUntitled | bool        | 判断文件是否为新建的无标题文件 |
| shouldSave | bool        | 判断文件是否需要保存           |
| curFile    | Qt::QString | 储存正在编辑的文件的文件名     |

#### 数据相关

| 变量名 | 变量类型       | 说明                       |
| ------ | -------------- | -------------------------- |
| data   | Data(自定义类) | 储存正在编辑的文件的字符串 |

#### 绘图相关

| 变量名         | 变量类型      | 说明                                                            |
| -------------- | ------------- | --------------------------------------------------------------- |
| PosLeftUp      | Pos(自定义类) | 文本光标，所指位置为当前显示区域需要显示的第一个字符            |
| PosCur         | Pos(自定义类) | 文本光标，所指位置为当前正在编辑的字符位置(与PosPre共同工作)    |
| PosPre         | Pos(自定义类) | 文本光标，所指位置为当前正在编辑的字符位置(与PosCur共同工作)    |
| DataTextHeight | int           | 储存文本总高度                                                  |
| DataTextTop    | int           | 储存位于显示的第一行上方的文本行数                              |
| DataParaTop    | int           | 储存位于显示的第一行上方的文本短段落数                          |
| TextBoxHeight  | int           | 储存文本框高度                                                  |
| TextBoxWidth   | int           | 储存文本框宽度                                                  |
| FontSizeW      | int           | 储存字体宽度                                                    |
| FontSizeH      | int           | 储存字体高度                                                    |
| SpaceStyle     | int           | 储存空格样式                                                    |
| TabWidth       | int           | 储存Tab宽度                                                     |
| TabStyle       | int           | 储存Tab样式                                                     |
| LineShowFlag   | int           | 储存行号显示策略                                                |
| CursorTimer    | int           | 光标闪烁计时器，范围0-1，为0时表示不绘制光标，为1时表示绘制光标 |
| MyCursorTimer  | QTimer        | 光标闪烁定时器，每700毫秒触发一次界面重绘，用于实现光标闪烁     |

#### 用户输入相关

| 变量名         | 变量类型 | 说明                                   |
| -------------- | -------- | -------------------------------------- |
| IsDragged      | bool     | 判断当前用户是否正在拖动鼠标           |
| InputPreStr    | QString  | 储存当前处于输入法活跃窗口中的字符     |
| InputPreStrPos | int      | 储存当前处于输入法活跃窗口中的光标位置 |

#### 滚动条相关

| 变量名      | 变量类型     | 说明                     |
| ----------- | ------------ | ------------------------ |
| MyScrollBar | QScrollBar\* | 一个仅可拖动的滚动条控件 |

#### 刷新策略相关

| 变量名                 | 变量类型 | 说明                                                                                               |
| ---------------------- | -------- | -------------------------------------------------------------------------------------------------- |
| ProtectedUpdateTimer   | int      | 保护式刷新计时器，取值为0-1，每次绘图后该值置为0，为0时禁止重绘，用于防止绘图过快导致的高CPU开销。 |
| MyProtectedUpdateTimer | QTimer   | 保护式刷新定时器每三十毫秒将ProtectedUpdateTimer重新置为1                                          |

#### UI界面相关

| 变量名          | 变量类型         | 说明                                           |
| --------------- | ---------------- | ---------------------------------------------- |
| ui              | Ui::MainWindow\* | MainWindow的UI界面成员，用于显示文件顶部导航栏 |
| rightMenu       | QMenu\*          | 右键菜单                                       |
| undoAction      | QAction\*        | 撤销按键                                       |
| redoAction      | QAction\*        | 重做按键                                       |
| cutAction       | QAction\*        | 剪贴按键                                       |
| copyAction      | QAction\*        | 复制按键                                       |
| pasteAction     | QAction\*        | 粘贴按键                                       |
| delAction       | QAction\*        | 删除按键                                       |
| seleteAllAction | QAction\*        | 全选按键                                       |

#### 对话框

| 变量名      | 变量类型                 | 说明       |
| ----------- | ------------------------ | ---------- |
| replaceDlg  | ReplaceDlg\*（自定义类） | 查找替换框 |
| settingsDlg | SettingDlg\*（自定义类） | 设置框     |
| charnumDlg  | CharNumDlg\*（自定义类） | 字符统计框 |

### MianWindow成员方法

#### 一般方法

| 方法名                           | 返回值类型 | 说明               |
| -------------------------------- | ---------- | ------------------ |
| MainWindow(QWidget \*parent = 0) | -          | MainWindow构造函数 |
| \~MainWindow()                   | -          | MainWindow析构函数 |

#### 绘图

| 方法名                                                | 返回值类型 | 说明                                                         |
| ----------------------------------------------------- | ---------- | ------------------------------------------------------------ |
| paintEvent(QPaintEvent \* ev)                         | void       | 重载的QWidget的绘图事件函数，用于文本的绘制                  |
| FillBlueArea(Pos &pos1, Pos &pos2, QPainter\*painter) | void       | 根据pos1和pos2的位置将其之间的区域涂成蓝色，用于显示鼠标选区 |

#### 绘图相关的变量控制

| 方法名                                              | 返回值类型 | 说明                                                                                                                 |
| --------------------------------------------------- | ---------- | -------------------------------------------------------------------------------------------------------------------- |
| LocateCursor(int x,int y)                           | void       | 通过鼠标点击的位置x，y将光标PosCur或PosPre定位在屏幕和数据结构的相应位置                                             |
| LocateLeftUpCursor(int newDataTextTop,int flag = 0) | void       | 将左上角光标位置置于正确的行，newDataTextTop为需要转到的行的值，flag为是否需要从头开始定位（如窗口大小改变时的情况） |
| Rolling(int flag)                                   | void       | 实现显示框的上下滚动，flag为0表示向上滚页，为1表示向下滚页                                                           |
| FindCursor()                                        | void       | 用于通过翻页的方式使光标位于显示框内                                                                                 |
| GetDataHeight()                                     | void       | 获取文本高度并刷新光标显示位置                                                                                       |
| ChangeFontSize(int Size)                            | void       | 改变字体大小为Size                                                                                                   |

#### UI

| 方法名           | 返回值类型 | 说明           |
| ---------------- | ---------- | -------------- |
| initRightMenu()  | void       | 初始化右键菜单 |
| resetRightMenu() | void       | 重置右键菜单   |

#### 文件处理

| 方法名                         | 返回值类型 | 说明                                                                                     |
| ------------------------------ | ---------- | ---------------------------------------------------------------------------------------- |
| newFile()                      | void       | 处理新建文件事件                                                                         |
| save()                         | bool       | 处理保存事件，返回值为是否成功                                                           |
| saveAs()                       | bool       | 处理另存为事件，返回值为是否成功                                                         |
| saveFile(const QString & path) | bool       | 保存文件，返回值为是否成功                                                               |
| maybeSave()                    | bool       | 处理用户在未保存文档的情况下新建、打开文件、退出程序的事件，返回值为用户是否未点击"取消" |

#### 输入事件处理

| 方法名                                    | 返回值类型 | 说明                                       |
| ----------------------------------------- | ---------- | ------------------------------------------ |
| closeEvent(QCloseEvent \* event)          | void       | 处理用户关闭程序事件                       |
| keyPressEvent(QKeyEvent \* ev)            | void       | 处理用户键盘输入事件                       |
| inputMethodEvent(QInputMethodEvent \* ev) | void       | 处理用户输入法输入事件                     |
| mousePressEvent(QMouseEvent \* ev)        | void       | 处理用户鼠标按下事件                       |
| mouseReleaseEvent(QMouseEvent \* ev)      | void       | 处理用户鼠标释放事件                       |
| mouseDoubleClickEvent(QMouseEvent \* ev)  | void       | 处理用户鼠标双击事件                       |
| mouseMoveEvent(QMouseEvent \* ev)         | void       | 处理用户鼠标移动事件                       |
| wheelEvent(QWheelEvent \*ev)              | void       | 处理用户鼠标滚轮事件                       |
| dragEnterEvent(QDragEnterEvent\*ev)       | void       | 处理用户拖入文件或字符串事件（拖入）       |
| dragMoveEvent(QDragMoveEvent\*ev)         | void       | 处理用户拖入文件或字符串事件（拖入并移动） |
| dropEvent(QDropEvent\*ev)                 | void       | 处理用户拖入文件或字符串事件（拖入并释放） |

#### 槽

| 槽名                                 | 说明                                                                             |
| ------------------------------------ | -------------------------------------------------------------------------------- |
| on\_action\_New\_triggered()         | "新建"按下转入该槽，用于处理相关事件                                             |
| on\_action\_Open\_triggered()        | "打开"按下转入该槽，用于处理相关事件                                             |
| on\_action\_Save\_triggered()        | "保存"按下转入该槽，用于处理相关事件                                             |
| on\_action\_SaveAs\_triggered()      | "另存为"按下转入该槽，用于处理相关事件                                           |
| on\_action\_Exit\_triggered()        | "退出"按下转入该槽，用于退出程序事件                                             |
| on\_action\_Undo\_triggered()        | "撤销"按下转入该槽，用于撤销事件                                                 |
| on\_action\_Redo\_triggered()        | "重做"按下转入该槽，用于重做事件                                                 |
| on\_action\_Cut\_triggered()         | "剪切"按下转入该槽，用于剪切事件                                                 |
| on\_action\_Copy\_triggered()        | "复制"按下转入该槽，用于复制事件                                                 |
| on\_action\_Paste\_triggered()       | "粘贴"按下转入该槽，用于粘贴事件                                                 |
| on\_action\_Delete\_triggered()      | "删除"按下转入该槽，用于处理删除事件                                             |
| on\_action\_Find\_triggered()        | "查找"按下转入该槽，用于开启查找替换框                                           |
| on\_action\_FindNext\_triggered()    | "查找下一个"按下转入该槽，实现查找下一个                                         |
| on\_action\_Replace\_triggered()     | "剪切"按下转入该槽，用于开启查找替换框                                           |
| on\_action\_SelectAll\_triggered()   | "全选"按下转入该槽，用于全选                                                     |
| on\_action\_Setting\_triggered()     | "设置"按下转入该槽，用于开启设置框                                               |
| on\_action\_GetCharNum\_triggered()  | "字符统计"按下转入该槽，开启字符统计框                                           |
| on\_action\_AddTime\_triggered()     | "附加日期"按下转入该槽，用于在文章末尾加入日期                                   |
| on\_action\_toLine\_triggered()      | "转到行"按下转入该槽，用于实现转到行                                             |
| on\_action\_toParagraph\_triggered() | "转到段"按下转入该槽，用于实现转到段                                             |
| on\_action\_ShowLine\_triggered()    | "按行显示"按下转入该槽，用于转换行号显示方式                                     |
| getMenu\_F\_state()                  | "文件"栏触发时转入该槽，用于根据某些情况将文件栏中的某些按键置为无效（如"保存"） |
| getMenu\_E\_state()                  | "编辑"栏触发时转入该槽，用于根据某些情况将编辑栏中的某些按键置为无效（如"剪切"） |
| data\_replace()                      | 查找替换框"替换"键按下时转入该槽，用于实现替换操作                               |
| data\_replace\_all()                 | 查找替换框"全部替换"键按下时转入该槽，用于实现全部替换操作                       |
| getDataChanged()                     | 内部数据结构发生改变时，处理内部数据改变事件（标题上加星号等）                   |
| ProtectedUpdate()                    | 实现保护式刷新                                                                   |
| void RefreshProtectTimer()           | 更新保护式刷新计时器                                                             |

### 基本绘图策略

通过MainWindow的paintEvent()函数实现文本的绘制。

#### 文字绘制

设置起始绘图位置DrawX与DrawY。
每次从PosLeftUp所指的字符开始，向后遍历数据结构，并依次绘制遍历到的字符。绘制的字符可能为以下几种情况：

1. 换行符(\'\\n\')，处理方法： DrawY 增加一个单位， DrawX 置零，同时指针后移
2. 制表符(\'\\t\')，处理方法：若 `(DrawX/TabWidth + 1)*TabWidth` 小于 *DataTextWidth* ，则将 *DrawX* 置为 `(DrawX/TabWidth + 1)*TabWidth` ，并根据 *TabStyle* 绘制制表符，同时指针后移；否则 *DrawY* 增加一个单位， *DrawX* 置零
3. 中文字符，处理方法：若`DrawX+2*FontSizeW`小于*DataTextWidth*，则在*DrawX*，*DrawY*处绘制中文字符，并将*DrawX*置为 `DrawX+2*FontSizeW`，同时指针后移；否则*DrawY*增加一个单位，*DrawX* 置零
4. 英文字符，处理方法：若*DrawX+FontSizeW*小于*DataTextWidth*，则在*DrawX*，*DrawY*处绘制中文字符，并将*DrawX*置为*DrawX+FontSizeW*，同时指针后移；否则 *DrawY*增加一个单位，*DrawX*置零
5. 其他字符：按照英文字符处理

重复以上流程，直到 *DrawY* 大于 *DataTextHeight*

由于文本框高度和文本框宽度的值是有范围的变量，故绘图过程的**算法复杂度为o(1)**

#### 光标绘制

光标的绘制由 *PosCur* 和 *PosPre* 的位置决定。若*PosCur* 与 *PosPre* 处于同一位置，则在 *PosCur*的位置（*PosCur.ShowPosX,PosCur.ShowPosY*）处绘制一条竖线；若两者位置不同，则遍历两者之间的文字，将这些文字的背景涂为蓝色。若起始位置在显示框上方，则将在起始位置置为*PosLeftUp*，若终止位置在显示框下方，则最多绘制至显示框的最后一行。该过程由函数 *FillBlueArea()*实现。

由于绘制蓝色区域的部分至多为整个屏幕，且整个屏幕的大小有限，故光标绘制部分的**算法复杂度为o(1)**

#### 行号、段号绘制：

行号：循环变量i从0开始，到*TextBoxHeight*结束，在相应的Y坐标位置处绘制**数字**【DataTextTop + i】

段号：设置临时变量*ParaCounter = DataParaTop + 1*，绘图时，若绘制到换行符，则在下一行绘制**数字**【ParaCounter】，同时*ParaCounter*增加1

由于TextBoxHeight的最大值有限，故段号、行号的绘制过程的**算法复杂度均为o(1)**，且绘制段号的过程与绘制文字的过程结合在一起，**提高了程序运行的效率**

综上所述，**整个绘图部分的算法复杂度为o(1)**

## 用户输入处理方案

### 鼠标输入

鼠标点击事件：鼠标左键点击事件会影响文本框中光标的位置。

**光标定位方案**：由**MainWindow**的 *mousePressEvent(MouseEvent\*event)* 中的参数 *event* 可以获得鼠标点击信息，包括鼠标的键位和位置。

根据*event-\>x()*, *event-\>y()*，*PosLeftUp* ，*TextBoxWidth*即可按以下方法找到光标在数据结构中的位置。

首先从 *PosLeftUp*开始，遍历*event-\>y()-1*行，行判断方式与绘图中的相同。然后，从第 *event-\>y()* 行的X位置为0处向右遍历字符总宽度为 *event-\>x()*的字符串，即可获得鼠标点击位置在数据结构中的位置。获取该位置后，将*PosCur* 与*PosPre*的数据结构位置均赋值为该位置。对*PosCur*的光标定位过程由函数*LocateCursor(int x,int y)*实现

鼠标左键点击事件中，由于鼠标点击的位置相对于窗口的(x,y)坐标的取值范围是有限的，故该过程的**算法复杂度为o(1)**

注：鼠标右键点击会弹出鼠标右键菜单，该菜单使用Qt控件实现，该过程使
用*rightMenu-\>exec()*即可

**鼠标拖动事件：**由**MainWindow**的*mouseMoveEvent(MouseEvent\*event)*即可获取鼠标移动时的鼠标信息，包括鼠标的键位和位置。鼠标点击后拖动可实现选择一片文字区域，从而实现块操作。

**鼠标选区实现方法：**鼠标移动时，若鼠标左键处于按下状态，则保持*PosPre* 的值不变，将 PosCur 定位至鼠标位置。并触发重绘。

**该过程复杂度同鼠标左键点击过程，为o(1)**

### 键盘输入

**键盘输入事件：**键盘输入可实现对内部数据结构的**增、删、改**，也能触发一些快捷功能效果。

当输入法处于关闭状态时，用户按下键盘的操作会触发**MainWindow**的 *keyPressEvent(QKeyEvent\*event)*，通过参数*event* 可以获得键盘输入的键值。

**内部数据结构的增加/修改操作：**通过*event-\>text()*获得键盘输入的字符串信息（类型为*QString*），若此时 *PosCur* 与 *PosPre*在同一位置，则调用内部数据结构的*add()* 方法（详见内部数据结构部分）；若 *PosCur* 与*PosPre*不在同一位置，则根据 *PosCur* 和 *PosPre*的显示位置将 PosCur 和 PosPre 在数据结构中的位置以正确的顺序传入内部数据结构的 *edit()*方法。将数据导入内部数据结构后，会根据 *add()*或 *edit()* 函数的返回值重定位光标在数据结构中的位置，然后使用 *GetDataHeight()* 函数重新计算内部数据结构在当前本文框宽度下的高度和光标的显示位置。若此时光标的显示位置在显示框之外，会调用**MainWindow**的 *FindCursor()*方法通过上下滚页的方式令光标处于显示框当中。

由于每次增加/修改会使用 *GetDataHeight()*函数重新计算内部数据结构在当前本文框宽度下的高度和光标的显示位置，故该过程的**算法复杂度为o(n)**

**内部数据结构的删除操作：**当键盘输入的键值为**BackSpace**时，若*PosCur* 和 *PosPre* 在同一位置，则触发内部数据结构的前删方法*del()*；若 *PosCur* 与 *PosPre* 不在同一位置，则根据 *PosCur*和 *PosPre*的显示位置将 *PosCur*和 PosPre 在数据结构中的位置以正确的顺序传入内部数据结构的 *del()*方法以实现块删除操作。当键盘输入的键值为**Delete**时，若 *PosCur*和 *PosPre* 在同一位置，则触发内部数据结构的后删方法 *del()*；若 *PosCur*与 *PosPre* 不在同一位置，处理方式同**BackSpace**。完成删除操作后，会根据*del()*函数的返回值重定位光标在数据结构中的位置，然后使用*GetDataHeight()* 函数重新计算内部数据结构在当前本文框宽度下的高度和光标的显示位置。若此时光标的显示位置在显示框之外，会调用**MainWindow**的 *FindCursor()*方法通过上下滚页的方式令光标处于显示框当中。

同增加/修改操作，该过程**算法复杂度为o(n)**

**光标移动操作：**键盘输入的键值为上下左右键时，会触发光标的移动。当按下左右按键时，会先修改光标在数据结构中的位置，在通过 *GetDataHeight()* 函数更新光标的显示位置；当按下上下按键时，会通过*LocateCursor()*方法更新光标的显示位置及在数据结构内的位置。

**其余功能按键：**按下**PageUp/PageDown/end**时，会通过 *LocateLeftUpPos()*方法来设置最左上角光标位置，从而实现翻页/定位至最后一行；按下**home**键时，会通过*LocateCursor()*方法将光标定位在当前行行首

**输入法输入：**当输入法处于开启状态时，会触发**MianWindow**的*inputMethodEvent(QInputMethodEvent\*event)*，通过 event 的的 *preeditString()*方法可以获取当前处于活跃编辑界面的字符串，通过 *event* 的*commitString()* 方法可以获取输入法返回的字符串。输入法输入/修改过程同内部数据结构的增加/修改操作。

### 查找操作

用户点击"查找"、"替换"按钮即可弹出查找替换框*ReplaceDlg* 在用户点击查找替换框上的按键时，查找替换框会发送对应的信号来令**MainWindow**处理相应的事件。*ReplaceDlg*发出的信号与**MainWindow**的处理事件的对应情况如下：

| **ReplaceDlg的信号** | **对应的MainWindow的槽函数**      |
| -------------------- | --------------------------------- |
| void FindNext()      | on\_action\_FindNext\_triggered() |
| void Replace()       | data\_replace()                   |
| void ReplaceAll()    | data\_replace\_all()              |

**on\_action\_FindNext\_triggered():**从当前光标位置开始，通过内部数据结构的 *find()* 方法查找字符串。当第一次没有查找到相应的字符串时，会再次从内部数据结构的第一个位置再次查找字符串，以达到循环查找的效果。若第二次查找依然未找到需要的字符串，则弹出"查找失败"提示框。

**data\_replace():**先判断*PosCur*与*PosPre*之间的字符串是否与需要查找的字符串相同，若相同，则调用内部数据结构的*edit()*函数实现字符串替换。无论相同与否，该过程结束后都会调用 *on\_action\_FindNext\_triggered()*方法实现查找下一个。

**data\_replace\_all():**从数据结构的首字符开始，持续调用内部数据结构的*find()*方法直到查找不成功。且在每一次查找成功之后，都会调用内部数据结构的*edit()*方法对找到的字符串进行替换。

### 鼠标滚轮操作

鼠标滚轮操作可以实现上下滚页，获取鼠标滚轮信息通过**MainWindow**的*WheelEvent(QWheelEvent\*event)*实现。上下滚页的实际操作由*LocateLeftUpPos()*实现。

鼠标滚轮操作时，若**Ctrl**键处于按下状态，则不进行上下滚页操作，改为通过 *SetFontSize()*函数修改字体大小。

### 滚动条拖拽操作

当用户拖动滚动条时，获取滚动条的*sliderPosition()*方法获取滚动条滑块的位置，并根据这个位置调用*LocateLeftUpPos()*实现翻页。

### 拖入文件操作

当用户从窗口外拖文件进入窗口内时，会触发**MianWindow**的*dragEnterEvent(QDragEnterEvent\*event)*，拖过*event*可获得拖住文件的文件名，并实现打开文件。

### 文件相关的操作

新建文件时，会将*isUntitled*变量置为true，将*shouldSave* 置为false，打开文件时，会将*isUntitled*置为false，将 *shouldSave*置为false；接收到内部数据结构的更改信号时，会将 *shouldSave* 置为true；进行保存或另存为操作后，会将*isUntitled*和 *shouldSave*置为false。

新建/打开/关闭事件处理之前，若*shouldSave* 为true，则会通过**QMessageBox**弹出询问保存的提示框，并根据用户的选择判断是否执行保存和相关的时间处理操作。
