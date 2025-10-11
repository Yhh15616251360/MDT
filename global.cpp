//
// Created by Yang on 2024/11/13.
//
#pragma once

#include <ctime>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <map>
#include <vector>
#include <queue>
#include <set>
#include <list>
#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include <cassert>
#include <random>
#include <ctime>
#include <iomanip>
using ui = int;

using namespace std;

typedef pair<int, int> sedge;

class TimeInfo {
 public:
  vector<int> TS;
};

class NodeInfo {
 public:
  map<int, TimeInfo> TE;      //时态邻居
};

class tgraph {
 public:
  vector<NodeInfo> nodes;     //时态图的邻接“表”
  int tmin{}, tmax{}, edge_num{}, maxid{};    //三角形中三边最大的时间间隔，计算需要调用enum_tringale

  vector<unordered_map<int, int>> vedge_set;
  vector<sedge> eid_touv;

  vector<int> eedge_sup;           //以eid为索引
  vector<unordered_map<int, int>> etriangle;
  vector<unordered_set<int>> etuv;
};

/*参考wsdm计算sup*/
int pos;

class ListLinearHeap {
 private:
  ui n; // number vertices,   节点的数目，节点id不一定需要连续，但是数目需要正确
  ui key_cap; // the maximum allowed key value

  ui max_key; // possible max key
  ui min_key; // possible min key

  ui *keys; // keys of vertices

  ui *heads; // head of doubly-linked list for a specific weight
  ui *pres; // pre for doubly-linked list
  ui *nexts; // next for doubly-linked list

 public:
  ListLinearHeap(ui _n, ui _key_cap) {
    n = _n;
    key_cap = _key_cap;

    min_key = key_cap;
    max_key = 0;

    keys = new ui[n];
    pres = new ui[n];
    nexts = new ui[n];
    heads = new ui[key_cap + 1];
  }
  ~ListLinearHeap() {

    if (heads != nullptr) {
      delete[] heads;
      heads = nullptr;
    }
    if (pres != nullptr) {
      delete[] pres;
      pres = nullptr;
    }
    if (nexts != nullptr) {
      delete[] nexts;
      nexts = nullptr;
    }
    if (keys != nullptr) {
      delete[] keys;
      keys = nullptr;
    }

  }

  // initialize the data structure by (id, key) pairs
  // _n is the number of pairs, _key_cap is the maximum possible key value
  void init(ui *_ids, ui *_keys) {
//    assert(_key_cap <= key_cap);
    min_key = key_cap;
    max_key = 0;
    for (ui i = 0; i <= key_cap; i++)
      heads[i] = n;

    for (ui i = 0; i < n; i++)         //这里的i<_n没有发挥作用
    {
      insert(_ids[i], _keys[i]);
    }
  }

  // insert (id, key) pair into the data structure
  void insert(ui id, ui key) {
    assert(id < n);
    assert(key <= key_cap);    //这里的id<n
    //assert(keys[id] > key_cap);

    keys[id] = key;
    pres[id] = n;
    nexts[id] = heads[key];
    if (heads[key] != n) pres[heads[key]] = id;
    heads[key] = id;

    if (key < min_key) min_key = key;
    if (key > max_key) max_key = key;
  }

  // remove a vertex from the data structure
  ui remove(ui id) {
    assert(keys[id] <= max_key);
    if (pres[id] == n) {
      assert(heads[keys[id]] == id);
      heads[keys[id]] = nexts[id];
      if (nexts[id] != n) pres[nexts[id]] = n;
    } else {
      ui pid = pres[id];
      nexts[pid] = nexts[id];
      if (nexts[id] != n) pres[nexts[id]] = pid;
    }

    return keys[id];
  }

  ui get_key(ui id) { return keys[id]; }

  bool empty() {
    tighten();
    return min_key > max_key;
  }

  // get the (id,key) pair with the minimum key value; return true if success, return false otherwise
  bool get_min(ui &id, ui &key) {
    if (empty()) return false;

    id = heads[min_key];
    key = min_key;
    assert(keys[id] == key);

    return true;
  }

  // pop the (id,key) pair with the minimum key value; return true if success, return false otherwise
  bool pop_min(ui &id, ui &key) {
    if (empty()) return false;

    id = heads[min_key];
    key = min_key;

    assert(keys[id] == key);

    heads[min_key] = nexts[id];
    if (heads[min_key] != n) pres[heads[min_key]] = n;
    return true;
  }

