#pragma once

#include "voe/detail/templates/front.h"
#include "voe/detail/type_list/list.h"

namespace util::tpl::impl {

template <template <typename...> class Template, typename... Args>
struct BindFront {
    template <typename... Ts>
    using TN = Template<Args..., Ts...>;

    template <typename Ts>
    using T1 = TN<Ts>;
};

template <template <typename...> class Template>
struct TransferTo {
    template <typename Ts>
    struct TransferHolder;

    template <template <typename...> class T, typename... Types>
    struct TransferHolder<T<Types...>> {
        using type = Template<Types...>;
    };

    template <typename Ts>
    using Transfer = typename TransferHolder<Ts>::type;
};

template <template <typename...> typename Template, typename TL>
struct ApplyToTemplate;

template <template <typename...> typename Template, typename... Ts>
struct ApplyToTemplate<Template, ::util::list::list<Ts...>> {
    using type = Template<Ts...>;
};

template <typename T, template <typename...> class Template>
struct IsInstanceOfTemplate : std::false_type {};

template <typename... V, template <typename...> class Template>
struct IsInstanceOfTemplate<Template<V...>, Template> : std::true_type {};

}  // namespace util::tpl::impl
