#pragma once

namespace util::tpl::impl {

template <template <typename...> class Template, typename... Args>
struct BindFront;

template <template <typename...> class Template>
struct TransferTo;

template <template <typename...> typename Template, typename TypeList>
struct ApplyToTemplate;

template <typename T, template <typename...> typename Template>
struct IsInstanceOfTemplate;

}  // namespace util::tpl::impl
