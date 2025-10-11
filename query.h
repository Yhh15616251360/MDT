#pragma once

#include "comp_metric.h"
#include "define.h"
#include "global.h"
#include "index_search.h"
#include "local_search.h"
#include <iostream>
#include <sys/time.h>

void query_case(tGraph &G, string file) {
  int delta = 8;

  string index = "./index/" + file + "_edge.txt";
  string tri_index = "./index/" + file + "_tri.txt";
  string query_node = "./case_seed/" + file + ".txt";

  ifstream fin;
  fin.open(query_node);
  assert(fin);

  vector<int> q_node;
  int qnode;
  while (fin >> qnode) {
    if (fin.eof())
      break;
    q_node.push_back(qnode);
  }
  fin.close();

  recover_index(G, index);
  recover_triangle(G, tri_index);

  for (auto &q: q_node) {
    string ans_edge = "./ans/dblp_case/" + to_string(q) + "_" + to_string(delta) + ".txt";
    vector<vector<sedge>> ans = index_search(G, q, delta);

    ofstream fout;
    fout.open(ans_edge, ios::out | ios::trunc);

    for (auto &vec: ans) {
      set<int> node;
      for (auto &eg: vec) {
        node.insert(eg.first);
        node.insert(eg.second);
      }
      for (auto &id: node)
        fout << id << endl;

      fout << "*" << endl;

      for (auto &eg: vec) {
        fout << eg.first << " " << eg.second << endl;
      }
    }

    fout.close();
  }

}

//void query(tgraph& G, string file, int delta, double per)
void query_ans_varydelta(tGraph &G, string file, int delta) {
  cout << "hello" << endl;
  string index = "./index/" + file + "_edge.txt";
  string tri_index = "./index/" + file + "_tri.txt";
  string query_node = "./seed/" + file + "_seed.txt";


  //string ans_edge = "./ans/edge/" + file + "_edge_" + to_string(delta) + "_" +  ".txt";
  string ans_node = "./ans/node/" + file + "/" + file + "_node_" + to_string(delta) + "_" + ".txt";
  //string ans_metric = "./ans/metric/" + file + "_metric_" + to_string(delta) + "_" + ".txt";

  /*string ans_edge = "./ans/edge/" + file + "_edge_" + to_string(delta) + "_" + (to_string((int)(per * 100))) + ".txt";
  string ans_node = "./ans/node/" + file + "_node_" + to_string(delta) + "_" + (to_string((int)(per * 100))) + ".txt";
  string ans_metric = "./ans/metric/" + file + "_metric_" + to_string(delta) + "_" + (to_string((int)(per * 100))) + ".txt";*/


  ifstream fin;
  fin.open(query_node);
  assert(fin);
  vector<int> q_node;
  int qnode;
  while (fin >> qnode) {
    if (fin.eof())
      break;
    q_node.push_back(qnode);
  }
  fin.close();

  ofstream an;
  an.open(ans_node, ios::out | ios::trunc);
  assert(an);

  recover_index(G, index);
  recover_triangle(G, tri_index);
  int zero_ans = 0;
  double TC = 0.0, EDB = 0.0;
  int node_num = 0;

  timeval s_t, e_t;
  int index_time = 0;
  for (auto &q: q_node) {
    cout << "node" << endl;
    gettimeofday(&s_t, 0);
    vector<vector<sedge>> ans = index_search(G, q, delta);
    gettimeofday(&e_t, 0);
    index_time += (e_t.tv_sec - s_t.tv_sec) * 1000 + (e_t.tv_usec - s_t.tv_usec) / 1000;

    if (ans.size() == 0) {
      an << q << ":" << endl;
      zero_ans++;
      continue;
    }

    an << q << ":";
    for (auto &vec: ans) {
      set<int> node;
      for (auto &eg: vec) {
        node.insert(eg.first);
        node.insert(eg.second);
      }

      for (auto &id: node)
        an << id << " ";
      an << endl;
    }

  }
  an.close();
}

