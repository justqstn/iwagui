#include "ui.hpp"

#include "windows.h"

#include <cmath>
#include <format>
#include <optional>
namespace vars
{
    bool rendering = false;
    bool toggled = false;
    bool first_time = true;
}

void ui::setstyle()
{
    ImGui::StyleColorsDark();
}

static iwa::event<> on_toggle_post;
static iwa::event<bool> on_toggle_rendering;

ui::ui()
{
    this->on_toggle.add([](){
        vars::rendering = true;
        on_toggle_rendering.call(true);
        on_toggle_post.call();
        vars::first_time = false;
    });

    iwa::load_font("Verdana", FONTS_DIR"verdana.ttf");
};

void ui::tick(float dt)
{
    iwa::tween::tickall(dt);
}

void ui::render(float dt)
{
    auto io = ImGui::GetIO();

    static auto bg_tween_style = [](iwa::tween_params* params) -> void {
        params->backwards = true;
        params->enabled = false; 
        params->backward_speed.value = 7.0f;   
    };
    static auto bg_tween_style_post = [](iwa::tween* tween) -> void { on_toggle_post.add([tween](){ tween->enable(); tween->backward();  }); };
    
    static iwa::tween sh_alpha = ({
        iwa::tween_params params;

        params.style(bg_tween_style);
        params.style_post(bg_tween_style_post);
        params.forward_speed.value = 0.4f;
        params.forward_speed.fn = iwa::easings::out::cubic;
        params.backward_speed.fn = iwa::easings::in::cubic;   

        params;
    });
    static iwa::tween bg_alpha = ({
        iwa::tween_params params;
        params.style(bg_tween_style);
        params.style_post(bg_tween_style_post);
        params.forward_speed.value = 3.5f; 
        params;
    }); 
    static iwa::tween g_alpha = ({
        iwa::tween_params params;
        params.style(bg_tween_style);
        params.style_post(bg_tween_style_post);
        params.forward_speed.value = 5.0f; 
        params.start = new iwa::event<iwa::tween*>();
        params.start->add([](auto){ vars::rendering = false; on_toggle_rendering.call(false); ui::get_instance()->on_closed.call(); });
        params;
    });
   

    
    static iwa::head_window main = ({
        auto params = new iwa::head_window_params();

        params->color = ImColor(25, 25, 25);
        params->header_window = new iwa::window(({
            auto params = new iwa::window_params();

            params->size = {1,0.035};
            params->color = ImColor(190,190,190);

            params;
        }));

        params->draggable = true;
        params->drag_canvas = params->header_window->data();

        params->header_text = new iwa::text(({
            auto params = new iwa::text_params();

            params->on_pre_draw([params](auto){
                static auto color = ImColor(0,0,0);
                auto& rect = params->compute_rect();
                ImGui::GetForegroundDrawList()->AddShadowRect({rect.Min.x, rect.GetCenter().y}, {rect.Max.x, rect.GetCenter().y}, iwa::apply_alpha(color, g_alpha.value), 50, {0,0}, ImDrawFlags_RoundCornersAll, 100.0f);
            });
            params->shadow = new iwa::text_params::shadow_params();
            params->shadow->angle = rad(-45);
            params->shadow->color = ImColor(128,128,128);
            params->shadow->distance = 3.0f;
            params->font = "Verdana";
            params->anchor = {0.5,0.5};
            params->size = 0.75;
            params->pos = {0.5,0.5};
            params->text = "IwaGui";
            params->color = ImColor(255,255,255);

            params;
        }));

        params->outline_color = ImColor(74,74,74);
        params->header_line_color = params->outline_color;

        params->header_line_thickness = 1.0f;

        params->pos = {0.5,0.5};
        params->anchor = {0.5,0.5};
        params->size = {0.5,0.6};
        params->padding = ratio({ {.05,.05},{.05,.05} }, {1,1});
        params->id = "main";

        params->clipping = false;
        
        params->on_pre_draw([](float dt){
            static auto color = ImColor(225, 180, 200, 175);
            auto& rect = main.data()->compute_rect();
            auto drawlist = ImGui::GetForegroundDrawList();
            drawlist->AddShadowRect(rect.Min, rect.Max, (color & 0x00FFFFFF) | ((int)((color >> 24) * sh_alpha.value) << 24), 60, {0,0});
        }); 
        
        params->on_enter([](){ LOGI("Entered"); });
        params->on_leave([](){ LOGI("Left"); });

        on_toggle_rendering.add([](bool status){
            main.data()->enabled = status;
        });

        LOGD("Main window created");
        params;
    });

    static auto button_style = [](iwa::button_params* params)
    {
        params->anchor = {0.5,0.5};
        params->pos = {0.5,0.5};
        params->size = {0.1,0.1};
        params->color = ImColor(255,0,0);
        params->add_parent("main");
        params->drawflags = ImDrawFlags_RoundCornersAll;
        params->rounding = 20.0f;
        params->zindex = 1;
        params->on_released([params](){
            params->destroy_widget();
        });
    };

    static auto button_test1 = new iwa::button(({
        auto params = new iwa::button_params();

        params->style(button_style);
        params->pos += ImVec2(0.025,0.025);
        params->id = "red";


        params;
    }));


    static auto button_test2 = new iwa::button(({
        auto params = new iwa::button_params();

        params->style(button_style);
        params->pos -= ImVec2(0.025,0.025);
        params->color = ImColor(0,255,0);
        params->zindex += 1;
        params->id = "green";

        
        params;
    }));



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
    if (vars::rendering)
    {
        ImGui::GetBackgroundDrawList()->AddRectFilled({0,0}, {4000,4000}, ImColor(0.0f, 0.0f, 0.0f, bg_alpha.value * 0.3f));
        main.render(0);
        
    }
    else
    {
        
    }

    iwa::depth_mapper::get_instance()->input();
    iwa::depth_mapper::get_instance()->render(dt);



    { // scale and position debugging
        auto mouse_pos = ImGui::GetMousePos();
        auto drawlist =  ImGui::GetForegroundDrawList();

        auto screen_res = iwa::get_screen_resolution();
        ImVec2 scale = mouse_pos / screen_res;

        drawlist->AddText(mouse_pos - ImVec2(0, -20), IM_COL32_WHITE, std::format("{} {}", scale.x, scale.y).c_str());
        drawlist->AddText(mouse_pos - ImVec2(0, 20), IM_COL32_WHITE, std::format("{} {}", mouse_pos.x, mouse_pos.y).c_str());
        drawlist->AddText(mouse_pos + ImVec2(0, 40), IM_COL32_WHITE, iwa::depth_mapper::get_instance()->debug_information().data());
    }
}

/**
 * class snake_outline
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
        this->max = (window.data().size.x + window.data().size.y) * 2;
        
        if (this->backwards)
        {
            corners[1] = window.data().size.x;
            corners[2] = corners[1] + window.data().size.y;
            corners[3] = corners[2] + window.data().size.x;
            corners[0] = corners[3] + window.data().size.y;
        }
        else
        {
            corners[1] = window.data().size.y;
            corners[2] = corners[1] + window.data().size.x;
            corners[3] = corners[2] + window.data().size.y;
            corners[0] = corners[3] + window.data().size.x;
        }
        
        window.data().post.add([&value = this->progress->value](float dt) -> void {
            
        });
    }

    
    float min = 0;
    float max;
    float corners[4];
    float length;
    const iwa::tween* progress;
    const bool backwards;
};
 * 
 */