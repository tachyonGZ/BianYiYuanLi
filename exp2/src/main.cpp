//实验二  语法分析

//  算术表达式->项|算术表达式+项|算术表达式-项
//  项->因子|项*因子|项/因子
//  因子->变量|常数|(算术表达式)    
// 
//          算符优先关系表    
//---------------------------------- 
//     +   -   *   /   (   )   i   #  
// +   >   >   <   <   <   >   <   >  
// -   >   >   <   <   <   >   <   >  
// *   >   >   >   >   <   >   <   >  
// /   >   >   >   >   <   >   <   >  
// (   <   <   <   <   <   =   <   ?  
// )   >   >   >   >   ?   >   ?   >  
// i   >   >   >   >   ?   >   ?   >   
// #   <   <   <   <   <   ?   <   =  
//---------------------------------- 

#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <map>

using namespace std;

/**
 * @brief 待规约字符串
 */
class Reduction
{
public:
	string impl_;
	//可归约字符串
	vector<string> words_{ 
		"N+N","N-N","N*N","N/N",")N(","i"
	};

	Reduction(const string &s) : impl_(s)
	{
		;
	}

	char Reduct()
	{
		for (auto &word : words_)
		{
			if (word == impl_)
			{
				return 'N';
			}
		}
	}
};

/**
 * @brief 算符优先表
 */
class PriorityTable
{
public:
	//二维数组定义字符之间的优先关系(1表示>,-1表示<,0表示=,-2表示错误)
	static vector<vector<int>> M_;
	static map<char, int> ch_di_;

	int compare(char a, char b)
	{
		int cmp = M_[ch_di_[a]][ch_di_[b]];
		if (-2 == cmp)
		{
			throw std::runtime_error("无法比较两个字符的优先关系");
		}

		return cmp;
	}

	void error(char ch)
	{
		if (ch_di_[ch] == 5 || ch_di_[ch] == 6)
		{
			printf("\n错误e2:缺少运算符!");
		}
		else if (ch_di_[ch] == 4)
		{
			printf("\n错误e1:非法左括号!");
		}
		else
			printf("\n错误e3:非法右括号!");
	}
};

vector<vector<int>> PriorityTable::M_{
	{1,1,-1,-1,-1,1,-1,1},
	{1,1,-1,-1,-1,1,-1,1},
	{1,1,1,1,-1,1,-1,1},
	{1,1,1,1,-1,1,-1,1},
	{-1,-1,-1,-1,-1,0,-1,-2},
	{1,1,1,1,-2,1,-2,1},
	{1,1,1,1,-2,1,-2,1},
	{-1,-1,-1,-1,-1,-2,-1,0}
};

std::map<char, int> PriorityTable::ch_di_{
	{'+', 0},
	{'-', 1},
	{'*', 2},
	{'/', 3},
	{'(', 4},
	{')', 5},
	{'i', 6},
	{'#', 7},
};
/**
 * @brief 输入字符串
 */
class InputString
{
public:
	string impl_;
	size_t k = 0;

	InputString(string &oldstr)
	{
		bool is_in_seq = false;
		for (auto ch : oldstr)
		{
			if (isalnum(ch))
			{
				if (is_in_seq)
				{
					continue;
				}

				impl_ += 'i';
				is_in_seq = true;
			}
			else
			{
				impl_ += ch;
				is_in_seq = false;
			}
		}
		impl_  += '#';
	}

	bool IsVT(char ch)
	{
		return ch != 'N';
	}

	char ReadVT()
	{
		char a;
		if (IsVT(impl_[k]))
		{
			a = impl_[k];
		}

		k++;
		return a;
	}

	friend std::ostream& operator<<(std::ostream& os, const InputString& istr);
};

std::ostream& operator<<(std::ostream& os, const InputString& istr)
{
	os << istr.impl_.substr(istr.k, string::npos);
	return os;
}

/**
 * @brief 符号栈
 */
class VStack
{
public:
	string impl_;

	VStack(const string &s) : impl_(s)
	{
		;
	}

	void push(char ch)
	{
		impl_.push_back(ch);
	}

	char Pop()
	{
		char ch = '\0';
		if (!impl_.empty())
		{
			ch = impl_.back();
			impl_.pop_back();
		}
		return ch;
	}

	string SuccessivePopUntil(size_t index)
	{
		string res;
		while ((TopIndex() - index) != 0)
		{
			res += Pop();
		}

		return res;
	}

	char Top()
	{
		return impl_.back();
	}

	size_t TopIndex()
	{
		return impl_.size() - 1;
	}

	char& operator[](size_t index)
	{
		if (index >= impl_.size())
		{
			throw std::out_of_range("Index out of range");
		}
		return impl_[index];
	}

	bool operator!=(const string &other)
	{
		return impl_ != other;
	}

