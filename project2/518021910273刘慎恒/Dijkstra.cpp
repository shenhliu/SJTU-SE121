#include "Dijkstra.h"
#include <iostream>

int DijkstraProject2::GetCommaNum(char s[])
{
	int num = 0;
	for (int i = 0; i < strlen(s); ++i)
	{
		if (s[i] == ',')
		{
			num++;
		}
	}
	return num;
}

vector<vector<int>> DijkstraProject2::getPaths(int start, int index, vector<vector<int>> &prevPoints)
{
	vector<vector<int>> childPaths;
	vector<vector<int>> midPaths;

	if (index != start)
	{
		for (int i = 0; i < prevPoints[index].size(); ++i)
		{
			childPaths = getPaths(start, prevPoints[index][i], prevPoints);

			for (int j = 0; j < childPaths.size(); ++j)
			{
				childPaths[j].push_back(index);
			}
			if (midPaths.empty())
			{
				midPaths = childPaths;
			}
			else
			{
				midPaths.insert(midPaths.end(), childPaths.begin(), childPaths.end());
			}
		}
	}
	else
	{
		// 第一个点
		midPaths.push_back(vector<int>(1, start));
	}
	return midPaths;
}

void DijkstraProject2::getAllPath(int i, vector<vector<int>> &allpath, vector<bool> &vest, vector<int> &next, const int &Vnum, int &cnt, int relation[1000][1000])
{
	vest[i] = true;

	if (i == Vnum - 1)
	{
		allpath.push_back(vector<int>(0)); //添加一条路径
		int length = allpath.size();
		cnt++;
		int k = 0;
		while (k != Vnum - 1)
		{
			allpath[length - 1].push_back(k);
			k = next[k];
		}
		allpath[length - 1].push_back(k);
		return;
	}

	for (int j = 0; j < Vnum; j++)
	{
		if (vest[j] == false && relation[i][j] != MAXSIZE)
		{
			next[i] = j; //对于路径0->1->3: nex[0]=1;nex[1]=3;
			getAllPath(j, allpath, vest, next, Vnum, cnt, relation);
			vest[j] = false;
		}
	}
}

bool DijkstraProject2::Is_mon(vector<int> path, int relation[1000][1000])
{
	//获得距离
	vector<int> distance;
	for (int l = 0; l < path.size() - 1; ++l)
	{
		int prenode = path[l];
		int nextnode = path[l + 1];
		int dis = relation[prenode][nextnode];
		distance.push_back(dis);
	}

	int i;
	int pre = distance[0];
	for (i = 1; i < distance.size(); ++i) //判断是否递增
	{
		if (distance[i] > pre)
		{
			pre = distance[i];
			continue;
		}
		else
			break;
	}
	if (i == distance.size()) //递增
	{
		return true;
	}
	else //判断是否递减
	{
		i = 1;
		pre = distance[0];
		for (i = 1; i < distance.size(); ++i)
		{
			if (distance[i] < pre)
			{
				pre = distance[i];
				continue;
			}
			else
			{
				break;
			}
		}
		if (i == distance.size())
		{
			return true;
		}
	}
	return false;
}

int DijkstraProject2::getTotDis(vector<int> path, int relation[1000][1000])
{
	//获得距离
	vector<int> distance;
	for (int l = 0; l < path.size() - 1; ++l)
	{
		int prenode = path[l];
		int nextnode = path[l + 1];
		int dis = relation[prenode][nextnode];
		distance.push_back(dis);
	}

	int ans = 0;
	for (int i = 0; i < distance.size(); ++i)
	{
		ans += distance[i];
	}
	return ans;
}

/**
 * You should implement the methods:`readFromFile`,`run1`and`run2` here.
 */
