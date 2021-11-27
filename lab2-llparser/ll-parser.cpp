#include <bits/stdc++.h>

// #define DEBUG
#define synch -2

using STR = std::string;
template <class T>
using V = std::vector<T>;
using VS = V<STR>;
using PII = std::pair<int, int>;

struct Parser {
    // 非终结符号、终结符号表
    std::unordered_map<STR, int> terminals, nonterminals;
    VS tt, nt;
    using rhs_t = V<PII>; // 存储产生式右侧的结构
    V<V<rhs_t>> productions; // 产生式
    V<int> could_be_epsilon, vis; // 处理非终结符号是否可能推出空
    V<std::set<int>> first, follow; // first、follow集
    V<V<int>> table; // ll(1) 预测分析表
    int eps_id, eof;
    int id(const STR& str, int flag) {
        if (flag) {
            auto iter = nonterminals.find(str);
            if (iter == nonterminals.end()) {
                nonterminals[str] = nonterminals.size();
                nt.push_back(str);
            }
            return nonterminals[str];
        }
        else {
            auto iter = terminals.find(str);
            if (iter == terminals.end()) {
                terminals[str] = terminals.size();
                tt.push_back(str);
            }
            return terminals[str];
        }
    };
    void init(const char* filename) {
        STR linebuf;
        eps_id = id("eps", 0);
        std::ifstream ifs(filename);
        // 读入产生式
        while (getline(ifs, linebuf)) {
            std::stringstream ss(linebuf);
            STR Lhs, Rhs; ss >> Lhs;
            int lhs = id(Lhs, 1);
            ss >> Rhs; // 跳过->
            while (lhs >= productions.size()) productions.push_back({});
            while (ss >> Rhs) {
                if (Rhs == "|") continue;
                int j = 0, rid, flag;
                rhs_t rhs;
                do {
                    STR symbol;
                    flag = 0;
                    if (isupper(Rhs[j])) {
                        flag = 1;
                        symbol = STR(1, Rhs[j++]);
                        while (Rhs[j] == '\'') symbol += Rhs[j++];
                    }
                    else if (islower(Rhs[j])) {
                        while (j < Rhs.size() && islower(Rhs[j]))
                            symbol += Rhs[j++];
                    }
                    else 
                        symbol = STR(1, Rhs[j++]);
                    rid = id(symbol, flag);
                    // if (symbol == "eps") eps_id = rid;
                    rhs.push_back({rid, flag});
                } while (j < Rhs.size());
                productions[lhs].push_back(rhs);
            }
        }
        convert();
        // return;
        int N = productions.size();
        eof = terminals.size();
        could_be_epsilon.resize(N), vis.resize(N), first.resize(N), follow.resize(N);
        handle_epsilon();
        for (int i = 0; i < N; ++i) vis[i] = 0;
        for (int i = 0; i < N; ++i)
            if (!vis[i]) getFirst({{i, 1}});
        for (int i = 0; i < N; ++i) {
            printf("First(%s) = ", nt[i].c_str());
            auto test = first[i];
            for (auto& iter: test) {
                printf("%s ", tt[iter].c_str());
            }
            puts("");
        }
        getFollow();
        for (int i = 0; i < N; ++i) {
            printf("Follow(%s) = ", nt[i].c_str());
            auto test = follow[i];
            for (auto& iter: test) {
                printf("%s ", iter == eof ? "$" : tt[iter].c_str());
            }
            puts("");
        }

        table.resize(N, V<int>(terminals.size() + 1, -1));
        construct();
        // 输出预测分析表
        for (int i = -1; i < int(table.size()); ++i) {
            if (i == -1) {
                printf("%-8s", "");
                for (int j = 1; j < tt.size(); ++j)
                    printf("%-8s", tt[j].c_str());
                printf("$\n");
            }
            else {
                printf("%-8s", nt[i].c_str());
                for (int j = 1; j < table[i].size(); ++j) {
                    // printf("%d\t", table[i][j]);
                    STR pro;
                    if (table[i][j] >= 0)
                    for (auto x: productions[i][table[i][j]])
                        pro += x.second ? nt[x.first] : tt[x.first];
                    else if (table[i][j] == -2)
                        pro = "synch";
                    printf("%-8s", pro.c_str());
                }
                puts("");
            }
        }
    }