  // decrement the key of vertex id by dec
  ui decrement(ui id, ui dec) {
    assert(keys[id] >= dec);

    ui new_key = keys[id] - dec;

    remove(id);
    insert(id, new_key);

    return new_key;
  }

 private:
  void tighten() {
    while (min_key <= max_key && heads[min_key] == n) ++min_key;
    while (min_key <= max_key && heads[max_key] == n) --max_key;
  }
};

void getVE(tgraph &G, int &V, const string &filename) {
  int u, v, t, tmin = INT32_MAX, tmax = INT32_MIN;
  ifstream fin;
  fin.open(filename);
  assert(fin);

  while (fin.peek() != EOF) {
    fin >> u >> v >> t;
    // cout << u << " " << v << " " << t << " " << endl;
    V = max(V, max(u, v)); //获得图中的结点最大序号
    tmin = min(tmin, t);
    tmax = max(tmax, t);
  }
  G.tmax = tmax;
  G.tmin = tmin;

  fin.close();
}

//输入数据是按时间戳有序的
bool Readin(const string &filename, tgraph &G) {
  ifstream fin;
  fin.open(filename);
  if (fin.fail()) {
    cout << filename << endl;
    return false;
  }

  int V = 0;
  getVE(G, V, filename);
  G.maxid = V;

  G.nodes.resize(V + 1);
  G.vedge_set.resize(V + 1);
  G.etuv.resize(G.tmax + 1);

  //  nodes.resize(V + 1);

  assert(fin);
  int u, v, t;
  int eid = 0;
  while (fin >> u >> v >> t) {
    if (fin.eof())
      break;
    if (u == v)
      continue;
    if (u > v)
      swap(u, v);
    sedge eg({u, v});
    if (G.nodes[u].TE.find(v) ==
        G.nodes[u].TE.end()) //时态边集大小为0，这条静态边是第一次出现
    {
      G.eid_touv.push_back(eg); //直接push_back进去
      // G.eid_touv[eid] = make_pair(u, v);
      G.vedge_set[u][v] = eid;
      eid++;
    }
    if (find(G.nodes[v].TE[u].TS.begin(), G.nodes[v].TE[u].TS.end(), t) ==
        G.nodes[v]
            .TE[u]
            .TS
            .end()) //这条时态边是第一次出现（因为用的是push_back所以需要检查）
    {
      G.nodes[v].TE[u].TS.push_back(t);
      G.nodes[u].TE[v].TS.push_back(t);
      G.etuv[t].insert(G.vedge_set[u][v]); //统计t时刻的时态边的eid
    }
    G.etuv[t].insert(G.vedge_set[u][v]);
  }
  G.edge_num = eid;

  cout << "number of vertices: " << V << endl;
  cout << "range of timestamps: [" << G.tmin << "," << G.tmax << "]" << endl;
  cout << "number of timestamps: " << G.tmax - G.tmin + 1 << endl;

  return true;
}

void sort_3vector(vector<int> &vec1, vector<int> &vec2, vector<int> &vec3)        //滑动窗口计数时将三个数组按元素个数排为vec1,vec2,vec3
{

  int n1 = (int) vec1.size();
  int n2 = (int) vec2.size();
  int n3 = (int) vec3.size();
  if (n1 < n2)    //vec1 vec2
  {
    if (n2 < n3)//vec1,vec2,vec3
    {
      return;
    } else   //  vec1 vec2, vec3 vec2
    {
      if (n1 < n3)   //vec1 vec2, vec3 vec2 , vec1 vec3 --->vec1 vec3 vec2 交换vec2,vec3
      {
        vector<int> vec4 = vec2;
        vec2 = vec3;
        vec3 = vec4;
      } else   //vec1 vec2, vec3 vec2 , vec3 vec1   --->vec3 vec1 vec2
      {
        vector<int> vec4 = vec3;
        vec3 = vec2;
        vec2 = vec1;
        vec1 = vec4;
      }
    }
  } else         //vec2 vec1
  {
    if (n2 > n3)     //vec2 vec1, vec3  vec2 --->vec3 vec2 vec1 交换vec1和vec3
    {
      vector<int> vec4 = vec1;
      vec1 = vec3;
      vec3 = vec4;
    } else {
      if (n1 < n3)    //vec2 vec1, vec1 vec3 --->vec2  vec1 vec3  交换vec1和vec2
      {
        vector<int> vec4 = vec1;
        vec1 = vec2;
        vec2 = vec4;
      } else         //vec2 vec1, vec3 vec2  ---> vec3 vec2 vec1交换vec3和vec1
      {
        vector<int> vec4 = vec3;
        vec3 = vec1;
        vec1 = vec4;
      }
    }
  }
}

