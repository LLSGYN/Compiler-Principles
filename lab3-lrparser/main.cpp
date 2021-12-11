#include <bits/stdc++.h>
#define MAXN 1024

using rhs_t = std::vector<int>;

struct Symbol;

struct rule
{
	int index;
	int lhs;
	rhs_t rhs;
};

struct Symbol
{
	std::string name;
	int index;
	bool calc = 0;
	enum
	{
		TERMINAL,
		NONTERMINAL
	} type;
	std::set<int> firstset;
	std::set<int> followset;
	std::vector<rule> rules;
};

struct item
{
	rule *rp = nullptr;
	int dot = 0;
	// std::set<int> lookhead;
	bool operator==(const item &x) const
	{
		return x.rp == rp ? x.dot == dot : 0;
	}
};

struct State;

struct action
{
	// std::set<Symbol*> sp;
	enum action_t
	{
		SHIFT,
		REDUCE,
		ACCEPT,
		ERROR,
		CONFLICT
	} type;
	union
	{
		State *stp; // the new state, if a shift
		rule *rp;	// the rule, if a reduce
	} x;
};

// the states that describe the DFA
struct State
{
	std::vector<item> lb;				 // kernel items
	std::vector<item> lal;				 // all items
	std::unordered_map<int, action> act; // actions
	int index;							 // seq num
};

struct hashnode
{
	void *value;
	hashnode *nextht;
};

struct Parser
{
	int start;
	int testid, eofid;
	int state_cnt = 0;
	std::vector<Symbol> stable;				 // symbol table
	std::unordered_map<std::string, int> ht; // hashtable
	State *start_state;
	hashnode hn[MAXN]; // state hash table

	unsigned statehash(const State &);
	void insert_state(unsigned key, void *val);
	unsigned hv; // hash value

	void init(char *);
	void readgram(const char *file);
	void print_rhs(const rhs_t &rhs) const;
	void print_rules() const;
	void print_item(const item &) const;
	void print_state(const State &) const;
	void print_table(const State &) const;
	int add_symbol(const std::string &sym);
	int find_symbol(const std::string &sym);
	void add_action(State &, int, void *, int);
	void get_start_state();
	State *get_state(State &st);
	State *find_state(const State &);
	std::set<int> get_first(rhs_t x);
	void get_follow();
	static bool rulecmp(const item &, const item &);
	bool statecmp(const State &, const State &);
	void build_shift(State &);
	bool update_item(std::vector<item> &, item &);
	void lookhead_init(State &);
	std::vector<item> build_closure(std::vector<item>);
	void parse(const std::string &str);
	rhs_t load_token(const std::string &str);
	Parser();
	// ~Parser();
};

void Parser::print_rhs(const rhs_t &rhs) const
{
	for (int id : rhs)
		printf("%s", stable[id].name.c_str());
}

void Parser::print_rules() const
{
	for (int id = 0; id < stable.size(); ++id)
	{
		if (stable[id].type == Symbol::TERMINAL)
			continue;
		auto &symbol = stable[id];
		for (auto &cur : symbol.rules)
		{
			printf("%s -> ", symbol.name.c_str());
			print_rhs(cur.rhs);
			printf("\n");
		}
	}
}

void Parser::print_item(const item &it) const
{
	printf("\t%s", stable[it.rp->lhs].name.c_str());
	printf(" -> ");
	int len = it.rp->rhs.size();
	for (int i = 0; i < it.dot; ++i)
		printf("%s", stable[it.rp->rhs[i]].name.c_str());
	printf("·");
	for (int i = it.dot; i < len; ++i)
		printf("%s", stable[it.rp->rhs[i]].name.c_str());
	printf("\n");
}

void Parser::print_state(const State &stat) const
{
	printf("I%d:\n", stat.index);
	for (auto &iter : stat.lb)
		print_item(iter);
}

void Parser::print_table(const State &stat) const
{
	printf("I%d:\t", stat.index);
	// action
	for (int i = 0; i < stable.size(); ++i)
	{
		if (i == 1 || stable[i].type)
			continue;
		auto iter = stat.act.find(i);
		if (iter == stat.act.end())
			printf("err\t");
		else if (iter->second.type == action::SHIFT)
			printf("s%d\t", iter->second.x.stp->index);
		else if (iter->second.type == action::REDUCE)
			printf("r%d\t", iter->second.x.rp->index);
		else if (iter->second.type == action::ACCEPT)
			printf("ACC\t");
	}
	// goto
	for (int i = 0; i < stable.size(); ++i)
	{
		if (i == 1 || i == start || !stable[i].type)
			continue;
		auto iter = stat.act.find(i);
		if (iter == stat.act.end())
			printf("err\t");
		else if (iter->second.type == action::SHIFT)
			printf("%d\t", iter->second.x.stp->index);
		else
		{
			fprintf(stderr, "wrong table consruction!\n");
			exit(-1);
		}
	}
	puts("");
}

