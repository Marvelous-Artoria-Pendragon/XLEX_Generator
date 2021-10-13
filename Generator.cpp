#include "Generator.h"

bool DFANode::insert(int c)
{
    if (cnt > MAX_STATE) return false; 
    for (int i = 0; i < cnt; i ++) if (c == condset[i]) return false;
    condset[cnt++] = c; return true;
}

DFANode &DFANode::operator =(const DFANode &p)           // 重载赋值运算符
{
    state = p.state; cnt = p.cnt;
    memcpy(condset, p.condset, sizeof(p.condset));
    terminal = p.terminal;
    return *this;
}

ostream &operator<<(ostream &output, const DFANode &n)
{
    output << "state: " << n.state << endl;
    output << "condset: ";
    for (int i = 0; i < n.cnt; i++) cout << n.condset[i] << " ";
    output << endl << "cnt: " << n.cnt << endl
           << "terminal: " << n.terminal << endl;
    return output;
}

XLEX_Generator::XLEX_Generator(int maxstate, int maxregex)
{
    unique_cnt = 0; end_state = 1; max_state = maxstate; max_regex_len = maxregex;
    NFAgraph = new NFANode*[max_state];
    regex = new char [max_regex_len]; memset(regex, 0, sizeof(regex[0]) * max_regex_len);
    for (int i = 0; i < MAX_STATE; i++)
        NFAgraph[i] = new NFANode[max_state];
    DFAgraph = statustable = NULL; state_map = NULL;
    table_rows = 0;
}

XLEX_Generator::~XLEX_Generator()
{
    for (int i = 0; i < MAX_STATE; i++)
        delete[] NFAgraph[i];
    delete[] regex;
    delete[] NFAgraph;
    for (int i = 0; i < end_state; i++) {delete[] statustable[i]; statustable[i] = NULL;}
    delete[] statustable;
    delete[] state_map;
}

bool XLEX_Generator::add_ch(const char &ch, bool escape)        // 对unique_ch数组添加唯一字符
{
    if (unique_cnt > MAX_STATE) exit(1);
    int t = int(ch); if (escape) t = -t;
    for (int i = 0; i < unique_cnt; i++)
        if (t == unique_ch[i]) return false;            // 已有记录，添加失败
    unique_ch[unique_cnt++] = t; return true;           // 添加成功
}

void XLEX_Generator::getRegex()              // 获取正则表达式，
{cin >> regex;}

void XLEX_Generator::displayNFAGraph()
{
    cout << setiosflags(ios::right) << setw(4) << " "; 
    for(int i = 0; i <= end_state; i++) 
        cout << setiosflags(ios::right) << setw(4) << i;
        cout << endl;
    for (int i = 0; i <= end_state; i++)
    {
        cout << setiosflags(ios::right) << setw(4) << i;
        for (int j = 0; j <= end_state; j++)
        {
            cout << setiosflags(ios::right) << setw(4);
            if (NFAgraph[i][j].ch == 0) cout << 0;
            else if (NFAgraph[i][j].ch == -1) cout << -1;
            else if (NFAgraph[i][j].ch < 0) cout << char(-NFAgraph[i][j].ch);
            else cout << char(NFAgraph[i][j].ch);
        } cout << endl;
    }
}