//void query(tgraph& G, string file, int delta, double per)
void query_ans(tGraph &G, string file, int delta) {
  string index = "./index/" + file + "_edge.txt";
  string tri_index = "./index/" + file + "_tri.txt";
  string query_node = "./seed/" + file + "_seed.txt";

  string ans_edge = "./ans/edge/" + file + "_edge_" + to_string(delta) + "_" + ".txt";
  string ans_node = "./ans/node/" + file + "_node_" + to_string(delta) + "_" + ".txt";
  string ans_metric = "./ans/metric/" + file + "_metric_" + to_string(delta) + "_" + ".txt";

  /*string ans_edge = "./ans/edge/" + file + "_edge_" + to_string(delta) + "_" + (to_string((int)(per * 100))) + ".txt";
  string ans_node = "./ans/node/" + file + "_node_" + to_string(delta) + "_" + (to_string((int)(per * 100))) + ".txt";
  string ans_metric = "./ans/metric/" + file + "_metric_" + to_string(delta) + "_" + (to_string((int)(per * 100))) + ".txt";*/


  ifstream fin;
  fin.open(query_node);
  assert(fin);

  vector<int> q_node;
  int qnode;
  while (fin >> qnode) {
    if (fin.eof())
      break;
    q_node.push_back(qnode);
  }
  fin.close();

  ofstream ae, an, am;
  ae.open(ans_edge, ios::out | ios::trunc);
  an.open(ans_node, ios::out | ios::trunc);
  am.open(ans_metric, ios::out | ios::trunc);
  assert(ae);
  assert(an);
  assert(am);
  am << "TC " << "EDB " << "node_num" << endl;



  /*for (auto& q:q_node)
  {
      G.vedge_sup.clear();
      G.vtriangle.clear();

      G.vtriangle.resize(G.maxid + 1);
      G.vedge_sup.resize(G.maxid + 1);

      //gettimeofday(&s_t, 0);
      vector<unordered_set<sedge, sedge_hash, sedge_equal>> ans1 = local_search_nok(G, q, delta);
      //gettimeofday(&e_t, 0);
      //local_time+= (e_t.tv_sec - s_t.tv_sec) * 1000 + (e_t.tv_usec - s_t.tv_usec) / 1000;

      print_ans(ans1);

      cout << endl << endl;
  }
  cout << endl << endl << endl;
  G.vedge_sup.clear();
  G.vtriangle.clear();*/


  recover_index(G, index);
  recover_triangle(G, tri_index);
  int zero_ans = 0;
  double TC = 0.0, EDB = 0.0;
  int node_num = 0;

  timeval s_t, e_t;
  int index_time = 0;
  for (auto &q: q_node) {
    gettimeofday(&s_t, 0);
    vector<vector<sedge>> ans = index_search(G, q, delta);
    gettimeofday(&e_t, 0);
    index_time += (e_t.tv_sec - s_t.tv_sec) * 1000 + (e_t.tv_usec - s_t.tv_usec) / 1000;

    if (ans.size() == 0) {
      ae << q << ":" << endl;
      an << q << ":" << endl;
      am << q << ":" << endl;
      zero_ans++;
      continue;
    }

    double tc = 0.0, edb = 0.0;
    int nn = 0;
    for (int i = 0; i < ans.size(); i++) {
      tc += comp_TC(G, ans[i]);
      edb += comp_EDB(G, ans[i]);
    }

    ae << q << ":";
    an << q << ":";
    for (auto &vec: ans) {
      set<int> node;
      for (auto &eg: vec) {
        ae << G.vedge_set[eg.first][eg.second] << " ";
        node.insert(eg.first);
        node.insert(eg.second);
      }
      ae << endl;

      for (auto &id: node)
        an << id << " ";
      an << endl;
      nn += node.size();
    }

    tc = (double) tc / ans.size();
    edb = (double) edb / ans.size();
    nn = nn / ans.size();

    TC += tc;
    EDB += edb;
    node_num += nn;

    am << q << ": " << tc << " " << edb << " " << nn << endl << endl;
  }
  double ATC, AEDB, Anode_num;
  ATC = (double) TC / (q_node.size() - zero_ans);
  AEDB = (double) EDB / (q_node.size() - zero_ans);
  Anode_num = (double) node_num / (q_node.size() - zero_ans);

  am << "zero_ans: " << zero_ans << endl;
  am << "avg_TC: " << ATC << endl;
  am << "avg_EDB: " << AEDB << endl;
  am << "avg_node_num: " << Anode_num << endl;
  ae.close();
  an.close();
  am.close();

}

