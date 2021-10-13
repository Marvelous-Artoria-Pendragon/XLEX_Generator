// Generator.h
#ifndef GENERATOR_H
#define GENERATOR_H
#include <iostream>
#include <string.h>
#include <iomanip>
#include <string.h>
#define MAX_STATE 100
#define MAX_REGEX_LEN 200
using namespace std;

/*
struct RegexGrammaException: public exception              // 非法表达式错误
{
    int error;                      // 错误类型。0：方括号内容为空；1：连续多个运算符；2：缺乏操作数；3：括号不匹配；
    char **descrption;
    RegexGrammaException(int e): error(e){};
    {

    }
    const char *what() const throw()
    {return "Invalid regex!";}
};
*/
struct NFANode                     // NFA结点
{
    int ch;                        // 字符的ASICII码，转义字符为负数
    NFANode(int c = -1): ch(c){}
};

struct DFANode                      // DFA结点
{
    int state;                      // 状态号
    int condset[MAX_STATE];         // 状态集合
    int cnt;                        // 当前集合状态数
    bool terminal;                  // 是否含终止态
    DFANode(int s = 0, int cnt = 0, bool end = false): state(s), cnt(cnt), terminal(end) 
        {for (int i = 0; i < MAX_STATE; i++) condset[i] = 0;}
    bool insert(int c);              // 将一个状态加入集合
    DFANode &operator=(const DFANode &p);           // 重载赋值函数 
    friend ostream &operator << (ostream &output, const DFANode &n);
};

struct TSTNode                      // 三叉树结点
{
    int data;
    TSTNode *child[3];              // 左子女、右子女
    TSTNode(int d = -1): data(d) {child[0] = child[1] = child[2] = NULL;}
};

class XLEX_Generator
{
    public:
        XLEX_Generator(int maxstate = MAX_STATE, int maxregex = MAX_REGEX_LEN);
        ~XLEX_Generator();
        bool add_ch(const char &ch, bool escape = false);        // 对unique_ch数组添加唯一字符
        void getRegex();                                         // 读取输入的正则表达式
        void displayNFAGraph();                                  // 显示NFA图
        void displayDFAGraph();                                  // 显示NFA图
        void displaystatustable();                               // 显示状态转移表（未最小化）
        void displayminimizedtable();                            // 显示最小化后的DFA图
        void toNFA();                                            // 根据正则表达式转换NFA
        void toDFA();                                            // 根据NFA转换为DFA
        void DFAminimize();                                      // DFA图最小化
        void generateCpp();
    private:
        NFANode **NFAgraph;             // NFA图
        DFANode **DFAgraph;             // DFA图
        DFANode **statustable;          // 状态转移表
        int unique_ch[MAX_STATE];       // 表达式中出现的所有唯一字符对应的ASCII码，转义字符为负
        int unique_cnt = 0;             // 当前所有唯一字符数
        int end_state;                  // NFA生成的最后一个状态
        int max_state;                  // 最大状态数
        int max_regex_len;              // 最长正则表达式长度
        int table_rows;                 // 状态转移表集合数
        int *state_map;                 // 最小化后的状态映射数组
        char *regex;                    // 当前处理的正则表达式
        void NFA(int &k, int &current_state, int &end_state, int recur_state);      // 递归生成NFA
        void DFA(int start_state, int &row, bool &terminal, bool *visited);         // 递归寻找某个状态的闭包
        bool convertBracket();          // 将方括号表达式转换为选择
};
#endif