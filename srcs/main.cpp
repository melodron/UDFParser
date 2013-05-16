#include <iostream>
#include <fstream>
#include <stdlib.h>

#include "UdfReader.hh"
#include "Command.hpp"
#include "FDiskData.h"

int main(int ac, char const * av[])
{
  std::ifstream is;
  UdfReader *udf;

  if (ac < 2)
    {
      std::cerr << "Usage: ./UdfReader udf_file.iso" << std::endl;
      return EXIT_FAILURE;
    }
  is.open(av[1], std::ifstream::binary);
  if (AUdf::detect(is)) {
    udf = new UdfReader(is);

    std::cout << "is UDF" << std::endl;
    // udf->getFDiskData(data);
    // udf->chdir("/dir2/");
    // udf->readFile("README2");
    // udf->copy("README2", "/tmp/toto");
    Command test(is);

    test.exec();
    delete udf;
  } else
    std::cerr << av[1] << " is not an UDF File." << std::endl;
  is.close();
  return EXIT_SUCCESS;
}
