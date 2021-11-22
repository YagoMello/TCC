#ifndef FOREACH_HPP
#define FOREACH_HPP

template <typename list_t, typename ... args_t>
constexpr void for_each_call_member(list_t ** list, void (list_t:: * const func)(args_t...), args_t ... args){
    list_t ** iterator = list;
    while(*iterator != nullptr){
        ((*iterator)->*func)(args...);
        iterator++;
    }
}

template <typename list_t, typename ... args_t>
constexpr void for_each_call_member(list_t ** list, void (list_t:: * const func)(args_t...) const, args_t ... args){
    list_t ** iterator = list;
    while(*iterator != nullptr){
        ((*iterator)->*func)(args...);
        iterator++;
    }
}

/*
#define FOR_EACH_CALL(list, func, ...) \
    while(*list != nullptr){ \
        (*list)->func(__VA_ARGS__); \
        list += 1; \
    }

#define FOR_EACH_WRITE(list, value) \
    while(*list != nullptr){ \
        *list = value; \
        list += 1; \
    }
*/

#endif // FOREACH_HPP
