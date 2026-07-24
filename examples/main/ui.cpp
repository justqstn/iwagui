#include "ui.hpp"

#include "windows.h"

#include <cmath>
#include <format>

namespace vars
{
    bool opened = false;
    bool toggled = false;
    bool first_time = true;
}

void ui::setstyle()
{
    ImGui::StyleColorsDark();
}

static iwa::event<> on_toggle_post;

ui::ui()
{
    this->on_toggle.add([](){
        vars::opened = true;
        on_toggle_post.call();
        vars::first_time = false;
    });

    iwa::load_font("Verdana", FONTS_DIR"verdana.ttf");
}   

class snake_outline
{
public:
    snake_outline(iwa::head_window& window, float length, float speed, bool backwards) : length(length), backwards(backwards)
    {
        iwa::tween::params params;
        params.forward_speed.value = speed;
        params.backward_speed.value = speed;
        params.tp = iwa::tween_type::restartable;
        
        progress = new iwa::tween(params);
        this->init(window);
    }
    snake_outline(iwa::head_window& window, float length, const iwa::tween& tween, bool backwards) : length(length), backwards(backwards)
    {
        progress = new iwa::tween(tween);
        this->init(window);
    }
private:
    void init(iwa::head_window& window)
    {
        this->max = (window.data.size.x + window.data.size.y) * 2;
        
        if (this->backwards)
        {
            corners[1] = window.data.size.x;
            corners[2] = corners[1] + window.data.size.y;
            corners[3] = corners[2] + window.data.size.x;
            corners[0] = corners[3] + window.data.size.y;
        }
        else
        {
            corners[1] = window.data.size.y;
            corners[2] = corners[1] + window.data.size.x;
            corners[3] = corners[2] + window.data.size.y;
            corners[0] = corners[3] + window.data.size.x;
        }
        
        window.data.post.addcpt([&value = this->progress->value](float dt) -> void {
            
        });
    }

    
    float min = 0;
    float max;
    float corners[4];
    float length;
    const iwa::tween* progress;
    const bool backwards;
};

void ui::tick(float dt)
{
    iwa::tween::tickall(dt);
}

