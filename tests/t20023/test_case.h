/**
 * tests/t20023/test_case.h
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

TEST_CASE("t20023")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20023", "t20023_sequence");

    CHECK_SEQUENCE_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {"tmain()", "A", "a()"}, //
                {"A", "A", "a2()"},      //
                {"A", "A", "a2()"},      //
                {"A", "A", "a3()"},      //
                {"A", "A", "a4()"},      //

            }));
    });
    /*
        {
            auto src = generate_sequence_puml(diagram, *model);
            AliasMatcher _A(src);

            REQUIRE_THAT(src, StartsWith("@startuml"));
            REQUIRE_THAT(src, EndsWith("@enduml\n"));

            // Check if all calls exist
            REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("A"), "a()"));
            REQUIRE_THAT(src, HasCall(_A("A"), _A("A"), "a1()"));
            REQUIRE_THAT(src, HasCall(_A("A"), _A("A"), "a2()"));
            REQUIRE_THAT(src, HasCall(_A("A"), _A("A"), "a3()"));
            REQUIRE_THAT(src, HasCall(_A("A"), _A("A"), "a4()"));

            save_puml(config.output_directory(), diagram->name + ".puml", src);
        }

        {
            auto j = generate_sequence_json(diagram, *model);

            using namespace json;

            std::vector<int> messages = {FindMessage(j, "tmain()", "A", "a()"),
                FindMessage(j, "A", "A", "a1()"), FindMessage(j, "A", "A",
       "a2()"), FindMessage(j, "A", "A", "a3()"), FindMessage(j, "A", "A",
       "a4()")};

            REQUIRE(std::is_sorted(messages.begin(), messages.end()));

            save_json(config.output_directory(), diagram->name + ".json", j);
        }

        {
            auto src = generate_sequence_mermaid(diagram, *model);

            mermaid::SequenceDiagramAliasMatcher _A(src);
            using mermaid::HasCall;

            REQUIRE_THAT(src, HasCall(_A("tmain()"), _A("A"), "a()"));
            REQUIRE_THAT(src, HasCall(_A("A"), _A("A"), "a1()"));
            REQUIRE_THAT(src, HasCall(_A("A"), _A("A"), "a2()"));
            REQUIRE_THAT(src, HasCall(_A("A"), _A("A"), "a3()"));
            REQUIRE_THAT(src, HasCall(_A("A"), _A("A"), "a4()"));

            save_mermaid(config.output_directory(), diagram->name + ".mmd",
       src);
        }
        */
}