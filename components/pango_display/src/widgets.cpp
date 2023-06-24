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

#include <thread>
#include <mutex>
#include <iostream>
#include <iomanip>

#include <pangolin/display/widgets.h>
#include <pangolin/display/display.h>
#include <pangolin/display/default_font.h>
#include <pangolin/gl/gldraw.h>
#include <pangolin/var/varextra.h>
#include <pangolin/utils/file_utils.h>

#include "pangolin_gl.h"


using namespace std;

namespace pangolin
{
static const auto& colour=default_color_scheme;


// TODO: It doesn't look like this is doing anything meaningful right now...
std::mutex display_mutex;

// Render at (x,y) in window coordinates.
void DrawTextWindowCoords(const GlText& text, GLfloat x, GLfloat y, GLfloat z)
{
    // Backup viewport
    GLint    view[4];
    glGetIntegerv(GL_VIEWPORT, view );

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    DisplayBase().ActivatePixelOrthographic();

    glTranslatef( std::floor(x), std::floor(y), z);
    text.Draw();

    // Restore viewport
    glViewport(view[0],view[1],view[2],view[3]);

    // Restore modelview / project matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

static inline int cb_height()
{
    return (int)(default_font().Height() * 1.0);
}

static inline int tab_h()
{
    return (int)(default_font().Height() * 1.4);
}

static inline float x_width()
{
  return default_font().Text("x").Width();
}

template<typename T>
inline void MarkVarChangedByGui(VarValueT<T>& var)
{
    var.Meta().gui_changed = true;
    FlagVarChanged();
}

void glLine(GLfloat vs[4])
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vs);
    glDrawArrays( GL_LINE_STRIP, 0, 2);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void glRect(const Viewport& v)
{
    GLfloat vs[] = { (float)v.l  -0.5f,(float)v.b  -0.5f,
                     (float)v.l  -0.5f,(float)v.t()-0.5f,
                     (float)v.r()-0.5f,(float)v.t()-0.5f,
                     (float)v.r()-0.5f,(float)v.b  -0.5f};

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vs);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void glRect(const Viewport& v, int inset)
{
    glRect(v.Inset(inset));
}

void DrawShadowRect(const Viewport& v)
{
    glColor4fv(colour.s2);
    glDrawRectPerimeter((GLfloat)v.l, (GLfloat)v.b, (GLfloat)(v.r()-1), (GLfloat)(v.t()-1));
}

void DrawShadowRect(const Viewport& v, bool pushed)
{
    const GLfloat* c1 = pushed ? colour.s1 : colour.s2;
    const GLfloat* c2 = pushed ? colour.s2 : colour.s1;

    GLfloat vs[] = { (float)v.l,(float)v.b,
                     (float)v.l,(float)(v.t()-1),
                     (float)(v.r()-1),(float)(v.t()-1),
                     (float)(v.r()-1),(float)v.b,
                     (float)v.l,(float)v.b };

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vs);
    glColor4fv(c1);
    glDrawArrays(GL_LINE_STRIP, 0, 3);

    glColor4fv(c2);
    glDrawArrays(GL_LINE_STRIP, 2, 3);
    glDisableClientState(GL_VERTEX_ARRAY);

}

Panel::Panel(const std::string& auto_register_var_prefix)
    : auto_register_var_prefix(auto_register_var_prefix)
{
    handler = this;
    layout = LayoutVertical;

    // Register for notifications on var additions
    if(auto_register_var_prefix!="")
        var_added_connection = VarState::I().RegisterForVarEvents(
            std::bind(&Panel::NewVarCallback,this,std::placeholders::_1),
            true
        );
}

void Panel::NewVarCallback(const VarState::Event& e)
{
    const std::string name = e.var->Meta().full_name;
    if(!StartsWith(name, auto_register_var_prefix))
        return;

    switch(e.action) {
    case VarState::Event::Action::Added:
        AddVariable(name, e.var);
        break;
    case VarState::Event::Action::Removed:
        RemoveVariable(name);
        break;
    }
}

