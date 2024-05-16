/**
 * tests/t20032/test_case.h
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

TEST_CASE("t20032")
{
    using namespace clanguml::test;

    auto [config, db, diagram, model] =
        CHECK_SEQUENCE_MODEL("t20032", "t20032_sequence");

    CHECK_SEQUENCE_DIAGRAM(config, diagram, *model, [](const auto &src) {
        REQUIRE(MessageOrder(src,
            {
                //
                {"tmain(int,char **)", "B", "b(int)"},          //
                {"B", "A", "a1(int)"},                          //
                {"A", "B", "int", Response{}},                  //
                {"B", "tmain(int,char **)", "int", Response{}}, //

                {"tmain(int,char **)", "B", "b(double)"},          //
                {"B", "A", "a2(double)"},                          //
                {"A", "B", "double", Response{}},                  //
                {"B", "tmain(int,char **)", "double", Response{}}, //

                {"tmain(int,char **)", "B", "b(const char *)"},          //
                {"B", "A", "a3(const char *)"},                          //
                {"A", "B", "const char *", Response{}},                  //
                {"B", "tmain(int,char **)", "const char *", Response{}} //
            }));
    });
    /*
    {
        auto src = generate_sequence_puml(diagram, *model);
        AliasMatcher _A(src);

        REQUIRE_THAT(src, StartsWith("@startuml"));
        REQUIRE_THAT(src, EndsWith("@enduml\n"));

        // Check if all calls exist
        REQUIRE_THAT(src, HasCall(_A("tmain(int,char **)"), _A("B"), "b(int)"));
        REQUIRE_THAT(
            src, HasResponse(_A("tmain(int,char **)"), _A("B"), "int"));

        REQUIRE_THAT(src, HasCall(_A("B"), _A("A"), "a1(int)"));
        REQUIRE_THAT(src, HasResponse(_A("B"), _A("A"), "int"));

        REQUIRE_THAT(
            src, HasCall(_A("tmain(int,char **)"), _A("B"), "b(double)"));
        REQUIRE_THAT(src, HasCall(_A("B"), _A("A"), "a2(double)"));
        REQUIRE_THAT(src, HasResponse(_A("B"), _A("A"), "double"));

        REQUIRE_THAT(
            src, HasCall(_A("tmain(int,char **)"), _A("B"), "b(const char *)"));
        REQUIRE_THAT(src, HasCall(_A("B"), _A("A"), "a3(const char *)"));
        REQUIRE_THAT(src, HasResponse(_A("B"), _A("A"), "const char *"));

        save_puml(config.output_directory(), diagram->name + ".puml", src);
    }

    {
        auto j = generate_sequence_json(diagram, *model);

        using namespace json;

        std::vector<int> messages = {
            FindMessage(j, "tmain(int,char **)", "B", "b(int)", "int"),
            FindMessage(j, "B", "A", "a1(int)", "int"),
            FindMessage(j, "tmain(int,char **)", "B", "b(double)"),
            FindMessage(j, "B", "A", "a2(double)"),
            FindMessage(j, "tmain(int,char **)", "B", "b(const char *)"),
            FindMessage(j, "B", "A", "a3(const char *)")};

        REQUIRE(std::is_sorted(messages.begin(), messages.end()));

        save_json(config.output_directory(), diagram->name + ".json", j);
    }

    {
        auto src = generate_sequence_mermaid(diagram, *model);

        mermaid::SequenceDiagramAliasMatcher _A(src);
        using mermaid::HasCall;
        using mermaid::HasResponse;

        REQUIRE_THAT(src, HasCall(_A("tmain(int,char **)"), _A("B"), "b(int)"));
        REQUIRE_THAT(
            src, HasResponse(_A("tmain(int,char **)"), _A("B"), "int"));

        REQUIRE_THAT(src, HasCall(_A("B"), _A("A"), "a1(int)"));
        REQUIRE_THAT(src, HasResponse(_A("B"), _A("A"), "int"));

        REQUIRE_THAT(
            src, HasCall(_A("tmain(int,char **)"), _A("B"), "b(double)"));
        REQUIRE_THAT(src, HasCall(_A("B"), _A("A"), "a2(double)"));
        REQUIRE_THAT(src, HasResponse(_A("B"), _A("A"), "double"));

        REQUIRE_THAT(
            src, HasCall(_A("tmain(int,char **)"), _A("B"), "b(const char *)"));
        REQUIRE_THAT(src, HasCall(_A("B"), _A("A"), "a3(const char *)"));
        REQUIRE_THAT(src, HasResponse(_A("B"), _A("A"), "const char *"));

        save_mermaid(config.output_directory(), diagram->name + ".mmd", src);
    }*/
}