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
#include <vtkDataArraySelection.h>
#include <vtkDataObject.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLUnstructuredGridReader.h>

#include <fstream>
#include <stdlib.h>

void SetCompression(vtkXMLWriter* writer, int compression)
{
  if (compression == 1)
  {
    writer->SetCompressorTypeToZLib();
  }
  else if (compression == 2)
  {
    writer->SetCompressorTypeToLZ4();
  }
  else
  {
    writer->SetCompressorTypeToNone();
  }
}

void EnableArrays(vtkDataArraySelection* selection, bool v02, bool v03, bool tev)
{
  if (v02)
  {
    selection->EnableArray("v02");
  }
  if (v03)
  {
    selection->EnableArray("v03");
  }
  if (tev)
  {
    selection->EnableArray("tev");
  }
}

int Run(const char* inputFile, const char* outputFile1, const char* outputFile2,
  const char* outputFile3, bool v02, bool v03, bool tev, int compression)
{
  vtkNew<vtkXMLUnstructuredGridReader> reader;
  reader->SetFileName(inputFile);
  reader->UpdateInformation();
  reader->GetPointDataArraySelection()->DisableAllArrays();
  EnableArrays(reader->GetPointDataArraySelection(), v02, v03, tev);
  reader->Update();

  vtkDataSet* const inputData = reader->GetOutput();
  vtkNew<vtkPointData> inputPointData;
  inputPointData->ShallowCopy(inputData->GetPointData());

  if (v02)
  {
    vtkNew<vtkPointData> pd;
    pd->AddArray(inputPointData->GetAbstractArray("v02"));
    inputData->GetPointData()->ShallowCopy(pd);
    vtkNew<vtkContourFilter> cf1;
    cf1->SetInputData(inputData);
    cf1->ComputeScalarsOff();
    cf1->ComputeNormalsOff();
    cf1->SetInputArrayToProcess(
      0, 0, 0, vtkDataObject::FieldAssociations::FIELD_ASSOCIATION_POINTS, "v02");
    cf1->SetValue(0, 0.8);
    cf1->Update();

    vtkNew<vtkXMLPolyDataWriter> w1;
    w1->SetFileName(outputFile1);
    w1->SetInputConnection(cf1->GetOutputPort());
    SetCompression(w1, compression);
    w1->EncodeAppendedDataOff();
    w1->Write();
  }

  if (v03)
  {
    vtkNew<vtkPointData> pd;
    pd->AddArray(inputPointData->GetAbstractArray("v03"));
    inputData->GetPointData()->ShallowCopy(pd);
    vtkNew<vtkContourFilter> cf2;
    cf2->SetInputData(inputData);
    cf2->ComputeScalarsOff();
    cf2->ComputeNormalsOff();
    cf2->SetInputArrayToProcess(
      0, 0, 0, vtkDataObject::FieldAssociations::FIELD_ASSOCIATION_POINTS, "v03");
    cf2->SetValue(0, 0.5);
    cf2->Update();

    vtkNew<vtkXMLPolyDataWriter> w2;
    w2->SetFileName(outputFile2);
    w2->SetInputConnection(cf2->GetOutputPort());
    SetCompression(w2, compression);
    w2->EncodeAppendedDataOff();
    w2->Write();
  }

  if (tev)
  {
    vtkNew<vtkPointData> pd;
    pd->AddArray(inputPointData->GetAbstractArray("tev"));
    inputData->GetPointData()->ShallowCopy(pd);
    vtkNew<vtkContourFilter> cf3;
    cf3->SetInputData(inputData);
    cf3->ComputeScalarsOff();
    cf3->ComputeNormalsOff();
    cf3->SetInputArrayToProcess(
      0, 0, 0, vtkDataObject::FieldAssociations::FIELD_ASSOCIATION_POINTS, "tev");
    cf3->SetValue(0, 0.1);
    cf3->Update();

    vtkNew<vtkXMLPolyDataWriter> w3;
    w3->SetFileName(outputFile3);
    w3->SetInputConnection(cf3->GetOutputPort());
    SetCompression(w3, compression);
    w3->EncodeAppendedDataOff();
    w3->Write();
  }

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
  bool v02, v03, tev;
  int compression;
  input >> fileName >> v02 >> v03 >> tev >> compression;
  if (!input.good())
  {
    exit(EXIT_FAILURE);
  }
  return Run(
    fileName.c_str(), argv[2], argv[3], argv[4] /* result files */, v02, v03, tev, compression);
}