void Panel::AddVariable(const std::string& name, const std::shared_ptr<VarValueGeneric>& var)
{
    const string& title = var->Meta().friendly;
    
    display_mutex.lock();
    
    ViewMap::iterator pnl = GetCurrentContext()->named_managed_views.find(name);
    
    // Only add if a widget by the same name doesn't already exist
    if( pnl == GetCurrentContext()->named_managed_views.end() )
    {
        View* nv = NULL;
        if( !strcmp(var->TypeId(), typeid(bool).name()) ) {
            nv = (var->Meta().flags & META_FLAG_TOGGLE) ? (View*)new Checkbox(title,var) : (View*)new Button(title,var);
        } else if (!strcmp(var->TypeId(), typeid(double).name()) ||
                   !strcmp(var->TypeId(), typeid(float).name()) ||
                   !strcmp(var->TypeId(), typeid(int8_t).name()) ||
                   !strcmp(var->TypeId(), typeid(uint8_t).name()) ||
                   !strcmp(var->TypeId(), typeid(int16_t).name()) ||
                   !strcmp(var->TypeId(), typeid(uint16_t).name()) ||
                   !strcmp(var->TypeId(), typeid(int32_t).name()) ||
                   !strcmp(var->TypeId(), typeid(uint32_t).name()) ||
                   !strcmp(var->TypeId(), typeid(int64_t).name()) ||
                   !strcmp(var->TypeId(), typeid(uint64_t).name())
                   )
        {
            nv = new Slider(title, var);
        } else if (!strcmp(var->TypeId(), typeid(std::function<void(void)>).name() ) ) {
            nv = (View*)new FunctionButton(title, var);
        }else if(var->str){
            nv = new TextInput(title,var);
        }
        if(nv) {
            GetCurrentContext()->named_managed_views[name] = nv;
            views.push_back( nv );
            var->Meta().associated_widget=nv;
            ResizeChildren();
        }
    }

    display_mutex.unlock();
}

void Panel::RemoveVariable(const std::string& name)
{
    display_mutex.lock();

    ViewMap::iterator pnl = GetCurrentContext()->named_managed_views.find(name);

    if( pnl != GetCurrentContext()->named_managed_views.end() ) {
      views.erase(std::remove(views.begin(), views.end(), pnl->second), views.end());
      ResizeChildren();

      delete pnl->second;
      GetCurrentContext()->named_managed_views.erase(pnl);
    }

    display_mutex.unlock();
}

PrepareForWidgetDrawing::PrepareForWidgetDrawing()
{
#ifndef HAVE_GLES
    glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_SCISSOR_BIT | GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
#endif
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    DisplayBase().ActivatePixelOrthographic();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    //glDisable(GL_SCISSOR_TEST);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_COLOR_MATERIAL);
}


PrepareForWidgetDrawing::~PrepareForWidgetDrawing()
{
#ifndef HAVE_GLES
    glPopAttrib();
#else
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_DEPTH_TEST);
#endif
}

void Panel::Render()
{
    if(actual_scroll_offset != (float)target_scroll_offset)
    {
        actual_scroll_offset += (target_scroll_offset - actual_scroll_offset)*0.15;
        ResizeChildren();
        if(std::abs(actual_scroll_offset - (float)target_scroll_offset) < 0.5f)
            actual_scroll_offset = target_scroll_offset;
    }
    
    PrepareForWidgetDrawing raii;
    
    glLineWidth(1.0);

    glColor4fv(colour.bg);
    glRect(v);
    DrawShadowRect(v.Inset(0));
    
    (v.Inset(panel_v_margin)).Scissor();
    RenderChildren();
}

std::pair<int,int> Panel::GetMinimumSize() const
  {
  int total_height=0;
  for(auto& iv : views) total_height += iv->GetMinimumSize().second + panel_v_margin;
  if(views.size())total_height -= panel_v_margin;
  total_height += 2*panel_v_margin;
  return {-1,total_height};
  }


void Panel::ResizeChildren()
{
    // Allocate space incrementally
    Viewport space = v.Inset(panel_v_margin);
    space.h+=(int)std::round(actual_scroll_offset);
    total_children_height=0;
    
    for(auto& iv : views)
    {
        if (!iv->show) continue;
        int child_height=iv->GetMinimumSize().second;
        Viewport exact=space;
        exact.b=space.b+space.h-child_height;
        exact.h=child_height;
        iv->Resize(exact);
        
        space.h = iv->v.b - panel_v_margin - space.b;
        total_children_height += iv->v.h + panel_v_margin;
    }
    
    if(views.size())total_children_height -= panel_v_margin;
}