int Parser::find_symbol(const std::string &sym)
{
	if (ht.find(sym) == ht.end())
		return -1;
	return ht[sym];
}

int Parser::add_symbol(const std::string &sym)
{
	if (ht.find(sym) == ht.end())
	{ // not found
		ht[sym] = ht.size();
		Symbol ns;
		ns.name = sym;
		ns.index = ht[sym];
		ns.type = isupper(sym[0]) ? Symbol::NONTERMINAL : Symbol::TERMINAL;
		stable.emplace_back(ns);
	}
	return ht[sym];
}

void Parser::readgram(const char *file)
{
	std::ifstream fp(file);
	if (!fp)
		throw std::runtime_error("cannot find or open the file.");
	// epsid = add_symbol("eps");
	eofid = add_symbol("$");
	testid = add_symbol("#");
	int readcount = 0, linecount = 0;
	std::string linebuf;
	while (std::getline(fp, linebuf))
	{
		linecount++;
		if (linebuf[0] == '\n')
			continue; // skips blank line
		readcount++;
		std::stringstream ss(linebuf);
		std::string lhstr, temp, rhstr;
		int lhsid, rhsid;
		rule curr;
		ss >> lhstr >> temp;
		lhsid = add_symbol(lhstr);
		curr.index = readcount;
		curr.lhs = lhsid;
		// printf("->%p\n", &curr.lhs->name);
		if (readcount == 1)
			start = lhsid; // the first symbol is start by default
		while (ss >> rhstr)
		{
			rhsid = add_symbol(rhstr);
			curr.rhs.push_back(rhsid);
		}
		if (!curr.rhs.size())
		{
			fprintf(stderr, "invalid production at line %d\n", linecount);
			exit(-1);
		}
		// add production
		stable[lhsid].rules.emplace_back(curr);
	}
	// add start
	if (!readcount)
	{
		fprintf(stderr, "empty grammar!\n");
		printf("%d\n", readcount);
		exit(-1);
	}
	// ensure that the start symbol never appears on the right hand side
	for (auto &symbol : stable)
		for (auto &rule : symbol.rules)
			for (int x : rule.rhs)
				if (x == start)
				{
					fprintf(stderr, "invalid grammar! start should never appears on the rhs.\n");
					exit(-1);
				}
}

bool Parser::rulecmp(const item &x, const item &y)
{
	if (x.rp->lhs != y.rp->lhs) // compare left-hand side firstly
		return x.rp->lhs < y.rp->lhs;
	else
	{
		// if same, compare rhs
		if (x.rp->rhs != y.rp->rhs)
			return x.rp->rhs < y.rp->rhs;
		else // if same, compare dot
			return x.dot < y.dot;
	}
}

unsigned Parser::statehash(const State &st)
{
	unsigned hash = 5381;
	for (auto bp : st.lb)
		hash = ((hash << 5) + hash) + bp.rp->index * 17 + bp.dot;
	return hash;
}

// compare two states by each productions
bool Parser::statecmp(const State &x, const State &y)
{
	auto rx = x.lb, ry = y.lb;
	if (rx.size() != ry.size())
		return 1;
	for (int i = 0; i < rx.size(); ++i)
		if (rx[i].rp->index != ry[i].rp->index)
			return 1;
	return 0;
}

State *Parser::find_state(const State &st)
{
	hv = statehash(st) & (MAXN - 1);
	hashnode *htp = hn[hv].nextht;
	// if (htp) std::cout << "it seems something there\n";
	// else std::cout << "nothing there orz\n";
	State *stp = nullptr;
	while (htp)
	{
		if (statecmp(st, *(State *)htp->value) == 0)
		{
			stp = (State *)htp->value;
			break;
		}
		htp = htp->nextht;
	}
	// if (stp) std::cout << "11111!\n";
	// else std::cout << "00000....\n";
	return stp;
}

bool Parser::update_item(std::vector<item> &s, item &x)
{
	if (std::find(s.begin(), s.end(), x) != s.end())
		return 0;
	s.emplace_back(x);
	return 1;
}

