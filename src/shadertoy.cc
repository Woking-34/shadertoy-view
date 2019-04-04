/*
A native shadertoy-compatible GLSL fragment shader previewer.
Copyright (C) 2013  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <assert.h>
#include <GL/glew.h>

#ifndef __APPLE__
#include <GL/freeglut.h>
#else
#include <GLUT/glut.h>
#endif

#include <imago2.h>

struct Texture {
	unsigned int id;
	unsigned int targ;
	const char *stype;
};

int frameCounter = 0;
int prev_time = 0;
int curr_time = 0;
int delta_time = 0;
std::string sampleNameStr;

void disp();
void idle();
void reshape(int x, int y);
void keyb(unsigned char key, int x, int y);
void mouse(int bn, int state, int x, int y);
void motion(int x, int y);
void fps(int value);
unsigned int load_shader(const char *fname);
bool load_shader_metadata(const char *sdrname);
Texture *load_texture(const char *fname);
Texture *load_cubemap(const char *fname_fmt);
bool parse_args(int argc, char **argv);

unsigned int sdr;
static struct {
	int resolution;
	int globaltime;
	int channeltime[4];
	int channelres[4];
	int mouse;
	int sampler[4];
	int date;
} uloc;
std::vector<Texture*> textures;
Texture notex = { 0, GL_TEXTURE_2D, "2D" };
Texture *activetex[4] = { &notex, &notex, &notex, &notex };

int texW[4] = { -1, -1, -1, -1 };
int texH[4] = { -1, -1, -1, -1 };

bool isFullScreen = false;

int win_width, win_height;
int mouse_x, mouse_y, click_x, click_y;

std::vector<int> tex_arg;
const char *sdrfname_arg;

int LOADTEX(const std::string& fname, int& dataidx)
{
	Texture *tex = load_texture(fname.c_str());
	if(!tex) {
		return 1;
	}
	textures.push_back(tex);
	printf("  Texture2D %2d: %s\n", dataidx++, fname.c_str());
}

//#define LOADCUBE(fname) \
//	do { \
//		Texture *tex = load_cubemap(fname.c_str()); \
//		if(!tex) { \
//			return 1; \
//		} \
//		textures.push_back(tex); \
//		printf("TextureCube %2d: %s\n", dataidx++, fname.c_str()); \
//	} while(0)

#include <string>
#include <fstream>
#include <sstream>

std::string rootData = "";

/** file path helper */
static bool findFullPath(const std::string& root, std::string& filePath)
{
	bool fileFound = false;
	const std::string resourcePath = root;

	filePath = resourcePath + filePath;
	for (unsigned int i = 0; i < 16; ++i)
	{
		std::ifstream file;
		file.open(filePath.c_str());
		if (file.is_open())
		{
			fileFound = true;
			break;
		}

		filePath = "../" + filePath;
	}

	return fileFound;
}

// ../data/Lanterns.glsl -t 8 -t 11
// ../data/Woods.glsl -t 13 -t 6 -t 9 -t 7
// ../data/Clouds.glsl -t 13
// ../data/MountainPeak.glsl