int count_temp_triangle(vector<int> &vec1, vector<int> &vec2, vector<int> &vec3, int dealt)        //给三条边的时间戳计算形成的时态三角形
{
  int s1 = 0;
  int num = 0;
  for (int i: vec1) {
    int flag1 = 1;
    int s2 = 0;
    for (int j = s1; j < vec2.size(); j++) {
      int flag2 = 1;           //每一个j记录一个i
      if (abs(vec2[j] - i) <= dealt) {
        if (flag1) {
          s1 = j;        //记录vec2中满足条件的第一个下标
          flag1 = 0;
        }
        for (int k = s2; k < vec3.size(); k++)    //原来这里是s2
        {
          if (abs(vec3[k] - vec2[j]) <= dealt) {
            if (flag2) {
              s2 = k;
              flag2 = 0;
            }
            if (abs(vec3[k] - i) <= dealt)
              num++;
          }
          if (vec3[k] - vec2[j] > dealt)
            break;
        }
        if (vec2[j] - i > dealt)
          break;
      }

    }
  }
  return num;
}

int count_edge_sup(tgraph &G, int eid, int dealt, vector<unordered_set<int>> &non_temptri) {
  if (G.eedge_sup[eid] != -1)        //不能直接用sup值为0来判断，而是要用看这条边是否被计算过
    return G.eedge_sup[eid];

  int esup = 0;
  int u = G.eid_touv[eid].first, v = G.eid_touv[eid].second;
  if (G.nodes[u].TE.size() > G.nodes[v].TE.size())    //选择邻节点少的点遍历，寻找第三个点
    swap(u, v);
  for (auto &u3_ifo: G.nodes[u].TE)        //找第三个点
  {
    int w = u3_ifo.first;
    if (w == u || w == v)
      continue;
    if (G.nodes[v].TE.find(w) != G.nodes[v].TE.end())        //（v,w）这条边存在
    {
      if (G.etriangle[eid].count(w) == 0)        //这个三角形还没有被计算过
      {
        vector<int> vec1 = G.nodes[u].TE[v].TS;
        vector<int> vec2 = G.nodes[u].TE[w].TS;
        vector<int> vec3 = G.nodes[v].TE[w].TS;
        sort_3vector(vec1, vec2, vec3);        //因为传的是引用，所以不能直接传G，
        int trian_num = count_temp_triangle(vec1, vec2, vec3, dealt);

        int eiduw = G.vedge_set[min(u, w)][max(u, w)], eidvw = G.vedge_set[min(v, w)][max(v, w)];

        //sub_trian已经保证了是最小两边的id->,w
        if (trian_num == 0) {
          if (u < w && v < w)                        //因为前面swap了所以id和id2的大小还需要判断，可以用个flag
            non_temptri[eid].insert(w);
          else if (v < u && w < u)
            non_temptri[eidvw].insert(u);
          else if (u < v && w < v)
            non_temptri[eiduw].insert(v);
        }

        G.etriangle[eid][w] = trian_num;
        G.etriangle[eiduw][v] = trian_num;
        G.etriangle[eidvw][u] = trian_num;
      }
    }
    esup += G.etriangle[eid][w];
  }
  G.eedge_sup[eid] = esup;
  return esup;
}

int count_global_sup(tgraph &G, int dealt, vector<unordered_set<int>> &non_temptri) {
  int maxsup = 0;
  for (int i = 0; i < G.edge_num; i++)
    maxsup = max(maxsup, count_edge_sup(G, i, dealt, non_temptri));
  return maxsup;
}