void Panel::Mouse(View& d, MouseButton button, int x, int y, bool pressed, int button_state)
{
    if( pressed && (button == MouseWheelUp || button == MouseWheelDown) )
    {
        if( button == MouseWheelUp) target_scroll_offset   -= panel_scroll_rate;
        if( button == MouseWheelDown) target_scroll_offset += panel_scroll_rate;
        target_scroll_offset = std::max(0, std::min(target_scroll_offset, total_children_height - v.h + 2*panel_v_margin ));
        ResizeChildren();
    }else{
        Handler::Mouse(d,button,x,y,pressed,button_state);
    }
}

void Panel::Special(View& d, InputSpecial inType, float x, float y, float p1, float p2, float p3, float p4, int button_state)
{
    if( inType == InputSpecialScroll )
    {
        target_scroll_offset -= panel_scroll_rate*((int)(p2 / fabs(p2)));
        target_scroll_offset = std::max(0, std::min(target_scroll_offset, total_children_height - v.h + 2*panel_v_margin ));
        d.ResizeChildren();
    }else{
        Handler::Special(d,inType,x,y,p1,p2,p3,p4,button_state);
    }
}

View& CreatePanel(const std::string& name)
{
    if(GetCurrentContext()->named_managed_views.find(name) != GetCurrentContext()->named_managed_views.end()) {
        throw std::runtime_error("Panel already registered with this name.");
    }
    Panel * p = new Panel(name);
    GetCurrentContext()->named_managed_views[name] = p;
    GetCurrentContext()->base.views.push_back(p);
    return *p;
}

Button::Button(string title, const std::shared_ptr<VarValueGeneric>& tv)
    : Widget<bool>(title,tv), down(false)
{
    top = 1.0; bottom = Attach::Pix(-tab_h());
    left = 0.0; right = 1.0;
    hlock = LockLeft;
    vlock = LockBottom;
    gltext = default_font().Text(title);
}

void Button::Mouse(View&, MouseButton button, int /*x*/, int /*y*/, bool pressed, int /*mouse_state*/)
{
    if(button == MouseButtonLeft )
    {
        down = pressed;
        if( !pressed ) {
            var->Set(!var->Get());
            MarkVarChangedByGui(*var);
        }
    }
}

void Button::Render()
{
    glColor4fv(colour.fg );
    glRect(v);
    glColor4fv(colour.tx);
    if(gltext.Text() != var->Meta().friendly) {
        gltext = default_font().Text(var->Meta().friendly);
    }
    DrawTextWindowCoords(gltext, raster[0],raster[1]-down);
    DrawShadowRect(v, down);
}

void Button::ResizeChildren()
{
    raster[0] = floor(v.l + (v.w-gltext.Width())/2.0f);
    raster[1] = floor(v.b + (v.h-gltext.Height())/2.0f);
}

FunctionButton::FunctionButton(string title, const std::shared_ptr<VarValueGeneric> &tv)
    : Widget<std::function<void(void)> >(title, tv), down(false)
{
    top = 1.0; bottom = Attach::Pix(-tab_h());
    left = 0.0; right = 1.0;
    hlock = LockLeft;
    vlock = LockBottom;
    gltext = default_font().Text(title);
}

void FunctionButton::Mouse(View&, MouseButton button, int /*x*/, int /*y*/, bool pressed, int /*mouse_state*/)
{
    if (button == MouseButtonLeft)
    {
        down = pressed;
        if (!pressed) {
            var->Get()();
            MarkVarChangedByGui(*var);
        }
    }
}

void FunctionButton::Render()
{
    glColor4fv(colour.fg);
    glRect(v);
    glColor4fv(colour.tx);
    if(gltext.Text() != var->Meta().friendly) {
        gltext = default_font().Text(var->Meta().friendly);
    }
    DrawTextWindowCoords(gltext, raster[0],raster[1]-down);
    DrawShadowRect(v, down);
}

void FunctionButton::ResizeChildren()
{
    raster[0] = v.l + (v.w - gltext.Width()) / 2.0f;
    raster[1] = v.b + (v.h - gltext.Height()) / 2.0f;
}

Checkbox::Checkbox(std::string title, const std::shared_ptr<VarValueGeneric> &tv)
    : Widget<bool>(title,tv)
{
    top = 1.0; bottom = Attach::Pix(-tab_h());
    left = 0.0; right = 1.0;
    hlock = LockLeft;
    vlock = LockBottom;
    handler = this;
    gltext = default_font().Text(title);
}