void DijkstraProject2::readFromFile(const char *inputfile) //要求每个图最后都要有一行空行，包括最后一个图
{
	std::cout << "readFromFile: " << inputfile << std::endl;
	char buffer[1000];
	graph *curg = new graph();
	fstream File;
	File.open(inputfile, ios::in);
	while (!File.eof())
	{
		File.getline(buffer, 1000, '\n');
		if (GetCommaNum(buffer) == 1) //如果有一个逗号，就是每个图的第一行
		{
			string strde = ",";
			char *delim = (char *)strde.data();
			char *token;
			token = strtok(buffer, delim);
			curg->_Vnum = atoi(token);
			token = strtok(NULL, delim);
			curg->_Enum = atoi(token);
			//cout << curg->_Vnum << " " << curg->_Enum << endl;
		}
		else if (GetCommaNum(buffer) == 2) //如果有两个逗号，这一行指明图的关系
		{
			string strde = ",";
			char *delim = (char *)strde.data();
			char *token;
			token = strtok(buffer, delim);
			int v1 = atoi(token);
			token = strtok(NULL, delim);
			int v2 = atoi(token);
			token = strtok(NULL, delim);
			int weight = atoi(token);
			curg->_relation[v1][v2] = weight;
			//cout << v1 << " " << v2 << " " << weight << endl;
		}
		else //剩下的情况是一个空行，即代表一个图已经结束了
		{
			graph *tmp = new graph(*curg); //一定要用拷贝构造函数，否则curg调用reset时vector里的也被reset了
			sample.push_back(tmp);		   //先把当前的图存入vector
			//然后清空curg
			curg->reset();
		}
	}
	File.close();
	// for (int p = 0; p < sample.size(); ++p)
	// {
	// 	for (int i = 0; i < 10; ++i)
	// 	{
	// 		for (int j = 0; j < 10; ++j)
	// 		{
	// 			cout << sample[p]->_relation[i][j] << " ";
	// 		}
	// 		cout << endl;
	// 	}
	// 	cout << "end" << endl;
	// }
}

void DijkstraProject2::run1(const char *outputFile)
{
	//TODO
	std::cout << "Save result to file:" << outputFile << std::endl;
	for (int p = 0; p < sample.size(); ++p)
	{
		int i, j, k;
		int min;
		int tmp;
		sample[p]->distance.resize(sample[p]->_Vnum);
		sample[p]->flag.resize(sample[p]->_Vnum);
		//初始化
		for (int l = 0; l < sample[p]->_Vnum; ++l)
		{
			sample[p]->prev.push_back(vector<int>(1, -1)); //每个点先设只有一个合适的前驱节点
		}
		for (int l = 0; l < sample[p]->_Vnum; ++l)
		{
			sample[p]->distance[l] = MAXSIZE; //初始把所有距离都设为最大
		}
		for (i = 0; i < sample[p]->_Vnum; ++i)
		{
			sample[p]->flag[i] = 0;								 //初始时认为都没有找到最短路径
			sample[p]->prev[i][0] = 0;							 //初始时设置前驱顶点都为0
			sample[p]->distance[i] = sample[p]->_relation[0][i]; //顶点i的最短路径为0到i的权
		}

		//对0号顶点初始化
		sample[p]->flag[0] = 1;
		sample[p]->distance[0] = 0;

		//遍历_Vnum - 1次，每次找出一个顶点的最短路径
		for (i = 1; i < sample[p]->_Vnum; ++i)
		{
			// 寻找当前最小的路径；
			// 即，在未获取最短路径的顶点中，找到离vs最近的顶点(k)。
			min = MAXSIZE;
			for (j = 0; j < sample[p]->_Vnum; ++j)
			{
				if (sample[p]->flag[j] == 0 && sample[p]->distance[j] < min)
				{
					min = sample[p]->distance[j];
					k = j;
				}
			}
			//标记顶点k已经获得最短路径
			sample[p]->flag[k] = 1;
			//cout << "k:" << k << endl;
			// 修正当前最短路径和前驱顶点
			// 即，当已经"顶点k的最短路径"之后，更新"未获取最短路径的顶点的最短路径和前驱顶点"。
			for (j = 0; j < sample[p]->_Vnum; ++j)
			{
				tmp = ((sample[p]->_relation[k][j] == MAXSIZE) ? MAXSIZE : (min + sample[p]->_relation[k][j]));
				//// cout << "min:" << min << endl;
				//// cout << "relation" << sample[p]->_relation[k][j] << endl;
				//// cout << "tmp:" << tmp << endl;
				if (sample[p]->flag[j] == 0 && tmp < sample[p]->distance[j]) //如果顶点k带来更短路径，替换现在的prev
				{
					sample[p]->distance[j] = tmp;
					sample[p]->prev[j].clear();
					sample[p]->prev[j].push_back(k);
				}
				else if (sample[p]->flag[j] == 0 && sample[p]->distance[j] == tmp && tmp != MAXSIZE) //如果顶点k带来的新的距离等于现在的距离，那么新加入一个前驱节点
				{
					sample[p]->prev[j].push_back(k);
				}
			}
		}

		// for (int x = 0; x < sample[p]->prev.size(); ++x)
		// {
		// 	for (int y = 0; y < sample[p]->prev[x].size(); ++y)
		// 	{
		// 		cout << sample[p]->prev[x][y] << " ";
		// 	}
		// 	cout << endl;
		// 	cout << endl;
		// }
		// cout << "end" << endl;

		//到这里，所有前驱节点都存在prev里了，下边开始获得路径并输出到文件中
		vector<vector<int>> results;
		results = getPaths(0, sample[p]->_Vnum - 1, sample[p]->prev);
		//// cout << "test ok" << endl;
		int dist = sample[p]->distance[sample[p]->_Vnum - 1]; //最短距离
		int pathNum = results.size();						  //最短路径数量
		////cout << "test ok" << endl;
		fstream File("output.txt", ios::in | ios::app);
		File << dist << "\n";
		File << pathNum << "\n";
		for (int i = 0; i < results.size(); ++i)
		{
			for (int j = 0; j < results[i].size() - 1; ++j)
			{
				File << results[i][j] << ","; //写入一个节点数，一个逗号
			}
			File << results[i][results[i].size() - 1]; //最后只写入节点数
			File << "\n";
		}
		File << "end"
			 << "\n";
		File << "\n";
		File.close();
	}
}