unordered_set<int> scan_subgraph(tgraph &G, tgraph &Gs, vector<int> &vec, int maxnode) {
  unordered_set<int> nid_edge;

  if (Gs.nodes.empty())                                //从0开始构建子图时，初始化edge_num=0
    Gs.edge_num = 0;

  if (Gs.nodes.empty() || maxnode > Gs.maxid)             //重置Gs.vnodes的大小
  {
    Gs.nodes.resize(maxnode + 1);
    Gs.vedge_set.resize(maxnode + 1);
    Gs.maxid = maxnode;
    //cout << "maxid: " << maxnode << endl;
  }

  int cur_eid = Gs.edge_num;                                //新的eid起点

  for (auto &eid: vec) {
    int id1 = G.eid_touv[eid].first, id2 = G.eid_touv[eid].second;
    Gs.nodes[id1].TE[id2].TS = G.nodes[id1].TE[id2].TS;
    Gs.nodes[id2].TE[id1].TS = G.nodes[id2].TE[id1].TS;
    if (id1 > id2)
      swap(id1, id2);
    Gs.vedge_set[id1][id2] = cur_eid;                   //u，v->eid
    sedge eg{id1, id2};
    Gs.eid_touv.emplace_back(eg);                        //eid->u,v, 可以直接emplace_back
    nid_edge.insert(cur_eid);
    cur_eid++;
  }
  Gs.edge_num = cur_eid;

  //cout << "edge_num: " << Gs.eid_touv.size() << endl;
  Gs.eedge_sup.resize(cur_eid, -1);                          //sup和triangle的大小需要重新扩展
  Gs.etriangle.resize(cur_eid);
  return nid_edge;
}

vector<int> shrink_prune(tgraph &Gs, int q, int maxsup, int delta, int &ansk, int midk) {
  int q_edge = (int) Gs.nodes[q].TE.size();                        //q诱导的边的数量，就是静态林及诶单的数量

  vector<bool> edge_del(Gs.edge_num, false);
  ListLinearHeap heap(Gs.edge_num, maxsup);            //第一个参数是最大的边id值(用来开辟数组空间），第二个参数是最大的value值

  int *Ids = new int[Gs.edge_num];
  int *Keys = new int[Gs.edge_num];
  int i_ids = 0;

  int mmax = 0;
  for (int i = 0; i < Gs.edge_num; i++) {
    Ids[i_ids] = i;
    Keys[i_ids] = Gs.eedge_sup[i];                    //记录eid对应的sup值  (出现了270，-1这样的对)
    i_ids++;

  }

  heap.init(Ids, Keys);            //初始化堆
  vector<int> recover_edge;                            //记录本轮被删除的边，便于后续恢复
  while (!heap.empty() && q_edge >= 2) {
    recover_edge.clear();                        //还能进入循环表明，上一圈被完整删除了
    int eid_min, sup_min;
    heap.get_min(eid_min, sup_min);                //取出了当前sup最小的边，作为本轮的k值
    ansk = sup_min;                                //最大的k值

    while (!heap.empty() && q_edge >= 2)                            //迭代删除小于k的边
    {
      int eid_i, supl;
      heap.get_min(eid_i, supl);

      if (supl <= sup_min)                    //当前值小于等于本轮k值，最多只能保留到本轮
      {
        edge_del[eid_i] = true;
        heap.pop_min(eid_i, supl);
        recover_edge.push_back(eid_i);

        int id1 = Gs.eid_touv[eid_i].first, id2 = Gs.eid_touv[eid_i].second;
        if (q == id1 || q == id2)
          q_edge--;

        for (auto &
              id3_ifo: Gs.etriangle[eid_i])                    //记录的是已经存在的三角形，但是如果有不存在的边访问，则会将那个三角形的值置为0,同时也会产生不存在的边，然后边的id值就为0
        {
          int id3 = id3_ifo.first, dec = id3_ifo.second;
          if (dec == 0)
            continue;

          int eid13 = id1 < id3 ? Gs.vedge_set[id1][id3] : Gs.vedge_set[id3][id1];
          int eid23 = id2 < id3 ? Gs.vedge_set[id2][id3] : Gs.vedge_set[id3][id2];

          if (!edge_del[eid13] && !edge_del[eid23]) {
            if (heap.get_key(eid13) > sup_min)                   //只用更新大于sup_min的值，小于的本来就会被删除
              heap.decrement(eid13, dec);
            if (heap.get_key(eid23) > sup_min)
              heap.decrement(eid23, dec);
          }
        }
      } else
        break;
    }
  }
  if (ansk >= midk && q_edge < 2)            //ansk满足要求才恢复结果，送入check——connect
  {
    while (!heap.empty())                //此时至少包含q的两条边，感觉不太对，应该是要记录进入当前圈的k的所有剩余边？所以需要在迭代删的时候记下来这些边，后面加上que里剩下的
    {
      int e, s;
      heap.pop_min(e, s);
      recover_edge.emplace_back(e);
    }
  }
  /*for (auto& eg : recover_edge)
      cout << Gs.eid_touv[eg].first << "+" << Gs.eid_touv[eg].second << " ";
  cout << endl;*/

  delete[] Ids;
  delete[] Keys;
//  heap.~ListLinearHeap();
  return recover_edge;
}
/*可以在second——prune阶段直接更新G*/
/*重新从q点出发，按照搜索三角形的方式找到三角形连通的边*/