void query_expand_ans_size(tGraph &G, string file, int delta) {
  string query_node = "./seed/" + file + "_seed.txt";

  ifstream fin;
  fin.open(query_node);
  assert(fin);

  vector<int> q_node;
  int qnode;
  while (fin >> qnode) {
    if (fin.eof())
      break;
    q_node.push_back(qnode);
  }
  fin.close();

  string file_x = "./ans/expand_size/" + file + "_expand_size.txt";
  ofstream exp;
  exp.open(file_x, ios::out | ios::trunc);
  assert(exp);
  exp << "query_node " << "expand_size " << "ans_size " << "round " << endl;
  exp.close();

  for (auto &q: q_node) {
    vector<vector<sedge>> ans1 = local_search(G, q, delta, file);
  }

}

void query_time_varydeg(tGraph &G, string file, int per)            //每次查询都是单独的，不依赖于之前已经计算过的边的sup值。
{
  int delta = 8;
  string index = "./index/" + file + "_edge.txt";
  string tri_index = "./index/" + file + "_tri.txt";
  string query_node = "./seed/" + file + "_seed_" + to_string(per) + ".txt";

  //使用默认的delta=8
  string comp_time = "./ans/time/" + file + "_time_" + to_string(per) + ".txt";

  int local_time = 0, index_time = 0, global_time = 0;

  ifstream fin;
  fin.open(query_node);
  assert(fin);

  vector<int> q_node;
  int qnode;
  while (fin >> qnode) {
    if (fin.eof())
      break;
    q_node.push_back(qnode);
  }
  fin.close();

  ofstream ct;
  ct.open(comp_time, ios::out | ios::trunc);

  timeval s_t, e_t;
  recover_index(G, index);
  recover_triangle(G, tri_index);

  ct << "index: " << endl;
  cout << "index: " << endl;
  for (auto &q: q_node) {
    ct << q << " ";
    gettimeofday(&s_t, 0);
    cout << endl;
    vector<vector<sedge>> ans = index_search(G, q, delta);
    gettimeofday(&e_t, 0);
    ct << (e_t.tv_sec - s_t.tv_sec) * 1000 + (e_t.tv_usec - s_t.tv_usec) / 1000 << "ms" << endl;
    index_time += (e_t.tv_sec - s_t.tv_sec) * 1000 + (e_t.tv_usec - s_t.tv_usec) / 1000;
  }
  ct << "total_index_search_time: " << index_time << "ms" << "  avg_time" << index_time / q_node.size() << "ms" << endl;

  ct << endl << endl << "local: " << endl;
  cout << endl << endl << "local: " << endl;
  for (auto &q: q_node) {
    ct << q << " ";
    cout << endl;
    gettimeofday(&s_t, 0);
    vector<vector<sedge>> ans1 = local_search(G, q, delta);
    gettimeofday(&e_t, 0);
    ct << (e_t.tv_sec - s_t.tv_sec) * 1000 + (e_t.tv_usec - s_t.tv_usec) / 1000 << "ms" << endl;
    local_time += (e_t.tv_sec - s_t.tv_sec) * 1000 + (e_t.tv_usec - s_t.tv_usec) / 1000;
  }
  ct << "total_local_search_time: " << local_time << "ms" << "  avg_time" << local_time / q_node.size() << "ms" << endl;

  ct << endl << endl << "global: " << endl;
  cout << endl << endl << "global: " << endl;
  /*for (auto& q : q_node)
  {
      ct << q << " ";
      cout << endl;
      gettimeofday(&s_t, 0);
      vector<vector<sedge>> ans1 = global_search(G, q, delta);
      gettimeofday(&e_t, 0);
      ct << (e_t.tv_sec - s_t.tv_sec) * 1000 + (e_t.tv_usec - s_t.tv_usec) / 1000 << "ms" << endl;
      global_time += (e_t.tv_sec - s_t.tv_sec) * 1000 + (e_t.tv_usec - s_t.tv_usec) / 1000;
  }*/

  ct << endl << endl << endl;
  ct << "total_global_search_time: " << global_time << "ms" << "  avg_time" << global_time / q_node.size() << "ms"
     << endl;
  ct << "total_local_search_time: " << local_time << "ms" << "  avg_time" << local_time / q_node.size() << "ms" << endl;
  ct << "total_index_search_time: " << index_time << "ms" << "  avg_time" << index_time / q_node.size() << "ms" << endl;

  ct.close();

  cout << endl;
  cout << "global_search_time: " << global_time << "ms" << "  avg_time" << global_time / q_node.size() << "ms" << endl;
  cout << "local_search_time: " << local_time << "ms" << "  avg_time" << local_time / q_node.size() << "ms" << endl;
  cout << "index_search_time: " << index_time << "ms" << "  avg_time" << index_time / q_node.size() << "ms" << endl;
}

