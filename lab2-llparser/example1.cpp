#include <bits/stdc++.h>

using STR = std::string;
template <class T>
using V = std::vector<T>;
using VS = V<STR>;

VS Split(STR s, STR sep) {
  VS v;
  s += sep;
  for (size_t pos = 0, i;;) {
    i = s.find(sep, pos);
    if (i == STR::npos) break;
    v.push_back(s.substr(pos, i - pos));
    pos = i + sep.length();
  }
  return v;
}

void Erase(STR &v, char c) {
  v.erase(std::remove(v.begin(), v.end(), c), v.end());
}

struct Selecter {
  STR tbl, col;
  Selecter() {}
  Selecter(STR s) {
    if (s.find('.') != STR::npos) {
      auto v = Split(s, ".");
      tbl = v[0];
      col = v[1];
    } else
      col = s;
  }
  Selecter(STR tbl, STR col) : tbl(tbl), col(col) {}
};

struct Cond {
  Selecter lhs;
  char op;
  Selecter rhs;
  Cond() {}
  Cond(STR lhss, char op, STR rhss) : lhs(lhss), op(op), rhs(rhss) {}
};

using BS = std::bitset<10000>;
struct Table {
  VS hdr;
  STR name;
  V<VS> cont;
  std::map<STR, std::map<STR, BS>> idx;
  void IndexUp() {
    for (int i = 0; i < cont.size(); ++i)
      for (int j = 0; j < hdr.size(); ++j) {
        STR h = hdr[j];
        idx[h][cont[i][j]][i] = 1;
      }
  }
  BS SelfQuery(V<Cond> const &conds) {
    BS res;
    res.set();
    for (auto cond : conds) {
      if (cond.lhs.tbl == "Const") {
        std::swap(cond.lhs, cond.rhs);
        cond.op ^= ((cond.op) != '=') << 1;
      }
      if (cond.lhs.tbl == name && cond.rhs.tbl == "Const") {
        BS cst;
        if (cond.op == '=') {
          cst = idx[cond.lhs.col][cond.rhs.col];
        } else {
          for (int i = 0; i <= 100; ++i)
            if ((cond.op == '<' && i < std::stoi(cond.rhs.col)) ||
                (cond.op == '>' && i > std::stoi(cond.rhs.col)))
              cst = cst | idx[cond.lhs.col][std::to_string(i)];
        }
        res = res & cst;
      }
    }
    return res;
  }
  VS Extract(VS &cont, V<Selecter> sel) {
    VS v;
    for (auto &item : sel)
      if (item.tbl == name)
        for (int i = 0; i < hdr.size(); ++i)
          if (hdr[i] == item.col) v.push_back(cont[i]);
    return v;
  }
  V<VS> Output(V<Selecter> const &sel, BS b) {
    V<VS> res;
    for (int i = b._Find_first(); i != b.size() && i < cont.size();
         i = b._Find_next(i))
      res.push_back(Extract(cont[i], sel));
    return res;
  }
};

std::map<STR, Table> db;

