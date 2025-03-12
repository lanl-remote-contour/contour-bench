/*
 * Copyright (c) 2025 Triad National Security, LLC, as operator of Los Alamos
 * National Laboratory with the U.S. Department of Energy/National Nuclear
 * Security Administration. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * with the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of TRIAD, Los Alamos National Laboratory, LANL, the
 *    U.S. Government, nor the names of its contributors may be used to endorse
 *    or promote products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <vtkDataArraySelection.h>
#include <vtkNew.h>
#include <vtkResampleToImage.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkXMLMultiBlockDataReader.h>

#include <filesystem>
#include <getopt.h>
#include <iostream>
#include <stdlib.h>

int main(int argc, char* argv[])
{
  int size = 750;
  int c;
  while ((c = getopt(argc, argv, "s:")) != -1)
  {
    switch (c)
    {
      case 's':
        size = atoi(optarg);
        break;
      default:
        std::cerr << "Use -s to specify image size" << std::endl;
        exit(EXIT_FAILURE);
    }
  }
  argc -= optind;
  argv += optind;
  if (!argc)
  {
    std::cerr << "Lack target vtm filename" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::cout << "vtk file: " << argv[0] << std::endl;
  std::cout << "array: v02,v03,tev" << std::endl;
  std::cout << "result image size: " << size << "x" << size << "x" << size << std::endl;
  vtkNew<vtkXMLMultiBlockDataReader> mbr;
  mbr->SetFileName(argv[0]);
  mbr->UpdateInformation();
  vtkDataArraySelection* sel = mbr->GetCellDataArraySelection();
  sel->DisableAllArrays();
  sel->EnableArray("v02");
  sel->EnableArray("v03");
  sel->EnableArray("tev");

  vtkNew<vtkResampleToImage> r2i;
  r2i->SetInputConnection(mbr->GetOutputPort());
  r2i->SetSamplingDimensions(size, size, size);

  vtkNew<vtkXMLImageDataWriter> writer;
  writer->SetInputConnection(r2i->GetOutputPort());
  writer->SetCompressorTypeToNone();
  writer->EncodeAppendedDataOff();
  std::filesystem::path stem = std::filesystem::path(argv[0]).stem();
  char tmp[100];
  snprintf(tmp, sizeof(tmp), "%s.vti", stem.c_str());
  writer->SetFileName(tmp);
  writer->Update();

  return 0;
}