void query_time(tGraph &G, string file, int delta)            //每次查询都是单独的，不依赖于之前已经计算过的边的sup值。
{
  string index = "./index/" + file + "_edge.txt";
  string tri_index = "./index/" + file + "_tri.txt";
  string query_node = "./seed/" + file + "_seed.txt";

  string comp_time = "./ans/time/" + file + "_time_" + to_string(delta) + ".txt";

  int local_time = 0, index_time = 0, global_time = 0;

  ifstream fin;
  fin.open(query_node);
  assert(fin);

  vector<int> q_node;
  int qnode;
  while (fin >> qnode) {
    if (fin.eof())
      break;
    q_node.push_back(qnode);
  }
  fin.close();

  ofstream ct;
  ct.open(comp_time, ios::out | ios::trunc);

  timeval s_t, e_t;
  recover_index(G, index);
  recover_triangle(G, tri_index);

  ct << "index: " << endl;
  cout << "index: " << endl;
  for (auto &q: q_node) {
    ct << q << " ";
    gettimeofday(&s_t, 0);
    cout << endl;
    vector<vector<sedge>> ans = index_search(G, q, delta);
    gettimeofday(&e_t, 0);
    ct << (e_t.tv_sec - s_t.tv_sec) * 1000 + (e_t.tv_usec - s_t.tv_usec) / 1000 << "ms" << endl;
    index_time += (e_t.tv_sec - s_t.tv_sec) * 1000 + (e_t.tv_usec - s_t.tv_usec) / 1000;
  }
  ct << "total_index_search_time: " << index_time << "ms" << "  avg_time" << index_time / q_node.size() << "ms" << endl;

  ct << endl << endl << "local: " << endl;
  cout << endl << endl << "local: " << endl;
  for (auto &q: q_node) {
    ct << q << " ";
    cout << endl;
    gettimeofday(&s_t, 0);
    vector<vector<sedge>> ans1 = local_search(G, q, delta);
    gettimeofday(&e_t, 0);
    ct << (e_t.tv_sec - s_t.tv_sec) * 1000 + (e_t.tv_usec - s_t.tv_usec) / 1000 << "ms" << endl;
    local_time += (e_t.tv_sec - s_t.tv_sec) * 1000 + (e_t.tv_usec - s_t.tv_usec) / 1000;
  }
  ct << "total_local_search_time: " << local_time << "ms" << "  avg_time" << local_time / q_node.size() << "ms" << endl;

  ct << endl << endl << "global: " << endl;
  cout << endl << endl << "global: " << endl;
  for (auto &q: q_node) {
    ct << q << " ";
    cout << endl;
    gettimeofday(&s_t, 0);
    vector<vector<sedge>> ans1 = global_search(G, q, delta);
    gettimeofday(&e_t, 0);
    ct << (e_t.tv_sec - s_t.tv_sec) * 1000 + (e_t.tv_usec - s_t.tv_usec) / 1000 << "ms" << endl;
    global_time += (e_t.tv_sec - s_t.tv_sec) * 1000 + (e_t.tv_usec - s_t.tv_usec) / 1000;
  }

  ct << endl << endl << endl;
  ct << "total_global_search_time: " << global_time << "ms" << "  avg_time" << global_time / q_node.size() << "ms"
     << endl;
  ct << "total_local_search_time: " << local_time << "ms" << "  avg_time" << local_time / q_node.size() << "ms" << endl;
  ct << "total_index_search_time: " << index_time << "ms" << "  avg_time" << index_time / q_node.size() << "ms" << endl;

  ct.close();

  cout << endl;
  cout << "global_search_time: " << global_time << "ms" << "  avg_time" << global_time / q_node.size() << "ms" << endl;
  cout << "local_search_time: " << local_time << "ms" << "  avg_time" << local_time / q_node.size() << "ms" << endl;
  cout << "index_search_time: " << index_time << "ms" << "  avg_time" << index_time / q_node.size() << "ms" << endl;
}

