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

#include <vtkActor.h>
#include <vtkCellData.h>
#include <vtkCellDataToPointData.h>
#include <vtkContourFilter.h>
#include <vtkDataArraySelection.h>
#include <vtkFieldData.h>
#include <vtkImageData.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkNew.h>
#include <vtkPNGWriter.h>
#include <vtkPointData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkResampleToImage.h>
#include <vtkWindowToImageFilter.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkXMLMultiBlockDataReader.h>
#include <vtkXMLPolyDataWriter.h>

#include <chrono>
#include <getopt.h>
#include <iostream>
#include <stdlib.h>

int main(int argc, char* argv[])
{
  auto t0 = std::chrono::high_resolution_clock::now();

  vtkNew<vtkXMLMultiBlockDataReader> mbr;
  mbr->SetFileName("/Users/qingzheng/DeepWaterImpact/origin/full/pv_insitu_09782.vtm");
  mbr->UpdateInformation();
  vtkDataArraySelection* sel = mbr->GetCellDataArraySelection();
  sel->DisableAllArrays();
  sel->EnableArray("v03");
  mbr->Update();

  auto t1 = std::chrono::high_resolution_clock::now();

  vtkNew<vtkResampleToImage> r2i;
  r2i->SetInputConnection(mbr->GetOutputPort());
  r2i->SetSamplingDimensions(150, 150, 150);
  r2i->Update();
  r2i->GetOutput()->GetFieldData()->AllocateArrays(0);
  if (0)
  {
    vtkNew<vtkXMLImageDataWriter> writer;
    writer->SetInputConnection(r2i->GetOutputPort());
    // writer->SetCompressorTypeToZLib();
    writer->EncodeAppendedDataOff();
    writer->SetFileName("temp.vti");
    writer->Update();
  }

  auto t2 = std::chrono::high_resolution_clock::now();

  vtkNew<vtkContourFilter> cf;
  cf->SetInputConnection(r2i->GetOutputPort());
  cf->SetInputArrayToProcess(
    0, 0, 0, vtkDataObject::FieldAssociations::FIELD_ASSOCIATION_POINTS, "v03");
  cf->SetValue(0, 0.5);
  cf->Update();
  cf->GetOutput()->GetCellData()->AllocateArrays(0);
  cf->GetOutput()->GetPointData()->AllocateArrays(0);
  if (0)
  {
    vtkNew<vtkXMLPolyDataWriter> writer;
    writer->SetInputConnection(cf->GetOutputPort());
    // writer->SetCompressorTypeToZLib();
    writer->EncodeAppendedDataOff();
    writer->SetFileName("temp.vtp");
    writer->Update();
  }

  auto t3 = std::chrono::high_resolution_clock::now();

  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(cf->GetOutputPort());
  mapper->ScalarVisibilityOff();

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  actor->GetProperty()->LightingOff();
  actor->GetProperty()->SetColor(1, 0.333, 0);

  vtkNew<vtkRenderer> renderer;
  renderer->SetBackground(0.321, 0.341, 0.431);
  renderer->AddActor(actor);

  vtkNew<vtkRenderWindow> window;
  window->AddRenderer(renderer);
  window->SetOffScreenRendering(true);
  window->SetSize(1024, 768);

  renderer->ResetCamera();

  vtkNew<vtkWindowToImageFilter> image;
  image->SetInput(window);
  image->SetInputBufferTypeToRGB();
  image->Update();

  vtkNew<vtkPNGWriter> png;
  png->SetFileName("screenshot.png");
  png->SetInputConnection(image->GetOutputPort());
  png->Write();

  auto t4 = std::chrono::high_resolution_clock::now();

  std::cout << "Time: " << std::chrono::duration<double>(t1 - t0).count() << " "
            << std::chrono::duration<double>(t2 - t1).count() << " "
            << std::chrono::duration<double>(t3 - t2).count() << " "
            << std::chrono::duration<double>(t4 - t3).count() << " " << std::endl;

  return 0;
}
