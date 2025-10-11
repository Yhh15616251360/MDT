#pragma once
#include "define.h"
#include "print.h"

using namespace std;

//获取边eid在当前delta下的trussness值
int get_trussness(const int &eid, const int &delta) {
  //索引中最后一个delta的位置
  int pos = (int)sEdges[eid].deltaTruss.size() - 2;
  //假设没有合并时delta的位置
  if (pos > delta * 2)
    pos = delta * 2;
  while (pos > 0) {
    if (sEdges[eid].deltaTruss[pos] <= delta) {
      break;
    }
    pos -= 2;
  }
  return sEdges[eid].deltaTruss[pos + 1];
}

// q-MDT下的搜索算法
vector<int> index_search(int &q, const int &delta, const int &vis, int &maxk) {
  vector<int> ansEdges;
  vector<int> candidates;

  /*
  //获取包含q的边id及其在当前delta下的trussness
  maxk = 0;
  for (auto &e : adjEdges[q]) {
    //当前边不在任何truss里，跳过
    if (sEdges[e].deltaTruss.empty()) continue;
    int k = get_trussness(e, delta);
    //    idTruss.emplace_back(e, k);
    //    k = ((int) sEdges[e].timestamps.size() / (delta + 1)) * k;
    if (maxk < k) {
      maxk = k;
      candidates.clear();
      candidates.push_back(e);
    } else if (maxk == k) {
      candidates.push_back(e);
    }
  }
   */

  //  优化版本
  vector<pair<int, int>> idTruss;
  set<int> ks; //记录该顶点存储的所有k值
  for (auto &e : adjEdges[q]) {
    //当前边不在任何truss里，跳过
    if (sEdges[e].deltaTruss.empty())
      continue;
    int k = get_trussness(e, delta);
    idTruss.emplace_back(e, k);
    ks.insert(k);
  }

  //   使用 std::sort 对 vector 进行排序，按 pair 的第二个元素排序
  std::sort(idTruss.begin(), idTruss.end(),
            [](const std::pair<int, int> &a, const std::pair<int, int> &b) {
              return a.second > b.second; // 按照第二个字段升序排序
            });
  //  // 输出排序后的结果
  //  for (const auto &p : idTruss) {
  //    std::cout << "(" << sEdges[p.first].source << ", " <<
  //    sEdges[p.first].target
  //              << ") " << p.second << endl;
  //  }

  if (maxk == 0)
    maxk = idTruss[0].first;
  else {
    cout << "all saved k: ";
    for (auto &k : ks) {
      cout << k << " ";
    }
    cout << endl;
  }
  for (auto &p : idTruss) {
    if (p.second < maxk)
      break;
    candidates.push_back(p.first);
  }

  while (!candidates.empty()) {
    int eid = candidates.back();
    sEdge &e = sEdges[eid];
    candidates.pop_back();
    if (e.support == vis)
      continue;
    ansEdges.push_back(eid);
    e.support = vis;

    for (int i = 0; i < e.triangles.size(); i += 3) {
      int tri = e.triangles[i];
      if (sTriangles[tri].minDelta > delta)
        continue;
      int e1 = e.triangles[i + 1], e2 = e.triangles[i + 2];
      int k1 = get_trussness(e1, delta), k2 = get_trussness(e2, delta);
      if (k1 >= maxk && sEdges[e1].support != vis) {
        candidates.push_back(e1);
      }
      if (k2 >= maxk && sEdges[e2].support != vis) {
        candidates.push_back(e2);
      }
    }
  }

  return ansEdges;
}

// MDT下的搜索算法
vector<int> index_search(const int &delta, const int &maxk) {
  vector<int> candidates;

  /*
  for (int e = 0; e < sEdges.size(); e++) {
    //当前边不在任何truss里，跳过
    if (sEdges[e].deltaTruss.empty())
      continue;
    int k = get_trussness(e, delta);
    if (k == maxk) {
      candidates.push_back(e);
    }
  }
   */

  //  优化版本
  if (maxk > 2) {
    for (auto eid : deltaKEdges[delta][maxk])
      candidates.push_back(eid);
  }

  return candidates;
}
