#pragma once

#include "result/detail/templates/preface.h"

namespace util::tpl {

template <template <typename...> class Template, typename... Args>
using bindFront = impl::BindFront<Template, Args...>;

template <template <typename...> class Template>
using transferTo = impl::TransferTo<Template>;

template <template <typename...> typename Template, typename TypeList>
using applyToTemplate = typename impl::ApplyToTemplate<Template, TypeList>::type;

template <typename T, template <typename...> typename Template>
[[maybe_unused]] static constexpr bool isInstanceOfTemplate =
    impl::IsInstanceOfTemplate<T, Template>::value;

}  // namespace util::tpl
