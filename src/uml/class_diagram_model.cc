/**
 * src/uml/class_diagram_model.cc
 *
 * Copyright (c) 2021 Bartek Kryza <bkryza@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "class_diagram_model.h"

namespace clanguml {
namespace model {
namespace class_diagram {
std::atomic_uint64_t element::m_nextId = 1;

std::string to_string(relationship_t r)
{
    switch (r) {
    case relationship_t::kNone:
        return "none";
    case relationship_t::kExtension:
        return "extension";
    case relationship_t::kComposition:
        return "composition";
    case relationship_t::kAggregation:
        return "aggregation";
    case relationship_t::kContainment:
        return "containment";
    case relationship_t::kOwnership:
        return "ownership";
    case relationship_t::kAssociation:
        return "association";
    case relationship_t::kInstantiation:
        return "instantiation";
    case relationship_t::kFriendship:
        return "frendship";
    case relationship_t::kDependency:
        return "dependency";
    default:
        return "invalid";
    }
}

//
// decorated_element
//

bool decorated_element::skip() const
{
    for (auto d : decorators)
        if (std::dynamic_pointer_cast<decorators::skip>(d))
            return true;

    return false;
}

bool decorated_element::skip_relationship() const
{
    for (auto d : decorators)
        if (std::dynamic_pointer_cast<decorators::skip_relationship>(d))
            return true;

    return false;
}

std::pair<relationship_t, std::string> decorated_element::relationship() const
{
    for (auto &d : decorators)
        if (std::dynamic_pointer_cast<decorators::association>(d))
            return {relationship_t::kAssociation,
                std::dynamic_pointer_cast<decorators::relationship>(d)
                    ->multiplicity};
        else if (std::dynamic_pointer_cast<decorators::aggregation>(d))
            return {relationship_t::kAggregation,
                std::dynamic_pointer_cast<decorators::relationship>(d)
                    ->multiplicity};
        else if (std::dynamic_pointer_cast<decorators::composition>(d))
            return {relationship_t::kComposition,
                std::dynamic_pointer_cast<decorators::relationship>(d)
                    ->multiplicity};

    return {relationship_t::kNone, ""};
}

std::string decorated_element::style_spec()
{
    for (auto d : decorators)
        if (std::dynamic_pointer_cast<decorators::style>(d))
            return std::dynamic_pointer_cast<decorators::style>(d)->spec;

    return "";
}

//
// element
//

element::element(const std::vector<std::string> &using_namespaces)
    : using_namespaces_{using_namespaces}
    , m_id{m_nextId++}
{
}

std::string element::alias() const { return fmt::format("C_{:010}", m_id); }

void element::add_relationship(class_relationship &&cr)
{
    if (cr.destination.empty()) {
        LOG_WARN("Skipping relationship '{}' - {} - '{}' due empty "
                 "destination",
            cr.destination, to_string(cr.type), full_name(true));
        return;
    }

    auto it = std::find(relationships_.begin(), relationships_.end(), cr);
    if (it == relationships_.end())
        relationships_.emplace_back(std::move(cr));
}

void element::set_using_namespaces(const std::vector<std::string> &un)
{
    using_namespaces_ = un;
}

const std::vector<std::string> &element::using_namespaces() const
{
    return using_namespaces_;
}

std::vector<class_relationship> &element::relationships()
{
    return relationships_;
}

const std::vector<class_relationship> &element::relationships() const
{
    return relationships_;
}

//
// method_parameter
//

std::string method_parameter::to_string(
    const std::vector<std::string> &using_namespaces) const
{
    using namespace clanguml::util;
    auto t = ns_relative(using_namespaces, type);
    if (default_value.empty())
        return fmt::format("{} {}", t, name);

    return fmt::format("{} {} = {}", t, name, default_value);
}

//
// class_relationship
//

bool operator==(const class_relationship &l, const class_relationship &r)
{
    return l.type == r.type && l.destination == r.destination &&
        l.label == r.label;
}

//
// class_template
//

bool operator==(const class_template &l, const class_template &r)
{
    return (l.name == r.name) && (l.type == r.type);
}

//
// class_
//

bool operator==(const class_ &l, const class_ &r)
{
    return l.full_name() == r.full_name();
}

void class_::add_type_alias(type_alias &&ta)
{
    LOG_DBG("Adding class alias: {} -> {}", ta.alias, ta.underlying_type);
    type_aliases[ta.alias] = std::move(ta);
}

std::string class_::full_name(bool relative) const
{
    using namespace clanguml::util;

    std::ostringstream ostr;
    if (relative)
        ostr << ns_relative(using_namespaces(), name());
    else
        ostr << name();

    if (!templates.empty()) {
        std::vector<std::string> tnames;
        std::transform(templates.cbegin(), templates.cend(),
            std::back_inserter(tnames), [this](const auto &tmplt) {
                std::vector<std::string> res;

                if (!tmplt.type.empty())
                    res.push_back(ns_relative(using_namespaces(), tmplt.type));

                if (!tmplt.name.empty())
                    res.push_back(ns_relative(using_namespaces(), tmplt.name));

                if (!tmplt.default_value.empty()) {
                    res.push_back("=");
                    res.push_back(tmplt.default_value);
                }

                return fmt::format("{}", fmt::join(res, " "));
            });
        ostr << fmt::format("<{}>", fmt::join(tnames, ","));
    }

    return ostr.str();
}

bool class_::is_abstract() const
{
    // TODO check if all base abstract methods are overriden
    // with non-abstract methods
    return std::any_of(methods.begin(), methods.end(),
        [](const auto &method) { return method.is_pure_virtual; });
}

//
// enum_
//

bool operator==(const enum_ &l, const enum_ &r) { return l.name() == r.name(); }

std::string enum_::full_name(bool relative) const
{
    using namespace clanguml::util;

    std::ostringstream ostr;
    if (relative)
        ostr << ns_relative(using_namespaces(), name());
    else
        ostr << name();

    return ostr.str();
}

//
// diagram
//
std::string diagram::name() const { return name_; }

void diagram::set_name(const std::string &name) { name_ = name; }

const std::vector<class_> diagram::classes() const { return classes_; }

const std::vector<enum_> diagram::enums() const { return enums_; }

bool diagram::has_class(const class_ &c) const
{
    return std::any_of(classes_.cbegin(), classes_.cend(),
        [&c](const auto &cc) { return cc.full_name() == c.full_name(); });
}

void diagram::add_type_alias(type_alias &&ta)
{
    LOG_DBG("Adding global alias: {} -> {}", ta.alias, ta.underlying_type);

    type_aliases_[ta.alias] = std::move(ta);
}

void diagram::add_class(class_ &&c)
{
    LOG_DBG("Adding class: {}, {}", c.name(), c.full_name());
    if (!has_class(c))
        classes_.emplace_back(std::move(c));
    else
        LOG_DBG("Class {} ({}) already in the model", c.name(), c.full_name());
}

void diagram::add_enum(enum_ &&e)
{
    LOG_DBG("Adding enum: {}", e.name());
    auto it = std::find(enums_.begin(), enums_.end(), e);
    if (it == enums_.end())
        enums_.emplace_back(std::move(e));
    else
        LOG_DBG("Enum {} already in the model", e.name());
}

std::string diagram::to_alias(const std::string &full_name) const
{
    LOG_DBG("Looking for alias for {}", full_name);

    for (const auto &c : classes_) {
        if (c.full_name() == full_name) {
            return c.alias();
        }
    }

    for (const auto &e : enums_) {
        if (e.full_name() == full_name) {
            return e.alias();
        }
    }

    throw error::uml_alias_missing(
        fmt::format("Missing alias for {}", full_name));
}

}
}
}
