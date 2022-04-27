/*
 * Copyright © 2020 Antonin Stefanutti <antonin.stefanutti@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#define _GNU_SOURCE

#include <GLES3/gl3.h>
#include <assert.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "Shader.hpp"
#include "common.h"

GLint iTime, iFrame;

Shader *gShader;
int gVertexCount;

float width, height;
unsigned int VAO;
unsigned int VBO;

static const char *shadertoy_vs =
    "attribute vec3 position;                \n"
    "void main()                             \n"
    "{                                       \n"
    "    gl_Position = vec4(position, 1.0);  \n"
    "}                                       \n";

static const char *shadertoy_fs_tmpl =
    "precision mediump float;                                                  "
    "           \n"
    "uniform vec3      iResolution;           // viewport resolution (in "
    "pixels)          \n"
    "uniform float     iTime;                 // shader playback time (in "
    "seconds)        \n"
    "uniform int       iFrame;                // current frame number          "
    "           \n"
    "uniform vec4      iMouse;                // mouse pixel coords            "
    "           \n"
    "uniform vec4      iDate;                 // (year, month, day, time in "
    "seconds)      \n"
    "uniform float     iSampleRate;           // sound sample rate (i.e., "
    "44100)          \n"
    "uniform vec3      iChannelResolution[4]; // channel resolution (in "
    "pixels)           \n"
    "uniform float     iChannelTime[4];       // channel playback time (in "
    "sec)           \n"
    "                                                                          "
    "           \n"
    "%s                                                                        "
    "           \n"
    "                                                                          "
    "           \n"
    "void main()                                                               "
    "           \n"
    "{                                                                         "
    "           \n"
    "    mainImage(gl_FragColor, gl_FragCoord.xy);                             "
    "           \n"
    "}                                                                         "
    "           \n";

static const GLfloat vertices[] = {
    // First triangle:
    1.0f,
    1.0f,
    -1.0f,
    1.0f,
    -1.0f,
    -1.0f,
    // Second triangle:
    -1.0f,
    -1.0f,
    1.0f,
    -1.0f,
    1.0f,
    1.0f,
};

// static char *load_shader(const char *file) {
// 	struct stat statbuf;
// 	char *frag;
// 	int fd, ret;

// 	fd = open(file, 0);
// 	if (fd < 0) {
// 		err(fd, "could not open '%s'", file);
// 	}

// 	ret = fstat(fd, &statbuf);
// 	if (ret < 0) {
// 		err(ret, "could not stat '%s'", file);
// 	}

// 	const char *text = mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE,
// fd, 0); 	asprintf(&frag, shadertoy_fs_tmpl, text);

// 	return frag;
// }

extern "C" void draw_shadertoy(uint64_t start_time, unsigned frame) {
  // glUniform1f(iTime, (get_time_ns() - start_time) / (double) NSEC_PER_SEC);
  // // Replace the above to input ellapsed time relative to 60 FPS
  // // glUniform1f(iTime, (float) frame / 60.0f);
  // glUniform1ui(iFrame, frame);

	std::vector<float> mVertices;
  for (int i = 0; i < 256; ++i) {
  	float x = i / 256. * width;
    float y = (std::sin(3.14 * 8 * i + frame * 0.01) + 0.5 * 0.5) * height;
    mVertices.insert(mVertices.end(), {x, y});
  }

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(*mVertices.data()),
               mVertices.data(), GL_STREAM_DRAW);

  gVertexCount = mVertices.size() / 2;

  // glm::mat4 trans = glm::mat4(1.0f);
  // trans = glm::translate(
      // trans, glm::vec3(std::fmod(frame / 60.f, 2.0f) - 1.f, 0.0, 0.0f));

  // gShader->setMat4("transform", trans);

  start_perfcntrs();

  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);

  glDrawArrays(GL_LINE_STRIP, 0, gVertexCount);

  end_perfcntrs();
}

extern "C" int init_shadertoy(const struct gbm *gbm, struct egl *egl,
                              const char *file) {
  int ret;

  gShader = new Shader{"shaders/geomVertexShader.glsl",
                       "shaders/geomFragmentShader.glsl"};

  width = gbm->width;
  height = gbm->height;

  glm::mat4 mProjectionMatrix;

  glViewport(0, 0, width, height);

  mProjectionMatrix = glm::ortho(0.0f, width, 0.0f, height);
  gShader->use();
  gShader->setMat4("projection", mProjectionMatrix);
  gShader->setVec2("viewportSize", {width, height});
  // gShader->setFloat("lineWidth", 40.0);

  glEnable(GL_DEPTH_TEST);

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  egl->draw = draw_shadertoy;

  return 0;
}
