//level代表文件目录，从level[0]开始，level[i]代表第i层有几个文件，0就是没有，1就是1个
//sstable代表文件，从sstable0开始

#include "kvstore.h"

namespace fs = std::filesystem;

uint64_t KVStore::gettime()
{
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch());

	long long int time = ms.count();
	return time;
}

KVStore::KVStore(const std::string &dir) : KVStoreAPI(dir)
{
	level.push_back(0); //level[i]表示第i层有几个文件，一开始先初始化level0，里面有0个文件
}

KVStore::~KVStore()
{
}

/**
 * Insert/Update the key-value pair.
 * No return values for simplicity.
 */
void KVStore::put(uint64_t key, const std::string &s)
{
	list.put(key, s);
	if (list.space() > 1048576)
	{
		if (!fs::exists("sstables/level0"))
		{
			fs::create_directories("sstables/level0");
		}
		std::ofstream outFile("newsstable.dat", std::ios::app | std::ios::binary); //创建一个新的文件，先叫newsstable
		QuadListNode *node = list.bottomFirst();
		uint64_t tmpkey = 0;
		std::string tmpval = "";
		uint64_t time = 0;
		char *charval;
		int valsize = 0;
		int tmpoffset = 0;
		uint64_t lo = list.bottomFirst()->key; //存放较小的范围
		uint64_t hi = list.bottomLast()->key;  //存放较大的范围
		while (node != list.bottomLast()->succ)
		{
			//先写时间戳，再写key,再写value
			tmpkey = node->key;
			tmpval = node->value;
			valsize = tmpval.size() + 1;
			charval = (char *)tmpval.data();
			time = gettime();
			outFile.write((char *)(&time), sizeof(uint64_t));
			outFile.write((char *)(&tmpkey), sizeof(uint64_t));
			outFile.write((char *)(&charval), valsize);
			//写索引
			IndexNode tmpNode(tmpoffset, 0, level[0], time, valsize);			   //先写到level0文件目录,文件编码为level[0].不需要+1
			index[tmpkey] = tmpNode;											   //索引中自动覆盖即可
			tmpoffset = tmpoffset + sizeof(uint64_t) + sizeof(uint64_t) + valsize; //算出下一个offset
			node = node->succ;
		}
		outFile.close();
		level[0]++;	  //第0层文件目录多了1个
		list.reset(); //清空跳表

		//修改文件名称
		int fileNum = level[0] - 1; //注意，文件标号是文件数目-1
		std::string sstableName = "./sstables/level0/sstable" + std::to_string(fileNum) + ".dat";
		char *charsstableName = (char *)sstableName.data();				  //转成char*才能写，我也不知道为啥
		std::rename("./sstables/level0/newsstable.dat", charsstableName); //文件从newsstable改成文件标号
		ScanNode tmpscan(lo, hi);
		scan[0][fileNum] = tmpscan; //写入这个sstable的key的范围

		compaction(level, index);
	}
}
/**
 * Returns the (string) value of the given key.
 * An empty string indicates not found.
 */
std::string KVStore::get(uint64_t key)
{
	if (list.get(key) != "")
	{
		return list.get(key);
	}
	else
	{
		map<uint64_t, IndexNode>::iterator iter;
		iter = index.find(key);
		if (iter == index.end()) //在索引里也没找到
		{
			return "";
		}
		else //在索引里找到了
		{
			int offset = index[key].offset;
			int dirLevel = index[key].dirLevel;
			int fileNum = index[key].fileNum;
			std::string fileName = "./sstables/level" + std::to_string(dirLevel) +
								   "sstable" + std::to_string(fileNum) + ".dat";
			std::ifstream inFile(fileName, std::ios::in | std::ios::binary);
			inFile.seekg(index[key].offset + 2 * sizeof(uint64_t), ios::beg);
			char *buf;
			inFile.read((char *)&buf, index[key].length);
			std::string ans = buf;
			return ans;
		}
	}
}
/**
 * Delete the given key-value pair if it exists.
 * Returns false iff the key is not found.
 */
