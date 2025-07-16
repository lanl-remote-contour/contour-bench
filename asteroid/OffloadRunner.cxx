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
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkOutlineFilter.h>
#include <vtkPNGWriter.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkWindowToImageFilter.h>
#include <vtkXMLPolyDataReader.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <stdlib.h>
#include <string>

void Run(const char* pushdown_command_dest, const char* result1, const char* result2,
  const char* result3, const char* inputVtk, const char* outputPng)
{
  auto t0 = std::chrono::high_resolution_clock::now();

  {
    std::ofstream cmd;
    cmd.open(pushdown_command_dest, std::ios::out | std::ios::binary | std::ios::trunc);
    cmd << inputVtk << std::endl;
    cmd.close();
    if (!cmd.good())
    {
      std::cerr << "Cannot write pushdown commands" << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  vtkNew<vtkXMLPolyDataReader> r1;
  r1->SetFileName(result1);
  r1->Update();

  vtkNew<vtkXMLPolyDataReader> r2;
  r2->SetFileName(result2);
  r2->Update();

  vtkNew<vtkXMLPolyDataReader> r3;
  r3->SetFileName(result3);
  r3->Update();

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
  mp1->SetInputConnection(r1->GetOutputPort());
  mp1->ScalarVisibilityOff();

  vtkNew<vtkActor> ac1;
  ac1->SetMapper(mp1);
  ac1->GetProperty()->LightingOff();
  ac1->GetProperty()->SetColor(0.012, 0.686, 1);
  ac1->GetProperty()->SetOpacity(0.3);
  renderer->AddActor(ac1);

  // v03
  vtkNew<vtkPolyDataMapper> mp2;
  mp2->SetInputConnection(r2->GetOutputPort());
  mp2->ScalarVisibilityOff();

  vtkNew<vtkActor> ac2;
  ac2->SetMapper(mp2);
  ac2->GetProperty()->LightingOff();
  ac2->GetProperty()->SetColor(1, 0.333, 0);
  ac2->GetProperty()->SetOpacity(0.8);
  renderer->AddActor(ac2);

  // tev
  vtkNew<vtkPolyDataMapper> mp3;
  mp3->SetInputConnection(r3->GetOutputPort());
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

  std::cout << "v02-mesh, " << r1->GetOutput()->GetNumberOfCells() << ", "
            << r1->GetOutput()->GetNumberOfPoints() << std::endl;
  std::cout << "v03-mesh, " << r2->GetOutput()->GetNumberOfCells() << ", "
            << r2->GetOutput()->GetNumberOfPoints() << std::endl;
  std::cout << "tev-mesh, " << r3->GetOutput()->GetNumberOfCells() << ", "
            << r3->GetOutput()->GetNumberOfPoints() << std::endl;

  std::cout << "io-contouring: " << std::chrono::duration<double>(t1 - t0).count() << std::endl
            << "rendering: " << std::chrono::duration<double>(t3 - t1).count() << std::endl
            << " - win2image: " << std::chrono::duration<double>(t2 - t1).count() << std::endl
            << " - png: " << std::chrono::duration<double>(t3 - t2).count() << std::endl;
}

int main(int argc, char* argv[])
{
  const char* pushdown_command_dest = "/fuse/command";
  const char* result_prefix = "/fuse/result";
  int c;
  while ((c = getopt(argc, argv, "d:s:h")) != -1)
  {
    switch (c)
    {
      case 'd':
        pushdown_command_dest = optarg;
        break;
      case 's':
        result_prefix = optarg;
        break;
      case 'h':
      default:
        std::cerr
          << "-d to specify pushdown command file, and -s to specify pushdown result file prefix"
          << std::endl;
        exit(EXIT_FAILURE);
    }
  }
  argc -= optind;
  argv += optind;
  if (!argc)
  {
    std::cerr << "Lack target vtk filename" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::string r0 = std::string(result_prefix) + "0";
  std::string r1 = std::string(result_prefix) + "1";
  std::string r2 = std::string(result_prefix) + "2";
  std::string outputPng = std::filesystem::path(argv[0]).stem().string() + ".png";
  std::cout << "pushdown analysis command file: " << pushdown_command_dest << std::endl;
  std::cout << "pushdown result file: " << result_prefix << "[0-2]" << std::endl;
  std::cout << "vtk file: " << argv[0] << std::endl;
  Run(pushdown_command_dest, r0.c_str(), r1.c_str(), r2.c_str(), argv[0], outputPng.c_str());
  return 0;
}
