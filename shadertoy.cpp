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

#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <GLES3/gl3.h>

#include "common.h"

GLint iTime, iFrame;

static const char *shadertoy_vs =
		"attribute vec3 position;                \n"
		"void main()                             \n"
		"{                                       \n"
		"    gl_Position = vec4(position, 1.0);  \n"
		"}                                       \n";

static const char *shadertoy_fs_tmpl =
		"precision mediump float;                                                             \n"
		"uniform vec3      iResolution;           // viewport resolution (in pixels)          \n"
		"uniform float     iTime;                 // shader playback time (in seconds)        \n"
		"uniform int       iFrame;                // current frame number                     \n"
		"uniform vec4      iMouse;                // mouse pixel coords                       \n"
		"uniform vec4      iDate;                 // (year, month, day, time in seconds)      \n"
		"uniform float     iSampleRate;           // sound sample rate (i.e., 44100)          \n"
		"uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)           \n"
		"uniform float     iChannelTime[4];       // channel playback time (in sec)           \n"
		"                                                                                     \n"
		"%s                                                                                   \n"
		"                                                                                     \n"
		"void main()                                                                          \n"
		"{                                                                                    \n"
		"    mainImage(gl_FragColor, gl_FragCoord.xy);                                        \n"
		"}                                                                                    \n";

static const GLfloat vertices[] = {
		// First triangle:
		1.0f, 1.0f,
		-1.0f, 1.0f,
		-1.0f, -1.0f,
		// Second triangle:
		-1.0f, -1.0f,
		1.0f, -1.0f,
		1.0f, 1.0f,
};

static char *load_shader(const char *file) {
	struct stat statbuf;
	char *frag;
	int fd, ret;

	fd = open(file, 0);
	if (fd < 0) {
		err(fd, "could not open '%s'", file);
	}

	ret = fstat(fd, &statbuf);
	if (ret < 0) {
		err(ret, "could not stat '%s'", file);
	}

	const char *text = mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	asprintf(&frag, shadertoy_fs_tmpl, text);

	return frag;
}

static void draw_shadertoy(uint64_t start_time, unsigned frame) {
	// glUniform1f(iTime, (get_time_ns() - start_time) / (double) NSEC_PER_SEC);
	// // Replace the above to input ellapsed time relative to 60 FPS
	// // glUniform1f(iTime, (float) frame / 60.0f);
	// glUniform1ui(iFrame, frame);

	start_perfcntrs();

	// glDrawArrays(GL_TRIANGLES, 0, 6);

	glm::mat4 trans = glm::mat4(1.0f);
	trans = glm::translate(trans, glm::vec3(std::fmod(frame / 30.f, 2.0f) - 1.f, 0.0, 0.0f));

	mGeomShader.setMat4("transform", trans);

	// glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_LINE_STRIP, 0, vertexCount);


	end_perfcntrs();
}

int init_shadertoy(const struct gbm *gbm, struct egl *egl, const char *file) {
	int ret;

	Shader mGeomShader{SHADER_PATH "/geomVertexShader.glsl",
	                   SHADER_PATH "/geomFragmentShader.glsl"};

	float width = WIDTH, height = HEIGHT;
	std::vector<float> mVertices;

	unsigned int VAO;
	unsigned int VBO;

	glm::mat4 mProjectionMatrix;

	glViewport(0, 0, WIDTH, HEIGHT);

	mProjectionMatrix = glm::ortho(0.0f, width, 0.0f, height);
	mGeomShader.use();
	mGeomShader.setMat4("projection", mProjectionMatrix);
	mGeomShader.setVec2("viewportSize", {width, height});
	// mGeomShader.setFloat("lineWidth", 40.0);

	glEnable(GL_DEPTH_TEST);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	float t = 0.0f;
	float x = WIDTH * 0.5f;
	mVertices.insert(mVertices.end(), {x, 0.f});
	mVertices.insert(mVertices.end(), {x, HEIGHT});

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(*mVertices.data()),
	             mVertices.data(), GL_STREAM_DRAW);

	int vertexCount = mVertices.size() / 2;
	mVertices.clear();

	egl->draw = draw_shadertoy;

	return 0;
}