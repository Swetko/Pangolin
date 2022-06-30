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

#include <pangolin/display/view.h>
#include <pangolin/display/display.h>
//#include <pangolin/display/pangolin_gl.h>
#include <pangolin/var/var.h>
#include <pangolin/handler/handler.h>
#include <pangolin/gl/glfont.h>

#include <functional>

namespace pangolin
{

struct WidgetColorScheme
  {
  GLfloat s1[4] = {0.2f, 0.2f, 0.2f, 1.0f};
  GLfloat s2[4] = {0.6f, 0.6f, 0.6f, 1.0f};
  GLfloat s3[4] = {0.8f, 0.8f, 0.8f, 1.0f};
  GLfloat bg[4] = {0.9f, 0.9f, 0.9f, 1.0f};
  GLfloat fg[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  GLfloat tx[4] = {0.0f, 0.0f, 0.0f, 1.0f};
  GLfloat dn[4] = {1.0f, 0.7f, 0.7f, 1.0f};
  GLfloat dm[4] = {1.0f, 0.8f, 0.8f, 1.0f};
  };

inline const WidgetColorScheme default_color_scheme;

PANGOLIN_EXPORT
View& CreatePanel(const std::string& name);

PANGOLIN_EXPORT
void DrawTextWindowCoords(GlText& text, GLfloat x, GLfloat y, GLfloat z = 0.0);


PANGOLIN_EXPORT void glLine(GLfloat vs[4]);
PANGOLIN_EXPORT void glRect(const Viewport& v);
PANGOLIN_EXPORT void glRect(const Viewport& v, int inset);

PANGOLIN_EXPORT void DrawShadowRect(const Viewport& v);
PANGOLIN_EXPORT void DrawShadowRect(const Viewport& v, bool pushed);

// RAII class to avoid code duplication
struct PANGOLIN_EXPORT PrepareForWidgetDrawing
{
    PrepareForWidgetDrawing();
    ~PrepareForWidgetDrawing();
};


struct PANGOLIN_EXPORT Panel : public View, Handler
{
    //Panel();
    Panel(const std::string& auto_register_var_prefix);
    void Render();
    void ResizeChildren();
    
    std::pair<int,int> GetMinimumSize() const;
    
protected:
    sigslot::scoped_connection var_added_connection;
    std::string auto_register_var_prefix;
    
    void NewVarCallback(const VarState::Event& e);
    void AddVariable(const std::string& name, const std::shared_ptr<VarValueGeneric> &var);
    void RemoveVariable(const std::string& name);
    
    // Visuals
    static constexpr int panel_v_margin = 6;
    static constexpr int panel_scroll_rate = 32;
    
    // Handler
    void Mouse(View&, MouseButton button, int x, int y, bool pressed, int button_state);
    void Special(View&, InputSpecial inType, float x, float y, float p1, float p2, float p3, float p4, int button_state);
    
    float actual_scroll_offset=0;
    int target_scroll_offset=0;
    int total_children_height=0;
};

template<typename T>
struct Widget : public View, Handler, Var<T>
{
    Widget(std::string title, const std::shared_ptr<VarValueGeneric>& tv)
        : Var<T>(tv), title(title)
    {
        handler = this;
    }
    
    std::pair<int,int> GetMinimumSize() const
    {
        PANGO_ASSERT(bottom.unit==Unit::ReversePixel && "Widget needs to override GetMinimumSize.");
        return {-1,std::abs((int)bottom.p)};
    }
    
    std::string title;
};

struct PANGOLIN_EXPORT Button : public Widget<bool>
{
    Button(std::string title, const std::shared_ptr<VarValueGeneric> &tv);
    void Mouse(View&, MouseButton button, int x, int y, bool pressed, int mouse_state);
    void Render();
    
    //Cache params on resize
    void ResizeChildren();
    GlText gltext;
    GLfloat raster[2];
    bool down;
};

struct PANGOLIN_EXPORT FunctionButton : public Widget<std::function<void(void)> >
{
    FunctionButton(std::string title, const std::shared_ptr<VarValueGeneric> &tv);
    void Mouse(View&, MouseButton button, int x, int y, bool pressed, int mouse_state);
    void Render();

    //Cache params on resize
    void ResizeChildren();
    GlText gltext;
    GLfloat raster[2];
    bool down;
};

struct PANGOLIN_EXPORT Checkbox : public Widget<bool>
{
    Checkbox(std::string title, const std::shared_ptr<VarValueGeneric>& tv);
    void Mouse(View&, MouseButton button, int x, int y, bool pressed, int mouse_state);
    void Render();
    
    //Cache params on resize
    void ResizeChildren();
    GlText gltext;
    GLfloat raster[2];
    Viewport vcb;
};

struct PANGOLIN_EXPORT Slider : public Widget<double>
{
    Slider(std::string title, const std::shared_ptr<VarValueGeneric>& tv);
    void Mouse(View&, MouseButton button, int x, int y, bool pressed, int mouse_state);
    void MouseMotion(View&, int x, int y, int mouse_state);
    void Keyboard(View&, unsigned char key, int x, int y, bool pressed);
    void Render();
    
    //Cache params on resize
    void ResizeChildren();
    GlText gltext;
    GLfloat raster[2];
    bool lock_bounds;
    bool logscale;
    bool is_integral_type;
};

struct PANGOLIN_EXPORT TextInput : public Widget<std::string>
{
    TextInput(std::string title, const std::shared_ptr<VarValueGeneric>& tv);
    void Mouse(View&, MouseButton button, int x, int y, bool pressed, int mouse_state);
    void MouseMotion(View&, int x, int y, int mouse_state);
    void Keyboard(View&, unsigned char key, int x, int y, bool pressed);
    void Render();
    
    std::string edit;
    GlText gledit;

    //Cache params on resize
    void ResizeChildren();
    void CalcVisibleEditPart();
    GlText gltext;
    bool can_edit;
    bool do_edit;
    int sel[2];
    GLfloat vertical_margin;
    GLfloat horizontal_margin = 2.f;
    int input_width;
    int edit_visible_part[2] = {0,1};
};


inline bool GuiVarHasChanged() {
    Var<bool> changed("pango.widgets.gui_changed");
    return bool(changed) && !(changed = false);
}

inline void FlagVarChanged() {
    Var<bool>("pango.widgets.gui_changed") = true;
}

}
