/*
 * Parses a classfile and serializes it into another file named "dupe.class"
 */

#include <ClassFile/ClassFile.hpp>
#include <ClassFile/Parser.hpp>
#include <ClassFile/Serializer.hpp>

#include <iostream>
#include <fstream>
#include <chrono>

int main(int argc, char** argv)
{
  if(argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " <classfile>\n";
    return -1;
  }

  std::ifstream infile{argv[1], std::ios::ate};

  if(!infile.good())
  {
    std::cout << "Unable to open file \"" << argv[1] << "\"\n";
    return -2;
  }

  size_t fileSize = infile.tellg();
  infile.seekg(0);

  auto before = std::chrono::high_resolution_clock::now();
  auto errOrClass = ClassFile::Parser::ParseClassFile(infile);
  auto after = std::chrono::high_resolution_clock::now();

  if(errOrClass.IsError())
  {
    std::cout << "PARSING ERROR: " << errOrClass.GetError().What << '\n';
    return -3;
  }

  std::cout << "Parsed " << infile.tellg() << '/' << fileSize << " bytes ";
  std::cout << "in ~" << std::chrono::duration_cast<std::chrono::nanoseconds>(after-before).count() / 1000000.0f << " milliseconds\n";

  ClassFile::ClassFile cf = errOrClass.Release();

  std::ofstream outfile{"dupe.class"};

  if(!outfile.good())
  {
    std::cout << "Unable to create output file\n";
    return -4;
  }

  before = std::chrono::high_resolution_clock::now();
  auto err = ClassFile::Serializer::SerializeClassFile(outfile, cf);
  after = std::chrono::high_resolution_clock::now();

  if(err.IsError())
  {
    std::cout << "SERIALIZATION ERROR: " << err.GetError().What << '\n';
    return -5;
  }

  std::cout << "Serialized class in ~" << std::chrono::duration_cast<std::chrono::nanoseconds>(after-before).count() / 1000000.0f << " milliseconds\n";

}
