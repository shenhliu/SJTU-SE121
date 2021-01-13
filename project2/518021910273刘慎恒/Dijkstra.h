#pragma once

#include <cstdint>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <string.h>
using namespace std;

#define MAXSIZE 10000

/**
 * This file declare the main class of Project2:DijkstraProject2.
 * You should implement the methods:`readFromFile`,`run1`and`run2` in Dijkstra.cpp.
 * You can add anything in DijkstraProject2 class to implement Project2.
 */
class DijkstraProject2
{
private:
	//You can declare your graph structure here.
	struct graph
	{
		int _relation[1000][1000]; //存储图的关系,初始时认为每条边间的距离为MAXSIZE
		int _Vnum = 0;			   //存储节点数量
		int _Enum = 0;			   //存储边的数量
		vector<int> distance;	   //存储各个顶点到顶点0的距离
		vector<vector<int>> prev;  //存储每个顶点Dijkstra之后前驱节点，可能有多个，对应多条路径
		vector<int> flag;		   //存储每个顶点是否找过了最短路径

		std::vector<vector<int>> allpath; //记录开始点到结束点的每一条路径
		std::vector<bool> vest;			  //若若vest[i]已并入树中,则vest[i]=true;
		std::vector<int> next;			  //记录顶点的下一个顶点
		int cnt = 0;

		graph()
		{
			for (int i = 0; i < 1000; ++i)
			{
				for (int j = 0; j < 1000; ++j)
				{
					_relation[i][j] = MAXSIZE;
				}
			}
		};

		graph(const graph &obj)
		{
			for (int i = 0; i < 1000; ++i)
			{
				for (int j = 0; j < 1000; ++j)
				{
					_relation[i][j] = obj._relation[i][j];
				}
			}
			_Vnum = obj._Vnum;
			_Enum = obj._Enum;
			for (int i = 0; i < distance.size(); ++i)
			{
				distance[i] = obj.distance[i];
			}
		};

		void reset()
		{
			for (int i = 0; i < 1000; ++i)
			{
				for (int j = 0; j < 1000; ++j)
				{
					_relation[i][j] = MAXSIZE;
				}
			}
			_Vnum = 0;
			_Enum = 0;
		};
	};
	int _Gnum = 0;				 //存储图的数量
	std::vector<graph *> sample; //存储测试样例中的每一个图

public:
	//返回一个字符串中逗号的数量
	int GetCommaNum(char s[]);

	//已知前驱节点序列，获得最短路径
	vector<vector<int>> getPaths(int start, int index, vector<vector<int>> &prevPoints);

	//获得两点之间的所有路径
	void getAllPath(int i, vector<vector<int>> &allpath, vector<bool> &vest, vector<int> &next, const int &Vnum, int &cnt, int relation[1000][1000]);

	//判断vector中的数字是否单调
	bool Is_mon(vector<int> path,int relation[1000][1000]);

	//获得该条路径的距离之和
	int getTotDis(vector<int> path ,int relation[1000][1000]);
	/**
	 * Read graph from Param:`inputfile`.
	 * 
	 */
	
	

	void readFromFile(const char *inputfile = "input.txt");

	/**
	 * Part 1, implement Dijkstra algorithm to finish Part 1
	 * and save the result to Param:`outputFile`.
	 * Save the path as: node_1,node_2...node_n. (seperate nodes with comma)
	 *
	 */
	void run1(const char *outputFile = "output.txt");

	/**
	 * Part 2, find the monotonically increasing path to finish Part 2
	 * and save the result to Param:`outputFile`.
	 * Save the path as: node_1,node_2...node_n. (seperate nodes with comma)
	 *
	 */
	void run2(const char *outputFile = "output.txt");
};