std::vector<item> Parser::build_closure(std::vector<item> I)
{
	std::vector<item> res;
	for (item &it : I)
		res.emplace_back(it);
	bool upd = 1;
	while (upd)
	{
		upd = 0;
		for (item &it : res)
		{
			// A -> x.By
			if (it.dot < it.rp->rhs.size() &&
				stable[it.rp->rhs[it.dot]].type == 1)
			{
				Symbol &Lh = stable[it.rp->rhs[it.dot]];
				for (rule &r : Lh.rules)
				{
					item ni;
					ni.rp = &r;
					ni.dot = 0;
					upd |= update_item(res, ni);
				}
			}
		}
	}
	return res;
}

void Parser::add_action(State &st, int sid, void *x, int tp)
{
	action na;
	switch (tp)
	{
	case action::SHIFT:
		na.type = action::SHIFT;
		na.x.stp = (State *)x;
		break;
	case action::REDUCE:
		na.type = action::REDUCE;
		na.x.rp = (rule *)x;
		break;
	case action::ACCEPT:
		na.type = action::ACCEPT;
		break;
	default:
		na.type = action::ERROR;
		break;
	}
	if (st.act.count(sid))
	{ // collision occurs
		int tp0 = st.act[sid].type;
		st.act[sid].type = action::CONFLICT;
		if (tp0 ^ tp)
			fprintf(stderr, "shift-reduce conflict detected, stop processing!\n");
		else
			fprintf(stderr, "reduce-reduce conflict detected, stop processing!\n");
		print_state(st);
		exit(-1);
	}
	st.act[sid] = na;
}

// construct action, add relative basis to the shift state
void Parser::build_shift(State &st)
{
	// if (++call == 5) exit(-1);
	// <shift_elem, set<item>>
	std::unordered_map<int, std::vector<item>> to_build;
	for (item &it : st.lal)
	{
		assert(it.dot <= it.rp->rhs.size());
		if (it.dot == it.rp->rhs.size())
		{
			/* cannot shift!
			** build reduce*/
			int lhs = it.rp->lhs;
			for (int x : stable[lhs].followset)
			{
				add_action(
					st,											   // state reference
					x,											   // action symbol id
					it.rp,										   // a pointer to the reduce rule
					lhs == start ? action::ACCEPT : action::REDUCE // the type
				);
			}
			continue;
		}
		to_build[it.rp->rhs[it.dot]].emplace_back(it);
	}
	// handling shift actions
	for (auto &shifts : to_build)
	{
		// get the state
		// std::cout << "shift: " << stable[shifts.first].name << "\n";
		State ns;
		for (item &it : shifts.second)
		{
			// std::cout << "\t";
			// print_item(it);
			item ni;
			ni.dot = it.dot + 1;
			ni.rp = it.rp;
			ns.lb.emplace_back(ni);
		}
		State *fs = get_state(ns);
		// add action
		add_action(st, shifts.first, fs, action::SHIFT);
	}
}

void Parser::insert_state(unsigned key, void *val)
{
	hashnode *hp = &hn[key];
	while (hp->nextht)
		hp = hp->nextht;
	hp->nextht = new hashnode;
	hp = hp->nextht;
	hp->value = val;
}

State *Parser::get_state(State &st)
{
	std::sort(st.lb.begin(), st.lb.end(), rulecmp);
	State *fs = find_state(st);
	if (!fs)
	{
		/*
		 * there does not exist a state with same kernel
		 * this is a new state!
		 * get its details
		 */
		fs = new State(st);
		insert_state(hv, fs);
		fs->index = state_cnt++; // allocate new index
		// build_closure(*fs);
		fs->lal = build_closure(fs->lb);
		std::sort(fs->lal.begin(), fs->lal.end(), rulecmp);
		build_shift(*fs); // get actions
#ifdef DEBUG
		// print_table(*fs);
		print_state(*fs);
#endif
	}
	return fs;
}

void Parser::get_start_state()
{
	assert(stable[start].rules.size() == 1);
	// add kernel item, i.e. S' -> S
	item basis;
	basis.rp = &stable[start].rules[0];
	basis.dot = 0;
	// basis.status = item::INCOMPLETE;

	State _start;
	_start.lb.push_back(basis);
	// start_state.lb.push_back(basis);

	start_state = get_state(_start);
}

// calculate first
std::set<int> Parser::get_first(rhs_t x)
{
	if (!x.size())
		return {};
	Symbol &s = stable[x[0]];
	if (!s.calc)
	{
		s.calc = 1;
		std::set<int> fs;
		if (s.type == 1)
		{ // a nonterminal
			for (auto &r : s.rules)
			{
				std::set<int> temp = get_first(r.rhs);
				fs.insert(temp.begin(), temp.end());
			}
			s.firstset = fs;
		}
		else
			s.firstset = {x[0]};
	}
	return s.firstset;
}