void Checkbox::Mouse(View&, MouseButton button, int /*x*/, int /*y*/, bool pressed, int /*mouse_state*/)
{
    if( button == MouseButtonLeft && pressed ) {
        var->Set(!var->Get());
        MarkVarChangedByGui(*var);
    }
}

void Checkbox::ResizeChildren()
{
    raster[0] = v.l + cb_height() + 4.0f;
    raster[1] = v.b + (v.h-gltext.Height())/2.0f;
    const int h = v.h;
    const int t = (int)((h-cb_height()) / 2.0f);
    vcb = Viewport(v.l,v.b+t,cb_height(),cb_height());
}

void Checkbox::Render()
{
    const bool val = var->Get();

    if( val )
    {
        glColor4fv(colour.dn);
        glRect(vcb);
    }
    glColor4fv(colour.tx);
    if(gltext.Text() != var->Meta().friendly) {
        gltext = default_font().Text(var->Meta().friendly);
    }
    DrawTextWindowCoords(gltext, raster[0], raster[1]);
    DrawShadowRect(vcb, val);
}

inline bool IsIntegral(const char* typeidname)
{
    // TODO: There must be a better way of doing this...
    return !strcmp(typeidname, typeid(char).name()) ||
           !strcmp(typeidname, typeid(short).name()) ||
           !strcmp(typeidname, typeid(int).name())  ||
           !strcmp(typeidname, typeid(long).name()) ||
           !strcmp(typeidname, typeid(unsigned char).name()) ||
           !strcmp(typeidname, typeid(unsigned short).name()) ||
           !strcmp(typeidname, typeid(unsigned int).name()) ||
           !strcmp(typeidname, typeid(unsigned long).name());
}

Slider::Slider(std::string title, const std::shared_ptr<VarValueGeneric> &tv)
    : Widget<double>(title+":", tv), lock_bounds(true)
{
    top = 1.0; bottom = Attach::Pix(-tab_h());
    left = 0.0; right = 1.0;
    hlock = LockLeft;
    vlock = LockBottom;
    handler = this;
    logscale = (int)tv->Meta().logscale;
    gltext = default_font().Text(title);
    is_integral_type = IsIntegral(tv->TypeId());
}

void Slider::Keyboard(View&, unsigned char key, int /*x*/, int /*y*/, bool pressed)
{
    if(!active)return;
    if( pressed && var->Meta().range[0] < var->Meta().range[1] )
    {
        double val = !logscale ? var->Get() : log(var->Get());

        if(key=='-' || key=='_' || key=='=' || key=='+') {
            double inc = var->Meta().increment;
            if (key == '-') inc *= -1.0;
            if (key == '_') inc *= -0.1;
            if (key == '+') inc *=  0.1;
            const double newval = max(var->Meta().range[0], min(var->Meta().range[1], val + inc));
            var->Set( logscale ? exp(newval) : newval );
            MarkVarChangedByGui(*var);
        }else if(key == 'r'){
            Reset();
            MarkVarChangedByGui(*var);
        }else{
            return;
        }
    }
}

void Slider::Mouse(View& view, MouseButton button, int x, int y, bool pressed, int mouse_state)
{
    if(!active)return;
    if(pressed)
    {
        // Wheel
        if( button == MouseWheelUp || button == MouseWheelDown )
        {
            // Change scale around current value
            const double frac = max(0.0,min(1.0,(double)(x - v.l)/(double)v.w));
            double val = frac * (var->Meta().range[1] - var->Meta().range[0]) + var->Meta().range[0];

            if (logscale)
            {
                if (val<=0)
                    val = std::numeric_limits<double>::min();
                else
                    val = log(val);
            }

            const double scale = (button == MouseWheelUp ? 1.2 : 1.0 / 1.2 );
            var->Meta().range[1] = val + (var->Meta().range[1] - val)*scale;
            var->Meta().range[0] = val - (val - var->Meta().range[0])*scale;
        }else{
            lock_bounds = (button == MouseButtonLeft);
            MouseMotion(view,x,y,mouse_state);
        }
    }else{
        if(!lock_bounds)
        {
            double val = !logscale ? var->Get() : log(var->Get());

            var->Meta().range[0] = min(var->Meta().range[0], val);
            var->Meta().range[1] = max(var->Meta().range[1], val);
        }
    }
}

