//
// Created by Yang on 2024/11/14.
//

#include "Graph_IO.h"
#include "index_build.h"
#include "index_search.h"
#include "random"
#include <iomanip>
#include <regex>

using namespace std;

//由用户输入查询参数，验证正确性
void input_MDTs() {
  int q = 332338, delta = 2;
  // 用户输入q和δ
  //  try {
  //    cin >> q;
  //    if (vertices.find(q) == vertices.end()) {
  //      cerr << "Invalid query node: " << q << " is not in the graph!" <<
  //      endl; return;
  //    }
  //  } catch (const invalid_argument &e) {
  //    cerr << "Invalid query node: " << q << " is not an integer!" << endl;
  //    return;
  //  }
  //  try {
  //    cin >> delta;
  //    if (delta < 0) {
  //      cerr << "Invalid delta: " << delta << " is a negative number!" <<
  //      endl; return;
  //    }
  //  } catch (const invalid_argument &e) {
  //    cerr << "Invalid delta: " << delta << " is not an integer!" << endl;
  //    return;
  //  }
  int maxk = 270;
  q = vertices[q];
  vector<int> ansi = index_search(q, delta, 0, maxk);

  cout << "the max trussness is: " << maxk << endl;
  //对应原始图中的顶点编号
  int num = (int)vertices.size();
  vector<int> nodes;
  nodes.resize(num);
  for (auto &v : vertices) {
    nodes[v.second] = v.first;
  }

  ofstream fout("answer.csv");
  fout << "source,target,timestamps" << endl;
  for (auto &eid : ansi) {
    fout << nodes[sEdges[eid].source] << "," << nodes[sEdges[eid].target]
         << ",\"" << sEdges[eid].timestamps[0];
//    cout << nodes[sEdges[eid].source] << "," << nodes[sEdges[eid].target]
//         << ",\"" << sEdges[eid].timestamps[0];
    for (int t = 1; t < sEdges[eid].timestamps.size(); t++) {
      fout << "," << sEdges[eid].timestamps[t];
//      cout << "," << sEdges[eid].timestamps[t];
    }
    fout << "\"" << endl;
//    cout << "\"" << endl;
  }
  fout.close();
}

// q-MDT，随机生成多个q和δ，统计时间，测试性能
double random_qMDT(const string &filename, const int &vNum,
                   const int &interval) {
  // 创建随机数生成器
  std::random_device rd;  // 获取随机种子
  std::mt19937 gen(rd()); // 使用 Mersenne Twister 算法生成随机数
  std::uniform_int_distribution<> qs(0, vNum - 1); // 定义查询顶点生成范围
  std::uniform_int_distribution<> deltas(0, interval); // 定义delta生成范围

  int q, delta;
  int expCount = 100;
  time_t s_t, e_t;
  double time_taken = 0;
  string file = "./ans/" + filename + "_qMDT";

  // 随机生成q和δ
  for (int i = 1; i <= expCount; ++i) {
    // 生成一个随机数
    q = qs(gen);
    delta = deltas(gen);
    int maxk = 0;
    s_t = clock();
    vector<int> ansi = index_search(q, delta, i, maxk);
    e_t = clock();
    time_taken += (double(e_t - s_t) / double(CLOCKS_PER_SEC)) * 1000;

    print_ans_to_file(ansi, file, q, delta, maxk);
  }
  return time_taken;
}

// MDT，每个δ下运行多次，查找最大k值的truss，统计时间，测试性能
double random_MDT(const string &filename, const int &interval) {

  time_t s_t, e_t;
  double time_taken = 0;
  string file = "./ans/" + filename + "_MDT";

  for (int d = 0; d < interval; ++d) {
    s_t = clock();
    //获取在当前delta下全图上最大的trussness
    int k = deltaMaxK[d];
    vector<int> ansi = index_search(d, k);
    for (int i = 0; i < 100; ++i) {
      ansi = index_search(d, k);
    }
    e_t = clock();
    time_taken += (double(e_t - s_t) / double(CLOCKS_PER_SEC)) * 1000;
    print_ans_to_file(ansi, file, d);
  }
  return time_taken;
}

