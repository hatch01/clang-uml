/**
 * src/sequence_diagram/model/diagram.cc
 *
 * Copyright (c) 2021-2022 Bartek Kryza <bkryza@gmail.com>
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

#include "diagram.h"

#include <functional>
#include <memory>

namespace clanguml::sequence_diagram::model {

common::model::diagram_t diagram::type() const
{
    return common::model::diagram_t::kSequence;
}

common::optional_ref<common::model::diagram_element> diagram::get(
    const std::string &full_name) const
{
    for (const auto &[id, participant] : participants_) {
        if (participant->full_name(false) == full_name)
            return {*participant};
    }

    return {};
}

common::optional_ref<common::model::diagram_element> diagram::get(
    const common::model::diagram_element::id_t id) const
{
    if (participants_.find(id) != participants_.end())
        return {*participants_.at(id)};

    return {};
}

std::string diagram::to_alias(const std::string &full_name) const
{
    return full_name;
}

inja::json diagram::context() const
{
    inja::json ctx;
    ctx["name"] = name();
    ctx["type"] = "sequence";

    inja::json::array_t elements{};

    // Add classes
    for (const auto &[id, p] : participants_) {
        elements.emplace_back(p->context());
    }

    ctx["elements"] = elements;

    return ctx;
}

void diagram::add_participant(std::unique_ptr<participant> p)
{
    const auto participant_id = p->id();

    if (participants_.find(participant_id) == participants_.end()) {
        LOG_DBG("Adding '{}' participant: {}, {} [{}]", p->type_name(),
            p->full_name(false), p->id(),
            p->type_name() == "method"
                ? dynamic_cast<method *>(p.get())->method_name()
                : "");

        participants_.emplace(participant_id, std::move(p));
    }
}

void diagram::add_active_participant(common::model::diagram_element::id_t id)
{
    active_participants_.emplace(id);
}

activity &diagram::get_activity(common::model::diagram_element::id_t id)
{
    return sequences_.at(id);
}

void diagram::add_for_stmt(
    const common::model::diagram_element::id_t current_caller_id)
{
    add_loop_stmt(current_caller_id, common::model::message_t::kFor);
}

void diagram::end_for_stmt(
    const common::model::diagram_element::id_t current_caller_id)
{
    end_loop_stmt(current_caller_id, common::model::message_t::kForEnd);
}

void diagram::add_while_stmt(
    const common::model::diagram_element::id_t current_caller_id)
{
    add_loop_stmt(current_caller_id, common::model::message_t::kWhile);
}

void diagram::end_while_stmt(
    const common::model::diagram_element::id_t current_caller_id)
{
    end_loop_stmt(current_caller_id, common::model::message_t::kWhileEnd);
}

void diagram::add_do_stmt(
    const common::model::diagram_element::id_t current_caller_id)
{
    add_loop_stmt(current_caller_id, common::model::message_t::kDo);
}

void diagram::end_do_stmt(
    const common::model::diagram_element::id_t current_caller_id)
{
    end_loop_stmt(current_caller_id, common::model::message_t::kDoEnd);
}

void diagram::add_loop_stmt(
    const common::model::diagram_element::id_t current_caller_id,
    common::model::message_t type)
{
    using clanguml::common::model::message_t;

    if (current_caller_id == 0)
        return;

    if (sequences_.find(current_caller_id) == sequences_.end()) {
        activity a{current_caller_id};
        sequences_.insert({current_caller_id, std::move(a)});
    }

    get_activity(current_caller_id).add_message({type, current_caller_id});
}

void diagram::end_loop_stmt(
    const common::model::diagram_element::id_t current_caller_id,
    common::model::message_t type)
{
    using clanguml::common::model::message_t;

    if (current_caller_id == 0)
        return;

    message m{type, current_caller_id};

    message_t loop_type = message_t::kWhile;

    if (type == message_t::kForEnd)
        loop_type = message_t::kFor;
    else if (type == message_t::kDoEnd)
        loop_type = message_t::kDo;

    if (sequences_.find(current_caller_id) != sequences_.end()) {
        auto &current_messages = get_activity(current_caller_id).messages();

        if (current_messages.back().type() == loop_type) {
            current_messages.pop_back();
        }
        else {
            current_messages.emplace_back(std::move(m));
        }
    }
}

void diagram::add_if_stmt(
    const common::model::diagram_element::id_t current_caller_id,
    common::model::message_t type)
{
    using clanguml::common::model::message_t;

    if (sequences_.find(current_caller_id) == sequences_.end()) {
        activity a{current_caller_id};
        sequences_.insert({current_caller_id, std::move(a)});
    }

    get_activity(current_caller_id).add_message({type, current_caller_id});
}

void diagram::end_if_stmt(
    const common::model::diagram_element::id_t current_caller_id,
    common::model::message_t type)
{
    using clanguml::common::model::message_t;

    message m{message_t::kIfEnd, current_caller_id};

    if (sequences_.find(current_caller_id) != sequences_.end()) {

        auto &current_messages = get_activity(current_caller_id).messages();
        // Remove the if/else messages if there were no calls
        // added to the diagram between them
        auto last_if_it =
            std::find_if(current_messages.rbegin(), current_messages.rend(),
                [](const message &m) { return m.type() == message_t::kIf; });

        bool last_if_block_is_empty =
            std::none_of(current_messages.rbegin(), last_if_it,
                [](const message &m) { return m.type() == message_t::kCall; });

        if (!last_if_block_is_empty) {
            current_messages.emplace_back(std::move(m));
        }
        else {
            current_messages.erase(
                (last_if_it + 1).base(), current_messages.end());
        }
    }
}

bool diagram::started() const { return started_; }

void diagram::started(bool s) { started_ = s; }

std::map<common::model::diagram_element::id_t, activity> &diagram::sequences()
{
    return sequences_;
}

const std::map<common::model::diagram_element::id_t, activity> &
diagram::sequences() const
{
    return sequences_;
}

std::map<common::model::diagram_element::id_t, std::unique_ptr<participant>> &
diagram::participants()
{
    return participants_;
}

const std::map<common::model::diagram_element::id_t,
    std::unique_ptr<participant>> &
diagram::participants() const
{
    return participants_;
}

std::set<common::model::diagram_element::id_t> &diagram::active_participants()
{
    return active_participants_;
};

const std::set<common::model::diagram_element::id_t> &
diagram::active_participants() const
{
    return active_participants_;
};

void diagram::print() const
{
    LOG_TRACE(" --- Participants ---");
    for (const auto &[id, participant] : participants_) {
        LOG_DBG("{} - {}", id, participant->to_string());
    }

    LOG_TRACE(" --- Activities ---");
    for (const auto &[from_id, act] : sequences_) {

        LOG_TRACE("Sequence id={}:", from_id);

        const auto &from_activity = *(participants_.at(from_id));

        LOG_TRACE("   Activity id={}, from={}:", act.from(),
            from_activity.full_name(false));

        for (const auto &message : act.messages()) {
            if (participants_.find(message.from()) == participants_.end())
                continue;

            const auto &from_participant = *participants_.at(message.from());

            if (participants_.find(message.to()) == participants_.end()) {
                LOG_TRACE("       Message from={}, from_id={}, "
                          "to={}, to_id={}, name={}, type={}",
                    from_participant.full_name(false), from_participant.id(),
                    "__UNRESOLVABLE_ID__", message.to(), message.message_name(),
                    to_string(message.type()));
            }
            else {
                const auto &to_participant = *participants_.at(message.to());

                LOG_TRACE("       Message from={}, from_id={}, "
                          "to={}, to_id={}, name={}, type={}",
                    from_participant.full_name(false), from_participant.id(),
                    to_participant.full_name(false), to_participant.id(),
                    message.message_name(), to_string(message.type()));
            }
        }
    }
}

}

namespace clanguml::common::model {
template <>
bool check_diagram_type<clanguml::sequence_diagram::model::diagram>(diagram_t t)
{
    return t == diagram_t::kSequence;
}
}