void Slider::MouseMotion(View&, int x, int /*y*/, int /*mouse_state*/)
{
    if(!active)return;
    if( var->Meta().range[0] != var->Meta().range[1] )
    {
        const double range = (var->Meta().range[1] - var->Meta().range[0]);
        const double frac = (double)(x - v.l)/(double)v.w;
        double val;

        if( lock_bounds )
        {
            const double bfrac = max(0.0,min(1.0,frac));
            val = bfrac * range + var->Meta().range[0] ;
        }else{
            val = frac * range + var->Meta().range[0];
        }

        if (logscale) {
            val = exp(val);
        }

        if( is_integral_type ) {
            val = std::round(val);
        }

        var->Set(val);
        MarkVarChangedByGui(*var);
    }
}


void Slider::ResizeChildren()
{
    raster[0] = v.l + 2.0f;
    raster[1] = v.b + (v.h-default_font().Height())/2.0f;
}

void Slider::Render()
{
    PrepareForWidgetDrawing raii;
    const double val = var->Get();
    ResizeChildren();
    
    if( var->Meta().range[0] != var->Meta().range[1] )
    {
        double rval = val;
        if (logscale)
        {
            rval = log(val);
        }
        
        glColor4fv(active ? colour.fg : colour.bg);
        glRect(v);
        glColor4fv(active ? colour.dn : colour.s3);
        
        const double norm_val = max(0.0,min(1.0,(rval - var->Meta().range[0]) / (var->Meta().range[1] - var->Meta().range[0])));
        glRect(Viewport(v.l,v.b, (int)(v.w*norm_val),v.h));
        DrawShadowRect(v);
    }

    glColor4fv(colour.tx);
    if(gltext.Text() != var->Meta().friendly) {
        gltext = default_font().Text(var->Meta().friendly);
    }
    //printf("%d %d\n",raster[0],raster[1]);
    DrawTextWindowCoords(gltext, raster[0], raster[1]);

    std::ostringstream oss;
    oss << setprecision(4) << val;
    string str = oss.str();
    GlText glval = default_font().Text(str);
    const float l = glval.Width() + 2.0f;
    DrawTextWindowCoords(glval,  v.l + v.w - l, raster[1] );
}


TextInput::TextInput(std::string title, const std::shared_ptr<VarValueGeneric> &tv)
    : Widget<std::string>(title+":", tv), can_edit(!(tv->Meta().flags & META_FLAG_READONLY)), do_edit(false)
{
    top = 1.0; bottom = Attach::Pix(-2 * tab_h());
    left = 0.0; right = 1.0;
    hlock = LockLeft;
    vlock = LockBottom;
    handler = this;
    sel[0] = -1;
    sel[1] = -1;
    gltext = default_font().Text(title);
}

void TextInput::Keyboard(View&, unsigned char key, int /*x*/, int /*y*/, bool pressed)
{
    if(can_edit && pressed && do_edit)
    {
        const bool selection = sel[1] > sel[0] && sel[0] >= 0;

        if(key == 13)
        {
            var->Set(edit);
            MarkVarChangedByGui(*var);

            do_edit = false;
            sel[0] = sel[1] = -1;
        }else if(key == 8) {
            // backspace
            if(selection)
            {
                edit = edit.substr(0,sel[0]) + edit.substr(sel[1],edit.length()-sel[1]);
                sel[1] = sel[0];
            }else{
                if(sel[0] >0)
                {
                    edit = edit.substr(0,sel[0]-1) + edit.substr(sel[0],edit.length()-sel[0]);
                    sel[0]--;
                    sel[1]--;
                }
            }
        }else if(key == 127){
            // delete
            if(selection)
            {
                edit = edit.substr(0,sel[0]) + edit.substr(sel[1],edit.length()-sel[1]);
                sel[1] = sel[0];
            }else{
                if(sel[0] < (int)edit.length())
                {
                    edit = edit.substr(0,sel[0]) + edit.substr(sel[0]+1,edit.length()-sel[0]+1);
                }
            }
        }else if(key == 230){
            // right
            sel[0] = min((int)edit.length(),sel[0]+1);
            sel[1] = sel[0];
        }else if(key == 228){
            // left
            sel[0] = max(0,sel[0]-1);
            sel[1] = sel[0];
        }else if(key == 234){
            // home
            sel[0] = sel[1] = 0;
        }else if(key == 235){
            // end
            sel[0] = sel[1] = (int)edit.length();
        }else if(key < PANGO_SPECIAL){
            edit = edit.substr(0,sel[0]).append(1,key) + edit.substr(sel[1],edit.length()-sel[1]);
            sel[1] = sel[0];
            sel[0]++;
            sel[1]++;
        }
        CalcVisibleEditPart();
    }
}