void check_connect(tgraph &G,
                   tgraph &Gs,
                   vector<vector<sedge>> &solution,
                   vector<int> &ans,
                   int q,
                   int delta,
                   int k,
                   vector<unordered_set<int>> &non_temptri) {
  tgraph Gf;
  scan_subgraph(Gs, Gf, ans, Gs.maxid);                                //重新扫描的子图，改子图内所有边的truness满足要求
  vector<bool> vis(Gf.edge_num, false);                                //标记边是否访问过
  vector<unordered_set<int>> trian_vis(Gf.edge_num);

  for (auto &v_ifo: Gf.nodes[q].TE)                                    //从查询顶点q出现开始找
  {
    int u = q, v = v_ifo.first;
    if (u > v)                                                        //调整边的端点顺序
      swap(u, v);
    int eid = Gf.vedge_set[u][v];                                    //取出改变的eid
    if (!vis[eid])                                            //当前q诱导的边还没被访问过，以该边开始诱导
    {
      queue<int> que;                                                //将其加入队列中
      que.push(eid);
      vis[eid] = true;
      vector<sedge> comu;                                            //与该边在一个社区的边
      while (!que.empty())                                        //还可以继续扩展
      {
        int i = que.front();
        que.pop();
        int x = Gf.eid_touv[i].first, y = Gf.eid_touv[i].second;
        sedge eg{x, y};
        comu.emplace_back(eg);

        if (Gf.nodes[x].TE.size() > Gf.nodes[y].TE.size())                //这里没有保证是时态三角形，需要判断是否形成了时态三角形
          swap(x, y);
        for (auto &z_ifo: Gf.nodes[x].TE) {
          int z = z_ifo.first;
          if (x == z || y == z)
            continue;
          if (Gf.nodes[y].TE.find(z) != Gf.nodes[y].TE.end()) {
            int eidxz, eidyz;
            if (x < z && y < z) {
              if (non_temptri[G.vedge_set[min(x, y)][max(x, y)]].find(z) != non_temptri[G.vedge_set[min(x, y)][max(x,
                                                                                                                   y)]].end())            //trian_vis[i].find(z) != trian_vis[i].end() ||
                continue;
              //trian_vis[i].insert(z);
              eidxz = Gf.vedge_set[x][z];
              eidyz = Gf.vedge_set[y][z];
            } else if (x < y && z < y) {
              eidxz = x < z ? Gf.vedge_set[x][z] : Gf.vedge_set[z][x];
              if (non_temptri[G.vedge_set[min(x, z)][max(x, z)]].find(y) != non_temptri[G.vedge_set[min(x, z)][max(x,
                                                                                                                   z)]].end())        //trian_vis[eidxz].find(y) != trian_vis[eidxz].end() ||
                continue;
              //trian_vis[eidxz].insert(y);
              eidyz = Gf.vedge_set[z][y];
            } else if (z < x && y < x)//这里取eid取错了
            {
              eidyz = y < z ? Gf.vedge_set[y][z]
                            : Gf.vedge_set[z][y];        //trian_vis[eidyz].find(x) != trian_vis[eidyz].end() ||
              if (non_temptri[G.vedge_set[min(z, y)][max(z, y)]].find(x) != non_temptri[G.vedge_set[min(z, y)][max(z,
                                                                                                                   y)]].end())        //错误原因：这里的non_temptri存的是原始图的id，但是调用却用的Gf的eid(是重新编号的）
                continue;
              //trian_vis[eidyz].insert(x);
              eidxz = Gf.vedge_set[z][x];
            }

            if (!vis[eidxz])                                                    //有必要标记三角形被访问过吗，因为即使被访问过，后面在访问也只是执行两个判断？
            {
              que.push(eidxz);
              vis[eidxz] = true;
            }

            if (!vis[eidyz]) {
              que.push(eidyz);
              vis[eidyz] = true;
            }
          }
        }
      }
      if (comu.size() >= 3)
        solution.push_back(comu);

      //tgraph Gc;
      //vector<int> aedge;
      //for (auto& eg : comu)
      //	aedge.emplace_back(Gf.vedge_set[eg.first][eg.second]);
      //scan_subgraph(Gf, Gc, aedge, Gf.maxid);
      //count_global_sup(Gc, delta);
      //print_edge_unvalid(Gc, k);
    }
  }
}

