// Copyright (C) 2017-2023 Jonathan Müller and cppast contributors
// SPDX-License-Identifier: MIT

#include <cppast/cpp_alias_template.hpp>

#include "test_parser.hpp"

using namespace cppast;

TEST_CASE("cpp_alias_template")
{
// Ignoring test for full argument parsing for now : must be fixed !
#ifndef CPPAST_TEMPLATE_FULL_ARGUMENTS_PARSING
    // no need to check advanced types here nor template parameters
    auto code = R"(
/// template<typename T>
/// using a=int;
template <typename T>
using a = int;

/// template<int I,typename T=void>
/// using b=T;
template <int I, typename T = void>
using b = T;

/// template<typename T>
/// using c=T const*;
template <typename T>
using c = const T*;

/// template<typename T>
/// using d=a<void>;
template <typename T>
using d = a<void>;

/// template<int I>
/// using e=b<I> const;
template <int I>
using e = const b<I>;

/// template<int I>
/// using f=b<I < a<int>{(0 , 1)}, int>;
template <int I>
using f = b<I < a<int>{(0,1)}, int>;

/// template<typename T,template<typename>class Templ>
/// using g=Templ<T>;
template <typename T, template <typename> class Templ>
using g = Templ<T>;

/// template<typename T>
/// using h=g<T, a>;
template <typename T>
using h = g<T, a>;
)";

    cpp_entity_index idx;
    auto             file = parse(idx, "cpp_alias_template.cpp", code);
    auto count = test_visit<cpp_alias_template>(*file, [&](const cpp_alias_template& alias) {
        REQUIRE(is_templated(alias.type_alias()));
        REQUIRE(!alias.scope_name());

        if (alias.name() == "a")
        {
            check_template_parameters(alias, {{cpp_entity_kind::template_type_parameter_t, "T"}});
            REQUIRE(equal_types(idx, alias.type_alias().underlying_type(),
                                *cpp_builtin_type::build(cpp_int)));
        }
        else if (alias.name() == "b")
        {
            check_template_parameters(alias, {{cpp_entity_kind::non_type_template_parameter_t, "I"},
                                              {cpp_entity_kind::template_type_parameter_t, "T"}});
            REQUIRE(equal_types(idx, alias.type_alias().underlying_type(),
                                *cpp_template_parameter_type::build(
                                    cpp_template_type_parameter_ref(cpp_entity_id(""), "T"))));
        }
        else if (alias.name() == "c")
        {
            check_template_parameters(alias, {{cpp_entity_kind::template_type_parameter_t, "T"}});
            auto param = cpp_template_parameter_type::build(
                cpp_template_type_parameter_ref(cpp_entity_id(""), "T"));
            REQUIRE(equal_types(idx, alias.type_alias().underlying_type(),
                                *cpp_pointer_type::build(
                                    cpp_cv_qualified_type::build(std::move(param), cpp_cv_const))));
        }
        else if (alias.name() == "d")
        {
            check_template_parameters(alias, {{cpp_entity_kind::template_type_parameter_t, "T"}});
            #ifndef CPPAST_TEMPLATE_FULL_ARGUMENTS_PARSING
            cpp_template_instantiation_type::builder builder(
                cpp_template_ref(cpp_entity_id(""), "a"));
            builder.add_unexposed_arguments("void");
            REQUIRE(equal_types(idx, alias.type_alias().underlying_type(), *builder.finish()));
            #else                         
            // need to write the code for full argument parsing using add_argument
               /* std::unique_ptr<cpp_builtin_type> voidType = cpp_builtin_type::build(cpp_builtin_type_kind::cpp_void);
                cpp_template_argument cppastTmplArg(std::move(voidType));
                builder.add_argument(std::move(cppastTmplArg));*/
            #endif
        }
        else if (alias.name() == "e")
        {
            check_template_parameters(alias,
                                      {{cpp_entity_kind::non_type_template_parameter_t, "I"}});
            cpp_template_instantiation_type::builder builder(
                cpp_template_ref(cpp_entity_id(""), "b"));
            #ifndef CPPAST_TEMPLATE_FULL_ARGUMENTS_PARSING    
            builder.add_unexposed_arguments("I");
            REQUIRE(equal_types(idx, alias.type_alias().underlying_type(),
                                *cpp_cv_qualified_type::build(builder.finish(), cpp_cv_const)));
            #else                         
            // need to write the code for full argument parsing using add_argument
            #endif                    
        }
        else if (alias.name() == "f")
        {
            check_template_parameters(alias,
                                      {{cpp_entity_kind::non_type_template_parameter_t, "I"}});

            cpp_template_instantiation_type::builder builder(
                cpp_template_ref(cpp_entity_id(""), "b"));
            #ifndef CPPAST_TEMPLATE_FULL_ARGUMENTS_PARSING     
            builder.add_unexposed_arguments("I < a<int>{(0 , 1)}, int");
            REQUIRE(equal_types(idx, alias.type_alias().underlying_type(), *builder.finish()));
            #else                         
            // need to write the code for full argument parsing using add_argument
            #endif
        }
        else if (alias.name() == "g")
        {
            check_template_parameters(alias,
                                      {{cpp_entity_kind::template_type_parameter_t, "T"},
                                       {cpp_entity_kind::template_template_parameter_t, "Templ"}});

            cpp_template_instantiation_type::builder builder(
                cpp_template_ref(cpp_entity_id(""), "Templ"));
            #ifndef CPPAST_TEMPLATE_FULL_ARGUMENTS_PARSING    
            builder.add_unexposed_arguments("T");
            REQUIRE(equal_types(idx, alias.type_alias().underlying_type(), *builder.finish()));
            #else                         
            // need to write the code for full argument parsing using add_argument
            #endif
        }
        else if (alias.name() == "h")
        {
            check_template_parameters(alias, {{cpp_entity_kind::template_type_parameter_t, "T"}});

            cpp_template_instantiation_type::builder builder(
                cpp_template_ref(cpp_entity_id(""), "g"));
            #ifndef CPPAST_TEMPLATE_FULL_ARGUMENTS_PARSING    
            builder.add_unexposed_arguments("T, a");
            REQUIRE(equal_types(idx, alias.type_alias().underlying_type(), *builder.finish()));
            #else                         
            // need to write the code for full argument parsing using add_argument
            #endif
        }
        else
            REQUIRE(false);
    });
    REQUIRE(count == 8u);
    #endif
}
