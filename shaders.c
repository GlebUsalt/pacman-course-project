#include "shaders.h"
#include <stdio.h>

GLuint shaderProg = 0;
GLint uView, uOffset, uScale, uAngle, uColor, uMode, uParams, uTime, uAlpha;

static const char* VERT_SRC =
    "#version 330 core\n"
    "layout(location = 0) in vec2 aPos;\n"
    "uniform vec2  uView;\n"       /* logical window size, replaces glOrtho   */
    "uniform vec2  uOffset;\n"     /* replaces glTranslatef                   */
    "uniform vec2  uScale;\n"      /* replaces glScalef                       */
    "uniform float uAngle;\n"      /* radians, replaces glRotatef             */
    "out vec2 vLocal;\n"
    "void main() {\n"
    "    vLocal = aPos;\n"
    "    float c = cos(uAngle), s = sin(uAngle);\n"
    "    vec2 p = aPos * uScale;\n"
    "    p = vec2(c * p.x - s * p.y, s * p.x + c * p.y) + uOffset;\n"
    "    gl_Position = vec4(p / uView * 2.0 - 1.0, 0.0, 1.0);\n"
    "}\n";

static const char* FRAG_SRC =
    "#version 330 core\n"
    "in vec2 vLocal;\n"
    "uniform vec3  uColor;\n"
    "uniform int   uMode;\n"
    "uniform vec4  uParams;\n"
    "uniform float uTime;\n"
    "uniform float uAlpha;\n"
    "out vec4 FragColor;\n"
    "const float aa = 0.75;\n"
    "float segDist(vec2 p, vec2 a, vec2 b) {\n"
    "    vec2 pa = p - a, ba = b - a;\n"
    "    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);\n"
    "    return length(pa - ba * h);\n"
    "}\n"
    "vec4 shade() {\n"
    "    if (uMode == 0) return vec4(uColor, 1.0);\n"
    "    if (uMode == 5) {\n"
    "        vec2 q = vLocal * vec2(uParams.w, uParams.y);\n"
    "        float d = length(vec2(max(abs(q.x) - uParams.x, 0.0), q.y)) - uParams.z;\n"
    "        return vec4(uColor, 1.0 - smoothstep(-aa, aa, d));\n"
    "    }\n"
    "    float pad = uParams.y;\n"
    "    vec2 p = vLocal * pad;\n"
    "    if (uMode == 4) {\n"
    "        const float h = 11.0;\n"
    "        int m = int(uParams.x + 0.5);\n"
    "        float d = 1000.0;\n"
    "        if ((m & 1) == 0) d = min(d, segDist(p, vec2(-h,  h), vec2( h,  h)));\n"
    "        if ((m & 2) == 0) d = min(d, segDist(p, vec2(-h, -h), vec2( h, -h)));\n"
    "        if ((m & 4) == 0) d = min(d, segDist(p, vec2( h, -h), vec2( h,  h)));\n"
    "        if ((m & 8) == 0) d = min(d, segDist(p, vec2(-h, -h), vec2(-h,  h)));\n"
    "        float flicker = 0.92 + 0.08 * sin(uTime * 2.0);\n"
    "        float core = 1.0 - smoothstep(0.8, 1.7, d);\n"
    "        float glow = 0.6 * flicker * exp(-d * 0.5);\n"
    "        return vec4(mix(uColor, vec3(1.0), core * 0.75), max(core, glow));\n"
    "    }\n"
    "    if (uMode == 3) {\n"
    "        float yb = -7.0 + 1.5 * sin(p.x * 1.05 + uTime * 8.0);\n"
    "        float d = (p.y >= 0.0) ? length(p) - 9.0\n"
    "                               : max(abs(p.x) - 9.0, yb - p.y);\n"
    "        return vec4(uColor, 1.0 - smoothstep(-aa, aa, d));\n"
    "    }\n"
    "    float r = uParams.x;\n"
    "    float dist = length(p);\n"
    "    float core = 1.0 - smoothstep(r - aa, r + aa, dist);\n"
    "    if (uMode == 2) {\n"
    "        float m = uParams.w;\n"
    "        vec2 q = vec2(p.x, abs(p.y));\n"
    "        core *= smoothstep(-aa, aa, dot(q, vec2(-sin(m), cos(m))));\n"
    "    }\n"
    "    float halo = 0.0;\n"
    "    if (uParams.z > 0.0 && pad > r && dist > r) {\n"
    "        float t = clamp((dist - r) / (pad - r), 0.0, 1.0);\n"
    "        halo = uParams.z * (1.0 - t) * (1.0 - t);\n"
    "    }\n"
    "    return vec4(uColor, max(core, halo));\n"
    "}\n"
    "void main() {\n"
    "    vec4 c = shade();\n"
    "    FragColor = vec4(c.rgb, c.a * uAlpha);\n"
    "}\n";

//Компилирует один шейдер
static GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(s, sizeof(log), NULL, log);
        fprintf(stderr, "Shader compile error (%s):\n%s\n",
                type == GL_VERTEX_SHADER ? "vertex" : "fragment", log);
    }
    return s;
}

//Собирает вершинный и фрагментный шейдеры в одну программу
void buildProgram(void) {
    GLuint vs = compileShader(GL_VERTEX_SHADER, VERT_SRC);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, FRAG_SRC);
    shaderProg = glCreateProgram();
    glAttachShader(shaderProg, vs);
    glAttachShader(shaderProg, fs);
    glLinkProgram(shaderProg);
    GLint ok = 0;
    glGetProgramiv(shaderProg, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(shaderProg, sizeof(log), NULL, log);
        fprintf(stderr, "Shader link error:\n%s\n", log);
    }
    glDeleteShader(vs);      
    glDeleteShader(fs);

    //запоминаем адреса переменных шейдера
    uView   = glGetUniformLocation(shaderProg, "uView");
    uOffset = glGetUniformLocation(shaderProg, "uOffset");
    uScale  = glGetUniformLocation(shaderProg, "uScale");
    uAngle  = glGetUniformLocation(shaderProg, "uAngle");
    uColor  = glGetUniformLocation(shaderProg, "uColor");
    uMode   = glGetUniformLocation(shaderProg, "uMode");
    uParams = glGetUniformLocation(shaderProg, "uParams");
    uTime   = glGetUniformLocation(shaderProg, "uTime");
    uAlpha  = glGetUniformLocation(shaderProg, "uAlpha");
}

void destroyProgram(void) {
    if (shaderProg) glDeleteProgram(shaderProg);
    shaderProg = 0;
}
