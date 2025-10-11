#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;
using ui = int;

map<int, int> vertices;//确保输入图的顶点从0开始

vector<vector<int>> singleTime;//单个时刻的子图，内层vector存储的是边id
vector<int> deltaMaxK;         //不同delta下，图中最大的边trussness
vector<unordered_map<int, vector<int>>> deltaKEdges; //不同delta下，相同k值对应的边

//单向存储所有静态边。静态边起点id为offset的位置
vector<int> offset; //每个起点在targets中的偏移量
vector<int> targets;//所有静态边的终点。

// 双向存储每个顶点的邻接边，便于后续根据给定顶点查询社区
vector<vector<int>> adjEdges;

int tMin, tMax;//图中最小的时刻和最大的时刻

//静态边
class sEdge {
 public:
  vector<int> triangles; //存储时三个一组，每组为三角形编号和构成三角形的另外两条边
  vector<int> timestamps;//边上时间戳
  vector<int> deltaTruss;//输入时两个一组，(delta, trussness)
  vector<int> weights;  //在各个子图中的权重
  int support;           //索引构建时为该边在delta下的support，查询时表示该边是否访问过
  int source, target;

  sEdge(int u, int v) {
    source = u;
    target = v;
    timestamps.clear();
    support = 0;
  }
};
vector<sEdge> sEdges;

//静态三角形
class sTriangle {
 public:
  int e1, e2, e3;//构成三角形的三条边
  int minDelta;  //形成三角形所需的最小δ
  int deltaTri;  //delta三角形的个数
  int sumDiff;   //累积的时序三角形个数差
  int sumDiff_0; //第一个子图下的时序delta三角形个数差
  int sub0Temp;  //第一个子图下时序delta三角形个数
  int subMaxTemp;//记录最后一个子图中时序三角形的个数
  int sumTemps;  //除去最后一个子图的时序三角形总数

  sTriangle(int a, int b, int c) {
    e1 = a;
    e2 = b;
    e3 = c;
    minDelta = -1;
    deltaTri = 0;
    sumDiff = 0;
    sumDiff_0 = 0;
    sub0Temp = 0;
    subMaxTemp = 0;
    sumTemps = 0;
  }
};
vector<sTriangle> sTriangles;

class subGraph {
 public:
  unordered_set<int> edges; //边id
//  unordered_map<int, int> wEdges;   及其在子图中的权重
  vector<int> triangles;//静态三角形编号
};

//自定义一个可修改的堆，用于decompose阶段修改support值之后还能弹出最小值
class ModifiableHeap {
 private:
  using Pair = std::pair<int, int>;
  std::vector<Pair> heap;
  std::unordered_map<int, std::unordered_map<int, size_t>> pos; // first → (second → 位置)    // 值到堆位置的映射
  std::unordered_map<int, int> second_to_first; // second → first

  // 自定义比较函数：先按 first，再按 second
  bool is_less(const Pair &a, const Pair &b) const {
    if (a.first != b.first) return a.first < b.first;
    return a.second < b.second; // first 相同时按 second 升序
  }

  void heapify_up(size_t i) {
    while (i > 0) {
      size_t parent = (i - 1) / 2;
      if (!is_less(heap[i], heap[parent])) break; // 使用自定义比较
      std::swap(heap[i], heap[parent]);
      pos[heap[i].first][heap[i].second] = i;
      pos[heap[parent].first][heap[parent].second] = parent;
      i = parent;
    }
  }

  void heapify_down(size_t i) {
    size_t n = heap.size();
    while (true) {
      size_t left = 2 * i + 1;
      size_t right = 2 * i + 2;
      size_t smallest = i;
      if (left < n && is_less(heap[left], heap[smallest])) smallest = left;
      if (right < n && is_less(heap[right], heap[smallest])) smallest = right;
      if (smallest == i) break;
      std::swap(heap[i], heap[smallest]);
      pos[heap[i].first][heap[i].second] = i;
      pos[heap[smallest].first][heap[smallest].second] = smallest;
      i = smallest;
    }
  }

 public:
  // 插入元素（确保 (first, second) 唯一）
  void push(const Pair &p) {
    if (pos.count(p.first) && pos[p.first].count(p.second)) {
      throw std::runtime_error("Duplicate (first, second) pair");
    }
    heap.push_back(p);
    pos[p.first][p.second] = heap.size() - 1;
    second_to_first[p.second] = p.first;
    heapify_up(heap.size() - 1);
  }

  // 修改元素（需提供旧值和新值）
  void modify(const Pair &old_val, const Pair &new_val) {
    if (!pos.count(old_val.first) || !pos[old_val.first].count(old_val.second)) {
      throw std::runtime_error("Pair not found in heap");
    }
    size_t i = pos[old_val.first][old_val.second];
    heap[i] = new_val;
    pos[old_val.first].erase(old_val.second);
    second_to_first.erase(old_val.second);
    pos[new_val.first][new_val.second] = i;
    second_to_first[new_val.second] = new_val.first;
    if (is_less(new_val, old_val)) heapify_up(i);
    else heapify_down(i);
  }

  // 弹出最小值
  Pair pop_min() {
    if (heap.empty()) {
      throw std::runtime_error("Heap is empty");
    }
    Pair min_val = heap[0];
    pos[min_val.first].erase(min_val.second);
    second_to_first.erase(min_val.second);
    heap[0] = heap.back();
    heap.pop_back();
    if (!heap.empty()) {
      pos[heap[0].first][heap[0].second] = 0;
      second_to_first[heap[0].second] = heap[0].first;
      heapify_down(0);
    }
    return min_val;
  }

  // 根据 second 获取 first
  int get_support(const int &second_val) const {
    auto it = second_to_first.find(second_val);
    if (it == second_to_first.end()) {
      throw std::runtime_error("Second value not found in heap");
    }
    return it->second;
  }

  // 堆是否为空
  bool empty() const {
    return heap.empty();
  }

  // 堆大小
  size_t size() const {
    return heap.size();
  }
};