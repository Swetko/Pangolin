/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011 Steven Lovegrove
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <sstream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <map>
#include <cctype>

#include <pangolin/gl/glplatform.h>
#include <pangolin/gl/colour.h>
#include <pangolin/gl/opengl_render_state.h>
#include <pangolin/utils/file_utils.h>

#ifdef HAVE_GLES
    #define GLhandleARB GLuint
#endif

#if defined(HAVE_EIGEN) && !defined(__CUDACC__) //prevent including Eigen in cuda files
#define USE_EIGEN
#endif

#ifdef USE_EIGEN
#include <Eigen/Core>
#endif // USE_EIGEN

namespace pangolin
{

////////////////////////////////////////////////
// Standard attribute locations
////////////////////////////////////////////////

const GLuint DEFAULT_LOCATION_POSITION = 0;
const GLuint DEFAULT_LOCATION_COLOUR   = 1;
const GLuint DEFAULT_LOCATION_NORMAL   = 2;
const GLuint DEFAULT_LOCATION_TEXCOORD = 3;

const char DEFAULT_NAME_POSITION[] = "a_position";
const char DEFAULT_NAME_COLOUR[]   = "a_color";
const char DEFAULT_NAME_NORMAL[]   = "a_normal";
const char DEFAULT_NAME_TEXCOORD[] = "a_texcoord";

////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////

enum GlSlShaderType
{
    GlSlAnnotatedShader = 0,
    GlSlFragmentShader = GL_FRAGMENT_SHADER,
    GlSlVertexShader = GL_VERTEX_SHADER,
    GlSlGeometryShader = 0x8DD9 /*GL_GEOMETRY_SHADER*/,
    GlSlComputeShader = 0x91B9 /*GL_COMPUTE_SHADER*/
};

class GlSlProgram
{
public:
    GlSlProgram();

    //! Move Constructor
    GlSlProgram(GlSlProgram&& tex);

    ~GlSlProgram();

    bool AddShader(
        GlSlShaderType shader_type,
        const std::string& source_code,
        const std::map<std::string,std::string>& program_defines = std::map<std::string,std::string>(),
        const std::vector<std::string>& search_path = std::vector<std::string>()
    );

    bool AddShaderFromFile(
        GlSlShaderType shader_type,
        const std::string& filename,
        const std::map<std::string,std::string>& program_defines = std::map<std::string,std::string>(),
        const std::vector<std::string>& search_path = std::vector<std::string>()
    );

    bool Link();

    // Remove all shaders from this program, and reload from files.
    bool ReloadShaderFiles();

    GLint GetAttributeHandle(const std::string& name);
    GLint GetUniformHandle(const std::string& name);

    // Before setting uniforms, be sure to Bind() the GlSl program first.
    void SetUniform(const std::string& name, int x);
    void SetUniform(const std::string& name, int x1, int x2);
    void SetUniform(const std::string& name, int x1, int x2, int x3);
    void SetUniform(const std::string& name, int x1, int x2, int x3, int x4);

    void SetUniform(const std::string& name, float f);
    void SetUniform(const std::string& name, float f1, float f2);
    void SetUniform(const std::string& name, float f1, float f2, float f3);
    void SetUniform(const std::string& name, float f1, float f2, float f3, float f4);

    void SetUniform(const std::string& name, Colour c);

    void SetUniform(const std::string& name, const OpenGlMatrix& m);

#ifdef USE_EIGEN
    void SetUniform(const std::string& name, const Eigen::Vector2f& v);
    void SetUniform(const std::string& name, const Eigen::Vector3f& v);
    void SetUniform(const std::string& name, const Eigen::Vector4f& v);
    void SetUniform(const std::string& name, const Eigen::Matrix2f& m);
    void SetUniform(const std::string& name, const Eigen::Matrix3f& m);
    void SetUniform(const std::string& name, const Eigen::Matrix4f& m);

    void SetUniform(const std::string& name, const Eigen::Vector2d& v);
    void SetUniform(const std::string& name, const Eigen::Vector3d& v);
    void SetUniform(const std::string& name, const Eigen::Vector4d& v);
    void SetUniform(const std::string& name, const Eigen::Matrix2d& m);
    void SetUniform(const std::string& name, const Eigen::Matrix3d& m);
    void SetUniform(const std::string& name, const Eigen::Matrix4d& m);
#endif

#if GL_VERSION_4_3
    GLint GetProgramResourceIndex(const std::string& name);
    void SetShaderStorageBlock(const std::string& name, const int& bindingIndex);
#endif

    void Bind();
    void SaveBind();
    void Unbind();


    void BindPangolinDefaultAttribLocationsAndLink();

    // Unlink all shaders from program
    void ClearShaders();

    GLint ProgramId() const {
        return prog;
    }

    bool Valid() const {
        return ProgramId() != 0;
    }

protected:
    struct ShaderFileOrCode
    {
        GlSlShaderType shader_type;
        std::string filename;
        std::string code;
        std::map<std::string,std::string> program_defines;
        std::vector<std::string> search_path;
    };


    // Convenience method to load shader description
    bool AddShaderFile(const ShaderFileOrCode &shader_file);

