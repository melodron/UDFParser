#include <iostream>
#include <fstream>

#include "UdfReader.hh"
#include "Command.hpp"

int main(int ac, char const * av[])
{
  std::ifstream is;
  UdfReader *udf;

  if (ac < 2)
    return 0;

  is.open(av[1], std::ifstream::binary);

  if (AUdf::detect(is)) {
    udf = new UdfReader(is);
    std::cout << "is UDF"  << std::endl;
    delete udf;
  } else {
    std::cout << "is not UDF :(" << std::endl;
  }

  is.close();
/*  Command test;

  test.exec();*/
  return 0;
}
