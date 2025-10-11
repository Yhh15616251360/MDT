#pragma once
#include "define.h"
#include "print.h"
//Linux下使用signal信号机制设置定时器终止程序时的头文件
#include <csignal>
#include <limits>
#include <unistd.h>

using namespace std;

//Linux下的超时函数
void timeout_handler(int sig) {
  std::cout << "Time out!!! It is terminating..." << std::endl;
  exit(1);// 超时后退出程序
}

inline void insert_edges(const int &t,
                         subGraph &sub_graph,
                         const int &delta,
                         unordered_map<int, int> &allTri) {
  auto &subEdges = sub_graph.edges;
  for (auto &eid: singleTime[t + delta]) {
    // 向子图中插入边
    auto[it, inserted] = subEdges.insert(eid);
    // 若插入不成功，说明存在该边，则边的权重加1
    if (!inserted) {
      sEdges[eid].weights[t]++;
      continue;
    } else {
      sEdges[eid].weights[t] = 1;
    }
    // 判断子图中是否有包含新边的三角形
    auto &triangles = sEdges[eid].triangles;
    for (size_t i = 0; i < triangles.size(); i += 3) {
      int tri = triangles[i];
      int e2 = triangles[i + 1], e3 = triangles[i + 2];
      if (subEdges.find(e2) != subEdges.end() && subEdges.find(e3) != subEdges.end()) {
        sub_graph.triangles.push_back(tri);
        //形成新的三角形则记录当前delta
        auto result = allTri.insert({tri, 0});
        if (result.second) sTriangles[tri].minDelta = delta;
      }
    }

    /*
    //若子图中不存在该边，则插入这条边
    if (sub_graph.wEdges.find(eid) == sub_graph.wEdges.end()) {
      // 设新加入的边的权重为1
      sub_graph.wEdges[eid] = 1;
      //判断子图中是否有包含该边的三角形
      for (int i = 0; i < sEdges[eid].triangles.size(); i += 3) {
        int tri = sEdges[eid].triangles[i];
        int e2 = sEdges[eid].triangles[i + 1], e3 = sEdges[eid].triangles[i + 2];
        if (sub_graph.wEdges.find(e2) != sub_graph.wEdges.end()
            && sub_graph.wEdges.find(e3) != sub_graph.wEdges.end()) {
          sub_graph.triangles[tri] = 0;
          //形成新的三角形则记录当前delta
          auto result = allTri.insert({tri, 0});
          if (result.second) sTriangles[tri].minDelta = delta;
        }
      }
    } else {
      sub_graph.wEdges[eid]++;
    }
     */
  }
}

void compute_support(vector<subGraph> &subGraphs, unordered_map<int, int> &allTri, const int &delta) {
  int subMax = (int) subGraphs.size();

  for (int t = 0; t < subMax; ++t) {
    subGraph &subG = subGraphs[t];
    //跳过空的时刻子图
    if (delta != 0 && subG.edges.empty()) continue;
    insert_edges(t, subG, delta, allTri);
    //计算子图中每个静态三角形对应的时序三角形的个数
    for (auto &tri: subG.triangles) {
      sTriangle &sTri = sTriangles[tri];
      int w1 = sEdges[sTri.e1].weights[t],
          w2 = sEdges[sTri.e2].weights[t],
          w3 = sEdges[sTri.e3].weights[t];
      int w = w1 * w2 * w3;
      sTri.sumDiff += w;
      if (t == 0) {
        sTri.sumDiff_0 = w - sTri.sub0Temp;
        sTri.sub0Temp = w;
      }
      if (t == subMax - 1) {
        sTri.sumTemps -= sTri.subMaxTemp;
        sTri.subMaxTemp = w;
      }
    }
  }

  for (auto &tri: allTri) {
    sTriangle &sTri = sTriangles[tri.first];
    int sumTemps = sTri.sumDiff;
    sTri.sumDiff -= sTri.sumTemps;
    //如果没有增量，则无需处理该三角形的边
    if (sTri.sumDiff == 0) continue;

    int diff = sTri.sumDiff - tri.second;
    // 计算三角形在整个图中的support
    sTri.deltaTri += diff;
    // 计算三角形每条边的support
    int e1 = sTri.e1, e2 = sTri.e2, e3 = sTri.e3;
    sEdges[e1].support += diff;
    sEdges[e2].support += diff;
    sEdges[e3].support += diff;

    // 更新和备份sumDiff，用于下一轮计算
    tri.second = sTri.sumDiff - sTri.sumDiff_0;
    sTri.sumDiff = 0;
    sTri.sumTemps = sumTemps;
  }
}

void save_trussness(const int &delta, const int &k, const int &eid) {
  sEdge &e = sEdges[eid];
  //trussness为k+2的边集
  deltaKEdges[delta][k + 2].push_back(eid);
  if (e.deltaTruss.empty()) {
    e.deltaTruss.push_back(delta);
    e.deltaTruss.push_back(k + 2);
  } else {
    int k1 = e.deltaTruss.back();
    // Q: 这是一种简化吗？可以保证delta越大的时候边的truss值不递减吗？
    // A: 是的，因为就算delta不变时，其truss值也会和原来保持相同
    if (k + 2 > k1) {
      e.deltaTruss.push_back(delta);
      e.deltaTruss.push_back(k + 2);
    }
  }
}

