#pragma once
#include <utility>

namespace mdsp
{
    // Helper class that wraps a reference to the parent object and the value type.
    // ConfigBuilder is used to make chainable/fluent configurations easier, without writing any setters or getters.
    // Assignment to another ConfigBuilder overwrites only the value that's stored, keeping the parent reference intact.
    //
    // Config example:
    //      struct Configuration
    //      {
    //          template<typename T>
    //          using Builder = ConfigBuilder<Configuration, T>;
    //
    //          Builder<int> int_option;
    //          Builder<double> double_option;
    //          
    //          Configuration()
    //              : int_option(*this, 0)
    //              , double_option(*this, 1.0)
    //          {
    //          }
    //          
    //          Configuration(const Configuration& other)
    //              : int_option(*this, other.int_option())
    //              , double_option(*this, other.double_option())
    //          {
    //          }
    //
    //          *copy/move operators*
    //      };
    //
    // Usage example:
    //      auto cfg = Configuration()
    //          .int_option(1389)
    //          .double_option(42.0);
    //      
    //      int i = cfg.int_option();
    //      double d = cfg.double_option();
    //
    template<typename Self, typename T>
    class ConfigBuilder
    {
    protected:
        Self* _self;
        T _value;

    public:
        ConfigBuilder(Self& self, T defaultValue)
            : _self(&self)
            , _value(std::move(defaultValue))
        {
        }

        ConfigBuilder(const ConfigBuilder& other) = delete;
        ConfigBuilder(ConfigBuilder&& other) = delete;

        ConfigBuilder& operator=(const ConfigBuilder& other)
        {
            _value = other._value;

            return *this;
        }
        
        ConfigBuilder& operator=(ConfigBuilder&& other) noexcept
        {
            _value = std::move(other._value);

            return *this;
        }

        const T& operator()() const
        {
            return _value;
        }

        T& operator()()
        {
            return _value;
        }

        Self& operator()(const T& value)
        {
            _value = value;

            return *_self;
        }

        Self& operator()(T&& value)
        {
            _value = std::move(value);

            return *_self;
        }
    };
}