//迭代删除的过程就需要考虑删除掉非时态三角形(删除条件：1. 是非时态三角形则立刻删除另外两边  2. 时态三角形当前边被删除，另外两边也不满足条件了
//策略：一开始就把所有的非时态三角形给删除(不能这样做，因为可能会导致连锁反应)（可能会得到好几个连通分量）
vector<vector<sedge>> global_search(tgraph &G, int q, int delta) {
  vector<vector<sedge>> solution;
  vector<unordered_set<int>> non_temptri;//用来记录非时态三角形, 这个要在计算sup值时确定（需要不同参数的count_global_sup）

  G.eedge_sup.clear();
  G.etriangle.clear();

  G.etriangle.resize(G.edge_num);
  G.eedge_sup.resize(G.edge_num, -1);
  non_temptri.resize(G.edge_num);

  int ansk = 0, maxsup;
  maxsup = count_global_sup(G, delta, non_temptri);
  vector<int> ans1 = shrink_prune(G, q, maxsup, delta, ansk, 0);
  check_connect(G, G, solution, ans1, q, delta, ansk, non_temptri);

  return solution;
}

void print_ans_to_file(vector<vector<sedge>> &ans, string &filename) {
  ofstream fout;
  fout.open(filename, ios::app);

  if (ans.empty())
    fout << "There is no qualified community!" << endl;

  for (auto &vec: ans) {
    fout << "edge_size: " << vec.size();
    set<int> node;
    for (auto &eg: vec) {
      node.insert(eg.first);
      node.insert(eg.second);
    }
    fout << " node_size: " << node.size() << endl;
    fout << "node: ";
    for (auto &id: node)
      fout << id << " ";
    fout << endl << "edge: ";
    for (auto &eg: vec) {
      fout << "(" << eg.first << "," << eg.second << ") ";
    }
    fout << endl;
  }
  fout.close();
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cerr << "The number of parameters is error!" << endl;
    cout << "Usage: ./global file_path" << endl;
    return 1;
  }

  // read graph
  string filePath = argv[1];
  std::vector<std::string> files = {
      "primary",
      "thiers",
      "email",
      "lyon",
      "mathoverflow",
      "facebook",
      "lkml",
      "enron",
      "twitter",
      "dblp"
  };
  for (auto &filename: files) {
    //read graph
    string file = filePath + filename + ".txt";
    tgraph G;
    if (!Readin(file, G)) {
      cerr << "The dataset is not exist! Please input again!" << endl;
      return 1;
    }

    time_t s_t, e_t;
    double time_taken = 0;
    //记录运行时间
    ofstream outfile("./runtime/" + filename, ios::app);
    outfile << "***GS***" << endl;

    int q, delta;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> qs(1, G.maxid);
    std::uniform_int_distribution<> deltas(G.tmin, G.tmax);

    int expCount = 100;
    file = "./ans/" + filename + "_GS";
    ofstream f;
    for (int i = 0; i < expCount; ++i) {
      // 生成一个随机数
      q = qs(gen);
      delta = deltas(gen);
      f.open(file, ios::app);
      f << "query node: " << q << " delta: " << delta << endl;
      f.close();
      s_t = clock();
      vector<vector<sedge>> ansg = global_search(G, q, delta);
      e_t = clock();
      time_taken += (double(e_t - s_t) / double(CLOCKS_PER_SEC)) * 1000;
      print_ans_to_file(ansg, file);
    }
    cout << "GS takes time: " << fixed << setprecision(5) << time_taken << "ms" << endl;
    outfile << "GS takes time: " << fixed << setprecision(5) << time_taken << "ms" << endl;

    outfile << endl;
    outfile.close();
  }

  return 0;
}
