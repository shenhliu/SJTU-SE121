#include "common.h"

using namespace std;
void Gauss(vector<vector<double>> &A, vector<double> &b, vector<double> &x);
//You should only code here.Don't edit any other files in this
int func1(int amount, vector<int> &coins)
{
	int answer = 0;
	int N = coins.size(); //不同面额的硬币
	int M = amount;		  //总金额

	// int N = 5;
	// int M = 2;

	int dp[N + 1][M + 1];
	for (int i = 0; i <= M; ++i)
	{
		dp[0][i] = 0;
	}
	for (int i = 0; i <= N; ++i)
	{
		dp[i][0] = 1;
	}
	for (int i = 1; i <= N; ++i)
	{
		for (int j = 1; j <= M; ++j)
		{
			if (j - coins[i - 1] >= 0)
			{
				dp[i][j] = dp[i][j - coins[i - 1]] + dp[i - 1][j];
				//cout << dp[i][j] << endl;
			}
			else
			{
				dp[i][j] = dp[i - 1][j];
				//cout << dp[i][j] << endl;
			}
		}
	}
	answer = dp[N][M];
	// cout << answer << endl;
	return answer;
}

int func2(int amount, vector<vector<int>> &conquer)
{
	int count = 0;
	int meet[amount][amount];
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//数组一定要赋初始值。不然会出现奇奇怪怪的错误
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	for (int i = 0; i < amount; ++i)
	{
		for (int j = 0; j < amount; ++j)
		{
			meet[i][j] = 0;
		}
	}
	for (int i = 0; i < amount; ++i)
	{
		meet[i][(i + 1) % amount] = 1;
		//meet[(i + 1) % amount][i % amount] = 1;
	}

	// for (int i = 0; i < amount; ++i)
	// {
	// 	for (int j = 0; j < amount; ++j)
	// 	{
	// 		cout << meet[i][j] << " ";
	// 	}
	// 	cout << endl;
	// }
	// cout << endl;

	// 这个写法的问题在于，在填meet[i][j]的时候，meet [k][j]还没有判断过，不能直接用
	// for (int i = 0; i < amount; ++i)
	// {
	// 	for (int j = i + 2; j <= (amount + i); ++j)
	// 	{
	// 		for (int k = i + 1; k < j; ++k)
	// 		{
	// 			if (meet[i][k % amount] == 1 &&
	// 				meet[k % amount][j % amount] == 1 &&
	// 				(conquer[i][k % amount] == 1 || conquer[j % amount][k % amount] == 1))
	// 			{
	// 				meet[i][j % amount] = 1;
	// 			}
	// 		}
	// 	}
	// }

	//利用偏移量在最外层循环，可以避免上面的问题：偏移量为3的时候，夹在中间的一定是偏移量为1或者2的，之前已经填过表了

	for (int d = 2; d <= amount; d++)
	{
		for (int i = 0; i < amount; i++)
		{
			int j = i + d;
			for (int k = i + 1; k < j; k++)
			{
				if (meet[i][k % amount] && meet[k % amount][j % amount] && (conquer[i][k % amount] || conquer[j % amount][k % amount]))
				{
					meet[i][j % amount] = 1;
					break;
				}
			}
		}
	}

	for (int i = 0; i < amount; ++i)
	{
		if (meet[i][i] == 1)
		{
			count++;
		}
	}
	//cout << count << endl;
	// for (int i = 0; i < amount; ++i)
	// {
	// 	for (int j = 0; j < amount; ++j)
	// 	{
	// 		cout << meet[i][j] << " ";
	// 	}
	// 	cout << endl;
	// }
	// cout << endl;
	return count;
}

