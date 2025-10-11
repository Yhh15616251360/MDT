#pragma once
#include "define.h"
#include <fstream>
#include <iostream>

using namespace std;

//qMDT的查询结果写入文件
void print_ans_to_file(vector<int> &ans, string &filename, const int &q, const int &delta, const int &maxk) {
  ofstream fout;
  fout.open(filename, ios::app);
  if (!fout) {
    cerr << "result file is error!" << endl;
    //    return;
  }

  //对应原始图中的顶点编号
  int num = (int) vertices.size();
  vector<int> nodes;
  nodes.resize(num);
  for (auto &v: vertices) {
    nodes[v.second] = v.first;
  }

  fout << "query node: " << nodes[q] << " delta: " << delta << " maxk: " << maxk << endl;
  //  cout << "query node: " << q << " delta: " << delta << " maxk: " << maxk << endl;
  //  cout << ans.size() << endl;
  if (ans.empty()) {
    fout << "There is no qualified community!" << endl;
    fout.close();
    return;
  }

  sort(ans.begin(), ans.end());
  set<int> ansNode;
  for (auto &e: ans) {
    ansNode.insert(sEdges[e].source);
    ansNode.insert(sEdges[e].target);
  }

  fout << "edge_size: " << ans.size() << ", node_size: " << ansNode.size() << endl;
  for (auto &eid: ans) {
    fout << sEdges[eid].source << "," << sEdges[eid].target << "," << sEdges[eid].timestamps[0];
    for (int t = 1; t < sEdges[eid].timestamps.size(); t++) {
      fout << " " << sEdges[eid].timestamps[t];
    }
    fout << endl;
  }
  fout.close();

  //  cout << endl;

  //  vector<pair<int, int>> temp;
  //  for (int uid = 0; uid < adjEdges.size(); uid++) {
  //    temp.emplace_back(uid,(int) adjEdges[uid].size());
  //  }
  //  // 使用 std::sort 对 vector 进行排序，按 pair 的第二个元素排序
  //  std::sort(temp.begin(), temp.end(), [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
  //    return a.second < b.second;  // 按照第二个字段升序排序
  //  });
  //  // 输出排序后的结果
  //  for (const auto& p : temp) {
  //    std::cout << "(" << p.first << ", " << p.second << ") ";
  //  }

  //  for (auto &eid: adjEdges[q]) {
  //    cout << sEdges[eid].source << " " << sEdges[eid].target << " " << sEdges[eid].timestamps[0];
  //    for (int t = 1; t < sEdges[eid].timestamps.size(); t++) {
  //      cout << "," << sEdges[eid].timestamps[t];
  //    }
  //    cout << endl;
  //  }
}

//MDT的查询结果写入文件
void print_ans_to_file(vector<int> &ans, string &filename, const int &delta) {
  ofstream fout;
  fout.open(filename, ios::app);
  if (!fout) {
    cerr << "result file is error!" << endl;
    return;
  }

  fout << "delta: " << delta << "; maxk: " << deltaMaxK[delta] << endl;

  if (ans.empty()) {
    fout << "There is no qualified community!" << endl;
    fout.close();
    return;
  }

  sort(ans.begin(), ans.end());
  set<int> ansNode;
  for (auto &eid: ans) {
    sEdge &edge = sEdges[eid];
    ansNode.insert(edge.source);
    ansNode.insert(edge.target);
  }

  fout << "edge_size: " << ans.size() << ", node_size: " << ansNode.size() << endl;

  for (auto &eid: ans) {
    sEdge &edge = sEdges[eid];
    fout << edge.source << "," << edge.target << "," << edge.timestamps[0];
    for (int t = 1; t < edge.timestamps.size(); t++) {
      fout << " " << edge.timestamps[t];
    }
    fout << endl;
  }
  fout.close();
}