int main(int argc, char **argv)
{
	{
		std::string locStr = "filesystem.loc";
		size_t len = locStr.size();

		bool fileFound = findFullPath("data/", locStr);
		rootData = locStr.substr(0, locStr.size() - len);
	}

	glutInitWindowSize(1280, 720);
	glutInit(&argc, argv);

	if (!parse_args(argc, argv)) {
		return 1;
	}

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_MULTISAMPLE);
	glutCreateWindow("ShaderToy viewer");

	glutDisplayFunc(disp);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyb);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutTimerFunc(0, &fps, 0);

	//glDisable(GL_MULTISAMPLE);

	glEnable(GL_MULTISAMPLE);
	glutSetOption(GLUT_MULTISAMPLE, 16);
	glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);

	// GLEW_OK
	GLenum isGLEWOK = glewInit();
	if (GLEW_OK != isGLEWOK)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(isGLEWOK));
		return 1;
	}

	int dataidx = 0;

	LOADTEX(rootData + "textures/tex00.jpg", dataidx);
	LOADTEX(rootData + "textures/tex01.jpg", dataidx);
	LOADTEX(rootData + "textures/tex02.jpg", dataidx);
	LOADTEX(rootData + "textures/tex03.jpg", dataidx);
	LOADTEX(rootData + "textures/tex04.jpg", dataidx);
	LOADTEX(rootData + "textures/tex05.jpg", dataidx);
	LOADTEX(rootData + "textures/tex06.jpg", dataidx);
	LOADTEX(rootData + "textures/tex07.jpg", dataidx);
	LOADTEX(rootData + "textures/tex08.jpg", dataidx);
	LOADTEX(rootData + "textures/tex09.jpg", dataidx);
	LOADTEX(rootData + "textures/tex10.png", dataidx);
	LOADTEX(rootData + "textures/tex11.png", dataidx);
	LOADTEX(rootData + "textures/tex12.png", dataidx);
	LOADTEX(rootData + "textures/tex16.png", dataidx);
	assert(glGetError() == GL_NO_ERROR);

	//LOADCUBE("data/cube00_%d.jpg");
	//LOADCUBE("data/cube01_%d.png");
	//LOADCUBE("data/cube02_%d.jpg");
	//LOADCUBE("data/cube03_%d.png");
	//LOADCUBE("data/cube04_%d.png");
	//LOADCUBE("data/cube05_%d.png");

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	//load_shader_metadata(sdrfname_arg);

	// override with -t arguments
	for(size_t i=0; i<tex_arg.size(); i++) {
		activetex[i] = textures[tex_arg[i]];

		texW[i] = 64;
		texH[i] = 64;
	}

	if(!(sdr = load_shader(sdrfname_arg))) {
		return 1;
	}

	assert(glGetError() == GL_NO_ERROR);

	if (isFullScreen)
	{
		glutFullScreen();
		assert(glGetError() == GL_NO_ERROR);
	}

	{
		GLint iMultiSample = 0;
		GLint iNumSamples = 0;
		glGetIntegerv(GL_SAMPLE_BUFFERS, &iMultiSample);
		glGetIntegerv(GL_SAMPLES, &iNumSamples);
		assert(glGetError() == GL_NO_ERROR);

		printf("GL_SAMPLE_BUFFERS = %d, GL_SAMPLES = %d\n", iMultiSample, iNumSamples);
	}

	{
		printf("WINDOW_WIDTH x WINDOW_HEIGHT: %d x %d", glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	}

	glutMainLoop();

	return 0;
}

void disp()
{
	++frameCounter;

	time_t tmsec = time(0);
	struct tm *tm = localtime(&tmsec);

	glClear(GL_COLOR_BUFFER_BIT);

	assert(glGetError() == GL_NO_ERROR);
	glUseProgram(sdr);
	assert(glGetError() == GL_NO_ERROR);

	// set uniforms
	glUniform2f(uloc.resolution, win_width, win_height);
	GLenum err = glGetError();

	assert(glGetError() == GL_NO_ERROR);
	glUniform1f(uloc.globaltime, glutGet(GLUT_ELAPSED_TIME) / 1000.0);
	assert(glGetError() == GL_NO_ERROR);
	glUniform4f(uloc.mouse, mouse_x, mouse_y, click_x, click_y);
	assert(glGetError() == GL_NO_ERROR);
	//glUniform4f(uloc.date, tm->tm_year, tm->tm_mon, tm->tm_mday,
	//		tm->tm_sec + tm->tm_min * 60 + tm->tm_hour * 3600);
	//assert(glGetError() == GL_NO_ERROR);

	int tunit = 0;
	for(int i=0; i<4; i++) {
		if(activetex[i]->id) {
			glActiveTexture(GL_TEXTURE0 + tunit);
			glBindTexture(activetex[i]->targ, activetex[i]->id);
			glUniform1i(uloc.sampler[i], tunit);
			tunit++;
		}
	}
	assert(glGetError() == GL_NO_ERROR);

	glBegin(GL_QUADS);
	glVertex2f(-1, -1);
	glVertex2f(1, -1);
	glVertex2f(1, 1);
	glVertex2f(-1, 1);
	glEnd();

	glutSwapBuffers();
	assert(glGetError() == GL_NO_ERROR);
}

void idle()
{
	prev_time = curr_time;
	curr_time = glutGet(GLUT_ELAPSED_TIME);
	delta_time = curr_time - prev_time;

	glutPostRedisplay();
}

void reshape(int x, int y)
{
	glViewport(0, 0, x, y);
	win_width = x;
	win_height = y;
}