	friend std::ostream& operator<<(std::ostream&, const VStack&);
};

std::ostream& operator<<(std::ostream& os, const VStack& vs)
{
	os << vs.impl_;
	return os;
}

class StepRecorder
{
public:
	enum class StepType {
		Move,
		Reduct
	};

	int step = 1;

	vector<int> ReductSteps;

	void Record(StepType st, int cmp, char a, InputString& istr, VStack& vs)
	{
		cout << "\n " << step++ << setw(10);

		cout << vs << setw(10);

		static map<int, string> relationship{
			{-1, "<"},
			{0, "="},
			{1, ">"},
		};
		cout << relationship[cmp] << setw(8);

		cout << a << setw(10);

		cout << istr << setw(10);

		if (st == StepType::Reduct)
		{
			cout << "归约" << setw(8);
			ReductSteps.push_back(step - 1);
		}
		else
			cout << "移进" << setw(8);
	}
};

void PriorAnalysis(StepRecorder &sr, InputString &istr)
{
	PriorityTable pt;
	VStack vs("#");
	sr.Record(StepRecorder::StepType::Move, -1, '\0', istr, vs);
	char a;
	do
	{
		a = istr.ReadVT();
		size_t j = vs.TopIndex();
		if (!istr.IsVT(vs.Top()))
			j -= 1;

		while (pt.compare(vs[j], a) > 0 && (vs != "#N"))
		{
			char q;
			do
			{
				q = vs[j];
				if (istr.IsVT(vs[j - 1]))
					j -= 1;
				else
					j -= 2;
			} while (pt.compare(vs[j], q) >= 0);

			// 规约
			Reduction r(vs.SuccessivePopUntil(j));
			vs.push(r.Reduct());
			sr.Record(StepRecorder::StepType::Reduct, 1, a, istr, vs);
		}

		int cmp = pt.compare(vs[j], a);
		if (cmp <= 0)
		{
			vs.push(a);
			sr.Record(StepRecorder::StepType::Move, cmp, a, istr, vs);
			if (vs.Top() != '#')
				continue;
		}
		else
		{
			pt.error(vs[j]);
			a = '#';
		}
	} while (a != '#');
}

int main()
{
	cout << "******************************算符优先语法分析程序******************************" << endl;
	cout << " E->E+T|E-T|T" << endl;
	cout << " T->T*F|T/F|F" << endl;
	cout << " F->(E)|i" << endl;
	cout << " E表示算术表达式.T表示项.F表示因子.i表示变量或常数." << endl;
	cout << "              优先表" << endl;
	cout << "     +   -   *   /   (   )   i   #" << endl;
	cout << " +   >   >   <   <   <   >   <   >" << endl;
	cout << " -   >   >   <   <   <   >   <   >" << endl;
	cout << " *   >   >   >   >   <   >   <   >" << endl;
	cout << " /   >   >   >   >   <   >   <   >" << endl;
	cout << " (   <   <   <   <   <   =   <   e1" << endl;
	cout << " )   >   >   >   >   e2  >   e2  >" << endl;
	cout << " i   >   >   >   >   e2  >   e2  >" << endl;
	cout << " #   <   <   <   <   <   e3  <   =" << endl;

	std::ifstream pre_file("预处理.txt");
	if (!pre_file)
	{
		cerr << "请先将实验1文件夹中的预处理.txt文件复制到实验2文件夹中!" << endl;
		return -1;
	}

	char ch;
	char ch1;
	pre_file.get(ch);

	
	int id = 1;
	while (ch != '#')
	{
		int i = -1, j = 0, m = -1;
		while (ch != '=' && ch != '#')
		{
			ch1 = ch;
			pre_file.get(ch);
			if ((ch1 == '>' || ch1 == '<') && ch == '=')
				pre_file.get(ch);
		}
		if (ch == '#')
		{
			cout << "算符优先语法分析结束!" << endl;
			pre_file.close();
			return 0;
		}

		string oldstr;
		while (ch != ' ' && ch != '#')
		{
			pre_file.get(ch);
			oldstr += ch;
		}
		oldstr.pop_back();

		InputString istr(oldstr);

		StepRecorder sr;

		cout << "算术表达式" << id << "为: " << oldstr << endl;
		cout << "转换为输入串: " << istr << endl;
		cout << " 步骤号 符号栈 优先关系 当前分析符 剩余输入串 动作";
		PriorAnalysis(sr, istr);
		cout << endl;
		cout << "算术表达式" << id++ << "的" << "归约产生式步骤号为: ";
		copy(
			sr.ReductSteps.begin(),
			sr.ReductSteps.end(),
			ostream_iterator<int>(cout, " ")
		);
		cout << endl;
	}
	cout << "算符优先语法分析结束!" << endl;
	return 0;
}