void DijkstraProject2::run2(const char *outputFile)
{
	//TODO
	for (int p = 0; p < sample.size(); ++p)
	{
		sample[p]->allpath.clear();
		sample[p]->next.clear();
		sample[p]->vest.clear();
		sample[p]->vest.resize(sample[p]->_Vnum);
		sample[p]->next.resize(sample[p]->_Vnum);
		sample[p]->cnt = 0;
		vector<vector<int>> answer;
		//cout << "test ok" << endl;
		getAllPath(0, sample[p]->allpath, sample[p]->vest, sample[p]->next, sample[p]->_Vnum, sample[p]->cnt, sample[p]->_relation);

		//cout << "test ok" << endl;
		//cout << sample[p]->allpath.size() << endl;
		// for (int i = 0; i < sample[p]->allpath.size(); ++i)
		// {
		// 	for (int j = 0; j < sample[p]->allpath[i].size(); ++j)
		// 	{
		// 		cout << sample[p]->allpath[i][j] << endl;
		// 	}
		// 	cout << endl;
		// }
		// cout << "end" << endl;

		int mindistance = MAXSIZE;
		for (int i = 0; i < sample[p]->allpath.size(); ++i)
		{
			//cout << "test ok" << endl;
			if (Is_mon(sample[p]->allpath[i], sample[p]->_relation)) //如果单调
			{
				int tmp = getTotDis(sample[p]->allpath[i], sample[p]->_relation);
				//cout << tmp << endl;
				if (tmp < mindistance) //如果得到了一条更短的路径
				{
					mindistance = tmp;
					answer.clear();
					answer.push_back(sample[p]->allpath[i]);
				}
				else if (tmp == mindistance)
				{
					answer.push_back(sample[p]->allpath[i]);
				}
				else
				{
					continue;
				}
			}
		}
		//cout << answer.size() << endl;
		//cout <<写入文件
		fstream File("output.txt", ios::in | ios::app);
		File << mindistance << "\n";
		File << answer.size() << "\n";
		for (int i = 0; i < answer.size(); ++i)
		{
			for (int j = 0; j < answer[i].size() - 1; ++j)
			{
				File << answer[i][j] << ","; //写入一个节点数，一个逗号
			}
			File << answer[i][answer[i].size() - 1]; //最后只写入节点数
			File << "\n";
		}
		File << "end"
			 << "\n";
		File << "\n";
		File.close();
	}
}
