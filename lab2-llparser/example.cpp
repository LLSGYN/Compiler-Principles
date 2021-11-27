#include <bits/stdc++.h>

#ifdef DEBUG
#define cout cerr
#define PRINT_FAIL() std::cerr << "FAIL\n"
#else
#define PRINT_FAIL()
#endif

template <class T>
inline bool Enlarge(T &a, T const &b) {
  return a < b ? a = b, 1 : 0;
}

template <class T>
inline bool Reduce(T &a, T const &b) {
  return a > b ? a = b, 1 : 0;
}

int globalTick;
struct Edits {
  struct Entry {
    int pos;
    std::string s;
  };

  std::vector<Entry> edits;
  int fileID, created;
  bool dark;

  void Add(int pos, std::string const &s) { edits.push_back({pos, s}); }
};

// std::map<int, int> fileCreated;

struct Commit {
  std::string fa[2];
  std::map<std::string, Edits> files;
  int findTick;
} uncom;

std::map<std::string, Commit> coms;
std::string HEAD;

int findTick;
std::vector<Edits *> FindFile(std::string const &fname) {
  std::string dname = '-' + fname;
  ++findTick;
  std::vector<Edits *> result;
  std::function<void(std::string)> DFS = [&](std::string const &com) {
    if (com == "") return;
    auto &cur = coms[com];
    if (cur.findTick == findTick) return;
    cur.findTick = findTick;
    DFS(cur.fa[0]);
    DFS(cur.fa[1]);
    if (cur.files.count(fname))
      result.push_back(&cur.files[fname]);
    else if (cur.files.count(dname))
      result.push_back(&cur.files[dname]);
  };
  DFS(HEAD);
  if (uncom.files.count(fname))
    result.push_back(&uncom.files[fname]);
  else if (uncom.files.count(dname))
    result.push_back(&uncom.files[dname]);
  Edits *choice = NULL;
  for (auto &p : result)
    if (!choice || p->created > choice->created) choice = p;
  std::vector<Edits *> res;
  for (auto &p : result)
    if (p->fileID == choice->fileID && !p->dark)  // XXX!!!!!
      res.push_back(p);
  return res;
}

std::vector<std::string> FindAllFiles() {
  ++findTick;
  std::vector<std::string> result;
  std::map<std::string, int> created, darkCreated;
  auto Deal = [&](Commit const &cur) {
    for (auto &e : cur.files) {
      if (e.second.dark)
        Enlarge(darkCreated[e.first], e.second.created);
      else
        Enlarge(created[e.first], e.second.created);
    }
  };
  std::function<void(std::string)> DFS = [&](std::string const &com) {
    if (com == "") return;
    auto &cur = coms[com];
    if (cur.findTick == findTick) return;
    cur.findTick = findTick;
    DFS(cur.fa[0]);
    DFS(cur.fa[1]);
    Deal(cur);
  };
  DFS(HEAD);
  Deal(uncom);
  for (auto &p : created)
    if (!darkCreated.count('-' + p.first) ||
        darkCreated['-' + p.first] < p.second)
      result.push_back(p.first);
  return result;
}

struct Range {
  int l, r;
  bool Empty() const { return l > r; }

  friend Range operator&(Range const &a, Range const &b) {
    return {std::max(a.l, b.l), std::min(a.r, b.r)};
  }
};

void Solve() {
  ++globalTick;
  std::string cmd;
  std::cin >> cmd;

  // std::cerr << '\'' << globalTick << '\n' << cmd << '\n';

  if (cmd == "write") {
    std::string fname;
    int offset, len;
    std::cin >> fname >> offset >> len;
    std::cin.get();
    std::string s;
    std::getline(std::cin, s);
    if (uncom.files.count(fname)) {
    } else {
      auto edits = FindFile(fname);
      if (edits.size() > 0 && !edits[0]->dark) {
        uncom.files[fname] = {{}, edits[0]->fileID, globalTick, false};
      } else {
        int id = rand();
        uncom.files[fname] = {{}, id, globalTick, false};
        // fileCreated[id] = globalTick;
      }
    }
    uncom.files[fname].Add(offset, s);
  } else if (cmd == "read") {
    std::string fname;
    int offset, len;
    std::cin >> fname >> offset >> len;
    std::string res = std::string(len, '.');
    std::vector<Edits *> edits = FindFile(fname);
    for (auto &e : edits) {
      for (auto &i : e->edits) {
        auto rg = Range{i.pos, i.pos + i.s.length() - 1} &
                  Range{offset, offset + len - 1};
        if (!rg.Empty())
          std::copy(i.s.begin() + rg.l - i.pos, i.s.begin() + rg.r + 1 - i.pos,
                    res.begin() + rg.l - offset);
      }
    }
    std::cout << res << '\n';
  } else if (cmd == "unlink") {
    std::string fname;
    std::cin >> fname;
    auto edits = FindFile(fname);
    if (!edits.empty() && !edits[0]->dark) {
      int id = rand();
      uncom.files['-' + fname] = {{}, id, globalTick, true};
      // fileCreated[id] = globalTick;
      if (uncom.files.count(fname)) uncom.files.erase(fname);
    } else
      PRINT_FAIL();
  } else if (cmd == "ls") {
    auto res = FindAllFiles();
    std::cout << res.size();
    if (res.size() != 0) std::cout << ' ' << res.front() << ' ' << res.back();
    std::cout << '\n';
  } else if (cmd == "commit") {
    std::string cmname;
    std::cin >> cmname;
    if (!uncom.files.empty() && !coms.count(cmname)) {
      auto &com = (coms[cmname] = uncom);
      com.fa[0] = HEAD;
      HEAD = cmname;
      uncom = {};
    } else
      PRINT_FAIL();
  } else if (cmd == "merge") {
    std::string mge[2], cmname;
    std::cin >> mge[0] >> cmname;
    mge[1] = HEAD;
    if (uncom.files.empty() && mge[0] != HEAD && coms.count(mge[0]) &&
        coms.count(mge[1]) && !coms.count(cmname)) {
      // count(cmname) may be flawed
      auto &com = coms[cmname];
      com.fa[0] = mge[0];
      com.fa[1] = mge[1];
      HEAD = cmname;
    }
  } else if (cmd == "checkout") {
    std::string newh;
    std::cin >> newh;
    if (uncom.files.empty() && coms.count(newh)) {
      HEAD = newh;
    } else
      PRINT_FAIL();
  }
}

int main() {
#ifdef INFILE
  freopen("in", "r", stdin);
#endif
  std::ios::sync_with_stdio(0);
  std::cin.tie(0);
  std::cout.tie(0);
  int T;
  std::cin >> T;
  while (T--) Solve();
  return 0;
}