void TextInput::Mouse(View& /*view*/, MouseButton button, int x, int /*y*/, bool pressed, int /*mouse_state*/)
{
    if(can_edit && button != MouseWheelUp && button != MouseWheelDown )
    {

        if(do_edit)
        {
            const int sl = (int)gledit.Width() + vertical_margin;
            const int rl = v.l + v.w - sl;
            std::string edit_visible = edit.substr(edit_visible_part[0], edit_visible_part[1]);
            int ep = (int)edit_visible.length();
            
            if( x < rl )
            {
                ep = 0;
            }else{
                for( unsigned i=0; i<edit_visible.length(); ++i )
                {
                    const int tl = (int)(rl + default_font().Text(edit_visible.substr(0,i)).Width());
                    if(x < tl+2)
                    {
                        ep = i;
                        break;
                    }
                }
            }
            if(pressed)
            {
                sel[0] = sel[1] = ep + edit_visible_part[0];
            }else{
                sel[1] = ep + edit_visible_part[0];
            }

            if(sel[0] > sel[1])
                std::swap(sel[0],sel[1]);
        }else{
            do_edit = !pressed;
            sel[0] = 0;
            sel[1] = (int)edit.length();
        }
    }
}

void TextInput::MouseMotion(View&, int x, int /*y*/, int /*mouse_state*/)
{
    if(can_edit && do_edit)
    {
        const int sl = (int)gledit.Width() + 2;
        const int rl = v.l + v.w - sl;
        std::string edit_visible = edit.substr(edit_visible_part[0], edit_visible_part[1]);
        int ep = (int)edit_visible.length();
        
        if( x < rl )
        {
            ep = 0;
        }else{
            for( unsigned i=0; i<edit.length(); ++i )
            {
                const int tl = (int)(rl + default_font().Text(edit_visible.substr(0,i)).Width());
                if(x < tl+2)
                {
                    ep = i;
                    break;
                }
            }
        }
        
        sel[1] = ep + edit_visible_part[0];
    }
}


void TextInput::ResizeChildren()
{
    vertical_margin = (v.h-2.f*gltext.Height()) / 4.0f;
    input_width = v.w - 2 * horizontal_margin;
    int max_possible_chars = floor(input_width / x_width());
    edit_visible_part[1] = max_possible_chars;
    CalcVisibleEditPart();
}

void TextInput::CalcVisibleEditPart()
{
    edit_visible_part[0] =  0;

    if(default_font().Text(edit).Width() > input_width)
    {
      if(sel[1] >= 0 )
      {
        edit_visible_part[0] = max(0, sel[1] - edit_visible_part[1]);
      }
    }
};

void TextInput::Render()
{
    if(!do_edit) edit = var->Get();

    Viewport input_v(v.l,v.b,v.w,v.h / 2);
    
    glColor4fv(colour.fg);
    if(can_edit) glRect(input_v);

    std::string edit_visible = edit.substr(edit_visible_part[0], edit_visible_part[1]);
    gledit = default_font().Text(edit_visible);

    const int sl = (int)gledit.Width() + horizontal_margin;
    const int rl = v.l + v.w - sl;

    if( do_edit && sel[0] >= 0)
    {
        const int tl = (int)(rl + default_font().Text(edit_visible.substr(0,sel[0] - edit_visible_part[0])).Width());
        const int tr = (int)(rl + default_font().Text(edit_visible.substr(0,sel[1] - edit_visible_part[0])).Width());
        glColor4fv(colour.dn);
        glRect(Viewport(tl,input_v.b,tr-tl,input_v.h));

        glColor4fv(colour.tx);
        GLfloat caret[4] = {(float) tr, (float) input_v.b, (float) tr, (float) input_v.b + input_v.h};
        glLine(caret);
    }

    glColor4fv(colour.tx);
    DrawTextWindowCoords(gltext, v.l + horizontal_margin, v.b + gltext.Height() + 3.f * vertical_margin);

    DrawTextWindowCoords(gledit, (GLfloat)(rl), input_v.b + vertical_margin);
    if(can_edit) DrawShadowRect(input_v);
}

}
