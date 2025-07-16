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

#include <vtkContourFilter.h>
#include <vtkDataObject.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLPolyDataWriter.h>

#include <fstream>
#include <stdlib.h>

int Run(
  const char* inputFile, const char* outputFile1, const char* outputFile2, const char* outputFile3)
{
  vtkNew<vtkXMLImageDataReader> reader;
  reader->SetFileName(inputFile);
  reader->Update();

  vtkNew<vtkContourFilter> cf;
  cf->SetInputConnection(reader->GetOutputPort());
  cf->ComputeScalarsOff();
  cf->ComputeNormalsOff();
  cf->SetInputArrayToProcess(
    0, 0, 0, vtkDataObject::FieldAssociations::FIELD_ASSOCIATION_POINTS, "baryon_density");
  cf->SetValue(0, 81.66);
  cf->Update();

  vtkNew<vtkXMLPolyDataWriter> wr;
  wr->SetCompressorTypeToNone();
  wr->EncodeAppendedDataOff();
  wr->SetInputConnection(cf->GetOutputPort());
  wr->SetFileName(outputFile1);
  wr->Write();

  return 0;
}

/*
 * Usage: argc=5, argv1=command_file, argv2=result_file1, argv3=result_file2,
 *   argv4=result_file3
 */
int main(int argc, char* argv[])
{
  if (argc < 5)
  {
    exit(EXIT_FAILURE);
  }
  std::ifstream input(argv[1] /* command file */);
  std::string fileName;
  input >> fileName;
  if (!input.good())
  {
    exit(EXIT_FAILURE);
  }
  return Run(fileName.c_str(), argv[2], argv[3], argv[4] /* result files */);
}
