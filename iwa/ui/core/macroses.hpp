#include "signal.hpp"
#include "logger.hpp"

#define INTERNAL_MACROSES
#ifdef INTERNAL_MACROSES

#define EMPTY

#define BUILDER_TYPE int

#define CONSTRUCT(builder_type, widget_type)                                              \
   builder_type *construct()                                                              \
   {                                                                                      \
      auto object = static_cast<builder_type *>(new abstract_builder<widget_type>(this)); \
      this->__builder_ptr = object;                                                       \
      return object;                                                                      \
   };

#define VALUE(type, name)                          \
   constexpr BUILDER_TYPE *name(const type &value) \
   {                                               \
      this->instance->__##name = value;            \
      return static_cast<BUILDER_TYPE *>(this);    \
   }

#define RECOMPUTABLE_VALUE(type, name)             \
   constexpr BUILDER_TYPE *name(const type &value) \
   {                                               \
      this->instance->__##name = value;            \
      this->instance->recompute();                 \
      return static_cast<BUILDER_TYPE *>(this);    \
   }

#define VALUE_PTR(type, name)                \
   constexpr BUILDER_TYPE *name(type *value)    \
   {                                            \
      this->instance->__##name = value;         \
      return static_cast<BUILDER_TYPE *>(this); \
   }

#define TOGGEABLE_VALUE(name)                           \
   VALUE(bool, name)                                    \
   constexpr BUILDER_TYPE *name##_toggle()              \
   {                                                    \
      this->instance->__##name = !this->instance->__##name; \
      return static_cast<BUILDER_TYPE *>(this);         \
   }

#define SIGNAL_VALUE(type, name)                                          \
   constexpr BUILDER_TYPE *on_##name(iwa::signal<type>::callback_t value) \
   {                                                                      \
      if (!this->instance->__##name##_signal)                             \
         this->instance->__##name##_signal = new iwa::signal<type>();     \
      this->instance->__##name##_signal->add(value);                      \
      return static_cast<BUILDER_TYPE *>(this);                           \
   }

#define CALL_SIGNAL(signal_ptr, params...) \
   if (signal_ptr)                         \
      signal_ptr->call(params);

#define FRIEND_BUILDER()               \
   template <typename T>               \
   friend class abstract_builder;      \
   template <typename T, typename U>   \
   friend class abstract_widget_builder;

#define COMPONENT(type, name, default_value)    \
   protected:                                   \
      type __##name = default_value;            \
   public:                                      \
      void set_##name(const type &value)        \
      {                                         \
         this->__##name = value;                \
      }                                         \
      const type &get_##name()                  \
      {                                         \
         return this->__##name;                 \
      }

#define NCSTREF_COMPONENT(type, name, default_value)  \
   protected:                                         \
      type __##name = default_value;                  \
   public:                                            \
      void set_##name(const type &value)              \
      {                                               \
         this->__##name = value;                      \
      }                                               \
      type &get_##name()                              \
      {                                               \
         return this->__##name;                       \
      }

#define TOGGEABLE_COMPONENT(name, default_value)   \
   protected:                                      \
      bool __##name = default_value;               \
   public:                                         \
      void set_##name(bool value)                  \
      {                                            \
         this->__##name = value;                   \
      }                                            \
      bool get_##name()                            \
      {                                            \
         return this->__##name;                    \
      }                                            \
      void toggle_##name()                         \
      {                                            \
         this->__##name = !this->__##name;         \
      }
   
#define COMPONENT_PTR(type, name, default_value)   \
   protected:                                      \
      type* __##name = default_value;              \
   public:                                         \
      void set_##name(type* value)                 \
      {                                            \
         this->__##name = value;                   \
      }                                            \
      type *get_##name()                           \
      {                                            \
         return this->__##name;                    \
      }
      
#define SIGNAL_COMPONENT(type, name, default_value)   \
   protected:                                         \
      signal<type>* __##name##_signal = default_value;\
   public:                                            \
      void set_##name##_signal(signal<type>* value)   \
      {                                               \
         this->__##name##_signal = value;             \
      }                                               \
      signal<type>* get_##name##_signal()             \
      {                                               \
         return this->__##name##_signal;              \
      }

#define CONST_COMPONENT(type, name, default_value) \
   protected:                                      \
      type __##name = default_value;               \
   public:                                         \
      const type &get_##name()                     \
      {                                            \
         return this->__##name;                    \
      }

#define RECOMPUTABLE_COMPONENT(type, name, default_value)   \
   protected:                                               \
      type __##name = default_value;                        \
   public:                                                  \
      void set_##name(const type &value)                    \
      {                                                     \
         if (this->__##name != value)                       \
         {                                                  \
            this->__##name = value;                         \
            this->recompute();                              \
         }                                                  \
      }                                                     \
      const type &get_##name()                              \
      {                                                     \
         return this->__##name;                             \
      }



#define DRAW_OPTIONAL(optional, data) \
   if (optional)                      \
      optional->draw(data);

#endif