//边上的(delta，trussness)和构成三角形的最小delta都写入文件
int print_index_to_file(const string &eFile, const string &triFile) {
  ofstream wEdge(eFile, ios::out | ios::trunc);
  ofstream wSupplement(triFile, ios::out | ios::trunc);
  if (!wEdge || !wSupplement) {
    cerr << "index file is error!" << endl;
    //    return;
  }

  int eid = 0;
  int numIndex = 0;
  for (auto &e: sEdges) {
    wEdge << eid << " ";
    //    cout << eid << " ";
    //    int i = 0;
    for (auto &kd: e.deltaTruss) {
      wEdge << kd << " ";
      //      if (i % 2 == 0) cout << "(" << kd << ", ";
      //      if (i % 2 == 1) cout << kd << ") ";
      //      i++;
      numIndex++;
    }
    wEdge << endl;
    //    cout << endl;
    eid++;
  }
  wEdge.close();

  for (auto &k: deltaMaxK) {
    wSupplement << k << " ";
  }
  wSupplement << endl;

  for (auto &kEid: deltaKEdges) {
    for (auto &[k, edges]: kEid) {
      wSupplement << k << ":";
      for (auto &e: edges) {
        wSupplement << " " << e;
      }
      wSupplement << ";";
    }
    wSupplement << endl;
  }

  int triId = 0;
  for (auto &tri: sTriangles) {
    wSupplement << triId << " " << tri.minDelta << endl;
    triId++;
  }
  wSupplement.close();
  return numIndex / 2;
}

//只记录边上的(delta，trussness)
void print_index_to_file(const string &eFile) {
  ofstream wEdge(eFile, ios::out | ios::trunc);
  if (!wEdge) {
    cerr << "index file is error!" << endl;
    return;
  }

  int eid = 0;
  for (auto &e: sEdges) {
    wEdge << eid << " ";
    //    int i = 0;
    for (auto &kd: e.deltaTruss) {
      wEdge << kd << " ";
      //      if (i % 2 == 0) cout << "(" << kd << ",";
      //      else
      //        cout << kd << ") ";
      //      i++;
    }
    //    cout << endl;
    wEdge << endl;
    eid++;
  }
  wEdge.close();
}

void split_edges(const string &str, char split, int &num) {
  istringstream iss(str);// 输入流
  string token;          // 接收缓冲区
  int eid = -1, d = -1;
  while (getline(iss, token, split))// 以split为分隔符
  {
    if (eid == -1)
      eid = atoi(token.c_str());
    else if (d == -1) {
      d = atoi(token.c_str());
    } else {
      int k = atoi(token.c_str());
      sEdges[eid].deltaTruss.push_back(d);
      sEdges[eid].deltaTruss.push_back(k);
      num++;
      d = -1;
    }
  }
}

int recover_index(const string &filename) {
  char split = ' ';
  ifstream infile(filename);
  assert(infile);

  string esky;
  int numIndex = 0;
  while (getline(infile, esky)) {
    split_edges(esky, split, numIndex);
  }
  return numIndex;
}

void split_maxK(const string &str, char split) {
  istringstream iss(str);// 输入流
  string token;          // 接收缓冲区
  int i = 0;
  while (getline(iss, token, split)) {
    int maxk = atoi(token.c_str());
    deltaMaxK[i] = maxk;
    i++;
  }
}

void split_trussEid(const int &d, const string &str) {
  istringstream iss(str);// 输入流
  string token;          // 接收缓冲区
  while (getline(iss, token, ';')) {
    // 分割k和edges（按":"）
    size_t colonPos = token.find(':');
    if (colonPos == std::string::npos) continue;

    // 解析k值
    int k = std::stoi(token.substr(0, colonPos));
    // 解析edges（剩下的部分按空格分割）
    std::string edgesStr = token.substr(colonPos + 1);
    std::stringstream edgesStream(edgesStr);
    int e;
    while (edgesStream >> e) {
      deltaKEdges[d][k].push_back(e);
    }
  }
}

void recover_supplement(const string &filename, const int &interval) {
  ifstream infile(filename);
  assert(infile);

  string str;

  //解析deltaMaxK
  getline(infile, str);
  split_maxK(str, ' ');

  //解析deltaKEdges
  for (int i = 0; i < interval; ++i) {
    getline(infile, str);
    split_trussEid(i, str);
  }

  int id, minDelta;
  while (infile >> id >> minDelta) {
    sTriangles[id].minDelta = minDelta;
  }
}