void keyb(unsigned char key, int x, int y)
{
	switch(key) {
	case 27:
		exit(0);

	case 'f':
	case 'F':
	{
		static int orig_width = -1, orig_height;

		if(orig_width != -1) {
			glutReshapeWindow(orig_width, orig_height);
			orig_width = -1;
		} else {
			orig_width = win_width;
			orig_height = win_height;
			glutFullScreen();
		}
	}
	break;

	case '*':
	{
		unsigned char* buffer = new unsigned char[win_width * win_height * 4];

		glReadBuffer(GL_BACK);

		glReadPixels(0, 0, win_width, win_height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)buffer);

		std::fstream fh((sampleNameStr + ".ppm").c_str(), std::fstream::out | std::fstream::binary);

		fh << "P6\n";
		fh << win_width << "\n" << win_height << "\n" << 0xff << std::endl;

		for (int j = 0; j < win_height; ++j)
		{
			for (int i = 0; i < win_width; ++i)
			{
				// flip ogl backbuffer vertically
				fh << buffer[4 * ((win_height - 1 - j)*win_width + i) + 0];
				fh << buffer[4 * ((win_height - 1 - j)*win_width + i) + 1];
				fh << buffer[4 * ((win_height - 1 - j)*win_width + i) + 2];
			}
		}
		fh.flush();
		fh.close();

		delete[] buffer;
	}
	break;
	}
}

void mouse(int bn, int state, int x, int y)
{
	click_x = x;
	click_y = y;
}

void motion(int x, int y)
{
	mouse_x = x;
	mouse_y = y;
}

void fps(int value)
{
	std::stringstream titleSS;
	titleSS << "ShaderToy viewer - " << sampleNameStr << " - " << delta_time << " ms / " << 4 * frameCounter << " FPS @ " << win_width << " x " << win_height;

	frameCounter = 0;

	glutSetWindowTitle(titleSS.str().c_str());
	glutTimerFunc(250, &fps, 1);
}

//static const char *header =
//	"uniform vec2 iResolution;\n"
//	"uniform float iGlobalTime;\n"
//	"uniform float iChannelTime[4];\n"
//	"uniform vec4 iMouse;\n"
//	"uniform sampler%s iChannel0;\n"
//	"uniform sampler%s iChannel1;\n"
//	"uniform sampler%s iChannel2;\n"
//	"uniform sampler%s iChannel3;\n"
//	"uniform vec4 iDate;\n";

static const char *header = "\n";

unsigned int load_shader(const char *fname)
{
	FILE *fp = fopen(fname, "rb");
	if(!fp) {
		fprintf(stderr, "failed to open shader: %s: %s\n", fname, strerror(errno));
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	long filesz = ftell(fp);
	rewind(fp);

	int sz = filesz + strlen(header) + 32;

	char *src = new char[sz + 1];
	memset(src, ' ', sz);
	sprintf(src, header, activetex[0]->stype, activetex[1]->stype, activetex[2]->stype, activetex[3]->stype);

	if(fread(src + strlen(src), 1, filesz, fp) < (size_t)filesz) {
		fprintf(stderr, "failed to read shader: %s: %s\n", fname, strerror(errno));
		fclose(fp);
		return 0;
	}
	fclose(fp);
	src[sz] = 0;

	printf("compiling shader: %s\n", fname);
	//printf("SOURCE: %s\n", src);
	unsigned int sdr = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(sdr, 1, (const char**)&src, 0);
	glCompileShader(sdr);
	delete [] src;

	int status, loglength;
	glGetShaderiv(sdr, GL_COMPILE_STATUS, &status);
	glGetShaderiv(sdr, GL_INFO_LOG_LENGTH, &loglength);

	if(loglength) {
		char *buf = new char[loglength + 1];
		glGetShaderInfoLog(sdr, loglength, 0, buf);
		buf[loglength] = 0;

		fprintf(stderr, "%s\n", buf);
		delete [] buf;
	}
	if(!status) {
		fprintf(stderr, "failed\n");
		glDeleteShader(sdr);
		return 0;
	}

	unsigned int prog = glCreateProgram();
	glAttachShader(prog, sdr);
	glLinkProgram(prog);

	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &loglength);

	if(loglength) {
		char *buf = new char[loglength + 1];
		glGetProgramInfoLog(prog, loglength, 0, buf);
		buf[loglength] = 0;

		fprintf(stderr, "linking: %s\n", buf);
		delete [] buf;
	}
	if(!status) {
		fprintf(stderr, "failed\n");
		glDeleteShader(sdr);
		glDeleteProgram(prog);
		return 0;
	}

	glUseProgram(prog);
	assert(glGetError() == GL_NO_ERROR);

	uloc.resolution = glGetUniformLocation(prog, "iResolution");
	uloc.globaltime = glGetUniformLocation(prog, "iGlobalTime");
	uloc.mouse = glGetUniformLocation(prog, "iMouse");
	uloc.date = glGetUniformLocation(prog, "iDate");
	for(int i=0; i<4; i++) {
		char buf[64];

		sprintf(buf, "iChannelTime[%d]", i);
		uloc.channeltime[i] = glGetUniformLocation(prog, buf);

		sprintf(buf, "iChannelResolution[%d]", i);
		uloc.channelres[i] = glGetUniformLocation(prog, buf);

		sprintf(buf, "iChannel%d", i);
		uloc.sampler[i] = glGetUniformLocation(prog, buf);
	}
	return prog;
}