//清空所有容器，处理下一个文件
void clear_all() {
  // 释放嵌套容器内容
  for (auto &subvec : singleTime) {
    vector<int>().swap(subvec);
  }
  for (auto &subvec : adjEdges) {
    vector<int>().swap(subvec);
  }
  for (auto &umap : deltaKEdges) {
    for (auto &pair : umap) {
      vector<int>().swap(pair.second);
    }
    umap.clear();
  }
  for (auto &e : sEdges) {
    vector<int>().swap(e.deltaTruss);
    vector<int>().swap(e.timestamps);
    vector<int>().swap(e.triangles);
  }

  // 清空主容器
  map<int, int>().swap(vertices);
  vector<vector<int>>().swap(singleTime);
  vector<int>().swap(deltaMaxK);
  vector<unordered_map<int, vector<int>>>().swap(deltaKEdges);
  vector<int>().swap(offset);
  vector<int>().swap(targets);
  vector<vector<int>>().swap(adjEdges);
  vector<sEdge>().swap(sEdges);
  vector<sTriangle>().swap(sTriangles);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cerr << "The number of parameters is error!" << endl;
    cout << "Usage: ./TTS file_path" << endl;
    return 1;
  }

  // read graph
  string filePath = argv[1];
  std::vector<std::string> files = {//      "example"
                                    //      "primary",
                                    //      "thiers",
                                    //      "email",
                                    //      "lyon",
                                    //      "mathoverflow",
                                    //      "facebook",
                                    //      "lkml",
                                    //      "enron",
                                    //      "twitter",
                                    "dblp"};
  for (auto &filename : files) {
    string file = filePath + filename + ".txt";
    int vNum = 0, sEdgeNum = 0, tEdgeNum = 0, interval = 0;
    if (!readFile(file, vNum, sEdgeNum, tEdgeNum, interval)) {
      cerr << "The dataset " << filename << " is not exist!" << endl;
      return 1;
    }
    int tri_num = findAllTriangles(vNum);

    //  regex txt_regex("\\.txt"); //匹配以“.txt”结尾的字符串 filename =
    //  regex_replace(filename, txt_regex, "");//删除".txt"

    cout << filename << " " << vNum << " " << tEdgeNum << " " << sEdgeNum << " "
         << tri_num << endl;
    cout << "The number of timestamps is " << interval << ", its range is ["
         << tMin << ", " << tMax << "]." << endl;

    string index = "./index/" + filename + "_edge.txt";
    string supplement_index = "./index/" + filename + "_supplement.txt";

    time_t s_t, e_t;
    double time_taken = 0;
    ifstream indexFile(index);
    //记录运行时间
    ofstream outfile("./runtime/" + filename, ios::app);

    if (indexFile) {
      cout << "The index has been built!" << endl;
      deltaMaxK.resize(interval);
      deltaKEdges.resize(interval);
      int numIndex = recover_index(index);
      recover_supplement(supplement_index, interval);
      //计算索引的压缩比
      double r = 1.0 * numIndex / (sEdgeNum * interval);
      cout << "The compression ratio is: " << fixed << setprecision(5) << r
           << endl;
    } else {
      cout << "It is building index!" << endl;
      s_t = clock();
      build_index_vec(interval);
      e_t = clock();
      //    print_to_file(index);
      time_taken += double(e_t - s_t) / double(CLOCKS_PER_SEC);
      cout << "TT-index building time: " << fixed << setprecision(5)
           << time_taken << " sec" << endl;
      int numIndex = print_index_to_file(index, supplement_index);
      //计算索引的压缩比
      double r = 1.0 * numIndex / (sEdgeNum * interval);
      cout << "The compression ratio is: " << fixed << setprecision(5) << r
           << endl;
      outfile << "***TTS***" << endl;
      outfile << "Index construction time: " << fixed << time_taken
              << setprecision(5) << " sec " << endl;
      outfile << "The compression ratio is: " << fixed << setprecision(5) << r
              << endl;
    }

    for (auto &e : sEdges) {
      e.support = -1; //用support标识该边是否访问过
    }

    input_MDTs();

    //    time_taken = random_MDT(filename, interval);
    //    cout << "TTS takes time to answer random_MDT: " << fixed <<
    //    setprecision(5) << time_taken << "ms" << endl; outfile << "TTS takes
    //    time to answer random_MDT: " << fixed << setprecision(5) << time_taken
    //    << "ms" << endl;
    //
    //    time_taken = random_qMDT(filename, vNum, interval);
    //    cout << "TTS takes time to answer random_qMDT: " << fixed <<
    //    setprecision(5) << time_taken << "ms" << endl; outfile << "TTS takes
    //    time to answer random_qMDT: " << fixed << setprecision(5) <<
    //    time_taken << "ms" << endl;
    //
    //    outfile << endl;
    //    outfile.close();

    clear_all();
  }

  return 0;
}