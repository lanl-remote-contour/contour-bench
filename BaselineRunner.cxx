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
#include <vtkDataArraySelection.h>
#include <vtkFieldData.h>
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
#include <vtkXMLUnstructuredGridReader.h>

#include <chrono>
#include <filesystem>
#include <getopt.h>
#include <iostream>
#include <stdlib.h>

void Run0(vtkAlgorithm* input, const char* arrayName, double contourValue, const char* outputPng)
{
  auto t0 = std::chrono::high_resolution_clock::now();

  vtkNew<vtkContourFilter> cf;
  cf->SetInputConnection(input->GetOutputPort());
  cf->ComputeScalarsOff(); // No scalars or normals please
  cf->ComputeNormalsOff();
  cf->SetInputArrayToProcess(
    0, 0, 0, vtkDataObject::FieldAssociations::FIELD_ASSOCIATION_POINTS, arrayName);
  cf->SetValue(0, contourValue);
  cf->Update();

  auto t1 = std::chrono::high_resolution_clock::now();

  vtkNew<vtkRenderer> renderer;
  renderer->SetBackground(0.321, 0.341, 0.431);

  {
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(cf->GetOutputPort());
    mapper->ScalarVisibilityOff();

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->LightingOff();
    actor->GetProperty()->SetColor(1, 0.333, 0);
    actor->GetProperty()->SetOpacity(0.8);
    renderer->AddActor(actor);
  }

  {
    vtkNew<vtkImageData> image;
    image->SetExtent(0, 149, 0, 149, 0, 149);
    image->SetOrigin(-2300000, -500000, -1200000);
    image->SetSpacing(30872.4, 18791.9, 16107.4);

    vtkNew<vtkOutlineFilter> of;
    of->SetInputData(image);

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(of->GetOutputPort());
    mapper->ScalarVisibilityOff();

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->LightingOff();
    actor->GetProperty()->SetColor(1, 1, 1);
    renderer->AddActor(actor);
  }

  vtkNew<vtkRenderWindow> window;
  window->AddRenderer(renderer);
  window->SetOffScreenRendering(true);
  window->SetSize(1024, 768);

  renderer->ResetCamera();

  vtkNew<vtkWindowToImageFilter> image;
  image->SetInput(window);
  image->SetInputBufferTypeToRGB();
  image->Update();

  auto t2 = std::chrono::high_resolution_clock::now();

  vtkNew<vtkPNGWriter> png;
  png->SetFileName(outputPng);
  png->SetInputConnection(image->GetOutputPort());
  png->Write();

  auto t3 = std::chrono::high_resolution_clock::now();

  std::cout << "contouring: " << std::chrono::duration<double>(t1 - t0).count() << std::endl
            << "rendering: " << std::chrono::duration<double>(t3 - t1).count() << std::endl
            << " - win2image: " << std::chrono::duration<double>(t2 - t1).count() << std::endl
            << " - png: " << std::chrono::duration<double>(t3 - t2).count() << std::endl;
}

void Run(const char* inputVTK, const char* arrayName, double contourValue, const char* outputPng)
{
  char t = inputVTK[strlen(inputVTK) - 1];
  if (t == 'i')
  {
    auto t0 = std::chrono::high_resolution_clock::now();

    vtkNew<vtkXMLImageDataReader> reader;
    reader->SetFileName(inputVTK);
    reader->UpdateInformation();
    vtkDataArraySelection* das = reader->GetPointDataArraySelection();
    das->DisableAllArrays();
    das->EnableArray(arrayName);
    reader->Update();
    reader->GetOutput()
      ->GetFieldData() // Remove all field data
      ->AllocateArrays(0);

    auto t1 = std::chrono::high_resolution_clock::now();

    std::cout << "io: " << std::chrono::duration<double>(t1 - t0).count() << std::endl;

    Run0(reader.Get(), arrayName, contourValue, outputPng);
  }
  else if (t == 'u')
  {
    auto t0 = std::chrono::high_resolution_clock::now();

    vtkNew<vtkXMLUnstructuredGridReader> reader;
    reader->SetFileName(inputVTK);
    reader->UpdateInformation();
    vtkDataArraySelection* das = reader->GetCellDataArraySelection();
    das->DisableAllArrays();
    das->EnableArray(arrayName);
    reader->Update();
    reader->GetOutput()
      ->GetFieldData() // Remove all field data
      ->AllocateArrays(0);

    auto t1 = std::chrono::high_resolution_clock::now();

    std::cout << "io: " << std::chrono::duration<double>(t1 - t0).count() << std::endl;

    Run0(reader.Get(), arrayName, contourValue, outputPng);
  }
  else
  {
    std::cerr << "Unknown VTK file type: " << inputVTK << std::endl;
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char* argv[])
{
  const char* arr = "v03";
  double value = 0.5;
  int c;
  while ((c = getopt(argc, argv, "a:c:h")) != -1)
  {
    switch (c)
    {
      case 'a':
        arr = optarg;
        break;
      case 'c':
        value = atof(optarg);
        break;
      case 'h':
      default:
        std::cerr << "Use -a to specify array name and -c to specify contour value" << std::endl;
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
  std::cout << "contour value: " << value << std::endl;
  std::cout << "array: " << arr << std::endl;
  std::cout << "output png: " << outputPng << std::endl;
  Run(argv[0], arr, value, outputPng.c_str());
  return 0;
}