bool KVStore::del(uint64_t key)
{
	//如果插入到文件中，插入"`"（键盘左上角的那个键）视为删除标记
	if (list.get(key) != "")
	{
		return list.remove(key);
		return true;
	}

	else
	{
		put(key, "`");
		map<uint64_t, IndexNode>::iterator tmp = index.find(key);
		if (tmp != index.end())
		{
			index.erase(key);
			return true;
		}
	}
	return false;
}

/**
 * This resets the kvstore. All key-value pairs should be removed,
 * including memtable and all sstables files.
 */
void KVStore::reset()
{
}

int KVStore::maxfile(int level)
{
	return pow(2, (level + 1));
}

void KVStore::remdup(std::vector<TableNode> &vec)
{
	int i = 0;
	for (i = 0; i < vec.size() - 1; ++i)
	{
		if (vec[i].key == vec[i + 1].key)
		{
			if (vec[i].time < vec[i + 1].time) //如果mergevec[i]的时间比mergevec[i+1]要早,删掉mergevec[i]
			{
				vec.erase(vec.begin() + i);
				i--;	  //把迭代器退回一个
				continue; //重新进入循环，不然可能会出现下标为[-1]的情况
			}
			else //如果mergevec[i+1]的时间比mergevec[i]要早,删掉mergevec[i+1]
			{
				vec.erase(vec.begin() + i + 1);
				i--;
				continue;
			}
		}
	}
}

void KVStore::handleDel(std::vector<TableNode> &vec)
{
	int i = 0;
	for (i = 0; i < vec.size(); ++i)
	{
		if (vec[i].value == "`") //如果遇到了删除记号
		{
			vec.erase(vec.begin() + i);
		}
	}
}

