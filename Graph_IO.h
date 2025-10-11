/*
* Modified by Yang on 2025/1/6.
*/
#pragma once
#include "fstream"
#include <cassert>
//#include "set"
#include "define.h"
//#include "vector"
#include <cstdint>
#include <map>
#include <set>
#include <sstream>

using namespace std;

/* 功能:
 * 1. 将输入图按照u，v，t排序
 * 2. 获取顶点、静态边、动态边、时间戳个数和时间戳范围
 * 3. 通过边id存储单个时刻的子图
 * 4. CSR方式存储无向边（每条边只存储1次，u<v）
*/
bool readFile(const string &filename, int &v_num, int &s_edges, int &t_edges, int &t_num) {
  fstream file;
  file.open(filename, ios::in);
  assert(file);
  if (!file.is_open()) {
    return false;
  }

  string line;
  map<int, int>::iterator it;
  tMin = INT32_MAX, tMax = INT32_MIN;
  vector<map<int, set<int>>> edges;//时态边，起点id为点id
  while (getline(file, line)) {
    istringstream iss(line);
    std::vector<std::string> tokens;
    std::string token;

    // 以空格划分每一行，并存储到 tokens 向量中
    while (iss >> token) {
      tokens.push_back(token);
    }

    int u, v, t;
    if (tokens.empty()) continue;
    try {
      // 尝试将变量转换为整数
      u = std::stoi(tokens[0]);
      v = std::stoi(tokens[1]);
      t = std::stoi(tokens[2]);
    } catch (const std::exception &e) {
      // 如果无法转换为整数，则跳过该行
      continue;
    }

    if (u == v) continue;

    it = vertices.find(u);
    if (it == vertices.end()) {
      v_num = (int) vertices.size();
      vertices[u] = v_num;
      u = v_num;
      // 创建一个空的 map<int, set<int>>
      map<int, set<int>> emptyMap;
      // 将空的 map 推入到 vector 中
      edges.push_back(emptyMap);
    } else {
      u = it->second;
    }

    it = vertices.find(v);
    if (it == vertices.end()) {
      v_num = (int) vertices.size();
      vertices[v] = v_num;
      v = v_num;
      // 创建一个空的 map<int, set<int>>
      map<int, set<int>> emptyMap;
      // 将空的 map 推入到 vector 中
      edges.push_back(emptyMap);
    } else {
      v = it->second;
    }

    if (v < u) {
      edges[v][u].insert(t);
    } else {
      edges[u][v].insert(t);
    }

    if (t < tMin) tMin = t;
    if (t > tMax) tMax = t;
  }
  file.close();

  v_num += 1;
  offset.resize(v_num);
  t_num = tMax - tMin + 1;
  singleTime.resize(t_num);
  adjEdges.resize(v_num);
  //The number of static edges, temporal edges and vertices.
  int uid = 0;
  for (auto &eid: edges) {
    for (const auto &item: eid) {
      int vid = item.first;
      targets.push_back(vid);
      //构建邻接表，点-邻接边集
      adjEdges[uid].push_back(s_edges);
      adjEdges[vid].push_back(s_edges);

      //记录当前静态边上时间戳的个数
      sEdge et(uid, vid);
      for (const auto &t: item.second) {
        //构建单个时刻的子图，边集
        singleTime[t - tMin].push_back(s_edges);
        et.timestamps.push_back(t);
        t_edges++;
      }
      sEdges.push_back(et);
      s_edges++;
    }
    offset[uid] = s_edges;
    uid++;
  }

  return true;
}

// 找出图中所有三角形并存储
int findAllTriangles(const int &n) {
  int nTri = 0;//三角形的编号
  // 由于邻接点是有序的，因此可以使用归并算法查找交集
  for (int u = 0; u < n; u++) {
    int vid = 0;
    if (u != 0) vid = offset[u - 1];
    for (; vid < offset[u]; vid++) {
      int v = targets[vid];
      int i = vid + 1, j = offset[v - 1];
      while (i < offset[u] && j < offset[v]) {
        if (targets[i] == targets[j]) {
          // 找到共同的邻接点，构成一个三角形
          int e1 = vid, e2 = i, e3 = j;
          sEdges[e1].triangles.push_back(nTri);
          sEdges[e1].triangles.push_back(e2);
          sEdges[e1].triangles.push_back(e3);
          sEdges[e2].triangles.push_back(nTri);
          sEdges[e2].triangles.push_back(e1);
          sEdges[e2].triangles.push_back(e3);
          sEdges[e3].triangles.push_back(nTri);
          sEdges[e3].triangles.push_back(e1);
          sEdges[e3].triangles.push_back(e2);
          sTriangle tri(e1, e2, e3);
          sTriangles.push_back(tri);
          i++;
          j++;
          nTri++;
        } else if (targets[i] < targets[j]) {
          // 如果 adj_u[i] 小于 adj_v[j]，则移动 i
          i++;
        } else {
          // 如果 adj_u[i] 大于 adj_v[j]，则移动 j
          j++;
        }
      }
    }
  }
  return nTri;//返回全图中静态三角形的个数
}