void query_try(tGraph &G, string file, int delta) {

  string index = "./index/" + file + "_edge.txt";
  string tri_index = "./index/" + file + "_tri.txt";
  string query_node = "./seed/" + file + "_seed.txt";

  ifstream fin;
  fin.open(query_node);
  assert(fin);

  vector<int> q_node;
  int qnode;
  while (fin >> qnode) {
    if (fin.eof())
      break;
    q_node.push_back(qnode);
  }
  fin.close();

  for (auto &q: q_node) {
    vector<vector<sedge>> ans1 = local_search(G, q, delta);
    print_ans_to_file(ans1);

  }

  G.vedge_sup.clear();
  G.vtriangle.clear();

  recover_index(G, index);
  recover_triangle(G, tri_index);
  int zero_ans = 0;
  double TC = 0.0, EDB = 0.0;
  int node_num = 0;
  for (auto &q: q_node) {
    vector<vector<sedge>> ans = index_search(G, q, delta);
    print_ans_to_file(ans);
  }

}

void query_try_q(tGraph &G, string file, int delta, int q) {

  string index = "./index/" + file + "_edge.txt";
  string tri_index = "./index/" + file + "_tri.txt";
  timeval s_t, e_t;

  gettimeofday(&s_t, 0);
  vector<vector<sedge>> ansg = global_search(G, q, delta);
  gettimeofday(&e_t, 0);
  cout << "GS takes time: " << (e_t.tv_sec - s_t.tv_sec) * 1000 + (e_t.tv_usec - s_t.tv_usec) / 1000 << "ms" << endl;
  print_ans_to_file(ansg);
  cout << endl;

  gettimeofday(&s_t, 0);
  vector<vector<sedge>> ansl = local_search(G, q, delta);
  gettimeofday(&e_t, 0);
  cout << "LS takes time: " << (e_t.tv_sec - s_t.tv_sec) * 1000 + (e_t.tv_usec - s_t.tv_usec) / 1000 << "ms" << endl;
  print_ans_to_file(ansl);
  cout << endl;

  recover_index(G, index);
  recover_triangle(G, tri_index);

  gettimeofday(&s_t, 0);
  vector<vector<sedge>> ansi = index_search(G, q, delta);
  gettimeofday(&e_t, 0);
  cout << "IS takes time: " << (e_t.tv_sec - s_t.tv_sec) * 1000 + (e_t.tv_usec - s_t.tv_usec) / 1000 << "ms" << endl;
  print_ans_to_file(ansi);

}

int comp_node(vector<int> &vec1, vector<int> &vec2) {
  if (vec1[1] == vec2[1])
    return vec1[0] < vec2[0];
  else
    return vec1[1] < vec2[1];
}