void XLEX_Generator::NFA(int &k, int &current_state, int &end_state, int recur_state)              // 递归生成
{
    // regex: 输入的正则表达式
    // k: 指示当前处理表达式的字符
    // graph: NFA图
    // currrent_state: 状态计数
    // end_state: 当前分支结束状态，未指定为-1
    // recur_state: 当前递归所在最初状态
    int group_state = recur_state, temp_state = recur_state, this_end = -1;                      // 当前表达式所在组group、组结束的下一个状态end
    NFAgraph[temp_state][current_state].ch = 0;              // 建立空串连接
    temp_state = current_state++;
    while (regex[k] != '\0')
    {
        switch (regex[k])
        {
            case '(': {
                group_state = temp_state; k++; 
                NFA(k, current_state, temp_state, temp_state);                   // 遇到新的表达式，递归进入，回溯时重置组结束状态到temp_state
                if (regex[k] == '*') 
                {
                    NFAgraph[temp_state][group_state].ch = 0;             // 表达式重复多次
                    NFAgraph[group_state][temp_state].ch = 0;             // 表达式出现0次
                    k++;
                } NFAgraph[temp_state][current_state].ch = 0; 
                temp_state = current_state++; break;
            }
            case '|': {                                                 // 多分支
                if (this_end == -1) this_end = current_state++;
                NFAgraph[temp_state][this_end].ch = 0;                      // 空串结束
                temp_state = recur_state; k++;
                NFAgraph[temp_state][current_state].ch = 0;              // 建立空串新分支
                temp_state = current_state++; break;
            }
            case ')': {
                if (this_end == -1) this_end = current_state++;
                NFAgraph[temp_state][this_end].ch = 0;        // 当前组结束，建立空串
                end_state = this_end;
                k++; return;
            }
            case '*': {                                 // 闭包
                NFAgraph[temp_state][group_state].ch = 0;             // 表达式重复多次
                NFAgraph[group_state][temp_state].ch = 0;             // 表达式出现0次
                k++; break;
            }
            case '?': {
                NFAgraph[group_state][temp_state].ch = 0;             // 表达式出现0次
                k++; break;
            }
            case '+': {
                NFAgraph[temp_state][group_state].ch = 0;             // 表达式出现至少一次
                k++; break;
            }
            default: {                                  // 原始字符，不包括匹配任意字符'.'
                group_state = temp_state;
                if (regex[k] == '\\' && (k++, regex[k] == 'd' || regex[k] == 'l'))  // 目前只支持字母：\l, 数字: \d
                    {NFAgraph[temp_state++][current_state++].ch = -int(regex[k]); add_ch(regex[k], true);}
                else {
                    NFAgraph[temp_state++][current_state++].ch = int(regex[k]);
                    add_ch(regex[k]); k++;
                } NFAgraph[temp_state++][current_state++].ch = 0;                 // 空串结束
            }
        }
    } 
    if (this_end != -1)                             // 考虑最外层表达式末尾可能无括号或其它结束符(选择)
    {
        NFAgraph[temp_state][this_end].ch = 0;        // 将最后一个选择表达式合并到空串
        temp_state = this_end;
    }
    NFAgraph[temp_state][current_state].ch = 0;       // 所有分支合并到空串结束
    end_state = current_state;
}
void XLEX_Generator::toNFA()
{
    getRegex();
    if (!convertBracket()) cout << "Invalid regex!";
    int current_state = 1, group = 0, k = 0;
    NFA(k, current_state, this->end_state, 0);
}

void XLEX_Generator::DFA(int start_state, int &row, bool &terminal, bool *visited)
{
    for (int i = 0; i <= end_state; i++)                     // 搜索当前状态集合的所有可转移状态
        if (NFAgraph[start_state][i].ch == -1 || visited[i]) continue;        // 无边或已访问跳过
           else if (NFAgraph[start_state][i].ch == 0)              // 仅有空串
        {
            statustable[row][0].insert(i); 
            if (i == end_state) terminal = true;
            visited[i] = true;                      // 标记已访问
            DFA(i, row, terminal, visited);                             // 深度优先搜索所有同状态
        } else {
            for (int col = 0; col < unique_cnt; col++)                  // 寻找对应的转移条件所在列
                if (unique_ch[col] == NFAgraph[start_state][i].ch)
                {
                    statustable[row][col + 1].insert(i); 
                    return;       // 标记非终态
                }
        }
}

void XLEX_Generator::toDFA()
{
    statustable = new DFANode*[MAX_STATE];                        // 从1开始，不包括列名(转移状态)，行名为状态集合，生成状态转移表
    DFANode start; int start_row = 0, state = 1;            // start: 第一个初始态结点；start_row:递归的初始状态; state:DFA状态计数
    int *visit = new int[end_state];                        // visit数组存储每个状态对应的在DFA表中的行号,无则为0(初始0状态默认不会查询)
    bool terminal = false, *visited = new bool[end_state + 1];  // visited 标记深搜访问
    for (int i = 0; i < end_state; i++) 
        {statustable[i] = new DFANode[unique_cnt + 1]; visit[i] = 0;}
    for (int i = 0; i <= end_state; i++) visited[i] = false;
    statustable[0][0].insert(0); 
    DFA(0, start_row, terminal, visited);
    int row = 1;
    for (int i = 0; i < row; i++)
        for (int j = 1; j <= unique_cnt; j++)
        {
            if (!statustable[i][j].cnt) continue;                  // 不存在转移状态
            // cout << statustable[i][j];
            if (!visit[statustable[i][j].condset[0]])             // 集合未重复
            {
                terminal = false;
                statustable[row][0] = statustable[i][j]; visit[statustable[row][0].condset[0]] = row;        // 新建行，并记录所在行号
                for (int t = 0; t < statustable[i][j].cnt; t++)
                {
                    for (int ii = 0; ii <= end_state; ii++) visited[ii] = false;             // 重置访问标记
                    DFA(statustable[i][j].condset[t], row, terminal, visited);
                }
                statustable[row][0].terminal = terminal; statustable[row][0].state = state++;
                statustable[i][j] = statustable[row++][0];              // 补充完集合
            } else {statustable[i][j] = statustable[visit[statustable[i][j].condset[0]]][0]; }        // 状态集合重复，直接拷贝
        } table_rows = row;
    delete[] visit;
}