    // 消除左递归、提取左公因子
    void convert() {
        decltype(productions) newp(productions.size());
        for (int i = 0; i < productions.size(); ++i) {
            V<int> hasl(productions[i].size());
            bool flag = 0;
            for (int j = 0; j < productions[i].size(); ++j) {
                if (productions[i][j][0].second && productions[i][j][0].first == i)
                    hasl[j] = true, flag = true;
            }
            STR tsymbol;
            int tid = -1;
            // 有直接左递归出现
            if (flag) {
                // 为A增加A'
                tsymbol = nt[i] + "\'";
                tid = id(tsymbol, 1);
                newp.push_back({});
                newp[tid].push_back(rhs_t(1, {eps_id, 0}));
            }
            // 增加A'的产生式
            for (int j = 0; j < productions[i].size(); ++j) {
                if (hasl[j]) {
                    rhs_t nr(productions[i][j].begin() + 1, productions[i][j].end());
                    nr.push_back({tid, 1});
                    newp[tid].push_back(nr);
                }
                else {
                    rhs_t nr(productions[i][j]);
                    if (flag) nr.push_back({tid, 1});
                    newp[i].push_back(nr);
                }
            }
        }
        decltype(productions) newp1(newp.size());
        for (int i = 0; i < newp.size(); ++i) {
            int m = newp[i].size();
            V<int> vis(m);
            STR tsymbol = nt[i];
            for (int j = 0; j < m; ++j) {
                if (!vis[j]) {
                    V<int> mk(m);
                    int len = 0, tot = 0;
                    vis[j] = 1, mk[j] = 1;
                    do {
                        int cnt = 0, flg = 0;
                        for (int k = j + 1; k < m; ++k) {
                            if (newp[i][k].size() - 1 == len) flg = 1;
                            if (newp[i][k][len] == newp[i][j][len] && (!len || mk[k]))
                                cnt++, vis[k] = true, mk[k] = true;
                        }
                        if (!cnt || cnt < tot) break;
                        tot = cnt, len++;
                        if (flg || len == newp[i][j].size() - 1) break;
                    } while (true);
                    if (!len)
                        newp1[i].push_back(newp[i][j]);
                    else {
                        tsymbol +=  "\'";
                        int tid = id(tsymbol, 1);
                        newp1.push_back({});
                        rhs_t rhs(newp[i][j].begin(), newp[i][j].begin() + len);
                        rhs.push_back({tid, 1});
                        newp1[i].push_back(rhs);
                        for (int k = j; k < m; ++k) if (mk[k]) {
                            auto rhs = rhs_t(newp[i][k].begin() + len, newp[i][k].end());
                            if (rhs.size() == 0) rhs.push_back({eps_id, 0});
                            newp1[tid].push_back(rhs);
                        }
                    }
                }
            }
        }
        productions = newp1;
    }
    #ifdef DEBUG
    void print() {
        for (int i = 0; i < productions.size(); ++i) {
            printf("%s -> ", nt[i].c_str());
            for (int j = 0; j < productions[i].size(); ++j) {
                for (auto& it: productions[i][j]) {
                    printf("%s", it.second ? nt[it.first].c_str() : tt[it.first].c_str());
                }
                if (j != productions[i].size() - 1) printf(" | ");
            }
            printf("\n");
        }
    }
    #endif
    void handle_epsilon() {
        std::function<bool(int)> dfs = [&](int u) {
            bool res = false;
            vis[u] = true;
            for (auto& p: productions[u]) {
                bool flg = true;
                for (int i = 0; i < p.size(); ++i) {
                    if (!p[i].second) {
                        flg = p[i].first == eps_id ? true : false;
                        continue;
                    }
                    int v = p[i].first;
                    if (!vis[v])
                        could_be_epsilon[v] = dfs(v);
                    flg &= could_be_epsilon[v];
                }
                res |= flg;
            }
            return res;
        };
        for (int i = 0; i < productions.size(); ++i) {
            if (!vis[i]) dfs(i);
        }
    }
    // void left_recursive();
    STR get_production(int lid, int rid) {
        STR res;
        res += nt[lid] + " -> ";
        for (auto& it: productions[lid][rid]) {
            if (it.second)
                res += nt[it.first];
            else {
                if (it.first == eof)
                    res += "$";
                else res += tt[it.first];
            }
        }
        return res;
    }
    // 求first集
    std::set<int> getFirst(rhs_t x) {
        if (!x.size()) return {};
        if (!x[0].second) return {x[0].first}; // 如果开头是终结符，则直接返回自身的集合
        if (vis[x[0].first]) return first[x[0].first]; // 如果之前已经计算过，则直接返回结果（记忆化）
        vis[x[0].first] = true; // 标记访问
        std::set<int> ans;
        int lhs = x[0].first;
        for (auto& p: productions[lhs]) {
            auto res = getFirst(p);
            ans.insert(res.begin(), res.end()); //加入第一个非终结符的first
            if (p[0].second && could_be_epsilon[p[0].first]) // 如果有一个能够推导出epsilon的前缀，则加入多个first
            for (int i = 1; i < p.size() && p[i].second && could_be_epsilon[p[i].first]; ++i) {
                res = getFirst(rhs_t(p.begin() + i, p.end()));
                ans.insert(res.begin(), res.end());
            }
        }
        return first[x[0].first] = ans; // 记忆化
    }
    // 求follow集
    void getFollow() {
        follow[0].insert(eof); // 加入$
        bool has_update = 1;
        // 检查更新
        std::function<bool(std::set<int>&, const std::set<int>&)>
            update = [&](std::set<int>& des, const std::set<int>& src) {
                int sz = des.size();
                des.insert(src.begin(), src.end());
                if (des.find(eps_id) != des.end()) des.erase(eps_id); // 不加入epsilon
                return sz < des.size();
            };
        while (has_update) { // 没有更新则算法结束
            has_update = 0;
            for (int i = 0; i < productions.size(); ++i)
                for (auto& rhs : productions[i]) {
                    for (int j = 0; j < rhs.size(); ++j) {
                        if (rhs[j].second)
                            has_update |= update(follow[rhs[j].first], getFirst(rhs_t(rhs.begin() + j + 1, rhs.end())) );
                    }
                    for (int j = rhs.size() - 1; (~j) && rhs[j].second; --j) {
                        if (j == rhs.size() - 1 || could_be_epsilon[rhs[j + 1].first]) // 考虑epsilon的情况
                            has_update |= update(follow[rhs[j].first], follow[i]);
                        else break;
                    }
                }
        }
    }
    // 构造预测分析表
    void construct() {
        for (int i = 0; i < productions.size(); ++i)  {
            for (int j = 0; j < productions[i].size(); ++j) {
                auto fi = getFirst(productions[i][j]);
                bool flg = 0;
                for (auto& it: fi) {
                    flg |= (it == eps_id);
                    if (table[i][it] != -1 && table[i][it] != j)
                        throw std::runtime_error("is not LL(1) grammar!\n");
                    table[i][it] = j;
                }
                if (flg) {
                    for (auto& it: follow[i]) {
                        if (table[i][it] != -1 && table[i][it] != j)
                            throw std::runtime_error("is not LL(1) grammar!\n");
                        table[i][it] = j;
                    }
                }
            }
            for (auto& it: follow[i]) { // 加入同步信息
                if (table[i][it] == -1)
                    table[i][it] = synch;
            }
        }
    }
    void load_token(V<int>& tok, const STR& str) {
        int j = 0;
        while (j < str.length()) {
            STR symbol;
            if (isalpha(str[j]))
            while (j < str.length() && isalpha(str[j])) {
                symbol += str[j++];
            }
            else symbol += str[j++];
            if (terminals.find(symbol) == terminals.end()) {
                fprintf(stderr, "undefined terminal\n");
                exit(-1);
            }
            tok.push_back(id(symbol, 0));
        }
        tok.push_back(eof);
    }
    // LL(1)预测分析
    void parse(const STR& str) {
        // 输出栈
        std::function<void(std::deque<PII>&)> print_stack = [&](std::deque<PII>& q) {
            STR buf;
            for (auto it = q.rbegin(); it != q.rend(); ++it) {
                if (it->second) buf += nt[it->first];
                else buf += (it->first == eof ? "$" : tt[it->first]);
            }
            printf("%-24s", buf.c_str());
        };
        // 输出待输入内容
        auto print_input = [&](V<int>::iterator l, V<int>::iterator r) {
            STR buf;
            for (auto it = l; it < r; ++it) 
                buf += (*it == eof ? "$" : tt[*it]);
            printf("%-24s", buf.c_str());
        };
        V<int> token;
        load_token(token, str);
        auto iter = token.begin();
        std::deque<PII> st;
        // 应急处理函数
        auto error = [&]() {
            auto cur = st.front();
            if (cur.second) {
                if (table[cur.first][*iter] == -1)
                    fprintf(stderr, "ERROR! Ignore input %s\n", tt[*iter++].c_str());
                else {
                    st.pop_front();
                    fprintf(stderr, "SYNCHRONIZED! Ignore %s\n", nt[cur.first].c_str());
                }
            }
            else if (!cur.second && cur.first != *iter) { // 弹出栈顶的终结符号
                fprintf(stderr, "ERROR! Ignore %s\n", tt[cur.first].c_str());
                st.pop_front();
            }
        };
        st.push_front({eof, 0}), st.push_front({0, 1});
        while (!st.empty() && iter < token.end()) {
            print_stack(st);
            print_input(iter, token.end());
            auto cur = st.front();
            if (cur.second == 0) {
                if (cur.first == *iter) {
                    printf("matches %s", *iter == eof ? "$" : tt[*iter].c_str());
                    st.pop_front(), iter++;
                }
                else error();
            }
            else {
                int ent = table[cur.first][*iter];
                if (ent == -1 || ent == -2) error();
                else {
                    rhs_t p = productions[cur.first][ent];
                    if (p == rhs_t(1, {eps_id, 0})){ /*puts("del");*/ st.pop_front(); }
                    else {
                        st.pop_front();
                        for (int i = p.size() - 1; ~i; --i) {
                            // printf("add to stack: %s\n", p[i].second ? nt[p[i].first].c_str() : tt[p[i].first].c_str());
                            st.push_front(p[i]);
                        }
                    }
                    printf("%-24s", get_production(cur.first, ent).c_str());
                }
            }
            puts("");
        }
        if (!st.empty() || iter != token.end()) {
            // for (; iter < token.end(); ++iter)
            //     printf("%s, ", tt[*iter].c_str());
            // puts("");
            // while (!st.empty()) {
            //     printf("%d %d\n", st.top().first, st.top().second);
            //     // if (st.top().second)
            //     //     printf("%s", nt[st.top().first].c_str());
            //     // else printf("%s", tt[st.top().first].c_str());
            //     st.pop();
            // }
            // puts("");
            // error();
            printf("FATAL ERROR, parse fail\n");
            exit(-1);
        }
    }
};


int main(int argc, char **argv) {
    #ifdef DEBUG
    freopen("0.txt", "r", stdin);
    #endif
    if (argc != 2) {
        printf("enter the filename of the grammar!\n");
        exit(-1);
    }
    Parser parser;
    parser.init(argv[1]);
    STR expr;
    std::cin >> expr;
    parser.parse(expr);
    // parser.parse("(num+(num*num))/(num-num)");
    return 0;
}