#include <iostream>
#include <fstream>
#include <stdlib.h>

#include "UdfReader.hh"
#include "Command.hpp"
#include "FDiskData.h"

int main(int ac, char const * av[])
{
  std::ifstream is;

  if (ac < 2)
    {
      std::cerr << "Usage: ./UdfReader udf_file.iso" << std::endl;
      return EXIT_FAILURE;
    }
  is.open(av[1], std::ifstream::binary);
  if (AUdf::detect(is)) {

    std::cout << "is UDF" << std::endl;
    Command test(is);

    test.exec();
  } else
    std::cerr << av[1] << " is not an UDF File." << std::endl;
  is.close();
  return EXIT_SUCCESS;
}
