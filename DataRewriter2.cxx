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
#include <vtkDataArraySelection.h>
#include <vtkNew.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkXMLUnstructuredGridWriter.h>

#include <filesystem>
#include <iostream>
#include <string>

int Process(vtkAppendDataSets* append, const char* path, int i)
{
  vtkNew<vtkXMLUnstructuredGridReader> reader;
  reader->SetFileName(path);
  reader->UpdateInformation();
  vtkDataArraySelection* sel = reader->GetCellDataArraySelection();
  sel->DisableAllArrays();
  sel->EnableArray("v02");
  sel->EnableArray("v03");
  sel->EnableArray("tev");
  reader->Update();

  if (i == 0)
  {
    append->SetInputData(reader->GetOutput());
  }
  else
  {
    append->AddInputData(reader->GetOutput());
  }
}

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " pv_insitu_N" << std::endl;
    return 1;
  }
  std::string dir = argv[1];
  std::cout << "Converting " << dir << "..." << std::endl;
  std::filesystem::path filename = std::filesystem::path(dir.c_str()).filename();
  vtkNew<vtkAppendDataSets> append;
  char tmp[100];
  for (int i = 0; i < 512; i++)
  {
    snprintf(tmp, sizeof(tmp), "/%s_0_%d.vtu", filename.c_str(), i);
    std::string path = dir + tmp;
    std::cout << "Processing " << tmp + 1 << " ..." << std::endl;
    Process(append.Get(), path.c_str(), i);
  }
  append->Update();
  vtkNew<vtkXMLUnstructuredGridWriter> writer;
  writer->SetFileName(filename.c_str());
  writer->SetInputData(append->GetUnstructuredGridOutput());
  writer->SetCompressorTypeToZLib();
  writer->EncodeAppendedDataOff();
  std::cout << "Done!" << std::endl;
  return 0;
}
