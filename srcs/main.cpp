#include <iostream>
#include <fstream>

#include "UdfReader.hh"
#include "Command.hpp"
#include "FDiskData.h"

int main(int ac, char const * av[])
{
  std::ifstream is;
  UdfReader *udf;

  if (ac < 2)
    return 0;

  is.open(av[1], std::ifstream::binary);

  if (AUdf::detect(is)) {
    FDiskData data;

    udf = new UdfReader(is);
    std::cout << "is UDF" << std::endl;
    //udf->getFDiskData(data);
    udf->chdir("/dir2/");
    udf->readFile("README2");
    udf->copy("README2", "/tmp/toto");
    delete udf;
  } else {
    std::cout << "is not UDF :(" << std::endl;
  }

  //is.close();
/*  Command test;

  test.exec();*/
  return 0;
}
