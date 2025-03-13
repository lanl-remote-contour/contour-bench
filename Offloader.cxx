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

#include <vtkCellData.h>
#include <vtkContourFilter.h>
#include <vtkDataArraySelection.h>
#include <vtkFieldData.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkResampleToImage.h>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLPolyDataWriter.h>

#include <fstream>
#include <stdlib.h>

int Run(const char* fileName, const char* arrayName, double contourValue, const char* outputFile)
{
  vtkNew<vtkXMLImageDataReader> reader;
  reader->SetFileName(fileName);
  reader->UpdateInformation();
  vtkDataArraySelection* das = reader->GetPointDataArraySelection();
  das->DisableAllArrays();
  das->EnableArray(arrayName);
  reader->Update();
  reader->GetOutput()
    ->GetFieldData() // Remove all field data
    ->AllocateArrays(0);

  vtkNew<vtkResampleToImage> r2i;
  r2i->SetInputConnection(reader->GetOutputPort());
  r2i->SetSamplingDimensions(150, 150, 150);
  r2i->Update();
  r2i->GetOutput()
    ->GetCellData() // Remove all cell data
    ->AllocateArrays(0);
  r2i->GetOutput()
    ->GetPointData() // Remove vtkGhostType and vtkValidPointMask arrays
    ->RemoveArray("vtkValidPointMask");
  r2i->GetOutput()->GetPointData()->RemoveArray("vtkGhostType");

  vtkNew<vtkContourFilter> cf;
  cf->SetInputConnection(r2i->GetOutputPort());
  cf->ComputeScalarsOff(); // No scalars or normals please
  cf->ComputeNormalsOff();
  cf->SetInputArrayToProcess(
    0, 0, 0, vtkDataObject::FieldAssociations::FIELD_ASSOCIATION_POINTS, arrayName);
  cf->SetValue(0, contourValue);
  cf->Update();

  vtkNew<vtkXMLPolyDataWriter> writer;
  writer->SetInputConnection(cf->GetOutputPort());
  writer->SetCompressorTypeToNone(); // Turn off compression
  writer->EncodeAppendedDataOff();
  writer->SetFileName(outputFile);
  writer->Update();

  return 0;
}

int main(int argc, char* argv[])
{
  if (argc < 3)
  {
    exit(EXIT_FAILURE);
  }
  std::ifstream input(argv[1] /* command file */);
  std::string fileName;
  std::string arrayName;
  double contourValue;
  input >> fileName >> arrayName >> contourValue;
  if (!input.good())
  {
    exit(EXIT_FAILURE);
  }
  return Run(fileName.c_str(), arrayName.c_str(), contourValue, argv[2] /* result file */);
}