void KVStore::compaction(std::vector<int> &level, map<uint64_t, IndexNode> &index)
{
	//如果是level0的compaction
	if (level.size() == 1)
	{
		if (!fs::exists("sstables/level1")) //如果level1不存在,创建level1目录,并在level加一个元素
		{
			fs::create_directories("sstables/level1");
			level.push_back(0);
		}

		if (level[1] == 0) //如果level1中没有文件
		{
			vector<TableNode> mergevec; //多路归并完成后用来储存键值对
			int level1num = level[1];	//得到level1的文件数
			int level0num = level[0];	//得到level0的文件数
			//files用来存储要归并的数组
			std::vector<std::vector<TableNode>> files(3 + level1num); //创建一个二维数组，其中含有的一维vector个数是level0和level1中sstables的总和
			uint64_t tmptime;
			uint64_t tmpkey;
			string tmpval;
			char *chartmpval;
			for (int p = 0; p < level[0]; ++p) //先对level0里面的文件进行处理
			{
				std::string filename = "./sstables/level0/sstable" + std::to_string(p) + ".dat";
				char *charFileName = (char *)filename.data();
				std::ifstream curtable(charFileName, std::ios::in | std::ios::binary);
				while (!curtable.eof()) //一口气读完
				{
					curtable.read((char *)(&tmptime), sizeof(uint64_t));
					curtable.read((char *)(&tmpkey), sizeof(uint64_t));
					int valsize = index[tmpkey].length;
					curtable.read((char *)(&chartmpval), valsize);
					tmpval = chartmpval;
					TableNode tmpnode(tmptime, tmpkey, tmpval);
					files[p].push_back(tmpnode);
				}
				curtable.close();
				std::remove(charFileName); //读完就把文件删掉
			}

			//到这里，level0中所有的键值对就都读到files里了
			//为每一个数组添加有MAX作为key的尾巴，方便之后归并
			for (int i = 0; i < files.size(); ++i)
			{
				TableNode tailNode(0, MAX, "");
				files[i].push_back(tailNode);
			}
			std::vector<int> mergeover; //保存每个数组MAX的下标
			std::vector<int> curnum;	//保存当前 每个数组应该参与归并的下标
			for (int p = 0; p < files.size(); ++p)
			{
				std::vector<TableNode> tmp = files[p];
				mergeover.push_back(tmp.size() - 1); //此处要-1，因为是下标
			}
			curnum.resize(mergeover.size());
			for (int i = 0; i < curnum.size(); ++i)
			{
				curnum[i] = 0;
			}

			//开始归并
			while (mergeover != curnum) //每一次循环加入一个node到mergevec里面
			{
				std::vector<uint64_t> minkeyvec;
				for (int i = 0; i < files.size(); ++i) //获得每个数组的最小key存放在minkey里
				{
					int tmp = curnum[i];
					TableNode tmpnode = files[i][tmp];
					minkeyvec.push_back(tmpnode.key);
				}

				std::vector<uint64_t>::iterator smallest = std::max_element(std::begin(minkeyvec), std::end(minkeyvec));
				int distance = std::distance(std::begin(minkeyvec), smallest); //distance是最小key的下标
				TableNode minNode = files[distance][curnum[distance]];
				mergevec.push_back(minNode);
				curnum[distance]++;
			}

			//到这里已经归并完成，所有键值对都在mergevec里面了
			//去除重复
			remdup(mergevec);
			//处理删除
			handleDel(mergevec);
			//重新划分文件并写入level1
			std::string level1FileName = "./sstables/level1/sstable" + std::to_string(level[1]) + ".dat";
			std::ofstream sstable(level1FileName, std::ios::app | std::ios::binary); //要写入的sstable

			uint64_t lo = mergevec.front().key; //存放较小的范围
			uint64_t hi = mergevec.back().key;	//存放较大的范围
			int curoffset = 0;
			int curdirlevel = 0;
			int curfilenum = 0;
			//写sstable
			for (int i = 0; i < mergevec.size(); ++i)
			{
				TableNode node = mergevec[i];
				sstable.write((char *)(&(node.time)), sizeof(uint64_t));
				sstable.write((char *)(&(node.key)), sizeof(uint64_t));
				char *charval = (char *)node.value.data();
				int valsize = node.value.size() + 1;
				sstable.write((char *)(&charval), valsize);
				//更新索引
				curdirlevel = 1;
				curfilenum = level[1];
				IndexNode curIndexNode(curoffset, curdirlevel, curfilenum, node.time, valsize);
				index[node.key] = curIndexNode;
				curoffset = curoffset + sizeof(uint64_t) + sizeof(uint64_t) + valsize;
			}
			sstable.close();
			//更新范围
			ScanNode newscan(lo, hi);
			scan[1][0] = newscan;
			//更新level
			level[1]++;
			level[0] = 0;
		}
		else //如果level1中有文件
		{
			//先找到level1中scan与level0重合的sstable
			uint64_t level0Low = MAX; //level0中的最大key和最小key
			uint64_t level0High = 0;
			for (int i = 0; i < level[0]; ++i)
			{
				if (scan[0][i].low < level0Low)
				{
					level0Low = scan[0][i].low;
				}
				if (scan[0][i].high > level0High)
				{
					level0High = scan[0][i].high;
				}
			}
			int level1start = 0; //需要参与归并的level1中sstable的下标
			int level1end = 0;
			for (int i = 0; i < level[1]; i++)
			{
				if (scan[1][i].low < level0Low && scan[1][i].high > level0Low)
				{
					level1start = i;
				}
				if (scan[1][i].low < level0High && scan[1][i].high > level0High)
				{
					level1end = i;
				}
			}
			vector<TableNode> mergevec;													//多路归并完成后用来储存键值对
			std::vector<std::vector<TableNode>> files(3 + level1end - level1start + 1); //创建一个二维数组
			uint64_t tmptime;
			uint64_t tmpkey;
			string tmpval;
			char *chartmpval;
			//下边将level0里面的键值对都存到files里面
			for (int p = 0; p < level[0]; ++p) //先对level0里面的文件进行处理
			{
				std::string filename = "./sstables/level0/sstable" + std::to_string(p) + ".dat";
				char *charFileName = (char *)filename.data();
				std::ifstream curtable(charFileName, std::ios::in | std::ios::binary);
				while (!curtable.eof()) //一口气读完
				{
					curtable.read((char *)(&tmptime), sizeof(uint64_t));
					curtable.read((char *)(&tmpkey), sizeof(uint64_t));
					int valsize = index[tmpkey].length;
					curtable.read((char *)(&chartmpval), valsize);
					tmpval = chartmpval;
					TableNode tmpnode(tmptime, tmpkey, tmpval);
					files[p].push_back(tmpnode);
				}
				curtable.close();
				std::remove(charFileName); //读完就把文件删掉
			}

			//下边将level1里面范围重合的sstable读到内存里
			for (int p = level1start; p <= level1end; ++p)
			{
				std::string filename = "./sstables/level1/sstable" + std::to_string(p) + ".dat";
				char *charFileName = (char *)filename.data();
				std::ifstream curtable(charFileName, std::ios::in | std::ios::binary);
				while (!curtable.eof()) //一口气读完
				{
					curtable.read((char *)(&tmptime), sizeof(uint64_t));
					curtable.read((char *)(&tmpkey), sizeof(uint64_t));
					int valsize = index[tmpkey].length;
					curtable.read((char *)(&chartmpval), valsize);
					tmpval = chartmpval;
					TableNode tmpnode(tmptime, tmpkey, tmpval);
					files[p + level[0]].push_back(tmpnode);
				}
				curtable.close();
				std::remove(charFileName); //读完就把文件删掉
			}

			//为每一个数组添加MAX作为key的尾巴，方便之后归并
			for (int i = 0; i < files.size(); ++i)
			{
				TableNode tailNode(0, MAX, "");
				files[i].push_back(tailNode);
			}

			std::vector<int> mergeover; //保存每个数组MAX的下标
			std::vector<int> curnum;	//保存当前 每个数组应该参与归并的下标
			for (int p = 0; p < files.size(); ++p)
			{
				std::vector<TableNode> tmp = files[p];
				mergeover.push_back(tmp.size() - 1); //此处要-1，因为是下标
			}
			curnum.resize(mergeover.size());
			for (int i = 0; i < curnum.size(); ++i)
			{
				curnum[i] = 0;
			}

			//开始归并
			while (mergeover != curnum) //每一次循环加入一个node到mergevec里面
			{
				std::vector<uint64_t> minkeyvec;
				for (int i = 0; i < files.size(); ++i) //获得每个数组的最小key存放在minkey里
				{
					int tmp = curnum[i];
					TableNode tmpnode = files[i][tmp];
					minkeyvec.push_back(tmpnode.key);
				}

				std::vector<uint64_t>::iterator smallest = std::max_element(std::begin(minkeyvec), std::end(minkeyvec));
				int distance = std::distance(std::begin(minkeyvec), smallest); //distance是最小key的下标
				TableNode minNode = files[distance][curnum[distance]];
				mergevec.push_back(minNode);
				curnum[distance]++;
			}

			//到这里已经归并完成，所有键值对都在mergevec里面了
			//去除重复
			remdup(mergevec);
			//处理删除
			handleDel(mergevec);
			//重新划分文件并写入level1
			//首先把scan不重合的后边的sstable更名，腾出位置来
			for (int i = level1end + 1; i < level[1]; ++i)
			{
				std::string oldname = "./sstables/level1/sstable" + std::to_string(i) + ".dat";
				char *charold = (char *)oldname.data();
				std::string newname = "./sstables/level1/sstable" + std::to_string(i + 1) + ".dat";
				char *charnew = (char *)newname.data();
				std::rename(charold, charnew); //完成更名
			}

			//写sstable
			for (int i = level1start; i <= level1end + 1; ++i)
			{
				std::string newtablename = "./sstables/level1/sstable" + std::to_string(i) + ".dat";
				std::ofstream sstable(newtablename, std::ios::app | std::ios::binary);
				//每个新的sstable中键值对的个数
				int newNum = mergevec.size() / (level1end - level1start + 2);
				uint64_t lo = mergevec[(i - level1start) * newNum].key;			//存放较小的范围
				uint64_t hi = mergevec[(i - level1start + 1) * newNum - 1].key; //存放较大的范围
				int curoffset = 0;
				int curdirlevel = 0;
				int curfilenum = 0;
				for (int p = (i - level1start) * newNum; p < (i - level1start + 1) * newNum; ++p)
				{
					TableNode node = mergevec[p];
					sstable.write((char *)(&(node.time)), sizeof(uint64_t));
					sstable.write((char *)(&(node.key)), sizeof(uint64_t));
					char *charval = (char *)node.value.data();
					int valsize = node.value.size() + 1;
					sstable.write((char *)(&charval), valsize);
					//更新索引
					curdirlevel = 1;
					curfilenum = i;
					IndexNode curIndexNode(curoffset, curdirlevel, curfilenum, node.time, valsize);
					index[node.key] = curIndexNode;
					curoffset = curoffset + sizeof(uint64_t) + sizeof(uint64_t) + valsize;
				}
				sstable.close();
				//更新范围
				ScanNode newscan(lo, hi);
				scan[1][i] = newscan;
				//处理一下尾巴（不能整除的情况）
				if (i == level1end)
				{
					std::ofstream sstable(newtablename, std::ios::app | std::ios::binary);
					hi = mergevec.back().key;
					for (int p = (i - level1start + 1) * newNum; p < mergevec.size(); ++p)
					{
						TableNode node = mergevec[p];
						sstable.write((char *)(&(node.time)), sizeof(uint64_t));
						sstable.write((char *)(&(node.key)), sizeof(uint64_t));
						char *charval = (char *)node.value.data();
						int valsize = node.value.size() + 1;
						sstable.write((char *)(&charval), valsize);
						//更新索引
						curdirlevel = 1;
						curfilenum = i;
						IndexNode curIndexNode(curoffset, curdirlevel, curfilenum, node.time, valsize);
						index[node.key] = curIndexNode;
						curoffset = curoffset + sizeof(uint64_t) + sizeof(uint64_t) + valsize;
					}
					sstable.close();
					//更新范围
					ScanNode newscan(lo, hi);
					scan[1][i] = newscan;
					//更新level
					level[1]++;
				}
			}
		}
	}
	//策略：3变1
	else //如果是level0下面的level的compaction
	{
		int levelCheck = 1; //从第一层开始检查每一层目录,levelCheck是当前检查到的层数,level[k]是第k层现在有的文件数
		for (levelCheck = 1; levelCheck < level.size(); ++levelCheck)
		{
			if (level[levelCheck] > maxfile(levelCheck)) //如果有一层的文件数多于可以容纳的最多的文件数
			{
				if (level.size() == levelCheck + 1) //如果文件数超标的是最底层的文件目录，那么就新开文件目录放进去
				{
					level.push_back(0);
					string dirName = "sstables/level" + std::to_string(levelCheck + 1);
					fs::create_directories(dirName); //新建文件目录
					//取出最后三个sstable
					vector<TableNode> mergevec;			  //多路归并完成后用来储存键值对i
					int lastLevelNum = level[levelCheck]; //得到sstable超出数量的目录的文件数
					int nextLevelNum = level[levelCheck + 1];
					//files用来存储要归并的数组
					std::vector<std::vector<TableNode>> files(3); //创建一个二维数组，其中有三个一维向量
					uint64_t tmptime;
					uint64_t tmpkey;
					string tmpval;
					char *chartmpval;
					for (int p = level[levelCheck] - 3; p < level[levelCheck]; ++p)
					{
						std::string filename = "./sstables/level0/sstable" + std::to_string(p) + ".dat";
						char *charFileName = (char *)filename.data();
						std::ifstream curtable(charFileName, std::ios::in | std::ios::binary);
						while (!curtable.eof()) //一口气读完
						{
							curtable.read((char *)(&tmptime), sizeof(uint64_t));
							curtable.read((char *)(&tmpkey), sizeof(uint64_t));
							int valsize = index[tmpkey].length;
							curtable.read((char *)(&chartmpval), valsize);
							tmpval = chartmpval;
							TableNode tmpnode(tmptime, tmpkey, tmpval);
							files[p - (level[levelCheck] - 3)].push_back(tmpnode);
						}
						curtable.close();
						std::remove(charFileName); //读完就把文件删掉
					}

					//为每一个数组添加有MAX作为key的尾巴，方便之后归并
					for (int i = 0; i < files.size(); ++i)
					{
						TableNode tailNode(0, MAX, "");
						files[i].push_back(tailNode);
					}

					std::vector<int> mergeover; //保存每个数组MAX的下标
					std::vector<int> curnum;	//保存当前 每个数组应该参与归并的下标
					for (int p = 0; p < files.size(); ++p)
					{
						std::vector<TableNode> tmp = files[p];
						mergeover.push_back(tmp.size() - 1); //此处要-1，因为是下标
					}
					curnum.resize(mergeover.size());
					for (int i = 0; i < curnum.size(); ++i)
					{
						curnum[i] = 0;
					}

					//开始归并
					while (mergeover != curnum) //每一次循环加入一个node到mergevec里面
					{
						std::vector<uint64_t> minkeyvec;
						for (int i = 0; i < files.size(); ++i) //获得每个数组的最小key存放在minkey里
						{
							int tmp = curnum[i];
							TableNode tmpnode = files[i][tmp];
							minkeyvec.push_back(tmpnode.key);
						}

						std::vector<uint64_t>::iterator smallest = std::max_element(std::begin(minkeyvec), std::end(minkeyvec));
						int distance = std::distance(std::begin(minkeyvec), smallest); //distance是最小key的下标
						TableNode minNode = files[distance][curnum[distance]];
						mergevec.push_back(minNode);
						curnum[distance]++;
					}

					//去除重复
					remdup(mergevec);
					//处理删除
					handleDel(mergevec);
					//重新划分文件并写入level[levelCheck+1]
					std::string level1FileName = "./sstables/level" +
												 std::to_string(levelCheck + 1) +
												 "/sstable" + std::to_string(nextLevelNum) + ".dat";
					std::ofstream sstable(level1FileName, std::ios::app | std::ios::binary); //要写入的sstable
					uint64_t lo = mergevec.front().key;										 //存放较小的范围
					uint64_t hi = mergevec.back().key;										 //存放较大的范围
					int curoffset = 0;
					int curdirlevel = 0;
					int curfilenum = 0;
					//写sstable
					for (int i = 0; i < mergevec.size(); ++i)
					{
						TableNode node = mergevec[i];
						sstable.write((char *)(&(node.time)), sizeof(uint64_t));
						sstable.write((char *)(&(node.key)), sizeof(uint64_t));
						char *charval = (char *)node.value.data();
						int valsize = node.value.size() + 1;
						sstable.write((char *)(&charval), valsize);
						//更新索引
						curdirlevel = levelCheck + 1;
						curfilenum = level[levelCheck + 1];
						IndexNode curIndexNode(curoffset, curdirlevel, curfilenum, node.time, valsize);
						index[node.key] = curIndexNode;
						curoffset = curoffset + sizeof(uint64_t) + sizeof(uint64_t) + valsize;
					}
					sstable.close();
					//更新范围
					ScanNode newscan(lo, hi);
					scan[levelCheck + 1][level[levelCheck + 1]] = newscan;
					//更新level
					level[levelCheck + 1]++;
					level[levelCheck] -= 3;
				}
				else //如果不是最底层文件目录
				{
					//先找到levelCheck+1层与levelCheck范围重合的sstable
					uint64_t lastLow = MAX; //当前层最大的key
					uint64_t lastHigh = 0;	//当前层最小的key
					for (int i = 0; i < level[levelCheck]; ++i)
					{
						if (scan[levelCheck][i].low < lastLow)
						{
							lastLow = scan[levelCheck][i].low;
						}
						if (scan[levelCheck][i].high > lastHigh)
						{
							lastHigh = scan[levelCheck][i].low;
						}
					}
					int nextstart; //需要参与归并的下一层的sstable的下标中最小的
					int nextend;
					for (int i = 0; i < level[levelCheck + 1]; i++)
					{
						if (scan[levelCheck][i].low < lastLow && scan[levelCheck][i].high > lastHigh)
						{
							nextstart = i;
						}
						if (scan[levelCheck][i].low < lastHigh && scan[levelCheck][i].high > lastHigh)
						{
							nextend = i;
						}
					}
					vector<TableNode> mergevec; //多路归并完成后用来储存键值对
					std::vector<std::vector<TableNode>> files(3 + nextend - nextstart + 1);
					uint64_t tmptime;
					uint64_t tmpkey;
					string tmpval;
					char *chartmpval;
					//将levelCheck层的键值对都存到files里面
					for (int p = level[levelCheck] - 3; p < level[levelCheck]; ++p)
					{
						std::string filename = "./sstables/level0/sstable" + std::to_string(p) + ".dat";
						char *charFileName = (char *)filename.data();
						std::ifstream curtable(charFileName, std::ios::in | std::ios::binary);
						while (!curtable.eof()) //一口气读完
						{
							curtable.read((char *)(&tmptime), sizeof(uint64_t));
							curtable.read((char *)(&tmpkey), sizeof(uint64_t));
							int valsize = index[tmpkey].length;
							curtable.read((char *)(&chartmpval), valsize);
							tmpval = chartmpval;
							TableNode tmpnode(tmptime, tmpkey, tmpval);
							files[p - (level[levelCheck] - 3)].push_back(tmpnode);
						}
						curtable.close();
						std::remove(charFileName); //读完就把文件删掉
					}
					//将下一层里面范围重合的sstable读到内存里
					for (int p = nextstart; p <= nextend; ++p)
					{
						std::string filename = "./sstables/level" +
											   std::to_string(levelCheck + 1) +
											   "/sstable" + std::to_string(p) + ".dat";

						char *charFileName = (char *)filename.data();
						std::ifstream curtable(charFileName, std::ios::in | std::ios::binary);
						while (!curtable.eof()) //一口气读完
						{
							curtable.read((char *)(&tmptime), sizeof(uint64_t));
							curtable.read((char *)(&tmpkey), sizeof(uint64_t));
							int valsize = index[tmpkey].length;
							curtable.read((char *)(&chartmpval), valsize);
							tmpval = chartmpval;
							TableNode tmpnode(tmptime, tmpkey, tmpval);
							files[p + 3].push_back(tmpnode);
						}
						curtable.close();
						std::remove(charFileName); //读完就把文件删掉
						//为每一个数组添加MAX作为key的尾巴，方便之后归并
						for (int i = 0; i < files.size(); ++i)
						{
							TableNode tailNode(0, MAX, "");
							files[i].push_back(tailNode);
						}

						std::vector<int> mergeover; //保存每个数组MAX的下标
						std::vector<int> curnum;	//保存当前 每个数组应该参与归并的下标
						for (int p = 0; p < files.size(); ++p)
						{
							std::vector<TableNode> tmp = files[p];
							mergeover.push_back(tmp.size() - 1); //此处要-1，因为是下标
						}
						curnum.resize(mergeover.size());
						for (int i = 0; i < curnum.size(); ++i)
						{
							curnum[i] = 0;
						}

						//开始归并
						while (mergeover != curnum) //每一次循环加入一个node到mergevec里面
						{
							std::vector<uint64_t> minkeyvec;
							for (int i = 0; i < files.size(); ++i) //获得每个数组的最小key存放在minkey里
							{
								int tmp = curnum[i];
								TableNode tmpnode = files[i][tmp];
								minkeyvec.push_back(tmpnode.key);
							}

							std::vector<uint64_t>::iterator smallest = std::max_element(std::begin(minkeyvec), std::end(minkeyvec));
							int distance = std::distance(std::begin(minkeyvec), smallest); //distance是最小key的下标
							TableNode minNode = files[distance][curnum[distance]];
							mergevec.push_back(minNode);
							curnum[distance]++;
						}

						//到这里已经归并完成，所有键值对都在mergevec里面了
						//去除重复
						remdup(mergevec);
						//处理删除
						handleDel(mergevec);
						//重新划分文件并写入下一层
						//首先把scan不重合的后边的sstable更名，腾出位置来
						for (int i = nextend + 1; i < level[levelCheck + 1]; ++i)
						{
							std::string oldname = "./sstables/level" + std::to_string(levelCheck + 1) +
												  "/sstable" + std::to_string(i) + ".dat";
							char *charold = (char *)oldname.data();
							std::string newname = "./sstables/level" + std::to_string(levelCheck + 1) +
												  "/sstable" + std::to_string(i + 1) + ".dat";
							char *charnew = (char *)newname.data();
							std::rename(charold, charnew); //完成更名
						}

						//写sstable
						for (int i = nextstart; i <= nextend + 1; ++i)
						{
							std::string newtablename = "./sstables/level" + std::to_string(levelCheck + 1) +
													   "/sstable" + std::to_string(i) + ".dat";
							std::ofstream sstable(newtablename, std::ios::app | std::ios::binary);
							//每个新的sstable中键值对的个数
							int newNum = mergevec.size() / (nextend - nextstart + 2);
							uint64_t lo = mergevec[(i - nextstart) * newNum].key;		  //存放较小的范围
							uint64_t hi = mergevec[(i - nextstart + 1) * newNum - 1].key; //存放较大的范围
							int curoffset = 0;
							int curdirlevel = 0;
							int curfilenum = 0;
							for (int p = (i - nextstart) * newNum; p < (i - nextstart + 1) * newNum; ++p)
							{
								TableNode node = mergevec[p];
								sstable.write((char *)(&(node.time)), sizeof(uint64_t));
								sstable.write((char *)(&(node.key)), sizeof(uint64_t));
								char *charval = (char *)node.value.data();
								int valsize = node.value.size() + 1;
								sstable.write((char *)(&charval), valsize);
								//更新索引
								curdirlevel = levelCheck + 1;
								curfilenum = i;
								IndexNode curIndexNode(curoffset, curdirlevel, curfilenum, node.time, valsize);
								index[node.key] = curIndexNode;
								curoffset = curoffset + sizeof(uint64_t) + sizeof(uint64_t) + valsize;
							}
							sstable.close();
							//更新范围
							ScanNode newscan(lo, hi);
							scan[levelCheck + 1][i] = newscan;
							//处理一下尾巴（不能整除的情况）
							if (i == nextend)
							{
								std::ofstream sstable(newtablename, std::ios::app | std::ios::binary);
								hi = mergevec.back().key;
								for (int p = (i - nextstart + 1) * newNum; p < mergevec.size(); ++p)
								{
									TableNode node = mergevec[p];
									sstable.write((char *)(&(node.time)), sizeof(uint64_t));
									sstable.write((char *)(&(node.key)), sizeof(uint64_t));
									char *charval = (char *)node.value.data();
									int valsize = node.value.size() + 1;
									sstable.write((char *)(&charval), valsize);
									//更新索引
									curdirlevel = levelCheck + 1;
									curfilenum = i;
									IndexNode curIndexNode(curoffset, curdirlevel, curfilenum, node.time, valsize);
									index[node.key] = curIndexNode;
									curoffset = curoffset + sizeof(uint64_t) + sizeof(uint64_t) + valsize;
								}
								sstable.close();
								//更新范围
								ScanNode newscan(lo, hi);
								scan[levelCheck][i] = newscan;
								//更新level
								level[levelCheck + 1]++;
							}
						}
					}
				}
			}
		}
	}
}