bool load_shader_metadata(const char *sdrname)
{
	// load shader metadata
	int idx = 0;
	char *nbuf = new char[strlen(sdrname) + 6];
	sprintf(nbuf, "%s.meta", sdrname);
	printf("looking for metadata file: %s ...", nbuf);

	FILE *fp = fopen(nbuf, "rb");
	if(!fp) {
		printf("not found\n");
		delete [] nbuf;
		return false;
	}

	printf("found\n");
	char buf[512];
	while(fgets(buf, sizeof buf, fp)) {
		int id;
		if(sscanf(buf, "texture %d", &id) == 1) {
			if(id < 0) {
				activetex[idx++] = &notex;
			} else {
				if(idx < 4) {
					printf("using texture %d in slot %d\n", id, idx);
					activetex[idx++] = textures[id];
				}
			}
		}
	}
	fclose(fp);
	delete [] nbuf;
	return true;
}

Texture *load_texture(const char *fname)
{
	Texture *tex = new Texture;
	tex->id = img_gltexture_load(fname);
	if(!tex->id) {
		fprintf(stderr, "failed to load texture: %s\n", fname);
		return 0;
	}
	tex->targ = GL_TEXTURE_2D;
	tex->stype = "2D";
	return tex;
}

Texture *load_cubemap(const char *fname_fmt)
{
	char *fname = new char[strlen(fname_fmt) + 1];

	Texture *tex = new Texture;
	tex->targ = GL_TEXTURE_CUBE_MAP;
	tex->stype = "Cube";

	glGenTextures(1, &tex->id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex->id);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	assert(glGetError() == GL_NO_ERROR);

	int first_xsz = 0, first_ysz = 0;

	for(int i=0; i<6; i++) {
		int xsz, ysz;
		sprintf(fname, fname_fmt, i);
		void *pixels = img_load_pixels(fname, &xsz, &ysz, IMG_FMT_RGBAF);
		if(!pixels) {
			fprintf(stderr, "failed to load image: %s\n", fname);
			glDeleteTextures(1, &tex->id);
			delete tex;
			return 0;
		}

		if(i == 0) {
			first_xsz = xsz;
			first_ysz = ysz;
		} else {
			if(first_xsz != xsz || first_ysz != ysz) {
				fprintf(stderr, "cubemap face %s isn't %dx%d like the rest\n", fname, first_xsz, first_ysz);
				img_free_pixels(pixels);
				glDeleteTextures(1, &tex->id);
				delete tex;
				return 0;
			}
		}

		assert(glGetError() == GL_NO_ERROR);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA16F, xsz, ysz, 0, GL_RGBA, GL_FLOAT, pixels);
		assert(glGetError() == GL_NO_ERROR);
		img_free_pixels(pixels);
	}

	return tex;
}

bool parse_args(int argc, char **argv)
{
	for(int i=0; i<argc; i++) {
		if(argv[i][0] == '-') {
			switch(argv[i][1]) {
			case 't':
				if(tex_arg.size() >= 4) {
					fprintf(stderr, "too many textures specified\n");
					return false;
				} else {
					char *endp;
					int idx = strtol(argv[++i], &endp, 10);
					if(endp == argv[i]) {
						fprintf(stderr, "-t must be followed by a number\n");
						return false;
					}
					tex_arg.push_back(idx);
				}
				break;
			case 'b':
				isFullScreen = true;
				break;

			default:
				fprintf(stderr, "unrecognized option: %s\n", argv[i]);
				return false;
			}
		} else {
			if(sdrfname_arg) {
				fprintf(stderr, "too many shaders specified\n");
				return false;
			}
			sdrfname_arg = argv[++i];
			{
				// tidy shader path for title bar
				sampleNameStr = sdrfname_arg;
				{
					std::size_t found = sampleNameStr.find_last_of("/\\");
					sampleNameStr = sampleNameStr.substr(found + 1);
				}
				{
					std::size_t found = sampleNameStr.find_last_of(".");
					sampleNameStr = sampleNameStr.substr(0, found);
				}
			}
		}
	}
	return true;
}
