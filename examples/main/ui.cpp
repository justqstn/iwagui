#include "ui.hpp"


#include "windows.h"

#include <cmath>
#include <format>
#include <optional>
#include <meta>

using namespace iwa;


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

static iwa::signal<> on_toggle_post;
static iwa::signal<bool> on_toggle_rendering;



ui::ui()
{
    logger::get_instance()->ignore("RECOMPUTING");
    context::get_global()->set_enabled(false);
    this->on_toggle.add([](){
        context::get_global()->set_enabled(true);
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

auto bg_tween_style = [](iwa::tween::builder* builder){
    builder
        ->backwards(true)
        ->enabled(false)
        ->rev_speed(4.5f)
        ;

    on_toggle_post.add([instance = builder->instance](){
        instance->set_enabled(true);
        instance->toggle_backwards();
    });
};


auto global_alpha = [](){
    return (new iwa::tween)->construct()
        ->style(bg_tween_style)
        ->speed(7.5f)
        ->build();
    }();

auto global_ctx = [](){
    auto builded = (new context::builder)->global()->build();
    global_alpha->bind(builded->get_alpha());
    return builded;
}();

auto background_alpha = [](){
    return (new iwa::tween)->construct()
        ->style(bg_tween_style)
        ->speed(3.0f)
        ->range({0.0f, 0.4f})
        ->on_start([](){
            context::get_global()->set_enabled(false);
            on_toggle_rendering.call(false); 
            ui::get_instance()->on_closed.call(); 
        })
        ->build();
    }();






auto ctx2 = [](){
    auto builded = (new context::builder)->build();

    return builded; 
}();


void ui::render(float dt)
{
    global_ctx->set_bounds({{0,0}, get_screen_resolution()});

    static auto frame = ({
        (new bounded_draggable_frame::builder())
            ->bind()
            ->id("frame")
            ->child_tag("child")
            ->size({0.5,0.5})
            ->pos({0.5,0.5})
            ->anchor({0.5,0.5})
            ->color(color_t(0,0,0,150))
            ->drag_bounds(&global_ctx->get_bounds())
            ->on_enter([](){ LOGI("enter") })
            ->on_leave([](){ LOGI("leave") })
            ->on_click([](){ LOGI("click") })
            ->on_release([](){ LOGI("release") })
            ->on_start_dragging([](){ LOGI("start dragging") } )
            ->on_end_dragging([](){ LOGI("end dragging") } )
            ->on_collide_with_side([](side_t side) { 
                if (!!(side & side_t::left))
                    LOGI("colliding with left")
                if (!!(side & side_t::right))
                    LOGI("colliding with right")
                if (!!(side & side_t::upper))
                    LOGI("colliding with upper")
                if (!!(side & side_t::bottom))
                    LOGI("colliding with bottom")
            })
            ->build();
    });

    static auto child_frame = ({
        (new frame::builder())
            ->tag("child")
            ->id("frame2")
            ->size({1,0.03})
            ->color(color_t(64,44,55))
            ->build();
    });

    
    gui_controller::get_instance()->tick(dt);
}