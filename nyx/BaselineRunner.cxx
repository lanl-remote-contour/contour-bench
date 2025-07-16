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
#include <vtkContourFilter.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkOutlineFilter.h>
#include <vtkPNGWriter.h>
#include <vtkPointData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkWindowToImageFilter.h>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLPolyDataWriter.h>

#include <chrono>
#include <filesystem>
#include <getopt.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <string>

void Run0(vtkAlgorithm* input, const char* outputPng)
{
  auto t0 = std::chrono::high_resolution_clock::now();
  // baryon_density
  vtkNew<vtkContourFilter> cf;
  cf->SetInputConnection(input->GetOutputPort());
  cf->ComputeScalarsOff();
  cf->ComputeNormalsOff();
  cf->SetInputArrayToProcess(
    0, 0, 0, vtkDataObject::FieldAssociations::FIELD_ASSOCIATION_POINTS, "baryon_density");
  cf->SetValue(0, 81.66);
  cf->Update();

  auto t1 = std::chrono::high_resolution_clock::now();

  vtkNew<vtkRenderer> renderer;
  renderer->SetBackground(0.321, 0.341, 0.431);

  vtkNew<vtkOutlineFilter> of;
  of->SetInputConnection(input->GetOutputPort());

  vtkNew<vtkPolyDataMapper> mp0;
  mp0->SetInputConnection(of->GetOutputPort());
  mp0->ScalarVisibilityOff();

  vtkNew<vtkActor> ac0;
  ac0->SetMapper(mp0);
  ac0->GetProperty()->LightingOff();
  ac0->GetProperty()->SetColor(1, 1, 1);
  renderer->AddActor(ac0);

  // baryon_density
  vtkNew<vtkPolyDataMapper> mp1;
  mp1->SetInputConnection(cf->GetOutputPort());
  mp1->ScalarVisibilityOff();

  vtkNew<vtkActor> ac1;
  ac1->SetMapper(mp1);
  ac1->GetProperty()->LightingOff();
  ac1->GetProperty()->SetColor(0, 1, 1);
  renderer->AddActor(ac1);

  vtkNew<vtkRenderWindow> window;
  window->AddRenderer(renderer);
  window->SetOffScreenRendering(true);
  window->SetSize(1024, 768);

  renderer->ResetCamera();

  vtkNew<vtkWindowToImageFilter> w2i;
  w2i->SetInput(window);
  w2i->SetInputBufferTypeToRGB();
  w2i->Update();

  auto t2 = std::chrono::high_resolution_clock::now();

  vtkNew<vtkPNGWriter> png;
  png->SetFileName(outputPng);
  png->SetInputConnection(w2i->GetOutputPort());
  png->Write();

  auto t3 = std::chrono::high_resolution_clock::now();

  std::cout << "baryon-mesh, " << cf->GetOutput()->GetNumberOfCells() << ", "
            << cf->GetOutput()->GetNumberOfPoints() << std::endl;
  std::cout << "contouring: " << std::chrono::duration<double>(t1 - t0).count() << std::endl
            << "rendering: " << std::chrono::duration<double>(t3 - t1).count() << std::endl
            << " - win2image: " << std::chrono::duration<double>(t2 - t1).count() << std::endl
            << " - png: " << std::chrono::duration<double>(t3 - t2).count() << std::endl;

  vtkNew<vtkXMLPolyDataWriter> writer;
  writer->SetCompressorTypeToNone();
  writer->EncodeAppendedDataOff();
  writer->SetInputConnection(cf->GetOutputPort());
  writer->SetWriteToOutputString(true);
  writer->Write();
  std::cout << "baryon-size, " << writer->GetOutputString().size() << std::endl;
}

void Run(const char* inputVTK, const char* outputPng)
{
  auto t0 = std::chrono::high_resolution_clock::now();

  vtkNew<vtkXMLImageDataReader> reader;
  reader->SetFileName(inputVTK);
  reader->Update();

  auto t1 = std::chrono::high_resolution_clock::now();

  std::cout << "io: " << std::chrono::duration<double>(t1 - t0).count() << std::endl;

  Run0(reader.Get(), outputPng);
}

int main(int argc, char* argv[])
{
  int c;
  while ((c = getopt(argc, argv, "h")) != -1)
  {
    switch (c)
    {
      case 'h':
      default:
        std::cerr << "Usage: " << argv[0] << " <VTK filename>" << std::endl;
        exit(EXIT_FAILURE);
    }
  }
  argc -= optind;
  argv += optind;
  if (!argc)
  {
    std::cerr << "Lack target vti filename" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::string outputPng = std::filesystem::path(argv[0]).stem().string() + ".png";
  std::cout << "vtk file: " << argv[0] << std::endl;
  std::cout << "output png: " << outputPng << std::endl;
  Run(argv[0], outputPng.c_str());
  return 0;
}
