#include "context.hpp"
#include "widget_controller.hpp"
#include "render_controller.hpp"

using namespace iwa;

context* context::__global_ctx = nullptr;
std::unordered_map<std::string, context*> context::__ctxs = {};

context::context() {}

context::~context()
{
    context::__ctxs.erase(this->__id);
}

void context::build()
{
    this->buildable<context>::build();
    context::__ctxs[__id] = this;
}

context* context::get(const std::string& id)
{
    if (context::__ctxs.contains(id))
    {
        return context::__ctxs[id];
    }
    else
    {
        LOGE("No context with id \"%s\" created.", id.data())
        return nullptr;
    }
}

context* context::get_global()
{   
    if (!context::__global_ctx) LOGE("No global context created.")
    return context::__global_ctx;
}

const ctx_map_t& context::get_ctx_map()
{
    return context::__ctxs;
}

void context::push()
{
    for (auto widget : this->widgets)
    {
        widget->set_bounds(__bounds);
        widget->push(__zindex);
    }
}