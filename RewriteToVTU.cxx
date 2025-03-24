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

#include <vtkAppendDataSets.h>
#include <vtkCellDataToPointData.h>
#include <vtkDataArraySelection.h>
#include <vtkNew.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkXMLUnstructuredGridWriter.h>

#include <filesystem>
#include <getopt.h>
#include <iostream>
#include <stdlib.h>
#include <string>

void Append(vtkAppendDataSets* dst, const std::string& path, int i)
{
  vtkNew<vtkXMLUnstructuredGridReader> reader;
  reader->SetFileName(path.c_str());
  reader->UpdateInformation();
  vtkDataArraySelection* sel = reader->GetCellDataArraySelection();
  sel->DisableAllArrays();
  sel->EnableArray("v02");
  sel->EnableArray("v03");
  sel->EnableArray("tev");
  reader->Update();

  if (i == 0)
  {
    dst->SetInputData(reader->GetOutput());
  }
  else
  {
    dst->AddInputData(reader->GetOutput());
  }
}

int main(int argc, char* argv[])
{
  int gzip = 1;
  int c;
  while ((c = getopt(argc, argv, "z:h")) != -1)
  {
    switch (c)
    {
      case 'z':
        gzip = atoi(optarg);
        break;
      case 'h':
      default:
        std::cerr << "Use -z=0/1 to disable/enable gzip compression" << std::endl;
        exit(EXIT_FAILURE);
    }
  }
  argc -= optind;
  argv += optind;
  if (!argc)
  {
    std::cerr << "Lack input dir pv_insitu_N" << std::endl;
    return 1;
  }
  std::string dir = argv[0];
  std::cout << "Converting " << dir << " (gz=" << gzip << ")..." << std::endl;
  std::filesystem::path filename = std::filesystem::path(dir.c_str()).filename();
  vtkNew<vtkAppendDataSets> dst;
  char tmp[100];
  for (int i = 0; i < 512; i++)
  {
    snprintf(tmp, sizeof(tmp), "/%s_0_%d.vtu", filename.c_str(), i);
    std::string path = dir + tmp;
    std::cout << "Processing " << tmp + 1 << " ..." << std::endl;
    Append(dst.Get(), path, i);
  }
  dst->Update();
  vtkUnstructuredGrid* const grid = dst->GetUnstructuredGridOutput();
  std::cout << "Num of points: " << grid->GetNumberOfPoints() << std::endl;
  std::cout << "Num of cells: " << grid->GetNumberOfCells() << std::endl;
  vtkNew<vtkCellDataToPointData> c2p;
  c2p->SetInputData(grid);
  vtkNew<vtkXMLUnstructuredGridWriter> writer;
  writer->SetInputConnection(c2p->GetOutputPort());
  snprintf(tmp, sizeof(tmp), "%s.vtu", filename.c_str());
  writer->SetFileName(tmp);
  if (gzip)
  {
    writer->SetCompressorTypeToZLib();
    writer->SetHeaderTypeToUInt32(); // int32 is okay
  }
  else
  {
    writer->SetCompressorTypeToNone();
    writer->SetHeaderTypeToUInt64();
  }
  writer->EncodeAppendedDataOff();
  writer->Update();
  std::cout << "Done!" << std::endl;
  return 0;
}