void Query(STR qstring) {
  auto iselect = qstring.find("SELECT"), ifrom = qstring.find("FROM"),
       iwhere = qstring.find("WHERE");
  auto sselect = qstring.substr(iselect + 6, ifrom - (iselect + 6)),
       sfrom = qstring.substr(
           ifrom + 4,
           (iwhere == STR::npos ? qstring.length() : iwhere) - (ifrom + 4));
  Erase(sselect, ' ');
  Erase(sfrom, ' ');
  V<Selecter> qselect;
  if (sselect != "*")
    for (auto &s : Split(sselect, ",")) qselect.emplace_back(s);
  auto qfrom = Split(sfrom, STR("") + ',');
  auto FindTable = [&](Selecter &s) {
    if (s.tbl == "") {
      for (auto tbl : qfrom)
        if (std::count(db[tbl].hdr.begin(), db[tbl].hdr.end(), s.col)) {
          s.tbl = tbl;
          return;
        }
      s.tbl = "Const";
    }
  };
  V<Cond> qcond;
  if (iwhere != STR::npos) {
    auto scond = qstring.substr(iwhere + 5, qstring.length() - (iwhere + 5));
    for (auto s : Split(scond, " AND ")) {
      char op;
      Erase(s, ' ');
      for (char c : {'=', '<', '>'})
        if (Split(s, STR(1, c)).size() == 2) op = c;
      auto sp = Split(s, STR(1, op));
      Cond cond(sp[0], op, sp[1]);
      if (cond.lhs.col[0] == '"') {
        cond.lhs.tbl = "Const";
        Erase(cond.lhs.col, '"');
      }
      if (cond.rhs.col[0] == '"') {
        cond.rhs.tbl = "Const";
        Erase(cond.rhs.col, '"');
      }
      FindTable(cond.lhs);
      FindTable(cond.rhs);
      qcond.push_back(cond);
    }
  }
  for (auto &item : qselect) FindTable(item);
  if (sselect == "*")
    for (auto tbl : qfrom)
      for (auto h : db[tbl].hdr) qselect.emplace_back(tbl, h);
  V<VS> vall;
  BS b1 = db[qfrom[0]].SelfQuery(qcond);
  auto v1 = db[qfrom[0]].Output(qselect, b1);
  if (qfrom.size() == 1) {
    vall = v1;
  } else {
    auto Cartesian = [&](VS &iv1, STR name1, VS &iv2,
                         V<Selecter> &qselect) -> VS {
      int i1 = 0, i2 = 0;
      VS curl;
      for (auto &sel : qselect)
        curl.push_back(sel.tbl == name1 ? iv1[i1++] : iv2[i2++]);
      return curl;
    };
    BS b2 = db[qfrom[1]].SelfQuery(qcond);
    auto iter =
        std::find_if(qcond.begin(), qcond.end(), [&](Cond const &x) -> bool {
          return x.lhs.tbl != x.rhs.tbl && x.lhs.tbl != "Const" &&
                 x.rhs.tbl != "Const";
        });
    if (iter != qcond.end()) {
      auto entg = *iter;
      if (entg.lhs.tbl == qfrom[1]) std::swap(entg.lhs, entg.rhs);
      int colID = std::find(db[qfrom[0]].hdr.begin(), db[qfrom[0]].hdr.end(),
                            entg.lhs.col) -
                  db[qfrom[0]].hdr.begin();
      for (int i = b1._Find_first();
           i != b1.size() && i < db[qfrom[0]].cont.size();
           i = b1._Find_next(i)) {
        entg.lhs.tbl = "Const";
        entg.lhs.col = db[qfrom[0]].cont[i][colID];
        BS ba2 = b2 & db[qfrom[1]].SelfQuery({entg});
        if (ba2.any()) {
          auto v2 = db[qfrom[1]].Output(qselect, ba2);
          VS iv1 = db[qfrom[0]].Extract(db[qfrom[0]].cont[i], qselect);
          for (auto &iv2 : v2)
            vall.push_back(Cartesian(iv1, qfrom[0], iv2, qselect));
        }
      }
    } else {
      auto v2 = db[qfrom[1]].Output(qselect, b2);
      for (auto &iv1 : v1)
        for (auto &iv2 : v2)
          vall.push_back(Cartesian(iv1, qfrom[0], iv2, qselect));
    }
  }
  for (auto &v : vall)
    for (size_t i = 0; i < v.size(); ++i)
      std::cout << v[i] << (i == v.size() - 1 ? '\n' : ' ');
}

int main() {
  db = {{"Student", {{"sid", "dept", "age"}}},
        {"Course", {{"cid", "name"}}},
        {"Teacher", {{"tid", "dept", "age"}}},
        {"Grade", {{"sid", "cid", "score"}}},
        {"Teach", {{"cid", "tid"}}}};
  for (STR tname : {"Student", "Course", "Teacher", "Grade", "Teach"}) {
    int m;
    std::cin >> m;
    auto &tbl = db[db[tname].name = tname];
    tbl.cont.resize(m);
    for (auto &item : tbl.cont) {
      item.resize(tbl.hdr.size());
      for (auto &s : item) std::cin >> s;
    }
    tbl.IndexUp();
  }
  int m;
  std::cin >> m;
  std::cin.get();
  for (STR qs; m-- && std::getline(std::cin, qs); Query(qs))
    ;
  return 0;
}