double func3(int n, int hp, vector<int> &damage, vector<int> &edges)
{
	//初始化边，转化为关系矩阵
	int edgeNum = edges.size() / 2;

	//初始化图中点的度
	int degree[n];
	for (int i = 0; i < n; ++i)
	{
		degree[i] = 0;
	}

	int relation[n][n];
	for (int i = 0; i < n + 1; ++i)
	{
		for (int j = 0; j < n + 1; ++j)
		{
			relation[i][j] = 0;
		}
	}

	for (int i = 0; i < edgeNum; i++)
	{
		int start = edges[i * 2] - 1;
		int end = edges[i * 2 + 1] - 1;
		relation[start][end] = 1;
		relation[end][start] = 1;
		degree[start]++;
		degree[end]++;
	}

	//把有伤害的点和没伤害的点分开
	vector<int> DamP;	//有伤害的点
	vector<int> noDamP; //没有伤害的点
	for (int i = 0; i < n; ++i)
	{
		if (damage[i] == 0)
		{
			noDamP.push_back(i);
		}
		else
		{
			DamP.push_back(i);
		}
	}
	int DamNum = DamP.size();
	int noDamNum = noDamP.size();

	// for (int i = 1; i < n + 1; ++i)
	// {
	// 	cout << degree[i] << endl;
	// }

	vector<vector<double>> A; //没有伤害点的系数矩阵
	A.resize(noDamNum);
	for (int i = 0; i < noDamNum; ++i)
	{
		A[i].resize(noDamNum);
	}

	for (int i = 0; i < noDamNum; ++i)
	{
		for (int j = 0; j < noDamNum; ++j)
		{
			A[i][j] = 0;
		}
	}

	vector<double> b; //常数向量
	b.resize(noDamNum);
	for (int i = 0; i < noDamNum; ++i)
	{
		b[i] = 0;
	}

	vector<double> x; //解向量
	x.resize(noDamNum);
	for (int i = 0; i < noDamNum; ++i)
	{
		b[i] = 0;
	}

	vector<vector<double>> f;
	f.resize(hp + 1);
	for (int i = 0; i < (int)f.size(); ++i)
	{
		f[i].resize(n);
	}
	for (int i = 0; i < hp + 1; ++i)
	{
		for (int j = 0; j < n; ++j)
		{
			f[i][j] = 0;
		}
	}

	for (int i = hp; i > 0; --i)
	{
		//处理有伤害的点
		for (int j = 0; j < DamNum; ++j)
		{
			int curp = DamP[j];
			int prehp = i + damage[curp];
			if (prehp > hp) //如果伤害加现在血量超过了hp，那么不可能
			{
				f[i][curp] = 0;
			}
			else
			{
				double ans = 0;
				for (int k = 0; k < n - 1; k++) //注意是n-1，最后一个点不为其他点做出贡献
				{
					ans = ans + (double)relation[curp][k] * f[prehp][k] / degree[k];
				}
				f[i][curp] = ans;
			}
		}

		//处理无伤害的点
		for (int k = 0; k < noDamNum; ++k)
		{
			int curP = noDamP[k];
			for (int j = 0; j < noDamNum - 1; ++j)
			{
				int anotherP = noDamP[j];
				A[k][j] = 0 - (double)relation[curP][anotherP] / degree[anotherP];
				//cout << degree[anotherP] << endl;
				//cout << anotherP << endl;
			}

			if (k == 0 && i == hp)
			{
				b[k] = 1;
				//cout << "ok" << endl;
			}
			else
			{
				double sum = 0;
				for (int j = 0; j < DamNum; ++j)
				{
					int anotherP = DamP[j];
					sum = sum + (double)relation[curP][anotherP] / degree[anotherP] * f[i][anotherP];
				}
				b[k] = sum;
			}
			A[k][k] = 1;
		}
		// int test[noDamNum];
		// for (int i = 0; i < noDamNum; ++i)
		// {
		// 	test[i] = b[i];
		// }

		Gauss(A, b, x);
		for (int k = 0; k < noDamNum; ++k)
		{
			int anotherP = noDamP[k];
			f[i][anotherP] = x[k];
		}
	}

	// if (hp < 3)
	// {
	// 	for (int i = 0; i < hp + 1; ++i)
	// 	{
	// 		for (int j = 0; j < n; ++j)
	// 		{
	// 			cout << f[i][j] << " ";
	// 		}
	// 		cout << endl;
	// 	}
	// }

	double ans = 0;
	for (int i = 0; i <= hp; ++i)
	{
		ans += f[i][n - 1];
	}
	cout << ans << endl;
	return ans;
	//return -1;
}

void Gauss(vector<vector<double>> &vA, vector<double> &vb, vector<double> &vx)
{
	int n = vb.size();
	double temp[n + 1][n + 1];
	vx.resize(n);
	double A[n + 1][n + 1];
	double b[n + 1];
	double x[n + 1];
	for (int i = 0; i < n + 1; ++i)
	{
		for (int j = 0; j < n + 1; ++j)
		{
			temp[i][j] = 0;
			A[i][j] = 0;
		}
		b[i] = 0;
		x[i] = 0;
	}

	for (int i = 0; i < n; ++i)
	{
		for (int j = 0; j < n; ++j)
		{
			A[i + 1][j + 1] = vA[i][j];
		}
		b[i + 1] = vb[i];
	}

	for (int k = 1; k < n; k++)
	{
		for (int i = k + 1; i <= n; i++)
		{
			temp[i][k] = A[i][k] / A[k][k];
			for (int j = 1; j <= n; j++)
			{
				A[i][j] -= temp[i][k] * A[k][j];
			}
		}
		for (int i = k + 1; i <= n; i++)
		{
			b[i] -= temp[i][k] * b[k];
		}
	}

	// 回代
	x[n] = b[n] / A[n][n];
	for (int i = n - 1; i >= 1; i--)
	{
		x[i] = b[i];
		for (int j = i + 1; j <= n; j++)
		{
			x[i] -= A[i][j] * x[j];
		}
		x[i] /= A[i][i];
	}

	for (int i = 0; i < n; ++i)
	{
		vx[i] = x[i + 1];
	}
}