/**
 * @file tests/test_cases.h
 *
 * Copyright (c) 2021-2024 Bartek Kryza <bkryza@gmail.com>
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
#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include "class_diagram/generators/mermaid/class_diagram_generator.h"
#include "class_diagram/generators/plantuml/class_diagram_generator.h"
#include "class_diagram/model/diagram.h"
#include "class_diagram/visitor/translation_unit_visitor.h"
#include "common/clang_utils.h"
#include "common/compilation_database.h"
#include "common/generators/generators.h"
#include "config/config.h"
#include "include_diagram/generators/plantuml/include_diagram_generator.h"
#include "include_diagram/visitor/translation_unit_visitor.h"
#include "package_diagram/generators/plantuml/package_diagram_generator.h"
#include "package_diagram/visitor/translation_unit_visitor.h"
#include "sequence_diagram/generators/plantuml/sequence_diagram_generator.h"
#include "sequence_diagram/visitor/translation_unit_visitor.h"
#include "util/util.h"

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"

#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>

#include <algorithm>
#include <complex>
#include <filesystem>
#include <string>

using namespace clanguml::util;

std::pair<clanguml::config::config, clanguml::common::compilation_database_ptr>
load_config(const std::string &test_name);

void save_puml(const std::string &path, const std::string &puml);

namespace clanguml::test {

template <typename T, typename... Ts> constexpr bool has_type() noexcept
{
    return (std::is_same_v<T, Ts> || ... || false);
}

struct Public { };

struct Protected { };

struct Private { };

struct Abstract { };

struct Static { };

struct Const { };

struct Constexpr { };

struct Consteval { };

struct Coroutine { };

struct Noexcept { };

struct Default { };

struct Deleted { };

struct Entrypoint { };

struct Exitpoint { };

struct InControlCondition { };
struct Response { };
struct NamespacePackage { };
struct ModulePackage { };
struct DirectoryPackage { };

template <typename PackageT> std::string package_type_name();

template <> std::string package_type_name<NamespacePackage>()
{
    return "namespace";
}

template <> std::string package_type_name<ModulePackage>() { return "module"; }

template <> std::string package_type_name<DirectoryPackage>()
{
    return "directory";
}

template <typename T> struct diagram_source_t {
    diagram_source_t(clanguml::common::model::diagram_t dt, T &&s)
    {
        diagram_type = dt;
        src = std::move(s);
    }

    bool contains(std::string name) const;

    virtual std::string get_alias(std::string name) const
    {
        return "__INVALID_ALIAS__";
    }

    virtual std::string get_alias(std::string ns, std::string name) const
    {
        return get_alias(fmt::format("{}::{}", ns, name));
    }

    bool search(const std::string &pattern) const;

    int64_t find(const std::string &pattern, int64_t offset = 0) const;

    std::string to_string() const;

    T src;
    clanguml::common::model::diagram_t diagram_type;
    bool generate_packages{false};
};

struct plantuml_t : public diagram_source_t<std::string> {
    using diagram_source_t::diagram_source_t;
    using source_type = std::string;
    using generator_tag = clanguml::common::generators::plantuml_generator_tag;

    inline static const std::string diagram_type_name{"PlantUML"};

    std::string get_alias(std::string name) const override
    {
        std::vector<std::regex> patterns;

        const std::string alias_regex("([A-Z]_[0-9]+)");

        util::replace_all(name, "(", "\\(");
        util::replace_all(name, ")", "\\)");
        util::replace_all(name, " ", "\\s");
        util::replace_all(name, "*", "\\*");
        util::replace_all(name, "[", "\\[");
        util::replace_all(name, "]", "\\]");

        patterns.push_back(
            std::regex{"class\\s\"" + name + "\"\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"abstract\\s\"" + name + "\"\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"enum\\s\"" + name + "\"\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"package\\s\"" + name + "\"\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"package\\s\\[" + name + "\\]\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"file\\s\"" + name + "\"\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"folder\\s\"" + name + "\"\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"participant\\s\"" + name + "\"\\sas\\s" + alias_regex});

        std::smatch base_match;

        for (const auto &pattern : patterns) {
            if (std::regex_search(src, base_match, pattern) &&
                base_match.size() == 2) {
                std::ssub_match base_sub_match = base_match[1];
                std::string alias = base_sub_match.str();
                return trim(alias);
            }
        }

        return fmt::format("__INVALID__ALIAS__({})", name);
    }
};

struct mermaid_t : public diagram_source_t<std::string> {
    using diagram_source_t::diagram_source_t;
    using source_type = std::string;
    using generator_tag = clanguml::common::generators::mermaid_generator_tag;

    inline static const std::string diagram_type_name{"MermaidJS"};

    std::string get_alias_impl(std::string name) const
    {
        std::vector<std::regex> patterns;

        const std::string alias_regex("([A-Z]_[0-9]+)");

        util::replace_all(name, "(", "&lpar;");
        util::replace_all(name, ")", "&rpar;");
        util::replace_all(name, " ", "\\s");
        util::replace_all(name, "*", "\\*");
        util::replace_all(name, "[", "\\[");
        util::replace_all(name, "]", "\\]");
        util::replace_all(name, "<", "&lt;");
        util::replace_all(name, ">", "&gt;");

        patterns.push_back(
            std::regex{"class\\s" + alias_regex + "\\[\"" + name + "\"\\]"});
        patterns.push_back(
            std::regex{"subgraph\\s" + alias_regex + "\\[" + name + "\\]"});
        patterns.push_back(
            std::regex{"\\s\\s" + alias_regex + "\\[" + name + "\\]"}); // file

        std::smatch base_match;

        for (const auto &pattern : patterns) {
            if (std::regex_search(src, base_match, pattern) &&
                base_match.size() == 2) {
                std::ssub_match base_sub_match = base_match[1];
                std::string alias = base_sub_match.str();
                return trim(alias);
            }
        }

        return fmt::format("__INVALID__ALIAS__({})", name);
    }

    std::string get_alias_sequence_diagram_impl(std::string name) const
    {
        std::vector<std::regex> patterns;

        const std::string alias_regex("([A-Z]_[0-9]+)");

        util::replace_all(name, "(", "\\(");
        util::replace_all(name, ")", "\\)");
        util::replace_all(name, " ", "\\s");
        util::replace_all(name, "*", "\\*");
        util::replace_all(name, "[", "\\[");
        util::replace_all(name, "]", "\\]");

        patterns.push_back(
            std::regex{"participant\\s" + alias_regex + "\\sas\\s" + name+"\\n"});

        std::smatch base_match;

        for (const auto &pattern : patterns) {
            if (std::regex_search(src, base_match, pattern) &&
                base_match.size() == 2) {
                std::ssub_match base_sub_match = base_match[1];
                std::string alias = base_sub_match.str();
                return trim(alias);
            }
        }

        return fmt::format("__INVALID__ALIAS__({})", name);
    }

    std::string get_alias(std::string name) const override
    {
        if (diagram_type == common::model::diagram_t::kSequence)
            return get_alias_sequence_diagram_impl(name);

        return get_alias_impl(name);
    }
};

struct json_t : public diagram_source_t<nlohmann::json> {
    using diagram_source_t::diagram_source_t;
    using source_type = nlohmann::json;
    using generator_tag = clanguml::common::generators::json_generator_tag;

    inline static const std::string diagram_type_name{"JSON"};
};

std::optional<nlohmann::json> get_element_by_id(
    const nlohmann::json &j, const std::string &id)
{
    if (!j.contains("elements"))
        return {};

    for (const nlohmann::json &e : j["elements"]) {
        if (e["id"] == id)
            return {e};

        if (e["type"] == "namespace" || e["type"] == "folder") {
            auto maybe_e = get_element_by_id(e, id);
            if (maybe_e)
                return maybe_e;
        }
    }

    return {};
}

std::optional<nlohmann::json> get_element(
    const nlohmann::json &j, const std::string &name)
{
    if (!j.contains("elements"))
        return {};

    for (const nlohmann::json &e : j["elements"]) {
        if (e["display_name"] == name)
            return {e};

        if (e["type"] == "namespace" || e["type"] == "folder" ||
            e["type"] == "directory" || e["type"] == "module") {
            auto maybe_e = get_element(e, name);
            if (maybe_e)
                return maybe_e;
        }
    }

    return {};
}

std::optional<nlohmann::json> get_element(
    const json_t &src, const std::string &name)
{
    return get_element(src.src, name);
}

std::optional<nlohmann::json> get_participant(
    const nlohmann::json &j, const std::string &name)
{
    if (!j.contains("participants"))
        return {};

    for (const nlohmann::json &e : j.at("participants")) {
        if (e["display_name"] == name)
            return {e};
    }

    return {};
}

auto get_relationship(const nlohmann::json &j, const nlohmann::json &from,
    const nlohmann::json &to, const std::string &type, const std::string &label)
{
    return std::find_if(j["relationships"].begin(), j["relationships"].end(),
        [&](const auto &it) {
            auto match = (it["source"] == from) && (it["destination"] == to) &&
                (it["type"] == type);

            if (match && label.empty())
                return true;

            if (match && (label == it["label"]))
                return true;

            return false;
        });
}

auto get_relationship(const nlohmann::json &j, const std::string &from,
    const std::string &to, const std::string &type,
    const std::string &label = {})
{
    auto source = get_element(j, from);
    auto destination = get_element(j, to);

    if (!(source && destination))
        return j["relationships"].end();

    return get_relationship(
        j, source->at("id"), destination->at("id"), type, label);
}

std::string expand_name(const nlohmann::json &j, const std::string &name)
{
    return name;
}

template <> bool diagram_source_t<std::string>::contains(std::string name) const
{
    return util::contains(src, name);
}

template <>
int64_t diagram_source_t<std::string>::find(
    const std::string &pattern, int64_t offset) const
{
    std::regex pattern_regex{pattern};

    std::smatch base_match;
    auto offset_it = src.begin();
    std::advance(offset_it, offset);
    bool found =
        std::regex_search(offset_it, src.end(), base_match, pattern_regex);
    if (!found)
        return -1;

    return base_match.position(0);
}

template <>
bool diagram_source_t<std::string>::search(const std::string &pattern) const
{
    return find(pattern) > -1;
}

template <>
bool diagram_source_t<nlohmann::json>::contains(std::string name) const
{
    return false;
}

template <> std::string diagram_source_t<std::string>::to_string() const
{
    return src;
}

template <> std::string diagram_source_t<nlohmann::json>::to_string() const
{
    return src.dump(2);
}

struct QualifiedName {
    QualifiedName(const char *n)
        : name{n}
    {
    }

    QualifiedName(std::string_view n)
        : name{n}
    {
    }

    QualifiedName(std::string_view ns_, std::string_view n)
        : ns{ns_}
        , name{n}
    {
    }

    QualifiedName(const char *ns_, const char *n)
        : ns{ns_}
        , name{n}
    {
    }

    operator std::string() const { return str(); }

    std::string str(bool generate_packages = false) const
    {
        if (ns && !generate_packages)
            return fmt::format("{}::{}", ns.value(), name);

        return name;
    }

    std::optional<std::string> ns;
    std::string name;
};

struct Message {
    template <typename... Attrs>
    Message(QualifiedName f, QualifiedName t, std::string m, Attrs &&...attrs)
        : from{std::move(f)}
        , to{std::move(t)}
        , message{std::move(m)}
        , is_static{has_type<Static, Attrs...>()}
        , is_incontrolcondition{has_type<InControlCondition, Attrs...>()}
        , is_response{has_type<Response, Attrs...>()}
    {
    }

    template <typename... Attrs>
    Message(Entrypoint &&e, QualifiedName t, std::string m, Attrs &&...attrs)
        : Message(QualifiedName{""}, std::move(t), {},
              std::forward<Attrs>(attrs)...)
    {
        is_entrypoint = true;
    }

    template <typename... Attrs>
    Message(Exitpoint &&e, QualifiedName t, Attrs &&...attrs)
        : Message(QualifiedName{""}, std::move(t), {},
              std::forward<Attrs>(attrs)...)
    {
        is_exitpoint = true;
    }

    QualifiedName from;
    QualifiedName to;
    std::string message;
    std::optional<std::string> return_type;

    bool is_static{false};
    bool is_entrypoint{false};
    bool is_exitpoint{false};
    bool is_incontrolcondition{false};
    bool is_response{false};
};

///
/// The following functions declarations define various checks on generated
/// diagrams.
/// They must be specialized for each diagram format (DiagramType) separately.
///
/// @defgroup Test Cases diagram checks
/// @{
///

// Check if generated diagram source starts with pattern
template <typename DiagramType>
bool StartsWith(const DiagramType &d, std::string pattern);

// Check if generated diagram source ends with pattern
template <typename DiagramType>
bool EndsWith(const DiagramType &d, std::string pattern);

template <typename DiagramType>
bool HasTitle(const DiagramType &d, std::string const &str);

// Check if generated diagram contains a specified enum
template <typename DiagramType>
bool IsEnum(const DiagramType &d, QualifiedName name);

// Check if generated diagram contains a specified union
template <typename DiagramType>
bool IsUnion(const DiagramType &d, QualifiedName name);

// Check if generated diagram contains a specified class
template <typename DiagramType>
bool IsClass(const DiagramType &d, QualifiedName name);

// Check if generated diagram contains a specified class template
template <typename DiagramType>
bool IsClassTemplate(const DiagramType &d, QualifiedName name);

template <typename DiagramType>
bool IsAbstractClassTemplate(const DiagramType &d, QualifiedName name);

// Check if generated diagram contains a specified abstract class
template <typename DiagramType>
bool IsAbstractClass(const DiagramType &d, QualifiedName name);

// Check if generated diagram contains a specified class
template <typename DiagramType>
bool IsBaseClass(
    const DiagramType &d, QualifiedName base, QualifiedName subclass);

template <typename DiagramType>
bool IsInnerClass(
    const DiagramType &d, std::string const &parent, std::string const &inner);

template <typename DiagramType, typename... Ts>
bool IsMethod(const DiagramType &d, const std::string &cls,
    const std::string &name, const std::string &type = "void",
    const std::string &params = "");

template <typename DiagramType, typename... Ts>
bool IsField(const DiagramType &d, QualifiedName cls, std::string const &name,
    std::string type = "void");

template <typename DiagramType, typename... Ts>
bool IsAssociation(const DiagramType &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "");

template <typename DiagramType, typename... Ts>
bool IsComposition(const DiagramType &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "");

template <typename DiagramType, typename... Ts>
bool IsAggregation(const DiagramType &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "");

template <typename DiagramType>
bool IsInstantiation(const DiagramType &d, std::string const &from,
    std::string const &to, std::string style = "");

template <typename DiagramType>
bool IsDependency(const DiagramType &d, QualifiedName from, QualifiedName to,
    std::string style = "");

template <typename DiagramType, typename... Ts>
bool IsFriend(
    const DiagramType &d, std::string const &from, std::string const &to);

template <typename DiagramType>
bool IsPackageDependency(
    const DiagramType &d, std::string const &from, std::string const &to);

template <typename DiagramType>
bool IsIncludeDependency(
    const DiagramType &d, std::string const &from, std::string const &to);

template <typename DiagramType>
bool IsConstraint(const DiagramType &d, QualifiedName from, QualifiedName to,
    std::string label = {}, std::string style = "");

template <typename DiagramType>
bool IsConcept(const DiagramType &d, QualifiedName cpt);

template <typename DiagramType>
bool IsConceptParameterList(
    const DiagramType &d, std::string const &cpt, std::string param);

template <typename DiagramType>
bool IsConceptRequirement(
    const DiagramType &d, std::string const &cpt, std::string requirement);

template <typename DiagramType>
bool IsLayoutHint(const DiagramType &d, std::string const &from,
    std::string const &hint, std::string const &to);

template <typename DiagramType>
bool HasComment(const DiagramType &d, std::string const &comment);

template <typename DiagramType>
bool HasNote(const DiagramType &d, std::string const &cls,
    std::string const &position, std::string const &note = "");

template <typename DiagramType>
bool HasPackageNote(const DiagramType &d, std::string const &cls,
    std::string const &position, std::string const &note = "");

template <typename DiagramType>
bool HasMessageComment(
    const DiagramType &d, std::string const &alias, std::string const &note);

template <typename DiagramType>
bool HasMemberNote(const DiagramType &d, std::string const &cls,
    std::string const &member, std::string const &position,
    std::string const &note = "");

template <typename DiagramType>
bool HasLink(const DiagramType &d, std::string const &alias,
    std::string const &link, std::string const &tooltip);

template <typename DiagramType>
bool HasMemberLink(const DiagramType &d, std::string const &method,
    std::string const &link, std::string const &tooltip);

template <typename DiagramType>
bool IsFolder(const DiagramType &d, std::string const &path);

template <typename DiagramType>
bool IsFile(const DiagramType &d, std::string const &str);

template <typename DiagramType>
bool IsSystemHeader(const DiagramType &d, std::string const &str);

template <typename DiagramType>
bool IsHeaderDependency(const DiagramType &d, std::string const &from,
    std::string const &to, std::string style = "");

template <typename DiagramType>
bool IsSystemHeaderDependency(const DiagramType &d, std::string const &from,
    std::string const &to, std::string style = "");

template <typename DiagramType, typename... Args>
bool IsNamespacePackage(const DiagramType &d, Args... args);

template <typename DiagramType, typename... Args>
bool IsDirectoryPackage(const DiagramType &d, Args... args);

template <typename DiagramType, typename... Args>
bool IsModulePackage(const DiagramType &d, Args... args);

template <typename DiagramType>
bool IsDeprecated(const DiagramType &d, std::string const &str);

template <typename DiagramType>
int64_t FindMessage(const DiagramType &d, const Message &msg,
    int64_t offset = 0, bool fail = true);

template <typename DiagramType>
bool HasMessage(const DiagramType &d, const Message &msg)
{
    return FindMessage(d, msg, 0, false) >= 0;
}

template <typename DiagramType>
bool MessageOrder(const DiagramType &d, std::vector<Message> messages)
{
    std::vector<int64_t> order;
    int64_t offset{0};
    order.reserve(messages.size());
    std::transform(messages.begin(), messages.end(), std::back_inserter(order),
        [&d, &offset](const auto &m) {
            offset = FindMessage(d, m, offset);
            return offset;
        });
    bool are_messages_in_order = std::is_sorted(order.begin(), order.end());

    if (!are_messages_in_order) {
        FAIL(fmt::format(
            "Messages are not in order: \n[{}]", fmt::join(order, ",\n")));
        return false;
    }

    return true;
}

template <typename DiagramType>
bool MessageChainsOrder(
    const DiagramType &d, std::vector<std::vector<Message>> message_chains)
{
    std::vector<Message> flattenned;
    for (const auto &mc : message_chains) {
        for (const auto &m : mc)
            flattenned.emplace_back(m);
    }

    return MessageOrder(d, std::move(flattenned));
}

template <typename DiagramType>
bool IsParticipant(
    const DiagramType &d, const std::string &name, const std::string &type);

template <typename DiagramType>
bool IsFunctionParticipant(const DiagramType &d, const std::string &name)
{
    return IsParticipant(d, name, "function");
}

template <typename DiagramType>
bool IsFunctionTemplateParticipant(
    const DiagramType &d, const std::string &name)
{
    return IsParticipant(d, name, "function_template");
}

template <typename DiagramType>
bool IsClassParticipant(const DiagramType &d, const std::string &name)
{
    return IsParticipant(d, name, "class");
}

template <typename DiagramType>
bool IsFileParticipant(const DiagramType &d, const std::string &name)
{
    return IsParticipant(d, name, "file");
}

///
/// @}
///

template <> bool StartsWith(const plantuml_t &d, std::string pattern)
{
    return util::starts_with(d.src, pattern);
}

template <> bool EndsWith(const plantuml_t &d, std::string pattern)
{
    return util::ends_with(d.src, pattern);
}

template <> bool HasTitle(const plantuml_t &d, std::string const &str)
{
    return d.contains("title " + str);
}

template <> bool IsEnum(const plantuml_t &d, QualifiedName enm)
{
    return d.contains(
        fmt::format("enum {}", d.get_alias(enm.str(d.generate_packages))));
}

template <> bool IsUnion(const plantuml_t &d, QualifiedName cls)
{
    return d.contains(fmt::format(
        "class {} <<union>>", d.get_alias(cls.str(d.generate_packages))));
}

template <> bool IsClass(const plantuml_t &d, QualifiedName cls)
{
    return d.contains(
        fmt::format("class {}", d.get_alias(cls.str(d.generate_packages))));
}

template <> bool IsClassTemplate(const plantuml_t &d, QualifiedName cls)
{
    return d.contains(
        fmt::format("class \"{}\"", cls.str(d.generate_packages)));
}

template <> bool IsAbstractClassTemplate(const plantuml_t &d, QualifiedName cls)
{
    return d.contains(
        fmt::format("abstract \"{}\"", cls.str(d.generate_packages)));
}

template <> bool IsAbstractClass(const plantuml_t &d, QualifiedName cls)
{
    return d.contains(
        fmt::format("abstract {}", d.get_alias(cls.str(d.generate_packages))));
}

template <>
bool IsBaseClass(
    const plantuml_t &d, QualifiedName base, QualifiedName subclass)
{
    return d.contains(
        fmt::format("{} <|-- {}", d.get_alias(base.str(d.generate_packages)),
            d.get_alias(subclass.str(d.generate_packages))));
}

template <>
bool IsInnerClass(
    const plantuml_t &d, std::string const &parent, std::string const &inner)
{
    return d.contains(d.get_alias(inner) + " --+ " + d.get_alias(parent));
}

template <typename... Ts>
bool IsMethod(const plantuml_t &d, std::string const &cls,
    std::string const &name, std::string const &type = "void",
    std::string const &params = "")
{
    std::string pattern;
    if constexpr (has_type<Static, Ts...>())
        pattern += "{static} ";

    if constexpr (has_type<Abstract, Ts...>())
        pattern += "{abstract} ";

    if constexpr (has_type<Public, Ts...>())
        pattern = "+";
    else if constexpr (has_type<Protected, Ts...>())
        pattern = "#";
    else
        pattern = "-";

    pattern += name;

    pattern += "(" + params + ")";

    if constexpr (has_type<Constexpr, Ts...>())
        pattern += " constexpr";

    if constexpr (has_type<Consteval, Ts...>())
        pattern += " consteval";

    if constexpr (has_type<Const, Ts...>())
        pattern += " const";

    if constexpr (has_type<Abstract, Ts...>())
        pattern += " = 0";

    if constexpr (has_type<Default, Ts...>())
        pattern += " = default";

    if constexpr (has_type<Deleted, Ts...>())
        pattern += " = deleted";

    if constexpr (has_type<Coroutine, Ts...>())
        pattern += " [coroutine]";

    pattern += " : " + type;

    return d.contains(pattern);
}

template <typename... Ts>
bool IsField(const plantuml_t &d, QualifiedName cls, std::string const &name,
    std::string type)
{
    std::string pattern;
    if constexpr (has_type<Static, Ts...>())
        pattern += "{static} ";

    if constexpr (has_type<Public, Ts...>())
        pattern = "+";
    else if constexpr (has_type<Protected, Ts...>())
        pattern = "#";
    else
        pattern = "-";

    pattern += name;

    return d.contains(pattern + " : " + type);
}

template <typename... Ts>
bool IsAssociation(const plantuml_t &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "")
{
    auto from_id = d.get_alias(from);
    auto to_id = d.get_alias(to);

    std::string format_string = "{}";
    if (!multiplicity_source.empty())
        format_string += " \"" + multiplicity_source + "\"";

    format_string += fmt::format(" -{}->", style);

    if (!multiplicity_dest.empty())
        format_string += " \"" + multiplicity_dest + "\"";

    format_string += " {}";

    if (!label.empty()) {
        std::string label_prefix;
        if constexpr (has_type<Public, Ts...>())
            label_prefix = "+";
        else if constexpr (has_type<Protected, Ts...>())
            label_prefix = "#";
        else
            label_prefix = "-";

        format_string += " : {}{}";
        return d.contains(fmt::format(
            fmt::runtime(format_string), from_id, to_id, label_prefix, label));
    }

    return d.contains(fmt::format(fmt::runtime(format_string), from_id, to_id));
}

template <typename... Ts>
bool IsComposition(const plantuml_t &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "")
{
    std::string label_prefix;
    if constexpr (has_type<Public, Ts...>())
        label_prefix = "+";
    else if constexpr (has_type<Protected, Ts...>())
        label_prefix = "#";
    else
        label_prefix = "-";

    std::string format_string = "{}";
    if (!multiplicity_source.empty())
        format_string += " \"" + multiplicity_source + "\"";

    format_string += fmt::format(" *-{}-", style);

    if (!multiplicity_dest.empty())
        format_string += " \"" + multiplicity_dest + "\"";

    format_string += " {} : {}{}";

    return d.contains(fmt::format(fmt::runtime(format_string),
        d.get_alias(from), d.get_alias(to), label_prefix, label));
}

template <typename... Ts>
bool IsAggregation(const plantuml_t &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "")
{
    std::string label_prefix;
    if constexpr (has_type<Public, Ts...>())
        label_prefix = "+";
    else if constexpr (has_type<Protected, Ts...>())
        label_prefix = "#";
    else
        label_prefix = "-";

    std::string format_string = "{}";
    if (!multiplicity_source.empty())
        format_string += " \"" + multiplicity_source + "\"";

    format_string += fmt::format(" o-{}-", style);

    if (!multiplicity_dest.empty())
        format_string += " \"" + multiplicity_dest + "\"";

    format_string += " {} : {}{}";

    return d.contains(fmt::format(fmt::runtime(format_string),
        d.get_alias(from), d.get_alias(to), label_prefix, label));
}

template <>
bool IsInstantiation(const plantuml_t &d, std::string const &from,
    std::string const &to, std::string style)
{
    return d.contains(
        fmt::format("{} .{}.|> {}", d.get_alias(to), style, d.get_alias(from)));
}

template <>
bool IsDependency(const plantuml_t &d, QualifiedName from, QualifiedName to,
    std::string style)
{
    return d.contains(fmt::format(
               "{} .{}.> {}", d.get_alias(from), style, d.get_alias(to))) ||
        d.contains(fmt::format("{} .{}.> {}", d.get_alias(from.name), style,
            d.get_alias(to.name)));
}

template <typename... Ts>
bool IsFriend(
    const plantuml_t &d, std::string const &from, std::string const &to)
{
    std::string pattern;

    if constexpr (has_type<Public, Ts...>())
        pattern = "+";
    else if constexpr (has_type<Protected, Ts...>())
        pattern = "#";
    else
        pattern = "-";

    return d.contains(fmt::format("{} <.. {} : {}<<friend>>", d.get_alias(from),
        d.get_alias(to), pattern));
}

template <>
bool IsPackageDependency(
    const plantuml_t &d, std::string const &from, std::string const &to)
{
    return d.contains(
        fmt::format("{} .{}.> {}", d.get_alias(from), "", d.get_alias(to)));
}

template <>
bool IsIncludeDependency(
    const plantuml_t &d, std::string const &from, std::string const &to)
{
    return d.contains(
        fmt::format("{} .{}.> {}", d.get_alias(from), "", d.get_alias(to)));
}

template <>
bool IsConstraint(const plantuml_t &d, QualifiedName from, QualifiedName to,
    std::string label, std::string style)
{
    if (label.empty())
        return d.contains(fmt::format("{} .{}.> {}", d.get_alias(from.name),
            style, d.get_alias(to.name)));

    return d.contains(fmt::format("{} .{}.> {} : {}", d.get_alias(from.name),
        style, d.get_alias(to.name), label));
}

template <> bool IsConcept(const plantuml_t &d, QualifiedName cpt)
{
    return d.contains("class " + d.get_alias(cpt) + " <<concept>>") ||
        d.contains("class " + d.get_alias(cpt.name) + " <<concept>>");
}

template <>
bool IsConceptRequirement(
    const plantuml_t &d, std::string const &cpt, std::string requirement)
{
    return d.contains(requirement);
}

template <>
bool IsConceptParameterList(
    const plantuml_t &d, std::string const &cpt, std::string params)
{
    return d.contains(params);
}

template <>
bool IsLayoutHint(const plantuml_t &d, std::string const &from,
    std::string const &hint, std::string const &to)
{
    return d.contains(fmt::format(
        "{} -[hidden]{}- {}", d.get_alias(from), hint, d.get_alias(to)));
}

template <> bool HasComment(const plantuml_t &d, std::string const &comment)
{
    return d.contains(fmt::format("' {}", comment));
}

template <>
bool HasNote(const plantuml_t &d, std::string const &cls,
    std::string const &position, std::string const &note)
{
    return d.contains(fmt::format("note {} of {}", position, d.get_alias(cls)));
}

template <>
bool HasMemberNote(const plantuml_t &d, std::string const &cls,
    std::string const &member, std::string const &position,
    std::string const &note)
{
    return d.contains(
        fmt::format("note {} of {}::{}", position, d.get_alias(cls), member));
}

template <>
bool HasPackageNote(const plantuml_t &d, std::string const &cls,
    std::string const &position, std::string const &note)
{
    return d.contains(fmt::format("note {} of {}", position, d.get_alias(cls)));
}

template <>
bool HasLink(const plantuml_t &d, std::string const &element,
    std::string const &link, std::string const &tooltip)
{
    return d.contains(
        fmt::format("{} [[{}{{{}}}]]", d.get_alias(element), link, tooltip));
}

template <>
bool HasMemberLink(const plantuml_t &d, std::string const &method,
    std::string const &link, std::string const &tooltip)
{
    return d.contains(fmt::format("{} [[[{}{{{}}}]]]", method, link, tooltip));
}

template <> bool IsFolder(const plantuml_t &d, std::string const &path)
{
    return d.contains("folder \"" + util::split(path, "/").back() + "\"");
}

template <> bool IsFile(const plantuml_t &d, std::string const &path)
{
    return d.contains("file \"" + util::split(path, "/").back() + "\"");
}

template <> bool IsSystemHeader(const plantuml_t &d, std::string const &path)
{
    return d.contains("file \"" + path + "\"");
}

template <>
bool IsHeaderDependency(const plantuml_t &d, std::string const &from,
    std::string const &to, std::string style)
{
    assert(d.diagram_type == common::model::diagram_t::kInclude);

    return d.contains(
        fmt::format("{} --> {}", d.get_alias(util::split(from, "/").back()),
            d.get_alias(util::split(to, "/").back())));
}

template <>
bool IsSystemHeaderDependency(const plantuml_t &d, std::string const &from,
    std::string const &to, std::string style)
{
    assert(d.diagram_type == common::model::diagram_t::kInclude);

    return d.contains(
        fmt::format("{} ..> {}", d.get_alias(util::split(from, "/").back()),
            d.get_alias(util::split(to, "/").back())));
}

template <typename... Args> auto get_last(Args &&...args)
{
    return std::get<sizeof...(Args) - 1>(std::forward_as_tuple(args...));
}

template <typename... Args>
bool IsNamespacePackage(const plantuml_t &d, Args... args)
{
    const auto &name = get_last(args...);
    return d.contains("package [" + name + "]");
}

template <typename... Args>
bool IsDirectoryPackage(const plantuml_t &d, Args... args)
{
    const auto &name = get_last(args...);
    return d.contains("package [" + name + "]");
}

template <typename... Args>
bool IsModulePackage(const plantuml_t &d, Args... args)
{
    const auto &name = get_last(args...);
    return d.contains("package [" + name + "]");
}

template <> bool IsDeprecated(const plantuml_t &d, const std::string &name)
{
    return d.contains(d.get_alias(name) + " <<deprecated>> ");
}

template <>
int64_t FindMessage(
    const plantuml_t &d, const Message &msg, int64_t offset, bool fail)
{
    auto msg_str = msg.message;
    util::replace_all(msg_str, "(", "\\(");
    util::replace_all(msg_str, ")", "\\)");
    util::replace_all(msg_str, "*", "\\*");
    util::replace_all(msg_str, "[", "\\[");
    util::replace_all(msg_str, "]", "\\]");
    util::replace_all(msg_str, "+", "\\+");

    std::string style;
    if (msg.is_static)
        style = "__";

    std::string call_pattern{"__INVALID__"};

    if (msg.is_entrypoint) {
        call_pattern =
            fmt::format("\\[-> {} : {}", d.get_alias(msg.to), msg_str);
    }
    else if (msg.is_exitpoint) {
        call_pattern = fmt::format("\\[<-- {}", d.get_alias(msg.to));
    }
    else if (msg.is_incontrolcondition) {
        call_pattern = fmt::format(
            "{} {} {} "
            "(\\[\\[.*\\]\\] )?: \\*\\*\\[\\*\\*{}{}{}\\*\\*\\]\\*\\*",
            d.get_alias(msg.from), "->", d.get_alias(msg.to), style, msg_str,
            style);
    }
    else if (msg.is_response) {
        call_pattern = fmt::format("{} {} {} : //{}//", d.get_alias(msg.from),
            "-->", d.get_alias(msg.to), msg_str);
    }
    else {
        call_pattern = fmt::format("{} {} {} "
                                   "(\\[\\[.*\\]\\] )?: {}{}{}",
            d.get_alias(msg.from), "->", d.get_alias(msg.to), style, msg_str,
            style);
    }

    auto match_offset = d.find(call_pattern, offset);

    if (match_offset < 0) {
        if (fail)
            FAIL(fmt::format("Missing message: {} -> {} {} ({})",
                msg.from.str(), msg.to.str(), msg.message, call_pattern));
        return -1;
    }

    return match_offset + offset;
}

template <>
bool HasMessageComment(const plantuml_t &d, std::string const &participant,
    std::string const &note)
{
    std::string note_escaped{note};
    util::replace_all(note_escaped, "(", "\\(");
    util::replace_all(note_escaped, ")", "\\)");

    return d.search(fmt::format("note over {}\\n{}\\nend note",
        d.get_alias(participant), note_escaped));
}

template <>
bool IsParticipant(
    const plantuml_t &d, const std::string &name, const std::string &type)
{
    return d.contains(
        fmt::format("participant \"{}\" as ", name, d.get_alias(name)));
}

//
// MermaidJS test helpers
//

template <> bool HasTitle(const mermaid_t &d, std::string const &str)
{
    return d.contains("title: " + str);
}

template <> bool IsEnum(const mermaid_t &d, QualifiedName enm)
{
    return d.search(std::string("class ") + d.get_alias(enm) +
        " \\{\\n\\s+<<enumeration>>");
}

template <> bool IsUnion(const mermaid_t &d, QualifiedName cls)
{
    return d.search(
        std::string("class ") + d.get_alias(cls) + " \\{\\n\\s+<<union>>");
}

template <> bool IsClass(const mermaid_t &d, QualifiedName cls)
{
    return d.contains(fmt::format("class {}", d.get_alias(cls)));
}

template <> bool IsClassTemplate(const mermaid_t &d, QualifiedName cls)
{
    return d.contains(fmt::format("class {}", d.get_alias(cls)));
}

template <> bool IsAbstractClassTemplate(const mermaid_t &d, QualifiedName cls)
{
    return d.search(
        std::string("class ") + d.get_alias(cls) + " \\{\\n\\s+<<abstract>>");
}

template <> bool IsAbstractClass(const mermaid_t &d, QualifiedName name)
{
    return d.search(
        std::string("class ") + d.get_alias(name) + " \\{\\n\\s+<<abstract>>");
}

template <>
bool IsBaseClass(const mermaid_t &d, QualifiedName base, QualifiedName subclass)
{
    return d.contains(
        fmt::format("{} <|-- {}", d.get_alias(base), d.get_alias(subclass)));
}

template <>
bool IsInnerClass(
    const mermaid_t &d, std::string const &parent, std::string const &inner)
{
    return d.contains(d.get_alias(parent) + " ()-- " + d.get_alias(inner));
}

template <typename... Ts>
bool IsMethod(const mermaid_t &d, std::string const &cls,
    std::string const &name, std::string type = "void",
    std::string const &params = "")
{
    std::string pattern;

    if constexpr (has_type<Public, Ts...>())
        pattern = "+";
    else if constexpr (has_type<Protected, Ts...>())
        pattern = "#";
    else
        pattern = "-";

    pattern += name;

    pattern += "(" + params + ")";

    std::vector<std::string> method_mods;
    if constexpr (has_type<Default, Ts...>())
        method_mods.push_back("default");
    if constexpr (has_type<Const, Ts...>())
        method_mods.push_back("const");
    if constexpr (has_type<Constexpr, Ts...>())
        method_mods.push_back("constexpr");
    if constexpr (has_type<Consteval, Ts...>())
        method_mods.push_back("consteval");
    if constexpr (has_type<Coroutine, Ts...>())
        method_mods.push_back("coroutine");

    pattern += " : ";

    if (!method_mods.empty()) {
        pattern += fmt::format("[{}] ", fmt::join(method_mods, ","));
    }

    util::replace_all(type, "<", "&lt;");
    util::replace_all(type, ">", "&gt;");
    util::replace_all(type, "(", "&lpar;");
    util::replace_all(type, ")", "&rpar;");
    util::replace_all(type, "##", "::");
    util::replace_all(type, "{", "&lbrace;");
    util::replace_all(type, "}", "&rbrace;");

    pattern += type;

    if constexpr (has_type<Abstract, Ts...>())
        pattern += "*";

    if constexpr (has_type<Static, Ts...>())
        pattern += "$";

    return d.contains(pattern);
}

template <typename... Ts>
bool IsField(const mermaid_t &d, QualifiedName cls, std::string const &name,
    std::string type)
{
    std::string pattern;
    if constexpr (has_type<Static, Ts...>())
        pattern += "{static} ";

    if constexpr (has_type<Public, Ts...>())
        pattern = "+";
    else if constexpr (has_type<Protected, Ts...>())
        pattern = "#";
    else
        pattern = "-";

    pattern += name;

    util::replace_all(type, "<", "&lt;");
    util::replace_all(type, ">", "&gt;");
    util::replace_all(type, "(", "&lpar;");
    util::replace_all(type, ")", "&rpar;");
    util::replace_all(type, "##", "::");
    util::replace_all(type, "{", "&lbrace;");
    util::replace_all(type, "}", "&rbrace;");

    return d.contains(pattern + " : " + type);
}

template <typename... Ts>
bool IsAssociation(const mermaid_t &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "")
{
    auto from_id = d.get_alias(from);
    auto to_id = d.get_alias(to);

    std::string label_prefix;
    if constexpr (has_type<Public, Ts...>())
        label_prefix = "+";
    else if constexpr (has_type<Protected, Ts...>())
        label_prefix = "#";
    else
        label_prefix = "-";

    std::string format_string = "{}";
    if (!multiplicity_source.empty())
        format_string += " \"" + multiplicity_source + "\"";

    format_string += " -->";

    if (!multiplicity_dest.empty())
        format_string += " \"" + multiplicity_dest + "\"";

    format_string += " {}";

    if (!label.empty()) {
        format_string += " : {}{}";
        return d.contains(fmt::format(
            fmt::runtime(format_string), from_id, to_id, label_prefix, label));
    }

    return d.contains(fmt::format(fmt::runtime(format_string), from_id, to_id));
}

template <typename... Ts>
bool IsComposition(const mermaid_t &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "")
{
    std::string label_prefix;
    if constexpr (has_type<Public, Ts...>())
        label_prefix = "+";
    else if constexpr (has_type<Protected, Ts...>())
        label_prefix = "#";
    else
        label_prefix = "-";

    std::string format_string = "{}";
    if (!multiplicity_source.empty())
        format_string += " \"" + multiplicity_source + "\"";

    format_string += fmt::format(" *--", style);

    if (!multiplicity_dest.empty())
        format_string += " \"" + multiplicity_dest + "\"";

    format_string += " {} : {}{}";

    return d.contains(fmt::format(fmt::runtime(format_string),
        d.get_alias(from), d.get_alias(to), label_prefix, label));
}

template <typename... Ts>
bool IsAggregation(const mermaid_t &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "")
{
    std::string label_prefix;
    if constexpr (has_type<Public, Ts...>())
        label_prefix = "+";
    else if constexpr (has_type<Protected, Ts...>())
        label_prefix = "#";
    else
        label_prefix = "-";

    std::string format_string = "{}";
    if (!multiplicity_source.empty())
        format_string += " \"" + multiplicity_source + "\"";

    format_string += " o--";

    if (!multiplicity_dest.empty())
        format_string += " \"" + multiplicity_dest + "\"";

    format_string += " {} : {}{}";

    return d.contains(fmt::format(fmt::runtime(format_string),
        d.get_alias(from), d.get_alias(to), label_prefix, label));
}

template <>
bool IsInstantiation(const mermaid_t &d, std::string const &from,
    std::string const &to, std::string style)
{
    return d.contains(
        fmt::format("{} ..|> {}", d.get_alias(to), d.get_alias(from)));
}

template <>
bool IsDependency(
    const mermaid_t &d, QualifiedName from, QualifiedName to, std::string style)
{
    if (d.diagram_type == common::model::diagram_t::kClass) {
        return d.contains(
            fmt::format("{} ..> {}", d.get_alias(from), d.get_alias(to)));
    }

    return d.contains(
        fmt::format("{} -.-> {}", d.get_alias(from), d.get_alias(to)));
}

template <>
bool IsHeaderDependency(const mermaid_t &d, std::string const &from,
    std::string const &to, std::string style)
{
    assert(d.diagram_type == common::model::diagram_t::kInclude);

    return d.contains(
        fmt::format("{} --> {}", d.get_alias(util::split(from, "/").back()),
            d.get_alias(util::split(to, "/").back())));
}

template <>
bool IsSystemHeaderDependency(const mermaid_t &d, std::string const &from,
    std::string const &to, std::string style)
{
    assert(d.diagram_type == common::model::diagram_t::kInclude);

    return d.contains(
        fmt::format("{} -.-> {}", d.get_alias(util::split(from, "/").back()),
            d.get_alias(util::split(to, "/").back())));
}

template <typename... Ts>
bool IsFriend(
    const mermaid_t &d, std::string const &from, std::string const &to)
{
    std::string pattern;

    if constexpr (has_type<Public, Ts...>())
        pattern = "+";
    else if constexpr (has_type<Protected, Ts...>())
        pattern = "#";
    else
        pattern = "-";

    return d.contains(fmt::format(
        "{} <.. {} : {}[friend]", d.get_alias(from), d.get_alias(to), pattern));
}

template <>
bool IsPackageDependency(
    const mermaid_t &d, std::string const &from, std::string const &to)
{
    return d.contains(
        fmt::format("{} -.-> {}", d.get_alias(from), "", d.get_alias(to)));
}

template <>
bool IsIncludeDependency(
    const mermaid_t &d, std::string const &from, std::string const &to)
{
    return d.contains(
        fmt::format("{} -.-> {}", d.get_alias(from), "", d.get_alias(to)));
}

template <>
bool IsConstraint(const mermaid_t &d, QualifiedName from, QualifiedName to,
    std::string label, std::string style)
{
    auto from_id = d.get_alias(from);
    auto to_id = d.get_alias(to);

    if (label.empty())
        return d.contains(fmt::format("{} ..> {}", from_id, "", to_id));

    util::replace_all(label, "<", "&lt;");
    util::replace_all(label, ">", "&gt;");
    util::replace_all(label, "(", "&lpar;");
    util::replace_all(label, ")", "&rpar;");
    util::replace_all(label, "##", "::");
    util::replace_all(label, "{", "&lbrace;");
    util::replace_all(label, "}", "&rbrace;");

    if (label.empty())
        return d.contains(fmt::format("{} ..> {}", from_id, to_id));

    return d.contains(fmt::format("{} ..> {} : {}", from_id, to_id, label));
}

template <> bool IsConcept(const mermaid_t &d, QualifiedName cpt)
{
    return d.search(
        std::string("class ") + d.get_alias(cpt) + " \\{\\n\\s+<<concept>>");
}

template <>
bool IsConceptRequirement(
    const mermaid_t &d, std::string const &cpt, std::string requirement)
{
    util::replace_all(requirement, "<", "&lt;");
    util::replace_all(requirement, ">", "&gt;");
    util::replace_all(requirement, "##", "::");
    util::replace_all(requirement, "{", "&lbrace;");
    util::replace_all(requirement, "}", "&rbrace;");

    return d.contains(requirement);
}

template <>
bool IsConceptParameterList(
    const mermaid_t &d, std::string const &cpt, std::string params)
{
    util::replace_all(params, "<", "&lt;");
    util::replace_all(params, ">", "&gt;");
    util::replace_all(params, "##", "::");
    util::replace_all(params, "{", "&lbrace;");
    util::replace_all(params, "}", "&rbrace;");

    return d.contains(params);
}

template <>
bool IsLayoutHint(const mermaid_t &d, std::string const &from,
    std::string const &hint, std::string const &to)
{
    return true;
}

template <> bool HasComment(const mermaid_t &d, std::string const &comment)
{
    return d.contains(fmt::format("%% {}", comment));
}

template <>
bool HasNote(const mermaid_t &d, std::string const &cls,
    std::string const &position, std::string const &note)
{
    if (d.diagram_type == common::model::diagram_t::kPackage) {
        return d.contains(fmt::format("-.- {}", d.get_alias(cls)));
    }

    return d.contains(fmt::format("note for {}", d.get_alias(cls)));
}

template <>
bool HasMemberNote(const mermaid_t &d, std::string const &cls,
    std::string const &member, std::string const &position,
    std::string const &note)
{
    return d.contains(
        fmt::format("note for {} \"{}\"", d.get_alias(cls), note));
}

template <>
bool HasPackageNote(const mermaid_t &d, std::string const &cls,
    std::string const &position, std::string const &note)
{
    return d.contains(fmt::format("-.- {}", d.get_alias(cls)));
}

template <>
bool HasLink(const mermaid_t &d, std::string const &element,
    std::string const &link, std::string const &tooltip)
{
    return d.contains(fmt::format(
        "click {} href \"{}\" \"{}\"", d.get_alias(element), link, tooltip));
}

template <>
bool HasMemberLink(const mermaid_t &d, std::string const &method,
    std::string const &link, std::string const &tooltip)
{
    return true;
}

template <> bool IsFolder(const mermaid_t &d, std::string const &path)
{
    return d.contains("subgraph " + d.get_alias(util::split(path, "/").back()));
}

template <> bool IsFile(const mermaid_t &d, std::string const &path)
{
    return d.contains(d.get_alias(util::split(path, "/").back()) + "[");
}

template <> bool IsSystemHeader(const mermaid_t &d, std::string const &path)
{
    return d.contains(d.get_alias(path) + "[");
}

template <typename... Args>
bool IsNamespacePackage(const mermaid_t &d, Args... args)
{
    if (d.diagram_type == class_diagram::model::diagram_t::kClass) {
        std::vector<std::string> toks{{args...}};
        return d.contains(fmt::format("[\"{}", fmt::join(toks, "::")));
    }

    const auto &name = get_last(args...);
    return d.contains(fmt::format("subgraph {}", d.get_alias(name)));
}

template <typename... Args>
bool IsDirectoryPackage(const mermaid_t &d, Args... args)
{
    if (d.diagram_type == class_diagram::model::diagram_t::kClass) {
        // MermaidJS does not support packages in class diagrams
        return true;
    }

    const auto &name = get_last(args...);
    return d.contains("subgraph " + d.get_alias(name));
}

template <typename... Args>
bool IsModulePackage(const mermaid_t &d, Args... args)
{
    if (d.diagram_type == class_diagram::model::diagram_t::kClass) {
        // MermaidJS does not support packages in class diagrams
        return true;
    }

    const auto &name = get_last(args...);
    return d.contains("subgraph " + d.get_alias(name));
}

template <> bool IsDeprecated(const mermaid_t &d, const std::string &name)
{
    return d.contains(d.get_alias(name));
}

template <>
int64_t FindMessage(
    const mermaid_t &d, const Message &msg, int64_t offset, bool fail)
{
    auto msg_str = msg.message;

    util::replace_all(msg_str, "(", "\\(");
    util::replace_all(msg_str, ")", "\\)");
    util::replace_all(msg_str, "*", "\\*");
    util::replace_all(msg_str, "[", "\\[");
    util::replace_all(msg_str, "]", "\\]");
    util::replace_all(msg_str, "+", "\\+");

    std::string call_pattern{"__INVALID__"};

    if (msg.is_entrypoint) {
        call_pattern =
            fmt::format("\\* ->> {} : {}", d.get_alias(msg.to), msg_str);
    }
    else if (msg.is_exitpoint) {
        call_pattern = fmt::format("{} -->> \\*", d.get_alias(msg.to), msg_str);
    }
    else if (msg.is_incontrolcondition) {
        call_pattern = fmt::format("{} {} {} : \\[{}\\]", d.get_alias(msg.from),
            "->>", d.get_alias(msg.to), msg_str);
    }
    else if (msg.is_response) {
        call_pattern = fmt::format("{} {} {} : {}", d.get_alias(msg.from),
            "-->>", d.get_alias(msg.to), msg_str);
    }
    else {
        call_pattern = fmt::format("{} {} {} : {}", d.get_alias(msg.from),
            "->>", d.get_alias(msg.to), msg_str);
    }

    auto match_offset = d.find(call_pattern, offset);

    if (match_offset < 0) {
        if (fail)
            FAIL(fmt::format("Missing message: {} -> {} {} ({})",
                msg.from.str(), msg.to.str(), msg.message, call_pattern));
        return -1;
    }

    return match_offset + offset;
}

template <>
bool HasMessageComment(
    const mermaid_t &d, std::string const &participant, std::string const &note)
{
    std::string note_escaped{note};
    util::replace_all(note_escaped, "\\n", "<br/>");

    return d.contains(std::string("note over ") + d.get_alias(participant) +
        ": " + note_escaped);
}

template <>
bool IsParticipant(
    const mermaid_t &d, const std::string &name, const std::string &type)
{
    return d.contains(fmt::format("participant {}", d.get_alias(name)));
}

//
// JSON test helpers
//
struct File {
    explicit File(const std::string &f)
        : file{f}
    {
    }

    const std::string file;
};

template <> bool HasTitle(const json_t &d, std::string const &str)
{
    return d.src.contains("title") && d.src["title"] == str;
}

template <> bool IsAbstractClass(const json_t &d, QualifiedName cls)
{
    auto e =
        get_element(d.src, expand_name(d.src, cls.str(d.generate_packages)));
    return e && e->at("type") == "class" && e->at("is_abstract");
}

template <> bool IsEnum(const json_t &d, QualifiedName enm)
{
    auto e =
        get_element(d.src, expand_name(d.src, enm.str(d.generate_packages)));
    return e && e->at("type") == "enum";
}

template <> bool IsUnion(const json_t &d, QualifiedName enm)
{
    auto e =
        get_element(d.src, expand_name(d.src, enm.str(d.generate_packages)));
    return e && e->at("type") == "class" && e->at("is_union");
}

template <> bool IsClass(const json_t &d, QualifiedName cls)
{
    auto e =
        get_element(d.src, expand_name(d.src, cls.str(d.generate_packages)));
    return e && e->at("type") == "class" && !e->at("is_abstract");
}

template <> bool IsClassTemplate(const json_t &d, QualifiedName cls)
{
    auto e =
        get_element(d.src, expand_name(d.src, cls.str(d.generate_packages)));
    return e && e->at("type") == "class";
}

template <> bool IsAbstractClassTemplate(const json_t &d, QualifiedName cls)
{
    auto e =
        get_element(d.src, expand_name(d.src, cls.str(d.generate_packages)));
    return e && e->at("type") == "class" && e->at("is_abstract");
}

template <>
bool IsBaseClass(const json_t &d, QualifiedName base, QualifiedName subclass)
{
    const auto &j = d.src;
    auto base_el =
        get_element(j, expand_name(j, base.str(d.generate_packages)));
    auto subclass_el =
        get_element(j, expand_name(j, subclass.str(d.generate_packages)));

    if (!base_el || !subclass_el)
        return false;

    const nlohmann::json &bases = (*subclass_el)["bases"];

    return std::find_if(bases.begin(), bases.end(), [&](const auto &it) {
        return it["id"] == base_el.value()["id"];
    }) != bases.end();
}

template <>
bool IsInnerClass(
    const json_t &d, std::string const &parent, std::string const &inner)
{
    const auto &j = d.src;

    auto rel = get_relationship(
        j, expand_name(j, inner), expand_name(j, parent), "containment");

    return rel != j["relationships"].end();
}

template <typename... Ts>
bool IsMethod(const json_t &d, const std::string &cls, std::string const &name,
    std::string type = "void", std::string const &params = "")
{
    const auto &j = d.src;
    auto sc = get_element(j, expand_name(j, cls));

    if (!sc)
        return false;

    const nlohmann::json &methods = (*sc)["methods"];

    return std::find_if(methods.begin(), methods.end(), [name](const auto &it) {
        return it["display_name"] == name;
    }) != methods.end();
}

template <typename... Ts>
bool IsField(const json_t &d, QualifiedName cls, std::string const &name,
    std::string type)
{
    const auto &j = d.src;

    auto sc = get_element(j, expand_name(j, cls.str(d.generate_packages)));

    if (!sc)
        return false;

    const nlohmann::json &members = (*sc)["members"];

    return std::find_if(members.begin(), members.end(), [&](const auto &it) {
        return it["name"] == name && it["type"] == type;
    }) != members.end();
}

template <typename... Ts>
bool IsAssociation(const json_t &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "")
{
    const auto &j = d.src;

    auto rel = get_relationship(
        j, expand_name(j, from), expand_name(j, to), "association", label);

    if (rel == j["relationships"].end())
        return false;

    if (!label.empty() && (label != rel->at("label")))
        return false;

    std::string access;
    if constexpr (has_type<Public, Ts...>())
        access = "public";
    else if constexpr (has_type<Protected, Ts...>())
        access = "protected";
    else
        access = "private";

    if (access != rel->at("access"))
        return false;

    return true;
}

template <typename... Ts>
bool IsComposition(const json_t &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "")
{
    const auto &j = d.src;

    auto rel = get_relationship(
        j, expand_name(j, from), expand_name(j, to), "composition", label);

    if (rel == j["relationships"].end())
        return false;

    if (!label.empty() && label != rel->at("label"))
        return false;

    std::string access;
    if constexpr (has_type<Public, Ts...>())
        access = "public";
    else if constexpr (has_type<Protected, Ts...>())
        access = "protected";
    else
        access = "private";

    if (access != rel->at("access"))
        return false;

    return true;
}

template <typename... Ts>
bool IsAggregation(const json_t &d, std::string const &from,
    std::string const &to, std::string const &label = "",
    std::string multiplicity_source = "", std::string multiplicity_dest = "",
    std::string style = "")
{
    const auto &j = d.src;

    auto rel = get_relationship(
        j, expand_name(j, from), expand_name(j, to), "aggregation", label);

    if (rel == j["relationships"].end())
        return false;

    if (!label.empty() && label != rel->at("label"))
        return false;

    std::string access;
    if constexpr (has_type<Public, Ts...>())
        access = "public";
    else if constexpr (has_type<Protected, Ts...>())
        access = "protected";
    else
        access = "private";

    if (access != rel->at("access"))
        return false;

    return true;
}

template <>
bool IsInstantiation(const json_t &d, std::string const &from,
    std::string const &to, std::string style)
{
    const auto &j = d.src;

    auto rel = get_relationship(
        j, expand_name(j, to), expand_name(j, from), "instantiation");

    if (rel == j["relationships"].end())
        return false;

    return true;
}

template <>
bool IsDependency(
    const json_t &d, QualifiedName from, QualifiedName to, std::string style)
{
    const auto &j = d.src;

    auto rel =
        get_relationship(j, expand_name(j, from.str(d.generate_packages)),
            expand_name(j, to.str(d.generate_packages)), "dependency");

    if (rel == j["relationships"].end())
        return false;

    return true;
}

template <typename... Ts>
bool IsFriend(const json_t &d, std::string const &from, std::string const &to)
{
    std::string access;

    if constexpr (has_type<Public, Ts...>())
        access = "public";
    else if constexpr (has_type<Protected, Ts...>())
        access = "protected";
    else
        access = "private";

    const auto &j = d.src;

    auto rel = get_relationship(
        j, expand_name(j, from), expand_name(j, to), "friendship");

    return rel != j["relationships"].end() && rel->at("access") == access;
}

template <>
bool IsPackageDependency(
    const json_t &d, std::string const &from, std::string const &to)
{
    const auto &j = d.src;

    auto rel = get_relationship(
        j, expand_name(j, from), expand_name(j, to), "dependency");

    if (rel == j["relationships"].end())
        return false;

    return true;
}

template <>
bool IsIncludeDependency(
    const json_t &d, std::string const &from, std::string const &to)
{
    const auto &j = d.src;

    auto rel = get_relationship(
        j, expand_name(j, from), expand_name(j, to), "dependency");

    if (rel == j["relationships"].end())
        return false;

    return true;
}

template <> bool IsConcept(const json_t &d, QualifiedName cpt)
{
    const auto &j = d.src;

    auto e = get_element(j, expand_name(j, cpt.str(d.generate_packages)));
    return e && e->at("type") == "concept";
}

template <>
bool IsConstraint(const json_t &d, QualifiedName from, QualifiedName to,
    std::string label, std::string style)
{
    const auto &j = d.src;

    auto rel =
        get_relationship(j, expand_name(j, from.str(d.generate_packages)),
            expand_name(j, to.str(d.generate_packages)), "constraint", label);

    if (rel == j["relationships"].end())
        return false;

    if ((*rel)["label"] != label)
        return false;

    return true;
}

template <>
bool IsConceptRequirement(
    const json_t &d, std::string const &cpt, std::string requirement)
{
    const auto &j = d.src;

    auto e = get_element(j, expand_name(j, cpt));

    if (!e)
        return false;

    const nlohmann::json &statements = (*e)["statements"];

    return std::find_if(statements.begin(), statements.end(),
               [requirement](const auto &it) { return it == requirement; }) !=
        statements.end();
}

template <>
bool IsConceptParameterList(
    const json_t &d, std::string const &cpt, std::string parameter_list)
{
    const auto &j = d.src;

    auto e = get_element(j, expand_name(j, cpt));

    if (!e)
        return false;

    const nlohmann::json &parameters = (*e)["parameters"];

    std::vector<std::string> params;
    for (const auto &it : parameters) {
        params.push_back(fmt::format("{} {}", it["type"], it["name"]));
    }

    return parameter_list == fmt::format("({})", fmt::join(params, ","));
}

template <>
bool IsLayoutHint(const json_t &d, std::string const &from,
    std::string const &hint, std::string const &to)
{
    return true;
}

template <> bool HasComment(const json_t &d, std::string const &comment)
{
    // Comments are not included in JSON
    return true;
}

template <>
bool HasNote(const json_t &d, std::string const &cls,
    std::string const &position, std::string const &note)
{
    const auto &j = d.src;

    auto sc = get_element(j, expand_name(j, cls));

    if (!sc)
        return false;

    std::string formatted = (*sc)["comment"]["formatted"];

    return util::contains(formatted, note);
}

template <>
bool HasPackageNote(const json_t &d, std::string const &cls,
    std::string const &position, std::string const &note)
{
    return true;
}

template <>
bool HasMemberNote(const json_t &d, std::string const &cls,
    std::string const &member, std::string const &position,
    std::string const &note)
{
    return true;
}

template <>
bool HasLink(const json_t &d, std::string const &alias, std::string const &link,
    std::string const &tooltip)
{
    return true;
}

template <>
bool HasMemberLink(const json_t &d, std::string const &method,
    std::string const &link, std::string const &tooltip)
{
    return true;
}

template <> bool IsFolder(const json_t &d, std::string const &path)
{
    const auto &j = d.src;

    auto e = get_element(j, path);
    return e && e->at("type") == "folder";
}

template <> bool IsFile(const json_t &d, std::string const &path)
{
    const auto &j = d.src;

    auto e = get_element(j, path);
    return e && e->at("type") == "file";
}

template <> bool IsSystemHeader(const json_t &d, std::string const &path)
{
    const auto &j = d.src;

    auto e = get_element(j, path);
    return e && e->at("type") == "file" && e->at("file_kind") == "header" &&
        e->at("is_system");
}

template <>
bool IsHeaderDependency(const json_t &d, std::string const &from,
    std::string const &to, std::string style)
{
    assert(d.diagram_type == common::model::diagram_t::kInclude);
    const auto &j = d.src;

    auto rel = get_relationship(
        j, expand_name(j, from), expand_name(j, to), "association");

    if (rel == j["relationships"].end())
        return false;

    return true;
}

template <>
bool IsSystemHeaderDependency(const json_t &d, std::string const &from,
    std::string const &to, std::string style)
{
    assert(d.diagram_type == common::model::diagram_t::kInclude);
    const auto &j = d.src;

    auto rel = get_relationship(
        j, expand_name(j, from), expand_name(j, to), "dependency");

    if (rel == j["relationships"].end())
        return false;

    return true;
}

template <typename PackageT, typename... Args>
bool IsPackagePath(
    const nlohmann::json &j, const std::string &head, Args... args)
{
    if constexpr (sizeof...(Args) == 0) {
        auto e = get_element(j, expand_name(j, head));

        return e && e->at("type") == package_type_name<PackageT>();
    }
    else {
        auto e = get_element(j, head);
        if (!e.has_value())
            return false;

        return IsPackagePath<PackageT>(*e, args...);
    }
}

template <typename... Args>
bool IsNamespacePackage(const json_t &d, Args... args)
{
    const auto &j = d.src;

    return IsPackagePath<NamespacePackage>(j, std::forward<Args>(args)...);
}

template <typename... Args>
bool IsDirectoryPackage(const json_t &d, Args... args)
{
    const auto &j = d.src;

    return IsPackagePath<DirectoryPackage>(j, std::forward<Args>(args)...);
}

template <typename... Args> bool IsModulePackage(const json_t &d, Args... args)
{
    const auto &j = d.src;

    return IsPackagePath<ModulePackage>(j, std::forward<Args>(args)...);
}

template <> bool IsDeprecated(const json_t &d, const std::string &name)
{
    const auto &j = d.src;

    auto e = get_element(j, expand_name(j, name));
    return e && e->at("is_deprecated") == true;
}

namespace json_helpers {
int find_message_nested(const nlohmann::json &j, const std::string &from,
    const std::string &to, const std::string &msg,
    std::optional<std::string> return_type, const nlohmann::json &from_p,
    const nlohmann::json &to_p, int &count, const int64_t offset,
    std::optional<int32_t> chain_index = {})
{
    if (!j.contains("messages") && !j.contains("message_chains"))
        return -1;

    const auto &messages = !chain_index.has_value()
        ? j["messages"]
        : j["message_chains"][chain_index.value()]["messages"];

    int res{-1};

    for (const auto &m : messages) {
        if (m.contains("branches")) {
            for (const auto &b : m["branches"]) {
                auto nested_res = find_message_nested(
                    b, from, to, msg, return_type, from_p, to_p, count, offset);

                if (nested_res >= offset)
                    return nested_res;
            }
        }
        else if (m.contains("messages")) {
            auto nested_res = find_message_nested(
                m, from, to, msg, return_type, from_p, to_p, count, offset);

            if (nested_res >= offset)
                return nested_res;
        }
        else {
            if (count >= offset &&
                (m["from"]["participant_id"] == from_p["id"]) &&
                (m["to"]["participant_id"] == to_p["id"]) &&
                (m["name"] == msg) &&
                (!return_type || m["return_type"] == *return_type))
                return count;

            count++;
        }
    }

    return res;
}

int find_message_impl(const nlohmann::json &j, const std::string &from,
    const std::string &to, const std::string &msg,
    std::optional<std::string> return_type, int64_t offset,
    std::optional<int32_t> chain_index = {})
{

    auto from_p = get_participant(j, from);
    auto to_p = get_participant(j, to);

    // TODO: support diagrams with multiple sequences...
    int count{0};

    for (const auto &seq : j["sequences"]) {
        int64_t res{-1};

        res = find_message_nested(seq, from, to, msg, return_type, *from_p,
            *to_p, count, offset, chain_index);

        if (res >= 0)
            return res;
    }

    throw std::runtime_error(fmt::format(
        "No such message {} {} {} after offset {}", from, to, msg, offset));
}

int64_t find_message(const nlohmann::json &j, const File &from, const File &to,
    const std::string &msg, int64_t offset)
{
    return find_message_impl(j, from.file, to.file, msg, {}, offset);
}

int64_t find_message(const nlohmann::json &j, const std::string &from,
    const std::string &to, const std::string &msg,
    std::optional<std::string> return_type = {}, int64_t offset = 0)
{
    return find_message_impl(
        j, expand_name(j, from), expand_name(j, to), msg, return_type, offset);
}

int64_t find_message_in_chain(const nlohmann::json &j, const std::string &from,
    const std::string &to, const std::string &msg,
    std::optional<std::string> return_type = {}, int64_t offset = 0,
    uint32_t chain_index = 0)
{
    return find_message_impl(j, expand_name(j, from), expand_name(j, to), msg,
        return_type, offset, chain_index);
}

} // namespace detail

template <>
int64_t FindMessage(
    const json_t &d, const Message &msg, int64_t offset, bool fail)
{
    if (msg.is_response) {
        // TODO: Currently response are not generated as separate messages in
        //       JSON format
        return offset;
    }

    if (msg.is_entrypoint || msg.is_exitpoint)
        return offset;

    try {
        return json_helpers::find_message(d.src, msg.from.str(), msg.to.str(),
            msg.message, msg.return_type, offset);
    }
    catch (std::exception &e) {
        if (!fail)
            return -1;

        std::cout << "FindMessage failed with error " << e.what() << "\n";

        throw e;
    }
}

int64_t find_message_in_chain(const json_t &d, const Message &msg,
    int64_t offset, bool fail, uint32_t chain_index)
{
    if (msg.is_response) {
        // TODO: Currently response are not generated as separate messages in
        //       JSON format
        return offset;
    }

    if (msg.is_entrypoint || msg.is_exitpoint)
        return offset;

    try {
        return json_helpers::find_message_in_chain(d.src, msg.from.str(),
            msg.to.str(), msg.message, msg.return_type, offset, chain_index);
    }
    catch (std::exception &e) {
        if (!fail)
            return -1;

        std::cout << "find_message_in_chain failed with " << e.what() << "\n";

        throw e;
    }
}

template <>
bool MessageChainsOrder<json_t>(
    const json_t &d, std::vector<std::vector<Message>> message_chains)
{
    uint32_t chain_index{0};
    for (const auto &messages : message_chains) {
        int64_t offset{0};

        std::vector<int64_t> order;
        order.reserve(messages.size());
        std::transform(messages.begin(), messages.end(),
            std::back_inserter(order),
            [&d, &offset, chain_index](const auto &m) {
                offset = find_message_in_chain(d, m, offset, true, chain_index);
                return offset;
            });

        bool are_messages_in_order = std::is_sorted(order.begin(), order.end());

        chain_index++;

        if (!are_messages_in_order) {
            FAIL(fmt::format(
                "Messages are not in order: \n[{}]", fmt::join(order, ",\n")));
            return false;
        }
    }

    return true;
}

template <>
bool HasMessageComment(
    const json_t &d, std::string const &alias, std::string const &note)
{
    return true;
}

template <>
bool IsParticipant(
    const json_t &d, const std::string &name, const std::string &type)
{
    const auto &j = d.src;

    auto p = get_participant(j, expand_name(j, name));

    return p && (p->at("type") == type);
}
}

/*
namespace clanguml {
namespace test {
namespace matchers {

using Catch::CaseSensitive;
using Catch::Matchers::StdString::CasedString;
using Catch::Matchers::StdString::ContainsMatcher;
using Catch::Matchers::StdString::RegexMatcher;

using Catch::Matchers::Contains;
using Catch::Matchers::EndsWith;
using Catch::Matchers::Equals;
using Catch::Matchers::Matches;
using Catch::Matchers::StartsWith;
using Catch::Matchers::VectorContains;

struct JsonMatcherBase : Catch::MatcherBase<nlohmann::json> {
    JsonMatcherBase(
        std::string const &operation, CasedString const &comparator);
    std::string describe() const override;

    CasedString m_comparator;
    std::string m_operation;
};

template <typename T, typename... Ts> constexpr bool has_type() noexcept
{
    return (std::is_same_v<T, Ts> || ... || false);
}

struct Public { };

struct Protected { };

struct Private { };

struct Abstract { };

struct Static { };

struct Const { };

struct Constexpr { };

struct Consteval { };

struct Coroutine { };

struct Noexcept { };

struct Default { };

struct Deleted { };

struct HasCallWithResultMatcher : ContainsMatcher {
    HasCallWithResultMatcher(
        CasedString const &comparator, CasedString const &resultComparator)
        : ContainsMatcher(comparator)
        , m_resultComparator{resultComparator}
    {
    }

    bool match(std::string const &source) const override
    {
        return Catch::contains(
                   m_comparator.adjustString(source), m_comparator.m_str) &&
            Catch::contains(
                m_comparator.adjustString(source), m_resultComparator.m_str);
    }

    CasedString m_resultComparator;
};

template <typename T> class HasCallMatcher : public Catch::MatcherBase<T> {
    T m_from, m_to, m_message;
    bool m_is_response;
    std::string m_call_arrow, m_return_arrow;
    std::string call_pattern, response_pattern;

public:
    HasCallMatcher(T from, T to, T message, bool is_response = false,
        const std::string &call_arrow = "->",
        const std::string &return_arrow = "-->")
        : m_from(from)
        , m_to{to}
        , m_message{message}
        , m_is_response{is_response}
        , m_call_arrow{call_arrow}
        , m_return_arrow{return_arrow}
    {
        util::replace_all(m_message, "(", "\\(");
        util::replace_all(m_message, ")", "\\)");
        util::replace_all(m_message, "*", "\\*");
        util::replace_all(m_message, "[", "\\[");
        util::replace_all(m_message, "]", "\\]");
        util::replace_all(m_message, "+", "\\+");

        call_pattern = fmt::format("{} {} {} "
                                   "(\\[\\[.*\\]\\] )?: {}",
            m_from, m_call_arrow, m_to, m_message);

        response_pattern = fmt::format(
            "{} {} {} : //{}//", m_from, m_return_arrow, m_to, m_message);
    }

    bool match(T const &in) const override
    {
        std::regex r{m_is_response ? response_pattern : call_pattern};
        std::smatch base_match;
        std::regex_search(in, base_match, r);
        if (base_match.size() > 0)
            return true;

        return false;
    }

    std::string describe() const override
    {
        std::ostringstream ss;
        ss << "has call "
           << fmt::format(
                  "{} {} {} : {}", m_from, m_call_arrow, m_to, m_message);
        return ss.str();
    }
};

auto HasCall(std::string const &from, std::string const &to,
    std::string const &message,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return HasCallMatcher(from, to, message, false);
}

auto HasResponse(std::string const &from, std::string const &to,
    std::string const &message,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return HasCallMatcher(to, from, message, true);
}

auto HasCallInControlCondition(std::string const &from, std::string const &to,
    std::string const &message,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return HasCallMatcher(from, to, fmt::format("**[**{}**]**", message));
}

auto HasCall(std::string const &from, std::string const &message,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return HasCall(from, from, message, caseSensitivity);
}

namespace mermaid {
template <typename T> class HasCallMatcher : public Catch::MatcherBase<T> {
    T m_from, m_to, m_message;
    bool m_is_response;
    std::string m_call_arrow, m_return_arrow;
    std::string call_pattern, response_pattern;

public:
    HasCallMatcher(T from, T to, T message, bool is_response = false,
        const std::string &call_arrow = "->>",
        const std::string &return_arrow = "-->>")
        : m_from(from)
        , m_to{to}
        , m_message{message}
        , m_is_response{is_response}
        , m_call_arrow{call_arrow}
        , m_return_arrow{return_arrow}
    {
        util::replace_all(m_message, "(", "\\(");
        util::replace_all(m_message, ")", "\\)");
        util::replace_all(m_message, "*", "\\*");
        util::replace_all(m_message, "[", "\\[");
        util::replace_all(m_message, "]", "\\]");
        util::replace_all(m_message, "+", "\\+");

        call_pattern =
            fmt::format("{} {} {} : {}", m_from, m_call_arrow, m_to, m_message);

        response_pattern = fmt::format(
            "{} {} {} : {}", m_from, m_return_arrow, m_to, m_message);
    }

    bool match(T const &in) const override
    {
        std::regex r{m_is_response ? response_pattern : call_pattern};
        std::smatch base_match;
        std::regex_search(in, base_match, r);
        if (base_match.size() > 0)
            return true;

        return false;
    }

    std::string describe() const override
    {
        std::ostringstream ss;
        ss << "has call "
           << fmt::format(
                  "{} {} {} : {}", m_from, m_call_arrow, m_to, m_message);
        return ss.str();
    }
};

auto HasCall(std::string const &from, std::string const &to,
    std::string const &message,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return mermaid::HasCallMatcher(from, to, message, false);
}

auto HasCallInControlCondition(std::string const &from, std::string const &to,
    std::string const &message,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return mermaid::HasCallMatcher(from, to, fmt::format("[{}]", message));
}

auto HasResponse(std::string const &from, std::string const &to,
    std::string const &message,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return HasCallMatcher(to, from, message, true, "->>", "-->>");
}

auto HasCall(std::string const &from, std::string const &message,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return mermaid::HasCall(from, from, message, caseSensitivity);
}
}

ContainsMatcher HasEntrypoint(std::string const &to, std::string const &message,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(fmt::format("[-> {} : {}", to, message), caseSensitivity));
}

namespace mermaid {
ContainsMatcher HasEntrypoint(std::string const &to, std::string const &message,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(
        fmt::format("* ->> {} : {}", to, message), caseSensitivity));
}
}

ContainsMatcher HasExitpoint(std::string const &to,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(fmt::format("[<-- {}", to), caseSensitivity));
}

namespace mermaid {
ContainsMatcher HasExitpoint(std::string const &to,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(fmt::format("{} -->> *", to), caseSensitivity));
}
}

std::string _NS(std::string_view s)
{
    return fmt::format(
        "clanguml::{}::{}", Catch::getResultCapture().getCurrentTestName(), s);
}

class NamespaceWrapper {

private:
};

struct AliasMatcher {
    AliasMatcher(const std::string &puml_)
        : puml{split(puml_, "\n")}
    {
    }

    std::string operator()(std::string name)
    {
        std::vector<std::regex> patterns;

        const std::string alias_regex("([A-Z]_[0-9]+)");

        util::replace_all(name, "(", "\\(");
        util::replace_all(name, ")", "\\)");
        util::replace_all(name, " ", "\\s");
        util::replace_all(name, "*", "\\*");
        util::replace_all(name, "[", "\\[");
        util::replace_all(name, "]", "\\]");

        patterns.push_back(
            std::regex{"class\\s\"" + name + "\"\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"abstract\\s\"" + name + "\"\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"enum\\s\"" + name + "\"\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"package\\s\"" + name + "\"\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"package\\s\\[" + name + "\\]\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"file\\s\"" + name + "\"\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"folder\\s\"" + name + "\"\\sas\\s" + alias_regex});
        patterns.push_back(
            std::regex{"participant\\s\"" + name + "\"\\sas\\s" + alias_regex});

        std::smatch base_match;

        for (const auto &line : puml) {
            for (const auto &pattern : patterns) {
                if (std::regex_search(line, base_match, pattern) &&
                    base_match.size() == 2) {
                    std::ssub_match base_sub_match = base_match[1];
                    std::string alias = base_sub_match.str();
                    return trim(alias);
                }
            }
        }

        return fmt::format("__INVALID__ALIAS__({})", name);
    }

    const std::vector<std::string> puml;
};

namespace mermaid {
struct AliasMatcher {
    AliasMatcher(const std::string &mmd_)
        : mmd{split(mmd_, "\n")}
    {
    }

    std::string operator()(std::string name)
    {
        std::vector<std::regex> patterns;

        const std::string alias_regex("([A-Z]_[0-9]+)");

        util::replace_all(name, "(", "&lpar;");
        util::replace_all(name, ")", "&rpar;");
        util::replace_all(name, " ", "\\s");
        util::replace_all(name, "*", "\\*");
        util::replace_all(name, "[", "\\[");
        util::replace_all(name, "]", "\\]");
        util::replace_all(name, "<", "&lt;");
        util::replace_all(name, ">", "&gt;");

        patterns.push_back(
            std::regex{"class\\s" + alias_regex + "\\[\"" + name + "\"\\]"});
        patterns.push_back(
            std::regex{"subgraph\\s" + alias_regex + "\\[" + name + "\\]"});
        patterns.push_back(
            std::regex{"\\s\\s" + alias_regex + "\\[" + name + "\\]"}); // file

        std::smatch base_match;

        for (const auto &line : mmd) {
            for (const auto &pattern : patterns) {
                if (std::regex_search(line, base_match, pattern) &&
                    base_match.size() == 2) {
                    std::ssub_match base_sub_match = base_match[1];
                    std::string alias = base_sub_match.str();
                    return trim(alias);
                }
            }
        }

        return "__INVALID__ALIAS__";
    }

    const std::vector<std::string> mmd;
};

struct SequenceDiagramAliasMatcher {
    SequenceDiagramAliasMatcher(const std::string &mmd_)
        : mmd{split(mmd_, "\n")}
    {
    }

    std::string operator()(std::string name)
    {
        std::vector<std::regex> patterns;

        const std::string alias_regex("([A-Z]_[0-9]+)");

        util::replace_all(name, "(", "\\(");
        util::replace_all(name, ")", "\\)");
        util::replace_all(name, " ", "\\s");
        util::replace_all(name, "*", "\\*");
        util::replace_all(name, "[", "\\[");
        util::replace_all(name, "]", "\\]");

        patterns.push_back(
            std::regex{"participant\\s" + alias_regex + "\\sas\\s" + name});

        std::smatch base_match;

        for (const auto &line : mmd) {
            for (const auto &pattern : patterns) {
                if (std::regex_search(line, base_match, pattern) &&
                    base_match.size() == 2) {
                    std::ssub_match base_sub_match = base_match[1];
                    std::string alias = base_sub_match.str();
                    return trim(alias);
                }
            }
        }

        return "__INVALID__ALIAS__";
    }

    const std::vector<std::string> mmd;
};
}

ContainsMatcher HasTitle(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString("title " + str, caseSensitivity));
}

namespace mermaid {
ContainsMatcher HasTitle(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString("title: " + str, caseSensitivity));
}
}

ContainsMatcher IsClass(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString("class " + str, caseSensitivity));
}

namespace mermaid {
auto IsClass(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString("class " + str, caseSensitivity));
}
}

ContainsMatcher IsUnion(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString("class " + str + " <<union>>", caseSensitivity));
}

namespace mermaid {
auto IsUnion(std::string const &alias,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return Catch::Matchers::Matches(
        std::string("class ") + alias + " \\{\\n\\s+<<union>>",
        caseSensitivity);
}
}

ContainsMatcher IsClassTemplate(std::string const &str,
    std::string const &tmplt,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(
        fmt::format("class \"{}<{}>\"", str, tmplt), caseSensitivity));
}

ContainsMatcher IsConcept(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString("class " + str + " <<concept>>", caseSensitivity));
}

namespace mermaid {
auto IsConcept(std::string const &alias,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return Catch::Matchers::Matches(
        std::string("class ") + alias + " \\{\\n\\s+<<concept>>",
        caseSensitivity);
}
}

ContainsMatcher IsEnum(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString("enum " + str, caseSensitivity));
}

namespace mermaid {
auto IsEnum(std::string const &alias,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return Catch::Matchers::Matches(
        std::string("class ") + alias + " \\{\\n\\s+<<enumeration>>",
        caseSensitivity);
}
}

ContainsMatcher IsAbstractClass(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString("abstract " + str, caseSensitivity));
}

namespace mermaid {
auto IsAbstractClass(std::string const &alias,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return Catch::Matchers::Matches(
        std::string("class ") + alias + " \\{\\n\\s+<<abstract>>",
        caseSensitivity);
}
}

ContainsMatcher IsAbstractClassTemplate(std::string const &str,
    std::string const &tmplt,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(
        fmt::format("abstract \"{}<{}>\"", str, tmplt), caseSensitivity));
}

ContainsMatcher IsBaseClass(std::string const &base, std::string const &sub,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(base + " <|-- " + sub, caseSensitivity));
}

namespace mermaid {
ContainsMatcher IsBaseClass(std::string const &base, std::string const &sub,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(base + " <|-- " + sub, caseSensitivity));
}
}

ContainsMatcher IsInnerClass(std::string const &parent,
    std::string const &inner,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(inner + " --+ " + parent, caseSensitivity));
}

namespace mermaid {
ContainsMatcher IsInnerClass(std::string const &parent,
    std::string const &inner,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(parent + " ()-- " + inner + " : ", caseSensitivity));
}
}

ContainsMatcher IsAssociation(std::string const &from, std::string const &to,
    std::string const &label = "", std::string multiplicity_source = "",
    std::string multiplicity_dest = "", std::string style = "",
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    std::string format_string = "{}";
    if (!multiplicity_source.empty())
        format_string += " \"" + multiplicity_source + "\"";

    format_string += fmt::format(" -{}->", style);

    if (!multiplicity_dest.empty())
        format_string += " \"" + multiplicity_dest + "\"";

    format_string += " {}";

    if (!label.empty()) {
        format_string += " : {}";
        return ContainsMatcher(CasedString(
            fmt::format(fmt::runtime(format_string), from, to, label),
            caseSensitivity));
    }
    else
        return ContainsMatcher(
            CasedString(fmt::format(fmt::runtime(format_string), from, to),
                caseSensitivity));
}

ContainsMatcher IsComposition(std::string const &from, std::string const &to,
    std::string const &label, std::string multiplicity_source = "",
    std::string multiplicity_dest = "", std::string style = "",
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    std::string format_string = "{}";
    if (!multiplicity_source.empty())
        format_string += " \"" + multiplicity_source + "\"";

    format_string += fmt::format(" *-{}-", style);

    if (!multiplicity_dest.empty())
        format_string += " \"" + multiplicity_dest + "\"";

    format_string += " {} : {}";

    return ContainsMatcher(
        CasedString(fmt::format(fmt::runtime(format_string), from, to, label),
            caseSensitivity));
}

ContainsMatcher IsAggregation(std::string const &from, std::string const &to,
    std::string const &label, std::string multiplicity_source = "",
    std::string multiplicity_dest = "", std::string style = "",
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    std::string format_string = "{}";
    if (!multiplicity_source.empty())
        format_string += " \"" + multiplicity_source + "\"";

    format_string += fmt::format(" o-{}-", style);

    if (!multiplicity_dest.empty())
        format_string += " \"" + multiplicity_dest + "\"";

    format_string += " {} : {}";

    return ContainsMatcher(
        CasedString(fmt::format(fmt::runtime(format_string), from, to, label),
            caseSensitivity));
}

ContainsMatcher IsInstantiation(std::string const &from, std::string const &to,
    std::string style = "",
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(
        fmt::format("{} .{}.|> {}", to, style, from), caseSensitivity));
}

ContainsMatcher IsDependency(std::string const &from, std::string const &to,
    std::string style = "",
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(
        fmt::format("{} .{}.> {}", from, style, to), caseSensitivity));
}

namespace mermaid {
ContainsMatcher IsPackageDependency(std::string const &from,
    std::string const &to,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(fmt::format("{} -.-> {}", from, to), caseSensitivity));
}
ContainsMatcher IsIncludeDependency(std::string const &from,
    std::string const &to,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(fmt::format("{} -.-> {}", from, to), caseSensitivity));
}
}

ContainsMatcher IsConstraint(std::string const &from, std::string const &to,
    std::string const &label = {}, std::string style = "",
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    if (label.empty())
        return ContainsMatcher(CasedString(
            fmt::format("{} .{}.> {}", from, style, to), caseSensitivity));
    else
        return ContainsMatcher(
            CasedString(fmt::format("{} .{}.> {} : {}", from, style, to, label),
                caseSensitivity));
}

ContainsMatcher IsConceptRequirement(std::string const &cpt,
    std::string const &requirement,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(requirement, caseSensitivity));
}

namespace mermaid {
ContainsMatcher IsConstraint(std::string const &from, std::string const &to,
    std::string label = {},
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    util::replace_all(label, "<", "&lt;");
    util::replace_all(label, ">", "&gt;");
    util::replace_all(label, "(", "&lpar;");
    util::replace_all(label, ")", "&rpar;
    util::replace_all(label, "##", "::");
    util::replace_all(label, "{", "&lbrace;");
    util::replace_all(label, "}", "&rbrace;");

    if (label.empty())
        return ContainsMatcher(
            CasedString(fmt::format("{} ..> {}", from, to), caseSensitivity));
    else
        return ContainsMatcher(CasedString(
            fmt::format("{} ..> {} : {}", from, to, label), caseSensitivity));
}

ContainsMatcher IsConceptRequirement(std::string const &cpt,
    std::string requirement,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    util::replace_all(requirement, "<", "&lt;");
    util::replace_all(requirement, ">", "&gt;");
    util::replace_all(requirement, "(", "&lpar;");
    util::replace_all(requirement, ")", "&rpar;");
    util::replace_all(requirement, "##", "::");
    util::replace_all(requirement, "{", "&lbrace;");
    util::replace_all(requirement, "}", "&rbrace;");

    return ContainsMatcher(CasedString(requirement, caseSensitivity));
}
}

ContainsMatcher IsLayoutHint(std::string const &from, std::string const &hint,
    std::string const &to,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(
        fmt::format("{} -[hidden]{}- {}", from, hint, to), caseSensitivity));
}

ContainsMatcher HasComment(std::string const &comment,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(fmt::format("' {}", comment), caseSensitivity));
}

namespace mermaid {
ContainsMatcher HasComment(std::string const &comment,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(fmt::format("%% {}", comment), caseSensitivity));
}
}

ContainsMatcher HasNote(std::string const &cls, std::string const &position,
    std::string const &note = "",
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(
        fmt::format("note {} of {}", position, cls), caseSensitivity));
}

auto HasMessageComment(std::string const &alias, std::string const &note,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return Catch::Matchers::Matches(
        std::string("note over ") + alias + "\\n" + note + "\\n" + "end note",
        caseSensitivity);
}

namespace mermaid {
auto HasMessageComment(std::string const &alias, std::string const &note,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return Catch::Matchers::Matches(
        std::string("note over ") + alias + ": " + note, caseSensitivity);
}

ContainsMatcher HasNote(std::string const &cls,
    std::string const &position = "", std::string const &note = "",
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(fmt::format("note for {}", cls), caseSensitivity));
}
ContainsMatcher HasPackageNote(std::string const &cls,
    std::string const &position = "", std::string const &note = "",
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(fmt::format("-.- {}", cls), caseSensitivity));
}
}

ContainsMatcher HasMemberNote(std::string const &cls, std::string const &member,
    std::string const &position, std::string const &note = "",
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(fmt::format("note {} of {}::{}", position, cls, member),
            caseSensitivity));
}

ContainsMatcher HasLink(std::string const &alias, std::string const &link,
    std::string const &tooltip,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(
        fmt::format("{} [[{}{{{}}}]]", alias, link, tooltip), caseSensitivity));
}

namespace mermaid {
ContainsMatcher HasLink(std::string const &alias, std::string const &link,
    std::string const &tooltip,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(
        fmt::format("click {} href \"{}\" \"{}\"", alias, link, tooltip),
        caseSensitivity));
}
}

ContainsMatcher HasMemberLink(std::string const &method,
    std::string const &link, std::string const &tooltip,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(fmt::format("{} [[[{}{{{}}}]]]", method, link, tooltip),
            caseSensitivity));
}

template <typename... Ts>
ContainsMatcher IsMethod(std::string const &name,
    std::string const &type = "void", std::string const &params = "",
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    std::string pattern;
    if constexpr (has_type<Static, Ts...>())
        pattern += "{static} ";

    if constexpr (has_type<Abstract, Ts...>())
        pattern += "{abstract} ";

    if constexpr (has_type<Public, Ts...>())
        pattern = "+";
    else if constexpr (has_type<Protected, Ts...>())
        pattern = "#";
    else
        pattern = "-";

    pattern += name;

    pattern += "(" + params + ")";

    if constexpr (has_type<Constexpr, Ts...>())
        pattern += " constexpr";

    if constexpr (has_type<Consteval, Ts...>())
        pattern += " consteval";

    if constexpr (has_type<Const, Ts...>())
        pattern += " const";

    if constexpr (has_type<Abstract, Ts...>())
        pattern += " = 0";

    if constexpr (has_type<Default, Ts...>())
        pattern += " = default";

    if constexpr (has_type<Deleted, Ts...>())
        pattern += " = deleted";

    if constexpr (has_type<Coroutine, Ts...>())
        pattern += " [coroutine]";

    pattern += " : " + type;

    return ContainsMatcher(CasedString(pattern, caseSensitivity));
}

namespace mermaid {
template <typename... Ts>
ContainsMatcher IsMethod(std::string const &name, std::string type = "void",
    std::string const &params = "",
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    std::string pattern;

    if constexpr (has_type<Public, Ts...>())
        pattern = "+";
    else if constexpr (has_type<Protected, Ts...>())
        pattern = "#";
    else
        pattern = "-";

    pattern += name;

    pattern += "(" + params + ")";

    std::vector<std::string> method_mods;
    if constexpr (has_type<Default, Ts...>())
        method_mods.push_back("default");
    if constexpr (has_type<Const, Ts...>())
        method_mods.push_back("const");
    if constexpr (has_type<Constexpr, Ts...>())
        method_mods.push_back("constexpr");
    if constexpr (has_type<Consteval, Ts...>())
        method_mods.push_back("consteval");
    if constexpr (has_type<Coroutine, Ts...>())
        method_mods.push_back("coroutine");

    pattern += " : ";

    if (!method_mods.empty()) {
        pattern += fmt::format("[{}] ", fmt::join(method_mods, ","));
    }

    util::replace_all(type, "<", "&lt;");
    util::replace_all(type, ">", "&gt;");
    util::replace_all(type, "(", "&lpar;");
    util::replace_all(type, ")", "&rpar;");
    util::replace_all(type, "##", "::");
    util::replace_all(type, "{", "&lbrace;");
    util::replace_all(type, "}", "&rbrace;");

    pattern += type;

    if constexpr (has_type<Abstract, Ts...>())
        pattern += "*";

    if constexpr (has_type<Static, Ts...>())
        pattern += "$";

    return ContainsMatcher(CasedString(pattern, caseSensitivity));
}
}

template <typename... Ts>
ContainsMatcher IsField(std::string const &name,
    std::string const &type = "void",
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    std::string pattern;
    if constexpr (has_type<Static, Ts...>())
        pattern += "{static} ";

    if constexpr (has_type<Public, Ts...>())
        pattern = "+";
    else if constexpr (has_type<Protected, Ts...>())
        pattern = "#";
    else
        pattern = "-";

    pattern += name;

    return ContainsMatcher(
        CasedString(pattern + " : " + type, caseSensitivity));
}

namespace mermaid {
template <typename... Ts>
ContainsMatcher IsField(std::string const &name, std::string type = "void",
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    std::string pattern;
    if constexpr (has_type<Static, Ts...>())
        pattern += "{static} ";

    if constexpr (has_type<Public, Ts...>())
        pattern = "+";
    else if constexpr (has_type<Protected, Ts...>())
        pattern = "#";
    else
        pattern = "-";

    pattern += name;

    util::replace_all(type, "<", "&lt;");
    util::replace_all(type, ">", "&gt;");
    util::replace_all(type, "(", "&lpar;");
    util::replace_all(type, ")", "&rpar;");
    util::replace_all(type, "##", "::");
    util::replace_all(type, "{", "&lbrace;");
    util::replace_all(type, "}", "&rbrace;");

    return ContainsMatcher(
        CasedString(pattern + " : " + type, caseSensitivity));
}
}

template <typename... Ts>
ContainsMatcher IsFriend(std::string const &from, std::string const &to,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    std::string pattern;

    if constexpr (has_type<Public, Ts...>())
        pattern = "+";
    else if constexpr (has_type<Protected, Ts...>())
        pattern = "#";
    else
        pattern = "-";

    return ContainsMatcher(
        CasedString(fmt::format("{} <.. {} : {}<<friend>>", from, to, pattern),
            caseSensitivity));
}

namespace mermaid {
template <typename... Ts>
ContainsMatcher IsFriend(std::string const &from, std::string const &to,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    std::string pattern;

    if constexpr (has_type<Public, Ts...>())
        pattern = "+";
    else if constexpr (has_type<Protected, Ts...>())
        pattern = "#";
    else
        pattern = "-";

    return ContainsMatcher(
        CasedString(fmt::format("{} <.. {} : {}[friend]", from, to, pattern),
            caseSensitivity));
}
}

ContainsMatcher IsPackage(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString("package [" + str + "]", caseSensitivity));
}

namespace mermaid {
ContainsMatcher IsPackage(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString("subgraph " + str, caseSensitivity));
}
}

ContainsMatcher IsFolder(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString("folder \"" + str + "\"", caseSensitivity));
}

ContainsMatcher IsFile(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString("file \"" + str + "\"", caseSensitivity));
}

namespace mermaid {
ContainsMatcher IsFolder(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString("subgraph " + str, caseSensitivity));
}

ContainsMatcher IsFile(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(CasedString(str + "[", caseSensitivity));
}
}

ContainsMatcher IsDeprecated(std::string const &str,
    CaseSensitive::Choice caseSensitivity = CaseSensitive::Yes)
{
    return ContainsMatcher(
        CasedString(str + " <<deprecated>> ", caseSensitivity));
}

namespace json {
struct File {
    explicit File(const std::string &f)
        : file{f}
    {
    }

    const std::string file;
};

std::optional<nlohmann::json> get_element_by_id(
    const nlohmann::json &j, const std::string &id)
{
    if (!j.contains("elements"))
        return {};

    for (const nlohmann::json &e : j["elements"]) {
        if (e["id"] == id)
            return {e};

        if (e["type"] == "namespace" || e["type"] == "folder") {
            auto maybe_e = get_element_by_id(e, id);
            if (maybe_e)
                return maybe_e;
        }
    }

    return {};
}

std::optional<nlohmann::json> get_element(
    const nlohmann::json &j, const std::string &name)
{
    if (!j.contains("elements"))
        return {};

    for (const nlohmann::json &e : j["elements"]) {
        if (e["display_name"] == name)
            return {e};

        if (e["type"] == "namespace" || e["type"] == "folder" ||
            e["type"] == "directory" || e["type"] == "module") {
            auto maybe_e = get_element(e, name);
            if (maybe_e)
                return maybe_e;
        }
    }

    return {};
}

std::optional<nlohmann::json> get_participant(
    const nlohmann::json &j, const std::string &name)
{
    if (!j.contains("participants"))
        return {};

    for (const nlohmann::json &e : j.at("participants")) {
        if (e["display_name"] == name)
            return {e};
    }

    return {};
}

auto get_relationship(const nlohmann::json &j, const nlohmann::json &from,
    const nlohmann::json &to, const std::string &type)
{
    return std::find_if(j["relationships"].begin(), j["relationships"].end(),
        [&](const auto &it) {
            return (it["source"] == from) && (it["destination"] == to) &&
                (it["type"] == type);
        });
}

auto get_relationship(const nlohmann::json &j, const std::string &from,
    const std::string &to, const std::string &type)
{
    auto source = get_element(j, from);
    auto destination = get_element(j, to);

    if (!(source && destination))
        return j["relationships"].end();

    return get_relationship(j, source->at("id"), destination->at("id"), type);
}

std::string expand_name(const nlohmann::json &j, const std::string &name)
{
    return name;
}

bool HasTitle(const nlohmann::json &j, const std::string &title)
{
    return j.contains("title") && j["title"] == title;
}

bool IsClass(const nlohmann::json &j, const std::string &name)
{
    auto e = get_element(j, expand_name(j, name));
    return e && e->at("type") == "class";
}

bool IsStruct(const nlohmann::json &j, const std::string &name)
{
    auto e = get_element(j, expand_name(j, name));
    return e && e->at("type") == "class" && e->at("is_struct");
}

bool InPublicModule(const nlohmann::json &j, const std::string &element,
    const std::string &module)
{
    auto e = get_element(j, expand_name(j, element));
    return e && e->contains("module") && e->at("module")["name"] == module &&
        !e->at("module")["is_private"];
}

bool InPrivateModule(const nlohmann::json &j, const std::string &element,
    const std::string &module)
{
    auto e = get_element(j, expand_name(j, element));
    return e && e->contains("module") && e->at("module")["name"] == module &&
        e->at("module")["is_private"];
}

bool IsAbstractClass(const nlohmann::json &j, const std::string &name)
{
    auto e = get_element(j, expand_name(j, name));
    return e && (e->at("type") == "class") && (e->at("is_abstract") == true);
}

bool IsClassTemplate(const nlohmann::json &j, const std::string &name)
{
    auto e = get_element(j, expand_name(j, name));
    return e && e->at("type") == "class" && e->at("is_template") == true;
}

bool IsConcept(const nlohmann::json &j, const std::string &name)
{
    auto e = get_element(j, expand_name(j, name));
    return e && e->at("type") == "concept";
}

bool IsEnum(const nlohmann::json &j, const std::string &name)
{
    auto e = get_element(j, expand_name(j, name));
    return e && e->at("type") == "enum";
}

bool IsPackage(const nlohmann::json &j, const std::string &name,
    const std::string &type = "namespace")
{
    auto e = get_element(j, expand_name(j, name));
    return e && e->at("type") == type;
}

struct NamespacePackage { };
struct ModulePackage { };
struct DirectoryPackage { };

template <typename PackageT> std::string package_type_name();

template <> std::string package_type_name<NamespacePackage>()
{
    return "namespace";
}

template <> std::string package_type_name<ModulePackage>() { return "module"; }

template <> std::string package_type_name<DirectoryPackage>()
{
    return "directory";
}

template <typename PackageT, typename... Args>
bool IsPackagePath(
    const nlohmann::json &j, const std::string &head, Args... args)
{
    if constexpr (sizeof...(Args) == 0) {
        auto e = get_element(j, expand_name(j, head));

        return e && e->at("type") == package_type_name<PackageT>();
    }
    else {
        auto e = get_element(j, head);
        if (!e.has_value())
            return false;

        return IsPackagePath<PackageT>(*e, args...);
    }
}

template <typename... Args>
bool IsNamespacePackage(
    const nlohmann::json &j, const std::string &head, Args... args)
{
    return IsPackagePath<NamespacePackage>(
        j, head, std::forward<Args>(args)...);
}

template <typename... Args>
bool IsDirectoryPackage(
    const nlohmann::json &j, const std::string &head, Args... args)
{
    return IsPackagePath<DirectoryPackage>(
        j, head, std::forward<Args>(args)...);
}

template <typename... Args>
bool IsModulePackage(
    const nlohmann::json &j, const std::string &head, Args... args)
{
    return IsPackagePath<ModulePackage>(j, head, std::forward<Args>(args)...);
}

bool IsFolder(const nlohmann::json &j, const std::string &name)
{
    auto e = get_element(j, name);
    return e && e->at("type") == "folder";
}

bool IsFile(const nlohmann::json &j, const std::string &name)
{
    auto e = get_element(j, name);
    return e && e->at("type") == "file";
}

bool IsSystemHeader(const nlohmann::json &j, const std::string &name)
{
    auto e = get_element(j, name);
    return e && e->at("type") == "file" && e->at("file_kind") == "header" &&
        e->at("is_system");
}

bool IsHeader(const nlohmann::json &j, const std::string &name)
{
    auto e = get_element(j, name);
    return e && e->at("type") == "file" && e->at("file_kind") == "header" &&
        !e->at("is_system");
}

bool IsDeprecated(const nlohmann::json &j, const std::string &name)
{
    auto e = get_element(j, expand_name(j, name));
    return e && e->at("is_deprecated") == true;
}

bool IsBaseClass(const nlohmann::json &j, const std::string &base,
    const std::string &subclass)
{
    auto base_el = get_element(j, expand_name(j, base));
    auto subclass_el = get_element(j, expand_name(j, subclass));

    if (!base_el || !subclass_el)
        return false;

    const nlohmann::json &bases = (*subclass_el)["bases"];

    return std::find_if(bases.begin(), bases.end(), [&](const auto &it) {
        return it["id"] == base_el.value()["id"];
    }) != bases.end();
}

bool IsMethod(
    const nlohmann::json &j, const std::string &cls, const std::string &name)
{
    auto sc = get_element(j, expand_name(j, cls));

    if (!sc)
        return false;

    const nlohmann::json &methods = (*sc)["methods"];

    return std::find_if(methods.begin(), methods.end(), [&](const auto &it) {
        return it["name"] == name;
    }) != methods.end();
}

bool IsField(const nlohmann::json &j, const std::string &cls,
    const std::string &name, const std::string &type)
{
    auto sc = get_element(j, expand_name(j, cls));

    if (!sc)
        return false;

    const nlohmann::json &members = (*sc)["members"];

    return std::find_if(members.begin(), members.end(), [&](const auto &it) {
        return it["name"] == name && it["type"] == type;
    }) != members.end();
}

bool IsAssociation(nlohmann::json j, const std::string &from,
    const std::string &to, const std::string &label = "")
{
    auto rel = get_relationship(
        j, expand_name(j, from), expand_name(j, to), "association");

    if (rel == j["relationships"].end())
        return false;

    if (!label.empty() && rel->at("label") != label)
        return false;

    return true;
}

bool IsComposition(nlohmann::json j, const std::string &from,
    const std::string &to, const std::string &label = "")
{
    auto rel = get_relationship(
        j, expand_name(j, from), expand_name(j, to), "composition");

    if (rel == j["relationships"].end())
        return false;

    if (!label.empty() && rel->at("label") != label)
        return false;

    return true;
}

bool IsAggregation(nlohmann::json j, const std::string &from,
    const std::string &to, const std::string &label = "")
{
    auto rel = get_relationship(
        j, expand_name(j, from), expand_name(j, to), "aggregation");

    if (rel == j["relationships"].end())
        return false;

    if (!label.empty() && rel->at("label") != label)
        return false;

    return true;
}

namespace detail {
bool is_dependency_impl(
    nlohmann::json j, const std::string &from, const std::string &to)
{
    auto rel = get_relationship(j, from, to, "dependency");

    return rel != j["relationships"].end();
}

} // namespace detail

bool IsDependency(
    nlohmann::json j, const std::string &from, const std::string &to)
{
    return detail::is_dependency_impl(
        j, expand_name(j, from), expand_name(j, to));
}

bool IsDependency(nlohmann::json j, const File &from, const File &to)
{
    return detail::is_dependency_impl(j, from.file, to.file);
}

bool IsInstantiation(
    nlohmann::json j, const std::string &from, const std::string &to)
{
    auto rel = get_relationship(
        j, expand_name(j, to), expand_name(j, from), "instantiation");

    return rel != j["relationships"].end();
}

bool IsFriend(nlohmann::json j, const std::string &from, const std::string &to)
{
    auto rel = get_relationship(
        j, expand_name(j, from), expand_name(j, to), "friendship");

    return rel != j["relationships"].end();
}

bool IsInnerClass(
    nlohmann::json j, const std::string &from, const std::string &to)
{
    auto rel = get_relationship(
        j, expand_name(j, to), expand_name(j, from), "containment");

    return rel != j["relationships"].end();
}

bool IsParticipant(
    const nlohmann::json &j, const std::string &name, const std::string &type)
{
    auto p = get_participant(j, expand_name(j, name));

    return p && (p->at("type") == type);
}

bool IsFunctionParticipant(const nlohmann::json &j, const std::string &name)
{
    return IsParticipant(j, name, "function");
}

bool IsClassParticipant(const nlohmann::json &j, const std::string &name)
{
    return IsParticipant(j, name, "class");
}

bool IsFileParticipant(const nlohmann::json &j, const std::string &name)
{
    return IsParticipant(j, name, "file");
}

namespace detail {
int find_message_nested(const nlohmann::json &j, const std::string &from,
    const std::string &to, const std::string &msg,
    std::optional<std::string> return_type, const nlohmann::json &from_p,
    const nlohmann::json &to_p, int &count)
{
    const auto &messages = j["messages"];

    int res{-1};

    for (const auto &m : messages) {
        if (m.contains("branches")) {
            for (const auto &b : m["branches"]) {
                auto nested_res = find_message_nested(
                    b, from, to, msg, return_type, from_p, to_p, count);

                if (nested_res >= 0)
                    return nested_res;
            }
        }
        else if (m.contains("messages")) {
            auto nested_res = find_message_nested(
                m, from, to, msg, return_type, from_p, to_p, count);

            if (nested_res >= 0)
                return nested_res;
        }
        else {
            if ((m["from"]["participant_id"] == from_p["id"]) &&
                (m["to"]["participant_id"] == to_p["id"]) &&
                (m["name"] == msg) &&
                (!return_type || m["return_type"] == *return_type))
                return count;

            count++;
        }
    }

    return res;
}

int find_message_impl(const nlohmann::json &j, const std::string &from,
    const std::string &to, const std::string &msg,
    std::optional<std::string> return_type)
{

    auto from_p = get_participant(j, from);
    auto to_p = get_participant(j, to);

    // TODO: support diagrams with multiple sequences...
    const auto &sequence_0 = j["sequences"][0];

    int count{0};

    auto res = detail::find_message_nested(
        sequence_0, from, to, msg, return_type, *from_p, *to_p, count);

    if (res >= 0)
        return res;

    throw std::runtime_error(
        fmt::format("No such message {} {} {}", from, to, msg));
}

} // namespace detail

int FindMessage(const nlohmann::json &j, const File &from, const File &to,
    const std::string &msg)
{
    return detail::find_message_impl(j, from.file, to.file, msg, {});
}

int FindMessage(const nlohmann::json &j, const std::string &from,
    const std::string &to, const std::string &msg,
    std::optional<std::string> return_type = {})
{
    return detail::find_message_impl(
        j, expand_name(j, from), expand_name(j, to), msg, return_type);
}

struct message_test_spec_t {
    std::string from;
    std::string to;
    std::optional<std::string> return_type;

    bool operator==(const message_test_spec_t &r) const noexcept
    {
        return from == r.from && to == r.to && return_type == r.return_type;
    }
};

void from_json(const nlohmann::json &j, message_test_spec_t &p)
{
    j.at("from").at("activity_id").get_to(p.from);
    j.at("to").at("activity_id").get_to(p.to);
    j.at("return_type").get_to(p.return_type);
}

std::string get_activity_id(
    const nlohmann::json &j, const std::string &display_name)
{
    for (const auto &p : j["participants"]) {
        if (p.contains("activities")) {
            for (const auto &a : p["activities"]) {
                if (a["display_name"] == display_name) {
                    return a["id"];
                }
            }
        }
        else if (p["display_name"] == display_name) {
            return p["id"];
        }
    }

    return {};
}

bool HasMessageChain(
    const nlohmann::json &j, std::vector<message_test_spec_t> msgs)
{
    std::vector<message_test_spec_t> full_name_messages;
    std::transform(msgs.begin(), msgs.end(),
        std::back_inserter(full_name_messages),
        [&j](const message_test_spec_t &m) {
            auto res = m;
            res.from = get_activity_id(j, m.from);
            res.to = get_activity_id(j, m.to);
            return res;
        });

    for (const auto &seq : j.at("sequences"))
        for (const auto &mc : seq.at("message_chains")) {
            auto mc_msgs =
                mc.at("messages").get<std::vector<message_test_spec_t>>();

            if (full_name_messages == mc_msgs)
                return true;
        }

    return false;
}

} // namespace json
}
}
}
*/