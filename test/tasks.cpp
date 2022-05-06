#include <iostream>
#include <functional>
#include "realtime_utilities/realtime_ipc.h"
#include "realtime_utilities/parallel_computing.h"

int main(int argc, char* argv[])
{

  auto function = [](int a) -> int
  {
    std::cout << "inside: " << a * 10 << std::endl;
    return a*10;
  };

  realtime_utilities::tasks tasks;

  std::vector< std::future<int> > res;
  for(size_t i=0; i<11; i++)
  {
    auto f = std::bind(function, i);
    res.push_back( tasks.queue(f) );
  }
  
  tasks.start(5);
  tasks.finish();

  for(auto & ret : res)
  {
    std::cout << ret.get() << std::endl;
  }


  return 0;
}