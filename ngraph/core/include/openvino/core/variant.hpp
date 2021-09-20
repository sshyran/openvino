// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <map>
#include <string>

#include "ngraph/output_vector.hpp"
#include "ngraph/type.hpp"
#include "openvino/core/core_visibility.hpp"

namespace ov {
class Node;
using VariantTypeInfo = ngraph::DiscreteTypeInfo;

class OPENVINO_API Variant {
public:
    virtual ~Variant();
    virtual const VariantTypeInfo& get_type_info() const = 0;

    virtual bool is_copyable() const;
    virtual std::shared_ptr<ov::Variant> init(const std::shared_ptr<Node>& node);
    virtual std::shared_ptr<ov::Variant> merge(const ngraph::NodeVector& nodes);
    virtual std::string to_string() {
        return "";
    }
};

template <typename VT>
class VariantImpl : public Variant {
public:
    using value_type = VT;

    VariantImpl(const value_type& value) : m_value(value) {}

    const value_type& get() const {
        return m_value;
    }
    value_type& get() {
        return m_value;
    }
    void set(const value_type& value) {
        m_value = value;
    }

protected:
    value_type m_value;
};

extern template class OPENVINO_API VariantImpl<std::string>;
extern template class OPENVINO_API VariantImpl<int64_t>;

template <typename VT>
class VariantWrapper {};

template <>
class OPENVINO_API VariantWrapper<std::string> : public VariantImpl<std::string> {
public:
    static constexpr VariantTypeInfo type_info{"Variant::std::string", 0};
    const VariantTypeInfo& get_type_info() const override {
        return type_info;
    }
    VariantWrapper(const value_type& value) : VariantImpl<value_type>(value) {}
};

template <>
class OPENVINO_API VariantWrapper<int64_t> : public VariantImpl<int64_t> {
public:
    static constexpr VariantTypeInfo type_info{"Variant::int64_t", 0};
    const VariantTypeInfo& get_type_info() const override {
        return type_info;
    }
    VariantWrapper(const value_type& value) : VariantImpl<value_type>(value) {}
};

template <typename T>
inline std::shared_ptr<Variant> make_variant(const T& p) {
    return std::dynamic_pointer_cast<VariantImpl<T>>(std::make_shared<VariantWrapper<T>>(p));
}

template <size_t N>
inline std::shared_ptr<Variant> make_variant(const char (&s)[N]) {
    return std::dynamic_pointer_cast<VariantImpl<std::string>>(std::make_shared<VariantWrapper<std::string>>(s));
}

#if defined(ENABLE_UNICODE_PATH_SUPPORT) && defined(_WIN32)
template <size_t N>
inline std::shared_ptr<Variant> make_variant(const wchar_t (&s)[N]) {
    return std::dynamic_pointer_cast<VariantImpl<std::wstring>>(std::make_shared<VariantWrapper<std::wstring>>(s));
}
#endif

using RTMap = std::map<std::string, std::shared_ptr<Variant>>;
using VariantVector = std::vector<std::shared_ptr<Variant>>;
}  // namespace ov