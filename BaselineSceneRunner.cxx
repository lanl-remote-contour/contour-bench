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
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkWindowToImageFilter.h>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLUnstructuredGridReader.h>

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

  // v02
  vtkNew<vtkContourFilter> cf1;
  cf1->SetInputConnection(input->GetOutputPort());
  cf1->ComputeScalarsOff(); // No scalars or normals please
  cf1->ComputeNormalsOff();
  cf1->SetInputArrayToProcess(
    0, 0, 0, vtkDataObject::FieldAssociations::FIELD_ASSOCIATION_POINTS, "v02");
  cf1->SetValue(0, 0.8);
  cf1->Update();
  std::cout << "v02, " << cf1->GetOutput()->GetNumberOfCells() << ", "
            << cf1->GetOutput()->GetNumberOfPoints() << std::endl;

  // v03
  vtkNew<vtkContourFilter> cf2;
  cf2->SetInputConnection(input->GetOutputPort());
  cf2->ComputeScalarsOff(); // No scalars or normals please
  cf2->ComputeNormalsOff();
  cf2->SetInputArrayToProcess(
    0, 0, 0, vtkDataObject::FieldAssociations::FIELD_ASSOCIATION_POINTS, "v03");
  cf2->SetValue(0, 0.5);
  cf2->Update();
  std::cout << "v03: " << cf2->GetOutput()->GetNumberOfCells() << ", "
            << cf2->GetOutput()->GetNumberOfPoints() << std::endl;

  // tev
  vtkNew<vtkContourFilter> cf3;
  cf3->SetInputConnection(input->GetOutputPort());
  cf3->ComputeScalarsOff(); // No scalars or normals please
  cf3->ComputeNormalsOff();
  cf3->SetInputArrayToProcess(
    0, 0, 0, vtkDataObject::FieldAssociations::FIELD_ASSOCIATION_POINTS, "tev");
  cf3->SetValue(0, 0.1);
  cf3->Update();
  std::cout << "tev: " << cf3->GetOutput()->GetNumberOfCells() << ", "
            << cf3->GetOutput()->GetNumberOfPoints() << std::endl;

  auto t1 = std::chrono::high_resolution_clock::now();

  vtkNew<vtkRenderer> renderer;
  renderer->SetBackground(0.321, 0.341, 0.431);

  vtkNew<vtkImageData> img;
  img->SetExtent(0, 149, 0, 149, 0, 149);
  img->SetOrigin(-2300000, -500000, -1200000);
  img->SetSpacing(30872.4, 18791.9, 16107.4);

  vtkNew<vtkOutlineFilter> of;
  of->SetInputData(img);

  vtkNew<vtkPolyDataMapper> mp0;
  mp0->SetInputConnection(of->GetOutputPort());
  mp0->ScalarVisibilityOff();

  vtkNew<vtkActor> ac0;
  ac0->SetMapper(mp0);
  ac0->GetProperty()->LightingOff();
  ac0->GetProperty()->SetColor(1, 1, 1);
  renderer->AddActor(ac0);

  // v02
  vtkNew<vtkPolyDataMapper> mp1;
  mp1->SetInputConnection(cf1->GetOutputPort());
  mp1->ScalarVisibilityOff();

  vtkNew<vtkActor> ac1;
  ac1->SetMapper(mp1);
  ac1->GetProperty()->LightingOff();
  ac1->GetProperty()->SetColor(0.012, 0.686, 1);
  ac1->GetProperty()->SetOpacity(0.3);
  renderer->AddActor(ac1);

  // v03
  vtkNew<vtkPolyDataMapper> mp2;
  mp2->SetInputConnection(cf2->GetOutputPort());
  mp2->ScalarVisibilityOff();

  vtkNew<vtkActor> ac2;
  ac2->SetMapper(mp2);
  ac2->GetProperty()->LightingOff();
  ac2->GetProperty()->SetColor(1, 0.333, 0);
  ac2->GetProperty()->SetOpacity(0.8);
  renderer->AddActor(ac2);

  // tev
  vtkNew<vtkPolyDataMapper> mp3;
  mp3->SetInputConnection(cf3->GetOutputPort());
  mp3->ScalarVisibilityOff();

  vtkNew<vtkActor> ac3;
  ac3->SetMapper(mp3);
  ac3->GetProperty()->LightingOff();
  ac3->GetProperty()->SetColor(0.816, 0.816, 0);
  ac3->GetProperty()->SetOpacity(0.15);
  renderer->AddActor(ac3);

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

  std::cout << "contouring: " << std::chrono::duration<double>(t1 - t0).count() << std::endl
            << "rendering: " << std::chrono::duration<double>(t3 - t1).count() << std::endl
            << " - win2image: " << std::chrono::duration<double>(t2 - t1).count() << std::endl
            << " - png: " << std::chrono::duration<double>(t3 - t2).count() << std::endl;

  vtkNew<vtkXMLPolyDataWriter> writer;
  writer->SetWriteToOutputString(true);
  writer->SetInputConnection(cf1->GetOutputPort());
  writer->Write();
  std::cout << "v02, " << writer->GetOutputString().size() << std::endl;
  
  writer->SetWriteToOutputString(true);
  writer->SetInputConnection(cf2->GetOutputPort());
  writer->Write();
  std::cout << "v03, " << writer->GetOutputString().size() << std::endl;

  writer->SetWriteToOutputString(true);
  writer->SetInputConnection(cf3->GetOutputPort());
  writer->Write();
  std::cout << "tev, " << writer->GetOutputString().size() << std::endl;
}

void Run(const char* inputVTK, const char* outputPng)
{
  char t = inputVTK[strlen(inputVTK) - 1];
  if (t == 'i')
  {
    auto t0 = std::chrono::high_resolution_clock::now();

    vtkNew<vtkXMLImageDataReader> reader;
    reader->SetFileName(inputVTK);
    reader->Update();

    auto t1 = std::chrono::high_resolution_clock::now();

    std::cout << "io: " << std::chrono::duration<double>(t1 - t0).count() << std::endl;

    Run0(reader.Get(), outputPng);
  }
  else if (t == 'u')
  {
    auto t0 = std::chrono::high_resolution_clock::now();

    vtkNew<vtkXMLUnstructuredGridReader> reader;
    reader->SetFileName(inputVTK);
    reader->Update();

    auto t1 = std::chrono::high_resolution_clock::now();

    std::cout << "io: " << std::chrono::duration<double>(t1 - t0).count() << std::endl;

    Run0(reader.Get(), outputPng);
  }
  else
  {
    std::cerr << "Unknown VTK file type: " << inputVTK << std::endl;
    exit(EXIT_FAILURE);
  }
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
    std::cerr << "Lack target vti/vtu filename" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::string outputPng = std::filesystem::path(argv[0]).stem().string() + ".png";
  std::cout << "vtk file: " << argv[0] << std::endl;
  std::cout << "output png: " << outputPng << std::endl;
  Run(argv[0], outputPng.c_str());
  return 0;
}
