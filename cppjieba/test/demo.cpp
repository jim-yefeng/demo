#include "cppjieba/Jieba.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cmath>
#include <list>
#include <chrono>
#include <unordered_map>
#include <thread>
#include <boost/program_options.hpp>

// const int ROOT_MIN_NUM = 2;
// const int ROOT_MAX_NUM = 20;
const int TH_NUM = 1;
const float COS_SIMLARITY = 0.8;

const char *const DICT_PATH = "../dict/jieba.dict.utf8";
const char *const HMM_PATH = "../dict/hmm_model.utf8";
const char *const USER_DICT_PATH = "../dict/user.dict.utf8";
const char *const IDF_PATH = "../dict/idf.utf8";
const char *const STOP_WORD_PATH = "../dict/stop_words.utf8";

// static inline int GetUtf8LetterNumber(const char *s)
// {
//   int i = 0, j = 0;
//   while (s[i])
//   {
//     if ((s[i] & 0xc0) != 0x80)
//       j++;
//     i++;
//   }
//   return j;
// }

template <typename T>
inline void printVec(const T &in, bool lf = true)
{
  if (in.size() < 0)
    return;
  int cnt = 0;
  std::cout << "[";
  for (auto item : in)
  {
    if (cnt > 0)
      std::cout << ",";

    std::cout << item;
    ++cnt;
  }
  std::cout << "]";
  if (lf)
    std::cout << std::endl;
}

template <typename T, typename U>
inline int countDupicate(T &i, U &v)
{
  // if (v.size() <= 0)
  //   return 0;

  int cnt = 1;
  for (auto k = i + 1; k < v.size(); ++k)
  {
    if (v[i] != v[k])
      break;

    ++i;
    cnt += 1;
  }

  return cnt;
}