// calculate follow
void Parser::get_follow()
{
	stable[start].followset.insert(eofid); // add $ to S'
	bool has_update = 1;
	std::function<bool(std::set<int> &, const std::set<int> &)>
		update = [&](std::set<int> &des, const std::set<int> &src)
	{
		int sz = des.size();
		des.insert(src.begin(), src.end());
		return sz < des.size();
	};
	while (has_update)
	{ // 没有更新则算法结束
		has_update = 0;
		for (auto &sym : stable)
			for (int i = 0; i < stable.size(); ++i)
			{
				if (!stable[i].type)
					continue; // skip terminals
				for (auto &r : stable[i].rules)
				{
					for (int j = 0; j < r.rhs.size(); ++j)
						if (stable[r.rhs[j]].type) // a nonterinal
							has_update |= update(stable[r.rhs[j]].followset, get_first(rhs_t(r.rhs.begin() + j + 1, r.rhs.end())));
					if (stable[r.rhs.back()].type)
						has_update |= update(stable[r.rhs.back()].followset, stable[i].followset);
				}
			}
	}
}

void Parser::init(char *buf)
{
	readgram(buf);
	for (int i = 0; i < stable.size(); ++i)
		get_first({i});
	get_follow();
	// printf("\t");
	// for (int i = 0; i < stable.size(); ++i)
	// 	if (i != 1 && !stable[i].type)
	// 		printf("%s\t", stable[i].name.c_str());
	// for (int i = 0; i < stable.size(); ++i)
	// 	if (i != 1 && i != start && stable[i].type)
	// 		printf("%s\t", stable[i].name.c_str());
	// printf("\n");
	get_start_state(); // lr(0)
#ifdef DEBUG
// print_rules();
#endif
}

rhs_t Parser::load_token(const std::string &str)
{
	rhs_t res;
	int i = 0;
	while (i < str.length())
	{
		std::string temp;
		if (isalpha(str[i]))
			while (isalpha(str[i]))
				temp += str[i++];
		else
			temp += str[i++];
		res.emplace_back(find_symbol(temp));
	}
	res.emplace_back(eofid); // $
	return res;
}

void Parser::parse(const std::string &str)
{
	rhs_t tok = load_token(str);
	std::deque<State *> s; // state stack
	s.push_front(start_state);
	std::deque<int> t; // symbol stack
	int ip = 0;
	int errid = 0;
	while (1)
	{
		std::string buf;
		// printing state stack
		for (auto it = s.rbegin(); it != s.rend(); ++it)
			buf += std::to_string((*it)->index) + " ";
		printf("%-20s\t", buf.c_str());
		buf = "";
		// printing symbol stack
		for (auto it = t.rbegin(); it != t.rend(); ++it)
			buf += stable[*it].name;
		printf("| %-20s| ", buf.c_str());
		// printing buffer
		buf = "";
		for (int i = ip; i < tok.size(); ++i)
			buf += stable[tok[i]].name;
		printf("%-24s\t| ", buf.c_str());

		State *S = s.front();
		int a = tok[ip];
		if (!S->act.count(a))
		{
			errid = 1;
			goto Error;
		}
		if (S->act[a].type == action::SHIFT)
		{
			printf("shift s%d\n", S->act[a].x.stp->index);
			t.push_front(tok[ip++]);
			s.push_front(S->act[a].x.stp);
		}
		else if (S->act[a].type == action::REDUCE)
		{
			rule *rp = S->act[a].x.rp;
			int len = rp->rhs.size();
			while (len--)
				t.pop_front(), s.pop_front();
			S = s.front();
			t.push_front(rp->lhs);
			if (!S->act.count(rp->lhs))
			{
				errid = 2;
				goto Error;
			}
			s.push_front(S->act[rp->lhs].x.stp);
			printf("reduce ");
			printf("%s->", stable[rp->lhs].name.c_str());
			print_rhs(rp->rhs);
			printf("\n");
		}
		else if (S->act[a].type == action::ACCEPT)
		{
			printf("ACCEPT\n");
			return;
		}
	}
	errid = 3;
Error:
	fprintf(stderr, "an error occurs during parsing, errid = %d\n", errid);
	exit(-1);
}

Parser::Parser()
{
	for (int i = 0; i < MAXN; ++i)
	{
		hn[i].nextht = nullptr;
		hn[i].value = nullptr;
	}
}

int main(int argc, char **argv)
{
	assert(argc > 1);
	Parser parser;
	parser.init(argv[1]);
	char input[MAXN];
	scanf("%s", input);
	parser.parse(input);
	// parser.parse("num+(num*num-num)");
	// parser.parse("aabab");
	// parser.parse("((@)@(@@))");
	return 0;
}