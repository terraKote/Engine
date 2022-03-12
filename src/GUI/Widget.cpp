/*!
  boldtemple Gaming ©, http://gaming.boldtemple.net
  OpenSource Project - Check Development, License & Readme Sections.
  
  BGE - boldtemple Cross Platform Game Engine
  /GUI/Widget.cpp
!*/

#include "GUI/Widget.h"

Widget::Widget(const Vec2& pos, const Vec2& size):
    _visible(true), _bg_visible(true)
{
    Object::SetPos(pos);
    Object::SetSize(size);

    _back.SetFrameSize(size);

    IgnoreWheel(true);

    Object::SetCamera(GUI::GetCamera());
}

Widget::~Widget() {
    if (_back.GetTexture() != nullptr) {
        SDL_DestroyTexture(_back.GetTexture());
    }

    Object::SetCamera(GUI::GetCamera());
}

void Widget::Show(bool show){
    _visible = show;
}

void Widget::ShowBack(bool show_bg){
    _bg_visible = show_bg;
}

void Widget::SetBackGround(const std::string& tileset,const Vec2& skin, int tile_size) {
    if(_back.GetTexture() != nullptr){
        SDL_DestroyTexture(_back.GetTexture());
        _back.SetTexture(nullptr);
    }

    //TODO FIX THIS
    if(tileset.empty()){
        return;
    }

    SDL_Texture *texture_skin = Resources::GetTexture(tileset);

    if(!texture_skin){
        return;
    }

    SDL_Rect rect = {0, 0, static_cast<int>(GetSize().x), static_cast<int>(GetSize().y)};

    SDL_Texture *texture_back = SDL_CreateTexture(Window::GetRenderer(), SDL_PIXELFORMAT_RGBA8888,
                                                  SDL_TEXTUREACCESS_TARGET, rect.w,
                                                  rect.h);
    SDL_SetTextureBlendMode(texture_back, SDL_BLENDMODE_BLEND);

    Surface::BeginViewport(Vec2::ZERO, Vec2(rect.x, rect.y));
    Surface::GetSkinnedRect(texture_skin, texture_back, &skin, &rect, tile_size);
    Surface::EndViewport();

    _back.SetTexture(texture_back);
}

void Widget::OnTopMouseEvent(){
    if(Mouse::AnyPressed()){
        if(Mouse::Pressed(MOUSE_LEFT)){
            if ((_state == WIDGET_HOVERED || _state == WIDGET_NORMAL)){
                EmitAction("mousedown");

                _state = WIDGET_PRESSED;
            }
        }
    }else if(_state == WIDGET_PRESSED){
            EmitAction("mouseup");
            _state = WIDGET_NORMAL;
    }
    else if(_state != WIDGET_HOVERED){
        EmitAction("mousehover");
        _state = WIDGET_HOVERED;
    }

    _was_intersected = true;
}

void Widget::OnTopMouseWheelEvent(){
    if(Mouse::Wheeled(MOUSE_WHEEL_UP)){
        EmitAction("wheelup");
    }
    else if(Mouse::Wheeled(MOUSE_WHEEL_DOWN)){
        EmitAction("wheeldown");
    }
}

void Widget::OnMouse(){
    _intersected = true;
}

void Widget::OnUpdate(){
    if (!IsVisible() || IsClickIgnored()) {
        return;
    }

    CheckTop();

    if(!_intersected){
        if(Mouse::AnyPressed()){
            EmitAction("mousepressout");
        }
    }

    if(_was_intersected && !_intersected){
        EmitAction("mouseleave");

        _was_intersected = false;
        _state = WIDGET_NORMAL;
    }

    if(_intersected){
       _intersected = false;
    }
}

const bool& Widget::IsVisible() const{
    return _visible;
}

const bool& Widget::BackIsVisible() const{
    return _bg_visible;
}

void Widget::AddAction(const std::string& action_name, std::function<void ()> action, Object* listener){
    size_t s = _Listeners.size();
    bool found = false;

    for(size_t i = 0; i < s; ++i){
        if(_Listeners[i].listener == listener){
            _Listeners[i].actions[action_name] = action;
            found = true;
            break;
        }
    }

    if(!found){
        Actions actions;
        actions.listener = listener;
        actions.actions[action_name] = action;

        _Listeners.push_back(actions);
    }
}

void Widget::DeleteAction(const std::string& action_name, Object* listener){
    size_t s = _Listeners.size();

    for(size_t i = 0; i < s; ++i){
        if(_Listeners[i].listener == listener){
            if(_Listeners[i].actions.count(action_name)){
                _Listeners[i].actions.erase(action_name);
                break;
            }
        }
    }
}

void Widget::DeleteAllActions(Object* listener){
    for(auto it = _Listeners.begin(); it != _Listeners.end(); ++it){
        if(it->listener == listener){
            _Listeners.erase(it);
            break;
        }
    }
}

void Widget::EmitAction(const std::string& action_name){
    size_t s = _Listeners.size();

    for(size_t i = 0; i < s; ++i){
       if(_Listeners[i].actions.count(action_name)){
           auto fun = _Listeners[i].actions[action_name];

           if(fun){
               fun();
           }
       }
    }
}