void XLEX_Generator::displaystatustable()
{
    cout << endl << setiosflags(ios::right) << setw(4) << " "; 
    for(int i = 0; i < unique_cnt; i++) 
        cout << setiosflags(ios::right) << setw(4) << char(unique_ch[i]);
        cout << endl;
    for (int i = 0; i < table_rows; i++)
    {
        for (int j = 0; j <= unique_cnt; j++)
            cout << setiosflags(ios::right) << setw(4) << statustable[i][j].state;
        cout << endl;
    }
    for (int i = 0; i < table_rows; i++) cout << statustable[i][0] << endl;
}

void removeTST(TSTNode *p)
{
    for (int i = 0; i < 2; i++) if (p->child[i] != NULL) removeTST(p->child[i]);
    delete p; return;
}

void XLEX_Generator::DFAminimize()
{
    TSTNode *TST = new TSTNode(), *p = TST;
    state_map = new int[table_rows];               // 标记状态转换态所在行
    for (int i = 0; i < table_rows; i++)
    {
        state_map[i] = i; p = TST;
        for (int j = 0; j <= unique_cnt; j++)
        {
            int t = 2; 
            if (statustable[i][j].cnt) t = statustable[i][j].terminal;      // 转移状态不为空
            if (p->child[t] == NULL) p->child[t] = new TSTNode();
            p = p->child[t];                   // 新建结点
        } 
        if (p->data > -1) state_map[i] = p->data;                            // 状态重复
        else p->data = i;                                                   // 增加新状态
    } removeTST(TST);
}

void XLEX_Generator::displayminimizedtable()
{
    cout << endl << setiosflags(ios::right) << setw(4) << " "; 
    for(int i = 0; i < unique_cnt; i++) 
        cout << setiosflags(ios::right) << setw(4) << char(unique_ch[i]);
        cout << endl;
    for (int i = 0; i < table_rows; i++)
    {
        for (int j = 0; j <= unique_cnt; j++)
            cout << setiosflags(ios::right) << setw(4) << state_map[statustable[i][j].state];
        cout << endl;
    }
}

bool XLEX_Generator::convertBracket()
{
    char *tmp = new char[max_regex_len]; memset(tmp, 0, sizeof(tmp[0]) * max_regex_len);
    for (int i = 0, j = 0; i < max_regex_len && regex[i] != '\0';)
        if (regex[i] == '[')
        {
            tmp[j++] = '('; 
            if (regex[++i] == ']') return false;                // 方括号内容为空
            else tmp[j++] = regex[i++];
            while (regex[i] != ']')
            {
                if (regex[i] == '\0') return false;        // 没有配对
                tmp[j++] = '|'; tmp[j++] = regex[i++];
            } tmp[j++] = ')'; i++;
        } else tmp[j++] = regex[i++];
    strcpy_s(regex, max_regex_len, tmp); delete[] tmp; return true;
}

void XLEX_Generator::generateCpp()
{
    cout << "int state = 0;\n"
         << "while (state " ;
    for (int i = 0; i < table_rows; i++)
        if (state_map[i] == i && statustable[state_map[i]]->terminal) 
            cout << "!= " << i << " ||";
    cout << " 0) {\n"
         << "\t";
}
// b|(a|b)*c
// (ab(sds|a*|z)d|c)d
// ((a|b)|c)|d
// a*|(dc)
// c|(ab)*
int main()
{
    XLEX_Generator G;
    G.toNFA();
    G.displayNFAGraph();
    G.toDFA();
    G.displaystatustable();
    G.DFAminimize();
    G.displayminimizedtable();
    system("pause");
    return 0;
}