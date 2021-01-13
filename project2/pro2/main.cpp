/*
reference:

1.版权声明：本文为CSDN博主「茄砸」的原创文章，遵循 CC 4.0 BY-SA 版权协议，转载请附上原文出处链接及本声明。
原文链接：https://blog.csdn.net/u013615687/article/details/69062803

2.博客园：Dijkstra算法(二)之 C++详解
原文链接：https://www.cnblogs.com/skywang12345/p/3711514.html

3.简书:DFS求两点间所有路径
原文链接：https://www.jianshu.com/p/8a83aa1c24c5
*/

#include <iostream>
#include "Dijkstra.h"

/**
 * You can use this file to test your code.
 */
int main()
{
    DijkstraProject2 pro;
    pro.readFromFile();
    pro.run1();
    pro.run2();
}
/*Debug记录：
read：
一开始用了一行的长度而不是逗号数量来判断，并且默认一个数字只占一个byte。。。找了好久问题
添加新的图到图的vector中时，一定要拷贝构造函数，而不是直接把curg放进去，不然之后reset的时候，vector里面的curg也被干掉了
run1： 
tmp = xxx 打成了tmp == xxx 导致tmp 并没有被赋值
连续if判断距离时，没加else导致上边给tmp赋值对下边if造成影响，以后记得一定加else
results = getPaths(0, sample[p]->_Vnum - 1, sample[p]->prev);  最大下标没-1.。。
*/