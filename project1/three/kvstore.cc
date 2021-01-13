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
		while (node != list.bottomLast->succ)
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

		if (level[0] < 3) //第一层不需要compaction的时候
		{
			int fileNum = level[0] - 1; //注意，文件标号是文件数目-1
			std::string sstableName = "./sstables/level0/sstable" + std::to_string(fileNum) + ".dat";
			char *charsstableName = (char *)sstableName.data();				  //转成char*才能写，我也不知道为啥
			std::rename("./sstables/level0/newsstable.dat", charsstableName); //文件从newsstable改成文件标号
		}
		else
		{
			compaction(level, index);
		}
	}
}
/**
 * Returns the (string) value of the given key.
 * An empty string indicates not found.
 */
std::string KVStore::get(uint64_t key)
{
	return list.get(key);
}
/**
 * Delete the given key-value pair if it exists.
 * Returns false iff the key is not found.
 */
bool KVStore::del(uint64_t key)
{
	//如果插入到文件中，插入"`"（键盘左上角的那个键）视为删除标记
	return list.remove(key);
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

void KVStore::remdup(vector<TableNode> &vec)
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
				vec.erase(vec.begin + i + 1);
				i--;
				continue;
			}
		}
	}
}

void KVStore::handleDel(vector<TableNode> &vec)
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

void KVStore::compaction(vector<int> &level, map<int, IndexNode> &index)
{
	//如果是level0的compaction
	if (level.size() == 1)
	{
		if (!fs::exists("sstables/level1")) //如果level1不存在,创建level1目录
		{
			fs::create_directories("sstables/level1");
		}
		vector<TableNode> file0; //存放第0个文件中的键值对
		vector<TableNode> file1;
		vector<TableNode> file2;
		vector<TableNode> mergevec; //存放归并完成后的键值对
		uint64_t tmptime;
		uint64_t tmpkey;
		string tmpval;
		char *chartmpval;
		int valnum0 = 0; //记录第0个sstable里面的键值对数目
		int valnum1 = 0;
		int valnum2 = 0;
		std::ifstream sstable0("sstables/level0/sstable0.dat", std::ios::in | std::ios::binary);
		while (!sstable0.eof()) //一口气读完
		{
			sstable0.read((char *)(&tmptime), sizeof(uint64_t));
			sstable0.read((char *)(&tmpkey), sizeof(uint64_t));
			int valsize = index[tmpkey].length;
			sstable0.read((char *)(&chartmpval), valsize);
			tmpval = chartmpval;
			TableNode tmpnode(tmptime, tmpkey, tmpval);
			file0.push_back(tmpnode);
		}
		sstable0.close();

		std::ifstream sstable1("sstables/level0/sstable1.dat", std::ios::in | std::ios::binary);
		while (!sstable1.eof()) //一口气读完
		{
			sstable1.read((char *)(&tmptime), sizeof(uint64_t));
			sstable1.read((char *)(&tmpkey), sizeof(uint64_t));
			int valsize = index[tmpkey].length;
			sstable1.read((char *)(&chartmpval), valsize);
			tmpval = chartmpval;
			TableNode tmpnode(tmptime, tmpkey, tmpval);
			file1.push_back(tmpnode);
		}
		sstable1.close();

		std::ifstream sstable2("sstables/level0/sstable2.dat", std::ios::in | std::ios::binary);
		while (!sstable2.eof()) //一口气读完
		{
			sstable2.read((char *)(&tmptime), sizeof(uint64_t));
			sstable2.read((char *)(&tmpkey), sizeof(uint64_t));
			int valsize = index[tmpkey].length;
			sstable2.read((char *)(&chartmpval), valsize);
			tmpval = chartmpval;
			TableNode tmpnode(tmptime, tmpkey, tmpval);
			file2.push_back(tmpnode);
		}
		sstable2.close();

		TableNode tailNode(0, MAX, ""); //为每一个vector置一个尾部node，方便一会归并排序
		file0.push_back(tailNode);
		file1.push_back(tailNode);
		file2.push_back(tailNode);

		
		//归并排序：
		int i = 0;
		int j = 0;
		int k = 0;
		while (i < file0.size() || j < file1.size() || k < file2.size()) //这里先留着相等的情况
		{
			if (file0[i].key <= file1[j].key && file0[i].key <= file2[k].key)
			{
				mergevec.push_back(file0[i]);
				i++;
			}

			if (file1[j].key <= file0[i].key && file1[j].key <= file2[k].key)
			{
				mergevec.push_back(file1[j]);
				j++;
			}

			if (file2[k].key <= file0[i].key && file2[k].key <= file1[j].key)
			{
				mergevec.push_back(file2[k]);
				k++;
			}

			if (file0[i].key == MAX && file1[j].key == MAX && file2[k].key == MAX) //如果都到最后了
			{
				break;
			}
		}

		//到这里，第0层所有文件的所有键值对就都读到内存里了，存放在mergevec里面
		//对mergevec去除重复
		remdup(mergevec);

		//对mergevec处理删除，即查看是否有val = "`"的情况
		handleDel(mergevec);

		//第一层
	}

	int levelCheck = 0; //检查每一层目录,levelCheck是当前检查到的层数,level[k]是第k层现在有的文件数
	for (levelCheck = 0; levelCheck < level.size(); ++levelCheck)
	{
		if (level[levelCheck] > maxfile(levelCheck)) //如果有一层的文件数多于可以容纳的最多的文件数
		{
			if (level.size() == levelCheck + 1) //如果文件数超标的是最底层的文件目录，那么就新开文件目录放进去
			{
				level.push_back(0);
				string dirName = "sstables/level" + std::to_string(levelCheck + 1);
				fs::create_directories(dirName);			//新建文件目录
				for (int i = 0; i < level[levelCheck]; ++i) //把这一层的所有文件打开
				{
					std::string tmpFileName = "sstables/level" + std::to_string(levelCheck) + "/sstable" + std::to_string(i) + ".dat";
					char *charFileName = (char *)tmpFileName.data();
				}
			}
		}
	}
}