    std::string ParseIncludeFilename(
        const std::string& location
    );

    std::string SearchIncludePath(
        const std::string& filename,
        const std::vector<std::string>& search_path,
        const std::string& current_path
    );

    bool AddPreprocessedShader(
        GlSlShaderType shader_type,
        const std::string& source_code,
        const std::string& name_for_errors
    );

    void PreprocessGLSL(
        std::istream& input,
        std::ostream& output,
        const std::map<std::string,std::string>& program_defines,
        const std::vector<std::string>& search_path,
        const std::string& current_path
    );

    // Split 'code' into several code blocks per shader type
    // shader blocks in 'code' must be annotated with:
    // @start vertex, @start fragment, @start geometry or @start compute
    static std::map<GlSlShaderType,std::string>
    SplitAnnotatedShaders(const std::string& code);

    bool linked;
    std::vector<GLhandleARB> shaders;
    GLenum prog;
    GLint prev_prog;
    std::vector<ShaderFileOrCode> shader_files;
};

class GlSlUtilities
{
public:
    inline static GlSlProgram& OffsetAndScale(float offset, float scale) {
        GlSlProgram& prog = Instance().prog_offsetscale;
        prog.Bind();
        prog.SetUniform("offset", offset);
        prog.SetUniform("scale",  scale);
        return prog;
    }

    inline static GlSlProgram& OffsetAndScaleGamma(float offset, float scale) {
        GlSlProgram& prog = Instance().prog_offsetscalegamma;
        prog.Bind();
        prog.SetUniform("offset", offset);
        prog.SetUniform("scale",  scale);
        return prog;
    }

    inline static GlSlProgram& Scale(float scale, float bias = 0.0f) {
        GlSlProgram& prog = Instance().prog_scale;
        prog.Bind();
        prog.SetUniform("scale", scale);
        prog.SetUniform("bias",  bias);
        return prog;
    }

    inline static void UseNone()
    {
        glUseProgram(0);
    }

protected:
    static GlSlUtilities& Instance() {
        // TODO: BUG: The GlSLUtilities instance needs to be tied
        // to the OpenGL context, not the thread, or globally.
#ifndef PANGO_NO_THREADLOCAL
        thread_local
#else
        static
#endif
        GlSlUtilities instance;
        return instance;
    }

    // protected constructor
    GlSlUtilities() {
        const char* source_scale =
                "uniform float scale;"
                "uniform float bias;"
                "uniform sampler2D tex;"
                "void main() {"
                "  vec2 uv = gl_TexCoord[0].st;"
                "  if(0.0 <= uv.x && uv.x <= 1.0 && 0.0 <= uv.y && uv.y <= 1.0) {"
                "    gl_FragColor = texture2D(tex,uv);"
                "    gl_FragColor.xyz *= scale;"
                "    gl_FragColor.xyz += vec3(bias,bias,bias);"
                "  }else{"
                "    float v = 0.1;"
                "    gl_FragColor.xyz = vec3(v,v,v);"
                "  }"
                "}";
        prog_scale.AddShader(GlSlFragmentShader, source_scale);
        prog_scale.Link();

        // shader performs automatically gamma correction, assuming that image data is linear
        // maps to (approximate) sRGB
        const char* source_offsetscalegamma =
                "uniform float offset;"
                "uniform float scale;"
                "uniform sampler2D tex;"
                "void main() {"
                "  vec2 uv = gl_TexCoord[0].st;"
                "  if(0.0 <= uv.x && uv.x <= 1.0 && 0.0 <= uv.y && uv.y <= 1.0) {"
                "    gl_FragColor = texture2D(tex,gl_TexCoord[0].st);"
                "    gl_FragColor.xyz += vec3(offset,offset,offset);"
                "    gl_FragColor.xyz *= scale;"
                "    gl_FragColor.xyz = pow(gl_FragColor.xyz,vec3(0.45,0.45,0.45));"
                "  }else{"
                "    float v = 0.1;"
                "    gl_FragColor.xyz = vec3(v,v,v);"
                "  }"
                "}";
        prog_offsetscalegamma.AddShader(GlSlFragmentShader, source_offsetscalegamma);
        prog_offsetscalegamma.Link();

         const char* source_offsetscale =
                "uniform float offset;"
                "uniform float scale;"
                "uniform sampler2D tex;"
                "void main() {"
                "  vec2 uv = gl_TexCoord[0].st;"
                "  if(0.0 <= uv.x && uv.x <= 1.0 && 0.0 <= uv.y && uv.y <= 1.0) {"
                "    gl_FragColor = texture2D(tex,gl_TexCoord[0].st);"
                "    gl_FragColor.xyz += vec3(offset,offset,offset);"
                "    gl_FragColor.xyz *= scale;"
                "  }else{"
                "    float v = 0.1;"
                "    gl_FragColor.xyz = vec3(v,v,v);"
                "  }"
                "}";
        prog_offsetscale.AddShader(GlSlFragmentShader, source_offsetscale);
        prog_offsetscale.Link();
    }

    GlSlProgram prog_scale;
    GlSlProgram prog_offsetscale;
    GlSlProgram prog_offsetscalegamma;
};


}
