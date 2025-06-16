#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdio>

using namespace std;

class SymbolTable
{
private:
	template <typename T>
	void NewSymbol(vector<T> list, T& value)
	{
		if (find(list.begin(), list.end(), value) != list.end())
			return;
		list.push_back(value);
	}
public:
	vector<string> identifiers;
	vector<string> keywords;
	vector<string> operators;
	vector<string> punctuations;
	vector<int> labels;
	vector<int> Constants;

	void NewIdentifier(string &identifier)
	{
		NewSymbol(identifiers, identifier);
	}

	void NewLabel(int label)
	{
		NewSymbol(labels, label);
	}

	void NewConstant(int constant)
	{
		NewSymbol(Constants, constant);
	}
} g_st;

enum class WordType {
	keyword,
	operatornum,
	comparison,
	interpunction
};

bool search(string &searchstr, WordType word)
{
	vector<string_view> keyword = {"IF","THEN","ELSE","GOTO"};
	vector<string_view> operatornum = { "+","-","*","/" };
	vector<string_view> comparison = { ">","<",">=","<=","=","<>" };
	vector<string_view> interpunction = { ",",":","(",")" };
    switch(word)
	{
    case WordType::keyword:
		return find(keyword.begin(), keyword.end(), searchstr) != keyword.end();
    case WordType::operatornum:
		return find(operatornum.begin(), operatornum.end(), searchstr) != operatornum.end();
    case WordType::comparison:
		return find(comparison.begin(), comparison.end(), searchstr) != comparison.end();
    case WordType::interpunction:
		return find(interpunction.begin(), interpunction.end(), searchstr) != interpunction.end();
	}
    return false;
}


char LetterProc (char ch, std::istream &is, std::ostream& os)
{
	string letter, temp;
    while(isalnum(ch))
	{
        letter += ch;
		is.get(ch);
	}
    if(search(letter,WordType::keyword))
	{
		os << "(K," << letter  << ")\n";
		cout << "(K," << letter << ")\n";
		if(letter == "GOTO")
		{
			is.get(ch);
            while(isdigit(ch))
			{
                temp += ch;
				is.get(ch);
			}
			os << "(L," << temp << ")\n";
		    cout << "(L,"<< temp <<")\n";
			g_st.NewLabel(stoi(temp));
		}
	}
    else
	{
		os << "(I," << letter << ")\n";
		cout << "(I," << letter << ")\n";
		g_st.NewIdentifier(letter);
	}
    return ch;
}

char NumProc(char ch, istream &is, ostream &os)
{
	string num;
    while(isdigit(ch)!=0)
	{
        num += ch;
        is.get(ch);
	}

    if(isalpha(ch))
	{
        while(isalpha(ch))
		{
            num += ch;
			is.get(ch);
		}
        cerr << "非法标识符: " << num << "\n";
	}

    if(ch == ':')
	{
		os << "(L," << num << ")\n";
		cout << "(L," << num << ")\n";
		g_st.NewLabel(stoi(num));
	}
    else
	{
		os << "(C," << num << ")\n";
		cout << "(C," << num << ")\n";
		g_st.NewConstant(stoi(num));
	}
    return ch;
}

char OtherProc(char ch, istream &is, ostream &os)
{
    string other;
    if(ch == '(')
	{
		other +=ch;
		is.get(ch);
	}
    while(isalnum(ch) == 0 && ch != ' ' && ch != '(' && ch != ')')
	{
        other +=ch;
		is.get(ch);
	}
	if(ch == ')')
	{
		other +=ch;
		is.get(ch);
	}

    if(search(other,WordType::interpunction))
	{
		os << "(P," << other << ")\n";
		cout << "(P," << other << ")\n";
	}
    else if(search(other,WordType::operatornum)||search(other,WordType::comparison))
	{
		os << "(O,"<< other << ")\n";
		cout << "(O," << other << ")\n";
	}
    else
	{
		cerr << "非法字符: " << other << "\n";
	}

    return ch;
}

char SpaceProc(std::istream &is, char ch)
{
	while (ch == ' ')
		is.get(ch);
	return ch;
}

/**
 * @brief 预处理
 * @param buf 
 */
void PreProc(char *buf, istream &source_file)
{
	int i=0;
	char old_ch='\0',cur_ch;
	bool comment=false;
	while(source_file.read(&cur_ch,sizeof(char)))
	{
		switch(comment)
		{
		case false:
			if(old_ch=='/'&&cur_ch=='*')
			{
				i--;
				comment=true;
			}
			else
			{
				if(old_ch=='\\'&&cur_ch=='\n')
					i--;
				else
				{
					if(cur_ch=='\t'||cur_ch=='\r'||cur_ch=='\n')
						cur_ch=' ';
					if(old_ch==' '&&cur_ch==' ')
						i--;
					buf[i++]=cur_ch;
				}
			}
			break;
		case true:
			if(old_ch=='*'&&cur_ch=='/')
				comment=false;
		}
		old_ch=cur_ch;
	}
	buf[i++] = '#';
}

int main ()
{
	char buf[4048]={'\0'};
	string filename;
	cout << "请输入源文件路径及名称:";
	cin >> filename;
	cout << 
	"**********************************词法分析程序**********************************\n"
	"K:关键字 I:标识符 C:常数 O:运算符 P:界符 L:标号" << endl;

	// 预处理 source.txt --> 预处理.txt
	std::ifstream source_file(filename + ".txt");
	if (!source_file)
	{
		cerr << "源文件无法打开!";
		return -1;
	}
	PreProc(buf, source_file);
	source_file.close();

	std::ofstream pre_file("预处理.txt");
	if (!pre_file) {
		std::cerr << "无法打开文件!" << std::endl;
		return -1;
	}
	std::copy(buf, buf + strlen(buf), std::ostream_iterator<char>(pre_file));
	pre_file.close();
	

	// 词法分析 预处理.txt --> 二元式表.txt
	std::ifstream pre_file2("预处理.txt");
	std::ofstream dual_file("二元式表.txt");
	if (!pre_file2 || !dual_file)
	{
		std::cerr << "无法打开文件!" << std::endl;
		return -1;
	}
	char ch;
	pre_file2.get(ch);
	while ('#' != ch)
	{
		ch = SpaceProc(pre_file2, ch);
		if (std::isalpha(ch))
			ch = LetterProc(ch, pre_file2, dual_file);
		else
		{
			if (std::isdigit(ch))
				ch = NumProc(ch, pre_file2, dual_file);
			else
				ch = OtherProc(ch, pre_file2, dual_file);
		}
	}
	dual_file.close();
	pre_file.close();

	std::cout << "输出结果保存在二元式表.txt文件中,请打开查看。\n词法分析结束!\n";
	return 0;
}