int main(int argc, char **argv)
{
  namespace po = boost::program_options;
  po::options_description opts("Allowed options");
  po::variables_map vm;

  std::string input_file, result_file;
  int ths_num = 1; // thread number
  float cos_value = 0;
  int sleep_time = 0; // In seconds

  opts.add_options()
  ("input,i", po::value<std::string>(), "the file which needs to parse.")
  ("output,o", po::value<std::string>(), "the file which stores result.")
  ("ths-num,t", po::value<int>(&ths_num)->default_value(TH_NUM), "thread number")
  ("cos,c", po::value<float>(&cos_value)->default_value(COS_SIMLARITY), "the cosine similarity value")
  ("wait,w", po::value<int>(&sleep_time)->default_value(1), "The interval to show progress in seconds")
  ("verbose,V", "verbose level to print")
  ("help,h", "This is tool to calculate sentence similarity")
  ;

  try {
    po::store(po::parse_command_line(argc, argv, opts), vm);
  }
  catch(...) {
    std::cout << "Error: invalid option!!!" << std::endl;
    std::cout << opts << std::endl;
    return EXIT_FAILURE;
  }

  if (vm.count("help")) {
    std::cout << opts << std::endl;
    return EXIT_SUCCESS;
  }

  if (vm.count("input"))
  {
    input_file = vm["input"].as<std::string>();
  }

  if (vm.count("output"))
  {
    result_file = vm["output"].as<std::string>();
  }

  if (vm.count("ths-num"))
  {
    ths_num = vm["ths-num"].as<int>();
    if (ths_num <= 0)
      ths_num = 1; // minimum
  }

  if (vm.count("cos"))
  {
    cos_value = vm["cos"].as<float>();
  }

  if (vm.count("wait"))
  {
    sleep_time = vm["wait"].as<int>();
  }

  if (vm.count("verbose"))
  {
    std::cout << "input file:" << input_file << std::endl;
    std::cout << "result file:" << result_file << std::endl;
    std::cout << "thread number:" << ths_num << std::endl;
    std::cout << "cos similarity:" << cos_value << std::endl;
    std::cout << "wait internal:" << sleep_time << std::endl;
  }

  /////////////////////////////////////////////////////
  cppjieba::Jieba jieba(DICT_PATH,
                        HMM_PATH,
                        USER_DICT_PATH,
                        IDF_PATH,
                        STOP_WORD_PATH);
  std::vector<std::string> words;
  std::vector<uint32_t> wordsNum;
  std::vector<std::string> fileData;
  // std::vector<std::vector<std::string>> fileDataCut;
  std::vector<std::vector<uint32_t>> fileDataCutNum;
  std::vector<cppjieba::Word> jiebawords;
  std::string s;
  std::string result;
  std::unordered_map<std::string, int> rootMap;
  uint32_t rootNo = 0;

  // Read file contents
  std::ifstream fin;
  std::ofstream fout;
  std::stringstream buf;

  if (!input_file.empty())
  {
    fin.open(input_file, std::ios::in);
    buf << fin.rdbuf();
  }
  else
  {
    buf << std::cin.rdbuf();
  }
  
  if (!result_file.empty())
    fout.open(result_file, std::ios::out);
  
  int cnt = 0;
  for (std::string line; std::getline(buf, line);)
  {
    ++cnt;

    // Skip blank
    if (line.size() <= 0)
      continue;

    // // Skip shorter or longer
    // int charNum = 0;
    // charNum = GetUtf8LetterNumber(line.c_str());
    // if (charNum <= ROOT_MIN_NUM || charNum >= ROOT_MAX_NUM)
    //   continue;

    // std::cout << cnt << ":" << line << ":" << line.size() << ":" << charNum << std::endl;
    fileData.push_back(line);
    jieba.Cut(line, words, true);
    wordsNum.clear();
    for (auto &word : words)
    {
      auto iter = rootMap.find(word);
      if (iter != rootMap.end())
        wordsNum.push_back(iter->second);
      else
      {
        rootMap[word] = ++rootNo;
        wordsNum.push_back(rootNo);
      }
    }

    std::sort(wordsNum.begin(), wordsNum.end());
    fileDataCutNum.push_back(wordsNum);
    // std::cout << limonp::Join(words.begin(), words.end(), "/") << std::endl;
  }

  // std::cout << "Load data success!" << std::endl;
  std::cout << "Total cnt:" << cnt << std::endl;
  // std::cout << "Valid cnt:" << fileData.size() << std::endl;
  // std::cout << "Key cnt:" << rootMap.size() << std::endl;
  // std::cout << "Key max number:" << rootNo << std::endl;

  // int th_step = 0;
  // if (fileDataCutNum.size() < ths_num)
  // {
  //   ths_num = fileDataCutNum.size(); // Reduce thread
  //   th_step = 1;
  // }
  // else
  // {
  //   th_step = fileDataCutNum.size() / ths_num;
  //   if (fileDataCutNum.size() % ths_num)
  //     th_step += 1;
  // }

  std::vector<std::list<std::list<std::string>>> ths_similarity(ths_num);
  std::vector<std::thread> ths;
  std::vector<int> th_prog(ths_num);

  for (auto th_i = 0; th_i < ths_num; ++th_i)
  {
    ths.push_back(std::thread([&, th_i, ths_num] {
      // std::cout << "Start thread " << th_i << std::endl;
      auto &allSimilarSentence = ths_similarity[th_i];

      for (int i = th_i; i < fileDataCutNum.size(); i += ths_num)
      {
           
        th_prog[th_i]++;
        // std::cout << "th " << th_i << "No.:" << i << std::endl;

        // auto stime = std::chrono::high_resolution_clock::now();

        std::list<std::string> similarSentence;

        for (int j = i + 1; j < fileDataCutNum.size(); ++j)
        {
          auto &av = fileDataCutNum[i];
          auto &bv = fileDataCutNum[j];

          auto ia = 0;
          auto ib = 0;
          uint64_t AVecDot = 0;
          uint64_t BVecDot = 0;

          // Build A B root vector
          uint64_t sum_ABdot = 0;
          uint64_t sum_AAdot = 0;
          uint64_t sum_BBdot = 0;

          while (ia < av.size() && ib < bv.size())
          {
            if (av[ia] == bv[ib])
            {
              AVecDot = countDupicate(ia, av);
              BVecDot = countDupicate(ib, bv);
              ++ib;
              ++ia;
            }
            else if (av[ia] < bv[ib])
            {
              AVecDot = countDupicate(ia, av);
              BVecDot = 0;
              ++ia;
            }
            else
            {
              AVecDot = 0;
              BVecDot = countDupicate(ib, bv);
              ++ib;
            }

            sum_ABdot += AVecDot * BVecDot;
            sum_AAdot += AVecDot * AVecDot;
            sum_BBdot += BVecDot * BVecDot;
          }

          // Skip if AB dot is 0
          if (sum_ABdot == 0)
            continue;

          while (ia < av.size())
          {
            AVecDot = countDupicate(ia, av);
            BVecDot = 0;

            sum_ABdot += AVecDot * BVecDot;
            sum_AAdot += AVecDot * AVecDot;
            sum_BBdot += BVecDot * BVecDot;
            ++ia;
          }

          while (ib < bv.size())
          {
            BVecDot = countDupicate(ib, bv);
            AVecDot = 0;

            sum_ABdot += AVecDot * BVecDot;
            sum_AAdot += AVecDot * AVecDot;
            sum_BBdot += BVecDot * BVecDot;
            ++ib;
          }

          double cosineSimilarity = (double)sum_ABdot / (std::sqrt(sum_AAdot) * std::sqrt(sum_BBdot));
          if (cosineSimilarity >= cos_value)
          {
            similarSentence.push_back(fileData[j]);
          }
        }

        if (similarSentence.size() > 0)
        {
          similarSentence.push_front(fileData[i]);
          allSimilarSentence.push_back(std::move(similarSentence));
        }

        // auto etime = std::chrono::high_resolution_clock::now();
        // std::chrono::duration<double> diff = etime - stime;
        // std::cout << "elapse:" << std::fixed << std::setprecision(9) << diff.count() << "s" << std::endl;
      }

      // for (auto item : allSimilarSentence)
      // {
      //   std::cout << "(" << item.size() << ",";
      //   printVec(item, false);
      //   std::cout << ")" << std::endl;
      // }
    }));

    // ths.push_back(std::move(th));
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  while (1)
  {
    int total = 0;
    for (int i = 0; i < th_prog.size(); ++i)
    {
      std::cout << "thread[" << i << "]=" << th_prog[i] << " ";
      total += th_prog[i];
    }

    std::cout << "The rest ..." << fileDataCutNum.size() - total << std::endl;

    if (total >= fileDataCutNum.size())
      break;
    
    if (sleep_time > 0)
      std::this_thread::sleep_for(std::chrono::seconds(sleep_time));
  } 

  for (auto i = 0; i < ths.size(); ++i)
  {
    std::ostringstream oss;
    ths[i].join();
    for (auto &item : ths_similarity[i])
    {
      oss << "" << item.size() << ",";
      for (auto iter = item.begin(); iter != item.end();)
      {
        oss << *iter;
        if (++iter != item.end())
          oss << ",";
      }
      oss << "" << std::endl;
      // std::cout << "(" << item.size() << ",";
      // printVec(item, false);
      // std::cout << ")" << std::endl;
    }
    if (fout.is_open())
      fout << oss.str();
    else
      std::cout << oss.str();

    // std::cout << "Join thread " << i << std::endl;
  }

  return EXIT_SUCCESS;
}