void select_seed_bytemp(tGraph &G, string file, double per) {

  string path = "./seed/" + file + "_seed_" + (to_string((int) (per * 100))) + ".txt";

  vector<vector<int>> sorted_node;

  for (int i = 0; i < G.maxid + 1; i++) {
    vector<int> vec(2);
    if (G.nodes[i].TE.size() == 0)            //度数为0就不管了
      continue;
    vec[0] = i;
    int temp_deg = 0;
    for (auto &mp: G.nodes[i].TE) {
      temp_deg += mp.second.TS.size();
    }
    vec[1] = temp_deg;

    sorted_node.push_back(vec);
  }

  sort(sorted_node.begin(), sorted_node.end(), comp_node);

  set<int> query_set;

  srand((unsigned) time(NULL));

  int round_num = 100;
  int n = sorted_node.size();

  ofstream seed;
  seed.open(path, ios::out | ios::trunc);
  assert(seed);

  int mnum2 = (int) (per * n), mnum1 = (int) (n * (double) (per - 0.25));
  while (round_num) {
    int q_index = 0;
    int q;
    while (1) {
      q_index = rand() % (mnum2 - mnum1) + mnum1;

      q = sorted_node[q_index][0];
      if (query_set.find(q) == query_set.end())
        break;

    }

    seed << q << endl;
    round_num--;
  }

  seed.close();
}

void select_seed(tGraph &G, string file) {

  string path = "./seed/" + file + "_seed" + ".txt";

  vector<vector<int>> sorted_node;

  for (int i = 0; i < G.maxid + 1; i++) {
    vector<int> vec(2);
    if (G.nodes[i].TE.size() == 0)            //度数为0就不管了
      continue;
    vec[0] = i;
    int temp_deg = 0;
    for (auto &mp: G.nodes[i].TE) {
      temp_deg += mp.second.TS.size();
    }
    vec[1] = temp_deg;

    sorted_node.push_back(vec);
  }

  sort(sorted_node.begin(), sorted_node.end(), comp_node);

  set<int> query_set;

  srand((unsigned) time(NULL));

  int n = sorted_node.size();

  ofstream seed;
  seed.open(path, ios::out | ios::trunc);
  assert(seed);
  vector<double> percent = {0.25, 0.5, 0.75, 1.0};
  for (double &per: percent) {
    int mnum2 = (int) (per * n), mnum1 = (int) (n * (double) (per - 0.25));
    int round_num = 25;
    while (round_num) {
      int q_index = 0;
      int q;
      while (1) {
        q_index = rand() % (mnum2 - mnum1) + mnum1;

        q = sorted_node[q_index][0];
        if (query_set.find(q) == query_set.end())
          break;

      }
      query_set.insert(q);
      seed << q << endl;
      round_num--;
    }
  }

  seed.close();
}

void select_seed_varydeg(tGraph &G, string file) {

  vector<vector<int>> sorted_node;

  for (int i = 0; i < G.maxid + 1; i++) {
    vector<int> vec(2);
    if (G.nodes[i].TE.size() == 0)            //度数为0就不管了
      continue;
    vec[0] = i;
    int temp_deg = 0;
    for (auto &mp: G.nodes[i].TE) {
      temp_deg += mp.second.TS.size();
    }
    vec[1] = temp_deg;

    sorted_node.push_back(vec);
  }

  sort(sorted_node.begin(), sorted_node.end(), comp_node);

  srand((unsigned) time(NULL));

  int n = sorted_node.size();

  vector<double> percent = {0.2, 0.4, 0.6, 0.8, 1.0};
  for (double &per: percent) {
    string path = "./seed/" + file + "_seed_" + to_string((int) (per * 100)) + ".txt";

    ofstream seed;
    seed.open(path, ios::out | ios::trunc);
    assert(seed);

    set<int> query_set;

    int mnum2 = (int) (per * n), mnum1 = (int) (n * (double) (per - 0.2));
    int round_num = 100;
    while (round_num) {
      int q_index = 0;
      int q;
      while (1) {
        q_index = rand() % (mnum2 - mnum1) + mnum1;

        q = sorted_node[q_index][0];
        if (query_set.find(q) == query_set.end())
          break;

      }

      seed << q << endl;
      query_set.insert(q);
      round_num--;
    }

    seed.close();
  }
}