void ui::render(float dt)
{
    auto io = ImGui::GetIO();


    static auto bg_tween_style = [](iwa::tween::params& params) -> void {
        params.backwards = true;
        params.enabled = false; 
        params.backward_speed.value = 7.0f;   
    };
    static auto bg_tween_style_post = [](iwa::tween& tween) -> void { on_toggle_post.addcpt([&tween = tween](){ tween.enable(); tween.backward();  }); };
    
    static iwa::tween sh_alpha = ({
        iwa::tween::params params;

        params.style(bg_tween_style);
        params.style_post(bg_tween_style_post);
        params.forward_speed.value = 0.4f;
        params.forward_speed.fn = iwa::easings::out::cubic;
        params.backward_speed.fn = iwa::easings::in::cubic;   

        params;
    });
    static iwa::tween bg_alpha = ({
        iwa::tween::params params;
        params.style(bg_tween_style);
        params.style_post(bg_tween_style_post);
        params.forward_speed.value = 3.5f; 
        params;
    });
    static iwa::tween g_alpha = ({
        iwa::tween::params params;
        params.style(bg_tween_style);
        params.style_post(bg_tween_style_post);
        params.forward_speed.value = 5.0f; 
        params.on_start.add([](auto&){ vars::opened = false; });
        params;
    });
   

    
    static iwa::head_window main = ({
        iwa::head_window::params params;

        params.color = ImColor(25, 25, 25);
        params.header_color = ImColor(19,19,19); params.header_line_color = ImColor(177, 177, 177, 45);
        params.header_thickness = 0.03f;  params.header_thickness.scaling();

        params.pos = {0.5,0.5};
        params.anchor = {0.5,0.5};
        params.size = {0.5,0.6};
        params.padding = ratio({ {.05,.05},{.05,.05} }, {1,1});


        params.scaling();
        params.clipping = false;
        params.outline_color = ImColor(177, 177, 177, 45);
        params.pre.add([](float dt){
            static auto color = ImColor(225, 180, 200, 175);
            auto& rect = main.data.compute_rect();
            auto drawlist = ImGui::GetForegroundDrawList();
            drawlist->AddShadowRect(rect.Min, rect.Max, (color & 0x00FFFFFF) | ((int)((color >> 24) * sh_alpha.value) << 24), 60, {0,0});
        }); 
        
        params.enter.addcpt([](){ LOGI("Entered"); });
        params.leave.add([](){ LOGI("Left"); });

        LOGD("Main window created");
        params;
    });

    static auto test_button_style = [](iwa::button::params& params) -> void{ 
        params.pos = {0.5,0.5};
        params.anchor = {0.5,0.5};
        params.size = {0.1,0.1};
        params.scaling();
        params.color = ImColor(100,100,100);        
    };

    static auto test_button_style_post = [](iwa::button& button) -> void 
    {
        button.data.down.addcpt([&](){
            auto& params = button.data;
            LOGI("down");
            params.color = ImColor(0,0,0);
        });

        button.data.up.addcpt([&](){
            auto& params = button.data;
            LOGI("up");
            params.color = ImColor(100,100,100);
        });

        main.add_widget(button);
    };

    static iwa::button test_button = ({
        iwa::button::params params;

        params.style_post(test_button_style_post);
        params.style(test_button_style);
        
        params;
    });

    static iwa::button test_button2 = ({
        iwa::button::params params;

        params.style_post(test_button_style_post);
        params.style(test_button_style);
        params.pos += {.025,.025};
        params.zindex = 100;
        
        
        params;
    });

    static auto verdana_text = [](iwa::text& widget)
    {
        auto& params = widget.data;
        params.text = "Test text";
        params.font = "Verdana";
        params.color = IM_COL32_WHITE;
        params.size = 0.75f; params.size.factor(main.data.header_thickness.get());
        params.anchor = {0.5,0.5};

        params.scaling();
        //main.add_widget(widget);
    };

    static iwa::window test_window = ({
        iwa::window::params params;

        params.pos = {0.5,0.5};
        params.anchor = {0.5,0.5};
        params.size = {1,1};
        params.color = ImColor(50,50,50);
        params.clipping = false;
        params.scaling();
       // params.zindex = 100;

        //main.add_widget(test_window);

        params;
    });
    
    static iwa::text example_new = ({
        iwa::text::params params;
        params.style_post(verdana_text);
        params.pos = {0.25,0};
        params;
    });


    static iwa::text example = ({
        iwa::text::params params;
        params.style_post(verdana_text);
        params.pos = {0.25,0.03 / 2};
        params;
        //test_window.add_widget(example_new);
        params;
    });

    static iwa::text example_shadowed = ({
        iwa::text::params params;
        params.style_post(verdana_text);
        params.pos = {0.75,0.03 / 2};
        params.shadow.angle = rad(-45); params.shadow.distance = 1.5f; params.shadow.color = ImColor(0,0,0);
        params;
    });


    //LOGE("%s", ((iwa::text*)example.ptr())->data.text.data());
    


    static snake_outline outline(main, 100.f, 1.0f, false);

    /*{
        static const float snake_length = 850;
        static float snake_speed = 400; 

        static ImVec2 corner_point;
        static ImVec2 snake_head = main.params.pos;
        static ImVec2 snake_tail = ({
            main.params.post.add([](const float dt){
                auto drawlist = ImGui::GetForegroundDrawList();
                static ImColor color(225,180,200,100);
                auto current_color = (color & 0x00FFFFFF) | ((int)((color >> 24) * sh_alpha.value) << 24);
                
                if (corner_point.x != 0)
                {
                    drawlist->AddShadowRect(ImMin(snake_head, corner_point), ImMax(snake_head, corner_point)+ImVec2(1,1), current_color, 25, {0,0});
                    drawlist->AddShadowRect(ImMin(snake_tail, corner_point), ImMax(snake_tail, corner_point)+ImVec2(1,1), current_color, 25, {0,0});
                }
                else
                {
                    drawlist->AddShadowRect(ImMin(snake_head, snake_tail), ImMax(snake_head, snake_tail)+ImVec2(1,1), current_color, 25, {0,0});
                }
                
            });

            ImVec2{snake_head.x + snake_length, snake_head.y};
        });
        
        static float x_min = main.params.pos.x; static float y_min = main.params.pos.y;
        static float y_max = main.params.pos.y + main.params.size.y; static float x_max = main.params.pos.x + main.params.size.x;


        
        static byte snake_head_state = 0; 
        static byte snake_tail_state = 3; 

        static auto do_snake_state = [](ImVec2& snake_position, byte& snake_state, const float dt)
        {
            if (snake_position.y == y_min)
            {
                if (snake_position.x == x_min) snake_state = 0;
                else if (snake_position.x == x_max) snake_state = 3;
            }
            else if (snake_position.y == y_max)
            {
                if (snake_position.x == x_min) snake_state = 1;
                else if (snake_position.x == x_max) snake_state = 2;
            }
            switch (snake_state)
            {
            case 0: snake_position.y += dt * snake_speed; break;
            case 1: snake_position.x += dt * snake_speed; break;
            case 2: snake_position.y -= dt * snake_speed; break;
            case 3: snake_position.x -= dt * snake_speed; break;
            }
        };
        
        do_snake_state(snake_head, snake_head_state, dt);
        do_snake_state(snake_tail, snake_tail_state, dt);

        snake_head = ImClamp(snake_head, main.params.pos, {x_max, y_max});
        snake_tail = ImClamp(snake_tail, main.params.pos, {x_max, y_max});
        
             if (snake_head_state == 0 && snake_tail_state == 3) corner_point = {x_min, y_min};
        else if (snake_head_state == 1 && snake_tail_state == 0) corner_point = {x_min, y_max};
        else if (snake_head_state == 2 && snake_tail_state == 1) corner_point = {x_max, y_max};
        else if (snake_head_state == 3 && snake_tail_state == 2) corner_point = {x_max, y_min};
        else corner_point = {0,0};
    }*/

    

    ImGui::GetStyle().Alpha = g_alpha.value;
    if (vars::opened)
    {
        ImGui::GetBackgroundDrawList()->AddRectFilled({0,0}, {4000,4000}, ImColor(0.0f, 0.0f, 0.0f, bg_alpha.value));
        main.render();
        iwa::zindex_manager::get_instance()->render(dt);
    }
    else
    {
        
    }



    { // scale and position debugging
        auto mouse_pos = ImGui::GetMousePos();
        auto drawlist =  ImGui::GetForegroundDrawList();

        auto screen_res = iwa::get_screen_resolution();
        ImVec2 scale = mouse_pos / screen_res;

        drawlist->AddText(mouse_pos - ImVec2(0, -20), IM_COL32_WHITE, std::format("{} {}", scale.x, scale.y).c_str());
        drawlist->AddText(mouse_pos - ImVec2(0, 20), IM_COL32_WHITE, std::format("{} {}", mouse_pos.x, mouse_pos.y).c_str());
    }
}