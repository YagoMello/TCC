#ifndef AUTOREG_HPP
#define AUTOREG_HPP

// INCLUDE <>
#include <map>
#include <string>
#include <string_view>
#include <memory>
#include <utility> // std::forward

namespace autoreg {

template <typename base_type>
using builder_type = base_type * (*)();

template <typename base_type>
using list = std::map<std::string, builder_type<base_type>>;

template <typename base_type, typename derived_type>
base_type * default_builder() {
    return new derived_type;
}

template <typename derived_type>
class reg {
public:
    template <typename base_type>
    reg(list<base_type> & ls, const std::string & key, builder_type<base_type> builder = &default_builder<base_type, derived_type>) {
        ls[key] = builder;
    }
    template <typename base_type>
    reg(list<base_type> & ls, builder_type<base_type> builder = &default_builder<base_type, derived_type>) {
        ls[derived_type::key()] = builder;
    }
};

template <typename base_type>
inline base_type * safe_build(list<base_type> & ls, const std::string & key) {
    auto iter = ls.find(key);
    if(iter != ls.end()){
        return (iter->second)();
    }
    else{
        return nullptr;
    }
}

} // namespace autoreg

namespace autoreg_uptr {

template <typename base_type>
class factory_base {
public:
    virtual std::unique_ptr<base_type> build() const = 0;
};

template <typename base_type, typename derived_type>
class factory : public factory_base<base_type> {
public:
    std::unique_ptr<base_type> build() const final {
        return std::make_unique<derived_type>();
    }
};

template <typename base_type>
using list = std::map<std::string, std::unique_ptr<factory_base<base_type>>>;

template <typename derived_type>
class reg {
public:
    template <typename base_type>
    reg(list<base_type> & ls, const std::string & key) {
        ls[key] = std::make_unique<factory<base_type, derived_type>>();
    }
};

}

namespace autoreg_bad {

template <typename base_type, typename ... pack_type>
class factory_base {
public:
    virtual std::unique_ptr<base_type> build(pack_type && ...) const = 0;
};

template <typename base_type, typename derived_type, typename ... pack_type>
class factory : public factory_base<base_type, pack_type ...> {
public:
    std::unique_ptr<base_type> build(pack_type && ... args) const final {
        return std::make_unique<derived_type>(std::forward<pack_type ...>(args ...));
    }
};

template <typename base_type, typename ... pack_type>
using list = std::map<std::string, std::unique_ptr<factory_base<base_type, pack_type ...>>>;

template <typename derived_type, typename ... pack_type>
class reg {
public:
    template <typename base_type>
    reg(list<base_type, pack_type ...> & ls, const std::string & key) {
        ls[key] = std::make_unique<factory<base_type, derived_type, pack_type ...>>();
    }
};

}

#endif // AUTOREG_HPP