void select_seed_degslice(tGraph &G, string file) {

  vector<vector<int>> sorted_node;

  int maxdeg = 0;

  for (int i = 0; i < G.maxid + 1; i++) {
    vector<int> vec(2);
    if (G.nodes[i].TE.size() == 0)            //度数为0就不管了
      continue;
    vec[0] = i;
    int temp_deg = 0;
    for (auto &mp: G.nodes[i].TE) {
      temp_deg += mp.second.TS.size();
    }
    maxdeg = max(maxdeg, temp_deg);
    vec[1] = temp_deg;
    sorted_node.push_back(vec);
  }

  double punit = (double) maxdeg / 5;
  vector<vector<int>> buck_deg(5);

  for (auto &vec: sorted_node) {
    int ddeg = vec[1], nid = vec[0];
    int loc = ddeg / punit;
    if (loc == 5)
      buck_deg[loc - 1].emplace_back(nid);
    else
      buck_deg[loc].emplace_back(nid);
  }

  vector<double> per = {0.2, 0.4, 0.6, 0.8, 1.0};

  for (int i = 0; i < 5; i++) {
    srand((unsigned) time(NULL));

    string path = "./seed/seed_degslice/" + file + "_seed_" + (to_string((int) (per[i] * 100))) + ".txt";
    ofstream seed;
    seed.open(path, ios::out | ios::trunc);
    assert(seed);
    set<int> query_set;
    int round_num = 100;
    int nnum = buck_deg[i].size();

    while (round_num) {
      int q_index = 0;
      int q;
      while (1) {
        q_index = rand() % nnum;

        q = buck_deg[i][q_index];
        if (query_set.find(q) == query_set.end())
          break;

      }
      seed << q << endl;
      query_set.insert(q);
      round_num--;
    }
    seed.close();
  }

}

void select_seed_withtruss(tGraph &G, string file) {

  string index = "./index/" + file + "_edge.txt";
  string tri_index = "./index/" + file + "_tri.txt";

  recover_index(G, index);
  recover_triangle(G, tri_index);

  vector<vector<int>> sorted_node;

  for (int i = 0; i < G.maxid + 1; i++) {
    vector<int> vec(2);
    if (G.nodes[i].TE.size() == 0)            //度数为0就不管了
      continue;
    vec[0] = i;
    int temp_deg = 0;
    for (auto &mp: G.nodes[i].TE) {
      temp_deg += mp.second.TS.size();
    }
    vec[1] = temp_deg;

    sorted_node.push_back(vec);
  }

  sort(sorted_node.begin(), sorted_node.end(), comp_node);

  srand((unsigned) time(NULL));

  int n = sorted_node.size();
  int delta = 8;

  vector<double> percent = {0.2, 0.4, 0.6, 0.8, 1.0};
  for (double &per: percent) {
    string path = "./seed/seed_truss/" + file + "_seed_" + to_string((int) (per * 100)) + ".txt";

    ofstream seed;
    seed.open(path, ios::out | ios::trunc);
    assert(seed);

    set<int> query_set;

    int mnum2 = (int) (per * n), mnum1 = (int) (n * (double) (per - 0.2));
    int round_num = 100;
    while (round_num) {
      int q_index = 0;
      int q;
      while (1) {
        q_index = rand() % (mnum2 - mnum1) + mnum1;
        int flag = 0;

        q = sorted_node[q_index][0];
        if (query_set.find(q) == query_set.end()) {
          for (auto &v_ifo: G.nodes[q].TE) {
            int u = q, v = v_ifo.first;
            if (u > v)
              swap(u, v);
            int eid = G.vedge_set[u][v];

            int mk = find_index(G, eid, delta);
            cout << mk << endl;
            if (mk >= 1) {
              flag = 1;
              break;
            }
          }
          if (flag)
            break;
        }

      }

      seed << q << endl;
      query_set.insert(q);
      round_num--;
    }

    seed.close();
  }
}