void decompose(const int &d) {
  /*
  int edge_num = (int) sEdges.size();
  vector<bool> edge_del(edge_num, false);//将所有边标记为未处理

  //记录所有边的support
  vector<int> Keys(edge_num);
  for (int i = 0; i < edge_num; ++i) {
    //若当前边的support为0，说明是孤立的边，直接剥离
    if (sEdges[i].support == 0) edge_del[i] = true;
    Keys[i] = sEdges[i].support;
  }

  while (true) {
    int sup_min = std::numeric_limits<int>::max();
    int eid_min = -1;
    //寻找未被剥离的、具有最小support的边
    for (int i = 0; i < edge_num; ++i) {
      if (edge_del[i]) continue;
      if (sup_min > Keys[i]) {
        sup_min = Keys[i];
        eid_min = i;
      }
    }
    if (eid_min == -1) break;

    // 保存当前delta下最大的trussness
    deltaMaxK[d] = sup_min + 2;

    //剥离该边造成的影响
    edge_del[eid_min] = true;
    save_trussness(d, sup_min, eid_min);//合并具有包含关系的(k,truss)对
    sEdge &e = sEdges[eid_min];
    for (int i = 0; i < e.triangles.size(); i += 3) {
      int e1 = e.triangles[i + 1], e2 = e.triangles[i + 2];
      //另外两条边都未被处理时，说明删除该边会影响到三角形的存在
      if (!edge_del[e1] && !edge_del[e2]) {
        int dec = sTriangles[e.triangles[i]].deltaTri;
        Keys[e1] = Keys[e1] - dec, Keys[e2] = Keys[e2] - dec;

        if (Keys[e1] < sup_min) {
          Keys[e1] = sup_min;
        }
        if (Keys[e2] < sup_min) {
          Keys[e2] = sup_min;
        }
      }
    }
  }
   */

  //优化后的版本
  int edgeNum = (int) sEdges.size();
  //初始化所有边未被处理
  vector<char> edgeDeal(edgeNum, 0);
  // 初始化堆并插入元素
  ModifiableHeap minSupport;
  for (int i = 0; i < edgeNum; ++i) {
    //若当前边的support为0，说明是孤立的边，直接剥离
    if (sEdges[i].support == 0) {
      edgeDeal[i] = 1;
      //trussness为2的边集
//      deltaKEdges[d][2].push_back(i);
      continue;
    }
    minSupport.push({sEdges[i].support, i});
  }

  while (!minSupport.empty()) {
    auto minValue = minSupport.pop_min();
    int sup_min = minValue.first, eid_min = minValue.second;

    // 保存当前delta下最大的trussness
    deltaMaxK[d] = sup_min + 2;

    //剥离该边造成的影响
    edgeDeal[eid_min] = 1;              //该边已被处理
    save_trussness(d, sup_min, eid_min);//合并具有包含关系的(k,truss)对

    sEdge &e = sEdges[eid_min];
    for (int i = 0; i < e.triangles.size(); i += 3) {
      int e1 = e.triangles[i + 1], e2 = e.triangles[i + 2];
      //另外两条边都未被处理时，说明删除该边会影响到三角形的存在
      if (!edgeDeal[e1] && !edgeDeal[e2]) {
        int dec = sTriangles[e.triangles[i]].deltaTri;
        int oldSup1 = minSupport.get_support(e1), oldSup2 = minSupport.get_support(e2);
        int newSup1 = oldSup1 - dec, newSup2 = oldSup2 - dec;

        if (newSup1 > sup_min) {
          minSupport.modify({oldSup1, e1}, {newSup1, e1});
        } else {
          minSupport.modify({oldSup1, e1}, {sup_min, e1});
        }
        if (newSup2 > sup_min) {
          minSupport.modify({oldSup2, e2}, {newSup2, e2});
        } else {
          minSupport.modify({oldSup2, e2}, {sup_min, e2});
        }
      }
    }
  }
}

void build_index_vec(int &interval) {

  vector<subGraph> subGraphs;
  //初始化
  subGraphs.resize(interval);
  for (auto &e: sEdges) {
    e.weights.resize(interval);
  }
  deltaMaxK.resize(interval);
  deltaKEdges.resize(interval);

  //当前delta下所有子图中形成的所有静态三角形,（编号，时序三角形增量）
  unordered_map<int, int> allTri;
  //  unordered_set<int> allTri;

  //  Linux系统下使用signal函数设置超时信号处理函数
  //  signal(SIGALRM, timeout_handler);// 设置超时信号处理
  //  alarm(10800);                    // 设置超时为 3 小时

  time_t t1, t2;
  double time_taken1 = 0, time_taken2 = 0;
  for (int d = 0; d < interval; d++) {
//    cout << "delta=" << d << ": ";
    t1 = clock();
    compute_support(subGraphs, allTri, d);//这里的d是代表delta
    t2 = clock();
    time_taken1 += double(t2 - t1) / double(CLOCKS_PER_SEC);

//    cout << "delta=" << d << endl;
//    for (auto &e: sEdges) {
//      cout << "(" << e.source << ", " << e.target << ").sup = " << e.support << endl;
//    }
    deltaMaxK[d] = 0;//delta下最大trussness初始化为0
    t1 = clock();
    decompose(d);
    t2 = clock();
    time_taken2 += double(t2 - t1) / double(CLOCKS_PER_SEC);
    //    cout << deltaMaxK[d] << endl;
//    cout << " " << time_taken1 << " " << time_taken2 << endl;

    subGraphs.pop_back();//节省空间，释放掉最后一个子图
    for (auto &e: sEdges) {
      e.weights.pop_back();
    }
  }
  cout << time_taken1 << " " << time_taken2 << endl;
}
